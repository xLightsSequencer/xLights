#include "EffectAssist.h"
#include "xLightsMain.h"
#include "BitmapCache.h"
#include "xlColorPicker.h"
#include "xlColorPickerFields.h"
#include "../include/PaintToolIcons.h"
#include "xlGridCanvasMorph.h"
#include "xlGridCanvasEmpty.h"

//(*InternalHeaders(EffectAssist)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EffectAssist)
const long EffectAssist::ID_BUTTON_NewImage = wxNewId();
const long EffectAssist::ID_BUTTON_LoadImage = wxNewId();
const long EffectAssist::ID_BUTTON_SaveImage = wxNewId();
const long EffectAssist::ID_BUTTON_SaveAs = wxNewId();
const long EffectAssist::ID_STATICTEXT_CurrentImage = wxNewId();
const long EffectAssist::ID_STATICTEXT_ImageSize = wxNewId();
const long EffectAssist::ID_STATICTEXT_ModelSize = wxNewId();
const long EffectAssist::ID_PANEL_RightSide = wxNewId();
const long EffectAssist::ID_SCROLLED_EffectAssist = wxNewId();
const long EffectAssist::ID_PANEL1 = wxNewId();
//*)

const long EffectAssist::ID_PANEL_EffectGrid = wxNewId();
const long EffectAssist::ID_BITMAPBUTTON_Paint_Pencil = wxNewId();
const long EffectAssist::ID_BITMAPBUTTON_Paint_Eraser = wxNewId();
const long EffectAssist::ID_BITMAPBUTTON_Paint_Eyedropper = wxNewId();

wxDEFINE_EVENT(EVT_PAINT_COLOR, wxCommandEvent);
wxDEFINE_EVENT(EVT_IMAGE_FILE_SELECTED, wxCommandEvent);
wxDEFINE_EVENT(EVT_IMAGE_SIZE, wxCommandEvent);
wxDEFINE_EVENT(EVT_EYEDROPPER_COLOR, wxCommandEvent);

BEGIN_EVENT_TABLE(EffectAssist,wxPanel)
	//(*EventTable(EffectAssist)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_PAINT_COLOR, EffectAssist::OnColorChange)
    EVT_COMMAND(wxID_ANY, EVT_IMAGE_FILE_SELECTED, EffectAssist::OnImageFileSelected)
    EVT_COMMAND(wxID_ANY, EVT_IMAGE_SIZE, EffectAssist::OnImageSize)
    EVT_COMMAND(wxID_ANY, EVT_EYEDROPPER_COLOR, EffectAssist::OnEyedropperColor)
END_EVENT_TABLE()

