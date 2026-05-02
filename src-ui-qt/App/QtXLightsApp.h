#pragma once

#include <nlohmann/json.hpp>
#include "../Bridge/QtSequenceDoc.h"

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

class SequencerModel;

class QtXLightsApp : public QObject {
    Q_OBJECT

public:
    static QtXLightsApp& instance();

    struct EffectInfo {
        QString name;
        QString jsonPath;
    };

    const QList<EffectInfo>& effects() const { return _effects; }
    nlohmann::json effectMetadata(const QString& name) const;
    QString effectMetadataDir() const;
    QString resourcesDir() const;

    QString showFolder() const { return _showFolder; }
    void    setShowFolder(const QString& path);

    // Path to the INI settings file — use this whenever creating a QSettings.
    static QString settingsFilePath();

    // Load an .xsq file and push rows into the given SequencerModel.
    // Returns the loaded sequence info (isValid() == false on failure).
    QtSequenceInfo openSequence(const QString& path, SequencerModel* model);

    const QtSequenceInfo& currentSequence() const { return _sequence; }

signals:
    void initialized();
    void sequenceLoaded(const QtSequenceInfo& info);

private:
    explicit QtXLightsApp(QObject* parent = nullptr);
    void    loadEffectMetadata();
    QString resolveResourcesDir() const;

    QList<EffectInfo>             _effects;
    QMap<QString, nlohmann::json> _metadata;
    QString                       _resourcesDir;
    QString                       _showFolder;
    QtSequenceInfo                _sequence;
};
