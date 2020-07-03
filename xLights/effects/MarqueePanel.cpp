/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(MarqueePanel)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/gbsizer.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/notebook.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

#include "MarqueePanel.h"
#include "EffectPanelUtils.h"
#include "MarqueeEffect.h"

//(*IdInit(MarqueePanel)
const long MarqueePanel::ID_STATICTEXT_Marquee_Band_Size = wxNewId();
const long MarqueePanel::ID_SLIDER_Marquee_Band_Size = wxNewId();
const long MarqueePanel::ID_VALUECURVE_Marquee_Band_Size = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_Marquee_Band_Size = wxNewId();
const long MarqueePanel::ID_BITMAPBUTTON_SLIDER_Marquee_Band_Size = wxNewId();
const long MarqueePanel::ID_STATICTEXT_Marquee_Skip_Size = wxNewId();
const long MarqueePanel::ID_SLIDER_Marquee_Skip_Size = wxNewId();
const long MarqueePanel::ID_VALUECURVE_Marquee_Skip_Size = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_Marquee_Skip_Size = wxNewId();
const long MarqueePanel::ID_BITMAPBUTTON_SLIDER_Marquee_Skip_Size = wxNewId();
const long MarqueePanel::ID_STATICTEXT_Marquee_Thickness = wxNewId();
const long MarqueePanel::ID_SLIDER_Marquee_Thickness = wxNewId();
const long MarqueePanel::ID_VALUECURVE_Marquee_Thickness = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_Marquee_Thickness = wxNewId();
const long MarqueePanel::ID_BITMAPBUTTON_SLIDER_Marquee_Thickness = wxNewId();
const long MarqueePanel::ID_STATICTEXT_Marquee_Stagger = wxNewId();
const long MarqueePanel::ID_SLIDER_Marquee_Stagger = wxNewId();
const long MarqueePanel::ID_VALUECURVE_Marquee_Stagger = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_Marquee_Stagger = wxNewId();
const long MarqueePanel::ID_BITMAPBUTTON_SLIDER_Marquee_Stagger = wxNewId();
const long MarqueePanel::ID_STATICTEXT_Marquee_Speed = wxNewId();
const long MarqueePanel::ID_SLIDER_Marquee_Speed = wxNewId();
const long MarqueePanel::ID_VALUECURVE_Marquee_Speed = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_Marquee_Speed = wxNewId();
const long MarqueePanel::ID_BITMAPBUTTON_SLIDER_Marquee_Speed = wxNewId();
const long MarqueePanel::ID_STATICTEXT_Marquee_Start = wxNewId();
const long MarqueePanel::ID_SLIDER_Marquee_Start = wxNewId();
const long MarqueePanel::ID_VALUECURVE_Marquee_Start = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_Marquee_Start = wxNewId();
const long MarqueePanel::ID_BITMAPBUTTON_SLIDER_Marquee_Start = wxNewId();
const long MarqueePanel::ID_CHECKBOX_Marquee_Reverse = wxNewId();
const long MarqueePanel::ID_PANEL_Marquee_Settings = wxNewId();
const long MarqueePanel::ID_STATICTEXT_Marquee_ScaleX = wxNewId();
const long MarqueePanel::ID_SLIDER_Marquee_ScaleX = wxNewId();
const long MarqueePanel::ID_VALUECURVE_Marquee_ScaleX = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_Marquee_ScaleX = wxNewId();
const long MarqueePanel::ID_BITMAPBUTTON_SLIDER_Marquee_ScaleX = wxNewId();
const long MarqueePanel::ID_STATICTEXT_Marquee_ScaleY = wxNewId();
const long MarqueePanel::ID_SLIDER_Marquee_ScaleY = wxNewId();
const long MarqueePanel::ID_VALUECURVE_Marquee_ScaleY = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_Marquee_ScaleY = wxNewId();
const long MarqueePanel::ID_BITMAPBUTTON_SLIDER_Marquee_ScaleY = wxNewId();
const long MarqueePanel::ID_STATICLINE1 = wxNewId();
const long MarqueePanel::ID_CHECKBOX_Marquee_PixelOffsets = wxNewId();
const long MarqueePanel::ID_STATICTEXT_MarqueeXC = wxNewId();
const long MarqueePanel::ID_SLIDER_MarqueeXC = wxNewId();
const long MarqueePanel::ID_CHECKBOX_Marquee_WrapX = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_MarqueeXC = wxNewId();
const long MarqueePanel::ID_VALUECURVE_MarqueeXC = wxNewId();
const long MarqueePanel::ID_STATICTEXT_MarqueeYC = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_MarqueeYC = wxNewId();
const long MarqueePanel::ID_SLIDER_MarqueeYC = wxNewId();
const long MarqueePanel::ID_VALUECURVE_MarqueeYC = wxNewId();
const long MarqueePanel::ID_CHECKBOX_Marquee_WrapY = wxNewId();
const long MarqueePanel::ID_PANEL_Marquee_Position = wxNewId();
const long MarqueePanel::ID_NOTEBOOK_Marquee = wxNewId();
//*)

