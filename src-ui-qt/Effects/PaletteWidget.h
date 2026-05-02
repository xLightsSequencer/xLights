#pragma once
#include <QColor>
#include <QList>
#include <QWidget>

class ColorSwatchButton;

// 8-swatch color palette shown at the top of every effect panel.
// All effects in xLights share a palette; individual swatches can be
// enabled/disabled.  Emits paletteChanged whenever any color changes.
class PaletteWidget : public QWidget {
    Q_OBJECT
public:
    static constexpr int kMaxColors = 8;

    explicit PaletteWidget(QWidget* parent = nullptr);

    QList<QColor> colors() const;
    void setColors(const QList<QColor>& colors);

signals:
    void paletteChanged(const QList<QColor>& colors);

private:
    ColorSwatchButton* _swatches[kMaxColors] = {};
};
