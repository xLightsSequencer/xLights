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

#include "LayoutPanel.h"
#include "ModelPreview.h"
#include "xLightsMain.h"
#include "DrawGLUtils.h"
#include "ChannelLayoutDialog.h"
#include "ControllerConnectionDialog.h"
#include "ModelGroupPanel.h"
#include "ViewObjectPanel.h"
#include "LayoutGroup.h"
#include "models/ModelImages.h"
#include "models/SubModel.h"
#include "models/ModelGroup.h"
#include "models/ViewObject.h"
#include "WiringDialog.h"
#include "ModelDimmingCurveDialog.h"
#include "UtilFunctions.h"
#include "ColorManager.h"
#include "support/VectorMath.h"
#include "osxMacUtils.h"
#include "KeyBindings.h"
#include "sequencer/MainSequencer.h"

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
const long LayoutPanel::ID_CHECKBOXOVERLAP = wxNewId();
const long LayoutPanel::ID_BUTTON_SAVE_PREVIEW = wxNewId();
const long LayoutPanel::ID_PANEL5 = wxNewId();
const long LayoutPanel::ID_STATICTEXT1 = wxNewId();
const long LayoutPanel::ID_CHOICE_PREVIEWS = wxNewId();
const long LayoutPanel::ID_CHECKBOX_3D = wxNewId();
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
    //EVT_TREELIST_ITEM_EXPANDING(wxID_ANY, LayoutPanel::OnItemExpanding)
    //EVT_TREELIST_ITEM_EXPANDED(wxID_ANY, LayoutPanel::OnSelectionChanged)
    //EVT_TREELIST_ITEM_CHECKED(wxID_ANY, LayoutPanel::OnItemChecked)
    //EVT_TREELIST_ITEM_ACTIVATED(wxID_ANY, LayoutPanel::OnSelectionChanged)
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
const long LayoutPanel::ID_PREVIEW_MODEL_CREATEGROUP = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_WIRINGVIEW = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_ASPECTRATIO = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL = wxNewId();
const long LayoutPanel::ID_PREVIEW_RESIZE_SAMEWIDTH = wxNewId();
const long LayoutPanel::ID_PREVIEW_RESIZE_SAMEHEIGHT = wxNewId();
const long LayoutPanel::ID_PREVIEW_RESIZE_SAMESIZE = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT = wxNewId();
const long LayoutPanel::ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTION = wxNewId();
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
const long LayoutPanel::ID_MNU_DELETE_MODEL = wxNewId();
const long LayoutPanel::ID_MNU_DELETE_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_MNU_DELETE_EMPTY_MODEL_GROUPS = wxNewId();
const long LayoutPanel::ID_MNU_RENAME_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_MNU_MAKESCVALID = wxNewId();
const long LayoutPanel::ID_MNU_MAKEALLSCVALID = wxNewId();
const long LayoutPanel::ID_MNU_MAKEALLSCNOTOVERLAPPING = wxNewId();
const long LayoutPanel::ID_MNU_ADD_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_PREVIEW_DELETE_ACTIVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_ADDPOINT = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_DELETEPOINT = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_ADDCURVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_DELCURVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_SAVE_LAYOUT_IMAGE = wxNewId();
const long LayoutPanel::ID_PREVIEW_PRINT_LAYOUT_IMAGE = wxNewId();
const long LayoutPanel::ID_PREVIEW_SAVE_VIEWPOINT = wxNewId();
const long LayoutPanel::ID_PREVIEW_VIEWPOINT2D = wxNewId();
const long LayoutPanel::ID_PREVIEW_VIEWPOINT3D = wxNewId();
const long LayoutPanel::ID_PREVIEW_DELETEVIEWPOINT2D = wxNewId();
const long LayoutPanel::ID_PREVIEW_DELETEVIEWPOINT3D = wxNewId();
const long LayoutPanel::ID_ADD_OBJECT_IMAGE = wxNewId();
const long LayoutPanel::ID_ADD_OBJECT_GRIDLINES = wxNewId();
const long LayoutPanel::ID_ADD_OBJECT_MESH = wxNewId();

#define CHNUMWIDTH "10000000000000"

class ModelTreeData : public wxTreeItemData {
public:
    ModelTreeData(Model *m, int NativeOrder) :wxTreeItemData(), model(m) {
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
private:
    Model *model;
    std::string subModel;
};

class NewModelBitmapButton : public wxBitmapButton
{
public:

    NewModelBitmapButton(wxWindow *parent, const wxBitmap &bmp, const std::string &type)
        : wxBitmapButton(parent, wxID_ANY, bmp), bitmap(bmp), state(0), modelType(type) {
        SetToolTip("Create new " + type);
    }
    virtual ~NewModelBitmapButton() {}

