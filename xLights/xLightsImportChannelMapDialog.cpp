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
#include <wx/regex.h>

#include "xLightsImportChannelMapDialog.h"
#include "sequencer/SequenceElements.h"
#include "xLightsMain.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "UtilFunctions.h"
#include "ExternalHooks.h"
#include "MediaImportOptionsDialog.h"
#include "LayoutUtils.h"
#include "../../xSchedule/wxJSON/jsonreader.h"
#include "../../xSchedule/wxJSON/jsonwriter.h"
#include <algorithm>

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

        if (dlg.ShowModal() == wxID_OK) {
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
        if (value.GetString() != "") {
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
    if (!node) {     // happens if item.IsOk()==false
        return false;
    }

    bool set = false;

    if (!node->_mappingExists) {
        attr.SetBackgroundColour(*wxRED);
        set = true;
    }

    if (node->IsGroup()) {
            attr.SetColour(CyanOrBlue());
        set = true;
    }

    return set;
}

int xLightsImportTreeModel::Compare(const wxDataViewItem& item1, const wxDataViewItem& item2, unsigned column, bool ascending) const
{
    if (column == 0) {
        xLightsImportModelNode *node1 = (xLightsImportModelNode*)item1.GetID();
        xLightsImportModelNode *node2 = (xLightsImportModelNode*)item2.GetID();

        if (node1->_node != "" && node2->_node != "") {
            if (ascending) {
                return NumberAwareStringCompare(node1->_node.ToStdString(), node2->_node.ToStdString());
            }
            else {
                return NumberAwareStringCompareRev(node1->_node.ToStdString(), node2->_node.ToStdString());
            }
        } else if (node1->_strand != "" && node2->_strand != "") {
            if (ascending) {
                return NumberAwareStringCompare(node1->_strand.ToStdString(), node2->_strand.ToStdString());
            }
            else {
                return NumberAwareStringCompareRev(node1->_strand.ToStdString(), node2->_strand.ToStdString());
            }
        }
        else
        {
            if (ascending) {
                return NumberAwareStringCompare(GetModel(item1).ToStdString(), GetModel(item2).ToStdString());
            }
            else {
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
    if (col == 1) {
        node->_mapping = variant.GetString();
        node->_mappingExists = false;
        return true;
    } else if (col == 2) {
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
const long xLightsImportChannelMapDialog::ID_SPINCTRL1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_CHECKBOX1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_CHECKBOX11 = wxNewId();
const long xLightsImportChannelMapDialog::ID_CHECKBOX4 = wxNewId();
const long xLightsImportChannelMapDialog::ID_CHECKBOX2 = wxNewId();
const long xLightsImportChannelMapDialog::ID_STATICTEXT_BLEND_TYPE = wxNewId();
const long xLightsImportChannelMapDialog::ID_CHECKBOX3 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON_IMPORT_OPTIONS = wxNewId();
const long xLightsImportChannelMapDialog::ID_CHECKLISTBOX1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_STATICTEXT2 = wxNewId();
const long xLightsImportChannelMapDialog::ID_TEXTCTRL2 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON3 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON4 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON5 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON2 = wxNewId();
const long xLightsImportChannelMapDialog::ID_PANEL1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_STATICTEXT1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_TEXTCTRL1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_LISTCTRL1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_PANEL2 = wxNewId();
const long xLightsImportChannelMapDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

const long xLightsImportChannelMapDialog::ID_MNU_SELECTALL = wxNewId();
const long xLightsImportChannelMapDialog::ID_MNU_SELECTNONE = wxNewId();

BEGIN_EVENT_TABLE(xLightsImportChannelMapDialog,wxDialog)
	//(*EventTable(xLightsImportChannelMapDialog)
	//*)
END_EVENT_TABLE()

xLightsImportChannelMapDialog::xLightsImportChannelMapDialog(wxWindow* parent, const wxFileName& filename, bool allowTimingOffset, bool allowTimingTrack, bool allowColorChoice, bool allowCCRStrand, bool allowImportBlend, wxWindowID id, const wxPoint& pos, const wxSize& size)
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
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;

    Create(parent, wxID_ANY, _("Map Channels"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX|wxFULL_REPAINT_ON_RESIZE, _T("wxID_ANY"));
    OldSizer = new wxFlexGridSizer(0, 1, 0, 0);
    OldSizer->AddGrowableCol(0);
    OldSizer->AddGrowableRow(0);
    SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
    SplitterWindow1->SetSashGravity(0.5);
    Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
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
    FlexGridSizer11 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer11->AddGrowableCol(0);
    CheckBox_EraseExistingEffects = new wxCheckBox(Panel1, ID_CHECKBOX11, _("Erase existing effects on imported models"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX11"));
    CheckBox_EraseExistingEffects->SetValue(false);
    FlexGridSizer11->Add(CheckBox_EraseExistingEffects, 1, wxALL|wxEXPAND, 5);
    CheckBox_LockEffects = new wxCheckBox(Panel1, ID_CHECKBOX4, _("Lock effects on import"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
    CheckBox_LockEffects->SetValue(true);
    FlexGridSizer11->Add(CheckBox_LockEffects, 1, wxALL|wxEXPAND, 5);
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
    TimingTrackListBox = new wxCheckListBox(Panel1, ID_CHECKLISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, wxVSCROLL, wxDefaultValidator, _T("ID_CHECKLISTBOX1"));
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
    FlexGridSizer2 = new wxFlexGridSizer(0, 7, 0, 0);
    FlexGridSizer2->AddGrowableCol(2);
    Button_Ok = new wxButton(Panel1, ID_BUTTON3, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Cancel = new wxButton(Panel1, ID_BUTTON4, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxEXPAND, 5);
    Button_AutoMap = new wxButton(Panel1, ID_BUTTON5, _("Auto Map"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    FlexGridSizer2->Add(Button_AutoMap, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button01 = new wxButton(Panel1, ID_BUTTON1, _("Load Mapping"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer2->Add(Button01, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button02 = new wxButton(Panel1, ID_BUTTON2, _("Save Mapping"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer2->Add(Button02, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Sizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
    Panel1->SetSizer(Sizer1);
    Panel2 = new wxPanel(SplitterWindow1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    Sizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    Sizer2->AddGrowableCol(0);
    Sizer2->AddGrowableRow(1);
    FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer3->AddGrowableCol(1);
    StaticText1 = new wxStaticText(Panel2, ID_STATICTEXT1, _("Find:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_FindFrom = new wxTextCtrl(Panel2, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer3->Add(TextCtrl_FindFrom, 1, wxALL|wxEXPAND, 5);
    Sizer2->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
    ListCtrl_Available = new wxListCtrl(Panel2, ID_LISTCTRL1, wxDefaultPosition, wxDLG_UNIT(Panel2,wxSize(100,-1)), wxLC_REPORT|wxLC_SINGLE_SEL|wxVSCROLL, wxDefaultValidator, _T("ID_LISTCTRL1"));
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
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::LoadMapping);
    Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::SaveMapping);
    Connect(ID_TEXTCTRL1, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnTextCtrl_FindFromText);
    Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_BEGIN_DRAG, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnListCtrl_AvailableBeginDrag);
    Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_ITEM_SELECTED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnListCtrl_AvailableItemSelect);
    Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_ITEM_ACTIVATED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnListCtrl_AvailableItemActivated);
    Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_COL_CLICK, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnListCtrl_AvailableColumnClick);
    //*)

    Connect(ID_CHECKLISTBOX1, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xLightsImportChannelMapDialog::RightClickTimingTracks);

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
    CheckBox_LockEffects->SetValue(config->ReadBool("ImportEffectsLocked", true));
    
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

    m_imageList = std::make_unique<wxImageList>(16, 16, true);
    LayoutUtils::CreateImageList(m_imageList.get(), m_iconIndexMap);
    ListCtrl_Available->SetImageList(m_imageList.get(), wxIMAGE_LIST_SMALL);
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
    TreeListCtrl_Mapping->AppendColumn(new wxDataViewColumn("Map To", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_ACTIVATABLE, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL), 1, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
    if (_allowColorChoice) {
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

        ListCtrl_Available->AppendColumn("# Effects");
        bool countEnabled{false};
        for (auto const& m : importChannels) {
            ListCtrl_Available->InsertItem(j, m->name);
            ListCtrl_Available->SetItemData(j, j);
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
                ListCtrl_Available->SetItemTextColour(j, CyanOrBlue());
            }
            j++;
        }
        if (!countEnabled) {
            ListCtrl_Available->DeleteColumn(1);
        }
    }

    _sortOrder = 1;
    ListCtrl_Available->SortItems(MyCompareFunctionAsc, (wxIntPtr)ListCtrl_Available);

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

    xLightsImportModelNode *lastmodel = new xLightsImportModelNode(nullptr, m->GetName(), std::string(""), true, *wxWHITE, (m->GetDisplayAs() == "ModelGroup"));
    _dataModel->BulkInsert(lastmodel, ms++);
    for (int s = 0; s < m->GetNumSubModels(); ++s) {
        Model *subModel = m->GetSubModel(s);
        xLightsImportModelNode* laststrand;
        if (channelColors.find(subModel->GetName()) != channelColors.end()) {
            laststrand = new xLightsImportModelNode(lastmodel, m->GetName(), subModel->GetName(), std::string(""), true, channelColors.find(subModel->GetName())->second.asWxColor());
        } else {
            laststrand = new xLightsImportModelNode(lastmodel, m->GetName(), subModel->GetName(), std::string(""), true, *wxWHITE);
        }
        lastmodel->Append(laststrand);
    }

    for (int s = 0; s < m->GetNumStrands(); ++s) {
        wxString sn = m->GetStrandName(s);
        if (sn == "") {
            sn = wxString::Format("Strand %d", s + 1);
        }
        xLightsImportModelNode* laststrand;
        if (channelColors.find(sn.ToStdString()) != channelColors.end()) {
            laststrand = new xLightsImportModelNode(lastmodel, m->GetName(), sn, std::string(""), true, channelColors.find(sn.ToStdString())->second.asWxColor());
        } else {
            laststrand = new xLightsImportModelNode(lastmodel, m->GetName(), sn, std::string(""), true, *wxWHITE);
        }
        lastmodel->Append(laststrand);
        for (int n = 0; n < m->GetStrandLength(s); ++n) {
            wxString nn = m->GetNodeName(n);
            if (nn == "") {
                nn = wxString::Format("Node %d", n + 1);
            }
            xLightsImportModelNode* lastnode;
            if (channelColors.find(nn.ToStdString()) != channelColors.end()) {
                lastnode = new xLightsImportModelNode(laststrand, m->GetName(), sn, nn, std::string(""), true, channelColors.find(nn.ToStdString())->second.asWxColor());
            } else {
                lastnode = new xLightsImportModelNode(laststrand, m->GetName(), sn, nn, std::string(""), true, *wxWHITE);
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
                    // this is a submodel ... dont add it

                    // The risk here is the submodel is in the group but the parent model isnt so there will be no way to
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
        event.GetModel()->GetValue(vvalue, event.GetItem(), 1);
        std::string mapped = vvalue.GetString().ToStdString();
        TreeListCtrl_Mapping->UnselectAll();
        TreeListCtrl_Mapping->Select(event.GetItem());
        if (mapped == "" && ListCtrl_Available->GetSelectedItemCount() > 0) {
            int itemIndex = ListCtrl_Available->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            Map(event.GetItem(), ListCtrl_Available->GetItemText(itemIndex).ToStdString());
        } else {
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
    if (_allowColorChoice) {
        if (event.GetItem().IsOk()) {
            if (event.GetColumn() == 1) {
                wxVariant vvalue;
                event.GetModel()->GetValue(vvalue, event.GetItem(), 1);
                std::string value = vvalue.GetString().ToStdString();
                if (channelColors.find(value) != channelColors.end()) {
                    event.GetModel()->SetValue(wxVariant(channelColors.find(value)->second.asWxColor().GetAsString()), event.GetItem(), 2);
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
        if (m->_model.ToStdString() == model) {
            if (strand == "") {
                return m;
            } else {
                for (size_t j = 0; j < m->GetChildCount(); ++j) {
                    xLightsImportModelNode* s = m->GetNthChild(j);
                    if (s->_strand.ToStdString() == strand) {
                        if (node == "") {
                            return s;
                        } else {
                            for (size_t k = 0; k < s->GetChildCount(); ++k) {
                                xLightsImportModelNode* n = s->GetNthChild(k);
                                if (n->_node.ToStdString() == node) {
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
        if (wxMessageBox("Are you sure you dont want to save your changes for future imports?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
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

    wxJSONValue data;
    wxJSONReader reader;
    wxFileInputStream f(filename);
    int errors = reader.Parse(f, &data);
    if (!errors) {

        //zip package settings
        if (_xsqPkg != nullptr && _xsqPkg->IsPkg()) {
            if (data.HasMember("facesdir") && !data["facesdir"].AsString().empty()) {
                _xsqPkg->GetImportOptions()->SetDir(MediaTargetDir::FACES_DIR,data["facesdir"].AsString());
            }
            if (data.HasMember("gladiatordir") && !data["gladiatordir"].AsString().empty()) {
                _xsqPkg->GetImportOptions()->SetDir(MediaTargetDir::GLEDIATORS_DIR,data["gladiatordir"].AsString());
            }
            if (data.HasMember("imagedir") && !data["imagedir"].AsString().empty()) {
                _xsqPkg->GetImportOptions()->SetDir(MediaTargetDir::IMAGES_DIR,data["imagedir"].AsString());
            }
            if (data.HasMember("shaderdir") && !data["shaderdir"].AsString().empty()) {
                _xsqPkg->GetImportOptions()->SetDir(MediaTargetDir::SHADERS_DIR,data["shaderdir"].AsString());
            }
            if (data.HasMember("videodir") && !data["videodir"].AsString().empty()) {
                _xsqPkg->GetImportOptions()->SetDir(MediaTargetDir::VIDEOS_DIR,data["videodir"].AsString());
            }
            if (data.HasMember("importmedia")) {
                CheckBoxImportMedia->SetValue(data.Get("importmedia", true).AsBool());
            }
        }

        //settings
        if (data.HasMember("eraseexisting")) {
            CheckBox_EraseExistingEffects->SetValue(data.Get("eraseexisting", false).AsBool());
        }
        if (_allowCCRStrand && data.HasMember("mapccrstrand")) {
            CheckBox_MapCCRStrand->SetValue(data.Get("mapccrstrand", false).AsBool());
        }
        if (_allowImportBlend && data.HasMember("importblendmode")) {
            CheckBox_Import_Blend_Mode->SetValue(data.Get("importblendmode", true).AsBool());
        }

        //selected timmings
        auto timingtracks = data["timingtracks"].AsArray();
        for (int i = 0; i < timingtracks->Count(); ++i) {
            wxString const ttname = timingtracks->Item(i).Get("name", wxString()).AsString();
            bool const ttenabled = timingtracks->Item(i).Get("enabled", true).AsBool();
            if(auto const& idx{std::find(timingTracks.begin(), timingTracks.end(), ttname)}; idx !=  timingTracks.end()) {
                auto index = std::distance(timingTracks.begin(), idx);
                TimingTrackListBox->Check(index, ttenabled);
            }
        }

        auto SetMapping = [&](wxString const& mapping, wxDataViewItem item, wxColor const& color) {
            TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(mapping), item, 1);
            ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel())->SetMappingExists(item, ListCtrl_Available->FindItem(0, mapping) >= 0);
            TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(color.GetAsString()), item, 2);
        };

        //mappings
        auto mappings = data["mappings"].AsArray();
        for (int i = 0; i < mappings->Count(); ++i) {
            wxString const model = mappings->Item(i).Get("model", wxString()).AsString();
            wxString const strand = mappings->Item(i).Get("strand", wxString()).AsString();
            wxString const node = mappings->Item(i).Get("node", wxString()).AsString();
            wxString const mapping = mappings->Item(i).Get("mapping", wxString()).AsString();
            wxColor color = wxColor(mappings->Item(i).Get("color", wxString("white")).AsString());
            bool stashed = mappings->Item(i).Get("stashed", false).AsBool();

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
                        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(mapping), item, 1);
                        ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel())->SetMappingExists(item, ListCtrl_Available->FindItem(0, mapping) >= 0);
                        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(color.GetAsString()), item, 2);
                    } else if (msi != nullptr) {
                        wxDataViewItem item = FindItem(model.ToStdString(), strand.ToStdString());
                        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(mapping), item, 1);
                        ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel())->SetMappingExists(item, ListCtrl_Available->FindItem(0, mapping) >= 0);
                        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(color.GetAsString()), item, 2);
                    } else {
                        wxDataViewItem item = FindItem(model.ToStdString());
                        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(mapping), item, 1);
                        ((xLightsImportTreeModel*)TreeListCtrl_Mapping->GetModel())->SetMappingExists(item, ListCtrl_Available->FindItem(0, mapping) >= 0);
                        TreeListCtrl_Mapping->GetModel()->SetValue(wxVariant(color.GetAsString()), item, 2);
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
    wxJSONValue data;

    //mappings
    wxJSONValue allMappings;
    for (size_t i = 0; i < _dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = _dataModel->GetNthChild(i);
        if (m->HasMapping()) {
            wxJSONValue mapping;

            wxString mn = m->_model;
            mapping["model"] = mn;
            StashedMapping* sm = GetStashedMapping(mn, "", "");
            if (m->_mapping.empty() && sm != nullptr) {
                mapping["mapping"] = sm->_mapping;
                mapping["color"] = sm->_color.GetAsString();
                mapping["stashed"] = true;
            } else {
                mapping["mapping"] = m->_mapping;
                mapping["color"] = m->_color.GetAsString();
            }
            allMappings.Append(mapping);
            for (size_t j = 0; j < m->GetChildCount(); ++j) {
                xLightsImportModelNode* s = m->GetNthChild(j);
                if (s->HasMapping() || AnyStashedMappingExists(mn, s->_strand)) {
                    sm = GetStashedMapping(mn, s->_strand, "");

                    wxJSONValue smapping;
                    smapping["model"] = mn;
                    if (s->_mapping.empty() && sm != nullptr) {
                        smapping["strand"] = sm->_strand;
                        smapping["mapping"] = sm->_mapping;
                        smapping["color"] = sm->_color.GetAsString();
                        smapping["stashed"] = true;
                    } else {
                        smapping["strand"] = s->_strand;
                        smapping["mapping"] = s->_mapping;
                        smapping["color"] = s->_color.GetAsString();
                    }
                    allMappings.Append(smapping);

                    for (size_t k = 0; k < s->GetChildCount(); ++k) {
                        xLightsImportModelNode* n = s->GetNthChild(k);
                        sm = GetStashedMapping(mn, n->_strand, n->_node);
                        if (n->_mapping.empty() && sm != nullptr) {
                            wxJSONValue nmapping;
                            nmapping["model"] = mn;
                            nmapping["strand"] = sm->_strand;
                            nmapping["node"] = sm->_node;
                            nmapping["mapping"] = sm->_mapping;
                            nmapping["color"] = sm->_color.GetAsString();
                            nmapping["stashed"] = true;
                            allMappings.Append(nmapping);
                        } else {
                            if (n->HasMapping()) {
                                wxJSONValue nmapping;
                                nmapping["model"] = mn;
                                nmapping["strand"] = n->_strand;
                                nmapping["node"] = n->_node;
                                nmapping["mapping"] = n->_mapping;
                                nmapping["color"] = n->_color.GetAsString();
                                allMappings.Append(nmapping);
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
            wxJSONValue mapping;
            mapping["model"] = (*it)->_model;
            mapping["strand"] = (*it)->_strand;
            mapping["node"] = (*it)->_node;
            mapping["mapping"] = (*it)->_mapping;
            mapping["color"] = (*it)->_color.GetAsString();
            mapping["stashed"] = true;
            allMappings.Append(mapping);
        }
    }

    data["mappings"] = allMappings;

    //selected timmings
    for (size_t tt = 0; tt < TimingTrackListBox->GetCount(); ++tt)
    {
        wxJSONValue timtrack;
        timtrack["name"] = timingTracks[tt];
        timtrack["enabled"] = TimingTrackListBox->IsChecked(tt);
        data["timingtracks"].Append(timtrack);
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

    wxFileOutputStream mapfile(filename);
    wxJSONWriter writer(wxJSONWRITER_STYLED, 0, 3);
    writer.Write(data, mapfile);
    mapfile.Close();
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
        if (wxMessageBox("Are you sure you want to exit WITHOUT saving your mapping changes for future imports?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxYES) {
            EndDialog(wxID_OK);
        }
    } else {
        EndDialog(wxID_OK);
    }
}

void xLightsImportChannelMapDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    if (_dirty) {
        if (wxMessageBox("Are you sure you dont want to save your changes for future imports?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxYES) {
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
    if (_sortOrder == 0) {
        _sortOrder = 1;
        ListCtrl_Available->SortItems(MyCompareFunctionAsc, (wxIntPtr) ListCtrl_Available);
    } else {
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
                HandleDropAvailable(item, model);
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
        event.GetModel()->GetValue(vvalue, event.GetItem(), 1);
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

                HandleDropAvailable(item, parms[1].ToStdString());
            }
        }
        break;
        default:
            break;
    }

    TreeListCtrl_Mapping->Refresh();

    MarkUsed();
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
                Map(it2, fromname);
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
            Map(it, fromname);
        }
    }
}

void xLightsImportChannelMapDialog::HandleDropAvailable(wxDataViewItem dropTarget, std::string availableModelName) {

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
                Map(selItem, availableModelName);
            }
            lastSelected = selectedItems.Item(selectedItems.size() - 1);
        } else {
            Map(dropTarget, availableModelName);
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
        if (model->_mapping != "") {
            if (std::find(used.begin(), used.end(), model->_mapping) == used.end()) {
                used.push_back(model->_mapping.ToStdString());
            }
        }

        for (unsigned int j = 0; j < model->GetChildCount(); ++j) {
            auto strand = model->GetNthChild(j);
            if (strand->_mapping != "") {
                if (std::find(used.begin(), used.end(), strand->_mapping) == used.end()) {
                    used.push_back(strand->_mapping.ToStdString());
                }
            }

            for (unsigned int k = 0; k < strand->GetChildCount(); ++k) {
                auto node = strand->GetNthChild(k);
                if (node->_mapping != "") {
                    if (std::find(used.begin(), used.end(), node->_mapping) == used.end()) {
                        used.push_back(node->_mapping.ToStdString());
                    }
                }
            }
        }
    }

    used.sort();

    int items = ListCtrl_Available->GetItemCount();
    ListCtrl_Available->Freeze();
    for (int i = 0; i < items; ++i) {
        if (!std::binary_search(used.begin(), used.end(), ListCtrl_Available->GetItemText(i).ToStdString())) {
            // not used
            ImportChannel* im = GetImportChannel(ListCtrl_Available->GetItemText(i).ToStdString());
            if (im != nullptr && im->name == "ModelGroup") {
                ListCtrl_Available->SetItemTextColour(i, CyanOrBlue());
            } else {
                ListCtrl_Available->SetItemTextColour(i, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
            }
        } else {
            //used
            ListCtrl_Available->SetItemTextColour(i, LightOrMediumGrey());
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

void xLightsImportChannelMapDialog::DoAutoMap(
    std::function<bool (const std::string&, const std::string&, const std::string&, const std::string&)> lambda_model,
    std::function<bool (const std::string&, const std::string&, const std::string&, const std::string&)> lambda_strand,
    std::function<bool (const std::string&, const std::string&, const std::string&, const std::string&)> lambda_node,
    const std::string& extra1, const std::string& extra2, const std::string& mg)
{
    for (unsigned int i = 0; i < _dataModel->GetChildCount(); ++i) {
        auto model = _dataModel->GetNthChild(i);
        if (model != nullptr) {
            if ((model->IsGroup() && (mg == "B" || mg == "G")) || (!model->IsGroup() && (mg == "B" || mg == "M"))) {
                for (int j = 0; j < ListCtrl_Available->GetItemCount(); ++j) {
                    wxString const availName = ListCtrl_Available->GetItemText(j).Trim(true).Trim(false).Lower();
                    if (availName.Contains("/")) {
                        wxArrayString const parts = wxSplit(availName, '/');
                        if (lambda_model(model->_model, parts[0], extra1, extra2)) {
                            // matched the model name ... need to look at strands and submodels
                            for (unsigned int k = 0; k < model->GetChildCount(); ++k) {
                                auto strand = model->GetNthChild(k);
                                if (strand != nullptr) {
                                    if (lambda_strand(strand->_strand, parts[1], extra1, extra2)) {
                                        // matched to the strand level
                                        if (parts.size() == 2) {
                                            if (strand->_mapping.empty()) {
                                                strand->_mapping = ListCtrl_Available->GetItemText(j);
                                                strand->_mappingExists = true;
                                            }
                                        } else {
                                            // need to map the node level
                                            for (unsigned int m = 0; m < strand->GetChildCount(); ++m) {
                                                auto node = strand->GetNthChild(m);
                                                if (node != nullptr) {
                                                    if (node->_mapping.empty()) {
                                                        if (lambda_node(node->_node, parts[2], extra1, extra2)) {
                                                            // matched to the node level
                                                            if (parts.size() == 3) {
                                                                node->_mapping = ListCtrl_Available->GetItemText(j);
                                                                node->_mappingExists = true;
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
                    } else {
                        if (model->_mapping.empty() && lambda_model(model->_model, availName, extra1, extra2)) {
                            model->_mapping = ListCtrl_Available->GetItemText(j);
                            model->_mappingExists = true;
                        }
                        for (unsigned int k = 0; k < model->GetChildCount(); ++k) {
                            auto strand = model->GetNthChild(k);
                            if (strand != nullptr) {
                                if (strand->_mapping.empty() &&lambda_strand(model->_model + "/" + strand->_strand, availName, extra1, extra2)) {
                                    strand->_mapping = ListCtrl_Available->GetItemText(j);
                                    strand->_mappingExists = true;
                                }
                                //for (unsigned int m = 0; m < strand->GetChildCount(); ++m) {
                                //    auto node = strand->GetNthChild(m);
                                //    if (node != nullptr) {
                                //        if (node->_mapping.empty()) {
                                //            if (lambda_node(model->_model + "/" + strand->_strand + "/" + node->_node, availName, extra1, extra2)) {
                                //                // matched to the node level
                                //                node->_mapping = ListCtrl_Available->GetItemText(j);
                                //                node->_mappingExists = true;
                                //            }
                                //        }
                                //    }
                                //}
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
}

void xLightsImportChannelMapDialog::OnButton_AutoMapClick(wxCommandEvent& event)
{
    if (_dataModel == nullptr) return;

    DoAutoMap(norm, norm, norm, "", "", "B");
    DoAutoMap(aggressive, aggressive, aggressive, "", "", "B");

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
        Map(mapTo, ListCtrl_Available->GetItemText(event.GetItem()).ToStdString());
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
                }
            }
        }
        if (modelNode) {
            for (wxXmlNode* node = modelNode->GetChildren(); node != nullptr; node = node->GetNext()) {
                if (auto mm = GetImportChannel(node->GetAttribute("name")); mm) {
                    mm->type = node->GetAttribute("DisplayAs");
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

void xLightsImportChannelMapDialog::AddChannel(std::string const& name, int effectCount)
{
    importChannels.emplace_back(new ImportChannel(name, effectCount));
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
                    DoAutoMap(regex, regex, norm, toRegex, fromModel, applyTo);
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
    auto to = TextCtrl_FindTo->GetValue().Lower();

    if (to == "") {
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

            if (m->_model.Lower().StartsWith(to)) {
                index = (wxDataViewItem)m;
                break;
            }
        }

        // if nothing found then find the first line containing the text
        if (index.GetID() == 0) {
            for (size_t i = 0; i < _dataModel->GetChildCount(); ++i) {
                xLightsImportModelNode* m = _dataModel->GetNthChild(i);
                if (m->_model.Lower().Contains(to)) {
                    index = (wxDataViewItem)m;
                    break;
                }
            }
        }

        // scroll to it
        TreeListCtrl_Mapping->EnsureVisible(index);
    }
}
