#include "ColorManagerDialog.h"
#include <wx/colour.h>
#include <wx/colourdata.h>
#include <wx/colordlg.h>
#include "ColorManager.h"
#include "xLightsMain.h"
#include "ColorFanImage.h"

//(*InternalHeaders(ColorManagerDialog)
#include <wx/bitmap.h>
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(ColorManagerDialog)
const long ColorManagerDialog::ID_STATICTEXT1 = wxNewId();
const long ColorManagerDialog::ID_STATICBITMAP1 = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT2 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_Timing1 = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT3 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_Timing2 = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT4 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_Timing3 = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT5 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_Timing4 = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT6 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_Timing5 = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT19 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_TimingDefault = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT12 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_EffectDefault = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT7 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_EffectSelected = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT9 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_ReferenceEffect = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT10 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_RowHeader = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT11 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_RowHeaderSelected = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT13 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_Gridlines = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT14 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_Labels = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT15 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_LabelOutline = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT16 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_Phrases = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT17 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_Words = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT18 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_Phonemes = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT8 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_ModelSelected = wxNewId();
const long ColorManagerDialog::ID_BUTTON_RESET = wxNewId();
const long ColorManagerDialog::ID_BUTTON_Cancel = wxNewId();
const long ColorManagerDialog::ID_BUTTON_Close = wxNewId();
//*)

BEGIN_EVENT_TABLE(ColorManagerDialog,wxDialog)
	//(*EventTable(ColorManagerDialog)
	//*)
END_EVENT_TABLE()

