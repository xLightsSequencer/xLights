#include "HousePreviewWidget.h"

HousePreviewWidget::HousePreviewWidget(QWidget* parent)
    : ModelLayoutCanvas(parent) {}

void HousePreviewWidget::setModelPixels(const QString& modelName,
                                        const QList<QColor>& pixels) {
    auto it = index().find(modelName);
    if (it == index().end()) return;
    ModelData& d = models()[it.value()];
    d.pixels    = pixels;
    d.hasPixels = true;
    update();
}

void HousePreviewWidget::clearPixels() {
    for (auto& d : models()) {
        d.pixels.fill(QColor(40, 40, 40), d.info.nodePositions.size());
        d.hasPixels = false;
    }
    update();
}
