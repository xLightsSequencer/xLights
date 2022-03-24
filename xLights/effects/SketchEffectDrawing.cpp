#include "SketchEffectDrawing.h"

#include <wx/graphics.h>

#include <regex>
#include <xutility>

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

void SketchLine::DrawPartialSegment(wxGraphicsPath& path, const wxSize&sz, std::optional<double> startPercentage, double endPercentage)
{
    if (!startPercentage.has_value()) {
        if (endPercentage <= 0. || endPercentage > 1.)
            return;
        auto pt = (1 - endPercentage) * m_fromPt + endPercentage * m_toPt;
        path.AddLineToPoint(sz.x * pt.m_x, sz.y * pt.m_y);
    } else {
        double percentage = std::clamp(startPercentage.value(), 0., 1.);
        endPercentage = std::clamp(endPercentage, 0., 1.);
        if (percentage == endPercentage)
            return;

        auto pt = (1 - endPercentage) * m_fromPt + endPercentage * m_toPt;
        auto startPt = (1 - percentage) * m_fromPt + percentage * m_toPt;
        path.MoveToPoint(sz.x * startPt.m_x, sz.y * startPt.m_y);

        path.AddLineToPoint(sz.x * pt.m_x, sz.y * pt.m_y);
    }
}


double SketchQuadraticBezier::Length()
{
    return bezierLength(m_fromPt, m_cp, m_toPt);
}

void SketchQuadraticBezier::DrawEntireSegment(wxGraphicsPath& path, const wxSize& sz)
{
    path.AddQuadCurveToPoint(sz.x * m_cp.m_x, sz.y  * m_cp.m_y, sz.x * m_toPt.m_x, sz.y * m_toPt.m_y);
}

void SketchQuadraticBezier::DrawPartialSegment(wxGraphicsPath& path, const wxSize& sz, std::optional<double> startPercentage, double endPercentage)
{
    if (!startPercentage.has_value()) {
        auto pts = piecewiseLinearApproximation(m_fromPt, m_cp, m_toPt);
        int i = 0;
        double increment = 1. / pts.size();
        for (double t = 0.; t <= endPercentage; t += increment, ++i)
            path.AddLineToPoint(sz.x * pts[i].m_x, sz.y * pts[i].m_y);
    } else {
        // todo - startPercentage/endPercentage
    }
}


double SketchCubicBezier::Length()
{
    return bezierLength(m_fromPt, m_cp1, m_cp2, m_toPt);
}

 void SketchCubicBezier::DrawEntireSegment(wxGraphicsPath& path, const wxSize& sz)
{
    path.AddCurveToPoint(sz.x*m_cp1.m_x, sz.y*m_cp1.m_y, sz.x*m_cp2.m_x, sz.y*m_cp2.m_y, sz.x*m_toPt.m_x, sz.y*m_toPt.m_y);
}
void SketchCubicBezier::DrawPartialSegment(wxGraphicsPath& path, const wxSize& sz, std::optional<double> startPercentage, double endPercentage)
{
    if (!startPercentage.has_value()) {
        auto pts = piecewiseLinearApproximation(m_fromPt, m_cp1, m_cp2, m_toPt);
        int i = 0;
        double increment = 1. / pts.size();
        for (double t = 0.; t <= endPercentage; t += increment, ++i)
            path.AddLineToPoint(sz.x * pts[i].m_x, sz.y * pts[i].m_y);
    } else {
        // todo - startPercentage/endPercentage
    }
}

double SketchEffectPath::Length()
{
    if (m_segments.empty())
        return 0.;

    double len = 0.0;
    for (const auto& cmd : m_segments)
        len += cmd->Length();

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

    gc->StrokePath(path);
}

void SketchEffectPath::drawPartialPath(wxGraphicsContext* gc, const wxSize& sz, std::optional<double> startPercentage, double endPercentage)
{
    if (m_segments.empty())
        return;
    double totalLength = Length();
    double cumulativeLength = 0.;

    auto startPt(m_segments.front()->StartPoint());
    wxGraphicsPath path(gc->CreatePath());
    path.MoveToPoint(sz.x * startPt.m_x, sz.y * startPt.m_y);

    if (!startPercentage.has_value()) {
        for (auto& segment : m_segments) {
            double length = segment->Length();
            double percentageAtEndOfSegment = (cumulativeLength + length) / totalLength;
            if (endPercentage > percentageAtEndOfSegment) {
                segment->DrawEntireSegment(path, sz);
            } else {
                double percentageAtStartOfSegment = cumulativeLength / totalLength;
                if (endPercentage >= percentageAtStartOfSegment && endPercentage < percentageAtEndOfSegment) {
                    double segmentPercentage = (endPercentage - percentageAtStartOfSegment) / (percentageAtEndOfSegment - percentageAtStartOfSegment);
                    segment->DrawPartialSegment(path, sz, std::nullopt, segmentPercentage);
                }
            }
            cumulativeLength += length;
        }
    } else {
        for (auto& segment : m_segments) {
            double length = segment->Length();
            double percentageAtStartOfSegment = cumulativeLength / totalLength;
            double percentageAtEndOfSegment = (cumulativeLength + length) / totalLength;

            double segmentPercentage = (endPercentage - percentageAtStartOfSegment) / (percentageAtEndOfSegment - percentageAtStartOfSegment);
            double segmentDrawPercentage = (startPercentage.value() - percentageAtStartOfSegment) / (percentageAtEndOfSegment - percentageAtStartOfSegment);
            segment->DrawPartialSegment(path, sz, segmentDrawPercentage, segmentPercentage);

            cumulativeLength += length;
        }
    }

    gc->StrokePath(path);
}

