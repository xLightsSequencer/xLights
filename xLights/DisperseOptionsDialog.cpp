/***************************************************************
* This source files comes from the xLights project
* https://www.xlights.org
* https://github.com/smeighan/xLights
* See the github commit history for a record of contributing
* developers.
* Copyright claimed based on commit dates recorded in Github
* License: https://github.com/smeighan/xLights/blob/master/License.txt
**************************************************************/

#include "DisperseOptionsDialog.h"

//(*InternalHeaders(DisperseOptionsDialog)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dnd.h>
#include <wx/time.h>
#include <wx/persist/toplevel.h>

#include "UtilFunctions.h"

wxDEFINE_EVENT(EVT_SMDROP, wxCommandEvent);

//(*IdInit(DisperseOptionsDialog)
const long DisperseOptionsDialog::ID_STATICTEXT4 = wxNewId();
const long DisperseOptionsDialog::ID_CHOICE_DISPERSE_FROM = wxNewId();
const long DisperseOptionsDialog::ID_CHECKBOX_USE_PRIMARY = wxNewId();
const long DisperseOptionsDialog::ID_STATICTEXT2 = wxNewId();
const long DisperseOptionsDialog::ID_SPINCTRL_OFFSET = wxNewId();
const long DisperseOptionsDialog::ID_CB_FVIEW_UP_LEFT = wxNewId();
const long DisperseOptionsDialog::ID_CB_FVIEW_LEFT = wxNewId();
const long DisperseOptionsDialog::ID_CB_FVIEW_DOWN_LEFT = wxNewId();
const long DisperseOptionsDialog::ID_CB_FVIEW_DOWN = wxNewId();
const long DisperseOptionsDialog::ID_CB_FVIEW_UP = wxNewId();
const long DisperseOptionsDialog::ID_CB_FVIEW_RIGHT = wxNewId();
const long DisperseOptionsDialog::ID_CB_FVIEW_DOWN_RIGHT = wxNewId();
const long DisperseOptionsDialog::ID_TEXTCTRL1 = wxNewId();
const long DisperseOptionsDialog::ID_CB_FVIEW_UP_RIGHT = wxNewId();
const long DisperseOptionsDialog::ID_PANEL1 = wxNewId();
const long DisperseOptionsDialog::ID_CB_TVIEW_BACK_LEFT = wxNewId();
const long DisperseOptionsDialog::ID_CB_TVIEW_LEFT = wxNewId();
const long DisperseOptionsDialog::ID_CB_TVIEW_FRONT_LEFT = wxNewId();
const long DisperseOptionsDialog::ID_CB_TVIEW_FRONT = wxNewId();
const long DisperseOptionsDialog::ID_CB_TVIEW_FRONT_RIGHT = wxNewId();
const long DisperseOptionsDialog::ID_CB_TVIEW_RIGHT = wxNewId();
const long DisperseOptionsDialog::ID_CB_TVIEW_BACK = wxNewId();
const long DisperseOptionsDialog::ID_CB_TVIEW_BACK_RIGHT = wxNewId();
const long DisperseOptionsDialog::ID_TEXTCTRL2 = wxNewId();
const long DisperseOptionsDialog::ID_PANEL2 = wxNewId();
const long DisperseOptionsDialog::ID_NOTEBOOK1 = wxNewId();
const long DisperseOptionsDialog::ID_STATICTEXT3 = wxNewId();
const long DisperseOptionsDialog::ID_DISPERSE_ORDER_LIST = wxNewId();
const long DisperseOptionsDialog::ID_BUTTON_ORDER_UP = wxNewId();
const long DisperseOptionsDialog::ID_BUTTON_ORDER_DOWN = wxNewId();
const long DisperseOptionsDialog::ID_BUTTON_CANCEL = wxNewId();
const long DisperseOptionsDialog::ID_BUTTON_OK = wxNewId();
//*)

BEGIN_EVENT_TABLE(DisperseOptionsDialog,wxDialog)
    //(*EventTable(DisperseOptionsDialog)
    //*)
    EVT_COMMAND(wxID_ANY, EVT_SMDROP, DisperseOptionsDialog::OnDrop)
END_EVENT_TABLE()

