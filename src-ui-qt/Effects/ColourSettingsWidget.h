#pragma once
#include <QColor>
#include <QVariantMap>
#include <QWidget>

class QCheckBox;
class QLabel;
class QSlider;
class QPushButton;

// Tab 2 — Colour settings.
// Mirrors xLights' colour panel: brightness, contrast, sparkle,
// shimmer, and chroma-key.  Settings are stored as C_* keys.
class ColourSettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ColourSettingsWidget(QWidget* parent = nullptr);

    void loadSettings(const QVariantMap& settings);
    void writeSettings(QVariantMap& settings) const;

signals:
    void changed();

private:
    void connectSignals();
    void updateChromaButton();

    // HSV adjustment
    QSlider* _hue    = nullptr;
    QLabel*  _hueVal = nullptr;
    QSlider* _sat    = nullptr;
    QLabel*  _satVal = nullptr;
    QSlider* _val    = nullptr;
    QLabel*  _valVal = nullptr;

    // Brightness / contrast
    QSlider* _brightness    = nullptr;
    QLabel*  _brightnessVal = nullptr;
    QSlider* _contrast      = nullptr;
    QLabel*  _contrastVal   = nullptr;

    // Sparkle
    QSlider*   _sparkle      = nullptr;
    QLabel*    _sparkleVal   = nullptr;
    QCheckBox* _musicSparkle = nullptr;

    // Shimmer
    QCheckBox* _shimmer = nullptr;

    // Chroma key
    QCheckBox*  _chromaEnable = nullptr;
    QPushButton* _chromaColour = nullptr;
    QSlider*    _chromaThresh = nullptr;
    QLabel*     _chromaThreshVal = nullptr;
    QColor      _chromaColor  = Qt::green;
};
