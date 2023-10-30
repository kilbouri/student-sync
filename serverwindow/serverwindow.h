#include <wx/wx.h>
#include "../server/server.h"

class ServerWindow : public wxFrame
{
public:
	ServerWindow(std::string& hostname, int port);

private:
	Server server;

	void OnExit(wxCommandEvent& event);
	void OnDetails(wxCommandEvent& event);
};

enum {
	ID_Details,
};