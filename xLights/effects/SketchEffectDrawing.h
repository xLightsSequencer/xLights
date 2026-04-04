#pragma once

#include "../utils/xlPoint.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../ui/effectpanels/assist/SketchCanvasPanel.h"

// A single sub-path: a starting point followed by cubic bezier segments.
// cubics stores groups of 6 floats: [cpx1,cpy1, cpx2,cpy2, x1,y1, ...]
struct SketchSubPath {
    float startX = 0;
    float startY = 0;
    std::vector<float> cubics;
};

// Collection of sub-paths emitted by Draw methods.
struct SketchPathData {
    std::vector<SketchSubPath> subPaths;

    void moveTo(float x, float y) {
        subPaths.push_back({x, y, {}});
    }
    void cubicTo(float cp1x, float cp1y, float cp2x, float cp2y, float ex, float ey) {
        if (subPaths.empty()) subPaths.push_back({0, 0, {}});
        auto& c = subPaths.back().cubics;
        c.push_back(cp1x); c.push_back(cp1y);
        c.push_back(cp2x); c.push_back(cp2y);
        c.push_back(ex);   c.push_back(ey);
    }
    // Emit a line as a degenerate cubic (cp1=from, cp2=to)
    void lineTo(float fromX, float fromY, float toX, float toY) {
        cubicTo(fromX, fromY, toX, toY, toX, toY);
    }
    void append(const SketchPathData& other) {
        subPaths.insert(subPaths.end(), other.subPaths.begin(), other.subPaths.end());
    }
    bool empty() const {
        return subPaths.empty();
    }
};

// A segment can currently be either a line segment (start and end point)
// or a quadratic-bezier segment (start and end point with one control point)
// or a cubic-bezier segment (start and end point with two control points).
//
// Coordinates are normalized to [0,1] and scaled for drawing in the Draw*() methods.
// Draw methods emit cubic bezier data into a SketchPathData structure.
class SketchPathSegment
{
public:
    virtual ~SketchPathSegment() = default;

    [[nodiscard]] virtual double Length() const = 0;
    [[nodiscard]] virtual xlPointD StartPoint() const = 0;
    virtual void SetStartPoint(const xlPointD& pt) = 0;
    [[nodiscard]] virtual xlPointD EndPoint() const = 0;
    virtual void SetEndPoint(const xlPointD& pt) = 0;
    virtual void DrawEntireSegment(SketchPathData& pathData, int w, int h) const = 0;
    virtual void DrawPartialSegment(SketchPathData& pathData, int w, int h,
                                    std::optional<double> startPercentage,
                                    double endPercentage) const = 0;
    [[nodiscard]] virtual bool HitTest(const xlPointD& pt) const = 0;
    virtual void ReverseSegment() = 0;
    virtual void getProgressPosition( double partialLength, double& x, double& y ) = 0;
};

// A path is simply a collection of segments. If there are at least two segments,
// the path can be closed to potentially represent a polygon.
// collectEntirePath/collectPartialPath emit cubic bezier points in nanosvg format
// into a float vector. The first two floats are the initial moveTo point.
class SketchEffectPath
{
public:
    virtual ~SketchEffectPath() = default;

    [[nodiscard]] double Length() const;

    void appendSegment(std::shared_ptr<SketchPathSegment> cmd);
    void closePath(bool updateSegments, SketchCanvasPathState state);
    void collectEntirePath(SketchPathData& pathData, int w, int h) const;
    void collectPartialPath(SketchPathData& pathData, int w, int h,
                         std::optional<double> startPercentage,
                         double endPercentage) const;
    void reversePath();

    [[nodiscard]] const std::vector<std::shared_ptr<SketchPathSegment>>& segments() const
    {
        return m_segments;
    }

    [[nodiscard]] bool isClosed() const { return m_isClosed; }
    [[nodiscard]] SketchCanvasPathState GetClosedState() const { return m_closedState; }

protected:
    std::vector<std::shared_ptr<SketchPathSegment>> m_segments;
    bool m_isClosed {false};
    SketchCanvasPathState  m_closedState {SketchCanvasPathState::Undefined};
};

// A sketch is a collection of paths... just a thin std::vector wrapper currently
class SketchEffectSketch
{
public:
    [[nodiscard]] static std::string DefaultSketchString();
    [[nodiscard]] static SketchEffectSketch DefaultSketch();

    [[nodiscard]] static SketchEffectSketch SketchFromString(const std::string& sketchDef);

    [[nodiscard]] std::string toString() const;

    virtual ~SketchEffectSketch() = default;

    [[nodiscard]] std::vector<std::shared_ptr<SketchEffectPath>>& paths()
    {
        return m_paths;
    }
    [[nodiscard]] const std::vector<std::shared_ptr<SketchEffectPath>>& paths() const
    {
        return m_paths;
    }
    [[nodiscard]] size_t pathCount() const
    {
        return m_paths.size();
    }

