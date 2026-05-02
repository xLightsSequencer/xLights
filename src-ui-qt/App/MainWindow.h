#pragma once

#include <QList>
#include <QMainWindow>
#include <QString>

class ControllerInfoWindow;
class EffectPanelWidget;
class EffectToolBar;
class HousePreviewWidget;
class ModelInfoWindow;
class PlaybackController;
class PreviewWidget;
class QColor;
class QProgressBar;
class QtRenderBridge;
class QSplitter;
class SequencerWidget;
class TransportToolBar;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent*) override;

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

    // Render ALL models in the sequence and update the house preview.
    void renderAllModels();

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
    int                 _lastRenderedFrame = -1;
    QProgressBar*       _renderProgress    = nullptr;

    ModelInfoWindow*      _modelInfoWin      = nullptr;
    ControllerInfoWindow* _controllerInfoWin = nullptr;
};
