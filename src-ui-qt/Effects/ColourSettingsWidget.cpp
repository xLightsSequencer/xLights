#include "ColourSettingsWidget.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>

// ── Helper: labelled slider row ───────────────────────────────────────────────
static QSlider* makeSlider(int lo, int hi, int def, QWidget* parent) {
    auto* s = new QSlider(Qt::Horizontal, parent);
    s->setRange(lo, hi);
    s->setValue(def);
    return s;
}

// ── Construction ──────────────────────────────────────────────────────────────

ColourSettingsWidget::ColourSettingsWidget(QWidget* parent) : QWidget(parent) {
    auto* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(8, 8, 8, 8);
    vbox->setSpacing(10);

    auto makeValLabel = [](int v, int w = 36) {
        auto* l = new QLabel(QString::number(v));
        l->setFixedWidth(w);
        l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        return l;
    };

    auto sliderRow = [&](QFormLayout* form, const QString& label,
                         QSlider* slider, QLabel* valLbl) {
        auto* row = new QWidget;
        auto* h   = new QHBoxLayout(row);
        h->setContentsMargins(0,0,0,0);
        h->setSpacing(4);
        h->addWidget(slider, 1);
        h->addWidget(valLbl);
        form->addRow(label, row);
    };

    // ── HSV Adjustment ────────────────────────────────────────────────────────
    {
        auto* grp  = new QGroupBox("HSV Adjustment", this);
        auto* form = new QFormLayout(grp);
        form->setSpacing(6);

        _hue    = makeSlider(-180, 180, 0, grp);
        _hueVal = makeValLabel(0, 40);
        sliderRow(form, "Hue:", _hue, _hueVal);

        _sat    = makeSlider(-100, 100, 0, grp);
        _satVal = makeValLabel(0, 40);
        sliderRow(form, "Saturation:", _sat, _satVal);

        _val    = makeSlider(-100, 100, 0, grp);
        _valVal = makeValLabel(0, 40);
        sliderRow(form, "Value:", _val, _valVal);

        vbox->addWidget(grp);
    }

    // ── Brightness / Contrast ─────────────────────────────────────────────────
    {
        auto* grp  = new QGroupBox("Brightness / Contrast", this);
        auto* form = new QFormLayout(grp);
        form->setSpacing(6);

        _brightness    = makeSlider(0, 100, 100, grp);
        _brightnessVal = makeValLabel(100);
        sliderRow(form, "Brightness:", _brightness, _brightnessVal);

        _contrast    = makeSlider(0, 100, 0, grp);
        _contrastVal = makeValLabel(0);
        sliderRow(form, "Contrast:", _contrast, _contrastVal);

        vbox->addWidget(grp);
    }

    // ── Sparkle ───────────────────────────────────────────────────────────────
    {
        auto* grp  = new QGroupBox("Sparkle", this);
        auto* form = new QFormLayout(grp);
        form->setSpacing(6);

        _sparkle    = makeSlider(0, 200, 0, grp);
        _sparkleVal = makeValLabel(0);
        sliderRow(form, "Frequency:", _sparkle, _sparkleVal);

        _musicSparkle = new QCheckBox("Sync to music", grp);
        form->addRow("Music sparkle:", _musicSparkle);

        vbox->addWidget(grp);
    }

    // ── Shimmer ───────────────────────────────────────────────────────────────
    {
        auto* grp  = new QGroupBox("Shimmer", this);
        auto* form = new QFormLayout(grp);
        form->setSpacing(6);

        _shimmer = new QCheckBox("Enable shimmer", grp);
        form->addRow(_shimmer);

        vbox->addWidget(grp);
    }

    // ── Chroma Key ────────────────────────────────────────────────────────────
    {
        auto* grp  = new QGroupBox("Chroma Key", this);
        auto* form = new QFormLayout(grp);
        form->setSpacing(6);

        _chromaEnable = new QCheckBox("Enable", grp);
        form->addRow(_chromaEnable);

        _chromaColour = new QPushButton(grp);
        _chromaColour->setFixedHeight(24);
        _chromaColour->setEnabled(false);
        updateChromaButton();
        form->addRow("Key colour:", _chromaColour);

        _chromaThresh    = makeSlider(0, 100, 10, grp);
        _chromaThreshVal = makeValLabel(10);
        _chromaThresh->setEnabled(false);
        sliderRow(form, "Threshold:", _chromaThresh, _chromaThreshVal);

        vbox->addWidget(grp);
    }

    vbox->addStretch(1);

    connectSignals();
}

// ── Signal wiring ─────────────────────────────────────────────────────────────

