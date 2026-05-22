#pragma once
#include "../Bridge/QtSequenceDoc.h"
#include <QColor>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QWidget>

class ModelManager;

// Base canvas that draws all models in their physical yard layout.
// Two drawing modes:
//   Rect mode  — loadLayoutFromManager(): draws bounding-box rectangles derived
//                from the src-core ModelScreenLocation (used by LayoutWindow).
//   Dot mode   — loadLayout():            draws per-node dots at globalPositions
//                (used by HousePreviewWidget for live pixel updates).
class ModelLayoutCanvas : public QWidget {
    Q_OBJECT
public:
    explicit ModelLayoutCanvas(QWidget* parent = nullptr);
    QSize sizeHint() const override { return {800, 400}; }

    // Rect mode: load model bounding boxes from the src-core ModelManager.
    // Uses ModelScreenLocation::GetTop/Left/Right/Bottom() for each model.
    void loadLayoutFromManager(ModelManager* mm);

    // Dot mode: load per-node positions from QtSequenceInfo.
    // Resets all node colors to dim gray.
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
    // Dot-mode model data (used by HousePreviewWidget for live colors).
    struct ModelData {
        QtModelInfo   info;
        QList<QColor> pixels;    // node colors; defaults to dim gray
        bool          hasPixels = false;
    };

    // Rect-mode model bounding box (world-space, Y-up).
    struct ModelRect {
        QString name;
        float   left   = 0.f;
        float   right  = 0.f;
        float   top    = 0.f;    // GetTop()    > GetBottom()
        float   bottom = 0.f;
        bool    isGroup = false;
    };

    // Subclass access to the dot-mode model list for pixel updates.
    QList<ModelData>&        models()       { return _models; }
    const QList<ModelData>&  models() const { return _models; }
    const QMap<QString,int>& index()  const { return _index; }

    // Convert a dot-mode layout-space point to widget pixels (Y down = down).
    QPointF toWidget(double gx, double gy) const;

    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;

private:
    // Dot mode
    QList<ModelData>   _models;

    // Rect mode
    QList<ModelRect>   _rects;

    // Shared
    QMap<QString, int> _index;
    bool               _rectMode = false;

    double _minX = 0, _minY = 0, _maxX = 1920, _maxY = 1080;
    mutable float _nodeR = 3.0f;

    QString     _highlightGroup;
    QStringList _highlightMembers;
    QString     _selectedModel;

    void recomputeBounds();
    void paintRects(QPainter& p);
};
