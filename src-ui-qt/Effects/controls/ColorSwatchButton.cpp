#include "ColorSwatchButton.h"

#include <QColorDialog>
#include <QMouseEvent>
#include <QPainter>

ColorSwatchButton::ColorSwatchButton(const QColor& initial, QWidget* parent)
    : QFrame(parent), _color(initial)
{
    setFixedSize(28, 22);
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Sunken);
    setCursor(Qt::PointingHandCursor);
    setToolTip("Click to change color");
}

void ColorSwatchButton::setColor(const QColor& c) {
    if (_color == c) return;
    _color = c;
    update();
    emit colorChanged(_color);
}

void ColorSwatchButton::mousePressEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton) { QFrame::mousePressEvent(e); return; }
    QColor picked = QColorDialog::getColor(_color, this, "Pick Color",
                                           QColorDialog::ShowAlphaChannel);
    if (picked.isValid()) setColor(picked);
}

void ColorSwatchButton::paintEvent(QPaintEvent*) {
    QPainter p(this);
    // Checkerboard behind alpha
    const int cs = 5;
    for (int y = 0; y < height(); y += cs)
        for (int x = 0; x < width(); x += cs)
            p.fillRect(x, y, cs, cs, ((x / cs + y / cs) % 2) ? QColor(200, 200, 200) : Qt::white);
    p.fillRect(rect(), _color);
    p.setPen(QColor(0, 0, 0, 80));
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}
