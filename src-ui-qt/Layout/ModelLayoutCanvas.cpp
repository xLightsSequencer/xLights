#include "ModelLayoutCanvas.h"

#include <QMouseEvent>
#include <QPainter>
#include <QtMath>
#include <cmath>

ModelLayoutCanvas::ModelLayoutCanvas(QWidget* parent) : QWidget(parent) {
    setMinimumSize(200, 150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
}

// ── Layout ────────────────────────────────────────────────────────────────────

void ModelLayoutCanvas::loadLayout(const QtSequenceInfo& seq) {
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

void ModelLayoutCanvas::setGroupHighlight(const QString& groupName,
                                          const QStringList& memberModels) {
    _highlightGroup   = groupName;
    _highlightMembers = memberModels;
    update();
}

void ModelLayoutCanvas::setSelectedModel(const QString& modelName) {
    _selectedModel = modelName;
    update();
}

void ModelLayoutCanvas::clearSelection() {
    _selectedModel.clear();
    update();
}

// ── Bounds ────────────────────────────────────────────────────────────────────

void ModelLayoutCanvas::recomputeBounds() {
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
    const double padX = (_maxX - _minX) * 0.05;
    const double padY = (_maxY - _minY) * 0.05;
    _minX -= padX; _maxX += padX;
    _minY -= padY; _maxY += padY;
    if (_maxX <= _minX) _maxX = _minX + 1;
    if (_maxY <= _minY) _maxY = _minY + 1;
}

QPointF ModelLayoutCanvas::toWidget(double gx, double gy) const {
    const double rangeX = _maxX - _minX;
    const double rangeY = _maxY - _minY;
    return { float((gx - _minX) / rangeX * width()),
             float((gy - _minY) / rangeY * height()) };
}

// ── Paint ─────────────────────────────────────────────────────────────────────

void ModelLayoutCanvas::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor(15, 15, 20));

    if (_models.isEmpty()) {
        p.setPen(QColor(80, 80, 80));
        p.drawText(rect(), Qt::AlignCenter, "Open a sequence to see the layout");
        return;
    }

    const double rangeX = _maxX - _minX;
    const double rangeY = _maxY - _minY;
    const double pxPerU = qMin(width() / rangeX, height() / rangeY);
    _nodeR = float(qBound(1.0, pxPerU * 0.40, 6.0));

    const bool hasGroupHL  = !_highlightGroup.isEmpty();
    const bool hasModelSel = !_selectedModel.isEmpty();

    p.setPen(Qt::NoPen);

    for (const auto& d : _models) {
        const int    N       = d.info.globalPositions.size();
        const bool   inGroup = hasGroupHL && _highlightMembers.contains(d.info.name);
        const bool   isSel   = hasModelSel && d.info.name == _selectedModel;
        double cx = 0, cy = 0;

        for (int i = 0; i < N; ++i) {
            const QPointF& gp = d.info.globalPositions[i];
            const QPointF  wp = toWidget(gp.x(), gp.y());
            cx += wp.x(); cy += wp.y();

            QColor col = (i < d.pixels.size()) ? d.pixels[i] : QColor(40, 40, 40);

            // Dim models not part of the active group highlight.
            if (hasGroupHL && !inGroup)
                col = col.darker(200);
            // Dim models that are not the selected model (when a model is selected).
            else if (hasModelSel && !isSel && !inGroup)
                col = col.darker(160);

            p.setBrush(col);
            p.drawEllipse(QRectF(wp.x() - _nodeR, wp.y() - _nodeR,
                                 _nodeR * 2.f, _nodeR * 2.f));
        }

        if (N == 0) continue;
        cx /= N; cy /= N;

        // Selected model: solid yellow border around its centroid bounding box.
        if (isSel) {
            double wx0 = 1e9, wy0 = 1e9, wx1 = -1e9, wy1 = -1e9;
            for (const QPointF& gp : d.info.globalPositions) {
                const QPointF wp = toWidget(gp.x(), gp.y());
                wx0 = qMin(wx0, wp.x()); wx1 = qMax(wx1, wp.x());
                wy0 = qMin(wy0, wp.y()); wy1 = qMax(wy1, wp.y());
            }
            const float pad = _nodeR + 3.f;
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(255, 220, 0, 230), 2.0));
            p.drawRoundedRect(QRectF(wx0 - pad, wy0 - pad,
                                     wx1 - wx0 + pad * 2, wy1 - wy0 + pad * 2), 4, 4);
            p.setPen(Qt::NoPen);
        }

        // Model name label.
        if (_nodeR >= 2.0f) {
            QFont lf;
            lf.setPointSizeF(qBound(6.0, double(_nodeR) * 2.5, 11.0));
            p.setFont(lf);
            const QColor labelColor = (inGroup || isSel)
                ? QColor(255, 220, 80, 230)
                : QColor(200, 200, 200, 160);
            p.setPen(labelColor);
            QFontMetrics fm(lf);
            const QString label = d.info.name;
            QRectF lr(cx - fm.horizontalAdvance(label) / 2.0 - 2, cy - fm.height() - 2,
                      fm.horizontalAdvance(label) + 4, fm.height() + 2);
            p.fillRect(lr, QColor(0, 0, 0, (inGroup || isSel) ? 140 : 80));
            p.drawText(lr, Qt::AlignCenter, label);
            p.setPen(Qt::NoPen);
        }
    }

    // Dashed outline + label around the highlighted group's member extent.
    if (hasGroupHL && !_highlightMembers.isEmpty()) {
        double wx0 = 1e9, wy0 = 1e9, wx1 = -1e9, wy1 = -1e9;
        for (const auto& d : _models) {
            if (!_highlightMembers.contains(d.info.name)) continue;
            for (const QPointF& gp : d.info.globalPositions) {
                const QPointF wp = toWidget(gp.x(), gp.y());
                wx0 = qMin(wx0, wp.x()); wx1 = qMax(wx1, wp.x());
                wy0 = qMin(wy0, wp.y()); wy1 = qMax(wy1, wp.y());
            }
        }
        if (wx1 > wx0 && wy1 > wy0) {
            const float pad = _nodeR + 4.f;
            QRectF outline(wx0 - pad, wy0 - pad,
                           wx1 - wx0 + pad * 2, wy1 - wy0 + pad * 2);
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(255, 200, 60, 200), 2.0, Qt::DashLine));
            p.drawRoundedRect(outline, 6, 6);

            QFont gf;
            gf.setPointSizeF(qBound(7.0, double(_nodeR) * 3.0, 12.0));
            gf.setBold(true);
            p.setFont(gf);
            p.setPen(QColor(255, 200, 60, 220));
            QFontMetrics gfm(gf);
            QRectF glr(outline.x(), outline.y() - gfm.height() - 4,
                       gfm.horizontalAdvance(_highlightGroup) + 8,
                       gfm.height() + 4);
            p.fillRect(glr, QColor(0, 0, 0, 160));
            p.drawText(glr, Qt::AlignCenter, _highlightGroup);
        }
    }
}

// ── Mouse ─────────────────────────────────────────────────────────────────────

void ModelLayoutCanvas::mousePressEvent(QMouseEvent* ev) {
    const QPointF click = ev->position();
    double bestDist = 1e9;
    QString bestModel;

    for (const auto& d : _models) {
        for (const QPointF& gp : d.info.globalPositions) {
            const QPointF wp = toWidget(gp.x(), gp.y());
            const double dx = click.x() - wp.x();
            const double dy = click.y() - wp.y();
            const double dist = std::sqrt(dx*dx + dy*dy);
            if (dist < bestDist) {
                bestDist  = dist;
                bestModel = d.info.name;
            }
        }
    }

    if (!bestModel.isEmpty() && bestDist < 20.0)
        emit modelClicked(bestModel);
}
