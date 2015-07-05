#include "SceneEditor.h"
#include "xLightsMain.h"

//(*InternalHeaders(SceneEditor)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(SceneEditor)
const long SceneEditor::ID_PANEL_Scene_Editor = wxNewId();
const long SceneEditor::ID_SCROLLED_Scene_Editor = wxNewId();
const long SceneEditor::ID_PANEL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SceneEditor,wxPanel)
	//(*EventTable(SceneEditor)
	//*)
END_EVENT_TABLE()

SceneEditor::SceneEditor(wxWindow* parent, xLightsFrame* xlights_parent)
: mxLightsParent(xlights_parent)
{
	//(*Initialize(SceneEditor)
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	Panel_Sizer = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	ScrolledWindowSceneEditor = new wxScrolledWindow(Panel_Sizer, ID_SCROLLED_Scene_Editor, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLED_Scene_Editor"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer5->AddGrowableRow(0);
	PanelSceneEditor = new xlGridCanvas(ScrolledWindowSceneEditor, ID_PANEL_Scene_Editor, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE, _T("ID_PANEL_Scene_Editor"));
	FlexGridSizer5->Add(PanelSceneEditor, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ScrolledWindowSceneEditor->SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(ScrolledWindowSceneEditor);
	FlexGridSizer5->SetSizeHints(ScrolledWindowSceneEditor);
	FlexGridSizer2->Add(ScrolledWindowSceneEditor, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_Sizer->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel_Sizer);
	FlexGridSizer2->SetSizeHints(Panel_Sizer);
	FlexGridSizer1->Add(Panel_Sizer, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(wxEVT_SIZE,(wxObjectEventFunction)&SceneEditor::OnResize);
	//*)
}

SceneEditor::~SceneEditor()
{
	//(*Destroy(SceneEditor)
	//*)
}

void SceneEditor::ForceRefresh()
{
    PanelSceneEditor->Refresh();
    PanelSceneEditor->Update();
    ScrolledWindowSceneEditor->Refresh();
}

void SceneEditor::SetEffect(Effect* effect_)
{
    mEffect = effect_;
    if( mEffect != NULL )
    {
        EffectLayer* layer = mEffect->GetParentEffectLayer();
        Element* elem = layer->GetParentElement();
        wxString model_name = elem->GetName();
        ModelClass &cls = mxLightsParent->GetModelClass(model_name);
        PanelSceneEditor->SetModelClass(&cls);
        PanelSceneEditor->SetNumColumns(cls.BufferWi);
        PanelSceneEditor->SetNumRows(cls.BufferHt);
    }
    PanelSceneEditor->SetEffect(mEffect);

    wxSizeEvent dummy;
    OnResize(dummy);
}

void SceneEditor::OnResize(wxSizeEvent& event)
{
    wxSize s = GetSize();
    s.SetWidth(s.GetWidth()-15);
    s.SetHeight(s.GetHeight()-15);
    Panel_Sizer->SetSize(s);
    Panel_Sizer->SetMinSize(s);
    Panel_Sizer->SetMaxSize(s);
    Panel_Sizer->Refresh();

    PanelSceneEditor->AdjustSize(s);
    PanelSceneEditor->Refresh();

    ScrolledWindowSceneEditor->SetSize(s);
    ScrolledWindowSceneEditor->SetMinSize(s);
    ScrolledWindowSceneEditor->SetMaxSize(s);

    ScrolledWindowSceneEditor->FitInside();
    ScrolledWindowSceneEditor->SetScrollRate(5, 5);
    ScrolledWindowSceneEditor->Refresh();
    Layout();
}
