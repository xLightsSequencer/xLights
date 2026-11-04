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
const long MultiControllerUploadDialog::ID_MCU_CHECKSELECTED = wxNewId();
const long MultiControllerUploadDialog::ID_MCU_UNCHECKSELECTED = wxNewId();
const long MultiControllerUploadDialog::ID_MCU_INVERTSELECTION = wxNewId();

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
    ListCtrl_Controllers = new wxListCtrl(this, ID_LISTCTRL_CONTROLLERS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER|wxVSCROLL, wxDefaultValidator, _T("ID_LISTCTRL_CONTROLLERS"));
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

    auto* headerRow = new wxBoxSizer(wxHORIZONTAL);
    _selectAllCB = new wxCheckBox(this, wxID_ANY, _("Select All"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE);
    _selectAllCB->SetToolTip(_("Select / clear all controllers shown in the list"));
    headerRow->Add(_selectAllCB, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
    headerRow->Add(new wxStaticText(this, wxID_ANY, _("Filter:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    _filterCtrl = new wxSearchCtrl(this, wxID_ANY);
    _filterCtrl->ShowCancelButton(true);
    _filterCtrl->SetDescriptiveText(_("name, IP or description"));
    headerRow->Add(_filterCtrl, 1, wxALIGN_CENTER_VERTICAL);
    FlexGridSizer1->Insert(1, headerRow, 0, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 5);
    FlexGridSizer1->RemoveGrowableRow(1);
    FlexGridSizer1->AddGrowableRow(2);

    _selectAllCB->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) {
        const bool check = _selectAllCB->Get3StateValue() != wxCHK_UNCHECKED;
        for (long i = 0; i < ListCtrl_Controllers->GetItemCount(); i++) {
            CheckRow(i, check);
        }
        ValidateWindow();
    });
    _filterTimer.SetOwner(this);
    Bind(wxEVT_TIMER, [this](wxTimerEvent&) { RebuildList(); }, _filterTimer.GetId());
    _filterCtrl->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { _filterTimer.StartOnce(200); });
    _filterCtrl->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, [this](wxCommandEvent&) {
        _filterCtrl->ChangeValue(wxEmptyString);
        _filterTimer.Stop();
        RebuildList();
    });

    const int RESULT_COLUMN_WIDTH = FromDIP(26);
    ListCtrl_Controllers->EnableCheckBoxes(true);
    ListCtrl_Controllers->AppendColumn("");
    ListCtrl_Controllers->AppendColumn("");
    ListCtrl_Controllers->SetColumnWidth(1, RESULT_COLUMN_WIDTH);
    auto stretchNameColumn = [this, RESULT_COLUMN_WIDTH]() {
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

                // GetControllerFPPProxy() returns the raw, unresolved proxy string. Using
                // GetFPPProxy() here instead triggers a synchronous DNS lookup per controller
                // (uncached on failure, ~5s timeout each) just to build a display label.
                auto fppProxy = eth->GetControllerFPPProxy();
                wxString label;
                if (!fppProxy.empty()) {
                    label = eth->GetIP() + " (via FPP " + fppProxy + ") " + eth->GetDescription() + " " + eth->GetName();
                } else {
                    label = eth->GetIP() + " " + eth->GetDescription() + " " + eth->GetName();
                }
                _labels.push_back(label);
            }
        }
    }
    _checked.assign(_controllers.size(), false);
    _resultIcons.assign(_controllers.size(), -1);

    LoadChecked();
    RebuildList();
    _pingRefreshTimer.SetOwner(this);
    Bind(wxEVT_TIMER, [this](wxTimerEvent& WXUNUSED(evt)) { RefreshPingIcons(); }, _pingRefreshTimer.GetId());
    _pingRefreshTimer.Start(1000);

    Fit();
    ValidateWindow();
}

MultiControllerUploadDialog::~MultiControllerUploadDialog()
{
	//(*Destroy(MultiControllerUploadDialog)
	//*)
    _pingRefreshTimer.Stop();
    _filterTimer.Stop();
}

bool MultiControllerUploadDialog::MatchesFilter(size_t idx, const wxArrayString& tokens) const
{
    if (tokens.empty()) {
        return true;
    }
    const wxString label = _labels[idx].Lower();
    for (const auto& token : tokens) {
        if (!token.empty() && !label.Contains(token)) {
            return false;
        }
    }
    return true;
}

