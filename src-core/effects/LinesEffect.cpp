/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LinesEffect.h"
#include "../render/SequenceElements.h"

#include <list>
#include <memory>

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "../models/Model.h"
#include "UtilFunctions.h"

#include "../../include/lines-16.xpm"
#include "../../include/lines-24.xpm"
#include "../../include/lines-32.xpm"
#include "../../include/lines-48.xpm"
#include "../../include/lines-64.xpm"

int LinesEffect::sObjectsDefault = 2;
int LinesEffect::sSegmentsDefault = 3;
int LinesEffect::sThicknessDefault = 1;
int LinesEffect::sThicknessMin = 1;
int LinesEffect::sThicknessMax = 10;

double LinesEffect::sSpeedDefault = 1.0;
double LinesEffect::sSpeedMin = 0.0;
double LinesEffect::sSpeedMax = 100.0;
int LinesEffect::sSpeedDivisor = 10;
int LinesEffect::sTrailsDefault = 0;
bool LinesEffect::sFadeTrailsDefault = true;

LinesEffect::LinesEffect(int id) : RenderableEffect(id, "Lines", lines_16, lines_24, lines_32, lines_48, lines_64)
{
}

LinesEffect::~LinesEffect()
{
}

void LinesEffect::OnMetadataLoaded()
{
    sObjectsDefault = GetIntDefault("Lines_Objects", sObjectsDefault);
    sSegmentsDefault = GetIntDefault("Lines_Segments", sSegmentsDefault);
    sThicknessDefault = GetIntDefault("Lines_Thickness", sThicknessDefault);
    sThicknessMin = (int)GetMinFromMetadata("Lines_Thickness", sThicknessMin);
    sThicknessMax = (int)GetMaxFromMetadata("Lines_Thickness", sThicknessMax);
    sSpeedDefault = GetDoubleDefault("Lines_Speed", sSpeedDefault);
    sSpeedMin = GetMinFromMetadata("Lines_Speed", sSpeedMin);
    sSpeedMax = GetMaxFromMetadata("Lines_Speed", sSpeedMax);
    sSpeedDivisor = GetDivisorFromMetadata("Lines_Speed", sSpeedDivisor);
    sTrailsDefault = GetIntDefault("Lines_Trails", sTrailsDefault);
    sFadeTrailsDefault = GetBoolDefault("Lines_FadeTrails", sFadeTrailsDefault);
}

void LinesEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    Render(buffer,
        SettingsMap.GetInt("SLIDER_Lines_Objects", sObjectsDefault),
        SettingsMap.GetInt("SLIDER_Lines_Segments", sSegmentsDefault),
        GetValueCurveInt("Lines_Thickness", sThicknessDefault, SettingsMap, oset, sThicknessMin, sThicknessMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
        GetValueCurveDouble("Lines_Speed", sSpeedDefault, SettingsMap, oset, sSpeedMin, sSpeedMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sSpeedDivisor),
        SettingsMap.GetInt("SLIDER_Lines_Trails", sTrailsDefault),
        SettingsMap.GetBool("CHECKBOX_Lines_FadeTrails", sFadeTrailsDefault)
    );
}

#define pi2 6.283185307

struct LinePoint
{
    float _x;
    float _y;
    double _angle;
    void FlipX() {
        _angle = toRadians(540) - _angle;
        if (_angle >= pi2) {
            _angle -= pi2;
        }
    }
    void FlipY() {
        _angle = toRadians(360) - _angle;
    }
};

class LineObject
{
    std::list<std::list<LinePoint>> _points;

    static LinePoint CreatePoint(RenderBuffer& buffer, int width, int height)
    {
        LinePoint pt;
        pt._x = buffer.rand01() * width;
        pt._y = buffer.rand01() * height;
        pt._angle = buffer.rand01() * pi2;
        return pt;
    }

    static void DrawTrail(const std::list<LinePoint>& t, RenderBuffer& buffer, xlColor c, int thickness)
    {
        auto p1 = t.front();
        auto p2 = t.back();
        buffer.DrawThickLine(p1._x, p1._y, p2._x, p2._y, c, thickness, true);

        if (t.size() > 2) {
            auto it1 = t.begin();
            auto it2 = std::next(t.begin());
            while (it2 != t.end()) {
                buffer.DrawThickLine(it1->_x, it1->_y, it2->_x, it2->_y, c, thickness, true);
                ++it1;
                ++it2;
            }
        }
    }

public:
    void CreateFirst(RenderBuffer& buffer, int points, int width, int height)
    {
        if (_points.size() != 0) return;

        std::list<LinePoint> pts;
        while ((int)pts.size() < points) {
            pts.push_back(CreatePoint(buffer, width, height));
        }
        _points.push_back(std::move(pts));
    }
    void Advance(const RenderBuffer& buffer, double speed, int trails)
    {
        while ((int)_points.size() > trails + 1) {
            _points.pop_back();
        }

        std::list<LinePoint> last = _points.back();
        for (auto& trail : _points) {
            for (auto& pt : trail) {
                float speedX = cos(pt._angle) * speed;
                float speedY = sin(pt._angle) * speed;

                float x = pt._x + speedX;
                float y = pt._y + speedY;

                // bounce
                if (x < 0) {
                    x = std::abs(x);
                    pt.FlipX();
                }
                if (x >= buffer.BufferWi) {
                    x = 2 * buffer.BufferWi - x;
                    pt.FlipX();
                }
                if (y < 0) {
                    y = std::abs(y);
                    pt.FlipY();
                }
                if (y >= buffer.BufferHt) {
                    y = 2 * buffer.BufferHt - y;
                    pt.FlipY();
                }
                pt._x = x;
                pt._y = y;
            }
        }
        if ((int)_points.size() < trails + 1) {
            _points.push_back(last);
        }
    }

