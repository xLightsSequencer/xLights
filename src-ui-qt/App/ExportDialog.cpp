#include "ExportDialog.h"
#include "../Bridge/FseqWriter.h"
#include "../Bridge/QtRenderBridge.h"
#include "../Bridge/SubBufferUtil.h"
#include "../Sequencer/SequencerModel.h"

#include <spdlog/spdlog.h>

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

// Frames rendered per timer tick — keep UI responsive while exporting.
// src-core renders are slower than the old software renderer so we use 4 at a time.
static constexpr int kBatchSize = 4;

ExportDialog::ExportDialog(SequencerModel*       model,
                           const QtSequenceInfo& seq,
                           QtRenderBridge*        bridge,
                           QWidget*               parent)
    : QDialog(parent), _model(model), _seq(seq), _bridge(bridge)
{
    setWindowTitle("Export FSEQ");
    setMinimumWidth(500);

    auto* vbox = new QVBoxLayout(this);

    // ── Path row ───────────────────────────────────────────────────────────
    auto* form    = new QFormLayout;
    auto* pathRow = new QHBoxLayout;
    _pathEdit = new QLineEdit;
    _pathEdit->setPlaceholderText("output.fseq");
    auto* browseBtn = new QPushButton("Browse…");
    browseBtn->setFixedWidth(80);
    pathRow->addWidget(_pathEdit, 1);
    pathRow->addWidget(browseBtn);
    form->addRow("Output file:", pathRow);

    buildChannelMap();
    int fps  = qMax(1, _model->fps());
    int dur  = _model->totalFrames() * 1000 / fps;
    form->addRow("Sequence:", new QLabel(
        QString("%1 frames  |  %2 fps  |  %3 s  |  %4 channels  |  %5 models")
            .arg(_model->totalFrames()).arg(fps).arg(dur / 1000)
            .arg(_totalChannels).arg(_modelOrder.size())));
    vbox->addLayout(form);

    _progress = new QProgressBar;
    _progress->setRange(0, _model->totalFrames());
    _progress->setValue(0);
    _progress->setVisible(false);
    vbox->addWidget(_progress);

    _statusLbl = new QLabel;
    _statusLbl->setAlignment(Qt::AlignCenter);
    _statusLbl->setVisible(false);
    vbox->addWidget(_statusLbl);

    auto* btnRow = new QHBoxLayout;
    _exportBtn = new QPushButton("Export");
    _cancelBtn = new QPushButton("Cancel");
    _exportBtn->setDefault(true);
    btnRow->addStretch(1);
    btnRow->addWidget(_exportBtn);
    btnRow->addWidget(_cancelBtn);
    vbox->addLayout(btnRow);

    connect(browseBtn,  &QPushButton::clicked, this, &ExportDialog::onBrowse);
    connect(_exportBtn, &QPushButton::clicked, this, &ExportDialog::onExport);
    connect(_cancelBtn, &QPushButton::clicked, this, &ExportDialog::onCancel);

    _timer = new QTimer(this);
    _timer->setInterval(0);
    connect(_timer, &QTimer::timeout, this, &ExportDialog::onRenderTick);
}

// ── Channel map ───────────────────────────────────────────────────────────────

void ExportDialog::buildChannelMap() {
    _channelMap.clear();
    _modelOrder.clear();
    _totalChannels = 0;

    // Walk all rows; collect unique models in sequencer order.
    for (int r = 0; r < _model->rowCount(); ++r) {
        const QString& mn = _model->row(r).modelName;
        if (_channelMap.contains(mn)) continue;

        const QtModelInfo mi = _seq.modelInfo(mn);
        // nodeCount channels per model (each node = 3 RGB channels).
        const int nCh = mi.nodeCount * 3;
        // Use show-file start channel when it's a plain integer (1-based).
        // Controller-relative start channels ("!Controller:N") are not numeric
        // and produce as_int() == 0 in the parser, so we pack sequentially.
        const int startCh = (mi.startChannel > 0) ? (mi.startChannel - 1) : _totalChannels;

        _channelMap[mn] = { startCh, nCh };
        _modelOrder.append(mn);
        _totalChannels = qMax(_totalChannels, startCh + nCh);
    }
    if (_totalChannels == 0) _totalChannels = 1;
}

// ── Render one model, all layers, at a specific frame ────────────────────────

