#include "EffectsPanel.h"

//(*InternalHeaders(EffectsPanel)
#include <wx/settings.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EffectsPanel)
const long EffectsPanel::ID_PANEL4 = wxNewId();
const long EffectsPanel::ID_STATICTEXT25 = wxNewId();
const long EffectsPanel::ID_SLIDER_Bars_BarCount = wxNewId();
const long EffectsPanel::ID_STATICTEXT26 = wxNewId();
const long EffectsPanel::ID_CHOICE_Bars_Direction = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Bars_Highlight = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Bars_3D = wxNewId();
const long EffectsPanel::ID_PANEL8 = wxNewId();
const long EffectsPanel::ID_STATICTEXT27 = wxNewId();
const long EffectsPanel::ID_CHOICE_Butterfly_Colors = wxNewId();
const long EffectsPanel::ID_STATICTEXT28 = wxNewId();
const long EffectsPanel::ID_SLIDER_Butterfly_Style = wxNewId();
const long EffectsPanel::ID_STATICTEXT30 = wxNewId();
const long EffectsPanel::ID_SLIDER_Butterfly_Chunks = wxNewId();
const long EffectsPanel::ID_STATICTEXT31 = wxNewId();
const long EffectsPanel::ID_SLIDER_Butterfly_Skip = wxNewId();
const long EffectsPanel::ID_PANEL10 = wxNewId();
const long EffectsPanel::ID_STATICTEXT137 = wxNewId();
const long EffectsPanel::ID_SLIDER_Circles_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT138 = wxNewId();
const long EffectsPanel::ID_SLIDER_Circles_Size = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Circles_Bounce = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Circles_Radial = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Circles_Radial_3D = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Circles_Plasma = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Circles_Collide = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Circles_Random_m = wxNewId();
const long EffectsPanel::ID_PANEL37 = wxNewId();
const long EffectsPanel::ID_STATICTEXT21 = wxNewId();
const long EffectsPanel::ID_SLIDER_ColorWash_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT32 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_ColorWash_HFade = wxNewId();
const long EffectsPanel::ID_STATICTEXT33 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_ColorWash_VFade = wxNewId();
const long EffectsPanel::ID_PANEL5 = wxNewId();
const long EffectsPanel::ID_STATICTEXT6 = wxNewId();
const long EffectsPanel::ID_CHOICE_Curtain_Edge = wxNewId();
const long EffectsPanel::ID_STATICTEXT5 = wxNewId();
const long EffectsPanel::ID_CHOICE_Curtain_Effect = wxNewId();
const long EffectsPanel::ID_STATICTEXT7 = wxNewId();
const long EffectsPanel::ID_SLIDER_Curtain_Swag = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Curtain_Repeat = wxNewId();
const long EffectsPanel::ID_PANEL1 = wxNewId();
const long EffectsPanel::ID_STATICTEXT84 = wxNewId();
const long EffectsPanel::ID_SLIDER_Fire_Height = wxNewId();
const long EffectsPanel::ID_STATICTEXT133 = wxNewId();
const long EffectsPanel::ID_SLIDER_Fire_HueShift = wxNewId();
const long EffectsPanel::ID_STATICTEXT135 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Fire_GrowFire = wxNewId();
const long EffectsPanel::ID_PANEL12 = wxNewId();
const long EffectsPanel::ID_STATICTEXT95 = wxNewId();
const long EffectsPanel::ID_SLIDER_Fireworks_Number_Explosions = wxNewId();
const long EffectsPanel::ID_STATICTEXT92 = wxNewId();
const long EffectsPanel::ID_SLIDER_Fireworks_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT94 = wxNewId();
const long EffectsPanel::ID_SLIDER_Fireworks_Velocity = wxNewId();
const long EffectsPanel::ID_STATICTEXT96 = wxNewId();
const long EffectsPanel::ID_SLIDER_Fireworks_Fade = wxNewId();
const long EffectsPanel::ID_PANEL36 = wxNewId();
const long EffectsPanel::ID_STATICTEXT34 = wxNewId();
const long EffectsPanel::ID_SLIDER_Garlands_Type = wxNewId();
const long EffectsPanel::ID_STATICTEXT35 = wxNewId();
const long EffectsPanel::ID_SLIDER_Garlands_Spacing = wxNewId();
const long EffectsPanel::ID_PANEL14 = wxNewId();
const long EffectsPanel::ID_STATICTEXT36 = wxNewId();
const long EffectsPanel::ID_SLIDER_Life_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT37 = wxNewId();
const long EffectsPanel::ID_SLIDER_Life_Seed = wxNewId();
const long EffectsPanel::ID_PANEL16 = wxNewId();
const long EffectsPanel::ID_STATICTEXT39 = wxNewId();
const long EffectsPanel::ID_CHOICE_Meteors_Type = wxNewId();
const long EffectsPanel::ID_STATICTEXT129 = wxNewId();
const long EffectsPanel::ID_CHOICE_Meteors_Effect = wxNewId();
const long EffectsPanel::ID_STATICTEXT41 = wxNewId();
const long EffectsPanel::ID_SLIDER_Meteors_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT43 = wxNewId();
const long EffectsPanel::ID_SLIDER_Meteors_Length = wxNewId();
const long EffectsPanel::ID_STATICTEXT131 = wxNewId();
const long EffectsPanel::ID_SLIDER_Meteors_Swirl_Intensity = wxNewId();
const long EffectsPanel::ID_PANEL18 = wxNewId();
const long EffectsPanel::ID_STATICTEXT1 = wxNewId();
const long EffectsPanel::ID_SLIDER_Piano_Keyboard = wxNewId();
const long EffectsPanel::ID_STATICTEXT4 = wxNewId();
const long EffectsPanel::ID_PANEL39 = wxNewId();
const long EffectsPanel::ID_BUTTON_PICTURES_FILENAME = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Pictures_Filename = wxNewId();
const long EffectsPanel::ID_STATICTEXT46 = wxNewId();
const long EffectsPanel::ID_CHOICE_Pictures_Direction = wxNewId();
const long EffectsPanel::ID_STATICTEXT97 = wxNewId();
const long EffectsPanel::ID_SLIDER_Pictures_GifSpeed = wxNewId();
const long EffectsPanel::ID_PANEL20 = wxNewId();
const long EffectsPanel::ID_STATICTEXT80 = wxNewId();
const long EffectsPanel::ID_SLIDER_Snowflakes_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT81 = wxNewId();
const long EffectsPanel::ID_SLIDER_Snowflakes_Type = wxNewId();
const long EffectsPanel::ID_PANEL22 = wxNewId();
const long EffectsPanel::ID_STATICTEXT45 = wxNewId();
const long EffectsPanel::ID_SLIDER_Snowstorm_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT51 = wxNewId();
const long EffectsPanel::ID_SLIDER_Snowstorm_Length = wxNewId();
const long EffectsPanel::ID_PANEL24 = wxNewId();
const long EffectsPanel::ID_STATICTEXT38 = wxNewId();
const long EffectsPanel::ID_SLIDER_Spirals_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT40 = wxNewId();
const long EffectsPanel::ID_SLIDER_Spirals_Rotation = wxNewId();
const long EffectsPanel::ID_STATICTEXT42 = wxNewId();
const long EffectsPanel::ID_SLIDER_Spirals_Thickness = wxNewId();
const long EffectsPanel::ID_STATICTEXT44 = wxNewId();
const long EffectsPanel::ID_SLIDER_Spirals_Direction = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Spirals_Blend = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Spirals_3D = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Spirals_Grow = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Spirals_Shrink = wxNewId();
const long EffectsPanel::ID_PANEL26 = wxNewId();
const long EffectsPanel::ID_STATICTEXT88 = wxNewId();
const long EffectsPanel::ID_SLIDER_Spirograph_R = wxNewId();
const long EffectsPanel::ID_STATICTEXT89 = wxNewId();
const long EffectsPanel::ID_SLIDER_Spirograph_r = wxNewId();
const long EffectsPanel::ID_STATICTEXT90 = wxNewId();
const long EffectsPanel::ID_SLIDER_Spirograph_d = wxNewId();
const long EffectsPanel::ID_STATICTEXT93 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Spirograph_Animate = wxNewId();
const long EffectsPanel::ID_STATICTEXT91 = wxNewId();
const long EffectsPanel::ID_PANEL35 = wxNewId();
const long EffectsPanel::ID_STATICTEXT53 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Text_Line1 = wxNewId();
const long EffectsPanel::ID_STATICTEXT54 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Text_Line2 = wxNewId();
const long EffectsPanel::ID_BUTTON_TEXT_FONT1 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Text_Font1 = wxNewId();
const long EffectsPanel::ID_STATICTEXT79 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Dir1 = wxNewId();
const long EffectsPanel::ID_STATICTEXT108 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Effect1 = wxNewId();
const long EffectsPanel::ID_STATICTEXT99 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Count1 = wxNewId();
const long EffectsPanel::ID_STATICTEXT101 = wxNewId();
const long EffectsPanel::ID_SLIDER_Text_Position1 = wxNewId();
const long EffectsPanel::ID_PANEL_Text1 = wxNewId();
const long EffectsPanel::ID_BUTTON_TEXT_FONT2 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Text_Font2 = wxNewId();
const long EffectsPanel::ID_STATICTEXT57 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Dir2 = wxNewId();
const long EffectsPanel::ID_STATICTEXT109 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Effect2 = wxNewId();
const long EffectsPanel::ID_STATICTEXT104 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Count2 = wxNewId();
const long EffectsPanel::ID_STATICTEXT103 = wxNewId();
const long EffectsPanel::ID_SLIDER_Text_Position2 = wxNewId();
const long EffectsPanel::ID_PANEL_Text2 = wxNewId();
const long EffectsPanel::ID_NOTEBOOK_Text1 = wxNewId();
const long EffectsPanel::ID_PANEL7 = wxNewId();
const long EffectsPanel::ID_STATICTEXT87 = wxNewId();
const long EffectsPanel::ID_SLIDER_Tree_Branches = wxNewId();
const long EffectsPanel::ID_PANEL34 = wxNewId();
const long EffectsPanel::ID_STATICTEXT86 = wxNewId();
const long EffectsPanel::ID_SLIDER_Twinkle_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT105 = wxNewId();
const long EffectsPanel::ID_SLIDER_Twinkle_Steps = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Twinkle_Strobe = wxNewId();
const long EffectsPanel::ID_PANEL33 = wxNewId();
const long EffectsPanel::ID_CHOICEBOOK1 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Palette1 = wxNewId();
const long EffectsPanel::ID_BUTTON_Palette1 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Palette2 = wxNewId();
const long EffectsPanel::ID_BUTTON_Palette2 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Palette3 = wxNewId();
const long EffectsPanel::ID_BUTTON_Palette3 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Palette4 = wxNewId();
const long EffectsPanel::ID_BUTTON_Palette4 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Palette5 = wxNewId();
const long EffectsPanel::ID_BUTTON_Palette5 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Palette6 = wxNewId();
const long EffectsPanel::ID_BUTTON_Palette6 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_FitToTime = wxNewId();
const long EffectsPanel::ID_STATICTEXT61 = wxNewId();
const long EffectsPanel::ID_SLIDER_Speed = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Speed = wxNewId();
const long EffectsPanel::ID_STATICTEXT2 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Fadein = wxNewId();
const long EffectsPanel::ID_STATICTEXT3 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Fadeout = wxNewId();
//*)

