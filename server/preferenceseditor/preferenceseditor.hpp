#pragma once

#include <string>

#include <wx/wx.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>

#include "../preferencesmanager/preferencesmanager.hpp"
#include "../../common/preferenceseditor/preferenceseditor.hpp"

namespace StudentSync::Server {
	struct PreferencesEditor : public Common::PreferencesEditor<Preferences> {

		PreferencesEditor(const Preferences& currentValues, wxWindow* parent, wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize);

	protected:
		virtual wxPanel* CreatePreferencePanel(wxWindow* parent, const Preferences& currentValues);

		virtual void OnApply();
		virtual void OnDiscard();

	private:
		wxSpinCtrl* maxConcurrentClientsField;
		wxSpinCtrl* frameRateField;
		wxComboBox* resolutionField;
	};
}