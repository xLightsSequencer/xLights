#include "LayoutPanel.h"

//(*InternalHeaders(LayoutPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/splitter.h>
#include <wx/font.h>
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

#include <wx/clipbrd.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/tglbtn.h>
#include <wx/sstream.h>
#include <wx/artprov.h>

#include "ModelPreview.h"
#include "xLightsMain.h"
#include "DrawGLUtils.h"
#include "ChannelLayoutDialog.h"
#include "models/ModelGroup.h"
#include "ModelGroupPanel.h"

#include "../include/eye-16.xpm"
#include "../include/eye-16_gray.xpm"

#include "models/ModelImages.h"
#include "PreviewPane.h"

//(*IdInit(LayoutPanel)
const long LayoutPanel::ID_PANEL3 = wxNewId();
const long LayoutPanel::ID_PANEL2 = wxNewId();
const long LayoutPanel::ID_SPLITTERWINDOW1 = wxNewId();
const long LayoutPanel::ID_CHECKBOXOVERLAP = wxNewId();
const long LayoutPanel::ID_BUTTON_SAVE_PREVIEW = wxNewId();
const long LayoutPanel::ID_PANEL5 = wxNewId();
const long LayoutPanel::ID_STATICTEXT1 = wxNewId();
const long LayoutPanel::ID_CHOICE_PREVIEWS = wxNewId();
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
const long LayoutPanel::ID_PREVIEW_ALIGN = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_NODELAYOUT = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_EXPORTCSV = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_TOP = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_BOTTOM = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_LEFT = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_RIGHT = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_H_CENTER = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_V_CENTER = wxNewId();
const long LayoutPanel::ID_PREVIEW_DISTRIBUTE = wxNewId();
const long LayoutPanel::ID_PREVIEW_H_DISTRIBUTE = wxNewId();
const long LayoutPanel::ID_PREVIEW_V_DISTRIBUTE = wxNewId();
const long LayoutPanel::ID_MNU_DELETE_MODEL = wxNewId();
const long LayoutPanel::ID_MNU_DELETE_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_MNU_RENAME_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_MNU_ADD_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_PREVIEW_DELETE_ACTIVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_ADDPOINT = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_DELETEPOINT = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_ADDCURVE = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_DELCURVE = wxNewId();

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
#ifdef __WXOSX__
            SetBitmap(wxBitmap(imgDisabled, -1, bitmap.GetScaleFactor()));
#else
            SetBitmap(wxBitmap(imgDisabled));