// Checked controllers stay listed whatever the filter, so nothing hidden gets uploaded.
void MultiControllerUploadDialog::RebuildList()
{
    _rebuilding = true;
    ListCtrl_Controllers->Freeze();
    ListCtrl_Controllers->DeleteAllItems();
    _rows.clear();
    const wxString filter = _filterCtrl->GetValue().Lower().Trim().Trim(false);
    const wxArrayString tokens = filter.empty() ? wxArrayString() : wxSplit(filter, ' ');
    for (size_t i = 0; i < _controllers.size(); i++) {
        if (!_checked[i] && !MatchesFilter(i, tokens)) {
            continue;
        }
        long row = ListCtrl_Controllers->InsertItem(ListCtrl_Controllers->GetItemCount(), _labels[i]);
        ListCtrl_Controllers->SetItemColumnImage(row, 0, -1);
        ListCtrl_Controllers->SetItemColumnImage(row, 1, _resultIcons[i]);
        ListCtrl_Controllers->CheckItem(row, _checked[i]);
        _rows.push_back((int)i);
    }
    ListCtrl_Controllers->Thaw();
    _rebuilding = false;
    RefreshPingIcons();
    ValidateWindow();
}

void MultiControllerUploadDialog::CheckRow(long row, bool checked)
{
    if (row < 0 || row >= (long)_rows.size()) {
        return;
    }
    _checked[_rows[row]] = checked;
    ListCtrl_Controllers->CheckItem(row, checked);
}

long MultiControllerUploadDialog::RowForController(int idx) const
{
    auto it = std::find(_rows.begin(), _rows.end(), idx);
    return it == _rows.end() ? -1 : (long)(it - _rows.begin());
}

void MultiControllerUploadDialog::UpdateSelectAll()
{
    size_t checkedRows = 0;
    for (int idx : _rows) {
        if (_checked[idx]) {
            ++checkedRows;
        }
    }
    _selectAllCB->Enable(!_rows.empty());
    if (checkedRows == 0) {
        _selectAllCB->Set3StateValue(wxCHK_UNCHECKED);
    } else if (checkedRows == _rows.size()) {
        _selectAllCB->Set3StateValue(wxCHK_CHECKED);
    } else {
        _selectAllCB->Set3StateValue(wxCHK_UNDETERMINED);
    }
}

void MultiControllerUploadDialog::RefreshPingIcons()
{
    for (size_t row = 0; row < _rows.size(); row++) {
        int idx = _pingIcons.idxGray;
        switch (ControllerTree::ClassifyControllerPing(_controllers[_rows[row]])) {
        case ControllerTree::ControllerPingBucket::Green: idx = _pingIcons.idxGreen; break;
        case ControllerTree::ControllerPingBucket::Red: idx = _pingIcons.idxRed; break;
        default: break;
        }
        ListCtrl_Controllers->SetItemColumnImage((long)row, 0, idx);
    }
}

void MultiControllerUploadDialog::OnButton_UploadClick(wxCommandEvent& event)
{
    SetCursor(wxCURSOR_WAIT);

    ListCtrl_Controllers->Disable();
    Button_Upload->Disable();
    Button_Cancel->Disable();
    _filterCtrl->Disable();
    _selectAllCB->Disable();

    // ensure all start channels etc are up to date
    _frame->RecalcModels();

    const std::vector<int> ch = GetCheckedControllers();

    for (size_t i = 0; i < ch.size() && wxGetKeyState(WXK_ESCAPE) == false; i++) {
        const int idx = ch[i];
        const long row = RowForController(idx);
        auto c = _controllers[idx];
        wxString message;

        _resultIcons[idx] = _resultIconProcessing;
        if (row >= 0) {
            ListCtrl_Controllers->SetItemColumnImage(row, 1, _resultIconProcessing);
            ListCtrl_Controllers->Refresh();
            ListCtrl_Controllers->Update();
        }

        TextCtrl_Log->AppendText("Uploading to controller '" + c->GetName() + "' [" + c->GetIP() + "] " + c->GetVMV() + "\n");
        _frame->UploadInputToController(c, message);
        TextCtrl_Log->AppendText(message);
        TextCtrl_Log->AppendText("\n");
        bool outputOk = _frame->UploadOutputToController(c, message);
        TextCtrl_Log->AppendText(message);
        TextCtrl_Log->AppendText("\n");
        TextCtrl_Log->AppendText("    Done.");
	TextCtrl_Log->AppendText("\n");

        _resultIcons[idx] = outputOk ? _resultIconSuccess : _resultIconFailure;
        if (row >= 0) {
            ListCtrl_Controllers->SetItemColumnImage(row, 1, _resultIcons[idx]);
        }
    }

    ListCtrl_Controllers->Enable();
    Button_Upload->Enable();
    Button_Cancel->Enable();
    _filterCtrl->Enable();
    UpdateSelectAll();
    SetCursor(wxCURSOR_ARROW);
}

void MultiControllerUploadDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    SaveChecked();
    EndDialog(wxID_CLOSE);
}

