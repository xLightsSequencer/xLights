/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ControllerFullColumnsDialog.h"

#include "shared/utils/wxUtilities.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/treelist.h>

#include "xLightsMain.h"
#include "layout/ControllerTreeUtils.h"

#include "layout/LayoutUtils.h"
#include "outputs/Controller.h"
#include "outputs/OutputManager.h"

using namespace ControllerTree;

namespace {
const long ID_TREELIST_ALL_CONTROLLER_COLUMNS = wxNewId();
const long ID_BTN_CLOSE_ALL_CONTROLLER_COLUMNS = wxNewId();
} // namespace

ControllerFullColumnsDialog::ControllerFullColumnsDialog(wxWindow* parent, xLightsFrame* frame) :
    wxDialog(parent, wxID_ANY, "All Controller Columns", wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    _frame(frame) {

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    CreateTree();
    sizer->Add(_tree, 1, wxEXPAND | wxALL, 4);

    wxButton* closeBtn = new wxButton(this, ID_BTN_CLOSE_ALL_CONTROLLER_COLUMNS, "Close");
    closeBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { EndModal(wxID_CLOSE); });
    sizer->Add(closeBtn, 0, wxALIGN_RIGHT | wxALL, 6);

    SetSizer(sizer);

    wxPoint loc;
    wxSize sz;
    LoadWindowPosition("ControllerFullColumnsDialogPosition", sz, loc);
    if (loc.x != -1) {
        SetPosition(loc);
        SetSize(sz);
        Layout();
    } else {
        wxSize appSize = _frame != nullptr ? _frame->GetSize() : (parent != nullptr ? parent->GetSize() : wxGetDisplaySize());
        SetSize(wxSize((int)(appSize.GetWidth() * 0.95), (int)(appSize.GetHeight() * 0.9)));
        CentreOnParent();
    }
    EnsureWindowHeaderIsOnScreen(this);

    PopulateAllControllers();
}

ControllerFullColumnsDialog::~ControllerFullColumnsDialog() {
    SaveWindowPosition("ControllerFullColumnsDialogPosition", this);
}

void ControllerFullColumnsDialog::CreateTree() {
    _tree = new wxTreeListCtrl(this, ID_TREELIST_ALL_CONTROLLER_COLUMNS, wxDefaultPosition, wxDefaultSize,
                                wxTL_SINGLE, "ID_TREELIST_ALL_CONTROLLER_COLUMNS");

    wxVector<wxBitmapBundle> bundles;
    LayoutUtils::CreateImageList(bundles);
    _ledIcons = AppendControllerLedIcons(bundles);

    wxImageList* imgList = wxBitmapBundle::CreateImageList(_tree, bundles);
    _tree->AssignImageList(imgList);

    // Every column from CTRL_FULL_COLUMN_ORDER is appended, in that order,
    // and never hidden — column position i always corresponds to
    // CTRL_FULL_COLUMN_ORDER[i], so no id-to-position bookkeeping is needed
    // beyond that array.
    for (size_t i = 0; i < CTRL_FULL_COLUMN_ORDER_COUNT; i++) {
        const char* title = ControllerColumnTitle(CTRL_FULL_COLUMN_ORDER[i]);
        _tree->AppendColumn(title, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE);
    }

    _tree->Bind(wxEVT_TREELIST_ITEM_EXPANDING, &ControllerFullColumnsDialog::OnItemExpanding, this);
}

void ControllerFullColumnsDialog::PopulateAllControllers() {
    _tree->Freeze();
    _tree->DeleteAllItems();
    wxTreeListItem root = _tree->GetRootItem();
    for (const auto& it : _frame->GetOutputManager()->GetControllers()) {
        int iconIndex = GetControllerIconIndex(it, _frame, _ledIcons);
        wxTreeListItem item = _tree->AppendItem(root, it->GetName(), iconIndex, iconIndex);
        for (size_t pos = 0; pos < CTRL_FULL_COLUMN_ORDER_COUNT; pos++) {
            int id = CTRL_FULL_COLUMN_ORDER[pos];
            if (id == 0) continue; // Name is the item's own label, column 0
            _tree->SetItemText(item, (int)pos, ControllerColumnLabel(it, id));
        }
        wxTreeListItem ph = _tree->AppendItem(item, "loading...");
        _tree->SetItemData(ph, new CtrlRowData(CtrlRowData::Kind::Placeholder, it->GetName()));
    }
    _tree->Thaw();
}

void ControllerFullColumnsDialog::OnItemExpanding(wxTreeListEvent& event) {
    PopulateControllerPorts(_tree, event.GetItem(), _frame);
}
