#include "LinesEffect.h"
#include "LinesPanel.h"
#include "../AudioManager.h"
#include "../sequencer/SequenceElements.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../models/Model.h"
#include "../UtilFunctions.h"

#include "../../include/lines-16.xpm"
#include "../../include/lines-24.xpm"
#include "../../include/lines-32.xpm"
#include "../../include/lines-48.xpm"
#include "../../include/lines-64.xpm"

LinesEffect::LinesEffect(int id) : RenderableEffect(id, "Lines", lines_16, lines_24, lines_32, lines_48, lines_64)
{
}

LinesEffect::~LinesEffect()
{
}

wxPanel *LinesEffect::CreatePanel(wxWindow *parent) {
	return new LinesPanel(parent);
}

void LinesEffect::SetDefaultParameters() 
{
    LinesPanel *lp = static_cast<LinesPanel*>(panel);
    if (lp == nullptr) {
        return;
    }

    lp->BitmapButton_Lines_Speed->SetActive(false);
    lp->BitmapButton_Lines_Thickness->SetActive(false);

    SetSliderValue(lp->Slider_Lines_Objects, 2);
    SetSliderValue(lp->Slider_Lines_Segments, 3);
    SetSliderValue(lp->Slider_Lines_Speed, 1);
    SetSliderValue(lp->Slider_Lines_Trails, 0);
    SetSliderValue(lp->Slider_Lines_Thickness, 1);
    SetCheckBoxValue(lp->CheckBox_FadeTrails, true);
    lp->ValidateWindow();
}

void LinesEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    Render(buffer,
        SettingsMap.GetInt("SLIDER_Lines_Objects", 2),
        SettingsMap.GetInt("SLIDER_Lines_Segments", 3),
        GetValueCurveInt("Lines_Thickness", 1, SettingsMap, oset, LINES_THICKNESS_MIN, LINES_THICKNESS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
        GetValueCurveInt("Lines_Speed", 1, SettingsMap, oset, LINES_SPEED_MIN, LINES_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
        SettingsMap.GetInt("SLIDER_Lines_Trails", 0),
        SettingsMap.GetBool("CHECKBOX_Lines_FadeTrails", true)
    );
}

#define pi2 6.283185307

struct LinePoint
{
    float _x;
    float _y;
    double _angle;
    void FlipX()
    {
        _angle = toRadians(540) - _angle;
        if (_angle >= pi2)
        {
            _angle -= pi2;
        }

    }
    void FlipY()
    {
        _angle = toRadians(360) - _angle;
    }
};

class LineObject
{
    std::list<std::list<LinePoint>> _points;

    static LinePoint CreatePoint(int width, int height)
    {
        LinePoint pt;
        pt._x = rand01() * width;
        pt._y = rand01() * height;
        pt._angle = rand01() * pi2;
        return pt;
    }

    static void DrawTrail(const std::list<LinePoint>& t, RenderBuffer& buffer, xlColor c, int thickness)
    {
            auto p1 = t.front();
            auto p2 = t.back();
            buffer.DrawThickLine(p1._x, p1._y, p2._x, p2._y, c, thickness, true);

            if (t.size() > 2)
            {
                auto it1 = t.begin();
                auto it2 = std::next(t.begin());
                while (it2 != t.end())
                {
                    buffer.DrawThickLine(it1->_x, it1->_y, it2->_x, it2->_y, c, thickness, true);
                    ++it1;
                    ++it2;
                }
            }
    }

public:
    void CreateFirst(int points, int width, int height)
    {
        if (_points.size() != 0) return;

        std::list<LinePoint> pts;
        while (pts.size() < points)
        {
            pts.push_back(CreatePoint(width, height));
        }
        _points.push_back(std::move(pts));
    }
    void Advance(const RenderBuffer& buffer, int speed, int trails)
    {
        while (_points.size() > trails + 1)
        {
            _points.pop_back();
        }

        std::list<LinePoint> last = _points.back();
        for (auto& trail : _points)
        {
            for (auto& pt : trail)
            {
                float speedX = cos(pt._angle) * speed;
                float speedY = sin(pt._angle) * speed;

                float x = pt._x + speedX;
                float y = pt._y + speedY;

                // bounce
                if (x < 0)
                {
                    x = std::abs(x);
                    pt.FlipX();
                }
                if (x >= buffer.BufferWi)
                {
                    x = 2 * buffer.BufferWi - x;
                    pt.FlipX();
                }
                if (y < 0)
                {
                    y = std::abs(y);
                    pt.FlipY();
                }
                if (y >= buffer.BufferHt)
                {
                    y = 2 * buffer.BufferHt - y;
                    pt.FlipY();
                }
                pt._x = x;
                pt._y = y;
            }
        }
        if (_points.size() < trails + 1)
        {
            _points.push_back(last);
        }
    }

    void Draw(RenderBuffer& buffer, xlColor c, int trails, bool fadeTrails, int thickness)
    {
        int i = 1;
        for (auto t = _points.rbegin(); t != _points.rend(); ++t)
        {
            if (fadeTrails && trails > 0)
            {
                c.SetAlpha(255 * i++ / _points.size());
                DrawTrail(*t, buffer, c, thickness);
            }
            else
            {
                DrawTrail(*t, buffer, c, thickness);
            }
        }
    }
};

class LinesRenderCache : public EffectRenderCache
{

public:
    LinesRenderCache()
    {
    };
    virtual ~LinesRenderCache() {};
    std::list<LineObject> _lineObjects;
    void Advance(const RenderBuffer& buffer, int speed, int trails)
    {
        for (auto& it : _lineObjects)
        {
            it.Advance(buffer, speed, trails);
        }
    }
    void CreateDestroy(int objects, int points, int width, int height)
    {
        while (_lineObjects.size() > objects)
        {
            _lineObjects.pop_back();
        }
        while (_lineObjects.size() < objects)
        {
            LineObject line;
            line.CreateFirst(points, width, height);
            _lineObjects.push_back(std::move(line));
        }
    }
};

void LinesEffect::Render(RenderBuffer &buffer, int objects, int points, int thickness, int speed, int trails, bool fadeTrails)
{
	// Grab our cache
	LinesRenderCache *cache = static_cast<LinesRenderCache*>(buffer.infoCache[id]);
	if (cache == nullptr) {
		cache = new LinesRenderCache();
		buffer.infoCache[id] = cache;
	}

    auto& _lines = cache->_lineObjects;

	// Check for config changes which require us to reset
	if (buffer.needToInit)
	{
        buffer.needToInit = false;
	}

    cache->CreateDestroy(objects, points, buffer.BufferWi, buffer.BufferHt);
    cache->Advance(buffer, speed, trails);

    RenderBuffer temp(buffer);
    temp.SetAllowAlphaChannel(true);

    int color = 0;
    for (auto line : _lines)
    {
        xlColor c = buffer.palette.GetColor(color++ % buffer.GetColorCount());

        ///////////////////////////////////////////// DMX Model Support //////////////////////////////////////////
        // if the model is a DMX model this will write the color into the proper red, green, and blue channels. //
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (RenderDMXModelPalette(buffer)) {
            // function exits here
            return;
        }

        // Draw into a temp buffer and then alpha blend that into the main buffer
        // This minimises artefacts due to over-rendering
        temp.Clear();
        line.Draw(temp, c, trails, fadeTrails, thickness);
        buffer.AlphaBlend(temp);
    }
}