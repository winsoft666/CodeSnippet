//async_select_client.cpp: 定义应用程序的入口点。
//

#include "stdafx.h"
#include "async_select_client.h"

#include <winsock2.h>
#include <vector>
#include <strsafe.h>

#pragma comment(lib, "Ws2_32.lib")

const std::string kIP = "127.0.0.1";
const u_short kPort = 10001;


#define WUM_SOCKET (WM_USER+1)

HWND g_hwnd;

void TraceMsgA(const char *lpFormat, ...) {
	if (!lpFormat)
		return;

	char *pMsgBuffer = NULL;
	unsigned int iMsgBufCount = 0;

	va_list arglist;
	va_start(arglist, lpFormat);
	HRESULT hr = STRSAFE_E_INSUFFICIENT_BUFFER;
	while (hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
		iMsgBufCount += 1024;
		if (pMsgBuffer) {
			free(pMsgBuffer);
			pMsgBuffer = NULL;
		}
		pMsgBuffer = (char*)malloc(iMsgBufCount * sizeof(char));
		if (!pMsgBuffer) {
			break;
		}

		hr = StringCchVPrintfA(pMsgBuffer, iMsgBufCount, lpFormat, arglist);
	}
	va_end(arglist);
	if (hr == S_OK) {
		OutputDebugStringA(pMsgBuffer);
	}

	if (pMsgBuffer) {
		free(pMsgBuffer);
		pMsgBuffer = NULL;
	}
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int err;
	SOCKET s;
	char buf[100] = { 0 };

	switch (message) {
	// (4)
	case WUM_SOCKET:
		if (WSAGETSELECTERROR(lParam)) {
			closesocket((SOCKET)wParam);
			TraceMsgA("connect failed, %d\n", WSAGETSELECTERROR(lParam));
			break;
		}

		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_CONNECT:
			TraceMsgA("connection to server\n");
			break;
		case FD_READ:
			err = recv((SOCKET)wParam, buf, 100, 0);
			if (err > 0) {
				TraceMsgA("recv: %s\n", buf);
			}
			else if (err == 0) {
				closesocket((SOCKET)wParam);
				TraceMsgA("connection closed\n");
			}
			else {
				closesocket((SOCKET)wParam);
				TraceMsgA("recv failed, GLE: %d\n", WSAGetLastError());
			}
			break;
		case FD_WRITE:
			break;
		case FD_CLOSE:
			closesocket((SOCKET)wParam);
			TraceMsgA("connection closed\n");
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


void InitWindow(HINSTANCE hInstance) {
	WCHAR szWindowClass[100] = L"Test";

	WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = szWindowClass;

	RegisterClassExW(&wcex);

	g_hwnd = CreateWindowW(szWindowClass, L"client", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (g_hwnd) {
		ShowWindow(g_hwnd, SW_SHOW);
		UpdateWindow(g_hwnd);
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	InitWindow(hInstance);

	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);

	SOCKET socket_ = INVALID_SOCKET;

	do
	{
		// (1)
		socket_ = ::socket(AF_INET, SOCK_STREAM, 0);
		if (socket_ == INVALID_SOCKET) {
			TraceMsgA("create socket failed, GLE: %d\n", WSAGetLastError());
			break;
		}

		// (2)
		WSAAsyncSelect(socket_, g_hwnd, WUM_SOCKET, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);

		// (3)
		struct sockaddr_in addr = { 0 };
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(kIP.c_str());
		addr.sin_port = htons(kPort);
		if (connect(socket_, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
			int gle_err = WSAGetLastError();
			if (gle_err != WSAEWOULDBLOCK) {
				TraceMsgA("connect failed, GLE: %d\n", WSAGetLastError());
				break;
			}
		}
	} while (false);


	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// (5)
	closesocket(socket_);

	WSACleanup();
	return 0;
}
