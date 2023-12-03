/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*InternalHeaders(ModelGroupPanel)
 #include <wx/bitmap.h>
 #include <wx/image.h>
 #include <wx/intl.h>
 #include <wx/string.h>
 //*)

#include <wx/artprov.h>
#include <wx/xml/xml.h>
#include <wx/time.h>

#include "ModelGroupPanel.h"
#include "models/ModelManager.h"
#include "models/ModelGroup.h"
#include "LayoutPanel.h"
#include "OutputModelManager.h"
#include "xLightsMain.h"
#include "UtilFunctions.h"
#include "EditAliasesDialog.h"

#include <log4cpp/Category.hh>

// This event is fired when a model is dropped between lists
wxDEFINE_EVENT(EVT_MGDROP, wxCommandEvent);

class MGDropSource : public wxDropSource
{
    ModelGroupPanel* _window;
    bool _nonModels;
    bool _models;

public:

    MGDropSource(ModelGroupPanel* window, bool models, bool nonModels) : wxDropSource(window)
    {
        _window = window;
        _nonModels = nonModels;
        _models = models;
    }

    virtual bool GiveFeedback(wxDragResult effect) override
    {
        wxPoint point = wxGetMousePosition();

        if (_models)
        {
            if (_window->ListBoxModelsInGroup->GetScreenRect().Contains(point) ||
                _window->ListBoxAddToModelGroup->GetScreenRect().Contains(point))
            {
                _window->SetCursor(wxCursor(wxCURSOR_HAND));
            }
            else
            {
                _window->SetCursor(wxCursor(wxCURSOR_NO_ENTRY));
            }
        }
        else if (_nonModels)
        {
            if (_window->ListBoxModelsInGroup->GetScreenRect().Contains(point))
            {
                _window->SetCursor(wxCursor(wxCURSOR_HAND));
            }
            else
            {
                _window->SetCursor(wxCursor(wxCURSOR_NO_ENTRY));
            }
        }
        else
        {
            return false;
        }

        return true;
    }
};

//(*IdInit(ModelGroupPanel)
const long ModelGroupPanel::ID_STATICTEXT5 = wxNewId();
const long ModelGroupPanel::ID_STATICTEXT6 = wxNewId();
const long ModelGroupPanel::ID_CHOICE1 = wxNewId();
const long ModelGroupPanel::ID_STATICTEXT12 = wxNewId();
const long ModelGroupPanel::ID_CHOICE2 = wxNewId();
const long ModelGroupPanel::ID_STATICTEXT4 = wxNewId();
const long ModelGroupPanel::ID_SPINCTRL1 = wxNewId();
const long ModelGroupPanel::ID_CHOICE_PREVIEWS = wxNewId();
const long ModelGroupPanel::ID_STATICTEXT7 = wxNewId();
const long ModelGroupPanel::ID_STATICTEXT10 = wxNewId();
const long ModelGroupPanel::ID_SPINCTRL2 = wxNewId();
const long ModelGroupPanel::ID_STATICTEXT11 = wxNewId();
const long ModelGroupPanel::ID_SPINCTRL3 = wxNewId();
const long ModelGroupPanel::ID_STATICTEXT8 = wxNewId();
const long ModelGroupPanel::ID_COLOURPICKERCTRL_MG_TAGCOLOUR = wxNewId();
const long ModelGroupPanel::ID_BUTTON2 = wxNewId();
const long ModelGroupPanel::ID_CHECKBOX1 = wxNewId();
const long ModelGroupPanel::ID_CHECKBOX3 = wxNewId();
const long ModelGroupPanel::ID_CHECKBOX2 = wxNewId();
const long ModelGroupPanel::ID_CHECKBOX4 = wxNewId();
const long ModelGroupPanel::ID_STATICTEXT3 = wxNewId();
const long ModelGroupPanel::ID_STATICTEXT2 = wxNewId();
const long ModelGroupPanel::ID_STATICTEXT9 = wxNewId();
const long ModelGroupPanel::ID_TEXTCTRL1 = wxNewId();
const long ModelGroupPanel::ID_BUTTON1 = wxNewId();
const long ModelGroupPanel::ID_LISTCTRL1 = wxNewId();
const long ModelGroupPanel::ID_BITMAPBUTTON4 = wxNewId();
const long ModelGroupPanel::ID_BITMAPBUTTON3 = wxNewId();
const long ModelGroupPanel::ID_BITMAPBUTTON1 = wxNewId();
const long ModelGroupPanel::ID_BITMAPBUTTON2 = wxNewId();
const long ModelGroupPanel::ID_STATICTEXT1 = wxNewId();
const long ModelGroupPanel::ID_LISTCTRL2 = wxNewId();
//*)

const long ModelGroupPanel::ID_MNU_CLEARALL = wxNewId();
const long ModelGroupPanel::ID_MNU_COPY = wxNewId();
const long ModelGroupPanel::ID_MNU_SORTBYNAME = wxNewId();

BEGIN_EVENT_TABLE(ModelGroupPanel,wxPanel)
	//(*EventTable(ModelGroupPanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_MGDROP, ModelGroupPanel::OnDrop)
END_EVENT_TABLE()

