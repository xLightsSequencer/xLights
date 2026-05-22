#pragma once

#include <QHash>
#include <QList>
#include <QMainWindow>
#include <QString>
#include <QVector>

class ControllerUploadDialog;
class ControllerVisualizerWindow;
class EffectPanelWidget;
class LayoutWindow;
class EffectToolBar;
class HousePreviewWidget;
class ModelInfoWindow;
class PlaybackController;
class PreviewWidget;
class QColor;
class QEvent;
class QObject;
class QProgressBar;
class QtRenderBridge;
class QSplitter;
class RenderDetailDialog;
class SequencerWidget;
class TransportToolBar;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent*) override;
    bool eventFilter(QObject* watched, QEvent* ev) override;

private slots:
    void onEffectSelected(const QString& name);
    void onBlockSelected(int row, int block);

private:
    void setupMenuBar();
    void setupStatusBar();
    void populateEffectBar();
    void syncDuration();
    void triggerRender(const QString& effectName, double progress = 0.5,
                       const QString& rawSettings = {}, const QString& rawPalette = {});

    // Composite all active layers for _currentModel at the current playhead.
    void renderAllLayers();

    // Render active layers for a single model; returns composite pixel array.
    // Returns empty list if model has no active blocks at the current frame.
    QList<QColor> renderModelLayers(const QString& modelName);

    // Build the full frame-by-frame render cache for all models.
    // Called by the Render All button.  Uses the software renderer for speed.
    void renderAllModels();

    // Display a specific frame from the pre-built cache on house + model previews.
    void displayCacheFrame(int frame);

    // After rendering a group effect, distribute pixels to each member model's
    // nodes based on their global positions within the group bounding box,
    // and update both the house preview and the model preview widget.
    void distributeGroupToMembers(const QString& groupName,
                                  const QList<QColor>& groupPixels,
                                  bool updateModelPreview = false);

    // Variant used during cache build: stores pixels in the render cache
    // for each member model at the given frame instead of updating previews.
    void cacheGroupDistribution(const QString& groupName,
                                const QList<QColor>& groupPixels,
                                int frame);


PlaybackController*  _playback      = nullptr;
    TransportToolBar*    _transport     = nullptr;
    EffectToolBar*       _effectsBar    = nullptr;
    QtRenderBridge*      _renderBridge  = nullptr;
    QSplitter*           _splitter      = nullptr;
    QSplitter*           _centerV       = nullptr;
    SequencerWidget*     _sequencer     = nullptr;
    PreviewWidget*       _preview       = nullptr;
    EffectPanelWidget*   _effectPanel   = nullptr;
    HousePreviewWidget*  _housePreview  = nullptr;

    QString             _currentModel;
    QProgressBar*       _renderProgress    = nullptr;
    RenderDetailDialog* _renderDetailDlg   = nullptr;

    bool                _renderAllInProgress = false;

    // Full timeline render cache built by renderAllModels().
    // _renderCache[modelName][frame] = pixel data (empty = no active effect → black)
    // Uses QRgb (4 bytes/pixel) for compact storage.
    QHash<QString, QVector<QVector<QRgb>>> _renderCache;
    bool                                   _renderCacheValid = false;

    ModelInfoWindow*             _modelInfoWin     = nullptr;
    LayoutWindow*                _layoutWin        = nullptr;
    ControllerVisualizerWindow*  _controllerVizWin    = nullptr;
    ControllerUploadDialog*      _controllerUploadDlg = nullptr;
};
