#include "xLightsImportChannelMapDialog.h"
#include "sequencer/SequenceElements.h"
#include "xLightsMain.h"
#include "models/Model.h"

#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>

//(*InternalHeaders(xLightsImportChannelMapDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

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

wxString xLightsImportTreeModel::GetMapping(const wxDataViewItem &item) const
{
    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();
    if (!node)      // happens if item.IsOk()==false
        return wxEmptyString;

    return node->_mapping;
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
    if (node->GetParent() != NULL)
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
        if (parent == NULL)
        {
            variant = wxVariant(wxDataViewIconText(node->_model));
        }
        else
        {
            variant = wxVariant(wxDataViewIconText(node->_strand));
        }
        break;
    case 1:
        variant = wxVariant(wxDataViewIconText(node->_mapping));
        break;
    default:
        wxLogError("xLightsImportTreeModel::GetValue: wrong column %d", col);
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
    return false;
}

wxDataViewItem xLightsImportTreeModel::GetParent(const wxDataViewItem &item) const
{
    // the invisible root node has no parent
    if (!item.IsOk())
        return wxDataViewItem(0);

    xLightsImportModelNode *node = (xLightsImportModelNode*)item.GetID();

    if (node->GetParent() == NULL)
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

const long xLightsImportChannelMapDialog::ID_TREELISTCTRL1 = wxNewId();

//(*IdInit(xLightsImportChannelMapDialog)
const long xLightsImportChannelMapDialog::ID_BUTTON3 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON4 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(xLightsImportChannelMapDialog,wxDialog)
	//(*EventTable(xLightsImportChannelMapDialog)
	//*)
END_EVENT_TABLE()

xLightsImportChannelMapDialog::xLightsImportChannelMapDialog(wxWindow* parent, const wxFileName &filename, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _filename = filename;

	//(*Initialize(xLightsImportChannelMapDialog)
	wxButton* Button01;
	wxButton* Button02;

	Create(parent, wxID_ANY, _("Map Channels"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("wxID_ANY"));
	SetMaxSize(wxDLG_UNIT(parent,wxSize(-1,500)));
	Sizer = new wxFlexGridSizer(0, 1, 0, 0);
	Sizer->AddGrowableCol(0);
	Sizer->AddGrowableRow(0);
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
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button01 = new wxButton(this, ID_BUTTON1, _("Load Mapping"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button01, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button02 = new wxButton(this, ID_BUTTON2, _("Save Mapping"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button02, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Sizer->Add(FlexGridSizer2, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
	SetSizer(Sizer);
	Sizer->Fit(this);
	Sizer->SetSizeHints(this);

	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsImportChannelMapDialog::OnButton_OkClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsImportChannelMapDialog::OnButton_CancelClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsImportChannelMapDialog::LoadMapping);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsImportChannelMapDialog::SaveMapping);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&xLightsImportChannelMapDialog::OnResize);
	//*)

    dataModel = new xLightsImportTreeModel();

    TreeListCtrl_Mapping = new wxDataViewTreeCtrl(this, ID_TREELISTCTRL1, wxDefaultPosition, wxDefaultSize, wxDV_ROW_LINES, wxDefaultValidator);
    TreeListCtrl_Mapping->AssociateModel(dataModel);
    //TreeListCtrl_Mapping->AppendColumn(new wxDataViewColumn("Model", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT), 0, 80, wxALIGN_LEFT));
    //TreeListCtrl_Mapping->AppendColumn(new wxDataViewColumn("Map To", new wxDataViewChoiceRenderer(_importModels, wxDATAVIEW_CELL_EDITABLE, wxALIGN_LEFT), 1, 80, wxALIGN_LEFT));
    TreeListCtrl_Mapping->SetMinSize(wxSize(0, 300));
    SizerMap->Add(TreeListCtrl_Mapping, 1, wxALL | wxEXPAND, 5);
    SizerMap->Layout();
    SetSize(500, 500);
    Layout();

    //Connect(ID_TREELISTCTRL1, wxEVT_COMMAND_TREELIST_SELECTION_CHANGED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnTreeListCtrlItemActivated);
    //Connect(ID_TREELISTCTRL1, wxEVT_COMMAND_TREELIST_ITEM_ACTIVATED, (wxObjectEventFunction)&xLightsImportChannelMapDialog::OnTreeListCtrlItemActivated);

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
    delete dataModel;
}

static wxArrayString Convert(const std::vector<std::string> arr) {
    wxArrayString ret;
    for (auto it = arr.begin(); it != arr.end(); it++) {
        ret.push_back(*it);
    }
    return ret;
}

bool xLightsImportChannelMapDialog::Init() {
    if (channelNames.size() == 0)
    {
        wxMessageBox("No models to import to. Add some models to the rows of the effects grid.");
        return false;
    }

    // load the tree
    for (auto it = channelNames.begin(); it != channelNames.end(); it++)
    {
        if ((*it).find('/') != std::string::npos)
        {
        }
        else
        {
            _importModels.push_back(*it);
        }
    }

    xLightsImportModelNode* last;
    int ms = 0;
    for (size_t i = 0; i<mSequenceElements->GetElementCount(); i++) {
        if (mSequenceElements->GetElement(i)->GetType() == "model") {
            Element* e = mSequenceElements->GetElement(i);
            last = new xLightsImportModelNode(NULL, e->GetName(), "");
            dataModel->Insert(last, ms++);
            Model *m = xlights->GetModel(e->GetName());
            for (int s = 0; s < m->GetNumStrands(); s++) {
                wxString sn = m->GetStrandName(s);
                if ("" == sn) {
                    sn = wxString::Format("Strand %d", s + 1);
                }
                last->Insert(new xLightsImportModelNode(last, e->GetName(), sn, ""), s);
            }
        }
    }

    return true;
}

//void xLightsImportChannelMapDialog::OnChannelMapGridCellChange(wxGridEvent& event)
//{
//    _dirty = true;
//    int row = event.GetRow();
//    int col = event.GetCol();
//    std::string s = ChannelMapGrid->GetCellValue(row, col).ToStdString();
//    ChannelMapGrid->Refresh();
//}

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

int CountTabs(wxString& line)
{
    int count = 0;
    for (size_t x = 0; x < line.size(); x++) {
        if (line[x] == '\t') {
            count++;
        }
    }
    return count;
}

xLightsImportModelNode* xLightsImportChannelMapDialog::TreeContainsModel(std::string model, std::string strand)
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
                        return s;
                    }
                }
            }
        }
    }
    return NULL;
}

