#pragma once

#include <QColor>
#include <QList>
#include <QVariantMap>
#include <QWidget>

class BufferWidget;
class ColourSettingsWidget;
class LayerBlendWidget;
class PaletteWidget;
class QLabel;
class QScrollArea;
class QTabWidget;
class QTimer;
class QVBoxLayout;
class SubBufferWidget;

class EffectPanelWidget : public QWidget {
    Q_OBJECT

public:
    explicit EffectPanelWidget(QWidget* parent = nullptr);

    // Show an effect with default settings (e.g. effect toolbar click).
    void showEffect(const QString& effectName);
    // Show an effect pre-populated from a raw xsq settings string.
    // Always rebuilds controls even if effectName matches the current effect.
    void showEffect(const QString& effectName, const QString& rawSettings);

    QString currentEffect() const { return _currentEffect; }
    const QVariantMap& currentSettings() const { return _settings; }

    QList<QColor> palette() const;

    // Load the palette from a raw xsq palette string.
    void loadBlockPalette(const QString& rawPalette);

    // Push rendered buffer pixels into the Tab 4 canvas.
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

    // ── Tab 1 — Effect ───────────────────────────────────────────────────────
    PaletteWidget*  _palette    = nullptr;
    QScrollArea*    _scroll     = nullptr;
    QWidget*        _content    = nullptr;
    QVBoxLayout*    _contentLay = nullptr;

    // ── Tab 2 — Colour ───────────────────────────────────────────────────────
    ColourSettingsWidget* _colour = nullptr;

    // ── Tab 3 — Layer ────────────────────────────────────────────────────────
    LayerBlendWidget* _layerBlend = nullptr;

    // ── Tab 4 — Buffer ───────────────────────────────────────────────────────
    SubBufferWidget* _subBuffer = nullptr;
    BufferWidget*    _buffer    = nullptr;

    // ── Shared ───────────────────────────────────────────────────────────────
    QTabWidget* _tabs          = nullptr;
    QLabel*     _titleLabel    = nullptr;
    QTimer*     _debounce      = nullptr;
    QString     _currentEffect;
    QVariantMap _settings;
};
