#include "RowHeaderWidget.h"
#include "SequencerModel.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

static constexpr int kHeaderW = 160;

RowHeaderWidget::RowHeaderWidget(SequencerModel* model, QWidget* parent)
    : QWidget(parent), _model(model) {
    setFixedWidth(kHeaderW);
    connect(model, &SequencerModel::modelChanged,    this, QOverload<>::of(&QWidget::update));
    connect(model, &SequencerModel::geometryChanged, this, QOverload<>::of(&QWidget::update));
}

void RowHeaderWidget::setScrollOffset(int yOffset) { _yOff = yOffset; update(); }

QSize RowHeaderWidget::sizeHint() const { return {kHeaderW, _model->gridHeight()}; }

void RowHeaderWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor(0x2a, 0x2a, 0x2a));

    const int rh = _model->rowHeight();

    for (int r = 0; r < _model->rowCount(); ++r) {
        if (!_model->isRowVisible(r)) continue;
        int y = _model->yAt(r) - _yOff;
        if (y + rh < 0 || y > height()) continue;

        const SequencerRow& row     = _model->row(r);
        const bool          sel     = (r == _model->selectedRow());
        const bool          isLayer = row.isLayerRow();

        QRect cell(0, y, kHeaderW - 1, rh - 1);

        QColor bg = sel ? QColor(0x40, 0x40, 0x60)
                        : isLayer ? QColor(0x26, 0x26, 0x30)
                                  : QColor(0x32, 0x32, 0x32);
        p.fillRect(cell, bg);

        if (!isLayer) {
            uint h = qHash(row.modelName);
            QColor accent = QColor::fromHsv(int((h * 137u) % 360), 120, 160);
            p.fillRect(QRect(0, y, 3, rh - 1), accent);
        } else {
            p.fillRect(QRect(0, y, 1, rh - 1), QColor(0x44, 0x44, 0x55));
        }

        p.setPen(QColor(isLayer ? 0x3a : 0x44, isLayer ? 0x3a : 0x44, 0x44));
        p.drawRect(cell);

        // Collapse triangle for multi-layer model rows
        if (!isLayer && row.layerCount > 1) {
            const int cx = 12, cy2 = y + rh / 2;
            const int sz = 4;
            QPainterPath tri;
            if (row.collapsed) {
                // Right-pointing ▶
                tri.moveTo(cx - sz, cy2 - sz);
                tri.lineTo(cx + sz, cy2);
                tri.lineTo(cx - sz, cy2 + sz);
            } else {
                // Down-pointing ▼
                tri.moveTo(cx - sz, cy2 - sz + 1);
                tri.lineTo(cx + sz, cy2 - sz + 1);
                tri.lineTo(cx,      cy2 + sz - 1);
            }
            tri.closeSubpath();
            p.setBrush(QColor(0xaa, 0xaa, 0xbb));
            p.setPen(Qt::NoPen);
            p.drawPath(tri);
        }

        QFont f("Segoe UI", isLayer ? 8 : 9);
        p.setFont(f);
        p.setPen(isLayer ? QColor(0x99, 0x99, 0xbb) : Qt::white);
        int indent = (!isLayer && row.layerCount > 1) ? 22 : (isLayer ? 16 : 8);
        p.drawText(cell.adjusted(indent, 0, -4, 0),
                   Qt::AlignVCenter | Qt::AlignLeft, row.name);

        // Layer count badge
        if (!isLayer && row.layerCount > 1) {
            QString badge = row.collapsed
                ? QString("(%1L)").arg(row.layerCount)
                : QString::number(row.layerCount) + "L";
            p.setFont(QFont("Segoe UI", 7));
            p.setPen(QColor(0x77, 0x77, 0x99));
            p.drawText(cell.adjusted(0, 0, -5, 0),
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
