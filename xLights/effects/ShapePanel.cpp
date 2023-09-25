/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ShapePanel.h"
#include "EffectPanelUtils.h"
#include "ShapeEffect.h"
#include "RenderBuffer.h"
#include "../CharMapDialog.h"

//(*InternalHeaders(ShapePanel)
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/font.h>
#include <wx/fontpicker.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

#include <wx/fontenum.h>

class Emoji
{
    std::string _name;
    std::string _font;
    int _c;

    public:
    Emoji(std::string name, std::string font, int c)
    {
        _name = name;
        _font = font;
        _c = c;
    }
    virtual ~Emoji() {}
    std::string GetName() const { return _name; }
    std::string GetFont() const { return _font; }
    int GetChar() const { return _c; }
};

class MyFontEnumerator : public wxFontEnumerator
{
    std::list<std::string> _fonts;
public:
    bool Exists(std::string font)
    {
        return std::find(_fonts.begin(), _fonts.end(), font) != _fonts.end();
    }
    virtual bool OnFacename(const wxString& facename) wxOVERRIDE
    {
        _fonts.push_back(facename.ToStdString());
        return true;
    }
};

//(*IdInit(ShapePanel)
const long ShapePanel::ID_STATICTEXT_Shape_ObjectToDraw = wxNewId();
const long ShapePanel::ID_CHOICE_Shape_ObjectToDraw = wxNewId();
const long ShapePanel::ID_FONTPICKER_Shape_Font = wxNewId();
const long ShapePanel::ID_SPINCTRL_Shape_Char = wxNewId();
const long ShapePanel::ID_STATICTEXT2 = wxNewId();
const long ShapePanel::ID_CHOICE_Shape_SkinTone = wxNewId();
const long ShapePanel::ID_STATICTEXT1 = wxNewId();
const long ShapePanel::ID_FILEPICKERCTRL_SVG = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_Thickness = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_Thickness = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shape_Thickness = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_Thickness = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_Count = wxNewId();
const long ShapePanel::IDD_SLIDER_Shape_Count = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shape_Count = wxNewId();
const long ShapePanel::ID_TEXTCTRL_Shape_Count = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_StartSize = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_StartSize = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shape_StartSize = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_StartSize = wxNewId();
const long ShapePanel::ID_CHECKBOX_Shape_RandomInitial = wxNewId();
const long ShapePanel::ID_STATICTEXT3 = wxNewId();
const long ShapePanel::ID_SLIDER_Shapes_Velocity = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shapes_Velocity = wxNewId();
const long ShapePanel::ID_TEXTCTRL_Shapes_Velocity = wxNewId();
const long ShapePanel::ID_STATICTEXT4 = wxNewId();
const long ShapePanel::ID_SLIDER_Shapes_Direction = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shapes_Direction = wxNewId();
const long ShapePanel::ID_TEXTCTRL_Shapes_Direction = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_Lifetime = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_Lifetime = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shape_Lifetime = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_Lifetime = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_Growth = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_Growth = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shape_Growth = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_Growth = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_CentreX = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_CentreX = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shape_CentreX = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_CentreX = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_CentreY = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_CentreY = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shape_CentreY = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_CentreY = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_Points = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_Points = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_Points = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_Rotation = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_Rotation = wxNewId();
const long ShapePanel::ID_VALUECURVE_Shape_Rotation = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_Rotation = wxNewId();
const long ShapePanel::ID_CHECKBOX_Shape_RandomLocation = wxNewId();
const long ShapePanel::ID_CHECKBOX_Shapes_RandomMovement = wxNewId();
const long ShapePanel::ID_CHECKBOX_Shape_FadeAway = wxNewId();
const long ShapePanel::ID_CHECKBOX_Shape_HoldColour = wxNewId();
const long ShapePanel::ID_CHECKBOX_Shape_UseMusic = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_Sensitivity = wxNewId();
const long ShapePanel::ID_SLIDER_Shape_Sensitivity = wxNewId();
const long ShapePanel::IDD_TEXTCTRL_Shape_Sensitivity = wxNewId();
const long ShapePanel::ID_CHECKBOX_Shape_FireTiming = wxNewId();
const long ShapePanel::ID_STATICTEXT_Shape_FireTimingTrack = wxNewId();
const long ShapePanel::ID_CHOICE_Shape_FireTimingTrack = wxNewId();
const long ShapePanel::ID_STATICTEXT5 = wxNewId();
const long ShapePanel::ID_TEXTCTRL_Shape_FilterLabel = wxNewId();
const long ShapePanel::ID_CHECKBOX_Shape_FilterReg = wxNewId();
//*)

BEGIN_EVENT_TABLE(ShapePanel,wxPanel)
	//(*EventTable(ShapePanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_SETTIMINGTRACKS, ShapePanel::SetTimingTracks)
END_EVENT_TABLE()

