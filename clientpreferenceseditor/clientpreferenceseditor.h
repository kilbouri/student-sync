#pragma once

#include <string>

#include <wx/wx.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>

#include "../preferenceseditor/preferenceseditor.h"

struct ClientPreferences {
	std::string username;
	std::string serverIp;
	int serverPort;
};

class ClientPreferencesEditor : public PreferencesEditor<ClientPreferences>
{
public:
	ClientPreferencesEditor(const ClientPreferences& currentValues, wxWindow* parent, wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize);

protected:
	virtual wxPanel* CreatePreferencePanel(wxWindow* parent, const ClientPreferences& currentValues);

	virtual void OnApply() {};
	virtual void OnDiscard() {};

private:
	wxFlexGridSizer* fgSizer2;
	wxStaticText* m_staticText8;
	wxComboBox* m_comboBox18;
	wxStaticText* m_staticText9;
	wxComboBox* m_comboBox19;
	wxStaticText* m_staticText10;
	wxComboBox* m_comboBox20;
	wxStaticText* m_staticText11;
	wxComboBox* m_comboBox21;
	wxStaticText* m_staticText12;
	wxComboBox* m_comboBox22;
	wxStaticText* m_staticText13;
	wxComboBox* m_comboBox23;
	wxStaticText* m_staticText15;
	wxTextCtrl* m_textCtrl3;
	wxStaticText* m_staticText16;
	wxTextCtrl* m_textCtrl31;
	wxStaticText* m_staticText17;
	wxTextCtrl* m_textCtrl32;
	wxStaticText* m_staticText18;
	wxTextCtrl* m_textCtrl33;
	wxStaticText* m_staticText19;
	wxSpinCtrlDouble* m_spinCtrlDouble1;
	wxStaticText* m_staticText21;
	wxSpinCtrlDouble* m_spinCtrlDouble2;
	wxStaticText* m_staticText28;
	wxSpinCtrlDouble* m_spinCtrlDouble3;
};