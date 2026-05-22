#pragma once
#include <QColor>
#include <QList>
#include <QPointF>
#include <QSet>
#include <QWidget>

// Displays a model's physical node layout and supports:
//   • Highlighting a subset of nodes (driven by the current table row selection)
//   • Rubber-band lasso selection that emits the selected node indices
class ModelNodePreview : public QWidget {
    Q_OBJECT
public:
    explicit ModelNodePreview(QWidget* parent = nullptr);

    // Load node positions (normalized [0,1]×[0,1]).
    void setNodePositions(const QList<QPointF>& positions);

    // Highlight the given 0-based node indices in color.
    // Pass an empty list to clear all highlights.
    void highlightNodes(const QList<int>& indices,
                        const QColor& color = QColor(0xff, 0xd7, 0x00));

    void clearHighlight();

    QSize sizeHint() const override { return {240, 200}; }

signals:
    // Emitted when the user finishes a lasso drag.
    // indices are 0-based node indices that fell inside the rubber-band rect.
    void nodesLassoed(const QList<int>& indices);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    // Map a normalized position to widget pixel coordinates (with padding).
    QPointF toWidget(const QPointF& norm) const;

    QList<QPointF> _positions;
    QSet<int>      _highlighted;
    QColor         _hlColor  = QColor(0xff, 0xd7, 0x00);

    bool   _lassoing = false;
    QPoint _lassoP1, _lassoP2;
};