ShapePanel::ShapePanel(wxWindow* parent) : xlEffectPanel(parent)
{
    //(*Initialize(ShapePanel)
    BulkEditTextCtrl* TextCtrl1;
    BulkEditTextCtrl* TextCtrl34;
    BulkEditTextCtrl* TextCtrl_Shape_Growth;
    BulkEditTextCtrl* TextCtrl_Shape_StartSize;
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer11;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer57;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer9;
    wxStaticText* StaticText14;
    wxStaticText* StaticText9;

    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
    FlexGridSizer57 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer57->AddGrowableCol(1);
    StaticText69 = new wxStaticText(this, ID_STATICTEXT_Shape_ObjectToDraw, _("Object to Draw"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_ObjectToDraw"));
    FlexGridSizer57->Add(StaticText69, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Shape_ObjectToDraw = new BulkEditChoice(this, ID_CHOICE_Shape_ObjectToDraw, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Shape_ObjectToDraw"));
    Choice_Shape_ObjectToDraw->SetSelection( Choice_Shape_ObjectToDraw->Append(_("Circle")) );
    Choice_Shape_ObjectToDraw->Append(_("Ellipse"));
    Choice_Shape_ObjectToDraw->Append(_("Triangle"));
    Choice_Shape_ObjectToDraw->Append(_("Square"));
    Choice_Shape_ObjectToDraw->Append(_("Pentagon"));
    Choice_Shape_ObjectToDraw->Append(_("Hexagon"));
    Choice_Shape_ObjectToDraw->Append(_("Octagon"));
    Choice_Shape_ObjectToDraw->Append(_("Star"));
    Choice_Shape_ObjectToDraw->Append(_("Heart"));
    Choice_Shape_ObjectToDraw->Append(_("Tree"));
    Choice_Shape_ObjectToDraw->Append(_("Snowflake"));
    Choice_Shape_ObjectToDraw->Append(_("Candy Cane"));
    Choice_Shape_ObjectToDraw->Append(_("Random"));
    Choice_Shape_ObjectToDraw->Append(_("Crucifix"));
    Choice_Shape_ObjectToDraw->Append(_("Present"));
    Choice_Shape_ObjectToDraw->Append(_("Emoji"));
    Choice_Shape_ObjectToDraw->Append(_("SVG"));
    FlexGridSizer57->Add(Choice_Shape_ObjectToDraw, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 2);
    StaticText9 = new wxStaticText(this, wxID_ANY, _("Character"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer57->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer8->AddGrowableCol(1);
    FlexGridSizer8->AddGrowableRow(0);
    wxFont PickerFont_1(10,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,_T("Segoe UI Emoji"),wxFONTENCODING_DEFAULT);
    FontPickerCtrl_Font = new BulkEditFontPicker(this, ID_FONTPICKER_Shape_Font, PickerFont_1, wxDefaultPosition, wxDefaultSize, wxFNTP_FONTDESC_AS_LABEL, wxDefaultValidator, _T("ID_FONTPICKER_Shape_Font"));
    wxFont FontPickerCtrl_FontFont(10,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,_T("Segoe UI Emoji"),wxFONTENCODING_DEFAULT);
    FontPickerCtrl_Font->SetFont(FontPickerCtrl_FontFont);
    FlexGridSizer8->Add(FontPickerCtrl_Font, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    SpinCtrl_CharCode = new BulkEditSpinCtrl(this, ID_SPINCTRL_Shape_Char, _T("127876"), wxDefaultPosition, wxSize(65,20), 0, 32, 917631, 127876, _T("ID_SPINCTRL_Shape_Char"));
    SpinCtrl_CharCode->SetValue(_T("127876"));
    FlexGridSizer8->Add(SpinCtrl_CharCode, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer57->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 0);
    EmojiDisplay = new ClickableStaticText(this, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE, _T("ID_STATICTEXT2"));
    FlexGridSizer57->Add(EmojiDisplay, 1, wxALL|wxEXPAND, 2);
    StaticText14 = new wxStaticText(this, wxID_ANY, _("Tone"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer57->Add(StaticText14, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SkinToneChoice = new wxChoice(this, ID_CHOICE_Shape_SkinTone, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Shape_SkinTone"));
    SkinToneChoice->SetSelection( SkinToneChoice->Append(_("Default")) );
    SkinToneChoice->Append(_("Light"));
    SkinToneChoice->Append(_("Medium Light"));
    SkinToneChoice->Append(_("Medium"));
    SkinToneChoice->Append(_("Medium Dark"));
    SkinToneChoice->Append(_("Dark"));
    FlexGridSizer57->Add(SkinToneChoice, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 5);
    StaticText10 = new wxStaticText(this, ID_STATICTEXT1, _("SVG File"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer57->Add(StaticText10, 1, wxALL|wxEXPAND, 5);
    FilePickerCtrl_SVG = new BulkEditFilePickerCtrl(this, ID_FILEPICKERCTRL_SVG, wxEmptyString, wxEmptyString, _T("*.svg"), wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL_SVG"));
    FlexGridSizer57->Add(FilePickerCtrl_SVG, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 5);
    StaticText72 = new wxStaticText(this, ID_STATICTEXT_Shape_Thickness, _("Thickness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_Thickness"));
    FlexGridSizer57->Add(StaticText72, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    Slider_Shape_Thickness = new BulkEditSlider(this, ID_SLIDER_Shape_Thickness, 3, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_Thickness"));
    FlexGridSizer1->Add(Slider_Shape_Thickness, 1, wxALL|wxEXPAND, 2);
    BitmapButton_Shape_ThicknessVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shape_Thickness, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Shape_Thickness"));
    FlexGridSizer1->Add(BitmapButton_Shape_ThicknessVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer57->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
    TextCtrl_Shape_Thickness = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_Thickness, _("3"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_Thickness"));
    TextCtrl_Shape_Thickness->SetMaxLength(3);
    FlexGridSizer57->Add(TextCtrl_Shape_Thickness, 1, wxALL|wxEXPAND, 2);
    StaticText176 = new wxStaticText(this, ID_STATICTEXT_Shape_Count, _("Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_Count"));
    FlexGridSizer57->Add(StaticText176, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    Slider_Shape_Count = new BulkEditSlider(this, IDD_SLIDER_Shape_Count, 5, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Shape_Count"));
    FlexGridSizer2->Add(Slider_Shape_Count, 1, wxALL|wxEXPAND, 2);
    BitmapButton_Shape_CountVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shape_Count, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Shape_Count"));
    FlexGridSizer2->Add(BitmapButton_Shape_CountVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer57->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
    TextCtrl34 = new BulkEditTextCtrl(this, ID_TEXTCTRL_Shape_Count, _("5"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Shape_Count"));
    TextCtrl34->SetMaxLength(3);
    FlexGridSizer57->Add(TextCtrl34, 1, wxALL|wxEXPAND, 2);
    StaticText6 = new wxStaticText(this, ID_STATICTEXT_Shape_StartSize, _("Start Size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_StartSize"));
    FlexGridSizer57->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer7->AddGrowableCol(0);
    Slider_Shape_StartSize = new BulkEditSlider(this, ID_SLIDER_Shape_StartSize, 5, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_StartSize"));
    FlexGridSizer7->Add(Slider_Shape_StartSize, 1, wxALL|wxEXPAND, 2);
    BitmapButton_Shape_StartSizeVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shape_StartSize, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Shape_StartSize"));
    FlexGridSizer7->Add(BitmapButton_Shape_StartSizeVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer57->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 0);
    TextCtrl_Shape_StartSize = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_StartSize, _("5"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_StartSize"));
    TextCtrl_Shape_StartSize->SetMaxLength(3);
    FlexGridSizer57->Add(TextCtrl_Shape_StartSize, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 5);
    CheckBox_Shape_RandomInitial = new BulkEditCheckBox(this, ID_CHECKBOX_Shape_RandomInitial, _("Random initial shape sizes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Shape_RandomInitial"));
    CheckBox_Shape_RandomInitial->SetValue(false);
    FlexGridSizer57->Add(CheckBox_Shape_RandomInitial, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 5);
    StaticText12 = new wxStaticText(this, ID_STATICTEXT3, _("Velocity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer57->Add(StaticText12, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer10 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer10->AddGrowableCol(0);
    Slider_Shapes_Velocity = new BulkEditSlider(this, ID_SLIDER_Shapes_Velocity, 0, 0, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shapes_Velocity"));
    FlexGridSizer10->Add(Slider_Shapes_Velocity, 1, wxALL|wxEXPAND, 2);
    BitmapButton_Shapes_Velocity = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shapes_Velocity, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Shapes_Velocity"));
    FlexGridSizer10->Add(BitmapButton_Shapes_Velocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer57->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 0);
    TextCtrl_Shapes_Velocity = new BulkEditTextCtrl(this, ID_TEXTCTRL_Shapes_Velocity, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Shapes_Velocity"));
    TextCtrl_Shapes_Velocity->SetMaxLength(3);
    FlexGridSizer57->Add(TextCtrl_Shapes_Velocity, 1, wxALL|wxEXPAND, 2);
    StaticText13 = new wxStaticText(this, ID_STATICTEXT4, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer57->Add(StaticText13, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer11 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer11->AddGrowableCol(0);
    Slider_Shapes_Direction = new BulkEditSlider(this, ID_SLIDER_Shapes_Direction, 90, 0, 359, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shapes_Direction"));
    FlexGridSizer11->Add(Slider_Shapes_Direction, 1, wxALL|wxEXPAND, 2);
    BitmapButton_Shapes_Direction = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shapes_Direction, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Shapes_Direction"));
    FlexGridSizer11->Add(BitmapButton_Shapes_Direction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer57->Add(FlexGridSizer11, 1, wxALL|wxEXPAND, 0);
    TextCtrl_Shapes_Direction = new BulkEditTextCtrl(this, ID_TEXTCTRL_Shapes_Direction, _("90"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Shapes_Direction"));
    TextCtrl_Shapes_Direction->SetMaxLength(3);
    FlexGridSizer57->Add(TextCtrl_Shapes_Direction, 1, wxALL|wxEXPAND, 2);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT_Shape_Lifetime, _("Lifetime"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_Lifetime"));
    FlexGridSizer57->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer3->AddGrowableCol(0);
    Slider_Shape_Lifetime = new BulkEditSlider(this, ID_SLIDER_Shape_Lifetime, 5, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_Lifetime"));
    FlexGridSizer3->Add(Slider_Shape_Lifetime, 1, wxALL|wxEXPAND, 2);
    BitmapButton_Shape_LifetimeVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shape_Lifetime, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Shape_Lifetime"));
    FlexGridSizer3->Add(BitmapButton_Shape_LifetimeVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer57->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
    TextCtrl1 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_Lifetime, _("5"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_Lifetime"));
    TextCtrl1->SetMaxLength(3);
    FlexGridSizer57->Add(TextCtrl1, 1, wxALL|wxEXPAND, 2);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT_Shape_Growth, _("Growth"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_Growth"));
    FlexGridSizer57->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer4->AddGrowableCol(0);
    Slider_Shape_Growth = new BulkEditSlider(this, ID_SLIDER_Shape_Growth, 10, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_Growth"));
    FlexGridSizer4->Add(Slider_Shape_Growth, 1, wxALL|wxEXPAND, 2);
    BitmapButton_Shape_GrowthVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shape_Growth, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Shape_Growth"));
    FlexGridSizer4->Add(BitmapButton_Shape_GrowthVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer57->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 0);
    TextCtrl_Shape_Growth = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_Growth, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_Growth"));
    TextCtrl_Shape_Growth->SetMaxLength(3);
    FlexGridSizer57->Add(TextCtrl_Shape_Growth, 1, wxALL|wxEXPAND, 2);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT_Shape_CentreX, _("X Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_CentreX"));
    FlexGridSizer57->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer5->AddGrowableCol(0);
    Slider_Shape_CentreX = new BulkEditSlider(this, ID_SLIDER_Shape_CentreX, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_CentreX"));
    FlexGridSizer5->Add(Slider_Shape_CentreX, 1, wxALL|wxEXPAND, 2);
    BitmapButton_Shape_CentreXVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shape_CentreX, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Shape_CentreX"));
    FlexGridSizer5->Add(BitmapButton_Shape_CentreXVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer57->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 0);
    TextCtrl_Shape_CentreX = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_CentreX, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_CentreX"));
    TextCtrl_Shape_CentreX->SetMaxLength(4);
    FlexGridSizer57->Add(TextCtrl_Shape_CentreX, 1, wxALL|wxEXPAND, 2);
    StaticText5 = new wxStaticText(this, ID_STATICTEXT_Shape_CentreY, _("Y Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_CentreY"));
    FlexGridSizer57->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer6->AddGrowableCol(0);
    Slider_Shape_CentreY = new BulkEditSlider(this, ID_SLIDER_Shape_CentreY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_CentreY"));
    FlexGridSizer6->Add(Slider_Shape_CentreY, 1, wxALL|wxEXPAND, 2);
    BitmapButton_Shape_CentreYVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shape_CentreY, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Shape_CentreY"));
    FlexGridSizer6->Add(BitmapButton_Shape_CentreYVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer57->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 0);
    TextCtrl_Shape_CentreY = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_CentreY, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_CentreY"));
    TextCtrl_Shape_CentreY->SetMaxLength(4);
    FlexGridSizer57->Add(TextCtrl_Shape_CentreY, 1, wxALL|wxEXPAND, 2);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT_Shape_Points, _("Points"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_Points"));
    FlexGridSizer57->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Shape_Points = new BulkEditSlider(this, ID_SLIDER_Shape_Points, 5, 2, 9, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_Points"));
    FlexGridSizer57->Add(Slider_Shape_Points, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Shape_Points = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_Points, _("5"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_Points"));
    TextCtrl_Shape_Points->SetMaxLength(1);
    FlexGridSizer57->Add(TextCtrl_Shape_Points, 1, wxALL|wxEXPAND, 2);
    StaticText11 = new wxStaticText(this, ID_STATICTEXT_Shape_Rotation, _("Rotation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_Rotation"));
    FlexGridSizer57->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer9->AddGrowableCol(0);
    Slider_Shape_Rotation = new BulkEditSlider(this, ID_SLIDER_Shape_Rotation, 0, 0, 360, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_Rotation"));
    FlexGridSizer9->Add(Slider_Shape_Rotation, 1, wxALL|wxEXPAND, 5);
    BitmapButton_Shape_RotationVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shape_Rotation, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Shape_Rotation"));
    FlexGridSizer9->Add(BitmapButton_Shape_RotationVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer57->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 0);
    TextCtrl_Shape_Rotation = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_Rotation, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_Rotation"));
    TextCtrl_Shape_Rotation->SetMaxLength(3);
    FlexGridSizer57->Add(TextCtrl_Shape_Rotation, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 2);
    CheckBox_Shape_RandomLocation = new BulkEditCheckBox(this, ID_CHECKBOX_Shape_RandomLocation, _("Random Location"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Shape_RandomLocation"));
    CheckBox_Shape_RandomLocation->SetValue(false);
    FlexGridSizer57->Add(CheckBox_Shape_RandomLocation, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 5);
    CheckBox_Shapes_RandomMovement = new BulkEditCheckBox(this, ID_CHECKBOX_Shapes_RandomMovement, _("Random movement"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Shapes_RandomMovement"));
    CheckBox_Shapes_RandomMovement->SetValue(false);
    FlexGridSizer57->Add(CheckBox_Shapes_RandomMovement, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 2);
    CheckBox_Shape_FadeAway = new BulkEditCheckBox(this, ID_CHECKBOX_Shape_FadeAway, _("Fade Away"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Shape_FadeAway"));
    CheckBox_Shape_FadeAway->SetValue(false);
    FlexGridSizer57->Add(CheckBox_Shape_FadeAway, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 2);
    CheckBox_Shape_HoldColour = new BulkEditCheckBox(this, ID_CHECKBOX_Shape_HoldColour, _("Hold Color"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Shape_HoldColour"));
    CheckBox_Shape_HoldColour->SetValue(true);
    FlexGridSizer57->Add(CheckBox_Shape_HoldColour, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 2);
    CheckBox_Shape_UseMusic = new BulkEditCheckBox(this, ID_CHECKBOX_Shape_UseMusic, _("Fire with music"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Shape_UseMusic"));
    CheckBox_Shape_UseMusic->SetValue(false);
    FlexGridSizer57->Add(CheckBox_Shape_UseMusic, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 2);
    StaticText7 = new wxStaticText(this, ID_STATICTEXT_Shape_Sensitivity, _("Trigger level"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_Sensitivity"));
    FlexGridSizer57->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Shape_Sensitivity = new BulkEditSlider(this, ID_SLIDER_Shape_Sensitivity, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shape_Sensitivity"));
    FlexGridSizer57->Add(Slider_Shape_Sensitivity, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Shape_Sensitivity = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shape_Sensitivity, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shape_Sensitivity"));
    TextCtrl_Shape_Sensitivity->SetMaxLength(3);
    FlexGridSizer57->Add(TextCtrl_Shape_Sensitivity, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 2);
    CheckBox_Shape_FireTiming = new BulkEditCheckBox(this, ID_CHECKBOX_Shape_FireTiming, _("Fire with timing track"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Shape_FireTiming"));
    CheckBox_Shape_FireTiming->SetValue(false);
    FlexGridSizer57->Add(CheckBox_Shape_FireTiming, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 2);
    StaticText8 = new wxStaticText(this, ID_STATICTEXT_Shape_FireTimingTrack, _("Timing Track"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shape_FireTimingTrack"));
    FlexGridSizer57->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Shape_TimingTrack = new BulkEditChoice(this, ID_CHOICE_Shape_FireTimingTrack, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Shape_FireTimingTrack"));
    FlexGridSizer57->Add(Choice_Shape_TimingTrack, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer57->Add(-1,-1,1, wxALL|wxEXPAND, 2);
    StaticText15 = new wxStaticText(this, ID_STATICTEXT5, _("Filter Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer57->Add(StaticText15, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Shape_FilterLabel = new BulkEditTextCtrl(this, ID_TEXTCTRL_Shape_FilterLabel, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Shape_FilterLabel"));
    FlexGridSizer57->Add(TextCtrl_Shape_FilterLabel, 1, wxALL|wxEXPAND, 2);
    CheckBox_FilterLabelReg = new BulkEditCheckBox(this, ID_CHECKBOX_Shape_FilterReg, _("Reg"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Shape_FilterReg"));
    CheckBox_FilterLabelReg->SetValue(false);
    FlexGridSizer57->Add(CheckBox_FilterLabelReg, 1, wxALL|wxEXPAND, 2);
    SetSizer(FlexGridSizer57);
    FlexGridSizer57->Fit(this);
    FlexGridSizer57->SetSizeHints(this);

    Connect(ID_CHOICE_Shape_ObjectToDraw,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ShapePanel::OnChoice_Shape_ObjectToDrawSelect);
    Connect(ID_FONTPICKER_Shape_Font,wxEVT_COMMAND_FONTPICKER_CHANGED,(wxObjectEventFunction)&ShapePanel::OnFontPickerCtrl_FontFontChanged);
    Connect(ID_SPINCTRL_Shape_Char,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ShapePanel::OnSpinCtrl_CharCodeChange);
    Connect(ID_CHOICE_Shape_SkinTone,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ShapePanel::OnSkinToneChoiceSelect);
    Connect(ID_VALUECURVE_Shape_Thickness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Shape_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Shape_StartSize,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Shapes_Velocity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Shapes_Direction,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Shape_Lifetime,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Shape_Growth,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Shape_CentreX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Shape_CentreY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Shape_Rotation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShapePanel::OnVCButtonClick);
    Connect(ID_CHECKBOX_Shape_RandomLocation,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ShapePanel::OnCheckBox_Shape_RandomLocationClick);
    Connect(ID_CHECKBOX_Shapes_RandomMovement,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ShapePanel::OnCheckBox_Shapes_RandomMovementClick);
    Connect(ID_CHECKBOX_Shape_UseMusic,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ShapePanel::OnCheckBox_Shape_UseMusicClick);
    Connect(ID_CHECKBOX_Shape_FireTiming,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ShapePanel::OnCheckBox_Shape_FireTimingClick);
    Connect(ID_CHOICE_Shape_FireTimingTrack,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ShapePanel::OnChoice_Shape_TimingTrackSelect);
    //*)
    SetName("ID_PANEL_SHAPE");

    wxFont font(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, NATIVE_EMOJI_FONT, wxFONTENCODING_DEFAULT);
    FontPickerCtrl_Font->SetSelectedFont(font);

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&ShapePanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&ShapePanel::OnValidateWindow, 0, this);

    BitmapButton_Shape_ThicknessVC->GetValue()->SetLimits(SHAPE_THICKNESS_MIN, SHAPE_THICKNESS_MAX);
    BitmapButton_Shape_GrowthVC->GetValue()->SetLimits(SHAPE_GROWTH_MIN, SHAPE_GROWTH_MAX);
    BitmapButton_Shape_CentreYVC->GetValue()->SetLimits(SHAPE_CENTREY_MIN, SHAPE_CENTREY_MAX);
    BitmapButton_Shape_CentreXVC->GetValue()->SetLimits(SHAPE_CENTREX_MIN, SHAPE_CENTREX_MAX);
    BitmapButton_Shape_LifetimeVC->GetValue()->SetLimits(SHAPE_LIFETIME_MIN, SHAPE_LIFETIME_MAX);
    BitmapButton_Shape_CountVC->GetValue()->SetLimits(SHAPE_COUNT_MIN, SHAPE_COUNT_MAX);
    BitmapButton_Shape_StartSizeVC->GetValue()->SetLimits(SHAPE_STARTSIZE_MIN, SHAPE_STARTSIZE_MAX);
    BitmapButton_Shape_RotationVC->GetValue()->SetLimits(SHAPE_ROTATION_MIN, SHAPE_ROTATION_MAX);
    BitmapButton_Shapes_Velocity->GetValue()->SetLimits(SHAPE_VELOCITY_MIN, SHAPE_VELOCITY_MAX);
    BitmapButton_Shapes_Direction->GetValue()->SetLimits(SHAPE_DIRECTION_MIN, SHAPE_DIRECTION_MAX);

    wxFont f = FontPickerCtrl_Font->GetSelectedFont();
    f.SetPixelSize(wxSize(20, 20));

    EmojiDisplay->SetFont(f);
    EmojiDisplay->SetLabel(wxString(wxUniChar(SpinCtrl_CharCode->GetValue())));

    // get all the fonts
    MyFontEnumerator fontEnumerator;
    fontEnumerator.EnumerateFacenames();

    if (fontEnumerator.Exists("Webdings")) {
        _emojis.push_back(new Emoji("Spider", "Webdings", 33));
        _emojis.push_back(new Emoji("Web", "Webdings", 34));
        _emojis.push_back(new Emoji("Explosion", "Webdings", 42));
        _emojis.push_back(new Emoji("Cause Ribbon", "Webdings", 45));
        _emojis.push_back(new Emoji("Heart", "Webdings", 89));
        _emojis.push_back(new Emoji("Roses", "Webdings", 90));
        _emojis.push_back(new Emoji("Badge", "Webdings", 100));
        _emojis.push_back(new Emoji("Present 1", "Webdings", 101));
        _emojis.push_back(new Emoji("Fire Truck", "Webdings", 102));
        _emojis.push_back(new Emoji("Police Car", "Webdings", 112));
        _emojis.push_back(new Emoji("Musical Note", "Webdings", 175));
    }

    if (fontEnumerator.Exists("Wingdings")) {
        _emojis.push_back(new Emoji("Bell", "Wingdings", 37));
        _emojis.push_back(new Emoji("Candle", "Wingdings", 39));
        _emojis.push_back(new Emoji("Smiley", "Wingdings", 74));
        _emojis.push_back(new Emoji("Snowflake 1", "Wingdings", 84));
        _emojis.push_back(new Emoji("Cross 1", "Wingdings", 85));
        _emojis.push_back(new Emoji("Cross 2", "Wingdings", 86));
        _emojis.push_back(new Emoji("Cross 3", "Wingdings", 87));
        _emojis.push_back(new Emoji("Star of David", "Wingdings", 89));
        _emojis.push_back(new Emoji("Star", "Wingdings", 171));
    }

    if (fontEnumerator.Exists(NATIVE_EMOJI_FONT)) {
        _emojis.push_back(new Emoji("Snowman 1", NATIVE_EMOJI_FONT, 9924));
        _emojis.push_back(new Emoji("Snowman 2", NATIVE_EMOJI_FONT, 9927));
        _emojis.push_back(new Emoji("Snowflake 2", NATIVE_EMOJI_FONT, 10052));
        _emojis.push_back(new Emoji("Snowflake 3", NATIVE_EMOJI_FONT, 10053));
        _emojis.push_back(new Emoji("Snowflake 4", NATIVE_EMOJI_FONT, 10054));

        _emojis.push_back(new Emoji("Christmas Tree", NATIVE_EMOJI_FONT, 0x1F384));
        _emojis.push_back(new Emoji("Gift", NATIVE_EMOJI_FONT, 0x1F381));
        _emojis.push_back(new Emoji("Mr. Claus", NATIVE_EMOJI_FONT, 0x1F385));
        _emojis.push_back(new Emoji("Mrs. Claus", NATIVE_EMOJI_FONT, 0x1F936));
    }

    if (fontEnumerator.Exists("XmasDings")) {
        _emojis.push_back(new Emoji("Presents ->", "XmasDings", 49));
        _emojis.push_back(new Emoji("Baubles ->", "XmasDings", 66));
        _emojis.push_back(new Emoji("Bells ->", "XmasDings", 80));
        _emojis.push_back(new Emoji("Angels ->", "XmasDings", 85));
        _emojis.push_back(new Emoji("Stockings ->", "XmasDings", 87));
        _emojis.push_back(new Emoji("Trees ->", "XmasDings", 97));
        _emojis.push_back(new Emoji("Wreaths ->", "XmasDings", 104));
        _emojis.push_back(new Emoji("Hollies ->", "XmasDings", 107));
        _emojis.push_back(new Emoji("Candles ->", "XmasDings", 109));
        _emojis.push_back(new Emoji("Snowmen ->", "XmasDings", 113));
        _emojis.push_back(new Emoji("Santas ->", "XmasDings", 116));
        _emojis.push_back(new Emoji("Candy Canes ->", "XmasDings", 120));
    }

    if (fontEnumerator.Exists("broken")) {
        // This should never happen ... this font is not meant to exist
        wxASSERT(false);
    }

    Connect(ID_STATICTEXT2, wxEVT_COMMAND_LEFT_DCLICK, (wxObjectEventFunction)& ShapePanel::OnShowCharMap);
    Connect(ID_STATICTEXT2, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&ShapePanel::EmojiMenu);

    ValidateWindow();
}


void ShapePanel::OnShowCharMap(wxCommandEvent& event)
{
    CharMapDialog dlg(this, FontPickerCtrl_Font->GetSelectedFont(), SpinCtrl_CharCode->GetValue());

    dlg.ShowModal();

    SpinCtrl_CharCode->SetValue(dlg.GetCharCode());
    wxSpinEvent sce;
    HandleSpinChange(sce);

    ValidateWindow();
}

ShapePanel::~ShapePanel()
{
	//(*Destroy(ShapePanel)
	//*)

    while (_emojis.size() > 0) {
        delete _emojis.front();
        _emojis.pop_front();
    }
}

void ShapePanel::ValidateWindow()
{
    if (Choice_Shape_ObjectToDraw->GetStringSelection() == "Star" ||
		Choice_Shape_ObjectToDraw->GetStringSelection() == "Ellipse") {
        Slider_Shape_Points->Enable();
        TextCtrl_Shape_Points->Enable();
    } else {
        Slider_Shape_Points->Disable();
        TextCtrl_Shape_Points->Disable();
    }
	if (Choice_Shape_ObjectToDraw->GetStringSelection() == "Ellipse") {
		StaticText1->SetLabel("Ratio");
    } else {
		StaticText1->SetLabel("Points");
	}

    if (Choice_Shape_ObjectToDraw->GetStringSelection() == "SVG") {
        FilePickerCtrl_SVG->Enable();
    } else {
        FilePickerCtrl_SVG->Enable(false);
        FilePickerCtrl_SVG->SetFileName(wxFileName(""));
    }

    if (Choice_Shape_ObjectToDraw->GetStringSelection() == "Emoji") {
        FontPickerCtrl_Font->Enable();
        SpinCtrl_CharCode->Enable();
        SkinToneChoice->Enable();
        wxFont f = FontPickerCtrl_Font->GetSelectedFont();

        wxString face = f.GetFaceName();
        if (face == WIN_NATIVE_EMOJI_FONT || face == OSX_NATIVE_EMOJI_FONT || face == LINUX_NATIVE_EMOJI_FONT) {
            f.SetFaceName(NATIVE_EMOJI_FONT);
        }
        f.SetPixelSize(wxSize(20, 20));

        EmojiDisplay->SetFont(f);

        wxString label = wxString(wxUniChar(SpinCtrl_CharCode->GetValue()));
        if (SkinToneChoice->GetSelection() > 0) {
            int i = SkinToneChoice->GetSelection() + 0x1F3FA;
            label.Append(wxString(wxUniChar(i)));
        }
        EmojiDisplay->SetLabel(label);
    } else {
        EmojiDisplay->SetLabel(" ");
        SkinToneChoice->SetSelection(0);
        FontPickerCtrl_Font->Enable(false);
        SpinCtrl_CharCode->Enable(false);
        SkinToneChoice->Enable(false);
    }

    if (Choice_Shape_ObjectToDraw->GetStringSelection() == "Snowflake" ||
        Choice_Shape_ObjectToDraw->GetStringSelection() == "Emoji")
    {
        Slider_Shape_Thickness->Disable();
        TextCtrl_Shape_Thickness->Disable();
    } else {
        Slider_Shape_Thickness->Enable();
        TextCtrl_Shape_Thickness->Enable();
    }

    if (Choice_Shape_ObjectToDraw->GetStringSelection() == "SVG") {
        CheckBox_Shape_HoldColour->Enable(false);
    } else {
        CheckBox_Shape_HoldColour->Enable();
    }

    if (Choice_Shape_ObjectToDraw->GetStringSelection() == "Emoji" ||
		Choice_Shape_ObjectToDraw->GetStringSelection() == "Candy Cane" ||
        Choice_Shape_ObjectToDraw->GetStringSelection() == "SVG" ||
        Choice_Shape_ObjectToDraw->GetStringSelection() == "Random" ||
        Choice_Shape_ObjectToDraw->GetStringSelection() == "Circle" )
    {
		Slider_Shape_Rotation->Disable();
		TextCtrl_Shape_Rotation->Disable();
		BitmapButton_Shape_RotationVC->Disable();
    } else {
		Slider_Shape_Rotation->Enable();
		TextCtrl_Shape_Rotation->Enable();
		BitmapButton_Shape_RotationVC->Enable();
    }

    if (CheckBox_Shapes_RandomMovement->IsChecked()) {
        Slider_Shapes_Direction->Disable();
        Slider_Shapes_Velocity->Disable();
        TextCtrl_Shapes_Direction->Disable();
        TextCtrl_Shapes_Velocity->Disable();
        BitmapButton_Shapes_Direction->Disable();
        BitmapButton_Shapes_Velocity->Disable();
    } else {
        Slider_Shapes_Direction->Enable();
        Slider_Shapes_Velocity->Enable();
        TextCtrl_Shapes_Direction->Enable();
        TextCtrl_Shapes_Velocity->Enable();
        BitmapButton_Shapes_Direction->Enable();
        BitmapButton_Shapes_Velocity->Enable();
    }

    if (CheckBox_Shape_RandomLocation->IsChecked()) {
        Slider_Shape_CentreX->Disable();
        Slider_Shape_CentreY->Disable();
        TextCtrl_Shape_CentreX->Disable();
        TextCtrl_Shape_CentreY->Disable();
        BitmapButton_Shape_CentreXVC->Disable();
        BitmapButton_Shape_CentreYVC->Disable();
    } else {
        Slider_Shape_CentreX->Enable();
        Slider_Shape_CentreY->Enable();
        TextCtrl_Shape_CentreX->Enable();
        TextCtrl_Shape_CentreY->Enable();
        BitmapButton_Shape_CentreXVC->Enable();
        BitmapButton_Shape_CentreYVC->Enable();
    }

    if (CheckBox_Shape_UseMusic->IsChecked()) {
        Slider_Shape_Sensitivity->Enable();
        TextCtrl_Shape_Sensitivity->Enable();
    } else {
        Slider_Shape_Sensitivity->Disable();
        TextCtrl_Shape_Sensitivity->Disable();
    }

    if (CheckBox_Shape_FireTiming->IsChecked()) {
        Choice_Shape_TimingTrack->Enable();
        TextCtrl_Shape_FilterLabel->Enable();
        CheckBox_FilterLabelReg->Enable();
    } else {
        Choice_Shape_TimingTrack->Disable();
        TextCtrl_Shape_FilterLabel->Disable();
        CheckBox_FilterLabelReg->Disable();
    }
}

void ShapePanel::OnChoice_Shape_ObjectToDrawSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void ShapePanel::OnCheckBox_Shape_RandomLocationClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void ShapePanel::OnCheckBox_Shape_UseMusicClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void ShapePanel::OnCheckBox_Shape_FireTimingClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void ShapePanel::OnChoice_Shape_TimingTrackSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void ShapePanel::SetTimingTracks(wxCommandEvent& event)
{
    auto timingtracks = wxSplit(event.GetString(), '|');

    wxString selection = Choice_Shape_TimingTrack->GetStringSelection();

    // check if anything has been removed ... if it has clear the list and we will have to rebuild it as you cant delete items from a combo box
    bool removed = false;
    for (size_t i = 0; i < Choice_Shape_TimingTrack->GetCount(); i++) {
        bool found = false;
        for (auto it = timingtracks.begin(); it != timingtracks.end(); ++it) {
            if (*it == Choice_Shape_TimingTrack->GetString(i)) {
                found = true;
                break;
            }
        }
        if (!found) {
            Choice_Shape_TimingTrack->Clear();
            removed = true;
            break;
        }
    }

    // add any new timing tracks
    for (const auto& it : timingtracks) {
        bool found = false;
        for (size_t i = 0; i < Choice_Shape_TimingTrack->GetCount(); i++) {
            if (it == Choice_Shape_TimingTrack->GetString(i)) {
                found = true;
                break;
            }
        }
        if (!found) {
            Choice_Shape_TimingTrack->Append(it);
        }
    }

    if (removed && Choice_Shape_TimingTrack->GetCount() > 0) {
        // go through the list and see if our selected item is there
        bool found = false;
        for (size_t i = 0; i < Choice_Shape_TimingTrack->GetCount(); i++) {
            if (selection == Choice_Shape_TimingTrack->GetString(i)) {
                found = true;
                Choice_Shape_TimingTrack->SetSelection(i);
                break;
            }
        }
        if (!found) {
            Choice_Shape_TimingTrack->SetSelection(0);
        }
    }
    ValidateWindow();
}

void ShapePanel::EmojiMenu(wxContextMenuEvent& event)
{
    wxMenu mnuEmoji;
    for (const auto& it : _emojis) {
        mnuEmoji.Append(wxNewId(), it->GetName());
    }
    mnuEmoji.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ShapePanel::OnPopupEmoji, nullptr, this);
    PopupMenu(&mnuEmoji);
}

void ShapePanel::OnPopupEmoji(wxCommandEvent& event)
{
    wxMenu* eo = (wxMenu*)event.GetEventObject();
    auto item = eo->GetLabelText(event.GetId());

    for (const auto& it : _emojis) {
        if (it->GetName() == item) {
            wxFont f;
            f.Create(10, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, it->GetFont(), wxFONTENCODING_DEFAULT);
            if (f.IsOk()) {
                FontPickerCtrl_Font->SetSelectedFont(f);
                wxFontPickerEvent fpe;
                HandleFontChange(fpe);
                SpinCtrl_CharCode->SetValue(it->GetChar());
                wxSpinEvent sce;
                HandleSpinChange(sce);
                ValidateWindow();
                break;
            }
        }
    }
}

void ShapePanel::OnFontPickerCtrl_FontFontChanged(wxFontPickerEvent& event)
{
    //wxFont f = event.GetFont();
    //f.SetPixelSize(wxSize(20, 20));
    //EmojiDisplay->SetFont(f);
    ValidateWindow();
}

void ShapePanel::OnSpinCtrl_CharCodeChange(wxSpinEvent& event)
{
    //EmojiDisplay->SetLabel(wxString(wxUniChar(event.GetValue())));
    ValidateWindow();
}

void ShapePanel::OnCheckBox_Shapes_RandomMovementClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void ShapePanel::OnSkinToneChoiceSelect(wxCommandEvent& event)
{
    FireChangeEvent();
    ValidateWindow();
}
