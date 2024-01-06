#pragma once

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/dialog.h>

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

	wxButton* applyButton;
	wxButton* okButton;
	wxButton* cancelButton;
};

template<typename TPrefs>
inline PreferencesEditor<TPrefs>::PreferencesEditor(wxWindow* parent, wxString title, wxPoint pos, wxSize size)
	: wxDialog(parent, wxID_ANY, title, pos, size), editorPanel{ nullptr }, confirmationButtonSizer{ nullptr }, applyButton{ nullptr }, okButton{ nullptr }, cancelButton{ nullptr }
{}

template<typename TPrefs>
inline void PreferencesEditor<TPrefs>::Initialize(const TPrefs& currentValues)
{
	wxBoxSizer* layout = new wxBoxSizer(wxVERTICAL);

	editorPanel = this->CreatePreferencePanel(this, currentValues);
	layout->Add(editorPanel);

	confirmationButtonSizer = new wxStdDialogButtonSizer();

	applyButton = new wxButton(this, wxID_APPLY, "Apply");
	confirmationButtonSizer->AddButton(applyButton);

	okButton = new wxButton(this, wxID_OK, "OK");
	okButton->SetWindowStyleFlag(wxALIGN_RIGHT);
	confirmationButtonSizer->AddButton(okButton);

	cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
	cancelButton->SetWindowStyleFlag(wxALIGN_RIGHT);
	confirmationButtonSizer->AddButton(cancelButton);

	// arranges buttons in the proper order and fixes tab index
	confirmationButtonSizer->Realize();

	layout->Add(confirmationButtonSizer, 0, wxEXPAND | wxBOTTOM, 5);
	this->SetSizerAndFit(layout);
}