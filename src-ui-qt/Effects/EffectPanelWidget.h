#pragma once

#include <QColor>
#include <QList>
#include <QVariantMap>
#include <QWidget>

class BufferWidget;
class PaletteWidget;
class QLabel;
class QScrollArea;
class QTimer;
class QVBoxLayout;
class SubBufferWidget;

class EffectPanelWidget : public QWidget {
    Q_OBJECT

public:
    explicit EffectPanelWidget(QWidget* parent = nullptr);

    // Show an effect with default settings (e.g. effect toolbar click).
    void    showEffect(const QString& effectName);
    // Show an effect pre-populated with values from a raw xsq settings string.
    // Always rebuilds controls even if effectName matches the current effect.
    void    showEffect(const QString& effectName, const QString& rawSettings);

    QString currentEffect() const { return _currentEffect; }
    const QVariantMap& currentSettings() const { return _settings; }

    QList<QColor> palette() const;

    // Load the palette from a raw xsq palette string
    // ("C_BUTTON_Palette1=#RRGGBB,C_CHECKBOX_Palette1=1,...").
    // Silently ignored if the string is empty or unparseable.
    void loadBlockPalette(const QString& rawPalette);

    // Push rendered buffer pixels (from QtRenderBridge) into the buffer view.
    void setBufferPixels(int w, int h, const QList<QColor>& pixels);

signals:
    void settingsChanged(const QString& effectName, const QVariantMap& settings);
    void paletteChanged(const QString& effectName, const QList<QColor>& palette);

private slots:
    void onDebounceTimeout();

private:
    void clearControls();
    void buildControls(const QString& effectName);
    void notifyChange();

    PaletteWidget*  _palette;
    BufferWidget*   _buffer;
    SubBufferWidget* _subBuffer = nullptr;
    QLabel*        _titleLabel;
    QScrollArea*   _scroll;
    QWidget*       _content    = nullptr;
    QVBoxLayout*   _contentLay = nullptr;
    QTimer*        _debounce;
    QString        _currentEffect;
    QVariantMap    _settings;
};
