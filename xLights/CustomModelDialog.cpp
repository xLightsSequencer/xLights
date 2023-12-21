/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "CustomModelDialog.h"

#include <wx/artprov.h>
#include <wx/msgdlg.h>
#include <wx/clipbrd.h>
#include <wx/graphics.h>
#include <wx/file.h>
#include <wx/numdlg.h>
#include <wx/config.h>
#include <wx/config.h>
#include <wx/choicdlg.h>

//(*InternalHeaders(CustomModelDialog)
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/filepicker.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

#include "models/CustomModel.h"
#include "WiringDialog.h"
#include "wxModelGridCellRenderer.h"
#include "UtilClasses.h"
#include "UtilFunctions.h"
#include "ExternalHooks.h"
#include "ModelPreview.h"
#include "outputs/TwinklyOutput.h"
#include "Discovery.h"
#include "outputs/OutputManager.h"

//(*IdInit(CustomModelDialog)
const long CustomModelDialog::ID_SPINCTRL1 = wxNewId();
const long CustomModelDialog::ID_SPINCTRL2 = wxNewId();
const long CustomModelDialog::ID_STATICTEXT1 = wxNewId();
const long CustomModelDialog::ID_SPINCTRL3 = wxNewId();
const long CustomModelDialog::ID_CHECKBOX1 = wxNewId();
const long CustomModelDialog::ID_BUTTON3 = wxNewId();
const long CustomModelDialog::ID_CHECKBOX_SHOW_DUPS = wxNewId();
const long CustomModelDialog::ID_CHECKBOX2 = wxNewId();
const long CustomModelDialog::ID_BITMAPBUTTON_CUSTOM_CUT = wxNewId();
const long CustomModelDialog::ID_BITMAPBUTTON_CUSTOM_COPY = wxNewId();
const long CustomModelDialog::ID_BITMAPBUTTON_CUSTOM_PASTE = wxNewId();
const long CustomModelDialog::ID_BUTTON_CustomModelZoomIn = wxNewId();
const long CustomModelDialog::ID_BUTTON_CustomModelZoomOut = wxNewId();
const long CustomModelDialog::ID_FILEPICKERCTRL1 = wxNewId();
const long CustomModelDialog::ID_SLIDER_CUSTOM_LIGHTNESS = wxNewId();
const long CustomModelDialog::ID_BITMAPBUTTON_CUSTOM_BKGRD = wxNewId();
const long CustomModelDialog::ID_CHECKBOX_AUTO_NUMBER = wxNewId();
const long CustomModelDialog::ID_CHECKBOX_AUTO_INCREMENT = wxNewId();
const long CustomModelDialog::ID_SPINCTRL_NEXT_CHANNEL = wxNewId();
const long CustomModelDialog::ID_BUTTON4 = wxNewId();
const long CustomModelDialog::ID_BUTTON1 = wxNewId();
const long CustomModelDialog::ID_BUTTON2 = wxNewId();
const long CustomModelDialog::ID_NOTEBOOK1 = wxNewId();
const long CustomModelDialog::ID_PANEL2 = wxNewId();
const long CustomModelDialog::ID_PANEL1 = wxNewId();
const long CustomModelDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

const long CustomModelDialog::CUSTOMMODELDLGMNU_CUT = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_COPY = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_PASTE = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_DELETE = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_FLIPH = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_FLIPHSELECTED = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_FLIPV = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_FLIPVSELECTED = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_ROTATE90 = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_ROTATE = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_REVERSE = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_SHIFT = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_SHIFTSELECTED = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_INSERT = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_COMPRESS = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_FIND = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_FINDLAST = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_MAKESINGLENODE = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_TRIMUNUSEDSPACE = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_SHRINKSPACE10 = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_SHRINKSPACE50 = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_SHRINKSPACE99 = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_EXPANDSPACE = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_COPYLAYERFWD1 = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_COPYLAYERBKWD1 = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_COPYLAYERFWDALL = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_COPYLAYERBKWDALL = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_CREATESUBMODELFROMLAYER = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMLAYER = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_CREATESUBMODELFROMROW = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMROW = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_CREATESUBMODELFROMCOLUMN = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMCOLUMN = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_CREATESUBMODELFROMALLLAYERS = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMALLLAYERS = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_CREATESUBMODELFROMALLROWS = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMALLROWS = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_CREATESUBMODELFROMALLCOLUMNS = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMALLCOLUMNS = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_WIREHORIZONTALLEFT = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_WIREHORIZONTALRIGHT = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_WIREVERTICALTOP = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_WIREVERTICALBOTTOM = wxNewId();
const long CustomModelDialog::ID_TIMER1 = wxNewId();

wxDEFINE_EVENT(EVT_GRID_KEY, wxCommandEvent);
wxDEFINE_EVENT(EVT_SWITCH_GRID, wxCommandEvent);

BEGIN_EVENT_TABLE(CustomModelDialog,wxDialog)
	//(*EventTable(CustomModelDialog)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_GRID_KEY, CustomModelDialog::OnGridKey)
    EVT_COMMAND(wxID_ANY, EVT_SWITCH_GRID, CustomModelDialog::OnSwitchGrid)
END_EVENT_TABLE()

class CustomNotebook : public wxNotebook
{
public:
    void DoNavigation(wxNavigationKeyEvent event)
    {
        if (event.GetDirection())
        {
            wxCommandEvent keyEvent(EVT_GRID_KEY);
            keyEvent.SetInt(WXK_PAGEDOWN);
            wxPostEvent(this, keyEvent);
            event.StopPropagation();
        }
        else
        {
            wxCommandEvent keyEvent(EVT_GRID_KEY);
            keyEvent.SetInt(WXK_PAGEUP);
            wxPostEvent(this, keyEvent);
            event.StopPropagation();
        }
    }

    CustomNotebook(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) : wxNotebook(parent, id, pos, size, style, name)
    {
        Connect(wxEVT_NAVIGATION_KEY, (wxObjectEventFunction)&CustomNotebook::DoNavigation, 0, this);
    }

    virtual ~CustomNotebook() {}
};

// Subclassing wxGrid is the only way to get keyboard copy and paste working without breaking the grid behaviour
class CopyPasteGrid : public wxGrid
{
    void HandleOnMouseWheel(wxMouseEvent& event)
    {
        m_wheelRotation += event.GetWheelRotation();
        int lines = m_wheelRotation / event.GetWheelDelta();
        m_wheelRotation -= lines * event.GetWheelDelta();

        if (lines != 0)
        {

            wxScrollWinEvent newEvent;

            newEvent.SetPosition(0);
            newEvent.SetOrientation(event.GetWheelAxis() == 0 ? wxVERTICAL : wxHORIZONTAL);

            if(event.ShiftDown())
            {
                if (newEvent.GetOrientation() == wxVERTICAL)
                {
                    newEvent.SetOrientation(wxHORIZONTAL);
                }
                else
                {
                    newEvent.SetOrientation(wxVERTICAL);
                }
            }

            if (event.ControlDown()|| event.CmdDown())
            {
                wxCommandEvent keyEvent(EVT_SWITCH_GRID);
                keyEvent.SetInt(lines);
                wxPostEvent(this, keyEvent);
                event.StopPropagation();
                return;
            }

            newEvent.SetEventObject(m_win);

            if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL)
                lines = -lines;

            if (event.IsPageScroll())
            {
                if (lines > 0)
                    newEvent.SetEventType(wxEVT_SCROLLWIN_PAGEUP);
                else
                    newEvent.SetEventType(wxEVT_SCROLLWIN_PAGEDOWN);

                m_win->GetEventHandler()->ProcessEvent(newEvent);
            }
            else
            {
                lines *= event.GetLinesPerAction();
                if (lines > 0)
                    newEvent.SetEventType(wxEVT_SCROLLWIN_LINEUP);
                else
                    newEvent.SetEventType(wxEVT_SCROLLWIN_LINEDOWN);

                int times = abs(lines);
                for (; times > 0; times--)
                    m_win->GetEventHandler()->ProcessEvent(newEvent);
            }
        }
    }

    void DoOnChar(wxKeyEvent& event)
    {
        wxChar uc = event.GetKeyCode();

        switch (uc)
        {
        case 'c':
        case 'C':
        case WXK_CONTROL_C:
            if (event.CmdDown() || event.ControlDown()) {
                wxCommandEvent pasteEvent(wxEVT_TEXT_COPY);
                wxPostEvent(this, pasteEvent);
                event.StopPropagation();
            }
            break;
        case 'x':
        case 'X':
        case WXK_CONTROL_X:
            if (event.CmdDown() || event.ControlDown()) {
                wxCommandEvent pasteEvent(wxEVT_TEXT_CUT);
                wxPostEvent(this, pasteEvent);
                event.StopPropagation();
            }
            break;
        case 'v':
        case 'V':
        case WXK_CONTROL_V:
            if (event.CmdDown() || event.ControlDown()) {
                wxCommandEvent pasteEvent(wxEVT_TEXT_PASTE);
                wxPostEvent(this, pasteEvent);
                event.StopPropagation();
            }
            break;
        case 'a':
        case 'A':
        case WXK_CONTROL_A:
            if (event.CmdDown() || event.ControlDown()) {
                wxCommandEvent keyEvent(EVT_GRID_KEY);
                keyEvent.SetInt(WXK_CONTROL_A);
                wxPostEvent(this, keyEvent);
                event.StopPropagation();
            }
            break;
        case WXK_F4: // move fwd & stay
        case WXK_F5: // move back & stay
        case WXK_F7: // move fwd & go
        case WXK_F8: // move back & go
            if (!event.ShiftDown() && !event.CmdDown() && !event.ControlDown() && !event.AltDown()) {
                wxCommandEvent keyEvent(EVT_GRID_KEY);
                keyEvent.SetInt(event.GetKeyCode());
                wxPostEvent(this, keyEvent);
                event.StopPropagation();
            }
            break;
        case WXK_PAGEUP:
        case WXK_PAGEDOWN:
        case WXK_HOME:
        case WXK_END:
            if (event.ShiftDown() && (event.CmdDown() || event.ControlDown())) {
                wxCommandEvent keyEvent(EVT_GRID_KEY);
                keyEvent.SetInt(event.GetUnicodeKey());
                wxPostEvent(this, keyEvent);
                event.StopPropagation();
            }
            break;
        default:
            wxGrid::OnChar(event);
            break;
        }
    }

    public:
    CopyPasteGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) : wxGrid(parent, id, pos, size, style, name)
    {
        Connect(wxEVT_CHAR, (wxObjectEventFunction)&CopyPasteGrid::DoOnChar, 0, this);
        Connect(wxEVT_MOUSEWHEEL, (wxObjectEventFunction)&CopyPasteGrid::HandleOnMouseWheel, 0, this);
    }

    virtual ~CopyPasteGrid()
    {

    }
};

