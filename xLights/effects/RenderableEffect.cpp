/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"
#include "../sequencer/Effect.h"
#include "EffectManager.h"
#include "assist/xlGridCanvasEmpty.h"
#include "../UtilFunctions.h"
#include "../sequencer/SequenceElements.h"

#include <wx/fontpicker.h>
#include <wx/filepicker.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>

#include <sstream>
#include "../UtilFunctions.h"
#include "../ValueCurveButton.h"
#include "PixelBuffer.h"
#include "FanEffect.h"
#include "SpiralsEffect.h"
#include "PinwheelEffect.h"

#include "../xLightsApp.h"
#include "../xLightsMain.h"

RenderableEffect::RenderableEffect(int i, std::string n,
                                   const char **data16,
                                   const char **data24,
                                   const char **data32,
                                   const char **data48,
                                   const char **data64)
    : id(i), name(n), tooltip(n), panel(nullptr), mSequenceElements(nullptr)
{
    initBitmaps(data16, data24, data32, data48, data64);
}

RenderableEffect::~RenderableEffect()
{
    //dtor
}

const wxBitmap &RenderableEffect::GetEffectIcon(int size, bool exact) const {
    if (exact || GetSystemContentScaleFactor() < 1.5) {
        if (size <= 16) {
            return icon16e;
        } else if (size <= 24) {
            return icon24e;
        } else if (size <= 32) {
            return icon32e;
        } else if (size <= 48) {
            return icon48e;
        }
    }
    if (size <= 16) {
        return icon16;
    } else if (size <= 24) {
        return icon24;
    } else if (size <= 32) {
        return icon32;
    } else if (size <= 48) {
        return icon48;
    } else {
        return icon64;
    }
}


wxPanel *RenderableEffect::GetPanel(wxWindow *parent) {
    if (panel == nullptr) {
        panel = CreatePanel(parent);
    }
    return panel;
}

AssistPanel *RenderableEffect::GetAssistPanel(wxWindow *parent, xLightsFrame* xl_frame) {
    AssistPanel *assist_panel = new AssistPanel(parent);
    xlGridCanvas* grid = new xlGridCanvasEmpty(assist_panel->GetCanvasParent(), wxNewId(), wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE, _T("EmptyGrid"));
    assist_panel->SetGridCanvas(grid);
    return assist_panel;
}

int RenderableEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                           DrawGLUtils::xlAccumulator &background, xlColor* colorMask, bool ramps) {
    if (e->HasBackgroundDisplayList()) {
        DrawGLUtils::DrawDisplayList(x1, y1, x2-x1, y2-y1, e->GetBackgroundDisplayList(), background);
        return e->GetBackgroundDisplayList().iconSize;
    }
    return 1;
}


void AdjustAndSetBitmap(int size, wxImage &image, wxImage &dbl, wxBitmap&bitmap) {
    if (dbl.GetHeight() == (2 * size)) {
        bitmap = wxBitmap(dbl, -1, 2.0);
    } else if (dbl.GetHeight() > (2*size)) {
        wxImage scaled = image.Scale(size*2, size*2, wxIMAGE_QUALITY_HIGH);
        bitmap = wxBitmap(scaled, -1, 2.0);
    } else if (image.GetHeight() == size) {
        bitmap = wxBitmap(image);
    } else {
        wxImage scaled = image.Scale(size, size, wxIMAGE_QUALITY_HIGH);
        bitmap = wxBitmap(scaled);
    }
}

void AdjustAndSetBitmap(int size, wxImage &image, wxBitmap&bitmap) {
    if (image.GetHeight() == size) {
        bitmap = wxBitmap(image);
    } else {
        wxImage scaled = image.Scale(size, size, wxIMAGE_QUALITY_HIGH);
        bitmap = wxBitmap(scaled);
    }
}

void RenderableEffect::initBitmaps(const char **data16,
                                   const char **data24,
                                   const char **data32,
                                   const char **data48,
                                   const char **data64) {
    wxImage image16(data16);
    wxImage image24(data24);
    wxImage image32(data32);
    wxImage image48(data48);
    wxImage image64(data64);
    AdjustAndSetBitmap(16, image16, image32, icon16);
    AdjustAndSetBitmap(24, image24, image48, icon24);
    AdjustAndSetBitmap(32, image32, image64, icon32);
    AdjustAndSetBitmap(48, image48, icon48);
    AdjustAndSetBitmap(64, image64, icon64);
    AdjustAndSetBitmap(16, image16, icon16e);
    AdjustAndSetBitmap(24, image24, icon24e);
    AdjustAndSetBitmap(32, image32, icon32e);
    AdjustAndSetBitmap(48, image48, icon48e);
    AdjustAndSetBitmap(48, image64, icon64e);
}

// return true if version string is older than compare string
bool RenderableEffect::IsVersionOlder(const std::string& compare, const std::string& version)
{
    return ::IsVersionOlder(compare, version);
}

