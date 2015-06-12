#include "PixelAppearanceDlg.h"

//(*InternalHeaders(PixelAppearanceDlg)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(PixelAppearanceDlg)
const long PixelAppearanceDlg::ID_STATICTEXT1 = wxNewId();
const long PixelAppearanceDlg::ID_SPINCTRL1 = wxNewId();
const long PixelAppearanceDlg::ID_CHOICE1 = wxNewId();
const long PixelAppearanceDlg::ID_SLIDER_TRANSPARENCY = wxNewId();
const long PixelAppearanceDlg::IDD_TEXTCTRL_TRANSPARENCY = wxNewId();
const long PixelAppearanceDlg::ID_STATICTEXT2 = wxNewId();
const long PixelAppearanceDlg::ID_SLIDER_BLACKT = wxNewId();
const long PixelAppearanceDlg::IDD_TEXTCTRL_BLACKT = wxNewId();
const long PixelAppearanceDlg::ID_SLIDER_BRIGHTNESS = wxNewId();
const long PixelAppearanceDlg::IDD_TEXTCTRL_BRIGHTNESS = wxNewId();
//*)

BEGIN_EVENT_TABLE(PixelAppearanceDlg,wxDialog)
	//(*EventTable(PixelAppearanceDlg)
	//*)
END_EVENT_TABLE()

PixelAppearanceDlg::PixelAppearanceDlg(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(PixelAppearanceDlg)
	wxStaticText* StaticText2;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticText* StaticText3;
	wxStaticText* StaticText5;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, _("Pixel Appearance"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer1->AddGrowableRow(0);
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer5->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Pixel Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer5->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	PixelSizeSpinner = new wxSpinCtrl(this, ID_SPINCTRL1, _T("2"), wxDefaultPosition, wxDefaultSize, 0, 1, 300, 2, _T("ID_SPINCTRL1"));
	PixelSizeSpinner->SetValue(_T("2"));
	FlexGridSizer5->Add(PixelSizeSpinner, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Pixel Style:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer5->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	PixelStyleBox = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	PixelStyleBox->SetSelection( PixelStyleBox->Append(_("Default (fastest)")) );
	PixelStyleBox->Append(_("Smooth"));
	PixelStyleBox->Append(_("Solid Circle"));
	PixelStyleBox->Append(_("Blended Circle"));
	FlexGridSizer5->Add(PixelStyleBox, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, wxID_ANY, _("Transparency:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer5->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TransparencySlider = new wxSlider(this, ID_SLIDER_TRANSPARENCY, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_TRANSPARENCY"));
	FlexGridSizer5->Add(TransparencySlider, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TransparencyText = new wxTextCtrl(this, IDD_TEXTCTRL_TRANSPARENCY, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_TRANSPARENCY"));
	TransparencyText->SetMaxLength(4);
	FlexGridSizer5->Add(TransparencyText, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT2, _("Black Transparency:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer5->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BlackTransparency = new wxSlider(this, ID_SLIDER_BLACKT, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BLACKT"));
	FlexGridSizer5->Add(BlackTransparency, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BlackTransparencyText = new wxTextCtrl(this, IDD_TEXTCTRL_BLACKT, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_BLACKT"));
	BlackTransparencyText->SetMaxLength(4);
	FlexGridSizer5->Add(BlackTransparencyText, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, wxID_ANY, _("Preview Brightness:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer5->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Brightness = new wxSlider(this, ID_SLIDER_BRIGHTNESS, 100, 1, 300, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BRIGHTNESS"));
	FlexGridSizer5->Add(Brightness, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BrightnessText = new wxTextCtrl(this, IDD_TEXTCTRL_BRIGHTNESS, _("100"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_BRIGHTNESS"));
	BrightnessText->SetMaxLength(4);
	FlexGridSizer5->Add(BrightnessText, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer2->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_SLIDER_TRANSPARENCY,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&PixelAppearanceDlg::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_TRANSPARENCY,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PixelAppearanceDlg::UpdateLinkedSlider);
	Connect(ID_SLIDER_BLACKT,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&PixelAppearanceDlg::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_BLACKT,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PixelAppearanceDlg::UpdateLinkedSlider);
	Connect(ID_SLIDER_BRIGHTNESS,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&PixelAppearanceDlg::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_BRIGHTNESS,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PixelAppearanceDlg::UpdateLinkedSlider);
	//*)
}

PixelAppearanceDlg::~PixelAppearanceDlg()
{
	//(*Destroy(PixelAppearanceDlg)
	//*)
}


void PixelAppearanceDlg::OnSpinButton1Change(wxSpinEvent& event)
{
}

void PixelAppearanceDlg::UpdateLinkedSlider(wxCommandEvent& event)
{
    wxTextCtrl * txt = (wxTextCtrl*)event.GetEventObject();
    wxString name = txt->GetName();
    if (name.Contains("IDD_")) {
        name.Replace("IDD_TEXTCTRL_", "ID_SLIDER_");
    } else {
        name.Replace("ID_TEXTCTRL_", "IDD_SLIDER_");
    }
    wxSlider *slider = (wxSlider*)txt->GetParent()->FindWindowByName(name);
    if (slider == nullptr) {
        return;
    }
    int value = wxAtoi(txt->GetValue());
    
    if (value < slider->GetMin()) {
        value = slider->GetMin();
        wxString val_str;
        val_str << value;
        txt->ChangeValue(val_str);
    }
    else if (value > slider->GetMax()) {
        value = slider->GetMax();
        wxString val_str;
        val_str << value;
        txt->ChangeValue(val_str);
    }
    slider->SetValue(value);
}

void PixelAppearanceDlg::UpdateLinkedTextCtrl(wxScrollEvent& event)
{
    wxSlider * slider = (wxSlider*)event.GetEventObject();
    wxString name = slider->GetName();
    if (name.Contains("ID_")) {
        name.Replace("ID_SLIDER_", "IDD_TEXTCTRL_");
    } else {
        name.Replace("IDD_SLIDER_", "ID_TEXTCTRL_");
    }
    wxTextCtrl *txt = (wxTextCtrl*)slider->GetParent()->FindWindowByName(name);
    txt->ChangeValue(wxString::Format("%d",slider->GetValue()));
}
