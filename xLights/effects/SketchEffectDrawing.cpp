#include "SketchEffectDrawing.h"

#include <log.h>

#include <algorithm>
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

    xlPointD bezierPoint(double t,
                                const xlPointD& startPt,
                                const xlPointD& ctrlPt1,
                                const xlPointD& ctrlPt2,
                                const xlPointD& endPt)
    {
        return xlPointD(bezier(t, startPt.x, ctrlPt1.x, ctrlPt2.x, endPt.x),
                               bezier(t, startPt.y, ctrlPt1.y, ctrlPt2.y, endPt.y));
    }
    xlPointD bezierPoint(double t,
                                const xlPointD& startPt,
                                const xlPointD& ctrlPt,
                                const xlPointD& endPt)
    {
        return xlPointD(bezier(t, startPt.x, ctrlPt.x, endPt.x),
                               bezier(t, startPt.y, ctrlPt.y, endPt.y));
    }

    // number of steps to use for approximating bezier segment length
    // and approximating a partial bezier segment with a piecewise-linear
    // approximation... bumping this up (temporarily?) to fix weirdness
    // when the 'motion' attribute is enabled. So far, a higher number
    // doesn't seem to affect render times very much.
    const int NUM_STEPS = /*50*/ 500;

    double bezierLength(const xlPointD& startPt,
                        const xlPointD& ctrlPt1,
                        const xlPointD& ctrlPt2,
                        const xlPointD& endPt)
    {
        xlPointD previousPt(startPt);
        double length = 0.;

        for (int i = 1; i <= NUM_STEPS; ++i) {
            double t = double(i) / NUM_STEPS;
            xlPointD pt(bezierPoint(t, startPt, ctrlPt1, ctrlPt2, endPt));
            length += pt.distance(previousPt);
            previousPt = pt;
        }
        return length;
    }
    double bezierLength(const xlPointD& startPt,
                        const xlPointD& ctrlPt,
                        const xlPointD& endPt)
    {
        xlPointD previousPt(startPt);
        double length = 0.;

        for (int i = 1; i <= NUM_STEPS; ++i) {
            double t = double(i) / NUM_STEPS;
            xlPointD pt(bezierPoint(t, startPt, ctrlPt, endPt));
            length += pt.distance(previousPt);
            previousPt = pt;
        }
        return length;
    }
    void bezierProgressPosition(const xlPointD& startPt,
                                const xlPointD& ctrlPt,
                                const xlPointD& endPt,
                                const double targetLength,
                                double& x,
                                double& y)
    {
        xlPointD previousPt(startPt);
        double curLength = targetLength;

        for (int i = 1; i <= NUM_STEPS; ++i) {
            double t = double(i) / NUM_STEPS;
            xlPointD pt(bezierPoint(t, startPt, ctrlPt, endPt));
            double length = pt.distance(previousPt);
            if( length >= curLength ) {
                double percent = curLength / length;
                x = previousPt.x + percent * (pt.x - previousPt.x);
                y = previousPt.y + percent * (pt.y - previousPt.y);
                return;
            } else {
                curLength -= length;
            }
            previousPt = pt;
        }
    }
    void bezierProgressPosition(const xlPointD& startPt,
                                const xlPointD& ctrlPt1,
                                const xlPointD& ctrlPt2,
                                const xlPointD& endPt,
                                const double targetLength,
                                double& x,
                                double& y)
    {
        xlPointD previousPt(startPt);
        double curLength = targetLength;

        for (int i = 1; i <= NUM_STEPS; ++i) {
            double t = double(i) / NUM_STEPS;
            xlPointD pt(bezierPoint(t, startPt, ctrlPt1, ctrlPt2, endPt));
            double length = pt.distance(previousPt);
            if( length >= curLength ) {
                double percent = curLength / length;
                x = previousPt.x + percent * (pt.x - previousPt.x);
                y = previousPt.y + percent * (pt.y - previousPt.y);
                return;
            } else {
                curLength -= length;
            }
            previousPt = pt;
        }
    }

    std::vector<xlPointD> piecewiseLinearApproximation(const xlPointD& startPt,
                                                              const xlPointD& ctrlPt1,
                                                              const xlPointD& ctrlPt2,
                                                              const xlPointD& endPt,
                                                              int numSteps = NUM_STEPS)
    {
        std::vector<xlPointD> pts(numSteps);
        for (int i = 1; i <= numSteps; ++i)
            pts[i - 1] = bezierPoint(i / float(numSteps), startPt, ctrlPt1, ctrlPt2, endPt);

        return pts;
    }
    std::vector<xlPointD> piecewiseLinearApproximation(const xlPointD& startPt,
                                                              const xlPointD& ctrlPt,
                                                              const xlPointD& endPt,
                                                              int numSteps = NUM_STEPS)
    {
        std::vector<xlPointD> pts(numSteps);
        for (int i = 1; i <= numSteps; ++i)
            pts[i - 1] = bezierPoint(i / float(numSteps), startPt, ctrlPt, endPt);

        return pts;
    }

    double minDistSqrPointToLineSegment(const xlPointD& v, const xlPointD& w, const xlPointD& p)
    {
        double l2 = (w - v).dot(w - v);
        if (l2 == 0.) //  v == w case
            return p.distance(v);

        // Consider the line extending the segment, parameterized as v + t (w - v).
        // We find projection of point p onto the line.
        // It falls where t = [(p-v) . (w-v)] / |w-v|^2
        // We clamp t from [0,1] to handle points outside the segent vw.
        double dot = (p - v).dot(w - v);
        double t = std::clamp(dot / l2, 0., 1.);
        xlPointD projection(v + t * (w - v));
        return (p - projection).dot(p - projection);
    }

    double calcPercentage(double v, double s, double e)
    {
        return (v - s) / (e - s);
    }

    const double HIT_TEST_DIST_SQR_LIMIT = 0.0075 * 0.0075;

}


