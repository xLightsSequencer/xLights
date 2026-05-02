#pragma once
#include <QWidget>
class QSlider;
class QDoubleSpinBox;
class QLabel;

// Horizontal row: label | slider | spinbox
// Supports integer and float values (via divisor).
class SliderSpinRow : public QWidget {
    Q_OBJECT
public:
    SliderSpinRow(const QString& label, int min, int max,
                  double divisor, double defaultVal,
                  QWidget* parent = nullptr);

    double value() const;
    void   setValue(double v);

signals:
    void valueChanged(double v);

private:
    QSlider*      _slider;
    QDoubleSpinBox* _spin;
    double          _divisor;
    bool            _updating = false;
};
