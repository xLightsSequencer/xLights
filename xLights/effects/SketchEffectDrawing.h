#pragma once

#include <wx/geometry.h>

#include <memory>
#include <vector>

class wxGraphicsContext;
class wxGraphicsPath;

// A segment can currently be either a line segment (start and end point) or a
// cubic-bezier segment (start and end point with two control points)
class SketchPathSegment
{
public:
    virtual ~SketchPathSegment() = default;

    virtual double Length() = 0;
    virtual wxPoint2DDouble StartPoint() = 0;
    virtual wxPoint2DDouble EndPoint() = 0;
    virtual void DrawEntireSegment(wxGraphicsPath& path) = 0;
    virtual void DrawPartialSegment(wxGraphicsPath& path, double percentage) = 0;
};

// A path is simply a collection of segments. If there are at least two segments,
// the path can be closed to potentially represent a polygon
class SketchEffectPath
{
public:
    SketchEffectPath(const wxPoint2DDouble& startPt = wxPoint2DDouble(), double startDelay = 0.) :
        m_startPt(startPt),
        m_startDelay(startDelay)
    {}
    virtual ~SketchEffectPath() = default;

    const wxPoint2DDouble& StartPt() const
    {
        return m_startPt;
    }
    double Length();

    void appendSegment(std::shared_ptr<SketchPathSegment> cmd);
    void closePath();
    void drawEntirePath(wxGraphicsContext* gc);
    void drawPartialPath(wxGraphicsContext* gc, double percentage);

protected:
    const wxPoint2DDouble m_startPt;
    double m_startDelay = 0.;
    std::vector<std::shared_ptr<SketchPathSegment>> m_segments;
    bool m_closePath = false;
};

// A sketch is a collection of paths along with a canvas size
// Currently it is assumed that none of the paths will exceed
// the bounds of the canvas
class SketchEffectSketch
{
public:
    static SketchEffectSketch DefaultSketch();

    SketchEffectSketch(int width = 0, int height = 0) :
        m_width(width),
        m_height(height)
    {}
    virtual ~SketchEffectSketch() = default;

    int width() const
    {
        return m_width;
    }
    int height() const
    {
        return m_height;
    }

    const std::vector<std::shared_ptr<SketchEffectPath>> paths() const
    {
        return m_paths;
    }

    void appendPath(std::shared_ptr<SketchEffectPath> path)
    {
        m_paths.push_back(path);
    }

protected:
    int m_width = 0;
    int m_height = 0;
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
    void DrawEntireSegment(wxGraphicsPath& path) override;
    void DrawPartialSegment(wxGraphicsPath& path, double percentage) override;

protected:
    const wxPoint2DDouble m_fromPt;
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
    void DrawEntireSegment(wxGraphicsPath& path) override;
    void DrawPartialSegment(wxGraphicsPath& path, double percentage) override;

protected:
    const wxPoint2DDouble m_fromPt;
    const wxPoint2DDouble m_cp1;
    const wxPoint2DDouble m_cp2;
    const wxPoint2DDouble m_toPt;
};
