#include <wx/wx.h>

class ServerWindow : public wxFrame
{
public:
	ServerWindow();

private:
	void OnExit(wxCommandEvent& event);
	void OnDetails(wxCommandEvent& event);
};

enum {
	ID_Details,
};