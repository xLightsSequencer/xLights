#include "QtXLightsApp.h"
#include "../Bridge/QtSequenceDoc.h"
#include "../Sequencer/SequencerModel.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>

// ── Settings file path ────────────────────────────────────────────────────────

QString QtXLightsApp::settingsFilePath() {
    // Use AppDataLocation so the file is in a writable, user-specific directory
    // on every platform, independent of how QSettings::setPath is configured.
    const QString dir = QStandardPaths::writableLocation(
                            QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/settings.ini";
}

// ── Singleton ─────────────────────────────────────────────────────────────────

QtXLightsApp& QtXLightsApp::instance() {
    static QtXLightsApp inst;
    return inst;
}

QtXLightsApp::QtXLightsApp(QObject* parent) : QObject(parent) {
    // ── File logging ──────────────────────────────────────────────────────
    // Mirror the wx app: one rotating file sink shared by all named loggers.
    // 10 MB per file, keep 5 rotations.
    const std::string logDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            .toStdString();
    QDir().mkpath(QString::fromStdString(logDir));
    const std::string logPath = logDir + "/xLights-Qt.log";

    try {
        auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logPath, 1024 * 1024 * 10, 5);

        // Default logger (replaces the spdlog stdout default).
        auto mainLogger = std::make_shared<spdlog::logger>("xLights-Qt", fileSink);
        spdlog::set_default_logger(mainLogger);
        spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e [%n %l] %v");
        spdlog::flush_on(spdlog::level::warn);

        // Named loggers expected by src-core.
        for (const char* name : {"work", "render", "curl", "opengl", "job"}) {
            if (!spdlog::get(name))
                spdlog::register_logger(
                    std::make_shared<spdlog::logger>(name, fileSink));
        }
    } catch (const std::exception& e) {
        // If the log file can't be opened (e.g. first-run permission issue),
        // fall back to the spdlog default stdout logger so the app still runs.
        spdlog::warn("Could not open log file '{}': {}", logPath, e.what());
    }

    _resourcesDir = resolveResourcesDir();
    spdlog::info("QtXLightsApp: resources at '{}'", _resourcesDir.toStdString());
    loadEffectMetadata();
    spdlog::info("QtXLightsApp: {} effects loaded", _effects.size());

    // Restore persisted show folder.
    QSettings s(settingsFilePath(), QSettings::IniFormat);
    _showFolder = s.value("showFolder").toString();
    if (!_showFolder.isEmpty())
        spdlog::info("QtXLightsApp: restored show folder '{}'",
                     _showFolder.toStdString());

    emit initialized();
}

QString QtXLightsApp::resolveResourcesDir() const {
    QDir exeDir(QApplication::applicationDirPath());
    if (QDir(exeDir.filePath("effectmetadata")).exists())
        return exeDir.absolutePath();
    QDir d = exeDir;
    for (int i = 0; i < 8; ++i) {
        QString candidate = d.filePath("resources");
        if (QDir(candidate + "/effectmetadata").exists())
            return candidate;
        if (!d.cdUp()) break;
    }
    spdlog::warn("QtXLightsApp: could not locate resources directory from '{}'",
                 exeDir.absolutePath().toStdString());
    return exeDir.absolutePath();
}

QString QtXLightsApp::effectMetadataDir() const {
    QString direct = QApplication::applicationDirPath() + "/effectmetadata";
    if (QDir(direct).exists()) return direct;
    return _resourcesDir + "/effectmetadata";
}

QString QtXLightsApp::resourcesDir() const { return _resourcesDir; }

void QtXLightsApp::loadEffectMetadata() {
    QDir dir(effectMetadataDir());
    if (!dir.exists()) {
        spdlog::error("QtXLightsApp: effectmetadata dir not found: '{}'",
                      dir.absolutePath().toStdString());
        return;
    }
    for (const QFileInfo& fi : dir.entryInfoList({"*.json"}, QDir::Files, QDir::Name)) {
        QFile f(fi.absoluteFilePath());
        if (!f.open(QIODevice::ReadOnly)) continue;
        nlohmann::json doc;
        try {
            doc = nlohmann::json::parse(f.readAll().toStdString());
        } catch (const nlohmann::json::exception& ex) {
            spdlog::warn("QtXLightsApp: skip '{}': {}", fi.fileName().toStdString(), ex.what());
            continue;
        }
        if (!doc.contains("effectName") || !doc["effectName"].is_string()) continue;
        QString name = QString::fromStdString(doc["effectName"].get<std::string>());
        _effects.append({name, fi.absoluteFilePath()});
        _metadata.insert(name, doc);
    }
}

nlohmann::json QtXLightsApp::effectMetadata(const QString& name) const {
    auto it = _metadata.find(name);
    return it != _metadata.end() ? *it : nlohmann::json{};
}

// ── Show folder ───────────────────────────────────────────────────────────────
void QtXLightsApp::setShowFolder(const QString& path) {
    _showFolder = path;
    QSettings(settingsFilePath(), QSettings::IniFormat).setValue("showFolder", path);
    spdlog::info("QtXLightsApp: show folder → '{}'", path.toStdString());
    // If a sequence is already loaded, refresh model definitions from the new folder.
    if (_sequence.isValid() && !path.isEmpty()) {
        _sequence.models.clear();
        QtSequenceDoc::loadModels(path + "/xlights_rgbeffects.xml", _sequence);
        spdlog::info("QtXLightsApp: reloaded {} model defs from show folder",
                     _sequence.models.size());
        emit sequenceLoaded(_sequence);
    }
}

// ── Sequence loading ──────────────────────────────────────────────────────────
QtSequenceInfo QtXLightsApp::openSequence(const QString& path, SequencerModel* model) {
    _sequence = QtSequenceDoc::load(path, _showFolder);
    if (!_sequence.isValid()) {
        spdlog::error("QtXLightsApp: failed to load sequence '{}'", path.toStdString());
        return _sequence;
    }

    if (model) {
        model->loadFromSequence(_sequence);
    }

    emit sequenceLoaded(_sequence);
    return _sequence;
}
