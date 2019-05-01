#include "MultiControllerUploadDialog.h"

//(*InternalHeaders(MultiControllerUploadDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/listbase.h>

#include "xLightsMain.h"
#include "outputs/OutputManager.h"
#include "outputs/Output.h"
#include "controllers/ESPixelStick.h"
#include "controllers/SanDevices.h"
#include "controllers/Pixlite16.h"
#include "controllers/Falcon.h"

//(*IdInit(MultiControllerUploadDialog)
const long MultiControllerUploadDialog::ID_STATICTEXT1 = wxNewId();
const long MultiControllerUploadDialog::ID_STATICTEXT2 = wxNewId();
const long MultiControllerUploadDialog::ID_CHOICE1 = wxNewId();
const long MultiControllerUploadDialog::ID_CHECKLISTBOX1 = wxNewId();
const long MultiControllerUploadDialog::ID_BUTTON1 = wxNewId();
const long MultiControllerUploadDialog::ID_BUTTON2 = wxNewId();
const long MultiControllerUploadDialog::ID_TEXTCTRL1 = wxNewId();
//*)

const long MultiControllerUploadDialog::ID_MCU_SELECTALL = wxNewId();
const long MultiControllerUploadDialog::ID_MCU_SELECTNONE = wxNewId();

BEGIN_EVENT_TABLE(MultiControllerUploadDialog,wxDialog)
	//(*EventTable(MultiControllerUploadDialog)
	//*)
END_EVENT_TABLE()

