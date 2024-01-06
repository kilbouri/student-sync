#pragma once

#include <string>

#include <wx/wx.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>

#include "../preferenceseditor/preferenceseditor.h"
#include "../clientpreferencesmanager/clientpreferencesmanager.h"

class ClientPreferencesEditor : public PreferencesEditor<ClientPreferences>
{
public:
	ClientPreferencesEditor(const ClientPreferences& currentValues, wxWindow* parent, wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize);

protected:
	virtual wxPanel* CreatePreferencePanel(wxWindow* parent, const ClientPreferences& currentValues);

	virtual void OnApply();
	virtual void OnDiscard();

private:
	wxTextCtrl* displayNameField;
	wxSpinCtrl* frameRateField;
	wxComboBox* resolutionField;
};