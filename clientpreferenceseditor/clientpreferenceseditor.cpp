#include "clientpreferenceseditor.h"

ClientPreferencesEditor::ClientPreferencesEditor(const ClientPreferences& currentValues, wxWindow* parent, wxPoint pos, wxSize size)
	: PreferencesEditor<ClientPreferences>(parent, "Client Preferences", pos, size)
{
	Initialize(currentValues);
}

wxPanel* ClientPreferencesEditor::CreatePreferencePanel(wxWindow* parent, const ClientPreferences& currentValues) {
	wxPanel* panel = new wxPanel(parent, wxID_ANY);

	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
	fgSizer2->SetFlexibleDirection(wxBOTH);
	fgSizer2->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_staticText8 = new wxStaticText(panel, wxID_ANY, wxT("Pick Selection"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText8->Wrap(-1);
	fgSizer2->Add(m_staticText8, 0, wxALL, 5);

	m_comboBox18 = new wxComboBox(panel, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	fgSizer2->Add(m_comboBox18, 0, wxALL, 5);

	m_staticText9 = new wxStaticText(panel, wxID_ANY, wxT("Pick Selection"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText9->Wrap(-1);
	fgSizer2->Add(m_staticText9, 0, wxALL, 5);

	m_comboBox19 = new wxComboBox(panel, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	fgSizer2->Add(m_comboBox19, 0, wxALL, 5);

	m_staticText10 = new wxStaticText(panel, wxID_ANY, wxT("Pick Selection"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText10->Wrap(-1);
	fgSizer2->Add(m_staticText10, 0, wxALL, 5);

	m_comboBox20 = new wxComboBox(panel, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	fgSizer2->Add(m_comboBox20, 0, wxALL, 5);

	m_staticText11 = new wxStaticText(panel, wxID_ANY, wxT("Pick Selection"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText11->Wrap(-1);
	fgSizer2->Add(m_staticText11, 0, wxALL, 5);

	m_comboBox21 = new wxComboBox(panel, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	fgSizer2->Add(m_comboBox21, 0, wxALL, 5);

	m_staticText12 = new wxStaticText(panel, wxID_ANY, wxT("Pick Selection"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText12->Wrap(-1);
	fgSizer2->Add(m_staticText12, 0, wxALL, 5);

	m_comboBox22 = new wxComboBox(panel, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	fgSizer2->Add(m_comboBox22, 0, wxALL, 5);

	m_staticText13 = new wxStaticText(panel, wxID_ANY, wxT("Pick Selection"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText13->Wrap(-1);
	fgSizer2->Add(m_staticText13, 0, wxALL, 5);

	m_comboBox23 = new wxComboBox(panel, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	fgSizer2->Add(m_comboBox23, 0, wxALL, 5);

	m_staticText15 = new wxStaticText(panel, wxID_ANY, wxT("Type String"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText15->Wrap(-1);
	fgSizer2->Add(m_staticText15, 0, wxALL, 5);

	m_textCtrl3 = new wxTextCtrl(panel, wxID_ANY, wxT("Original Value"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer2->Add(m_textCtrl3, 0, wxALL, 5);

	m_staticText16 = new wxStaticText(panel, wxID_ANY, wxT("Type String"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText16->Wrap(-1);
	fgSizer2->Add(m_staticText16, 0, wxALL, 5);

	m_textCtrl31 = new wxTextCtrl(panel, wxID_ANY, wxT("Original Value"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer2->Add(m_textCtrl31, 0, wxALL, 5);

	m_staticText17 = new wxStaticText(panel, wxID_ANY, wxT("Type String"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText17->Wrap(-1);
	fgSizer2->Add(m_staticText17, 0, wxALL, 5);

	m_textCtrl32 = new wxTextCtrl(panel, wxID_ANY, wxT("Original Value"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer2->Add(m_textCtrl32, 0, wxALL, 5);

	m_staticText18 = new wxStaticText(panel, wxID_ANY, wxT("Type String"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText18->Wrap(-1);
	fgSizer2->Add(m_staticText18, 0, wxALL, 5);

	m_textCtrl33 = new wxTextCtrl(panel, wxID_ANY, wxT("Original Value"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer2->Add(m_textCtrl33, 0, wxALL, 5);

	m_staticText19 = new wxStaticText(panel, wxID_ANY, wxT("Choose Number"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText19->Wrap(-1);
	fgSizer2->Add(m_staticText19, 0, wxALL, 5);

	m_spinCtrlDouble1 = new wxSpinCtrlDouble(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0.000000, 1);
	m_spinCtrlDouble1->SetDigits(0);
	fgSizer2->Add(m_spinCtrlDouble1, 0, wxALL, 5);

	m_staticText21 = new wxStaticText(panel, wxID_ANY, wxT("Choose Number"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText21->Wrap(-1);
	fgSizer2->Add(m_staticText21, 0, wxALL, 5);

	m_spinCtrlDouble2 = new wxSpinCtrlDouble(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0, 1);
	m_spinCtrlDouble2->SetDigits(0);
	fgSizer2->Add(m_spinCtrlDouble2, 0, wxALL, 5);

	m_staticText28 = new wxStaticText(panel, wxID_ANY, wxT("Choose Number"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText28->Wrap(-1);
	fgSizer2->Add(m_staticText28, 0, wxALL, 5);

	m_spinCtrlDouble3 = new wxSpinCtrlDouble(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0, 1);
	m_spinCtrlDouble3->SetDigits(0);
	fgSizer2->Add(m_spinCtrlDouble3, 0, wxALL, 5);


	panel->SetSizer(fgSizer2);
	panel->Layout();

	return panel;
}