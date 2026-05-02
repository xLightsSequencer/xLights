#include "RowHeaderWidget.h"
#include "SequencerModel.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

static constexpr int kHeaderW = 180;

RowHeaderWidget::RowHeaderWidget(SequencerModel* model, QWidget* parent)
    : QWidget(parent), _model(model) {
    setFixedWidth(kHeaderW);
    // Use a dark background that won't be overridden by the system palette.
    setAutoFillBackground(false);
    connect(model, &SequencerModel::modelChanged,    this, QOverload<>::of(&QWidget::update));
    connect(model, &SequencerModel::geometryChanged, this, QOverload<>::of(&QWidget::update));
}

void RowHeaderWidget::setScrollOffset(int yOffset) { _yOff = yOffset; update(); }

QSize RowHeaderWidget::sizeHint() const { return {kHeaderW, _model->gridHeight()}; }

void RowHeaderWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Solid dark background — paint the whole widget first to avoid any
    // light-gray system background showing through gaps between rows.
    p.fillRect(rect(), QColor(0x1c, 0x1c, 0x1c));

    const int rh = _model->rowHeight();

    for (int r = 0; r < _model->rowCount(); ++r) {
        if (!_model->isRowVisible(r)) continue;
        const int y = _model->yAt(r) - _yOff;
        if (y + rh < 0 || y > height()) continue;

        const SequencerRow& row     = _model->row(r);
        const bool          sel     = (r == _model->selectedRow());
        const bool          isLayer = row.isLayerRow();

        const QRect cell(0, y, width(), rh);

        // ── Background ────────────────────────────────────────────────────
        QColor bg;
        if (sel)
            bg = QColor(0x3a, 0x3a, 0x5a);
        else if (isLayer)
            bg = QColor(0x22, 0x22, 0x28);
        else
            bg = QColor(0x2c, 0x2c, 0x2c);
        p.fillRect(cell, bg);

        // ── Accent stripe (model rows) / indent bar (layer rows) ──────────
        if (!isLayer) {
            const uint  h      = qHash(row.modelName);
            const QColor accent = QColor::fromHsv(int((h * 137u) % 360), 150, 200);
            p.fillRect(QRect(0, y, 4, rh), accent);
        } else {
            // Subtle left indent line
            p.fillRect(QRect(4, y, 1, rh), QColor(0x44, 0x44, 0x60));
        }

        // ── Bottom separator ──────────────────────────────────────────────
        p.fillRect(QRect(0, y + rh - 1, width(), 1),
                   isLayer ? QColor(0x2a, 0x2a, 0x2a) : QColor(0x18, 0x18, 0x18));

        // ── Collapse triangle (multi-layer model rows only) ───────────────
        int textIndent = isLayer ? 20 : 8;
        if (!isLayer && row.layerCount > 1) {
            textIndent = 24;
            const int cx  = 14;
            const int cy2 = y + rh / 2;
            const int sz  = 4;
            QPainterPath tri;
            if (row.collapsed) {
                tri.moveTo(cx - sz, cy2 - sz);
                tri.lineTo(cx + sz, cy2);
                tri.lineTo(cx - sz, cy2 + sz);
            } else {
                tri.moveTo(cx - sz, cy2 - sz + 1);
                tri.lineTo(cx + sz, cy2 - sz + 1);
                tri.lineTo(cx,      cy2 + sz - 1);
            }
            tri.closeSubpath();
            p.setBrush(QColor(0xcc, 0xcc, 0xdd));
            p.setPen(Qt::NoPen);
            p.drawPath(tri);
        }

        // ── Label ─────────────────────────────────────────────────────────
        QFont f;
        f.setFamily("Segoe UI");
        if (!isLayer) {
            f.setPointSize(9);
            f.setBold(true);
            p.setPen(QColor(0xee, 0xee, 0xee));   // near-white, high contrast
        } else {
            f.setPointSize(8);
            f.setBold(false);
            p.setPen(QColor(0xbb, 0xbb, 0xcc));   // light lavender-gray, readable
        }
        p.setFont(f);
        const QRect textRect = cell.adjusted(textIndent, 0, -6, 0);
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
                   p.fontMetrics().elidedText(row.name, Qt::ElideRight, textRect.width()));

        // ── Layer count badge ─────────────────────────────────────────────
        if (!isLayer && row.layerCount > 1) {
            const QString badge = row.collapsed
                ? QString("(%1L)").arg(row.layerCount)
                : QString("%1L").arg(row.layerCount);
            QFont bf;
            bf.setFamily("Segoe UI");
            bf.setPointSize(7);
            p.setFont(bf);
            p.setPen(QColor(0x88, 0x88, 0xaa));
            p.drawText(cell.adjusted(0, 0, -6, 0),
                       Qt::AlignVCenter | Qt::AlignRight, badge);
        }
    }
}

void RowHeaderWidget::mousePressEvent(QMouseEvent* ev) {
    const int rh = _model->rowHeight();
    const int vy  = ev->position().y() + _yOff;
    const int dataRow = _model->rowAt(int(ev->position().y()));
    if (dataRow < 0) return;

    const SequencerRow& row = _model->row(dataRow);
    // Click the triangle area (left 24px of a multi-layer model row) → toggle collapse
    if (row.isModelRow() && row.layerCount > 1 && ev->position().x() < 24) {
        _model->toggleCollapse(dataRow);
        emit collapseToggled(dataRow);
        return;
    }
    QWidget::mousePressEvent(ev);
}
