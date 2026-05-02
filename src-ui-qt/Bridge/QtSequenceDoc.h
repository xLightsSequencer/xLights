#pragma once

#include <QColor>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QString>

// Lightweight xsq parser built on pugixml.
// Reads the elements/layers/effects from a .xsq file without linking src-core.
// Phase 5+ will replace this with a full SequenceElements integration.

// Model dimensions and physical node layout loaded from xlights_rgbeffects.xml.
struct QtModelInfo {
    QString       name;
    QString       type;           // xLights DisplayAs string, e.g. "Matrix", "Arch"
    int           bufferW   = 100;
    int           bufferH   = 1;
    int           nodeCount = 100;
    int           parm1        = 1;   // NumStrings / NumArches / etc.
    int           parm2        = 1;   // NodesPerString / NodesPerArch / etc.
    int           startChannel = 1;   // 1-based DMX start channel from show file

    // World position and scale from the layout canvas.
    // worldPosX/Y: center of the model in layout units (Y increases downward).
    // scaleX/Y: scale factor applied to each buffer pixel to get layout units.
    double worldPosX = 0.0, worldPosY = 0.0;
    double scaleX    = 1.0, scaleY    = 1.0;

    // For Star models with a "LayerSizes" attribute (concentric rings):
    // each entry is the node count in that ring, innermost first.
    // Empty for single-layer stars.
    QList<int> layerSizes;

    // Normalized [0,1]×[0,1] position for each node (size == nodeCount).
    // Gives the physical layout so PreviewWidget can draw the model shape.
    // Empty until a show file is successfully parsed.
    QList<QPointF> nodePositions;

    // Global layout positions for each node in layout units (same space as worldPos).
    // Computed in loadModels from nodePositions + worldPos + scale.
    QList<QPointF> globalPositions;
};

// Controller record extracted from xlights_networks.xml at sequence open time.
struct QtControllerInfo {
    QString name;
    QString type;           // "Ethernet", "Serial", "DMX", etc.
    QString ip;             // IP address or COM port
    int     startChannel = 1;
    int     channelCount = 0;
    int     universeOrBaud = 0;
    QString protocol;       // e131, artnet, DDP, DMX, …
};

struct QtEffectBlock {
    int     startMs   = 0;
    int     endMs     = 0;
    QString name;         // effect name e.g. "Bars", "Rainbow"
    QString settings;     // raw "key=value,..." settings string from xsq effectDB
    QString palette;      // raw "key=value,..." palette string from xsq ColorPalettes
};

struct QtSequenceLayer {
    QList<QtEffectBlock> blocks;
};

struct QtSequenceElement {
    QString name;          // model name
    QString type;          // "model", "timing", etc.
    QList<QtSequenceLayer> layers;
};

struct QtSequenceInfo {
    QString title;
    QString mediaFile;
    int     frameMs       = 50;    // ms per frame (50 = 20fps)
    int     durationMs    = 0;
    QList<QtSequenceElement> elements;
    QMap<QString, QtModelInfo>      models;      // keyed by model name
    QList<QtControllerInfo>         controllers; // from xlights_networks.xml

    int fps() const { return frameMs > 0 ? 1000 / frameMs : 20; }
    int totalFrames() const { return frameMs > 0 ? durationMs / frameMs : 0; }

    bool isValid() const { return durationMs > 0; }

    // Returns model info for the given name, or a 100×1 default if unknown.
    QtModelInfo modelInfo(const QString& name) const {
        auto it = models.find(name);
        if (it != models.end()) return *it;
        QtModelInfo def; def.name = name; return def;
    }
};

class QtSequenceDoc {
public:
    // Load a .xsq file. showFolderHint is checked first for xlights_rgbeffects.xml;
    // falls back to the xsq directory and its parent if the hint is empty or misses.
    static QtSequenceInfo load(const QString& path,
                               const QString& showFolderHint = {});

    // Parse model definitions from a show file into an existing QtSequenceInfo.
    static void loadModels(const QString& showFilePath, QtSequenceInfo& info);
};
