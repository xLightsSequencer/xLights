/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "BufferSizeDialog.h"
#include "ValueCurveDialog.h"
#include "UtilFunctions.h"

//(*InternalHeaders(BufferSizeDialog)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(BufferSizeDialog)
const long BufferSizeDialog::ID_STATICTEXT1 = wxNewId();
const long BufferSizeDialog::ID_SPINCTRL1 = wxNewId();
const long BufferSizeDialog::ID_VALUECURVE_BufferTop = wxNewId();
const long BufferSizeDialog::ID_STATICTEXT2 = wxNewId();
const long BufferSizeDialog::ID_SPINCTRL2 = wxNewId();
const long BufferSizeDialog::ID_VALUECURVE_BufferLeft = wxNewId();
const long BufferSizeDialog::ID_STATICTEXT3 = wxNewId();
const long BufferSizeDialog::ID_SPINCTRL3 = wxNewId();
const long BufferSizeDialog::ID_VALUECURVE_BufferBottom = wxNewId();
const long BufferSizeDialog::ID_STATICTEXT4 = wxNewId();
const long BufferSizeDialog::ID_SPINCTRL4 = wxNewId();
const long BufferSizeDialog::ID_VALUECURVE_BufferRight = wxNewId();
//*)

BEGIN_EVENT_TABLE(BufferSizeDialog,wxDialog)
	//(*EventTable(BufferSizeDialog)
	//*)
END_EVENT_TABLE()

