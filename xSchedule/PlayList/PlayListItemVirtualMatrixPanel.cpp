#include "PlayListItemVirtualMatrixPanel.h"
#include "PlayListItemVirtualMatrix.h"
#include "VideoWindowPositionDialog.h"
#include "PlayListDialog.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"

//(*InternalHeaders(PlayListItemVirtualMatrixPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemVirtualMatrixPanel)
const long PlayListItemVirtualMatrixPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemVirtualMatrixPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemVirtualMatrixPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemVirtualMatrixPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemVirtualMatrixPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemVirtualMatrixPanel::ID_SPINCTRL2 = wxNewId();
const long PlayListItemVirtualMatrixPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemVirtualMatrixPanel::ID_SPINCTRL3 = wxNewId();
const long PlayListItemVirtualMatrixPanel::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemVirtualMatrixPanel,wxPanel)
	//(*EventTable(PlayListItemVirtualMatrixPanel)
	//*)
END_EVENT_TABLE()

PlayListItemVirtualMatrixPanel::PlayListItemVirtualMatrixPanel(wxWindow* parent, PlayListItemVirtualMatrix* VirtualMatrix,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _VirtualMatrix = VirtualMatrix;

	//(*Initialize(PlayListItemVirtualMatrixPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Name = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Name, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Width:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Width = new wxSpinCtrl(this, ID_SPINCTRL1, _T("100"), wxDefaultPosition, wxDefaultSize, 0, 1, 5000, 100, _T("ID_SPINCTRL1"));
	SpinCtrl_Width->SetValue(_T("100"));
	FlexGridSizer1->Add(SpinCtrl_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Height:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Height = new wxSpinCtrl(this, ID_SPINCTRL2, _T("100"), wxDefaultPosition, wxDefaultSize, 0, 1, 5000, 100, _T("ID_SPINCTRL2"));
	SpinCtrl_Height->SetValue(_T("100"));
	FlexGridSizer1->Add(SpinCtrl_Height, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Start Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_StartChannel = new wxSpinCtrl(this, ID_SPINCTRL3, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL3"));
	SpinCtrl_StartChannel->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_StartChannel, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_PositionWindow = new wxButton(this, ID_BUTTON1, _("Position Window"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(Button_PositionWindow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlayListItemVirtualMatrixPanel::OnButton_PositionWindowClick);
	//*)

    long channels = xScheduleFrame::GetScheduleManager()->GetTotalChannels();
    SpinCtrl_StartChannel->SetRange(1, channels);

    TextCtrl_Name->SetValue(VirtualMatrix->GetRawName());
    SpinCtrl_Height->SetValue(VirtualMatrix->GetHeight());
    SpinCtrl_Width->SetValue(VirtualMatrix->GetWidth());
    SpinCtrl_StartChannel->SetValue(VirtualMatrix->GetStartChannel());
}

PlayListItemVirtualMatrixPanel::~PlayListItemVirtualMatrixPanel()
{
	//(*Destroy(PlayListItemVirtualMatrixPanel)
	//*)
    _VirtualMatrix->SetWidth(SpinCtrl_Width->GetValue());
    _VirtualMatrix->SetHeight(SpinCtrl_Height->GetValue());
    _VirtualMatrix->SetStartChannel(SpinCtrl_StartChannel->GetValue());
}

void PlayListItemVirtualMatrixPanel::OnButton_PositionWindowClick(wxCommandEvent& event)
{
    VideoWindowPositionDialog dlg(this, wxID_ANY, _VirtualMatrix->GetPosition(), _VirtualMatrix->GetSize());

    dlg.ShowModal();

    if (dlg.IsFullScreen())
    {
        _VirtualMatrix->SetLocation(dlg.GetClientAreaOrigin(), dlg.GetClientSize());
    }
    else
    {
        _VirtualMatrix->SetLocation(dlg.GetPosition(), dlg.GetSize());
    }
}

void PlayListItemVirtualMatrixPanel::OnTextCtrl_NameText(wxCommandEvent& event)
{
    _VirtualMatrix->SetName(TextCtrl_Name->GetValue().ToStdString());
    ((PlayListDialog*)GetParent()->GetParent()->GetParent()->GetParent())->UpdateTree();
}
