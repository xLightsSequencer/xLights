#include "MainWindow.h"
#include "PlaybackController.h"
#include "QtXLightsApp.h"
#include "TransportToolBar.h"
#include "../Info/ModelInfoWindow.h"
#include "../Bridge/FseqWriter.h"
#include "../Bridge/QtEffectRenderer.h"
#include "../Bridge/QtRenderBridge.h"
#include "../Bridge/QtSequenceDoc.h"   // QtModelInfo, QtSequenceInfo
#include "ExportDialog.h"
#include "RenderDetailDialog.h"
#include "../Effects/EffectPanelWidget.h"
#include "../Effects/EffectToolBar.h"
#include "../Bridge/SubBufferUtil.h"
#include "../Layout/HousePreviewWidget.h"
#include "../Layout/LayoutWindow.h"
#include "../Controllers/ControllerVisualizerWindow.h"
#include "../Preview/PreviewWidget.h"
#include "../Sequencer/SequencerModel.h"
#include "../Sequencer/SequencerWidget.h"

#include <QAction>
#include <QDesktopServices>
#include <QRegularExpression>
#include <QRgb>
#include <QDir>
#include <QApplication>
#include <QEvent>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMenuBar>
#include <QProgressBar>
#include <QSettings>
#include <QSplitter>
#include <QStatusBar>
#include <QUrl>

