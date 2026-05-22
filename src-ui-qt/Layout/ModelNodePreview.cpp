#include "ModelNodePreview.h"

#include <cmath>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

static constexpr int kPad = 10;

ModelNodePreview::ModelNodePreview(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(160, 120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    // WA_OpaquePaintEvent: we paint every pixel ourselves — no background fill needed.
    // Do NOT set a stylesheet with border-radius; the style engine co-paints and can
    // crash before the native backing store is ready.
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
}

// Only schedule a repaint when the widget is actually visible; calling update()
// on a hidden widget can trigger paintEvent before the backing store exists.
static void safeUpdate(QWidget* w) {
    if (w->isVisible()) w->update();
}

void ModelNodePreview::setNodePositions(const QList<QPointF>& positions) {
    _positions = positions;
    _highlighted.clear();
    safeUpdate(this);
}

void ModelNodePreview::highlightNodes(const QList<int>& indices, const QColor& color) {
    _highlighted.clear();
    for (int i : indices) _highlighted.insert(i);
    _hlColor = color;
    safeUpdate(this);
}

void ModelNodePreview::clearHighlight() {
    _highlighted.clear();
    safeUpdate(this);
}

QPointF ModelNodePreview::toWidget(const QPointF& norm) const {
    const int w = width()  - 2 * kPad;
    const int h = height() - 2 * kPad;
    return { kPad + norm.x() * w,
             kPad + norm.y() * h };
}

void ModelNodePreview::paintEvent(QPaintEvent*) {
    QPainter p(this);
    // Guard against a partially-initialised backing store (can happen if Qt
    // processes a deferred repaint before the window's native handle is ready).
    if (!p.isActive()) return;

    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(0x11, 0x11, 0x11));

    if (_positions.isEmpty()) {
        p.setPen(QColor(0x44, 0x44, 0x44));
        p.drawText(rect(), Qt::AlignCenter, "No model loaded");
        return;
    }

    const qreal r = qBound(2.0, 6.0, 280.0 / double(_positions.size()));

    for (int i = 0; i < _positions.size(); ++i) {
        const QPointF wp = toWidget(_positions[i]);
        if (!std::isfinite(wp.x()) || !std::isfinite(wp.y())) continue;

        if (_highlighted.contains(i)) {
            p.setBrush(_hlColor);
            p.setPen(_hlColor.darker(150));
            p.drawEllipse(wp, r + 1.0, r + 1.0);
        } else {
            const QColor dim(0x44, 0x44, 0x44);
            p.setBrush(dim);
            p.setPen(dim.darker(120));
            p.drawEllipse(wp, r, r);
        }
    }

    if (_lassoing) {
        p.setBrush(QColor(0x28, 0x78, 0xff, 40));
        p.setPen(QPen(QColor(0x28, 0x78, 0xff), 1, Qt::DashLine));
        p.drawRect(QRect(_lassoP1, _lassoP2).normalized());
    }
}

void ModelNodePreview::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        _lassoing = true;
        _lassoP1 = _lassoP2 = e->pos();
    }
}

void ModelNodePreview::mouseMoveEvent(QMouseEvent* e) {
    if (_lassoing) {
        _lassoP2 = e->pos();
        update();
    }
}

void ModelNodePreview::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton && _lassoing) {
        _lassoing = false;
        const QRectF sel = QRect(_lassoP1, e->pos()).normalized();

        QList<int> selected;
        for (int i = 0; i < _positions.size(); ++i) {
            if (sel.contains(toWidget(_positions[i])))
                selected.append(i);
        }
        update();
        emit nodesLassoed(selected);
    }
}
