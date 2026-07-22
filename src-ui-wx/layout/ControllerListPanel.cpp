/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ControllerListPanel.h"

#include <wx/button.h>
#include <wx/dataobj.h>
#include <wx/dataview.h>
#include <wx/dnd.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/srchctrl.h>
#include <wx/stopwatch.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/wrapsizer.h>

#include <wx/statbmp.h>

#include <algorithm>
#include <map>
#include <set>

#include "xLightsMain.h"
#include "layout/LayoutPanel.h"
#include "layout/ModelPreview.h"
#include "layout/LayoutUtils.h"
#include "layout/ControllerFullColumnsDialog.h"
#include "layout/ControllerTreeUtils.h"

#include "shared/utils/TreeListColumnVisibility.h"
#include "shared/utils/wxUtilities.h"
#include "settings/XLightsConfigAdapter.h"
#include "setup/ControllerModelDialog.h"
#include "UtilFunctions.h"
#include "controllerproperties/ControllerPropertyAdapter.h"
#include "controllerproperties/ControllerPropertyManager.h"
#include "controllers/ControllerCaps.h"
#include "controllers/ControllerUploadData.h"
#include "models/Model.h"
#include "models/OutputModelManager.h"
#include "outputs/Controller.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/ControllerNull.h"
#include "outputs/ControllerSerial.h"
#include "outputs/Output.h"
#include "outputs/OutputManager.h"
#include "shared/utils/xlPropertyGrid.h"
#include "utils/SpecialOptions.h"
#include "utils/ip_utils.h"
#include "controllers/FPPConnectDialog.h"

#include <log.h>

using namespace ControllerTree;

const long ControllerListPanel::ID_TREELIST_CONTROLLERS = wxNewId();

namespace {
const long ID_TEXTCTRL_CONTROLLER_FILTER = wxNewId();
const long ID_CTRL_MNU_ADDETHERNET = wxNewId();
const long ID_CTRL_MNU_ADDNULL = wxNewId();
const long ID_CTRL_MNU_ADDSERIAL = wxNewId();
const long ID_CTRL_MNU_ACTIVE = wxNewId();
const long ID_CTRL_MNU_ACTIVEXLIGHTS = wxNewId();
const long ID_CTRL_MNU_INACTIVE = wxNewId();
const long ID_CTRL_MNU_DELETE = wxNewId();
const long ID_CTRL_MNU_UNLINKFROMBASE = wxNewId();
const long ID_CTRL_MNU_UPLOADOUTPUT = wxNewId();
const long ID_CTRL_MNU_FPP_CONNECT = wxNewId();
const long ID_CTRL_MNU_SORT_NAME = wxNewId();
const long ID_CTRL_MNU_SORT_ID = wxNewId();
const long ID_CTRL_MNU_SORT_IP = wxNewId();
const long ID_CTRL_MNU_SORT_FPP_PROXY = wxNewId();
const long ID_CTRL_MNU_SORT_VENDOR = wxNewId();
const long ID_CTRL_MNU_SORT_PROTOCOL = wxNewId();
const long ID_CTRL_MNU_EXPAND_ALL = wxNewId();
const long ID_CTRL_MNU_COLLAPSE_ALL = wxNewId();

// Marks our own drag payload so a drop originating elsewhere is ignored.
const char* const CTRL_DRAG_PREFIX = "xlctrlmove:";

std::string DataViewTopLevelName(const wxDataViewModel* model, wxDataViewItem item) {
    if (!item.IsOk()) return "";
    // climb until the parent's parent is invalid (parent == hidden root)
    while (model->GetParent(item).IsOk() && model->GetParent(model->GetParent(item)).IsOk()) {
        item = model->GetParent(item);
    }
    wxVariant v;
    model->GetValue(v, item, 0);
    wxDataViewIconText it;
    it << v;
    return it.GetText().ToStdString();
}

// wxTreeListCtrl's internal model has no GetAttr support, so colouring rows
// like the Setup tab list (base-linked cyan/blue, inactive base-linked pale
// blue, inactive local grey) requires wrapping that model in a forwarding
// proxy that supplies the attributes. Child rows inherit their controller's
// colour because the lookup climbs to the top-level row.
class CtrlAttrProxyModel : public wxDataViewModel {
public:
    CtrlAttrProxyModel(wxDataViewModel* inner, OutputManager* om) :
        _inner(inner), _om(om) {
        _inner->IncRef();
        _notifier = new Forward(this);
        _inner->AddNotifier(_notifier); // inner model owns and deletes it
    }

    void GetValue(wxVariant& v, const wxDataViewItem& item, unsigned int col) const override { _inner->GetValue(v, item, col); }
    bool SetValue(const wxVariant& v, const wxDataViewItem& item, unsigned int col) override { return _inner->SetValue(v, item, col); }
    bool HasValue(const wxDataViewItem& item, unsigned col) const override { return _inner->HasValue(item, col); }
    wxDataViewItem GetParent(const wxDataViewItem& item) const override { return _inner->GetParent(item); }
    bool IsContainer(const wxDataViewItem& item) const override { return _inner->IsContainer(item); }
    bool HasContainerColumns(const wxDataViewItem& item) const override { return _inner->HasContainerColumns(item); }
    unsigned int GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const override { return _inner->GetChildren(item, children); }
    int Compare(const wxDataViewItem& item1, const wxDataViewItem& item2, unsigned int column, bool ascending) const override { return _inner->Compare(item1, item2, column, ascending); }
    bool HasDefaultCompare() const override { return _inner->HasDefaultCompare(); }
    bool IsListModel() const override { return _inner->IsListModel(); }
    bool IsEnabled(const wxDataViewItem& item, unsigned int col) const override { return _inner->IsEnabled(item, col); }
    void Resort() override { _inner->Resort(); }

    bool GetAttr(const wxDataViewItem& item, unsigned int col, wxDataViewItemAttr& attr) const override {
        Controller* c = _om->GetController(TopLevelNameCached(item));
        if (c == nullptr) return false;
        bool hasAttr = false;
        if (col == 0) {
            attr.SetBold(true);
            hasAttr = true;
        }
        if (c->IsFromBase()) {
            attr.SetColour(c->IsActive() ? CyanOrBlue() : wxColour(0x80, 0x80, 0xFF));
            return true;
        }
        if (!c->IsActive()) {
            attr.SetColour(*wxLIGHT_GREY);
            return true;
        }
        return hasAttr;
    }

protected:
    ~CtrlAttrProxyModel() override {
        _inner->RemoveNotifier(_notifier); // deletes the notifier
        _inner->DecRef();
    }

private:
    // GetAttr runs per cell, and resolving the owning controller means climbing
    // to the top-level row and round-tripping its label through a wxVariant.
    // Consecutive calls walk the columns of one row, so remembering the last
    // item collapses that to once per row. The name (not the Controller*) is
    // cached so a controller deleted before the tree is rebuilt can't be
    // dereferenced.
    const std::string& TopLevelNameCached(const wxDataViewItem& item) const {
        if (item.GetID() != _lastId) {
            _lastId = item.GetID();
            _lastName = DataViewTopLevelName(_inner, item);
        }
        return _lastName;
    }
    void InvalidateNameCache() {
        _lastId = nullptr;
        _lastName.clear();
    }

