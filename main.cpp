#include "win32includes.h"
#include "server/server.h"
#include "client/client.h"

constexpr auto MODE_SERVER = 1;
constexpr auto MODE_CLIENT = 2;

int main()
{
	WORD winsockVersion = MAKEWORD(2, 2); // version 2.2
	WSADATA winsockData;

	int socketStartupCode = WSAStartup(winsockVersion, &winsockData);

	if (socketStartupCode != 0) {
		std::cerr << "Winsock2 Starup failed.\n";
		return 1;
	}
	else if (winsockData.wVersion != winsockVersion) {
		std::cerr << "Winsock2 version != 2.2\n";
		WSACleanup();
		return 1;
	}
	else {
		std::cout << "Winsock2 Startup succeeded.\n";
	}

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
			WSACleanup();
			return 1;
		}

		std::cout << "Starting server on port " << port << "\n";
		server.Start();
		WSACleanup();
		return 1;
	}
	else if (mode == MODE_CLIENT) {
		int serverPort;
		std::cout << "What port is the server listening on? ";
		std::cin >> serverPort;

		Client client{ "localhost", serverPort };

		if (client.Initialize() != 0) {
			std::cerr << "Failed to start client\n";
			WSACleanup();
			return 1;
		}

		client.Connect();
		WSACleanup();
		return 0;
	}
	else {
		std::cout << "Invalid selection.\n";
		WSACleanup();
		return 1;
	}
}