#include "QtSequenceDoc.h"

#include <pugixml.hpp>
#include <spdlog/spdlog.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPointF>

// src-core model deserialization (wx-free, compiled into xLights-Qt via SRC_CORE)
#include "../../src-core/XmlSerializer/XmlDeserializingModelFactory.h"
#include "../../src-core/models/Model.h"
#include "../../src-core/models/ModelManager.h"
#include "../../src-core/outputs/OutputManager.h"
#include "../../src-core/outputs/Controller.h"
#include "../../src-core/controllers/ControllerCaps.h"

// ── Helpers ───────────────────────────────────────────────────────────────────
static int roundToFrame(int ms, int frameMs) {
    if (frameMs <= 0) return ms;
    return (ms / frameMs) * frameMs;
}

QtSequenceInfo QtSequenceDoc::load(const QString& path, const QString& showFolderHint) {
    QtSequenceInfo info;

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        spdlog::error("QtSequenceDoc: cannot open '{}'", path.toStdString());
        return info;
    }
    QByteArray data = f.readAll();
    f.close();

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(data.constData(), data.size());
    if (!result) {
        spdlog::error("QtSequenceDoc: XML parse error in '{}': {}",
                      path.toStdString(), result.description());
        return info;
    }

    pugi::xml_node root = doc.child("xsequence");
    if (!root) {
        spdlog::error("QtSequenceDoc: no <xsequence> root in '{}'", path.toStdString());
        return info;
    }

    // ── Head ──────────────────────────────────────────────────────────────
    if (auto head = root.child("head")) {
        for (auto el : head.children()) {
            std::string name = el.name();
            std::string text = el.text().as_string("");
            if (name == "sequenceTiming") {
                int ms = std::strtol(text.c_str(), nullptr, 10);
                if (ms > 0) info.frameMs = ms;
            } else if (name == "sequenceDuration") {
                // xLights stores duration in seconds (e.g. "30.000"), not ms
                double secs = std::strtod(text.c_str(), nullptr);
                if (secs > 0.0) info.durationMs = int(secs * 1000.0);
            } else if (name == "mediaFile") {
                info.mediaFile = QString::fromStdString(text);
            } else if (name == "song") {
                info.title = QString::fromStdString(text);
            }
        }
    }

    if (info.title.isEmpty())
        info.title = QFileInfo(path).completeBaseName();

    spdlog::info("QtSequenceDoc: '{}' — {}ms/frame, {}ms duration, media='{}'",
                 info.title.toStdString(), info.frameMs, info.durationMs,
                 info.mediaFile.toStdString());

    // ── Effect lookup table (EffectDB) ────────────────────────────────────
    // xLights writes: <EffectDB><Effect>settings_string</Effect>...</EffectDB>
    // Children are indexed 0, 1, 2… by position — matching ref="N" on timeline
    // Effect nodes.  No id/name attributes exist on EffectDB children; the
    // effect name is always the inline name= attribute on the timeline node.
    std::unordered_map<int, std::string> effectDb;  // index → settings string
    if (auto db = root.child("EffectDB")) {
        int idx = 0;
        for (auto eff : db.children("Effect"))
            effectDb[idx++] = eff.text().as_string("");
    }

    // ── Color palette lookup table (ColorPalettes) ────────────────────────
    std::unordered_map<int, std::string> colorDb;
    if (auto cpNode = root.child("ColorPalettes")) {
        int idx = 0;
        for (auto cp : cpNode.children("ColorPalette"))
            colorDb[idx++] = cp.text().as_string("");
    }

    // ── ElementEffects / SequenceElements ─────────────────────────────────
    // Both tags appear in different file versions; try both.
    auto elementContainer = root.child("ElementEffects");
    if (!elementContainer)
        elementContainer = root.child("SequenceElements");
    if (!elementContainer) {
        spdlog::warn("QtSequenceDoc: no ElementEffects/SequenceElements in '{}'",
                     path.toStdString());
        return info;
    }

    for (auto elemNode : elementContainer.children("Element")) {
        std::string elemType = elemNode.attribute("type").as_string("model");
        if (elemType == "timing") continue;   // skip timing tracks for now

        QtSequenceElement elem;
        elem.name = QString::fromStdString(elemNode.attribute("name").as_string(""));
        elem.type = QString::fromStdString(elemType);

        for (auto layerNode : elemNode.children("EffectLayer")) {
            QtSequenceLayer layer;
            for (auto effNode : layerNode.children("Effect")) {
                // Effect name: always on the inline name= / label= attribute.
                // Settings: looked up by ref index into EffectDB.
                std::string effName = effNode.attribute("name").as_string(
                                          effNode.attribute("label").as_string(""));

                std::string effSettings;
                int ref = effNode.attribute("ref").as_int(-1);
                if (ref >= 0) {
                    auto it = effectDb.find(ref);
                    if (it != effectDb.end()) effSettings = it->second;
                }

                if (effName.empty()) effName = "Unknown";

                // Resolve palette string via palette ref.
                std::string effPalette;
                int palRef = effNode.attribute("palette").as_int(-1);
                if (palRef >= 0) {
                    auto it = colorDb.find(palRef);
                    if (it != colorDb.end()) effPalette = it->second;
                }

                int startMs = std::strtol(
                    effNode.attribute("startTime").as_string("0"), nullptr, 10);
                int endMs   = std::strtol(
                    effNode.attribute("endTime").as_string("0"), nullptr, 10);

                // Round to frame boundary
                startMs = roundToFrame(startMs, info.frameMs);
                endMs   = roundToFrame(endMs,   info.frameMs);
                if (endMs <= startMs) continue;

                QtEffectBlock blk;
                blk.startMs  = startMs;
                blk.endMs    = endMs;
                blk.name     = QString::fromStdString(effName);
                blk.settings = QString::fromStdString(effSettings);
                blk.palette  = QString::fromStdString(effPalette);
                layer.blocks.append(blk);
            }
            elem.layers.append(layer);
        }

        if (!elem.layers.isEmpty())
            info.elements.append(elem);
    }

    spdlog::info("QtSequenceDoc: loaded {} model elements", info.elements.size());

    // ── Show file (model dimensions) ──────────────────────────────────────
    // Check the explicit show folder hint first, then the xsq's own directory,
    // then one level up (sequences often live in a sub-folder of the show dir).
    QDir dir = QFileInfo(path).absoluteDir();
    QStringList candidates;
    if (!showFolderHint.isEmpty())
        candidates << showFolderHint + "/xlights_rgbeffects.xml";
    candidates << dir.filePath("xlights_rgbeffects.xml")
               << dir.absolutePath() + "/../xlights_rgbeffects.xml";
    for (const QString& sf : candidates) {
        if (!QFile::exists(sf)) continue;
        QtSequenceDoc::loadModels(sf, info);
        spdlog::info("QtSequenceDoc: loaded {} model defs from show file",
                     info.models.size());
        break;
    }
    if (info.models.isEmpty())
        spdlog::warn("QtSequenceDoc: no show file found — using default 100×1 buffers");

    return info;
}

