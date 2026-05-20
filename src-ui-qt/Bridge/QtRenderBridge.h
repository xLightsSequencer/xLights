#pragma once
#include "QtEffectRenderer.h"
#include <QObject>
#include <QString>
#include <QTimer>

// Debounces render requests and runs them through the src-core rendering
// pipeline (RenderableEffect::Render), matching the wx and iPad render paths.
// Returns a black result if src-core is unavailable or the effect throws.
class QtRenderBridge : public QObject {
    Q_OBJECT
public:
    explicit QtRenderBridge(QObject* parent = nullptr);

    // Set the path to effectmetadata/ so EffectManager can load effect JSON.
    void setMetadataDir(const QString& dir);

    // Load outputs/controllers from the show folder (xlights_networks.xml).
    // Safe to call before init — stored and applied when the context is created.
    void setShowFolder(const QString& showFolder);

    // Queue a render request; fires after 40 ms of silence.
    void request(const QtEffectRenderer::Request& req);

    // Synchronous render — returns result immediately, no debounce.
    // Used for live playback updates where latency matters more than debouncing.
    QtEffectRenderer::Result renderNow(const QtEffectRenderer::Request& req);

signals:
    void frameReady(QtEffectRenderer::Result result);

private:
    QTimer                    _debounce;
    QtEffectRenderer::Request _pending;
    QString                   _metadataDir;
    QString                   _showFolder;

    QtEffectRenderer::Result renderCore(const QtEffectRenderer::Request& req);
};
