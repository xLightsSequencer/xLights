/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*InternalHeaders(xLightsImportChannelMapDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>
#include <wx/colordlg.h>

#include "xLightsImportChannelMapDialog.h"
#include "sequencer/SequenceElements.h"
#include "xLightsMain.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "UtilFunctions.h"

#include <log4cpp/Category.hh>

wxDEFINE_EVENT(EVT_MDDROP, wxCommandEvent);

int wxCALLBACK MyCompareFunctionAsc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData)
{
    return item1 == item2 ? 0 : ((item1 < item2) ? -1 : 1);
}

int wxCALLBACK MyCompareFunctionDesc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData)
{
    return item1 == item2 ? 0 : ((item1 < item2) ? 1 : -1);
}

class MDDropSource : public wxDropSource
{
    xLightsImportChannelMapDialog* _window;

public:

    MDDropSource(xLightsImportChannelMapDialog* window) : wxDropSource(window)
    {
        _window = window;
    }

    virtual bool GiveFeedback(wxDragResult effect) override
    {
        int sx;
        int sy;
        _window->GetScreenPosition(&sx, &sy);

        wxRect rect = _window->TreeListCtrl_Mapping->GetItemRect(((xLightsImportTreeModel*)_window->TreeListCtrl_Mapping->GetModel())->GetNthItem(0));

        wxPoint point = wxGetMousePosition() - wxPoint(sx, sy);
        wxPoint pointTree = wxGetMousePosition() - wxPoint(sx, sy+rect.GetHeight());

        if (_window->TreeListCtrl_Mapping->GetRect().Contains(pointTree) ||
            _window->ListCtrl_Available->GetRect().Contains(point))
        {
            _window->SetCursor(wxCursor(wxCURSOR_HAND));
        }
        else
        {
            _window->SetCursor(wxCursor(wxCURSOR_NO_ENTRY));
        }

        return true;
    }
};

class ColorRenderer : public wxDataViewCustomRenderer
{
    static wxColourData _colorData;
    wxColor _color;

public:
    ColorRenderer() : wxDataViewCustomRenderer(GetDefaultType(), wxDATAVIEW_CELL_ACTIVATABLE)
    {
        _color = *wxWHITE;
    }

    virtual bool ActivateCell(const wxRect &cell, wxDataViewModel *model, const wxDataViewItem &item, unsigned int col, const wxMouseEvent *mouseEvent) override
    {
        _colorData.SetColour(_color);
        wxColourDialog dlg(GetOwner()->GetOwner()->GetParent(), &_colorData);

        if (dlg.ShowModal() == wxID_OK)
        {
            _colorData = dlg.GetColourData();
            _color = dlg.GetColourData().GetColour();
            model->SetValue(wxVariant(_color.GetAsString()), item, col);
        }

        return false;
    }

    virtual bool Render(wxRect cell, wxDC *dc, int state) override
    {
        wxPen p(_color);
        wxBrush b(_color);
        dc->SetPen(p);
        dc->SetBrush(b);
        dc->DrawRectangle(cell);
        dc->SetBrush(wxNullBrush);
        dc->SetPen(wxNullPen);
        return true;
    }

    virtual wxSize 	GetSize() const override
    {
        return wxSize(GetOwner()->GetWidth(), 15);
    }

    virtual bool GetValue(wxVariant &value) const override
    {
        value = wxVariant(_color.GetAsString());
        return true;
    }

    virtual bool SetValue(const wxVariant &value) override
    {
        if (value.GetString() != "")
        {
            _color = wxColor(value.GetString());
            return true;
        }
        return false;
    }
};

wxColourData ColorRenderer::_colorData;


xLightsImportTreeModel::xLightsImportTreeModel()
{
}

bool xLightsImportTreeModel::GetAttr(const wxDataViewItem &item, unsigned int col, wxDataViewItemAttr &attr) const
{
    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    if (!node)      // happens if item.IsOk()==false
        return false;

    bool set = false;

    if (!node->_mappingExists)
    {
        attr.SetBackgroundColour(*wxRED);
        set = true;
    }

    if (node->IsGroup())
    {
        attr.SetColour(*wxBLUE);
        set = true;
    }

    return set;
}

int xLightsImportTreeModel::Compare(const wxDataViewItem& item1, const wxDataViewItem& item2, unsigned column, bool ascending) const
{
    if (column == 0)
    {
        xLightsImportModelNode *node1 = (xLightsImportModelNode*)item1.GetID();
        xLightsImportModelNode *node2 = (xLightsImportModelNode*)item2.GetID();

        if (node1->_node != "" && node2->_node != "")
        {
            if (ascending)
                return NumberAwareStringCompare(node1->_node.ToStdString(), node2->_node.ToStdString());
            else
                return NumberAwareStringCompareRev(node1->_node.ToStdString(), node2->_node.ToStdString());
        }
        else if (node1->_strand != "" && node2->_strand != "")
        {
            if (ascending)
                return NumberAwareStringCompare(node1->_strand.ToStdString(), node2->_strand.ToStdString());
            else
                return NumberAwareStringCompareRev(node1->_strand.ToStdString(), node2->_strand.ToStdString());
        }
        else
        {
            if (ascending)
                return NumberAwareStringCompare(GetModel(item1).ToStdString(), GetModel(item2).ToStdString());
            else
                return NumberAwareStringCompareRev(GetModel(item1).ToStdString(), GetModel(item2).ToStdString());
        }
    }

    return 0;
}

wxString xLightsImportTreeModel::GetModel(const wxDataViewItem &item) const
{
    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    if (!node)      // happens if item.IsOk()==false
        return wxEmptyString;

    return node->_model;
}

wxString xLightsImportTreeModel::GetStrand(const wxDataViewItem &item) const
{
    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    if (!node)      // happens if item.IsOk()==false
        return wxEmptyString;

    return node->_strand;
}

wxString xLightsImportTreeModel::GetNode(const wxDataViewItem &item) const
{
    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    if (!node)      // happens if item.IsOk()==false
        return wxEmptyString;

    return node->_node;
}

wxString xLightsImportTreeModel::GetMapping(const wxDataViewItem &item) const
{
    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    if (!node)      // happens if item.IsOk()==false
        return wxEmptyString;

    return node->_mapping;
}

wxColor xLightsImportTreeModel::GetColor(const wxDataViewItem &item) const
{
    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    if (!node)      // happens if item.IsOk()==false
        return *wxWHITE;

    return node->_color;
}

void xLightsImportTreeModel::Delete(const wxDataViewItem &item)
{
    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    if (!node)      // happens if item.IsOk()==false
        return;

    wxDataViewItem parent(node->GetParent());
    // first remove the node from the parent's array of children;
    // NOTE: xLightsImportModelNodePtrArray is only an array of _pointers_
    //       thus removing the node from it doesn't result in freeing it
    if (node->GetParent() != nullptr)
    {
        node->GetParent()->GetChildren().Remove(node);
    }

    // free the node
    delete node;

    // notify control
    ItemDeleted(parent, item);
}

