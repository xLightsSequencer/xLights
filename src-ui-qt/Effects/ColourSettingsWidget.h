#pragma once
#include <QColor>
#include <QVariantMap>
#include <QWidget>
#include <nlohmann/json.hpp>

class QCheckBox;
class QLabel;
class QScrollArea;
class QSlider;
class QPushButton;

// Tab 2 — Colour settings loaded from Color.json.
// Non-custom properties (Brightness, Contrast, HSV) are built by EffectControlBuilder.
// Compound custom controls (Sparkle, Shimmer, Chroma Key) are hand-coded below.
// All settings keys are bare IDs matching Color.json property ids.
class ColourSettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ColourSettingsWidget(QWidget* parent = nullptr);

    void loadSettings(const QVariantMap& settings);
    void writeSettings(QVariantMap& settings) const;

signals:
    void changed();

private:
    void buildEcbSection();
    void connectCustomSignals();
    void updateChromaButton();

    nlohmann::json _json;
    QVariantMap    _settings;
    QScrollArea*   _scroll = nullptr;

    // Hand-coded custom controls
    QSlider*    _sparkle      = nullptr;
    QLabel*     _sparkleVal   = nullptr;
    QCheckBox*  _musicSparkle = nullptr;
    QCheckBox*  _shimmer      = nullptr;
    QCheckBox*  _chromaEnable = nullptr;
    QPushButton* _chromaColour = nullptr;
    QSlider*    _chromaThresh = nullptr;
    QLabel*     _chromaThreshVal = nullptr;
    QColor      _chromaColor  = Qt::green;
};