DisperseOptionsDialog::DisperseOptionsDialog(wxWindow* parent, wxWindowID id,const wxPoint& pos,const wxSize& size) {
    
    //(*Initialize(DisperseOptionsDialog)
    wxBoxSizer* BoxSizer1;
    wxBoxSizer* BoxSizer2;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxGridBagSizer* GridBagSizerTopView;

    Create(parent, wxID_ANY, _("Disperse from a Target Model"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    FlexGridSizer2 = new wxFlexGridSizer(1, 2, 0, 0);
    FlexGridSizer2->AddGrowableCol(1);
    FlexGridSizer2->AddGrowableRow(0);
    FlexGridSizer5 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer4 = new wxFlexGridSizer(3, 2, 0, 0);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Disperse From"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer4->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ChoiceDisperseFrom = new wxChoice(this, ID_CHOICE_DISPERSE_FROM, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_DISPERSE_FROM"));
    ChoiceDisperseFrom->SetMinSize(wxDLG_UNIT(this,wxSize(-1,-1)));
    ChoiceDisperseFrom->SetToolTip(_("Select the Model to disperse from."));
    FlexGridSizer4->Add(ChoiceDisperseFrom, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBoxUsePrimarySel = new wxCheckBox(this, ID_CHECKBOX_USE_PRIMARY, _("Disperse from Primary Selection"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_USE_PRIMARY"));
    CheckBoxUsePrimarySel->SetValue(false);
    CheckBoxUsePrimarySel->SetMinSize(wxSize(-1,-1));
    FlexGridSizer4->Add(CheckBoxUsePrimarySel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Offset"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer4->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrlOffset = new wxSpinCtrlDouble(this, ID_SPINCTRL_OFFSET, _T("20"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, 0, 500, 0, 1, _T("ID_SPINCTRL_OFFSET"));
    SpinCtrlOffset->SetValue(_T("20"));
    SpinCtrlOffset->SetToolTip(_("Offset/Spacing between dispursed items."));
    FlexGridSizer4->Add(SpinCtrlOffset, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer5->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    DirectionNotebook = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
    DirectionNotebook->SetFocus();
    DirectionNotebook->SetToolTip(_("Select the direction in which to disperse from the Target."));
    DirectionPanelFrontView = new wxPanel(DirectionNotebook, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    GridBagSizerFrontView = new wxGridBagSizer(0, wxDLG_UNIT(DirectionPanelFrontView,wxSize(1,0)).GetWidth());
    DirectionUpLeft = new wxCheckBox(DirectionPanelFrontView, ID_CB_FVIEW_UP_LEFT, _("Up + Left"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CB_FVIEW_UP_LEFT"));
    DirectionUpLeft->SetValue(false);
    GridBagSizerFrontView->Add(DirectionUpLeft, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    DirectionFviewLeft = new wxCheckBox(DirectionPanelFrontView, ID_CB_FVIEW_LEFT, _("Left"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CB_FVIEW_LEFT"));
    DirectionFviewLeft->SetValue(false);
    GridBagSizerFrontView->Add(DirectionFviewLeft, wxGBPosition(3, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    DirectionDownLeft = new wxCheckBox(DirectionPanelFrontView, ID_CB_FVIEW_DOWN_LEFT, _("Down + Left"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CB_FVIEW_DOWN_LEFT"));
    DirectionDownLeft->SetValue(false);
    GridBagSizerFrontView->Add(DirectionDownLeft, wxGBPosition(6, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    DirectionDown = new wxCheckBox(DirectionPanelFrontView, ID_CB_FVIEW_DOWN, _("Down"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CB_FVIEW_DOWN"));
    DirectionDown->SetValue(false);
    GridBagSizerFrontView->Add(DirectionDown, wxGBPosition(6, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    DirectionUp = new wxCheckBox(DirectionPanelFrontView, ID_CB_FVIEW_UP, _("Up"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CB_FVIEW_UP"));
    DirectionUp->SetValue(false);
    GridBagSizerFrontView->Add(DirectionUp, wxGBPosition(0, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    DirectionFviewRight = new wxCheckBox(DirectionPanelFrontView, ID_CB_FVIEW_RIGHT, _("Right"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CB_FVIEW_RIGHT"));
    DirectionFviewRight->SetValue(false);
    GridBagSizerFrontView->Add(DirectionFviewRight, wxGBPosition(3, 5), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    DirectionDownRight = new wxCheckBox(DirectionPanelFrontView, ID_CB_FVIEW_DOWN_RIGHT, _("Down + Right"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CB_FVIEW_DOWN_RIGHT"));
    DirectionDownRight->SetValue(false);
    GridBagSizerFrontView->Add(DirectionDownRight, wxGBPosition(6, 5), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    DirectionFrontLabel = new wxTextCtrl(DirectionPanelFrontView, ID_TEXTCTRL1, _("Direction to Dispurse"), wxDefaultPosition, wxSize(153,22), wxTE_READONLY|wxTE_CENTRE|wxBORDER_RAISED, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    GridBagSizerFrontView->Add(DirectionFrontLabel, wxGBPosition(3, 2), wxDefaultSpan, wxALL|wxEXPAND, 5);
    DirectionUpRight = new wxCheckBox(DirectionPanelFrontView, ID_CB_FVIEW_UP_RIGHT, _("Up + Right"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CB_FVIEW_UP_RIGHT"));
    DirectionUpRight->SetValue(false);
    GridBagSizerFrontView->Add(DirectionUpRight, wxGBPosition(0, 5), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    DirectionPanelFrontView->SetSizer(GridBagSizerFrontView);
    GridBagSizerFrontView->Fit(DirectionPanelFrontView);
    GridBagSizerFrontView->SetSizeHints(DirectionPanelFrontView);
    DirectionPanelTopView = new wxPanel(DirectionNotebook, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    GridBagSizerTopView = new wxGridBagSizer(0, 4);
    DirectionBackLeft = new wxCheckBox(DirectionPanelTopView, ID_CB_TVIEW_BACK_LEFT, _("Back + Left"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CB_TVIEW_BACK_LEFT"));
    DirectionBackLeft->SetValue(false);
    GridBagSizerTopView->Add(DirectionBackLeft, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    DirectionTviewLeft = new wxCheckBox(DirectionPanelTopView, ID_CB_TVIEW_LEFT, _("Left"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CB_TVIEW_LEFT"));
    DirectionTviewLeft->SetValue(false);
    GridBagSizerTopView->Add(DirectionTviewLeft, wxGBPosition(3, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    DirectionFrontLeft = new wxCheckBox(DirectionPanelTopView, ID_CB_TVIEW_FRONT_LEFT, _("Front + Left"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CB_TVIEW_FRONT_LEFT"));
    DirectionFrontLeft->SetValue(false);
    GridBagSizerTopView->Add(DirectionFrontLeft, wxGBPosition(6, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    DirectionFront = new wxCheckBox(DirectionPanelTopView, ID_CB_TVIEW_FRONT, _("Front"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CB_TVIEW_FRONT"));
    DirectionFront->SetValue(false);
    GridBagSizerTopView->Add(DirectionFront, wxGBPosition(6, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    DirectionFrontRight = new wxCheckBox(DirectionPanelTopView, ID_CB_TVIEW_FRONT_RIGHT, _("Front + Right"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CB_TVIEW_FRONT_RIGHT"));
    DirectionFrontRight->SetValue(false);
    GridBagSizerTopView->Add(DirectionFrontRight, wxGBPosition(6, 5), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    DirectionTviewRight = new wxCheckBox(DirectionPanelTopView, ID_CB_TVIEW_RIGHT, _("Right"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CB_TVIEW_RIGHT"));
    DirectionTviewRight->SetValue(false);
    GridBagSizerTopView->Add(DirectionTviewRight, wxGBPosition(3, 5), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    DirectionBack = new wxCheckBox(DirectionPanelTopView, ID_CB_TVIEW_BACK, _("Back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CB_TVIEW_BACK"));
    DirectionBack->SetValue(false);
    GridBagSizerTopView->Add(DirectionBack, wxGBPosition(0, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    DirectionBackRight = new wxCheckBox(DirectionPanelTopView, ID_CB_TVIEW_BACK_RIGHT, _("Back + Right"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CB_TVIEW_BACK_RIGHT"));
    DirectionBackRight->SetValue(false);
    GridBagSizerTopView->Add(DirectionBackRight, wxGBPosition(0, 5), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    DirectionTopLabel = new wxTextCtrl(DirectionPanelTopView, ID_TEXTCTRL2, _("Direction to Dispurse"), wxDefaultPosition, wxSize(148,22), wxTE_READONLY|wxTE_CENTRE|wxBORDER_RAISED, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    GridBagSizerTopView->Add(DirectionTopLabel, wxGBPosition(3, 2), wxDefaultSpan, wxALL|wxEXPAND, 5);
    DirectionPanelTopView->SetSizer(GridBagSizerTopView);
    GridBagSizerTopView->Fit(DirectionPanelTopView);
    GridBagSizerTopView->SetSizeHints(DirectionPanelTopView);
    DirectionNotebook->AddPage(DirectionPanelFrontView, _("Front View"), false);
    DirectionNotebook->AddPage(DirectionPanelTopView, _("Top View"), false);
    FlexGridSizer5->Add(DirectionNotebook, 1, wxALL, 5);
    FlexGridSizer2->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer6 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer6->AddGrowableCol(0);
    FlexGridSizer6->AddGrowableRow(1);
    DisperseOrderLabel = new wxStaticText(this, ID_STATICTEXT3, _("Model Disperse Order"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer6->Add(DisperseOrderLabel, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    DisperseOrderSizer = new wxFlexGridSizer(1, 2, 0, 0);
    DisperseOrderSizer->AddGrowableCol(0);
    DisperseOrderSizer->AddGrowableRow(0);
    DisperseOrderList = new wxListView(this, ID_DISPERSE_ORDER_LIST, wxDefaultPosition, wxDLG_UNIT(this,wxSize(-1,-1)), wxLC_REPORT|wxLC_NO_HEADER|wxLC_SORT_ASCENDING|wxBORDER_SIMPLE|wxVSCROLL|wxHSCROLL|wxFULL_REPAINT_ON_RESIZE, wxDefaultValidator, _T("ID_DISPERSE_ORDER_LIST"));
    DisperseOrderList->SetMaxSize(wxSize(-1,-1));
    DisperseOrderSizer->Add(DisperseOrderList, 1, wxTOP|wxEXPAND, 5);
    BoxSizer2 = new wxBoxSizer(wxVERTICAL);
    ButtonOrderUp = new wxBitmapButton(this, ID_BUTTON_ORDER_UP, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_UP")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BUTTON_ORDER_UP"));
    BoxSizer2->Add(ButtonOrderUp, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonOrderDown = new wxBitmapButton(this, ID_BUTTON_ORDER_DOWN, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_DOWN")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BUTTON_ORDER_DOWN"));
    BoxSizer2->Add(ButtonOrderDown, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    DisperseOrderSizer->Add(BoxSizer2, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer6->Add(DisperseOrderSizer, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
    FlexGridSizer2->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    ButtonCancel = new wxButton(this, ID_BUTTON_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CANCEL"));
    BoxSizer1->Add(ButtonCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonOK = new wxButton(this, ID_BUTTON_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_OK"));
    BoxSizer1->Add(ButtonOK, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_CHOICE_DISPERSE_FROM,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&DisperseOptionsDialog::OnChoiceTargetSelect);
    Connect(ID_CHECKBOX_USE_PRIMARY,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnCheckBoxUsePrimarySelClick);
    Connect(ID_CB_FVIEW_UP_LEFT,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDirectionClick);
    Connect(ID_CB_FVIEW_LEFT,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDirectionClick);
    Connect(ID_CB_FVIEW_DOWN_LEFT,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDirectionClick);
    Connect(ID_CB_FVIEW_DOWN,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDirectionClick);
    Connect(ID_CB_FVIEW_UP,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDirectionClick);
    Connect(ID_CB_FVIEW_RIGHT,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDirectionClick);
    Connect(ID_CB_FVIEW_DOWN_RIGHT,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDirectionClick);
    Connect(ID_CB_FVIEW_UP_RIGHT,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDirectionClick);
    Connect(ID_CB_TVIEW_BACK_LEFT,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDirectionClick);
    Connect(ID_CB_TVIEW_LEFT,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDirectionClick);
    Connect(ID_CB_TVIEW_FRONT_LEFT,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDirectionClick);
    Connect(ID_CB_TVIEW_FRONT,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDirectionClick);
    Connect(ID_CB_TVIEW_FRONT_RIGHT,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDirectionClick);
    Connect(ID_CB_TVIEW_RIGHT,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDirectionClick);
    Connect(ID_CB_TVIEW_BACK,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDirectionClick);
    Connect(ID_CB_TVIEW_BACK_RIGHT,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDirectionClick);
    Connect(ID_DISPERSE_ORDER_LIST,wxEVT_COMMAND_LIST_BEGIN_DRAG,(wxObjectEventFunction)&DisperseOptionsDialog::OnDisperseOrderListBeginDrag);
    Connect(ID_DISPERSE_ORDER_LIST,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDisperseOrderListItemSelect);
    Connect(ID_DISPERSE_ORDER_LIST,wxEVT_COMMAND_LIST_ITEM_DESELECTED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDisperseOrderListItemDeselect);
    Connect(ID_DISPERSE_ORDER_LIST,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&DisperseOptionsDialog::OnDisperseOrderListItemActivated);
    Connect(ID_BUTTON_ORDER_UP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnButtonOrderUpClick);
    Connect(ID_BUTTON_ORDER_DOWN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnButtonOrderDownClick);
    Connect(ID_BUTTON_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnButtonCancelClick);
    Connect(ID_BUTTON_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisperseOptionsDialog::OnButtonOKClick);
    Connect(wxID_ANY,wxEVT_INIT_DIALOG,(wxObjectEventFunction)&DisperseOptionsDialog::OnInit);
    Connect(wxEVT_SIZE,(wxObjectEventFunction)&DisperseOptionsDialog::OnResize);
    //*)

    DisperseOptionsDialogTextDropTarget *mdt = new DisperseOptionsDialogTextDropTarget(this, DisperseOrderList, "ModelOrObject");
    DisperseOrderList->SetDropTarget(mdt);
    FillDirectionCbList();
    
    // remember dialag position and restore otherwise optimise first view of it
    if (!wxPersistentRegisterAndRestore(this, "xLights_DisperseOptionsDialog")) {
        OptimiseDialogPosition(this);
    }
    
    ValidateWindow();
}

void DisperseOptionsDialog::Setup(SetupData data) {
    _is3d = data.is3d;
    _primarySelection = data.primarySelection;
    _availableTargets = data.availableTargets;
    _objectsToDisperse = data.objectsToDisperse;
    _selectedDirection = DisperseDirection::NONE;
    
    // no top view for 2d
    if (!_is3d) {
        DirectionNotebook->RemovePage(1);
    }
    
    // adjust dialog text for 3d objects
    if (!data.forModels) {
        SetTitle("Disperse from a Target 3D Object");
        ChoiceDisperseFrom->SetToolTip("Select the 3D Object to disperse from.");
        DisperseOrderLabel->SetLabel("3D Object Disperse Order");
    }
    
    // if all models selected then default primary as target, probably user error, but handle it
    if (_availableTargets.size() == 0) {
        CheckBoxUsePrimarySel->SetValue(true);
        CheckBoxUsePrimarySel->Disable();
        _availableTargets.push_back(_primarySelection);
        _objectsToDisperse.Remove(_primarySelection);
    }
    
    // single model/object selected, don't allow user to flag it as target
    if (_objectsToDisperse.size() == 1) {
        CheckBoxUsePrimarySel->Disable();
    }
    
    // if primary is locked it has to be the one and only target
    if (data.primaryLocked) {
        CheckBoxUsePrimarySel->SetValue(true);
        CheckBoxUsePrimarySel->Disable();
        _availableTargets.Clear();
        _availableTargets.Add(_primarySelection);
        _objectsToDisperse.Remove(_primarySelection);
    }
        
    PopulateTargetChoices(_availableTargets);
    PopulateOrderList(_objectsToDisperse);
    
    ValidateWindow();
}

void DisperseOptionsDialog::ValidateWindow() {
    if (ChoiceDisperseFrom->GetStringSelection() == "" || _selectedDirection == DisperseDirection::NONE) {
        ButtonOK->Disable();
    } else {
        ButtonOK->Enable();
    }

    int numSelected = DisperseOrderList->GetSelectedItemCount();
    if (numSelected > 0 && numSelected < DisperseOrderList->GetItemCount()) {
        ButtonOrderUp->Enable(true);
        ButtonOrderDown->Enable(true);
    } else {
        ButtonOrderUp->Enable(false);
        ButtonOrderDown->Enable(false);
    }
}

void DisperseOptionsDialog::PopulateOrderList(wxArrayString objectList) {
    DisperseOrderList->Freeze();
    DisperseOrderList->ClearAll();
    DisperseOrderList->AppendColumn("Objects");

    for (const auto& it : objectList) {
        DisperseOrderList->InsertItem(DisperseOrderList->GetItemCount(), it);
    }

    int w,h;
    DisperseOrderList->GetSize(&w, &h);
    DisperseOrderList->SetColumnWidth(0, wxLIST_AUTOSIZE);
    if (DisperseOrderList->GetColumnWidth(0) < w)
    {
        DisperseOrderList->SetColumnWidth(0, w);
    }

    DisperseOrderList->Thaw();
    ValidateWindow();
}

void DisperseOptionsDialog::RemoveItemFromOrderList(wxString objectName) {
    int removeIdx = -1;
    for (size_t i = 0; i < DisperseOrderList->GetItemCount(); i++) {
        if (objectName == DisperseOrderList->GetItemText(i)) {
            removeIdx = i;
            break;
        }
    }

    if (removeIdx > -1) {
        DisperseOrderList->Freeze();
        DisperseOrderList->DeleteItem(removeIdx);
        DisperseOrderList->Thaw();
        DisperseOrderList->Refresh();
    }
}

void DisperseOptionsDialog::PopulateTargetChoices(wxArrayString availableTargets) {
    ChoiceDisperseFrom->Freeze();
    ChoiceDisperseFrom->Clear();
    ChoiceDisperseFrom->Append(availableTargets);
    ChoiceDisperseFrom->Thaw();
    ValidateWindow();
}

void DisperseOptionsDialog::FillDirectionCbList() {
    // repetitive but do it once then we can iterate over all the checkboxes
    _directionCbList.push_back(DirectionFviewLeft);
    _directionCbList.push_back(DirectionFviewRight);
    _directionCbList.push_back(DirectionUp);
    _directionCbList.push_back(DirectionUpLeft);
    _directionCbList.push_back(DirectionUpRight);
    _directionCbList.push_back(DirectionDown);
    _directionCbList.push_back(DirectionDownLeft);
    _directionCbList.push_back(DirectionDownRight);
    _directionCbList.push_back(DirectionBack);
    _directionCbList.push_back(DirectionBackLeft);
    _directionCbList.push_back(DirectionBackRight);
    _directionCbList.push_back(DirectionFront);
    _directionCbList.push_back(DirectionFrontLeft);
    _directionCbList.push_back(DirectionFrontRight);
    _directionCbList.push_back(DirectionTviewLeft);
    _directionCbList.push_back(DirectionTviewRight);
}

wxString DisperseOptionsDialog::GetTarget() {
    return ChoiceDisperseFrom->GetStringSelection();
}

DisperseDirection DisperseOptionsDialog::GetDirection() {
    return _selectedDirection;
}

float DisperseOptionsDialog::GetOffset() {
    return SpinCtrlOffset->GetValue();
}

wxArrayString DisperseOptionsDialog::GetDisperseOrder() {
    wxArrayString modelOrder;

    for (int i = 0; i < DisperseOrderList->GetItemCount(); i++) {
        modelOrder.push_back(DisperseOrderList->GetItemText(i));
    }

    return modelOrder;
}

DisperseOptionsDialog::DisperseOptions DisperseOptionsDialog::GetDisperseOptions() {
    DisperseOptions options;

    options.fromWhat = GetTarget();
    options.direction = GetDirection();
    options.offset = GetOffset();
    options.order = GetDisperseOrder();

    return options;
}

DisperseDirection DisperseOptionsDialog::GetDirectionByControlId(long ctrlId) {

    if (ctrlId == ID_CB_FVIEW_UP) {
        return DisperseDirection::UP;
    } else if (ctrlId == ID_CB_FVIEW_DOWN) {
        return DisperseDirection::DOWN;
    } else if (ctrlId == ID_CB_FVIEW_LEFT || ctrlId == ID_CB_TVIEW_LEFT) {
        return DisperseDirection::LEFT;
    } else if (ctrlId == ID_CB_FVIEW_UP_LEFT) {
        return DisperseDirection::UPLEFT;
    } else if (ctrlId == ID_CB_FVIEW_DOWN_LEFT) {
        return DisperseDirection::DOWNLEFT;
    } else if (ctrlId == ID_CB_FVIEW_RIGHT || ctrlId == ID_CB_TVIEW_RIGHT) {
        return DisperseDirection::RIGHT;
    } else if (ctrlId == ID_CB_FVIEW_UP_RIGHT) {
        return DisperseDirection::UPRIGHT;
    } else if (ctrlId == ID_CB_FVIEW_DOWN_RIGHT) {
        return DisperseDirection::DOWNRIGHT;
    } else if (ctrlId == ID_CB_TVIEW_FRONT) {
        return DisperseDirection::FRONT;
    } else if (ctrlId == ID_CB_TVIEW_BACK) {
        return DisperseDirection::BACK;
    } else if (ctrlId == ID_CB_TVIEW_FRONT_LEFT) {
        return DisperseDirection::FRONTLEFT;
    } else if (ctrlId == ID_CB_TVIEW_FRONT_RIGHT) {
        return DisperseDirection::FRONTRIGHT;
    } else if (ctrlId == ID_CB_TVIEW_BACK_LEFT) {
        return DisperseDirection::BACKLEFT;
    } else if (ctrlId == ID_CB_TVIEW_BACK_RIGHT) {
        return DisperseDirection::BACKRIGHT;
    } else {
        return DisperseDirection::NONE;
    }
}

// move selected items up/down by 1 in order
void DisperseOptionsDialog::StepSelectedItems(bool up) {
    DisperseOrderList->Freeze();

    if (up) {
        for (int i = 0; i < DisperseOrderList->GetItemCount(); ++i) {
            if (DisperseOrderList->IsSelected(i) && i != 0) {
                wxString modelName = DisperseOrderList->GetItemText(i);
                DisperseOrderList->DeleteItem(i);
                int newIdx = DisperseOrderList->InsertItem(i - 1, modelName);
                DisperseOrderList->Select(newIdx);
            }
        }
    } else {
        int numItems = DisperseOrderList->GetItemCount();
        for (int i = numItems - 1; i >= 0; i--) {
            if (DisperseOrderList->IsSelected(i) && i != numItems - 1) {
                wxString modelName = DisperseOrderList->GetItemText(i);
                DisperseOrderList->DeleteItem(i);
                int newIdx = DisperseOrderList->InsertItem(i + 1, modelName);
                DisperseOrderList->Select(newIdx);
            }
        }
    }

    DisperseOrderList->Thaw();
    DisperseOrderList->Refresh();
}

// move selected items to index of whereTo
void DisperseOptionsDialog::MoveSelectedItems(int whereTo) {
    if (whereTo < 0) { return; }

    DisperseOrderList->Freeze();
    wxArrayString draggedModels;
    int selected = DisperseOrderList->GetFirstSelected();

    while (selected != -1) {
        draggedModels.Add(DisperseOrderList->GetItemText(selected));
        DisperseOrderList->DeleteItem(selected);
        selected = DisperseOrderList->GetNextSelected(selected - 1);
    }

    if (whereTo > DisperseOrderList->GetItemCount()) {
        whereTo = DisperseOrderList->GetItemCount();
    }

    for (const auto& it : draggedModels) {
        DisperseOrderList->InsertItem(whereTo, it);
        DisperseOrderList->Select(whereTo);
        whereTo++;
    }

    DisperseOrderList->Thaw();
    DisperseOrderList->Refresh();
}

void DisperseOptionsDialog::OnDisperseOrderListBeginDrag(wxListEvent& event) {
    if (DisperseOrderList->GetSelectedItemCount() == 0) return;

    wxString drag = "ModelOrObject";
    wxTextDataObject my_data(drag);
    wxDropSource dragSource(this);
    dragSource.SetData(my_data);
    dragSource.DoDragDrop(wxDrag_DefaultMove);
    SetCursor(wxCURSOR_HAND);
}

void DisperseOptionsDialog::OnDrop(wxCommandEvent& event) {
    wxArrayString parms = wxSplit(event.GetString(), ',');
    int x = event.GetExtraLong() >> 16;
    int y = event.GetExtraLong() & 0xFFFF;

    switch(event.GetInt()) {
        case 0: {
            // Model dropped, reorder
            int flags = wxLIST_HITTEST_ONITEM;
            long index = DisperseOrderList->HitTest(wxPoint(x, y), flags, nullptr);
            MoveSelectedItems(index);
            break;
        }
        default:
            break;
    }
}

wxDragResult DisperseOptionsDialogTextDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
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
        if (_type == "ModelOrObject" && _list->GetItemCount() > 0)
        {
            int flags = wxLIST_HITTEST_ONITEM;
            int lastItem = _list->HitTest(wxPoint(x, y), flags, nullptr);

            for (int i = 0; i < _list->GetItemCount(); ++i)
            {
                if (i == lastItem)
                {
                    _list->SetItemState(i, wxLIST_STATE_DROPHILITED, wxLIST_STATE_DROPHILITED);
                }
                else
                {
                    _list->SetItemState(i, 0, wxLIST_STATE_DROPHILITED);
                }
            }

            wxRect rect;
            _list->GetItemRect(0, rect);
            int itemSize = rect.GetHeight();

            if (y < 2 * itemSize)
            {
                // scroll up
                if (_list->GetTopItem() > 0)
                {
                    lastTime = wxGetUTCTimeMillis();
                    _list->EnsureVisible(_list->GetTopItem()-1);
                    scrollDelay = scrollDelay / 2;
                    if (scrollDelay < MINSCROLLDELAY) scrollDelay = MINSCROLLDELAY;
                }
            }
            else if (y > _list->GetRect().GetHeight() - itemSize)
            {
                // scroll down
                if (lastItem >= 0 && lastItem < _list->GetItemCount())
                {
                    _list->EnsureVisible(lastItem+1);
                    lastTime = wxGetUTCTimeMillis();
                    scrollDelay = scrollDelay / 2;
                    if (scrollDelay < MINSCROLLDELAY) scrollDelay = MINSCROLLDELAY;
                }
            }
            else
            {
                scrollDelay = STARTSCROLLDELAY;
            }
        }
    }

    return wxDragMove;
}

bool DisperseOptionsDialogTextDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
    if (data == "") return false;

    long mousePos = x;
    mousePos = mousePos << 16;
    mousePos += y;
    wxCommandEvent event(EVT_SMDROP);
    event.SetString(data); // this is the dropped string
    event.SetExtraLong(mousePos); // this is the mouse position packed into a long

    wxArrayString parms = wxSplit(data, ',');

    if (parms[0] == "ModelOrObject")
    {
        if (_type == "ModelOrObject")
        {
            event.SetInt(0);
            wxPostEvent(_owner, event);
            return true;
        }
    }

    return false;
}

void DisperseOptionsDialog::OnButtonOrderUpClick(wxCommandEvent& event) {
    if (DisperseOrderList->GetSelectedItemCount() == 0) return;

    StepSelectedItems(true);
}

void DisperseOptionsDialog::OnButtonOrderDownClick(wxCommandEvent& event) {
    if (DisperseOrderList->GetSelectedItemCount() == 0) return;

    StepSelectedItems(false);
}

void DisperseOptionsDialog::OnResize(wxSizeEvent& event)
{
    wxDialog::OnSize(event);
    DisperseOrderList->Refresh();
}

void DisperseOptionsDialog::OnChoiceTargetSelect(wxCommandEvent& event) {
    ValidateWindow();
}

void DisperseOptionsDialog::OnCheckBoxUsePrimarySelClick(wxCommandEvent& event) {
    DisperseOrderList->Freeze();
    if (CheckBoxUsePrimarySel->IsChecked()) {
        ChoiceDisperseFrom->Clear();
        ChoiceDisperseFrom->Append(_primarySelection);
        RemoveItemFromOrderList(_primarySelection);
    } else {
        ChoiceDisperseFrom->Clear();
        ChoiceDisperseFrom->Append(_availableTargets);
        DisperseOrderList->InsertItem(DisperseOrderList->GetItemCount(), _primarySelection);
    }

    DisperseOrderList->Thaw();
    DisperseOrderList->Refresh();

    ValidateWindow();
}

void DisperseOptionsDialog::OnDirectionClick(wxCommandEvent& event) {
    if (event.IsChecked()) {
        _selectedDirection = GetDirectionByControlId(event.GetId());

        for (const auto& cb : _directionCbList) {
            if (event.GetId() != cb->GetId()) {
                cb->SetValue(false);
            }
        }
    } else {
        _selectedDirection = DisperseDirection::NONE;
    }

    ValidateWindow();
}


void DisperseOptionsDialog::OnInit(wxInitDialogEvent& event) {}

void DisperseOptionsDialog::OnDisperseOrderListItemSelect(wxListEvent& event) {
    ValidateWindow();
}

void DisperseOptionsDialog::OnDisperseOrderListItemDeselect(wxListEvent& event) {
    ValidateWindow();
}

void DisperseOptionsDialog::OnDisperseOrderListItemActivated(wxListEvent& event) {
    ValidateWindow();
}

void DisperseOptionsDialog::OnButtonCancelClick(wxCommandEvent& event) {
    EndDialog(wxID_CANCEL);
}

void DisperseOptionsDialog::OnButtonOKClick(wxCommandEvent& event) {
    EndDialog(wxID_OK);
}

DisperseOptionsDialog::~DisperseOptionsDialog() {
    //(*Destroy(DisperseOptionsDialog)
    //*)
}
