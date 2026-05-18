#include "SequencerGridCanvas.h"
#include "SequencerModel.h"
#include "SequencerController.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

SequencerGridCanvas::SequencerGridCanvas(SequencerModel* model,
                                          SequencerController* ctrl,
                                          QWidget* parent)
    : QWidget(parent), _model(model), _ctrl(ctrl) {
    setMouseTracking(true);
    setAcceptDrops(true);
    connect(model, &SequencerModel::modelChanged,    this, [this]{ updateGeometry(); update(); });
    connect(model, &SequencerModel::geometryChanged, this, [this]{ updateGeometry(); update(); });
    connect(model, &SequencerModel::selectionChanged,this, QOverload<>::of(&QWidget::update));
    connect(ctrl,  &SequencerController::repaintNeeded, this, QOverload<>::of(&QWidget::update));
}

// ── Drawing helpers ───────────────────────────────────────────────────────────
void SequencerGridCanvas::drawBlock(QPainter& p, int x0, int x1, int y,
                                     int h, const QColor& col,
                                     const QString& name,
                                     bool selected, float opacity) {
    if (x1 <= x0) return;
    QRect r(x0 + 1, y + 1, x1 - x0 - 2, h - 2);
    QColor fill = col;
    fill.setAlphaF(opacity);
    p.fillRect(r, fill);

    // Lighter top edge for 3-D feel
    QColor hi = fill.lighter(140);
    hi.setAlphaF(opacity);
    p.setPen(hi);
    p.drawLine(r.left(), r.top(), r.right(), r.top());

    // Selection border
    if (selected) {
        QPen pen(QColor(0xff, 0xee, 0x00), 2);
        p.setPen(pen);
        p.drawRect(r.adjusted(0, 0, -1, -1));
    }

    // Resize handles (subtle lighter strips at left/right edges)
    QColor handle = col.lighter(160);
    handle.setAlphaF(opacity * 0.6f);
    p.fillRect(r.left(), r.top(), 4, r.height(), handle);
    p.fillRect(r.right() - 3, r.top(), 4, r.height(), handle);

    // Label (clip to block width)
    if (r.width() > 24) {
        p.setPen(QColor(255, 255, 255, int(opacity * 220)));
        p.setClipRect(r.adjusted(6, 0, -6, 0));
        p.drawText(r.adjusted(6, 0, -6, 0), Qt::AlignVCenter | Qt::AlignLeft, name);
        p.setClipping(false);
    }
}

// ── Paint ─────────────────────────────────────────────────────────────────────
void SequencerGridCanvas::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.fillRect(rect(), QColor(0x1e, 0x1e, 0x1e));

    const int rh    = _model->rowHeight();
    const int total = _model->rowCount();

    // Row backgrounds + grid lines
    for (int r = 0; r < total; ++r) {
        if (!_model->isRowVisible(r)) continue;
        int y = _model->yAt(r);
        if (y + rh < 0 || y > height()) continue;
        const SequencerRow& row = _model->row(r);
        QColor bg;
        if (r == _model->selectedRow())
            bg = QColor(0x2a, 0x2a, 0x3a);
        else if (row.isLayerRow())
            bg = QColor(0x1e, 0x1e, 0x28);   // darker tint for layer sub-rows
        else
            bg = (r % 2 == 0) ? QColor(0x26, 0x26, 0x26) : QColor(0x22, 0x22, 0x22);
        p.fillRect(0, y, width(), rh, bg);
        // Layer rows get a dashed separator; model rows get a solid line
        p.setPen(row.isLayerRow() ? QColor(0x2e, 0x2e, 0x38) : QColor(0x33, 0x33, 0x33));
        p.drawLine(0, y + rh - 1, width(), y + rh - 1);
    }

    // Vertical second marks
    p.setPen(QColor(0x33, 0x33, 0x33));
    for (int f = 0; f <= _model->totalFrames(); f += _model->fps()) {
        int x = _model->xAt(f);
        if (x < 0 || x > width()) continue;
        p.drawLine(x, 0, x, height());
    }

    // Effect blocks — dim the block being dragged at its source row
    for (int r = 0; r < total; ++r) {
        if (!_model->isRowVisible(r)) continue;
        int y = _model->yAt(r);
        for (int b = 0; b < _model->row(r).blocks.size(); ++b) {
            const EffectBlock& blk = _model->row(r).blocks[b];
            int x0 = _model->xAt(blk.startFrame);
            int x1 = _model->xAt(blk.endFrame);
            // Dim at the SOURCE row while dragging (regardless of target row).
            bool isDragged = _ctrl->hasGhost() && r == _ctrl->dragSourceRow()
                             && blk.selected;
            float opacity = isDragged ? 0.3f : 1.0f;
            drawBlock(p, x0, x1, y, rh, blk.color, blk.effectName,
                      blk.selected && !isDragged, opacity);
        }
    }

    // Playhead — drawn above blocks so it's always visible
    {
        int phX = _model->xAt(_model->playheadFrame());
        if (phX >= 0 && phX <= width()) {
            p.setPen(QPen(QColor(255, 60, 60), 2));
            p.drawLine(phX, 0, phX, height());
        }
    }

    // Ghost during drag — drawn at the TARGET row with the source block's color
    if (_ctrl->hasGhost()) {
        const int ghostR  = _ctrl->ghostRow();
        const int srcR    = _ctrl->dragSourceRow();
        const int ghostY  = _model->yAt(ghostR);
        const int x0      = _model->xAt(_ctrl->ghostStartFrame());
        const int x1      = _model->xAt(_ctrl->ghostEndFrame());

        // Read effect name + color from the source block.
        QColor  gc   = QColor(0x80, 0x80, 0xff);
        QString name;
        const int sb = _model->selectedBlock();
        if (sb >= 0 && sb < _model->row(srcR).blocks.size()) {
            gc   = _model->row(srcR).blocks[sb].color;
            name = _model->row(srcR).blocks[sb].effectName;
        }
        drawBlock(p, x0, x1, ghostY, rh, gc, name, false, 0.7f);

        // Dashed yellow border on the ghost
        QPen dash(QColor(0xff, 0xee, 0x00, 200), 1, Qt::DashLine);
        p.setPen(dash);
        p.drawRect(QRect(x0 + 1, ghostY + 1, x1 - x0 - 2, rh - 2));

        // Highlight the target row with a subtle tint when crossing rows
        if (ghostR != srcR && _model->isRowVisible(ghostR)) {
            p.fillRect(0, ghostY, width(), rh, QColor(0x80, 0x80, 0xff, 20));
        }
    }

    // Drop ghost — new block dragged in from the effect toolbar
    if (_hasDrop && _dropRow >= 0 && _model->isRowVisible(_dropRow)) {
        const int  dy  = _model->yAt(_dropRow);
        const int  dx0 = _model->xAt(_dropFrame);
        const int  dx1 = _model->xAt(_dropFrame + _dropDuration);
        const uint h   = qHash(_dropEffectName);
        const QColor gc = QColor::fromHsv(int((h * 137u) % 360), 160, 200);
        drawBlock(p, dx0, dx1, dy, rh, gc, _dropEffectName, false, 0.75f);
        QPen dash(QColor(0xff, 0xee, 0x00, 220), 1, Qt::DashLine);
        p.setPen(dash);
        p.drawRect(QRect(dx0 + 1, dy + 1, dx1 - dx0 - 2, rh - 2));
    }
}

