/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "TextEffect.h"
#include "AudioManager.h"

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <fstream>
#include <format>
#include <mutex>
#include <array>
#include <sstream>
#include <unordered_map>

#include "../render/Effect.h"
#include "../render/Element.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "../models/Model.h"
#include "UtilFunctions.h"
#include "../render/FontManager.h"
#include "../render/RenderContext.h"
#include "../render/SequenceElements.h"
#include "../ExternalHooks.h"
#include "../render/SequenceFile.h"
#include "../render/SequenceMedia.h"
#include "../utils/string_utils.h"
#include "../utils/xlRect.h"
#include "../utils/xlSize.h"

#include "../../include/text-16.xpm"
#include "../../include/text-24.xpm"
#include "../../include/text-32.xpm"
#include "../../include/text-48.xpm"
#include "../../include/text-64.xpm"
#include <log.h>

#define MAXTEXTLINES 100

// Local alignment constants (matching wx values) so we don't depend on wx headers
constexpr int TEXT_ALIGN_CENTER_HORIZONTAL = 0x0100;
constexpr int TEXT_ALIGN_RIGHT = 0x0200;
constexpr int TEXT_ALIGN_BOTTOM = 0x0400;
constexpr int TEXT_ALIGN_CENTER_VERTICAL = 0x0800;

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
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    std::string textFilename = settings.Get("E_FILEPICKERCTRL_Text_File", "");
    std::string text = settings.Get("E_TEXTCTRL_Text", "");
    std::string lyricTrack = settings.Get("E_CHOICE_Text_LyricTrack", "");

    if (text == "" && textFilename == "" && lyricTrack == "") {
        res.push_back(std::format("    ERR: Text effect has no actual text. Model '{}', Start {}", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    } else if (textFilename != "") {
        auto& mm = eff->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetSequenceMedia();
        auto entry = mm.GetTextFile(textFilename);
        entry->MarkIsUsed();
        if (entry->GetContent().empty() && !entry->IsEmbedded()) {
            res.push_back(std::format("    ERR: Text effect cant find file '{}'. Model '{}', Start {}", textFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
        } else if (!entry->IsEmbedded() && !IsFileInShowDir(std::string(), textFilename)) {
            res.push_back(std::format("    WARN: Text effect file '{}' not under show directory. Model '{}', Start {}", textFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
        }
    }

    if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
        res.push_back(std::format("    WARN: Text effect generally does not work well on a model group. Model '{}', Start {}", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    }
    return res;
}

std::list<std::string> TextEffect::GetFileReferences(Model* model, const SettingsMap &SettingsMap) const
{
    std::list<std::string> res;    
    std::string textFilename = SettingsMap["E_FILEPICKERCTRL_Text_File"];
    if (textFilename != "")
    {
        res.push_back(textFilename);
    }
    return res;
}

bool TextEffect::CleanupFileLocations(RenderContext* ctx, SettingsMap &SettingsMap)
{
    bool rc = false;
    std::string file = SettingsMap["E_FILEPICKERCTRL_Text_File"];
    if (FileExists(file))
    {
        if (!ctx->IsInShowFolder(file))
        {
            SettingsMap["E_FILEPICKERCTRL_Text_File"] = ctx->MoveToShowFolder(file, std::string(1, std::filesystem::path::preferred_separator));
            rc = true;
        }
    }

    return rc;
}

bool TextEffect::SupportsRenderCache(const SettingsMap& settings) const
{
    // we dont want to use render cache if text is coming from a file as the file might have changed
    if (settings["TEXTCTRL_Text"] == "" && FileExists(settings["FILEPICKERCTRL_Text_File"]))
        return false;
    // we dont want to use render cache if text is coming from lyric track, if you have text then it overrides the lyric track
    if (settings["TEXTCTRL_Text"] == ""  && settings["CHOICE_Text_LyricTrack"] != "")
        return false;
    return true;
}

void TextEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults)
{
    if (RenderableEffect::needToAdjustSettings(version)) {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }

    SettingsMap &settings = effect->GetSettings();

    // Resolve broken paths first, then convert to relative for portability
    std::string file = settings["E_FILEPICKERCTRL_Text_File"];
    if (!file.empty() && !FileExists(file)) {
        std::string fixed = FixFile("", file);
        if (!fixed.empty() && fixed != file) {
            settings["E_FILEPICKERCTRL_Text_File"] = fixed;
            file = fixed;
        }
    }
    if (!file.empty()) {
        if (std::filesystem::path(file).is_absolute()) {
            if (!FileExists(file, false)) {
                std::string fixed = FixFile("", file);
                std::string rel = MakeRelativeFile(fixed);
                settings["E_FILEPICKERCTRL_Text_File"] = rel.empty() ? fixed : rel;
            } else {
                std::string rel = MakeRelativeFile(file);
                if (!rel.empty())
                    settings["E_FILEPICKERCTRL_Text_File"] = rel;
            }
        }
        // Register with SequenceMedia so it appears in the Media tab
        auto& media = effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetSequenceMedia();
        media.GetTextFile(settings["E_FILEPICKERCTRL_Text_File"]);
    }
}

// RGBA image data returned by FlushAndGetImage and cached
struct CachedRGBAImage {
    std::vector<uint8_t> data;
    int width = 0;
    int height = 0;
};

//formatting notes:
//countdown == seconds: put a non-0 value in text line 1 to count down
//countdown == any of the "to date" options: put "Sat, 18 Dec 1999 00:48:30 +0100" in the text line
//countdown = !to date!%fmt: put delimiter + target date + same delimiter + format string with %x markers in it (described down below)

void SetFont(TextDrawingContext *dc, const std::string& FontString, const xlColor &color) {
    TextFontInfo fnt = TextDrawingContext::GetTextFont(FontString);
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
    TEXTDIR_WORDFLIP,
    TEXTDIR_LEFTRIGHT,
    TEXTDIR_UPDOWN
};

static TextDirection TextEffectDirectionsIndex(const std::string &st) {
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
    if (st == "left-right") return TEXTDIR_LEFTRIGHT;
    if (st == "up-down") return TEXTDIR_UPDOWN;
    return TEXTDIR_NONE;
}
static int TextCountDownIndex(const std::string &st) {
    if (st == "seconds") return 1;
    if (st == "to date 'd h m s'") return 2;
    if (st == "to date 'h:m:s'") return 3;
    if (st == "to date 'm' or 's'") return 4;
    if (st == "to date 's'") return 5;
    if (st == "!to date!%fmt") return 6;
    if (st == "minutes seconds") return 7;
    return 0;
}

static int TextEffectsIndex(const std::string &st) {
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
    std::string xl_font = SettingsMap.Get("CHOICE_Text_Font", "Use OS Fonts");
    if( xl_font != "Use OS Fonts" )
    {
        RenderXLText(effect, SettingsMap, buffer);
        return;
    }

    std::string text = SettingsMap["TEXTCTRL_Text"];
    Replace(text, "\\n", "\n");

    std::string filename = SettingsMap["FILEPICKERCTRL_Text_File"];
    std::string lyricTrack = SettingsMap["CHOICE_Text_LyricTrack"];

    if (text.empty())
    {
        if (!filename.empty()) {
            auto* seqMedia = buffer.GetSequenceMedia();
            std::string fileContent;
            if (seqMedia) {
                auto entry = seqMedia->GetTextFile(filename);
                if (entry) {
                    entry->MarkIsUsed();
                    fileContent = entry->GetContent();
                }
            }

            if (!fileContent.empty()) {
                auto lines = Split(fileContent, '\n');

                text.clear();
                int lineCount = std::min((int)lines.size(), MAXTEXTLINES);

                for (int i = 0; i < lineCount; i++) {
                    if (i > 0) {
                        text += "\n";
                    }
                    text += lines[i];
                }

                while (!text.empty() && text.back() == '\n') {
                    text.pop_back();
                }
            }
        }
        else
        {
            if (!lyricTrack.empty())
            {
                Element* t = nullptr;
                for (int i = 0; i < (int)mSequenceElements->GetElementCount(); i++)
                {
                    auto lt = BeforeLast(lyricTrack, '-');
                    lt = lt.substr(0, lt.size() - 1);
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
                    if (EndsWith(lyricTrack, " - Phrases"))
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

    if (!text.empty()) {

        int starty = std::strtol(SettingsMap.Get("SLIDER_Text_YStart", "0").c_str(), nullptr, 10);
        int startx = std::strtol(SettingsMap.Get("SLIDER_Text_XStart", "0").c_str(), nullptr, 10);
        int endy = std::strtol(SettingsMap.Get("SLIDER_Text_YEnd", "0").c_str(), nullptr, 10);
        int endx = std::strtol(SettingsMap.Get("SLIDER_Text_XEnd", "0").c_str(), nullptr, 10);
        bool pixelOffsets = std::strtol(SettingsMap.Get("CHECKBOX_Text_PixelOffsets", "0").c_str(), nullptr, 10);
        bool perWord = std::strtol(SettingsMap.Get("CHECKBOX_Text_Color_PerWord", "0").c_str(), nullptr, 10);

        const CachedRGBAImage* i = RenderTextLine(buffer,
                       buffer.GetTextDrawingContext(),
                       text,
                       SettingsMap["FONTPICKER_Text_Font"],
                       TextEffectDirectionsIndex(SettingsMap["CHOICE_Text_Dir"]),
                       std::strtol(SettingsMap["CHECKBOX_TextToCenter"].c_str(), nullptr, 10),
                       std::strtol(SettingsMap["CHECKBOX_TextNoRepeat"].c_str(), nullptr, 10),
                       TextEffectsIndex(SettingsMap["CHOICE_Text_Effect"]),
                       TextCountDownIndex(SettingsMap["CHOICE_Text_Count"]),
                       std::strtol(SettingsMap.Get("TEXTCTRL_Text_Speed", "10").c_str(), nullptr, 10),
                       startx, starty, endx, endy, pixelOffsets, perWord);

        if (i == nullptr) {
            return;
        }
        const uint8_t* rgba = i->data.data();
        int w = i->width;
        int h = i->height;
        xlColor c;
        int cur = 0;
        for (int y = h - 1; y >= 0; y--) {
            for (int x = 0; x < w; x++) {
                c.Set(rgba[cur], rgba[cur + 1], rgba[cur + 2], rgba[cur + 3]);
                cur += 4;
                buffer.SetPixel(x, y, c);
            }
        }
    }
}

xlSize GetMultiLineTextExtent(TextDrawingContext *dc,
                              const std::string& text,
                              int *widthText,
                              int *heightText,
                              int *hl)
{
    double widthTextMax = 0, widthLine;
    double heightTextTotal = 0;
    double heightLineDefault = 0, heightLine = 0;

    std::string curLine;
    for ( size_t idx = 0; ; ++idx )
    {
        if ( idx == text.size() || text[idx] == '\n' )
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
                    dc->GetTextExtent("W", &dummy, &heightLineDefault);
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

            if ( idx == text.size() )
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
            curLine += text[idx];
        }
    }
    *widthText = widthTextMax;
    *heightText = heightTextTotal;
    *hl = heightLine;
    return xlSize(widthTextMax, heightTextTotal);
}

class CachedTextInfo {
public:
    CachedTextInfo() {}
    CachedTextInfo(const std::string &txt, const std::string font, const std::vector<xlColor> &c, const xlRect &r)
    : text(txt), rect(r), color(c), fontString(font) {}
    ~CachedTextInfo() {}
    
    bool operator==(const CachedTextInfo &i) const {
        return (text == i.text)
            && (fontString == i.fontString)
            && (rect == i.rect)
            && (color == i.color);
    }
    
    std::string text;
    xlRect rect;
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
    TextRenderCache() : timer_countdown(0), synced_textsize(xlSize(0,0)) {};
    virtual ~TextRenderCache() = default;
    int timer_countdown;
    xlSize synced_textsize;

    CachedRGBAImage *GetImage(const CachedTextInfo &inf) {
        auto it = textCache.find(inf);
        if (it != textCache.end()) return &it->second;
        return nullptr;
    }
    CachedRGBAImage* PutImage(const CachedTextInfo &inf, std::vector<uint8_t> rgbaData, int w, int h) {
        auto& img = textCache[inf];
        img.width = w;
        img.height = h;
        img.data = std::move(rgbaData);
        return &img;
    }
    
    xlSize GetMultiLineTextExtent(const std::string &font, const std::string &msg) {
        std::pair<std::string, std::string> key(font, msg);
        auto i = textExtentCache.find(key);
        if (i == textExtentCache.end()) {
            return xlSize(-1, -1);
        }
        return i->second;
    }
    void PutMultiLineTextExtent(const std::string &font, const std::string &msg, const xlSize &sz) {
        std::pair<std::string, std::string> key(font, msg);
        textExtentCache[key] = sz;
    }

    CachedRGBAImage lastRendered; // temp storage for uncached FlushAndGetImage results
    std::unordered_map<CachedTextInfo, CachedRGBAImage, CachedTextInfoHasher> textCache;
    std::map<std::pair<std::string, std::string>, xlSize> textExtentCache;
};

xlSize GetMultiLineTextExtent(TextDrawingContext *dc,
                              const std::string& text,
                              TextRenderCache *cache,
                              const std::string &font,
                              bool &fontSet)
{
    xlSize i = cache->GetMultiLineTextExtent(font, text);
    if (i.width == -1 && i.height == -1) {
        if (!fontSet) {
            dc->Clear();
            SetFont(dc, font, xlWHITE);
            fontSet = true;
        }
        int x,y,z;
        i = GetMultiLineTextExtent(dc, text, &x, &y, &z);
        cache->PutMultiLineTextExtent(font, text, i);
    }
    return i;
}

void DrawLabel(TextDrawingContext *dc,
               const std::string& text,
               const xlRect& rect,
               int alignment,
               TextRenderCache *cache,
               const std::string &fontString,
               const std::vector<xlColor> colors,
               const bool perWord = false)
{
    // find the text position
    int widthText, heightText, heightLine;
    GetMultiLineTextExtent(dc, text, &widthText, &heightText, &heightLine);

    int width = widthText;
    int height = heightText;

    int x, y;
    if ( alignment & TEXT_ALIGN_RIGHT )
    {
        x = rect.GetRight() - width;
    }
    else if ( alignment & TEXT_ALIGN_CENTER_HORIZONTAL )
    {
        x = (rect.GetLeft() + rect.GetRight() + 1 - width) / 2;
    }
    else // alignment & wxALIGN_LEFT
    {
        x = rect.GetLeft();
    }

    if ( alignment & TEXT_ALIGN_BOTTOM )
    {
        y = rect.GetBottom() - height;
    }
    else if ( alignment & TEXT_ALIGN_CENTER_VERTICAL )
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
    std::string curLine;
    int curPos = 0;
    for ( size_t idx = 0; ; ++idx )
    {
        if ( idx == text.size() || text[idx] == '\n' ) {
            int xRealStart = x; // init it here to avoid compielr warnings
            if ( !curLine.empty() )
            {
                // NB: can't test for !(alignment & wxALIGN_LEFT) because
                //     wxALIGN_LEFT is 0
                if ( alignment & (TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_HORIZONTAL) )
                {
                    int x1,y1,z1;
                    int widthLine = GetMultiLineTextExtent(dc, curLine, &x1, &y1, &z1).width;

                    if ( alignment & TEXT_ALIGN_RIGHT )
                    {
                        xRealStart += width - widthLine;
                    }
                    else // if ( alignment & TEXT_ALIGN_CENTER_HORIZONTAL )
                    {
                        xRealStart += (width - widthLine) / 2;
                    }
                }
                //else: left aligned, nothing to do
                if (colors.size() != 1) {
                    std::vector<double> d;
                    dc->GetTextExtents(curLine, d);
                    for (int x1 = 0; x1 < (int)curLine.size(); x1++) {
                        std::string c(1, curLine[x1]);
                        if (c != " ") {
                            SetFont(dc, fontString, colors[curPos % colors.size()]);
                            double loc = xRealStart;
                            if (x1 != 0) {
                                if (x1 - 1 < (int)d.size()) {
                                    loc += d[x1 - 1];
                                }
                                else {
                                    assert(false); // this seems to happen when fonts are not good
                                }
                            }
                            dc->DrawText(c, loc, y);
                        }
                        if ((perWord && c == " " && x1 + 1 < (int)curLine.size() && curLine[x1 + 1] != ' ') ||
                            (!perWord && c != " ")) {
                            curPos++;
                        }
                    }
                } else {
                    dc->DrawText(curLine, xRealStart, y);
                }
            }

            y += heightLine;

            if ( idx == text.size() )
                break;

            curLine.clear();
        }
        else // not end of line
        {
            curLine += text[idx];
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


TextRenderCache *GetCache(RenderBuffer &buffer, int id) {
    TextRenderCache *cache = (TextRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new TextRenderCache();
        buffer.infoCache[id] = cache;
    }
    return cache;
}

//jwylie - 2016-11-01  -- enhancement: add minute seconds countdown
const CachedRGBAImage *TextEffect::RenderTextLine(RenderBuffer &buffer,
                                    TextDrawingContext* dc,
                                    const std::string& Line_orig,
                                    const std::string &fontString,
                                    int dir,
                                    bool center, bool norepeat, int Effect, int Countdown, int tspeed,
                                    int startx, int starty, int endx, int endy,
                                    bool isPixelBased, bool perWord) const
{
    int i;
    std::string Line = Line_orig;
    std::string msg, tempmsg;

    if (Line.empty()) return nullptr;

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
            for(i=0; i<(int)tempmsg.length(); i++)
            {
                msg = msg + tempmsg[tempmsg.length()-i-1] + "\n";
            }
            break;
        case 2:
            // vertical text down
            tempmsg=msg;
            msg.clear();
            for(i=0; i<(int)tempmsg.length(); i++)
            {
                msg = msg + tempmsg[i] + "\n";
            }
            break;
        default: break;
    }
    
    TextRenderCache *cache = GetCache(buffer, id);
    bool fontSet = false;
    
    xlSize textsize = GetMultiLineTextExtent(dc, msg, cache, fontString, fontSet);
    auto ltrim = [](const std::string& s) { size_t start = s.find_first_not_of(" \t"); return start == std::string::npos ? std::string() : s.substr(start); };
    auto rtrim = [](const std::string& s) { size_t end = s.find_last_not_of(" \t"); return end == std::string::npos ? std::string() : s.substr(0, end + 1); };
    int extra_left = IsGoingLeft(dir)? textsize.width - GetMultiLineTextExtent(dc, ltrim(msg), cache, fontString, fontSet).width: 0;
    int extra_right = IsGoingRight(dir)? textsize.width - GetMultiLineTextExtent(dc, rtrim(msg), cache, fontString, fontSet).width: 0;
    int xoffset=0;
    int yoffset=0;

    switch(Effect)
    {
        case 3:
            // rotate up 45
            TextRotation=45.0;
            yoffset=int(0.707*double(textsize.GetHeight()));
            i=int(0.707*double(textsize.GetWidth()+textsize.GetHeight()));
            textsize = xlSize(i,i);
            break;
        case 4:
            // rotate up 90
            TextRotation=90.0;
            textsize = xlSize(textsize.GetHeight(),textsize.GetWidth());  // swap width & height
            break;
        case 5:
            // rotate down 45
            TextRotation=-45.0;
            xoffset=int(0.707*double(textsize.GetHeight()));
            i=int(0.707*double(textsize.GetWidth()+textsize.GetHeight()));
            textsize = xlSize(i,i);
            yoffset=i;
            break;
        case 6:
            // rotate down 90
            TextRotation=-90.0;
            xoffset=textsize.GetHeight();
            yoffset=textsize.GetWidth();
            textsize = xlSize(textsize.GetHeight(),textsize.GetWidth());  // swap width & height
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
        xlRect rect(0,0,buffer.BufferWi,buffer.BufferHt);
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
            case TEXTDIR_LEFT: // OS FONTS
                {                
                    int state8 = state / 8;
                    if (state8 < 0) state8 += 32768;
                    if (state > 2000000)
                        state = state + 0;
                    if (norepeat && !center && state > xlimit) {
                        rect.Offset(-xlimit, OffsetTop);
                    } else {
                        rect.Offset(center ? std::max((int)(xlimit / 16 - state8), -extra_left/2) : xlimit/16 - state % xlimit/8, OffsetTop);
                    }
                }
                break; // left, optionally stop at center
            case TEXTDIR_RIGHT: 
                if (norepeat && !center && state > xlimit) {
                    rect.Offset(xlimit, OffsetTop);
                } else {
                    rect.Offset(center ? std::min((int)(state /*% xlimit*/ / 8 - xlimit/16), extra_right/2) : state % xlimit/8 - xlimit/16, OffsetTop);
                }
                break; // right, optionally stop at center
            case TEXTDIR_UP:
                if (norepeat && !center && state > ylimit) {
                    rect.Offset(OffsetLeft, -ylimit);
                } else {
                    rect.Offset(OffsetLeft, center ? std::max((int)(ylimit/16 - state /*% ylimit*/ /8), 0): ylimit/16 - state % ylimit/8);
                }
                break; // up, optionally stop at center
            case TEXTDIR_DOWN:;
                if (norepeat && !center && state > ylimit) {
                    rect.Offset(OffsetLeft, ylimit);
                } else {
                    rect.Offset(OffsetLeft, center ? std::min((int)(state /*% ylimit*/ /8 - ylimit/16), 0) : state % ylimit/8 - ylimit/16);
                }
                break; // down, optionally stop at center
            case TEXTDIR_UPLEFT:
                if (norepeat && !center && (state > ylimit || state > xlimit)) {
                    rect.Offset(-xlimit, -ylimit);
                } else {
                    rect.Offset(center ? std::max((int)(xlimit/16 - state /*% xlimit*/ /8) + startx, 0): xlimit/16 - state % xlimit/8 + startx, 
                                center ? std::max((int)(ylimit/16 - state /*% ylimit*/ /8) - starty, 0): ylimit/16 - state % ylimit/8 - starty);
                }
                break; // up-left, optionally stop at center
            case TEXTDIR_DOWNLEFT:
                if (norepeat && !center && (state > ylimit || state > xlimit)) {
                    rect.Offset(-xlimit, ylimit);
                } else {
                    rect.Offset(center ? std::max((int)(xlimit/16 - state /*% xlimit*/ /8) + startx, 0): xlimit/16 - state % xlimit/8 + startx,
                                center ? std::min((int)(state /*% ylimit*/ /8 - ylimit/16) + starty, 0): state % ylimit/8 - ylimit/16 + starty);
                }
                break; // down-left, optionally stop at center
            case TEXTDIR_UPRIGHT:
                if (norepeat && !center && (state > ylimit || state > xlimit)) {
                    rect.Offset(xlimit, -ylimit);
                } else {
                    rect.Offset(center ? std::min((int)(state /*% xlimit*/ /8 - xlimit/16) - startx, 0): state % xlimit/8 - xlimit/16 - startx,
                                center ? std::max((int)(ylimit/16 - state /*% ylimit*/ /8) - starty, 0): ylimit/16 - state % ylimit/8 - starty);
                }
                break; // up-right, optionally stop at center
            case TEXTDIR_DOWNRIGHT:
                if (norepeat && !center && (state > ylimit || state > xlimit)) {
                    rect.Offset(xlimit, ylimit);
                } else {
                    rect.Offset(center ? std::min((int)(state /*% xlimit*/ /8 - xlimit/16) - startx, 0): state % xlimit/8 - xlimit/16 - startx,
                                center ? std::min((int)(state /*% ylimit*/ /8 - ylimit/16) + starty, 0): state % ylimit/8 - ylimit/16 + starty);
                }
                break; // down-right, optionally stop at center
            case TEXTDIR_WAVEY_LRUPDOWN:
                if (center) //does to-center make sense with this one?
                    rect.Offset(std::min((int)(state /*% xlimit*/ /8 - xlimit/16), extra_right/2), std::max((int)zigzag(state/4, totheight)/2 - totheight/4, -extra_left/2));
                else
                    rect.Offset(xlimit/16 - state % xlimit/8, zigzag(state/4, totheight)/2 - totheight/4);
                break; // left-to-right, wavey up-down 1/2 height (too bouncy if full height is used), slow down up/down motion (too fast unless scaled)
            case TEXTDIR_LEFTRIGHT: {
                int OffsetX;
                const int cycle = xlimit;
                const int halfCycle = xlimit / 2;
                const int normalizedState = state % cycle;
                if (normalizedState <= halfCycle) {
                    OffsetX = xlimit / 8 - (normalizedState * (xlimit / 4)) / halfCycle;
                } else {
                    OffsetX = -xlimit / 8 + ((normalizedState - halfCycle) * (xlimit / 4)) / halfCycle;
                }
                if (norepeat && state > xlimit) {
                    rect.Offset(-xlimit, OffsetTop);
                } else {
                    rect.Offset(OffsetX, OffsetTop);
                }
            } break; // Moves right to left, then left to right
            case TEXTDIR_UPDOWN: {
                int OffsetY;
                const int cycle = ylimit;
                const int halfCycle = ylimit / 2;
                const int normalizedState = state % cycle;
                if (normalizedState <= halfCycle) {
                    OffsetY = (ylimit / 16) - (normalizedState * (ylimit / 8)) / halfCycle;
                } else {
                    OffsetY = -(ylimit / 16) + ((normalizedState - halfCycle) * (ylimit / 8)) / halfCycle;
                }
                if (norepeat && state > ylimit) {
                    rect.Offset(OffsetLeft, -ylimit);
                } else {
                    rect.Offset(OffsetLeft, OffsetY);
                }
            } break; // Moves top to bottom, then back bottom to top
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
        CachedTextInfo inf(msg, fontString, colors, rect);
        CachedRGBAImage *img = GetCache(buffer,id)->GetImage(inf);
        if (img == nullptr) {
            dc->Clear();
            SetFont(dc, fontString, colors[0]);
            DrawLabel(dc, msg, rect, TEXT_ALIGN_CENTER_HORIZONTAL|TEXT_ALIGN_CENTER_VERTICAL, GetCache(buffer,id), fontString, colors, perWord);
            int iw, ih;
            const uint8_t* rgba = dc->FlushAndGetImage(&iw, &ih);
            std::vector<uint8_t> rgbaData(rgba, rgba + (size_t)iw * ih * 4);
            img = GetCache(buffer,id)->PutImage(inf, std::move(rgbaData), iw, ih);
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

    int iw, ih;
    const uint8_t* rgba = buffer.GetTextDrawingContext()->FlushAndGetImage(&iw, &ih);
    cache->lastRendered.width = iw;
    cache->lastRendered.height = ih;
    cache->lastRendered.data.assign(rgba, rgba + (size_t)iw * ih * 4);
    return &cache->lastRendered;

}

void TextEffect::FormatCountdown(int Countdown, int state, std::string& Line, RenderBuffer &buffer, std::string& msg, std::string Line_orig) const
{
    long longsecs;
    int framesPerSec = 1000 / buffer.frameTimeInMs;
    int minutes,seconds;

    std::string fmt = Line_orig;
    std::string prepend = Line_orig;   //for prepended/appended text to countdown
    std::string append = Line_orig;   //for prepended/appended text to countdown
    std::string timePart = Line_orig;

    switch (Countdown)
    {
    case COUNTDOWN_SECONDS:
            {
                // countdown seconds
                if (state == 0)
                {
                    long tempLong = std::strtol(Line.c_str(), nullptr, 10);
                    GetCache(buffer, id)->timer_countdown = buffer.curPeriod + tempLong*framesPerSec + framesPerSec - 1;  // capture 0 period
                }
                seconds = (GetCache(buffer, id)->timer_countdown - buffer.curPeriod) / framesPerSec;
                if (seconds < 0) seconds = 0;
                msg = std::format("{}", seconds);
            }
            break;
//jwylie - 2016-11-01  -- enhancement: add minute seconds countdown
        case COUNTDOWN_MINUTES_SECONDS:
        {
            if (timePart.find('/') != std::string::npos)
            {
                timePart = BeforeLast(AfterFirst(timePart, '/'), '/');
                prepend = BeforeFirst(prepend, '/');
                append = AfterLast(append, '/');
            }
            else
            {
                append = "";
                prepend = "";
            }
            auto minSec = Split(timePart, ':');
            if (minSec.size() == 1)
            {
                seconds = std::strtol(minSec[0].c_str(), nullptr, 10);
            }
            else if (minSec.size() == 2)
            {
                minutes = std::strtol(minSec[0].c_str(), nullptr, 10);
                seconds = (minutes * 60) + std::strtol(minSec[1].c_str(), nullptr, 10);

            }
            else //invalid format
            {
                msg = "Invalid Format";
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

            std::string tempSeconds = std::format("{}", seconds);

            if (tempSeconds.size() == 1)
                tempSeconds = "0" + tempSeconds;

            msg = prepend + ' ' + std::format("{}", minutes) + " : " + tempSeconds + append;
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
                    char delim = Line[0]; //use first char as date delimiter; date and format string follows that, separated by delimiter
                    Line.erase(0, 1); //remove leading delim
                    fmt = AfterFirst(Line, delim);
                    auto delimPos = Line.find(delim);
                    if (delimPos != std::string::npos) Line.resize(delimPos); //remove fmt string, leaving only count down date
                }
                else fmt.clear();
            //CAUTION: fall thru here
        case COUNTDOWN_D_H_M_S:
        case COUNTDOWN_H_M_S:
        case COUNTDOWN_M_or_S:
        case COUNTDOWN_S:
        {
            // countdown to date
            if (state%framesPerSec == 0)   //1x/sec
            {
                // Parse RFC822 date string (e.g. "Wed, 02 Oct 2015 15:00:00 +0200")
                std::tm tmbuf = {};
                std::istringstream iss(Line);
                iss >> std::get_time(&tmbuf, "%a, %d %b %Y %H:%M:%S");
                if (!iss.fail())
                {
                    // Parse timezone offset if present (e.g. "+0200" or "-0500")
                    int tzOffsetSecs = 0;
                    std::string tzStr;
                    iss >> tzStr;
                    if (!tzStr.empty() && (tzStr[0] == '+' || tzStr[0] == '-')) {
                        int sign = (tzStr[0] == '+') ? 1 : -1;
                        int tzVal = (int)std::strtol(tzStr.c_str() + 1, nullptr, 10);
                        tzOffsetSecs = sign * ((tzVal / 100) * 3600 + (tzVal % 100) * 60);
                    }
                    // Convert parsed time to UTC time_t, then adjust for timezone
#ifdef _MSC_VER
                    std::time_t targetTime = _mkgmtime(&tmbuf) - tzOffsetSecs;
#else
                    std::time_t targetTime = timegm(&tmbuf) - tzOffsetSecs;
#endif
                    auto now = std::chrono::system_clock::now();
                    auto nowTime = std::chrono::system_clock::to_time_t(now);
                    int64_t diffSecs = static_cast<int64_t>(difftime(targetTime, nowTime));
                    if (diffSecs > LONG_MAX) diffSecs = LONG_MAX;
                    if (diffSecs < 0) diffSecs = 0;
                    longsecs = static_cast<long>(diffSecs);
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
            }
            if (!longsecs)
            {
                msg = "invalid date";    //show when invalid -DJ
                break;
            }
            int days = longsecs / 60 / 60 / 24;
            int hours = (longsecs / 60 / 60) % 24;
            minutes = (longsecs / 60) % 60;
            seconds = longsecs % 60;
            if (Countdown == COUNTDOWN_D_H_M_S)
                msg = std::format("{}d {}h {}m {}s", days, hours, minutes, seconds);
            else if (Countdown == COUNTDOWN_H_M_S)
                msg = std::format("{} : {} : {}", hours, minutes, seconds);
            else if (Countdown == COUNTDOWN_S)
                msg = std::format("{}", 60 * 60 * hours + 60 * minutes + seconds);
            else if (Countdown == COUNTDOWN_FREEFMT)
                //            msg = _T("%%") + Line + _T("%%") + fmt + _T("%%");
                if (fmt == "" || (EndsWith(fmt, "%") && !EndsWith(fmt, "%%")))
                {
                    msg = "invalid format";
                }
                else
                {
                    // Format timespan using wxTimeSpan-compatible format specifiers:
                    // %H=hours, %M=minutes, %S=seconds, %l=milliseconds, %D=days, %E=weeks, %%=literal %
                    long totalHours = longsecs / 3600;
                    int fmtMinutes = (longsecs / 60) % 60;
                    int fmtSeconds = longsecs % 60;
                    long fmtDays = longsecs / 86400;
                    long fmtWeeks = longsecs / (7 * 86400);
                    std::string result;
                    for (size_t i = 0; i < fmt.length(); i++) {
                        if (fmt[i] == '%' && i + 1 < fmt.length()) {
                            i++;
                            switch (fmt[i]) {
                                case 'H': result += std::format("{:02}", totalHours); break;
                                case 'M': result += std::format("{:02}", fmtMinutes); break;
                                case 'S': result += std::format("{:02}", fmtSeconds); break;
                                case 'l': result += "000"; break; // no sub-second precision in countdown
                                case 'D': result += std::format("{}", fmtDays); break;
                                case 'E': result += std::format("{}", fmtWeeks); break;
                                case '%': result += '%'; break;
                                default: result += '%'; result += fmt[i]; break;
                            }
                        } else {
                            result += fmt[i];
                        }
                    }
                    msg = result;
                }
            else //if (Countdown == COUNTDOWN_M_or_S)
                if (60 * hours + minutes < 5) //COUNTDOWN_M_or_S: show seconds
                    msg = std::format("{}", 60 * 60 * hours + 60 * minutes + seconds);
                else //COUNTDOWN_M_or_S: show minutes
                    msg = std::format("{} m", 60 * hours + minutes);
        }
            break;
        default:
            msg=Line;
            Replace(msg, "\\n", "\n"); //allow vertical spacing (mainly for up/down) -DJ
            break;
    }
}

#define msgReplace(a, b, c) \
    do                      \
    {                    \
       if (Contains(a, b)) { \
           Replace(a, b, (c)); \
       } \
    } while (0)

void TextEffect::ReplaceVaribles(std::string& msg, RenderBuffer& buffer) const
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

    if (Contains(msg, "${UPPER}")) {
        Replace(msg, "${UPPER}", "");
        msg = Upper(msg);
    }
    if (Contains(msg, "${LOWER}")) {
        Replace(msg, "${LOWER}", "");
        msg = Lower(msg);
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
        int tspeed = std::strtol(settings.Get("TEXTCTRL_Text_Speed", "10").c_str(), nullptr, 10); // 0 to 50

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

    int starty = std::strtol(settings.Get("SLIDER_Text_YStart", "0").c_str(), nullptr, 10);
    int startx = std::strtol(settings.Get("SLIDER_Text_XStart", "0").c_str(), nullptr, 10);
    int endy = std::strtol(settings.Get("SLIDER_Text_YEnd", "0").c_str(), nullptr, 10);
    int endx = std::strtol(settings.Get("SLIDER_Text_XEnd", "0").c_str(), nullptr, 10);
    bool pixelOffsets = std::strtol(settings.Get("CHECKBOX_Text_PixelOffsets", "0").c_str(), nullptr, 10);
    bool perWord = std::strtol(settings.Get("CHECKBOX_Text_Color_PerWord", "0").c_str(), nullptr, 10);

    int OffsetLeft = startx * buffer.BufferWi / 100;
    int OffsetTop = -starty * buffer.BufferHt / 100;
    if (pixelOffsets) {
        OffsetLeft = startx;
        OffsetTop = -starty;
    }

    font_mgr.init();
    std::string xl_font = settings["CHOICE_Text_Font"];
    xlFont* font = font_mgr.get_font(xl_font);
    const xlImage* image = font->get_image();
    int char_width = font->GetWidth();
    int char_height = font->GetHeight();

    std::string text = settings["TEXTCTRL_Text"];
    Replace(text, "\\n", "\n");

    std::string filename = settings["FILEPICKERCTRL_Text_File"];
    std::string lyricTrack = settings["CHOICE_Text_LyricTrack"];

    if (text == "") {
        if (!filename.empty()) {
            auto* seqMedia = buffer.GetSequenceMedia();
            std::string fileContent;
            if (seqMedia) {
                auto entry = seqMedia->GetTextFile(filename);
                if (entry) {
                    entry->MarkIsUsed();
                    fileContent = entry->GetContent();
                }
            }

            if (!fileContent.empty()) {
                auto lines = Split(fileContent, '\n');

                text.clear();
                int lineCount = std::min((int)lines.size(), MAXTEXTLINES);

                for (int i = 0; i < lineCount; i++) {
                    if (i > 0) {
                        text += "\n";
                    }
                    text += lines[i];
                }

                while (!text.empty() && text.back() == '\n') {
                    text.pop_back();
                }
            }
        }
        else if (lyricTrack != "") {
            Element* t = nullptr;
            for (int i = 0; i < (int)mSequenceElements->GetElementCount(); i++) {
                auto lt = BeforeLast(lyricTrack, '-');
                lt = lt.substr(0, lt.size() - 1);
                Element* e = mSequenceElements->GetElement(i);
                if (e->GetEffectLayerCount() > 1 && e->GetType() == ElementType::ELEMENT_TYPE_TIMING && e->GetName() == lt) {
                    t = e;
                    break;
                }
            }
            if (t != nullptr) {
                long time = buffer.curPeriod * buffer.frameTimeInMs;
                EffectLayer* el = nullptr;
                if (EndsWith(lyricTrack, " - Phrases")) {
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

    std::string msg = text;
    int Countdown = TextCountDownIndex(settings["CHOICE_Text_Count"]);
    if (Countdown > 0) {
        int tspeed = std::strtol(settings.Get("TEXTCTRL_Text_Speed", "10").c_str(), nullptr, 10);
        int state = (buffer.curPeriod - buffer.curEffStartPer) * tspeed * buffer.frameTimeInMs / 50;
        std::string Line = text;
        FormatCountdown(Countdown, state, Line, buffer, msg, text);
        Replace(msg, " : ", ":");
    }
    ReplaceVaribles(msg, buffer);
    text = msg;

    auto lines = Split(text, '\n');
    std::vector<int> line_lengths;
    int max_line_length = 0;
    for (const auto& line : lines) {
        int len = font_mgr.get_length(font, line);
        line_lengths.push_back(len);
        max_line_length = std::max(max_line_length, len);
    }

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
        for (size_t i = 0; i < lines.size(); i++) {
            lines[i] = FlipWord(settings, lines[i], buffer);
        }
    }

    int PreOffsetLeft = OffsetLeft;
    int PreOffsetTop = OffsetTop;

    AddMotions(OffsetLeft, OffsetTop, settings, buffer, text.size(), endx, endy, pixelOffsets, PreOffsetLeft, PreOffsetTop, max_line_length, char_width, char_height, vertical, rotate_90);

    if (rotate_90) {
        OffsetLeft += buffer.BufferWi / 2 - font->GetCapsHeight() / 2;
        if (up) {
            OffsetTop += buffer.BufferHt / 2 + max_line_length / 2;
        }
        else {
            OffsetTop += buffer.BufferHt / 2 - max_line_length / 2;
        }
    }
    else if (vertical) {
        OffsetLeft += buffer.BufferWi / 2 - char_width / 2 + 2;
        if (up) {
            OffsetTop += buffer.BufferHt / 2 + (char_height * lines.size()) / 2;
        }
        else {
            OffsetTop += buffer.BufferHt / 2 - (char_height * lines.size()) / 2;
        }
    }
    else {
        OffsetLeft += buffer.BufferWi / 2;
        OffsetTop += buffer.BufferHt / 2 - (font->GetCapsHeight() * lines.size()) / 2;
    }

    if (!lines.empty()) {
        int curPos = 0;
        for (size_t line_idx = 0; line_idx < lines.size(); line_idx++) {
            const std::string& line = lines[line_idx];
            int line_offset_left = OffsetLeft - line_lengths[line_idx] / 2;
            if (rotate_90 || vertical) {
                line_offset_left = OffsetLeft;
            }

            for (int i = 0; i < (int)line.length(); i++) {
                buffer.palette.GetColor(curPos % num_colors, c);
                if ((perWord && line[i] == ' ' && i + 1 < (int)line.size() && line[i + 1] != ' ') ||
                    (!perWord && line[i] != ' ')) {
                    curPos++;
                }
                char ascii = line[i];
                int x_start_corner = (ascii % 8) * (char_width + 1) + 1;
                int y_start_corner = (ascii / 8) * (char_height + 1) + 1;

                int actual_width = font->GetCharWidth(ascii);
                assert(actual_width > 0);
                if (rotate_90 && up) {
                    OffsetTop -= actual_width;
                }
                for (int w = 0; w < actual_width; w++) {
                    int x_pos = x_start_corner + w;
                    for (int y_pos = y_start_corner; y_pos < y_start_corner + char_height; y_pos++) {
                        if (x_pos >= 0 && x_pos < image->GetWidth() && y_pos >= 0 && y_pos < image->GetHeight()) {
                            int red = image->GetRed(x_pos, y_pos);
                            int green = image->GetGreen(x_pos, y_pos);
                            int blue = image->GetBlue(x_pos, y_pos);
                            if (red == 255 && green == 255 && blue == 255) {
                                if (rotate_90) {
                                    if (up) {
                                        buffer.SetPixel(y_pos - y_start_corner + line_offset_left, (buffer.BufferHt - 1) - (actual_width - 1 - x_pos + x_start_corner + OffsetTop), c, false);
                                    } else {
                                        buffer.SetPixel(char_height - 1 - y_pos + y_start_corner + line_offset_left, (buffer.BufferHt - 1) - (x_pos - x_start_corner + OffsetTop), c, false);
                                    }
                                } else {
                                    buffer.SetPixel(x_pos - x_start_corner + line_offset_left, buffer.BufferHt - (y_pos - y_start_corner + OffsetTop) - 1, c, false);
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
                    line_offset_left += actual_width;
                }
            }
            OffsetTop += font->GetHeight() + 1;
        }
    }
}

void TextEffect::AddMotions(int& OffsetLeft, int& OffsetTop, const SettingsMap& settings, RenderBuffer &buffer,
    int txtLen, int endx, int endy, bool pixelOffsets, int PreOffsetLeft, int PreOffsetTop, int text_len, int char_width, int char_height, bool vertical, bool rotate_90) const
{
    int tspeed = std::strtol(settings.Get("TEXTCTRL_Text_Speed", "10").c_str(), nullptr, 10);
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
    int center = std::strtol(settings["CHECKBOX_TextToCenter"].c_str(), nullptr, 10);
    int norepeat = std::strtol(settings["CHECKBOX_TextNoRepeat"].c_str(), nullptr, 10);

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
    case TEXTDIR_LEFT: // XL FONTS REALLY
        OffsetLeft = center ? std::max(buffer.BufferWi - (state / 8) + PreOffsetLeft + (txtwidth / 2),0):
                                        (buffer.BufferWi - (state % (xlimit + buffer.BufferWi)) / 8) + PreOffsetLeft + (txtwidth / 2);
        if (norepeat && !center && state > (buffer.BufferWi + PreOffsetLeft + txtwidth) * 8) {
            OffsetLeft = -(buffer.BufferWi + PreOffsetLeft + txtwidth);
        }
        break; // left
    case TEXTDIR_RIGHT:
        OffsetLeft = center ? std::min(state / 8 - txtwidth + PreOffsetLeft, 0) : (state % xlimit / 8 - txtwidth + PreOffsetLeft);
        if (norepeat && !center && state > xlimit) {
            OffsetLeft = xlimit + PreOffsetLeft;
        }
        break; // right
    case TEXTDIR_UP:
        OffsetTop = center ? std::max(buffer.BufferHt - state / 8 - PreOffsetTop, 0) : (buffer.BufferHt - state % ylimit / 8 - PreOffsetTop);
        if (norepeat && !center && state > ylimit) {
            OffsetTop = -ylimit;
        }
        break; // up
    case TEXTDIR_DOWN:
        OffsetTop = center ? std::min(state / 8 - (buffer.BufferHt / 2) - PreOffsetTop, 0):(state % ylimit / 8 - (buffer.BufferHt / 2) - PreOffsetTop);
        if (norepeat && !center && state > (ylimit - (buffer.BufferHt / 2))) {
            OffsetTop = ylimit;
        }
        break; // down
    case TEXTDIR_UPLEFT:
        OffsetLeft = center ? std::max(buffer.BufferWi - state / 8 + PreOffsetLeft,0):(buffer.BufferWi - state % xlimit / 8 + PreOffsetLeft);
        OffsetTop = center ? std::max(buffer.BufferHt - state / 8 - PreOffsetTop,0):(buffer.BufferHt - state % ylimit / 8 - PreOffsetTop);
        if (norepeat && !center && (state > ylimit || state > xlimit)) {
                OffsetTop = -ylimit;
                OffsetLeft= -xlimit;
        }
        break; // up-left
    case TEXTDIR_DOWNLEFT:
        OffsetLeft = center ? std::max(buffer.BufferWi - state / 8 + PreOffsetLeft, 0) :(buffer.BufferWi - state % xlimit / 8 + PreOffsetLeft);
        OffsetTop = center ? std::min(state / 8 - txtheight - PreOffsetTop,0):(state % ylimit / 8 - txtheight - PreOffsetTop);
        if (norepeat && !center && (state > (ylimit - (buffer.BufferHt / 2)) || state > (buffer.BufferWi + PreOffsetLeft + txtwidth) * 8)) {
            OffsetLeft = -(buffer.BufferWi + PreOffsetLeft + txtwidth);
            OffsetTop = ylimit;
        }
        break; // down-left
    case TEXTDIR_UPRIGHT:
        OffsetLeft = center ? std::min(state / 8 - txtwidth + PreOffsetLeft,0):(state % xlimit / 8 - txtwidth + PreOffsetLeft);
        OffsetTop = center ? std::max(buffer.BufferHt - state / 8 - PreOffsetTop, 0) : (buffer.BufferHt - state % ylimit / 8 - PreOffsetTop);
        if (norepeat && !center && (state > ylimit || state > xlimit)) {
            OffsetLeft = xlimit + PreOffsetLeft;
            OffsetTop = -ylimit;
        }
        break; // up-right
    case TEXTDIR_DOWNRIGHT:
        OffsetLeft = center ? std::min(state / 8 - txtwidth + PreOffsetLeft, 0) : (state % xlimit / 8 - txtwidth + PreOffsetLeft);
        OffsetTop = center ? std::min(state / 8 - txtheight - PreOffsetTop, 0) : (state % ylimit / 8 - txtheight - PreOffsetTop);
        if (norepeat && !center && (state > ylimit || state > (buffer.BufferWi + PreOffsetLeft + txtwidth) * 8)) {
            OffsetLeft = -(buffer.BufferWi + PreOffsetLeft + txtwidth);
            OffsetTop = -ylimit;
        }
        break; // down-right
    case TEXTDIR_WAVEY_LRUPDOWN:
        OffsetLeft = xlimit / 16 - state % xlimit / 8;
        OffsetTop = zigzag(state / 4, totheight) / 2 - totheight / 4;
        break; // left-to-right, wavey up-down 1/2 height (too bouncy if full height is used), slow down up/down motion (too fast unless scaled)
    case TEXTDIR_LEFTRIGHT: {
        int OffsetX;
        const int cycle = xlimit;
        const int halfCycle = xlimit / 2;
        const int normalizedState = state % cycle;
        if (normalizedState <= halfCycle) {
            OffsetX = (xlimit / 16) - (normalizedState * (xlimit / 8)) / halfCycle;
        } else {
            OffsetX = -(xlimit / 16) + ((normalizedState - halfCycle) * (xlimit / 8)) / halfCycle;
        }
        if (norepeat && state > xlimit) {
            OffsetLeft = -xlimit;
        } else {
            OffsetLeft = OffsetX;
        }
    } break; // Moves left then back right
    case TEXTDIR_UPDOWN: {
        int OffsetY;
        const int cycle = ylimit;
        const int halfCycle = ylimit / 2;
        const int normalizedState = state % cycle;
        if (normalizedState <= halfCycle) {
            OffsetY = (ylimit / 16) - (normalizedState * (ylimit / 8)) / halfCycle;
        } else {
            OffsetY = -(ylimit / 16) + ((normalizedState - halfCycle) * (ylimit / 8)) / halfCycle;
        }
        if (norepeat && state > ylimit) {
            OffsetTop = -ylimit;
        } else {
            OffsetTop = OffsetY;
        }
    } break; // Moves bottom to top, then back top to bottom
    default:
        break;
    }
}
