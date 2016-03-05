#include "TestDialog.h"

//(*InternalHeaders(TestDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

const long TestDialog::ID_TREELISTCTRL_Channels = wxNewId();

//(*IdInit(TestDialog)
const long TestDialog::ID_AUINOTEBOOK1 = wxNewId();
const long TestDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(TestDialog,wxDialog)
	//(*EventTable(TestDialog)
	//*)
END_EVENT_TABLE()

TestDialog::TestDialog(wxWindow* parent, wxXmlDocument* network, wxWindowID id)
{
	_network = network;

	//(*Initialize(TestDialog)
	Create(parent, wxID_ANY, _("Test Lights"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	SetClientSize(wxSize(1329,450));
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxSize(300,300), wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetMinimumPaneSize(10);
	SplitterWindow1->SetSashGravity(0.5);
	AuiNotebook1 = new wxAuiNotebook(SplitterWindow1, ID_AUINOTEBOOK1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE);
	SplitterWindow1->Initialize(AuiNotebook1);
	Center();
	//*)

	TreeListCtrl_Channels = new wxTreeListCtrl(SplitterWindow1, ID_TREELISTCTRL_Channels, wxPoint(149, 370), wxDefaultSize, wxTR_FULL_ROW_HIGHLIGHT | wxTR_DEFAULT_STYLE| wxTL_CHECKBOX | wxTL_USER_3STATE, _T("ID_TREELISTCTRL_Channels"));
	TreeListCtrl_Channels->AppendColumn(L"Controller");
	_all = TreeListCtrl_Channels->AppendItem(TreeListCtrl_Channels->GetRootItem(), _("(ALL)"));

	SplitterWindow1->SplitVertically(TreeListCtrl_Channels, AuiNotebook1);
	SplitterWindow1->SetBestFittingSize();
	AuiNotebook1->SetFitToCurrentPage(true);
	TreeListCtrl_Channels->Fit();

	// add checkbox events
	Connect(ID_TREELISTCTRL_Channels,wxEVT_COMMAND_CHECKLISTBOX_TOGGLED,(wxObjectEventFunction)&TestDialog::OnTreeListCtrl1Checkboxtoggled);
	Connect(ID_TREELISTCTRL_Channels, wxEVT_COMMAND_TREELIST_ITEM_CHECKED, (wxObjectEventFunction)&TestDialog::OnTreeListCtrl1Checkboxtoggled);
	Connect(ID_TREELISTCTRL_Channels, wxEVT_COMMAND_TREE_ITEM_GETTOOLTIP, (wxObjectEventFunction)&TestDialog::OnTreeListCtrl1ItemGetToolTip);

	PopulateTree(_network);
}

std::string TreeController::GenerateName()
{
	_name = "";
	if (_inactive)
	{
		_name += "INACTIVE ";
	}
	if (_description != "")
	{
		_name += _description + " ";
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
			_name += " {" + std::string(wxString::Format(wxT("%i"), _universe)) + "} ";
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
	_universes = 0;
	_type = type;
	_startxlightschannel = start;
	_endxlightschannel = end;
	_name = GenerateName();
	_inactive = false;
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
		TreeListCtrl_Channels->CheckItem(i, state);
		SetSelected(i, state);
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

TestDialog::~TestDialog()
{
	// need to delete all the TreeController Objects
	wxTreeListItem root = TreeListCtrl_Channels->GetRootItem();
	DestroyTreeControllerData(root);

	// need to delete the TreeController.
	SplitterWindow1->RemoveChild(TreeListCtrl_Channels);
	//delete TreeListCtrl_Channels;

	//(*Destroy(TestDialog)
	//*)
}

void TestDialog::OnTreeListCtrl1Checkboxtoggled(wxTreeListEvent& event)
{
	wxTreeListItem item = event.GetItem();

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
			wxCheckBoxState ncbs = TreeListCtrl_Channels->GetCheckedState(i);
			if (ncbs != cbs)
			{
				cbs = wxCHK_UNDETERMINED;
				break;
			}
			i = TreeListCtrl_Channels->GetNextSibling(i);
		}
		TreeListCtrl_Channels->CheckItem(item, cbs);

		wxTreeListItem p = TreeListCtrl_Channels->GetItemParent(item);
		UpdateSelectedState(p, cbs);
	}
}

void TestDialog::OnTreeListCtrl1ItemGetToolTip(wxTreeEvent& event)
{
	int a = 0;
	//wxTreeListItem item  = event.GetItem();
}