    // Relays change notifications from the wrapped model to this proxy's
    // listeners (the view), which otherwise would never hear about updates
    // made through the wxTreeListCtrl API.
    class Forward : public wxDataViewModelNotifier {
    public:
        explicit Forward(CtrlAttrProxyModel* t) : _t(t) {}
        bool ItemAdded(const wxDataViewItem& parent, const wxDataViewItem& item) override { return _t->ItemAdded(parent, item); }
        bool ItemDeleted(const wxDataViewItem& parent, const wxDataViewItem& item) override { _t->InvalidateNameCache(); return _t->ItemDeleted(parent, item); }
        bool ItemChanged(const wxDataViewItem& item) override { _t->InvalidateNameCache(); return _t->ItemChanged(item); }
        bool ItemsAdded(const wxDataViewItem& parent, const wxDataViewItemArray& items) override { return _t->ItemsAdded(parent, items); }
        bool ItemsDeleted(const wxDataViewItem& parent, const wxDataViewItemArray& items) override { _t->InvalidateNameCache(); return _t->ItemsDeleted(parent, items); }
        bool ItemsChanged(const wxDataViewItemArray& items) override { _t->InvalidateNameCache(); return _t->ItemsChanged(items); }
        bool ValueChanged(const wxDataViewItem& item, unsigned int col) override { _t->InvalidateNameCache(); return _t->ValueChanged(item, col); }
        bool Cleared() override { _t->InvalidateNameCache(); return _t->Cleared(); }
        bool BeforeReset() override { _t->InvalidateNameCache(); return _t->BeforeReset(); }
        bool AfterReset() override { _t->InvalidateNameCache(); return _t->AfterReset(); }
        // base-class Resort() only notifies the proxy's listeners; calling the
        // proxy's own override would bounce back into the inner model forever
        void Resort() override { _t->wxDataViewModel::Resort(); }

    private:
        CtrlAttrProxyModel* _t;
    };

    wxDataViewModel* _inner;
    OutputManager* _om;
    Forward* _notifier;
    mutable void* _lastId = nullptr;
    mutable std::string _lastName;
};

void HighlightModelRow(ModelPreview* preview, const CtrlRowData* md, ModelManager& allModels) {
    Model* m = allModels[md->modelName];
    if (m == nullptr) return;
    // CtrlRowData channels come from UDControllerPortModel (1-based absolute);
    // ModelPreview compares against Model::NodeStartChannel (0-based absolute).
    const uint32_t first = md->startChannel > 0 ? (uint32_t)(md->startChannel - 1) : 0;
    const uint32_t last = md->endChannel > 0 ? (uint32_t)(md->endChannel - 1) : 0;
    if (md->isShadow) {
        Model* main = allModels[md->shadowOf];
        if (main != nullptr) {
            preview->SetPortChannelHighlight(main, first, last);
        }
    } else if (md->stringIndex >= 0) {
        preview->SetPortStringHighlight(m, md->stringIndex);
    } else {
        preview->SetPortChannelHighlight(m, first, last);
    }
}
} // namespace

ControllerListPanel::ControllerListPanel(wxWindow* parent, xLightsFrame* frame, LayoutPanel* layoutPanel) :
    wxPanel(parent, wxID_ANY), _frame(frame), _layoutPanel(layoutPanel) {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* filterSizer = new wxBoxSizer(wxHORIZONTAL);
    _controllerFilterCtrl = new wxSearchCtrl(this, ID_TEXTCTRL_CONTROLLER_FILTER,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    _controllerFilterCtrl->SetDescriptiveText("Filter controllers...");
    _controllerFilterCtrl->ShowCancelButton(true);
    _controllerFilterCtrl->Bind(wxEVT_TEXT_ENTER, &ControllerListPanel::OnControllerFilterTextChanged, this);
    _controllerFilterCtrl->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &ControllerListPanel::OnControllerFilterTextChanged, this);
    _controllerFilterCtrl->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &ControllerListPanel::OnControllerFilterCancelBtn, this);
    _controllerFilterCtrl->Bind(wxEVT_TEXT, [this](wxCommandEvent&) {
        _controllerFilterString = _controllerFilterCtrl->GetValue().Trim();
        _controllerFilterRegex.Compile(_controllerFilterString, wxRE_ICASE);
        _controllerFilterRegexValid = _controllerFilterRegex.IsValid();
        if (_controllerFilterString.IsEmpty()) {
            UpdateControllerList();
        }
        });
    filterSizer->Add(_controllerFilterCtrl, 1, wxEXPAND | wxTOP, 2);

    _btnFullColumns = new wxButton(this, wxID_ANY, "Show All Info", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    _btnFullColumns->SetToolTip("Show every controller column for every controller in a larger window");
    _btnFullColumns->Bind(wxEVT_BUTTON, &ControllerListPanel::OnFullColumnsClick, this);
    filterSizer->Add(_btnFullColumns, 0, wxEXPAND | wxLEFT | wxTOP, 2);

    sizer->Add(filterSizer, 0, wxEXPAND);

    CreateTree();
    sizer->Add(_tree, 1, wxEXPAND | wxALL, 0);
    SetSizer(sizer);
}

ControllerListPanel::~ControllerListPanel() {
    SaveColumnOrder();
}

void ControllerListPanel::CreateTree() {
    _tree = new wxTreeListCtrl(this, ID_TREELIST_CONTROLLERS, wxDefaultPosition, wxDefaultSize,
                               wxTL_MULTIPLE, "ID_TREELIST_CONTROLLERS");

    wxVector<wxBitmapBundle> bundles;
    LayoutUtils::CreateImageList(bundles);
    _ledIcons = AppendControllerLedIcons(bundles);

    wxImageList* imgList = wxBitmapBundle::CreateImageList(_tree, bundles);
    _tree->AssignImageList(imgList);

    // Column 0 (Name) is fixed; the rest are appended in saved order because
    // wxTreeListCtrl cannot programmatically reorder columns after creation.
    // With no saved order, columns default to CTRL_COLS' (alphabetical) order.
    _tree->AppendColumn(CTRL_COLS[0].title, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE);
    wxArrayString saved = wxSplit(GetXLightsConfig()->Read("LayoutControllerListCols", ""), ',');
    wxArrayString order;
    for (const auto& t : saved) {
        for (size_t i = 1; i < CTRL_COL_COUNT; i++) {
            if (t == CTRL_COLS[i].title && order.Index(t) == wxNOT_FOUND) {
                order.push_back(t);
            }
        }
    }
    for (size_t i = 1; i < CTRL_COL_COUNT; i++) {
        if (order.Index(CTRL_COLS[i].title) == wxNOT_FOUND) {
            order.push_back(CTRL_COLS[i].title);
        }
    }
    for (const auto& t : order) {
        _tree->AppendColumn(t, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT,
                            wxCOL_RESIZABLE | wxCOL_REORDERABLE);
    }
    _colIdx.assign(CTRL_COL_COUNT, 0);
    for (size_t i = 0; i < CTRL_COL_COUNT; i++) {
        _colIdx[i] = ColIndex(CTRL_COLS[i].title);
    }

    {
        // Re-associate the view with a proxy so base-linked / inactive
        // controllers get the same row colours as the Setup tab list.
        auto* dv = _tree->GetDataView();
        auto* proxy = new CtrlAttrProxyModel(dv->GetModel(), _frame->GetOutputManager());
        dv->AssociateModel(proxy);
        proxy->DecRef(); // the view holds the remaining reference
    }

    // New columns default to unselected (hidden) the first time they appear;
    // once a user toggles visibility their choice persists like any other column.
    static const std::vector<std::string> DEFAULT_HIDDEN_CTRL_COLS = {
        "Auto Upload", "Full xLights Control", "Keep Channels", "Port Brightness", "Port Gamma", "Proxy"
    };
    InstallTreeListColumnVisibility(_tree, "LayoutControllerListColsHidden", DEFAULT_HIDDEN_CTRL_COLS);

    _tree->GetDataView()->EnableDragSource(wxDataFormat(wxDF_TEXT));
    _tree->GetDataView()->EnableDropTarget(wxDataFormat(wxDF_TEXT));
    _tree->GetDataView()->Bind(wxEVT_DATAVIEW_ITEM_BEGIN_DRAG, &ControllerListPanel::OnBeginDrag, this);
    _tree->GetDataView()->Bind(wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE, &ControllerListPanel::OnDragPossible, this);
    _tree->GetDataView()->Bind(wxEVT_DATAVIEW_ITEM_DROP, &ControllerListPanel::OnDragDrop, this);


    _tree->Bind(wxEVT_TREELIST_SELECTION_CHANGED, &ControllerListPanel::OnSelectionChanged, this);
    _tree->Bind(wxEVT_TREELIST_ITEM_EXPANDING, &ControllerListPanel::OnItemExpanding, this);
    _tree->Bind(wxEVT_TREELIST_ITEM_ACTIVATED, &ControllerListPanel::OnItemActivated, this);
    _tree->GetView()->Bind(wxEVT_CONTEXT_MENU, &ControllerListPanel::OnContextMenu, this);
    _tree->GetView()->Bind(wxEVT_CHAR_HOOK, &ControllerListPanel::OnCharHook, this);
    // We are a descendant of LayoutPanel, whose event table catches
    // EVT_TREELIST_ITEM_CONTEXT_MENU for wxID_ANY and would pop up the
    // model-tree menu on top of ours. Swallow it here; wxEVT_CONTEXT_MENU above
    // is what actually shows the controller menu.
    _tree->Bind(wxEVT_TREELIST_ITEM_CONTEXT_MENU, [](wxTreeListEvent&) {});
}

