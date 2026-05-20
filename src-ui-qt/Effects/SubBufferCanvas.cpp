#include "SubBufferCanvas.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <cmath>

SubBufferCanvas::SubBufferCanvas(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
}

// ── Coordinate helpers ────────────────────────────────────────────────────────

// Buffer coords → widget pixels.
// Y: 0 = model bottom → widget bottom; 100 (or max) = model top → widget top.
QPointF SubBufferCanvas::bufToWidget(double bx, double by) const {
    const double r = coordRange();
    const double mn = coordMin();
    return { (bx - mn) / r * width(),
             (1.0 - (by - mn) / r) * height() };
}

// Widget pixels → buffer coords (floating-point, not clamped).
QPointF SubBufferCanvas::widgetToBuf(double wx, double wy) const {
    const double r = coordRange();
    const double mn = coordMin();
    return { wx / width() * r + mn,
             (1.0 - wy / height()) * r + mn };
}

QRectF SubBufferCanvas::boxRect() const {
    const QPointF tl = bufToWidget(_left,  _top);
    const QPointF br = bufToWidget(_right, _bottom);
    return QRectF(tl, br).normalized();
}

QRectF SubBufferCanvas::handleRect(QPointF pt) const {
    return QRectF(pt.x() - kHandleR, pt.y() - kHandleR, kHandleR*2, kHandleR*2);
}

// ── Hit testing ───────────────────────────────────────────────────────────────

SubBufferCanvas::HitZone SubBufferCanvas::hitTest(QPointF p) const {
    const QRectF box = boxRect();
    const QPointF tl(box.left(), box.top()),    tr(box.right(), box.top());
    const QPointF bl(box.left(), box.bottom()), br(box.right(), box.bottom());
    const QPointF mt(box.center().x(), box.top()),    mb(box.center().x(), box.bottom());
    const QPointF ml(box.left(), box.center().y()),   mr(box.right(), box.center().y());

    auto near = [&](QPointF q) {
        double dx = p.x()-q.x(), dy = p.y()-q.y();
        return std::sqrt(dx*dx+dy*dy) < kHitR;
    };
    if (near(tl)) return CornerTL;
    if (near(tr)) return CornerTR;
    if (near(bl)) return CornerBL;
    if (near(br)) return CornerBR;
    if (near(mt)) return EdgeT;
    if (near(mb)) return EdgeB;
    if (near(ml)) return EdgeL;
    if (near(mr)) return EdgeR;
    if (box.contains(p)) return Body;
    return None;
}

// ── Drag ─────────────────────────────────────────────────────────────────────

void SubBufferCanvas::applyDrag(QPointF pos) {
    const QPointF origBuf = widgetToBuf(_pressWidget.x(), _pressWidget.y());
    const QPointF curBuf  = widgetToBuf(pos.x(), pos.y());
    const double  dx = curBuf.x() - origBuf.x();
    const double  dy = curBuf.y() - origBuf.y();

    const int mn = coordMin(), mx = coordMax();
    auto cv = [&](int v) { return qBound(mn, v, mx); };

    int l = _origL, b = _origB, r = _origR, t = _origT;

    switch (_dragZone) {
    case Body: {
        const int w = _origR - _origL, h = _origT - _origB;
        l = cv(int(_origL + dx));
        b = cv(int(_origB + dy));
        r = qMin(l + w, mx);  t = qMin(b + h, mx);
        if (r == mx) l = mx - w;
        if (t == mx) b = mx - h;
        break;
    }
    case EdgeL:    l = cv(int(_origL + dx)); break;
    case EdgeR:    r = cv(int(_origR + dx)); break;
    case EdgeT:    t = cv(int(_origT + dy)); break;
    case EdgeB:    b = cv(int(_origB + dy)); break;
    case CornerTL: l = cv(int(_origL + dx)); t = cv(int(_origT + dy)); break;
    case CornerTR: r = cv(int(_origR + dx)); t = cv(int(_origT + dy)); break;
    case CornerBL: l = cv(int(_origL + dx)); b = cv(int(_origB + dy)); break;
    case CornerBR: r = cv(int(_origR + dx)); b = cv(int(_origB + dy)); break;
    default: return;
    }

    // Enforce min 1-unit box.
    if (l >= r) { if (_dragZone==EdgeL||_dragZone==CornerTL||_dragZone==CornerBL) l=r-1; else r=l+1; }
    if (b >= t) { if (_dragZone==EdgeB||_dragZone==CornerBL||_dragZone==CornerBR) b=t-1; else t=b+1; }

    _left=l; _bottom=b; _right=r; _top=t;
    update();
    emit regionChanged(_left, _bottom, _right, _top);
}

void SubBufferCanvas::clamp() {
    const int mn = coordMin(), mx = coordMax();
    _left   = qBound(mn, _left,   mx);
    _right  = qBound(mn, _right,  mx);
    _bottom = qBound(mn, _bottom, mx);
    _top    = qBound(mn, _top,    mx);
    if (_left  >= _right)  _right  = _left  + 1;
    if (_bottom >= _top)   _top    = _bottom + 1;
}

// ── Public API ────────────────────────────────────────────────────────────────

void SubBufferCanvas::setRegion(int left, int bottom, int right, int top) {
    _left=left; _bottom=bottom; _right=right; _top=top;
    clamp();
    update();
}

