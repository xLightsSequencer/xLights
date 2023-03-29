/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "RippleEffect.h"
#include "RipplePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../ExternalHooks.h"
#include "../models/Model.h"
#include "../xLightsMain.h"

#include "nanosvg/src/nanosvg.h"

#include "../../include/ripple-16.xpm"
#include "../../include/ripple-24.xpm"
#include "../../include/ripple-32.xpm"
#include "../../include/ripple-48.xpm"
#include "../../include/ripple-64.xpm"

RippleEffect::RippleEffect(int id) : RenderableEffect(id, "Ripple", ripple_16, ripple_24, ripple_32, ripple_48, ripple_64)
{
    //ctor
}

RippleEffect::~RippleEffect()
{
    //dtor
}
xlEffectPanel *RippleEffect::CreatePanel(wxWindow *parent) {
    return new RipplePanel(parent);
}

#define RENDER_RIPPLE_CIRCLE     0
#define RENDER_RIPPLE_SQUARE     1
#define RENDER_RIPPLE_TRIANGLE   2
#define RENDER_RIPPLE_STAR       3
#define RENDER_RIPPLE_POLYGON    4
#define RENDER_RIPPLE_HEART      5
#define RENDER_RIPPLE_TREE       6
#define RENDER_RIPPLE_CANDYCANE  7
#define RENDER_RIPPLE_SNOWFLAKE  8
#define RENDER_RIPPLE_CRUCIFIX   9
#define RENDER_RIPPLE_PRESENT    10
#define RENDER_RIPPLE_SVG        11

#define MOVEMENT_EXPLODE    0
#define MOVEMENT_IMPLODE    1
#define MOVEMENT_NONE       2

void RippleEffect::SetDefaultParameters()
{
    RipplePanel* rp = (RipplePanel*)panel;
    if (rp == nullptr) {
        return;
    }

    rp->BitmapButton_Ripple_CyclesVC->SetActive(false);
    rp->BitmapButton_Ripple_ThicknessVC->SetActive(false);
    rp->BitmapButton_Ripple_RotationVC->SetActive(false);
    rp->BitmapButton_Ripple_XCVC->SetActive(false);
    rp->BitmapButton_Ripple_YCVC->SetActive(false);

    rp->BitmapButton_Ripple_SpacingVC->SetActive(false);
    rp->BitmapButton_Ripple_ScaleVC->SetActive(false);
    rp->BitmapButton_Ripple_TwistVC->SetActive(false);
    rp->BitmapButton_Ripple_VelocityVC->SetActive(false);
    rp->BitmapButton_Ripple_DirectionVC->SetActive(false);
    rp->BitmapButton_Ripple_OutlineVC->SetActive(false);

    SetChoiceValue(rp->Choice_Ripple_Object_To_Draw, "Circle");
    SetChoiceValue(rp->Choice_Ripple_Movement, "Explode");

    SetSliderValue(rp->Slider_Ripple_Thickness, 3);
    SetSliderValue(rp->Slider_Ripple_Cycles, 10);
    SetSliderValue(rp->Slider_Ripple_Points, 5);
    SetSliderValue(rp->Slider_Ripple_Rotation, 0);

    SetSliderValue(rp->Slider_Ripple_Scale, 100);
    SetSliderValue(rp->Slider_Ripple_Direction, 0);
    SetSliderValue(rp->Slider_Ripple_Velocity, 0);
    SetSliderValue(rp->Slider_Ripple_Twist, 0);
    SetSliderValue(rp->Slider_Ripple_Spacing, 10);
    SetSliderValue(rp->Slider_Ripple_Outline, 10);

    rp->FilePickerCtrl_SVG->SetFileName(wxFileName(""));

    SetCheckBoxValue(rp->CheckBox_Ripple3D, false);
    SetChoiceValue(rp->Choice_Ripple_Draw_Style, "Old");
}

typedef std::pair<double, double> dpoint;
typedef std::pair<int, int> ipoint;
typedef std::vector<dpoint> dpointvec;
typedef std::vector<ipoint> ipointvec;

// It is somewhat redundant with 3-sided polygon isn't it?
//  Especially now that it is equilateral
static void getTrianglePoints(dpointvec& tpts)
{
    tpts.resize(3);
#define ROOT3DIV3 0.577350269
#define SIN30 0.5
#define COS30 0.866025404

    tpts[0] = { 0, 1 };
    tpts[1] = { -COS30, -SIN30 };
    tpts[2] = { COS30, -SIN30 };
}

// Now see, the square becomes a rectangle later, if desired
// It is somewhat redundant with 4-side polygon isn't it...
static void getSquarePoints(dpointvec& spts)
{
    spts.resize(4);
    spts[0] = { 1, 1 };
    spts[1] = { 1, -1 };
    spts[2] = { -1, -1 };
    spts[3] = { -1, 1 };
}

// Get polygon points; someone else applies that
static void getPolygonPoints(dpointvec& ppts, int points)
{
    double rotation = 0;
    if (points % 2 != 0)
        rotation += 90;
    if (points == 4)
        rotation += 45;
    if (points == 8)
        rotation += 22.5;

    double increment = 360.0 / points;
    ppts.resize(points);

    for (int i = 0; i < points; ++i, rotation += increment) {
        double radian = (rotation)*M_PI / 180.0;
        ppts[i] = { cos(radian), sin(radian) };
    }
}

// OK can we just admit it is a polygon with a lot of points?
static void getCirclePoints(dpointvec& ppts)
{
    getPolygonPoints(ppts, 100);
}

static void getCrossPoints(dpointvec& ppts)
{
    const wxPoint points[] = { wxPoint(2, 0),
                               wxPoint(2, 6),
                               wxPoint(0, 6),
                               wxPoint(0, 7),
                               wxPoint(2, 7),
                               wxPoint(2, 10),
                               wxPoint(3, 10),
                               wxPoint(3, 7),
                               wxPoint(5, 7),
                               wxPoint(5, 6),
                               wxPoint(3, 6),
                               wxPoint(3, 0) };
    ppts.clear();
    for (const auto& pt : points) {
        ppts.push_back({ (pt.x - 2.5) / 7.0, (pt.y - 6.5) / 10 });
    }
}

static void getTreePoints(dpointvec& ppts)
{
    const wxPoint points[] = {
        wxPoint(3, 3),
        wxPoint(3, 0),
        wxPoint(5, 0),
        wxPoint(5, 3),
        wxPoint(0, 3),
        wxPoint(2, 6),
        wxPoint(1, 6),
        wxPoint(3, 9),
        wxPoint(2, 9),
        wxPoint(4, 11),
        wxPoint(6, 9),
        wxPoint(5, 9),
        wxPoint(7, 6),
        wxPoint(6, 6),
        wxPoint(8, 3)
    };
    ppts.clear();
    for (const auto& pt : points) {
        ppts.push_back({ (pt.x - 4.0) / 11.0, (pt.y - 5.5) / 11.0 });
    }
}

