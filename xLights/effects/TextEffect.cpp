/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "TextEffect.h"

#include <mutex>
#include <array>
#include <unordered_map>

#include "TextPanel.h"
#include <wx/checkbox.h>

#include "../sequencer/Effect.h"
#include "../sequencer/Element.h"
#include "../sequencer/EffectsGrid.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../models/Model.h"
#include "../UtilFunctions.h"
#include "../FontManager.h"
#include "../xLightsMain.h"
#include "../ExternalHooks.h"
#include "../xLightsXmlFile.h"

#include "../../include/text-16.xpm"
#include "../../include/text-24.xpm"
#include "../../include/text-32.xpm"
#include "../../include/text-48.xpm"
#include "../../include/text-64.xpm"
#include <log4cpp/Category.hh>

#define MAXTEXTLINES 100

TextEffect::TextEffect(int id) : RenderableEffect(id, "Text", text_16, text_24, text_32, text_48, text_64), font_mgr(FontManager::instance())
{
    //ctor
}

TextEffect::~TextEffect()
{
    //dtor
}

std::list<std::string> TextEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    wxString textFilename = settings.Get("E_FILEPICKERCTRL_Text_File", "");
    wxString text = ToWXString(settings.Get("E_TEXTCTRL_Text", ""));
    wxString lyricTrack = settings.Get("E_CHOICE_Text_LyricTrack", "");

    if (text == "" && textFilename == "" && lyricTrack == "")
    {
        res.push_back(wxString::Format("    ERR: Text effect has no actual text. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }
    else if (textFilename != "" && !FileExists(textFilename))
    {
        res.push_back(wxString::Format("    ERR: Text effect cant find file '%s'. Model '%s', Start %s", textFilename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }
    else if (textFilename != "" && !IsFileInShowDir(xLightsFrame::CurrentDir, textFilename.ToStdString()))
    {
        res.push_back(wxString::Format("    WARN: Text effect file '%s' not under show directory. Model '%s', Start %s", textFilename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    if (model->GetDisplayAs() == "ModelGroup")
    {
        res.push_back(wxString::Format("    WARN: Text effect generally does not work well on a model group. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

std::list<std::string> TextEffect::GetFileReferences(Model* model, const SettingsMap &SettingsMap) const
{
    std::list<std::string> res;    
    wxString textFilename = SettingsMap["E_FILEPICKERCTRL_Text_File"];
    if (textFilename != "")
    {
        res.push_back(textFilename);
    }
    return res;
}

bool TextEffect::CleanupFileLocations(xLightsFrame* frame, SettingsMap &SettingsMap)
{
    bool rc = false;
    wxString file = SettingsMap["E_FILEPICKERCTRL_Text_File"];
    if (FileExists(file))
    {
        if (!frame->IsInShowFolder(file))
        {
            SettingsMap["E_FILEPICKERCTRL_Text_File"] = frame->MoveToShowFolder(file, wxString(wxFileName::GetPathSeparator()));
            rc = true;
        }
    }

    return rc;
}

xlEffectPanel *TextEffect::CreatePanel(wxWindow *parent) {
    return new TextPanel(parent);
}

static inline void SetCheckboxValue(wxWindow *w, int id, bool b) {
    wxCheckBox *c = (wxCheckBox*)w->FindWindowById(id);
    c->SetValue(b);
    wxCommandEvent evt(wxEVT_COMMAND_CHECKBOX_CLICKED, id);
    evt.SetEventObject(c);
    evt.SetInt(b);
    c->ProcessWindowEvent(evt);
}

bool TextEffect::SupportsRenderCache(const SettingsMap& settings) const
{
    // we dont want to use render cache if text is coming from a file as the file might have changed
    if (ToWXString(settings["TEXTCTRL_Text"]) == "" && FileExists(settings["FILEPICKERCTRL_Text_File"]))
        return false;
    return true;
}

void TextEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults)
{
    SettingsMap &settings = effect->GetSettings();
    if (IsVersionOlder("2016.46", version) || RenderableEffect::needToAdjustSettings(version))
    {
        // this is to prevent recursive adjustments since we are adding
        // layers and may be called by for loops based on number of layers
        if (settings.Get("Converted", "xxx") == "1") {
            settings.erase("Converted");
            return;
        }

        if (RenderableEffect::needToAdjustSettings(version)) {
            RenderableEffect::adjustSettings(version, effect, removeDefaults);
        }

        if (IsVersionOlder("2016.46", version)) {
            settings["E_CHECKBOX_TextToCenter"] = settings["E_CHECKBOX_TextToCenter1"];
            settings["E_CHECKBOX_Text_PixelOffsets"] = settings["E_CHECKBOX_Text_PixelOffsets1"];
            settings["E_CHOICE_Text_Count"] = settings["E_CHOICE_Text_Count1"];
            settings["E_CHOICE_Text_Dir"] = settings["E_CHOICE_Text_Dir1"];
            settings["E_CHOICE_Text_Effect"] = settings["E_CHOICE_Text_Effect1"];
            settings["E_FONTPICKER_Text_Font"] = settings["E_FONTPICKER_Text_Font1"];
            settings["E_SLIDER_Text_XEnd"] = settings["E_SLIDER_Text_XEnd1"];
            settings["E_SLIDER_Text_XStart"] = settings["E_SLIDER_Text_XStart1"];
            settings["E_SLIDER_Text_YEnd"] = settings["E_SLIDER_Text_YEnd1"];
            settings["E_SLIDER_Text_YStart"] = settings["E_SLIDER_Text_YStart1"];
            settings["E_TEXTCTRL_Text"] = settings["E_TEXTCTRL_Text_Line1"];
            settings["E_TEXTCTRL_Text_Speed"] = settings["E_TEXTCTRL_Text_Speed1"];

            EffectLayer* el = effect->GetParentEffectLayer();
            Element* elem = el->GetParentElement();

            std::string line2 = settings["E_TEXTCTRL_Text_Line2"];
            std::string line3 = settings["E_TEXTCTRL_Text_Line3"];
            std::string line4 = settings["E_TEXTCTRL_Text_Line4"];

            if (line2 != "") {
                std::string palette = effect->GetPaletteAsString();
                EffectLayer* layer = EffectsGrid::FindOpenLayer(elem, effect->GetStartTimeMS(), effect->GetEndTimeMS());
                Effect* new_eff = layer->AddEffect(0, "Text", "", palette, effect->GetStartTimeMS(), effect->GetEndTimeMS(), false, false);
                SettingsMap &new_settings = new_eff->GetSettings();
                new_settings["Converted"] = "1";
                new_settings["E_TEXTCTRL_Text"] = line2;
                new_settings["E_CHECKBOX_Text_PixelOffsets"] = "0";
                new_settings["E_CHECKBOX_TextToCenter"] = settings["E_CHECKBOX_TextToCenter2"];
                new_settings["E_CHOICE_Text_Count"] = settings["E_CHOICE_Text_Count2"];
                new_settings["E_CHOICE_Text_Dir"] = settings["E_CHOICE_Text_Dir2"];
                new_settings["E_CHOICE_Text_Effect"] = settings["E_CHOICE_Text_Effect2"];
                new_settings["E_FONTPICKER_Text_Font"] = settings["E_FONTPICKER_Text_Font2"];
                new_settings["E_TEXTCTRL_Text_Speed"] = settings["E_TEXTCTRL_Text_Speed2"];
                int pos = (wxAtoi(settings["E_SLIDER_Text_Position2"]) * 2) - 100;
                wxString strpos = wxString::Format("%d", pos);
                new_settings["E_SLIDER_Text_XStart"] = "0";
                new_settings["E_SLIDER_Text_XEnd"] = "0";
                new_settings["E_SLIDER_Text_YStart"] = strpos;
                new_settings["E_SLIDER_Text_YEnd"] = strpos;
                SelectTextColor(palette, 2);
                new_eff->SetPalette(palette);
            }
            if (line3 != "") {
                std::string palette = effect->GetPaletteAsString();
                EffectLayer* layer = EffectsGrid::FindOpenLayer(elem, effect->GetStartTimeMS(), effect->GetEndTimeMS());
                Effect* new_eff = layer->AddEffect(0, "Text", "", palette, effect->GetStartTimeMS(), effect->GetEndTimeMS(), false, false);
                SettingsMap &new_settings = new_eff->GetSettings();
                new_settings["Converted"] = "1";
                new_settings["E_TEXTCTRL_Text"] = line3;
                new_settings["E_CHECKBOX_Text_PixelOffsets"] = "0";
                new_settings["E_CHECKBOX_TextToCenter"] = settings["E_CHECKBOX_TextToCenter3"];
                new_settings["E_CHOICE_Text_Count"] = settings["E_CHOICE_Text_Count3"];
                new_settings["E_CHOICE_Text_Dir"] = settings["E_CHOICE_Text_Dir3"];
                new_settings["E_CHOICE_Text_Effect"] = settings["E_CHOICE_Text_Effect3"];
                new_settings["E_FONTPICKER_Text_Font"] = settings["E_FONTPICKER_Text_Font3"];
                new_settings["E_TEXTCTRL_Text_Speed"] = settings["E_TEXTCTRL_Text_Speed3"];
                int pos = (wxAtoi(settings["E_SLIDER_Text_Position3"]) * 2) - 100;
                wxString strpos = wxString::Format("%d", pos);
                new_settings["E_SLIDER_Text_XStart"] = "0";
                new_settings["E_SLIDER_Text_XEnd"] = "0";
                new_settings["E_SLIDER_Text_YStart"] = strpos;
                new_settings["E_SLIDER_Text_YEnd"] = strpos;
                SelectTextColor(palette, 3);
                new_eff->SetPalette(palette);
            }
            if (line4 != "") {
                std::string palette = effect->GetPaletteAsString();
                EffectLayer* layer = EffectsGrid::FindOpenLayer(elem, effect->GetStartTimeMS(), effect->GetEndTimeMS());
                Effect* new_eff = layer->AddEffect(0, "Text", "", palette, effect->GetStartTimeMS(), effect->GetEndTimeMS(), false, false);
                SettingsMap &new_settings = new_eff->GetSettings();
                new_settings["Converted"] = "1";
                new_settings["E_TEXTCTRL_Text"] = line4;
                new_settings["E_CHECKBOX_Text_PixelOffsets"] = "0";
                new_settings["E_CHECKBOX_TextToCenter"] = settings["E_CHECKBOX_TextToCenter4"];
                new_settings["E_CHOICE_Text_Count"] = settings["E_CHOICE_Text_Count4"];
                new_settings["E_CHOICE_Text_Dir"] = settings["E_CHOICE_Text_Dir4"];
                new_settings["E_CHOICE_Text_Effect"] = settings["E_CHOICE_Text_Effect4"];
                new_settings["E_FONTPICKER_Text_Font"] = settings["E_FONTPICKER_Text_Font4"];
                new_settings["E_TEXTCTRL_Text_Speed"] = settings["E_TEXTCTRL_Text_Speed4"];
                int pos = (wxAtoi(settings["E_SLIDER_Text_Position4"]) * 2) - 100;
                wxString strpos = wxString::Format("%d", pos);
                new_settings["E_SLIDER_Text_XStart"] = "0";
                new_settings["E_SLIDER_Text_XEnd"] = "0";
                new_settings["E_SLIDER_Text_YStart"] = strpos;
                new_settings["E_SLIDER_Text_YEnd"] = strpos;
                SelectTextColor(palette, 4);
                new_eff->SetPalette(palette);
            }
            std::string palette = effect->GetPaletteAsString();
            SelectTextColor(palette, 1);
            effect->SetPalette(palette);

            settings.erase("E_CHECKBOX_TextToCenter1");
            settings.erase("E_CHECKBOX_TextToCenter2");
            settings.erase("E_CHECKBOX_TextToCenter3");
            settings.erase("E_CHECKBOX_TextToCenter4");
            settings.erase("E_CHECKBOX_Text_PixelOffsets1");
            settings.erase("E_CHOICE_Text_Count1");
            settings.erase("E_CHOICE_Text_Count2");
            settings.erase("E_CHOICE_Text_Count3");
            settings.erase("E_CHOICE_Text_Count4");
            settings.erase("E_CHOICE_Text_Dir1");
            settings.erase("E_CHOICE_Text_Dir2");
            settings.erase("E_CHOICE_Text_Dir3");
            settings.erase("E_CHOICE_Text_Dir4");
            settings.erase("E_CHOICE_Text_Effect1");
            settings.erase("E_CHOICE_Text_Effect2");
            settings.erase("E_CHOICE_Text_Effect3");
            settings.erase("E_CHOICE_Text_Effect4");
            settings.erase("E_FONTPICKER_Text_Font1");
            settings.erase("E_FONTPICKER_Text_Font2");
            settings.erase("E_FONTPICKER_Text_Font3");
            settings.erase("E_FONTPICKER_Text_Font4");
            settings.erase("E_SLIDER_Text_Position2");
            settings.erase("E_SLIDER_Text_Position3");
            settings.erase("E_SLIDER_Text_Position4");
            settings.erase("E_SLIDER_Text_XEnd1");
            settings.erase("E_SLIDER_Text_XStart1");
            settings.erase("E_SLIDER_Text_YEnd1");
            settings.erase("E_SLIDER_Text_YStart1");
            settings.erase("E_TEXTCTRL_Text_Line1");
            settings.erase("E_TEXTCTRL_Text_Line2");
            settings.erase("E_TEXTCTRL_Text_Line3");
            settings.erase("E_TEXTCTRL_Text_Line4");
            settings.erase("E_TEXTCTRL_Text_Speed1");
            settings.erase("E_TEXTCTRL_Text_Speed2");
            settings.erase("E_TEXTCTRL_Text_Speed3");
            settings.erase("E_TEXTCTRL_Text_Speed4");
        }
    }

    wxString file = settings["E_FILEPICKERCTRL_Text_File"];
    if (file != "")
    {
        if (!FileExists(file))
        {
            settings["E_FILEPICKERCTRL_Text_File"] = FixFile("", file);
        }
    }
}

void TextEffect::SelectTextColor(std::string& palette, int index) const
{
    wxString new_palette = "";
    wxArrayString palette_array = wxSplit(palette, ',');
    int found_color = 0;
    for( int i=0; i < palette_array.size(); i++ ) {
        if( palette_array[i].StartsWith("C_CHECKBOX_Palette") ) {
            found_color++;
            if( found_color != index ) {
                continue;
            }
        }
        new_palette += palette_array[i];
        if( i < palette_array.size() - 1 ) {
            new_palette += ",";
        }
    }
    palette = new_palette;
}

void TextEffect::SetDefaultParameters() {
    TextPanel *tp = (TextPanel*)panel;
    if (tp == nullptr) {
        return;
    }

    SetTextValue(tp->TextCtrl_Text, "");
    tp->FilePickerCtrl1->SetFileName(wxFileName(""));
    SetChoiceValue(tp->Choice_Text_Dir, "none");
    tp->Choice_LyricTrack->SetSelection(-1);
    SetSliderValue(tp->Slider_Text_Speed, 10);
    SetChoiceValue(tp->Choice_Text_Effect, "normal");
    SetChoiceValue(tp->Choice_Text_Count, "none");
    SetCheckboxValue(tp, tp->ID_CHECKBOX_TextToCenter, false);
    SetCheckBoxValue(tp->CheckBox_Text_PixelOffsets, false);
    SetSliderValue(tp->Slider_Text_XStart, 0);
    SetSliderValue(tp->Slider_Text_YStart, 0);
    SetSliderValue(tp->Slider_Text_XEnd, 0);
    SetSliderValue(tp->Slider_Text_YEnd, 0);
}

void TextEffect::SetPanelStatus(Model* cls)
{
    TextPanel* tp = static_cast<TextPanel*>(panel);
    if (tp == nullptr)
    {
        return;
    }

    tp->Choice_LyricTrack->Clear();
    if (mSequenceElements == nullptr)
    {
        tp->ValidateWindow();
        return;
    }

    // Load the names of the timing tracks
    tp->Choice_LyricTrack->Append("");
    for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
    {
        Element* e = mSequenceElements->GetElement(i);
        if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING)
        {
            TimingElement* te = dynamic_cast<TimingElement*>(e);
            auto n = e->GetName();
            if (e->GetEffectLayerCount() > 1)
            {
                if (te->HasLyrics(0)) {
                    tp->Choice_LyricTrack->Append(n + " - Phrases");
                }
                if (te->HasLyrics(1)) {
                    tp->Choice_LyricTrack->Append(n + " - Words");
                }
            }
            else
            {
                if (te->HasLyrics(0)) {
                    tp->Choice_LyricTrack->Append(n + " - Phrases");
                }
            }
        }
    }

    // Select the first one
    if (tp->Choice_LyricTrack->GetCount() > 0)
    {
        tp->Choice_LyricTrack->Select(0);
    }

    // Validate the window (includes enabling and disabling controls)
    tp->ValidateWindow();
}

//formatting notes:
//countdown == seconds: put a non-0 value in text line 1 to count down
//countdown == any of the "to date" options: put "Sat, 18 Dec 1999 00:48:30 +0100" in the text line
//countdown = !to date!%fmt: put delimiter + target date + same delimiter + format string with %x markers in it (described down below)

void SetFont(TextDrawingContext *dc, const std::string& FontString, const xlColor &color) {
    const wxFontInfo& fnt = TextDrawingContext::GetTextFont(FontString);
    dc->SetFont(fnt, color);
}

enum TextDirection {
    TEXTDIR_LEFT,
    TEXTDIR_RIGHT,
    TEXTDIR_UP,
    TEXTDIR_DOWN,
    TEXTDIR_NONE,
    TEXTDIR_UPLEFT,
    TEXTDIR_DOWNLEFT,
    TEXTDIR_UPRIGHT,
    TEXTDIR_DOWNRIGHT,
    TEXTDIR_WAVEY_LRUPDOWN,
    TEXTDIR_VECTOR,
    TEXTDIR_WORDFLIP
};

static TextDirection TextEffectDirectionsIndex(const wxString &st) {
    if (st == "left") return TEXTDIR_LEFT;
    if (st == "right") return TEXTDIR_RIGHT;
    if (st == "up") return TEXTDIR_UP;
    if (st == "down") return TEXTDIR_DOWN;
    if (st == "none") return TEXTDIR_NONE;
    if (st == "up-left") return TEXTDIR_UPLEFT;
    if (st == "down-left") return TEXTDIR_DOWNLEFT;
    if (st == "up-right") return TEXTDIR_UPRIGHT;
    if (st == "down-right") return TEXTDIR_DOWNRIGHT;
    if (st == "wavey") return TEXTDIR_WAVEY_LRUPDOWN;
    if (st == "vector")
        return TEXTDIR_VECTOR;
    if (st == "word-flip")
        return TEXTDIR_WORDFLIP;
    return TEXTDIR_NONE;
}
static int TextCountDownIndex(const wxString &st) {
    if (st == "seconds") return 1;
    if (st == "to date 'd h m s'") return 2;
    if (st == "to date 'h:m:s'") return 3;
    if (st == "to date 'm' or 's'") return 4;
    if (st == "to date 's'") return 5;
    if (st == "!to date!%fmt") return 6;
    if (st == "minutes seconds") return 7;
    return 0;
}

static int TextEffectsIndex(const wxString &st) {
    if (st == "vert text up") return 1;
    if (st == "vert text down") return 2;
    if (st == "rotate up 45") return 3;
    if (st == "rotate up 90") return 4;
    if (st == "rotate down 45") return 5;
    if (st == "rotate down 90") return 6;
    return 0;
}

void TextEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {

    // determine if we are rendering an xLights Font
    wxString xl_font = SettingsMap.Get("CHOICE_Text_Font", "Use OS Fonts");
    if( xl_font != "Use OS Fonts" )
    {
        RenderXLText(effect, SettingsMap, buffer);
        return;
    }

    wxString text = ToWXString(SettingsMap["TEXTCTRL_Text"]);
    text.Replace("\\n", "\n");

    wxString filename = SettingsMap["FILEPICKERCTRL_Text_File"];
    wxString lyricTrack = SettingsMap["CHOICE_Text_LyricTrack"];

    if (text.IsEmpty())
    {
        if (FileExists(filename))
        {
            wxTextFile f(filename);
            f.Open();
            int i = 0;
            text = f.GetFirstLine() + "\n";
            while (!f.Eof() && i < MAXTEXTLINES)
            {
                text += f.GetNextLine() + "\n";
                i++;
            }
            if (!text.IsEmpty())
            {
                while (!text.IsEmpty() && text.Last() == '\n' )
                {
                    text = text.BeforeLast('\n');
                }
            }
            f.Close();
        }
        else
        {
            if (!lyricTrack.IsEmpty())
            {
                Element* t = nullptr;
                for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
                {
                    auto lt = lyricTrack.BeforeLast('-');
                    lt = lt.Left(lt.size() - 1);
                    Element* e = mSequenceElements->GetElement(i);
                    if (e->GetEffectLayerCount() > 1 && e->GetType() == ElementType::ELEMENT_TYPE_TIMING && e->GetName() == lt)
                    {
                        t = e;
                        break;
                    }
                }

                if (t != nullptr)
                {
                    long time = buffer.curPeriod * buffer.frameTimeInMs;
                    EffectLayer* el = nullptr;
                    if (lyricTrack.EndsWith(" - Phrases"))
                    {
                        el = t->GetEffectLayer(0);
                    }
                    else
                    {
                        el = t->GetEffectLayer(1);
                    }
                    for (int j = 0; j < el->GetEffectCount(); j++)
                    {
                        Effect* e = el->GetEffect(j);
                        if (e->GetStartTimeMS() <= time && e->GetEndTimeMS() > time)
                        {
                            text = e->GetEffectName();
                            break;
                        }
                    }

                }
            }
        }
    }

    TextDirection dir = TextEffectDirectionsIndex(SettingsMap["CHOICE_Text_Dir"]);
    if (dir == TEXTDIR_WORDFLIP) {
        text = FlipWord(SettingsMap, text, buffer);
    }

    if (!text.IsEmpty()) {

        int starty = wxAtoi(SettingsMap.Get("SLIDER_Text_YStart", "0"));
        int startx = wxAtoi(SettingsMap.Get("SLIDER_Text_XStart", "0"));
        int endy = wxAtoi(SettingsMap.Get("SLIDER_Text_YEnd", "0"));
        int endx = wxAtoi(SettingsMap.Get("SLIDER_Text_XEnd", "0"));
        bool pixelOffsets = wxAtoi(SettingsMap.Get("CHECKBOX_Text_PixelOffsets", "0"));

        wxImage * i = RenderTextLine(buffer,
                       buffer.GetTextDrawingContext(),
                       text,
                       SettingsMap["FONTPICKER_Text_Font"],
                       TextEffectDirectionsIndex(SettingsMap["CHOICE_Text_Dir"]),
                       wxAtoi(SettingsMap["CHECKBOX_TextToCenter"]),
                       TextEffectsIndex(SettingsMap["CHOICE_Text_Effect"]),
                       TextCountDownIndex(SettingsMap["CHOICE_Text_Count"]),
                       wxAtoi(SettingsMap.Get("TEXTCTRL_Text_Speed", "10")),
                       startx, starty, endx, endy, pixelOffsets);
        
        if (i == nullptr) {
            return;
        }
        xlColor c;
        bool ha = i->HasAlpha();
        unsigned char* data = i->GetData();
        unsigned char* alpha = ha ? i->GetAlpha() : nullptr;
        int w = i->GetWidth();
        int h = i->GetHeight();
        int cur = 0;
        int cura = 0;
        for(int y = h - 1; y >= 0; y--)
        {
            for(int x=0; x < w; x++)
            {
                if (ha) {
                    c.Set(data[cur], data[cur + 1], data[cur + 2], alpha[cura++]);
                } else {
                    c.Set(data[cur], data[cur + 1], data[cur + 2]);
                    if (c == xlBLACK) {
                        c.alpha = 0;
                    }
                }
                cur += 3;
                buffer.SetPixel(x, y, c);
            }
        }
    }
}

wxSize GetMultiLineTextExtent(TextDrawingContext *dc,
                              const wxString& text,
                              wxCoord *widthText,
                              wxCoord *heightText,
                              wxCoord *hl)
{
    double widthTextMax = 0, widthLine;
    double heightTextTotal = 0;
    double heightLineDefault = 0, heightLine = 0;

    wxString curLine;
    for ( wxString::const_iterator pc = text.begin(); ; ++pc )
    {
        if ( pc == text.end() || *pc == wxS('\n') )
        {
            if ( curLine.empty() )
            {
                // we can't use GetTextExtent - it will return 0 for both width
                // and height and an empty line should count in height
                // calculation

                // assume that this line has the same height as the previous
                // one
                if ( !heightLineDefault )
                    heightLineDefault = heightLine;

                if ( !heightLineDefault )
                {
                    // but we don't know it yet - choose something reasonable
                    double dummy;
                    dc->GetTextExtent(wxS("W"), &dummy, &heightLineDefault);
                }

                heightTextTotal += heightLineDefault;
            }
            else
            {
                dc->GetTextExtent(curLine, &widthLine, &heightLine);
                if ( widthLine > widthTextMax )
                    widthTextMax = widthLine;
                heightTextTotal += heightLine;
            }

            if ( pc == text.end() )
            {
                break;
            }
            else // '\n'
            {
                curLine.clear();
            }
        }
        else
        {
            curLine += *pc;
        }
    }
    *widthText = widthTextMax;
    *heightText = heightTextTotal;
    *hl = heightLine;
    return wxSize(widthTextMax, heightTextTotal);
}

class CachedTextInfo {
public:
    CachedTextInfo() {}
    CachedTextInfo(const std::string &txt, const std::string font, const std::vector<xlColor> &c, const wxRect &r)
    : text(txt), rect(r), color(c), fontString(font) {}
    ~CachedTextInfo() {}
    
    bool operator==(const CachedTextInfo &i) const {
        return (text == i.text)
            && (fontString == i.fontString)
            && (rect == i.rect)
            && (color == i.color);
    }
    
    std::string text;
    wxRect rect;
    std::vector<xlColor> color;
    std::string fontString;
};

struct CachedTextInfoHasher {
    size_t operator()(const CachedTextInfo& t) const {
        std::size_t h1 = std::hash<std::string>{}(t.text);
        std::size_t h2 = std::hash<std::string>{}(t.fontString);
        h1 ^= (h2 << 1);
        for (auto a : t.color) {
            h1 ^= a.GetRGB() << 3;
        }
        h1 ^= (std::abs(t.rect.x) << 8) + (std::abs(t.rect.y) << 16);
        return h1;
    }
};

class TextRenderCache : public EffectRenderCache {
public:
    TextRenderCache() : timer_countdown(0), synced_textsize(wxSize(0,0)) {};
    virtual ~TextRenderCache() {
        for (const auto& it : textCache) {
            delete it.second;
        }
    };
    int timer_countdown;
    wxSize synced_textsize;
    
    wxImage *GetImage(const CachedTextInfo &inf) {
        return textCache[inf];
    }
    void PutImage(const CachedTextInfo &inf, wxImage *img) {
        textCache[inf] = img;
    }
    
    wxSize GetMultiLineTextExtent(const std::string &font, const wxString &msg) {
        std::pair<std::string, wxString> key(font, msg);
        auto i = textExtentCache.find(key);
        if (i == textExtentCache.end()) {
            return wxSize(-1, -1);
        }
        return i->second;
    }
    void PutMultiLineTextExtent(const std::string &font, const wxString &msg, const wxSize &sz) {
        std::pair<std::string, wxString> key(font, msg);
        textExtentCache[key] = sz;
    }
    
    std::unordered_map<CachedTextInfo, wxImage*, CachedTextInfoHasher> textCache;
    std::map<std::pair<std::string, wxString>, wxSize> textExtentCache;
};

wxSize GetMultiLineTextExtent(TextDrawingContext *dc,
                              const wxString& text,
                              TextRenderCache *cache,
                              const std::string &font,
                              bool &fontSet)
{
    wxSize i = cache->GetMultiLineTextExtent(font, text);
    if (i.x == -1 && i.y == -1) {
        if (!fontSet) {
            dc->Clear();
            SetFont(dc, font, xlWHITE);
            fontSet = true;
        }
        wxCoord x,y,z;
        i = GetMultiLineTextExtent(dc, text, &x, &y, &z);
        cache->PutMultiLineTextExtent(font, text, i);
    }
    return i;
}

void DrawLabel(TextDrawingContext *dc,
               const wxString& text,
               const wxRect& rect,
               int alignment,
               TextRenderCache *cache,
               const std::string &fontString,
               const std::vector<xlColor> colors)
{
    // find the text position
    wxCoord widthText, heightText, heightLine;
    GetMultiLineTextExtent(dc, text, &widthText, &heightText, &heightLine);

    wxCoord width = widthText;
    wxCoord height = heightText;

    wxCoord x, y;
    if ( alignment & wxALIGN_RIGHT )
    {
        x = rect.GetRight() - width;
    }
    else if ( alignment & wxALIGN_CENTRE_HORIZONTAL )
    {
        x = (rect.GetLeft() + rect.GetRight() + 1 - width) / 2;
    }
    else // alignment & wxALIGN_LEFT
    {
        x = rect.GetLeft();
    }

    if ( alignment & wxALIGN_BOTTOM )
    {
        y = rect.GetBottom() - height;
    }
    else if ( alignment & wxALIGN_CENTRE_VERTICAL )
    {
        y = (rect.GetTop() + rect.GetBottom() + 1 - height) / 2;
    }
    else // alignment & wxALIGN_TOP
    {
        y = rect.GetTop();
    }

    // split the string into lines and draw each of them separately
    //
    // NB: while wxDC::DrawText() on some platforms supports drawing multi-line
    //     strings natively, this is not the case for all of them, notably not
    //     wxMSW which uses this function for multi-line texts, so we may only
    //     call DrawText() for single-line strings from here to avoid infinite
    //     recursion.
    wxString curLine;
    int curPos = 0;
    for ( wxString::const_iterator pc = text.begin(); ; ++pc )
    {
        if ( pc == text.end() || *pc == '\n' ) {
            int xRealStart = x; // init it here to avoid compielr warnings
            if ( !curLine.empty() )
            {
                // NB: can't test for !(alignment & wxALIGN_LEFT) because
                //     wxALIGN_LEFT is 0
                if ( alignment & (wxALIGN_RIGHT | wxALIGN_CENTRE_HORIZONTAL) )
                {
                    wxCoord x1,y1,z1;
                    wxCoord widthLine = GetMultiLineTextExtent(dc, curLine, &x1, &y1, &z1).x;

                    if ( alignment & wxALIGN_RIGHT )
                    {
                        xRealStart += width - widthLine;
                    }
                    else // if ( alignment & wxALIGN_CENTRE_HORIZONTAL )
                    {
                        xRealStart += (width - widthLine) / 2;
                    }
                }
                //else: left aligned, nothing to do
                if (colors.size() != 1) {
                    wxArrayDouble d;
                    dc->GetTextExtents(curLine, d);
                    for (int x1 = 0; x1 < curLine.size(); x1++) {
                        wxString c = curLine[x1];
                        if (c != " ") {
                            SetFont(dc, fontString, colors[curPos % colors.size()]);
                            curPos++;
                            double loc = xRealStart;
                            if (x1 != 0) {
                                if (x1 - 1 < d.size()) {
                                    loc += d[x1 - 1];
                                }
                                else {
                                    wxASSERT(false); // this seems to happen when fonts are not good
                                }
                            }
                            dc->DrawText(c, loc, y);
                        }
                    }
                } else {
                    dc->DrawText(curLine, xRealStart, y);
                }
            }

            y += heightLine;

            if ( pc == text.end() )
                break;

            curLine.clear();
        }
        else // not end of line
        {
            curLine += *pc;
        }
    }
}





// dir is 0: move left, 1: move right, 2: up, 3: down, 4: no movement
// Effect is 0: normal, 1: vertical text down, 2: vertical text up,
//           3: timer in seconds, where Line is the starting value in seconds
//           4: timer in days, hours, minute, seconds, where Line is the target date as YYYYMMDD

//these must match list in xLightsMain.cpp: -DJ
#define COUNTDOWN_NONE  0
#define COUNTDOWN_SECONDS  1
#define COUNTDOWN_D_H_M_S  2
#define COUNTDOWN_H_M_S  3
#define COUNTDOWN_M_or_S  4
#define COUNTDOWN_S  5
#define COUNTDOWN_FREEFMT  6
#define COUNTDOWN_MINUTES_SECONDS 7




#define IsGoingLeft(dir)  (((dir) == TEXTDIR_LEFT) || ((dir) == TEXTDIR_UPLEFT) || ((dir) == TEXTDIR_DOWNLEFT))
#define IsGoingRight(dir)  (((dir) == TEXTDIR_RIGHT) || ((dir) == TEXTDIR_UPRIGHT) || ((dir) == TEXTDIR_DOWNRIGHT))
#define IsGoingUp(dir)  (((dir) == TEXTDIR_UP) || ((dir) == TEXTDIR_UPLEFT) || ((dir) == TEXTDIR_UPRIGHT))
#define IsGoingDown(dir)  (((dir) == TEXTDIR_DOWN) || ((dir) == TEXTDIR_DOWNLEFT) || ((dir) == TEXTDIR_DOWNRIGHT))

//provide back-and-forth movement (linear):
//in future this could use exp/log functions for "gravity" type bounces, but for now linear is adequate
#define zigzag(value, range)  \
((((value) / (range)) & 1)? \
(value) % (range): /*increase during odd cycles*/ \
(range) - (value) % (range) - 1) /*descrease during even cycles*/

//#define WANT_DEBUG 99
//#include "djdebug.cpp"

static wxString StripLeft(wxString str, wxString pattern)
{
    while (str.StartsWith(pattern, &str)) {};
    return str;
}

static wxString StripRight(wxString str, wxString pattern)
{
    while (str.EndsWith(pattern, &str)) {};
    return str;
}

TextRenderCache *GetCache(RenderBuffer &buffer, int id) {
    TextRenderCache *cache = (TextRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new TextRenderCache();
        buffer.infoCache[id] = cache;
    }
    return cache;
}

//jwylie - 2016-11-01  -- enhancement: add minute seconds countdown
wxImage *TextEffect::RenderTextLine(RenderBuffer &buffer,
                                    TextDrawingContext* dc,
                                    const wxString& Line_orig,
                                    const std::string &fontString,
                                    int dir,
                                    bool center, int Effect, int Countdown, int tspeed,
                                    int startx, int starty, int endx, int endy,
                                    bool isPixelBased) const
{
    int i;
    wxString Line = Line_orig;
    wxString msg, tempmsg;

    if (Line.IsEmpty()) return nullptr;

    int state = (buffer.curPeriod - buffer.curEffStartPer) * tspeed * buffer.frameTimeInMs / 50;

    FormatCountdown(Countdown, state, Line, buffer, msg, Line_orig);

    ReplaceVaribles(msg, buffer);

    double TextRotation=0.0;
    switch(Effect)
    {
        case 1:
            // vertical text up
            tempmsg=msg;
            msg.clear();
            for(i=0; i<tempmsg.length(); i++)
            {
                msg = msg + tempmsg.GetChar(tempmsg.length()-i-1) + "\n";
            }
            break;
        case 2:
            // vertical text down
            tempmsg=msg;
            msg.clear();
            for(i=0; i<tempmsg.length(); i++)
            {
                msg = msg + tempmsg.GetChar(i) + "\n";
            }
            break;
        default: break;
    }
    
    TextRenderCache *cache = GetCache(buffer, id);
    bool fontSet = false;
    
    wxSize textsize = GetMultiLineTextExtent(dc, msg, cache, fontString, fontSet);
    int extra_left = IsGoingLeft(dir)? textsize.x - GetMultiLineTextExtent(dc, wxString(msg).Trim(false), cache, fontString, fontSet).x: 0; //CAUTION: trim() alters object, so make a copy first
    int extra_right = IsGoingRight(dir)? textsize.x - GetMultiLineTextExtent(dc, wxString(msg).Trim(true), cache, fontString, fontSet).x: 0;
    int extra_down = IsGoingDown(dir)? textsize.y - GetMultiLineTextExtent(dc, StripRight(msg, "\n"), cache, fontString, fontSet).y: 0;
    int extra_up = IsGoingUp(dir)? textsize.y - GetMultiLineTextExtent(dc, StripLeft(msg, "\n"), cache, fontString, fontSet).y: 0;
    //    debug(1, "size %d lstrip %d, rstrip %d, = %d, %d, text %s", dc.GetMultiLineTextExtent(msg).y, dc.GetMultiLineTextExtent(StripLeft(msg, "\n")).y, dc.GetMultiLineTextExtent(StripRight(msg, "\n")).y, extra_down, extra_up, (const char*)StripLeft(msg, "\n"));
    int lineh = GetMultiLineTextExtent(dc, "X", cache, fontString, fontSet).y;
    //    wxString debmsg = msg; debmsg.Replace("\n","\\n", true);
    int xoffset=0;
    int yoffset=0;

    switch(Effect)
    {
        case 3:
            // rotate up 45
            TextRotation=45.0;
            yoffset=int(0.707*double(textsize.GetHeight()));
            i=int(0.707*double(textsize.GetWidth()+textsize.GetHeight()));
            textsize.Set(i,i);
            break;
        case 4:
            // rotate up 90
            TextRotation=90.0;
            textsize.Set(textsize.GetHeight(),textsize.GetWidth());  // swap width & height
            break;
        case 5:
            // rotate down 45
            TextRotation=-45.0;
            xoffset=int(0.707*double(textsize.GetHeight()));
            i=int(0.707*double(textsize.GetWidth()+textsize.GetHeight()));
            textsize.Set(i,i);
            yoffset=i;
            break;
        case 6:
            // rotate down 90
            TextRotation=-90.0;
            xoffset=textsize.GetHeight();
            yoffset=textsize.GetWidth();
            textsize.Set(textsize.GetHeight(),textsize.GetWidth());  // swap width & height
            break;
        default: break;
    }
    //msg.Printf(wxS("w=%d, h=%d"),textsize.GetWidth(),textsize.GetHeight());

    int txtwidth=textsize.GetWidth();
    int totwidth=buffer.BufferWi+txtwidth;
    int totheight=buffer.BufferHt+textsize.GetHeight();

    int OffsetLeft = startx * buffer.BufferWi / 100;
    int OffsetTop = -starty * buffer.BufferHt / 100;
    if (isPixelBased) {
        OffsetLeft = startx;
        OffsetTop =  -starty;
    }

    int xlimit=totwidth*8 + 1;
    int ylimit=totheight*8 + 1;

    if (TextRotation == 0.0)
    {
        wxRect rect(0,0,buffer.BufferWi,buffer.BufferHt);
        switch (dir)
        {
            case TEXTDIR_VECTOR: {
                double position = buffer.GetEffectTimeIntervalPosition(1.0);
                double ex =  endx * buffer.BufferWi / 100;
                double ey = -endy * buffer.BufferHt / 100;
                if (isPixelBased) {
                    ex = endx;
                    ey = -endy;
                }
                ex = OffsetLeft + (ex - OffsetLeft) * position;
                ey = OffsetTop + (ey - OffsetTop) * position;
                rect.Offset(ex, ey);
            }
                break;
            case TEXTDIR_LEFT:
                //           debug(1, "l2r[%d] center? %d, xlim/16 %d, state %d, xofs %d, extra l %d r %d, text %s", idx, center, xlimit/16, state, center? std::max((int)(xlimit/16 - state /*% xlimit*/ /8), -extra_left/2): xlimit/16 - state % xlimit/8, extra_left, extra_right, (const char*)msg);
                // rect.Offset(center? std::max((int)(xlimit/16 - state /*% xlimit*/ /8), -extra_left/2): xlimit/16 - state % xlimit/8, OffsetTop);
                {                
                    int state8 = state / 8;
                    if (state8 < 0) state8 += 32768;
                    if (state > 2000000)
                        state = state + 0;
                    rect.Offset(center ? std::max((int)(xlimit / 16 - state8), -extra_left / 2) : xlimit / 16 - state % xlimit / 8, OffsetTop);
                }
                break; // left, optionally stop at center
            case TEXTDIR_RIGHT:
                rect.Offset(center? std::min((int)(state /*% xlimit*/ /8 - xlimit/16), extra_right/2): state % xlimit/8 - xlimit/16, OffsetTop);
                break; // right, optionally stop at center
            case TEXTDIR_UP:
                rect.Offset(OffsetLeft, center? std::max((int)(ylimit/16 - state /*% ylimit*/ /8), lineh/2 - extra_up/2): ylimit/16 - state % ylimit/8);
                break; // up, optionally stop at center
            case TEXTDIR_DOWN:
                //            debug(1, "t2b[%d] center? %d, totht %d, ylimit %d, extra u %d d %d, lineh %d, text %s => yofs min(%d - %d, %d + %d)", idx, center, totheight, ylimit, extra_up, extra_down, lineh, (const char*)debmsg, state /*% ylimit*/ /8, ylimit/16, -lineh/2, extra_down/2);
                rect.Offset(OffsetLeft, center? std::min((int)(state /*% ylimit*/ /8 - ylimit/16), -lineh/2 + extra_down/2): state % ylimit/8 - ylimit/16);
                break; // down, optionally stop at center
            case TEXTDIR_UPLEFT:
                rect.Offset(center? std::max((int)(xlimit/16 - state /*% xlimit*/ /8), -extra_left/2): xlimit/16 - state % xlimit/8, center? std::max((int)(ylimit/16 - state /*% ylimit*/ /8), lineh/2 - extra_up/2): ylimit/16 - state % ylimit/8);
                break; // up-left, optionally stop at center
            case TEXTDIR_DOWNLEFT:
                rect.Offset(center? std::max((int)(xlimit/16 - state /*% xlimit*/ /8), -extra_left/2): xlimit/16 - state % xlimit/8, center? std::min((int)(state /*% ylimit*/ /8 - ylimit/16), -lineh/2 + extra_down/2): state % ylimit/8 - ylimit/16);
                break; // down-left, optionally stop at center
            case TEXTDIR_UPRIGHT:
                rect.Offset(center? std::min((int)(state /*% xlimit*/ /8 - xlimit/16), extra_right/2): state % xlimit/8 - xlimit/16, center? std::max((int)(ylimit/16 - state /*% ylimit*/ /8), lineh/2 - extra_up/2): ylimit/16 - state % ylimit/8);
                break; // up-right, optionally stop at center
            case TEXTDIR_DOWNRIGHT:
                rect.Offset(center? std::min((int)(state /*% xlimit*/ /8 - xlimit/16), extra_right/2): state % xlimit/8 - xlimit/16, center? std::min((int)(state /*% ylimit*/ /8 - ylimit/16), -lineh/2 + extra_down/2): state % ylimit/8 - ylimit/16);
                break; // down-right, optionally stop at center
            case TEXTDIR_WAVEY_LRUPDOWN:
                if (center) //does to-center make sense with this one?
                    rect.Offset(std::min((int)(state /*% xlimit*/ /8 - xlimit/16), extra_right/2), std::max((int)zigzag(state/4, totheight)/2 - totheight/4, -extra_left/2));
                else
                    rect.Offset(xlimit/16 - state % xlimit/8, zigzag(state/4, totheight)/2 - totheight/4);
                break; // left-to-right, wavey up-down 1/2 height (too bouncy if full height is used), slow down up/down motion (too fast unless scaled)
            case TEXTDIR_WORDFLIP:
            case TEXTDIR_NONE: //fall thru to default
            default:
                //rect.Offset(0, OffsetTop);
                rect.Offset(OffsetLeft, OffsetTop);
                break; // static
        }
        int num_colors = buffer.GetColorCount();
        std::vector<xlColor> colors;
        for (int x = 0; x < num_colors; x++) {
            colors.push_back(buffer.GetPalette().GetColor(x));
        }
        if (colors.size() == 0) {
            colors.push_back(xlWHITE);
        }
        CachedTextInfo inf(msg.ToStdString(), fontString, colors, rect);
        wxImage *img = GetCache(buffer,id)->GetImage(inf);
        if (img == nullptr) {
            dc->Clear();
            SetFont(dc, fontString, colors[0]);
            DrawLabel(dc, msg, rect, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, GetCache(buffer,id), fontString, colors);
            wxImage *i2 = dc->FlushAndGetImage();
            img = new wxImage(i2->GetSize());
            *img = i2->Copy();
            GetCache(buffer,id)->PutImage(inf, img);
        }
        return img;
    }
    
    xlColor c;
    buffer.palette.GetColor(0,c);
    dc->Clear();
    SetFont(dc,fontString,c);
    switch (dir) {
        case TEXTDIR_VECTOR: {
            double position = buffer.GetEffectTimeIntervalPosition(1.0);
            double ex = endx * buffer.BufferWi / 100;
            double ey = -endy * buffer.BufferHt / 100;
            if (isPixelBased) {
                ex = endx;
                ey = -endy;
            }
            ex = OffsetLeft + (ex - OffsetLeft) * position;
            ey = OffsetTop + (ey - OffsetTop) * position;
            if (TextRotation > 50) {
                dc->DrawText(msg, buffer.BufferWi / 2 + ex - txtwidth / 2, buffer.BufferHt / 2 + ey + textsize.GetHeight() / 2, TextRotation);
            } else if (TextRotation > 0) {
                dc->DrawText(msg, buffer.BufferWi / 2 + ex - txtwidth / 2, buffer.BufferHt / 2 + ey + yoffset * 2, TextRotation);
            } else if (TextRotation < -50) {
                dc->DrawText(msg, buffer.BufferWi / 2 + ex + txtwidth / 2, buffer.BufferHt / 2 + ey - textsize.GetHeight() / 2, TextRotation);
            } else {
                dc->DrawText(msg, buffer.BufferWi / 2 + ex - txtwidth / 2 + xoffset, buffer.BufferHt / 2 + ey - textsize.GetHeight() / 2, TextRotation);
            }
        }
            break;
        case TEXTDIR_LEFT:
            dc->DrawText(msg, buffer.BufferWi - state % xlimit/8 + xoffset, OffsetTop, TextRotation);
            break; // left
        case TEXTDIR_RIGHT:
            dc->DrawText(msg, state % xlimit/8 - txtwidth + xoffset, OffsetTop, TextRotation);
            break; // right
        case TEXTDIR_UP:
            dc->DrawText(msg, OffsetLeft, totheight - state % ylimit/8 - yoffset, TextRotation);
            break; // up
        case TEXTDIR_DOWN:
            dc->DrawText(msg, OffsetLeft, state % ylimit/8 - yoffset, TextRotation);
            break; // down
        case TEXTDIR_UPLEFT:
            dc->DrawText(msg, buffer.BufferWi - state % xlimit/8 + xoffset, totheight - state % ylimit/8 - yoffset, TextRotation);
            break; // up-left
        case TEXTDIR_DOWNLEFT:
            dc->DrawText(msg, buffer.BufferWi - state % xlimit/8 + xoffset, state % ylimit/8 - yoffset, TextRotation);
            break; // down-left
        case TEXTDIR_UPRIGHT:
            dc->DrawText(msg, state % xlimit/8 - txtwidth + xoffset, totheight - state % ylimit/8 - yoffset, TextRotation);
            break; // up-right
        case TEXTDIR_DOWNRIGHT:
            dc->DrawText(msg, state % xlimit/8 - txtwidth + xoffset, state % ylimit/8 - yoffset, TextRotation);
            break; // down-right
        default:
            dc->DrawText(msg, 0, OffsetTop, TextRotation);
            break; // static
    }

    return buffer.GetTextDrawingContext()->FlushAndGetImage();
}

void TextEffect::FormatCountdown(int Countdown, int state, wxString& Line, RenderBuffer &buffer, wxString& msg, wxString Line_orig) const
{
    long longsecs;
    int framesPerSec = 1000 / buffer.frameTimeInMs;
    int minutes,seconds;

    wxDateTime dt;
    wxTimeSpan ts;
    wxString::const_iterator end;
    wxString fmt = Line_orig;
    wxString prepend = Line_orig;   //for prepended/appended text to countdown
    wxString append = Line_orig;   //for prepended/appended text to countdown
    wxString timePart = Line_orig;

    switch (Countdown)
    {
    case COUNTDOWN_SECONDS:
            {
                // countdown seconds
                if (state == 0)
                {
                    long tempLong;
                    if (!Line.ToLong(&tempLong)) tempLong = 0;
                    GetCache(buffer, id)->timer_countdown = buffer.curPeriod + tempLong*framesPerSec + framesPerSec - 1;  // capture 0 period
                }
                seconds = (GetCache(buffer, id)->timer_countdown - buffer.curPeriod) / framesPerSec;
                if (seconds < 0) seconds = 0;
                msg = wxString::Format("%i", seconds);
            }
            break;
//jwylie - 2016-11-01  -- enhancement: add minute seconds countdown
        case COUNTDOWN_MINUTES_SECONDS:
        {
            if (timePart.Find('/') != -1)
            {
                timePart = timePart.AfterFirst('/').BeforeLast('/');
                prepend = prepend.BeforeFirst('/');
                append = append.AfterLast('/');
            }
            else
            {
                append = "";
                prepend = "";
            }
            wxArrayString minSec = wxSplit(timePart, ':');
            if (minSec.size() == 1)
            {
                seconds = wxAtoi(minSec[0]);
            }
            else if (minSec.size() == 2)
            {
                minutes = wxAtoi(minSec[0]);
                seconds = (minutes * 60) + wxAtoi(minSec[1]);
                //MessageBoxA(NULL, "total seconds: " + wxString::Format("%i", seconds), "message", MB_ICONINFORMATION | MB_OK | MB_DEFBUTTON2);
            }
            else //invalid format
            {
                msg = _T("Invalid Format");
                break;
            }
            if (state == 0)
                GetCache(buffer, id)->timer_countdown = buffer.curPeriod + seconds*framesPerSec + framesPerSec - 1;

            else
                seconds = (GetCache(buffer, id)->timer_countdown - buffer.curPeriod) / framesPerSec;

            minutes = (seconds / 60);
            seconds = seconds - (minutes * 60);

            if (seconds < 0)
                seconds = 0;

            wxString tempSeconds = wxString::Format("%i", seconds);

            if (tempSeconds.Len() == 1)
                tempSeconds = tempSeconds.Pad(1, '0', false);

            msg = prepend + ' ' + wxString::Format("%i", minutes) + " : " + tempSeconds + append;
            }
           break;

        case COUNTDOWN_FREEFMT: //free format text with embedded formatting chars -DJ
#if 0

            Aug 14,2015 <scm>
            Sample datestrings that are valid for the countdown timer
                Wed, 02 Oct 2015 15:00:00 +0200
                Wed, 02 Oct 2015 15:00:00 EST

                Note, dates must be in the future, any date in the past will show as "Invalid Date" when converted



                clear(

                )
                wxTimeSpan format chars are described at:
                http://docs.wxwidgets.org/trunk/classwx_time_span.html
                The following format specifiers are allowed after %:
                ïH - Number of Hours
                ïM - Number of Minutes
                ïS - Number of Seconds
                ïl - Number of Milliseconds
                ïD - Number of Days
                ïE - Number of Weeks
                ï% - The percent character

                //Format Characters are described at: http://www.cplusplus.com/reference/ctime/strftime/
                TIME FORMAT CHARACTERS:
                %a Abbreviated weekday name eg. Thu
                %A Full weekday name eg. Thursday
                %b Abbreviated month name eg. Aug
                %B Full month name eg. August
                %c Date and time representation eg. Thu Aug 23 14:55:02 2001
                %d Day of the month (01-31) eg. 23
                %H Hour in 24h format (00-23) eg. 14
                %I Hour in 12h format (01-12) eg. 02
                %j Day of the year (001-366) eg. 235
                %m Month as a decimal number (01-12) eg. 08
                %M Minute (00-59) eg. 55
                %p AM or PM designation eg. PM
                %S Second (00-61) eg. 02
                %U Week number with the first Sunday as the first day of week one (00-53) eg. 33
                %w Weekday as a decimal number with Sunday as 0 (0-6) eg. 4
                %W Week number with the first Monday as the first day of week one (00-53) eg. 34
                %x Date representation eg. 08/23/01
                %X Time representation eg. 14:55:02
                %y Year, last two digits (00-99) eg. 01
                %Y Year eg. 2001
                %Z Timezone name or abbreviation CDT
                %% A % sign eg. %
#endif // 0
                //time_local = time.Format(wxT("%T"), wxDateTime::A_EST).c_str();
                if (Line.size() >= 4)
                {
                    wxChar delim = Line[0]; //use first char as date delimiter; date and format string follows that, separated by delimiter
                    Line.Remove(0, 1); //.erase(Line.begin(), Line.begin() + 1); //remove leading delim
                    //            Line.RemoveLast(); //remove delimiter
                    fmt = Line.After(delim);
                    Line.Truncate(Line.find(delim)); //remove fmt string, leaving only count down date
                }
                else fmt.Empty();
            //CAUTION: fall thru here
        case COUNTDOWN_D_H_M_S:
        case COUNTDOWN_H_M_S:
        case COUNTDOWN_M_or_S:
        case COUNTDOWN_S:
        {
            // countdown to date
            if (state%framesPerSec == 0)   //1x/sec
            {
                //            if ( dt.ParseDateTime(Line, &end) ) { //broken, force RFC822 for now -DJ
                if (dt.ParseRfc822Date(Line, &end))
                {
                    // dt is (at least partially) valid, so calc # of seconds until then
                    ts = dt.Subtract(wxDateTime::Now());
                    wxLongLong ll = ts.GetSeconds();
                    if (ll > LONG_MAX) ll = LONG_MAX;
                    if (ll < 0) ll = 0;
                    longsecs = ll.ToLong();
                }
                else
                {
                    // invalid date/time
                    longsecs = 0;
                }
                GetCache(buffer, id)->timer_countdown = longsecs;
            }
            else
            {
                longsecs = GetCache(buffer, id)->timer_countdown;
                ts = wxTimeSpan(0, 0, longsecs, 0); //reconstruct wxTimeSpan so we can call .Format method -DJ
            }
            if (!longsecs)
            {
                msg = _T("invalid date");    //show when invalid -DJ
                break;
            }
            int days = longsecs / 60 / 60 / 24;
            int hours = (longsecs / 60 / 60) % 24;
            minutes = (longsecs / 60) % 60;
            seconds = longsecs % 60;
            if (Countdown == COUNTDOWN_D_H_M_S)
                msg = wxString::Format("%dd %dh %dm %ds", days, hours, minutes, seconds);
            else if (Countdown == COUNTDOWN_H_M_S)
                msg = wxString::Format("%d : %d : %d", hours, minutes, seconds);
            else if (Countdown == COUNTDOWN_S)
                msg = wxString::Format("%d", 60 * 60 * hours + 60 * minutes + seconds);
            else if (Countdown == COUNTDOWN_FREEFMT)
                //            msg = _T("%%") + Line + _T("%%") + fmt + _T("%%");
                if (fmt == "" || (fmt.EndsWith("%") && !fmt.EndsWith("%%")))
                {
                    msg = _T("invalid format");
                }
                else
                {
                    msg = ts.Format(fmt); //dt.Format(fmt)
                }
            else //if (Countdown == COUNTDOWN_M_or_S)
                if (60 * hours + minutes < 5) //COUNTDOWN_M_or_S: show seconds
                    msg = wxString::Format("%d", 60 * 60 * hours + 60 * minutes + seconds);
                else //COUNTDOWN_M_or_S: show minutes
                    msg = wxString::Format("%d m", 60 * hours + minutes);
        }
            break;
        default:
            msg=Line;
            msg.Replace("\\n", "\n", true); //allow vertical spacing (mainly for up/down) -DJ
            break;
    }
}

#define msgReplace(a, b, c) \
    do                      \
    {                    \
       if (a.Contains(b)) { \
           a.Replace(b, (c)); \
       } \
    } while (0)

void TextEffect::ReplaceVaribles(wxString& msg, RenderBuffer& buffer) const
{
    msgReplace(msg, "${TITLE}", buffer.GetXmlHeaderInfo(HEADER_INFO_TYPES::SONG));
    msgReplace(msg, "${SONG}", buffer.GetXmlHeaderInfo(HEADER_INFO_TYPES::SONG));
    msgReplace(msg, "${ARTIST}", buffer.GetXmlHeaderInfo(HEADER_INFO_TYPES::ARTIST));
    msgReplace(msg, "${ALBUM}", buffer.GetXmlHeaderInfo(HEADER_INFO_TYPES::ALBUM));
    if (buffer.GetMedia() != nullptr) {
        msgReplace(msg, "${FILENAME}", buffer.GetMedia()->FileName());
    }
    msgReplace(msg, "${AUTHOR}", buffer.GetXmlHeaderInfo(HEADER_INFO_TYPES::AUTHOR));
    msgReplace(msg, "${AUTHOREMAIL}", buffer.GetXmlHeaderInfo(HEADER_INFO_TYPES::AUTHOR_EMAIL));
    msgReplace(msg, "${COMMENT}", buffer.GetXmlHeaderInfo(HEADER_INFO_TYPES::COMMENT));
    msgReplace(msg, "${URL}", buffer.GetXmlHeaderInfo(HEADER_INFO_TYPES::URL));
    msgReplace(msg, "${WEBSITE}", buffer.GetXmlHeaderInfo(HEADER_INFO_TYPES::WEBSITE));

    if (msg.Contains("${UPPER}")) {
        msg.Replace("${UPPER}", "");
        msg = msg.Upper();
    }
    if (msg.Contains("${LOWER}")) {
        msg.Replace("${LOWER}", "");
        msg = msg.Lower();
    }
}

std::vector<std::string> TextEffect::WordSplit(const std::string& text) const
{
    std::vector<std::string> res;

    std::string word;
    for (auto c : text) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (word != "") {
                res.push_back(word);
                word = "";
            }
        }
        else {
            word += c;
        }
    }

    if (word != "") {
        res.push_back(word);
    }

    return res;
}

std::string TextEffect::FlipWord(const SettingsMap& settings, const std::string& text, RenderBuffer& buffer) const
{
    auto words = WordSplit(text);

    if (words.size() > 1) {
        // we need to adjust the text
        int tspeed = wxAtoi(settings.Get("TEXTCTRL_Text_Speed", "10")); // 0 to 50

        // zero means just show the first word ... never advance
        // one means go through words once

        float msPerWord = 0;
        if (tspeed != 0) {
            msPerWord = ((buffer.curEffEndPer - buffer.curEffStartPer + 1) * buffer.frameTimeInMs) / (words.size() * tspeed);
        }
        int word = 0;        
        if (msPerWord != 0) {
            word = ((float)(buffer.curPeriod - buffer.curEffStartPer) * (float)buffer.frameTimeInMs) / msPerWord;
        }

        word = word % words.size();
        return words[word];
    }

    return text;
}

void TextEffect::RenderXLText(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer)
{
    xlColor c;
    int num_colors = buffer.palette.Size();
    buffer.palette.GetColor(0, c);

    int starty = wxAtoi(settings.Get("SLIDER_Text_YStart", "0"));
    int startx = wxAtoi(settings.Get("SLIDER_Text_XStart", "0"));
    int endy = wxAtoi(settings.Get("SLIDER_Text_YEnd", "0"));
    int endx = wxAtoi(settings.Get("SLIDER_Text_XEnd", "0"));
    bool pixelOffsets = wxAtoi(settings.Get("CHECKBOX_Text_PixelOffsets", "0"));

    int OffsetLeft = startx * buffer.BufferWi / 100;
    int OffsetTop = -starty * buffer.BufferHt / 100;
    if (pixelOffsets) {
        OffsetLeft = startx;
        OffsetTop = -starty;
    }

    font_mgr.init();  // make sure font class is initialized
    wxString xl_font = settings["CHOICE_Text_Font"];
    xlFont* font = font_mgr.get_font(xl_font);
    wxBitmap* bmp = font->get_bitmap();
    wxImage image = bmp->ConvertToImage();
    int char_width = font->GetWidth();
    int char_height = font->GetHeight();

    wxString text = ToWXString(settings["TEXTCTRL_Text"]);
    text.Replace("\\n", "\n");

    wxString filename = settings["FILEPICKERCTRL_Text_File"];
    wxString lyricTrack = settings["CHOICE_Text_LyricTrack"];

    if (text == "") {
        if (FileExists(filename)) {
            wxTextFile f(filename);
            f.Open();
            int i = 0;
            text = f.GetFirstLine() + "\n";
            while (!f.Eof() && i < MAXTEXTLINES) {
                text += f.GetNextLine() + "\n";
                i++;
            }
            if (text != "") {
                while (text.Last() == '\n') {
                    text = text.BeforeLast('\n');
                }
            }
            f.Close();
        }
        else {
            if (lyricTrack != "") {
                Element* t = nullptr;
                for (int i = 0; i < mSequenceElements->GetElementCount(); i++) {
                    auto lt = lyricTrack.BeforeLast('-');
                    lt = lt.Left(lt.size() - 1);
                    Element* e = mSequenceElements->GetElement(i);
                    if (e->GetEffectLayerCount() > 1 && e->GetType() == ElementType::ELEMENT_TYPE_TIMING && e->GetName() == lt) {
                        t = e;
                        break;
                    }
                }

                if (t != nullptr) {
                    long time = buffer.curPeriod * buffer.frameTimeInMs;
                    EffectLayer* el = nullptr;
                    if (lyricTrack.EndsWith(" - Phrases")) {
                        el = t->GetEffectLayer(0);
                    }
                    else {
                        el = t->GetEffectLayer(1);
                    }
                    for (int j = 0; j < el->GetEffectCount(); j++) {
                        Effect* e = el->GetEffect(j);
                        if (e->GetStartTimeMS() <= time && e->GetEndTimeMS() > time) {
                            text = e->GetEffectName();
                            break;
                        }
                    }

                }
            }
        }
    }

    wxString msg = text;
    int Countdown = TextCountDownIndex(settings["CHOICE_Text_Count"]);
    if (Countdown > 0) {
        int tspeed = wxAtoi(settings.Get("TEXTCTRL_Text_Speed", "10"));
        int state = (buffer.curPeriod - buffer.curEffStartPer) * tspeed * buffer.frameTimeInMs / 50;
        wxString Line = text;
        FormatCountdown(Countdown, state, Line, buffer, msg, text);
        msg.Replace(" : ", ":");
    }
    ReplaceVaribles(msg, buffer);
    text = msg;
    int text_length = font_mgr.get_length(font, text);

    int text_effect = TextEffectsIndex(settings["CHOICE_Text_Effect"]);
    bool vertical = false;
    bool rotate_90 = false;
    bool up = false;
    if (text_effect == 1 || text_effect == 2) {
        vertical = true;
        if (text_effect == 1) {
            up = true;
        }
    }
    else if (text_effect == 4 || text_effect == 6) {
        rotate_90 = true;
        if (text_effect == 4) {
            up = true;
        }
    }

    TextDirection dir = TextEffectDirectionsIndex(settings["CHOICE_Text_Dir"]);
    if (dir == TEXTDIR_WORDFLIP) {
        text = FlipWord(settings, text, buffer);
    }

    int PreOffsetLeft = OffsetLeft;
    int PreOffsetTop = OffsetTop;

    AddMotions(OffsetLeft, OffsetTop, settings, buffer, text.size(), endx, endy, pixelOffsets, PreOffsetLeft, PreOffsetTop, text_length, char_width, char_height, vertical, rotate_90);
    if (rotate_90) {
        OffsetLeft += buffer.BufferWi / 2 - font->GetCapsHeight() / 2;
        if (up) {
            OffsetTop += buffer.BufferHt / 2 + text_length / 2;
        }
        else {
            OffsetTop += buffer.BufferHt / 2 - text_length / 2;
        }
    }
    else if (vertical) {
        OffsetLeft += buffer.BufferWi / 2 - char_width / 2;
        if (up) {
            OffsetTop += buffer.BufferHt / 2 + (char_height * text.length()) / 2;
        }
        else {
            OffsetTop += buffer.BufferHt / 2 - (char_height * text.length()) / 2;
        }
    }
    else {
        OffsetLeft += buffer.BufferWi / 2 - text_length / 2;
        OffsetTop += buffer.BufferHt / 2 - font->GetCapsHeight() / 2;
    }

    auto startOffsetLeft = OffsetLeft;

    if (text != "") {
        int space_offset = 0;
        for (int i = 0; i < text.length(); i++) {

            if (text[i] == '\n')
            {
                OffsetLeft = startOffsetLeft;
                OffsetTop += font->GetHeight() + 1;
            } else {
                buffer.palette.GetColor((i - space_offset) % num_colors, c);
                if (text[i] == ' ') {
                    space_offset++;
                }
                char ascii = text[i];
                int x_start_corner = (ascii % 8) * (char_width + 1) + 1;
                int y_start_corner = (ascii / 8) * (char_height + 1) + 1;

                int actual_width = font->GetCharWidth(ascii);
                wxASSERT(actual_width > 0);
                if (rotate_90 && up) {
                    OffsetTop -= actual_width;
                }
                for (int w = 0; w < actual_width; w++) {
                    int x_pos = x_start_corner + w;
                    for (int y_pos = y_start_corner; y_pos < y_start_corner + char_height; y_pos++) {
                        if (x_pos >= 0 && x_pos < image.GetWidth() && y_pos >= 0 && y_pos < image.GetHeight()) {
                            int red = image.GetRed(x_pos, y_pos);
                            int green = image.GetGreen(x_pos, y_pos);
                            int blue = image.GetBlue(x_pos, y_pos);
                            if (red == 255 && green == 255 && blue == 255) {
                                if (rotate_90) {
                                    if (up) {
                                        buffer.SetPixel(y_pos - y_start_corner + OffsetLeft, (buffer.BufferHt - 1) - (actual_width - 1 - x_pos + x_start_corner + OffsetTop), c, false);
                                    } else {
                                        buffer.SetPixel(char_height - 1 - y_pos + y_start_corner + OffsetLeft, (buffer.BufferHt - 1) - (x_pos - x_start_corner + OffsetTop), c, false);
                                    }
                                } else {
                                    buffer.SetPixel(x_pos - x_start_corner + OffsetLeft, buffer.BufferHt - (y_pos - y_start_corner + OffsetTop) - 1, c, false);
                                }
                            }
                        }
                    }
                }
                if (vertical) {
                    if (up) {
                        OffsetTop -= char_height + 1;
                    } else {
                        OffsetTop += char_height + 1;
                    }
                } else if (rotate_90 && !up) {
                    OffsetTop += actual_width;
                } else if (!rotate_90) {
                    OffsetLeft += actual_width;
                }
            }
        }
    }
}

void TextEffect::AddMotions(int& OffsetLeft, int& OffsetTop, const SettingsMap& settings, RenderBuffer &buffer,
    int txtLen, int endx, int endy, bool pixelOffsets, int PreOffsetLeft, int PreOffsetTop, int text_len, int char_width, int char_height, bool vertical, bool rotate_90) const
{
    int tspeed = wxAtoi(settings.Get("TEXTCTRL_Text_Speed", "10"));
    int state = (buffer.curPeriod - buffer.curEffStartPer) * tspeed * buffer.frameTimeInMs / 50;

    int txtwidth = text_len;
    int txtheight = char_height;
    int totwidth = buffer.BufferWi + text_len;
    int totheight = buffer.BufferHt + char_height;

    if (vertical)         {
        totwidth = buffer.BufferWi + char_width;
        totheight = buffer.BufferHt + txtLen * char_height;
        txtwidth = char_width;
        txtheight = txtLen * char_height;
    } else if (rotate_90)         {
        totwidth = buffer.BufferWi + char_height;
        totheight = buffer.BufferHt + text_len;
        txtwidth = char_height;
        txtheight = text_len;
    }

    int xlimit = totwidth * 8 + 1;
    int ylimit = totheight * 8 + 1;

    TextDirection dir = TextEffectDirectionsIndex(settings["CHOICE_Text_Dir"]);
    //int center = wxAtoi(settings["CHECKBOX_TextToCenter"]);  // not implemented yet - hoping to switch to value curves anyways

    switch (dir) {
    case TEXTDIR_VECTOR:
    {
        double position = buffer.GetEffectTimeIntervalPosition(1.0);
        double ex = endx * buffer.BufferWi / 100;
        double ey = -endy * buffer.BufferHt / 100;
        if (pixelOffsets) {
            ex = endx;
            ey = -endy;
        }
        OffsetLeft += (ex - OffsetLeft) * position;
        OffsetTop += (ey - OffsetTop) * position;
    }
    break;
    case TEXTDIR_LEFT:
        OffsetLeft = buffer.BufferWi - state % (xlimit + buffer.BufferWi) / 8 + PreOffsetLeft + txtwidth / 2;
        break; // left
    case TEXTDIR_RIGHT:
        OffsetLeft = state % xlimit / 8 - txtwidth + PreOffsetLeft;
        break; // right
    case TEXTDIR_UP:
        OffsetTop = buffer.BufferHt - state % ylimit / 8 - PreOffsetTop;
        break; // up
    case TEXTDIR_DOWN:
        OffsetTop = state % ylimit / 8 - txtheight - PreOffsetTop;
        break; // down
    case TEXTDIR_UPLEFT:
        OffsetLeft = buffer.BufferWi - state % xlimit / 8 + PreOffsetLeft;
        OffsetTop = buffer.BufferHt - state % ylimit / 8 - PreOffsetTop;
        break; // up-left
    case TEXTDIR_DOWNLEFT:
        OffsetLeft = buffer.BufferWi - state % xlimit / 8 + PreOffsetLeft;
        OffsetTop = state % ylimit / 8 - txtheight - PreOffsetTop;
        break; // down-left
    case TEXTDIR_UPRIGHT:
        OffsetLeft = state % xlimit / 8 - txtwidth + PreOffsetLeft;
        OffsetTop = buffer.BufferHt - state % ylimit / 8 - PreOffsetTop;
        break; // up-right
    case TEXTDIR_DOWNRIGHT:
        OffsetLeft = state % xlimit / 8 - txtwidth + PreOffsetLeft;
        OffsetTop = state % ylimit / 8 - txtheight - PreOffsetTop;
        break; // down-right
    case TEXTDIR_WAVEY_LRUPDOWN:
        OffsetLeft = xlimit / 16 - state % xlimit / 8;
        OffsetTop = zigzag(state / 4, totheight) / 2 - totheight / 4;
        break; // left-to-right, wavey up-down 1/2 height (too bouncy if full height is used), slow down up/down motion (too fast unless scaled)
    default:
        break;
    }
}
