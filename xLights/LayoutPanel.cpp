/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*InternalHeaders(LayoutPanel)
 #include <wx/button.h>
 #include <wx/checkbox.h>
 #include <wx/choice.h>
 #include <wx/font.h>
 #include <wx/intl.h>
 #include <wx/notebook.h>
 #include <wx/scrolbar.h>
 #include <wx/settings.h>
 #include <wx/sizer.h>
 #include <wx/splitter.h>
 #include <wx/stattext.h>
 #include <wx/string.h>
 //*)

#include <wx/clipbrd.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/tglbtn.h>
#include <wx/sstream.h>
#include <wx/artprov.h>
#include <wx/dataview.h>
#include <wx/config.h>
#include <wx/treebase.h>
#include <wx/colordlg.h>
#include <wx/numdlg.h>

#include "LayoutPanel.h"
#include "ModelPreview.h"
#include "xLightsMain.h"
#include "ChannelLayoutDialog.h"
#include "ControllerConnectionDialog.h"
#include "ModelGroupPanel.h"
#include "ViewObjectPanel.h"
#include "LayoutGroup.h"
#include "models/ModelImages.h"
#include "models/SubModel.h"
#include "models/PolyLineModel.h"
#include "models/ModelGroup.h"
#include "models/ViewObject.h"
#include "models/RulerObject.h"
#include "models/CustomModel.h"
#include "WiringDialog.h"
#include "ModelDimmingCurveDialog.h"
#include "UtilFunctions.h"
#include "ExternalHooks.h"
#include "ColorManager.h"
#include "support/VectorMath.h"
#include "KeyBindings.h"
#include "sequencer/MainSequencer.h"
#include "ImportPreviewsModelsDialog.h"
#include "ViewsModelsPanel.h"
#include "outputs/OutputManager.h"
#include "outputs/Output.h"
#include "cad/ModelToCAD.h"
#include "LORPreview.h"
#include "ExternalHooks.h"
#include "ModelFaceDialog.h"
#include "ModelStateDialog.h"
#include "CustomModelDialog.h"
#include "SubModelsDialog.h"

#include "LayoutUtils.h"

#include <log4cpp/Category.hh>

#include <set>

#define MODELCOLNAME "Model/Group"
#define STARTCHANCOLNAME "Start Chan"
#define ENDCHANCOLNAME "End Chan"
#define CONTCONNCOLNAME "Ctrlr Conn"

static wxRect scaledRect(int srcWidth, int srcHeight, int dstWidth, int dstHeight)
{
	wxRect r;
	float srcAspectRatio = float(srcWidth) / srcHeight;
	float dstAspectRatio = float(dstWidth) / dstHeight;

	if (srcAspectRatio > dstAspectRatio)
	{
		r.SetWidth(dstWidth);
		r.SetHeight(int(dstWidth / srcAspectRatio));
		r.SetTopLeft(wxPoint(0, (dstHeight - r.GetHeight()) / 2));
	}
	else
	{
		r.SetHeight(dstHeight);
		r.SetWidth(int(dstHeight * srcAspectRatio));
		r.SetTopLeft(wxPoint((dstWidth - r.GetWidth()) / 2, 0));
	}
	return r;
}

//(*IdInit(LayoutPanel)
const long LayoutPanel::ID_PANEL4 = wxNewId();
const long LayoutPanel::ID_PANEL_Objects = wxNewId();
const long LayoutPanel::ID_NOTEBOOK_OBJECTS = wxNewId();
const long LayoutPanel::ID_PANEL3 = wxNewId();
const long LayoutPanel::ID_PANEL2 = wxNewId();
const long LayoutPanel::ID_SPLITTERWINDOW1 = wxNewId();
const long LayoutPanel::ID_CHECKBOX_3D = wxNewId();
const long LayoutPanel::ID_CHECKBOXOVERLAP = wxNewId();
const long LayoutPanel::ID_BUTTON_SAVE_PREVIEW = wxNewId();
const long LayoutPanel::ID_PANEL5 = wxNewId();
const long LayoutPanel::ID_STATICTEXT1 = wxNewId();
const long LayoutPanel::ID_CHOICE_PREVIEWS = wxNewId();
const long LayoutPanel::ID_SCROLLBAR1 = wxNewId();
const long LayoutPanel::ID_SCROLLBAR2 = wxNewId();
const long LayoutPanel::ID_PANEL1 = wxNewId();
const long LayoutPanel::ID_SPLITTERWINDOW2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(LayoutPanel,wxPanel)
	//(*EventTable(LayoutPanel)
	//*)
    EVT_TREELIST_SELECTION_CHANGED(wxID_ANY, LayoutPanel::OnSelectionChanged)
    EVT_TREELIST_ITEM_CONTEXT_MENU(wxID_ANY, LayoutPanel::OnItemContextMenu)
END_EVENT_TABLE()

const long LayoutPanel::ID_TREELISTVIEW_MODELS = wxNewId();
const long LayoutPanel::ID_PREVIEW_REPLACEMODEL = wxNewId();
const long LayoutPanel::ID_PREVIEW_RESET = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN = wxNewId();
const long LayoutPanel::ID_PREVIEW_RESIZE = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_NODELAYOUT = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_LOCK = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_UNLOCK = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_EXPORTASCUSTOM = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_EXPORTASCUSTOM3D = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_CREATEGROUP = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_WIRINGVIEW = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_ASPECTRATIO = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL = wxNewId();
const long LayoutPanel::ID_PREVIEW_RESIZE_SAMEWIDTH = wxNewId();
const long LayoutPanel::ID_PREVIEW_RESIZE_SAMEHEIGHT = wxNewId();
const long LayoutPanel::ID_PREVIEW_RESIZE_SAMESIZE = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTION = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERNAME = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_SETACTIVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_SETINACTIVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_SMARTREMOTE = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_TAGCOLOUR = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_PIXELSIZE = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_PIXELSTYLE = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_TRANSPARENCY = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_BLACKTRANSPARENCY = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERGAMMA = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERCOLOURORDER = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERBRIGHTNESS = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERSTARTNULLNODES = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERENDNULLNODES = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERDIRECTION = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERGROUPCOUNT = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERPROTOCOL = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTIONINCREMENT = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_SMARTREMOTETYPE = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_PREVIEW = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_DIMMINGCURVES = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_TOP = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_GROUND = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_BOTTOM = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_LEFT = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_RIGHT = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_H_CENTER = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_V_CENTER = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_FRONT = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_BACK = wxNewId();
const long LayoutPanel::ID_PREVIEW_DISTRIBUTE = wxNewId();
const long LayoutPanel::ID_PREVIEW_H_DISTRIBUTE = wxNewId();
const long LayoutPanel::ID_PREVIEW_V_DISTRIBUTE = wxNewId();
const long LayoutPanel::ID_MNU_REMOVE_MODEL_FROM_GROUP = wxNewId();
const long LayoutPanel::ID_MNU_DELETE_MODEL = wxNewId();
const long LayoutPanel::ID_MNU_DELETE_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_MNU_DELETE_EMPTY_MODEL_GROUPS = wxNewId();
const long LayoutPanel::ID_MNU_RENAME_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_MNU_CLONE_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_MNU_MAKESCVALID = wxNewId();
const long LayoutPanel::ID_MNU_MAKEALLSCVALID = wxNewId();
const long LayoutPanel::ID_MNU_MAKEALLSCNOTOVERLAPPING = wxNewId();
const long LayoutPanel::ID_MNU_ADD_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_MNU_ADD_TO_EXISTING_GROUPS = wxNewId();
const long LayoutPanel::ID_PREVIEW_DELETE_ACTIVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_RENAME_ACTIVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_ADDPOINT = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_DELETEPOINT = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_ADDCURVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_DELCURVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_SAVE_LAYOUT_IMAGE = wxNewId();
const long LayoutPanel::ID_PREVIEW_PRINT_LAYOUT_IMAGE = wxNewId();
const long LayoutPanel::ID_PREVIEW_SAVE_VIEWPOINT = wxNewId();
const long LayoutPanel::ID_PREVIEW_VIEWPOINT_DEFAULT = wxNewId();
const long LayoutPanel::ID_PREVIEW_VIEWPOINT_DEFAULT_RESTORE = wxNewId();
const long LayoutPanel::ID_PREVIEW_VIEWPOINT2D = wxNewId();
const long LayoutPanel::ID_PREVIEW_VIEWPOINT3D = wxNewId();
const long LayoutPanel::ID_PREVIEW_DELETEVIEWPOINT2D = wxNewId();
const long LayoutPanel::ID_PREVIEW_DELETEVIEWPOINT3D = wxNewId();
const long LayoutPanel::ID_PREVIEW_IMPORTMODELSFROMRGBEFFECTS = wxNewId();
const long LayoutPanel::ID_PREVIEW_IMPORT_MODELS_FROM_LORS5 = wxNewId();
const long LayoutPanel::ID_ADD_OBJECT_IMAGE = wxNewId();
const long LayoutPanel::ID_ADD_OBJECT_GRIDLINES = wxNewId();
const long LayoutPanel::ID_ADD_OBJECT_TERRIAN = wxNewId();
const long LayoutPanel::ID_ADD_OBJECT_RULER = wxNewId();
const long LayoutPanel::ID_ADD_OBJECT_MESH = wxNewId();
const long LayoutPanel::ID_ADD_DMX_MOVING_HEAD = wxNewId();
const long LayoutPanel::ID_ADD_DMX_MOVING_HEAD_3D = wxNewId();
const long LayoutPanel::ID_ADD_DMX_GENERAL = wxNewId();
const long LayoutPanel::ID_ADD_DMX_SKULL = wxNewId();
const long LayoutPanel::ID_ADD_DMX_SERVO = wxNewId();
const long LayoutPanel::ID_ADD_DMX_SERVO_3D = wxNewId();
const long LayoutPanel::ID_ADD_DMX_FLOODLIGHT = wxNewId();
const long LayoutPanel::ID_ADD_DMX_FLOODAREA = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_CAD_EXPORT = wxNewId();
const long LayoutPanel::ID_PREVIEW_LAYOUT_DXF_EXPORT = wxNewId();
const long LayoutPanel::ID_PREVIEW_FLIP_HORIZONTAL = wxNewId();
const long LayoutPanel::ID_PREVIEW_FLIP_VERTICAL = wxNewId();

#define CHNUMWIDTH "10000000000000"

// OnSelectionChanged() doesn't fire on MSW or GTK when Select is called
// se we mimic it by calling HandleSelectionChanged as needed
#if defined(__WXMSW__) || defined(__LINUX__)
#define PlatformHandleSelectionChanged() HandleSelectionChanged()
#else
#define PlatformHandleSelectionChanged()
#endif

class ModelTreeData : public wxTreeItemData {
public:
    ModelTreeData(Model *m, int NativeOrder, bool fullname) :wxTreeItemData(), model(m), fullname(fullname) {
        wxASSERT(m != nullptr);
        //a SetFromXML call on the parent (example: recalc start channels) will cause
        //submodel pointers to be deleted.  Need to not save them, but instead, use the parent
        //and query by name
        nativeOrder = NativeOrder;
        SubModel *s = dynamic_cast<SubModel*>(m);
        if (s != nullptr) {
            model = s->GetParent();
            subModel = s->GetName();
        }
        startingChannel = m->GetNumberFromChannelString(m->ModelStartChannel);
        endingChannel = m->GetLastChannel();
    };
    virtual ~ModelTreeData() {};

    Model *GetModel() const
    {
        if ("" != subModel) {
            return model->GetSubModel(subModel);
        }
        return model;
    }

    int startingChannel;
    int endingChannel;
    int nativeOrder;
    bool fullname;
private:
    Model *model;
    std::string subModel;
};

class NewModelBitmapButton : public wxBitmapButton
{
public:

    NewModelBitmapButton(wxWindow *parent, const wxBitmapBundle &bmp, const wxBitmapBundle& bmpDis, const wxBitmapBundle& pBmp, const std::string &type)
        : wxBitmapButton(parent, wxID_ANY, bmp), bitmap(bmp), bitmapDisabled(bmpDis), pressedBitmap(pBmp), modelType(type) {
        SetToolTip("Create new " + type);
    }
    virtual ~NewModelBitmapButton() {}

    void SetState(unsigned int s) {
        if (s > 2) {
            s = 0;
        }
        state = s;
        if (state == 2) {
            SetBitmap(bitmapDisabled);
        } else if (state == 1) {
            SetBitmap(pressedBitmap);
        } else {
            SetBitmap(bitmap);
        }
    }
    unsigned int GetState() const
    {
        return state;
    }
    const std::string &GetModelType() const
    {
        return modelType;
    }
protected:
private:
    const std::string modelType;
    uint32_t state = 0;
    wxBitmapBundle bitmap;
    wxBitmapBundle bitmapDisabled;
    wxBitmapBundle pressedBitmap;
};

LayoutPanel::LayoutPanel(wxWindow* parent, xLightsFrame *xl, wxPanel* sequencer) : xlights(xl), main_sequencer(sequencer)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	//(*Initialize(LayoutPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizerPreview;
	wxFlexGridSizer* LayoutGLSizer;
	wxFlexGridSizer* LeftPanelSizer;
	wxFlexGridSizer* PreviewGLSizer;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizerPreview = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizerPreview->AddGrowableCol(0);
	FlexGridSizerPreview->AddGrowableRow(0);
	SplitterWindow2 = new wxSplitterWindow(this, ID_SPLITTERWINDOW2, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW2"));
	SplitterWindow2->SetMinimumPaneSize(10);
	SplitterWindow2->SetSashGravity(0.5);
	LeftPanel = new wxPanel(SplitterWindow2, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
	LeftPanelSizer = new wxFlexGridSizer(0, 1, 0, 0);
	LeftPanelSizer->AddGrowableCol(0);
	LeftPanelSizer->AddGrowableRow(0);
	ModelSplitter = new wxSplitterWindow(LeftPanel, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	ModelSplitter->SetMinimumPaneSize(100);
	ModelSplitter->SetSashGravity(0.5);
	FirstPanel = new wxPanel(ModelSplitter, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(0);
	Notebook_Objects = new wxNotebook(FirstPanel, ID_NOTEBOOK_OBJECTS, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_OBJECTS"));
	PanelModels = new wxPanel(Notebook_Objects, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
	PanelObjects = new wxPanel(Notebook_Objects, ID_PANEL_Objects, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Objects"));
	Notebook_Objects->AddPage(PanelModels, _("Models"), false);
	Notebook_Objects->AddPage(PanelObjects, _("3D Objects"), false);
	FlexGridSizer4->Add(Notebook_Objects, 1, wxALL|wxEXPAND, 1);
	FirstPanel->SetSizer(FlexGridSizer4);
	SecondPanel = new wxPanel(ModelSplitter, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	ModelSplitter->SplitHorizontally(FirstPanel, SecondPanel);
	LeftPanelSizer->Add(ModelSplitter, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	CheckBox_3D = new wxCheckBox(LeftPanel, ID_CHECKBOX_3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_3D"));
	CheckBox_3D->SetValue(false);
	FlexGridSizer3->Add(CheckBox_3D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBoxOverlap = new wxCheckBox(LeftPanel, ID_CHECKBOXOVERLAP, _("Overlap checks enabled"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOXOVERLAP"));
	CheckBoxOverlap->SetValue(false);
	FlexGridSizer3->Add(CheckBoxOverlap, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	ButtonSavePreview = new wxButton(LeftPanel, ID_BUTTON_SAVE_PREVIEW, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_PREVIEW"));
	FlexGridSizer2->Add(ButtonSavePreview, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	LeftPanelSizer->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	LeftPanel->SetSizer(LeftPanelSizer);
	PreviewGLPanel = new wxPanel(SplitterWindow2, ID_PANEL1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_PANEL1"));
	PreviewGLSizer = new wxFlexGridSizer(2, 1, 0, 0);
	PreviewGLSizer->AddGrowableCol(0);
	PreviewGLSizer->AddGrowableRow(1);
	FlexGridSizer1 = new wxFlexGridSizer(0, 6, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	ToolSizer = new wxFlexGridSizer(0, 10, 0, 0);
	FlexGridSizer1->Add(ToolSizer, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 3);
	StaticText1 = new wxStaticText(PreviewGLPanel, ID_STATICTEXT1, _("Preview:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	wxFont StaticText1Font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
	if ( !StaticText1Font.Ok() ) StaticText1Font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
	StaticText1Font.SetWeight(wxFONTWEIGHT_BOLD);
	StaticText1->SetFont(StaticText1Font);
	FlexGridSizer1->Add(StaticText1, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 40);
	ChoiceLayoutGroups = new wxChoice(PreviewGLPanel, ID_CHOICE_PREVIEWS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PREVIEWS"));
	FlexGridSizer1->Add(ChoiceLayoutGroups, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	PreviewGLSizer->Add(FlexGridSizer1, 1, wxALL|wxALIGN_LEFT, 3);
	LayoutGLSizer = new wxFlexGridSizer(0, 2, 0, 0);
	LayoutGLSizer->AddGrowableCol(0);
	LayoutGLSizer->AddGrowableRow(0);
	ScrollBarLayoutVert = new wxScrollBar(PreviewGLPanel, ID_SCROLLBAR1, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL, wxDefaultValidator, _T("ID_SCROLLBAR1"));
	ScrollBarLayoutVert->SetScrollbar(0, 1, 10, 1);
	LayoutGLSizer->Add(ScrollBarLayoutVert, 1, wxALL|wxEXPAND, 0);
	ScrollBarLayoutHorz = new wxScrollBar(PreviewGLPanel, ID_SCROLLBAR2, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL, wxDefaultValidator, _T("ID_SCROLLBAR2"));
	ScrollBarLayoutHorz->SetScrollbar(0, 1, 10, 1);
	LayoutGLSizer->Add(ScrollBarLayoutHorz, 1, wxALL|wxEXPAND, 0);
	LayoutGLSizer->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	PreviewGLSizer->Add(LayoutGLSizer, 1, wxALL|wxEXPAND, 0);
	PreviewGLPanel->SetSizer(PreviewGLSizer);
	SplitterWindow2->SplitVertically(LeftPanel, PreviewGLPanel);
	FlexGridSizerPreview->Add(SplitterWindow2, 1, wxALL|wxEXPAND, 1);
	SetSizer(FlexGridSizerPreview);

	Connect(ID_NOTEBOOK_OBJECTS,wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&LayoutPanel::OnNotebook_ObjectsPageChanged);
	Connect(ID_SPLITTERWINDOW1,wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED,(wxObjectEventFunction)&LayoutPanel::OnModelSplitterSashPosChanged);
	Connect(ID_CHECKBOX_3D,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&LayoutPanel::OnCheckBox_3DClick);
	Connect(ID_CHECKBOXOVERLAP,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&LayoutPanel::OnCheckBoxOverlapClick);
	Connect(ID_BUTTON_SAVE_PREVIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LayoutPanel::OnButtonSavePreviewClick);
	Connect(ID_CHOICE_PREVIEWS,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&LayoutPanel::OnChoiceLayoutGroupsSelect);
	Connect(ID_SPLITTERWINDOW2,wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED,(wxObjectEventFunction)&LayoutPanel::OnSplitterWindowSashPosChanged);
	//*)

    ScrollBarLayoutHorz->Hide();
    ScrollBarLayoutVert->Hide();

    logger_base.debug("LayoutPanel basic setup complete");
    modelPreview = new ModelPreview( (wxPanel*) PreviewGLPanel, xlights, true, 0, false, true);
    LayoutGLSizer->Insert(0, modelPreview, 1, wxALL | wxEXPAND, 0);
    PreviewGLSizer->Fit(PreviewGLPanel);
    PreviewGLSizer->SetSizeHints(PreviewGLPanel);
    FlexGridSizerPreview->Fit(this);
    FlexGridSizerPreview->SetSizeHints(this);
    logger_base.debug("LayoutPanel ModelPreview created");

    modelPreview->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&LayoutPanel::OnPreviewLeftDown, nullptr,this);
    modelPreview->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&LayoutPanel::OnPreviewLeftUp, nullptr,this);
    modelPreview->Connect(wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&LayoutPanel::OnPreviewRightDown, nullptr,this);
    modelPreview->Connect(wxEVT_MOTION,(wxObjectEventFunction)&LayoutPanel::OnPreviewMouseMove, nullptr,this);
    modelPreview->Connect(wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&LayoutPanel::OnPreviewMouseLeave, nullptr, this);
    modelPreview->Connect(wxEVT_LEFT_DCLICK, (wxObjectEventFunction)&LayoutPanel::OnPreviewLeftDClick, nullptr, this);
    modelPreview->Connect(wxEVT_MOUSEWHEEL, (wxObjectEventFunction)&LayoutPanel::OnPreviewMouseWheel, nullptr, this);
    modelPreview->Connect(wxEVT_MIDDLE_DOWN, (wxObjectEventFunction)&LayoutPanel::OnPreviewMouseWheelDown, nullptr, this);
    modelPreview->Connect(wxEVT_MIDDLE_UP, (wxObjectEventFunction)&LayoutPanel::OnPreviewMouseWheelUp, nullptr, this);
    modelPreview->Connect(wxEVT_MAGNIFY, (wxObjectEventFunction)&LayoutPanel::OnPreviewMagnify, nullptr, this);

    if (modelPreview->EnableTouchEvents(wxTOUCH_ROTATE_GESTURE | wxTOUCH_ZOOM_GESTURE)) {
        modelPreview->Connect(wxEVT_GESTURE_ROTATE, (wxObjectEventFunction)&LayoutPanel::OnPreviewRotateGesture, nullptr, this);
        modelPreview->Connect(wxEVT_GESTURE_ZOOM, (wxObjectEventFunction)&LayoutPanel::OnPreviewZoomGesture, nullptr, this);
    }
    modelPreview->Connect(EVT_MOTION3D, (wxObjectEventFunction)&LayoutPanel::OnPreviewMotion3D, nullptr, this);
    modelPreview->Connect(EVT_MOTION3D_BUTTONCLICKED, (wxObjectEventFunction)&LayoutPanel::OnPreviewMotion3DButtonEvent, nullptr, this);

    propertyEditor = new wxPropertyGrid(ModelSplitter,
                                        wxID_ANY, // id
                                        wxDefaultPosition, // position
                                        wxDefaultSize, // size
                                        // Here are just some of the supported window styles
                                        //wxPG_AUTO_SORT | // Automatic sorting after items added
                                        wxPG_SPLITTER_AUTO_CENTER | // Automatically center splitter until user manually adjusts it
                                        // Default style
                                        wxPG_DEFAULT_STYLE);
    propertyEditor->SetExtraStyle(wxWS_EX_PROCESS_IDLE | wxPG_EX_HELP_AS_TOOLTIPS);
    LayoutUtils::CreateImageList(m_imageList);

    wxFlexGridSizer* FlexGridSizerModels = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizerModels->AddGrowableCol(0);
	FlexGridSizerModels->AddGrowableRow(0);
	wxPanel* new_panel = new wxPanel(PanelModels, wxNewId(), wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_OBJECTS_PANEL"));
	FlexGridSizerModels->Add(new_panel, 1, wxALL|wxEXPAND, 0);
    PanelModels->SetSizer(FlexGridSizerModels);
    wxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
    TreeListViewModels = CreateTreeListCtrl(wxTL_MULTIPLE, new_panel);
    sizer1->Add(TreeListViewModels, wxSizerFlags(2).Expand());
    new_panel->SetSizer(sizer1);
    sizer1->SetSizeHints(new_panel);

    comparator.SetFrame(xlights);
    TreeListViewModels->SetItemComparator(&comparator);

    ModelSplitter->ReplaceWindow(SecondPanel, propertyEditor);

    wxConfigBase* config = wxConfigBase::Get();
    int msp = config->Read("LayoutModelSplitterSash", -1);
    int sp = config->Read("LayoutMainSplitterSash", -1);
    is_3d = config->ReadBool("LayoutMode3D", false);

    CheckBox_3D->SetValue(is_3d);
    xlights->GetHousePreview()->Set3D(is_3d);

    if (is_3d)
    {
        ChoiceLayoutGroups->Disable();
        ChoiceLayoutGroups->SetToolTip("3D is only supported in the Default preview.");
    }
    else
    {
        ChoiceLayoutGroups->Enable();
        ChoiceLayoutGroups->UnsetToolTip();
    }
    modelPreview->Set3D(is_3d);

    propertyEditor->Connect(wxEVT_PG_CHANGING, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridChanging,0,this);
    propertyEditor->Connect(wxEVT_PG_CHANGED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridChange,0,this);
    propertyEditor->Connect(wxEVT_PG_SELECTED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridSelection,0,this);
    propertyEditor->Connect(wxEVT_PG_ITEM_COLLAPSED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridItemCollapsed,0,this);
    propertyEditor->Connect(wxEVT_PG_ITEM_EXPANDED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridItemExpanded,0,this);
    propertyEditor->Connect(wxEVT_PG_RIGHT_CLICK, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridRightClick, 0, this);
    propertyEditor->SetValidationFailureBehavior(wxPG_VFB_MARK_CELL | wxPG_VFB_BEEP);

    logger_base.debug("LayoutPanel property grid created");

    ToolSizer->SetCols(21);
    AddModelButton("Arches", arches);
    AddModelButton("Candy Canes", canes);
    AddModelButton("Channel Block", channelblock_xpm);
    AddModelButton("Circle", circles);
    AddModelButton("Cube", cube_xpm);
    AddModelButton("Custom", custom);
    AddModelButton("DMX", add_dmx_xpm);
    AddModelButton("Image", image_xpm);
    AddModelButton("Icicles", icicles_xpm);
    AddModelButton("Matrix", matrix);
    AddModelButton("Single Line", singleline);
    AddModelButton("Poly Line", polyline);
    AddModelButton("Sphere", sphere);
    AddModelButton("Spinner", spinner);
    AddModelButton("Star", star);
    AddModelButton("Tree", tree);
    AddModelButton("Window Frame", frame);
    AddModelButton("Wreath", wreath);
    AddModelButton("Import Custom", import);
    AddModelButton("Download", download);
    obj_button = AddModelButton("Add Object", object);
    obj_button->Enable(is_3d && ChoiceLayoutGroups->GetStringSelection() == "Default");

    logger_base.debug("LayoutPanel model buttons created");

    modelPreview->Connect(wxID_CUT, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoCut, nullptr,this);
    modelPreview->Connect(wxID_COPY, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoCopy, nullptr,this);
    modelPreview->Connect(wxID_PASTE, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoPaste, nullptr,this);
    modelPreview->Connect(wxID_UNDO, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoUndo, nullptr,this);
    modelPreview->Connect(wxID_ANY, wxEVT_CHAR_HOOK, wxKeyEventHandler(LayoutPanel::OnCharHook), nullptr,this);
    modelPreview->Connect(wxID_ANY, wxEVT_CHAR, wxKeyEventHandler(LayoutPanel::OnChar), nullptr,this);

    TreeListViewModels->GetView()->Connect(wxID_CUT, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoCut, nullptr,this);
    TreeListViewModels->GetView()->Connect(wxID_COPY, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoCopy, nullptr,this);
    TreeListViewModels->GetView()->Connect(wxID_PASTE, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoPaste, nullptr,this);
    TreeListViewModels->GetView()->Connect(wxID_UNDO, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoUndo, nullptr,this);
    TreeListViewModels->GetView()->Connect(wxID_ANY, wxEVT_CHAR_HOOK, wxKeyEventHandler(LayoutPanel::OnListCharHook), nullptr, this);
    wxScrolledWindow *sw = new wxScrolledWindow(ModelSplitter);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sw->SetSizer(sizer);
    model_grp_panel = new ModelGroupPanel(sw, xlights->AllModels, this);
    sizer->Add( model_grp_panel, 1, wxEXPAND | wxALL, 1 );
    sw->SetScrollRate(5,5);
    sw->Hide();

    logger_base.debug("LayoutPanel model group panel created");

    // Setup the Object List Panel
    //wxScrolledWindow *sw2 = new wxScrolledWindow(ModelSplitter);
    wxBoxSizer* sizer2 = new wxBoxSizer(wxVERTICAL);
    //->SetSizer(sizer2);
    objects_panel = new ViewObjectPanel(PanelObjects, xlights->AllObjects, this);
    sizer2->Add( objects_panel, 1, wxEXPAND | wxALL, 1 );
    sizer2->SetSizeHints(objects_panel);
    PanelObjects->SetSizer(sizer2);
    sizer1->SetSizeHints(PanelObjects);
    //sw2->SetScrollRate(5,5);
    //sw2->Hide();
    //ViewObjectWindow = sw2;
    logger_base.debug("LayoutPanel object panel created");

    LeftPanelSizer->Fit(LeftPanel);
    LeftPanelSizer->SetSizeHints(LeftPanel);
    FlexGridSizerPreview->Fit(this);
    FlexGridSizerPreview->SetSizeHints(this);

    ModelGroupWindow = sw;

    if( !is_3d ) {
        Notebook_Objects->RemovePage(1);
    }

    if (sp != -1) {
        SplitterWindow2->SetSashGravity(0.0);
        SplitterWindow2->SetSashPosition(sp);
    }
    if (msp != -1) {
        ModelSplitter->SetSashGravity(0.0);
        ModelSplitter->SetSashPosition(msp);
    }

    TreeListViewModels->SetColumnWidth(0, wxCOL_WIDTH_AUTOSIZE);
    TreeListViewModels->SetColumnWidth(1, TreeListViewModels->WidthFor(CHNUMWIDTH));
    TreeListViewModels->SetColumnWidth(2, TreeListViewModels->WidthFor(CHNUMWIDTH));
    TreeListViewModels->SetColumnWidth(3, wxCOL_WIDTH_AUTOSIZE);

    if (ModelSplitter->GetSashPosition() < 200)
    {
        ModelSplitter->SetSashPosition(200, true);
    }
}

wxTreeListCtrl* LayoutPanel::CreateTreeListCtrl(long style, wxPanel* panel)
{
    wxTreeListCtrl* const
        tree = new wxTreeListCtrl(panel, ID_TREELISTVIEW_MODELS,
                                  wxDefaultPosition, wxDefaultSize,
                                  style, "ID_TREELISTVIEW_MODELS");
    tree->SetImages(m_imageList);

    tree->AppendColumn(MODELCOLNAME,
                       wxCOL_WIDTH_AUTOSIZE,
                       wxALIGN_LEFT,
                       wxCOL_RESIZABLE | wxCOL_SORTABLE);

    // Because you cant programmatically reorder the columns we have to add them in the right order
    wxConfigBase* config = wxConfigBase::Get();
    auto colOrder = config->Read("LayoutModelListCols", "");

    int sortcol = 0;
    bool sortasc = true;

    wxArrayString cols;
    if (colOrder != "") {
        cols = wxSplit(colOrder, ',');
        int cc = 1;
        for (int i = 0; i < cols.size(); i++) {
            if (cols[i] != "") {
                if (cols[i][0] == 'U') {
                    sortcol = cc++;
                    sortasc = true;
                }
                else if (cols[i][0] == 'D') {
                    sortcol = cc++;
                    sortasc = false;
                }
                else if (cols[i][0] == ' ') {
                    cc++;
                }
                cols[i] = cols[i].substr(1);
            }
        }

        // If cols are missing ... add them in
        if (std::find(begin(cols), end(cols), STARTCHANCOLNAME) ==  end(cols)) cols.push_back(STARTCHANCOLNAME);
        if (std::find(begin(cols), end(cols), ENDCHANCOLNAME) == end(cols)) cols.push_back(ENDCHANCOLNAME);
        if (std::find(begin(cols), end(cols), CONTCONNCOLNAME) == end(cols)) cols.push_back(CONTCONNCOLNAME);
    }
    else {
        cols.push_back(STARTCHANCOLNAME);
        cols.push_back(ENDCHANCOLNAME);
        cols.push_back(CONTCONNCOLNAME);
    }

    int i = 1;
    for (const auto& c : cols) {
        if (c == STARTCHANCOLNAME) {
            tree->AppendColumn(STARTCHANCOLNAME,
                tree->WidthFor(CHNUMWIDTH),
                wxALIGN_LEFT,
                wxCOL_RESIZABLE | wxCOL_SORTABLE | wxCOL_REORDERABLE);
            Col_StartChan = i++;
        }
        else if (c == ENDCHANCOLNAME) {
            tree->AppendColumn(ENDCHANCOLNAME,
                tree->WidthFor(CHNUMWIDTH),
                wxALIGN_LEFT,
                wxCOL_RESIZABLE | wxCOL_SORTABLE | wxCOL_REORDERABLE);
            Col_EndChan = i++;
        }
        else if (c == CONTCONNCOLNAME) {
            tree->AppendColumn(CONTCONNCOLNAME,
                wxCOL_WIDTH_AUTOSIZE,
                wxALIGN_LEFT,
                wxCOL_RESIZABLE | wxCOL_SORTABLE | wxCOL_REORDERABLE);
            Col_ControllerConnection = i++;
        }
    }

    tree->SetSortColumn(sortcol, sortasc);
    return tree;
}

void LayoutPanel::Reset()
{
    selectedBaseObject = nullptr;
    highlightedBaseObject = nullptr;
    SetCurrentLayoutGroup(xlights->GetStoredLayoutGroup());
    ChoiceLayoutGroups->Clear();
    ChoiceLayoutGroups->Append("Default");
    ChoiceLayoutGroups->Append("All Models");
    ChoiceLayoutGroups->Append("Unassigned");
    for (const auto& it : xlights->LayoutGroups) {
        LayoutGroup* grp = (LayoutGroup*)(it);
        ChoiceLayoutGroups->Append(grp->GetName());
    }
    ChoiceLayoutGroups->Append("<Create New Preview>");
    ChoiceLayoutGroups->SetSelection(0);
    for( int i = 0; i < (int)ChoiceLayoutGroups->GetCount(); i++ )
    {
        if( ChoiceLayoutGroups->GetString(i) == currentLayoutGroup )
        {
            ChoiceLayoutGroups->SetSelection(i);
            break;
        }
    }
    obj_button->Enable(is_3d && ChoiceLayoutGroups->GetStringSelection() == "Default");
}

void LayoutPanel::SetDirtyHiLight(bool dirty) {
    if (dirty) {
        xlights->UnsavedRgbEffectsChanges = true;
    }
//    if (dirty)
//    {
//        xlights->RebuildControllerConfig(xlights->GetOutputManager(), &xlights->AllModels);
//    }
    xlights->UpdateLayoutSave();
    xlights->UpdateControllerSave();
}

std::string LayoutPanel::GetCurrentPreview() const
{
    return ChoiceLayoutGroups->GetStringSelection().ToStdString();
}

#ifdef __WXMSW__
// On windows wxIMAGE_QUALITY_HIGH results in blank rescaled images
#define RESCALE_MODEL_BUTTON_QUALITY wxIMAGE_QUALITY_BICUBIC 
#else
#define RESCALE_MODEL_BUTTON_QUALITY wxIMAGE_QUALITY_HIGH
#endif

NewModelBitmapButton* LayoutPanel::AddModelButton(const std::string &type, const char *data[]) {

    wxImage image(data);
    wxImage disImage = image.ConvertToDisabled();
    wxImage presImage = image.ConvertToDisabled(128);

    wxImage img24 = image.Scale(24, 24, RESCALE_MODEL_BUTTON_QUALITY);
    wxImage disImg24 = disImage.Scale(24, 24, RESCALE_MODEL_BUTTON_QUALITY);
    wxImage presImg24 = presImage.Scale(24, 24, RESCALE_MODEL_BUTTON_QUALITY);

    wxBitmapBundle bmp = wxBitmapBundle::FromBitmaps(img24, image);
    wxBitmapBundle disBmp = wxBitmapBundle::FromBitmaps(disImg24, disImage);
    wxBitmapBundle presBmp = wxBitmapBundle::FromBitmaps(presImg24, presImage);

    NewModelBitmapButton *button = new NewModelBitmapButton(PreviewGLPanel, bmp, disBmp, presBmp, type);
    ToolSizer->Add(button, 1, wxALL, 0);
    buttons.push_back(button);
    Connect(button->GetId(), wxEVT_BUTTON, (wxObjectEventFunction)&LayoutPanel::OnNewModelTypeButtonClicked);
    return button;
}

int LayoutPanel::GetColumnIndex(const std::string& name) const
{
    for (size_t i = 0; i < TreeListViewModels->GetColumnCount(); i++) {
        auto col = TreeListViewModels->GetDataView()->GetColumn(i);
        if (col->GetTitle() == name) return i;
    }
    wxASSERT(false);
    return -1;
}

void LayoutPanel::SaveModelsListColumns()
{
    wxString colOrder;
    for (size_t i = 0; i < TreeListViewModels->GetColumnCount(); i++) {
        for (size_t j = 0; j < TreeListViewModels->GetColumnCount(); j++) {
            auto col = TreeListViewModels->GetDataView()->GetColumn(j);
            auto p = TreeListViewModels->GetDataView()->GetColumnPosition(col);
            if (p == i && col->GetTitle() != MODELCOLNAME) {
                if (col->IsSortKey()) {
                    if (col->IsSortOrderAscending()) {
                        colOrder += "U";
                    }
                    else {
                        colOrder += "D";
                    }
                }
                else {
                    colOrder += " ";
                }
                colOrder += col->GetTitle() + ",";
            }
        }
    }

    wxConfigBase* config = wxConfigBase::Get();
    config->Write("LayoutModelListCols", colOrder);
}

LayoutPanel::~LayoutPanel()
{
    SaveModelsListColumns();
    if (background != nullptr) {
        delete background;
    }
    TreeListViewModels->SetItemComparator(nullptr);
    TreeListViewModels->DeleteAllItems();
	//(*Destroy(LayoutPanel)
	//*)
}

void LayoutPanel::OnPropertyGridChange(wxPropertyGridEvent& event) {
    wxString name = event.GetPropertyName();
    updatingProperty = true;
    if (name == "BkgBrightness") {
        if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
            xlights->SetPreviewBackgroundBrightness(event.GetValue().GetLong(), previewBackgroundAlpha);
        }
        else {
            pGrp->SetBackgroundBrightness(event.GetValue().GetLong(), previewBackgroundAlpha);
            modelPreview->SetBackgroundBrightness(event.GetValue().GetLong(), previewBackgroundAlpha);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPropertyGridChange::Brightness");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPropertyGridChange::Brightness");
        }
    }
    else if (name == "BkgTransparency") {
        if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
            xlights->SetPreviewBackgroundBrightness(previewBackgroundBrightness, 100 - event.GetValue().GetLong());
        }
        else {
            pGrp->SetBackgroundBrightness(previewBackgroundBrightness, 100 - event.GetValue().GetLong());
            modelPreview->SetBackgroundBrightness(previewBackgroundBrightness, 100 - event.GetValue().GetLong());
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPropertyGridChange::BkgTransparency");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPropertyGridChange::BkgTransparency");
        }
    }
    else if (name == "BkgSizeWidth") {
        xlights->SetPreviewSize(event.GetValue().GetLong(), modelPreview->GetVirtualCanvasHeight());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPropertyGridChange::BkgTransparency");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPropertyGridChange::BkgSizeWidth");
    }
    else if (name == "BkgSizeHeight") {
        xlights->SetPreviewSize(modelPreview->GetVirtualCanvasWidth(), event.GetValue().GetLong());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPropertyGridChange::BkgTransparency");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPropertyGridChange::BkgSizeHeight");
    }
    else if (name == "BoundingBox") {
        modelPreview->SetDisplay2DBoundingBox(event.GetValue().GetBool());
        xlights->SetDisplay2DBoundingBox(event.GetValue().GetBool());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPropertyGridChange::BoundingBox");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPropertyGridChange::BoundingBox");
    }
    else if (name == "2DGrid") {
        modelPreview->SetDisplay2DGrid(event.GetValue().GetBool(), xlights->GetDisplay2DGridSpacing());
        xlights->SetDisplay2DGrid(event.GetValue().GetBool());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPropertyGridChange::2DGrid");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPropertyGridChange::2DGrid");
    }
    else if (name == "2DGridSpacing") {
        modelPreview->SetDisplay2DGrid(xlights->GetDisplay2DGrid(), event.GetValue().GetLong());
        xlights->SetDisplay2DGridSpacing(event.GetValue().GetLong());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPropertyGridChange::2DGridSpacing");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPropertyGridChange::2DGridSpacing");
    } else if (name == "2DXZeroIsCenter") {
        modelPreview->SetDisplay2DCenter0(event.GetValue().GetBool());
        xlights->SetDisplay2DCenter0(event.GetValue().GetBool());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPropertyGridChange::2DXZeroIsCenter");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPropertyGridChange::2DXZeroIsCenter");
    } else if (name == "BkgImage") {
        if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
            xlights->SetPreviewBackgroundImage(event.GetValue().GetString());
        }
        else {
            pGrp->SetBackgroundImage(event.GetValue().GetString());
            modelPreview->SetbackgroundImage(event.GetValue().GetString());
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPropertyGridChange::BkgImage");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPropertyGridChange::BkgImage");
        }
    }
    else if (name == "BkgFill") {
        if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
            xlights->SetPreviewBackgroundScaled(event.GetValue().GetBool());
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPropertyGridChange::BkgFill");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPropertyGridChange::BkgFill");
        } else {
            pGrp->SetBackgroundScaled(wxAtoi(event.GetValue().GetString()) > 0);
            modelPreview->SetScaleBackgroundImage(wxAtoi(event.GetValue().GetString()) > 0);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPropertyGridChange::BkgFill");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPropertyGridChange::BkgFill");
        }
    }
    else {
        if (editing_models) {
            if (selectedBaseObject != nullptr) {
                Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
                //model property
                if ("ModelName" == name) {
                    std::string safename = Model::SafeModelName(event.GetValue().GetString().ToStdString());

                    if (safename != event.GetValue().GetString().ToStdString())
                    {
                        // need to update the property grid with the modified name
                        wxPGProperty* prop = propertyEditor->GetPropertyByName("ModelName");
                        if (prop != nullptr) {
                            prop->SetValue(safename);
                        }
                    }
                    std::string oldname = selectedModel->name;
                    if (oldname != safename) {
                        RenameModelInTree(selectedModel, safename);
                        selectedBaseObject = nullptr;
                        xlights->RenameModel(oldname, safename);
                        if (oldname == lastModelName) {
                            lastModelName = safename;
                        }
                    }
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPropertyGridChange::ModelName");
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnPropertyGridChange::ModelName");
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "LayoutPanel::OnPropertyGridChange::ModelName", nullptr, nullptr, safename);
                }
                else {
                    if (selectedModel != nullptr) {
                        selectedModel->SaveDisplayDimensions();
                        int i = selectedModel->OnPropertyGridChange(propertyEditor, event);
                        if ((i & GRIDCHANGE_SUPPRESS_HOLDSIZE) == 0 &&
                            (dynamic_cast<ModelWithScreenLocation<BoxedScreenLocation>*>(selectedModel) != nullptr ||
                                dynamic_cast<ModelWithScreenLocation<ThreePointScreenLocation>*>(selectedModel) != nullptr)) {
                            // only restore if not suppressed and if it is a boxed screen location
                            xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::OnPropertyGridChange", selectedModel);
                            selectedModel->RestoreDisplayDimensions();
                        }
                        wxASSERT(i == 0 || i == GRIDCHANGE_SUPPRESS_HOLDSIZE);
                    }
                }
            }
        }
        else {
            objects_panel->OnPropertyGridChange(propertyEditor, event);
        }
    }
    updatingProperty = false;
}

void LayoutPanel::SetDisplay2DBoundingBox(bool bb)
{
    modelPreview->SetDisplay2DBoundingBox(bb);
}

void LayoutPanel::SetDisplay2DGridSpacing(bool bb, long spacing)
{
    modelPreview->SetDisplay2DGrid(bb, spacing);
}

void LayoutPanel::SetDisplay2DCenter0(bool bb) {
    modelPreview->SetDisplay2DCenter0(bb);
}

void LayoutPanel::OnPropertyGridChanging(wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    xlights->AddTraceMessage("LayoutPanel::OnPropertyGridChanging  Property: " + name);
    if (selectedBaseObject != nullptr) {
        if( editing_models ) {
            Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
            if ("ModelName" == name) {
                std::string safename = Model::SafeModelName(event.GetValue().GetString().ToStdString());
                // refuse clashing names or names with unsafe characters
                if (xlights->AllModels[safename] != nullptr || safename != event.GetValue().GetString().ToStdString()) {
                    CreateUndoPoint("ModelName", selectedModel->name, safename);
                    event.Veto();
                }
                // todo do I need to do anything special here
            //} else if ("SubModels" == name) {
                // ignore the submodel changes for now.
            //    int a = 0;
            } else {
                CreateUndoPoint("ModelProperty", selectedModel->name, name, event.GetProperty()->GetValue().GetString().ToStdString());
                selectedModel->OnPropertyGridChanging(propertyEditor, event);
            }
        } else {
            ViewObject* selectedObject = dynamic_cast<ViewObject*>(selectedBaseObject);
            if ("ObjectName" == name) {
                std::string safename = Model::SafeModelName(event.GetValue().GetString().ToStdString());
                // refuse clashing names or names with unsafe characters
                if (xlights->AllObjects[safename] != nullptr || safename != event.GetValue().GetString().ToStdString()) {
                    CreateUndoPoint("ObjectName", selectedObject->name, safename);
                    event.Veto();
                }
            } else {
                CreateUndoPoint("ObjectProperty", selectedObject->name, name, event.GetProperty()->GetValue().GetString().ToStdString());
                //objects_panel->GetSelectedObject()->OnPropertyGridChanging(propertyEditor, event);
            }
        }
    } else {
        CreateUndoPoint("Background", "", name, event.GetProperty()->GetValue().GetString().ToStdString());
    }
}

void LayoutPanel::OnPropertyGridSelection(wxPropertyGridEvent& event) {
    if (selectedBaseObject != nullptr) {
        if( editing_models ) {
            Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
            if( selectedModel->GetDisplayAs() == "Poly Line" ) {
                int segment = selectedModel->OnPropertyGridSelection(propertyEditor, event);
                selectedModel->GetBaseObjectScreenLocation().SelectSegment(segment);
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPropertyGridSelection");
            }
        }
    }
}

void LayoutPanel::OnPropertyGridItemCollapsed(wxPropertyGridEvent& event) {
    if (selectedBaseObject != nullptr) {
        if( editing_models ) {
            Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
            if( selectedModel->GetDisplayAs() == "Poly Line" ) {
                selectedModel->OnPropertyGridItemCollapsed(propertyEditor, event);
            }
        }
    }
}

void LayoutPanel::OnPropertyGridItemExpanded(wxPropertyGridEvent& event) {
    if (selectedBaseObject != nullptr) {
        if( editing_models ) {
            Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
            if( selectedModel->GetDisplayAs() == "Poly Line" ) {
                selectedModel->OnPropertyGridItemExpanded(propertyEditor, event);
            }
        }
    }
}

void LayoutPanel::OnPropertyGridRightClick(wxPropertyGridEvent& event)
{
    if (selectedBaseObject != nullptr) {
        wxMenu mnu;
        selectedBaseObject->HandlePropertyGridRightClick(event, mnu);
        if (mnu.GetMenuItemCount() != 0) {
            mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridContextMenu, nullptr, this);
            PopupMenu(&mnu);
        }
    }
}

void LayoutPanel::OnPropertyGridContextMenu(wxCommandEvent& event)
{
    if (selectedBaseObject != nullptr) {
        selectedBaseObject->HandlePropertyGridContextMenu(event);
    }
}

void LayoutPanel::RefreshLayout()
{
    std::string selectedBaseObjectName = "";
    if (selectedBaseObject != nullptr) selectedBaseObjectName = selectedBaseObject->name;
    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::RefreshLayout", nullptr, nullptr, selectedBaseObjectName);
    ShowPropGrid(true);
}

void LayoutPanel::RenderLayout()
{
    if (modelPreview == nullptr || !modelPreview->StartDrawing(mPointSize)) return;

    modelPreview->Render();
    if (m_creating_bound_rect)
    {
        if (!is_3d) {
            modelPreview->AddBoundingBoxToAccumulator(m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y);
        }
    }
    modelPreview->EndDrawing();
}

void LayoutPanel::UpdatePreview()
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("        UpdatePreview.");
    SetDirtyHiLight(xlights->UnsavedRgbEffectsChanges);
    RenderLayout();
}

void LayoutPanel::updatePropertyGrid()
{
    if (selectedBaseObject == nullptr || propertyEditor == nullptr || ModelsSelectedCount() > 1) return;

    selectedBaseObject->UpdateProperties(propertyEditor, xlights->GetOutputManager());
}

void LayoutPanel::ClearSelectedModelGroup()
{
    if (model_grp_panel != nullptr) model_grp_panel->UpdatePanel("");
}

void LayoutPanel::resetPropertyGrid() {
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("        resetPropertyGrid.");

    if (selectedBaseObject != nullptr && selectedBaseObject->GetDisplayAs() == "ModelGroup")
    {
        ShowPropGrid(false);
    }
    else
    {
        ShowPropGrid(true);
        propertyEditor->Freeze();
        auto save = propertyEditor->SaveEditableState();
        //auto scroll = propertyEditor->GetScrollPos(wxVERTICAL);
        //auto top = propertyEditor->GetItemAtY(0);
        wxString selProp = "";
        if (propertyEditor->GetSelection() != nullptr)
        {
            selProp = propertyEditor->GetSelection()->GetName();
        }
        clearPropGrid();
        if (selectedBaseObject != nullptr) {
            SetupPropGrid(selectedBaseObject);
            propertyEditor->RestoreEditableState(save);
            if (selProp != "")
            {
                auto p = propertyEditor->GetPropertyByName(selProp);
                if (p != nullptr) propertyEditor->EnsureVisible(p);
            }
            else
            {
                //propertyEditor->SetScrollPos(wxVERTICAL, scroll - 1, true);
                //wxScrollWinEvent e(wxEVT_SCROLLWIN_THUMBRELEASE);
                //e.SetOrientation(wxVERTICAL);
                //e.SetPosition(scroll);
                //propertyEditor->HandleOnScroll(e);
            }
        }
        propertyEditor->Thaw();
    }
}

void LayoutPanel::clearPropGrid() {

    // remember last selected item
    if (propertyEditor->GetSelection() != nullptr) {
        _lastSelProp = propertyEditor->GetSelection()->GetName();
    }

    wxPGProperty *p = propertyEditor->GetPropertyByName("ModelAppearance");
    if (p != nullptr) {
        appearanceVisible = propertyEditor->IsPropertyExpanded(p);
    }
    p = propertyEditor->GetPropertyByName("ModelSize");
    if (p != nullptr) {
        sizeVisible = propertyEditor->IsPropertyExpanded(p);
    }
    p = propertyEditor->GetPropertyByName("Dimensions");
    if (p != nullptr) {
        dimensionsVisible = propertyEditor->IsPropertyExpanded(p);
    }
    p = propertyEditor->GetPropertyByName("ModelStringProperties");
    if (p != nullptr) {
        stringPropsVisible = propertyEditor->IsPropertyExpanded(p);
    }
    p = propertyEditor->GetPropertyByName("ModelControllerConnectionProperties");
    if (p != nullptr) {
        controllerConnectionVisible = propertyEditor->IsPropertyExpanded(p);
    }
    p = propertyEditor->GetPropertyByName("Layers");
    if (p != nullptr) {
        layersVisible = propertyEditor->IsPropertyExpanded(p);
    }

    propertyEditor->Clear();
}

void LayoutPanel::refreshObjectList() {
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("        refreshObjectList.");
    objects_panel->refreshObjectList();
}

std::string LayoutPanel::TreeModelName(const Model* model, bool fullname)
{
    std::string name = fullname ? model->GetFullName() : model->name;
    if (model->IsActive())
    {
        return name;
    }
    else
    {
        return "<" + name + ">";
    }
}
void LayoutPanel::FreezeTreeListView() {
    TreeListViewModels->Freeze();
    //turn off the column width auto-resize.  Makes it REALLY slow to populate the tree
    TreeListViewModels->SetColumnWidth(0, TreeListViewModels->GetColumnWidth(0));
    TreeListViewModels->SetColumnWidth(3, TreeListViewModels->GetColumnWidth(3));
}

void LayoutPanel::ThawTreeListView() {
    TreeListViewModels->SetColumnWidth(0, wxCOL_WIDTH_AUTOSIZE);
    // we should have calculated a size, now turn off the auto-sizes as it's SLOW to update anything later
    int i = TreeListViewModels->GetColumnWidth(0);
    if (i <= 20) {
        i = TreeListViewModels->GetSize().GetWidth() / 3;
    }
    if (i <= 20) {
        i = 100;
    }
    TreeListViewModels->SetColumnWidth(0, i);
    TreeListViewModels->SetColumnWidth(3, wxCOL_WIDTH_AUTOSIZE);
    TreeListViewModels->Thaw();
    TreeListViewModels->Refresh();
}

void LayoutPanel::SetTreeListViewItemText(wxTreeListItem &item, int col, const wxString &txt) {
    wxString orig = TreeListViewModels->GetItemText(item, col);
    if (orig != txt) {
        TreeListViewModels->SetItemText(item, col, txt);
    }
}

void LayoutPanel::refreshModelList() {

    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("        refreshModelList.");

    FreezeTreeListView();

    for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
          item.IsOk();
          item = TreeListViewModels->GetNextItem(item) )
    {
        ModelTreeData *data = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
        Model *model = data != nullptr ? data->GetModel() : nullptr;

        if (model != nullptr ) {

            if( model->GetDisplayAs() != "ModelGroup" ) {
                wxString cv = TreeListViewModels->GetItemText(item, Col_StartChan);
                wxString startStr = model->GetStartChannelInDisplayFormat(xlights->GetOutputManager());
                if (cv != startStr) {
                    data->startingChannel = model->GetNumberFromChannelString(model->ModelStartChannel);
                    if (model->GetDisplayAs() == "SubModel" || (model->CouldComputeStartChannel && model->IsValidStartChannelString())) {
                        SetTreeListViewItemText(item, Col_StartChan, startStr);
                    } else {
                        SetTreeListViewItemText(item, Col_StartChan, "*** " + model->ModelStartChannel);
                    }
                }
                cv = TreeListViewModels->GetItemText(item, Col_EndChan);
                wxString endStr = model->GetLastChannelInStartChannelFormat(xlights->GetOutputManager());
                if (cv != endStr) {
                    data->endingChannel = model->GetLastChannel()+1;

                    SetTreeListViewItemText(item, Col_EndChan, endStr);
                }
                cv = TreeListViewModels->GetItemText(item, Col_ControllerConnection);

                std::string cc = model->GetControllerConnectionRangeString();
                if (cv != cc) {
                    SetTreeListViewItemText(item, Col_ControllerConnection, cc);
                }
            }
        }
    }
    ThawTreeListView();
}

void LayoutPanel::RenameModelInTree(Model *model, const std::string& new_name)
{
    for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
          item.IsOk();
          item = TreeListViewModels->GetNextItem(item) )
    {
        ModelTreeData *data = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
        if (data != nullptr && data->GetModel() == model) {
            if (model->IsActive())
            {
                SetTreeListViewItemText(item, 0, new_name);
            }
            else
            {
                SetTreeListViewItemText(item, 0, "<" + new_name + ">");
            }
        }
    }
}


int LayoutPanel::AddModelToTree(Model *model, wxTreeListItem* parent, bool expanded, int nativeOrder, bool fullName) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (model == nullptr) {
        logger_base.crit("LayoutPanel::AddModelToTree model is null ... this is going to crash.");
        wxASSERT(false);
    }

    //logger_base.debug("Adding model %s", (const char *)model->GetFullName().c_str());

    wxTreeListItem item = TreeListViewModels->AppendItem(*parent, TreeModelName(model, fullName),
                                                         LayoutUtils::GetModelTreeIcon(model->DisplayAs, LayoutUtils::GroupMode::Closed),
                                                         LayoutUtils::GetModelTreeIcon(model->DisplayAs, LayoutUtils::GroupMode::Opened),
                                                         new ModelTreeData(model, nativeOrder, fullName));
    if( model->GetDisplayAs() != "ModelGroup" ) {
        wxString endStr = model->GetLastChannelInStartChannelFormat(xlights->GetOutputManager());
        wxString startStr = model->GetStartChannelInDisplayFormat(xlights->GetOutputManager());
        if (model->GetDisplayAs() == "SubModel" || (model->CouldComputeStartChannel && model->IsValidStartChannelString()))
        {
            SetTreeListViewItemText(item, Col_StartChan, startStr);
        }
        else
        {
            SetTreeListViewItemText(item, Col_StartChan, "*** " + model->ModelStartChannel);
        }
        SetTreeListViewItemText(item, Col_EndChan, endStr);

        std::string cc = model->GetControllerConnectionRangeString();
        SetTreeListViewItemText(item, Col_ControllerConnection, cc);
    }

    for (int x = 0; x < model->GetNumSubModels(); x++) {
        AddModelToTree(model->GetSubModel(x), &item, false, x);
    }
    if( model->GetDisplayAs() == "ModelGroup" ) {
        ModelGroup *grp = (ModelGroup*)model;
        int i = 0;
        for (const auto& it : grp->ModelNames()) {
            Model *m = xlights->AllModels[it];

            if (m == nullptr)
            {
                logger_base.error("Model group %s thought it contained model. '%s' but it didnt. This would have crashed.", (const char *)grp->GetName().c_str(), (const char *)it.c_str());
            }
            else if (m == grp)
            {
                // This is bad ... a model group contains itself
                logger_base.error("Model group contains itself. '%s'", (const char *)grp->GetName().c_str());
            }
            else
            {
                AddModelToTree(m, &item, false, i, true);
                i++;
            }
        }
    }

    if (expanded) TreeListViewModels->Expand(item);

    return 0;
}

void LayoutPanel::ReloadModelList() {
    UpdateModelList(true);
}

void LayoutPanel::UpdateModelList(bool full_refresh) {
    std::vector<Model *> models;
    UpdateModelList(full_refresh, models);
    objects_panel->UpdateObjectList(full_refresh, currentLayoutGroup);
}

void LayoutPanel::UpdateModelList(bool full_refresh, std::vector<Model*> &models) {

    //static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    FreezeTreeListView();
    unsigned sortcol;
    bool ascending;
    bool sorted = TreeListViewModels->GetSortColumn(&sortcol, &ascending);

    std::vector<Model *> dummy_models;

    // Update all the custom previews
    for (const auto& it : xlights->LayoutGroups) {
        LayoutGroup* grp = (LayoutGroup*)(it);
        dummy_models.clear();
        if (grp->GetName() == currentLayoutGroup) {
            UpdateModelsForPreview(currentLayoutGroup, grp, models, true);
        } else {
            UpdateModelsForPreview(grp->GetName(), grp, dummy_models, false);
        }
    }

    // update the Layout tab preview for default options
    if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
        UpdateModelsForPreview(currentLayoutGroup, nullptr, models, true);
    }

    //logger_base.debug("Layout tab preview models updated.");
    xlights->PreviewModels = models;

    if (full_refresh) {
        UnSelectAllModels();

        //turn off the sorting as that is ALSO really slow
        TreeListViewModels->SetItemComparator(nullptr);
        if (sorted) {
            //UnsetAsSortKey may be unimplemented on all  platforms so we'll set a
            //sort column to 0 which is faster due to straight string compare
            TreeListViewModels->GetDataView()->GetSortingColumn()->UnsetAsSortKey();
        }

        //delete all items will attempt to resort as each item is deleted, however, our Model pointers
        //stored in the items may be invalid
        wxTreeListItem child = TreeListViewModels->GetFirstItem();
        std::list<std::string> expanded;
        while (child.IsOk()) {
            if (TreeListViewModels->IsExpanded(child)) {
                expanded.push_back(TreeListViewModels->GetItemText(child));
            }
            TreeListViewModels->DeleteItem(child);
            child = TreeListViewModels->GetFirstItem();
        }
        TreeListViewModels->DeleteAllItems();
        if (sorted) {
            //UnsetAsSortKey may be unimplemented on all  platforms so we'll set a
            //sort column to 0 which is faster due to straight string compare
            TreeListViewModels->SetSortColumn(0, true);
            //then turn it off again so platforms that DO support this can benefit
            TreeListViewModels->GetDataView()->GetSortingColumn()->UnsetAsSortKey();
        }

        wxTreeListItem root = TreeListViewModels->GetRootItem();
        // add all the model groups
        for (const auto& it : xlights->AllModels) {
            Model *model = it.second;
            if (model->GetDisplayAs() == "ModelGroup") {
                if (currentLayoutGroup == "All Models" || model->GetLayoutGroup() == currentLayoutGroup
                    || (model->GetLayoutGroup() == "All Previews" && currentLayoutGroup != "Unassigned")) {
                    bool expand = (std::find(expanded.begin(), expanded.end(), model->GetName()) != expanded.end());
                    AddModelToTree(model, &root, expand, 0);
                }
            }
        }

        // add all the models
        for (const auto& it : models) {
            Model *model = it;
            if (model->GetDisplayAs() != "ModelGroup" && model->GetDisplayAs() != "SubModel") {
                bool expand = (std::find(expanded.begin(), expanded.end(), model->GetName()) != expanded.end());
                AddModelToTree(model, &root, expand, 0);
            }
        }

        // Only set the column sizes the very first time we load it
        if (_firstTreeLoad) {
            _firstTreeLoad = false;

            TreeListViewModels->SetColumnWidth(1, wxCOL_WIDTH_AUTOSIZE);
            int width = TreeListViewModels->GetColumnWidth(1);
            if (width < 20) {
                width = TreeListViewModels->WidthFor(STARTCHANCOLNAME);
            }
            TreeListViewModels->SetColumnWidth(1, width);

            TreeListViewModels->SetColumnWidth(2, wxCOL_WIDTH_AUTOSIZE);
            width = TreeListViewModels->GetColumnWidth(2);
            if (width < 20) {
                width = TreeListViewModels->WidthFor(STARTCHANCOLNAME);
            }
            TreeListViewModels->SetColumnWidth(2, width);
        }

        //turn the sorting back on
        TreeListViewModels->SetItemComparator(&comparator);
        if (sorted) {
            TreeListViewModels->SetSortColumn(sortcol, ascending);
            TreeListViewModels->GetDataView()->GetModel()->Resort();
        }
    }
    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::UpdateModelList");

    ThawTreeListView();
}

void LayoutPanel::UpdateModelsForPreview(const std::string &group, LayoutGroup* layout_grp, std::vector<Model *> &prev_models, bool filtering)
{
    //static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //logger_base.debug("Updated models for preview: %s.", (const char*)group.c_str());

    std::set<std::string> modelsAdded;

    for (const auto& it : xlights->AllModels) {
        Model *model = it.second;
        if (model->GetDisplayAs() != "ModelGroup") {
            if (group == "All Models" ||
                model->GetLayoutGroup() == group ||
                (model->GetLayoutGroup() == "All Previews" && group != "Unassigned")) {
                prev_models.push_back(model);
                modelsAdded.insert(model->name);
            }
        }
    }

    // add in any models that were not in preview but belong to a group that is in the preview
    wxArrayString selectedGroupNames;

    if (selectedTreeGroups.size() > 0 && filtering) {
        for (int i = 0; i < selectedTreeGroups.size(); i++) {
            wxString selectedName = TreeListViewModels->GetItemText(selectedTreeGroups[i]);
            selectedGroupNames.Add(selectedName);
        }
    }

    for (const auto& it : xlights->AllModels) {
        Model *model = it.second;
        bool mark_selected = false;
        if (selectedGroupNames.size() > 0 && filtering && (selectedGroupNames.Index(model->name) != -1)) {
            mark_selected = true;
        }
        if (model->GetDisplayAs() == "ModelGroup") {
            ModelGroup *grp = (ModelGroup*)(model);
            if (group == "All Models" ||
                model->GetLayoutGroup() == group ||
                (model->GetLayoutGroup() == "All Previews" && group != "Unassigned")) {
                for (const auto& it2 : grp->ModelNames()) {
                    Model *m = xlights->AllModels[it2];
                    if (m != nullptr) {
                        if (mark_selected) {
                            if (selectedBaseObject == nullptr)
                            {
                                // This code here used to choose the first model in the group
                                // which was great if you wanted to align to that model but it causes
                                // problems when trying to display the selected submodels.
                                // Given the model selected to the user was essentially random it is
                                // of limited value (and they can always shift click to select a particular
                                // model for alignment so i am removing this and setting it to the group
                                // itself
                                SelectModel(model, false);
                            }
                            // DP - should not need these
                            // m->GroupSelected = true;
                            // m->Highlighted = true;
                        }
                        if (m->DisplayAs == "SubModel") {
                            if (mark_selected) {
                                prev_models.push_back(m);
                            }
                        }
                        else if (m->DisplayAs == "ModelGroup") {
                            ModelGroup *mg = (ModelGroup*)m;
                            if (mark_selected) {
                                for (const auto& it3 : mg->Models()) {
                                    if (it3->DisplayAs != "ModelGroup") {
                                        if (selectedBaseObject == nullptr)
                                        {
                                            SelectModel(it3, false);
                                        }
                                        // DP - shouldn't need these
                                        // it3->GroupSelected = true;
                                        // it3->Highlighted = true;
                                        prev_models.push_back(it3);
                                    }
                                    else
                                    {
                                        // need to process groups within groups ... safely
                                        for (const auto& itm : xlights->AllModels)
                                        {
                                            if (itm.second->GetDisplayAs() == "ModelGroup")
                                            {
                                                // ignore these
                                            }
                                            else
                                            {
                                                if (dynamic_cast<ModelGroup*>(it3)->ContainsModel(itm.second))
                                                {
                                                    if (std::find(prev_models.begin(), prev_models.end(), itm.second) == prev_models.end())
                                                    {
                                                        if (modelsAdded.find(itm.first) == modelsAdded.end()) {
                                                            modelsAdded.insert(itm.first);
                                                        }
                                                        prev_models.push_back(itm.second);
                                                    }
                                                    // DP - shouldn't need this
                                                    // itm.second->GroupSelected = true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (modelsAdded.find(it2) == modelsAdded.end()) {
                            modelsAdded.insert(it2);
                            prev_models.push_back(m);
                        }
                    }
                }
            }
        }
    }

    // only run this for layout group previews
    if (layout_grp != nullptr) {
        layout_grp->SetModels(prev_models);
        ModelPreview* preview = layout_grp->GetModelPreview();
        if (layout_grp->GetPreviewCreated()) {
            preview->SetActiveLayoutGroup(layout_grp->GetName());
            if (preview->GetActive()) {
                preview->Refresh();
                preview->Update();
            }
        }
    }
}

void LayoutPanel::BulkEditDimmingCurves()
{
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();

    // get the first dimming curve
    ModelDimmingCurveDialog dlg(this);
    std::map<std::string, std::map<std::string, std::string>> dimmingInfo;

    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            wxXmlNode *f = model->GetModelXml()->GetChildren();
            while (f != nullptr) {
                if ("dimmingCurve" == f->GetName()) {
                    wxXmlNode *dc = f->GetChildren();
                    while (dc != nullptr) {
                        std::string name = dc->GetName().ToStdString();
                        wxXmlAttribute *att = dc->GetAttributes();
                        while (att != nullptr) {
                            dimmingInfo[name][att->GetName().ToStdString()] = att->GetValue();
                            att = att->GetNext();
                        }
                        dc = dc->GetNext();
                    }
                }
                f = f->GetNext();
            }

            if (model->GetModelXml()->GetAttribute("ModelBrightness", "-1") != "-1")
            {
                wxString b = model->GetModelXml()->GetAttribute("ModelBrightness", "0");
                dimmingInfo["all"]["gamma"] = "1.0";
                dimmingInfo["all"]["brightness"] = b;
            }

            if (!dimmingInfo.empty()) {
                break;
            }
        }
    }

    if (dimmingInfo.empty()) {
        dimmingInfo["all"]["gamma"] = "1.0";
        dimmingInfo["all"]["brightness"] = "0";
    }

    dlg.Init(dimmingInfo);
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK) {
        dimmingInfo.clear();
        dlg.Update(dimmingInfo);

        // remember selected tree models and paths for reselect
        std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

        for (Model* model : modelsToEdit) {
            if (model != nullptr) {
                wxXmlNode *f1 = model->GetModelXml()->GetChildren();
                while (f1 != nullptr) {
                    if ("dimmingCurve" == f1->GetName()) {
                        model->GetModelXml()->RemoveChild(f1);
                        delete f1;
                        f1 = model->GetModelXml()->GetChildren();
                    }
                    else {
                        f1 = f1->GetNext();
                    }
                }
                f1 = new wxXmlNode(wxXML_ELEMENT_NODE, "dimmingCurve");
                model->GetModelXml()->AddChild(f1);
                model->IncrementChangeCount();
                for (const auto& it : dimmingInfo) {
                    wxXmlNode *dc = new wxXmlNode(wxXML_ELEMENT_NODE, it.first);
                    f1->AddChild(dc);
                    for (const auto& it2 : it.second) {
                        dc->AddAttribute(it2.first, it2.second);
                    }
                }
            }
        }

        // If we don't do these as ImmediateWork then the Model Tree is still frozen and models don't get reselected after refresh
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BulkEditDimmingCurves");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "BulkEditDimmingCurves");
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditDimmingCurves");

        ReselectTreeModels(selectedModelPaths);
    }
}

void LayoutPanel::BulkEditControllerConnection(int id)
{
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();

    // get the first controller connection
    Model *m = nullptr;

    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            std::string protocol = model->GetControllerProtocol();
            if (protocol != "") {
                m = model;
                break;
            }
        }
    }

    controller_connection_bulkedit ccbe = controller_connection_bulkedit::CEBE_CONTROLLERCONNECTION;
    if (id == ID_PREVIEW_BULKEDIT_CONTROLLERDIRECTION) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERDIRECTION;
    }
    else if (id == ID_PREVIEW_BULKEDIT_CONTROLLERSTARTNULLNODES) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERSTARTNULLNODES;
    }
    else if (id == ID_PREVIEW_BULKEDIT_CONTROLLERENDNULLNODES) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERENDNULLNODES;
    }
    else if (id == ID_PREVIEW_BULKEDIT_CONTROLLERGAMMA) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERGAMMA;
    }
    else if (id == ID_PREVIEW_BULKEDIT_CONTROLLERBRIGHTNESS) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERBRIGHTNESS;
    }
    else if (id == ID_PREVIEW_BULKEDIT_CONTROLLERCOLOURORDER) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERCOLOURORDER;
    }
    else if (id == ID_PREVIEW_BULKEDIT_SMARTREMOTE) {
        ccbe = controller_connection_bulkedit::CEBE_SMARTREMOTE;
    }
    else if (id == ID_PREVIEW_BULKEDIT_CONTROLLERGROUPCOUNT) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERGROUPCOUNT;
    }
    else if (id == ID_PREVIEW_BULKEDIT_CONTROLLERPROTOCOL) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERPROTOCOL;
    }
    else if (id == ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTIONINCREMENT) {
        ccbe = controller_connection_bulkedit::CEBE_CONTROLLERCONNECTIONINCREMENT;
    }
    else if (id == ID_PREVIEW_BULKEDIT_SMARTREMOTETYPE) {
        ccbe = controller_connection_bulkedit::CEBE_SMARTREMOTETYPE;
    }

    ControllerConnectionDialog dlg(this, ccbe);
    dlg.Set(m);
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK) {
        // remember selected tree models and paths for reselect
        std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

        for (Model* model: modelsToEdit) {
            if (model != nullptr) {
                dlg.Get(model->GetControllerConnection(), model->GetNumPhysicalStrings());
            }
        }

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnectionDialog::Get");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnectionDialog::Get");
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ControllerConnectionDialog::Get");
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditControllerConnection");

        ReselectTreeModels(selectedModelPaths);
    }
}

void LayoutPanel::BulkEditActive(bool active)
{
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();

    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            std::string preChangeName = TreeModelName(model, false);
            model->SetActive(active);
            std::string postChangeName = TreeModelName(model, false);

            // fix name for reselect
            for (auto& path : selectedModelPaths) {
                if (path.back() == preChangeName) {
                    path.pop_back();
                    path.push_back(postChangeName);
                }
            }
        }
    }

    for (const auto& it : xlights->AllObjects) {

        if (it.second->Selected || it.second->GroupSelected)
        {
            it.second->SetActive(active);
        }

        ViewObject* view_object = it.second;

        if (selectedBaseObject == nullptr)
        {
            SelectBaseObject(view_object);
        }
    }

    if (selectedBaseObject != nullptr)
    {
        selectedBaseObject->SetActive(active);
    }

    if (editing_models) {
        // If we don't clear selection and bulk edits were made with models nested in groups selected then as part of the work
        // the model is also selected in the root of the tree even though it wasn't before.  I could not find any issues with
        // subsequent bulk edits/move/show wiring/export/etc but if this going to be a problem we can also force it to be unselected
        // in tree/preview after. BulkEditDimming was the only one this issue didn't appear and after the work there OMM selectedmodel is "".
        xlights->GetOutputModelManager()->ClearSelectedModel();
    }

    xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditActive");

    // reselect all the models
    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::BulkEditControllerName()
{
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();

    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    std::string name = "";
    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            name = model->GetControllerName();
            if (name != "") {
                break;
            }
        }
    }

    wxArrayString cn;
    int sel = 0;
    int i = 2;
    cn.push_back("");
    cn.push_back(NO_CONTROLLER);
    if (name == NO_CONTROLLER)
        sel = 1;
    for (const auto& it : xlights->GetOutputManager()->GetAutoLayoutControllerNames())
    {
        if (it == name) sel = i;
        cn.push_back(it);
        i++;
    }
    wxSingleChoiceDialog dlg(this, "Choose the controller name", "Controller Name", cn);
    dlg.SetSelection(sel);
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK) {
        name = dlg.GetStringSelection();

        for (Model* model : modelsToEdit) {
            // model->SetControllerName() adds WORK_RELOAD_MODELLIST as ASAP work which causes issues with Reselect on
            // msw and linux when ASAP work needs to be Immediate work before reselect so just update model here and add
            // the appropriate work here.
            //model->SetControllerName(name);
            wxXmlNode* modelXml = model->GetModelXml();
            modelXml->DeleteAttribute("Controller");
            modelXml->AddAttribute("Controller", name);
        }

        std::string sm = xlights->GetOutputModelManager()->GetSelectedModel();
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "BulkEditControllerName");
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "BulkEditControllerName");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BulkEditControllerName");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "BulkEditControllerName");
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_UPDATE_PROPERTYGRID, "BulkEditControllerName");
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_MODELLIST, "BulkEditControllerName");
        // if we don't do this then we get stuck in a reselect loop on msw/linux in tree
        xlights->GetOutputModelManager()->ForceSelectedModel(sm);

        ReselectTreeModels(selectedModelPaths);
    }
}
void LayoutPanel::BulkEditPixelSize() {
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();
    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    int size = 0;
    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            size = std::max(model->GetPixelSize(), size);
        }
    }
    wxNumberEntryDialog dlg(this, "Choose the pixel size",  "Pixel Size:", "Pixel Size", size, 1, 500);
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK) {
        size = dlg.GetValue();
        for (Model* model: modelsToEdit) {
            model->SetPixelSize(size);
        }

        // see comment in BulkEditActive()
        xlights->GetOutputModelManager()->ClearSelectedModel();
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditPixelSize");
        // reselect all the models
        ReselectTreeModels(selectedModelPaths);
    }
}
void LayoutPanel::BulkEditPixelStyle() {
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();
    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    int style = 3;
    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            style = std::min(model->transparency, style);
        }
    }

    static const char *PIXEL_STYLES_VALUES[] = {"Square", "Smooth", "Solid Circle", "Blended Circle"};
    static wxArrayString PIXEL_STYLES(4, PIXEL_STYLES_VALUES);
    wxSingleChoiceDialog dlg(this, "Choose the Pixel Style",  "Pixel Style", PIXEL_STYLES);
    dlg.SetSelection(style);
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK) {
        style = dlg.GetSelection();
        for (Model* model: modelsToEdit) {
            model->SetPixelStyle((Model::PIXEL_STYLE)style);
        }

        // see comment in BulkEditActive()
        xlights->GetOutputModelManager()->ClearSelectedModel();
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditPixelStyle");
        // reselect all the models
        ReselectTreeModels(selectedModelPaths);
    }
}
void LayoutPanel::BulkEditTransparency() {
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();
    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    int trans = 100;
    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            trans = std::min(model->transparency, trans);
        }
    }
    wxNumberEntryDialog dlg(this, "Choose the transparency",  "Transparency:", "Transparency", trans, 0, 100);
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK) {
        trans = dlg.GetValue();
        for (Model* model: modelsToEdit) {
            model->SetTransparency(trans);
        }

        // see comment in BulkEditActive()
        xlights->GetOutputModelManager()->ClearSelectedModel();
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditTransparency");
        // reselect all the models
        ReselectTreeModels(selectedModelPaths);
    }
}
void LayoutPanel::BulkEditBlackTranparency() {
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();
    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    int trans = 100;
    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            trans = std::min(model->blackTransparency, trans);
        }
    }
    wxNumberEntryDialog dlg(this, "Choose the black transparency",  "Black Transparency:", "Black Transparency", trans, 0, 100);
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK) {
        trans = dlg.GetValue();
        for (Model* model: modelsToEdit) {
            model->SetBlackTransparency(trans);
        }

        // see comment in BulkEditActive()
        xlights->GetOutputModelManager()->ClearSelectedModel();
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditBlackTranparency");
        // reselect all the models
        ReselectTreeModels(selectedModelPaths);
    }
}
void LayoutPanel::BulkEditTagColour()
{
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();
    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    wxColour colour = *wxBLACK;
    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            colour = model->GetTagColour();
            if (colour != *wxBLACK) {
                break;
            }
        }
    }

    wxColourData colorData;
    colorData.SetColour(colour);
    wxColourDialog dialog(this, &colorData);
    OptimiseDialogPosition(&dialog);
    if (dialog.ShowModal() == wxID_OK)
    {
        colorData = dialog.GetColourData();
        colour = colorData.GetColour();

        for (Model* model: modelsToEdit) {
            model->SetTagColour(colour);
        }

        // see comment in BulkEditActive()
        xlights->GetOutputModelManager()->ClearSelectedModel();
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditTagColour");

        // reselect all the models
        ReselectTreeModels(selectedModelPaths);
    }
}

void LayoutPanel::BulkEditControllerPreview()
{
    std::vector<Model*> modelsToEdit = GetSelectedModelsForEdit();

    // remember the selected models
    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    // get the first preview
    std::string p = "";
    for (Model* model: modelsToEdit) {
        if (model != nullptr) {
            p = model->GetLayoutGroup();
            if (p != "") break;
        }
    }

    wxArrayString choices = Model::GetLayoutGroups(xlights->AllModels);
    int sel = 0;
    int j = 0;
    for (const auto& it : choices)
    {
        if (it == p) {
            sel = j;
            break;
        }
        j++;
    }
    wxSingleChoiceDialog dlg(this, "Preview", "Preview", choices);
    dlg.SetSelection(sel);
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK)
    {
        for (Model* model : modelsToEdit) {
            if (model != nullptr) {
                model->SetLayoutGroup(dlg.GetStringSelection().ToStdString());
            }
        }

        // see comment in BulkEditActive()
        xlights->GetOutputModelManager()->ClearSelectedModel();
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BulkEditControllerPreview");

        // reselect all the models
        ReselectTreeModels(selectedModelPaths);

        RenderLayout();
    }
}

void LayoutPanel::CreateModelGroupFromSelected()
{
    wxTextEntryDialog dlg(this, "Enter name for new group", "Enter name for new group");
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK) {
        wxString name = wxString(Model::SafeModelName(dlg.GetValue().ToStdString()));
        while (xlights->AllModels.GetModel(name.ToStdString()) != nullptr || name.IsEmpty()) {
            wxTextEntryDialog dlg2(this, "Model of name '" + name + "' already exists. Enter name for new group", "Enter name for new group");
            OptimiseDialogPosition(&dlg2);
            if (dlg2.ShowModal() == wxID_OK) {
                name = wxString(Model::SafeModelName(dlg2.GetValue().ToStdString()));
            }
            else {
                return;
            }
        }

        wxArrayString newGroupModels;

        // save selections so they can be added after create/reload
        // add selected groups
        for (const auto& group : selectedTreeGroups) {
            newGroupModels.Add(TreeListViewModels->GetItemText(group));
        }

        // add selected models
        for (const auto& model : selectedTreeModels) {
            newGroupModels.Add(TreeListViewModels->GetItemText(model));
        }

        // add selected submodels
        for (const auto& submodel : selectedTreeSubModels) {
            ModelTreeData *submodelData = (ModelTreeData*)TreeListViewModels->GetItemData(submodel);
            Model* subModel = ((submodelData != nullptr) ? submodelData->GetModel() : nullptr);

            if (subModel != nullptr) {
                newGroupModels.Add(subModel->GetFullName());
            }
        }

        wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE, "modelGroup");
        xlights->ModelGroupsNode->AddChild(node);
        node->AddAttribute("selected", "0");
        node->AddAttribute("name", name);
        node->AddAttribute("layout", "minimalGrid");
        node->AddAttribute("GridSize", "400");
        wxString grp = currentLayoutGroup == "All Models" ? "Unassigned" : currentLayoutGroup;
        node->AddAttribute("LayoutGroup", grp);

        // create group and reload before adding selected models. prior models were added before create and I was seeing frequent
        // crashes in Render() with invalid model pointers especially with mixed selections (groups, submodels & models)
        xlights->AllModels.AddModel(xlights->AllModels.CreateModel(node));
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "CreateModelGroupFromSelected");

        // now add the group models to already created group
        node->DeleteAttribute("models");
        wxString groups = wxJoin(newGroupModels, ',');
        node->AddAttribute("models", groups);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CreateModelGroupFromSelected");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "CreateModelGroupFromSelected", nullptr, nullptr, name.ToStdString());

        // we don't needs these, when group is selected after work is finished OnSelectionChange is fired and takes care of these
        //xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "CreateModelGroupFromSelected", nullptr, nullptr, name.ToStdString());
        //xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CreateModelGroupFromSelected", nullptr, nullptr, name.ToStdString());
    }
}

void LayoutPanel::AddSelectedToExistingGroups() {

    wxArrayString selectedModels;

    // add selected groups
    for (const auto& group : selectedTreeGroups) {
        selectedModels.Add(TreeListViewModels->GetItemText(group));
    }

    // add selected models
    for (const auto& model : selectedTreeModels) {
        selectedModels.Add(TreeListViewModels->GetItemText(model));
    }

    // add selected submodels
    for (const auto& submodel : selectedTreeSubModels) {
        ModelTreeData *submodelData = (ModelTreeData*)TreeListViewModels->GetItemData(submodel);
        Model* subModel = ((submodelData != nullptr) ? submodelData->GetModel() : nullptr);

        if (subModel != nullptr) {
            selectedModels.Add(subModel->GetFullName());
        }
    }

    wxArrayString choices;

    for (const auto& it : xlights->AllModels) {
        // Don't add selected groups to choices
        if (it.second->GetDisplayAs() == "ModelGroup" && selectedModels.Index(it.second->GetName()) == -1) {
            choices.Add(it.second->GetName());
        }
    }

    // User has selected all available groups, let them know and bail
    if (choices.size() == 0) {
        std::string userMsg = "You have selected all available groups, there must be at least one group that is not selected and available to add selections to.";
        wxMessageDialog msgDlg(this, _(userMsg), _("No Available Groups To Add To"), wxOK | wxCENTRE);
        msgDlg.ShowModal();
        return;
    }

    wxMultiChoiceDialog dlg(this, "Select existing groups to add selections to", "Existing Group", choices);
    OptimiseDialogPosition(&dlg);

    std::string selectgroupName;
    bool reload = false;

    if (dlg.ShowModal() == wxID_OK) {
        for (auto const& idx : dlg.GetSelections()) {
            std::string groupName = choices.at(idx).ToStdString();

            Model* addToGroup = xlights->GetModel(groupName);

            if (addToGroup != nullptr) {
                wxXmlNode* node = addToGroup->GetModelXml();
                wxArrayString groupModels = wxSplit(node->GetAttribute("models", ""), ',');

                bool groupChanged = false;
                for (const auto& selModel : selectedModels) {
                    // only add if model doesn't already exist in group
                    if (groupModels.Index(selModel) == -1) {
                        groupChanged = true;
                        groupModels.Add(selModel);
                    }
                }

                if (groupChanged) {
                    wxString xmlModels = wxJoin(groupModels, ',');
                    node->DeleteAttribute("models");
                    node->AddAttribute("models", xmlModels);
                    reload = true;
                    selectgroupName = groupName;
                }
            }
        }

        if (reload) {
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::AddSelectedToExistingGroups");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::AddSelectedToExistingGroups");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::AddSelectedToExistingGroups", nullptr, nullptr, selectgroupName);
        }
    }
}

class xlImageProperty : public wxImageFileProperty {
public:
    xlImageProperty(const wxString& label,
                    const wxString& name,
                    const wxString& value,
                    const wxImage *img)
        : wxImageFileProperty(label, name, ""), lastFileName(value)
    {

        SetValueFromString(value);
        if (img != nullptr) {
            setImage(*img);
        }
    }
    virtual ~xlImageProperty() {}

    virtual void OnSetValue() override {
        wxFileProperty::OnSetValue();
        wxFileName fn = GetFileName();
        ObtainAccessToURL(fn.GetFullPath());
        if (fn != lastFileName) {
            lastFileName = fn;
            if (FileExists(fn)) {
                setImage(wxImage(fn.GetFullPath()));
            } else {
                setImage(wxImage());
            }
        }
    }
    
    void setImage(const wxImage &img) {
#if (wxRELEASE_NUMBER > 6) || (wxMINOR_VERSION >= 2)
        m_image = img;
#else
        if (img.IsOk()) {
            m_pImage = new wxImage(img);
        } else {
            delete m_pImage;
            m_pImage = nullptr;
        }
#endif
    }

private:
    wxFileName lastFileName;
};

void LayoutPanel::UnSelectAllModels(bool addBkgProps)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    highlightedBaseObject = nullptr;
    selectedBaseObject = nullptr;
    selectionLatched = false;
    selectedPrimaryTreeItem = nullptr;
    selectedTreeGroups.clear();
    selectedTreeModels.clear();
    selectedTreeSubModels.clear();

    // process all models
    for (const auto& m : modelPreview->GetModels()) {
        if (!xlights->AllModels.IsModelValid(m) && m != _newModel) {
            logger_base.error("Really strange ... unselect all models returned an invalid model pointer");
        }
        else {
            xlights->AddTraceMessage("LayoutPanel::UnSelectAllModels Model " + m->GetName());
            if (m != nullptr) {
                m->Selected = false;
                m->Highlighted = false;
                m->GroupSelected = false;
                m->SelectHandle(-1);
                m->GetBaseObjectScreenLocation().SetActiveHandle(-1);

                for (const auto& sm : m->GetSubModels()) {
                    sm->Selected = false;
                    sm->Highlighted = false;
                    sm->GroupSelected = false;
                }
            }
            else {
                logger_base.error("Really strange ... unselect all models returned a null model pointer");
            }
        }
    }

    // process all view objects
    for (const auto& it : xlights->AllObjects) {
        ViewObject* view_object = it.second;
        xlights->AddTraceMessage("LayoutPanel::UnSelectAllModels Object " + view_object->GetName());
        if (view_object != nullptr) {
            view_object->Selected = false;
            view_object->Highlighted = false;
            view_object->GroupSelected = false;
            view_object->SelectHandle(-1);
            view_object->GetBaseObjectScreenLocation().SetActiveHandle(-1);
        }
        else {
            logger_base.error("Really strange ... unselect all models returned a null view object pointer");
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::UnselectAllModels");

    if (!updatingProperty && addBkgProps) {
        showBackgroundProperties();
    }
}

void LayoutPanel::showBackgroundProperties()
{
    propertyEditor->Freeze();
    clearPropGrid();

    GetBackgroundImageForSelectedPreview();       // don't need return value....just let it set local variable previewBackgroundFile
    GetBackgroundScaledForSelectedPreview();      // don't need return value....just let it set local variable previewBackgroundScaled
    GetBackgroundBrightnessForSelectedPreview();  // don't need return value....just let it set local variable previewBackgroundBrightness
    GetBackgroundAlphaForSelectedPreview();       // don't need return value....just let it set local variable previewBackgroundBrightness

    if (backgroundFile != previewBackgroundFile) {
        delete background;
        background = nullptr;
    }

    if (background == nullptr) {
        backgroundFile = previewBackgroundFile;
        if (backgroundFile != "" && FileExists(backgroundFile) && wxIsReadable(backgroundFile)) {
            background = new wxImage(backgroundFile);
        }
    }
    wxPGProperty* p = propertyEditor->Append(new xlImageProperty("Background Image",
        "BkgImage",
        previewBackgroundFile,
        background));
    p->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg|All files (*.*)|*.*");
    propertyEditor->Append(new wxBoolProperty("Fill", "BkgFill", previewBackgroundScaled))->SetAttribute("UseCheckbox", 1);
    if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
        wxPGProperty* prop = propertyEditor->Append(new wxUIntProperty("Width", "BkgSizeWidth", modelPreview->GetVirtualCanvasWidth()));
        prop->SetAttribute("Min", 0);
        prop->SetAttribute("Max", 16384);
        prop->SetEditor("SpinCtrl");
        prop = propertyEditor->Append(new wxUIntProperty("Height", "BkgSizeHeight", modelPreview->GetVirtualCanvasHeight()));
        prop->SetAttribute("Min", 0);
        prop->SetAttribute("Max", 16384);
        prop->SetEditor("SpinCtrl");
    }
    wxPGProperty* prop = propertyEditor->Append(new wxUIntProperty("Brightness", "BkgBrightness", previewBackgroundBrightness));
    prop->SetAttribute("Min", 0);
    prop->SetAttribute("Max", 100);
    prop->SetEditor("SpinCtrl");

    prop = propertyEditor->Append(new wxUIntProperty("Transparency", "BkgTransparency", 100 - previewBackgroundAlpha));
    prop->SetAttribute("Min", 0);
    prop->SetAttribute("Max", 100);
    prop->SetEditor("SpinCtrl");

    prop = propertyEditor->Append(new wxBoolProperty("2D Bounding Box", "BoundingBox", xlights->GetDisplay2DBoundingBox()));
    prop->SetAttribute("UseCheckbox", true);

    prop = propertyEditor->Append(new wxBoolProperty("2D Grid", "2DGrid", xlights->GetDisplay2DGrid()));
    prop->SetAttribute("UseCheckbox", true);

    prop = propertyEditor->Append(new wxUIntProperty("2D Grid Spacing", "2DGridSpacing", xlights->GetDisplay2DGridSpacing()));
    prop->SetAttribute("Min", 5);
    prop->SetAttribute("Max", 200);
    prop->SetEditor("SpinCtrl");

    prop = propertyEditor->Append(new wxBoolProperty("X0 Is Center", "2DXZeroIsCenter", xlights->GetDisplay2DCenter0()));
    prop->SetAttribute("UseCheckbox", true);

    propertyEditor->Thaw();
}

void LayoutPanel::SelectAllModels()
{
    highlightedBaseObject = nullptr;
    selectedBaseObject = nullptr;
    selectionLatched = false;

    if (editing_models) {
        auto& models = modelPreview->GetModels();
        for (size_t i = 0; i < models.size(); i++)
        {
            Model* m = models[i];

            SelectModelInTree(m);
        }
    }
    else {
        for (const auto& it : xlights->AllObjects) {
            ViewObject* view_object = it.second;

            if (selectedBaseObject == nullptr)
            {
                SelectBaseObject(view_object);
            }

            view_object->Selected = false;
            view_object->Highlighted = true;
            view_object->GroupSelected = true;
            view_object->SelectHandle(-1);
            view_object->GetBaseObjectScreenLocation().SetActiveHandle(-1);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::SelectAllModels");
}

void LayoutPanel::SetupPropGrid(BaseObject *base_object) {

    if (base_object == nullptr || propertyEditor == nullptr) return;
    if (dynamic_cast<ModelGroup*>(base_object) != nullptr) {
        //groups don't use the property grid
        return;
    }

    if (propertyEditor->GetSelection() != nullptr) {
        _lastSelProp = propertyEditor->GetSelection()->GetName();
    }

    auto frozen = propertyEditor->IsFrozen();
    if (!frozen) propertyEditor->Freeze();
    clearPropGrid();

    if( editing_models ) {
        auto p = propertyEditor->Append(new wxStringProperty("Name", "ModelName", base_object->name));
        if (dynamic_cast<SubModel*>(base_object) != nullptr) {
            p->ChangeFlag(wxPG_PROP_READONLY, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("SubModel names cannot be changed here.");
        }
    } else {
        propertyEditor->Append(new wxStringProperty("Name", "ObjectName", base_object->name));
    }

    base_object->AddProperties(propertyEditor, xlights->GetOutputManager());
    if (is_3d)
    {
        base_object->EnableLayoutGroupProperty(propertyEditor, false);
    }

    if (dynamic_cast<SubModel*>(base_object) == nullptr) {

        wxPGProperty* p2;

        if (RulerObject::GetRuler() != nullptr) {
            p2 = propertyEditor->Append(new wxPropertyCategory("Dimensions", "Dimensions"));

            base_object->AddDimensionProperties(propertyEditor);
            if (!dimensionsVisible) {
                propertyEditor->Collapse(p2);
            }

            if (p2->GetChildCount() == 0) {
                p2->Hide(true);
            }
        }

        p2 = propertyEditor->Append(new wxPropertyCategory("Size/Location", "ModelSize"));
        base_object->AddSizeLocationProperties(propertyEditor);
        if (!sizeVisible) {
            propertyEditor->Collapse(p2);
        }
        if (!appearanceVisible) {
            wxPGProperty *prop = propertyEditor->GetPropertyByName("ModelAppearance");
            if (prop != nullptr) {
                propertyEditor->Collapse(prop);
            }
        }
        if (!stringPropsVisible) {
            wxPGProperty *prop = propertyEditor->GetPropertyByName("ModelStringProperties");
            if (prop != nullptr) {
                propertyEditor->Collapse(prop);
            }
        }
        if (!controllerConnectionVisible) {
            wxPGProperty *prop = propertyEditor->GetPropertyByName("ModelControllerConnectionProperties");
            if (prop != nullptr) {
                propertyEditor->Collapse(prop);
            }
        }
        if (!layersVisible) {
            wxPGProperty* prop = propertyEditor->GetPropertyByName("Layers");
            if (prop != nullptr) {
                propertyEditor->Collapse(prop);
            }
        }
    }
    if (!frozen) propertyEditor->Thaw();

    if (_lastSelProp != "") {
        auto p = propertyEditor->GetPropertyByName(_lastSelProp);
        if (p != nullptr) propertyEditor->EnsureVisible(p);
    }
}

void LayoutPanel::SelectBaseObject3D()
{
    if (is_3d) {
        // latch center handle immediately
        if (selectedBaseObject != nullptr) {
            if (editing_models) {
                Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
                // I think the selected model might not be a model in some undo situations
                if (selectedModel != nullptr) {
                    selectedModel->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
                    selectedModel->GetBaseObjectScreenLocation().SetActiveAxis(ModelScreenLocation::MSLAXIS::NO_AXIS);
                }
            }
            else {
                ViewObject* selectedViewObject = dynamic_cast<ViewObject*>(selectedBaseObject);
                // I think the selected model might not be a view object in some undo situations
                if (selectedViewObject != nullptr) {
                    selectedViewObject->GetObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
                    selectedViewObject->GetObjectScreenLocation().SetActiveAxis(ModelScreenLocation::MSLAXIS::NO_AXIS);
                }
            }
            highlightedBaseObject = selectedBaseObject;
            selectionLatched = true;
        }
    }
}

void LayoutPanel::SelectBaseObject(const std::string & name, bool highlight_tree)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if( editing_models ) {
        Model *m = xlights->AllModels[name];
        if (m == nullptr)
        {
            // Hmmm ... model doesnt exist ... maybe it is the additional model
            if (modelPreview->GetAdditionalModel() != nullptr && modelPreview->GetAdditionalModel()->GetName() == name)
            {
                m = modelPreview->GetAdditionalModel();
            }
            else
            {
                logger_base.warn("LayoutPanel:SelectBaseObject Unable to select model '%s'.", (const char*)name.c_str());
            }
        }
        if (m != selectedBaseObject)
        {
            SelectModelInTree(m);
        }
    } else {
        ViewObject *v = xlights->AllObjects[name];
        if (v == nullptr)
        {
            logger_base.warn("LayoutPanel:SelectBaseObject Unable to select object '%s'.", (const char *)name.c_str());
        }
        if (v != selectedBaseObject)
        {
            for (const auto& it : xlights->AllObjects) {
                ViewObject* view_object = it.second;
                view_object->Selected = false;
                view_object->Highlighted = false;
                view_object->GroupSelected = false;
                view_object->SelectHandle(-1);
                view_object->GetBaseObjectScreenLocation().SetActiveHandle(-1);
            }
            SelectViewObject(v, highlight_tree);
        }
    }
}

void LayoutPanel::SelectBaseObject(BaseObject *obj, bool highlight_tree)
{
    if( editing_models ) {
        SelectBaseObjectInTree(obj);
    } else {
        SelectViewObject(dynamic_cast<ViewObject*>(obj), highlight_tree);
    }
}

void LayoutPanel::SelectModel(const std::string & name, bool highlight_tree)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    xlights->AddTraceMessage("LayoutPanel::SelectModel: " + name);
    Model *m = xlights->AllModels[name];
    if (m == nullptr)
    {
        logger_base.warn("LayoutPanel:SelectModel Unable to select model '%s'.", (const char *)name.c_str());
    }
    else {
        SelectModelInTree(m);
    }
}

void LayoutPanel::SelectModelGroupModels(ModelGroup* m, std::list<ModelGroup*>& processed)
{
    processed.push_back(m);
    for (const auto& it : m->Models())
    {
        if (it->GetDisplayAs() == "ModelGroup")
        {
            if (std::find(processed.begin(), processed.end(), it) == processed.end())
            {
                SelectModelGroupModels(dynamic_cast<ModelGroup*>(it), processed);
            }
        }
        else
        {
            SelectModelInTree(m);
        }
    }
}

void LayoutPanel::SelectModel(Model *m, bool highlight_tree) {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // TODO need to strip out extra logging once I know for sure what is going on
    if (modelPreview == nullptr) logger_base.crit("LayoutPanel::SelectModel modelPreview is nullptr ... this is going to crash.");

    SelectModelInTree(m);
    //SelectBaseObject3D();
}

void LayoutPanel::SelectViewObject(ViewObject *v, bool highlight_tree) {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // TODO need to strip out extra logging once I know for sure what is going on
    if (modelPreview == nullptr) logger_base.crit("LayoutPanel::SelectViewObject modelPreview is nullptr ... this is going to crash.");

    bool changed = false;
    if (v != selectedBaseObject)
    {
        changed = true;
    }

    modelPreview->SetFocus();

    if (v != nullptr) {
        v->Selected = true;

        if( highlight_tree ) {
            objects_panel->HighlightObject(v);
        }
        if (changed) {
            SetupPropGrid(v);
        }
    } else {
        propertyEditor->Freeze();
        clearPropGrid();
        propertyEditor->Thaw();
    }

    selectedBaseObject = v;
    if (selectedBaseObject != nullptr) {
        selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
        selectionLatched = true;
    }
    SelectBaseObject3D();

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::SelectViewObject");
}

bool LayoutPanel::Is3d() const
{
    return CheckBox_3D->IsChecked();
}

void LayoutPanel::Set3d(bool is3d)
{
    if (CheckBox_3D->IsChecked() != is3d)
    {
        CheckBox_3D->SetValue(is3d);
        wxCommandEvent e;
        OnCheckBox_3DClick(e);
    }
}

std::string LayoutPanel::GetSelectedModelName() const
{
    if (selectedBaseObject == nullptr) return "";
    return selectedBaseObject->GetName();
}

void LayoutPanel::OnCheckBoxOverlapClick(wxCommandEvent& event)
{
    if (CheckBoxOverlap->GetValue() == false) {
        for (wxTreeListItem item = TreeListViewModels->GetFirstItem();
            item.IsOk();
            item = TreeListViewModels->GetNextSibling(item)) {
            ModelTreeData* data = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
            Model* model = data != nullptr ? data->GetModel() : nullptr;

            if (model != nullptr) {
                if (model->GetDisplayAs() != "ModelGroup") {
                    model->Overlapping = false;
                }
            }
        }
    }
}

void LayoutPanel::SaveEffects()
{
    // update xml with offsets and scale
    for (const auto& it : modelPreview->GetModels()) {
        if (xlights->AllModels.IsModelValid(it) ||
            IsNewModel(it)) { // this IsModelValid should not be necessary but we are getting crashes due to invalid models
            it->UpdateXmlWithScale();
        }
    }
    for (const auto& it : xlights->AllObjects) {
        ViewObject* view_object = it.second;
        view_object->UpdateXmlWithScale();
    }
    xlights->SaveEffectsFile();
    xlights->SetStatusText(_("Preview layout saved"));
    SetDirtyHiLight(false);
}

void LayoutPanel::OnButtonSavePreviewClick(wxCommandEvent& event)
{
    // if we have auto layout make sure everything is up to date ... and update zcpp files
    if (xlights->GetOutputManager()->GetAutoLayoutControllerNames().size() > 0) {
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnButtonSavePreviewClick");
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "LayoutPanel::OnButtonSavePreviewClick");
        xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "LayoutPanel::OnButtonSavePreviewClick");
    }

    SaveEffects();
    if (xlights->IsControllersAndLayoutTabSaveLinked()) {
        xlights->SaveNetworksFile();
        xlights->UpdateLayoutSave(); // SaveEffects tried to do this, but if the saves are linked it is marked dirty til nets are saved.
    }
}

int LayoutPanel::ModelListComparator::SortElementsFunction(wxTreeListCtrl* treelist, wxTreeListItem item1, wxTreeListItem item2, unsigned sortColumn)
{
    unsigned col;
    bool ascending;
    treelist->GetSortColumn(&col, &ascending);

    ModelTreeData* data1 = dynamic_cast<ModelTreeData*>(treelist->GetItemData(item1));
    ModelTreeData* data2 = dynamic_cast<ModelTreeData*>(treelist->GetItemData(item2));

    Model* a = data1->GetModel();
    Model* b = data2->GetModel();

    if (a == nullptr || b == nullptr) {
        return 0;
    }

    if (a->GetDisplayAs() == "ModelGroup") {
        if (b->GetDisplayAs() == "ModelGroup") {
            return NumberAwareStringCompare(a->name, b->name);
        }
        else {
            if (ascending)
                return -1;
            else
                return 1;
        }
    }
    else if (b->GetDisplayAs() == "ModelGroup") {
        if (ascending)
            return 1;
        else
            return -1;
    }

    auto colobj = treelist->GetDataView()->GetColumn(sortColumn);

    if (colobj->GetTitle() == STARTCHANCOLNAME) {
        int ia = data1->startingChannel;
        int ib = data2->startingChannel;
        if (ia > ib)
            return 1;
        if (ia < ib)
            return -1;
        return NumberAwareStringCompare(a->name, b->name);
    }
    else if (colobj->GetTitle() == ENDCHANCOLNAME) {
        int ia = data1->endingChannel;
        int ib = data2->endingChannel;
        if (ia > ib)
            return 1;
        if (ia < ib)
            return -1;
        return NumberAwareStringCompare(a->name, b->name);
    }
    else if (colobj->GetTitle() == CONTCONNCOLNAME) {
        int32_t sc;
        // group controllers first
        Output* oa = xlights->GetOutputManager()->GetOutput(data1->startingChannel, sc);
        std::string sna = "";
        if (oa != nullptr) sna = oa->GetSortName();
        Output* ob = xlights->GetOutputManager()->GetOutput(data2->startingChannel, sc);
        std::string snb = "";
        if (ob != nullptr) snb = ob->GetSortName();

        if (sna == snb) {
            // then protocol
            std::string pra = data1->GetModel()->GetControllerProtocol();
            std::string prb = data2->GetModel()->GetControllerProtocol();
            if (pra > prb)
                return 1;
            if (pra < prb)
                return -1;

            // then start port
            int pa = data1->GetModel()->GetControllerPort();
            int pb = data2->GetModel()->GetControllerPort();

            if (pa > pb)
                return 1;
            if (pa < pb)
                return -1;

            // then start channel
            if (data1->startingChannel > data2->startingChannel)
                return 1;
            if (data1->startingChannel < data2->startingChannel)
                return -1;
            return 0;
        }
        else {
            return NumberAwareStringCompare(sna, snb);
        }
    }

    // Dont sort things with parents
    auto parent1 = treelist->GetItemParent(item1);
    auto parent2 = treelist->GetItemParent(item2);
    auto root = treelist->GetRootItem();
    if ((parent1 != root || parent2 != root) && parent1 == parent2) {
        int ia = data1->nativeOrder;
        int ib = data2->nativeOrder;
        if (ia > ib) {
            if (ascending) {
                return 1;
            }
            return -1;
        }
        if (ia < ib) {
            if (ascending) {
                return -1;
            }
            return 1;
        }
    }

    return NumberAwareStringCompare(a->name, b->name);
}

int LayoutPanel::ModelListComparator::Compare(wxTreeListCtrl* treelist, unsigned column, wxTreeListItem first, wxTreeListItem second)
{
    return SortElementsFunction(treelist, first, second, column);
}

void LayoutPanel::GetMouseLocation(int x, int y, glm::vec3& ray_origin, glm::vec3& ray_direction)
{
    VectorMath::ScreenPosToWorldRay(
        x, modelPreview->getHeight() - y,
        modelPreview->getWidth(), modelPreview->getHeight(),
        modelPreview->GetProjViewMatrix(),
        ray_origin,
        ray_direction
    );
}

int LayoutPanel::FindModelsClicked(int x, int y, std::vector<int>& found)
{
    glm::vec3 ray_origin;
    glm::vec3 ray_direction;
    GetMouseLocation(x, y, ray_origin, ray_direction);

    for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
        // This should not be necessary but i see enough crashes to think it is worthwhile
        if (xlights->AllModels.IsModelValid(modelPreview->GetModels()[i])) {
            if (modelPreview->GetModels()[i]->HitTest(modelPreview, ray_origin, ray_direction)) {
                found.push_back(i);
            }
        }
    }
    return found.size();
}

Model* LayoutPanel::SelectSingleModel(int x, int y)
{
    UnSelectAllModelsInTree();
    std::vector<int> found;
    int modelCount = FindModelsClicked(x, y, found);

    if (modelCount == 0) {
        return nullptr;
    }
    else if (modelCount == 1) {
        mHitTestNextSelectModelIndex = 0;
        return modelPreview->GetModels()[found[0]];
    }
    else if (modelCount > 1) {
        for (int i = 0; i < modelCount; i++) {
            if (mHitTestNextSelectModelIndex == i) {
                mHitTestNextSelectModelIndex += 1;
                mHitTestNextSelectModelIndex %= modelCount;
                return modelPreview->GetModels()[found[i]];
            }
        }
    }
    return nullptr;
}

void LayoutPanel::SelectAllInBoundingRect(bool models_and_objects)
{
    if (editing_models || models_and_objects) {
        int count = 0;
        for (const auto& it : modelPreview->GetModels()) {
            if (xlights->AllModels.IsModelValid(it) || it == _newModel) {
                if (it->IsContained(modelPreview, m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y)) {
                    SelectModelInTree(it);
                    count++;
                }
            }
        }
        if (count > 1) showBackgroundProperties();
    }
    if (!editing_models || models_and_objects) {
        for (const auto& it : xlights->AllObjects) {
            ViewObject* view_object = it.second;
            {
                if (view_object->IsContained(modelPreview, m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y)) {
                    // if we dont have a selected model make the first one we find the selected model so alignment etc works
                    if (selectedBaseObject == nullptr) {
                        SelectBaseObject(view_object->GetName(), false);
                    }
                    view_object->GroupSelected = true;
                }
            }
        }
    }

    modelPreview->SetFocus();
}

void LayoutPanel::HighlightAllInBoundingRect(bool models_and_objects)
{
    if (editing_models || models_and_objects) {
        for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
        {
            if (modelPreview->GetModels()[i]->IsContained(modelPreview, m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y)) {
                modelPreview->GetModels()[i]->Highlighted = true;
            }
            else if (!modelPreview->GetModels()[i]->Selected &&
                !modelPreview->GetModels()[i]->GroupSelected) {
                modelPreview->GetModels()[i]->Highlighted = false;
            }
        }
    }
    if (!editing_models || models_and_objects) {
        for (const auto& it : xlights->AllObjects) {
            ViewObject* view_object = it.second;
            if (view_object->GetBaseObjectScreenLocation().IsContained(modelPreview, m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y)) {
                view_object->Highlighted = true;
            }
            else if (!view_object->Selected &&
                !view_object->GroupSelected) {
                view_object->Highlighted = false;
            }
        }
    }
}

bool LayoutPanel::SelectMultipleModels(int x,int y)
{
    std::vector<int> found;
    int modelCount = FindModelsClicked(x, y, found);
    if (modelCount == 0)
        return false;

    propertyEditor->Freeze();
    clearPropGrid();
    propertyEditor->Thaw();
    bool mmWorkRequired = false;
    Model* clickedModel = modelPreview->GetModels()[found[0]];

    if (clickedModel->Selected)
    {
        UnSelectModelInTree(clickedModel);
    }
    else if (clickedModel->GroupSelected)
    {
        clickedModel->GroupSelected = false;
        clickedModel->Selected = true;
        if (selectedBaseObject != nullptr) {
            selectedBaseObject->GroupSelected = true;
            selectedBaseObject->Selected = false;
            selectedBaseObject->SelectHandle(-1);
            selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(-1);
        }

        selectedBaseObject = clickedModel;
        highlightedBaseObject = selectedBaseObject;
        selectedBaseObject->SelectHandle(-1);
        selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
        mmWorkRequired = true;
    }
    else
    {
        SelectModelInTree(clickedModel);
    }

    if (mmWorkRequired) {
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::SelectMultipleModels");
    }

    return true;
}

void LayoutPanel::SetMouseStateForModels(bool value)
{
    if ( mouse_state_set != value) {
        for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
            modelPreview->GetModels()[i]->GetModelScreenLocation().MouseDown(value);
        }
        mouse_state_set = value;
    }
}

void LayoutPanel::SetSelectedModelToGroupSelected()
{
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++) {
        if(modelPreview->GetModels()[i]->Selected) {
            modelPreview->GetModels()[i]->Selected = false;
            modelPreview->GetModels()[i]->GroupSelected = true;
        }
    }
}

void LayoutPanel::OnPreviewLeftDClick(wxMouseEvent& event)
{
    if (!event.ControlDown()) {
        if (editing_models) {
            UnSelectAllModelsInTree();
        }
        else {
            UnSelectAllModels();
        }
        m_mouse_down = false;
        SetMouseStateForModels(m_mouse_down);
    }
}

void LayoutPanel::ProcessLeftMouseClick3D(wxMouseEvent& event)
{
    m_moving_handle = false;
    // don't mark mouse down if a selection is being made
    if (highlightedBaseObject != nullptr) {
        if (selectionLatched) {
            m_over_handle = -1;
            if (selectedBaseObject != nullptr) {
                glm::vec3 ray_origin;
                glm::vec3 ray_direction;
                GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
                selectedBaseObject->GetBaseObjectScreenLocation().CheckIfOverHandles3D(ray_origin, ray_direction, m_over_handle, modelPreview->GetCameraZoomForHandles(), modelPreview->GetHandleScale());
            }
            if (m_over_handle != -1) {
                if ((m_over_handle & HANDLE_AXIS) > 0) {
                    // an axis was selected
                    if (selectedBaseObject != nullptr) {
                        int active_handle = selectedBaseObject->GetBaseObjectScreenLocation().GetActiveHandle();
                        selectedBaseObject->GetBaseObjectScreenLocation().SetActiveAxis((ModelScreenLocation::MSLAXIS)(m_over_handle & 0xff));
                        selectedBaseObject->GetBaseObjectScreenLocation().MouseOverHandle(-1);
                        bool z_scale = selectedBaseObject->GetBaseObjectScreenLocation().GetSupportsZScaling();
                        // this is designed to pretend the control and shift keys are down when creating models to
                        // make them scale from the desired handle depending on model type
                        xlights->AbortRender();
                        auto pos = selectedBaseObject->MoveHandle3D(modelPreview, active_handle, event.ShiftDown() || creating_model, event.ControlDown() || (creating_model && z_scale), event.GetX(), event.GetY(), true, z_scale);
                        xlights->SetStatusText(wxString::Format("x=%.2f y=%.2f z=%.2f %s", pos.x, pos.y, pos.z, selectedBaseObject->GetDimension()));
                        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::ProcessLeftMouseClick3D");
                        m_moving_handle = true;
                        m_mouse_down = true;
                        last_centerpos = selectedBaseObject->GetBaseObjectScreenLocation().GetCenterPosition();
                        last_worldrotate = selectedBaseObject->GetBaseObjectScreenLocation().GetRotationAngles();
                        last_worldscale = selectedBaseObject->GetBaseObjectScreenLocation().GetScaleMatrix();
                    }
                }
                else if ((m_over_handle & HANDLE_SEGMENT) > 0) {
                    // a segment was selected
                    if (selectedBaseObject != nullptr) {
                        selectedBaseObject->GetBaseObjectScreenLocation().SelectSegment(m_over_handle & 0xFFF);
                        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::ProcessLeftMouseClick3D");
                    }
                }
                else {
                    if (selectedBaseObject->GetBaseObjectScreenLocation().GetActiveHandle() == m_over_handle) {
                        selectedBaseObject->GetBaseObjectScreenLocation().AdvanceAxisTool();
                    }
                    selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(m_over_handle);
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::ProcessLeftMouseClick3D");
                }
            }
            else {
                m_mouse_down = true;
            }
        }
        else {
            if (editing_models) {
                SelectBaseObjectInTree(highlightedBaseObject);
            } else {
                SelectBaseObject(highlightedBaseObject);
                selectionLatched = true;
                // latch center handle immediately
                selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::ProcessLeftMouseClick3D");
            }
        }
    }
    else {
        m_mouse_down = true;
    }

    m_last_mouse_x = event.GetX();
    m_last_mouse_y = event.GetY();

    if (selectedButton != nullptr) {
        //create a new model
        int wi, ht;
        modelPreview->GetVirtualCanvasSize(wi, ht);
        m_creating_bound_rect = false;
        const std::string& model_type = selectedButton->GetModelType();
        if (model_type != "DMX") {
            _newModel = CreateNewModel(model_type);
            _newModel->SetLayoutGroup(currentLayoutGroup);
        }
        else if (model_type == "DMX" && selectedDmxModelType != "") {
            _newModel = CreateNewModel(selectedDmxModelType);
            _newModel->SetLayoutGroup(currentLayoutGroup);
        }

        if (_newModel != nullptr) {
            m_moving_handle = true;
            if (model_type == "Poly Line") {
                m_polyline_active = true;
            }
            UnSelectAllModels();
            _newModel->Selected = true;
            _newModel->GetBaseObjectScreenLocation().SetActiveHandle(_newModel->GetBaseObjectScreenLocation().GetDefaultHandle());
            _newModel->GetBaseObjectScreenLocation().SetAxisTool(_newModel->GetBaseObjectScreenLocation().GetDefaultTool());
            selectionLatched = true;
            highlightedBaseObject = _newModel;
            selectedBaseObject = _newModel;
            // need to ensure the model stays selected
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::ProcessLeftMouseClick3D", _newModel, nullptr, _newModel->GetName());
            creating_model = true;
            if (wi > 0 && ht > 0)
            {
                modelPreview->SetCursor(_newModel->InitializeLocation(m_over_handle, event.GetX(), event.GetY(), modelPreview));
                _newModel->UpdateXmlWithScale();
            }
            bool z_scale = selectedBaseObject->GetBaseObjectScreenLocation().GetSupportsZScaling();
            // this is designed to pretend the control and shift keys are down when creating models to
            // make them scale from the desired handle depending on model type
            xlights->AbortRender();
            auto pos = selectedBaseObject->MoveHandle3D(modelPreview, selectedBaseObject->GetBaseObjectScreenLocation().GetDefaultHandle(), event.ShiftDown() || creating_model, event.ControlDown() || (creating_model && z_scale), event.GetX(), event.GetY(), true, z_scale);
            xlights->SetStatusText(wxString::Format("x=%.2f y=%.2f z=%.2f %s", pos.x, pos.y, pos.z, selectedBaseObject->GetDimension()));
            lastModelName = _newModel->name;
            modelPreview->SetAdditionalModel(_newModel);
        }
    }

    ShowPropGrid(true);
    modelPreview->SetFocus();

    if (event.ControlDown() && !event.ShiftDown())
    {
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
        // if control key is down check to see if we are highlighting another model for group selection
        BaseObject* which_object = nullptr;
        float distance = 1000000000.0f;
        float intersection_distance = 1000000000.0f;
        if (editing_models) {
            for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
            {
                if (modelPreview->GetModels()[i]->GetBaseObjectScreenLocation().HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                    if (intersection_distance < distance) {
                        distance = intersection_distance;
                        which_object = modelPreview->GetModels()[i];
                    }
                }
            }
        }
        else {
            for (const auto& it : xlights->AllObjects) {
                ViewObject* view_object = it.second;
                if (view_object->GetBaseObjectScreenLocation().HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                    if (intersection_distance < distance) {
                        distance = intersection_distance;
                        which_object = view_object;
                    }
                }
            }
        }
        if (which_object != nullptr)
        {
            bool mmWorkRequired = false;
            if (which_object->Highlighted) {
                if (!which_object->GroupSelected && !which_object->Selected) {
                    if (editing_models) {
                        SelectBaseObjectInTree(which_object);
                    } else {
                        which_object->GroupSelected = true;
                        mmWorkRequired = true;
                    }
                }
                else if (which_object->GroupSelected) {
                    which_object->GroupSelected = false;
                    which_object->Selected = true;
                    if (selectedBaseObject != nullptr) {
                        selectedBaseObject->GroupSelected = true;
                        selectedBaseObject->Selected = false;
                        selectedBaseObject->SelectHandle(-1);
                        selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(-1);
                    }
                    selectedBaseObject = which_object;
                    highlightedBaseObject = selectedBaseObject;
                    selectedBaseObject->SelectHandle(-1);
                    selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
                    mmWorkRequired = true;
                }
                else if (which_object->Selected) {
                    if (editing_models) {
                        UnSelectBaseObjectInTree(which_object);
                    } else {
                        which_object->Selected = false;
                        which_object->Highlighted = false;
                        which_object->SelectHandle(-1);
                        which_object->GetBaseObjectScreenLocation().SetActiveHandle(-1);
                        selectedBaseObject = nullptr;

                        for (const auto& it : xlights->AllObjects) {
                            ViewObject* view_object = it.second;
                            if (view_object->GroupSelected) {
                                selectedBaseObject = view_object;
                                break;
                            }
                        }
                        if (selectedBaseObject != nullptr) {
                            selectedBaseObject->GroupSelected = false;
                            selectedBaseObject->Selected = true;
                            selectedBaseObject->SelectHandle(-1);
                            selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
                            highlightedBaseObject = selectedBaseObject;
                        }
                        mmWorkRequired = true;
                    }
                }

                if (mmWorkRequired) {
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::ProcessLeftMouseClick3D");
                }
            }
        }
    }
    else if (event.ShiftDown())
    {
        m_creating_bound_rect = true;
        m_bound_start_x = event.GetX();
        m_bound_start_y = event.GetY();
        m_bound_end_x = m_bound_start_x;
        m_bound_end_y = m_bound_start_y;
    }
    else {
        m_creating_bound_rect = false;
        m_dragging = false;
    }
}

void LayoutPanel::OnPreviewLeftDown(wxMouseEvent& event)
{
    if (m_polyline_active)
    {
        Model *m = _newModel;
        m->AddHandle(modelPreview, event.GetX(), event.GetY());
        m->UpdateXmlWithScale();
        m->InitModel();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPreviewLeftDown");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnPreviewLeftDown");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewLeftDown");
        m_over_handle++;
        int handle = m->GetBaseObjectScreenLocation().GetActiveHandle();
        handle++;
        m->GetBaseObjectScreenLocation().SetActiveHandle(handle);
        return;
    }

    ShowPropGrid(true);
    modelPreview->SetFocus();

    if (is_3d) {
        ProcessLeftMouseClick3D(event);
        return;
    }

    if (event.ControlDown())
    {
        SelectMultipleModels(event.GetX(), event.GetY());
        m_dragging = true;
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
    }
    else if (event.ShiftDown())
    {
        m_creating_bound_rect = true;
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
        m_bound_start_x = ray_origin.x;
        m_bound_start_y = ray_origin.y;
        m_bound_end_x = m_bound_start_x;
        m_bound_end_y = m_bound_start_y;
    }
    else if (event.AltDown())
    {
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
        m_wheel_down = true;
    }
    else if (m_over_handle != -1)
    {
        if ((m_over_handle & HANDLE_SEGMENT) > 0) {
            // a segment was selected
            if (selectedBaseObject != nullptr) {
                selectedBaseObject->GetBaseObjectScreenLocation().SelectSegment(m_over_handle & 0xFFF);
                modelPreview->SetCursor(wxCURSOR_DEFAULT);
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewLeftDown");
            }
        }
        else {
            m_moving_handle = true;
            if (selectedBaseObject != nullptr) {
                selectedBaseObject->SelectHandle(m_over_handle);
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewLeftDown");
            }
        }
    }
    else if (selectedButton != nullptr)
    {
        //create a new model
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
        m_creating_bound_rect = false;
        const std::string& model_type = selectedButton->GetModelType();

        if (model_type != "DMX") {
            _newModel = CreateNewModel(model_type);
        }
        else if (model_type == "DMX" && selectedDmxModelType != "") {
            _newModel = CreateNewModel(selectedDmxModelType);
        }

        if (_newModel != nullptr) {
            m_moving_handle = true;
            _newModel->SetLayoutGroup(currentLayoutGroup);
            _newModel->SetControllerName(NO_CONTROLLER);
            if (model_type == "Poly Line") {
                m_polyline_active = true;
            }
            UnSelectAllModels();
            _newModel->Selected = true;
            modelPreview->SetCursor(_newModel->InitializeLocation(m_over_handle, event.GetX(), event.GetY(), modelPreview));
            _newModel->UpdateXmlWithScale();
            lastModelName = _newModel->name;
            modelPreview->SetAdditionalModel(_newModel);
        }
    }
    else
    {
        m_moving_handle = false;
        m_creating_bound_rect = false;

        if (!event.wxKeyboardState::ControlDown())
        {
            UnSelectAllModelsInTree();
        }

        Model* singleModel = SelectSingleModel(event.GetX(), event.GetY());
        if (singleModel != nullptr)
        {
            SelectModelInTree(singleModel);
            m_dragging = true;
            m_previous_mouse_x = event.GetX();
            m_previous_mouse_y = event.GetY();
            xlights->SetStatusText(wxString::Format("x=%d y=%d", m_previous_mouse_x, m_previous_mouse_y));
        }
        else
        {
            m_creating_bound_rect = true;
            glm::vec3 ray_origin;
            glm::vec3 ray_direction;
            GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
            m_bound_start_x = ray_origin.x;
            m_bound_start_y = ray_origin.y;
            m_bound_end_x = m_bound_start_x;
            m_bound_end_y = m_bound_start_y;
        }
    }
}

void LayoutPanel::OnPreviewLeftUp(wxMouseEvent& event)
{
    if (m_polyline_active) {
        m_mouse_down = false;
        SetMouseStateForModels(m_mouse_down);
        return;
    }

    if (m_wheel_down)
    {
        m_wheel_down = false;
        return;
    }

    if (is_3d && m_mouse_down) {
        if (selectedBaseObject != nullptr) {
            selectedBaseObject->GetBaseObjectScreenLocation().SetActiveAxis(ModelScreenLocation::MSLAXIS::NO_AXIS);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewLeftDown");
        }
        modelPreview->SetCameraView(0, 0, true);
    }

    m_mouse_down = false;
    SetMouseStateForModels(m_mouse_down);
    m_moving_handle = false;
    over_handle = NO_HANDLE;

    if (m_creating_bound_rect) {
        if (is_3d) {
            m_bound_end_x = event.GetX();
            m_bound_end_y = event.GetY();
        } else {
            glm::vec3 ray_origin;
            glm::vec3 ray_direction;
            GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
            m_bound_end_x = ray_origin.x;
            m_bound_end_y = ray_origin.y;
        }
        SelectAllInBoundingRect(event.ControlDown() && event.ShiftDown());
        m_creating_bound_rect = false;
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewLeftUp");
    }
    FinalizeModel();
}

void LayoutPanel::FinalizeModel()
{
    xlights->AddTraceMessage("In LayoutPanel::FinalizeModel");
    if (m_polyline_active && m_over_handle > 1) {
        Model *m = _newModel;
        if (m != nullptr)
        {
            xlights->AddTraceMessage("LayoutPanel::FinalizeModel Polyline deleting handle.");
            m->DeleteHandle(m_over_handle);

            auto plm = dynamic_cast<PolyLineModel*>(m);
            if (plm->GetNumHandles() < 2) {
                // If we end up with less than 2 points then we destroy the polyline
                highlightedBaseObject = nullptr;
                selectedBaseObject = nullptr;
                modelPreview->SetAdditionalModel(nullptr);
                delete _newModel;
                _newModel = nullptr;
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "FinalizeModel");
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "FinalizeModel");
            }
        }
    }
    m_moving_handle = false;
    m_dragging = false;
    m_polyline_active = false;
    creating_model = false;
    m_over_handle = NO_HANDLE;

    if (_newModel != nullptr) {
        xlights->AddTraceMessage("LayoutPanel::FinalizeModel New model is not null.");
        // cache the selected button as it may change during a download or some such event
        auto b = selectedButton;
        if (b != nullptr && (b->GetModelType() == "Import Custom" || b->GetModelType() == "Download"))
        {
            xlights->AddTraceMessage("LayoutPanel::FinalizeModel We were downloading or importing.");
            float min_x = (float)(_newModel->GetBaseObjectScreenLocation().GetLeft());
            float max_x = (float)(_newModel->GetBaseObjectScreenLocation().GetRight());
            float min_y = (float)(_newModel->GetBaseObjectScreenLocation().GetBottom());
            float max_y = (float)(_newModel->GetBaseObjectScreenLocation().GetTop());
            bool cancelled = false;
            auto pos = _newModel->GetBaseObjectScreenLocation().GetWorldPosition();

            wxProgressDialog* prog = nullptr;
            if (b->GetModelType() == "Download")
            {
                prog = new wxProgressDialog("Model download", "Downloading models ...", 100, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
                prog->Show();
            }
            auto oldNewModel = _newModel;
            auto oldam = modelPreview->GetAdditionalModel();
            modelPreview->SetAdditionalModel(nullptr); // just in case we delete the model

            _newModel = Model::GetXlightsModel(_newModel, _lastXlightsModel, xlights, cancelled, b->GetModelType() == "Download", prog, 0, 99, modelPreview);

            // These statements ensure the Additional model and _newModel pointers are all ok and any unnecessary models is cleaned up
            if (_newModel != oldNewModel) {
                // model was changed

                if (highlightedBaseObject == oldNewModel) {
                    highlightedBaseObject = _newModel;
                }
                if (selectedBaseObject == oldNewModel) {
                    selectedBaseObject = _newModel;
                }

                if (oldam == oldNewModel) {
                    modelPreview->SetAdditionalModel(_newModel);
                    oldam = nullptr;
                }
                else {
                    modelPreview->SetAdditionalModel(oldam);
                }
                // dont delete the oldNewModel as it should already be deleted
                //delete oldNewModel;
                oldNewModel = nullptr;
            }
            else {
                modelPreview->SetAdditionalModel(oldam);
            }

            // we get rid of additional model here
            if (prog != nullptr)
            {
                delete prog;
            }

            if (cancelled || _newModel == nullptr) {
                _lastXlightsModel = "";
                xlights->AddTraceMessage("LayoutPanel::FinalizeModel Downloading or importing cancelled.");
                xlights->GetOutputModelManager()->ClearSelectedModel();
                modelPreview->SetAdditionalModel(nullptr);
                xlights->AddTraceMessage("LayoutPanel::FinalizeModel Additional model cleared.");
                if (_newModel != nullptr)
                {
                    xlights->AddTraceMessage("LayoutPanel::FinalizeModel About to delete newModel.");
                    delete _newModel; // I am not sure this may cause issues ... but if we dont have it i think it leaks
                    xlights->AddTraceMessage("LayoutPanel::FinalizeModel newModel successfully deleted.");
                    _newModel = nullptr;
                }
                xlights->AddTraceMessage("LayoutPanel::Model deleted.");
                modelPreview->SetCursor(wxCURSOR_DEFAULT);
                b->SetState(0);
                selectedButton = nullptr;
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "FinalizeModel");
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "FinalizeModel");
                return;
            }

            xlights->AddTraceMessage("LayoutPanel::FinalizeModel Do the import. " + _lastXlightsModel);
            xlights->AddTraceMessage("LayoutPanel::FinalizeModel Model type " + _newModel->GetDisplayAs());
            _newModel->ImportXlightsModel(_lastXlightsModel, xlights, min_x, max_x, min_y, max_y);
            xlights->AddTraceMessage("LayoutPanel::FinalizeModel Import done.");
            if (_newModel->GetDisplayAs() == "Poly Line")
            {
                _newModel->SetPosition(pos.x, pos.y);
            }
            if (b->GetState() == 1)
            {
                _lastXlightsModel = "";
            }
        }
        xlights->AddTraceMessage("LayoutPanel::FinalizeModel Adding the model.");
        CreateUndoPoint("All", "", "");
        _newModel->UpdateXmlWithScale();
        xlights->AllModels.AddModel(_newModel);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "FinalizeModel");

        _newModel->SetLayoutGroup(currentLayoutGroup == "All Models" ? "Default" : currentLayoutGroup);

        modelPreview->SetCursor(wxCURSOR_DEFAULT);
        modelPreview->SetAdditionalModel(nullptr);
        if ((b != nullptr && b->GetState() == 1) || selectedDmxModelType != "") {
            xlights->AddTraceMessage("LayoutPanel::FinalizeModel Exiting done.");
            std::string name = _newModel->name;
            _newModel = nullptr;
            if (b != nullptr) {  // needed if dmx model type
                b->SetState(0);
            }
            selectedButton = nullptr;
            selectedBaseObject = nullptr;
            selectedDmxModelType = "";
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "FinalizeModel", nullptr, nullptr, name);
            //SelectBaseObject(name);
            SelectBaseObject3D();
        }
        else {
            xlights->AddTraceMessage("LayoutPanel::FinalizeModel Exiting but can draw another model.");
            _newModel = nullptr;
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "FinalizeModel");
        }
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "FinalizeModel");
    }
}

void LayoutPanel::OnPreviewMouseLeave(wxMouseEvent& event)
{
    m_dragging = false;
    m_wheel_down = false;
}

void LayoutPanel::OnPreviewMouseWheelDown(wxMouseEvent& event)
{
    m_previous_mouse_x = event.GetX();
    m_previous_mouse_y = event.GetY();
    m_wheel_down = true;
}

void LayoutPanel::OnPreviewMouseWheelUp(wxMouseEvent& event)
{
    m_wheel_down = false;
}
void LayoutPanel::OnPreviewMotion3DButtonEvent(wxCommandEvent &event) {
    
    if (event.GetString() == "BUTTON_MENU") {
        wxMouseEvent evt;
        OnPreviewRightDown(evt);
    } else {
        modelPreview->OnMotion3DButtonEvent(event);
        /*
        int gSize = selectedTreeGroups.size();
        int smSize = selectedTreeSubModels.size();
        if (selectedBaseObject != nullptr && gSize == 0 && smSize == 0) {
        } else {
        }
        */
    }
}

void LayoutPanel::OnPreviewMotion3D(Motion3DEvent &event) {
    int gSize = selectedTreeGroups.size();
    int smSize = selectedTreeSubModels.size();
    if (selectedBaseObject != nullptr && gSize == 0 && smSize == 0 && !event.ControlDown() && !event.RawControlDown()) {
        int active_handle = selectedBaseObject->GetBaseObjectScreenLocation().GetActiveHandle();
        if (!xlights->AbortRender()) return;
        CreateUndoPoint(editing_models ? "SingleModel" : "SingleObject", selectedBaseObject->name, std::to_string(active_handle));

        float scale = modelPreview->translateToBacking(1.0) * 20.0 * modelPreview->GetZoom(); //20 pixels at max speed, default zoom
        if (!modelPreview->Is3D()) {
            //moving/rotating the entire model
            constexpr float rscale = 10; //10 degrees per full 1.0 aka: max speed
            selectedBaseObject->Rotate(ModelScreenLocation::MSLAXIS::X_AXIS, event.rotations.x * rscale);
            selectedBaseObject->Rotate(ModelScreenLocation::MSLAXIS::Y_AXIS, -event.rotations.z * rscale);
            selectedBaseObject->Rotate(ModelScreenLocation::MSLAXIS::Z_AXIS, event.rotations.y * rscale);

            selectedBaseObject->AddOffset(event.translations.x * scale,
                                          -event.translations.z * scale - event.translations.y * scale,
                                          0.0f);

            last_centerpos = selectedBaseObject->GetBaseObjectScreenLocation().GetCenterPosition();
            last_worldrotate = selectedBaseObject->GetBaseObjectScreenLocation().GetRotationAngles();
            last_worldscale = selectedBaseObject->GetBaseObjectScreenLocation().GetScaleMatrix();
            selectedBaseObject->UpdateXmlWithScale();

            SetupPropGrid(selectedBaseObject);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPreviewMotion3D");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnPreviewMotion3D");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMotion3D");
        } else if (modelPreview->Is3D()) {
            selectedBaseObject->MoveHandle3D(scale, active_handle, event.rotations, event.translations);

            last_centerpos = selectedBaseObject->GetBaseObjectScreenLocation().GetCenterPosition();
            last_worldrotate = selectedBaseObject->GetBaseObjectScreenLocation().GetRotationAngles();
            last_worldscale = selectedBaseObject->GetBaseObjectScreenLocation().GetScaleMatrix();
            selectedBaseObject->UpdateXmlWithScale();
            
            SetupPropGrid(selectedBaseObject);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPreviewMotion3D");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnPreviewMotion3D");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMotion3D");
        }
    } else {
        modelPreview->OnMotion3DEvent(event);
    }
}

void LayoutPanel::OnPreviewRotateGesture(wxRotateGestureEvent& event) {
    if (!rotate_gesture_active && !event.IsGestureStart()) {
        return;
    }
    if (event.IsGestureEnd()) {
        rotate_gesture_active = false;
    } else if (event.IsGestureStart()) {
        rotate_gesture_active = true;
    }
    int gSize = selectedTreeGroups.size();
    int smSize = selectedTreeSubModels.size();
    
    if (selectedBaseObject != nullptr && gSize == 0 && smSize == 0) {
        //groups and submodels shouldn't rotate
        float delta = (m_last_mouse_x - (event.GetRotationAngle() * 1000)) / 1000.0;
        if (!event.IsGestureStart()) {
            //convert to degrees
            delta = (delta/(2*M_PI))*360.0;
            if (delta > 90) {
                delta -= 360;
            }
            if (delta < -90) {
                delta += 360;
            }

            ModelScreenLocation::MSLAXIS axis = ModelScreenLocation::MSLAXIS::Z_AXIS; //default is around z axis
            if (wxGetKeyState(WXK_SHIFT)) {
                axis = ModelScreenLocation::MSLAXIS::X_AXIS;
            } else if (wxGetKeyState(WXK_CONTROL)) {
                axis = ModelScreenLocation::MSLAXIS::Y_AXIS;
            }
            if (selectedBaseObject->Rotate(axis, delta)) {
                SetupPropGrid(selectedBaseObject);
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPreviewRotateGesture");
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnPreviewRotateGesture");
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewRotateGesture");
            }
        } else {
            CreateUndoPoint(editing_models ? "SingleModel" : "SingleObject", selectedBaseObject->name, "Zoom");
        }
    }
    m_last_mouse_x = (event.GetRotationAngle() * 1000);
}

void LayoutPanel::OnPreviewZoomGesture(wxZoomGestureEvent& event) {
    if (!zoom_gesture_active && !event.IsGestureStart()) {
        return;
    }
    if (event.IsGestureEnd()) {
        zoom_gesture_active = false;
    } else if (event.IsGestureStart()) {
        zoom_gesture_active = true;
    }
    float delta = (m_last_mouse_x - (event.GetZoomFactor() * 1000)) / 1000.0;
    
    int gSize = selectedTreeGroups.size();
    int smSize = selectedTreeSubModels.size();
    
    if (selectedBaseObject != nullptr && gSize == 0 && smSize == 0) {
        //groups and submodels shouldn't resize/scale
        if (!event.IsGestureStart()) {
            //resize model
            if (selectedBaseObject->Scale(glm::vec3(1.0f - delta))) {
                SetupPropGrid(selectedBaseObject);
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPreviewZoomGesture");
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnPreviewZoomGesture");
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewZoomGesture");
            }

            if (Is3d()) {
                auto pos = selectedBaseObject->GetBaseObjectScreenLocation().GetWorldPosition();
                xlights->SetStatusText(wxString::Format("x=%.2f y=%.2f z=%.2f %s", pos.x, pos.y, pos.z, selectedBaseObject->GetDimension()));
            }
        } else {
            CreateUndoPoint(editing_models ? "SingleModel" : "SingleObject", selectedBaseObject->name, "Zoom");
        }
    } else if (!event.IsGestureStart()) {
        modelPreview->SetZoomDelta(delta);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewZoomGesture");
    }
    m_last_mouse_x = (event.GetZoomFactor() * 1000);
}

void LayoutPanel::OnPreviewMagnify(wxMouseEvent& event) {
    if (event.GetWheelRotation() == 0 || event.GetMagnification() == 0.0f) {
        //magnification of 0 is sometimes generated for other gestures (pinch/zoom), ignore
        return;
    }
    modelPreview->SetZoomDelta(event.GetMagnification() > 0 ? -0.1f : 0.1f);
}

void LayoutPanel::OnPreviewMouseWheel(wxMouseEvent& event)
{
    if (event.GetWheelRotation() == 0) {
        //rotation of 0 is sometimes generated for other gestures (pinch/zoom), ignore
        return;
    }
    // Don't allow zoom if panning
    if (!m_wheel_down) {
        bool fromTrackPad = IsMouseEventFromTouchpad();
        if (is_3d) {
            if (!fromTrackPad || event.ControlDown()) {
                int mouse_x = event.GetX();
                int mouse_y = event.GetY();
                float centerx = modelPreview->getWidth() / 2.0f;
                float centery = modelPreview->getHeight() / 2.0f;
                float deltax = mouse_x - centerx;
                float deltay = mouse_y - centery;
                float zoom_delta = event.GetWheelRotation() > 0 ? -0.1f : 0.1f;
                if (fromTrackPad) {
                    float f = event.GetWheelRotation();
                    zoom_delta = -f / 100.0f;
                }
                modelPreview->SetZoomDelta(zoom_delta);
                float zoom = modelPreview->GetZoom();
                float new_x = deltax * zoom_delta / zoom;
                float new_y = deltay * zoom_delta / zoom;

                // account for grid rotation
                float angleX = glm::radians(modelPreview->GetCameraRotationX());
                float angleY = glm::radians(modelPreview->GetCameraRotationY());
                float delta_x = 0.0f;
                float delta_y = 0.0f;
                float delta_z = 0.0f;
                bool top_view = (angleX > glm::radians(45.0f)) && (angleX < glm::radians(135.0f));
                bool bottom_view = (angleX > glm::radians(225.0f)) && (angleX < glm::radians(315.0f));
                bool upside_down_view = (angleX >= glm::radians(135.0f)) && (angleX <= glm::radians(225.0f));
                if (top_view) {
                    delta_x = new_x * std::cos(angleY) - new_y * std::sin(angleY);
                    delta_z = new_y * std::cos(angleY) + new_x * std::sin(angleY);
                }
                else if (bottom_view) {
                    delta_x = new_x * std::cos(angleY) + new_y * std::sin(angleY);
                    delta_z = -new_y * std::cos(angleY) + new_x * std::sin(angleY);
                }
                else {
                    delta_x = new_x * std::cos(angleY);
                    delta_y = new_y;
                    delta_z = new_x * std::sin(angleY);
                    if (!upside_down_view) {
                        delta_y *= -1.0f;
                    }
                }
                delta_x *= modelPreview->GetZoom() * 2.0f;
                delta_y *= modelPreview->GetZoom() * 2.0f;
                delta_z *= modelPreview->GetZoom() * 2.0f;
                modelPreview->SetPan(delta_x, delta_y, delta_z);
            } else {
                if (event.ShiftDown()) {
                    float new_x = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? 0 : -event.GetWheelRotation();
                    float new_y = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? -event.GetWheelRotation() : 0;

                    // account for grid rotation
                    float angleX = glm::radians(modelPreview->GetCameraRotationX());
                    float angleY = glm::radians(modelPreview->GetCameraRotationY());
                    float delta_x = 0.0f;
                    float delta_y = 0.0f;
                    float delta_z = 0.0f;
                    bool top_view = (angleX > glm::radians(45.0f)) && (angleX < glm::radians(135.0f));
                    bool bottom_view = (angleX > glm::radians(225.0f)) && (angleX < glm::radians(315.0f));
                    bool upside_down_view = (angleX >= glm::radians(135.0f)) && (angleX <= glm::radians(225.0f));
                    if( top_view ) {
                        delta_x = new_x * std::cos(angleY) - new_y * std::sin(angleY);
                        delta_z = new_y * std::cos(angleY) + new_x * std::sin(angleY);
                    } else if( bottom_view ) {
                        delta_x = new_x * std::cos(angleY) + new_y * std::sin(angleY);
                        delta_z = -new_y * std::cos(angleY) + new_x * std::sin(angleY);
                    } else {
                        delta_x = new_x * std::cos(angleY);
                        delta_y = new_y;
                        delta_z = new_x * std::sin(angleY);
                        if( upside_down_view ) {
                            delta_y *= -1.0f;
                        }
                    }
                    delta_x *= modelPreview->GetZoom() * 2.0f;
                    delta_y *= modelPreview->GetZoom() * 2.0f;
                    delta_z *= modelPreview->GetZoom() * 2.0f;
                    modelPreview->SetPan(delta_x, delta_y, delta_z);
                } else {
                    float delta_x = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? 0 : -event.GetWheelRotation();
                    float delta_y = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? -event.GetWheelRotation() : 0;

                    modelPreview->SetCameraView(delta_x, delta_y, false);
                    modelPreview->SetCameraView(0, 0, true);
                }
            }
        }
        else {
            if (!fromTrackPad || event.ControlDown()) {
                int mouse_x = event.GetX();
                int mouse_y = event.GetY();
                float centerx = modelPreview->getWidth() / 2.0f;
                float centery = modelPreview->getHeight() / 2.0f;
                float deltax = mouse_x - centerx;
                float deltay = mouse_y - centery;
                float zoom_delta = event.GetWheelRotation() > 0 ? -0.1f : 0.1f;
                if (fromTrackPad) {
                    float f = event.GetWheelRotation();
                    zoom_delta = -f / 100.0f;
                }
                modelPreview->SetZoomDelta(zoom_delta);
                float zoom = modelPreview->GetZoom();
                float new_x = deltax * zoom_delta / zoom;
                float new_y = deltay * zoom_delta / zoom;
                modelPreview->SetPan(new_x, -new_y, 0.0f);
            }
            else {
                float delta_x = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? 0 : -event.GetWheelRotation();
                float delta_y = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? -event.GetWheelRotation() : 0;
                if (!fromTrackPad) {
                    delta_x *= modelPreview->GetZoom() * 2.0f;
                    delta_y *= modelPreview->GetZoom() * 2.0f;
                }
                modelPreview->SetPan(delta_x, delta_y, 0.0f);
                m_previous_mouse_x = event.GetX();
                m_previous_mouse_y = event.GetY();
            }
        }
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseWheel");
    }
}

void LayoutPanel::OnPreviewMouseMove3D(wxMouseEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D");

    if (m_creating_bound_rect)
    {
        xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Creating bounding rectangle");
        m_bound_end_x = event.GetX();
        m_bound_end_y = event.GetY();
        HighlightAllInBoundingRect(event.ControlDown() && event.ShiftDown());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
        return;
    }
    else if (m_wheel_down)
    {
        xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Wheel down");
        float new_x = event.GetX() - m_previous_mouse_x;
        float new_y = event.GetY() - m_previous_mouse_y;
        // account for grid rotation
        float angleX = glm::radians(modelPreview->GetCameraRotationX());
        float angleY = glm::radians(modelPreview->GetCameraRotationY());
        float delta_x = 0.0f;
        float delta_y = 0.0f;
        float delta_z = 0.0f;
        bool top_view = (angleX > glm::radians(45.0f)) && (angleX < glm::radians(135.0f));
        bool bottom_view = (angleX > glm::radians(225.0f)) && (angleX < glm::radians(315.0f));
        bool upside_down_view = (angleX >= glm::radians(135.0f)) && (angleX <= glm::radians(225.0f));
        if( top_view ) {
            delta_x = new_x * std::cos(angleY) - new_y * std::sin(angleY);
            delta_z = new_y * std::cos(angleY) + new_x * std::sin(angleY);
        } else if( bottom_view ) {
            delta_x = new_x * std::cos(angleY) + new_y * std::sin(angleY);
            delta_z = -new_y * std::cos(angleY) + new_x * std::sin(angleY);
        } else {
            delta_x = new_x * std::cos(angleY);
            delta_y = new_y;
            delta_z = new_x * std::sin(angleY);
            if( !upside_down_view ) {
                delta_y *= -1.0f;
            }
        }
        delta_x *= modelPreview->GetZoom() * 2.0f;
        delta_y *= modelPreview->GetZoom() * 2.0f;
        delta_z *= modelPreview->GetZoom() * 2.0f;
        modelPreview->SetPan(delta_x, delta_y, delta_z);
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
    }
    else if (m_mouse_down) {
        if (m_moving_handle) {
            xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Mouse down moving handle");
            Model* m = dynamic_cast<Model*>(selectedBaseObject);
            if (selectedBaseObject != nullptr && (_newModel == selectedBaseObject || xlights->AllModels.IsModelValid(m))) {
                if (!xlights->AbortRender()) return;

                int active_handle = selectedBaseObject->GetBaseObjectScreenLocation().GetActiveHandle();

                int selectedModelCnt = ModelsSelectedCount();
                int selectedViewObjectCnt = ViewObjectsSelectedCount();
                if (selectedBaseObject != _newModel) {
                    CreateUndoPoint(editing_models ? "SingleModel" : "SingleObject", selectedBaseObject->name, std::to_string(active_handle));
                }
                bool z_scale = selectedBaseObject->GetBaseObjectScreenLocation().GetSupportsZScaling();
                if (selectedBaseObject->GetBaseObjectScreenLocation().GetAxisTool() == ModelScreenLocation::MSLTOOL::TOOL_ROTATE) {
                    SetMouseStateForModels(true);
                }
                // this is designed to pretend the control and shift keys are down when creating models to
                // make them scale from the desired handle depending on model type
                auto pos = selectedBaseObject->MoveHandle3D(modelPreview, active_handle, event.ShiftDown() || creating_model, event.ControlDown() || (creating_model && z_scale), event.GetX(), event.GetY(), false, z_scale);
                xlights->SetStatusText(wxString::Format("x=%.2f y=%.2f z=%.2f %s", pos.x, pos.y, pos.z, selectedBaseObject->GetDimension()));
                //SetupPropGrid(selectedBaseObject);
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewMouseMove");
                // dont need these until released
                //xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPreviewMouseMove3D");
                //xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnPreviewMouseMove3D");
                if (selectedModelCnt > 1 || selectedViewObjectCnt > 1) {
                    if (selectedBaseObject->GetBaseObjectScreenLocation().GetAxisTool() == ModelScreenLocation::MSLTOOL::TOOL_TRANSLATE) {
                        glm::vec3 new_centerpos = selectedBaseObject->GetBaseObjectScreenLocation().GetCenterPosition();
                        glm::vec3 pos_offset = new_centerpos - last_centerpos;
                        for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
                        {
                            if (modelPreview->GetModels()[i]->GroupSelected || modelPreview->GetModels()[i]->Selected) {
                                if (modelPreview->GetModels()[i] != selectedBaseObject) {
                                    modelPreview->GetModels()[i]->AddOffset(pos_offset.x, pos_offset.y, pos_offset.z);
                                }
                            }
                        }
                        for (const auto& it : xlights->AllObjects) {
                            ViewObject* view_object = it.second;
                            if (view_object->GroupSelected || view_object->Selected) {
                                if (view_object != selectedBaseObject) {
                                    view_object->AddOffset(pos_offset.x, pos_offset.y, pos_offset.z);
                                }
                            }
                        }
                        last_centerpos = new_centerpos;
                    } else if (selectedBaseObject->GetBaseObjectScreenLocation().GetAxisTool() == ModelScreenLocation::MSLTOOL::TOOL_ROTATE) {
                        glm::vec3 new_worldrotate = selectedBaseObject->GetBaseObjectScreenLocation().GetRotationAngles();
                        glm::vec3 rotate_offset = new_worldrotate - last_worldrotate;
                        glm::vec3 active_handle_position = selectedBaseObject->GetBaseObjectScreenLocation().GetActiveHandlePosition();
                        if( rotate_offset.x > 180.0f ) { rotate_offset.x -= 360.0f; }
                        if( rotate_offset.y > 180.0f ) { rotate_offset.y -= 360.0f; }
                        if( rotate_offset.z > 180.0f ) { rotate_offset.z -= 360.0f; }
                        if( rotate_offset.x < -180.0f ) { rotate_offset.x += 360.0f; }
                        if( rotate_offset.y < -180.0f ) { rotate_offset.y += 360.0f; }
                        if( rotate_offset.z < -180.0f ) { rotate_offset.z += 360.0f; }
                        rotate_offset.x = -rotate_offset.x;
                        for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
                            if (modelPreview->GetModels()[i]->GroupSelected || modelPreview->GetModels()[i]->Selected) {
                                if (modelPreview->GetModels()[i] != selectedBaseObject) {
                                    xlights->AbortRender();
                                    modelPreview->GetModels()[i]->RotateAboutPoint(active_handle_position, rotate_offset);
                                }
                            }
                        }
                        for (const auto& it : xlights->AllObjects) {
                            ViewObject* view_object = it.second;
                            if (view_object->GroupSelected || view_object->Selected) {
                                if (view_object != selectedBaseObject) {
                                    xlights->AbortRender();
                                    view_object->RotateAboutPoint(active_handle_position, rotate_offset);
                                }
                            }
                        }
                        last_worldrotate = new_worldrotate;
                    }
                    if (selectedBaseObject->GetBaseObjectScreenLocation().GetAxisTool() == ModelScreenLocation::MSLTOOL::TOOL_SCALE) {
                        glm::vec3 new_worldscale = selectedBaseObject->GetBaseObjectScreenLocation().GetScaleMatrix();
                        if (last_worldscale.x == 0 || last_worldscale.y == 0 || last_worldscale.z == 0) {
                            logger_base.crit("This is not going to end well last_world_scale has a zero parameter and we are about to divide using it.");
                        }
                        glm::vec3 scale_offset = glm::vec3(new_worldscale / last_worldscale);
                        for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
                        {
                            if (modelPreview->GetModels()[i]->GroupSelected || modelPreview->GetModels()[i]->Selected) {
                                if (modelPreview->GetModels()[i] != selectedBaseObject) {
                                    modelPreview->GetModels()[i]->Scale(scale_offset);
                                }
                            }
                        }
                        for (const auto& it : xlights->AllObjects) {
                            ViewObject* view_object = it.second;
                            if (view_object->GroupSelected || view_object->Selected) {
                                if (view_object != selectedBaseObject) {
                                    view_object->Scale(scale_offset);
                                }
                            }
                        }
                        last_worldscale = new_worldscale;
                    }
                }
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
            }
        }
        else {
        xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Mouse down NOT moving handle");
            int delta_x = event.GetPosition().x - m_last_mouse_x;
            int delta_y = event.GetPosition().y - m_last_mouse_y;
            modelPreview->SetCameraView(delta_x, delta_y, false);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
        }
    }
    else {
        if (!selectionLatched) {
            xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Not selection latched");
            glm::vec3 ray_origin;
            glm::vec3 ray_direction;
            GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
            BaseObject* which_object = nullptr;
            float distance = 1000000000.0f;
            float intersection_distance = 1000000000.0f;
            if( editing_models ) {
                xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Not selection latched - Editing models");
                for (const auto& it : modelPreview->GetModels())
                {
                    if (it->GetBaseObjectScreenLocation().HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                        if (intersection_distance < distance) {
                            distance = intersection_distance;
                            which_object = it;
                        }
                    }
                }
            } else {
                xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Not selection latched - Not editing models");
                for (const auto& it : xlights->AllObjects) {
                    ViewObject *view_object = it.second;
                    if (view_object->GetBaseObjectScreenLocation().HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                        if (intersection_distance < distance) {
                            distance = intersection_distance;
                            which_object = view_object;
                        }
                    }
                }
            }
            if (which_object == nullptr)
            {
                xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Not selection latched - Not editing models - AAA");
                if (highlightedBaseObject != nullptr) {
                    highlightedBaseObject->Highlighted = false;
                    highlightedBaseObject = nullptr;
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
                }
            }
            else
            {
                xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Not selection latched - Not editing models - BBB");
                if (which_object != last_selection) {
                    UnSelectAllModels();
                    highlightedBaseObject = which_object;
                    highlightedBaseObject->Highlighted = true;
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
                }
            }
            last_selection = which_object;
        }
        xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D CCC");
        if (m_moving_handle)
        {
            xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Moving handle");
            BaseObject *obj = _newModel;
            if (obj == nullptr) {
                obj = selectedBaseObject;
                Model* m = dynamic_cast<Model*>(obj);
                if (obj == nullptr || (!xlights->AllModels.IsModelValid(m) && _newModel != obj)) return;
            }
            int active_handle = obj->GetBaseObjectScreenLocation().GetActiveHandle();
            if (obj != _newModel) {
                CreateUndoPoint(editing_models ? "SingleModel" : "SingleObject", obj->name, std::to_string(active_handle));
            }
            bool z_scale = obj->GetBaseObjectScreenLocation().GetSupportsZScaling();
            // this is designed to pretend the control and shift keys are down when creating models to
            // make them scale from the desired handle depending on model type
            xlights->AbortRender();
            auto pos = obj->MoveHandle3D(modelPreview, active_handle, event.ShiftDown() || creating_model, event.ControlDown() || (creating_model && z_scale), event.GetX(), event.GetY(), false, z_scale);
            xlights->SetStatusText(wxString::Format("x=%.2f y=%.2f z=%.2f %s", pos.x, pos.y, pos.z, obj->GetDimension()));
            //SetupPropGrid(obj);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewMouseMove");
            // dont need these until model is finished moving
            //xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPreviewMouseMove3D");
            //xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnPreviewMouseMove3D");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
        }
        else {
            Model* m = dynamic_cast<Model*>(selectedBaseObject);
            if (selectedBaseObject != nullptr && (_newModel == selectedBaseObject || xlights->AllModels.IsModelValid(m))) {
                xlights->AddTraceMessage("LayoutPanel::OnPreviewMouseMove3D Moving but no model selected");
                glm::vec3 ray_origin;
                glm::vec3 ray_direction;
                GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
                // check for mouse over handle and if so highlight it
                modelPreview->SetCursor(selectedBaseObject->GetBaseObjectScreenLocation().CheckIfOverHandles3D(ray_origin, ray_direction, m_over_handle, modelPreview->GetCameraZoomForHandles(), modelPreview->GetHandleScale()));
                if (m_over_handle != over_handle) {
                    selectedBaseObject->GetBaseObjectScreenLocation().MouseOverHandle(m_over_handle);
                    over_handle = m_over_handle;
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
                } else if( event.ControlDown() ) {
                    // For now require control to be active before we start highlighting other models while a model is selected otherwise
                    // it gets hard to work on selected model with everything else highlighting.
                    // See if hovering over a model and if so highlight it or remove highlight as you leave it if it wasn't selected.
                    BaseObject* which_object = nullptr;
                    float distance = 1000000000.0f;
                    float intersection_distance = 1000000000.0f;
                    if( editing_models ) {
                        for (const auto& it : modelPreview->GetModels())
                        {
                            if (it != selectedBaseObject) {
                                if (it->GetBaseObjectScreenLocation().HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                                    if (intersection_distance < distance) {
                                        distance = intersection_distance;
                                        which_object = it;
                                    }
                                }
                            }
                        }
                    } else {
                        for (const auto& it : xlights->AllObjects) {
                            ViewObject *view_object = it.second;
                            if (view_object->GetBaseObjectScreenLocation().HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                                if (intersection_distance < distance) {
                                    distance = intersection_distance;
                                    which_object = view_object;
                                }
                            }
                        }
                    }
                    if (which_object != nullptr)
                    {
                        if (last_highlight != which_object) {
                            if (!which_object->Highlighted) {
                                which_object->Highlighted = true;
                            }
                            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
                        }
                    }
                    if (last_highlight != nullptr && last_highlight != which_object) {
                        if (last_highlight->Highlighted &&
                            !(last_highlight->Selected || last_highlight->GroupSelected)) {
                            last_highlight->Highlighted = false;
                            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove3D");
                        }
                    }
                    last_highlight = which_object;
                }
            }
        }
    }
}

void LayoutPanel::OnPreviewMouseMove(wxMouseEvent& event)
{
    if (is_3d) {
        OnPreviewMouseMove3D(event);
        return;
    }

    /*/ FIXME:  Delete when not needed for debugging
    /////////////////////////////////////
    // temporary for debugging
    glm::vec3 ray_origin;
    glm::vec3 ray_direction;
    GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
    {
        Model* whichModel = modelPreview->GetModels()[i];
        if (modelPreview->GetModels()[i]->GetBaseObjectScreenLocation().HitTest(ray_origin, ray_direction)) {
            whichModel->Highlighted = true;
        }
        else {
            whichModel->Highlighted = false;
        }
        UpdatePreview();
    }
    /////////////////////////////////////*/

    if (m_creating_bound_rect) {
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
        m_bound_end_x = ray_origin.x;
        m_bound_end_y = ray_origin.y;
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove");
        return;
    }
    else if (m_wheel_down) {
        float delta_x = event.GetX() - m_previous_mouse_x;
        float delta_y = event.GetY() - m_previous_mouse_y;
        delta_x /= modelPreview->GetZoom();
        delta_y /= modelPreview->GetZoom();
        modelPreview->SetPan(delta_x, -delta_y, 0.0f);
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove");
    }

    Model* m = _newModel;
    if (m == nullptr) {
        m = dynamic_cast<Model*>(selectedBaseObject);
        if (m == nullptr) return;
    }

    if (m_moving_handle) {
        if (!xlights->AbortRender()) return;
        if (m != _newModel) {
            CreateUndoPoint("SingleModel", m->name, std::to_string(m_over_handle));
        }
        auto pos = m->MoveHandle(modelPreview, m_over_handle, event.ShiftDown(), event.GetX(), event.GetY());
        xlights->SetStatusText(wxString::Format("x=%.2f y=%.2f", pos.x, pos.y));

        //SetupPropGrid(m);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewMouseMove");
        // dont need these until finish moving
        //xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPreviewMouseMove");
        //xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnPreviewMouseMove");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove");
    }
    else if (m_dragging && event.Dragging()) {
        double delta_x = (double)event.GetX() - m_previous_mouse_x;
        double delta_y = -((double)event.GetY() - m_previous_mouse_y);

        auto scale = modelPreview->GetCurrentScaleFactor();
        delta_x /= modelPreview->GetZoom() / scale;
        delta_y /= modelPreview->GetZoom() / scale;
        int wi, ht;
        modelPreview->GetVirtualCanvasSize(wi, ht);
        if (wi > 0 && ht > 0) {
            for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
                if (modelPreview->GetModels()[i]->Selected || modelPreview->GetModels()[i]->GroupSelected) {
                    CreateUndoPoint("SingleModel", m->name, "location");
                    modelPreview->GetModels()[i]->AddOffset(delta_x, delta_y, 0.0);
                    modelPreview->GetModels()[i]->UpdateXmlWithScale();
                    //SetupPropGrid(modelPreview->GetModels()[i]);
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewMouseMove");
                    // dont need these until finished moving
                    //xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnPreviewMouseMove");
                    //xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnPreviewMouseMove");
                }
            }
        }
        m_previous_mouse_x = event.GetPosition().x;
        m_previous_mouse_y = event.GetPosition().y;
        xlights->SetStatusText(wxString::Format("x=%d y=%d", m_previous_mouse_x, m_previous_mouse_y));
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewMouseMove");
    }
    else {
        if (m->Selected) {
            modelPreview->SetCursor(m->GetBaseObjectScreenLocation().CheckIfOverHandles(modelPreview, m_over_handle, event.GetX(), event.GetY()));
        }
    }
}

bool LayoutPanel::IsAllSelectedModelsArePixelProtocol() const
{
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
        if (modelPreview->GetModels()[i]->Selected || modelPreview->GetModels()[i]->GroupSelected) {
            if (!modelPreview->GetModels()[i]->IsPixelProtocol()) {
                return false;
            }
        }
    }
    return true;
}

void LayoutPanel::AddSingleModelOptionsToBaseMenu(wxMenu &menu) {
    int selectedObjectCnt = editing_models ? ModelsSelectedCount() : ViewObjectsSelectedCount();

    if (selectedBaseObject != nullptr && !selectedBaseObject->GetBaseObjectScreenLocation().IsLocked())
    {
        bool need_sep = false;
        if( editing_models ) {
            Model* model = dynamic_cast<Model*>(selectedBaseObject);
            int sel_seg = model->GetSelectedSegment();
            if( sel_seg != -1 ) {
                if( !model->HasCurve(sel_seg) ) {
                    menu.Append(ID_PREVIEW_MODEL_ADDPOINT,"Add Point");
                    if( model->SupportsCurves() ) {
                        menu.Append(ID_PREVIEW_MODEL_ADDCURVE,"Define Curve");
                    }
                } else {
                    menu.Append(ID_PREVIEW_MODEL_DELCURVE,"Remove Curve");
                }
                need_sep = true;
            }
            int sel_hdl = model->GetSelectedHandle();
            // Center handle is 0 and selected segments are greater than 0x4000
            if( (sel_hdl > 0) && (sel_hdl < 0x4000) && (sel_hdl < model->GetNumHandles()) && (model->GetNumHandles() > 2) ) {
                menu.Append(ID_PREVIEW_MODEL_DELETEPOINT,"Delete Point");
                need_sep = true;
            }
        }
        if( need_sep ) {
            menu.AppendSeparator();
        }
        if( editing_models && (selectedBaseObject->GetDisplayAs() == "Matrix" ))
        {
            menu.Append(ID_PREVIEW_MODEL_ASPECTRATIO,"Correct Aspect Ratio");
        }
        if (is_3d && selectedObjectCnt == 1) {
            menu.Append(ID_PREVIEW_ALIGN_GROUND, "Align With Ground");
        }
    }
    if (editing_models && (selectedBaseObject != nullptr))
    {
        menu.Append(ID_PREVIEW_MODEL_LOCK, "Lock");
        menu.Append(ID_PREVIEW_MODEL_UNLOCK, "Unlock");
        Model* model = dynamic_cast<Model*>(selectedBaseObject);
        if (model != nullptr && model->GetDisplayAs() != "ModelGroup" && model->GetDisplayAs() != "SubModel") {
            menu.Append(ID_PREVIEW_MODEL_NODELAYOUT, "Node Layout");
        }
        if (model->SupportsWiringView()) {
            menu.Append(ID_PREVIEW_MODEL_WIRINGVIEW, "Wiring View");
        }
        menu.AppendSeparator();
        if (model->SupportsExportAsCustom())
        {
            menu.Append(ID_PREVIEW_MODEL_EXPORTASCUSTOM, "Export as Custom xLights Model");
        }
        if (model->SupportsExportAsCustom3D()) {
            menu.Append(ID_PREVIEW_MODEL_EXPORTASCUSTOM3D, "Export as 3D Custom xLights Model");
        }
        if (model->SupportsXlightsModel())
        {
            menu.Append(ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL, "Export xLights Model");
        }
        menu.Append(ID_PREVIEW_MODEL_CAD_EXPORT, "Export As DXF/STL/VRML");

        menu.AppendSeparator();
        for (const auto& it : xlights->AllModels) {
            if (it.second->GetDisplayAs() == "ModelGroup") {
                menu.Append(ID_MNU_ADD_TO_EXISTING_GROUPS, "Add to Existing Groups");
                break;
            }
        }
        menu.Append(ID_PREVIEW_MODEL_CREATEGROUP, "Create Group");
        menu.AppendSeparator();
        menu.Append(ID_PREVIEW_FLIP_HORIZONTAL, "Flip Horizontal");
        menu.Append(ID_PREVIEW_FLIP_VERTICAL, "Flip Vertical");
    }

    if (editing_models && (selectedObjectCnt == 1) && (modelPreview->GetModels().size() > 1))
    {
        menu.Append(ID_PREVIEW_REPLACEMODEL, "Replace A Model With This Model");
    }
}

void LayoutPanel::AddBulkEditOptionsToMenu(wxMenu* mnuBulkEdit) {
    mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_SETACTIVE, "Active");
    mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_SETINACTIVE, "Inactive");
    if (editing_models) {
        if (xlights->GetOutputManager()->GetAutoLayoutControllerNames().size() > 0)
        {
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERNAME, "Controller Name");
        }
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_TAGCOLOUR, "Tag Color");
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_PIXELSIZE, "Pixel Size");
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_PIXELSTYLE, "Pixel Style");
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_TRANSPARENCY, "Transparency");
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_BLACKTRANSPARENCY, "Black Transparency");

        mnuBulkEdit->AppendSeparator();
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTION, "Controller Port");
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTIONINCREMENT, "Controller Port and Increment");
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERPROTOCOL, "Controller Protocol");
        if (IsAllSelectedModelsArePixelProtocol())
        {
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_SMARTREMOTE, "Controller Smart Remote");
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_SMARTREMOTETYPE, "Controller Smart Remote Type");
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERDIRECTION, "Controller Direction");
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERBRIGHTNESS, "Controller Brightness");
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERGAMMA, "Controller Gamma");
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERCOLOURORDER, "Controller Colour Order");
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERSTARTNULLNODES, "Controller Start Null Nodes");
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERENDNULLNODES, "Controller End Null Nodes");
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERGROUPCOUNT, "Controller Group Count");
            mnuBulkEdit->AppendSeparator();
        }
    }
    mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_PREVIEW, "Preview");
    if (editing_models) {
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_DIMMINGCURVES, "Dimming Curves");
    }
}

void LayoutPanel::AddAlignOptionsToMenu(wxMenu* mnuAlign) {
    mnuAlign->Append(ID_PREVIEW_ALIGN_TOP,"Top");
    mnuAlign->Append(ID_PREVIEW_ALIGN_BOTTOM,"Bottom");
    mnuAlign->Append(ID_PREVIEW_ALIGN_LEFT,"Left");
    mnuAlign->Append(ID_PREVIEW_ALIGN_RIGHT, "Right");
    if (is_3d) {
        mnuAlign->Append(ID_PREVIEW_ALIGN_FRONT, "Front");
        mnuAlign->Append(ID_PREVIEW_ALIGN_BACK, "Back");
        mnuAlign->Append(ID_PREVIEW_ALIGN_GROUND, "With Ground");
    }
    mnuAlign->Append(ID_PREVIEW_ALIGN_H_CENTER,"Horizontal Center");
    mnuAlign->Append(ID_PREVIEW_ALIGN_V_CENTER,"Vertical Center");
}

void LayoutPanel::AddDistributeOptionsToMenu(wxMenu* mnuDistribute) {
    mnuDistribute->Append(ID_PREVIEW_H_DISTRIBUTE,"Horizontal");
    mnuDistribute->Append(ID_PREVIEW_V_DISTRIBUTE,"Vertical");
}

void LayoutPanel::AddResizeOptionsToMenu(wxMenu* mnuResize) {
    mnuResize->Append(ID_PREVIEW_RESIZE_SAMEWIDTH, "Match Width");
    mnuResize->Append(ID_PREVIEW_RESIZE_SAMEHEIGHT, "Match Height");
    mnuResize->Append(ID_PREVIEW_RESIZE_SAMESIZE, "Match Size");
}

void LayoutPanel::OnPreviewRightDown(wxMouseEvent& event)
{
    modelPreview->SetFocus();
    wxMenu mnu;

    int selectedObjectCnt = editing_models ? ModelsSelectedCount() : ViewObjectsSelectedCount();

    if (selectedObjectCnt > 1)
    {
        wxMenu* mnuBulkEdit = new wxMenu();
        AddBulkEditOptionsToMenu(mnuBulkEdit);
        mnuBulkEdit->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);

        wxMenu* mnuAlign = new wxMenu();
        AddAlignOptionsToMenu(mnuAlign);
        mnuAlign->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);

        wxMenu* mnuDistribute = new wxMenu();
        AddDistributeOptionsToMenu(mnuDistribute);
        mnuDistribute->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);

        wxMenu* mnuResize = new wxMenu();
        AddResizeOptionsToMenu(mnuResize);
        mnuResize->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);

        mnu.Append(ID_PREVIEW_BULKEDIT, "Bulk Edit", mnuBulkEdit, "");
        mnu.Append(ID_PREVIEW_ALIGN, "Align", mnuAlign, "");
        mnu.Append(ID_PREVIEW_DISTRIBUTE, "Distribute", mnuDistribute, "");
        mnu.Append(ID_PREVIEW_RESIZE, "Resize", mnuResize, "");

        mnu.AppendSeparator();
    }

    if (selectedObjectCnt > 0) {
        AddSingleModelOptionsToBaseMenu(mnu);
    }

    if( currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned" ) {
        if (selectedObjectCnt > 0) {
            mnu.AppendSeparator();
        }
        mnu.Append(ID_PREVIEW_DELETE_ACTIVE,"Delete this Preview");
        mnu.Append(ID_PREVIEW_RENAME_ACTIVE, "Rename this Preview");
    }

    mnu.Append(ID_PREVIEW_SAVE_LAYOUT_IMAGE, _("Save Layout Image"));
    mnu.Append(ID_PREVIEW_PRINT_LAYOUT_IMAGE, _("Print Layout Image"));
    mnu.Append(ID_PREVIEW_IMPORTMODELSFROMRGBEFFECTS, _("Import Previews/Models/Groups"));
    mnu.Append(ID_PREVIEW_IMPORT_MODELS_FROM_LORS5, _("Import LOR S5 Models/Groups"));
    mnu.Append(ID_PREVIEW_LAYOUT_DXF_EXPORT, _("Export Layout As DXF"));

    // ViewPoint menus
    mnu.AppendSeparator();
    mnu.Append(ID_PREVIEW_RESET, "Reset");
    mnu.Append(ID_PREVIEW_VIEWPOINT_DEFAULT, _("Set Current ViewPoint as Default"));
    mnu.Append(ID_PREVIEW_VIEWPOINT_DEFAULT_RESTORE, _("Restore Default ViewPoint"));
    mnu.AppendSeparator();
    mnu.Append(ID_PREVIEW_SAVE_VIEWPOINT, _("Save Current ViewPoint"));
    if (is_3d) {
        if (xlights->viewpoint_mgr.GetNum3DCameras() > 0) {
            wxMenu* mnuViewPoint = new wxMenu();
            for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum3DCameras(); ++i)
            {
                mnuViewPoint->Append(xlights->viewpoint_mgr.GetCamera3D(i)->GetMenuId(), xlights->viewpoint_mgr.GetCamera3D(i)->GetName());
            }
            mnu.Append(ID_PREVIEW_VIEWPOINT3D, "Load ViewPoint", mnuViewPoint, "");
            mnuViewPoint->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);

            mnuViewPoint = new wxMenu();
            for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum3DCameras(); ++i)
            {
                mnuViewPoint->Append(xlights->viewpoint_mgr.GetCamera3D(i)->GetDeleteMenuId(), xlights->viewpoint_mgr.GetCamera3D(i)->GetName());
            }
            mnu.Append(ID_PREVIEW_DELETEVIEWPOINT3D, "Delete ViewPoint", mnuViewPoint, "");
            mnuViewPoint->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);
        }
    }
    else {
        if (xlights->viewpoint_mgr.GetNum2DCameras() > 0) {
            wxMenu* mnuViewPoint = new wxMenu();
            for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum2DCameras(); ++i)
            {
                mnuViewPoint->Append(xlights->viewpoint_mgr.GetCamera2D(i)->GetMenuId(), xlights->viewpoint_mgr.GetCamera2D(i)->GetName());
            }
            mnu.Append(ID_PREVIEW_VIEWPOINT2D, "Load ViewPoint", mnuViewPoint, "");
            mnuViewPoint->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);

            mnuViewPoint = new wxMenu();
            for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum2DCameras(); ++i)
            {
                mnuViewPoint->Append(xlights->viewpoint_mgr.GetCamera2D(i)->GetDeleteMenuId(), xlights->viewpoint_mgr.GetCamera2D(i)->GetName());
            }
            mnu.Append(ID_PREVIEW_DELETEVIEWPOINT2D, "Delete ViewPoint", mnuViewPoint, "");
            mnuViewPoint->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);
        }
    }

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);
    PopupMenu(&mnu);
    modelPreview->SetFocus();
}

void LayoutPanel::OnPreviewModelPopup(wxCommandEvent& event)
{
    if (event.GetId() == ID_PREVIEW_RESET) {
        modelPreview->Reset();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_RESET");
    } else if (event.GetId() == ID_PREVIEW_REPLACEMODEL) {
        ReplaceModel();
    } else if (event.GetId() == ID_PREVIEW_ALIGN_TOP) {
        if (editing_models) {
            PreviewModelAlignTops();
        } else {
            objects_panel->PreviewObjectAlignTops();
        }
    } else if (event.GetId() == ID_PREVIEW_SAVE_LAYOUT_IMAGE) {
        PreviewSaveImage();
    } else if (event.GetId() == ID_PREVIEW_PRINT_LAYOUT_IMAGE) {
        PreviewPrintImage();
    } else if (event.GetId() == ID_PREVIEW_IMPORTMODELSFROMRGBEFFECTS) {
        ImportModelsFromRGBEffects();
    } else if (event.GetId() == ID_PREVIEW_IMPORT_MODELS_FROM_LORS5) {
        ImportModelsFromLORS5();
    } else if (event.GetId() == ID_PREVIEW_ALIGN_BOTTOM) {
        if (editing_models) {
            PreviewModelAlignBottoms();
        } else {
            objects_panel->PreviewObjectAlignBottoms();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_GROUND) {
        if (editing_models) {
            PreviewModelAlignWithGround();
        } else {
            objects_panel->PreviewObjectAlignWithGround();
        }
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTION ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERSTARTNULLNODES ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERENDNULLNODES ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERBRIGHTNESS ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERCOLOURORDER ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERGAMMA ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERGROUPCOUNT ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERDIRECTION ||
               event.GetId() == ID_PREVIEW_BULKEDIT_SMARTREMOTE ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERPROTOCOL ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTIONINCREMENT ||
               event.GetId() == ID_PREVIEW_BULKEDIT_SMARTREMOTETYPE) {
        BulkEditControllerConnection(event.GetId());
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERNAME) {
        BulkEditControllerName();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_SETACTIVE) {
        BulkEditActive(true);
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_SETINACTIVE) {
        BulkEditActive(false);
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_TAGCOLOUR) {
        BulkEditTagColour();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_PIXELSIZE) {
        BulkEditPixelSize();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_PIXELSTYLE) {
        BulkEditPixelStyle();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_TRANSPARENCY) {
        BulkEditTransparency();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_BLACKTRANSPARENCY) {
        BulkEditBlackTranparency();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_PREVIEW) {
        BulkEditControllerPreview();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_DIMMINGCURVES) {
        BulkEditDimmingCurves();
    } else if (event.GetId() == ID_PREVIEW_ALIGN_LEFT) {
        if (editing_models) {
            PreviewModelAlignLeft();
        } else {
            objects_panel->PreviewObjectAlignLeft();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_RIGHT) {
        if (editing_models) {
            PreviewModelAlignRight();
        } else {
            objects_panel->PreviewObjectAlignRight();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_FRONT) {
        if (editing_models) {
            PreviewModelAlignFronts();
        } else {
            objects_panel->PreviewObjectAlignFronts();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_BACK) {
        if (editing_models) {
            PreviewModelAlignBacks();
        } else {
            objects_panel->PreviewObjectAlignBacks();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_H_CENTER) {
        if (editing_models) {
            PreviewModelAlignHCenter();
        } else {
            objects_panel->PreviewObjectAlignHCenter();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_V_CENTER) {
        if (editing_models) {
            PreviewModelAlignVCenter();
        } else {
            objects_panel->PreviewObjectAlignVCenter();
        }
    } else if (event.GetId() == ID_PREVIEW_H_DISTRIBUTE) {
        if (editing_models) {
            PreviewModelHDistribute();
        } else {
            objects_panel->PreviewObjectHDistribute();
        }
    } else if (event.GetId() == ID_PREVIEW_V_DISTRIBUTE) {
        if (editing_models) {
            PreviewModelVDistribute();
        } else {
            objects_panel->PreviewObjectVDistribute();
        }
    } else if (event.GetId() == ID_PREVIEW_RESIZE_SAMEWIDTH) {
        if (editing_models) {
            PreviewModelResize(true, false);
        } else {
            objects_panel->PreviewObjectResize(true, false);
        }
    } else if (event.GetId() == ID_PREVIEW_RESIZE_SAMEHEIGHT) {
        if (editing_models) {
            PreviewModelResize(false, true);
        } else {
            objects_panel->PreviewObjectResize(false, true);
        }
    } else if (event.GetId() == ID_PREVIEW_RESIZE_SAMESIZE) {
        if (editing_models) {
            PreviewModelResize(true, true);
        } else {
            objects_panel->PreviewObjectResize(true, true);
        }
    } else if (event.GetId() == ID_PREVIEW_MODEL_NODELAYOUT) {
        ShowNodeLayout();
    } else if (event.GetId() == ID_PREVIEW_MODEL_LOCK) {
        LockSelectedModels(true);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_LOCK");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_LOCK");
    } else if (event.GetId() == ID_PREVIEW_MODEL_UNLOCK) {
        LockSelectedModels(false);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_UNLOCK");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_UNLOCK");
    } else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTASCUSTOM) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        md->ExportAsCustomXModel();
    } else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTASCUSTOM3D) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        md->ExportAsCustomXModel3D();
    } else if (event.GetId() == ID_PREVIEW_MODEL_CREATEGROUP) {
        CreateModelGroupFromSelected();
    } else if (event.GetId() == ID_MNU_ADD_TO_EXISTING_GROUPS) {
        AddSelectedToExistingGroups();
    } else if (event.GetId() == ID_PREVIEW_MODEL_WIRINGVIEW) {
        ShowWiring();
    } else if (event.GetId() == ID_PREVIEW_MODEL_CAD_EXPORT) {
        ExportModelAsCAD();
    } else if (event.GetId() == ID_PREVIEW_LAYOUT_DXF_EXPORT) {
        ExportLayoutDXF();
    } else if (event.GetId() == ID_PREVIEW_MODEL_ASPECTRATIO) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int screen_wi = md->GetBaseObjectScreenLocation().GetMWidth();
        int screen_ht = md->GetBaseObjectScreenLocation().GetMHeight();
        float render_ht = md->GetBaseObjectScreenLocation().GetRenderHt();
        float render_wi = md->GetBaseObjectScreenLocation().GetRenderWi();
        float ht_ratio = render_ht / (float)screen_ht;
        float wi_ratio = render_wi / (float)screen_wi;
        if (ht_ratio > wi_ratio) {
            render_wi = render_wi / ht_ratio;
            md->GetBaseObjectScreenLocation().SetMWidth((int)render_wi);
        } else {
            render_ht = render_ht / wi_ratio;
            md->GetBaseObjectScreenLocation().SetMHeight((int)render_ht);
        }
        md->GetBaseObjectScreenLocation().Write(md->ModelXml);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ASPECTRATIO");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ASPECTRATIO");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ASPECTRATIO", nullptr, nullptr, GetSelectedModelName());
    } else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        md->ExportXlightsModel();
    } else if (event.GetId() == ID_PREVIEW_DELETE_ACTIVE) {
        DeleteCurrentPreview();
    } else if (event.GetId() == ID_PREVIEW_RENAME_ACTIVE) {
        RenameCurrentPreview();
    } else if (event.GetId() == ID_PREVIEW_MODEL_ADDPOINT) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int handle = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(handle + 0x8000));
        md->InsertHandle(handle, modelPreview->GetCameraZoomForHandles(), modelPreview->GetHandleScale());
        md->UpdateXmlWithScale();
        md->InitModel();
        // SetupPropGrid(md);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ADDPOINT");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ADDPOINT");
    } else if (event.GetId() == ID_PREVIEW_MODEL_DELETEPOINT) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int selected_handle = md->GetSelectedHandle();
        if ((selected_handle != -1) && (md->GetNumHandles() > 2)) {
            CreateUndoPoint("SingleModel", md->name, std::to_string(selected_handle + 0x4000));
            md->DeleteHandle(selected_handle);
            md->SelectHandle(-1);
            md->GetModelScreenLocation().SelectSegment(-1);
            md->UpdateXmlWithScale();
            md->InitModel();
            // SetupPropGrid(md);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_DELETEPOINT");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_DELETEPOINT");
        }
    } else if (event.GetId() == ID_PREVIEW_MODEL_ADDCURVE) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int seg = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(seg + 0x2000));
        md->SetCurve(seg, true);
        md->UpdateXmlWithScale();
        md->InitModel();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ADDCURVE");
    } else if (event.GetId() == ID_PREVIEW_MODEL_DELCURVE) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int seg = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(seg + 0x1000));
        md->SetCurve(seg, false);
        md->UpdateXmlWithScale();
        md->InitModel();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_DELCURVE");
    } else if (event.GetId() == ID_PREVIEW_VIEWPOINT_DEFAULT) {
        modelPreview->SaveDefaultCameraPosition();
    } else if (event.GetId() == ID_PREVIEW_VIEWPOINT_DEFAULT_RESTORE) {
        modelPreview->RestoreDefaultCameraPosition();
    } else if (event.GetId() == ID_PREVIEW_SAVE_VIEWPOINT) {
        modelPreview->SaveCurrentCameraPosition();
        SetDirtyHiLight(true);
    } else if (event.GetId() == ID_PREVIEW_FLIP_HORIZONTAL) {
        if (editing_models) {
            PreviewModelFlipH();
        } else {
            objects_panel->PreviewObjectFlipH();
        }
    } else if (event.GetId() == ID_PREVIEW_FLIP_VERTICAL) {
        if (editing_models) {
            PreviewModelFlipV();
        } else {
            objects_panel->PreviewObjectFlipV();
        }
    } else if (is_3d) {
        if (xlights->viewpoint_mgr.GetNum3DCameras() > 0) {
            for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum3DCameras(); ++i) {
                if (event.GetId() == xlights->viewpoint_mgr.GetCamera3D(i)->GetMenuId()) {
                    modelPreview->SetCamera3D(i);
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::3dCamera");
                    break;
                } else if (event.GetId() == xlights->viewpoint_mgr.GetCamera3D(i)->GetDeleteMenuId()) {
                    xlights->viewpoint_mgr.DeleteCamera3D(i);
                }
            }
        }
    } else {
        if (xlights->viewpoint_mgr.GetNum2DCameras() > 0) {
            for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum2DCameras(); ++i) {
                if (event.GetId() == xlights->viewpoint_mgr.GetCamera2D(i)->GetMenuId()) {
                    modelPreview->SetCamera2D(i);
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::2dCamera");
                    break;
                } else if (event.GetId() == xlights->viewpoint_mgr.GetCamera2D(i)->GetDeleteMenuId()) {
                    xlights->viewpoint_mgr.DeleteCamera2D(i);
                }
            }
        }
    }
}

void LayoutPanel::PreviewModelAlignWithGround()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->GroupSelected || modelPreview->GetModels()[i]->Selected)
        {
            modelPreview->GetModels()[i]->SetBottom(0.0F);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::PreviewModelAlignWithGround");
    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignWithGround");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::EditSubmodels()
{
    Model* md = dynamic_cast<Model*>(selectedBaseObject);
    if (md == nullptr || md->GetDisplayAs() == "ModelGoup" || md->GetDisplayAs() == "SubModel")
        return;

    SubModelsDialog dlg(this, &xlights->_outputManager);
    dlg.Setup(md);
    if (dlg.ShowModal() == wxID_OK) {
        dlg.Save();
        md->IncrementChangeCount();
        md->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::EditSubmodels");
    }
    if (dlg.ReloadLayout) { //force grid to reload
        wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
        wxPostEvent(md->GetModelManager().GetXLightsFrame(), eventForceRefresh);
        md->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::EditSubmodels");
        md->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::EditSubmodels");
    }
}

void LayoutPanel::EditFaces()
{
    Model* md = dynamic_cast<Model*>(selectedBaseObject);
    if (md == nullptr || md->GetDisplayAs() == "ModelGoup" || md->GetDisplayAs() == "SubModel")
        return;

    ModelFaceDialog dlg(this, &xlights->_outputManager);
    dlg.SetFaceInfo(md, md->faceInfo);
    if (dlg.ShowModal() == wxID_OK) {
        md->faceInfo.clear();
        dlg.GetFaceInfo(md->faceInfo);
        md->IncrementChangeCount();
        md->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::EditFaces");
    }
}

void LayoutPanel::EditStates()
{
    Model* md = dynamic_cast<Model*>(selectedBaseObject);
    if (md == nullptr || md->GetDisplayAs() == "ModelGoup" || md->GetDisplayAs() == "SubModel")
        return;

    ModelStateDialog dlg(this, &xlights->_outputManager);
    dlg.SetStateInfo(md, md->stateInfo);
    if (dlg.ShowModal() == wxID_OK) {
        md->stateInfo.clear();
        dlg.GetStateInfo(md->stateInfo);
        md->IncrementChangeCount();
        md->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::EditStates");
    }
}

void LayoutPanel::EditModelData()
{
    CustomModel* md = dynamic_cast<CustomModel*>(selectedBaseObject);
    if (md == nullptr || md->GetDisplayAs() != "Custom")
        return;

    md->SaveDisplayDimensions();
    auto oldAutoSave = md->GetModelManager().GetXLightsFrame()->_suspendAutoSave;
    md->GetModelManager().GetXLightsFrame()->_suspendAutoSave = true; // because we will tamper with model we need to suspend autosave
    CustomModelDialog dlg(this, &xlights->_outputManager);
    dlg.Setup(md);
    if (dlg.ShowModal() == wxID_OK) {
        dlg.Save(md);
        md->RestoreDisplayDimensions();
        md->IncrementChangeCount();
        md->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::EditModelData");
    } else {
        md->RestoreDisplayDimensions();
        md->GetModelManager().GetXLightsFrame()->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CustomModel::CancelCustomData");
    }
    md->GetModelManager().GetXLightsFrame()->_suspendAutoSave = oldAutoSave;
}

void LayoutPanel::ShowNodeLayout()
{
    Model* md = dynamic_cast<Model*>(selectedBaseObject);
    if (md == nullptr || md->GetDisplayAs() == "ModelGoup" || md->GetDisplayAs() == "SubModel") return;
    wxString html = md->ChannelLayoutHtml(xlights->GetOutputManager());
    ChannelLayoutDialog dialog(this);
    dialog.SetHtmlSource(html);
    dialog.ShowModal();
}

void LayoutPanel::ShowWiring()
{
    Model* md = dynamic_cast<Model*>(selectedBaseObject);
    if (md == nullptr || md->GetDisplayAs() == "ModelGoup" || md->GetDisplayAs() == "SubModel") return;
    WiringDialog dlg(this, md->GetName());
    dlg.SetData(md);
    dlg.ShowModal();
}

void LayoutPanel::ExportModelAsCAD()
{
    Model* md = dynamic_cast<Model*>(selectedBaseObject);
    if (md == nullptr || md->GetDisplayAs() == "ModelGoup" || md->GetDisplayAs() == "SubModel") return;

#ifdef __WXOSX__
    static std::string const filter = "DXF STL VRML File (*.dxf;*.stl;*.wrl)|*.dxf;*.stl;*.wrl";
#else
    static std::string const filter = "DXF File (*.dxf)|*.dxf|STL File (*.stl)|*.stl|VRML File (*.wrl)|*.wrl";
#endif

    wxString const filename = wxFileSelector(_("Choose Output File"), wxEmptyString, md->GetName(), wxEmptyString, filter, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename != "") {
        wxFileName file(filename);
        if (ModelToCAD::ExportCAD(md, filename, file.GetExt())) {
            xlights->SetStatusText(wxString::Format("Exported '%s' Successfully", filename));
        } else {
            xlights->SetStatusText(wxString::Format("Export Failed '%s'", filename));
        }
    }
}

void LayoutPanel::ExportLayoutDXF()
{
    wxString const filename = wxFileSelector(_("Choose output file"), wxEmptyString, "Layout", wxEmptyString, "DXF File (*.dxf)|*.dxf", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename != "") {
        if (ModelToCAD::ExportCAD(&xlights->AllModels, filename, "dxf")) {
            xlights->SetStatusText(wxString::Format("Exported '%s' Successfully", filename));
        } else {
            xlights->SetStatusText(wxString::Format("Export Failed '%s'", filename));
        }
    }
}

void LayoutPanel::PreviewModelAlignTops()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float top = modelPreview->GetModels()[selectedindex]->GetTop();
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetTop(top);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignTops");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelAlignBottoms()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float bottom = modelPreview->GetModels()[selectedindex]->GetBottom();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetBottom(bottom);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignBottoms");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelAlignLeft()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float left = modelPreview->GetModels()[selectedindex]->GetLeft();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetLeft(left);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignLeft");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelAlignFronts()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float front = modelPreview->GetModels()[selectedindex]->GetFront();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetFront(front);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignFronts");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelAlignBacks()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float back = modelPreview->GetModels()[selectedindex]->GetBack();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetBack(back);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignBacks");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelResize(bool sameWidth, bool sameHeight)
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    if (!xlights->AbortRender()) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);

    Model* selectedModel = modelPreview->GetModels()[selectedindex];
    std::string selectedType = selectedModel->GetDisplayAs();
    float width = selectedModel->GetWidth();
    float height = selectedModel->GetHeight();

    bool isBoxed = false;
    if ((dynamic_cast<ModelWithScreenLocation<BoxedScreenLocation>*>(selectedModel) != nullptr)) {
        isBoxed = true;
    }

    bool isCustom3d = false;
    wxString customFingerprint = "";

    // check if custom 3d and set model fingerprint
    if (selectedType == "Custom") {
        customFingerprint = selectedModel->GetModelXml()->GetAttribute("CustomModel", "");
        if (wxSplit(customFingerprint, '|').size() > 1) {
            isCustom3d = true;
        }
    }

    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->GroupSelected)
        {
            Model* modelToResize = modelPreview->GetModels()[i];
            std::string modelType = modelToResize->GetDisplayAs();
            bool custom3dPrintsMatch = false;

            // if selected is Custom3d check if model fingerprints match
            if (isCustom3d && customFingerprint != "") {
                wxString mToResizeFingerprint = modelToResize->GetModelXml()->GetAttribute("CustomModel", "");

                if (customFingerprint == mToResizeFingerprint) {
                    custom3dPrintsMatch = true;
                }
            }

            if ((isBoxed && selectedType == modelType && selectedType != "Custom") || custom3dPrintsMatch) {
                // boxed model, types match and not a custom model OR custom 3d model and fingerprints matched so use scale matrix
                glm::vec3 matrixScale = selectedModel->GetModelScreenLocation().GetScaleMatrix();
                if (sameWidth && sameHeight) {
                    modelToResize->GetModelScreenLocation().SetScaleMatrix(matrixScale);
                } else if (sameWidth) {
                    float scaleY = ((BoxedScreenLocation&)modelToResize->GetModelScreenLocation()).GetScaleY();
                    ((BoxedScreenLocation&)modelToResize->GetModelScreenLocation()).SetScale(matrixScale.x, scaleY);
                    ((BoxedScreenLocation&)modelToResize->GetModelScreenLocation()).SetScaleZ(matrixScale.z);
                } else {
                    float scaleX = ((BoxedScreenLocation&)modelToResize->GetModelScreenLocation()).GetScaleX();
                    ((BoxedScreenLocation&)modelToResize->GetModelScreenLocation()).SetScale(scaleX, matrixScale.y);
                    ((BoxedScreenLocation&)modelToResize->GetModelScreenLocation()).SetScaleZ(matrixScale.z);
                }
            } else {
                // no special resizing, same as 2020.24 and prior
                bool z_scale = modelPreview->GetModels()[i]->GetBaseObjectScreenLocation().GetSupportsZScaling();

                if (sameWidth) {
                    modelPreview->GetModels()[i]->SetWidth(width);
                    if (z_scale) {
                        modelPreview->GetModels()[i]->GetBaseObjectScreenLocation().SetMDepth(width);
                    }
                }

                if (sameHeight) {
                    modelToResize->SetHeight(height);
                }
            }
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignResize");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelAlignRight()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float right = modelPreview->GetModels()[selectedindex]->GetRight();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetRight(right);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignRight");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelAlignHCenter()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float center = modelPreview->GetModels()[selectedindex]->GetHcenterPos();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetHcenterPos(center);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelAlignHCenter");

    ReselectTreeModels(selectedModelPaths);
}

bool SortModelX(const Model* first, const Model* second)
{
    float firstmodelX = first->GetBaseObjectScreenLocation().GetHcenterPos();
    float secondmodelX = second->GetBaseObjectScreenLocation().GetHcenterPos();

    return firstmodelX < secondmodelX;
}

bool SortModelY(const Model* first, const Model* second)
{
    float firstmodelY = first->GetBaseObjectScreenLocation().GetVcenterPos();
    float secondmodelY = second->GetBaseObjectScreenLocation().GetVcenterPos();

    return firstmodelY < secondmodelY;
}

void LayoutPanel::PreviewModelHDistribute()
{
    int count = 0;
    float minx = 999999;
    float maxx = -999999;

    std::list<Model*> models;

    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        Model* m = modelPreview->GetModels()[i];
        if (m->GroupSelected || m->Selected)
        {
            count++;
            float x = m->GetHcenterPos();

            if (x < minx) minx = x;
            if (x > maxx) maxx = x;
            models.push_back(m);
        }
    }

    if (count <= 2) return;

    models.sort(SortModelX);

    float space = (maxx - minx) / (count - 1);

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", models.front()->name);

    float x = -1;
    for (const auto& it : models)
    {
        if (it == models.front())
        {
            x = it->GetHcenterPos() + space;
        }
        else if (it == models.back())
        {
            // do nothing
        }
        else
        {
            it->SetHcenterPos(x);
            x += space;
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelHDistribute");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelVDistribute()
{
    int count = 0;
    float miny = 999999;
    float maxy = -999999;

    std::list<Model*> models;

    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        Model* m = modelPreview->GetModels()[i];
        if (m->GroupSelected || m->Selected)
        {
            count++;
            float y = m->GetVcenterPos();

            if (y < miny) miny = y;
            if (y > maxy) maxy = y;
            models.push_back(m);
        }
    }

    if (count <= 2) return;

    models.sort(SortModelY);

    float space = (maxy - miny) / (count - 1);

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", models.front()->name);

    float y = -1;
    for (const auto& it : models)
    {
        if (it == models.front())
        {
            y = it->GetVcenterPos() + space;
        }
        else if (it == models.back())
        {
            // do nothing
        }
        else
        {
            it->SetVcenterPos(y);
            y += space;
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelVDistribute");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelAlignVCenter()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float center = modelPreview->GetModels()[selectedindex]->GetVcenterPos();
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetVcenterPos(center);
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelVCenter");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelFlipV() {
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) {
        return;
    }

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);

    for (auto model : modelPreview->GetModels()) {
        if (model->Selected) {
            model->FlipVertical();
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelVCenter");

    ReselectTreeModels(selectedModelPaths);
}

void LayoutPanel::PreviewModelFlipH() {
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) {
        return;
    }

    std::vector<std::list<std::string>> selectedModelPaths = GetSelectedTreeModelPaths();

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);

    for (auto model : modelPreview->GetModels()) {
        if (model->Selected) {
            model->FlipHorizontal();
        }
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::PreviewModelVCenter");

    ReselectTreeModels(selectedModelPaths);    
}

int LayoutPanel::GetSelectedModelIndex() const
{
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->Selected)
        {
            return i;
        }
    }
    return -1;
}

// Shortcut to get model pointer from tree item
Model* LayoutPanel::GetModelFromTreeItem(wxTreeListItem treeItem) {
    ModelTreeData *data = (ModelTreeData*)TreeListViewModels->GetItemData(treeItem);
    Model* model = ((data != nullptr) ? data->GetModel() : nullptr);
    return model;
}

// Select a Model in the tree, currently only selects top level model if found
void LayoutPanel::SelectModelInTree(Model* modelToSelect) {
    for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
          item.IsOk();
          item = TreeListViewModels->GetNextSibling(item) )
    {
        if (TreeListViewModels->GetItemData(item) != nullptr)
        {
            ModelTreeData *mitem = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
            if (mitem != nullptr && mitem->GetModel() == modelToSelect) {
                TreeListViewModels->Select(item);

                PlatformHandleSelectionChanged();
                TreeListViewModels->EnsureVisible(item);
                break;
            }
        }
    }
}

// Unselect a Model in the tree, currently only unselects top level model if found
void LayoutPanel::UnSelectModelInTree(Model* modelToUnSelect) {
    // clear any flags used for preview
    modelToUnSelect->Selected = false;
    modelToUnSelect->Highlighted = false;
    modelToUnSelect->GroupSelected = false;
    modelToUnSelect->SelectHandle(-1);
    modelToUnSelect->GetBaseObjectScreenLocation().SetActiveHandle(-1);

    for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
          item.IsOk();
          item = TreeListViewModels->GetNextSibling(item) )
    {
        if (TreeListViewModels->GetItemData(item) != nullptr)
        {
            ModelTreeData *mitem = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
            if (mitem != nullptr && mitem->GetModel() == modelToUnSelect) {
                TreeListViewModels->Unselect(item);
                PlatformHandleSelectionChanged();
                break;
            }
        }
    }
}

// Select a BaseObject in the model tree
void LayoutPanel::SelectBaseObjectInTree(BaseObject* baseObjectToSelect) {
    Model* modelToSelect = dynamic_cast<Model*>(baseObjectToSelect);
    SelectModelInTree(modelToSelect);
}

// Perform unselect of tree item by BaseObject
void LayoutPanel::UnSelectBaseObjectInTree(BaseObject* baseObjectToUnSelect) {
    Model* modelToUnSelect = dynamic_cast<Model*>(baseObjectToUnSelect);
    UnSelectModelInTree(modelToUnSelect);
}

// Get first tree item for a model, 'first' is the key word as a model pointer could exist in numerous
// places in model tree.
wxTreeListItem LayoutPanel::GetTreeItemFromModel(Model* model) {
    wxTreeListItem modelTreeItem = nullptr;
    for (wxTreeListItem item = TreeListViewModels->GetFirstItem(); item.IsOk(); item = TreeListViewModels->GetNextItem(item)) {
        ModelTreeData *data = (ModelTreeData*)TreeListViewModels->GetItemData(item);
        Model* itemModel = ((data != nullptr) ? data->GetModel() : nullptr);
        if (model == itemModel) {
            modelTreeItem = item;
            break;
        }
    }

    return modelTreeItem;
}

void LayoutPanel::UnSelectAllModelsInTree() {
    TreeListViewModels->UnselectAll();
    PlatformHandleSelectionChanged();
}

// Get unique models from selected tree model group included those deeply nested
std::vector<Model *> LayoutPanel::GetSelectedModelsFromGroup(wxTreeListItem groupItem, bool nested) {
    std::vector<Model *> groupModels;

    if (groupItem.IsOk()) {
        for (wxTreeListItem item = TreeListViewModels->GetFirstChild(groupItem);
              item.IsOk();
              item = TreeListViewModels->GetNextSibling(item) )
        {
            ModelTreeData *data = (ModelTreeData*)TreeListViewModels->GetItemData(item);
            Model* model = ((data != nullptr) ? data->GetModel() : nullptr);

            if (model->GetDisplayAs() == "ModelGroup" && nested == true) {
                std::vector<Model *> nestedModels = GetSelectedModelsFromGroup(item, true);
                for (Model* nestedModel: nestedModels) {
                    if (std::find(groupModels.begin(), groupModels.end(), nestedModel) == groupModels.end()) {
                        groupModels.push_back(nestedModel);
                    }
                }
            } else {
                if (std::find(groupModels.begin(), groupModels.end(), model) == groupModels.end()) {
                    groupModels.push_back(model);
                }
            }
        }

    }

    return groupModels;
}

// The will return unique selected models for edit, useful when groups are also selected in model tree
std::vector<Model*> LayoutPanel::GetSelectedModelsForEdit() {
    std::vector<Model*> modelsForEdit;

    for (const auto& groupItem : selectedTreeGroups) {
        std::vector<Model*> groupModels = GetSelectedModelsFromGroup(groupItem);
        for (Model* model: groupModels) {
            if (std::find(modelsForEdit.begin(), modelsForEdit.end(), model) == modelsForEdit.end()) {
                if (model->GetDisplayAs() != "SubModel") {
                    modelsForEdit.push_back(model);
                }
            }
        }
    }

    for (const auto& modelItem : selectedTreeModels) {
        Model* model = GetModelFromTreeItem(modelItem);
        if (std::find(modelsForEdit.begin(), modelsForEdit.end(), model) == modelsForEdit.end()) {
            modelsForEdit.push_back(model);
        }
    }

    return modelsForEdit;
}

void LayoutPanel::SetTreeModelSelected(Model* model, bool isPrimary) {
    if (isPrimary) {
        model->Selected = true;
        model->GroupSelected = false;
        selectionLatched = true;
        selectedBaseObject = model;
        highlightedBaseObject = model;
        selectedBaseObject->Highlighted = true;
        selectedBaseObject->SelectHandle(-1);
        selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
        if (CheckBoxOverlap->GetValue() == true) {
            CheckModelForOverlaps(model);
        }
    } else {
        model->Selected = false;
        model->GroupSelected = true;
        model->Highlighted = true;
        model->SelectHandle(-1);
        model->GetBaseObjectScreenLocation().SetActiveHandle(-1);
    }
}

void LayoutPanel::SetTreeGroupModelsSelected(Model* model, bool isPrimary) {
    if (isPrimary) {
        selectedBaseObject = model;
        selectionLatched = true;
    }

    std::vector<Model *> groupModels = GetSelectedModelsFromGroup(GetTreeItemFromModel(model), true);

    for (const auto& m : groupModels) {
        m->GroupSelected = true;
        m->Highlighted = true;
        model->SelectHandle(-1);
        model->GetBaseObjectScreenLocation().SetActiveHandle(-1);
    }
}

void LayoutPanel::SetTreeSubModelSelected(Model* model, bool isPrimary) {
    if (isPrimary) {
        selectedBaseObject = model;
        highlightedBaseObject = model;
        selectionLatched = true;
    }
    model->GroupSelected = true;
    model->Highlighted = true;
    model->SelectHandle(-1);
    model->GetBaseObjectScreenLocation().SetActiveHandle(-1);
}

void LayoutPanel::CheckModelForOverlaps(Model* model) {
    int mStart = model->GetNumberFromChannelString(model->ModelStartChannel);
    int mEnd = model->GetLastChannel();

    for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
          item.IsOk();
          item = TreeListViewModels->GetNextSibling(item) ) {
        if (TreeListViewModels->GetItemData(item) != nullptr) {
            ModelTreeData *data = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
            Model *mm = data != nullptr ? data->GetModel() : nullptr;
            if (mm != nullptr && mm != selectedBaseObject) {
                int startChan = mm->GetNumberFromChannelString(mm->ModelStartChannel);
                int endChan = mm->GetLastChannel();
                if ((startChan >= mStart) && (endChan <= mEnd)) {
                    mm->Overlapping = true;
                }
                else if ((startChan >= mStart) && (startChan <= mEnd)) {
                    mm->Overlapping = true;
                }
                else if ((endChan >= mStart) && (endChan <= mEnd)) {
                    mm->Overlapping = true;
                }
                else {
                    mm->Overlapping = false;
                }
            }
        }
    }
}

std::vector<std::list<std::string>> LayoutPanel::GetSelectedTreeModelPaths() {
    std::vector<std::list<std::string>> modelPaths;

    // Add selectedPrimaryTreeItem first if exists
    if (selectedPrimaryTreeItem != nullptr && selectedPrimaryTreeItem.IsOk()) {
        std::list<std::string> baseItemPath = GetTreeItemPath(selectedPrimaryTreeItem);
        modelPaths.push_back(baseItemPath);
    }

    for (const auto& item : selectedTreeGroups) {
        if (selectedPrimaryTreeItem != item) {
            std::list<std::string> modelPath = GetTreeItemPath(item);
            modelPaths.push_back(modelPath);
        }
    }

    for (const auto& item : selectedTreeModels) {
        if (selectedPrimaryTreeItem != item) {
            std::list<std::string> modelPath = GetTreeItemPath(item);
            modelPaths.push_back(modelPath);
        }
    }

    for (const auto& item : selectedTreeSubModels) {
        if (selectedPrimaryTreeItem != item) {
            std::list<std::string> modelPath = GetTreeItemPath(item);
            modelPaths.push_back(modelPath);
        }
    }

    return modelPaths;
}

std::list<std::string> LayoutPanel::GetTreeItemPath(wxTreeListItem item) {
    std::list<std::string> itemPath;
    wxTreeListItem root = TreeListViewModels->GetRootItem();
    itemPath.push_back(TreeListViewModels->GetItemText(item));
    wxTreeListItem parent = TreeListViewModels->GetItemParent(item);
    while (parent.IsOk() && parent != root) {
        itemPath.push_front(TreeListViewModels->GetItemText(parent));
        parent = TreeListViewModels->GetItemParent(parent);
    }

    return itemPath;
}

wxTreeListItem LayoutPanel::GetTreeItemBranch(wxTreeListItem parent, std::string branchName) {
    wxTreeListItem branch = TreeListViewModels->GetFirstChild(parent);
    bool branchFound = false;
    while(branch.IsOk() && !branchFound) {
        if (branchName == TreeListViewModels->GetItemText(branch)) {
            branchFound = true;
        } else {
            branch = TreeListViewModels->GetNextSibling(branch);
        }
    }

    return branch;
}

void LayoutPanel::ReselectTreeModels(std::vector<std::list<std::string>> modelPaths) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    for (auto path : modelPaths) {
        // model name is last string in path
        std::string modelName = path.back();
        path.pop_back();
        wxTreeListItem branch = TreeListViewModels->GetRootItem();
        for (auto part : path) {
            branch = GetTreeItemBranch(branch, part);
        }

        if (branch != nullptr && branch.IsOk()) {
            for (wxTreeListItem child = TreeListViewModels->GetFirstChild(branch); child.IsOk(); child = TreeListViewModels->GetNextSibling(child)) {
                std::string childName = TreeListViewModels->GetItemText(child);
                if (TreeListViewModels->GetItemText(child) == modelName) {
                    TreeListViewModels->Select(child);
                    PlatformHandleSelectionChanged();
                    break;
                }
            }
        } else {
            logger_base.crit("LayoutPanel::ReselectTreeModels branch could not be found in tree... this shouldn't happen.");
        }
    }
}

int LayoutPanel::ModelsSelectedCount() const
{
    int selectedModelCount = 0;
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->Selected || modelPreview->GetModels()[i]->GroupSelected)
        {
            selectedModelCount++;
        }
    }

    return selectedModelCount;
}

int LayoutPanel::ViewObjectsSelectedCount() const
{
    int selectedObjectCount = 0;
    for (const auto& it : xlights->AllObjects) {
        ViewObject *view_object = it.second;
        if(view_object->Selected || view_object->GroupSelected)
        {
            selectedObjectCount++;
        }
    }
    return selectedObjectCount;
}

void LayoutPanel::OnModelSplitterSashPosChanged(wxSplitterEvent& event)
{
    if (ModelGroupWindow == nullptr) {
        //event during creation
        return;
    }
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("LayoutModelSplitterSash", event.GetSashPosition());
}

void LayoutPanel::OnSplitterWindowSashPosChanged(wxSplitterEvent& event)
{
    if (ModelGroupWindow == nullptr) {
        //event during creation
        return;
    }
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("LayoutMainSplitterSash", event.GetSashPosition());
}

void LayoutPanel::OnNewModelTypeButtonClicked(wxCommandEvent& event) {
    for (const auto& it : buttons) {
        if (event.GetId() == it->GetId()) {
            if (it->GetModelType() == "Add Object") {
                DisplayAddObjectPopup();
            } else if (it->GetModelType() == "DMX") {
                selectedButton = it;
                DisplayAddDmxPopup();
            } else {
                int state = it->GetState();
                it->SetState(state + 1);
                if (it->GetState()) {
                    selectedButton = it;
                    UnSelectAllModels();
                    modelPreview->SetFocus();
                } else {
                    selectedButton = nullptr;
                    _lastXlightsModel = "";
                }
                Notebook_Objects->ChangeSelection(0);
                editing_models = true;
            }
        } else if (it->GetState()) {
            it->SetState(0);
        }
    }
}

void LayoutPanel::AddObjectButton(wxMenu& mnu, const long id, const std::string &name, const char *icon[]) {
    wxMenuItem* menu_item = mnu.Append(id, name);
    if (icon != nullptr) {
        wxImage image(icon);
        wxImage halfImg = image.Scale(24, 24, wxIMAGE_QUALITY_HIGH);
        menu_item->SetBitmap(wxBitmapBundle::FromBitmaps(halfImg, image));
    }
}

void LayoutPanel::DisplayAddObjectPopup() {
    wxMenu mnuObjects;
    AddObjectButton(mnuObjects, ID_ADD_OBJECT_IMAGE, "Image", add_object_image_xpm);
    AddObjectButton(mnuObjects, ID_ADD_OBJECT_GRIDLINES, "Gridlines", add_object_gridlines_xpm);
    AddObjectButton(mnuObjects, ID_ADD_OBJECT_TERRIAN, "Terrian", add_object_terrian_xpm);
    AddObjectButton(mnuObjects, ID_ADD_OBJECT_MESH, "Mesh", add_object_mesh_xpm);
    if (RulerObject::GetRuler() == nullptr) {
        AddObjectButton(mnuObjects, ID_ADD_OBJECT_RULER, "Ruler", add_object_ruler_xpm);
    }
    mnuObjects.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&LayoutPanel::OnAddObjectPopup, nullptr, this);
    PopupMenu(&mnuObjects);
}

void LayoutPanel::OnAddObjectPopup(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    int id = event.GetId();
    ViewObject* vobj = nullptr;
    bool object_created = false;
    if (id == ID_ADD_OBJECT_IMAGE)
    {
        logger_base.debug("OnAddObjectPopup - ID_ADD_OBJECT_IMAGE");
        CreateUndoPoint("All", "", "");
        vobj = xlights->AllObjects.CreateAndAddObject("Image");
        vobj->SetLayoutGroup("Default"); // only Default supports 3D and hence objects
        objects_panel->UpdateObjectList(true, currentLayoutGroup);
        object_created = true;
    }
    else if (id == ID_ADD_OBJECT_GRIDLINES)
    {
        logger_base.debug("OnAddObjectPopup - ID_ADD_OBJECT_GRIDLINES");
        CreateUndoPoint("All", "", "");
        vobj = xlights->AllObjects.CreateAndAddObject("Gridlines");
        vobj->SetLayoutGroup("Default"); // only Default supports 3D and hence objects
        objects_panel->UpdateObjectList(true, currentLayoutGroup);
        object_created = true;
    }
    else if (id == ID_ADD_OBJECT_TERRIAN)
    {
        logger_base.debug("OnAddObjectPopup - ID_ADD_OBJECT_TERRIAN");
        CreateUndoPoint("All", "", "");
        vobj = xlights->AllObjects.CreateAndAddObject("Terrian");
        vobj->SetLayoutGroup("Default"); // only Default supports 3D and hence objects
        objects_panel->UpdateObjectList(true, currentLayoutGroup);
        object_created = true;
    }
    else if (id == ID_ADD_OBJECT_RULER) {
        logger_base.debug("OnAddObjectPopup - ID_ADD_OBJECT_RULER");
        CreateUndoPoint("All", "", "");
        vobj = xlights->AllObjects.CreateAndAddObject("Ruler");
        vobj->SetLayoutGroup("Default"); // only Default supports 3D and hence objects
        vobj->GetObjectScreenLocation().SetVcenterPos(100);
        vobj->GetObjectScreenLocation().SetLeft(-50);
        vobj->GetObjectScreenLocation().SetMWidth(100);
        objects_panel->UpdateObjectList(true, currentLayoutGroup);
        object_created = true;
    }
    else if (id == ID_ADD_OBJECT_MESH)
    {
        logger_base.debug("OnAddObjectPopup - ID_ADD_OBJECT_MESH");
        CreateUndoPoint("All", "", "");
        vobj = xlights->AllObjects.CreateAndAddObject("Mesh");
        vobj->SetLayoutGroup("Default"); // only Default supports 3D and hence objects
        objects_panel->UpdateObjectList(true, currentLayoutGroup);
        object_created = true;
    }

    if( object_created ) {
        Notebook_Objects->ChangeSelection(1);
        editing_models = false;
        SelectViewObject(vobj, true);
        vobj->UpdateXmlWithScale();
        //SetupPropGrid(vobj);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_ADD_OBJECT_MESH");
    }

    Refresh();
}

void LayoutPanel::DisplayAddDmxPopup() {
    wxMenu mnuObjects;
    AddObjectButton(mnuObjects, ID_ADD_DMX_GENERAL, "General", add_dmx_general_xpm);
    AddObjectButton(mnuObjects, ID_ADD_DMX_FLOODLIGHT, "Floodlight", add_dmx_floodlight_xpm);
    AddObjectButton(mnuObjects, ID_ADD_DMX_FLOODAREA, "Area Flood", add_dmx_floodlight_xpm);
    AddObjectButton(mnuObjects, ID_ADD_DMX_MOVING_HEAD_3D, "Moving Head 3D", dmx_xpm);
    AddObjectButton(mnuObjects, ID_ADD_DMX_MOVING_HEAD, "Moving Head", add_dmx_moving_head_xpm);
    AddObjectButton(mnuObjects, ID_ADD_DMX_SERVO, "Servo", add_dmx_servo_xpm);
    if (is_3d) {
        AddObjectButton(mnuObjects, ID_ADD_DMX_SERVO_3D, "Servo 3D", add_dmx_servo3d_xpm);
    }
    AddObjectButton(mnuObjects, ID_ADD_DMX_SKULL, "Skull", add_dmx_skulltronix_xpm);
    mnuObjects.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&LayoutPanel::OnAddDmxPopup, nullptr, this);
    selectedDmxModelType = "";
    PopupMenu(&mnuObjects);
}

void LayoutPanel::OnAddDmxPopup(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    int id = event.GetId();
    bool object_created = false;
    if (id == ID_ADD_DMX_FLOODLIGHT) {
        logger_base.debug("OnAddDmxPopup - ID_ADD_DMX_FLOODLIGHT");
        selectedDmxModelType = "DmxFloodlight";
        object_created = true;
    }
    else if (id == ID_ADD_DMX_GENERAL) {
        logger_base.debug("OnAddDmxPopup - ID_ADD_DMX_GENERAL");
        selectedDmxModelType = "DmxGeneral";
        object_created = true;
    }
    else if (id == ID_ADD_DMX_FLOODAREA) {
        logger_base.debug("OnAddDmxPopup - ID_ADD_DMX_FLOODAREA");
        selectedDmxModelType = "DmxFloodArea";
        object_created = true;
    }
    else if (id == ID_ADD_DMX_MOVING_HEAD) {
        logger_base.debug("OnAddDmxPopup - ID_ADD_DMX_MOVING_HEAD");
        selectedDmxModelType = "DmxMovingHead";
        object_created = true;
    }
    else if (id == ID_ADD_DMX_MOVING_HEAD_3D) {
        logger_base.debug("OnAddDmxPopup - ID_ADD_DMX_MOVING_HEAD_3D");
        selectedDmxModelType = "DmxMovingHead3D";
        object_created = true;
    }
    else if (id == ID_ADD_DMX_SERVO) {
        logger_base.debug("OnAddDmxPopup - ID_ADD_DMX_SERVO");
        selectedDmxModelType = "DmxServo";
        object_created = true;
    }
    else if (id == ID_ADD_DMX_SERVO_3D) {
        logger_base.debug("OnAddDmxPopup - ID_ADD_DMX_SERVO_3D");
        selectedDmxModelType = "DmxServo3d";
        object_created = true;
    }
    else if (id == ID_ADD_DMX_SKULL) {
        logger_base.debug("OnAddDmxPopup - ID_ADD_DMX_SKULL");
        selectedDmxModelType = "DmxSkull";
        object_created = true;
    }

    if (object_created) {
        UnSelectAllModels();
        modelPreview->SetFocus();
        Notebook_Objects->ChangeSelection(0);
        editing_models = true;
    }
}

Model *LayoutPanel::CreateNewModel(const std::string &type) const
{
    std::string t = type;
    if (t == "Import Custom" || t == "Download")
    {
        t = "Custom";
    }
    //std::string startChannel = xlights->AllModels.GenerateNewStartChannel( lastModelName );

    Model* m = xlights->AllModels.CreateDefaultModel(t, "1");

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "LayoutPanel::CreateNewModel");

    return m;
}

std::list<BaseObject*> LayoutPanel::GetSelectedBaseObjects() const
{
    std::list<BaseObject*> res;

    if (selectedBaseObject != nullptr)
    {
        res.push_back(selectedBaseObject);
    }

    if (editing_models)
    {
        for (const auto& it : modelPreview->GetModels())
        {
            if (it != selectedBaseObject && (it->Selected || it->GroupSelected))
            {
                res.push_back(it);
            }
        }
    }

    return res;
}

void LayoutPanel::Nudge(int key)
{
    std::list<BaseObject*> selectedBaseObjects = GetSelectedBaseObjects();
    if (selectedBaseObjects.size() > 0)
    {
        if (selectedBaseObjects.size() == 1)
        {
            CreateUndoPoint(editing_models ? "SingleModel" : "SingleObject", selectedBaseObjects.front()->name, "location");
        }
        else
        {
            CreateUndoPoint("All", "", "");
        }

        // this code speeds up the move the more times it is moved in the same direction in a short period of time
        static wxLongLong lastTime = 0;
        static float lastDelta = 0;
        if (key == WXK_UP || key == WXK_DOWN || key == WXK_LEFT || key == WXK_RIGHT) {
            static int repeats = 0;
            static int lastKey = 0;

            if (wxGetUTCTimeMillis() - lastTime > 500 || key != lastKey)
            {
                lastDelta = 1.0;
                repeats = 0;
            }
            else
            {
                repeats++;
                if (repeats > 5)
                {
                    lastDelta *= 2.0;
                    if (lastDelta > 30) lastDelta = 30;
                }
            }

            lastKey = key;
        }

        for (const auto& it : selectedBaseObjects)
        {
            float deltax = 0;
            float deltay = 0;

            if (key == WXK_UP) {
                deltay = 1.0;
            }
            else if (key == WXK_DOWN) {
                deltay = -1.0;
            }
            else if (key == WXK_LEFT) {
                deltax = -1.0;
            }
            else if (key == WXK_RIGHT) {
                deltax = 1.0;
            }

            if (deltax != 0 || deltay != 0)
            {
                deltax *= lastDelta;
                deltay *= lastDelta;

                if (is_3d)
                {
                    deltay *= -1;
                    int x = modelPreview->GetCameraRotationX();
                    int y = modelPreview->GetCameraRotationY();

                    // round to nearest 90 degrees
                    x += 45;
                    x /= 90;
                    x *= 90;
                    y += 45;
                    y /= 90;
                    y *= 90;

                    float xx = 0.0;
                    float yy = 0.0;
                    float zz = 0.0;

                    if (x == 0 || x == 360)
                    {
                        yy = -deltay;
                        if (y == 0 || y == 360)
                        {
                            xx = deltax;
                        }
                        else if (y == 90)
                        {
                            zz = deltax;
                        }
                        else if (y == 180)
                        {
                            xx = -deltax;
                        }
                        else if (y == 270)
                        {
                            zz = -deltax;
                        }
                    }
                    else if (x == 90)
                    {
                        if (y == 0 || y == 360)
                        {
                            xx = deltax;
                            zz = deltay;
                        }
                        else if (y == 90)
                        {
                            zz = deltax;
                            xx = -deltay;
                        }
                        else if (y == 180)
                        {
                            xx = -deltax;
                            zz = -deltay;
                        }
                        else if (y == 270)
                        {
                            zz = -deltax;
                            xx = deltay;
                        }
                    }
                    else if (x == 180)
                    {
                        yy = deltay;
                        if (y == 0 || y == 360)
                        {
                            xx = deltax;
                        }
                        else if (y == 90)
                        {
                            zz = deltax;
                        }
                        else if (y == 180)
                        {
                            xx = -deltax;
                        }
                        else if (y == 270)
                        {
                            zz = -deltax;
                        }
                    }
                    else if (x == 270)
                    {
                        if (y == 0 || y == 360)
                        {
                            xx = deltax;
                            zz = -deltay;
                        }
                        else if (y == 90)
                        {
                            zz = deltax;
                            xx = deltay;
                        }
                        else if (y == 180)
                        {
                            xx = -deltax;
                            zz = deltay;
                        }
                        else if (y == 270)
                        {
                            zz = -deltax;
                            xx = -deltay;
                        }
                    }

                    it->AddOffset(xx, yy, zz);
                }
                else
                {
                    it->AddOffset(deltax, deltay, 0.0);
                }
                it->UpdateXmlWithScale();
                //SetupPropGrid(it);
            }
        }

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::Nudge");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::Nudge");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::Nudge");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::Nudge");

        // set last time after everything is done
        lastTime = wxGetUTCTimeMillis();
    }
}

void LayoutPanel::OnChar(wxKeyEvent& event) {

    if (HandleLayoutKeyBinding(event)) return;

    wxChar uc = event.GetKeyCode();
    switch (uc) {
        case WXK_UP:
        case WXK_DOWN:
        case WXK_LEFT:
        case WXK_RIGHT:
            Nudge(uc);
            break;
        default:
            break;
    }
}
void LayoutPanel::OnCharHook(wxKeyEvent& event) {

    if (HandleLayoutKeyBinding(event)) return;

  wxChar uc = event.GetKeyCode();

    switch(uc) {
#ifndef __WXOSX__
        case 'z':
        case 'Z':
            if (event.ControlDown()) {
                wxCommandEvent evt(wxEVT_MENU, wxID_UNDO);
                DoUndo(evt);
            }
            break;
        case 'x':
        case 'X':
            if (event.ControlDown()) {
                wxCommandEvent evt(wxEVT_MENU, wxID_CUT);
                DoCut(evt);
            }
            break;
        case 'c':
        case 'C':
            if (event.ControlDown()) {
                wxCommandEvent evt(wxEVT_MENU, wxID_COPY);
                DoCopy(evt);
            }
            break;
        case 'v':
        case 'V':
            if (event.ControlDown()) {
                wxCommandEvent evt(wxEVT_MENU, wxID_PASTE);
                DoPaste(evt);
            }
            break;
    case WXK_INSERT:
    case WXK_NUMPAD_INSERT:
        if (event.ControlDown()) // Copy
        {
            wxCommandEvent evt(wxEVT_MENU, wxID_COPY);
            DoCopy(evt);
            event.StopPropagation();
        }
        else if (event.ShiftDown()) // Paste
        {
            wxCommandEvent evt(wxEVT_MENU, wxID_PASTE);
            DoPaste(evt);
            event.StopPropagation();
        }
        break;
#endif
        case WXK_DELETE:
        case WXK_BACK:
#ifndef __WXOSX__
            if (event.ShiftDown()) // Cut
            {
                wxCommandEvent evt(wxEVT_MENU, wxID_CUT);
                DoCut(evt);
                event.StopPropagation();
            }
            else
#endif
                if (editing_models)
                {
                    if (selectedTreeGroups.size() > 0) {
                        DeleteSelectedGroups();
                    }
                    else {
                        DeleteSelectedModels();
                    }
                }
                else
                {
                    DeleteSelectedObject();
                }
            event.StopPropagation();
            break;
        case WXK_UP:
        case WXK_DOWN:
        case WXK_LEFT:
        case WXK_RIGHT:
            Nudge(uc);
            break;

        case WXK_ESCAPE:
            if (m_polyline_active) {
                FinalizeModel();
            } else if (is_3d) {
                if (editing_models) {
                    UnSelectAllModelsInTree();
                } else {
                    UnSelectAllModels();
                }
            }
            break;

        case WXK_RETURN:
            if (m_polyline_active) {
                FinalizeModel();
            }
            event.Skip();
            break;
        default:
            event.Skip();
            break;
    }
}

void LayoutPanel::OnListCharHook(wxKeyEvent& event)
{
    wxChar uc = event.GetKeyCode();
    switch (uc) {
    case WXK_UP:
    case WXK_DOWN:
    case WXK_LEFT:
    case WXK_RIGHT:
        event.DoAllowNextEvent();
        break;
    default:
        OnCharHook(event);
        break;
    }
}

ModelGroup* LayoutPanel::GetSelectedModelGroup() const
{
    ModelGroup* res = nullptr;

    // This is here because I am seeing crashes which i believe originate here
    xlights->AddTraceMessage("LayoutPanel::GetSelectedModelGroup");
    //if (mSelectedGroup.IsOk()) {
    //    ModelTreeData* data = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(mSelectedGroup));
    //    Model* model = data != nullptr ? data->GetModel() : nullptr;
    //    if (model != nullptr) {
    //        if (model->GetDisplayAs() == "ModelGroup") {
    //            res = dynamic_cast<ModelGroup*>(model);
    //        }
    //    }
    //}
    //else {
        if (ModelGroupWindow->IsShown()) {
            res = dynamic_cast<ModelGroup*>(xlights->AllModels[model_grp_panel->GetGroupName()]);
        }
    //}
    xlights->AddTraceMessage("LayoutPanel::GetSelectedModelGroup done");
    return res;
}
void LayoutPanel::RemoveSelectedModelsFromGroup() {
    if (selectedBaseObject != nullptr && !selectedBaseObject->GetBaseObjectScreenLocation().IsLocked()) {
        xlights->AddTraceMessage("LayoutPanel::Remove Selected Models From Group");

        wxArrayString modelsToRemove;
        wxString modelsToConfirm = "";
        wxString parentGroup = "";
        for (const auto& item : selectedTreeModels) {
            if (item.IsOk()) {
                parentGroup = TreeListViewModels->GetItemText(TreeListViewModels->GetItemParent(item));
                wxString modelName = TreeListViewModels->GetItemText(item);
                modelsToRemove.Add(modelName);
                modelsToConfirm = modelsToConfirm + wxString::Format("%s- %s\n", "    ", modelName);
            }
        }
        for (const auto& item : selectedTreeSubModels) {
            if (item.IsOk()) {
                parentGroup = TreeListViewModels->GetItemText(TreeListViewModels->GetItemParent(item));
                wxString modelName = TreeListViewModels->GetItemText(item);
                modelsToRemove.Add(modelName);
                modelsToConfirm = modelsToConfirm + wxString::Format("%s- %s\n", "    ", modelName);
            }
        }
        ModelGroup *grp = dynamic_cast<ModelGroup*>(xlights->GetModel(parentGroup));
        if (grp && wxMessageBox("Are you sure you want to remove the folowing model(s)?:\n\n" + modelsToConfirm, "Confirm Delete?", wxICON_QUESTION | wxYES_NO) == wxYES) {

            // we suspend deferred work because if the delete model pops a dialog then the ASAP work gets done prematurely
            xlights->GetOutputModelManager()->SuspendDeferredWork(true);
            xlights->UnselectEffect(); // we do this just in case the effect is on the model we are deleting

            CreateUndoPoint("All", wxJoin(modelsToRemove, ','));

            for (const auto& it: modelsToRemove) {
                grp->ModelRemoved(it);
            }
            selectedBaseObject = nullptr;

            xlights->GetOutputModelManager()->SuspendDeferredWork(false);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::RemoveSelectedModelsFromGroup");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::RemoveSelectedModelsFromGroup");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::RemoveSelectedModelsFromGroup");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "LayoutPanel::RemoveSelectedModelsFromGroup");
        }
    }

}
void LayoutPanel::DeleteSelectedModels() {
    if (selectedBaseObject != nullptr && !selectedBaseObject->GetBaseObjectScreenLocation().IsLocked()) {
        xlights->AddTraceMessage("LayoutPanel::Delete Selected Model");

        wxArrayString modelsToDelete;
        wxString modelsToConfirm = "";
        for (const auto& item : selectedTreeModels) {
            if (item.IsOk()) {
                wxString modelName = TreeListViewModels->GetItemText(item);
                modelsToDelete.Add(modelName);
                modelsToConfirm = modelsToConfirm + wxString::Format("%s- %s\n", "    ", modelName);
            }
        }

        if (wxMessageBox("Are you sure you want to delete the folowing model(s)?:\n\n" + modelsToConfirm, "Confirm Delete?", wxICON_QUESTION | wxYES_NO) == wxYES) {

            // we suspend deferred work because if the delete model pops a dialog then the ASAP work gets done prematurely
            xlights->GetOutputModelManager()->SuspendDeferredWork(true);
            xlights->UnselectEffect(); // we do this just in case the effect is on the model we are deleting
            xlights->AbortRender(); // stop any rendering as deleting models from under the renderer will crash xlights

            CreateUndoPoint("All", wxJoin(modelsToDelete, ','));

            for (const auto& it: modelsToDelete) {
                xlights->GetDisplayElementsPanel()->RemoveModelFromLists(it);
                xlights->AllModels.Delete(it);
                xlights->AddTraceMessage(wxString::Format("LayoutPanel::Delete Selected Model : %s", it));
            }

            selectedBaseObject = nullptr;

            xlights->GetOutputModelManager()->SuspendDeferredWork(false);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::DeleteSelectedModels");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::DeleteSelectedModels");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::DeleteSelectedModels");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "LayoutPanel::DeleteSelectedModels");
        }
    }
}

void LayoutPanel::DeleteSelectedObject()
{
    objects_panel->DeleteSelectedObject();
}

void LayoutPanel::DeleteSelectedGroups()
{
	wxArrayString groupsToDelete;
	wxString groupsToConfirm = "";
	for (const auto& item : selectedTreeGroups) {
		if (item.IsOk()) {
			wxString groupName = TreeListViewModels->GetItemText(item);
			groupsToDelete.Add(groupName);
			groupsToConfirm = groupsToConfirm + wxString::Format("%s- %s\n", "    ", groupName);
		}
	}

	wxString const namesToDelete = wxJoin(groupsToDelete, ',');

	if (wxMessageBox("Are you sure you want to delete the following group(s)?:\n\n" + groupsToConfirm, "Confirm Remove?", wxICON_QUESTION | wxYES_NO) == wxYES) {

		CreateUndoPoint("All", wxJoin(groupsToDelete, ','));

		xlights->UnselectEffect(); // we do this just in case the effect is on the model we are deleting
        xlights->AbortRender(); // stop rendering as deleting groups while rendering is not good

		for (const auto& it : groupsToDelete) {
			xlights->AllModels.Delete(it.ToStdString());
		}
		UnSelectAllModels();

		xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::OnModelsPopup::ID_MNU_DELETE_MODEL_GROUP");
		xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnModelsPopup::ID_MNU_DELETE_MODEL_GROUP");
		xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnModelsPopup::ID_MNU_DELETE_MODEL_GROUP");
		xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_MNU_DELETE_MODEL_GROUP");
	}
}

void LayoutPanel::ReplaceModel()
{
    if (selectedBaseObject == nullptr) return;

    Model* modelToReplaceItWith = dynamic_cast<Model*>(selectedBaseObject);

    wxArrayString choices;

    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->GetName() != selectedBaseObject->GetName())
        {
            choices.Add(modelPreview->GetModels()[i]->GetName());
        }
    }

    wxSingleChoiceDialog dlg(this, "", "Select the model to replace with this model.", choices);
    dlg.SetSelection(0);
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK)
    {
        xlights->UnselectEffect(); // we do this just in case the effect is on the model we are deleting
        xlights->AbortRender(); // we dont want to be rendering when we do this

        Model* replaceModel = nullptr;
        for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
        {
            if (modelPreview->GetModels()[i]->GetName() == dlg.GetStringSelection())
            {
                replaceModel = modelPreview->GetModels()[i];
            }
        }

        if (replaceModel == nullptr) return;

        // Prompt user to copy the target models start channel ...but only if
        // they are not already the same and the new model uses a chaining start
        // channel ... the theory being if you took time to set the start channel
        // you probably want to keep it and so a prompt will just be annoying
        if ((replaceModel->ModelStartChannel !=
            modelToReplaceItWith->ModelStartChannel &&
            wxString(modelToReplaceItWith->ModelStartChannel).StartsWith(">")) ||
            (replaceModel->GetControllerName() != modelToReplaceItWith->GetControllerName() && (modelToReplaceItWith->GetControllerName() == "" || modelToReplaceItWith->GetControllerName() == NO_CONTROLLER)))
        {
            auto msg = wxString::Format("Should I copy the replaced models start channel '%s' to the replacement model whose start channel is currently '%s'?", replaceModel->ModelStartChannel, modelToReplaceItWith->ModelStartChannel);
            if (wxMessageBox(msg, "Update Start Channel", wxYES_NO) == wxYES)
            {
                modelToReplaceItWith->SetStartChannel(replaceModel->ModelStartChannel);

                modelToReplaceItWith->SetControllerProtocol(replaceModel->GetControllerProtocol());
                modelToReplaceItWith->SetControllerPort(replaceModel->GetControllerPort());
                modelToReplaceItWith->SetControllerName(replaceModel->GetControllerName());
                modelToReplaceItWith->SetSmartRemote(replaceModel->GetSmartRemote());
                modelToReplaceItWith->SetSmartRemoteType(replaceModel->GetSmartRemoteType());
                modelToReplaceItWith->SetSRMaxCascade(replaceModel->GetSRMaxCascade());
                modelToReplaceItWith->SetSRCascadeOnPort(replaceModel->GetSRCascadeOnPort());
            }
        }

        if (replaceModel->GetNumSubModels() > 0 ) {
            if (wxMessageBox("Merge The Submodels", "Merge The Submodels", wxYES_NO) == wxYES) {
                for (int i = 0; i < replaceModel->GetNumSubModels(); ++i) {
                    auto name{ replaceModel->GetSubModel(i)->Name() };
                    if (modelToReplaceItWith->GetSubModel(name) != nullptr) {
                        continue;
                    }
                    modelToReplaceItWith->AddSubmodel(replaceModel->GetSubModel(i)->GetModelXml());
                }
            }
        }

        auto rmn = replaceModel->GetName();
        auto riw = modelToReplaceItWith->GetName();
        xlights->AllModels.RenameInListOnly(dlg.GetStringSelection().ToStdString(), "Iamgoingtodeletethismodel");
        replaceModel->Rename("Iamgoingtodeletethismodel");
        xlights->AllModels.RenameInListOnly(modelToReplaceItWith->GetName(), dlg.GetStringSelection().ToStdString());
        modelToReplaceItWith->Rename(dlg.GetStringSelection().ToStdString());
        xlights->AllModels.Delete("Iamgoingtodeletethismodel");
        xlights->ReplaceModelWithModelFixGroups(rmn, riw);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "ReplaceModel", nullptr, nullptr, dlg.GetStringSelection().ToStdString());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ReplaceModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ReplaceModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ReplaceModel");
    }
}

void LayoutPanel::LockSelectedModels(bool lock)
{
    std::vector<Model*> modelsToLock = GetSelectedModelsForEdit();

    for (const auto& model : modelsToLock) {
        model->Lock(lock);
    }

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::LockSelectedModels");
}

void LayoutPanel::DoCopy(wxCommandEvent& event) {
    if (!modelPreview->HasFocus() && !TreeListViewModels->HasFocus() && !TreeListViewModels->GetView()->HasFocus() && !objects_panel->ObjectListHasFocus()) {
        event.Skip();
    } else if (selectedBaseObject != nullptr) {
        CopyPasteBaseObject copyData;

        copyData.SetBaseObject(selectedBaseObject);

        if (copyData.IsOk() && wxTheClipboard->Open()) {
            if (!wxTheClipboard->SetData(new wxTextDataObject(copyData.Serialise()))) {
                DisplayError("Unable to copy data to clipboard.", this);
            }
            wxTheClipboard->Close();
        }
    }
}

void LayoutPanel::DoCut(wxCommandEvent& event) {
    if (!modelPreview->HasFocus() && !TreeListViewModels->HasFocus() && !TreeListViewModels->GetView()->HasFocus() && !objects_panel->ObjectListHasFocus()) {
        event.Skip();
    } else if (selectedBaseObject != nullptr) {
        DoCopy(event);
		if (editing_models)
			DeleteSelectedModels();
		else
			DeleteSelectedObject();
    }
}

void LayoutPanel::DoPaste(wxCommandEvent& event) {
    if (!modelPreview->HasFocus() && !TreeListViewModels->HasFocus() && !TreeListViewModels->GetView()->HasFocus() && !objects_panel->ObjectListHasFocus()) {
        event.Skip();
    } else {
        if (wxTheClipboard->Open()) {
            CreateUndoPoint("All", selectedBaseObject == nullptr ? "" : selectedBaseObject->name);

            wxTextDataObject data;
            wxTheClipboard->GetData(data);

            CopyPasteBaseObject copyData(data.GetText().ToStdString());

            wxTheClipboard->Close();

            if (copyData.IsOk())
            {
                wxXmlNode* nd = copyData.GetBaseObjectXml();

                if (nd != nullptr)
                {
                    auto nda = nd->GetAttribute("DisplayAs");
                    auto nx = (int)wxAtof(nd->GetAttribute("WorldPosX"));
                    auto ny = (int)wxAtof(nd->GetAttribute("WorldPosY"));
                    auto nz = (int)wxAtof(nd->GetAttribute("WorldPosZ"));

                    bool moved = true;
                    while (moved)
                    {
                        moved = false;
                        // is there a model in the same location of the same type ... if so offset the pasting of the model
                        for (const auto& it : xlights->AllModels)
                        {
                            if (nda == it.second->GetModelXml()->GetAttribute("DisplayAs"))
                            {
                                auto x = (int)wxAtof(it.second->GetModelXml()->GetAttribute("WorldPosX"));
                                auto y = (int)wxAtof(it.second->GetModelXml()->GetAttribute("WorldPosY"));
                                auto z = (int)wxAtof(it.second->GetModelXml()->GetAttribute("WorldPosZ"));
                                if (nx == x &&
                                    ny == y &&
                                    nz == z)
                                {
                                    nx += 40;
                                    ny -= 40;
                                    nd->DeleteAttribute("WorldPosX");
                                    nd->DeleteAttribute("WorldPosY");
                                    nd->AddAttribute("WorldPosX", wxString::Format("%6.4f", (float)nx));
                                    nd->AddAttribute("WorldPosY", wxString::Format("%6.4f", (float)ny));
                                    moved = true;
                                    break;
                                }
                            }
                        }
                    }

                    std::string name = "";

                    if (!copyData.IsViewObject())
					{
						if (!editing_models)//dont paste model in View Object mode
							return;

                        // Remove any existing controller port config
                        nd->DeleteAttribute("ModelChain");
                        nd->DeleteAttribute("Controller");
                        for (auto n = nd->GetChildren(); n != nullptr; n = n->GetNext()) {
                            if (n->GetName() == "ControllerConnection") {
                                nd->RemoveChild(n);
                                delete n;
                                break;
                            }
                        }

						nd->DeleteAttribute("StartChannel");
						nd->AddAttribute("StartChannel", "1");

						Model *newModel = xlights->AllModels.CreateModel(nd);
						name = xlights->AllModels.GenerateModelName(newModel->name);
                        newModel->SetControllerName(NO_CONTROLLER);
						newModel->name = name;
						newModel->GetModelXml()->DeleteAttribute("name");
						newModel->Lock(false);
						newModel->GetModelXml()->AddAttribute("name", name);
						newModel->AddOffset(0.02, 0.02, 0.0);
						newModel->UpdateXmlWithScale();
						xlights->AllModels.AddModel(newModel);
						lastModelName = name;
					}
					else
					{
						if (editing_models)//dont paste view objects in model editing mode
							return;

						ViewObject *newViewObject = xlights->AllObjects.CreateObject(nd);
						name = xlights->AllObjects.GenerateObjectName(newViewObject->name);
						newViewObject->name = name;
						newViewObject->GetModelXml()->DeleteAttribute("name");
						newViewObject->Lock(false);
						newViewObject->GetModelXml()->AddAttribute("name", name);
						newViewObject->AddOffset(50.0, 0.0, 0.0);
						newViewObject->UpdateXmlWithScale();
						xlights->AllObjects.AddViewObject(newViewObject);
						lastModelName = name;
					}

                    //SelectBaseObject(name);
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::DoPaste");
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::DoPaste");
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::DoPaste", nullptr, nullptr, name);
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "LayoutPanel::DoPaste", nullptr, nullptr, name);
                    modelPreview->SetCursor(wxCURSOR_DEFAULT);
                }
            }
            else
            {
                static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
                logger_base.warn("LayoutPanel: Error trying to parse XML for paste. Paste request ignored. %s.", (const char *)data.GetText().c_str());
            }
        }
    }
}

void LayoutPanel::DoUndo(wxCommandEvent& event) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("LayoutPanel::DoUndo");
    int sz = undoBuffer.size() - 1;
    if (sz >= 0) {
        UnSelectAllModels();
        xlights->AbortRender();


        if (undoBuffer[sz].type == "Background") {
            logger_base.debug("LayoutPanel::DoUndo Background");
            wxPropertyGridEvent pgEvent;
            pgEvent.SetPropertyGrid(propertyEditor);
            wxStringProperty wsp("Background", undoBuffer[sz].key, undoBuffer[sz].data);
            pgEvent.SetProperty(&wsp);
            wxVariant value(undoBuffer[sz].data);
            pgEvent.SetPropertyValue(value);
            OnPropertyGridChange(pgEvent);
            UnSelectAllModels();
        } else if (undoBuffer[sz].type == "ModelProperty") {
            logger_base.debug("LayoutPanel::DoUndo ModelProperty");
            SelectModel(undoBuffer[sz].model);
            wxPropertyGridEvent event2;
            event2.SetPropertyGrid(propertyEditor);
            wxStringProperty wsp("Model", undoBuffer[sz].key, undoBuffer[sz].data);
            event2.SetProperty(&wsp);
            wxVariant value(undoBuffer[sz].data);
            event2.SetPropertyValue(value);
            OnPropertyGridChange(event2);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::DoUndo");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::DoUndo");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::DoUndo");
        } else if (undoBuffer[sz].type == "ObjectProperty") {
            logger_base.debug("LayoutPanel::DoUndo ObjectProperty");
            ViewObject* vobj = xlights->AllObjects[undoBuffer[sz].model];
            SelectViewObject(vobj);
            wxPropertyGridEvent event2;
            event2.SetPropertyGrid(propertyEditor);
            wxStringProperty wsp("Object", undoBuffer[sz].key, undoBuffer[sz].data);
            event2.SetProperty(&wsp);
            wxVariant value(undoBuffer[sz].data);
            event2.SetPropertyValue(value);
            OnPropertyGridChange(event2);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::DoUndo");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::DoUndo");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::DoUndo");
        } else if (undoBuffer[sz].type == "SingleModel") {
            logger_base.debug("LayoutPanel::DoUndo SingleModel");
            Model *m = xlights->AllModels[undoBuffer[sz].model];
            if (m != nullptr) {
                wxStringInputStream min(undoBuffer[sz].data);
                wxXmlDocument mdoc(min);

                wxXmlNode *parent = m->GetModelXml()->GetParent();
                wxXmlNode *next = m->GetModelXml()->GetNext();
                parent->RemoveChild(m->GetModelXml());

                delete m->GetModelXml();
                m->SetFromXml(mdoc.GetRoot());
                mdoc.DetachRoot();
                parent->InsertChild(m->GetModelXml(), next);
                SelectModel(undoBuffer[sz].model);
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::DoUndo");
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::DoUndo");
            }
        } else if (undoBuffer[sz].type == "SingleObject") {
            logger_base.debug("LayoutPanel::DoUndo SingleObject");
            ViewObject *m = xlights->AllObjects[undoBuffer[sz].model];
            if (m != nullptr) {
                wxStringInputStream min(undoBuffer[sz].data);
                wxXmlDocument mdoc(min);

                wxXmlNode *parent = m->GetModelXml()->GetParent();
                wxXmlNode *next = m->GetModelXml()->GetNext();
                parent->RemoveChild(m->GetModelXml());

                delete m->GetModelXml();
                m->SetFromXml(mdoc.GetRoot());
                mdoc.DetachRoot();
                parent->InsertChild(m->GetModelXml(), next);
                SelectModel(undoBuffer[sz].model);
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::DoUndo");
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::DoUndo");
            }
        } else if (undoBuffer[sz].type == "All") {
            logger_base.debug("LayoutPanel::DoUndo All");
            UnSelectAllModels();

            wxStringInputStream gin(undoBuffer[sz].groups);
            wxXmlDocument gdoc;
            gdoc.Load(gin);
            wxStringInputStream min(undoBuffer[sz].models);
            wxXmlDocument mdoc(min);
            wxStringInputStream oin(undoBuffer[sz].objects);
            wxXmlDocument odoc(oin);

            wxXmlNode *m = xlights->ModelsNode->GetChildren();
            while (m != nullptr) {
                xlights->ModelsNode->RemoveChild(m);
                delete m;
                m = xlights->ModelsNode->GetChildren();
            }
            m = mdoc.GetRoot()->GetChildren();
            while (m != nullptr) {
                mdoc.GetRoot()->RemoveChild(m);
                xlights->ModelsNode->AddChild(m);
                m = mdoc.GetRoot()->GetChildren();
            }

            wxXmlNode *o = xlights->ViewObjectsNode->GetChildren();
            while (o != nullptr) {
                xlights->ViewObjectsNode->RemoveChild(o);
                delete o;
                o = xlights->ViewObjectsNode->GetChildren();
            }
            o = odoc.GetRoot()->GetChildren();
            while (o != nullptr) {
                odoc.GetRoot()->RemoveChild(o);
                xlights->ViewObjectsNode->AddChild(o);
                o = odoc.GetRoot()->GetChildren();
            }

            m = xlights->ModelGroupsNode->GetChildren();
            while (m != nullptr) {
                xlights->ModelGroupsNode->RemoveChild(m);
                delete m;
                m = xlights->ModelGroupsNode->GetChildren();
            }
            m = gdoc.GetRoot()->GetChildren();
            while (m != nullptr) {
                gdoc.GetRoot()->RemoveChild(m);
                xlights->ModelGroupsNode->AddChild(m);
                m = gdoc.GetRoot()->GetChildren();
            }

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::DoUndo");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::DoUndo");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::DoUndo", nullptr, nullptr, undoBuffer[sz].model);
        } else if (undoBuffer[sz].type == "ModelName") {
            logger_base.debug("LayoutPanel::DoUndo ModelName");
            std::string origName = undoBuffer[sz].model;
            std::string newName = undoBuffer[sz].key;
            if (lastModelName == newName) {
                lastModelName = origName;
            }
            xlights->RenameModel(newName, origName);

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::DoUndo", nullptr, nullptr, origName);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::DoUndo");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::DoUndo");
        } else if (undoBuffer[sz].type == "ObjectName") {
            logger_base.debug("LayoutPanel::DoUndo ObjectName");
            std::string origName = undoBuffer[sz].model;
            std::string newName = undoBuffer[sz].key;
            xlights->RenameObject(newName, origName);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::DoUndo");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::DoUndo");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::DoUndo", nullptr, nullptr, origName);
        }
        else
        {
            wxASSERT(false);
        }
        modelPreview->SetFocus();

        undoBuffer.resize(sz);
    }
}

void LayoutPanel::CreateUndoPoint(const std::string &type, const std::string &model, const std::string &key, const std::string &data) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::CreateUndoPoint");
    size_t idx = undoBuffer.size();

    //printf("%s   %s   %s  %s\n", type.c_str(), model.c_str(), key.c_str(), data.c_str());
    if (idx > 0 ) {
        if ((type == "SingleModel" || type == "ModelProperty" || type == "Background")
            && undoBuffer[idx - 1].model == model && undoBuffer[idx - 1].key == key) {
            //SingleModel - multi mouse movement, just record the original
            //Background/ModelProperty - multiple changes of the same property (like spinning spin button)
            return;
        }
        else if ((type == "SingleObject" || type == "ObjectProperty")
            && undoBuffer[idx - 1].model == model && undoBuffer[idx - 1].key == key) {
            return;
        }
    }
    if (idx >= 100) {  //100 steps is more than enough IMO
        for (size_t x = 1; x < idx; x++) {
            undoBuffer[x-1] = undoBuffer[x];
        }
        idx--;
    }
    undoBuffer.resize(idx + 1);

    undoBuffer[idx].type = type;
    undoBuffer[idx].model = model;
    undoBuffer[idx].key = key;
    undoBuffer[idx].data = data;

    if (type == "SingleModel") {
        Model *m = _newModel;
        if (m == nullptr) {
            if( selectedBaseObject == nullptr ) {
                undoBuffer.resize(idx);
                return;
            }
            m=dynamic_cast<Model*>(selectedBaseObject);
            wxASSERT(m != nullptr);
        }
        wxXmlDocument doc;
        wxXmlNode *parent = m->GetModelXml()->GetParent();
        if (parent == nullptr) {
            undoBuffer.resize(idx); // restore undo buffer back to its old size
            return; // this shouldnt happend but it does and when it does it crashes.
        }
        wxXmlNode *next = m->GetModelXml()->GetNext();
        parent->RemoveChild(m->GetModelXml());
        doc.SetRoot(m->GetModelXml());
        wxStringOutputStream stream;
        doc.Save(stream);
        undoBuffer[idx].data = stream.GetString();
        doc.DetachRoot();
        parent->InsertChild(m->GetModelXml(), next);
    } else if (type == "SingleObject") {
        ViewObject *obj = nullptr;
        if( selectedBaseObject == nullptr ) {
            undoBuffer.resize(idx);
            return;
        }
        obj=dynamic_cast<ViewObject*>(selectedBaseObject);
        wxASSERT(obj != nullptr);
        wxXmlDocument doc;
        wxXmlNode *parent = obj->GetModelXml()->GetParent();
        if (parent == nullptr) {
            undoBuffer.resize(idx); // restore undo buffer back to its old size
            return; // this shouldnt happend but it does and when it does it crashes.
        }
        wxXmlNode *next = obj->GetModelXml()->GetNext();
        parent->RemoveChild(obj->GetModelXml());
        doc.SetRoot(obj->GetModelXml());
        wxStringOutputStream stream;
        doc.Save(stream);
        undoBuffer[idx].data = stream.GetString();
        doc.DetachRoot();
        parent->InsertChild(obj->GetModelXml(), next);
    } else if (type == "All") {
        wxXmlDocument doc;
        wxXmlNode *parent = xlights->ModelsNode->GetParent();
        if (parent == nullptr) {
            undoBuffer.resize(idx); // restore undo buffer back to its old size
            return; // this shouldnt happend but it does and when it does it crashes.
        }
        wxXmlNode *next = xlights->ModelsNode->GetNext();
        parent->RemoveChild(xlights->ModelsNode);
        doc.SetRoot(xlights->ModelsNode);
        wxStringOutputStream stream;
        doc.Save(stream);
        undoBuffer[idx].models = stream.GetString();
        doc.DetachRoot();
        parent->InsertChild(xlights->ModelsNode, next);

        parent = xlights->ViewObjectsNode->GetParent();
        if (parent == nullptr) logger_base.crit("LayoutPanel::CreateUndoPoint ViewObjectsNode Parent was NULL ... this is going to get ugly.");
        next = xlights->ViewObjectsNode->GetNext();
        parent->RemoveChild(xlights->ViewObjectsNode);
        doc.SetRoot(xlights->ViewObjectsNode);
        wxStringOutputStream stream3;
        doc.Save(stream3);
        undoBuffer[idx].objects = stream3.GetString();
        doc.DetachRoot();
        parent->InsertChild(xlights->ViewObjectsNode, next);

        wxStringOutputStream stream2;
        parent = xlights->ModelGroupsNode->GetParent();
        if (parent == nullptr) {
            undoBuffer.resize(idx); // restore undo buffer back to its old size
            return; // this shouldnt happend but it does and when it does it crashes.
        }
        next = xlights->ModelGroupsNode->GetNext();
        parent->RemoveChild(xlights->ModelGroupsNode);
        doc.SetRoot(xlights->ModelGroupsNode);
        doc.Save(stream2);
        undoBuffer[idx].groups = stream2.GetString();
        stream.Reset();
        doc.DetachRoot();
        parent->InsertChild(xlights->ModelGroupsNode, next);
    }
}

void LayoutPanel::OnModelsPopup(wxCommandEvent& event) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int id = event.GetId();
    if (id == ID_MNU_DELETE_MODEL) {
        logger_base.debug("LayoutPanel::OnModelsPopup DELETE_MODEL");
        DeleteSelectedModels();
    } else if (id == ID_MNU_REMOVE_MODEL_FROM_GROUP) {
        logger_base.debug("LayoutPanel::OnModelsPopup REMOVE_MODEL_FROM_GROUP");
        RemoveSelectedModelsFromGroup();
    } else if (event.GetId() == ID_PREVIEW_REPLACEMODEL) {
        ReplaceModel();
    } else if (event.GetId() == ID_PREVIEW_MODEL_NODELAYOUT) {
        ShowNodeLayout();
    } else if (event.GetId() == ID_PREVIEW_MODEL_LOCK) {
        LockSelectedModels(true);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_LOCK");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_LOCK");
    } else if (event.GetId() == ID_PREVIEW_MODEL_UNLOCK) {
        LockSelectedModels(false);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_UNLOCK");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_UNLOCK");
    } else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTASCUSTOM) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        md->ExportAsCustomXModel();
    } else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTASCUSTOM3D) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        md->ExportAsCustomXModel3D();
    } else if (event.GetId() == ID_PREVIEW_MODEL_CREATEGROUP) {
        CreateModelGroupFromSelected();
    } else if (event.GetId() == ID_MNU_ADD_TO_EXISTING_GROUPS) {
        AddSelectedToExistingGroups();
    } else if (event.GetId() == ID_PREVIEW_MODEL_WIRINGVIEW) {
        ShowWiring();
    } else if (event.GetId() == ID_PREVIEW_MODEL_CAD_EXPORT) {
        ExportModelAsCAD();
    } else if (event.GetId() == ID_PREVIEW_MODEL_ASPECTRATIO) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int screen_wi = md->GetBaseObjectScreenLocation().GetMWidth();
        int screen_ht = md->GetBaseObjectScreenLocation().GetMHeight();
        float render_ht = md->GetBaseObjectScreenLocation().GetRenderHt();
        float render_wi = md->GetBaseObjectScreenLocation().GetRenderWi();
        float ht_ratio = render_ht / (float)screen_ht;
        float wi_ratio = render_wi / (float)screen_wi;
        if (ht_ratio > wi_ratio) {
            render_wi = render_wi / ht_ratio;
            md->GetBaseObjectScreenLocation().SetMWidth((int)render_wi);
        } else {
            render_ht = render_ht / wi_ratio;
            md->GetBaseObjectScreenLocation().SetMHeight((int)render_ht);
        }
        md->GetBaseObjectScreenLocation().Write(md->ModelXml);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ASPECTRATIO");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ASPECTRATIO");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ASPECTRATIO", nullptr, nullptr, GetSelectedModelName());
    } else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        md->ExportXlightsModel();
    } else if (event.GetId() == ID_PREVIEW_DELETE_ACTIVE) {
        DeleteCurrentPreview();
    } else if (event.GetId() == ID_PREVIEW_RENAME_ACTIVE) {
        RenameCurrentPreview();
    } else if (event.GetId() == ID_PREVIEW_MODEL_ADDPOINT) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int handle = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(handle + 0x8000));
        md->InsertHandle(handle, modelPreview->GetCameraZoomForHandles(), modelPreview->GetHandleScale());
        md->UpdateXmlWithScale();
        md->InitModel();
        //SetupPropGrid(md);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ADDPOINT");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ADDPOINT");
    } else if (event.GetId() == ID_PREVIEW_MODEL_DELETEPOINT) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int selected_handle = md->GetSelectedHandle();
        if ((selected_handle != -1) && (md->GetNumHandles() > 2)) {
            CreateUndoPoint("SingleModel", md->name, std::to_string(selected_handle + 0x4000));
            md->DeleteHandle(selected_handle);
            md->SelectHandle(-1);
            md->GetModelScreenLocation().SelectSegment(-1);
            md->UpdateXmlWithScale();
            md->InitModel();
            //SetupPropGrid(md);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_DELETEPOINT");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_DELETEPOINT");
        }
    } else if (event.GetId() == ID_PREVIEW_MODEL_ADDCURVE) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int seg = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(seg + 0x2000));
        md->SetCurve(seg, true);
        md->UpdateXmlWithScale();
        md->InitModel();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_ADDCURVE");
    } else if (event.GetId() == ID_PREVIEW_MODEL_DELCURVE) {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr)
            return;
        int seg = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(seg + 0x1000));
        md->SetCurve(seg, false);
        md->UpdateXmlWithScale();
        md->InitModel();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnPreviewModelPopup::ID_PREVIEW_MODEL_DELCURVE");
    } else if (event.GetId() == ID_PREVIEW_ALIGN_BOTTOM) {
        if (editing_models) {
            PreviewModelAlignBottoms();
        } else {
            objects_panel->PreviewObjectAlignBottoms();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_GROUND) {
        if (editing_models) {
            PreviewModelAlignWithGround();
        } else {
            objects_panel->PreviewObjectAlignWithGround();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_TOP) {
        if (editing_models) {
            PreviewModelAlignTops();
        } else {
            objects_panel->PreviewObjectAlignTops();
        }
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTION ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERSTARTNULLNODES ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERENDNULLNODES ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERBRIGHTNESS ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERCOLOURORDER ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERGAMMA ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERGROUPCOUNT ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERDIRECTION ||
               event.GetId() == ID_PREVIEW_BULKEDIT_SMARTREMOTE ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERPROTOCOL ||
               event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTIONINCREMENT ||
               event.GetId() == ID_PREVIEW_BULKEDIT_SMARTREMOTETYPE) {
        BulkEditControllerConnection(event.GetId());
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERNAME) {
        BulkEditControllerName();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_SETACTIVE) {
        BulkEditActive(true);
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_SETINACTIVE) {
        BulkEditActive(false);
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_TAGCOLOUR) {
        BulkEditTagColour();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_PIXELSIZE) {
        BulkEditPixelSize();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_PIXELSTYLE) {
        BulkEditPixelStyle();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_TRANSPARENCY) {
        BulkEditTransparency();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_BLACKTRANSPARENCY) {
        BulkEditBlackTranparency();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_PREVIEW) {
        BulkEditControllerPreview();
    } else if (event.GetId() == ID_PREVIEW_BULKEDIT_DIMMINGCURVES) {
        BulkEditDimmingCurves();
    } else if (event.GetId() == ID_PREVIEW_ALIGN_LEFT) {
        if (editing_models) {
            PreviewModelAlignLeft();
        } else {
            objects_panel->PreviewObjectAlignLeft();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_RIGHT) {
        if (editing_models) {
            PreviewModelAlignRight();
        } else {
            objects_panel->PreviewObjectAlignRight();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_FRONT) {
        if (editing_models) {
            PreviewModelAlignFronts();
        } else {
            objects_panel->PreviewObjectAlignFronts();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_BACK) {
        if (editing_models) {
            PreviewModelAlignBacks();
        } else {
            objects_panel->PreviewObjectAlignBacks();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_H_CENTER) {
        if (editing_models) {
            PreviewModelAlignHCenter();
        } else {
            objects_panel->PreviewObjectAlignHCenter();
        }
    } else if (event.GetId() == ID_PREVIEW_ALIGN_V_CENTER) {
        if (editing_models) {
            PreviewModelAlignVCenter();
        } else {
            objects_panel->PreviewObjectAlignVCenter();
        }
    } else if (event.GetId() == ID_PREVIEW_H_DISTRIBUTE) {
        if (editing_models) {
            PreviewModelHDistribute();
        } else {
            objects_panel->PreviewObjectHDistribute();
        }
    } else if (event.GetId() == ID_PREVIEW_V_DISTRIBUTE) {
        if (editing_models) {
            PreviewModelVDistribute();
        } else {
            objects_panel->PreviewObjectVDistribute();
        }
    } else if (event.GetId() == ID_PREVIEW_RESIZE_SAMEWIDTH) {
        if (editing_models) {
            PreviewModelResize(true, false);
        } else {
            objects_panel->PreviewObjectResize(true, false);
        }
    } else if (event.GetId() == ID_PREVIEW_RESIZE_SAMEHEIGHT) {
        if (editing_models) {
            PreviewModelResize(false, true);
        } else {
            objects_panel->PreviewObjectResize(false, true);
        }
    } else if (event.GetId() == ID_PREVIEW_RESIZE_SAMESIZE) {
        if (editing_models) {
            PreviewModelResize(true, true);
        } else {
            objects_panel->PreviewObjectResize(true, true);
        }
    } else if (id == ID_MNU_DELETE_MODEL_GROUP) {
        logger_base.debug("LayoutPanel::OnModelsPopup DELETE_MODEL_GROUP");
        DeleteSelectedGroups();
    } else if (id == ID_MNU_DELETE_EMPTY_MODEL_GROUPS) {
        logger_base.debug("LayoutPanel::OnModelsPopup DELETE_EMPTY_MODEL_GROUPS");

        bool deleted = true;

        while (deleted) {
            deleted = false;
            auto it = xlights->AllModels.begin();
            while (it != xlights->AllModels.end()) {
                if (it->second->GetDisplayAs() == "ModelGroup") {
                    ModelGroup* mg = dynamic_cast<ModelGroup*>(it->second);
                    ++it;
                    if (mg->GetModelCount() == 0) {
                        xlights->UnselectEffect(); // we do this just in case the effect is on the model we are deleting
                        xlights->AbortRender();

                        xlights->AllModels.Delete(mg->GetName());
                        deleted = true;
                    }
                } else {
                    ++it;
                }
            }
        }

        UnSelectAllModels();

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::OnModelsPopup::ID_MNU_DELETE_EMPTY_MODEL_GROUPS");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnModelsPopup::ID_MNU_DELETE_EMPTY_MODEL_GROUPS");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnModelsPopup::ID_MNU_DELETE_EMPTY_MODEL_GROUPS");
    } else if (id == ID_MNU_MAKEALLSCVALID) {
        if (wxMessageBox("While this will make all your start channels valid it will not magically make your start channel right for your show. It will however solve strange nodes lighting up in the sequencer.\nAre you ok with this?", "WARNING", wxYES_NO | wxCENTRE) == wxYES) {
            for (const auto& it : xlights->AllModels) {
                if (it.second->GetDisplayAs() != "ModelGroup" && (!it.second->CouldComputeStartChannel || !it.second->IsValidStartChannelString())) {
                    it.second->SetControllerName(NO_CONTROLLER);
                }
            }

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::OnModelsPopup::ID_MNU_MAKEALLSCVALID");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnModelsPopup::ID_MNU_MAKEALLSCVALID");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnModelsPopup::ID_MNU_MAKEALLSCVALID");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "LayoutPanel::OnModelsPopup::ID_MNU_MAKEALLSCVALID");
        }
    } else if (id == ID_MNU_MAKEALLSCNOTOVERLAPPING) {
        if (wxMessageBox("While this will make all your start channels not overlapping it will not magically make your start channel right for your show. It will however solve strange nodes lighting up in the sequencer.\nAre you ok with this?", "WARNING", wxYES_NO | wxCENTRE) == wxYES) {
            for (const auto& it : xlights->AllModels) {
                if (it.second->GetDisplayAs() != "ModelGroup" && xlights->AllModels.IsModelOverlapping(it.second)) {
                    it.second->SetControllerName(NO_CONTROLLER);
                }
            }

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::OnModelsPopup::ID_MNU_MAKEALLSCNOTOVERLAPPING");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnModelsPopup::ID_MNU_MAKEALLSCNOTOVERLAPPING");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnModelsPopup::ID_MNU_MAKEALLSCNOTOVERLAPPING");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "LayoutPanel::OnModelsPopup::ID_MNU_MAKEALLSCNOTOVERLAPPING");
        }
    } else if (id == ID_MNU_MAKESCVALID) {
        if (wxMessageBox("While this will make your start channel valid and not overlapping it will not magically make your start channel right for your show. It will however solve strange nodes lighting up in the sequencer.\nAre you ok with this?", "WARNING", wxYES_NO | wxCENTRE) == wxYES) {
            if (selectedBaseObject != nullptr) {
                Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
                selectedModel->SetControllerName(NO_CONTROLLER);

                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::OnModelsPopup::ID_MNU_MAKESCVALID");
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnModelsPopup::ID_MNU_MAKESCVALID");
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnModelsPopup::ID_MNU_MAKESCVALID");
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "LayoutPanel::OnModelsPopup::ID_MNU_MAKESCVALID");
            }
        }
    } else if (id == ID_MNU_RENAME_MODEL_GROUP) {
        logger_base.debug("LayoutPanel::OnModelsPopup RENAME_MODEL_GROUP");
        if (selectedTreeGroups[0].IsOk()) {
            wxString sel = TreeListViewModels->GetItemText(selectedTreeGroups[0]);
            wxTextEntryDialog dlg(this, "Enter new name for group " + sel, "Rename " + sel, sel);
            OptimiseDialogPosition(&dlg);
            if (dlg.ShowModal() == wxID_OK) {
                wxString name = wxString(Model::SafeModelName(dlg.GetValue().ToStdString()));

                while (xlights->AllModels.GetModel(name.ToStdString()) != nullptr || name.IsEmpty()) {
                    wxTextEntryDialog dlg2(this, "Model or Group of name '" + name + "' already exists. Enter new name for group", "Enter new name for group");
                    OptimiseDialogPosition(&dlg2);
                    if (dlg2.ShowModal() == wxID_OK) {
                        name = wxString(Model::SafeModelName(dlg2.GetValue().ToStdString()));
                    } else {
                        return;
                    }
                }

                xlights->RenameModel(sel.ToStdString(), name.ToStdString());
                model_grp_panel->UpdatePanel(name.ToStdString());

                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::OnModelsPopup::ID_MNU_RENAME_MODEL_GROUP", nullptr, nullptr, name.ToStdString());
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnModelsPopup::ID_MNU_RENAME_MODEL_GROUP");
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnModelsPopup::ID_MNU_RENAME_MODEL_GROUP");
            }
        }
    } else if (event.GetId() == ID_PREVIEW_MODEL_CREATEGROUP) {
        CreateModelGroupFromSelected();
    } else if (id == ID_MNU_ADD_MODEL_GROUP) {
        logger_base.debug("LayoutPanel::OnModelsPopup ADD_MODEL_GROUP");
        wxTextEntryDialog dlg(this, "Enter name for new group", "Enter name for new group");
        OptimiseDialogPosition(&dlg);
        if (dlg.ShowModal() == wxID_OK) {
            wxString name = wxString(Model::SafeModelName(dlg.GetValue().ToStdString()));
            while (xlights->AllModels.GetModel(name.ToStdString()) != nullptr || name.IsEmpty()) {
                wxTextEntryDialog dlg2(this, "Model of name '" + name + "' already exists. Enter name for new group", "Enter name for new group");
                OptimiseDialogPosition(&dlg2);
                if (dlg2.ShowModal() == wxID_OK) {
                    name = wxString(Model::SafeModelName(dlg2.GetValue().ToStdString()));
                } else {
                    return;
                }
            }
            wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "modelGroup");
            xlights->ModelGroupsNode->AddChild(node);
            node->AddAttribute("selected", "0");
            node->AddAttribute("name", name);
            node->AddAttribute("models", "");
            node->AddAttribute("layout", "minimalGrid");
            node->AddAttribute("GridSize", "400");
            wxString grp = currentLayoutGroup == "All Models" ? "Unassigned" : currentLayoutGroup;
            node->AddAttribute("LayoutGroup", grp);

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnModelsPopup::ID_MNU_ADD_MODEL_GROUP");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::OnModelsPopup::ID_MNU_ADD_MODEL_GROUP", nullptr, nullptr, name.ToStdString());

            //Model* model = xlights->GetModel(name.ToStdString());
            //SelectModelInTree(model);

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnModelsPopup::ID_MNU_ADD_MODEL_GROUP", nullptr, nullptr, name.ToStdString());
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnModelsPopup::ID_MNU_ADD_MODEL_GROUP", nullptr, nullptr, name.ToStdString());
        }
    } else if (id == ID_MNU_CLONE_MODEL_GROUP) {
        logger_base.debug("LayoutPanel::OnModelsPopup CLONE_MODEL_GROUP");

        wxString sel = TreeListViewModels->GetItemText(selectedTreeGroups[0]);
        ModelGroup* mg = dynamic_cast<ModelGroup*>(xlights->AllModels.GetModel(sel));
        if (mg == nullptr)
            return;
        std::string name = xlights->AllModels.GenerateModelName(sel);

        wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "modelGroup");
        for (auto it = mg->GetModelXml()->GetAttributes(); it != nullptr; it = it->GetNext()) {
            if (it->GetName() == "name") {
                node->AddAttribute("name", name);
            } else {
                node->AddAttribute(it->GetName(), it->GetValue());
            }
        }
        xlights->AllModels.AddModel(xlights->AllModels.CreateModel(node));
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::OnModelsPopup::ID_MNU_CLONE_MODEL_GROUP", nullptr, nullptr, name);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::OnModelsPopup::ID_MNU_CLONE_MODEL_GROUP");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::OnModelsPopup::ID_MNU_CLONE_MODEL_GROUP");
        //model_grp_panel->UpdatePanel(name);
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnModelsPopup::ID_MNU_CLONE_MODEL_GROUP");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "LayoutPanel::OnModelsPopup::ID_MNU_CLONE_MODEL_GROUP");
    }
    else if (event.GetId() == ID_PREVIEW_FLIP_HORIZONTAL) {
        if (editing_models) {
            PreviewModelFlipH();
        } else {
            objects_panel->PreviewObjectFlipH();
        }
    }
    else if (event.GetId() == ID_PREVIEW_FLIP_VERTICAL) {
        if (editing_models) {
            PreviewModelFlipV();
        } else {
            objects_panel->PreviewObjectFlipV();
        }
    }
}

LayoutGroup* LayoutPanel::GetLayoutGroup(const std::string &name)
{
    for (const auto& it : xlights->LayoutGroups) {
        LayoutGroup* grp = (LayoutGroup*)(it);
        if( grp->GetName() == name ) {
            return grp;
        }
    }
    return nullptr;
}

void LayoutPanel::OnChoiceLayoutGroupsSelect(wxCommandEvent& event)
{
    UnSelectAllModels();

    //for (const auto& it : xlights->AllModels) {
    //    Model *model = it.second;
    //    model->Selected = false;
    //    model->GroupSelected = false;
    //    model->Highlighted = false;
    //}

    std::string choice_layout = std::string(ChoiceLayoutGroups->GetStringSelection().c_str());
    if( choice_layout == "<Create New Preview>" ) {
        wxTextEntryDialog dlg(this, "Enter name for new preview", "Create New Preview");
        OptimiseDialogPosition(&dlg);
        if (dlg.ShowModal() == wxID_OK) {
            wxString name = dlg.GetValue();
            while (GetLayoutGroup(name.ToStdString()) != nullptr || name == "Default" || name == "All Models" || name == "Unassigned") {
                wxTextEntryDialog dlg2(this, "Preview of name " + name + " already exists. Enter name for new preview", "Create New Preview");
                OptimiseDialogPosition(&dlg2);
                if (dlg2.ShowModal() == wxID_OK) {
                    name = dlg2.GetValue();
                } else {
                    SwitchChoiceToCurrentLayoutGroup();
                    return;
                }
            }
            wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE, "layoutGroup");
            xlights->LayoutGroupsNode->AddChild(node);
            node->AddAttribute("name", name);

            //mSelectedGroup = nullptr;
            LayoutGroup* grp = new LayoutGroup(name.ToStdString(), xlights, node);
            grp->SetBackgroundImage(xlights->GetDefaultPreviewBackgroundImage());
            xlights->LayoutGroups.push_back(grp);
            xlights->AddPreviewOption(grp);
            SetCurrentLayoutGroup(name.ToStdString());
            AddPreviewChoice(name.ToStdString());
            ChoiceLayoutGroups->SetSelection(ChoiceLayoutGroups->GetCount() - 2);

            //xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::OnChoiceLayoutGroupsSelect");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::OnChoiceLayoutGroupsSelect");
            //ShowPropGrid(true);
        } else {
            SwitchChoiceToCurrentLayoutGroup();
            return;
        }
    } else {
        SetCurrentLayoutGroup(choice_layout);
        UpdateModelList(true);
    }
    modelPreview->SetDisplay2DBoundingBox(xlights->GetDisplay2DBoundingBox());
    modelPreview->SetDisplay2DGrid(xlights->GetDisplay2DGrid(), xlights->GetDisplay2DGridSpacing());
    modelPreview->SetDisplay2DCenter0(xlights->GetDisplay2DCenter0());
    modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
    modelPreview->SetScaleBackgroundImage(GetBackgroundScaledForSelectedPreview());
    modelPreview->SetBackgroundBrightness(GetBackgroundBrightnessForSelectedPreview(), GetBackgroundAlphaForSelectedPreview());
    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::OnChoiceLayoutGroupsSelect");

    xlights->SetStoredLayoutGroup(currentLayoutGroup);
    obj_button->Enable(is_3d && currentLayoutGroup == "Default");
}

void LayoutPanel::PreviewSaveImage()
{
	wxImage *image = modelPreview->GrabImage();
	if (image == nullptr)
	{
		static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
		logger_base.error("SavePreviewImage() - problem grabbing ModelPreview image");

		wxMessageDialog msgDlg(this, _("Error capturing preview image"), _("Image Capture Error"), wxOK | wxCENTRE);
		msgDlg.ShowModal();
		return;
	}

	const char wildcard[] = "JPG files (*.jpg;*.jpeg)|*.jpg;*.jpeg|PNG files (*.png)|*.png";
	wxFileDialog saveDlg(this, _("Save Image"), wxEmptyString, wxEmptyString,
		wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (saveDlg.ShowModal() == wxID_OK)
	{
		wxString path = saveDlg.GetPath();
		wxBitmapType bitmapType = path.EndsWith(".png") ? wxBITMAP_TYPE_PNG : wxBITMAP_TYPE_JPEG;
		image->SaveFile(path, bitmapType);
	}

	delete image;
}

void LayoutPanel::ImportModelsFromPreview(std::list<impTreeItemData*> models, wxString const& layoutGroup, bool includeEmptyGroups)
{
    log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    //add models first
    for (auto const& it2 : models)
    {
        if (!it2->IsModelGroup())
        {
            std::string newName = it2->GetName();
            if (xlights->AllModels.GetModel(newName) != nullptr) {
                newName = xlights->AllModels.GenerateModelName(it2->GetName());
            }
            it2->GetModelXml()->DeleteAttribute("name");
            it2->GetModelXml()->DeleteAttribute("LayoutGroup");
            it2->GetModelXml()->AddAttribute("name", newName);
            it2->GetModelXml()->AddAttribute("LayoutGroup", layoutGroup);
            xlights->AllModels.createAndAddModel(it2->GetModelXml(), modelPreview->getWidth(), modelPreview->getHeight());
            logger_base.debug("Imported model '%s' as '%s'.", (const char*)it2->GetName().c_str(), (const char*)newName.c_str());
        }
    }

    //add model groups second, skip adding duplicates, just add models to existing group
    for (auto const& it2 : models)
    {
        if (it2->IsModelGroup())//if a group, try to add models if exist
        {
            wxString const smodels = it2->GetModelXml()->GetAttribute("models");
            auto models = wxSplit(smodels, ',');

            models.erase(std::remove_if(models.begin(), models.end(), [&](std::string const& s)
                {
                    return (xlights->AllModels.GetModel(s) == nullptr);
                }), models.end());

            if (!includeEmptyGroups && models.empty()) {
                logger_base.warn("Import model group '%s' failed as no models in the group exist in this display.", (const char*)it2->GetName().c_str());
                continue;
            }

            wxString const name = it2->GetName();
            Model* model = xlights->AllModels.GetModel(name);
            if (model == nullptr) {//if group doesnt exist, create it
                it2->GetModelXml()->DeleteAttribute("LayoutGroup");
                it2->GetModelXml()->AddAttribute("LayoutGroup", layoutGroup);
                model = xlights->AllModels.createAndAddModel(it2->GetModelXml(), modelPreview->getWidth(), modelPreview->getHeight());
                logger_base.debug("Imported model group '%s'.", (const char*)name.c_str());
            }

            if (model->GetDisplayAs() == "ModelGroup") {
                ModelGroup *group = (ModelGroup*)model;
                for (const auto& m : models) {
                    // only add model to group if it doesn't already exist
                    if (group->GetModel(m) == nullptr) {
                        group->AddModel(m);
                        logger_base.debug("    Models model group '%s' added model '%s'.", (const char*)name.c_str(), (const char*)m.c_str());
                    }
                }
            }
        }
    }
}

void LayoutPanel::ImportModelsFromRGBEffects()
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
#ifdef __WXOSX__
    wxString wildcard = "*.xml";
#else
    wxString wildcard = "xlights_rgbeffects.xml";
#endif
    wxString filename = wxFileSelector(_("Choose RGB Effects file to import from"), wxEmptyString,
                                       XLIGHTS_RGBEFFECTS_FILE, wxEmptyString,
                                       "RGB Effects Files (xlights_rgbeffects.xml)|" + wildcard,
                                       wxFD_FILE_MUST_EXIST | wxFD_OPEN);
    if (filename.IsEmpty()) return;

    ImportPreviewsModelsDialog dlg(this, filename, xlights->AllModels, xlights->LayoutGroups);
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString lg = ChoiceLayoutGroups->GetStringSelection();
        if (lg == "All Models") lg = "Default";

        ImportModelsFromPreview(dlg.GetModelsInPreview(""), lg, dlg.GetIncludeEmptyGroups());

        for (const auto& it : dlg.GetPreviews())
        {
            bool found = false;
            for (size_t i = 0; i < ChoiceLayoutGroups->GetCount(); i++)
            {
                if (ChoiceLayoutGroups->GetString(i) == it)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "layoutGroup");
                xlights->LayoutGroupsNode->AddChild(node);
                node->AddAttribute("name", it);

                LayoutGroup* grp = new LayoutGroup(it.ToStdString(), xlights, node);
                grp->SetBackgroundImage(xlights->GetDefaultPreviewBackgroundImage());
                xlights->LayoutGroups.push_back(grp);
                xlights->AddPreviewOption(grp);
                AddPreviewChoice(it.ToStdString());
            }
            ImportModelsFromPreview(dlg.GetModelsInPreview(it), it, dlg.GetIncludeEmptyGroups());
        }
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::ImportModelsFromRGBEffects");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::ImportModelsFromRGBEffects");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "LayoutPanel::ImportModelsFromRGBEffects");
    }
}

void LayoutPanel::ImportModelsFromLORS5()
{
    wxString lg = ChoiceLayoutGroups->GetStringSelection();
    if (lg == "All Models") lg = "Default";
    LORPreview lorPreview(xlights, lg);

    if (lorPreview.LoadPreviewFile())
    {
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LayoutPanel::ImportModelsFromLORS5");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::ImportModelsFromLORS5");
    }
}

void LayoutPanel::PreviewPrintImage()
{
	class Printout : public wxPrintout
	{
	public:
		Printout(ModelPreview *canvas, bool invert) : _invert(invert), m_canvas(canvas) {}
		virtual ~Printout() {
			clearImage();
		}

		void clearImage() {
			if (m_image != nullptr) {
				delete m_image;
				m_image = nullptr;
			}
		}

		virtual bool GrabImage() {
			clearImage();

			wxRect rect = GetLogicalPageRect();
			rect.Deflate(rect.GetWidth() / 20, rect.GetHeight() / 20);
			wxRect adjustedRect = scaledRect(m_canvas->getWidth(), m_canvas->getHeight(), rect.GetWidth(), rect.GetHeight());

			m_image = m_canvas->GrabImage(wxSize(adjustedRect.GetWidth(), adjustedRect.GetHeight()));

            // invert the image for printing
            if (_invert && m_image != nullptr) {
                unsigned char* imgdata = m_image->GetData();
                unsigned int ch = m_image->HasAlpha() ? 4 : 3;
                const int imgdata_size = m_image->GetWidth() * m_image->GetHeight() * ch;
                for (auto i = 0; i < imgdata_size; i += ch) {
                    imgdata[i] = 255 - imgdata[i];
                    imgdata[i + 1] = 255 - imgdata[i + 1];
                    imgdata[i + 2] = 255 - imgdata[i + 2];
                }
            }

			m_grabbedImage = (m_image != nullptr);
			return m_grabbedImage;
		}

		virtual bool OnPrintPage(int page) override {
			if ( GrabImage() == false )
				return false;

			wxDC* dc = GetDC();
			wxRect rect = GetLogicalPageRect();


            wxRect r = scaledRect(m_image->GetWidth(), m_image->GetHeight(), rect.GetWidth(), rect.GetHeight());
            wxAffineMatrix2D mtx;
            double xScale = r.GetWidth() / double(m_image->GetWidth());
            double yScale = r.GetHeight() / double(m_image->GetHeight());
            mtx.Scale(xScale, yScale);
            dc->SetTransformMatrix(mtx);
            dc->DrawBitmap(*m_image, rect.GetTopLeft());
			return true;
		}

		bool grabbedImage() const { return m_grabbedImage; }
	protected:
        ModelPreview *m_canvas = nullptr;
		wxImage *m_image = nullptr;
		bool m_grabbedImage = false;
        bool _invert = false;
	};

	Printout printout(modelPreview, true);

	static wxPrintDialogData printDialogData;
	wxPrinter printer(&printDialogData);

	if (!printer.Print(this, &printout, true)) {
		if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
			DisplayError(wxString::Format("Problem printing. %d", wxPrinter::GetLastError()).ToStdString());
		}
    }
	else {
		printDialogData = printer.GetPrintDialogData();
        if (!printout.grabbedImage()) {
            DisplayError("Problem grabbing ModelPreview image for printing", this);
        }
    }
}

void LayoutPanel::AddPreviewChoice(const std::string& name)
{
    ChoiceLayoutGroups->Insert(name, ChoiceLayoutGroups->GetCount() - 1);
    model_grp_panel->AddPreviewChoice(name);

    // see if we need to switch to this one
    const std::string& storedLayoutGroup = xlights->GetStoredLayoutGroup();
    if (storedLayoutGroup == name) {
        for (int i = 0; i < (int)ChoiceLayoutGroups->GetCount(); i++) {
            if (ChoiceLayoutGroups->GetString(i) == storedLayoutGroup) {
                SetCurrentLayoutGroup(storedLayoutGroup);
                ChoiceLayoutGroups->SetSelection(i);
                modelPreview->SetDisplay2DBoundingBox(xlights->GetDisplay2DBoundingBox());
                modelPreview->SetDisplay2DGrid(xlights->GetDisplay2DGrid(), xlights->GetDisplay2DGridSpacing());
                modelPreview->SetDisplay2DCenter0(xlights->GetDisplay2DCenter0());
                modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
                modelPreview->SetScaleBackgroundImage(GetBackgroundScaledForSelectedPreview());
                modelPreview->SetBackgroundBrightness(GetBackgroundBrightnessForSelectedPreview(), GetBackgroundAlphaForSelectedPreview());
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::AddPreview");
                break;
            }
        }
    }
}

const wxString& LayoutPanel::GetBackgroundImageForSelectedPreview() {
    previewBackgroundFile = xlights->GetDefaultPreviewBackgroundImage();
    if (pGrp != nullptr && currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned") {
        previewBackgroundFile = pGrp->GetBackgroundImage();
    }
    return previewBackgroundFile;
}

bool LayoutPanel::GetBackgroundScaledForSelectedPreview() {
    previewBackgroundScaled = xlights->GetDefaultPreviewBackgroundScaled();
    if (pGrp != nullptr && currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned") {
        previewBackgroundScaled = pGrp->GetBackgroundScaled();
    }
    return previewBackgroundScaled;
}

int LayoutPanel::GetBackgroundBrightnessForSelectedPreview() {
    previewBackgroundBrightness = xlights->GetDefaultPreviewBackgroundBrightness();
    if (pGrp != nullptr && currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned") {
        previewBackgroundBrightness = pGrp->GetBackgroundBrightness();
    }
    return previewBackgroundBrightness;
}

int LayoutPanel::GetBackgroundAlphaForSelectedPreview()
{
    previewBackgroundAlpha = xlights->GetDefaultPreviewBackgroundAlpha();
    if (pGrp != nullptr && currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned") {
        previewBackgroundAlpha = pGrp->GetBackgroundAlpha();
    }
    return previewBackgroundAlpha;
}

void LayoutPanel::SwitchChoiceToCurrentLayoutGroup() {
    ChoiceLayoutGroups->SetSelection(0);
    for (int i = 0; i < (int)ChoiceLayoutGroups->GetCount(); ++i) {
        if (ChoiceLayoutGroups->GetString(i) == currentLayoutGroup) {
            ChoiceLayoutGroups->SetSelection(i);
            break;
        }
    }
}

void LayoutPanel::DeleteCurrentPreview() {
    if (wxMessageBox("Are you sure you want to delete the " + currentLayoutGroup + " preview?", "Confirm Delete?", wxICON_QUESTION | wxYES_NO) == wxYES) {
        for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); ++it) {
            LayoutGroup* grp = (LayoutGroup*)(*it);
            if (grp != nullptr) {
                if (currentLayoutGroup == grp->GetName()) {
                    xlights->RemovePreviewOption(grp);
                    grp->GetLayoutGroupXml()->GetParent()->RemoveChild(grp->GetLayoutGroupXml());
                    xlights->LayoutGroups.erase(it);
                    delete grp->GetLayoutGroupXml();
                    delete grp;
                    break;
                }
            }
        }
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::DeleteCurrentPreview");

        for (size_t i = 0; i < ChoiceLayoutGroups->GetCount(); ++i) {
            if (ChoiceLayoutGroups->GetString(i) == currentLayoutGroup) {
                ChoiceLayoutGroups->Delete(i);
                break;
            }
        }
        // change any existing assignments to this preview to be unassigned
        for (const auto& it : xlights->AllModels) {
            Model* model = it.second;
            if (model->GetLayoutGroup() == currentLayoutGroup) {
                model->SetLayoutGroup("Unassigned");
            }
        }

        SetCurrentLayoutGroup("Default");
        ChoiceLayoutGroups->SetSelection(0);
        xlights->SetStoredLayoutGroup(currentLayoutGroup);

        UpdateModelList(true);
        modelPreview->SetDisplay2DBoundingBox(xlights->GetDisplay2DBoundingBox());
        modelPreview->SetDisplay2DGrid(xlights->GetDisplay2DGrid(), xlights->GetDisplay2DGridSpacing());
        modelPreview->SetDisplay2DCenter0(xlights->GetDisplay2DCenter0());
        modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
        modelPreview->SetScaleBackgroundImage(GetBackgroundScaledForSelectedPreview());
        modelPreview->SetBackgroundBrightness(GetBackgroundBrightnessForSelectedPreview(), GetBackgroundAlphaForSelectedPreview());
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::DeleteCurrentPreview");
    }
}

void LayoutPanel::RenameCurrentPreview()
{
    wxTextEntryDialog dlg(this, "Change preview name", "Enter the new preview name:", currentLayoutGroup);
    do {
        if (dlg.ShowModal() == wxCANCEL)
            return;
    } while (::Lower(currentLayoutGroup) != ::Lower(dlg.GetValue()) && GetLayoutGroup(dlg.GetValue().ToStdString()) != nullptr);

    for (const auto& it : xlights->AllModels) {
        Model* model = it.second;
        if (model->GetLayoutGroup() == currentLayoutGroup) {
            model->SetLayoutGroup(dlg.GetValue());
        }
    }

    wxXmlNode* node = nullptr;
    for (wxXmlNode* n = xlights->LayoutGroupsNode->GetChildren(); n != nullptr; n = n->GetNext()) {
        if (n->GetAttribute("name") == currentLayoutGroup) {
            n->DeleteAttribute("name");
            n->AddAttribute("name", dlg.GetValue());
            node = n;
            break;
        }
    }

    for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); ++it) {
        if ((*it)->GetName() == currentLayoutGroup) {
            (*it)->SetName(dlg.GetValue());
            break;
        }
    }

     for (size_t i = 0; i < ChoiceLayoutGroups->GetCount(); ++i) {
        if (ChoiceLayoutGroups->GetString(i) == currentLayoutGroup) {
            ChoiceLayoutGroups->SetString(i, dlg.GetValue());
            break;
        }
    }

    currentLayoutGroup = dlg.GetValue();

    SetCurrentLayoutGroup(currentLayoutGroup);
    SwitchChoiceToCurrentLayoutGroup();
    xlights->SetStoredLayoutGroup(currentLayoutGroup);
    UpdateModelList(true);

    modelPreview->SetDisplay2DBoundingBox(xlights->GetDisplay2DBoundingBox());
    modelPreview->SetDisplay2DGrid(xlights->GetDisplay2DGrid(), xlights->GetDisplay2DGridSpacing());
    modelPreview->SetDisplay2DCenter0(xlights->GetDisplay2DCenter0());
    modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
    modelPreview->SetScaleBackgroundImage(GetBackgroundScaledForSelectedPreview());
    modelPreview->SetBackgroundBrightness(GetBackgroundBrightnessForSelectedPreview(), GetBackgroundAlphaForSelectedPreview());

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::RenameCurrentPreview");
    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::RenameCurrentPreview");
    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::RenameCurrentPreview");
}

void LayoutPanel::ShowPropGrid(bool show) {
    if( !mPropGridActive && show ) {
        ModelSplitter->ReplaceWindow(ModelGroupWindow, propertyEditor);
        ModelGroupWindow->Hide();
        propertyEditor->Show();
        mPropGridActive = true;
    } else if( mPropGridActive && !show) {
        ModelSplitter->ReplaceWindow(propertyEditor, ModelGroupWindow);
        propertyEditor->Hide();
        ModelGroupWindow->Show();
        mPropGridActive = false;
    }
}

void LayoutPanel::SetCurrentLayoutGroup(const std::string& group)
{
    currentLayoutGroup = group;
    for (const auto& it : xlights->LayoutGroups) {
        LayoutGroup* grp = (LayoutGroup*)(it);
        if (grp != nullptr) {
            if( currentLayoutGroup == grp->GetName() ) {
                pGrp = grp;
                modelPreview->SetActiveLayoutGroup(grp->GetName());
                return;
            }
        }
    }
    modelPreview->SetActiveLayoutGroup(group);
}

void LayoutPanel::OnItemContextMenu(wxTreeListEvent& event)
{
    if (!editing_models) {
        objects_panel->OnItemContextMenu(event);
        return;
    }

    wxMenu mnuContext;

    if (selectedTreeGroups.size() == 0) {
        if (selectedTreeSubModels.size() == 0) {
            if (selectedTreeModels.size() == 1) {
                mnuContext.Append(ID_MNU_DELETE_MODEL, "Delete Model");
                auto par = TreeListViewModels->GetItemParent(selectedTreeModels[0]);
                if (par != TreeListViewModels->GetRootItem()) {
                    mnuContext.Append(ID_MNU_REMOVE_MODEL_FROM_GROUP, "Remove Model From Group");
                }
                mnuContext.AppendSeparator();
            }

            if (selectedTreeModels.size() > 1) {
                auto parent = TreeListViewModels->GetItemParent(selectedTreeModels[0]);
                bool allSameParent = true;
                for (auto &i : selectedTreeModels) {
                    if (parent != TreeListViewModels->GetItemParent(i)) {
                        allSameParent = false;
                    }
                }
                mnuContext.Append(ID_MNU_DELETE_MODEL, "Delete Models");
                mnuContext.Append(ID_PREVIEW_MODEL_LOCK, "Lock Models");
                mnuContext.Append(ID_PREVIEW_MODEL_UNLOCK, "Unlock Models");
                
                if (allSameParent && parent != TreeListViewModels->GetRootItem()) {
                    mnuContext.Append(ID_MNU_REMOVE_MODEL_FROM_GROUP, "Remove Models From Group");
                }
                mnuContext.AppendSeparator();
            }
        } else {
            auto par = TreeListViewModels->GetItemParent(selectedTreeSubModels[0]);
            if (par != TreeListViewModels->GetRootItem()) {
                if ((selectedTreeSubModels.size() + selectedTreeModels.size()) == 1) {
                    mnuContext.Append(ID_MNU_REMOVE_MODEL_FROM_GROUP, "Remove Model From Group");
                } else {
                    mnuContext.Append(ID_MNU_REMOVE_MODEL_FROM_GROUP, "Remove Models From Group");
                }
                mnuContext.AppendSeparator();
            }
        }
    }

    if (selectedTreeModels.size() == 1 && selectedTreeGroups.size() + selectedTreeSubModels.size() == 0) {
        AddSingleModelOptionsToBaseMenu(mnuContext);
        // Remove preview 'Create Group' option as it may be confusing with tree list 'Create Group from Selections'
        mnuContext.Remove(ID_PREVIEW_MODEL_CREATEGROUP);
        // Remove preview option 'Add to Existing Group' as it is added with the other group options below
        wxMenuItem* addToExisting = mnuContext.FindItem(ID_MNU_ADD_TO_EXISTING_GROUPS);
        if (addToExisting != nullptr) {
            mnuContext.Remove(ID_MNU_ADD_TO_EXISTING_GROUPS);
        }
        mnuContext.AppendSeparator();
    }

    // add model menu options if only models selected and selected > 1n
    if (selectedTreeModels.size() > 1 && selectedTreeGroups.size() + selectedTreeSubModels.size() == 0) {
        wxMenu* mnuBulkEdit = new wxMenu();
        AddBulkEditOptionsToMenu(mnuBulkEdit);
        mnuBulkEdit->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnModelsPopup, nullptr, this);

        wxMenu* mnuAlign = new wxMenu();
        AddAlignOptionsToMenu(mnuAlign);
        mnuAlign->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnModelsPopup, nullptr, this);

        wxMenu* mnuDistribute = new wxMenu();
        AddDistributeOptionsToMenu(mnuDistribute);
        mnuDistribute->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnModelsPopup, nullptr, this);

        wxMenu* mnuResize = new wxMenu();
        AddResizeOptionsToMenu(mnuResize);
        mnuResize->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnModelsPopup, nullptr, this);

        mnuContext.Append(ID_PREVIEW_BULKEDIT, "Bulk Edit", mnuBulkEdit, "");
        mnuContext.Append(ID_PREVIEW_ALIGN, "Align", mnuAlign, "");
        mnuContext.Append(ID_PREVIEW_DISTRIBUTE, "Distribute", mnuDistribute, "");
        mnuContext.Append(ID_PREVIEW_RESIZE, "Resize", mnuResize, "");

        mnuContext.AppendSeparator();
    }

    mnuContext.Append(ID_MNU_ADD_MODEL_GROUP, "Add Empty Group");
    if ((selectedTreeModels.size() + selectedTreeSubModels.size() + selectedTreeGroups.size()) > 0) {
        for (const auto& m : xlights->AllModels) {
            if (m.second->GetDisplayAs() == "ModelGroup") {
                // adjust label for tree
                mnuContext.Append(ID_MNU_ADD_TO_EXISTING_GROUPS, "Add Selections to Existing Groups");
                break;
            }
        }
        mnuContext.Append(ID_PREVIEW_MODEL_CREATEGROUP, "Create Group from Selections");
    }

    if (selectedTreeModels.size() == 0 && selectedTreeSubModels.size() == 0) {
        if (selectedTreeGroups.size() > 1) {
            mnuContext.Append(ID_MNU_DELETE_MODEL_GROUP, "Delete Groups");
        }

        if (selectedTreeGroups.size() == 1) {
            mnuContext.Append(ID_MNU_DELETE_MODEL_GROUP, "Delete Group");
            mnuContext.Append(ID_MNU_RENAME_MODEL_GROUP, "Rename Group");
            mnuContext.Append(ID_MNU_CLONE_MODEL_GROUP, "Clone Group");
        }
    }

    mnuContext.Append(ID_MNU_DELETE_EMPTY_MODEL_GROUPS, "Delete Empty Groups");

    bool foundInvalid = false;
    bool foundOverlapping = false;

    if (selectedTreeModels.size() == 1 && selectedTreeSubModels.size() == 0 && selectedTreeGroups.size() == 0) {
        ModelTreeData* data = (ModelTreeData*)TreeListViewModels->GetItemData(selectedTreeModels[0]);
        Model* model = ((data != nullptr) ? data->GetModel() : nullptr);
        if (model) {
            if (!model->CouldComputeStartChannel || !model->IsValidStartChannelString()) {
                mnuContext.Append(ID_MNU_MAKESCVALID, "Make Start Channel Valid");
                foundInvalid = true;
            }
            if (xlights->AllModels.IsModelOverlapping(model)) {
                foundOverlapping = true;
                mnuContext.Append(ID_MNU_MAKESCVALID, "Make Start Channel Not Overlapping");
            }
        }
    }

    if (!foundOverlapping && !foundInvalid) { // no point looking again if we already know we have an issue
        for (const auto& it : xlights->AllModels) {
            if (it.second->GetDisplayAs() != "ModelGroup") {
                if (!foundInvalid && (!it.second->CouldComputeStartChannel || !it.second->IsValidStartChannelString())) {
                    foundInvalid = true;
                    if (foundOverlapping) break;
                }
                if (!foundOverlapping && xlights->AllModels.IsModelOverlapping(it.second)) {
                    foundOverlapping = true;
                    if (foundInvalid) break;
                }
            }
        }
    }

    if (foundInvalid) {
        mnuContext.Append(ID_MNU_MAKEALLSCVALID, "Make All Start Channels Valid");
    }
    if (foundOverlapping) {
        mnuContext.Append(ID_MNU_MAKEALLSCNOTOVERLAPPING, "Make All Start Channels Not Overlapping");
    }

    mnuContext.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&LayoutPanel::OnModelsPopup, nullptr, this);
    PopupMenu(&mnuContext);
}

static inline void SetToolTipForTreeList(wxTreeListCtrl *tv, const std::string &tip) {
#ifdef __WXMSW__
    if (tip == "") {
        tv->GetView()->UnsetToolTip();
    } else {
        tv->GetView()->SetToolTip(tip);
    }
#else
    if (tip == "") {
        tv->UnsetToolTip();
    } else {
        tv->SetToolTip(tip);
    }
#endif
}

void LayoutPanel::OnSelectionChanged(wxTreeListEvent& event)
{
    if (editing_models) {
        HandleSelectionChanged();
    } else {
        UnSelectAllModels(false);
        ViewObject* view_object = nullptr;
        bool show_prop_grid = objects_panel->OnSelectionChanged(event, &view_object, currentLayoutGroup);
        SelectViewObject(view_object, false);
        ShowPropGrid(show_prop_grid);
        selectedBaseObject = view_object;
    }
}

void LayoutPanel::HandleSelectionChanged() {

    log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // Even when Tree is Frozen which happens during full refresh this event is still fired on DeleteItem()/DeleteItems()
    // and randomly causes crash when model is nullptr, so bail when Frozen.  Also make sure tooltip is empty and property
    // grid is shown so background props show after full refresh when nothing is selected.
    if (TreeListViewModels->IsFrozen()) {
        //ShowPropGrid(true);
        showBackgroundProperties();
        SetToolTipForTreeList(TreeListViewModels, "");
        return;
    }

    BaseObject* lastSelectedBaseObject = selectedBaseObject;
    Model* lastSelectedModel = dynamic_cast<Model*>(lastSelectedBaseObject);
    wxTreeListItems selectedItems;
    TreeListViewModels->GetSelections(selectedItems);

    UnSelectAllModels(false);
    resetPropertyGrid();

    if (selectedItems.size() > 0) {
        bool isPrimary = false;
        if (selectedItems.size() == 1) {
            isPrimary = true;
        }

        for (const auto& item : selectedItems) {
            Model* model = GetModelFromTreeItem(item);
            if (model != nullptr) {
                #ifdef __LINUX__
                                // This seems to happen only on Linux so prevent the crash
                                if (!xlights->AllModels.IsModelValid(model)) {
                                    logger_base.debug("LINUX ONLY Error: LayoutPanel::OnSelectionChanged Model is Not Valid pointer. This would have crashed. Ignoring.");
                                    return;
                                }
                #elif defined(__WXOSX__)
                                // Given I am seeing these crashes on OSX but not windows I suspect like LINUX these crashes occur
                                // If is likely due to differences in the order messages arrive on the different platforms that results in invalid pointers
                                // This code will prove that theory
                                if (!xlights->AllModels.IsModelValid(model)) {
                                    logger_base.crit("LayoutPanel::OnSelectionChanged model was not valid ... this is going to crash.");
                                }
                #else
                                wxASSERT(xlights->AllModels.IsModelValid(model));
                #endif
                if (model->GetDisplayAs() == "ModelGroup") {
                    selectedTreeGroups.push_back(item);
                    SetTreeGroupModelsSelected(model, isPrimary);
                } else if (model->GetDisplayAs() == "SubModel") {
                    selectedTreeSubModels.push_back(item);
                    SetTreeSubModelSelected(model, isPrimary);
                } else {
                    selectedTreeModels.push_back(item);
                    if (model == lastSelectedModel || lastSelectedBaseObject == nullptr || isPrimary) {
                        selectedPrimaryTreeItem = item;
                        SetTreeModelSelected(model, true);
                    } else {
                        SetTreeModelSelected(model, false);
                    }
                }
            }
        }

        // if we still don't have a primary model selected then force one if we can
        if (selectedPrimaryTreeItem == nullptr) {
            if (selectedTreeModels.size() > 0) {
                Model* model = GetModelFromTreeItem(selectedTreeModels[0]);
                SetTreeModelSelected(model, true);
                selectedPrimaryTreeItem = selectedTreeModels[0];
            } else if (selectedTreeSubModels.size() > 0) {
                Model* model = GetModelFromTreeItem(selectedTreeSubModels[0]);
                SetTreeSubModelSelected(model, true);
            } else if (selectedTreeGroups.size() > 0){
                Model* model = GetModelFromTreeItem(selectedTreeGroups[0]);
                SetTreeGroupModelsSelected(model, true);
            }
        }

        // determine which panel and tooltip to show if any
        int mSize = selectedTreeModels.size();
        int gSize = selectedTreeGroups.size();
        int smSize = selectedTreeSubModels.size();
        int totalSelections = mSize + smSize + gSize;

        std::string tooltip = "";

        if (totalSelections > 1) {
            showBackgroundProperties();
            tooltip = wxString::Format("Selected Items:\n -Groups: %d\n -Models: %d\n -SubModels: %d\n", gSize, mSize, smSize);
        } else if (gSize == 1) {
            ShowPropGrid(false);
            model_grp_panel->UpdatePanel(TreeListViewModels->GetItemText(selectedTreeGroups[0]));
            model_grp_panel->Show();
        } else if (smSize == 1) {
            Model* subModel = GetModelFromTreeItem(selectedTreeSubModels[0]);
            SetupPropGrid(subModel);
            ShowPropGrid(true);
        } else if (mSize == 1) {
            Model* model = GetModelFromTreeItem(selectedTreeModels[0]);
            if (model != nullptr) {
                tooltip = xlights->GetChannelToControllerMapping(model->GetNumberFromChannelString(model->ModelStartChannel)) + "Nodes: " + wxString::Format("%d", (int)model->GetNodeCount()).ToStdString();
            } else {
                logger_base.crit("LayoutPanel::HandleSelectionChanged Model was selected and now is null, this should not have happened.");
            }
            SetupPropGrid(model);
            ShowPropGrid(true);
        } else {
            logger_base.crit("LayoutPanel::HandleSelectionChanged No models selected after processing, this should not have happen, when we started there were %d selections.", selectedItems.size());
            showBackgroundProperties();
        }

        SetToolTipForTreeList(TreeListViewModels, tooltip);

        // removing below or Keyboard Cut/Copy/Paste/etc will not fire when making selections in preview
        // #ifndef LINUX
        // TreeListViewModels->SetFocus();
        // #endif

        auto pos = selectedBaseObject->GetBaseObjectScreenLocation().GetWorldPosition();
        if (Is3d()) {
            auto pos = selectedBaseObject->GetBaseObjectScreenLocation().GetWorldPosition();
            xlights->SetStatusText(wxString::Format("x=%.2f y=%.2f z=%.2f %s", pos.x, pos.y, pos.z, selectedBaseObject->GetDimension()));
        } else {
            xlights->SetStatusText(wxString::Format("x=%.2f y=%.2f", pos.x, pos.y));
        }

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::HandleSelectionChanged");
    } else {
        selectedBaseObject = nullptr;
        UnSelectAllModels(true);
        showBackgroundProperties();
        SetToolTipForTreeList(TreeListViewModels, "");
        xlights->SetStatusText("");
    }
}

void LayoutPanel::ModelGroupUpdated(ModelGroup *grp, bool full_refresh) {

    if (grp == nullptr) return;

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::ModelGroupUpdated", nullptr, nullptr, grp->GetName());
    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LayoutPanel::ModelGroupUpdated");
    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "LayoutPanel::ModelGroupUpdated");

    std::vector<Model *> models;
    UpdateModelList(full_refresh, models);

    // Not sure why this was here as I commented this out in older build and didn't notice any different,
    // just commenting out for now in case I missed something as it was causing issues with tree selection
    //UnSelectAllModels();

    if (full_refresh) return;

    FreezeTreeListView();

    std::vector<Model *> modelsToAdd(models);

    wxTreeListItem root = TreeListViewModels->GetRootItem();
    std::vector<wxTreeListItem> toRemove;

    for (wxTreeListItem item = TreeListViewModels->GetFirstItem(); item.IsOk(); item = TreeListViewModels->GetNextItem(item))
    {
        ModelTreeData *data = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
        if (data != nullptr && data->GetModel() != nullptr) {
            if (data->GetModel()->GetFullName() == grp->GetFullName())
            {
                bool expanded = TreeListViewModels->IsExpanded(item);
                wxTreeListItem child = TreeListViewModels->GetFirstChild(item);
                while (child.IsOk()) {
                    TreeListViewModels->DeleteItem(child);
                    child = TreeListViewModels->GetFirstChild(item);
                }
                int i = 0;
                for (const auto& it : grp->ModelNames()) {
                    Model *m = xlights->AllModels[it];
                    if (m != nullptr) {
                        if (currentLayoutGroup == "All Models" ||
                            m->GetLayoutGroup() == currentLayoutGroup ||
                            (m->GetLayoutGroup() == "All Previews" && currentLayoutGroup != "Unassigned"))
                        {
                            AddModelToTree(m, &item, false, i, true);
                        }
                        if (m->DisplayAs == "SubModel"
                            && std::find(modelsToAdd.begin(), modelsToAdd.end(), m) != modelsToAdd.end()) {
                            modelsToAdd.erase(std::find(modelsToAdd.begin(), modelsToAdd.end(), m));
                        }
                        i++;
                    }
                }
                if (expanded) {
                    TreeListViewModels->Expand(item);
                }
            }
            else if (data->GetModel()->GetDisplayAs() != "ModelGroup"
                && data->GetModel()->GetDisplayAs() != "SubModel") {
                if (std::find(models.begin(), models.end(), data->GetModel()) == models.end()) {
                    toRemove.push_back(item);
                }
            }
            wxTreeListItem parent = TreeListViewModels->GetItemParent(item);
            if (!parent.IsOk() || parent == root) {
                //root item, see if we have this
                if (std::find(modelsToAdd.begin(), modelsToAdd.end(), data->GetModel()) != modelsToAdd.end()) {
                    modelsToAdd.erase(std::find(modelsToAdd.begin(), modelsToAdd.end(), data->GetModel()));
                }
            }
        }
    }

    for (const auto& a : toRemove) {
        TreeListViewModels->DeleteItem(a);
    }

    for (const auto& a : modelsToAdd) {
        TreeListViewModels->GetRootItem();
        AddModelToTree(a, &root, false, 0);
    }

    ThawTreeListView();
}

CopyPasteBaseObject::~CopyPasteBaseObject()
{
    if (_xmlNode != nullptr)
    {
        delete _xmlNode;
    }
}

CopyPasteBaseObject::CopyPasteBaseObject()
{
    _ok = false;
    _xmlNode = nullptr;
	_viewObject = false;
}

CopyPasteBaseObject::CopyPasteBaseObject(const std::string& in)
{
    _ok = false;
    _xmlNode = nullptr;
	_viewObject = false;

    // check it looks like xml ... to stop parser errors
    wxString xml = in;
    if (!xml.StartsWith("<?xml") || (!xml.Contains("<model ") && !xml.Contains("<view_object ")))
    {
        // not valid
        return;
    }

	if (xml.Contains("<view_object "))
	{
		// not valid
		_viewObject = true;
	}

    wxStringInputStream strm(xml);
    wxXmlDocument doc(strm);

    wxXmlNode* xmlNode = doc.GetRoot();

    if (xmlNode == nullptr)
    {
        // not valid
        return;
    }

    _xmlNode = new wxXmlNode(*xmlNode);

    _ok = true;
}

void CopyPasteBaseObject::SetBaseObject(BaseObject* model)
{
    if (_xmlNode != nullptr)
    {
        delete _xmlNode;
        _xmlNode = nullptr;
    }

    if (model == nullptr)
    {
        _ok = false;
    }
    else
    {
        _xmlNode = new wxXmlNode(*model->GetModelXml());
        _ok = true;
    }
}

std::string CopyPasteBaseObject::Serialise() const
{
    if (_xmlNode == nullptr)
    {
        return "";
    }
    else
    {
        wxXmlDocument doc;
        doc.SetRoot(_xmlNode);
        wxStringOutputStream stream;
        doc.Save(stream);
        std::string copyData = stream.GetString().ToStdString();
        doc.DetachRoot();
        return copyData;
    }
}

void LayoutPanel::OnCheckBox_3DClick(wxCommandEvent& event)
{
    is_3d = CheckBox_3D->GetValue();

    if (is_3d) {
        if (ChoiceLayoutGroups->GetStringSelection() != "Default") {
            ChoiceLayoutGroups->SetStringSelection("Default");
            wxCommandEvent e;
            OnChoiceLayoutGroupsSelect(e);
        }
        ChoiceLayoutGroups->Disable();
        ChoiceLayoutGroups->SetToolTip("3D is only supported in the Default preview.");
    } else {
        ChoiceLayoutGroups->Enable();
        ChoiceLayoutGroups->UnsetToolTip();
    }

    modelPreview->Set3D(is_3d);
    if (is_3d) {
        if (selectedBaseObject != nullptr) {
            selectionLatched = true;
            highlightedBaseObject = selectedBaseObject;
            selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
            selectedBaseObject->EnableLayoutGroupProperty(propertyEditor, false);
        } else {
            UnSelectAllModels();
        }
	    Notebook_Objects->AddPage(PanelObjects, _("3D Objects"), false);
    } else {
        editing_models = true;
        Model *m = dynamic_cast<Model*>(selectedBaseObject);
        if (m != nullptr) {
            m->EnableLayoutGroupProperty(propertyEditor, true);
        } else {
            UnSelectAllModels();
        }
        Notebook_Objects->RemovePage(1);
    }
    obj_button->Enable(is_3d && ChoiceLayoutGroups->GetStringSelection() == "Default");

    wxConfigBase* config = wxConfigBase::Get();
    config->Write("LayoutMode3D", is_3d);
    Refresh();
}

bool LayoutPanel::HandleLayoutKeyBinding(wxKeyEvent& event)
{
    log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    auto k = event.GetKeyCode();
    if (k == WXK_SHIFT || k == WXK_CONTROL || k == WXK_ALT) return false;

    if ((!event.ControlDown() && !event.CmdDown() && !event.AltDown()) ||
        (k == 'A' && (event.ControlDown() || event.CmdDown()) && !event.AltDown())) {
        // let crontrol A through
        // Just a regular key ... If current focus is a control then we need to not process this
        if (dynamic_cast<wxControl*>(event.GetEventObject()) != nullptr &&
            (k < 128 || k == WXK_NUMPAD_END || k == WXK_NUMPAD_HOME || k == WXK_NUMPAD_INSERT || k == WXK_HOME || k == WXK_END || k == WXK_NUMPAD_SUBTRACT || k == WXK_NUMPAD_DECIMAL)) {
            return false;
        }
    }

    auto binding = xlights->GetMainSequencer()->keyBindings.Find(event, KBSCOPE::Layout);
    if (binding != nullptr) {
        std::string type = binding->GetType();
        if (type == "LOCK_MODEL") {
            LockSelectedModels(true);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::HandleLayoutKey::LOCK_MODEL");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::HandleLayoutKey::LOCK_MODEL");
        }
        else if (type == "UNLOCK_MODEL") {
            LockSelectedModels(false);
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::HandleLayoutKey::UNLOCK_MODEL");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "LayoutPanel::HandleLayoutKey::UNLOCK_MODEL");
        }
        else if (type == "GROUP_MODELS") {
            CreateModelGroupFromSelected();
        }
        else if (type == "WIRING_VIEW") {
            ShowWiring();
        }
        else if (type == "EXPORT_MODEL_CAD") {
            ExportModelAsCAD();
        }
        else if (type == "EXPORT_LAYOUT_DXF") {
            ExportLayoutDXF();
        }
        else if (type == "NODE_LAYOUT") {
            ShowNodeLayout();
        } else if (type == "MODEL_SUBMODELS") {
            EditSubmodels();
        } else if (type == "MODEL_FACES") {
            EditFaces();
        } else if (type == "MODEL_STATES") {
            EditStates();
        } else if (type == "MODEL_MODELDATA") {
            EditModelData();
        } else if (type == "SAVE_LAYOUT") {
            SaveEffects();
            if (xlights->IsControllersAndLayoutTabSaveLinked()) {
                xlights->SaveNetworksFile();
            }
        }
        else if (type == "MODEL_ALIGN_TOP") {
            PreviewModelAlignTops();
        }
        else if (type == "MODEL_ALIGN_BOTTOM") {
            PreviewModelAlignBottoms();
        }
        else if (type == "MODEL_ALIGN_LEFT") {
            PreviewModelAlignLeft();
        }
        else if (type == "MODEL_ALIGN_RIGHT") {
            PreviewModelAlignRight();
        }
        else if (type == "MODEL_ALIGN_CENTER_VERT") {
            PreviewModelAlignVCenter();
        }
        else if (type == "MODEL_ALIGN_CENTER_HORIZ") {
            PreviewModelAlignHCenter();
        } 
        else if (type == "MODEL_ALIGN_BACKS") {
            PreviewModelAlignBacks();
        } 
        else if (type == "MODEL_ALIGN_FRONTS") {
            PreviewModelAlignFronts();
        } 
        else if (type == "MODEL_ALIGN_GROUND") {
            PreviewModelAlignWithGround();
        }
        else if (type == "MODEL_DISTRIBUTE_HORIZ") {
            PreviewModelHDistribute();
        }
        else if (type == "SELECT_ALL_MODELS") {
            SelectAllModels();
        }
        else if (type == "MODEL_DISTRIBUTE_VERT") {
            PreviewModelVDistribute();
        } else if (type == "MODEL_FLIP_VERT") {
            PreviewModelFlipV();
        } else if (type == "MODEL_FLIP_HORIZ") {
            PreviewModelFlipH();
        }
        else {
            logger_base.warn("Keybinding '%s' not recognised.", (const char*)type.c_str());
            wxASSERT(false);
            return false;
        }
        event.StopPropagation();
        return true;
    }

    return xlights->HandleAllKeyBinding(event);
}

void LayoutPanel::OnNotebook_ObjectsPageChanged(wxNotebookEvent& event)
{
#ifdef __WXOSX__
    UnSelectAllModels();
#else
    UnSelectAllModelsInTree();
#endif
    if (Notebook_Objects->GetPageText(Notebook_Objects->GetSelection()) == "Models") {
        editing_models = true;
    } else {
        editing_models = false;
    }
}

bool LayoutPanel::IsNewModel(Model* m) const
{
    // if nullptr is passed then it is true if the new model pointer is valid
    if (m == nullptr) return _newModel != nullptr;

    return m == _newModel;
}
