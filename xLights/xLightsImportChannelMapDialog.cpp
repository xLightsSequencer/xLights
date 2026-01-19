/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

 //(*InternalHeaders(xLightsImportChannelMapDialog)
 #include <wx/intl.h>
 #include <wx/string.h>
 //*)

#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>
#include <wx/colordlg.h>
#include <wx/regex.h>
#include <wx/stdpaths.h>

#include "xLightsImportChannelMapDialog.h"
#include "sequencer/SequenceElements.h"
#include "xLightsMain.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/CustomModel.h"
#include "XmlSerializer/XmlSerializer.h"
#include "ColorManager.h"
#include "UtilFunctions.h"
#include "ExternalHooks.h"
#include "MediaImportOptionsDialog.h"
#include "LayoutUtils.h"
#include "xlColourData.h"
#include "utils/string_utils.h"
#include "ai/aiBase.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <iostream>
#include <nlohmann/json.hpp>

#include <log4cpp/Category.hh>

wxDEFINE_EVENT(EVT_MDDROP, wxCommandEvent);

int wxCALLBACK MyCompareFunctionAscEffects(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData)
{
    ImportChannel *i1 = (ImportChannel*)item1;
    ImportChannel *i2 = (ImportChannel*)item2;
    return i1->effectCount - i2->effectCount;
}

int wxCALLBACK MyCompareFunctionDescEffects(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData)
{
    ImportChannel *i1 = (ImportChannel*)item1;
    ImportChannel *i2 = (ImportChannel*)item2;
    return i2->effectCount - i1->effectCount;
}

int wxCALLBACK MyCompareFunctionAscName(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData)
{
    if (sortData) {
        //map by CCR is on
        return item1 - item2;
    }
    ImportChannel *i1 = (ImportChannel*)item1;
    ImportChannel *i2 = (ImportChannel*)item2;
    return NumberAwareStringCompare(i1->name, i2->name);
}

int wxCALLBACK MyCompareFunctionDescName(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData)
{
    if (sortData) {
        //map by CCR is on
        return item2 - item1;
    }
    ImportChannel *i1 = (ImportChannel*)item1;
    ImportChannel *i2 = (ImportChannel*)item2;
    return NumberAwareStringCompareRev(i1->name, i2->name);
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
            _window->ListCtrl_Available->GetRect().Contains(point)) {
            _window->SetCursor(wxCursor(wxCURSOR_HAND));
        } else {
            _window->SetCursor(wxCursor(wxCURSOR_NO_ENTRY));
        }

        return true;
    }
};

class ColorRenderer : public wxDataViewCustomRenderer
{
    wxColor _color;

public:
    ColorRenderer() : wxDataViewCustomRenderer(GetDefaultType(), wxDATAVIEW_CELL_ACTIVATABLE)
    {
        _color = *wxWHITE;
    }

