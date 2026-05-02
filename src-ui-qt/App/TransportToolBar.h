#pragma once
#include <QLabel>
#include <QSlider>
#include <QToolBar>

class PlaybackController;

class TransportToolBar : public QToolBar {
    Q_OBJECT
public:
    explicit TransportToolBar(PlaybackController* ctrl, QWidget* parent = nullptr);

    void setDuration(int ms);

signals:
    void renderAllRequested();
    void renderHouseRequested();

private slots:
    void onPositionChanged(int ms);
    void onPlayingChanged(bool playing);

private:
    PlaybackController* _ctrl;
    QAction*  _playAct;
    QAction*  _pauseAct;
    QSlider*  _slider;
    QLabel*   _timeLbl;
    QSlider*  _volSlider = nullptr;
    bool      _dragging  = false;

    static QString msToStr(int ms);
};