static void getPresentPoints(dpointvec& ppts)
{
    const wxPoint points[] = {
        wxPoint(5, 9),
        wxPoint(2, 11),
        wxPoint(2, 9),
        wxPoint(5, 9),
        wxPoint(8, 11),
        wxPoint(8, 9),
        wxPoint(5, 9),
        wxPoint(0, 9),
        wxPoint(0, 0),
        wxPoint(10, 0),
        wxPoint(10, 9),
        wxPoint(5, 9),
        wxPoint(5, 0)
    };

    for (const auto& pt : points) {
        ppts.push_back({ (pt.x - 5.0) / 7.0, (pt.y - 5.5) / 10.0 });
    }
}

static void getHeartPoints(dpointvec& pts)
{
    dpointvec rpts;

    double xincr = 1.0 / 64.0;
    for (double x = -2.0; x <= 2.0; x += xincr) {
        double y1 = std::sqrt(1.0 - (std::abs(x) - 1.0) * (std::abs(x) - 1.0));
        double y2 = std::acos(1.0 - std::abs(x)) - M_PI;

        double xx1 = x / 2.0;
        double yy1 = y1 / 2.0;
        double yy2 = y2 / 2.0;

        pts.push_back({ xx1, yy1+.2 });
        rpts.push_back({ xx1, yy2+.2 });
    }

    while (!rpts.empty()) {
        pts.push_back(rpts.back());
        rpts.pop_back();
    }
}

static void getCanePoints(dpointvec& pts)
{
    // the stick
    double ys1 = 1.0 / 3;
    double ys2 = -2.0 / 3;
    double xs = 1.0 / 3;
    pts.clear();
    pts.push_back({ xs, ys2 });
    pts.push_back({ xs, ys1 });

    // The hook
    for (double degrees = 0.0; degrees < 180; degrees += 1.0) {
        double radian = degrees * (M_PI / 180.0);
        pts.push_back({ cos(radian) / 3, sin(radian) / 3 + ys1 });
    }
}

static void getStarPoints(dpointvec& pts, int npts)
{
    double offsetangle = 90 - 360 / npts; // May as well be 90.  We just want a point that is up.

    double InnerRadius = 1.0 / 2.618034; // divide by golden ratio squared

    double increment = 360.0 / npts;

    double deg = offsetangle;
    for (int i = 0; i < npts; deg += increment, ++i) {
        double radian = (deg) * (M_PI / 180.0);
        double xouter = cos(radian);
        double youter = sin(radian);

        radian = (deg + increment / 2.0) * (M_PI / 180.0);
        double xinner = InnerRadius * cos(radian);
        double yinner = InnerRadius * sin(radian);

        pts.push_back({ xouter, youter });
        pts.push_back({ xinner, yinner });
    }
}

static void getSnowflakePoints(dpointvec& pts, int npts)
{
    // The original is not, really, all that good for our purpose as it has no width.  Considering what to do instead.
    npts *= 2;

    double increment = 360.0 / npts;
    pts.resize(npts * 3);

    double rotation = 0; // The original always has spoke horizontal, don't like it, set rotation to 90

    for (int i = 0; i < npts; ++i, rotation += increment) {
        double delta = increment / 20;
        double inrad = .05;

        double r1 = (rotation - delta) * M_PI / 180.0;
        double r2 = (rotation + delta) * M_PI / 180.0;
        double r3 = (rotation + increment / 2) * M_PI / 180.0;

        pts[i * 3 + 0] = { cos(r1), sin(r1) };
        pts[i * 3 + 1] = { cos(r2), sin(r2) };
        pts[i * 3 + 2] = { inrad * cos(r3), inrad * sin(r3) };
    }
}

#if 0
inline uint8_t GetSVGRed(uint32_t colour)
{
    return (colour);
}

inline uint8_t GetSVGGreen(uint32_t colour)
{
    return (colour >> 8);
}

inline uint8_t GetSVGBlue(uint32_t colour)
{
    return (colour >> 16);
}

inline uint8_t GetSVGAlpha(uint32_t colour)
{
    return (colour >> 24);
}

inline uint32_t GetSVGExAlpha(uint32_t colour)
{
    return (colour & 0xFFFFFF);
}