#include <spdlog/spdlog.h>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // Title shows show folder name from the moment it's loaded.
    const QString sf = QtXLightsApp::instance().showFolder();
    setWindowTitle(sf.isEmpty() ? "xLights Qt"
                                : "xLights Qt  [" + QDir(sf).dirName() + "]");
    setMinimumSize(1280, 800);
    resize(1440, 900);

    QtXLightsApp::instance();

    // ── Toolbars ──────────────────────────────────────────────────────────
    _playback  = new PlaybackController(this);
    _transport = new TransportToolBar(_playback, this);
    addToolBar(Qt::TopToolBarArea, _transport);

    addToolBarBreak(Qt::TopToolBarArea);

    _effectsBar = new EffectToolBar(this);
    addToolBar(Qt::TopToolBarArea, _effectsBar);

    // ── Render bridge ─────────────────────────────────────────────────────
    _renderBridge = new QtRenderBridge(this);
    _renderBridge->setMetadataDir(QtXLightsApp::instance().effectMetadataDir());
    _renderBridge->setShowFolder(QtXLightsApp::instance().showFolder());

    setupMenuBar();
    setupStatusBar();

    // ── Central layout ────────────────────────────────────────────────────
    _splitter = new QSplitter(Qt::Horizontal, this);
    _splitter->setChildrenCollapsible(false);

    // Left pane: sequencer on top, single-model preview below, house preview at bottom.
    _centerV   = new QSplitter(Qt::Vertical, _splitter);
    _centerV->setChildrenCollapsible(false);
    _sequencer    = new SequencerWidget(_centerV);
    _preview      = new PreviewWidget(_centerV);
    _housePreview = new HousePreviewWidget(_centerV);
    _centerV->addWidget(_sequencer);
    _centerV->addWidget(_preview);
    _centerV->addWidget(_housePreview);
    _centerV->setSizes({450, 150, 200});
    _splitter->addWidget(_centerV);

    _effectPanel = new EffectPanelWidget(_splitter);
    _effectPanel->setMinimumWidth(280);
    _splitter->addWidget(_effectPanel);

    _splitter->setSizes({1100, 300});

    QSettings s(QtXLightsApp::settingsFilePath(), QSettings::IniFormat);
    if (s.contains("window/splitter"))  _splitter->restoreState(s.value("window/splitter").toByteArray());
    if (s.contains("window/centerV"))   _centerV->restoreState(s.value("window/centerV").toByteArray());
    if (s.contains("window/geometry"))  restoreGeometry(s.value("window/geometry").toByteArray());

    setCentralWidget(_splitter);

    // ── Connections ───────────────────────────────────────────────────────

    // Transport: ⚡ Render Selected → composite all layers for the selected model.
    //            🏠 Render House  → render all models and update the house preview.
    connect(_transport, &TransportToolBar::renderAllRequested,
            this, &MainWindow::renderAllLayers);
    connect(_transport, &TransportToolBar::renderHouseRequested,
            this, &MainWindow::renderAllModels);

    // House preview model click → select that model for single-model preview.
    connect(_housePreview, &ModelLayoutCanvas::modelClicked,
            this, [this](const QString& name) {
        _currentModel = name;
        renderAllLayers();
        statusBar()->showMessage("Selected: " + name, 2000);
    });

    // Effect toolbar click
    connect(_effectsBar, &EffectToolBar::effectSelected,
            this, &MainWindow::onEffectSelected);

    // Sequencer block click
    connect(_sequencer, &SequencerWidget::blockSelected,
            this, &MainWindow::onBlockSelected);

    // Effect settings/palette change → composite-render all layers when a model
    // is selected, or single-effect render when editing from the effect toolbar.
    connect(_effectPanel, &EffectPanelWidget::settingsChanged,
            this, [this](const QString& name, const QVariantMap& settings) {
        Q_UNUSED(settings)
        if (!_currentModel.isEmpty())
            renderAllLayers();
        else
            triggerRender(name);
        statusBar()->showMessage(
            QString("%1 — settings changed").arg(name), 1500);
    });
    connect(_effectPanel, &EffectPanelWidget::paletteChanged,
            this, [this](const QString& name, const QList<QColor>&) {
        if (!_currentModel.isEmpty())
            renderAllLayers();
        else
            triggerRender(name);
    });

    // Render result → buffer canvas + model preview.
    // The house preview is NOT touched here — it is owned exclusively by
    // renderAllModels() so all models in the yard always come from the same
    // render pass at the same frame position.
    connect(_renderBridge, &QtRenderBridge::frameReady,
            this, [this](const QtEffectRenderer::Result& r) {
        _effectPanel->setBufferPixels(r.w, r.h, r.pixels);
        const QtSequenceInfo& seq = QtXLightsApp::instance().currentSequence();
        if (seq.isGroup(_currentModel)) {
            // Groups distribute to the model preview; house is still owned by renderAllModels.
            distributeGroupToMembers(_currentModel, r.pixels, /*updateModelPreview=*/true);
            return;
        }
        const QtModelInfo mi = seq.modelInfo(_currentModel);
        _preview->setResult(r, mi.nodePositions);
    });

    // Playback → advance the playhead.
    // • Selected model preview: rendered live every frame via renderAllLayers()
    //   so the user always sees the correct effect output for the current position.
    // • House preview: reads from the pre-built frame cache when available
    //   (built by Render All); otherwise stays at the last Render All snapshot.
    connect(_playback, &PlaybackController::positionChanged, this, [this](int ms) {
        const int fps      = qMax(1, _sequencer->model()->fps());
        const int curFrame = ms * fps / 1000;
        _sequencer->setPlayhead(curFrame);

        if (_playback->isPlaying()) {
            // Always update the selected model so its preview tracks playback.
            if (!_currentModel.isEmpty())
                renderAllLayers();

            // House preview from cache if pre-built; otherwise it stays frozen
            // at whatever renderAllModels() last produced.
            if (_renderCacheValid)
                displayCacheFrame(curFrame);
        }
    });

    // On stop/pause: do a full house render at the stopped position so the
    // preview accurately reflects the frame the user landed on.
    connect(_playback, &PlaybackController::playingChanged, this, [this](bool playing) {
        if (!playing && QtXLightsApp::instance().currentSequence().isValid())
            renderAllModels();
    });

    // Model change → keep duration in sync
    connect(_sequencer->model(), &SequencerModel::modelChanged,
            this, &MainWindow::syncDuration);

    populateEffectBar();
    syncDuration();

    // ── Info windows (created once, shown on demand) ──────────────────────
    _modelInfoWin     = new ModelInfoWindow(this);
    _layoutWin        = new LayoutWindow(this);
    _controllerVizWin = new ControllerVisualizerWindow(this);

    connect(_layoutWin, &LayoutWindow::visualizerRequested,
            this, [this](const QString& name) {
        _controllerVizWin->openForController(name);
    });
    connect(_layoutWin, &LayoutWindow::uploadRequested,
            this, [this](const QString& name) {
        Q_UNUSED(name)
        statusBar()->showMessage("Upload not yet implemented", 3000);
    });

    // If a show folder was restored from settings, load the layout immediately
    // so the layout window is populated before any sequence is opened.
    if (!QtXLightsApp::instance().showFolder().isEmpty()) {
        _renderBridge->setShowFolder(QtXLightsApp::instance().showFolder());
        _layoutWin->refresh();
    }
}

// ── Render helper ─────────────────────────────────────────────────────────────

