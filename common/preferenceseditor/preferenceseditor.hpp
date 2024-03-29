#pragma once

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/dialog.h>

namespace StudentSync::Common {
	template <typename TPrefs>
	class PreferencesEditor : public wxDialog {
	public:
		PreferencesEditor(wxWindow* parent, wxString title, wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize);

	protected:
		void Initialize(const TPrefs& currentValues);

		virtual wxPanel* CreatePreferencePanel(wxWindow* parent, const TPrefs& currentValues) = 0;

		virtual void OnApply() = 0;
		virtual void OnDiscard() = 0;

	private:
		wxPanel* editorPanel;
		wxStdDialogButtonSizer* confirmationButtonSizer;

		wxButton* okButton;
		void HandleOk(wxCommandEvent& event);

		wxButton* cancelButton;
		void HandleCancel(wxCommandEvent& event);
	};

	template<typename TPrefs>
	inline PreferencesEditor<TPrefs>::PreferencesEditor(wxWindow* parent, wxString title, wxPoint pos, wxSize size)
		: wxDialog(parent, wxID_ANY, title, pos, size)
		, editorPanel{ nullptr }, confirmationButtonSizer{ nullptr }, okButton{ nullptr }, cancelButton{ nullptr }
	{}

	template<typename TPrefs>
	inline void PreferencesEditor<TPrefs>::Initialize(const TPrefs& currentValues)
	{
		wxBoxSizer* layout = new wxBoxSizer(wxVERTICAL);

		editorPanel = this->CreatePreferencePanel(this, currentValues);
		layout->Add(editorPanel, 1, wxALL | wxEXPAND, 10);

		confirmationButtonSizer = new wxStdDialogButtonSizer();

		okButton = new wxButton(this, wxID_OK, "OK");
		okButton->SetWindowStyleFlag(wxALIGN_RIGHT);
		okButton->Bind(wxEVT_BUTTON, &PreferencesEditor::HandleOk, this);
		confirmationButtonSizer->AddButton(okButton);

		cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
		cancelButton->SetWindowStyleFlag(wxALIGN_RIGHT);
		cancelButton->Bind(wxEVT_BUTTON, &PreferencesEditor::HandleCancel, this);
		confirmationButtonSizer->AddButton(cancelButton);

		// arranges buttons in the proper order and fixes tab index
		confirmationButtonSizer->Realize();

		layout->Add(confirmationButtonSizer, 0, wxEXPAND | wxALL, 5);
		this->SetSizerAndFit(layout);
	}

	template<typename TPrefs>
	inline void PreferencesEditor<TPrefs>::HandleOk(wxCommandEvent& event) {
		OnApply();
		event.Skip(); // allow event to continue to bubble
	}

	template<typename TPrefs>
	inline void PreferencesEditor<TPrefs>::HandleCancel(wxCommandEvent& event) {
		event.Skip(); // allow event to continue to bubble
	}
}