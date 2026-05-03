#pragma once
#include <QColor>
#include <QList>
#include <QString>
#include <QVariantMap>

// Software effect renderer — produces pixel data from effect name + settings
// + palette without requiring src-core.  Covers the most common effects with
// visually correct output.  Phase 7 will swap this for real RenderableEffect.
namespace QtEffectRenderer {

struct Request {
    QString       effectName;
    QString       modelName;    // show-file model/group name; empty = use stub
    QVariantMap   settings;     // UI panel key→value map (used by software renderer)
    QList<QColor> palette;
    int           bufferW  = 100;   // pixels wide (used when model not found)
    int           bufferH  = 1;     // pixels tall  (used when model not found)
    double        progress = 0.0;   // 0..1 within the effect's duration

    // Raw xsq settings/palette strings for src-core rendering.
    // When non-empty, passed directly to SettingsMap::Parse() instead of
    // the prefix-guessing approach used for the software renderer.
    QString       rawSettings;
    QString       rawPalette;
};

struct Result {
    int           w      = 0;
    int           h      = 0;
    QList<QColor> pixels;           // row-major: pixel[y*w + x]

    bool isValid() const { return w > 0 && h > 0 && pixels.size() == w * h; }
};

Result render(const Request& req);

} // namespace QtEffectRenderer