#endif
        } else {
            SetBitmap(bitmap);
        }
    }
    unsigned int GetState() {
        return state;
    }
    const std::string &GetModelType() {
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
    m_over_handle(-1), selectedButton(nullptr), newModel(nullptr), selectedModel(nullptr),
    colSizesSet(false), updatingProperty(false), mNumGroups(0), mPropGridActive(true),
    mSelectedGroup(nullptr), currentLayoutGroup("Default"), pGrp(nullptr), backgroundFile(""), previewBackgroundScaled(false),
    previewBackgroundBrightness(100), m_polyline_active(false), ignore_next_event(false)
{
    background = nullptr;

    _lastXlightsModel = "";
    appearanceVisible = sizeVisible = stringPropsVisible = false;

	//(*Initialize(LayoutPanel)
	wxFlexGridSizer* LeftPanelSizer;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* PreviewGLSizer;
	wxFlexGridSizer* FlexGridSizerPreview;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizerPreview = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizerPreview->AddGrowableCol(0);
	FlexGridSizerPreview->AddGrowableRow(0);
	SplitterWindow2 = new wxSplitterWindow(this, ID_SPLITTERWINDOW2, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW2"));
	SplitterWindow2->SetMinSize(wxSize(10,10));
	SplitterWindow2->SetMinimumPaneSize(10);
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
	SecondPanel = new wxPanel(ModelSplitter, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	ModelSplitter->SplitHorizontally(FirstPanel, SecondPanel);
	LeftPanelSizer->Add(ModelSplitter, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 2);
	CheckBoxOverlap = new wxCheckBox(LeftPanel, ID_CHECKBOXOVERLAP, _("Overlap checks enabled"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOXOVERLAP"));
	CheckBoxOverlap->SetValue(false);
	LeftPanelSizer->Add(CheckBoxOverlap, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
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
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	ToolSizer = new wxFlexGridSizer(0, 10, 0, 0);
	FlexGridSizer1->Add(ToolSizer, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 3);
	StaticText1 = new wxStaticText(PreviewGLPanel, ID_STATICTEXT1, _("Preview:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	wxFont StaticText1Font(12,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText1->SetFont(StaticText1Font);
	FlexGridSizer1->Add(StaticText1, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 40);
	ChoiceLayoutGroups = new wxChoice(PreviewGLPanel, ID_CHOICE_PREVIEWS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PREVIEWS"));
	FlexGridSizer1->Add(ChoiceLayoutGroups, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	PreviewGLSizer->Add(FlexGridSizer1, 1, wxALL|wxALIGN_LEFT, 3);
	PreviewGLPanel->SetSizer(PreviewGLSizer);
	PreviewGLSizer->Fit(PreviewGLPanel);
	PreviewGLSizer->SetSizeHints(PreviewGLPanel);
	SplitterWindow2->SplitVertically(LeftPanel, PreviewGLPanel);
	SplitterWindow2->SetSashPosition(175);
	FlexGridSizerPreview->Add(SplitterWindow2, 1, wxALL|wxEXPAND, 1);
	SetSizer(FlexGridSizerPreview);
	FlexGridSizerPreview->Fit(this);
	FlexGridSizerPreview->SetSizeHints(this);

	Connect(ID_SPLITTERWINDOW1,wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED,(wxObjectEventFunction)&LayoutPanel::OnModelSplitterSashPosChanged);
	Connect(ID_CHECKBOXOVERLAP,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&LayoutPanel::OnCheckBoxOverlapClick);
	Connect(ID_BUTTON_SAVE_PREVIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LayoutPanel::OnButtonSavePreviewClick);
	Connect(ID_CHOICE_PREVIEWS,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&LayoutPanel::OnChoiceLayoutGroupsSelect);
	Connect(ID_SPLITTERWINDOW2,wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED,(wxObjectEventFunction)&LayoutPanel::OnSplitterWindowSashPosChanged);
	//*)

    modelPreview = new ModelPreview( (wxPanel*) PreviewGLPanel, xlights->PreviewModels, true);
    PreviewGLSizer->Add(modelPreview, 1, wxALL | wxEXPAND, 0);
    PreviewGLSizer->Fit(PreviewGLPanel);
    PreviewGLSizer->SetSizeHints(PreviewGLPanel);
    FlexGridSizerPreview->Fit(this);
    FlexGridSizerPreview->SetSizeHints(this);

    modelPreview->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&LayoutPanel::OnPreviewLeftDown,0,this);
    modelPreview->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&LayoutPanel::OnPreviewLeftUp,0,this);
    modelPreview->Connect(wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&LayoutPanel::OnPreviewRightDown,0,this);
    modelPreview->Connect(wxEVT_MOTION,(wxObjectEventFunction)&LayoutPanel::OnPreviewMouseMove,0,this);
    modelPreview->Connect(wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&LayoutPanel::OnPreviewMouseLeave,0,this);

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
    TreeListViewModels = CreateTreeListCtrl(wxTL_DEFAULT_STYLE);
    comparator.SetFrame(xlights);
    TreeListViewModels->SetItemComparator(&comparator);

    wxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
    sizer1->Add(TreeListViewModels, wxSizerFlags(2).Expand());
    FirstPanel->SetSizer(sizer1);
    sizer1->SetSizeHints(FirstPanel);

    ModelSplitter->ReplaceWindow(SecondPanel, propertyEditor);
    LeftPanelSizer->Fit(LeftPanel);
    LeftPanelSizer->SetSizeHints(LeftPanel);
    FlexGridSizerPreview->Fit(this);
    FlexGridSizerPreview->SetSizeHints(this);

    propertyEditor->Connect(wxEVT_PG_CHANGING, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridChanging,0,this);
    propertyEditor->Connect(wxEVT_PG_CHANGED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridChange,0,this);
    propertyEditor->Connect(wxEVT_PG_SELECTED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridSelection,0,this);
    propertyEditor->Connect(wxEVT_PG_ITEM_COLLAPSED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridItemCollapsed,0,this);
    propertyEditor->Connect(wxEVT_PG_ITEM_EXPANDED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridItemExpanded,0,this);
    propertyEditor->SetValidationFailureBehavior(wxPG_VFB_MARK_CELL | wxPG_VFB_BEEP);

    wxConfigBase* config = wxConfigBase::Get();
    int msp = config->Read("LayoutModelSplitterSash", -1);
    int sp = config->Read("LayoutMainSplitterSash", -1);
    if (sp != -1) {
        SplitterWindow2->SetSashGravity(0.0);
        SplitterWindow2->SetSashPosition(sp);
    }
    if (msp != -1) {
        ModelSplitter->SetSashGravity(0.0);
        ModelSplitter->SetSashPosition(msp);
    }

    ToolSizer->SetCols(15);
    AddModelButton("Arches", arches);
    AddModelButton("Candy Canes", canes);
    AddModelButton("Circle", circles);
    AddModelButton("Custom", custom);
    AddModelButton("DMX", dmx_xpm);
    AddModelButton("Icicles", icicles_xpm);
    AddModelButton("Matrix", matrix);
    AddModelButton("Single Line", singleline);
    AddModelButton("Poly Line", polyline);
    AddModelButton("Spinner", spinner);
    AddModelButton("Star", star);
    AddModelButton("Tree", tree);
    AddModelButton("Window Frame", frame);
    AddModelButton("Wreath", wreath);
    AddModelButton("Import Custom", import);

    modelPreview->Connect(wxID_CUT, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoCut,0,this);
    modelPreview->Connect(wxID_COPY, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoCopy,0,this);
    modelPreview->Connect(wxID_PASTE, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoPaste,0,this);
    modelPreview->Connect(wxID_UNDO, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoUndo,0,this);
    modelPreview->Connect(wxID_ANY, wxEVT_CHAR_HOOK, wxKeyEventHandler(LayoutPanel::OnCharHook),0,this);
    modelPreview->Connect(wxID_ANY, wxEVT_CHAR, wxKeyEventHandler(LayoutPanel::OnChar),0,this);

    ModelGroupWindow = new wxScrolledWindow(ModelSplitter);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    ModelGroupWindow->SetSizer(sizer);
    model_grp_panel = new ModelGroupPanel(ModelGroupWindow, xlights->AllModels, xlights);
    sizer->Add( model_grp_panel, 1, wxEXPAND | wxALL, 1 );
    ModelGroupWindow->SetScrollRate(5,5);
    ModelGroupWindow->Hide();

    mDefaultSaveBtnColor = ButtonSavePreview->GetBackgroundColour();

    Reset();
    
    TreeListViewModels->SetColumnWidth(0, wxCOL_WIDTH_AUTOSIZE);
    TreeListViewModels->SetColumnWidth(1, TreeListViewModels->WidthFor("1000000000000"));
    TreeListViewModels->SetColumnWidth(2, TreeListViewModels->WidthFor("1000000000000"));

}

void LayoutPanel::InitImageList()
{
    wxSize iconSize = wxArtProvider::GetSizeHint(wxART_LIST);
    if ( iconSize == wxDefaultSize )
        iconSize = wxSize(16, 16);

    m_imageList = new wxImageList(iconSize.x, iconSize.y);

    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_NORMAL_FILE")), wxART_LIST, iconSize));
    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_GROUP_CLOSED")), wxART_LIST, iconSize));
    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_GROUP_OPEN")), wxART_LIST, iconSize));
    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_ARCH_ICON")), wxART_LIST, iconSize));
    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_CANE_ICON")), wxART_LIST, iconSize));
    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_CIRCLE_ICON")), wxART_LIST, iconSize));
    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_CUSTOM_ICON")), wxART_LIST, iconSize));
    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_DMX_ICON")), wxART_LIST, iconSize));
    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_ICICLE_ICON")), wxART_LIST, iconSize));
    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_LINE_ICON")), wxART_LIST, iconSize));
    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_MATRIX_ICON")), wxART_LIST, iconSize));
    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_POLY_ICON")), wxART_LIST, iconSize));
    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_SPINNER_ICON")), wxART_LIST, iconSize));
    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_STAR_ICON")), wxART_LIST, iconSize));
    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_TREE_ICON")), wxART_LIST, iconSize));
    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_WINDOW_ICON")), wxART_LIST, iconSize));
    m_imageList->Add( wxArtProvider::GetIcon(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_WREATH_ICON")), wxART_LIST, iconSize));
}

wxTreeListCtrl* LayoutPanel::CreateTreeListCtrl(long style)
{
    wxTreeListCtrl* const
        tree = new wxTreeListCtrl(FirstPanel, ID_TREELISTVIEW_MODELS,
                                  wxDefaultPosition, wxDefaultSize,
                                  style, "ID_TREELISTVIEW_MODELS");
    tree->SetImageList(m_imageList);

    tree->AppendColumn("Model / Group",
                       wxCOL_WIDTH_AUTOSIZE,
                       wxALIGN_LEFT,
                       wxCOL_RESIZABLE | wxCOL_SORTABLE);
    tree->AppendColumn("Start Chan",
                       tree->WidthFor("1000000000000"),
                       wxALIGN_CENTER,
                       wxCOL_RESIZABLE | wxCOL_SORTABLE);
    tree->AppendColumn("End Chan",
                       tree->WidthFor("1000000000000"),
                       wxALIGN_CENTER,
                       wxCOL_RESIZABLE | wxCOL_SORTABLE);

    return tree;
}

void LayoutPanel::Reset()
{
    SetCurrentLayoutGroup(xlights->GetStoredLayoutGroup());
    ChoiceLayoutGroups->Clear();
    ChoiceLayoutGroups->Append("Default");
    ChoiceLayoutGroups->Append("All Models");
    ChoiceLayoutGroups->Append("Unassigned");
    for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); it++) {
        LayoutGroup* grp = (LayoutGroup*)(*it);
        ChoiceLayoutGroups->Append(grp->GetName());
    }
    ChoiceLayoutGroups->Append("<Create New Preview>");
    ChoiceLayoutGroups->SetSelection(0);
    for( int i = 0; i < ChoiceLayoutGroups->GetCount(); i++ )
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
        ButtonSavePreview->SetForegroundColour(wxColour(255,0,0));
        ButtonSavePreview->SetBackgroundColour(wxColour(255,0,0));
    } else {
        ButtonSavePreview->SetForegroundColour(*wxBLACK);
        ButtonSavePreview->SetBackgroundColour(mDefaultSaveBtnColor);
    }
#else
    if (dirty) {
        ButtonSavePreview->SetBackgroundColour(wxColour(255,108,108));
    } else {
        ButtonSavePreview->SetBackgroundColour(mDefaultSaveBtnColor);
    }
#endif
}

void LayoutPanel::AddModelButton(const std::string &type, const char *data[]) {
    wxImage image(data);
#ifdef __WXOSX__
    wxBitmap bitmap(image, -1, 2.0);
#else
    image.Rescale(24, 24, wxIMAGE_QUALITY_HIGH);
    wxBitmap bitmap(image);
#endif
    NewModelBitmapButton *button = new NewModelBitmapButton(PreviewGLPanel, bitmap, type);
    ToolSizer->Add(button, 1, wxALL, 0);
    buttons.push_back(button);
    Connect(button->GetId(), wxEVT_BUTTON, (wxObjectEventFunction)&LayoutPanel::OnNewModelTypeButtonClicked);
}

LayoutPanel::~LayoutPanel()
{
    if (background != nullptr) {
        delete background;
    }
    TreeListViewModels->DeleteAllItems();
    delete m_imageList;
	//(*Destroy(LayoutPanel)
	//*)
}

void LayoutPanel::OnPropertyGridChange(wxPropertyGridEvent& event) {
    wxString name = event.GetPropertyName();
    updatingProperty = true;
    if (name == "BkgBrightness") {
       if( currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned" ) {
            xlights->SetPreviewBackgroundBrightness(event.GetValue().GetLong());
         } else {
            pGrp->SetBackgroundBrightness(wxAtoi(event.GetValue().GetString()));
            modelPreview->SetBackgroundBrightness(wxAtoi(event.GetValue().GetString()));
            xlights->MarkEffectsFileDirty();
            UpdatePreview();
        }
    } else if (name == "BkgSizeWidth") {
        xlights->SetPreviewSize(event.GetValue().GetLong(), modelPreview->GetVirtualCanvasHeight());
        xlights->UpdateModelsList();
    } else if (name == "BkgSizeHeight") {
        xlights->SetPreviewSize(modelPreview->GetVirtualCanvasWidth(), event.GetValue().GetLong());
        xlights->UpdateModelsList();
    } else if (name == "BkgImage") {
        if( currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned" ) {
            xlights->SetPreviewBackgroundImage(event.GetValue().GetString());
        } else {
            pGrp->SetBackgroundImage(event.GetValue().GetString());
            modelPreview->SetbackgroundImage(event.GetValue().GetString());
            xlights->MarkEffectsFileDirty();
            UpdatePreview();
        }
    } else if (name == "BkgFill") {
       if( currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned" ) {
            xlights->SetPreviewBackgroundScaled(event.GetValue().GetBool());
         } else {
            pGrp->SetBackgroundScaled(wxAtoi(event.GetValue().GetString()));
            modelPreview->SetScaleBackgroundImage(wxAtoi(event.GetValue().GetString()));
            xlights->MarkEffectsFileDirty();
            UpdatePreview();
        }
    } else if (selectedModel != nullptr) {
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
            if (selectedModel->name != safename) {
                RenameModelInTree(selectedModel->name, safename);
                if (selectedModel->name == lastModelName) {
                    lastModelName = safename;
                }
                if (xlights->RenameModel(selectedModel->name, safename)) {
                    CallAfter(&LayoutPanel::UpdateModelList, true);
                }
            }
        } else {
            int i = selectedModel->OnPropertyGridChange(propertyEditor, event);
            if (i & 0x0001) {
                xlights->UpdatePreview();
            }
            if (i & 0x0002) {
                xlights->MarkEffectsFileDirty();
            }
            if (i & 0x0004) {
                CallAfter(&LayoutPanel::resetPropertyGrid);
            }
            if (i & 0x0008) {
                CallAfter(&LayoutPanel::refreshModelList);
            }
            if (i & 0x0010) {
                // Preview assignment change so model may not exist in current preview anymore
                CallAfter(&LayoutPanel::RefreshLayout);
            }
            if (i == 0) {
                printf("Did not handle %s   %s\n",
                       event.GetPropertyName().ToStdString().c_str(),
                       event.GetValue().GetString().ToStdString().c_str());
            }
        }
    }
    updatingProperty = false;
}
void LayoutPanel::OnPropertyGridChanging(wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    if (selectedModel != nullptr) {
        if ("ModelName" == name) {
            std::string safename = Model::SafeModelName(event.GetValue().GetString().ToStdString());
            // refuse clashing names or names with unsafe characters
            if (xlights->AllModels[safename] != nullptr || safename != event.GetValue().GetString().ToStdString()) {
                CreateUndoPoint("ModelName", selectedModel->name, safename);
                event.Veto();
            }
        } else {
            CreateUndoPoint("ModelProperty", selectedModel->name, name, event.GetProperty()->GetValue().GetString().ToStdString());
            selectedModel->OnPropertyGridChanging(propertyEditor, event);
        }
    } else {
        CreateUndoPoint("Background", "", name, event.GetProperty()->GetValue().GetString().ToStdString());
    }
}

void LayoutPanel::OnPropertyGridSelection(wxPropertyGridEvent& event) {
    if (selectedModel != nullptr) {
        if( selectedModel->GetDisplayAs() == "Poly Line" ) {
            int segment = selectedModel->OnPropertyGridSelection(propertyEditor, event);
            selectedModel->GetModelScreenLocation().SelectSegment(segment);
            UpdatePreview();
        }
    }
}

void LayoutPanel::OnPropertyGridItemCollapsed(wxPropertyGridEvent& event) {
    if (selectedModel != nullptr) {
        if( selectedModel->GetDisplayAs() == "Poly Line" ) {
            selectedModel->OnPropertyGridItemCollapsed(propertyEditor, event);
        }
    }
}

void LayoutPanel::OnPropertyGridItemExpanded(wxPropertyGridEvent& event) {
    if (selectedModel != nullptr) {
        if( selectedModel->GetDisplayAs() == "Poly Line" ) {
            selectedModel->OnPropertyGridItemExpanded(propertyEditor, event);
        }
    }
}

void LayoutPanel::RefreshLayout()
{
    xlights->UpdateModelsList();
    ShowPropGrid(true);
}

void LayoutPanel::UpdatePreview()
{
    SetDirtyHiLight(xlights->UnsavedRgbEffectsChanges);
    if(!modelPreview->StartDrawing(mPointSize)) return;
    modelPreview->Render();
    if(m_creating_bound_rect)
    {
        modelPreview->GetAccumulator().AddDottedLinesRect(m_bound_start_x,m_bound_start_y,m_bound_end_x,m_bound_end_y, xlYELLOW);
        modelPreview->GetAccumulator().Finish(GL_LINES);
    }
    modelPreview->EndDrawing();
}

void LayoutPanel::resetPropertyGrid() {
    clearPropGrid();
    if (selectedModel != nullptr) {
        SetupPropGrid(selectedModel);
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
    propertyEditor->Clear();
}

void LayoutPanel::refreshModelList() {
    std::string item_name = "";
    for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
          item.IsOk();
          item = TreeListViewModels->GetNextItem(item) )
    {
        item_name = TreeListViewModels->GetItemText(item);
        Model *model = xlights->AllModels[item_name];
        if( model != nullptr ) {
            std::string start_channel = model->GetModelXml()->GetAttribute("StartChannel").ToStdString();
            model->SetModelStartChan(start_channel);
            int end_channel = model->GetLastChannel()+1;
            if( model->GetDisplayAs() != "ModelGroup" ) {
                TreeListViewModels->SetItemText(item, Col_StartChan, start_channel);
                TreeListViewModels->SetItemText(item, Col_EndChan, wxString::Format(wxT("%i"),end_channel));
            }
        }
    }
}

void LayoutPanel::RenameModelInTree(const std::string old_name, const std::string new_name)
{
    std::string item_name = "";
    for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
          item.IsOk();
          item = TreeListViewModels->GetNextItem(item) )
    {
        item_name = TreeListViewModels->GetItemText(item);
        if( item_name == old_name ) {
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
        } else if( type == "Custom" ) {
            return Icon_Custom;
        } else if( type == "DMX" ) {
            return Icon_Dmx;
        } else if( type == "Icicles" ) {
            return Icon_Icicle;
        } else if( type == "Single Line" ) {
            return Icon_Line;
        } else if( type == "Matrix" ) {
            return Icon_Matrix;
        } else if( type == "Poly Line" ) {
            return Icon_Poly;
        } else if( type == "Spinner" ) {
            return Icon_Spinner;
        } else if( type == "Star" ) {
            return Icon_Star;
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


void LayoutPanel::AddModelToTree(Model *model, wxTreeListItem* parent) {
    std::string start_channel = model->GetModelXml()->GetAttribute("StartChannel").ToStdString();
    model->SetModelStartChan(start_channel);
    int end_channel = model->GetLastChannel()+1;

    wxTreeListItem item = TreeListViewModels->AppendItem(*parent, model->name,
                                                         GetModelTreeIcon(model, false),
                                                         GetModelTreeIcon(model, true));
    if( model->GetDisplayAs() != "ModelGroup" ) {
        TreeListViewModels->SetItemText(item, Col_StartChan, start_channel);
        TreeListViewModels->SetItemText(item, Col_EndChan, wxString::Format(wxT("%i"),end_channel));
    }

    if( model->GetDisplayAs() == "ModelGroup" ) {
        ModelGroup *grp = (ModelGroup*)model;
        for (auto it = grp->ModelNames().begin(); it != grp->ModelNames().end(); it++) {
            Model *m = xlights->AllModels[*it];
            AddModelToTree(m, &item);
        }
    }
}

void LayoutPanel::UpdateModelList(bool full_refresh) {
    UnSelectAllModels();

    std::vector<Model *> models;
    std::vector<Model *> dummy_models;

    // Update all the custom previews
    for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); it++) {
        LayoutGroup* grp = (LayoutGroup*)(*it);
        dummy_models.clear();
        if( grp->GetName() == currentLayoutGroup ) {
            UpdateModelsForPreview( currentLayoutGroup, grp, models, true );
        } else {
            UpdateModelsForPreview( grp->GetName(), grp, dummy_models, false );
        }
    }

    // update the Layout tab preview for default options
    if( currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned" ) {
        UpdateModelsForPreview( currentLayoutGroup, nullptr, models, true );
    }

    if( full_refresh ) {

        TreeListViewModels->Freeze();
        //turn off the colum width auto-resize.  Makes it REALLY slow to populate the tree
        TreeListViewModels->SetColumnWidth(0, 10);

        TreeListViewModels->DeleteAllItems();
        // add all the model groups
        wxTreeListItem root = TreeListViewModels->GetRootItem();
        for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); it++) {
            Model *model = it->second;
            if (model->GetDisplayAs() == "ModelGroup") {
                if (currentLayoutGroup == "All Models" || model->GetLayoutGroup() == currentLayoutGroup
                    || (model->GetLayoutGroup() == "All Previews" && currentLayoutGroup != "Unassigned")) {
                    AddModelToTree(model, &root);
                }
            }
        }

        // add all the models
        for (auto it = models.begin(); it != models.end(); it++) {
            Model *model = *it;
            if (model->GetDisplayAs() != "ModelGroup") {
                AddModelToTree(model, &root);
            }
        }
 
        unsigned int mc = xlights->GetMaxNumChannels();
        wxString mcs = wxString::Format("%ld", mc);
        int sz = TreeListViewModels->WidthFor(mcs);
        int sz2 = TreeListViewModels->WidthFor("Start Chan");
        if (sz2 > sz) {
            sz = sz2;
        }
        
        TreeListViewModels->SetColumnWidth(2, sz);
        TreeListViewModels->SetColumnWidth(1, sz);
        TreeListViewModels->SetColumnWidth(0, wxCOL_WIDTH_AUTOSIZE);
        TreeListViewModels->Thaw();
    }

    modelPreview->SetModels(models);
    UpdatePreview();
}

void LayoutPanel::UpdateModelsForPreview(const std::string &group, LayoutGroup* layout_grp, std::vector<Model *> &prev_models, bool filtering)
{
    std::set<std::string> modelsAdded;

    for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); it++) {
        Model *model = it->second;
        if (model->GetDisplayAs() != "ModelGroup") {
            if (group == "All Models" || model->GetLayoutGroup() == group || (model->GetLayoutGroup() == "All Previews" && group != "Unassigned")) {
                prev_models.push_back(model);
                modelsAdded.insert(model->name);
            }
        }
    }

    // add in any models that were not in preview but belong to a group that is in the preview
    std::string selected_group_name = "";
    if( mSelectedGroup != nullptr && filtering) {
        selected_group_name = TreeListViewModels->GetItemText(mSelectedGroup);
    }

    for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); it++) {
        Model *model = it->second;
        bool mark_selected = false;
        if( (mSelectedGroup != nullptr) && filtering && (model->name == selected_group_name) ) {
            mark_selected = true;
        }
        if (model->GetDisplayAs() == "ModelGroup") {
            ModelGroup *grp = (ModelGroup*)(model);
            if (group == "All Models" || model->GetLayoutGroup() == group || (model->GetLayoutGroup() == "All Previews" && group != "Unassigned")) {
                for (auto it = grp->ModelNames().begin(); it != grp->ModelNames().end(); it++) {
                    Model *m = xlights->AllModels[*it];
                    if( mark_selected ) {
                        m->GroupSelected = true;
                    }
                    if (modelsAdded.find(*it) == modelsAdded.end()) {
                        if (m != nullptr) {
                            modelsAdded.insert(*it);
                            prev_models.push_back(m);
                        }
                    }
                }
            }
        }
    }

    // only run this for layout group previews
    if( layout_grp != nullptr ) {
        layout_grp->SetModels(prev_models);
        ModelPreview* preview = layout_grp->GetModelPreview();
        if( layout_grp->GetPreviewCreated() ) {
            preview->SetModels(layout_grp->GetModels());
            if( preview->GetActive() ) {
                preview->Refresh();
                preview->Update();
            }
        }
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
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        modelPreview->GetModels()[i]->Selected = false;
        modelPreview->GetModels()[i]->GroupSelected = false;
        modelPreview->GetModels()[i]->SelectHandle(-1);
    }
    UpdatePreview();
    selectedModel = nullptr;

    if (!updatingProperty && addBkgProps) {
        propertyEditor->Freeze();
        clearPropGrid();

        GetBackgroundImageForSelectedPreview();       // don't need return value....just let it set local variable previewBackgroundFile
        GetBackgroundScaledForSelectedPreview();      // don't need return value....just let it set local variable previewBackgroundScaled
        GetBackgroundBrightnessForSelectedPreview();  // don't need return value....just let it set local variable previewBackgroundBrightness

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
        if( currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned" ) {
            wxPGProperty* prop = propertyEditor->Append(new wxUIntProperty("Width", "BkgSizeWidth", modelPreview->GetVirtualCanvasWidth()));
            prop->SetAttribute("Min", 0);
            prop->SetAttribute("Max", 4096);
            prop->SetEditor("SpinCtrl");
            prop = propertyEditor->Append(new wxUIntProperty("Height", "BkgSizeHeight", modelPreview->GetVirtualCanvasHeight()));
            prop->SetAttribute("Min", 0);
            prop->SetAttribute("Max", 4096);
            prop->SetEditor("SpinCtrl");
        }
        wxPGProperty* prop = propertyEditor->Append(new wxStringProperty("Brightness", "BkgBrightness", wxString::Format("%d", previewBackgroundBrightness)));
        prop->SetAttribute("Min", 0);
        prop->SetAttribute("Max", 100);
        prop->SetEditor("SpinCtrl");
        propertyEditor->Thaw();
    }
}

