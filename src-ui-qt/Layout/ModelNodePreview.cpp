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
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
}

// ── Cache rebuild (off-screen QPainter — always safe) ─────────────────────────

void ModelNodePreview::rebuildCache() {
    const int w = width(), h = height();
    if (w <= 0 || h <= 0) return;

    _cache = QPixmap(w, h);
    QPainter p(&_cache);          // painting to QPixmap — never crashes
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(0, 0, w, h, QColor(0x11, 0x11, 0x11));

    if (_positions.isEmpty()) {
        p.setPen(QColor(0x44, 0x44, 0x44));
        p.drawText(QRect(0, 0, w, h), Qt::AlignCenter, "No model loaded");
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
}

// ── Public API ────────────────────────────────────────────────────────────────

void ModelNodePreview::setNodePositions(const QList<QPointF>& positions) {
    _positions = positions;
    _highlighted.clear();
    rebuildCache();
    if (isVisible()) update();
}

void ModelNodePreview::highlightNodes(const QList<int>& indices, const QColor& color) {
    _highlighted.clear();
    for (int i : indices) _highlighted.insert(i);
    _hlColor = color;
    rebuildCache();
    if (isVisible()) update();
}

void ModelNodePreview::clearHighlight() {
    _highlighted.clear();
    rebuildCache();
    if (isVisible()) update();
}

// ── QWidget overrides ─────────────────────────────────────────────────────────

QPointF ModelNodePreview::toWidget(const QPointF& norm) const {
    const int w = width()  - 2 * kPad;
    const int h = height() - 2 * kPad;
    return { kPad + norm.x() * w, kPad + norm.y() * h };
}

void ModelNodePreview::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    rebuildCache();
}

void ModelNodePreview::paintEvent(QPaintEvent*) {
    // All complex drawing is in the QPixmap cache — this just blits it.
    // Keeping paintEvent minimal avoids the QPainter-on-widget crash caused
    // by a partially-initialised backing-store raster buffer on Windows.
    QPainter p(this);
    if (!p.isActive()) return;

    if (_cache.isNull() || _cache.size() != size()) {
        rebuildCache();
        if (_cache.isNull()) { p.fillRect(rect(), QColor(0x11,0x11,0x11)); return; }
    }

    p.drawPixmap(0, 0, _cache);

    // Draw lasso overlay on top of the cached image.
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
