#include "BufferPanel.h"

//(*InternalHeaders(BufferPanel)
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/intl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
//*)

#include "effects/EffectPanelUtils.h"
#include "../include/padlock16x16-blue.xpm" //-DJ

//(*IdInit(BufferPanel)
const long BufferPanel::ID_CHOICE_BufferStyle = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_CHOICE_BufferStyle = wxNewId();
const long BufferPanel::ID_CHOICE_BufferTransform = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_CHOICE_BufferTransform = wxNewId();
const long BufferPanel::ID_STATICTEXT2 = wxNewId();
const long BufferPanel::ID_SLIDER_EffectBlur = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_EffectBlur = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_SLIDER_EffectBlur = wxNewId();
const long BufferPanel::ID_CHECKBOX_OverlayBkg = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_OverlayBkg = wxNewId();
const long BufferPanel::ID_CHECKBOX_RotoZoom = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_CHECKBOX_RotoZoom = wxNewId();
const long BufferPanel::ID_STATICTEXT4 = wxNewId();
const long BufferPanel::ID_SLIDER_ZoomCycles = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_ZoomCycles = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_SLIDER_ZoomCycles = wxNewId();
const long BufferPanel::ID_STATICTEXT5 = wxNewId();
const long BufferPanel::ID_SLIDER_ZoomRotation = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_ZoomRotation = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_SLIDER_ZoomRotation = wxNewId();
const long BufferPanel::ID_STATICTEXT6 = wxNewId();
const long BufferPanel::ID_SLIDER_ZoomInOut = wxNewId();
const long BufferPanel::IDD_TEXTCTRL_ZoomInOut = wxNewId();
const long BufferPanel::ID_BITMAPBUTTON_SLIDER_ZoomInOut = wxNewId();
const long BufferPanel::ID_SCROLLEDWINDOW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(BufferPanel,wxPanel)
	//(*EventTable(BufferPanel)
	//*)
END_EVENT_TABLE()