// this is recursive
static std::string GetEffectStringFromWindow(wxWindow *ParentWin)
{
    wxString s;
    for (const auto& it : ParentWin->GetChildren())
    {
        wxWindow *ChildWin = it;
        if (!ChildWin->IsEnabled()) {
            continue;
        }
        wxString ChildName = ChildWin->GetName();
        wxString AttrName = "E_" + ChildName.Mid(3);
        if (ChildName.StartsWith("ID_SLIDER"))
        {
            wxSlider* ctrl=(wxSlider*)ChildWin;
            s+=AttrName+ "=" + wxString::Format("%d",ctrl->GetValue()) + ",";
        }
        else if (ChildName.StartsWith("ID_VALUECURVE"))
        {
            ValueCurveButton* ctrl = (ValueCurveButton*)ChildWin;
            if (ctrl->GetValue()->IsActive())
            {
                s += AttrName + "=" + ctrl->GetValue()->Serialise() + ",";
            }
        }
        else if (ChildName.StartsWith("ID_TEXTCTRL"))
        {
            wxTextCtrl* ctrl=(wxTextCtrl*)ChildWin;
            wxString v = ctrl->GetValue();
            v.Replace("&", "&amp;", true);
            v.Replace(",", "&comma;", true);
            s+=AttrName + "=" + v + ",";
        }
		else if (ChildName.StartsWith("ID_SPINCTRL"))
		{
			wxSpinCtrl* ctrl = (wxSpinCtrl*)ChildWin;
			int i = ctrl->GetValue();
			s += AttrName + "=" + wxString::Format(wxT("%i"), i) + ",";
		}
		else if (ChildName.StartsWith("ID_CHOICE"))
        {
            wxChoice* ctrl=(wxChoice*)ChildWin;
            s+=AttrName + "=" + ctrl->GetStringSelection() + ",";
        }
        else if (ChildName.StartsWith("ID_CHECKBOX"))
        {
            wxCheckBox* ctrl=(wxCheckBox*)ChildWin;
            wxString checkedVal =(ctrl->IsChecked()) ? "1" : "0";
            s+=AttrName + "=" + checkedVal + ",";
        }
        else if (ChildName.StartsWith("ID_FILEPICKER") || ChildName.StartsWith("ID_0FILEPICKER"))
        {
            wxFilePickerCtrl* ctrl=(wxFilePickerCtrl*)ChildWin;
            s+=AttrName + "=" + ctrl->GetFileName().GetFullPath() + ",";
        }
        else if (ChildName.StartsWith("ID_FONTPICKER"))
        {
            wxFontPickerCtrl* ctrl=(wxFontPickerCtrl*)ChildWin;
            wxFont f = ctrl->GetSelectedFont();
            if (f.IsOk()) {
                wxString FontDesc=f.GetNativeFontInfoUserDesc();
                FontDesc.Replace(" unknown-90","");
                s+=AttrName + "=" + FontDesc + ",";
            } else {
                s+=AttrName + "=,";
            }
        }
        else if (ChildName.StartsWith("ID_NOTEBOOK") || ChildName.StartsWith("IDD_NOTEBOOK"))
        {
            wxNotebook* ctrl=(wxNotebook*)ChildWin;
            //for IDD_ stuff, don't record the value of the actual page selected
            if (ChildName.StartsWith("ID_NOTEBOOK")) {
                s+= AttrName + "=" ;
                s+=ctrl->GetPageText(ctrl->GetSelection());
                s+=",";
            }
            for(int i = 0; i<ctrl->GetPageCount(); i++)
            {
                wxString pageString = GetEffectStringFromWindow(ctrl->GetPage(i));
                if (pageString.size() > 0) {
                    s += pageString;
                    if (!s.EndsWith(",")) {
                        s += ",";
                    }
                }
            }
        }
        else if (ChildName.StartsWith("ID_PANEL_"))
        {
            wxString pageString = GetEffectStringFromWindow(ChildWin);
            if (pageString.size() > 0) {
                s += pageString;
                if (!s.EndsWith(",")) {
                    s += ",";
                }
            }
        }
    }
    s = s.Mid(0,s.size()-1);
    return s.ToStdString();
}

std::string RenderableEffect::GetEffectString() {
    return GetEffectStringFromWindow(panel);
}

bool RenderableEffect::SupportsRenderCache(const SettingsMap& settings) const
{
    for (const auto& it : settings)
    {
        // we want to cache blur because of compute cost
        if (Contains(it.first, "SLIDER_Blur") ||
            Contains(it.first, "VALUECURVE_Blur"))
        {
            return true;
        }
        
        // we want to cache rotations because of compute cost
        if (Contains(it.first, "VALUECURVE_Rotations"))
        {
            return true;
        }
    }
    return false;
}

bool RenderableEffect::needToAdjustSettings(const std::string &version) {
    return IsVersionOlder("2019.61", version);
}

void RenderableEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults) {

    if (IsVersionOlder("2019.61", version))
    {
        SettingsMap& sm = effect->GetSettings();

        wxString rzRotations = sm.Get("B_VALUECURVE_Rotations", "");
        if (rzRotations.Contains("VALUECURVE") && !rzRotations.Contains("RV=TRUE"))
        {
            ValueCurve vc;
            vc.SetLimits(0, 200);
            vc.SetDivisor(10);
            vc.Deserialise(rzRotations);
            sm["B_VALUECURVE_Rotations"] = vc.Serialise();
            wxASSERT(vc.IsRealValue());
        }

        wxString rzZoom = sm.Get("B_VALUECURVE_Zoom", "");
        if (rzZoom.Contains("VALUECURVE") && !rzZoom.Contains("RV=TRUE"))
        {
            ValueCurve vc;
            vc.SetLimits(0, 30);
            vc.SetDivisor(10);
            vc.Deserialise(rzZoom);
            sm["B_VALUECURVE_Zoom"] = vc.Serialise();
            wxASSERT(vc.IsRealValue());
        }

        if (IsVersionOlder("2018.50", version))
        {
            // Try to fix value curve issues
            for (auto s : sm)
            {
                wxString f(s.first);
                if (f.Contains("VALUECURVE") && !f.Contains("RV=TRUE"))
                {
                    ValueCurve vc(s.second);
                    sm[s.first] = vc.Serialise();
                }

                wxString v(s.second);
                if (v.Contains("ID_VALUECURVE_Blur"))
                {
                    ValueCurve vc;
                    vc.SetLimits(BLUR_MIN, BLUR_MAX);
                    vc.SetDivisor(1);
                    vc.Deserialise(s.second);
                    sm[s.first] = vc.Serialise();
                    wxASSERT(vc.IsRealValue());
                }
                else if (v.Contains("ID_VALUECURVE_Fan_Blade_Angle"))
                {
                    ValueCurve vc;
                    vc.SetLimits(FAN_BLADEANGLE_MIN, FAN_BLADEANGLE_MAX);
                    vc.SetDivisor(1);
                    vc.Deserialise(s.second);
                    sm[s.first] = vc.Serialise();
                    wxASSERT(vc.IsRealValue());
                }
                else if (v.Contains("ID_VALUECURVE_Spirals_Rotation"))
                {
                    ValueCurve vc;
                    vc.SetLimits(SPIRALS_ROTATION_MIN, SPIRALS_ROTATION_MAX);
                    vc.SetDivisor(SPIRALS_ROTATION_DIVISOR);
                    vc.Deserialise(s.second);
                    sm[s.first] = vc.Serialise();
                    wxASSERT(vc.IsRealValue());
                }
                else if (v.Contains("ID_VALUECURVE_Fan_Start_Angle"))
                {
                    ValueCurve vc;
                    vc.SetLimits(FAN_STARTANGLE_MIN, FAN_STARTANGLE_MAX);
                    vc.Deserialise(s.second);
                    sm[s.first] = vc.Serialise();
                    wxASSERT(vc.IsRealValue());
                }
                else if (v.Contains("ID_VALUECURVE_PinwheelXC"))
                {
                    ValueCurve vc;
                    vc.SetLimits(PINWHEEL_X_MIN, PINWHEEL_X_MAX);
                    vc.Deserialise(s.second);
                    sm[s.first] = vc.Serialise();
                    wxASSERT(vc.IsRealValue());
                }
                else if (v.Contains("ID_VALUECURVE_PinwheelYC"))
                {
                    ValueCurve vc;
                    vc.SetLimits(PINWHEEL_Y_MIN, PINWHEEL_Y_MAX);
                    vc.Deserialise(s.second);
                    sm[s.first] = vc.Serialise();
                    wxASSERT(vc.IsRealValue());
                }
                else if (v.Contains("ID_VALUECURVE_Spirals_Count"))
                {
                    ValueCurve vc;
                    vc.SetLimits(SPIRALS_COUNT_MIN, SPIRALS_COUNT_MAX);
                    vc.Deserialise(s.second);
                    sm[s.first] = vc.Serialise();
                    wxASSERT(vc.IsRealValue());
                }
            }

            if (IsVersionOlder("2018.12", version))
            {
                wxString layerMethod = sm.Get("T_CHOICE_LayerMethod", "");

                if (layerMethod == "Canvas")
                {
                    sm["T_CHOICE_LayerMethod"] = "Effect 1";
                    sm["T_CHECKBOX_Canvas"] = "1";
                }

                if (IsVersionOlder("2016.50", version))
                {
                    // Fix #622 - circle and square explode on transition out ... this code stops me breaking existing sequences
                    if (sm.Get("T_CHOICE_Out_Transition_Type", "") == "Square Explode" ||
                        sm.Get("T_CHOICE_Out_Transition_Type", "") == "Circle Explode")
                    {
                        if (sm.GetBool("T_CHECKBOX_Out_Transition_Reverse", false))
                        {
                            sm.erase("T_CHECKBOX_Out_Transition_Reverse");
                        }
                        else
                        {
                            sm["T_CHECKBOX_Out_Transition_Reverse"] = "1";
                        }
                    }

                    if (IsVersionOlder("2016.36", version) && removeDefaults) {
                        RemoveDefaults(version, effect);

                        if (IsVersionOlder("4.2.20", version)) {
                            // almost all of the settings from older 4.x series need adjustment for speed things
                            AdjustSettingsToBeFitToTime(effect->GetEffectIndex(), effect->GetSettings(), effect->GetStartTimeMS(), effect->GetEndTimeMS(), effect->GetPalette());
                        }
                    }
                }
            }
        }
    }
}

