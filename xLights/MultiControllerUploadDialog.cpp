/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "MultiControllerUploadDialog.h"

//(*InternalHeaders(MultiControllerUploadDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/listbase.h>

#include "xLightsMain.h"
#include "controllers/ControllerCaps.h"
#include "outputs/ControllerEthernet.h"

#include "utils/ip_utils.h"

//(*IdInit(MultiControllerUploadDialog)
const long MultiControllerUploadDialog::ID_STATICTEXT1 = wxNewId();
const long MultiControllerUploadDialog::ID_CHECKLISTBOX1 = wxNewId();
const long MultiControllerUploadDialog::ID_BUTTON1 = wxNewId();
const long MultiControllerUploadDialog::ID_BUTTON2 = wxNewId();
const long MultiControllerUploadDialog::ID_TEXTCTRL1 = wxNewId();
//*)

const long MultiControllerUploadDialog::ID_MCU_SELECTALL = wxNewId();
const long MultiControllerUploadDialog::ID_MCU_SELECTNONE = wxNewId();
const long MultiControllerUploadDialog::ID_MCU_SELECTACTIVE = wxNewId();
const long MultiControllerUploadDialog::ID_MCU_DESELECTINACTIVE = wxNewId();
const long MultiControllerUploadDialog::ID_MCU_SELECTAUTO = wxNewId();

BEGIN_EVENT_TABLE(MultiControllerUploadDialog, wxDialog)
	//(*EventTable(MultiControllerUploadDialog)
	//*)
END_EVENT_TABLE()


MultiControllerUploadDialog::MultiControllerUploadDialog(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    _frame = (xLightsFrame*)parent;

    //(*Initialize(MultiControllerUploadDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer4;

    Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(1);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Select all the controllers you want to upload to and the type of controller it is.\nThis upload will upload input and output definitions to each controller."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
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

    auto controllers = _frame->GetOutputManager()->GetControllers();
    for (const auto& it : controllers)
    {
        auto eth = it;
        if (eth != nullptr && eth->SupportsUpload() && eth->GetResolvedIP() != "MULTICAST") {
            auto caps = ControllerCaps::GetControllerConfig(eth->GetVendor(), eth->GetModel(), eth->GetVariant());
            if (caps && caps->SupportsUpload()) {
                _controllers.push_back(eth);

                if (eth->GetFPPProxy() != "") {
                    CheckListBox_Controllers->AppendString(eth->GetIP() + " (via FPP " + eth->GetFPPProxy() + ") " + eth->GetDescription() + " " + eth->GetName());
                } else {
                    CheckListBox_Controllers->AppendString(eth->GetIP() + " " + eth->GetDescription() + " " + eth->GetName());
                }
            }
        }
    }
    LoadChecked();
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

    CheckListBox_Controllers->Disable();
    Button_Upload->Disable();
    Button_Cancel->Disable();

    // ensure all start channels etc are up to date
    _frame->RecalcModels();

    wxArrayInt ch;
    CheckListBox_Controllers->GetCheckedItems(ch);
    std::list<int> fake;

    for (int i = 0; i < ch.Count() && wxGetKeyState(WXK_ESCAPE) == false; i++) {
        auto c = _controllers[ch[i]];
        wxString message;
        TextCtrl_Log->AppendText("Uploading to controller '" + c->GetName() + "' [" + c->GetIP() + "] " + c->GetVMV() + "\n");
        _frame->UploadInputToController(c, message);
        TextCtrl_Log->AppendText(message);
        TextCtrl_Log->AppendText("\n");
        _frame->UploadOutputToController(c, message);
        TextCtrl_Log->AppendText(message);
        TextCtrl_Log->AppendText("\n");
        TextCtrl_Log->AppendText("    Done.");
	TextCtrl_Log->AppendText("\n");
    }

    CheckListBox_Controllers->Enable();
    Button_Upload->Enable();
    Button_Cancel->Enable();
    SetCursor(wxCURSOR_ARROW);
}

void MultiControllerUploadDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    SaveChecked();
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
    Button_Upload->Enable(ci.Count() > 0);
}