void LayoutPanel::SetupPropGrid(Model *model) {
    propertyEditor->Freeze();
    clearPropGrid();

    wxPGProperty* prop = propertyEditor->Append(new wxStringProperty("Name", "ModelName", model->name));

    model->AddProperties(propertyEditor);

    wxPGProperty *p2 = propertyEditor->Append(new wxPropertyCategory("Size/Location", "ModelSize"));
    p2->GetCell(0).SetFgCol(*wxBLACK);

    model->AddSizeLocationProperties(propertyEditor);
    if (!sizeVisible) {
        propertyEditor->Collapse(p2);
    }
    if (!appearanceVisible) {
        prop = propertyEditor->GetPropertyByName("ModelAppearance");
        if (prop != nullptr) {
            propertyEditor->Collapse(prop);
        }
    }
    if (!stringPropsVisible) {
        prop = propertyEditor->GetPropertyByName("ModelStringProperties");
        if (prop != nullptr) {
            propertyEditor->Collapse(prop);
        }
    }
    propertyEditor->Thaw();
}

void LayoutPanel::SelectModel(const std::string & name, bool highlight_tree)
{
    modelPreview->SetFocus();
    int foundStart = 0;
    int foundEnd = 0;

    Model *m = xlights->AllModels[name];
    if( m != nullptr ) {
        m->Selected = true;
        std::string item_name = "";
        if( highlight_tree ) {
            for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
                  item.IsOk();
                  item = TreeListViewModels->GetNextSibling(item) )
            {
                item_name = TreeListViewModels->GetItemText(item);
                if( item_name == m->name ) {
                    TreeListViewModels->Select(item);
                    TreeListViewModels->EnsureVisible(item);
                    break;
                }
            }
        }
        if (CheckBoxOverlap->GetValue()== true) {
            foundStart = m->GetNumberFromChannelString(m->ModelStartChannel);
            foundEnd = m->GetLastChannel();
        }
        SetupPropGrid(m);
    } else {
        propertyEditor->Freeze();
        clearPropGrid();
        propertyEditor->Thaw();
    }

    selectedModel = m;
    if (CheckBoxOverlap->GetValue()) {
        std::string item_name = "";
        for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
              item.IsOk();
              item = TreeListViewModels->GetNextSibling(item) )
        {
            item_name = TreeListViewModels->GetItemText(item);
            Model *m = xlights->AllModels[item_name];
            if( m != nullptr ) {
                int startChan = m->GetNumberFromChannelString(m->ModelStartChannel);
                int endChan = m->GetLastChannel();
                if ((startChan >= foundStart) && (endChan <= foundEnd)) {
                    m->Overlapping = true;
                } else if ((startChan >= foundStart) && (startChan <= foundEnd)) {
                    m->Overlapping = true;
                } else if ((endChan >= foundStart) && (endChan <= foundEnd)) {
                    m->Overlapping = true;
                } else {
                    m->Overlapping = false;
                }
            }
        }
    }
    UpdatePreview();
}