void MainWindow::triggerRender(const QString& effectName, double progress,
                               const QString& rawSettings, const QString& rawPalette) {
    QtEffectRenderer::Request req;
    req.effectName  = effectName;
    req.modelName   = _currentModel;   // lets renderCore() use the real model topology
    req.settings    = _effectPanel->currentSettings();
    req.palette     = _effectPanel->palette();
    req.progress    = progress;
    req.rawSettings = rawSettings;
    req.rawPalette  = rawPalette;

    // Use real model buffer dimensions when we know the selected model.
    const QtModelInfo mi =
        QtXLightsApp::instance().currentSequence().modelInfo(_currentModel);
    req.bufferW = mi.bufferW;
    req.bufferH = mi.bufferH;

    _renderBridge->request(req);
}

// ── Composite render at playhead ──────────────────────────────────────────────

static QString layerMethodFromSettings(const QString& raw) {
    for (const QString& part : raw.split(QRegularExpression("[,\n]"), Qt::SkipEmptyParts)) {
        const int eq = part.indexOf('=');
        if (eq < 0) continue;
        const QString key = part.left(eq).trimmed();
        if (key == "T_CHOICE_LayerMethod" || key == "CHOICE_LayerMethod")
            return part.mid(eq + 1).trimmed();
    }
    return QStringLiteral("Normal");
}

static QColor blendLayers(const QColor& dst, const QColor& src, const QString& method) {
    if (method == "Effect 1")
        return dst;
    if (method == "Effect 2")
        return src;
    if (method == "Additive")
        return QColor(qMin(255, dst.red()   + src.red()),
                      qMin(255, dst.green() + src.green()),
                      qMin(255, dst.blue()  + src.blue()));
    if (method == "Average")
        return QColor((dst.red()   + src.red())   / 2,
                      (dst.green() + src.green()) / 2,
                      (dst.blue()  + src.blue())  / 2);
    if (method == "Max")
        return QColor(qMax(dst.red(),   src.red()),
                      qMax(dst.green(), src.green()),
                      qMax(dst.blue(),  src.blue()));
    if (method == "Min")
        return QColor(qMin(dst.red(),   src.red()),
                      qMin(dst.green(), src.green()),
                      qMin(dst.blue(),  src.blue()));
    // Normal / Layered / default: src overwrites dst; black src = transparent.
    return (src.red() || src.green() || src.blue()) ? src : dst;
}

// ── Core per-model render (shared by single-model and house preview) ──────────

QList<QColor> MainWindow::renderModelLayers(const QString& modelName) {
    const QtModelInfo mi =
        QtXLightsApp::instance().currentSequence().modelInfo(modelName);
    const int bufW = mi.bufferW;
    const int bufH = mi.bufferH;
    const int n    = bufW * bufH;
    if (n == 0) return {};

    const int  fps      = qMax(1, _sequencer->model()->fps());
    const int  frameMs  = 1000 / fps;
    const int  curMs    = _playback->positionMs();
    const int  curFrame = curMs / frameMs;

    // Collect active blocks for all layer rows belonging to this model.
    struct LayerBlock { int layerIndex; const EffectBlock* blk; };
    QList<LayerBlock> active;
    for (int r = 0; r < _sequencer->model()->rowCount(); ++r) {
        const SequencerRow& row = _sequencer->model()->row(r);
        if (row.modelName != modelName) continue;
        for (const EffectBlock& blk : row.blocks) {
            if (curFrame >= blk.startFrame && curFrame < blk.endFrame) {
                active.append({row.layerIndex, &blk});
                break;
            }
        }
    }

    if (active.isEmpty()) return {};

    QList<QColor> composite(n, Qt::black);

    for (const auto& lb : active) {
        const EffectBlock& blk = *lb.blk;

        double progress = 0.5;
        if (blk.endFrame > blk.startFrame)
            progress = double(curFrame - blk.startFrame) /
                       double(blk.endFrame - blk.startFrame);

        double sbLeft = 0, sbBottom = 0, sbRight = 100, sbTop = 100;
        parseSubBuffer(blk.settings, sbLeft, sbBottom, sbRight, sbTop);
        const int subLeft_px   = qBound(0, int(sbLeft   * bufW / 100.0), bufW);
        const int subBottom_px = qBound(0, int(sbBottom * bufH / 100.0), bufH);
        const int subRight_px  = qBound(0, int(sbRight  * bufW / 100.0 + 0.5), bufW);
        const int subTop_px    = qBound(0, int(sbTop    * bufH / 100.0 + 0.5), bufH);
        const int subW = qMax(1, subRight_px  - subLeft_px);
        const int subH = qMax(1, subTop_px    - subBottom_px);
        const bool fullBuf = (subLeft_px == 0 && subBottom_px == 0 &&
                              subRight_px == bufW && subTop_px == bufH);

        QtEffectRenderer::Request req;
        req.effectName  = blk.effectName;
        req.modelName   = modelName;
        req.settings    = _effectPanel->currentSettings();
        req.palette     = _effectPanel->palette();
        req.progress    = qBound(0.0, progress, 1.0);
        req.startMs     = blk.startFrame * frameMs;
        req.endMs       = blk.endFrame   * frameMs;
        req.curMs       = curMs;
        req.frameMs     = frameMs;
        req.rawSettings = blk.settings;
        req.rawPalette  = blk.palette;
        req.bufferW     = fullBuf ? bufW : subW;
        req.bufferH     = fullBuf ? bufH : subH;

        const QtEffectRenderer::Result r = _renderBridge->renderNow(req);
        if (!r.isValid()) continue;

        const QString method = layerMethodFromSettings(blk.settings);
        if (fullBuf) {
            for (int i = 0; i < n && i < r.pixels.size(); ++i)
                composite[i] = blendLayers(composite[i], r.pixels[i], method);
        } else {
            for (int sy = 0; sy < subH; ++sy) {
                const int fy = subBottom_px + sy;
                if (fy >= bufH) break;
                for (int sx = 0; sx < subW; ++sx) {
                    const int fx = subLeft_px + sx;
                    if (fx >= bufW) break;
                    const int di = fy * bufW + fx;
                    composite[di] = blendLayers(composite[di],
                                                r.pixels[sy * subW + sx], method);
                }
            }
        }
    }

    return composite;
}