// ── Show file parser ──────────────────────────────────────────────────────────

// ── Node position computation ─────────────────────────────────────────────────
// Returns normalized [0,1]×[0,1] positions matching the xLights OpenGL preview.
// Formulas are derived from xLights' model screenX/screenY calculations in
// SingleLineModel.cpp, MatrixModel.cpp, TreeModel.cpp, etc.

static QList<QPointF> computeNodePositions(const QString& type,
                                            int parm1, int parm2,
                                            int bufW, int bufH,
                                            const QList<int>& layerSizes = {})
{
    const int N = bufW * bufH;
    QList<QPointF> pos;
    pos.reserve(N);

    // ── Single Line / Poly Line / Arch / Icicles / Window Frame / etc. ──
    // Explicitly named 1-D types — drawn as a horizontal strip.
    // NOTE: do NOT add a generic "bufH==1" catch here; shaped models like
    // Star, Arches, and Candy Canes also have bufH==1 but need their own
    // shape-aware branches below.  Unknown bufH==1 types fall to the final else.
    if (type == "Single Line" || type == "Poly Line"
        || type == "Icicles"   || type == "Window Frame"
        || type == "Gridlines" || type == "Arch") {
        for (int i = 0; i < N; ++i)
            pos.append({ N > 1 ? double(i) / (N - 1) : 0.5, 0.5 });

    // ── Arch (single) ────────────────────────────────────────────────────
    } else if (type == "Arch" && bufH > 1) {   // only if 2-D (shouldn't happen)
        for (int i = 0; i < N; ++i) {
            double t = N > 1 ? double(i) / (N - 1) : 0.5;
            double a = M_PI * t;                       // 0 (left) → π (right)
            pos.append({ (1.0 - std::cos(a)) / 2.0,   // x: 0 → 1
                          1.0 - std::sin(a) * 0.90 }); // y: peak near top
        }

    // ── Multiple arches ───────────────────────────────────────────────────
    } else if (type == "Arches") {
        int nA = qMax(1, parm1), nP = qMax(1, parm2);
        double hw = 0.44 / nA;
        for (int a = 0; a < nA; ++a) {
            double cx = (a + 0.5) / nA;
            for (int n = 0; n < nP; ++n) {
                double t     = nP > 1 ? double(n) / (nP - 1) : 0.5;
                double angle = M_PI * t;
                pos.append({ cx + std::cos(M_PI - angle) * hw,
                              1.0 - std::sin(angle) * 0.88 });
            }
        }

    // ── Matrix (Horiz or Vert) ────────────────────────────────────────────
    // Grid of W×H nodes. bufW=cols, bufH=rows (already corrected in loadModels).
    } else if (type == "Horiz Matrix" || type == "Vert Matrix"
               || type == "Matrix"    || type == "Spinner") {
        // row=0 is the physical bottom (xLights y=0 is lower-left); flip so that
        // it maps to y=1.0 (bottom of screen in Qt coordinates).
        for (int row = 0; row < bufH; ++row)
            for (int col = 0; col < bufW; ++col)
                pos.append({ bufW > 1 ? double(col) / (bufW - 1) : 0.5,
                              bufH > 1 ? 1.0 - double(row) / (bufH - 1) : 0.5 });

    // ── Tree 360 (top-down circular) ──────────────────────────────────────
    // bufW = strands, bufH = nodes per strand.
    // Buffer is node-major: index = node*strands + strand.
    // Outer loop must be over nodes (bufY), inner over strands (bufX).
    } else if (type == "Tree 360") {
        int nS = qMax(1, parm1), nP = qMax(1, parm2);
        for (int n = 0; n < nP; ++n) {                    // bufY = node
            double r = double(n + 1) / nP * 0.46;
            for (int s = 0; s < nS; ++s) {                // bufX = strand
                double angle = 2.0 * M_PI * s / nS - M_PI / 2.0;
                pos.append({ 0.5 + std::cos(angle) * r,
                              0.5 + std::sin(angle) * r });
            }
        }

    // ── Tree 270 (¾-arc view) ─────────────────────────────────────────────
    } else if (type == "Tree 270") {
        int nS = qMax(1, parm1), nP = qMax(1, parm2);
        for (int n = 0; n < nP; ++n) {                    // bufY = node
            double r = double(n + 1) / nP * 0.46;
            for (int s = 0; s < nS; ++s) {                // bufX = strand
                double angle = (2.0 * M_PI * 0.75) * s / qMax(1, nS - 1)
                               - M_PI * 0.875;
                pos.append({ 0.5 + std::cos(angle) * r,
                              0.5 + std::sin(angle) * r });
            }
        }

    // ── Tree 180 (front view, half-arc fan) ──────────────────────────────
    // Strands fan from -90° to +90° at the base and converge to the tip.
    // Using arc distribution (sin) to match xLights layout canvas.
    } else if (type == "Tree 180") {
        int nS = qMax(1, parm1), nP = qMax(1, parm2);
        for (int n = 0; n < nP; ++n) {                    // bufY = node
            double t = nP > 1 ? double(n) / (nP - 1) : 0.0;
            double y = 1.0 - t;
            for (int s = 0; s < nS; ++s) {                // bufX = strand
                double angle = nS > 1 ? (double(s) / (nS - 1) - 0.5) * M_PI : 0.0;
                double xBase = 0.5 + 0.5 * std::sin(angle);
                double x = xBase * (1.0 - t) + 0.5 * t;
                pos.append({ x, y });
            }
        }

    // ── Tree (other/generic front view, triangular cone) ──────────────────
    // Strands spread at the base and converge to a tip at the top centre.
    } else if (type.startsWith("Tree")) {
        int nS = qMax(1, parm1), nP = qMax(1, parm2);
        for (int n = 0; n < nP; ++n) {                    // bufY = node
            double t = nP > 1 ? double(n) / (nP - 1) : 0.0;
            double y = 1.0 - t;
            for (int s = 0; s < nS; ++s) {                // bufX = strand
                double xBase = (s + 0.5) / nS;
                double x = xBase * (1.0 - t) + 0.5 * t;
                pos.append({ x, y });
            }
        }

    // ── Circle / concentric rings ─────────────────────────────────────────
    } else if (type == "Circle") {
        int nR = qMax(1, parm1), nP = qMax(1, parm2);
        for (int r = 0; r < nR; ++r) {
            double radius = double(r + 1) / nR * 0.45;
            for (int n = 0; n < nP; ++n) {
                double a = 2.0 * M_PI * n / nP - M_PI / 2.0;
                pos.append({ 0.5 + std::cos(a) * radius,
                              0.5 + std::sin(a) * radius });
            }
        }

    // ── Star ─────────────────────────────────────────────────────────────
    // parm1 = StarPoints.  Nodes arranged along the star outline with
    // alternating outer (tip) and inner (valley) radii.
    // Multi-layer (concentric) stars: layerSizes gives node count per ring,
    // innermost first; each ring scales its radii proportionally.
    } else if (type == "Star") {
        int    P         = qMax(3, parm1);
        double outerRMax = 0.45;
        double innerRMax = outerRMax * 0.40;

        auto placeStarNodes = [&](int count, double outerR, double innerR) {
            for (int i = 0; i < count; ++i) {
                double t     = double(i) / count;
                double angle = 2.0 * M_PI * t - M_PI / 2.0;
                double phase = std::fmod(t * P, 1.0);
                double r = phase < 0.5
                           ? outerR - (outerR - innerR) * phase * 2.0
                           : innerR + (outerR - innerR) * (phase - 0.5) * 2.0;
                pos.append({ 0.5 + std::cos(angle) * r,
                              0.5 + std::sin(angle) * r });
            }
        };

        if (!layerSizes.isEmpty()) {
            // Concentric rings: innermost at smallest radius, outermost at max.
            int nL = layerSizes.size();
            for (int l = 0; l < nL; ++l) {
                double scale  = double(l + 1) / nL;
                placeStarNodes(layerSizes[l],
                               outerRMax * scale, innerRMax * scale);
            }
        } else {
            placeStarNodes(N, outerRMax, innerRMax);
        }

    // ── Candy Canes ───────────────────────────────────────────────────────
    } else if (type == "Candy Canes") {
        int    nC = qMax(1, parm1), nP = qMax(1, parm2);
        double hw = 0.36 / nC;
        for (int c = 0; c < nC; ++c) {
            double cx = (c + 0.5) / nC;
            for (int n = 0; n < nP; ++n) {
                double t = nP > 1 ? double(n) / (nP - 1) : 0.5;
                double x, y;
                if (t < 0.72) {
                    x = cx;
                    y = 1.0 - t / 0.72 * 0.80;
                } else {
                    double a = M_PI * (t - 0.72) / 0.28;
                    x = cx + std::sin(a) * hw * 0.7;
                    y = 0.20 - std::cos(a) * hw * 0.7 + hw * 0.7;
                }
                pos.append({ x, y });
            }
        }

    // ── Custom model ──────────────────────────────────────────────────────
    // Custom model data parsed separately; fall through to grid.

    // ── Generic 2-D fallback ─────────────────────────────────────────────
    // Flip y so row=0 (physical bottom in xLights coords) maps to y=1.0 (screen bottom).
    } else if (bufH > 1) {
        for (int row = 0; row < bufH; ++row)
            for (int col = 0; col < bufW; ++col)
                pos.append({ bufW > 1 ? double(col) / (bufW - 1) : 0.5,
                              bufH > 1 ? 1.0 - double(row) / (bufH - 1) : 0.5 });

    // ── Generic 1-D fallback ─────────────────────────────────────────────
    } else {
        for (int i = 0; i < N; ++i)
            pos.append({ N > 1 ? double(i) / (N - 1) : 0.5, 0.5 });
    }

    return pos;
}

