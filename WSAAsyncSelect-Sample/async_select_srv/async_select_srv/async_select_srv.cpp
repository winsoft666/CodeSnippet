//async_select_srv.cpp: 定义应用程序的入口点。
//

#include "stdafx.h"
#include "async_select_srv.h"
#include <winsock2.h>
#include <vector>
#include <strsafe.h>

#pragma comment(lib, "Ws2_32.lib")

const u_short kPort = 10001;
const std::string kHelloServer = "hello, I'm server.";

#define WUM_SOCKET (WM_USER+1)

HWND g_hwnd;

SOCKET socket_srv = INVALID_SOCKET;
std::vector<SOCKET> clients;


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
	// (5)
	case WUM_SOCKET:
		if (WSAGETSELECTERROR(lParam)) {
			closesocket((SOCKET)wParam);
			break;
		}

		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_ACCEPT:
			s = accept((SOCKET)wParam, NULL, NULL);

			clients.push_back(s);

			WSAAsyncSelect(s, g_hwnd, WUM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);

			TraceMsgA("new connection\n");

			err = send(s, (const char*)kHelloServer.c_str(), kHelloServer.length(), 0);
			if (err == SOCKET_ERROR) {
				TraceMsgA("send failed, GLE: %d\n", WSAGetLastError());
				break;
			}
			break;
		case FD_READ:
			err = recv((SOCKET)wParam, buf, 100, 0);
			if (err > 0) {
				TraceMsgA("recv: %s\n", buf);
			}
			else if (err == 0) {
				TraceMsgA("connection closed\n");
				closesocket((SOCKET)wParam);
			}
			else {
				TraceMsgA("recv failed, GLE: %d\n", WSAGetLastError());
				closesocket((SOCKET)wParam);
			}
			break;
		case FD_WRITE:
			break;
		case FD_CLOSE:
			closesocket((SOCKET)wParam);

			for (std::vector<SOCKET>::iterator it = clients.begin(); it != clients.end(); it++) {
				if (*it == (SOCKET)wParam) {
					clients.erase(it);
					break;
				}
			}

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

	g_hwnd = CreateWindowW(szWindowClass, L"server", WS_OVERLAPPEDWINDOW,
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


	do
	{
		// (1)
		socket_srv = ::socket(AF_INET, SOCK_STREAM, 0);
		if (socket_srv == INVALID_SOCKET) {
			TraceMsgA("create socket failed, GLE: %d\n", WSAGetLastError());
			break;
		}

		// (2)
		struct sockaddr_in addr = { 0 };
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(kPort);
		if (bind(socket_srv, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
			TraceMsgA("bind failed, GLE: %d\n", WSAGetLastError());
			break;
		}

		// (3)
		WSAAsyncSelect(socket_srv, g_hwnd, WUM_SOCKET, FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE);

		// (4)
		if (listen(socket_srv, 5) == SOCKET_ERROR) {
			TraceMsgA("listen failed, GLE: %d\n", WSAGetLastError());
			break;
		}
		TraceMsgA("listen on port: %d\n", kPort);

	} while (false);


    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
    }

	for (std::vector<SOCKET>::iterator it = clients.begin(); it != clients.end(); it++) {
		closesocket(*it);
	}

	// (6)
	closesocket(socket_srv);

	WSACleanup();
    return 0;
}