void SketchEffectPath::closePath()
{
    if (m_segments.size() >= 1) {
        wxPoint2DDouble startPt( m_segments.back()->EndPoint() );
        wxPoint2DDouble endPt( m_segments.front()->StartPoint() );
        m_segments.push_back(std::make_shared <SketchLine>(startPt, endPt));
    }
}

std::string SketchEffectSketch::DefaultSketchString()
{
    return std::string("0.0,0.0;L0.5,0.5;C1.0,1.0,0.25,0.25,0.65,0.30;c|0.35,0.45;Q0.7,0.75,0.9,0.45|0.8,0.8;L0.3,0.7;L0.05,0.1");
}

SketchEffectSketch SketchEffectSketch::DefaultSketch()
{
    return SketchFromString(DefaultSketchString());
}

SketchEffectSketch SketchEffectSketch::SketchFromString(const std::string& sketchDef)
{
    static const std::regex pathsRegex("([^\\|]+)");
    static const std::regex pathComponentsRegex("([^;]+)");
    static const std::string doubleCapture("(-?[0-9]*\\.?[0-9]*)");
    static const std::string pointCapture(doubleCapture + ',' + doubleCapture);
    static const std::regex pathStartRegex(pointCapture);
    static const std::regex lineRegex("^L" + pointCapture);
    static const std::regex quadraticRegex("^Q" + pointCapture + ',' + pointCapture);
    static const std::regex cubicRegex("^C" + pointCapture + ',' + pointCapture + ',' + pointCapture);

    using std::sregex_iterator;
    SketchEffectSketch sketch;

    auto paths_begin = sregex_iterator(sketchDef.cbegin(), sketchDef.cend(), pathsRegex);
    for (sregex_iterator iter = paths_begin; iter != sregex_iterator(); ++iter) {
        auto path = std::make_shared<SketchEffectPath>();
        wxPoint2DDouble prevPt;

        std::string path_str((*iter).str());
        auto pathComponents_begin = sregex_iterator(path_str.cbegin(), path_str.cend(), pathComponentsRegex);
        for (sregex_iterator iter2 = pathComponents_begin; iter2 != sregex_iterator(); ++iter2) {
            std::string pathComponents_str((*iter2).str());
            // Path always begins with a start point
            if (iter2 == pathComponents_begin) {
                auto startPt_begin = sregex_iterator(pathComponents_str.cbegin(), pathComponents_str.cend(), pathStartRegex);
                if (std::distance(startPt_begin, sregex_iterator()) == 1 && (*startPt_begin).size() == 3) {
                    prevPt.m_x = std::stod((*startPt_begin)[1]);
                    prevPt.m_y = std::stod((*startPt_begin)[2]);
                }
            } else if (pathComponents_str.at(0) == 'L') {
                wxPoint2DDouble toPt;
                auto line_begin = sregex_iterator(pathComponents_str.cbegin(), pathComponents_str.cend(), lineRegex);
                if (std::distance(line_begin, sregex_iterator()) == 1 && (*line_begin).size() == 3) {
                    toPt.m_x = std::stod((*line_begin)[1]);
                    toPt.m_y = std::stod((*line_begin)[2]);
                    path->appendSegment(std::make_shared<SketchLine>(prevPt, toPt));
                    prevPt = toPt;
                }
            } else if (pathComponents_str.at(0) == 'Q') {
                wxPoint2DDouble ctrlPt, toPt;
                auto curve_begin = sregex_iterator(pathComponents_str.cbegin(), pathComponents_str.cend(), quadraticRegex);
                if (std::distance(curve_begin, sregex_iterator()) == 1 && (*curve_begin).size() == 5) {
                    ctrlPt.m_x = std::stod((*curve_begin)[1]);
                    ctrlPt.m_y = std::stod((*curve_begin)[2]);
                    toPt.m_x = std::stod((*curve_begin)[3]);
                    toPt.m_y = std::stod((*curve_begin)[4]);
                    path->appendSegment(std::make_shared<SketchQuadraticBezier>(prevPt, ctrlPt, toPt));
                    prevPt = toPt;
                }

            } else if (pathComponents_str.at(0) == 'C') {
                wxPoint2DDouble ctrlPt1, ctrlPt2, toPt;
                auto curve_begin = sregex_iterator(pathComponents_str.cbegin(), pathComponents_str.cend(), cubicRegex);
                if (std::distance(curve_begin, sregex_iterator()) == 1 && (*curve_begin).size() == 7) {
                    ctrlPt1.m_x = std::stod((*curve_begin)[1]);
                    ctrlPt1.m_y = std::stod((*curve_begin)[2]);
                    ctrlPt2.m_x = std::stod((*curve_begin)[3]);
                    ctrlPt2.m_y = std::stod((*curve_begin)[4]);
                    toPt.m_x = std::stod((*curve_begin)[5]);
                    toPt.m_y = std::stod((*curve_begin)[6]);
                    path->appendSegment(std::make_shared<SketchCubicBezier>(prevPt, ctrlPt1, ctrlPt2, toPt));
                    prevPt = toPt;
                }
            } else if (pathComponents_str.at(0) == 'c') {
                path->closePath();
            }
        }

        sketch.appendPath(path);
    }

    return sketch;
}