void RenderableEffect::RemoveDefaults(const std::string &version, Effect *effect) {
    SettingsMap &palette = effect->GetPaletteMap();
    bool changed = false;
    if (palette.Get("C_CHECKBOX_Palette1", "") == "0") {
        palette.erase("C_CHECKBOX_Palette1");
        changed = true;
    }
    if (palette.Get("C_CHECKBOX_Palette2", "") == "0") {
        palette.erase("C_CHECKBOX_Palette2");
        changed = true;
    }
    if (palette.Get("C_CHECKBOX_Palette3", "") == "0") {
        palette.erase("C_CHECKBOX_Palette3");
        changed = true;
    }
    if (palette.Get("C_CHECKBOX_Palette4", "") == "0") {
        palette.erase("C_CHECKBOX_Palette4");
        changed = true;
    }
    if (palette.Get("C_CHECKBOX_Palette5", "") == "0") {
        palette.erase("C_CHECKBOX_Palette5");
        changed = true;
    }
    if (palette.Get("C_CHECKBOX_Palette6", "") == "0") {
        palette.erase("C_CHECKBOX_Palette6");
        changed = true;
    }
    if (palette.Get("C_CHECKBOX_Palette7", "") == "0") {
        palette.erase("C_CHECKBOX_Palette7");
        changed = true;
    }
    if (palette.Get("C_CHECKBOX_Palette8", "") == "0") {
        palette.erase("C_CHECKBOX_Palette8");
        changed = true;
    }
    if (palette.Get("C_SLIDER_Brightness", "") == "100") {
        palette.erase("C_SLIDER_Brightness");
        changed = true;
    }
    if (palette.Get("C_SLIDER_Color_HueAdjust", "") == "0") {
        palette.erase("C_SLIDER_Color_HueAdjust");
        changed = true;
    }
    if (palette.Get("C_SLIDER_Color_SaturationAdjust", "") == "0") {
        palette.erase("C_SLIDER_Color_SaturationAdjust");
        changed = true;
    }
    if (palette.Get("C_SLIDER_Color_ValueAdjust", "") == "0") {
        palette.erase("C_SLIDER_Color_ValueAdjust");
        changed = true;
    }
    if (palette.Get("C_SLIDER_Contrast", "") == "0") {
        palette.erase("C_SLIDER_Contrast");
        changed = true;
    }
    if (palette.Get("C_SLIDER_SparkleFrequency", "") == "0") {
        palette.erase("C_SLIDER_SparkleFrequency");
        changed = true;
    }
    if (changed) {
        effect->PaletteMapUpdated();
    }
    SettingsMap &settings = effect->GetSettings();
    if (settings.Get("T_CHECKBOX_LayerMorph", "") == "0") {
        settings.erase("T_CHECKBOX_LayerMorph");
    }
    if (settings.Get("T_CHECKBOX_OverlayBkg", "") == "0") {
        settings.erase("T_CHECKBOX_OverlayBkg");
    }
    if (settings.Get("T_CHOICE_LayerMethod", "") == "Normal") {
        settings.erase("T_CHOICE_LayerMethod");
    }
    if (settings.Get("T_SLIDER_EffectLayerMix", "") == "0") {
        settings.erase("T_SLIDER_EffectLayerMix");
    }
    if (settings.GetFloat("T_TEXTCTRL_Fadein", 1.0f) == 0.0f) {
        settings.erase("T_TEXTCTRL_Fadein");
    }
    if (settings.GetFloat("T_TEXTCTRL_Fadeout", 1.0f) == 0.0f) {
        settings.erase("T_TEXTCTRL_Fadeout");
    }
}

