#include "LayerBlendWidget.h"
#include "EffectControlBuilder.h"
#include "../App/QtXLightsApp.h"

#include <set>
#include <spdlog/spdlog.h>

#include <QComboBox>
#include <QFile>
#include <QFormLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QVBoxLayout>

// ── JSON helpers ──────────────────────────────────────────────────────────────

// Top-level non-custom, non-tab properties (SuppressEffectUntil, FreezeEffectAtFrame).
nlohmann::json LayerBlendWidget::nonTabProps() const {
    // Collect IDs that are in any tab
    std::set<std::string> inTab;
    if (_json.contains("groups")) {
        for (const auto& g : _json["groups"]) {
            if (g.value("type","") != "tabs") continue;
            for (const auto& tab : g.value("tabs", nlohmann::json::array()))
                for (const auto& pid : tab.value("properties", nlohmann::json::array()))
                    inTab.insert(pid.get<std::string>());
        }
    }

    nlohmann::json result = nlohmann::json::array();
    if (_json.contains("properties") && _json["properties"].is_array()) {
        for (const auto& p : _json["properties"]) {
            if (p.value("controlType","") == "custom") continue;
            const std::string id = p.value("id","");
            if (!id.empty() && !inTab.count(id))
                result.push_back(p);
        }
    }
    return result;
}

nlohmann::json LayerBlendWidget::tabProps(const std::string& tabLabel) const {
    std::map<std::string, nlohmann::json> byId;
    if (_json.contains("properties") && _json["properties"].is_array())
        for (const auto& p : _json["properties"])
            if (p.contains("id")) byId[p["id"].get<std::string>()] = p;

    nlohmann::json result = nlohmann::json::array();
    if (!_json.contains("groups")) return result;
    for (const auto& group : _json["groups"]) {
        if (group.value("type","") != "tabs") continue;
        for (const auto& tab : group.value("tabs", nlohmann::json::array())) {
            if (tab.value("label","") != tabLabel) continue;
            for (const auto& pid : tab.value("properties", nlohmann::json::array())) {
                auto it = byId.find(pid.get<std::string>());
                if (it == byId.end()) continue;
                if (it->second.value("controlType","") != "custom")
                    result.push_back(it->second);
            }
        }
    }
    return result;
}

// ── ECB rebuild helpers ───────────────────────────────────────────────────────

void LayerBlendWidget::buildTopEcb() {
    if (auto* old = _topScroll->takeWidget()) old->deleteLater();
    auto* w = new QWidget;
    auto* lay = new QVBoxLayout(w);
    lay->setContentsMargins(4, 4, 4, 4);
    lay->setSpacing(4);
    EffectControlBuilder::build(w, lay, nonTabProps(),
                                _settings, [this]() { emit changed(); });
    _topScroll->setWidget(w);
}

static void buildEcbInScroll(QScrollArea* scroll, const nlohmann::json& props,
                              QVariantMap& settings, std::function<void()> onChange) {
    if (auto* old = scroll->takeWidget()) old->deleteLater();
    auto* w = new QWidget;
    auto* lay = new QVBoxLayout(w);
    lay->setContentsMargins(4, 4, 4, 4);
    lay->setSpacing(4);
    EffectControlBuilder::build(w, lay, props, settings, std::move(onChange));
    scroll->setWidget(w);
}

void LayerBlendWidget::buildTransitionEcb() {
    buildEcbInScroll(_inScroll,  tabProps("In Transition"),
                     _settings, [this]() { emit changed(); });
    buildEcbInScroll(_outScroll, tabProps("Out Transition"),
                     _settings, [this]() { emit changed(); });
}

// ── Constructor ───────────────────────────────────────────────────────────────

static const QStringList kBlendModes = {
    "Normal", "Effect 1", "Effect 2",
    "1 is Mask", "2 is Mask", "1 reveals 2", "2 reveals 1",
    "Shadow 1 on 2", "Shadow 2 on 1",
    "Layered", "Average", "Combine",
    "Bottom-Top", "Left-Right",
};