int ControllerListPanel::ColIndex(const wxString& title) const {
    for (unsigned i = 0; i < _tree->GetColumnCount(); i++) {
        if (_tree->GetDataView()->GetColumn(i)->GetTitle() == title) {
            return (int)i;
        }
    }
    return -1;
}

void ControllerListPanel::SaveColumnOrder() {
    if (_tree == nullptr) return;
    wxString colOrder;
    for (size_t i = 0; i < _tree->GetColumnCount(); i++) {
        for (unsigned j = 0; j < _tree->GetColumnCount(); j++) {
            auto* col = _tree->GetDataView()->GetColumn(j);
            auto p = _tree->GetDataView()->GetColumnPosition(col);
            if (p == (int)i && col->GetTitle() != CTRL_COLS[0].title) {
                if (!colOrder.empty()) colOrder += ",";
                colOrder += col->GetTitle();
            }
        }
    }
    GetXLightsConfig()->Write("LayoutControllerListCols", colOrder);
}

void ControllerListPanel::UpdateControllerList() {
    wxStopWatch sw;
    auto selections = GetSelectedControllerNames();

    std::set<std::string> expanded;
    for (wxTreeListItem item = _tree->GetFirstChild(_tree->GetRootItem()); item.IsOk(); item = _tree->GetNextSibling(item)) {
        if (_tree->IsExpanded(item)) {
            expanded.insert(_tree->GetItemText(item, 0).ToStdString());
        }
    }

    _tree->Freeze();
    _tree->DeleteAllItems();
    wxTreeListItem root = _tree->GetRootItem();
    for (const auto& it : _frame->GetOutputManager()->GetControllers()) {
        if (!ControllerMatchesFilter(it)) continue;
        int iconIndex = GetControllerIconIndex(it, _frame, _ledIcons);
        wxTreeListItem item = _tree->AppendItem(root, it->GetName(), iconIndex, iconIndex);
        for (size_t col = 1; col < CTRL_COL_COUNT; col++) {
            _tree->SetItemText(item, _colIdx[col], ControllerColumnLabel(it, CTRL_COLS[col].id));
        }
        wxTreeListItem ph = _tree->AppendItem(item, "loading...");
        _tree->SetItemData(ph, new CtrlRowData(CtrlRowData::Kind::Placeholder, it->GetName()));
        if (expanded.count(it->GetName())) {
            // Expand() does not fire ITEM_EXPANDING, so the ports have to be
            // built here. Each one constructs a UDController, which walks every
            // model — the cost scales with how many controllers the user left
            // expanded, hence the timing below.
            PopulatePorts(item);
            if (_tree->GetFirstChild(item).IsOk()) {
                _tree->Expand(item);
            }
        }
        if (std::find(selections.begin(), selections.end(), it->GetName()) != selections.end()) {
            _tree->Select(item);
        }
    }
    _tree->Thaw();
    UpdateControllerProperties();
    UpdatePreviewHighlights();

    if (sw.Time() > 500) {
        spdlog::debug("        ControllerListPanel::UpdateControllerList took {}ms", sw.Time());
    }
}

void ControllerListPanel::OnItemExpanding(wxTreeListEvent& event) {
    wxTreeListItem item = event.GetItem();
    PopulatePorts(item);
    if (!_tree->GetFirstChild(item).IsOk()) {
        // No ports were added (controller has no configured/populated ports) -
        // there's nothing to expand into, and the placeholder that made the
        // item look expandable is gone, so cancel the expand rather than
        // letting the tree try to expand a now-childless item.
        event.Veto();
    }
}

void ControllerListPanel::PopulatePorts(wxTreeListItem ctrlItem) {
    PopulateControllerPorts(_tree, ctrlItem, _frame);
}

void ControllerListPanel::RefreshPingIndicator(const Controller* controller) {
    switch (ClassifyControllerPing(controller)) {
    case ControllerPingBucket::Green:
        _ledPing->SetBitmap(_ledPingBitmaps.green);
        break;
    case ControllerPingBucket::Red:
        _ledPing->SetBitmap(_ledPingBitmaps.red);
        break;
    default:
        _ledPing->SetBitmap(_ledPingBitmaps.gray);
        break;
    }
}

void ControllerListPanel::RefreshStatusColumn() {
    auto selectedController = GetFirstSelectedController();
    for (wxTreeListItem item = _tree->GetFirstChild(_tree->GetRootItem()); item.IsOk(); item = _tree->GetNextSibling(item)) {
        auto controller = _frame->GetOutputManager()->GetController(_tree->GetItemText(item, 0).ToStdString());
        if (controller != nullptr) {
            int iconIndex = GetControllerIconIndex(controller, _frame, _ledIcons);
            _tree->SetItemImage(item, iconIndex, iconIndex);
            if (controller == selectedController) {
                RefreshPingIndicator(controller);
            }
        }
    }
}

wxTreeListItem ControllerListPanel::TopLevelItem(wxTreeListItem item) const {
    while (item.IsOk() && _tree->GetItemParent(item).IsOk() &&
           _tree->GetItemParent(item) != _tree->GetRootItem()) {
        item = _tree->GetItemParent(item);
    }
    return item;
}

std::list<std::string> ControllerListPanel::GetSelectedControllerNames() const {
    std::list<std::string> res;
    wxTreeListItems items;
    _tree->GetSelections(items);
    for (const auto& it : items) {
        wxTreeListItem top = TopLevelItem(it);
        if (!top.IsOk()) continue;
        auto name = _tree->GetItemText(top, 0).ToStdString();
        if (std::find(res.begin(), res.end(), name) == res.end()) {
            res.push_back(name);
        }
    }
    return res;
}

Controller* ControllerListPanel::GetFirstSelectedController() const {
    auto names = GetSelectedControllerNames();
    if (names.empty()) return nullptr;
    return _frame->GetOutputManager()->GetController(names.front());
}

void ControllerListPanel::SelectController(const std::string& name) {
    for (wxTreeListItem item = _tree->GetFirstChild(_tree->GetRootItem()); item.IsOk(); item = _tree->GetNextSibling(item)) {
        if (_tree->GetItemText(item, 0) == name) {
            _tree->UnselectAll();
            _tree->Select(item);
            _tree->EnsureVisible(item);
            break;
        }
    }
}

bool ControllerListPanel::NetworkChangesAllowed() const {
    return !_frame->GetOutputManager()->IsOutputting() && !_frame->GetCurrentDir().empty();
}

void ControllerListPanel::OnControllerFilterCancelBtn(wxCommandEvent& event) {
    _controllerFilterCtrl->SetValue("");
    _controllerFilterString = "";
    _controllerFilterRegexValid = false;
    UpdateControllerList();
}

void ControllerListPanel::OnControllerFilterTextChanged(wxCommandEvent& event) {
    _controllerFilterString = _controllerFilterCtrl->GetValue().Trim();
    _controllerFilterRegex.Compile(_controllerFilterString, wxRE_ICASE);
    _controllerFilterRegexValid = _controllerFilterRegex.IsValid();
    UpdateControllerList();
}

void ControllerListPanel::OnFullColumnsClick(wxCommandEvent& event) {
    ControllerFullColumnsDialog dlg(this, _frame);
    dlg.ShowModal();
}

bool ControllerListPanel::ControllerMatchesFilter(const Controller* controller) const {
    if (_controllerFilterCtrl == nullptr || _controllerFilterString.IsEmpty()) return true;

    if (_controllerFilterRegexValid)
        return _controllerFilterRegex.Matches(controller->GetName());

    return wxString(controller->GetName()).Lower().Contains(_controllerFilterString.Lower());
}

