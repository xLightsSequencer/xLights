#pragma once

#include <QColor>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QString>

// Lightweight xsq parser built on pugixml.
// Reads the elements/layers/effects from a .xsq file without linking src-core.
// Phase 5+ will replace this with a full SequenceElements integration.

// One named sub-model definition.
// Ranges type: line0/line1/… hold node-range strings ("1-10").
// Subbuffer type: ranges[0] holds the subBuffer expression.
struct QtSubModelInfo {
    QString     name;
    QString     layout      = "horizontal"; // "horizontal" | "vertical"
    QString     type        = "ranges";     // "ranges" | "subbuffer"
    QString     bufferStyle = "Default";
    QStringList ranges;     // line0, line1, … values (ranges type) OR single subBuffer expr
};

// One face definition (maps phonemes → node ranges or single nodes).
// attrs contains all XML attributes verbatim, including Name and Type.
// Colour for feature F is stored in attrs[F + "Color"] when present.
struct QtFaceInfo {
    QString               name;
    QString               type;   // "SingleNode" | "NodeRange" | "Matrix"
    bool forceColor { false }; // whether to use the color attributes or ignore them //CustomColors="" or CustomColors="1"
    QMap<QString,QString> attrs;
};

// One entry inside a state definition.
struct QtStateEntry {
    QString name;    // raw XML value, "s001-name"
    QString nodes;  // raw XML value, "s001"
    QString color;  // raw XML value, "s001-color", color value, e.g. "Red" or "#FF0000"
};

// One named state (maps node keys → colors).
struct QtStateInfo {
    QString              name;
    QString              type;   // "SingleNode" | "NodeRange"
    bool forceColor { false }; //CustomColors="" or CustomColors="1"
    QList<QtStateEntry>  entries;
};

// Model dimensions and physical node layout loaded from xlights_rgbeffects.xml.
struct QtModelInfo {
    QString       name;
    QString       type;           // xLights DisplayAs string, e.g. "Matrix", "Arch"
    int           bufferW   = 100;
    int           bufferH   = 1;
    int           nodeCount = 100;
    int           startChannel = 1;   // 1-based DMX start channel from show file

    // World position and scale from the layout canvas.
    // worldPosX/Y: center of the model in layout units (Y increases downward).
    // scaleX/Y: scale factor applied to each buffer pixel to get layout units.
    double worldPosX = 0.0, worldPosY = 0.0;
    double scaleX    = 1.0, scaleY    = 1.0;

    // Physical extent in world space derived from the core's screenX/Y range.
    // Populated by coreNodePositions(); 0 when the math fallback was used.
    // Used to compute globalPositions with the correct physical width/height
    // (replaces bufferW × scaleX which is wrong for shaped models like trees).
    double screenRangeX = 0.0, screenRangeY = 0.0;

    // Normalized [0,1]×[0,1] position for each node (size == nodeCount).
    // Gives the physical layout so PreviewWidget can draw the model shape.
    // Empty until a show file is successfully parsed.
    QList<QPointF> nodePositions;

    // Global layout positions for each node in layout units (same space as worldPos).
    // Computed in loadModels from nodePositions + worldPos + scale.
    QList<QPointF> globalPositions;

    // Controller wiring from the <model> element.
    QString controllerName;     // value of Controller="" attribute
    int     controllerPort = 0; // Port from <ControllerConnection Port="N"/>

    // Sub-models, faces, and states parsed from the <model> element.
    QList<QtSubModelInfo> subModels;
    QList<QtFaceInfo>     faces;
    QList<QtStateInfo>    states;
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

    // Port capabilities from ControllerCaps (0 = unknown / not a smart controller).
    int     pixelPortCount    = 0;
    int     serialPortCount   = 0;
    int     pixelPortChannels = 0;   // max channels per pixel port
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

// A model group: a named collection of models that can have effects applied
// to them collectively.  The group buffer is computed from the bounding box
// of all member models' global positions.
struct QtModelGroupInfo {
    QString     name;
    QStringList modelNames;    // member model names (ordered as in the show file)
    QString     layout;        // e.g. "minimalGrid", "Default", "Per Model Default"

    // Group render buffer dimensions (bounding-box scaled to ≤256 pixels).
    int    bufferW = 100;
    int    bufferH = 100;

    // Bounding box of member models in global layout units.
    double minX = 0, minY = 0, maxX = 1, maxY = 1;

    bool isValid() const { return !name.isEmpty() && !modelNames.isEmpty(); }
};

struct QtSequenceInfo {
    QString title;
    QString mediaFile;
    int     frameMs       = 50;    // ms per frame (50 = 20fps)
    int     durationMs    = 0;
    QList<QtSequenceElement> elements;
    QMap<QString, QtModelInfo>      models;      // keyed by model name
    QMap<QString, QtModelGroupInfo> groups;      // keyed by group name
    QList<QtControllerInfo>         controllers; // from xlights_networks.xml

    int fps() const { return frameMs > 0 ? 1000 / frameMs : 20; }
    int totalFrames() const { return frameMs > 0 ? durationMs / frameMs : 0; }

    bool isValid() const { return durationMs > 0; }

    // True if the name is a model group rather than an individual model.
    bool isGroup(const QString& name) const { return groups.contains(name); }

    // Returns model info for the given name.
    // For groups: returns a synthetic QtModelInfo with group buffer dimensions.
    // For unknown names: returns the 100×1 default.
    QtModelInfo modelInfo(const QString& name) const {
        auto mit = models.find(name);
        if (mit != models.end()) return *mit;

        auto git = groups.find(name);
        if (git != groups.end()) {
            QtModelInfo g;
            g.name    = name;
            g.type    = "Group";
            g.bufferW = git->bufferW;
            g.bufferH = git->bufferH;
            g.nodeCount = g.bufferW * g.bufferH;
            // Place the group at the centre of its bounding box.
            g.worldPosX = (git->minX + git->maxX) * 0.5;
            g.worldPosY = (git->minY + git->maxY) * 0.5;
            g.scaleX = (git->maxX - git->minX) / qMax(1, g.bufferW);
            g.scaleY = (git->maxY - git->minY) / qMax(1, g.bufferH);
            return g;
        }

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
