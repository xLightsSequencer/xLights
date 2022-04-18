#include "SketchEffectDrawing.h"

#include <wx/graphics.h>

#include <iterator>
#include <regex>
#include <sstream>

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
    // approximation... bumping this up (temporarily?) to fix weirdness
    // when the 'motion' attribute is enabled. So far, a higher number
    // doesn't seem to affect render times very much.
    const int NUM_STEPS = /*50*/500;

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
                                                              const wxPoint2DDouble& endPt,
                                                              int numSteps = NUM_STEPS)
    {
        std::vector<wxPoint2DDouble> pts(numSteps);
        for (int i = 1; i <= numSteps; ++i)
            pts[i - 1] = bezierPoint(i / float(numSteps), startPt, ctrlPt1, ctrlPt2, endPt);

        return pts;
    }
    std::vector<wxPoint2DDouble> piecewiseLinearApproximation(const wxPoint2DDouble& startPt,
                                                              const wxPoint2DDouble& ctrlPt,
                                                              const wxPoint2DDouble& endPt,
                                                              int numSteps = NUM_STEPS)
    {
        std::vector<wxPoint2DDouble> pts(numSteps);
        for (int i = 1; i <= numSteps; ++i)
            pts[i - 1] = bezierPoint(i / float(numSteps), startPt, ctrlPt, endPt);

        return pts;
    }

    double minDistSqrPointToLineSegment(const wxPoint2DDouble& v, const wxPoint2DDouble& w, const wxPoint2DDouble& p)
    {
        double l2 = v.GetDistanceSquare(w);
        if (l2 == 0.) //  v == w case
            return p.GetDistance(v);

        // Consider the line extending the segment, parameterized as v + t (w - v).
        // We find projection of point p onto the line.
        // It falls where t = [(p-v) . (w-v)] / |w-v|^2
        // We clamp t from [0,1] to handle points outside the segent vw.
        double dot = wxPoint2DDouble(p - v).GetDotProduct(wxPoint2DDouble(w - v));
        double t = std::clamp(dot / l2, 0., 1.);
        wxPoint2DDouble projection(v + t * (w - v));
        return p.GetDistanceSquare(projection);
    }

    const double HIT_TEST_DIST_SQR_LIMIT = 0.0075 * 0.0075;
}


void SketchLine::DrawEntireSegment(wxGraphicsPath& path, const wxSize& sz) const
{
    path.AddLineToPoint(sz.x * m_toPt.m_x, sz.y * m_toPt.m_y);
}

void SketchLine::DrawPartialSegment(wxGraphicsPath& path, const wxSize&sz, std::optional<double> startPercentage, double endPercentage) const
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

bool SketchLine::HitTest(const wxPoint2DDouble& pt) const
{
    double distSqr = minDistSqrPointToLineSegment(m_fromPt, m_toPt, pt);
    return distSqr <= HIT_TEST_DIST_SQR_LIMIT;
}


double SketchQuadraticBezier::Length() const
{
    return bezierLength(m_fromPt, m_cp, m_toPt);
}

void SketchQuadraticBezier::DrawEntireSegment(wxGraphicsPath& path, const wxSize& sz) const
{
    path.AddQuadCurveToPoint(sz.x * m_cp.m_x, sz.y  * m_cp.m_y, sz.x * m_toPt.m_x, sz.y * m_toPt.m_y);
}

void SketchQuadraticBezier::DrawPartialSegment(wxGraphicsPath& path, const wxSize& sz, std::optional<double> startPercentage, double endPercentage) const
{
    if (!startPercentage.has_value()) {
        auto pts = piecewiseLinearApproximation(m_fromPt, m_cp, m_toPt);
        int i = 0;
        double increment = 1. / pts.size();
        for (double t = 0.; t <= endPercentage; t += increment, ++i)
            path.AddLineToPoint(sz.x * pts[i].m_x, sz.y * pts[i].m_y);
    } else {
        double percentage = std::clamp(startPercentage.value(), 0., 1.);
        endPercentage = std::clamp(endPercentage, 0., 1.);
        if (percentage == endPercentage)
            return;

        auto pts = piecewiseLinearApproximation(m_fromPt, m_cp, m_toPt);
        int i = 0;
        double increment = 1. / pts.size();
        bool started = false;
        for (double t = 0.; t <= endPercentage; t += increment, ++i) {
            if (t >= startPercentage && !started) {
                path.MoveToPoint(sz.x * pts[i].m_x, sz.y * pts[i].m_y);
                started = true;
            } else if (t >= startPercentage && t <= endPercentage) {
                path.AddLineToPoint(sz.x * pts[i].m_x, sz.y * pts[i].m_y);
            }
        }
    }
}