void ColourSettingsWidget::connectSignals() {
    connect(_hue, &QSlider::valueChanged, this, [this](int v) {
        _hueVal->setText(QString::number(v));
        emit changed();
    });
    connect(_sat, &QSlider::valueChanged, this, [this](int v) {
        _satVal->setText(QString::number(v));
        emit changed();
    });
    connect(_val, &QSlider::valueChanged, this, [this](int v) {
        _valVal->setText(QString::number(v));
        emit changed();
    });
    connect(_brightness, &QSlider::valueChanged, this, [this](int v) {
        _brightnessVal->setText(QString::number(v));
        emit changed();
    });
    connect(_contrast, &QSlider::valueChanged, this, [this](int v) {
        _contrastVal->setText(QString::number(v));
        emit changed();
    });
    connect(_sparkle, &QSlider::valueChanged, this, [this](int v) {
        _sparkleVal->setText(QString::number(v));
        emit changed();
    });
    connect(_musicSparkle, &QCheckBox::toggled, this, [this]() { emit changed(); });
    connect(_shimmer,      &QCheckBox::toggled, this, [this]() { emit changed(); });

    connect(_chromaEnable, &QCheckBox::toggled, this, [this](bool on) {
        _chromaColour->setEnabled(on);
        _chromaThresh->setEnabled(on);
        emit changed();
    });
    connect(_chromaColour, &QPushButton::clicked, this, [this]() {
        QColor c = QColorDialog::getColor(_chromaColor, this, "Chroma Key Colour");
        if (c.isValid()) {
            _chromaColor = c;
            updateChromaButton();
            emit changed();
        }
    });
    connect(_chromaThresh, &QSlider::valueChanged, this, [this](int v) {
        _chromaThreshVal->setText(QString::number(v));
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
// Keys use the C_ control-class form (SLIDER_Brightness etc.) since
// parseRawSettings strips only the "C_" type prefix, leaving the
// SLIDER_/CHECKBOX_/BUTTON_ control-class prefix intact for C_ keys.

void ColourSettingsWidget::loadSettings(const QVariantMap& s) {
    QSignalBlocker b1(_brightness), b2(_contrast), b3(_sparkle),
                   b4(_musicSparkle), b5(_shimmer),
                   b6(_chromaEnable), b7(_chromaThresh),
                   b8(_hue), b9(_sat), b10(_val);

    _hue->setValue(s.value("SLIDER_Color_HueAdjust",        0).toInt());
    _hueVal->setText(QString::number(_hue->value()));
    _sat->setValue(s.value("SLIDER_Color_SaturationAdjust", 0).toInt());
    _satVal->setText(QString::number(_sat->value()));
    _val->setValue(s.value("SLIDER_Color_ValueAdjust",      0).toInt());
    _valVal->setText(QString::number(_val->value()));

    _brightness->setValue(s.value("SLIDER_Brightness",      100).toInt());
    _brightnessVal->setText(QString::number(_brightness->value()));

    _contrast->setValue(s.value("SLIDER_Contrast",          0).toInt());
    _contrastVal->setText(QString::number(_contrast->value()));

    _sparkle->setValue(s.value("SLIDER_SparkleFrequency",   0).toInt());
    _sparkleVal->setText(QString::number(_sparkle->value()));

    _musicSparkle->setChecked(s.value("CHECKBOX_MusicSparkle", "0").toString() == "1");
    _shimmer->setChecked(     s.value("CHECKBOX_Shimmer",      "0").toString() == "1");

    const bool chromaOn = s.value("CHECKBOX_ChromaKey", "0").toString() == "1";
    _chromaEnable->setChecked(chromaOn);
    _chromaColour->setEnabled(chromaOn);
    _chromaThresh->setEnabled(chromaOn);

    const QString colStr = s.value("BUTTON_ChromaColour", "#00FF00").toString();
    _chromaColor = QColor(colStr.isEmpty() ? "#00FF00" : colStr);
    if (!_chromaColor.isValid()) _chromaColor = Qt::green;
    updateChromaButton();

    _chromaThresh->setValue(s.value("SLIDER_ChromaKeyThreshold", 10).toInt());
    _chromaThreshVal->setText(QString::number(_chromaThresh->value()));
}

void ColourSettingsWidget::writeSettings(QVariantMap& s) const {
    s["SLIDER_Color_HueAdjust"]        = _hue->value();
    s["SLIDER_Color_SaturationAdjust"] = _sat->value();
    s["SLIDER_Color_ValueAdjust"]      = _val->value();
    s["SLIDER_Brightness"]             = _brightness->value();
    s["SLIDER_Contrast"]          = _contrast->value();
    s["SLIDER_SparkleFrequency"]  = _sparkle->value();
    s["CHECKBOX_MusicSparkle"]    = _musicSparkle->isChecked() ? "1" : "0";
    s["CHECKBOX_Shimmer"]         = _shimmer->isChecked()      ? "1" : "0";
    s["CHECKBOX_ChromaKey"]       = _chromaEnable->isChecked() ? "1" : "0";
    s["BUTTON_ChromaColour"]      = _chromaColor.name().toUpper();
    s["SLIDER_ChromaKeyThreshold"] = _chromaThresh->value();
}
