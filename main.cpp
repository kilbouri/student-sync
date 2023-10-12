#include "win32includes.h"
#include "server/server.h"
#include "client/client.h"

constexpr auto MODE_SERVER = 1;
constexpr auto MODE_CLIENT = 2;

int Winsock2Startup();
void Winsock2Shutdown();
int InitializeCOM();
void UninitializeCOM();
int GdiPlusStartup(ULONG_PTR* tokenOut);
void GdiPlusShutdown(ULONG_PTR token);

int main() {
	// Initialize GDI+
	ULONG_PTR gdiPlusToken;
	if (GdiPlusStartup(&gdiPlusToken) != 0) {
		std::cerr << "GDI+ startup failed\n";
		return 1;
	}
	else {
		std::cout << "GDI+ startup succeeded\n";
	}

	if (!InitializeCOM()) {
		std::cerr << "Failed to start COM Library\n";
		GdiPlusShutdown(gdiPlusToken);
		return 1;
	}
	else {
		std::cout << "COM library initialized\n";
	}

	// Initialize WinSock2
	int winsockStartupCode = Winsock2Startup();
	if (winsockStartupCode == 0) {
		std::cout << "Winsock2 startup succeeded\n";
	}
	else if (winsockStartupCode == 1) {
		std::cerr << "Winsock2 version != 2.2\n";
		GdiPlusShutdown(gdiPlusToken);
		UninitializeCOM();
		return 1;
	}
	else {
		std::cerr << "Winsock2 startup failed\n";
		GdiPlusShutdown(gdiPlusToken);
		UninitializeCOM();
		return 1;
	}

	auto Quit = [gdiPlusToken](int exitCode) {
		Winsock2Shutdown();
		GdiPlusShutdown(gdiPlusToken);
		return exitCode;
	};

	std::cout << "Should I be a client, or a server?\n" << MODE_SERVER << ": server\n" << MODE_CLIENT << ": client\n";

	int mode;
	std::cin >> mode;

	if (mode == MODE_SERVER) {
		int port;
		std::cout << "On which port shall I listen? ";
		std::cin >> port;

		Server server{ port };

		if (server.Initialize() != 0) {
			std::cerr << "Failed to start server\n";
			return Quit(1);
		}

		std::cout << "Starting server on port " << port << "\n";
		server.Start();
		return Quit(1);
	}
	else if (mode == MODE_CLIENT) {
		int serverPort;
		std::cout << "What port is the server listening on? ";
		std::cin >> serverPort;

		Client client{ "localhost", serverPort };

		if (client.Initialize() != 0) {
			std::cerr << "Failed to start client\n";
			return Quit(1);
		}

		client.Connect();
		return Quit(0);
	}
	else {
		std::cout << "Invalid selection.\n";
		return Quit(1);
	}
}

int Winsock2Startup() {
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

void Winsock2Shutdown() {
	WSACleanup();
}

int InitializeCOM() {
	return SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED));
}

void UninitializeCOM() {
	CoUninitialize();
}

int GdiPlusStartup(ULONG_PTR* tokenOut) {
	if (tokenOut == NULL) {
		throw "Null tokenOut pointer received in GdiPlusStartup";
	}

	Gdiplus::GdiplusStartupInput startupInfo;
	Gdiplus::Status startStatus = Gdiplus::GdiplusStartup(tokenOut, &startupInfo, NULL);

	if (startStatus != Gdiplus::Status::Ok) {
		return 1;
	}

	// this is important for image capture to obtain the entire screen
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	return 0;
}

void GdiPlusShutdown(ULONG_PTR token) {
	Gdiplus::GdiplusShutdown(token);
}