void LayoutPanel::OnCheckBoxOverlapClick(wxCommandEvent& event)
{
    if (CheckBoxOverlap->GetValue() == false) {
        std::string item_name = "";
        for ( wxTreeListItem item = TreeListViewModels->GetFirstItem();
              item.IsOk();
              item = TreeListViewModels->GetNextSibling(item) )
        {
            item_name = TreeListViewModels->GetItemText(item);
            Model *model = xlights->AllModels[item_name];
            if( model != nullptr ) {
                if( model->GetDisplayAs() != "ModelGroup" ) {
                    model->Overlapping = false;
                }
            }
        }
    }
}

void LayoutPanel::OnButtonSavePreviewClick(wxCommandEvent& event)
{
    // update xml with offsets and scale
    for (size_t i=0; i < modelPreview->GetModels().size(); i++)
    {
        modelPreview->GetModels()[i]->UpdateXmlWithScale();
    }
    xlights->SaveEffectsFile();
    xlights->SetStatusText(_("Preview layout saved"));
    SetDirtyHiLight(false);
}

int LayoutPanel::ModelListComparator::SortElementsFunction(wxTreeListCtrl *treelist, wxTreeListItem item1, wxTreeListItem item2, unsigned sortColumn)
{
    std::string item_name1 = "";
    std::string item_name2 = "";
    item_name1 = treelist->GetItemText(item1);
    item_name2 = treelist->GetItemText(item2);
    Model* a = xlights->AllModels[item_name1];
    Model* b = xlights->AllModels[item_name2];

    if( a == nullptr || b == nullptr ) return 0;

    if( a->GetDisplayAs() == "ModelGroup" ) {
        if( b->GetDisplayAs() == "ModelGroup" ) {
            return 0;
        } else {
            return -1;
        }
    } else if( b->GetDisplayAs() == "ModelGroup" ) {
        return 1;
    }

    if (sortColumn == 1) {
        if( a->GetDisplayAs() == "ModelGroup" ) {
            if( b->GetDisplayAs() == "ModelGroup" ) {
                return 0;
            } else {
                return 1;
            }
        } else if( b->GetDisplayAs() == "ModelGroup" ) {
            return -1;
        }
        int ia = a->GetNumberFromChannelString(a->ModelStartChannel);
        int ib = b->GetNumberFromChannelString(b->ModelStartChannel);
        if (ia > ib)
            return 1;
        if (ia < ib)
            return -1;
        return 0;
    } else if (sortColumn == 2) {
        int ia = a->GetLastChannel();
        int ib = b->GetLastChannel();
        if (ia > ib)
            return 1;
        if (ia < ib)
            return -1;
        return 0;
    } else {
        return strcasecmp(b->name.c_str(), a->name.c_str());
    }
    return 0;
}

