/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SketchEffect.h"

#include <filesystem>

#include "../BulkEditControls.h"
#include "../render/RenderBuffer.h"
#include "SketchEffectDrawing.h"
#include "../ui/effectpanels/SketchPanel.h"
#include "../ui/effectpanels/EffectPanelManager.h"
#include "../UtilFunctions.h"
#include "../xLightsApp.h"
#include "../xLightsMain.h"
#include "../ExternalHooks.h"

#include "../../include/sketch-16.xpm"
#include "../../include/sketch-24.xpm"
#include "../../include/sketch-32.xpm"
#include "../../include/sketch-48.xpm"
#include "../../include/sketch-64.xpm"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <format>

#include "nanosvg/src/nanosvg.h"
#include "nanosvg/src/nanosvgrast.h"

namespace
{
    struct LinearInterpolater {
        double operator()(double t) const
        {
            return t;
        }
    };

    template<class T>
    double interpolate(double x, double loIn, double loOut, double hiIn, double hiOut, const T& interpolater)
    {
        return (loIn != hiIn)
                   ? (loOut + (hiOut - loOut) * interpolater((x - loIn) / (hiIn - loIn)))
                   : ((loOut + hiOut) / 2);
    }

    double calcPercentage(double v, double s, double e)
    {
        return (v - s) / (e - s);
    }
}

class SketchRenderCache : public EffectRenderCache {
public:
    SketchRenderCache() {
        rasterizer = nsvgCreateRasterizer();
    }
    ~SketchRenderCache() override {
        if (rasterizer) nsvgDeleteRasterizer(rasterizer);
    }
    NSVGrasterizer* rasterizer = nullptr;
    std::vector<uint8_t> rasterBuf;
};

SketchEffect::SketchEffect(int id) :
    RenderableEffect(id, "Sketch", sketch_16_xpm, sketch_24_xpm, sketch_32_xpm, sketch_48_xpm, sketch_64_xpm)
{
}

SketchEffect::~SketchEffect()
{
}

void SketchEffect::Render(Effect* /*effect*/, const SettingsMap& settings, RenderBuffer& buffer)
{
    double progress = buffer.GetEffectTimeIntervalPosition(1.f);

    std::string sketchDef = settings.Get("TEXTCTRL_SketchDef", "");
    double drawPercentage = GetValueCurveDouble("DrawPercentage", SketchPanel::DrawPercentageDef, settings, progress,
                                                SketchPanel::DrawPercentageMin, SketchPanel::DrawPercentageMax,
                                                buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int thickness = GetValueCurveInt("Thickness", SketchPanel::ThicknessDef, settings, progress,
                                     SketchPanel::ThicknessMin, SketchPanel::ThicknessMax,
                                     buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool motionEnabled = settings.GetBool("CHECKBOX_MotionEnabled");
    int motionPercentage = GetValueCurveInt("MotionPercentage", SketchPanel::MotionPercentageDef, settings, progress,
                                            SketchPanel::MotionPercentageMin, SketchPanel::MotionPercentageMax,
                                            buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    xlColorVector colors(buffer.GetColorCount());
    for (size_t i = 0; i < buffer.GetColorCount(); ++i)
        colors[i] = buffer.palette.GetColor(i);

    if (sketchDef.empty())
        return;
    SketchEffectSketch sketch(SketchEffectSketch::SketchFromString(sketchDef));

    renderSketch(sketch, buffer, progress, 0.01 * drawPercentage, thickness, motionEnabled, 0.01 * motionPercentage, colors);
}

bool SketchEffect::CleanupFileLocations(xLightsFrame* frame, SettingsMap& SettingsMap)
{
    bool rc = false;
    std::string file = SettingsMap["E_FILEPICKER_SketchBackground"];
    if (FileExists(file)) {
        if (!frame->IsInShowFolder(file)) {
            SettingsMap["E_FILEPICKER_SketchBackground"] = frame->MoveToShowFolder(file, std::string(1, std::filesystem::path::preferred_separator) + "Images");
            rc = true;
        }
    }

    return rc;
}

bool SketchEffect::needToAdjustSettings(const std::string& /*version*/)
{
    return false;
}

void SketchEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults /*=true*/)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version)) {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }
}