BEGIN_EVENT_TABLE(EffectsPanel,wxPanel)
	//(*EventTable(EffectsPanel)
	//*)
END_EVENT_TABLE()

EffectsPanel::EffectsPanel(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
{
	//(*Initialize(EffectsPanel)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer71;
	wxFlexGridSizer* FlexGridSizer45;
	wxFlexGridSizer* FlexGridSizer73;
	wxFlexGridSizer* FlexGridSizer38;
	wxFlexGridSizer* FlexGridSizer41;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer80;
	wxFlexGridSizer* FlexGridSizer44;
	wxFlexGridSizer* FlexGridSizer37;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer69;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer74;
	wxFlexGridSizer* FlexGridSizer35;
	wxFlexGridSizer* FlexGridSizer36;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer46;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer43;
	wxFlexGridSizer* FlexGridSizer65;
	wxFlexGridSizer* FlexGridSizer42;
	wxFlexGridSizer* FlexGridSizer72;
	wxFlexGridSizer* FlexGridSizer31;
	wxFlexGridSizer* FlexGridSizer40;
	wxFlexGridSizer* FlexGridSizer39;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	Choicebook1 = new wxChoicebook(this, ID_CHOICEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CHOICEBOOK1"));
	Panel1_None = new wxPanel(Choicebook1, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
	Panel1_Bars = new wxPanel(Choicebook1, ID_PANEL8, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL8"));
	FlexGridSizer35 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer35->AddGrowableCol(1);
	StaticText23 = new wxStaticText(Panel1_Bars, ID_STATICTEXT25, _("Palette Rep"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT25"));
	FlexGridSizer35->Add(StaticText23, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Bars_BarCount = new wxSlider(Panel1_Bars, ID_SLIDER_Bars_BarCount, 1, 1, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Bars_BarCount"));
	FlexGridSizer35->Add(Slider_Bars_BarCount, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText24 = new wxStaticText(Panel1_Bars, ID_STATICTEXT26, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT26"));
	FlexGridSizer35->Add(StaticText24, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Bars_Direction = new wxChoice(Panel1_Bars, ID_CHOICE_Bars_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Bars_Direction"));
	FlexGridSizer35->Add(Choice_Bars_Direction, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_Bars_Highlight = new wxCheckBox(Panel1_Bars, ID_CHECKBOX_Bars_Highlight, _("Highlight"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Bars_Highlight"));
	CheckBox_Bars_Highlight->SetValue(false);
	FlexGridSizer35->Add(CheckBox_Bars_Highlight, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_Bars_3D = new wxCheckBox(Panel1_Bars, ID_CHECKBOX_Bars_3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Bars_3D"));
	CheckBox_Bars_3D->SetValue(false);
	FlexGridSizer35->Add(CheckBox_Bars_3D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Panel1_Bars->SetSizer(FlexGridSizer35);
	FlexGridSizer35->Fit(Panel1_Bars);
	FlexGridSizer35->SetSizeHints(Panel1_Bars);
	Panel1_Butterfly = new wxPanel(Choicebook1, ID_PANEL10, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL10"));
	FlexGridSizer36 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer36->AddGrowableCol(1);
	StaticText25 = new wxStaticText(Panel1_Butterfly, ID_STATICTEXT27, _("Colors"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT27"));
	FlexGridSizer36->Add(StaticText25, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Butterfly_Colors = new wxChoice(Panel1_Butterfly, ID_CHOICE_Butterfly_Colors, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Butterfly_Colors"));
	FlexGridSizer36->Add(Choice_Butterfly_Colors, 1, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText26 = new wxStaticText(Panel1_Butterfly, ID_STATICTEXT28, _("Style"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT28"));
	FlexGridSizer36->Add(StaticText26, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Butterfly_Style = new wxSlider(Panel1_Butterfly, ID_SLIDER_Butterfly_Style, 1, 1, 3, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Butterfly_Style"));
	FlexGridSizer36->Add(Slider_Butterfly_Style, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText27 = new wxStaticText(Panel1_Butterfly, ID_STATICTEXT30, _("Bkgrd Chunks"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT30"));
	FlexGridSizer36->Add(StaticText27, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Butterfly_Chunks = new wxSlider(Panel1_Butterfly, ID_SLIDER_Butterfly_Chunks, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Butterfly_Chunks"));
	FlexGridSizer36->Add(Slider_Butterfly_Chunks, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText28 = new wxStaticText(Panel1_Butterfly, ID_STATICTEXT31, _("Bkgrd Skip"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT31"));
	FlexGridSizer36->Add(StaticText28, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Butterfly_Skip = new wxSlider(Panel1_Butterfly, ID_SLIDER_Butterfly_Skip, 2, 2, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Butterfly_Skip"));
	FlexGridSizer36->Add(Slider_Butterfly_Skip, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Panel1_Butterfly->SetSizer(FlexGridSizer36);
	FlexGridSizer36->Fit(Panel1_Butterfly);
	FlexGridSizer36->SetSizeHints(Panel1_Butterfly);
	Panel1_Circles = new wxPanel(Choicebook1, ID_PANEL37, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL37"));
	FlexGridSizer80 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText136 = new wxStaticText(Panel1_Circles, ID_STATICTEXT137, _("Number of Circles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT137"));
	FlexGridSizer80->Add(StaticText136, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Circles_Count = new wxSlider(Panel1_Circles, ID_SLIDER_Circles_Count, 3, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Circles_Count"));
	FlexGridSizer80->Add(Slider_Circles_Count, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText137 = new wxStaticText(Panel1_Circles, ID_STATICTEXT138, _("Size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT138"));
	FlexGridSizer80->Add(StaticText137, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Circles_Size = new wxSlider(Panel1_Circles, ID_SLIDER_Circles_Size, 5, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Circles_Size"));
	FlexGridSizer80->Add(Slider_Circles_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Circles_Bounce = new wxCheckBox(Panel1_Circles, ID_CHECKBOX_Circles_Bounce, _("Bounce"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Bounce"));
	CheckBox_Circles_Bounce->SetValue(false);
	FlexGridSizer80->Add(CheckBox_Circles_Bounce, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Circles_Radial = new wxCheckBox(Panel1_Circles, ID_CHECKBOX_Circles_Radial, _("Radial"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Radial"));
	CheckBox_Circles_Radial->SetValue(false);
	FlexGridSizer80->Add(CheckBox_Circles_Radial, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Circles_Radial_3D = new wxCheckBox(Panel1_Circles, ID_CHECKBOX_Circles_Radial_3D, _("Radial 3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Radial_3D"));
	CheckBox_Circles_Radial_3D->SetValue(false);
	FlexGridSizer80->Add(CheckBox_Circles_Radial_3D, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Circles_Plasma = new wxCheckBox(Panel1_Circles, ID_CHECKBOX_Circles_Plasma, _("Plasma"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Plasma"));
	CheckBox_Circles_Plasma->SetValue(false);
	FlexGridSizer80->Add(CheckBox_Circles_Plasma, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Circles_Collide = new wxCheckBox(Panel1_Circles, ID_CHECKBOX_Circles_Collide, _("Collide"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Collide"));
	CheckBox_Circles_Collide->SetValue(false);
	CheckBox_Circles_Collide->Hide();
	FlexGridSizer80->Add(CheckBox_Circles_Collide, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Circles_Random_m = new wxCheckBox(Panel1_Circles, ID_CHECKBOX_Circles_Random_m, _("Random Motion"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Random_m"));
	CheckBox_Circles_Random_m->SetValue(false);
	CheckBox_Circles_Random_m->Hide();
	FlexGridSizer80->Add(CheckBox_Circles_Random_m, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Panel1_Circles->SetSizer(FlexGridSizer80);
	FlexGridSizer80->Fit(Panel1_Circles);
	FlexGridSizer80->SetSizeHints(Panel1_Circles);
	Panel1_ColorWash = new wxPanel(Choicebook1, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
	FlexGridSizer37 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer37->AddGrowableCol(1);
	StaticText5 = new wxStaticText(Panel1_ColorWash, ID_STATICTEXT21, _("Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT21"));
	FlexGridSizer37->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_ColorWash_Count = new wxSlider(Panel1_ColorWash, ID_SLIDER_ColorWash_Count, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_ColorWash_Count"));
	FlexGridSizer37->Add(Slider_ColorWash_Count, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText30 = new wxStaticText(Panel1_ColorWash, ID_STATICTEXT32, _("Horizontal"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT32"));
	FlexGridSizer37->Add(StaticText30, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_ColorWash_HFade = new wxCheckBox(Panel1_ColorWash, ID_CHECKBOX_ColorWash_HFade, _("Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash_HFade"));
	CheckBox_ColorWash_HFade->SetValue(false);
	FlexGridSizer37->Add(CheckBox_ColorWash_HFade, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText31 = new wxStaticText(Panel1_ColorWash, ID_STATICTEXT33, _("Vertical"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT33"));
	FlexGridSizer37->Add(StaticText31, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_ColorWash_VFade = new wxCheckBox(Panel1_ColorWash, ID_CHECKBOX_ColorWash_VFade, _("Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash_VFade"));
	CheckBox_ColorWash_VFade->SetValue(false);
	FlexGridSizer37->Add(CheckBox_ColorWash_VFade, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Panel1_ColorWash->SetSizer(FlexGridSizer37);
	FlexGridSizer37->Fit(Panel1_ColorWash);
	FlexGridSizer37->SetSizeHints(Panel1_ColorWash);
	Panel1_Curtain = new wxPanel(Choicebook1, ID_PANEL1, wxPoint(150,14), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer7->AddGrowableCol(1);
	StaticText7 = new wxStaticText(Panel1_Curtain, ID_STATICTEXT6, _("Curtain Edge"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer7->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Curtain_Edge = new wxChoice(Panel1_Curtain, ID_CHOICE_Curtain_Edge, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Curtain_Edge"));
	FlexGridSizer7->Add(Choice_Curtain_Edge, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(Panel1_Curtain, ID_STATICTEXT5, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer7->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Curtain_Effect = new wxChoice(Panel1_Curtain, ID_CHOICE_Curtain_Effect, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Curtain_Effect"));
	FlexGridSizer7->Add(Choice_Curtain_Effect, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText8 = new wxStaticText(Panel1_Curtain, ID_STATICTEXT7, _("Swag Width"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer7->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Curtain_Swag = new wxSlider(Panel1_Curtain, ID_SLIDER_Curtain_Swag, 3, 0, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Curtain_Swag"));
	FlexGridSizer7->Add(Slider_Curtain_Swag, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Curtain_Repeat = new wxCheckBox(Panel1_Curtain, ID_CHECKBOX_Curtain_Repeat, _("Repeat"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Curtain_Repeat"));
	CheckBox_Curtain_Repeat->SetValue(false);
	FlexGridSizer7->Add(CheckBox_Curtain_Repeat, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Panel1_Curtain->SetSizer(FlexGridSizer7);
	FlexGridSizer7->Fit(Panel1_Curtain);
	FlexGridSizer7->SetSizeHints(Panel1_Curtain);
	Panel1_Fire = new wxPanel(Choicebook1, ID_PANEL12, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL12"));
	FlexGridSizer38 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer38->AddGrowableCol(1);
	StaticText83 = new wxStaticText(Panel1_Fire, ID_STATICTEXT84, _("Height"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT84"));
	FlexGridSizer38->Add(StaticText83, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Fire_Height = new wxSlider(Panel1_Fire, ID_SLIDER_Fire_Height, 50, 10, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fire_Height"));
	FlexGridSizer38->Add(Slider_Fire_Height, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText132 = new wxStaticText(Panel1_Fire, ID_STATICTEXT133, _("Hue Shift"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT133"));
	FlexGridSizer38->Add(StaticText132, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Fire_HueShift = new wxSlider(Panel1_Fire, ID_SLIDER_Fire_HueShift, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fire_HueShift"));
	FlexGridSizer38->Add(Slider_Fire_HueShift, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText134 = new wxStaticText(Panel1_Fire, ID_STATICTEXT135, _("Grow Fire\?"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT135"));
	FlexGridSizer38->Add(StaticText134, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_Fire_GrowFire = new wxCheckBox(Panel1_Fire, ID_CHECKBOX_Fire_GrowFire, _("Yes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fire_GrowFire"));
	CheckBox_Fire_GrowFire->SetValue(false);
	FlexGridSizer38->Add(CheckBox_Fire_GrowFire, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Panel1_Fire->SetSizer(FlexGridSizer38);
	FlexGridSizer38->Fit(Panel1_Fire);
	FlexGridSizer38->SetSizeHints(Panel1_Fire);
	Panel1_Fireworks = new wxPanel(Choicebook1, ID_PANEL36, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL36"));
	FlexGridSizer73 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText94 = new wxStaticText(Panel1_Fireworks, ID_STATICTEXT95, _("Number of Explosions"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT95"));
	FlexGridSizer73->Add(StaticText94, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Number_Explosions = new wxSlider(Panel1_Fireworks, ID_SLIDER_Fireworks_Number_Explosions, 10, 1, 95, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Number_Explosions"));
	FlexGridSizer73->Add(Slider_Fireworks_Number_Explosions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText91 = new wxStaticText(Panel1_Fireworks, ID_STATICTEXT92, _("Particles in Explosion"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT92"));
	FlexGridSizer73->Add(StaticText91, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Count = new wxSlider(Panel1_Fireworks, ID_SLIDER_Fireworks_Count, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Count"));
	FlexGridSizer73->Add(Slider_Fireworks_Count, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText93 = new wxStaticText(Panel1_Fireworks, ID_STATICTEXT94, _("Velocity of Particles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT94"));
	FlexGridSizer73->Add(StaticText93, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Velocity = new wxSlider(Panel1_Fireworks, ID_SLIDER_Fireworks_Velocity, 2, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Velocity"));
	FlexGridSizer73->Add(Slider_Fireworks_Velocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText95 = new wxStaticText(Panel1_Fireworks, ID_STATICTEXT96, _("Particle Fade"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT96"));
	FlexGridSizer73->Add(StaticText95, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Fade = new wxSlider(Panel1_Fireworks, ID_SLIDER_Fireworks_Fade, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Fade"));
	FlexGridSizer73->Add(Slider_Fireworks_Fade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel1_Fireworks->SetSizer(FlexGridSizer73);
	FlexGridSizer73->Fit(Panel1_Fireworks);
	FlexGridSizer73->SetSizeHints(Panel1_Fireworks);
	Panel1_Garlands = new wxPanel(Choicebook1, ID_PANEL14, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL14"));
	FlexGridSizer39 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer39->AddGrowableCol(1);
	StaticText32 = new wxStaticText(Panel1_Garlands, ID_STATICTEXT34, _("Garland Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT34"));
	FlexGridSizer39->Add(StaticText32, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Garlands_Type = new wxSlider(Panel1_Garlands, ID_SLIDER_Garlands_Type, 0, 0, 4, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Garlands_Type"));
	FlexGridSizer39->Add(Slider_Garlands_Type, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText33 = new wxStaticText(Panel1_Garlands, ID_STATICTEXT35, _("Spacing"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT35"));
	FlexGridSizer39->Add(StaticText33, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Garlands_Spacing = new wxSlider(Panel1_Garlands, ID_SLIDER_Garlands_Spacing, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Garlands_Spacing"));
	FlexGridSizer39->Add(Slider_Garlands_Spacing, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Panel1_Garlands->SetSizer(FlexGridSizer39);
	FlexGridSizer39->Fit(Panel1_Garlands);
	FlexGridSizer39->SetSizeHints(Panel1_Garlands);
	Panel1_Life = new wxPanel(Choicebook1, ID_PANEL16, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL16"));
	FlexGridSizer40 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer40->AddGrowableCol(1);
	StaticText35 = new wxStaticText(Panel1_Life, ID_STATICTEXT36, _("Cells to start"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT36"));
	FlexGridSizer40->Add(StaticText35, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Life_Count = new wxSlider(Panel1_Life, ID_SLIDER_Life_Count, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Life_Count"));
	FlexGridSizer40->Add(Slider_Life_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText37 = new wxStaticText(Panel1_Life, ID_STATICTEXT37, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT37"));
	FlexGridSizer40->Add(StaticText37, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Life_Seed = new wxSlider(Panel1_Life, ID_SLIDER_Life_Seed, 0, 0, 4, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Life_Seed"));
	FlexGridSizer40->Add(Slider_Life_Seed, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Panel1_Life->SetSizer(FlexGridSizer40);
	FlexGridSizer40->Fit(Panel1_Life);
	FlexGridSizer40->SetSizeHints(Panel1_Life);
	Panel1_Meteors = new wxPanel(Choicebook1, ID_PANEL18, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL18"));
	FlexGridSizer41 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer41->AddGrowableCol(1);
	StaticText39 = new wxStaticText(Panel1_Meteors, ID_STATICTEXT39, _("Colors"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT39"));
	FlexGridSizer41->Add(StaticText39, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Meteors_Type = new wxChoice(Panel1_Meteors, ID_CHOICE_Meteors_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Meteors_Type"));
	FlexGridSizer41->Add(Choice_Meteors_Type, 1, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText128 = new wxStaticText(Panel1_Meteors, ID_STATICTEXT129, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT129"));
	FlexGridSizer41->Add(StaticText128, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Meteors_Effect = new wxChoice(Panel1_Meteors, ID_CHOICE_Meteors_Effect, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Meteors_Effect"));
	FlexGridSizer41->Add(Choice_Meteors_Effect, 1, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText41 = new wxStaticText(Panel1_Meteors, ID_STATICTEXT41, _("Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT41"));
	FlexGridSizer41->Add(StaticText41, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Meteors_Count = new wxSlider(Panel1_Meteors, ID_SLIDER_Meteors_Count, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Meteors_Count"));
	FlexGridSizer41->Add(Slider_Meteors_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText43 = new wxStaticText(Panel1_Meteors, ID_STATICTEXT43, _("Trail Length"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT43"));
	FlexGridSizer41->Add(StaticText43, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Meteors_Length = new wxSlider(Panel1_Meteors, ID_SLIDER_Meteors_Length, 25, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Meteors_Length"));
	FlexGridSizer41->Add(Slider_Meteors_Length, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText130 = new wxStaticText(Panel1_Meteors, ID_STATICTEXT131, _("Swirl Intensity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT131"));
	FlexGridSizer41->Add(StaticText130, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Meteors_Swirl_Intensity = new wxSlider(Panel1_Meteors, ID_SLIDER_Meteors_Swirl_Intensity, 0, 0, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Meteors_Swirl_Intensity"));
	FlexGridSizer41->Add(Slider_Meteors_Swirl_Intensity, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Panel1_Meteors->SetSizer(FlexGridSizer41);
	FlexGridSizer41->Fit(Panel1_Meteors);
	FlexGridSizer41->SetSizeHints(Panel1_Meteors);
	Panel1_Piano = new wxPanel(Choicebook1, ID_PANEL39, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL39"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(1);
	StaticText1 = new wxStaticText(Panel1_Piano, ID_STATICTEXT1, _("Keyboard"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer5->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Piano_Keyboard = new wxSlider(Panel1_Piano, ID_SLIDER_Piano_Keyboard, 3, 1, 3, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Piano_Keyboard"));
	FlexGridSizer5->Add(Slider_Piano_Keyboard, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(Panel1_Piano, ID_STATICTEXT4, _("This effect \nis still under \ndevelopement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer5->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel1_Piano->SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(Panel1_Piano);
	FlexGridSizer5->SetSizeHints(Panel1_Piano);
	Panel1_Pictures = new wxPanel(Choicebook1, ID_PANEL20, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL20"));
	FlexGridSizer42 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer42->AddGrowableCol(0);
	Button_Pictures_Filename = new wxButton(Panel1_Pictures, ID_BUTTON_PICTURES_FILENAME, _("File name"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_PICTURES_FILENAME"));
	Button_Pictures_Filename->SetBackgroundColour(wxColour(224,224,224));
	FlexGridSizer42->Add(Button_Pictures_Filename, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Pictures_Filename = new wxTextCtrl(Panel1_Pictures, ID_TEXTCTRL_Pictures_Filename, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Pictures_Filename"));
	FlexGridSizer42->Add(TextCtrl_Pictures_Filename, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer31 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer31->AddGrowableCol(1);
	StaticText46 = new wxStaticText(Panel1_Pictures, ID_STATICTEXT46, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT46"));
	FlexGridSizer31->Add(StaticText46, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Pictures_Direction = new wxChoice(Panel1_Pictures, ID_CHOICE_Pictures_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Pictures_Direction"));
	FlexGridSizer31->Add(Choice_Pictures_Direction, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText96 = new wxStaticText(Panel1_Pictures, ID_STATICTEXT97, _("Anim Gif Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT97"));
	FlexGridSizer31->Add(StaticText96, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Pictures_GifSpeed = new wxSlider(Panel1_Pictures, ID_SLIDER_Pictures_GifSpeed, 20, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Pictures_GifSpeed"));
	FlexGridSizer31->Add(Slider_Pictures_GifSpeed, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer42->Add(FlexGridSizer31, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Panel1_Pictures->SetSizer(FlexGridSizer42);
	FlexGridSizer42->Fit(Panel1_Pictures);
	FlexGridSizer42->SetSizeHints(Panel1_Pictures);
	Panel1_Snowflakes = new wxPanel(Choicebook1, ID_PANEL22, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL22"));
	FlexGridSizer43 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer43->AddGrowableCol(1);
	StaticText79 = new wxStaticText(Panel1_Snowflakes, ID_STATICTEXT80, _("Max flakes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT80"));
	FlexGridSizer43->Add(StaticText79, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Snowflakes_Count = new wxSlider(Panel1_Snowflakes, ID_SLIDER_Snowflakes_Count, 5, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowflakes_Count"));
	FlexGridSizer43->Add(Slider_Snowflakes_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText80 = new wxStaticText(Panel1_Snowflakes, ID_STATICTEXT81, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT81"));
	FlexGridSizer43->Add(StaticText80, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Snowflakes_Type = new wxSlider(Panel1_Snowflakes, ID_SLIDER_Snowflakes_Type, 1, 0, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowflakes_Type"));
	FlexGridSizer43->Add(Slider_Snowflakes_Type, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Panel1_Snowflakes->SetSizer(FlexGridSizer43);
	FlexGridSizer43->Fit(Panel1_Snowflakes);
	FlexGridSizer43->SetSizeHints(Panel1_Snowflakes);
	Panel1_Snowstorm = new wxPanel(Choicebook1, ID_PANEL24, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL24"));
	FlexGridSizer44 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer44->AddGrowableCol(1);
	StaticText45 = new wxStaticText(Panel1_Snowstorm, ID_STATICTEXT45, _("Max flakes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT45"));
	FlexGridSizer44->Add(StaticText45, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Snowstorm_Count = new wxSlider(Panel1_Snowstorm, ID_SLIDER_Snowstorm_Count, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowstorm_Count"));
	FlexGridSizer44->Add(Slider_Snowstorm_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText51 = new wxStaticText(Panel1_Snowstorm, ID_STATICTEXT51, _("Trail Length"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT51"));
	FlexGridSizer44->Add(StaticText51, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Snowstorm_Length = new wxSlider(Panel1_Snowstorm, ID_SLIDER_Snowstorm_Length, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowstorm_Length"));
	FlexGridSizer44->Add(Slider_Snowstorm_Length, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Panel1_Snowstorm->SetSizer(FlexGridSizer44);
	FlexGridSizer44->Fit(Panel1_Snowstorm);
	FlexGridSizer44->SetSizeHints(Panel1_Snowstorm);
	Panel1_Spirals = new wxPanel(Choicebook1, ID_PANEL26, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL26"));
	FlexGridSizer45 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer45->AddGrowableCol(1);
	StaticText34 = new wxStaticText(Panel1_Spirals, ID_STATICTEXT38, _("Palette Rep"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT38"));
	FlexGridSizer45->Add(StaticText34, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Spirals_Count = new wxSlider(Panel1_Spirals, ID_SLIDER_Spirals_Count, 1, 1, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals_Count"));
	FlexGridSizer45->Add(Slider_Spirals_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText36 = new wxStaticText(Panel1_Spirals, ID_STATICTEXT40, _("Rotations"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT40"));
	FlexGridSizer45->Add(StaticText36, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Spirals_Rotation = new wxSlider(Panel1_Spirals, ID_SLIDER_Spirals_Rotation, 20, -500, 500, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals_Rotation"));
	FlexGridSizer45->Add(Slider_Spirals_Rotation, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText38 = new wxStaticText(Panel1_Spirals, ID_STATICTEXT42, _("Thickness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT42"));
	FlexGridSizer45->Add(StaticText38, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Spirals_Thickness = new wxSlider(Panel1_Spirals, ID_SLIDER_Spirals_Thickness, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals_Thickness"));
	FlexGridSizer45->Add(Slider_Spirals_Thickness, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText40 = new wxStaticText(Panel1_Spirals, ID_STATICTEXT44, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT44"));
	FlexGridSizer45->Add(StaticText40, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Spirals_Direction = new wxSlider(Panel1_Spirals, ID_SLIDER_Spirals_Direction, 1, -1, 1, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals_Direction"));
	FlexGridSizer45->Add(Slider_Spirals_Direction, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Spirals_Blend = new wxCheckBox(Panel1_Spirals, ID_CHECKBOX_Spirals_Blend, _("Blend"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals_Blend"));
	CheckBox_Spirals_Blend->SetValue(false);
	FlexGridSizer45->Add(CheckBox_Spirals_Blend, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_Spirals_3D = new wxCheckBox(Panel1_Spirals, ID_CHECKBOX_Spirals_3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals_3D"));
	CheckBox_Spirals_3D->SetValue(false);
	FlexGridSizer45->Add(CheckBox_Spirals_3D, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_Spirals_Grow = new wxCheckBox(Panel1_Spirals, ID_CHECKBOX_Spirals_Grow, _("Grow"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals_Grow"));
	CheckBox_Spirals_Grow->SetValue(false);
	FlexGridSizer45->Add(CheckBox_Spirals_Grow, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_Spirlas_Shrink = new wxCheckBox(Panel1_Spirals, ID_CHECKBOX_Spirals_Shrink, _("Shrink"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals_Shrink"));
	CheckBox_Spirlas_Shrink->SetValue(false);
	FlexGridSizer45->Add(CheckBox_Spirlas_Shrink, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Panel1_Spirals->SetSizer(FlexGridSizer45);
	FlexGridSizer45->Fit(Panel1_Spirals);
	FlexGridSizer45->SetSizeHints(Panel1_Spirals);
	Panel1_Spirograph = new wxPanel(Choicebook1, ID_PANEL35, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL35"));
	FlexGridSizer72 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer72->AddGrowableCol(1);
	StaticText87 = new wxStaticText(Panel1_Spirograph, ID_STATICTEXT88, _("R - Radius of outer circle"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT88"));
	FlexGridSizer72->Add(StaticText87, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Spirograph_R = new wxSlider(Panel1_Spirograph, ID_SLIDER_Spirograph_R, 20, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirograph_R"));
	FlexGridSizer72->Add(Slider_Spirograph_R, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText88 = new wxStaticText(Panel1_Spirograph, ID_STATICTEXT89, _("r - Radius of inner circle"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT89"));
	FlexGridSizer72->Add(StaticText88, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Spirograph_r = new wxSlider(Panel1_Spirograph, ID_SLIDER_Spirograph_r, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirograph_r"));
	FlexGridSizer72->Add(Slider_Spirograph_r, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText89 = new wxStaticText(Panel1_Spirograph, ID_STATICTEXT90, _("d - Distance"), wxDefaultPosition, wxSize(84,13), 0, _T("ID_STATICTEXT90"));
	FlexGridSizer72->Add(StaticText89, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Spirograph_d = new wxSlider(Panel1_Spirograph, ID_SLIDER_Spirograph_d, 30, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirograph_d"));
	FlexGridSizer72->Add(Slider_Spirograph_d, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText92 = new wxStaticText(Panel1_Spirograph, ID_STATICTEXT93, _("Should distance\nbe animated\?"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT93"));
	FlexGridSizer72->Add(StaticText92, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_Spirograph_Animate = new wxCheckBox(Panel1_Spirograph, ID_CHECKBOX_Spirograph_Animate, _("Yes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirograph_Animate"));
	CheckBox_Spirograph_Animate->SetValue(false);
	FlexGridSizer72->Add(CheckBox_Spirograph_Animate, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText90 = new wxStaticText(Panel1_Spirograph, ID_STATICTEXT91, _("Note: r should be <= R"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT91"));
	FlexGridSizer72->Add(StaticText90, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Panel1_Spirograph->SetSizer(FlexGridSizer72);
	FlexGridSizer72->Fit(Panel1_Spirograph);
	FlexGridSizer72->SetSizeHints(Panel1_Spirograph);
	Panel1_Text = new wxPanel(Choicebook1, ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL7"));
	FlexGridSizer46 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer46->AddGrowableCol(0);
	FlexGridSizer65 = new wxFlexGridSizer(2, 2, 0, 0);
	FlexGridSizer65->AddGrowableCol(1);
	StaticText53 = new wxStaticText(Panel1_Text, ID_STATICTEXT53, _("Line 1"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT53"));
	FlexGridSizer65->Add(StaticText53, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Text_Line1 = new wxTextCtrl(Panel1_Text, ID_TEXTCTRL_Text_Line1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Text_Line1"));
	TextCtrl_Text_Line1->SetMaxLength(256);
	FlexGridSizer65->Add(TextCtrl_Text_Line1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText54 = new wxStaticText(Panel1_Text, ID_STATICTEXT54, _("Line 2"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT54"));
	FlexGridSizer65->Add(StaticText54, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Text_Line2 = new wxTextCtrl(Panel1_Text, ID_TEXTCTRL_Text_Line2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Text_Line2"));
	TextCtrl_Text_Line2->SetMaxLength(256);
	FlexGridSizer65->Add(TextCtrl_Text_Line2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer46->Add(FlexGridSizer65, 1, wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Notebook_Text1 = new wxNotebook(Panel1_Text, ID_NOTEBOOK_Text1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_Text1"));
	Notebook_Text1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR));
	Panel_Text1 = new wxPanel(Notebook_Text1, ID_PANEL_Text1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Text1"));
	FlexGridSizer69 = new wxFlexGridSizer(5, 2, 0, 0);
	Button_Text_Font1 = new wxButton(Panel_Text1, ID_BUTTON_TEXT_FONT1, _("Font"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_TEXT_FONT1"));
	Button_Text_Font1->SetBackgroundColour(wxColour(224,224,224));
	FlexGridSizer69->Add(Button_Text_Font1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Text_Font1 = new wxTextCtrl(Panel_Text1, ID_TEXTCTRL_Text_Font1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Text_Font1"));
	FlexGridSizer69->Add(TextCtrl_Text_Font1, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText78 = new wxStaticText(Panel_Text1, ID_STATICTEXT79, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT79"));
	FlexGridSizer69->Add(StaticText78, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Text_Dir1 = new wxChoice(Panel_Text1, ID_CHOICE_Text_Dir1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Dir1"));
	FlexGridSizer69->Add(Choice_Text_Dir1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText107 = new wxStaticText(Panel_Text1, ID_STATICTEXT108, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT108"));
	FlexGridSizer69->Add(StaticText107, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Text_Effect1 = new wxChoice(Panel_Text1, ID_CHOICE_Text_Effect1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Effect1"));
	FlexGridSizer69->Add(Choice_Text_Effect1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText98 = new wxStaticText(Panel_Text1, ID_STATICTEXT99, _("Count down"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT99"));
	FlexGridSizer69->Add(StaticText98, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Text_Count1 = new wxChoice(Panel_Text1, ID_CHOICE_Text_Count1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Count1"));
	FlexGridSizer69->Add(Choice_Text_Count1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText100 = new wxStaticText(Panel_Text1, ID_STATICTEXT101, _("Position"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT101"));
	FlexGridSizer69->Add(StaticText100, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Text_Position1 = new wxSlider(Panel_Text1, ID_SLIDER_Text_Position1, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Text_Position1"));
	FlexGridSizer69->Add(Slider_Text_Position1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Panel_Text1->SetSizer(FlexGridSizer69);
	FlexGridSizer69->Fit(Panel_Text1);
	FlexGridSizer69->SetSizeHints(Panel_Text1);
	Panel_Text2 = new wxPanel(Notebook_Text1, ID_PANEL_Text2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Text2"));
	FlexGridSizer74 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Text_Font2 = new wxButton(Panel_Text2, ID_BUTTON_TEXT_FONT2, _("Font"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_TEXT_FONT2"));
	Button_Text_Font2->SetBackgroundColour(wxColour(224,224,224));
	FlexGridSizer74->Add(Button_Text_Font2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Text_Font2 = new wxTextCtrl(Panel_Text2, ID_TEXTCTRL_Text_Font2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Text_Font2"));
	FlexGridSizer74->Add(TextCtrl_Text_Font2, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText57 = new wxStaticText(Panel_Text2, ID_STATICTEXT57, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT57"));
	FlexGridSizer74->Add(StaticText57, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Text_Dir2 = new wxChoice(Panel_Text2, ID_CHOICE_Text_Dir2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Dir2"));
	FlexGridSizer74->Add(Choice_Text_Dir2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText108 = new wxStaticText(Panel_Text2, ID_STATICTEXT109, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT109"));
	FlexGridSizer74->Add(StaticText108, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Text_Effect2 = new wxChoice(Panel_Text2, ID_CHOICE_Text_Effect2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Effect2"));
	FlexGridSizer74->Add(Choice_Text_Effect2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText103 = new wxStaticText(Panel_Text2, ID_STATICTEXT104, _("Count down"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT104"));
	FlexGridSizer74->Add(StaticText103, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Text_Count2 = new wxChoice(Panel_Text2, ID_CHOICE_Text_Count2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Count2"));
	FlexGridSizer74->Add(Choice_Text_Count2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText102 = new wxStaticText(Panel_Text2, ID_STATICTEXT103, _("Position"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT103"));
	FlexGridSizer74->Add(StaticText102, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Text_Position2 = new wxSlider(Panel_Text2, ID_SLIDER_Text_Position2, 50, 0, 100, wxDefaultPosition, wxSize(118,24), 0, wxDefaultValidator, _T("ID_SLIDER_Text_Position2"));
	FlexGridSizer74->Add(Slider_Text_Position2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Panel_Text2->SetSizer(FlexGridSizer74);
	FlexGridSizer74->Fit(Panel_Text2);
	FlexGridSizer74->SetSizeHints(Panel_Text2);
	Notebook_Text1->AddPage(Panel_Text1, _("Line 1 Details"), false);
	Notebook_Text1->AddPage(Panel_Text2, _("Line 2 Details"), false);
	FlexGridSizer46->Add(Notebook_Text1, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 0);
	Panel1_Text->SetSizer(FlexGridSizer46);
	FlexGridSizer46->Fit(Panel1_Text);
	FlexGridSizer46->SetSizeHints(Panel1_Text);
	Panel1_Tree = new wxPanel(Choicebook1, ID_PANEL34, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL34"));
	FlexGridSizer71 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer71->AddGrowableCol(1);
	StaticText86 = new wxStaticText(Panel1_Tree, ID_STATICTEXT87, _("Number Branches"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT87"));
	FlexGridSizer71->Add(StaticText86, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Tree_Branches = new wxSlider(Panel1_Tree, ID_SLIDER_Tree_Branches, 3, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Tree_Branches"));
	FlexGridSizer71->Add(Slider_Tree_Branches, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Panel1_Tree->SetSizer(FlexGridSizer71);
	FlexGridSizer71->Fit(Panel1_Tree);
	FlexGridSizer71->SetSizeHints(Panel1_Tree);
	Panel1_Twinkle = new wxPanel(Choicebook1, ID_PANEL33, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL33"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	StaticText85 = new wxStaticText(Panel1_Twinkle, ID_STATICTEXT86, _("Number Lights"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT86"));
	FlexGridSizer4->Add(StaticText85, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Twinkle_Count = new wxSlider(Panel1_Twinkle, ID_SLIDER_Twinkle_Count, 3, 2, 100, wxDefaultPosition, wxSize(117,24), 0, wxDefaultValidator, _T("ID_SLIDER_Twinkle_Count"));
	FlexGridSizer4->Add(Slider_Twinkle_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText104 = new wxStaticText(Panel1_Twinkle, ID_STATICTEXT105, _("Twinkle Steps"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT105"));
	FlexGridSizer4->Add(StaticText104, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Twinkle_Steps = new wxSlider(Panel1_Twinkle, ID_SLIDER_Twinkle_Steps, 30, 2, 200, wxDefaultPosition, wxSize(119,24), 0, wxDefaultValidator, _T("ID_SLIDER_Twinkle_Steps"));
	FlexGridSizer4->Add(Slider_Twinkle_Steps, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Twinkle_Strobe = new wxCheckBox(Panel1_Twinkle, ID_CHECKBOX_Twinkle_Strobe, _("Strobe"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Twinkle_Strobe"));
	CheckBox_Twinkle_Strobe->SetValue(false);
	FlexGridSizer4->Add(CheckBox_Twinkle_Strobe, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel1_Twinkle->SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(Panel1_Twinkle);
	FlexGridSizer4->SetSizeHints(Panel1_Twinkle);
	Choicebook1->AddPage(Panel1_None, _("None"), false);
	Choicebook1->AddPage(Panel1_Bars, _("Bars"), false);
	Choicebook1->AddPage(Panel1_Butterfly, _("Butterfly"), false);
	Choicebook1->AddPage(Panel1_Circles, _("Circles"), false);
	Choicebook1->AddPage(Panel1_ColorWash, _("Color Wash"), false);
	Choicebook1->AddPage(Panel1_Curtain, _("Curtain"), false);
	Choicebook1->AddPage(Panel1_Fire, _("Fire"), false);
	Choicebook1->AddPage(Panel1_Fireworks, _("Fireworks"), false);
	Choicebook1->AddPage(Panel1_Garlands, _("Garlands"), false);
	Choicebook1->AddPage(Panel1_Life, _("Life"), false);
	Choicebook1->AddPage(Panel1_Meteors, _("Meteors"), false);
	Choicebook1->AddPage(Panel1_Piano, _("Piano"), false);
	Choicebook1->AddPage(Panel1_Pictures, _("Pictures"), false);
	Choicebook1->AddPage(Panel1_Snowflakes, _("Snowflakes"), false);
	Choicebook1->AddPage(Panel1_Snowstorm, _("Snowstorm"), false);
	Choicebook1->AddPage(Panel1_Spirals, _("Spirals"), false);
	Choicebook1->AddPage(Panel1_Spirograph, _("Spirograph"), false);
	Choicebook1->AddPage(Panel1_Text, _("Text"), false);
	Choicebook1->AddPage(Panel1_Tree, _("Tree"), false);
	Choicebook1->AddPage(Panel1_Twinkle, _("Twinkle"), false);
	FlexGridSizer1->Add(Choicebook1, 1, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer_Palette = new wxFlexGridSizer(0, 2, 0, 0);
	CheckBox_Palette1 = new wxCheckBox(this, ID_CHECKBOX_Palette1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette1"));
	CheckBox_Palette1->SetValue(true);
	FlexGridSizer_Palette->Add(CheckBox_Palette1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Palette1 = new wxButton(this, ID_BUTTON_Palette1, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette1"));
	Button_Palette1->SetMinSize(wxSize(30,20));
	FlexGridSizer_Palette->Add(Button_Palette1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	CheckBox_Palette2 = new wxCheckBox(this, ID_CHECKBOX_Palette2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette2"));
	CheckBox_Palette2->SetValue(true);
	FlexGridSizer_Palette->Add(CheckBox_Palette2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Palette2 = new wxButton(this, ID_BUTTON_Palette2, _("2"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette2"));
	Button_Palette2->SetMinSize(wxSize(30,20));
	FlexGridSizer_Palette->Add(Button_Palette2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	CheckBox_Palette3 = new wxCheckBox(this, ID_CHECKBOX_Palette3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette3"));
	CheckBox_Palette3->SetValue(false);
	FlexGridSizer_Palette->Add(CheckBox_Palette3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Palette3 = new wxButton(this, ID_BUTTON_Palette3, _("3"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette3"));
	Button_Palette3->SetMinSize(wxSize(30,20));
	FlexGridSizer_Palette->Add(Button_Palette3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	CheckBox_Palette4 = new wxCheckBox(this, ID_CHECKBOX_Palette4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette4"));
	CheckBox_Palette4->SetValue(false);
	FlexGridSizer_Palette->Add(CheckBox_Palette4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Palette4 = new wxButton(this, ID_BUTTON_Palette4, _("4"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette4"));
	Button_Palette4->SetMinSize(wxSize(30,20));
	FlexGridSizer_Palette->Add(Button_Palette4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	CheckBox_Palette5 = new wxCheckBox(this, ID_CHECKBOX_Palette5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette5"));
	CheckBox_Palette5->SetValue(false);
	FlexGridSizer_Palette->Add(CheckBox_Palette5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Palette5 = new wxButton(this, ID_BUTTON_Palette5, _("5"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette5"));
	Button_Palette5->SetMinSize(wxSize(30,20));
	FlexGridSizer_Palette->Add(Button_Palette5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	CheckBox_Palette6 = new wxCheckBox(this, ID_CHECKBOX_Palette6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette6"));
	CheckBox_Palette6->SetValue(false);
	FlexGridSizer_Palette->Add(CheckBox_Palette6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Palette6 = new wxButton(this, ID_BUTTON_Palette6, _("6"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette6"));
	Button_Palette6->SetMinSize(wxSize(30,20));
	FlexGridSizer_Palette->Add(Button_Palette6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	FlexGridSizer2->Add(FlexGridSizer_Palette, 1, wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
	CheckBox_FitToTime = new wxCheckBox(this, ID_CHECKBOX_FitToTime, _("Fit to time"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FitToTime"));
	CheckBox_FitToTime->SetValue(false);
	FlexGridSizer2->Add(CheckBox_FitToTime, 1, wxTOP|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 8);
	FlexGridSizer6->Add(FlexGridSizer2, 1, wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText59 = new wxStaticText(this, ID_STATICTEXT61, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT61"));
	FlexGridSizer3->Add(StaticText59, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Speed = new wxSlider(this, ID_SLIDER_Speed, 10, 1, 20, wxDefaultPosition, wxSize(30,150), wxSL_VERTICAL|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER_Speed"));
	FlexGridSizer3->Add(Slider_Speed, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Speed = new wxTextCtrl(this, ID_TEXTCTRL_Speed, _("10"), wxDefaultPosition, wxSize(30,20), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Speed"));
	FlexGridSizer3->Add(TextCtrl_Speed, 1, wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6->Add(FlexGridSizer3, 1, wxLEFT|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Fade In"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer6->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Fadein = new wxTextCtrl(this, ID_TEXTCTRL_Fadein, _("0.00"), wxDefaultPosition, wxSize(30,20), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fadein"));
	TextCtrl_Fadein->SetMaxLength(4);
	FlexGridSizer6->Add(TextCtrl_Fadein, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Fade Out"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer6->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Fadeout = new wxTextCtrl(this, ID_TEXTCTRL_Fadeout, _("0.00"), wxDefaultPosition, wxSize(30,20), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fadeout"));
	TextCtrl_Fadeout->SetMaxLength(4);
	FlexGridSizer6->Add(TextCtrl_Fadeout, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1->Add(FlexGridSizer6, 1, wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
	SetSizer(FlexGridSizer1);
	ColourDialog1 = new wxColourDialog(this);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_PICTURES_FILENAME,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_Pictures_FilenameClick);
	Connect(ID_BUTTON_TEXT_FONT1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_Text_Font1Click);
	Connect(ID_BUTTON_TEXT_FONT2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_Text_Font2Click);
	Connect(ID_CHOICEBOOK1,wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&EffectsPanel::OnChoicebook1PageChanged);
	Connect(ID_CHECKBOX_Palette1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnCheckBox_PaletteClick);
	Connect(ID_BUTTON_Palette1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_PaletteNumberClick);
	Connect(ID_CHECKBOX_Palette2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnCheckBox_PaletteClick);
	Connect(ID_BUTTON_Palette2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_PaletteNumberClick);
	Connect(ID_CHECKBOX_Palette3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnCheckBox_PaletteClick);
	Connect(ID_BUTTON_Palette3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_PaletteNumberClick);
	Connect(ID_CHECKBOX_Palette4,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnCheckBox_PaletteClick);
	Connect(ID_BUTTON_Palette4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_PaletteNumberClick);
	Connect(ID_CHECKBOX_Palette5,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnCheckBox_PaletteClick);
	Connect(ID_BUTTON_Palette5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_PaletteNumberClick);
	Connect(ID_CHECKBOX_Palette6,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnCheckBox_PaletteClick);
	Connect(ID_BUTTON_Palette6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_PaletteNumberClick);
	Connect(ID_SLIDER_Speed,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&EffectsPanel::OnSlider_SpeedCmdScroll);
	Connect(ID_SLIDER_Speed,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&EffectsPanel::OnSlider_SpeedCmdScroll);
	//*)

    this->SetName(name);
}

EffectsPanel::~EffectsPanel()
{
	//(*Destroy(EffectsPanel)
	//*)
}

// idx is 1-6
wxColour EffectsPanel::GetPaletteColor(int idx)
{
    switch (idx) {
    case 1: return Button_Palette1->GetBackgroundColour();
    case 2: return Button_Palette2->GetBackgroundColour();
    case 3: return Button_Palette3->GetBackgroundColour();
    case 4: return Button_Palette4->GetBackgroundColour();
    case 5: return Button_Palette5->GetBackgroundColour();
    case 6: return Button_Palette6->GetBackgroundColour();
    }
}

// idx is 1-6
void EffectsPanel::SetPaletteColor(int idx, const wxColour* c)
{
    switch (idx) {
    case 1: SetButtonColor(Button_Palette1,c); break;
    case 2: SetButtonColor(Button_Palette2,c); break;
    case 3: SetButtonColor(Button_Palette3,c); break;
    case 4: SetButtonColor(Button_Palette4,c); break;
    case 5: SetButtonColor(Button_Palette5,c); break;
    case 6: SetButtonColor(Button_Palette6,c); break;
    }
}

void EffectsPanel::SetButtonColor(wxButton* btn, const wxColour* c)
{
    btn->SetBackgroundColour(*c);
    int test=c->Red()*0.299 + c->Green()*0.587 + c->Blue()*0.114;
    btn->SetForegroundColour(test < 186 ? *wxWHITE : *wxBLACK);
    
#ifdef __WXOSX__
    //OSX does NOT allow active buttons to have a color other than the default.
    //We'll use an image of the appropriate color instead
    wxImage image(15, 15);
    image.SetRGB(wxRect(0, 0, 15, 15),
                 c->Red(), c->Green(), c->Blue());
    wxBitmap bmp(image);
    
    btn->SetBitmap(bmp);
    btn->SetLabelText("");
#endif
}

void EffectsPanel::SetDefaultPalette()
{
    SetButtonColor(Button_Palette1,wxRED);
    SetButtonColor(Button_Palette2,wxGREEN);
    SetButtonColor(Button_Palette3,wxBLUE);
    SetButtonColor(Button_Palette4,wxYELLOW);
    SetButtonColor(Button_Palette5,wxWHITE);
    SetButtonColor(Button_Palette6,wxBLACK);
}

int EffectsPanel::GetRandomSliderValue(wxSlider* slider)
{
    return rand() % (slider->GetMax()-slider->GetMin()) + slider->GetMin();
}

// returns ",E1_" or ",E2_"
wxString EffectsPanel::GetAttrPrefix()
{
    wxString WinName=this->GetName();
    return wxT(",E") + WinName.Right(1) + wxT("_");
}

// this is recursive
wxString EffectsPanel::GetEffectStringFromWindow(wxWindow *ParentWin)
{
    wxString s,ChildName,AttrName;
    int i;
    wxString prefix=GetAttrPrefix();
    wxWindowList &ChildList = ParentWin->GetChildren();
    for ( wxWindowList::Node *node = ChildList.GetFirst(); node; node = node->GetNext() )
    {
        wxWindow *ChildWin = (wxWindow *)node->GetData();
        ChildName=ChildWin->GetName();
        AttrName=prefix+ChildName.Mid(3)+wxT("=");
        if (ChildName.StartsWith(wxT("ID_SLIDER")))
        {
            wxSlider* ctrl=(wxSlider*)ChildWin;
            s+=AttrName+wxString::Format(wxT("%d"),ctrl->GetValue());
        }
        else if (ChildName.StartsWith(wxT("ID_TEXTCTRL")))
        {
            wxTextCtrl* ctrl=(wxTextCtrl*)ChildWin;
            s+=AttrName+ctrl->GetValue();
        }
        else if (ChildName.StartsWith(wxT("ID_CHOICE")))
        {
            wxChoice* ctrl=(wxChoice*)ChildWin;
            s+=AttrName+ctrl->GetStringSelection();
        }
        else if (ChildName.StartsWith(wxT("ID_CHECKBOX")))
        {
            wxCheckBox* ctrl=(wxCheckBox*)ChildWin;
            wxString v=(ctrl->IsChecked()) ? wxT("1") : wxT("0");
            s+=AttrName+v;
        }
        else if (ChildName.StartsWith(wxT("ID_NOTEBOOK")))
        {
            wxNotebook* ctrl=(wxNotebook*)ChildWin;
            for(i=0;i<ctrl->GetPageCount();i++) {
                s+=GetEffectStringFromWindow(ctrl->GetPage(i));
            }
        }
    }
    return s;
}

wxString EffectsPanel::GetEffectString()
{
    wxString s,ChildName,AttrName;
    wxString prefix=GetAttrPrefix();
    s=prefix+wxString::Format(wxT("SLIDER_Speed=%d"),Slider_Speed->GetValue());
    s+=prefix+wxT("TEXTCTRL_Fadein=")+TextCtrl_Fadein->GetValue();
    s+=prefix+wxT("TEXTCTRL_Fadeout=")+TextCtrl_Fadeout->GetValue();
    s+=prefix+wxString::Format(wxT("CHECKBOX_FitToTime=%d"),CheckBox_FitToTime->GetValue()?1:0);

    // get effect controls
    s+=GetEffectStringFromWindow(Choicebook1->GetCurrentPage());

    // get palette
    wxColour color;
    for (int i=1; i<=PALETTE_SIZE; i++) {
        color=GetPaletteColor(i);
        AttrName.Printf(wxT("BUTTON_Palette%d="),i);
        s+=prefix+AttrName+color.GetAsString(wxC2S_HTML_SYNTAX);
        AttrName.Printf(wxT("CHECKBOX_Palette%d"),i);
        wxCheckBox* ctrl=(wxCheckBox*)wxWindow::FindWindowByName(wxT("ID_")+AttrName,this);
        wxString v=(ctrl->IsChecked()) ? wxT("1") : wxT("0");
        s+=prefix+AttrName+wxT("=")+v;
    }
    return s;
}

// assumes effidx does not refer to Text effect
wxString EffectsPanel::GetRandomEffectString(int effidx)
{
    wxString s,ChildName,AttrName;
    wxString prefix=GetAttrPrefix();

    // get speed
    s=prefix+wxString::Format(wxT("SLIDER_Speed=%d"),GetRandomSliderValue(Slider_Speed));

    // get effect controls
    wxWindowList &ChildList = Choicebook1->GetPage(effidx)->GetChildren();
    for ( wxWindowList::Node *node = ChildList.GetFirst(); node; node = node->GetNext() )
    {
        wxWindow *ChildWin = (wxWindow *)node->GetData();
        ChildName=ChildWin->GetName();
        AttrName=prefix+ChildName.Mid(3)+wxT("=");
        if (ChildName.StartsWith(wxT("ID_SLIDER")))
        {
            wxSlider* ctrl=(wxSlider*)ChildWin;
            if(ChildName.Contains(wxT("Spirograph_r")))
            {
                // always set little radius, r, to its minimum value
                s+=AttrName+wxString::Format(wxT("%d"), 0 );
            }
            else
            {
                s+=AttrName+wxString::Format(wxT("%d"),GetRandomSliderValue(ctrl));
            }
        }
        else if (ChildName.StartsWith(wxT("ID_TEXTCTRL")))
        {
            wxTextCtrl* ctrl=(wxTextCtrl*)ChildWin;
            s+=AttrName+ctrl->GetValue();
        }
        else if (ChildName.StartsWith(wxT("ID_CHOICE")))
        {
            wxChoice* ctrl=(wxChoice*)ChildWin;
            s+=AttrName+ctrl->GetString(rand()%ctrl->GetCount());
        }
        else if (ChildName.StartsWith(wxT("ID_CHECKBOX")))
        {
            wxCheckBox* ctrl=(wxCheckBox*)ChildWin;
            if(ChildName.Contains(wxT("Spirograph_Animate")))
            {
                // always animate spirograph
                s+=AttrName+wxString::Format(wxT("%d"), 1 );
            }
            else
            {
                wxString v=(rand()%2) ? wxT("1") : wxT("0");
                s+=AttrName+v;
            }
        }
    }

    // get palette
    wxColour color;
    for (int i=1; i<=PALETTE_SIZE; i++) {
        color=GetPaletteColor(i);
        AttrName.Printf(wxT("BUTTON_Palette%d="),i);
        s+=prefix+AttrName+color.GetAsString(wxC2S_HTML_SYNTAX);
        wxString v=(rand() % 2) ? wxT("1") : wxT("0");
        AttrName.Printf(wxT("CHECKBOX_Palette%d="),i);
        s+=prefix+AttrName+v;
    }
    return s;
}

void EffectsPanel::OnButton_Pictures_FilenameClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Image", *CurrentDir, "", "", wxImage::GetImageExtWildcard(), wxFD_OPEN );
    if (!filename.IsEmpty()) TextCtrl_Pictures_Filename->SetValue(filename);
}

void EffectsPanel::UpdateFont(wxTextCtrl* FontCtrl)
{
    wxFont oldfont,newfont;
    oldfont.SetNativeFontInfoUserDesc(FontCtrl->GetValue());
    newfont=wxGetFontFromUser(this,oldfont);
    if (newfont.IsOk())
    {
        wxString FontDesc=newfont.GetNativeFontInfoUserDesc();
        FontDesc.Replace(wxT(" unknown-90"),wxT(""));
        FontCtrl->SetValue(FontDesc);
    }
}

void EffectsPanel::OnButton_Text_Font1Click(wxCommandEvent& event)
{
    UpdateFont(TextCtrl_Text_Font1);
}

void EffectsPanel::OnButton_Text_Font2Click(wxCommandEvent& event)
{
    UpdateFont(TextCtrl_Text_Font2);
}

void EffectsPanel::OnCheckBox_PaletteClick(wxCommandEvent& event)
{
    PaletteChanged=true;
}

void EffectsPanel::OnButton_PaletteNumberClick(wxCommandEvent& event)
{
    wxButton* w=(wxButton*)event.GetEventObject();
    if (ColourDialog1->ShowModal() == wxID_OK)
    {
        wxColourData retData = ColourDialog1->GetColourData();
        wxColour color = retData.GetColour();
        SetButtonColor(w, &color);
        PaletteChanged=true;
    }
}

void EffectsPanel::OnChoicebook1PageChanged(wxChoicebookEvent& event)
{
    EffectChanged=true;
}

void EffectsPanel::OnSlider_SpeedCmdScroll(wxScrollEvent& event)
{
    TextCtrl_Speed->SetValue(wxString::Format("%d",Slider_Speed->GetValue()));
}