int LayoutPanel::ModelListComparator::Compare(wxTreeListCtrl *treelist, unsigned column, wxTreeListItem first, wxTreeListItem second)
{
    return SortElementsFunction( treelist, first, second, column);
}

int LayoutPanel::FindModelsClicked(int x,int y,std::vector<int> &found)
{
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->HitTest(modelPreview,x,y))
        {
            found.push_back(i);
        }
    }
    return found.size();
}



bool LayoutPanel::SelectSingleModel(int x,int y)
{
    std::vector<int> found;
    int modelCount = FindModelsClicked(x,y,found);
    if (modelCount==0)
    {
        TreeListViewModels->UnselectAll();
        return false;
    }
    else if(modelCount==1)
    {
        SelectModel(modelPreview->GetModels()[found[0]]->name);
        mHitTestNextSelectModelIndex = 0;
        return true;
    }
    else if (modelCount>1)
    {
        for(int i=0;i<modelCount;i++)
        {
            if(mHitTestNextSelectModelIndex==i)
            {
                SelectModel(modelPreview->GetModels()[found[i]]->name);
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
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->IsContained(modelPreview,m_bound_start_x,m_bound_start_y,
                                         m_bound_end_x,m_bound_end_y))
        {
            modelPreview->GetModels()[i]->GroupSelected = true;
        }
    }
}

bool LayoutPanel::SelectMultipleModels(int x,int y)
{
    std::vector<int> found;
    int modelCount = FindModelsClicked(x,y,found);
    if (modelCount==0)
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
            modelPreview->GetModels()[found[0]]->GroupSelected = false;
        }
        else if (modelPreview->GetModels()[found[0]]->GroupSelected)
        {
            SetSelectedModelToGroupSelected();
            modelPreview->GetModels()[found[0]]->Selected = true;
            SelectModel(modelPreview->GetModels()[found[0]]->name);
        }
        else
        {
            modelPreview->GetModels()[found[0]]->GroupSelected = true;
        }
        UpdatePreview();
        return true;
    }
    return false;
}

void LayoutPanel::SetSelectedModelToGroupSelected()
{
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->Selected)
        {
            modelPreview->GetModels()[i]->Selected = false;
            modelPreview->GetModels()[i]->GroupSelected = true;
        }
    }
}

void LayoutPanel::OnPreviewLeftDown(wxMouseEvent& event)
{
    if( m_polyline_active )
    {
        Model *m = newModel;
        int y = event.GetY();
        y = modelPreview->GetVirtualCanvasHeight() - y;
        m->AddHandle(modelPreview, event.GetPosition().x, y);
        m->UpdateXmlWithScale();
        m->InitModel();
        xlights->MarkEffectsFileDirty();
        UpdatePreview();
        m_over_handle++;
        return;
    }


    ShowPropGrid(true);
    modelPreview->SetFocus();
    int y = event.GetY();
    if (event.ControlDown())
    {
        ignore_next_event = true;
        SelectMultipleModels(event.GetX(),y);
        m_dragging = true;
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
    }
    else if (event.ShiftDown())
    {
        m_creating_bound_rect = true;
        m_bound_start_x = event.GetX();
        m_bound_start_y = modelPreview->GetVirtualCanvasHeight() - y;
    }
    else if (m_over_handle != -1)
    {
        m_moving_handle = true;
        if( selectedModel != nullptr ) {
            selectedModel->SelectHandle(m_over_handle);
            UpdatePreview();
        }
    }
    else if (selectedButton != nullptr)
    {
        //create a new model
        int wi, ht;
        modelPreview->GetVirtualCanvasSize(wi,ht);
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
        int cy = modelPreview->GetVirtualCanvasHeight() - m_previous_mouse_y;
        if (m_previous_mouse_x < wi
            && cy < ht
            && cy >= 0) {

            m_moving_handle = true;
            m_creating_bound_rect = false;
            const std::string& model_type = selectedButton->GetModelType();
            newModel = CreateNewModel(model_type);
            newModel->SetLayoutGroup(currentLayoutGroup);

            if (newModel != nullptr) {
                if( model_type == "Poly Line" ) {
                    m_polyline_active = true;
                }

                newModel->Selected = true;
                if (wi > 0 && ht > 0)
                {
                    modelPreview->SetCursor(newModel->InitializeLocation(m_over_handle, event.GetPosition().x,modelPreview->GetVirtualCanvasHeight() - y));
                    newModel->UpdateXmlWithScale();
                }
                lastModelName = newModel->name;
                modelPreview->GetModels().push_back(newModel);
            }
        }
    }
    else
    {
        m_moving_handle = false;
        m_creating_bound_rect = false;

        if(!event.wxKeyboardState::ControlDown())
        {
            UnSelectAllModels();
        }

        if(SelectSingleModel(event.GetX(),y))
        {
            m_dragging = true;
            m_previous_mouse_x = event.GetX();
            m_previous_mouse_y = event.GetY();
            xlights->SetStatusText(wxString::Format("x=%d y=%d",m_previous_mouse_x,m_previous_mouse_y));
        }
    }
}