    void SetState(unsigned int s) {
        if (s > 2) {
            s = 0;
        }
        state = s;
        if (state == 2) {
            SetBitmap(bitmap.ConvertToDisabled());
        } else if (state == 1) {
            const wxImage imgDisabled = bitmap.ConvertToImage().ConvertToDisabled(128);
            SetBitmap(wxBitmap(imgDisabled, -1, bitmap.GetScaleFactor()));
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
    unsigned int state;
    wxBitmap bitmap;
};

#include <log4cpp/Category.hh>

LayoutPanel::LayoutPanel(wxWindow* parent, xLightsFrame *xl, wxPanel* sequencer) : xlights(xl), main_sequencer(sequencer),
    m_creating_bound_rect(false), mPointSize(2), m_moving_handle(false), m_dragging(false),
    m_over_handle(-1), selectedButton(nullptr), obj_button(nullptr), newModel(nullptr), selectedBaseObject(nullptr), highlightedBaseObject(nullptr),
    colSizesSet(false), updatingProperty(false), mNumGroups(0), mPropGridActive(true), last_selection(nullptr), last_highlight(nullptr),
    mSelectedGroup(nullptr), currentLayoutGroup("Default"), pGrp(nullptr), backgroundFile(""), previewBackgroundScaled(false),
    previewBackgroundBrightness(100), previewBackgroundAlpha(100), m_polyline_active(false), mHitTestNextSelectModelIndex(0),
    ModelGroupWindow(nullptr), ViewObjectWindow(nullptr), editing_models(true), m_mouse_down(false), m_wheel_down(false),
    selectionLatched(false), over_handle(-1), creating_model(false)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    background = nullptr;
    _firstTreeLoad = true;
    _lastXlightsModel = "";
    appearanceVisible = sizeVisible = stringPropsVisible = controllerConnectionVisible = false;

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
	SplitterWindow2->SetMinSize(wxSize(10,10));
	SplitterWindow2->SetSashGravity(0.5);
	LeftPanel = new wxPanel(SplitterWindow2, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
	LeftPanelSizer = new wxFlexGridSizer(0, 1, 0, 0);
	LeftPanelSizer->AddGrowableCol(0);
	LeftPanelSizer->AddGrowableRow(0);
	ModelSplitter = new wxSplitterWindow(LeftPanel, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	ModelSplitter->SetMinSize(wxSize(100,100));
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
	FlexGridSizer4->Fit(FirstPanel);
	FlexGridSizer4->SetSizeHints(FirstPanel);
	SecondPanel = new wxPanel(ModelSplitter, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	ModelSplitter->SplitHorizontally(FirstPanel, SecondPanel);
	LeftPanelSizer->Add(ModelSplitter, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	CheckBoxOverlap = new wxCheckBox(LeftPanel, ID_CHECKBOXOVERLAP, _("Overlap checks enabled"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOXOVERLAP"));
	CheckBoxOverlap->SetValue(false);
	FlexGridSizer3->Add(CheckBoxOverlap, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	ButtonSavePreview = new wxButton(LeftPanel, ID_BUTTON_SAVE_PREVIEW, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_PREVIEW"));
	FlexGridSizer2->Add(ButtonSavePreview, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	LeftPanelSizer->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	LeftPanel->SetSizer(LeftPanelSizer);
	LeftPanelSizer->Fit(LeftPanel);
	LeftPanelSizer->SetSizeHints(LeftPanel);
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
	CheckBox_3D = new wxCheckBox(PreviewGLPanel, ID_CHECKBOX_3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_3D"));
	CheckBox_3D->SetValue(false);
	FlexGridSizer1->Add(CheckBox_3D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	PreviewGLSizer->Fit(PreviewGLPanel);
	PreviewGLSizer->SetSizeHints(PreviewGLPanel);
	SplitterWindow2->SplitVertically(LeftPanel, PreviewGLPanel);
	FlexGridSizerPreview->Add(SplitterWindow2, 1, wxALL|wxEXPAND, 1);
	SetSizer(FlexGridSizerPreview);
	FlexGridSizerPreview->Fit(this);
	FlexGridSizerPreview->SetSizeHints(this);

	Connect(ID_NOTEBOOK_OBJECTS,wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&LayoutPanel::OnNotebook_ObjectsPageChanged);
	Connect(ID_SPLITTERWINDOW1,wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED,(wxObjectEventFunction)&LayoutPanel::OnModelSplitterSashPosChanged);
	Connect(ID_CHECKBOXOVERLAP,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&LayoutPanel::OnCheckBoxOverlapClick);
	Connect(ID_BUTTON_SAVE_PREVIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LayoutPanel::OnButtonSavePreviewClick);
	Connect(ID_CHOICE_PREVIEWS,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&LayoutPanel::OnChoiceLayoutGroupsSelect);
	Connect(ID_CHECKBOX_3D,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&LayoutPanel::OnCheckBox_3DClick);
	Connect(ID_SPLITTERWINDOW2,wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED,(wxObjectEventFunction)&LayoutPanel::OnSplitterWindowSashPosChanged);
	//*)

    ScrollBarLayoutHorz->Hide();
    ScrollBarLayoutVert->Hide();

    logger_base.debug("LayoutPanel basic setup complete");
    modelPreview = new ModelPreview( (wxPanel*) PreviewGLPanel, xlights, true);
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

    modelPreview->EnableTouchEvents(wxTOUCH_ROTATE_GESTURE | wxTOUCH_ZOOM_GESTURE);
    modelPreview->Connect(wxEVT_GESTURE_ROTATE, (wxObjectEventFunction)&LayoutPanel::OnPreviewRotateGesture, nullptr, this);
    modelPreview->Connect(wxEVT_GESTURE_ZOOM, (wxObjectEventFunction)&LayoutPanel::OnPreviewZoomGesture, nullptr, this);


    propertyEditor = new wxPropertyGrid(ModelSplitter,
                                        wxID_ANY, // id
                                        wxDefaultPosition, // position
                                        wxDefaultSize, // size
                                        // Here are just some of the supported window styles
                                        //wxPG_AUTO_SORT | // Automatic sorting after items added
                                        wxPG_SPLITTER_AUTO_CENTER | // Automatically center splitter until user manually adjusts it
                                        // Default style
                                        wxPG_DEFAULT_STYLE );
#ifdef __WXOSX__
    propertyEditor->SetExtraStyle(wxPG_EX_NATIVE_DOUBLE_BUFFERING | wxWS_EX_PROCESS_IDLE);
#else
    propertyEditor->SetExtraStyle(wxWS_EX_PROCESS_IDLE);
#endif
    InitImageList();

    wxFlexGridSizer* FlexGridSizerModels = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizerModels->AddGrowableCol(0);
	FlexGridSizerModels->AddGrowableRow(0);
	wxPanel* new_panel = new wxPanel(PanelModels, wxNewId(), wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_OBJECTS_PANEL"));
	FlexGridSizerModels->Add(new_panel, 1, wxALL|wxEXPAND, 0);
    PanelModels->SetSizer(FlexGridSizerModels);
    wxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
    TreeListViewModels = CreateTreeListCtrl(wxTL_DEFAULT_STYLE, new_panel);
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
    modelPreview->Set3D(is_3d);

    propertyEditor->Connect(wxEVT_PG_CHANGING, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridChanging,0,this);
    propertyEditor->Connect(wxEVT_PG_CHANGED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridChange,0,this);
    propertyEditor->Connect(wxEVT_PG_SELECTED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridSelection,0,this);
    propertyEditor->Connect(wxEVT_PG_ITEM_COLLAPSED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridItemCollapsed,0,this);
    propertyEditor->Connect(wxEVT_PG_ITEM_EXPANDED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridItemExpanded,0,this);
    propertyEditor->SetValidationFailureBehavior(wxPG_VFB_MARK_CELL | wxPG_VFB_BEEP);

    logger_base.debug("LayoutPanel property grid created");

    ToolSizer->SetCols(21);
    AddModelButton("Arches", arches);
    AddModelButton("Candy Canes", canes);
    AddModelButton("Channel Block", channelblock_xpm);
    AddModelButton("Circle", circles);
    AddModelButton("Cube", cube_xpm);
    AddModelButton("Custom", custom);
    AddModelButton("DMX", dmx_xpm);
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
    obj_button->Enable(is_3d);

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

void AddIcon(wxImageList &list, const std::string &id, double scaleFactor) {
    wxSize iconSize = list.GetSize();
    wxBitmap bmp =  wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(id), wxART_LIST, wxDefaultSize);
    if (bmp.GetSize() != iconSize) {
        wxImage img = bmp.ConvertToImage();
#ifdef __WXOSX__
        img.Rescale(iconSize.x, iconSize.y);
        wxBitmap bmp2 = wxBitmap(img);
        wxIcon icon;
        icon.CopyFromBitmap(bmp2);
        int i = list.Add(icon);
        img = bmp.ConvertToImage();
        img.Rescale(iconSize.x * scaleFactor, iconSize.y * scaleFactor);
        bmp2 = wxBitmap(img);
        icon.CopyFromBitmap(bmp2);
        list.Replace(i, icon);
#else
        img.Rescale(iconSize.x, iconSize.y);
        wxBitmap bmp2 = wxBitmap(img);
        wxIcon icon;
        icon.CopyFromBitmap(bmp2);
        list.Add(icon);
#endif
    } else {
        wxIcon icon;
        icon.CopyFromBitmap(bmp);
        list.Add(icon);
    }
}

void LayoutPanel::InitImageList()
{
    double scaleFactor = GetContentScaleFactor();
    wxSize iconSize = wxArtProvider::GetSizeHint(wxART_LIST);
    if ( iconSize == wxDefaultSize ) {
        iconSize = wxSize(ScaleWithSystemDPI(scaleFactor, 16),
                          ScaleWithSystemDPI(scaleFactor, 16));
    } else {
        iconSize = wxSize(ScaleWithSystemDPI(scaleFactor, iconSize.x),
                          ScaleWithSystemDPI(scaleFactor, iconSize.y));
    }

    m_imageList = new wxImageList(iconSize.x, iconSize.y);

    AddIcon(*m_imageList, "wxART_NORMAL_FILE", scaleFactor);
    AddIcon(*m_imageList, "xlART_GROUP_CLOSED", scaleFactor);
    AddIcon(*m_imageList, "xlART_GROUP_OPEN", scaleFactor);
    AddIcon(*m_imageList, "xlART_ARCH_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_CANE_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_CIRCLE_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_CHANNELBLOCK_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_CUBE_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_CUSTOM_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_DMX_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_ICICLE_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_IMAGE_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_LINE_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_MATRIX_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_POLY_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_SPHERE_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_SPINNER_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_STAR_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_SUBMODEL_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_TREE_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_WINDOW_ICON", scaleFactor);
    AddIcon(*m_imageList, "xlART_WREATH_ICON", scaleFactor);
}
wxTreeListCtrl* LayoutPanel::CreateTreeListCtrl(long style, wxPanel* panel)
{
    wxTreeListCtrl* const
        tree = new wxTreeListCtrl(panel, ID_TREELISTVIEW_MODELS,
                                  wxDefaultPosition, wxDefaultSize,
                                  style, "ID_TREELISTVIEW_MODELS");
    tree->SetImageList(m_imageList);

    tree->AppendColumn("Model / Group",
                       wxCOL_WIDTH_AUTOSIZE,
                       wxALIGN_LEFT,
                       wxCOL_RESIZABLE | wxCOL_SORTABLE);
    tree->AppendColumn("Start Chan",
                       tree->WidthFor(CHNUMWIDTH),
                       wxALIGN_LEFT,
                       wxCOL_RESIZABLE | wxCOL_SORTABLE);
    tree->AppendColumn("End Chan",
                       tree->WidthFor(CHNUMWIDTH),
                       wxALIGN_LEFT,
                       wxCOL_RESIZABLE | wxCOL_SORTABLE);
    tree->AppendColumn("Ctrlr Conn",
                       wxCOL_WIDTH_AUTOSIZE,
                       wxALIGN_LEFT,
                       wxCOL_RESIZABLE);
    tree->SetSortColumn(0, true);
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
    for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); ++it) {
        LayoutGroup* grp = (LayoutGroup*)(*it);
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
}

void LayoutPanel::SetDirtyHiLight(bool dirty) {
#ifdef __WXOSX__
    if (dirty) {
        ButtonSavePreview->SetBackgroundColour(wxColour(255,0,0));
        xlights->UnsavedRgbEffectsChanges = true;
    } else {
        ButtonSavePreview->SetBackgroundColour(wxTransparentColour);
    }
    ButtonSavePreview->Refresh();
#else
    if (dirty) {
        ButtonSavePreview->SetBackgroundColour(wxColour(255,108,108));
        xlights->UnsavedRgbEffectsChanges = true;
    } else {
        ButtonSavePreview->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    }
#endif
}

std::string LayoutPanel::GetCurrentPreview() const
{
    return ChoiceLayoutGroups->GetStringSelection().ToStdString();
}

NewModelBitmapButton* LayoutPanel::AddModelButton(const std::string &type, const char *data[]) {
    wxImage image(data);
#ifdef __WXOSX__
    wxBitmap bitmap(image, -1, 2.0);
#else
    image.Rescale(ScaleWithSystemDPI(GetContentScaleFactor(), 24),
                  ScaleWithSystemDPI(GetContentScaleFactor(), 24),
                  wxIMAGE_QUALITY_HIGH);
    wxBitmap bitmap(image);
#endif

    NewModelBitmapButton *button = new NewModelBitmapButton(PreviewGLPanel, bitmap, type);
    ToolSizer->Add(button, 1, wxALL, 0);
    buttons.push_back(button);
    Connect(button->GetId(), wxEVT_BUTTON, (wxObjectEventFunction)&LayoutPanel::OnNewModelTypeButtonClicked);
    return button;
}

LayoutPanel::~LayoutPanel()
{
    if (background != nullptr) {
        delete background;
    }
    TreeListViewModels->SetItemComparator(nullptr);
    TreeListViewModels->DeleteAllItems();
    delete m_imageList;
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
            xlights->MarkEffectsFileDirty(false);
            UpdatePreview();
        }
    }
    else if (name == "BkgTransparency") {
        if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
            xlights->SetPreviewBackgroundBrightness(previewBackgroundBrightness, 100 - event.GetValue().GetLong());
        }
        else {
            pGrp->SetBackgroundBrightness(previewBackgroundBrightness, 100 - event.GetValue().GetLong());
            modelPreview->SetBackgroundBrightness(previewBackgroundBrightness, 100 - event.GetValue().GetLong());
            xlights->MarkEffectsFileDirty(false);
            UpdatePreview();
        }
    }
    else if (name == "BkgSizeWidth") {
        xlights->SetPreviewSize(event.GetValue().GetLong(), modelPreview->GetVirtualCanvasHeight());
        xlights->UpdateModelsList();
    }
    else if (name == "BkgSizeHeight") {
        xlights->SetPreviewSize(modelPreview->GetVirtualCanvasWidth(), event.GetValue().GetLong());
        xlights->UpdateModelsList();
    }
    else if (name == "BoundingBox") {
        modelPreview->SetDisplay2DBoundingBox(event.GetValue().GetBool());
        xlights->SetDisplay2DBoundingBox(event.GetValue().GetBool());
        xlights->MarkEffectsFileDirty(false);
    } else if (name == "2DXZeroIsCenter") {
        modelPreview->SetDisplay2DCenter0(event.GetValue().GetBool());
        xlights->SetDisplay2DCenter0(event.GetValue().GetBool());
        xlights->MarkEffectsFileDirty(false);
    } else if (name == "BkgImage") {
        if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
            xlights->SetPreviewBackgroundImage(event.GetValue().GetString());
        }
        else {
            pGrp->SetBackgroundImage(event.GetValue().GetString());
            modelPreview->SetbackgroundImage(event.GetValue().GetString());
            xlights->MarkEffectsFileDirty(false);
            UpdatePreview();
        }
    }
    else if (name == "BkgFill") {
        if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
            xlights->SetPreviewBackgroundScaled(event.GetValue().GetBool());
        } else {
            pGrp->SetBackgroundScaled(wxAtoi(event.GetValue().GetString()) > 0);
            modelPreview->SetScaleBackgroundImage(wxAtoi(event.GetValue().GetString()) > 0);
            xlights->MarkEffectsFileDirty(false);
            UpdatePreview();
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
                        xlights->RecalcModels(true);
                        SelectBaseObject(safename);
                        CallAfter(&LayoutPanel::RefreshLayout); // refresh whole layout seems the most reliable at this point
                        xlights->MarkEffectsFileDirty(true);
                    }
                    xlights->RecalcModels(true);
                    SelectModel(safename);
                    CallAfter(&LayoutPanel::RefreshLayout); // refresh whole layout seems the most reliable at this point
                    xlights->MarkEffectsFileDirty(true);
                }
                else {
                    int i = selectedModel->OnPropertyGridChange(propertyEditor, event);
                    if (i & GRIDCHANGE_REFRESH_DISPLAY) {
                        xlights->UpdatePreview();
                    }
                    if (i & GRIDCHANGE_MARK_DIRTY) {
                        xlights->MarkEffectsFileDirty(true);
                    }
                    if (i & GRIDCHANGE_REBUILD_PROP_GRID) {
                        CallAfter(&LayoutPanel::resetPropertyGrid);
                    }
                    if (i & (GRIDCHANGE_REBUILD_MODEL_LIST | GRIDCHANGE_UPDATE_ALL_MODEL_LISTS))
                    {
                        // if these values were returned then some absolute start channels may have changed
                        xlights->RecalcModels(true);
                    }
                    if (i & GRIDCHANGE_REBUILD_MODEL_LIST) {
                        CallAfter(&LayoutPanel::refreshModelList);
                    }
                    if (i & GRIDCHANGE_UPDATE_ALL_MODEL_LISTS) {
                        // Preview assignment change so model may not exist in current preview anymore
                        CallAfter(&LayoutPanel::RefreshLayout);
                    }
                    if (i == 0) {
                        printf("Did not handle %s   %s\n",
                            (const char *)event.GetPropertyName().c_str(),
                            (const char *)event.GetValue().GetString().c_str());
                    }

                    if ("SubModels" == name) {
                        // if the sequencer is open we need to force a refresh to make sure submodel names are right
                        wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
                        wxPostEvent(xlights, eventForceRefresh);
                        CallAfter(&LayoutPanel::ReloadModelList);
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
void LayoutPanel::SetDisplay2DCenter0(bool bb) {
    modelPreview->SetDisplay2DCenter0(bb);
}


void LayoutPanel::OnPropertyGridChanging(wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
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
    }
    else {
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
                UpdatePreview();
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

void LayoutPanel::RefreshLayout()
{
    std::string selectedBaseObjectName = "";
    if (selectedBaseObject != nullptr) selectedBaseObjectName = selectedBaseObject->name;
    xlights->UpdateModelsList();
    if (selectedBaseObjectName != "") {
         SelectBaseObject(selectedBaseObjectName);
    }
    ShowPropGrid(true);
}

void LayoutPanel::RenderLayout()
{
    if (modelPreview == nullptr || !modelPreview->StartDrawing(mPointSize)) return;

    modelPreview->Render();
    if (m_creating_bound_rect)
    {
        if (!is_3d) {
            modelPreview->GetAccumulator().AddDottedLinesRect(m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y,
                ColorManager::instance()->GetColor(ColorManager::COLOR_LAYOUT_DASHES));
            modelPreview->GetAccumulator().Finish(GL_LINES);
        }
    }
    modelPreview->EndDrawing();
}

void LayoutPanel::UpdatePreview()
{
    SetDirtyHiLight(xlights->UnsavedRgbEffectsChanges);
    RenderLayout();
}

void LayoutPanel::resetPropertyGrid() {
    auto scroll = propertyEditor->GetScrollPos(wxVERTICAL);
    auto top = propertyEditor->GetItemAtY(0);
    clearPropGrid();
    if (selectedBaseObject != nullptr) {
        SetupPropGrid(selectedBaseObject);
        propertyEditor->SetScrollPos(wxVERTICAL, scroll - 1, true);
        wxScrollWinEvent e(wxEVT_SCROLLWIN_THUMBRELEASE);
        e.SetOrientation(wxVERTICAL);
        e.SetPosition(scroll);
        propertyEditor->HandleOnScroll(e);
    }
}

void LayoutPanel::clearPropGrid() {
    wxPGProperty *p = propertyEditor->GetPropertyByName("ModelAppearance");
    if (p != nullptr) {
        appearanceVisible = propertyEditor->IsPropertyExpanded(p);
    }
    p = propertyEditor->GetPropertyByName("ModelSize");
    if (p != nullptr) {
        sizeVisible = propertyEditor->IsPropertyExpanded(p);
    }
    p = propertyEditor->GetPropertyByName("ModelStringProperties");
    if (p != nullptr) {
        stringPropsVisible = propertyEditor->IsPropertyExpanded(p);
    }
    p = propertyEditor->GetPropertyByName("ModelControllerConnectionProperties");
    if (p != nullptr) {
        controllerConnectionVisible = propertyEditor->IsPropertyExpanded(p);
    }
    propertyEditor->Clear();
}

void LayoutPanel::refreshModelList() {

    TreeListViewModels->Freeze();

    for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
          item.IsOk();
          item = TreeListViewModels->GetNextItem(item) )
    {
        ModelTreeData *data = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
        Model *model = data != nullptr ? data->GetModel() : nullptr;

        if (model != nullptr ) {
            int end_channel = model->GetLastChannel()+1;
            wxString endStr = model->GetLastChannelInStartChannelFormat(xlights->GetOutputManager());
            if( model->GetDisplayAs() != "ModelGroup" ) {
                wxString cv = TreeListViewModels->GetItemText(item, Col_StartChan);
                wxString startStr = model->GetStartChannelInDisplayFormat(xlights->GetOutputManager());
                if (cv != startStr) {
                    data->startingChannel = model->GetNumberFromChannelString(model->ModelStartChannel);
                    if ((model->CouldComputeStartChannel || model->GetDisplayAs() == "SubModel") && model->IsValidStartChannelString())
                    {
                        TreeListViewModels->SetItemText(item, Col_StartChan, startStr);
                    }
                    else
                    {
                        TreeListViewModels->SetItemText(item, Col_StartChan, "*** " + startStr);
                    }
                }
                cv = TreeListViewModels->GetItemText(item, Col_EndChan);
                if (cv != endStr) {
                    data->endingChannel = end_channel;
                    TreeListViewModels->SetItemText(item, Col_EndChan, endStr);
                }
                cv = TreeListViewModels->GetItemText(item, Col_ControllerConnection);

                std::string cc = model->GetControllerConnectionRangeString();
                if (cv != cc)
                {
                    TreeListViewModels->SetItemText(item, Col_ControllerConnection, cc);
                }
            }
        }
    }
    TreeListViewModels->Thaw();
    TreeListViewModels->Refresh();
}

void LayoutPanel::RenameModelInTree(Model *model, const std::string new_name)
{
    for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
          item.IsOk();
          item = TreeListViewModels->GetNextItem(item) )
    {
        ModelTreeData *data = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
        if (data != nullptr && data->GetModel() == model) {
            TreeListViewModels->SetItemText(item, wxString(new_name.c_str()));
        }
    }
}

int LayoutPanel::GetModelTreeIcon(Model* model, bool open) {
    if( model->GetDisplayAs() == "ModelGroup" ) {
        return open ? Icon_FolderOpened : Icon_FolderClosed;
    } else {
        const std::string type = model->GetDisplayAs();
        if( type == "Arches" ) {
            return Icon_Arches;
        } else if( type == "Candy Canes" ) {
            return Icon_CandyCane;
        } else if( type == "Circle" ) {
            return Icon_Circle;
        } else if (type == "Channel Block") {
            return Icon_ChannelBlock;
        } else if( type == "Cube" ) {
            return Icon_Cube;
        } else if( type == "Custom" ) {
            return Icon_Custom;
        } else if( type == "DMX" ) {
            return Icon_Dmx;
        } else if( type == "Image" ) {
            return Icon_Image;
        } else if( type == "Icicles" ) {
            return Icon_Icicle;
        } else if( type == "Single Line" ) {
            return Icon_Line;
        } else if( type == "Matrix" ) {
            return Icon_Matrix;
        } else if( type == "Poly Line" ) {
            return Icon_Poly;
        } else if( type == "Sphere" ) {
            return Icon_Sphere;
        } else if( type == "Spinner" ) {
            return Icon_Spinner;
        } else if( type == "Star" ) {
            return Icon_Star;
        } else if( type == "SubModel" ) {
            return Icon_SubModel;
        } else if( type == "Tree" ) {
            return Icon_Tree;
        } else if( type == "Wreath" ) {
            return Icon_Wreath;
        } else if( type == "Window Frame" ) {
            return Icon_Window;
        } else {
            return Icon_File;
        }
    }
    return 0;
}

int LayoutPanel::AddModelToTree(Model *model, wxTreeListItem* parent, bool expanded, int nativeOrder, bool fullName) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    int width = 0;

    if (model == nullptr)
    {
        logger_base.crit("LayoutPanel::AddModelToTree model is null ... this is going to crash.");
    }

    //logger_base.debug("Adding model %s", (const char *)model->GetFullName().c_str());

    wxTreeListItem item = TreeListViewModels->AppendItem(*parent, fullName ? model->GetFullName() : model->name,
                                                         GetModelTreeIcon(model, false),
                                                         GetModelTreeIcon(model, true),
                                                         new ModelTreeData(model, nativeOrder));
    if( model->GetDisplayAs() != "ModelGroup" ) {
        wxString endStr = model->GetLastChannelInStartChannelFormat(xlights->GetOutputManager());
        wxString startStr = model->GetStartChannelInDisplayFormat(xlights->GetOutputManager());
        if (model->GetDisplayAs() == "SubModel" || (model->CouldComputeStartChannel && model->IsValidStartChannelString()))
        {
            TreeListViewModels->SetItemText(item, Col_StartChan, startStr);
        }
        else
        {
            TreeListViewModels->SetItemText(item, Col_StartChan, "*** " + startStr);
        }
        TreeListViewModels->SetItemText(item, Col_EndChan, endStr);

        std::string cc = model->GetControllerConnectionRangeString();
        TreeListViewModels->SetItemText(item, Col_ControllerConnection, cc);

        width = std::max(TreeListViewModels->WidthFor(TreeListViewModels->GetItemText(item, Col_StartChan)), TreeListViewModels->WidthFor(TreeListViewModels->GetItemText(item, Col_EndChan)));
    }

    for (int x = 0; x < model->GetNumSubModels(); x++) {
        AddModelToTree(model->GetSubModel(x), &item, false, x);
    }
    if( model->GetDisplayAs() == "ModelGroup" ) {
        ModelGroup *grp = (ModelGroup*)model;
        int i = 0;
        for (auto it = grp->ModelNames().begin(); it != grp->ModelNames().end(); ++it) {
            Model *m = xlights->AllModels[*it];

            if (m == nullptr)
            {
                logger_base.error("Model group %s thought it contained model. '%s' but it didnt. This would have crashed.", (const char *)grp->GetName().c_str(), (const char *)it->c_str());
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

    return width;
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

    TreeListViewModels->Freeze();
    unsigned sortcol;
    bool ascending;
    bool sorted = TreeListViewModels->GetSortColumn(&sortcol, &ascending);

    std::vector<Model *> dummy_models;

    // Update all the custom previews
    for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); ++it) {
        LayoutGroup* grp = (LayoutGroup*)(*it);
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

    if (full_refresh) {
        UnSelectAllModels();
        int width = 0;
        //turn off the colum width auto-resize.  Makes it REALLY slow to populate the tree
        TreeListViewModels->SetColumnWidth(0, 10);
        //turn off the sorting as that is ALSO really slow
        TreeListViewModels->SetItemComparator(nullptr);
        if (sorted) {
            //UnsetAsSortKey may be unimplemented on all  platforms so we'll set a
            //sort column to 0 which is faster due to straight string compare
            TreeListViewModels->GetDataView()->GetSortingColumn()->UnsetAsSortKey();
        }

        //delete all items will atempt to resort as each item is deleted, however, our Model pointers
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
        for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it) {
            Model *model = it->second;
            if (model->GetDisplayAs() == "ModelGroup") {
                if (currentLayoutGroup == "All Models" || model->GetLayoutGroup() == currentLayoutGroup
                    || (model->GetLayoutGroup() == "All Previews" && currentLayoutGroup != "Unassigned")) {
                    bool expand = (std::find(expanded.begin(), expanded.end(), model->GetName()) != expanded.end());
                    width = std::max(width, AddModelToTree(model, &root, expand, 0));
                }
            }
        }

        // add all the models
        for (auto it = models.begin(); it != models.end(); ++it) {
            Model *model = *it;
            if (model->GetDisplayAs() != "ModelGroup" && model->GetDisplayAs() != "SubModel") {
                bool expand = (std::find(expanded.begin(), expanded.end(), model->GetName()) != expanded.end());
                width = std::max(width, AddModelToTree(model, &root, expand, 0));
            }
        }

        // Only set the column sizes the very first time we load it
        if (_firstTreeLoad) {
            _firstTreeLoad = false;
            width = std::max(width, TreeListViewModels->WidthFor("Start Chan"));
            TreeListViewModels->SetColumnWidth(1, width);
            TreeListViewModels->SetColumnWidth(2, width);
            TreeListViewModels->SetColumnWidth(3, width);
        }

        TreeListViewModels->SetColumnWidth(0, wxCOL_WIDTH_AUTOSIZE);

        // we should have calculated a size, now turn off the auto-sizes as it's SLOW to update anything later
        int i = TreeListViewModels->GetColumnWidth(0);

#ifdef LINUX // Calculate size on linux as GTK doesn't size the window in time

        i = TreeListViewModels->GetSize().GetWidth() - (width * 2);
#endif
        if (i > 10) {
            TreeListViewModels->SetColumnWidth(0, i);
        }
        //turn the sorting back on
        TreeListViewModels->SetItemComparator(&comparator);
        if (sorted) {
            TreeListViewModels->SetSortColumn(sortcol, ascending);
            TreeListViewModels->GetDataView()->GetModel()->Resort();
        }
    }
    xlights->PreviewModels = models;
    UpdatePreview();

    TreeListViewModels->Thaw();
    TreeListViewModels->Refresh();
}

void LayoutPanel::UpdateModelsForPreview(const std::string &group, LayoutGroup* layout_grp, std::vector<Model *> &prev_models, bool filtering)
{
    std::set<std::string> modelsAdded;

    for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it) {
        Model *model = it->second;
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
    std::string selected_group_name = "";
    if (mSelectedGroup.IsOk() && filtering) {
        selected_group_name = TreeListViewModels->GetItemText(mSelectedGroup);
    }

    for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it) {
        Model *model = it->second;
        bool mark_selected = false;
        if (mSelectedGroup.IsOk() && filtering && (model->name == selected_group_name)) {
            mark_selected = true;
        }
        if (model->GetDisplayAs() == "ModelGroup") {
            ModelGroup *grp = (ModelGroup*)(model);
            if (group == "All Models" ||
                model->GetLayoutGroup() == group ||
                (model->GetLayoutGroup() == "All Previews" && group != "Unassigned")) {
                for (auto it2 = grp->ModelNames().begin(); it2 != grp->ModelNames().end(); ++it2) {
                    Model *m = xlights->AllModels[*it2];
                    if (m != nullptr) {
                        if (mark_selected) {
                            if (selectedBaseObject == nullptr)
                            {
                                SelectModel(m, false);
                            }
                            m->GroupSelected = true;
                            m->Highlighted = true;
                        }
                        if (m->DisplayAs == "SubModel") {
                            if (mark_selected) {
                                prev_models.push_back(m);
                            }
                        }
                        else if (m->DisplayAs == "ModelGroup") {
                            ModelGroup *mg = (ModelGroup*)m;
                            if (mark_selected) {
                                for (auto it3 = mg->Models().begin(); it3 != mg->Models().end(); ++it3) {
                                    if ((*it3)->DisplayAs != "ModelGroup") {
                                        if (selectedBaseObject == nullptr)
                                        {
                                            SelectModel((*it3), false);
                                        }
                                        (*it3)->GroupSelected = true;
                                        (*it3)->Highlighted = true;
                                        prev_models.push_back(*it3);
                                    }
                                    else
                                    {
                                        // need to process groups within groups ... safely
                                        for (auto itm = xlights->AllModels.begin(); itm != xlights->AllModels.end(); ++itm)
                                        {
                                            if (itm->second->GetDisplayAs() == "ModelGroup")
                                            {
                                                // ignore these
                                            }
                                            else
                                            {
                                                if (dynamic_cast<ModelGroup*>(*it3)->ContainsModel(itm->second))
                                                {
                                                    if (std::find(prev_models.begin(), prev_models.end(), itm->second) == prev_models.end())
                                                    {
                                                        if (modelsAdded.find(itm->first) == modelsAdded.end()) {
                                                            modelsAdded.insert(itm->first);
                                                        }
                                                        prev_models.push_back(itm->second);
                                                    }
                                                    itm->second->GroupSelected = true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (modelsAdded.find(*it2) == modelsAdded.end()) {
                            modelsAdded.insert(*it2);
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
    // get the first dimming curve
    ModelDimmingCurveDialog dlg(this);
    std::map<std::string, std::map<std::string, std::string>> dimmingInfo;
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->GroupSelected)
        {
            wxXmlNode *f = modelPreview->GetModels()[i]->GetModelXml()->GetChildren();
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
        }

        if (modelPreview->GetModels()[i]->GetModelXml()->GetAttribute("ModelBrightness", "-1") != "-1")
        {
            wxString b = modelPreview->GetModels()[i]->GetModelXml()->GetAttribute("ModelBrightness", "0");
            dimmingInfo["all"]["gamma"] = "1.0";
            dimmingInfo["all"]["brightness"] = b;
        }

        if (!dimmingInfo.empty()) {
            break;
        }
    }

    if (dimmingInfo.empty()) {
        dimmingInfo["all"]["gamma"] = "1.0";
        dimmingInfo["all"]["brightness"] = "0";
    }

    dlg.Init(dimmingInfo);
    if (dlg.ShowModal() == wxID_OK) {
        dimmingInfo.clear();
        dlg.Update(dimmingInfo);

        for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
        {
            if (modelPreview->GetModels()[i]->GroupSelected)
            {
                wxXmlNode *f1 = modelPreview->GetModels()[i]->GetModelXml()->GetChildren();
                while (f1 != nullptr) {
                    if ("dimmingCurve" == f1->GetName()) {
                        modelPreview->GetModels()[i]->GetModelXml()->RemoveChild(f1);
                        delete f1;
                        f1 = modelPreview->GetModels()[i]->GetModelXml()->GetChildren();
                    }
                    else {
                        f1 = f1->GetNext();
                    }
                }
                f1 = new wxXmlNode(wxXML_ELEMENT_NODE, "dimmingCurve");
                modelPreview->GetModels()[i]->GetModelXml()->AddChild(f1);
                for (auto it = dimmingInfo.begin(); it != dimmingInfo.end(); ++it) {
                    wxXmlNode *dc = new wxXmlNode(wxXML_ELEMENT_NODE, it->first);
                    f1->AddChild(dc);
                    for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
                        dc->AddAttribute(it2->first, it2->second);
                    }
                }
                xlights->MarkEffectsFileDirty(true);
            }
        }
    }
}

void LayoutPanel::BulkEditControllerConnection()
{
    // get the first controller connection
    wxXmlNode *cc = nullptr;
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
        if (modelPreview->GetModels()[i]->GroupSelected) {
            std::string port = modelPreview->GetModels()[i]->GetProtocol();
            if (port != "") {
                cc = modelPreview->GetModels()[i]->GetControllerConnection();
                break;
            }
        }
    }

    ControllerConnectionDialog dlg(this);
    dlg.Set(cc);

    if (dlg.ShowModal() == wxID_OK) {
        for (size_t i = 0; i < modelPreview->GetModels().size(); i++) {
            if (modelPreview->GetModels()[i]->GroupSelected) {
                dlg.Get(modelPreview->GetModels()[i]->GetControllerConnection());
            }
        }

        xlights->UpdateModelsList();
        xlights->MarkEffectsFileDirty(true);
        resetPropertyGrid();
    }
}

void LayoutPanel::BulkEditControllerPreview()
{
    // get the first preview
    std::string p = "";
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->GroupSelected)
        {
            p = modelPreview->GetModels()[i]->GetLayoutGroup();
            if (p != "") break;
        }
    }

    // remember the selected models
    wxString selected = "";
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->GroupSelected)
        {
            if (selected != "")
            {
                selected += ",";
            }
            selected += modelPreview->GetModels()[i]->GetName();
        }
    }

    Model* sm = dynamic_cast<Model*>(selectedBaseObject);

    wxArrayString choices = Model::GetLayoutGroups(xlights->AllModels);
    int sel = 0;
    int j = 0;
    for (auto it = choices.begin(); it != choices.end(); ++it)
    {
        if (*it == p) {
            sel = j;
            break;
        }
        j++;
    }
    wxSingleChoiceDialog dlg(this, "Preview", "Preview", choices);
    dlg.SetSelection(sel);

    if (dlg.ShowModal() == wxID_OK)
    {
        for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
        {
            if (modelPreview->GetModels()[i]->GroupSelected)
            {
                modelPreview->GetModels()[i]->SetLayoutGroup(dlg.GetStringSelection().ToStdString());
            }
        }

        xlights->MarkEffectsFileDirty(true);
        UpdateModelList(true);
        SelectModel(sm, true);
        resetPropertyGrid();

        // reselect all the models
        wxArrayString models = wxSplit(selected, ',');
        for (auto it = models.begin(); it != models.end(); ++it)
        {
            for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
            {
                if (modelPreview->GetModels()[i]->GetName() == it->ToStdString())
                {
                    modelPreview->GetModels()[i]->GroupSelected = true;
                }
            }
        }

        RenderLayout();
    }
}

void LayoutPanel::CreateModelGroupFromSelected()
{
    wxTextEntryDialog dlg(this, "Enter name for new group", "Enter name for new group");
    if (dlg.ShowModal() == wxID_OK) {
        wxString name = wxString(Model::SafeModelName(dlg.GetValue().ToStdString()));
        while (xlights->AllModels.GetModel(name.ToStdString()) != nullptr) {
            wxTextEntryDialog dlg2(this, "Model of name " + name + " already exists. Enter name for new group", "Enter name for new group");
            if (dlg2.ShowModal() == wxID_OK) {
                name = wxString(Model::SafeModelName(dlg2.GetValue().ToStdString()));
            }
            else {
                return;
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

        bool selectedModelAdded = selectedBaseObject == nullptr;
        wxString ModelsInGroup = "";
        for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
        {
            if (modelPreview->GetModels()[i]->GroupSelected)
            {
                if (modelPreview->GetModels()[i] == selectedBaseObject)
                {
                    selectedModelAdded = true;
                }
                if (ModelsInGroup != "")
                {
                    ModelsInGroup += ",";
                }
                ModelsInGroup += modelPreview->GetModels()[i]->GetName();
            }
        }
        if (!selectedModelAdded)
        {
            if (ModelsInGroup != "")
            {
                ModelsInGroup += ",";
            }
            ModelsInGroup += selectedBaseObject->GetName();
        }

        node->AddAttribute("models", ModelsInGroup);

        xlights->AllModels.AddModel(xlights->AllModels.CreateModel(node));
        xlights->UpdateModelsList();
        xlights->MarkEffectsFileDirty(true);
        model_grp_panel->UpdatePanel(name.ToStdString());
        ShowPropGrid(false);
        SelectBaseObject(name.ToStdString());
        wxArrayString models = wxSplit(ModelsInGroup, ',');
        for (auto it = models.begin(); it != models.end(); ++it)
        {
            for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
            {
                if (modelPreview->GetModels()[i]->GetName() == it->ToStdString())
                {
                    modelPreview->GetModels()[i]->GroupSelected = true;
                }
            }
        }
        RenderLayout();
    }
}

class xlImageProperty : public wxImageFileProperty {
public:
    xlImageProperty(const wxString& label,
                    const wxString& name,
                    const wxString& value,
                    const wxImage *img)
        : lastFileName(value), wxImageFileProperty(label, name, "") {

        SetValueFromString(value);
        if (img != nullptr) {
            m_pImage = new wxImage(*img);
        }
    }
    virtual ~xlImageProperty() {}

    virtual void OnSetValue() override {
        wxFileProperty::OnSetValue();
        wxFileName fn = GetFileName();
        if (fn != lastFileName) {
            lastFileName = fn;
            delete m_pImage;
            m_pImage = nullptr;
            if (fn.Exists()) {
                m_pImage = new wxImage(fn.GetFullPath());
            }
        }
    }

private:
    wxFileName lastFileName;
};

void LayoutPanel::UnSelectAllModels(bool addBkgProps)
{
    highlightedBaseObject = nullptr;
    selectedBaseObject = nullptr;
    selectionLatched = false;

    if( editing_models ) {
        auto models = modelPreview->GetModels();
        for (size_t i = 0; i < models.size(); i++)
        {
            Model* m = modelPreview->GetModels()[i];
            m->Selected = false;
            m->Highlighted = false;
            m->GroupSelected = false;
            m->SelectHandle(-1);
            m->GetBaseObjectScreenLocation().SetActiveHandle(-1);
        }
    } else {
        for (auto it = xlights->AllObjects.begin(); it != xlights->AllObjects.end(); ++it) {
            ViewObject *view_object = it->second;
            view_object->Selected = false;
            view_object->Highlighted = false;
            view_object->GroupSelected = false;
            view_object->SelectHandle(-1);
            view_object->GetBaseObjectScreenLocation().SetActiveHandle(-1);
        }
    }

    UpdatePreview();
    selectedBaseObject = nullptr;
    mSelectedGroup = nullptr;

    if (!updatingProperty && addBkgProps) {
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
            if (backgroundFile != "" && wxFileExists(backgroundFile) && wxIsReadable(backgroundFile)) {
                background = new wxImage(backgroundFile);
            }
        }
        wxPGProperty* p = propertyEditor->Append(new xlImageProperty("Background Image",
            "BkgImage",
            previewBackgroundFile,
            background));
        p->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif|All files (*.*)|*.*");
        propertyEditor->Append(new wxBoolProperty("Fill", "BkgFill", previewBackgroundScaled))->SetAttribute("UseCheckbox", 1);
        if (currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned") {
            wxPGProperty* prop = propertyEditor->Append(new wxUIntProperty("Width", "BkgSizeWidth", modelPreview->GetVirtualCanvasWidth()));
            prop->SetAttribute("Min", 0);
            prop->SetAttribute("Max", 4096);
            prop->SetEditor("SpinCtrl");
            prop = propertyEditor->Append(new wxUIntProperty("Height", "BkgSizeHeight", modelPreview->GetVirtualCanvasHeight()));
            prop->SetAttribute("Min", 0);
            prop->SetAttribute("Max", 4096);
            prop->SetEditor("SpinCtrl");
        }
        wxPGProperty* prop = propertyEditor->Append(new wxUIntProperty("Brightness", "BkgBrightness",  previewBackgroundBrightness));
        prop->SetAttribute("Min", 0);
        prop->SetAttribute("Max", 100);
        prop->SetEditor("SpinCtrl");

        prop = propertyEditor->Append(new wxUIntProperty("Transparency", "BkgTransparency",  100 - previewBackgroundAlpha));
        prop->SetAttribute("Min", 0);
        prop->SetAttribute("Max", 100);
        prop->SetEditor("SpinCtrl");

        prop = propertyEditor->Append(new wxBoolProperty("2D Bounding Box", "BoundingBox", xlights->GetDisplay2DBoundingBox()));
        prop->SetAttribute("UseCheckbox", true);

        prop = propertyEditor->Append(new wxBoolProperty("X0 Is Center", "2DXZeroIsCenter", xlights->GetDisplay2DCenter0()));
        prop->SetAttribute("UseCheckbox", true);

        propertyEditor->Thaw();
    }
}

void LayoutPanel::SetupPropGrid(BaseObject *base_object) {

    if (base_object == nullptr || propertyEditor == nullptr) return;

    propertyEditor->Freeze();
    clearPropGrid();

    if( editing_models ) {
        propertyEditor->Append(new wxStringProperty("Name", "ModelName", base_object->name));
    } else {
        propertyEditor->Append(new wxStringProperty("Name", "ObjectName", base_object->name));
    }

    base_object->AddProperties(propertyEditor);

    if (dynamic_cast<SubModel*>(base_object) == nullptr) {
        wxPGProperty *p2 = propertyEditor->Append(new wxPropertyCategory("Size/Location", "ModelSize"));

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
    }
    propertyEditor->Thaw();
}

void LayoutPanel::SelectBaseObject3D()
{
    if (is_3d) {
        // latch center handle immediately
        if (selectedBaseObject != nullptr) {
            if( editing_models ) {
                Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
                selectedModel->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
                selectedModel->GetBaseObjectScreenLocation().SetActiveAxis(-1);
            } else {
                ViewObject* selectedViewObject = dynamic_cast<ViewObject*>(selectedBaseObject);
                selectedViewObject->GetObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
                selectedViewObject->GetObjectScreenLocation().SetActiveAxis(-1);
            }
            highlightedBaseObject = selectedBaseObject;
            selectionLatched = true;
        }
    }
}
void LayoutPanel::SelectBaseObject(const std::string & name, bool highlight_tree)
{
    if( editing_models ) {
        Model *m = xlights->AllModels[name];
        if (m == nullptr)
        {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.warn("LayoutPanel:SelectBaseObject Unable to select model '%s'.", (const char *)name.c_str());
        }
        SelectModel(m, highlight_tree);
    } else {
        ViewObject *v = xlights->AllObjects[name];
        if (v == nullptr)
        {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.warn("LayoutPanel:SelectBaseObject Unable to select object '%s'.", (const char *)name.c_str());
        }
        SelectViewObject(v, highlight_tree);
    }
}

void LayoutPanel::SelectBaseObject(BaseObject *obj, bool highlight_tree)
{
    if( editing_models ) {
        SelectModel(dynamic_cast<Model*>(obj), highlight_tree);
    } else {
        SelectViewObject(dynamic_cast<ViewObject*>(obj), highlight_tree);
    }
}

void LayoutPanel::SelectModel(const std::string & name, bool highlight_tree)
{
    Model *m = xlights->AllModels[name];
    if (m == nullptr)
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.warn("LayoutPanel:SelectModel Unable to select model '%s'.", (const char *)name.c_str());
    }
    SelectModel(m, highlight_tree);
}

void LayoutPanel::SelectModel(Model *m, bool highlight_tree) {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // TODO need to strip out extra logging once I know for sure what is going on
    if (modelPreview == nullptr) logger_base.crit("LayoutPanel::SelectModel modelPreview is nullptr ... this is going to crash.");

    modelPreview->SetFocus();
    int foundStart = 0;
    int foundEnd = 0;

    if (m != nullptr) {

        if (m->GetDisplayAs() == "SubModel")
        {
            SubModel *subModel = dynamic_cast<SubModel*>(m);
            if (subModel != nullptr) {
                // this is the only thing I can see here that could crash
                if (subModel->GetParent() == nullptr) logger_base.crit("LayoutPanel::SelectModel subModel->GetParent is nullptr ... this is going to crash.");
                subModel->GetParent()->Selected = true;
            }
            else {
                m->Selected = true;
            }
        } else {
            m->Selected = true;
        }

        if( highlight_tree ) {
            for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
                  item.IsOk();
                  item = TreeListViewModels->GetNextSibling(item) )
            {
                if (TreeListViewModels->GetItemData(item) != nullptr)
                {
                    ModelTreeData *mitem = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
                    if (mitem != nullptr && mitem->GetModel() == m) {
                        TreeListViewModels->Select(item);
                        TreeListViewModels->EnsureVisible(item);
                        break;
                    }
                }
            }
        }
        if (CheckBoxOverlap->GetValue() == true) {
            foundStart = m->GetNumberFromChannelString(m->ModelStartChannel);
            foundEnd = m->GetLastChannel();
        }
        SetupPropGrid(m);
    } else {
        propertyEditor->Freeze();
        clearPropGrid();
        propertyEditor->Thaw();
    }

    selectedBaseObject = m;
    selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
    selectionLatched = true;

    if (CheckBoxOverlap->GetValue()) {
        for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
              item.IsOk();
              item = TreeListViewModels->GetNextSibling(item) )
        {
            if (TreeListViewModels->GetItemData(item) != nullptr)
            {
                ModelTreeData *data = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
                Model *mm = data != nullptr ? data->GetModel() : nullptr;
                if (mm != nullptr && mm != selectedBaseObject) {
                    int startChan = mm->GetNumberFromChannelString(mm->ModelStartChannel);
                    int endChan = mm->GetLastChannel();
                    if ((startChan >= foundStart) && (endChan <= foundEnd)) {
                        mm->Overlapping = true;
                    }
                    else if ((startChan >= foundStart) && (startChan <= foundEnd)) {
                        mm->Overlapping = true;
                    }
                    else if ((endChan >= foundStart) && (endChan <= foundEnd)) {
                        mm->Overlapping = true;
                    }
                    else {
                        mm->Overlapping = false;
                    }
                }
            }
        }
    }
    SelectBaseObject3D();
    UpdatePreview();
}

void LayoutPanel::SelectViewObject(ViewObject *v, bool highlight_tree) {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // TODO need to strip out extra logging once I know for sure what is going on
    if (modelPreview == nullptr) logger_base.crit("LayoutPanel::SelectViewObject modelPreview is nullptr ... this is going to crash.");

    modelPreview->SetFocus();

    if (v != nullptr) {
        v->Selected = true;

        if( highlight_tree ) {
            objects_panel->HighlightObject(v);
        }
        SetupPropGrid(v);
    } else {
        propertyEditor->Freeze();
        clearPropGrid();
        propertyEditor->Thaw();
    }

    selectedBaseObject = v;
    if (v) {
        selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
        selectionLatched = true;
    }
    SelectBaseObject3D();

    UpdatePreview();
}

void LayoutPanel::OnCheckBoxOverlapClick(wxCommandEvent& event)
{
    if (CheckBoxOverlap->GetValue() == false) {
        for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
              item.IsOk();
              item = TreeListViewModels->GetNextSibling(item) )
        {
            ModelTreeData *data = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
            Model *model = data != nullptr ? data->GetModel() : nullptr;

            if( model != nullptr ) {
                if( model->GetDisplayAs() != "ModelGroup" ) {
                    model->Overlapping = false;
                }
            }
        }
    }
}

void LayoutPanel::SaveEffects()
{
    // update xml with offsets and scale
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
    {
        modelPreview->GetModels()[i]->UpdateXmlWithScale();
    }
    for (auto it = xlights->AllObjects.begin(); it != xlights->AllObjects.end(); ++it) {
        ViewObject *view_object = it->second;
        view_object->UpdateXmlWithScale();
    }
    xlights->SaveEffectsFile();
    xlights->SetStatusText(_("Preview layout saved"));
    SetDirtyHiLight(false);
}

void LayoutPanel::OnButtonSavePreviewClick(wxCommandEvent& event)
{
    SaveEffects();
}

int LayoutPanel::ModelListComparator::SortElementsFunction(wxTreeListCtrl *treelist, wxTreeListItem item1, wxTreeListItem item2, unsigned sortColumn)
{
    unsigned col;
    bool ascending;
    treelist->GetSortColumn(&col, &ascending);

    ModelTreeData *data1 = dynamic_cast<ModelTreeData*>(treelist->GetItemData(item1));
    ModelTreeData *data2 = dynamic_cast<ModelTreeData*>(treelist->GetItemData(item2));

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

    if (sortColumn == 1) {
        int ia = data1->startingChannel;
        int ib = data2->startingChannel;
        if (ia > ib)
            return 1;
        if (ia < ib)
            return -1;
        return NumberAwareStringCompare(a->name, b->name);
    }
    else if (sortColumn == 2) {
        int ia = data1->endingChannel;
        int ib = data2->endingChannel;
        if (ia > ib)
            return 1;
        if (ia < ib)
    return -1;
        return NumberAwareStringCompare(a->name, b->name);
    }

    // Dont sort things with parents
    auto parent1 = treelist->GetItemParent(item1);
    auto parent2 = treelist->GetItemParent(item2);
    auto root = treelist->GetRootItem();
    if ((parent1 != root || parent2 != root) && parent1 == parent2)
    {
        int ia = data1->nativeOrder;
        int ib = data2->nativeOrder;
        if (ia > ib)
        {
            if (ascending)
            {
                return 1;
            }
            return -1;
        }
        if (ia < ib)
        {
            if (ascending)
            {
                return -1;
            }
            return 1;
        }
    }

    return NumberAwareStringCompare(a->name, b->name);
}

int LayoutPanel::ModelListComparator::Compare(wxTreeListCtrl *treelist, unsigned column, wxTreeListItem first, wxTreeListItem second)
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

int LayoutPanel::FindModelsClicked(int x, int y, std::vector<int> &found)
{
    glm::vec3 ray_origin;
    glm::vec3 ray_direction;
    GetMouseLocation(x, y, ray_origin, ray_direction);

    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->HitTest(modelPreview, ray_origin, ray_direction))
        {
            found.push_back(i);
        }
    }
    return found.size();
}

bool LayoutPanel::SelectSingleModel(int x, int y)
{
    std::vector<int> found;
    int modelCount = FindModelsClicked(x, y, found);
    if (modelCount == 0)
    {
        TreeListViewModels->UnselectAll();
        return false;
    }
    else if (modelCount == 1)
    {
        SelectModel(modelPreview->GetModels()[found[0]]);
        mHitTestNextSelectModelIndex = 0;
        return true;
    }
    else if (modelCount > 1)
    {
        for (int i = 0; i < modelCount; i++)
        {
            if (mHitTestNextSelectModelIndex == i)
            {
                SelectModel(modelPreview->GetModels()[found[i]]);
                mHitTestNextSelectModelIndex += 1;
                mHitTestNextSelectModelIndex %= modelCount;
                return true;
            }
        }
    }
    return false;
}

void LayoutPanel::SelectAllInBoundingRect()
{
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->IsContained(modelPreview, m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y))
        {
            // if we dont have a selected model make the first one we find the selected model so alignment etc works
            if (selectedBaseObject == nullptr)
            {
                SelectBaseObject(modelPreview->GetModels()[i]->GetName(), false);
            }
            modelPreview->GetModels()[i]->GroupSelected = true;
        }
    }
}

void LayoutPanel::HighlightAllInBoundingRect()
{
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->IsContained(modelPreview, m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y))
        {
            modelPreview->GetModels()[i]->Highlighted = true;
        }
        else if(!modelPreview->GetModels()[i]->Selected &&
               !modelPreview->GetModels()[i]->GroupSelected){
            modelPreview->GetModels()[i]->Highlighted = false;
        }
    }
}

bool LayoutPanel::SelectMultipleModels(int x,int y)
{
    std::vector<int> found;
    int modelCount = FindModelsClicked(x, y, found);
    if (modelCount == 0)
    {
        return false;
    }
    else if(modelCount>0)
    {
        propertyEditor->Freeze();
        clearPropGrid();
        propertyEditor->Thaw();
        if(modelPreview->GetModels()[found[0]]->Selected)
        {
            modelPreview->GetModels()[found[0]]->Selected = false;
            modelPreview->GetModels()[found[0]]->Highlighted = false;
            modelPreview->GetModels()[found[0]]->GroupSelected = false;
            modelPreview->GetModels()[found[0]]->SelectHandle(-1);
            modelPreview->GetModels()[found[0]]->GetBaseObjectScreenLocation().SetActiveHandle(-1);
        }
        else if (modelPreview->GetModels()[found[0]]->GroupSelected)
        {
            SetSelectedModelToGroupSelected();
            modelPreview->GetModels()[found[0]]->Selected = true;
            modelPreview->GetModels()[found[0]]->Highlighted = true;
            SelectModel(modelPreview->GetModels()[found[0]]);
            modelPreview->GetModels()[found[0]]->SelectHandle(-1);
            modelPreview->GetModels()[found[0]]->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
        }
        else
        {
            modelPreview->GetModels()[found[0]]->Highlighted = true;
            if( ModelsSelectedCount() == 0 ) {
                modelPreview->GetModels()[found[0]]->Selected = true;
                SelectModel(modelPreview->GetModels()[found[0]]);
                modelPreview->GetModels()[found[0]]->SelectHandle(-1);
                modelPreview->GetModels()[found[0]]->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
            } else {
                modelPreview->GetModels()[found[0]]->GroupSelected = true;
            }
        }
        UpdatePreview();
        return true;
    }
    return false;
}

void LayoutPanel::SetSelectedModelToGroupSelected()
{
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->Selected)
        {
            modelPreview->GetModels()[i]->Selected = false;
            modelPreview->GetModels()[i]->GroupSelected = true;
        }
    }
}

void LayoutPanel::OnPreviewLeftDClick(wxMouseEvent& event)
{
    UnSelectAllModels();
    m_mouse_down = false;
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
                selectedBaseObject->GetBaseObjectScreenLocation().CheckIfOverHandles3D(ray_origin, ray_direction, m_over_handle);
            }
            if (m_over_handle != -1) {
                if ((m_over_handle & 0x2000) > 0) {
                    // an axis was selected
                    if (selectedBaseObject != nullptr) {
                        int active_handle = selectedBaseObject->GetBaseObjectScreenLocation().GetActiveHandle();
                        selectedBaseObject->GetBaseObjectScreenLocation().SetActiveAxis(m_over_handle & 0xff);
                        selectedBaseObject->GetBaseObjectScreenLocation().MouseOverHandle(-1);
                        bool z_scale = selectedBaseObject->GetBaseObjectScreenLocation().GetSupportsZScaling();
                        // this is designed to pretend the control and shift keys are down when creating models to
                        // make them scale from the desired handle depending on model type
                        selectedBaseObject->MoveHandle3D(modelPreview, active_handle, event.ShiftDown() | creating_model, event.ControlDown() | (creating_model & z_scale), event.GetX(), event.GetY(), true, z_scale);
                        UpdatePreview();
                        m_moving_handle = true;
                        m_mouse_down = true;
                        last_worldpos = selectedBaseObject->GetBaseObjectScreenLocation().GetWorldPosition();
                    }
                }
                else if ((m_over_handle & 0x10000) > 0) {
                    // a segment was selected
                    if (selectedBaseObject != nullptr) {
                        selectedBaseObject->GetBaseObjectScreenLocation().SelectSegment(m_over_handle & 0xFFF);
                        UpdatePreview();
                    }
                }
                else {
                    if (selectedBaseObject->GetBaseObjectScreenLocation().GetActiveHandle() == m_over_handle) {
                        selectedBaseObject->GetBaseObjectScreenLocation().AdvanceAxisTool();
                    }
                    selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(m_over_handle);
                    UpdatePreview();
                }
            }
            else {
                m_mouse_down = true;
            }
        }
        else {
            SelectBaseObject(highlightedBaseObject);
            selectionLatched = true;
            // latch center handle immediately
            selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
            UpdatePreview();
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
        m_moving_handle = true;
        m_creating_bound_rect = false;
        const std::string& model_type = selectedButton->GetModelType();
        newModel = CreateNewModel(model_type);
        newModel->SetLayoutGroup(currentLayoutGroup);

        if (newModel != nullptr) {
            if (model_type == "Poly Line") {
                m_polyline_active = true;
            }
            UnSelectAllModels();
            newModel->Selected = true;
            newModel->GetBaseObjectScreenLocation().SetActiveHandle(newModel->GetBaseObjectScreenLocation().GetDefaultHandle());
            newModel->GetBaseObjectScreenLocation().SetAxisTool(newModel->GetBaseObjectScreenLocation().GetDefaultTool());
            selectionLatched = true;
            highlightedBaseObject = newModel;
            selectedBaseObject = newModel;
            creating_model = true;
            if (wi > 0 && ht > 0)
            {
                modelPreview->SetCursor(newModel->InitializeLocation(m_over_handle, event.GetX(), event.GetY(), modelPreview));
                newModel->UpdateXmlWithScale();
            }
            bool z_scale = selectedBaseObject->GetBaseObjectScreenLocation().GetSupportsZScaling();
            // this is designed to pretend the control and shift keys are down when creating models to
            // make them scale from the desired handle depending on model type
            selectedBaseObject->MoveHandle3D(modelPreview, selectedBaseObject->GetBaseObjectScreenLocation().GetDefaultHandle(), event.ShiftDown() | creating_model, event.ControlDown() | (creating_model & z_scale), event.GetX(), event.GetY(), true, z_scale);
            lastModelName = newModel->name;
            modelPreview->SetAdditionalModel(newModel);
        }
    }