void RenderableEffect::AdjustSettingsToBeFitToTime(int effectIdx, SettingsMap &settings, int startMS, int endMS, xlColorVector &colors)
{
    if (effectIdx == EffectManager::eff_FACES
        && settings.Get("E_CHOICE_Faces_FaceDefinition", "") == ""
        && settings.Get("E_CHOICE_Faces_TimingTrack", "") == "")
    {
        settings["E_CHOICE_Faces_FaceDefinition"] = "xlights_papagayo.xml";
    }

    int ftt = wxAtoi(settings.Get("T_CHECKBOX_FitToTime", "1"));
    switch (effectIdx)
    {
            //these effects have never used the FitToTime or speed settings, nothing to do
        case EffectManager::eff_OFF:
        case EffectManager::eff_GALAXY:
        case EffectManager::eff_FAN:
        case EffectManager::eff_MARQUEE:
        case EffectManager::eff_MORPH:
        case EffectManager::eff_SHOCKWAVE:
        case EffectManager::eff_GLEDIATOR:
        case EffectManager::eff_FACES:
            break;
        case EffectManager::eff_STROBE:
        case EffectManager::eff_TWINKLE:
            break;
            //these effects have been updated to have a dedicated repeat or speed or other control
            //and now ignore the FitToTime and Speed sliders, but the settings need adjusting
        case EffectManager::eff_ON:
            if (settings.Get("E_TEXTCTRL_On_Cycles", "") == "")
            {
                float cycles = 1.0;
                if (!ftt)
                {
                    int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                    int totalTime = endMS - startMS;
                    int maxState = totalTime * speed / 50;
                    cycles = maxState / 200;
                }
                settings["E_TEXTCTRL_On_Cycles"] = wxString::Format("%0.2f", cycles);
            }
            break;
        case EffectManager::eff_SNOWSTORM:
            if (settings.Get("E_SLIDER_Snowstorm_Speed", "") == "")
            {
                settings["E_SLIDER_Snowstorm_Speed"] = settings.Get("T_SLIDER_Speed", "10");
            }
            break;
        case EffectManager::eff_SNOWFLAKES:
            if (settings.Get("E_SLIDER_Snowflakes_Speed", "") == "")
            {
                settings["E_SLIDER_Snowflakes_Speed"] = settings.Get("T_SLIDER_Speed", "10");
            }
            break;
        case EffectManager::eff_BUTTERFLY:
            if (settings.Get("E_SLIDER_Butterfly_Speed", "") == "")
            {
                settings["E_SLIDER_Butterfly_Speed"] = settings.Get("T_SLIDER_Speed", "10");
            }
            break;
        case EffectManager::eff_CIRCLES:
            if (settings.Get("E_SLIDER_Circles_Speed", "") == "")
            {
                settings["E_SLIDER_Circles_Speed"] = settings.Get("T_SLIDER_Speed", "10");
            }
            break;
        case EffectManager::eff_LIFE:
            if (settings.Get("E_SLIDER_Life_Speed", "") == "")
            {
                settings["E_SLIDER_Life_Speed"] = settings.Get("T_SLIDER_Speed", "10");
            }
            break;
        case EffectManager::eff_METEORS:
            if (settings.Get("E_SLIDER_Meteors_Speed", "") == "")
            {
                settings["E_SLIDER_Meteors_Speed"] = settings.Get("T_SLIDER_Speed", "10");
            }
            break;
        case EffectManager::eff_TREE:
            if (settings.Get("E_SLIDER_Tree_Speed", "") == "")
            {
                settings["E_SLIDER_Tree_Speed"] = settings.Get("T_SLIDER_Speed", "10");
            }
            break;
        case EffectManager::eff_PINWHEEL:
            if (settings.Get("E_TEXTCTRL_Pinwheel_Speed", "") == "")
            {
                settings["E_TEXTCTRL_Pinwheel_Speed"] = settings.Get("T_SLIDER_Speed", "10");
            }
            break;
        case EffectManager::eff_PLASMA:
            if (settings.Get("E_TEXTCTRL_Plasma_Speed", "") == "")
            {
                settings["E_TEXTCTRL_Plasma_Speed"] = settings.Get("T_SLIDER_Speed", "10");
            }
            break;
        case EffectManager::eff_TEXT:
            if (settings.Get("E_TEXTCTRL_Text_Speed1", "") == "")
            {
                settings["E_TEXTCTRL_Text_Speed1"] = settings.Get("T_SLIDER_Speed", "10");
            }
            if (settings.Get("E_TEXTCTRL_Text_Speed2", "") == "")
            {
                settings["E_TEXTCTRL_Text_Speed2"] = settings.Get("T_SLIDER_Speed", "10");
            }
            if (settings.Get("E_TEXTCTRL_Text_Speed3", "") == "")
            {
                settings["E_TEXTCTRL_Text_Speed3"] = settings.Get("T_SLIDER_Speed", "10");
            }
            if (settings.Get("E_TEXTCTRL_Text_Speed4", "") == "")
            {
                settings["E_TEXTCTRL_Text_Speed4"] = settings.Get("T_SLIDER_Speed", "10");
            }

            if (settings.Get("E_SLIDER_Text_Position1", "") != "") {
                int pos = wxAtoi(settings.Get("E_SLIDER_Text_Position1", "50")) * 2 - 100;
                settings.erase("E_SLIDER_Text_Position1");
                settings["E_SLIDER_Text_XStart1"] = wxString::Format("%d", 0);
                settings["E_SLIDER_Text_YStart1"] = wxString::Format("%d", pos);
                settings["E_SLIDER_Text_XEnd1"] = wxString::Format("%d", 0);
                settings["E_SLIDER_Text_YEnd1"] = wxString::Format("%d", pos);
            }
            break;
        case EffectManager::eff_WAVE:
            if (settings.Get("E_TEXTCTRL_Wave_Speed", "") == "")
            {
                settings["E_TEXTCTRL_Wave_Speed"] = settings.Get("T_SLIDER_Speed", "10");
            }
            break;
        case EffectManager::eff_SPIROGRAPH:
            if (settings.Get("E_TEXTCTRL_Spirograph_Speed", "") == "")
            {
                settings["E_TEXTCTRL_Spirograph_Speed"] = settings.Get("T_SLIDER_Speed", "10");
            }
            if (settings.Get("E_CHECKBOX_Spirograph_Animate", "") != "")
            {
                int i = wxAtoi(settings.Get("E_CHECKBOX_Spirograph_Animate", "0"));
                settings["E_TEXTCTRL_Spirograph_Animate"] = (i == 0 ? "0" : "10");
                settings.erase("E_CHECKBOX_Spirograph_Animate");
            }
            break;

        case EffectManager::eff_COLORWASH:
            if (settings.Get("E_TEXTCTRL_ColorWash_Cycles", "") == "")
            {
                double count = wxAtoi(settings.Get("E_SLIDER_ColorWash_Count", "1"));
                settings.erase("E_SLIDER_ColorWash_Count");
                if (settings["T_CHECKBOX_FitToTime"] == "1")
                {
                    count = 1.0;
                    settings["E_CHECKBOX_ColorWash_CircularPalette"] = "0";
                }
                else
                {
                    settings["E_CHECKBOX_ColorWash_CircularPalette"] = "1";
                }
                settings["E_TEXTCTRL_ColorWash_Cycles"] = wxString::Format("%0.2f", count);
            }
            break;
        case EffectManager::eff_FIRE:
            if (settings.Get("E_TEXTCTRL_Fire_GrowthCycles", "") == "")
            {
                bool grow = settings["E_CHECKBOX_Fire_GrowFire"] == "1";
                settings.erase("E_CHECKBOX_Fire_GrowFire");
                if (grow)
                {
                    int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                    int totalTime = endMS - startMS;
                    double maxState = totalTime * speed / 50;
                    double cycles = maxState / 500.0;
                    settings["E_TEXTCTRL_Fire_GrowthCycles"] = wxString::Format("%0.2f", cycles);
                }
                else
                {
                    settings["E_TEXTCTRL_Fire_GrowthCycles"] = "0.0";
                }
            }
            break;
        case EffectManager::eff_FIREWORKS:
            if (settings.Get("E_SLIDER_Fireworks_Number_Explosions", "") != "")
            {
                int cnt = wxAtoi(settings.Get("E_SLIDER_Fireworks_Number_Explosions", "10"));
                settings.erase("E_SLIDER_Fireworks_Number_Explosions");
                int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                int total = (speed * cnt) / 50;
                if (total > 50)
                {
                    total = 50;
                }
                if (total < 1)
                {
                    total = 1;
                }
                settings["E_SLIDER_Fireworks_Explosions"] = wxString::Format("%d", total);
            }
            break;
        case EffectManager::eff_RIPPLE:
            if (settings.Get("E_TEXTCTRL_Ripple_Cycles", "") == "")
            {
                float cycles = 1.0;
                if (!ftt)
                {
                    int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                    int totalTime = endMS - startMS;
                    int maxState = totalTime * speed / 50;
                    cycles = maxState / 200;
                }
                settings["E_TEXTCTRL_Ripple_Cycles"] = wxString::Format("%0.2f", cycles);
            }
            break;
        case EffectManager::eff_BARS:
            if (settings.Get("E_TEXTCTRL_Bars_Cycles", "") == "")
            {
                float cycles = 1.0;
                wxString dir = settings["E_CHOICE_Bars_Direction"];
                if (!ftt)
                {
                    int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                    int totalTime = endMS - startMS;
                    int maxState = totalTime * speed / 50;
                    if (dir.Contains("Altern"))
                    {
                        cycles = maxState * 2;
                    }
                    else
                    {
                        cycles = maxState / 200;
                    }
                }
                settings["E_TEXTCTRL_Bars_Cycles"] = wxString::Format("%0.2f", cycles);
            }
            break;
        case EffectManager::eff_SPIRALS:
            if (settings.Get("E_TEXTCTRL_Spirals_Movement", "") == "")
            {
                float cycles = 1.0;
                int dir = wxAtoi(settings.Get("E_SLIDER_Spirals_Direction", "1"));
                settings.erase("E_SLIDER_Spirals_Direction");
                if (!ftt)
                {
                    int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                    int totalTime = endMS - startMS;
                    int maxState = totalTime * speed / 50;
                    cycles = maxState / 600;
                }
                settings["E_TEXTCTRL_Spirals_Movement"] = wxString::Format("%0.2f", dir * cycles);
            }
            break;
        case EffectManager::eff_CURTAIN:
            if (settings.Get("E_TEXTCTRL_Curtain_Speed", "") == "")
            {
                float cycles = 1.0;
                if (!ftt)
                {
                    int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                    int totalTime = endMS - startMS;
                    int maxState = totalTime * speed / 50;
                    cycles = maxState / 200;
                }
                settings["E_TEXTCTRL_Curtain_Speed"] = wxString::Format("%0.2f", cycles);
            }
            break;
        case EffectManager::eff_SINGLESTRAND:
            if ("Skips" == settings["E_NOTEBOOK_SSEFFECT_TYPE"])
            {
                if (settings.Get("E_SLIDER_Skips_Advance", "") == "")
                {
                    int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                    settings["E_SLIDER_Skips_Advance"] = wxString::Format("%d", speed - 1);
                }
            }
            else
            {
                wxString type = settings.Get("E_CHOICE_Chase_Type1", "Left-Right");
                if (type == "Auto reverse")
                {
                    type = "Bounce from Left";
                    settings["E_CHOICE_Chase_Type1"] = type;
                }
                else if (type == "Bounce" || type == "Pacman")
                {
                    type = "Dual Bounce";
                    settings["E_CHOICE_Chase_Type1"] = type;
                }
                if (settings.Get("E_TEXTCTRL_Chase_Rotations", "") == "")
                {
                    float cycles = 1.0;
                    if (!ftt)
                    {
                        int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                        int totalTime = endMS - startMS;
                        int maxState = totalTime * speed / 50;
                        cycles = maxState / 250.0;
                    }
                    settings["E_TEXTCTRL_Chase_Rotations"] = wxString::Format("%0.2f", cycles);
                }
            }
            break;
        case EffectManager::eff_SHIMMER:
            if (settings.Get("E_TEXTCTRL_Shimmer_Cycles", "") == "")
            {
                float cycles = 1.0;
                int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                int totalTime = endMS - startMS;
                int maxState = totalTime * speed / 50;
                cycles = maxState / (100.0 * colors.size());
                settings["E_TEXTCTRL_Shimmer_Cycles"] = wxString::Format("%0.2f", cycles);
            }
            break;
        case EffectManager::eff_PICTURES:
            if (settings.Get("E_TEXTCTRL_Pictures_FrameRateAdj", "") == "")
            {
                if (settings.Get("E_CHECKBOX_MovieIs20FPS", "") == "1")
                {
                    settings["E_TEXTCTRL_Pictures_FrameRateAdj"] = "1.0";
                }
                else if (settings.Get("E_SLIDER_Pictures_GifSpeed", "") == "0")
                {
                    settings["E_TEXTCTRL_Pictures_FrameRateAdj"] = "0.0";
                }
                else if (!ftt)
                {
                    int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                    int totalTime = endMS - startMS;
                    int maxState = totalTime * speed / 50;
                    double cycles = maxState / 300.0;
                    settings["E_TEXTCTRL_Pictures_Speed"] = wxString::Format("%0.2f", cycles);
                }

                settings.erase("E_CHECKBOX_MovieIs20FPS");
                settings.erase("E_SLIDER_Pictures_GifSpeed");
            }
            break;
        case EffectManager::eff_GARLANDS:
            //Don't attempt to map the Garlands speed settings.  In v3, the Garland speed depended on the Speed setting, the
            //Spacing setting as well as the height of the model.  We don't have the height of the model here so really
            //no way to figure out the speed or an appropriate mapping
            break;

            //these all need code updated and new sliders and such before we can map them
            //these all have state/speed requirements
        case EffectManager::eff_PIANO:
            break;
    }
    settings.erase("T_CHECKBOX_FitToTime");
    settings.erase("T_SLIDER_Speed");
}


