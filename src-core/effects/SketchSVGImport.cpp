#include "SketchSVGImport.h"

#include "SketchEffectDrawing.h"
#include "../utils/xlPoint.h"
#include "../utils/nanosvg_xl.h"

#include <cmath>
#include <memory>

namespace {
bool areSame(double a, double b, double eps) {
    return std::fabs(a - b) < eps;
}
bool areCollinear(const xlPointD& a, const xlPointD& b, const xlPointD& c, double eps) {
    auto [ax, ay] = a;
    auto [bx, by] = b;
    auto [cx, cy] = c;
    auto test = (bx - ax) * (cy - ay) - (cx - ax) * (by - ay);
    return std::abs(test) < eps;
}
}

std::string SketchDefFromSVGFile(const std::string& filename) {
    // 100px scaling matches the desktop assist panel's call.
    NSVGimage* image = nsvgParseFromFile(filename.c_str(), "px", 100);
    if (image == nullptr)
        return std::string();

    SketchEffectSketch sketch;
    float h = image->height;
    float w = image->width;
    if (w <= 0 || h <= 0) {
        nsvgDelete(image);
        return std::string();
    }

    for (NSVGshape* shape = image->shapes; shape != nullptr; shape = shape->next) {
        for (NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
            auto skpath = std::make_shared<SketchEffectPath>();
            for (int i = 0; i < path->npts - 1; i += 3) {
                float* p = &path->pts[i * 2];
                // sketch coords are 0..1; y is flipped relative to SVG pixels.
                xlPointD start(p[0] / w, 1 - (p[1] / h));
                xlPointD cp1(p[2] / w, 1 - (p[3] / h));
                xlPointD cp2(p[4] / w, 1 - (p[5] / h));
                xlPointD end(p[6] / w, 1 - (p[7] / h));
                if (areCollinear(start, cp1, end, 0.001) && areCollinear(start, cp2, end, 0.001)) {
                    skpath->appendSegment(std::make_shared<SketchLine>(start, end));
                } else if (areSame(end.x, cp2.x, 0.001) && areSame(end.y, cp2.y, 0.001)) {
                    skpath->appendSegment(std::make_shared<SketchQuadraticBezier>(start, cp1, end));
                } else {
                    skpath->appendSegment(std::make_shared<SketchCubicBezier>(start, cp1, cp2, end));
                }
            }
            if (!skpath->segments().empty())
                sketch.appendPath(skpath);
        }
    }
    nsvgDelete(image);
    return sketch.toString();
}
