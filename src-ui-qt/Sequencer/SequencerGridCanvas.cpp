#include "SequencerGridCanvas.h"
#include "SequencerModel.h"
#include "SequencerController.h"

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

SequencerGridCanvas::SequencerGridCanvas(SequencerModel* model,
                                          SequencerController* ctrl,
                                          QWidget* parent)
    : QWidget(parent), _model(model), _ctrl(ctrl) {
    setMouseTracking(true);
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

    // Effect blocks — dimmed originals while dragging
    for (int r = 0; r < total; ++r) {
        if (!_model->isRowVisible(r)) continue;
        int y = _model->yAt(r);
        for (int b = 0; b < _model->row(r).blocks.size(); ++b) {
            const EffectBlock& blk = _model->row(r).blocks[b];
            int x0 = _model->xAt(blk.startFrame);
            int x1 = _model->xAt(blk.endFrame);
            bool isDragged = _ctrl->hasGhost() && r == _ctrl->ghostRow()
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

    // Ghost during drag
    if (_ctrl->hasGhost()) {
        int r  = _ctrl->ghostRow();
        int y  = _model->yAt(r);
        int x0 = _model->xAt(_ctrl->ghostStartFrame());
        int x1 = _model->xAt(_ctrl->ghostEndFrame());
        // Determine ghost color from the selected block
        QColor gc = QColor(0x80, 0x80, 0xff);
        if (_model->selectedBlock() >= 0 && _model->selectedBlock() < _model->row(r).blocks.size())
            gc = _model->row(r).blocks[_model->selectedBlock()].color;
        drawBlock(p, x0, x1, y, rh, gc, "", false, 0.7f);
        // Dashed border on ghost
        QPen dash(QColor(0xff, 0xee, 0x00, 200), 1, Qt::DashLine);
        p.setPen(dash);
        p.drawRect(QRect(x0+1, y+1, x1-x0-2, rh-2));
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

    // Update cursor based on hover zone
    int row, block;
    SequencerModel::HitZone zone;
    if (_model->hitTest(e->pos(), row, block, zone)) {
        if (zone == SequencerModel::ResizeLeft || zone == SequencerModel::ResizeRight)
            setCursor(Qt::SizeHorCursor);
        else
            setCursor(Qt::SizeAllCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void SequencerGridCanvas::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) _ctrl->mouseRelease(e->pos());
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