BufferSizeDialog::BufferSizeDialog(wxWindow* parent, bool usevc,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(BufferSizeDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, _("Buffer Size"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Top"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Top = new wxSpinCtrlDouble(this, ID_SPINCTRL1, _T("100"), wxDefaultPosition, wxDefaultSize, 0, 1, 200, 100, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_Top->SetValue(_T("100"));
	FlexGridSizer2->Add(SpinCtrl_Top, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Top = new ValueCurveButton(this, ID_VALUECURVE_BufferTop, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_BufferTop"));
	FlexGridSizer2->Add(ValueCurve_Top, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Left"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Left = new wxSpinCtrlDouble(this, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -100, 99, 0, 1, _T("ID_SPINCTRL2"));
	SpinCtrl_Left->SetValue(_T("0"));
	FlexGridSizer2->Add(SpinCtrl_Left, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Left = new ValueCurveButton(this, ID_VALUECURVE_BufferLeft, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_BufferLeft"));
	FlexGridSizer2->Add(ValueCurve_Left, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Bottom"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Bottom = new wxSpinCtrlDouble(this, ID_SPINCTRL3, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -100, 99, 0, 1, _T("ID_SPINCTRL3"));
	SpinCtrl_Bottom->SetValue(_T("0"));
	FlexGridSizer2->Add(SpinCtrl_Bottom, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Bottom = new ValueCurveButton(this, ID_VALUECURVE_BufferBottom, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_BufferBottom"));
	FlexGridSizer2->Add(ValueCurve_Bottom, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Right"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Right = new wxSpinCtrlDouble(this, ID_SPINCTRL4, _T("100"), wxDefaultPosition, wxDefaultSize, 0, 1, 200, 100, 1, _T("ID_SPINCTRL4"));
	SpinCtrl_Right->SetValue(_T("100"));
	FlexGridSizer2->Add(SpinCtrl_Right, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Right = new ValueCurveButton(this, ID_VALUECURVE_BufferRight, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_BufferRight"));
	FlexGridSizer2->Add(ValueCurve_Right, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_SPINCTRL1,wxEVT_SPINCTRLDOUBLE,(wxObjectEventFunction)&BufferSizeDialog::OnSpinCtrl_TopChange);
	Connect(ID_VALUECURVE_BufferTop,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferSizeDialog::OnValueCurve_Click);
	Connect(ID_SPINCTRL2,wxEVT_SPINCTRLDOUBLE,(wxObjectEventFunction)&BufferSizeDialog::OnSpinCtrl_LeftChange);
	Connect(ID_VALUECURVE_BufferLeft,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferSizeDialog::OnValueCurve_Click);
	Connect(ID_SPINCTRL3,wxEVT_SPINCTRLDOUBLE,(wxObjectEventFunction)&BufferSizeDialog::OnSpinCtrl_BottomChange);
	Connect(ID_VALUECURVE_BufferBottom,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferSizeDialog::OnValueCurve_Click);
	Connect(ID_SPINCTRL4,wxEVT_SPINCTRLDOUBLE,(wxObjectEventFunction)&BufferSizeDialog::OnSpinCtrl_RightChange);
	Connect(ID_VALUECURVE_BufferRight,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BufferSizeDialog::OnValueCurve_Click);
	//*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&BufferSizeDialog::OnVCChanged, nullptr, this);

    int item = 1;
    wxWindow* ok_btn = StdDialogButtonSizer1->GetItem(item)->GetWindow();
    SetDefaultItem(ok_btn);
    SetEscapeId(wxID_CANCEL);

    SpinCtrl_Top->SetDigits(2);
    SpinCtrl_Left->SetDigits(2);
    SpinCtrl_Bottom->SetDigits(2);
    SpinCtrl_Right->SetDigits(2);

    if (usevc)
    {
        ValueCurve_Top->GetValue()->SetLimits(-100, 200);
        ValueCurve_Bottom->GetValue()->SetLimits(-100, 200);
        ValueCurve_Left->GetValue()->SetLimits(-100, 200);
        ValueCurve_Right->GetValue()->SetLimits(-100, 200);
    }
    else
    {
        ValueCurve_Top->Enable(false);
        ValueCurve_Bottom->Enable(false);
        ValueCurve_Left->Enable(false);
        ValueCurve_Right->Enable(false);
    }
}

BufferSizeDialog::~BufferSizeDialog()
{
	//(*Destroy(BufferSizeDialog)
	//*)
}

void BufferSizeDialog::SetSizes(double top, double left, double bottom, double right, const std::string& topvc, const std::string& leftvc, const std::string& bottomvc, const std::string& rightvc)
{
    SpinCtrl_Top->SetValue(top);
    SpinCtrl_Bottom->SetValue(bottom);
    if (SpinCtrl_Top->GetValue() <= SpinCtrl_Bottom->GetValue()) SpinCtrl_Bottom->SetValue(SpinCtrl_Bottom->GetValue() - 1);

    SpinCtrl_Left->SetValue(left);
    SpinCtrl_Right->SetValue(right);
    if (SpinCtrl_Left->GetValue() >= SpinCtrl_Right->GetValue()) SpinCtrl_Right->SetValue(SpinCtrl_Left->GetValue() + 1);

    ValueCurve_Top->SetValue(topvc);
    ValueCurve_Bottom->SetValue(bottomvc);
    ValueCurve_Left->SetValue(leftvc);
    ValueCurve_Right->SetValue(rightvc);
    ValueCurve_Top->GetValue()->SetLimits(-100, 200);
    ValueCurve_Bottom->GetValue()->SetLimits(-100, 200);
    ValueCurve_Left->GetValue()->SetLimits(-100, 200);
    ValueCurve_Right->GetValue()->SetLimits(-100, 200);

    ValidateWindow();
}

void BufferSizeDialog::OnSpinCtrl_TopChange(wxSpinDoubleEvent & event)
{
    if (SpinCtrl_Top->GetValue() <= SpinCtrl_Bottom->GetValue()) SpinCtrl_Bottom->SetValue(SpinCtrl_Top->GetValue() - 1);
    ValidateWindow();
}

void BufferSizeDialog::OnSpinCtrl_LeftChange(wxSpinDoubleEvent & event)
{
    if (SpinCtrl_Left->GetValue() >= SpinCtrl_Right->GetValue()) SpinCtrl_Right->SetValue(SpinCtrl_Left->GetValue() + 1);
    ValidateWindow();
}

void BufferSizeDialog::OnSpinCtrl_BottomChange(wxSpinDoubleEvent & event)
{
    if (SpinCtrl_Top->GetValue() <= SpinCtrl_Bottom->GetValue()) SpinCtrl_Top->SetValue(SpinCtrl_Bottom->GetValue() + 1);
    ValidateWindow();
}

void BufferSizeDialog::OnSpinCtrl_RightChange(wxSpinDoubleEvent & event)
{
    if (SpinCtrl_Left->GetValue() >= SpinCtrl_Right->GetValue()) SpinCtrl_Left->SetValue(SpinCtrl_Right->GetValue() - 1);
    ValidateWindow();
}

void BufferSizeDialog::OnValueCurve_Click(wxCommandEvent& event)
{
    ValueCurveButton * vc = (ValueCurveButton*)event.GetEventObject();
    int id = event.GetId();
    wxSpinCtrlDouble* spin = nullptr;
    if (id == ID_VALUECURVE_BufferTop)
    {
        spin = SpinCtrl_Top;
    }
    else if (id == ID_VALUECURVE_BufferBottom)
    {
        spin = SpinCtrl_Bottom;
    }
    else if (id == ID_VALUECURVE_BufferLeft)
    {
        spin = SpinCtrl_Left;
    }
    else if (id == ID_VALUECURVE_BufferRight)
    {
        spin = SpinCtrl_Right;
    }

    vc->ToggleActive();
    if (vc->GetValue()->IsActive())
    {
        ValueCurveDialog vcd(vc->GetParent(), vc->GetValue(), true);
        OptimiseDialogPosition(&vcd);
        if (vcd.ShowModal() != wxOK)
        {
            vc->SetActive(false);
        }
        else
        {
            if (vc->GetValue()->GetType() == "Flat")
            {
                spin->SetValue(vc->GetValue()->GetParameter1());
            }
        }
        vc->UpdateState();
    }

    ValidateWindow();
}

void BufferSizeDialog::ValidateWindow()
{
    if (ValueCurve_Top->GetValue()->IsActive())
    {
        SpinCtrl_Top->Enable(false);
    }
    else
    {
        SpinCtrl_Top->Enable();
    }
    if (ValueCurve_Bottom->GetValue()->IsActive())
    {
        SpinCtrl_Bottom->Enable(false);
    }
    else
    {
        SpinCtrl_Bottom->Enable();
    }
    if (ValueCurve_Left->GetValue()->IsActive())
    {
        SpinCtrl_Left->Enable(false);
    }
    else
    {
        SpinCtrl_Left->Enable();
    }
    if (ValueCurve_Right->GetValue()->IsActive())
    {
        SpinCtrl_Right->Enable(false);
    }
    else
    {
        SpinCtrl_Right->Enable();
    }

    if (ValueCurve_Top->GetValue()->GetType() == "Flat")
    {
        ValueCurve_Top->GetValue()->SetParameter1(SpinCtrl_Top->GetValue());
    }
    if (ValueCurve_Bottom->GetValue()->GetType() == "Flat")
    {
        ValueCurve_Bottom->GetValue()->SetParameter1(SpinCtrl_Bottom->GetValue());
    }
    if (ValueCurve_Left->GetValue()->GetType() == "Flat")
    {
        ValueCurve_Left->GetValue()->SetParameter1(SpinCtrl_Left->GetValue());
    }
    if (ValueCurve_Right->GetValue()->GetType() == "Flat")
    {
        ValueCurve_Right->GetValue()->SetParameter1(SpinCtrl_Right->GetValue());
    }
}

void BufferSizeDialog::OnVCChanged(wxCommandEvent& event)
{
    ValidateWindow();
}