#include "SketchEffectDrawing.h"

#include <wx/graphics.h>

namespace
{
    double bezier(double t, double start, double control1, double control2, double end)
    {
        double u = 1 - t;
        return start * u * u * u + control1 * 3 * u * u * t + control2 * 3 * u * t * t + end * t * t * t;
    }
    double bezier(double t, double start, double control, double end)
    {
        double u = 1 - t;
        return start * u * u + control * 2 * u * t + end * t * t;
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
    wxPoint2DDouble bezierPoint(double t,
                                const wxPoint2DDouble& startPt,
                                const wxPoint2DDouble& ctrlPt,
                                const wxPoint2DDouble& endPt)
    {
        return wxPoint2DDouble(bezier(t, startPt.m_x, ctrlPt.m_x, endPt.m_x),
                               bezier(t, startPt.m_y, ctrlPt.m_y, endPt.m_y));
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
    double bezierLength(const wxPoint2DDouble& startPt,
                        const wxPoint2DDouble& ctrlPt,
                        const wxPoint2DDouble& endPt)
    {
        wxPoint2DDouble previousPt(startPt);
        double length = 0.;

        for (int i = 1; i <= NUM_STEPS; ++i) {
            double t = double(i) / NUM_STEPS;
            wxPoint2DDouble pt(bezierPoint(t, startPt, ctrlPt, endPt));
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
    std::vector<wxPoint2DDouble> piecewiseLinearApproximation(const wxPoint2DDouble& startPt,
                                                              const wxPoint2DDouble& ctrlPt,
                                                              const wxPoint2DDouble& endPt)
    {
        std::vector<wxPoint2DDouble> pts(NUM_STEPS);
        for (int i = 1; i <= NUM_STEPS; ++i)
            pts[i - 1] = bezierPoint(i / float(NUM_STEPS), startPt, ctrlPt, endPt);

        return pts;
    }

}


void SketchLine::DrawEntireSegment(wxGraphicsPath& path, const wxSize& sz)
{
    path.AddLineToPoint(sz.x * m_toPt.m_x, sz.y * m_toPt.m_y);
}

void SketchLine::DrawPartialSegment(wxGraphicsPath& path, const wxSize&sz, double startPercentage, double endPercentage)
{
    // todo - startPercentage/endPercentage
    auto pt = (1 - endPercentage) * m_fromPt + endPercentage * m_toPt;
    path.AddLineToPoint(sz.x *pt.m_x, sz.y * pt.m_y);
}


double SketchQuadraticBezier::Length()
{
    return bezierLength(m_fromPt, m_cp, m_toPt);
}

void SketchQuadraticBezier::DrawEntireSegment(wxGraphicsPath& path, const wxSize& sz)
{
    path.AddQuadCurveToPoint(sz.x * m_cp.m_x, sz.y  * m_cp.m_y, sz.x * m_toPt.m_x, sz.y * m_toPt.m_y);
}

void SketchQuadraticBezier::DrawPartialSegment(wxGraphicsPath& path, const wxSize& sz, double percentage, double endPercentage)
{
    // todo - startPercentage/endPercentage
    auto pts = piecewiseLinearApproximation(m_fromPt, m_cp, m_toPt);
    int i = 0;
    double increment = 1. / pts.size();
    for (double t = 0.; t <= endPercentage; t += increment, ++i)
        path.AddLineToPoint(sz.x * pts[i].m_x, sz.y * pts[i].m_y);
}


double SketchCubicBezier::Length()
{
    return bezierLength(m_fromPt, m_cp1, m_cp2, m_toPt);
}

 void SketchCubicBezier::DrawEntireSegment(wxGraphicsPath& path, const wxSize& sz)
{
    path.AddCurveToPoint(sz.x*m_cp1.m_x, sz.y*m_cp1.m_y, sz.x*m_cp2.m_x, sz.y*m_cp2.m_y, sz.x*m_toPt.m_x, sz.y*m_toPt.m_y);
}
void SketchCubicBezier::DrawPartialSegment(wxGraphicsPath& path, const wxSize& sz, double startPercentage, double endPercentage)
{
    // todo - startPercentage/endPercentage
    auto pts = piecewiseLinearApproximation(m_fromPt, m_cp1, m_cp2, m_toPt);
    int i = 0;
    double increment = 1. / pts.size();
    for (double t = 0.; t <= endPercentage; t += increment, ++i)
        path.AddLineToPoint(sz.x * pts[i].m_x, sz.y * pts[i].m_y);
}

double SketchEffectPath::Length()
{
    if (m_segments.empty())
        return 0.;

    double len = 0.0;
    for (const auto& cmd : m_segments)
        len += cmd->Length();

    if (m_closePath)
        len += m_segments.front()->StartPoint().GetDistance(m_segments.back()->EndPoint());

    return len;
}

void SketchEffectPath::appendSegment(std::shared_ptr<SketchPathSegment> cmd)
{
    m_segments.push_back(cmd);
}

void SketchEffectPath::drawEntirePath(wxGraphicsContext* gc, const wxSize& sz)
{
    if (m_segments.empty())
        return;
    auto startPt(m_segments.front()->StartPoint());
    wxGraphicsPath path(gc->CreatePath());
    path.MoveToPoint(sz.x * startPt.m_x, sz.y * startPt.m_y);

    for (auto& cmd : m_segments)
        cmd->DrawEntireSegment(path, sz);

    if (m_closePath && !m_segments.empty())
        path.CloseSubpath();

    gc->StrokePath(path);
}

void SketchEffectPath::drawPartialPath(wxGraphicsContext* gc, const wxSize& sz, double startPercentage, double endPercentage)
{
    if (m_segments.empty())
        return;

    // todo - startPercentage/endPercentage
    double totalLength = Length();

    wxGraphicsPath path(gc->CreatePath());
    auto startPt(m_segments.front()->StartPoint());
    path.MoveToPoint(sz.x * startPt.m_x, sz.y * startPt.m_y);
    double cumulativeLength = 0.;
    for (auto& cmd : m_segments) {
        double length = cmd->Length();
        double percentageAtEndOfSegment = (cumulativeLength + length) / totalLength;
        if (endPercentage > percentageAtEndOfSegment)
            cmd->DrawEntireSegment(path, sz);
        else {
            double percentageAtStartOfSegment = cumulativeLength / totalLength;
            if (endPercentage >= percentageAtStartOfSegment && endPercentage < percentageAtEndOfSegment) {
                double segmentPercentage = (endPercentage - percentageAtStartOfSegment) / (percentageAtEndOfSegment - percentageAtStartOfSegment);
                cmd->DrawPartialSegment(path, sz, 0., segmentPercentage);
            }
        }
        cumulativeLength += length;
    }

    if (m_closePath && !m_segments.empty()) {
        double percentageAtStartOfClose = cumulativeLength / totalLength;
        if (endPercentage >= percentageAtStartOfClose && endPercentage < 1.) {
            double closePercentage = (endPercentage - percentageAtStartOfClose) / (1. - percentageAtStartOfClose);
            auto pt = (1 - closePercentage) * m_segments.back()->EndPoint() + closePercentage * startPt;
            path.AddLineToPoint(sz.x * pt.m_x, sz.y * pt.m_y);
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
    SketchEffectSketch sketch;
    const wxPoint2DDouble path1Start(0., 0.);
    const wxPoint2DDouble path2Start(0.35, 0.45);
    const wxPoint2DDouble path3Start(0.8, 0.8);

    auto path1 = std::make_shared<SketchEffectPath>();
    auto path1Segment1 = std::make_shared<SketchLine>(path1Start, wxPoint2DDouble(0.5, 0.5));
    auto path1Segment2 = std::make_shared<SketchCubicBezier>(path1Segment1->EndPoint(),
                                                             wxPoint2DDouble(1., 1.),
                                                             wxPoint2DDouble(0.25, 0.25),
                                                             wxPoint2DDouble(0.65, 0.3));
    path1->appendSegment(path1Segment1);
    path1->appendSegment(path1Segment2);
    path1->closePath();

    auto path2 = std::make_shared<SketchEffectPath>();
    auto path2Segment1 = std::make_shared<SketchQuadraticBezier>(path2Start,
                                                                 wxPoint2DDouble(0.7, 0.75),
                                                                 wxPoint2DDouble(0.9, 0.45));
    path2->appendSegment(path2Segment1);

    auto path3 = std::make_shared<SketchEffectPath>();
    auto path3Segment1 = std::make_shared<SketchLine>(path3Start, wxPoint2DDouble(0.3, 0.7));
    auto path3Segment2 = std::make_shared<SketchLine>(wxPoint2DDouble(0.3, 0.7),
                                                      wxPoint2DDouble(0.05, 0.1));
    path3->appendSegment(path3Segment1);
    path3->appendSegment(path3Segment2);

    sketch.appendPath(path1);
    sketch.appendPath(path2);
    sketch.appendPath(path3);

    return sketch;
}
