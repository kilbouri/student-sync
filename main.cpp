#include "win32includes.h"
#include "server/server.h"
#include "client/client.h"
#include <optional>
#include <iomanip>

constexpr auto MODE_SERVER = 1;
constexpr auto MODE_CLIENT = 2;

int Winsock2Startup();
void Winsock2Shutdown();
int GdiPlusStartup(ULONG_PTR* tokenOut);
void GdiPlusShutdown(ULONG_PTR token);

typedef struct {
	std::string name;
	std::wstring friendlyName;
	std::vector<std::string> ipAddresses;
} SystemInterface;

std::optional<std::vector<SystemInterface>> GetInterfaces();

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

	// Initialize WinSock2
	int winsockStartupCode = Winsock2Startup();
	if (winsockStartupCode == 0) {
		std::cout << "Winsock2 startup succeeded\n";
	}
	else if (winsockStartupCode == 1) {
		std::cerr << "Winsock2 version != 2.2\n";
		GdiPlusShutdown(gdiPlusToken);
		return 1;
	}
	else {
		std::cerr << "Winsock2 startup failed\n";
		GdiPlusShutdown(gdiPlusToken);
		return 1;
	}

	auto Quit = [gdiPlusToken](int exitCode) {
		Winsock2Shutdown();
		GdiPlusShutdown(gdiPlusToken);
		return exitCode;
	};

	std::cout << "\n--- BEGIN INTERFACE LIST ---\n";
	auto interfaces = GetInterfaces();
	if (interfaces) {
		size_t maxInterfaceNameLength = 0;
		for (auto& $interface : *interfaces) {
			maxInterfaceNameLength = std::max(maxInterfaceNameLength, $interface.friendlyName.length());
		}

		for (auto& $interface : *interfaces) {
			int addrCount = $interface.ipAddresses.size();

			std::wcout << std::setw(maxInterfaceNameLength) << $interface.friendlyName;
			std::cout << ": ";

			std::string_view sep = "";
			for (auto& address : $interface.ipAddresses) {
				std::cout << sep << address;
				sep = ", ";
			}
			std::cout << "\n";
		}
	}
	std::cout << "--- END INTERFACE LIST ---\n\n";

	std::cout << "Should I be a client, or a server?\n" << MODE_SERVER << ": server\n" << MODE_CLIENT << ": client\n";

	int mode;
	std::cin >> mode;

	if (mode == MODE_SERVER) {
		int port;
		std::cout << "On which port shall I listen? ";
		std::cin >> port;

		int localhostOnly;
		std::cout << "Local host only? (yes = 1, no = 0)";
		std::cin >> localhostOnly;

		Server server(port, !localhostOnly);

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

std::optional<std::vector<SystemInterface>> GetInterfaces() {
	ULONG family = AF_INET;
	PIP_ADAPTER_ADDRESSES adapters = nullptr;
	ULONG bufferSize = 0;

	int flags = GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST;
	if (GetAdaptersAddresses(family, flags, NULL, adapters, &bufferSize) == ERROR_BUFFER_OVERFLOW) {
		adapters = (PIP_ADAPTER_ADDRESSES)std::malloc(bufferSize);

		if (adapters == nullptr) {
			return std::nullopt;
		}

		if (GetAdaptersAddresses(family, flags, NULL, adapters, &bufferSize) != NOERROR) {
			std::free(adapters);
			return std::nullopt;
		}
	}

	std::vector<SystemInterface> result;
	for (PIP_ADAPTER_ADDRESSES currentAdapter = adapters; currentAdapter != nullptr; currentAdapter = currentAdapter->Next) {
		SystemInterface thisInterface{};

		if (!currentAdapter->FirstUnicastAddress) {
			continue;
		}

		std::vector<std::string> addresses;
		for (PIP_ADAPTER_UNICAST_ADDRESS unicastAddr = currentAdapter->FirstUnicastAddress; unicastAddr != nullptr; unicastAddr = unicastAddr->Next) {
			struct sockaddr* socketAddress = unicastAddr->Address.lpSockaddr;
			if (socketAddress->sa_family != AF_INET) {
				continue; // not IPv4, skip
			}

			struct sockaddr_in* ipv4 = (struct sockaddr_in*)socketAddress;
			char ipString[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(ipv4->sin_addr), ipString, sizeof(ipString));

			addresses.push_back(std::string(ipString));
		}

		thisInterface.name = std::string(currentAdapter->AdapterName);
		thisInterface.friendlyName = std::wstring(currentAdapter->FriendlyName);
		thisInterface.ipAddresses = addresses;
		result.push_back(thisInterface);
	}

	return result;
}