std::list<std::string> SketchEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    wxLogNull logNo; // suppress popups from png images. See http://trac.wxwidgets.org/ticket/15331
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    if (!xLightsFrame::IsCheckSequenceOptionDisabled("SketchImage")) {
        std::string filename = settings.Get("E_FILEPICKER_SketchBackground", "");
        if (filename.empty() || !FileExists(filename)) {
            // this is only a warning as it does not affect rendering
            res.push_back(std::format("    WARN: Sketch effect cant find image file '{}'. Model '{}', Start {}", filename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
        } else {
            if (!IsFileInShowDir(xLightsFrame::CurrentDir, filename)) {
                res.push_back(std::format("    WARN: Sketch effect image file '{}' not under show directory. Model '{}', Start {}", filename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
            }
        }
    }

    return res;
}

std::list<std::string> SketchEffect::GetFileReferences(Model* model, const SettingsMap& SettingsMap) const
{
    std::list<std::string> res;
    return res;
}

SketchPanel* SketchEffect::getPanel() const
{
    return static_cast<SketchPanel*>(xLightsApp::GetFrame()->effectPanelManager.GetPanel(id, nullptr));
}

double SketchEffect::GetSettingVCMin(const std::string& name) const
{
    if (name == "E_VALUECURVE_DrawPercentage")
        return SketchPanel::DrawPercentageMin;
    if (name == "E_VALUECURVE_Thickness")
        return SketchPanel::ThicknessMin;
    if (name == "E_VALUECURVE_MotionPercentage")
        return SketchPanel::MotionPercentageMin;
    return RenderableEffect::GetSettingVCMin(name);
}

double SketchEffect::GetSettingVCMax(const std::string& name) const
{
    if (name == "E_VALUECURVE_DrawPercentage")
        return SketchPanel::DrawPercentageMax;
    if (name == "E_VALUECURVE_Thickness")
        return SketchPanel::ThicknessMax;
    if (name == "E_VALUECURVE_MotionPercentage")
        return SketchPanel::MotionPercentageMax;
    return RenderableEffect::GetSettingVCMax(name);
}

void SketchEffect::RemoveDefaults(const std::string& version, Effect* effect)
{
}


namespace {
    // Build an NSVGpath from a SketchSubPath.
    // nanosvg format: pts[0],pts[1] = start, then groups of 6: cpx1,cpy1,cpx2,cpy2,x,y
    // npts = 1 + nCubics*3 (each cubic is 3 points: cp1, cp2, end)
    NSVGpath* buildNsvgPath(const SketchSubPath& subPath) {
        int nCubics = (int)subPath.cubics.size() / 6;
        if (nCubics == 0)
            return nullptr;
        int npts = 1 + nCubics * 3;
        // nanosvg is a C library — its structures must be C-allocated
        NSVGpath* path = (NSVGpath*)malloc(sizeof(NSVGpath));
        if (!path) return nullptr;
        std::memset(path, 0, sizeof(NSVGpath));
        path->pts = (float*)malloc(npts * 2 * sizeof(float));
        if (!path->pts) { free(path); return nullptr; }
        path->npts = npts;
        path->closed = 0;
        path->pts[0] = subPath.startX;
        path->pts[1] = subPath.startY;
        std::memcpy(&path->pts[2], subPath.cubics.data(), subPath.cubics.size() * sizeof(float));

        // Bounding box (conservative: uses control points)
        path->bounds[0] = path->bounds[2] = path->pts[0];
        path->bounds[1] = path->bounds[3] = path->pts[1];
        for (int i = 1; i < npts; i++) {
            float x = path->pts[i * 2];
            float y = path->pts[i * 2 + 1];
            if (x < path->bounds[0]) path->bounds[0] = x;
            if (y < path->bounds[1]) path->bounds[1] = y;
            if (x > path->bounds[2]) path->bounds[2] = x;
            if (y > path->bounds[3]) path->bounds[3] = y;
        }
        return path;
    }

    // Build an NSVGshape with stroke from SketchPathData sub-paths.
    NSVGshape* buildNsvgShape(const SketchPathData& pathData, const xlColor& color, float strokeWidth) {
        // nanosvg is a C library — its structures must be C-allocated
        NSVGshape* shape = (NSVGshape*)malloc(sizeof(NSVGshape));
        if (!shape) return nullptr;
        std::memset(shape, 0, sizeof(NSVGshape));
        shape->fill.type = NSVG_PAINT_NONE;
        shape->stroke.type = NSVG_PAINT_COLOR;
        // nanosvg color format is ABGR
        shape->stroke.color = ((unsigned int)color.alpha << 24) |
                              ((unsigned int)color.blue << 16) |
                              ((unsigned int)color.green << 8) |
                              ((unsigned int)color.red);
        shape->opacity = 1.0f;
        shape->strokeWidth = strokeWidth;
        shape->strokeLineJoin = NSVG_JOIN_ROUND;
        shape->strokeLineCap = NSVG_CAP_ROUND;
        shape->miterLimit = 4.0f;
        shape->flags = NSVG_FLAGS_VISIBLE;

        NSVGpath* lastPath = nullptr;
        for (const auto& subPath : pathData.subPaths) {
            NSVGpath* npath = buildNsvgPath(subPath);
            if (npath) {
                if (lastPath)
                    lastPath->next = npath;
                else
                    shape->paths = npath;
                lastPath = npath;
            }
        }

        // If no paths were added, discard the empty shape
        if (!shape->paths) {
            free(shape);
            return nullptr;
        }

        // Compute shape bounds from paths
        shape->bounds[0] = shape->paths->bounds[0];
        shape->bounds[1] = shape->paths->bounds[1];
        shape->bounds[2] = shape->paths->bounds[2];
        shape->bounds[3] = shape->paths->bounds[3];
        for (NSVGpath* p = shape->paths->next; p; p = p->next) {
            if (p->bounds[0] < shape->bounds[0]) shape->bounds[0] = p->bounds[0];
            if (p->bounds[1] < shape->bounds[1]) shape->bounds[1] = p->bounds[1];
            if (p->bounds[2] > shape->bounds[2]) shape->bounds[2] = p->bounds[2];
            if (p->bounds[3] > shape->bounds[3]) shape->bounds[3] = p->bounds[3];
        }
        return shape;
    }

    void freeNsvgImage(NSVGimage* img) {
        if (!img) return;
        NSVGshape* shape = img->shapes;
        while (shape) {
            NSVGshape* next = shape->next;
            NSVGpath* path = shape->paths;
            while (path) {
                NSVGpath* nextPath = path->next;
                free(path->pts);
                free(path);
                path = nextPath;
            }
            free(shape);
            shape = next;
        }
        free(img);
    }
}

void SketchEffect::renderSketch(const SketchEffectSketch& sketch, RenderBuffer& buffer, double progress, double drawPercentage, int lineThickness, bool hasMotion, double motionPercentage, const xlColorVector& colors)
{
    auto paths = sketch.paths();
    int bw = buffer.BufferWi;
    int bh = buffer.BufferHt;

    // In order for the animation to both "draw in" and "draw out" we adjust the [0,1] range...
    double maxProgress = hasMotion ? (1. + motionPercentage) : 1.;
    double adjustedProgress = interpolate(progress, 0., 0., 1., maxProgress, LinearInterpolater());

    // ... but we do a slightly different adjustment for the non-motion case
    if (!hasMotion)
        adjustedProgress = interpolate(progress, 0.0, 0.0, drawPercentage, 1.0, LinearInterpolater());

    double totalLength = 0.;
    for (const auto& path : paths)
        totalLength += path->Length();
    if (totalLength == 0.)
        return;

    // Build NSVGimage with one shape per path (each shape has its own color)
    NSVGimage* nsvgImg = (NSVGimage*)malloc(sizeof(NSVGimage));
    if (!nsvgImg) return;
    std::memset(nsvgImg, 0, sizeof(NSVGimage));
    nsvgImg->width = (float)bw;
    nsvgImg->height = (float)bh;

    NSVGshape* lastShape = nullptr;

    // Single closed path with motion is a special case
    if (hasMotion && paths.size() == 1 && paths.front()->isClosed()) {
        auto sketchPath = paths.front();
        SketchPathData pathData;
        sketchPath->collectPartialPath(pathData, bw, bh, progress, progress + motionPercentage);
        if (progress + motionPercentage > 1.) {
            SketchPathData pathData2;
            sketchPath->collectPartialPath(pathData2, bw, bh, 0., progress + motionPercentage - 1.);
            pathData.append(pathData2);
        }
        NSVGshape* shape = buildNsvgShape(pathData, colors[0], (float)lineThickness);
        if (shape) {
            nsvgImg->shapes = shape;
            lastShape = shape;
        }
    } else {
        double cumulativeLength = 0.;
        int i = 0;
        for (auto iter = paths.cbegin(); iter != paths.cend(); ++iter, ++i) {
            xlColor color = colors[i % colors.size()];
            double pathLength = (*iter)->Length();
            double percentageAtEndOfThisPath = (cumulativeLength + pathLength) / totalLength;

            SketchPathData pathData;
            if (!hasMotion && percentageAtEndOfThisPath <= adjustedProgress)
                (*iter)->collectEntirePath(pathData, bw, bh);
            else {
                double percentageAtStartOfThisPath = cumulativeLength / totalLength;
                double percentageThroughThisPath = calcPercentage(adjustedProgress, percentageAtStartOfThisPath, percentageAtEndOfThisPath);
                if (!hasMotion)
                    (*iter)->collectPartialPath(pathData, bw, bh, std::nullopt, percentageThroughThisPath);
                else {
                    double drawPercentageThroughThisPath = calcPercentage(adjustedProgress - motionPercentage, percentageAtStartOfThisPath, percentageAtEndOfThisPath);
                    drawPercentageThroughThisPath = std::clamp(drawPercentageThroughThisPath, 0., 1.);
                    (*iter)->collectPartialPath(pathData, bw, bh, drawPercentageThroughThisPath, percentageThroughThisPath);
                }
            }

            if (!pathData.empty()) {
                NSVGshape* shape = buildNsvgShape(pathData, color, (float)lineThickness);
                if (shape) {
                    if (lastShape)
                        lastShape->next = shape;
                    else
                        nsvgImg->shapes = shape;
                    lastShape = shape;
                }
            }
            cumulativeLength += pathLength;
        }
    }

    // Get or create the render cache for the rasterizer and buffer
    SketchRenderCache* cache = (SketchRenderCache*)buffer.infoCache[id];
    if (!cache) {
        cache = new SketchRenderCache();
        buffer.infoCache[id] = cache;
    }

    // Rasterize with nanosvg (rasterizer and buffer are cached across frames)
    size_t bufSize = (size_t)bw * bh * 4;
    cache->rasterBuf.resize(bufSize);
    std::memset(cache->rasterBuf.data(), 0, bufSize);
    nsvgRasterize(cache->rasterizer, nsvgImg, 0, 0, 1.0f, cache->rasterBuf.data(), bw, bh, bw * 4);

    // Composite rasterized image into RenderBuffer.
    // Both the old wxImage path and nanosvg use y=0 at top, and the buffer
    // copy is done linearly (no flip), matching the original behavior.
    for (int y = 0; y < bh; ++y) {
        const uint8_t* row = &cache->rasterBuf[y * bw * 4];
        for (int x = 0; x < bw; ++x) {
            const uint8_t* p = &row[x * 4];
            if (p[3] > 0) {
                xlColor src(p[0], p[1], p[2], p[3]);
                buffer.SetPixel(x, y, src, false, true);
            }
        }
    }

    freeNsvgImage(nsvgImg);
}