void MainWindow::renderAllLayers() {
    if (_currentModel.isEmpty()) return;

    const QtModelInfo mi =
        QtXLightsApp::instance().currentSequence().modelInfo(_currentModel);
    const int bufW = mi.bufferW;
    const int bufH = mi.bufferH;
    const int n    = bufW * bufH;
    if (n == 0) return;

    _renderProgress->setRange(0, 1);
    _renderProgress->setValue(0);
    _renderProgress->setFormat("Rendering…");
    _renderProgress->show();
    _renderProgress->repaint();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    const QList<QColor> composite = renderModelLayers(_currentModel);
    _renderProgress->hide();

    if (composite.isEmpty()) {
        // No active sequence blocks at this frame — fall back to a single-effect
        // preview using the panel's current effect and the model's buffer dimensions.
        const QString eff = _effectPanel->currentEffect();
        if (!eff.isEmpty())
            triggerRender(eff);
        return;
    }

    // Always show the raw effect buffer in the buffer widget.
    _effectPanel->setBufferPixels(bufW, bufH, composite);

    const QtSequenceInfo& seq = QtXLightsApp::instance().currentSequence();
    if (seq.isGroup(_currentModel)) {
        distributeGroupToMembers(_currentModel, composite, /*updateModelPreview=*/true);
        return;
    }

    // Individual models: update both the model preview and the house preview
    // node colours for this model only — no need to re-render every model.
    QtEffectRenderer::Result result;
    result.w = bufW; result.h = bufH; result.pixels = composite;
    _preview->setResult(result, mi.nodePositions);
    _housePreview->setModelPixels(_currentModel, composite);
}

void MainWindow::renderAllModels() {
    if (_renderAllInProgress) return;
    _renderAllInProgress = true;

    const QtSequenceInfo& seq = QtXLightsApp::instance().currentSequence();
    if (!seq.isValid()) { _renderAllInProgress = false; return; }

    // All models and groups at the current playhead — groups after individual
    // models so their composites overwrite member-model pixels.
    QStringList modelNames;
    for (auto it = seq.models.cbegin(); it != seq.models.cend(); ++it)
        modelNames.append(it.key());
    for (auto it = seq.groups.cbegin(); it != seq.groups.cend(); ++it)
        modelNames.append(it.key());

    const int total = modelNames.size();
    if (total == 0) { _renderAllInProgress = false; return; }

    if (_renderDetailDlg) {
        _renderDetailDlg->beginRender(modelNames);
        _renderDetailDlg->show();
        _renderDetailDlg->raise();
    }

    _renderProgress->setRange(0, total);
    _renderProgress->setValue(0);
    _renderProgress->setFormat("%v / %m");
    _renderProgress->setTextVisible(true);
    _renderProgress->show();
    _renderProgress->repaint();
    statusBar()->repaint();
    QApplication::processEvents();

    int rendered = 0;
    for (int i = 0; i < total; ++i) {
        const QString&    mn = modelNames[i];
        const QtModelInfo mi = seq.modelInfo(mn);
        const int         n  = mi.bufferW * mi.bufferH;

        if (_renderDetailDlg)
            _renderDetailDlg->setModelStatus(i, RenderDetailDialog::Rendering);

        const QList<QColor> composite = renderModelLayers(mn);

        if (!composite.isEmpty()) {
            if (seq.isGroup(mn))
                distributeGroupToMembers(mn, composite, /*updateModelPreview=*/false);
            else
                _housePreview->setModelPixels(mn, composite);
            if (_renderDetailDlg)
                _renderDetailDlg->setModelStatus(i, RenderDetailDialog::Done);
            ++rendered;
        } else {
            if (n > 0 && !seq.isGroup(mn))
                _housePreview->setModelPixels(mn, QList<QColor>(n, Qt::black));
            if (_renderDetailDlg)
                _renderDetailDlg->setModelStatus(i, RenderDetailDialog::NoEffects);
        }

        _renderProgress->setValue(i + 1);
        statusBar()->showMessage(
            QString("Rendering %1  (%2/%3)").arg(mn).arg(i + 1).arg(total), 0);
        QApplication::processEvents();
    }

    if (_renderDetailDlg)
        _renderDetailDlg->endRender(rendered, total);

    _renderProgress->hide();
    _renderAllInProgress = false;
    statusBar()->showMessage(
        QString("House render — %1 / %2 had active effects").arg(rendered).arg(total), 4000);
}