// ── Core model → node positions ───────────────────────────────────────────────
// Deserialise one model using src-core, run InitRenderBufferNodes() to get
// the same screenX/screenY/screenZ the xLights OpenGL preview uses, then
// project onto the best 2-D plane for the model type.
//
// Projection rules (chosen to match what xLights' layout canvas shows):
//   Tree 360            → top-down  (X, Z)  — shows the radial circle
//   Tree 270 / 180      → front     (X, Y)  — shows the fan/cone profile
//   Everything else     → front     (X, Y)
static bool coreNodePositions(pugi::xml_node modelXml,
                               ModelManager& mm,
                               QtModelInfo& out)
{
    // Deserialise — XmlDeserializingModelFactory is wx-free (src-core only).
    XmlDeserializingModelFactory factory;
    Model* model = factory.Deserialize(modelXml, mm, false);
    if (!model) return false;

    // Decide projection plane before running InitRenderBufferNodes.
    // Read DisplayAs from the XML (GetDisplayAs() returns an enum, not a string).
    const std::string displayAs = modelXml.attribute("DisplayAs").as_string(
                                      modelXml.attribute("type").as_string(""));
    const bool topDown = (displayAs == "Tree 360");   // use X,Z for top-down

    std::vector<NodeBaseClassPtr> nodes;
    int bufW = 1, bufH = 1;
    model->InitRenderBufferNodes("Default", "2D", "", nodes, bufW, bufH, 0);

    if (bufW < 1 || bufH < 1 || nodes.empty()) {
        delete model;
        return false;
    }

    // Build (bufX, bufY) → projected (a, b) lookup.
    std::vector<float> sxArr(bufW * bufH, 0.f);
    std::vector<float> syArr(bufW * bufH, 0.f);
    std::vector<bool>  used(bufW * bufH, false);

    float minX =  1e9f, maxX = -1e9f;
    float minY =  1e9f, maxY = -1e9f;

    for (auto& node : nodes) {
        for (auto& coord : node->Coords) {
            int idx = coord.bufY * bufW + coord.bufX;
            if (idx < 0 || idx >= bufW * bufH) continue;
            // Choose projection axes.
            float a = coord.screenX;
            float b = topDown ? coord.screenZ   // top-down: X and Z
                               : coord.screenY;  // front:    X and Y
            sxArr[idx] = a;
            syArr[idx] = b;
            used[idx]  = true;
            if (a < minX) minX = a;
            if (a > maxX) maxX = a;
            if (b < minY) minY = b;
            if (b > maxY) maxY = b;
        }
    }

    const float rangeX = maxX - minX;
    const float rangeY = maxY - minY;

    // Normalize X and Y independently so the model always fills the preview
    // canvas regardless of aspect ratio — same visual behaviour as the xLights
    // layout-view panel.  When one axis has zero range (e.g. a 1-D strand where
    // all screenY = 0) place those nodes at the centre of that axis instead of
    // collapsing them to one edge.
    // Add 5 % padding on each side so nodes near the boundary aren't clipped.
    constexpr float PAD = 0.05f;
    constexpr float INN = 1.f - 2.f * PAD;

    // Build positions in wiring order so positions[i] == xLights node i+1.
    // Using the first Coord of each node as its representative screen position.
    // This makes node numbers in sub-model/face/state ranges map correctly.
    QList<QPointF> positions;
    positions.reserve((int)nodes.size());
    for (const auto& node : nodes) {
        float nx = 0.5f, ny = 0.5f;
        if (!node->Coords.empty()) {
            const auto& c = node->Coords[0];
            float a = c.screenX;
            float b = topDown ? c.screenZ : c.screenY;
            nx = rangeX > 0.f ? PAD + (a - minX) / rangeX * INN : 0.5f;
            if (rangeY > 0.f)
                ny = topDown ? PAD + (b - minY) / rangeY * INN
                             : PAD + (1.f - (b - minY) / rangeY) * INN;
        }
        positions.append({ double(nx), double(ny) });
    }

    out.bufferW       = bufW;
    out.bufferH       = bufH;
    out.nodeCount     = (int)nodes.size();   // physical nodes, not buffer cells
    out.nodePositions = positions;

    // Store the raw screen coordinate ranges so loadModels() can compute
    // globalPositions with the correct physical extent.  For shaped models
    // (trees, stars) this range differs significantly from bufferW/H.
    out.screenRangeX = double(rangeX);
    out.screenRangeY = double(rangeY);

    delete model;
    return true;
}

