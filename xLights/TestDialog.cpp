#include "TestDialog.h"

#include <wx/choicdlg.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/settings.h>
#include <wx/dataview.h>
#include <wx/confbase.h>

//(*InternalHeaders(TestDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "models/Model.h"
#include "models/ModelGroup.h"
#include <log4cpp/Category.hh>
#include "osxMacUtils.h"
#include "xLightsXmlFile.h"
#include "outputs/TestPreset.h"
#include "outputs/Output.h"
#include "UtilFunctions.h"

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
    _output = nullptr;
	_colour = ' ';
	_type = type;
	_startchannel = channel;
	_startxlightschannel = xLightsChannel;
    _endxlightschannel = xLightsChannel;
	_inactive = false;
	_nodes = -1;
	_doesNotExist = false;
	_name = GenerateName();
}

// This for for a node node
TreeController::TreeController(CONTROLLERTYPE type, int xLightsChannel, int node, int channelspernode)
{
    _output = nullptr;
    _colour = ' ';
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
    _output = nullptr;
    _colour = ' ';
	_inactive = false;
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
    _output = nullptr;
    _colour = ' ';
	_inactive = false;
	_type = type;
	_startxlightschannel = start;
	_endxlightschannel = end;
	_nodes = -1;
	_doesNotExist = false;
	_name = GenerateName();
}