// ── Cache helpers ─────────────────────────────────────────────────────────────

void MainWindow::displayCacheFrame(int frame) {
    if (!_renderCacheValid) return;
    const QtSequenceInfo& seq = QtXLightsApp::instance().currentSequence();

    for (auto it = _renderCache.cbegin(); it != _renderCache.cend(); ++it) {
        const QString& mn = it.key();
        if (frame >= it.value().size()) continue;
        const QVector<QRgb>& cached = it.value()[frame];

        if (cached.isEmpty()) {
            // No effect active at this frame → go dark.
            const QtModelInfo mi = seq.modelInfo(mn);
            const int n = mi.bufferW * mi.bufferH;
            if (n > 0)
                _housePreview->setModelPixels(mn, QList<QColor>(n, Qt::black));
        } else {
            QList<QColor> pixels;
            pixels.reserve(cached.size());
            for (QRgb rgb : cached)
                pixels.append(QColor::fromRgb(rgb));
            _housePreview->setModelPixels(mn, pixels);
        }
    }

    // Update the model preview for the currently selected model.
    if (!_currentModel.isEmpty() && _renderCache.contains(_currentModel)) {
        const QVector<QRgb>& cached = _renderCache[_currentModel].value(frame);
        if (!cached.isEmpty()) {
            const QtModelInfo mi = seq.modelInfo(_currentModel);
            QList<QColor> pixels;
            pixels.reserve(cached.size());
            for (QRgb rgb : cached)
                pixels.append(QColor::fromRgb(rgb));

            if (seq.isGroup(_currentModel)) {
                distributeGroupToMembers(_currentModel, pixels, /*updateModelPreview=*/true);
            } else {
                QtEffectRenderer::Result r;
                r.w = mi.bufferW; r.h = mi.bufferH; r.pixels = pixels;
                _preview->setResult(r, mi.nodePositions);
            }
        }
    }
}

void MainWindow::cacheGroupDistribution(const QString& groupName,
                                        const QList<QColor>& groupPixels,
                                        int frame) {
    const QtSequenceInfo& seq = QtXLightsApp::instance().currentSequence();
    auto git = seq.groups.find(groupName);
    if (git == seq.groups.end()) return;

    const QtModelGroupInfo& gi = *git;
    const double rangeX = gi.maxX - gi.minX;
    const double rangeY = gi.maxY - gi.minY;
    if (rangeX <= 0 && rangeY <= 0) return;

    auto normX = [&](double x) { return rangeX > 0 ? qBound(0.0,(x-gi.minX)/rangeX,1.0) : 0.5; };
    auto normY = [&](double y) { return rangeY > 0 ? qBound(0.0,(y-gi.minY)/rangeY,1.0) : 0.5; };
    auto sample = [&](double nx, double ny) -> QRgb {
        const int bx = qBound(0, int(nx*(gi.bufferW-1)), gi.bufferW-1);
        const int by = qBound(0, (gi.bufferH-1)-int(ny*(gi.bufferH-1)), gi.bufferH-1);
        const int bi = by*gi.bufferW + bx;
        return bi < groupPixels.size() ? groupPixels[bi].rgb() : qRgb(0,0,0);
    };

    for (const QString& mn : gi.modelNames) {
        if (!_renderCache.contains(mn)) continue;
        auto mit = seq.models.find(mn);
        if (mit == seq.models.end()) continue;
        const QtModelInfo& mi = *mit;
        const int nc = mi.globalPositions.size();
        if (nc == 0) continue;

        QVector<QRgb> pixels(nc);
        for (int i = 0; i < nc; ++i) {
            const QPointF& gp = mi.globalPositions[i];
            pixels[i] = sample(normX(gp.x()), normY(gp.y()));
        }
        if (frame < _renderCache[mn].size())
            _renderCache[mn][frame] = std::move(pixels);
    }
}

