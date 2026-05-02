#include "EffectPanelWidget.h"
#include "BufferWidget.h"
#include "EffectControlBuilder.h"
#include "PaletteWidget.h"
#include "SubBufferWidget.h"
#include "../App/QtXLightsApp.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <QFile>
#include <QLabel>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTimer>
#include <QVBoxLayout>

EffectPanelWidget::EffectPanelWidget(QWidget* parent) : QWidget(parent) {
    auto* vlay = new QVBoxLayout(this);
    vlay->setContentsMargins(4, 4, 4, 4);
    vlay->setSpacing(4);

    _titleLabel = new QLabel("No effect selected", this);
    _titleLabel->setStyleSheet("font-weight: bold; font-size: 13px; padding: 4px;");
    vlay->addWidget(_titleLabel);

    _palette = new PaletteWidget(this);
    vlay->addWidget(_palette);

    // Vertical splitter: buffer view on top, effect controls below.
    auto* splitter = new QSplitter(Qt::Vertical, this);
    splitter->setChildrenCollapsible(true);

    _buffer = new BufferWidget(splitter);
    splitter->addWidget(_buffer);

    _scroll = new QScrollArea(splitter);
    _scroll->setWidgetResizable(true);
    _scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    splitter->addWidget(_scroll);

    splitter->setSizes({160, 400});
    vlay->addWidget(splitter, 1);

    // Sub-buffer / buffer settings panel below the effect controls scroll area.
    _subBuffer = new SubBufferWidget(this);
    vlay->addWidget(_subBuffer);

    _debounce = new QTimer(this);
    _debounce->setSingleShot(true);
    _debounce->setInterval(40);
    connect(_debounce, &QTimer::timeout, this, &EffectPanelWidget::onDebounceTimeout);

    connect(_palette, &PaletteWidget::paletteChanged, this, [this](const QList<QColor>&) {
        notifyChange();
    });
    connect(_buffer, &BufferWidget::blendModeChanged,  this, [this](const QString&) { notifyChange(); });
    connect(_buffer, &BufferWidget::bufferStyleChanged, this, [this](const QString&) { notifyChange(); });
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

    // The xsq palette string is "key=value,key=value,..." where commas inside
    // values are escaped as "&comma;" by xLights' SettingsMap::AsString().
    // We parse it manually because we only need the simple C_BUTTON/CHECKBOX keys
    // and hex colors don't contain commas, so a two-pass unescape approach is safe.
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

    // Update the palette widget without triggering an extra settingsChanged /
    // paletteChanged render (the caller will trigger its own render with rawPalette).
    QSignalBlocker blocker(_palette);
    _palette->setColors(colors);
}

void EffectPanelWidget::setBufferPixels(int w, int h, const QList<QColor>& pixels) {
    _buffer->setPixels(w, h, pixels);
}

// ── Settings parser ───────────────────────────────────────────────────────────
// Converts an xsq settings string like
//   "E_TEXTCTRL_Text_Line1=Hello,E_CHOICE_Text_Dir=left,..."
// into a QVariantMap keyed by bare control ID:
//   { "Text_Line1": "Hello", "Text_Dir": "left", ... }
// The mapping strips the 2-char type prefix (E_, T_, B_, C_) and then the
// control-class prefix (TEXTCTRL_, CHOICE_, SLIDER_, CHECKBOX_, etc.) so the
// resulting keys match what EffectControlBuilder stores in _settings.
static QVariantMap parseRawSettings(const QString& raw) {
    // Control-class prefixes stripped only for E_* (effect) keys → bare ID.
    // B_* (buffer), T_* (transition) keep their CHOICE_/SLIDER_/etc. prefix.
    static const QStringList kCtrlPfx = {
        "TEXTCTRL_", "SLIDER_", "CHECKBOX_", "CHOICE_", "SPINCTRL_",
        "FILEPICKERCTRL_", "FONTPICKER_", "CUSTOM_", "VALUECURVE_",
        "0FILEPICKERCTRL_", "TOGGLEBUTTON_", "NOTEBOOK_", "PANEL_"
    };
    QVariantMap result;
    for (const QString& part : raw.split(',', Qt::SkipEmptyParts)) {
        const int eq = part.indexOf('=');
        if (eq <= 0) continue;
        QString key = part.left(eq).trimmed();
        QString val = part.mid(eq + 1).trimmed();
        val.replace("&comma;", ",").replace("&amp;", "&");

        if (key.size() < 3 || key[1] != '_') {
            result[key] = val;
            continue;
        }
        const QChar pfxChar = key[0];
        key = key.mid(2);   // strip "X_"

        if (pfxChar == 'E') {
            // Effect settings: also strip control-class prefix → bare ID
            for (const QString& cpfx : kCtrlPfx)
                if (key.startsWith(cpfx)) { key = key.mid(cpfx.size()); break; }
        }
        // B_/T_: keep CHOICE_/SLIDER_/CHECKBOX_/CUSTOM_ prefix so SubBufferWidget
        //        can find them without colliding with bare effect IDs.
        // C_: skip palette entries (handled by loadBlockPalette separately).
        if (pfxChar != 'C')
            result[key] = val;
    }
    return result;
}

void EffectPanelWidget::showEffect(const QString& effectName,
                                    const QString& rawSettings) {
    // Force-rebuild controls even if the same effect name is already shown,
    // because a different block may have different settings values.
    _currentEffect = effectName;
    _settings = parseRawSettings(rawSettings);   // pre-populate with block values
    clearControls();
    _buffer->clear();
    _titleLabel->setText(effectName);
    buildControls(effectName);
    // Populate sub-buffer panel from B_* settings (kept with CHOICE_/CHECKBOX_/CUSTOM_ prefix).
    _subBuffer->loadSettings(_settings);
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
    // _settings may already contain pre-populated values from parseRawSettings();
    // EffectControlBuilder respects existing values and only fills in defaults for
    // keys that are absent.
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

void EffectPanelWidget::notifyChange() {
    _debounce->start();
}

void EffectPanelWidget::onDebounceTimeout() {
    emit settingsChanged(_currentEffect, _settings);
    emit paletteChanged(_currentEffect, _palette->colors());
}