bool SketchQuadraticBezier::HitTest(const wxPoint2DDouble& pt) const
{
    auto points = piecewiseLinearApproximation(m_fromPt, m_cp, m_toPt, 40);
    wxPoint2DDouble startPt(points.front());
    for (size_t i = 1; i < points.size(); ++i) {
        double distSqr = minDistSqrPointToLineSegment(startPt, points[i], pt);
        if (distSqr < HIT_TEST_DIST_SQR_LIMIT)
            return true;
        startPt = points[i];
    }
    return false;
}


double SketchCubicBezier::Length() const
{
    return bezierLength(m_fromPt, m_cp1, m_cp2, m_toPt);
}

 void SketchCubicBezier::DrawEntireSegment(wxGraphicsPath& path, const wxSize& sz) const
{
    path.AddCurveToPoint(sz.x*m_cp1.m_x, sz.y*m_cp1.m_y, sz.x*m_cp2.m_x, sz.y*m_cp2.m_y, sz.x*m_toPt.m_x, sz.y*m_toPt.m_y);
}
void SketchCubicBezier::DrawPartialSegment(wxGraphicsPath& path, const wxSize& sz, std::optional<double> startPercentage, double endPercentage) const
{
    if (!startPercentage.has_value()) {
        auto pts = piecewiseLinearApproximation(m_fromPt, m_cp1, m_cp2, m_toPt);
        int i = 0;
        double increment = 1. / pts.size();
        for (double t = 0.; t <= endPercentage; t += increment, ++i)
            path.AddLineToPoint(sz.x * pts[i].m_x, sz.y * pts[i].m_y);
    } else {
        double percentage = std::clamp(startPercentage.value(), 0., 1.);
        endPercentage = std::clamp(endPercentage, 0., 1.);
        if (percentage == endPercentage)
            return;

        auto pts = piecewiseLinearApproximation(m_fromPt, m_cp1, m_cp2, m_toPt);
        int i = 0;
        double increment = 1. / pts.size();
        bool started = false;
        for (double t = 0.; t <= endPercentage; t += increment, ++i) {
            if (t >= startPercentage && !started) {
                path.MoveToPoint(sz.x * pts[i].m_x, sz.y * pts[i].m_y);
                started = true;
            } else if (t >= startPercentage && t <= endPercentage) {
                path.AddLineToPoint(sz.x * pts[i].m_x, sz.y * pts[i].m_y);
            }
        }
    }
}

bool SketchCubicBezier::HitTest(const wxPoint2DDouble& pt) const
{
    auto points = piecewiseLinearApproximation(m_fromPt, m_cp1, m_cp2, m_toPt, 40);
    wxPoint2DDouble startPt(points.front());
    for (size_t i = 1; i < points.size(); ++i) {
        double distSqr = minDistSqrPointToLineSegment(startPt, points[i], pt);
        if (distSqr < HIT_TEST_DIST_SQR_LIMIT)
            return true;
        startPt = points[i];
    }
    return false;
}


double SketchEffectPath::Length() const
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

void SketchEffectPath::drawEntirePath(wxGraphicsContext* gc, const wxSize& sz) const
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

void SketchEffectPath::drawPartialPath(wxGraphicsContext* gc, const wxSize& sz, std::optional<double> startPercentage, double endPercentage) const
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
            if (length != 0.0) {
                //if length is 0, skip (partially because you get divide by 0 for the two percentages below
                double percentageAtStartOfSegment = cumulativeLength / totalLength;
                double percentageAtEndOfSegment = (cumulativeLength + length) / totalLength;

                double segmentPercentage = (endPercentage - percentageAtStartOfSegment) / (percentageAtEndOfSegment - percentageAtStartOfSegment);
                double segmentDrawPercentage = (startPercentage.value() - percentageAtStartOfSegment) / (percentageAtEndOfSegment - percentageAtStartOfSegment);
                
                segment->DrawPartialSegment(path, sz, segmentDrawPercentage, segmentPercentage);

                cumulativeLength += length;
            }
        }
    }

    gc->StrokePath(path);
}

