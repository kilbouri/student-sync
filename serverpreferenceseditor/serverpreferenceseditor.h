#pragma once

#include <string>

#include <wx/wx.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>

#include "../common/preferenceseditor/preferenceseditor.h"
#include "../serverpreferencesmanager/serverpreferencesmanager.h"

class ServerPreferencesEditor : public PreferencesEditor<ServerPreferences>
{
public:
	ServerPreferencesEditor(const ServerPreferences& currentValues, wxWindow* parent, wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize);

protected:
	virtual wxPanel* CreatePreferencePanel(wxWindow* parent, const ServerPreferences& currentValues);

	virtual void OnApply();
	virtual void OnDiscard();

private:
	wxSpinCtrl* maxConcurrentClientsField;
	wxSpinCtrl* frameRateField;
	wxComboBox* resolutionField;
};