void xLightsImportTreeModel::GetValue(wxVariant &variant,
    const wxDataViewItem &item, unsigned int col) const
{
    wxASSERT(item.IsOk());

    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    xLightsImportModelNode *parent = node->GetParent();
    switch (col)
    {
    case 0:
        if (parent == nullptr)
        {
            //variant = wxVariant(wxDataViewIconText(node->_model));
            variant = wxVariant(node->_model);
        }
        else if (node->GetChildCount() != 0 || "" == node->_node)
        {
            //variant = wxVariant(wxDataViewIconText(node->_strand));
            variant = wxVariant(node->_strand);
        }
        else
        {
            variant = wxVariant(node->_node);
        }
        break;
    case 1:
        variant = wxVariant(node->_mapping);
        break;
    case 2:
        variant = wxVariant(node->_color.GetAsString());
        break;
    default:
        {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.warn("xLightsImportTreeModel::GetValue: wrong column %d", col);
            wxLogError("xLightsImportTreeModel::GetValue: wrong column %d", col);
        }
    }
}

bool xLightsImportTreeModel::SetValue(const wxVariant &variant,
    const wxDataViewItem &item, unsigned int col)
{
    wxASSERT(item.IsOk());

    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    if (col == 1)
    {
        node->_mapping = variant.GetString();
        node->_mappingExists = false;
        return true;
    }
    else if (col == 2)
    {
        node->_color = wxColour(variant.GetString());
        return true;
    }
    return false;
}

void xLightsImportTreeModel::SetMappingExists(const wxDataViewItem &item, bool exists)
{
    wxASSERT(item.IsOk());

    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    node->_mappingExists = exists;
}

wxDataViewItem xLightsImportTreeModel::GetParent(const wxDataViewItem &item) const
{
    // the invisible root node has no parent
    if (!item.IsOk())
        return wxDataViewItem(0);

    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();

    if (node->GetParent() == nullptr)
        return wxDataViewItem(0);

    return wxDataViewItem((void*)node->GetParent());
}

bool xLightsImportTreeModel::IsContainer(const wxDataViewItem &item) const
{
    // the invisble root node can have children
    if (!item.IsOk())
        return true;

    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    return node->IsContainer();
}

unsigned int xLightsImportTreeModel::GetChildren(const wxDataViewItem &parent,
    wxDataViewItemArray &array) const
{
    unsigned int count;
    xLightsImportModelNode *node = (xLightsImportModelNode*)parent.GetID();
    if (!node)
    {
        count = m_children.size();
        for (unsigned int pos = 0; pos < count; pos++)
        {
            xLightsImportModelNode *child = m_children.Item(pos);
            array.Add(wxDataViewItem((void*)child));
        }
    }
    else
    {
        if (node->GetChildCount() == 0)
        {
            return 0;
        }

        count = node->GetChildren().GetCount();
        for (unsigned int pos = 0; pos < count; pos++)
        {
            xLightsImportModelNode *child = node->GetChildren().Item(pos);
            array.Add(wxDataViewItem((void*)child));
        }
    }
    return count;
}

wxDataViewItem xLightsImportTreeModel::GetNthItem(unsigned int n) const
{
    unsigned int count = m_children.size();
    for (unsigned int pos = 0; pos < count; pos++)
    {
        if (pos == n)
        {
            xLightsImportModelNode *child = m_children.Item(pos);
            return wxDataViewItem((void*)child);
        }
    }

    return wxDataViewItem(nullptr);
}

void xLightsImportTreeModel::ClearMapping()
{
    size_t count = m_children.size();
    for (size_t i = 0; i < count; i++)
    {
        GetNthChild(i)->ClearMapping();
    }
}

const long xLightsImportChannelMapDialog::ID_TREELISTCTRL1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_CHOICE = wxNewId();

//(*IdInit(xLightsImportChannelMapDialog)
const long xLightsImportChannelMapDialog::ID_SPINCTRL1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_CHECKBOX1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_CHECKLISTBOX1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON3 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON4 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON5 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON2 = wxNewId();
const long xLightsImportChannelMapDialog::ID_PANEL1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_LISTCTRL1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_PANEL2 = wxNewId();
const long xLightsImportChannelMapDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(xLightsImportChannelMapDialog,wxDialog)
	//(*EventTable(xLightsImportChannelMapDialog)
	//*)
END_EVENT_TABLE()

