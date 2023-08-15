#include <iostream>
#include <future>
#include <atomic>
#include <Windows.h>
#include <AudioClient.h>
#include <AudioPolicy.h>
#include <MMDeviceApi.h>

#define BUFFER_TIME_100NS       (10000000)  // 100ns

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)     \
    do {                    \
        if ((p)) {          \
            (p)->Release(); \
            (p) = NULL;     \
        }                   \
    } while (false)
#endif


const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);


IMMDeviceEnumerator* pDeviceEnum = NULL;
IMMDevice* pDevice = NULL;
IAudioClient* pAudioClient = NULL;
WAVEFORMATEX* pWaveFormat = NULL;
IAudioCaptureClient* pAudioCaptureClient = NULL;

std::atomic_bool exitFlag;
std::shared_future<void> asyncQuery;

FILE* fOutput = NULL;

bool WriteSample(BYTE* buffer, UINT32 bufferSize) {
    return fwrite(buffer, 1, bufferSize, fOutput) == bufferSize;
}

HRESULT InitRecord() {
    HRESULT hr;

    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pDeviceEnum);
    if (FAILED(hr)) {
        printf("Create device enumerator failed, hr: 0x%x", hr);
        return hr;
    }

    hr = pDeviceEnum->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr)) {
        printf("Get default audio device failed, hr: 0x%x", hr);
        return hr;
    }

    hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
    if (FAILED(hr)) {
        printf("Create audio client failed, hr: 0x%x", hr);
        return hr;
    }

    hr = pAudioClient->GetMixFormat(&pWaveFormat);
    if (FAILED(hr)) {
        printf("Get mix format failed, hr: 0x%x", hr);
        return hr;
    }

    printf("Channel: %d, SamplesPerSec: %d, BitsPerSample: %d\n", pWaveFormat->nChannels, pWaveFormat->nSamplesPerSec, pWaveFormat->wBitsPerSample);

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, BUFFER_TIME_100NS, 0, pWaveFormat, NULL);
    if (FAILED(hr)) {
        // 兼容Nahimic音频驱动
        // https://github.com/rainmeter/rainmeter/commit/0a3dfa35357270512ec4a3c722674b67bff541d6
        // https://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/bd8cd9f2-974f-4a9f-8e9c-e83001819942/iaudioclient-initialize-failure

        // 初始化失败，尝试使用立体声格式进行初始化
        pWaveFormat->nChannels = 2;
        pWaveFormat->nBlockAlign = (2 * pWaveFormat->wBitsPerSample) / 8;
        pWaveFormat->nAvgBytesPerSec = pWaveFormat->nSamplesPerSec * pWaveFormat->nBlockAlign;

        hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, BUFFER_TIME_100NS, 0, pWaveFormat, NULL);
        if (FAILED(hr)) {
            printf("Initialize audio client failed, hr: 0x%x", hr);
            return hr;
        }
    }

    hr = pAudioClient->GetService(IID_IAudioCaptureClient, (void**)&pAudioCaptureClient);
    if (FAILED(hr)) {
        printf("Get audio capture client failed, hr: 0x%x", hr);
        return hr;
    }


    return S_OK;
}

void QueryAudioSampleThread() {
    UINT32 bufferFrameCount = 0;
    HRESULT hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    if (FAILED(hr)) {
        printf("Get buffer frame count failed, hr: 0x%x", hr);
        return;
    }

    // 根据实际缓冲区中的样本数计算实际填满缓冲区需要的时间
    REFERENCE_TIME  hnsActualDuration = (double)BUFFER_TIME_100NS *
        bufferFrameCount / pWaveFormat->nSamplesPerSec;

    UINT32 packetLength = 0;
    BYTE* buffer = NULL;
    UINT32 numFramesAvailable = 0;
    DWORD flags = 0;
    while (!exitFlag.load())
    {
        // 等待半个缓冲周期
        Sleep(hnsActualDuration / 10000 / 2);

        hr = pAudioCaptureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr)) {
            printf("Get next package size failed, hr: 0x%x", hr);
            break;
        }

        while (packetLength > 0)
        {
            hr = pAudioCaptureClient->GetBuffer(&buffer, &numFramesAvailable, &flags, NULL, NULL);
            if (FAILED(hr)) {
                printf("Get capture buffer failed, hr: 0x%x", hr);
                break;
            }

            if (!WriteSample(buffer, numFramesAvailable * pWaveFormat->nChannels * pWaveFormat->wBitsPerSample / 8)) {
                printf("Write sample to file failed");
            }

            hr = pAudioCaptureClient->ReleaseBuffer(numFramesAvailable);
            if (FAILED(hr)) {
                printf("Release capture buffer failed, hr: 0x%x", hr);
                break;
            }

            hr = pAudioCaptureClient->GetNextPacketSize(&packetLength);
            if (FAILED(hr)) {
                printf("Get next package size failed, hr: 0x%x", hr);
                break;
            }
        }
    }
}

void UnInitRecord() {
    if (pWaveFormat) {
        CoTaskMemFree(pWaveFormat);
        pWaveFormat = NULL;
    }

    SAFE_RELEASE(pDeviceEnum);
    SAFE_RELEASE(pDevice);
    SAFE_RELEASE(pAudioClient);
    SAFE_RELEASE(pAudioCaptureClient);
}

int main()
{
    ::CoInitialize(NULL);

    if (FAILED(InitRecord())) {
        UnInitRecord();
        return 1;
    }

    fopen_s(&fOutput, "Output.pcm", "wb");

    HRESULT hr = pAudioClient->Start();
    if (FAILED(hr)) {
        printf("Start record failed, hr: 0x%x", hr);
        UnInitRecord();
        return 1;
    }

    // 开启独立线程
    asyncQuery = std::async(std::launch::async, QueryAudioSampleThread);

    printf("Press any key to stop...\n");
    getchar();

    if (pAudioClient) {
        pAudioClient->Stop();
    }

    exitFlag.store(true);

    // 等待线程退出
    asyncQuery.wait();

    fclose(fOutput);

    UnInitRecord();

    ::CoUninitialize();
    return 0;
}