BufferPanel::BufferPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(BufferPanel)
	wxBitmapButton* BitmapButtonBufferStyle;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer9;
	wxFlexGridSizer* FlexGridSizer6;
	wxStaticText* StaticText4;
	wxFlexGridSizer* FlexGridSizer10;
	wxStaticText* StaticText2;
	wxBitmapButton* BitmapButton1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	ScrolledWindow1 = new wxScrolledWindow(this, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText4 = new wxStaticText(ScrolledWindow1, wxID_ANY, _("Render Style"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BufferStyleChoice = new wxChoice(ScrolledWindow1, ID_CHOICE_BufferStyle, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_BufferStyle"));
	BufferStyleChoice->SetSelection( BufferStyleChoice->Append(_("Default")) );
	BufferStyleChoice->Append(_("Per Preview"));
	FlexGridSizer2->Add(BufferStyleChoice, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButtonBufferStyle = new wxBitmapButton(ScrolledWindow1, ID_BITMAPBUTTON_CHOICE_BufferStyle, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_BufferStyle"));
	BitmapButtonBufferStyle->SetDefault();
	BitmapButtonBufferStyle->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButtonBufferStyle->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	FlexGridSizer2->Add(BitmapButtonBufferStyle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(ScrolledWindow1, wxID_ANY, _("Transformation"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BufferTransform = new wxChoice(ScrolledWindow1, ID_CHOICE_BufferTransform, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_BufferTransform"));
	BufferTransform->SetSelection( BufferTransform->Append(_("None")) );
	BufferTransform->Append(_("Rotate CC 90"));
	BufferTransform->Append(_("Rotate CW 90"));
	BufferTransform->Append(_("Rotate 180"));
	BufferTransform->Append(_("Flip Vertical"));
	BufferTransform->Append(_("Flip Horizontal"));
	FlexGridSizer2->Add(BufferTransform, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton1 = new wxBitmapButton(ScrolledWindow1, ID_BITMAPBUTTON_CHOICE_BufferTransform, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_BufferTransform"));
	BitmapButton1->SetDefault();
	BitmapButton1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton1->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	FlexGridSizer2->Add(BitmapButton1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(ScrolledWindow1, ID_STATICTEXT2, _("Blur"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	Slider_EffectBlur = new wxSlider(ScrolledWindow1, ID_SLIDER_EffectBlur, 0, 1, 15, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_EffectBlur"));
	FlexGridSizer6->Add(Slider_EffectBlur, 1, wxALL|wxEXPAND, 1);
	txtCtlEffectBlur = new wxTextCtrl(ScrolledWindow1, IDD_TEXTCTRL_EffectBlur, _("1"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow1,wxSize(20,-1)), wxTE_PROCESS_ENTER|wxTAB_TRAVERSAL, wxDefaultValidator, _T("IDD_TEXTCTRL_EffectBlur"));
	txtCtlEffectBlur->SetMaxLength(2);
	FlexGridSizer6->Add(txtCtlEffectBlur, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer2->Add(FlexGridSizer6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_EffectBlur = new wxBitmapButton(ScrolledWindow1, ID_BITMAPBUTTON_SLIDER_EffectBlur, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_EffectBlur"));
	BitmapButton_EffectBlur->SetDefault();
	BitmapButton_EffectBlur->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_EffectBlur->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	FlexGridSizer2->Add(BitmapButton_EffectBlur, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_OverlayBkg = new wxCheckBox(ScrolledWindow1, ID_CHECKBOX_OverlayBkg, _("Persistent"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_OverlayBkg"));
	CheckBox_OverlayBkg->SetValue(false);
	FlexGridSizer2->Add(CheckBox_OverlayBkg, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_OverlayBkg = new wxBitmapButton(ScrolledWindow1, ID_BITMAPBUTTON_OverlayBkg, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_OverlayBkg"));
	BitmapButton_OverlayBkg->SetDefault();
	BitmapButton_OverlayBkg->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_OverlayBkg->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	FlexGridSizer2->Add(BitmapButton_OverlayBkg, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_RotoZoom = new wxCheckBox(ScrolledWindow1, ID_CHECKBOX_RotoZoom, _("RotoZoom\?"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_RotoZoom"));
	CheckBox_RotoZoom->SetValue(false);
	CheckBox_RotoZoom->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVECAPTION));
	FlexGridSizer2->Add(CheckBox_RotoZoom, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_RotoZoom = new wxBitmapButton(ScrolledWindow1, ID_BITMAPBUTTON_CHECKBOX_RotoZoom, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_RotoZoom"));
	BitmapButton_RotoZoom->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_RotoZoom->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	FlexGridSizer2->Add(BitmapButton_RotoZoom, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(ScrolledWindow1, ID_STATICTEXT4, _("Zoom Cycles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	StaticText6->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVECAPTION));
	FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer8 = new wxFlexGridSizer(0, 3, 0, 0);
	Slider_ZoomCycles = new wxSlider(ScrolledWindow1, ID_SLIDER_ZoomCycles, 0, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_ZoomCycles"));
	FlexGridSizer8->Add(Slider_ZoomCycles, 1, wxALL|wxEXPAND, 2);
	TextCtrl_ZoomCycles = new wxTextCtrl(ScrolledWindow1, IDD_TEXTCTRL_ZoomCycles, _("1"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow1,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_ZoomCycles"));
	FlexGridSizer8->Add(TextCtrl_ZoomCycles, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer2->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 2);
	BitmapButton_ZoomCycles = new wxBitmapButton(ScrolledWindow1, ID_BITMAPBUTTON_SLIDER_ZoomCycles, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_ZoomCycles"));
	BitmapButton_ZoomCycles->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_ZoomCycles->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	FlexGridSizer2->Add(BitmapButton_ZoomCycles, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText7 = new wxStaticText(ScrolledWindow1, ID_STATICTEXT5, _("Zoom Rotation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	StaticText7->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVECAPTION));
	FlexGridSizer2->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer9 = new wxFlexGridSizer(0, 3, 0, 0);
	Slider_ZoomRotation = new wxSlider(ScrolledWindow1, ID_SLIDER_ZoomRotation, 0, -20, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_ZoomRotation"));
	FlexGridSizer9->Add(Slider_ZoomRotation, 1, wxALL|wxEXPAND, 2);
	TextCtrl_ZoomRotation = new wxTextCtrl(ScrolledWindow1, IDD_TEXTCTRL_ZoomRotation, _("0"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow1,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_ZoomRotation"));
	FlexGridSizer9->Add(TextCtrl_ZoomRotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer2->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 2);
	BitmapButton_ZoomRotation = new wxBitmapButton(ScrolledWindow1, ID_BITMAPBUTTON_SLIDER_ZoomRotation, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_ZoomRotation"));
	BitmapButton_ZoomRotation->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer2->Add(BitmapButton_ZoomRotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText8 = new wxStaticText(ScrolledWindow1, ID_STATICTEXT6, _("Zoom In/Out"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	StaticText8->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVECAPTION));
	FlexGridSizer2->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer10 = new wxFlexGridSizer(0, 3, 0, 0);
	Slider_ZoomInOut = new wxSlider(ScrolledWindow1, ID_SLIDER_ZoomInOut, 0, -10, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_ZoomInOut"));
	FlexGridSizer10->Add(Slider_ZoomInOut, 1, wxALL|wxEXPAND, 2);
	TextCtrl_ZoomInOut = new wxTextCtrl(ScrolledWindow1, IDD_TEXTCTRL_ZoomInOut, _("0"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindow1,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_ZoomInOut"));
	FlexGridSizer10->Add(TextCtrl_ZoomInOut, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer2->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 2);
	BitmapButton_ZoomInOut = new wxBitmapButton(ScrolledWindow1, ID_BITMAPBUTTON_SLIDER_ZoomInOut, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_ZoomInOut"));
	BitmapButton_ZoomInOut->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer2->Add(BitmapButton_ZoomInOut, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ScrolledWindow1->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(ScrolledWindow1);
	FlexGridSizer2->SetSizeHints(ScrolledWindow1);
	FlexGridSizer1->Add(ScrolledWindow1, 1, wxALL|wxEXPAND, 0);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_CHOICE_BufferStyle,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_BufferTransform,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_SLIDER_EffectBlur,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferPanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_EffectBlur,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_EffectBlur,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_OverlayBkg,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_RotoZoom,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_SLIDER_ZoomCycles,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferPanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_ZoomCycles,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_ZoomCycles,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_SLIDER_ZoomRotation,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferPanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_ZoomRotation,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_ZoomRotation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	Connect(ID_SLIDER_ZoomInOut,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BufferPanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_ZoomInOut,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BufferPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_ZoomInOut,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferPanel::OnLockButtonClick);
	//*)
}

BufferPanel::~BufferPanel()
{
	//(*Destroy(BufferPanel)
	//*)
}


PANEL_EVENT_HANDLERS(BufferPanel)

wxString BufferPanel::GetBufferString() {
    wxString s;
    // Blur
    if (Slider_EffectBlur->GetValue() > 1) {
        s += wxString::Format("B_SLIDER_EffectBlur=%d,",Slider_EffectBlur->GetValue());
    }
    // RotoZoom
    if (CheckBox_RotoZoom->GetValue()) {
        s += wxString::Format("B_SLIDER_ZoomCycles=%d,",Slider_ZoomCycles->GetValue());
        s += wxString::Format("B_SLIDER_ZoomRotation=%d,",Slider_ZoomRotation->GetValue());
        s += wxString::Format("B_SLIDER_ZoomInOut=%d,",Slider_ZoomInOut->GetValue());
    }
    // Persistent
    if (CheckBox_OverlayBkg->GetValue()) {
        s += "B_CHECKBOX_OverlayBkg=1,";
    }
    if (BufferStyleChoice->GetSelection() != 0) {
        s += "B_CHOICE_BufferStyle=";
        s += BufferStyleChoice->GetStringSelection();
        s += ",";
    }
    if (BufferTransform->GetSelection() != 0) {
        s += "B_CHOICE_BufferTransform=";
        s += BufferTransform->GetStringSelection();
        s += ",";
    }
    
    return s;
}