    ShowPropGrid(true);
    modelPreview->SetFocus();

    if (event.ControlDown())
    {
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
        // if control key is down check to see if we are highlighting another model for group selection
        if (event.ControlDown()) {
            int which_model = -1;
            float distance = 1000000000.0f;
            float intersection_distance = 1000000000.0f;
            for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
            {
                if (modelPreview->GetModels()[i]->GetBaseObjectScreenLocation().HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                    if (intersection_distance < distance) {
                        distance = intersection_distance;
                        which_model = i;
                    }
                }
            }
            if (which_model != -1)
            {
                if (modelPreview->GetModels()[which_model]->Highlighted) {
                    if (!modelPreview->GetModels()[which_model]->GroupSelected &&
                        !modelPreview->GetModels()[which_model]->Selected) {
                        modelPreview->GetModels()[which_model]->GroupSelected = true;
                    }
                    else if (modelPreview->GetModels()[which_model]->GroupSelected) {
                        modelPreview->GetModels()[which_model]->GroupSelected = false;
                        modelPreview->GetModels()[which_model]->Selected = true;
                        if (selectedBaseObject != nullptr) {
                            selectedBaseObject->GroupSelected = true;
                            selectedBaseObject->Selected = false;
                            selectedBaseObject->SelectHandle(-1);
                            selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(-1);
                        }
                        selectedBaseObject = modelPreview->GetModels()[which_model];
                        highlightedBaseObject = selectedBaseObject;
                        selectedBaseObject->SelectHandle(-1);
                        selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
                    }
                    else if (modelPreview->GetModels()[which_model]->Selected) {
                        modelPreview->GetModels()[which_model]->Selected = false;
                        modelPreview->GetModels()[which_model]->Highlighted = false;
                        modelPreview->GetModels()[which_model]->SelectHandle(-1);
                        modelPreview->GetModels()[which_model]->GetBaseObjectScreenLocation().SetActiveHandle(-1);
                        selectedBaseObject = nullptr;
                        // select first model we find
                        for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
                        {
                            if (modelPreview->GetModels()[i]->GroupSelected) {
                                selectedBaseObject = modelPreview->GetModels()[i];
                                selectedBaseObject->GroupSelected = false;
                                selectedBaseObject->Selected = true;
                                selectedBaseObject->SelectHandle(-1);
                                selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
                                break;
                            }
                        }
                        highlightedBaseObject = selectedBaseObject;
                    }
                    UpdatePreview();
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
        Model *m = newModel;
        m->AddHandle(modelPreview, event.GetX(), event.GetY());
        m->UpdateXmlWithScale();
        m->InitModel();
        xlights->MarkEffectsFileDirty(true);
        UpdatePreview();
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
    else if (m_over_handle != -1)
    {
        if ((m_over_handle & 0x10000) > 0) {
            // a segment was selected
            if (selectedBaseObject != nullptr) {
                selectedBaseObject->GetBaseObjectScreenLocation().SelectSegment(m_over_handle & 0xFFF);
                modelPreview->SetCursor(wxCURSOR_DEFAULT);
                UpdatePreview();
            }
        }
        else {
            m_moving_handle = true;
            if (selectedBaseObject != nullptr) {
                selectedBaseObject->SelectHandle(m_over_handle);
                UpdatePreview();
            }
        }
    }
    else if (selectedButton != nullptr)
    {
        //create a new model
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
        m_moving_handle = true;
        m_creating_bound_rect = false;
        const std::string& model_type = selectedButton->GetModelType();
        newModel = CreateNewModel(model_type);

        if (newModel != nullptr) {
            newModel->SetLayoutGroup(currentLayoutGroup);
            if (model_type == "Poly Line") {
                m_polyline_active = true;
            }
            UnSelectAllModels();
            newModel->Selected = true;
            modelPreview->SetCursor(newModel->InitializeLocation(m_over_handle, event.GetX(), event.GetY(), modelPreview));
            newModel->UpdateXmlWithScale();
            lastModelName = newModel->name;
            modelPreview->SetAdditionalModel(newModel);
        }
    }
    else
    {
        m_moving_handle = false;
        m_creating_bound_rect = false;

        if (!event.wxKeyboardState::ControlDown())
        {
            UnSelectAllModels();
        }

        if (SelectSingleModel(event.GetX(), event.GetY()))
        {
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
        return;
    }

    if (is_3d && m_mouse_down) {
        if (selectedBaseObject != nullptr) {
            selectedBaseObject->GetBaseObjectScreenLocation().SetActiveAxis(-1);
            UpdatePreview();
        }
        modelPreview->SetCameraView(0, 0, true);
    }

    m_mouse_down = false;
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
        SelectAllInBoundingRect();
        m_creating_bound_rect = false;
        UpdatePreview();
    }
    FinalizeModel();
}

void LayoutPanel::FinalizeModel()
{
    if( m_polyline_active && m_over_handle > 1 ) {
        Model *m = newModel;
        if (m != nullptr)
        {
            m->DeleteHandle(m_over_handle);
        }
    }
    m_moving_handle = false;
    m_dragging = false;
    m_polyline_active = false;
    creating_model = false;
    m_over_handle = NO_HANDLE;

    if (newModel != nullptr) {
        if (selectedButton != nullptr && (selectedButton->GetModelType() == "Import Custom" || selectedButton->GetModelType() == "Download"))
        {
            float min_x = (float)(newModel->GetBaseObjectScreenLocation().GetLeft());
            float max_x = (float)(newModel->GetBaseObjectScreenLocation().GetRight());
            float min_y = (float)(newModel->GetBaseObjectScreenLocation().GetBottom());
            float max_y = (float)(newModel->GetBaseObjectScreenLocation().GetTop());
            bool cancelled = false;

            wxProgressDialog prog("Model download", "Downloading models ...", 100, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
            if (selectedButton->GetModelType() == "Download")
            {
                prog.Show();
            }
            newModel = Model::GetXlightsModel(newModel, _lastXlightsModel, xlights, cancelled, selectedButton->GetModelType() == "Download", &prog, 0, 99);
            if (cancelled || newModel == nullptr) {
                newModel = nullptr;
                modelPreview->SetCursor(wxCURSOR_DEFAULT);
                selectedButton->SetState(0);
                selectedButton = nullptr;
                xlights->UpdateModelsList();
                UpdatePreview();
                return;
            }
            newModel->ImportXlightsModel(_lastXlightsModel, xlights, min_x, max_x, min_y, max_y);
            if (selectedButton->GetState() == 1)
            {
                _lastXlightsModel = "";
            }
        }
        CreateUndoPoint("All", "", "");
        newModel->UpdateXmlWithScale();
        xlights->AllModels.AddModel(newModel);

        newModel->SetLayoutGroup(currentLayoutGroup == "All Models" ? "Default" : currentLayoutGroup);

        modelPreview->SetCursor(wxCURSOR_DEFAULT);
        modelPreview->SetAdditionalModel(nullptr);
        if (selectedButton != nullptr && selectedButton->GetState() == 1) {
            std::string name = newModel->name;
            newModel = nullptr;
            if (selectedButton != nullptr) {
                selectedButton->SetState(0);
                selectedButton = nullptr;
            }
            selectedBaseObject = nullptr;
            xlights->UpdateModelsList();
            SelectBaseObject(name);
            SelectBaseObject3D();
        } else {
            newModel = nullptr;
            xlights->UpdateModelsList();
        }
        UpdatePreview();
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
void LayoutPanel::OnPreviewRotateGesture(wxRotateGestureEvent& event) {
    if (selectedBaseObject != nullptr) {
        //rotate model
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

            int axis = 2;  //default is around z axis
            if (wxGetKeyState(WXK_SHIFT)) {
                axis = 0;
            } else if (wxGetKeyState(WXK_CONTROL)) {
                axis = 1;
            }
            if (selectedBaseObject->Rotate(axis, delta)) {
                SetupPropGrid(selectedBaseObject);
                xlights->MarkEffectsFileDirty(true);
                UpdatePreview();
            }
        } else {
            CreateUndoPoint(editing_models ? "SingleModel" : "SingleObject", selectedBaseObject->name, "Zoom");
        }
    }
    m_last_mouse_x = (event.GetRotationAngle() * 1000);
}
void LayoutPanel::OnPreviewZoomGesture(wxZoomGestureEvent& event) {
    float delta = (m_last_mouse_x - (event.GetZoomFactor() * 1000)) / 1000.0;
    if (selectedBaseObject != nullptr) {
        if (!event.IsGestureStart()) {
            //resize model
            if (selectedBaseObject->Scale(1.0f - delta)) {
                SetupPropGrid(selectedBaseObject);
                xlights->MarkEffectsFileDirty(true);
                UpdatePreview();
            }
        } else {
            CreateUndoPoint(editing_models ? "SingleModel" : "SingleObject", selectedBaseObject->name, "Zoom");
        }
    }  else {
        modelPreview->SetZoomDelta(delta > 0.0 ? 0.1f : -0.1f);
        UpdatePreview();
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
    bool fromTrackPad = IsMouseEventFromTouchpad();
    if (is_3d) {
        if (!fromTrackPad || event.ControlDown()) {
            modelPreview->SetZoomDelta(event.GetWheelRotation() > 0 ? -0.1f : 0.1f);
        } else {
            float delta_x = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? 0 : -event.GetWheelRotation();
            float delta_y = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? -event.GetWheelRotation() : 0;
            if (event.ShiftDown()) {
                modelPreview->SetPan(delta_x, delta_y, 0.0f);
            } else {
                modelPreview->SetCameraView(delta_x, delta_y, false);
                modelPreview->SetCameraView(0, 0, true);
            }
        }
    } else {
        if (!fromTrackPad || event.ControlDown()) {
            modelPreview->SetZoomDelta(event.GetWheelRotation() > 0 ? -0.1f : 0.1f);
        } else {
            float new_x = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? 0 : -event.GetWheelRotation();
            float new_y = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? -event.GetWheelRotation() : 0;

            // account for grid rotation
            float angle = glm::radians(modelPreview->GetCameraRotationY());
            float delta_x = new_x * std::cos(angle) - new_y * std::sin(angle);
            float delta_y = new_y * std::cos(angle) + new_x * std::sin(angle);
            delta_x *= modelPreview->GetZoom() * 2.0f;
            delta_y *= modelPreview->GetZoom() * 2.0f;
            modelPreview->SetPan(delta_x, delta_y, 0.0f);
            m_previous_mouse_x = event.GetX();
            m_previous_mouse_y = event.GetY();
        }
    }
    UpdatePreview();
}

void LayoutPanel::OnPreviewMouseMove3D(wxMouseEvent& event)
{
    if (m_creating_bound_rect)
    {
        m_bound_end_x = event.GetX();
        m_bound_end_y = event.GetY();
        HighlightAllInBoundingRect();
        UpdatePreview();
        return;
    }
    else if (m_wheel_down)
    {
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
        UpdatePreview();
    }
    else if (m_mouse_down) {
        if (m_moving_handle) {
            if (selectedBaseObject != nullptr) {
                int active_handle = selectedBaseObject->GetBaseObjectScreenLocation().GetActiveHandle();
                int selectedModelCnt = ModelsSelectedCount();
                if (selectedBaseObject != newModel) {
                    CreateUndoPoint(editing_models ? "SingleModel" : "SingleObject", selectedBaseObject->name, std::to_string(active_handle));
                }
                bool z_scale = selectedBaseObject->GetBaseObjectScreenLocation().GetSupportsZScaling();
                // this is designed to pretend the control and shift keys are down when creating models to
                // make them scale from the desired handle depending on model type
                selectedBaseObject->MoveHandle3D(modelPreview, active_handle, event.ShiftDown() | creating_model, event.ControlDown() | (creating_model & z_scale), event.GetX(), event.GetY(), false, z_scale);
                SetupPropGrid(selectedBaseObject);
                xlights->MarkEffectsFileDirty(true);
                if (selectedModelCnt > 1) {
                    glm::vec3 new_worldpos = selectedBaseObject->GetBaseObjectScreenLocation().GetWorldPosition();
                    new_worldpos = new_worldpos - last_worldpos;
                    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
                    {
                        if (modelPreview->GetModels()[i]->GroupSelected || modelPreview->GetModels()[i]->Selected) {
                            if (modelPreview->GetModels()[i] != selectedBaseObject) {
                                modelPreview->GetModels()[i]->AddOffset(new_worldpos.x, new_worldpos.y, new_worldpos.z);
                            }
                        }
                    }
                    last_worldpos = selectedBaseObject->GetBaseObjectScreenLocation().GetWorldPosition();
                }
                UpdatePreview();
            }
        }
        else {
            int delta_x = event.GetPosition().x - m_last_mouse_x;
            int delta_y = event.GetPosition().y - m_last_mouse_y;
            modelPreview->SetCameraView(delta_x, delta_y, false);
            UpdatePreview();
        }
    }
    else {
        if (!selectionLatched) {
            glm::vec3 ray_origin;
            glm::vec3 ray_direction;
            GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
            BaseObject* which_object = nullptr;
            float distance = 1000000000.0f;
            float intersection_distance = 1000000000.0f;
            if( editing_models ) {
                for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
                {
                    if (modelPreview->GetModels()[i]->GetBaseObjectScreenLocation().HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                        if (intersection_distance < distance) {
                            distance = intersection_distance;
                            which_object = modelPreview->GetModels()[i];
                        }
                    }
                }
            } else {
                for (auto it = xlights->AllObjects.begin(); it != xlights->AllObjects.end(); ++it) {
                    ViewObject *view_object = it->second;
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
                if (highlightedBaseObject != nullptr) {
                    highlightedBaseObject->Highlighted = false;
                    highlightedBaseObject = nullptr;
                    UpdatePreview();
                }
            }
            else
            {
                if (which_object != last_selection) {
                    UnSelectAllModels();
                    highlightedBaseObject = which_object;
                    highlightedBaseObject->Highlighted = true;
                    UpdatePreview();
                }
            }
            last_selection = which_object;
        }
        if (m_moving_handle)
        {
            BaseObject *obj = newModel;
            if (obj == nullptr) {
                obj = selectedBaseObject;
                if (obj == nullptr) return;
            }
            int active_handle = obj->GetBaseObjectScreenLocation().GetActiveHandle();
            if (obj != newModel) {
                CreateUndoPoint(editing_models ? "SingleModel" : "SingleObject", obj->name, std::to_string(active_handle));
            }
            bool z_scale = obj->GetBaseObjectScreenLocation().GetSupportsZScaling();
            // this is designed to pretend the control and shift keys are down when creating models to
            // make them scale from the desired handle depending on model type
            obj->MoveHandle3D(modelPreview, active_handle, event.ShiftDown() | creating_model, event.ControlDown() | (creating_model & z_scale), event.GetX(), event.GetY(), false, z_scale);
            SetupPropGrid(obj);
            xlights->MarkEffectsFileDirty(true);
            UpdatePreview();
        }
        else {
            if (selectedBaseObject != nullptr) {
                glm::vec3 ray_origin;
                glm::vec3 ray_direction;
                GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
                // check for mouse over handle and if so highlight it
                modelPreview->SetCursor(selectedBaseObject->GetBaseObjectScreenLocation().CheckIfOverHandles3D(ray_origin, ray_direction, m_over_handle));
                if (m_over_handle != over_handle) {
                    selectedBaseObject->GetBaseObjectScreenLocation().MouseOverHandle(m_over_handle);
                    over_handle = m_over_handle;
                    UpdatePreview();
                }
                else if( event.ControlDown() ) {
                    // For now require control to be active before we start highlighting other models while a model is selected otherwise
                    // it gets hard to work on selected model with everything else highlighting.
                    // See if hovering over a model and if so highlight it or remove highlight as you leave it if it wasn't selected.
                    BaseObject* which_object = nullptr;
                    float distance = 1000000000.0f;
                    float intersection_distance = 1000000000.0f;
                    if( editing_models ) {
                        for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
                        {
                            if (modelPreview->GetModels()[i] != selectedBaseObject) {
                                if (modelPreview->GetModels()[i]->GetBaseObjectScreenLocation().HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                                    if (intersection_distance < distance) {
                                        distance = intersection_distance;
                                        which_object = modelPreview->GetModels()[i];
                                    }
                                }
                            }
                        }
                    } else {
                        for (auto it = xlights->AllObjects.begin(); it != xlights->AllObjects.end(); ++it) {
                            ViewObject *view_object = it->second;
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
                            UpdatePreview();
                        }
                    }
                    if (last_highlight != nullptr && last_highlight != which_object) {
                        if (last_highlight->Highlighted &&
                            !(last_highlight->Selected || last_highlight->GroupSelected)) {
                            last_highlight->Highlighted = false;
                            UpdatePreview();
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

    if (m_creating_bound_rect)
    {
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
        m_bound_end_x = ray_origin.x;
        m_bound_end_y = ray_origin.y;
        UpdatePreview();
        return;
    }
    else if (m_wheel_down)
    {
        float delta_x = event.GetX() - m_previous_mouse_x;
        float delta_y = event.GetY() - m_previous_mouse_y;
        delta_x /= modelPreview->GetZoom();
        delta_y /= modelPreview->GetZoom();
        modelPreview->SetPan(delta_x, -delta_y, 0.0f);
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
        UpdatePreview();
    }

    Model *m = newModel;
    if (m == nullptr) {
        m = dynamic_cast<Model*>(selectedBaseObject);
        if( m == nullptr ) return;
    }

    if(m_moving_handle)
    {
        if (m != newModel) {
            CreateUndoPoint("SingleModel", m->name, std::to_string(m_over_handle));
        }
        m->MoveHandle(modelPreview,m_over_handle, event.ShiftDown(), event.GetX(), event.GetY());
        SetupPropGrid(m);
        xlights->MarkEffectsFileDirty(true);
        UpdatePreview();
    }
    else if (m_dragging && event.Dragging())
    {
        double delta_x = event.GetPosition().x - m_previous_mouse_x;
        double delta_y = -(event.GetPosition().y - m_previous_mouse_y);
        delta_x /= modelPreview->GetZoom();
        delta_y /= modelPreview->GetZoom();
        int wi, ht;
        modelPreview->GetVirtualCanvasSize(wi, ht);
        if (wi > 0 && ht > 0)
        {
            for (size_t i=0; i<modelPreview->GetModels().size(); i++)
            {
                if(modelPreview->GetModels()[i]->Selected || modelPreview->GetModels()[i]->GroupSelected)
                {
                    CreateUndoPoint("SingleModel", m->name, "location");
                    modelPreview->GetModels()[i]->AddOffset(delta_x, delta_y, 0.0);
                    modelPreview->GetModels()[i]->UpdateXmlWithScale();
                    SetupPropGrid(modelPreview->GetModels()[i]);
                    xlights->MarkEffectsFileDirty(true);
                }
            }
        }
        m_previous_mouse_x = event.GetPosition().x;
        m_previous_mouse_y = event.GetPosition().y;
        xlights->SetStatusText(wxString::Format("x=%d y=%d",m_previous_mouse_x,m_previous_mouse_y));
        UpdatePreview();
    }
    else
    {
        if(m->Selected)
        {
            modelPreview->SetCursor(m->GetBaseObjectScreenLocation().CheckIfOverHandles(modelPreview, m_over_handle, event.GetX(), event.GetY()));
        }
    }
}

void LayoutPanel::OnPreviewRightDown(wxMouseEvent& event)
{
    modelPreview->SetFocus();
    wxMenu mnu;

    mnu.Append(ID_PREVIEW_RESET, "Reset");

    int selectedObjectCnt = editing_models ? ModelsSelectedCount() : ViewObjectsSelectedCount();

    if (selectedObjectCnt > 1)
    {
        wxMenu* mnuBulkEdit = new wxMenu();
        if( editing_models ) {
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTION, "Controller Connection");
        }
        mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_PREVIEW, "Preview");
        if( editing_models ) {
            mnuBulkEdit->Append(ID_PREVIEW_BULKEDIT_DIMMINGCURVES, "Dimming Curves");
        }
        mnuBulkEdit->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);

        wxMenu* mnuAlign = new wxMenu();
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
        mnuAlign->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);

        wxMenu* mnuDistribute = new wxMenu();
        mnuDistribute->Append(ID_PREVIEW_H_DISTRIBUTE,"Horizontal");
        mnuDistribute->Append(ID_PREVIEW_V_DISTRIBUTE,"Vertical");
        mnuDistribute->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);

        wxMenu* mnuResize = new wxMenu();
        mnuResize->Append(ID_PREVIEW_RESIZE_SAMEWIDTH, "Match Width");
        mnuResize->Append(ID_PREVIEW_RESIZE_SAMEHEIGHT, "Match Height");
        mnuResize->Append(ID_PREVIEW_RESIZE_SAMESIZE, "Match Size");
        mnuResize->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, nullptr, this);

        mnu.Append(ID_PREVIEW_BULKEDIT, "Bulk Edit", mnuBulkEdit, "");
        mnu.Append(ID_PREVIEW_ALIGN, "Align", mnuAlign, "");
        mnu.Append(ID_PREVIEW_DISTRIBUTE, "Distribute", mnuDistribute, "");
        mnu.Append(ID_PREVIEW_RESIZE, "Resize", mnuResize, "");

        mnu.AppendSeparator();
    }

    if (selectedObjectCnt > 0) {
        if (selectedBaseObject != nullptr && !selectedBaseObject->GetBaseObjectScreenLocation().IsLocked())
        {
            bool need_sep = false;
            if( editing_models ) {
                Model* model = dynamic_cast<Model*>(selectedBaseObject);
                int sel_seg = model->GetSelectedSegment();
                if( sel_seg != -1 ) {
                    if( !model->HasCurve(sel_seg) ) {
                        mnu.Append(ID_PREVIEW_MODEL_ADDPOINT,"Add Point");
                        mnu.Append(ID_PREVIEW_MODEL_ADDCURVE,"Define Curve");
                    } else {
                        mnu.Append(ID_PREVIEW_MODEL_DELCURVE,"Remove Curve");
                    }
                    need_sep = true;
                }
                int sel_hdl = model->GetSelectedHandle();
                if( (sel_hdl != -1) && (sel_hdl < 0x4000) && (sel_hdl < model->GetNumHandles()) && (model->GetNumHandles() > 2) ) {
                    mnu.Append(ID_PREVIEW_MODEL_DELETEPOINT,"Delete Point");
                    need_sep = true;
                }
            }
            if( need_sep ) {
                mnu.AppendSeparator();
            }
            if( editing_models && (selectedBaseObject->GetDisplayAs() == "Matrix" ))
            {
                mnu.Append(ID_PREVIEW_MODEL_ASPECTRATIO,"Correct Aspect Ratio");
            }
            if (is_3d && selectedObjectCnt == 1) {
                mnu.Append(ID_PREVIEW_ALIGN_GROUND, "Align With Ground");
            }
        }
        if (editing_models && (selectedBaseObject != nullptr))
        {
            Model* model = dynamic_cast<Model*>(selectedBaseObject);
            mnu.Append(ID_PREVIEW_MODEL_NODELAYOUT, "Node Layout");
            mnu.Append(ID_PREVIEW_MODEL_LOCK, "Lock");
            mnu.Append(ID_PREVIEW_MODEL_UNLOCK, "Unlock");
            if (model->SupportsExportAsCustom())
            {
                mnu.Append(ID_PREVIEW_MODEL_EXPORTASCUSTOM, "Export as Custom xLights Model");
            }
            if (model->SupportsWiringView())
            {
                mnu.Append(ID_PREVIEW_MODEL_WIRINGVIEW, "Wiring View");
            }
            if (model->SupportsXlightsModel())
            {
                mnu.Append(ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL, "Export xLights Model");
            }
            mnu.Append(ID_PREVIEW_MODEL_CREATEGROUP, "Create Group");
        }

        if (editing_models && (selectedObjectCnt == 1) && (modelPreview->GetModels().size() > 1))
        {
            mnu.Append(ID_PREVIEW_REPLACEMODEL, "Replace A Model With This Model");
        }
    }

    if( currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned" ) {
        if (selectedObjectCnt > 0) {
            mnu.AppendSeparator();
        }
        mnu.Append(ID_PREVIEW_DELETE_ACTIVE,"Delete this Preview");
    }

    mnu.Append(ID_PREVIEW_SAVE_LAYOUT_IMAGE, _("Save Layout Image"));
    mnu.Append(ID_PREVIEW_PRINT_LAYOUT_IMAGE, _("Print Layout Image"));

    // ViewPoint menus
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

void LayoutPanel::OnPreviewModelPopup(wxCommandEvent &event)
{
    if (event.GetId() == ID_PREVIEW_RESET)
    {
        modelPreview->Reset();
        UpdatePreview();
    }
    else if (event.GetId() == ID_PREVIEW_REPLACEMODEL)
    {
        ReplaceModel();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_TOP)
    {
        if(editing_models ) {
            PreviewModelAlignTops();
        } else {
            objects_panel->PreviewObjectAlignTops();
        }
    }
    else if (event.GetId() == ID_PREVIEW_SAVE_LAYOUT_IMAGE)
    {
        PreviewSaveImage();
    }
    else if (event.GetId() == ID_PREVIEW_PRINT_LAYOUT_IMAGE)
    {
        PreviewPrintImage();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_BOTTOM)
    {
        if(editing_models ) {
            PreviewModelAlignBottoms();
        } else {
            objects_panel->PreviewObjectAlignBottoms();
        }
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_GROUND)
    {
        if(editing_models ) {
            PreviewModelAlignWithGround();
        } else {
            objects_panel->PreviewObjectAlignWithGround();
        }
    }
    else if (event.GetId() == ID_PREVIEW_BULKEDIT_CONTROLLERCONNECTION)
    {
        BulkEditControllerConnection();
    }
    else if (event.GetId() == ID_PREVIEW_BULKEDIT_PREVIEW)
    {
        BulkEditControllerPreview();
    }
    else if (event.GetId() == ID_PREVIEW_BULKEDIT_DIMMINGCURVES)
    {
        BulkEditDimmingCurves();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_LEFT)
    {
        if(editing_models ) {
            PreviewModelAlignLeft();
        } else {
            objects_panel->PreviewObjectAlignLeft();
        }
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_RIGHT)
    {
        if(editing_models ) {
            PreviewModelAlignRight();
        } else {
            objects_panel->PreviewObjectAlignRight();
        }
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_FRONT)
    {
        if(editing_models ) {
            PreviewModelAlignFronts();
        } else {
            objects_panel->PreviewObjectAlignFronts();
        }
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_BACK)
    {
        if(editing_models ) {
            PreviewModelAlignBacks();
        } else {
            objects_panel->PreviewObjectAlignBacks();
        }
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_H_CENTER)
    {
        if(editing_models ) {
            PreviewModelAlignHCenter();
        } else {
            objects_panel->PreviewObjectAlignHCenter();
        }
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_V_CENTER)
    {
        if(editing_models ) {
            PreviewModelAlignVCenter();
        } else {
            objects_panel->PreviewObjectAlignVCenter();
        }
    }
    else if (event.GetId() == ID_PREVIEW_H_DISTRIBUTE)
    {
        if(editing_models ) {
            PreviewModelHDistribute();
        } else {
            objects_panel->PreviewObjectHDistribute();
        }
    }
    else if (event.GetId() == ID_PREVIEW_V_DISTRIBUTE)
    {
        if(editing_models ) {
            PreviewModelVDistribute();
        } else {
            objects_panel->PreviewObjectVDistribute();
        }
    }
    else if (event.GetId() == ID_PREVIEW_RESIZE_SAMEWIDTH)
    {
        if(editing_models ) {
            PreviewModelResize(true, false);
        } else {
            objects_panel->PreviewObjectResize(true, false);
        }
    }
    else if (event.GetId() == ID_PREVIEW_RESIZE_SAMEHEIGHT)
    {
        if(editing_models ) {
            PreviewModelResize(false, true);
        } else {
            objects_panel->PreviewObjectResize(false, true);
        }
    }
    else if (event.GetId() == ID_PREVIEW_RESIZE_SAMESIZE)
    {
        if(editing_models ) {
            PreviewModelResize(true, true);
        } else {
            objects_panel->PreviewObjectResize(true, true);
        }
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_NODELAYOUT)
    {
        ShowNodeLayout();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_LOCK)
    {
        LockSelectedModels(true);
        UpdatePreview();
        resetPropertyGrid();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_UNLOCK)
    {
        LockSelectedModels(false);
        UpdatePreview();
        resetPropertyGrid();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTASCUSTOM)
    {
        Model* md = dynamic_cast<Model*>(selectedBaseObject);
        if (md == nullptr) return;
        md->ExportAsCustomXModel();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_CREATEGROUP)
    {
        CreateModelGroupFromSelected();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_WIRINGVIEW)
    {
        ShowWiring();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_ASPECTRATIO)
    {
        Model* md=dynamic_cast<Model*>(selectedBaseObject);
        if( md == nullptr ) return;
        int screen_wi = md->GetBaseObjectScreenLocation().GetMWidth();
        int screen_ht = md->GetBaseObjectScreenLocation().GetMHeight();
        float render_ht = md->GetBaseObjectScreenLocation().GetRenderHt();
        float render_wi = md->GetBaseObjectScreenLocation().GetRenderWi();
        float ht_ratio = render_ht / (float)screen_ht;
        float wi_ratio = render_wi / (float)screen_wi;
        if( ht_ratio > wi_ratio) {
            render_wi = render_wi / ht_ratio;
            md->GetBaseObjectScreenLocation().SetMWidth((int)render_wi);
        } else {
            render_ht = render_ht / wi_ratio;
            md->GetBaseObjectScreenLocation().SetMHeight((int)render_ht);
        }
        UpdatePreview();
        resetPropertyGrid();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL)
    {
        Model* md=dynamic_cast<Model*>(selectedBaseObject);
        if( md == nullptr ) return;
        md->ExportXlightsModel();
    }
    else if (event.GetId() == ID_PREVIEW_DELETE_ACTIVE)
    {
        DeleteCurrentPreview();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_ADDPOINT)
    {
        Model* md=dynamic_cast<Model*>(selectedBaseObject);
        if( md == nullptr ) return;
        int handle = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(handle+0x8000));
        md->InsertHandle(handle);
        md->UpdateXmlWithScale();
        md->InitModel();
        SetupPropGrid(md);
        UpdatePreview();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_DELETEPOINT)
    {
        Model* md=dynamic_cast<Model*>(selectedBaseObject);
        if( md == nullptr ) return;
        int selected_handle = md->GetSelectedHandle();
        if( (selected_handle != -1) && (md->GetNumHandles() > 2) )
        {
            CreateUndoPoint("SingleModel", md->name, std::to_string(selected_handle+0x4000));
            md->DeleteHandle(selected_handle);
            md->UpdateXmlWithScale();
            md->InitModel();
            SetupPropGrid(md);
            UpdatePreview();
        }
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_ADDCURVE)
    {
        Model* md=dynamic_cast<Model*>(selectedBaseObject);
        if( md == nullptr ) return;
        int seg = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(seg+0x2000));
        md->SetCurve(seg, true);
        md->UpdateXmlWithScale();
        md->InitModel();
        UpdatePreview();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_DELCURVE)
    {
        Model* md=dynamic_cast<Model*>(selectedBaseObject);
        if( md == nullptr ) return;
        int seg = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(seg+0x1000));
        md->SetCurve(seg, false);
        md->UpdateXmlWithScale();
        md->InitModel();
        UpdatePreview();
    }
    else if (event.GetId() == ID_PREVIEW_SAVE_VIEWPOINT)
    {
        modelPreview->SaveCurrentCameraPosition();
        SetDirtyHiLight(true);
    }
    else if (is_3d) {
        if (xlights->viewpoint_mgr.GetNum3DCameras() > 0) {
            for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum3DCameras(); ++i)
            {
                if (event.GetId() == xlights->viewpoint_mgr.GetCamera3D(i)->GetMenuId())
                {
                    modelPreview->SetCamera3D(i);
                    UpdatePreview();
                    break;
                }
                else if (event.GetId() == xlights->viewpoint_mgr.GetCamera3D(i)->GetDeleteMenuId())
                {
                    xlights->viewpoint_mgr.DeleteCamera3D(i);
                }
            }
        }
    }
    else {
        if (xlights->viewpoint_mgr.GetNum2DCameras() > 0) {
            for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum2DCameras(); ++i)
            {
                if (event.GetId() == xlights->viewpoint_mgr.GetCamera2D(i)->GetMenuId())
                {
                    modelPreview->SetCamera2D(i);
                    UpdatePreview();
                    break;
                }
                else if (event.GetId() == xlights->viewpoint_mgr.GetCamera2D(i)->GetDeleteMenuId())
                {
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
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->GroupSelected || modelPreview->GetModels()[i]->Selected)
        {
            modelPreview->GetModels()[i]->SetBottom(0.0f);
        }
    }
    UpdatePreview();
}

void LayoutPanel::ShowNodeLayout()
{
    Model* md = dynamic_cast<Model*>(selectedBaseObject);
    if (md == nullptr) return;
    wxString html = md->ChannelLayoutHtml(xlights->GetOutputManager());
    ChannelLayoutDialog dialog(this);
    dialog.SetHtmlSource(html);
    dialog.ShowModal();
}

void LayoutPanel::ShowWiring()
{
    Model* md = dynamic_cast<Model*>(selectedBaseObject);
    if (md == nullptr) return;
    WiringDialog dlg(this, md->GetName());
    dlg.SetData(md);
    dlg.ShowModal();
}

void LayoutPanel::PreviewModelAlignTops()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0) return;

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float top = modelPreview->GetModels()[selectedindex]->GetTop();
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetTop(top);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignBottoms()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float bottom = modelPreview->GetModels()[selectedindex]->GetBottom();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetBottom(bottom);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignLeft()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float left = modelPreview->GetModels()[selectedindex]->GetLeft();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetLeft(left);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignFronts()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float front = modelPreview->GetModels()[selectedindex]->GetFront();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetFront(front);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignBacks()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float back = modelPreview->GetModels()[selectedindex]->GetBack();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetBack(back);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelResize(bool sameWidth, bool sameHeight)
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);

    if (sameWidth)

    {
        int width = modelPreview->GetModels()[selectedindex]->GetWidth();
        for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
        {
            if (modelPreview->GetModels()[i]->GroupSelected)
            {
                modelPreview->GetModels()[i]->SetWidth(width);
                bool z_scale = modelPreview->GetModels()[i]->GetBaseObjectScreenLocation().GetSupportsZScaling();
                if (z_scale) {
                    modelPreview->GetModels()[i]->GetBaseObjectScreenLocation().SetMDepth(width);
                }
            }
        }
    }

    if (sameHeight)
    {
        int height = modelPreview->GetModels()[selectedindex]->GetHeight();
        for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
        {
            if (modelPreview->GetModels()[i]->GroupSelected)
            {
                modelPreview->GetModels()[i]->SetHeight(height);
            }
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignRight()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float right = modelPreview->GetModels()[selectedindex]->GetRight();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetRight(right);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignHCenter()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float center = modelPreview->GetModels()[selectedindex]->GetHcenterPos();
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetHcenterPos(center);
        }
    }
    UpdatePreview();
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

    CreateUndoPoint("All", models.front()->name);

    float x = -1;
    for (auto it = models.begin(); it != models.end(); ++it)
    {
        if (it == models.begin())
        {
            x = (*it)->GetHcenterPos() + space;
        }
        else if (*it == models.back())
        {
            // do nothing
        }
        else
        {
            (*it)->SetHcenterPos(x);
            x += space;
        }
    }
    UpdatePreview();
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

    CreateUndoPoint("All", models.front()->name);

    float y = -1;
    for (auto it = models.begin(); it != models.end(); ++it)
    {
        if (it == models.begin())
        {
            y = (*it)->GetVcenterPos() + space;
        }
        else if (*it == models.back())
        {
            // do nothing
        }
        else
        {
            (*it)->SetVcenterPos(y);
            y += space;
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignVCenter()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex < 0) return;

    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float center = modelPreview->GetModels()[selectedindex]->GetVcenterPos();
    for (size_t i = 0; i < modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetVcenterPos(center);
        }
    }
    UpdatePreview();
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
    for (auto it = xlights->AllObjects.begin(); it != xlights->AllObjects.end(); ++it) {
        ViewObject *view_object = it->second;
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
    for (auto it = buttons.begin(); it != buttons.end(); ++it) {
        if (event.GetId() == (*it)->GetId()) {
            if ((*it)->GetModelType() == "Add Object") {
                DisplayAddObjectPopup();
            }
            else {
                int state = (*it)->GetState();
                (*it)->SetState(state + 1);
                if ((*it)->GetState()) {
                    selectedButton = (*it);
                    UnSelectAllModels();
                    modelPreview->SetFocus();
                }
                else {
                    selectedButton = nullptr;
                    _lastXlightsModel = "";
                }
                Notebook_Objects->ChangeSelection(0);
                editing_models = true;
            }
        } else if ((*it)->GetState()) {
            (*it)->SetState(0);
        }
    }
}

void LayoutPanel::AddObjectButton(wxMenu& mnu, const long id, const std::string &name, const char *icon[]) {
    wxMenuItem* menu_item = mnu.Append(id, name);
    if (icon != nullptr) {
        wxImage image(icon);
#ifdef __WXOSX__
        wxBitmap bitmap(image, -1, 2.0);
#else
        image.Rescale(ScaleWithSystemDPI(GetContentScaleFactor(), 24),
            ScaleWithSystemDPI(GetContentScaleFactor(), 24),
            wxIMAGE_QUALITY_HIGH);
        wxBitmap bitmap(image);
#endif
        menu_item->SetBitmap(image);
    }
}

void LayoutPanel::DisplayAddObjectPopup() {
    wxMenu mnuObjects;
    AddObjectButton(mnuObjects, ID_ADD_OBJECT_IMAGE, "Image", add_object_image_xpm);
    AddObjectButton(mnuObjects, ID_ADD_OBJECT_GRIDLINES, "Gridlines", add_object_gridlines_xpm);
    AddObjectButton(mnuObjects, ID_ADD_OBJECT_MESH, "Mesh", add_object_mesh_xpm);
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
        vobj->SetLayoutGroup(currentLayoutGroup);
        objects_panel->UpdateObjectList(true, currentLayoutGroup);
        object_created = true;
    }
    else if (id == ID_ADD_OBJECT_GRIDLINES)
    {
        logger_base.debug("OnAddObjectPopup - ID_ADD_OBJECT_GRIDLINES");
        CreateUndoPoint("All", "", "");
        vobj = xlights->AllObjects.CreateAndAddObject("Gridlines");
        vobj->SetLayoutGroup(currentLayoutGroup);
        objects_panel->UpdateObjectList(true, currentLayoutGroup);
        object_created = true;
    }
    else if (id == ID_ADD_OBJECT_MESH)
    {
        logger_base.debug("OnAddObjectPopup - ID_ADD_OBJECT_MESH");
        CreateUndoPoint("All", "", "");
        vobj = xlights->AllObjects.CreateAndAddObject("Mesh");
        vobj->SetLayoutGroup(currentLayoutGroup);
        objects_panel->UpdateObjectList(true, currentLayoutGroup);
        object_created = true;
    }

    if( object_created ) {
        Notebook_Objects->ChangeSelection(1);
        editing_models = false;
        SelectViewObject(vobj, true);
        vobj->UpdateXmlWithScale();
        SetupPropGrid(vobj);
    }

    Refresh();
}

Model *LayoutPanel::CreateNewModel(const std::string &type) const
{
    std::string t = type;
    if (t == "Import Custom" || t == "Download")
    {
        t = "Custom";
    }
    std::string startChannel = "1";
    if (xlights->AllModels[lastModelName] != nullptr) {
        startChannel = ">" + lastModelName + ":1";
    }
    else
    {
        long highestch = 0;
        Model* highest = nullptr;
        for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it)
        {
            if (it->second->GetDisplayAs() != "ModelGroup")
            {
                if (it->second->GetLastChannel() > highestch)
                {
                    highestch = it->second->GetLastChannel();
                    highest = it->second;
                }
            }
        }

        if (highest != nullptr)
        {
            startChannel = ">" + highest->GetName() + ":1";
        }
    }
    Model* m = xlights->AllModels.CreateDefaultModel(t, startChannel);

    return m;
}

std::list<Model*> LayoutPanel::GetSelectedModels() const
{
    std::list<Model*> res;

    if( editing_models ) {
        if (selectedBaseObject != nullptr)
        {
            res.push_back(dynamic_cast<Model*>(selectedBaseObject));
        }

        for (auto it = modelPreview->GetModels().begin(); it!= modelPreview->GetModels().end(); ++it)
        {
            if ((*it) != selectedBaseObject && ((*it)->Selected || (*it)->GroupSelected))
            {
                res.push_back(*it);
            }
        }
    }

    return res;
}

void LayoutPanel::Nudge(int key)
{
    if (!editing_models) return;
    std::list<Model*> selectedModels = GetSelectedModels();
    if (selectedModels.size() > 0)
    {
        if (selectedModels.size() == 1)
        {
            CreateUndoPoint("SingleModel", selectedModels.front()->name, "location");
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

        for (auto it : selectedModels)
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
                SetupPropGrid(it);
            }
        }

        xlights->MarkEffectsFileDirty(true);
        UpdatePreview();
        resetPropertyGrid();

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
#ifndef __WXOSX__
            if (event.ShiftDown()) // Cut
            {
                wxCommandEvent evt(wxEVT_MENU, wxID_CUT);
                DoCut(evt);
                event.StopPropagation();
            }
            else
#endif
            DeleteSelectedModel();
            event.StopPropagation();
            break;
        case WXK_BACK:
            DeleteSelectedModel();
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
            }
            else if (is_3d) {
                UnSelectAllModels();
            }
            break;

        default:
            event.Skip();
            break;
    }
}

void LayoutPanel::DeleteSelectedModel() {
    if( selectedBaseObject != nullptr && !selectedBaseObject->GetBaseObjectScreenLocation().IsLocked()) {
        CreateUndoPoint("All", selectedBaseObject->name);
        // This should delete all selected models
        //xlights->AllModels.Delete(selectedBaseObject->name);
        bool selectedModelFound = false;
        for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
        {
            if (modelPreview->GetModels()[i]->GroupSelected)
            {
                if (!selectedModelFound && modelPreview->GetModels()[i]->name == selectedBaseObject->name)
                {
                    selectedModelFound = true;
                }
                xlights->AllModels.Delete(modelPreview->GetModels()[i]->name);
            }
        }
        if (!selectedModelFound)
        {
            xlights->AllModels.Delete(selectedBaseObject->name);
        }
        selectedBaseObject = nullptr;
        xlights->UpdateModelsList();
        xlights->MarkEffectsFileDirty(true);
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

    if (dlg.ShowModal() == wxID_OK)
    {
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
        if (replaceModel->ModelStartChannel !=
            modelToReplaceItWith->ModelStartChannel &&
            wxString(modelToReplaceItWith->ModelStartChannel).StartsWith(">"))
        {
            auto msg = wxString::Format("Should I copy the replaced models start channel %s to the replacement model whose start channel is currently %s?", replaceModel->ModelStartChannel, modelToReplaceItWith->ModelStartChannel);
            if (wxMessageBox(msg, "Update Start Channel", wxYES_NO) == wxYES)
            {
                modelToReplaceItWith->SetStartChannel(replaceModel->ModelStartChannel, true);

                auto tocc = modelToReplaceItWith->GetControllerConnection();
                auto fromcc = replaceModel->GetControllerConnection();
                if (fromcc->GetAttribute("Protocol", "") != "")
                {
                    tocc->DeleteAttribute("Protocol");
                    tocc->AddAttribute("Protocol", fromcc->GetAttribute("Protocol"));
                }

                if (fromcc->GetAttribute("Port", "") != "")
                {
                    tocc->DeleteAttribute("Port");
                    tocc->AddAttribute("Port", fromcc->GetAttribute("Port"));
                }
            }
        }

        xlights->AllModels.RenameInListOnly(dlg.GetStringSelection().ToStdString(), "Iamgoingtodeletethismodel");
        replaceModel->Rename("Iamgoingtodeletethismodel");
        xlights->AllModels.RenameInListOnly(modelToReplaceItWith->GetName(), dlg.GetStringSelection().ToStdString());
        modelToReplaceItWith->Rename(dlg.GetStringSelection().ToStdString());
        xlights->AllModels.Delete("Iamgoingtodeletethismodel");
        xlights->UpdateModelsList();
        xlights->MarkEffectsFileDirty(true);
    }
}

void LayoutPanel::LockSelectedModels(bool lock)
{
    bool selectedModelFound = false;
    for (size_t i = 0; i<modelPreview->GetModels().size(); i++)
    {
        if (modelPreview->GetModels()[i]->GroupSelected)
        {
            if (!selectedModelFound && modelPreview->GetModels()[i]->name == selectedBaseObject->name)
            {
                selectedModelFound = true;
            }

            modelPreview->GetModels()[i]->Lock(lock);
        }
    }
    if (!selectedModelFound)
    {
        selectedBaseObject->Lock(lock);
    }
    xlights->MarkEffectsFileDirty(true);
}

void LayoutPanel::DoCopy(wxCommandEvent& event) {
    if (!modelPreview->HasFocus() && !TreeListViewModels->HasFocus() && !TreeListViewModels->GetView()->HasFocus()) {
        event.Skip();
    } else if (selectedBaseObject != nullptr) {
        CopyPasteModel copyData;

        copyData.SetModel(dynamic_cast<Model*>(selectedBaseObject));

        if (copyData.IsOk() && wxTheClipboard->Open()) {
            if (!wxTheClipboard->SetData(new wxTextDataObject(copyData.Serialise()))) {
                DisplayError("Unable to copy data to clipboard.", this);
            }
            wxTheClipboard->Close();
        }
    }
}

void LayoutPanel::DoCut(wxCommandEvent& event) {
    if (!modelPreview->HasFocus() && !TreeListViewModels->HasFocus() && !TreeListViewModels->GetView()->HasFocus()) {
        event.Skip();
    } else if (selectedBaseObject != nullptr) {
        DoCopy(event);
        DeleteSelectedModel();
    }
}

void LayoutPanel::DoPaste(wxCommandEvent& event) {
    if (!modelPreview->HasFocus() && !TreeListViewModels->HasFocus() && !TreeListViewModels->GetView()->HasFocus()) {
        event.Skip();
    } else {
        if (wxTheClipboard->Open()) {
            CreateUndoPoint("All", selectedBaseObject == nullptr ? "" : selectedBaseObject->name);

            wxTextDataObject data;
            wxTheClipboard->GetData(data);

            CopyPasteModel copyData(data.GetText().ToStdString());

            wxTheClipboard->Close();

            if (copyData.IsOk())
            {
                wxXmlNode* nd = copyData.GetModelXml();

                if (nd != nullptr)
                {
                    if (xlights->AllModels[lastModelName] != nullptr) {
                        nd->DeleteAttribute("StartChannel");
                        nd->AddAttribute("StartChannel", ">" + lastModelName + ":1");
                    }
                    else
                    {
                        long highestch = 0;
                        Model* highest = nullptr;
                        for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it)
                        {
                            if (it->second->GetDisplayAs() != "ModelGroup")
                            {
                                if (it->second->GetLastChannel() > highestch)
                                {
                                    highestch = it->second->GetLastChannel();
                                    highest = it->second;
                                }
                            }
                        }

                        if (highest != nullptr)
                        {
                            nd->DeleteAttribute("StartChannel");
                            nd->AddAttribute("StartChannel", ">" + highest->GetName() + ":1");
                        }
                        else
                        {
                            nd->DeleteAttribute("StartChannel");
                            nd->AddAttribute("StartChannel", "1");
                        }
                    }

                    Model *newModel = xlights->AllModels.CreateModel(nd);
                    std::string name = xlights->AllModels.GenerateModelName(newModel->name);
                    newModel->name = name;
                    newModel->GetModelXml()->DeleteAttribute("name");
                    newModel->Lock(false);
                    newModel->GetModelXml()->AddAttribute("name", name);
                    newModel->AddOffset(0.02, 0.02, 0.0);
                    newModel->UpdateXmlWithScale();
                    xlights->AllModels.AddModel(newModel);
                    lastModelName = name;

                    xlights->UpdateModelsList();
                    xlights->MarkEffectsFileDirty(true);
                    SelectBaseObject(name);
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
    int sz = undoBuffer.size() - 1;
    if (sz >= 0) {
        UnSelectAllModels();

        if (undoBuffer[sz].type == "Background") {
            wxPropertyGridEvent pgEvent;
            pgEvent.SetPropertyGrid(propertyEditor);
            wxStringProperty wsp("Background", undoBuffer[sz].key, undoBuffer[sz].data);
            pgEvent.SetProperty(&wsp);
            wxVariant value(undoBuffer[sz].data);
            pgEvent.SetPropertyValue(value);
            OnPropertyGridChange(pgEvent);
            UnSelectAllModels();
        } else if (undoBuffer[sz].type == "ModelProperty") {
            SelectModel(undoBuffer[sz].model);
            wxPropertyGridEvent event2;
            event2.SetPropertyGrid(propertyEditor);
            wxStringProperty wsp("Model", undoBuffer[sz].key, undoBuffer[sz].data);
            event2.SetProperty(&wsp);
            wxVariant value(undoBuffer[sz].data);
            event2.SetPropertyValue(value);
            OnPropertyGridChange(event2);
            xlights->MarkEffectsFileDirty(true);
            resetPropertyGrid();
        } else if (undoBuffer[sz].type == "ObjectProperty") {
            ViewObject* vobj = xlights->AllObjects[undoBuffer[sz].model];
            SelectViewObject(vobj);
            wxPropertyGridEvent event2;
            event2.SetPropertyGrid(propertyEditor);
            wxStringProperty wsp("Object", undoBuffer[sz].key, undoBuffer[sz].data);
            event2.SetProperty(&wsp);
            wxVariant value(undoBuffer[sz].data);
            event2.SetPropertyValue(value);
            OnPropertyGridChange(event2);
            xlights->MarkEffectsFileDirty(true);
            resetPropertyGrid();
        } else if (undoBuffer[sz].type == "SingleModel") {
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
                xlights->MarkEffectsFileDirty(true);
            }
        } else if (undoBuffer[sz].type == "All") {
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

            xlights->UpdateModelsList();
            xlights->MarkEffectsFileDirty(true);
            if (undoBuffer[sz].model != "") {
                SelectModel(undoBuffer[sz].model);
            }
        } else if (undoBuffer[sz].type == "ModelName") {
            std::string origName = undoBuffer[sz].model;
            std::string newName = undoBuffer[sz].key;
            if (lastModelName == newName) {
                lastModelName = origName;
            }
            xlights->RenameModel(newName, origName);

            xlights->UpdateModelsList();
            xlights->MarkEffectsFileDirty(true);
            SelectModel(origName);
        } else if (undoBuffer[sz].type == "ObjectName") {
            std::string origName = undoBuffer[sz].model;
            std::string newName = undoBuffer[sz].key;
            xlights->RenameObject(newName, origName);
            xlights->UpdateModelsList();
            xlights->MarkEffectsFileDirty(true);
            ViewObject *vobj = xlights->AllObjects[origName];
            SelectViewObject(vobj);
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
    xlights->MarkEffectsFileDirty(false);
    int idx = undoBuffer.size();

    //printf("%s   %s   %s  %s\n", type.c_str(), model.c_str(), key.c_str(), data.c_str());
    if (idx > 0 )  {
        if ((type == "SingleModel" || type == "ModelProperty" || type == "Background")
            && undoBuffer[idx - 1].model == model && undoBuffer[idx - 1].key == key)  {
            //SingleModel - multi mouse movement, just record the original
            //Background/ModelProperty - multiple changes of the same property (like spinning spin button)
            return;
        }
        else if ((type == "SingleObject" || type == "ObjectProperty")
            && undoBuffer[idx - 1].model == model && undoBuffer[idx - 1].key == key)  {
            return;
        }
    }
    if (idx >= 100) {  //100 steps is more than enough IMO
        for (int x = 1; x < idx; x++) {
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
        Model *m = newModel;
        if (m == nullptr) {
            if( selectedBaseObject == nullptr ) {
                undoBuffer.resize(idx);
                return;
            }
            m=dynamic_cast<Model*>(selectedBaseObject);
        }
        wxXmlDocument doc;
        wxXmlNode *parent = m->GetModelXml()->GetParent();
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
        wxXmlDocument doc;
        wxXmlNode *parent = obj->GetModelXml()->GetParent();
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
        wxXmlNode *next = xlights->ModelsNode->GetNext();
        parent->RemoveChild(xlights->ModelsNode);
        doc.SetRoot(xlights->ModelsNode);
        wxStringOutputStream stream;
        doc.Save(stream);
        undoBuffer[idx].models = stream.GetString();
        doc.DetachRoot();
        parent->InsertChild(xlights->ModelsNode, next);

        parent = xlights->ViewObjectsNode->GetParent();
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

void LayoutPanel::OnModelsPopup(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int id = event.GetId();
    if(id == ID_MNU_DELETE_MODEL)
    {
        logger_base.debug("LayoutPanel::OnModelsPopup DELETE_MODEL");
        DeleteSelectedModel();
    }
    else if(id == ID_MNU_DELETE_MODEL_GROUP)
    {
        logger_base.debug("LayoutPanel::OnModelsPopup DELETE_MODEL_GROUP");
        if( mSelectedGroup.IsOk() ) {
            wxString name = TreeListViewModels->GetItemText(mSelectedGroup);
            if (wxMessageBox("Are you sure you want to remove the " + name + " group?", "Confirm Remove?", wxICON_QUESTION | wxYES_NO) == wxYES) {
                xlights->AllModels.Delete(name.ToStdString());
                selectedBaseObject = nullptr;
                mSelectedGroup = nullptr;
                UnSelectAllModels();
                ShowPropGrid(true);
                xlights->UpdateModelsList();
                xlights->MarkEffectsFileDirty(true);
            }
        }
    }
    else if(id == ID_MNU_DELETE_EMPTY_MODEL_GROUPS)
    {
        logger_base.debug("LayoutPanel::OnModelsPopup DELETE_EMPTY_MODEL_GROUPS");

        bool deleted = true;

        while (deleted)
        {
            deleted = false;
            auto it = xlights->AllModels.begin();
            while (it != xlights->AllModels.end())
            {
                if (it->second->GetDisplayAs() == "ModelGroup")
                {
                    ModelGroup* mg = dynamic_cast<ModelGroup*>(it->second);
                    ++it;
                    if (mg->GetModelCount() == 0)
                    {
                        xlights->AllModels.Delete(mg->GetName());
                        deleted = true;
                    }
                }
                else
                {
                    ++it;
                }
            }
        }

        mSelectedGroup = nullptr;
        UnSelectAllModels();
        ShowPropGrid(true);
        xlights->UpdateModelsList();
        xlights->MarkEffectsFileDirty(true);
    }
    else if (id == ID_MNU_MAKEALLSCVALID)
    {
        if (wxMessageBox("While this will make all your start channels valid it will not magically make your start channel right for your show. It will however solve strange nodes lighting up in the sequencer.\nAre you ok with this?", "WARNING", wxYES_NO | wxCENTRE) == wxYES)
        {
            Model* lastModel = nullptr;
            long lastModelEndChannel = 0;
            for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it)
            {
                if (it->second->GetLastChannel() > lastModelEndChannel)
                {
                    if (it->second->GetDisplayAs() != "ModelGroup" && it->second->CouldComputeStartChannel && it->second->IsValidStartChannelString())
                    {
                        lastModelEndChannel = it->second->GetLastChannel();
                        lastModel = it->second;
                    }
                }
            }

            for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it)
            {
                if (it->second->GetDisplayAs() != "ModelGroup" && (!it->second->CouldComputeStartChannel || !it->second->IsValidStartChannelString()))
                {
                    if (lastModel == nullptr)
                    {
                        it->second->SetStartChannel("1", true);
                        lastModel = it->second;
                    }
                    else
                    {
                        it->second->SetStartChannel(">" + lastModel->GetName() + ":1", true);
                        lastModel = it->second;
                    }
                }
            }
            //xlights->RecalcModels(true);
            xlights->UpdateModelsList();
            xlights->MarkEffectsFileDirty(true);
        }
    }
    else if (id == ID_MNU_MAKEALLSCNOTOVERLAPPING)
    {
        if (wxMessageBox("While this will make all your start channels not overlapping it will not magically make your start channel right for your show. It will however solve strange nodes lighting up in the sequencer.\nAre you ok with this?", "WARNING", wxYES_NO | wxCENTRE) == wxYES)
        {
            Model* lastModel = nullptr;
            long lastModelEndChannel = 0;
            for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it)
            {
                if (it->second->GetLastChannel() > lastModelEndChannel)
                {
                    if (it->second->GetDisplayAs() != "ModelGroup" && it->second->CouldComputeStartChannel && it->second->IsValidStartChannelString())
                    {
                        lastModelEndChannel = it->second->GetLastChannel();
                        lastModel = it->second;
                    }
                }
            }

            for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it)
            {
                if (it->second->GetDisplayAs() != "ModelGroup" && xlights->AllModels.IsModelOverlapping(it->second))
                {
                    if (lastModel == nullptr)
                    {
                        it->second->SetStartChannel("1", true);
                        lastModel = it->second;
                    }
                    else
                    {
                        it->second->SetStartChannel(">" + lastModel->GetName() + ":1", true);
                        lastModel = it->second;
                    }
                }
            }
            //xlights->RecalcModels(true);
            xlights->UpdateModelsList();
            xlights->MarkEffectsFileDirty(true);
        }
    }
    else if (id == ID_MNU_MAKESCVALID)
    {
        if (wxMessageBox("While this will make your start channel valid and not overlapping it will not magically make your start channel right for your show. It will however solve strange nodes lighting up in the sequencer.\nAre you ok with this?", "WARNING", wxYES_NO | wxCENTRE) == wxYES)
        {
            if (selectedBaseObject != nullptr)
            {
                Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
                Model* lastModel = nullptr;
                long lastModelEndChannel = 0;
                for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it)
                {
                    if (it->second->GetLastChannel() > lastModelEndChannel)
                    {
                        if (it->second->GetDisplayAs() != "ModelGroup" && it->second->CouldComputeStartChannel && it->second->IsValidStartChannelString())
                        {
                            lastModelEndChannel = it->second->GetLastChannel();
                            lastModel = it->second;
                        }
                    }
                }
                if (lastModel == nullptr)
                {
                    selectedModel->SetStartChannel("1", true);
                }
                else
                {
                    selectedModel->SetStartChannel(">" + lastModel->GetName() + ":1", true);
                }
                xlights->UpdateModelsList();
                xlights->MarkEffectsFileDirty(true);
            }
        }
    }
    else if (id == ID_MNU_RENAME_MODEL_GROUP)
    {
        logger_base.debug("LayoutPanel::OnModelsPopup RENAME_MODEL_GROUP");
        if( mSelectedGroup.IsOk() ) {
            wxString sel = TreeListViewModels->GetItemText(mSelectedGroup);
            wxTextEntryDialog dlg(this, "Enter new name for group " + sel, "Rename " + sel, sel);
            if (dlg.ShowModal() == wxID_OK) {
                wxString name = wxString(Model::SafeModelName(dlg.GetValue().ToStdString()));

                while (xlights->AllModels.GetModel(name.ToStdString()) != nullptr) {
                    wxTextEntryDialog dlg2(this, "Model or Group of name " + name + " already exists. Enter new name for group", "Enter new name for group");
                    if (dlg2.ShowModal() == wxID_OK) {
                        name = wxString(Model::SafeModelName(dlg2.GetValue().ToStdString()));
                    } else {
                        return;
                    }
                }

                if (xlights->RenameModel(sel.ToStdString(), name.ToStdString())) {
                    CallAfter(&LayoutPanel::UpdateModelList, true);
                }

                xlights->UpdateModelsList();
                xlights->MarkEffectsFileDirty(true);
                model_grp_panel->UpdatePanel(name.ToStdString());
            }
        }
    }
    else if(id == ID_MNU_ADD_MODEL_GROUP)
    {
        logger_base.debug("LayoutPanel::OnModelsPopup ADD_MODEL_GROUP");
        wxTextEntryDialog dlg(this, "Enter name for new group", "Enter name for new group");
        if (dlg.ShowModal() == wxID_OK && !Model::SafeModelName(dlg.GetValue().ToStdString()).empty()) {
            wxString name = wxString(Model::SafeModelName(dlg.GetValue().ToStdString()));
            while (xlights->AllModels.GetModel(name.ToStdString()) != nullptr) {
                wxTextEntryDialog dlg2(this, "Model of name " + name + " already exists. Enter name for new group", "Enter name for new group");
                if (dlg2.ShowModal() == wxID_OK) {
                    name = wxString(Model::SafeModelName(dlg2.GetValue().ToStdString()));
                } else {
                    return;
                }
            }
            wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE, "modelGroup");
            xlights->ModelGroupsNode->AddChild(node);
            node->AddAttribute("selected", "0");
            node->AddAttribute("name", name);
            node->AddAttribute("models", "");
            node->AddAttribute("layout", "minimalGrid");
            node->AddAttribute("GridSize", "400");
            wxString grp = currentLayoutGroup == "All Models" ? "Unassigned" : currentLayoutGroup;
            node->AddAttribute("LayoutGroup", grp);

            xlights->AllModels.AddModel(xlights->AllModels.CreateModel(node));
            xlights->UpdateModelsList();
            xlights->MarkEffectsFileDirty(true);
            model_grp_panel->UpdatePanel(name.ToStdString());
            ShowPropGrid(false);
        }
    }
}

LayoutGroup* LayoutPanel::GetLayoutGroup(const std::string &name)
{
    for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); ++it) {
        LayoutGroup* grp = (LayoutGroup*)(*it);
        if( grp->GetName() == name ) {
            return grp;
        }
    }
    return nullptr;
}

void LayoutPanel::OnChoiceLayoutGroupsSelect(wxCommandEvent& event)
{
    for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it) {
        Model *model = it->second;
        model->Selected = false;
        model->GroupSelected = false;
        model->Highlighted = false;
    }