// ── Show file parser ──────────────────────────────────────────────────────────

void QtSequenceDoc::loadModels(const QString& showFilePath, QtSequenceInfo& info) {
    QFile f(showFilePath);
    if (!f.open(QIODevice::ReadOnly)) return;
    QByteArray data = f.readAll();

    pugi::xml_document doc;
    if (!doc.load_buffer(data.constData(), data.size())) return;

    // 2026.06+: root element is <xrgb>.
    // Legacy (pre-2026): root element is <xlights_rgbeffects>.
    pugi::xml_node root = doc.child("xrgb");
    if (!root) root = doc.child("xlights_rgbeffects");
    if (!root) return;

    pugi::xml_node modelsList  = root.child("models");
    pugi::xml_node groupsList  = root.child("modelGroups"); // sibling of <models>
    if (!modelsList) return;

    // Load outputs/controllers from xlights_networks.xml so models can resolve
    // controller-based start channels during Setup().  The file is always in the
    // same directory as xlights_rgbeffects.xml.
    const std::string showDir =
        QFileInfo(showFilePath).absoluteDir().absolutePath().toStdString();
    OutputManager outputManager;
    outputManager.Load(showDir);

    // One shared ModelManager for the whole show file.
    ModelManager mm(&outputManager, nullptr);

    for (auto m : modelsList.children("model")) {
        const char* nameStr = m.attribute("name").as_string("");
        if (!nameStr || nameStr[0] == '\0') continue;

        QtModelInfo mi;
        mi.name = QString::fromUtf8(nameStr);

        // 2026.06: model type is "DisplayAs"; legacy files used "type".
        mi.type = QString::fromUtf8(m.attribute("DisplayAs").as_string(
                      m.attribute("type").as_string("")));

        // Read a named attribute; fall back to the legacy parm attribute.
        // Mirrors xLights' ReadAttrWithParmFallback() behaviour.
        auto iattr = [&](const char* named, const char* parm, int def = 1) {
            int v = m.attribute(named).as_int(-1);
            if (v >= 0) return v;
            v = m.attribute(parm).as_int(-1);
            return v >= 0 ? v : def;
        };

        // Derive buffer dimensions per DisplayAs type.
        // These are the real DisplayAs strings from xlights_rgbeffects.xml.
        const QString& t = mi.type;

        // ── Horizontal matrix (most common) ──────────────────────────────
        // parm1 = NumStrings = rows, parm2 = NodesPerString = cols.
        // bufferW = cols (parm2), bufferH = rows (parm1).
        if (t == "Horiz Matrix" || t == "Matrix") {
            int nStr = iattr("NumStrings",     "parm1");   // rows
            int nPer = iattr("NodesPerString", "parm2");   // cols
            mi.parm1 = nStr; mi.parm2 = nPer;
            mi.bufferW = qMax(1, nPer);   // cols = width
            mi.bufferH = qMax(1, nStr);   // rows = height

        // ── Vertical matrix ───────────────────────────────────────────────
        // parm1 = NumStrings = cols, parm2 = NodesPerString = rows.
        } else if (t == "Vert Matrix" || t == "Spinner") {
            int nStr = iattr("NumStrings",     "parm1");   // cols
            int nPer = iattr("NodesPerString", "parm2");   // rows
            mi.parm1 = nStr; mi.parm2 = nPer;
            mi.bufferW = qMax(1, nStr);   // cols = width
            mi.bufferH = qMax(1, nPer);   // rows = height

        // ── Tree variants ─────────────────────────────────────────────────
        // Multi-string tree (parm1 > 1): parm1=strands, parm2=nodes/strand.
        // Single-string tree (parm1 == 1): parm3 is the visual strand count;
        //   nodes are wrapped into parm3 arms of parm2/parm3 nodes each.
        } else if (t.startsWith("Tree")) {
            int nStr = iattr("NumStrings",     "parm1");
            int nPer = iattr("NodesPerString", "parm2");
            int lpn  = iattr("LightsPerNode",  "parm3");
            if (nStr > 1) {
                // Standard multi-string tree: parm3 = lights/node (ignored here)
                mi.parm1 = nStr;
                mi.parm2 = nPer;
                mi.bufferW = qMax(1, nStr);
                mi.bufferH = qMax(1, nPer);
            } else {
                // Single-string tree: parm3 = visual strand count, e.g. parm3=8 → 8 arms
                int visualStrands = qMax(1, lpn);
                int nodesPerArm   = qMax(1, nPer / visualStrands);
                mi.parm1 = visualStrands;
                mi.parm2 = nodesPerArm;
                mi.bufferW = visualStrands;
                mi.bufferH = nodesPerArm;
            }

        // ── Custom model ──────────────────────────────────────────────────
        } else if (t == "Custom") {
            mi.bufferW = qMax(1, iattr("CustomWidth",  "parm1"));
            mi.bufferH = qMax(1, iattr("CustomHeight", "parm2"));
            mi.parm1 = mi.bufferW; mi.parm2 = mi.bufferH;

        // ── Cube ──────────────────────────────────────────────────────────
        } else if (t == "Cube") {
            mi.bufferW = qMax(1, iattr("CubeWidth",  "parm1"));
            mi.bufferH = qMax(1, iattr("CubeHeight", "parm2"));
            mi.parm1 = mi.bufferW; mi.parm2 = mi.bufferH;

        // ── Arches (multiple) ─────────────────────────────────────────────
        } else if (t == "Arches") {
            int nArch = iattr("NumArches",    "parm1");
            int nPer  = iattr("NodesPerArch", "parm2");
            mi.parm1 = nArch; mi.parm2 = nPer;
            mi.bufferW = qMax(1, nArch * nPer);
            mi.bufferH = 1;

        // ── Candy Canes ───────────────────────────────────────────────────
        } else if (t == "Candy Canes") {
            int nCane = iattr("NumCanes",     "parm1");
            int nPer  = iattr("NodesPerCane", "parm2");
            mi.parm1 = nCane; mi.parm2 = nPer;
            mi.bufferW = qMax(1, nCane * nPer);
            mi.bufferH = 1;

        // ── Circle ────────────────────────────────────────────────────────
        } else if (t == "Circle") {
            int nStr = iattr("NumStrings",     "parm1");
            int nPer = iattr("NodesPerString", "parm2");
            mi.parm1 = nStr; mi.parm2 = nPer;
            mi.bufferW = qMax(1, nStr);
            mi.bufferH = qMax(1, nPer);

        // ── Star ──────────────────────────────────────────────────────────
        // parm3 = StarPoints (NOT LightsPerNode) in xLights Star model.
        // LayerSizes="40,60,80" → concentric rings (innermost first).
        } else if (t == "Star") {
            int pts = m.attribute("StarPoints").as_int(
                          m.attribute("parm3").as_int(5));   // parm3, not parm1
            int p1  = iattr("NumStrings",     "parm1");      // usually 1
            int p2  = iattr("NodesPerString", "parm2");      // nodes per string
            mi.parm1 = pts; mi.parm2 = p2;

            // Parse LayerSizes (e.g. "40,60,80") — present on PixelTreeStar etc.
            const char* lsStr = m.attribute("LayerSizes").as_string("");
            if (lsStr && lsStr[0] != '\0') {
                int total = 0;
                for (const QString& part :
                         QString::fromUtf8(lsStr).split(',', Qt::SkipEmptyParts)) {
                    int n = part.trimmed().toInt();
                    if (n > 0) { mi.layerSizes.append(n); total += n; }
                }
                mi.bufferW = qMax(1, total);
            } else {
                mi.bufferW = qMax(1, p1 * p2);
            }
            mi.bufferH = 1;

        // ── 1-D: Single Line, Poly Line, Arch, Icicles, Window Frame, etc.
        } else {
            int p1 = iattr("NumStrings",     "parm1");
            int p2 = iattr("NodesPerString", "parm2");
            int p3 = iattr("LightsPerNode",  "parm3");
            mi.parm1 = p1; mi.parm2 = p2;
            mi.bufferW = qMax(1, p1 * p2 * p3);
            mi.bufferH = 1;
        }

        mi.startChannel = m.attribute("StartChannel").as_int(0);

        // World position and scale for the house preview layout.
        mi.worldPosX = m.attribute("WorldPosX").as_double(0.0);
        mi.worldPosY = m.attribute("WorldPosY").as_double(0.0);
        mi.scaleX    = m.attribute("ScaleX").as_double(1.0);
        mi.scaleY    = m.attribute("ScaleY").as_double(1.0);

        // Try src-core first for all model types.  coreNodePositions() returns
        // exact screenX/Y positions from InitRenderBufferNodes and computes
        // correct world-space globalPositions (worldPos + screenXY × scale).
        // The math fallback is used only when core fails.
        bool coreOk = false;
        try {
            coreOk = coreNodePositions(m, mm, mi);
        } catch (...) {
            coreOk = false;
        }

        if (!coreOk) {
            // Math fallback: compute node positions from buffer dimensions + model type.
            mi.nodeCount     = mi.bufferW * mi.bufferH;
            mi.nodePositions = computeNodePositions(t, mi.parm1, mi.parm2,
                                                    mi.bufferW, mi.bufferH,
                                                    mi.layerSizes);
        }

        // Compute global layout positions from nodePositions + world transform.
        // nodePositions are in [PAD, 1-PAD]×[PAD, 1-PAD] (PAD=0.05, span=INN=0.9).
        // Physical extent: use the core's screenX/Y range when available (accurate
        // for all model types including trees and stars); fall back to bufW/H×scale
        // (the old behaviour, which is correct for matrices, arches, custom, etc.).
        {
            constexpr double INN = 0.9;   // same constant as in coreNodePositions
            const double w = (coreOk && mi.screenRangeX > 0)
                ? mi.screenRangeX * mi.scaleX / INN
                : mi.bufferW * mi.scaleX;
            const double h = (coreOk && mi.screenRangeY > 0)
                ? mi.screenRangeY * mi.scaleY / INN
                : mi.bufferH * mi.scaleY;
            mi.globalPositions.reserve(mi.nodePositions.size());
            for (const QPointF& lp : mi.nodePositions) {
                mi.globalPositions.append({
                    mi.worldPosX + (lp.x() - 0.5) * w,
                    mi.worldPosY + (lp.y() - 0.5) * h
                });
            }
        }

        // ── Controller wiring ─────────────────────────────────────────────────
        mi.controllerName = QString::fromUtf8(m.attribute("Controller").as_string(""));
        if (auto cc = m.child("ControllerConnection"))
            mi.controllerPort = cc.attribute("Port").as_int(0);

        // ── Sub-models ────────────────────────────────────────────────────────
        for (auto smNode : m.children("subModel")) {
            QtSubModelInfo sm;
            sm.name        = QString::fromUtf8(smNode.attribute("name").as_string(""));
            sm.layout      = QString::fromUtf8(smNode.attribute("layout").as_string("horizontal"));
            sm.type        = QString::fromUtf8(smNode.attribute("type").as_string("ranges"));
            sm.bufferStyle = QString::fromUtf8(smNode.attribute("bufferstyle").as_string("Default"));
            if (sm.type == "ranges") {
                // Ranges are stored as line0, line1, line2, … attributes.
                for (int li = 0; ; ++li) {
                    auto lineAttr = smNode.attribute(("line" + std::to_string(li)).c_str());
                    if (!lineAttr) break;
                    sm.ranges.append(QString::fromUtf8(lineAttr.as_string("")));
                }
            } else {
                // Subbuffer expression stored in a single "subBuffer" attribute.
                const char* sb = smNode.attribute("subBuffer").as_string("");
                if (sb && sb[0]) sm.ranges.append(QString::fromUtf8(sb));
            }
            if (!sm.name.isEmpty())
                mi.subModels.append(sm);
        }

        // ── Faces ─────────────────────────────────────────────────────────────
        for (auto fNode : m.children("faceInfo")) {
            QtFaceInfo fi;
            fi.name = QString::fromUtf8(fNode.attribute("Name").as_string(""));
            fi.type = QString::fromUtf8(fNode.attribute("Type").as_string("NodeRange"));
            const char* fcc = fNode.attribute("CustomColors").as_string("");
            fi.forceColor = (fcc && fcc[0] != '\0');
            for (auto attr : fNode.attributes())
                fi.attrs[QString::fromUtf8(attr.name())] =
                    QString::fromUtf8(attr.value());
            if (!fi.name.isEmpty())
                mi.faces.append(fi);
        }

        // ── States ────────────────────────────────────────────────────────────
        // Format: s1="node-range"  s1-Color="color"  s1-Name="label"
        // Up to s200 (or however many are defined).
        for (auto sNode : m.children("stateInfo")) {
            QtStateInfo si;
            si.name = QString::fromUtf8(sNode.attribute("Name").as_string(""));
            si.type = QString::fromUtf8(sNode.attribute("Type").as_string("NodeRange"));
            const char* scc = sNode.attribute("CustomColors").as_string("");
            si.forceColor = (scc && scc[0] != '\0');

            // First pass: collect all numeric keys from sN attributes.
            QMap<int, QtStateEntry> entryMap;
            for (auto attr : sNode.attributes()) {
                const QString k = QString::fromUtf8(attr.name());
                if (k == "Name" || k == "Type" || k == "CustomColors") continue;
                if (!k.startsWith('s') || k.size() < 2) continue;

                // Split on first '-': "s1" → base="s1", suffix=""
                //                     "s1-Color" → base="s1", suffix="-Color"
                //                     "s1-Name"  → base="s1", suffix="-Name"
                const int dash = k.indexOf('-');
                const QString base   = (dash < 0) ? k : k.left(dash);
                const QString suffix = (dash < 0) ? QString() : k.mid(dash);

                // base must be 's' followed by digits only
                const QString numStr = base.mid(1);
                bool ok = false;
                const int n = numStr.toInt(&ok);
                if (!ok || n < 1) continue;

                const QString val = QString::fromUtf8(attr.value());
                if (suffix.isEmpty())
                    entryMap[n].nodes = val;
                else if (suffix.compare("-Color", Qt::CaseInsensitive) == 0)
                    entryMap[n].color = val;
                else if (suffix.compare("-Name", Qt::CaseInsensitive) == 0)
                    entryMap[n].name = val;
            }

            for (auto it = entryMap.constBegin(); it != entryMap.constEnd(); ++it)
                si.entries.append(it.value());

            if (!si.name.isEmpty())
                mi.states.append(si);
        }

        info.models[mi.name] = mi;
    }

    // ── Model groups ──────────────────────────────────────────────────────────
    // <modelGroup> elements in xlights_rgbeffects.xml define named collections
    // of models.  Groups appear in sequence ElementEffects as type="model", so
    // the sequence parser already loads their effects; we just need the membership
    // and buffer dimensions so we can render and distribute pixels correctly.
    for (auto g : groupsList.children("modelGroup")) {
        const char* nameStr = g.attribute("name").as_string("");
        if (!nameStr || nameStr[0] == '\0') continue;

        QtModelGroupInfo gi;
        gi.name   = QString::fromUtf8(nameStr);
        gi.layout = QString::fromUtf8(g.attribute("layout").as_string("minimalGrid"));

        const QString modelsAttr = QString::fromUtf8(g.attribute("models").as_string(""));
        for (const QString& mn : modelsAttr.split(',', Qt::SkipEmptyParts))
            gi.modelNames.append(mn.trimmed());

        if (gi.modelNames.isEmpty()) continue;

        // Compute the bounding box from actual globalPositions of each member model.
        gi.minX = gi.minY =  1e9;
        gi.maxX = gi.maxY = -1e9;
        for (const QString& mn : gi.modelNames) {
            auto it = info.models.find(mn);
            if (it == info.models.end()) continue;
            const QtModelInfo& m = *it;
            if (!m.globalPositions.isEmpty()) {
                for (const QPointF& gp : m.globalPositions) {
                    if (gp.x() < gi.minX) gi.minX = gp.x();
                    if (gp.x() > gi.maxX) gi.maxX = gp.x();
                    if (gp.y() < gi.minY) gi.minY = gp.y();
                    if (gp.y() > gi.maxY) gi.maxY = gp.y();
                }
            } else {
                // No node positions — fall back to worldPos ± bufferW×scaleX estimate.
                const double hw = m.bufferW * m.scaleX * 0.5;
                const double hh = m.bufferH * m.scaleY * 0.5;
                gi.minX = qMin(gi.minX, m.worldPosX - hw);
                gi.maxX = qMax(gi.maxX, m.worldPosX + hw);
                gi.minY = qMin(gi.minY, m.worldPosY - hh);
                gi.maxY = qMax(gi.maxY, m.worldPosY + hh);
            }
        }
        if (gi.maxX <= gi.minX && gi.maxY <= gi.minY) continue;
        if (gi.maxX <= gi.minX) { gi.minX -= 0.5; gi.maxX += 0.5; }
        if (gi.maxY <= gi.minY) { gi.minY -= 0.5; gi.maxY += 0.5; }

        // Scale the bounding box to a buffer capped at 256 pixels in the
        // largest dimension, keeping the aspect ratio.
        const double rangeX = gi.maxX - gi.minX;
        const double rangeY = gi.maxY - gi.minY;
        const double maxPx  = 256.0;
        if (rangeX >= rangeY) {
            gi.bufferW = int(maxPx);
            gi.bufferH = qMax(1, int(maxPx * rangeY / rangeX));
        } else {
            gi.bufferH = int(maxPx);
            gi.bufferW = qMax(1, int(maxPx * rangeX / rangeY));
        }

        info.groups[gi.name] = gi;
    }
    spdlog::info("QtSequenceDoc: {} groups loaded from show file", info.groups.size());

    // ── Controller records ────────────────────────────────────────────────────
    for (const auto* c : outputManager.GetControllers()) {
        if (!c) continue;
        QtControllerInfo ci;
        ci.name         = QString::fromStdString(c->GetName());
        ci.type         = QString::fromStdString(c->GetType());
        ci.ip           = QString::fromStdString(c->GetIP());
        ci.startChannel = c->GetStartChannel();
        ci.channelCount = c->GetChannels();
        ci.protocol     = QString::fromStdString(c->GetProtocol());
        ci.universeOrBaud = 0;
        // Port capabilities from ControllerCaps if this is a known smart controller.
        if (const ControllerCaps* caps = ControllerCaps::GetControllerConfig(c)) {
            ci.pixelPortCount    = caps->GetMaxPixelPort();
            ci.serialPortCount   = caps->GetMaxSerialPort();
            ci.pixelPortChannels = caps->GetMaxPixelPortChannels();
        }
        info.controllers.append(ci);
    }
}
