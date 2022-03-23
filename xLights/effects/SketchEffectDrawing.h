#pragma once

#include <wx/geometry.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

class wxGraphicsContext;
class wxGraphicsPath;

// A segment can currently be either a line segment (start and end point)
// or a quadratic-bezier segment (start and end point with one control point)
// or a cubic-bezier segment (start and end point with two control points).
//
// Coordinates are normalized to [0,1] and scaled for drawing in the Draw*() methods
class SketchPathSegment
{
public:
    virtual ~SketchPathSegment() = default;

    virtual double Length() = 0;
    virtual wxPoint2DDouble StartPoint() = 0;
    virtual wxPoint2DDouble EndPoint() = 0;
    virtual void DrawEntireSegment(wxGraphicsPath& path, const wxSize& sz) = 0;
    virtual void DrawPartialSegment(wxGraphicsPath& path, const wxSize& sz,
                                    std::optional<double> startPercentage,
                                    double endPercentage) = 0;
};

// A path is simply a collection of segments. If there are at least two segments,
// the path can be closed to potentially represent a polygon
class SketchEffectPath
{
public:
    SketchEffectPath(double startDelay = 0.) :
        m_startDelay(startDelay)
    {}
    virtual ~SketchEffectPath() = default;

    double Length();

    void appendSegment(std::shared_ptr<SketchPathSegment> cmd);
    void closePath();
    void drawEntirePath(wxGraphicsContext* gc, const wxSize& sz);
    void drawPartialPath(wxGraphicsContext* gc, const wxSize& sz,
                         std::optional<double> startPercentage,
                         double endPercentage);
protected:
    double m_startDelay = 0.;
    std::vector<std::shared_ptr<SketchPathSegment>> m_segments;
    bool m_closePath = false;
};

// A sketch is a collection of paths... just a thin std::vector wrapper currently
class SketchEffectSketch
{
public:
    static SketchEffectSketch DefaultSketch();
    static SketchEffectSketch SketchFromString(const std::string& sketchDef);

    virtual ~SketchEffectSketch() = default;

    const std::vector<std::shared_ptr<SketchEffectPath>> paths() const
    {
        return m_paths;
    }

    void appendPath(std::shared_ptr<SketchEffectPath> path)
    {
        m_paths.push_back(path);
    }

protected:
    std::vector<std::shared_ptr<SketchEffectPath>> m_paths;
};

class SketchLine : public SketchPathSegment
{
public:
    SketchLine(const wxPoint2DDouble& fromPt, const wxPoint2DDouble& toPt) :
        m_fromPt(fromPt),
        m_toPt(toPt)
    {}
    double Length() override
    {
        return m_fromPt.GetDistance(m_toPt);
    }
    wxPoint2DDouble StartPoint() override
    {
        return m_fromPt;
    }
    wxPoint2DDouble EndPoint() override
    {
        return m_toPt;
    }
    void DrawEntireSegment(wxGraphicsPath& path, const wxSize& sz) override;
    void DrawPartialSegment(wxGraphicsPath& path, const wxSize& sz,
                            std::optional<double> startPercentage,
                            double endPercentage) override;

protected:
    const wxPoint2DDouble m_fromPt;
    const wxPoint2DDouble m_toPt;
};

class SketchQuadraticBezier : public SketchPathSegment
{
public:
    SketchQuadraticBezier(const wxPoint2DDouble& fromPt,
                          const wxPoint2DDouble& cp,
                          const wxPoint2DDouble& toPt) :
        m_fromPt(fromPt),
        m_cp(cp),
        m_toPt(toPt)
    {
    }
    double Length() override;
    wxPoint2DDouble StartPoint() override
    {
        return m_fromPt;
    }
    wxPoint2DDouble EndPoint() override
    {
        return m_toPt;
    }
    void DrawEntireSegment(wxGraphicsPath& path, const wxSize& sz) override;
    void DrawPartialSegment(wxGraphicsPath& path, const wxSize& sz,
                            std::optional<double> startPercentage,
                            double endPercentage) override;

protected:
    const wxPoint2DDouble m_fromPt;
    const wxPoint2DDouble m_cp;
    const wxPoint2DDouble m_toPt;
};

class SketchCubicBezier : public SketchPathSegment
{
public:
    SketchCubicBezier(const wxPoint2DDouble& fromPt,
                      const wxPoint2DDouble& cp1,
                      const wxPoint2DDouble& cp2,
                      const wxPoint2DDouble& toPt) :
        m_fromPt(fromPt),
        m_cp1(cp1),
        m_cp2(cp2),
        m_toPt(toPt)
    {
    }
    double Length() override;
    wxPoint2DDouble StartPoint() override
    {
        return m_fromPt;
    }
    wxPoint2DDouble EndPoint() override
    {
        return m_toPt;
    }
    void DrawEntireSegment(wxGraphicsPath& path, const wxSize& sz) override;
    void DrawPartialSegment(wxGraphicsPath& path, const wxSize& sz,
                            std::optional<double> startPercentage,
                            double endPercentage) override;

protected:
    const wxPoint2DDouble m_fromPt;
    const wxPoint2DDouble m_cp1;
    const wxPoint2DDouble m_cp2;
    const wxPoint2DDouble m_toPt;
};
