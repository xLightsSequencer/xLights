/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MultiControllerUploadDialog.h"

//(*InternalHeaders(MultiControllerUploadDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/listbase.h>
#include <wx/bitmap.h>
#include <wx/bmpbndl.h>
#include <wx/colour.h>
#include <wx/gdicmn.h>
#include <wx/dcmemory.h>
#include <wx/pen.h>

#include <algorithm>

#include "xLightsMain.h"
#include "settings/XLightsConfigAdapter.h"
#include "controllers/ControllerCaps.h"
#include "outputs/ControllerEthernet.h"

#include "utils/ip_utils.h"

namespace {

wxBitmap CreateUploadResultBitmap(const wxColour& color, bool checkmark, bool cross, int size)
{
    wxBitmap bmp = ControllerTree::CreateLedBitmap(color, size);
    wxMemoryDC dc(bmp);
    dc.SetPen(wxPen(*wxWHITE, std::max(1, size / 8)));
    if (checkmark) {
        dc.DrawLine(size * 26 / 100, size * 52 / 100, size * 42 / 100, size * 70 / 100);
        dc.DrawLine(size * 42 / 100, size * 70 / 100, size * 76 / 100, size * 28 / 100);
    } else if (cross) {
        dc.DrawLine(size * 28 / 100, size * 28 / 100, size * 72 / 100, size * 72 / 100);
        dc.DrawLine(size * 72 / 100, size * 28 / 100, size * 28 / 100, size * 72 / 100);
    }
    dc.SelectObject(wxNullBitmap);
    return bmp;
}

wxBitmapBundle CreateUploadResultBitmapBundle(const wxColour& color, bool checkmark, bool cross, int size)
{
    wxVector<wxBitmap> bitmaps;
    for (int scale = 1; scale <= 3; ++scale) {
        bitmaps.push_back(CreateUploadResultBitmap(color, checkmark, cross, size * scale));
    }
    return wxBitmapBundle::FromBitmaps(bitmaps);
}

} // namespace

//(*IdInit(MultiControllerUploadDialog)
const long MultiControllerUploadDialog::ID_STATICTEXT1 = wxNewId();
const long MultiControllerUploadDialog::ID_LISTCTRL_CONTROLLERS = wxNewId();
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
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Select all the controllers you want to upload to."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
    ListCtrl_Controllers = new wxListCtrl(this, ID_LISTCTRL_CONTROLLERS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL|wxVSCROLL, wxDefaultValidator, _T("ID_LISTCTRL_CONTROLLERS"));
    FlexGridSizer1->Add(ListCtrl_Controllers, 1, wxALL|wxEXPAND, 5);
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

    Connect(ID_LISTCTRL_CONTROLLERS,wxEVT_LIST_ITEM_CHECKED,(wxObjectEventFunction)&MultiControllerUploadDialog::OnListCtrl_ControllersItemChecked);
    Connect(ID_LISTCTRL_CONTROLLERS,wxEVT_LIST_ITEM_UNCHECKED,(wxObjectEventFunction)&MultiControllerUploadDialog::OnListCtrl_ControllersItemChecked);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MultiControllerUploadDialog::OnButton_UploadClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MultiControllerUploadDialog::OnButton_CancelClick);
    //*)

    Connect(ID_LISTCTRL_CONTROLLERS, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&MultiControllerUploadDialog::OnListRClick);

    const int RESULT_COLUMN_WIDTH = FromDIP(26);
    ListCtrl_Controllers->EnableCheckBoxes(true);
    ListCtrl_Controllers->AppendColumn("");
    ListCtrl_Controllers->AppendColumn("");
    ListCtrl_Controllers->SetColumnWidth(1, RESULT_COLUMN_WIDTH);
    auto stretchNameColumn = [this]() {
        if (ListCtrl_Controllers == nullptr) return;
        int w = ListCtrl_Controllers->GetClientSize().GetWidth() - RESULT_COLUMN_WIDTH;
        if (w > 100) {
            ListCtrl_Controllers->SetColumnWidth(0, w);
        }
    };
    
    ListCtrl_Controllers->Bind(wxEVT_SIZE, [stretchNameColumn](wxSizeEvent& evt) {
        evt.Skip();
        stretchNameColumn();
    });

    wxVector<wxBitmapBundle> images;
    _pingIcons = ControllerTree::AppendControllerLedIcons(images);
    _resultIconProcessing = (int)images.size();
    images.push_back(ControllerTree::CreateLedBitmapBundle(wxColour(230, 160, 20), 16));
    _resultIconSuccess = (int)images.size();
    images.push_back(CreateUploadResultBitmapBundle(wxColour(40, 180, 40), true, false, 16));
    _resultIconFailure = (int)images.size();
    images.push_back(CreateUploadResultBitmapBundle(wxColour(210, 40, 40), false, true, 16));
    ListCtrl_Controllers->SetSmallImages(images);

    auto controllers = _frame->GetOutputManager()->GetControllers();
    for (const auto& it : controllers)
    {
        auto eth = it;
        if (eth != nullptr && eth->SupportsUpload() && eth->GetResolvedIP() != "MULTICAST") {
            auto caps = ControllerCaps::GetControllerConfig(eth->GetVendor(), eth->GetModel(), eth->GetVariant());
            if (caps && caps->SupportsUpload()) {
                _controllers.push_back(eth);

                wxString label;
                if (eth->GetFPPProxy() != "") {
                    label = eth->GetIP() + " (via FPP " + eth->GetFPPProxy() + ") " + eth->GetDescription() + " " + eth->GetName();
                } else {
                    label = eth->GetIP() + " " + eth->GetDescription() + " " + eth->GetName();
                }
                long row = ListCtrl_Controllers->InsertItem(ListCtrl_Controllers->GetItemCount(), label);
                ListCtrl_Controllers->SetItemColumnImage(row, 0, -1);
            }
        }
    }

    RefreshPingIcons();
    _pingRefreshTimer.SetOwner(this);
    Bind(wxEVT_TIMER, [this](wxTimerEvent& WXUNUSED(evt)) { RefreshPingIcons(); }, _pingRefreshTimer.GetId());
    _pingRefreshTimer.Start(1000);

    LoadChecked();
    Fit();
    ValidateWindow();
}