void SketchEffectPath::closePath()
{
    if (!m_isClosed && m_segments.size() >= 1) {
        wxPoint2DDouble startPt( m_segments.back()->EndPoint() );
        wxPoint2DDouble endPt( m_segments.front()->StartPoint() );
        m_segments.push_back(std::make_shared <SketchLine>(startPt, endPt));
        m_isClosed = true;
    }
}

std::string SketchEffectSketch::DefaultSketchString()
{
    return "0.490716,0.0175163;L0.435564,0.0173168;L0.436271,0.174786;L0.0321957,0.17094;L0.21812,0.374054;L0.105848,0.374764;L0.311564,0.594017;L0.174158,0.596749;L0.45484,0.901005;Q0.493053,0.881147,0.530929,0.899586;L0.824461,0.599586;L0.667346,0.601709;L0.888037,0.373345;L0.77475,0.375473;L0.965797,0.170425;L0.559246,0.173291;L0.55832,0.0166076;L0.504768,0.0175163;L0.490716,0.0175163;c|0.493955,0.894099;Q0.434437,0.884836,0.421587,0.934047;Q0.44864,0.980943,0.496661,0.978048;Q0.553473,0.972837,0.574102,0.928258;Q0.549077,0.885415,0.496999,0.893521;L0.493955,0.894099;L0.493955,0.894099;c";
}

SketchEffectSketch SketchEffectSketch::DefaultSketch()
{
    return SketchFromString(DefaultSketchString());
}

SketchEffectSketch SketchEffectSketch::SketchFromString(const std::string& sketchDef)
{
    // Basic syntax of a sketch:
    //  * paths are separated by '|'
    //  * within a path, ';' is the separator
    //  * paths always begin with a start point (followed by the ';' separator)
    //  * paths with a start point and no segments... probably do not work currently
    //  * within a path, only four "commands" (in SVG syntax) are supported currently:
    //      - 'L' for line segments
    //      - 'Q' for quadratic-curve segments
    //      - 'C' for cubic-curve segments
    //      - 'c' close the path with a line to the start point (must be the last command)
    //      - 'p' pause time (probably in seconds) -- CURRENTLY UNIMPLEMENTED
    //
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

std::string SketchEffectSketch::toString() const
{
    if (m_paths.empty() )
        return std::string();

    std::ostringstream stream;
    for ( size_t i = 0; i < m_paths.size(); ++i ) {
        std::shared_ptr<SketchEffectPath> path = m_paths[i];
        auto segments = path->segments();
        if (segments.empty())
            continue;

        auto startPt(segments[0]->StartPoint());
        stream << startPt.m_x << ',' << startPt.m_y << ';';
        for (size_t ii = 0; ii < segments.size(); ++ii) {
            std::shared_ptr<SketchCubicBezier> cubic;
            std::shared_ptr<SketchQuadraticBezier> quadratic;

            if (std::dynamic_pointer_cast<SketchLine>(segments[ii]) != nullptr) {
                auto endPt(segments[ii]->EndPoint());
                stream << 'L' << endPt.m_x << ',' << endPt.m_y << ';';
            } else if ((quadratic = std::dynamic_pointer_cast<SketchQuadraticBezier>(segments[ii]))) {
                auto ctrlPt(quadratic->ControlPoint());
                auto endPt(quadratic->EndPoint());
                stream << 'Q' << ctrlPt.m_x << ',' << ctrlPt.m_y
                       << ',' << endPt.m_x << ',' << endPt.m_y << ';';
            } else if ((cubic = std::dynamic_pointer_cast<SketchCubicBezier>(segments[ii]))) {
                auto ctrlPt1(cubic->ControlPoint1());
                auto ctrlPt2(cubic->ControlPoint2());
                auto endPt(cubic->EndPoint());
                stream << 'C' << ctrlPt1.m_x << ',' << ctrlPt1.m_y << ','
                       << ctrlPt2.m_x << ',' << ctrlPt2.m_y << ','
                       << endPt.m_x << ',' << endPt.m_y << ';';
            }
        }
        if (path->isClosed())
            stream << 'c';
        if (i != m_paths.size() - 1)
            stream << '|';
    }
    return stream.str();
}

void SketchEffectSketch::updatePath(int index, std::shared_ptr<SketchEffectPath> path)
{
    if (index < 0 || index >= m_paths.size())
        return;
    m_paths[index] = path;
}

void SketchEffectSketch::deletePath(int pathIndex)
{
    auto iter = m_paths.begin();
    std::advance(iter, pathIndex);

    m_paths.erase(iter);
}
