#include "win32includes.h"
#include "server/server.h"
#include "client/client.h"
#include "GUI/GUI.h"
#include <optional>
#include <iomanip>
#include <unordered_map>
#include <string>

wxIMPLEMENT_APP(MyApp);

//constexpr auto MODE_SERVER = 1;
//constexpr auto MODE_CLIENT = 2;
//constexpr auto INVALID_TRY_AGAIN = "Invalid, try again: ";
//
//int DoRunServer();
//int DoRunClient();
//
//typedef struct {
//	std::string name;
//	std::wstring friendlyName;
//	std::vector<std::string> ipAddresses;
//} SystemInterface;
//std::optional<std::vector<SystemInterface>> GetNetworkAdapters();
//
//int main() {
//	// Initialize GDI+
//	ULONG_PTR gdiPlusToken;
//	if (GdiPlusStartup(&gdiPlusToken) != 0) {
//		std::cerr << "GDI+ startup failed\n";
//		return 1;
//	}
//	else {
//		std::cout << "GDI+ startup succeeded\n";
//	}
//
//	// Initialize WinSock2
//	int winsockStartupCode = Winsock2Startup();
//	if (winsockStartupCode == 0) {
//		std::cout << "Winsock2 startup succeeded\n";
//	}
//	else if (winsockStartupCode == 1) {
//		std::cerr << "Winsock2 version != 2.2\n";
//		GdiPlusShutdown(gdiPlusToken);
//		return 1;
//	}
//	else {
//		std::cerr << "Winsock2 startup failed\n";
//		GdiPlusShutdown(gdiPlusToken);
//		return 1;
//	}
//
//	// A closure that performs pre-exit cleanup. Any exiting path should call this.
//	// TODO: does a better way exist?
//	auto Quit = [gdiPlusToken](int exitCode) {
//		Winsock2Shutdown();
//		GdiPlusShutdown(gdiPlusToken);
//		return exitCode; // allows return chaining for pretty syntax :)
//	};
//
//	int mode;
//	std::cout << "Client or server? (server = " << MODE_SERVER << ", client = " << MODE_CLIENT << "): ";
//	while (!(std::cin >> mode) || !(mode == MODE_SERVER || mode == MODE_CLIENT)) {
//		std::cout << INVALID_TRY_AGAIN;
//	}
//
//	if (mode == MODE_SERVER) {
//		return Quit(DoRunServer());
//	}
//	else if (mode == MODE_CLIENT) {
//		return Quit(DoRunClient());
//	}
//	else {
//		// should never happen, but  ＼（〇_ｏ）／
//		std::cout << "Invalid choice. Somehow.\n";
//		return Quit(1);
//	}
//}
//
//int DoRunServer() {
//	auto getInterfaceResult = GetNetworkAdapters();
//	if (!getInterfaceResult) {
//		std::cerr << "Failed to enumerate network adapters\n";
//		return 1;
//	}
//
//	auto& adapters = *getInterfaceResult;
//
//	SystemInterface allInterfaces{
//		.name = "all interfaces",
//		.friendlyName = L"all interfaces",
//		.ipAddresses = std::vector<std::string>{"0.0.0.0"}
//	};
//
//	adapters.push_back(allInterfaces);
//
//	std::cout << "Which network adapter should the server bind to?\n";
//	std::unordered_map<int, std::string&> indexToAddress{};
//
//	int addressOptionNumber = 1;
//	for (auto& adapter : adapters) {
//		for (auto& address : adapter.ipAddresses) {
//			indexToAddress.insert(std::pair<const int, std::string&>(addressOptionNumber, address));
//
//			std::cout << addressOptionNumber << ": " << address << " (";
//			std::wcout << adapter.friendlyName;
//			std::cout << ")\n";
//
//			addressOptionNumber++;
//		}
//	}
//
//
//	int addressChoiceNumber;
//	while (!(std::cin >> addressChoiceNumber) || !(1 <= addressChoiceNumber && addressChoiceNumber <= addressOptionNumber)) {
//		std::cout << INVALID_TRY_AGAIN;
//	}
//
//	std::string& chosenAddress = indexToAddress.at(addressChoiceNumber);
//
//	int port;
//	std::cout << "What port should the server use? (0 = any) ";
//	while (!(std::cin >> port)) {
//		std::cout << INVALID_TRY_AGAIN;
//	}
//
//	Server server(chosenAddress, port);
//
//	if (server.Initialize() != 0) {
//		int errorCode = GetLastError();
//		if (errorCode == WSAEADDRNOTAVAIL) {
//			std::cerr << "Unable to bind to requested interface/port\n";
//		}
//		else {
//			std::cerr << "Failed to start server with code " << GetLastError() << "\n";
//		}
//		return 1;
//	}
//
//	std::cout << "Starting server at " << server.GetExternalAddress() << ":" << server.GetPortNumber() << "\n";
//	server.Start();
//
//	return 1; // we should never get here, as Start is non-returning
//}
//
//int DoRunClient() {
//	std::cout << "What is the server address? ";
//	std::string serverAddress;
//	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//	std::getline(std::cin, serverAddress);
//
//	std::cout << "What is the server port? ";
//	int portNumber;
//	while (!(std::cin >> portNumber) || portNumber == 0) {
//		std::cout << INVALID_TRY_AGAIN;
//	}
//
//	Client client(serverAddress, portNumber);
//	if (client.Initialize() != 0) {
//		std::cerr << "Failed to start client\n";
//		return 1;
//	}
//
//	client.Connect();
//	return 0;
//}
//
//// Retrieves information about all IPv4-compatible interfaces on the host.
//std::optional<std::vector<SystemInterface>> GetNetworkAdapters() {
//	ULONG family = AF_INET;
//	PIP_ADAPTER_ADDRESSES adapters = nullptr;
//	ULONG bufferSize = 0;
//
//	int flags = GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST;
//	if (GetAdaptersAddresses(family, flags, NULL, adapters, &bufferSize) == ERROR_BUFFER_OVERFLOW) {
//		adapters = (PIP_ADAPTER_ADDRESSES)std::malloc(bufferSize);
//
//		if (adapters == nullptr) {
//			return std::nullopt;
//		}
//
//		if (GetAdaptersAddresses(family, flags, nullptr, adapters, &bufferSize) != NOERROR) {
//			std::free(adapters);
//			return std::nullopt;
//		}
//	}
//
//	std::vector<SystemInterface> result;
//	for (PIP_ADAPTER_ADDRESSES currentAdapter = adapters; currentAdapter != nullptr; currentAdapter = currentAdapter->Next) {
//		SystemInterface thisInterface{};
//
//		if (!currentAdapter->FirstUnicastAddress) {
//			continue;
//		}
//
//		std::vector<std::string> addresses;
//		for (PIP_ADAPTER_UNICAST_ADDRESS unicastAddr = currentAdapter->FirstUnicastAddress; unicastAddr != nullptr; unicastAddr = unicastAddr->Next) {
//			struct sockaddr* socketAddress = unicastAddr->Address.lpSockaddr;
//			if (socketAddress->sa_family != AF_INET) {
//				continue; // not IPv4, skip
//			}
//
//			struct sockaddr_in* ipv4 = (struct sockaddr_in*)socketAddress;
//			char ipString[INET_ADDRSTRLEN];
//			inet_ntop(AF_INET, &(ipv4->sin_addr), ipString, sizeof(ipString));
//
//			addresses.push_back(std::string(ipString));
//		}
//
//		if (addresses.size() == 0) {
//			// ignore adapters with no assigned IP address
//			continue;
//		}
//
//		thisInterface.name = std::string(currentAdapter->AdapterName);
//		thisInterface.friendlyName = std::wstring(currentAdapter->FriendlyName);
//		thisInterface.ipAddresses = addresses;
//		result.push_back(thisInterface);
//	}
//
//	std::free(adapters);
//	return result;
//}