BEGIN_EVENT_TABLE(MarqueePanel,wxPanel)
	//(*EventTable(MarqueePanel)
	//*)
END_EVENT_TABLE()

MarqueePanel::MarqueePanel(wxWindow* parent)
{
	//(*Initialize(MarqueePanel)
	BulkEditTextCtrl* TextCtrl_MarqueeXC;
	BulkEditTextCtrl* TextCtrl_MarqueeYC;
	BulkEditTextCtrl* TextCtrl_Marquee_Band_Size;
	BulkEditTextCtrl* TextCtrl_Marquee_ScaleX;
	BulkEditTextCtrl* TextCtrl_Marquee_ScaleY;
	BulkEditTextCtrl* TextCtrl_Marquee_Skip_Size;
	BulkEditTextCtrl* TextCtrl_Marquee_Speed;
	BulkEditTextCtrl* TextCtrl_Marquee_Stagger;
	BulkEditTextCtrl* TextCtrl_Marquee_Start;
	BulkEditTextCtrl* TextCtrl_Marquee_Thickness;
	wxFlexGridSizer* FlexGridSizer135;
	wxFlexGridSizer* FlexGridSizer136;
	wxFlexGridSizer* FlexGridSizer137;
	wxFlexGridSizer* FlexGridSizer138;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer23;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer58;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxGridBagSizer* GridBagSizer5;
	xlLockButton* BitmapButton_Marquee_Band_Size;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer23 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer23->AddGrowableCol(0);
	Notebook_Marquee = new wxNotebook(this, ID_NOTEBOOK_Marquee, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_Marquee"));
	Panel_Marquee_Settings = new wxPanel(Notebook_Marquee, ID_PANEL_Marquee_Settings, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Marquee_Settings"));
	FlexGridSizer58 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer58->AddGrowableCol(1);
	StaticText116 = new wxStaticText(Panel_Marquee_Settings, ID_STATICTEXT_Marquee_Band_Size, _("Band Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Marquee_Band_Size"));
	FlexGridSizer58->Add(StaticText116, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Marquee_Band_Size = new BulkEditSlider(Panel_Marquee_Settings, ID_SLIDER_Marquee_Band_Size, 3, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Marquee_Band_Size"));
	FlexGridSizer1->Add(Slider_Marquee_Band_Size, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Marquee_Band_SizeVC = new BulkEditValueCurveButton(Panel_Marquee_Settings, ID_VALUECURVE_Marquee_Band_Size, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Marquee_Band_Size"));
	FlexGridSizer1->Add(BitmapButton_Marquee_Band_SizeVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer58->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Marquee_Band_Size = new BulkEditTextCtrl(Panel_Marquee_Settings, IDD_TEXTCTRL_Marquee_Band_Size, _("3"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Settings,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Marquee_Band_Size"));
	TextCtrl_Marquee_Band_Size->SetMaxLength(3);
	FlexGridSizer58->Add(TextCtrl_Marquee_Band_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Marquee_Band_Size = new xlLockButton(Panel_Marquee_Settings, ID_BITMAPBUTTON_SLIDER_Marquee_Band_Size, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Marquee_Band_Size"));
	BitmapButton_Marquee_Band_Size->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer58->Add(BitmapButton_Marquee_Band_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText194 = new wxStaticText(Panel_Marquee_Settings, ID_STATICTEXT_Marquee_Skip_Size, _("Skip Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Marquee_Skip_Size"));
	FlexGridSizer58->Add(StaticText194, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Marquee_Skip_Size = new BulkEditSlider(Panel_Marquee_Settings, ID_SLIDER_Marquee_Skip_Size, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Marquee_Skip_Size"));
	FlexGridSizer2->Add(Slider_Marquee_Skip_Size, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Marquee_Skip_SizeVC = new BulkEditValueCurveButton(Panel_Marquee_Settings, ID_VALUECURVE_Marquee_Skip_Size, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Marquee_Skip_Size"));
	FlexGridSizer2->Add(BitmapButton_Marquee_Skip_SizeVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer58->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Marquee_Skip_Size = new BulkEditTextCtrl(Panel_Marquee_Settings, IDD_TEXTCTRL_Marquee_Skip_Size, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Settings,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Marquee_Skip_Size"));
	TextCtrl_Marquee_Skip_Size->SetMaxLength(3);
	FlexGridSizer58->Add(TextCtrl_Marquee_Skip_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Marquee_Skip_Size = new xlLockButton(Panel_Marquee_Settings, ID_BITMAPBUTTON_SLIDER_Marquee_Skip_Size, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Marquee_Skip_Size"));
	BitmapButton_Marquee_Skip_Size->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer58->Add(BitmapButton_Marquee_Skip_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText195 = new wxStaticText(Panel_Marquee_Settings, ID_STATICTEXT_Marquee_Thickness, _("Thickness:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Marquee_Thickness"));
	FlexGridSizer58->Add(StaticText195, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Marquee_Thickness = new BulkEditSlider(Panel_Marquee_Settings, ID_SLIDER_Marquee_Thickness, 1, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Marquee_Thickness"));
	FlexGridSizer3->Add(Slider_Marquee_Thickness, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Marquee_ThicknessVC = new BulkEditValueCurveButton(Panel_Marquee_Settings, ID_VALUECURVE_Marquee_Thickness, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Marquee_Thickness"));
	FlexGridSizer3->Add(BitmapButton_Marquee_ThicknessVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer58->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Marquee_Thickness = new BulkEditTextCtrl(Panel_Marquee_Settings, IDD_TEXTCTRL_Marquee_Thickness, _("1"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Settings,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Marquee_Thickness"));
	TextCtrl_Marquee_Thickness->SetMaxLength(3);
	FlexGridSizer58->Add(TextCtrl_Marquee_Thickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Marquee_Thickness = new xlLockButton(Panel_Marquee_Settings, ID_BITMAPBUTTON_SLIDER_Marquee_Thickness, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Marquee_Thickness"));
	BitmapButton_Marquee_Thickness->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer58->Add(BitmapButton_Marquee_Thickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText62 = new wxStaticText(Panel_Marquee_Settings, ID_STATICTEXT_Marquee_Stagger, _("Stagger:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Marquee_Stagger"));
	FlexGridSizer58->Add(StaticText62, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	Slider_Marquee_Stagger = new BulkEditSlider(Panel_Marquee_Settings, ID_SLIDER_Marquee_Stagger, 0, 0, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Marquee_Stagger"));
	FlexGridSizer4->Add(Slider_Marquee_Stagger, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Marquee_StaggerVC = new BulkEditValueCurveButton(Panel_Marquee_Settings, ID_VALUECURVE_Marquee_Stagger, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Marquee_Stagger"));
	FlexGridSizer4->Add(BitmapButton_Marquee_StaggerVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer58->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Marquee_Stagger = new BulkEditTextCtrl(Panel_Marquee_Settings, IDD_TEXTCTRL_Marquee_Stagger, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Settings,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Marquee_Stagger"));
	TextCtrl_Marquee_Stagger->SetMaxLength(3);
	FlexGridSizer58->Add(TextCtrl_Marquee_Stagger, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Marquee_Stagger = new xlLockButton(Panel_Marquee_Settings, ID_BITMAPBUTTON_SLIDER_Marquee_Stagger, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Marquee_Stagger"));
	BitmapButton_Marquee_Stagger->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer58->Add(BitmapButton_Marquee_Stagger, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText196 = new wxStaticText(Panel_Marquee_Settings, ID_STATICTEXT_Marquee_Speed, _("Speed:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Marquee_Speed"));
	FlexGridSizer58->Add(StaticText196, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	Slider_Marquee_Speed = new BulkEditSlider(Panel_Marquee_Settings, ID_SLIDER_Marquee_Speed, 3, 0, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Marquee_Speed"));
	FlexGridSizer5->Add(Slider_Marquee_Speed, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Marquee_SpeedVC = new BulkEditValueCurveButton(Panel_Marquee_Settings, ID_VALUECURVE_Marquee_Speed, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Marquee_Speed"));
	FlexGridSizer5->Add(BitmapButton_Marquee_SpeedVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer58->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Marquee_Speed = new BulkEditTextCtrl(Panel_Marquee_Settings, IDD_TEXTCTRL_Marquee_Speed, _("3"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Settings,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Marquee_Speed"));
	TextCtrl_Marquee_Speed->SetMaxLength(3);
	FlexGridSizer58->Add(TextCtrl_Marquee_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Marquee_Speed = new xlLockButton(Panel_Marquee_Settings, ID_BITMAPBUTTON_SLIDER_Marquee_Speed, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Marquee_Speed"));
	BitmapButton_Marquee_Speed->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer58->Add(BitmapButton_Marquee_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText244 = new wxStaticText(Panel_Marquee_Settings, ID_STATICTEXT_Marquee_Start, _("Start Pos:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Marquee_Start"));
	FlexGridSizer58->Add(StaticText244, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	Slider_Marquee_Start = new BulkEditSlider(Panel_Marquee_Settings, ID_SLIDER_Marquee_Start, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Marquee_Start"));
	FlexGridSizer6->Add(Slider_Marquee_Start, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Marquee_StartVC = new BulkEditValueCurveButton(Panel_Marquee_Settings, ID_VALUECURVE_Marquee_Start, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Marquee_Start"));
	FlexGridSizer6->Add(BitmapButton_Marquee_StartVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer58->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Marquee_Start = new BulkEditTextCtrl(Panel_Marquee_Settings, IDD_TEXTCTRL_Marquee_Start, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Settings,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Marquee_Start"));
	TextCtrl_Marquee_Start->SetMaxLength(3);
	FlexGridSizer58->Add(TextCtrl_Marquee_Start, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Marquee_Start = new xlLockButton(Panel_Marquee_Settings, ID_BITMAPBUTTON_SLIDER_Marquee_Start, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Marquee_Start"));
	BitmapButton_Marquee_Start->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer58->Add(BitmapButton_Marquee_Start, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer58->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Marquee_Reverse = new BulkEditCheckBox(Panel_Marquee_Settings, ID_CHECKBOX_Marquee_Reverse, _("Reverse:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxDefaultValidator, _T("ID_CHECKBOX_Marquee_Reverse"));
	CheckBox_Marquee_Reverse->SetValue(false);
	FlexGridSizer58->Add(CheckBox_Marquee_Reverse, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Panel_Marquee_Settings->SetSizer(FlexGridSizer58);
	FlexGridSizer58->Fit(Panel_Marquee_Settings);
	FlexGridSizer58->SetSizeHints(Panel_Marquee_Settings);
	Panel_Marquee_Position = new wxPanel(Notebook_Marquee, ID_PANEL_Marquee_Position, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Marquee_Position"));
	FlexGridSizer135 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer138 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer138->AddGrowableCol(1);
	StaticText198 = new wxStaticText(Panel_Marquee_Position, ID_STATICTEXT_Marquee_ScaleX, _("Scale X:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Marquee_ScaleX"));
	FlexGridSizer138->Add(StaticText198, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	Slider_Marquee_ScaleX = new BulkEditSlider(Panel_Marquee_Position, ID_SLIDER_Marquee_ScaleX, 100, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Marquee_ScaleX"));
	FlexGridSizer7->Add(Slider_Marquee_ScaleX, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Marquee_ScaleXVC = new BulkEditValueCurveButton(Panel_Marquee_Position, ID_VALUECURVE_Marquee_ScaleX, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Marquee_ScaleX"));
	FlexGridSizer7->Add(BitmapButton_Marquee_ScaleXVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer138->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Marquee_ScaleX = new BulkEditTextCtrl(Panel_Marquee_Position, IDD_TEXTCTRL_Marquee_ScaleX, _("100"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Position,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Marquee_ScaleX"));
	TextCtrl_Marquee_ScaleX->SetMaxLength(3);
	FlexGridSizer138->Add(TextCtrl_Marquee_ScaleX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Marquee_ScaleX = new xlLockButton(Panel_Marquee_Position, ID_BITMAPBUTTON_SLIDER_Marquee_ScaleX, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Marquee_ScaleX"));
	BitmapButton_Marquee_ScaleX->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer138->Add(BitmapButton_Marquee_ScaleX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText199 = new wxStaticText(Panel_Marquee_Position, ID_STATICTEXT_Marquee_ScaleY, _("Scale Y:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Marquee_ScaleY"));
	FlexGridSizer138->Add(StaticText199, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	Slider_Marquee_ScaleY = new BulkEditSlider(Panel_Marquee_Position, ID_SLIDER_Marquee_ScaleY, 100, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Marquee_ScaleY"));
	FlexGridSizer8->Add(Slider_Marquee_ScaleY, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Marquee_ScaleYVC = new BulkEditValueCurveButton(Panel_Marquee_Position, ID_VALUECURVE_Marquee_ScaleY, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Marquee_ScaleY"));
	FlexGridSizer8->Add(BitmapButton_Marquee_ScaleYVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer138->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Marquee_ScaleY = new BulkEditTextCtrl(Panel_Marquee_Position, IDD_TEXTCTRL_Marquee_ScaleY, _("100"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Position,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Marquee_ScaleY"));
	TextCtrl_Marquee_ScaleY->SetMaxLength(3);
	FlexGridSizer138->Add(TextCtrl_Marquee_ScaleY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Marquee_ScaleY = new xlLockButton(Panel_Marquee_Position, ID_BITMAPBUTTON_SLIDER_Marquee_ScaleY, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Marquee_ScaleY"));
	BitmapButton_Marquee_ScaleY->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer138->Add(BitmapButton_Marquee_ScaleY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer135->Add(FlexGridSizer138, 1, wxALL|wxEXPAND, 5);
	StaticLine1 = new wxStaticLine(Panel_Marquee_Position, ID_STATICLINE1, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL, _T("ID_STATICLINE1"));
	FlexGridSizer135->Add(StaticLine1, 1, wxALL|wxEXPAND, 5);
	CheckBox_Marquee_PixelOffsets = new BulkEditCheckBox(Panel_Marquee_Position, ID_CHECKBOX_Marquee_PixelOffsets, _("Offsets In Pixels"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Marquee_PixelOffsets"));
	CheckBox_Marquee_PixelOffsets->SetValue(false);
	FlexGridSizer135->Add(CheckBox_Marquee_PixelOffsets, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer136 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer136->AddGrowableCol(0);
	FlexGridSizer137 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer137->AddGrowableCol(1);
	StaticText200 = new wxStaticText(Panel_Marquee_Position, ID_STATICTEXT_MarqueeXC, _("X-axis Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MarqueeXC"));
	FlexGridSizer137->Add(StaticText200, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_MarqueeXC = new BulkEditSlider(Panel_Marquee_Position, ID_SLIDER_MarqueeXC, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_MarqueeXC"));
	FlexGridSizer137->Add(Slider_MarqueeXC, 1, wxALL|wxEXPAND, 5);
	CheckBox_Marquee_WrapX = new BulkEditCheckBox(Panel_Marquee_Position, ID_CHECKBOX_Marquee_WrapX, _("Wrap X"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Marquee_WrapX"));
	CheckBox_Marquee_WrapX->SetValue(false);
	FlexGridSizer137->Add(CheckBox_Marquee_WrapX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_MarqueeXC = new BulkEditTextCtrl(Panel_Marquee_Position, IDD_TEXTCTRL_MarqueeXC, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Position,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_MarqueeXC"));
	TextCtrl_MarqueeXC->SetMaxLength(3);
	FlexGridSizer137->Add(TextCtrl_MarqueeXC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer137->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_MarqueeXCVC = new BulkEditValueCurveButton(Panel_Marquee_Position, ID_VALUECURVE_MarqueeXC, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_MarqueeXC"));
	FlexGridSizer137->Add(BitmapButton_MarqueeXCVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer136->Add(FlexGridSizer137, 1, wxALL|wxEXPAND, 5);
	GridBagSizer5 = new wxGridBagSizer(0, 0);
	StaticText201 = new wxStaticText(Panel_Marquee_Position, ID_STATICTEXT_MarqueeYC, _("Y-axis Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MarqueeYC"));
	GridBagSizer5->Add(StaticText201, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_MarqueeYC = new BulkEditTextCtrl(Panel_Marquee_Position, IDD_TEXTCTRL_MarqueeYC, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Position,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_MarqueeYC"));
	TextCtrl_MarqueeYC->SetMaxLength(3);
	GridBagSizer5->Add(TextCtrl_MarqueeYC, wxGBPosition(2, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_MarqueeYC = new BulkEditSlider(Panel_Marquee_Position, ID_SLIDER_MarqueeYC, 0, -100, 100, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER_MarqueeYC"));
	GridBagSizer5->Add(Slider_MarqueeYC, wxGBPosition(0, 1), wxGBSpan(5, 1), wxALL|wxEXPAND, 5);
	BitmapButton_MarqueeYCVC = new BulkEditValueCurveButton(Panel_Marquee_Position, ID_VALUECURVE_MarqueeYC, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_MarqueeYC"));
	GridBagSizer5->Add(BitmapButton_MarqueeYCVC, wxGBPosition(4, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox1 = new BulkEditCheckBox(Panel_Marquee_Position, ID_CHECKBOX_Marquee_WrapY, _("Wrap Y"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Marquee_WrapY"));
	CheckBox1->SetValue(false);
	GridBagSizer5->Add(CheckBox1, wxGBPosition(3, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer136->Add(GridBagSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer135->Add(FlexGridSizer136, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_Marquee_Position->SetSizer(FlexGridSizer135);
	FlexGridSizer135->Fit(Panel_Marquee_Position);
	FlexGridSizer135->SetSizeHints(Panel_Marquee_Position);
	Notebook_Marquee->AddPage(Panel_Marquee_Settings, _("Settings"), false);
	Notebook_Marquee->AddPage(Panel_Marquee_Position, _("Position"), false);
	FlexGridSizer23->Add(Notebook_Marquee, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer23);
	FlexGridSizer23->Fit(this);
	FlexGridSizer23->SetSizeHints(this);

	Connect(ID_VALUECURVE_Marquee_Band_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Marquee_Band_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Marquee_Skip_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Marquee_Skip_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Marquee_Thickness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Marquee_Thickness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Marquee_Stagger,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Marquee_Stagger,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Marquee_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Marquee_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Marquee_Start,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Marquee_Start,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Marquee_ScaleX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Marquee_ScaleX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Marquee_ScaleY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Marquee_ScaleY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_MarqueeXC,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_MarqueeYC,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnVCButtonClick);
	//*)
    SetName("ID_PANEL_MARQUEE");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&MarqueePanel::OnVCChanged, 0, this);

    BitmapButton_MarqueeXCVC->GetValue()->SetLimits(MARQUEE_XC_MIN, MARQUEE_XC_MAX);
    BitmapButton_MarqueeYCVC->GetValue()->SetLimits(MARQUEE_YC_MIN, MARQUEE_YC_MAX);
    BitmapButton_Marquee_ScaleXVC->GetValue()->SetLimits(MARQUEE_SCALEX_MIN, MARQUEE_SCALEX_MAX);
    BitmapButton_Marquee_ScaleYVC->GetValue()->SetLimits(MARQUEE_SCALEY_MIN, MARQUEE_SCALEY_MAX);
    BitmapButton_Marquee_Band_SizeVC->GetValue()->SetLimits(MARQUEE_BAND_SIZE_MIN, MARQUEE_BAND_SIZE_MAX);
    BitmapButton_Marquee_Skip_SizeVC->GetValue()->SetLimits(MARQUEE_SKIP_SIZE_MIN, MARQUEE_SKIP_SIZE_MAX);
    BitmapButton_Marquee_SpeedVC->GetValue()->SetLimits(MARQUEE_SPEED_MIN, MARQUEE_SPEED_MAX);
    BitmapButton_Marquee_StaggerVC->GetValue()->SetLimits(MARQUEE_STAGGER_MIN, MARQUEE_STAGGER_MAX);
    BitmapButton_Marquee_StartVC->GetValue()->SetLimits(MARQUEE_START_MIN, MARQUEE_START_MAX);
    BitmapButton_Marquee_ThicknessVC->GetValue()->SetLimits(MARQUEE_THICKNESS_MIN, MARQUEE_THICKNESS_MAX);
}

MarqueePanel::~MarqueePanel()
{
	//(*Destroy(MarqueePanel)
	//*)
}

PANEL_EVENT_HANDLERS(MarqueePanel)
