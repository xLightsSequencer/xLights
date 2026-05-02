#include "RowHeaderWidget.h"
#include "SequencerModel.h"
#include <QPainter>

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
    p.fillRect(rect(), QColor(0x2a, 0x2a, 0x2a));

    const int rh = _model->rowHeight();

    for (int r = 0; r < _model->rowCount(); ++r) {
        int y = _model->yAt(r) - _yOff;
        if (y + rh < 0 || y > height()) continue;

        const SequencerRow& row = _model->row(r);
        const bool sel    = (r == _model->selectedRow());
        const bool isLayer = row.isLayerRow();

        QRect cell(0, y, kHeaderW - 1, rh - 1);

        // Background: model rows are slightly brighter, layer sub-rows are darker/indented
        QColor bg = sel ? QColor(0x40, 0x40, 0x60)
                        : isLayer ? QColor(0x26, 0x26, 0x30)
                                  : QColor(0x32, 0x32, 0x32);
        p.fillRect(cell, bg);

        // Left accent bar: model rows get a coloured stripe, layer rows get a thin indent line
        if (!isLayer) {
            uint h = qHash(row.modelName);
            QColor accent = QColor::fromHsv(int((h * 137u) % 360), 120, 160);
            p.fillRect(QRect(0, y, 3, rh - 1), accent);
        } else {
            p.fillRect(QRect(0, y, 1, rh - 1), QColor(0x44, 0x44, 0x55));
        }

        // Border
        p.setPen(QColor(isLayer ? 0x3a : 0x44, isLayer ? 0x3a : 0x44, isLayer ? 0x44 : 0x44));
        p.drawRect(cell);

        // Label
        QFont f("Segoe UI", isLayer ? 8 : 9);
        if (!isLayer) f.setBold(false);
        p.setFont(f);
        p.setPen(isLayer ? QColor(0x99, 0x99, 0xbb) : Qt::white);
        int indent = isLayer ? 16 : 8;
        p.drawText(cell.adjusted(indent, 0, -4, 0),
                   Qt::AlignVCenter | Qt::AlignLeft, row.name);

        // Layer count badge on the model row when it has sub-layers
        if (!isLayer && row.layerCount > 1) {
            QString badge = QString::number(row.layerCount) + "L";
            p.setFont(QFont("Segoe UI", 7));
            p.setPen(QColor(0x77, 0x77, 0x99));
            p.drawText(cell.adjusted(0, 0, -5, 0),
                       Qt::AlignVCenter | Qt::AlignRight, badge);
        }
    }
}
