#include "TestDialog.h"

#include <wx/choicdlg.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>

//(*InternalHeaders(TestDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

const long TestDialog::ID_TREELISTCTRL_Channels = wxNewId();

//(*IdInit(TestDialog)
const long TestDialog::ID_BUTTON_Load = wxNewId();
const long TestDialog::ID_BUTTON_Save = wxNewId();
const long TestDialog::ID_PANEL1 = wxNewId();
const long TestDialog::ID_PANEL3 = wxNewId();
const long TestDialog::ID_PANEL4 = wxNewId();
const long TestDialog::ID_PANEL5 = wxNewId();
const long TestDialog::ID_AUINOTEBOOK1 = wxNewId();
const long TestDialog::ID_PANEL2 = wxNewId();
const long TestDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(TestDialog,wxDialog)
	//(*EventTable(TestDialog)
	//*)
END_EVENT_TABLE()

TestDialog::TestDialog(wxWindow* parent, wxXmlDocument* network, wxFileName networkFile, wxWindowID id)
{
	_network = network;
	_networkFile = networkFile;

	//(*Initialize(TestDialog)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, _("Test Lights"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	SetClientSize(wxSize(1329,450));
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxPoint(0,0), wxSize(1336,456), wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetMinSize(wxSize(10,10));
	SplitterWindow1->SetMinimumPaneSize(10);
	SplitterWindow1->SetSashGravity(0.5);
	Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxPoint(83,262), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Load = new wxButton(Panel1, ID_BUTTON_Load, _("Load"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Load"));
	FlexGridSizer2->Add(Button_Load, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Save = new wxButton(Panel1, ID_BUTTON_Save, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Save"));
	FlexGridSizer2->Add(Button_Save, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(0);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	Panel1->SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(Panel1);
	FlexGridSizer1->SetSizeHints(Panel1);
	Panel2 = new wxPanel(SplitterWindow1, ID_PANEL2, wxPoint(368,72), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(0);
	AuiNotebook1 = new wxAuiNotebook(Panel2, ID_AUINOTEBOOK1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE);
	Panel3 = new wxPanel(AuiNotebook1, ID_PANEL3, wxPoint(203,250), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	Panel4 = new wxPanel(AuiNotebook1, ID_PANEL4, wxPoint(139,1), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
	Panel5 = new wxPanel(AuiNotebook1, ID_PANEL5, wxPoint(163,20), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
	AuiNotebook1->AddPage(Panel3, _("Page name"));
	AuiNotebook1->AddPage(Panel4, wxEmptyString);
	AuiNotebook1->AddPage(Panel5, _("Page name"));
	FlexGridSizer4->Add(AuiNotebook1, 1, wxALL|wxEXPAND, 5);
	Panel2->SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(Panel2);
	FlexGridSizer4->SetSizeHints(Panel2);
	SplitterWindow1->SplitVertically(Panel1, Panel2);
	Center();

	Connect(ID_BUTTON_Load,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TestDialog::OnButton_LoadClick);
	Connect(ID_BUTTON_Save,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TestDialog::OnButton_SaveClick);
	//*)

	SplitterWindow1->SetMinimumPaneSize(100);

	TreeListCtrl_Channels = new wxTreeListCtrl(Panel1, ID_TREELISTCTRL_Channels, wxPoint(0, 0), Panel3->GetSize(), wxTR_FULL_ROW_HIGHLIGHT | wxTR_DEFAULT_STYLE| wxTL_CHECKBOX | wxTL_USER_3STATE, _T("ID_TREELISTCTRL_Channels"));
	TreeListCtrl_Channels->AppendColumn(L"Controller");
	_all = TreeListCtrl_Channels->AppendItem(TreeListCtrl_Channels->GetRootItem(), _("(ALL)"));
	FlexGridSizer3->Add(TreeListCtrl_Channels, 1, wxALL | wxEXPAND, 5);

	// add checkbox events
	Connect(ID_TREELISTCTRL_Channels,wxEVT_COMMAND_CHECKLISTBOX_TOGGLED,(wxObjectEventFunction)&TestDialog::OnTreeListCtrl1Checkboxtoggled);
	Connect(ID_TREELISTCTRL_Channels, wxEVT_COMMAND_TREELIST_ITEM_CHECKED, (wxObjectEventFunction)&TestDialog::OnTreeListCtrl1Checkboxtoggled);
	Connect(ID_TREELISTCTRL_Channels, wxEVT_COMMAND_TREELIST_SELECTION_CHANGED, (wxObjectEventFunction)&TestDialog::OnTreeListCtrl1ItemActivated);
	Connect(ID_TREELISTCTRL_Channels, wxEVT_COMMAND_TREELIST_ITEM_ACTIVATED, (wxObjectEventFunction)&TestDialog::OnTreeListCtrl1ItemActivated);

	PopulateTree(_network);
}

TestDialog::~TestDialog()
{
	// need to delete all the TreeController Objects
	wxTreeListItem root = TreeListCtrl_Channels->GetRootItem();
	DestroyTreeControllerData(root);

	// need to delete the TreeController.
	Panel1->RemoveChild(TreeListCtrl_Channels);
	//delete TreeListCtrl_Channels;

	//(*Destroy(TestDialog)
	//*)
}

std::string TreeController::GenerateName()
{
	_name = "";
	if (_inactive)
	{
		_name += "INACTIVE ";
	}
	switch (_type)
	{
	case CONTROLLERTYPE::CT_NULL:
		_name += "NULL (" + std::string(wxString::Format(wxT("%i"), _nullcount)) + ") ";
		_name += "(" + std::string(wxString::Format(wxT("%i"), _startxlightschannel)) + "-" + std::string(wxString::Format(wxT("%i"), _endxlightschannel)) + ")";
		break;
	case CONTROLLERTYPE::CT_E131:
		_name += "E1.31 " + _ipaddress + " {" + _universe + "} ";
		_name += "[1-" + std::string(wxString::Format(wxT("%i"), _endchannel)) + "] ";
		_name += "(" + std::string(wxString::Format(wxT("%i"), _startxlightschannel)) + "-" + std::string(wxString::Format(wxT("%i"), _endxlightschannel)) + ")";
		break;
	case CONTROLLERTYPE::CT_DMX:
		_name += "DMX " + _comport;
		if (_multiuniversedmx)
		{
			_name += " {" + _universe + "} ";
		}
		_name += "[1-" + std::string(wxString::Format(wxT("%i"), _endchannel)) + "] ";
		_name += "(" + std::string(wxString::Format(wxT("%i"), _startxlightschannel)) + "-" + std::string(wxString::Format(wxT("%i"), _endxlightschannel)) + ")";
		break;
	case CONTROLLERTYPE::CT_ROOT:
		_name += "All ";
		_name += "(" + std::string(wxString::Format(wxT("%i"), _startxlightschannel)) + "-" + std::string(wxString::Format(wxT("%i"), _endxlightschannel)) + ")";
		break;
	case CONTROLLERTYPE::CT_CHANNEL:
		_name += "Channel [" + std::string(wxString::Format(wxT("%i"), _startchannel)) + "] ";
		_name += "(" + std::string(wxString::Format(wxT("%i"), _startxlightschannel)) + ")";
		break;
	}
	if (_description != "")
	{
		_name += " " + _description;
	}

	return _name;
}

// This for for a channel node
TreeController::TreeController(int channel, CONTROLLERTYPE type, int xLightsChannel)
{
	_universes = 0;
	_type = type;
	_startchannel = channel;
	_startxlightschannel = xLightsChannel;
	_inactive = false;
	_name = GenerateName();
}

// This is for a root node
TreeController::TreeController(CONTROLLERTYPE type, int start, int end)
{
	_inactive = false;
	_universes = 0;
	_type = type;
	_startxlightschannel = start;
	_endxlightschannel = end;
	_name = GenerateName();
}

// This is for a DMX multiple node
TreeController::TreeController(CONTROLLERTYPE type, std::string comport, int universe, int startxlightschannel, int channels, bool inactive, bool multiuniversedmx, std::string description)
{
	_universes = 0;
	_type = type;
	_comport = comport;
	_universe = std::string(wxString::Format(wxT("%i"), universe));
	_startchannel = 1;
	_endchannel = channels;
	_startxlightschannel = startxlightschannel;
	_endxlightschannel = _startxlightschannel + channels - 1;
	_inactive = inactive;
	_multiuniversedmx = multiuniversedmx;
	_description = description;
	_name = GenerateName();
}

// this is for a regular node
TreeController::TreeController(wxXmlNode* n, int startchannel, int nullcount)
{
	_universes = 0;
	_startxlightschannel = startchannel + 1;
	wxString MaxChannelsStr = n->GetAttribute("MaxChannels", "0");
	_endchannel = wxAtoi(MaxChannelsStr);
	_startchannel = 1;
	_endxlightschannel = _startxlightschannel + _endchannel - 1;
	_inactive = (n->GetAttribute("Enabled", "Yes") != "Yes");
	_description = n->GetAttribute("Description", "");
	std::string type = std::string(n->GetAttribute("NetworkType", ""));
	if (type == "NULL")
	{
		_type = CONTROLLERTYPE::CT_NULL;
		_nullcount = nullcount;
	}
	else if (type == "E131")
	{
		_type = CONTROLLERTYPE::CT_E131;
		_ipaddress = std::string(n->GetAttribute("ComPort", ""));
		_universe = std::string(n->GetAttribute("BaudRate", ""));
	}
	else if (type == "DMX")
	{
		_type = CONTROLLERTYPE::CT_DMX;
		_comport = std::string(n->GetAttribute("ComPort", ""));
		_universe = n->GetAttribute("BaudRate", "1");
		_universes = wxAtoi(n->GetAttribute("NumUniverses", "1"));
	}
	_name = GenerateName();
}

TreeController* TreeController::GenerateDMXUniverse(int universeoffset)
{
	if (_universes > 1)
	{
		_universes--;
		return new TreeController(_type, _comport, wxAtoi(_universe) + universeoffset, _startxlightschannel + universeoffset * Channels(), Channels(), _inactive, _multiuniversedmx, _description);
	}

	return NULL;
}

void TestDialog::SetSelected(wxTreeListItem& item, wxCheckBoxState state)
{
	wxTreeListItem i = TreeListCtrl_Channels->GetFirstChild(item);
	while (i != NULL)
	{
		TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(i);
		// Dont cascade to inactive or NULL nodes
		if (!tc->IsNULL() && !tc->Inactive())
		{
			TreeListCtrl_Channels->CheckItem(i, state);
			SetSelected(i, state);
		}
		i = TreeListCtrl_Channels->GetNextSibling(i);
	}
}

void TestDialog::PopulateTree(wxXmlDocument* network)
{
	wxXmlNode* e = network->GetRoot();
	long currentcontrollerstartchannel = 0;
	int nullcount = 1;

	for (e = e->GetChildren(); e != NULL; e = e->GetNext())
	{
		if (e->GetName() == "network")
		{
			TreeController* controller = new TreeController(e, currentcontrollerstartchannel, nullcount);
			currentcontrollerstartchannel += controller->Channels();
			if (controller->IsNULL())
			{
				nullcount++;
			}
			wxTreeListItem c = TreeListCtrl_Channels->AppendItem(_all, controller->Name(), -1, -1, (wxClientData*)controller);
			controller->SetTreeListItem(&c);
			if (!controller->IsNULL() && !controller->Inactive())
			{
				for (int i = 1; i <= controller->Channels(); i++)
				{
					TreeController* tc = new TreeController(i, TreeController::CONTROLLERTYPE::CT_CHANNEL, controller->StartXLightsChannel() + i - 1);
					TreeListCtrl_Channels->AppendItem(c, tc->Name(), -1, -1, (wxClientData*)tc);
				}
			}
			else
			{
				//wxWindow* item = TreeListCtrl_Channels->FindItem((long)c.GetID());
				//item->Enable(false);
			}

			int universeoffset = 1;
			TreeController* c2 = controller->GenerateDMXUniverse(universeoffset++);
			while (c2 != NULL)
			{
				currentcontrollerstartchannel += c2->Channels();
				wxTreeListItem c2c = TreeListCtrl_Channels->AppendItem(_all, c2->Name(), -1, -1, (wxClientData*)c2);
				controller->SetTreeListItem(&c2c);
				if (!c2->Inactive())
				{
					for (int i = 1; i <= c2->Channels(); i++)
					{
						TreeController* tc = new TreeController(i, TreeController::CONTROLLERTYPE::CT_CHANNEL, c2->StartXLightsChannel() + i - 1);
						TreeListCtrl_Channels->AppendItem(c2c, tc->Name(), -1, -1, (wxClientData*)tc);
					}
				}
				else
				{
					// to disable those items that should be disabled
					//wxWindow* item = TreeListCtrl_Channels->FindItem((long)(c2c.GetID()));
					//item->Enable(false);
				}
				c2 = controller->GenerateDMXUniverse(universeoffset++);
			}
		}
		TreeListCtrl_Channels->Expand(_all);
	}

	TreeController* root = new TreeController(TreeController::CONTROLLERTYPE::CT_ROOT, 1, currentcontrollerstartchannel);
	TreeListCtrl_Channels->SetItemData(_all, (wxClientData*)root);
	TreeListCtrl_Channels->SetItemText(_all, root->Name());
}

void TestDialog::DestroyTreeControllerData(wxTreeListItem& item)
{
	TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(item);
	if (tc != NULL)
	{
		delete tc;
		//TreeListCtrl_Channels->SetItemData(item, NULL);
	}

	wxTreeListItem i = TreeListCtrl_Channels->GetFirstChild(item);
	while (i != NULL)
	{
		DestroyTreeControllerData(i);
		i = TreeListCtrl_Channels->GetNextSibling(i);
	}
}

std::list<std::string> TestDialog::GetModelsOnChannels(int start, int end)
{
	std::list<std::string> res;

	res.push_back("Test");

	return res;
}

void TestDialog::OnTreeListCtrl1ItemActivated(wxTreeListEvent& event)
{
	if (event.GetItem() == _all)
	{
		// dont do anything
		TreeListCtrl_Channels->SetToolTip("");
	}
	else
	{
		TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(event.GetItem());
		std::list<std::string> models = GetModelsOnChannels(tc->StartXLightsChannel(), tc->EndXLightsChannel());
		std::string tt = "";
		for (std::list<std::string>::iterator it = models.begin(); it != models.end(); ++it)
		{
			if (tt != "")
			{
				tt += "\n";
			}
			tt = tt + *it;
		}
		TreeListCtrl_Channels->SetToolTip(tt);
	}
}
void TestDialog::OnTreeListCtrl1Checkboxtoggled(wxTreeListEvent& event)
{
	wxTreeListItem item = event.GetItem();

	TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(item);

	// You cannot check these items
	if (tc->IsNULL() || tc->Inactive())
	{
		TreeListCtrl_Channels->CheckItem(item, wxCHK_UNCHECKED);
		wxBell();
		return;
	}

	wxCheckBoxState checked = TreeListCtrl_Channels->GetCheckedState(item);

	if (checked == wxCheckBoxState::wxCHK_UNDETERMINED)
	{
		TreeListCtrl_Channels->CheckItem(item, wxCHK_UNCHECKED);
		checked = wxCheckBoxState::wxCHK_UNCHECKED;
	}

	if (checked == wxCheckBoxState::wxCHK_CHECKED)
	{
		SetSelected(item, wxCheckBoxState::wxCHK_CHECKED);
	}
	else if (checked == wxCheckBoxState::wxCHK_UNCHECKED)
	{
		SetSelected(item, wxCheckBoxState::wxCHK_UNCHECKED);
	}
	UpdateSelectedState(TreeListCtrl_Channels->GetItemParent(item), checked);
}

void TestDialog::UpdateSelectedState(wxTreeListItem& item, wxCheckBoxState state)
{
	if (item != NULL)
	{
		wxCheckBoxState cbs = state;

		if (cbs == wxCHK_UNDETERMINED)
		{
			TreeListCtrl_Channels->CheckItem(item, cbs);
			return;
		}

		wxTreeListItem i = TreeListCtrl_Channels->GetFirstChild(item);
		while (i != NULL)
		{
			TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(i);

			// ignore nodes that cant be clicked on
			if (!tc->IsNULL() && !tc->Inactive())
			{
				wxCheckBoxState ncbs = TreeListCtrl_Channels->GetCheckedState(i);
				if (ncbs != cbs)
				{
					cbs = wxCHK_UNDETERMINED;
					break;
				}
			}
			i = TreeListCtrl_Channels->GetNextSibling(i);
		}
		TreeListCtrl_Channels->CheckItem(item, cbs);

		wxTreeListItem p = TreeListCtrl_Channels->GetItemParent(item);
		UpdateSelectedState(p, cbs);
	}
}

// get list of test config names
void TestDialog::GetTestPresetNames(wxArrayString& PresetNames)
{
	wxString name;
	wxXmlNode* root = _network->GetRoot();
	if (!root) return;
	for (wxXmlNode* e = root->GetChildren(); e != NULL; e = e->GetNext())
	{
		if (e->GetName() == "testpreset")
		{
			name = e->GetAttribute("name", "");
			if (!name.IsEmpty()) PresetNames.Add(name);
		}
	}
}

// Find a given xlightschannel in the treelist and check it if it is not inactive or in a null controller
void TestDialog::CheckChannel(long chid)
{
	wxTreeListItem c = TreeListCtrl_Channels->GetFirstChild(_all);

	while (c != NULL)
	{
		TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(c);
		if (chid >= tc->StartXLightsChannel() && chid <= tc->EndXLightsChannel())
		{
			// skip if inactive or a NULL controller
			if (!tc->IsNULL() && !tc->Inactive())
			{
				wxTreeListItem cc = TreeListCtrl_Channels->GetFirstChild(c);
				while (cc != NULL)
				{
					TreeController* tcc = (TreeController*)TreeListCtrl_Channels->GetItemData(cc);
					if (chid == tcc->StartXLightsChannel())
					{
						TreeListCtrl_Channels->CheckItem(cc, wxCHK_CHECKED);
						UpdateSelectedState(cc, wxCHK_CHECKED);
						break;
					}
					cc = TreeListCtrl_Channels->GetNextSibling(cc);
				}
			}
			break;
		}
		c = TreeListCtrl_Channels->GetNextSibling(c);
	}
}

void TestDialog::OnButton_LoadClick(wxCommandEvent& event)
{
	wxArrayString PresetNames;
	GetTestPresetNames(PresetNames);
	if (PresetNames.Count() == 0)
	{
		wxMessageBox(_("No test configurations found"), _("Error"));
		return;
	}

	// get user selection
	PresetNames.Sort();
	wxSingleChoiceDialog dialog(this, _("Select test configuration"), _("Load Test Settings"), PresetNames);
	if (dialog.ShowModal() != wxID_OK) return;

	// re-find testpreset node, then set channels
	UpdateSelectedState(TreeListCtrl_Channels->GetRootItem(), wxCHK_UNCHECKED);
	//SetTestCheckboxes(false);
	wxString name = dialog.GetStringSelection();
	wxString chidstr;
	long chid;
	TreeController* rootcb = (TreeController*)TreeListCtrl_Channels->GetItemData(_all);
	long ChCount = rootcb->EndXLightsChannel();
//	long ChCount = CheckListBoxTestChannels->GetCount();
	wxXmlNode* root = _network->GetRoot();
	for (wxXmlNode* e = root->GetChildren(); e != NULL; e = e->GetNext())
	{
		if (e->GetName() == "testpreset" && e->GetAttribute("name", "") == name)
		{
			for (wxXmlNode* c = e->GetChildren(); c != NULL; c = c->GetNext())
			{
				if (c->GetName() == "channel" && c->GetAttribute("id", &chidstr) && chidstr.ToLong(&chid) && chid >= 0 && chid < ChCount)
				{
					CheckChannel(chid);
					//CheckListBoxTestChannels->Check(chid, true);
				}
			}
			break;
		}
	}
}

void TestDialog::OnButton_SaveClick(wxCommandEvent& event)
{
	wxString name;
	wxXmlNode *channel, *PresetNode;
	wxArrayString PresetNames;
	GetTestPresetNames(PresetNames);
	wxXmlNode* root = _network->GetRoot();
	wxTextEntryDialog NameDialog(this, _("Enter a name for this test configuration"), _("Save Test Settings"));
	if (NameDialog.ShowModal() != wxID_OK) return;
	name = NameDialog.GetValue();
	name.Trim(true);
	name.Trim(false);
	if (name.IsEmpty())
	{
		wxMessageBox(_("Name cannot be empty"), _("Error"));
	}
	else if (name.Len() > 240)
	{
		wxMessageBox(_("Name is too long"), _("Error"));
	}
	else if (name.Find('"') != wxNOT_FOUND)
	{
		wxMessageBox(_("Name cannot contain quotes"), _("Error"));
	}
	else if (PresetNames.Index(name, false) != wxNOT_FOUND)
	{
		wxMessageBox(_("Name already exists, please enter a unique name"), _("Error"));
	}
	else
	{
		PresetNode = new wxXmlNode(wxXML_ELEMENT_NODE, "testpreset");
		PresetNode->AddAttribute("name", name);
		root->AddChild(PresetNode);
		TreeController* rootcb = (TreeController*)TreeListCtrl_Channels->GetItemData(_all);
		long ChCount = rootcb->EndXLightsChannel();
		//int ChCount = CheckListBoxTestChannels->GetCount();

		wxTreeListItem c = TreeListCtrl_Channels->GetFirstChild(_all);
		while (c != NULL)
		{
			TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(c);
			if (!tc->IsNULL() && !tc->Inactive())
			{
				wxTreeListItem cc = TreeListCtrl_Channels->GetFirstChild(c);
				while (cc != NULL)
				{
					if (TreeListCtrl_Channels->GetCheckedState(cc) == wxCHK_CHECKED)
					{
						TreeController* tcc = (TreeController*)TreeListCtrl_Channels->GetItemData(cc);

						channel = new wxXmlNode(wxXML_ELEMENT_NODE, "channel");
						channel->AddAttribute("id", wxString::Format("%d", tcc->StartXLightsChannel()));
						PresetNode->AddChild(channel);
					}
					cc = TreeListCtrl_Channels->GetNextSibling(cc);
				}
			}
			c = TreeListCtrl_Channels->GetNextSibling(c);
		}
		//for (int c = 0; c < ChCount; c++)
		//{
		//	if (CheckListBoxTestChannels->IsChecked(c))
		//	{
		//		channel = new wxXmlNode(wxXML_ELEMENT_NODE, "channel");
		//		channel->AddAttribute("id", wxString::Format("%d", c));
		//		PresetNode->AddChild(channel);
		//	}
		//}

		if (_network->Save(_networkFile.GetFullPath()))
		{
			//UnsavedNetworkChanges = false;
		}
		else
		{
			wxMessageBox(_("Unable to save network definition file"), _("Error"));
		}
	}
}