    void appendPath(std::shared_ptr<SketchEffectPath> path)
    {
        m_paths.push_back(path);
    }
    void updatePath(int index, std::shared_ptr<SketchEffectPath> path);
    void reversePath(int pathIndex);
    void deletePath(int pathIndex);
    void swapPaths(int pathIndex0, int pathIndex1);

    void getProgressPosition( double progress, double& x, double& y );
    [[nodiscard]] double getLength();

protected:
    std::vector<std::shared_ptr<SketchEffectPath>> m_paths;
};

class SketchLine : public SketchPathSegment
{
public:
    SketchLine(const xlPointD& fromPt, const xlPointD& toPt) :
        m_fromPt(fromPt),
        m_toPt(toPt)
    {}
    [[nodiscard]] double Length() const override
    {
        return m_fromPt.distance(m_toPt);
    }
    [[nodiscard]] xlPointD StartPoint() const override
    {
        return m_fromPt;
    }
    void SetStartPoint(const xlPointD& pt) override
    {
        m_fromPt = pt;
    }
    [[nodiscard]] xlPointD EndPoint() const override
    {
        return m_toPt;
    }
    void SetEndPoint(const xlPointD& pt) override
    {
        m_toPt = pt;
    }
    void DrawEntireSegment(SketchPathData& pathData, int w, int h) const override;
    void DrawPartialSegment(SketchPathData& pathData, int w, int h,
                            std::optional<double> startPercentage,
                            double endPercentage) const override;
    bool HitTest(const xlPointD& pt) const override;
    void ReverseSegment() override;
    void getProgressPosition( double partialLength, double& x, double& y ) override;

protected:
    xlPointD m_fromPt;
    xlPointD m_toPt;
};

class SketchQuadraticBezier : public SketchPathSegment
{
public:
    SketchQuadraticBezier(const xlPointD& fromPt,
                          const xlPointD& cp,
                          const xlPointD& toPt) :
        m_fromPt(fromPt),
        m_cp(cp),
        m_toPt(toPt)
    {
    }
    [[nodiscard]] double Length() const override;
    [[nodiscard]] xlPointD StartPoint() const override
    {
        return m_fromPt;
    }
    void SetStartPoint(const xlPointD& pt) override
    {
        m_fromPt = pt;
    }
    [[nodiscard]] xlPointD EndPoint() const override
    {
        return m_toPt;
    }
    void SetEndPoint(const xlPointD& pt) override
    {
        m_toPt = pt;
    }
    void DrawEntireSegment(SketchPathData& pathData, int w, int h) const override;
    void DrawPartialSegment(SketchPathData& pathData, int w, int h,
                            std::optional<double> startPercentage,
                            double endPercentage) const override;
    bool HitTest(const xlPointD& pt) const override;
    void ReverseSegment() override;
    void getProgressPosition( double partialLength, double& x, double& y ) override;

    [[nodiscard]] xlPointD ControlPoint() const
    {
        return m_cp;
    }
    void SetControlPoint(const xlPointD& pt)
    {
        m_cp = pt;
    }

protected:
    xlPointD m_fromPt;
    xlPointD m_cp;
    xlPointD m_toPt;
};

class SketchCubicBezier : public SketchPathSegment
{
public:
    SketchCubicBezier(const xlPointD& fromPt,
                      const xlPointD& cp1,
                      const xlPointD& cp2,
                      const xlPointD& toPt) :
        m_fromPt(fromPt),
        m_cp1(cp1),
        m_cp2(cp2),
        m_toPt(toPt)
    {
    }
    [[nodiscard]] double Length() const override;
    [[nodiscard]] xlPointD StartPoint() const override
    {
        return m_fromPt;
    }
    void SetStartPoint(const xlPointD& pt) override
    {
        m_fromPt = pt;
    }
    [[nodiscard]] xlPointD EndPoint() const override
    {
        return m_toPt;
    }
    void SetEndPoint(const xlPointD& pt) override
    {
        m_toPt = pt;
    }
    void DrawEntireSegment(SketchPathData& pathData, int w, int h) const override;
    void DrawPartialSegment(SketchPathData& pathData, int w, int h,
                            std::optional<double> startPercentage,
                            double endPercentage) const override;
    [[nodiscard]] bool HitTest(const xlPointD& pt) const override;
    void ReverseSegment() override;
    void getProgressPosition( double partialLength, double& x, double& y ) override;

    [[nodiscard]] xlPointD ControlPoint1() const
    {
        return m_cp1;
    }
    void SetControlPoint1(const xlPointD& pt)
    {
        m_cp1 = pt;
    }
    [[nodiscard]] xlPointD ControlPoint2() const
    {
        return m_cp2;
    }
    void SetControlPoint2(const xlPointD& pt)
    {
        m_cp2 = pt;
    }

protected:
    xlPointD m_fromPt;
    xlPointD m_cp1;
    xlPointD m_cp2;
    xlPointD m_toPt;
};