void SketchLine::DrawEntireSegment(SketchPathData& pathData, int w, int h) const
{
    float fromX = (float)(w * m_fromPt.x);
    float fromY = (float)(h * m_fromPt.y);
    float toX = (float)(w * m_toPt.x);
    float toY = (float)(h * m_toPt.y);
    pathData.lineTo(fromX, fromY, toX, toY);
}

void SketchLine::DrawPartialSegment(SketchPathData& pathData, int w, int h,
                                     std::optional<double> startPercentage, double endPercentage) const
{
    if (!startPercentage.has_value()) {
        if (endPercentage <= 0. || endPercentage > 1.)
            return;
        auto pt = (1 - endPercentage) * m_fromPt + endPercentage * m_toPt;
        pathData.lineTo((float)(w * m_fromPt.x), (float)(h * m_fromPt.y),
                        (float)(w * pt.x), (float)(h * pt.y));
    } else {
        double percentage = std::clamp(startPercentage.value(), 0., 1.);
        endPercentage = std::clamp(endPercentage, 0., 1.);
        if (percentage == endPercentage)
            return;

        auto endPt = (1 - endPercentage) * m_fromPt + endPercentage * m_toPt;
        auto startPt = (1 - percentage) * m_fromPt + percentage * m_toPt;

        pathData.moveTo((float)(w * startPt.x), (float)(h * startPt.y));
        pathData.lineTo((float)(w * startPt.x), (float)(h * startPt.y),
                        (float)(w * endPt.x), (float)(h * endPt.y));
    }
}

bool SketchLine::HitTest(const xlPointD& pt) const
{
    double distSqr = minDistSqrPointToLineSegment(m_fromPt, m_toPt, pt);
    return distSqr <= HIT_TEST_DIST_SQR_LIMIT;
}

void SketchLine::ReverseSegment()
{
    xlPointD tmp = m_fromPt;
    m_fromPt = m_toPt;
    m_toPt = tmp;
}


double SketchQuadraticBezier::Length() const
{
    return bezierLength(m_fromPt, m_cp, m_toPt);
}

void SketchQuadraticBezier::DrawEntireSegment(SketchPathData& pathData, int w, int h) const
{
    // Quadratic→Cubic degree elevation: cp1 = (P0+2*CP)/3, cp2 = (2*CP+P1)/3
    float p0x = (float)(w * m_fromPt.x), p0y = (float)(h * m_fromPt.y);
    float cpx = (float)(w * m_cp.x),     cpy = (float)(h * m_cp.y);
    float p1x = (float)(w * m_toPt.x),   p1y = (float)(h * m_toPt.y);
    pathData.cubicTo((p0x + 2.0f * cpx) / 3.0f, (p0y + 2.0f * cpy) / 3.0f,
                     (2.0f * cpx + p1x) / 3.0f, (2.0f * cpy + p1y) / 3.0f,
                     p1x, p1y);
}

