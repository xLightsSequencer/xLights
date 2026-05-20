#include "SubBufferWidget.h"
#include "SubBufferCanvas.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>

SubBufferWidget::SubBufferWidget(QWidget* parent) : QWidget(parent) {
    auto* box = new QGroupBox("Buffer", this);
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(box);

    auto* form = new QFormLayout(box);
    form->setContentsMargins(6, 4, 6, 4);
    form->setSpacing(4);

    // ── Buffer Style ──────────────────────────────────────────────────────
    _style = new QComboBox(box);
    for (const char* s : {"Default",
                           "Single Line",
                           "Horizontal Per Model/Strand",
                           "Vertical Per Model/Strand",
                           "Per Model Default",
                           "Per Preview",
                           "Per Preview Model",
                           "Rotate CW 90",
                           "Rotate CCW 90"})
        _style->addItem(s);
    form->addRow("Style:", _style);

    // ── Buffer Transform ──────────────────────────────────────────────────
    _transform = new QComboBox(box);
    for (const char* t : {"None",
                           "Rotate CW 90",
                           "Rotate CCW 90",
                           "Rotate 180",
                           "Flip Horizontal",
                           "Flip Vertical",
                           "Flip H&V"})
        _transform->addItem(t);
    form->addRow("Transform:", _transform);

    // ── Overlay Background ────────────────────────────────────────────────
    _overlay = new QCheckBox(box);
    form->addRow("Overlay Bkg:", _overlay);

    // ── Sub-Buffer canvas ─────────────────────────────────────────────────────
    _oversized = new QCheckBox("Oversized (-100 to 200)", box);
    form->addRow(_oversized);

    _canvas = new SubBufferCanvas(box);
    form->addRow(_canvas);

    // ── Sub-Buffer sliders ─────────────────────────────────────────────────────
    auto makeSlider = [&](const QString& label) {
        auto* sl = new QSlider(Qt::Horizontal, box);
        sl->setRange(0, 100);
        sl->setValue(0);
        sl->setTickPosition(QSlider::TicksBelow);
        sl->setTickInterval(25);
        form->addRow(label, sl);
        return sl;
    };
    _left   = makeSlider("Sub Left %:");
    _right  = makeSlider("Sub Right %:");
    _bottom = makeSlider("Sub Bottom %:");
    _top    = makeSlider("Sub Top %:");
    _right->setValue(100);
    _top->setValue(100);

    _sbLabel = new QLabel("L:0  R:100  B:0  T:100", box);
    _sbLabel->setStyleSheet("color:#888; font-size:10px;");
    form->addRow("", _sbLabel);

    connectSignals();
}

void SubBufferWidget::connectSignals() {
    auto notify = [this]() {
        updateSubBufferLabel();
        // Keep canvas in sync with sliders.
        QSignalBlocker b(_canvas);
        _canvas->setRegion(_left->value(), _bottom->value(),
                           _right->value(), _top->value());
        emit changed();
    };
    connect(_style,     QOverload<int>::of(&QComboBox::currentIndexChanged), this, notify);
    connect(_transform, QOverload<int>::of(&QComboBox::currentIndexChanged), this, notify);
    connect(_overlay,   &QCheckBox::toggled,          this, notify);
    connect(_left,      &QSlider::valueChanged,       this, notify);
    connect(_right,     &QSlider::valueChanged,       this, notify);
    connect(_bottom,    &QSlider::valueChanged,       this, notify);
    connect(_top,       &QSlider::valueChanged,       this, notify);

    // Canvas → sliders
    connect(_canvas, &SubBufferCanvas::regionChanged,
            this, [this](int l, int b, int r, int t) {
        QSignalBlocker bL(_left), bR(_right), bB(_bottom), bT(_top);
        _left->setValue(l); _right->setValue(r);
        _bottom->setValue(b); _top->setValue(t);
        updateSubBufferLabel();
        emit changed();
    });

    // Oversized toggle: expand/contract slider ranges and canvas coordinate space.
    connect(_oversized, &QCheckBox::toggled, this, [this](bool on) {
        setSliderRanges(on);
        _canvas->setOversized(on);
        // setOversized emits regionChanged which updates sliders; suppress double-emit.
        emit changed();
    });
}

