#pragma once
#include "ModelLayoutCanvas.h"

// House preview: extends ModelLayoutCanvas with live per-node pixel updates
// driven by the sequencer render pipeline. All geometry/drawing is in the base.
class HousePreviewWidget : public ModelLayoutCanvas {
    Q_OBJECT
public:
    explicit HousePreviewWidget(QWidget* parent = nullptr);

    // Update the node colors for one model. pixels[i] corresponds to globalPositions[i].
    void setModelPixels(const QString& modelName, const QList<QColor>& pixels);

    // Reset all node colors to the dim placeholder.
    void clearPixels();
};
