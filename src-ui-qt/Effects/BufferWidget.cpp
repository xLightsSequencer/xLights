#include "BufferWidget.h"

#include <QPainter>
#include <QVBoxLayout>

BufferWidget::BufferWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(80);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void BufferWidget::setPixels(int w, int h, const QList<QColor>& pixels) {
    _bufW   = w;
    _bufH   = h;
    _pixels = pixels;
    update();
}

void BufferWidget::clear() {
    _bufW = _bufH = 0;
    _pixels.clear();
    update();
}

void BufferWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    QRect canvas = rect().adjusted(2, 2, -2, -2);
    p.fillRect(canvas, QColor(10, 10, 10));

    if (_bufW <= 0 || _bufH <= 0 || _pixels.isEmpty()) {
        p.setPen(QColor(70, 70, 70));
        p.drawText(canvas, Qt::AlignCenter, "Buffer\n(render pending)");
        return;
    }

    const float pw = float(canvas.width())  / _bufW;
    const float ph = float(canvas.height()) / _bufH;

    // xLights y=0 is physical bottom — flip vertically for display.
    for (int row = 0; row < _bufH && row * _bufW < _pixels.size(); ++row) {
        const float cy = canvas.y() + (_bufH - 1 - row) * ph;
        for (int col = 0; col < _bufW; ++col) {
            const int idx = row * _bufW + col;
            if (idx >= _pixels.size()) break;
            p.fillRect(QRectF(canvas.x() + col * pw, cy,
                              qMax(1.f, pw - 0.5f), qMax(1.f, ph - 0.5f)),
                       _pixels[idx]);
        }
    }
}