void SubBufferCanvas::setOversized(bool oversized) {
    if (_oversized == oversized) return;
    _oversized = oversized;
    clamp();  // re-clamp to new coordinate range
    update();
    emit regionChanged(_left, _bottom, _right, _top);
}

// ── Paint ─────────────────────────────────────────────────────────────────────

void SubBufferCanvas::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF canvas(0, 0, width(), height());
    p.fillRect(canvas, QColor(18, 18, 22));

    if (_oversized) {
        // Highlight the normal 0–100 region so the user can see where
        // the model boundary is within the extended coordinate space.
        const QPointF tl = bufToWidget(0,   100);
        const QPointF br = bufToWidget(100, 0);
        const QRectF  normalRect = QRectF(tl, br).normalized();
        p.fillRect(normalRect, QColor(35, 45, 55));

        // Label the normal region boundary.
        p.setPen(QPen(QColor(70, 90, 110), 1, Qt::DashDotLine));
        p.setBrush(Qt::NoBrush);
        p.drawRect(normalRect.adjusted(0.5, 0.5, -0.5, -0.5));

        p.setPen(QColor(80, 105, 130, 200));
        QFont sf; sf.setPixelSize(8); p.setFont(sf);
        p.drawText(normalRect.adjusted(3, 2, 0, 0), Qt::AlignTop | Qt::AlignLeft, "0,0");
        p.drawText(normalRect.adjusted(0, 0, -3, -2), Qt::AlignBottom | Qt::AlignRight, "100,100");
    }

    // Grid lines every 25 units across the full coordinate range.
    p.setPen(QPen(QColor(45, 45, 55), 1, Qt::DotLine));
    const int step = 25;
    for (int v = coordMin(); v <= coordMax(); v += step) {
        const QPointF hL = bufToWidget(coordMin(), v);
        const QPointF hR = bufToWidget(coordMax(), v);
        const QPointF vT = bufToWidget(v, coordMax());
        const QPointF vB = bufToWidget(v, coordMin());
        p.drawLine(hL, hR);
        p.drawLine(vT, vB);
    }

    // Sub-buffer region fill + dashed border.
    const QRectF box = boxRect();
    p.fillRect(box, QColor(60, 80, 120, 120));

    QPen dashPen(QColor(255, 220, 40, 230), 1.5, Qt::DashLine);
    dashPen.setDashPattern({6, 3});
    p.setPen(dashPen);
    p.setBrush(Qt::NoBrush);
    p.drawRect(box);

    // Handles at corners and edge midpoints.
    p.setPen(QPen(QColor(255, 220, 40), 1));
    p.setBrush(QColor(255, 255, 255, 200));
    for (QPointF pt : {
             QPointF{box.left(),         box.top()},
             QPointF{box.right(),        box.top()},
             QPointF{box.left(),         box.bottom()},
             QPointF{box.right(),        box.bottom()},
             QPointF{box.center().x(),   box.top()},
             QPointF{box.center().x(),   box.bottom()},
             QPointF{box.left(),         box.center().y()},
             QPointF{box.right(),        box.center().y()},
         })
        p.drawEllipse(handleRect(pt));

    // Percentage label inside box.
    if (box.width() > 44 && box.height() > 20) {
        p.setPen(QColor(210, 210, 210, 200));
        QFont f; f.setPixelSize(9); p.setFont(f);
        p.drawText(box.adjusted(4,2,-4,-2), Qt::AlignCenter | Qt::TextWordWrap,
                   QString("L%1 R%2\nB%3 T%4").arg(_left).arg(_right).arg(_bottom).arg(_top));
    }

    // Outer border.
    p.setPen(QPen(QColor(70, 70, 80), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(canvas.adjusted(0,0,-1,-1));
}

// ── Mouse events ──────────────────────────────────────────────────────────────

void SubBufferCanvas::mousePressEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton) return;
    _pressWidget = e->position();
    _dragZone    = hitTest(_pressWidget);

    if (_dragZone == None) {
        // Start a fresh 1-unit box at the clicked point.
        const QPointF bp = widgetToBuf(_pressWidget.x(), _pressWidget.y());
        const int mn = coordMin(), mx = coordMax();
        const int bx = qBound(mn, int(bp.x()), mx);
        const int by = qBound(mn, int(bp.y()), mx);
        _left=bx; _right=qMin(bx+1,mx);
        _bottom=by; _top=qMin(by+1,mx);
        _dragZone = CornerBR;
    }

    _origL=_left; _origB=_bottom; _origR=_right; _origT=_top;
    _dragging = true;
    update();
}

void SubBufferCanvas::mouseMoveEvent(QMouseEvent* e) {
    if (_dragging) {
        applyDrag(e->position());
        return;
    }
    switch (hitTest(e->position())) {
    case Body:     setCursor(Qt::SizeAllCursor);   break;
    case EdgeL:
    case EdgeR:    setCursor(Qt::SizeHorCursor);   break;
    case EdgeT:
    case EdgeB:    setCursor(Qt::SizeVerCursor);   break;
    case CornerTL:
    case CornerBR: setCursor(Qt::SizeFDiagCursor); break;
    case CornerTR:
    case CornerBL: setCursor(Qt::SizeBDiagCursor); break;
    default:       setCursor(Qt::CrossCursor);     break;
    }
}

void SubBufferCanvas::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton || !_dragging) return;
    applyDrag(e->position());
    _dragging = false;
    setCursor(Qt::CrossCursor);
}
