#include <sstream>

#include "../../include/kaleidoscope-64.xpm"
#include "../../include/kaleidoscope-48.xpm"
#include "../../include/kaleidoscope-32.xpm"
#include "../../include/kaleidoscope-24.xpm"
#include "../../include/kaleidoscope-16.xpm"

#include "KaleidoscopeEffect.h"
#include "KaleidoscopePanel.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../xLightsMain.h"
#include "../xLightsApp.h"
#include "../TimingPanel.h"
#include "UtilFunctions.h"

#include "../Parallel.h"
#include <log4cpp/Category.hh>

KaleidoscopeEffect::KaleidoscopeEffect(int i) : RenderableEffect(i, "Kaleidoscope", kaleidoscope_16, kaleidoscope_24, kaleidoscope_32, kaleidoscope_48, kaleidoscope_64)
{
}

KaleidoscopeEffect::~KaleidoscopeEffect()
{
}

wxPanel *KaleidoscopeEffect::CreatePanel(wxWindow *parent)
{
    return new KaleidoscopePanel(parent);
}

std::list<std::string> KaleidoscopeEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff)
{
    std::list<std::string> res;

    if (settings.Get("T_CHECKBOX_Canvas", "0") == "0")
    {
        res.push_back(wxString::Format("    WARN: Canvas mode not enabled on a Kaleidoscope effect. Without canvas mode Kaleidoscope won't do anything. Effect: Kaleidoscope, Model: %s, Start %s", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

void KaleidoscopeEffect::SetDefaultParameters()
{
    KaleidoscopePanel *p = (KaleidoscopePanel *)panel;

    p->BitmapButton_Kaleidoscope_X->SetActive(false);
    p->BitmapButton_Kaleidoscope_Y->SetActive(false);
    p->BitmapButton_Kaleidoscope_Size->SetActive(false);
    p->BitmapButton_Kaleidoscope_Rotation->SetActive(false);

    p->Choice_Kaleidoscope_Type->SetSelection(0);

    SetSliderValue(p->Slider_Kaleidoscope_X, 50);
    SetSliderValue(p->Slider_Kaleidoscope_Y, 50);
    SetSliderValue(p->Slider_Kaleidoscope_Size, 5);
    SetSliderValue(p->Slider_Kaleidoscope_Rotation, 0);

    // Turn on canvas mode as this really only makes sense in canvas mode
    xLightsFrame* frame = xLightsApp::GetFrame();
    TimingPanel* layerBlendingPanel = frame->GetLayerBlendingPanel();
    layerBlendingPanel->CheckBox_Canvas->SetValue(true);
}

class KaleidoscopeRenderCache : public EffectRenderCache {
public:
    KaleidoscopeRenderCache()
    {
        _size = -1;
        _rotation = -1;
        _x = -1;
        _y = -1;
        _width = -1;
        _height = -1;
    };
    virtual ~KaleidoscopeRenderCache() {};
    int _size;
    int _rotation;
    int _x;
    int _y;
    int _width;
    int _height;
    void Initialise(int size, int rotation, int x, int y, int width, int height, const std::string& type, bool force)
    {
        if (force || size != _size || rotation != _rotation || x != _x || y != _y || width != _width || height != _height)
        {
            _size = size;
            _rotation = rotation;
            _x = x;
            _y = y;
            _width = width;
            _height = height;

            // clear the used arrary
            _startUsed.resize(width);
            for (auto& xx : _startUsed)
            {
                xx.resize(height);
                for (auto yy = xx.begin(); yy != xx.end(); ++yy)
                {
                    *yy = false;
                }
            }
            _edges.clear();

            // now repopulate
            if (type == "Square")
            {
                InitialiseSquare(size, rotation, x, y, width, height);
            }
            else if (type == "Triangle")
            {
                InitialiseTriangle(size, rotation, x, y, width, height);
            }
            else
            {
                wxASSERT(false);
            }
        }
    }

    bool CreateEdge(int x1, int y1, int x2, int y2)
    {
        if (x1 < 0 && x2 < 0) return false;
        if (x1 > _width && x2 > _width) return false;
        if (y1 < 0 && y2 < 0) return false;
        if (y1 > _height && y2 > _height) return false;

        if (x1 != x2 && y1 != y2)
        {
            bool bl = IsPointAboveLineThroughPoints(0, 0, x1, y1, x2, y2);
            bool tl = IsPointAboveLineThroughPoints(0, _height - 1, x1, y1, x2, y2);
            bool tr = IsPointAboveLineThroughPoints(_width - 1, _height - 1, x1, y1, x2, y2);
            bool br = IsPointAboveLineThroughPoints(_width - 1, 0, x1, y1, x2, y2);

            // if all corners are on the same side of the line then this line isnt interesting
            if (bl == tl && bl == tr && bl == br) return false;
        }

        _edges.push_back(KaleidoscopeEdge(wxPoint(x1, y1), wxPoint(x2, y2)));
        return true;
    }

    bool IsPointAboveLineThroughPoints(int x, int y, int x1, int y1, int x2, int y2)
    {
        double slope = (double)(y2 - y1) / (double)(x2 - x1);
        double yintercept = slope * -1.0 * (double)x1 + (double)y1;
        double ytest = slope * x + yintercept;
        return (double)y > ytest;
    }

    std::pair<int,int> GetPointAfterMove(int x, int y, int degrees, double d)
    {
        double a = toRadians(degrees);
        double dx = d * cos(a);

        int f = 1;
        if (dx < 0) f = -1;

        double aa = std::abs(std::abs(dx) - (int)std::abs(dx) - 0.5);
        if (aa < 0.0000001)
        {
            dx = f * ((int)std::abs(dx) + 0.5);
        }
        double dy = d * sin(a);
        f = 1;
        if (dy < 0) f = -1;
        double bb = std::abs(std::abs(dy) - (int)std::abs(dy) - 0.5);
        if (bb < 0.0000001)
        {
            dy = f * ((int)std::abs(dy) + 0.5);
        }
        return { x + std::round(dx), y + std::round(dy) };
    }

    void InitialiseSquare(int size, int rotation, int x, int y, int width, int height)
    {
        for (int xx = std::max(0, x - size / 2); xx <= std::min(x + size / 2, width -1); xx++)
        {
            for (int yy = std::max(0, y - size / 2); yy <= std::min(y + size / 2, height -1); yy++)
            {
                if (xx >= 0 && xx < width && yy >= 0 && yy < height)
                {
                    _startUsed[xx][yy] = true;
                }
            }
        }

        double cornerdistance = sqrt(2.0 * size * size ) / 2.0;
        int iterations = 0;
        int added;
        do
        {
            added = 0;
            auto p1 = GetPointAfterMove(x, y, rotation + 45, (2 * iterations + 1) * cornerdistance);
            auto p2 = GetPointAfterMove(x, y, rotation + 45 + 90, (2 * iterations + 1) * cornerdistance);
            auto p3 = GetPointAfterMove(x, y, rotation + 45 + 180, (2 * iterations + 1) * cornerdistance);
            auto p4 = GetPointAfterMove(x, y, rotation + 45 + 270, (2 * iterations + 1) * cornerdistance);

            if (CreateEdge(p1.first, p1.second, p2.first, p2.second)) added++;
            if (CreateEdge(p2.first, p2.second, p3.first, p3.second)) added++;
            if (CreateEdge(p3.first, p3.second, p4.first, p4.second)) added++;
            if (CreateEdge(p4.first, p4.second, p1.first, p1.second)) added++;

            iterations++;
        } while (added > 0);
    }

    void InitialiseTriangle(int size, int rotation, int x, int y, int width, int height)
    {
        const double radiusFactor = tan(toRadians(30)) / 2.0;
        double radius = size * radiusFactor;

        int minx = GetPointAfterMove(x, y, 240, radius).first;
        int miny = GetPointAfterMove(x, y, 240, radius).second;
        int maxx = GetPointAfterMove(x, y, 300, radius).first;
        int maxy = GetPointAfterMove(x, y, 90, radius).second;
        int midx = (maxx + minx) / 2;

        for (int xx = std::max(0,minx); xx < std::min(midx, width -1); xx++)
        {
            for (int yy = std::max(0, miny); yy <= std::min(maxy, height - 1); yy++)
            {
                if (!IsPointAboveLineThroughPoints(xx, yy, minx, miny, midx, maxy))
                {
                    _startUsed[xx][yy] = true;
                }
            }
        }
        for (int xx = std::max(0,midx); xx <= std::min(maxx, width -1); xx++)
        {
            for (int yy = std::max(0,miny); yy <= std::min(maxy, height -1); yy++)
            {
                if (!IsPointAboveLineThroughPoints(xx, yy, midx, maxy, maxx, miny))
                {
                    _startUsed[xx][yy] = true;
                }
            }
        }

        // bring the edges inside the shape area
        if (radius > 10)
        {
            radius -= 1;
        }
        else if (radius > 5)
        {
            radius -= 0.5;
        }
        else
        {
            radius -= -.1;
        }

        double s = size;
        double gap = sqrt(s * s - s / 2.0 * s / 2.0);
        int iterations = 0;
        int added;
        do
        {
            added = 0;
            if (iterations == 0)
            {
                auto p1 = GetPointAfterMove(x, y, rotation + 90, radius);
                auto p2 = GetPointAfterMove(x, y, rotation + 240, radius);
                auto p3 = GetPointAfterMove(x, y, rotation + 300, radius);

                if (CreateEdge(p1.first, p1.second, p2.first, p2.second)) added++;
                if (CreateEdge(p2.first, p2.second, p3.first, p3.second)) added++;
                if (CreateEdge(p3.first, p3.second, p1.first, p1.second)) added++;
                if (rotation == 0)
                {
                    wxASSERT(p2.second == p3.second);
                }
            }
            else
            {
                auto p1 = GetPointAfterMove(x, y, rotation + 90, iterations * gap + radius);
                auto p2 = GetPointAfterMove(x, y, rotation + 240, iterations * gap + radius);
                auto p3 = GetPointAfterMove(x, y, rotation + 300, iterations * gap + radius);

                if (CreateEdge(p1.first, p1.second, p2.first, p2.second)) added++;
                if (CreateEdge(p2.first, p2.second, p3.first, p3.second)) added++;
                if (CreateEdge(p3.first, p3.second, p1.first, p1.second)) added++;
            }

            iterations++;
        } while (added > 0);
    }

    std::vector<std::vector<bool>> _startUsed;
    std::list<KaleidoscopeEdge> _edges;
};

bool KaleidoscopeEffect::KaleidoscopeDone(const std::vector<std::vector<bool>>& current)
{
    for (const auto& xx : current)
    {
        for (const auto yy : xx)
        {
            if (!yy) return false;
        }
    }
    return true;
}

std::pair<int, int> KaleidoscopeEffect::GetSourceLocation(int x, int y, const KaleidoscopeEdge& edge, int width, int height)
{
    double x1 = edge._p1.x;
    double x2 = edge._p2.x;
    double y1 = edge._p1.y;
    double y2 = edge._p2.y;

    double dx = x2 - x1;
    double dy = y2 - y1;

    double a = (dx * dx - dy * dy) / (dx * dx + dy * dy);
    double b = 2.0 * dx * dy / (dx*dx + dy * dy);

    return { std::round(a * (x - x1) + b * (y - y1) + x1),
            std::round(b * (x - x1) - a * (y - y1) + y1) };
}

void DumpUsed(const std::vector<std::vector<bool>>& current, int width, int height)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    for (int y = height - 1; y >= 0; y--)
    {
        std::string row;
        for (int x = 0; x < width; x++)
        {
            bool b = current[x][y];
            row += wxString::Format(" %d", (int)b);
        }
        logger_base.debug(row);
    }
}

void KaleidoscopeEffect::Render(Effect *eff, SettingsMap &SettingsMap, RenderBuffer &buffer)
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    float progress = buffer.GetEffectTimeIntervalPosition(1.f);

    std::string type = SettingsMap.Get("CHOICE_Kaleidoscope_Type", "Triangle");
    int xCentre = GetValueCurveInt("Kaleidoscope_X", 50, SettingsMap, progress, KALEIDOSCOPE_X_MIN, KALEIDOSCOPE_X_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) * buffer.BufferWi / 100;
    int yCentre = GetValueCurveInt("Kaleidoscope_Y", 50, SettingsMap, progress, KALEIDOSCOPE_Y_MIN, KALEIDOSCOPE_Y_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) * buffer.BufferHt / 100;
    int size = GetValueCurveInt("Kaleidoscope_Size", 5, SettingsMap, progress, KALEIDOSCOPE_SIZE_MIN, KALEIDOSCOPE_SIZE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int rotation = GetValueCurveInt("Kaleidoscope_Rotation", 0, SettingsMap, progress, KALEIDOSCOPE_ROTATION_MIN, KALEIDOSCOPE_ROTATION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    KaleidoscopeRenderCache *cache = static_cast<KaleidoscopeRenderCache*>(buffer.infoCache[id]);
    if (cache == nullptr) {
        cache = new KaleidoscopeRenderCache();
        buffer.infoCache[id] = cache;
    }

    if (buffer.needToInit)
    {
        buffer.needToInit = false;
        cache->Initialise(size, rotation, xCentre, yCentre, buffer.BufferWi, buffer.BufferHt, type, true);
    }
    else
    {
        // reinitialise ... but only if something has changed
        cache->Initialise(size, rotation, xCentre, yCentre, buffer.BufferWi, buffer.BufferHt, type, false);
    }

    auto currentUsed = cache->_startUsed;
    auto &edges = cache->_edges;

    auto edge = edges.begin();
    //logger_base.debug("frame. Edges %d", (int)edges.size());
    std::atomic_int setSinceBegin = 0;
    while (!KaleidoscopeDone(currentUsed) && edges.size() > 0)
    {
        //logger_base.debug("   iterate");
        //int set = 0;

        //DumpUsed(currentUsed, buffer.BufferWi, buffer.BufferHt);
        parallel_for(0, buffer.BufferHt, [&currentUsed, this, &buffer, &edge, &setSinceBegin] (int y) {
            for (int x = 0; x < buffer.BufferWi; x++) {
                if (!currentUsed[x][y]) {
                    // this pixel needs to be set
                    auto source = GetSourceLocation(x, y, *edge, buffer.BufferWi, buffer.BufferHt);
                    if (source.first >= 0 && source.first < buffer.BufferWi && source.second >= 0 && source.second < buffer.BufferHt) {
                        if (currentUsed[source.first][source.second]) {
                            buffer.SetPixel(x, y, buffer.GetPixel(source.first, source.second));
                            currentUsed[x][y] = true;
                            //set++;
                            setSinceBegin++;
                        }
                    }
                }
            }
        });
        //logger_base.debug("   set this iteration %d", set);
        ++edge;
        if (edge == edges.end()) {
            if (setSinceBegin == 0)
            {
                break;
            }
            setSinceBegin = 0;
            edge = edges.begin();
        }
    }
}
