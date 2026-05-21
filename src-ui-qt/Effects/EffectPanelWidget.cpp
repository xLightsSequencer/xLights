#include "EffectPanelWidget.h"
#include "BufferWidget.h"
#include "ColourSettingsWidget.h"
#include "EffectControlBuilder.h"
#include "LayerBlendWidget.h"
#include "PaletteWidget.h"
#include "SubBufferWidget.h"
#include "../App/QtXLightsApp.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <QFile>
#include <QFrame>
#include <QLabel>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

EffectPanelWidget::EffectPanelWidget(QWidget* parent) : QWidget(parent) {
    auto* vlay = new QVBoxLayout(this);
    vlay->setContentsMargins(4, 4, 4, 4);
    vlay->setSpacing(4);

    // Effect name label above the tab bar.
    _titleLabel = new QLabel("No effect selected", this);
    _titleLabel->setStyleSheet(
        "font-weight: bold; font-size: 13px; padding: 4px 4px 2px 4px;");
    vlay->addWidget(_titleLabel);

    _tabs = new QTabWidget(this);
    _tabs->setTabPosition(QTabWidget::North);
    _tabs->setDocumentMode(true);
    vlay->addWidget(_tabs, 1);

    // ── Tab 1 — Effect ───────────────────────────────────────────────────────
    auto* tab1 = new QWidget;
    auto* t1v  = new QVBoxLayout(tab1);
    t1v->setContentsMargins(4, 4, 4, 4);
    t1v->setSpacing(4);

    _palette = new PaletteWidget(tab1);
    t1v->addWidget(_palette);

    _scroll = new QScrollArea(tab1);
    _scroll->setWidgetResizable(true);
    _scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    t1v->addWidget(_scroll, 1);

    _tabs->addTab(tab1, "Effect");

    // ── Tab 2 — Colour ────────────────────────────────────────────────────────
    _colour = new ColourSettingsWidget;
    _tabs->addTab(_colour, "Colour");

    // ── Tab 3 — Layer ────────────────────────────────────────────────────────
    _layerBlend = new LayerBlendWidget;
    _tabs->addTab(_layerBlend, "Layer");

    // ── Tab 4 — Buffer ───────────────────────────────────────────────────────
    auto* tab4    = new QWidget;
    auto* t4v     = new QVBoxLayout(tab4);
    t4v->setContentsMargins(4, 4, 4, 4);
    t4v->setSpacing(4);

    _subBuffer = new SubBufferWidget(tab4);
    t4v->addWidget(_subBuffer);

    auto* sep = new QFrame(tab4);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    t4v->addWidget(sep);

    _buffer = new BufferWidget(tab4);
    t4v->addWidget(_buffer, 1);

    _tabs->addTab(tab4, "Buffer");

    // ── Debounce timer ────────────────────────────────────────────────────────
    _debounce = new QTimer(this);
    _debounce->setSingleShot(true);
    _debounce->setInterval(40);
    connect(_debounce, &QTimer::timeout, this, &EffectPanelWidget::onDebounceTimeout);

    // ── Signal wiring ─────────────────────────────────────────────────────────
    connect(_palette, &PaletteWidget::paletteChanged,
            this, [this](const QList<QColor>&) { notifyChange(); });

    connect(_colour, &ColourSettingsWidget::changed, this, [this]() {
        _colour->writeSettings(_settings);
        notifyChange();
    });

    connect(_layerBlend, &LayerBlendWidget::changed, this, [this]() {
        _layerBlend->writeSettings(_settings);
        notifyChange();
    });

    connect(_subBuffer, &SubBufferWidget::changed, this, [this]() {
        _subBuffer->writeSettings(_settings);
        notifyChange();
    });
}

QList<QColor> EffectPanelWidget::palette() const {
    return _palette->colors();
}

void EffectPanelWidget::loadBlockPalette(const QString& rawPalette) {
    if (rawPalette.isEmpty()) return;

    QMap<QString, QString> vals;
    for (const QString& part : rawPalette.split(',', Qt::SkipEmptyParts)) {
        const int eq = part.indexOf('=');
        if (eq <= 0) continue;
        QString key = part.left(eq).trimmed();
        QString val = part.mid(eq + 1).trimmed();
        val.replace("&comma;", ",").replace("&amp;", "&");
        vals[key] = val;
    }

    QList<QColor> colors;
    for (int i = 1; i <= PaletteWidget::kMaxColors; ++i) {
        if (vals.value(QString("C_CHECKBOX_Palette%1").arg(i)) == "1") {
            QColor c(vals.value(QString("C_BUTTON_Palette%1").arg(i)));
            if (c.isValid()) colors.append(c);
        }
    }
    if (colors.isEmpty()) return;

    QSignalBlocker blocker(_palette);
    _palette->setColors(colors);
}

