#include "ColorManagerDialog.h"
#include <wx/colour.h>
#include <wx/colourdata.h>
#include <wx/colordlg.h>
#include "ColorManager.h"
#include "xLightsMain.h"

//(*InternalHeaders(ColorManagerDialog)
#include <wx/bitmap.h>
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/button.h>
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
const long ColorManagerDialog::ID_STATICTEXT7 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_EffectSelected = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT9 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_ReferenceEffect = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT10 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_HeaderColor = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT11 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_HeaderSelectedColor = wxNewId();
const long ColorManagerDialog::ID_STATICTEXT8 = wxNewId();
const long ColorManagerDialog::ID_BITMAPBUTTON_ModelSelected = wxNewId();
//*)

BEGIN_EVENT_TABLE(ColorManagerDialog,wxDialog)
	//(*EventTable(ColorManagerDialog)
	//*)
END_EVENT_TABLE()

ColorManagerDialog::ColorManagerDialog(wxWindow* parent,ColorManager& color_mgr_,wxWindowID id,const wxPoint& pos,const wxSize& size)
: color_mgr(color_mgr_)
{
	//(*Initialize(ColorManagerDialog)
	wxStaticBoxSizer* StaticBoxSizer2;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer9;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticBoxSizer* StaticBoxSizer3;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer6;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Color Manager"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	wxFont StaticText1Font(16,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText1->SetFont(StaticText1Font);
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBitmap1 = new wxStaticBitmap(this, ID_STATICBITMAP1, wxBitmap(wxImage(_T("C:\\Software Development\\Git\\xLights\\include\\ColorFan.jpg"))), wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER, _T("ID_STATICBITMAP1"));
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
	FlexGridSizer6->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Timing1 = new wxBitmapButton(this, ID_BITMAPBUTTON_Timing1, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Timing1"));
	FlexGridSizer6->Add(BitmapButton_Timing1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Timing 2"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer6->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Timing2 = new wxBitmapButton(this, ID_BITMAPBUTTON_Timing2, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Timing2"));
	FlexGridSizer6->Add(BitmapButton_Timing2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Timing 3"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer6->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Timing3 = new wxBitmapButton(this, ID_BITMAPBUTTON_Timing3, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Timing3"));
	FlexGridSizer6->Add(BitmapButton_Timing3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Timing 4"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer6->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Timing4 = new wxBitmapButton(this, ID_BITMAPBUTTON_Timing4, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Timing4"));
	FlexGridSizer6->Add(BitmapButton_Timing4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Timing 5"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer6->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Timing5 = new wxBitmapButton(this, ID_BITMAPBUTTON_Timing5, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Timing5"));
	FlexGridSizer6->Add(BitmapButton_Timing5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxVERTICAL, this, _("Effect Grid"));
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Effect Selected"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer8->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_EffectSelected = new wxBitmapButton(this, ID_BITMAPBUTTON_EffectSelected, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_EffectSelected"));
	FlexGridSizer8->Add(BitmapButton_EffectSelected, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("Reference Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer8->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_ReferenceEffect = new wxBitmapButton(this, ID_BITMAPBUTTON_ReferenceEffect, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_ReferenceEffect"));
	FlexGridSizer8->Add(BitmapButton_ReferenceEffect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("Header Color"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer8->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_HeaderColor = new wxBitmapButton(this, ID_BITMAPBUTTON_HeaderColor, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_HeaderColor"));
	FlexGridSizer8->Add(BitmapButton_HeaderColor, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText11 = new wxStaticText(this, ID_STATICTEXT11, _("Header Selected Color"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	FlexGridSizer8->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_HeaderSelectedColor = new wxBitmapButton(this, ID_BITMAPBUTTON_HeaderSelectedColor, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_HeaderSelectedColor"));
	FlexGridSizer8->Add(BitmapButton_HeaderSelectedColor, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer2->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer3 = new wxStaticBoxSizer(wxVERTICAL, this, _("Layout Tab"));
	FlexGridSizer9 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Reserved"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer9->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_ModelSelected = new wxBitmapButton(this, ID_BITMAPBUTTON_ModelSelected, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_ModelSelected"));
	FlexGridSizer9->Add(BitmapButton_ModelSelected, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer3->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(StaticBoxSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer10->Add(StdDialogButtonSizer1, 1, wxALL, 5);
	FlexGridSizer1->Add(FlexGridSizer10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_Timing1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_Timing2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_Timing3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_Timing4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_Timing5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_EffectSelected,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_ReferenceEffect,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_HeaderColor,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_HeaderSelectedColor,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	Connect(ID_BITMAPBUTTON_ModelSelected,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorManagerDialog::ColorButtonSelected);
	//*)

	SetButtonColor( BitmapButton_Timing1, color_mgr.GetColor("Timing1"));
	SetButtonColor( BitmapButton_Timing2, color_mgr.GetColor("Timing2"));
	SetButtonColor( BitmapButton_Timing3, color_mgr.GetColor("Timing3"));
	SetButtonColor( BitmapButton_Timing4, color_mgr.GetColor("Timing4"));
	SetButtonColor( BitmapButton_Timing5, color_mgr.GetColor("Timing5"));
	SetButtonColor( BitmapButton_EffectSelected, color_mgr.GetColor("EffectSelected"));
	SetButtonColor( BitmapButton_ReferenceEffect, color_mgr.GetColor("ReferenceEffect"));
	SetButtonColor( BitmapButton_ModelSelected, color_mgr.GetColor("ModelSelected"));
	SetButtonColor( BitmapButton_HeaderColor, color_mgr.GetColor("HeaderColor"));
	SetButtonColor( BitmapButton_HeaderSelectedColor, color_mgr.GetColor("HeaderSelectedColor"));

	Layout();
}

ColorManagerDialog::~ColorManagerDialog()
{
	//(*Destroy(ColorManagerDialog)
	//*)
}

void ColorManagerDialog::SetMainSequencer(MainSequencer* sequencer)
{
    main_sequencer = sequencer;
}

void ColorManagerDialog::SetButtonColor(wxBitmapButton* btn, const wxColour* c)
{
    btn->SetBackgroundColour(*c);
    btn->SetForegroundColour(*c);

    wxImage image(36,18);
    image.SetRGB(wxRect(0,0,36,18), c->Red(), c->Green(), c->Blue());
    wxBitmap bmp(image);

    btn->SetBitmap(bmp);
}

void ColorManagerDialog::SetButtonColor(wxBitmapButton* btn, const xlColor* color)
{
    wxColour c = (wxColour)(*color);
    SetButtonColor(btn, &c);
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
        SetButtonColor(button, &color);
        xlColor c(color);
        color_mgr.SetNewColor(name.ToStdString(), &c);
        main_sequencer->PanelEffectGrid->ForceRefresh();
        main_sequencer->PanelRowHeadings->Refresh();
    }
}
