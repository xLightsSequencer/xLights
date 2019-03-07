#include "BulkEditSliderDialog.h"

//(*InternalHeaders(BulkEditSliderDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "ValueCurveDialog.h"

//(*IdInit(BulkEditSliderDialog)
const long BulkEditSliderDialog::ID_STATICTEXT_BulkEdit = wxNewId();
const long BulkEditSliderDialog::ID_SLIDER_BulkEdit = wxNewId();
const long BulkEditSliderDialog::ID_VALUECURVE_BulkEdit = wxNewId();
const long BulkEditSliderDialog::ID_TEXTCTRL_BulkEdit = wxNewId();
const long BulkEditSliderDialog::ID_BUTTON1 = wxNewId();
const long BulkEditSliderDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(BulkEditSliderDialog,wxDialog)
	//(*EventTable(BulkEditSliderDialog)
	//*)
END_EVENT_TABLE()

BulkEditSliderDialog::BulkEditSliderDialog(wxWindow* parent, const std::string& label, int value, int min, int max, int pageSize, BESLIDERTYPE type, ValueCurveButton* vcb, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _vcb = vcb;

	//(*Initialize(BulkEditSliderDialog)
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText_Label = new wxStaticText(this, ID_STATICTEXT_BulkEdit, _("Label:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_BulkEdit"));
	FlexGridSizer1->Add(StaticText_Label, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_BulkEdit = new wxSlider(this, ID_SLIDER_BulkEdit, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BulkEdit"));
	FlexGridSizer3->Add(Slider_BulkEdit, 1, wxALL|wxEXPAND, 5);
	BitmapButton_VC = new ValueCurveButton(this, ID_VALUECURVE_BulkEdit, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_BulkEdit"));
	FlexGridSizer3->Add(BitmapButton_VC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	TextCtrl_BulkEdit = new wxTextCtrl(this, ID_TEXTCTRL_BulkEdit, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_BulkEdit"));
	FlexGridSizer1->Add(TextCtrl_BulkEdit, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_SLIDER_BulkEdit,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BulkEditSliderDialog::OnSlider_BulkEditCmdSliderUpdated);
	Connect(ID_VALUECURVE_BulkEdit,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BulkEditSliderDialog::OnBitmapButton_VCClick);
	Connect(ID_TEXTCTRL_BulkEdit,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BulkEditSliderDialog::OnTextCtrl_BulkEditText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BulkEditSliderDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BulkEditSliderDialog::OnButton_CancelClick);
	//*)

    _type = type;

    if (vcb == nullptr)
    {
        BitmapButton_VC->Hide();
    }
    else
    {
        Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&BulkEditSliderDialog::OnVCChanged, 0, this);
        auto vcs = _vcb->GetValue()->Serialise();
        BitmapButton_VC->SetLimits(_vcb->GetValue()->GetMin(), _vcb->GetValue()->GetMax());
        BitmapButton_VC->SetValue(vcs);
    }

    StaticText_Label->SetLabel(label);
    Slider_BulkEdit->SetMin(min);
    Slider_BulkEdit->SetMax(max);
    Slider_BulkEdit->SetValue(value);
    Slider_BulkEdit->SetPageSize(pageSize);

    wxScrollEvent e;
    OnSlider_BulkEditCmdSliderUpdated(e);

    SetEscapeId(Button_Cancel->GetId());

    FlexGridSizer1->Fit(this);
}

BulkEditSliderDialog::~BulkEditSliderDialog()
{
	//(*Destroy(BulkEditSliderDialog)
	//*)
}


void BulkEditSliderDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void BulkEditSliderDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void BulkEditSliderDialog::OnSlider_BulkEditCmdSliderUpdated(wxScrollEvent& event)
{
    switch(_type)
    {
    case BESLIDERTYPE::BE_INT:
    {
        auto s = wxString::Format("%d", Slider_BulkEdit->GetValue());
        if (TextCtrl_BulkEdit->GetValue() != s)
        {
            TextCtrl_BulkEdit->SetValue(s);
        }
    }
        break;
    case BESLIDERTYPE::BE_FLOAT1:
    {
        auto s = wxString::Format("%.1f", (float)Slider_BulkEdit->GetValue() / 10.0);
        if (TextCtrl_BulkEdit->GetValue() != s)
        {
            TextCtrl_BulkEdit->SetValue(s);
        }
    }
    break;
    case BESLIDERTYPE::BE_FLOAT2:
    {
        auto s = wxString::Format("%.2f", (float)Slider_BulkEdit->GetValue() / 100.0);
        if (TextCtrl_BulkEdit->GetValue() != s)
        {
            TextCtrl_BulkEdit->SetValue(s);
        }
    }
    break;
    case BESLIDERTYPE::BE_FLOAT360:
    {
        auto s = wxString::Format("%.2f", (float)Slider_BulkEdit->GetValue() / 360.0);
        if (TextCtrl_BulkEdit->GetValue() != s)
        {
            TextCtrl_BulkEdit->SetValue(s);
        }
    }
    break;
    }
}

void BulkEditSliderDialog::OnTextCtrl_BulkEditText(wxCommandEvent& event)
{
    switch (_type)
    {
    case BESLIDERTYPE::BE_INT:
    {
        int i = wxAtoi(TextCtrl_BulkEdit->GetValue());
        if (Slider_BulkEdit->GetValue() != i)
        {
            Slider_BulkEdit->SetValue(i);
        }
    }
    break;
    case BESLIDERTYPE::BE_FLOAT1:
    {
        float f = wxAtof(TextCtrl_BulkEdit->GetValue());
        if (Slider_BulkEdit->GetValue() != (int)(f * 10))
        {
            Slider_BulkEdit->SetValue(f * 10);
        }
    }
    break;
    case BESLIDERTYPE::BE_FLOAT2:
    {
        float f = wxAtof(TextCtrl_BulkEdit->GetValue());
        if (Slider_BulkEdit->GetValue() != (int)(f * 100))
        {
            Slider_BulkEdit->SetValue(f * 100);
        }
    }
    break;
    case BESLIDERTYPE::BE_FLOAT360:
    {
        float f = wxAtof(TextCtrl_BulkEdit->GetValue());
        if (Slider_BulkEdit->GetValue() != (int)(f * 360))
        {
            Slider_BulkEdit->SetValue(f * 360);
        }
    }
    break;
    }
}

void BulkEditSliderDialog::OnVCChanged(wxCommandEvent& event)
{
    ValueCurveButton * vcb = (ValueCurveButton*)event.GetEventObject();
    wxString name = vcb->GetName();
    wxString slidername = name;
    wxString slidername2 = name;
    slidername.Replace("ID_VALUECURVE_", "ID_SLIDER_");
    slidername2.Replace("ID_VALUECURVE_", "IDD_SLIDER_");
    wxString textctrlname = name;
    wxString textctrlname2 = name;
    textctrlname.Replace("ID_VALUECURVE_", "ID_TEXTCTRL_");
    textctrlname2.Replace("ID_VALUECURVE_", "IDD_TEXTCTRL_");
    wxSlider* slider = (wxSlider*)vcb->GetParent()->FindWindowByName(slidername);
    if (slider == nullptr || (void*)slider == (void*)vcb)
    {
        slider = (wxSlider*)vcb->GetParent()->FindWindowByName(slidername2);
    }
    wxTextCtrl* textctrl = (wxTextCtrl*)vcb->GetParent()->FindWindowByName(textctrlname);
    if (textctrl == nullptr || (void*)textctrl == (void*)vcb)
    {
        textctrl = (wxTextCtrl*)vcb->GetParent()->FindWindowByName(textctrlname2);
    }

    wxASSERT(slider != nullptr && (void*)slider != (void*)vcb);
    wxASSERT(textctrl != nullptr && (void*)textctrl != (void*)vcb);

    if (vcb->GetValue()->IsActive())
    {
        if (slider != nullptr)
        {
            slider->Disable();
        }
        if (textctrl != nullptr)
        {
            textctrl->Disable();
        }
    }
    else
    {
        if (slider != nullptr)
        {
            slider->Enable();
        }
        if (textctrl != nullptr)
        {
            textctrl->Enable();
        }
    }
}

void BulkEditSliderDialog::OnBitmapButton_VCClick(wxCommandEvent& event)
{
    ValueCurveButton * vc = (ValueCurveButton*)event.GetEventObject();

    bool slideridd = false;
    wxString name = vc->GetName();
    name.Replace("IDD_VALUECURVE_", "ID_SLIDER_");
    name.Replace("ID_VALUECURVE_", "ID_SLIDER_");
    wxSlider *slider = (wxSlider*)vc->GetParent()->FindWindowByName(name);
    if (slider == nullptr || (void*)slider == (void*)vc)
    {
        name = vc->GetName();
        name.Replace("IDD_VALUECURVE_", "IDD_SLIDER_");
        name.Replace("ID_VALUECURVE_", "IDD_SLIDER_");
        slider = (wxSlider*)vc->GetParent()->FindWindowByName(name);
        if (slider != nullptr)
        {
            slideridd = true;
        }
    }

    name = vc->GetName();
    name.Replace("IDD_VALUECURVE_", "ID_TEXTCTRL_");
    name.Replace("ID_VALUECURVE_", "ID_TEXTCTRL_");
    wxTextCtrl *txt = (wxTextCtrl*)vc->GetParent()->FindWindowByName(name);
    if (txt == nullptr || (void*)txt == (void*)vc)
    {
        name = vc->GetName();
        name.Replace("IDD_VALUECURVE_", "IDD_TEXTCTRL_");
        name.Replace("ID_VALUECURVE_", "IDD_TEXTCTRL_");
        txt = (wxTextCtrl*)vc->GetParent()->FindWindowByName(name);
    }

    vc->ToggleActive();
    if (vc->GetValue()->IsActive())
    {
        ValueCurveDialog vcd(vc->GetParent(), vc->GetValue(), slideridd);
        if (vcd.ShowModal() == wxOK)
        {
            if (slider != nullptr)
            {
                slider->Disable();
            }
            if (txt != nullptr)
            {
                txt->Disable();
            }
        }
        else
        {
            if (slider != nullptr)
            {
                slider->Enable();
            }
            if (txt != nullptr)
            {
                txt->Enable();
            }
            vc->SetActive(false);
        }
        vc->UpdateState();
    }
    else
    {
        if (slider != nullptr)
        {
            slider->Enable();
        }
        if (txt != nullptr)
        {
            txt->Enable();
        }
    }
}
