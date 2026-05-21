#include "ColourSettingsWidget.h"
#include "EffectControlBuilder.h"
#include "../App/QtXLightsApp.h"

#include <spdlog/spdlog.h>

#include <QCheckBox>
#include <QColorDialog>
#include <QFile>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>

// ── ECB section (re)build ─────────────────────────────────────────────────────

void ColourSettingsWidget::buildEcbSection() {
    if (auto* old = _scroll->takeWidget()) old->deleteLater();

    // Collect non-custom properties from Color.json
    nlohmann::json props = nlohmann::json::array();
    if (_json.contains("properties") && _json["properties"].is_array()) {
        for (const auto& p : _json["properties"]) {
            if (p.value("controlType", "") != "custom")
                props.push_back(p);
        }
    }

    auto* w = new QWidget;
    auto* lay = new QVBoxLayout(w);
    lay->setContentsMargins(4, 4, 4, 4);
    lay->setSpacing(4);
    EffectControlBuilder::build(w, lay, props,
                                _settings, [this]() { emit changed(); });
    _scroll->setWidget(w);
}

// ── Construction ──────────────────────────────────────────────────────────────

ColourSettingsWidget::ColourSettingsWidget(QWidget* parent) : QWidget(parent) {
    // Load Color.json
    const QString jsonPath = QtXLightsApp::instance().effectMetadataDir()
                             + "/shared/Color.json";
    QFile f(jsonPath);
    if (f.open(QIODevice::ReadOnly)) {
        try { _json = nlohmann::json::parse(f.readAll().toStdString()); }
        catch (...) { spdlog::warn("ColourSettingsWidget: failed to parse Color.json"); }
    } else {
        spdlog::warn("ColourSettingsWidget: cannot open {}", jsonPath.toStdString());
    }

    auto* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(4, 4, 4, 4);
    vbox->setSpacing(6);

    // ── ECB scroll (Brightness, Contrast, HSV sliders) ────────────────────────
    _scroll = new QScrollArea(this);
    _scroll->setWidgetResizable(true);
    _scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    vbox->addWidget(_scroll);

    buildEcbSection();

    // ── Sparkle ───────────────────────────────────────────────────────────────
    {
        auto* grp  = new QGroupBox("Sparkle", this);
        auto* form = new QFormLayout(grp);
        form->setSpacing(4);

        _sparkle    = new QSlider(Qt::Horizontal, grp);
        _sparkle->setRange(0, 200);
        _sparkle->setValue(0);
        _sparkleVal = new QLabel("0", grp);
        _sparkleVal->setFixedWidth(32);

        auto* srow = new QWidget(grp);
        auto* sh   = new QHBoxLayout(srow);
        sh->setContentsMargins(0,0,0,0);
        sh->addWidget(_sparkle, 1);
        sh->addWidget(_sparkleVal);
        form->addRow("Frequency:", srow);

        _musicSparkle = new QCheckBox("Sync to music", grp);
        form->addRow(_musicSparkle);

        vbox->addWidget(grp);
    }

    // ── Shimmer ───────────────────────────────────────────────────────────────
    {
        auto* grp  = new QGroupBox("Shimmer", this);
        auto* form = new QFormLayout(grp);
        form->setSpacing(4);
        _shimmer = new QCheckBox("Enable shimmer", grp);
        form->addRow(_shimmer);
        vbox->addWidget(grp);
    }

    // ── Chroma Key ────────────────────────────────────────────────────────────
    {
        auto* grp  = new QGroupBox("Chroma Key", this);
        auto* form = new QFormLayout(grp);
        form->setSpacing(4);

        _chromaEnable = new QCheckBox("Enable", grp);
        form->addRow(_chromaEnable);

        _chromaColour = new QPushButton(grp);
        _chromaColour->setFixedHeight(24);
        _chromaColour->setEnabled(false);
        updateChromaButton();
        form->addRow("Key colour:", _chromaColour);

        _chromaThresh = new QSlider(Qt::Horizontal, grp);
        _chromaThresh->setRange(0, 100);
        _chromaThresh->setValue(10);
        _chromaThresh->setEnabled(false);
        _chromaThreshVal = new QLabel("10", grp);
        _chromaThreshVal->setFixedWidth(32);

        auto* trow = new QWidget(grp);
        auto* th   = new QHBoxLayout(trow);
        th->setContentsMargins(0,0,0,0);
        th->addWidget(_chromaThresh, 1);
        th->addWidget(_chromaThreshVal);
        form->addRow("Threshold:", trow);

        vbox->addWidget(grp);
    }

    connectCustomSignals();
}

