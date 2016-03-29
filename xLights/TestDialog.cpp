#include "TestDialog.h"

#include <wx/choicdlg.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/settings.h>

//(*InternalHeaders(TestDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "models/Model.h"
#include "models/ModelGroup.h"

//
//
// Tree Controller Class Functions
//
//
//

// Constructors

// This for for a channel node
TreeController::TreeController(int channel, CONTROLLERTYPE type, int xLightsChannel)
{
	_colour = ' ';
	_universes = 0;
	_type = type;
	_startchannel = channel;
	_startxlightschannel = xLightsChannel;
	_inactive = false;
	_nodes = -1;
	_doesNotExist = false;
	_name = GenerateName();
}

// This for for a node node
TreeController::TreeController(CONTROLLERTYPE type, int xLightsChannel, int node, int channelspernode)
{
	_colour = ' ';
	_universes = 0;
	_type = type;
	_nodeNumber = node;
	_startchannel = -1;
	_endchannel = -1;
	_startxlightschannel = xLightsChannel;
	_endxlightschannel = xLightsChannel + channelspernode - 1;
	_inactive = false;
	_nodes = -1;
	_doesNotExist = false;
	_name = GenerateName();
}

// model or model group
TreeController::TreeController(CONTROLLERTYPE type, std::string name)
{
	_colour = ' ';
	_inactive = false;
	_universes = 0;
	_type = type;
	_startxlightschannel = -1;
	_endxlightschannel = -1;
	_startchannel = -1;
	_endchannel = -1;
	_modelName = name;
	_doesNotExist = false;
	_nodes = -1;
	_name = GenerateName();
}

// This is for a root node
TreeController::TreeController(CONTROLLERTYPE type, int start, int end)
{
	_colour = ' ';
	_inactive = false;
	_universes = 0;
	_type = type;
	_startxlightschannel = start;
	_endxlightschannel = end;
	_nodes = -1;
	_doesNotExist = false;
	_name = GenerateName();
}

// This is for a DMX multiple node
TreeController::TreeController(CONTROLLERTYPE type, std::string comport, int universe, int startxlightschannel, int channels, bool inactive, bool multiuniversedmx, std::string description)
{
	_colour = ' ';
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
	_nodes = -1;
	_doesNotExist = false;
	_name = GenerateName();
}

// this is for a regular node
TreeController::TreeController(wxXmlNode* n, int startchannel, int nullcount)
{
	_colour = ' ';
	_doesNotExist = false;
	_nodes = -1;
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
	else if (type == "LOR")
	{
		_type = CONTROLLERTYPE::CT_LOR;
		_comport = std::string(n->GetAttribute("ComPort", ""));
		_baudrate = n->GetAttribute("BaudRate", "1");
	}
	_name = GenerateName();
}

// Checks if the nominated channel is within the range of this element
bool TreeController::ContainsChannel(int ch)
{
	if (_startxlightschannel < 1)
	{
		return false;
	}
	else if (_endxlightschannel < 1)
	{
		return ch == _startxlightschannel;
	}
	else
	{
		return (ch >= _startxlightschannel && ch <= _endxlightschannel);
	}
}

// This generates 2nd & subsequent universes for a DMX controller
TreeController* TreeController::GenerateDMXUniverse(int universeoffset)
{
	if (_universes > 1)
	{
		_universes--;
		return new TreeController(_type, _comport, wxAtoi(_universe) + universeoffset, _startxlightschannel + universeoffset * Channels(), Channels(), _inactive, _multiuniversedmx, _description);
	}

	return NULL;
}

// Generate the text representation to display in the tree
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
	case CONTROLLERTYPE::CT_LOR:
		_name += "LOR " + _comport + " at " + _baudrate + " ";
		_name += "[1-" + std::string(wxString::Format(wxT("%i"), _endchannel)) + "] ";
		_name += "(" + std::string(wxString::Format(wxT("%i"), _startxlightschannel)) + "-" + std::string(wxString::Format(wxT("%i"), _endxlightschannel)) + ")";
		break;
	case CONTROLLERTYPE::CT_MODEL:
		if (!Clickable())
		{
			_name += "UNAVAILABLE ";
		}
		_name += _modelName;
		if (_nodes > 0)
		{
			_name += " [1-" + std::string(wxString::Format(wxT("%i"), _nodes)) + "]";
		}
		if (_startxlightschannel < 1)
		{
			// dont add anything
		}
		else if (_endxlightschannel < _startxlightschannel)
		{
			_name += " (" + std::string(wxString::Format(wxT("%i"), _startxlightschannel)) + ")";
		}
		else
		{
			_name += " (" + std::string(wxString::Format(wxT("%i"), _startxlightschannel)) + "-" + std::string(wxString::Format(wxT("%i"), _endxlightschannel)) + ")";
		}
		break;
	case CONTROLLERTYPE::CT_MODELGROUP:
		_name += _modelName;
		if (_startxlightschannel < 1)
		{
			// dont add anything
		}
		else if (_endxlightschannel < _startxlightschannel)
		{
			_name += " (" + std::string(wxString::Format(wxT("%i"), _startxlightschannel)) + ")";
		}
		else
		{
			_name += " (" + std::string(wxString::Format(wxT("%i"), _startxlightschannel)) + "-" + std::string(wxString::Format(wxT("%i"), _endxlightschannel)) + ")";
		}
		break;
	case CONTROLLERTYPE::CT_CONTROLLERROOT:
		_name += "Controllers ";
		if (_endxlightschannel >= _startxlightschannel)
		{
			_name += "(" + std::string(wxString::Format(wxT("%i"), _startxlightschannel)) + "-" + std::string(wxString::Format(wxT("%i"), _endxlightschannel)) + ")";
		}
		break;
	case CONTROLLERTYPE::CT_MODELGROUPROOT:
		_name += "Model Groups";
		break;
	case CONTROLLERTYPE::CT_MODELROOT:
		_name += "Models ";
		if (_endxlightschannel >= _startxlightschannel)
		{
			_name += "(" + std::string(wxString::Format(wxT("%i"), _startxlightschannel)) + "-" + std::string(wxString::Format(wxT("%i"), _endxlightschannel)) + ")";
		}
		break;
	case CONTROLLERTYPE::CT_NODE:
		_name += "Node {" + std::string(wxString::Format(wxT("%i"), _nodeNumber)) + "} ";
		_name += "(" + std::string(wxString::Format(wxT("%i"), _startxlightschannel)) + "-" + std::string(wxString::Format(wxT("%i"), _endxlightschannel)) + ")";
		break;
	case CONTROLLERTYPE::CT_CHANNEL:
		_name += "Channel ";
		if (_colour != ' ')
		{
			_name += "{";
			_name += _colour;
			_name += "} ";
		}
		if (_startchannel >= 0)
		{
			_name += "[" + std::string(wxString::Format(wxT("%i"), _startchannel)) + "] ";
		}
		_name += "(" + std::string(wxString::Format(wxT("%i"), _startxlightschannel)) + ")";
		break;
	}
	if (_description != "")
	{
		_name += " " + _description;
	}

	return _name;
}

// Test Dialog Methods

const long TestDialog::ID_TREELISTCTRL_Channels = wxNewId();

//(*IdInit(TestDialog)
const long TestDialog::ID_BUTTON_Load = wxNewId();
const long TestDialog::ID_BUTTON_Save = wxNewId();
const long TestDialog::ID_PANEL1 = wxNewId();
const long TestDialog::ID_CHECKBOX_OutputToLights = wxNewId();
const long TestDialog::ID_STATICTEXT2 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_Standard_Off = wxNewId();
const long TestDialog::ID_RADIOBUTTON_Standard_Chase = wxNewId();
const long TestDialog::ID_RADIOBUTTON_Standard_Chase13 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_Standard_Chase14 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_Standard_Chase15 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_Standard_Alternate = wxNewId();
const long TestDialog::ID_RADIOBUTTON_Standard_Twinke5 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_Standard_Twinkle10 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_Standard_Twinkle25 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_Standard_Twinkle50 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_Standard_Shimmer = wxNewId();
const long TestDialog::ID_RADIOBUTTON_Standard_Background = wxNewId();
const long TestDialog::ID_STATICTEXT3 = wxNewId();
const long TestDialog::ID_SLIDER_Standard_Background = wxNewId();
const long TestDialog::ID_STATICTEXT4 = wxNewId();
const long TestDialog::ID_SLIDER_Standard_Highlight = wxNewId();
const long TestDialog::ID_PANEL_Standard = wxNewId();
const long TestDialog::ID_STATICTEXT5 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGB_Off = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGB_Chase = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGB_Chase13 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGB_Chase14 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGB_Chase15 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGB_Alternate = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGB_Twinkle5 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGB_Twinkle10 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGB_Twinkle25 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGB_Twinkle50 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGB_Shimmer = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGB_Background = wxNewId();
const long TestDialog::ID_SLIDER1 = wxNewId();
const long TestDialog::ID_SLIDER2 = wxNewId();
const long TestDialog::ID_SLIDER3 = wxNewId();
const long TestDialog::ID_SLIDER4 = wxNewId();
const long TestDialog::ID_SLIDER5 = wxNewId();
const long TestDialog::ID_SLIDER6 = wxNewId();
const long TestDialog::ID_PANEL4 = wxNewId();
const long TestDialog::ID_STATICTEXT6 = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGBCycle_Off = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGBCycle_ABC = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGBCycle_ABCAll = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGBCycle_ABCAllNone = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGBCycle_MixedColors = wxNewId();
const long TestDialog::ID_RADIOBUTTON_RGBCycle_RGBW = wxNewId();
const long TestDialog::ID_PANEL5 = wxNewId();
const long TestDialog::ID_AUINOTEBOOK1 = wxNewId();
const long TestDialog::ID_STATICTEXT1 = wxNewId();
const long TestDialog::ID_SLIDER_Speed = wxNewId();
const long TestDialog::ID_PANEL2 = wxNewId();
const long TestDialog::ID_SPLITTERWINDOW1 = wxNewId();
const long TestDialog::ID_STATICTEXT7 = wxNewId();
const long TestDialog::ID_TIMER1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(TestDialog,wxDialog)
	//(*EventTable(TestDialog)
	//*)
END_EVENT_TABLE()

// Constructor

