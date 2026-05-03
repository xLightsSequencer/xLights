// /EHa is set via set_source_files_properties in CMakeLists.txt so that
// catch(...) below also catches Windows SEH exceptions (access violations).
// This lets us fall back to the software renderer if src-core initialisation
// crashes rather than taking down the whole process.

#include "QtRenderBridge.h"
#include "QtModelStub.h"
#include "QtRenderContext.h"

#include "../../src-core/effects/RenderableEffect.h"
#include "../../src-core/effects/EffectManager.h"
#include "../../src-core/models/ModelManager.h"
#include "../../src-core/models/Model.h"
#include "../../src-core/render/Effect.h"
#include "../../src-core/render/EffectLayer.h"
#include "../../src-core/render/Element.h"
#include "../../src-core/render/RenderBuffer.h"
#include "../../src-core/render/ColorCurve.h"
#include "../../src-core/utils/UtilClasses.h"
#include "../../src-core/utils/Color.h"

#include <pugixml.hpp>
#include <spdlog/spdlog.h>

// ── File-scope statics (cleaned up on first exception) ───────────────────────
// Kept outside renderCore() so unique_ptr destructors run properly if we reset
// them from inside a catch block (local statics can't be safely reset).

static std::unique_ptr<QtRenderContext> s_ctx;
static std::unique_ptr<ModelManager>    s_mm;
static bool                             s_initFailed = false;

// ── Constructor ───────────────────────────────────────────────────────────────

QtRenderBridge::QtRenderBridge(QObject* parent) : QObject(parent) {
    _debounce.setSingleShot(true);
    _debounce.setInterval(40);
    connect(&_debounce, &QTimer::timeout, this, [this]() {
        emit frameReady(renderCore(_pending));
    });
}

void QtRenderBridge::setMetadataDir(const QString& dir) {
    _metadataDir = dir;
}

void QtRenderBridge::setShowFolder(const QString& showFolder) {
    _showFolder = showFolder;
    if (!s_ctx || showFolder.isEmpty()) return;

    // Reload outputs.
    s_ctx->outputManager().Load(showFolder.toStdString());

    // Load all models (including groups) from xlights_rgbeffects.xml so that
    // renderCore() can use the real model topology (InitRenderBufferNodes).
    // This matches what the wx version does with ModelManager::LoadModels().
    try {
        const std::string rgbFile = showFolder.toStdString() + "/xlights_rgbeffects.xml";
        pugi::xml_document doc;
        if (doc.load_file(rgbFile.c_str())) {
            auto root = doc.first_child();
            auto modelsNode = root.child("models");
            if (modelsNode && s_mm)
                s_mm->LoadModels(modelsNode, 1920, 1080);
        }
    } catch (...) {
        spdlog::warn("QtRenderBridge: failed to load show models");
    }
}

void QtRenderBridge::request(const QtEffectRenderer::Request& req) {
    _pending = req;
    _debounce.start();
}

QtEffectRenderer::Result QtRenderBridge::renderNow(const QtEffectRenderer::Request& req) {
    return renderCore(req);
}

// ── src-core render path ──────────────────────────────────────────────────────