MultiControllerUploadDialog::MultiControllerUploadDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _frame = (xLightsFrame*)parent;

	//(*Initialize(MultiControllerUploadDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer4;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(2);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Select all the controllers you want to upload to and the type of controller it is.\nThis upload will upload input and output definitions to each controller."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Controller Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice1 = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice1->SetSelection( Choice1->Append(_("Falcon")) );
	Choice1->Append(_("San Devices"));
	Choice1->Append(_("ESP Pixel Stick"));
	Choice1->Append(_("PixLite/PixCon"));
	FlexGridSizer2->Add(Choice1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	CheckListBox_Controllers = new wxCheckListBox(this, ID_CHECKLISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_ALWAYS_SB|wxVSCROLL, wxDefaultValidator, _T("ID_CHECKLISTBOX1"));
	FlexGridSizer1->Add(CheckListBox_Controllers, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Upload = new wxButton(this, ID_BUTTON1, _("Upload"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer4->Add(Button_Upload, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer4->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Log = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(0,300), wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Log, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHECKLISTBOX1,wxEVT_COMMAND_CHECKLISTBOX_TOGGLED,(wxObjectEventFunction)&MultiControllerUploadDialog::OnCheckListBox_ControllersToggled);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MultiControllerUploadDialog::OnButton_UploadClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MultiControllerUploadDialog::OnButton_CancelClick);
	//*)

    Connect(ID_CHECKLISTBOX1, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&MultiControllerUploadDialog::OnListRClick);

    auto outputs = _frame->GetOutputManager()->GetOutputs();
    for (auto it = outputs.begin(); it != outputs.end(); ++it)
    {
        if ((*it)->IsIpOutput() && (*it)->GetIP() != "MULTICAST" && !(*it)->IsAutoLayoutModels())
        {
            if (std::find(_ips.begin(), _ips.end(), (*it)->GetIP()) == _ips.end())
            {
                _ips.push_back((*it)->GetIP());
                CheckListBox_Controllers->AppendString((*it)->GetIP() + " " + (*it)->GetDescription());
            }
        }
    }

    Fit();

    ValidateWindow();
}

MultiControllerUploadDialog::~MultiControllerUploadDialog()
{
	//(*Destroy(MultiControllerUploadDialog)
	//*)
}

void MultiControllerUploadDialog::OnButton_UploadClick(wxCommandEvent& event)
{
    SetCursor(wxCURSOR_WAIT);

    Choice1->Disable();
    CheckListBox_Controllers->Disable();
    Button_Upload->Disable();
    Button_Cancel->Disable();

    // ensure all start channels etc are up to date
    _frame->RecalcModels();

    auto selected = Choice1->GetStringSelection();

    wxArrayInt ch;
    CheckListBox_Controllers->GetCheckedItems(ch);
    std::list<int> fake;

    for (int i = 0; i < ch.Count() && wxGetKeyState(WXK_ESCAPE) == false; i++)
    {
        wxString ip = _ips[ch[i]];
        TextCtrl_Log->AppendText("Processing " + ip + ".\n");
        if (selected == "Falcon")
        {
            Falcon falcon(ip.ToStdString());
            if (falcon.IsConnected()) {
                if (falcon.SetInputUniverses(_frame->GetOutputManager(), fake)) {
                    TextCtrl_Log->AppendText("Falcon Input Upload Complete to " + ip + ".\n");
                    if (falcon.SetOutputs(&_frame->AllModels, _frame->GetOutputManager(), fake, this)) {
                        TextCtrl_Log->AppendText("Falcon Output Upload Complete to " + ip + ".\n");
                    }
                    else {
                        TextCtrl_Log->AppendText("Falcon Output Upload FAILED to " + ip + ".\n");
                    }
                }
                else {
                    TextCtrl_Log->AppendText("Falcon Upload FAILED to " + ip + ".\n");
                }
            }
            else {
                TextCtrl_Log->AppendText("Falcon Output Upload FAILED to " + ip + ".\n");
            }
        }
        else if (selected == "San Devices")
        {
            // Input + Output
            SanDevices sanDevices(ip.ToStdString());
            if (sanDevices.IsConnected()) {
                if (sanDevices.SetInputUniverses(_frame->GetOutputManager(), fake)) {
                    TextCtrl_Log->AppendText("SanDevices Input Upload Complete to " + ip + ".\n");
                    if (sanDevices.SetOutputs(&_frame->AllModels, _frame->GetOutputManager(), fake, this)) {
                        TextCtrl_Log->AppendText("SanDevices Output Upload Complete to " + ip + ".\n");
                    }
                    else {
                        TextCtrl_Log->AppendText("SanDevices Output Upload FAILED to " + ip + ".\n");
                    }
                }
                else {
                    TextCtrl_Log->AppendText("SanDevices Input Upload FAILED to " + ip + ".\n");
                }
            }
            else {
                TextCtrl_Log->AppendText("SanDevices Upload FAILED to " + ip + ".\n");
            }
        }
        else if (selected == "ESP Pixel Stick")
        {
            // Output
            ESPixelStick esPixelStick(ip.ToStdString());
            if (esPixelStick.IsConnected()) {
                if (esPixelStick.SetOutputs(&_frame->AllModels, _frame->GetOutputManager(), fake, this)) {
                    TextCtrl_Log->AppendText("ES Pixel Stick Upload Complete to " + ip + ".\n");
                }
                else {
                    TextCtrl_Log->AppendText("ES Pixel Stick Upload FAILED to " + ip + ".\n");
                }
            }
            else {
                TextCtrl_Log->AppendText("ES Pixel Stick Upload FAILED to " + ip + ".\n");
            }
        }
        else if (selected == "PixLite/PixCon")
        {
            Pixlite16 pixlite(ip.ToStdString());
            if (pixlite.IsConnected()) {
                if (pixlite.SetOutputs(&_frame->AllModels, _frame->GetOutputManager(), fake, this)) {
                    TextCtrl_Log->AppendText("PixLite/PixCon Upload Complete to " + ip + ".\n");
                }
                else {
                    TextCtrl_Log->AppendText("PixLite/PixCon Upload FAILED to " + ip + ".\n");
                }
            }
            else {
                TextCtrl_Log->AppendText("PixLite/PixCon Upload FAILED to " + ip + ".\n");
            }
        }
    }

    Choice1->Enable();
    CheckListBox_Controllers->Enable();
    Button_Upload->Enable();
    Button_Cancel->Enable();
    SetCursor(wxCURSOR_ARROW);
}

void MultiControllerUploadDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CLOSE);
}

void MultiControllerUploadDialog::OnCheckListBox_ControllersToggled(wxCommandEvent& event)
{
    ValidateWindow();
}

void MultiControllerUploadDialog::ValidateWindow()
{
    wxArrayInt ci;
    CheckListBox_Controllers->GetCheckedItems(ci);
    if (ci.Count() > 0)
    {
        Button_Upload->Enable();
    }
    else
    {
        Button_Upload->Disable();
    }
}

void MultiControllerUploadDialog::OnListRClick(wxContextMenuEvent& event)
{
    wxMenu mnu;
    mnu.Append(ID_MCU_SELECTALL, "Select All");
    mnu.Append(ID_MCU_SELECTNONE, "Select None");

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&MultiControllerUploadDialog::OnPopup, nullptr, this);
    PopupMenu(&mnu);
}

void MultiControllerUploadDialog::OnPopup(wxCommandEvent& event)
{
    if (event.GetId() == ID_MCU_SELECTALL)
    {
        for (size_t i = 0; i < CheckListBox_Controllers->GetCount(); i++)
        {
            CheckListBox_Controllers->Check(i);
        }
    }
    else if (event.GetId() == ID_MCU_SELECTNONE)
    {
        for (size_t i = 0; i < CheckListBox_Controllers->GetCount(); i++)
        {
            CheckListBox_Controllers->Check(i, false);
        }
    }
}