void EffectPanelWidget::setBufferPixels(int w, int h, const QList<QColor>& pixels) {
    _buffer->setPixels(w, h, pixels);
}

// ── Settings parser ───────────────────────────────────────────────────────────

static QVariantMap parseRawSettings(const QString& raw) {
    static const QStringList kCtrlPfx = {
        "TEXTCTRL_", "SLIDER_", "CHECKBOX_", "CHOICE_", "SPINCTRL_",
        "FILEPICKERCTRL_", "FONTPICKER_", "CUSTOM_", "VALUECURVE_",
        "BUTTON_", "0FILEPICKERCTRL_", "TOGGLEBUTTON_", "NOTEBOOK_", "PANEL_"
    };
    QVariantMap result;
    for (const QString& part : raw.split(',', Qt::SkipEmptyParts)) {
        const int eq = part.indexOf('=');
        if (eq <= 0) continue;
        QString key = part.left(eq).trimmed();
        QString val = part.mid(eq + 1).trimmed();
        val.replace("&comma;", ",").replace("&amp;", "&");

        if (key.size() < 3 || key[1] != '_') { result[key] = val; continue; }
        const QChar pfxChar = key[0];
        key = key.mid(2);

        // E_, B_, C_, T_ — all strip the inner control-class prefix so bare IDs
        // flow into _settings and match the JSON property id fields directly.
        for (const QString& cpfx : kCtrlPfx)
            if (key.startsWith(cpfx)) { key = key.mid(cpfx.size()); break; }

        // C_: skip palette entries (handled by loadBlockPalette).
        if (pfxChar == 'C') {
            static const QRegularExpression kPaletteKey(
                "^Palette\\d+$");
            if (!kPaletteKey.match(key).hasMatch())
                result[key] = val;
        } else {
            result[key] = val;
        }
    }
    return result;
}

// ── Effect display ────────────────────────────────────────────────────────────

void EffectPanelWidget::showEffect(const QString& effectName,
                                    const QString& rawSettings) {
    _currentEffect = effectName;
    _settings      = parseRawSettings(rawSettings);
    clearControls();
    _buffer->clear();
    _titleLabel->setText(effectName);
    buildControls(effectName);
    _subBuffer->loadSettings(_settings);
    _colour->loadSettings(_settings);
    _layerBlend->loadSettings(_settings);
    // Switch to Tab 1 automatically so the user sees effect controls.
    _tabs->setCurrentIndex(0);
    notifyChange();
}

void EffectPanelWidget::showEffect(const QString& effectName) {
    if (effectName == _currentEffect) return;
    _currentEffect = effectName;
    _settings.clear();
    clearControls();
    _buffer->clear();
    _titleLabel->setText(effectName);
    buildControls(effectName);
    _subBuffer->loadSettings(_settings);
    _colour->loadSettings(_settings);
    _layerBlend->loadSettings(_settings);
    _tabs->setCurrentIndex(0);
    notifyChange();
}

void EffectPanelWidget::buildControls(const QString& effectName) {
    const QtXLightsApp& app = QtXLightsApp::instance();
    QString jsonPath;
    for (const auto& e : app.effects())
        if (e.name == effectName) { jsonPath = e.jsonPath; break; }

    if (jsonPath.isEmpty()) {
        spdlog::warn("EffectPanelWidget: no metadata for '{}'", effectName.toStdString());
        return;
    }

    QFile f(jsonPath);
    if (!f.open(QIODevice::ReadOnly)) {
        spdlog::error("EffectPanelWidget: cannot open '{}'", jsonPath.toStdString());
        return;
    }

    nlohmann::json doc;
    try { doc = nlohmann::json::parse(f.readAll().toStdString()); }
    catch (const nlohmann::json::exception& ex) {
        spdlog::error("EffectPanelWidget: JSON parse error: {}", ex.what());
        return;
    }

    _content    = new QWidget;
    _contentLay = new QVBoxLayout(_content);
    _contentLay->setContentsMargins(4, 4, 4, 4);
    _contentLay->setSpacing(6);

    auto onChange = [this]() { _debounce->start(); };
    EffectControlBuilder::build(_content, _contentLay,
                                doc.value("properties", nlohmann::json::array()),
                                _settings, std::move(onChange));
    _scroll->setWidget(_content);
}

void EffectPanelWidget::clearControls() {
    if (QWidget* old = _scroll->takeWidget()) old->deleteLater();
    _content    = nullptr;
    _contentLay = nullptr;
}

void EffectPanelWidget::notifyChange() { _debounce->start(); }

void EffectPanelWidget::onDebounceTimeout() {
    emit settingsChanged(_currentEffect, _settings);
    emit paletteChanged(_currentEffect, _palette->colors());
}