void RenderableEffect::SetSliderValue(wxSlider *slider, int value) {
    slider->SetValue(value);
    wxScrollEvent event(wxEVT_SLIDER, slider->GetId());
    event.SetEventObject(slider);
    event.SetInt(value);
    slider->ProcessWindowEvent(event);
}

void RenderableEffect::SetSpinValue(wxSpinCtrl *spin, int value) {
    spin->SetValue(value);
    wxCommandEvent event(wxEVT_SPIN, spin->GetId());
    event.SetEventObject(spin);
    event.SetInt(value);
    spin->ProcessWindowEvent(event);
}

void RenderableEffect::SetChoiceValue(wxChoice *choice, std::string value) {
    choice->SetStringSelection(wxString(value.c_str()));
    wxCommandEvent event(wxEVT_CHOICE, choice->GetId());
    event.SetEventObject(choice);
    event.SetString(wxString(value.c_str()));
    choice->ProcessWindowEvent(event);
}

void RenderableEffect::SetTextValue(wxTextCtrl *text, std::string value) {
    text->SetValue(wxString(value.c_str()));
    wxCommandEvent event(wxEVT_TEXT, text->GetId());
    event.SetEventObject(text);
    event.SetString(wxString(value.c_str()));
    text->ProcessWindowEvent(event);
}

