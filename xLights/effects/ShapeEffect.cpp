/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ShapeEffect.h"
#include "ShapePanel.h"
#include "TextEffect.h" // FontMapLock

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../models/Model.h"
#include "../sequencer/SequenceElements.h"
#include "UtilFunctions.h"
#include "AudioManager.h"
#include "../ExternalHooks.h"
#include "../xLightsMain.h" 

#include "nanosvg/src/nanosvg.h"

#include <regex>

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
    std::list<std::string> res;

    if (media == nullptr && settings.GetBool("E_CHECKBOX_Shape_UseMusic", false))
    {
        res.push_back(wxString::Format("    WARN: Shape effect cant grow to music if there is no music. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    std::string object = settings["E_CHOICE_Shape_ObjectToDraw"];
    if (object == "SVG") {
        auto svgFilename = settings.Get("E_FILEPICKERCTRL_SVG", "");

        if (svgFilename == "" || !FileExists(svgFilename)) {
            res.push_back(wxString::Format("    ERR: Shape effect cant find SVG file '%s'. Model '%s', Start %s", svgFilename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        } else {
            if (!IsFileInShowDir(xLightsFrame::CurrentDir, svgFilename)) {
                res.push_back(wxString::Format("    WARN: Shape effect SVG file '%s' not under show directory. Model '%s', Start %s", svgFilename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
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
    wxString file = SettingsMap["E_FILEPICKERCTRL_SVG"];
    if (FileExists(file)) {
        if (!frame->IsInShowFolder(file)) {
            SettingsMap["E_FILEPICKERCTRL_SVG"] = frame->MoveToShowFolder(file, wxString(wxFileName::GetPathSeparator()) + "Images");
            rc = true;
        }
    }

    return rc;
}

void ShapeEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect)
{
    wxString timing = effect->GetSettings().Get("E_CHOICE_Shape_FireTimingTrack", "");

    if (timing.ToStdString() == oldname)
    {
        effect->GetSettings()["E_CHOICE_Shape_FireTimingTrack"] = wxString(newname);
    }

    SetPanelTimingTracks();
}

void ShapeEffect::SetPanelStatus(Model *cls)
{
    SetPanelTimingTracks();
}

void ShapeEffect::SetPanelTimingTracks() const
{
    ShapePanel *fp = (ShapePanel*)panel;
    if (fp == nullptr)
    {
        return;
    }

    if (mSequenceElements == nullptr)
    {
        return;
    }

    // Load the names of the timing tracks
    std::string timingtracks = "";
    for (size_t i = 0; i < mSequenceElements->GetElementCount(); i++)
    {
        Element* e = mSequenceElements->GetElement(i);
        if (e->GetEffectLayerCount() == 1 && e->GetType() == ElementType::ELEMENT_TYPE_TIMING)
        {
            if (timingtracks != "") timingtracks += "|";
            timingtracks += e->GetName();
        }
    }

    wxCommandEvent event(EVT_SETTIMINGTRACKS);
    event.SetString(timingtracks);
    wxPostEvent(fp, event);
}

xlEffectPanel *ShapeEffect::CreatePanel(wxWindow *parent) {
    return new ShapePanel(parent);
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

void ShapeEffect::SetDefaultParameters() {
    ShapePanel *sp = (ShapePanel*)panel;
    if (sp == nullptr) {
        return;
    }

    sp->BitmapButton_Shape_ThicknessVC->SetActive(false);
    sp->BitmapButton_Shape_CentreXVC->SetActive(false);
    sp->BitmapButton_Shape_CentreYVC->SetActive(false);
    sp->BitmapButton_Shapes_Velocity->SetActive(false);
    sp->BitmapButton_Shapes_Direction->SetActive(false);
    sp->BitmapButton_Shape_LifetimeVC->SetActive(false);
    sp->BitmapButton_Shape_GrowthVC->SetActive(false);
    sp->BitmapButton_Shape_CountVC->SetActive(false);
    sp->BitmapButton_Shape_StartSizeVC->SetActive(false);
    sp->BitmapButton_Shape_RotationVC->SetActive(false);

    SetChoiceValue(sp->Choice_Shape_ObjectToDraw, "Circle");

    SetSliderValue(sp->Slider_Shape_Thickness, 1);
    SetSliderValue(sp->Slider_Shape_StartSize, 1);
    SetSliderValue(sp->Slider_Shape_CentreX, 50);
    SetSliderValue(sp->Slider_Shape_CentreY, 50);
    SetSliderValue(sp->Slider_Shape_Points, 5);
    SetSliderValue(sp->Slider_Shape_Count, 5);
    SetSliderValue(sp->Slider_Shape_Growth, 10);
    SetSliderValue(sp->Slider_Shape_Lifetime, 5);
    SetSliderValue(sp->Slider_Shape_Sensitivity, 50);
    SetSliderValue(sp->Slider_Shape_Rotation, 0);
    SetSliderValue(sp->Slider_Shapes_Velocity, 0);
    SetSliderValue(sp->Slider_Shapes_Direction, 90);

    SetCheckBoxValue(sp->CheckBox_Shape_RandomLocation, true);
    SetCheckBoxValue(sp->CheckBox_Shape_FadeAway, true);
    SetCheckBoxValue(sp->CheckBox_Shape_UseMusic, false);
    SetCheckBoxValue(sp->CheckBox_Shape_FireTiming, false);
    SetCheckBoxValue(sp->CheckBox_Shape_RandomInitial, true);
    SetCheckBoxValue(sp->CheckBox_Shape_HoldColour, true);
    SetCheckBoxValue(sp->CheckBox_FilterLabelReg, false);

    sp->TextCtrl_Shape_FilterLabel->SetValue("");

    sp->FilePickerCtrl_SVG->SetFileName(wxFileName(""));
}

struct ShapeData
{
private:
    xlColor _color;
public:
    wxPoint _centre;
    wxSize _movement;
    float _size;
    int _oset;
    int _shape;
    float _angle;
    int _speed;
    int _colourIndex;
    bool _holdColour;

    ShapeData(wxPoint centre, float size, int oset, xlColor color, int shape, int angle, int speed, bool holdColour, int colourIndex)
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

    void SetCentre(wxPoint centre)
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
    }

    std::list<ShapeData*> _shapes;
    int _lastColorIdx = 0;
    int _sinceLastTriggered = 0;
    wxFontInfo _font;
    NSVGimage* _svgImage = nullptr;
    std::string _svgFilename;
    float _svgScaleBase = 1.0f;
    std::string _filterLabel;
    std::regex _filterRegex;
    bool _useRegex;

    void SetFilter(const std::string& filterLabel, bool useRegex)
    {
        _filterLabel = filterLabel;
        _useRegex = useRegex;
        if (useRegex)
            _filterRegex = std::regex(filterLabel, std::regex_constants::extended);
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

    void InitialiseSVG(const std::string filename)
    {
        if (_svgImage != nullptr) {
            nsvgDelete(_svgImage);
            _svgImage = nullptr;
        }

        _svgFilename = filename;
        _svgImage = nsvgParseFromFile(_svgFilename.c_str(), "px", 96);
        if (_svgImage != nullptr) {
            auto max = std::max(_svgImage->height, _svgImage->width);
            _svgScaleBase = 1.0f / (float)max;
        }
    }

    void AddShape(wxPoint centre, float size, xlColor color, int oset, int shape, int angle, int speed, bool randomMovement, bool holdColour, int colourIndex)
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
    wxString timing = SettingsMap.Get("CHOICE_Shape_FireTimingTrack", "");
    if (timing == "") useTiming = false;
    if (useMusic) {
        if (buffer.GetMedia() != nullptr) {
            std::list<float> const * const pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
            if (pf != nullptr)
            {
                f = *(pf->cbegin());
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
    wxFontInfo& _font = cache->_font;

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
            cache->InitialiseSVG(svgFilename);
        }

        cache->DeleteShapes();
        _lastColorIdx = -1;

        if (!useTiming && !useMusic) {
            for (int i = _shapes.size(); i < count; ++i) {
                wxPoint pt;
                if (randomLocation) {
                    pt = wxPoint(rand01() * buffer.BufferWi, rand01() * buffer.BufferHt);
                } else {
                    pt = wxPoint(xc, yc);
                }

                size_t colorcnt = buffer.GetColorCount();
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
                        wxPoint pt;
                        if (randomLocation)
                        {
                            pt = wxPoint(rand01() * buffer.BufferWi, rand01() * buffer.BufferHt);
                        }
                        else
                        {
                            pt = wxPoint(xc, yc);
                        }

                        size_t colorcnt = buffer.GetColorCount();
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
                wxPoint pt;
                if (randomLocation)
                {
                    pt = wxPoint(rand01() * buffer.BufferWi, rand01() * buffer.BufferHt);
                }
                else
                {
                    pt = wxPoint(xc, yc);
                }

                size_t colorcnt = buffer.GetColorCount();
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
            wxPoint pt;
            if (randomLocation)
            {
                pt = wxPoint(rand01() * buffer.BufferWi, rand01() * buffer.BufferHt);
            }
            else
            {
                pt = wxPoint(xc, yc);
            }

            size_t colorcnt = buffer.GetColorCount();
            _lastColorIdx++;
            if (_lastColorIdx >= colorcnt)
            {
                _lastColorIdx = 0;
            }

            cache->AddShape(pt, startSize, buffer.palette.GetColor(_lastColorIdx), 0, Object_To_Draw, direction, velocity, randomMovement, holdColour, _lastColorIdx);
        }
    }

    if (Object_To_Draw == RENDER_SHAPE_EMOJI || Object_To_Draw == RENDER_SHAPE_SVG) {
        auto context = buffer.GetTextDrawingContext();
        context->Clear();
    }

    for (const auto& it : _shapes) {
        // if location is not random then update it to whatever the current location is
        // as it may be value curve controlled
        if (!randomLocation)
        {
            it->SetCentre(wxPoint(xc, yc));
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
            wxASSERT(false);
            break;
        }

        // move etc after drawing otherwise first frame has already moved
        it->Move();
        it->_oset++;
        it->_size += growthPerFrame;

        if (it->_size < 0) it->_size = 0;
    }

    if (Object_To_Draw == RENDER_SHAPE_EMOJI) {
        wxImage *i = buffer.GetTextDrawingContext()->FlushAndGetImage();
        unsigned char* data = i->GetData();
        unsigned char* alpha = i->HasAlpha() ? i->GetAlpha() : nullptr;
        int w = i->GetWidth();
        int h = i->GetHeight();
        int cur = 0;
        int curA = 0;
        xlColor c, c2;
        for (int y = h - 1; y >= 0; y--) {
            for (int x = 0; x < w; x++) {
                xlColor c(data[cur], data[cur + 1], data[cur + 2]);
                if (alpha) {
                    c.Set(data[cur], data[cur + 1], data[cur + 2], alpha[curA]);
                } else {
                    c.Set(data[cur], data[cur + 1], data[cur + 2]);
                    if (c == xlBLACK) {
                        c.alpha = 0;
                    }
                }
                buffer.GetPixel(x, y, c2);
                if (c2 != xlBLACK) {
                    c.AlphaBlend(c2);
                }
                buffer.SetPixel(x, y, c);
                cur += 3;
                ++curA;
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

bool ShapeEffect::areSame(double a, double b, float eps) const
{
    return std::fabs(a - b) < eps;
}

bool ShapeEffect::areCollinear(const wxPoint2DDouble& a, const wxPoint2DDouble& b, const wxPoint2DDouble& c, double eps) const
{
    // use dot product to determine if point are in a strait line
    auto [a_x, a_y] = a;
    auto [b_x, b_y] = b;
    auto [c_x, c_y] = c;

    auto test = (b_x - a_x) * (c_y - a_y) - (c_x - a_x) * (b_y - a_y);
    return std::abs(test) < eps;
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
        wxPoint start;
        wxPoint end;

        line(const wxPoint s, const wxPoint e)
        {
            start = s;
            end = e;
        }
    };

    const line points[] = {line(wxPoint(3,0), wxPoint(5,0)),
                           line(wxPoint(5,0), wxPoint(5,3)),
                           line(wxPoint(3,0), wxPoint(3,3)),
                           line(wxPoint(0,3), wxPoint(8,3)),
                           line(wxPoint(0,3), wxPoint(2,6)),
                           line(wxPoint(8,3), wxPoint(6,6)),
                           line(wxPoint(1,6), wxPoint(2,6)),
                           line(wxPoint(6,6), wxPoint(7,6)),
                           line(wxPoint(1,6), wxPoint(3,9)),
                           line(wxPoint(7,6), wxPoint(5,9)),
                           line(wxPoint(2,9), wxPoint(3,9)),
                           line(wxPoint(5,9), wxPoint(6,9)),
                           line(wxPoint(6,9), wxPoint(4,11)),
                           line(wxPoint(2,9), wxPoint(4,11))
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
        wxPoint start;
        wxPoint end;

        line(const wxPoint s, const wxPoint e)
        {
            start = s;
            end = e;
        }
    };

    const line points[] = {line(wxPoint(2,0), wxPoint(2,6)),
                           line(wxPoint(2,6), wxPoint(0,6)),
                           line(wxPoint(0,6), wxPoint(0,7)),
                           line(wxPoint(0,7), wxPoint(2,7)),
                           line(wxPoint(2,7), wxPoint(2,10)),
                           line(wxPoint(2,10), wxPoint(3,10)),
                           line(wxPoint(3,10), wxPoint(3,7)),
                           line(wxPoint(3,7), wxPoint(5,7)),
                           line(wxPoint(5,7), wxPoint(5,6)),
                           line(wxPoint(5,6), wxPoint(3,6)),
                           line(wxPoint(3,6), wxPoint(3,0)),
                           line(wxPoint(3,0), wxPoint(2,0))
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
        wxPoint start;
        wxPoint end;

        line(const wxPoint s, const wxPoint e)
        {
            start = s;
            end = e;
        }
    };

    const line points[] = { line(wxPoint(0, 0), wxPoint(0, 9)),
                            line(wxPoint(0, 9), wxPoint(10, 9)),
                            line(wxPoint(10, 9), wxPoint(10, 0)),
                            line(wxPoint(10, 0), wxPoint(0, 0)),
                            line(wxPoint(5, 0), wxPoint(5, 9)),
                            line(wxPoint(5, 9), wxPoint(2, 11)),
                            line(wxPoint(2, 11), wxPoint(2, 9)),
                            line(wxPoint(5, 9), wxPoint(8, 11)),
                            line(wxPoint(8, 11), wxPoint(8, 9)) };
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

void ShapeEffect::Drawemoji(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int emoji, int emojiTone, wxFontInfo& font) const
{
    if (radius < 1)
        return;

    auto context = buffer.GetTextDrawingContext();

    wxFontInfo fi(wxSize(0, radius));
    fi.FaceName(font.GetFaceName());
    fi.Light(font.GetWeight() == wxFONTWEIGHT_LIGHT);
    fi.AntiAliased(font.IsAntiAliased());
    fi.Encoding(font.GetEncoding());

    context->SetFont(fi, color);

    wxUniChar ch = emoji;
    auto text = wxString(ch);
    if (emojiTone) {
        wxUniChar ch2 = emojiTone;
        text.Append(wxString(ch2));
    }

    double width;
    double height;
    context->GetTextExtent(text, &width, &height);

    context->SetOverlayMode(true);
    context->DrawText(text, std::round((float)xc - width / 2.0), std::round((float)yc - height / 2.0));
    context->SetOverlayMode(false);
}

static inline wxPoint2DDouble ScaleMovePoint(const wxPoint2DDouble pt, const wxPoint2DDouble imageCentre, const wxPoint2DDouble centre, float factor, float scaleTo)
{
    return centre + ((pt - imageCentre) * factor * scaleTo * 10);
}

static inline uint8_t GetSVGRed(uint32_t colour)
{
    return (colour);
}

static inline uint8_t GetSVGGreen(uint32_t colour)
{
    return (colour >> 8);
}

static inline uint8_t GetSVGBlue(uint32_t colour)
{
    return (colour >> 16);
}

static inline uint8_t GetSVGAlpha(uint32_t colour)
{
    return (colour >> 24);
}

static inline uint32_t GetSVGExAlpha(uint32_t colour)
{
    return (colour & 0xFFFFFF);
}

void ShapeEffect::DrawSVG(ShapeRenderCache* cache, RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness) const
{
    auto image = cache->GetImage();
    if (image == nullptr) {
        for (size_t x = 0; x < buffer.BufferWi; ++ x) {
            for (size_t y = 0; y < buffer.BufferHt; ++y) {
                buffer.SetPixel(x, y, xlRED);
            }
        }
    } else {
        auto context = buffer.GetPathDrawingContext();
        context->Clear();

        wxPoint2DDouble centre((float)xc, (float)yc);
        wxPoint2DDouble imageCentre((float)image->width / 2.0, (float)image->height / 2.0);

        for (NSVGshape* shape = image->shapes; shape != nullptr; shape = shape->next) {

            if (GetSVGExAlpha(shape->fill.color) != 0) {
                if (shape->fill.type == 0) {
                    context->SetBrush(wxNullBrush);
                }
                else if (shape->fill.type == 1) {
                    wxColor bc(GetSVGRed(shape->fill.color), GetSVGGreen(shape->fill.color), GetSVGBlue(shape->fill.color), /*shape->opacity * */ GetSVGAlpha(shape->fill.color) * color.alpha / 255);
                    wxBrush brush(bc, wxBrushStyle::wxBRUSHSTYLE_SOLID);
                    context->SetBrush(brush);
                } else {
                    // these are gradients and I know they are not right
                    if (shape->fill.gradient->nstops == 2) {
                        wxColor c1(GetSVGRed(shape->fill.gradient->stops[0].color), GetSVGGreen(shape->fill.gradient->stops[0].color), GetSVGBlue(shape->fill.gradient->stops[0].color), /*shape->opacity * */ GetSVGAlpha(shape->fill.gradient->stops[0].color) * color.alpha / 255);
                        wxColor c2(GetSVGRed(shape->fill.gradient->stops[1].color), GetSVGGreen(shape->fill.gradient->stops[1].color), GetSVGBlue(shape->fill.gradient->stops[1].color), /*shape->opacity * */ GetSVGAlpha(shape->fill.gradient->stops[1].color) * color.alpha / 255);
                        wxGraphicsBrush brush = context->CreateLinearGradientBrush(0, buffer.BufferHt, 0, 0, c1, c2);
                        context->SetBrush(brush);
                    } else {
                        wxGraphicsGradientStops stops;
                        for (size_t i = 0; i < shape->fill.gradient->nstops; ++i) {
                            wxColor sc(GetSVGRed(shape->fill.gradient->stops[i].color), GetSVGGreen(shape->fill.gradient->stops[i].color), GetSVGBlue(shape->fill.gradient->stops[i].color), /*shape->opacity * */ GetSVGAlpha(shape->fill.gradient->stops[i].color) * color.alpha / 255);
                            if (i == 0) {
                                stops.SetStartColour(sc);
                            } else if (i == shape->fill.gradient->nstops - 1) {
                                stops.SetEndColour(sc);
                            } else {
                                stops.Add(sc, 1.0 - shape->fill.gradient->stops[i].offset);
                            }
                        }
                        wxGraphicsBrush brush = context->CreateLinearGradientBrush(0, buffer.BufferHt, 0, 0, stops);
                        context->SetBrush(brush);
                    }
                }
            }

            if (shape->stroke.type == 0) {
                context->SetPen(wxNullPen);
            } else if (shape->stroke.type == 1) {
                wxColor pc(GetSVGRed(shape->stroke.color), GetSVGGreen(shape->stroke.color), GetSVGBlue(shape->stroke.color), /*shape->opacity * */ GetSVGAlpha(shape->stroke.color) * color.alpha / 255);
                wxPen pen(pc, thickness);
                context->SetPen(pen);
            } else {
                // we dont fo gradient lines yet
            }

            for (NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
                wxGraphicsPath cpath = context->CreatePath();
                for (int i = 0; i < path->npts - 1; i += 3) {
                    float* p = &path->pts[i * 2];
                    auto ih = image->height;
                    wxPoint2DDouble start = ScaleMovePoint(wxPoint2DDouble(p[0], ih - p[1]), imageCentre, centre, cache->_svgScaleBase, radius);
                    wxPoint2DDouble cp1 = ScaleMovePoint(wxPoint2DDouble(p[2], ih - p[3]), imageCentre, centre, cache->_svgScaleBase, radius);
                    wxPoint2DDouble cp2 = ScaleMovePoint(wxPoint2DDouble(p[4], ih - p[5]), imageCentre, centre, cache->_svgScaleBase, radius);
                    wxPoint2DDouble end = ScaleMovePoint(wxPoint2DDouble(p[6], ih - p[7]), imageCentre, centre, cache->_svgScaleBase, radius);

                    if (i == 0) cpath.MoveToPoint(start);

                    if (areCollinear(start, cp1, end, 0.001f) && areCollinear(start, cp2, end, 0.001f)) { // check if its a straight line
                        cpath.AddLineToPoint(end);
                    } else if (areSame(end.m_x, cp2.m_x, 0.001f) && areSame(end.m_y, cp2.m_y, 0.001f)) { // check if control points2 is the end
                        cpath.AddQuadCurveToPoint(cp1.m_x, cp1.m_y, end.m_x, end.m_y);
                    } else {
                        cpath.AddCurveToPoint(cp1.m_x, cp1.m_y, cp2.m_x, cp2.m_y, end.m_x, end.m_y);
                    }
                }
                if (path->closed) {
                    cpath.CloseSubpath();
                    context->FillPath(cpath, wxPolygonFillMode::wxODDEVEN_RULE);
                } 
                context->StrokePath(cpath);
            }
        }

        wxImage* image = buffer.GetPathDrawingContext()->FlushAndGetImage();
        bool hasAlpha = image->HasAlpha();

        xlColor cc;
        for (int y = 0; y < buffer.BufferHt; ++y) {
            for (int x = 0; x < buffer.BufferWi; ++x) {
                if (hasAlpha) {
                    cc = xlColor(image->GetRed(x, y), image->GetGreen(x, y), image->GetBlue(x, y), image->GetAlpha(x, y));
                    cc = cc.AlphaBlend(buffer.GetPixel(x, y));
                } else {
                    cc.Set(image->GetRed(x, y), image->GetGreen(x, y), image->GetBlue(x, y), 255);
                }
                buffer.SetPixel(x, y, cc);
            }
        }
    }
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