    void Draw(RenderBuffer& buffer, xlColor c, int trails, bool fadeTrails, int thickness)
    {
        int i = 1;
        for (auto t = _points.rbegin(); t != _points.rend(); ++t) {
            if (fadeTrails && trails > 0) {
                c.SetAlpha(255 * i++ / (int)_points.size());
                DrawTrail(*t, buffer, c, thickness);
            } else {
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
    void Advance(const RenderBuffer& buffer, double speed, int trails) {
        for (auto& it : _lineObjects) {
            it.Advance(buffer, speed, trails);
        }
    }
    void CreateDestroy(RenderBuffer& buffer, int objects, int points, int width, int height) {
        while ((int)_lineObjects.size() > objects) {
            _lineObjects.pop_back();
        }
        while ((int)_lineObjects.size() < objects) {
            LineObject line;
            line.CreateFirst(buffer, points, width, height);
            _lineObjects.push_back(std::move(line));
        }
    }
};

// Tier-2 immutable per-frame draw state: the whole line-object list (points,
// angles, trails) plus the buffer's needToInit flag, captured BEFORE this
// frame's advance.  A frame-parallel draw pass restores these onto a clone and
// re-runs the normal render, reproducing the serial frame byte-for-byte.
struct LinesFrameState : public EffectFrameState {
    std::list<LineObject> lineObjects;
    bool needToInit = false;
};

RenderableEffect::FrameParallelism LinesEffect::GetFrameParallelism(const SettingsMap& settings) const {
    // All cross-frame state is the LinesRenderCache line-object list, which
    // LinesFrameState snapshots in full; the random draws are per-frame-seeded,
    // so a draw pass that restores the pre-frame state and re-runs the advance
    // reproduces the serial frame.
    return FrameParallelism::Snapshottable;
}

void LinesEffect::Render(RenderBuffer &buffer, int objects, int points, int thickness, double speed, int trails, bool fadeTrails)
{
	// Grab our cache
	LinesRenderCache *cache = static_cast<LinesRenderCache*>(buffer.infoCache[id]);
	if (cache == nullptr) {
		cache = new LinesRenderCache();
		buffer.infoCache[id] = cache;
	}

    auto& _lines = cache->_lineObjects;

    // Frame-parallel Snapshottable paths (see GetFrameParallelism): restore or
    // capture the pre-frame state, then run the normal advance either way.
    if (buffer.pendingSnapshot != nullptr) {
        const LinesFrameState& fs = static_cast<const LinesFrameState&>(*buffer.pendingSnapshot);
        cache->_lineObjects = fs.lineObjects;
        buffer.needToInit = fs.needToInit;
    } else if (buffer.captureSnapshot != nullptr) {
        auto fs = std::make_unique<LinesFrameState>();
        fs->lineObjects = cache->_lineObjects;
        fs->needToInit = buffer.needToInit;
        *buffer.captureSnapshot = std::move(fs);
    }

	// Check for config changes which require us to reset
	if (buffer.needToInit) {
        buffer.needToInit = false;
	}

    cache->CreateDestroy(buffer, objects, points, buffer.BufferWi, buffer.BufferHt);
    cache->Advance(buffer, speed, trails);

    // Frame-parallel serial capture pass is advance-only: the draw below goes
    // through a temp buffer + AlphaBlend (raw pixel writes the SetPixel capture
    // guard cannot suppress) - skip it; the clones draw these frames.
    if (buffer.captureSnapshot != nullptr) {
        return;
    }

    RenderBuffer temp(buffer);
    temp.SetAllowAlphaChannel(true);

    int color = 0;
    for (auto line : _lines) {
        xlColor c = buffer.palette.GetColor(color++ % buffer.GetColorCount());

        // Draw into a temp buffer and then alpha blend that into the main buffer
        // This minimises artefacts due to over-rendering
        temp.Clear();
        line.Draw(temp, c, trails, fadeTrails, thickness);
        buffer.AlphaBlend(temp);
    }
}

bool LinesEffect::needToAdjustSettings(const std::string& version)
{
    return IsVersionOlder("2026.07", version) || RenderableEffect::needToAdjustSettings(version);
}

void LinesEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults)
{
    if (RenderableEffect::needToAdjustSettings(version)) {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }

    if (IsVersionOlder("2026.07", version)) {
        // Lines_Speed changed from int (E_SLIDER_Lines_Speed, range 1-10)
        // to float (E_TEXTCTRL_Lines_Speed, range 0.0-10.0).
        // The stored integer value equals the actual speed, so no scaling is
        // needed — just rename the key so the new panel control finds it.
        SettingsMap& settings = effect->GetSettings();
        const std::string oldKey = "E_SLIDER_Lines_Speed";
        const std::string newKey = "E_TEXTCTRL_Lines_Speed";
        if (settings.Contains(oldKey) && !settings.Contains(newKey)) {
            settings[newKey] = settings.Get(oldKey, "");
            settings.erase(oldKey);
        }
    }
}
