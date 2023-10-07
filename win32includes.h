#pragma once

#pragma comment(lib, "Ws2_32.lib") // this indicates the WinSock2 DLL is needed

#define NOMINMAX // disable Min/Max macros in minwindef.h (included transitively below)

// WinSock2 must be first, to prevent the inclusion of
// WinSock1. Both cannot coexist.
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>