void ControllerListPanel::OnSelectionChanged(wxTreeListEvent& event) {
    UpdateControllerProperties();
    UpdatePreviewHighlights();
}

void ControllerListPanel::ClearPreviewHighlights() {
    auto* preview = _layoutPanel->GetMainPreview();
    if (preview == nullptr) return;
    preview->ClearPortStringHighlights();
    _frame->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW,
                                                 "ControllerListPanel::ClearPreviewHighlights");
}

void ControllerListPanel::UpdatePreviewHighlights() {
    auto* preview = _layoutPanel->GetMainPreview();
    if (preview == nullptr) return;
    preview->ClearPortStringHighlights();

    auto highlightWholeModel = [&](Model* m) {
        if (m == nullptr) return;
        // Shadow models resolve to the model they shadow
        if (m->IsShadowModel()) {
            Model* main = _frame->AllModels[m->GetShadowModelFor()];
            if (main != nullptr) {
                preview->SetPortChannelHighlight(main, m->GetFirstChannel(), m->GetLastChannel());
            }
            return;
        }
        preview->SetPortChannelHighlight(m, m->GetFirstChannel(), m->GetLastChannel());
    };

    wxTreeListItems items;
    _tree->GetSelections(items);
    for (const auto& item : items) {
        auto* data = dynamic_cast<CtrlRowData*>(_tree->GetItemData(item));
        if (data == nullptr) {
            // Controller row: highlight every model on the controller
            auto ctrlName = _tree->GetItemText(item, 0).ToStdString();
            for (const auto& [name, model] : _frame->AllModels) {
                if (model->GetDisplayAs() == DisplayAsType::ModelGroup) continue;
                Controller* ctrl = model->GetController();
                if (ctrl == nullptr || ctrl->GetName() != ctrlName) continue;
                highlightWholeModel(model);
            }
        } else if (data->kind == CtrlRowData::Kind::Port) {
            // Port row: highlight each model row beneath it
            for (wxTreeListItem mi = _tree->GetFirstChild(item); mi.IsOk(); mi = _tree->GetNextSibling(mi)) {
                auto* md = dynamic_cast<CtrlRowData*>(_tree->GetItemData(mi));
                if (md != nullptr && md->kind == CtrlRowData::Kind::Model) {
                    HighlightModelRow(preview, md, _frame->AllModels);
                }
            }
        } else if (data->kind == CtrlRowData::Kind::Model) {
            HighlightModelRow(preview, data, _frame->AllModels);
        }
    }

    _frame->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW,
                                                 "ControllerListPanel::UpdatePreviewHighlights");
}

void ControllerListPanel::OnItemActivated(wxTreeListEvent& event) {
    auto name = _tree->GetItemText(TopLevelItem(event.GetItem()), 0);
    auto controller = _frame->GetOutputManager()->GetController(name);
    if (wxGetKeyState(WXK_CONTROL) || wxGetKeyState(WXK_SHIFT)) {
        if (controller != nullptr && controller->GetIP() != "") {
            if (controller->GetFPPProxy() != "" && controller->GetFPPProxy() != controller->GetIP()) {
                ::wxLaunchDefaultBrowser("http://" + controller->GetFPPProxy() + "/proxy/" + controller->GetIP() + "/");
            } else {
                ::wxLaunchDefaultBrowser("http://" + controller->GetIP());
            }
        }
    } else {
        if (!NetworkChangesAllowed()) {
            return;
        }
        if (controller != nullptr) {
            int usingip = _frame->GetOutputManager()->GetControllerCount(controller->GetType(), controller->GetColumn2Label());
            if (usingip == 1 && controller->CanVisualise()) {
                UDController cud(controller, _frame->GetOutputManager(), &_frame->AllModels, true);
                ControllerModelDialog dlg(_frame, &cud, &_frame->AllModels, controller);
                dlg.ShowModal();
            } else {
                DisplayError(name + " cannot be Visualised", this);
            }
        } else {
            DisplayError(name + " cannot find the controller", this);
        }
    }
}

void ControllerListPanel::OnContextMenu(wxContextMenuEvent& event) {
    wxMenu mnu;
    std::string ethernet = "Insert E1.31/ArtNET/ZCPP/DDP";
    if (SpecialOptions::GetOption("xxx") == "true") {
        ethernet += "xxx";
    }

    std::vector<Controller*> selected;
    for (const auto& name : GetSelectedControllerNames()) {
        Controller* c = _frame->GetOutputManager()->GetController(name);
        if (c != nullptr) selected.push_back(c);
    }
    bool anyFromBase = std::any_of(selected.begin(), selected.end(), [](const Controller* c) { return c->IsFromBase(); });
    bool allFromBase = !selected.empty() && std::all_of(selected.begin(), selected.end(), [](const Controller* c) { return c->IsFromBase(); });
    
    bool allowed = NetworkChangesAllowed();
    bool canActivate = allowed && !selected.empty() && !anyFromBase;
    bool canUpload = allowed && selected.size() == 1 && selected.front()->SupportsUpload();

    mnu.Append(ID_CTRL_MNU_ADDETHERNET, ethernet)->Enable(allowed);
    mnu.Append(ID_CTRL_MNU_ADDNULL, "Insert NULL")->Enable(allowed);
    mnu.Append(ID_CTRL_MNU_ADDSERIAL, "Insert DMX/LOR/DLight/Renard")->Enable(allowed);
    mnu.Append(ID_CTRL_MNU_ACTIVE, "Activate")->Enable(canActivate);
    mnu.Append(ID_CTRL_MNU_ACTIVEXLIGHTS, "Activate in xLights Only")->Enable(canActivate);
    mnu.Append(ID_CTRL_MNU_INACTIVE, "Inactivate")->Enable(canActivate);
    mnu.Append(ID_CTRL_MNU_DELETE, "Delete")->Enable(allowed && !selected.empty());
    mnu.Append(ID_CTRL_MNU_UNLINKFROMBASE, "Unlink from Base Show Folder")->Enable(allowed && allFromBase);
    bool isIpController = false;
    std::string targetIp;
    if (selected.size() == 1) {
        targetIp = selected.front()->GetResolvedIP(true);
        if (targetIp.empty()) {
            targetIp = selected.front()->GetIP();
        }
        isIpController = !targetIp.empty() && targetIp != "MULTICAST";
    }

    mnu.Append(ID_CTRL_MNU_UPLOADOUTPUT, "Upload Output")->Enable(canUpload);
    mnu.Append(ID_CTRL_MNU_FPP_CONNECT, "FPP Connect")->Enable(allowed && isIpController);

    mnu.AppendSeparator();
    wxMenu* sort = new wxMenu();
    sort->Append(ID_CTRL_MNU_SORT_NAME, "by Name")->Enable(allowed);
    sort->Append(ID_CTRL_MNU_SORT_ID, "by ID")->Enable(allowed);
    sort->Append(ID_CTRL_MNU_SORT_IP, "by IP")->Enable(allowed);
    sort->Append(ID_CTRL_MNU_SORT_FPP_PROXY, "by FPP Proxy")->Enable(allowed);
    sort->Append(ID_CTRL_MNU_SORT_VENDOR, "by Controller Model")->Enable(allowed);
    sort->Append(ID_CTRL_MNU_SORT_PROTOCOL, "by Controller Protocol")->Enable(allowed);
    mnu.AppendSubMenu(sort, "Sort");

    mnu.AppendSeparator();
    mnu.Append(ID_CTRL_MNU_EXPAND_ALL, "Expand All Controllers");
    mnu.Append(ID_CTRL_MNU_COLLAPSE_ALL, "Collapse All");

    mnu.Bind(wxEVT_MENU, &ControllerListPanel::OnPopup, this);
    PopupMenu(&mnu);
}

