#include "xLightsImportChannelMapDialog.h"
#include "sequencer/SequenceElements.h"
#include "xLightsMain.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include <log4cpp/Category.hh>

#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>
#include <wx/colordlg.h>

//(*InternalHeaders(xLightsImportChannelMapDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

class ColorRenderer : public wxDataViewCustomRenderer
{
    wxColor _color;

public:
    ColorRenderer() : wxDataViewCustomRenderer(GetDefaultType(), wxDATAVIEW_CELL_ACTIVATABLE)
    {
    }

    virtual bool ActivateCell(const wxRect &cell, wxDataViewModel *model, const wxDataViewItem &item, unsigned int col, const wxMouseEvent *mouseEvent) override
    {
        wxColourData data;
        data.SetColour(_color);
        wxColourDialog dlg(GetOwner()->GetOwner()->GetParent(), &data);

        if (dlg.ShowModal() == wxID_OK)
        {
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
        _color = wxColor(value.GetString());
        return true;
    }
};

xLightsImportTreeModel::xLightsImportTreeModel()
{
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
        return true;
    }
    else if (col == 2)
    {
        node->_color = wxColour(variant.GetString());
        return true;
    }
    return false;
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
    unsigned int count = 0;
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
const long xLightsImportChannelMapDialog::ID_CHECKLISTBOX1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON3 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON4 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(xLightsImportChannelMapDialog,wxDialog)
	//(*EventTable(xLightsImportChannelMapDialog)
	//*)
END_EVENT_TABLE()

xLightsImportChannelMapDialog::xLightsImportChannelMapDialog(wxWindow* parent, const wxFileName &filename, bool allowTimingOffset, bool allowTimingTrack, bool allowColorChoice, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _allowTimingOffset = allowTimingOffset;
    _allowTimingTrack = allowTimingTrack;
    _allowColorChoice = allowColorChoice;
    _filename = filename;

	//(*Initialize(xLightsImportChannelMapDialog)
	wxButton* Button01;
	wxFlexGridSizer* FlexGridSizer2;
	wxButton* Button02;

	Create(parent, wxID_ANY, _("Map Channels"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("wxID_ANY"));
	SetMaxSize(wxDLG_UNIT(parent,wxSize(-1,500)));
	Sizer = new wxFlexGridSizer(0, 1, 0, 0);
	Sizer->AddGrowableCol(0);
	Sizer_TimeAdjust = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText_TimeAdjust = new wxStaticText(this, wxID_ANY, _("Time Adjust (ms)"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	Sizer_TimeAdjust->Add(StaticText_TimeAdjust, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TimeAdjustSpinCtrl = new wxSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -10000, 600000, 0, _T("ID_SPINCTRL1"));
	TimeAdjustSpinCtrl->SetValue(_T("0"));
	Sizer_TimeAdjust->Add(TimeAdjustSpinCtrl, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Sizer->Add(Sizer_TimeAdjust, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TimingTrackPanel = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Timing Tracks"));
	TimingTrackListBox = new wxCheckListBox(this, ID_CHECKLISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, wxVSCROLL, wxDefaultValidator, _T("ID_CHECKLISTBOX1"));
	TimingTrackPanel->Add(TimingTrackListBox, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	Sizer->Add(TimingTrackPanel, 0, wxEXPAND, 0);
	SizerMap = new wxFlexGridSizer(0, 1, 0, 0);
	SizerMap->AddGrowableCol(0);
	SizerMap->AddGrowableRow(0);
	Sizer->Add(SizerMap, 0, wxEXPAND, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 5, 0, 0);
	FlexGridSizer2->AddGrowableCol(2);
	Button_Ok = new wxButton(this, ID_BUTTON3, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON4, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	Button01 = new wxButton(this, ID_BUTTON1, _("Load Mapping"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button01, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button02 = new wxButton(this, ID_BUTTON2, _("Save Mapping"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button02, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Sizer->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	SetSizer(Sizer);
	Sizer->Fit(this);
	Sizer->SetSizeHints(this);

	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsImportChannelMapDialog::OnButton_OkClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsImportChannelMapDialog::OnButton_CancelClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsImportChannelMapDialog::LoadMapping);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsImportChannelMapDialog::SaveMapping);
	//*)

    if (_filename != "")
    {
        SetLabel(GetLabel() + " - " + _filename.GetFullName());
    }

    _dirty = false;
}

xLightsImportChannelMapDialog::~xLightsImportChannelMapDialog()
{
	//(*Destroy(xLightsImportChannelMapDialog)
	//*)
    //delete dataModel;
}

int CountChar(wxString& line, char c)
{
    int count = 0;
    for (size_t x = 0; x < line.size(); x++) {
        if (line[x] == c) {
            count++;
        }
    }
    return count;
}

bool xLightsImportChannelMapDialog::Init() {
    if (channelNames.size() == 0)
    {
        wxMessageBox("No models to import to. Add some models to the rows of the effects grid.");
        return false;
    }

    if (!_allowTimingOffset)
    {
        TimeAdjustSpinCtrl->Hide();
        StaticText_TimeAdjust->Hide();
        Sizer->Remove(Sizer_TimeAdjust);
    }

    if (timingTracks.empty() || !_allowTimingTrack) {
        Sizer->Remove(TimingTrackPanel);
        TimingTrackListBox->Hide();
        if (!_allowTimingOffset)
        {
            Sizer->AddGrowableRow(0);
        }
        else
        {
            Sizer->AddGrowableRow(1);
        }
    } else {
        if (!_allowTimingOffset)
        {
            Sizer->AddGrowableRow(1);
        }
        else
        {
            Sizer->AddGrowableRow(2);
        }
        for (auto it = timingTracks.begin(); it != timingTracks.end(); it++) {
            TimingTrackListBox->Append(*it);
        }
    }
    Sizer->Fit(this);
    Sizer->SetSizeHints(this);

    // load the tree
    for (auto it = channelNames.begin(); it != channelNames.end(); it++)
    {
        _importModels.push_back(wxString(it->c_str()));
    }

    dataModel = new xLightsImportTreeModel();

    TreeListCtrl_Mapping = new wxDataViewCtrl(this, ID_TREELISTCTRL1, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES | wxDV_VERT_RULES, wxDefaultValidator);
    TreeListCtrl_Mapping->AssociateModel(dataModel);
    TreeListCtrl_Mapping->AppendColumn(new wxDataViewColumn("Model", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT, wxALIGN_LEFT), 0, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
    TreeListCtrl_Mapping->AppendColumn(new wxDataViewColumn("Map To", new wxDataViewChoiceRenderer(_importModels, wxDATAVIEW_CELL_EDITABLE, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL), 1, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
    if (_allowColorChoice)
    {
        TreeListCtrl_Mapping->AppendColumn(new wxDataViewColumn("Color", new ColorRenderer(), 2, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
    }
    TreeListCtrl_Mapping->SetMinSize(wxSize(0, 300));
    SizerMap->Add(TreeListCtrl_Mapping, 1, wxALL | wxEXPAND, 5);
    SizerMap->Layout();
    Sizer->Fit(this);
    Sizer->SetSizeHints(this);
    Sizer->Layout();
    Layout();

    Connect(ID_TREELISTCTRL1, wxEVT_DATAVIEW_SELECTION_CHANGED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnSelectionChanged);
    //Connect(ID_TREELISTCTRL1, wxEVT_DATAVIEW_ITEM_ACTIVATED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnItemActivated);
    Connect(ID_TREELISTCTRL1, wxEVT_DATAVIEW_ITEM_VALUE_CHANGED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnValueChanged);

    int ms = 0;
    for (size_t i = 0; i<mSequenceElements->GetElementCount(); i++) {
        if (mSequenceElements->GetElement(i)->GetType() == ELEMENT_TYPE_MODEL) {
            Element* e = mSequenceElements->GetElement(i);
            
            Model *m = xlights->GetModel(e->GetName());
            if (m != nullptr) {
                AddModel(m, ms);
            }
        }
    }
    
    return true;
}

void xLightsImportChannelMapDialog::AddModel(Model *m, int &ms) {
    for (size_t x = 0; x < dataModel->GetChildCount(); x++) {
        xLightsImportModelNode * tmp = dataModel->GetNthChild(x);
        if (tmp->_model == m->GetName()) {
            return;
        }
    }
    
    xLightsImportModelNode *lastmodel = new xLightsImportModelNode(nullptr, m->GetName(), "");
    dataModel->Insert(lastmodel, ms++);
            
    for (int s = 0; s < m->GetNumSubModels(); s++) {
        Model *subModel = m->GetSubModel(s);
        xLightsImportModelNode* laststrand = nullptr;
        if (channelColors.find(subModel->GetName()) != channelColors.end())
        {
            laststrand = new xLightsImportModelNode(lastmodel, m->GetName(), subModel->GetName(), "", channelColors.find(subModel->GetName())->second.asWxColor());
        }
        else
        {
            laststrand = new xLightsImportModelNode(lastmodel, m->GetName(), subModel->GetName(), "", *wxWHITE);
        }
        lastmodel->Append(laststrand);
    }
    
    for (int s = 0; s < m->GetNumStrands(); s++) {
        wxString sn = m->GetStrandName(s);
        if ("" == sn) {
            sn = wxString::Format("Strand %d", s + 1);
        }
        xLightsImportModelNode* laststrand = nullptr;
        if (channelColors.find(sn.ToStdString()) != channelColors.end())
        {
            laststrand = new xLightsImportModelNode(lastmodel, m->GetName(), sn, "", channelColors.find(sn.ToStdString())->second.asWxColor());
        }
        else
        {
            laststrand = new xLightsImportModelNode(lastmodel, m->GetName(), sn, "", *wxWHITE);
        }
        lastmodel->Append(laststrand);
        for (int n = 0; n < m->GetStrandLength(s); n++)
        {
            wxString nn = m->GetNodeName(n);
            if ("" == nn)
            {
                nn = wxString::Format("Node %d", n + 1);
            }
            xLightsImportModelNode* lastnode = nullptr;
            if (channelColors.find(nn.ToStdString()) != channelColors.end())
            {
                lastnode = new xLightsImportModelNode(laststrand, m->GetName(), sn, nn, "", channelColors.find(nn.ToStdString())->second.asWxColor());
            }
            else
            {
                lastnode = new xLightsImportModelNode(laststrand, m->GetName(), sn, nn, "", *wxWHITE);
            }
            laststrand->Insert(lastnode, n);
        }
    }
    if (dynamic_cast<ModelGroup *>(m) != nullptr) {
        ModelGroup *grp = dynamic_cast<ModelGroup *>(m);
        std::vector<Model *> models =  grp->Models();
        for (auto a = models.begin(); a != models.end(); a++) {
            AddModel(*a, ms);
        }
    }
}

void xLightsImportChannelMapDialog::OnItemActivated(wxDataViewEvent& event)
{
}

void xLightsImportChannelMapDialog::OnSelectionChanged(wxDataViewEvent& event)
{
    //if (event.GetItem().IsOk())
    //{
        // I am pretty sure this doesnt do anything
        // What I am trying to do is single click activate the choice box.
        //wxRect r = TreeListCtrl_Mapping->GetItemRect(event.GetItem(), TreeListCtrl_Mapping->GetColumn(1));
        //TreeListCtrl_Mapping->GetColumn(1)->GetRenderer()->ActivateCell(r, dataModel, event.GetItem(), 1, nullptr);

        // This also does not work and causes left behind choice boxes which then generate exceptions
        //TreeListCtrl_Mapping->EditItem(event.GetItem(), TreeListCtrl_Mapping->GetColumn(1));
    //}
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
    dataModel->GetChildren(wxDataViewItem(0), models);
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
                dataModel->GetChildren(models[i], strands);
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
                            dataModel->GetChildren(strands[j], nodes);
                            for (size_t k = 0; k < nodes.size(); k++)
                            {
                                xLightsImportModelNode* anode = (xLightsImportModelNode*)nodes[k].GetID();
                                if (anode->_node == node)
                                {
                                    return nodes[j];
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
    for (size_t i = 0; i < dataModel->GetChildCount(); i++)
    {
        xLightsImportModelNode* m = dataModel->GetNthChild(i);
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
        dataModel->ClearMapping();
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
            if (modelEl == nullptr && xlights->GetModel(model.ToStdString()) != nullptr) {
                mSequenceElements->AddMissingModelsToSequence(model.ToStdString(), false);
                ModelElement *mel = dynamic_cast<ModelElement*>(mSequenceElements->GetElement(model.ToStdString()));
                mel->Init(*xlights->GetModel(model.ToStdString()));
                modelEl = mel;
            }
            //if (modelEl == nullptr) {
            //    modelEl = mSequenceElements->AddElement(model.ToStdString(), "model", false, false, false, false);
            //    modelEl->AddEffectLayer();
            //}
            if (modelEl != nullptr) {
                xLightsImportModelNode* mi = TreeContainsModel(model.ToStdString());
                xLightsImportModelNode* msi = TreeContainsModel(model.ToStdString(), strand.ToStdString());
                xLightsImportModelNode* mni = TreeContainsModel(model.ToStdString(), strand.ToStdString(), node.ToStdString());

                if (mi == nullptr || (msi == nullptr && strand != "") || (mni == nullptr && node != ""))
                {
                    if (!strandwarning)
                    {
                        if (wxMessageBox(model + "/" + strand + "/" + node + " not found.  Has the models changed? Do you want to see future occurences of this error during this import?", "", wxICON_WARNING | wxYES_NO, this) == wxNO)
                        {
                            strandwarning = true;
                        }
                    }
                } else 
                {
                    if (mni != nullptr)
                    {
                        wxDataViewItem item = FindItem(model.ToStdString(), strand.ToStdString(), node.ToStdString());
                        mni->_mapping = mapping;
                        mni->_color = color;
                        dataModel->ValueChanged(item, 1);
                    }
                    else if (msi != nullptr)
                    {
                        wxDataViewItem item = FindItem(model.ToStdString(), strand.ToStdString());
                        msi->_mapping = mapping;
                        msi->_color = color;
                        dataModel->ValueChanged(item, 1);
                    }
                    else
                    {
                        wxDataViewItem item = FindItem(model.ToStdString());
                        mi->_mapping = mapping;
                        mi->_color = color;
                        dataModel->ValueChanged(item, 1);
                    }
                }
            }
            line = text.ReadLine();
        }
        _dirty = false;

        // expand all models that have strands that have a value
        wxDataViewItemArray models;
        dataModel->GetChildren(wxDataViewItem(0), models);
        for (size_t i = 0; i < models.size(); i++)
        {
            wxDataViewItemArray strands;
            dataModel->GetChildren(models[i], strands);
            for (size_t j = 0; j < strands.size(); j++)
            {
                xLightsImportModelNode* astrand = (xLightsImportModelNode*)strands[j].GetID();
                if (astrand->HasMapping())
                {
                    TreeListCtrl_Mapping->Expand(models[i]);
                }
                wxDataViewItemArray nodes;
                dataModel->GetChildren(strands[j], nodes);
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
}

void xLightsImportChannelMapDialog::SaveMapping(wxCommandEvent& event)
{
    wxFileDialog dlg(this, "Save mapping", wxEmptyString, "mapping", "Mapping Files (*.xmap)|*.xmap|All Files (*.)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        wxFileOutputStream output(dlg.GetPath());
        wxTextOutputStream text(output);
        text.WriteString("false\n");
        int modelcount = dataModel->GetMappedChildCount();
        text.WriteString(wxString::Format("%d\n", modelcount));
        for (size_t i = 0; i < dataModel->GetChildCount(); i++)
        {
            xLightsImportModelNode* m = dataModel->GetNthChild(i);
            if (m->HasMapping())
            {
                text.WriteString(m->_model + "\n");
            }
        }
        for (size_t i = 0; i < dataModel->GetChildCount(); i++)
        {
            xLightsImportModelNode* m = dataModel->GetNthChild(i);
            if (m->HasMapping())
            {
                wxString mn = m->_model;
                text.WriteString(mn
                    + "\t" +
                    + "\t" +
                    + "\t" + m->_mapping
                    + "\t" + m->_color.GetAsString()
                    + "\n");
                for (size_t j = 0; j < m->GetChildCount(); j++)
                {
                    xLightsImportModelNode* s = m->GetNthChild(j);
                    if (s->HasMapping())
                    {
                        text.WriteString(mn
                            + "\t" + s->_strand
                            + "\t" +
                            + "\t" + s->_mapping
                            + "\t" + s->_color.GetAsString()
                            + "\n");
                        for (size_t k = 0; k < s->GetChildCount(); k++)
                        {
                            xLightsImportModelNode* n = s->GetNthChild(k);
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

void xLightsImportChannelMapDialog::OnButton_OkClick(wxCommandEvent& event)
{
    if (_dirty)
    {
        if (wxMessageBox("Are you sure you dont want to save your changes for future imports?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxYES)
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