void LayoutPanel::OnPreviewLeftUp(wxMouseEvent& event)
{
    if( m_polyline_active ) return;

    int y = event.GetY();

    if(m_creating_bound_rect)
    {
        m_bound_end_x = event.GetPosition().x;
        m_bound_end_y = modelPreview->GetVirtualCanvasHeight() - y;
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
        m->DeleteHandle(m_over_handle);
    }
    m_moving_handle = false;
    m_dragging = false;
    m_polyline_active = false;

    if (newModel != nullptr) {
        if (selectedButton->GetModelType() == "Import Custom")
        {
            float min_x = (float)(newModel->GetModelScreenLocation().GetLeft()) / (float)(newModel->GetModelScreenLocation().previewW);
            float max_x = (float)(newModel->GetModelScreenLocation().GetRight()) / (float)(newModel->GetModelScreenLocation().previewW);
            float min_y = (float)(newModel->GetModelScreenLocation().GetBottom()) / (float)(newModel->GetModelScreenLocation().previewH);
            float max_y = (float)(newModel->GetModelScreenLocation().GetTop()) / (float)(newModel->GetModelScreenLocation().previewH);
            bool cancelled = false;
            newModel = Model::GetXlightsModel(newModel, _lastXlightsModel, xlights, cancelled);
            if( cancelled ) {
                newModel = nullptr;
                m_over_handle = -1;
                modelPreview->SetCursor(wxCURSOR_DEFAULT);
                if (selectedButton != nullptr) {
                    selectedButton->SetState(0);
                    selectedButton = nullptr;
                }
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

        m_over_handle = -1;
        modelPreview->SetCursor(wxCURSOR_DEFAULT);
        if (selectedButton->GetState() == 1) {
            std::string name = newModel->name;
            newModel = nullptr;
            if (selectedButton != nullptr) {
                selectedButton->SetState(0);
                selectedButton = nullptr;
            }
            xlights->UpdateModelsList();
            UpdatePreview();
            SelectModel(name);
        } else {
            newModel = nullptr;
            xlights->UpdateModelsList();
            UpdatePreview();
        }
    }
}

void LayoutPanel::OnPreviewMouseLeave(wxMouseEvent& event)
{
    m_dragging = false;
}

void LayoutPanel::OnPreviewMouseMove(wxMouseEvent& event)
{
    int y = event.GetY();
    int wi,ht;

    if (m_creating_bound_rect)
    {
        m_bound_end_x = event.GetPosition().x;
        m_bound_end_y = modelPreview->GetVirtualCanvasHeight() - y;
        UpdatePreview();
        return;
    }

    Model *m = newModel;
    if (m == nullptr) {
        m = selectedModel;
        if( m == nullptr ) return;
    }

    if(m_moving_handle)
    {
        if (m != newModel) {
            CreateUndoPoint("SingleModel", m->name, std::to_string(m_over_handle));
        }
        y = modelPreview->GetVirtualCanvasHeight() - y;
        m->MoveHandle(modelPreview,m_over_handle, event.ShiftDown(), event.GetPosition().x, y);
        SetupPropGrid(m);
        xlights->MarkEffectsFileDirty();
        UpdatePreview();
    }
    else if (m_dragging && event.Dragging())
    {
        double delta_x = event.GetPosition().x - m_previous_mouse_x;
        double delta_y = -(event.GetPosition().y - m_previous_mouse_y);
        modelPreview->GetVirtualCanvasSize(wi, ht);
        if (wi > 0 && ht > 0)
        {
            for (size_t i=0; i<modelPreview->GetModels().size(); i++)
            {
                if(modelPreview->GetModels()[i]->Selected || modelPreview->GetModels()[i]->GroupSelected)
                {
                    CreateUndoPoint("SingleModel", m->name, "location");

                    modelPreview->GetModels()[i]->AddOffset(delta_x/wi, delta_y/ht);
                    modelPreview->GetModels()[i]->UpdateXmlWithScale();
                    SetupPropGrid(modelPreview->GetModels()[i]);
                    xlights->MarkEffectsFileDirty();
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
            modelPreview->SetCursor(m->CheckIfOverHandles(m_over_handle,event.GetPosition().x,modelPreview->GetVirtualCanvasHeight() - y));
        }
    }
}

void LayoutPanel::OnPreviewRightDown(wxMouseEvent& event)
{
    modelPreview->SetFocus();

    wxMenu mnu;
    int selectedModelCnt = ModelsSelectedCount();
    if (selectedModelCnt > 1)
    {
        wxMenu* mnuAlign = new wxMenu();
        wxMenu* mnuDistribute = new wxMenu();
        mnuAlign->Append(ID_PREVIEW_ALIGN_TOP,"Top");
        mnuAlign->Append(ID_PREVIEW_ALIGN_BOTTOM,"Bottom");
        mnuAlign->Append(ID_PREVIEW_ALIGN_LEFT,"Left");
        mnuAlign->Append(ID_PREVIEW_ALIGN_RIGHT,"Right");
        mnuAlign->Append(ID_PREVIEW_ALIGN_H_CENTER,"Horizontal Center");
        mnuAlign->Append(ID_PREVIEW_ALIGN_V_CENTER,"Vertical Center");
        mnuAlign->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, NULL, this);

        mnuDistribute->Append(ID_PREVIEW_H_DISTRIBUTE,"Horizontal");
        mnuDistribute->Append(ID_PREVIEW_V_DISTRIBUTE,"Vertical");
        mnuDistribute->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, NULL, this);

        mnu.Append(ID_PREVIEW_ALIGN, 	        "Align", mnuAlign,"");
        mnu.Append(ID_PREVIEW_DISTRIBUTE,"Distribute", mnuDistribute,"");
        mnu.AppendSeparator();
    }
    if (selectedModelCnt > 0) {
        Model* model = selectedModel;
        if (model != nullptr)
        {
            bool need_sep = false;
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
            if( (sel_hdl != -1) && (sel_hdl < 0x4000) && (model->GetNumHandles() > 2) ) {
                mnu.Append(ID_PREVIEW_MODEL_DELETEPOINT,"Delete Point");
                need_sep = true;
            }
            if( need_sep ) {
                mnu.AppendSeparator();
            }
        }
        mnu.Append(ID_PREVIEW_MODEL_NODELAYOUT,"Node Layout");
        mnu.Append(ID_PREVIEW_MODEL_EXPORTCSV,"Export CSV");
        if (model != nullptr)
        {
            if (model->SupportsXlightsModel())
            {
                mnu.Append(ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL, "Export xLights Model");
            }
        }
    }

    if( currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned" ) {
        if (selectedModelCnt > 0) {
            mnu.AppendSeparator();
        }
        mnu.Append(ID_PREVIEW_DELETE_ACTIVE,"Delete this Preview");
    }

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, NULL, this);
    PopupMenu(&mnu);
    modelPreview->SetFocus();
}


void LayoutPanel::OnPreviewModelPopup(wxCommandEvent &event)
{
    if (event.GetId() == ID_PREVIEW_ALIGN_TOP)
    {
        PreviewModelAlignTops();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_BOTTOM)
    {
        PreviewModelAlignBottoms();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_LEFT)
    {
        PreviewModelAlignLeft();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_RIGHT)
    {
        PreviewModelAlignRight();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_H_CENTER)
    {
        PreviewModelAlignHCenter();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_V_CENTER)
    {
        PreviewModelAlignVCenter();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_NODELAYOUT)
    {
        Model* md=selectedModel;
        if( md == nullptr ) return;
        wxString html=md->ChannelLayoutHtml();
        ChannelLayoutDialog dialog(this);
        dialog.SetHtmlSource(html);
        dialog.ShowModal();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTCSV)
    {
        ExportModel();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL)
    {
        Model* md=selectedModel;
        if( md == nullptr ) return;
        md->ExportXlightsModel();
    }
    else if (event.GetId() == ID_PREVIEW_DELETE_ACTIVE)
    {
        DeleteCurrentPreview();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_ADDPOINT)
    {
        Model* md=selectedModel;
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
        Model* md=selectedModel;
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
        Model* md=selectedModel;
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
        Model* md=selectedModel;
        if( md == nullptr ) return;
        int seg = md->GetSelectedSegment();
        CreateUndoPoint("SingleModel", md->name, std::to_string(seg+0x1000));
        md->SetCurve(seg, false);
        md->UpdateXmlWithScale();
        md->InitModel();
        UpdatePreview();
    }

}


#define retmsg(msg)  \
{ \
wxMessageBox(msg, _("Export Error")); \
return; \
}

void LayoutPanel::ExportModel()
{
#if 0
    model name
    display as
    type of strings
#strings
#nodes
    start channel
    start node = (channel+2)/3;
    my display
    brightness
#endif // 0
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, wxEmptyString, wxEmptyString, "Export files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    //    if (filename.IsEmpty()) retmsg(wxString("Please choose an output file name."));
    if (filename.IsEmpty()) return;

    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) retmsg(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()));
    f.Write(_("Model_name, Display_as, String_type, String_count, Node_count, Start_channel, Start_node, My_display, Brightness+-\n"));

    Model* model=selectedModel;
    if( model == nullptr ) return;
    wxString stch = model->GetModelXml()->GetAttribute("StartChannel", wxString::Format("%d?", model->NodeStartChannel(0) + 1)); //NOTE: value coming from model is probably not what is wanted, so show the base ch# instead
    f.Write(wxString::Format("\"%s\", \"%s\", \"%s\", %d, %d, %s, %d, %s\n", model->name, model->GetDisplayAs(), model->GetStringType(), model->GetNodeCount() / model->NodesPerString(), model->GetNodeCount(), stch, /*WRONG:*/ model->NodeStartChannel(0) / model->NodesPerString() + 1, model->GetLayoutGroup()));
    f.Close();
}

void LayoutPanel::PreviewModelAlignTops()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    int top = modelPreview->GetModels()[selectedindex]->GetTop(modelPreview);
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetTop(modelPreview,top);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignBottoms()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    int bottom = modelPreview->GetModels()[selectedindex]->GetBottom(modelPreview);
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetBottom(modelPreview,bottom);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignLeft()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    int left = modelPreview->GetModels()[selectedindex]->GetLeft(modelPreview);
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetLeft(modelPreview,left);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignRight()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    int right = modelPreview->GetModels()[selectedindex]->GetRight(modelPreview);
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetRight(modelPreview,right);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignHCenter()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float center = modelPreview->GetModels()[selectedindex]->GetHcenterOffset();
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetHcenterOffset(center);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignVCenter()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float center = modelPreview->GetModels()[selectedindex]->GetVcenterOffset();
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetVcenterOffset(center);
        }
    }
    UpdatePreview();
}


int LayoutPanel::GetSelectedModelIndex()
{
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->Selected)
        {
            return i;
        }
    }
    return -1;
}
int LayoutPanel::ModelsSelectedCount()
{
    int selectedModelCount=0;
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->Selected || modelPreview->GetModels()[i]->GroupSelected)
        {
            selectedModelCount++;
        }
    }
    return selectedModelCount;
}

