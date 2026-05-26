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

    // Persist the named model's current state back to xlights_rgbeffects.xml
    // (or the corresponding model-group / view-object element).  Uses
    // pugixml: loads the existing file, finds the matching node by name,
    // replaces its attributes/children with a fresh serialisation from the
    // live Model*, and writes the file back.  All other content in the file
    // (other models, controllers, layout groups, perspectives, etc.) is
    // preserved verbatim.
    //
    // Returns true on success.  No-op + returns false if the show folder is
    // unset or the model can't be found.
    bool saveModelToShowFile(const QString& modelName);

    // Persist the entire OutputManager (all controllers + outputs) back to
    // xlights_networks.xml.  Uses the src-core save path (OutputManager::Save)
    // which builds the XML from in-memory state.  Returns true on success.
    bool saveControllersToShowFile();

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
