#include "HousePreviewWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QtMath>
#include <cmath>

HousePreviewWidget::HousePreviewWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(200, 150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
}

// ── Layout load ───────────────────────────────────────────────────────────────

void HousePreviewWidget::loadLayout(const QtSequenceInfo& seq) {
    _models.clear();
    _index.clear();

    for (const auto& mi : seq.models) {
        if (mi.globalPositions.isEmpty()) continue;
        ModelData d;
        d.info = mi;
        d.pixels.fill(QColor(40, 40, 40), mi.nodePositions.size());
        _models.append(d);
        _index[mi.name] = _models.size() - 1;
    }

    recomputeBounds();
    update();
}

void HousePreviewWidget::setModelPixels(const QString& modelName,
                                        const QList<QColor>& pixels) {
    auto it = _index.find(modelName);
    if (it == _index.end()) return;
    ModelData& d = _models[it.value()];
    d.pixels     = pixels;
    d.hasPixels  = true;
    update();
}

void HousePreviewWidget::clearPixels() {
    for (auto& d : _models) {
        d.pixels.fill(QColor(40, 40, 40), d.info.nodePositions.size());
        d.hasPixels = false;
    }
    update();
}

// ── Bounds ────────────────────────────────────────────────────────────────────

void HousePreviewWidget::recomputeBounds() {
    if (_models.isEmpty()) {
        _minX = 0; _minY = 0; _maxX = 1000; _maxY = 600;
        return;
    }

    _minX = _minY =  1e9;
    _maxX = _maxY = -1e9;

    for (const auto& d : _models) {
        for (const QPointF& gp : d.info.globalPositions) {
            if (gp.x() < _minX) _minX = gp.x();
            if (gp.x() > _maxX) _maxX = gp.x();
            if (gp.y() < _minY) _minY = gp.y();
            if (gp.y() > _maxY) _maxY = gp.y();
        }
    }

    // Add 5% padding on each side.
    const double padX = (_maxX - _minX) * 0.05;
    const double padY = (_maxY - _minY) * 0.05;
    _minX -= padX; _maxX += padX;
    _minY -= padY; _maxY += padY;
    if (_maxX <= _minX) _maxX = _minX + 1;
    if (_maxY <= _minY) _maxY = _minY + 1;
}

QPointF HousePreviewWidget::toWidget(double gx, double gy) const {
    const double rangeX = _maxX - _minX;
    const double rangeY = _maxY - _minY;
    const float  wx     = float((gx - _minX) / rangeX * width());
    const float  wy     = float((gy - _minY) / rangeY * height());
    return {wx, wy};
}

// ── Paint ─────────────────────────────────────────────────────────────────────

void HousePreviewWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor(15, 15, 20));

    if (_models.isEmpty()) {
        p.setPen(QColor(80, 80, 80));
        p.drawText(rect(), Qt::AlignCenter,
                   "Open a sequence to see the house layout");
        return;
    }

    // Compute node radius: scale so densely-packed models still look reasonable.
    const double rangeX  = _maxX - _minX;
    const double rangeY  = _maxY - _minY;
    const double pxPerUX = width()  / rangeX;  // pixels per layout unit, X
    const double pxPerUY = height() / rangeY;  // pixels per layout unit, Y
    const double pxPerU  = qMin(pxPerUX, pxPerUY);

    // Node radius: 40% of one layout-unit in widget pixels, clamped 1–6 px.
    _nodeR = float(qBound(1.0, pxPerU * 0.40, 6.0));

    p.setPen(Qt::NoPen);

    for (const auto& d : _models) {
        const int N = d.info.globalPositions.size();
        double cx = 0, cy = 0;

        for (int i = 0; i < N; ++i) {
            const QPointF& gp = d.info.globalPositions[i];
            const QPointF  wp = toWidget(gp.x(), gp.y());
            cx += wp.x(); cy += wp.y();

            const QColor col = (i < d.pixels.size()) ? d.pixels[i] : QColor(40, 40, 40);
            p.setBrush(col);
            p.drawEllipse(QRectF(wp.x() - _nodeR, wp.y() - _nodeR,
                                 _nodeR * 2.f, _nodeR * 2.f));
        }

        // Draw model name label at centroid when nodes are large enough to read.
        if (N > 0 && _nodeR >= 2.0f) {
            cx /= N; cy /= N;
            QFont labelFont;
            labelFont.setPointSizeF(qBound(6.0, double(_nodeR) * 2.5, 11.0));
            p.setFont(labelFont);
            p.setPen(QColor(220, 220, 220, 180));
            const QString label = d.info.name;
            QFontMetrics fm(labelFont);
            QRectF lr(cx - fm.horizontalAdvance(label) / 2.0 - 2, cy - fm.height() - 2,
                      fm.horizontalAdvance(label) + 4, fm.height() + 2);
            p.fillRect(lr, QColor(0, 0, 0, 100));
            p.drawText(lr, Qt::AlignCenter, label);
            p.setPen(Qt::NoPen);
        }
    }
}

// ── Mouse ─────────────────────────────────────────────────────────────────────

void HousePreviewWidget::mousePressEvent(QMouseEvent* ev) {
    // Find the closest model to the click and emit its name.
    const QPointF click = ev->position();
    double bestDist = 1e9;
    QString bestModel;

    for (const auto& d : _models) {
        const int N = d.info.globalPositions.size();
        for (int i = 0; i < N; ++i) {
            const QPointF wp = toWidget(d.info.globalPositions[i].x(),
                                        d.info.globalPositions[i].y());
            double dx = click.x() - wp.x();
            double dy = click.y() - wp.y();
            double dist = std::sqrt(dx*dx + dy*dy);
            if (dist < bestDist) {
                bestDist  = dist;
                bestModel = d.info.name;
            }
        }
    }

    if (!bestModel.isEmpty() && bestDist < 20.0)
        emit modelClicked(bestModel);
}
