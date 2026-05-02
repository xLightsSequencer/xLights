#pragma once
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>

class PlaybackController : public QObject {
    Q_OBJECT
public:
    explicit PlaybackController(QObject* parent = nullptr);

    int  positionMs() const { return _posMs; }
    bool isPlaying()  const { return _playing; }
    int  durationMs() const { return _durationMs; }

    // Load the audio file for the current sequence (empty string = no audio).
    void setMediaFile(const QString& path);

    // Set volume 0–100.
    void setVolume(int pct);

public slots:
    void play();
    void pause();
    void stop();
    void seek(int ms);
    void setDuration(int ms);

signals:
    void positionChanged(int ms);
    void playingChanged(bool playing);

private slots:
    void tick();

private:
    QTimer        _timer;
    QElapsedTimer _elapsed;
    int  _posMs      = 0;
    int  _baseMs     = 0;
    int  _durationMs = 0;
    bool _playing    = false;

    QMediaPlayer* _player = nullptr;
    QAudioOutput* _audio  = nullptr;
};
