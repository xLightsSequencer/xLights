#include "BufferWidget.h"

#include <QFormLayout>
#include <QFrame>
#include <QPainter>
#include <QVBoxLayout>

static const QStringList kBlendModes = {
    "Normal", "Effect 1", "Effect 2",
    "1 is Mask", "2 is Mask", "1 reveals 2", "2 reveals 1",
    "Shadow 1 on 2", "Shadow 2 on 1",
    "Layered", "Average", "Combine",
    "Bottom-Top", "Left-Right",
};

static const QStringList kBufferStyles = {
    "Default", "Per Model Default",
    "Per Model Strand - Horizontal",
    "Per Model Strand - Vertical",
    "Per Model Column",
    "Per Model Row",
    "Single Line",
};

BufferWidget::BufferWidget(QWidget* parent) : QWidget(parent) {
    auto* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(4, 4, 4, 4);
    vbox->setSpacing(4);

    // ── Controls row ────────────────────────────────────────────────────────
    auto* form = new QFormLayout;
    form->setContentsMargins(0, 0, 0, 0);
    form->setSpacing(3);

    _blendCombo  = new QComboBox;
    _blendCombo->addItems(kBlendModes);
    form->addRow("Blend:", _blendCombo);

    _bufferCombo = new QComboBox;
    _bufferCombo->addItems(kBufferStyles);
    form->addRow("Buffer:", _bufferCombo);

    vbox->addLayout(form);

    // ── Buffer canvas ───────────────────────────────────────────────────────
    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    vbox->addWidget(sep);

    // The canvas is this widget itself — paintEvent draws into the remaining area.
    vbox->addStretch(1);

    connect(_blendCombo,  &QComboBox::currentTextChanged,
            this, &BufferWidget::blendModeChanged);
    connect(_bufferCombo, &QComboBox::currentTextChanged,
            this, &BufferWidget::bufferStyleChanged);
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

QString BufferWidget::blendMode()   const { return _blendCombo->currentText(); }
QString BufferWidget::bufferStyle() const { return _bufferCombo->currentText(); }

void BufferWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);

    // Canvas rect = area below the form controls (~60px from top)
    const int topOff = 60;
    QRect canvas(4, topOff, width() - 8, height() - topOff - 4);
    p.fillRect(canvas, QColor(10, 10, 10));

    if (_bufW <= 0 || _bufH <= 0 || _pixels.isEmpty()) {
        p.setPen(QColor(80, 80, 80));
        p.drawText(canvas, Qt::AlignCenter, "Buffer\n(render pending)");
        return;
    }

    const float pw = float(canvas.width())  / _bufW;
    const float ph = float(canvas.height()) / _bufH;

    // xLights pixel buffers have y=0 at the physical bottom, so flip vertically
    // when drawing: row=0 (bottom of model) maps to the bottom of the canvas.
    for (int row = 0; row < _bufH && row * _bufW < _pixels.size(); ++row) {
        const float canvasY = canvas.y() + (_bufH - 1 - row) * ph;
        for (int col = 0; col < _bufW; ++col) {
            int idx = row * _bufW + col;
            if (idx >= _pixels.size()) break;
            QRectF r(canvas.x() + col * pw, canvasY,
                     qMax(1.f, pw - 0.5f), qMax(1.f, ph - 0.5f));
            p.fillRect(r, _pixels[idx]);
        }
    }
}