TestDialog::TestDialog(wxWindow* parent, wxXmlDocument* network, wxFileName networkFile, ModelManager* modelManager, wxWindowID id)
{
	_network = network;
	_networkFile = networkFile;
	_modelManager = modelManager;
	_checkChannelList = false;
	_xout = NULL;
	_cascading = false;

	//(*Initialize(TestDialog)
	wxBoxSizer* BoxSizer4;
	wxStaticBoxSizer* StaticBoxSizer2;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer9;
	wxFlexGridSizer* FlexGridSizer2;
	wxBoxSizer* BoxSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer14;
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer13;
	wxFlexGridSizer* FlexGridSizer12;
	wxFlexGridSizer* FlexGridSizer6;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer11;
	wxBoxSizer* BoxSizer3;

	Create(parent, wxID_ANY, _("Test Lights"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	SetClientSize(wxSize(300,200));
	SetMinSize(wxSize(300,200));
	FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxSize(283,125), wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetMinSize(wxSize(10,10));
	SplitterWindow1->SetMinimumPaneSize(10);
	SplitterWindow1->SetSashGravity(0.5);
	Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxPoint(95,46), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer2 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(1);
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer4->AddGrowableCol(2);
	Button_Load = new wxButton(Panel1, ID_BUTTON_Load, _("Load"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Load"));
	FlexGridSizer4->Add(Button_Load, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Button_Save = new wxButton(Panel1, ID_BUTTON_Save, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Save"));
	FlexGridSizer4->Add(Button_Save, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	Panel1->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel1);
	FlexGridSizer2->SetSizeHints(Panel1);
	Panel2 = new wxPanel(SplitterWindow1, ID_PANEL2, wxPoint(128,40), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(1);
	FlexGridSizer14 = new wxFlexGridSizer(0, 3, 0, 0);
	CheckBox_OutputToLights = new wxCheckBox(Panel2, ID_CHECKBOX_OutputToLights, _("Output to lights"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_OutputToLights"));
	CheckBox_OutputToLights->SetValue(false);
	FlexGridSizer14->Add(CheckBox_OutputToLights, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer3->Add(FlexGridSizer14, 1, wxALL|wxEXPAND, 5);
	AuiNotebook1 = new wxAuiNotebook(Panel2, ID_AUINOTEBOOK1, wxDefaultPosition, wxSize(422,400), wxAUI_NB_TAB_SPLIT|wxTAB_TRAVERSAL);
	Panel_Standard = new wxPanel(AuiNotebook1, ID_PANEL_Standard, wxPoint(80,57), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Standard"));
	Panel_Standard->SetMinSize(wxSize(300,300));
	FlexGridSizer6 = new wxFlexGridSizer(1, 4, 0, 0);
	FlexGridSizer6->AddGrowableCol(3);
	FlexGridSizer6->AddGrowableRow(0);
	FlexGridSizer7 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	StaticText2 = new wxStaticText(Panel_Standard, ID_STATICTEXT2, _("Function"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer7->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	RadioButton_Standard_Off = new wxRadioButton(Panel_Standard, ID_RADIOBUTTON_Standard_Off, _("Off"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Off"));
	FlexGridSizer7->Add(RadioButton_Standard_Off, 1, wxALL|wxEXPAND, 5);
	RadioButton_Standard_Chase = new wxRadioButton(Panel_Standard, ID_RADIOBUTTON_Standard_Chase, _("Chase"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Chase"));
	FlexGridSizer7->Add(RadioButton_Standard_Chase, 1, wxALL|wxEXPAND, 5);
	RadioButton_Standard_Chase13 = new wxRadioButton(Panel_Standard, ID_RADIOBUTTON_Standard_Chase13, _("Chase 1/3"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Chase13"));
	FlexGridSizer7->Add(RadioButton_Standard_Chase13, 1, wxALL|wxEXPAND, 5);
	RadioButton_Standard_Chase14 = new wxRadioButton(Panel_Standard, ID_RADIOBUTTON_Standard_Chase14, _("Chase 1/4"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Chase14"));
	FlexGridSizer7->Add(RadioButton_Standard_Chase14, 1, wxALL|wxEXPAND, 5);
	RadioButton_Standard_Chase15 = new wxRadioButton(Panel_Standard, ID_RADIOBUTTON_Standard_Chase15, _("Chase 1/5"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Chase15"));
	FlexGridSizer7->Add(RadioButton_Standard_Chase15, 1, wxALL|wxEXPAND, 5);
	RadioButton_Standard_Alternate = new wxRadioButton(Panel_Standard, ID_RADIOBUTTON_Standard_Alternate, _("Alternate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Alternate"));
	FlexGridSizer7->Add(RadioButton_Standard_Alternate, 1, wxALL|wxEXPAND, 5);
	RadioButton_Standard_Twinkle5 = new wxRadioButton(Panel_Standard, ID_RADIOBUTTON_Standard_Twinke5, _("Twinkle 5%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Twinke5"));
	FlexGridSizer7->Add(RadioButton_Standard_Twinkle5, 1, wxALL|wxEXPAND, 5);
	RadioButton_Standard_Twinkle10 = new wxRadioButton(Panel_Standard, ID_RADIOBUTTON_Standard_Twinkle10, _("Twinkle 10%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Twinkle10"));
	FlexGridSizer7->Add(RadioButton_Standard_Twinkle10, 1, wxALL|wxEXPAND, 5);
	RadioButton_Standard_Twinkle25 = new wxRadioButton(Panel_Standard, ID_RADIOBUTTON_Standard_Twinkle25, _("Twinkle 25%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Twinkle25"));
	FlexGridSizer7->Add(RadioButton_Standard_Twinkle25, 1, wxALL|wxEXPAND, 5);
	RadioButton_Standard_Twinkle50 = new wxRadioButton(Panel_Standard, ID_RADIOBUTTON_Standard_Twinkle50, _("Twinkle 50%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Twinkle50"));
	FlexGridSizer7->Add(RadioButton_Standard_Twinkle50, 1, wxALL|wxEXPAND, 5);
	RadioButton_Standard_Shimmer = new wxRadioButton(Panel_Standard, ID_RADIOBUTTON_Standard_Shimmer, _("Shimmer"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Shimmer"));
	FlexGridSizer7->Add(RadioButton_Standard_Shimmer, 1, wxALL|wxEXPAND, 5);
	RadioButton_Standard_Background = new wxRadioButton(Panel_Standard, ID_RADIOBUTTON_Standard_Background, _("Background Only"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Background"));
	FlexGridSizer7->Add(RadioButton_Standard_Background, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer6->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer6->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	FlexGridSizer8 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	FlexGridSizer8->AddGrowableRow(1);
	StaticText3 = new wxStaticText(Panel_Standard, ID_STATICTEXT3, _("Background\nIntensity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer8->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Standard_Background = new wxSlider(Panel_Standard, ID_SLIDER_Standard_Background, 0, 0, 255, wxDefaultPosition, wxSize(74,181), wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_SLIDER_Standard_Background"));
	FlexGridSizer8->Add(Slider_Standard_Background, 1, wxALL|wxEXPAND, 5);
	BoxSizer1->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer9 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	FlexGridSizer9->AddGrowableRow(1);
	StaticText4 = new wxStaticText(Panel_Standard, ID_STATICTEXT4, _("Highlight\nIntensity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer9->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Standard_Highlight = new wxSlider(Panel_Standard, ID_SLIDER_Standard_Highlight, 255, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_SLIDER_Standard_Highlight"));
	FlexGridSizer9->Add(Slider_Standard_Highlight, 1, wxALL|wxEXPAND, 5);
	BoxSizer1->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer6->Add(BoxSizer1, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer6->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_Standard->SetSizer(FlexGridSizer6);
	FlexGridSizer6->Fit(Panel_Standard);
	FlexGridSizer6->SetSizeHints(Panel_Standard);
	Panel4 = new wxPanel(AuiNotebook1, ID_PANEL4, wxPoint(128,17), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
	FlexGridSizer10 = new wxFlexGridSizer(1, 4, 0, 0);
	FlexGridSizer10->AddGrowableCol(3);
	FlexGridSizer10->AddGrowableRow(0);
	FlexGridSizer11 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer11->AddGrowableCol(0);
	StaticText5 = new wxStaticText(Panel4, ID_STATICTEXT5, _("Function"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer11->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	RadioButton_RGB_Off = new wxRadioButton(Panel4, ID_RADIOBUTTON_RGB_Off, _("Off"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Off"));
	FlexGridSizer11->Add(RadioButton_RGB_Off, 1, wxALL|wxEXPAND, 5);
	RadioButton_RGB_Chase = new wxRadioButton(Panel4, ID_RADIOBUTTON_RGB_Chase, _("Chase"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Chase"));
	FlexGridSizer11->Add(RadioButton_RGB_Chase, 1, wxALL|wxEXPAND, 5);
	RadioButton_RGB_Chase13 = new wxRadioButton(Panel4, ID_RADIOBUTTON_RGB_Chase13, _("Chase 1/3"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Chase13"));
	FlexGridSizer11->Add(RadioButton_RGB_Chase13, 1, wxALL|wxEXPAND, 5);
	RadioButton_RGB_Chase14 = new wxRadioButton(Panel4, ID_RADIOBUTTON_RGB_Chase14, _("Chase 1/4"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Chase14"));
	FlexGridSizer11->Add(RadioButton_RGB_Chase14, 1, wxALL|wxEXPAND, 5);
	RadioButton_RGB_Chase15 = new wxRadioButton(Panel4, ID_RADIOBUTTON_RGB_Chase15, _("Chase 1/5"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Chase15"));
	FlexGridSizer11->Add(RadioButton_RGB_Chase15, 1, wxALL|wxEXPAND, 5);
	RadioButton_RGB_Alternate = new wxRadioButton(Panel4, ID_RADIOBUTTON_RGB_Alternate, _("Alternate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Alternate"));
	FlexGridSizer11->Add(RadioButton_RGB_Alternate, 1, wxALL|wxEXPAND, 5);
	RadioButton_RGB_Twinkle5 = new wxRadioButton(Panel4, ID_RADIOBUTTON_RGB_Twinkle5, _("Twinkle 5%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Twinkle5"));
	FlexGridSizer11->Add(RadioButton_RGB_Twinkle5, 1, wxALL|wxEXPAND, 5);
	RadioButton_RGB_Twinkle10 = new wxRadioButton(Panel4, ID_RADIOBUTTON_RGB_Twinkle10, _("Twinkle 10%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Twinkle10"));
	FlexGridSizer11->Add(RadioButton_RGB_Twinkle10, 1, wxALL|wxEXPAND, 5);
	RadioButton_RGB_Twinkle25 = new wxRadioButton(Panel4, ID_RADIOBUTTON_RGB_Twinkle25, _("Twinkle 25%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Twinkle25"));
	FlexGridSizer11->Add(RadioButton_RGB_Twinkle25, 1, wxALL|wxEXPAND, 5);
	RadioButton_RGB_Twinkle50 = new wxRadioButton(Panel4, ID_RADIOBUTTON_RGB_Twinkle50, _("Twinkle 50%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Twinkle50"));
	FlexGridSizer11->Add(RadioButton_RGB_Twinkle50, 1, wxALL|wxEXPAND, 5);
	RadioButton_RGB_Shimmer = new wxRadioButton(Panel4, ID_RADIOBUTTON_RGB_Shimmer, _("Shimmer"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Shimmer"));
	FlexGridSizer11->Add(RadioButton_RGB_Shimmer, 1, wxALL|wxEXPAND, 5);
	RadioButton_RGB_Background = new wxRadioButton(Panel4, ID_RADIOBUTTON_RGB_Background, _("Background Only"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Background"));
	FlexGridSizer11->Add(RadioButton_RGB_Background, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer10->Add(FlexGridSizer11, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer10->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer2 = new wxBoxSizer(wxVERTICAL);
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, Panel4, _("Background Color"));
	Slider_RGB_BG_R = new wxSlider(Panel4, ID_SLIDER1, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER1"));
	StaticBoxSizer1->Add(Slider_RGB_BG_R, 1, wxALL|wxEXPAND, 5);
	Slider_RGB_BG_G = new wxSlider(Panel4, ID_SLIDER2, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER2"));
	StaticBoxSizer1->Add(Slider_RGB_BG_G, 1, wxALL|wxEXPAND, 5);
	Slider_RGB_BG_B = new wxSlider(Panel4, ID_SLIDER3, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER3"));
	StaticBoxSizer1->Add(Slider_RGB_BG_B, 1, wxALL|wxEXPAND, 5);
	BoxSizer3->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 5);
	BoxSizer2->Add(BoxSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, Panel4, _("Highlight Color"));
	Slider_RGB_H_R = new wxSlider(Panel4, ID_SLIDER4, 255, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER4"));
	StaticBoxSizer2->Add(Slider_RGB_H_R, 1, wxALL|wxEXPAND, 5);
	Slider_RGB_H_G = new wxSlider(Panel4, ID_SLIDER5, 255, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER5"));
	StaticBoxSizer2->Add(Slider_RGB_H_G, 1, wxALL|wxEXPAND, 5);
	Slider_RGB_H_B = new wxSlider(Panel4, ID_SLIDER6, 255, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER6"));
	StaticBoxSizer2->Add(Slider_RGB_H_B, 1, wxALL|wxEXPAND, 5);
	BoxSizer4->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
	BoxSizer2->Add(BoxSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer10->Add(BoxSizer2, 1, wxALL|wxEXPAND, 5);
	Panel4->SetSizer(FlexGridSizer10);
	FlexGridSizer10->Fit(Panel4);
	FlexGridSizer10->SetSizeHints(Panel4);
	Panel5 = new wxPanel(AuiNotebook1, ID_PANEL5, wxPoint(132,14), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
	FlexGridSizer12 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer12->AddGrowableRow(0);
	FlexGridSizer13 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText6 = new wxStaticText(Panel5, ID_STATICTEXT6, _("Function"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer13->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RadioButton_RGBCycle_Off = new wxRadioButton(Panel5, ID_RADIOBUTTON_RGBCycle_Off, _("Off"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGBCycle_Off"));
	FlexGridSizer13->Add(RadioButton_RGBCycle_Off, 1, wxALL|wxEXPAND, 5);
	RadioButton_RGBCycle_ABC = new wxRadioButton(Panel5, ID_RADIOBUTTON_RGBCycle_ABC, _("A-B-C"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGBCycle_ABC"));
	FlexGridSizer13->Add(RadioButton_RGBCycle_ABC, 1, wxALL|wxEXPAND, 5);
	RadioButton_RGBCycle_ABCAll = new wxRadioButton(Panel5, ID_RADIOBUTTON_RGBCycle_ABCAll, _("A-B-C-All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGBCycle_ABCAll"));
	FlexGridSizer13->Add(RadioButton_RGBCycle_ABCAll, 1, wxALL|wxEXPAND, 5);
	RadioButton_RGBCycle_ABCAllNone = new wxRadioButton(Panel5, ID_RADIOBUTTON_RGBCycle_ABCAllNone, _("A-B-C-All-None"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGBCycle_ABCAllNone"));
	FlexGridSizer13->Add(RadioButton_RGBCycle_ABCAllNone, 1, wxALL|wxEXPAND, 5);
	RadioButton_RGBCycle_MixedColors = new wxRadioButton(Panel5, ID_RADIOBUTTON_RGBCycle_MixedColors, _("Mixed Colors"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGBCycle_MixedColors"));
	FlexGridSizer13->Add(RadioButton_RGBCycle_MixedColors, 1, wxALL|wxEXPAND, 5);
	RadioButton_RGBCycle_RGBW = new wxRadioButton(Panel5, ID_RADIOBUTTON_RGBCycle_RGBW, _("R-G-B-W"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGBCycle_RGBW"));
	FlexGridSizer13->Add(RadioButton_RGBCycle_RGBW, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer12->Add(FlexGridSizer13, 1, wxALL|wxEXPAND, 5);
	Panel5->SetSizer(FlexGridSizer12);
	FlexGridSizer12->Fit(Panel5);
	FlexGridSizer12->SetSizeHints(Panel5);
	AuiNotebook1->AddPage(Panel_Standard, _("Standard"), true);
	AuiNotebook1->AddPage(Panel4, _("RGB"));
	AuiNotebook1->AddPage(Panel5, _("RGB Cycle"));
	FlexGridSizer3->Add(AuiNotebook1, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 2);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(1);
	StaticText1 = new wxStaticText(Panel2, ID_STATICTEXT1, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer5->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Speed = new wxSlider(Panel2, ID_SLIDER_Speed, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Speed"));
	FlexGridSizer5->Add(Slider_Speed, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer3->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
	Panel2->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Panel2);
	FlexGridSizer3->SetSizeHints(Panel2);
	SplitterWindow1->SplitVertically(Panel1, Panel2);
	FlexGridSizer1->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 2);
	StatusBar1 = new wxStaticText(this, ID_STATICTEXT7, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDOUBLE_BORDER, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StatusBar1, 1, wxALL|wxEXPAND, 2);
	SetSizer(FlexGridSizer1);
	Timer1.SetOwner(this, ID_TIMER1);
	SetSizer(FlexGridSizer1);
	Layout();
	Center();

	Connect(ID_BUTTON_Load,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TestDialog::OnButton_LoadClick);
	Connect(ID_BUTTON_Save,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TestDialog::OnButton_SaveClick);
	Connect(ID_CHECKBOX_OutputToLights,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&TestDialog::OnCheckBox_OutputToLightsClick);
	Connect(ID_RADIOBUTTON_Standard_Off,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_Standard_OffSelect);
	Connect(ID_RADIOBUTTON_Standard_Chase,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_Standard_ChaseSelect);
	Connect(ID_RADIOBUTTON_Standard_Chase13,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_Standard_Chase13Select);
	Connect(ID_RADIOBUTTON_Standard_Chase14,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_Standard_Chase14Select);
	Connect(ID_RADIOBUTTON_Standard_Chase15,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_Standard_Chase15Select);
	Connect(ID_RADIOBUTTON_Standard_Alternate,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_Standard_AlternateSelect);
	Connect(ID_RADIOBUTTON_Standard_Twinke5,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_Standard_Twinkle5Select);
	Connect(ID_RADIOBUTTON_Standard_Twinkle10,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_Standard_Twinkle10Select);
	Connect(ID_RADIOBUTTON_Standard_Twinkle25,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_Standard_Twinkle25Select);
	Connect(ID_RADIOBUTTON_Standard_Twinkle50,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_Standard_Twinkle50Select);
	Connect(ID_RADIOBUTTON_Standard_Shimmer,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_Standard_ShimmerSelect);
	Connect(ID_RADIOBUTTON_Standard_Background,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_Standard_BackgroundSelect);
	Connect(ID_RADIOBUTTON_RGB_Off,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGB_OffSelect);
	Connect(ID_RADIOBUTTON_RGB_Chase,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGB_ChaseSelect);
	Connect(ID_RADIOBUTTON_RGB_Chase13,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGB_Chase13Select);
	Connect(ID_RADIOBUTTON_RGB_Chase14,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGB_Chase14Select);
	Connect(ID_RADIOBUTTON_RGB_Chase15,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGB_Chase15Select);
	Connect(ID_RADIOBUTTON_RGB_Alternate,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGB_AlternateSelect);
	Connect(ID_RADIOBUTTON_RGB_Twinkle5,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGB_Twinkle5Select);
	Connect(ID_RADIOBUTTON_RGB_Twinkle10,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGB_Twinkle10Select);
	Connect(ID_RADIOBUTTON_RGB_Twinkle25,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGB_Twinkle25Select);
	Connect(ID_RADIOBUTTON_RGB_Twinkle50,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGB_Twinkle50Select);
	Connect(ID_RADIOBUTTON_RGB_Shimmer,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGB_ShimmerSelect);
	Connect(ID_RADIOBUTTON_RGB_Background,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGB_BackgroundSelect);
	Connect(ID_RADIOBUTTON_RGBCycle_Off,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGBCycle_OffSelect);
	Connect(ID_RADIOBUTTON_RGBCycle_ABC,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGBCycle_ABCSelect);
	Connect(ID_RADIOBUTTON_RGBCycle_ABCAll,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGBCycle_ABCAllSelect);
	Connect(ID_RADIOBUTTON_RGBCycle_ABCAllNone,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGBCycle_ABCAllNoneSelect);
	Connect(ID_RADIOBUTTON_RGBCycle_MixedColors,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGBCycle_MixedColorsSelect);
	Connect(ID_RADIOBUTTON_RGBCycle_RGBW,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&TestDialog::OnRadioButton_RGBCycle_RGBWSelect);
	Connect(ID_SLIDER_Speed,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&TestDialog::OnSlider_SpeedCmdSliderUpdated);
	Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&TestDialog::OnTimer1Trigger);
	//*)

	SetSize(wxSystemSettings::GetMetric(wxSYS_SCREEN_X) * 3 / 4, wxSystemSettings::GetMetric(wxSYS_SCREEN_Y) * 3 / 4);

	SplitterWindow1->SetMinimumPaneSize(100);

	TreeListCtrl_Channels = new wxTreeListCtrl(Panel1, ID_TREELISTCTRL_Channels, wxPoint(0, 0), Panel1->GetSize(), wxTR_FULL_ROW_HIGHLIGHT | wxTR_DEFAULT_STYLE| wxTL_CHECKBOX | wxTL_USER_3STATE, _T("ID_TREELISTCTRL_Channels"));
	TreeListCtrl_Channels->AppendColumn(L"Select channels ...");
	_controllers = TreeListCtrl_Channels->AppendItem(TreeListCtrl_Channels->GetRootItem(), _("(CONTROLLERS)"));
	_modelGroups = TreeListCtrl_Channels->AppendItem(TreeListCtrl_Channels->GetRootItem(), _("(MODELGROUPS)"));
	_models = TreeListCtrl_Channels->AppendItem(TreeListCtrl_Channels->GetRootItem(), _("(MODELS)"));
	FlexGridSizer2->Add(TreeListCtrl_Channels, 1, wxALL | wxEXPAND, 5);
	FlexGridSizer2->Layout();

	// add checkbox events
	Connect(ID_TREELISTCTRL_Channels,wxEVT_COMMAND_CHECKLISTBOX_TOGGLED,(wxObjectEventFunction)&TestDialog::OnTreeListCtrl1Checkboxtoggled);
	Connect(ID_TREELISTCTRL_Channels, wxEVT_COMMAND_TREELIST_ITEM_CHECKED, (wxObjectEventFunction)&TestDialog::OnTreeListCtrl1Checkboxtoggled);
	Connect(ID_TREELISTCTRL_Channels, wxEVT_COMMAND_TREELIST_SELECTION_CHANGED, (wxObjectEventFunction)&TestDialog::OnTreeListCtrl1ItemActivated);
	//Connect(ID_TREELISTCTRL_Channels, wxEVT_COMMAND_TREELIST_ITEM_ACTIVATED, (wxObjectEventFunction)&TestDialog::OnTreeListCtrl1ItemActivated);

	PopulateControllerTree(_network);
	PopulateModelsTree(_modelManager);
	PopulateModelGroupsTree(_modelManager);
	CascadeModelDoesNotExist();
	DeactivateNotClickableModels();

	if (TreeListCtrl_Channels->GetFirstChild(_controllers) == NULL)
	{
		TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(_controllers);
		tc->DoesNotExist();
	}
	if (TreeListCtrl_Channels->GetFirstChild(_models) == NULL)
	{
		TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(_models);
		tc->DoesNotExist();
	}
	if (TreeListCtrl_Channels->GetFirstChild(_modelGroups) == NULL)
	{
		TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(_modelGroups);
		tc->DoesNotExist();
	}

	_starttime = wxDateTime::UNow();
	DisableSleepModes();

	Timer1.Start(50, wxTIMER_CONTINUOUS);
}

// Destructor

TestDialog::~TestDialog()
{
	// need to delete all the TreeController Objects
	wxTreeListItem root = TreeListCtrl_Channels->GetRootItem();
	DestroyTreeControllerData(root);

	// need to delete the TreeController.
	Panel1->RemoveChild(TreeListCtrl_Channels);
	//delete TreeListCtrl_Channels;

	if (_xout)
	{
		_xout->alloff();
		delete _xout;
		_xout = NULL;
	}
	EnableSleepModes();

	//(*Destroy(TestDialog)
	//*)
}

// Functions for navigating the tree

TreeController::CONTROLLERTYPE TestDialog::GetTreeItemType(const wxTreeListItem& item)
{
	wxTreeListItem current = item;

	while (TreeListCtrl_Channels->GetItemParent(current) != TreeListCtrl_Channels->GetRootItem())
	{
		current = TreeListCtrl_Channels->GetItemParent(current);
	}

	return ((TreeController*)TreeListCtrl_Channels->GetItemData(current))->GetType();
}

// Cascading Functions

bool TestDialog::CascadeSelected(wxTreeListItem& item, wxCheckBoxState state)
{
	bool rc = true;

	TreeController::CONTROLLERTYPE type = GetTreeItemType(item);

	if (type == TreeController::CONTROLLERTYPE::CT_CONTROLLERROOT)
	{
		bool processed = false;
		wxTreeListItem i = TreeListCtrl_Channels->GetFirstChild(item);

		if (i == NULL)
		{
			TreeController* itemtc = (TreeController*)TreeListCtrl_Channels->GetItemData(item);
			if (itemtc->GetType() == TreeController::CONTROLLERTYPE::CT_CHANNEL)
			{
				processed = true;
				rc &= CheckChannel(itemtc->StartXLightsChannel(), state);
			}
		}

		while (i != NULL)
		{
			TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(i);

			// Dont cascade to inactive or NULL nodes
			if (tc->Clickable())
			{
				processed = true;
				TreeListCtrl_Channels->CheckItem(i, state);
				rc &= CascadeSelected(i, state);

				if (tc->GetType() == TreeController::CONTROLLERTYPE::CT_CHANNEL)
				{
					rc &= CheckChannel(tc->StartXLightsChannel(), state);
				}
			}
			i = TreeListCtrl_Channels->GetNextSibling(i);
		}

		if (!processed)
		{
			rc = false;
		}
	}
	else if (type == TreeController::CONTROLLERTYPE::CT_MODELGROUPROOT)
	{
		TreeController* itc = (TreeController*)TreeListCtrl_Channels->GetItemData(item);
		if (itc->GetType() == TreeController::CONTROLLERTYPE::CT_MODEL)
		{
			// go and find the models
			TreeListCtrl_Channels->CheckItem(item, state);

			if (!_cascading)
			{
				_cascading = true;
				for (int i = 0; i < _modelLookup[itc->ModelName()].size(); i++)
				{
					wxTreeListItem tli = (_modelLookup[itc->ModelName()][i]->GetTreeListItem());
					if (tli == item)
					{
						// dont do anything ... this is the node we are already processing
					}
					else
					{
						// apply this state to other instances of this model
						TreeListCtrl_Channels->CheckItem(tli, state);
						rc &= CascadeSelected(tli, state);
					}
				}
				_cascading = false;
			}
		}

		wxTreeListItem i = TreeListCtrl_Channels->GetFirstChild(item);
		while (i != NULL)
		{
			TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(i);

			if (tc->GetType() == TreeController::CONTROLLERTYPE::CT_MODEL)
			{
				if (tc->Clickable())
				{
					TreeListCtrl_Channels->CheckItem(i, state);
					// go and find the models
					if (!_cascading)
					{
						_cascading = true;
						for (int j = 0; j < _modelLookup[tc->ModelName()].size(); j++)
						{
							wxTreeListItem tli = (_modelLookup[tc->ModelName()][j]->GetTreeListItem());
							if (tli == i)
							{
								// dont do anything ... this is the node we are already processing
							}
							else
							{
								// apply this state to other instances of this model
								TreeListCtrl_Channels->CheckItem(tli, state);
								rc &= CascadeSelected(tli, state);
							}
						}
						_cascading = false;
					}
				}
			}
			else
			{
				TreeListCtrl_Channels->CheckItem(i, state);
				rc &= CascadeSelected(i, state);
			}
			i = TreeListCtrl_Channels->GetNextSibling(i);
		}
	}
	else // Model root
	{
		bool processed = false;
		TreeController* m = (TreeController*)TreeListCtrl_Channels->GetItemData(item);
		if (m->GetType() == TreeController::CONTROLLERTYPE::CT_MODEL)
		{
			// go and find the models
			TreeListCtrl_Channels->CheckItem(item, state);
			if (!_cascading)
			{
				_cascading = true;
				for (int i = 0; i < _modelLookup[m->ModelName()].size(); i++)
				{
					wxTreeListItem tli = (_modelLookup[m->ModelName()][i]->GetTreeListItem());
					if (tli == item)
					{
						// dont do anything ... this is the node we are already processing
					}
					else
					{
						processed = true;
						// apply this state to other instances of this model
						TreeListCtrl_Channels->CheckItem(tli, state);
						rc &= CascadeSelected(tli, state);
					}
				}
				_cascading = false;
			}
			//CascadeSelectedToModelGroup(m->ModelName(), state);
		}

		wxTreeListItem i = TreeListCtrl_Channels->GetFirstChild(item);

		if (i == NULL)
		{
			if (m->GetType() == TreeController::CONTROLLERTYPE::CT_CHANNEL)
			{
				processed = true;
				rc &= CheckChannel(m->StartXLightsChannel(), state);
			}
		}

		while (i != NULL)
		{
			TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(i);
			if (tc->GetType() == TreeController::CONTROLLERTYPE::CT_CHANNEL)
			{
				processed = true;
				rc &= CheckChannel(tc->StartXLightsChannel(), state);
			}
			else
			{
				if (tc->Clickable() || state == wxCHK_UNCHECKED)
				{
					processed = true;
					TreeListCtrl_Channels->CheckItem(i, state);
					rc &= CascadeSelected(i, state);
				}
			}
			i = TreeListCtrl_Channels->GetNextSibling(i);
		}

		if (!processed)
		{
			rc = false;
		}
	}

	return rc;
}

// Populate the tree functions

void TestDialog::PopulateControllerTree(wxXmlDocument* network)
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
			wxTreeListItem c = TreeListCtrl_Channels->AppendItem(_controllers, controller->Name(), -1, -1, (wxClientData*)controller);
			controller->SetTreeListItem(c);
			if (controller->Clickable())
			{
				for (int i = 1; i <= controller->Channels(); i++)
				{
					TreeController* tc = new TreeController(i, TreeController::CONTROLLERTYPE::CT_CHANNEL, controller->StartXLightsChannel() + i - 1);
					wxTreeListItem tli = TreeListCtrl_Channels->AppendItem(c, tc->Name(), -1, -1, (wxClientData*)tc);
					tc->SetTreeListItem(tli);
					_channelLookup[tc->StartXLightsChannel()].push_back(tc);
				}
			}

			int universeoffset = 1;
			TreeController* c2 = controller->GenerateDMXUniverse(universeoffset++);
			while (c2 != NULL)
			{
				currentcontrollerstartchannel += c2->Channels();
				wxTreeListItem c2c = TreeListCtrl_Channels->AppendItem(_controllers, c2->Name(), -1, -1, (wxClientData*)c2);
				controller->SetTreeListItem(c2c);
				if (!c2->Inactive())
				{
					for (int i = 1; i <= c2->Channels(); i++)
					{
						TreeController* tc = new TreeController(i, TreeController::CONTROLLERTYPE::CT_CHANNEL, c2->StartXLightsChannel() + i - 1);
						wxTreeListItem c = TreeListCtrl_Channels->AppendItem(c2c, tc->Name(), -1, -1, (wxClientData*)tc);
						tc->SetTreeListItem(c);
						_channelLookup[tc->StartXLightsChannel()].push_back(tc);
					}
				}
				c2 = controller->GenerateDMXUniverse(universeoffset++);
			}
		}
		TreeListCtrl_Channels->Expand(_controllers);
	}

	TreeController* root = new TreeController(TreeController::CONTROLLERTYPE::CT_CONTROLLERROOT, 1, currentcontrollerstartchannel);
	TreeListCtrl_Channels->SetItemData(_controllers, (wxClientData*)root);
	TreeListCtrl_Channels->SetItemText(_controllers, root->Name());
}

void TestDialog::PopulateModelGroupsTree(ModelManager* modelManager)
{
	for (auto it = _modelManager->begin(); it != _modelManager->end(); it++)
	{
		Model* m = it->second;

		if (m->GetDisplayAs() == "ModelGroup")
		{
			TreeController* modelgroupcontroller = new TreeController(TreeController::CONTROLLERTYPE::CT_MODELGROUP, m->name);
			wxTreeListItem modelgroupitem = TreeListCtrl_Channels->AppendItem(_modelGroups, modelgroupcontroller->Name(), -1, -1, (wxClientData*)modelgroupcontroller);
			modelgroupcontroller->SetTreeListItem(modelgroupitem);

			// now look for all the models in the model group
			ModelGroup* mg = (ModelGroup*)m;
			for (auto mn = mg->ModelNames().begin(); mn != mg->ModelNames().end(); ++mn)
			{
				for (auto findm = _modelManager->begin(); findm != _modelManager->end(); findm++)
				{
					Model* m2 = findm->second;

					if (m2->GetDisplayAs() != "ModelGroup" && m2->name == *mn)
					{
						// m2 is the model to insert the details for
						TreeController* modelcontroller = new TreeController(TreeController::CONTROLLERTYPE::CT_MODEL, m2->name);
						wxTreeListItem modelitem = TreeListCtrl_Channels->AppendItem(modelgroupitem, modelcontroller->Name(), -1, -1, (wxClientData*)modelcontroller);
						modelcontroller->SetTreeListItem(modelitem);
						_modelLookup[modelcontroller->ModelName()].push_back(modelcontroller);

						// dont add nodes or channels ... they can go to the models section
					}
				}
			}
			TreeListCtrl_Channels->SetItemText(modelgroupitem, modelgroupcontroller->Name());
		}
	}
	TreeListCtrl_Channels->Expand(_modelGroups);

	TreeController* controllerroot = (TreeController*)TreeListCtrl_Channels->GetItemData(_controllers);
	TreeController* root = new TreeController(TreeController::CONTROLLERTYPE::CT_MODELGROUPROOT, controllerroot->StartXLightsChannel(), controllerroot->EndXLightsChannel());
	if (TreeListCtrl_Channels->GetFirstChild(_modelGroups) == NULL)
	{
		root->DoesNotExist();
	}
	TreeListCtrl_Channels->SetItemData(_modelGroups, (wxClientData*)root);
	TreeListCtrl_Channels->SetItemText(_modelGroups, root->Name());
}

void TestDialog::CascadeColour(TreeController* tc)
{
	auto chs = _channelLookup[tc->StartXLightsChannel()];

	for (int i = 0; i < chs.size(); i++)
	{
		if (chs[i] != tc)
		{
			chs[i]->SetColour(tc->GetColour());
			TreeListCtrl_Channels->SetItemText(chs[i]->GetTreeListItem(), chs[i]->Name());
		}
	}
}

void TestDialog::PopulateModelsTree(ModelManager* modelManager)
{
	for (auto it = _modelManager->begin(); it != _modelManager->end(); it++)
	{
		Model* m = it->second;

		if (m->GetDisplayAs() != "ModelGroup")
		{
			// we found a model
			TreeController* modelcontroller = new TreeController(TreeController::CONTROLLERTYPE::CT_MODEL, m->name);
			wxTreeListItem modelitem = TreeListCtrl_Channels->AppendItem(_models, modelcontroller->Name(), -1, -1, (wxClientData*)modelcontroller);
			modelcontroller->SetTreeListItem(modelitem);
			_modelLookup[modelcontroller->ModelName()].push_back(modelcontroller);
			int modelendchannel = 0;
			int modelstartchannel = 0xFFFFFFF;
            int msc = m->GetNumberFromChannelString(m->ModelStartChannel);

			if (m->SingleChannel)
			{
				TreeController* tc = new TreeController(-1, TreeController::CONTROLLERTYPE::CT_CHANNEL, msc);
				xlColor col(m->GetNodeColor(0));
				tc->SetColour(DoEncodeColour(col));
				CascadeColour(tc);
				modelendchannel = std::max(modelendchannel, tc->EndXLightsChannel());
				modelstartchannel = std::min(modelstartchannel, tc->StartXLightsChannel());
				if (_channelLookup[tc->StartXLightsChannel()].size() == 0)
				{
					// no existing item ... this means either channel not defned or has been excluded because controller is NULL or INACTIVE
					delete tc;
				}
				else
				{
					wxTreeListItem mc = TreeListCtrl_Channels->AppendItem(modelitem, tc->Name(), -1, -1, (wxClientData*)tc);
					tc->SetTreeListItem(mc);
					_channelLookup[tc->StartXLightsChannel()].push_back(tc);
				}
			}
			else
			{
				modelcontroller->SetNodes(m->GetNodeCount());
				for (int i = 0; i < m->GetNodeCount(); i++)
				{
					TreeController* tc = new TreeController(TreeController::CONTROLLERTYPE::CT_NODE, msc + i * m->GetChanCountPerNode(), i + 1, m->GetChanCountPerNode());
					modelendchannel = std::max(modelendchannel, tc->EndXLightsChannel());
					modelstartchannel = std::min(modelstartchannel, tc->StartXLightsChannel());
					wxTreeListItem nodeitem = TreeListCtrl_Channels->AppendItem(modelitem, tc->Name(), -1, -1, NULL);
					tc->SetTreeListItem(nodeitem);

					for (int j = 0; j < m->GetChanCountPerNode(); j++)
					{
						TreeController* tcc = new TreeController(-1, TreeController::CONTROLLERTYPE::CT_CHANNEL, msc + i * m->GetChanCountPerNode() + j);
						char col = std::string(wxString(m->GetChannelColorLetter(j)).c_str())[0];
						tcc->SetColour(col);
						CascadeColour(tcc);
						if (_channelLookup[tcc->StartXLightsChannel()].size() == 0)
						{
							// no existing item ... this means either channel not defned or has been excluded because controller is NULL or INACTIVE
							delete tcc;
						}
						else
						{
							wxTreeListItem mc = TreeListCtrl_Channels->AppendItem(nodeitem, tcc->Name(), -1, -1, (wxClientData*)tcc);
							tcc->SetTreeListItem(mc);
							_channelLookup[tcc->StartXLightsChannel()].push_back(tcc);
						}
					}

					if (TreeListCtrl_Channels->GetFirstChild(nodeitem) == NULL)
					{
						// No children ... so no channels were created so we should remove the node
						delete tc;
						TreeListCtrl_Channels->DeleteItem(nodeitem);
					}
					else
					{
						TreeListCtrl_Channels->SetItemData(nodeitem, (wxClientData*)tc);
					}
				}
			}
			if (TreeListCtrl_Channels->GetFirstChild(modelitem) == NULL)
			{
				// No children ... so no channels/nodes were created so we should remove the node
				modelcontroller->DoesNotExist();
			}
			modelcontroller->SetEndXLightsChannel(modelendchannel);
			modelcontroller->SetStartXLightsChannel(modelstartchannel);
			TreeListCtrl_Channels->SetItemText(modelitem, modelcontroller->Name());
		}
	}
	TreeListCtrl_Channels->Expand(_models);

	TreeController* controllerroot = (TreeController*)TreeListCtrl_Channels->GetItemData(_controllers);
	TreeController* root = new TreeController(TreeController::CONTROLLERTYPE::CT_MODELROOT, controllerroot->StartXLightsChannel(), controllerroot->EndXLightsChannel());
	TreeListCtrl_Channels->SetItemData(_models, (wxClientData*)root);
	TreeListCtrl_Channels->SetItemText(_models, root->Name());
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

void TestDialog::DeactivateNotClickableModels()
{
	wxTreeListItem i = TreeListCtrl_Channels->GetFirstChild(_models);
	while (i != NULL)
	{
		TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(i);
		if (!tc->Clickable())
		{
			TreeListCtrl_Channels->SetItemText(i, tc->Name());
		}
		i = TreeListCtrl_Channels->GetNextSibling(i);
	}
	i = TreeListCtrl_Channels->GetFirstChild(_modelGroups);
	while (i != NULL)
	{
		wxTreeListItem j = TreeListCtrl_Channels->GetFirstChild(i);
		while (j != NULL)
		{
			TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(j);
			if (!tc->Clickable())
			{
				TreeListCtrl_Channels->SetItemText(j, tc->Name());
			}
			j = TreeListCtrl_Channels->GetNextSibling(j);
		}
		i = TreeListCtrl_Channels->GetNextSibling(i);
	}
}

std::list<std::string> TestDialog::GetModelsOnChannels(int start, int end)
{
	std::list<std::string> res;

	for (auto it = _modelManager->begin(); it != _modelManager->end(); it++)
	{
		Model* m = it->second;
		int st = wxAtoi(m->ModelStartChannel);
		int en = m->GetLastChannel();
		if (start <= en+1 && end >= st)
		{
			res.push_back(it->first);
		}
	}

	return res;
}

void TestDialog::OnTreeListCtrl1ItemActivated(wxTreeListEvent& event)
{
	// Tooltips dont work on the TreeListCtrl ... so dont bother
    return;

	wxTreeListItem item = event.GetItem();
	if (item == _controllers || item == _modelGroups || item == _models)
	{
		TreeListCtrl_Channels->UnsetToolTip();
	}
	else
	{
		TreeController::CONTROLLERTYPE rootType = GetTreeItemType(item);

		if (rootType == TreeController::CONTROLLERTYPE::CT_CONTROLLERROOT)
		{
			TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(item);
			int start = tc->StartXLightsChannel();
			int end = tc->EndXLightsChannel();
			if (tc->IsChannel())
			{
				end = start;
			}
			std::list<std::string> models = GetModelsOnChannels(start, end);
			std::string tt = "";
			for (std::list<std::string>::iterator it = models.begin(); it != models.end(); ++it)
			{
				if (tt != "")
				{
					tt += "\n";
				}
				tt = tt + *it;
			}
			if (tt != "")
			{
				if (start == end)
				{
					tt = "[" + std::string(wxString::Format(wxT("%i"), start)) + "] maps to\n" + tt;
				}
				else
				{
					tt = "[" + std::string(wxString::Format(wxT("%i"), start)) + "-" + std::string(wxString::Format(wxT("%i"), end)) + "] maps to\n" + tt;
				}
				// This does not work ... there is a bug in wxWidgets which prevents tooltip display.
				TreeListCtrl_Channels->SetToolTip(tt);
			}
			else
			{
				TreeListCtrl_Channels->UnsetToolTip();
			}
		}
		else
		{
			TreeListCtrl_Channels->UnsetToolTip();
		}
	}
}

void TestDialog::OnTreeListCtrl1Checkboxtoggled(wxTreeListEvent& event)
{
	wxTreeListItem item = event.GetItem();

	TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(item);

	// You cannot check these items
	if (!tc->Clickable())
	{
		TreeListCtrl_Channels->CheckItem(item, wxCHK_UNCHECKED);
		wxBell();
		return;
	}

	wxCheckBoxState checked = TreeListCtrl_Channels->GetCheckedState(item);

	if (checked == wxCheckBoxState::wxCHK_UNDETERMINED)
	{
		if (tc->GetType() == TreeController::CONTROLLERTYPE::CT_CHANNEL)
		{
			CheckChannel(tc->StartXLightsChannel(), wxCHK_UNCHECKED);
		}
		else
		{
			TreeListCtrl_Channels->CheckItem(item, wxCHK_UNCHECKED);
		}
		checked = wxCheckBoxState::wxCHK_UNCHECKED;
	}

	if (checked == wxCheckBoxState::wxCHK_CHECKED)
	{
		if (!CascadeSelected(item, wxCheckBoxState::wxCHK_CHECKED))
		{
			wxBell();
		}
	}
	else if (checked == wxCheckBoxState::wxCHK_UNCHECKED)
	{
		CascadeSelected(item, wxCheckBoxState::wxCHK_UNCHECKED);
	}

		RollUpAll(_controllers);
	RollUpAll(_models);
	RollUpAll(_modelGroups);

	_checkChannelList = true;

	// handle multiple selected items
	wxTreeListItems selections;
	TreeListCtrl_Channels->GetSelections(selections);
	if (selections.size() > 1)
	{
		for (int i = 0; i < selections.size(); i++)
		{
			// dont double process the item that was passed into the event
			if (selections[i] != item)
			{
				if (TreeListCtrl_Channels->GetCheckedState(selections[i]) == wxCHK_UNCHECKED)
				{
					// check the items
					TreeListCtrl_Channels->CheckItem(selections[i], wxCheckBoxState::wxCHK_CHECKED);
					CascadeSelected(selections[i], wxCheckBoxState::wxCHK_CHECKED);
				}
				else if (TreeListCtrl_Channels->GetCheckedState(selections[i]) == wxCHK_CHECKED)
				{
					// uncheck the items
					TreeListCtrl_Channels->CheckItem(selections[i], wxCheckBoxState::wxCHK_UNCHECKED);
					CascadeSelected(selections[i], wxCheckBoxState::wxCHK_UNCHECKED);
				}
			}
		}
	}

	RollUpAll(_controllers);
	RollUpAll(_models);
	RollUpAll(_modelGroups);

	_checkChannelList = true;
}

wxCheckBoxState TestDialog::RollUpAll(wxTreeListItem start)
{
	wxCheckBoxState start_state = TreeListCtrl_Channels->GetCheckedState(start);
	wxTreeListItem a = TreeListCtrl_Channels->GetFirstChild(start);
	if (a == NULL)
	{
		return start_state;
	}

	if (a != NULL)
	{
		start_state = RollUpAll(a);
	}
	while (a != NULL)
	{
		TreeController* tca = (TreeController*)TreeListCtrl_Channels->GetItemData(a);
		if (!tca->Clickable())
		{
			// does not matter its state ... ignore it
		}
		else
		{
			wxCheckBoxState nas = RollUpAll(a);
			if (nas != start_state || nas == wxCHK_UNDETERMINED)
			{
				start_state = wxCHK_UNDETERMINED;
				//break;
			}
		}
		a = TreeListCtrl_Channels->GetNextSibling(a);
	}
	TreeListCtrl_Channels->CheckItem(start, start_state);

	// if we are a model we need to copy it to the model group
	TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(start);
	if (tc->GetType() == TreeController::CONTROLLERTYPE::CT_MODEL && GetTreeItemType(start) == TreeController::CONTROLLERTYPE::CT_MODELROOT)
	{
		for (int j = 0; j < _modelLookup[tc->ModelName()].size(); j++)
		{
			wxTreeListItem tli = (_modelLookup[tc->ModelName()][j]->GetTreeListItem());
			if (tli == start)
			{
				// dont do anything ... this is the node we are already processing
			}
			else
			{
				// apply this state to other instances of this model
				TreeListCtrl_Channels->CheckItem(tli, start_state);
			}
		}
	}

	return start_state;
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
bool TestDialog::CheckChannel(long chid, wxCheckBoxState state)
{
	bool rc = true;

	std::vector<TreeController*> chs = _channelLookup[chid];

	for (int i = 0; i < chs.size(); i++)
	{
		// skip if inactive or a NULL controller
		if (chs[i] != NULL && chs[i]->Clickable())
		{
			TreeListCtrl_Channels->CheckItem((chs[i]->GetTreeListItem()), state);
		}
		else
		{
			rc = false;
		}
	}

	return rc;
}

void TestDialog::Clear(wxTreeListItem& item)
{
	TreeListCtrl_Channels->CheckItem(item, wxCHK_UNCHECKED);

	wxTreeListItem i = TreeListCtrl_Channels->GetFirstChild(item);
	while (i != NULL)
	{
		Clear(i);
		i = TreeListCtrl_Channels->GetNextSibling(i);
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
	Clear(_controllers);
	Clear(_modelGroups);
	Clear(_models);
	wxString name = dialog.GetStringSelection();
	wxString chidstr;
	long chid;
	TreeController* rootcb = (TreeController*)TreeListCtrl_Channels->GetItemData(_controllers);
	long ChCount = rootcb->EndXLightsChannel();
	wxXmlNode* root = _network->GetRoot();
	for (wxXmlNode* e = root->GetChildren(); e != NULL; e = e->GetNext())
	{
		if (e->GetName() == "testpreset" && e->GetAttribute("name", "") == name)
		{
			for (wxXmlNode* c = e->GetChildren(); c != NULL; c = c->GetNext())
			{
				if (c->GetName() == "channel" && c->GetAttribute("id", &chidstr) && chidstr.ToLong(&chid) && chid >= 0 && chid < ChCount)
				{
					CheckChannel(chid, wxCHK_CHECKED);
				}
			}
			break;
		}
	}
	RollUpAll(_controllers);
	RollUpAll(_models);
	RollUpAll(_modelGroups);

	_checkChannelList = true;
}

// for each model copy down the does not exist flag from the first to subsequent items
void TestDialog::CascadeModelDoesNotExist()
{
	for (auto it = _modelLookup.begin(); it != _modelLookup.end(); ++it)
	{
		bool doesnotexist = false;
		for (int i = 0; i < it->second.size(); i++)
		{
			if (i == 0)
			{
				doesnotexist = it->second[i]->IsDoesNotExist();
			}
			else
			{
				if (doesnotexist)
				{
					it->second[i]->DoesNotExist();
				}
			}
		}
	}

	// Now go through all the model groups ... if all models are not clickable then make the model group not clickable
	wxTreeListItem i = TreeListCtrl_Channels->GetFirstChild(_modelGroups);
	while (i != NULL)
	{
		bool noneclickable = true;

		wxTreeListItem j = TreeListCtrl_Channels->GetFirstChild(i);
		while (j != NULL)
		{
			TreeController* tc = (TreeController *)TreeListCtrl_Channels->GetItemData(j);
			if (tc->Clickable())
			{
				noneclickable = false;
				break;
			}
			j = TreeListCtrl_Channels->GetNextSibling(j);
		}

		if (noneclickable)
		{
			TreeController* tc = (TreeController *)TreeListCtrl_Channels->GetItemData(i);
			tc->DoesNotExist();
		}

		i = TreeListCtrl_Channels->GetNextSibling(i);
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
		TreeController* rootcb = (TreeController*)TreeListCtrl_Channels->GetItemData(_controllers);
		long ChCount = rootcb->EndXLightsChannel();
		//int ChCount = CheckListBoxTestChannels->GetCount();

		wxTreeListItem c = TreeListCtrl_Channels->GetFirstChild(_controllers);
		while (c != NULL)
		{
			TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(c);
			if (tc->Clickable())
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

void TestDialog::OnSlider_SpeedCmdSliderUpdated(wxScrollEvent& event)
{
}

void TestDialog::OnRadioButton_RGB_OffSelect(wxCommandEvent& event)
{
	_testFunc = OFF;
}

void TestDialog::OnRadioButton_RGB_ChaseSelect(wxCommandEvent& event)
{
	_testFunc = CHASE;
	_chaseGrouping = std::numeric_limits<int>::max();
}

void TestDialog::OnRadioButton_RGB_Chase13Select(wxCommandEvent& event)
{
	_testFunc = CHASE;
	_chaseGrouping = 3;
}

void TestDialog::OnRadioButton_RGB_Chase14Select(wxCommandEvent& event)
{
	_testFunc = CHASE;
	_chaseGrouping = 4;
}

void TestDialog::OnRadioButton_RGB_Chase15Select(wxCommandEvent& event)
{
	_testFunc = CHASE;
	_chaseGrouping = 5;
}

void TestDialog::OnRadioButton_RGB_AlternateSelect(wxCommandEvent& event)
{
	_testFunc = CHASE;
	_chaseGrouping = 2;
}

void TestDialog::OnRadioButton_RGB_Twinkle5Select(wxCommandEvent& event)
{
	_testFunc = TWINKLE;
	_twinkleRatio = 20;
}

void TestDialog::OnRadioButton_RGB_Twinkle10Select(wxCommandEvent& event)
{
	_testFunc = TWINKLE;
	_twinkleRatio = 10;
}

void TestDialog::OnRadioButton_RGB_Twinkle25Select(wxCommandEvent& event)
{
	_testFunc = TWINKLE;
	_twinkleRatio = 4;
}

void TestDialog::OnRadioButton_RGB_Twinkle50Select(wxCommandEvent& event)
{
	_testFunc = TWINKLE;
	_twinkleRatio = 2;
}

void TestDialog::OnRadioButton_RGB_ShimmerSelect(wxCommandEvent& event)
{
	_testFunc = SHIMMER;
}

void TestDialog::OnRadioButton_RGB_BackgroundSelect(wxCommandEvent& event)
{
	_testFunc = DIM;
}

void TestDialog::OnRadioButton_RGBCycle_OffSelect(wxCommandEvent& event)
{
	_testFunc = OFF;
}

void TestDialog::OnRadioButton_RGBCycle_ABCSelect(wxCommandEvent& event)
{
	_testFunc = CHASE;
	_chaseGrouping = 3;
}

void TestDialog::OnRadioButton_RGBCycle_ABCAllSelect(wxCommandEvent& event)
{
	_testFunc = CHASE;
	_chaseGrouping = 4;
}

void TestDialog::OnRadioButton_RGBCycle_ABCAllNoneSelect(wxCommandEvent& event)
{
	_testFunc = CHASE;
	_chaseGrouping = 5;
}

void TestDialog::OnRadioButton_RGBCycle_MixedColorsSelect(wxCommandEvent& event)
{
	_testFunc = DIM;
}

void TestDialog::OnRadioButton_Standard_OffSelect(wxCommandEvent& event)
{
	_testFunc = OFF;
}

void TestDialog::OnRadioButton_Standard_ChaseSelect(wxCommandEvent& event)
{
	_testFunc = CHASE;
	_chaseGrouping = std::numeric_limits<int>::max();
}

void TestDialog::OnRadioButton_Standard_Chase13Select(wxCommandEvent& event)
{
	_testFunc = CHASE;
	_chaseGrouping = 3;
}

void TestDialog::OnRadioButton_Standard_Chase14Select(wxCommandEvent& event)
{
	_testFunc = CHASE;
	_chaseGrouping = 4;
}

void TestDialog::OnRadioButton_Standard_Chase15Select(wxCommandEvent& event)
{
	_testFunc = CHASE;
	_chaseGrouping = 5;
}

void TestDialog::OnRadioButton_Standard_AlternateSelect(wxCommandEvent& event)
{
	_testFunc = CHASE;
	_chaseGrouping = 2;
}

void TestDialog::OnRadioButton_Standard_Twinkle5Select(wxCommandEvent& event)
{
	_testFunc = TWINKLE;
	_twinkleRatio = 20;
}

void TestDialog::OnRadioButton_Standard_Twinkle10Select(wxCommandEvent& event)
{
	_testFunc = TWINKLE;
	_twinkleRatio = 10;
}

void TestDialog::OnRadioButton_Standard_Twinkle25Select(wxCommandEvent& event)
{
	_testFunc = TWINKLE;
	_twinkleRatio = 4;
}

void TestDialog::OnRadioButton_Standard_Twinkle50Select(wxCommandEvent& event)
{
	_testFunc = TWINKLE;
	_twinkleRatio = 2;
}

void TestDialog::OnRadioButton_Standard_ShimmerSelect(wxCommandEvent& event)
{
	_testFunc = SHIMMER;
}

void TestDialog::OnRadioButton_Standard_BackgroundSelect(wxCommandEvent& event)
{
	_testFunc = DIM;
}

void TestDialog::OnRadioButton_RGBCycle_RGBWSelect(wxCommandEvent& event)
{
    _testFunc = RGBW;
}

void TestDialog::GetCheckedItems(wxArrayInt& chArray)
{
	chArray.Clear();

	for (auto ch = _channelLookup.begin(); ch != _channelLookup.end(); ++ch)
	{
		if (ch->second.size() > 0)
		{
			if (TreeListCtrl_Channels->GetCheckedState((ch->second[0]->GetTreeListItem())))
			{
				chArray.Add(ch->first);
			}
		}
	}
}

void TestDialog::GetCheckedItems(wxArrayInt& chArray, char col)
{
	chArray.Clear();

	for (auto ch = _channelLookup.begin(); ch != _channelLookup.end(); ++ch)
	{
		if (ch->second.size() > 0)
		{
			if (TreeListCtrl_Channels->GetCheckedState((ch->second[0]->GetTreeListItem())) && ch->second[0]->GetColour() == col)
			{
				chArray.Add(ch->first);
			}
		}
	}
}

bool TestDialog::InitialiseOutputs()
{
	wxCriticalSectionLocker locker(_xoutCriticalSection);
	long MaxChan;
	bool ok = true;

	for (wxXmlNode* e = _network->GetRoot()->GetChildren(); e != NULL && ok; e = e->GetNext())
	{
		wxString tagname = e->GetName();
		if (tagname == "network")
		{
			wxString tempstr = e->GetAttribute("MaxChannels", "0");
			tempstr.ToLong(&MaxChan);
			wxString NetworkType = e->GetAttribute("NetworkType", "");
			wxString ComPort = e->GetAttribute("ComPort", "");
			wxString BaudRate = e->GetAttribute("BaudRate", "");
			int baud = (BaudRate == _("n/a")) ? 115200 : wxAtoi(BaudRate);
			bool enabled = e->GetAttribute("Enabled", "Yes") == "Yes";
			wxString Description = e->GetAttribute("Description", "");
			static wxString choices;

			int numU = wxAtoi(e->GetAttribute("NumUniverses", "1"));

#ifdef __WXMSW__ //TODO: enumerate comm ports on all platforms -DJ
			TCHAR valname[32];
			/*byte*/TCHAR portname[32];
			DWORD vallen = sizeof(valname);
			DWORD portlen = sizeof(portname);
			HKEY hkey = NULL;
			DWORD err = 0;

			//enum serial comm ports (more user friendly, especially if USB-to-serial ports change):
			//logic based on http://www.cplusplus.com/forum/windows/73821/
			if (choices.empty()) //should this be cached?  it's not really that expensive
			{
				if (!(err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_READ, &hkey)))
					for (DWORD inx = 0; !(err = RegEnumValue(hkey, inx, (LPTSTR)valname, &vallen, NULL, NULL, (LPBYTE)portname, &portlen)) || (err == ERROR_MORE_DATA); ++inx)
					{
						if (err == ERROR_MORE_DATA) portname[sizeof(portname) / sizeof(portname[0]) - 1] = '\0'; //need to enlarge read buf if this happens; just truncate string for now
																													//                            debug(3, "found port[%d] %d:'%s' = %d:'%s', err 0x%x", inx, vallen, valname, portlen, portname, err);
						choices += _(", ") + portname;
						vallen = sizeof(valname);
						portlen = sizeof(portname);
					}
				if (err && (err != /*ERROR_FILE_NOT_FOUND*/ ERROR_NO_MORE_ITEMS)) choices = wxString::Format(", error %d (can't get serial comm ports from registry)", err);
				if (hkey) RegCloseKey(hkey);
				//                    if (err) SetLastError(err); //tell caller about last real error
				if (!choices.empty()) choices = "\n(available ports: " + choices.substr(2) + ")";
				else choices = "\n(no available ports)";
			}
#endif // __WXMSW__
			wxString msg = _("Error occurred while connecting to ") + NetworkType + _(" network on ") + ComPort +
				choices +
				_("\n\nThings to check:\n1. Are all required cables plugged in?") +
				_("\n2. Is there another program running that is accessing the port (like the LOR Control Panel)? If so, then you must close the other program and then restart xLights.") +
				_("\n3. If this is a USB dongle, are the FTDI Virtual COM Port drivers loaded?\n\n");

			try
			{
				_xout->addnetwork(NetworkType, MaxChan, ComPort, baud, numU, enabled);
			}
			catch (const char *str)
			{
				wxString errmsg(str, wxConvUTF8);
				if (wxMessageBox(msg + errmsg + _("\nProceed anyway?"), _("Communication Error"), wxYES_NO | wxNO_DEFAULT) != wxYES)
					ok = false;
			}
		}
	}
	return ok;
}

void TestDialog::OnTimer1Trigger(wxTimerEvent& event)
{
	if (!_xoutCriticalSection.TryEnter() || _xout == NULL)
	{
		return;
	}
	wxTimeSpan ts = wxDateTime::UNow() - _starttime;
	long curtime = ts.GetMilliseconds().ToLong();
	_xout->TimerStart(curtime);
	OnTimer(curtime);
	_xout->TimerEnd();
	_xoutCriticalSection.Leave();
}

void TestDialog::TestButtonsOff()
{
	RadioButton_Standard_Off->SetValue(true);
	RadioButton_Standard_Chase->SetValue(false);
	RadioButton_Standard_Chase13->SetValue(false);
	RadioButton_Standard_Chase14->SetValue(false);
	RadioButton_Standard_Chase15->SetValue(false);
	RadioButton_Standard_Alternate->SetValue(false);
	RadioButton_Standard_Twinkle5->SetValue(false);
	RadioButton_Standard_Twinkle10->SetValue(false);
	RadioButton_Standard_Twinkle25->SetValue(false);
	RadioButton_Standard_Twinkle50->SetValue(false);
	RadioButton_Standard_Shimmer->SetValue(false);
	RadioButton_Standard_Background->SetValue(false);

	RadioButton_RGB_Off->SetValue(true);
	RadioButton_RGB_Chase->SetValue(false);
	RadioButton_RGB_Chase13->SetValue(false);
	RadioButton_RGB_Chase14->SetValue(false);
	RadioButton_RGB_Chase15->SetValue(false);
	RadioButton_RGB_Alternate->SetValue(false);
	RadioButton_RGB_Twinkle5->SetValue(false);
	RadioButton_RGB_Twinkle10->SetValue(false);
	RadioButton_RGB_Twinkle25->SetValue(false);
	RadioButton_RGB_Twinkle50->SetValue(false);
	RadioButton_RGB_Shimmer->SetValue(false);
	RadioButton_RGB_Background->SetValue(false);

	RadioButton_RGBCycle_Off->SetValue(true);
	RadioButton_RGBCycle_ABC->SetValue(false);
	RadioButton_RGBCycle_ABCAll->SetValue(false);
	RadioButton_RGBCycle_ABCAllNone->SetValue(false);
	RadioButton_RGBCycle_MixedColors->SetValue(false);
	RadioButton_RGBCycle_RGBW->SetValue(false);

	_testFunc = OFF;
}

void TestDialog::OnTimer(long curtime)
{
	static int LastNotebookSelection = -1;
	static int LastBgIntensity, LastFgIntensity, LastBgColor[3], LastFgColor[3], *ShimColor, ShimIntensity;
	static int LastSequenceSpeed;
	static int LastTwinkleRatio;
	static int LastAutomatedTest;
	static long NextSequenceStart = -1;
	static TestFunctions LastFunc = OFF;
	static unsigned int interval, rgbCycle, TestSeqIdx;
	static wxArrayInt chArray, chArrayR, chArrayG, chArrayB, chArrayW, TwinkleState;
	static float frequency;
	int v, BgIntensity, FgIntensity, BgColor[3], FgColor[3];
	unsigned int i;
	bool ColorChange;

	if (_xout == NULL)
	{
		return;
	}

	//_xout->TimerStart(curtime);
	int NotebookSelection = AuiNotebook1->GetSelection();
	if (NotebookSelection != LastNotebookSelection)
	{
		LastNotebookSelection = NotebookSelection;
		_checkChannelList = true;
		TestSeqIdx = 0;
		TestButtonsOff();
	}
	if (_testFunc != LastFunc)
	{
		LastFunc = _testFunc;
		rgbCycle = 0;
		_checkChannelList = true;
		NextSequenceStart = -1;
	}

	if (_checkChannelList)
	{
		// get list of checked channels
		_xout->alloff();
		GetCheckedItems(chArray);
		if (RadioButton_RGB_Chase->GetValue() > 0 || RadioButton_Standard_Chase->GetValue() > 0)
		{
			_chaseGrouping = chArray.Count();
			if (_chaseGrouping == 0)
			{
				_chaseGrouping = std::numeric_limits<int>::max();
			}
		}
		if (RadioButton_RGBCycle_RGBW->GetValue() > 0)
		{
			GetCheckedItems(chArrayR, 'R');
			GetCheckedItems(chArrayG, 'G');
			GetCheckedItems(chArrayB, 'B');
			GetCheckedItems(chArrayW, 'W');
		}

		LastSequenceSpeed = -1;
		LastBgIntensity = -1;
		LastFgIntensity = -1;
		LastAutomatedTest = -1;
		LastTwinkleRatio = -1;
		for (i = 0; i < 3; i++)
		{
			LastBgColor[i] = -1;
			LastFgColor[i] = -1;
		}
		if (_testFunc == OFF)
		{
			StatusBar1->SetLabelText(_("Testing off"));
		}
		else
		{
			StatusBar1->SetLabelText(wxString::Format(_("Testing %ld channels"), static_cast<long>(chArray.Count())));
		}
		_checkChannelList = false;
	}

	if (_testFunc != OFF && chArray.Count() > 0) switch (NotebookSelection)
	{
	case 0:
		// standard tests
		v = Slider_Speed->GetValue();  // 0-100
		BgIntensity = Slider_Standard_Background->GetValue();
		FgIntensity = Slider_Standard_Highlight->GetValue();
		ColorChange = BgIntensity != LastBgIntensity || FgIntensity != LastFgIntensity;
		LastBgIntensity = BgIntensity;
		LastFgIntensity = FgIntensity;
		interval = 1600 - v * 15;

		switch (_testFunc)
		{
		case DIM:
			if (ColorChange)
			{
				for (i = 0; i < chArray.Count(); i++)
				{
					_xout->SetIntensity(chArray[i], BgIntensity);
				}
			}
			break;

		case TWINKLE:
			if (LastSequenceSpeed < 0 || _twinkleRatio != LastTwinkleRatio)
			{
				LastSequenceSpeed = 0;
				TwinkleState.Clear();
				for (i = 0; i < chArray.Count(); i++)
				{
					TestSeqIdx = static_cast<int>(Rand01()*(double)_twinkleRatio);
					TwinkleState.Add(TestSeqIdx == 0 ? -1 : 1);
				}
			}
			for (i = 0; i < TwinkleState.Count(); i++)
			{
				if (TwinkleState[i] < -1)
				{
					// background
					TwinkleState[i]++;
				}
				else if (TwinkleState[i] > 1)
				{
					// highlight
					TwinkleState[i]--;
				}
				else if (TwinkleState[i] == -1)
				{
					// was background, now highlight for random period
					TwinkleState[i] = static_cast<int>(Rand01()*(double)interval + 100.0) / (double)_seqData.FrameTime();
					_xout->SetIntensity(chArray[i], FgIntensity);
				}
				else
				{
					// was on, now go to bg color for random period
					TwinkleState[i] = -static_cast<int>(Rand01()*(double)interval + 100.0) / (double)_seqData.FrameTime() * ((double)_twinkleRatio - 1.0);
					_xout->SetIntensity(chArray[i], BgIntensity);
				}
			}
			break;

		case SHIMMER:
			if (ColorChange || curtime >= NextSequenceStart)
			{
				ShimIntensity = (ShimIntensity == FgIntensity) ? BgIntensity : FgIntensity;
				for (i = 0; i < chArray.Count(); i++)
				{
					_xout->SetIntensity(chArray[i], ShimIntensity);
				}
			}
			if (curtime >= NextSequenceStart)
			{
				NextSequenceStart = curtime + interval / 2;
			}
			break;

		case CHASE:
			//StatusBar1->SetStatusText(wxString::Format(_("chase curtime=%ld, NextSequenceStart=%ld"),curtime,NextSequenceStart));
			if (ColorChange || curtime >= NextSequenceStart)
			{
				for (i = 0; i < chArray.Count(); i++)
				{
					v = (i % _chaseGrouping) == TestSeqIdx ? FgIntensity : BgIntensity;
					_xout->SetIntensity(chArray[i], v);
				}
			}
			if (curtime >= NextSequenceStart)
			{
				NextSequenceStart = curtime + interval;
				TestSeqIdx = (TestSeqIdx + 1) % _chaseGrouping;
				if (TestSeqIdx >= chArray.Count()) TestSeqIdx = 0;
			}
			StatusBar1->SetLabelText(wxString::Format(_("Testing %ld channels; chase now at ch# %d"), static_cast<long>(chArray.Count()), TestSeqIdx)); //show current ch# -DJ
			break;
		default:
			break;
		}
		break;

	case 1:
		// RGB tests
		v = Slider_Speed->GetValue();  // 0-100
		BgColor[0] = Slider_RGB_BG_R->GetValue();
		BgColor[1] = Slider_RGB_BG_G->GetValue();
		BgColor[2] = Slider_RGB_BG_B->GetValue();
		FgColor[0] = Slider_RGB_H_R->GetValue();
		FgColor[1] = Slider_RGB_H_G->GetValue();
		FgColor[2] = Slider_RGB_H_B->GetValue();

		interval = 1600 - v * 15;
		for (ColorChange = false, i = 0; i < 3; i++)
		{
			ColorChange |= (BgColor[i] != LastBgColor[i]);
			ColorChange |= (FgColor[i] != LastFgColor[i]);
			LastBgColor[i] = BgColor[i];
			LastFgColor[i] = FgColor[i];
		}
		switch (_testFunc)
		{
		case DIM:
			if (ColorChange)
			{
				for (i = 0; i < chArray.Count(); i++)
				{
					_xout->SetIntensity(chArray[i], BgColor[i % 3]);
				}
			}
			break;

		case TWINKLE:
			if (LastSequenceSpeed < 0 || LastTwinkleRatio != _twinkleRatio)
			{
				LastSequenceSpeed = 0;
				TwinkleState.Clear();
				for (i = 0; i < chArray.Count() - 2; i += 3)
				{
					TestSeqIdx = static_cast<int>(Rand01()*(double)_twinkleRatio);
					TwinkleState.Add(TestSeqIdx == 0 ? -1 : 1);
				}
			}
			for (i = 0; i < TwinkleState.Count(); i++)
			{
				if (TwinkleState[i] < -1)
				{
					// background
					TwinkleState[i]++;
				}
				else if (TwinkleState[i] > 1)
				{
					// highlight
					TwinkleState[i]--;
				}
				else if (TwinkleState[i] == -1)
				{
					// was background, now highlight for random period
					TwinkleState[i] = static_cast<int>(Rand01()*(double)interval + 100.0) / (double)_seqData.FrameTime();
					TestSeqIdx = i * 3;
					_xout->SetIntensity(chArray[TestSeqIdx], FgColor[0]);
					_xout->SetIntensity(chArray[TestSeqIdx + 1], FgColor[1]);
					_xout->SetIntensity(chArray[TestSeqIdx + 2], FgColor[2]);
				}
				else
				{
					// was on, now go to bg color for random period
					TwinkleState[i] = -static_cast<int>(Rand01()*(double)interval + 100.0) / (double)_seqData.FrameTime() * ((double)_twinkleRatio - 1.0);
					TestSeqIdx = i * 3;
					_xout->SetIntensity(chArray[TestSeqIdx], BgColor[0]);
					_xout->SetIntensity(chArray[TestSeqIdx + 1], BgColor[1]);
					_xout->SetIntensity(chArray[TestSeqIdx + 2], BgColor[2]);
				}
			}
			break;
		case SHIMMER:
			if (ColorChange || curtime >= NextSequenceStart)
			{
				ShimColor = (ShimColor == FgColor) ? BgColor : FgColor;
				for (i = 0; i < chArray.Count(); i++)
				{
					_xout->SetIntensity(chArray[i], ShimColor[i % 3]);
				}
			}
			if (curtime >= NextSequenceStart)
			{
				NextSequenceStart = curtime + interval / 2;
			}
			break;
		case CHASE:
			if (ColorChange || curtime >= NextSequenceStart)
			{
				for (i = 0; i < chArray.Count(); i++)
				{
					v = (i / 3 % _chaseGrouping) == TestSeqIdx ? FgColor[i % 3] : BgColor[i % 3];
					_xout->SetIntensity(chArray[i], v);
				}
			}
			if (curtime >= NextSequenceStart)
			{
				NextSequenceStart = curtime + interval;
				TestSeqIdx = (TestSeqIdx + 1) % _chaseGrouping;
				if (TestSeqIdx >= (chArray.Count() + 2) / 3) TestSeqIdx = 0;
			}
			StatusBar1->SetLabelText(wxString::Format(_("Testing %ld channels; chase now at ch# %d"), static_cast<long>(chArray.Count()), TestSeqIdx)); //show current ch# -DJ
			break;
		default:
			break;
		}
		break;

	case 2:
		// RGB Cycle
		v = Slider_Speed->GetValue();  // 0-100
		if (_testFunc == DIM)
		{
			// color mixing
			if (v != LastSequenceSpeed)
			{
				frequency = v / 1000.0 + 0.05;
				LastSequenceSpeed = v;
			}
			BgColor[0] = sin(frequency*TestSeqIdx + 0.0) * 127 + 128;
			BgColor[1] = sin(frequency*TestSeqIdx + 2.0) * 127 + 128;
			BgColor[2] = sin(frequency*TestSeqIdx + 4.0) * 127 + 128;
			TestSeqIdx++;
			for (i = 0; i < chArray.Count(); i++)
			{
				_xout->SetIntensity(chArray[i], BgColor[i % 3]);
			}
		}
		else if (_testFunc == RGBW)
		{
			if (v != LastSequenceSpeed)
			{
				interval = (101 - v) * 50;
				NextSequenceStart = curtime + interval;
				LastSequenceSpeed = v;
			}
			if (curtime >= NextSequenceStart)
			{
				// blank everything first
				for (i = 0; i < chArray.Count(); i++)
				{
					_xout->SetIntensity(chArray[i], 0);
				}
				switch (rgbCycle)
				{
				case 0: // red
					for (i = 0; i < chArrayR.Count(); i++)
					{
						_xout->SetIntensity(chArrayR[i], 255);
					}
					break;
				case 1: // green
					for (i = 0; i < chArrayG.Count(); i++)
					{
						_xout->SetIntensity(chArrayG[i], 255);
					}
					break;
				case 2: // blue
					for (i = 0; i < chArrayB.Count(); i++)
					{
						_xout->SetIntensity(chArrayB[i], 255);
					}
					break;
				case 3: // white
					for (i = 0; i < chArrayW.Count(); i++)
					{
						_xout->SetIntensity(chArrayW[i], 255);
					}
					break;
				}
				rgbCycle = (rgbCycle + 1) % 4;
				NextSequenceStart += interval;
			}
		}
		else
		{
			// RGB cycle
			if (v != LastSequenceSpeed)
			{
				interval = (101 - v) * 50;
				NextSequenceStart = curtime + interval;
				LastSequenceSpeed = v;
			}
			if (curtime >= NextSequenceStart)
			{
				for (i = 0; i < chArray.Count(); i++)
				{
					switch (rgbCycle)
					{
					case 3:
						v = 255;
						break;
					default:
						v = (i % 3) == rgbCycle ? 255 : 0;
						break;
					}
					_xout->SetIntensity(chArray[i], v);
				}
				rgbCycle = (rgbCycle + 1) % _chaseGrouping;
				NextSequenceStart += interval;
			}
		}
		break;
	}
	//_xout->TimerEnd();
}

char TestDialog::DoEncodeColour(xlColor& c)
{
	if (c.red > 0 && c.green == 0 && c.blue == 0)
	{
		return 'R';
	}
	if (c.red == 0 && c.green > 0 && c.blue == 0)
	{
		return 'G';
	}
	if (c.red == 0 && c.green == 0 && c.blue > 0)
	{
		return 'B';
	}
	if (c.red > 0 && c.red == c.green && c.red == c.blue)
	{
		return 'W';
	}

	return 'X';
}

void TestDialog::OnCheckBox_OutputToLightsClick(wxCommandEvent& event)
{
	if (CheckBox_OutputToLights->IsChecked())
	{
		if (_xout == NULL)
		{
			_xout = new xOutput();
			InitialiseOutputs();
		}
	}
	else
	{
		if (_xout)
		{
			_xout->alloff();
			delete _xout;
			_xout = NULL;
		}
	}
}