xLightsImportChannelMapDialog::xLightsImportChannelMapDialog(wxWindow* parent, const wxFileName& filename, bool allowTimingOffset, bool allowTimingTrack, bool allowColorChoice, bool allowCCRStrand, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    TreeListCtrl_Mapping = nullptr;
    _dataModel = nullptr;
    _sortOrder = 0;
    _allowTimingOffset = allowTimingOffset;
    _allowTimingTrack = allowTimingTrack;
    _allowColorChoice = allowColorChoice;
    _allowCCRStrand = allowCCRStrand;
    _filename = filename;
    _dragItem = wxDataViewItem(nullptr);

    //(*Initialize(xLightsImportChannelMapDialog)
    wxButton* Button01;
    wxButton* Button02;
    wxFlexGridSizer* FlexGridSizer2;

    Create(parent, wxID_ANY, _("Map Channels"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX, _T("wxID_ANY"));
    OldSizer = new wxFlexGridSizer(0, 1, 0, 0);
    OldSizer->AddGrowableCol(0);
    OldSizer->AddGrowableRow(0);
    SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
    SplitterWindow1->SetMinSize(wxSize(10, 10));
    SplitterWindow1->SetSashGravity(0.5);
    Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    Sizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    Sizer1->AddGrowableCol(0);
    Sizer1->AddGrowableRow(3);
    Sizer_TimeAdjust = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText_TimeAdjust = new wxStaticText(Panel1, wxID_ANY, _("Time Adjust (ms)"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    Sizer_TimeAdjust->Add(StaticText_TimeAdjust, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    TimeAdjustSpinCtrl = new wxSpinCtrl(Panel1, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -10000, 600000, 0, _T("ID_SPINCTRL1"));
    TimeAdjustSpinCtrl->SetValue(_T("0"));
    Sizer_TimeAdjust->Add(TimeAdjustSpinCtrl, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Sizer1->Add(Sizer_TimeAdjust, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    CheckBox_MapCCRStrand = new wxCheckBox(Panel1, ID_CHECKBOX1, _("Map CCR/Strand"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_MapCCRStrand->SetValue(false);
    FlexGridSizer1->Add(CheckBox_MapCCRStrand, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Sizer1->Add(FlexGridSizer1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    TimingTrackPanel = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Timing Tracks"));
    TimingTrackListBox = new wxCheckListBox(Panel1, ID_CHECKLISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, wxVSCROLL, wxDefaultValidator, _T("ID_CHECKLISTBOX1"));
    TimingTrackPanel->Add(TimingTrackListBox, 1, wxALL | wxEXPAND, 0);
    Sizer1->Add(TimingTrackPanel, 0, wxEXPAND, 0);
    SizerMap = new wxFlexGridSizer(1, 1, 0, 0);
    SizerMap->AddGrowableCol(0);
    SizerMap->AddGrowableRow(0);
    Sizer1->Add(SizerMap, 0, wxEXPAND, 0);
    FlexGridSizer2 = new wxFlexGridSizer(0, 7, 0, 0);
    FlexGridSizer2->AddGrowableCol(2);
    Button_Ok = new wxButton(Panel1, ID_BUTTON3, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer2->Add(Button_Ok, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Button_Cancel = new wxButton(Panel1, ID_BUTTON4, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer2->Add(Button_Cancel, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(-1, -1, 1, wxALL | wxEXPAND, 5);
    Button_AutoMap = new wxButton(Panel1, ID_BUTTON5, _("Auto Map"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    FlexGridSizer2->Add(Button_AutoMap, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(-1, -1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Button01 = new wxButton(Panel1, ID_BUTTON1, _("Load Mapping"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer2->Add(Button01, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Button02 = new wxButton(Panel1, ID_BUTTON2, _("Save Mapping"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer2->Add(Button02, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Sizer1->Add(FlexGridSizer2, 1, wxALL | wxEXPAND, 0);
    Panel1->SetSizer(Sizer1);
    Sizer1->Fit(Panel1);
    Sizer1->SetSizeHints(Panel1);
    Panel2 = new wxPanel(SplitterWindow1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    Sizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    Sizer2->AddGrowableCol(0);
    Sizer2->AddGrowableRow(0);
    ListCtrl_Available = new wxListCtrl(Panel2, ID_LISTCTRL1, wxDefaultPosition, wxDLG_UNIT(Panel2, wxSize(100, -1)), wxLC_REPORT | wxLC_SINGLE_SEL | wxVSCROLL, wxDefaultValidator, _T("ID_LISTCTRL1"));
    Sizer2->Add(ListCtrl_Available, 1, wxALL | wxEXPAND, 5);
    Panel2->SetSizer(Sizer2);
    Sizer2->Fit(Panel2);
    Sizer2->SetSizeHints(Panel2);
    SplitterWindow1->SplitVertically(Panel1, Panel2);
    OldSizer->Add(SplitterWindow1, 1, wxALL | wxEXPAND, 5);
    SetSizer(OldSizer);
    OldSizer->Fit(this);
    OldSizer->SetSizeHints(this);

    Connect(ID_CHECKBOX1, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnCheckBox_MapCCRStrandClick);
    Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnButton_OkClick);
    Connect(ID_BUTTON4, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnButton_CancelClick);
    Connect(ID_BUTTON5, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnButton_AutoMapClick);
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::LoadMapping);
    Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::SaveMapping);
    Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_BEGIN_DRAG, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnListCtrl_AvailableBeginDrag);
    Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_ITEM_SELECTED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnListCtrl_AvailableItemSelect);
    Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_ITEM_ACTIVATED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnListCtrl_AvailableItemActivated);
    Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_COL_CLICK, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnListCtrl_AvailableColumnClick);
    //*)

    SetSize(800, 600);

    ListCtrl_Available->SetSize(150, -1);
    if (ListCtrl_Available->GetColumnCount() > 0)
    {
        ListCtrl_Available->SetColumnWidth(0, wxLIST_AUTOSIZE);
        if (ListCtrl_Available->GetColumnWidth(0) < 150)
        {
            ListCtrl_Available->SetColumnWidth(0, 150);
        }
    }

    if (_filename != "")
    {
        SetLabel(GetLabel() + " - " + _filename.GetFullName());
    }

    Connect(wxID_ANY, EVT_MDDROP, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnDrop);

    MDTextDropTarget* mdt = new MDTextDropTarget(this, ListCtrl_Available, "Remove");
    ListCtrl_Available->SetDropTarget(mdt);

    _dirty = false;

    SetSize(1200, 800);
    wxPoint loc;
    wxSize sz;
    LoadWindowPosition("xLightsImportDialogPosition", sz, loc);
    if (loc.x != -1)
    {
        if (sz.GetWidth() < 400) sz.SetWidth(400);
        if (sz.GetHeight() < 300) sz.SetHeight(300);
        SetPosition(loc);
        SetSize(sz);
        Layout();
    }

    EnsureWindowHeaderIsOnScreen(this);
}

xLightsImportChannelMapDialog::~xLightsImportChannelMapDialog()
{
	//(*Destroy(xLightsImportChannelMapDialog)
	//*)

    // disconnect the model and then delete it ... this ensures the destructors are called
    // which stops memory leaks
    if (TreeListCtrl_Mapping != nullptr)
    {
        TreeListCtrl_Mapping->AssociateModel(nullptr);
        if (_dataModel != nullptr)
        {
            delete _dataModel;
            _dataModel = nullptr;
        }
    }

    // clear any stashed mappings
    while (_stashedMappings.size() > 0)
    {
        delete _stashedMappings.front();
        _stashedMappings.pop_front();
    }

    SaveWindowPosition("xLightsImportDialogPosition", this);
}

bool xLightsImportChannelMapDialog::InitImport() {
    if (channelNames.size() == 0)
    {
        DisplayError("No models to import from. Source sequence had no data.");
        return false;
    }

    if (!_allowCCRStrand)
    {
        Sizer1->Hide(FlexGridSizer1, true);
    }

    if (!_allowTimingOffset)
    {
        Sizer1->Hide(Sizer_TimeAdjust, true);
    }

    if (timingTracks.empty() || !_allowTimingTrack)
    {
        Sizer1->Hide(TimingTrackPanel, true);
    }
    else
    {
        for (auto it = timingTracks.begin(); it != timingTracks.end(); ++it) {
            TimingTrackListBox->Append(*it);
        }
    }

    PopulateAvailable(false);

    _dataModel = new xLightsImportTreeModel();

    TreeListCtrl_Mapping = new wxDataViewCtrl(Panel1, ID_TREELISTCTRL1, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES | wxDV_VERT_RULES, wxDefaultValidator);
    TreeListCtrl_Mapping->AssociateModel(_dataModel);
    TreeListCtrl_Mapping->AppendColumn(new wxDataViewColumn("Model", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT, wxALIGN_LEFT), 0, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE));
    TreeListCtrl_Mapping->GetColumn(0)->SetSortOrder(true);
    TreeListCtrl_Mapping->AppendColumn(new wxDataViewColumn("Map To", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_ACTIVATABLE, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL), 1, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
    if (_allowColorChoice)
    {
        TreeListCtrl_Mapping->AppendColumn(new wxDataViewColumn("Color", new ColorRenderer(), 2, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
    }

    TreeListCtrl_Mapping->SetMinSize(wxSize(0, 300));
    SizerMap->Add(TreeListCtrl_Mapping, 1, wxALL | wxEXPAND, 5);
    SizerMap->Layout();
    Sizer1->Layout();
    Sizer2->Layout();
    Layout();

    Connect(ID_TREELISTCTRL1, wxEVT_DATAVIEW_SELECTION_CHANGED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnSelectionChanged);
    Connect(ID_TREELISTCTRL1, wxEVT_DATAVIEW_ITEM_ACTIVATED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnItemActivated);
    Connect(ID_TREELISTCTRL1, wxEVT_DATAVIEW_ITEM_VALUE_CHANGED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnValueChanged);
    Connect(ID_TREELISTCTRL1, wxEVT_DATAVIEW_ITEM_BEGIN_DRAG, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnBeginDrag);
    // This does not work ... I suspect the control is not letting it through
    //Connect(ID_TREELISTCTRL1, wxEVT_KEY_DOWN, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnKeyDown);

    
#ifdef __WXOSX__
    Connect(ID_TREELISTCTRL1, wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnDragPossible);
    Connect(ID_TREELISTCTRL1, wxEVT_DATAVIEW_ITEM_DROP, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnDragDrop);
#else
    MDTextDropTarget* mdt = new MDTextDropTarget(this, TreeListCtrl_Mapping, "Map");
    TreeListCtrl_Mapping->SetDropTarget(mdt);
#endif
    TreeListCtrl_Mapping->SetIndent(8);

    int ms = 0;
    for (size_t i = 0; i < mSequenceElements->GetElementCount(); ++i) {
        if (mSequenceElements->GetElement(i)->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            Element* e = mSequenceElements->GetElement(i);

            Model *m = xlights->GetModel(e->GetName());
            if (m != nullptr) {
                AddModel(m, ms);
            }
        }
    }

    if (_dataModel->GetChildCount() == 0)
    {
        DisplayError("No models to import to. Add some models to the rows of the effects grid.");
        return false;
    }
    else
    {
        _dataModel->Resort();
    }

    return true;
}

void xLightsImportChannelMapDialog::PopulateAvailable(bool ccr)
{
    ListCtrl_Available->Freeze();
    ListCtrl_Available->ClearAll();

    // load the available list
    ListCtrl_Available->AppendColumn("Available");
    ListCtrl_Available->SetColumnWidth(0, wxLIST_AUTOSIZE);
    if (ListCtrl_Available->GetColumnWidth(0) < 150)
    {
        ListCtrl_Available->SetColumnWidth(0, 150);
    }

    if (ccr)
    {
        int j = 0;
        for (auto it = ccrNames.begin(); it != ccrNames.end(); ++it)
        {
            wxString name = *it;
            ListCtrl_Available->InsertItem(j, name);
            ListCtrl_Available->SetItemData(j, j);
            j++;
        }
    }
    else
    {
        int j = 0;
        for (auto it = channelNames.begin(); it != channelNames.end(); ++it)
        {
            wxString name = *it;
            ListCtrl_Available->InsertItem(j, name);
            ListCtrl_Available->SetItemData(j, j);
            j++;
        }
    }

    _sortOrder = 1;
    ListCtrl_Available->SortItems(MyCompareFunctionAsc, (wxIntPtr)ListCtrl_Available);

    ListCtrl_Available->Thaw();
    ListCtrl_Available->Update();
}

void xLightsImportChannelMapDialog::AddModel(Model *m, int &ms) {

    if (m == nullptr) return;

    for (size_t x = 0; x < _dataModel->GetChildCount(); ++x) {
        xLightsImportModelNode * tmp = _dataModel->GetNthChild(x);
        if (tmp != nullptr && tmp->_model == m->GetName()) {
            return;
        }
    }

    xLightsImportModelNode *lastmodel = new xLightsImportModelNode(nullptr, m->GetName(), std::string(""), true, *wxWHITE, (m->GetDisplayAs() == "ModelGroup"));
    _dataModel->Insert(lastmodel, ms++);

    for (int s = 0; s < m->GetNumSubModels(); s++) {
        Model *subModel = m->GetSubModel(s);
        xLightsImportModelNode* laststrand;
        if (channelColors.find(subModel->GetName()) != channelColors.end())
        {
            laststrand = new xLightsImportModelNode(lastmodel, m->GetName(), subModel->GetName(), std::string(""), true, channelColors.find(subModel->GetName())->second.asWxColor());
        }
        else
        {
            laststrand = new xLightsImportModelNode(lastmodel, m->GetName(), subModel->GetName(), std::string(""), true, *wxWHITE);
        }
        lastmodel->Append(laststrand);
    }

    for (int s = 0; s < m->GetNumStrands(); s++) {
        wxString sn = m->GetStrandName(s);
        if (sn == "") {
            sn = wxString::Format("Strand %d", s + 1);
        }
        xLightsImportModelNode* laststrand;
        if (channelColors.find(sn.ToStdString()) != channelColors.end())
        {
            laststrand = new xLightsImportModelNode(lastmodel, m->GetName(), sn, std::string(""), true, channelColors.find(sn.ToStdString())->second.asWxColor());
        }
        else
        {
            laststrand = new xLightsImportModelNode(lastmodel, m->GetName(), sn, std::string(""), true, *wxWHITE);
        }
        lastmodel->Append(laststrand);
        for (int n = 0; n < m->GetStrandLength(s); n++)
        {
            wxString nn = m->GetNodeName(n);
            if (nn == "")
            {
                nn = wxString::Format("Node %d", n + 1);
            }
            xLightsImportModelNode* lastnode;
            if (channelColors.find(nn.ToStdString()) != channelColors.end())
            {
                lastnode = new xLightsImportModelNode(laststrand, m->GetName(), sn, nn, std::string(""), true, channelColors.find(nn.ToStdString())->second.asWxColor());
            }
            else
            {
                lastnode = new xLightsImportModelNode(laststrand, m->GetName(), sn, nn, std::string(""), true, *wxWHITE);
            }
            laststrand->Insert(lastnode, n);
        }
    }
    if (dynamic_cast<ModelGroup*>(m) != nullptr) {
        ModelGroup *grp = dynamic_cast<ModelGroup*>(m);
        if (grp != nullptr)
        {
            auto modelNames = grp->ModelNames();

            for (const auto& it : modelNames)
            {
                if (std::find(it.begin(), it.end(), '/') != it.end())
                {
                    // this is a submodel ... dont add it

                    // The risk here is the submodel is in the group but the parent model isnt so there will be no way to
                    // map it. Maybe we should grab the parent model and ensure it is included in the list
                    // Given up until this change this actually crashed my guess is no one is screaming for it now
                }
                else
                {
                    Model* mdl = grp->GetModel(it);
                    if (mdl != nullptr)
                    {
                        AddModel(mdl, ms);
                    }
                }
            }
        }
    }
}

void xLightsImportChannelMapDialog::OnKeyDown(wxKeyEvent& event)
{
    auto key = event.GetKeyCode();
    if (key == WXK_DELETE || key == WXK_NUMPAD_DELETE)
    {
        if (TreeListCtrl_Mapping->GetSelectedItemsCount() > 0)
        {
            Unmap(TreeListCtrl_Mapping->GetSelection());
        }
    }
}

void xLightsImportChannelMapDialog::OnItemActivated(wxDataViewEvent& event)
{
    if (event.GetItem().IsOk())
    {
        wxVariant vvalue;
        event.GetModel()->GetValue(vvalue, event.GetItem(), 1);
        std::string mapped = vvalue.GetString().ToStdString();
        if (mapped == "" && ListCtrl_Available->GetSelectedItemCount() > 0)
        {
            int itemIndex = ListCtrl_Available->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            Map(event.GetItem(), ListCtrl_Available->GetItemText(itemIndex).ToStdString());
        }
        else
        {
            Unmap(event.GetItem());
        }
        TreeListCtrl_Mapping->Refresh();
    }
}

void xLightsImportChannelMapDialog::Map(const wxDataViewItem& item, const wxString& mapping)
{
    _dirty = true;
    TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(mapping), item, 1);
    ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel())->SetMappingExists(item, true);
    MarkUsed();
}

void xLightsImportChannelMapDialog::Unmap(const wxDataViewItem& item)
{
    _dirty = true;
    TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(""), item, 1);
    ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel())->SetMappingExists(item, true);
    MarkUsed();
}

void xLightsImportChannelMapDialog::OnSelectionChanged(wxDataViewEvent& event)
{
}

void xLightsImportChannelMapDialog::OnValueChanged(wxDataViewEvent& event)
{
    _dirty = true;
    if (_allowColorChoice)
    {
        if (event.GetItem().IsOk())
        {
            if (event.GetColumn() == 1)
            {
                wxVariant vvalue;
                event.GetModel()->GetValue(vvalue, event.GetItem(), 1);
                std::string value = vvalue.GetString().ToStdString();
                if (channelColors.find(value) != channelColors.end())
                {
                    event.GetModel()->SetValue(wxVariant(channelColors.find(value)->second.asWxColor().GetAsString()), event.GetItem(), 2);
                }
            }
        }
    }
}

wxString xLightsImportChannelMapDialog::FindTab(wxString &line) {
    for (size_t x = 0; x < line.size(); x++) {
        if (line[x] == '\t') {
            wxString first = line.SubString(0, x - 1);
            line = line.SubString(x+1, line.size());
            return first;
        }
    }
    return line;
}

wxDataViewItem xLightsImportChannelMapDialog::FindItem(std::string model, std::string strand, std::string node)
{
    wxDataViewItemArray models;
    _dataModel->GetChildren(wxDataViewItem(0), models);
    for (size_t i = 0; i < models.size(); i++)
    {
        xLightsImportModelNode* amodel = (xLightsImportModelNode*)models[i].GetID();
        if (amodel->_model == model)
        {
            if (strand == "")
            {
                return models[i];
            }
            else
            {
                wxDataViewItemArray strands;
                _dataModel->GetChildren(models[i], strands);
                for (size_t j = 0; j < strands.size(); j++)
                {
                    xLightsImportModelNode* astrand = (xLightsImportModelNode*)strands[j].GetID();
                    if (astrand->_strand == strand)
                    {
                        if (node == "")
                        {
                            return strands[j];
                        }
                        else
                        {
                            wxDataViewItemArray nodes;
                            _dataModel->GetChildren(strands[j], nodes);
                            for (size_t k = 0; k < nodes.size(); k++)
                            {
                                xLightsImportModelNode* anode = (xLightsImportModelNode*)nodes[k].GetID();
                                if (anode->_node == node)
                                {
                                    return nodes[k];
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return wxDataViewItem(nullptr);
}

xLightsImportModelNode* xLightsImportChannelMapDialog::TreeContainsModel(std::string model, std::string strand, std::string node)
{
    for (size_t i = 0; i < _dataModel->GetChildCount(); i++)
    {
        xLightsImportModelNode* m = _dataModel->GetNthChild(i);
        if (m->_model.ToStdString() == model)
        {
            if (strand == "")
            {
                return m;
            }
            else
            {
                for (size_t j = 0; j < m->GetChildCount(); j++)
                {
                    xLightsImportModelNode* s = m->GetNthChild(j);
                    if (s->_strand.ToStdString() == strand)
                    {
                        if (node == "")
                        {
                            return s;
                        }
                        else
                        {
                            for (size_t k = 0; k < s->GetChildCount(); k++)
                            {
                                xLightsImportModelNode* n = s->GetNthChild(k);
                                if (n->_node.ToStdString() == node)
                                {
                                    return n;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}

void xLightsImportChannelMapDialog::LoadMapping(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool strandwarning = false;
    bool modelwarning = false;
    if (_dirty)
    {
        if (wxMessageBox("Are you sure you dont want to save your changes for future imports?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO)
        {
            return;
        }
    }

    wxFileDialog dlg(this, "Load mapping", wxEmptyString, wxEmptyString, "Mapping Files (*.xmap)|*.xmap|All Files (*.)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        _mappingFile = dlg.GetPath();
        _dataModel->ClearMapping();

        for (auto it = _stashedMappings.begin(); it != _stashedMappings.end(); ++it)
        {
            delete (*it);
        }
        _stashedMappings.clear();

        wxFileInputStream input(dlg.GetPath());
        wxTextInputStream text(input, "\t");
        text.ReadLine(); // map by strand ... ignore this
        int count = wxAtoi(text.ReadLine());
        for (int x = 0; x < count; x++) {
            std::string mn = text.ReadLine().ToStdString();
            if (TreeContainsModel(mn) == nullptr) {
                if (!modelwarning)
                {
                    if (wxMessageBox("Model " + mn + " not part of sequence.  Not mapping channels to this model. Do you want to see future occurences of this error during this import?", "", wxICON_WARNING | wxYES_NO, this) == wxNO)
                    {
                        modelwarning = true;
                    }
                }
            }
        }
        wxString line = text.ReadLine();
        while (line != "") {
            wxString model;
            wxString strand;
            wxString node;
            wxString mapping;
            wxColor color = *wxWHITE;

            if (CountChar(line, '\t') == 4)
            {
                model = FindTab(line);
                strand = FindTab(line);
                node = FindTab(line);
                mapping = FindTab(line);
                color = wxColor(FindTab(line));
            }
            else
            {
                model = FindTab(line);
                strand = FindTab(line);
                node = FindTab(line);
                mapping = FindTab(line);
            }
            Element *modelEl = mSequenceElements->GetElement(model.ToStdString());

            // This code adds the model into the sequence ... it is useful if the model was previously only 
            // in a model group that was in the sequence.
            if (modelEl == nullptr && xlights->GetModel(model.ToStdString()) != nullptr) {
                mSequenceElements->AddMissingModelsToSequence(model.ToStdString(), false);
                ModelElement *mel = dynamic_cast<ModelElement*>(mSequenceElements->GetElement(model.ToStdString()));
                if (mel != nullptr) {
                    mel->Init(*xlights->GetModel(model.ToStdString()));
                }
                else {
                    logger_base.warn("Strange ... load mapping returned null model for " + model);
                }
                modelEl = mel;
            }

            if (modelEl != nullptr) {
                xLightsImportModelNode* mi = TreeContainsModel(model.ToStdString());
                xLightsImportModelNode* msi = TreeContainsModel(model.ToStdString(), strand.ToStdString());
                xLightsImportModelNode* mni = TreeContainsModel(model.ToStdString(), strand.ToStdString(), node.ToStdString());

                if (mni == msi) mni = nullptr;
                if (msi == mi) msi = nullptr;

                if (mi == nullptr || (msi == nullptr && strand != "") || (mni == nullptr && node != ""))
                {
                    if (mi != nullptr && !strandwarning)
                    {
                        if (wxMessageBox(model + "/" + strand + "/" + node + " not found.  Has the models changed? Do you want to see future occurences of this error during this import?", "", wxICON_WARNING | wxYES_NO, this) == wxNO)
                        {
                            strandwarning = true;
                        }
                    }

                    // save the unused mappings
                    _stashedMappings.push_back(new StashedMapping(model, strand, node, mapping, color));
                }
                else
                {
                    if (mapping != "")
                    {
                        if (mni != nullptr)
                        {
                            wxDataViewItem item = FindItem(model.ToStdString(), strand.ToStdString(), node.ToStdString());
                            TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(mapping), item, 1);
                            ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel())->SetMappingExists(item, ListCtrl_Available->FindItem(0, mapping) >= 0);
                            TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(color.GetAsString()), item, 2);
                        }
                        else if (msi != nullptr)
                        {
                            wxDataViewItem item = FindItem(model.ToStdString(), strand.ToStdString());
                            TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(mapping), item, 1);
                            ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel())->SetMappingExists(item, ListCtrl_Available->FindItem(0, mapping) >= 0);
                            TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(color.GetAsString()), item, 2);
                        }
                        else
                        {
                            wxDataViewItem item = FindItem(model.ToStdString());
                            TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(mapping), item, 1);
                            ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel())->SetMappingExists(item, ListCtrl_Available->FindItem(0, mapping) >= 0);
                            TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(color.GetAsString()), item, 2);
                        }
                    }
                }
            }
            else
            {
                // save this unused mapping
                _stashedMappings.push_back(new StashedMapping(model, strand, node, mapping, color));
            }
            line = text.ReadLine();
        }
        _dirty = false;

        logger_base.debug("Mapping %s loaded. %d mappings stashed.", (const char*)dlg.GetPath().c_str(), _stashedMappings.size());

        // expand all models that have strands that have a value
        wxDataViewItemArray models;
        _dataModel->GetChildren(wxDataViewItem(0), models);
        for (size_t i = 0; i < models.size(); i++)
        {
            wxDataViewItemArray strands;
            _dataModel->GetChildren(models[i], strands);
            for (size_t j = 0; j < strands.size(); j++)
            {
                xLightsImportModelNode* astrand = (xLightsImportModelNode*)strands[j].GetID();
                if (astrand->HasMapping())
                {
                    TreeListCtrl_Mapping->Expand(models[i]);
                }
                wxDataViewItemArray nodes;
                _dataModel->GetChildren(strands[j], nodes);
                for (size_t k = 0; k < nodes.size(); k++)
                {
                    xLightsImportModelNode* anode = (xLightsImportModelNode*)nodes[k].GetID();
                    if (anode->_mapping != "")
                    {
                        TreeListCtrl_Mapping->Expand(strands[j]);
                    }
                }
            }
        }
    }
    MarkUsed();
    TreeListCtrl_Mapping->Refresh();
}

void xLightsImportChannelMapDialog::SaveMapping(wxCommandEvent& event)
{
    wxFileDialog dlg(this, "Save mapping", wxEmptyString, _mappingFile, "Mapping Files (*.xmap)|*.xmap|All Files (*.)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        wxFileOutputStream output(dlg.GetPath());
        wxTextOutputStream text(output);
        text.WriteString("false\n");
        int modelcount = _dataModel->GetMappedChildCount();
        text.WriteString(wxString::Format("%d\n", modelcount));
        for (size_t i = 0; i < _dataModel->GetChildCount(); i++)
        {
            xLightsImportModelNode* m = _dataModel->GetNthChild(i);
            if (m->HasMapping())
            {
                text.WriteString(m->_model + "\n");
            }
        }
        for (size_t i = 0; i < _dataModel->GetChildCount(); i++)
        {
            xLightsImportModelNode* m = _dataModel->GetNthChild(i);
            if (m->HasMapping())
            {
                wxString mn = m->_model;
                StashedMapping* sm = GetStashedMapping(mn, "", "");
                if (m->_mapping == "" && sm != nullptr)
                {
                    text.WriteString(mn
                        + "\t" +
                        +"\t" +
                        +"\t" + sm->_mapping
                        + "\t" + sm->_color.GetAsString()
                        + "\n");
                }
                else
                {
                    text.WriteString(mn
                        + "\t" +
                        +"\t" +
                        +"\t" + m->_mapping
                        + "\t" + m->_color.GetAsString()
                        + "\n");
                }
                for (size_t j = 0; j < m->GetChildCount(); j++)
                {
                    xLightsImportModelNode* s = m->GetNthChild(j);
                    if (s->HasMapping() || AnyStashedMappingExists(mn, s->_strand))
                    {
                        sm = GetStashedMapping(mn, s->_strand, "");
                        if (s->_mapping == "" && sm != nullptr)
                        {
                            text.WriteString(mn
                                + "\t" + sm->_strand
                                + "\t" +
                                +"\t" + sm->_mapping
                                + "\t" + sm->_color.GetAsString()
                                + "\n");
                        }
                        else
                        {
                            text.WriteString(mn
                                + "\t" + s->_strand
                                + "\t" +
                                +"\t" + s->_mapping
                                + "\t" + s->_color.GetAsString()
                                + "\n");
                        }
                        for (size_t k = 0; k < s->GetChildCount(); k++)
                        {
                            xLightsImportModelNode* n = s->GetNthChild(k);
                            sm = GetStashedMapping(mn, n->_strand, n->_node);
                            if (n->_mapping == "" && sm != nullptr)
                            {
                                text.WriteString(mn
                                    + "\t" + sm->_strand
                                    + "\t" + sm->_node
                                    + "\t" + sm->_mapping
                                    + "\t" + sm->_color.GetAsString()
                                    + "\n");
                            }
                            else
                            {
                                if (n->HasMapping())
                                {
                                    text.WriteString(mn
                                        + "\t" + n->_strand
                                        + "\t" + n->_node
                                        + "\t" + n->_mapping
                                        + "\t" + n->_color.GetAsString()
                                        + "\n");
                                }
                            }
                        }
                    }
                }
            }
        }

        for (auto it = _stashedMappings.begin(); it != _stashedMappings.end(); ++it)
        {
            Element *modelEl = mSequenceElements->GetElement((*it)->_model.ToStdString());
            if (modelEl == nullptr)
            {
                text.WriteString((*it)->_model
                    + "\t" + (*it)->_strand
                    + "\t" + (*it)->_node
                    + "\t" + (*it)->_mapping
                    + "\t" + (*it)->_color.GetAsString()
                    + "\n");
            }
        }
        _dirty = false;
    }
}

void xLightsImportChannelMapDialog::OnResize(wxSizeEvent& event)
{
    /*
    wxSize s = GetSize();
    s.SetWidth(s.GetWidth()-15);
    s.SetHeight(s.GetHeight()-75);

    wxSize s2 = FlexGridSizer2->GetSize();

    s.SetHeight(s.GetHeight()-s2.GetHeight());

    TreeListCtrl_Mapping->SetSize(s);
    TreeListCtrl_Mapping->SetMinSize(s);
    TreeListCtrl_Mapping->SetMaxSize(s);

    TreeListCtrl_Mapping->FitInside();
    TreeListCtrl_Mapping->Refresh();
    Layout();
     */
}

wxDataViewItem xLightsImportChannelMapDialog::GetNextTreeItem(const wxDataViewItem item) const
{
    int bottom = TreeListCtrl_Mapping->GetItemRect(item).GetBottom();

    wxDataViewItemArray models;
    _dataModel->GetChildren(wxDataViewItem(0), models);
    for (size_t i = 0; i < models.size(); i++)
    {
        int mtop = TreeListCtrl_Mapping->GetItemRect(models[i]).GetTop();
        if (mtop == bottom + 1)
        {
            return models[i];
        }

        if (TreeListCtrl_Mapping->IsExpanded(models[i]))
        {
            wxDataViewItemArray strands;
            _dataModel->GetChildren(models[i], strands);
            for (size_t j = 0; j < strands.size(); j++)
            {
                int stop = TreeListCtrl_Mapping->GetItemRect(strands[j]).GetTop();
                if (stop == bottom + 1)
                {
                    return strands[j];
                }

                if (TreeListCtrl_Mapping->IsExpanded(strands[j]))
                {
                    wxDataViewItemArray nodes;
                    _dataModel->GetChildren(strands[j], nodes);
                    for (size_t k = 0; k < nodes.size(); k++)
                    {
                        int ntop = TreeListCtrl_Mapping->GetItemRect(nodes[k]).GetTop();
                        if (ntop == bottom + 1)
                        {
                            return nodes[k];
                        }
                    }
                }
            }
        }
    }
    return wxDataViewItem(nullptr);
}

wxDataViewItem xLightsImportChannelMapDialog::GetPriorTreeItem(const wxDataViewItem item) const
{
    int top = TreeListCtrl_Mapping->GetItemRect(item).GetTop();
    if (top == 0) top = -2;

    wxDataViewItemArray models;
    _dataModel->GetChildren(wxDataViewItem(0), models);
    for (size_t i = 0; i < models.size(); i++)
    {
        int mbottom = TreeListCtrl_Mapping->GetItemRect(models[i]).GetBottom();
        if (top == mbottom - 1)
        {
            return models[i];
        }
        if (TreeListCtrl_Mapping->IsExpanded(models[i]))
        {
            wxDataViewItemArray strands;
            _dataModel->GetChildren(models[i], strands);
            for (size_t j = 0; j < strands.size(); j++)
            {
                int sbottom = TreeListCtrl_Mapping->GetItemRect(strands[j]).GetBottom();
                if (top == sbottom - 1)
                {
                    return strands[j];
                }

                if (TreeListCtrl_Mapping->IsExpanded(strands[j]))
                {
                    wxDataViewItemArray nodes;
                    _dataModel->GetChildren(strands[j], nodes);
                    for (size_t k = 0; k < nodes.size(); k++)
                    {
                        int nbottom = TreeListCtrl_Mapping->GetItemRect(nodes[k]).GetBottom();
                        if (top == nbottom - 1)
                        {
                            return nodes[k];
                        }
                    }
                }
            }
        }
    }

    return wxDataViewItem(nullptr);
}

void xLightsImportChannelMapDialog::OnButton_OkClick(wxCommandEvent& event)
{
    if (_dirty)
    {
        if (wxMessageBox("Are you sure you want to exit WITHOUT saving your mapping changes for future imports?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxYES)
        {
            EndDialog(wxID_OK);
        }

    }
    else
    {
            EndDialog(wxID_OK);
    }
}

void xLightsImportChannelMapDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    if (_dirty)
    {
        if (wxMessageBox("Are you sure you dont want to save your changes for future imports?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxYES)
        {
            EndDialog(wxID_CANCEL);
        }

    }
    else
    {
            EndDialog(wxID_CANCEL);
    }
}

void xLightsImportChannelMapDialog::OnListCtrl_AvailableItemSelect(wxListEvent& event)
{
}

void xLightsImportChannelMapDialog::OnListCtrl_AvailableColumnClick(wxListEvent& event)
{
    if (_sortOrder == 0)
    {
        _sortOrder = 1;
        ListCtrl_Available->SortItems(MyCompareFunctionAsc, (wxIntPtr) ListCtrl_Available);
    }
    else
    {
        _sortOrder = 0;
        ListCtrl_Available->SortItems(MyCompareFunctionDesc, (wxIntPtr)ListCtrl_Available);
    }
}

#pragma region Drag and Drop

void xLightsImportChannelMapDialog::OnListCtrl_AvailableBeginDrag(wxListEvent& event)
{
    wxLogNull logNo; //kludge: Prevent wx logging

    if (_dataModel->GetChildCount() == 0) return;

    _dragItem = wxDataViewItem(nullptr);
    if (ListCtrl_Available->GetSelectedItemCount() == 0) return;

    wxString drag = "Map";
    int itemIndex = ListCtrl_Available->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    drag += "," + ListCtrl_Available->GetItemText(itemIndex, 0);

    wxTextDataObject my_data(drag);
    MDDropSource dragSource(this);
    dragSource.SetData(my_data);
    dragSource.DoDragDrop(wxDrag_DefaultMove);
    SetCursor(wxCURSOR_ARROW);
}
void xLightsImportChannelMapDialog::OnDragPossible(wxDataViewEvent& event) {
    if (event.GetItem().IsOk()) {
#ifdef __WXOSX__
        if (event.GetProposedDropIndex() != -1) return;
#endif
        event.Allow();
        event.SetDropEffect(wxDragCopy);
    }
}
void xLightsImportChannelMapDialog::OnDragDrop(wxDataViewEvent& event) {
    if (event.GetItem().IsOk()) {
        wxDataViewItem  item = event.GetItem();
        wxDataObjectComposite *comp = (wxDataObjectComposite*)event.GetDataObject();
        wxTextDataObject *obj = (wxTextDataObject*)comp->GetObject(wxDF_TEXT);
        wxArrayString parms = wxSplit(obj->GetText(), ',');
        Map(item, parms[1]);
    }
}

void xLightsImportChannelMapDialog::OnBeginDrag(wxDataViewEvent& event)
{
    wxLogNull logNo; //kludge: Prevent wx logging

    if (event.GetItem().IsOk())
    {
        _dragItem = event.GetItem();
        wxString drag = "Map";
        wxVariant vvalue;
        event.GetModel()->GetValue(vvalue, event.GetItem(), 1);
        std::string mapped = vvalue.GetString().ToStdString();

        if (mapped != "")
        {
            drag += "," + mapped;

            wxTextDataObject my_data(drag);
            MDDropSource dragSource(this);
            dragSource.SetData(my_data);
            dragSource.DoDragDrop(wxDrag_DefaultMove);
        }
        SetCursor(wxCURSOR_ARROW);
    }
}

wxDragResult MDTextDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
    wxLogNull logNo; //kludge: Prevent wx logging

    if (_list != nullptr) return wxDragMove;
    if (((xLightsImportTreeModel*)_tree->GetModel())->GetChildCount() == 0) return wxDragMove;

    //wxRect rect = _tree->GetItemRect(((xLightsImportTreeModel*)_tree->GetModel())->GetNthItem(0));
    //y += rect.GetHeight();

    static int MINSCROLLDELAY = 10;
    static int STARTSCROLLDELAY = 300;
    static int scrollDelay = STARTSCROLLDELAY;
    static wxLongLong lastTime = wxGetUTCTimeMillis();

    if (wxGetUTCTimeMillis() - lastTime < scrollDelay)
    {
        // too soon to scroll again
    }
    else
    {
        wxDataViewItem item;
        wxDataViewColumn* col;
        _tree->HitTest(wxPoint(x, y), item, col);

        if (item.IsOk())
        {
            // we are above an item
            _tree->SetCurrentItem(item);

            // get the first visible item
            wxRect itemRect = _tree->GetItemRect(item);

            if (y < itemRect.GetHeight())
            {
                // scroll up
                lastTime = wxGetUTCTimeMillis();
                wxDataViewItem prev = ((xLightsImportChannelMapDialog*)_owner)->GetPriorTreeItem(item);

                if (prev.IsOk())
                {
                    lastTime = wxGetUTCTimeMillis();
                    _tree->EnsureVisible(prev);
                    scrollDelay = scrollDelay / 2;
                    if (scrollDelay < MINSCROLLDELAY) scrollDelay = MINSCROLLDELAY;
                }
            }
            else if (y > _tree->GetRect().GetTop() - itemRect.GetHeight())
            {
                // scroll down
                wxDataViewItem next = ((xLightsImportChannelMapDialog*)_owner)->GetNextTreeItem(item);
                if (next.IsOk())
                {
                    lastTime = wxGetUTCTimeMillis();
                    _tree->EnsureVisible(next);
                    scrollDelay = scrollDelay / 2;
                    if (scrollDelay < MINSCROLLDELAY) scrollDelay = MINSCROLLDELAY;
                }
            }
            else
            {
                // dont scroll
                scrollDelay = STARTSCROLLDELAY;
            }
        }
    }
    return wxDragMove;
}

bool MDTextDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
    wxLogNull logNo; //kludge: Prevent wx logging

    long mousePos = x;
    mousePos = mousePos << 16;
    mousePos += y;
    wxCommandEvent event(EVT_MDDROP);
    event.SetString(data); // this is the dropped string
    event.SetExtraLong(mousePos); // this is the mouse position packed into a long

    if (_list != nullptr)
    {
        event.SetInt(0);
    }
    else
    {
        event.SetInt(1);
    }
    wxPostEvent(_owner, event);
    return true;
}

void xLightsImportChannelMapDialog::OnDrop(wxCommandEvent& event)
{
    wxLogNull logNo; //kludge: Prevent wx logging

    wxArrayString parms = wxSplit(event.GetString(), ',');
    int x = event.GetExtraLong() >> 16;
    int y = event.GetExtraLong() & 0xFFFF;

    switch (event.GetInt())
    {
    case 0:
        // drop on list ... so a remove
    {
        // if source was the tree
        if (_dragItem.IsOk())
        {
            Unmap(_dragItem);
        }
    }
    break;
    case 1:
        // drop on tree ... do a map
    {
        // and map the destination
        wxDataViewItem item;
        wxDataViewColumn* col;
        TreeListCtrl_Mapping->HitTest(wxPoint(x, y), item, col);

        if (item.IsOk() && parms.size() == 2)
        {
            // if source was the tree then unmap the source
            if (_dragItem.IsOk())
            {
                Unmap(_dragItem);
            }

            Map(item, parms[1].ToStdString());
        }
    }
    break;
    default:
        break;
    }

    TreeListCtrl_Mapping->Refresh();

    MarkUsed();
}

#pragma endregion Drag and Drop

void xLightsImportChannelMapDialog::OnCheckBox_MapCCRStrandClick(wxCommandEvent& event)
{
    if (CheckBox_MapCCRStrand->IsChecked())
    {
        SetCCROn();
    }
    else
    {
        SetCCROff();
    }
}

void xLightsImportChannelMapDialog::SetCCROn()
{
    PopulateAvailable(true);
}

void xLightsImportChannelMapDialog::SetCCROff()
{
    PopulateAvailable(false);
}

void xLightsImportChannelMapDialog::MarkUsed()
{
    std::list<std::string> used;

    // go through each tree row where mapping is not blank
    for (unsigned int i = 0; i < _dataModel->GetChildCount(); ++i)
    {
        bool found = false;
        auto model = _dataModel->GetNthChild(i);
        if (model->_mapping != "")
        {
            if (std::find(used.begin(), used.end(), model->_mapping) == used.end())
            {
                used.push_back(model->_mapping.ToStdString());
                found = true;
            }
        }

        for (unsigned int j = 0; j < model->GetChildCount(); j++)
        {
            auto strand = model->GetNthChild(j);
            if (strand->_mapping != "")
            {
                if (std::find(used.begin(), used.end(), strand->_mapping) == used.end())
                {
                    used.push_back(strand->_mapping.ToStdString());
                    found = true;
                }
            }

            for (unsigned int k = 0; k < strand->GetChildCount(); k++)
            {
                auto node = strand->GetNthChild(k);
                if (node->_mapping != "")
                {
                    if (std::find(used.begin(), used.end(), node->_mapping) == used.end())
                    {
                        used.push_back(node->_mapping.ToStdString());
                        found = true;
                    }
                }
            }
        }
    }

    used.sort();

    int items = ListCtrl_Available->GetItemCount();
    ListCtrl_Available->Freeze();
    for (int i = 0; i < items; ++i)
    {
        if (!std::binary_search(used.begin(), used.end(), ListCtrl_Available->GetItemText(i).ToStdString()))
        {
            // not used
            ListCtrl_Available->SetItemTextColour(i, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
        }
        else
        {
            //used
            ListCtrl_Available->SetItemTextColour(i, *wxLIGHT_GREY);
        }
    }
    ListCtrl_Available->Thaw();
    ListCtrl_Available->Refresh();
}

StashedMapping* xLightsImportChannelMapDialog::GetStashedMapping(wxString modelName, wxString strandName, wxString nodeName)
{
    for (auto it = _stashedMappings.begin(); it != _stashedMappings.end(); ++it)
    {
        if ((*it)->_model == modelName && (*it)->_strand == strandName && (*it)->_node == nodeName)
        {
            return *it;
        }
    }

    return nullptr;
}

bool xLightsImportChannelMapDialog::AnyStashedMappingExists(wxString modelName, wxString strandName)
{
    for (auto it = _stashedMappings.begin(); it != _stashedMappings.end(); ++it)
    {
        if ((*it)->_model == modelName && (strandName == "" || (*it)->_strand == strandName))
        {
            if ((*it)->_mapping != "")
            {
                return true;
            }
        }
    }

    return false;
}

void xLightsImportChannelMapDialog::OnButton_AutoMapClick(wxCommandEvent& event)
{
    if (_dataModel == nullptr) return;

    for (unsigned int i = 0; i < _dataModel->GetChildCount(); ++i)
    {
        auto model = _dataModel->GetNthChild(i);
        if (model != nullptr)
        {
            if (model->_mapping == "")
            {
                for (int j = 0; j < ListCtrl_Available->GetItemCount(); ++j)
                {
                    wxString availName = ListCtrl_Available->GetItemText(j).Trim(true).Trim(false).Lower();
                    if (availName.Contains("/"))
                    {
                        wxArrayString parts = wxSplit(availName, '/');
                        if (wxString(model->_model).Trim(true).Trim(false).Lower() == parts[0])
                        {
                            // matched the model name ... need to look at strands and submodels
                            for (unsigned int k = 0; k < model->GetChildCount(); ++k)
                            {
                                auto strand = model->GetNthChild(k);
                                if (strand != nullptr)
                                {
                                    if (strand->_mapping == "")
                                    {
                                        if (wxString(strand->_strand).Trim(true).Trim(false).Lower() == parts[1])
                                        {
                                            // matched to the strand level
                                            if (parts.size() == 2)
                                            {
                                                strand->_mapping = ListCtrl_Available->GetItemText(j);
                                                strand->_mappingExists = true;
                                            }
                                            else
                                            {
                                                // need to map the node level
                                                for (unsigned int m = 0; m < strand->GetChildCount(); ++m)
                                                {
                                                    auto node = strand->GetNthChild(m);
                                                    if (node != nullptr)
                                                    {
                                                        if (node->_mapping == "")
                                                        {
                                                            if (wxString(node->_node).Trim(true).Trim(false).Lower() == parts[2])
                                                            {
                                                                // matched to the strand level
                                                                if (parts.size() == 3)
                                                                {
                                                                    node->_mapping = ListCtrl_Available->GetItemText(j);
                                                                    node->_mappingExists = true;
                                                                }
                                                                else
                                                                {
                                                                    wxASSERT(false);
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (wxString(model->_model).Trim(true).Trim(false).Lower() == availName)
                        {
                            model->_mapping = ListCtrl_Available->GetItemText(j);
                            model->_mappingExists = true;
                        }
                    }
                }
            }
        }
        else
        {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.warn("xLightsImportTreeModel::OnButton_AutoMapClick: Weird ... model %d was nullptr", i);
        }
    }
    TreeListCtrl_Mapping->Refresh();
    MarkUsed();
}

void xLightsImportChannelMapDialog::OnListCtrl_AvailableItemActivated(wxListEvent& event)
{
    if (TreeListCtrl_Mapping->GetSelectedItemsCount() != 1) return;

    wxDataViewItem mapTo = TreeListCtrl_Mapping->GetSelection();
    Map(mapTo, ListCtrl_Available->GetItemText(event.GetItem()).ToStdString());

    wxDataViewItem nextMapTo = GetNextTreeItem(mapTo);
    TreeListCtrl_Mapping->Unselect(mapTo);
    if (nextMapTo.IsOk())
    {
        TreeListCtrl_Mapping->Select(nextMapTo);
        TreeListCtrl_Mapping->EnsureVisible(nextMapTo);
    }

    if (event.GetIndex() + 1 < ListCtrl_Available->GetItemCount())
    {
        ListCtrl_Available->SetItemState(event.GetIndex(), 0, wxLIST_STATE_SELECTED);
        ListCtrl_Available->SetItemState(event.GetIndex() + 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        ListCtrl_Available->EnsureVisible(event.GetIndex() + 1);
    }
}