// ── Group pixel distribution ──────────────────────────────────────────────────
// After a group effect renders at (gi.bufferW × gi.bufferH), map each member
// model's nodes to the corresponding group-buffer pixel using the node's global
// position within the group bounding box.
//
// Coordinate conventions:
//   • globalPositions: layout space, Y increases downward (screen convention).
//   • Group bounding box (minX/Y .. maxX/Y): same layout space.
//   • ny = (gp.y - minY) / rangeY: 0=top of group, 1=bottom.
//   • xLights render buffer: y=0 is the BOTTOM row of the effect.
//     So to map ny→buffer_y: by = (bufH-1) - int(ny*(bufH-1)).

void MainWindow::distributeGroupToMembers(const QString& groupName,
                                          const QList<QColor>& groupPixels,
                                          bool updateModelPreview) {
    const QtSequenceInfo& seq = QtXLightsApp::instance().currentSequence();
    auto git = seq.groups.find(groupName);
    if (git == seq.groups.end()) return;

    const QtModelGroupInfo& gi = *git;
    const double rangeX = gi.maxX - gi.minX;
    const double rangeY = gi.maxY - gi.minY;
    if (rangeX <= 0 && rangeY <= 0) return;   // truly empty — both axes degenerate

    // Normalise a layout coordinate to [0,1] within the group bounding box.
    // When one axis is degenerate (rangeX or rangeY == 0, e.g. all Tree-360 models
    // at the same worldPosY), return 0.5 so nodes land at the centre of that axis.
    auto normX = [&](double x) -> double {
        return rangeX > 0 ? qBound(0.0, (x - gi.minX) / rangeX, 1.0) : 0.5;
    };
    auto normY = [&](double y) -> double {
        return rangeY > 0 ? qBound(0.0, (y - gi.minY) / rangeY, 1.0) : 0.5;
    };
    // Sample the group render buffer at a normalised (nx, ny) position.
    // xLights buffer: row 0 = physical bottom; flip ny so 0=top → row bufH-1.
    auto sampleGroup = [&](double nx, double ny) -> QColor {
        const int bx = qBound(0, int(nx * (gi.bufferW - 1)), gi.bufferW - 1);
        const int by = qBound(0, (gi.bufferH - 1) - int(ny * (gi.bufferH - 1)), gi.bufferH - 1);
        const int bi = by * gi.bufferW + bx;
        return bi < groupPixels.size() ? groupPixels[bi] : Qt::black;
    };

    // ── Per-node distribution for house preview ───────────────────────────
    for (const QString& mn : gi.modelNames) {
        auto mit = seq.models.find(mn);
        if (mit == seq.models.end()) continue;
        const QtModelInfo& mi = *mit;
        const int nc = mi.globalPositions.size();
        if (nc == 0) continue;

        QList<QColor> pixels(nc, Qt::black);
        for (int i = 0; i < nc; ++i) {
            const QPointF& gp = mi.globalPositions[i];
            pixels[i] = sampleGroup(normX(gp.x()), normY(gp.y()));
        }
        _housePreview->setModelPixels(mn, pixels);
    }

    // ── Physical-layout model preview ────────────────────────────────────
    // Place each node at its actual normalised position within the group bounding
    // box using globalPositions directly.  The previous approach (worldPos-centre
    // + local-shape × dispScale) collapsed to a horizontal line whenever lp.y()
    // was constant (e.g. Tree 360 top-down projection where all nodes have Z=0).
    if (updateModelPreview) {
        QList<QPointF> positions;
        QList<QColor>  colors;

        for (const QString& mn : gi.modelNames) {
            auto mit = seq.models.find(mn);
            if (mit == seq.models.end()) continue;
            const QtModelInfo& mi = *mit;
            const int nc = mi.globalPositions.size();
            if (nc == 0) continue;

            for (int i = 0; i < nc; ++i) {
                const QPointF& gp = mi.globalPositions[i];
                const double nx = normX(gp.x());
                const double ny = normY(gp.y());
                positions.append({nx, ny});
                colors.append(sampleGroup(nx, ny));
            }
        }

        if (!positions.isEmpty()) {
            QtEffectRenderer::Result gv;
            gv.w = colors.size();
            gv.h = 1;
            gv.pixels = colors;
            _preview->setResult(gv, positions);
        }
    }
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void MainWindow::syncDuration() {
    int fps = qMax(1, _sequencer->model()->fps());
    int dur = _sequencer->model()->totalFrames() * 1000 / fps;
    _playback->setDuration(dur);
    _transport->setDuration(dur);
}

void MainWindow::populateEffectBar() {
    QStringList names;
    for (const auto& e : QtXLightsApp::instance().effects())
        names.append(e.name);
    _effectsBar->populate(names);
    statusBar()->showMessage(
        QString("%1 effects loaded  |  Open a sequence to begin")
            .arg(names.size()),
        0);
}


void MainWindow::onEffectSelected(const QString& name) {
    if (name.isEmpty()) return;
    _effectPanel->showEffect(name);
    _preview->showEffect(name);     // mock placeholder until bridge fires (40ms)
}

void MainWindow::onBlockSelected(int row, int block) {
    const SequencerRow& seqRow = _sequencer->model()->row(row);
    const EffectBlock&  blk    = seqRow.blocks.at(block);
    _currentModel = seqRow.modelName;  // used by renderAllLayers for dim/position lookup

    // If this is a group, highlight its member models in the house preview.
    {
        const QtSequenceInfo& sq = QtXLightsApp::instance().currentSequence();
        if (sq.isGroup(_currentModel)) {
            const QtModelGroupInfo& gi = sq.groups[_currentModel];
            _housePreview->setGroupHighlight(_currentModel, gi.modelNames);
        } else {
            _housePreview->setGroupHighlight({}, {});   // clear group highlight
        }
    }

    // Show effect panel pre-populated with the block's xsq settings.
    _effectPanel->showEffect(blk.effectName, blk.settings);
    _effectPanel->loadBlockPalette(blk.palette);

    // If the playhead is not inside the clicked block, move it to the block's
    // midpoint so renderAllLayers() finds active blocks at the current frame.
    {
        const int ph = _sequencer->model()->playheadFrame();
        if (ph < blk.startFrame || ph >= blk.endFrame) {
            const int mid   = (blk.startFrame + blk.endFrame) / 2;
            const int fps   = qMax(1, _sequencer->model()->fps());
            const int midMs = mid * 1000 / fps;
            _playback->seek(midMs);
            _sequencer->setPlayhead(mid);
        }
    }

    // Composite all layers of this model and update its house preview node colours.
    // Other models keep their last rendered state — no full re-render needed.
    renderAllLayers();

    statusBar()->showMessage(
        QString("Row %1 | %2 | frames %3–%4")
            .arg(seqRow.name)
            .arg(blk.effectName)
            .arg(blk.startFrame)
            .arg(blk.endFrame),
        3000);
}

// ── Menu + statusbar ──────────────────────────────────────────────────────────

void MainWindow::setupMenuBar() {
    auto* file    = menuBar()->addMenu("&File");
    auto* openAct = file->addAction("&Open Sequence…", QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, [this]() {
        QString path = QFileDialog::getOpenFileName(
            this, "Open Sequence",
            QtXLightsApp::instance().showFolder(),
            "xLights Sequence (*.xsq)");
        if (path.isEmpty()) return;
        _playback->stop();
        statusBar()->showMessage("Loading…");
        const QtSequenceInfo info =
            QtXLightsApp::instance().openSequence(path, _sequencer->model());
        if (info.isValid()) {
            QString sfName = QtXLightsApp::instance().showFolder();
            _renderBridge->setShowFolder(sfName);
            _modelInfoWin->refresh();
            _layoutWin->refresh();
            _housePreview->loadLayout(QtXLightsApp::instance().currentSequence());

            // Resolve and load audio — try show folder first, then xsq directory.
            QString audioPath;
            if (!info.mediaFile.isEmpty()) {
                // Absolute path as-is, or resolve relative to show / xsq dir.
                if (QFileInfo(info.mediaFile).isAbsolute() &&
                    QFileInfo::exists(info.mediaFile)) {
                    audioPath = info.mediaFile;
                } else {
                    for (const QString& base : {sfName, QFileInfo(path).absolutePath()}) {
                        QString candidate = QDir(base).filePath(info.mediaFile);
                        if (QFileInfo::exists(candidate)) { audioPath = candidate; break; }
                    }
                }
            }
            _playback->setMediaFile(audioPath);

            QString title  = "xLights Qt — " + info.title;
            if (!sfName.isEmpty())
                title += "  [" + QDir(sfName).dirName() + "]";
            setWindowTitle(title);
            statusBar()->showMessage(
                QString("%1  |  %2 models  |  %3 fps  |  %4 s")
                    .arg(info.title)
                    .arg(_sequencer->model()->rowCount())
                    .arg(info.fps())
                    .arg(info.durationMs / 1000),
                6000);
        } else {
            statusBar()->showMessage("Failed to load sequence.", 4000);
        }
    });
    file->addSeparator();
    auto* showFolderAct = file->addAction("Set Show &Folder…");
    connect(showFolderAct, &QAction::triggered, this, [this]() {
        const QString current = QtXLightsApp::instance().showFolder();
        const QString path = QFileDialog::getExistingDirectory(
            this, "Set Show Folder", current.isEmpty() ? QDir::homePath() : current);
        if (path.isEmpty()) return;
        QtXLightsApp::instance().setShowFolder(path);  // also persists to INI
        _renderBridge->setShowFolder(path);            // reload xlights_networks.xml
        _layoutWin->refresh();                         // load models from show file
        _playback->setMediaFile({});                   // clear audio until sequence reloaded
        statusBar()->showMessage("Show folder: " + path, 5000);
        // Refresh title bar if a sequence is loaded
        const auto& seq = QtXLightsApp::instance().currentSequence();
        if (seq.isValid())
            setWindowTitle("xLights Qt — " + seq.title
                           + "  [" + QDir(path).dirName() + "]");
    });

    file->addSeparator();
    auto* exportAct = file->addAction("Export &FSEQ…", QKeySequence("Ctrl+E"));
    connect(exportAct, &QAction::triggered, this, [this]() {
        if (_sequencer->model()->totalFrames() == 0) {
            return;   // nothing to export
        }
        ExportDialog dlg(_sequencer->model(),
                         QtXLightsApp::instance().currentSequence(),
                         _renderBridge,
                         this);
        dlg.exec();
    });
    file->addSeparator();
    file->addAction("E&xit", qApp, &QApplication::quit, QKeySequence::Quit);

    // ── View menu ─────────────────────────────────────────────────────────
    auto* view = menuBar()->addMenu("&View");

    auto* layoutAct = view->addAction("&Layout…", QKeySequence("Ctrl+L"));
    connect(layoutAct, &QAction::triggered, this, [this]() {
        _layoutWin->show();
        _layoutWin->raise();
        _layoutWin->activateWindow();
    });
    view->addSeparator();
    auto* logAct = view->addAction("View &Log…", QKeySequence("Ctrl+L"));
    connect(logAct, &QAction::triggered, this, []() {
        spdlog::default_logger()->flush();
        QDesktopServices::openUrl(QUrl::fromLocalFile(QtXLightsApp::logFilePath()));
    });

    menuBar()->addMenu("&Help")->addAction("&About xLights Qt…");
}

void MainWindow::setupStatusBar() {
    statusBar()->showMessage("Initialising…");

    _renderProgress = new QProgressBar(this);
    _renderProgress->setRange(0, 100);
    _renderProgress->setFixedWidth(200);
    _renderProgress->setFixedHeight(16);
    _renderProgress->setTextVisible(true);
    _renderProgress->setFormat("Rendering… %p%");
    _renderProgress->setToolTip("Double-click for detailed model progress");
    _renderProgress->hide();
    _renderProgress->installEventFilter(this);
    statusBar()->addPermanentWidget(_renderProgress);

    // Create the detail dialog once; it stays hidden until the user double-clicks.
    _renderDetailDlg = new RenderDetailDialog(this);
}

bool MainWindow::eventFilter(QObject* watched, QEvent* ev) {
    if (watched == _renderProgress && ev->type() == QEvent::MouseButtonDblClick) {
        if (_renderDetailDlg) {
            _renderDetailDlg->show();
            _renderDetailDlg->raise();
            _renderDetailDlg->activateWindow();
        }
        return true;
    }
    return QMainWindow::eventFilter(watched, ev);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    _playback->stop();
    QSettings s(QtXLightsApp::settingsFilePath(), QSettings::IniFormat);
    s.setValue("window/geometry", saveGeometry());
    s.setValue("window/splitter", _splitter->saveState());
    s.setValue("window/centerV",  _centerV->saveState());
    // Persist show folder on shutdown (also saved immediately on change, but be explicit).
    const QString sf = QtXLightsApp::instance().showFolder();
    if (!sf.isEmpty())
        s.setValue("showFolder", sf);
    s.sync();   // force flush to disk before the process exits
    QMainWindow::closeEvent(event);
}
