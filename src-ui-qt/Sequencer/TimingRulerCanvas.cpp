#include "TimingRulerCanvas.h"
#include "SequencerModel.h"
#include <QPainter>

TimingRulerCanvas::TimingRulerCanvas(SequencerModel* model, QWidget* parent)
    : QWidget(parent), _model(model) {
    setFixedHeight(24);
    connect(model, &SequencerModel::geometryChanged, this, QOverload<>::of(&QWidget::update));
}

void TimingRulerCanvas::setScrollOffset(int xOffset) { _xOff = xOffset; update(); }
void TimingRulerCanvas::setPlayhead(int frame)        { _playhead = frame; update(); }

void TimingRulerCanvas::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor(0x22, 0x22, 0x22));
    p.setPen(QColor(0x55, 0x55, 0x55));
    p.drawLine(0, height()-1, width(), height()-1);

    const int fps   = _model->fps();
    const int total = _model->totalFrames();
    const double ppf = _model->pixelsPerFrame();

    // Draw tick marks every second; label every 5 s.
    p.setFont(QFont("Segoe UI", 8));
    for (int f = 0; f <= total; f += fps) {
        int x = _model->xAt(f) - _xOff;
        if (x < -40 || x > width() + 40) continue;
        bool major = (f % (fps * 5) == 0);
        p.setPen(major ? QColor(0xaa, 0xaa, 0xaa) : QColor(0x66, 0x66, 0x66));
        p.drawLine(x, major ? 6 : 14, x, height() - 1);
        if (major) {
            int secs = f / fps;
            p.setPen(Qt::white);
            p.drawText(QRect(x + 2, 0, 60, 20), Qt::AlignVCenter,
                       QString("%1:%2").arg(secs/60).arg(secs%60, 2, 10, QChar('0')));
        }
    }

    // Playhead
    int phX = _model->xAt(_playhead) - _xOff;
    if (phX >= 0 && phX < width()) {
        p.setPen(QPen(QColor(0xff, 0x44, 0x44), 2));
        p.drawLine(phX, 0, phX, height());
    }
}
