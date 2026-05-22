#pragma once
#include <QColor>
#include <QList>
#include <QPixmap>
#include <QPointF>
#include <QSet>
#include <QWidget>

// Displays a model's physical node layout and supports:
//   • Highlighting a subset of nodes (driven by the current table row selection)
//   • Rubber-band lasso selection that emits the selected node indices
//
// All complex drawing is done to an off-screen QPixmap so that paintEvent
// only issues a single drawPixmap call — avoiding the QPainter-on-widget
// crash that occurs when the backing store's raster buffer isn't yet ready
// on Windows (access violation reading 0x3F0 inside Qt6Guid.dll).
class ModelNodePreview : public QWidget {
    Q_OBJECT
public:
    explicit ModelNodePreview(QWidget* parent = nullptr);

    void setNodePositions(const QList<QPointF>& positions);
    void highlightNodes(const QList<int>& indices,
                        const QColor& color = QColor(0xff, 0xd7, 0x00));
    void clearHighlight();

    QSize sizeHint() const override { return {240, 200}; }

signals:
    void nodesLassoed(const QList<int>& indices);

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    void rebuildCache();              // render nodes → _cache pixmap
    QPointF toWidget(const QPointF& norm) const;

    QList<QPointF> _positions;
    QSet<int>      _highlighted;
    QColor         _hlColor  = QColor(0xff, 0xd7, 0x00);
    QPixmap        _cache;            // off-screen rendered node dots

    bool   _lassoing = false;
    QPoint _lassoP1, _lassoP2;
};