void SubBufferWidget::setSliderRanges(bool oversized) {
    const int lo = oversized ? -100 : 0;
    const int hi = oversized ?  200 : 100;
    for (QSlider* sl : {_left, _right, _bottom, _top})
        sl->setRange(lo, hi);
}

void SubBufferWidget::updateSubBufferLabel() {
    _sbLabel->setText(QString("L:%1  R:%2  B:%3  T:%4")
                      .arg(_left->value())
                      .arg(_right->value())
                      .arg(_bottom->value())
                      .arg(_top->value()));
}

// ── Sub-buffer codec ──────────────────────────────────────────────────────────
// xLights encodes as "x1xy1xx2xy2x0x0" where the separator is the letter 'x'.
// x1=left%, y1=bottom%, x2=right%, y2=top%, last two are centre offsets (0).

QString SubBufferWidget::encodeSubBuffer(int left, int bottom, int right, int top) {
    if (left == 0 && bottom == 0 && right == 100 && top == 100)
        return {};   // empty = full buffer (no sub-buffer)
    return QString("%1x%2x%3x%4x0x0").arg(left).arg(bottom).arg(right).arg(top);
}

void SubBufferWidget::decodeSubBuffer(const QString& sb,
                                      int& left, int& bottom, int& right, int& top) {
    left = 0; bottom = 0; right = 100; top = 100;
    if (sb.isEmpty()) return;

    // Replace "Max" with a placeholder that has no 'x', then split on 'x'.
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

// ── Public API ────────────────────────────────────────────────────────────────

void SubBufferWidget::loadSettings(const QVariantMap& settings) {
    QSignalBlocker bStyle(_style), bTransform(_transform),
                   bOverlay(_overlay), bL(_left), bR(_right), bB(_bottom), bT(_top),
                   bCanvas(_canvas), bOver(_oversized);

    // Buffer style
    const QString style = settings.value("CHOICE_BufferStyle", "Default").toString();
    int idx = _style->findText(style);
    _style->setCurrentIndex(idx >= 0 ? idx : 0);

    // Transform
    const QString xform = settings.value("CHOICE_BufferTransform", "None").toString();
    int tidx = _transform->findText(xform);
    _transform->setCurrentIndex(tidx >= 0 ? tidx : 0);

    // Overlay background
    _overlay->setChecked(settings.value("CHECKBOX_OverlayBkg", "0").toString() == "1");

    // Sub-buffer
    const QString sb = settings.value("CUSTOM_SubBuffer", "").toString();
    int l, bo, r, t;
    decodeSubBuffer(sb, l, bo, r, t);
    // Auto-enable oversized mode if the loaded values are outside 0–100.
    const bool needOversized = (l < 0 || bo < 0 || r > 100 || t > 100);
    _oversized->setChecked(needOversized);
    setSliderRanges(needOversized);
    _canvas->setOversized(needOversized);

    _left->setValue(l);
    _bottom->setValue(bo);
    _right->setValue(r);
    _top->setValue(t);

    updateSubBufferLabel();
    _canvas->setRegion(l, bo, r, t);
}

void SubBufferWidget::writeSettings(QVariantMap& settings) const {
    settings["CHOICE_BufferStyle"]    = _style->currentText();
    settings["CHOICE_BufferTransform"]= _transform->currentText();
    settings["CHECKBOX_OverlayBkg"]   = _overlay->isChecked() ? "1" : "0";
    settings["CUSTOM_SubBuffer"]      = encodeSubBuffer(
        _left->value(), _bottom->value(), _right->value(), _top->value());
}
