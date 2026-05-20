#pragma once
#include <QColor>
#include <QList>
#include <QString>
#include <QVariantMap>

// Data-transfer types shared by QtRenderBridge and its callers.
// All rendering goes through QtRenderBridge → src-core (RenderableEffect::Render).
// The software renderer that previously lived in QtEffectRenderer.cpp has been
// removed; these structs are retained because they are the public API surface.
namespace QtEffectRenderer {

struct Request {
    QString       effectName;
    QString       modelName;    // show-file model/group name; empty = use stub
    QVariantMap   settings;     // UI panel key→value map
    QList<QColor> palette;
    int           bufferW  = 100;
    int           bufferH  = 1;
    double        progress = 0.0;

    // Raw xsq settings/palette strings — passed directly to src-core
    // SettingsMap::Parse() when non-empty.  Takes precedence over `settings`.
    QString       rawSettings;
    QString       rawPalette;
};

struct Result {
    int           w      = 0;
    int           h      = 0;
    QList<QColor> pixels;           // row-major: pixel[y*w + x]

    bool isValid() const { return w > 0 && h > 0 && pixels.size() == w * h; }
};

} // namespace QtEffectRenderer
