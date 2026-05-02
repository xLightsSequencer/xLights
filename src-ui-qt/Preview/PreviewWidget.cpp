#include "PreviewWidget.h"

#include <QPainter>
#include <QtMath>
#include <cmath>

PreviewWidget::PreviewWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(160);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

// ── Public slots ──────────────────────────────────────────────────────────────

void PreviewWidget::setResult(const QtEffectRenderer::Result& r,
                               const QList<QPointF>& nodePositions) {
    if (!r.isValid()) return;
    _bufW          = r.w;
    _bufH          = r.h;
    _pixels        = r.pixels;
    _nodePositions = nodePositions;
    _label.clear();
    update();
}

void PreviewWidget::showEffect(const QString& name, int pixelCount) {
    _label         = name;
    _bufW          = pixelCount;
    _bufH          = 1;
    _pixels        = mockColors(name, pixelCount);
    _nodePositions.clear();
    update();
}

void PreviewWidget::clear() {
    _bufW = _bufH = 0;
    _pixels.clear();
    _nodePositions.clear();
    _label.clear();
    update();
}

// ── Paint ─────────────────────────────────────────────────────────────────────

void PreviewWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Dark background matching the xLights OpenGL preview
    p.fillRect(rect(), QColor(30, 30, 30));

    if (_pixels.isEmpty() || _bufW == 0) {
        p.setPen(QColor(90, 90, 90));
        p.drawText(rect(), Qt::AlignCenter, "No selection");
        return;
    }

    const int   labelH = _label.isEmpty() ? 0 : 18;
    const QRect canvas(4, 4, width() - 8, height() - 8 - labelH);

    const int N = _pixels.size();
    p.setPen(Qt::NoPen);

    // ── Physical node positions (from show file) ──────────────────────────
    if (!_nodePositions.isEmpty() && _nodePositions.size() == N) {
        // Use a square rendering area so circular / tree shapes aren't
        // distorted by a wide-and-short canvas.  Centre the square; the
        // rest of the canvas stays dark.
        int side = qMin(canvas.width(), canvas.height());
        int sqX  = canvas.x() + (canvas.width()  - side) / 2;
        int sqY  = canvas.y() + (canvas.height() - side) / 2;

        // Node radius: use spatial density so sparse group views (few dots)
        // get large visible circles and dense model views get small ones.
        // density = nodes per (side²) pixels of canvas area.
        const float density = float(N) / float(qMax(1, side * side));
        const float nodeR   = qBound(2.0f, 0.6f / std::sqrt(qMax(1e-6f, density)), 12.0f);

        for (int i = 0; i < N; ++i) {
            const QPointF& np = _nodePositions[i];
            float cx = float(sqX) + float(np.x()) * float(side);
            float cy = float(sqY) + float(np.y()) * float(side);
            p.setBrush(_pixels[i]);
            p.drawEllipse(QRectF(cx - nodeR, cy - nodeR, nodeR * 2.f, nodeR * 2.f));
        }

    // ── 2-D matrix/buffer ─────────────────────────────────────────────────
    } else if (_bufH > 1) {
        float cellW = float(canvas.width())  / _bufW;
        float cellH = float(canvas.height()) / _bufH;
        float nodeR = qMax(0.8f, qMin(cellW, cellH) * 0.42f);

        for (int row = 0; row < _bufH; ++row) {
            for (int col = 0; col < _bufW; ++col) {
                int idx = row * _bufW + col;
                if (idx >= N) break;
                float cx = canvas.x() + (col + 0.5f) * cellW;
                // Flip y: xLights row=0 is physical bottom; draw it at canvas bottom.
                float cy = canvas.y() + (_bufH - 1 - row + 0.5f) * cellH;
                p.setBrush(_pixels[idx]);
                p.drawEllipse(QRectF(cx - nodeR, cy - nodeR, nodeR * 2.f, nodeR * 2.f));
            }
        }

    // ── 1-D strand ────────────────────────────────────────────────────────
    } else {
        float cellW = float(canvas.width()) / _bufW;
        float cy    = canvas.y() + canvas.height() * 0.5f;
        float nodeR = qBound(1.5f, qMin(cellW * 0.45f, canvas.height() * 0.42f), 10.0f);

        for (int x = 0; x < _bufW && x < N; ++x) {
            float cx = canvas.x() + (x + 0.5f) * cellW;
            p.setBrush(_pixels[x]);
            p.drawEllipse(QRectF(cx - nodeR, cy - nodeR, nodeR * 2.f, nodeR * 2.f));
        }
    }

    if (!_label.isEmpty()) {
        p.setPen(QColor(170, 170, 170));
        p.drawText(QRect(0, height() - labelH, width(), labelH),
                   Qt::AlignCenter, _label);
    }
}

// ── Mock colors (placeholder before first real render) ────────────────────────

QList<QColor> PreviewWidget::mockColors(const QString& name, int count) {
    QList<QColor> out;
    out.reserve(count);
    uint  h    = qHash(name);
    float base = float(h % 360) / 360.f;

    if (name == "Rainbow") {
        for (int i = 0; i < count; ++i)
            out.append(QColor::fromHsvF(float(i) / count, 1.f, 1.f));
    } else if (name == "Off") {
        out.fill(Qt::black, count);
    } else if (name == "Bars") {
        for (int i = 0; i < count; ++i) {
            float hue = float(std::fmod(base + double(i % 6) / 6.0, 1.0));
            out.append(QColor::fromHsvF(hue, 1.f, 1.f));
        }
    } else if (name == "Fire") {
        for (int i = 0; i < count; ++i)
            out.append(QColor::fromHsvF(float(i) / count * 0.1f, 1.f, 1.f));
    } else if (name == "Sparkle" || name == "Twinkle") {
        for (int i = 0; i < count; ++i) {
            bool on = ((h + uint(i) * 7u) % 5u == 0u);
            out.append(on ? Qt::white : QColor(10, 10, 10));
        }
    } else {
        for (int i = 0; i < count; ++i) {
            float t   = float(i) / count;
            float hue = float(std::fmod(base + t * 0.4, 1.0));
            float val = 0.7f + 0.3f * float(qSin(t * float(M_PI) * 4));
            out.append(QColor::fromHsvF(hue, 0.85f, val));
        }
    }
    return out;
}