void LayoutPanel::OnModelSplitterSashPosChanged(wxSplitterEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("LayoutModelSplitterSash", event.GetSashPosition());
}

void LayoutPanel::OnSplitterWindowSashPosChanged(wxSplitterEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("LayoutMainSplitterSash", event.GetSashPosition());
}

void LayoutPanel::OnNewModelTypeButtonClicked(wxCommandEvent& event) {
    for (auto it = buttons.begin(); it != buttons.end(); it++) {
        if (event.GetId() == (*it)->GetId()) {
            int state = (*it)->GetState();
            (*it)->SetState(state + 1);
            if ((*it)->GetState()) {
                selectedButton = (*it);
                UnSelectAllModels();
                modelPreview->SetFocus();
            } else {
                selectedButton = nullptr;
                _lastXlightsModel = "";
            }
        } else if ((*it)->GetState()) {
            (*it)->SetState(0);
        }
    }
}

Model *LayoutPanel::CreateNewModel(const std::string &type) {
    std::string t = type;
    if (t == "Import Custom")
    {
        t = "Custom";
    }
    std::string startChannel = "1";
    if (xlights->AllModels[lastModelName] != nullptr) {
        startChannel = ">" + lastModelName + ":1";
    }
    Model* m = xlights->AllModels.CreateDefaultModel(t, startChannel);

    return m;
}
void LayoutPanel::OnChar(wxKeyEvent& event) {

    wxChar uc = event.GetKeyCode();
    switch (uc) {
        case WXK_UP:
        case WXK_DOWN:
        case WXK_LEFT:
        case WXK_RIGHT:
            if (selectedModel != nullptr) {
                int wi, ht;
                modelPreview->GetVirtualCanvasSize(wi, ht);
                float xpct = 0;
                float ypct = 0;
                if (uc == WXK_UP) {
                    ypct = 1.0 / ht;
                } else if (uc == WXK_DOWN) {
                    ypct = -1.0 / ht;
                } else if (uc == WXK_LEFT) {
                    xpct = -1.0 / wi;
                } else if (uc == WXK_RIGHT) {
                    xpct = 1.0 / wi;
                }
                CreateUndoPoint("SingleModel", selectedModel->name, "location");
                selectedModel->AddOffset(xpct, ypct);
                selectedModel->UpdateXmlWithScale();
                SetupPropGrid(selectedModel);
                xlights->MarkEffectsFileDirty();
                UpdatePreview();
            }
            break;
        default:
            break;
    }
}
void LayoutPanel::OnCharHook(wxKeyEvent& event) {

    wxChar uc = event.GetKeyCode();

    switch(uc) {
#ifdef __WXMSW__
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
#ifdef __WXMSW__
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
            if (selectedModel != nullptr) {
                int wi, ht;
                modelPreview->GetVirtualCanvasSize(wi, ht);
                float xpct = 0;
                float ypct = 0;
                if (uc == WXK_UP) {
                    ypct = 1.0 / ht;
                } else if (uc == WXK_DOWN) {
                    ypct = -1.0 / ht;
                } else if (uc == WXK_LEFT) {
                    xpct = -1.0 / wi;
                } else if (uc == WXK_RIGHT) {
                    xpct = 1.0 / wi;
                }
                CreateUndoPoint("SingleModel", selectedModel->name, "location");
                selectedModel->AddOffset(xpct, ypct);
                selectedModel->UpdateXmlWithScale();
                SetupPropGrid(selectedModel);
                xlights->MarkEffectsFileDirty();
                UpdatePreview();
            }
            break;

        case WXK_ESCAPE:
            FinalizeModel();
            break;

        default:
            event.Skip();
            break;
    }
}
void LayoutPanel::DeleteSelectedModel() {
    if( selectedModel != nullptr ) {
        CreateUndoPoint("All", selectedModel->name);
        xlights->AllModels.Delete(selectedModel->name);
        selectedModel = nullptr;
        xlights->UpdateModelsList();
        xlights->MarkEffectsFileDirty();
    }
}

void LayoutPanel::DoCopy(wxCommandEvent& event) {
    if (!modelPreview->HasFocus()) {
        event.Skip();
    } else if (selectedModel != nullptr) {
        wxString copy_data;
        wxXmlDocument doc;
        wxXmlNode *parent = selectedModel->GetModelXml()->GetParent();
        wxXmlNode *next = selectedModel->GetModelXml()->GetNext();
        parent->RemoveChild(selectedModel->GetModelXml());
        doc.SetRoot(selectedModel->GetModelXml());

        wxStringOutputStream stream;
        doc.Save(stream);
        copy_data = stream.GetString();
        doc.DetachRoot();

        parent->InsertChild(selectedModel->GetModelXml(), next);

        if (!copy_data.IsEmpty() && wxTheClipboard->Open()) {
            if (!wxTheClipboard->SetData(new wxTextDataObject(copy_data))) {
                wxMessageBox(_("Unable to copy data to clipboard."), _("Error"));
            }
            wxTheClipboard->Close();
        }
    }
}
void LayoutPanel::DoCut(wxCommandEvent& event) {
    if (!modelPreview->HasFocus()) {
        event.Skip();
    } else if (selectedModel != nullptr) {
        DoCopy(event);
        DeleteSelectedModel();
    }
}
void LayoutPanel::DoPaste(wxCommandEvent& event) {
    if (!modelPreview->HasFocus()) {
        event.Skip();
    } else {
        if (wxTheClipboard->Open()) {
            CreateUndoPoint("All", selectedModel == nullptr ? "" : selectedModel->name);
            wxTextDataObject data;
            wxTheClipboard->GetData(data);
            wxStringInputStream in(data.GetText());
            wxXmlDocument doc(in);

            wxXmlNode *nd = doc.GetRoot();
            if (nd != nullptr) // Issue #589 ... this would appear to be the only reason why it would occur ... so guarding against it
            {
                doc.DetachRoot();

                if (xlights->AllModels[lastModelName] != nullptr
                    && nd->GetAttribute("Advanced", "0") != "1") {
                    std::string startChannel = ">" + lastModelName + ":1";
                    nd->DeleteAttribute("StartChannel");
                    nd->AddAttribute("StartChannel", startChannel);
                }

                Model *newModel = xlights->AllModels.CreateModel(nd);
                int cnt = 1;
                std::string name = newModel->name;
                while (xlights->AllModels[name] != nullptr) {
                    name = newModel->name + "-" + std::to_string(cnt++);
                }
                newModel->name = name;
                newModel->GetModelXml()->DeleteAttribute("name");
                newModel->GetModelXml()->AddAttribute("name", name);
                newModel->AddOffset(0.02, 0.02);
                newModel->UpdateXmlWithScale();
                xlights->AllModels.AddModel(newModel);
                lastModelName = name;

                xlights->UpdateModelsList();
                xlights->MarkEffectsFileDirty();
                wxTheClipboard->Close();
                SelectModel(name);
            }
            else
            {
                log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
                logger_base.warn("LayoutPanel: Error trying to parse XML for paste. Paste request ignored. %s.", (const char *)data.GetText().c_str());
                wxTheClipboard->Close();
            }
        }
    }
}