void ControllerListPanel::OnPopup(wxCommandEvent& event) {
    long id = event.GetId();
    if (id != ID_CTRL_MNU_EXPAND_ALL && id != ID_CTRL_MNU_COLLAPSE_ALL) {
        if (!NetworkChangesAllowed()) return;
    }
    auto* om = _frame->GetOutputManager();
    auto* omm = _frame->GetOutputModelManager();

    int insertAt = -1;
    {
        wxTreeListItems items;
        _tree->GetSelections(items);
        if (!items.empty()) {
            wxTreeListItem top = TopLevelItem(items.front());
            int idx = 0;
            for (wxTreeListItem it = _tree->GetFirstChild(_tree->GetRootItem()); it.IsOk(); it = _tree->GetNextSibling(it), idx++) {
                if (it == top) {
                    insertAt = idx;
                    break;
                }
            }
        }
    }

    auto queueStandardWork = [omm](const std::string& from, Controller* c = nullptr) {
        omm->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, from);
        omm->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, from);
        omm->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, from, nullptr, c);
        omm->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, from);
    };

    if (id == ID_CTRL_MNU_ADDSERIAL) {
        auto c = new ControllerSerial(om);
        om->AddController(c, insertAt);
        queueStandardWork("ControllerListPanel:ADDSERIAL", c);
    } else if (id == ID_CTRL_MNU_ADDETHERNET) {
        auto c = new ControllerEthernet(om);
        om->AddController(c, insertAt);
        queueStandardWork("ControllerListPanel:ADDETHERNET", c);
    } else if (id == ID_CTRL_MNU_ADDNULL) {
        auto c = new ControllerNull(om);
        om->AddController(c, insertAt);
        queueStandardWork("ControllerListPanel:ADDNULL", c);
    } else if (id == ID_CTRL_MNU_ACTIVE) {
        ActivateSelectedControllers("Active");
        queueStandardWork("ControllerListPanel:ACTIVE");
    } else if (id == ID_CTRL_MNU_ACTIVEXLIGHTS) {
        ActivateSelectedControllers("xLights Only");
        queueStandardWork("ControllerListPanel:ACTIVEXLIGHTS");
    } else if (id == ID_CTRL_MNU_INACTIVE) {
        ActivateSelectedControllers("Inactive");
        queueStandardWork("ControllerListPanel:INACTIVE");
    } else if (id == ID_CTRL_MNU_DELETE) {
        DeleteSelectedControllers();
    } else if (id == ID_CTRL_MNU_UNLINKFROMBASE) {
        UnlinkSelectedControllers();
        omm->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerListPanel:UNLINK");
        omm->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerListPanel:UNLINK");
    } else if (id == ID_CTRL_MNU_UPLOADOUTPUT) {
        auto names = GetSelectedControllerNames();
        if (!names.empty()) {
            _frame->UploadControllerOutput(_frame->GetOutputManager()->GetController(names.front()));
            UpdateControllerProperties();
        }
    } else if (id == ID_CTRL_MNU_FPP_CONNECT) {
        auto names = GetSelectedControllerNames();
        if (!names.empty()) {
            Controller* c = om->GetController(names.front());
            if (c != nullptr) {
                std::string targetIp = c->GetResolvedIP(true);
                if (targetIp.empty()) {
                    targetIp = c->GetIP();
                }
                if (!targetIp.empty() && targetIp != "MULTICAST") {
                    _layoutPanel->UnSelectAllModels();
                    _frame->RecalcModels();

                    if (_frame->mSavedChangeCount != _frame->_sequenceElements.GetChangeCount()) {
                        if (wxMessageBox("Open sequence has not been saved. If you plan on uploading it to FPP then it needs to be saved. Do you want to save it now?", "Save Sequence", wxYES_NO | wxCENTRE, _frame) == wxYES) {
                            _frame->SaveSequence();
                        }
                    }

                    FPPConnectDialog dlg(_frame, _frame->GetOutputManager(), targetIp);
                    dlg.ShowModal();
                }
            }
        }
    } else if (id == ID_CTRL_MNU_SORT_NAME) {
        om->SortControllersbyName();
        queueStandardWork("ControllerListPanel:SORT_NAME");
    } else if (id == ID_CTRL_MNU_SORT_ID) {
        om->SortControllersbyID();
        queueStandardWork("ControllerListPanel:SORT_ID");
    } else if (id == ID_CTRL_MNU_SORT_IP) {
        om->SortControllersbyIP();
        queueStandardWork("ControllerListPanel:SORT_IP");
    } else if (id == ID_CTRL_MNU_SORT_FPP_PROXY) {
        om->SortControllersbyFPPProxy();
        queueStandardWork("ControllerListPanel:SORT_FPP_PROXY");
    } else if (id == ID_CTRL_MNU_SORT_VENDOR) {
        om->SortControllersbyModel();
        queueStandardWork("ControllerListPanel:SORT_VENDOR");
    } else if (id == ID_CTRL_MNU_SORT_PROTOCOL) {
        om->SortControllersbyProtocal();
        queueStandardWork("ControllerListPanel:SORT_PROTOCOL");
    } else if (id == ID_CTRL_MNU_EXPAND_ALL) {
        ExpandAllControllers();
    } else if (id == ID_CTRL_MNU_COLLAPSE_ALL) {
        CollapseAll();
    }
}

void ControllerListPanel::ActivateSelectedControllers(const std::string& active) {
    for (const auto& name : GetSelectedControllerNames()) {
        auto c = _frame->GetOutputManager()->GetController(name);
        if (c != nullptr) {
            c->SetActive(active);
        }
    }
}

void ControllerListPanel::UnlinkSelectedControllers() {
    for (const auto& name : GetSelectedControllerNames()) {
        auto c = _frame->GetOutputManager()->GetController(name);
        if (c != nullptr) {
            c->SetFromBase(false);
        }
    }
}

void ControllerListPanel::DeleteSelectedControllers() {
    if (!NetworkChangesAllowed()) return;
    auto todel = GetSelectedControllerNames();
    if (todel.empty()) return;
    auto msg = wxString::Format("Are you sure you want to delete %d controllers.", (int)todel.size());
    if (wxMessageBox(msg, "Delete controller(s)", wxYES_NO) != wxYES) return;
    _frame->waitForPingsToComplete();
    _frame->AbortRender();
    for (const auto& it : todel) {
        _frame->AllModels.DeleteController(it);
        _frame->GetOutputManager()->DeleteController(it);
    }
    auto* omm = _frame->GetOutputModelManager();
    omm->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerListPanel:DELETE");
    omm->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerListPanel:DELETE");
    omm->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerListPanel:DELETE");
    omm->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerListPanel:DELETE");
}

void ControllerListPanel::OnBeginDrag(wxDataViewEvent& event) {
    if (!NetworkChangesAllowed()) { event.Veto(); return; }
    auto names = GetSelectedControllerNames();
    // if the drag started on a row outside the selection, drag just that row
    auto dragged = DataViewTopLevelName(event.GetModel(), event.GetItem());
    if (dragged.empty()) { event.Veto(); return; }
    if (std::find(names.begin(), names.end(), dragged) == names.end()) {
        names = { dragged };
    }
    wxString payload = CTRL_DRAG_PREFIX;
    for (const auto& n : names) {
        if (payload != CTRL_DRAG_PREFIX) payload += "|";
        payload += n;
    }
    event.SetDataObject(new wxTextDataObject(payload));
    event.SetDragFlags(wxDrag_DefaultMove);
}

void ControllerListPanel::OnDragPossible(wxDataViewEvent& event) {
#ifdef __WXOSX__
    if (event.GetProposedDropIndex() != -1) return; // same kludge as xLightsImportChannelMapDialog
#endif
    event.Allow();
    event.SetDropEffect(wxDragMove);
}

