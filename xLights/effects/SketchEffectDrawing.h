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

    virtual double Length() const = 0;
    virtual wxPoint2DDouble StartPoint() const = 0;
    virtual void SetStartPoint(const wxPoint2DDouble& pt) = 0;
    virtual wxPoint2DDouble EndPoint() const = 0;
    virtual void SetEndPoint(const wxPoint2DDouble& pt) = 0;
    virtual void DrawEntireSegment(wxGraphicsPath& path, const wxSize& sz) const = 0;
    virtual void DrawPartialSegment(wxGraphicsPath& path, const wxSize& sz,
                                    std::optional<double> startPercentage,
                                    double endPercentage) const = 0;
    virtual bool HitTest(const wxPoint2DDouble& pt) const = 0;
};

// A path is simply a collection of segments. If there are at least two segments,
// the path can be closed to potentially represent a polygon
class SketchEffectPath
{
public:
    virtual ~SketchEffectPath() = default;

    double Length() const;

    void appendSegment(std::shared_ptr<SketchPathSegment> cmd);
    void closePath();
    void drawEntirePath(wxGraphicsContext* gc, const wxSize& sz) const;
    void drawPartialPath(wxGraphicsContext* gc, const wxSize& sz,
                         std::optional<double> startPercentage,
                         double endPercentage) const;

    const std::vector<std::shared_ptr<SketchPathSegment>>& segments() const
    {
        return m_segments;
    }

    bool isClosed() const
    {
        return m_isClosed;
    }

protected:
    std::vector<std::shared_ptr<SketchPathSegment>> m_segments;
    bool m_isClosed = false;
};

// A sketch is a collection of paths... just a thin std::vector wrapper currently
class SketchEffectSketch
{
public:
    static std::string DefaultSketchString();
    static SketchEffectSketch DefaultSketch();

    static SketchEffectSketch SketchFromString(const std::string& sketchDef);

    std::string toString() const;

    virtual ~SketchEffectSketch() = default;

    std::vector<std::shared_ptr<SketchEffectPath>>& paths()
    {
        return m_paths;
    }
    const std::vector<std::shared_ptr<SketchEffectPath>>& paths() const
    {
        return m_paths;
    }
    size_t pathCount() const
    {
        return m_paths.size();
    }

    void appendPath(std::shared_ptr<SketchEffectPath> path)
    {
        m_paths.push_back(path);
    }
    void updatePath(int index, std::shared_ptr<SketchEffectPath> path);
    void deletePath(int pathIndex);

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
    double Length() const override
    {
        return m_fromPt.GetDistance(m_toPt);
    }
    wxPoint2DDouble StartPoint() const override
    {
        return m_fromPt;
    }
    void SetStartPoint(const wxPoint2DDouble& pt) override
    {
        m_fromPt = pt;
    }
    wxPoint2DDouble EndPoint() const override
    {
        return m_toPt;
    }
    void SetEndPoint(const wxPoint2DDouble& pt) override
    {
        m_toPt = pt;
    }
    void DrawEntireSegment(wxGraphicsPath& path, const wxSize& sz) const override;
    void DrawPartialSegment(wxGraphicsPath& path, const wxSize& sz,
                            std::optional<double> startPercentage,
                            double endPercentage) const override;
    bool HitTest(const wxPoint2DDouble& pt) const override;

protected:
    wxPoint2DDouble m_fromPt;
    wxPoint2DDouble m_toPt;
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
    double Length() const override;
    wxPoint2DDouble StartPoint() const override
    {
        return m_fromPt;
    }
    void SetStartPoint(const wxPoint2DDouble& pt) override
    {
        m_fromPt = pt;
    }
    wxPoint2DDouble EndPoint() const override
    {
        return m_toPt;
    }
    void SetEndPoint(const wxPoint2DDouble& pt) override
    {
        m_toPt = pt;
    }
    void DrawEntireSegment(wxGraphicsPath& path, const wxSize& sz) const override;
    void DrawPartialSegment(wxGraphicsPath& path, const wxSize& sz,
                            std::optional<double> startPercentage,
                            double endPercentage) const override;
    bool HitTest(const wxPoint2DDouble& pt) const override;

    wxPoint2DDouble ControlPoint() const
    {
        return m_cp;
    }
    void SetControlPoint(const wxPoint2DDouble& pt)
    {
        m_cp = pt;
    }

protected:
    wxPoint2DDouble m_fromPt;
    wxPoint2DDouble m_cp;
    wxPoint2DDouble m_toPt;
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
    double Length() const override;
    wxPoint2DDouble StartPoint() const override
    {
        return m_fromPt;
    }
    void SetStartPoint(const wxPoint2DDouble& pt) override
    {
        m_fromPt = pt;
    }
    wxPoint2DDouble EndPoint() const override
    {
        return m_toPt;
    }
    void SetEndPoint(const wxPoint2DDouble& pt) override
    {
        m_toPt = pt;
    }
    void DrawEntireSegment(wxGraphicsPath& path, const wxSize& sz) const override;
    void DrawPartialSegment(wxGraphicsPath& path, const wxSize& sz,
                            std::optional<double> startPercentage,
                            double endPercentage) const override;
    bool HitTest(const wxPoint2DDouble& pt) const override;

    wxPoint2DDouble ControlPoint1() const
    {
        return m_cp1;
    }
    void SetControlPoint1(const wxPoint2DDouble& pt)
    {
        m_cp1 = pt;
    }
    wxPoint2DDouble ControlPoint2() const
    {
        return m_cp2;
    }
    void SetControlPoint2(const wxPoint2DDouble& pt)
    {
        m_cp2 = pt;
    }

protected:
    wxPoint2DDouble m_fromPt;
    wxPoint2DDouble m_cp1;
    wxPoint2DDouble m_cp2;
    wxPoint2DDouble m_toPt;
};
