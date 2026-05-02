#include "RhiCanvas.h"

RhiCanvas::RhiCanvas(QWidget* parent) : QRhiWidget(parent) {
    setSampleCount(1);
}

void RhiCanvas::initialize(QRhiCommandBuffer* cb) {
    Q_UNUSED(cb)
    // Pipeline setup will be added in Phase 3+ when we have shaders.
    // Phase 1: nothing to initialise beyond what QRhiWidget does internally.
}

void RhiCanvas::render(QRhiCommandBuffer* cb) {
    cb->beginPass(renderTarget(), backgroundColor(), {1.0f, 0});
    paintCanvas(cb);
    cb->endPass();
}
