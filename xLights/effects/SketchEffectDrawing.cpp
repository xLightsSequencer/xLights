#include "SketchEffectDrawing.h"

#include <wx/graphics.h>

namespace
{
    double bezier(double t, double start, double control1, double control2, double end)
    {
        double u = 1 - t;
        return start * u * u * u + control1 * 3 * u * u * t + control2 * 3 * u * t * t + end * t * t * t;
    }

    wxPoint2DDouble bezierPoint(double t,
                                const wxPoint2DDouble& startPt,
                                const wxPoint2DDouble& ctrlPt1,
                                const wxPoint2DDouble& ctrlPt2,
                                const wxPoint2DDouble& endPt)
    {
        return wxPoint2DDouble(bezier(t, startPt.m_x, ctrlPt1.m_x, ctrlPt2.m_x, endPt.m_x),
                               bezier(t, startPt.m_y, ctrlPt1.m_y, ctrlPt2.m_y, endPt.m_y));
    }

    // number of steps to use for approximating bezier segment length
    // and approximating a partial bezier segment with a piecewise-linear
    // approximation
    const int NUM_STEPS = 40;

    double bezierLength(const wxPoint2DDouble& startPt,
                        const wxPoint2DDouble& ctrlPt1,
                        const wxPoint2DDouble& ctrlPt2,
                        const wxPoint2DDouble& endPt)
    {
        wxPoint2DDouble previousPt(startPt);
        double length = 0.;

        for (int i = 1; i <= NUM_STEPS; ++i) {
            double t = double(i) / NUM_STEPS;
            wxPoint2DDouble pt(bezierPoint(t, startPt, ctrlPt1, ctrlPt2, endPt));
            length += pt.GetDistance(previousPt);
            previousPt = pt;
        }
        return length;
    }

    std::vector<wxPoint2DDouble> piecewiseLinearApproximation(const wxPoint2DDouble& startPt,
                                                              const wxPoint2DDouble& ctrlPt1,
                                                              const wxPoint2DDouble& ctrlPt2,
                                                              const wxPoint2DDouble& endPt)
    {
        std::vector<wxPoint2DDouble> pts(NUM_STEPS);
        for (int i = 1; i <= NUM_STEPS; ++i)
            pts[i - 1] = bezierPoint(i / float(NUM_STEPS), startPt, ctrlPt1, ctrlPt2, endPt);

        return pts;
    }
}

void SketchLine::DrawEntireSegment(wxGraphicsPath& path)
{
    path.AddLineToPoint(m_toPt);
}

void SketchLine::DrawPartialSegment(wxGraphicsPath& path, double percentage)
{
    if (percentage <= 0. || percentage > 1.)
        return;
    auto pt = (1 - percentage) * m_fromPt + percentage * m_toPt;
    path.AddLineToPoint(pt);
}

double SketchCubicBezier::Length()
{
    return bezierLength(m_fromPt, m_cp1, m_cp2, m_toPt);
}

 void SketchCubicBezier::DrawEntireSegment(wxGraphicsPath& path)
{
    path.AddCurveToPoint(m_cp1, m_cp2, m_toPt);
}
void SketchCubicBezier::DrawPartialSegment(wxGraphicsPath& path, double percentage)
{
    if (percentage <= 0. || percentage > 1.)
        return;
    auto pts = piecewiseLinearApproximation(m_fromPt, m_cp1, m_cp2, m_toPt);
    int i = 0;
    double increment = 1. / pts.size();
    for (double t = 0.; t <= percentage; t += increment, ++i)
        path.AddLineToPoint(pts[i]);
}

double SketchEffectPath::Length()
{
    double len = 0.0;
    for (const auto& cmd : m_segments)
        len += cmd->Length();

    if (m_closePath && !m_segments.empty())
        len += m_startPt.GetDistance(m_segments.back()->EndPoint());

    return len;
}

void SketchEffectPath::appendSegment(std::shared_ptr<SketchPathSegment> cmd)
{
    m_segments.push_back(cmd);
}

void SketchEffectPath::drawEntirePath(wxGraphicsContext* gc)
{
    wxGraphicsPath path(gc->CreatePath());
    path.MoveToPoint(m_startPt);

    for (auto& cmd : m_segments)
        cmd->DrawEntireSegment(path);

    if (m_closePath && !m_segments.empty())
        path.CloseSubpath();

    gc->StrokePath(path);
}

void SketchEffectPath::drawPartialPath(wxGraphicsContext* gc, double percentage)
{
    double totalLength = Length();

    wxGraphicsPath path(gc->CreatePath());
    path.MoveToPoint(m_startPt);
    double cumulativeLength = 0.;
    for (auto& cmd : m_segments) {
        double length = cmd->Length();
        double percentageAtEndOfSegment = (cumulativeLength + length) / totalLength;
        if (percentage > percentageAtEndOfSegment)
            cmd->DrawEntireSegment(path);
        else {
            double percentageAtStartOfSegment = cumulativeLength / totalLength;
            if (percentage >= percentageAtStartOfSegment && percentage < percentageAtEndOfSegment) {
                double segmentPercentage = (percentage - percentageAtStartOfSegment) / (percentageAtEndOfSegment - percentageAtStartOfSegment);
                cmd->DrawPartialSegment(path, segmentPercentage);
            }
        }
        cumulativeLength += length;
    }

    if (m_closePath && !m_segments.empty()) {
        double percentageAtStartOfClose = cumulativeLength / totalLength;
        if (percentage >= percentageAtStartOfClose && percentage < 1.) {
            double closePercentage = (percentage - percentageAtStartOfClose) / (1. - percentageAtStartOfClose);
            auto pt = (1 - closePercentage) * m_segments.back()->EndPoint() + closePercentage * m_startPt;
            path.AddLineToPoint(pt);
        }
    }

    gc->StrokePath(path);
}

void SketchEffectPath::closePath()
{
    m_closePath = true;
}

SketchEffectSketch SketchEffectSketch::DefaultSketch()
{
    const int Width = 128;
    const int Height = 96;

    SketchEffectSketch sketch(Width, Height);
    const wxPoint2DDouble path1Start(0., 0.);
    const wxPoint2DDouble path2Start(0.8 * Width, 0.8 * Height);

    auto path1 = std::make_shared<SketchEffectPath>(path1Start);
    auto path1Segment1 = std::make_shared<SketchLine>(path1Start, wxPoint2DDouble(0.5 * Width, 0.5 * Height));
    auto path1Segment2 = std::make_shared<SketchCubicBezier>(path1Segment1->EndPoint(),
                                                             wxPoint2DDouble(Width, Height),
                                                             wxPoint2DDouble(0.25 * Width, 0.25 * Height),
                                                             wxPoint2DDouble(0.65 * Width, 0.3 * Height));
    path1->appendSegment(path1Segment1);
    path1->appendSegment(path1Segment2);
    path1->closePath();

    auto path2 = std::make_shared<SketchEffectPath>(path2Start);
    auto path2Segment1 = std::make_shared<SketchLine>(path2Start, wxPoint2DDouble(0.3 * Width, 0.7 * Height));
    auto path2Segment2 = std::make_shared<SketchLine>(wxPoint2DDouble(0.3 * Width, 0.7 * Height),
                                                      wxPoint2DDouble(0.05 * Width, 0.1 * Height));
    path2->appendSegment(path2Segment1);
    path2->appendSegment(path2Segment2);

    sketch.appendPath(path1);
    sketch.appendPath(path2);

    return sketch;
}