void SketchQuadraticBezier::DrawPartialSegment(SketchPathData& pathData, int w, int h,
                                                std::optional<double> startPercentage, double endPercentage) const
{
    if (!startPercentage.has_value()) {
        auto approxPts = piecewiseLinearApproximation(m_fromPt, m_cp, m_toPt);
        int i = 0;
        double increment = 1. / approxPts.size();
        float prevX = (float)(w * m_fromPt.x);
        float prevY = (float)(h * m_fromPt.y);
        for (double t = 0.; t <= endPercentage; t += increment, ++i) {
            if (i >= (int)approxPts.size()) break;
            float px = (float)(w * approxPts[i].x);
            float py = (float)(h * approxPts[i].y);
            pathData.lineTo(prevX, prevY, px, py);
            prevX = px;
            prevY = py;
        }
    } else {
        double percentage = std::clamp(startPercentage.value(), 0., 1.);
        endPercentage = std::clamp(endPercentage, 0., 1.);
        if (percentage == endPercentage)
            return;

        auto approxPts = piecewiseLinearApproximation(m_fromPt, m_cp, m_toPt);
        int i = 0;
        double increment = 1. / approxPts.size();
        bool started = false;
        float prevX = 0, prevY = 0;
        for (double t = 0.; t <= endPercentage; t += increment, ++i) {
            if (i >= (int)approxPts.size()) break;
            float px = (float)(w * approxPts[i].x);
            float py = (float)(h * approxPts[i].y);
            if (t >= startPercentage && !started) {
                pathData.moveTo(px, py);
                started = true;
                prevX = px;
                prevY = py;
            } else if (t >= startPercentage && t <= endPercentage) {
                pathData.lineTo(prevX, prevY, px, py);
                prevX = px;
                prevY = py;
            }
        }
    }
}

bool SketchQuadraticBezier::HitTest(const xlPointD& pt) const
{
    auto points = piecewiseLinearApproximation(m_fromPt, m_cp, m_toPt, 40);
    xlPointD startPt(points.front());
    for (size_t i = 1; i < points.size(); ++i) {
        double distSqr = minDistSqrPointToLineSegment(startPt, points[i], pt);
        if (distSqr < HIT_TEST_DIST_SQR_LIMIT)
            return true;
        startPt = points[i];
    }
    return false;
}

void SketchQuadraticBezier::ReverseSegment()
{
    xlPointD tmp = m_fromPt;
    m_fromPt = m_toPt;
    m_toPt = tmp;
}


double SketchCubicBezier::Length() const
{
    return bezierLength(m_fromPt, m_cp1, m_cp2, m_toPt);
}

void SketchCubicBezier::DrawEntireSegment(SketchPathData& pathData, int w, int h) const
{
    pathData.cubicTo((float)(w * m_cp1.x), (float)(h * m_cp1.y),
                     (float)(w * m_cp2.x), (float)(h * m_cp2.y),
                     (float)(w * m_toPt.x), (float)(h * m_toPt.y));
}

void SketchCubicBezier::DrawPartialSegment(SketchPathData& pathData, int w, int h,
                                            std::optional<double> startPercentage, double endPercentage) const
{
    if (!startPercentage.has_value()) {
        auto approxPts = piecewiseLinearApproximation(m_fromPt, m_cp1, m_cp2, m_toPt);
        int i = 0;
        double increment = 1. / approxPts.size();
        float prevX = (float)(w * m_fromPt.x);
        float prevY = (float)(h * m_fromPt.y);
        for (double t = 0.; t <= endPercentage; t += increment, ++i) {
            if (i >= (int)approxPts.size()) break;
            float px = (float)(w * approxPts[i].x);
            float py = (float)(h * approxPts[i].y);
            pathData.lineTo(prevX, prevY, px, py);
            prevX = px;
            prevY = py;
        }
    } else {
        double percentage = std::clamp(startPercentage.value(), 0., 1.);
        endPercentage = std::clamp(endPercentage, 0., 1.);
        if (percentage == endPercentage)
            return;

        auto approxPts = piecewiseLinearApproximation(m_fromPt, m_cp1, m_cp2, m_toPt);
        int i = 0;
        double increment = 1. / approxPts.size();
        bool started = false;
        float prevX = 0, prevY = 0;
        for (double t = 0.; t <= endPercentage; t += increment, ++i) {
            if (i >= (int)approxPts.size()) break;
            float px = (float)(w * approxPts[i].x);
            float py = (float)(h * approxPts[i].y);
            if (t >= startPercentage && !started) {
                pathData.moveTo(px, py);
                started = true;
                prevX = px;
                prevY = py;
            } else if (t >= startPercentage && t <= endPercentage) {
                pathData.lineTo(prevX, prevY, px, py);
                prevX = px;
                prevY = py;
            }
        }
    }
}

