#include "MainWindow.h"
#include "PlaybackController.h"
#include "QtXLightsApp.h"
#include "TransportToolBar.h"
#include "../Info/ControllerInfoWindow.h"
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
#include "../Preview/PreviewWidget.h"
#include "../Sequencer/SequencerModel.h"
#include "../Sequencer/SequencerWidget.h"

#include <QAction>
#include <QDir>
#include <QApplication>
#include <QEvent>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMenuBar>
#include <QSettings>
#include <QSplitter>
#include <QProgressBar>
#include <QStatusBar>

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
    connect(_housePreview, &HousePreviewWidget::modelClicked,
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

    // Render result → buffer widget + preview (with physical node layout)
    connect(_renderBridge, &QtRenderBridge::frameReady,
            this, [this](const QtEffectRenderer::Result& r) {
        _effectPanel->setBufferPixels(r.w, r.h, r.pixels);
        const QtModelInfo mi =
            QtXLightsApp::instance().currentSequence().modelInfo(_currentModel);
        _preview->setResult(r, mi.nodePositions);
    });

    // Playback → playhead + live preview update (only when frame changes)
    connect(_playback, &PlaybackController::positionChanged, this, [this](int ms) {
        const int fps      = qMax(1, _sequencer->model()->fps());
        const int curFrame = ms * fps / 1000;
        _sequencer->setPlayhead(curFrame);

        if (_playback->isPlaying() && curFrame != _lastRenderedFrame) {
            _lastRenderedFrame = curFrame;

            // Selected model: full src-core composite render every frame.
            if (!_currentModel.isEmpty())
                renderAllLayers();

            // House preview: software-renderer batch, cycling through all models.
            tickHousePreview(curFrame);
        }
    });

    // On stop/pause: do one accurate src-core house render at the stopped frame.
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
    _modelInfoWin      = new ModelInfoWindow(this);
    _controllerInfoWin = new ControllerInfoWindow(this);
}

// ── Render helper ─────────────────────────────────────────────────────────────

