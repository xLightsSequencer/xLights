#pragma once
#include "QtEffectRenderer.h"
#include <QObject>
#include <QString>
#include <QTimer>

// Forward declarations so callers don't need to pull in all src-core headers.
class UICallbacks;
class ModelManager;

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

    // Upload outputs + inputs for the named controller.
    // Runs synchronously — call from a background thread to avoid blocking the UI.
    // Returns true on success. ui receives progress and error messages.
    bool upload(const QString& controllerName, UICallbacks* ui);

    // Synchronous render — returns result immediately, no debounce.
    // Used for live playback updates where latency matters more than debouncing.
    QtEffectRenderer::Result renderNow(const QtEffectRenderer::Request& req);

    // Access the live ModelManager (populated after setShowFolder succeeds).
    // May return nullptr if no show folder has been set or init failed.
    ModelManager* modelManager() const;

signals:
    void frameReady(QtEffectRenderer::Result result);

private:
    QTimer                    _debounce;
    QtEffectRenderer::Request _pending;
    QString                   _metadataDir;
    QString                   _showFolder;

    // Ensure s_ctx / s_mm are created.  Called eagerly from setShowFolder()
    // so modelManager() returns valid data before the first render is requested.
    void ensureInitialized();

    QtEffectRenderer::Result renderCore(const QtEffectRenderer::Request& req);
};
