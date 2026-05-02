#pragma once
#include <QColor>
#include <QFrame>

// Clickable color swatch — opens QColorDialog on click, emits colorChanged.
class ColorSwatchButton : public QFrame {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
public:
    explicit ColorSwatchButton(const QColor& initial = Qt::white,
                               QWidget* parent = nullptr);

    QColor color() const { return _color; }
    void   setColor(const QColor& c);

signals:
    void colorChanged(const QColor& color);

protected:
    void mousePressEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*) override;

private:
    QColor _color;
};
