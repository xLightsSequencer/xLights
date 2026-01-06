/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "VideoWindowPositionDialog.h"
#include "../ScheduleOptions.h"

//(*InternalHeaders(VideoWindowPositionDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "./utils/spdlog_macros.h"

//(*IdInit(VideoWindowPositionDialog)
const long VideoWindowPositionDialog::ID_STATICTEXT1 = wxNewId();
const long VideoWindowPositionDialog::ID_STATICTEXT2 = wxNewId();
const long VideoWindowPositionDialog::ID_CHECKBOX1 = wxNewId();
const long VideoWindowPositionDialog::ID_STATICTEXT3 = wxNewId();
const long VideoWindowPositionDialog::ID_SPINCTRL1 = wxNewId();
const long VideoWindowPositionDialog::ID_BUTTON2 = wxNewId();
const long VideoWindowPositionDialog::ID_BUTTON3 = wxNewId();
const long VideoWindowPositionDialog::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(VideoWindowPositionDialog,wxDialog)
	//(*EventTable(VideoWindowPositionDialog)
	//*)
END_EVENT_TABLE()

VideoWindowPositionDialog::VideoWindowPositionDialog(wxWindow* parent, ScheduleOptions* options, wxWindowID id,const wxPoint& pos,const wxSize& size, const wxSize& matrixSize, bool useMatrixSize, int matrixMultiplier)
{
    _options = options;
    _matrixSize = matrixSize;

	//(*Initialize(VideoWindowPositionDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Position and size window where you want the content displayed."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
	StaticText_Position = new wxStaticText(this, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText_Position, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_SetSizeBasedOnMatrix = new wxCheckBox(this, ID_CHECKBOX1, _("Set size basd on matrix size"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_SetSizeBasedOnMatrix->SetValue(false);
	FlexGridSizer2->Add(CheckBox_SetSizeBasedOnMatrix, 0, wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT3, _("Size Multiplier:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_SizeMultiplier = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_SizeMultiplier->SetValue(_T("1"));
	FlexGridSizer2->Add(SpinCtrl_SizeMultiplier, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_ApplyDefault = new wxButton(this, ID_BUTTON2, _("Apply Default"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(Button_ApplyDefault, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_MakeDefault = new wxButton(this, ID_BUTTON3, _("Make Default"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer3->Add(Button_MakeDefault, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(Button_ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&VideoWindowPositionDialog::OnCheckBox_SetSizeBasedOnMatrixClick);
	Connect(ID_SPINCTRL1,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&VideoWindowPositionDialog::OnSpinCtrl_SizeMultiplierChange);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoWindowPositionDialog::OnButton_ApplyDefaultClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoWindowPositionDialog::OnButton_MakeDefaultClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoWindowPositionDialog::OnButton_OkClick);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&VideoWindowPositionDialog::OnResize);
	//*)

    Connect(wxEVT_MOVE, (wxObjectEventFunction)&VideoWindowPositionDialog::OnMove);

    Move(pos);

    if (_matrixSize.GetWidth() != -1)
    {
        CheckBox_SetSizeBasedOnMatrix->SetValue(useMatrixSize);
        SpinCtrl_SizeMultiplier->SetValue(matrixMultiplier);
    }
    else
    {
        useMatrixSize = false;
    }

    if (useMatrixSize)
    {
        SetSize(200, 100);
    }
    else
    {
        SetSize(size);
    }

    ValidateWindow();
    SetWindowPositionText();

    
    int w, h;
    GetSize(&w, &h);
    int x, y;
    GetPosition(&x, &y);
    LOG_INFO("Position window created location (%d, %d) size (%d, %d).", x, y, w, h);
}

VideoWindowPositionDialog::~VideoWindowPositionDialog()
{
	//(*Destroy(VideoWindowPositionDialog)
	//*)
}

wxSize VideoWindowPositionDialog::GetDesiredSize() const
{
    wxSize res = GetSize();

    if (IsFullScreen())
    {
        res = GetClientSize();
    }

    if (_matrixSize.GetWidth() != -1 && CheckBox_SetSizeBasedOnMatrix->GetValue())
    {
        res.SetWidth(_matrixSize.GetWidth() * SpinCtrl_SizeMultiplier->GetValue());
        res.SetHeight(_matrixSize.GetHeight() * SpinCtrl_SizeMultiplier->GetValue());
    }

    return res;
}

wxPoint VideoWindowPositionDialog::GetDesiredPosition() const
{
    wxPoint res = GetPosition();

    if (IsFullScreen())
    {
        res = GetClientAreaOrigin();
    }
    return res;
}

void VideoWindowPositionDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void VideoWindowPositionDialog::OnResize(wxSizeEvent& event)
{
    SetWindowPositionText();
    ValidateWindow();
}

void VideoWindowPositionDialog::OnMove(wxMoveEvent& event)
{
    SetWindowPositionText();
    ValidateWindow();
}

void VideoWindowPositionDialog::SetWindowPositionText()
{
    wxSize sz = GetDesiredSize();
    wxPoint pos = GetDesiredPosition();
    StaticText_Position->SetLabel(wxString::Format("X: %d, Y: %d, W: %d, H: %d", pos.x, pos.y, sz.GetWidth(), sz.GetHeight()));
}

void VideoWindowPositionDialog::ValidateWindow()
{
    if (_matrixSize.GetWidth() == -1) {
        SetWindowStyle(GetWindowStyle() | wxRESIZE_BORDER);
        SpinCtrl_SizeMultiplier->Disable();
        CheckBox_SetSizeBasedOnMatrix->Disable();
    }
    else {
        CheckBox_SetSizeBasedOnMatrix->Enable();
        if (CheckBox_SetSizeBasedOnMatrix->GetValue()) {
            SetWindowStyle(GetWindowStyle() & ~wxRESIZE_BORDER);
            SpinCtrl_SizeMultiplier->Enable();
        }
        else {
            SetWindowStyle(GetWindowStyle() | wxRESIZE_BORDER);
            SpinCtrl_SizeMultiplier->Disable();
        }
    }

    if (CheckBox_SetSizeBasedOnMatrix->GetValue()) {
        Button_ApplyDefault->Enable(false);
        Button_MakeDefault->Enable(false);
    }
    else {
        if (_options->GetDefaultVideoPos() != GetDesiredPosition() || _options->GetDefaultVideoSize() != GetDesiredSize()) {
            Button_ApplyDefault->Enable();
            Button_MakeDefault->Enable();
        }
        else {
            Button_ApplyDefault->Enable(false);
            Button_MakeDefault->Enable(false);
        }
    }
}

void VideoWindowPositionDialog::OnCheckBox_SetSizeBasedOnMatrixClick(wxCommandEvent& event)
{
    if (CheckBox_SetSizeBasedOnMatrix->GetValue())
    {
        SetSize(200, 100);
    }
    SetWindowPositionText();
    ValidateWindow();
}

void VideoWindowPositionDialog::OnSpinCtrl_SizeMultiplierChange(wxSpinEvent& event)
{
    SetWindowPositionText();
    ValidateWindow();
}

void VideoWindowPositionDialog::OnButton_ApplyDefaultClick(wxCommandEvent& event)
{
    Move(_options->GetDefaultVideoPos());
    SetSize(_options->GetDefaultVideoSize());
    ValidateWindow();
    SetWindowPositionText();
}

void VideoWindowPositionDialog::OnButton_MakeDefaultClick(wxCommandEvent& event)
{
    _options->SetDefaultVideoSize(GetDesiredSize());
    _options->SetDefaultVideoPos(GetDesiredPosition());
    ValidateWindow();
}
