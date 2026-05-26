#pragma once
#include "../Bridge/QtSequenceDoc.h"
#include <QColor>
#include <QHash>
#include <QList>
#include <QMap>
#include <QSet>
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

    // Selection.  setSelectedModel replaces the selection with a single
    // model; setSelectedModels replaces it with a set; clearSelection
    // empties it.  Selected models get a solid yellow border in paintRects.
    // The "primary" model is the most recently added one — drives the
    // property panel.  Independent from the group highlight.
    void setSelectedModel(const QString& modelName);
    void setSelectedModels(const QStringList& modelNames);
    void clearSelection();

    // Placement mode (phase 20f).  While true, the next mouse click does NOT
    // emit modelClicked — it emits placementClicked with the world-space
    // coordinates of the click, then auto-exits placement mode.  Cursor
    // switches to a crosshair while waiting.  Escape cancels.
    void setPlacementMode(bool on);
    bool isPlacementMode() const { return _placementMode; }

signals:
    void modelClicked(const QString& modelName);

    // Fired once when the user clicks the canvas while in placement mode.
    // wx/wy are in the canvas's world-space coordinates (same space that
    // ModelScreenLocation::SetWorldPos uses).
    void placementClicked(double wx, double wy);

    // Fired when the user cancels placement mode (Escape, or the caller
    // explicitly turns it off).  Allows LayoutWindow to discard a pending
    // placement and clean up.
    void placementCancelled();

    // Fired on mouse release after a drag actually moved one or more
    // models — every selected model's ModelScreenLocation has already
    // been mutated by then.  LayoutWindow uses this to persist + refresh
    // all of them.  No emission for clicks that didn't exceed the drag
    // threshold (those still emit modelClicked).
    void modelDragged(const QString& modelName);
    void modelsDragged(const QStringList& modelNames);

    // Fired when the canvas-driven selection changes (Ctrl+click).  Caller
    // (LayoutWindow) syncs the list-widget selection from this.
    void selectionChanged(const QStringList& selectedNames);

protected:
    // Dot-mode model data (used by HousePreviewWidget for live colors).
    struct ModelData {
        QtModelInfo   info;
        QList<QColor> pixels;    // node colors; defaults to dim gray
        bool          hasPixels = false;
    };

    // Rect-mode model: bounding box + individual node world positions (Y-up).
    struct ModelRect {
        QString        name;
        float          left   = 0.f;
        float          right  = 0.f;
        float          top    = 0.f;    // GetTop()    > GetBottom()
        float          bottom = 0.f;
        bool           isGroup = false;
        QList<QPointF> nodeWorldPos;    // per-node world-space coords (Y-up)
    };

    // Subclass access to the dot-mode model list for pixel updates.
    QList<ModelData>&        models()       { return _models; }
    const QList<ModelData>&  models() const { return _models; }
    const QMap<QString,int>& index()  const { return _index; }

    // Convert a dot-mode layout-space point to widget pixels (Y down = down).
    QPointF toWidget(double gx, double gy) const;

    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;

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
    // Primary selection (drives the property panel) + full set (drives
    // multi-drag and visual highlight).  _selectedModel is always the most
    // recently added entry in _selectedModels, or empty when both are empty.
    QString     _selectedModel;
    QSet<QString> _selectedModels;

    // Placement mode (phase 20f).
    bool        _placementMode = false;

    // Rubber-band selection (phase 22).  Press on empty rect-mode space
    // starts a band; mouseMove updates the current corner; release commits
    // selection of every model whose rect intersects.  _rubberBandActive
    // means "in progress this drag"; _rubberBandShown means "the rect has
    // moved enough to be visible" — distinguishes a real rubber-band from
    // a press-release-without-motion that should clear selection instead.
    bool        _rubberBandActive = false;
    bool        _rubberBandShown  = false;
    QPointF     _rubberBandStart;
    QPointF     _rubberBandCurrent;

    // Live ModelManager so mousePress can mutate a model's world position
    // during a drag.  Populated by loadLayoutFromManager; reset by loadLayout.
    ModelManager* _mm = nullptr;

    // Drag-to-move state.  _dragModel is the "anchor" — the model the user
    // clicked on; populated from the hit-test on press.  When the press is
    // on a model already in _selectedModels, every selected model is part
    // of the drag (multi-drag).  Otherwise only _dragModel moves and the
    // selection is replaced by it.  _dragStarts records each dragging
    // model's world position at press time so the delta is applied to the
    // STARTING position, not the cumulative moving position (drift-free).
    QString             _dragModel;
    QPointF             _dragPressClick;
    QHash<QString,QPointF> _dragStarts;     // name → (worldX, worldY) at press
    bool                _dragMoved = false; // exceeded threshold this drag

    void recomputeBounds();
    void paintRects(QPainter& p);
};