ModelGroupPanel::ModelGroupPanel(wxWindow* parent, ModelManager &Models, LayoutPanel* xl,wxWindowID id, const wxPoint& pos, const wxSize& size)
:   layoutPanel(xl), mModels(Models)
{
	//(*Initialize(ModelGroupPanel)
	wxFlexGridSizer* FlexGridSizer11;
	wxFlexGridSizer* FlexGridSizer12;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;
	wxStaticText* StaticText4;
	wxStaticText* StaticText6;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	Panel_Sizer = new wxFlexGridSizer(0, 1, 0, 0);
	Panel_Sizer->AddGrowableCol(0);
	Panel_Sizer->AddGrowableRow(0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(1);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(1);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Model Group Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer6->Add(StaticText5, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	LabelModelGroupName = new wxStaticText(this, ID_STATICTEXT6, _("<group name>"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer6->Add(LabelModelGroupName, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, wxID_ANY, _("Default Layout Mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText4, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	ChoiceModelLayoutType = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	ChoiceModelLayoutType->SetSelection( ChoiceModelLayoutType->Append(_("Grid as per preview")) );
	ChoiceModelLayoutType->Append(_("Minimal Grid"));
	ChoiceModelLayoutType->Append(_("Horizontal Stack"));
	ChoiceModelLayoutType->Append(_("Vertical Stack"));
	ChoiceModelLayoutType->Append(_("Horizontal Stack - Scaled"));
	ChoiceModelLayoutType->Append(_("Vertical Stack - Scaled"));
	ChoiceModelLayoutType->Append(_("Horizontal Per Model"));
	ChoiceModelLayoutType->Append(_("Vertical Per Model"));
	ChoiceModelLayoutType->Append(_("Horizontal Per Model/Strand"));
	ChoiceModelLayoutType->Append(_("Vertical Per Model/Strand"));
	ChoiceModelLayoutType->Append(_("Single Line"));
	ChoiceModelLayoutType->Append(_("Overlay - Centered"));
	ChoiceModelLayoutType->Append(_("Overlay - Scaled"));
	ChoiceModelLayoutType->Append(_("Single Line Model As A Pixel"));
	ChoiceModelLayoutType->Append(_("Default Model As A Pixel"));
	FlexGridSizer6->Add(ChoiceModelLayoutType, 1, wxALL|wxEXPAND, 2);
	StaticText12 = new wxStaticText(this, ID_STATICTEXT12, _("Default Camera:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
	FlexGridSizer6->Add(StaticText12, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_DefaultCamera = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	FlexGridSizer6->Add(Choice_DefaultCamera, 1, wxALL|wxEXPAND, 5);
	GridSizeLabel = new wxStaticText(this, ID_STATICTEXT4, _("Max Grid Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer6->Add(GridSizeLabel, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	SizeSpinCtrl = new wxSpinCtrl(this, ID_SPINCTRL1, _T("400"), wxDefaultPosition, wxDefaultSize, 0, 10, 2000, 400, _T("ID_SPINCTRL1"));
	SizeSpinCtrl->SetValue(_T("400"));
	FlexGridSizer6->Add(SizeSpinCtrl, 1, wxALL|wxEXPAND, 2);
	StaticText6 = new wxStaticText(this, wxID_ANY, _("Preview:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText6, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	ChoicePreviews = new wxChoice(this, ID_CHOICE_PREVIEWS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PREVIEWS"));
	FlexGridSizer6->Add(ChoicePreviews, 1, wxALL|wxEXPAND, 2);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Center Offset:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer6->Add(StaticText7, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4 = new wxFlexGridSizer(0, 4, 0, 0);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("X"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer4->Add(StaticText10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_XCentreOffset = new wxSpinCtrl(this, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -1000, 1000, 0, _T("ID_SPINCTRL2"));
	SpinCtrl_XCentreOffset->SetValue(_T("0"));
	FlexGridSizer4->Add(SpinCtrl_XCentreOffset, 1, wxALL|wxEXPAND, 2);
	StaticText11 = new wxStaticText(this, ID_STATICTEXT11, _("Y"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	FlexGridSizer4->Add(StaticText11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_YCentreOffset = new wxSpinCtrl(this, ID_SPINCTRL3, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -1000, 1000, 0, _T("ID_SPINCTRL3"));
	SpinCtrl_YCentreOffset->SetValue(_T("0"));
	FlexGridSizer4->Add(SpinCtrl_YCentreOffset, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer6->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Tag Color:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer6->Add(StaticText8, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	ColourPickerCtrl_ModelGroupTagColour = new wxColourPickerCtrl(this, ID_COLOURPICKERCTRL_MG_TAGCOLOUR, wxColour(0,0,0), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_COLOURPICKERCTRL_MG_TAGCOLOUR"));
	FlexGridSizer5->Add(ColourPickerCtrl_ModelGroupTagColour, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ButtonAliases = new wxButton(this, ID_BUTTON2, _("Aliases"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer5->Add(ButtonAliases, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
	CheckBox_ShowSubmodels = new wxCheckBox(this, ID_CHECKBOX1, _("Show SubModels to Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_ShowSubmodels->SetValue(true);
	FlexGridSizer6->Add(CheckBox_ShowSubmodels, 1, wxALL|wxEXPAND, 2);
	CheckBox_ShowInactiveModels = new wxCheckBox(this, ID_CHECKBOX3, _("Show Inactive Models to Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	CheckBox_ShowInactiveModels->SetValue(false);
	FlexGridSizer6->Add(CheckBox_ShowInactiveModels, 1, wxALL|wxEXPAND, 5);
	CheckBox_ShowModelGroups = new wxCheckBox(this, ID_CHECKBOX2, _("Show Model Groups to Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_ShowModelGroups->SetValue(true);
	FlexGridSizer6->Add(CheckBox_ShowModelGroups, 1, wxALL|wxEXPAND, 2);
	CheckBox_ShowOnlyModelsInCurrentView = new wxCheckBox(this, ID_CHECKBOX4, _("Show Only Models In Current View"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
	CheckBox_ShowOnlyModelsInCurrentView->SetValue(true);
	FlexGridSizer6->Add(CheckBox_ShowOnlyModelsInCurrentView, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer12 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer12->AddGrowableCol(0);
	FlexGridSizer12->AddGrowableCol(2);
	FlexGridSizer12->AddGrowableRow(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Add to Group:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer12->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer12->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Models in Group:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer12->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer2->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_Filter = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer2->Add(TextCtrl_Filter, 1, wxALL|wxEXPAND, 1);
	ButtonClearFilter = new wxButton(this, ID_BUTTON1, _("x"), wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(ButtonClearFilter, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	ListBoxAddToModelGroup = new wxListCtrl(this, ID_LISTCTRL1, wxDefaultPosition, wxDLG_UNIT(this,wxSize(65,-1)), wxLC_REPORT|wxLC_NO_HEADER|wxLC_SORT_ASCENDING, wxDefaultValidator, _T("ID_LISTCTRL1"));
	ListBoxAddToModelGroup->SetMinSize(wxSize(65,-1));
	FlexGridSizer1->Add(ListBoxAddToModelGroup, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer12->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer11 = new wxFlexGridSizer(0, 1, 0, 0);
	ButtonAddModel = new wxBitmapButton(this, ID_BITMAPBUTTON4, wxArtProvider::GetBitmapBundle("wxART_GO_FORWARD", wxART_TOOLBAR), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON4"));
	FlexGridSizer11->Add(ButtonAddModel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	ButtonRemoveModel = new wxBitmapButton(this, ID_BITMAPBUTTON3, wxArtProvider::GetBitmapBundle("wxART_GO_BACK", wxART_TOOLBAR), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON3"));
	FlexGridSizer11->Add(ButtonRemoveModel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	wxSize __SpacerSize_1 = wxDLG_UNIT(this,wxSize(-1,7));
	FlexGridSizer11->Add(__SpacerSize_1.GetWidth(),__SpacerSize_1.GetHeight(),1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonMoveUp = new wxBitmapButton(this, ID_BITMAPBUTTON1, wxArtProvider::GetBitmapBundle("wxART_GO_UP", wxART_TOOLBAR), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
	FlexGridSizer11->Add(ButtonMoveUp, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	ButtonMoveDown = new wxBitmapButton(this, ID_BITMAPBUTTON2, wxArtProvider::GetBitmapBundle("wxART_GO_DOWN", wxART_TOOLBAR), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON2"));
	FlexGridSizer11->Add(ButtonMoveDown, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer11->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer12->Add(FlexGridSizer11, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 2);
	ListBoxModelsInGroup = new wxListCtrl(this, ID_LISTCTRL2, wxDefaultPosition, wxDLG_UNIT(this,wxSize(65,-1)), wxLC_REPORT|wxLC_NO_HEADER, wxDefaultValidator, _T("ID_LISTCTRL2"));
	ListBoxModelsInGroup->SetMinSize(wxSize(65,-1));
	FlexGridSizer12->Add(ListBoxModelsInGroup, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 0);
	FlexGridSizer3->Add(FlexGridSizer12, 1, wxALL|wxEXPAND, 0);
	Panel_Sizer->Add(FlexGridSizer3, 0, wxEXPAND, 0);
	SetSizer(Panel_Sizer);
	Panel_Sizer->Fit(this);
	Panel_Sizer->SetSizeHints(this);

	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ModelGroupPanel::OnChoiceModelLayoutTypeSelect);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ModelGroupPanel::OnChoice_DefaultCameraSelect);
	Connect(ID_SPINCTRL1,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ModelGroupPanel::OnSizeSpinCtrlChange);
	Connect(ID_CHOICE_PREVIEWS,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ModelGroupPanel::OnChoicePreviewsSelect);
	Connect(ID_SPINCTRL2,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ModelGroupPanel::OnSpinCtrl_XCentreOffsetChange);
	Connect(ID_SPINCTRL3,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ModelGroupPanel::OnSpinCtrl_YCentreOffsetChange);
	Connect(ID_COLOURPICKERCTRL_MG_TAGCOLOUR,wxEVT_COMMAND_COLOURPICKER_CHANGED,(wxObjectEventFunction)&ModelGroupPanel::OnColourPickerCtrl_ModelGroupTagColourColourChanged);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelGroupPanel::OnButtonAliasesClick);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ModelGroupPanel::OnCheckBox_ShowSubmodelsClick);
	Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ModelGroupPanel::OnCheckBox_ShowInactiveModelsClick);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ModelGroupPanel::OnCheckBox_ShowModelGroupsClick);
	Connect(ID_CHECKBOX4,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ModelGroupPanel::OnCheckBox_ShowOnlyModelsInCurrentViewClick);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ModelGroupPanel::OnTextCtrl_FilterText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelGroupPanel::OnButtonClearFilterClick);
	Connect(ID_LISTCTRL1,wxEVT_COMMAND_LIST_BEGIN_DRAG,(wxObjectEventFunction)&ModelGroupPanel::OnListBoxAddToModelGroupBeginDrag);
	Connect(ID_LISTCTRL1,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&ModelGroupPanel::OnListBoxAddToModelGroupItemSelect);
	Connect(ID_LISTCTRL1,wxEVT_COMMAND_LIST_ITEM_DESELECTED,(wxObjectEventFunction)&ModelGroupPanel::OnListBoxAddToModelGroupItemDeselect);
	Connect(ID_LISTCTRL1,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&ModelGroupPanel::OnListBoxAddToModelGroupItemActivated);
	Connect(ID_BITMAPBUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelGroupPanel::OnButtonAddToModelGroupClick);
	Connect(ID_BITMAPBUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelGroupPanel::OnButtonRemoveFromModelGroupClick);
	Connect(ID_BITMAPBUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelGroupPanel::OnButtonUpClick);
	Connect(ID_BITMAPBUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelGroupPanel::OnButtonDownClick);
	Connect(ID_LISTCTRL2,wxEVT_COMMAND_LIST_BEGIN_DRAG,(wxObjectEventFunction)&ModelGroupPanel::OnListBoxModelsInGroupBeginDrag);
	Connect(ID_LISTCTRL2,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&ModelGroupPanel::OnListBoxModelsInGroupItemSelect);
	Connect(ID_LISTCTRL2,wxEVT_COMMAND_LIST_ITEM_DESELECTED,(wxObjectEventFunction)&ModelGroupPanel::OnListBoxModelsInGroupItemDeselect);
	Connect(ID_LISTCTRL2,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&ModelGroupPanel::OnListBoxModelsInGroupItemActivated);
	//*)

    Connect(ID_LISTCTRL2, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&ModelGroupPanel::OnListBoxModelsInGroupItemRClick);

    ChoicePreviews->Append("Default");
    ChoicePreviews->Append("All Previews");
    ChoicePreviews->Append("Unassigned");

    Choice_DefaultCamera->Append("2D");
    for (size_t i = 0; i < Models.GetXLightsFrame()->viewpoint_mgr.GetNum3DCameras(); ++i) {
        Choice_DefaultCamera->Append(Models.GetXLightsFrame()->viewpoint_mgr.GetCamera3D(i)->GetName());
    }

    MGTextDropTarget *mdt = new MGTextDropTarget(this, ListBoxModelsInGroup, "ModelGroup");
    ListBoxModelsInGroup->SetDropTarget(mdt);

    mdt = new MGTextDropTarget(this, ListBoxAddToModelGroup, "NonModelGroup");
    ListBoxAddToModelGroup->SetDropTarget(mdt);

    _dragRowModel = false;
    _dragRowNonModel = false;

    ValidateWindow();
}

ModelGroupPanel::~ModelGroupPanel()
{
	//(*Destroy(ModelGroupPanel)
	//*)
}

void ModelGroupPanel::AddPreviewChoice(const std::string& name)
{
    ChoicePreviews->Append(name);
}

bool canAddToGroup(ModelGroup *g, ModelManager &models, const std::string &model, std::list<std::string> &modelGroupsInGroup, std::list<std::string>& visitedGroups) {

    if (model == g->GetName()) {
        return false;
    }

    for (const auto& it : modelGroupsInGroup) {
        if (it == model) {
            return false;
        }

        Model *m = models[model];
        if (m != nullptr) {
            ModelGroup *grp = dynamic_cast<ModelGroup*>(m);
            if (grp != nullptr) {

                // If we have already visited this group dont look at it again
                for (auto& it3 : visitedGroups) {
                    if (it3 == grp->GetName())
                    {
                        return false;
                    }
                }
                visitedGroups.push_back(grp->GetName());

                for (auto& it2 : grp->ModelNames()) {
                    if (!canAddToGroup(g, models, it2, modelGroupsInGroup, visitedGroups)) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

void ModelGroupPanel::UpdatePanel(const std::string& group)
{
    // static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static wxColour BLUE_ON_DARK(100, 100, 255);

    Choice_DefaultCamera->Clear();
    Choice_DefaultCamera->Append("2D");
    for (size_t i = 0; i < mModels.GetXLightsFrame()->viewpoint_mgr.GetNum3DCameras(); ++i) {
        Choice_DefaultCamera->Append(mModels.GetXLightsFrame()->viewpoint_mgr.GetCamera3D(i)->GetName());
    }

    int spam = ListBoxAddToModelGroup->GetTopItem();
    int spig = ListBoxModelsInGroup->GetTopItem();

    if (spam < 0) spam = 0;
    if (spig < 0) spig = 0;

    if (_lastFirstSelectedModelIndex >= 0) {
        spam = _lastFirstSelectedModelIndex;
    }

    if (_lastFirstSelectedModelInGroupIndex >= 0) {
        spig = _lastFirstSelectedModelInGroupIndex;
    }

    if (group != mGroup) {
        spam = 0;
        spig = 0;
    }

    mModels.ResetModelGroups(); // make sure all our pointers are valid
    mGroup = group;
    LabelModelGroupName->SetLabel(group);
    ListBoxModelsInGroup->Freeze();
    ListBoxModelsInGroup->ClearAll();
    ListBoxModelsInGroup->AppendColumn("Models");

    ListBoxAddToModelGroup->Freeze();
    ListBoxAddToModelGroup->ClearAll();
    ListBoxAddToModelGroup->AppendColumn("Models");

    ChoiceModelLayoutType->SetSelection(1);

    if (group != "")
    {
        ModelGroup* g = (ModelGroup*)mModels[group];
        wxXmlNode* e = g->GetModelXml();
        std::list<std::string> modelsInGroup;
        modelsInGroup.push_back(g->GetName());
        for (const auto& it : g->ModelNames()) {
            long item = ListBoxModelsInGroup->InsertItem(ListBoxModelsInGroup->GetItemCount(), it);
            if (mModels[it] != nullptr) {
                if (mModels[it]->GetDisplayAs() == "ModelGroup") {
                    ListBoxModelsInGroup->SetItemTextColour(item,
                                                            IsDarkMode()
                                                                ? BLUE_ON_DARK : *wxBLUE);
                }
                else if (Contains(it, "/")) {
                    ListBoxModelsInGroup->SetItemTextColour(item, xlORANGE.asWxColor());
                }
            }
            modelsInGroup.push_back(it);
        }

        auto filter = TextCtrl_Filter->GetValue().Lower();
        auto& layoutGroup = layoutPanel->GetCurrentLayoutGroup();

        // dont allow any group that contains this group to be added as that would create a loop
        for (const auto& it : mModels) {
            if (CheckBox_ShowInactiveModels->GetValue() || it.second->IsActive()) {
                if (!CheckBox_ShowOnlyModelsInCurrentView->GetValue() || layoutGroup == "All Models" || it.second->GetLayoutGroup() == layoutGroup) {
                    if (std::find(modelsInGroup.begin(), modelsInGroup.end(), it.first) != modelsInGroup.end() ||
                        (it.second->GetDisplayAs() == "ModelGroup" && (!CheckBox_ShowModelGroups->GetValue() || it.first == group || dynamic_cast<ModelGroup*>(it.second)->ContainsModelGroup(g)))) {
                        // dont add this group
                        // logger_base.debug("Model not eligible to be added to group or already in group " + group + " : " + it.first);
                    }
                    else {
                        if (filter == "" || Contains(::Lower(it.first), filter)) {
                            long item = ListBoxAddToModelGroup->InsertItem(ListBoxAddToModelGroup->GetItemCount(), it.first);
                            if (it.second->GetDisplayAs() == "ModelGroup") {
                                ListBoxAddToModelGroup->SetItemTextColour(item,
                                                                          IsDarkMode()
                                                                              ? BLUE_ON_DARK
                                                                              : *wxBLUE);
                            }
                        }
                    }
                    if (CheckBox_ShowSubmodels->GetValue()) {
                        for (auto& smit : it.second->GetSubModels()) {
                            Model* sm = smit;

                            if (std::find(g->ModelNames().begin(), g->ModelNames().end(), sm->GetFullName()) == g->ModelNames().end()) {
                                if (filter == "" || Contains(::Lower(sm->GetFullName()), filter)) {
                                    long item = ListBoxAddToModelGroup->InsertItem(ListBoxAddToModelGroup->GetItemCount(), sm->GetFullName());
                                    ListBoxAddToModelGroup->SetItemTextColour(item, xlORANGE.asWxColor());
                                }
                            }
                        }
                    }
                }
            }
        }

        auto dc = e->GetAttribute("DefaultCamera", "2D");
        Choice_DefaultCamera->SetStringSelection(dc);
        if (Choice_DefaultCamera->GetStringSelection() != dc) {
            Choice_DefaultCamera->SetStringSelection("2D");
        }

        wxString v = e->GetAttribute("layout", "minimalGrid");
        if (v == "grid") {
            ChoiceModelLayoutType->SetSelection(0);
        }
        else if (v == "minimalGrid") {
            ChoiceModelLayoutType->SetSelection(1);
        }
        else if (v == "horizontal") {
            ChoiceModelLayoutType->SetSelection(2);
        }
        else if (v == "vertical") {
            ChoiceModelLayoutType->SetSelection(3);
        }
        else {
            int idx = ChoiceModelLayoutType->FindString(v);
            if (idx >= 0) {
                ChoiceModelLayoutType->SetSelection(idx);
            }
            else {
                ChoiceModelLayoutType->Append(v);
                ChoiceModelLayoutType->SetSelection(ChoiceModelLayoutType->GetCount() - 1);
            }
        }

        wxString preview = e->GetAttribute("LayoutGroup", "Default");
        ChoicePreviews->SetSelection(0);
        for (size_t i = 0; i < ChoicePreviews->GetCount(); i++) {
            if (ChoicePreviews->GetString(i) == preview)
            {
                ChoicePreviews->SetSelection(i);
            }
        }
        SizeSpinCtrl->SetValue(wxAtoi(e->GetAttribute("GridSize", "400")));
        SpinCtrl_XCentreOffset->SetValue(wxAtoi(e->GetAttribute("XCentreOffset", "0")));
        SpinCtrl_YCentreOffset->SetValue(wxAtoi(e->GetAttribute("YCentreOffset", "0")));
        ColourPickerCtrl_ModelGroupTagColour->SetColour(e->GetAttribute("TagColour", "Black"));
    }

    ResizeColumns();

    ListBoxModelsInGroup->Thaw();
    ListBoxAddToModelGroup->Thaw();

    if (!ListBoxAddToModelGroup->IsEmpty()) {
        if (spam < ListBoxAddToModelGroup->GetItemCount()) {
            ListBoxAddToModelGroup->EnsureVisible(spam);
        } else {
            ListBoxAddToModelGroup->EnsureVisible(0);
        }
    }
    if (!ListBoxModelsInGroup->IsEmpty()) {
        if (spig < ListBoxModelsInGroup->GetItemCount()) {
            ListBoxModelsInGroup->EnsureVisible(spig);
        } else {
            ListBoxModelsInGroup->EnsureVisible(0);
        }
    }

    if (_lastFirstSelectedModelIndex >= ListBoxAddToModelGroup->GetItemCount()) {
        _lastFirstSelectedModelIndex = ListBoxAddToModelGroup->GetItemCount() - 1;
    }
    if (_lastFirstSelectedModelIndex >= 0 && _lastFirstSelectedModelIndex < ListBoxAddToModelGroup->GetItemCount()) {
        ListBoxAddToModelGroup->SetItemState(_lastFirstSelectedModelIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        ListBoxAddToModelGroup->EnsureVisible(_lastFirstSelectedModelIndex);
    }

    if (_lastFirstSelectedModelInGroupIndex >= ListBoxModelsInGroup->GetItemCount()) {
        _lastFirstSelectedModelInGroupIndex = ListBoxModelsInGroup->GetItemCount() - 1;
    }
    if (_lastFirstSelectedModelInGroupIndex >= 0 && _lastFirstSelectedModelInGroupIndex < ListBoxModelsInGroup->GetItemCount()) {
        ListBoxModelsInGroup->SetItemState(_lastFirstSelectedModelInGroupIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        ListBoxModelsInGroup->EnsureVisible(_lastFirstSelectedModelInGroupIndex);
    }

    _lastFirstSelectedModelIndex = -1;
    _lastFirstSelectedModelInGroupIndex = -1;

    ListBoxAddToModelGroup->Refresh();
    ListBoxModelsInGroup->Refresh();

    ValidateWindow();
}

void ModelGroupPanel::ResizeColumns()
{
    int w, h;
    ListBoxModelsInGroup->GetSize(&w, &h);
    ListBoxModelsInGroup->SetColumnWidth(0, wxLIST_AUTOSIZE);
    if (ListBoxModelsInGroup->GetColumnWidth(0) < w)
    {
        ListBoxModelsInGroup->SetColumnWidth(0, w);
    }
    ListBoxAddToModelGroup->GetSize(&w, &h);
    ListBoxAddToModelGroup->SetColumnWidth(0, wxLIST_AUTOSIZE);
    if (ListBoxAddToModelGroup->GetColumnWidth(0) < w)
    {
        ListBoxAddToModelGroup->SetColumnWidth(0, w);
    }
}

void ModelGroupPanel::OnChoiceModelLayoutTypeSelect(wxCommandEvent& event)
{
    SaveGroupChanges();
    ValidateWindow();
}

void ModelGroupPanel::OnButtonAddToModelGroupClick(wxCommandEvent& event)
{
    int first = GetFirstSelectedModel(ListBoxAddToModelGroup);
    _lastFirstSelectedModelIndex = first;

    AddSelectedModels(-1);

    if (first >= ListBoxAddToModelGroup->GetItemCount())
    {
        first = ListBoxAddToModelGroup->GetItemCount() - 1;
    }

    if (first != -1)
    {
        ListBoxAddToModelGroup->SetItemState(first, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }

    ValidateWindow();
}

int ModelGroupPanel::GetModelsVisibleInList(wxListCtrl* list)
{
    if (list->GetItemCount() == 0) return 0;

    wxRect rect;
    list->GetItemRect(0, rect);
    int itemSize = rect.GetHeight();

    return list->GetRect().GetHeight() / itemSize;
}

int ModelGroupPanel::GetFirstSelectedModel(wxListCtrl* list)
{
    for (size_t i = 0; i < list->GetItemCount(); ++i)
    {
        if (IsItemSelected(list, i))
        {
            return i;
        }
    }

    return -1;
}

void ModelGroupPanel::OnButtonRemoveFromModelGroupClick(wxCommandEvent& event)
{
    int first = GetFirstSelectedModel(ListBoxModelsInGroup);

    _lastFirstSelectedModelInGroupIndex = first;

    RemoveSelectedModels();

    if (first >= ListBoxModelsInGroup->GetItemCount())
    {
        first = ListBoxModelsInGroup->GetItemCount() - 1;
    }

    if (first != -1)
    {
        ListBoxModelsInGroup->SetItemState(first, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }

    ValidateWindow();
}

void ModelGroupPanel::OnButtonUpClick(wxCommandEvent& event)
{
    if (GetSelectedModelCount() == 0) return;

    int selected = -1;

    for (size_t i = 0; i < ListBoxModelsInGroup->GetItemCount(); ++i)
    {
        if (selected == -1 && IsItemSelected(ListBoxModelsInGroup, i))
        {
            if (i == 0)
            {
                selected = i;
            }
            else
            {
                selected = i - 1;
            }
            break;
        }
    }
    MoveSelectedModelsTo(selected);
}

void ModelGroupPanel::OnButtonDownClick(wxCommandEvent& event)
{
    if (GetSelectedModelCount() == 0) return;

    int selected = -1;
    int unselected = -1;

    // find the first unselected index after the first selected index
    for (size_t i = 0; i < ListBoxModelsInGroup->GetItemCount(); ++i)
    {
        if (selected == -1 && IsItemSelected(ListBoxModelsInGroup, i))
        {
            selected = i;
        }

        if (selected != -1 && unselected == -1 && !IsItemSelected(ListBoxModelsInGroup, i))
        {
            unselected = i + 1;
            break;
        }
    }

    MoveSelectedModelsTo(unselected);
}

void ModelGroupPanel::SaveGroupChanges()
{
    ModelGroup *g = (ModelGroup*)mModels[mGroup];

    if (g == nullptr) return;

    mModels.GetXLightsFrame()->AbortRender();

    wxXmlNode *e = g->GetModelXml();

    wxString ModelsInGroup = "";
    for (int i = 0; i < ListBoxModelsInGroup->GetItemCount(); i++) {
        if (i < ListBoxModelsInGroup->GetItemCount() - 1) {
            ModelsInGroup += ListBoxModelsInGroup->GetItemText(i, 0) + ",";
        } else {
            ModelsInGroup += ListBoxModelsInGroup->GetItemText(i, 0);
        }
    }

    e->DeleteAttribute("models");
    e->AddAttribute("models", ModelsInGroup);

    e->DeleteAttribute("GridSize");
    e->DeleteAttribute("layout");
    e->AddAttribute("GridSize", wxString::Format("%d", SizeSpinCtrl->GetValue()));
    e->DeleteAttribute("XCentreOffset");
    e->DeleteAttribute("YCentreOffset");
    e->DeleteAttribute("TagColour");
    if (ChoiceModelLayoutType->GetSelection() == 1) {
        e->AddAttribute("XCentreOffset", wxString::Format("%d", SpinCtrl_XCentreOffset->GetValue()));
        e->AddAttribute("YCentreOffset", wxString::Format("%d", SpinCtrl_YCentreOffset->GetValue()));
    }
    e->DeleteAttribute("DefaultCamera");
    e->AddAttribute("DefaultCamera", Choice_DefaultCamera->GetStringSelection());
    switch (ChoiceModelLayoutType->GetSelection()) {
    case 0:
        e->AddAttribute("layout", "grid");
        break;
    case 1:
        e->AddAttribute("layout", "minimalGrid");
        break;
    case 6:
        e->AddAttribute("layout", "horizontal");
        break;
    case 7:
        e->AddAttribute("layout", "vertical");
        break;
    default:
        e->AddAttribute("layout", ChoiceModelLayoutType->GetStringSelection());
        break;
    }
    e->AddAttribute("TagColour", ColourPickerCtrl_ModelGroupTagColour->GetColour().GetAsString());
    g->Reset();
    layoutPanel->ModelGroupUpdated(g, true); // if i dont set this to true then it leaves the house preview with a pointer to an invalid model which crashes
}

void ModelGroupPanel::OnChoicePreviewsSelect(wxCommandEvent& event)
{
    ModelGroup *g = (ModelGroup*)mModels[mGroup];
    std::string layout_group = std::string(ChoicePreviews->GetString(ChoicePreviews->GetCurrentSelection()).mb_str());
    mModels[mGroup]->SetLayoutGroup(layout_group);
    layoutPanel->ModelGroupUpdated(g, true);
}

void ModelGroupPanel::OnSizeSpinCtrlChange(wxSpinEvent& event)
{
    SaveGroupChanges();
}

#pragma region Drag and Drop

wxDragResult MGTextDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
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
        if (_type == "ModelGroup" && _list->GetItemCount() > 0)
        {
            int flags = wxLIST_HITTEST_ONITEM;
            int lastItem = _list->HitTest(wxPoint(x, y), flags);

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
                    _list->EnsureVisible(_list->GetTopItem() - 1);
                    scrollDelay = scrollDelay / 2;
                    if (scrollDelay < MINSCROLLDELAY) scrollDelay = MINSCROLLDELAY;
                }
            }
            else if (y > _list->GetRect().GetHeight() - itemSize)
            {
                // scroll down
                if (lastItem >= 0 && lastItem < _list->GetItemCount())
                {
                    int ev = lastItem + 1;
                    if (ev >= _list->GetItemCount())
                    {
                        ev = _list->GetItemCount() - 1;
                    }
                    if (ev >= 0)
                    {
                        _list->EnsureVisible(ev);
                    }
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

bool MGTextDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
    if (data == "") return false;

    long mousePos = x;
    mousePos = mousePos << 16;
    mousePos += y;
    wxCommandEvent event(EVT_MGDROP);
    event.SetString(data); // this is the dropped string
    event.SetExtraLong(mousePos); // this is the mouse position packed into a long

    wxArrayString parms = wxSplit(data, ',');

    if (parms[0] == "NonModelGroup")
    {
        if (_type == "ModelGroup")
        {
            event.SetInt(0);
            wxPostEvent(_owner, event);
            return true;
        }
    }
    else if (parms[0] == "ModelGroup")
    {
        if (_type == "ModelGroup")
        {
            event.SetInt(1);
            wxPostEvent(_owner, event);
            return true;
        }
        else if (_type == "NonModelGroup")
        {
            event.SetInt(2);
            wxPostEvent(_owner, event);
            return true;
        }
    }

    return false;
}

void ModelGroupPanel::OnDrop(wxCommandEvent& event)
{
    wxArrayString parms = wxSplit(event.GetString(), ',');
    int x = event.GetExtraLong() >> 16;
    int y = event.GetExtraLong() & 0xFFFF;

    switch (event.GetInt())
    {
    case 0:
        // Non model dropped into models (an add)
    {
        int flags = wxLIST_HITTEST_ONITEM;
        long index = ListBoxModelsInGroup->HitTest(wxPoint(x, y), flags);
        AddSelectedModels(index);
    }
    break;
    case 1:
        // Model dropped into models (a reorder)
    {
        int flags = wxLIST_HITTEST_ONITEM;
        long index = ListBoxModelsInGroup->HitTest(wxPoint(x, y), flags);

        ClearSelections(ListBoxModelsInGroup, wxLIST_STATE_DROPHILITED);
        MoveSelectedModelsTo(index);
    }
    break;
    case 2:
        // Model dropped into non model (a remove)
        RemoveSelectedModels();
        break;
    default:
        break;
    }

    ValidateWindow();
}

#pragma endregion Drag and Drop

void ModelGroupPanel::OnListBoxAddToModelGroupBeginDrag(wxListEvent& event)
{
    if (ListBoxAddToModelGroup->GetSelectedItemCount() == 0) return;

    _dragRowModel = false;
    _dragRowNonModel = true;

    wxString drag = "NonModelGroup";
    for (size_t i = 0; i < ListBoxAddToModelGroup->GetItemCount(); ++i)
    {
        if (IsItemSelected(ListBoxAddToModelGroup, i))
        {
            drag += "," + ListBoxAddToModelGroup->GetItemText(i, 0);
        }
    }

    wxTextDataObject my_data(drag);
    MGDropSource dragSource(this, false, true);
    dragSource.SetData(my_data);
    dragSource.DoDragDrop(wxDrag_DefaultMove);
    SetCursor(wxCURSOR_ARROW);

    ValidateWindow();
}

void ModelGroupPanel::OnListBoxAddToModelGroupItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void ModelGroupPanel::OnListBoxModelsInGroupBeginDrag(wxListEvent& event)
{
    if (ListBoxModelsInGroup->GetSelectedItemCount() == 0) return;

    _dragRowModel = true;
    _dragRowNonModel = false;

    wxString drag = "ModelGroup";
    for (size_t i = 0; i < ListBoxModelsInGroup->GetItemCount(); ++i)
    {
        if (IsItemSelected(ListBoxModelsInGroup, i))
        {
            drag += "," + ListBoxModelsInGroup->GetItemText(i, 0);
        }
    }

    wxTextDataObject my_data(drag);
    MGDropSource dragSource(this, true, false);
    dragSource.SetData(my_data);
    dragSource.DoDragDrop(wxDrag_DefaultMove);
    SetCursor(wxCURSOR_ARROW);

    ValidateWindow();
}

void ModelGroupPanel::OnListBoxModelsInGroupItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

bool ModelGroupPanel::IsItemSelected(wxListCtrl* ctrl, int item)
{
    return ctrl->GetItemState(item, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED;
}

void ModelGroupPanel::ValidateWindow()
{
    ModelGroup* mg = static_cast<ModelGroup*>(mModels[mGroup]);
    if (ChoiceModelLayoutType->GetStringSelection() == "Minimal Grid" && mg != nullptr && !mg->IsFromBase())
    {
        SpinCtrl_XCentreOffset->Enable(true);
        SpinCtrl_YCentreOffset->Enable(true);
    }
    else
    {
        SpinCtrl_XCentreOffset->Enable(false);
        SpinCtrl_YCentreOffset->Enable(false);
    }

    SizeSpinCtrl->Enable(mg != nullptr && !mg->IsFromBase());
    ColourPickerCtrl_ModelGroupTagColour->Enable(mg != nullptr && !mg->IsFromBase());
    Choice_DefaultCamera->Enable(mg != nullptr && !mg->IsFromBase());
    ChoiceModelLayoutType->Enable(mg != nullptr && !mg->IsFromBase());
    ChoicePreviews->Enable(mg != nullptr && !mg->IsFromBase());

    bool moveable = true;

    // this is incomplete but its a start
    if (mg != nullptr && mg->IsFromBase() && ListBoxModelsInGroup->GetSelectedItemCount() == 1)
    {
        for (int i = 0; i < ListBoxModelsInGroup->GetItemCount(); ++i) {
            if (IsItemSelected(ListBoxModelsInGroup, i)) {
                std::string modelName = ListBoxModelsInGroup->GetItemText(i, 0).ToStdString();
                if (mg->IsModelFromBase(modelName)) {
                    moveable = false;
                }
            }
        }
    }

    if (ListBoxAddToModelGroup->GetSelectedItemCount() == 0)
    {
        ButtonAddModel->Enable(false);
    }
    else
    {
        ButtonAddModel->Enable(true);
    }

    if (ListBoxModelsInGroup->GetSelectedItemCount() == 0)
    {
        ButtonRemoveModel->Enable(false);
    }
    else
    {
        ButtonRemoveModel->Enable(moveable);
    }

    if (GetSelectedModelCount() > 0 && GetSelectedModelCount() < ListBoxModelsInGroup->GetItemCount())
    {
        ButtonMoveUp->Enable(moveable);
        ButtonMoveDown->Enable(moveable);
    }
    else
    {
        ButtonMoveUp->Enable(false);
        ButtonMoveDown->Enable(false);
    }
}

int ModelGroupPanel::GetSelectedModelCount()
{
    int count = 0;

    for (int i = 0; i < ListBoxModelsInGroup->GetItemCount(); ++i)
    {
        if (IsItemSelected(ListBoxModelsInGroup, i))
        {
            count++;
        }
    }

    return count;
}

void ModelGroupPanel::AddSelectedModels(int index)
{
    ListBoxModelsInGroup->Freeze();
    ListBoxAddToModelGroup->Freeze();
    ClearSelections(ListBoxModelsInGroup, wxLIST_STATE_SELECTED | wxLIST_STATE_DROPHILITED);
    if (index == -1) index = ListBoxModelsInGroup->GetItemCount();
    int added = 0;
    for (size_t i = 0; i < ListBoxAddToModelGroup->GetItemCount(); ++i)
    {
        if (IsItemSelected(ListBoxAddToModelGroup, i))
        {
            std::string modelName = ListBoxAddToModelGroup->GetItemText(i, 0).ToStdString();
            int idx = ListBoxModelsInGroup->InsertItem(index + added, modelName);
            ListBoxModelsInGroup->SetItemState(idx, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            ListBoxAddToModelGroup->DeleteItem(i);
            Model* model = mModels[modelName];
            if (model != nullptr) {
                model->GroupSelected = true;
                model->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ModelGroupPanel::AddSelectedModels");
                model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ModelGroupPanel::AddSelectedModels");
            }
            i--;
            added++;
        }
    }
    SaveGroupChanges();

    ResizeColumns();

    ListBoxModelsInGroup->Thaw();
    ListBoxModelsInGroup->Refresh();
    ListBoxAddToModelGroup->Thaw();
    ListBoxAddToModelGroup->Refresh();
}

void ModelGroupPanel::RemoveSelectedModels()
{
    ModelGroup* mg = static_cast<ModelGroup*>(mModels[mGroup]);

    ListBoxModelsInGroup->Freeze();
    ListBoxAddToModelGroup->Freeze();

    ClearSelections(ListBoxAddToModelGroup, wxLIST_STATE_SELECTED | wxLIST_STATE_DROPHILITED);
    for (size_t i = 0; i < ListBoxModelsInGroup->GetItemCount(); ++i)
    {
        if (IsItemSelected(ListBoxModelsInGroup, i))
        {
            std::string modelName = ListBoxModelsInGroup->GetItemText(i, 0).ToStdString();
            Model* model = mModels[modelName];

            if (mg != nullptr && mg->IsFromBase() && mg->IsModelFromBase(modelName))
            {
                // we wont remove these models as they came from the base show folder
            } else {
                if (model != nullptr) {
                    if ((model->GetDisplayAs() == "ModelGroup" && !CheckBox_ShowModelGroups->GetValue()) ||
                        (model->GetDisplayAs() == "SubModel" && !CheckBox_ShowSubmodels->GetValue())) {
                        // these should not be moved
                    } else {
                        int idx = ListBoxAddToModelGroup->InsertItem(0, modelName);
                        ListBoxAddToModelGroup->SetItemState(idx, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                    }
                    model->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ModelGroupPanel::RemoveSelectedModels");
                    model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ModelGroupPanel::RemoveSelectedModels");
                    model->GroupSelected = false;
                }
                ListBoxModelsInGroup->DeleteItem(i);
                i--;
            }
        }
    }
    SaveGroupChanges();

    ResizeColumns();

    ListBoxModelsInGroup->Thaw();
    ListBoxModelsInGroup->Refresh();
    ListBoxAddToModelGroup->Thaw();
    ListBoxAddToModelGroup->Refresh();
}

void ModelGroupPanel::MoveSelectedModelsTo(int indexTo)
{
    ListBoxModelsInGroup->Freeze();
    ListBoxAddToModelGroup->Freeze();

    std::list<std::string> moved;

    int adj = 0;
    for (int i = 0; i < ListBoxModelsInGroup->GetItemCount(); ++i)
    {
        if (IsItemSelected(ListBoxModelsInGroup, i))
        {
            std::string modelName = ListBoxModelsInGroup->GetItemText(i, 0).ToStdString();
            moved.push_back(modelName);
            ListBoxModelsInGroup->SetItemState(i, 0, wxLIST_STATE_SELECTED);
            ListBoxModelsInGroup->DeleteItem(i);
            if (i < indexTo)
            {
                adj--;
            }
            i--;
        }
    }

    int added = 0;
    for (auto it = moved.begin(); it != moved.end(); ++it)
    {
        if (indexTo == -1)
        {
            int idx = ListBoxModelsInGroup->InsertItem(ListBoxModelsInGroup->GetItemCount(), *it);
            ListBoxModelsInGroup->SetItemState(idx, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        }
        else
        {
            int to = indexTo + added + adj;
            if (to < 0) to = 0;
            int idx = ListBoxModelsInGroup->InsertItem(to, *it);
            ListBoxModelsInGroup->SetItemState(idx, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        }
        added++;
    }

    SaveGroupChanges();

    ListBoxModelsInGroup->Thaw();
    ListBoxModelsInGroup->Refresh();
    ListBoxAddToModelGroup->Thaw();
    ListBoxAddToModelGroup->Refresh();
}

void ModelGroupPanel::ClearSelections(wxListCtrl* listCtrl, long stateMask)
{
    for (int i = 0; i < listCtrl->GetItemCount(); ++i)
    {
        listCtrl->SetItemState(i, 0, stateMask);
    }
}

void ModelGroupPanel::OnCheckBox_ShowSubmodelsClick(wxCommandEvent& event)
{
    UpdatePanel(mGroup);
}

void ModelGroupPanel::OnListBoxAddToModelGroupItemActivated(wxListEvent& event)
{
    wxCommandEvent e;
    OnButtonAddToModelGroupClick(e);
}

void ModelGroupPanel::OnListBoxModelsInGroupItemActivated(wxListEvent& event)
{
    wxCommandEvent e;
    OnButtonRemoveFromModelGroupClick(e);
}

void ModelGroupPanel::OnCheckBox_ShowModelGroupsClick(wxCommandEvent& event)
{
    UpdatePanel(mGroup);
}

void ModelGroupPanel::OnListBoxModelsInGroupItemDeselect(wxListEvent& event)
{
    ValidateWindow();
}

void ModelGroupPanel::OnListBoxAddToModelGroupItemDeselect(wxListEvent& event)
{
    ValidateWindow();
}

void ModelGroupPanel::OnSpinCtrl_XCentreOffsetChange(wxSpinEvent& event)
{
    SaveGroupChanges();
}

void ModelGroupPanel::OnSpinCtrl_YCentreOffsetChange(wxSpinEvent& event)
{
    SaveGroupChanges();
}

void ModelGroupPanel::OnListBoxModelsInGroupItemRClick(wxListEvent& event)
{
    wxMenu mnu;
    mnu.Append(ID_MNU_COPY, "Copy From...");
    if(ListBoxModelsInGroup->GetItemCount() != 0) {
        mnu.AppendSeparator();
        mnu.Append(ID_MNU_SORTBYNAME, "Sort By Name");
        mnu.AppendSeparator();
        mnu.Append(ID_MNU_CLEARALL, "Clear");
    }

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&ModelGroupPanel::OnPopup, nullptr, this);
    PopupMenu(&mnu);
}

void ModelGroupPanel::OnPopup(wxCommandEvent& event)
{
    int id = event.GetId();
    if (id == ID_MNU_CLEARALL) {
        if (wxMessageBox("Remove All Models From Group?", "Clear Group", wxYES_NO, this) == wxYES) {
            for (int i = ListBoxModelsInGroup->GetItemCount(); i >= 0; --i) {
                ListBoxModelsInGroup->SetItemState(i, 0, wxLIST_STATE_SELECTED);
                ListBoxModelsInGroup->DeleteItem(i);
            }
            SaveGroupChanges();
            UpdatePanel(mGroup);
        }
    }
    else if (id == ID_MNU_COPY) {
        CopyModelList();
    }
    else if (id == ID_MNU_SORTBYNAME) {
        SortModelsByName();
    }
}

void ModelGroupPanel::CopyModelList()
{
    wxArrayString choices = getGroupList();
    wxSingleChoiceDialog dlg(GetParent(), "", "Select Group", choices);
    if (dlg.ShowModal() == wxID_OK)
    {
        ModelGroup* cg = (ModelGroup*)mModels[dlg.GetStringSelection()];
        if (cg == nullptr) return;
        wxString const models = cg->GetModelXml()->GetAttribute("models");
        ClearSelections(ListBoxModelsInGroup, wxLIST_STATE_SELECTED | wxLIST_STATE_DROPHILITED);
        int index = ListBoxModelsInGroup->GetItemCount();
        ModelGroup* g = (ModelGroup*)mModels[mGroup];
        wxArrayString const modelArray = wxSplit(models, ',');
        for (size_t i = 0; i < modelArray.size(); ++i) {
            wxString const modelName = modelArray[i];
            if (std::find(g->ModelNames().begin(), g->ModelNames().end(), modelName) != g->ModelNames().end())
                continue;
            ListBoxModelsInGroup->InsertItem(index, modelName);
            index++;
        }
        SaveGroupChanges();
        UpdatePanel(mGroup);
    }
}

void ModelGroupPanel::SortModelsByName()
{
    ModelGroup* g = (ModelGroup*)mModels[mGroup];
    if (g == nullptr) return;
    wxArrayString models;
    for (int i = ListBoxModelsInGroup->GetItemCount(); i >= 0; --i) {
        wxString const modelName = ListBoxModelsInGroup->GetItemText(i, 0);
        models.push_back(modelName);
        ListBoxModelsInGroup->SetItemState(i, 0, wxLIST_STATE_SELECTED);
        ListBoxModelsInGroup->DeleteItem(i);
    }
    models.Sort(wxStringNumberAwareStringCompare);
    for (int i = 0; i < models.size(); ++i) {
        ListBoxModelsInGroup->InsertItem(i, models[i]);
    }
    SaveGroupChanges();
    UpdatePanel(mGroup);
}

wxArrayString ModelGroupPanel::getGroupList()
{
    wxArrayString choices;
    for (auto it = mModels.begin(); it != mModels.end(); ++it) {
        ModelGroup* g = (ModelGroup*)it->second;
        if (g == nullptr) continue;
        if (g->GetDisplayAs() != "ModelGroup") continue;
        if (g->Name() == mGroup)//Skip Current Group
            continue;
        choices.Add(g->Name());
    }
    return choices;
}

void ModelGroupPanel::OnButtonClearFilterClick(wxCommandEvent& event)
{
    TextCtrl_Filter->SetValue("");
    UpdatePanel(mGroup);
}

void ModelGroupPanel::OnTextCtrl_FilterText(wxCommandEvent& event)
{
    UpdatePanel(mGroup);
}

void ModelGroupPanel::OnCheckBox_ShowInactiveModelsClick(wxCommandEvent& event)
{
    UpdatePanel(mGroup);
}

void ModelGroupPanel::OnCheckBox_ShowOnlyModelsInCurrentViewClick(wxCommandEvent& event)
{
    UpdatePanel(mGroup);
}

void ModelGroupPanel::OnColourPickerCtrl_ModelGroupTagColourColourChanged(wxColourPickerEvent& event)
{
    SaveGroupChanges();
}

void ModelGroupPanel::OnChoice_DefaultCameraSelect(wxCommandEvent& event)
{
    SaveGroupChanges();
}

void ModelGroupPanel::OnButtonAliasesClick(wxCommandEvent& event)
{
    ModelGroup* g = (ModelGroup*)mModels[mGroup];
    if (g == nullptr)
        return;
    EditAliasesDialog dlg(this, g);

    dlg.ShowModal();
}