ColorManagerDialog::ColorManagerDialog(wxWindow* parent,ColorManager& color_mgr_,wxWindowID id,const wxPoint& pos,const wxSize& size)
: color_mgr(color_mgr_)
{
    color_fan = wxBITMAP_PNG_FROM_DATA(colorfan);

	//(*Initialize(ColorManagerDialog)
	wxStaticBoxSizer* StaticBoxSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer9;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxStaticBoxSizer* StaticBoxSizer3;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer6;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Color Manager"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	wxFont StaticText1Font(16,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText1->SetFont(StaticText1Font);
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBitmap1 = new wxStaticBitmap(this, ID_STATICBITMAP1, color_fan, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, _T("ID_STATICBITMAP1"));
	FlexGridSizer2->Add(StaticBitmap1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableCol(1);
	FlexGridSizer3->AddGrowableCol(2);
	FlexGridSizer3->AddGrowableRow(0);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxVERTICAL, this, _("Timing Tracks"));
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Timing 1"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer6->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Timing1 = new wxBitmapButton(this, ID_BITMAPBUTTON_Timing1, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Timing1"));
	FlexGridSizer6->Add(BitmapButton_Timing1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Timing 2"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer6->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Timing2 = new wxBitmapButton(this, ID_BITMAPBUTTON_Timing2, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Timing2"));
	FlexGridSizer6->Add(BitmapButton_Timing2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Timing 3"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer6->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Timing3 = new wxBitmapButton(this, ID_BITMAPBUTTON_Timing3, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Timing3"));
	FlexGridSizer6->Add(BitmapButton_Timing3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Timing 4"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer6->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Timing4 = new wxBitmapButton(this, ID_BITMAPBUTTON_Timing4, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Timing4"));
	FlexGridSizer6->Add(BitmapButton_Timing4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Timing 5"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer6->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Timing5 = new wxBitmapButton(this, ID_BITMAPBUTTON_Timing5, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Timing5"));
	FlexGridSizer6->Add(BitmapButton_Timing5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticBoxSizer1->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxVERTICAL, this, _("Effect Grid"));
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText19 = new wxStaticText(this, ID_STATICTEXT19, _("Timings"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT19"));
	FlexGridSizer8->Add(StaticText19, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_TimingDefault = new wxBitmapButton(this, ID_BITMAPBUTTON_TimingDefault, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_TimingDefault"));
	FlexGridSizer8->Add(BitmapButton_TimingDefault, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText12 = new wxStaticText(this, ID_STATICTEXT12, _("Effects"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
	FlexGridSizer8->Add(StaticText12, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_EffectDefault = new wxBitmapButton(this, ID_BITMAPBUTTON_EffectDefault, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_EffectDefault"));
	FlexGridSizer8->Add(BitmapButton_EffectDefault, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Effect Selected"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer8->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_EffectSelected = new wxBitmapButton(this, ID_BITMAPBUTTON_EffectSelected, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_EffectSelected"));
	FlexGridSizer8->Add(BitmapButton_EffectSelected, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("Reference Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer8->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_ReferenceEffect = new wxBitmapButton(this, ID_BITMAPBUTTON_ReferenceEffect, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_ReferenceEffect"));
	FlexGridSizer8->Add(BitmapButton_ReferenceEffect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("Row Header"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer8->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_RowHeader = new wxBitmapButton(this, ID_BITMAPBUTTON_RowHeader, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_RowHeader"));
	FlexGridSizer8->Add(BitmapButton_RowHeader, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText11 = new wxStaticText(this, ID_STATICTEXT11, _("Row Header Selected"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	FlexGridSizer8->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_RowHeaderSelected = new wxBitmapButton(this, ID_BITMAPBUTTON_RowHeaderSelected, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_RowHeaderSelected"));
	FlexGridSizer8->Add(BitmapButton_RowHeaderSelected, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText13 = new wxStaticText(this, ID_STATICTEXT13, _("Gridlines"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
	FlexGridSizer8->Add(StaticText13, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Gridlines = new wxBitmapButton(this, ID_BITMAPBUTTON_Gridlines, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Gridlines"));
	FlexGridSizer8->Add(BitmapButton_Gridlines, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText14 = new wxStaticText(this, ID_STATICTEXT14, _("Labels"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
	FlexGridSizer8->Add(StaticText14, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Labels = new wxBitmapButton(this, ID_BITMAPBUTTON_Labels, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Labels"));
	FlexGridSizer8->Add(BitmapButton_Labels, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText15 = new wxStaticText(this, ID_STATICTEXT15, _("Label Outline"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
	FlexGridSizer8->Add(StaticText15, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_LabelOutline = new wxBitmapButton(this, ID_BITMAPBUTTON_LabelOutline, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_LabelOutline"));
	FlexGridSizer8->Add(BitmapButton_LabelOutline, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText16 = new wxStaticText(this, ID_STATICTEXT16, _("Phrases"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT16"));
	FlexGridSizer8->Add(StaticText16, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Phrases = new wxBitmapButton(this, ID_BITMAPBUTTON_Phrases, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Phrases"));
	FlexGridSizer8->Add(BitmapButton_Phrases, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText17 = new wxStaticText(this, ID_STATICTEXT17, _("Words"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT17"));
	FlexGridSizer8->Add(StaticText17, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Words = new wxBitmapButton(this, ID_BITMAPBUTTON_Words, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Words"));
	FlexGridSizer8->Add(BitmapButton_Words, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText18 = new wxStaticText(this, ID_STATICTEXT18, _("Phonemes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT18"));
	FlexGridSizer8->Add(StaticText18, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Phonemes = new wxBitmapButton(this, ID_BITMAPBUTTON_Phonemes, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Phonemes"));
	FlexGridSizer8->Add(BitmapButton_Phonemes, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticBoxSizer2->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer3 = new wxStaticBoxSizer(wxVERTICAL, this, _("Layout Tab"));
	FlexGridSizer9 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Reserved"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer9->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_ModelSelected = new wxBitmapButton(this, ID_BITMAPBUTTON_ModelSelected, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_ModelSelected"));
	FlexGridSizer9->Add(BitmapButton_ModelSelected, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticBoxSizer3->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(StaticBoxSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer7 = new wxFlexGridSizer(0, 5, 0, 0);
	Button_Reset = new wxButton(this, ID_BUTTON_RESET, _("Reset Defaults"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_RESET"));
	FlexGridSizer7->Add(Button_Reset, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON_Cancel, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Cancel"));
	FlexGridSizer7->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Close = new wxButton(this, ID_BUTTON_Close, _("OK"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Close"));
	FlexGridSizer7->Add(Button_Close, 1, wxALL|wxALIGN_RIGHT, 5);
	FlexGridSizer1->Add(FlexGridSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_Timing1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_Timing2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_Timing3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_Timing4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_Timing5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_TimingDefault,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_EffectDefault,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_EffectSelected,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_ReferenceEffect,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_RowHeader,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_RowHeaderSelected,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_Gridlines,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_Labels,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_LabelOutline,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_Phrases,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_Words,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_Phonemes,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_ModelSelected,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BUTTON_RESET,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::OnButton_Reset_DefaultsClick);
	Connect(ID_BUTTON_Cancel,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::OnButton_CancelClick);
	Connect(ID_BUTTON_Close,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::OnButton_OkClick);
	//*)

    UpdateButtonColors();
    color_mgr.Snapshot();
}

ColorManagerDialog::~ColorManagerDialog()
{
	//(*Destroy(ColorManagerDialog)
	//*)
}

void ColorManagerDialog::UpdateButtonColors()
{
	SetButtonColor( BitmapButton_Timing1, color_mgr.GetColor("Timing1"));
	SetButtonColor( BitmapButton_Timing2, color_mgr.GetColor("Timing2"));
	SetButtonColor( BitmapButton_Timing3, color_mgr.GetColor("Timing3"));
	SetButtonColor( BitmapButton_Timing4, color_mgr.GetColor("Timing4"));
	SetButtonColor( BitmapButton_Timing5, color_mgr.GetColor("Timing5"));
	SetButtonColor( BitmapButton_TimingDefault, color_mgr.GetColor("TimingDefault"));
	SetButtonColor( BitmapButton_EffectDefault, color_mgr.GetColor("EffectDefault"));
	SetButtonColor( BitmapButton_EffectSelected, color_mgr.GetColor("EffectSelected"));
	SetButtonColor( BitmapButton_ReferenceEffect, color_mgr.GetColor("ReferenceEffect"));
	SetButtonColor( BitmapButton_RowHeader, color_mgr.GetColor("RowHeader"));
	SetButtonColor( BitmapButton_RowHeaderSelected, color_mgr.GetColor("RowHeaderSelected"));
	SetButtonColor( BitmapButton_Gridlines, color_mgr.GetColor("Gridlines"));
	SetButtonColor( BitmapButton_Labels, color_mgr.GetColor("Labels"));
	SetButtonColor( BitmapButton_LabelOutline, color_mgr.GetColor("LabelOutline"));
	SetButtonColor( BitmapButton_Phrases, color_mgr.GetColor("Phrases"));
	SetButtonColor( BitmapButton_Words, color_mgr.GetColor("Words"));
	SetButtonColor( BitmapButton_Phonemes, color_mgr.GetColor("Phonemes"));
	//SetButtonColor( BitmapButton_ModelSelected, color_mgr.GetColor("ModelSelected"));
}

void ColorManagerDialog::SetMainSequencer(MainSequencer* sequencer)
{
    main_sequencer = sequencer;
}

void ColorManagerDialog::SetButtonColor(wxBitmapButton* btn, const wxColour c)
{
    btn->SetBackgroundColour(c);
    btn->SetForegroundColour(c);

    wxImage image(36,18);
    image.SetRGB(wxRect(0,0,36,18), c.Red(), c.Green(), c.Blue());
    wxBitmap bmp(image);

    btn->SetBitmap(bmp);
}

void ColorManagerDialog::SetButtonColor(wxBitmapButton* btn, const xlColor color)
{
    wxColour c = (wxColour)(color);
    SetButtonColor(btn, c);
}

void ColorManagerDialog::ColorButtonSelected(wxCommandEvent& event)
{
    wxBitmapButton * button = (wxBitmapButton*)event.GetEventObject();
    wxString name = button->GetName();

    wxColour color = button->GetBackgroundColour();
    wxColourData colorData;
    colorData.SetColour(color);
    wxColourDialog dialog(this, &colorData);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxColourData retData = dialog.GetColourData();
        color = retData.GetColour();
        SetButtonColor(button, color);
        xlColor c(color);
        color_mgr.SetNewColor(name.ToStdString(), c);
        RefreshColors();
        color_mgr->SetDirty();
    }
}

void ColorManagerDialog::OnButton_Reset_DefaultsClick(wxCommandEvent& event)
{
    if (wxMessageBox("Are you sure you want to reset all colors to the defaults?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO)
    {
        return;
    }

    color_mgr.ResetDefaults();
    UpdateButtonColors();
    RefreshColors();
    color_mgr->SetDirty();
}

void ColorManagerDialog::RefreshColors()
{
    main_sequencer->PanelEffectGrid->ForceRefresh();
    main_sequencer->PanelRowHeadings->Refresh();
}

void ColorManagerDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndModal(wxID_OK);
}

void ColorManagerDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    color_mgr.RestoreSnapshot();
    RefreshColors();
    EndModal(wxID_CANCEL);
}

