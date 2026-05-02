#include "QtEffectRenderer.h"
#include <QtMath>
#include <cmath>

namespace QtEffectRenderer {

// ── Helpers ───────────────────────────────────────────────────────────────────

static QColor pal(const QList<QColor>& p, int idx) {
    return p.isEmpty() ? Qt::white : p[((idx % p.size()) + p.size()) % p.size()];
}

// Linearly interpolate between adjacent palette entries using t in [0,1].
static QColor palF(const QList<QColor>& p, double t) {
    if (p.isEmpty()) return Qt::black;
    if (p.size() == 1) return p[0];
    t = std::fmod(t, 1.0); if (t < 0) t += 1.0;
    double pos = t * p.size();
    int i = int(pos) % p.size();
    double f = pos - int(pos);
    const QColor& a = p[i];
    const QColor& b = p[(i + 1) % p.size()];
    return QColor(int(a.red()  + f*(b.red()  - a.red())),
                  int(a.green()+ f*(b.green()- a.green())),
                  int(a.blue() + f*(b.blue() - a.blue())));
}

static Result blank(const Request& r) {
    Result out; out.w = r.bufferW; out.h = r.bufferH;
    out.pixels.fill(Qt::black, r.bufferW * r.bufferH);
    return out;
}

static double dget(const QVariantMap& s, const char* k, double def) {
    auto it = s.find(k);
    return it != s.end() ? it->toDouble() : def;
}
static QString sget(const QVariantMap& s, const char* k, const char* def) {
    auto it = s.find(k);
    return it != s.end() ? it->toString() : def;
}

// ── Effects ───────────────────────────────────────────────────────────────────

static Result renderOn(const Request& r) {
    Result out = blank(r);
    out.pixels.fill(r.palette.isEmpty() ? Qt::white : r.palette[0]);
    return out;
}

static Result renderOff(const Request& r) { return blank(r); }

static Result renderBars(const Request& r) {
    Result out = blank(r);
    const auto& p  = r.palette;
    if (p.isEmpty()) return out;
    int    bars   = int(dget(r.settings, "Bars_BarCount", 1));
    double cycles = dget(r.settings, "Bars_Cycles", 10.0) / 10.0;
    QString dir   = sget(r.settings, "Bars_Direction", "up");
    bool    vert  = (dir=="up"||dir=="down"||dir=="expand"||dir=="compress"
                     ||dir=="Alternate Up"||dir=="Alternate Down");
    double  off   = r.progress * cycles;

    for (int y = 0; y < r.bufferH; ++y)
        for (int x = 0; x < r.bufferW; ++x) {
            double pos = vert ? double(y) / qMax(1, r.bufferH)
                               : double(x) / qMax(1, r.bufferW);
            double t = std::fmod(pos * bars + off, 1.0);
            if (t < 0) t += 1.0;
            out.pixels[y * r.bufferW + x] = pal(p, int(t * p.size()));
        }
    return out;
}

static Result renderRainbow(const Request& r) {
    Result out = blank(r);
    double cycles = dget(r.settings, "Rainbow_Cycles", 10.0) / 10.0;
    double off    = r.progress * cycles;
    for (int y = 0; y < r.bufferH; ++y)
        for (int x = 0; x < r.bufferW; ++x) {
            double hue = std::fmod(double(x) / r.bufferW + off, 1.0);
            if (hue < 0) hue += 1.0;
            out.pixels[y * r.bufferW + x] = QColor::fromHsvF(float(hue), 1.f, 1.f);
        }
    return out;
}

static Result renderColorloop(const Request& r) {
    Result out = blank(r);
    double hue = std::fmod(r.progress, 1.0);
    out.pixels.fill(QColor::fromHsvF(float(hue), 1.f, 1.f));
    return out;
}

static Result renderFire(const Request& r) {
    Result out = blank(r);
    uint seed = uint(r.progress * 997);
    for (int y = 0; y < r.bufferH; ++y) {
        for (int x = 0; x < r.bufferW; ++x) {
            // bottom = hot, top = cool
            double t = r.bufferH > 1
                ? double(r.bufferH - 1 - y) / r.bufferH
                : 0.6;
            // turbulence
            double n = 0.06 * std::sin(x * 1.3 + seed * 0.05 + t * 4.0);
            t = qBound(0.0, t + n, 1.0);
            // Use palette if given (expect warm colors), else fall back to fire colors
            QColor c;
            if (!r.palette.isEmpty()) {
                c = palF(r.palette, t);
            } else {
                double hue = t * 0.1;   // yellow(0) → dark red(0.1)
                c = QColor::fromHsvF(float(hue), 1.f, float(qMax(0.0, 1.0 - t * 0.6)));
            }
            out.pixels[y * r.bufferW + x] = c;
        }
    }
    return out;
}

static Result renderChase(const Request& r) {
    Result out = blank(r);
    if (r.palette.isEmpty()) return out;
    int    tailLen = qMax(1, r.bufferW / 6);
    double cycles  = dget(r.settings, "Chase_Cycles", 10.0) / 10.0;
    int    head    = int(r.progress * cycles * r.bufferW) % r.bufferW;
    if (head < 0) head += r.bufferW;

    for (int x = 0; x < r.bufferW; ++x) {
        int dist = (x - head + r.bufferW) % r.bufferW;
        if (dist >= tailLen) continue;
        double bright = 1.0 - double(dist) / tailLen;
        QColor c = r.palette[dist % r.palette.size()];
        c.setRgbF(c.redF()*bright, c.greenF()*bright, c.blueF()*bright);
        for (int y = 0; y < r.bufferH; ++y)
            out.pixels[y * r.bufferW + x] = c;
    }
    return out;
}

static Result renderMeteor(const Request& r) {
    // Meteor is a single-direction chase with a brighter head
    return renderChase(r);
}

static Result renderSparkle(const Request& r) {
    Result out = blank(r);
    const auto& p = r.palette;
    uint seed = uint(r.progress * 97);
    int sparks = qMax(1, r.bufferW * r.bufferH / 6);
    for (int i = 0; i < sparks; ++i) {
        uint rx = (seed * 1664525u + uint(i) * 214013u + 12345u) % uint(r.bufferW);
        uint ry = (seed * 214013u  + uint(i) * 1664525u + 54321u) % uint(r.bufferH);
        out.pixels[int(ry) * r.bufferW + int(rx)] =
            p.isEmpty() ? Qt::white : p[int(rx) % p.size()];
    }
    return out;
}

static Result renderTwinkle(const Request& r) {
    Result out = blank(r);
    const auto& p = r.palette;
    uint seed = uint(r.progress * 53);
    for (int y = 0; y < r.bufferH; ++y)
        for (int x = 0; x < r.bufferW; ++x) {
            uint h = seed * 1664525u + uint(y * r.bufferW + x) * 214013u + 1u;
            if (h % 5u == 0u && !p.isEmpty())
                out.pixels[y * r.bufferW + x] = p[int(h % uint(p.size()))];
        }
    return out;
}

static Result renderSpiral(const Request& r) {
    Result out = blank(r);
    const auto& p = r.palette;
    if (p.isEmpty()) return out;
    double off = r.progress * 3.0;
    double cx = r.bufferW * 0.5, cy = r.bufferH * 0.5;
    for (int y = 0; y < r.bufferH; ++y)
        for (int x = 0; x < r.bufferW; ++x) {
            double angle = std::atan2(y - cy, x - cx) / (2 * M_PI);
            double dist  = std::sqrt((x-cx)*(x-cx)/(cx*cx) + (y-cy)*(y-cy)/qMax(1.0,cy*cy));
            double t     = std::fmod(angle + dist * 1.5 + off, 1.0);
            if (t < 0) t += 1.0;
            out.pixels[y * r.bufferW + x] = palF(p, t);
        }
    return out;
}

static Result renderSingleStrand(const Request& r) {
    // Single colour sweep using palette
    Result out = blank(r);
    const auto& p = r.palette;
    if (p.isEmpty()) return out;
    for (int y = 0; y < r.bufferH; ++y)
        for (int x = 0; x < r.bufferW; ++x) {
            double t = std::fmod(double(x) / r.bufferW + r.progress, 1.0);
            out.pixels[y * r.bufferW + x] = palF(p, t);
        }
    return out;
}

static Result renderSnowflakes(const Request& r) {
    Result out = blank(r);
    const auto& p = r.palette;
    uint seed = uint(r.progress * 37);
    int flakes = qMax(1, r.bufferW / 5);
    for (int i = 0; i < flakes; ++i) {
        uint rx = (seed * 1013904u + uint(i) * 6364136u) % uint(r.bufferW);
        uint ry = (seed * 6364136u + uint(i) * 1013904u) % uint(r.bufferH);
        out.pixels[int(ry) * r.bufferW + int(rx)] =
            p.isEmpty() ? Qt::white : p[0];
    }
    return out;
}

static Result renderShimmer(const Request& r) {
    // Random per-pixel brightness flicker using the palette
    Result out = blank(r);
    const auto& p = r.palette;
    if (p.isEmpty()) return out;
    uint seed = uint(r.progress * 1000);
    for (int i = 0; i < out.pixels.size(); ++i) {
        uint h = seed * 1664525u + uint(i) * 214013u + 1u;
        double bright = 0.3 + 0.7 * double(h % 1000) / 1000.0;
        QColor c = p[int(h % uint(p.size()))];
        out.pixels[i] = QColor(int(c.red()*bright), int(c.green()*bright), int(c.blue()*bright));
    }
    return out;
}

static Result renderPinwheel(const Request& r) {
    Result out = blank(r);
    const auto& p = r.palette;
    if (p.isEmpty()) return out;
    int   arms = dget(r.settings, "Pinwheel_Arms", 3.0);
    double off = r.progress * 2.0;
    double cx = r.bufferW * 0.5, cy = r.bufferH * 0.5;
    for (int y = 0; y < r.bufferH; ++y)
        for (int x = 0; x < r.bufferW; ++x) {
            double angle = std::atan2(y - cy, x - cx) / (2*M_PI) + 0.5;
            double t = std::fmod(angle * arms + off, 1.0);
            if (t < 0) t += 1.0;
            out.pixels[y * r.bufferW + x] = palF(p, t);
        }
    return out;
}

static Result renderFireworks(const Request& r) {
    Result out = blank(r);
    const auto& p = r.palette;
    if (p.isEmpty()) return out;
    // Burst of dots expanding from a random center
    uint seed = uint(r.progress * 13);
    double cx = double((seed * 214013u) % uint(r.bufferW));
    double cy = double((seed * 1664525u) % uint(r.bufferH));
    double radius = r.progress * r.bufferW * 0.5;
    int particles = 32;
    for (int i = 0; i < particles; ++i) {
        double angle = 2.0 * M_PI * i / particles;
        int px = int(cx + std::cos(angle) * radius);
        int py = int(cy + std::sin(angle) * radius);
        if (px < 0 || px >= r.bufferW || py < 0 || py >= r.bufferH) continue;
        out.pixels[py * r.bufferW + px] = p[i % p.size()];
    }
    return out;
}

static Result renderGalaxy(const Request& r) {
    Result out = blank(r);
    const auto& p = r.palette;
    if (p.isEmpty()) return out;
    double off = r.progress * 2.0;
    double cx = r.bufferW * 0.5, cy = r.bufferH * 0.5;
    int arms = 3;
    for (int y = 0; y < r.bufferH; ++y)
        for (int x = 0; x < r.bufferW; ++x) {
            double dx = x - cx, dy = y - cy;
            double dist  = std::sqrt(dx*dx + dy*dy) / (r.bufferW * 0.5);
            double angle = std::atan2(dy, dx) / (2*M_PI);
            double spiral = std::fmod(angle * arms + dist * 2.0 + off, 1.0);
            if (spiral < 0) spiral += 1.0;
            double brightness = qMax(0.0, 1.0 - dist);
            QColor c = palF(p, spiral);
            out.pixels[y*r.bufferW+x] = QColor(int(c.red()*brightness),
                                                int(c.green()*brightness),
                                                int(c.blue()*brightness));
        }
    return out;
}

static Result renderWave(const Request& r) {
    Result out = blank(r);
    const auto& p = r.palette;
    if (p.isEmpty()) return out;
    double cycles = dget(r.settings, "Wave_Cycles", 30.0) / 10.0;
    double off = r.progress * cycles;
    for (int y = 0; y < r.bufferH; ++y)
        for (int x = 0; x < r.bufferW; ++x) {
            double wave = std::sin(double(x) / r.bufferW * M_PI * 4.0 + off * M_PI * 2.0);
            double t = (wave + 1.0) / 2.0;
            double yt = r.bufferH > 1 ? double(y) / r.bufferH : 0.5;
            double bright = qMax(0.0, 1.0 - std::abs(yt - 0.5 - wave * 0.3) * 4.0);
            QColor c = palF(p, t);
            out.pixels[y*r.bufferW+x] = bright > 0.05
                ? QColor(int(c.red()*bright), int(c.green()*bright), int(c.blue()*bright))
                : Qt::black;
        }
    return out;
}

static Result renderText(const Request& r) {
    // Text effect: palette gradient until src-core rendering is wired in.
    return renderSingleStrand(r);
}

// Generic: scroll palette across the buffer
static Result renderGeneric(const Request& r) {
    Result out = blank(r);
    const auto& p = r.palette;
    if (p.isEmpty()) return out;
    for (int y = 0; y < r.bufferH; ++y)
        for (int x = 0; x < r.bufferW; ++x) {
            double t = std::fmod(double(x) / r.bufferW + r.progress, 1.0);
            out.pixels[y * r.bufferW + x] = palF(p, t);
        }
    return out;
}

// ── Dispatch ──────────────────────────────────────────────────────────────────

Result render(const Request& req) {
    const QString& n = req.effectName;
    if (n == "On")           return renderOn(req);
    if (n == "Off")          return renderOff(req);
    if (n == "Bars")         return renderBars(req);
    if (n == "Rainbow")      return renderRainbow(req);
    if (n == "Colorloop")    return renderColorloop(req);
    if (n == "Fire")         return renderFire(req);
    if (n == "Chase")        return renderChase(req);
    if (n == "Meteor")       return renderMeteor(req);
    if (n == "Sparkle")      return renderSparkle(req);
    if (n == "Twinkle")      return renderTwinkle(req);
    if (n == "Spiral")       return renderSpiral(req);
    if (n == "Snowflakes")   return renderSnowflakes(req);
    if (n == "SingleStrand") return renderSingleStrand(req);
    if (n == "Shimmer")      return renderShimmer(req);
    if (n == "Pinwheel")     return renderPinwheel(req);
    if (n == "Fireworks")    return renderFireworks(req);
    if (n == "Galaxy")       return renderGalaxy(req);
    if (n == "Wave")         return renderWave(req);
    if (n == "Text")         return renderText(req);
    // All remaining effects fall through to the generic palette scroller.
    return renderGeneric(req);
}

} // namespace QtEffectRenderer