bool SketchCubicBezier::HitTest(const xlPointD& pt) const
{
    auto points = piecewiseLinearApproximation(m_fromPt, m_cp1, m_cp2, m_toPt, 40);
    xlPointD startPt(points.front());
    for (size_t i = 1; i < points.size(); ++i) {
        double distSqr = minDistSqrPointToLineSegment(startPt, points[i], pt);
        if (distSqr < HIT_TEST_DIST_SQR_LIMIT)
            return true;
        startPt = points[i];
    }
    return false;
}

void SketchCubicBezier::ReverseSegment()
{
    xlPointD tmp = m_fromPt;
    m_fromPt = m_toPt;
    m_toPt = tmp;

    tmp = m_cp1;
    m_cp1 = m_cp2;
    m_cp2 = tmp;
}


double SketchEffectPath::Length() const
{
    if (m_segments.empty())
        return 0.;

    double len = 0.0;
    for (const auto& segment : m_segments)
        len += segment->Length();
    return len;
}

void SketchEffectPath::appendSegment(std::shared_ptr<SketchPathSegment> segment)
{
    m_segments.push_back(segment);
}

void SketchEffectPath::collectEntirePath(SketchPathData& pathData, int w, int h) const
{
    if (m_segments.empty())
        return;
    auto startPt(m_segments.front()->StartPoint());
    pathData.moveTo((float)(w * startPt.x), (float)(h * startPt.y));

    for (const auto& segment : m_segments)
        segment->DrawEntireSegment(pathData, w, h);
}

void SketchEffectPath::collectPartialPath(SketchPathData& pathData, int w, int h,
                                           std::optional<double> startPercentage, double endPercentage) const
{
    if (m_segments.empty())
        return;
    double totalLength = Length();
    if (totalLength == 0.)
        return;
    double cumulativeLength = 0.;

    if (!startPercentage.has_value()) {
        // No startPercentage: emit initial moveTo, then draw from start to endPercentage
        auto startPt(m_segments.front()->StartPoint());
        pathData.moveTo((float)(w * startPt.x), (float)(h * startPt.y));

        for (const auto& segment : m_segments) {
            double length = segment->Length();
            double percentageAtEndOfSegment = (cumulativeLength + length) / totalLength;
            if (endPercentage > percentageAtEndOfSegment) {
                segment->DrawEntireSegment(pathData, w, h);
            } else {
                double percentageAtStartOfSegment = cumulativeLength / totalLength;
                if (endPercentage >= percentageAtStartOfSegment && endPercentage < percentageAtEndOfSegment) {
                    double segmentPercentage = calcPercentage(endPercentage, percentageAtStartOfSegment, percentageAtEndOfSegment);
                    segment->DrawPartialSegment(pathData, w, h, std::nullopt, segmentPercentage);
                }
            }
            cumulativeLength += length;
        }
    } else {
        // With startPercentage: each segment's DrawPartialSegment emits its own moveTo
        for (const auto& segment : m_segments) {
            double length = segment->Length();
            if (length != 0.0) {
                double percentageAtStartOfSegment = cumulativeLength / totalLength;
                double percentageAtEndOfSegment = (cumulativeLength + length) / totalLength;

                double segmentPercentage = calcPercentage(endPercentage, percentageAtStartOfSegment, percentageAtEndOfSegment);
                double segmentDrawPercentage = calcPercentage(startPercentage.value(), percentageAtStartOfSegment, percentageAtEndOfSegment);

                segment->DrawPartialSegment(pathData, w, h, segmentDrawPercentage, segmentPercentage);

                cumulativeLength += length;
            }
        }
    }
}

void SketchEffectPath::closePath(bool updateSegments, SketchCanvasPathState state)
{
    if (!m_isClosed && m_segments.size() >= 1) {
        if( updateSegments ) {
            xlPointD startPt( m_segments.back()->EndPoint() );
            xlPointD endPt( m_segments.front()->StartPoint() );

            if( state == SketchCanvasPathState::LineToNewPoint ) {
                m_segments.push_back(std::make_shared <SketchLine>(startPt, endPt));
            } else if( state == SketchCanvasPathState::QuadraticCurveToNewPoint ) {
                xlPointD cp = 0.5 * startPt + 0.5 * endPt;
                m_segments.push_back(std::make_shared <SketchQuadraticBezier>(startPt, cp, endPt));
            } else if( state == SketchCanvasPathState::CubicCurveToNewPoint ) {
                xlPointD cp1 = 0.75 * startPt + 0.25 * endPt;
                xlPointD cp2 = 0.25 * startPt + 0.75 * endPt;
                m_segments.push_back(std::make_shared <SketchCubicBezier>(startPt, cp1, cp2, endPt));
            }
        }

        m_isClosed = true;
        m_closedState = state;
    }
}

