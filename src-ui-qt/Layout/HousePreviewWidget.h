#pragma once
#include "../Bridge/QtSequenceDoc.h"
#include <QColor>
#include <QList>
#include <QMap>
#include <QString>
#include <QWidget>

// Shows all models in their physical yard/house layout simultaneously.
// Models are positioned using worldPosX/Y and scaled by scaleX/Y from the show file.
// Call setModelPixels() for each model to update its node colors.
class HousePreviewWidget : public QWidget {
    Q_OBJECT
public:
    explicit HousePreviewWidget(QWidget* parent = nullptr);
    QSize sizeHint() const override { return {800, 400}; }

    // Load model layout from current sequence info. Computes global bounding box.
    void loadLayout(const QtSequenceInfo& seq);

    // Update pixel colors for one model. pixels[i] corresponds to nodePositions[i].
    void setModelPixels(const QString& modelName, const QList<QColor>& pixels);

    // Clear all model pixel colors (set to dim placeholder).
    void clearPixels();

signals:
    // Emitted when the user clicks a model node in the layout.
    void modelClicked(const QString& modelName);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* ev) override;

private:
    struct ModelData {
        QtModelInfo      info;
        QList<QColor>    pixels;      // current node colors; empty = unrendered
        bool             hasPixels = false;
    };

    QList<ModelData>  _models;      // ordered for paint (z-order doesn't matter here)
    QMap<QString, int> _index;      // name → _models index

    // Global bounding box in layout units (layout space == world space).
    double _minX = 0, _minY = 0, _maxX = 1, _maxY = 1;

    // Map layout coords to widget coords.
    QPointF toWidget(double gx, double gy) const;
    // Recompute bounding box from all model extents.
    void    recomputeBounds();

    float _nodeR = 3.0f;   // node dot radius in pixels
};
