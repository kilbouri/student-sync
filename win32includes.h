#pragma once

// Shut the FUCK up Windows!
#define _CRT_SECURE_NO_WARNINGS 1

#pragma comment(lib, "Ws2_32.lib") // this indicates the WinSock2 DLL is needed
#pragma comment(lib, "gdiplus.lib") // this indicates the GDI+ DLL is needed
#pragma comment(lib, "IPHLPAPI.lib") // this indicates the IP Helper API DLL is needed

#define NOMINMAX // disable Min/Max macros in minwindef.h (included transitively below)

// WinSock2 must be first, to prevent the inclusion of
// WinSock1. Both cannot coexist.
#include <WinSock2.h>
#include <iphlpapi.h> // this must be after WinSock2.h
#include <WS2tcpip.h>
#include <mstcpip.h>
#include <Windows.h>
#include <gdiplus.h>
#include <KnownFolders.h>
#include <ShlObj.h>

#pragma region Library Helpers
static inline int Winsock2Startup() {
	WORD winsockVersion = MAKEWORD(2, 2); // version 2.2
	WSADATA winsockData;

	int socketStartupCode = WSAStartup(winsockVersion, &winsockData);
	if (socketStartupCode != 0) {
		return -1; // start failed
	}

	if (winsockData.wVersion != winsockVersion) {
		return 1; // bad version
	}

	return 0;
}

static inline void Winsock2Shutdown() {
	WSACleanup();
}

static inline int GdiPlusStartup(ULONG_PTR* tokenOut) {
	if (tokenOut == nullptr) {
		throw "Null tokenOut pointer received in GdiPlusStartup";
	}

	Gdiplus::GdiplusStartupInput startupInfo;
	Gdiplus::Status startStatus = Gdiplus::GdiplusStartup(tokenOut, &startupInfo, nullptr);

	if (startStatus != Gdiplus::Status::Ok) {
		return 1;
	}

	// this is important for image capture to obtain the entire screen
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	return 0;
}

static inline void GdiPlusShutdown(ULONG_PTR token) {
	Gdiplus::GdiplusShutdown(token);
}
#pragma endregion