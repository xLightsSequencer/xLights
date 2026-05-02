#include "TransportToolBar.h"
#include "PlaybackController.h"

#include <QAction>
#include <QKeySequence>
#include <QShortcut>
#include <QSignalBlocker>
#include <QWidgetAction>

TransportToolBar::TransportToolBar(PlaybackController* ctrl, QWidget* parent)
    : QToolBar("Transport", parent), _ctrl(ctrl)
{
    setMovable(false);

    _playAct  = addAction(u8"▶  Play",  ctrl, &PlaybackController::play);
    _pauseAct = addAction(u8"⏸  Pause", ctrl, &PlaybackController::pause);
    addAction(u8"■  Stop", ctrl, &PlaybackController::stop);

    // Space bar toggles play/pause from anywhere in the window.
    auto* spaceShortcut = new QShortcut(QKeySequence(Qt::Key_Space), parent,
                                        nullptr, nullptr, Qt::WindowShortcut);
    connect(spaceShortcut, &QShortcut::activated, ctrl, [ctrl]() {
        if (ctrl->isPlaying()) ctrl->pause();
        else                   ctrl->play();
    });
    addSeparator();
    auto* renderAct = addAction(u8"⚡ Render");
    renderAct->setToolTip("Render all layers for selected model at current frame");
    connect(renderAct, &QAction::triggered, this, &TransportToolBar::renderAllRequested);
    auto* houseAct  = addAction(u8"\U0001F3E0 House");
    houseAct->setToolTip("Render all models and update house preview");
    connect(houseAct, &QAction::triggered, this, &TransportToolBar::renderHouseRequested);
    addSeparator();

    _slider = new QSlider(Qt::Horizontal);
    _slider->setMinimum(0);
    _slider->setMaximum(0);
    _slider->setMinimumWidth(300);
    auto* sa = new QWidgetAction(this);
    sa->setDefaultWidget(_slider);
    addAction(sa);

    addSeparator();

    _timeLbl = new QLabel("0:00.000");
    _timeLbl->setMinimumWidth(80);
    _timeLbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    auto* la = new QWidgetAction(this);
    la->setDefaultWidget(_timeLbl);
    addAction(la);

    addSeparator();

    // Volume knob: 🔊 + slider 0–100
    auto* volLbl = new QLabel(u8" 🔊");
    auto* vla = new QWidgetAction(this);
    vla->setDefaultWidget(volLbl);
    addAction(vla);

    _volSlider = new QSlider(Qt::Horizontal);
    _volSlider->setRange(0, 100);
    _volSlider->setValue(100);
    _volSlider->setMaximumWidth(90);
    _volSlider->setToolTip("Volume");
    auto* vsa = new QWidgetAction(this);
    vsa->setDefaultWidget(_volSlider);
    addAction(vsa);

    connect(_volSlider, &QSlider::valueChanged, this, [ctrl](int v) {
        ctrl->setVolume(v);
    });

    connect(ctrl, &PlaybackController::positionChanged, this, &TransportToolBar::onPositionChanged);
    connect(ctrl, &PlaybackController::playingChanged,  this, &TransportToolBar::onPlayingChanged);

    connect(_slider, &QSlider::sliderPressed,  this, [this] { _dragging = true; });
    connect(_slider, &QSlider::sliderReleased, this, [this] {
        _dragging = false;
        _ctrl->seek(_slider->value());
    });
    connect(_slider, &QSlider::sliderMoved, this, [this](int v) {
        _timeLbl->setText(msToStr(v));
    });

    onPlayingChanged(false);
}

void TransportToolBar::setDuration(int ms) {
    _slider->setMaximum(ms);
}

void TransportToolBar::onPositionChanged(int ms) {
    if (!_dragging) {
        QSignalBlocker b(_slider);
        _slider->setValue(ms);
        _timeLbl->setText(msToStr(ms));
    }
}

void TransportToolBar::onPlayingChanged(bool playing) {
    _playAct->setEnabled(!playing);
    _pauseAct->setEnabled(playing);
}

QString TransportToolBar::msToStr(int ms) {
    int m   = ms / 60000;
    int s   = (ms % 60000) / 1000;
    int rem = ms % 1000;
    return QString("%1:%2.%3")
        .arg(m)
        .arg(s,   2, 10, QChar('0'))
        .arg(rem, 3, 10, QChar('0'));
}