QList<QColor> ExportDialog::renderModelAt(const QString& modelName, int frameIndex) {
    const QtModelInfo mi  = _seq.modelInfo(modelName);
    const int bufW = mi.bufferW;
    const int bufH = mi.bufferH;
    const int n    = bufW * bufH;
    if (n == 0) return {};

    const int fps     = qMax(1, _model->fps());
    const int frameMs = 1000 / fps;

    // Collect active blocks for all layers of this model at frameIndex.
    struct LayerBlock { int layerIndex; const EffectBlock* blk; };
    QList<LayerBlock> active;
    for (int r = 0; r < _model->rowCount(); ++r) {
        const SequencerRow& row = _model->row(r);
        if (row.modelName != modelName) continue;
        for (const EffectBlock& blk : row.blocks) {
            if (frameIndex >= blk.startFrame && frameIndex < blk.endFrame) {
                active.append({row.layerIndex, &blk});
                break;
            }
        }
    }
    if (active.isEmpty()) return {};

    QList<QColor> composite(n, Qt::black);

    for (const auto& lb : active) {
        const EffectBlock& blk = *lb.blk;
        const double progress = blk.endFrame > blk.startFrame
            ? double(frameIndex - blk.startFrame) / double(blk.endFrame - blk.startFrame)
            : 0.5;

        double sbLeft = 0, sbBottom = 0, sbRight = 100, sbTop = 100;
        parseSubBuffer(blk.settings, sbLeft, sbBottom, sbRight, sbTop);
        const int subLeft_px   = qBound(0, int(sbLeft   * bufW / 100.0), bufW);
        const int subBottom_px = qBound(0, int(sbBottom * bufH / 100.0), bufH);
        const int subRight_px  = qBound(0, int(sbRight  * bufW / 100.0 + 0.5), bufW);
        const int subTop_px    = qBound(0, int(sbTop    * bufH / 100.0 + 0.5), bufH);
        const int subW = qMax(1, subRight_px - subLeft_px);
        const int subH = qMax(1, subTop_px   - subBottom_px);
        const bool fullBuf = (subLeft_px == 0 && subBottom_px == 0 &&
                              subRight_px == bufW && subTop_px == bufH);

        QtEffectRenderer::Request req;
        req.effectName  = blk.effectName;
        req.progress    = qBound(0.0, progress, 1.0);
        req.rawSettings = blk.settings;
        req.rawPalette  = blk.palette;
        req.bufferW     = fullBuf ? bufW : subW;
        req.bufferH     = fullBuf ? bufH : subH;

        const QtEffectRenderer::Result r = _bridge->renderNow(req);
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

// ── Slots ─────────────────────────────────────────────────────────────────────

void ExportDialog::onBrowse() {
    QString path = QFileDialog::getSaveFileName(
        this, "Save FSEQ", _pathEdit->text(), "FSEQ Files (*.fseq)");
    if (!path.isEmpty()) {
        if (!path.endsWith(".fseq", Qt::CaseInsensitive)) path += ".fseq";
        _pathEdit->setText(path);
    }
}

void ExportDialog::onExport() {
    const QString path = _pathEdit->text().trimmed();
    if (path.isEmpty()) {
        QMessageBox::warning(this, "Export", "Please choose an output file.");
        return;
    }

    _totalFrames = _model->totalFrames();
    const int fps    = qMax(1, _model->fps());
    const int stepMs = 1000 / fps;

    _writer = new FseqWriter(path, _totalChannels, _totalFrames, stepMs);
    if (!_writer->isOpen()) {
        QMessageBox::critical(this, "Export",
            "Cannot create file:\n" + _writer->errorString());
        delete _writer; _writer = nullptr;
        return;
    }

    _currentFrame = 0;
    _cancelled    = false;
    setExporting(true);
    _timer->start();

    spdlog::info("ExportDialog: writing {} frames × {} ch ({} models) to '{}'",
                 _totalFrames, _totalChannels, _modelOrder.size(), path.toStdString());
}

void ExportDialog::onCancel() {
    if (_timer->isActive()) {
        _cancelled = true;
        _timer->stop();
        delete _writer; _writer = nullptr;
        setExporting(false);
        _statusLbl->setText("Export cancelled.");
    } else {
        reject();
    }
}

void ExportDialog::onRenderTick() {
    if (!_writer || _cancelled) return;

    for (int b = 0; b < kBatchSize && _currentFrame < _totalFrames; ++b, ++_currentFrame) {
        QByteArray frameData(_totalChannels, '\0');

        for (const QString& mn : _modelOrder) {
            const QList<QColor> composite = renderModelAt(mn, _currentFrame);
            if (composite.isEmpty()) continue;

            const ModelChannel& ch = _channelMap[mn];
            for (int i = 0; i < composite.size(); ++i) {
                int c = ch.start + i * 3;
                if (c + 2 >= _totalChannels) break;
                const QColor& px = composite[i];
                frameData[c]     = char(px.red());
                frameData[c + 1] = char(px.green());
                frameData[c + 2] = char(px.blue());
            }
        }

        _writer->writeFrame(frameData);
    }

    _progress->setValue(_currentFrame);
    const int fps = qMax(1, _model->fps());
    _statusLbl->setText(QString("Rendering frame %1 / %2  (%3 s)")
                        .arg(_currentFrame).arg(_totalFrames)
                        .arg(_currentFrame / fps));

    if (_currentFrame >= _totalFrames) {
        _timer->stop();
        _writer->finish();
        delete _writer; _writer = nullptr;
        setExporting(false);
        _statusLbl->setText(
            QString("Done — %1 frames, %2 models exported.").arg(_totalFrames).arg(_modelOrder.size()));
        _exportBtn->setEnabled(true);
        spdlog::info("ExportDialog: export complete ({} frames, {} models)",
                     _totalFrames, _modelOrder.size());
    }
}

void ExportDialog::setExporting(bool on) {
    _exportBtn->setEnabled(!on);
    _progress->setVisible(on || _statusLbl->isVisible());
    _statusLbl->setVisible(true);
    if (on) {
        _progress->setValue(0);
        _statusLbl->setText("Starting render…");
        _cancelBtn->setText("Cancel");
    } else {
        _cancelBtn->setText("Close");
    }
}