QtEffectRenderer::Result QtRenderBridge::renderCore(const QtEffectRenderer::Request& req) {

    // Permanently disabled if a previous attempt crashed.
    if (s_initFailed)
        return QtEffectRenderer::render(req);

    // ── One-time initialisation ────────────────────────────────────────────
    if (!s_ctx) {
        try {
            s_ctx = std::make_unique<QtRenderContext>(_metadataDir.toStdString());
            // Load outputs/controllers from the show folder so models can
            // resolve controller names.  Safe even if the file is absent.
            if (!_showFolder.isEmpty())
                s_ctx->outputManager().Load(_showFolder.toStdString());
            s_mm  = std::make_unique<ModelManager>(&s_ctx->outputManager(), s_ctx.get());
            spdlog::info("QtRenderBridge: src-core ready ({} effects)",
                         s_ctx->GetEffectManager().size());
        } catch (...) {
            spdlog::error("QtRenderBridge: src-core init failed — "
                          "using software renderer for all effects");
            s_ctx.reset();
            s_mm.reset();
            s_initFailed = true;
            return QtEffectRenderer::render(req);
        }
    }

    // ── Look up the effect ─────────────────────────────────────────────────
    RenderableEffect* effect = nullptr;
    try {
        effect = s_ctx->GetEffectManager().GetEffect(req.effectName.toStdString());
    } catch (...) {
        effect = nullptr;
    }

    if (!effect) {
        spdlog::debug("QtRenderBridge: '{}' not in EffectManager — software fallback",
                      req.effectName.toStdString());
        return QtEffectRenderer::render(req);
    }

    // ── Render via src-core ────────────────────────────────────────────────
    try {
        // ── 1. Build SettingsMap first so buffer params can be read ──────
        // Strip the 2-char prefix (E_, T_, B_, C_) — effects and RenderBuffer
        // both read keys without prefix, matching RenderEngine::CopySettingsMap.
        SettingsMap settings;
        if (!req.rawSettings.isEmpty()) {
            SettingsMap raw;
            raw.Parse(nullptr, req.rawSettings.toStdString(),
                      req.effectName.toStdString());
            for (auto it = raw.begin(); it != raw.end(); ++it) {
                const std::string& k = it->first;
                const std::string stripped =
                    (k.size() > 2 && k[1] == '_') ? k.substr(2) : k;
                settings[stripped] = it->second;
                settings[k]        = it->second;   // keep prefixed form too
            }
        } else {
            for (auto it = req.settings.begin(); it != req.settings.end(); ++it) {
                const std::string k = it.key().toStdString();
                const std::string v = it.value().toString().toStdString();
                settings[k]                       = v;
                settings["E_SLIDER_"        + k]  = v;
                settings["E_CHECKBOX_"      + k]  = v;
                settings["E_CHOICE_"        + k]  = v;
                settings["E_TEXTCTRL_"      + k]  = v;
                settings["E_FILEPICKERCTRL_"+ k]  = v;
            }
        }

        // ── 2. Buffer parameters from B_* settings ───────────────────────
        const std::string bufStyle     = settings.Get("CHOICE_BufferStyle",    "Default");
        const std::string bufTransform = settings.Get("CHOICE_BufferTransform","None");

        // ── 3. Look up the real model from the show file ─────────────────
        // Matches wx behaviour: use model->InitRenderBufferNodes() to get the
        // correct buffer dimensions AND node topology for all model types,
        // including group models (minimalGrid layout) and shaped models (trees,
        // stars, custom).  Falls back to the stub/caller-supplied dims when the
        // model is not found.
        int bufW = req.bufferW;
        int bufH = req.bufferH;
        std::vector<NodeBaseClassPtr> realNodes;

        if (!req.modelName.isEmpty() && s_mm) {
            Model* realModel = nullptr;
            try { realModel = s_mm->GetModel(req.modelName.toStdString()); }
            catch (...) { realModel = nullptr; }

            if (realModel) {
                try {
                    realModel->InitRenderBufferNodes(
                        bufStyle, "2D", bufTransform,
                        realNodes, bufW, bufH, 0);
                } catch (...) {
                    realNodes.clear();
                    bufW = req.bufferW;
                    bufH = req.bufferH;
                }
            }
        }

        // "Single Line" fallback when InitRenderBufferNodes wasn't called.
        if (realNodes.empty() && bufStyle == "Single Line") {
            bufW = req.bufferW * req.bufferH;
            bufH = 1;
        }

        // ── 4. RenderBuffer ──────────────────────────────────────────────
        QtModelStub model(*s_mm, req.effectName.toStdString(), bufW, bufH);
        RenderBuffer buffer(nullptr, nullptr, &model);
        buffer.InitBuffer(bufH, bufW, bufTransform);

        // Give the buffer the real node topology so effects that walk
        // buffer.Nodes (Spiral, SingleStrand, per-node colour, etc.) behave
        // correctly.  For groups this maps every member model's nodes to their
        // correct minimalGrid buffer positions — same as the wx version.
        if (!realNodes.empty())
            buffer.SetNodes(std::move(realNodes));

        // ── 4. Palette ───────────────────────────────────────────────────
        xlColorVector      pal;
        xlColorCurveVector curves;
        if (!req.rawPalette.isEmpty()) {
            SettingsMap palMap;
            palMap.Parse(nullptr, req.rawPalette.toStdString(), "");
            for (int i = 1; i <= 8; ++i) {
                std::string cbKey = "C_CHECKBOX_Palette" + std::to_string(i);
                std::string btKey = "C_BUTTON_Palette"   + std::to_string(i);
                if (palMap.GetBool(cbKey)) {
                    xlColor c(palMap.Get(btKey, ""));
                    pal.push_back(c);
                    curves.push_back(ColorCurve());
                }
            }
            if (!pal.empty()) buffer.SetPalette(pal, curves);
        } else {
            pal.reserve(req.palette.size());
            for (const QColor& c : req.palette)
                pal.emplace_back(uint8_t(c.red()), uint8_t(c.green()), uint8_t(c.blue()));
            if (!pal.empty()) {
                curves.resize(pal.size());
                buffer.SetPalette(pal, curves);
            }
        }

        // ── 5. Timing ────────────────────────────────────────────────────
        // curPeriod / curEffStartPer / curEffEndPer are all in "frames" units.
        // We use a virtual 0..200 frame range so progress maps linearly.
        // curEffEndPer must be > curEffStartPer so GetEffectTimeIntervalPosition()
        // doesn't short-circuit to 0 (its division-by-zero guard).
        buffer.curEffStartPer = 0;
        buffer.curEffEndPer   = 200;
        buffer.curPeriod      = int(req.progress * 200.0);
        buffer.frameTimeInMs  = 50;

        // ── 6. Minimal Effect* stub ───────────────────────────────────────
        // Prevents crashes in effects that walk effect→layer→element→seqElements.
        ModelElement stubElement(&s_ctx->GetSequenceElements(),
                                 req.effectName.toStdString(), false);
        EffectLayer   stubLayer(&stubElement);
        // Map our 0..200 virtual-frame range to ms so GetStartTimeMS/GetEndTimeMS
        // return meaningful values if effects query them.
        Effect        stubEffect(&s_ctx->GetEffectManager(), &stubLayer,
                                 /*id=*/0,
                                 req.effectName.toStdString(),
                                 /*settings=*/"", /*palette=*/"",
                                 /*startMs=*/0,   /*endMs=*/10000,
                                 EFFECT_NOT_SELECTED, /*protected=*/false,
                                 /*importing=*/true);

        effect->Render(&stubEffect, settings, buffer);

        // ── 7. Extract pixels ─────────────────────────────────────────────
        const int      n  = bufW * bufH;
        const xlColor* px = buffer.GetPixels();
        QtEffectRenderer::Result result;
        result.w = bufW;
        result.h = bufH;
        result.pixels.reserve(n);
        for (int i = 0; i < n; ++i)
            result.pixels.append(QColor(px[i].red, px[i].green, px[i].blue));
        return result;

    } catch (...) {
        spdlog::warn("QtRenderBridge: exception rendering '{}' — software fallback",
                     req.effectName.toStdString());
        return QtEffectRenderer::render(req);
    }
}
