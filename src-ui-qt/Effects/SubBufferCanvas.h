#pragma once
#include <QWidget>

// Interactive canvas showing the buffer coordinate space.
// Normal mode:   0–100 in each axis.
// Oversized mode: -100–200 in each axis (allows sub-buffer outside the model boundary).
// A dashed box shows the current left/bottom/right/top % region.
// Drag the box body to move, drag edges/corners to resize.
// Y axis matches xLights: 0 = bottom, 100 = top.
class SubBufferCanvas : public QWidget {
    Q_OBJECT
public:
    explicit SubBufferCanvas(QWidget* parent = nullptr);
    QSize sizeHint()        const override { return {200, 200}; }
    QSize minimumSizeHint() const override { return {120, 120}; }

    void setRegion(int left, int bottom, int right, int top);
    void setOversized(bool oversized);
    bool isOversized() const { return _oversized; }

signals:
    void regionChanged(int left, int bottom, int right, int top);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    enum HitZone {
        None, Body,
        EdgeL, EdgeR, EdgeT, EdgeB,
        CornerTL, CornerTR, CornerBL, CornerBR
    };

    int  _left = 0, _bottom = 0, _right = 100, _top = 100;
    bool _oversized = false;

    // Coordinate space: [_minCoord, _maxCoord] in each axis.
    static constexpr int kNormalMin   =    0;
    static constexpr int kNormalMax   =  100;
    static constexpr int kOverMin     = -100;
    static constexpr int kOverMax     =  200;

    int coordMin() const { return _oversized ? kOverMin   : kNormalMin; }
    int coordMax() const { return _oversized ? kOverMax   : kNormalMax; }
    int coordRange() const { return coordMax() - coordMin(); }

    // Drag state
    bool    _dragging = false;
    HitZone _dragZone = None;
    QPointF _pressWidget;
    int     _origL = 0, _origB = 0, _origR = 100, _origT = 100;

    QRectF  boxRect()                         const;
    QPointF bufToWidget(double bx, double by) const;
    QPointF widgetToBuf(double wx, double wy) const;
    QRectF  handleRect(QPointF pt)            const;
    HitZone hitTest(QPointF pos)              const;
    void    applyDrag(QPointF pos);
    void    clamp();

    static constexpr float kHandleR = 5.0f;
    static constexpr float kHitR    = 8.0f;
};
