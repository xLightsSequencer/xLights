/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/
 
//(*InternalHeaders(PicturesAssistPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "PicturesAssistPanel.h"
#include "../../xlColorPicker.h"
#include "../../xlColorPickerFields.h"
#include "../../../include/PaintToolIcons.h"
#include "../../models/Model.h"
#include "../../xLightsMain.h"
#include "../../xLightsXmlFile.h"

//(*IdInit(PicturesAssistPanel)
const long PicturesAssistPanel::ID_BUTTON_NewImage = wxNewId();
const long PicturesAssistPanel::ID_BUTTON_LoadImage = wxNewId();
const long PicturesAssistPanel::ID_BUTTON_SaveImage = wxNewId();
const long PicturesAssistPanel::ID_BUTTON_SaveAs = wxNewId();
const long PicturesAssistPanel::ID_BUTTON1 = wxNewId();
const long PicturesAssistPanel::ID_STATICTEXT_CurrentImage = wxNewId();
const long PicturesAssistPanel::ID_STATICTEXT_ImageSize = wxNewId();
const long PicturesAssistPanel::ID_STATICTEXT_ModelSize = wxNewId();
const long PicturesAssistPanel::ID_PANEL_RightSide = wxNewId();
const long PicturesAssistPanel::ID_SCROLLED_EffectAssist = wxNewId();
const long PicturesAssistPanel::ID_PANEL1 = wxNewId();
//*)

const long PicturesAssistPanel::ID_BITMAPBUTTON_Paint_Pencil = wxNewId();
const long PicturesAssistPanel::ID_BITMAPBUTTON_Paint_Eraser = wxNewId();
const long PicturesAssistPanel::ID_BITMAPBUTTON_Paint_Eyedropper = wxNewId();
const long PicturesAssistPanel::ID_BITMAPBUTTON_Paint_Selectcopy = wxNewId();

wxDEFINE_EVENT(EVT_PAINT_COLOR, wxCommandEvent);
wxDEFINE_EVENT(EVT_IMAGE_FILE_SELECTED, wxCommandEvent);
wxDEFINE_EVENT(EVT_IMAGE_SIZE, wxCommandEvent);
wxDEFINE_EVENT(EVT_EYEDROPPER_COLOR, wxCommandEvent);

BEGIN_EVENT_TABLE(PicturesAssistPanel,wxPanel)
	//(*EventTable(PicturesAssistPanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_PAINT_COLOR, PicturesAssistPanel::OnColorChange)
    EVT_COMMAND(wxID_ANY, EVT_IMAGE_FILE_SELECTED, PicturesAssistPanel::OnImageFileSelected)
    EVT_COMMAND(wxID_ANY, EVT_IMAGE_SIZE, PicturesAssistPanel::OnImageSize)
    EVT_COMMAND(wxID_ANY, EVT_EYEDROPPER_COLOR, PicturesAssistPanel::OnEyedropperColor)
END_EVENT_TABLE()

