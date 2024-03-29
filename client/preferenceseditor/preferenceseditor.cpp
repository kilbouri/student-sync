#include "preferenceseditor.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <ranges>

#include "../../common/screenresolution/screenresolution.hpp"

namespace StudentSync::Client {
	PreferencesEditor::PreferencesEditor(const Preferences& currentValues, wxWindow* parent, wxPoint pos, wxSize size)
		: Common::PreferencesEditor<Preferences>(parent, "Client Preferences", pos, size)
		, displayNameField{ nullptr }, frameRateField{ nullptr }, resolutionField{ nullptr }
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

		displayNameField = new wxTextCtrl(panel, wxID_ANY, currentValues.displayName);
		AddOption("Display Name", displayNameField);

		// todo: software-enforce the range, and provide constants to use instead of magics
		frameRateField = new wxSpinCtrl(panel, wxID_ANY);
		frameRateField->SetMin(5);
		frameRateField->SetValue(currentValues.maxFrameRate);
		frameRateField->SetMax(60);
		frameRateField->SetIncrement(5);
		AddOption("Max Stream FPS", frameRateField);

		// todo: a way to detect user screen resolution and generate resolution options
		resolutionField = new wxComboBox(panel, wxID_ANY);
		resolutionField->AppendString("1920x1080 (Native)");
		resolutionField->SetSelection(0);
		resolutionField->AppendString("1280x720 (1/3)");
		resolutionField->AppendString("640x360 (1/6)");
		AddOption("Stream Resolution", resolutionField);

		panel->SetSizer(flexGrid);
		return panel;
	}

	void PreferencesEditor::OnApply() {
		std::string displayName = displayNameField->GetValue();
		long frameRate = frameRateField->GetValue();

		std::string resolution = resolutionField->GetStringSelection();

		size_t delimPosition = resolution.rfind('x');
		if (delimPosition == std::string::npos) {
			throw "Invalid resolution selected";
		}

		std::string left = resolution.substr(0, delimPosition);
		std::string right = resolution.substr(delimPosition + 1);

		unsigned width = std::stoul(left);
		unsigned height = std::stoul(right);

		PreferencesManager::GetInstance().SetPreferences(
			Preferences{
				.displayName = displayName,
				.maxFrameRate = frameRate,
				.maxStreamResolution = Common::ScreenResolution{width, height}
			}
		);
	}

	void PreferencesEditor::OnDiscard() {
		/* no operation, since we only ever export the panel's values in OnApply */
	}
}