CustomModelDialog::CustomModelDialog(wxWindow* parent, OutputManager* om) : background_image(""),
  bkg_image(nullptr),
  bkgrd_active(true),
  lightness(80),
  autonumber(false),
  autoincrement(false),
  next_channel(1),
    _outputManager(om)
{
	//(*Initialize(CustomModelDialog)
	wxFlexGridSizer* FlexGridSizer11;
	wxFlexGridSizer* FlexGridSizer12;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer9;
	wxFlexGridSizer* Sizer2;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxStaticBoxSizer* StaticBoxSizer2;
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;
	wxStaticText* StaticText3;

	Create(parent, wxID_ANY, _("Custom Model"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	SetClientSize(wxDLG_UNIT(parent,wxSize(450,350)));
	SetMinSize(wxDLG_UNIT(parent,wxSize(300,200)));
	Sizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	Sizer1->AddGrowableCol(1);
	Sizer1->AddGrowableRow(0);
	Sizer2 = new wxFlexGridSizer(7, 1, 0, 0);
	Sizer2->AddGrowableCol(0);
	Sizer2->AddGrowableRow(6);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Width"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	WidthSpin = new wxSpinCtrl(this, ID_SPINCTRL1, _T("10"), wxDefaultPosition, wxDefaultSize, 0, 1, 1000, 10, _T("ID_SPINCTRL1"));
	WidthSpin->SetValue(_T("10"));
	FlexGridSizer2->Add(WidthSpin, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Height"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	HeightSpin = new wxSpinCtrl(this, ID_SPINCTRL2, _T("10"), wxDefaultPosition, wxDefaultSize, 0, 1, 1000, 10, _T("ID_SPINCTRL2"));
	HeightSpin->SetValue(_T("10"));
	FlexGridSizer2->Add(HeightSpin, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT1, _("Depth"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Depth = new wxSpinCtrl(this, ID_SPINCTRL3, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL3"));
	SpinCtrl_Depth->SetValue(_T("1"));
	FlexGridSizer2->Add(SpinCtrl_Depth, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Sizer2->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
	CheckBox_ShowWiring = new wxCheckBox(this, ID_CHECKBOX1, _("Show wiring in model preview"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_ShowWiring->SetValue(false);
	FlexGridSizer8->Add(CheckBox_ShowWiring, 1, wxALL|wxEXPAND, 5);
	ButtonWiring = new wxButton(this, ID_BUTTON3, _("Wiring View"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer8->Add(ButtonWiring, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Show_Duplicates = new wxCheckBox(this, ID_CHECKBOX_SHOW_DUPS, _("Show Duplicate Nodes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_SHOW_DUPS"));
	CheckBox_Show_Duplicates->SetValue(false);
	FlexGridSizer8->Add(CheckBox_Show_Duplicates, 1, wxALL|wxEXPAND, 5);
	CheckBox_OutputToLights = new wxCheckBox(this, ID_CHECKBOX2, _("Output to Lights"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_OutputToLights->SetValue(false);
	FlexGridSizer8->Add(CheckBox_OutputToLights, 1, wxALL|wxEXPAND, 5);
	Sizer2->Add(FlexGridSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 7, 0, 0);
	BitmapButtonCustomCut = new wxBitmapButton(this, ID_BITMAPBUTTON_CUSTOM_CUT, wxArtProvider::GetBitmapBundle("wxART_CUT", wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_CUSTOM_CUT"));
	BitmapButtonCustomCut->SetToolTip(_("Cut"));
	FlexGridSizer5->Add(BitmapButtonCustomCut, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButtonCustomCopy = new wxBitmapButton(this, ID_BITMAPBUTTON_CUSTOM_COPY, wxArtProvider::GetBitmapBundle("wxART_COPY", wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_CUSTOM_COPY"));
	BitmapButtonCustomCopy->SetDefault();
	BitmapButtonCustomCopy->SetToolTip(_("Copy"));
	FlexGridSizer5->Add(BitmapButtonCustomCopy, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButtonCustomPaste = new wxBitmapButton(this, ID_BITMAPBUTTON_CUSTOM_PASTE, wxArtProvider::GetBitmapBundle("wxART_PASTE", wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_CUSTOM_PASTE"));
	BitmapButtonCustomPaste->SetDefault();
	BitmapButtonCustomPaste->SetToolTip(_("Paste"));
	FlexGridSizer5->Add(BitmapButtonCustomPaste, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_CustomModelZoomIn = new wxButton(this, ID_BUTTON_CustomModelZoomIn, _("+"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CustomModelZoomIn"));
	Button_CustomModelZoomIn->SetMinSize(wxSize(24,-1));
	Button_CustomModelZoomIn->SetToolTip(_("Zoom In"));
	FlexGridSizer5->Add(Button_CustomModelZoomIn, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_CustomModelZoomOut = new wxButton(this, ID_BUTTON_CustomModelZoomOut, _("-"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CustomModelZoomOut"));
	Button_CustomModelZoomOut->SetMinSize(wxSize(24,-1));
	Button_CustomModelZoomOut->SetToolTip(_("Zoom Out"));
	FlexGridSizer5->Add(Button_CustomModelZoomOut, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Sizer2->Add(FlexGridSizer5, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Background Image"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FilePickerCtrl1 = new ImageFilePickerCtrl(this, ID_FILEPICKERCTRL1, wxEmptyString, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
	FlexGridSizer1->Add(FilePickerCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SliderCustomLightness = new wxSlider(this, ID_SLIDER_CUSTOM_LIGHTNESS, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_CUSTOM_LIGHTNESS"));
	FlexGridSizer1->Add(SliderCustomLightness, 1, wxTOP|wxBOTTOM|wxLEFT|wxEXPAND, 5);
	BitmapButtonCustomBkgrd = new wxBitmapButton(this, ID_BITMAPBUTTON_CUSTOM_BKGRD, wxArtProvider::GetBitmapBundle("wxART_FIND", wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_CUSTOM_BKGRD"));
	BitmapButtonCustomBkgrd->SetDefault();
	BitmapButtonCustomBkgrd->SetMinSize(wxSize(24,-1));
	FlexGridSizer1->Add(BitmapButtonCustomBkgrd, 1, wxTOP|wxBOTTOM|wxRIGHT, 5);
	StaticBoxSizer2->Add(FlexGridSizer1, 1, wxEXPAND, 5);
	Sizer2->Add(StaticBoxSizer2, 1, wxEXPAND, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxVERTICAL, this, _("Auto Numbering"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
	CheckBoxAutoNumber = new wxCheckBox(this, ID_CHECKBOX_AUTO_NUMBER, _("Active"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_AUTO_NUMBER"));
	CheckBoxAutoNumber->SetValue(false);
	FlexGridSizer6->Add(CheckBoxAutoNumber, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBoxAutoIncrement = new wxCheckBox(this, ID_CHECKBOX_AUTO_INCREMENT, _("Auto Increment"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_AUTO_INCREMENT"));
	CheckBoxAutoIncrement->SetValue(false);
	FlexGridSizer6->Add(CheckBoxAutoIncrement, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer6, 1, wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText3 = new wxStaticText(this, wxID_ANY, _("Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer4->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlNextChannel = new wxSpinCtrl(this, ID_SPINCTRL_NEXT_CHANNEL, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 9999999, 1, _T("ID_SPINCTRL_NEXT_CHANNEL"));
	SpinCtrlNextChannel->SetValue(_T("1"));
	FlexGridSizer4->Add(SpinCtrlNextChannel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer4, 1, wxEXPAND, 5);
	StaticBoxSizer1->Add(FlexGridSizer3, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Sizer2->Add(StaticBoxSizer1, 1, wxEXPAND, 5);
	FlexGridSizer12 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_ImportFromController = new wxButton(this, ID_BUTTON4, _("Import From Controller"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer12->Add(Button_ImportFromController, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Sizer2->Add(FlexGridSizer12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7 = new wxFlexGridSizer(0, 3, 0, 0);
	ButtonOk = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer7->Add(ButtonOk, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonCancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer7->Add(ButtonCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Sizer2->Add(FlexGridSizer7, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
	Sizer1->Add(Sizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	FlexGridSizer9->AddGrowableRow(0);
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetSashGravity(0.5);
	Panel11 = new wxPanel(SplitterWindow1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer11 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer11->AddGrowableCol(0);
	FlexGridSizer11->AddGrowableRow(0);
	Notebook1 = new CustomNotebook(Panel11, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	FlexGridSizer11->Add(Notebook1, 1, wxALL|wxEXPAND, 5);
	Panel11->SetSizer(FlexGridSizer11);
	FlexGridSizer11->Fit(Panel11);
	FlexGridSizer11->SetSizeHints(Panel11);
	Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer10->AddGrowableCol(0);
	FlexGridSizer10->AddGrowableRow(0);
	Panel1->SetSizer(FlexGridSizer10);
	FlexGridSizer10->Fit(Panel1);
	FlexGridSizer10->SetSizeHints(Panel1);
	SplitterWindow1->SplitHorizontally(Panel11, Panel1);
	SplitterWindow1->SetSashPosition(30);
	FlexGridSizer9->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 2);
	Sizer1->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 5);
	SetSizer(Sizer1);
	SetSizer(Sizer1);
	Layout();
	Center();

	Connect(ID_SPINCTRL1,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&CustomModelDialog::OnWidthSpinChange);
	Connect(ID_SPINCTRL2,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&CustomModelDialog::OnHeightSpinChange);
	Connect(ID_SPINCTRL3,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&CustomModelDialog::OnSpinCtrl_DepthChange);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnCheckBox_ShowWiringClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnButtonWiringClick);
	Connect(ID_CHECKBOX_SHOW_DUPS,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnCheckBox_Show_DuplicatesClick);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnCheckBox_OutputToLightsClick);
	Connect(ID_BITMAPBUTTON_CUSTOM_CUT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnBitmapButtonCustomCutClick);
	Connect(ID_BITMAPBUTTON_CUSTOM_COPY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnBitmapButtonCustomCopyClick);
	Connect(ID_BITMAPBUTTON_CUSTOM_PASTE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnBitmapButtonCustomPasteClick);
	Connect(ID_BUTTON_CustomModelZoomIn,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnButton_CustomModelZoomInClick);
	Connect(ID_BUTTON_CustomModelZoomOut,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnButton_CustomModelZoomOutClick);
	Connect(ID_FILEPICKERCTRL1,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&CustomModelDialog::OnFilePickerCtrl1FileChanged);
	Connect(ID_SLIDER_CUSTOM_LIGHTNESS,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&CustomModelDialog::OnSliderCustomLightnessCmdSliderUpdated);
	Connect(ID_BITMAPBUTTON_CUSTOM_BKGRD,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnBitmapButtonCustomBkgrdClick);
	Connect(ID_CHECKBOX_AUTO_NUMBER,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnCheckBoxAutoNumberClick);
	Connect(ID_CHECKBOX_AUTO_INCREMENT,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnCheckBoxAutoIncrementClick);
	Connect(ID_SPINCTRL_NEXT_CHANNEL,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&CustomModelDialog::OnSpinCtrlNextChannelChange);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnButton_ImportFromControllerClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnButtonOkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnButtonCancelClick);
	Connect(ID_NOTEBOOK1,wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&CustomModelDialog::OnNotebook1PageChanged);
	//*)

    name = "";

    SetEscapeId(ButtonCancel->GetId());

    wxConfigBase* config = wxConfigBase::Get();
    int sashPos = config->Read("xLightsCustomDialogSash", 30);
    if (sashPos < 5) sashPos = 5;
    if (sashPos > 95) sashPos = 95;

    SplitterWindow1->SetSashPosition((SplitterWindow1->GetSize().GetHeight() * (100 - sashPos)) / 100);
    SplitterWindow1->SetMinimumPaneSize(5);

    _modelPreview = new ModelPreview(Panel1, nullptr, true);
    _modelPreview->Set3D(true);
    FlexGridSizer10->Add(_modelPreview, 1, wxALL | wxEXPAND, 0);
    FlexGridSizer10->Fit(Panel1);
    FlexGridSizer10->SetSizeHints(Panel1);

    Layout();

    ValidateWindow();

    _oldOutputToLights = _outputManager->IsOutputting();
    if (_oldOutputToLights) {
        _outputManager->StopOutput();
    }
}

CustomModelDialog::~CustomModelDialog()
{
	//(*Destroy(CustomModelDialog)
	//*)

    wxConfigBase* config = wxConfigBase::Get();
    int sashPos = ((SplitterWindow1->GetSize().GetHeight() - SplitterWindow1->GetSashPosition()) * 100) / SplitterWindow1->GetSize().GetHeight();
    config->Write("xLightsCustomDialogSash", sashPos);

    if (_modelPreview != nullptr)
    {
        delete _modelPreview;
    }

	if( bkg_image != nullptr ) {
        delete bkg_image;
    }

    StopOutputToLights();
    if (_oldOutputToLights) {
        _outputManager->StartOutput();
    }
}

void CustomModelDialog::ValidateWindow()
{
    if (SpinCtrl_Depth->GetValue() < 1 ||
        WidthSpin->GetValue() < 1 ||
        HeightSpin->GetValue() < 1) {
        ButtonOk->Enable(false);
    }
    else {
        ButtonOk->Enable();
    }
}

void CustomModelDialog::UpdatePreview(int width, int height, int depth, const std::string& modelData)
{
    _model->GetBaseObjectScreenLocation().SetMDepth(depth);
    _model->UpdateModel(width, height, depth, modelData);
    _model->GetModelScreenLocation().SetWorldPosition(glm::vec3(0, 0, 0)); // centre the model
    int maxxy = std::max(width, height);
    float scale = 1500.0 / maxxy;
    _model->GetModelScreenLocation().SetScaleMatrix(glm::vec3(scale, scale, scale));
    if (_model != nullptr && _modelPreview != nullptr) {
        _modelPreview->RenderModel(_model, CheckBox_ShowWiring->IsChecked(), true, _highlightpixel);
    }
}

void CustomModelDialog::UpdatePreview()
{
    UpdatePreview(WidthSpin->GetValue(), HeightSpin->GetValue(), SpinCtrl_Depth->GetValue(), GetModelData());
}

void CustomModelDialog::Setup(CustomModel* m)
{
    _model = m;
    _modelPreview->SetModel(m, CheckBox_ShowWiring->IsChecked(), true);
    name = m->GetName();
    background_image = m->GetCustomBackground();
    FilePickerCtrl1->SetFileName(wxFileName(background_image));
    lightness = m->GetCustomLightness();
    SliderCustomLightness->SetValue(lightness);
    std::string data = m->GetCustomData();

    if (background_image != "" && FileExists(background_image)) {
        bkg_image = new wxImage(background_image);
    }

    SpinCtrl_Depth->SetValue(m->GetCustomDepth());
    WidthSpin->SetValue(m->GetCustomWidth());
    HeightSpin->SetValue(m->GetCustomHeight());

    if (data == "") {
        for (int i = 0; i < m->GetCustomDepth(); i++) {
            AddPage();
            auto grid = GetLayerGrid(i);

            wxFont font = grid->GetDefaultCellFont();
            SetGridSizeForFont(font);
        }
    }
    else {
        wxArrayString layers = wxSplit(data, '|');
        for (auto layer = 0; layer < layers.size(); layer++) {
            AddPage();
            //ResizeCustomGrid();
            auto grid = GetLayerGrid(layer);
            wxArrayString rows = wxSplit(layers[layer], ';');
            //grid->AppendRows(rows.size() - 1);

            for (auto row = 0; row < rows.size(); row++) {
                wxArrayString cols = wxSplit(rows[row], ',');
                //if (row == 0) {
                //    grid->AppendCols(cols.size() - 1);
                //}
                for (auto col = 0; col < cols.size(); col++) {
                    wxString value = cols[col];
                    if (!value.IsEmpty() && value != "0") {
                        grid->SetCellValue(row, col, value);
                    }
                }
            }

            wxFont font = grid->GetDefaultCellFont();
            SetGridSizeForFont(font);
        }
    }

    UpdateBackground();

    _saveScale = m->GetModelScreenLocation().GetScaleMatrix();
    _saveWorldPos = m->GetModelScreenLocation().GetWorldPosition();
    _saveCentreX = ((BoxedScreenLocation&)m->GetModelScreenLocation()).GetCentreX();
    _saveCentreY = ((BoxedScreenLocation&)m->GetModelScreenLocation()).GetCentreY();
    _saveCentreZ = ((BoxedScreenLocation&)m->GetModelScreenLocation()).GetCentreZ();
    _saveWidth = WidthSpin->GetValue();
    _saveHeight = HeightSpin->GetValue();
    _saveDepth = SpinCtrl_Depth->GetValue();
    _saveModelData = data;

    // UpdatePreview();  // this is called at the end so not sure why it is here as well, maybe wrong but appears to be redundant
    wxBookCtrlEvent e;
    e.SetSelection(0);
    OnNotebook1PageChanged(e);

    Sizer1->Layout();
    Layout();

    // This does not stop the user entering the illegal characters
    //        wxGridCellTextEditor *reditor = new wxGridCellTextEditor();
    //        wxString filter("0123456789");
    //        wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST);
    //        validator.SetCharIncludes(filter);
    //        reditor->SetValidator(validator);
    //        GridCustom->SetDefaultEditor(reditor);

    // neither does this
    //for (int r = 0; r < GridCustom->GetNumberRows(); ++r)
    //{
    //    for (int c = 0; c < GridCustom->GetNumberCols(); ++c)
    //    {
    //        wxGridCellTextEditor *reditor = new wxGridCellTextEditor();
    //        wxString filter("0123456789");
    //        wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST);
    //        validator.SetCharIncludes(filter);
    //        reditor->SetValidator(validator);
    //        //GridCustom->SetDefaultEditor(reditor);
    //        GridCustom->SetCellEditor(r, c, reditor);
    //    }
    //}
    UpdatePreview();
    ValidateWindow();
}

// make grid the size specified by the spin controls
void CustomModelDialog::ResizeCustomGrid()
{
    CheckBox_Show_Duplicates->SetValue(false);
    ClearDupNodes();

    int numCols = WidthSpin->GetValue();
    int numRows = HeightSpin->GetValue();
    int numLayers = SpinCtrl_Depth->GetValue();

    // remove/add layers first
    while (Notebook1->GetPageCount() < numLayers) {
        AddPage();
    }

    while (Notebook1->GetPageCount() > numLayers) {
        RemovePage();
    }

    for (auto grid : _grids) {
        int deltaCols = numCols - grid->GetNumberCols();
        int deltaRows = numRows - grid->GetNumberRows();
        if (deltaCols > 0) grid->AppendCols(deltaCols);
        if (deltaRows > 0) grid->AppendRows(deltaRows);
        if (deltaCols < 0) grid->DeleteCols(numCols, -deltaCols);
        if (deltaRows < 0) grid->DeleteRows(numRows, -deltaRows);
    }
    UpdateBackground();
}

wxString StripIllegalChars(const wxString& s)
{
    wxString res = "";

    for (auto it = s.begin(); it != s.end(); ++it)
    {
        if (*it >= '0' && *it <= '9')
        {
            res += *it;
        }
    }

    return res;
}

std::string CustomModelDialog::GetModelData()
{
    std::string customChannelData = "";
    for (int layer = 0; layer < Notebook1->GetPageCount(); layer++) {
        if (layer > 0) customChannelData += "|";
        auto grid = GetLayerGrid(layer);
        int numCols = grid->GetNumberCols();
        int numRows = grid->GetNumberRows();
        for (int row = 0; row < numRows; row++) {
            if (row > 0) customChannelData += ";";
            for (int col = 0; col < numCols; col++) {
                if (col > 0) customChannelData += ",";
                wxString value = StripIllegalChars(grid->GetCellValue(row, col));
                if (value == "0" || value.StartsWith("-") || value.size() > 6) value.clear();
                customChannelData += value;
            }
        }
    }
    return customChannelData;
}

void CustomModelDialog::Save(CustomModel *m) {
    m->SetCustomHeight(HeightSpin->GetValue());
    m->SetCustomWidth(WidthSpin->GetValue());
    m->SetCustomDepth(SpinCtrl_Depth->GetValue());
    std::string customChannelData = GetModelData();
    m->SetCustomData(customChannelData);
    m->SetCustomLightness(lightness);
    m->SetCustomBackground(FilePickerCtrl1->GetFileName().GetFullPath());
}

void CustomModelDialog::OnWidthSpinChange(wxSpinEvent& event)
{
    _changed = true;
    ResizeCustomGrid();
    UpdatePreview();
}

void CustomModelDialog::OnHeightSpinChange(wxSpinEvent& event)
{
    _changed = true;
    ResizeCustomGrid();
    UpdatePreview();
}

void CustomModelDialog::OnSpinCtrl_DepthChange(wxSpinEvent& event)
{
    _changed = true;
    ResizeCustomGrid();
    UpdatePreview();
}

void CustomModelDialog::OnButton_CustomModelZoomInClick(wxCommandEvent& event)
{
    for (auto grid : _grids)
    {
        grid->BeginBatch();
        wxFont font = grid->GetLabelFont();
        font.MakeLarger();
        auto lfs = font.GetPointSize();
        grid->SetLabelFont(font);
        font = grid->GetDefaultCellFont();
        font.SetPointSize(lfs);
        grid->SetDefaultCellFont(font);
        grid->EndBatch();

        font = grid->GetDefaultCellFont();
        SetGridSizeForFont(font);
    }

    UpdateBackground();
}

void CustomModelDialog::SetGridSizeForFont(const wxFont& font)
{
    for (auto grid : _grids) {
        grid->Freeze();
        grid->BeginBatch();
        grid->SetRowMinimalAcceptableHeight(5); //don't need to read text, just see the shape
        grid->SetColMinimalAcceptableWidth(5); //don't need to read text, just see the shape
        for (int c = 0; c < grid->GetNumberCols(); ++c) {
            grid->SetColSize(c, 2 * font.GetPixelSize().y); //GridCustom->GetColSize(c) * 5/4);
        }
        for (int r = 0; r < grid->GetNumberRows(); ++r) {
            grid->SetRowSize(r, int(1.5 * (float)font.GetPixelSize().y)); //GridCustom->GetRowSize(r) * 5/4);
        }
        grid->SetDefaultRowSize(int(1.5 * (float)font.GetPixelSize().y));

        grid->SetDefaultColSize(2 * font.GetPixelSize().y);
        grid->SetColLabelSize(int(1.5 * (float)font.GetPixelSize().y));
        grid->SetRowLabelSize(int(2.5 * font.GetPixelSize().y));

        grid->EndBatch();
        grid->Thaw();
    }
}

void CustomModelDialog::OnButton_CustomModelZoomOutClick(wxCommandEvent& event)
{
    for (auto grid : _grids) {
        grid->BeginBatch();
        wxFont font = grid->GetLabelFont();
        auto fs = font.GetPointSize();
        font.MakeSmaller();
        if (font.GetPointSize() != fs) {
            fs = font.GetPointSize();
            grid->SetLabelFont(font);
            font = grid->GetDefaultCellFont();
            font.SetPointSize(fs);
            grid->SetDefaultCellFont(font);
        }
        font = grid->GetDefaultCellFont();
        SetGridSizeForFont(font);
        grid->EndBatch();
        UpdateBackground();
    }
}

void CustomModelDialog::OnButtonCustomModelHelpClick(wxCommandEvent& event)
{
    DisplayInfo("To create a custom model, set 'Display As' to 'Custom';\nthen set the model width and model height values.\n\nA custom model allows you to represent almost any display element \ncreated from RGB lights. If your element has 12 RGB nodes in it, then \nyou will need to place the numbers 1 through 12 in the grid. \nPlace the numbers so that they are in the shape of your display \nelement. If you enter a number and then need to erase it, enter a 0.\n\nNode 1 will be assigned:\n  StartChannel\n  StartChannel+1\n  StartChannel+2\nNode 2 will be assigned:\n  StartChannel+3\n  StartChannel+4\n  StartChannel+5\netc.\n\nFor example, to model a candy cane with 12 nodes, you could \nstart with a grid 4 columns wide and 10 rows high. You would place the \nnumbers 1-9 up the right-hand side, 10 and 11 would go in the middle \ncells in the top row, and then 12 would go in column A, row 2.\n\n     11 10\n12           9\n               8\n               7\n               6\n               5\n               4\n               3\n               2\n               1");
}

void CustomModelDialog::OnGridCustomCellChange(wxGridEvent& event)
{
    UpdateHighlight(event.GetRow(), event.GetCol());
    _changed = true;
    UpdatePreview();
}

void CustomModelDialog::OnGridCustomCellSelected(wxGridEvent& event)
{
    UpdateHighlight(event.GetRow(), event.GetCol());
    UpdatePreview();
}

void CustomModelDialog::OnBitmapButtonCustomCutClick(wxCommandEvent& event)
{
    _changed = true;
    CutOrCopyToClipboard(true);
    UpdatePreview();
}

void CustomModelDialog::OnBitmapButtonCustomCopyClick(wxCommandEvent& event)
{
    CutOrCopyToClipboard(false);
}

void CustomModelDialog::CopyLayer(bool forward, int layers)
{
    int fromLayer = Notebook1->GetSelection();
    auto fromGrid = GetActiveGrid();
    auto fromCells = fromGrid->GetSelectedCells();
    auto activer = fromGrid->GetGridCursorRow();
    auto activec = fromGrid->GetGridCursorCol();

    if (layers == -1) layers = 999;

    if (forward)
    {
        for (int l  = fromLayer + 1; l <= fromLayer + layers && l < Notebook1->GetPageCount(); l++)
        {
            auto toGrid = GetLayerGrid(l);
            for (auto r = 0; r < fromGrid->GetNumberRows(); r++)
            {
                for (auto c = 0; c < fromGrid->GetNumberCols(); c++)
                {
                    if (fromGrid->IsInSelection(r, c) || (r == activer && c == activec))
                    {
                        toGrid->SetCellValue(r, c, fromGrid->GetCellValue(r, c));
                    }
                }
            }
        }
    }

    // trigger update of what cells are used on other grids
    UpdateGridColours();
}

void CustomModelDialog::UpdateGridColours()
{
    wxBookCtrlEvent e;
    e.SetSelection(Notebook1->GetSelection());
    OnNotebook1PageChanged(e);
}

void CustomModelDialog::CutOrCopyToClipboard(bool IsCut) {

    wxString copy_data;

    auto grid = GetActiveGrid();

    for (int i = 0; i< grid->GetNumberRows(); i++)        // step through all lines
    {
        bool something_in_this_line = false;             // nothing found yet
        for (int k = 0; k<grid->GetNumberCols(); k++)     // step through all colums
        {
            if (grid->IsInSelection(i,k))     // this field is selected!!!
            {
                if (!something_in_this_line)        // first field in this line => may need a linefeed
                {
                    if (!copy_data.IsEmpty())       // ... if it is not the very first field
                    {
                        copy_data += "\n";     // next LINE
                    }
                    something_in_this_line = true;
                }
                else                                    // if not the first field in this line we need a field seperator (TAB)
                {
                    copy_data += "\t";  // next COLUMN
                }
                copy_data += StripIllegalChars(grid->GetCellValue(i,k));    // finally we need the field value
                if (IsCut) grid->SetCellValue(i,k,wxEmptyString);
            }
        }
    }

    if (copy_data.IsEmpty())
    {
        copy_data += StripIllegalChars(grid->GetCellValue(grid->GetGridCursorRow(), grid->GetGridCursorCol()));    // finally we need the field value
        if (IsCut) grid->SetCellValue(grid->GetGridCursorRow(), grid->GetGridCursorCol(), wxEmptyString);
    }

    if (wxTheClipboard->Open())
    {
        if (!wxTheClipboard->SetData(new wxTextDataObject(copy_data)))
        {
            DisplayError(_("Unable to copy data to clipboard."), this);
        }
        wxTheClipboard->Close();
    }
    else
    {
        DisplayError(_("Error opening clipboard."), this);
    }

}

void CustomModelDialog::Paste()
{
    _changed = true;
    wxString copy_data = "";

    copy_data = GetOSFormattedClipboardData();

    if (copy_data == "") {
        if (wxTheClipboard->Open())
        {
            if (wxTheClipboard->IsSupported(wxDF_TEXT))
            {
                wxTextDataObject data;

                if (wxTheClipboard->GetData(data))
                {
                    copy_data = data.GetText();
                }
                else
                {
                    DisplayError(_("Unable to copy data from clipboard."), this);
                }
            }
            else
            {
                DisplayError(_("Non-Text data in clipboard."), this);
            }
            wxTheClipboard->Close();
        }
        else
        {
            DisplayError(_("Error opening clipboard."), this);
            return;
        }
    }

    auto grid = GetActiveGrid();

    int i = grid->GetGridCursorRow();
    int k = grid->GetGridCursorCol();
    int numrows = grid->GetNumberRows();
    int numcols = grid->GetNumberCols();
    bool errflag = false;
    wxString errdetails; //-DJ

    copy_data.Replace("\r\r", "\n");
    copy_data.Replace("\r\n", "\n");
    copy_data.Replace("\r", "\n");

    do
    {
        wxString cur_line = copy_data.BeforeFirst('\n');
        copy_data = copy_data.AfterFirst('\n');
        wxArrayString fields = wxSplit(cur_line, (cur_line.Find(',') != wxNOT_FOUND) ? ',' : '\t'); //allow comma or tab delim -DJ
        for (int fieldnum = 0; fieldnum < fields.Count(); fieldnum++)
        {
            if (i < numrows && k + fieldnum < numcols)
            {
                wxString field = fields[fieldnum].Trim(true).Trim(false);
                long val;
                if (field.IsEmpty() || field.ToLong(&val))
                {
                    grid->SetCellValue(i, k + fieldnum, fields[fieldnum].Trim(true).Trim(false)); //strip surrounding spaces -DJ
                }
                else
                {
                    errflag = true;
                    errdetails += wxString::Format("\n'%s' row %d/col %d of %d", fields[fieldnum].c_str(), i - grid->GetGridCursorRow(), fieldnum, fields.Count()); //tell the user what was wrong; show relative row#, col# (more user friendly) -DJ
                }
            }
        }
        i++;
    } while (copy_data.IsEmpty() == false);

    if (errflag)
    {
        DisplayError(_("One or more of the values were not pasted because they did not contain a number") + errdetails, this); //-DJ
    }
}

void CustomModelDialog::OnBitmapButtonCustomPasteClick(wxCommandEvent& event)
{
    UpdateHighlight(-1, -1);
    _changed = true;
    Paste();
    UpdatePreview();
}

void CustomModelDialog::UpdateBackground()
{
    auto r = _renderers.begin();
    for (auto grid : _grids)
    {
        if (*r != nullptr)
        {
            (*r)->UpdateSize(*grid, bkgrd_active, lightness);
        }
        ++r;
    }
}

void CustomModelDialog::OnBitmapButtonCustomBkgrdClick(wxCommandEvent& event)
{
    _changed = true;
    bkgrd_active = !bkgrd_active;
    GetActiveGrid()->Refresh();
    UpdateBackground();
}

void CustomModelDialog::OnSliderCustomLightnessCmdSliderUpdated(wxScrollEvent& event)
{
    lightness = SliderCustomLightness->GetValue();
    UpdateBackground();
    Refresh();
}

void CustomModelDialog::OnCheckBoxAutoNumberClick(wxCommandEvent& event)
{
    autonumber = CheckBoxAutoNumber->GetValue();
}

void CustomModelDialog::OnCheckBoxAutoIncrementClick(wxCommandEvent& event)
{
    autoincrement = CheckBoxAutoIncrement->GetValue();
}

void CustomModelDialog::OnSpinCtrlNextChannelChange(wxSpinEvent& event)
{
    next_channel = SpinCtrlNextChannel->GetValue();
}

void CustomModelDialog::OnGridCustomCellLeftClick(wxGridEvent& event)
{
    UpdateHighlight(event.GetRow(), event.GetCol());
    if( autonumber ) {
        _changed = true;
        GetActiveGrid()->SetCellValue(event.GetRow(), event.GetCol(), wxString::Format("%d", next_channel) );
        if( autoincrement ) {
            next_channel++;
            SpinCtrlNextChannel->SetValue(next_channel);
        }
        UpdatePreview();
    }
    event.Skip();
}

void CustomModelDialog::OnCheckBox_RearViewClick(wxCommandEvent& event)
{
    for (auto grid : _grids)
    {
        // reverse the rows
        for (auto r = 0; r < grid->GetNumberRows(); r++)
        {
            std::list<wxString> vals;
            for (auto c = 0; c < grid->GetNumberCols(); ++c)
            {
                vals.push_front(grid->GetCellValue(r, c));
            }
            auto c = 0;
            for (const auto& it : vals)
            {
                grid->SetCellValue(r, c++, it);
            }
        }
    }

    UpdateBackground();

    ValidateWindow();
}

void CustomModelDialog::OnButtonCancelClick(wxCommandEvent& event)
{
    if (_changed)
    {
        if (wxMessageBox("Abandon changes to this custom model?", "Abandon changes", wxYES_NO, this) == wxNO)
        {
            return;
        }
    }

    UpdatePreview(_saveWidth, _saveHeight, _saveDepth, _saveModelData);
    _model->GetModelScreenLocation().SetWorldPosition(_saveWorldPos);
    _model->GetModelScreenLocation().SetScaleMatrix(_saveScale);
    ((BoxedScreenLocation&)_model->GetModelScreenLocation()).SetCentreX(_saveCentreX);
    ((BoxedScreenLocation&)_model->GetModelScreenLocation()).SetCentreY(_saveCentreY);
    ((BoxedScreenLocation&)_model->GetModelScreenLocation()).SetCentreZ(_saveCentreZ);
    EndDialog(wxID_CANCEL);
}

void CustomModelDialog::OnButtonOkClick(wxCommandEvent& event)
{
    UpdatePreview(_saveWidth, _saveHeight, _saveDepth, _saveModelData);
    _model->GetModelScreenLocation().SetWorldPosition(_saveWorldPos);
    _model->GetModelScreenLocation().SetScaleMatrix(_saveScale);
    ((BoxedScreenLocation&)_model->GetModelScreenLocation()).SetCentreX(_saveCentreX);
    ((BoxedScreenLocation&)_model->GetModelScreenLocation()).SetCentreY(_saveCentreY);
    ((BoxedScreenLocation&)_model->GetModelScreenLocation()).SetCentreZ(_saveCentreZ);
    EndDialog(wxID_OK);
}

void CustomModelDialog::OnButtonWiringClick(wxCommandEvent& event)
{
    if (Notebook1->GetPageCount() == 1)
    {
        WiringDialog dlg(this, name);
        dlg.SetData(GetActiveGrid(), false);
        dlg.ShowModal();
    }
    else
    {
        wxMessageBox("Wiring view cannot be displayed for 3D models.");
    }
}

void CustomModelDialog::FlipHorizontal()
{
    // reverse the rows
    for (auto grid : _grids)
    {
        for (auto r = 0; r < grid->GetNumberRows(); r++)
        {
            std::list<wxString> vals;
            for (auto c = 0; c < grid->GetNumberCols(); ++c)
            {
                vals.push_front(grid->GetCellValue(r, c));
            }
            auto c = 0;
            for (const auto& it : vals)
            {
                grid->SetCellValue(r, c++, it);
            }
        }
    }

    UpdateBackground();
    UpdatePreview();

    ValidateWindow();
}

void CustomModelDialog::FlipVertical()
{
    // reverse the columns
    for (auto grid : _grids)
    {
        for (auto c = 0; c < grid->GetNumberCols(); c++)
        {
            std::list<wxString> vals;
            for (auto r = 0; r < grid->GetNumberRows(); ++r)
            {
                vals.push_front(grid->GetCellValue(r, c));
            }
            auto r = 0;
            for (const auto& it : vals)
            {
                grid->SetCellValue(r++, c, it);
            }
        }
    }

    UpdateBackground();
    UpdatePreview();

    ValidateWindow();
}

void CustomModelDialog::Rotate90()
{
    for (auto grid : _grids)
    {
        if (grid->GetNumberCols() > grid->GetNumberRows())
        {
            grid->AppendRows(grid->GetNumberCols() - grid->GetNumberRows());
        }
        else if (grid->GetNumberCols() < grid->GetNumberRows())
        {
            grid->AppendCols(grid->GetNumberRows() - grid->GetNumberCols());
        }
    }

    WidthSpin->SetValue(GetActiveGrid()->GetNumberCols());
    HeightSpin->SetValue(GetActiveGrid()->GetNumberRows());
    ResizeCustomGrid();

    for (auto grid : _grids)
    {
        const int n = grid->GetNumberCols();
        for (int i = 0; i < n / 2; i++) {
            for (int j = i; j < n - i - 1; j++) {
                const wxString tmp = grid->GetCellValue(i, j);
                grid->SetCellValue(i, j, grid->GetCellValue(n - j - 1, i));
                grid->SetCellValue(n - j - 1, i, grid->GetCellValue(n - i - 1, n - j - 1));
                grid->SetCellValue(n - i - 1, n - j - 1, grid->GetCellValue(j, n - i - 1));
                grid->SetCellValue(j, n - i - 1, tmp);
            }
        }
    }

    UpdateBackground();
    UpdatePreview();
    ValidateWindow();
}

void CustomModelDialog::CentreModel()
{
    int minx = GetActiveGrid()->GetNumberCols() - 1;
    int miny = GetActiveGrid()->GetNumberRows() - 1;
    int maxx = 0;
    int maxy = 0;

    std::vector<std::vector<std::vector<wxString>>> temp;
    temp.resize(_grids.size(), std::vector<std::vector<wxString>>());
    for (auto& it : temp)
    {
        it.resize(GetActiveGrid()->GetNumberCols(), std::vector<wxString>());
        for (auto& it2 : it)
        {
            it2.resize(GetActiveGrid()->GetNumberRows(), "");
        }
    }

    int g = 0;
    for (auto& grid : _grids)
    {
        for (int x = 0; x < grid->GetNumberCols(); x++)
        {
            for (int y = 0; y < grid->GetNumberRows(); y++)
            {
                if (grid->GetCellValue(x, y) != "")
                {
                    minx = std::min(x, minx);
                    miny = std::min(y, miny);
                    maxx = std::max(x, maxx);
                    maxy = std::max(y, maxy);
                }
                temp[g][x][y] = grid->GetCellValue(x, y);
                grid->SetCellValue(x, y, "");
            }
        }
        g++;
    }

    int deltax = (GetActiveGrid()->GetNumberCols() - (maxx - minx)) / 2;
    int deltay = (GetActiveGrid()->GetNumberRows() - (maxy - miny)) / 2;

    g = 0;
    for (auto& grid : _grids)
    {
        for (int x = minx; x <= maxx; x++)
        {
            for (int y = miny; y <= maxy; y++)
            {
                grid->SetCellValue(x + deltax, y + deltay, temp[g][x][y]);
            }
        }
        g++;
    }
    UpdatePreview();
}

void CustomModelDialog::Rotate()
{
    wxNumberEntryDialog dlg(this, "Degrees to rotate.", "Rotate by", "Rotate", 0, -180, 180);
    if (dlg.ShowModal() == wxID_OK)
    {
        auto degrees = -1 * dlg.GetValue();

        if (degrees != 0)
        {
            int max = std::max(GetActiveGrid()->GetNumberRows(), GetActiveGrid()->GetNumberCols());
            for (auto grid : _grids)
            {
                grid->AppendRows(2 * max - grid->GetNumberRows());
                grid->AppendCols(2 * max - grid->GetNumberCols());
            }
            WidthSpin->SetValue(GetActiveGrid()->GetNumberCols());
            HeightSpin->SetValue(GetActiveGrid()->GetNumberRows());
            ResizeCustomGrid();

            CentreModel();

            int cx = GetActiveGrid()->GetNumberCols() / 2;
            int cy = GetActiveGrid()->GetNumberRows() / 2;

            std::vector<std::vector<std::vector<wxString>>> temp;
            temp.resize(_grids.size(), std::vector<std::vector<wxString>>());
            for (auto& it : temp)
            {
                it.resize(GetActiveGrid()->GetNumberCols(), std::vector<wxString>());
                for (auto& it2 : it)
                {
                    it2.resize(GetActiveGrid()->GetNumberRows(), "");
                }
            }

            // save the current grid
            int g = 0;
            for (const auto& grid : _grids)
            {
                for (int x = 0; x < grid->GetNumberCols(); x++)
                {
                    for (int y = 0; y < grid->GetNumberRows(); y++)
                    {
                        temp[g][x][y] = grid->GetCellValue(x, y);
                        grid->SetCellValue(x, y, "");
                    }
                }
                g++;
            }

            // now rotate and put it back
            double rad = (double)degrees * (M_PI / 180);
            g = 0;
            for (auto& grid : _grids)
            {
                for (int x = 0; x < grid->GetNumberCols(); x++)
                {
                    for (int y = 0; y < grid->GetNumberRows(); y++)
                    {
                        if (temp[g][x][y] != "")
                        {
                            int newx = std::cos(rad) * (x - cx) - std::sin(rad) * (y - cy) + cx;
                            int newy = std::sin(rad) * (x - cx) + std::cos(rad) * (y - cy) + cy;
                            if (grid->GetCellValue(newx, newy) != "")
                            {
                                // ...
                                // ..x
                                // ...
                                newx++;
                            }
                            if (grid->GetCellValue(newx, newy) != "")
                            {
                                // ...
                                // ...
                                // .x.
                                newx--;
                                newy++;
                            }
                            if (grid->GetCellValue(newx, newy) != "")
                            {
                                // ...
                                // ...
                                // ..x
                                newx++;
                            }
                            if (grid->GetCellValue(newx, newy) != "")
                            {
                                // ..x
                                // ...
                                // ...
                                newy -= 2;
                            }
                            if (grid->GetCellValue(newx, newy) != "")
                            {
                                // .x.
                                // ...
                                // ...
                                newx --;
                            }
                            if (grid->GetCellValue(newx, newy) != "")
                            {
                                // x..
                                // ...
                                // ...
                                newx--;
                            }
                            if (grid->GetCellValue(newx, newy) != "")
                            {
                                // ...
                                // x..
                                // ...
                                newy++;
                            }
                            if (grid->GetCellValue(newx, newy) != "")
                            {
                                // ...
                                // ...
                                // x..
                                newy++;
                            }
                            if (grid->GetCellValue(newx, newy) == "")
                            {
                                grid->SetCellValue(newx, newy, temp[g][x][y]);
                            }
                            else
                            {
                                // This is a problem i have tried 9 locations and they are all used ... results are not going to be good
                            }
                        }
                    }
                }
                g++;
            }

            TrimSpace();

            UpdateBackground();
            UpdatePreview();
            ValidateWindow();
        }
    }
}

void CustomModelDialog::GetMinMaxNode(long& min, long& max)
{
    max = 0;
    min = 99999999;

    for (auto grid : _grids) {
        for (auto c = 0; c < grid->GetNumberCols(); c++) {
            for (auto r = 0; r < grid->GetNumberRows(); ++r) {
                wxString s = grid->GetCellValue(r, c);

                if (s.IsEmpty() == false) {
                    long v;
                    if (s.ToCLong(&v) == true) {
                        max = std::max(v, max);
                        min = std::min(v, min);
                    }
                }
            }
        }
    }
}

void CustomModelDialog::MakeSingleNode()
{
    for (auto grid : _grids) {
        for (auto c = 0; c < grid->GetNumberCols(); c++) {
            for (auto r = 0; r < grid->GetNumberRows(); ++r) {
                wxString s = grid->GetCellValue(r, c);

                if (s.IsEmpty() == false) {
                    grid->SetCellValue(r, c, "1");
                }
            }
        }
    }
    UpdatePreview();
}

void CustomModelDialog::Insert(int selRow, int selCol)
{
    long val;
    auto value = GetActiveGrid()->GetCellValue(selRow, selCol);
    value.ToCLong(&val);
    wxNumberEntryDialog dlg(this, wxString::Format("Number of nodes to create a gap for prior to node %ld.", val), "Nodes to create a gap for", "Insert", 1, 1, 500);
    if (dlg.ShowModal() == wxID_OK)
    {
        auto toinsert = dlg.GetValue();

        //Find the max value returned
        auto max = 0;
        for (auto grid : _grids)
        {
            for (auto c = 0; c < grid->GetNumberCols(); c++)
            {
                for (auto r = 0; r < grid->GetNumberRows(); ++r)
                {
                    wxString s = grid->GetCellValue(r, c);

                    if (s.IsEmpty() == false)
                    {
                        long v;
                        if (s.ToCLong(&v) == true)
                        {
                            if (v > max)max = v;
                        }
                    }
                }
            }
        }

        for (auto current = max; current >= val; current--)
        {
            AdjustNodeBy(current, toinsert);
        }
        UpdatePreview();
    }
}

bool CustomModelDialog::AdjustNodeBy(int node, int adjust)
{
    bool adjusted = false;
    for (auto grid : _grids)
    {
        for (auto c = 0; c < grid->GetNumberCols(); c++)
        {
            for (auto r = 0; r < grid->GetNumberRows(); ++r)
            {
                wxString s = grid->GetCellValue(r, c);

                if (s.IsEmpty() == false)
                {
                    long val;
                    s.ToCLong(&val);
                    if (val == node)
                    {
                        grid->SetCellValue(r, c, wxString::Format("%d", val + adjust));
                        adjusted = true;
                    }
                }
            }
        }
    }

    return adjusted;
}

void CustomModelDialog::Compress()
{
    //Find the max value returned
    auto max = 0;
    for (auto grid : _grids)
    {
        for (auto c = 0; c < grid->GetNumberCols(); c++)
        {
            for (auto r = 0; r < grid->GetNumberRows(); ++r)
            {
                wxString s = grid->GetCellValue(r, c);

                if (s.IsEmpty() == false)
                {
                    long val;
                    if (s.ToCLong(&val) == true)
                    {
                        if (val > max)max = val;
                    }
                }
            }
        }
    }

    int adjust = 0;
    for (int current = 1; current <= max; current++)
    {
        if (!AdjustNodeBy(current, adjust))
        {
            adjust--;
        }
    }
    UpdatePreview();
}

void CustomModelDialog::Reverse()
{
    auto min = 1;
    auto max = 1;

    //Find the max value returned
    for (auto grid : _grids)
    {
        for (auto c = 0; c < grid->GetNumberCols(); c++)
        {
            for (auto r = 0; r < grid->GetNumberRows(); ++r)
            {
                wxString s = grid->GetCellValue(r, c);

                if (s.IsEmpty() == false)
                {
                    long val;

                    if (s.ToCLong(&val) == true)
                    {
                        if (val > max)max = val;
                        if (val < min)min = val;
                    }
                }
            }
        }
    }

    max++;
    //Rewrite the grid values
    for (auto grid : _grids)
    {
        for (auto c = 0; c < grid->GetNumberCols(); c++)
        {
            std::list<wxString> vals;
            for (auto r = 0; r < grid->GetNumberRows(); ++r)
            {
                wxString s = grid->GetCellValue(r, c);

                if (s.IsEmpty() == false)
                {
                    long val;

                    if (s.ToCLong(&val) == true)
                    {
                        long newVal = max - val;
                        s.Printf("%d", newVal);

                        grid->SetCellValue(r, c, s);
                    }
                }
            }
        }
    }

    UpdateBackground();
    UpdatePreview();
    ValidateWindow();
}

bool CustomModelDialog::CheckScale(std::list<wxPoint>& points, float scale) const
{
    std::list<wxPoint> newPoints;

    for (auto it = points.begin(); it != points.end(); ++it)
    {
        int newX = (int)((float)it->x * scale);
        int newY = (int)((float)it->y * scale);

        for (auto it2 = newPoints.begin(); it2 != newPoints.end(); ++it2)
        {
            if (it2->x == newX && it2->y == newY)
            {
                return false;
            }
        }
        newPoints.push_back(wxPoint(newX, newY));
    }

    return true;
}

void CustomModelDialog::ExpandSpace()
{
    int width = _grids[0]->GetNumberCols();
    int height = _grids[0]->GetNumberRows();
    int firstRow = 999999;
    int lastRow = -999999;
    int firstCol = 999999;
    int lastCol = -999999;

    for (auto grid : _grids) {
        for (int c = 0; c < grid->GetNumberCols(); c++) {
            for (int r = 0; r < grid->GetNumberRows(); r++) {
                if (!grid->GetCellValue(r, c).IsEmpty()) {
                    if (r < firstRow)
                        firstRow = r;
                    if (r > lastRow)
                        lastRow = r;
                    if (c < firstCol)
                        firstCol = c;
                    if (c > lastCol)
                        lastCol = c;
                }
            }
        }
    }

    if (firstRow == 999999)
        return; // no pixels

    int usedWidth = lastCol - firstCol + 1;
    int usedHeight = lastRow - firstRow + 1;

    if (usedWidth == width || usedHeight == height || usedWidth == 1 || usedHeight == 1)
        return; // nothing to do already expanded

    double expandBy = std::min((double)width / (double)usedWidth, (double)height / (double)usedHeight);

    for (auto grid : _grids) {
        std::list<std::tuple<uint32_t, uint32_t, std::string>> points;
        for (int c = 0; c < grid->GetNumberCols(); c++) {
            for (int r = 0; r < grid->GetNumberRows(); r++) {
                if (!grid->GetCellValue(r, c).IsEmpty()) {
                    auto v = grid->GetCellValue(r, c);

                    uint32_t newr = (r - firstRow) * expandBy;
                    uint32_t newc = (c - firstCol) * expandBy;

                    points.push_back({ newr, newc, v.ToStdString() });

                    grid->SetCellValue(r, c, "");
                }
            }
        }

        for (auto& it : points) {
            grid->SetCellValue(std::get<0>(it), std::get<1>(it), std::get<2>(it));
        }
    }
}

void CustomModelDialog::ShrinkSpace(float min)
{
    TrimSpace();

    float scale = min;
    float scaleFactor = (1.0 - min) / 2.0;

    std::vector<std::list<wxPoint>> points;

    for (auto grid : _grids)
    {
        std::list<wxPoint> gpoints;
        for (int c = 0; c < grid->GetNumberCols(); c++)
        {
            for (int r = 0; r < grid->GetNumberRows(); r++)
            {
                if (!grid->GetCellValue(r, c).IsEmpty())
                {
                    gpoints.push_back(wxPoint(r, c));
                }
            }
        }

        float gscaleFactor = (1.0 - min) / 2.0;
        float gscale = min + gscaleFactor;

        for (int i = 0; i < 5; i++)
        {
            gscaleFactor /= 2.0;
            if (CheckScale(gpoints, gscale))
            {
                gscale -= gscaleFactor;
            }
            else
            {
                gscale += gscaleFactor;
            }
        }

        if (gscale > scale)
        {
            scale = gscale;
            scaleFactor = gscaleFactor;
        }
        points.push_back(gpoints);
    }


    if (scale < min) scale = min;

    for (auto gpoints : points)
    {
        if (!CheckScale(gpoints, scale))
        {
            scale += scaleFactor;
            if (!CheckScale(gpoints, scale))
            {
                // cant scale
                return;
            }
        }
    }

    int i = 0;
    for (auto grid : _grids)
    {
        auto gpoints = points[i];
        for (auto it = gpoints.begin(); it != gpoints.end(); ++it)
        {
            int newX = (int)((float)it->x * scale);
            int newY = (int)((float)it->y * scale);

            if (newX != it->x || newY != it->y)
            {
                wxASSERT(grid->GetCellValue(newX, newY).IsEmpty());

                grid->SetCellValue(newX, newY, grid->GetCellValue(it->x, it->y));
                grid->SetCellValue(it->x, it->y, "");
            }
        }
        i++;
    }

    TrimSpace();
}

void CustomModelDialog::TrimSpace()
{
    int minRow = 99999;
    int maxRow = -99999;
    int minCol = 99999;
    int maxCol = -99999;

    //Find the max value returned
    for (auto grid : _grids)
    {
        for (auto c = 0; c < grid->GetNumberCols(); c++)
        {
            for (auto r = 0; r < grid->GetNumberRows(); ++r)
            {
                wxString s = grid->GetCellValue(r, c);

                if (!s.IsEmpty())
                {
                    if (c < minCol) minCol = c;
                    if (r < minRow) minRow = r;
                    if (c > maxCol) maxCol = c;
                    if (r > maxRow) maxRow = r;
                }
            }
        }
    }

    if (minCol == 0 && minRow == 0 && maxRow == GetActiveGrid()->GetNumberRows() && maxCol == GetActiveGrid()->GetNumberCols())
    {
        // already the right size
        return;
    }

    int adjustRows = -1 * minRow;
    int adjustCols = -1 * minCol;

    for (auto grid : _grids)
    {
        for (int r = minRow; r <= maxRow; r++)
        {
            for (int c = minCol; c <= maxCol; c++)
            {
                grid->SetCellValue(r + adjustRows, c + adjustCols, grid->GetCellValue(r, c));
            }
        }
    }

    WidthSpin->SetValue(std::max(1, maxCol - minCol + 1));
    HeightSpin->SetValue(std::max(1, maxRow - minRow + 1));
    ResizeCustomGrid();
}

void CustomModelDialog::Shift()
{
    auto min = 1;
    auto max = 1;

    //Find the max value returned
    for (auto grid : _grids)
    {
        for (auto c = 0; c < grid->GetNumberCols(); c++)
        {
            for (auto r = 0; r < grid->GetNumberRows(); ++r)
            {
                wxString s = grid->GetCellValue(r, c);

                if (s.IsEmpty() == false)
                {
                    long val;

                    if (s.ToCLong(&val) == true)
                    {
                        if (val > max)max = val;
                        if (val < min)min = val;
                    }
                }
            }
        }
    }

    wxNumberEntryDialog dlg(this, "Enter Increase/Decrease Value", "", "Increment/Decrement Value", 0, -(max - 1), max - 1);
    if (dlg.ShowModal() == wxID_OK)
    {
        auto scaleFactor = dlg.GetValue();
        if (scaleFactor != 0)
        {
            for (auto grid : _grids)
            {
                //Rewrite the grid values
                for (auto c = 0; c < grid->GetNumberCols(); c++)
                {

                    for (auto r = 0; r < grid->GetNumberRows(); ++r)
                    {
                        wxString s = grid->GetCellValue(r, c);

                        if (s.IsEmpty() == false)
                        {
                            long val;

                            if (s.ToCLong(&val) == true)
                            {
                                long newVal = val + scaleFactor;
                                if (newVal > max)
                                {
                                    newVal -= max;
                                }
                                else if (newVal < min)
                                {
                                    newVal += max;
                                }
                                s.Printf("%d", newVal);

                                grid->SetCellValue(r, c, s);
                            }
                        }
                    }
                }
            }

            UpdateBackground();
            UpdatePreview();
            ValidateWindow();
        }
    }
}

void CustomModelDialog::ShiftSelected()
{
    //I think not "wrapping around" is better when shifting selected nodes.
    //I see this being used when a Node is missed and only half need to be shifted by one.
    wxNumberEntryDialog dlg(this, "Enter Increase/Decrease Value", "", "Increment/Decrement Value", 0, -10000, 10000);
    if (dlg.ShowModal() == wxID_OK) {
        auto scaleFactor = dlg.GetValue();
        if (scaleFactor != 0) {
            for (auto grid : _grids) {
                //Rewrite the grid values
                for (auto c = 0; c < grid->GetNumberCols(); c++) {
                    for (auto r = 0; r < grid->GetNumberRows(); ++r) {
                        if (grid->IsInSelection(r, c)) { //only if selected
                            wxString s = grid->GetCellValue(r, c);
                            if (s.IsEmpty() == false) {
                                long val;
                                if (s.ToCLong(&val) == true) {
                                    long newVal = val + scaleFactor;
                                    s.Printf("%d", newVal);
                                    grid->SetCellValue(r, c, s);
                                }
                            }
                        }
                    }
                }
            }
            UpdateBackground();
            UpdatePreview();
            ValidateWindow();
        }
    }
}

void CustomModelDialog::FlipHorzSelected()
{
    for (auto grid : _grids) 
    {
        // Rewrite the grid values
        for (auto r = 0; r < grid->GetNumberRows(); ++r) {
            std::list<wxString> vals;
            for (auto c = 0; c < grid->GetNumberCols(); c++) {
                if (grid->IsInSelection(r, c)) { // only if selected
                    vals.push_front(grid->GetCellValue(r, c));
                }
            }
            for (auto c = 0; c < grid->GetNumberCols(); c++) {
                if (grid->IsInSelection(r, c)) { // only if selected
                    grid->SetCellValue(r, c, vals.front());
                    vals.erase(vals.begin());
                }
            }
        }
    }
    UpdateBackground();
    UpdatePreview();
    ValidateWindow();
}

void CustomModelDialog::FlipVertSelected()
{
    for (auto grid : _grids) {
        // Rewrite the grid values
        for (auto c = 0; c < grid->GetNumberCols(); c++) {
            std::list<wxString> vals;
            for (auto r = 0; r < grid->GetNumberRows(); ++r) {
                if (grid->IsInSelection(r, c)) { // only if selected
                    vals.push_front(grid->GetCellValue(r, c));
                }
            }
            for (auto r = 0; r < grid->GetNumberRows(); ++r) {
                if (grid->IsInSelection(r, c)) { // only if selected
                    grid->SetCellValue(r, c, vals.front());
                    vals.erase(vals.begin());
                }
            }
        }
    }
    UpdateBackground();
    UpdatePreview();
    ValidateWindow();
}

void CustomModelDialog::OnPaste(wxCommandEvent& event)
{
    UpdateHighlight(-1,-1);
    _changed = true;
    Paste();
    UpdatePreview();
}

void CustomModelDialog::OnGridKey(wxCommandEvent& event)
{
    auto col = GetActiveGrid()->GetGridCursorCol();
    auto row = GetActiveGrid()->GetGridCursorRow();

    switch(event.GetInt())
    {
    case WXK_CONTROL_A:
        GetActiveGrid()->SelectAll();
        break;
    case WXK_PAGEUP: // CTRL+SHIFT
        if (Notebook1->GetSelection() != 0)
        {
            int newLayer = Notebook1->GetSelection() - 1;
            Notebook1->SetSelection(Notebook1->GetSelection() - 1);
            GetLayerGrid(newLayer)->SetGridCursor(row, col);
        }
        break;
    case WXK_PAGEDOWN: // CTRL+SHIFT
        if (Notebook1->GetSelection() != Notebook1->GetPageCount() - 1)
        {
            int newLayer = Notebook1->GetSelection() + 1;
            Notebook1->SetSelection(newLayer);
            GetLayerGrid(newLayer)->SetGridCursor(row, col);
        }
        break;
    case WXK_HOME: // CTRL+SHIFT
        Notebook1->SetSelection(0);
        GetLayerGrid(0)->SetGridCursor(row, col);
        break;
    case WXK_END: // CTRL+SHIFT
        Notebook1->SetSelection(Notebook1->GetPageCount()-1);
        GetLayerGrid(Notebook1->GetPageCount()-1)->SetGridCursor(row, col);
        break;
    case WXK_F4:
        PushPull(true, true); // fwd and stay
        break;
    case WXK_F5:
        PushPull(false, true); // back and stay
        break;
    case WXK_F7:
        PushPull(true, false); // fwd and go
        break;
    case WXK_F8:
        PushPull(false, false); // back and go
        break;
    default:
        wxASSERT(false);
        break;
    }
    UpdateHighlight(-1, -1);
}

void CustomModelDialog::UpdateHighlight(int r, int c)
{
    if (r == -1) r = GetActiveGrid()->GetGridCursorRow();
    if (c == -1) c = GetActiveGrid()->GetGridCursorCol();
    if (r != -1 && c != -1) {
        _highlightpixel = wxAtoi(GetActiveGrid()->GetCellValue(r, c));
    }
}

void CustomModelDialog::OnGridPopup(wxCommandEvent& event)
{
    _changed = true;
    int id = event.GetId();
    if (id == CUSTOMMODELDLGMNU_CUT) {
        CutOrCopyToClipboard(true);
    } else if (id == CUSTOMMODELDLGMNU_COPY) {
        CutOrCopyToClipboard(false);
    } else if (id == CUSTOMMODELDLGMNU_PASTE) {
        Paste();
    } else if (id == CUSTOMMODELDLGMNU_DELETE) {
        DeleteCells();
    } else if (id == CUSTOMMODELDLGMNU_FLIPH) {
        FlipHorizontal();
    } else if (id == CUSTOMMODELDLGMNU_FLIPHSELECTED) {
        FlipHorzSelected();
    } else if (id == CUSTOMMODELDLGMNU_FLIPV) {
        FlipVertical();
    } else if (id == CUSTOMMODELDLGMNU_FLIPVSELECTED) {
        FlipVertSelected();
    } else if (id == CUSTOMMODELDLGMNU_ROTATE90) {
        Rotate90();
    } else if (id == CUSTOMMODELDLGMNU_ROTATE) {
        Rotate();
    } else if (id == CUSTOMMODELDLGMNU_REVERSE) {
        Reverse();
    } else if (id == CUSTOMMODELDLGMNU_SHIFT) {
        Shift();
    } else if (id == CUSTOMMODELDLGMNU_SHIFTSELECTED) {
        ShiftSelected();
    } else if (id == CUSTOMMODELDLGMNU_INSERT) {
        Insert(_selRow, _selCol);
    } else if (id == CUSTOMMODELDLGMNU_COMPRESS) {
        Compress();
    } else if (id == CUSTOMMODELDLGMNU_FIND) {
        Find();
    } else if (id == CUSTOMMODELDLGMNU_FINDLAST) {
        FindLast();
    } else if (id == CUSTOMMODELDLGMNU_MAKESINGLENODE) {
        MakeSingleNode();
    } else if (id == CUSTOMMODELDLGMNU_TRIMUNUSEDSPACE) {
        TrimSpace();
    } else if (id == CUSTOMMODELDLGMNU_SHRINKSPACE10) {
        ShrinkSpace(0.9f);
    } else if (id == CUSTOMMODELDLGMNU_SHRINKSPACE50) {
        ShrinkSpace(0.5f);
    } else if (id == CUSTOMMODELDLGMNU_SHRINKSPACE99) {
        ShrinkSpace(0.01f);
    } else if (id == CUSTOMMODELDLGMNU_EXPANDSPACE) {
        ExpandSpace();
    } else if (id == CUSTOMMODELDLGMNU_COPYLAYERFWD1) {
        CopyLayer(true, 1);
    } else if (id == CUSTOMMODELDLGMNU_COPYLAYERBKWD1) {
        CopyLayer(false, 1);
    } else if (id == CUSTOMMODELDLGMNU_COPYLAYERFWDALL) {
        CopyLayer(true, -1);
    } else if (id == CUSTOMMODELDLGMNU_COPYLAYERBKWDALL) {
        CopyLayer(false, -1);
    } else if (id == CUSTOMMODELDLGMNU_CREATESUBMODELFROMALLLAYERS) {
        for (int i = 0; i < Notebook1->GetPageCount(); i++) {
            CreateSubmodelFromLayer(i + 1);
        }
    } else if (id == CUSTOMMODELDLGMNU_CREATESUBMODELFROMLAYER) {
        CreateSubmodelFromLayer(Notebook1->GetSelection() + 1);
    } else if (id == CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMALLLAYERS) {
        for (int i = 0; i < Notebook1->GetPageCount(); i++) {
            CreateMinimalSubmodelFromLayer(i + 1);
        }
    } else if (id == CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMLAYER) {
        CreateMinimalSubmodelFromLayer(Notebook1->GetSelection() + 1);
    } else if (id == CUSTOMMODELDLGMNU_WIREHORIZONTALLEFT ||
               id == CUSTOMMODELDLGMNU_WIREHORIZONTALRIGHT) {
        WireSelectedHorizontal(id);
    } else if (id == CUSTOMMODELDLGMNU_WIREVERTICALTOP ||
               id == CUSTOMMODELDLGMNU_WIREVERTICALBOTTOM) {
        WireSelectedVertical(id);
    }
}

void CustomModelDialog::CreateSubmodelFromLayer(int layer)
{
    int startcol = 0;
    int endcol = GetLayerGrid(layer - 1)->GetNumberCols();
    int startrow = 0;
    int endrow = GetLayerGrid(layer - 1)->GetNumberRows();

    wxXmlNode* sm = new wxXmlNode(wxXML_ELEMENT_NODE, "subModel");
    sm->AddAttribute("name", _model->GenerateUniqueSubmodelName(wxString::Format("Layer%d", layer)));
    sm->AddAttribute("layout", "horizontal");
    sm->AddAttribute("type", "ranges");

    for (int y = startrow; y < endrow; y++) {
        std::string row;
        for (int x = startcol; x < endcol; x++)
        {
            if (x != startcol) row += ",";
            if (!GetLayerGrid(layer - 1)->GetCellValue(y, x).IsEmpty())
            {
                row += GetLayerGrid(layer - 1)->GetCellValue(y, x);
            }
        }

        sm->AddAttribute(wxString::Format("line%d", endrow - y - 1), row);
    }

    _model->AddSubmodel(sm);
}

void CustomModelDialog::CreateMinimalSubmodelFromLayer(int layer)
{
    int startcol = 0;
    int endcol = GetLayerGrid(layer - 1)->GetNumberCols();
    int startrow = 0;
    int endrow = GetLayerGrid(layer - 1)->GetNumberRows();

    bool nonblank = false;
    for (int x = startcol; x < endcol && !nonblank; x++)
    {
        for (int y = startrow; y < endrow && !nonblank; y++)
        {
            if (!GetLayerGrid(layer - 1)->GetCellValue(y, x).IsEmpty())
            {
                nonblank = true;
            }
        }
        if (!nonblank)
        {
            startcol++;
        }
    }
    nonblank = false;
    for (int x = endcol - 1; x >= 0 && !nonblank; x--)
    {
        for (int y = startrow; y < endrow && !nonblank; y++)
        {
            if (!GetLayerGrid(layer - 1)->GetCellValue(y, x).IsEmpty())
            {
                nonblank = true;
            }
        }
        if (!nonblank)
        {
            endcol--;
        }
    }
    nonblank = false;
    for (int y = startrow; y < endrow && !nonblank; y++)
    {
        for (int x = startcol; x < endcol && !nonblank; x++)
        {
            if (!GetLayerGrid(layer - 1)->GetCellValue(y, x).IsEmpty())
            {
                nonblank = true;
            }
        }
        if (!nonblank)
        {
            startrow++;
        }
    }
    nonblank = false;
    for (int y = endrow - 1; y >= 0; y--)
    {
        for (int x = startcol; x < endcol && !nonblank; x++)
        {
            if (!GetLayerGrid(layer - 1)->GetCellValue(y, x).IsEmpty())
            {
                nonblank = true;
            }
        }
        if (!nonblank)
        {
            endrow--;
        }
    }

    wxXmlNode* sm = new wxXmlNode(wxXML_ELEMENT_NODE, "subModel");
    sm->AddAttribute("name", _model->GenerateUniqueSubmodelName(wxString::Format("MinimalLayer%d", layer)));
    sm->AddAttribute("layout", "horizontal");
    sm->AddAttribute("type", "ranges");

    for (int y = startrow; y < endrow; y++) {

        std::string row;
        for (int x = startcol; x < endcol; x++)
        {
            if (x != startcol) row += ",";
            if (!GetLayerGrid(layer - 1)->GetCellValue(y, x).IsEmpty())
            {
                row += GetLayerGrid(layer-1)->GetCellValue(y, x);
            }
        }

        sm->AddAttribute(wxString::Format("line%d", endrow - y - 1), row);
    }

    _model->AddSubmodel(sm);
}

void CustomModelDialog::CreateSubmodelFromRow(int row)
{
    int startcol = 0;
    int endcol = GetActiveGrid()->GetNumberCols();
    int startrow = 0;
    int endrow = SpinCtrl_Depth->GetValue();

    wxXmlNode* sm = new wxXmlNode(wxXML_ELEMENT_NODE, "subModel");
    sm->AddAttribute("name", _model->GenerateUniqueSubmodelName(wxString::Format("Row%d", row)));
    sm->AddAttribute("layout", "horizontal");
    sm->AddAttribute("type", "ranges");

    for (int y = startrow; y < endrow; y++) {
        std::string srow;
        for (int x = startcol; x < endcol; x++)
        {
            if (x != startcol) srow += ",";
            if (!GetLayerGrid(y)->GetCellValue(row - 1, x).IsEmpty())
            {
                srow += GetLayerGrid(y)->GetCellValue(row - 1, x);
            }
        }

        sm->AddAttribute(wxString::Format("line%d", y - startrow), srow);
    }

    _model->AddSubmodel(sm);
}

void CustomModelDialog::CreateMinimalSubmodelFromRow(int row)
{
    int startcol = 0;
    int endcol = GetActiveGrid()->GetNumberCols();
    int startrow = 0;
    int endrow = SpinCtrl_Depth->GetValue();

    bool nonblank = false;
    for (int x = startcol; x < endcol && !nonblank; x++)
    {
        for (int y = startrow; y < endrow && !nonblank; y++)
        {
            if (!GetLayerGrid(y)->GetCellValue(row - 1, x).IsEmpty())
            {
                nonblank = true;
            }
        }
        if (!nonblank)
        {
            startcol++;
        }
    }
    nonblank = false;
    for (int x = endcol - 1; x >= 0 && !nonblank; x--)
    {
        for (int y = startrow; y < endrow && !nonblank; y++)
        {
            if (!GetLayerGrid(y)->GetCellValue(row - 1, x).IsEmpty())
            {
                nonblank = true;
            }
        }
        if (!nonblank)
        {
            endcol--;
        }
    }
    nonblank = false;
    for (int y = startrow; y < endrow && !nonblank; y++)
    {
        for (int x = startcol; x < endcol && !nonblank; x++)
        {
            if (!GetLayerGrid(y)->GetCellValue(row - 1, x).IsEmpty())
            {
                nonblank = true;
            }
        }
        if (!nonblank)
        {
            startrow++;
        }
    }
    nonblank = false;
    for (int y = endrow - 1; y >= 0; y--)
    {
        for (int x = startcol; x < endcol && !nonblank; x++)
        {
            if (!GetLayerGrid(y)->GetCellValue(row - 1, x).IsEmpty())
            {
                nonblank = true;
            }
        }
        if (!nonblank)
        {
            endrow--;
        }
    }

    wxXmlNode* sm = new wxXmlNode(wxXML_ELEMENT_NODE, "subModel");
    sm->AddAttribute("name", _model->GenerateUniqueSubmodelName(wxString::Format("MinimalRow%d", row)));
    sm->AddAttribute("layout", "horizontal");
    sm->AddAttribute("type", "ranges");

    for (int y = startrow; y < endrow; y++) {
        std::string srow;
        for (int x = startcol; x < endcol; x++)
        {
            if (x != startcol) srow += ",";
            if (!GetLayerGrid(y)->GetCellValue(row - 1, x).IsEmpty())
            {
                srow += GetLayerGrid(y)->GetCellValue(row - 1, x);
            }
        }

        sm->AddAttribute(wxString::Format("line%d", y - startrow), srow);
    }

    _model->AddSubmodel(sm);
}

void CustomModelDialog::CreateSubmodelFromColumn(int col)
{
    int startcol = 0;
    int endcol = SpinCtrl_Depth->GetValue();
    int startrow = 0;
    int endrow = GetActiveGrid()->GetNumberRows();

    wxXmlNode* sm = new wxXmlNode(wxXML_ELEMENT_NODE, "subModel");
    sm->AddAttribute("name", _model->GenerateUniqueSubmodelName(wxString::Format("Column%d", col)));
    sm->AddAttribute("layout", "horizontal");
    sm->AddAttribute("type", "ranges");

    for (int y = startrow; y < endrow; y++) {
        std::string row;
        for (int x = endcol - 1; x >= startcol; x--)
        {
            if (x != endcol - 1) row += ",";
            if (!GetLayerGrid(x)->GetCellValue(y, col - 1).IsEmpty())
            {
                row += GetLayerGrid(x)->GetCellValue(y, col - 1);
            }
        }

        sm->AddAttribute(wxString::Format("line%d", endrow - y - 1), row);
    }

    _model->AddSubmodel(sm);
}

void CustomModelDialog::CreateMinimalSubmodelFromColumn(int col)
{
    int startcol = 0;
    int endcol = SpinCtrl_Depth->GetValue();
    int startrow = 0;
    int endrow = GetActiveGrid()->GetNumberRows();

    bool nonblank = false;
    for (int x = startcol; x < endcol && !nonblank; x++)
    {
        for (int y = startrow; y < endrow && !nonblank; y++)
        {
            if (!GetLayerGrid(x)->GetCellValue(y, col -1).IsEmpty())
            {
                nonblank = true;
            }
        }
        if (!nonblank)
        {
            startcol++;
        }
    }
    nonblank = false;
    for (int x = endcol - 1; x >= 0 && !nonblank; x--)
    {
        for (int y = startrow; y < endrow && !nonblank; y++)
        {
            if (!GetLayerGrid(x)->GetCellValue(y, col - 1).IsEmpty())
            {
                nonblank = true;
            }
        }
        if (!nonblank)
        {
            endcol--;
        }
    }
    nonblank = false;
    for (int y = startrow; y < endrow && !nonblank; y++)
    {
        for (int x = startcol; x < endcol && !nonblank; x++)
        {
            if (!GetLayerGrid(x)->GetCellValue(y, col - 1).IsEmpty())
            {
                nonblank = true;
            }
        }
        if (!nonblank)
        {
            startrow++;
        }
    }
    nonblank = false;
    for (int y = endrow - 1; y >= 0; y--)
    {
        for (int x = startcol; x < endcol && !nonblank; x++)
        {
            if (!GetLayerGrid(x)->GetCellValue(y, col - 1).IsEmpty())
            {
                nonblank = true;
            }
        }
        if (!nonblank)
        {
            endrow--;
        }
    }

    wxXmlNode* sm = new wxXmlNode(wxXML_ELEMENT_NODE, "subModel");
    sm->AddAttribute("name", _model->GenerateUniqueSubmodelName(wxString::Format("MinimalColumn%d", col)));
    sm->AddAttribute("layout", "horizontal");
    sm->AddAttribute("type", "ranges");

    for (int y = startrow; y < endrow; y++) {
        std::string row;
        for (int x = endcol - 1; x >= startcol; x--)
        {
            if (x != endcol - 1) row += ",";
            if (!GetLayerGrid(x)->GetCellValue(y, col - 1).IsEmpty())
            {
                row += GetLayerGrid(x)->GetCellValue(y, col - 1);
            }
        }

        sm->AddAttribute(wxString::Format("line%d", endrow - y - 1), row);
    }

    _model->AddSubmodel(sm);
}

void CustomModelDialog::OnGridPopupLabel(wxCommandEvent& event)
{
    int id = event.GetId();

    if (id == CUSTOMMODELDLGMNU_CREATESUBMODELFROMROW)
    {
        CreateSubmodelFromRow(_selRow + 1);
    }
    else if (id == CUSTOMMODELDLGMNU_CREATESUBMODELFROMALLROWS)
    {
        for (int i = 0; i < GetActiveGrid()->GetNumberRows(); i++) {
            CreateSubmodelFromRow(i + 1);
        }
    }
    else if (id == CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMROW)
    {
        CreateMinimalSubmodelFromRow(_selRow + 1);
    }
    else if (id == CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMALLROWS)
    {
        for (int i = 0; i < GetActiveGrid()->GetNumberRows(); i++) {
            CreateMinimalSubmodelFromRow(i + 1);
        }
    }
    else if (id == CUSTOMMODELDLGMNU_CREATESUBMODELFROMCOLUMN)
    {
            CreateSubmodelFromColumn(_selCol + 1);
    }
    else if (id == CUSTOMMODELDLGMNU_CREATESUBMODELFROMALLCOLUMNS)
    {
        for (int i = 0; i < GetActiveGrid()->GetNumberCols(); i++) {
            CreateSubmodelFromColumn(i + 1);
        }
    }
    else if (id == CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMCOLUMN)
    {
        CreateMinimalSubmodelFromColumn(_selCol + 1);
    }
    else if (id == CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMALLCOLUMNS)
    {
        for (int i = 0; i < GetActiveGrid()->GetNumberCols(); i++) {
            CreateMinimalSubmodelFromColumn(i + 1);
        }
    }
}

void CustomModelDialog::OnCut(wxCommandEvent& event)
{
    UpdateHighlight(-1, -1);
    _changed = true;
    CutOrCopyToClipboard(true);
    UpdatePreview();
}

void CustomModelDialog::OnCopy(wxCommandEvent& event)
{
    UpdateHighlight(-1, -1);
    CutOrCopyToClipboard(false);
}

void CustomModelDialog::OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event)
{
    _changed = true;
    background_image = FilePickerCtrl1->GetFileName().GetFullPath();
    if (background_image != "") {
        ObtainAccessToURL(background_image);
        if (FileExists(background_image)) {
            bkg_image = new wxImage(background_image);
            if (!bkg_image->IsOk()) {
                delete bkg_image;
                bkg_image = nullptr;
            }
        } else {
            bkg_image = nullptr;
        }
        for (auto r : _renderers) {
            r->SetImage(bkg_image);
        }
        UpdateBackground();
        GetActiveGrid()->Refresh();
    }
}

void CustomModelDialog::OnGridLabelRightClick(wxGridEvent& event)
{
    int selRow = event.GetRow();
    int selCol = event.GetCol();

    UpdateHighlight(selRow, selCol);

    if (SpinCtrl_Depth->GetValue() > 1 && (selRow >= 0 || selCol >= 0))
    {
        wxMenu mnu;
        mnu.AppendSeparator();
        if (selRow >= 0)
        {
            mnu.Append(CUSTOMMODELDLGMNU_CREATESUBMODELFROMROW, "Create SubModel From Row");
            mnu.Append(CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMROW, "Create Minimal SubModel From Row");
            mnu.Append(CUSTOMMODELDLGMNU_CREATESUBMODELFROMALLROWS, "Create SubModel From All Rows");
            mnu.Append(CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMALLROWS, "Create Minimal SubModel From All Rows");
            _selRow = selRow;
        }
        if (selCol >= 0)
        {
            mnu.Append(CUSTOMMODELDLGMNU_CREATESUBMODELFROMCOLUMN, "Create SubModel From Column");
            mnu.Append(CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMCOLUMN, "Create Minimal SubModel From Column");
            mnu.Append(CUSTOMMODELDLGMNU_CREATESUBMODELFROMALLCOLUMNS, "Create SubModel From AllColumns");
            mnu.Append(CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMALLCOLUMNS, "Create Minimal SubModel From All Columns");
            _selCol = selCol;
        }

        mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&CustomModelDialog::OnGridPopupLabel, nullptr, this);
        PopupMenu(&mnu);
    }
}

void CustomModelDialog::OnGridCustomCellRightClick(wxGridEvent& event)
{
    _selRow = event.GetRow();
    _selCol = event.GetCol();

    UpdateHighlight(_selRow, _selCol);

    GetActiveGrid()->SetGridCursor(_selRow, _selCol);
    auto s = GetActiveGrid()->GetCellValue(_selRow, _selCol);
    bool selectedCellWithValue = !s.IsEmpty() && s.IsNumber();

    wxMenu mnu;
    // Copy / Paste / Delete
    wxMenuItem* menu_cut = mnu.Append(CUSTOMMODELDLGMNU_CUT, "Cut");
    wxMenuItem* menu_copy = mnu.Append(CUSTOMMODELDLGMNU_COPY, "Copy");
    wxMenuItem* menu_paste = mnu.Append(CUSTOMMODELDLGMNU_PASTE, "Paste");
    if (GetActiveGrid()->GetSelectionBlockBottomRight().Count() > 0)
    {
        menu_cut->Enable(true);
        menu_copy->Enable(true);
        menu_paste->Enable(true);
    }
    else
    {
        menu_cut->Enable(false);
        menu_copy->Enable(false);
        menu_paste->Enable(false);
    }
    mnu.AppendSeparator();
    mnu.Append(CUSTOMMODELDLGMNU_DELETE, "Clear Cells");
    mnu.AppendSeparator();
    mnu.Append(CUSTOMMODELDLGMNU_FIND, "Find Node");
    mnu.Append(CUSTOMMODELDLGMNU_FINDLAST, "Find Last Node");
    mnu.AppendSeparator();

    mnu.Append(CUSTOMMODELDLGMNU_FLIPH, "Horizontal Flip");
    if (GetActiveGrid()->GetSelectionBlockBottomRight().Count() > 0) {
        mnu.Append(CUSTOMMODELDLGMNU_FLIPHSELECTED, "Horizontal Flip Selected");
    }
    mnu.Append(CUSTOMMODELDLGMNU_FLIPV, "Vertical Flip");
    if (GetActiveGrid()->GetSelectionBlockBottomRight().Count() > 0) {
        mnu.Append(CUSTOMMODELDLGMNU_FLIPVSELECTED, "Vertical Flip Selected");
    }
    mnu.Append(CUSTOMMODELDLGMNU_ROTATE90, "Rotate 90");
    mnu.Append(CUSTOMMODELDLGMNU_ROTATE, "Rotate x");
    mnu.Append(CUSTOMMODELDLGMNU_REVERSE, "Reverse");
    mnu.Append(CUSTOMMODELDLGMNU_SHIFT, "Shift");
    if (GetActiveGrid()->GetSelectionBlockBottomRight().Count() > 0) {
        mnu.Append(CUSTOMMODELDLGMNU_SHIFTSELECTED, "Shift Selected");
    }
    wxMenuItem* menu_insert = mnu.Append(CUSTOMMODELDLGMNU_INSERT, "Insert Prior");
    mnu.Append(CUSTOMMODELDLGMNU_COMPRESS, "Compress");
    mnu.Append(CUSTOMMODELDLGMNU_TRIMUNUSEDSPACE, "Trim Unused Space");
    mnu.Append(CUSTOMMODELDLGMNU_SHRINKSPACE10, "Shrink Space - Max 10%");
    mnu.Append(CUSTOMMODELDLGMNU_SHRINKSPACE50, "Shrink Space - Max 50%");
    mnu.Append(CUSTOMMODELDLGMNU_SHRINKSPACE99, "Shrink Space - Max 99%");
    mnu.Append(CUSTOMMODELDLGMNU_EXPANDSPACE, "Expand to use Space");
    mnu.Append(CUSTOMMODELDLGMNU_MAKESINGLENODE, "Convert to Single Node");
    if (SpinCtrl_Depth->GetValue() > 1)
    {
        mnu.AppendSeparator();
        mnu.Append(CUSTOMMODELDLGMNU_CREATESUBMODELFROMLAYER, "Create Submodel From Layer");
        mnu.Append(CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMLAYER, "Create Minimal Submodel From Layer");
        mnu.Append(CUSTOMMODELDLGMNU_CREATESUBMODELFROMALLLAYERS, "Create Submodel From All Layers");
        mnu.Append(CUSTOMMODELDLGMNU_CREATEMINIMALSUBMODELFROMALLLAYERS, "Create Minimal Submodel From All Layers");
    }

    if (selectedCellWithValue)
    {
        menu_insert->Enable(true);
    }
    else
    {
        menu_insert->Enable(false);
    }

    mnu.AppendSeparator();
    auto m = mnu.Append(CUSTOMMODELDLGMNU_COPYLAYERFWD1, "Copy Layer Forward 1");
    m->Enable(Notebook1->GetSelection() != Notebook1->GetPageCount() - 1);
    m = mnu.Append(CUSTOMMODELDLGMNU_COPYLAYERFWDALL, "Copy Layer Forward All");
    m->Enable(Notebook1->GetSelection() != Notebook1->GetPageCount() - 1);
    m = mnu.Append(CUSTOMMODELDLGMNU_COPYLAYERBKWD1, "Copy Layer Backward 1");
    m->Enable(Notebook1->GetSelection() != 0);
    m = mnu.Append(CUSTOMMODELDLGMNU_COPYLAYERBKWDALL, "Copy Layer Backward All");
    m->Enable(Notebook1->GetSelection() != 0);

    if (GetActiveGrid()->GetSelectionBlockBottomRight().Count() > 0)
    {
        wxMenu* sub = new wxMenu();
        wxMenu* horz = new wxMenu();
        horz->Append(CUSTOMMODELDLGMNU_WIREHORIZONTALLEFT, "Left -> Right");
        horz->Append(CUSTOMMODELDLGMNU_WIREHORIZONTALRIGHT, "Right -> Left");
        sub->AppendSubMenu(horz, "Horizontial");
        wxMenu* vert = new wxMenu();
        vert->Append(CUSTOMMODELDLGMNU_WIREVERTICALTOP, "Top -> Bottom");
        vert->Append(CUSTOMMODELDLGMNU_WIREVERTICALBOTTOM, "Bottom -> Top");
        sub->AppendSubMenu(vert, "Vertical");
        mnu.AppendSeparator();
        mnu.AppendSubMenu(sub, "Number Selected");
    }

    mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&CustomModelDialog::OnGridPopup, nullptr, this);
    PopupMenu(&mnu);
}

void CustomModelDialog::AddPage()
{
    auto id = wxNewId();
    wxPanel* p = new wxPanel(Notebook1, wxNewId(), wxPoint(26, 52), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    auto sizer = new wxFlexGridSizer(0, 1, 0, 0);
    sizer->AddGrowableCol(0);
    sizer->AddGrowableRow(0);
    auto grid = new CopyPasteGrid(p, id, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxHSCROLL, _T("ID_GRID_Custom"));
    grid->CreateGrid(1, 1);
    grid->EnableEditing(true);
    grid->EnableGridLines(true);

    if (_grids.size() == 0)
    {
        grid->SetColLabelSize(20);
        grid->SetRowLabelSize(30);
        grid->SetDefaultColSize(30, true);
        grid->SetDefaultCellFont(grid->GetFont());
    }
    else
    {
        grid->SetDefaultCellFont(_grids[0]->GetCellFont(0, 0));
        grid->SetCellFont(0, 0, _grids[0]->GetCellFont(0, 0));
        grid->SetLabelFont(_grids[0]->GetLabelFont());
        grid->SetColLabelSize(_grids[0]->GetColLabelSize());
        grid->SetRowLabelSize(_grids[0]->GetRowLabelSize());
        grid->SetDefaultColSize(_grids[0]->GetColSize(0), true);
        grid->SetDefaultRowSize(_grids[0]->GetRowSize(0), true); // for some reason this does not actually work
    }

    grid->SetDefaultCellTextColour(grid->GetForegroundColour());
    sizer->Add(grid, 0, wxEXPAND, 0);
    p->SetSizer(sizer);
    sizer->Fit(p);
    sizer->SetSizeHints(p);
    Notebook1->AddPage(p, wxString::Format("%d", (int)Notebook1->GetPageCount() + 1), false);

    grid->Connect(wxEVT_TEXT_CUT, (wxObjectEventFunction)&CustomModelDialog::OnCut, 0, this);
    grid->Connect(wxEVT_TEXT_COPY, (wxObjectEventFunction)&CustomModelDialog::OnCopy, 0, this);
    grid->Connect(wxEVT_TEXT_PASTE, (wxObjectEventFunction)&CustomModelDialog::OnPaste, 0, this);

    grid->EnableDragColSize(false);
    grid->EnableDragGridSize(false);
    grid->EnableDragRowSize(false);

    while (grid->GetNumberCols() < WidthSpin->GetValue())
    {
        grid->AppendCols();
    }

    while (grid->GetNumberRows() < HeightSpin->GetValue())
    {
        grid->AppendRows();
    }

    auto renderer = new wxModelGridCellRenderer(bkg_image, *grid);
    grid->SetDefaultRenderer(renderer);
    _renderers.push_back(renderer);

    _grids.push_back(grid);

    Connect(id, wxEVT_GRID_CELL_LEFT_CLICK, (wxObjectEventFunction)&CustomModelDialog::OnGridCustomCellLeftClick);
    Connect(id, wxEVT_GRID_CELL_RIGHT_CLICK, (wxObjectEventFunction)&CustomModelDialog::OnGridCustomCellRightClick);
    Connect(id, wxEVT_GRID_CELL_CHANGED, (wxObjectEventFunction)&CustomModelDialog::OnGridCustomCellChange);
    Connect(id, wxEVT_GRID_SELECT_CELL, (wxObjectEventFunction)&CustomModelDialog::OnGridCustomCellSelected);
    grid->Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&CustomModelDialog::OnGridKeyDown, 0, this);

    Connect(id, wxEVT_GRID_LABEL_RIGHT_CLICK, (wxObjectEventFunction)&CustomModelDialog::OnGridLabelRightClick);
}

void CustomModelDialog::PushPull(bool forward, bool stayOnLayer)
{
    auto current = GetActiveGrid();
    int layer = Notebook1->GetSelection();
    auto col = current->GetGridCursorCol();
    auto row = current->GetGridCursorRow();

    if (current->GetCellValue(row, col) == "") return;

    CopyPasteGrid* target = nullptr;
    int targetLayer = -1;

    if (forward) {
        if (layer == 0) return;
        targetLayer = layer - 1;
    }
    else         {
        if (layer == Notebook1->GetPageCount() - 1) return;
        targetLayer = layer + 1;
    }

    target = GetLayerGrid(targetLayer);
    if (target->GetCellValue(row, col) != "") return;

    target->SetCellValue(row, col, current->GetCellValue(row, col));
    current->SetCellValue(row, col, "");

    if (!stayOnLayer) {
        target->SetGridCursor(row, col);
        Notebook1->SetSelection(targetLayer);
    }

    UpdateGridColours();
}

void CustomModelDialog::OnGridKeyDown(wxKeyEvent& event)
{
    if (event.ControlDown()) {
        if (event.GetKeyCode() == 'F') {
            Find();
        }
        else if (event.GetKeyCode() == 'X') {
            CutOrCopyToClipboard(true);
        }
        else if (event.GetKeyCode() == 'C') {
            CutOrCopyToClipboard(false);
        }
        else if (event.GetKeyCode() == 'V') {
            Paste();
        }
        else {
            event.Skip(true);
        }
    }
    else {
        event.Skip(true);
    }
}

CopyPasteGrid* CustomModelDialog::GetActiveGrid() const
{
    return _grids[Notebook1->GetSelection()];
}

CopyPasteGrid* CustomModelDialog::GetLayerGrid(int layer) const
{
    return _grids[layer];
}

void CustomModelDialog::RemovePage()
{
    _grids.pop_back();
    _renderers.pop_back();
    Notebook1->RemovePage(Notebook1->GetPageCount() - 1);
}

void CustomModelDialog::OnNotebook1PageChanged(wxNotebookEvent& event)
{
    CheckBox_Show_Duplicates->SetValue(false);
    ClearDupNodes();
    bool first = (Notebook1->GetSelection() == 0);
    bool last = (Notebook1->GetSelection() == Notebook1->GetPageCount() - 1);

    CopyPasteGrid* prior = nullptr;
    if (!first) prior = _grids[Notebook1->GetSelection() - 1];

    auto current = GetActiveGrid();

    CopyPasteGrid* next = nullptr;
    if (!last) next = _grids[Notebook1->GetSelection() + 1];

    wxColor priorc = wxColor(255, 200, 200);
    wxColor nextc = wxColor(200, 200, 255);
    wxColor priornextc = wxColor(200, 255, 200);
    wxColor defaultColour = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX);

    // Prevent firing if no grids created yet (aka still in setup())
    if (_grids.size() > 0)
    {
        for (auto c = 0; c < current->GetNumberCols(); c++)
        {
            for (auto r = 0; r < current->GetNumberRows(); ++r)
            {
                if (_grids.size() == 1) {

                    if (current->GetCellBackgroundColour(r, c) != defaultColour) {
                        current->SetCellBackgroundColour(r, c, defaultColour);
                    }
                }
                else {
                    int state = 0;

                    if (prior != nullptr && !prior->GetCellValue(r, c).IsEmpty()) state += 1;
                    if (next != nullptr && !next->GetCellValue(r, c).IsEmpty()) state += 2;

                    if (state == 0) {
                        if (current->GetCellBackgroundColour(r, c) != defaultColour) {
                            current->SetCellBackgroundColour(r, c, defaultColour);
                        }
                    }
                    else if (state == 1) {
                        if (current->GetCellBackgroundColour(r, c) != priorc) {
                            current->SetCellBackgroundColour(r, c, priorc);
                        }
                    }
                    else if (state == 2) {
                        if (current->GetCellBackgroundColour(r, c) != nextc) {
                            current->SetCellBackgroundColour(r, c, nextc);
                        }
                    }
                    else if (state == 3) {
                        if (current->GetCellBackgroundColour(r, c) != priornextc) {
                            current->SetCellBackgroundColour(r, c, priornextc);
                        }
                    }
                }
            }
        }
    }
    UpdateHighlight(-1, -1);
}

void CustomModelDialog::OnCheckBox_ShowWiringClick(wxCommandEvent& event)
{
    _modelPreview->SetModel(_model, CheckBox_ShowWiring->IsChecked(), true);
    UpdatePreview();
}

void CustomModelDialog::DeleteCells()
{
    auto grid = GetActiveGrid();

    for (int k = 0; k < grid->GetNumberCols(); k++) {
        for (int i = grid->GetNumberRows(); i >= 0; i--) {
            if (grid->IsInSelection(i, k)) {
                _changed = true;
                grid->SetCellValue(i, k, wxEmptyString);
            }
        }
    }
    UpdatePreview();
}

void CustomModelDialog::WireSelectedHorizontal(long const id)
{
    auto grid = GetActiveGrid();

    bool right = true;
    if (id == CUSTOMMODELDLGMNU_WIREHORIZONTALRIGHT)
    {
        right = false;
    }

    for (int i = grid->GetNumberRows(); i >= 0; i--) {
        if (right) {
            for (int k = 0; k < grid->GetNumberCols(); k++) {
                if (grid->IsInSelection(i, k)) {
                    _changed = true;
                    GetActiveGrid()->SetCellValue(i, k, wxString::Format("%d", next_channel));
                    next_channel++;
                    SpinCtrlNextChannel->SetValue(next_channel);
                    right = false;
                }
            }
        }
        else {
            for (int k = grid->GetNumberCols(); k >= 0; k--) {
                if (grid->IsInSelection(i, k)) {
                    _changed = true;
                    GetActiveGrid()->SetCellValue(i, k, wxString::Format("%d", next_channel));
                    next_channel++;
                    SpinCtrlNextChannel->SetValue(next_channel);
                    right = true;
                }
            }
        }
    }
    UpdatePreview();
}

void CustomModelDialog::WireSelectedVertical(long const id)
{
    auto grid = GetActiveGrid();

    bool up = true;
    if (id == CUSTOMMODELDLGMNU_WIREVERTICALTOP)
    {
        up = false;
    }
    for (int k = 0; k < grid->GetNumberCols(); k++) {
        if (up) {
            for (int i = grid->GetNumberRows(); i >= 0; i--) {
                if (grid->IsInSelection(i, k)) {
                    _changed = true;
                    GetActiveGrid()->SetCellValue(i, k, wxString::Format("%d", next_channel));
                    next_channel++;
                    SpinCtrlNextChannel->SetValue(next_channel);
                    up = false;
                }
            }
        }
        else {
            for (int i = 0; i < grid->GetNumberRows(); i++) {
                if (grid->IsInSelection(i, k)) {
                    _changed = true;
                    GetActiveGrid()->SetCellValue(i, k, wxString::Format("%d", next_channel));
                    next_channel++;
                    SpinCtrlNextChannel->SetValue(next_channel);
                    up = true;
                }
            }
        }
    }
    UpdatePreview();
}

void CustomModelDialog::Find()
{
    long minNode;
    long maxNode;
    GetMinMaxNode(minNode, maxNode);

    if (minNode == 0) {
        wxMessageBox("No nodes present.");
        return;
    }

    wxNumberEntryDialog dlg(this, "Node to find.", "Node to find", "Node", 0, minNode, maxNode);
    if (dlg.ShowModal() == wxID_OK) {
        auto find = dlg.GetValue();

        int g = 0;
        for (auto grid : _grids) {
            for (auto c = 0; c < grid->GetNumberCols(); c++) {
                for (auto r = 0; r < grid->GetNumberRows(); ++r) {
                    wxString s = grid->GetCellValue(r, c);
                    if (s.IsEmpty() == false) {
                        long v;
                        if (s.ToCLong(&v) == true) {
                            if (v == find) {
                                // make this sell active
                                Notebook1->ChangeSelection(g);
                                grid->SetGridCursor(r, c);
                                grid->MakeCellVisible(r, c);
                                return;
                            }
                        }
                    }
                }
            }
            g++;
        }
    }
}

void CustomModelDialog::FindLast()
{
    long minNode;
    long maxNode;
    GetMinMaxNode(minNode, maxNode);

    if (minNode == 0) {
        wxMessageBox("No nodes present.");
        return;
    }

    auto find = maxNode;

    int g = 0;
    for (auto grid : _grids) {
        for (auto c = 0; c < grid->GetNumberCols(); c++) {
            for (auto r = 0; r < grid->GetNumberRows(); ++r) {
                wxString s = grid->GetCellValue(r, c);
                if (s.IsEmpty() == false) {
                    long v;
                    if (s.ToCLong(&v) == true) {
                        if (v == find) {
                            // make this sell active
                            Notebook1->ChangeSelection(g);
                            grid->SetGridCursor(r, c);
                            grid->MakeCellVisible(r, c);
                            return;
                        }
                    }
                }
            }
        }
        g++;
    }
}

void CustomModelDialog::OnSwitchGrid(wxCommandEvent& event)
{
    CheckBox_Show_Duplicates->SetValue(false);
    ClearDupNodes();

    int moveVal = event.GetInt();
    auto col = GetActiveGrid()->GetGridCursorCol();
    auto row = GetActiveGrid()->GetGridCursorRow();

    if (moveVal > 0) {
        if (Notebook1->GetSelection() != 0) {
            int newLayer = Notebook1->GetSelection() - 1;
            Notebook1->SetSelection(Notebook1->GetSelection() - 1);
            GetLayerGrid(newLayer)->SetGridCursor(row, col);
        }
    }
    else {
        if (Notebook1->GetSelection() != Notebook1->GetPageCount() - 1) {
            int newLayer = Notebook1->GetSelection() + 1;
            Notebook1->SetSelection(newLayer);
            GetLayerGrid(newLayer)->SetGridCursor(row, col);
        }
    }
    UpdateHighlight(-1, -1);
}

float CustomModelDialog::GetLineLen(const std::tuple<float, float, float>& pt1, const std::tuple<float, float, float>& pt2) const
{
    float xdiff = std::get<0>(pt1) - std::get<0>(pt2);
    float ydiff = std::get<1>(pt1) - std::get<1>(pt2);
    float zdiff = std::get<2>(pt1) - std::get<2>(pt2);
    return std::sqrt((xdiff * xdiff) + (ydiff * ydiff) + (zdiff * zdiff));
}

void CustomModelDialog::OnButton_ImportFromControllerClick(wxCommandEvent& event)
{
    wxArrayString choices;

    Discovery discovery(this, nullptr); // need to make sure we dont try to discover anything that cares about the output manager pointer
    TwinklyOutput::PrepareDiscovery(discovery);

    discovery.Discover();

    for (int x = 0; x < discovery.GetResults().size(); x++) {
        auto discovered = discovery.GetResults()[x];
        if (!discovered->controller) {
            continue;
        }
        ControllerEthernet* it = discovered->controller;

        if (it->GetProtocol() == OUTPUT_TWINKLY) {
            int32_t startchannel = 1;
            int nullnumber = 0;
            it->SetTransientData(startchannel, nullnumber);

            // If there's another controller already known with the same IP, give the discovered controller the same name
            // Otherwise it'll have a default Twinkly name created from the MAC address that isn't particularly useful if you have lots of devices.
            if (const Controller* existingController = _outputManager->GetControllerWithIP(it->GetIP())) {
                const std::string& existingName = existingController->GetName();
                if (existingName.size() > 0) {
                    it->SetName(existingName);
                }
            }

            choices.push_back(it->GetLongDescription());
        }
    }

#ifndef TEST_TWINKLY_FORMAT
    wxSingleChoiceDialog dlg(this, "Select controller to load from", "Download from controller", choices);

    if (dlg.ShowModal() == wxID_OK) {
        ControllerEthernet* downloadFrom = nullptr;
        for (int x = 0; x < discovery.GetResults().size(); x++) {
            auto discovered = discovery.GetResults()[x];
            if (!discovered->controller) {
                continue;
            }
            ControllerEthernet* it = discovered->controller;
            if (it->GetLongDescription() == choices[dlg.GetSelection()]) {
                downloadFrom = it;
                break;
            }
        }

        if (downloadFrom != nullptr) {
#endif
            std::vector<std::tuple<float, float, float>> modelData;
            bool is3D = false;
            TwinklyOutput::GetLayout(
#ifndef TEST_TWINKLY_FORMAT
                downloadFrom->GetResolvedIP()
#else
                ""
#endif
                , modelData, is3D);

            if (modelData.size() > 0) {
                // set the z size to 1
                SpinCtrl_Depth->SetValue(1);

                // clear existing model data
                DeleteCells();

                // now we need to work out a good size to hold the data
                std::tuple<float, float, float> last = { 9999, 9999, 9999 };
                float minDiff = 9999.0;
                float minX = 9999.0;
                float minY = 9999.0;
                float minZ = 9999.0;
                float maxX = -9990.0;
                float maxY = -9990.0;
                float maxZ = -9990.0;
                for (const auto& it : modelData) {
                    if (std::get<0>(last) == 9999) {
                        // dont process the first one
                    } else {
                        float diff = GetLineLen(last, it);
                        if (diff < minDiff && diff != 0.0f) {
                            minDiff = diff;
                        }
                    }
                    if (std::get<0>(it) < minX)
                        minX = std::get<0>(it);
                    if (std::get<1>(it) < minY)
                        minY = std::get<1>(it);
                    if (std::get<2>(it) < minZ)
                        minZ = std::get<2>(it);
                    if (std::get<0>(it) > maxX)
                        maxX = std::get<0>(it);
                    if (std::get<1>(it) > maxY)
                        maxY = std::get<1>(it);
                    if (std::get<2>(it) > maxZ)
                        maxZ = std::get<2>(it);
                    last = it;
                }

                // These are the max dimensions of the generated model
                #define MAXXY 300.0
                #define MAXZ 30.0

                if (minDiff == 9999.0) {
                    // must just be one node
                    for (auto& it : modelData) {
                        it = { 0, 0, 0 };
                    }
                    HeightSpin->SetValue(1);
                    WidthSpin->SetValue(1);
                } else {

                    float divxy = minX < 0.0 || minY < 0.0 ? MAXXY / 2 : MAXXY;
                    float divz = minZ < 0.0 ? MAXZ / 2 : MAXZ;

                    float multiplierxy = 1.0 / (minDiff < (1.0 / divxy) ? (1.0 / divxy) : minDiff);
                    float multiplierz = 1.0 / (minDiff < (1.0 / divz) ? (1.0 / divz) : minDiff);
                    maxX = 0;
                    maxY = 0;
                    maxZ = 0;

                    // rescale everything and zero base it
                    for (auto& it : modelData) {
                        it = { (std::get<0>(it) - minX) * multiplierxy, (std::get<1>(it) - minY) * multiplierxy, is3D ? (std::get<2>(it) - minZ) * multiplierz : 0 };
                        if (std::get<0>(it) > maxX)
                            maxX = std::get<0>(it);
                        if (std::get<1>(it) > maxY)
                            maxY = std::get<1>(it);
                        if (std::get<2>(it) > maxZ)
                            maxZ = std::get<2>(it);
                    }
                    HeightSpin->SetValue(maxY+1);
                    WidthSpin->SetValue(maxX+1);
                    SpinCtrl_Depth->SetValue(maxZ + 1);
                }

                ResizeCustomGrid();

                auto grid = GetLayerGrid(0);

                int i = 1;
                for (const auto& it : modelData) {
                    auto x = std::get<0>(it);
                    auto y = std::get<1>(it);
                    if (is3D)
                    {
                        grid = GetLayerGrid(std::get<2>(it));
                    }
                    if (grid->GetCellValue(y, x) == "") {
                        grid->SetCellValue(y, x, wxString::Format("%d", i++));
                    }
                }

                UpdatePreview();
                ValidateWindow();
            }
#ifndef TEST_TWINKLY_FORMAT
        }
    }
#endif
}

void CustomModelDialog::OnCheckBox_Show_DuplicatesClick(wxCommandEvent& event)
{
    if (CheckBox_Show_Duplicates->IsChecked()) {
        DrawDupNodes();
    } else {
        ClearDupNodes();
    }

}

void CustomModelDialog::DrawDupNodes()
{
    int const layer = Notebook1->GetSelection();
    int const numCols = WidthSpin->GetValue();
    auto grid = GetActiveGrid();
    _dup_pts.clear();
    for (size_t ii = 0; ii < _model->GetNodeCount(); ii++) {
        std::vector<wxPoint> pts;
        _model->GetNodeCoords(ii, pts);
        if (pts.size() > 1) {
            for (auto const& pt : pts)
            {
                int const x{ pt.x - (numCols * layer) };
                int const y{ (int)_model->GetCustomHeight() - (pt.y) - 1 };
                if (x < 0 || x > numCols) {
                    continue;
                }
                wxPoint npt{ x, y };
                _dup_pts.emplace_back(npt, grid->GetCellBackgroundColour(y, x));
                grid->SetCellBackgroundColour(y, x, *wxYELLOW);
            }
        }
    }
    grid->Refresh();
}

void CustomModelDialog::ClearDupNodes()
{
    auto grid = GetActiveGrid();
    for (auto const& pt :_dup_pts) {
        grid->SetCellBackgroundColour(pt.first.y, pt.first.x, pt.second);
    }
    grid->Refresh();
    _dup_pts.clear();
}

void CustomModelDialog::OnTimer1Trigger(wxTimerEvent& event)
{
    wxASSERT(_outputManager->IsOutputting());
    _outputManager->StartFrame(0);
    auto grid = GetActiveGrid();
    auto value = grid->GetCellValue(grid->GetGridCursorRow(), grid->GetGridCursorCol());
    auto v = -1;
    if (value != "") {
        v = wxAtoi(value) - 1;
    }
    for (uint32_t ch = _model->GetFirstChannel(); ch <= _model->GetLastChannel(); ++ch) {
        auto n = (ch - _model->GetFirstChannel()) / _model->GetChanCountPerNode();
        if (v != -1 && v == n) {
            _outputManager->SetOneChannel(ch, 30);
        } else {
            _outputManager->SetOneChannel(ch, 0);
        }
    }
    _outputManager->EndFrame();
}

void CustomModelDialog::StartOutputToLights()
{
    if (!timer1.IsRunning()) {
        _outputManager->StartOutput();
        timer1.SetOwner(this, ID_TIMER1);
        Connect(ID_TIMER1, wxEVT_TIMER, (wxObjectEventFunction)&CustomModelDialog::OnTimer1Trigger);
        timer1.Start(50, false);
    }
}

bool CustomModelDialog::StopOutputToLights()
{
    if (timer1.IsRunning()) {
        timer1.Stop();
        _outputManager->StartFrame(0);
        _outputManager->AllOff();
        _outputManager->EndFrame();
        _outputManager->StopOutput();
        return true;
    }
    return false;
}

void CustomModelDialog::OnCheckBox_OutputToLightsClick(wxCommandEvent& event)
{
    if (CheckBox_OutputToLights->IsChecked()) {
        StartOutputToLights();
    } else {
        StopOutputToLights();
    }
}