void MainWindow::triggerRender(const QString& effectName, double progress,
                               const QString& rawSettings, const QString& rawPalette) {
    QtEffectRenderer::Request req;
    req.effectName  = effectName;
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
        req.settings    = _effectPanel->currentSettings();
        req.palette     = _effectPanel->palette();
        req.progress    = qBound(0.0, progress, 1.0);
        req.rawSettings = blk.settings;
        req.rawPalette  = blk.palette;
        req.bufferW     = fullBuf ? bufW : subW;
        req.bufferH     = fullBuf ? bufH : subH;

        const QtEffectRenderer::Result r = _renderBridge->renderNow(req);
        if (!r.isValid()) continue;

        if (fullBuf) {
            for (int i = 0; i < n && i < r.pixels.size(); ++i) {
                const QColor& px = r.pixels[i];
                if (px.red() || px.green() || px.blue())
                    composite[i] = px;
            }
        } else {
            for (int sy = 0; sy < subH; ++sy) {
                const int fy = subBottom_px + sy;
                if (fy >= bufH) break;
                for (int sx = 0; sx < subW; ++sx) {
                    const int fx = subLeft_px + sx;
                    if (fx >= bufW) break;
                    const QColor& px = r.pixels[sy * subW + sx];
                    if (px.red() || px.green() || px.blue())
                        composite[fy * bufW + fx] = px;
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
        // Groups: distribute pixels to member models and show their physical
        // layout in the model preview (house-layout view of the group's members).
        distributeGroupToMembers(_currentModel, composite, /*updateModelPreview=*/true);
        return;
    }

    // Individual models: show the effect buffer in the model preview.
    QtEffectRenderer::Result result;
    result.w = bufW; result.h = bufH; result.pixels = composite;
    _preview->setResult(result, mi.nodePositions);
    _housePreview->setModelPixels(_currentModel, composite);
}

void MainWindow::renderAllModels() {
    // Guard against re-entrant calls (possible because we process all events below).
    if (_renderAllInProgress) return;
    _renderAllInProgress = true;

    const QtSequenceInfo& seq = QtXLightsApp::instance().currentSequence();
    if (!seq.isValid()) { _renderAllInProgress = false; return; }

    // Use ALL models from the show file (seq.models), not just the subset that
    // have sequencer rows.  Models with no active blocks at the current frame
    // are explicitly set to black so the house preview reflects the full show.
    QStringList modelNames;
    for (auto it = seq.models.cbegin(); it != seq.models.cend(); ++it)
        modelNames.append(it.key());

    const int total = modelNames.size();
    if (total == 0) return;

    // Initialise and auto-show the detail dialog so the user can see progress
    // without needing to double-click during the render.
    if (_renderDetailDlg) {
        _renderDetailDlg->beginRender(modelNames);
        _renderDetailDlg->show();
        _renderDetailDlg->raise();
    }

    // Force the status-bar progress bar to appear and repaint before the
    // first blocking renderNow() call (Windows batches repaints without this).
    _renderProgress->setRange(0, total);
    _renderProgress->setValue(0);
    _renderProgress->setFormat("%v / %m models");
    _renderProgress->setTextVisible(true);
    _renderProgress->show();
    _renderProgress->repaint();
    statusBar()->repaint();
    // Allow ALL events so the user can interact with the dialog during the render.
    // The _renderAllInProgress guard prevents re-entrant renderAllModels() calls.
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
        // Allow all events so the detail dialog updates and stays interactive.
        QApplication::processEvents();
    }

    if (_renderDetailDlg)
        _renderDetailDlg->endRender(rendered, total);

    _renderProgress->hide();
    _renderAllInProgress = false;
    statusBar()->showMessage(
        QString("House render — %1 / %2 models had active effects").arg(rendered).arg(total), 4000);
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

    const QtModelGroupInfo& gi     = *git;
    const double            rangeX = gi.maxX - gi.minX;
    const double            rangeY = gi.maxY - gi.minY;
    if (rangeX <= 0 || rangeY <= 0) return;

    // ── Per-node distribution for house preview ────────────────────────────
    // For each member model, sample the group buffer at each node's global
    // position and update the house preview accordingly.
    // Model-preview is handled separately below (one dot per model).

    for (const QString& mn : gi.modelNames) {
        // Only process models we know about (skip sub-model paths like "X/Y").
        auto mit = seq.models.find(mn);
        if (mit == seq.models.end()) continue;
        const QtModelInfo& mi = *mit;
        const int nc = mi.globalPositions.size();
        if (nc == 0) continue;

        QList<QColor> pixels(nc, Qt::black);
        for (int i = 0; i < nc; ++i) {
            const QPointF& gp = mi.globalPositions[i];

            // Normalise to [0,1] within the group bounding box (screen Y: 0=top).
            const double nx = qBound(0.0, (gp.x() - gi.minX) / rangeX, 1.0);
            const double ny = qBound(0.0, (gp.y() - gi.minY) / rangeY, 1.0);

            // xLights render buffer has y=0 at the BOTTOM; flip so top of
            // the physical group layout maps to the top of the rendered effect.
            const int bx = qBound(0, int(nx * (gi.bufferW - 1)), gi.bufferW - 1);
            const int by = qBound(0, (gi.bufferH - 1) - int(ny * (gi.bufferH - 1)),
                                  gi.bufferH - 1);
            const int bi = by * gi.bufferW + bx;
            if (bi < groupPixels.size())
                pixels[i] = groupPixels[bi];
        }
        _housePreview->setModelPixels(mn, pixels);
    }

    // ── Per-node positions for the model preview widget ────────────────────
    // Show each member model in its actual physical shape (tree, star, matrix,
    // custom, strand) by collecting all their nodes' global positions, normalised
    // to the group's bounding box.  PreviewWidget's density-based nodeR ensures
    // dots are visible regardless of total node count.
    if (updateModelPreview) {
        QList<QPointF> positions;
        QList<QColor>  colors;

        for (const QString& mn : gi.modelNames) {
            auto mit = seq.models.find(mn);
            if (mit == seq.models.end()) continue;
            const QtModelInfo& mi = *mit;
            if (mi.globalPositions.isEmpty()) continue;

            for (int i = 0; i < mi.globalPositions.size(); ++i) {
                const QPointF& gp = mi.globalPositions[i];

                // Normalise to [0,1] within group bounding box (Y: 0=top of group).
                const double nx = qBound(0.0, (gp.x() - gi.minX) / rangeX, 1.0);
                const double ny = qBound(0.0, (gp.y() - gi.minY) / rangeY, 1.0);

                // Sample group buffer (y-flipped: xLights y=0 is at the bottom).
                const int bx = qBound(0, int(nx * (gi.bufferW - 1)), gi.bufferW - 1);
                const int by = qBound(0, (gi.bufferH - 1) - int(ny * (gi.bufferH - 1)),
                                      gi.bufferH - 1);
                const int bi = by * gi.bufferW + bx;

                positions.append({nx, ny});
                colors.append(bi < groupPixels.size() ? groupPixels[bi] : Qt::black);
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

// ── Real-time house preview ───────────────────────────────────────────────────

QList<QColor> MainWindow::parsePaletteQuick(const QString& rawPalette) {
    QList<QColor> colors;
    if (rawPalette.isEmpty()) return colors;
    QMap<QString, QString> vals;
    for (const QString& part : rawPalette.split(',', Qt::SkipEmptyParts)) {
        const int eq = part.indexOf('=');
        if (eq <= 0) continue;
        vals[part.left(eq).trimmed()] = part.mid(eq + 1).trimmed();
    }
    for (int i = 1; i <= 8; ++i) {
        if (vals.value(QString("C_CHECKBOX_Palette%1").arg(i)) == "1") {
            QColor c(vals.value(QString("C_BUTTON_Palette%1").arg(i)));
            if (c.isValid()) colors.append(c);
        }
    }
    return colors;
}

void MainWindow::tickHousePreview(int curFrame) {
    if (_houseQueue.isEmpty()) return;
    const QtSequenceInfo& seq = QtXLightsApp::instance().currentSequence();
    if (!seq.isValid()) return;

    // Render kHouseModelsPerTick models per tick using the fast software renderer.
    // Cycling ensures every model is refreshed within a full queue pass.
    for (int i = 0; i < kHouseModelsPerTick; ++i) {
        if (_houseQueueIdx >= _houseQueue.size())
            _houseQueueIdx = 0;

        const QString  mn = _houseQueue[_houseQueueIdx++];
        const QtModelInfo mi = seq.modelInfo(mn);
        const int n = mi.bufferW * mi.bufferH;
        if (n == 0) continue;

        // Composite all active layers for this model using the software renderer.
        QList<QColor> composite(n, Qt::black);
        bool anyPixels = false;

        for (int r = 0; r < _sequencer->model()->rowCount(); ++r) {
            const SequencerRow& row = _sequencer->model()->row(r);
            if (row.modelName != mn) continue;
            for (const EffectBlock& blk : row.blocks) {
                if (curFrame < blk.startFrame || curFrame >= blk.endFrame) continue;

                const double progress = blk.endFrame > blk.startFrame
                    ? double(curFrame - blk.startFrame) / double(blk.endFrame - blk.startFrame)
                    : 0.5;

                QtEffectRenderer::Request req;
                req.effectName = blk.effectName;
                req.bufferW    = mi.bufferW;
                req.bufferH    = mi.bufferH;
                req.progress   = qBound(0.0, progress, 1.0);
                req.palette    = parsePaletteQuick(blk.palette);

                const QtEffectRenderer::Result res = QtEffectRenderer::render(req);
                if (res.isValid()) {
                    for (int j = 0; j < n && j < res.pixels.size(); ++j) {
                        const QColor& px = res.pixels[j];
                        if (px.red() || px.green() || px.blue()) {
                            composite[j] = px;
                            anyPixels = true;
                        }
                    }
                }
                break;  // one active block per layer row
            }
        }

        if (anyPixels)
            _housePreview->setModelPixels(mn, composite);
    }
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

    // Composite ALL layers of this model at the current playhead position
    // so sub-buffer bands, marquees, and overlapping effects are shown together.
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
            _lastRenderedFrame = -1;
            QString sfName = QtXLightsApp::instance().showFolder();
            _renderBridge->setShowFolder(sfName);   // load xlights_networks.xml
            _modelInfoWin->refresh();
            _controllerInfoWin->refresh();
            _housePreview->loadLayout(QtXLightsApp::instance().currentSequence());

            // Build the cycling queue for real-time house preview.
            // Use all models from the show file so the house preview includes
            // models that have no effects in this sequence.
            _houseQueue.clear();
            _houseQueueIdx = 0;
            const QtSequenceInfo& loadedSeq = QtXLightsApp::instance().currentSequence();
            for (auto it = loadedSeq.models.cbegin(); it != loadedSeq.models.cend(); ++it)
                _houseQueue.append(it.key());

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
    auto* modelsAct = view->addAction("&Models…", QKeySequence("Ctrl+M"));
    connect(modelsAct, &QAction::triggered, this, [this]() {
        _modelInfoWin->show();
        _modelInfoWin->raise();
        _modelInfoWin->activateWindow();
    });
    auto* ctrlAct = view->addAction("&Controllers…", QKeySequence("Ctrl+K"));
    connect(ctrlAct, &QAction::triggered, this, [this]() {
        _controllerInfoWin->show();
        _controllerInfoWin->raise();
        _controllerInfoWin->activateWindow();
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
