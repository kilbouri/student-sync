#include "preferenceseditor.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <ranges>

using namespace StudentSync::Common;

namespace StudentSync::Server {
	PreferencesEditor::PreferencesEditor(const Preferences& currentValues, wxWindow* parent, wxPoint pos, wxSize size)
		: Common::PreferencesEditor<Preferences>(parent, " Preferences", pos, size)
		, maxConcurrentClientsField{ nullptr }, frameRateField{ nullptr }, resolutionField{ nullptr }
	{
		Initialize(currentValues);
	}

	wxPanel* PreferencesEditor::CreatePreferencePanel(wxWindow* parent, const Preferences& currentValues) {
		wxPanel* panel = new wxPanel(parent, wxID_ANY);

		constexpr int NUM_COLUMNS = 1;
		wxFlexGridSizer* flexGrid = new wxFlexGridSizer(0, 2 * NUM_COLUMNS, 5, 10);
		flexGrid->SetFlexibleDirection(wxBOTH);
		flexGrid->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

		// Using this lets us avoid some really repetitive code later on
		auto AddOption = [=](wxString label, wxWindow* control) {
			wxStaticText* labelText = new wxStaticText(panel, wxID_ANY, label);
			labelText->Wrap(-1);
			flexGrid->Add(labelText, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND);
			flexGrid->Add(control, 1, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND);
		};

		maxConcurrentClientsField = new wxSpinCtrl(panel, wxID_ANY);
		maxConcurrentClientsField->SetMin(1);
		maxConcurrentClientsField->SetMax(std::numeric_limits<int>::max());
		maxConcurrentClientsField->SetValue(currentValues.maxConcurrentClients);
		AddOption("Max Concurrent Clients", maxConcurrentClientsField);

		// todo: software-enforce the range, and provide constants to use instead of magics
		frameRateField = new wxSpinCtrl(panel, wxID_ANY);
		frameRateField->SetMin(5);
		frameRateField->SetMax(60);
		frameRateField->SetValue(currentValues.maxFrameRate);
		frameRateField->SetIncrement(5);
		AddOption("Max Stream FPS", frameRateField);

		resolutionField = new wxComboBox(panel, wxID_ANY);
		resolutionField->AppendString(currentValues.maxStreamResolution.ToString());
		resolutionField->SetSelection(0);
		AddOption("Stream Resolution", resolutionField);

		panel->SetSizer(flexGrid);
		return panel;
	}

	void PreferencesEditor::OnApply() {
		PreferencesManager::GetInstance().SetPreferences(
			Preferences{
				.maxConcurrentClients = maxConcurrentClientsField->GetValue(),
				.maxFrameRate = frameRateField->GetValue(),
				.maxStreamResolution = ScreenResolution::Parse(resolutionField->GetStringSelection())
			}
		);
	}

	void PreferencesEditor::OnDiscard() {
		/* no operation, since we only ever export the panel's values in OnApply */
	}
}