void ControllerListPanel::OnDragDrop(wxDataViewEvent& event) {
    auto* obj = dynamic_cast<wxTextDataObject*>(event.GetDataObject());
    if (obj == nullptr) {
        if (auto* comp = dynamic_cast<wxDataObjectComposite*>(event.GetDataObject())) {
            obj = (wxTextDataObject*)comp->GetObject(wxDF_TEXT);
        }
    }
    if (obj == nullptr) return;
    wxString txt = obj->GetText();
    // OSX drag drop can prepend a unicode BOM (see xLightsImportChannelMapDialog::OnDragDrop)
    int at = txt.Find(CTRL_DRAG_PREFIX);
    if (at == wxNOT_FOUND) return;
    txt = txt.Mid(at + wxStrlen(CTRL_DRAG_PREFIX));

    std::list<std::string> names;
    for (const auto& n : wxSplit(txt, '|')) {
        if (!n.empty()) names.push_back(n.ToStdString());
    }
    if (names.empty() || !NetworkChangesAllowed()) return;

    auto* om = _frame->GetOutputManager();

    // Desired final order: current order minus dragged, dragged block inserted at the target row
    std::vector<Controller*> dragged;
    std::vector<Controller*> rest;
    for (auto* c : om->GetControllers()) {
        if (std::find(names.begin(), names.end(), c->GetName()) != names.end()) {
            dragged.push_back(c);
        } else {
            rest.push_back(c);
        }
    }
    if (dragged.empty()) return;

    auto targetName = DataViewTopLevelName(event.GetModel(), event.GetItem());
    size_t insertAt = rest.size(); // empty space → move to end
    if (!targetName.empty()) {
        if (targetName == dragged.front()->GetName()) return; // dropped on itself
        for (size_t i = 0; i < rest.size(); i++) {
            if (rest[i]->GetName() == targetName) { insertAt = i; break; }
        }
        // dragging downward drops BELOW the target row, upward drops ABOVE it
        int oldIdx = om->GetControllerIndex(dragged.front());
        int tgtIdx = om->GetControllerIndex(om->GetController(targetName));
        if (tgtIdx > oldIdx && insertAt < rest.size()) insertAt++;
    }

    std::vector<Controller*> desired(rest.begin(), rest.begin() + insertAt);
    desired.insert(desired.end(), dragged.begin(), dragged.end());
    desired.insert(desired.end(), rest.begin() + insertAt, rest.end());
    for (size_t i = 0; i < desired.size(); i++) {
        om->MoveController(desired[i], (int)i);
    }

    auto* omm = _frame->GetOutputModelManager();
    omm->AddImmediateWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerListPanel::Drop");
    omm->AddImmediateWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerListPanel::Drop");
    omm->AddImmediateWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerListPanel::Drop");
    omm->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerListPanel::Drop");
}

void ControllerListPanel::OnCharHook(wxKeyEvent& event) {
    if (event.GetKeyCode() == WXK_DELETE || event.GetKeyCode() == WXK_BACK) {
        DeleteSelectedControllers();
    } else if (event.GetKeyCode() == 'A' && (event.ControlDown() || event.CmdDown())) {
        _tree->SelectAll();
    } else {
        event.Skip();
    }
}

static const char* const NO_CONTROLLER_HINT =
    "No controller selected. Pick a controller in the list to see its properties, "
    "channel range and assigned models. The settings below apply to all controllers.";

wxWindow* ControllerListPanel::CreatePropertiesPanel(wxWindow* parent) {
    _propsPanel = new wxPanel(parent, wxID_ANY);
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

    _noSelectionHint = new wxStaticText(_propsPanel, wxID_ANY, "");
    _noSelectionHint->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    _propsPanel->Bind(wxEVT_SIZE, [this](wxSizeEvent& e) {
        if (_noSelectionHint->IsShown()) {
            _noSelectionHint->SetLabel(NO_CONTROLLER_HINT);
            _noSelectionHint->Wrap(std::max(120, e.GetSize().GetWidth() - 12));
            _noSelectionHint->InvalidateBestSize();
        }
        _propsPanel->InvalidateBestSize();
        e.Skip();
    });

    wxBoxSizer* headerSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* leftColSizer = new wxBoxSizer(wxVERTICAL);

    _ledPingBitmaps = CreateControllerLedBitmaps(20);
    _ledPing = new wxStaticBitmap(_propsPanel, wxID_ANY, _ledPingBitmaps.gray, wxDefaultPosition, FromDIP(wxSize(20, 20)));
    _ledPing->Disable();
    leftColSizer->Add(_ledPing, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 2);

    _btnAddController = new wxButton(_propsPanel, wxID_ANY, "Add");
    _btnAddController->SetToolTip("Add Controller");
    _btnAddController->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        if (!NetworkChangesAllowed()) return;
        wxMenu mnu;
        std::string ethernet = "Add E1.31/ArtNET/ZCPP/DDP";
        if (SpecialOptions::GetOption("xxx") == "true") {
            ethernet += "xxx";
        }
        mnu.Append(ID_CTRL_MNU_ADDETHERNET, ethernet);
        mnu.Append(ID_CTRL_MNU_ADDNULL, "Add NULL");
        mnu.Append(ID_CTRL_MNU_ADDSERIAL, "Add DMX/LOR/DLight/Renard");
        
        mnu.Bind(wxEVT_MENU, &ControllerListPanel::OnPopup, this);
        PopupMenu(&mnu);
    });
    leftColSizer->Add(_btnAddController, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 2);

    headerSizer->Add(leftColSizer, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    // wraps onto extra rows when the properties column is narrow; default
    // flags include wxEXTEND_LAST_ON_EACH_LINE, which would stretch the last
    // button on each row to fill the leftover width
    wxWrapSizer* buttonSizer = new wxWrapSizer(wxHORIZONTAL, wxWRAPSIZER_DEFAULT_FLAGS & ~wxEXTEND_LAST_ON_EACH_LINE);
    _btnVisualise = new wxButton(_propsPanel, wxID_ANY, "Visualise");
    _btnUploadInput = new wxButton(_propsPanel, wxID_ANY, "Upload Input");
    _btnUploadOutput = new wxButton(_propsPanel, wxID_ANY, "Upload Output");
    _btnOpen = new wxButton(_propsPanel, wxID_ANY, "Open");
    _btnOpenProxy = new wxButton(_propsPanel, wxID_ANY, "Open Proxy");
    for (wxButton* b : { _btnVisualise, _btnUploadInput, _btnUploadOutput, _btnOpen, _btnOpenProxy }) {
        buttonSizer->Add(b, 0, wxALL, 2);
        b->Enable(false);
    }
    // global actions — not tied to the selected controller
    _btnFPPConnect = new wxButton(_propsPanel, wxID_ANY, "FPP Connect");
    _btnDiscover = new wxButton(_propsPanel, wxID_ANY, "Discover");
    for (wxButton* b : { _btnFPPConnect, _btnDiscover }) {
        buttonSizer->Add(b, 0, wxALL, 2);
    }
    headerSizer->Add(buttonSizer, 1, wxEXPAND | wxALL, 2);
    topSizer->Add(headerSizer, 0, wxEXPAND | wxALL, 2);

    topSizer->Add(_noSelectionHint, 1, wxEXPAND | wxALL, 6);

    _propGrid = new xlPropertyGrid(_propsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                   wxPG_SPLITTER_AUTO_CENTER | wxPG_DEFAULT_STYLE);
    _propGrid->SetExtraStyle(wxWS_EX_PROCESS_IDLE | wxPG_EX_HELP_AS_TOOLTIPS);
    _propGrid->SetValidationFailureBehavior(wxPGVFBFlags::MarkCell | wxPGVFBFlags::Beep);
    _propGrid->AddActionTrigger(wxPGKeyboardAction::NextProperty, WXK_RETURN);
    _propGrid->DedicateKey(WXK_RETURN);
    _propGrid->AddActionTrigger(wxPGKeyboardAction::NextProperty, WXK_TAB);
    _propGrid->DedicateKey(WXK_TAB);
    _propGrid->Bind(wxEVT_PG_CHANGED, &ControllerListPanel::OnControllerPropertyGridChange, this);
    _propGrid->Bind(wxEVT_PG_ITEM_COLLAPSED, &ControllerListPanel::OnControllerPropertyGridCollapsed, this);
    _propGrid->Bind(wxEVT_PG_ITEM_EXPANDED, &ControllerListPanel::OnControllerPropertyGridExpanded, this);
    topSizer->Add(_propGrid, 1, wxEXPAND | wxALL, 2);

    _propsPanel->SetSizer(topSizer);

    _btnVisualise->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        if (!NetworkChangesAllowed()) return;
        auto controller = GetFirstSelectedController();
        if (controller != nullptr) {
            UDController cud(controller, _frame->GetOutputManager(), &_frame->AllModels, true);
            ControllerModelDialog dlg(_frame, &cud, &_frame->AllModels, controller);
            dlg.ShowModal();
        }
    });
    _btnUploadInput->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        if (!NetworkChangesAllowed()) return;
        auto names = GetSelectedControllerNames();
        if (!names.empty()) {
            _frame->UploadControllerInput(_frame->GetOutputManager()->GetController(names.front()));
            UpdateControllerProperties();
        }
    });
    _btnUploadOutput->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        if (!NetworkChangesAllowed()) return;
        auto names = GetSelectedControllerNames();
        if (!names.empty()) {
            _frame->UploadControllerOutput(_frame->GetOutputManager()->GetController(names.front()));
            UpdateControllerProperties();
        }
    });
    _btnOpen->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        if (!NetworkChangesAllowed()) return;
        auto controller = GetFirstSelectedController();
        if (controller != nullptr && controller->GetIP() != "") {
            if (controller->GetFPPProxy() != "" && controller->GetFPPProxy() != controller->GetIP()) {
                ::wxLaunchDefaultBrowser("http://" + controller->GetFPPProxy() + "/proxy/" + controller->GetIP() + "/");
            } else {
                ::wxLaunchDefaultBrowser("http://" + controller->GetIP());
            }
        }
    });
    _btnOpenProxy->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        if (!NetworkChangesAllowed()) return;
        auto controller = GetFirstSelectedController();
        if (controller != nullptr && controller->GetFPPProxy() != "") {
            ::wxLaunchDefaultBrowser("http://" + controller->GetFPPProxy());
        } else if (_frame->GetOutputManager()->GetGlobalFPPProxy() != "") {
            ::wxLaunchDefaultBrowser("http://" + _frame->GetOutputManager()->GetGlobalFPPProxy());
        }
    });
    _btnFPPConnect->Bind(wxEVT_BUTTON, [this](wxCommandEvent& e) {
        if (!NetworkChangesAllowed()) return;
        _frame->OnMenuItem_FPP_ConnectSelected(e);
    });
    _btnDiscover->Bind(wxEVT_BUTTON, [this](wxCommandEvent& e) {
        if (!NetworkChangesAllowed()) return;
        _frame->OnButtonDiscoverClick(e);
    });

    return _propsPanel;
}

