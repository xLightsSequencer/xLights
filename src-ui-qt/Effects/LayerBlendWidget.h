#pragma once
#include <QVariantMap>
#include <QWidget>
#include <nlohmann/json.hpp>

class QComboBox;
class QScrollArea;
class QTabWidget;

// Tab 3 — Blending settings loaded from Blending.json.
// Hand-coded blend-mode combo (LayerMethodRow is custom in JSON).
// ECB-driven SuppressEffectUntil, FreezeEffectAtFrame at top.
// In/Out Transition tabs with ECB-driven Adjust + Reverse controls.
// All settings keys are bare IDs matching Blending.json property ids.
class LayerBlendWidget : public QWidget {
    Q_OBJECT
public:
    explicit LayerBlendWidget(QWidget* parent = nullptr);

    void loadSettings(const QVariantMap& settings);
    void writeSettings(QVariantMap& settings) const;

    QString blendMode() const;
    void    setBlendMode(const QString& mode);

signals:
    void changed();
    void blendModeChanged(const QString& mode);

private:
    void buildTopEcb();
    void buildTransitionEcb();
    nlohmann::json nonTabProps() const;
    nlohmann::json tabProps(const std::string& tabLabel) const;

    nlohmann::json _json;
    QVariantMap    _settings;

    QComboBox*   _blendCombo   = nullptr;
    QScrollArea* _topScroll    = nullptr;
    QTabWidget*  _transTabs    = nullptr;
    QScrollArea* _inScroll     = nullptr;
    QScrollArea* _outScroll    = nullptr;
};