void RenderableEffect::SetCheckBoxValue(wxCheckBox *c, bool b) {
    c->SetValue(b);
    wxCommandEvent evt(wxEVT_COMMAND_CHECKBOX_CLICKED, c->GetId());
    evt.SetEventObject(c);
    evt.SetInt(b);
    c->ProcessWindowEvent(evt);
}

void RenderableEffect::SetRadioValue(wxRadioButton *r) {
    r->SetValue(true);
    wxCommandEvent evt(wxEVT_RADIOBUTTON, r->GetId());
    evt.SetEventObject(r);
    evt.SetInt(true);
    r->ProcessWindowEvent(evt);
}

double RenderableEffect::GetValueCurveDouble(const std::string &name, double def, SettingsMap &SettingsMap, float offset, double min, double max, long startMS, long endMS, int divisor)
{
    double res = def;

    const std::string sn = "SLIDER_" + name;
    const std::string tn = "TEXTCTRL_" + name;
    //bool slider = false;
    if (SettingsMap.Contains(sn))
    {
        res = SettingsMap.GetDouble(sn, def);
        //slider = true;
    }
    else if (SettingsMap.Contains(tn))
    {
        res = SettingsMap.GetDouble(tn, def);
    }

    wxString vn = "VALUECURVE_" + name;

    wxString vc = SettingsMap.Get(vn, "");

    // Temporary logging to try to find why we get a shader crash here
    xLightsApp::GetFrame()->AddTraceMessage("RenderableEffect::GetValueCurveDouble '" + name + "' '" + vn + "' '" + vc + "'");

    if (vc != "")
    {
        bool needsUpgrade = !vc.Contains("RV=TRUE");
        ValueCurve valc(vc.ToStdString());
        if (valc.IsActive())
        {
            valc.SetLimits(min, max);
            valc.SetDivisor(divisor);

            // If we ask for a double we always want it pre-divided
            //if (slider)
            //{
            //    res = valc.GetOutputValueAt(offset);
            //}
            //else
            //{
                res = valc.GetOutputValueAtDivided(offset, startMS, endMS);
            //}

            if (needsUpgrade)
            {
                SettingsMap[vn] = valc.Serialise();
            }
        }
    }

    return res;
}