// ── Signal wiring ─────────────────────────────────────────────────────────────

void ColourSettingsWidget::connectCustomSignals() {
    connect(_sparkle, &QSlider::valueChanged, this, [this](int v) {
        _sparkleVal->setText(QString::number(v));
        _settings["SparkleFrequency"] = v;
        emit changed();
    });
    connect(_musicSparkle, &QCheckBox::toggled, this, [this](bool on) {
        _settings["MusicSparkle"] = on ? "1" : "0";
        emit changed();
    });
    connect(_shimmer, &QCheckBox::toggled, this, [this](bool on) {
        _settings["Shimmer"] = on ? "1" : "0";
        emit changed();
    });
    connect(_chromaEnable, &QCheckBox::toggled, this, [this](bool on) {
        _chromaColour->setEnabled(on);
        _chromaThresh->setEnabled(on);
        _settings["ChromaKey"] = on ? "1" : "0";
        emit changed();
    });
    connect(_chromaColour, &QPushButton::clicked, this, [this]() {
        QColor c = QColorDialog::getColor(_chromaColor, this, "Chroma Key Colour");
        if (c.isValid()) {
            _chromaColor = c;
            updateChromaButton();
            _settings["ChromaColour"] = _chromaColor.name().toUpper();
            emit changed();
        }
    });
    connect(_chromaThresh, &QSlider::valueChanged, this, [this](int v) {
        _chromaThreshVal->setText(QString::number(v));
        _settings["ChromaKeyThreshold"] = v;
        emit changed();
    });
}

void ColourSettingsWidget::updateChromaButton() {
    QPixmap pm(60, 20);
    pm.fill(_chromaColor);
    _chromaColour->setIcon(QIcon(pm));
    _chromaColour->setText(_chromaColor.name().toUpper());
}

// ── Settings I/O ──────────────────────────────────────────────────────────────
// All keys are bare IDs matching Color.json ids (prefix stripped by parseRawSettings).

void ColourSettingsWidget::loadSettings(const QVariantMap& s) {
    // Only extract keys managed by this widget (Color.json IDs + custom compound keys).
    _settings.clear();
    if (_json.contains("properties") && _json["properties"].is_array()) {
        for (const auto& p : _json["properties"]) {
            if (p.value("controlType", "") == "custom") continue;
            const QString id = QString::fromStdString(p.value("id", ""));
            if (!id.isEmpty() && s.contains(id)) _settings[id] = s[id];
        }
    }
    // Hand-coded custom keys
    for (const QString& k : {"SparkleFrequency", "MusicSparkle", "Shimmer",
                              "ChromaKey", "ChromaColour", "ChromaKeyThreshold"})
        if (s.contains(k)) _settings[k] = s[k];

    buildEcbSection();

    QSignalBlocker b1(_sparkle), b2(_musicSparkle), b3(_shimmer),
                   b4(_chromaEnable), b5(_chromaThresh);

    _sparkle->setValue(_settings.value("SparkleFrequency", 0).toInt());
    _sparkleVal->setText(QString::number(_sparkle->value()));

    _musicSparkle->setChecked(_settings.value("MusicSparkle", "0").toString() == "1");
    _shimmer->setChecked(_settings.value("Shimmer", "0").toString() == "1");

    const bool chromaOn = _settings.value("ChromaKey", "0").toString() == "1";
    _chromaEnable->setChecked(chromaOn);
    _chromaColour->setEnabled(chromaOn);
    _chromaThresh->setEnabled(chromaOn);

    const QString colStr = _settings.value("ChromaColour", "#00FF00").toString();
    _chromaColor = QColor(colStr.isEmpty() ? "#00FF00" : colStr);
    if (!_chromaColor.isValid()) _chromaColor = Qt::green;
    updateChromaButton();

    _chromaThresh->setValue(_settings.value("ChromaKeyThreshold", 10).toInt());
    _chromaThreshVal->setText(QString::number(_chromaThresh->value()));
}

void ColourSettingsWidget::writeSettings(QVariantMap& s) const {
    for (auto it = _settings.cbegin(); it != _settings.cend(); ++it)
        s[it.key()] = it.value();
}
