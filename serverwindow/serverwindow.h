#include <wx/wx.h>

class ServerWindow : public wxFrame
{
public:
	ServerWindow(std::string_view hostname, int port);

private:
	void OnExit(wxCommandEvent& event);
	void OnDetails(wxCommandEvent& event);
};

enum {
	ID_Details,
};