void SketchEffectPath::reversePath()
{
    for (const auto& segment : m_segments)
        segment->ReverseSegment();

    std::reverse(m_segments.begin(), m_segments.end());
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
    //  * within a path, only three "commands" (in SVG syntax) are supported currently:
    //      - 'L' for line segments
    //      - 'Q' for quadratic-curve segments
    //      - 'C' for cubic-curve segments
    //      - 'c' close the path with a line to the start point (must be the last command)
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
        xlPointD prevPt;

        std::string path_str((*iter).str());
        auto pathComponents_begin = sregex_iterator(path_str.cbegin(), path_str.cend(), pathComponentsRegex);
        try {
            for (sregex_iterator iter2 = pathComponents_begin; iter2 != sregex_iterator(); ++iter2) {
                std::string pathComponents_str((*iter2).str());
                if (iter2 == pathComponents_begin) {
                    auto startPt_begin = sregex_iterator(pathComponents_str.cbegin(), pathComponents_str.cend(), pathStartRegex);
                    if (std::distance(startPt_begin, sregex_iterator()) == 1 && (*startPt_begin).size() == 3) {
                        prevPt.x = std::strtod((*startPt_begin)[1].str().c_str(), nullptr);
                        prevPt.y = std::strtod((*startPt_begin)[2].str().c_str(), nullptr);
                    }
                } else if (pathComponents_str.at(0) == 'L') {
                    xlPointD toPt;
                    auto line_begin = sregex_iterator(pathComponents_str.cbegin(), pathComponents_str.cend(), lineRegex);
                    if (std::distance(line_begin, sregex_iterator()) == 1 && (*line_begin).size() == 3) {
                        toPt.x = std::strtod((*line_begin)[1].str().c_str(), nullptr);
                        toPt.y = std::strtod((*line_begin)[2].str().c_str(), nullptr);
                        path->appendSegment(std::make_shared<SketchLine>(prevPt, toPt));
                        prevPt = toPt;
                    }
                } else if (pathComponents_str.at(0) == 'Q') {
                    xlPointD ctrlPt, toPt;
                    auto curve_begin = sregex_iterator(pathComponents_str.cbegin(), pathComponents_str.cend(), quadraticRegex);
                    if (std::distance(curve_begin, sregex_iterator()) == 1 && (*curve_begin).size() == 5) {
                        ctrlPt.x = std::strtod((*curve_begin)[1].str().c_str(), nullptr);
                        ctrlPt.y = std::strtod((*curve_begin)[2].str().c_str(), nullptr);
                        toPt.x = std::strtod((*curve_begin)[3].str().c_str(), nullptr);
                        toPt.y = std::strtod((*curve_begin)[4].str().c_str(), nullptr);
                        path->appendSegment(std::make_shared<SketchQuadraticBezier>(prevPt, ctrlPt, toPt));
                        prevPt = toPt;
                    }

                } else if (pathComponents_str.at(0) == 'C') {
                    xlPointD ctrlPt1, ctrlPt2, toPt;
                    auto curve_begin = sregex_iterator(pathComponents_str.cbegin(), pathComponents_str.cend(), cubicRegex);
                    if (std::distance(curve_begin, sregex_iterator()) == 1 && (*curve_begin).size() == 7) {
                        ctrlPt1.x = std::strtod((*curve_begin)[1].str().c_str(), nullptr);
                        ctrlPt1.y = std::strtod((*curve_begin)[2].str().c_str(), nullptr);
                        ctrlPt2.x = std::strtod((*curve_begin)[3].str().c_str(), nullptr);
                        ctrlPt2.y = std::strtod((*curve_begin)[4].str().c_str(), nullptr);
                        toPt.x = std::strtod((*curve_begin)[5].str().c_str(), nullptr);
                        toPt.y = std::strtod((*curve_begin)[6].str().c_str(), nullptr);
                        path->appendSegment(std::make_shared<SketchCubicBezier>(prevPt, ctrlPt1, ctrlPt2, toPt));
                        prevPt = toPt;
                    }
                } else if (pathComponents_str.at(0) == 'c') {
                    if( pathComponents_str.size() == 2) {
                        int type = pathComponents_str.at(1) - '0';
                        path->closePath(false, SketchCanvasPathState(type));
                    } else {
                        path->closePath(false, SketchCanvasPathState::LineToNewPoint);
                    }
                }
            }
        } catch (...) {

            spdlog::error("Error parsing sketch path : \"{}\"", path_str.c_str());
        }

        if (!path->segments().empty())
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
        stream << startPt.x << ',' << startPt.y << ';';
        for (size_t ii = 0; ii < segments.size(); ++ii) {
            std::shared_ptr<SketchCubicBezier> cubic;
            std::shared_ptr<SketchQuadraticBezier> quadratic;

            if (std::dynamic_pointer_cast<SketchLine>(segments[ii]) != nullptr) {
                auto endPt(segments[ii]->EndPoint());
                stream << 'L' << endPt.x << ',' << endPt.y << ';';
            } else if ((quadratic = std::dynamic_pointer_cast<SketchQuadraticBezier>(segments[ii]))) {
                auto ctrlPt(quadratic->ControlPoint());
                auto endPt(quadratic->EndPoint());
                stream << 'Q' << ctrlPt.x << ',' << ctrlPt.y
                       << ',' << endPt.x << ',' << endPt.y << ';';
            } else if ((cubic = std::dynamic_pointer_cast<SketchCubicBezier>(segments[ii]))) {
                auto ctrlPt1(cubic->ControlPoint1());
                auto ctrlPt2(cubic->ControlPoint2());
                auto endPt(cubic->EndPoint());
                stream << 'C' << ctrlPt1.x << ',' << ctrlPt1.y << ','
                       << ctrlPt2.x << ',' << ctrlPt2.y << ','
                       << endPt.x << ',' << endPt.y << ';';
            }
        }
        if (path->isClosed())
            stream << 'c' << static_cast<int>(path->GetClosedState());
        if (i != m_paths.size() - 1)
            stream << '|';
    }
    return stream.str();
}

