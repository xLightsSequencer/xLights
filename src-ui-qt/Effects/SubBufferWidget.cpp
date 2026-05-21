#include "SubBufferWidget.h"
#include "EffectControlBuilder.h"
#include "SubBufferCanvas.h"
#include "../App/QtXLightsApp.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <QCheckBox>
#include <QFile>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QVBoxLayout>

// ── JSON helpers ──────────────────────────────────────────────────────────────

// Returns the properties array for the named tab (skips custom controlType).
nlohmann::json SubBufferWidget::tabProps(const std::string& tabLabel) const {
    std::map<std::string, nlohmann::json> byId;
    if (_json.contains("properties") && _json["properties"].is_array())
        for (const auto& p : _json["properties"])
            if (p.contains("id")) byId[p["id"].get<std::string>()] = p;

    nlohmann::json result = nlohmann::json::array();
    if (!_json.contains("groups")) return result;
    for (const auto& group : _json["groups"]) {
        if (group.value("type", "") != "tabs") continue;
        for (const auto& tab : group.value("tabs", nlohmann::json::array())) {
            if (tab.value("label", "") != tabLabel) continue;
            for (const auto& pid : tab.value("properties", nlohmann::json::array())) {
                auto it = byId.find(pid.get<std::string>());
                if (it == byId.end()) continue;
                if (it->second.value("controlType", "") != "custom")
                    result.push_back(it->second);
            }
        }
    }
    return result;
}

// ── Sub-buffer codec ──────────────────────────────────────────────────────────

QString SubBufferWidget::encodeSubBuffer(int left, int bottom, int right, int top) {
    if (left == 0 && bottom == 0 && right == 100 && top == 100) return {};
    return QString("%1x%2x%3x%4x0x0").arg(left).arg(bottom).arg(right).arg(top);
}

void SubBufferWidget::decodeSubBuffer(const QString& sb,
                                      int& left, int& bottom, int& right, int& top) {
    left = 0; bottom = 0; right = 100; top = 100;
    if (sb.isEmpty()) return;
    QString s = sb;
    s.replace("Max", "___");
    const QStringList parts = s.split('x');
    auto toInt = [&](int idx, int def) {
        if (idx >= parts.size()) return def;
        bool ok; int v = parts[idx].toInt(&ok);
        return ok ? v : def;
    };
    left   = toInt(0, 0);
    bottom = toInt(1, 0);
    right  = toInt(2, 100);
    top    = toInt(3, 100);
}

// ── ECB rebuild helpers ───────────────────────────────────────────────────────

void SubBufferWidget::buildBufferEcb() {
    if (auto* old = _bufScroll->takeWidget()) old->deleteLater();
    auto* w = new QWidget;
    auto* lay = new QVBoxLayout(w);
    lay->setContentsMargins(4, 4, 4, 4);
    lay->setSpacing(4);
    EffectControlBuilder::build(w, lay, tabProps("Buffer"),
                                _settings, [this]() { emit changed(); });
    _bufScroll->setWidget(w);
}

void SubBufferWidget::buildRotoZoomEcb() {
    if (auto* old = _rzScroll->takeWidget()) old->deleteLater();
    auto* w = new QWidget;
    auto* lay = new QVBoxLayout(w);
    lay->setContentsMargins(4, 4, 4, 4);
    lay->setSpacing(4);
    EffectControlBuilder::build(w, lay, tabProps("Roto-Zoom"),
                                _settings, [this]() { emit changed(); });
    _rzScroll->setWidget(w);
}

// ── Constructor ───────────────────────────────────────────────────────────────

SubBufferWidget::SubBufferWidget(QWidget* parent) : QWidget(parent) {
    // Load Buffer.json
    const QString jsonPath = QtXLightsApp::instance().effectMetadataDir()
                             + "/shared/Buffer.json";
    QFile f(jsonPath);
    if (f.open(QIODevice::ReadOnly)) {
        try { _json = nlohmann::json::parse(f.readAll().toStdString()); }
        catch (...) { spdlog::warn("SubBufferWidget: failed to parse Buffer.json"); }
    } else {
        spdlog::warn("SubBufferWidget: cannot open {}", jsonPath.toStdString());
    }

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    _tabs = new QTabWidget(this);
    outer->addWidget(_tabs, 1);

    // ── Buffer tab ────────────────────────────────────────────────────────────
    auto* bufTab = new QWidget;
    auto* bufLay = new QVBoxLayout(bufTab);
    bufLay->setContentsMargins(4, 4, 4, 4);
    bufLay->setSpacing(4);

    _bufScroll = new QScrollArea(bufTab);
    _bufScroll->setWidgetResizable(true);
    _bufScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _bufScroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    bufLay->addWidget(_bufScroll);

    _oversized = new QCheckBox("Oversized (-100 to 200)", bufTab);
    bufLay->addWidget(_oversized);

    _canvas = new SubBufferCanvas(bufTab);
    bufLay->addWidget(_canvas, 1);

    _tabs->addTab(bufTab, "Buffer");

    // ── Roto-Zoom tab ─────────────────────────────────────────────────────────
    _rzScroll = new QScrollArea;
    _rzScroll->setWidgetResizable(true);
    _rzScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _tabs->addTab(_rzScroll, "Roto-Zoom");

    // Build initial ECB content
    buildBufferEcb();
    buildRotoZoomEcb();

    // ── Canvas / oversized signals ────────────────────────────────────────────
    connect(_canvas, &SubBufferCanvas::regionChanged,
            this, [this](int l, int b, int r, int t) {
        _settings["SubBuffer"] = encodeSubBuffer(l, b, r, t);
        emit changed();
    });

    connect(_oversized, &QCheckBox::toggled, this, [this](bool on) {
        _canvas->setOversized(on);
        emit changed();
    });
}

// ── Public API ────────────────────────────────────────────────────────────────

void SubBufferWidget::loadSettings(const QVariantMap& settings) {
    // Only extract keys managed by this widget (Buffer.json property IDs + SubBuffer).
    _settings.clear();
    if (_json.contains("properties") && _json["properties"].is_array()) {
        for (const auto& p : _json["properties"]) {
            const QString id = QString::fromStdString(p.value("id", ""));
            if (!id.isEmpty() && settings.contains(id))
                _settings[id] = settings[id];
        }
    }
    // Always seed SubBuffer even if not in properties (it's custom)
    _settings["SubBuffer"] = settings.value("SubBuffer", "");

    buildBufferEcb();
    buildRotoZoomEcb();

    // Decode SubBuffer for the canvas
    const QString sb = _settings.value("SubBuffer", "").toString();
    int l, bo, r, t;
    decodeSubBuffer(sb, l, bo, r, t);
    const bool needOversized = (l < 0 || bo < 0 || r > 100 || t > 100);

    {
        QSignalBlocker bOver(_oversized);
        _oversized->setChecked(needOversized);
    }
    _canvas->setOversized(needOversized);
    {
        QSignalBlocker bCanvas(_canvas);
        _canvas->setRegion(l, bo, r, t);
    }
}

void SubBufferWidget::writeSettings(QVariantMap& settings) const {
    for (auto it = _settings.cbegin(); it != _settings.cend(); ++it)
        settings[it.key()] = it.value();
    // Keep SubBuffer in sync with the canvas current state
    // (canvas may have been dragged without a regionChanged if setRegion was blocked)
}
