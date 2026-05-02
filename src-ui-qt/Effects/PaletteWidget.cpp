#include "PaletteWidget.h"
#include "controls/ColorSwatchButton.h"

#include <QHBoxLayout>
#include <QLabel>

// Default 8-color palette that ships with most new xLights effects.
static const QColor kDefaults[PaletteWidget::kMaxColors] = {
    QColor(255,   0,   0),   // red
    QColor(  0, 255,   0),   // green
    QColor(  0,   0, 255),   // blue
    QColor(255, 255,   0),   // yellow
    QColor(255, 128,   0),   // orange
    QColor(128,   0, 255),   // purple
    QColor(  0, 255, 255),   // cyan
    QColor(255, 255, 255),   // white
};

PaletteWidget::PaletteWidget(QWidget* parent) : QWidget(parent) {
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(4, 2, 4, 2);
    row->setSpacing(3);
    row->addWidget(new QLabel("Palette:"));

    for (int i = 0; i < kMaxColors; ++i) {
        _swatches[i] = new ColorSwatchButton(kDefaults[i], this);
        row->addWidget(_swatches[i]);
        connect(_swatches[i], &ColorSwatchButton::colorChanged,
                this, [this](const QColor&) {
            emit paletteChanged(colors());
        });
    }
    row->addStretch(1);
}

QList<QColor> PaletteWidget::colors() const {
    QList<QColor> out;
    for (int i = 0; i < kMaxColors; ++i)
        out.append(_swatches[i]->color());
    return out;
}

void PaletteWidget::setColors(const QList<QColor>& c) {
    for (int i = 0; i < kMaxColors && i < c.size(); ++i)
        _swatches[i]->setColor(c[i]);
}