    std::string choice_layout = std::string(ChoiceLayoutGroups->GetStringSelection().c_str());
    if( choice_layout == "<Create New Preview>" ) {
        wxTextEntryDialog dlg(this, "Enter name for new preview", "Create New Preview");
        if (dlg.ShowModal() == wxID_OK) {
            wxString name = dlg.GetValue();
            while (GetLayoutGroup(name.ToStdString()) != nullptr || name == "Default" || name == "All Models" || name == "Unassigned") {
                wxTextEntryDialog dlg2(this, "Preview of name " + name + " already exists. Enter name for new preview", "Create New Preview");
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

            mSelectedGroup = nullptr;
            LayoutGroup* grp = new LayoutGroup(name.ToStdString(), xlights, node);
            grp->SetBackgroundImage(xlights->GetDefaultPreviewBackgroundImage());
            xlights->LayoutGroups.push_back(grp);
            xlights->AddPreviewOption(grp);
            SetCurrentLayoutGroup(name.ToStdString());
            AddPreviewChoice(name.ToStdString());
            ChoiceLayoutGroups->SetSelection(ChoiceLayoutGroups->GetCount() - 2);

            xlights->UpdateModelsList();
            xlights->MarkEffectsFileDirty(true);
            ShowPropGrid(true);
        } else {
            SwitchChoiceToCurrentLayoutGroup();
            return;
        }
    } else {
        SetCurrentLayoutGroup(choice_layout);
        mSelectedGroup = nullptr;
        UpdateModelList(true);
    }
    modelPreview->SetDisplay2DBoundingBox(xlights->GetDisplay2DBoundingBox());
    modelPreview->SetDisplay2DCenter0(xlights->GetDisplay2DCenter0());
    modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
    modelPreview->SetScaleBackgroundImage(GetBackgroundScaledForSelectedPreview());
    modelPreview->SetBackgroundBrightness(GetBackgroundBrightnessForSelectedPreview(), GetBackgroundAlphaForSelectedPreview());
    UpdatePreview();

    xlights->SetStoredLayoutGroup(currentLayoutGroup);
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

void LayoutPanel::PreviewPrintImage()
{
	class Printout : public wxPrintout
	{
	public:
		Printout(xlGLCanvas *canvas) : m_canvas(canvas), m_image(nullptr), m_grabbedImage(false) {}
		virtual ~Printout()
		{
			clearImage();
		}

		void clearImage()
		{
			if (m_image != nullptr)
			{
				delete m_image;
				m_image = nullptr;
			}
		}

		virtual bool GrabImage()
		{
			clearImage();

			wxRect rect = GetLogicalPageRect();
			rect.Deflate(rect.GetWidth() / 20, rect.GetHeight() / 20);
			wxRect adjustedRect = scaledRect(m_canvas->getWidth(), m_canvas->getHeight(), rect.GetWidth(), rect.GetHeight());

			m_image = m_canvas->GrabImage(wxSize(adjustedRect.GetWidth(), adjustedRect.GetHeight()));
			m_grabbedImage = (m_image != nullptr);
			return m_grabbedImage;
		}

		virtual bool OnPrintPage(int page) override
		{
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
		xlGLCanvas *m_canvas;
		wxImage *m_image;
		bool m_grabbedImage;
	};

	Printout printout(modelPreview);

	static wxPrintDialogData printDialogData;
	wxPrinter printer(&printDialogData);

	if (!printer.Print(this, &printout, true))
	{
		if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
		{
			DisplayError(wxString::Format("Problem printing. %d", wxPrinter::GetLastError()).ToStdString());
		}
    }
	else
	{
		printDialogData = printer.GetPrintDialogData();
        if (!printout.grabbedImage())
        {
            DisplayError("Problem grabbing ModelPreview image for printing", this);
        }
    }
}

void LayoutPanel::AddPreviewChoice(const std::string &name)
{
    ChoiceLayoutGroups->Insert(name, ChoiceLayoutGroups->GetCount()-1);
    model_grp_panel->AddPreviewChoice(name);

    // see if we need to switch to this one
    const std::string& storedLayoutGroup = xlights->GetStoredLayoutGroup();
    if( storedLayoutGroup == name ) {
        for( int i = 0; i < (int)ChoiceLayoutGroups->GetCount(); i++ )
        {
            if( ChoiceLayoutGroups->GetString(i) == storedLayoutGroup )
            {
                SetCurrentLayoutGroup(storedLayoutGroup);
                ChoiceLayoutGroups->SetSelection(i);
                modelPreview->SetDisplay2DBoundingBox(xlights->GetDisplay2DBoundingBox());
                modelPreview->SetDisplay2DCenter0(xlights->GetDisplay2DCenter0());
                modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
                modelPreview->SetScaleBackgroundImage(GetBackgroundScaledForSelectedPreview());
                modelPreview->SetBackgroundBrightness(GetBackgroundBrightnessForSelectedPreview(), GetBackgroundAlphaForSelectedPreview());
                UpdatePreview();
                break;
            }
        }
    }
}

const wxString& LayoutPanel::GetBackgroundImageForSelectedPreview() {
    previewBackgroundFile = xlights->GetDefaultPreviewBackgroundImage();
    if( currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned" ) {
        previewBackgroundFile = pGrp->GetBackgroundImage();
    }
    return previewBackgroundFile;
}

bool LayoutPanel::GetBackgroundScaledForSelectedPreview()
{
    previewBackgroundScaled = xlights->GetDefaultPreviewBackgroundScaled();
    if( currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned" ) {
        previewBackgroundScaled = pGrp->GetBackgroundScaled();
    }
    return previewBackgroundScaled;
}

int LayoutPanel::GetBackgroundBrightnessForSelectedPreview()
{
    previewBackgroundBrightness = xlights->GetDefaultPreviewBackgroundBrightness();
    if( currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned" ) {
        previewBackgroundBrightness = pGrp->GetBackgroundBrightness();
    }
    return previewBackgroundBrightness;
}
int LayoutPanel::GetBackgroundAlphaForSelectedPreview()
{
    previewBackgroundAlpha = xlights->GetDefaultPreviewBackgroundAlpha();
    if( currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned" ) {
        previewBackgroundAlpha = pGrp->GetBackgroundAlpha();
    }
    return previewBackgroundAlpha;
}

void LayoutPanel::SwitchChoiceToCurrentLayoutGroup() {
    ChoiceLayoutGroups->SetSelection(0);
    for (int i = 0; i < (int)ChoiceLayoutGroups->GetCount(); i++ )
    {
        if( ChoiceLayoutGroups->GetString(i) == currentLayoutGroup )
        {
            ChoiceLayoutGroups->SetSelection(i);
            break;
        }
    }
}

void LayoutPanel::DeleteCurrentPreview()
{
    if (wxMessageBox("Are you sure you want to delete the " + currentLayoutGroup + " preview?", "Confirm Delete?", wxICON_QUESTION | wxYES_NO) == wxYES) {
        for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); ++it) {
            LayoutGroup* grp = (LayoutGroup*)(*it);
            if (grp != nullptr) {
                if( currentLayoutGroup == grp->GetName() ) {
                    xlights->RemovePreviewOption(grp);
                    grp->GetLayoutGroupXml()->GetParent()->RemoveChild(grp->GetLayoutGroupXml());
                    xlights->LayoutGroups.erase(it);
                    delete grp->GetLayoutGroupXml();
                    delete grp;
                    break;
                }
            }
        }
        xlights->MarkEffectsFileDirty(false);
        mSelectedGroup = nullptr;
        for( int i = 0; i < (int)ChoiceLayoutGroups->GetCount(); i++ )
        {
            if( ChoiceLayoutGroups->GetString(i) == currentLayoutGroup )
            {
                ChoiceLayoutGroups->Delete(i);
                break;
            }
        }
        // change any existing assignments to this preview to be unassigned
        for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it) {
            Model *model = it->second;
            if( model->GetLayoutGroup() == currentLayoutGroup) {
                model->SetLayoutGroup("Unassigned");
            }
        }

        SetCurrentLayoutGroup("Default");
        ChoiceLayoutGroups->SetSelection(0);
        xlights->SetStoredLayoutGroup(currentLayoutGroup);

        UpdateModelList(true);
        modelPreview->SetDisplay2DBoundingBox(xlights->GetDisplay2DBoundingBox());
        modelPreview->SetDisplay2DCenter0(xlights->GetDisplay2DCenter0());
        modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
        modelPreview->SetScaleBackgroundImage(GetBackgroundScaledForSelectedPreview());
        modelPreview->SetBackgroundBrightness(GetBackgroundBrightnessForSelectedPreview(), GetBackgroundAlphaForSelectedPreview());
        UpdatePreview();
    }
}