// This is for a DMX/E131 multiple node
TreeController::TreeController(CONTROLLERTYPE type, std::string comport, int universe, std::string ipaddress, int startxlightschannel, int channels, bool inactive, bool multiuniversedmx, std::string description)
{
    _output = nullptr;
    _ipaddress = ipaddress;
	_colour = ' ';
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
TreeController::TreeController(Output* output): _nodeNumber(0)
{
    _output = output;
    _colour = ' ';
    _doesNotExist = false;
    _nodes = -1;
    _startxlightschannel = output->GetStartChannel();
    _endchannel = output->GetChannels();
    _startchannel = 1;
    _endxlightschannel = output->GetEndChannel();
    _inactive = !output->IsEnabled();
    _description = output->GetDescription();
    _ipaddress = output->GetIP();
    _universe = wxString::Format(wxT("%i"), output->GetUniverse());
    _comport = output->GetCommPort();
    _baudrate = wxString::Format(wxT("%i"), output->GetBaudRate());
    _type = CONTROLLERTYPE::CT_CONTROLLER;
    _name = output->GetLongDescription();
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
	case CONTROLLERTYPE::CT_CONTROLLER:
        _name = _output->GetLongDescription();
		break;
	case CONTROLLERTYPE::CT_MODEL:
		if (!Clickable())
		{
			_name += "UNAVAILABLE ";
		}
		_name += _modelName;
        if (_nodes > 0)
        {
            if (_nodes == 1)
            {
                _name += " [1]";
            }
            else
            {
                _name += " [1-" + std::string(wxString::Format(wxT("%i"), _nodes)) + "]";
            }
        }
		if (_startxlightschannel < 1)
		{
			// dont add anything
		}
		else if (_endxlightschannel <= _startxlightschannel)
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

TestDialog::TestDialog(wxWindow* parent, OutputManager* outputManager, wxFileName networkFile, ModelManager* modelManager, wxWindowID id)
{
	_outputManager = outputManager;
	_networkFile = networkFile;
	_modelManager = modelManager;
	_checkChannelList = false;
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
	Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&TestDialog::OnTimer1Trigger);
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&TestDialog::OnClose);
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
	Connect(ID_TREELISTCTRL_Channels, wxEVT_COMMAND_TREELIST_SELECTION_CHANGED, (wxObjectEventFunction)&TestDialog::OnTreeListCtrl1ItemSelected);
#ifdef __WXOSX__
	Connect(ID_TREELISTCTRL_Channels, wxEVT_COMMAND_TREELIST_ITEM_ACTIVATED, (wxObjectEventFunction)&TestDialog::OnTreeListCtrl1ItemActivated);
    //work around http://trac.wxwidgets.org/ticket/17409
    TreeListCtrl_Channels->GetDataView()->SetIndent(8);
#endif

	PopulateControllerTree();
	PopulateModelsTree(_modelManager);
	PopulateModelGroupsTree(_modelManager);
	CascadeModelDoesNotExist();
	DeactivateNotClickableModels();

	if (TreeListCtrl_Channels->GetFirstChild(_controllers) == nullptr)
	{
		TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(_controllers);
		tc->DoesNotExist();
	}
	if (TreeListCtrl_Channels->GetFirstChild(_models) == nullptr)
	{
		TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(_models);
		tc->DoesNotExist();
	}
	if (TreeListCtrl_Channels->GetFirstChild(_modelGroups) == nullptr)
	{
		TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(_modelGroups);
		tc->DoesNotExist();
	}

    wxConfigBase* config = wxConfigBase::Get();
    DeserialiseSettings(config->Read("xLightsTestSettings").ToStdString());

	_starttime = wxDateTime::UNow();
	DisableSleepModes();

    CheckBox_OutputToLights->SetValue(true);
    if (!_outputManager->StartOutput())
    {
        wxMessageBox("At least one output could not be started. See log file for details.", "Warning");
    }
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

void TestDialog::AddController(Output* output)
{
    TreeController* controller = new TreeController(output);

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
}

void TestDialog::PopulateControllerTree()
{
    auto outputs = _outputManager->GetOutputs();
    for (auto e = outputs.begin(); e != outputs.end(); ++e)
    {
        if ((*e)->IsOutputCollection())
        {
            auto suboutputs = (*e)->GetOutputs();
            for (auto e1 = suboutputs.begin(); e1 != suboutputs.end(); ++e1)
            {
                AddController(*e1);
            }
        }
        else
        {
            AddController(*e);
        }
    }
	TreeListCtrl_Channels->Expand(_controllers);

	TreeController* root = new TreeController(TreeController::CONTROLLERTYPE::CT_CONTROLLERROOT, 1, _outputManager->GetTotalChannels());
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
			int modelstartchannel = m->GetNumberFromChannelString(m->ModelStartChannel);
            int msc = m->GetNumberFromChannelString(m->ModelStartChannel);

			if (m->SingleChannel)
			{
                modelcontroller->SetNodes(m->GetNodeCount());
                for (int i = 0; i < m->GetNodeCount(); i++)
                {
                    TreeController* tc = new TreeController(i+1, TreeController::CONTROLLERTYPE::CT_CHANNEL, msc + i);
                    xlColor col(m->GetNodeColor(i));
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

	for (auto it = _modelManager->begin(); it != _modelManager->end(); ++it)
	{
		Model* m = it->second;
        if (m->GetDisplayAs() != "ModelGroup")
        {
            int st = m->GetFirstChannel();
            int en = m->GetLastChannel();
            if (start <= en + 1 && end >= st)
            {
                res.push_back(it->first);
            }
        }
	}

	return res;
}

void TestDialog::SetTreeTooltip(wxTreeListItem& item)
{
    if (item == _controllers || item == _modelGroups || item == _models)
    {
#ifdef __WXOSX__
        TreeListCtrl_Channels->UnsetToolTip();
#else
        TreeListCtrl_Channels->GetView()->UnsetToolTip();
#endif
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
#ifdef __WXOSX__
                TreeListCtrl_Channels->SetToolTip(tt);
#else
                TreeListCtrl_Channels->GetView()->SetToolTip(tt);
#endif
            }
            else
            {
#ifdef __WXOSX__
                TreeListCtrl_Channels->UnsetToolTip();
#else
                TreeListCtrl_Channels->GetView()->UnsetToolTip();
#endif
            }
        }
        else
        {
#ifdef __WXOSX__
            TreeListCtrl_Channels->UnsetToolTip();
#else
            TreeListCtrl_Channels->GetView()->UnsetToolTip();
#endif
        }
    }
}

void TestDialog::OnTreeListCtrl1ItemSelected(wxTreeListEvent& event)
{
    wxTreeListItem item = event.GetItem();
    SetTreeTooltip(item);
}

void TestDialog::OnTreeListCtrl1ItemActivated(wxTreeListEvent& event)
{
    //On Mac, the checkboxes aren't working, we'll fake it with the double click activations
    wxTreeListItem item = event.GetItem();
    wxCheckBoxState checked = TreeListCtrl_Channels->GetCheckedState(item);
    if (checked != wxCHK_CHECKED) {
        TreeListCtrl_Channels->CheckItem(item, wxCHK_CHECKED);
    } else {
        TreeListCtrl_Channels->CheckItem(item, wxCHK_UNCHECKED);
    }
    OnTreeListCtrl1Checkboxtoggled(event);

    SetTreeTooltip(item);
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

    SetTreeTooltip(item);
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

// Find a given xlightschannel in the treelist and check it if it is not inactive or in a null controller
bool TestDialog::CheckChannel(long chid, wxCheckBoxState state)
{
	bool rc = true;

	std::vector<TreeController*> chs = _channelLookup[chid];

	for (int i = 0; i < chs.size(); i++)
	{
		// skip if inactive or a NULL controller
		if (chs[i] != nullptr && chs[i]->Clickable())
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
	while (i != nullptr)
	{
		Clear(i);
		i = TreeListCtrl_Channels->GetNextSibling(i);
	}
}

void TestDialog::OnButton_LoadClick(wxCommandEvent& event)
{
    auto presets = _outputManager->GetTestPresets();

	if (presets.size() == 0)
	{
		wxMessageBox(_("No test configurations found"), _("Error"));
		return;
	}

	// get user selection
	presets.sort();
    wxArrayString PresetNames;
    for (auto it = presets.begin(); it != presets.end(); ++it)
    {
        PresetNames.Add(wxString(it->c_str()));
    }
	wxSingleChoiceDialog dialog(this, _("Select test configuration"), _("Load Test Settings"), PresetNames);

    if (dialog.ShowModal() != wxID_OK) return;

	// re-find testpreset node, then set channels
	Clear(_controllers);
	Clear(_modelGroups);
	Clear(_models);
	wxString name = dialog.GetStringSelection();
	wxString chidstr;
	TreeController* rootcb = (TreeController*)TreeListCtrl_Channels->GetItemData(_controllers);
	long ChCount = rootcb->EndXLightsChannel();

    TestPreset* preset = _outputManager->GetTestPreset(name.ToStdString());

    if (preset == nullptr) return; // this should never happen

    auto chs = preset->GetChannels();
    for (auto c = chs.begin(); c != chs.end(); ++c)
    {
        if (*c > 0 && *c < ChCount)
        CheckChannel(*c, wxCHK_CHECKED);
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
	while (i != nullptr)
	{
		bool noneclickable = true;

		wxTreeListItem j = TreeListCtrl_Channels->GetFirstChild(i);
		while (j != nullptr)
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
    wxTextEntryDialog NameDialog(this, _("Enter a name for this test configuration"), _("Save Test Settings"));
    if (NameDialog.ShowModal() != wxID_OK) return;

    name = NameDialog.GetValue().Trim(true).Trim(false);

    if (name.IsEmpty())
    {
        wxMessageBox(_("Name cannot be empty"), _("Error"));
        return;
    }
    else if (name.Len() > 240)
    {
        wxMessageBox(_("Name is too long"), _("Error"));
        return;
    }
    else if (_outputManager->GetTestPreset(name.ToStdString()) != nullptr)
    {
        if (wxMessageBox(_("Name already exists. Do you want to overwrite it?"), _("Warning"), wxYES_NO) == wxNO)
        {
            return;
        }
    }

    TestPreset* testPreset = _outputManager->CreateTestPreset(name.ToStdString());

    wxTreeListItem c = TreeListCtrl_Channels->GetFirstChild(_controllers);
    while (c != nullptr)
    {
        TreeController* tc = (TreeController*)TreeListCtrl_Channels->GetItemData(c);
        if (tc->Clickable())
        {
            wxTreeListItem cc = TreeListCtrl_Channels->GetFirstChild(c);
            while (cc != nullptr)
            {
                if (TreeListCtrl_Channels->GetCheckedState(cc) == wxCHK_CHECKED)
                {
                    TreeController* tcc = (TreeController*)TreeListCtrl_Channels->GetItemData(cc);

                    testPreset->AddChannel(tcc->StartXLightsChannel());
                }
                cc = TreeListCtrl_Channels->GetNextSibling(cc);
            }
        }
        c = TreeListCtrl_Channels->GetNextSibling(c);
    }

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Saving test preset: %s", (const char*)name.c_str());
    _outputManager->Save();
    logger_base.debug("   Save done.");
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

void TestDialog::OnTimer1Trigger(wxTimerEvent& event)
{
	wxTimeSpan ts = wxDateTime::UNow() - _starttime;
	long curtime = ts.GetMilliseconds().ToLong();
	_outputManager->StartFrame(curtime);
	OnTimer(curtime);
	_outputManager->EndFrame();
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
}

TestDialog::TestFunctions TestDialog::GetTestFunction(int notebookSelection)
{
    switch (notebookSelection)
    {
    case 0:
        if (RadioButton_Standard_Off->GetValue())
        {
            return OFF;
        }
        else if (RadioButton_Standard_Chase->GetValue())
        {
            _chaseGrouping = std::numeric_limits<int>::max();
            return CHASE;
        }
        else if (RadioButton_Standard_Chase13->GetValue())
        {
            _chaseGrouping = 3;
            return CHASE;
        }
        else if (RadioButton_Standard_Chase14->GetValue())
        {
            _chaseGrouping = 4;
            return CHASE;
        }
        else if (RadioButton_Standard_Chase15->GetValue())
        {
            _chaseGrouping = 5;
            return CHASE;
        }
        else if (RadioButton_Standard_Alternate->GetValue())
        {
            _chaseGrouping = 2;
            return CHASE;
        }
        else if (RadioButton_Standard_Twinkle5->GetValue())
        {
            _twinkleRatio = 20;
            return TWINKLE;
        }
        else if (RadioButton_Standard_Twinkle10->GetValue())
        {
            _twinkleRatio = 10;
            return TWINKLE;
        }
        else if (RadioButton_Standard_Twinkle25->GetValue())
        {
            _twinkleRatio = 4;
            return TWINKLE;
        }
        else if (RadioButton_Standard_Twinkle50->GetValue())
        {
            _twinkleRatio = 2;
            return TWINKLE;
        }
        else if (RadioButton_Standard_Shimmer->GetValue())
        {
            return SHIMMER;
        }
        else if (RadioButton_Standard_Background->GetValue())
        {
            return DIM;
        }
        break;
    case 1:
        if (RadioButton_RGB_Off->GetValue())
        {
            return OFF;
        }
        else if (RadioButton_RGB_Chase->GetValue())
        {
            _chaseGrouping = std::numeric_limits<int>::max();
            return CHASE;
        }
        else if (RadioButton_RGB_Chase13->GetValue())
        {
            _chaseGrouping = 3;
            return CHASE;
        }
        else if (RadioButton_RGB_Chase14->GetValue())
        {
            _chaseGrouping = 4;
            return CHASE;
        }
        else if (RadioButton_RGB_Chase15->GetValue())
        {
            _chaseGrouping = 5;
            return CHASE;
        }
        else if (RadioButton_RGB_Alternate->GetValue())
        {
            _chaseGrouping = 2;
            return CHASE;
        }
        else if (RadioButton_RGB_Twinkle5->GetValue())
        {
            _twinkleRatio = 20;
            return TWINKLE;
        }
        else if (RadioButton_RGB_Twinkle10->GetValue())
        {
            _twinkleRatio = 10;
            return TWINKLE;
        }
        else if (RadioButton_RGB_Twinkle25->GetValue())
        {
            _twinkleRatio = 4;
            return TWINKLE;
        }
        else if (RadioButton_RGB_Twinkle50->GetValue())
        {
            _twinkleRatio = 2;
            return TWINKLE;
        }
        else if (RadioButton_RGB_Shimmer->GetValue())
        {
            return SHIMMER;
        }
        else if (RadioButton_RGB_Background->GetValue())
        {
            return DIM;
        }
        break;
    case 2:
        if (RadioButton_RGBCycle_Off->GetValue())
        {
            return OFF;
        }
        else if (RadioButton_RGBCycle_ABC->GetValue())
        {
            _chaseGrouping = 3;
            return CHASE;
        }
        else if (RadioButton_RGBCycle_ABCAll->GetValue())
        {
            _chaseGrouping = 4;
            return CHASE;
        }
        else if (RadioButton_RGBCycle_ABCAllNone->GetValue())
        {
            _chaseGrouping = 5;
            return CHASE;
        }
        else if (RadioButton_RGBCycle_MixedColors->GetValue())
        {
            return DIM;
        }
        else if (RadioButton_RGBCycle_RGBW->GetValue())
        {
            return RGBW;
        }
        break;
    }

    return OFF;
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

	int NotebookSelection = AuiNotebook1->GetSelection();
	if (NotebookSelection != LastNotebookSelection)
	{
		LastNotebookSelection = NotebookSelection;
		_checkChannelList = true;
		TestSeqIdx = 0;
		//TestButtonsOff();
	}

    TestFunctions testFunc = GetTestFunction(NotebookSelection);

	if (testFunc != LastFunc)
	{
		LastFunc = testFunc;
		rgbCycle = 0;
		_checkChannelList = true;
		NextSequenceStart = -1;
	}

	if (_checkChannelList)
	{
		// get list of checked channels
        _outputManager->AllOff();
		GetCheckedItems(chArray);
		if (RadioButton_RGB_Chase->GetValue() || RadioButton_Standard_Chase->GetValue())
		{
			_chaseGrouping = chArray.Count();
			if (_chaseGrouping == 0)
			{
				_chaseGrouping = std::numeric_limits<int>::max();
			}
		}
		if (RadioButton_RGBCycle_RGBW->GetValue())
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
		if (testFunc == OFF)
		{
			StatusBar1->SetLabelText(_("Testing off"));
		}
		else
		{
			StatusBar1->SetLabelText(wxString::Format(_("Testing %ld channels"), static_cast<long>(chArray.Count())));
		}
		_checkChannelList = false;
	}

	if (testFunc != OFF && chArray.Count() > 0) switch (NotebookSelection)
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

		switch (testFunc)
		{
		case DIM:
			if (ColorChange)
			{
				for (i = 0; i < chArray.Count(); i++)
				{
                    _outputManager->SetOneChannel(chArray[i]-1, BgIntensity);
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
					TestSeqIdx = static_cast<int>(rand01()*(double)_twinkleRatio);
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
					TwinkleState[i] = static_cast<int>(rand01()*(double)interval + 100.0) / (double)_seqData.FrameTime();
                    _outputManager->SetOneChannel(chArray[i]-1, FgIntensity);
				}
				else
				{
					// was on, now go to bg color for random period
					TwinkleState[i] = -static_cast<int>(rand01()*(double)interval + 100.0) / (double)_seqData.FrameTime() * ((double)_twinkleRatio - 1.0);
                    _outputManager->SetOneChannel(chArray[i]-1, BgIntensity);
				}
			}
			break;

		case SHIMMER:
			if (ColorChange || curtime >= NextSequenceStart)
			{
				ShimIntensity = (ShimIntensity == FgIntensity) ? BgIntensity : FgIntensity;
				for (i = 0; i < chArray.Count(); i++)
				{
                    _outputManager->SetOneChannel(chArray[i]-1, ShimIntensity);
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
                    _outputManager->SetOneChannel(chArray[i]-1, v);
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
		switch (testFunc)
		{
		case DIM:
			if (ColorChange)
			{
				for (i = 0; i < chArray.Count(); i++)
				{
                    _outputManager->SetOneChannel(chArray[i]-1, BgColor[i % 3]);
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
					TestSeqIdx = static_cast<int>(rand01()*(double)_twinkleRatio);
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
					TwinkleState[i] = static_cast<int>(rand01()*(double)interval + 100.0) / (double)_seqData.FrameTime();
					TestSeqIdx = i * 3;
                    _outputManager->SetOneChannel(chArray[TestSeqIdx]-1, FgColor[0]);
                    _outputManager->SetOneChannel(chArray[TestSeqIdx + 1]-1, FgColor[1]);
                    _outputManager->SetOneChannel(chArray[TestSeqIdx + 2]-1, FgColor[2]);
				}
				else
				{
					// was on, now go to bg color for random period
					TwinkleState[i] = -static_cast<int>(rand01()*(double)interval + 100.0) / (double)_seqData.FrameTime() * ((double)_twinkleRatio - 1.0);
					TestSeqIdx = i * 3;
                    _outputManager->SetOneChannel(chArray[TestSeqIdx]-1, BgColor[0]);
                    _outputManager->SetOneChannel(chArray[TestSeqIdx + 1]-1, BgColor[1]);
                    _outputManager->SetOneChannel(chArray[TestSeqIdx + 2]-1, BgColor[2]);
				}
			}
			break;
		case SHIMMER:
			if (ColorChange || curtime >= NextSequenceStart)
			{
				ShimColor = (ShimColor == FgColor) ? BgColor : FgColor;
				for (i = 0; i < chArray.Count(); i++)
				{
                    _outputManager->SetOneChannel(chArray[i]-1, ShimColor[i % 3]);
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
                    _outputManager->SetOneChannel(chArray[i]-1, v);
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
		if (testFunc == DIM)
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
                _outputManager->SetOneChannel(chArray[i]-1, BgColor[i % 3]);
			}
		}
		else if (testFunc == RGBW)
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
                    _outputManager->SetOneChannel(chArray[i]-1, 0);
				}
				switch (rgbCycle)
				{
				case 0: // red
					for (i = 0; i < chArrayR.Count(); i++)
					{
                        _outputManager->SetOneChannel(chArrayR[i]-1, 255);
					}
					break;
				case 1: // green
					for (i = 0; i < chArrayG.Count(); i++)
					{
                        _outputManager->SetOneChannel(chArrayG[i]-1, 255);
					}
					break;
				case 2: // blue
					for (i = 0; i < chArrayB.Count(); i++)
					{
                        _outputManager->SetOneChannel(chArrayB[i]-1, 255);
					}
					break;
				case 3: // white
					for (i = 0; i < chArrayW.Count(); i++)
					{
                        _outputManager->SetOneChannel(chArrayW[i]-1, 255);
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
                    _outputManager->SetOneChannel(chArray[i]-1, v);
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
        if (!_outputManager->StartOutput())
        {
            wxMessageBox("At least one output could not be started. See log file for details.", "Warning");
        }
        Timer1.Start(50, wxTIMER_CONTINUOUS);
    }
	else
	{
        Timer1.Stop();
        wxTimerEvent ev;
        OnTimer1Trigger(ev);
        _outputManager->StopOutput();
	}
}

std::string TestDialog::SerialiseSettings()
{
    int standardFunction = 0;
    if (RadioButton_Standard_Chase->GetValue())
    {
        standardFunction = 1;
    }
    else if (RadioButton_Standard_Chase13->GetValue())
    {
        standardFunction = 2;
    }
    else if (RadioButton_Standard_Chase14->GetValue())
    {
        standardFunction = 3;
    }
    else if (RadioButton_Standard_Chase15->GetValue())
    {
        standardFunction = 4;
    }
    else if (RadioButton_Standard_Alternate->GetValue())
    {
        standardFunction = 5;
    }
    else if (RadioButton_Standard_Twinkle5->GetValue())
    {
        standardFunction = 6;
    }
    else if (RadioButton_Standard_Twinkle10->GetValue())
    {
        standardFunction = 7;
    }
    else if (RadioButton_Standard_Twinkle25->GetValue())
    {
        standardFunction = 8;
    }
    else if (RadioButton_Standard_Twinkle50->GetValue())
    {
        standardFunction = 9;
    }
    else if (RadioButton_Standard_Shimmer->GetValue())
    {
        standardFunction = 10;
    }
    else if (RadioButton_Standard_Background->GetValue())
    {
        standardFunction = 11;
    }
    
    int rgbFunction = 0;
    if (RadioButton_RGB_Chase->GetValue())
    {
        rgbFunction = 1;
    }
    else if (RadioButton_RGB_Chase13->GetValue())
    {
        rgbFunction = 2;
    }
    else if (RadioButton_RGB_Chase14->GetValue())
    {
        rgbFunction = 3;
    }
    else if (RadioButton_RGB_Chase15->GetValue())
    {
        rgbFunction = 4;
    }
    else if (RadioButton_RGB_Alternate->GetValue())
    {
        rgbFunction = 5;
    }
    else if (RadioButton_RGB_Twinkle5->GetValue())
    {
        rgbFunction = 6;
    }
    else if (RadioButton_RGB_Twinkle10->GetValue())
    {
        rgbFunction = 7;
    }
    else if (RadioButton_RGB_Twinkle25->GetValue())
    {
        rgbFunction = 8;
    }
    else if (RadioButton_RGB_Twinkle50->GetValue())
    {
        rgbFunction = 9;
    }
    else if (RadioButton_RGB_Shimmer->GetValue())
    {
        rgbFunction = 10;
    }
    else if (RadioButton_RGB_Background->GetValue())
    {
        rgbFunction = 11;
    }

    int rgbCycleFunction = 0;
    if (RadioButton_RGBCycle_ABC->GetValue())
    {
        rgbCycleFunction = 1;
    }
    else if (RadioButton_RGBCycle_ABCAll->GetValue())
    {
        rgbCycleFunction = 2;
    }
    else if (RadioButton_RGBCycle_ABCAllNone->GetValue())
    {
        rgbCycleFunction = 3;
    }
    else if (RadioButton_RGBCycle_MixedColors->GetValue())
    {
        rgbCycleFunction = 4;
    }
    else if (RadioButton_RGBCycle_RGBW->GetValue())
    {
        rgbCycleFunction = 5;
    }

    int speed = Slider_Speed->GetValue();

    int standardBackground = Slider_Standard_Background->GetValue();
    int standardHighlight = Slider_Standard_Highlight->GetValue();

    int rgbBackgroundR = Slider_RGB_BG_R->GetValue();
    int rgbBackgroundG = Slider_RGB_BG_G->GetValue();
    int rgbBackgroundB = Slider_RGB_BG_B->GetValue();
    int rgbHighlightR = Slider_RGB_H_R->GetValue();
    int rgbHighlightG = Slider_RGB_H_G->GetValue();
    int rgbHighlightB = Slider_RGB_H_B->GetValue();

    return wxString::Format("%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d",
        speed,
        standardFunction, standardBackground, standardHighlight,
        rgbFunction, rgbBackgroundR, rgbBackgroundG, rgbBackgroundB, rgbHighlightR, rgbHighlightG, rgbHighlightB,
        rgbCycleFunction).ToStdString();
}

void TestDialog::DeserialiseSettings(const std::string& settings)
{
    if (settings == "") return;

    wxArrayString values = wxSplit(settings, '|');

    TestButtonsOff();
    RadioButton_Standard_Off->SetValue(false);
    RadioButton_RGB_Off->SetValue(false);
    RadioButton_RGBCycle_Off->SetValue(false);

    if (values.size() == 12)
    {
        Slider_Speed->SetValue(wxAtoi(values[0]));
        switch (wxAtoi(values[1]))
        {
        case 1:
            RadioButton_Standard_Chase->SetValue(true);
            break;
        case 2:
            RadioButton_Standard_Chase13->SetValue(true);
            break;
        case 3:
            RadioButton_Standard_Chase14->SetValue(true);
            break;
        case 4:
            RadioButton_Standard_Chase15->SetValue(true);
            break;
        case 5:
            RadioButton_Standard_Alternate->SetValue(true);
            break;
        case 6:
            RadioButton_Standard_Twinkle5->SetValue(true);
            break;
        case 7:
            RadioButton_Standard_Twinkle10->SetValue(true);
            break;
        case 8:
            RadioButton_Standard_Twinkle25->SetValue(true);
            break;
        case 9:
            RadioButton_Standard_Twinkle50->SetValue(true);
            break;
        case 10:
            RadioButton_Standard_Shimmer->SetValue(true);
            break;
        case 11:
            RadioButton_Standard_Background->SetValue(true);
            break;
        default:
            RadioButton_Standard_Off->SetValue(true);
            break;
        }
        Slider_Standard_Background->SetValue(wxAtoi(values[2]));
        Slider_Standard_Highlight->SetValue(wxAtoi(values[3]));

        switch (wxAtoi(values[4]))
        {
        case 1:
            RadioButton_RGB_Chase->SetValue(true);
            break;
        case 2:
            RadioButton_RGB_Chase13->SetValue(true);
            break;
        case 3:
            RadioButton_RGB_Chase14->SetValue(true);
            break;
        case 4:
            RadioButton_RGB_Chase15->SetValue(true);
            break;
        case 5:
            RadioButton_RGB_Alternate->SetValue(true);
            break;
        case 6:
            RadioButton_RGB_Twinkle5->SetValue(true);
            break;
        case 7:
            RadioButton_RGB_Twinkle10->SetValue(true);
            break;
        case 8:
            RadioButton_RGB_Twinkle25->SetValue(true);
            break;
        case 9:
            RadioButton_RGB_Twinkle50->SetValue(true);
            break;
        case 10:
            RadioButton_RGB_Shimmer->SetValue(true);
            break;
        case 11:
            RadioButton_RGB_Background->SetValue(true);
            break;
        default:
            RadioButton_RGB_Off->SetValue(true);
            break;
        }
        Slider_RGB_BG_R->SetValue(wxAtoi(values[5]));
        Slider_RGB_BG_G->SetValue(wxAtoi(values[6]));
        Slider_RGB_BG_B->SetValue(wxAtoi(values[7]));
        Slider_RGB_H_R->SetValue(wxAtoi(values[8]));
        Slider_RGB_H_G->SetValue(wxAtoi(values[9]));
        Slider_RGB_H_B->SetValue(wxAtoi(values[10]));

        switch(wxAtoi(values[11]))
        {
        case 1:
            RadioButton_RGBCycle_ABC->SetValue(true);
            break;
        case 2:
            RadioButton_RGBCycle_ABCAll->SetValue(true);
            break;
        case 3:
            RadioButton_RGBCycle_ABCAllNone->SetValue(true);
            break;
        case 4:
            RadioButton_RGBCycle_MixedColors->SetValue(true);
            break;
        case 5:
            RadioButton_RGBCycle_RGBW->SetValue(true);
            break;
        default:
            RadioButton_RGBCycle_Off->SetValue(true);
            break;
        }
    }
}

void TestDialog::OnClose(wxCloseEvent& event)
{
    if (CheckBox_OutputToLights->IsChecked())
    {
        Timer1.Stop();
        _outputManager->AllOff();
        _outputManager->StopOutput();
        EnableSleepModes();
    }

    wxConfigBase* config = wxConfigBase::Get();
    config->Write("xLightsTestSettings", wxString(SerialiseSettings()));

    EndDialog(0);
}