void MultiControllerUploadDialog::OnListCtrl_ControllersItemChecked(wxListEvent& event)
{
    if (_rebuilding) {
        return;
    }
    const long row = event.GetIndex();
    if (row >= 0 && row < (long)_rows.size()) {
        const bool checked = ListCtrl_Controllers->IsItemChecked(row);
        _checked[_rows[row]] = checked;
        // Ticking one row of a multi-row selection applies to the whole selection.
        if (ListCtrl_Controllers->GetItemState(row, wxLIST_STATE_SELECTED) != 0 && ListCtrl_Controllers->GetSelectedItemCount() > 1) {
            _rebuilding = true;
            for (long i = ListCtrl_Controllers->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED); i != -1;
                 i = ListCtrl_Controllers->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) {
                CheckRow(i, checked);
            }
            _rebuilding = false;
        }
    }
    ValidateWindow();
}

std::vector<int> MultiControllerUploadDialog::GetCheckedControllers() const
{
    std::vector<int> result;
    for (size_t i = 0; i < _checked.size(); i++) {
        if (_checked[i]) {
            result.push_back((int)i);
        }
    }
    return result;
}

void MultiControllerUploadDialog::ValidateWindow()
{
    Button_Upload->Enable(!GetCheckedControllers().empty());
    UpdateSelectAll();
}

void MultiControllerUploadDialog::OnListRClick(wxContextMenuEvent& event)
{
    wxMenu mnu;
    if (ListCtrl_Controllers->GetSelectedItemCount() > 0) {
        mnu.Append(ID_MCU_CHECKSELECTED, _("Check Selected"));
        mnu.Append(ID_MCU_UNCHECKSELECTED, _("Uncheck Selected"));
        mnu.AppendSeparator();
    }
    mnu.Append(ID_MCU_SELECTALL, "Select All");
    mnu.Append(ID_MCU_SELECTNONE, "Select None");
    mnu.Append(ID_MCU_INVERTSELECTION, _("Invert Selection"));
    mnu.Append(ID_MCU_SELECTACTIVE, "Select Active");
    mnu.Append(ID_MCU_SELECTAUTO, "Select Auto Config");
    mnu.Append(ID_MCU_DESELECTINACTIVE, "Deselect Inactive");

    std::vector<std::string> proxies;
    for (auto* c : _controllers) {
        // Raw, unresolved proxy string - avoids a synchronous DNS lookup per
        // controller (uncached on failure, ~5s timeout) just to build this menu.
        auto controllerproxy = c->GetControllerFPPProxy();
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
    if (event.GetId() == ID_MCU_CHECKSELECTED || event.GetId() == ID_MCU_UNCHECKSELECTED) {
        const bool check = event.GetId() == ID_MCU_CHECKSELECTED;
        for (long i = ListCtrl_Controllers->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED); i != -1;
             i = ListCtrl_Controllers->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) {
            CheckRow(i, check);
        }
        ValidateWindow();
    } else if (event.GetId() == ID_MCU_SELECTALL) {
        for (long i = 0; i < ListCtrl_Controllers->GetItemCount(); i++) {
            CheckRow(i, true);
        }
        ValidateWindow();
    } else if (event.GetId() == ID_MCU_INVERTSELECTION) {
        for (long i = 0; i < ListCtrl_Controllers->GetItemCount(); i++) {
            CheckRow(i, !_checked[_rows[i]]);
        }
        ValidateWindow();
    } else if (event.GetId() == ID_MCU_SELECTNONE) {
        for (long i = 0; i < ListCtrl_Controllers->GetItemCount(); i++) {
            CheckRow(i, false);
        }
        ValidateWindow();
    } else if (event.GetId() == ID_MCU_SELECTACTIVE) {
        for (long i = 0; i < ListCtrl_Controllers->GetItemCount(); i++) {
            if (_controllers[_rows[i]]->IsActive()) {
                CheckRow(i, true);
            }

        }
        ValidateWindow();
    } else if (event.GetId() == ID_MCU_DESELECTINACTIVE) {
        for (long i = 0; i < ListCtrl_Controllers->GetItemCount(); i++) {
            if (!_controllers[_rows[i]]->IsActive()) {
                CheckRow(i, false);
            }
        }
        ValidateWindow();
    } else if (event.GetId() == ID_MCU_SELECTAUTO) {
        for (long i = 0; i < ListCtrl_Controllers->GetItemCount(); i++) {
            if (_controllers[_rows[i]]->IsAutoLayout()) {
                CheckRow(i, true);
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
        if (!_controllers[_rows[i]] || _controllers[_rows[i]]->GetControllerFPPProxy().empty()) {
            continue;
        }
        if (label.compare(_controllers[_rows[i]]->GetControllerFPPProxy()) == 0) {
            CheckRow(i, true);
        }
    }
    ValidateWindow();
}

void MultiControllerUploadDialog::SaveChecked()
{
    std::vector<std::string> selected_controllers;
    for (int idx : GetCheckedControllers()) {
        selected_controllers.push_back(_controllers[idx]->GetIP());
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
        for (size_t i = 0; i < _controllers.size(); i++) {
            if (std::find(selected_controllers.begin(), selected_controllers.end(), _controllers[i]->GetIP()) != selected_controllers.end()) {
                _checked[i] = true;
            }
        }
    }
}