int RenderableEffect::GetValueCurveInt(const std::string &name, int def, SettingsMap &SettingsMap, float offset, int min, int max, long startMS, long endMS, int divisor)
{
    int res = def;
    const std::string sn = "SLIDER_" + name;
    const std::string tn = "TEXTCTRL_" + name;
    //bool slider = false;
    if (SettingsMap.Contains(sn))
    {
        res = SettingsMap.GetInt(sn, def);
        //slider = true;
    }
    else if (SettingsMap.Contains(tn))
    {
        res = SettingsMap.GetInt(tn, def);
    }

    const std::string vn = "VALUECURVE_" + name;
    if (SettingsMap.Contains(vn))
    {
        wxString vc = SettingsMap.Get(vn, "");

        bool needsUpgrade = !vc.Contains("RV=TRUE");

        ValueCurve valc;
        valc.SetDivisor(divisor);
        valc.SetLimits(min, max);
        valc.Deserialise(vc.ToStdString());
        if (valc.IsActive())
        {
            // If we ask for an int then we seem to want it undivided
            //if (!slider)
            //{
                res = valc.GetOutputValueAt(offset, startMS, endMS);
            //}
            //else
            //{
            //    res = valc.GetOutputValueAtDivided(offset);
            //}

            if (needsUpgrade)
            {
                // this updates the settings map ... but not the actual settings on the effect ... 
                // this is a problem as the error will keep occuring next time the sequence is loaded.
                // To fix it the user needs to click on the offending effect and save and it will go away
                SettingsMap[vn] = valc.Serialise();
            }
        }
    }

    return res;
}

EffectLayer* RenderableEffect::GetTiming(const std::string& timingtrack) const
{
    if (timingtrack == "") return nullptr;

    for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
    {
        Element* e = mSequenceElements->GetElement(i);
        if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING && e->GetName() == timingtrack)
        {
            return e->GetEffectLayer(0);
            break;
        }
    }
    return nullptr;
}

std::string RenderableEffect::GetTimingTracks(const int max, const int equals) const
{
    std::string timingtracks = "";
    for (size_t i = 0; i < mSequenceElements->GetElementCount(); i++)
    {
        Element* e = mSequenceElements->GetElement(i);
        if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING && (max < 1 || e->GetEffectLayerCount() <= max) && (equals == 0 || e->GetEffectLayerCount() == equals))
        {
            if (timingtracks != "")
            {
                timingtracks += "|";
            }
            timingtracks += e->GetName();
        }
    }
    return timingtracks;
}

Effect* RenderableEffect::GetCurrentTiming(const RenderBuffer& buffer, const std::string& timingtrack) const
{
    EffectLayer* el = GetTiming(timingtrack);

    if (el == nullptr) return nullptr;

    int currentMS = buffer.curPeriod * buffer.frameTimeInMs;
    for (int j = 0; j < el->GetEffectCount(); j++)
    {
        if (el->GetEffect(j)->GetStartTimeMS() <= currentMS &&
            el->GetEffect(j)->GetEndTimeMS() > currentMS)
        {
            return el->GetEffect(j);
        }
    }

    return nullptr;
}