void xLightsImportChannelMapDialog::LoadMapping(wxCommandEvent& event)
{
    bool strandwarning = false;
    if (_dirty)
    {
        if (wxMessageBox("Are you sure you want to lose your changes?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO)
        {
            return;
        }
    }

    wxFileDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK) {
        wxFileInputStream input(dlg.GetPath());
        wxTextInputStream text(input, "\t");
        text.ReadLine(); // map by strand ... ignore this
        int count = wxAtoi(text.ReadLine());
        modelNames.clear();
        for (int x = 0; x < count; x++) {
            std::string mn = text.ReadLine().ToStdString();
            if (TreeContainsModel(mn) == NULL) {
                wxMessageBox("Model " + mn + " not part of sequence.  Not mapping channels to this model.", "", wxICON_WARNING | wxOK , this);
            } else {
                modelNames.push_back(mn);
            }
        }
        wxString line = text.ReadLine();
        while (line != "") {
            wxString model;
            wxString strand;
            wxString node;
            wxString mapping;
            xlColor color;

            if (CountTabs(line) == 4)
            {
                model = FindTab(line);
                strand = FindTab(line);
                node = FindTab(line);
                mapping = FindTab(line);
                color = xlColor(FindTab(line));
            }
            else
            {
                model = FindTab(line);
                strand = FindTab(line);
                mapping = FindTab(line);
            }
            Element *modelEl = mSequenceElements->GetElement(model.ToStdString());
            //if (modelEl == nullptr) {
            //    modelEl = mSequenceElements->AddElement(model.ToStdString(), "model", false, false, false, false);
            //    modelEl->AddEffectLayer();
            //}
            if (modelEl != nullptr) {
                xLightsImportModelNode* mi = TreeContainsModel(model.ToStdString());
                xLightsImportModelNode* msi = TreeContainsModel(model.ToStdString(), strand.ToStdString());

                if (mi == NULL || (msi == NULL && strand != ""))
                {
                    if (!strandwarning)
                    {
                        if (wxMessageBox(model + "/" + strand + " not found.  Has the models changed? Do you want to see future occurences of this error during this import?", "", wxICON_WARNING | wxYES_NO, this) == wxID_NO)
                        {
                            strandwarning = true;
                        }
                    }
                } else 
                {
                    if (msi != NULL)
                    {
                        msi->_mapping = mapping;
                    }
                    else
                    {
                        mi->_mapping = mapping;
                    }
                }
            }
            line = text.ReadLine();
        }
        _dirty = false;
    }
}

void xLightsImportChannelMapDialog::SaveMapping(wxCommandEvent& event)
{
    wxFileDialog dlg(this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        wxFileOutputStream output(dlg.GetPath());
        wxTextOutputStream text(output);
        text.WriteString("false\n");
        int modelcount = dataModel->GetChildCount();
        text.WriteString(wxString::Format("%d\n", modelcount));
        for (size_t i = 0; i < dataModel->GetChildCount(); i++)
        {
            xLightsImportModelNode* m = dataModel->GetNthChild(i);
            text.WriteString(m->_model + "\n");
        }
        for (size_t i = 0; i < dataModel->GetChildCount(); i++)
        {
            xLightsImportModelNode* m = dataModel->GetNthChild(i);
            wxString mn = m->_model;
            text.WriteString(mn
                + "\t" +
                +"\t" + m->_mapping
                + "\n");
            for (size_t j = 0; j < m->GetChildCount(); j++)
            {
                xLightsImportModelNode* s = m->GetNthChild(j);
                text.WriteString(mn
                    + "\t" + s->_strand
                    + "\t" + s->_mapping
                    + "\n");
            }
        }
        _dirty = false;
    }
}

void xLightsImportChannelMapDialog::OnResize(wxSizeEvent& event)
{
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
}

//void xLightsImportChannelMapDialog::OnChannelMapGridCellLeftClick(wxGridEvent& event)
//{
//    ChannelMapGrid->SetGridCursor(event.GetRow(), event.GetCol());
//    event.Skip();
//}

void xLightsImportChannelMapDialog::OnButton_OkClick(wxCommandEvent& event)
{
    if (_dirty)
    {
        if (wxMessageBox("Are you sure you want to lose your changes?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxYES)
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
        if (wxMessageBox("Are you sure you want to lose your changes?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxYES)
        {
            EndDialog(wxID_CANCEL);
        }

    }
    else
    {
            EndDialog(wxID_CANCEL);
    }
}