void MultiControllerUploadDialog::OnListRClick(wxContextMenuEvent& event)
{
    wxMenu mnu;
    mnu.Append(ID_MCU_SELECTALL, "Select All");
    mnu.Append(ID_MCU_SELECTNONE, "Select None");
    mnu.Append(ID_MCU_SELECTACTIVE, "Select Active");
    mnu.Append(ID_MCU_SELECTAUTO, "Select Auto Config");
    mnu.Append(ID_MCU_DESELECTINACTIVE, "Deselect Inactive");

    std::vector<std::string> proxies;
    for (auto* c : _controllers) {
        auto controllerproxy = c->GetFPPProxy();
        if (!controllerproxy.empty()) {
            if (std::find(proxies.begin(), proxies.end(), controllerproxy) == proxies.end()) {
                proxies.push_back(controllerproxy);
            }
        }
    }
    if (!proxies.empty()) {
        std::sort(proxies.begin(), proxies.end());
        wxMenu* srMenu = new wxMenu();
        for (auto p : proxies) {
            srMenu->Append(wxNewId(), wxString(p));
        }
        srMenu->Connect(wxEVT_MENU, (wxObjectEventFunction)&MultiControllerUploadDialog::OnProxyPopup, nullptr, this);
        mnu.AppendSubMenu(srMenu, "Select with Proxy");
    }

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&MultiControllerUploadDialog::OnPopup, nullptr, this);
    PopupMenu(&mnu);
}

void MultiControllerUploadDialog::OnPopup(wxCommandEvent& event)
{
    if (event.GetId() == ID_MCU_SELECTALL) {
        for (size_t i = 0; i < CheckListBox_Controllers->GetCount(); i++) {
            CheckListBox_Controllers->Check(i);
        }
        ValidateWindow();
    } else if (event.GetId() == ID_MCU_SELECTNONE) {
        for (size_t i = 0; i < CheckListBox_Controllers->GetCount(); i++) {
            CheckListBox_Controllers->Check(i, false);
        }
        ValidateWindow();
    } else if (event.GetId() == ID_MCU_SELECTACTIVE) {
        for (size_t i = 0; i < CheckListBox_Controllers->GetCount(); i++) {
            if (_controllers[i]->IsActive()) {
                CheckListBox_Controllers->Check(i);
            }
            
        }
        ValidateWindow();
    } else if (event.GetId() == ID_MCU_DESELECTINACTIVE) {
        for (size_t i = 0; i < CheckListBox_Controllers->GetCount(); i++) {
            if (!_controllers[i]->IsActive()) {
                CheckListBox_Controllers->Check(i, false);
            }
        }
        ValidateWindow();
    } else if (event.GetId() == ID_MCU_SELECTAUTO) {
        for (size_t i = 0; i < CheckListBox_Controllers->GetCount(); i++) {
            if (_controllers[i]->IsAutoLayout()) {
                CheckListBox_Controllers->Check(i);
            }
        }
        ValidateWindow();
    }
}

void MultiControllerUploadDialog::OnProxyPopup(wxCommandEvent& event)
{
    auto id = event.GetId();
    wxString label = ((wxMenu*)event.GetEventObject())->GetLabelText(id);
    for (size_t i = 0; i < CheckListBox_Controllers->GetCount(); i++) {
        if (!_controllers[i] || _controllers[i]->GetFPPProxy().empty()) {
            continue;
        }
        if (label.compare(_controllers[i]->GetFPPProxy()) == 0) {
            CheckListBox_Controllers->Check(i);
        }
    }
    ValidateWindow();
}

void MultiControllerUploadDialog::SaveChecked()
{
    wxArrayInt ch;
    CheckListBox_Controllers->GetCheckedItems(ch);
    std::list<int> fake;
    std::vector<std::string> selected_controllers;
    for (int i = 0; i < ch.Count() ; i++) {
        auto c = _controllers[ch[i]];
        selected_controllers.push_back(c->GetIP());
    }

    wxConfigBase* config = wxConfigBase::Get();
    config->Write("MultiControllerUploadSelection", wxString(Join(selected_controllers, ",")));
    config->Flush();
}

void MultiControllerUploadDialog::LoadChecked()
{
    wxConfigBase* config = wxConfigBase::Get();

    if (config != nullptr) {
        wxString controllerSelect = "";

        config->Read("MultiControllerUploadSelection", &controllerSelect);
        std::vector<std::string> selected_controllers = Split(controllerSelect, ',');
        for (size_t i = 0; i < CheckListBox_Controllers->GetCount(); i++) {
            auto c = _controllers[i];
            if (std::find(selected_controllers.begin(), selected_controllers.end(), c->GetIP()) != selected_controllers.end()) {
                CheckListBox_Controllers->Check(i);
            }
        }
    }
}