PicturesAssistPanel::PicturesAssistPanel(wxWindow* parent, wxWindowID id,const wxPoint& pos,const wxSize& size)
: mPaintMode(xlGridCanvasPictures::PAINT_PENCIL),
  mPaintColor(xlRED)
{
    paint_pencil = wxBITMAP_PNG_FROM_DATA(pencil);
    paint_pencil_selected = wxBITMAP_PNG_FROM_DATA(pencil_sel);
    paint_eraser = wxBITMAP_PNG_FROM_DATA(eraser);
    paint_eraser_selected = wxBITMAP_PNG_FROM_DATA(eraser_sel);
    paint_eyedropper = wxBITMAP_PNG_FROM_DATA(eyedropper);
    paint_eyedropper_selected = wxBITMAP_PNG_FROM_DATA(eyedropper_sel);
    paint_selectcopy = wxBITMAP_PNG_FROM_DATA(selectcopy);
    paint_selectcopy_selected = wxBITMAP_PNG_FROM_DATA(selectcopy_sel);

	//(*Initialize(PicturesAssistPanel)
	wxFlexGridSizer* ColorPickerSizer;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* PaintFuntionsSizer;
	wxFlexGridSizer* PaintToolsSizer;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxWANTS_CHARS, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	Panel_Sizer = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxWANTS_CHARS, _T("ID_PANEL1"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	ScrolledWindowEffectAssist = new wxScrolledWindow(Panel_Sizer, ID_SCROLLED_EffectAssist, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS|wxVSCROLL|wxHSCROLL, _T("ID_SCROLLED_EffectAssist"));
	FlexGridSizer_Container = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer_Container->AddGrowableCol(0);
	FlexGridSizer_Container->AddGrowableRow(0);
	Panel_RightSide = new wxPanel(ScrolledWindowEffectAssist, ID_PANEL_RightSide, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_RightSide"));
	FlexGridSizer_RightSide = new wxFlexGridSizer(0, 1, 0, 0);
	ColorPickerSizer = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer_RightSide->Add(ColorPickerSizer, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	PaintToolsSizer = new wxFlexGridSizer(0, 4, 0, 10);
	FlexGridSizer_RightSide->Add(PaintToolsSizer, 1, wxALL|wxEXPAND, 5);
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
	Button_Resize = new wxButton(Panel_RightSide, ID_BUTTON1, _("Resize\nImage"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(Button_Resize, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	PaintFuntionsSizer->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_CurrentImage = new wxStaticText(Panel_RightSide, ID_STATICTEXT_CurrentImage, _("Current Image:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_CurrentImage"));
	PaintFuntionsSizer->Add(StaticText_CurrentImage, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_ImageSize = new wxStaticText(Panel_RightSide, ID_STATICTEXT_ImageSize, _("Image Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ImageSize"));
	PaintFuntionsSizer->Add(StaticText_ImageSize, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_ModelSize = new wxStaticText(Panel_RightSide, ID_STATICTEXT_ModelSize, _("Model Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ModelSize"));
	PaintFuntionsSizer->Add(StaticText_ModelSize, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer_RightSide->Add(PaintFuntionsSizer, 1, wxALL|wxEXPAND, 5);
	Panel_RightSide->SetSizer(FlexGridSizer_RightSide);
	FlexGridSizer_Container->Add(Panel_RightSide, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ScrolledWindowEffectAssist->SetSizer(FlexGridSizer_Container);
	FlexGridSizer2->Add(ScrolledWindowEffectAssist, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_Sizer->SetSizer(FlexGridSizer2);
	FlexGridSizer1->Add(Panel_Sizer, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);

	Connect(ID_BUTTON_NewImage,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PicturesAssistPanel::OnButton_NewImageClick);
	Connect(ID_BUTTON_LoadImage,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PicturesAssistPanel::OnButton_LoadImageClick);
	Connect(ID_BUTTON_SaveImage,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PicturesAssistPanel::OnButton_SaveImageClick);
	Connect(ID_BUTTON_SaveAs,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PicturesAssistPanel::OnButton_SaveAsClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PicturesAssistPanel::OnButton_ResizeClick);
	//*)

	//PanelEffectGrid = new xlGridCanvasEmpty(ScrolledWindowEffectAssist, ID_PANEL_EffectGrid, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE, _T("ID_PANEL_EffectGrid"));
	//FlexGridSizer_Container->Insert(0, PanelEffectGrid, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

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

    for (int i=wxEVT_SCROLLWIN_TOP; i<=wxEVT_SCROLLWIN_THUMBRELEASE; i++) {
        ScrolledWindowEffectAssist->Connect(wxID_ANY, i, wxScrollWinEventHandler(PicturesAssistPanel::OnWindowScrolled), NULL, this);
    }

}

PicturesAssistPanel::~PicturesAssistPanel()
{
	//(*Destroy(PicturesAssistPanel)
	//*)
}

void PicturesAssistPanel::ForceRefresh()
{
    ScrolledWindowEffectAssist->Refresh();
}

void PicturesAssistPanel::CreatePaintTools(wxWindow* parent, wxFlexGridSizer* container)
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
    BitmapButton_Paint_Selectcopy = new FlickerFreeBitmapButton(parent, ID_BITMAPBUTTON_Paint_Selectcopy, paint_selectcopy, wxDefaultPosition, wxSize(64,64), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Paint_Selectcopy"));
    BitmapButton_Paint_Selectcopy->SetBitmapSelected(paint_selectcopy_selected);
    BitmapButton_Paint_Selectcopy->SetBitmapFocus(paint_selectcopy_selected);
    BitmapButton_Paint_Selectcopy->SetBitmapCurrent(paint_selectcopy_selected);
    container->Add(BitmapButton_Paint_Selectcopy, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Connect(ID_BITMAPBUTTON_Paint_Pencil,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PicturesAssistPanel::OnBitmapButton_Paint_PencilClick);
    Connect(ID_BITMAPBUTTON_Paint_Eraser,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PicturesAssistPanel::OnBitmapButton_Paint_EraserClick);
    Connect(ID_BITMAPBUTTON_Paint_Eyedropper,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PicturesAssistPanel::OnBitmapButton_Paint_EyedropperClick);
    Connect(ID_BITMAPBUTTON_Paint_Selectcopy,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PicturesAssistPanel::OnBitmapButton_Paint_SelectcopyClick);
}

void PicturesAssistPanel::OnBitmapButton_Paint_PencilClick(wxCommandEvent& event)
{
    BitmapButton_Paint_Pencil->SetBitmap(paint_pencil_selected);
    BitmapButton_Paint_Eraser->SetBitmap(paint_eraser);
    BitmapButton_Paint_Eyedropper->SetBitmap(paint_eyedropper);
    BitmapButton_Paint_Selectcopy->SetBitmap(paint_selectcopy);
    mGridCanvas->SetPaintMode(xlGridCanvasPictures::PAINT_PENCIL);
}

void PicturesAssistPanel::OnBitmapButton_Paint_EraserClick(wxCommandEvent& event)
{
    BitmapButton_Paint_Pencil->SetBitmap(paint_pencil);
    BitmapButton_Paint_Eraser->SetBitmap(paint_eraser_selected);
    BitmapButton_Paint_Eyedropper->SetBitmap(paint_eyedropper);
    BitmapButton_Paint_Selectcopy->SetBitmap(paint_selectcopy);
    mGridCanvas->SetPaintMode(xlGridCanvasPictures::PAINT_ERASER);
}

void PicturesAssistPanel::OnBitmapButton_Paint_EyedropperClick(wxCommandEvent& event)
{
    BitmapButton_Paint_Pencil->SetBitmap(paint_pencil);
    BitmapButton_Paint_Eraser->SetBitmap(paint_eraser);
    BitmapButton_Paint_Eyedropper->SetBitmap(paint_eyedropper_selected);
    BitmapButton_Paint_Selectcopy->SetBitmap(paint_selectcopy);
    mGridCanvas->SetPaintMode(xlGridCanvasPictures::PAINT_EYEDROPPER);
}

void PicturesAssistPanel::OnBitmapButton_Paint_SelectcopyClick(wxCommandEvent& event)
{
    BitmapButton_Paint_Pencil->SetBitmap(paint_pencil);
    BitmapButton_Paint_Eraser->SetBitmap(paint_eraser);
    BitmapButton_Paint_Eyedropper->SetBitmap(paint_eyedropper);
    BitmapButton_Paint_Selectcopy->SetBitmap(paint_selectcopy_selected);
    mGridCanvas->SetPaintMode(xlGridCanvasPictures::PAINT_SELECTCOPY);
}

void PicturesAssistPanel::OnColorChange(wxCommandEvent& event)
{
    xlColor* color = (xlColor*)event.GetClientData();
    mGridCanvas->SetPaintColor(*color);
}

void PicturesAssistPanel::OnEyedropperColor(wxCommandEvent& event)
{
    xlColor color(event.GetInt(), true);
    mGridCanvas->SetPaintColor(color);
    mColorPicker->SetColor(color);
}

void PicturesAssistPanel::OnImageFileSelected(wxCommandEvent& event)
{
   wxString* name = (wxString*)event.GetClientData();
   StaticText_CurrentImage->SetLabelText( "Current Image: " + *name );
}

void PicturesAssistPanel::OnImageSize(wxCommandEvent& event)
{
    wxString image_size = event.GetString();
    StaticText_ImageSize->SetLabelText( image_size );
    int bw, bh;
    mGridCanvas->GetModel()->GetBufferSize(mGridCanvas->GetEffect()->GetSettings().Get("T_CHOICE_BufferStyle", "Default"),
                                           mGridCanvas->GetEffect()->GetSettings().Get("T_CHOICE_PerPreviewCamera", "2D"),
                                           mGridCanvas->GetEffect()->GetSettings().Get("T_CHOICE_BufferTransform", "None"),
                                           bw, bh, mGridCanvas->GetEffect()->GetSettings().GetInt("B_SPINCTRL_BufferStagger", 0));
    StaticText_ModelSize->SetLabelText(wxString::Format("Model Size: %d x %d", bw, bh));
}

void PicturesAssistPanel::OnButton_SaveImageClick(wxCommandEvent& event)
{
    mGridCanvas->SaveImage();
}

void PicturesAssistPanel::OnButton_NewImageClick(wxCommandEvent& event)
{
    wxString image_dir = mxLightsParent->CurrentSeqXmlFile->GetImageDir(mxLightsParent);
    mGridCanvas->CreateNewImage(image_dir);
}

void PicturesAssistPanel::OnButton_LoadImageClick(wxCommandEvent& event)
{
    mGridCanvas->LoadImage();
}

void PicturesAssistPanel::OnButton_SaveAsClick(wxCommandEvent& event)
{
    mGridCanvas->SaveAsImage();
}

void PicturesAssistPanel::OnButton_ResizeClick(wxCommandEvent& event)
{
    mGridCanvas->ResizeImage();
}

void PicturesAssistPanel::OnWindowScrolled(wxScrollWinEvent &event)
{
    ForceRefresh();
}