// ── Mouse events ──────────────────────────────────────────────────────────────
void SequencerGridCanvas::mousePressEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton) return;
    _ctrl->mousePress(e->pos(), e->modifiers());

    int sr = _model->selectedRow(), sb = _model->selectedBlock();
    if (sr >= 0 && sb >= 0) emit blockSelected(sr, sb);
}

void SequencerGridCanvas::mouseMoveEvent(QMouseEvent* e) {
    _ctrl->mouseMove(e->pos());

    // While dragging, show a fixed cursor; only do hit-test for hover.
    if (_ctrl->hasGhost()) {
        const auto zone = _ctrl->dragZone();
        if (zone == SequencerModel::ResizeLeft || zone == SequencerModel::ResizeRight)
            setCursor(Qt::SizeHorCursor);
        else
            setCursor(Qt::ClosedHandCursor);
        return;
    }

    int row, block;
    SequencerModel::HitZone zone;
    if (_model->hitTest(e->pos(), row, block, zone)) {
        if (zone == SequencerModel::ResizeLeft || zone == SequencerModel::ResizeRight)
            setCursor(Qt::SizeHorCursor);
        else
            setCursor(Qt::OpenHandCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void SequencerGridCanvas::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton) return;
    _ctrl->mouseRelease(e->pos());
    // Re-emit blockSelected so the effect panel refreshes after a move or resize.
    const int sr = _model->selectedRow(), sb = _model->selectedBlock();
    if (sr >= 0 && sb >= 0) emit blockSelected(sr, sb);
}

void SequencerGridCanvas::wheelEvent(QWheelEvent* e) {
    // Ctrl+wheel: zoom (pixels per frame)
    if (e->modifiers() & Qt::ControlModifier) {
        double factor = e->angleDelta().y() > 0 ? 1.15 : 1.0/1.15;
        _model->setPixelsPerFrame(_model->pixelsPerFrame() * factor);
        e->accept();
    } else {
        e->ignore();   // let scroll area handle it
    }
}

// ── Effect-toolbar drag-and-drop ──────────────────────────────────────────────

void SequencerGridCanvas::dragEnterEvent(QDragEnterEvent* e) {
    if (e->mimeData()->hasFormat("application/x-xlights-effect"))
        e->acceptProposedAction();
}

void SequencerGridCanvas::dragMoveEvent(QDragMoveEvent* e) {
    if (!e->mimeData()->hasFormat("application/x-xlights-effect")) { e->ignore(); return; }

    _dropEffectName = QString::fromUtf8(
        e->mimeData()->data("application/x-xlights-effect"));
    const QPoint pos = e->position().toPoint();
    _dropRow      = _model->rowAt(pos.y());
    _dropFrame    = _model->frameAt(pos.x());
    _dropDuration = qMax(1, _model->fps() * 2);   // default 2-second block
    _hasDrop      = (_dropRow >= 0 && _model->isRowVisible(_dropRow));

    e->acceptProposedAction();
    update();
}

void SequencerGridCanvas::dragLeaveEvent(QDragLeaveEvent*) {
    _hasDrop = false;
    update();
}

void SequencerGridCanvas::dropEvent(QDropEvent* e) {
    _hasDrop = false;
    if (!e->mimeData()->hasFormat("application/x-xlights-effect")) { e->ignore(); return; }
    if (_dropRow < 0 || !_model->isRowVisible(_dropRow))           { e->ignore(); return; }

    const int endFrame = qMin(_dropFrame + _dropDuration, _model->totalFrames());
    _model->createBlock(_dropRow, _dropFrame, endFrame, _dropEffectName);

    // Select the newly created block and open the effect panel.
    const SequencerRow& row = _model->row(_dropRow);
    for (int b = row.blocks.size() - 1; b >= 0; --b) {
        if (row.blocks[b].startFrame == _dropFrame
                && row.blocks[b].effectName == _dropEffectName) {
            _model->select(_dropRow, b);
            emit blockSelected(_dropRow, b);
            break;
        }
    }

    e->acceptProposedAction();
    update();
}
