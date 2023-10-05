#pragma once

#include <iostream>

// This must be defined here, before the inclusion of WS2tcpip.h as WS2tcpip.h
// defines a max() macro which conflicts.
constexpr auto MAX_STREAM_SIZE = std::numeric_limits<std::streamsize>::max();

// windows headers
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib") // this indicates the WinSock2 DLL is needed. Do not remove.
