#include "SketchEffect.h"

#include "RenderBuffer.h"
#include "SketchPanel.h"

#include "../../include/sketch-64.xpm"
#include "../../include/sketch-48.xpm"
#include "../../include/sketch-32.xpm"
#include "../../include/sketch-24.xpm"
#include "../../include/sketch-16.xpm"

#include <wx/geometry.h>
#include <wx/graphics.h>
#include <wx/image.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace
{
    double bezier(double t, double start, double control1, double control2, double end)
    {
        double u = 1 - t;
        return start * u * u * u + control1 * 3 * u * u * t + control2 * 3 * u * t * t + end * t * t * t;
    }

    wxPoint2DDouble bezierPoint(double t, const wxPoint2DDouble& startPt,
                                const wxPoint2DDouble& ctrlPt1,
                                const wxPoint2DDouble& ctrlPt2,
                                const wxPoint2DDouble& endPt )
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

        for (int i = 1; i <= NUM_STEPS; ++i)
        {
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

    class PathSegmentCmd
    {
    public:
        virtual double Length() = 0;
        virtual wxPoint2DDouble StartPoint() = 0;
        virtual wxPoint2DDouble EndPoint() = 0;
        virtual void DrawEntireSegment(wxGraphicsPath& path) = 0;
        virtual void DrawPartialSegment(wxGraphicsPath& path, double percentage) = 0;
    };

    class LineCmd : public PathSegmentCmd
    {
    public:
        LineCmd(const wxPoint2DDouble& fromPt, const wxPoint2DDouble& toPt) :
            m_fromPt(fromPt),
            m_toPt(toPt)
        {}
        double Length() override { return m_fromPt.GetDistance(m_toPt);}
        wxPoint2DDouble StartPoint() override { return m_fromPt; }
        wxPoint2DDouble EndPoint() override { return m_toPt; }
        void DrawEntireSegment(wxGraphicsPath& path) override;
        void DrawPartialSegment(wxGraphicsPath& path, double percentage) override;

    protected:
        const wxPoint2DDouble m_fromPt;
        const wxPoint2DDouble m_toPt;
    };

    void LineCmd::DrawEntireSegment(wxGraphicsPath& path)
    {
        path.AddLineToPoint(m_toPt);
    }

    void LineCmd::DrawPartialSegment(wxGraphicsPath& path, double percentage)
    {
        if ( percentage <= 0. || percentage > 1. )
            return;
        auto pt = (1 - percentage) * m_fromPt + percentage * m_toPt;
        path.AddLineToPoint(pt);
    }

    class CubicBezierCmd : public PathSegmentCmd
    {
    public:
        CubicBezierCmd(const wxPoint2DDouble& fromPt,
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
        wxPoint2DDouble StartPoint() override { return m_fromPt; }
        wxPoint2DDouble EndPoint() override { return m_toPt; }
        void DrawEntireSegment(wxGraphicsPath& path) override;
        void DrawPartialSegment(wxGraphicsPath& path, double percentage) override;

    protected:
        const wxPoint2DDouble m_fromPt;
        const wxPoint2DDouble m_cp1;
        const wxPoint2DDouble m_cp2;
        const wxPoint2DDouble m_toPt;
    };

    double CubicBezierCmd::Length()
    {
        return bezierLength(m_fromPt, m_cp1, m_cp2, m_toPt);
    }

    void CubicBezierCmd::DrawEntireSegment(wxGraphicsPath& path)
    {
        path.AddCurveToPoint(m_cp1, m_cp2, m_toPt);
    }

    void CubicBezierCmd::DrawPartialSegment(wxGraphicsPath& path, double percentage)
    {
        if (percentage <= 0. || percentage > 1.)
            return;
        auto pts = piecewiseLinearApproximation(m_fromPt, m_cp1, m_cp2, m_toPt);
        int i = 0;
        double increment = 1. / pts.size();
        for (double t = 0.; t <= percentage; t += increment, ++i)
            path.AddLineToPoint(pts[i]);
    }
}


class SketchEffectPath
{
public:
    SketchEffectPath(const wxPoint2DDouble& startPt = wxPoint2DDouble(), double startDelay = 0.) :
        m_startPt(startPt),
        m_startDelay(startDelay)
    {}

    const wxPoint2DDouble& StartPt() const
    {
        return m_startPt;
    }
    double Length();

    void appendPath(std::shared_ptr<PathSegmentCmd> cmd);
    void closePath();
    void drawEntirePath(wxGraphicsContext* gc);
    void drawPartialPath(wxGraphicsContext* gc, double percentage);

protected:
    const wxPoint2DDouble m_startPt;
    double m_startDelay = 0.;
    std::vector<std::shared_ptr<PathSegmentCmd>> m_segments;
    bool m_closePath = false;
};

double SketchEffectPath::Length()
{
    double len = 0.0;
    for (const auto& cmd : m_segments)
        len += cmd->Length();

    if ( m_closePath && !m_segments.empty() )
        len += m_startPt.GetDistance(m_segments.back()->EndPoint());

    return len;
}

void SketchEffectPath::appendPath(std::shared_ptr<PathSegmentCmd> cmd)
{
    m_segments.push_back(cmd);
}

void SketchEffectPath::drawEntirePath(wxGraphicsContext* gc)
{
    wxGraphicsPath path( gc->CreatePath() );
    path.MoveToPoint(m_startPt);

    for ( auto& cmd : m_segments )
        cmd->DrawEntireSegment(path);

    if ( m_closePath && !m_segments.empty() )
        path.CloseSubpath();

    gc->StrokePath(path);
}

void SketchEffectPath::drawPartialPath(wxGraphicsContext* gc, double percentage)
{
    double totalLength = Length();

    wxGraphicsPath path(gc->CreatePath());
    path.MoveToPoint(m_startPt);
    double cumulativeLength = 0.;
    for ( auto& cmd : m_segments)
    {
        double length = cmd->Length();
        double percentageAtEndOfSegment = (cumulativeLength + length) / totalLength;
        if (percentage > percentageAtEndOfSegment)
            cmd->DrawEntireSegment(path);
        else
        {
            double percentageAtStartOfSegment = cumulativeLength / totalLength;
            if (percentage >= percentageAtStartOfSegment && percentage < percentageAtEndOfSegment)
            {
                double segmentPercentage = (percentage - percentageAtStartOfSegment) / (percentageAtEndOfSegment - percentageAtStartOfSegment);
                cmd->DrawPartialSegment(path, segmentPercentage);
            }
        }
        cumulativeLength += length;
    }

    if ( m_closePath && !m_segments.empty() )
    {
        double percentageAtStartOfClose = cumulativeLength / totalLength;
        if ( percentage >= percentageAtStartOfClose && percentage < 1. )
        {
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

SketchEffect::SketchEffect( int id )
   : RenderableEffect( id, "Sketch", sketch_16_xpm, sketch_24_xpm, sketch_32_xpm, sketch_48_xpm, sketch_64_xpm )
{
    const int bw = 128;
    const int bh = 96;
    const wxPoint2DDouble path1Start(0., 0.);
    const wxPoint2DDouble path2Start(0.8 * bw, 0.8 * bh);

    auto path1 = std::make_shared<SketchEffectPath>(path1Start);
    auto path1Segment1 = std::make_shared<LineCmd>(path1Start, wxPoint2DDouble(0.5 * bw, 0.5 * bh));
    auto path1Segment2 = std::make_shared<CubicBezierCmd>(path1Segment1->EndPoint(),
                                                          wxPoint2DDouble(bw, bh),
                                                          wxPoint2DDouble(0.25 * bw, 0.25 * bh),
                                                          wxPoint2DDouble(0.65 * bw, 0.3 * bh));
    path1->appendPath(path1Segment1);
    path1->appendPath(path1Segment2);
    path1->closePath();

    auto path2 = std::make_shared<SketchEffectPath>(path2Start);
    auto path2Segment1 = std::make_shared<LineCmd>(path2Start, wxPoint2DDouble(0.3 * bw, 0.7 * bh));
    auto path2Segment2 = std::make_shared<LineCmd>(wxPoint2DDouble(0.3 * bw, 0.7 * bh),
                                                   wxPoint2DDouble(0.05 * bw, 0.1 * bh));
    path2->appendPath(path2Segment1);
    path2->appendPath(path2Segment2);

    m_paths.push_back(path1);
    m_paths.push_back(path2);
}

SketchEffect::~SketchEffect()
{

}

void SketchEffect::Render(Effect* /*effect*/, SettingsMap& /*settings*/, RenderBuffer& buffer )
{
    // This is a terrible effect... it currently just draws a hard-coded sketch!!
 
    double progress = buffer.GetEffectTimeIntervalPosition(1.f);

    int bw = buffer.BufferWi;
    int bh = buffer.BufferHt;
    std::vector<uint8_t> rgb(bw * 3 * bh);
    std::vector<uint8_t> alpha(bw * bh);
    xlColor* px = buffer.GetPixels();
    int pxIndex = 0;
    int rgbIndex = 0;
    int alphaIndex = 0;
    for ( int y = 0; y < bh; ++y )
    {
        for ( int x = 0; x < bw; ++x, ++pxIndex )
        {
            rgb[rgbIndex++] = px[pxIndex].Red();
            rgb[rgbIndex++] = px[pxIndex].Green();
            rgb[rgbIndex++] = px[pxIndex].Green();
            alpha[alphaIndex++] = px[pxIndex].Alpha();
        }
    }

    wxImage img(bw, bh, rgb.data(), alpha.data(), true);
    {
        std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(img));

        double totalLength = 0.;
        for (const auto& path : m_paths)
            totalLength += path->Length();

        double cumulativeLength = 0.;
        int i = 0;
        for (auto iter = m_paths.begin(); iter != m_paths.end(); ++iter, ++i)
        {
            gc->SetPen((i % 2) ? *wxGREEN : *wxRED);
            double pathLength = (*iter)->Length();
            double percentageAtEndOfThisPath = (cumulativeLength + pathLength) / totalLength;

            if (percentageAtEndOfThisPath <= progress)
                (*iter)->drawEntirePath(gc.get());
            else
            {
                double percentageAtStartOfThisPath = cumulativeLength / totalLength;
                double percentageThroughThisPath = (progress - percentageAtStartOfThisPath) / (percentageAtEndOfThisPath - percentageAtStartOfThisPath);
                if (percentageThroughThisPath >= 0.)
                    (*iter)->drawPartialPath(gc.get(), percentageThroughThisPath);
            }
            cumulativeLength += pathLength;
        }
    }

    for ( int y = 0; y < bh; ++y )
    {
        for (int x = 0; x < bw; ++x, ++px)
        {
            px->red = img.GetRed(x, y);
            px->green = img.GetGreen(x, y);
            px->blue = img.GetBlue(x, y);
            px->alpha = img.GetAlpha(x, y);
        }
    }
}

void SketchEffect::SetDefaultParameters()
{

}

bool SketchEffect::needToAdjustSettings( const std::string& /*version*/ )
{
    return false;
}

void SketchEffect::adjustSettings( const std::string& version, Effect* effect, bool removeDefaults/*=true*/ )
{
    // give the base class a chance to adjust any settings
    if ( RenderableEffect::needToAdjustSettings( version ) )
    {
        RenderableEffect::adjustSettings( version, effect, removeDefaults );
    }
}

std::list<std::string> SketchEffect::CheckEffectSettings(const SettingsMap& /*settings*/, AudioManager* /*media*/, Model* /*model*/, Effect* /*eff*/, bool /*renderCache*/ )
{
    return std::list<std::string>();
}

void SketchEffect::RemoveDefaults( const std::string& version, Effect* effect )
{

}

xlEffectPanel* SketchEffect::CreatePanel( wxWindow* parent )
{
    return new SketchPanel( parent );
}