inline uint32_t GetSVGColour(xlColor c)
{
    return (((uint32_t)c.alpha) << 24) +
           (((uint32_t)c.blue) << 16) +
           (((uint32_t)c.green) << 8) +
           c.red;
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
#endif

static void DrawShape(RenderBuffer &buffer, ipointvec &points, const xlColor &color, bool close)
{
    for (size_t i = 0; i < points.size() - 1; ++i) {
        buffer.DrawLine(points[i].first, points[i].second, points[i + 1].first, points[i + 1].second, color);
    }
    if (close) {
        buffer.DrawLine(points[points.size() - 1].first, points[points.size() - 1].second, points[0].first, points[0].second, color);
    }
}

static bool isConvex(const ipointvec& q)
{
    double cross = 0;
    for (int i = 0; i < 4; i++) {
        double x1 = q[(i + 1) % 4].first - q[i].first;
        double y1 = q[(i + 1) % 4].second - q[i].second;
        double x2 = q[(i + 2) % 4].first - q[(i + 1) % 4].first;
        double y2 = q[(i + 2) % 4].second - q[(i + 1) % 4].second;
        double cross_product = x1 * y2 - y1 * x2;

        if (i == 0) {
            cross = cross_product;
        } else if (cross_product * cross <= 0) {
            return false;
        }
    }

    return true;
}

static void FillSusQuad(RenderBuffer &buffer, const ipointvec& q, const xlColor &c)
{
    if (isConvex(q)) {
        buffer.FillConvexPoly(q, c);
        return;
    }
    // Hum the non-complex quad could be split in just the right place, which would be better.
    ipointvec tri1 = { q[0], q[1], q[2] };
    ipointvec tri2 = { q[0], q[1], q[3] };
    ipointvec tri3 = { q[0], q[2], q[3] };
    ipointvec tri4 = { q[1], q[2], q[3] };
    buffer.FillConvexPoly(tri1, c);
    buffer.FillConvexPoly(tri2, c);
    buffer.FillConvexPoly(tri3, c);
    buffer.FillConvexPoly(tri4, c);
}

static void DrawLine(RenderBuffer& buffer, const dpoint& p1, const dpoint& p2, const xlColor &c, bool thick = false)
{
    ipointvec q(4);
    if (abs(p1.second - p2.second) > abs(p1.first - p2.first)) {
        // More vertical travel
        if (round(p1.first + .5) != round(p1.first)) {
            q[0] = { round(p1.first), round(p1.second) };
            q[1] = { round(p1.first) + 1, round(p1.second) };
        } else {
            q[0] = { round(p1.first) - 1, round(p1.second) };
            q[1] = { round(p1.first), round(p1.second) };
        }
        if (round(p2.first + .5) != round(p2.first)) {
            q[2] = { round(p2.first) + 1, round(p2.second) };
            q[3] = { round(p2.first), round(p2.second) };
        } else {
            q[2] = { round(p2.first), round(p2.second) };
            q[3] = { round(p2.first) - 1, round(p2.second) };
        }
    } else {
        // More horizontal travel
        if (round(p1.second + .5) != round(p1.second)) {
            q[0] = { round(p1.first), round(p1.second) };
            q[1] = { round(p1.first), round(p1.second) + 1 };
        } else {
            q[0] = { round(p1.first), round(p1.second) - 1};
            q[1] = { round(p1.first), round(p1.second) };
        }
        if (round(p2.second + .5) != round(p2.second)) {
            q[2] = { round(p2.first), round(p2.second) + 1 };
            q[3] = { round(p2.first), round(p2.second) };
        } else {
            q[2] = { round(p2.first), round(p2.second) };
            q[3] = { round(p2.first), round(p2.second) - 1 };
        }
    }
    buffer.FillConvexPoly(q, c);
    if (thick) {
        buffer.SetPixel(round(p1.first), round(p1.second), c);
        buffer.SetPixel(round(p2.first), round(p2.second), c);
    }
}

static void DrawShapeD(RenderBuffer& buffer, dpointvec& points, const xlColor& color, bool close, bool thick)
{
    for (size_t i = 0; i < points.size() - 1; ++i) {
        DrawLine(buffer, { points[i].first, points[i].second }, { points[i + 1].first, points[i + 1].second }, color, thick);
    }
    if (close) {
        DrawLine(buffer, { points[points.size() - 1].first, points[points.size() - 1].second }, { points[0].first, points[0].second }, color, thick);
    }
}

static void FillRegion(RenderBuffer& buffer, ipointvec& oldpoints, const ipointvec& newpoints, const xlColor& color, bool close = true)
{
    if (oldpoints.empty())
        return;
    if (newpoints.size() != oldpoints.size())
        return;

    ipointvec quad(4);
    for (size_t i = 0; i < newpoints.size() - 1; ++i) {
        quad[0] = oldpoints[i];
        quad[1] = oldpoints[i + 1];
        quad[2] = newpoints[i + 1];
        quad[3] = newpoints[i];

        FillSusQuad(buffer, quad, color);
    }

    if (close) {
        quad[0] = oldpoints[oldpoints.size() - 1];
        quad[1] = oldpoints[0];
        quad[2] = newpoints[0];
        quad[3] = newpoints[oldpoints.size() - 1];

        FillSusQuad(buffer, quad, color);
    }
}

static ipointvec ScaleShape(const dpointvec& in, double sx, double sy, double cx, double cy, double rotation, bool round=false)
{
    ipointvec rv;
    double angle_rad = rotation * M_PI / 180.0;

    for (const auto& p : in) {
        double x = p.first * sx;
        double y = p.second * sy;
        double rx = x * cos(angle_rad) - y * sin(angle_rad);
        double ry = x * sin(angle_rad) + y * cos(angle_rad);

        if (round) {
            rv.push_back({ std::round(rx + cx), std::round(ry + cy) });
        } else {
            rv.push_back({ rx + cx, ry + cy });
        }
    }
    return rv;
}

static dpointvec ScaleShapeD(const dpointvec& in, double sx, double sy, double cx, double cy, double rotation)
{
    dpointvec rv;
    double angle_rad = rotation * M_PI / 180.0;

    for (const auto& p : in) {
        double x = p.first * sx;
        double y = p.second * sy;
        double rx = x * cos(angle_rad) - y * sin(angle_rad);
        double ry = x * sin(angle_rad) + y * cos(angle_rad);

        rv.push_back({ rx + cx, ry + cy });
    }
    return rv;
}

struct RippleShape
{
    dpointvec points;
    bool closedShape = false;
    HSVValue defColor = xlWHITE;
    RippleShape() {}
    RippleShape(const dpointvec& vec, bool closed) :
        points(vec), closedShape(closed)
    {}
};
struct RippleShapes
{
    std::vector<RippleShape> shapes;
    RippleShapes() {}
    RippleShapes(const dpointvec& vec, bool closed)
    {
        shapes.push_back(RippleShape(vec, closed));
    }
};

static void drawRippleNew(
    RenderBuffer& buffer, const RippleShapes& shapes,
    double time, double xcc, double ycc, double srotation, int mvmt, bool nonsquare,
    int thickness, bool doInside, bool doOutside,
    bool fade, bool lines, bool fill, bool ripple,
    double scale, double spacing, double twist, double vel, double veldir)
{
    // Center point
    double sxc = buffer.BufferWi / 2.0 + xcc * (buffer.BufferWi / 2.0) / 100.0;
    double syc = buffer.BufferHt / 2.0 + ycc * (buffer.BufferHt / 2.0) / 100.0;

    // Scaling for x and y directions - the original depended on whether it is on center...
    //  and it isn't exactly true.  We may want a scale factor or something...
    double maxRadius = 0.0;
    double maxRadiusX = buffer.BufferWi / 2.0;
    double maxRadiusY = buffer.BufferHt / 2.0;
    if (buffer.BufferWi > buffer.BufferHt) {
        maxRadius = maxRadiusX;
    } else {
        maxRadius = maxRadiusY;
    }

    // Base unit for width - a pixel, or .5% whichever is more sensible at the time
    //  Location things - x, y, are already percentages - that scales
    //  Radius/scale is a percentage also
    //  Rotation things are scale invariant (rotation, direction, twist)
    //  Cycles is a temporal count, that's not related to scaling
    //  Thickness is a number, that's invariant
    //  Velocity may as well be in percent
    // This means the width of things is pixels
    // The last thing expressed in pixels, then, is spacing.  Why is spacing expressed as pixels?  Why not have it be %?
    //   With this formula, they are percentages for resolutions over 200,
    //     but are pixels for resolutions below 200. Why?
    double pxw = 1.0;
    pxw = std::max(pxw, buffer.BufferWi * 0.005);
    pxw = std::max(pxw, buffer.BufferHt * 0.005);

    spacing *= pxw;

    // Color calculations
    int cidxShp = 0;
    HSVValue hsv;
    size_t colorcnt = buffer.GetColorCount();
    buffer.palette.GetHSV(0, hsv); // Now go and get the hsv value for this ColorIdx
    HSVValue lhsv = hsv;
    int cidxLines = cidxShp;
    if (colorcnt > 1) {
        buffer.palette.GetHSV(1, lhsv);
        cidxLines = 1;
    }
    HSVValue fhsv = lhsv;
    int cidxFill = cidxLines;
    if (colorcnt > 2) {
        buffer.palette.GetHSV(2, fhsv);
        cidxFill = 2;
    }

    // Radius
    double baseRadius = 1;
    if (mvmt == MOVEMENT_EXPLODE)
        baseRadius = time;
    if (mvmt == MOVEMENT_IMPLODE)
        baseRadius = 1 - time;
    double brX = baseRadius * maxRadius;
    double brY = baseRadius * maxRadius;
    if (nonsquare) {
        brX = baseRadius * maxRadiusX;
        brY = baseRadius * maxRadiusY;
    }
    baseRadius *= scale;
    brX *= scale;
    brY *= scale;
    vel *= maxRadius / 100; // vel was %

    // OK time to draw!
    if (fill) {
        ipointvec oldptsouter, oldptsinner;
        dpointvec oldedgeouter, oldedgeinner;
        for (int i = thickness; i >= 0; --i) {
            double xc = sxc + (vel * sin(veldir / 180 * M_PI) * i);
            double yc = syc - (vel * cos(veldir / 180 * M_PI) * i);
            double rotation = srotation + twist * i;
            double strength = (double(thickness) + 1 - i) / (thickness + 1); // Used for 3D/fade
            double delta = ripple ? ((i * i + 1) * 0.25) : (i * 0.5);
            delta *= spacing;

            xlColor fadeColor(fhsv);
            if (buffer.palette.IsSpatial(cidxFill)) {
                buffer.palette.GetSpatialColor(cidxFill, 0, 0, 0, i, 0, thickness, fadeColor);
            }

            xlColor lineColor(lhsv);
            if (buffer.palette.IsSpatial(cidxLines)) {
                buffer.palette.GetSpatialColor(cidxLines, 0, 0, 0, i, 0, thickness, lineColor);
            }

            xlColor fadeColor3D = fadeColor;
            if (buffer.allowAlpha) {
                fadeColor3D.alpha = 255.0 * strength;
            } else {
                HSVValue hsvc = fadeColor;
                hsvc.value *= strength;
                fadeColor3D = hsvc;
            }

            const dpointvec& points = shapes.shapes[0].points;
            bool closedShape = shapes.shapes[0].closedShape;
            dpointvec nxtedgeouter, nxtedgeinner;
            if (doInside && brX - delta > 0 && brY - delta > 0) {
                ipointvec ishp = ScaleShape(points, brX - delta, brY - delta, xc, yc, rotation, true);
                if (!oldptsinner.empty()) {
                    FillRegion(buffer, oldptsinner, ishp, fade ? fadeColor3D : fadeColor, closedShape);
                }
                oldptsinner = ishp;
                if (lines) {
                    nxtedgeinner = ScaleShapeD(points, brX - delta, brY - delta, xc, yc, rotation);
                }
            } else if (doInside) {
                oldptsinner = ScaleShape(points, 0, 0, xc, yc, rotation, true);
            }
            if (doOutside && brX + delta > 0 && brY + delta > 0) {
                ipointvec oshp = ScaleShape(points, brX + delta, brY + delta, xc, yc, rotation, true);
                if (!oldptsouter.empty()) {
                    FillRegion(buffer, oldptsouter, oshp, fade ? fadeColor3D : fadeColor, closedShape);
                }
                oldptsouter = oshp;
                if (lines) {
                    nxtedgeouter = ScaleShapeD(points, brX + delta, brY + delta, xc, yc, rotation);
                }
            }

            if (oldedgeinner.size()) {
                DrawShapeD(buffer, oldedgeinner, lineColor, closedShape, false);
            }
            if (oldedgeouter.size()) {
                DrawShapeD(buffer, oldedgeouter, lineColor, closedShape, false);
            }

            oldedgeouter = nxtedgeouter;
            oldedgeinner = nxtedgeinner;
        }

        bool closedShape = shapes.shapes[0].closedShape;
        if (oldptsinner.size() > 0 && oldptsouter.size() > 0) {
            FillRegion(buffer, oldptsinner, oldptsouter, xlColor(fhsv), closedShape);
        }
    }
    if (lines && !fill) {
        for (int i = thickness; i >= 0; --i) {
            double xc = sxc + (vel * sin(veldir / 180 * M_PI) * i);
            double yc = syc - (vel * cos(veldir / 180 * M_PI) * i);
            double rotation = srotation + twist * i;
            double strength = (double(thickness) + 1 - i) / (thickness + 1); // Used for 3D/fade
            double delta = ripple ? ((i * i + 1) * 0.25) : (i * 0.5);
            delta *= spacing;

            xlColor lineColor(lhsv);
            if (buffer.palette.IsSpatial(cidxLines)) {
                buffer.palette.GetSpatialColor(cidxLines, 0, 0, 0, i, 0, thickness, lineColor);
            }

            xlColor fadeColor3D = lineColor;

            if (buffer.allowAlpha) {
                fadeColor3D.alpha = 255.0 * strength;
            } else {
                HSVValue hsvc = lhsv;
                hsvc.value *= strength;
                fadeColor3D = hsvc;
            }

            const dpointvec& points = shapes.shapes[0].points;
            bool closedShape = shapes.shapes[0].closedShape;

            if (doInside && brX - delta > 0 && brY - delta > 0) {
                ipointvec ishp = ScaleShape(points, brX - delta, brY - delta, xc, yc, rotation);
                DrawShape(buffer, ishp, (fade) ? fadeColor3D : lineColor, closedShape);
            }
            if (doOutside && brX + delta > 0 && brY + delta > 0) {
                ipointvec oshp = ScaleShape(points, brX + delta, brY + delta, xc, yc, rotation);
                DrawShape(buffer, oshp, (fade) ? fadeColor3D : lineColor, closedShape);
            }
        }
    }

    // Draw the main shape
    bool closedShape = shapes.shapes[0].closedShape;
    const dpointvec& points = shapes.shapes[0].points;
    if (brX > 0 && brY > 0) {
        if (fill) {
            dpointvec mshp = ScaleShapeD(points, brX, brY, sxc, syc, srotation);
            DrawShapeD(buffer, mshp, hsv, closedShape, true);        
        } else {
            ipointvec mshp = ScaleShape(points, brX, brY, sxc, syc, srotation);
            DrawShape(buffer, mshp, hsv, closedShape);
        }
    }
}

class RippleRenderCache : public EffectRenderCache
{
public:
    RippleRenderCache()
    {
    }
    virtual ~RippleRenderCache()
    {
        if (_svgImage != nullptr) {
            nsvgDelete(_svgImage);
            _svgImage = nullptr;
        }
    }

    NSVGimage* _svgImage = nullptr;
    std::string _svgFilename;
    double _svgScaleBase = 1.0f;

    void InitialiseSVG(const std::string filename)
    {
        _svgFilename = filename;
        _svgImage = nsvgParseFromFile(_svgFilename.c_str(), "px", 96);
        if (_svgImage != nullptr) {
            auto max = std::max(_svgImage->height, _svgImage->width);
            _svgScaleBase = 1.0 / max;
        }
    }

    NSVGimage* GetImage()
    {
        return _svgImage;
    }
};

// TODO:
// 1 IMP: Scaling for spacing
// 3 ENH: Thickness of center shape?
// 4 ENH: There is the matter of colors (around; this is a matter of breaking long segments up)
// 2 ENH: SVG

void RippleEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    float oset = buffer.GetEffectTimeIntervalPosition();
    const std::string& Object_To_DrawStr = SettingsMap["CHOICE_Ripple_Object_To_Draw"];
    std::string svgFilename = SettingsMap["FILEPICKERCTRL_SVG"];
    const std::string& MovementStr = SettingsMap["CHOICE_Ripple_Movement"];
    int Ripple_Thickness = GetValueCurveInt("Ripple_Thickness", 3, SettingsMap, oset, RIPPLE_THICKNESS_MIN, RIPPLE_THICKNESS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool CheckBox_Ripple3D = SettingsMap.GetBool("CHECKBOX_Ripple3D", false);
    const std::string& StyleStr = SettingsMap.Get("CHOICE_Ripple_Draw_Style", "Old");
    float cycles = GetValueCurveDouble("Ripple_Cycles", 1.0, SettingsMap, oset, RIPPLE_CYCLES_MIN, RIPPLE_CYCLES_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), RIPPLE_CYCLES_DIVISOR);
    int points = SettingsMap.GetInt("SLIDER_RIPPLE_POINTS", 5);
    int rotation = GetValueCurveInt("Ripple_Rotation", 0, SettingsMap, oset, RIPPLE_ROTATION_MIN, RIPPLE_ROTATION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int xcc = GetValueCurveInt("Ripple_XC", 0, SettingsMap, oset, RIPPLE_XC_MIN, RIPPLE_XC_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int ycc = GetValueCurveInt("Ripple_YC", 0, SettingsMap, oset, RIPPLE_YC_MIN, RIPPLE_YC_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    double scale = GetValueCurveDouble("Ripple_Scale", 100, SettingsMap, oset, RIPPLE_SCALE_MIN, RIPPLE_SCALE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) / 100.0;
    double spacing = GetValueCurveDouble("Ripple_Spacing", 1.0, SettingsMap, oset, RIPPLE_SPACING_MIN, RIPPLE_SPACING_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), RIPPLE_SPACING_DIVISOR);
    double outline = GetValueCurveDouble("Ripple_Outline", 1.0, SettingsMap, oset, RIPPLE_OUTLINE_MIN, RIPPLE_OUTLINE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), RIPPLE_OUTLINE_DIVISOR);
    double twist = GetValueCurveDouble("Ripple_Twist", 0, SettingsMap, oset, RIPPLE_TWIST_MIN, RIPPLE_TWIST_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), RIPPLE_TWIST_DIVISOR);
    double vel = GetValueCurveDouble("Ripple_Velocity", 0, SettingsMap, oset, RIPPLE_VELOCITY_MIN, RIPPLE_VELOCITY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), RIPPLE_VELOCITY_DIVISOR);
    double veldir = GetValueCurveDouble("Ripple_Direction", 0, SettingsMap, oset, RIPPLE_DIRECTION_MIN, RIPPLE_DIRECTION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    RippleRenderCache* cache = (RippleRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new RippleRenderCache();
        buffer.infoCache[id] = cache;
    }

    double position = buffer.GetEffectTimeIntervalPosition(cycles); // how far are we into the effect; value is 0.0 to 1.0

    int Object_To_Draw;
    dpointvec shapePts;
    bool closeShape = true;
    bool uniformAspectRatio = true;

    RippleShapes shapes;

    if (Object_To_DrawStr == "SVG" && StyleStr != "Old") {
        Object_To_Draw = RENDER_RIPPLE_SVG;
    } else if (Object_To_DrawStr == "Circle") {
        getCirclePoints(shapePts);
        Object_To_Draw = RENDER_RIPPLE_CIRCLE;
    } else if (Object_To_DrawStr == "Square") {
        getSquarePoints(shapePts);
        Object_To_Draw = RENDER_RIPPLE_SQUARE;
        uniformAspectRatio = false; // This is actually a rectangle
    } else if (Object_To_DrawStr == "Triangle") {
        getTrianglePoints(shapePts);
        Object_To_Draw = RENDER_RIPPLE_TRIANGLE;
    } else if (Object_To_DrawStr == "Star") {
        getStarPoints(shapePts, points);
        Object_To_Draw = RENDER_RIPPLE_STAR;
    } else if (Object_To_DrawStr == "Polygon") {
        getPolygonPoints(shapePts, points);
        Object_To_Draw = RENDER_RIPPLE_POLYGON;
    } else if (Object_To_DrawStr == "Heart") {
        getHeartPoints(shapePts);
        Object_To_Draw = RENDER_RIPPLE_HEART;
    } else if (Object_To_DrawStr == "Tree") {
        getTreePoints(shapePts);
        Object_To_Draw = RENDER_RIPPLE_TREE;
    } else if (Object_To_DrawStr == "Candy Cane") {
        getCanePoints(shapePts);
        closeShape = false;
        Object_To_Draw = RENDER_RIPPLE_CANDYCANE;
    } else if (Object_To_DrawStr == "Snow Flake") {
        getSnowflakePoints(shapePts, points);
        Object_To_Draw = RENDER_RIPPLE_SNOWFLAKE;
    } else if (Object_To_DrawStr == "Crucifix") {
        Object_To_Draw = RENDER_RIPPLE_CRUCIFIX;
        getCrossPoints(shapePts);
    } else if (Object_To_DrawStr == "Present") {
        getPresentPoints(shapePts);
        closeShape = false;
        Object_To_Draw = RENDER_RIPPLE_PRESENT;
    } else {
        getCirclePoints(shapePts);
        Object_To_Draw = RENDER_RIPPLE_CIRCLE;
    }
    if (shapes.shapes.empty()) {
        // Build from shapePts, closed
        shapes = RippleShapes(shapePts, closeShape);
    }

    int Movement = MOVEMENT_NONE;
    if (MovementStr == "Explode") {
        Movement = MOVEMENT_EXPLODE;
    }
    if (MovementStr == "Implode") {
        Movement = MOVEMENT_IMPLODE;
    }

    bool drawNew = false;
    bool interiorDirection = false;
    bool exteriorDirection = false;
    bool drawFill = false;
    bool drawLines = false;
    bool rippleSpaced = false;

    auto swords = wxSplit(StyleStr, ' ');
    if (swords.size() > 1) {
        drawNew = true;
        if (swords[0] == "Lines") {
            drawLines = true;
        }
        if (swords[0] == "Solid") {
            drawFill = true;
        }
        if (swords[0] == "Highlight") {
            drawFill = true;
            drawLines = true;
        }
        if (swords[1] == "Inward") {
            interiorDirection = true;
        }
        if (swords[1] == "Outward") {
            exteriorDirection = true;
        }
        if (swords[1] == "Both") {
            interiorDirection = true;
            exteriorDirection = true;
        }
        if (swords.size() > 2) {
            // Assume it says "Ripple"
            rippleSpaced = true;
        }
    }

    if (buffer.needToInit) {
        buffer.needToInit = false;

        if (Object_To_Draw == RENDER_RIPPLE_SVG) {
            cache->InitialiseSVG(svgFilename);
        }
    }

    if (drawNew)
    {
        drawRippleNew(buffer, shapes, position, xcc, ycc, rotation, Movement,
                      !uniformAspectRatio, Ripple_Thickness, interiorDirection, exteriorDirection,
                 CheckBox_Ripple3D, drawLines, drawFill, rippleSpaced, scale, spacing, twist, vel, veldir);
        return;
    }

    int ColorIdx;
    int xc, yc;

    HSVValue hsv;
    size_t colorcnt = buffer.GetColorCount();

    float rx = position;
    xc = buffer.BufferWi / 2 + xcc * (buffer.BufferWi / 2) / 100;
    yc = buffer.BufferHt / 2 + ycc * (buffer.BufferHt / 2) / 100;
    double maxRadius = 0.0;
    double maxRadiusX = std::max(xc, buffer.BufferWi - xc);
    double maxRadiusY = std::max(yc, buffer.BufferHt - yc);
    if (buffer.BufferWi > buffer.BufferHt) {
        maxRadius = maxRadiusX;
    } else {
        maxRadius = maxRadiusY;
    }

    ColorIdx = static_cast<int>(rx * colorcnt);
    if (ColorIdx == colorcnt)
        ColorIdx--; // ColorIdx goes from 0-3 when colorcnt goes from 1-4. Make sure that is true

    double radius;
    double radiusX;
    double radiusY;
    double side;
    if (Movement == MOVEMENT_IMPLODE) {
        radius = maxRadius - (maxRadius * rx);
        side = maxRadius - (maxRadius * rx);
        radiusX = maxRadiusX - (maxRadiusX * rx);
        radiusY = maxRadiusY - (maxRadiusY * rx);
    } else {
        radius = (maxRadius * rx);
        side = (maxRadius * rx);
        radiusX = (maxRadiusX * rx);
        radiusY = (maxRadiusY * rx);
    }

    buffer.palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx

    switch (Object_To_Draw) {
    case RENDER_RIPPLE_SQUARE: {
        int x1 = xc - radiusX;
        int x2 = xc + radiusX;
        int y1 = yc - radiusY;
        int y2 = yc + radiusY;
        Drawsquare(buffer, Movement, x1, x2, y1, y2, Ripple_Thickness, CheckBox_Ripple3D, hsv);
    } break;
    case RENDER_RIPPLE_CIRCLE:
        Drawcircle(buffer, Movement, xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
        break;
    case RENDER_RIPPLE_STAR:
        Drawstar(buffer, Movement, xc, yc, radius, points, hsv, Ripple_Thickness, CheckBox_Ripple3D, rotation);
        break;
    case RENDER_RIPPLE_TRIANGLE:
        Drawtriangle(buffer, Movement, xc, yc, side, hsv, Ripple_Thickness, CheckBox_Ripple3D);
        break;
    case RENDER_RIPPLE_POLYGON:
        Drawpolygon(buffer, Movement, xc, yc, radius, points, hsv, Ripple_Thickness, CheckBox_Ripple3D, rotation);
        break;
    case RENDER_RIPPLE_HEART:
        Drawheart(buffer, Movement, xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
        break;
    case RENDER_RIPPLE_TREE:
        Drawtree(buffer, Movement, xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
        break;
    case RENDER_RIPPLE_CANDYCANE:
        Drawcandycane(buffer, Movement, xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
        break;
    case RENDER_RIPPLE_SNOWFLAKE:
        Drawsnowflake(buffer, Movement, xc, yc, radius, points, hsv, Ripple_Thickness, CheckBox_Ripple3D, rotation);
        break;
    case RENDER_RIPPLE_CRUCIFIX:
        Drawcrucifix(buffer, Movement, xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
        break;
    case RENDER_RIPPLE_PRESENT:
        Drawpresent(buffer, Movement, xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
        break;
    default:
        wxASSERT(false);
        break;
    }
}

void RippleEffect::Drawtriangle(RenderBuffer& buffer, int Movement, int xc, int yc, double side, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D)
{
    if (side < 0)
        return;

    xlColor color(hsv);

    for (int i = 0; i < Ripple_Thickness; i++) {
        double radius = (side + i) * ROOT3DIV3;
        double ytop = yc + radius;
        int xtop = xc;

        double xleft = xc - radius * COS30;
        double yleft = yc - radius * SIN30;

        double xright = xleft + side + i; // This is kinda weird
        double yright = yleft;

        if (CheckBox_Ripple3D) {
            if (buffer.allowAlpha) {
                color.alpha = 255.0 * (1.0 - ((float(i) / 2.0) / float(Ripple_Thickness)));
            } else {
                hsv.value *= 1.0 - ((float(i) / 2.0) / float(Ripple_Thickness)); // we multiply by 1.0 when Ripple_Thickness=0
                color = hsv;
            }
        }

        buffer.DrawLine(xtop, ytop, xleft, yleft, color);
        buffer.DrawLine(xtop, ytop, xright, yright, color);
        buffer.DrawLine(xleft, yleft, xright, yright, color);
    }
}

void RippleEffect::Drawsquare(RenderBuffer& buffer, int Movement, int x1, int x2, int y1, int y2, int Ripple_Thickness, int CheckBox_Ripple3D, HSVValue& hsv)
{
    xlColor color(hsv);

    for (int i = 0; i < Ripple_Thickness; i++) {
        if (CheckBox_Ripple3D) {
            if (buffer.allowAlpha) {
                color.alpha = 255.0 * (1.0 - ((float(i) / 2.0) / float(Ripple_Thickness)));
            } else {
                hsv.value *= 1.0 - ((float(i) / 2.0) / float(Ripple_Thickness)); // we multiply by 1.0 when Ripple_Thickness=0
                color = hsv;
            }
        }
        if (Movement == MOVEMENT_EXPLODE) {
            for (int y = y1 + i; y <= y2 - i; y++) {
                buffer.SetPixel(x1 + i, y, color); // Turn pixel
                buffer.SetPixel(x2 - i, y, color); // Turn pixel
            }
            for (int x = x1 + i; x <= x2 - i; x++) {
                buffer.SetPixel(x, y1 + i, color); // Turn pixel
                buffer.SetPixel(x, y2 - i, color); // Turn pixel
            }
        }
        if (Movement == MOVEMENT_IMPLODE) {
            for (int y = y2 + i; y >= y1 - i; y--) {
                buffer.SetPixel(x1 - i, y, color); // Turn pixel
                buffer.SetPixel(x2 + i, y, color); // Turn pixel
            }
            for (int x = x2 + i; x >= x1 - i; x--) {
                buffer.SetPixel(x, y1 - i, color); // Turn pixel
                buffer.SetPixel(x, y2 + i, color); // Turn pixel
            }
        }
    }
}

void RippleEffect::Drawcircle(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D)
{
    xlColor color(hsv);

    for (float i = 0; i < Ripple_Thickness; i += .5) {
        if (CheckBox_Ripple3D) {
            if (buffer.allowAlpha) {
                color.alpha = 255.0 * (1.0 - (float(i) / float(Ripple_Thickness)));
            } else {
                hsv.value *= 1.0 - (float(i) / float(Ripple_Thickness)); // we multiply by 1.0 when steps=0
                color = hsv;
            }
        }
        if (Movement == MOVEMENT_EXPLODE) {
            radius = radius + i;
        } else {
            radius = radius - i;
        }

        if (radius >= 0.0) {
            for (double degrees = 0.0; degrees < 360.0; degrees += 1.0) {
                double radian = degrees * (M_PI / 180.0);
                int x = radius * cos(radian) + xc;
                int y = radius * sin(radian) + yc;
                buffer.SetPixel(x, y, color); // Turn pixel
            }
        }
    }
}

void RippleEffect::Drawstar(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, int points, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D, float rotation)
{
    double offsetangle = 0.0;
    switch (points) {
    case 3:
        offsetangle = 90.0 - 360.0 / 3;
    case 4:
        break;
    case 5:
        offsetangle = 90.0 - 360.0 / 5;
        break;
    case 6:
        offsetangle = 30.0;
        break;
    case 7:
        offsetangle = 90.0 - 360.0 / 7;
        break;
    case 8:
        offsetangle = 90.0 - 360.0 / 8;
        break;
    default:
        wxASSERT(false);
        break;
    }

    xlColor color(hsv);

    for (double i = 0; i < Ripple_Thickness; i += .5) {
        if (CheckBox_Ripple3D) {
            if (buffer.allowAlpha) {
                color.alpha = 255.0 * (1.0 - (float(i) / float(Ripple_Thickness)));
            } else {
                hsv.value *= 1.0 - (float(i) / float(Ripple_Thickness)); // we multiply by 1.0 when steps=0
                color = hsv;
            }
        }
        if (Movement == MOVEMENT_EXPLODE) {
            radius = radius + i;
        } else {
            radius = radius - i;
        }

        if (radius >= 0.0) {
            double InnerRadius = radius / 2.618034; // divide by golden ratio squared

            double increment = 360.0 / points;

            for (double degrees = 0.0; degrees < 361.0; degrees += increment) // 361 because it allows for small rounding errors
            {
                if (degrees > 360.0)
                    degrees = 360.0;
                double radian = (rotation + offsetangle + degrees) * (M_PI / 180.0);
                int xouter = radius * cos(radian) + xc;
                int youter = radius * sin(radian) + yc;

                radian = (rotation + offsetangle + degrees + increment / 2.0) * (M_PI / 180.0);
                int xinner = InnerRadius * cos(radian) + xc;
                int yinner = InnerRadius * sin(radian) + yc;

                buffer.DrawLine(xinner, yinner, xouter, youter, color);

                radian = (rotation + offsetangle + degrees - increment / 2.0) * (M_PI / 180.0);
                xinner = InnerRadius * cos(radian) + xc;
                yinner = InnerRadius * sin(radian) + yc;

                buffer.DrawLine(xinner, yinner, xouter, youter, color);
            }
        }
    }
}

void RippleEffect::Drawpolygon(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, int points, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D, float rotation)
{
    double increment = 360.0 / points;

    if (points % 2 != 0)
        rotation += 90;
    if (points == 4)
        rotation += 45;
    if (points == 8)
        rotation += 22.5;

    xlColor color(hsv);

    std::vector<std::pair<int, int>> oldpts, newpts;

    for (double i = 0; i < Ripple_Thickness; i += .5) {
        if (CheckBox_Ripple3D) {
            if (buffer.allowAlpha) {
                color.alpha = 255.0 * (1.0 - (float(i) / float(Ripple_Thickness)));
            } else {
                hsv.value *= 1.0 - (float(i) / float(Ripple_Thickness)); // we multiply by 1.0 when steps=0
                color = hsv;
            }
        }
        if (Movement == MOVEMENT_EXPLODE) {
            radius = radius + i;
        } else {
            radius = radius - i;
        }

        if (radius >= 0) {
            for (double degrees = 0.0; degrees < 361.0; degrees += increment) // 361 because it allows for small rounding errors
            {
                if (degrees > 360.0)
                    degrees = 360.0;
                double radian = (rotation + degrees) * M_PI / 180.0;
                int x1 = std::round(radius * cos(radian)) + xc;
                int y1 = std::round(radius * sin(radian)) + yc;

                radian = (rotation + degrees + increment) * M_PI / 180.0;
                int x2 = std::round(radius * cos(radian)) + xc;
                int y2 = std::round(radius * sin(radian)) + yc;

                buffer.DrawLine(x1, y1, x2, y2, color);

                if (degrees == 360.0)
                    degrees = 361.0;
            }
        } else {
            break;
        }
    }
}

void RippleEffect::Drawsnowflake(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, int points, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D, double rotation)
{
    double increment = 360.0 / (points * 2);
    double angle = rotation;

    xlColor color(hsv);

    if (radius >= 0) {
        for (int i = 0; i < points * 2; i++) {
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

void RippleEffect::Drawheart(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D)
{
    xlColor color(hsv);

    for (float i = 0; i < Ripple_Thickness; i += 0.5) {
        if (CheckBox_Ripple3D) {
            if (buffer.allowAlpha) {
                color.alpha = 255.0 * (1.0 - (float(i) / float(Ripple_Thickness)));
            } else {
                hsv.value *= 1.0 - (float(i) / float(Ripple_Thickness)); // we multiply by 1.0 when steps=0
                color = hsv;
            }
        }

        if (Movement == MOVEMENT_EXPLODE) {
            radius = radius + i;
        } else {
            radius = radius - i;
        }

        if (radius >= 0) {
            double xincr = 0.01;
            for (double x = -2.0; x <= 2.0; x += xincr) {

                double y1 = std::sqrt(1.0 - (std::abs(x) - 1.0) * (std::abs(x) - 1.0));
                double y2 = std::acos(1.0 - std::abs(x)) - M_PI;

                double xx1 = std::round((x * radius) / 2.0) + xc;
                double yy1 = (y1 * radius) / 2.0 + yc;
                double yy2 = (y2 * radius) / 2.0 + yc;

                if (radius >= 0) {
                    buffer.SetPixel(xx1, std::round(yy1), color);
                    buffer.SetPixel(xx1, std::round(yy2), color);

                    if (x + xincr > 2.0 || x == -2.0 + xincr) {

                        if (yy1 > yy2)
                            std::swap(yy1, yy2);

                        for (double z = yy1; z < yy2; z += 0.5) {
                            buffer.SetPixel(xx1, std::round(z), color);
                        }
                    }
                } else {
                    break;
                }
            }
        } else {
            break;
        }
    }
}

void RippleEffect::Drawtree(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D)
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

    const line points[] = { line(wxPoint(3, 0), wxPoint(5, 0)),
                            line(wxPoint(5, 0), wxPoint(5, 3)),
                            line(wxPoint(3, 0), wxPoint(3, 3)),
                            line(wxPoint(0, 3), wxPoint(8, 3)),
                            line(wxPoint(0, 3), wxPoint(2, 6)),
                            line(wxPoint(8, 3), wxPoint(6, 6)),
                            line(wxPoint(1, 6), wxPoint(2, 6)),
                            line(wxPoint(6, 6), wxPoint(7, 6)),
                            line(wxPoint(1, 6), wxPoint(3, 9)),
                            line(wxPoint(7, 6), wxPoint(5, 9)),
                            line(wxPoint(2, 9), wxPoint(3, 9)),
                            line(wxPoint(5, 9), wxPoint(6, 9)),
                            line(wxPoint(6, 9), wxPoint(4, 11)),
                            line(wxPoint(2, 9), wxPoint(4, 11)) };
    int count = sizeof(points) / sizeof(line);

    xlColor color(hsv);

    for (float i = 0; i < Ripple_Thickness; i += .5) {
        if (CheckBox_Ripple3D) {
            if (buffer.allowAlpha) {
                color.alpha = 255.0 * (1.0 - (float(i) / float(Ripple_Thickness)));
            } else {
                hsv.value *= 1.0 - (float(i) / float(Ripple_Thickness)); // we multiply by 1.0 when steps=0
                color = hsv;
            }
        }
        if (Movement == MOVEMENT_EXPLODE) {
            radius = radius + i;
        } else {
            radius = radius - i;
        }
        if (radius >= 0) {
            for (int j = 0; j < count; ++j) {
                int x1 = std::round(((double)points[j].start.x - 4.0) / 11.0 * radius);
                int y1 = std::round(((double)points[j].start.y - 4.0) / 11.0 * radius);
                int x2 = std::round(((double)points[j].end.x - 4.0) / 11.0 * radius);
                int y2 = std::round(((double)points[j].end.y - 4.0) / 11.0 * radius);
                buffer.DrawLine(xc + x1, yc + y1, xc + x2, yc + y2, color);
            }
        } else {
            break;
        }
    }
}

void RippleEffect::Drawcrucifix(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D)
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

    const line points[] = { line(wxPoint(2, 0), wxPoint(2, 6)),
                            line(wxPoint(2, 6), wxPoint(0, 6)),
                            line(wxPoint(0, 6), wxPoint(0, 7)),
                            line(wxPoint(0, 7), wxPoint(2, 7)),
                            line(wxPoint(2, 7), wxPoint(2, 10)),
                            line(wxPoint(2, 10), wxPoint(3, 10)),
                            line(wxPoint(3, 10), wxPoint(3, 7)),
                            line(wxPoint(3, 7), wxPoint(5, 7)),
                            line(wxPoint(5, 7), wxPoint(5, 6)),
                            line(wxPoint(5, 6), wxPoint(3, 6)),
                            line(wxPoint(3, 6), wxPoint(3, 0)),
                            line(wxPoint(3, 0), wxPoint(2, 0)) };
    int count = sizeof(points) / sizeof(line);

    xlColor color(hsv);

    for (float i = 0; i < Ripple_Thickness; i += .5) {
        if (CheckBox_Ripple3D) {
            if (buffer.allowAlpha) {
                color.alpha = 255.0 * (1.0 - (float(i) / float(Ripple_Thickness)));
            } else {
                hsv.value *= 1.0 - (float(i) / float(Ripple_Thickness)); // we multiply by 1.0 when steps=0
                color = hsv;
            }
        }
        if (Movement == MOVEMENT_EXPLODE) {
            radius = radius + i;
        } else {
            radius = radius - i;
        }
        if (radius >= 0) {
            for (int j = 0; j < count; ++j) {
                int x1 = std::round(((double)points[j].start.x - 2.5) / 7.0 * radius);
                int y1 = std::round(((double)points[j].start.y - 6.5) / 10.0 * radius);
                int x2 = std::round(((double)points[j].end.x - 2.5) / 7.0 * radius);
                int y2 = std::round(((double)points[j].end.y - 6.5) / 10.0 * radius);
                buffer.DrawLine(xc + x1, yc + y1, xc + x2, yc + y2, color);
            }
        } else {
            break;
        }
    }
}

void RippleEffect::Drawpresent(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D)
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

    xlColor color(hsv);

    for (float i = 0; i < Ripple_Thickness; i += .5) {
        if (CheckBox_Ripple3D) {
            if (buffer.allowAlpha) {
                color.alpha = 255.0 * (1.0 - (float(i) / float(Ripple_Thickness)));
            } else {
                hsv.value *= 1.0 - (float(i) / float(Ripple_Thickness)); // we multiply by 1.0 when steps=0
                color = hsv;
            }
        }
        if (Movement == MOVEMENT_EXPLODE) {
            radius = radius + i;
        } else {
            radius = radius - i;
        }
        if (radius >= 0) {
            for (int j = 0; j < count; ++j) {
                int x1 = std::round(((double)points[j].start.x - 5) / 7.0 * radius);
                int y1 = std::round(((double)points[j].start.y - 5.5) / 10.0 * radius);
                int x2 = std::round(((double)points[j].end.x - 5) / 7.0 * radius);
                int y2 = std::round(((double)points[j].end.y - 5.5) / 10.0 * radius);
                buffer.DrawLine(xc + x1, yc + y1, xc + x2, yc + y2, color);
            }
        } else {
            break;
        }
    }
}

void RippleEffect::Drawcandycane(RenderBuffer& buffer, int Movement, int xc, int yc, double radius, HSVValue& hsv, int Ripple_Thickness, int CheckBox_Ripple3D)
{
    double originalRadius = radius;
    xlColor color(hsv);

    for (float i = 0; i < Ripple_Thickness; i += .5) {
        if (CheckBox_Ripple3D) {
            if (buffer.allowAlpha) {
                color.alpha = 255.0 * (1.0 - (float(i) / float(Ripple_Thickness)));
            } else {
                hsv.value *= 1.0 - (float(i) / float(Ripple_Thickness)); // we multiply by 1.0 when steps=0
                color = hsv;
            }
        }
        if (Movement == MOVEMENT_EXPLODE) {
            radius = radius + i;
        } else {
            radius = radius - i;
        }
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
                x = std::round((r)*buffer.cos(radian) + xc + originalRadius / 6.0);
                int y = std::round((r)*buffer.sin(radian) + y1);
                buffer.SetPixel(x, y, color);
            }
        } else {
            break;
        }
    }
}

std::list<std::string> RippleEffect::GetFileReferences(Model* model, const SettingsMap& SettingsMap) const
{
    std::list<std::string> res;
    if (SettingsMap["E_FILEPICKERCTRL_SVG"] != "") {
        res.push_back(SettingsMap["E_FILEPICKERCTRL_SVG"]);
    }
    return res;
}

std::list<std::string> RippleEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    std::string object = settings["E_CHOICE_Ripple_Object_To_Draw"];
    if (object == "SVG") {
        auto svgFilename = settings.Get("E_FILEPICKERCTRL_SVG", "");

        if (svgFilename == "" || !FileExists(svgFilename)) {
            res.push_back(wxString::Format("    ERR: Ripple effect can't find SVG file '%s'. Model '%s', Start %s", svgFilename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        } else {
            if (!IsFileInShowDir(xLightsFrame::CurrentDir, svgFilename)) {
                res.push_back(wxString::Format("    WARN: Ripple effect SVG file '%s' not under show directory. Model '%s', Start %s", svgFilename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
            }
        }
    }

    return res;
}

bool RippleEffect::CleanupFileLocations(xLightsFrame* frame, SettingsMap& SettingsMap)
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