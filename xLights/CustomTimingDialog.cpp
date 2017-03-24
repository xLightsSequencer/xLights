#include "CustomTimingDialog.h"

//(*InternalHeaders(CustomTimingDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(CustomTimingDialog)
const long CustomTimingDialog::ID_STATICTEXT1 = wxNewId();
const long CustomTimingDialog::ID_SPINCTRL_Interval = wxNewId();
const long CustomTimingDialog::ID_TEXTCTRL_FPS = wxNewId();
const long CustomTimingDialog::ID_BUTTON_Ok = wxNewId();
//*)

BEGIN_EVENT_TABLE(CustomTimingDialog,wxDialog)
	//(*EventTable(CustomTimingDialog)
	//*)
END_EVENT_TABLE()

CustomTimingDialog::CustomTimingDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(CustomTimingDialog)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Frame interval (msec):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Interval = new wxSpinCtrl(this, ID_SPINCTRL_Interval, _T("33"), wxDefaultPosition, wxDefaultSize, 0, 15, 1000, 33, _T("ID_SPINCTRL_Interval"));
	SpinCtrl_Interval->SetValue(_T("33"));
	FlexGridSizer1->Add(SpinCtrl_Interval, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_FPS = new wxTextCtrl(this, ID_TEXTCTRL_FPS, _("33.30 fps"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_FPS"));
	FlexGridSizer1->Add(TextCtrl_FPS, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Ok = new wxButton(this, ID_BUTTON_Ok, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Ok"));
	Button_Ok->SetDefault();
	FlexGridSizer1->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_SPINCTRL_Interval,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&CustomTimingDialog::OnSpinCtrl_IntervalChange);
	Connect(ID_BUTTON_Ok,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomTimingDialog::OnButton_OkClick);
	//*)
}

CustomTimingDialog::~CustomTimingDialog()
{
	//(*Destroy(CustomTimingDialog)
	//*)
}

void CustomTimingDialog::OnSpinCtrl_IntervalChange(wxSpinEvent& event)
{
    TextCtrl_FPS->SetValue(wxString::Format("%.2f fps", 1000.0 / SpinCtrl_Interval->GetValue()));
}

void CustomTimingDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

wxString CustomTimingDialog::GetTiming() const
{
    return wxString::Format("%i ms", SpinCtrl_Interval->GetValue());
}