EffectAssist::EffectAssist(wxWindow* parent, xLightsFrame* xlights_parent, wxWindowID id)
: mxLightsParent(xlights_parent),
  mPaintMode(xlGridCanvasPictures::PAINT_PENCIL),
  mPaintColor(xlRED)
{
    paint_pencil = wxBITMAP_PNG_FROM_DATA(pencil);
    paint_pencil_selected = wxBITMAP_PNG_FROM_DATA(pencil_sel);
    paint_eraser = wxBITMAP_PNG_FROM_DATA(eraser);
    paint_eraser_selected = wxBITMAP_PNG_FROM_DATA(eraser_sel);
    paint_eyedropper = wxBITMAP_PNG_FROM_DATA(eyedropper);
    paint_eyedropper_selected = wxBITMAP_PNG_FROM_DATA(eyedropper_sel);

	//(*Initialize(EffectAssist)
	wxFlexGridSizer* PaintFuntionsSizer;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* PaintToolsSizer;
	wxFlexGridSizer* ColorPickerSizer;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	Panel_Sizer = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	ScrolledWindowEffectAssist = new wxScrolledWindow(Panel_Sizer, ID_SCROLLED_EffectAssist, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLED_EffectAssist"));
	FlexGridSizer_Container = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer_Container->AddGrowableCol(0);
	FlexGridSizer_Container->AddGrowableRow(0);
	Panel_RightSide = new wxPanel(ScrolledWindowEffectAssist, ID_PANEL_RightSide, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_RightSide"));
	FlexGridSizer_RightSide = new wxFlexGridSizer(0, 1, 0, 0);
	ColorPickerSizer = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer_RightSide->Add(ColorPickerSizer, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	PaintToolsSizer = new wxFlexGridSizer(0, 3, 0, 10);
	FlexGridSizer_RightSide->Add(PaintToolsSizer, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	PaintFuntionsSizer = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 6, 0, 0);
	Button_NewImage = new wxButton(Panel_RightSide, ID_BUTTON_NewImage, _("New\nImage"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_NewImage"));
	FlexGridSizer3->Add(Button_NewImage, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Button_LoadImage = new wxButton(Panel_RightSide, ID_BUTTON_LoadImage, _("Load\nImage"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LoadImage"));
	FlexGridSizer3->Add(Button_LoadImage, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Button_SaveImage = new wxButton(Panel_RightSide, ID_BUTTON_SaveImage, _("Save\nImage"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SaveImage"));
	FlexGridSizer3->Add(Button_SaveImage, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Button_SaveAs = new wxButton(Panel_RightSide, ID_BUTTON_SaveAs, _("Save As\nImage"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SaveAs"));
	FlexGridSizer3->Add(Button_SaveAs, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	PaintFuntionsSizer->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_CurrentImage = new wxStaticText(Panel_RightSide, ID_STATICTEXT_CurrentImage, _("Current Image:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_CurrentImage"));
	PaintFuntionsSizer->Add(StaticText_CurrentImage, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_ImageSize = new wxStaticText(Panel_RightSide, ID_STATICTEXT_ImageSize, _("Image Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ImageSize"));
	PaintFuntionsSizer->Add(StaticText_ImageSize, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_ModelSize = new wxStaticText(Panel_RightSide, ID_STATICTEXT_ModelSize, _("Model Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ModelSize"));
	PaintFuntionsSizer->Add(StaticText_ModelSize, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer_RightSide->Add(PaintFuntionsSizer, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Panel_RightSide->SetSizer(FlexGridSizer_RightSide);
	FlexGridSizer_RightSide->Fit(Panel_RightSide);
	FlexGridSizer_RightSide->SetSizeHints(Panel_RightSide);
	FlexGridSizer_Container->Add(Panel_RightSide, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ScrolledWindowEffectAssist->SetSizer(FlexGridSizer_Container);
	FlexGridSizer_Container->Fit(ScrolledWindowEffectAssist);
	FlexGridSizer_Container->SetSizeHints(ScrolledWindowEffectAssist);
	FlexGridSizer2->Add(ScrolledWindowEffectAssist, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_Sizer->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel_Sizer);
	FlexGridSizer2->SetSizeHints(Panel_Sizer);
	FlexGridSizer1->Add(Panel_Sizer, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_NewImage,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectAssist::OnButton_NewImageClick);
	Connect(ID_BUTTON_LoadImage,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectAssist::OnButton_LoadImageClick);
	Connect(ID_BUTTON_SaveImage,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectAssist::OnButton_SaveImageClick);
	Connect(ID_BUTTON_SaveAs,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectAssist::OnButton_SaveAsClick);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&EffectAssist::OnResize);
	//*)

	PanelEffectGrid = new xlGridCanvasEmpty(ScrolledWindowEffectAssist, ID_PANEL_EffectGrid, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE, _T("ID_PANEL_EffectGrid"));
	FlexGridSizer_Container->Insert(0, PanelEffectGrid, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

    CreatePaintTools(Panel_RightSide, PaintToolsSizer);

    mColorPicker = new xlColorPickerFields(Panel_RightSide, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_ColorPicker"));
	ColorPickerSizer->Add(mColorPicker, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);

	FlexGridSizer_RightSide->Fit(Panel_RightSide);
	FlexGridSizer_RightSide->SetSizeHints(Panel_RightSide);
	FlexGridSizer_Container->Fit(ScrolledWindowEffectAssist);
	FlexGridSizer_Container->SetSizeHints(ScrolledWindowEffectAssist);
	FlexGridSizer2->Fit(Panel_Sizer);
	FlexGridSizer2->SetSizeHints(Panel_Sizer);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
}

EffectAssist::~EffectAssist()
{
	//(*Destroy(EffectAssist)
	//*)
}

void EffectAssist::CreatePaintTools(wxWindow* parent, wxFlexGridSizer* container)
{
    BitmapButton_Paint_Pencil = new FlickerFreeBitmapButton(parent, ID_BITMAPBUTTON_Paint_Pencil, paint_pencil_selected, wxDefaultPosition, wxSize(64,64), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Paint_Pencil"));
    BitmapButton_Paint_Pencil->SetBitmapSelected(paint_pencil_selected);
    BitmapButton_Paint_Pencil->SetBitmapFocus(paint_pencil_selected);
    BitmapButton_Paint_Pencil->SetBitmapCurrent(paint_pencil_selected);
    container->Add(BitmapButton_Paint_Pencil, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Paint_Eraser = new FlickerFreeBitmapButton(parent, ID_BITMAPBUTTON_Paint_Eraser, paint_eraser, wxDefaultPosition, wxSize(64,64), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Paint_Eraser"));
    BitmapButton_Paint_Eraser->SetBitmapSelected(paint_eraser_selected);
    BitmapButton_Paint_Eraser->SetBitmapFocus(paint_eraser_selected);
    BitmapButton_Paint_Eraser->SetBitmapCurrent(paint_eraser_selected);
    container->Add(BitmapButton_Paint_Eraser, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Paint_Eyedropper = new FlickerFreeBitmapButton(parent, ID_BITMAPBUTTON_Paint_Eyedropper, paint_eyedropper, wxDefaultPosition, wxSize(64,64), wxBU_AUTODRAW, wxDefaultValidator, _T("BitmapButton_Paint_Eyedropper"));
    BitmapButton_Paint_Eyedropper->SetBitmapSelected(paint_eyedropper_selected);
    BitmapButton_Paint_Eyedropper->SetBitmapFocus(paint_eyedropper_selected);
    BitmapButton_Paint_Eyedropper->SetBitmapCurrent(paint_eyedropper_selected);
    container->Add(BitmapButton_Paint_Eyedropper, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Connect(ID_BITMAPBUTTON_Paint_Pencil,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectAssist::OnBitmapButton_Paint_PencilClick);
    Connect(ID_BITMAPBUTTON_Paint_Eraser,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectAssist::OnBitmapButton_Paint_EraserClick);
    Connect(ID_BITMAPBUTTON_Paint_Eyedropper,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectAssist::OnBitmapButton_Paint_EyedropperClick);
}

void EffectAssist::ForceRefresh()
{
    PanelEffectGrid->Refresh();
    PanelEffectGrid->Update();
    ScrolledWindowEffectAssist->Refresh();
    PanelEffectGrid->ForceRefresh();
}

void EffectAssist::SetEffect(Effect* effect_)
{
    mEffect = effect_;

    xlGridCanvas* old_grid = PanelEffectGrid;
    if( mEffect == NULL )
    {
        PanelEffectGrid = new xlGridCanvasEmpty(ScrolledWindowEffectAssist, ID_PANEL_EffectGrid, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE, _T("ID_PANEL_EffectGrid"));
    }
    else if( mEffect->GetEffectIndex() == BitmapCache::eff_MORPH )
    {
        PanelEffectGrid = new xlGridCanvasMorph(ScrolledWindowEffectAssist, ID_PANEL_EffectGrid, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE, _T("ID_PANEL_EffectGrid"));
    }
    else if( mEffect->GetEffectIndex() == BitmapCache::eff_PICTURES )
    {
        PanelEffectGrid = new xlGridCanvasPictures(ScrolledWindowEffectAssist, ID_PANEL_EffectGrid, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE, _T("ID_PANEL_EffectGrid"));
    }
    else
    {
        PanelEffectGrid = new xlGridCanvasEmpty(ScrolledWindowEffectAssist, ID_PANEL_EffectGrid, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE, _T("ID_PANEL_EffectGrid"));
    }
    FlexGridSizer_Container->Replace(old_grid, PanelEffectGrid);
    delete old_grid;
	FlexGridSizer1->Layout();

    if( mEffect != NULL )
    {
        EffectLayer* layer = mEffect->GetParentEffectLayer();
        Element* elem = layer->GetParentElement();
        wxString model_name = elem->GetName();
        ModelClass &cls = mxLightsParent->GetModelClass(model_name);
        PanelEffectGrid->SetModelClass(&cls);
        PanelEffectGrid->SetNumColumns(cls.BufferWi);
        PanelEffectGrid->SetNumRows(cls.BufferHt);
        StaticText_ModelSize->SetLabelText(wxString::Format("Model Size: %d x %d", cls.BufferWi, cls.BufferHt));

        if( mEffect->GetEffectIndex() == BitmapCache::eff_PICTURES )
        {
            Panel_RightSide->Show();
        }
        else
        {
            Panel_RightSide->Hide();
        }
    }
    PanelEffectGrid->SetEffect(mEffect);

    wxSizeEvent dummy;
    OnResize(dummy);
}

void EffectAssist::OnResize(wxSizeEvent& event)
{
    wxSize s = GetSize();
    s.SetWidth(s.GetWidth()-15);
    s.SetHeight(s.GetHeight()-15);
    Panel_Sizer->SetSize(s);
    Panel_Sizer->SetMinSize(s);
    Panel_Sizer->SetMaxSize(s);
    Panel_Sizer->Refresh();

    PanelEffectGrid->AdjustSize(s);
    PanelEffectGrid->Refresh();

    ScrolledWindowEffectAssist->SetSize(s);
    ScrolledWindowEffectAssist->SetMinSize(s);
    ScrolledWindowEffectAssist->SetMaxSize(s);

    ScrolledWindowEffectAssist->FitInside();
    ScrolledWindowEffectAssist->SetScrollRate(5, 5);
    ScrolledWindowEffectAssist->Refresh();
    Layout();
}

void EffectAssist::OnBitmapButton_Paint_PencilClick(wxCommandEvent& event)
{
    BitmapButton_Paint_Pencil->SetBitmap(paint_pencil_selected);
    BitmapButton_Paint_Eraser->SetBitmap(paint_eraser);
    BitmapButton_Paint_Eyedropper->SetBitmap(paint_eyedropper);
    ((xlGridCanvasPictures*)PanelEffectGrid)->SetPaintMode(xlGridCanvasPictures::PAINT_PENCIL);
}

void EffectAssist::OnBitmapButton_Paint_EraserClick(wxCommandEvent& event)
{
    BitmapButton_Paint_Pencil->SetBitmap(paint_pencil);
    BitmapButton_Paint_Eraser->SetBitmap(paint_eraser_selected);
    BitmapButton_Paint_Eyedropper->SetBitmap(paint_eyedropper);
    ((xlGridCanvasPictures*)PanelEffectGrid)->SetPaintMode(xlGridCanvasPictures::PAINT_ERASER);
}

void EffectAssist::OnBitmapButton_Paint_EyedropperClick(wxCommandEvent& event)
{
    BitmapButton_Paint_Pencil->SetBitmap(paint_pencil);
    BitmapButton_Paint_Eraser->SetBitmap(paint_eraser);
    BitmapButton_Paint_Eyedropper->SetBitmap(paint_eyedropper_selected);
    ((xlGridCanvasPictures*)PanelEffectGrid)->SetPaintMode(xlGridCanvasPictures::PAINT_EYEDROPPER);
}

void EffectAssist::OnColorChange(wxCommandEvent& event)
{
    xlColor* color = (xlColor*)event.GetClientData();
    ((xlGridCanvasPictures*)PanelEffectGrid)->SetPaintColor(*color);
}

void EffectAssist::OnEyedropperColor(wxCommandEvent& event)
{
    xlColor* color = (xlColor*)event.GetClientData();
    ((xlGridCanvasPictures*)PanelEffectGrid)->SetPaintColor(*color);
    mColorPicker->SetColor(*color);
}

void EffectAssist::OnImageFileSelected(wxCommandEvent& event)
{
   wxString* name = (wxString*)event.GetClientData();
   StaticText_CurrentImage->SetLabelText( "Current Image: " + *name );
}

void EffectAssist::OnImageSize(wxCommandEvent& event)
{
   wxString* image_size = (wxString*)event.GetClientData();
   StaticText_ImageSize->SetLabelText( *image_size );
}

void EffectAssist::OnButton_SaveImageClick(wxCommandEvent& event)
{
    ((xlGridCanvasPictures*)PanelEffectGrid)->SaveImage();
}

void EffectAssist::OnButton_NewImageClick(wxCommandEvent& event)
{
    wxString image_dir = mxLightsParent->CurrentSeqXmlFile->GetImageDir(mxLightsParent);
    ((xlGridCanvasPictures*)PanelEffectGrid)->CreateNewImage(image_dir);
}

void EffectAssist::OnButton_LoadImageClick(wxCommandEvent& event)
{
    ((xlGridCanvasPictures*)PanelEffectGrid)->LoadImage();
}

void EffectAssist::OnButton_SaveAsClick(wxCommandEvent& event)
{
    ((xlGridCanvasPictures*)PanelEffectGrid)->SaveAsImage();
}
