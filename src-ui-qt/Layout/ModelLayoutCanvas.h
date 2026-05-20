#pragma once
#include "../Bridge/QtSequenceDoc.h"
#include <QColor>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QWidget>

// Base canvas that draws all models in their physical yard layout.
// Owns geometry (globalPositions → widget coords), bounding box, node rendering,
// group highlight, and single-model selection highlight.
//
// Subclass to add live pixel updates (HousePreviewWidget) or interactive editing
// (layout window canvas).
class ModelLayoutCanvas : public QWidget {
    Q_OBJECT
public:
    explicit ModelLayoutCanvas(QWidget* parent = nullptr);
    QSize sizeHint() const override { return {800, 400}; }

    // Load model positions from the current sequence. Resets all node colors to dim gray.
    void loadLayout(const QtSequenceInfo& seq);

    // Highlight the member models of a group with a dashed outline.
    // Pass an empty string to clear.
    void setGroupHighlight(const QString& groupName, const QStringList& memberModels);

    // Single-model selection highlight (for the layout window list).
    // The selected model gets a solid yellow border; others dim slightly.
    // Independent from the group highlight — both can be active simultaneously.
    void setSelectedModel(const QString& modelName);
    void clearSelection();

signals:
    void modelClicked(const QString& modelName);

protected:
    struct ModelData {
        QtModelInfo   info;
        QList<QColor> pixels;    // node colors; defaults to dim gray
        bool          hasPixels = false;
    };

    // Subclass access to the model list for pixel updates.
    QList<ModelData>&        models()       { return _models; }
    const QList<ModelData>&  models() const { return _models; }
    const QMap<QString,int>& index()  const { return _index; }

    // Convert a layout-space point to widget pixels.
    QPointF toWidget(double gx, double gy) const;

    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;

private:
    QList<ModelData>   _models;
    QMap<QString, int> _index;

    double _minX = 0, _minY = 0, _maxX = 1, _maxY = 1;
    mutable float _nodeR = 3.0f;

    QString     _highlightGroup;
    QStringList _highlightMembers;
    QString     _selectedModel;

    void recomputeBounds();
};
