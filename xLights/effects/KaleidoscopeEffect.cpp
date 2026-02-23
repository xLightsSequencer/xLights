/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <sstream>

#include "../../include/kaleidoscope-64.xpm"
#include "../../include/kaleidoscope-48.xpm"
#include "../../include/kaleidoscope-32.xpm"
#include "../../include/kaleidoscope-24.xpm"
#include "../../include/kaleidoscope-16.xpm"

#include "KaleidoscopeEffect.h"
#include "KaleidoscopePanel.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../xLightsMain.h"
#include "../xLightsApp.h"
#include "../TimingPanel.h"
#include "UtilFunctions.h"

#include "../Parallel.h"
#include <log4cpp/Category.hh>

KaleidoscopeEffect::KaleidoscopeEffect(int i) : RenderableEffect(i, "Kaleidoscope", kaleidoscope_16, kaleidoscope_24, kaleidoscope_32, kaleidoscope_48, kaleidoscope_64)
{
}

KaleidoscopeEffect::~KaleidoscopeEffect()
{
}

xlEffectPanel *KaleidoscopeEffect::CreatePanel(wxWindow *parent)
{
    return new KaleidoscopePanel(parent);
}

std::list<std::string> KaleidoscopeEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    if (settings.Get("T_CHECKBOX_Canvas", "0") == "0")
    {
        res.push_back(wxString::Format("    WARN: Canvas mode not enabled on a Kaleidoscope effect. Without canvas mode Kaleidoscope won't do anything. Effect: Kaleidoscope, Model: %s, Start %s", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

void KaleidoscopeEffect::SetDefaultParameters()
{
    KaleidoscopePanel *p = (KaleidoscopePanel *)panel;

    p->BitmapButton_Kaleidoscope_X->SetActive(false);
    p->BitmapButton_Kaleidoscope_Y->SetActive(false);
    p->BitmapButton_Kaleidoscope_Size->SetActive(false);
    p->BitmapButton_Kaleidoscope_Rotation->SetActive(false);

    p->Choice_Kaleidoscope_Type->SetSelection(0);

    SetSliderValue(p->Slider_Kaleidoscope_X, 50);
    SetSliderValue(p->Slider_Kaleidoscope_Y, 50);
    SetSliderValue(p->Slider_Kaleidoscope_Size, 5);
    SetSliderValue(p->Slider_Kaleidoscope_Rotation, 0);

    // Turn on canvas mode as this really only makes sense in canvas mode
    xLightsFrame* frame = xLightsApp::GetFrame();
    TimingPanel* layerBlendingPanel = frame->GetLayerBlendingPanel();
    layerBlendingPanel->CheckBox_Canvas->SetValue(true);
}

class KaleidoscopeRenderCache : public EffectRenderCache {
public:
    KaleidoscopeRenderCache()
    {
        _size = -1;
        _rotation = -1;
        _x = -1;
        _y = -1;
        _width = -1;
        _height = -1;
    };
    virtual ~KaleidoscopeRenderCache() {};
    int _size;
    int _rotation;
    int _x;
    int _y;
    int _width;
    int _height;
    void Initialise(int size, int rotation, int x, int y, int width, int height, const std::string& type, bool force)
    {
        if (force || size != _size || rotation != _rotation || x != _x || y != _y || width != _width || height != _height)
        {
            _size = size;
            _rotation = rotation;
            _x = x;
            _y = y;
            _width = width;
            _height = height;

            // clear the used arrary
            _startUsed.resize(width);
            for (auto& xx : _startUsed)
            {
                xx.resize(height);
                for (auto yy = xx.begin(); yy != xx.end(); ++yy)
                {
                    *yy = false;
                }
            }
            _edges.clear();

            // now repopulate
            if (type == "Square")
            {
                InitialiseSquare(size, rotation, x, y, width, height);
            }
            else if (type == "Triangle")
            {
                InitialiseTriangle(size, rotation, x, y, width, height);
            }
            else
            {
                wxASSERT(false);
            }
        }
    }

    bool CreateEdge(int x1, int y1, int x2, int y2)
    {
        if (x1 < 0 && x2 < 0) return false;
        if (x1 > _width && x2 > _width) return false;
        if (y1 < 0 && y2 < 0) return false;
        if (y1 > _height && y2 > _height) return false;

        if (x1 != x2 && y1 != y2)
        {
            bool bl = IsPointAboveLineThroughPoints(0, 0, x1, y1, x2, y2);
            bool tl = IsPointAboveLineThroughPoints(0, _height - 1, x1, y1, x2, y2);
            bool tr = IsPointAboveLineThroughPoints(_width - 1, _height - 1, x1, y1, x2, y2);
            bool br = IsPointAboveLineThroughPoints(_width - 1, 0, x1, y1, x2, y2);

            // if all corners are on the same side of the line then this line isnt interesting
            if (bl == tl && bl == tr && bl == br) return false;
        }

        _edges.push_back(KaleidoscopeEdge(wxPoint(x1, y1), wxPoint(x2, y2)));
        return true;
    }

    bool IsPointAboveLineThroughPoints(int x, int y, int x1, int y1, int x2, int y2)
    {
        double slope = (double)(y2 - y1) / (double)(x2 - x1);
        double yintercept = slope * -1.0 * (double)x1 + (double)y1;
        double ytest = slope * x + yintercept;
        return (double)y > ytest;
    }

    std::pair<int,int> GetPointAfterMove(int x, int y, int degrees, double d)
    {
        double a = toRadians(degrees);
        double dx = d * cos(a);

        int f = 1;
        if (dx < 0) f = -1;

        double aa = std::abs(std::abs(dx) - (int)std::abs(dx) - 0.5);
        if (aa < 0.0000001)
        {
            dx = f * ((int)std::abs(dx) + 0.5);
        }
        double dy = d * sin(a);
        f = 1;
        if (dy < 0) f = -1;
        double bb = std::abs(std::abs(dy) - (int)std::abs(dy) - 0.5);
        if (bb < 0.0000001)
        {
            dy = f * ((int)std::abs(dy) + 0.5);
        }
        return { x + std::round(dx), y + std::round(dy) };
    }

    void InitialiseSquare(int size, int rotation, int x, int y, int width, int height)
    {
        for (int xx = std::max(0, x - size / 2); xx <= std::min(x + size / 2, width -1); xx++)
        {
            for (int yy = std::max(0, y - size / 2); yy <= std::min(y + size / 2, height -1); yy++)
            {
                if (xx >= 0 && xx < width && yy >= 0 && yy < height)
                {
                    _startUsed[xx][yy] = true;
                }
            }
        }

        double cornerdistance = sqrt(2.0 * size * size ) / 2.0;
        int iterations = 0;
        int added;
        do
        {
            added = 0;
            auto p1 = GetPointAfterMove(x, y, rotation + 45, (2 * iterations + 1) * cornerdistance);
            auto p2 = GetPointAfterMove(x, y, rotation + 45 + 90, (2 * iterations + 1) * cornerdistance);
            auto p3 = GetPointAfterMove(x, y, rotation + 45 + 180, (2 * iterations + 1) * cornerdistance);
            auto p4 = GetPointAfterMove(x, y, rotation + 45 + 270, (2 * iterations + 1) * cornerdistance);

            if (CreateEdge(p1.first, p1.second, p2.first, p2.second)) added++;
            if (CreateEdge(p2.first, p2.second, p3.first, p3.second)) added++;
            if (CreateEdge(p3.first, p3.second, p4.first, p4.second)) added++;
            if (CreateEdge(p4.first, p4.second, p1.first, p1.second)) added++;

            iterations++;
        } while (added > 0);
    }

    void InitialiseTriangle(int size, int rotation, int x, int y, int width, int height)
    {
        const double radiusFactor = tan(toRadians(30)) / 2.0;
        double radius = size * radiusFactor;

        int minx = GetPointAfterMove(x, y, 240, radius).first;
        int miny = GetPointAfterMove(x, y, 240, radius).second;
        int maxx = GetPointAfterMove(x, y, 300, radius).first;
        int maxy = GetPointAfterMove(x, y, 90, radius).second;
        int midx = (maxx + minx) / 2;

        for (int xx = std::max(0,minx); xx < std::min(midx, width -1); xx++)
        {
            for (int yy = std::max(0, miny); yy <= std::min(maxy, height - 1); yy++)
            {
                if (!IsPointAboveLineThroughPoints(xx, yy, minx, miny, midx, maxy))
                {
                    _startUsed[xx][yy] = true;
                }
            }
        }
        for (int xx = std::max(0,midx); xx <= std::min(maxx, width -1); xx++)
        {
            for (int yy = std::max(0,miny); yy <= std::min(maxy, height -1); yy++)
            {
                if (!IsPointAboveLineThroughPoints(xx, yy, midx, maxy, maxx, miny))
                {
                    _startUsed[xx][yy] = true;
                }
            }
        }

        // bring the edges inside the shape area
        if (radius > 10)
        {
            radius -= 1;
        }
        else if (radius > 5)
        {
            radius -= 0.5;
        }
        else
        {
            radius -= 0.1;
        }

        double s = size;
        double gap = sqrt(s * s - s / 2.0 * s / 2.0);
        int iterations = 0;
        int added;
        do
        {
            added = 0;
            if (iterations == 0)
            {
                auto p1 = GetPointAfterMove(x, y, rotation + 90, radius);
                auto p2 = GetPointAfterMove(x, y, rotation + 240, radius);
                auto p3 = GetPointAfterMove(x, y, rotation + 300, radius);

                if (CreateEdge(p1.first, p1.second, p2.first, p2.second)) added++;
                if (CreateEdge(p2.first, p2.second, p3.first, p3.second)) added++;
                if (CreateEdge(p3.first, p3.second, p1.first, p1.second)) added++;
                if (rotation == 0)
                {
                    wxASSERT(p2.second == p3.second);
                }
            }
            else
            {
                auto p1 = GetPointAfterMove(x, y, rotation + 90, iterations * gap + radius);
                auto p2 = GetPointAfterMove(x, y, rotation + 240, iterations * gap + radius);
                auto p3 = GetPointAfterMove(x, y, rotation + 300, iterations * gap + radius);

                if (CreateEdge(p1.first, p1.second, p2.first, p2.second)) added++;
                if (CreateEdge(p2.first, p2.second, p3.first, p3.second)) added++;
                if (CreateEdge(p3.first, p3.second, p1.first, p1.second)) added++;
            }

            iterations++;
        } while (added > 0);
    }

    std::vector<std::vector<bool>> _startUsed;
    std::list<KaleidoscopeEdge> _edges;
};

bool KaleidoscopeEffect::KaleidoscopeDone(const std::vector<std::vector<bool>>& current)
{
    for (const auto& xx : current)
    {
        for (const auto yy : xx)
        {
            if (!yy) return false;
        }
    }
    return true;
}

std::pair<int, int> KaleidoscopeEffect::GetSourceLocation(int x, int y, const KaleidoscopeEdge& edge, int width, int height)
{
    double x1 = edge._p1.x;
    double x2 = edge._p2.x;
    double y1 = edge._p1.y;
    double y2 = edge._p2.y;

    double dx = x2 - x1;
    double dy = y2 - y1;

    double a = (dx * dx - dy * dy) / (dx * dx + dy * dy);
    double b = 2.0 * dx * dy / (dx*dx + dy * dy);

    return { std::round(a * (x - x1) + b * (y - y1) + x1),
            std::round(b * (x - x1) - a * (y - y1) + y1) };
}

void DumpUsed(const std::vector<std::vector<bool>>& current, int width, int height)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    for (int y = height - 1; y >= 0; y--)
    {
        std::string row;
        for (int x = 0; x < width; x++)
        {
            bool b = current[x][y];
            row += wxString::Format(" %d", (int)b);
        }
        logger_base.debug(row);
    }
}


double KaleidoscopeEffect::SignedDist(double px, double py, double lx1, double ly1, double lx2, double ly2) {
    return (lx2 - lx1) * (py - ly1) - (ly2 - ly1) * (px - lx1);
}

void KaleidoscopeEffect::ReflectPointAcrossLine(double& px, double& py, double lx1, double ly1, double lx2, double ly2) {
    double dx = lx2 - lx1;
    double dy = ly2 - ly1;
    double denom = dx * dx + dy * dy;
    double a = (dx * dx - dy * dy) / denom;
    double b = 2.0 * dx * dy / denom;
    double rx = px - lx1;
    double ry = py - ly1;
    px = a * rx + b * ry + lx1;
    py = b * rx - a * ry + ly1;
}

double KaleidoscopeEffect::ReflectCoord(double v, double halfSize) {
    double period = 4.0 * halfSize;
    v = std::fmod(v + halfSize, period);
    if (v < 0.0)
        v += period;
    if (v <= 2.0 * halfSize) {
        return v - halfSize;
    } else {
        return 3.0 * halfSize - v;
    }
}

// Compute triangle vertices for a given type, centered at (cx,cy) with given
// size and rotation. Vertices are ensured to be in counter-clockwise order.
KaleidoscopeTriangle KaleidoscopeEffect::ComputeTriangle(const std::string& type, double cx, double cy, double size, double rotRad) {
    KaleidoscopeTriangle tri;

    if (type == "6-Fold") {
        // Equilateral triangle (60-60-60) - classic 6-fold kaleidoscope
        // Circumradius = size / sqrt(3), using size as side length
        double R = size / std::sqrt(3.0);

        // Three vertices at 120 degree intervals
        for (int i = 0; i < 3; i++) {
            double angle = rotRad + toRadians(90.0 + 120.0 * i);
            tri.v[i].x = cx + R * std::cos(angle);
            tri.v[i].y = cy + R * std::sin(angle);
        }
    } else if (type == "8-Fold") {
        // Right isosceles triangle (45-45-90) - 8-fold kaleidoscope
        // Legs of length = size, centroid at center
        double leg = size;

        // Unrotated vertices relative to centroid:
        // Right angle at (-leg/3, -leg/3)
        // Other two at (2*leg/3, -leg/3) and (-leg/3, 2*leg/3)
        double v0x = -leg / 3.0, v0y = -leg / 3.0;
        double v1x = 2.0 * leg / 3.0, v1y = -leg / 3.0;
        double v2x = -leg / 3.0, v2y = 2.0 * leg / 3.0;

        double cosR = std::cos(rotRad);
        double sinR = std::sin(rotRad);

        tri.v[0].x = cx + v0x * cosR - v0y * sinR;
        tri.v[0].y = cy + v0x * sinR + v0y * cosR;
        tri.v[1].x = cx + v1x * cosR - v1y * sinR;
        tri.v[1].y = cy + v1x * sinR + v1y * cosR;
        tri.v[2].x = cx + v2x * cosR - v2y * sinR;
        tri.v[2].y = cy + v2x * sinR + v2y * cosR;
    } else if (type == "12-Fold") {
        // 30-60-90 triangle - 12-fold kaleidoscope
        // Hypotenuse = size, short leg = size/2, long leg = size * sqrt(3)/2
        double hyp = size;
        double shortLeg = hyp / 2.0;
        double longLeg = hyp * std::sqrt(3.0) / 2.0;

        // Unrotated vertices relative to centroid:
        // Right angle at origin, short leg along x, long leg along y
        // Centroid of right triangle = (shortLeg/3, longLeg/3)
        double centX = shortLeg / 3.0;
        double centY = longLeg / 3.0;

        double v0x = -centX;
        double v0y = -centY; // right angle
        double v1x = shortLeg - centX;
        double v1y = -centY; // end of short leg
        double v2x = -centX;
        double v2y = longLeg - centY; // end of long leg

        double cosR = std::cos(rotRad);
        double sinR = std::sin(rotRad);

        tri.v[0].x = cx + v0x * cosR - v0y * sinR;
        tri.v[0].y = cy + v0x * sinR + v0y * cosR;
        tri.v[1].x = cx + v1x * cosR - v1y * sinR;
        tri.v[1].y = cy + v1x * sinR + v1y * cosR;
        tri.v[2].x = cx + v2x * cosR - v2y * sinR;
        tri.v[2].y = cy + v2x * sinR + v2y * cosR;
    }

    // Ensure counter-clockwise vertex order (required for SignedDist to work)
    double signedArea = (tri.v[1].x - tri.v[0].x) * (tri.v[2].y - tri.v[0].y) -
                        (tri.v[2].x - tri.v[0].x) * (tri.v[1].y - tri.v[0].y);
    if (signedArea < 0.0) {
        std::swap(tri.v[1], tri.v[2]);
    }

    return tri;
}

// Map a pixel back to the source triangle by iterative reflection.
// Each reflection mirrors the point across the triangle edge it is outside of.

std::pair<int, int> KaleidoscopeEffect::MapToSourceTriangle(double px, double py, const KaleidoscopeTriangle& tri, int maxIter) {
    double x = px;
    double y = py;

    for (int i = 0; i < maxIter; i++) {
        double d0 = SignedDist(x, y, tri.v[0].x, tri.v[0].y, tri.v[1].x, tri.v[1].y);
        double d1 = SignedDist(x, y, tri.v[1].x, tri.v[1].y, tri.v[2].x, tri.v[2].y);
        double d2 = SignedDist(x, y, tri.v[2].x, tri.v[2].y, tri.v[0].x, tri.v[0].y);

        // Inside the triangle (small tolerance for rounding)
        if (d0 >= -0.5 && d1 >= -0.5 && d2 >= -0.5) {
            return { (int)std::round(x), (int)std::round(y) };
        }

        // Reflect across the edge we are most outside of
        if (d0 < d1 && d0 < d2) {
            ReflectPointAcrossLine(x, y, tri.v[0].x, tri.v[0].y, tri.v[1].x, tri.v[1].y);
        } else if (d1 < d2) {
            ReflectPointAcrossLine(x, y, tri.v[1].x, tri.v[1].y, tri.v[2].x, tri.v[2].y);
        } else {
            ReflectPointAcrossLine(x, y, tri.v[2].x, tri.v[2].y, tri.v[0].x, tri.v[0].y);
        }
    }

    // Didn't converge - return invalid
    return { -1, -1 };
}

// Map a pixel back to the source square using coordinate folding.
// Works by rotating into the square's local frame, folding both axes
// via triangle-wave reflection, then rotating back to world space.
// This is mathematically exact - no iteration needed.
std::pair<int, int> KaleidoscopeEffect::MapToSourceNewSquare(double px, double py, double cx, double cy, double halfSize, double rotRad) {
    // Transform to local coordinate frame (centered, unrotated)
    double dx = px - cx;
    double dy = py - cy;
    double cosR = std::cos(-rotRad);
    double sinR = std::sin(-rotRad);
    double lx = dx * cosR - dy * sinR;
    double ly = dx * sinR + dy * cosR;

    // Fold both coordinates into [-halfSize, +halfSize]
    lx = ReflectCoord(lx, halfSize);
    ly = ReflectCoord(ly, halfSize);

    // Transform back to world coordinates
    cosR = std::cos(rotRad);
    sinR = std::sin(rotRad);
    double wx = lx * cosR - ly * sinR + cx;
    double wy = lx * sinR + ly * cosR + cy;

    return { (int)std::round(wx), (int)std::round(wy) };
}

// ============================================================================
// For Square 2, 6-Fold, 8-Fold, 12-Fold, Radial
// ============================================================================

void KaleidoscopeEffect::RenderNew(const std::string& type, int xCentre, int yCentre, int size, int rotation, RenderBuffer& buffer) {
    int width = buffer.BufferWi;
    int height = buffer.BufferHt;
    double cx = (double)xCentre;
    double cy = (double)yCentre;
    double rotRad = toRadians((double)rotation);

    if (type == "Radial") {
        int segments = std::max(2, size);
        double wedgeAngle = 2.0 * M_PI / (double)segments;

        parallel_for(0, height, [&](int y) {
            for (int x = 0; x < width; x++) {
                double dx = (double)x - cx;
                double dy = (double)y - cy;
                double radius = std::sqrt(dx * dx + dy * dy);

                if (radius < 0.5)
                    continue;

                double angle = std::atan2(dy, dx);

                angle -= rotRad;

                angle = std::fmod(angle, 2.0 * M_PI);
                if (angle < 0.0)
                    angle += 2.0 * M_PI;

                double folded = std::fmod(angle, 2.0 * wedgeAngle);
                if (folded > wedgeAngle)
                    folded = 2.0 * wedgeAngle - folded;

                folded += rotRad;

                int sx = (int)std::round(cx + radius * std::cos(folded));
                int sy = (int)std::round(cy + radius * std::sin(folded));

                if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
                    if (sx != x || sy != y) {
                        buffer.SetPixel(x, y, buffer.GetPixel(sx, sy));
                    }
                }
            }
        });
        return;
    }

    if (type == "Square 2") {
        double halfSize = size / 2.0;

        parallel_for(0, height, [&](int y) {
            for (int x = 0; x < width; x++) {
                auto src = MapToSourceNewSquare((double)x, (double)y, cx, cy, halfSize, rotRad);
                if (src.first >= 0 && src.first < width && src.second >= 0 && src.second < height) {
                    if (src.first != x || src.second != y) {
                        buffer.SetPixel(x, y, buffer.GetPixel(src.first, src.second));
                    }
                }
            }
        });
    } else {
        KaleidoscopeTriangle tri = ComputeTriangle(type, cx, cy, (double)size, rotRad);

        // Scale max iterations to buffer/size ratio with a reasonable cap
        int maxDim = std::max(width, height);
        int maxIter = std::max(50, (maxDim * 3) / std::max(size, 1));
        if (maxIter > 500)
            maxIter = 500;

        parallel_for(0, height, [&](int y) {
            for (int x = 0; x < width; x++) {
                auto src = MapToSourceTriangle((double)x, (double)y, tri, maxIter);
                if (src.first >= 0 && src.first < width && src.second >= 0 && src.second < height) {
                    if (src.first != x || src.second != y) {
                        buffer.SetPixel(x, y, buffer.GetPixel(src.first, src.second));
                    }
                }
            }
        });
    }
}


void KaleidoscopeEffect::Render(Effect *eff, const SettingsMap &SettingsMap, RenderBuffer &buffer)
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    float progress = buffer.GetEffectTimeIntervalPosition(1.f);

    std::string type = SettingsMap.Get("CHOICE_Kaleidoscope_Type", "Triangle");
    int xCentre = GetValueCurveInt("Kaleidoscope_X", 50, SettingsMap, progress, KALEIDOSCOPE_X_MIN, KALEIDOSCOPE_X_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) * buffer.BufferWi / 100;
    int yCentre = GetValueCurveInt("Kaleidoscope_Y", 50, SettingsMap, progress, KALEIDOSCOPE_Y_MIN, KALEIDOSCOPE_Y_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) * buffer.BufferHt / 100;
    int size = GetValueCurveInt("Kaleidoscope_Size", 5, SettingsMap, progress, KALEIDOSCOPE_SIZE_MIN, KALEIDOSCOPE_SIZE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int rotation = GetValueCurveInt("Kaleidoscope_Rotation", 0, SettingsMap, progress, KALEIDOSCOPE_ROTATION_MIN, KALEIDOSCOPE_ROTATION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());


    if (type == "Square 2" || type == "6-Fold" || type == "8-Fold" || type == "12-Fold" || type == "Radial") {
        RenderNew(type, xCentre, yCentre, size, rotation, buffer);
        return;
    }


    KaleidoscopeRenderCache *cache = static_cast<KaleidoscopeRenderCache*>(buffer.infoCache[id]);
    if (cache == nullptr) {
        cache = new KaleidoscopeRenderCache();
        buffer.infoCache[id] = cache;
    }

    if (buffer.needToInit)
    {
        buffer.needToInit = false;
        cache->Initialise(size, rotation, xCentre, yCentre, buffer.BufferWi, buffer.BufferHt, type, true);
    }
    else
    {
        // reinitialise ... but only if something has changed
        cache->Initialise(size, rotation, xCentre, yCentre, buffer.BufferWi, buffer.BufferHt, type, false);
    }

    auto currentUsed = cache->_startUsed;
    auto &edges = cache->_edges;

    auto edge = edges.begin();
    //logger_base.debug("frame. Edges %d", (int)edges.size());
    std::atomic_int setSinceBegin;
    setSinceBegin = 0;
    while (!KaleidoscopeDone(currentUsed) && edges.size() > 0)
    {
        //logger_base.debug("   iterate");
        //int set = 0;

        //DumpUsed(currentUsed, buffer.BufferWi, buffer.BufferHt);
        parallel_for(0, buffer.BufferHt, [&currentUsed, this, &buffer, &edge, &setSinceBegin] (int y) {
            for (int x = 0; x < buffer.BufferWi; x++) {
                if (!currentUsed[x][y]) {
                    // this pixel needs to be set
                    auto source = GetSourceLocation(x, y, *edge, buffer.BufferWi, buffer.BufferHt);
                    if (source.first >= 0 && source.first < buffer.BufferWi && source.second >= 0 && source.second < buffer.BufferHt) {
                        if (currentUsed[source.first][source.second]) {
                            buffer.SetPixel(x, y, buffer.GetPixel(source.first, source.second));
                            currentUsed[x][y] = true;
                            //set++;
                            setSinceBegin++;
                        }
                    }
                }
            }
        });
        //logger_base.debug("   set this iteration %d", set);
        ++edge;
        if (edge == edges.end()) {
            if (setSinceBegin == 0)
            {
                break;
            }
            setSinceBegin = 0;
            edge = edges.begin();
        }
    }
}
