#pragma once
#include "../Bridge/QtEffectRenderer.h"
#include <QColor>
#include <QList>
#include <QWidget>

// Displays effect output pixels as a model preview.
// For 1-row buffers (strands): horizontal pixel strip.
// For multi-row buffers (matrices): 2-D pixel grid.
// Call setResult() with live render data; showEffect() generates mock colors
// as a placeholder when no render result is available yet.
class PreviewWidget : public QWidget {
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget* parent = nullptr);
    QSize sizeHint() const override { return {400, 100}; }

public slots:
    // Live render result. nodePositions: normalized [0,1]×[0,1] per node;
    // empty → fall back to grid/strip layout.
    void setResult(const QtEffectRenderer::Result& r,
                   const QList<QPointF>& nodePositions = {});

    // Fallback mock display used before first render fires.
    void showEffect(const QString& name, int pixelCount = 80);

    void clear();

protected:
    void paintEvent(QPaintEvent*) override;

private:
    int            _bufW = 0;
    int            _bufH = 0;
    QList<QColor>  _pixels;
    QList<QPointF> _nodePositions;   // normalized; empty → grid/strip fallback
    QString        _label;

    static QList<QColor> mockColors(const QString& name, int count);
};
