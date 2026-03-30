/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShapeEffect.h"

#include "../utils/xlPoint.h"
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <format>
#include "TextEffect.h" // FontMapLock

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "../UtilClasses.h"
#include "../models/Model.h"
#include "../render/SequenceElements.h"
#include "../render/SequenceMedia.h"
#include "UtilFunctions.h"
#include "AudioManager.h"
#include "../ExternalHooks.h"
#include "../xLightsMain.h" 

#include "../utils/nanosvg_xl.h"
#include "nanosvgrast_impl.h"

#include <regex>
#include <log.h>

#include "../../include/shape-16.xpm"
#include "../../include/shape-24.xpm"
#include "../../include/shape-32.xpm"
#include "../../include/shape-48.xpm"
#include "../../include/shape-64.xpm"

#define REPEATTRIGGER 20

ShapeEffect::ShapeEffect(int id) : RenderableEffect(id, "Shape", shape_16, shape_24, shape_32, shape_48, shape_64)
{
    //ctor
}

ShapeEffect::~ShapeEffect()
{
    //dtor
}

std::list<std::string> ShapeEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    if (media == nullptr && settings.GetBool("E_CHECKBOX_Shape_UseMusic", false)) {
        res.push_back(std::format("    WARN: Shape effect cant grow to music if there is no music. Model '{}', Start {}", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    }

    std::string object = settings["E_CHOICE_Shape_ObjectToDraw"];
    if (object == "SVG") {
        auto svgFilename = settings.Get("E_FILEPICKERCTRL_SVG", "");

        if (svgFilename.empty()) {
            res.push_back(std::format("    ERR: Shape effect cant find SVG file '{}'. Model '{}', Start {}", svgFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
        } else {
            auto& mm = eff->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetSequenceMedia();
            auto svgEntry = mm.GetSVG(svgFilename);
            if (svgEntry->GetSVGContent().empty()) {
                res.push_back(std::format("    ERR: Shape effect cant find SVG file '{}'. Model '{}', Start {}", svgFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
            } else {
                if (!svgEntry->IsEmbedded()) {
                    if (!IsFileInShowDir(std::string(), svgFilename)) {
                        res.push_back(std::format("    WARN: Shape effect SVG file '{}' not under show directory. Model '{}', Start {}", svgFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
                    }
                }
            }
        }
    }

    return res;
}

std::list<std::string> ShapeEffect::GetFileReferences(Model* model, const SettingsMap& SettingsMap) const
{
    std::list<std::string> res;
    if (SettingsMap["E_FILEPICKERCTRL_SVG"] != "") {
        res.push_back(SettingsMap["E_FILEPICKERCTRL_SVG"]);
    }
    return res;
}

bool ShapeEffect::CleanupFileLocations(xLightsFrame* frame, SettingsMap& SettingsMap)
{
    bool rc = false;
    std::string file = SettingsMap["E_FILEPICKERCTRL_SVG"];
    if (FileExists(file)) {
        if (!frame->IsInShowFolder(file)) {
            SettingsMap["E_FILEPICKERCTRL_SVG"] = frame->MoveToShowFolder(file, std::string(1, std::filesystem::path::preferred_separator) + "Images");
            rc = true;
        }
    }

    return rc;
}

void ShapeEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect)
{
    std::string timing = effect->GetSettings().Get("E_CHOICE_Shape_FireTimingTrack", "");

    if (timing == oldname)
    {
        effect->GetSettings()["E_CHOICE_Shape_FireTimingTrack"] = newname;
    }
}

#define RENDER_SHAPE_CIRCLE     0
#define RENDER_SHAPE_SQUARE     1
#define RENDER_SHAPE_TRIANGLE   2
#define RENDER_SHAPE_STAR       3
#define RENDER_SHAPE_PENTAGON   4
#define RENDER_SHAPE_HEXAGON    5
#define RENDER_SHAPE_OCTAGON    6
#define RENDER_SHAPE_HEART      7
#define RENDER_SHAPE_TREE       8
#define RENDER_SHAPE_CANDYCANE  9
#define RENDER_SHAPE_SNOWFLAKE  10
#define RENDER_SHAPE_CRUCIFIX   11
#define RENDER_SHAPE_PRESENT    12
#define RENDER_SHAPE_ELLIPSE    13
#define RENDER_SHAPE_EMOJI 14
#define RENDER_SHAPE_SVG 15


struct ShapeData
{
private:
    xlColor _color;
public:
    xlPoint _centre;
    xlPoint _movement;
    float _size;
    int _oset;
    int _shape;
    float _angle;
    int _speed;
    int _colourIndex;
    bool _holdColour;

    ShapeData(xlPoint centre, float size, int oset, xlColor color, int shape, int angle, int speed, bool holdColour, int colourIndex)
    {
        _holdColour = holdColour;
        _colourIndex = colourIndex;
        _centre = centre;
        _movement.x = 0;
        _movement.y = 0;
        _size = size;
        _oset = oset;
        _color = color;
        _shape = shape;
        _angle = toRadians(angle);
        _speed = speed;
    }

    xlColor GetColour(const PaletteClass& palette)
    {
        if (_holdColour)
        {
            return _color;
        }
        else
        {
            return palette.GetColor(_colourIndex);
        }
    }

    void Move()
    {
        int x = _speed * cos(_angle);
        int y = _speed * sin(_angle);
        _movement.x += x;
        _movement.y += y;
        _centre.x += x;
        _centre.y += y;
    }

    void SetCentre(xlPoint centre)
    {
        _centre = centre;
        _centre.x += _movement.x;
        _centre.y += _movement.y;
    }
};

bool compare_shapes(const ShapeData* first, const ShapeData* second)
{
    return first->_oset > second->_oset;
}

class ShapeRenderCache : public EffectRenderCache {

public:
    ShapeRenderCache() { _lastColorIdx = -1; _sinceLastTriggered = 0; }
    virtual ~ShapeRenderCache()
    {
        DeleteShapes();
        if (_svgImage != nullptr) {
            nsvgDelete(_svgImage);
            _svgImage = nullptr;
        }
        if (_svgRasterizer != nullptr) {
            nsvgDeleteRasterizer(_svgRasterizer);
            _svgRasterizer = nullptr;
        }
    }

    std::list<ShapeData*> _shapes;
    int _lastColorIdx = 0;
    int _sinceLastTriggered = 0;
    TextFontInfo _font;
    NSVGimage* _svgImage = nullptr;
    NSVGrasterizer* _svgRasterizer = nullptr;
    std::vector<uint8_t> _rasterBuf;
    std::string _svgFilename;
    float _svgScaleBase = 1.0f;
    std::string _filterLabel;
    std::regex _filterRegex;
    bool _useRegex;

    void SetFilter(const std::string& filterLabel, bool useRegex)
    {
        _filterLabel = filterLabel;
        _useRegex = useRegex;
        if (useRegex) {
            try {
                _filterRegex = std::regex(filterLabel, std::regex_constants::extended);
            } catch(std::exception&) {
                _useRegex = false;
            }
        }
    }

    bool IsLabelAMatch(const std::string& label)
    {
        if (_filterLabel == "")
            return true;

        if (_useRegex)
            return std::regex_search(label, _filterRegex);

        // tokenise the label and then check if any match the filter
        const std::string tokens = ": ;,";
        char n[4096] = { 0 };
        strncpy(n, label.c_str(), sizeof(n) - 1);
        const char* token = strtok(n, tokens.c_str());
        while (token != nullptr) {
            if (_filterLabel == token)
                return true;
            token = strtok(nullptr, tokens.c_str());
        }

        return false;
    }

    void InitialiseSVG(const std::string filename, RenderBuffer& buffer)
    {
        if (_svgImage != nullptr) {
            nsvgDelete(_svgImage);
            _svgImage = nullptr;
        }

        _svgFilename = filename;
        auto* seqMedia = buffer.GetSequenceMedia();
        if (seqMedia) {
            auto svgEntry = seqMedia->GetSVG(filename);
            if (svgEntry) {
                svgEntry->MarkIsUsed();
                std::string content = svgEntry->GetSVGContent();
                if (!content.empty()) {
                    char* svgCopy = strdup(content.c_str());
                    _svgImage = nsvgParse(svgCopy, "px", 96);
                    free(svgCopy);
                }
            }
        }
        if (_svgImage != nullptr) {
            auto max = std::max(_svgImage->height, _svgImage->width);
            _svgScaleBase = 1.0f / (float)max;
        }
    }

    void AddShape(xlPoint centre, float size, xlColor color, int oset, int shape, int angle, int speed, bool randomMovement, bool holdColour, int colourIndex)
    {
        if (randomMovement)
        {
            speed = rand01() * (SHAPE_VELOCITY_MAX - SHAPE_VELOCITY_MIN) - SHAPE_VELOCITY_MIN;
            angle = rand01() * (SHAPE_DIRECTION_MAX - SHAPE_DIRECTION_MIN) - SHAPE_VELOCITY_MIN;
        }
        _shapes.push_back(new ShapeData(centre, size, oset, color, shape, angle, speed, holdColour, colourIndex));
    }

    NSVGimage* GetImage()
    {
        return _svgImage;
    }

    NSVGrasterizer* GetRasterizer()
    {
        if (_svgRasterizer == nullptr) {
            _svgRasterizer = nsvgCreateRasterizer();
        }
        return _svgRasterizer;
    }

    void DeleteShapes()
    {
        while (_shapes.size() > 0)
        {
            auto todelete = _shapes.front();
            _shapes.pop_front();
            delete todelete;
        }
    }
    void RemoveOld(int maxAge)
    {
        // old are always at the front of the list
        while (_shapes.size() > 0 && _shapes.front()->_oset > maxAge)
        {
            auto todelete = _shapes.front();
            _shapes.pop_front();
            delete todelete;
        }
    }
    void SortShapes()
    {
        _shapes.sort(compare_shapes);
    }
};

int ShapeEffect::DecodeShape(const std::string& shape)
{
    if (shape == "Circle") {
        return RENDER_SHAPE_CIRCLE;
    } else if (shape == "Square") {
        return RENDER_SHAPE_SQUARE;
    } else if (shape == "Triangle") {
        return RENDER_SHAPE_TRIANGLE;
    } else if (shape == "Star") {
        return RENDER_SHAPE_STAR;
    } else if (shape == "Pentagon") {
        return RENDER_SHAPE_PENTAGON;
    } else if (shape == "Hexagon") {
        return RENDER_SHAPE_HEXAGON;
    } else if (shape == "Heart") {
        return RENDER_SHAPE_HEART;
    } else if (shape == "Tree") {
        return RENDER_SHAPE_TREE;
    } else if (shape == "Octagon") {
        return RENDER_SHAPE_OCTAGON;
    } else if (shape == "Candy Cane") {
        return RENDER_SHAPE_CANDYCANE;
    } else if (shape == "Snowflake") {
        return RENDER_SHAPE_SNOWFLAKE;
    } else if (shape == "Crucifix") {
        return RENDER_SHAPE_CRUCIFIX;
    } else if (shape == "Present") {
        return RENDER_SHAPE_PRESENT;
    } else if (shape == "Ellipse") {
        return RENDER_SHAPE_ELLIPSE;
    }
    // this must be the last as we dont want to randomly select it
    else if (shape == "Emoji") {
        return RENDER_SHAPE_EMOJI;
    } else if (shape == "SVG") {
        return RENDER_SHAPE_SVG;
    }

    return rand01() * 13; // exclude emoji
}

static int mapSkinTone(const std::string &v) {
    if (v == "Light") {
        return 0x1F3FB;
    } else if (v == "Medium Light") {
        return 0x1F3FC;
    } else if (v == "Medium") {
        return 0x1F3FD;
    } else if (v == "Medium Dark") {
        return 0x1F3FE;
    } else if (v == "Dark") {
        return 0x1F3FF;
    }
    return 0;
}

void ShapeEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
	float oset = buffer.GetEffectTimeIntervalPosition();

	std::string Object_To_DrawStr = SettingsMap["CHOICE_Shape_ObjectToDraw"];
    int thickness = GetValueCurveInt("Shape_Thickness", 1, SettingsMap, oset, SHAPE_THICKNESS_MIN, SHAPE_THICKNESS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int points = SettingsMap.GetInt("SLIDER_Shape_Points", 5);
    bool randomLocation = SettingsMap.GetBool("CHECKBOX_Shape_RandomLocation", true);
    bool fadeAway = SettingsMap.GetBool("CHECKBOX_Shape_FadeAway", true);
    bool startRandomly = SettingsMap.GetBool("CHECKBOX_Shape_RandomInitial", true);
    bool holdColour = SettingsMap.GetBool("CHECKBOX_Shape_HoldColour", true);
    int xc = GetValueCurveInt("Shape_CentreX", 50, SettingsMap, oset, SHAPE_CENTREX_MIN, SHAPE_CENTREX_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) * buffer.BufferWi / 100;
    int yc = GetValueCurveInt("Shape_CentreY", 50, SettingsMap, oset, SHAPE_CENTREY_MIN, SHAPE_CENTREY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) * buffer.BufferHt / 100;
    int lifetime = GetValueCurveInt("Shape_Lifetime", 5, SettingsMap, oset, SHAPE_LIFETIME_MIN, SHAPE_LIFETIME_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int growth = GetValueCurveInt("Shape_Growth", 10, SettingsMap, oset, SHAPE_GROWTH_MIN, SHAPE_GROWTH_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int count = GetValueCurveInt("Shape_Count", 5, SettingsMap, oset, SHAPE_COUNT_MIN, SHAPE_COUNT_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int startSize = GetValueCurveInt("Shape_StartSize", 5, SettingsMap, oset, SHAPE_STARTSIZE_MIN, SHAPE_STARTSIZE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int emoji = SettingsMap.GetInt("SPINCTRL_Shape_Char", 65);
    int emojiTone = 0;
    std::string font = SettingsMap["FONTPICKER_Shape_Font"];
    std::string svgFilename = SettingsMap["FILEPICKERCTRL_SVG"];
    int direction = GetValueCurveInt("Shapes_Direction", 90, SettingsMap, oset, SHAPE_DIRECTION_MIN, SHAPE_DIRECTION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int velocity = GetValueCurveInt("Shapes_Velocity", 0, SettingsMap, oset, SHAPE_VELOCITY_MIN, SHAPE_VELOCITY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool randomMovement = SettingsMap.GetBool("CHECKBOX_Shapes_RandomMovement", false);
    bool useRegex = SettingsMap.GetBool("CHECKBOX_Shape_FilterReg", false);
    std::string filterLabel = SettingsMap.Get("TEXTCTRL_Shape_FilterLabel", "");

    int rotation = GetValueCurveInt("Shape_Rotation", 0, SettingsMap, oset, SHAPE_ROTATION_MIN, SHAPE_ROTATION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    int Object_To_Draw = DecodeShape(Object_To_DrawStr);

    float f = 0.0;
    bool useMusic = SettingsMap.GetBool("CHECKBOX_Shape_UseMusic", false);
    float sensitivity = (float)SettingsMap.GetInt("SLIDER_Shape_Sensitivity", 50) / 100.0;
    bool useTiming = SettingsMap.GetBool("CHECKBOX_Shape_FireTiming", false);
    std::string timing = SettingsMap.Get("CHOICE_Shape_FireTimingTrack", "");
    if (timing == "") useTiming = false;
    if (useMusic) {
        if (buffer.GetMedia() != nullptr) {
            auto pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, "");
            if (pf != nullptr) {
                f = pf->max;
            }
        }
    }

    ShapeRenderCache *cache = (ShapeRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new ShapeRenderCache();
        buffer.infoCache[id] = cache;
    }

    std::list<ShapeData*>& _shapes = cache->_shapes;
    int& _lastColorIdx = cache->_lastColorIdx;
    int& _sinceLastTriggered = cache->_sinceLastTriggered;
    TextFontInfo& _font = cache->_font;

    float lifetimeFrames = (float)(buffer.curEffEndPer - buffer.curEffStartPer) * lifetime / 100.0;
    if (lifetimeFrames < 1) lifetimeFrames = 1;
    float growthPerFrame = (float)growth / lifetimeFrames;

    if (buffer.needToInit) {
        buffer.needToInit = false;

        cache->SetFilter(filterLabel, useRegex);

        _sinceLastTriggered = 0;

        if (Object_To_Draw == RENDER_SHAPE_EMOJI) {
            _font = TextDrawingContext::GetShapeFont(font);
        } else if (Object_To_Draw == RENDER_SHAPE_SVG) {
            cache->InitialiseSVG(svgFilename, buffer);
        }

        cache->DeleteShapes();
        _lastColorIdx = -1;

        if (!useTiming && !useMusic) {
            for (int i = _shapes.size(); i < count; ++i) {
                xlPoint pt;
                if (randomLocation) {
                    pt = xlPoint(rand01() * buffer.BufferWi, rand01() * buffer.BufferHt);
                } else {
                    pt = xlPoint(xc, yc);
                }

                int colorcnt = (int)buffer.GetColorCount();
                _lastColorIdx++;
                if (_lastColorIdx >= colorcnt) {
                    _lastColorIdx = 0;
                }

                int os = 0;
                if (startRandomly)
                {
                    os = rand01() * lifetimeFrames;
                }

                cache->AddShape(pt, startSize + os * growthPerFrame, buffer.palette.GetColor(_lastColorIdx), os, Object_To_Draw, direction, velocity, randomMovement, holdColour, _lastColorIdx);
            }
            cache->SortShapes();
        }
    }

    // create missing shapes
    if (useTiming) {
        if (mSequenceElements == nullptr) {
            // no timing tracks ... this shouldnt happen
        } else {
            // Load the names of the timing tracks
            Element* t = nullptr;
            for (size_t l = 0; l < mSequenceElements->GetElementCount(); l++)
            {
                Element* e = mSequenceElements->GetElement(l);
                if (e->GetEffectLayerCount() == 1 && e->GetType() == ElementType::ELEMENT_TYPE_TIMING)
                {
                    if (e->GetName() == timing)
                    {
                        t = e;
                        break;
                    }
                }
            }

            if (t == nullptr)
            {
                // timing track not found ... this shouldnt happen
            }
            else
            {
                _sinceLastTriggered = 0;
                EffectLayer* el = t->GetEffectLayer(0);
                for (int j = 0; j < el->GetEffectCount(); j++)
                {
                    Effect* ef = el->GetEffect(j);
                    if (buffer.curPeriod == ef->GetStartTimeMS() / buffer.frameTimeInMs && cache->IsLabelAMatch(ef->GetEffectName()))
                    {
                        xlPoint pt;
                        if (randomLocation)
                        {
                            pt = xlPoint(rand01() * buffer.BufferWi, rand01() * buffer.BufferHt);
                        }
                        else
                        {
                            pt = xlPoint(xc, yc);
                        }

                        int colorcnt = (int)buffer.GetColorCount();
                        _lastColorIdx++;
                        if (_lastColorIdx >= colorcnt)
                        {
                            _lastColorIdx = 0;
                        }

                        cache->AddShape(pt, startSize, buffer.palette.GetColor(_lastColorIdx), 0, Object_To_Draw, direction, velocity, randomMovement, holdColour, _lastColorIdx);
                        break;
                    }
                }
            }
        }
    }
    else if (useMusic)
    {
        // only trigger a firework if music is greater than the sensitivity
        if (f > sensitivity)
        {
            // trigger if it was not previously triggered or has been triggered for REPEATTRIGGER frames
            if (_sinceLastTriggered == 0 || _sinceLastTriggered > REPEATTRIGGER)
            {
                xlPoint pt;
                if (randomLocation)
                {
                    pt = xlPoint(rand01() * buffer.BufferWi, rand01() * buffer.BufferHt);
                }
                else
                {
                    pt = xlPoint(xc, yc);
                }

                int colorcnt = (int)buffer.GetColorCount();
                _lastColorIdx++;
                if (_lastColorIdx >= colorcnt)
                {
                    _lastColorIdx = 0;
                }

                cache->AddShape(pt, startSize, buffer.palette.GetColor(_lastColorIdx), 0, Object_To_Draw, direction, velocity, randomMovement, holdColour, _lastColorIdx);
            }

            // if music is over the trigger level for REPEATTRIGGER frames then we will trigger another firework
            _sinceLastTriggered++;
            if (_sinceLastTriggered > REPEATTRIGGER)
            {
                _sinceLastTriggered = 0;
            }
        }
        else
        {
            // not triggered so clear last triggered counter
            _sinceLastTriggered = 0;
        }
    }
    else
    {
        for (int i = _shapes.size(); i < count; ++i)
        {
            xlPoint pt;
            if (randomLocation)
            {
                pt = xlPoint(rand01() * buffer.BufferWi, rand01() * buffer.BufferHt);
            }
            else
            {
                pt = xlPoint(xc, yc);
            }

            int colorcnt = (int)buffer.GetColorCount();
            _lastColorIdx++;
            if (_lastColorIdx >= colorcnt)
            {
                _lastColorIdx = 0;
            }

            cache->AddShape(pt, startSize, buffer.palette.GetColor(_lastColorIdx), 0, Object_To_Draw, direction, velocity, randomMovement, holdColour, _lastColorIdx);
        }
    }

    if (Object_To_Draw == RENDER_SHAPE_EMOJI || Object_To_Draw == RENDER_SHAPE_SVG) {
        if (buffer.BufferWi <= 0 || buffer.BufferHt <= 0 || buffer.BufferWi > 15000) {
            
            spdlog::error("Shape Effect (Emoji/SVG): Invalid buffer size: width={}, height={}", buffer.BufferWi, buffer.BufferHt);
            return;
        }
        auto context = buffer.GetTextDrawingContext();
        context->Clear();
    }

    for (const auto& it : _shapes) {
        // if location is not random then update it to whatever the current location is
        // as it may be value curve controlled
        if (!randomLocation)
        {
            it->SetCentre(xlPoint(xc, yc));
        }

        xlColor color = it->GetColour(buffer.palette);

        if (fadeAway)
        {
            float brightness = (float)(lifetimeFrames - it->_oset) / lifetimeFrames;

            // draw text does not respect alpha
            if (buffer.allowAlpha && Object_To_Draw != RENDER_SHAPE_EMOJI) {
                color.alpha = 255.0 * brightness;
            } else {
                color.red = color.red * brightness;
                color.green = color.green * brightness;
                color.blue = color.blue * brightness;
            }
        }

        switch (it->_shape)
        {
        case RENDER_SHAPE_SQUARE:
            Drawpolygon(buffer, it->_centre.x, it->_centre.y, it->_size, 4, color, thickness, rotation + 45.0);
            break;
        case RENDER_SHAPE_CIRCLE:
            Drawcircle(buffer, it->_centre.x, it->_centre.y, it->_size, color, thickness);
            break;
        case RENDER_SHAPE_STAR:
            Drawstar(buffer, it->_centre.x, it->_centre.y, it->_size, points, color, thickness, rotation);
            break;
        case RENDER_SHAPE_TRIANGLE:
            Drawpolygon(buffer, it->_centre.x, it->_centre.y, it->_size, 3, color, thickness, rotation + 90.0);
            break;
        case RENDER_SHAPE_PENTAGON:
            Drawpolygon(buffer, it->_centre.x, it->_centre.y, it->_size, 5, color, thickness, rotation + 90.0);
            break;
        case RENDER_SHAPE_HEXAGON:
            Drawpolygon(buffer, it->_centre.x, it->_centre.y, it->_size, 6, color, thickness, rotation);
            break;
        case RENDER_SHAPE_OCTAGON:
            Drawpolygon(buffer, it->_centre.x, it->_centre.y, it->_size, 8, color, thickness, rotation + 22.5);
            break;
        case RENDER_SHAPE_TREE:
            Drawtree(buffer, it->_centre.x, it->_centre.y, it->_size, color, thickness, rotation);
            break;
        case RENDER_SHAPE_CRUCIFIX:
            Drawcrucifix(buffer, it->_centre.x, it->_centre.y, it->_size, color, thickness, rotation);
            break;
        case RENDER_SHAPE_PRESENT:
            Drawpresent(buffer, it->_centre.x, it->_centre.y, it->_size, color, thickness, rotation);
            break;
        case RENDER_SHAPE_EMOJI:
            emojiTone = mapSkinTone(SettingsMap["CHOICE_Shape_SkinTone"]);
            Drawemoji(buffer, it->_centre.x, it->_centre.y, it->_size, color, emoji, emojiTone, _font);
            break;
        case RENDER_SHAPE_SVG:
            DrawSVG(cache, buffer, it->_centre.x, it->_centre.y, it->_size, color, thickness);
            break;
        case RENDER_SHAPE_CANDYCANE:
            Drawcandycane(buffer, it->_centre.x, it->_centre.y, it->_size, color, thickness);
            break;
        case RENDER_SHAPE_SNOWFLAKE:
            Drawsnowflake(buffer, it->_centre.x, it->_centre.y, it->_size, 3, color, rotation + 30);
            break;
        case RENDER_SHAPE_HEART:
            Drawheart(buffer, it->_centre.x, it->_centre.y, it->_size, color, thickness, rotation);
            break;
		case RENDER_SHAPE_ELLIPSE:
			Drawellipse(buffer, it->_centre.x, it->_centre.y, it->_size, points, color, thickness, rotation);
			break;
        default:
            assert(false);
            break;
        }

        // move etc after drawing otherwise first frame has already moved
        it->Move();
        it->_oset++;
        it->_size += growthPerFrame;

        if (it->_size < 0) it->_size = 0;
    }

    if (Object_To_Draw == RENDER_SHAPE_EMOJI) {
        int w, h;
        const uint8_t* rgba = buffer.GetTextDrawingContext()->FlushAndGetImage(&w, &h);
        int cur = 0;
        xlColor c, c2;
        for (int y = h - 1; y >= 0; y--) {
            for (int x = 0; x < w; x++) {
                c.Set(rgba[cur], rgba[cur + 1], rgba[cur + 2], rgba[cur + 3]);
                buffer.GetPixel(x, y, c2);
                if (c2 != xlBLACK) {
                    c.AlphaBlend(c2);
                }
                buffer.SetPixel(x, y, c);
                cur += 4;
            }
        }
    }

    cache->RemoveOld(lifetimeFrames);
}

void ShapeEffect::Drawcircle(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness) const
{
    double interpolation = 0.75;
    double t = (double)thickness - 1.0 + interpolation;
    for (double i = 0; i < t; i += interpolation)
    {
        if (radius >= 0)
        {
            for (double degrees = 0.0; degrees < 360.0; degrees += 1.0)
            {
                double radian = degrees * (M_PI / 180.0);
                int x = std::round(radius * buffer.cos(radian)) + xc;
                int y = std::round(radius * buffer.sin(radian)) + yc;
                buffer.SetPixel(x, y, color);
            }
        }
        else
        {
            break;
        }
        radius -= interpolation;
    }
}

void ShapeEffect::Drawellipse(RenderBuffer& buffer, int xc, int yc, double radius, int multipler, xlColor color, int thickness, double rotation) const
{
	double interpolation = 0.75;
	double t = (double)thickness - 1.0 + interpolation;
	for (double i = 0; i < t; i += interpolation)
	{
		if (radius >= 0)
		{
			for (double degrees = 0.0; degrees < 360.0; degrees += 1.0)
			{
				double radian = (degrees) * (M_PI / 180.0);
				double scaleMul = ((double)multipler/10.0);
				double x = radius * cos(radian);
				double y = (radius * scaleMul) * sin(radian);

				//now rotation
				double radRot = (rotation) * (M_PI / 180.0);
				double rx = (x * cos(radRot)) - (y * sin(radRot));
				double ry = (y * cos(radRot)) + (x * sin(radRot));

				int xx = std::round(rx) + xc;
				int yy = std::round(ry) + yc;

				buffer.SetPixel(xx, yy, color);
			}
		}
		else
		{
			break;
		}
		radius -= interpolation;
	}
}

void ShapeEffect::Drawstar(RenderBuffer &buffer, int xc, int yc, double radius, int points, xlColor color, int thickness, double rotation) const
{
    double interpolation = 0.6;
    double t = (double)thickness - 1.0 + interpolation;
    double offsetangle = 0.0;
    switch (points)
    {
    case 5:
        offsetangle = 90.0 - 360.0 / 5.0;
        break;
    case 6:
        offsetangle = 30.0;
        break;
    case 7:
        offsetangle = 90.0 - 360.0 / 7.0;
        break;
    default:
        break;
    }

    for (double i = 0; i < t; i += interpolation)
    {
        if (radius >= 0)
        {
            double InnerRadius = radius / 2.618034;    // divide by golden ratio squared

            double increment = 360.0 / points;

            for (double degrees = 0.0; degrees < 361.0; degrees += increment) // 361 because it allows for small rounding errors
            {
                if (degrees > 360.0) degrees = 360.0;
                double radian = (rotation + offsetangle + degrees) * (M_PI / 180.0);
                int xouter = std::round(radius * buffer.cos(radian)) + xc;
                int youter = std::round(radius * buffer.sin(radian)) + yc;

                radian = (rotation + offsetangle + degrees + increment / 2.0) * (M_PI / 180.0);
                int xinner = std::round(InnerRadius * buffer.cos(radian)) + xc;
                int yinner = std::round(InnerRadius * buffer.sin(radian)) + yc;

                buffer.DrawLine(xinner, yinner, xouter, youter, color);

                radian = (rotation + offsetangle + degrees - increment / 2.0) * (M_PI / 180.0);
                xinner = std::round(InnerRadius * buffer.cos(radian)) + xc;
                yinner = std::round(InnerRadius * buffer.sin(radian)) + yc;

                buffer.DrawLine(xinner, yinner, xouter, youter, color);

                if (degrees == 360.0) degrees = 361.0;
            }
        }
        else
        {
            break;
        }
        radius -= interpolation;
    }
}

void ShapeEffect::Drawpolygon(RenderBuffer &buffer, int xc, int yc, double radius, int sides, xlColor color, int thickness, double rotation) const
{
    double interpolation = 0.05;
    double t = (double)thickness - 1.0 + interpolation;
    double increment = 360.0 / sides;

    for (double i = 0; i < t; i += interpolation)
    {
        if (radius >= 0)
        {
            for (double degrees = 0.0; degrees < 361.0; degrees += increment) // 361 because it allows for small rounding errors
            {
                if (degrees > 360.0) degrees = 360.0;
                double radian = (rotation + degrees) * M_PI / 180.0;
                int x1 = std::round(radius * cos(radian)) + xc;
                int y1 = std::round(radius * sin(radian)) + yc;

                radian = (rotation + degrees + increment) * M_PI / 180.0;
                int x2 = std::round(radius * cos(radian)) + xc;
                int y2 = std::round(radius * sin(radian)) + yc;

                buffer.DrawLine(x1, y1, x2, y2, color);

                if (degrees == 360.0) degrees = 361.0;
            }
        }
        else
        {
            break;
        }
        radius -= interpolation;
    }
}

void ShapeEffect::Drawsnowflake(RenderBuffer &buffer, int xc, int yc, double radius, int sides, xlColor color, double rotation) const
{
    double increment = 360.0 / (sides * 2);
    double angle = rotation;

    if (radius >= 0)
    {
        for (int i = 0; i < sides * 2; i++)
        {
            double radian = angle * M_PI / 180.0;

            int x1 = std::round(radius * cos(radian)) + xc;
            int y1 = std::round(radius * sin(radian)) + yc;

            radian = (180 + angle) * M_PI / 180.0;

            int x2 = std::round(radius * cos(radian)) + xc;
            int y2 = std::round(radius * sin(radian)) + yc;

            buffer.DrawLine(x1, y1, x2, y2, color);

            angle += increment;
        }
    }
}

void ShapeEffect::Drawheart(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness, double rotation) const
{
    double interpolation = 0.75;
    double t = (double)thickness - 1.0 + interpolation;
    double radRot = (rotation) * (M_PI / 180.0);

    double xincr = 0.01;
    for (double x = -2.0; x <= 2.0; x += xincr)
    {
        double y1 = std::sqrt(1.0 - (std::abs(x) - 1.0) * (std::abs(x) - 1.0));
        double y2 = std::acos(1.0 - std::abs(x)) - M_PI;

        double r = radius;

        for (double i = 0.0; i < t; i += interpolation)
        {
            if (r >= 0.0)
            {
				double xx = (x * r) / 2.0;
				double yy1 = (y1 * r) / 2.0;
				double yy2 = (y2 * r) / 2.0;

				//now rotation
				double rx1 = (xx * cos(radRot)) - (yy1 * sin(radRot)) + xc;
				double ry1 = (yy1 * cos(radRot)) + (xx * sin(radRot)) + yc;
				double rx2 = (xx * cos(radRot)) - (yy2 * sin(radRot)) + xc;
				double ry2 = (yy2 * cos(radRot)) + (xx * sin(radRot)) + yc;

                buffer.SetPixel(std::round(rx1), std::round(ry1), color);
                buffer.SetPixel(std::round(rx2), std::round(ry2), color);

                if (x + xincr > 2.0 || x == -2.0 + xincr) {
                    if (yy1 > yy2)
                        std::swap(yy1, yy2);
                    for (double z = yy1; z < yy2; z += 0.5) {
                        // we have to rotate here as well
                        double rx1 = (xx * cos(radRot)) - (z * sin(radRot)) + xc;
                        double ry1 = (z * cos(radRot)) + (xx * sin(radRot)) + yc;
                        buffer.SetPixel(std::round(rx1), std::round(ry1), color);
                    }
                }
            }
            else
            {
                break;
            }
            r -= interpolation;
        }
    }
}

void ShapeEffect::Drawtree(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness, double rotation) const
{
    struct line
    {
        xlPoint start;
        xlPoint end;

        line(const xlPoint s, const xlPoint e)
        {
            start = s;
            end = e;
        }
    };

    const line points[] = {line(xlPoint(3,0), xlPoint(5,0)),
                           line(xlPoint(5,0), xlPoint(5,3)),
                           line(xlPoint(3,0), xlPoint(3,3)),
                           line(xlPoint(0,3), xlPoint(8,3)),
                           line(xlPoint(0,3), xlPoint(2,6)),
                           line(xlPoint(8,3), xlPoint(6,6)),
                           line(xlPoint(1,6), xlPoint(2,6)),
                           line(xlPoint(6,6), xlPoint(7,6)),
                           line(xlPoint(1,6), xlPoint(3,9)),
                           line(xlPoint(7,6), xlPoint(5,9)),
                           line(xlPoint(2,9), xlPoint(3,9)),
                           line(xlPoint(5,9), xlPoint(6,9)),
                           line(xlPoint(6,9), xlPoint(4,11)),
                           line(xlPoint(2,9), xlPoint(4,11))
    };
    int count = sizeof(points) / sizeof(line);

    double interpolation = 0.75;
    double t = (double)thickness - 1.0 + interpolation;

    for (double i = 0; i < t; i += interpolation)
    {
        if (radius >= 0)
        {
            for (int j = 0; j < count; ++j)
            {
                int x1 = std::round(((double)points[j].start.x - 4.0) / 11.0 * radius);
                int y1 = std::round(((double)points[j].start.y - 4.0) / 11.0 * radius);
                int x2 = std::round(((double)points[j].end.x - 4.0) / 11.0 * radius);
                int y2 = std::round(((double)points[j].end.y - 4.0) / 11.0 * radius);

				//now rotation
				double radRot = (rotation) * (M_PI / 180.0);
				double rx1 = (x1 * cos(radRot)) - (y1 * sin(radRot));
				double ry1 = (y1 * cos(radRot)) + (x1 * sin(radRot));
				double rx2 = (x2 * cos(radRot)) - (y2 * sin(radRot));
				double ry2 = (y2 * cos(radRot)) + (x2 * sin(radRot));
                buffer.DrawLine(xc + rx1, yc + ry1, xc + rx2, yc + ry2, color);
            }
        }
        else
        {
            break;
        }
        radius -= interpolation;
    }
}

void ShapeEffect::Drawcrucifix(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness, double rotation) const
{
    struct line
    {
        xlPoint start;
        xlPoint end;

        line(const xlPoint s, const xlPoint e)
        {
            start = s;
            end = e;
        }
    };

    const line points[] = {line(xlPoint(2,0), xlPoint(2,6)),
                           line(xlPoint(2,6), xlPoint(0,6)),
                           line(xlPoint(0,6), xlPoint(0,7)),
                           line(xlPoint(0,7), xlPoint(2,7)),
                           line(xlPoint(2,7), xlPoint(2,10)),
                           line(xlPoint(2,10), xlPoint(3,10)),
                           line(xlPoint(3,10), xlPoint(3,7)),
                           line(xlPoint(3,7), xlPoint(5,7)),
                           line(xlPoint(5,7), xlPoint(5,6)),
                           line(xlPoint(5,6), xlPoint(3,6)),
                           line(xlPoint(3,6), xlPoint(3,0)),
                           line(xlPoint(3,0), xlPoint(2,0))
    };
    int count = sizeof(points) / sizeof(line);

    double interpolation = 0.75;
    double t = (double)thickness - 1.0 + interpolation;

    for (double i = 0; i < t; i += interpolation)
    {
        if (radius >= 0)
        {
            for (int j = 0; j < count; ++j)
            {
                int x1 = std::round(((double)points[j].start.x - 2.5) / 7.0 * radius);
                int y1 = std::round(((double)points[j].start.y - 6.5) / 10.0 * radius);
                int x2 = std::round(((double)points[j].end.x - 2.5) / 7.0 * radius);
                int y2 = std::round(((double)points[j].end.y - 6.5) / 10.0 * radius);

				//now rotation
				double radRot = (rotation) * (M_PI / 180.0);
				double rx1 = (x1 * cos(radRot)) - (y1 * sin(radRot));
				double ry1 = (y1 * cos(radRot)) + (x1 * sin(radRot));
				double rx2 = (x2 * cos(radRot)) - (y2 * sin(radRot));
				double ry2 = (y2 * cos(radRot)) + (x2 * sin(radRot));
                buffer.DrawLine(xc + rx1, yc + ry1, xc + rx2, yc + ry2, color);
            }
        }
        else
        {
            break;
        }
        radius -= interpolation;
    }
}

void ShapeEffect::Drawpresent(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness, double rotation) const
{
    struct line {
        xlPoint start;
        xlPoint end;

        line(const xlPoint s, const xlPoint e)
        {
            start = s;
            end = e;
        }
    };

    const line points[] = { line(xlPoint(0, 0), xlPoint(0, 9)),
                            line(xlPoint(0, 9), xlPoint(10, 9)),
                            line(xlPoint(10, 9), xlPoint(10, 0)),
                            line(xlPoint(10, 0), xlPoint(0, 0)),
                            line(xlPoint(5, 0), xlPoint(5, 9)),
                            line(xlPoint(5, 9), xlPoint(2, 11)),
                            line(xlPoint(2, 11), xlPoint(2, 9)),
                            line(xlPoint(5, 9), xlPoint(8, 11)),
                            line(xlPoint(8, 11), xlPoint(8, 9)) };
    int count = sizeof(points) / sizeof(line);

    double interpolation = 0.75;
    double t = (double)thickness - 1.0 + interpolation;

    for (double i = 0; i < t; i += interpolation) {
        if (radius >= 0) {
            for (int j = 0; j < count; ++j) {
                int x1 = std::round(((double)points[j].start.x - 5) / 7.0 * radius);
                int y1 = std::round(((double)points[j].start.y - 5.5) / 10.0 * radius);
                int x2 = std::round(((double)points[j].end.x - 5) / 7.0 * radius);
                int y2 = std::round(((double)points[j].end.y - 5.5) / 10.0 * radius);

                // now rotation
                double radRot = (rotation) * (M_PI / 180.0);
                double rx1 = (x1 * cos(radRot)) - (y1 * sin(radRot));
                double ry1 = (y1 * cos(radRot)) + (x1 * sin(radRot));
                double rx2 = (x2 * cos(radRot)) - (y2 * sin(radRot));
                double ry2 = (y2 * cos(radRot)) + (x2 * sin(radRot));

                buffer.DrawLine(xc + rx1, yc + ry1, xc + rx2, yc + ry2, color);
            }
        } else {
            break;
        }
        radius -= interpolation;
    }
}

bool ShapeEffect::needToAdjustSettings(const std::string& version)
{
    // give the base class a chance to adjust any settings
    return IsVersionOlder("2024.02", version);
}

void ShapeEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version)) {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }
    SettingsMap& settings = effect->GetSettings();
    if (settings.Contains("E_CHOICE_Shape_ObjectToDraw")) {
        if (settings["E_CHOICE_Shape_ObjectToDraw"] == "Emoji") {
            std::string val = settings.Get("E_SLIDER_Shape_CentreY", "");
            // int val = effect->GetSettings().GetInt("E_SLIDER_Shape_CentreY", 0);
            if (val != "") {
                settings["E_SLIDER_Shape_CentreY"] = std::to_string(100 - std::strtol(val.c_str(), nullptr, 10));
            }
        }
    }

    // Convert absolute file paths to relative for portability
    std::string file = settings["E_FILEPICKERCTRL_SVG"];
    if (!file.empty()) {
        if (std::filesystem::path(file).is_absolute()) {
            if (!FileExists(file, false)) {
                std::string fixed = FixFile("", file);
                std::string rel = MakeRelativeFile(fixed);
                settings["E_FILEPICKERCTRL_SVG"] = rel.empty() ? fixed : rel;
            } else {
                std::string rel = MakeRelativeFile(file);
                if (!rel.empty())
                    settings["E_FILEPICKERCTRL_SVG"] = rel;
            }
        }
        // Register with SequenceMedia so it appears in the Media tab
        auto& media = effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetSequenceMedia();
        media.GetSVG(settings["E_FILEPICKERCTRL_SVG"]);
    }
}

// Convert a Unicode code point to a UTF-8 std::string
static std::string CodePointToUTF8(int cp) {
    std::string result;
    if (cp < 0x80) {
        result += (char)cp;
    } else if (cp < 0x800) {
        result += (char)(0xC0 | (cp >> 6));
        result += (char)(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        result += (char)(0xE0 | (cp >> 12));
        result += (char)(0x80 | ((cp >> 6) & 0x3F));
        result += (char)(0x80 | (cp & 0x3F));
    } else {
        result += (char)(0xF0 | (cp >> 18));
        result += (char)(0x80 | ((cp >> 12) & 0x3F));
        result += (char)(0x80 | ((cp >> 6) & 0x3F));
        result += (char)(0x80 | (cp & 0x3F));
    }
    return result;
}

void ShapeEffect::Drawemoji(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int emoji, int emojiTone, TextFontInfo& font) const
{
    if (radius < 1)
        return;

    auto context = buffer.GetTextDrawingContext();

    TextFontInfo fi = font;
    fi.pixelSize = (int)radius;

    context->SetFont(fi, color);

    std::string textStr = CodePointToUTF8(emoji);
    if (emojiTone) {
        textStr += CodePointToUTF8(emojiTone);
    }

    double width;
    double height;
    context->GetTextExtent(textStr, &width, &height);

    context->SetOverlayMode(true);
    context->DrawText(textStr, std::round((float)xc - width / 2.0), std::round((float)(buffer.BufferHt - yc) - height / 2.0));
    context->SetOverlayMode(false);
}

void ShapeEffect::DrawSVG(ShapeRenderCache* cache, RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness) const
{
    RasterizeSVGToBuffer(cache->GetRasterizer(), cache->GetImage(),
                         cache->_rasterBuf, buffer,
                         xc, yc, radius, cache->_svgScaleBase, color);
}

void ShapeEffect::Drawcandycane(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness) const
{
    double originalRadius = radius;
    double interpolation = 0.75;
    double t = (double)thickness - 1.0 + interpolation;
    for (double i = 0; i < t; i += interpolation) {
        if (radius >= 0) {
            // draw the stick
            int y1 = std::round((double)yc + originalRadius / 6.0);
            int y2 = std::round((double)yc - originalRadius / 2.0);
            int x = std::round((double)xc + radius / 2.0);
            buffer.DrawLine(x, y1, x, y2, color);

            // draw the hook
            double r = radius / 3.0;
            for (double degrees = 0.0; degrees < 180; degrees += 1.0) {
                double radian = degrees * (M_PI / 180.0);
                x = std::round((r - interpolation) * buffer.cos(radian) + xc + originalRadius / 6.0);
                int y = std::round((r - interpolation) * buffer.sin(radian) + y1);
                buffer.SetPixel(x, y, color);
            }
        } else {
            break;
        }
        radius -= interpolation;
    }
}