MultiControllerUploadDialog::~MultiControllerUploadDialog()
{
	//(*Destroy(MultiControllerUploadDialog)
	//*)
    _pingRefreshTimer.Stop();
}

void MultiControllerUploadDialog::RefreshPingIcons()
{
    for (size_t i = 0; i < _controllers.size(); i++) {
        int idx = _pingIcons.idxGray;
        switch (ControllerTree::ClassifyControllerPing(_controllers[i])) {
        case ControllerTree::ControllerPingBucket::Green: idx = _pingIcons.idxGreen; break;
        case ControllerTree::ControllerPingBucket::Red: idx = _pingIcons.idxRed; break;
        default: break;
        }
        ListCtrl_Controllers->SetItemColumnImage((long)i, 0, idx);
    }
}

void MultiControllerUploadDialog::OnButton_UploadClick(wxCommandEvent& event)
{
    SetCursor(wxCURSOR_WAIT);

    ListCtrl_Controllers->Disable();
    Button_Upload->Disable();
    Button_Cancel->Disable();

    // ensure all start channels etc are up to date
    _frame->RecalcModels();

    wxArrayInt ch = GetCheckedRows();

    for (int i = 0; i < (int)ch.Count() && wxGetKeyState(WXK_ESCAPE) == false; i++) {
        int row = ch[i];
        auto c = _controllers[row];
        wxString message;

        ListCtrl_Controllers->SetItemColumnImage(row, 1, _resultIconProcessing);
        ListCtrl_Controllers->Refresh();
        ListCtrl_Controllers->Update();

        TextCtrl_Log->AppendText("Uploading to controller '" + c->GetName() + "' [" + c->GetIP() + "] " + c->GetVMV() + "\n");
        _frame->UploadInputToController(c, message);
        TextCtrl_Log->AppendText(message);
        TextCtrl_Log->AppendText("\n");
        bool outputOk = _frame->UploadOutputToController(c, message);
        TextCtrl_Log->AppendText(message);
        TextCtrl_Log->AppendText("\n");
        TextCtrl_Log->AppendText("    Done.");
	TextCtrl_Log->AppendText("\n");

        ListCtrl_Controllers->SetItemColumnImage(row, 1, outputOk ? _resultIconSuccess : _resultIconFailure);
    }

    ListCtrl_Controllers->Enable();
    Button_Upload->Enable();
    Button_Cancel->Enable();
    SetCursor(wxCURSOR_ARROW);
}

void MultiControllerUploadDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    SaveChecked();
    EndDialog(wxID_CLOSE);
}

void MultiControllerUploadDialog::OnListCtrl_ControllersItemChecked(wxListEvent& event)
{
    ValidateWindow();
}

wxArrayInt MultiControllerUploadDialog::GetCheckedRows() const
{
    wxArrayInt result;
    for (long i = 0; i < ListCtrl_Controllers->GetItemCount(); i++) {
        if (ListCtrl_Controllers->IsItemChecked(i)) {
            result.Add(i);
        }
    }
    return result;
}

void MultiControllerUploadDialog::ValidateWindow()
{
    Button_Upload->Enable(GetCheckedRows().Count() > 0);
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
        for (long i = 0; i < ListCtrl_Controllers->GetItemCount(); i++) {
            ListCtrl_Controllers->CheckItem(i, true);
        }
        ValidateWindow();
    } else if (event.GetId() == ID_MCU_SELECTNONE) {
        for (long i = 0; i < ListCtrl_Controllers->GetItemCount(); i++) {
            ListCtrl_Controllers->CheckItem(i, false);
        }
        ValidateWindow();
    } else if (event.GetId() == ID_MCU_SELECTACTIVE) {
        for (long i = 0; i < ListCtrl_Controllers->GetItemCount(); i++) {
            if (_controllers[i]->IsActive()) {
                ListCtrl_Controllers->CheckItem(i, true);
            }

        }
        ValidateWindow();
    } else if (event.GetId() == ID_MCU_DESELECTINACTIVE) {
        for (long i = 0; i < ListCtrl_Controllers->GetItemCount(); i++) {
            if (!_controllers[i]->IsActive()) {
                ListCtrl_Controllers->CheckItem(i, false);
            }
        }
        ValidateWindow();
    } else if (event.GetId() == ID_MCU_SELECTAUTO) {
        for (long i = 0; i < ListCtrl_Controllers->GetItemCount(); i++) {
            if (_controllers[i]->IsAutoLayout()) {
                ListCtrl_Controllers->CheckItem(i, true);
            }
        }
        ValidateWindow();
    }
}

void MultiControllerUploadDialog::OnProxyPopup(wxCommandEvent& event)
{
    auto id = event.GetId();
    wxString label = ((wxMenu*)event.GetEventObject())->GetLabelText(id);
    for (long i = 0; i < ListCtrl_Controllers->GetItemCount(); i++) {
        if (!_controllers[i] || _controllers[i]->GetFPPProxy().empty()) {
            continue;
        }
        if (label.compare(_controllers[i]->GetFPPProxy()) == 0) {
            ListCtrl_Controllers->CheckItem(i, true);
        }
    }
    ValidateWindow();
}

void MultiControllerUploadDialog::SaveChecked()
{
    wxArrayInt ch = GetCheckedRows();
    std::vector<std::string> selected_controllers;
    for (int i = 0; i < (int)ch.Count() ; i++) {
        auto c = _controllers[ch[i]];
        selected_controllers.push_back(c->GetIP());
    }

    auto* config = GetXLightsConfig();
    config->Write("MultiControllerUploadSelection", wxString(Join(selected_controllers, ",")));
    config->Flush();
}

void MultiControllerUploadDialog::LoadChecked()
{
    auto* config = GetXLightsConfig();

    if (config != nullptr) {
        wxString controllerSelect = "";

        config->Read("MultiControllerUploadSelection", &controllerSelect);
        std::vector<std::string> selected_controllers = Split(controllerSelect, ',');
        for (long i = 0; i < ListCtrl_Controllers->GetItemCount(); i++) {
            auto c = _controllers[i];
            if (std::find(selected_controllers.begin(), selected_controllers.end(), c->GetIP()) != selected_controllers.end()) {
                ListCtrl_Controllers->CheckItem(i, true);
            }
        }
    }
}
