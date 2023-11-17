#include <wx/numdlg.h>

#include "win32includes.h"
#include "modeselectdialog/modeselectdialog.h"
#include "clientwindow/clientwindow.h"
#include "serverwindow/serverwindow.h"

constexpr int DEFAULT_PORT_NUMBER = 1000;

class App : public wxApp {
public:
	bool OnInit() override;
	int OnExit() override;

private:
	ULONG_PTR gdiPlusToken;
};

typedef struct {
	std::string name;
	std::wstring friendlyName;
	std::vector<std::string> ipAddresses;
} SystemInterface;
std::optional<std::vector<SystemInterface>> GetNetworkAdapters();

std::optional<wxFrame*> DoClientStartup();
std::optional<wxFrame*> DoServerStartup();

bool App::OnInit() {
	// Initialize GDI+
	if (GdiPlusStartup(&gdiPlusToken) != 0) {
		wxLogFatalError("GDI+ startup failed");
		return false;
	}

	// Initialize WinSock2
	int winsockStartupCode = Winsock2Startup();
	if (winsockStartupCode == 1) {
		// we have to perform shutdown BEFORE we log fatal error, as wxLogFatalError does not return
		GdiPlusShutdown(gdiPlusToken);
		wxLogFatalError("Winsock2 version != 2.2");
		return false;
	}
	else if (winsockStartupCode != 0) {
		// we have to perform shutdown BEFORE we log fatal error, as wxLogFatalError does not return
		GdiPlusShutdown(gdiPlusToken);
		wxLogFatalError("Winsock2 startup failed");
		return false;
	}

	// Fire up the wxWidgets image handler(s)
	wxImage::AddHandler(new wxPNGHandler());

	// Determine main frame for this instance
	std::optional<wxFrame*> mainFrame = std::nullopt;

	ModeSelectDialog modeSelectDialog(nullptr);
	if (modeSelectDialog.ShowModal() != wxID_OK) {
		GdiPlusShutdown(gdiPlusToken);
		return false;
	}

	switch (modeSelectDialog.GetValue()) {
		case ModeSelectDialog::Result::Client: mainFrame = DoClientStartup(); break;
		case ModeSelectDialog::Result::Server: mainFrame = DoServerStartup(); break;
		default: mainFrame = std::nullopt; break;
	}

	if (!mainFrame) {
		Winsock2Shutdown();
		GdiPlusShutdown(gdiPlusToken);
		return false;
	}

	(*mainFrame)->Show();
	return true;
}

int App::OnExit() {
	// if you ever set breakpoints in this function, please be aware that
	// they may not hit when you use the Server mode... I have no idea why.
	// But if you add wxLogError("HIT!"); you will get a popup when you close.
	Winsock2Shutdown();
	GdiPlusShutdown(gdiPlusToken);

	return wxApp::OnExit();
}

wxIMPLEMENT_APP(App);

std::optional<wxFrame*> DoClientStartup() {
	wxTextEntryDialog addressDialog(nullptr, "Enter server address:", "Server Address", "localhost");
	if (addressDialog.ShowModal() != wxID_OK) {
		return std::nullopt;
	}

	wxNumberEntryDialog portDialog(nullptr, "Enter server port:", wxEmptyString, "Server Port", DEFAULT_PORT_NUMBER, 0, LONG_MAX);
	if (portDialog.ShowModal() != wxID_OK) {
		return std::nullopt;
	}

	return new ClientWindow("StudentSync - Client", addressDialog.GetValue().ToStdString(), portDialog.GetValue());
}

std::optional<wxFrame*> DoServerStartup()
{
	wxString title = "Select Network Interface";
	wxString prompt = "Select the network interface to bind to:";

	auto adapters = GetNetworkAdapters();
	if (!adapters) {
		wxMessageBox("Failed to enumerate network adapters.", "Server startup failed", wxICON_ERROR);
		return std::nullopt;
	}

	wxArrayString options;
	std::unordered_map<wxString, std::string> optionToAddress;
	auto makeLabel = [](std::wstring friendly, std::string ip) { return wxString(ip + " (" + friendly + ")"); };

	// add "All Interfaces" as the first option, since its likely to be the user preference
	std::string allInterfacesIpAddress = "0.0.0.0";
	wxString allInterfacesLabel = makeLabel(L"All Interfaces", allInterfacesIpAddress);

	options.push_back(allInterfacesLabel);
	optionToAddress.insert_or_assign(allInterfacesLabel, allInterfacesIpAddress);

	for (auto& adapter : *adapters) {
		for (auto& address : adapter.ipAddresses) {
			wxString label = makeLabel(adapter.friendlyName, address);
			options.push_back(label);
			optionToAddress.insert_or_assign(label, address);
		}
	}

	wxSingleChoiceDialog interfaceDialog(nullptr, prompt, title, options);
	interfaceDialog.SetSelection(0); // ensure 0.0.0.0 is default-selected

	if (interfaceDialog.ShowModal() != wxID_OK) {
		return std::nullopt;
	}

	wxNumberEntryDialog portDialog(nullptr, "Enter server port:", wxEmptyString, "Server Port", DEFAULT_PORT_NUMBER, 0, INT_MAX);
	if (portDialog.ShowModal() != wxID_OK) {
		return std::nullopt;
	}

	std::string address = optionToAddress.at(interfaceDialog.GetStringSelection());
	int port = static_cast<int>(portDialog.GetValue());

	return new ServerWindow("StudentSync - Server", address, port);
}

// Retrieves information about all IPv4-compatible interfaces on the host.
std::optional<std::vector<SystemInterface>> GetNetworkAdapters() {
	ULONG family = AF_INET;
	PIP_ADAPTER_ADDRESSES adapters = nullptr;
	ULONG bufferSize = 0;

	int flags = GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST;
	if (GetAdaptersAddresses(family, flags, NULL, adapters, &bufferSize) == ERROR_BUFFER_OVERFLOW) {
		adapters = (PIP_ADAPTER_ADDRESSES)std::malloc(bufferSize);

		if (adapters == nullptr) {
			return std::nullopt;
		}

		if (GetAdaptersAddresses(family, flags, nullptr, adapters, &bufferSize) != NOERROR) {
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

		if (addresses.size() == 0) {
			// ignore adapters with no assigned IP address
			continue;
		}

		thisInterface.name = std::string(currentAdapter->AdapterName);
		thisInterface.friendlyName = std::wstring(currentAdapter->FriendlyName);
		thisInterface.ipAddresses = addresses;
		result.push_back(thisInterface);
	}

	std::free(adapters);
	return result;
}