void LayoutPanel::ShowPropGrid(bool show)
{
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
    for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); ++it) {
        LayoutGroup* grp = (LayoutGroup*)(*it);
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
    if( !editing_models ) {
        objects_panel->OnItemContextMenu(event);
        return;
    }

    wxMenu mnuContext;
    wxTreeListItem item = event.GetItem();
    if( item.IsOk() ) {
        ModelTreeData *data = dynamic_cast<ModelTreeData*>(TreeListViewModels->GetItemData(item));
        Model* model = data != nullptr ? data->GetModel() : nullptr;
        if( model != nullptr ) {
            if( model->GetDisplayAs() == "ModelGroup" ) {
                mSelectedGroup = item;
            } else {
                mSelectedGroup = nullptr;
                SelectModel(model, false);
            }
        }
    } else {
        return;
    }

    Model* selectedModel = dynamic_cast<Model*>(selectedBaseObject);
    if (selectedModel != nullptr && selectedModel->GetDisplayAs() != "SubModel") {
        mnuContext.Append(ID_MNU_DELETE_MODEL,"Delete");
        mnuContext.AppendSeparator();
    }

    mnuContext.Append(ID_MNU_ADD_MODEL_GROUP,"Add Group");
    if( mSelectedGroup.IsOk() ) {
        mnuContext.Append(ID_MNU_DELETE_MODEL_GROUP,"Delete Group");
        mnuContext.Append(ID_MNU_RENAME_MODEL_GROUP,"Rename Group");
    }
    mnuContext.Append(ID_MNU_DELETE_EMPTY_MODEL_GROUPS, "Delete Empty Groups");

    bool foundInvalid = false;
    bool foundOverlapping = false;
    if (selectedModel != nullptr && selectedModel->GetDisplayAs() != "SubModel" && selectedModel->GetDisplayAs() != "ModelGroup")
    {
        if (!selectedModel->CouldComputeStartChannel || !selectedModel->IsValidStartChannelString())
        {
            mnuContext.Append(ID_MNU_MAKESCVALID, "Make Start Channel Valid");
            foundInvalid = true;
        }
        if (xlights->AllModels.IsModelOverlapping(selectedModel))
        {
            foundOverlapping = true;
            mnuContext.Append(ID_MNU_MAKESCVALID, "Make Start Channel Not Overlapping");
        }
    }

    for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it)
    {
        if (it->second->GetDisplayAs() != "ModelGroup")
        {
            if (!foundInvalid && (!it->second->CouldComputeStartChannel || !it->second->IsValidStartChannelString()))
            {
                foundInvalid = true;
            }
            if (!foundOverlapping && xlights->AllModels.IsModelOverlapping(it->second))
            {
                foundOverlapping = true;
            }
        }
    }

    if (foundInvalid)
    {
        mnuContext.Append(ID_MNU_MAKEALLSCVALID, "Make All Start Channels Valid");
    }
    if (foundOverlapping)
    {
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
    UnSelectAllModels(false);
    if( editing_models ) {
        wxTreeListItem item = event.GetItem();
        if (item.IsOk()) {

            ModelTreeData *data = (ModelTreeData*)TreeListViewModels->GetItemData(item);
            Model *model = ((data != nullptr) ? data->GetModel() : nullptr);
            if (model != nullptr) {
                if (model->GetDisplayAs() == "ModelGroup") {
                    mSelectedGroup = item;
                    ShowPropGrid(false);
                    UpdateModelList(false);
                    model_grp_panel->UpdatePanel(model->name);
                } else {
                    mSelectedGroup = nullptr;
                    ShowPropGrid(true);
                    SelectModel(model, false);
                    SetToolTipForTreeList(TreeListViewModels, xlights->GetChannelToControllerMapping(model->GetNumberFromChannelString(model->ModelStartChannel)));
                }
            } else {
                mSelectedGroup = nullptr;
                selectedBaseObject = nullptr;
                ShowPropGrid(true);
                UnSelectAllModels(true);
                SetToolTipForTreeList(TreeListViewModels, "");
            }
            #ifndef LINUX
            TreeListViewModels->SetFocus();
            #endif
        } else {
            SetToolTipForTreeList(TreeListViewModels, "");
        }
    } else {
        ViewObject* view_object = nullptr;
        bool show_prop_grid = objects_panel->OnSelectionChanged(event, &view_object, currentLayoutGroup);
        SelectViewObject(view_object, false);
        ShowPropGrid(show_prop_grid);
        selectedBaseObject = view_object;
    }
}