void LayoutPanel::DoUndo(wxCommandEvent& event) {
    int sz = undoBuffer.size() - 1;
    if (sz >= 0) {
        UnSelectAllModels();

        if (undoBuffer[sz].type == "Background") {
            wxPropertyGridEvent event;
            event.SetPropertyGrid(propertyEditor);
            wxStringProperty wsp("Background", undoBuffer[sz].key, undoBuffer[sz].data);
            event.SetProperty(&wsp);
            wxVariant value(undoBuffer[sz].data);
            event.SetPropertyValue(value);
            OnPropertyGridChange(event);
            UnSelectAllModels();
        } else if (undoBuffer[sz].type == "ModelProperty") {
            SelectModel(undoBuffer[sz].model);
            wxPropertyGridEvent event;
            event.SetPropertyGrid(propertyEditor);
            wxStringProperty wsp("Model", undoBuffer[sz].key, undoBuffer[sz].data);
            event.SetProperty(&wsp);
            wxVariant value(undoBuffer[sz].data);
            event.SetPropertyValue(value);
            OnPropertyGridChange(event);
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
            }
        } else if (undoBuffer[sz].type == "All") {
            UnSelectAllModels();

            wxStringInputStream gin(undoBuffer[sz].groups);
            wxXmlDocument gdoc;
            gdoc.Load(gin);
            wxStringInputStream min(undoBuffer[sz].models);
            wxXmlDocument mdoc(min);

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
            xlights->MarkEffectsFileDirty();
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
            xlights->MarkEffectsFileDirty();
            SelectModel(origName);
        }
        modelPreview->SetFocus();

        undoBuffer.resize(sz);
    }
}
void LayoutPanel::CreateUndoPoint(const std::string &type, const std::string &model, const std::string &key, const std::string &data) {
    xlights->MarkEffectsFileDirty();
    int idx = undoBuffer.size();

    //printf("%s   %s   %s  %s\n", type.c_str(), model.c_str(), key.c_str(), data.c_str());
    if (idx > 0 && (type == "SingleModel" || type == "ModelProperty" || type == "Background")
        && undoBuffer[idx - 1].model == model && undoBuffer[idx - 1].key == key)  {
        //SingleModel - multi mouse movement, just record the original
        //Background/ModelProperty - multiple changes of the same property (like spinning spin button)
        return;
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
            if( selectedModel == nullptr ) {
                undoBuffer.resize(idx);
                return;
            }
            m=selectedModel;
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
    int id = event.GetId();
    if(id == ID_MNU_DELETE_MODEL)
    {
        DeleteSelectedModel();
    }
    else if(id == ID_MNU_DELETE_MODEL_GROUP)
    {
        if( mSelectedGroup != nullptr ) {
            wxString name = TreeListViewModels->GetItemText(mSelectedGroup);
            if (wxMessageBox("Are you sure you want to remove the " + name + " group?", "Confirm Remove?", wxICON_QUESTION | wxYES_NO) == wxYES) {
                xlights->AllModels.Delete(name.ToStdString());
                selectedModel = nullptr;
                mSelectedGroup = nullptr;
                UnSelectAllModels();
                ShowPropGrid(true);
                xlights->UpdateModelsList();
                xlights->MarkEffectsFileDirty();
            }
        }
    }
    else if(id == ID_MNU_RENAME_MODEL_GROUP)
    {
        if( mSelectedGroup != nullptr ) {
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
                xlights->AllModels.Rename(sel.ToStdString(), name.ToStdString());
                xlights->UpdateModelsList();
                xlights->MarkEffectsFileDirty();
                model_grp_panel->UpdatePanel(name.ToStdString());
            }
        }
    }
    else if(id == ID_MNU_ADD_MODEL_GROUP)
    {
        wxTextEntryDialog dlg(this, "Enter name for new group", "Enter name for new group");
        if (dlg.ShowModal() == wxID_OK) {
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
            node->AddAttribute("layout", "grid");
            node->AddAttribute("GridSize", "400");
            wxString grp = currentLayoutGroup == "All Models" ? "Unassigned" : currentLayoutGroup;
            node->AddAttribute("LayoutGroup", grp);

            xlights->AllModels.AddModel(xlights->AllModels.CreateModel(node));
            xlights->UpdateModelsList();
            xlights->MarkEffectsFileDirty();
            model_grp_panel->UpdatePanel(name.ToStdString());
            ShowPropGrid(false);
        }
    }
}

LayoutGroup* LayoutPanel::GetLayoutGroup(const std::string &name)
{
    for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); it++) {
        LayoutGroup* grp = (LayoutGroup*)(*it);
        if( grp->GetName() == name ) {
            return grp;
        }
    }
    return nullptr;
}

void LayoutPanel::OnChoiceLayoutGroupsSelect(wxCommandEvent& event)
{
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
            ChoiceLayoutGroups->SetSelection(ChoiceLayoutGroups->GetCount()-2);

            xlights->UpdateModelsList();
            xlights->MarkEffectsFileDirty();
            ShowPropGrid(true);
        } else {
            SwitchChoiceToCurrentLayoutGroup();
            return;
        }
    } else {
        SetCurrentLayoutGroup(choice_layout);
        mSelectedGroup = nullptr;
        UpdateModelList();
    }
    modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
    modelPreview->SetScaleBackgroundImage(GetBackgroundScaledForSelectedPreview());
    modelPreview->SetBackgroundBrightness(GetBackgroundBrightnessForSelectedPreview());
    UpdatePreview();

    xlights->SetStoredLayoutGroup(currentLayoutGroup);
}

void LayoutPanel::AddPreviewChoice(const std::string &name)
{
    ChoiceLayoutGroups->Insert(name, ChoiceLayoutGroups->GetCount()-1);
    model_grp_panel->AddPreviewChoice(name);

    // see if we need to switch to this one
    const std::string& storedLayoutGroup = xlights->GetStoredLayoutGroup();
    if( storedLayoutGroup == name ) {
        for( int i = 0; i < ChoiceLayoutGroups->GetCount(); i++ )
        {
            if( ChoiceLayoutGroups->GetString(i) == storedLayoutGroup )
            {
                SetCurrentLayoutGroup(storedLayoutGroup);
                ChoiceLayoutGroups->SetSelection(i);
                modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
                modelPreview->SetScaleBackgroundImage(GetBackgroundScaledForSelectedPreview());
                modelPreview->SetBackgroundBrightness(GetBackgroundBrightnessForSelectedPreview());
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

void LayoutPanel::SwitchChoiceToCurrentLayoutGroup() {
    ChoiceLayoutGroups->SetSelection(0);
    for( int i = 0; i < ChoiceLayoutGroups->GetCount(); i++ )
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
        for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); it++) {
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
        xlights->MarkEffectsFileDirty();
        mSelectedGroup = nullptr;
        for( int i = 0; i < ChoiceLayoutGroups->GetCount(); i++ )
        {
            if( ChoiceLayoutGroups->GetString(i) == currentLayoutGroup )
            {
                ChoiceLayoutGroups->Delete(i);
                break;
            }
        }
        // change any existing assignments to this preview to be unassigned
        for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); it++) {
            Model *model = it->second;
            if( model->GetLayoutGroup() == currentLayoutGroup) {
                model->SetLayoutGroup("Unassigned");
            }
        }

        SetCurrentLayoutGroup("Default");
        ChoiceLayoutGroups->SetSelection(0);
        xlights->SetStoredLayoutGroup(currentLayoutGroup);

        UpdateModelList();
        modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
        modelPreview->SetScaleBackgroundImage(GetBackgroundScaledForSelectedPreview());
        modelPreview->SetBackgroundBrightness(GetBackgroundBrightnessForSelectedPreview());
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
    for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); it++) {
        LayoutGroup* grp = (LayoutGroup*)(*it);
        if (grp != nullptr) {
            if( currentLayoutGroup == grp->GetName() ) {
                pGrp = grp;
                break;
            }
        }
    }
}

//void OnItemExpanding(wxTreeListEvent& event);
//void OnItemExpanded(wxTreeListEvent& event);
//void OnItemChecked(wxTreeListEvent& event);

void LayoutPanel::OnItemContextMenu(wxTreeListEvent& event)
{
    wxMenu mnuContext;
    wxTreeListItem item = event.GetItem();
    if( item.IsOk() ) {
        std::string item_name = "";
        item_name = TreeListViewModels->GetItemText(item);
        Model *model = xlights->AllModels[item_name];
        if( model != nullptr ) {
            if( model->GetDisplayAs() == "ModelGroup" ) {
                mSelectedGroup = item;
            } else {
                mSelectedGroup = nullptr;
                SelectModel(model->name, false);
            }
        }
    } else {
        return;
    }
    if( selectedModel != nullptr ) {
        mnuContext.Append(ID_MNU_DELETE_MODEL,"Delete");
        mnuContext.AppendSeparator();
    }
    mnuContext.Append(ID_MNU_ADD_MODEL_GROUP,"Add Group");
    if( mSelectedGroup != nullptr ) {
        mnuContext.Append(ID_MNU_DELETE_MODEL_GROUP,"Delete Group");
        mnuContext.Append(ID_MNU_RENAME_MODEL_GROUP,"Rename Group");
    }
    mnuContext.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&LayoutPanel::OnModelsPopup, NULL, this);
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
    wxTreeListItem item = event.GetItem();
    if( item.IsOk() ) {
        std::string item_name = "";
        item_name = TreeListViewModels->GetItemText(item);
        Model *model = xlights->AllModels[item_name];
        if( model != nullptr ) {
            if( model->GetDisplayAs() == "ModelGroup" ) {
                mSelectedGroup = item;
                model_grp_panel->UpdatePanel(model->name);
                ShowPropGrid(false);
                UpdateModelList(false);
            } else {
                mSelectedGroup = nullptr;
                ShowPropGrid(true);
                SelectModel(model->name, false);
                SetToolTipForTreeList(TreeListViewModels, xlights->GetChannelToControllerMapping(model->GetNumberFromChannelString(model->ModelStartChannel)));
            }
        } else {
            mSelectedGroup = nullptr;
            selectedModel = nullptr;
            ShowPropGrid(true);
            UnSelectAllModels(true);
            SetToolTipForTreeList(TreeListViewModels, "");
        }
    } else {
        SetToolTipForTreeList(TreeListViewModels, "");
    }
}

