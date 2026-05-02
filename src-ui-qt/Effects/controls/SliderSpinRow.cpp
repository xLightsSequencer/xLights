#include "SliderSpinRow.h"

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>

SliderSpinRow::SliderSpinRow(const QString& label, int min, int max,
                              double divisor, double defaultVal,
                              QWidget* parent)
    : QWidget(parent), _divisor(divisor > 0 ? divisor : 1.0) {
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    auto* lbl = new QLabel(label, this);
    lbl->setFixedWidth(120);
    lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lay->addWidget(lbl);

    _slider = new QSlider(Qt::Horizontal, this);
    _slider->setRange(min, max);
    _slider->setValue(int(defaultVal * _divisor));
    lay->addWidget(_slider, 1);

    _spin = new QDoubleSpinBox(this);
    _spin->setRange(min / _divisor, max / _divisor);
    _spin->setDecimals(_divisor == 1.0 ? 0 : 1);
    _spin->setSingleStep(1.0 / _divisor);
    _spin->setValue(defaultVal);
    _spin->setFixedWidth(72);
    lay->addWidget(_spin);

    connect(_slider, &QSlider::valueChanged, this, [this](int v) {
        if (_updating) return;
        _updating = true;
        double val = v / _divisor;
        _spin->setValue(val);
        _updating = false;
        emit valueChanged(val);
    });
    connect(_spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double v) {
        if (_updating) return;
        _updating = true;
        _slider->setValue(int(v * _divisor));
        _updating = false;
        emit valueChanged(v);
    });
}

double SliderSpinRow::value() const { return _spin->value(); }

void SliderSpinRow::setValue(double v) {
    _updating = true;
    _spin->setValue(v);
    _slider->setValue(int(v * _divisor));
    _updating = false;
}