void LayoutPanel::ModelGroupUpdated(ModelGroup *grp, bool full_refresh) {

    if (grp == nullptr) return;

    xlights->UnsavedRgbEffectsChanges = true;
    xlights->modelsChangeCount++;
    std::vector<Model *> models;

    UpdateModelList(full_refresh, models);
    if (full_refresh) return;

    TreeListViewModels->Freeze();

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
                for (auto it = grp->ModelNames().begin(); it != grp->ModelNames().end(); ++it) {
                    Model *m = xlights->AllModels[*it];
                    if (m != nullptr)
                    {
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

    for (auto a = toRemove.begin(); a != toRemove.end(); ++a) {
        TreeListViewModels->DeleteItem(*a);
    }

    for (auto a = modelsToAdd.begin(); a != modelsToAdd.end(); ++a) {
        TreeListViewModels->GetRootItem();
        AddModelToTree(*a, &root, false, 0);
    }

    TreeListViewModels->Thaw();
    TreeListViewModels->Refresh();
}

CopyPasteModel::~CopyPasteModel()
{
    if (_xmlNode != nullptr)
    {
        delete _xmlNode;
    }
}

CopyPasteModel::CopyPasteModel()
{
    _ok = false;
    _xmlNode = nullptr;
}

CopyPasteModel::CopyPasteModel(const std::string& in)
{
    _ok = false;
    _xmlNode = nullptr;

    // check it looks like xml ... to stop parser errors
    wxString xml = in;
    if (!xml.StartsWith("<?xml") || !xml.Contains("<model "))
    {
        // not valid
        return;
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

void CopyPasteModel::SetModel(Model* model)
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

std::string CopyPasteModel::Serialise() const
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
    modelPreview->Set3D(is_3d);
    if (is_3d) {
        if (selectedBaseObject != nullptr) {
            selectionLatched = true;
            highlightedBaseObject = selectedBaseObject;
            selectedBaseObject->GetBaseObjectScreenLocation().SetActiveHandle(CENTER_HANDLE);
        }
        else {
            UnSelectAllModels();
        }
	    Notebook_Objects->AddPage(PanelObjects, _("3D Objects"), false);
    } else {
        Notebook_Objects->RemovePage(1);
    }
    obj_button->Enable(is_3d);

    wxConfigBase* config = wxConfigBase::Get();
    config->Write("LayoutMode3D", is_3d);
    Refresh();
}

bool LayoutPanel::HandleLayoutKeyBinding(wxKeyEvent& event)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    auto k = event.GetKeyCode();
    if (k == WXK_SHIFT || k == WXK_CONTROL || k == WXK_ALT) return false;

    if ((!event.ControlDown() && !event.CmdDown() && !event.AltDown()) ||
        (k == 'A' && (event.ControlDown() || event.CmdDown()) && !event.AltDown()))
    {
        // let crontrol A through
        // Just a regular key ... If current focus is a control then we need to not process this
        if (dynamic_cast<wxControl*>(event.GetEventObject()) != nullptr &&
            (k < 128 || k == WXK_NUMPAD_END || k == WXK_NUMPAD_HOME || k == WXK_NUMPAD_INSERT || k == WXK_HOME || k == WXK_END || k == WXK_NUMPAD_SUBTRACT || k == WXK_NUMPAD_DECIMAL))
        {
            return false;
        }
    }

    auto binding = xlights->GetMainSequencer()->keyBindings.Find(event, KBSCOPE::Layout);
    if (binding != nullptr) {
        std::string type = binding->GetType();
        if (type == "LOCK_MODEL")
        {
            LockSelectedModels(true);
            UpdatePreview();
            resetPropertyGrid();
        }
        else if (type == "UNLOCK_MODEL")
        {
            LockSelectedModels(false);
            UpdatePreview();
            resetPropertyGrid();
        }
        else if (type == "GROUP_MODELS")
        {
            CreateModelGroupFromSelected();
        }
        else if (type == "WIRING_VIEW")
        {
            ShowWiring();
        }
        else if (type == "NODE_LAYOUT")
        {
            ShowNodeLayout();
        }
        else if (type == "SAVE_LAYOUT")
        {
            SaveEffects();
        }
        else if (type == "MODEL_ALIGN_TOP")
        {
            PreviewModelAlignTops();
        }
        else if (type == "MODEL_ALIGN_BOTTOM")
        {
            PreviewModelAlignBottoms();
        }
        else if (type == "MODEL_ALIGN_LEFT")
        {
            PreviewModelAlignLeft();
        }
        else if (type == "MODEL_ALIGN_RIGHT")
        {
            PreviewModelAlignRight();
        }
        else if (type == "MODEL_ALIGN_CENTER_VERT")
        {
            PreviewModelAlignVCenter();
        }
        else if (type == "MODEL_ALIGN_CENTER_HORIZ")
        {
            PreviewModelAlignHCenter();
        }
        else if (type == "MODEL_DISTRIBUTE_HORIZ")
        {
            PreviewModelHDistribute();
        }
        else if (type == "MODEL_DISTRIBUTE_VERT")
        {
            PreviewModelVDistribute();
        }
        else
        {
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
    UnSelectAllModels();
    if (Notebook_Objects->GetPageText(Notebook_Objects->GetSelection()) == "Models") {
        editing_models = true;
    } else {
        editing_models = false;
    }
}