void SketchEffectSketch::updatePath(int index, std::shared_ptr<SketchEffectPath> path)
{
    if (index < 0 || index >= (int)m_paths.size())
        return;
    m_paths[index] = path;
}

void SketchEffectSketch::reversePath(int pathIndex)
{
    if (pathIndex < 0 || pathIndex >= (int)m_paths.size())
        return;

    m_paths[pathIndex]->reversePath();
}

void SketchEffectSketch::deletePath(int pathIndex)
{
    if (pathIndex < 0 || pathIndex >= (int)m_paths.size())
        return;

    auto iter = m_paths.begin();
    std::advance(iter, pathIndex);

    m_paths.erase(iter);
}

void SketchEffectSketch::swapPaths(int pathIndex0, int pathIndex1)
{
    if (pathIndex0 < 0 || pathIndex1 < 0 || pathIndex0 >= (int)m_paths.size() || pathIndex1 >= (int)m_paths.size())
        return;

    std::swap(m_paths[pathIndex0], m_paths[pathIndex1]);
}

double SketchEffectSketch::getLength()
{
    double totalLength = 0.;
    for (const auto& path : m_paths)
        totalLength += path->Length();
    return totalLength;
}

void SketchEffectSketch::getProgressPosition( double progress, double& x, double& y )
{
    double totalLength = 0.;
    for (const auto& path : m_paths)
        totalLength += path->Length();
    double targetLength = progress * totalLength;

    for (const auto& path : m_paths)
    {
        if( path->Length() >= targetLength ) {
            for(const auto& segment : path->segments() ) {
                if( segment->Length() >= targetLength ) {
                    segment->getProgressPosition( targetLength, x, y );
                    return;
                } else {
                    targetLength -= segment->Length();
                }
            }
        } else {
            targetLength -= path->Length();
        }
    }
}

void SketchLine::getProgressPosition( double partialLength, double& x, double& y )
{
    double percent = partialLength / Length();
    x = m_fromPt.x + percent * (m_toPt.x - m_fromPt.x);
    y = m_fromPt.y + percent * (m_toPt.y - m_fromPt.y);
}

void SketchQuadraticBezier::getProgressPosition( double partialLength, double& x, double& y )
{
    bezierProgressPosition(m_fromPt, m_cp, m_toPt,partialLength, x, y);
}

void SketchCubicBezier::getProgressPosition( double partialLength, double& x, double& y )
{
    bezierProgressPosition(m_fromPt, m_cp1, m_cp2, m_toPt, partialLength, x, y);
}