void ControllerListPanel::UpdateControllerProperties() {
    if (_propGrid == nullptr) return;

    std::list<wxPGProperty*> expandProperties;
    auto* om = _frame->GetOutputManager();

    _propGrid->Freeze();
    auto save = _propGrid->SaveEditableState();
    wxString selProp = "";
    if (_propGrid->GetSelection() != nullptr) {
        selProp = _propGrid->GetSelection()->GetName();
    }

    auto selections = GetSelectedControllerNames();

    _btnFPPConnect->Enable(NetworkChangesAllowed());
    _btnDiscover->Enable(NetworkChangesAllowed());
    if (_btnAddController != nullptr) {
        _btnAddController->Enable(NetworkChangesAllowed());
    }

    bool haveController = selections.size() == 1 && om->GetController(selections.front()) != nullptr;
    wxSizer* sizer = _propsPanel->GetSizer();
    if (!haveController) {
        _noSelectionHint->SetLabel(NO_CONTROLLER_HINT);
        _noSelectionHint->Wrap(std::max(120, _propsPanel->GetClientSize().GetWidth() - 12));
        _noSelectionHint->InvalidateBestSize();
        sizer->Show(_noSelectionHint, true);
        sizer->Show(_propGrid, false);
        _propsPanel->Layout();
    } else {
        sizer->Show(_noSelectionHint, false);
        sizer->Show(_propGrid, true);
        _propsPanel->Layout();
    }

    if (!haveController) {
        _adapter.reset();
        _propGrid->Clear();
        _ledPing->Disable();
        _ledPing->SetBitmap(_ledPingBitmaps.gray);
        _btnVisualise->Enable(false);
        _btnUploadInput->Show(_frame->GetLinkedControllerUpload() == "None");
        _btnUploadInput->Enable(false);
        _btnUploadOutput->Enable(false);
        _btnOpen->Enable(false);
        bool showOpenProxy = (om->GetGlobalFPPProxy() != "");
        _btnOpenProxy->Show(showOpenProxy);
        _btnOpenProxy->Enable(NetworkChangesAllowed() && showOpenProxy);

        wxPGProperty* p = _propGrid->Append(new wxBoolProperty("Controller Sync", "ControllerSync", _frame->E131Sync()));
        p->SetEditor("CheckBox");
        p->SetHelpString("Sends a sync packet at the end of each frame for controllers to synchronise light change to. Supported by E1.31, ArtNET and ZCPP. Controller support varies.");

        if (_frame->E131Sync()) {
            p = _propGrid->Append(new wxUIntProperty("E1.31 Sync Universe", "E131SyncUniverse", om->GetSyncUniverse()));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 64000);
            p->SetEditor("SpinCtrl");
        }
        p = _propGrid->Append(new wxUIntProperty("Max Duplicate Frames To Suppress", "MaxSuppressFrames", om->GetSuppressFrames()));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 1000);
        p->SetEditor("SpinCtrl");

        auto const ips = ip_utils::GetLocalIPs();
        wxPGChoices choices;
        int val = 0;
        choices.Add("");
        for (const auto& it : ips) {
            if (it == om->GetGlobalForceLocalIP()) val = choices.GetCount();
            choices.Add(it);
        }
        _propGrid->Append(new wxEnumProperty("Global Force Local IP", "ForceLocalIP", choices, val));
        _propGrid->Append(new wxStringProperty("Global FPP Proxy", "GlobalFPPProxy", om->GetGlobalFPPProxy()));

        if (!NetworkChangesAllowed()) {
            _propGrid->SetToolTip("Properties cannot be edited when outputting to lights.");
            auto it = _propGrid->GetIterator(wxPG_ITERATE_ALL, nullptr);
            while (!it.AtEnd()) {
                auto* prop = it.GetProperty();
                prop->Enable(false);
                prop->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
                it.Next(true);
            }
        } else {
            _propGrid->UnsetToolTip();
        }
    } else {
        auto controller = om->GetController(selections.front());
        if (controller != nullptr) {
            int usingip = om->GetControllerCount(controller->GetType(), controller->GetColumn2Label());

            _ledPing->Enable();
            RefreshPingIndicator(controller);

            bool allowed = NetworkChangesAllowed();
            _btnVisualise->Enable(allowed && usingip == 1 && controller->CanVisualise());

            auto eth = dynamic_cast<ControllerEthernet*>(controller);
            auto caps = _frame->GetControllerCaps(selections.front());
            bool showUploadInput = (_frame->GetLinkedControllerUpload() == "None");
            _btnUploadInput->Show(showUploadInput);
            if (showUploadInput) {
                bool supportsInputUpload = allowed && (caps != nullptr && caps->SupportsUpload() && usingip == 1 &&
                                            caps->SupportsInputOnlyUpload() &&
                                            (eth == nullptr || ((eth->GetProtocol() != OUTPUT_DDP || caps->NeedsDDPInputUpload()) && eth->GetProtocol() != OUTPUT_ZCPP)));
                _btnUploadInput->Enable(supportsInputUpload);
            } else {
                _btnUploadInput->Enable(false);
            }

            _btnUploadOutput->Enable(allowed && usingip == 1 && _frame->ControllerSupportsOutputUpload(controller));
            _btnOpen->Enable(allowed && eth != nullptr && eth->GetIP() != "MULTICAST" && eth->GetIP() != "" && (caps == nullptr || !caps->NoWebUI()));

            bool showOpenProxy = (eth != nullptr && eth->GetFPPProxy() != "");
            _btnOpenProxy->Show(showOpenProxy);
            _btnOpenProxy->Enable(allowed && showOpenProxy);

            if (!_adapter || _adapter->GetController() != controller) {
                _propGrid->Clear();
                _adapter = ControllerPropertyManager::CreateAdapter(*controller);
                _adapter->AddProperties(_propGrid, &_frame->AllModels, expandProperties);
            }
            if (_adapter) {
                _adapter->UpdateProperties(_propGrid, &_frame->AllModels, expandProperties, _frame->GetOutputModelManager());
            }

            {
                auto* config = GetXLightsConfig();
                auto ctrlName = controller->GetName();

                wxPGProperty* p = _propGrid->GetProperty("LastInputUpload");
                if (!p) {
                    p = _propGrid->Append(new wxStringProperty("Last Input Upload", "LastInputUpload", "Never"));
                }
                p->ChangeFlag(wxPGFlags::ReadOnly, true);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
                wxString ts;
                if (!config->Read(MakeControllerTimestampKey("LastInputUpload", ctrlName, _frame->GetShowDirectory()), &ts)) ts = "Never";
                p->SetValue(ts);

                p = _propGrid->GetProperty("LastOutputUpload");
                if (!p) {
                    p = _propGrid->Append(new wxStringProperty("Last Output Upload", "LastOutputUpload", "Never"));
                }
                p->ChangeFlag(wxPGFlags::ReadOnly, true);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
                if (!config->Read(MakeControllerTimestampKey("LastOutputUpload", ctrlName, _frame->GetShowDirectory()), &ts)) ts = "Never";
                p->SetValue(ts);
            }

            if (controller->IsFromBase() || !allowed) {
                if (controller->IsFromBase()) {
                    _propGrid->SetToolTip("This controller comes from the base folder and its properties cannot be edited.");
                } else {
                    _propGrid->SetToolTip("Properties cannot be edited when outputting to lights.");
                }
                auto it = _propGrid->GetIterator(wxPG_ITERATE_ALL, nullptr);
                while (!it.AtEnd()) {
                    auto* p = it.GetProperty();
                    p->Enable(false);
                    p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
                    if (p->GetValue().GetType() == "bool") {
                        p->SetEditor("Choice");
                    }
                    it.Next(true);
                }
            } else {
                _propGrid->UnsetToolTip();
                auto it = _propGrid->GetIterator(wxPG_ITERATE_ALL, nullptr);
                while (!it.AtEnd()) {
                    auto* p = it.GetProperty();
                    p->Enable(true);
                    p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
                    if (p->GetValue().GetType() == "bool") {
                        p->SetEditor("CheckBox");
                    }
                    it.Next(true);
                }
            }
        }
    }

    _propGrid->RestoreEditableState(save);
    if (selProp != "") {
        auto p = _propGrid->GetPropertyByName(selProp);
        if (p != nullptr) _propGrid->EnsureVisible(p);
    }

    if (_adapter && _propGrid->GetPropertyByName("ControllerName") != nullptr) {
        _adapter->ValidateProperties(om, _propGrid);
    }

    _propGrid->Thaw();

    if (expandProperties.size() > 0) {
        for (const auto& it : expandProperties) {
            it->SetExpanded(true);
        }
        _propGrid->Freeze();
        expandProperties.front()->Hide(true);
        expandProperties.front()->Hide(false);
        _propGrid->Thaw();
    }
    if (_propsPanel != nullptr) {
        _propsPanel->InvalidateBestSize();
        _propsPanel->Layout();
    }
}

