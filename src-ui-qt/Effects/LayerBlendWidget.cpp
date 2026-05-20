#include "LayerBlendWidget.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

static const QStringList kBlendModes = {
    "Normal", "Effect 1", "Effect 2",
    "1 is Mask", "2 is Mask", "1 reveals 2", "2 reveals 1",
    "Shadow 1 on 2", "Shadow 2 on 1",
    "Layered", "Average", "Combine",
    "Bottom-Top", "Left-Right",
};

LayerBlendWidget::LayerBlendWidget(QWidget* parent) : QWidget(parent) {
    auto* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(8, 8, 8, 8);
    vbox->setSpacing(10);

    auto* form = new QFormLayout;
    form->setContentsMargins(0, 0, 0, 0);
    form->setSpacing(6);

    _blendCombo = new QComboBox;
    _blendCombo->addItems(kBlendModes);
    form->addRow("Blend mode:", _blendCombo);

    vbox->addLayout(form);
    vbox->addStretch(1);

    connect(_blendCombo, &QComboBox::currentTextChanged,
            this, &LayerBlendWidget::blendModeChanged);
}

QString LayerBlendWidget::blendMode() const {
    return _blendCombo->currentText();
}

void LayerBlendWidget::setBlendMode(const QString& mode) {
    const int idx = _blendCombo->findText(mode);
    if (idx >= 0) _blendCombo->setCurrentIndex(idx);
}