LayerBlendWidget::LayerBlendWidget(QWidget* parent) : QWidget(parent) {
    // Load Blending.json
    const QString jsonPath = QtXLightsApp::instance().effectMetadataDir()
                             + "/shared/Blending.json";
    QFile f(jsonPath);
    if (f.open(QIODevice::ReadOnly)) {
        try { _json = nlohmann::json::parse(f.readAll().toStdString()); }
        catch (...) { spdlog::warn("LayerBlendWidget: failed to parse Blending.json"); }
    } else {
        spdlog::warn("LayerBlendWidget: cannot open {}", jsonPath.toStdString());
    }

    auto* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(4, 4, 4, 4);
    vbox->setSpacing(6);

    // ── Blend mode (LayerMethodRow is custom in JSON — hand-coded) ────────────
    {
        auto* grp  = new QGroupBox("Layer Method", this);
        auto* form = new QFormLayout(grp);
        form->setSpacing(4);
        _blendCombo = new QComboBox(grp);
        _blendCombo->addItems(kBlendModes);
        form->addRow("Blend mode:", _blendCombo);
        vbox->addWidget(grp);
    }

    // ── ECB scroll (SuppressEffectUntil, FreezeEffectAtFrame) ────────────────
    _topScroll = new QScrollArea(this);
    _topScroll->setWidgetResizable(true);
    _topScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _topScroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    vbox->addWidget(_topScroll);

    // ── In/Out Transition tabs ────────────────────────────────────────────────
    _transTabs = new QTabWidget(this);
    vbox->addWidget(_transTabs, 1);

    _inScroll = new QScrollArea;
    _inScroll->setWidgetResizable(true);
    _inScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _transTabs->addTab(_inScroll, "In Transition");

    _outScroll = new QScrollArea;
    _outScroll->setWidgetResizable(true);
    _outScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _transTabs->addTab(_outScroll, "Out Transition");

    buildTopEcb();
    buildTransitionEcb();

    // ── Blend mode signal ─────────────────────────────────────────────────────
    connect(_blendCombo, &QComboBox::currentTextChanged, this, [this](const QString& m) {
        _settings["LayerMethod"] = m;
        emit blendModeChanged(m);
        emit changed();
    });
}

// ── Public API ────────────────────────────────────────────────────────────────

void LayerBlendWidget::loadSettings(const QVariantMap& settings) {
    // Only extract keys managed by this widget (Blending.json IDs + LayerMethod).
    _settings.clear();
    if (_json.contains("properties") && _json["properties"].is_array()) {
        for (const auto& p : _json["properties"]) {
            if (p.value("controlType", "") == "custom") continue;
            const QString id = QString::fromStdString(p.value("id", ""));
            if (!id.isEmpty() && settings.contains(id)) _settings[id] = settings[id];
        }
    }
    _settings["LayerMethod"] = settings.value("LayerMethod", "Normal");

    buildTopEcb();
    buildTransitionEcb();

    QSignalBlocker bCombo(_blendCombo);
    const QString mode = _settings.value("LayerMethod", "Normal").toString();
    const int idx = _blendCombo->findText(mode);
    _blendCombo->setCurrentIndex(idx >= 0 ? idx : 0);
}

void LayerBlendWidget::writeSettings(QVariantMap& settings) const {
    for (auto it = _settings.cbegin(); it != _settings.cend(); ++it)
        settings[it.key()] = it.value();
    settings["LayerMethod"] = _blendCombo->currentText();
}

QString LayerBlendWidget::blendMode() const {
    return _blendCombo->currentText();
}

void LayerBlendWidget::setBlendMode(const QString& mode) {
    const int idx = _blendCombo->findText(mode);
    if (idx >= 0) _blendCombo->setCurrentIndex(idx);
}