    virtual bool ActivateCell(const wxRect &cell, wxDataViewModel *model, const wxDataViewItem &item, unsigned int col, const wxMouseEvent *mouseEvent) override
    {
        auto const& [res, newcolor] = xlColourData::INSTANCE.ShowColorDialog(GetOwner()->GetOwner()->GetParent(), _color);
        if (res == wxID_OK) {
            _color = newcolor;
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
        if (value.GetString() != "") {
            _color = wxColor(value.GetString());
            return true;
        }
        return false;
    }
};

//wxColourData ColorRenderer::_colorData;

xLightsImportTreeModel::xLightsImportTreeModel()
{
}

bool xLightsImportTreeModel::GetAttr(const wxDataViewItem &item, unsigned int col, wxDataViewItemAttr &attr) const
{
    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    if (!node) {     // happens if item.IsOk()==false
        return false;
    }

    bool set = false;

    if (!node->_mappingExists) {
        attr.SetBackgroundColour(*wxRED);
        set = true;
    }

    if (node->IsGroup()) {
        attr.SetColour(ColorManager::instance()->CyanOrBlueOverride());
        set = true;
    }

    return set;
}

inline int findChildIndex(xLightsImportModelNode *parent, const std::string &strandName) {
    int idx = 0;
    for (auto &i : parent->GetChildren()) {
        if (i->_strand == strandName) {
            return idx;
        }
        idx++;
    }
    return -1;
}

int xLightsImportTreeModel::Compare(const wxDataViewItem& item1, const wxDataViewItem& item2, unsigned column, bool ascending) const
{
    if (column == 0) {
        xLightsImportModelNode *node1 = (xLightsImportModelNode*)item1.GetID();
        xLightsImportModelNode *node2 = (xLightsImportModelNode*)item2.GetID();

        if (node1->_node != "" && node2->_node != "") {
            if (ascending) {
                return NumberAwareStringCompare(node1->_node, node2->_node);
            } else {
                return NumberAwareStringCompareRev(node1->_node, node2->_node);
            }
        } else if (node1->_strand != "" && node2->_strand != "") { // dont sort the submodels
            int idx1 = findChildIndex(node1->GetParent(), node1->_strand);
            int idx2 = findChildIndex(node2->GetParent(), node2->_strand);
            return idx1 - idx2;
        } else {
            if (ascending) {
                return NumberAwareStringCompare(GetModel(item1).ToStdString(), GetModel(item2).ToStdString());
            } else {
                return NumberAwareStringCompareRev(GetModel(item1).ToStdString(), GetModel(item2).ToStdString());
            }
        }
    }

    return 0;
}

wxString xLightsImportTreeModel::GetModel(const wxDataViewItem &item) const
{
    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    if (!node) {     // happens if item.IsOk()==false
        return wxEmptyString;
    }

    return node->_model;
}

wxString xLightsImportTreeModel::GetStrand(const wxDataViewItem &item) const
{
    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    if (!node) {     // happens if item.IsOk()==false
        return wxEmptyString;
    }

    return node->_strand;
}

wxString xLightsImportTreeModel::GetNode(const wxDataViewItem &item) const
{
    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    if (!node) {     // happens if item.IsOk()==false
        return wxEmptyString;
    }

    return node->_node;
}

wxString xLightsImportTreeModel::GetMapping(const wxDataViewItem &item) const
{
    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    if (!node) {      // happens if item.IsOk()==false
        return wxEmptyString;
    }

    return node->_mapping;
}

wxColor xLightsImportTreeModel::GetColor(const wxDataViewItem &item) const
{
    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    if (!node) {     // happens if item.IsOk()==false
        return *wxWHITE;
    }

    return node->_color;
}

void xLightsImportTreeModel::Delete(const wxDataViewItem &item)
{
    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    if (!node) {     // happens if item.IsOk()==false
        return;
    }

    wxDataViewItem parent(node->GetParent());
    // first remove the node from the parent's array of children;
    // NOTE: xLightsImportModelNodePtrArray is only an array of _pointers_
    //       thus removing the node from it doesn't result in freeing it
    if (node->GetParent() != nullptr) {
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
            if (parent == nullptr) {
                //variant = wxVariant(wxDataViewIconText(node->_model));
                variant = wxVariant(node->_model);
            } else if (node->GetChildCount() != 0 || "" == node->_node) {
                //variant = wxVariant(wxDataViewIconText(node->_strand));
                variant = wxVariant(node->_strand);
            } else {
                variant = wxVariant(node->_node);
            }
            break;
        case 1:
            variant = wxVariant(node->_effectCount > 0 ? wxString::Format("%d", node->_effectCount) : "");
            break;
        case 2:
            variant = wxVariant(node->_mapping);
            break;
        case 3:
            variant = wxVariant(node->_mappingModelType);
            break;
        case 4:
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
    if (col == 2) {
        node->_mapping = variant.GetString();
        node->_mappingExists = false;
        return true;
    } else if (col == 3) {
        node->_mappingModelType = variant.GetString();
        return true;
    } else if (col == 4) {
        node->_color = wxColour(variant.GetString());
        return true;
    }
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.warn("xLightsImportTreeModel::SetValue: wrong column %d", col);
    wxLogError("xLightsImportTreeModel::SetValue: wrong column %d", col);
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
    if (!item.IsOk()) {
        return wxDataViewItem(0);
    }

    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();

    if (node->GetParent() == nullptr) {
        return wxDataViewItem(0);
    }

    return wxDataViewItem((void*)node->GetParent());
}

bool xLightsImportTreeModel::IsContainer(const wxDataViewItem &item) const
{
    // the invisble root node can have children
    if (!item.IsOk()) {
        return true;
    }

    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    return node->IsContainer();
}

unsigned int xLightsImportTreeModel::GetChildren(const wxDataViewItem &parent,
    wxDataViewItemArray &array) const
{
    unsigned int count;
    xLightsImportModelNode *node = (xLightsImportModelNode*)parent.GetID();
    if (!node) {
        count = m_children.size();
        for (unsigned int pos = 0; pos < count; ++pos) {
            xLightsImportModelNode *child = m_children.Item(pos);
            array.Add(wxDataViewItem((void*)child));
        }
    } else {
        if (node->GetChildCount() == 0) {
            return 0;
        }

        count = node->GetChildren().GetCount();
        for (unsigned int pos = 0; pos < count; ++pos) {
            xLightsImportModelNode *child = node->GetChildren().Item(pos);
            array.Add(wxDataViewItem((void*)child));
        }
    }
    return count;
}

wxDataViewItem xLightsImportTreeModel::GetNthItem(unsigned int n) const
{
    unsigned int count = m_children.size();
    for (unsigned int pos = 0; pos < count; ++pos) {
        if (pos == n) {
            xLightsImportModelNode *child = m_children.Item(pos);
            return wxDataViewItem((void*)child);
        }
    }

    return wxDataViewItem(nullptr);
}

void xLightsImportTreeModel::ClearMapping()
{
    size_t count = m_children.size();
    for (size_t i = 0; i < count; ++i) {
        GetNthChild(i)->ClearMapping();
    }
}

const long xLightsImportChannelMapDialog::ID_TREELISTCTRL1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_CHOICE = wxNewId();

//(*IdInit(xLightsImportChannelMapDialog)
const wxWindowID xLightsImportChannelMapDialog::ID_SPINCTRL1 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_CHECKBOX1 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_CHECKBOX11 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_CHECKBOX4 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_CHECKBOX5 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_CHECKBOX2 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_STATICTEXT_BLEND_TYPE = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_CHECKBOX3 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_BUTTON_IMPORT_OPTIONS = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_CHECKLISTBOX1 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_STATICTEXT2 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_TEXTCTRL2 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_BUTTON3 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_BUTTON4 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_BUTTON5 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_BUTTON7 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_BUTTON6 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_BUTTON2 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_BUTTON1 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_PANEL1 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_STATICTEXT1 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_TEXTCTRL1 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_LISTCTRL1 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_PANEL2 = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

const long xLightsImportChannelMapDialog::ID_MNU_SELECTALL = wxNewId();
const long xLightsImportChannelMapDialog::ID_MNU_SELECTNONE = wxNewId();
const long xLightsImportChannelMapDialog::ID_MNU_COLLAPSEALL = wxNewId();
const long xLightsImportChannelMapDialog::ID_MNU_EXPANDALL = wxNewId();
const long xLightsImportChannelMapDialog::ID_MNU_SHOWALLMAPPED = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_MNU_CLEARSELECTED = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_MNU_CLEARALL = wxNewId();
const long xLightsImportChannelMapDialog::ID_MNU_AUTOMAPSELECTED = wxNewId();
const wxWindowID xLightsImportChannelMapDialog::ID_MNU_ADD_EMPTY_GROUP = wxNewId();


BEGIN_EVENT_TABLE(xLightsImportChannelMapDialog,wxDialog)
//(*EventTable(xLightsImportChannelMapDialog)
//*)
END_EVENT_TABLE()

xLightsImportChannelMapDialog::xLightsImportChannelMapDialog(xLightsFrame* parent, const wxFileName& filename, bool allowTimingOffset, bool allowTimingTrack, bool allowColorChoice, bool allowCCRStrand, bool allowImportBlend, wxWindowID id, const wxPoint& pos, const wxSize& size) : xlights(parent)
{
    TreeListCtrl_Mapping = nullptr;
    _dataModel = nullptr;
    _sortOrder = 0;
    _allowTimingOffset = allowTimingOffset;
    _allowTimingTrack = allowTimingTrack;
    _allowColorChoice = allowColorChoice;
    _allowCCRStrand = allowCCRStrand;
    _allowImportBlend = allowImportBlend;
    _filename = filename;
    _dragItem = wxDataViewItem(nullptr);

    //(*Initialize(xLightsImportChannelMapDialog)
    wxButton* Button01;
    wxButton* Button02;
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer12;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer9;

    Create(parent, wxID_ANY, _("Map Channels"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX|wxFULL_REPAINT_ON_RESIZE, _T("wxID_ANY"));
    OldSizer = new wxFlexGridSizer(0, 1, 0, 0);
    OldSizer->AddGrowableCol(0);
    OldSizer->AddGrowableRow(0);
    SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D|wxSP_LIVE_UPDATE, _T("ID_SPLITTERWINDOW1"));
    SplitterWindow1->SetSashGravity(0.5);
    Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    Panel1->SetMinSize(wxSize(500,-1));
    Sizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    Sizer1->AddGrowableCol(0);
    Sizer1->AddGrowableRow(7);
    Sizer_TimeAdjust = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText_TimeAdjust = new wxStaticText(Panel1, wxID_ANY, _("Time Adjust (ms)"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    Sizer_TimeAdjust->Add(StaticText_TimeAdjust, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TimeAdjustSpinCtrl = new wxSpinCtrl(Panel1, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -10000, 600000, 0, _T("ID_SPINCTRL1"));
    TimeAdjustSpinCtrl->SetValue(_T("0"));
    Sizer_TimeAdjust->Add(TimeAdjustSpinCtrl, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Sizer1->Add(Sizer_TimeAdjust, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    CheckBox_MapCCRStrand = new wxCheckBox(Panel1, ID_CHECKBOX1, _("Map CCR/Strand"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_MapCCRStrand->SetValue(false);
    FlexGridSizer1->Add(CheckBox_MapCCRStrand, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Sizer1->Add(FlexGridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer11 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer11->AddGrowableCol(0);
    CheckBox_EraseExistingEffects = new wxCheckBox(Panel1, ID_CHECKBOX11, _("Erase existing effects on imported models"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX11"));
    CheckBox_EraseExistingEffects->SetValue(false);
    FlexGridSizer11->Add(CheckBox_EraseExistingEffects, 1, wxALL|wxEXPAND, 5);
    CheckBox_LockEffects = new wxCheckBox(Panel1, ID_CHECKBOX4, _("Lock effects on import"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
    CheckBox_LockEffects->SetValue(true);
    FlexGridSizer11->Add(CheckBox_LockEffects, 1, wxALL|wxEXPAND, 5);
    CheckBox_ConvertRenderStyle = new wxCheckBox(Panel1, ID_CHECKBOX5, _("Convert Render Style"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
    CheckBox_ConvertRenderStyle->SetValue(false);
    CheckBox_ConvertRenderStyle->SetToolTip(_("When mapping model to group, convert render style to \'Per Model\' when applicable"));
    FlexGridSizer11->Add(CheckBox_ConvertRenderStyle, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    Sizer1->Add(FlexGridSizer11, 1, wxALL|wxEXPAND, 1);
    FlexGridSizer_Blend_Mode = new wxFlexGridSizer(0, 2, 0, 0);
    CheckBox_Import_Blend_Mode = new wxCheckBox(Panel1, ID_CHECKBOX2, _("Import Model Blend Mode"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    CheckBox_Import_Blend_Mode->SetValue(true);
    FlexGridSizer_Blend_Mode->Add(CheckBox_Import_Blend_Mode, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Blend_Type = new wxStaticText(Panel1, ID_STATICTEXT_BLEND_TYPE, _("Blend Mode"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_BLEND_TYPE"));
    FlexGridSizer_Blend_Mode->Add(StaticText_Blend_Type, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Sizer1->Add(FlexGridSizer_Blend_Mode, 1, wxALL|wxEXPAND, 1);
    FlexGridSizerImportMedia = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBoxImportMedia = new wxCheckBox(Panel1, ID_CHECKBOX3, _("Import Media"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
    CheckBoxImportMedia->SetValue(true);
    FlexGridSizerImportMedia->Add(CheckBoxImportMedia, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonImportOptions = new wxButton(Panel1, ID_BUTTON_IMPORT_OPTIONS, _("View/Change Options"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_IMPORT_OPTIONS"));
    FlexGridSizerImportMedia->Add(ButtonImportOptions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Sizer1->Add(FlexGridSizerImportMedia, 1, wxALL|wxEXPAND, 1);
    TimingTrackPanel = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Timing Tracks"));
    TimingTrackListBox = new wxCheckListBox(Panel1, ID_CHECKLISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_MULTIPLE|wxVSCROLL, wxDefaultValidator, _T("ID_CHECKLISTBOX1"));
    TimingTrackPanel->Add(TimingTrackListBox, 1, wxALL|wxEXPAND, 0);
    Sizer1->Add(TimingTrackPanel, 0, wxEXPAND, 0);
    FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer4->AddGrowableCol(1);
    StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("Find:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer4->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_FindTo = new wxTextCtrl(Panel1, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    FlexGridSizer4->Add(TextCtrl_FindTo, 1, wxALL|wxEXPAND, 5);
    Sizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
    SizerMap = new wxFlexGridSizer(1, 1, 0, 0);
    SizerMap->AddGrowableCol(0);
    SizerMap->AddGrowableRow(0);
    Sizer1->Add(SizerMap, 0, wxEXPAND, 0);
    FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizer2->AddGrowableCol(3);
    FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer7 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_Ok = new wxButton(Panel1, ID_BUTTON3, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer7->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Cancel = new wxButton(Panel1, ID_BUTTON4, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer7->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer7, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer5, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
    Button_AutoMap = new wxButton(Panel1, ID_BUTTON5, _("Auto Map"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    FlexGridSizer6->Add(Button_AutoMap, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 5);
    Button_AIMap = new wxButton(Panel1, ID_BUTTON7, _("AI Map"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
    FlexGridSizer6->Add(Button_AIMap, 1, wxALL|wxEXPAND, 5);
    Button_UpdateAliases = new wxButton(Panel1, ID_BUTTON6, _("Update Aliases w/ Maps"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
    FlexGridSizer6->Add(Button_UpdateAliases, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizer8->Add(FlexGridSizer6, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer8, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
    Button02 = new wxButton(Panel1, ID_BUTTON2, _("Save Map"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer10->Add(Button02, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 5);
    Button01 = new wxButton(Panel1, ID_BUTTON1, _("Load Map"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer10->Add(Button01, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizer9->Add(FlexGridSizer10, 1, wxALIGN_LEFT, 5);
    FlexGridSizer2->Add(FlexGridSizer9, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer12 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer12->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Sizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
    Panel1->SetSizer(Sizer1);
    Panel2 = new wxPanel(SplitterWindow1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    Panel2->SetMinSize(wxSize(400,-1));
    Sizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    Sizer2->AddGrowableCol(0);
    Sizer2->AddGrowableRow(1);
    FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer3->AddGrowableCol(1);
    StaticText1 = new wxStaticText(Panel2, ID_STATICTEXT1, _("Find:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_FindFrom = new wxTextCtrl(Panel2, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer3->Add(TextCtrl_FindFrom, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
    Sizer2->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
    ListCtrl_Available = new wxListCtrl(Panel2, ID_LISTCTRL1, wxDefaultPosition, wxDLG_UNIT(Panel2,wxSize(100,-1)), wxLC_REPORT|wxVSCROLL, wxDefaultValidator, _T("ID_LISTCTRL1"));
    Sizer2->Add(ListCtrl_Available, 1, wxALL|wxEXPAND, 5);
    Panel2->SetSizer(Sizer2);
    SplitterWindow1->SplitVertically(Panel1, Panel2);
    OldSizer->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
    SetSizer(OldSizer);
    OldSizer->SetSizeHints(this);

    Connect(ID_CHECKBOX1, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnCheckBox_MapCCRStrandClick);
    Connect(ID_CHECKBOX3, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnCheckBoxImportMediaClick);
    Connect(ID_BUTTON_IMPORT_OPTIONS, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnButtonImportOptionsClick);
    Connect(ID_TEXTCTRL2, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnTextCtrl_FindToText);
    Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnButton_OkClick);
    Connect(ID_BUTTON4, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnButton_CancelClick);
    Connect(ID_BUTTON5, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnButton_AutoMapClick);
    Connect(ID_BUTTON7, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnButton_AIMapClick);
    Connect(ID_BUTTON6, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnButton_UpdateAliasesClick);
    Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::SaveMapping);
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::LoadMapping);
    Connect(ID_TEXTCTRL1, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnTextCtrl_FindFromText);
    Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_BEGIN_DRAG, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnListCtrl_AvailableBeginDrag);
    Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_ITEM_SELECTED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnListCtrl_AvailableItemSelect);
    Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_ITEM_ACTIVATED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnListCtrl_AvailableItemActivated);
    Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_COL_CLICK, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnListCtrl_AvailableColumnClick);
    //*)

    Connect(ID_CHECKLISTBOX1, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xLightsImportChannelMapDialog::RightClickTimingTracks);
    Connect(ID_LISTCTRL1, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xLightsImportChannelMapDialog::RightClickModelsAvail);

    SetSize(800, 600);

    if (_filename != "") {
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
    if (loc.x != -1) {
        if (sz.GetWidth() < 400) sz.SetWidth(400);
        if (sz.GetHeight() < 300) sz.SetHeight(300);
        SetPosition(loc);
        SetSize(sz);
        Layout();
    }

    wxConfigBase* config = wxConfigBase::Get();
    CheckBox_LockEffects->SetValue(config->ReadBool("ImportEffectsLocked", false));
    CheckBox_ConvertRenderStyle->SetValue(config->ReadBool("ImportEffectsRenderStyle", false));

    auto ai = xlights->GetAIService();
    Button_AIMap->Enable(ai != nullptr);

    EnsureWindowHeaderIsOnScreen(this);
}

void xLightsImportChannelMapDialog::RightClickTimingTracks(wxContextMenuEvent& event)
{
    wxMenu mnuLayer;
    mnuLayer.Append(ID_MNU_SELECTALL, "Select All");
    mnuLayer.Append(ID_MNU_SELECTNONE, "Select None");
    mnuLayer.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnPopupTimingTracks, nullptr, this);
    PopupMenu(&mnuLayer);
}

void xLightsImportChannelMapDialog::RightClickModels(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();
    if (item.IsOk()) {
        wxMenu mnuLayer;
        mnuLayer.Append(ID_MNU_COLLAPSEALL, "Collapse All");
        mnuLayer.Append(ID_MNU_EXPANDALL, "Expand All");
        mnuLayer.Append(ID_MNU_SHOWALLMAPPED, "Show All Mapped Models");
        mnuLayer.Append(ID_MNU_AUTOMAPSELECTED, "Auto Map Selected");
        mnuLayer.AppendSeparator();
        mnuLayer.Append(ID_MNU_CLEARALL, "Clear All");
        mnuLayer.Append(ID_MNU_CLEARSELECTED, "Clear Selected");
        mnuLayer.AppendSeparator();
        mnuLayer.Append(ID_MNU_ADD_EMPTY_GROUP, "Add Empty Group");
        mnuLayer.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnPopupModels, nullptr, this);
        TreeListCtrl_Mapping->PopupMenu(&mnuLayer, event.GetPosition());
    }
}

void xLightsImportChannelMapDialog::RightClickModelsAvail(wxDataViewEvent& event) {
        wxMenu mnuLayer;
        mnuLayer.Append(ID_MNU_AUTOMAPSELECTED, "Auto Map Selected");
        mnuLayer.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnPopupModels, nullptr, this);
        PopupMenu(&mnuLayer);
}

void xLightsImportChannelMapDialog::OnPopupModels(wxCommandEvent& event)
{
    int id = event.GetId();
    if (id == ID_MNU_EXPANDALL) {
        ExpandAll();
    } else if (id == ID_MNU_COLLAPSEALL) {
        CollapseAll();
    } else if (id == ID_MNU_SHOWALLMAPPED) {
        ShowAllMapped();
    } else if (id == ID_MNU_AUTOMAPSELECTED) {
        OnButton_AutoMapSelClick(event);
    } else if (id == ID_MNU_CLEARSELECTED) {
        ClearSelected();
    } else if (id == ID_MNU_CLEARALL) {
        ClearAll();
    } else if (id == ID_MNU_ADD_EMPTY_GROUP) {
        AddEmptyGroup();
    }
}

void xLightsImportChannelMapDialog::AddEmptyGroup()
{
    wxTextEntryDialog dialog(this, "Enter the name for the new group:", "Add New Group", "");
    if (dialog.ShowModal() != wxID_OK) {
        return;
    }

    wxString groupName = dialog.GetValue().Trim();
    if (groupName.IsEmpty()) {
        wxMessageBox("Group name cannot be empty.", "Error", wxOK | wxICON_ERROR, this);
        return;
    }

    for (size_t i = 0; i < _dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* existingNode = _dataModel->GetNthChild(i);
        if (existingNode->_model == groupName) {
            wxMessageBox("A group or model with the name '" + groupName + "' already exists.", "Error", wxOK | wxICON_ERROR, this);
            return;
        }
    }

    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "modelGroup");
    xlights->ModelGroupsNode->AddChild(node);
    node->AddAttribute("selected", "0");
    node->AddAttribute("name", groupName);
    node->AddAttribute("models", "");
    node->AddAttribute("layout", "minimalGrid");
    node->AddAttribute("GridSize", "400");
    node->AddAttribute("LayoutGroup", "Default");

    xLightsImportModelNode* newGroup = new xLightsImportModelNode(
        nullptr, groupName, "", true, std::list<std::string>{}, "ModelGroup", "", false, "ModelGroup", 1000,
        *wxWHITE, true, "", 0);

    _dataModel->BulkInsert(newGroup, _dataModel->GetChildCount());

    wxDataViewItem parentItem;
    wxDataViewItem newItem(newGroup);
    _dataModel->ItemAdded(parentItem, newItem);

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnModelsPopup::ID_MNU_ADD_MODEL_GROUP");
    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::OnModelsPopup::ID_MNU_ADD_MODEL_GROUP", nullptr, nullptr, groupName.ToStdString());

    TreeListCtrl_Mapping->Refresh();
    DisplayInfo("Group '" + groupName + "' added successfully.", this);
}

void xLightsImportChannelMapDialog::CollapseAll()
{
    wxDataViewItemArray models;
    _dataModel->GetChildren(wxDataViewItem(0), models);
    for (size_t i = 0; i < models.size(); ++i) {
        TreeListCtrl_Mapping->Collapse(models[i]);
    }
}

void xLightsImportChannelMapDialog::ExpandAll()
{
    wxDataViewItemArray models;
    _dataModel->GetChildren(wxDataViewItem(0), models);
    for (size_t i = 0; i < models.size(); ++i) {
        wxDataViewItemArray strands;
        _dataModel->GetChildren(models[i], strands);
        for (size_t j = 0; j < strands.size(); ++j) {
            xLightsImportModelNode* astrand = (xLightsImportModelNode*)strands[j].GetID();
            if (!StartsWith(astrand->_strand, "Strand")) {
                TreeListCtrl_Mapping->Expand(models[i]);
            }
        }
    }
}

void xLightsImportChannelMapDialog::ClearAll() {
    if (_dataModel == nullptr)
        return;

    _dirty = true;
    TreeListCtrl_Mapping->Freeze();

    for (unsigned int i = 0; i < _dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* model = _dataModel->GetNthChild(i);
        if (model != nullptr) {
            model->ClearMapping();
            for (unsigned int j = 0; j < model->GetChildCount(); ++j) {
                xLightsImportModelNode* strand = model->GetNthChild(j);
                if (strand != nullptr) {
                    strand->ClearMapping();
                    for (unsigned int k = 0; k < strand->GetChildCount(); ++k) {
                        xLightsImportModelNode* node = strand->GetNthChild(k);
                        if (node != nullptr) {
                            node->ClearMapping();
                        }
                    }
                }
            }
            wxDataViewItem item(model);
            _dataModel->ValueChanged(item, 2);
            _dataModel->SetMappingExists(item, true);
        }
    }

    TreeListCtrl_Mapping->Thaw();
    TreeListCtrl_Mapping->Refresh();
    MarkUsed();
}

void xLightsImportChannelMapDialog::ClearSelected() {
    if (_dataModel == nullptr)
        return;

    _dirty = true;
    TreeListCtrl_Mapping->Freeze();

    wxDataViewItemArray selectedItems;
    TreeListCtrl_Mapping->GetSelections(selectedItems);

    if (selectedItems.empty()) {
        TreeListCtrl_Mapping->Thaw();
        return;
    }

    for (const auto& item : selectedItems) {
        xLightsImportModelNode* node = (xLightsImportModelNode*)item.GetID();
        if (node != nullptr) {
            if (node->GetParent() == nullptr && node->GetChildCount() > 0) {
                if (!TreeListCtrl_Mapping->IsExpanded(item)) {
                    node->ClearMapping();
                    for (unsigned int j = 0; j < node->GetChildCount(); ++j) {
                        xLightsImportModelNode* strand = node->GetNthChild(j);
                        if (strand != nullptr) {
                            strand->ClearMapping();
                            for (unsigned int k = 0; k < strand->GetChildCount(); ++k) {
                                xLightsImportModelNode* subNode = strand->GetNthChild(k);
                                if (subNode != nullptr) {
                                    subNode->ClearMapping();
                                }
                            }
                            wxDataViewItem strandItem(strand);
                            _dataModel->ValueChanged(strandItem, 2);
                            _dataModel->SetMappingExists(strandItem, true);
                        }
                    }
                    _dataModel->ValueChanged(item, 2);
                    _dataModel->SetMappingExists(item, true);
                } else {
                    Unmap(item);
                    _dataModel->ValueChanged(item, 2);
                }
            } else {
                node->ClearMapping();
                _dataModel->ValueChanged(item, 2);
                _dataModel->SetMappingExists(item, true);
            }
        }
    }

    TreeListCtrl_Mapping->Thaw();
    TreeListCtrl_Mapping->Refresh();
    MarkUsed();
}

void xLightsImportChannelMapDialog::ShowAllMapped()
{
    if (_dataModel == nullptr) 
        return;

    // expand all models that have strands that have a value
    wxDataViewItemArray models;
    _dataModel->GetChildren(wxDataViewItem(0), models);
    for (size_t i = 0; i < models.size(); ++i) {
        wxDataViewItemArray strands;
        _dataModel->GetChildren(models[i], strands);
        for (size_t j = 0; j < strands.size(); ++j) {
            xLightsImportModelNode* astrand = (xLightsImportModelNode*)strands[j].GetID();
            if (astrand->HasMapping()) {
                TreeListCtrl_Mapping->Expand(models[i]);
            }
        }
    }
}

void xLightsImportChannelMapDialog::OnPopupTimingTracks(wxCommandEvent& event)
{
    for (unsigned int i = 0; i < TimingTrackListBox->GetCount(); ++i) {
        TimingTrackListBox->Check(i, event.GetId() == ID_MNU_SELECTALL);
    }
}

xLightsImportChannelMapDialog::~xLightsImportChannelMapDialog()
{
	//(*Destroy(xLightsImportChannelMapDialog)
	//*)

    // disconnect the model and then delete it ... this ensures the destructors are called
    // which stops memory leaks
    if (TreeListCtrl_Mapping != nullptr) {
        TreeListCtrl_Mapping->AssociateModel(nullptr);
        if (_dataModel != nullptr) {
            delete _dataModel;
            _dataModel = nullptr;
        }
    }

    SaveWindowPosition("xLightsImportDialogPosition", this);
}

bool xLightsImportChannelMapDialog::InitImport(std::string checkboxText) {
    if (_xsqPkg != nullptr && _xsqPkg->IsPkg()) {
        SetImportMediaTooltip();
    } else {
        Sizer1->Hide(FlexGridSizerImportMedia, true);
    }
    if (_xsqPkg != nullptr && _xsqPkg->HasRGBEffects()) {
        LoadRgbEffectsFile();
    }

    if (importChannels.size() == 0 && timingTracks.size() == 0)
    {
        DisplayError("No models/timing tracks to import from. Source sequence had no data.");
        return false;
    }

    if (!_allowCCRStrand) {
        Sizer1->Hide(FlexGridSizer1, true);
    }

    if (!_allowTimingOffset) {
        Sizer1->Hide(Sizer_TimeAdjust, true);
    }

    if (timingTracks.empty() || !_allowTimingTrack) {
        Sizer1->Hide(TimingTrackPanel, true);
    } else {
        for (const auto& it : timingTracks) {
            int item = TimingTrackListBox->Append(it);
            if (!timingTrackAlreadyExists[it]) {
                TimingTrackListBox->Check(item, true);
            }
        }
    }

    if (!checkboxText.empty()) {
        CheckBox_MapCCRStrand->SetLabelText(checkboxText);
    }

    if (!_allowImportBlend) {
        Sizer1->Hide(FlexGridSizer_Blend_Mode, true);
    }

    wxVector<wxBitmapBundle> images;
    LayoutUtils::CreateImageList(images, m_iconIndexMap);
    ListCtrl_Available->SetSmallImages(images);
    PopulateAvailable(false);

    _dataModel = new xLightsImportTreeModel();
    //fill in the datamodel prior to sticking it in the tree
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
#ifndef __WXMSW__
    if (_dataModel->GetChildCount() != 0) {
        _dataModel->Resort();
    }
#endif
    TreeListCtrl_Mapping = new wxDataViewCtrl(Panel1, ID_TREELISTCTRL1, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES | wxDV_VERT_RULES | wxDV_MULTIPLE, wxDefaultValidator);
    TreeListCtrl_Mapping->Freeze();
    TreeListCtrl_Mapping->AssociateModel(_dataModel);
    TreeListCtrl_Mapping->AppendColumn(new wxDataViewColumn("Model", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT, wxALIGN_LEFT), 0, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE));
    TreeListCtrl_Mapping->GetColumn(0)->SetSortOrder(true);
    TreeListCtrl_Mapping->AppendColumn(new wxDataViewColumn("# Effects", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_ACTIVATABLE, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL), 1, wxCOL_WIDTH_AUTOSIZE, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE));
    TreeListCtrl_Mapping->AppendColumn(new wxDataViewColumn("Map To", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_ACTIVATABLE, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL), 2, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
    TreeListCtrl_Mapping->AppendColumn(new wxDataViewColumn("Model Type", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_ACTIVATABLE, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL), 3, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
    if (_allowColorChoice) {
        TreeListCtrl_Mapping->AppendColumn(new wxDataViewColumn("Color", new ColorRenderer(), 4, 100, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
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
    Connect(ID_TREELISTCTRL1, wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, (wxObjectEventFunction)&xLightsImportChannelMapDialog::RightClickModels);


#ifdef __WXOSX__
    Connect(ID_TREELISTCTRL1, wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnDragPossible);
    Connect(ID_TREELISTCTRL1, wxEVT_DATAVIEW_ITEM_DROP, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnDragDrop);
    TreeListCtrl_Mapping->EnableDropTarget(wxDataFormat(wxDF_TEXT));
#else
    MDTextDropTarget* mdt = new MDTextDropTarget(this, TreeListCtrl_Mapping, "Map");
    TreeListCtrl_Mapping->SetDropTarget(mdt);
#endif
    TreeListCtrl_Mapping->SetIndent(8);
    TreeListCtrl_Mapping->GetColumn(0)->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    TreeListCtrl_Mapping->Thaw();
    TreeListCtrl_Mapping->Refresh();

    if (_dataModel->GetChildCount() == 0) {
        DisplayError("No models to import to. Add some models to the rows of the effects grid.");
        return false;
    }
#ifdef __WXMSW__
    else {
        _dataModel->Resort();
    }
#endif
    return true;
}

void xLightsImportChannelMapDialog::SetModelBlending(bool enabled)
{
    wxString text = wxString::Format("Model Blending is %s in source file.",( enabled ? "ENABLED" : "DISABLED"));
    StaticText_Blend_Type->SetLabelText(text);
}

bool xLightsImportChannelMapDialog::GetImportModelBlending() const
{
    return CheckBox_Import_Blend_Mode->IsChecked();
}

bool xLightsImportChannelMapDialog::IsLockEffects() const
{
    wxConfigBase* config = wxConfigBase::Get();
    bool b = CheckBox_LockEffects->IsChecked();
    config->Write("ImportEffectsLocked", b);
    config->Flush();
    return b;
}

void xLightsImportChannelMapDialog::SetXsqPkg(SequencePackage* xsqPkg) {
    _xsqPkg = xsqPkg;
}

void xLightsImportChannelMapDialog::PopulateAvailable(bool ccr)
{
    ListCtrl_Available->Freeze();
    ListCtrl_Available->ClearAll();

    // load the available list
    ListCtrl_Available->AppendColumn("Available");
    ListCtrl_Available->SetColumnWidth(0, wxLIST_AUTOSIZE);
    if (ListCtrl_Available->GetColumnWidth(0) < 150) {
        ListCtrl_Available->SetColumnWidth(0, 150);
    }

    if (ccr) {
        int j{0};
        for (auto const& name : ccrNames) {
            ListCtrl_Available->InsertItem(j, name);
            ListCtrl_Available->SetItemData(j, j);
            ListCtrl_Available->SetItemColumnImage(j, 0, -1);
            j++;
        }
    } else {
        int j{0};

        int colIndex = ListCtrl_Available->AppendColumn("# Effects");
        wxListItem column;
        column.SetId(colIndex);
        column.SetAlign(wxLIST_FORMAT_CENTER);
        ListCtrl_Available->SetColumn(colIndex, column);

        bool countEnabled{false};
        for (auto const& m : importChannels) {
            ListCtrl_Available->InsertItem(j, m->name);
            wxUIntPtr ptr = (wxUIntPtr)m.get();
            ListCtrl_Available->SetItemPtrData(j, ptr);
            if (!m->type.empty()) {
                ListCtrl_Available->SetItemColumnImage(j, 0, m_iconIndexMap[LayoutUtils::GetModelTreeIcon(m->type, LayoutUtils::GroupMode::Regular)]);
            } else {
                ListCtrl_Available->SetItemColumnImage(j, 0, -1);
            }
            if (m->effectCount != 0) {
                ListCtrl_Available->SetItem(j, 1, wxString::Format("%d", m->effectCount));
                countEnabled = true;
            }

            // If importing from xsqPkg flag known groups by color like is currently done in mapped list
            if (m->type == "ModelGroup") {
                ListCtrl_Available->SetItemTextColour(j, ColorManager::instance()->CyanOrBlueOverride());
            }
            j++;
        }
        if (!countEnabled) {
            ListCtrl_Available->DeleteColumn(1);
        }
    }

    _sortOrder = 1;

    ListCtrl_Available->SortItems(MyCompareFunctionAscName, (wxIntPtr)CheckBox_MapCCRStrand->GetValue());
    ListCtrl_Available->ShowSortIndicator(0, true);

    // Set Autosize Width after it is populated or it doesn't work
    ListCtrl_Available->SetColumnWidth(0, wxLIST_AUTOSIZE);
    if (ListCtrl_Available->GetColumnWidth(0) < 150) {
        ListCtrl_Available->SetColumnWidth(0, 150);
    }

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

    std::string groupModels = "";
    if (dynamic_cast<ModelGroup*>(m) != nullptr) {
        ModelGroup* grp = dynamic_cast<ModelGroup*>(m);
        if (grp != nullptr) {
            auto modelNames = grp->ModelNames();
            for (const auto& it : modelNames) {
                    if (groupModels != "") {
                        groupModels += ",";
                    }
                    groupModels += it;
            }
        }
    }

    std::string modelClass = Model::DetermineClass(m->GetDisplayAs(), m->GetFaceInfo().size() != 0, 
        m->GetModelXml()->GetAttribute("TreeSpiralRotations", "0") != "0", 
        m->GetModelXml()->GetAttribute("CandyCaneSticks", "false") == "true", 
        m->GetModelXml()->GetAttribute("DropPattern", ""));

    int effectCount = 0;
    Element* em = xlights->GetSequenceElements().GetElement(m->GetName());
    if (em != nullptr) {
        for (auto* layer : em->GetEffectLayers()) {
            if (layer != nullptr) {
                effectCount += layer->GetEffectCount();
            }
        }
    }

    xLightsImportModelNode* lastmodel = new xLightsImportModelNode(nullptr, m->GetName(), std::string(""), true, m->GetAliases(), m->GetDisplayAs(), groupModels, false, modelClass, m->GetNodeCount(), *wxWHITE, (m->GetDisplayAs() == "ModelGroup"), wxString(""), effectCount);
    _dataModel->BulkInsert(lastmodel, ms++);

     for (int s = 0; s < m->GetNumSubModels(); ++s) {
        effectCount = 0;
        if (em != nullptr) {
            ModelElement* el = dynamic_cast<ModelElement*>(em);
            if (el != nullptr) {
                SubModelElement* sme = el->GetSubModel(s);
                if (sme != nullptr) {
                    for (auto* layer : sme->GetEffectLayers()) {
                        if (layer != nullptr) {
                            effectCount += layer->GetEffectCount();
                        }
                    }
                }
            }
        }
        Model *subModel = m->GetSubModel(s);
        xLightsImportModelNode* laststrand;
        // note we deliberately passing in the models node count ... as this is most relevant
        if (channelColors.find(subModel->GetName()) != channelColors.end()) {
            laststrand = new xLightsImportModelNode(lastmodel, wxString(m->GetName()), wxString(subModel->GetName()), std::string(""), true, m->GetAliases(), m->GetDisplayAs(), "", true, "", m->GetNodeCount(), channelColors.find(subModel->GetName())->second.asWxColor(), wxString(""), effectCount);
        } else {
            laststrand = new xLightsImportModelNode(lastmodel, wxString(m->GetName()), wxString(subModel->GetName()), std::string(""), true, m->GetAliases(), m->GetDisplayAs(), "", true, "", m->GetNodeCount(), *wxWHITE, wxString(""), effectCount);
        }
        lastmodel->Append(laststrand);
    }

    for (int s = 0; s < m->GetNumStrands(); ++s) {
        wxString sn = m->GetStrandName(s);
        if (sn == "") {
            sn = wxString::Format("Strand %d", s + 1);
        }
        effectCount = 0;
        xLightsImportModelNode* laststrand;
        // note we deliberately passing in the models node count ... as this is most relevant
        if (channelColors.find(sn.ToStdString()) != channelColors.end()) {
            laststrand = new xLightsImportModelNode(lastmodel, wxString(m->GetName()), sn, wxString(""), true, m->GetAliases(), m->GetDisplayAs(), "", false, "", m->GetNodeCount(), channelColors.find(sn.ToStdString())->second.asWxColor(), wxString(""), effectCount);
        } else {
            laststrand = new xLightsImportModelNode(lastmodel, wxString(m->GetName()), sn, wxString(""), true, m->GetAliases(), m->GetDisplayAs(), "", false, "", m->GetNodeCount(), *wxWHITE, wxString(""), effectCount);
        }
        lastmodel->Append(laststrand);
        for (int n = 0; n < m->GetStrandLength(s); ++n) {
            wxString nn = m->GetNodeName(n);
            if (nn == "") {
                nn = wxString::Format("Node %d", n + 1);
            }
            effectCount = 0;
            xLightsImportModelNode* lastnode;
            if (channelColors.find(nn.ToStdString()) != channelColors.end()) {
                lastnode = new xLightsImportModelNode(laststrand, m->GetName(), sn, nn, std::string(""), true, m->GetAliases(), m->GetDisplayAs(), "", false, "", m->GetNodeCount(), channelColors.find(nn.ToStdString())->second.asWxColor(), wxString(""), effectCount);
            } else {
                lastnode = new xLightsImportModelNode(laststrand, m->GetName(), sn, nn, std::string(""), true, m->GetAliases(), m->GetDisplayAs(), "", false, "", m->GetNodeCount(), *wxWHITE, wxString(""), effectCount);
            }
            laststrand->Insert(lastnode, n);
        }
    }
    if (dynamic_cast<ModelGroup*>(m) != nullptr) {
        ModelGroup *grp = dynamic_cast<ModelGroup*>(m);
        if (grp != nullptr) {
            auto modelNames = grp->ModelNames();

            for (const auto& it : modelNames) {
                if (std::find(it.begin(), it.end(), '/') != it.end()) {
                    // this is a submodel ... don't add it

                    // The risk here is the submodel is in the group but the parent model isn't so there will be no way to
                    // map it. Maybe we should grab the parent model and ensure it is included in the list
                    // Given up until this change this actually crashed my guess is no one is screaming for it now
                } else {
                    Model* mdl = grp->GetModel(it);
                    if (mdl != nullptr) {
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
    if (key == WXK_DELETE || key == WXK_NUMPAD_DELETE) {
        if (TreeListCtrl_Mapping->GetSelectedItemsCount() > 0) {
            wxDataViewItemArray selectedItems;
            TreeListCtrl_Mapping->GetSelections(selectedItems);

            for (const auto& item : selectedItems) {
                Unmap(item);
            }
        }
    }
}

void xLightsImportChannelMapDialog::OnItemActivated(wxDataViewEvent& event)
{
    if (event.GetItem().IsOk()) {
        wxVariant vvalue;
        event.GetModel()->GetValue(vvalue, event.GetItem(), 2);
        std::string mapped = vvalue.GetString().ToStdString();
        TreeListCtrl_Mapping->UnselectAll();
        TreeListCtrl_Mapping->Select(event.GetItem());
        if (mapped == "" && ListCtrl_Available->GetSelectedItemCount() > 0) {
            int itemIndex = ListCtrl_Available->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            Map(event.GetItem(), ListCtrl_Available->GetItemText(itemIndex).ToStdString(), findModelType(ListCtrl_Available->GetItemText(itemIndex)));
        } else {
            Unmap(event.GetItem());
        }
        TreeListCtrl_Mapping->Refresh();
    }
}

void xLightsImportChannelMapDialog::Map(const wxDataViewItem& item, const wxString& mapping, const wxString& mappingModelType)
{
    _dirty = true;
    TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(mapping), item, 2);
    TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(mappingModelType), item, 3);
    ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel())->SetMappingExists(item, true);
    MarkUsed();
}

void xLightsImportChannelMapDialog::Unmap(const wxDataViewItem& item)
{
    _dirty = true;
    TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(""), item, 2);
    TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(""), item, 3);
    ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel())->SetMappingExists(item, true);
    if (_allowColorChoice) {
        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(wxWHITE->GetAsString()), item, 4);
    }
    MarkUsed();
}

void xLightsImportChannelMapDialog::OnSelectionChanged(wxDataViewEvent& event)
{
}

void xLightsImportChannelMapDialog::OnValueChanged(wxDataViewEvent& event)
{
    _dirty = true;
    if (_allowColorChoice) {
        if (event.GetItem().IsOk()) {
            if (event.GetColumn() == 2) {
                wxVariant vvalue;
                event.GetModel()->GetValue(vvalue, event.GetItem(), 2);
                std::string value = vvalue.GetString().ToStdString();
                if (channelColors.find(value) != channelColors.end()) {
                    event.GetModel()->SetValue(wxVariant(channelColors.find(value)->second.asWxColor().GetAsString()), event.GetItem(), 4);
                }
            }
        }
    }
}

wxString xLightsImportChannelMapDialog::FindTab(wxString &line) {
    for (size_t x = 0; x < line.size(); ++x) {
        if (line[x] == '\t') {
            wxString first = line.SubString(0, x - 1);
            line = line.SubString(x+1, line.size());
            return first;
        }
    }
    return line;
}

wxDataViewItem xLightsImportChannelMapDialog::FindItem(std::string const& model, std::string const& strand, std::string const& node)
{
    wxDataViewItemArray models;
    _dataModel->GetChildren(wxDataViewItem(0), models);
    for (size_t i = 0; i < models.size(); ++i) {
        xLightsImportModelNode* amodel = (xLightsImportModelNode*)models[i].GetID();
        if (amodel->_model == model) {
            if (strand == "") {
                return models[i];
            } else {
                wxDataViewItemArray strands;
                _dataModel->GetChildren(models[i], strands);
                for (size_t j = 0; j < strands.size(); ++j) {
                    xLightsImportModelNode* astrand = (xLightsImportModelNode*)strands[j].GetID();
                    if (astrand->_strand == strand) {
                        if (node == "") {
                            return strands[j];
                        } else {
                            wxDataViewItemArray nodes;
                            _dataModel->GetChildren(strands[j], nodes);
                            for (size_t k = 0; k < nodes.size(); ++k) {
                                xLightsImportModelNode* anode = (xLightsImportModelNode*)nodes[k].GetID();
                                if (anode->_node == node) {
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

xLightsImportModelNode* xLightsImportChannelMapDialog::TreeContainsModel(std::string const& model, std::string const& strand, std::string const& node)
{
    for (size_t i = 0; i < _dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = _dataModel->GetNthChild(i);
        if (m->_model == model) {
            if (strand == "") {
                return m;
            } else {
                for (size_t j = 0; j < m->GetChildCount(); ++j) {
                    xLightsImportModelNode* s = m->GetNthChild(j);
                    if (s->_strand == strand) {
                        if (node == "") {
                            return s;
                        } else {
                            for (size_t k = 0; k < s->GetChildCount(); ++k) {
                                xLightsImportModelNode* n = s->GetNthChild(k);
                                if (n->_node == node) {
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
    if (_dirty) {
        if (wxMessageBox("Are you sure you want to leave WITHOUT saving your mapping changes for future imports?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
            return;
        }
    }

    wxFileDialog dlg(this, "Load mapping", wxEmptyString, wxEmptyString, "Mapping Files (*.xjmap;*.xmap;*.xmaphint)|*.xjmap;*.xmap;*.xmaphint|All Files (*.)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        LoadMappingFile( dlg.GetPath());
    }
}

void xLightsImportChannelMapDialog::LoadMappingFile(wxString const& filepath, bool hideWarnings)
{
     static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _mappingFile = filepath;
    _dataModel->ClearMapping();
    _stashedMappings.clear();

    wxFileName fn(filepath);
    wxString const& ext { fn.GetExt().Lower() };

    if (ext == "xmaphint")
    {
        loadMapHintsFile(filepath);
    }
    else if (ext == "xjmap")
    {
        LoadJSONMapping(filepath, hideWarnings);
    }
    else// if (ext == "xmap")
    {
        LoadXMapMapping(filepath, hideWarnings);
    }
    //else
    //{
    //    logger_base.error("Invalid Mapping file type %s.", (const char*)ext.c_str());
    //    return;
    //}

    _dirty = false;

    logger_base.debug("Mapping %s loaded. %d mappings stashed.", (const char*)filepath.c_str(), _stashedMappings.size());

    // expand all models that have strands that have a value
    wxDataViewItemArray models;
    _dataModel->GetChildren(wxDataViewItem(0), models);
    for (size_t i = 0; i < models.size(); ++i) {
        wxDataViewItemArray strands;
        _dataModel->GetChildren(models[i], strands);
        for (size_t j = 0; j < strands.size(); ++j) {
            xLightsImportModelNode* astrand = (xLightsImportModelNode*)strands[j].GetID();
            if (astrand->HasMapping()) {
                TreeListCtrl_Mapping->Expand(models[i]);
            }
            wxDataViewItemArray nodes;
            _dataModel->GetChildren(strands[j], nodes);
            for (size_t k = 0; k < nodes.size(); ++k) {
                xLightsImportModelNode* anode = (xLightsImportModelNode*)nodes[k].GetID();
                if (!anode->_mapping.empty()) {
                    TreeListCtrl_Mapping->Expand(strands[j]);
                }
            }
        }
    }

    MarkUsed();
    TreeListCtrl_Mapping->Refresh();
}

void xLightsImportChannelMapDialog::LoadJSONMapping(wxString const& filename, bool hideWarnings)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool strandwarning{hideWarnings};
    bool modelwarning{hideWarnings};

    nlohmann::json data;

    try {
        std::ifstream inputFile(filename.ToStdString());
        inputFile >> data;

    } catch (std::exception& ex) {
        return DisplayError("Error reading JSON mapping file: " + filename + "\n" + ex.what());
    } 


    //zip package settings
    if (_xsqPkg != nullptr && _xsqPkg->IsPkg()) {
        if (data.contains("facesdir") && !data["facesdir"].get<std::string>().empty()) {
            _xsqPkg->GetImportOptions()->SetDir(MediaTargetDir::FACES_DIR, data["facesdir"].get<std::string>());
        }
        if (data.contains("gladiatordir") && !data["gladiatordir"].get<std::string>().empty()) {
            _xsqPkg->GetImportOptions()->SetDir(MediaTargetDir::GLEDIATORS_DIR, data["gladiatordir"].get<std::string>());
        }
        if (data.contains("imagedir") && !data["imagedir"].get<std::string>().empty()) {
            _xsqPkg->GetImportOptions()->SetDir(MediaTargetDir::IMAGES_DIR, data["imagedir"].get<std::string>());
        }
        if (data.contains("shaderdir") && !data["shaderdir"].get<std::string>().empty()) {
            _xsqPkg->GetImportOptions()->SetDir(MediaTargetDir::SHADERS_DIR, data["shaderdir"].get<std::string>());
        }
        if (data.contains("videodir") && !data["videodir"].get<std::string>().empty()) {
            _xsqPkg->GetImportOptions()->SetDir(MediaTargetDir::VIDEOS_DIR, data["videodir"].get<std::string>());
        }
        if (data.contains("importmedia")) {
            CheckBoxImportMedia->SetValue(data.at("importmedia").get<bool>());
        }
    }

    //settings
    if (data.contains("eraseexisting")) {
        CheckBox_EraseExistingEffects->SetValue(data.at("eraseexisting").get<bool>());
    }
    if (_allowCCRStrand && data.contains("mapccrstrand")) {
        CheckBox_MapCCRStrand->SetValue(data.at("mapccrstrand").get<bool>());
    }
    if (_allowImportBlend && data.contains("importblendmode")) {
        CheckBox_Import_Blend_Mode->SetValue(data.at("importblendmode").get<bool>());
    }

    //selected timmings
    auto timingtracks = data["timingtracks"].array();
    if (timingtracks != nullptr) {
        for (int i = 0; i < timingtracks.size(); ++i) {
            wxString const ttname = timingtracks.at(i).at("name").get<std::string>();
            bool const ttenabled = timingtracks.at(i).at("enabled").get<bool>();
            if (auto const& idx{ std::find(timingTracks.begin(), timingTracks.end(), ttname) }; idx != timingTracks.end()) {
                auto index = std::distance(timingTracks.begin(), idx);
                TimingTrackListBox->Check(index, ttenabled);
            }
        }
    }

    auto SetMapping = [&](wxString const& mapping, wxDataViewItem item, wxColor const& color) {
        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(mapping), item, 2);
        ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel())->SetMappingExists(item, ListCtrl_Available->FindItem(0, mapping) >= 0);
        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(color.GetAsString()), item, 4);
        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(findModelType(mapping)), item, 3);
    };

    //mappings
    auto mappings = data["mappings"].array();
    for (int i = 0; i < mappings.size(); ++i) {
        wxString const model = mappings.at(i).at("model").get<std::string>();
        wxString const strand = mappings.at(i).at("strand").get<std::string>();
        wxString const node = mappings.at(i).at("node").get<std::string>();
        wxString const mapping = mappings.at(i).at("mapping").get<std::string>();
        wxColor color = wxColor(mappings.at(i).at("color").get<std::string>());
        bool stashed = mappings.at(i).at("stashed").get<bool>();

        Element *modelEl = mSequenceElements->GetElement(model.ToStdString());

        if (!stashed && TreeContainsModel(model) == nullptr && !modelwarning) {
            if (wxMessageBox("Model \"" + model + "\" not part of sequence.  Not mapping channels to this model. Do you want to see future occurrences of this error during this import?", "", wxICON_WARNING | wxYES_NO, this) == wxNO)
            {
                modelwarning = true;
            }
        }

        // This code adds the model into the sequence ... it is useful if the model was previously only
        // in a model group that was in the sequence.
        if (modelEl == nullptr && xlights->GetModel(model.ToStdString()) != nullptr) {
            mSequenceElements->AddMissingModelsToSequence(model.ToStdString(), false);
            ModelElement *mel = dynamic_cast<ModelElement*>(mSequenceElements->GetElement(model.ToStdString()));
            if (mel != nullptr) {
                mel->Init(*xlights->GetModel(model.ToStdString()));
            } else {
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

            if (mi == nullptr || (msi == nullptr && !strand.empty()) || (mni == nullptr && !node.empty())) {
                if (mi != nullptr && !strandwarning) {
                    if (wxMessageBox(model + "/" + strand + "/" + node + " not found.  Has the models changed? Do you want to see future occurrences of this error during this import?", "", wxICON_WARNING | wxYES_NO, this) == wxNO)
                    {
                        strandwarning = true;
                    }
                }

                // save the unused mappings
                _stashedMappings.emplace_back(new StashedMapping(model, strand, node, mapping, color));
            } else {
                if (!mapping.empty()) {
                    if (mni != nullptr) {
                        wxDataViewItem item = FindItem(model.ToStdString(), strand.ToStdString(), node.ToStdString());
                        SetMapping(mapping, item, color);
                    } else if (msi != nullptr) {
                        wxDataViewItem item = FindItem(model.ToStdString(), strand.ToStdString());
                        SetMapping(mapping, item, color);
                    } else {
                        wxDataViewItem item = FindItem(model.ToStdString());
                        SetMapping(mapping, item, color);
                    }
                }
            }
        } else {
            // save this unused mapping
            _stashedMappings.emplace_back(new StashedMapping(model, strand, node, mapping, color));
        }
    }

}

void xLightsImportChannelMapDialog::LoadXMapMapping(wxString const& filename, bool hideWarnings)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool strandwarning{hideWarnings};
    bool modelwarning{hideWarnings};

    wxFileInputStream input(filename);
    wxTextInputStream text(input, "\t");
    wxString const firstLine = text.ReadLine(); // map by strand ... ignore this
    if (firstLine.Contains("{")) {
        LoadJSONMapping(filename, hideWarnings);
        return;
    }
    int count = wxAtoi(text.ReadLine());
    for (int x = 0; x < count; ++x) {
        std::string mn = text.ReadLine().ToStdString();
        if (TreeContainsModel(mn) == nullptr) {
            if (!modelwarning) {
                if (wxMessageBox("Model " + mn + " not part of sequence.  Not mapping channels to this model. Do you want to see future occurrences of this error during this import?", "", wxICON_WARNING | wxYES_NO, this) == wxNO)
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

        if (CountChar(line, '\t') == 4) {
            model = FindTab(line);
            strand = FindTab(line);
            node = FindTab(line);
            mapping = FindTab(line);
            color = wxColor(FindTab(line));
        } else {
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
            } else {
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

            if (mi == nullptr || (msi == nullptr && strand != "") || (mni == nullptr && node != "")) {
                if (mi != nullptr && !strandwarning) {
                    if (wxMessageBox(model + "/" + strand + "/" + node + " not found.  Has the models changed? Do you want to see future occurrences of this error during this import?", "", wxICON_WARNING | wxYES_NO, this) == wxNO)
                    {
                        strandwarning = true;
                    }
                }

                // save the unused mappings
                _stashedMappings.emplace_back(new StashedMapping(model, strand, node, mapping, color));
            } else {
                if (mapping != "") {
                    if (mni != nullptr) {
                        wxDataViewItem item = FindItem(model.ToStdString(), strand.ToStdString(), node.ToStdString());
                        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(mapping), item, 2);
                        ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel())->SetMappingExists(item, ListCtrl_Available->FindItem(0, mapping) >= 0);
                        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(color.GetAsString()), item, 4);
                        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant( findModelType(mapping)), item, 3);
                    } else if (msi != nullptr) {
                        wxDataViewItem item = FindItem(model.ToStdString(), strand.ToStdString());
                        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(mapping), item, 2);
                        ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel())->SetMappingExists(item, ListCtrl_Available->FindItem(0, mapping) >= 0);
                        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(color.GetAsString()), item, 4);
                        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(findModelType(mapping)), item, 3);
                    } else {
                        wxDataViewItem item = FindItem(model.ToStdString());
                        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(mapping), item, 2);
                        ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel())->SetMappingExists(item, ListCtrl_Available->FindItem(0, mapping) >= 0);
                        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(color.GetAsString()), item, 4);
                        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(findModelType(mapping)), item, 3);
                    }
                }
            }
        } else {
            // save this unused mapping
            _stashedMappings.emplace_back(new StashedMapping(model, strand, node, mapping, color));
        }
        line = text.ReadLine();
    }
}

void xLightsImportChannelMapDialog::SaveMapping(wxCommandEvent& event)
{
    wxFileDialog dlg(this, "Save mapping", wxEmptyString, _mappingFile, "Text Maping (*.xmap)|*.xmap|JSON Mapping Files (*.xjmap)|*.xjmap|xMapHint (*.xmaphint)|*.xmaphint", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        if (dlg.GetPath().Lower().EndsWith(".xmaphint")) {
            generateMapHintsFile(dlg.GetPath());
            return;
        }
        if (dlg.GetPath().Lower().EndsWith(".xmap")) {
            SaveXMapMapping(dlg.GetPath());
            return;
        }
        SaveJSONMapping(dlg.GetPath());
    }
}

void xLightsImportChannelMapDialog::SaveXMapMapping(wxString const& filename)
{
    wxFileOutputStream output(filename);
    wxTextOutputStream text(output);
    text.WriteString("false\n");
    int modelcount = _dataModel->GetMappedChildCount();
    text.WriteString(wxString::Format("%d\n", modelcount));
    for (size_t i = 0; i < _dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = _dataModel->GetNthChild(i);
        if (m->HasMapping()) {
            text.WriteString(m->_model + "\n");
        }
    }
    for (size_t i = 0; i < _dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = _dataModel->GetNthChild(i);
        if (m->HasMapping()) {
            wxString mn = m->_model;
            StashedMapping* sm = GetStashedMapping(mn, "", "");
            if (m->_mapping == "" && sm != nullptr) {
                text.WriteString(mn
                    + "\t" +
                    +"\t" +
                    +"\t" + sm->_mapping
                    + "\t" + sm->_color.GetAsString()
                    + "\n");
            } else {
                text.WriteString(mn
                    + "\t" +
                    +"\t" +
                    +"\t" + m->_mapping
                    + "\t" + m->_color.GetAsString()
                    + "\n");
            }
            for (size_t j = 0; j < m->GetChildCount(); ++j) {
                xLightsImportModelNode* s = m->GetNthChild(j);
                if (s->HasMapping() || AnyStashedMappingExists(mn, s->_strand)) {
                    sm = GetStashedMapping(mn, s->_strand, "");
                    if (s->_mapping == "" && sm != nullptr) {
                        text.WriteString(mn
                            + "\t" + sm->_strand
                            + "\t" +
                            +"\t" + sm->_mapping
                            + "\t" + sm->_color.GetAsString()
                            + "\n");
                    } else {
                        text.WriteString(mn
                            + "\t" + s->_strand
                            + "\t" +
                            +"\t" + s->_mapping
                            + "\t" + s->_color.GetAsString()
                            + "\n");
                    }
                    for (size_t k = 0; k < s->GetChildCount(); ++k) {
                        xLightsImportModelNode* n = s->GetNthChild(k);
                        sm = GetStashedMapping(mn, n->_strand, n->_node);
                        if (n->_mapping == "" && sm != nullptr) {
                            text.WriteString(mn
                                + "\t" + sm->_strand
                                + "\t" + sm->_node
                                + "\t" + sm->_mapping
                                + "\t" + sm->_color.GetAsString()
                                + "\n");
                        } else {
                            if (n->HasMapping()) {
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

    for (auto it = _stashedMappings.begin(); it != _stashedMappings.end(); ++it) {
        Element *modelEl = mSequenceElements->GetElement((*it)->_model.ToStdString());
        if (modelEl == nullptr) {
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

void xLightsImportChannelMapDialog::SaveJSONMapping(wxString const& filename)
{
    nlohmann::json data;

    //mappings
    nlohmann::json allMappings;
    for (size_t i = 0; i < _dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = _dataModel->GetNthChild(i);
        if (m->HasMapping()) {
            nlohmann::json mapping;

            mapping["model"] = m->_model;
            StashedMapping* sm = GetStashedMapping(m->_model, "", "");
            if (m->_mapping.empty() && sm != nullptr) {
                mapping["mapping"] = sm->_mapping.ToStdString();
                mapping["color"] = sm->_color.GetAsString().ToStdString();
                mapping["stashed"] = true;
            } else {
                mapping["mapping"] = m->_mapping;
                mapping["color"] = m->_color.GetAsString().ToStdString();
            }
            allMappings.push_back(mapping);
            for (size_t j = 0; j < m->GetChildCount(); ++j) {
                xLightsImportModelNode* s = m->GetNthChild(j);
                if (s->HasMapping() || AnyStashedMappingExists(m->_model, s->_strand)) {
                    sm = GetStashedMapping(m->_model, s->_strand, "");

                    nlohmann::json smapping;
                    smapping["model"] = m->_model;
                    if (s->_mapping.empty() && sm != nullptr) {
                        smapping["strand"] = sm->_strand.ToStdString();
                        smapping["mapping"] = sm->_mapping.ToStdString();
                        smapping["color"] = sm->_color.GetAsString().ToStdString();
                        smapping["stashed"] = true;
                    } else {
                        smapping["strand"] = s->_strand;
                        smapping["mapping"] = s->_mapping;
                        smapping["color"] = s->_color.GetAsString().ToStdString();
                    }
                    allMappings.push_back(smapping);

                    for (size_t k = 0; k < s->GetChildCount(); ++k) {
                        xLightsImportModelNode* n = s->GetNthChild(k);
                        sm = GetStashedMapping(m->_model, n->_strand, n->_node);
                        if (n->_mapping.empty() && sm != nullptr) {
                            nlohmann::json nmapping;
                            nmapping["model"] = m->_model;
                            nmapping["strand"] = sm->_strand.ToStdString();
                            nmapping["node"] = sm->_node.ToStdString();
                            nmapping["mapping"] = sm->_mapping.ToStdString();
                            nmapping["color"] = sm->_color.GetAsString().ToStdString();
                            nmapping["stashed"] = true;
                            allMappings.push_back(nmapping);
                        } else {
                            if (n->HasMapping()) {
                                nlohmann::json nmapping;
                                nmapping["model"] = m->_model;
                                nmapping["strand"] = n->_strand;
                                nmapping["node"] = n->_node;
                                nmapping["mapping"] = n->_mapping;
                                nmapping["color"] = n->_color.GetAsString().ToStdString();
                                allMappings.push_back(nmapping);
                            }
                        }
                    }
                }
            }
        }
    }

    //stashed mappings
    for (auto it = _stashedMappings.begin(); it != _stashedMappings.end(); ++it) {
        Element *modelEl = mSequenceElements->GetElement((*it)->_model.ToStdString());
        if (modelEl == nullptr) {
            nlohmann::json mapping;
            mapping["model"] = (*it)->_model.ToStdString();
            mapping["strand"] = (*it)->_strand.ToStdString();
            mapping["node"] = (*it)->_node.ToStdString();
            mapping["mapping"] = (*it)->_mapping.ToStdString();
            mapping["color"] = (*it)->_color.GetAsString().ToStdString();
            mapping["stashed"] = true;
            allMappings.push_back(mapping);
        }
    }

    data["mappings"] = allMappings;

    //selected timmings
    for (size_t tt = 0; tt < TimingTrackListBox->GetCount(); ++tt)
    {
        nlohmann::json timtrack;
        timtrack["name"] = timingTracks[tt];
        timtrack["enabled"] = TimingTrackListBox->IsChecked(tt);
        data["timingtracks"].push_back(timtrack);
    }
    //other settings
    data["mapccrstrand"] = CheckBox_MapCCRStrand->IsChecked();
    data["eraseexisting"] = CheckBox_EraseExistingEffects->IsChecked();
    data["importblendmode"] = CheckBox_Import_Blend_Mode->IsChecked();

    //package sequence folders
    if (_xsqPkg != nullptr && _xsqPkg->IsPkg())
    {
        data["facesdir"] = _xsqPkg->GetImportOptions()->GetDir(MediaTargetDir::FACES_DIR);
        data["gladiatordir"] = _xsqPkg->GetImportOptions()->GetDir(MediaTargetDir::GLEDIATORS_DIR);
        data["imagedir"] = _xsqPkg->GetImportOptions()->GetDir(MediaTargetDir::IMAGES_DIR);
        data["shaderdir"] =  _xsqPkg->GetImportOptions()->GetDir(MediaTargetDir::SHADERS_DIR);
        data["videodir"] = _xsqPkg->GetImportOptions()->GetDir(MediaTargetDir::VIDEOS_DIR);
        data["importmedia"] =  CheckBoxImportMedia->IsChecked();
    }

    try {
        std::ofstream o(filename.ToStdString());
        if (o.is_open()) {
            o << std::setw(4) << data << std::endl;
        }
    } catch (const std::exception&) {
    }
    _dirty = false;
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
    int top = TreeListCtrl_Mapping->GetItemRect(item).GetTop();
    int bottom = TreeListCtrl_Mapping->GetItemRect(item).GetBottom();
    int height = bottom - top;

    wxDataViewItemArray models;
    _dataModel->GetChildren(wxDataViewItem(0), models);
    for (size_t i = 0; i < models.size(); ++i) {
        int mtop = TreeListCtrl_Mapping->GetItemRect(models[i]).GetTop();
        if (mtop - height > top && mtop - height < bottom) {
            return models[i];
        }

        if (TreeListCtrl_Mapping->IsExpanded(models[i])) {
            wxDataViewItemArray strands;
            _dataModel->GetChildren(models[i], strands);
            for (size_t j = 0; j < strands.size(); ++j) {
                int stop = TreeListCtrl_Mapping->GetItemRect(strands[j]).GetTop();
                if (stop - height > top && stop - height < bottom) {
                    return strands[j];
                }

                if (TreeListCtrl_Mapping->IsExpanded(strands[j])) {
                    wxDataViewItemArray nodes;
                    _dataModel->GetChildren(strands[j], nodes);
                    for (size_t k = 0; k < nodes.size(); ++k) {
                        int ntop = TreeListCtrl_Mapping->GetItemRect(nodes[k]).GetTop();
                        if (ntop - height > top && ntop - height < bottom) {
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
    for (size_t i = 0; i < models.size(); ++i) {
        int mbottom = TreeListCtrl_Mapping->GetItemRect(models[i]).GetBottom();
        if (top == mbottom - 1) {
            return models[i];
        }
        if (TreeListCtrl_Mapping->IsExpanded(models[i])) {
            wxDataViewItemArray strands;
            _dataModel->GetChildren(models[i], strands);
            for (size_t j = 0; j < strands.size(); ++j) {
                int sbottom = TreeListCtrl_Mapping->GetItemRect(strands[j]).GetBottom();
                if (top == sbottom - 1) {
                    return strands[j];
                }

                if (TreeListCtrl_Mapping->IsExpanded(strands[j])) {
                    wxDataViewItemArray nodes;
                    _dataModel->GetChildren(strands[j], nodes);
                    for (size_t k = 0; k < nodes.size(); ++k) {
                        int nbottom = TreeListCtrl_Mapping->GetItemRect(nodes[k]).GetBottom();
                        if (top == nbottom - 1) {
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
    if (_dirty) {
        if (wxMessageBox("Are you sure you want to leave WITHOUT saving your mapping changes for future imports?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxYES) {
            EndDialog(wxID_OK);
        }
    } else {
        EndDialog(wxID_OK);
    }
}

void xLightsImportChannelMapDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    if (_dirty) {
        if (wxMessageBox("Are you sure you want to cancel WITHOUT saving your mapping changes for future imports?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxYES) {
            EndDialog(wxID_CANCEL);
        }

    } else {
        EndDialog(wxID_CANCEL);
    }
}

void xLightsImportChannelMapDialog::OnListCtrl_AvailableItemSelect(wxListEvent& event)
{
}

void xLightsImportChannelMapDialog::OnListCtrl_AvailableColumnClick(wxListEvent& event)
{
    if (event.m_col == 0) {
        if (_sortOrder == 0) {
            _sortOrder = 1;
            ListCtrl_Available->SortItems(MyCompareFunctionAscName, (wxIntPtr)CheckBox_MapCCRStrand->GetValue());
            ListCtrl_Available->ShowSortIndicator(0, true);
         } else {
            _sortOrder = 0;
            ListCtrl_Available->SortItems(MyCompareFunctionDescName, (wxIntPtr)CheckBox_MapCCRStrand->GetValue());
            ListCtrl_Available->ShowSortIndicator(0, false);
        }
    } else if (event.m_col == 1) {
        if (_sortOrder == 3) {
            _sortOrder = 4;
            ListCtrl_Available->SortItems(MyCompareFunctionAscName, (wxIntPtr)CheckBox_MapCCRStrand->GetValue()); // put it back in start order as otherwise this does not work
            ListCtrl_Available->SortItems(MyCompareFunctionAscEffects, (wxIntPtr)CheckBox_MapCCRStrand->GetValue());
            ListCtrl_Available->ShowSortIndicator(1, true);
        } else {
            _sortOrder = 3;
            ListCtrl_Available->SortItems(MyCompareFunctionAscName, (wxIntPtr)CheckBox_MapCCRStrand->GetValue()); // put it back in start order as otherwise this does not work
            ListCtrl_Available->SortItems(MyCompareFunctionDescEffects, (wxIntPtr)CheckBox_MapCCRStrand->GetValue());
            ListCtrl_Available->ShowSortIndicator(1, false);
        }
    }
}

#pragma region Drag and Drop

void xLightsImportChannelMapDialog::OnListCtrl_AvailableBeginDrag(wxListEvent& event)
{
    wxLogNull logNo; //kludge: Prevent wx logging

    if (_dataModel->GetChildCount() == 0) return;

    _dragItem = wxDataViewItem(nullptr);
    if (ListCtrl_Available->GetSelectedItemCount() == 0) return;

    int itemIndex = ListCtrl_Available->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    wxString drag = "Map," + ListCtrl_Available->GetItemText(itemIndex, 0);

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
        wxDataObjectComposite *comp = dynamic_cast<wxDataObjectComposite*>(event.GetDataObject());
        wxTextDataObject *obj = dynamic_cast<wxTextDataObject*>(event.GetDataObject());
        if (obj == nullptr && comp != nullptr) {
            obj = (wxTextDataObject*)comp->GetObject(wxDF_TEXT);
        }
        wxString txt = obj != nullptr ? obj->GetText() : "";
        txt.Trim(true).Trim(false);
        if (txt.length() < 4) {
            return;
        }
        // this looks strange, but drag drop on OSX sometimes will but a unicode BOM mark in txt[0]
        // and we need to strip that off
        if (txt[1] == 'M' && txt[2] == 'a' && txt[3] == 'p') {
            txt = txt.substr(1);
        }
        size_t idx =txt.find(',');
        if (idx != std::string::npos) {
            wxString pfx = txt.Left(idx);
            wxString model = txt.substr(idx + 1);
            if (pfx == "Map") {
                HandleDropAvailable(item, model, findModelType(model));
            }
        }
    }
}

void xLightsImportChannelMapDialog::OnBeginDrag(wxDataViewEvent& event)
{
    wxLogNull logNo; //kludge: Prevent wx logging

    if (event.GetItem().IsOk()) {
        _dragItem = event.GetItem();
        wxVariant vvalue;
        event.GetModel()->GetValue(vvalue, event.GetItem(), 2);
        std::string mapped = vvalue.GetString().ToStdString();

        if (mapped != "") {
            wxString drag = "Map," + mapped;

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

    if (wxGetUTCTimeMillis() - lastTime < scrollDelay) {
        // too soon to scroll again
    } else {
        wxDataViewItem item;
        wxDataViewColumn* col;
        _tree->HitTest(wxPoint(x, y), item, col);

        if (item.IsOk()) {
            // we are above an item
            _tree->SetCurrentItem(item);

            // get the first visible item
            wxRect itemRect = _tree->GetItemRect(item);

            if (y < itemRect.GetHeight()) {
                // scroll up
                lastTime = wxGetUTCTimeMillis();
                wxDataViewItem prev = ((xLightsImportChannelMapDialog*)_owner)->GetPriorTreeItem(item);

                if (prev.IsOk()) {
                    lastTime = wxGetUTCTimeMillis();
                    _tree->EnsureVisible(prev);
                    scrollDelay = scrollDelay / 2;
                    if (scrollDelay < MINSCROLLDELAY) scrollDelay = MINSCROLLDELAY;
                }
            } else if (y > _tree->GetRect().GetTop() - itemRect.GetHeight()) {
                // scroll down
                wxDataViewItem next = ((xLightsImportChannelMapDialog*)_owner)->GetNextTreeItem(item);
                if (next.IsOk()) {
                    lastTime = wxGetUTCTimeMillis();
                    _tree->EnsureVisible(next);
                    scrollDelay = scrollDelay / 2;
                    if (scrollDelay < MINSCROLLDELAY) scrollDelay = MINSCROLLDELAY;
                }
            } else {
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

    if (_list != nullptr) {
        event.SetInt(0);
    } else {
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
            if (_dragItem.IsOk()) {
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

            if (item.IsOk() && parms.size() == 2) {
                // if source was the tree then unmap the source
                if (_dragItem.IsOk()) {
                    Unmap(_dragItem);
                }

                std::string modelType = findModelType(parms[1].ToStdString());
                HandleDropAvailable(item, parms[1].ToStdString(), modelType);
            }
        }
        break;
        default:
            break;
    }

    TreeListCtrl_Mapping->Refresh();
    MarkUsed();
}

std::string xLightsImportChannelMapDialog::findModelType(std::string modelName)
{
    if (modelName.find('/') != std::string::npos) {
        return "Strand";
    }

    for (size_t j = 0; j < ListCtrl_Available->GetItemCount(); ++j) {
        if (ListCtrl_Available->GetItemText(j, 0).Lower().ToStdString() == ::Lower(modelName)) {
            wxListItem item;
            item.SetId(j);
            item.SetColumn(0);
            item.SetMask(wxLIST_MASK_IMAGE);

            if (ListCtrl_Available->GetItem(item)) {
                // Retrieve the icon index
                const int iconIndex = item.GetImage();

                if (iconIndex == LayoutUtils::Icon_Group) {
                    return "ModelGroup";
                } else {
                    return "Model";
                }
            }

            break;
        }
    }
    return "ModelGroup";
}

void xLightsImportChannelMapDialog::BulkMapNodes(const std::string& fromModel, wxDataViewItem& toModel)
{
    auto mm = ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel());
    wxDataViewItemArray strands;
    TreeListCtrl_Mapping->GetModel()->GetChildren(toModel, strands);
    for (auto& it : strands) {
        wxDataViewItemArray nodes;
        TreeListCtrl_Mapping->GetModel()->GetChildren(it, nodes);
        for (auto& it2 : nodes) {
            auto sn = mm->GetStrand(it);
            auto nn = mm->GetNode(it2);
            bool fromExist = false;
            auto fromname = fromModel + "/" + sn + "/" + nn;
            for (size_t j = 0; j < ListCtrl_Available->GetItemCount(); ++j) {
                if (ListCtrl_Available->GetItemText(j, 0) == fromname) {
                    fromExist = true;
                }
            }
            if (fromExist) {
                Map(it2, fromname, "Strand");
            }
        }
    }
}

void xLightsImportChannelMapDialog::BulkMapSubmodelsStrands(const std::string& fromModel, wxDataViewItem& toModel)
{
    // Find the model item in the mapping list
    auto mm = ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel());
    wxDataViewItemArray strands;
    TreeListCtrl_Mapping->GetModel()->GetChildren(toModel, strands);
    for (auto& it : strands) {
        auto sn = mm->GetStrand(it);
        bool fromExist = false;
        auto fromname = fromModel + "/" + sn;
        for (size_t j = 0; j < ListCtrl_Available->GetItemCount(); ++j) {
            if (ListCtrl_Available->GetItemText(j, 0) == fromname) {
                fromExist = true;
            }
        }
        if (fromExist) {
            Map(it, fromname, "Strand");
        }
    }
}

void xLightsImportChannelMapDialog::HandleDropAvailable(wxDataViewItem dropTarget, std::string availableModelName, std::string availableModelType) {

    if (dropTarget.IsOk()) {
        wxDataViewItemArray selectedItems;
        TreeListCtrl_Mapping->GetSelections(selectedItems);

        bool dropTargetIsSelected = false;

        for (const auto& selItem : selectedItems) {
            if (selItem == dropTarget) {
                dropTargetIsSelected = true;
                break;
            }
        }

        wxDataViewItem lastSelected;
        if (dropTargetIsSelected) {
            for (const auto& selItem : selectedItems) {
                Map(selItem, availableModelName, availableModelType);
            }
            lastSelected = selectedItems.Item(selectedItems.size() - 1);
        } else {
            Map(dropTarget, availableModelName, availableModelType);
            lastSelected = dropTarget;
        }

        // if we are mapping a submodel and the shift key is down
        if (CountChar(availableModelName, '/') == 1 && wxGetKeyState(WXK_SHIFT)) {
            auto mm = ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel());
            auto ss = mm->GetStrand(lastSelected);
            auto ms = wxSplit(availableModelName, '/');
            if (ms[1] == ss) {
                auto m = TreeListCtrl_Mapping->GetModel();
                auto par = m->GetParent(lastSelected);
                // strand names match ... so looks like we are ok for a batch mapping of submodels/strands
                BulkMapSubmodelsStrands(ms[0], par);
            }
        }
        // if we are mapping nodes and the shift key is down
        else if (CountChar(availableModelName, '/') == 2 && wxGetKeyState(WXK_SHIFT)) {
            auto mm = ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel());
            auto nn = mm->GetNode(lastSelected);
            auto ns = wxSplit(availableModelName, '/');
            if (ns[2] == nn) {
                auto m = TreeListCtrl_Mapping->GetModel();
                auto par = m->GetParent(m->GetParent(lastSelected));
                BulkMapNodes(ns[0], par);
            }
        }

        TreeListCtrl_Mapping->UnselectAll();

        if (lastSelected.IsOk()) {
            wxDataViewItem nextItem = GetNextTreeItem(lastSelected);
            if (nextItem.IsOk()) {
                TreeListCtrl_Mapping->Select(nextItem);
                TreeListCtrl_Mapping->EnsureVisible(nextItem);
            }
        }
        TreeListCtrl_Mapping->Refresh();
    }
}

#pragma endregion Drag and Drop

void xLightsImportChannelMapDialog::OnCheckBox_MapCCRStrandClick(wxCommandEvent& event)
{
    if (CheckBox_MapCCRStrand->IsChecked()) {
        SetCCROn();
    } else {
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
    for (unsigned int i = 0; i < _dataModel->GetChildCount(); ++i) {
        auto model = _dataModel->GetNthChild(i);
        if (!model->_mapping.empty()) {
            std::string lowerMapping = ::Lower(model->_mapping);
            if (std::find(used.begin(), used.end(), lowerMapping) == used.end()) {
                used.push_back(lowerMapping);
            }
        }

        for (unsigned int j = 0; j < model->GetChildCount(); ++j) {
            auto strand = model->GetNthChild(j);
            if (!strand->_mapping.empty()) {
                std::string lowerMapping = ::Lower(strand->_mapping);
                if (std::find(used.begin(), used.end(), lowerMapping) == used.end()) {
                    used.push_back(lowerMapping);
                }
            }

            for (unsigned int k = 0; k < strand->GetChildCount(); ++k) {
                auto node = strand->GetNthChild(k);
                if (!node->_mapping.empty()) {
                    std::string lowerMapping = ::Lower(node->_mapping);
                    if (std::find(used.begin(), used.end(), lowerMapping) == used.end()) {
                        used.push_back(lowerMapping);
                    }
                }
            }
        }
    }

    used.sort();

    int items = ListCtrl_Available->GetItemCount();
    ListCtrl_Available->Freeze();
    for (int i = 0; i < items; ++i) {
        if (!std::binary_search(used.begin(), used.end(), ListCtrl_Available->GetItemText(i).Lower().ToStdString())) {
            // not used
            ImportChannel* im = GetImportChannel(ListCtrl_Available->GetItemText(i).ToStdString());
            if (im != nullptr && im->type == "ModelGroup") {
                ListCtrl_Available->SetItemTextColour(i, ColorManager::instance()->CyanOrBlueOverride());
            } else {
                ListCtrl_Available->SetItemTextColour(i, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
            }
        } else {
            //used
            ListCtrl_Available->SetItemTextColour(i, ColorManager::instance()->LightOrMediumGreyOverride());
        }
    }
    ListCtrl_Available->Thaw();
    ListCtrl_Available->Refresh();
}

StashedMapping* xLightsImportChannelMapDialog::GetStashedMapping(wxString const& modelName, wxString const& strandName, wxString const& nodeName)
{
    for (auto it = _stashedMappings.begin(); it != _stashedMappings.end(); ++it) {
        if ((*it)->_model == modelName && (*it)->_strand == strandName && (*it)->_node == nodeName) {
            return it->get();
        }
    }

    return nullptr;
}

bool xLightsImportChannelMapDialog::AnyStashedMappingExists(wxString const& modelName, wxString const& strandName)
{
    for (auto it = _stashedMappings.begin(); it != _stashedMappings.end(); ++it) {
        if ((*it)->_model == modelName && (strandName == "" || (*it)->_strand == strandName)) {
            if ((*it)->_mapping != "") {
                return true;
            }
        }
    }

    return false;
}

wxString xLightsImportChannelMapDialog::AggressiveAutomap(const wxString& name) {
    wxString s = name;

    s.Replace(" ", "");
    s.Replace("-", "");
    s.Replace("_", "");
    s.Replace("(", "");
    s.Replace(")", "");
    s.Replace(":", "");
    s.Replace(";", "");
    s.Replace("\\", "");
    s.Replace("|", "");
    s.Replace("{", "");
    s.Replace("}", "");
    s.Replace("[", "");
    s.Replace("]", "");
    s.Replace("+", "");
    s.Replace("=", "");
    s.Replace("*", "");
    s.Replace("^", "");
    s.Replace("#", "");
    s.Replace(",", "");
    s.Replace(".", "");

    return s;
}

std::string xLightsImportChannelMapDialog::GetAIPrompt(const std::string& promptFile) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string showFolderPromptFile = xlights->GetShowDirectory() + "/" + promptFile;
    std::string xlFolder = GetResourcesDirectory();
    std::string xLightsFolderPromptFile = xlFolder + "/prompts/" + promptFile;

    std::string fileToLoad;
    if (wxFileExists(showFolderPromptFile)) {
        logger_base.debug("Using prompt file from show folder: %s", showFolderPromptFile.c_str());
        fileToLoad = showFolderPromptFile;
    } else if (wxFileExists(xLightsFolderPromptFile)) {
        logger_base.debug("Using prompt file from xLights folder: %s", xLightsFolderPromptFile.c_str());
        fileToLoad = xLightsFolderPromptFile;
    } else {
        // This looks for a prompt without the aiEngine prefix
        std::string pf = AfterFirst(promptFile, '_');

        showFolderPromptFile = xlights->GetShowDirectory() + "/" + pf;
        xLightsFolderPromptFile = xlFolder + "/prompts/" + pf;

        if (wxFileExists(showFolderPromptFile)) {
            logger_base.debug("Using prompt file from show folder: %s", showFolderPromptFile.c_str());
            fileToLoad = showFolderPromptFile;
        } else if (wxFileExists(xLightsFolderPromptFile)) {
            logger_base.debug("Using prompt file from xLights folder: %s", xLightsFolderPromptFile.c_str());
            fileToLoad = xLightsFolderPromptFile;
        } else {
            logger_base.error("Prompt file not found: %s", promptFile.c_str());
            wxMessageBox("The prompt file could not be found " + promptFile, "Error", wxICON_ERROR | wxOK, this);
            return "";
        }
    }

    // read the prompt from the ./prompts/AIAutoMap.txt file relative to where this executable is
    std::string instructions;
    std::ifstream file(fileToLoad);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line != "" && line[0] == '#') {
                // we skip over lines with a # as the first character so we can add comments into prompt files
            } else {
                instructions += line + "\\n";
            }
        }
        file.close();
    }

    if (instructions == "") {
        wxMessageBox("The prompt file contained no prompt. " + fileToLoad, "Error", wxICON_ERROR | wxOK, this);
    }

    return instructions;
}

std::string xLightsImportChannelMapDialog::BuildSourceModelPrompt(const std::list<ImportChannel*>& sourceModels, std::function<bool(const ImportChannel*)> filter) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string sourceDescription = "<sourceModels>\\n";
    for (const auto& it : sourceModels) {
        if (filter(it)) {
            std::string type = it->type;
            if (type == "") {
                if (it->isNode)
                    type = "pixel";
                else
                    type = "strand";
            }

            std::string groupModels = "";
            if (type == "ModelGroup") {
                groupModels = " groupModels='" + it->groupModels + "'";
            }

            sourceDescription += "  <model name='" + it->name + "' type='" + type + "'" + groupModels + " class='" + it->modelClass + "' effectCount='" + std::to_string(it->effectCount) + "'/>\\n";
        }
    }
    sourceDescription += "</sourceModels>";
    logger_base.debug("Source models: %s", sourceDescription.c_str());

    return sourceDescription;
}

std::string xLightsImportChannelMapDialog::BuildTargetModelPrompt(const std::list<xLightsImportModelNode*>& targetModels, std::function<bool(const xLightsImportModelNode*)> filter) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string targetDescription = "<targetModels>\\n";
    for (const auto& it : targetModels) {
        if (filter(it)) {
            std::string name = it->GetModelName();
            std::string type = it->_modelType;
            if (it->_strand != "") {
                if (it->_isSubmodel) {
                    type = "SubModel";
                } else {
                    type = "strand";
                }
            }
            if (it->_node != "") {
                type = "pixel";
            }
            std::string groupModels = "";
            if (it->_groupModels != "") {
                groupModels = " groupModels='" + it->_groupModels + "'";
            }
            targetDescription += "  <model name='" + name + "' type='" + type + "' class='" + it->_modelClass + "'" + groupModels + "/>\\n";
        }
    }
    targetDescription += "</targetModels>";
    logger_base.debug("Target models: %s", targetDescription.c_str());

    return targetDescription;
}

std::string xLightsImportChannelMapDialog::BuildAlreadyMappedPrompt(const std::list<xLightsImportModelNode*>& targetModels, std::function<bool(const xLightsImportModelNode*)> filter) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string exampleMappings = "<exampleMappings>\\n";
    for (const auto& it : targetModels) {
        if (filter(it)) {
            std::string name = it->GetModelName();
            std::string type = it->_modelType;
            if (it->IsStrand())
                type = "strand";
            else if (it->IsNode())
                type = "pixel";
            else if (it->IsSubModel())
                type = "SubModel";
            exampleMappings += "  <model name='" + name + "' mappedTo='" + it->_mapping + "'/>\\n";
        }
    }
    exampleMappings += "</exampleMappings>";
    logger_base.debug("Example mappings: %s", exampleMappings.c_str());

    return exampleMappings;
}

bool xLightsImportChannelMapDialog::RunAIPrompt(wxProgressDialog* dlg, const std::string& prompt, const std::list<ImportChannel*>& sourceModels, const std::list<xLightsImportModelNode*>& targetModels) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Prompt: %s", prompt.c_str());

    auto ai = xlights->GetAIService();
    if (ai == nullptr)
        return false;

   auto const[ response, worked] = ai->CallLLM(prompt);
    if (!worked) {
        return false;
    }

    std::string possibleSources = "";
    for (const auto& it : sourceModels) {
        possibleSources += it->name + ", ";
    }

    logger_base.debug("Response: %s", response.c_str());

    bool mapped = false;

    try {
        nlohmann::json root = nlohmann::json::parse(response);

        logger_base.debug("Parsed response");

        nlohmann::json mappings = root["mappings"];
        if (mappings.is_null()) {
            logger_base.error("No mappings found in response");
        } else {
            // now go through all the targets
            for (const auto& it : targetModels) {
                if (!it->HasMapping()) {
                    auto mn = it->GetModelName();
                    // find the model in the mappings sourceModel
                    for (size_t i = 0; i < mappings.size(); ++i) {
                        std::string targetModel = mappings[i]["targetModel"].get<std::string>();
                        std::string mappingSource = mappings[i]["sourceModel"].get<std::string>();
                        if (targetModel == mn && possibleSources.find(mappingSource) != std::string::npos) {
                            it->Map(mappingSource,"Unknown");
                            mapped = true;
                            break;
                        }
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        logger_base.error("Error parsing response: %s", e.what());
    }

    return mapped;
}

bool xLightsImportChannelMapDialog::AIModelMap(wxProgressDialog* dlg, const std::list<ImportChannel*>& sourceModels, const std::list<xLightsImportModelNode*>& targetModels) {
    // we only model map if there are models in target
    if (targetModels.size() == 0)
        return false;

    auto llm = xlights->GetAIService();
    if (llm == nullptr)
        return false;
    std::string prompt = GetAIPrompt(llm->GetLLMName() + "_AI_Model_AutoMap.txt");

    // exclude pixels and strands
    std::string sourceModelsPrompt = BuildSourceModelPrompt(sourceModels, [](const ImportChannel* m) { return !m->IsNode() && !m->IsStrand(); });
    // exlude already mapped models and Submodels, strands and pixels
    std::string targetModelsPrompt = BuildTargetModelPrompt(targetModels, [](const xLightsImportModelNode* m) { return !m->IsMapped() && !m->IsSubModel() && !m->IsStrand() && !m->IsNode(); });
    // include already mapped models and exclude Submodels, strands and pixels
    std::string altreadyMappedPrompt = BuildAlreadyMappedPrompt(targetModels, [](const xLightsImportModelNode* m) { return m->IsMapped() && !m->IsSubModel() && !m->IsStrand() && m->IsNode(); });

    if (prompt.find("{sourcemodels}") != std::string::npos)
        prompt = prompt.replace(prompt.find("{sourcemodels}"), 14, sourceModelsPrompt);
    if (prompt.find("{targetmodels}") != std::string::npos)
        prompt = prompt.replace(prompt.find("{targetmodels}"), 14, targetModelsPrompt);
    if (prompt.find("{examplemapping}") != std::string::npos)
        prompt = prompt.replace(prompt.find("{examplemapping}"), 16, altreadyMappedPrompt);

    bool res = RunAIPrompt(dlg, prompt, sourceModels, targetModels);
    dlg->Update(25, "Models mapped");

    return res;
}

bool xLightsImportChannelMapDialog::AISubModelMap(wxProgressDialog* dlg, const std::list<ImportChannel*>& sourceModels, const std::list<xLightsImportModelNode*>& targetModels) {
    // we only submodel map if there are submodels in target
    int submodelCount = 0;
    for (const auto& it : targetModels) {
        if (it->IsSubModel()) {
            ++submodelCount;
            break;
        }
    }
    if (submodelCount == 0)
        return false;

    auto llm = xlights->GetAIService();
    if (llm == nullptr)
        return false;
    std::string prompt = GetAIPrompt(llm->GetLLMName() + "_AI_SubModel_AutoMap.txt");

    // exclude pixels and strands
    std::string sourceModelsPrompt = BuildSourceModelPrompt(sourceModels, [](const ImportChannel* m) { return !m->IsNode() && !m->IsStrand(); });
    // exlude already mapped submodels and only include submodels
    std::string targetModelsPrompt = BuildTargetModelPrompt(targetModels, [](const xLightsImportModelNode* m) { return !m->_mappingExists && m->IsSubModel(); });
    // include already mapped sub models only
    std::string altreadyMappedPrompt = BuildAlreadyMappedPrompt(targetModels, [](const xLightsImportModelNode* m) { return m->_mappingExists && m->IsSubModel(); });

    if (prompt.find("{sourcemodels}") != std::string::npos)
        prompt = prompt.replace(prompt.find("{sourcemodels}"), 14, sourceModelsPrompt);
    if (prompt.find("{targetmodels}") != std::string::npos)
        prompt = prompt.replace(prompt.find("{targetmodels}"), 14, targetModelsPrompt);
    if (prompt.find("{examplemapping}") != std::string::npos)
        prompt = prompt.replace(prompt.find("{examplemapping}"), 16, altreadyMappedPrompt);

    bool res = RunAIPrompt(dlg, prompt, sourceModels, targetModels);
    dlg->Update(50, "SubModels mapped");

    return res;
}

bool xLightsImportChannelMapDialog::AIStrandMap(wxProgressDialog* dlg, const std::list<ImportChannel*>& sourceModels, const std::list<xLightsImportModelNode*>& targetModels) {
    // we only strand map if there source models which are strands
    int strandCount = 0;
    for (const auto& it : sourceModels) {
        if (it->IsStrand()) {
            ++strandCount;
            break;
        }
    }
    if (strandCount == 0)
        return false;

    auto llm = xlights->GetAIService();
    if (llm == nullptr)
		return false;
    std::string prompt = GetAIPrompt(llm->GetLLMName() + "_AI_Strand_AutoMap.txt");

    // only include strands
    std::string sourceModelsPrompt = BuildSourceModelPrompt(sourceModels, [](const ImportChannel* m) { return m->IsStrand(); });
    // only include strands
    std::string targetModelsPrompt = BuildTargetModelPrompt(targetModels, [](const xLightsImportModelNode* m) { return !m->IsMapped() && m->IsStrand(); });
    // include already mapped strands only
    std::string altreadyMappedPrompt = BuildAlreadyMappedPrompt(targetModels, [](const xLightsImportModelNode* m) { return m->IsMapped() && m->IsStrand(); });

    if (prompt.find("{sourcemodels}") != std::string::npos)
        prompt = prompt.replace(prompt.find("{sourcemodels}"), 14, sourceModelsPrompt);
    if (prompt.find("{targetmodels}") != std::string::npos)
        prompt = prompt.replace(prompt.find("{targetmodels}"), 14, targetModelsPrompt);
    if (prompt.find("{examplemapping}") != std::string::npos)
        prompt = prompt.replace(prompt.find("{examplemapping}"), 16, altreadyMappedPrompt);

    bool res = RunAIPrompt(dlg, prompt, sourceModels, targetModels);
    dlg->Update(75, "Strands mapped");

    return res;
}

#define AI_NODE_COUNT_LIMIT 16

bool xLightsImportChannelMapDialog::AINodeMap(wxProgressDialog* dlg, const std::list<ImportChannel*>& sourceModels, const std::list<xLightsImportModelNode*>& targetModels) {
    // we only node map if there are > 0 models in target with < 16 nodes and there is some node level sequencing
    int nodeModelCount = 0;
    for (const auto& it : sourceModels) {
        if (it->IsNode()) {
            ++nodeModelCount;
            break;
        }
    }
    if (nodeModelCount == 0) {
        for (const auto& it : targetModels) {
            if (it->_nodeCount < AI_NODE_COUNT_LIMIT) {
                ++nodeModelCount;
                break;
            }
        }
    }
    if (nodeModelCount == 0)
        return false;

    auto llm = xlights->GetAIService();
    if (llm == nullptr)
        return false;
    std::string prompt = GetAIPrompt(llm->GetLLMName() + "_AI_Node_AutoMap.txt");

    // include all node level sequencing
    std::string sourceModelsPrompt = BuildSourceModelPrompt(sourceModels, [](const ImportChannel* m) { return m->IsNode(); });
    // only include nodes on models with less than AI_NODE_COUNT_LIMIT nodes
    std::string targetModelsPrompt = BuildTargetModelPrompt(targetModels, [](const xLightsImportModelNode* m) { return !m->IsMapped() && m->IsNode() && m->_nodeCount < AI_NODE_COUNT_LIMIT; });
    // include already mapped nodes
    std::string altreadyMappedPrompt = BuildAlreadyMappedPrompt(targetModels, [](const xLightsImportModelNode* m) { return m->IsMapped() && m->IsNode(); });

    if (prompt.find("{sourcemodels}") != std::string::npos)
        prompt = prompt.replace(prompt.find("{sourcemodels}"), 14, sourceModelsPrompt);
    if (prompt.find("{targetmodels}") != std::string::npos)
        prompt = prompt.replace(prompt.find("{targetmodels}"), 14, targetModelsPrompt);
    if (prompt.find("{examplemapping}") != std::string::npos)
        prompt = prompt.replace(prompt.find("{examplemapping}"), 16, altreadyMappedPrompt);

    bool res = RunAIPrompt(dlg, prompt, sourceModels, targetModels);
    dlg->Update(100, "Nodes mapped");

    return res;
}

void xLightsImportChannelMapDialog::DoAIAutoMap(bool select) {
    // Ideas for future improvement

    // - We could try to represent the location of the models against a normalised coordinate system to encourage left/right etc to map better
    // - We could build some sort of asset with special hints for really common custom models
    // - We could include aliases in the information as further hints
    // - We could drop the models list in model groups to save space as I dont think it helps that much
    // - we could include strand and node names where they exist (although that may already be there ... i have not checked)

    // I welcome other developers experimenting with the prompt and trying to improve it.

    bool selectMapAvail = (ListCtrl_Available->GetSelectedItemCount() != 0) && select;
    bool selectMapTarget = (TreeListCtrl_Mapping->GetSelectedItemsCount() != 0) && select;

    // build a list of possible sources .. this is the selected items in the list or all items
    bool sourceContainsNodes = false;
    std::list<ImportChannel*> sourceModels;
    for (int j = 0; j < ListCtrl_Available->GetItemCount(); ++j) {
        ImportChannel* m = (ImportChannel*)ListCtrl_Available->GetItemData(j);
        if (selectMapAvail) {
            bool isSourceSelected = ListCtrl_Available->GetItemState(j, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED;
            if (isSourceSelected) {
                sourceModels.push_back(m);
                sourceContainsNodes |= m->isNode;
            }
        } else {
            sourceModels.push_back(m);
            sourceContainsNodes |= m->isNode;
        }
    }

    std::list<xLightsImportModelNode*> targetModels;

    // build a list of possible targets .. this is the selected items in the tree or all items but only if they are not already mapped
    wxDataViewItemArray targetSelectedItems;
    TreeListCtrl_Mapping->GetSelections(targetSelectedItems);
    for (unsigned int i = 0; i < _dataModel->GetChildCount(); ++i) {
        auto model = _dataModel->GetNthChild(i);
        if (model != nullptr) {
            if (selectMapTarget) {
                auto index = (wxDataViewItem)model;
                for (const wxDataViewItem& selectedItem : targetSelectedItems) {
                    if (index.GetID() == selectedItem.GetID()) {
                        targetModels.push_back(model);
                        for (unsigned int k = 0; k < model->GetChildCount(); ++k) {
                            auto strand = model->GetNthChild(k);
                            if (strand != nullptr) {
                                targetModels.push_back(strand);
                                // we only add in nodes if the source sequence contains node level effects
                                if (sourceContainsNodes) {
                                    for (unsigned int m = 0; m < strand->GetChildCount(); ++m) {
                                        auto node = strand->GetNthChild(m);
                                        if (node != nullptr) {
                                            targetModels.push_back(node);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                targetModels.push_back(model);
                for (unsigned int k = 0; k < model->GetChildCount(); ++k) {
                    auto strand = model->GetNthChild(k);
                    if (strand != nullptr) {
                        targetModels.push_back(strand);
                        // we only add in nodes if the source sequence contains node level effects
                        if (sourceContainsNodes) {
                            for (unsigned int m = 0; m < strand->GetChildCount(); ++m) {
                                auto node = strand->GetNthChild(m);
                                if (node != nullptr) {
                                    targetModels.push_back(node);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    wxProgressDialog* dlg = new wxProgressDialog("Generating mapping", "Please give me some time to map your models. This can take a minute or two.", 100, this, wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_AUTO_HIDE);
    dlg->Show();

    bool mapped = AIModelMap(dlg, sourceModels, targetModels);
    TreeListCtrl_Mapping->Refresh();
    mapped = AISubModelMap(dlg, sourceModels, targetModels) || mapped;
    TreeListCtrl_Mapping->Refresh();
    mapped = AIStrandMap(dlg, sourceModels, targetModels) || mapped;
    TreeListCtrl_Mapping->Refresh();
    mapped = AINodeMap(dlg, sourceModels, targetModels) || mapped;

    delete dlg;

    if (!mapped) {
        wxMessageBox("Unable to generate mappings. Check log file for details.", "Mapping Failed", 5);
    }
}

void xLightsImportChannelMapDialog::DoAutoMap(
    std::function<bool(const std::string&, const std::string&, const std::string&, const std::string&, const std::list<std::string>&)> lambda_model,
    std::function<bool(const std::string&, const std::string&, const std::string&, const std::string&, const std::list<std::string>&)> lambda_strand,
    std::function<bool(const std::string&, const std::string&, const std::string&, const std::string&, const std::list<std::string>&)> lambda_node,
    const std::string& extra1, const std::string& extra2, const std::string& mg, const bool& select)
{
    bool selectMapAvail = (ListCtrl_Available->GetSelectedItemCount() != 0) && select;
    bool selectMapTarget = (TreeListCtrl_Mapping->GetSelectedItemsCount() != 0) && select;
    bool modelMapped = 0;
    wxDataViewItemArray targetSelectedItems;
    TreeListCtrl_Mapping->GetSelections(targetSelectedItems);
    for (unsigned int i = 0; i < _dataModel->GetChildCount(); ++i) {
        auto model = _dataModel->GetNthChild(i);
        if (model != nullptr) {
            bool isTargetSelected = false;
            auto index = (wxDataViewItem)model;
            for (const wxDataViewItem& selectedItem : targetSelectedItems) {
                isTargetSelected = (index.GetID() == selectedItem.GetID() ? true : false);
                if (isTargetSelected) break;
            }

            if ((selectMapTarget && isTargetSelected) || !selectMapTarget) { // If LS has selection or no selections required
                auto aliases = model->GetAliases();
                if ((model->IsGroup() && (mg == "B" || mg == "G")) || (!model->IsGroup() && (mg == "B" || mg == "M"))) {
                    for (int j = 0; j < ListCtrl_Available->GetItemCount(); ++j) {
                        bool isSourceSelected = ListCtrl_Available->GetItemState(j, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED;
                        if ((selectMapAvail && isSourceSelected) || !selectMapAvail) {
                            wxString const availName = ListCtrl_Available->GetItemText(j).Trim(true).Trim(false).Lower();
                            if (availName.Contains("/")) {
                                wxArrayString const parts = wxSplit(availName, '/');
                                if (lambda_model(model->_model, parts[0], extra1, extra2, aliases)) {
                                    // matched the model name ... need to look at strands and submodels
                                    for (unsigned int k = 0; k < model->GetChildCount(); ++k) {
                                        auto strand = model->GetNthChild(k);
                                        if (strand != nullptr) {
                                            if (lambda_strand(strand->_strand, parts[1], extra1, extra2, aliases)) {
                                                // matched to the strand level
                                                if (parts.size() == 2) {
                                                    if (strand->_mapping.empty()) {
                                                        strand->Map(ListCtrl_Available->GetItemText(j), "Strand");
                                                        modelMapped = 1;
                                                    }
                                                } else {
                                                    // need to map the node level
                                                    for (unsigned int m = 0; m < strand->GetChildCount(); ++m) {
                                                        auto node = strand->GetNthChild(m);
                                                        if (node != nullptr) {
                                                            if (node->_mapping.empty()) {
                                                                if (lambda_node(node->_node, parts[2], extra1, extra2, aliases)) {
                                                                    // matched to the node level
                                                                    if (parts.size() == 3) {
                                                                        node->Map(ListCtrl_Available->GetItemText(j), "Node");
                                                                    } else {
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
                            } else { // match model to model
                                if (model->_mapping.empty() && lambda_model(model->_model, availName, extra1, extra2, aliases)) {
                                    model->Map(ListCtrl_Available->GetItemText(j), findModelType(availName));
                                }
                            }
                        }
                    }
                    if (model->_mapping.empty()) {
                        for (int j = 0; j < ListCtrl_Available->GetItemCount(); ++j) {
                            bool isSourceSelected = ListCtrl_Available->GetItemState(j, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED;
                            if ((selectMapAvail && isSourceSelected) || !selectMapAvail) {
                                std::string const availName = ListCtrl_Available->GetItemText(j).Trim(true).Trim(false).Lower().ToStdString();
                                auto m = xlights->GetModel(model->_model);
                                for (unsigned int k = 0; k < model->GetChildCount(); ++k) {
                                    auto sm = model->GetNthChild(k);
                                    if (sm != nullptr) {
                                        auto sm2 = m->GetSubModel(sm->_strand);
                                        if (sm2 != nullptr) {
                                            auto& smAliases = sm2->GetAliases();
                                            if (!smAliases.empty()) {
                                                if (lambda_model(sm->_strand, availName, extra1, extra2, smAliases)) {
                                                    sm->Map(ListCtrl_Available->GetItemText(j), "Unknown");
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
        } else {
            static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.warn("xLightsImportTreeModel::OnButton_AutoMapClick: Weird ... model %d was nullptr", i);
        }
    }
    // Process selected submodels independently
    if (selectMapTarget) {
        for (unsigned int i = 0; i < _dataModel->GetChildCount(); ++i) {
            auto model = _dataModel->GetNthChild(i);
            if (model != nullptr) {
                for (unsigned int k = 0; k < model->GetChildCount(); ++k) {
                    auto submodel = model->GetNthChild(k);
                    if (submodel != nullptr) {
                        bool isSubmodelSelected = false;
                        auto index = (wxDataViewItem)submodel;
                        for (const wxDataViewItem& selectedItem : targetSelectedItems) {
                            isSubmodelSelected = (index.GetID() == selectedItem.GetID() ? true : false);
                            if (isSubmodelSelected)
                                break;
                        }

                        if ((selectMapTarget && isSubmodelSelected) || !selectMapTarget) {
                            for (int j = 0; j < ListCtrl_Available->GetItemCount(); ++j) {
                                bool isSourceSelected = ListCtrl_Available->GetItemState(j, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED;
                                if ((selectMapAvail && isSourceSelected) || !selectMapAvail) {
                                    std::string const availName = ListCtrl_Available->GetItemText(j).Trim(true).Trim(false).Lower().ToStdString();
                                    auto m = xlights->GetModel(model->_model);
                                    auto& mAliases = m->GetAliases();
                                    if (m != nullptr) {
                                        auto sm = m->GetSubModel(submodel->_strand);
                                        if (sm != nullptr) {
                                            auto& smAliases = sm->GetAliases();
                                            if (submodel->_mapping.empty()) {
                                                if (lambda_strand(submodel->GetModelName(), availName, extra1, extra2, smAliases)) {
                                                    submodel->Map(ListCtrl_Available->GetItemText(j), "SubModel");
                                                } else {
                                                    for (const auto& modelAlias : mAliases) { // check for any aliases on model itself
                                                        if (lambda_strand(modelAlias + "/" + submodel->_strand, availName, extra1, extra2, smAliases)) {
                                                            submodel->Map(ListCtrl_Available->GetItemText(j), "SubModel");
                                                            break;
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
        }
    }
}

void xLightsImportChannelMapDialog::OnButton_AutoMapClick(wxCommandEvent& event)
{
    if (_dataModel == nullptr) return;

    _dirty = true;
    TreeListCtrl_Mapping->Freeze();
    DoAutoMap(norm, norm, norm, "", "", "B", false);
    DoAutoMap(aggressive, aggressive, aggressive, "", "", "B", false);

    auto maphints = xlights->CurrentDir + wxFileName::GetPathSeparator() + "maphints";
    if (wxDir::Exists(maphints)) {
        wxArrayString files;
        GetAllFilesInDir(maphints, files, "*.xmaphint");
        for (auto &filename : files) {
            if (FileExists(filename)) {
                loadMapHintsFile(filename);
            }
        }
    }
    TreeListCtrl_Mapping->Thaw();
    MarkUsed();
}

void xLightsImportChannelMapDialog::OnButton_AutoMapSelClick(wxCommandEvent& event) {
    if (_dataModel == nullptr)
        return;

    _dirty = true;
    DoAutoMap(norm, norm, norm, "", "", "B", true);
    DoAutoMap(aggressive, aggressive, aggressive, "", "", "B", true);

    auto maphints = xlights->CurrentDir + wxFileName::GetPathSeparator() + "maphints";
    if (wxDir::Exists(maphints)) {
        wxArrayString files;
        GetAllFilesInDir(maphints, files, "*.xmaphint");
        for (auto& filename : files) {
            if (FileExists(filename)) {
                loadMapHintsFile(filename);
            }
        }
    }

    TreeListCtrl_Mapping->Refresh();
    MarkUsed();
}

void xLightsImportChannelMapDialog::OnListCtrl_AvailableItemActivated(wxListEvent& event)
{
    if (TreeListCtrl_Mapping->GetSelectedItemsCount() == 0) return;

    wxDataViewItemArray mapItems;
    TreeListCtrl_Mapping->GetSelections(mapItems);

    wxDataViewItem lastMapTo;

    for (const auto& mapTo : mapItems) {
        lastMapTo = mapTo;
        std::string modelType = findModelType(ListCtrl_Available->GetItemText(event.GetItem()).ToStdString());
        Map(mapTo, ListCtrl_Available->GetItemText(event.GetItem()).ToStdString(), modelType);
    }

    wxDataViewItem nextMapTo = GetNextTreeItem(lastMapTo);
    TreeListCtrl_Mapping->UnselectAll();

    if (nextMapTo.IsOk()) {
        TreeListCtrl_Mapping->Select(nextMapTo);
        TreeListCtrl_Mapping->EnsureVisible(nextMapTo);
    }

    if (event.GetIndex() + 1 < ListCtrl_Available->GetItemCount()) {
        ListCtrl_Available->SetItemState(event.GetIndex(), 0, wxLIST_STATE_SELECTED);
        ListCtrl_Available->SetItemState(event.GetIndex() + 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        ListCtrl_Available->EnsureVisible(event.GetIndex() + 1);
    }
}

void xLightsImportChannelMapDialog::SetImportMediaTooltip() {

    if (_xsqPkg == nullptr) {
        return;
    }

    wxString toolTip = "Available media will be imported into:\n";

    // add configured paths
    wxString paths = wxString::Format("\n  - Faces: %s", _xsqPkg->GetImportOptions()->GetDir(MediaTargetDir::FACES_DIR));
    paths = wxString::Format("%s\n  - Glediators: %s", paths, _xsqPkg->GetImportOptions()->GetDir(MediaTargetDir::GLEDIATORS_DIR));
    paths = wxString::Format("%s\n  - Images: %s", paths, _xsqPkg->GetImportOptions()->GetDir(MediaTargetDir::IMAGES_DIR));
    paths = wxString::Format("%s\n  - Shaders: %s", paths, _xsqPkg->GetImportOptions()->GetDir(MediaTargetDir::SHADERS_DIR));
    paths = wxString::Format("%s\n  - Videos: %s", paths, _xsqPkg->GetImportOptions()->GetDir(MediaTargetDir::VIDEOS_DIR));

    toolTip = toolTip + paths;

    CheckBoxImportMedia->SetToolTip(toolTip);
    ButtonImportOptions->SetToolTip(toolTip);
}

void xLightsImportChannelMapDialog::OnButtonImportOptionsClick(wxCommandEvent& event)
{
    MediaImportOptionsDialog dlg(this, _xsqPkg->GetImportOptions());

    if (dlg.ShowModal() == wxID_OK) {
        SetImportMediaTooltip();
    }
}

void xLightsImportChannelMapDialog::OnCheckBoxImportMediaClick(wxCommandEvent& event)
{
    _xsqPkg->GetImportOptions()->SetImportActive(CheckBoxImportMedia->IsChecked());
    if (CheckBoxImportMedia->IsChecked()) {
        ButtonImportOptions->Enable();
    } else {
        ButtonImportOptions->Disable();
    }
}

void xLightsImportChannelMapDialog::LoadRgbEffectsFile() {

    if (_xsqPkg->GetRgbEffectsFile().IsOk()) {
        wxXmlNode* grpNode = nullptr;
        wxXmlNode* modelNode = nullptr;

        for (wxXmlNode* node = _xsqPkg->GetRgbEffectsFile().GetRoot()->GetChildren(); node != nullptr; node = node->GetNext()) {
            if (node->GetName() == "modelGroups") {
                grpNode = node;
                break;
            }
            if (node->GetName() == "models") {
                modelNode = node;
            }
        }

        if (grpNode) {
            for (wxXmlNode* node = grpNode->GetChildren(); node != nullptr; node = node->GetNext()) {
                if (auto mm = GetImportChannel(node->GetAttribute("name")); mm) {
                    mm->type = "ModelGroup";
                    mm->groupModels = node->GetAttribute("models");
                    mm->nodeCount = 1000; // just hardcode a large value
                }
            }
        }
        if (modelNode) {
            for (wxXmlNode* node = modelNode->GetChildren(); node != nullptr; node = node->GetNext()) {
                if (auto mm = GetImportChannel(node->GetAttribute("name")); mm) {
                    mm->type = node->GetAttribute("DisplayAs");
                    bool singingFace = false;
                    if (mm->type == "Custom") {
                        for (wxXmlNode* nodechildren = node->GetChildren(); nodechildren != nullptr; nodechildren = nodechildren->GetNext()) {
                            if (nodechildren->GetName() == "faceInfo") {
                                singingFace = true;
                            }
                        }
                    }
                    bool spiralTree = node->GetAttribute("TreeSpiralRotations", "0") != "0";
                    bool sticks = node->GetAttribute("CandyCaneSticks", "false") == "true";
                    std::string dropPattern = node->GetAttribute("DropPattern", "");
                    mm->modelClass = Model::DetermineClass(mm->type, singingFace, spiralTree, sticks, dropPattern);
                    if (mm->type == "Channel Block")
                    {
                        // these models use param1 only
                        mm->nodeCount = wxAtoi(node->GetAttribute("param1", "1"));
                    } else if (mm->type == "Cube") {
                        // these use param1 * param2 * param3
                        mm->nodeCount = wxAtoi(node->GetAttribute("param1", "1")) * wxAtoi(node->GetAttribute("param2", "1")) * wxAtoi(node->GetAttribute("param3", "1"));
                    } 
                    else if (mm->type == "Custom") {
                        auto data = XmlSerialize::ParseCustomModel(node->GetAttribute("CustomModel", ""));

                        int count = 0;
                        for (int l = 0; l < data.size(); l++) {
                            for (int r = 0; r < data[l].size(); r++) {
                                for (int c = 0; c < data[l][r].size(); c++) {
                                    if (data[l][r][c] != 0) {
                                        ++count;
                                    }
                                }
                            }
                        }
                        mm->nodeCount = count;
                    }
                    else {
                        // these use param1 * param2
                        mm->nodeCount = wxAtoi(node->GetAttribute("param1", "1")) * wxAtoi(node->GetAttribute("param2", "1"));
                    }
                }
                for (wxXmlNode* n = node->GetChildren(); n != nullptr; n = n->GetNext()) {
                    if (n->GetName() == "subModel") {
                        if (auto mm = GetImportChannel(node->GetAttribute("name") + "/" + n->GetAttribute("name")); mm) {
                            mm->type = "SubModel";
                        }
                    }
                }

            }
        }
    }
}

std::vector<std::string> const xLightsImportChannelMapDialog::GetChannelNames() const
{
    std::vector<std::string> itemList;

    std::transform(importChannels.begin(), importChannels.end(), std::back_inserter(itemList),
                   [](auto const& str) { return str->name; });
    return itemList;
}

ImportChannel* xLightsImportChannelMapDialog::GetImportChannel(std::string const& name) const
{
    if (auto const found{ std::find_if(importChannels.begin(), importChannels.end(),
                                       [&name](auto const& c) { return c->name == name; }) };
        found != importChannels.end()) {
        return found->get();
    }
    return nullptr;
}

void xLightsImportChannelMapDialog::SortChannels()
{
    std::sort(importChannels.begin(), importChannels.end(), [](const auto& lhs, const auto& rhs) {
        return stdlistNumberAwareStringCompare(lhs->name, rhs->name);
    });
}

void xLightsImportChannelMapDialog::AddChannel(std::string const& name, int effectCount, bool isNode)
{
    importChannels.emplace_back(new ImportChannel(name, effectCount, isNode));
}

void xLightsImportChannelMapDialog::loadMapHintsFile(wxString const& filename) {
    // <MapHints>
    //  <Map ToRegex"" FromModel="" />
    // </MapHints>

    wxXmlDocument doc;
    doc.Load(filename);
    if (doc.IsOk()) {
        for (wxXmlNode* n = doc.GetRoot()->GetChildren(); n != nullptr; n = n->GetNext()) {
            if (n->GetName().Lower() == "map") {
                auto toRegex = n->GetAttribute("ToRegex");
                auto fromModel = n->GetAttribute("FromModel");
                auto applyTo = n->GetAttribute("ApplyTo", "B");
                if (toRegex != "" && fromModel != "") {
                    DoAutoMap(regex, regex, norm, toRegex, fromModel, applyTo, false);
                }
            }
        }
    }
}

void xLightsImportChannelMapDialog::generateMapHintsFile(wxString const& filename) {
    //create basic xmaphints file
    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
    }

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<MapHints>\n");

    for (size_t i = 0; i < _dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = _dataModel->GetNthChild(i);
        if (m->HasMapping()) {
            if (!m->_mapping.empty()) {
                f.Write(wxString::Format("    <Map ToRegex=\"^%s$\" FromModel=\"%s\" ApplyTo=\"B\" />\n", EscapeRegex(m->_model), m->_mapping));
            }
            for (size_t j = 0; j < m->GetChildCount(); ++j) {
                xLightsImportModelNode* s = m->GetNthChild(j);
                if (s->HasMapping()) {
                    if (!s->_mapping.empty()) {
                        f.Write(wxString::Format("    <Map ToRegex=\"^%s\\/%s$\" FromModel=\"%s\" ApplyTo=\"B\" />\n",  EscapeRegex(m->_model), EscapeRegex(s->_strand), s->_mapping));
                    }
                    for (size_t k = 0; k < s->GetChildCount(); ++k) {
                        xLightsImportModelNode* n = s->GetNthChild(k);
                        if (n->HasMapping()) {
                            f.Write(wxString::Format("    <Map ToRegex=\"^%s\\/%s\\/%s$\" FromModel=\"%s\" ApplyTo=\"B\" />\n",  EscapeRegex(m->_model), EscapeRegex(s->_strand), EscapeRegex(n->_node), n->_mapping));
                        }
                    }
                }
            }

        }
    }
    f.Write("<!-- Samples\n");
    f.Write("    <Map ToRegex=\"^Star(s?)(\\sGroup)?$\" FromModel=\"Star\" ApplyTo=\"B\" />\n");
    f.Write("    <Map ToRegex=\"(.*Mega|^Pixel)\\sTree$\" FromModel=\"Mega Tree\" ApplyTo=\"M\" />\n");
    f.Write("    <Map ToRegex=\"(^ALL|^Whole)\\sHouse$\" FromModel=\"ALL House\" ApplyTo=\"G\" />\n");
    f.Write("-->\n");

    f.Write("</MapHints>");
    f.Close();
}

void xLightsImportChannelMapDialog::OnTextCtrl_FindFromText(wxCommandEvent& event)
{
    // find the first line starting with the text
    int index = -1;
    auto from = TextCtrl_FindFrom->GetValue().Lower();

    if (from == "")
    {
        // text just erased ... so scroll to the top
        index = 0;
    }
    else
    {
        for (size_t i = 0; i < ListCtrl_Available->GetItemCount(); ++i)
        {
            if (ListCtrl_Available->GetItemText(i).Lower().StartsWith(from))
            {
                index = i;
                break;
            }
        }
    }

    // if nothing found then find the first line containing the text
    if (index == -1)
    {
        for (size_t i = 0; i < ListCtrl_Available->GetItemCount(); ++i) {
            if (ListCtrl_Available->GetItemText(i).Lower().Contains(from)) {
                index = i;
                break;
            }
        }

    }

    // scroll to it
    ListCtrl_Available->EnsureVisible(index);
}

void xLightsImportChannelMapDialog::OnTextCtrl_FindToText(wxCommandEvent& event)
{
    // find the first line starting with the text
    wxDataViewItem index = wxDataViewItem(0);
    std::string to = TextCtrl_FindTo->GetValue().Lower().ToStdString();

    if (to.empty()) {
        if (TreeListCtrl_Mapping->GetSelectedItemsCount() == 1) {
            // if there is a selection, scroll to it as that's the visible marker
            // at this point.
            TreeListCtrl_Mapping->EnsureVisible(TreeListCtrl_Mapping->GetSelection());
        }
#ifdef __WXMSW__
        else {
            // There isn't a way to scroll to top on MacOS and Linux as far as I can find. Honestly, this shouldn't
            // work on Windows either as wxDataViewControl does not inherit from wxScrollHelperBase according
            // to the wxWidgets docs and thus should not implement the Scroll method
            TreeListCtrl_Mapping->Scroll(wxPoint(0, 0));
        }
#endif
    } else {
        for (size_t i = 0; i < _dataModel->GetChildCount(); ++i) {
            xLightsImportModelNode* m = _dataModel->GetNthChild(i);
            if (StartsWith(::Lower(m->_model), to)) {
                index = (wxDataViewItem)m;
                break;
            }
        }

        // if nothing found then find the first line containing the text
        if (index.GetID() == 0) {
            for (size_t i = 0; i < _dataModel->GetChildCount(); ++i) {
                xLightsImportModelNode* m = _dataModel->GetNthChild(i);
                if (Contains(::Lower(m->_model), to)) {
                    index = (wxDataViewItem)m;
                    break;
                }
            }
        }

        // scroll to it
        TreeListCtrl_Mapping->EnsureVisible(index);
    }
}

void xLightsImportChannelMapDialog::OnButton_UpdateAliasesClick(wxCommandEvent& event)
{
    wxDataViewItemArray models;
    _dataModel->GetChildren(wxDataViewItem(0), models);
    for (size_t i = 0; i < models.size(); ++i) {
        xLightsImportModelNode* m = _dataModel->GetNthChild(i);
        if (m->HasMapping() && !m->_mapping.empty()) {
            xlights->GetModel(m->_model)->AddAlias(m->_mapping);
        }
        wxDataViewItemArray strands;
        _dataModel->GetChildren(models[i], strands);
        for (size_t j = 0; j < strands.size(); ++j) {
            xLightsImportModelNode* astrand = (xLightsImportModelNode*)strands[j].GetID();
            if (astrand->HasMapping() && !astrand->_mapping.empty()) {
                auto sm0 = Split(astrand->_mapping, '/');
                if ((sm0[0] != m->_mapping) || ((sm0.size() > 1) && (sm0[1] != astrand->_strand))) {
                    auto sm = xlights->GetModel((astrand->_model + "/" + astrand->_strand));
                    if (sm != nullptr)
                        sm->AddAlias(astrand->_mapping);
                }
            }
        }
    }
    xlights->SetStatusText(_("Update Aliases Done."));
}

void xLightsImportChannelMapDialog::OnClose(wxCloseEvent& event)
{
    if (_dirty) {
        if (wxMessageBox("Are you sure you want to exit WITHOUT saving your mapping changes for future imports?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxYES) {
            EndDialog(wxID_CANCEL);
        }

    } else {
        EndDialog(wxID_CANCEL);
    }
}

void xLightsImportChannelMapDialog::OnButton_AIMapClick(wxCommandEvent& event)
{
    if (_dataModel == nullptr)
        return;

    DoAIAutoMap(false);

    TreeListCtrl_Mapping->Refresh();
    MarkUsed();
}

bool xLightsImportChannelMapDialog::IsConvertRender() const
{
    wxConfigBase* config = wxConfigBase::Get();
    bool b = CheckBox_ConvertRenderStyle->IsChecked();
    config->Write("ImportEffectsRenderStyle", b);
    config->Flush();
    return b;
}
