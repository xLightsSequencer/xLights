#include "PlaybackController.h"

#include <QUrl>
#include <spdlog/spdlog.h>

PlaybackController::PlaybackController(QObject* parent) : QObject(parent) {
    _timer.setInterval(40);
    connect(&_timer, &QTimer::timeout, this, &PlaybackController::tick);

    _audio  = new QAudioOutput(this);
    _player = new QMediaPlayer(this);
    _player->setAudioOutput(_audio);
    _audio->setVolume(1.0f);
}

// ── Audio file ────────────────────────────────────────────────────────────────

void PlaybackController::setMediaFile(const QString& path) {
    bool wasPlaying = _playing;
    if (wasPlaying) pause();

    if (path.isEmpty()) {
        _player->setSource(QUrl());
        return;
    }

    QUrl url = QUrl::fromLocalFile(path);
    _player->setSource(url);
    spdlog::info("PlaybackController: audio source set to '{}'",
                 path.toStdString());

    if (wasPlaying) play();
}

void PlaybackController::setVolume(int pct) {
    _audio->setVolume(float(qBound(0, pct, 100)) / 100.f);
}

// ── Transport slots ───────────────────────────────────────────────────────────

void PlaybackController::play() {
    if (_playing || _durationMs == 0) return;
    if (_posMs >= _durationMs) _posMs = 0;
    _baseMs = _posMs;
    _elapsed.start();
    _playing = true;

    // Start audio at the current position.
    if (_player->source().isValid()) {
        _player->setPosition(qint64(_posMs));
        _player->play();
    }

    _timer.start();
    emit playingChanged(true);
}

void PlaybackController::pause() {
    if (!_playing) return;
    _timer.stop();
    _player->pause();
    _playing = false;
    emit playingChanged(false);
}

void PlaybackController::stop() {
    _timer.stop();
    bool was = _playing;
    _playing = false;
    _posMs   = 0;
    _baseMs  = 0;
    _player->stop();
    _player->setPosition(0);
    if (was) emit playingChanged(false);
    emit positionChanged(0);
}

void PlaybackController::seek(int ms) {
    _posMs  = qBound(0, ms, _durationMs);
    _baseMs = _posMs;
    if (_playing) {
        _elapsed.restart();
        _player->setPosition(qint64(_posMs));
    }
    emit positionChanged(_posMs);
}

void PlaybackController::setDuration(int ms) {
    _durationMs = qMax(0, ms);
    if (_posMs > _durationMs) seek(0);
}

// ── Tick ─────────────────────────────────────────────────────────────────────

void PlaybackController::tick() {
    // Use the audio player position as the authoritative clock when audio is
    // loaded and playing, so visuals stay in sync with the actual sound.
    int pos;
    if (_player->source().isValid() &&
        _player->playbackState() == QMediaPlayer::PlayingState) {
        pos = qBound(0, int(_player->position()), _durationMs);
    } else {
        pos = qBound(0, _baseMs + int(_elapsed.elapsed()), _durationMs);
    }

    if (pos != _posMs) {
        _posMs = pos;
        emit positionChanged(_posMs);
    }

    if (_posMs >= _durationMs) {
        _timer.stop();
        _player->stop();
        _playing = false;
        emit playingChanged(false);
    }
}