void ControllerListPanel::OnControllerPropertyGridCollapsed(wxPropertyGridEvent& event) {
    if (GetSelectedControllerNames().size() == 1 && _adapter) {
        _adapter->HandleExpanded(event, false);
        _frame->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerListPanel::OnControllerPropertyGridCollapsed");
    }
}

void ControllerListPanel::OnControllerPropertyGridExpanded(wxPropertyGridEvent& event) {
    if (GetSelectedControllerNames().size() == 1 && _adapter) {
        _adapter->HandleExpanded(event, true);
        _frame->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerListPanel::OnControllerPropertyGridExpanded");
    }
}

void ControllerListPanel::OnControllerPropertyGridChange(wxPropertyGridEvent& event) {
    if (!NetworkChangesAllowed()) {
        return;
    }
    wxString name = event.GetPropertyName();
    auto* om = _frame->GetOutputManager();
    auto* omm = _frame->GetOutputModelManager();
    auto selections = GetSelectedControllerNames();

    if (selections.size() == 1) {
        auto controllername = selections.front();
        auto controller = om->GetController(controllername);
        if (controller == nullptr) return;

        std::string oldName = controllername;
        std::string oldIP = controller->GetIP();

        auto processed = _adapter ? _adapter->HandlePropertyEvent(event, omm) : false;

        if (name == "ControllerName") {
            if (processed) {
                for (wxTreeListItem item = _tree->GetFirstChild(_tree->GetRootItem()); item.IsOk(); item = _tree->GetNextSibling(item)) {
                    if (_tree->GetItemText(item, 0) == oldName) {
                        _tree->SetItemText(item, 0, event.GetValue().GetString());
                        break;
                    }
                }
                _frame->AbortRender();
                _frame->AllModels.RenameController(oldName, ToStdString(event.GetValue().GetString()));
                omm->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ControllerListPanel::OnControllerPropertyGridChange::ControllerName", nullptr);
            }
        } else if (name == "IP") {
            if (ip_utils::IsIPValid(oldIP) && ip_utils::IsIPValid(controller->GetIP()) && om->GetControllers(oldIP).size() == 0) {
                _frame->AbortRender();
                _frame->AllModels.ReplaceIPInStartChannels(oldIP, controller->GetIP());
            }
        }
    } else {
        if (name == "ControllerSync") {
            _frame->SetE131Sync(event.GetValue().GetBool());
            UpdateControllerProperties();
            omm->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerListPanel::OnControllerPropertyGridChange::ControllerSync");
        } else if (name == "E131SyncUniverse") {
            om->SetSyncUniverse((int)event.GetValue().GetLong());
            omm->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerListPanel::OnControllerPropertyGridChange::E131SyncUniverse");
        } else if (name == "MaxSuppressFrames") {
            _frame->SetSuppressDuplicateFrames((int)event.GetValue().GetLong());
            omm->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerListPanel::OnControllerPropertyGridChange::MaxSuppressFrames");
        } else if (name == "GlobalFPPProxy") {
            om->SetGlobalFPPProxy(ToStdString(event.GetValue().GetString().Trim(true).Trim(false)));
            omm->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerListPanel::OnControllerPropertyGridChange::GlobalFPPProxy");
            omm->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerListPanel::OnControllerPropertyGridChange::GlobalFPPProxy", nullptr);
        } else if (name == "ForceLocalIP") {
            auto ips = ip_utils::GetLocalIPs();
            if (event.GetValue().GetLong() == 0) {
                om->SetGlobalForceLocalIP("");
            } else if ((size_t)event.GetValue().GetLong() < ips.size() + 1) {
                auto it = begin(ips);
                std::advance(it, event.GetValue().GetLong() - 1);
                om->SetGlobalForceLocalIP(*it);
            }
            omm->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerListPanel::OnControllerPropertyGridChange::ForceLocalIP");
            _frame->CycleOutputsIfOn();
        }
    }

    if (!omm->IsASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST) && _adapter && _propGrid->GetPropertyByName("ControllerName") != nullptr) {
        _adapter->ValidateProperties(om, _propGrid);
    }
}

void ControllerListPanel::ExpandAllControllers() {
    _tree->Freeze();
    for (wxTreeListItem ctrlItem = _tree->GetFirstChild(_tree->GetRootItem()); ctrlItem.IsOk(); ctrlItem = _tree->GetNextSibling(ctrlItem)) {
        PopulatePorts(ctrlItem);
        _tree->Expand(ctrlItem);
    }
    _tree->Thaw();
}

void ControllerListPanel::CollapseAll() {
    _tree->Freeze();
    for (wxTreeListItem ctrlItem = _tree->GetFirstChild(_tree->GetRootItem()); ctrlItem.IsOk(); ctrlItem = _tree->GetNextSibling(ctrlItem)) {
        for (wxTreeListItem portItem = _tree->GetFirstChild(ctrlItem); portItem.IsOk(); portItem = _tree->GetNextSibling(portItem)) {
            _tree->Collapse(portItem);
        }
        _tree->Collapse(ctrlItem);
    }
    _tree->Thaw();
}
