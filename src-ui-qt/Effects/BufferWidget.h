#pragma once
#include <QColor>
#include <QList>
#include <QWidget>

// Draws the raw 2-D render buffer (effect output before model mapping).
// Pixel data is pushed here after each src-core render.
// Blend mode and buffer-style controls live in the Layer and Buffer tabs
// of EffectPanelWidget — they are no longer part of this canvas widget.
class BufferWidget : public QWidget {
    Q_OBJECT
public:
    explicit BufferWidget(QWidget* parent = nullptr);
    QSize sizeHint() const override { return {260, 180}; }

    void setPixels(int w, int h, const QList<QColor>& pixels);
    void clear();

protected:
    void paintEvent(QPaintEvent*) override;

private:
    int           _bufW = 0;
    int           _bufH = 0;
    QList<QColor> _pixels;
};
