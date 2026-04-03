/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <map>
#include <memory>

#include "xLightsMain.h"
#include "SequenceFile.h"
#include "effects/RenderableEffect.h"
#include "IRenderJobCallbacks.h"
#include "IRenderJobStatus.h"
#include "IRenderProgressSink.h"
#include "RenderProgressInfo.h"
#include "RenderUtils.h"
#include "models/ModelGroup.h"
#include "UtilFunctions.h"
#include "PixelBuffer.h"
#include "Parallel.h"
#include "utils/ExternalHooks.h"
#include "GPURenderUtils.h"

#include <log.h>
// END_OF_RENDER_FRAME is defined in RenderProgressInfo.h


class EffectLayerInfo {
public:
    EffectLayerInfo(): element(nullptr)
    {
        numLayers = 0;
        buffer.reset(nullptr);
        strand = -1;
    }

    EffectLayerInfo(int l) : element(nullptr) {
        resize(l);
        buffer.reset(nullptr);
        strand = -1;
    }

    void resize(int l) {
        numLayers = l;
        currentEffects.resize(l);
        currentEffectIdxs.resize(l);
        settingsMaps.resize(l);
        effectStates.resize(l);
        validLayers.resize(l + 1); //extra one for the blending layer
    }

    int numLayers;
    int strand;
    int submodel = -1;
    Element *element;
    PixelBufferClassPtr buffer;
    std::vector<Effect*> currentEffects;
    std::vector<int> currentEffectIdxs;
    std::vector<SettingsMap> settingsMaps;
    std::vector<bool> effectStates;
    std::vector<bool> validLayers;
};

class RenderEvent {
public:
    RenderEvent() : mutex(), signal() {}
    std::mutex mutex;
    std::condition_variable signal;

    int layer;
    int period;
    Effect *effect;
    SettingsMap *settingsMap;
    PixelBufferClass *buffer;
    bool *ResetEffectState;
    int returnVal{ -1 };
    bool suppress{ false };
};

class NextRenderer {
public:

    NextRenderer() : nextLock(), nextSignal(), previousFrameDone(-1) {
    }

    virtual ~NextRenderer() {}

    bool addNext(NextRenderer *n) {
        for (const auto& i : next) {
            if (i == n) return false;
        }
        next.push_back(n);
        return true;
    }

    bool HasNext() const {
        return !next.empty();
    }

    void FrameDone(int frame) {
        for (const auto& i : next) {
            i->setPreviousFrameDone(frame);
        }
    }

    virtual void setPreviousFrameDone(int i) {
        previousFrameDone = i;
        nextSignal.notify_all();
    }

    int waitForFrame(int frame) {
        if (frame > previousFrameDone) {
            std::unique_lock<std::mutex> lock(nextLock);
            while (frame > previousFrameDone) {
                nextSignal.wait_for(lock, std::chrono::milliseconds(10));
            }
        }
        return previousFrameDone;
    }

    bool checkIfDone(int frame, int timeout = 5) {
        return previousFrameDone >= frame;
    }

    int GetPreviousFrameDone() const {
        return previousFrameDone;
    }

protected:
    std::mutex nextLock;
    std::condition_variable nextSignal;
    std::atomic_int previousFrameDone;
private:
    std::vector<NextRenderer *> next;
};

class AggregatorRenderer: public NextRenderer {
public:

    AggregatorRenderer(int numFrames) : NextRenderer(), data(numFrames + 20), finalFrame(numFrames + 19) {
        for (int x = 0; x < (numFrames + 20); ++x) {
            data[x] = 0;
        }
        max = 0;
    }

    virtual ~AggregatorRenderer() {
    }

    void incNumAggregated() {
        ++max;
    }

    int getNumAggregated() const
    {
        return max;
    }

    virtual void setPreviousFrameDone(int frame) {
        if (max <= 1) {
            previousFrameDone = frame;
            FrameDone(frame);
            return;
        }
        int idx = frame;
        if (idx == END_OF_RENDER_FRAME) {
            idx = finalFrame;
        }
        int i = data[idx].fetch_add(1);
        if (i == (max - 1)) {
            previousFrameDone = frame;
            FrameDone(previousFrameDone);
        }
    }

private:
    std::vector<std::atomic_int> data;
    int max;
    const int finalFrame;
};

void RenderProgressInfo::CleanupJobs() {
    for (int i = 0; i < numRows; ++i) {
        delete jobs[i];
        delete aggregators[i];
    }
    delete[] jobs;
    jobs = nullptr;
    delete[] aggregators;
    aggregators = nullptr;
    delete progressSink;
    progressSink = nullptr;
}

class SNPair {
public:
    SNPair(int s, int n) : strand(s), node(n) {}

    SNPair(const SNPair &p) : strand(p.strand), node(p.node) {}

    bool operator>(const SNPair &p) const {
        return strand > p.strand
               || (strand == p.strand && node > p.node);
    }

    bool operator<(const SNPair &p) const {
        return strand < p.strand
               || (strand == p.strand && node < p.node);
    }

    bool operator==(const SNPair &p) const {
        return strand == p.strand && node == p.node;
    }

    const int strand;
    const int node;
};


class RenderJob: public Job, public NextRenderer, public IRenderJobStatus {
public:
    RenderJob(ModelElement *row, SequenceData &data, RenderContext *ctx, IRenderJobCallbacks *callbacks)
        : Job(), NextRenderer(), rowToRender(row), _ctx(ctx), _callbacks(callbacks), seqData(&data),
            supportsModelBlending(false), statusMap(nullptr), m_logger(spdlog::get("render")),
            currentFrame(0), abort(false)
    {
        name = "";
        if (row != nullptr) {
            name = row->GetModelName();
            mainBuffer = new PixelBufferClass(_ctx);
            numLayers = rowToRender->GetEffectLayerCount();

            if (_ctx->InitPixelBuffer(name, *mainBuffer, numLayers)) {
                const Model *model = mainBuffer->GetModel();
                if (DisplayAsType::ModelGroup == model->GetDisplayAs()) {
                    //for (int l = 0; l < numLayers; ++l) {
                    for (int l = numLayers - 1; l >= 0; --l) {
                        EffectLayer *layer = row->GetEffectLayer(l);
                        bool perModelEffects = false;
                        bool perModelEffectsDeep = false;
                        for (int e = 0; e < layer->GetEffectCount() && !perModelEffects; ++e) {
                            static const std::string CHOICE_BufferStyle("B_CHOICE_BufferStyle");
                            static const std::string DEFAULT("Default");
                            static const std::string PER_MODEL("Per Model");
                            static const std::string DEEP("Deep");
                            const std::string& bt = layer->GetEffect(e)->GetSettings().Get(CHOICE_BufferStyle, DEFAULT);
                            if (bt.compare(0, 9, PER_MODEL) == 0) {
                                if (bt.compare(bt.length() - 4, 4, DEEP) == 0) {
                                    perModelEffectsDeep = true;
                                } else {
                                    perModelEffects = true;
                                }
                            }
                        }
                        const ModelGroup* grp = dynamic_cast<const ModelGroup*>(model);
                        if (perModelEffectsDeep) {
                            mainBuffer->InitPerModelBuffersDeep(*grp, l, data.FrameTime());
                        }
                        if (perModelEffects) {
                            mainBuffer->InitPerModelBuffers(*grp, l, data.FrameTime());
                        }
                    }
                }
                for (int x = 0; x < row->GetSubModelAndStrandCount(); ++x) {
                    SubModelElement *se = row->GetSubModel(x);
                    if (se->HasEffects()) {
                        if (se->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                            StrandElement *ste = (StrandElement*)se;
                            if (ste->GetStrand() < model->GetNumStrands()) {
                                subModelInfos.push_back(new EffectLayerInfo(se->GetEffectLayerCount() + 1));
                                subModelInfos.back()->element = se;
                                subModelInfos.back()->buffer.reset(new PixelBufferClass(_ctx));
                                subModelInfos.back()->strand = ste->GetStrand();
                                subModelInfos.back()->submodel = subModelInfos.size() -1;
                                subModelInfos.back()->buffer->InitStrandBuffer(*model, ste->GetStrand(), data.FrameTime(), se->GetEffectLayerCount());
                            }
                        } else {
                            Model *subModel = model->GetSubModel(se->GetName());
                            if (subModel != nullptr) {
                                subModelInfos.push_back(new EffectLayerInfo(se->GetEffectLayerCount() + 1));
                                subModelInfos.back()->element = se;
                                subModelInfos.back()->submodel = subModelInfos.size() -1;
                                subModelInfos.back()->buffer.reset(new PixelBufferClass(_ctx));
                                subModelInfos.back()->buffer->InitBuffer(*subModel, se->GetEffectLayerCount() + 1, data.FrameTime());
                            }
                        }
                    }
                    if (se->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                        StrandElement *ste = (StrandElement*)se;
                        if (ste->GetStrand() < model->GetNumStrands()) {
                            for (int n = 0; n < ste->GetNodeLayerCount(); ++n) {
                                if (n < model->GetStrandLength(ste->GetStrand())) {
                                    EffectLayer *nl = ste->GetNodeLayer(n);
                                    if (nl -> GetEffectCount() > 0) {
                                        nodeBuffers[SNPair(ste->GetStrand(), n)].reset(new PixelBufferClass(_ctx));
                                        nodeBuffers[SNPair(ste->GetStrand(), n)]->InitNodeBuffer(*model, ste->GetStrand(), n, data.FrameTime());
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                delete mainBuffer;
                mainBuffer = nullptr;
            }
        } else {
            mainBuffer = nullptr;
        }
        startFrame = 0;
        renderEvent.buffer = mainBuffer;
    }

    virtual ~RenderJob() {
        if (mainBuffer != nullptr) {
            delete mainBuffer;
        }
        for (const auto& a : subModelInfos) {
            EffectLayerInfo *info = a;
            delete info;
        }
    }

    void SetProgressCallback(std::function<void(int, const std::string&)> cb) override { progressCallback = std::move(cb); }
    void UpdateProgress(int value, const std::string& tooltip = {}) override {
        if (progressCallback) progressCallback(value, tooltip);
    }
    int GetCurrentFrame() const override { return currentFrame;}
    int GetEndFrame() const override { return endFrame;}
    int GetStartFrame() const override { return startFrame;}

    const std::string GetName() const override {
        return name;
    }

    virtual bool DeleteWhenComplete() override {
        return false;
    }

    void LogToLogger(int logLevel) {
        // these can only be set at start time
        static bool debug = m_logger->level() == spdlog::level::debug;
        static bool info = m_logger->level() == spdlog::level::info;

        if ((debug && logLevel == spdlog::level::debug) ||
            (info && logLevel == spdlog::level::info))
        {
            m_logger->log((spdlog::level::level_enum)logLevel, GetStatusString());
        }
    }

    void SetGenericStatus(const std::string &msg, int frame, bool debugLog = false, bool includeStatusMap = false) {
        statusType = includeStatusMap ? 8 : 4;
        statusMsg = msg;
        statusFrame = frame;
        LogToLogger(debugLog ? spdlog::level::debug : spdlog::level::info);
    }

    void SetGenericStatus(const std::string &msg, int frame, int layer, bool debugLog = false, bool includeStatusMap = false) {
        statusType = includeStatusMap ? 9 : 5;
        statusMsg = msg;
        statusFrame = frame;
        statusLayer = layer;
        LogToLogger(debugLog ? spdlog::level::debug : spdlog::level::info);
    }

    std::string PrintStatusMap() const {
        if (statusMap == nullptr) return "";
        return statusMap->AsString();
    }

    void SetRenderingStatus(int frame, SettingsMap*map, int layer, int submodel, int strand, int node, bool debugLog = false) {
        statusType = 2;
        statusFrame = frame;
        statusLayer = layer;
        statusStrand = strand;
        statusSubmodel = submodel;
        statusNode = node;
        statusMap = map;
        LogToLogger(debugLog ? spdlog::level::debug : spdlog::level::info);
    }

    void SetCalOutputStatus(int frame, int submodel, int strand, int node, bool debugLog = true) {
        statusType = 3;
        statusFrame = frame;
        statusStrand = strand;
        statusSubmodel = submodel;
        statusNode = node;
        LogToLogger(debugLog ? spdlog::level::debug : spdlog::level::info);
    }

    void SetInializingStatus(int frame, int layer, int submodel, int strand, int node, bool debugLog = false) {
        statusType = 1;
        statusFrame = frame;
        statusLayer = layer;
        statusStrand = strand;
        statusSubmodel = submodel;
        statusNode = node;
        LogToLogger(debugLog ? spdlog::level::debug : spdlog::level::info);
    }
    void SetWaitingStatus(int frame, bool debugLog = false) {
        statusType = 13;
        statusFrame = frame;
    }
    void SetStatus(const std::string &st, bool debugLog = false) {
        statusMsg = st;
        statusType = 0;
        LogToLogger(debugLog ? spdlog::level::debug : spdlog::level::info);
    }

    std::string GetStatus() const override {
        return GetStatusString();
    }

    std::string GetStatusForUser() override
    {
        int lastIdx = 0;
        int submodel = -1;
        if (statusType >= 1 && statusType <= 3) {
            submodel = statusSubmodel;
        }
        Effect* effect = findEffectForFrame(this->statusLayer, GetCurrentFrame(), submodel, lastIdx);

        if (effect != nullptr) {
            std::string mname;
            if (submodel >= 0) {
                mname = "Submodel: " + subModelInfos[submodel]->element->GetName() + " ";
            }
            return std::format("{}Effect: {} Start: {} End {}", mname, effect->GetEffectName(),
                               FORMATTIME(effect->GetStartTimeMS()), FORMATTIME(effect->GetEndTimeMS()));
        }
        if (statusType == 13) {
            return std::format("Waiting to start frame {}", (int)statusFrame);
        }

        return "";
    }

    std::string GetStatusString() const override {
        int frame = statusFrame;
        int layer = statusLayer;
        int strand = statusStrand;
        int node = statusNode;
        int submodel = statusSubmodel;
        std::string n = (submodel == -1 || submodel >= (int)subModelInfos.size()) ? name : subModelInfos[submodel]->element->GetFullName();
        switch (statusType) {
        case 0:
            return statusMsg;
        case 1:
            if (strand == -1) {
                return std::format("Initializing effect at frame {} for {}, layer {}", frame, n, layer);
            } else if (node == -1) {
                return std::format("Initializing strand effect at frame {} for {}, strand {}", frame, n, strand);
            } else {
                return std::format("Initializing node effect at frame {} for {}, strand {}, node {}", frame, n, strand, node);
            }
        case 2:
            if (strand == -1) {
                return std::format("Rendering layer effect for frame {} of {}, layer {}: ", frame, n, layer) + PrintStatusMap();
            } else if (node == -1) {
                return std::format("Rendering strand effect for frame {} of {}, strand {}: ", frame, n, strand) + PrintStatusMap();
            } else {
                return std::format("Rendering node effect for frame {} of {}, strand {}, node {}: ", frame, n, layer, node) + PrintStatusMap();
            }
        case 3:
            if (strand == -1) {
                return std::format("Calculating output at frame {} for {}: ", frame, n) + PrintStatusMap();
            } else if (node == -1) {
                return std::format("Calculating output at frame {} for {}, strand {}: ", frame, n, strand) + PrintStatusMap();
            } else {
                return std::format("Calculating output at frame {} for {}, strand {}, node {}: ", frame, n, strand, node) + PrintStatusMap();
            }
        case 4:
            return std::vformat(statusMsg, std::make_format_args(name, frame));
        case 5:
            return std::vformat(statusMsg, std::make_format_args(name, frame, layer));
        case 8:
            return std::vformat(statusMsg, std::make_format_args(name, frame)) + PrintStatusMap();
        case 9:
            return std::vformat(statusMsg, std::make_format_args(name, frame, layer)) + PrintStatusMap();
        case 13:
            return std::format("Waiting to start frame {} for {}", frame, n);
        }
        return statusMsg;
    }

    SequenceData *createExportBuffer() {
        SequenceData *sb = new SequenceData();
        sb->init(_ctx->GetModel(mainBuffer->GetModelName())->GetActChanCount(), seqData->NumFrames(), seqData->FrameTime(), false);
        seqData = sb;
        return sb;
    }

    PixelBufferClass *getBuffer() const
    {
        return mainBuffer;
    }

    void setRenderRange(int start, int end) {
        startFrame = start;
        endFrame = end;
    }

    void SetRangeRestriction(const std::list<NodeRange> &rng) {
        if (!rng.empty()) {
            rangeRestriction.resize(rng.back().end + 1);
            for (auto i : rng) {
                for (unsigned int s = i.start; s <= i.end; s++) {
                    rangeRestriction[s] = true;
                }
            }
        }
    }

    void SetModelBlending() {
        supportsModelBlending = true;
    }

    int GetEffectFrame(Effect* ef, int frame, int frameTime)
    {
        return frame - (ef->GetStartTimeMS() / frameTime);
    }

    bool ProcessFrame(int frame, Element *el, EffectLayerInfo &info, PixelBufferClass *buffer, int strand = -1, bool blend = false) {
        bool effectsToUpdate = false;
        Effect* tempEffect = nullptr;
        int numLayers = el->GetEffectLayerCount();

        std::vector<bool> partOfCanvas;
        partOfCanvas.resize(info.validLayers.size());
        for (int x = 0; x < (int)info.validLayers.size(); x++) {
            info.validLayers[x] = false;
            partOfCanvas[x] = false;
        }

        // To support canvas mix type we must render them bottom to top
        for (int layer = numLayers - 1; layer >= 0; --layer) {
            EffectLayer* elayer = el->GetEffectLayer(layer);
            //must lock the layer so the Effect* stays valid
            std::unique_lock<std::recursive_mutex> elayerLock(elayer->GetLock());
            Effect* ef = findEffectForFrame(elayer, frame, info.currentEffectIdxs[layer]);
            Effect* copy = nullptr;

            if (ef != nullptr && ef->GetEffectIndex() == EffectManager::eff_DUPLICATE) {
                // we are mirroring another model ... so find the right effect on that model/layer
                Effect* orig = ef;
                
                ef = findEffectForFrame(orig->GetSetting("E_CHOICE_Duplicate_Model"), orig->GetSettings().GetInt("E_SPINCTRL_Duplicate_Layer"), frame);

                if (ef != nullptr) {

                    copy = ef;

                    if (ef->GetEffectIndex() == EffectManager::eff_DUPLICATE) {
                        // we cant duplicate a duplicate
                        ef = nullptr;
                    } else {
                        tempEffect = new Effect(*ef);
                        ef = tempEffect;
                        
                        // disable the background display lists as the "duplicate" effect doesn't support this
                        tempEffect->EnableBackgroundDisplayLists(false);

                        if (orig->GetSetting("E_CHECKBOX_Duplicate_Override_Buffer") == "1") {
                            ef->EraseSettingsStartingWith("B_");
                            for (const auto& it : orig->GetSettings()) {
                                if (StartsWith(it.first, "B_"))
                                    ef->GetSettings()[it.first] = it.second;
                            }
                        }
                        if (orig->GetSetting("E_CHECKBOX_Duplicate_Override_Timing") == "1") {
                            ef->EraseSettingsStartingWith("T_");
                            for (const auto& it : orig->GetSettings()) {
                                if (StartsWith(it.first, "T_"))
                                    ef->GetSettings()[it.first] = it.second;
                            }
                        }
                        if (orig->GetSetting("E_CHECKBOX_Duplicate_Override_Palette") == "1") {
                            ef->ErasePalette();
                            for (const auto& it : orig->GetPaletteMap()) {
                                if (StartsWith(it.first, "C_BUTTON_Palette") || StartsWith(it.first, "C_CHECKBOX_Palette"))
                                    ef->GetPaletteMap()[it.first] = it.second;
                            }
                            ef->SetPalette(ef->GetPaletteAsString()); // this forces the colour parsing
                        }
                        if (orig->GetSetting("E_CHECKBOX_Duplicate_Override_Color") == "1") {
                            ef->EraseColourSettings();
                            for (const auto& it : orig->GetPaletteMap()) {
                                if (!StartsWith(it.first, "C_BUTTON_Palette") && !StartsWith(it.first, "C_CHECKBOX_Palette"))
                                    ef->GetPaletteMap()[it.first] = it.second;
                            }
                        }
                    }
                }
            } 

            Effect* compare = copy != nullptr ? copy : ef;

            if (compare != info.currentEffects[layer]) {
                if (copy != nullptr) {
                    info.currentEffects[layer] = copy;
                } else {
                    info.currentEffects[layer] = ef;
                }
                SetInializingStatus(frame, layer, info.submodel, strand, -1);
                initialize(layer, frame, ef, info.settingsMaps[layer], buffer);
                info.effectStates[layer] = true;
            }

            if (buffer->IsVariableSubBuffer(layer)) {
                buffer->PrepareVariableSubBuffer(frame, layer);
            }

            bool persist = buffer->IsPersistent(layer);
            bool freeze = false;
            if (ef != nullptr && buffer != nullptr) {
                freeze = buffer->GetFreezeFrame(layer) != 999999 && buffer->GetFreezeFrame(layer) <= GetEffectFrame(ef, frame, mainBuffer->GetFrameTimeInMS());
            }

            if ((!persist && !freeze) || info.currentEffects[layer] == nullptr || info.currentEffects[layer]->GetEffectIndex() == -1) {
                buffer->Clear(layer);
            }

            bool suppress = false;
            if (ef != nullptr && buffer != nullptr) {
                suppress = buffer->GetSuppressUntil(layer) > GetEffectFrame(ef, frame, mainBuffer->GetFrameTimeInMS());
            }

            SetRenderingStatus(frame, &info.settingsMaps[layer], layer, info.submodel, strand, -1, true);
            bool b = info.effectStates[layer];

            if (!freeze) {
                // Mix canvas pre-loads the buffer with data from underlying layers
                if (buffer->IsCanvasMix(layer) && layer < numLayers - 1 && !buffer->IsRenderingDisabled(layer)) {
                    maybeWaitForFrame(frame);

                    auto vl = info.validLayers;
                    bool doBlendLayer = false;
                    if (info.settingsMaps[layer].Get("LayersSelected", "") != "") {
                        // remove from valid layers any layers we dont need to include
                        std::vector<std::string> ls;
                        Split(info.settingsMaps[layer].Get("LayersSelected", ""), '|', ls);
                        if (!ls.empty() && ls.back() == "Blend") {
                            doBlendLayer = true;
                            ls.pop_back();
                        }
                        for (int i = layer + 1; i < (int)vl.size(); i++) {
                            if (vl[i]) {
                                bool found = false;
                                for (auto it = ls.begin(); !found && it != ls.end(); ++it) {
                                    if (std::atoi((*it).c_str()) + layer + 1 == i) {
                                        found = true;
                                    }
                                }
                                if (!found) {
                                    vl[i] = false;
                                } else {
                                    partOfCanvas[i] = true;
                                }
                            }
                        }
                        if (doBlendLayer) {
                            buffer->SetColors(numLayers, &((*seqData)[frame][0]));
                            vl[numLayers] = true;
                            blend = false;
                        }
                    } else {
                        // default if not specified is all valid layers below it except the blend layer
                        // mark them as being part of the
                        for (int i = layer + 1; i < (int)vl.size(); i++) {
                            if (vl[i]) {
                                partOfCanvas[i] = true;
                            }
                        }
                    }

                    // preload the buffer with the output from the lower layers
                    RenderBuffer& rb = buffer->BufferForLayer(layer, -1);

                    // We have to calc the output here to apply blend, rotozoom and transitions
                    buffer->CalcOutput(frame, vl, layer, true);
                    std::vector<uint8_t> done(rb.GetPixelCount());
                    parallel_for(0, rb.GetNodes().size(), [&](int n) {
                        for (auto &a : rb.GetNodes()[n]->Coords) {
                            int x = a.bufX;
                            int y = a.bufY;
                            if (x >= 0 && x < rb.BufferWi && y >= 0 && y < rb.BufferHt && y*rb.BufferWi + x < (int)rb.GetPixelCount()) {
                                done[y*rb.BufferWi+x] = true;
                            }
                        }
                    }, 500);
                    // now fill in any spaces in the buffer that don't have nodes mapped to them
                    parallel_for(0, rb.BufferHt, [&rb, &buffer, &done, &vl, frame](int y) {
                        xlColor c;
                        for (int x = 0; x < rb.BufferWi; x++) {
                            if (!done[y * rb.BufferWi + x]) {
                                buffer->GetMixedColor(x, y, c, vl, frame);
                                rb.SetPixel(x, y, c);
                            }
                        }
                        });
                    buffer->UnMergeBuffersForLayer(layer);
                }

                info.validLayers[layer] = _callbacks->RenderEffectFromMap(suppress, ef, layer, frame, info.settingsMaps[layer], *buffer, b, true, &renderEvent);
                effectsToUpdate |= info.validLayers[layer];
                info.effectStates[layer] = b;

                if (suppress) {
                    info.validLayers[layer] = false;
                } else if (info.validLayers[layer]) {
                    buffer->HandleLayerBlurZoom(frame, layer);
                    buffer->HandleLayerTransitions(frame, layer);
                }
            } else {
                info.validLayers[layer] = true;
                info.effectStates[layer] = b;
                effectsToUpdate = true;
            }
        }

        if (effectsToUpdate) {
            maybeWaitForFrame(frame);
            SetCalOutputStatus(frame, info.submodel, strand, -1);
            for (int x = 0; x < (int)partOfCanvas.size(); x++) {
                // if the layer was used for a canvas effect, we don't want it
                // reblended in
                if (partOfCanvas[x]) {
                    info.validLayers[x] = false;
                }
            }
            if (blend) {
                buffer->SetColors(numLayers, &((*seqData)[frame][0]));
                info.validLayers[numLayers] = true;
            }
            buffer->CalcOutput(frame, info.validLayers);
            buffer->GetColors(&((*seqData)[frame][0]), rangeRestriction);
        }

        if (tempEffect != nullptr)
            delete tempEffect;

        return effectsToUpdate;
    }

    std::atomic_int maxFrameBeforeCheck = -1;
    void maybeWaitForFrame(int frame) {
        //make sure we can do this frame
        if (frame >= maxFrameBeforeCheck) {
            SetWaitingStatus(frame);
            maxFrameBeforeCheck = waitForFrame(frame);
            SetGenericStatus("{}: Processing frame {} ", frame, true, true);
        }
    }
    virtual void Process() override {
        
        auto logger_jobpool = spdlog::get("job");
        // Log the thread ID as a hash value
        size_t tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
        logger_jobpool->debug("Render job thread id {0:x} or {0:d}", tid);

        SetGenericStatus("Initializing rendering thread for {}", 0);
        int origChangeCount;
        int ss, es;

        rowToRender->IncWaitCount();
        std::unique_lock<std::recursive_timed_mutex> lock(rowToRender->GetRenderLock());
        if (rowToRender->DecWaitCount() && !HasNext()) {
            // other threads for this model waiting, we'll bail fast and let them handle this
            m_logger->debug("Rendering thread exiting early.");
            currentFrame = END_OF_RENDER_FRAME; // this is needed otherwise the job does not look done
            return;
        }
        SetGenericStatus("Got lock on rendering thread for {}", 0);

        rowToRender->GetAndResetDirtyRange(origChangeCount, ss, es);
        if (ss != -1) {
            //expand to cover the whole dirty range
            ss = ss / seqData->FrameTime();
            if (ss < 0) {
                ss = 0;
            }
            es = es / seqData->FrameTime();
            if (es > (int)seqData->NumFrames()) {
                es = seqData->NumFrames();
            }
            if (ss < startFrame) {
                startFrame = ss;
            }
            if (es > endFrame) {
                endFrame = es;
            }
        }
        if (startFrame < 0) startFrame = 0;
        if (endFrame > (int)seqData->NumFrames()) endFrame = seqData->NumFrames() - 1;

        EffectLayerInfo mainModelInfo(numLayers);
        std::map<SNPair, Effect*> nodeEffects;
        std::map<SNPair, SettingsMap> nodeSettingsMaps;
        std::map<SNPair, bool> nodeEffectStates;
        std::map<SNPair, int> nodeEffectIdxs;

        try {
            //for (int layer = 0; layer < numLayers; ++layer) {
            for (int layer = numLayers - 1; layer >= 0; --layer) {
                SetGenericStatus("Finding starting effect for {}, startFrame {}, and layer {} ", (int)startFrame, layer, false, true);
                EffectLayer *elayer = rowToRender->GetEffectLayer(layer);
                std::unique_lock<std::recursive_mutex> elock(elayer->GetLock());
                mainModelInfo.currentEffects[layer] = findEffectForFrame(elayer, startFrame, mainModelInfo.currentEffectIdxs[layer]);
                SetGenericStatus("Initializing starting effect for {}, startFrame {}, and layer {} ", (int)startFrame, layer, false, true);
                initialize(layer, startFrame, mainModelInfo.currentEffects[layer], mainModelInfo.settingsMaps[layer], mainBuffer);
                mainModelInfo.effectStates[layer] = true;
            }

            for (int frame = startFrame; frame <= endFrame; ++frame) {
                currentFrame = frame;
                SetGenericStatus("{}: Starting frame {} ", frame, true, true);

                if (abort) {
                    rowToRender->SetDirtyRange(frame * seqData->FrameTime(), endFrame * seqData->FrameTime());
                    break;
                }

                if (!HasNext() &&
                        (origChangeCount != rowToRender->getChangeCount()
                         || rowToRender->GetWaitCount())) {
                    //we're bailing out but make sure this range is reconsidered
                    rowToRender->SetDirtyRange(frame * seqData->FrameTime(), endFrame * seqData->FrameTime());
                    break;
                }
                if (abort) {
                    rowToRender->SetDirtyRange(frame * seqData->FrameTime(), endFrame * seqData->FrameTime());
                    break;
                }

                bool cleared = ProcessFrame(frame, rowToRender, mainModelInfo, mainBuffer, -1, supportsModelBlending);
                if (!subModelInfos.empty()) {
                    maybeWaitForFrame(frame);
                    for (const auto& a : subModelInfos) {
                        if (abort) {
                            rowToRender->SetDirtyRange(frame * seqData->FrameTime(), endFrame * seqData->FrameTime());
                            break;
                        }
                        EffectLayerInfo *info = a;
                        ProcessFrame(frame, info->element, *info, info->buffer.get(), info->strand, supportsModelBlending ? true : cleared);
                    }
                }

                if (!nodeBuffers.empty()) {
                    maybeWaitForFrame(frame);
                    for (const auto& it : nodeBuffers) {
                        if (abort) {
                            rowToRender->SetDirtyRange(frame * seqData->FrameTime(), endFrame * seqData->FrameTime());
                            break;
                        }
                        SNPair node = it.first;
                        PixelBufferClass *buffer = it.second.get();

                        if (buffer == nullptr) {
                            spdlog::critical("RenderJob::Process PixelBufferPointer is null ... this is going to crash.");
                        }

                        int strand = node.strand;
                        int inode = node.node;
                        StrandElement *slayer = rowToRender->GetStrand(strand);
                        if (slayer == nullptr) {
                            //deleted strand
                            continue;
                        }
                        EffectLayer *nlayer = slayer->GetNodeLayer(inode, false);
                        if (nlayer == nullptr) {
                            //deleted node
                            continue;
                        }
                        std::unique_lock<std::recursive_mutex> nlayerLock(nlayer->GetLock());
                        Effect *el = findEffectForFrame(nlayer, frame, nodeEffectIdxs[node]);
                        if (el != nodeEffects[node] || frame == startFrame) {
                            nodeEffects[node] = el;
                            SetInializingStatus(frame, -1, -1, strand, inode);
                            initialize(0, frame, el, nodeSettingsMaps[node], buffer);
                            nodeEffectStates[node] = true;
                        }
                        bool persist=buffer->IsPersistent(0);
                        if (!persist || nodeEffects[node] == nullptr || nodeEffects[node]->GetEffectIndex() == -1) {
                            buffer->Clear(0);
                        }

                        SetRenderingStatus(frame, &nodeSettingsMaps[node], -1, -1, strand, inode, cleared);
                        if (_callbacks->RenderEffectFromMap(false, el, 0, frame, nodeSettingsMaps[node], *buffer, nodeEffectStates[node], true, &renderEvent)) {
                            SetCalOutputStatus(frame, -1, strand, inode);
                            buffer->HandleLayerBlurZoom(frame, 0);
                            buffer->HandleLayerTransitions(frame, 0);
                            //copy to output
                            std::vector<bool> valid(2, true);
                            buffer->SetColors(1, &((*seqData)[frame][0]));
                            buffer->CalcOutput(frame, valid);
                            buffer->GetColors(&((*seqData)[frame][0]), rangeRestriction);
                        }
                    }
                }
                //mainBuffer->ApplyDimmingCurves(&((*seqData)[frame][0]));
                if (HasNext()) {
                    SetGenericStatus("{}: Notifying next renderer of frame {} done", frame, true);
                    FrameDone(frame);
                }
            }
            SetGenericStatus("{}: All done - Completed frame {} ", endFrame, true, false);
        } catch ( std::exception &ex) {
            assert(false); // so when we debug we catch them
            printf("Caught an exception %s", ex.what());
            m_logger->error("Caught an exception on rendering thread: " + std::string(ex.what()));
            spdlog::error("Caught an exception on rendering thread: {}", ex.what());
		} catch ( ... ) {
            assert(false); // so when we debug we catch them
            printf("Caught an unknown exception");
            m_logger->error("Caught an unknown exception on rendering thread.");
            spdlog::error("Caught an unknown exception on rendering thread.");
        }
        if (HasNext()) {
            //make sure the previous has told us we're at the end.  If we return before waiting, the previous
            //may try sending the END_OF_RENDER_FRAME to us and we'll have been deleted
            SetGenericStatus("{}: Waiting on previous renderer for final frame", 0, true);
            waitForFrame(END_OF_RENDER_FRAME);

            //let the next know we're done
            SetGenericStatus("{}: Notifying next renderer of final frame", 0, true);
            FrameDone(END_OF_RENDER_FRAME);
            _callbacks->OnRenderJobComplete(rowToRender->GetModelName());
            SetGenericStatus("{}: All done - Completed frame {} ", endFrame, true, false);
        } else {
            _callbacks->OnAllRenderJobsComplete();
        }
        rowToRender->CleanupAfterRender();
        currentFrame = END_OF_RENDER_FRAME;
        //printf("Done rendering %lx (next %lx)\n", (unsigned long)this, (unsigned long)next);
        m_logger->debug("Rendering thread exiting.");
	}

    void AbortRender() override {
        abort = true;
    }

    ModelElement* GetModelElement() const { return rowToRender; }

private:

    void initialize(int layer, int frame, Effect *el, SettingsMap &settingsMap, PixelBufferClass *buffer) {
        bool layerEnabled = true;
        if (el == nullptr || el->GetEffectIndex() == -1) {
            settingsMap.clear();
            layerEnabled = false;
        } else {
            auto e = el->GetParentEffectLayer()->GetParentElement();
            if (e != nullptr) {
                layerEnabled = !e->IsRenderDisabled();
            }
            loadSettingsMap(el->GetEffectName(),
                            el,
                            settingsMap);
        }
        buffer->SetLayerSettings(layer, settingsMap, layerEnabled);
        if (el != nullptr) {
            xlColorVector newcolors;
            xlColorCurveVector newcc;
            el->CopyPalette(newcolors, newcc);
            buffer->SetPalette(layer, newcolors, newcc);
            buffer->SetTimes(layer, el->GetStartTimeMS(), el->GetEndTimeMS());
        }
    }

    Effect* findEffectForFrame(const std::string& model, const int layer, int frame)
    {
        Effect* res = nullptr;

        Element* e = rowToRender->GetSequenceElements()->GetElement(model);

        if (e != nullptr) {
            EffectLayer* el = e->GetEffectLayer(layer - 1);

            if (el != nullptr) {
                int discard = 0;
                res = findEffectForFrame(el, frame, discard);
            }
        }

        return res;
    }

    Effect *findEffectForFrame(EffectLayer* layer, int frame, int &lastIdx) {
        if (layer == nullptr) {
            return nullptr;
        }
        int time = frame * seqData->FrameTime();
        for (int e = lastIdx; e < layer->GetEffectCount(); ++e) {
            Effect *effect = layer->GetEffect(e);
            int st = effect->GetStartTimeMS();
            int et = effect->GetEndTimeMS();
            if (et > time && st <= time) {
                return effect;
            }
        }
        return nullptr;
    }

    Effect *findEffectForFrame(int layer, int frame, int &lastIdx) {
        return findEffectForFrame(rowToRender->GetEffectLayer(layer), frame, lastIdx);
    }
    Effect *findEffectForFrame(int layer, int frame, int submodel, int &lastIdx) {
        if (submodel == -1) {
            return findEffectForFrame(rowToRender->GetEffectLayer(layer), frame, lastIdx);
        }
        return findEffectForFrame(subModelInfos[submodel]->element->GetEffectLayer(layer), frame, lastIdx);
    }

    void loadSettingsMap(const std::string &effectName,
                         Effect *effect,
                         SettingsMap& settingsMap) {
        settingsMap.clear();
        effect->CopySettingsMap(settingsMap, true);
    }

    ModelElement *rowToRender;
    std::string name;
    PixelBufferClass *mainBuffer;
    int numLayers;
    std::atomic_int startFrame;
    std::atomic_int endFrame;
    RenderContext *_ctx;
    IRenderJobCallbacks *_callbacks;
    SequenceData *seqData;
    std::vector<bool> rangeRestriction;
    bool supportsModelBlending;
    RenderEvent renderEvent;

    //stuff for handling the status;
    std::string statusMsg;
    volatile int statusType;
    volatile int statusFrame;
    SettingsMap *statusMap;
    volatile int statusLayer;
    volatile int statusSubmodel = -1;
    volatile int statusStrand = -1;
    volatile int statusNode = -1;
    std::shared_ptr<spdlog::logger> m_logger{ nullptr };

    std::function<void(int, const std::string&)> progressCallback;
    std::atomic_int currentFrame;
    std::atomic_bool abort;

    std::vector<EffectLayerInfo *> subModelInfos;

    std::map<SNPair, PixelBufferClassPtr> nodeBuffers;
};


// RenderRange — moved to RenderUI.cpp (uses RenderCommandEvent wx type)

void xLightsFrame::RenderMainThreadEffects() {
    std::unique_lock<std::mutex>  lock(renderEventLock);
    while (!mainThreadRenderEvents.empty()) {
        RenderEvent *evt = mainThreadRenderEvents.front();
        mainThreadRenderEvents.pop();
        lock.unlock();
        RenderEffectOnMainThread(evt);
        lock.lock();
    }

}

void xLightsFrame::RenderEffectOnMainThread(RenderEvent *ev) {
    std::unique_lock<std::mutex> lock(ev->mutex);

    // validate that the effect still exists as this could be being processed after the effect was deleted
    // Also check preset elements - used when rendering shader/effect previews via RenderEffectToFrames
    if (_sequenceElements.IsValidEffect(ev->effect) || _presetSequenceElements.IsValidEffect(ev->effect)) {
        ev->returnVal = RenderEffectFromMap(ev->suppress, ev->effect,
            ev->layer,
            ev->period,
            *ev->settingsMap,
            *ev->buffer, *ev->ResetEffectState, false, ev) ? 1 : 0;
    } else {
        assert(false);
    }
    ev->signal.notify_all();
}

// RenderProgressInfo is defined in RenderProgressInfo.h (included above).
// It was moved to a header so RenderUI.cpp can also access it.

// LogRenderStatus — moved to RenderUI.cpp (needs access to RenderProgressInfo)

static bool HasEffects(ModelElement *me) {
    if (me->HasEffects()) {
        return true;
    }

    for (int x = 0; x < me->GetSubModelAndStrandCount(); ++x) {
        if (me->GetSubModel(x)->HasEffects()) {
            return true;
        }
    }
    for (int x = 0; x < me->GetStrandCount(); ++x) {
        StrandElement *se = me->GetStrand(x);
        for (int n = 0; n < se->GetNodeLayerCount(); ++n) {
            if (se->GetNodeLayer(n)->GetEffectCount() > 0) {
                return true;
            }
        }
    }
    return false;
}

// OnProgressBarDoubleClick, OnRenderStatusTimerTrigger, UpdateRenderStatus,
// RenderDone — all moved to RenderUI.cpp (wx UI handlers / progress-bar updates).

class RenderTreeData {
public:
    RenderTreeData(Model *e): model(e) {

        
        if (e == nullptr) {
            spdlog::critical("Render tree has a null model ... this is not going to end well.");
        }

        ModelGroup *mg = dynamic_cast<ModelGroup*>(e);
        if (mg != nullptr) {
            // might need to recalculate the group nodes
            mg->CheckForChanges();
        }

        for (size_t node = 0; node < e->GetNodeCount(); ++node) {
            unsigned int start = e->NodeStartChannel(node);
            unsigned int end = e->NodeEndChannel(node);
            AddRange(start, end);
        }
        sortRanges(ranges);
    }

    void AddRange(unsigned int start, unsigned int end) {
        if (!ranges.empty()) {
            if ((ranges.back().end + 1) == start) {
                ranges.back().end = end;
                return;
            }
        }
        ranges.push_back(NodeRange(start, end));
    }

    bool Overlaps(RenderTreeData &e) {
        for (const auto& it : ranges) {
            for (const auto& it2 : e.ranges) {
                if (it.Overlaps(it2)) {
                    return true;
                }
            }
        }
        return false;
    }

    static void sortRanges(std::list<NodeRange> &ranges) {
        ranges.sort();
        auto it = ranges.begin();
        auto it2 = ranges.begin();
        if (it2 != ranges.end()) {
            ++it2;
        }
        while (it2 != ranges.end()) {
            if ((it->end + 1) == it2->start) {
                //it2 is immediately at the end of it
                it->end = it2->end;
                it2 = ranges.erase(it2);
            } else if (it->end >= (it2->start - 1) && (it->end <= it2->end)) {
                // it2 overlaps the end of it
                it->end = it2->end;
                it2 = ranges.erase(it2);
            } else if (it2->start <= it->end && it->end >= it2->end) {
                //it2 fully contained in it
                it2 = ranges.erase(it2);
            } else {
                ++it;
                ++it2;
            }
        }
    }

    void Add(Model *el) {
        renderOrder.push_back(el);
    }
    std::list<Model *> renderOrder;
    std::list<NodeRange> ranges;
    Model *model;
};

void xLightsFrame::RenderTree::Clear() {
    for (auto it : data) {
        delete it;
    }
    data.clear();
}

void xLightsFrame::RenderTree::Add(Model *el) {
    RenderTreeData *elData = new RenderTreeData(el);
    for (const auto& it : data) {
        RenderTreeData *elData2 = it;
        if (elData2->Overlaps(*elData)) {
            elData->Add(elData2->model);
            elData2->Add(el);
        }
    }
    elData->Add(el);
    data.push_back(elData);
}

void xLightsFrame::RenderTree::Print() {
    auto logger_render = spdlog::get("render");
    logger_render->debug("========== RENDER TREE");
    for (const auto& it : data) {
        //printf("%s:   (%d)\n", (*it)->model->GetName().c_str(), (int)(*it)->ranges.size());
        logger_render->debug("   {}:   ({})", it->model->GetName(), (int)it->ranges.size());
        for (const auto& it2 : it->renderOrder) {
            //printf("    %s     \n", it2->GetName().c_str());
            logger_render->debug("        {}", it2->GetName());
        }
    }
    logger_render->debug("========== END RENDER TREE");
}

std::list<Model*> xLightsFrame::RenderTree::GetModels() const {
    std::list<Model*> models;
    for (const auto& it : data) {
        models.push_back(it->model);
    }
    return models;
}

void xLightsFrame::BuildRenderTree() {
    unsigned int curChangeCount = _sequenceElements.GetMasterViewChangeCount() + modelsChangeCount;
    if (renderTree.renderTreeChangeCount != curChangeCount) {
        renderTree.Clear();
        const int numEls = _sequenceElements.GetElementCount(MASTER_VIEW);
        if (numEls == 0) {
            //nothing to do....
            return;
        }
        for (size_t row = 0; row < (size_t)numEls; ++row) {
            Element *rowEl = _sequenceElements.GetElement(row, MASTER_VIEW);
            if (rowEl != nullptr && rowEl->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                Model *model = GetModel(rowEl->GetModelName());
                if (model != nullptr) {
                    renderTree.Add(model);
                }
            }
        }
        renderTree.Print();
        renderTree.renderTreeChangeCount = curChangeCount;
    }
}

void xLightsFrame::Render(SequenceElements& seqElements,
                          SequenceData& seqData,
                          const std::list<Model*> models,
                          const std::list<Model *> &restrictToModels,
                          int startFrame, int endFrame,
                          std::unique_ptr<IRenderProgressSink> sink, bool clear,
                          std::function<void(bool)>&& callback)
{
    abortedRenderJobs = 0;

    auto logger_render = spdlog::get("render");
    if (startFrame < 0) {
        startFrame = 0;
    }
    if (endFrame >= (int)seqData.NumFrames()) {
        endFrame = seqData.NumFrames() - 1;
    }
    std::list<NodeRange> ranges;
    if (restrictToModels.empty()) {
        ranges.push_back(NodeRange(0, seqData.NumChannels()));
    } else {
        for (const auto& it : restrictToModels) {
            RenderTreeData data(it);
            ranges.insert(ranges.end(), data.ranges.begin(), data.ranges.end());
        }
        RenderTreeData::sortRanges(ranges);
    }
    int numRows = models.size();
    RenderJob **jobs = new RenderJob*[numRows];
    AggregatorRenderer **aggregators = new AggregatorRenderer*[numRows];
    std::vector<std::set<int>> channelMaps(seqData.NumChannels());

    size_t row = 0;
    for (auto it = models.begin(); it != models.end(); ++it, ++row) {
        jobs[row] = nullptr;
        aggregators[row] = new AggregatorRenderer(seqData.NumFrames());

        Element *rowEl = seqElements.GetElement((*it)->GetName());

        if (rowEl == nullptr) {
            //spdlog::critical("xLightsFrame::Render rowEl is nullptr ... this is going to crash looking for '{}'.", (const char *)(*it)->GetName().c_str());
        } else {
            if (rowEl->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                ModelElement *me = dynamic_cast<ModelElement *>(rowEl);

                if (me == nullptr) {
                    logger_render->critical("xLightsFrame::Render me is nullptr ... this is going to crash.");
                }

                bool hasEffects = HasEffects(me);
                bool isRestricted = std::find(restrictToModels.begin(), restrictToModels.end(), *it) != restrictToModels.end();
                if (hasEffects || (isRestricted && clear)) {
                    RenderJob *job = new RenderJob(me, seqData, this, this);

                    if (job == nullptr) {
                        logger_render->critical("xLightsFrame::Render job is nullptr ... this is going to crash.");
                    }

                    job->setRenderRange(startFrame, endFrame);
                    job->SetRangeRestriction(ranges);
                    if (seqElements.SupportsModelBlending()) {
                        job->SetModelBlending();
                    }
                    PixelBufferClass *buffer = job->getBuffer();
                    if (buffer == nullptr || buffer->GetNodeCount() == 0) {
                        delete job;
                        continue;
                    }

                    jobs[row] = job;
                    aggregators[row]->addNext(job);
                    size_t cn = buffer->GetChanCountPerNode();
                    for (size_t node = 0; node < buffer->GetNodeCount(); ++node) {
                        uint32_t start = buffer->NodeStartChannel(node);
                        for (size_t c = 0; c < cn; ++c) {
                            size_t cnum = start + c;
                            if (cnum < seqData.NumChannels()) {
                                for (const auto i : channelMaps[cnum]) {
                                    int idx = i;
                                    if ((size_t)idx != row) {
                                        if (jobs[idx]->addNext(aggregators[row])) {
                                            aggregators[row]->incNumAggregated();
                                        }
                                    }
                                }
                                channelMaps[cnum].insert(row);
                            }
                        }
                    }
                }
            }
        }
    }

    logger_render->debug("Aggregators created.");

    channelMaps.clear();
    unsigned int count = 0;
    if (clear) {
        for (int f = startFrame; f <= endFrame; f++) {
            for (const auto& it : ranges) {
                seqData[f].Zero(it.start, it.end - it.start + 1);
            }
        }
    }

    logger_render->debug("Data cleared.");

    for (row = 0; row < (size_t)numRows; ++row) {
        if (jobs[row]) {
            if (aggregators[row]->getNumAggregated() == 0) {
                //start all the jobs that don't depend on anything above them
                //get them rendering while we setup the rest
                jobs[row]->setPreviousFrameDone(END_OF_RENDER_FRAME);
                jobPool.PushJob(jobs[row]);
                ++count;
            }
            if (sink) {
                sink->SetupJobProgress(jobs[row]);
            }
        }
    }

    logger_render->debug("Job pool start size {}.", (int)jobPool.size());
    for (row = 0; row < (size_t)numRows; ++row) {
        if (jobs[row] && aggregators[row]->getNumAggregated() != 0) {
            //now start the rest
            jobPool.PushJob(jobs[row]);
            ++count;
        }
    }
    logger_render->debug("Job pool new size {}.", (int)jobPool.size());

    if (count) {
        if (sink) {
            sink->OnRenderSetupComplete();
        }

        // Copy RenderJob* array into IRenderJobStatus* array for RenderProgressInfo
        // (RenderProgressInfo only needs the status interface for progress/cleanup).
        IRenderJobStatus **statusJobs = new IRenderJobStatus*[numRows];
        for (int i = 0; i < numRows; ++i) {
            statusJobs[i] = jobs[i]; // implicit upcast
        }
        delete[] jobs;

        RenderProgressInfo *pi = new RenderProgressInfo(std::move(callback));
        pi->numRows = numRows;
        pi->startFrame = startFrame;
        pi->endFrame = endFrame;
        pi->jobs = statusJobs;
        pi->progressSink = sink.release(); // RenderProgressInfo takes ownership
        pi->restriction = restrictToModels;
        pi->aggregators = aggregators;

        renderProgressInfo.push_back(pi);
        RenderStatusTimer.Start(100, false);
    } else {
        delete[] jobs;
        delete[] aggregators;
        callback(abortedRenderJobs > 0);
        // sink auto-deleted by unique_ptr
    }
}

static void addModelsUpTo(std::list<Model*> &models, const std::list<Model *> &toAdd, Model *upTo) {
    for (auto it = toAdd.begin(); it != toAdd.end(); ++it) {
        bool add = true;
        for (auto it2 = models.begin(); it2 != models.end() && add; ++it2) {
            if (*it2 == *it) {
                add = false;
            }
        }
        if (add) {
            models.push_back(*it);
        }
        if (upTo == *it) {
            return;
        }
    }
}

static void addModelsFrom(std::list<Model*> &models, const std::list<Model *> &toAdd, Model *from) {
    bool found = false;
    for (auto it = toAdd.begin(); it != toAdd.end(); ++it) {
        if (!found && from != *it) {
            continue;
        }
        found = true;
        bool add = true;
        for (auto it2 = models.begin(); it2 != models.end() && add; ++it2) {
            if (*it2 == *it) {
                add = false;
            }
        }
        if (add) {
            models.push_back(*it);
        }
    }
}

void xLightsFrame::RenderDirtyModels() {

    if (_suspendRender) return; // dont render if suspended

    BuildRenderTree();
    if (renderTree.data.empty()) {
        //nothing to do....
        return;
    }
    const int numRows = _sequenceElements.GetElementCount();
    if (numRows == 0) {
        return;
    }
    int startms = 9999999;
    int endms = -1;
    std::list<Model *> models;
    std::list<Model *> restricts;
    for (int x = 0; x < numRows; x++) {
        Element *el = _sequenceElements.GetElement(x);
        if (el->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
            int st, ed;
            el->GetDirtyRange(st, ed);
            if (st != -1) {
                startms = std::min(startms, st);
                endms = std::max(endms, ed);
                for (auto it = renderTree.data.begin(); it != renderTree.data.end(); ++it) {
                    if ((*it)->model->GetName() == el->GetModelName()) {
                        restricts.push_back((*it)->model);
                        addModelsUpTo(models, (*it)->renderOrder, (*it)->model);
                    }
                }
            }
        }
    }
    if (restricts.empty()) {
        return;
    }
    for (auto x = models.begin(); x != models.end(); ++x) {
        for (auto it = renderTree.data.begin(); it != renderTree.data.end(); ++it) {
            if ((*it)->model == *x) {
                addModelsFrom(models, (*it)->renderOrder, (*it)->model);
            }
        }
    }
    if (startms < 0) {
        startms = 0;
    }
    if (endms < 0) {
        endms = 0;
    }
    int startframe = startms /_seqData.FrameTime() - 1;
    if (startframe < 0) {
        startframe = 0;
    }
    int endframe = endms / _seqData.FrameTime() + 1;
    if (endframe >= (int)_seqData.NumFrames()) {
        endframe = _seqData.NumFrames() - 1;
    }
    if (endframe < startframe) {
        return;
    }
    Render(_sequenceElements, _seqData, models, restricts, startframe, endframe, nullptr, true, [] (bool) {});
}

bool xLightsFrame::AbortRender(int maxTimeMS)
{
    return AbortRender(maxTimeMS, nullptr);
}

bool xLightsFrame::AbortRender(int maxTimeMS, int* numThreadsAborted)
{
    
    static bool inAbort = false;
    if (renderProgressInfo.empty()) {
        //nothing to abort, return quickly
        return true;
    }
    if (inAbort) {
        return false;
    }
    inAbort = true;
    spdlog::info("Aborting rendering ...");
    int abortCount = 0;
    for (auto rpi : renderProgressInfo) {
        //abort whatever is rendering
        for (size_t row = 0; row < (size_t)rpi->numRows; ++row) {
            if (rpi->jobs[row]) {
                rpi->jobs[row]->AbortRender();
                ++abortCount;
                ++abortedRenderJobs;
            }
        }
    }
    int maxLoops = maxTimeMS;
    maxLoops /= 10;   //doing a 10ms sleep
    //must wait for the rendering to complete
    spdlog::info("Aborting {} renderers", abortCount);
    int loops = 0;
    while (!renderProgressInfo.empty() && loops < maxLoops) {
        loops++;
        RenderMainThreadEffects(); // make sure main thread effects are rendered
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        UpdateRenderStatus(); // a side effect is to clean up renderProgressInfo
        if (!renderProgressInfo.empty() && loops > 25) {
            wxYield(); // not sure this is advisable ... but it makes the app look responsive
            // only do this if it's taking more than 250ms to abort.  Less than 250ms and ui will
            // still be "responsive enough" and MOST of the time it should abort in under 250ms.
            // The yeild COULD end up sending more mouse events and such which could trigger
            // more aborts or have events prcessed out of order.   Thus, avoid the yield for most
            // cases.
        }
        if (loops % 200 == 0) {
            spdlog::info("    Waiting for renderers to abort. {} left.", (int)renderProgressInfo.size());
        }
    }
    spdlog::info("    Aborting renderers ... Done");
    inAbort = false;
    if( numThreadsAborted != nullptr ) {
        *numThreadsAborted = abortCount;
    }
    return renderProgressInfo.empty();
}

// RenderGridToSeqData — moved to RenderUI.cpp (creates WxRenderProgressSink)

void xLightsFrame::RenderEffectForModel(const std::string &model, int startms, int endms, bool clear) {

    

    if (_suspendRender) return;

    BuildRenderTree();

    spdlog::debug("Render tree built for model {} {}ms-{}ms. {} entries.",
        (const char *)model.c_str(),
        startms,
        endms,
        renderTree.data.size());

    int startframe = startms / _seqData.FrameTime();// -1; by expanding the range we end up rendering more than necessary for no obvious reason

    // If there is an effect at the start time that has the persistent flag set then include the prior frame
    // This expands the render time but only when it absolutely must
    if (GetPersistentEffectOnModelStartingAtTime(model, startms) != nullptr)
    {
        startframe -= 1;
    }

    if (startframe < 0) {
        startframe = 0;
    }
    int endframe = endms / _seqData.FrameTime();// +1; by expanding the range we end up rendering more than necessary for no obvious reason

    // If there is an effect at the end time that has the persistent flag set then include the nextframe
    // This expands the render time but only when it absolutely must
    Effect* persistentEffectAfter = GetPersistentEffectOnModelStartingAtTime(model, endms);
    if (persistentEffectAfter != nullptr)
    {
        endframe = persistentEffectAfter->GetEndTimeMS() / _seqData.FrameTime();
    }

    if (endframe >= (int)_seqData.NumFrames()) {
        endframe = _seqData.NumFrames() - 1;
    }
    for (const auto& it : renderTree.data) {
        if (it->model->GetName() == model) {

            for (const auto& it2 : renderProgressInfo) {
                //we're going to render this model, abort whatever is rendering and accumulate the frames
                RenderProgressInfo *rpi = it2;
                if (std::find(rpi->restriction.begin(), rpi->restriction.end(), it->model) != rpi->restriction.end()) {
                    if (startframe > rpi->startFrame) {
                        startframe = rpi->startFrame;
                    }
                    if (endframe < rpi->endFrame) {
                        endframe = rpi->endFrame;
                    }
                    for (size_t row = 0; row < (size_t)rpi->numRows; ++row) {
                        if (rpi->jobs[row]) {
                            rpi->jobs[row]->AbortRender();
                        }
                    }
                }
            }
            std::list<Model *> m;
            m.push_back(it->model);

            spdlog::debug("Rendering {} models {} frames.", m.size(), endframe - startframe + 1);

            Render(_sequenceElements, _seqData, it->renderOrder, m, startframe, endframe, nullptr, true, [] (bool) {});
        }
    }
}

// RenderTimeSlice — defined in RenderUI.cpp (wx: ProgressBar, EnableSequenceControls, CallAfter)

bool xLightsFrame::DoExportModel(unsigned int startFrame, unsigned int endFrame,
                                  const std::string& model, const std::string& fn,
                                  const std::string& fmt, bool doRender)
{
    if (endFrame == 0)
        endFrame = _seqData.NumFrames();

    Model* m = GetModel(model);
    if (m == nullptr)
        return false;

    if (m->GetDisplayAs() == DisplayAsType::ModelGroup)
        return false;

    std::string filename(fn);
    std::string format(fmt);

    auto sw = std::chrono::steady_clock::now();
    std::string Out3 = format.substr(0, 3);

    if (Out3 == "LSP") {
        filename = filename + "_USER";
    }
    std::filesystem::path oName(filename);

    if (oName.parent_path().empty()) {
        oName = std::filesystem::path(CurrentDir.ToStdString()) / oName.filename();
    }
    std::string fullpath;

    SetStatusText(std::format("Starting Export for {} - {}", format, Out3));
    wxYield();

    Element* el = _sequenceElements.GetElement(model);
    if (el == nullptr)
        return false;
    RenderJob* job = new RenderJob(dynamic_cast<ModelElement*>(el), _seqData, this, this);
    assert(job != nullptr);
    SequenceData* data = job->createExportBuffer();
    assert(data != nullptr);
    int cpn = job->getBuffer()->GetChanCountPerNode();

    if (doRender) {
        RenderAll();
        // Wait for any in-progress render to complete
        while (mRendering) {
            wxYield();
        }
    }
    Model* m2 = GetModel(model);
    // firstChan is the absolute 0-based channel of node 0; subtract it from nstart
    // so that node data is packed at the start of the export buffer (index 0).
    int firstChan = m2->NodeStartChannel(0);
    for (size_t frame = 0; frame < _seqData.NumFrames(); ++frame) {
        for (size_t x = 0; x < job->getBuffer()->GetNodeCount(); ++x) {
            int ostart = m2->NodeStartChannel(x);
            int nstart = ostart - firstChan;
            job->getBuffer()->SetNodeChannelValues(x, &_seqData[frame][ostart]);
            job->getBuffer()->GetNodeChannelValues(x, &((*data)[frame][nstart]));
        }
    }
    delete job;

    if (Out3 == "Lcb") {
        oName.replace_extension(".lcb");
        fullpath = oName.string();
        int lcbVer = 1;
        if (format.find("S5") != std::string::npos) {
            lcbVer = 2;
        }
        WriteLcbFile(fullpath, data->NumChannels(), startFrame, endFrame, data, lcbVer, cpn);
    } else if (Out3 == "Vir") {
        oName.replace_extension(".vir");
        fullpath = oName.string();
        WriteVirFile(fullpath, data->NumChannels(), startFrame, endFrame, data);
    } else if (Out3 == "LSP") {
        oName.replace_extension(".xml");
        fullpath = oName.string();
        WriteLSPFile(fullpath, data->NumChannels(), startFrame, endFrame, data, cpn);
    } else if (Out3 == "HLS") {
        oName.replace_extension(".hlsnc");
        fullpath = oName.string();
        WriteHLSFile(fullpath, data->NumChannels(), startFrame, endFrame, data);
    } else if (Out3 == "FPP") {
        int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
        oName.replace_extension(".eseq");
        fullpath = oName.string();
        bool v2 = format.find("Compressed") != std::string::npos;
        WriteFalconPiModelFile(fullpath, data->NumChannels(), startFrame, endFrame, data, stChan, data->NumChannels(), v2);
    } else if (Out3 == "Com") {
        int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
        oName.replace_extension(".mp4");
        fullpath = oName.string();
        WriteVideoModelFile(fullpath, data->NumChannels(), startFrame, endFrame, data, stChan, data->NumChannels(), GetModel(model), true);
    } else if (Out3 == "Unc") {
        int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
        fullpath = oName.string();
        WriteVideoModelFile(fullpath, data->NumChannels(), startFrame, endFrame, data, stChan, data->NumChannels(), GetModel(model), false);
    } else if (Out3 == "Min") {
        int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
        oName.replace_extension(".bin");
        fullpath = oName.string();
        WriteMinleonNECModelFile(fullpath, data->NumChannels(), startFrame, endFrame, data, stChan, data->NumChannels(), GetModel(model));
    } else if (Out3 == "GIF") {
        int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
        oName.replace_extension(".gif");
        fullpath = oName.string();
        WriteGIFModelFile(fullpath, data->NumChannels(), startFrame, endFrame, data, stChan, data->NumChannels(), GetModel(model), _seqData.FrameTime());
    }
    float s = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - sw).count();
    s /= 1000;
    SetStatusText(std::format("Finished writing model: {} in {:.3f}s ", fullpath, s));

    delete data;
    EnableSequenceControls(true);

    return true;
}

bool xLightsFrame::RenderEffectFromMap(bool suppress, Effect* effectObj, int layer, int period, SettingsMap& SettingsMap,
    PixelBufferClass& buffer, bool& resetEffectState,
    bool bgThread, RenderEvent* event)
{
    
    auto logger_render = spdlog::get("render");

    // dont render disabled effects
    if (effectObj == nullptr) return false;
    
    if (layer >= buffer.GetLayerCount()) {
        logger_render->error("Model {} Effect {} at frame {} tried to render on a layer {} that does not exist (Only {} found).",
            (const char*)buffer.GetModel()->GetName().c_str(), (const char*)effectObj->GetEffectName().c_str(), period, layer + 1, buffer.GetLayerCount());
        assert(false);
        return false;
    }

    if (buffer.IsRenderingDisabled(layer)) {
        return false;
    }
    
    if (buffer.BufferForLayer(layer, -1).BufferHt == 0 || buffer.BufferForLayer(layer, -1).BufferWi == 0) {
        return false;
    }

    if (buffer.GetModel() != nullptr && buffer.GetModel()->GetNodeCount() == 0) {
        // this happens with custom models with no nodes defined
        if (buffer.BufferForLayer(layer, 0).curEffStartPer == period) {
            logger_render->warn("Model {} has no nodes so skipping rendering.", (const char*)buffer.GetModel()->GetName().c_str());
        }
        return false;
    }

    bool retval = true;

    buffer.SetLayer(layer, period, resetEffectState);
    resetEffectState = false;
    int eidx = -1;

    xlColor colorMask = xlColor::NilColor();
    if (effectObj != nullptr) {
        eidx = effectObj->GetEffectIndex();

        // color masks use the underlying model color to ensure backgrounds show what it will really look like
        // need to do it every render as effects can move around
        const Model* m = buffer.GetModel();

        if (m == nullptr) {
            // this could be a strand or node
            m = GetModel(buffer.GetModelName());
        }

        if (m != nullptr) {
            if (m->GetStringType().compare(0, 12, "Single Color") == 0 || m->GetStringType() == "Node Single Color") {
                colorMask = buffer.GetNodeMaskColor(0);

                // If black ... then dont mask
                if (colorMask == xlBLACK) {
                    colorMask = xlColor::NilColor();
                }
            }
        }
        effectObj->SetColorMask(colorMask);
    }

    if (eidx >= 0) {
        RenderableEffect* reff = effectManager.GetEffect(eidx);

        if (reff) {
            RenderBuffer* b = &buffer.BufferForLayer(layer, -1);
            if (b == nullptr) {
                // this is bad nothing will work
                logger_render->warn("render on model {} layer {} effect {} from {}ms returned no buffer ... skipping rendering.", (const char*)buffer.GetModelName().c_str(), layer, (const char*)reff->Name().c_str(), effectObj->GetStartTimeMS());
            }
            else {
                if (bgThread && !reff->CanRenderOnBackgroundThread(effectObj, SettingsMap, *b)) {
                    event->effect = effectObj;
                    event->layer = layer;
                    event->period = period;
                    event->settingsMap = &SettingsMap;
                    event->ResetEffectState = &resetEffectState;
                    event->buffer = &buffer;
                    event->suppress = suppress;

                    std::unique_lock<std::mutex> lock(event->mutex);

                    std::unique_lock<std::mutex> qlock(renderEventLock);
                    mainThreadRenderEvents.push(event);
                    qlock.unlock();

                    CallAfter(&xLightsFrame::RenderMainThreadEffects);
                    if (event->signal.wait_for(lock, std::chrono::seconds(10)) == std::cv_status::no_timeout) {
                        retval = event->returnVal == 1;
                    }
                    else {
                        logger_render->warn("HELP!!!!   Frame #{} render on model {} ({}x{}) layer {} effect {} from {}ms (#{}) to {}ms (#{}) timed out 10 secs.", b->curPeriod, (const char*)buffer.GetModelName().c_str(), b->BufferWi, b->BufferHt, layer, (const char*)reff->Name().c_str(), effectObj->GetStartTimeMS(), b->curEffStartPer, effectObj->GetEndTimeMS(), b->curEffEndPer);
                        printf("HELP!!!!   Frame #%d render on model %s (%dx%d) layer %d effect %s from %dms (#%d) to %dms (#%d) timed out 10 secs.\n", b->curPeriod, (const char*)buffer.GetModelName().c_str(), b->BufferWi, b->BufferHt, layer, (const char*)reff->Name().c_str(), effectObj->GetStartTimeMS(), b->curEffStartPer, effectObj->GetEndTimeMS(), b->curEffEndPer);

                        // Give it one more chance
                        if (event->signal.wait_for(lock, std::chrono::seconds(60)) == std::cv_status::no_timeout) {
                            retval = event->returnVal == 1;
                        }
                        else {
                            logger_render->warn("DOUBLE HELP!!!!   Frame #{} render on model {} ({}x{}) layer {} effect {} from {}ms (#{}) to {}ms (#{}) timed out 70 secs.", b->curPeriod, (const char*)buffer.GetModelName().c_str(), b->BufferWi, b->BufferHt, layer, (const char*)reff->Name().c_str(), effectObj->GetStartTimeMS(), b->curEffStartPer, effectObj->GetEndTimeMS(), b->curEffEndPer);
                            printf("DOUBLE HELP!!!!   Frame #%d render on model %s (%dx%d) layer %d effect %s from %dms (#%d) to %dms (#%d) timed out 70 secs.\n", b->curPeriod, (const char*)buffer.GetModelName().c_str(), b->BufferWi, b->BufferHt, layer, (const char*)reff->Name().c_str(), effectObj->GetStartTimeMS(), b->curEffStartPer, effectObj->GetEndTimeMS(), b->curEffEndPer);
                        }
                    }
                    if (period % 10 == 0) {
                        //constantly putting stuff on CallAfter can result in the main
                        //dispatch thread never being able to empty the CallAfter
                        //queue and thus effectively blocking.   We'll yield periodically to
                        //allow the main thread to hopefully continue
                        std::this_thread::yield();

                        // After yield who knows what may or may not be valid so we need to revalidate it
                        if (!_sequenceElements.IsValidEffect(event->effect)) {
                            logger_render->error("In RenderEffectFromMap after Yield() call: effect is no longer valid (expected during abort/delete).");
                        }
                    }
                }
                else {
                    int bufCnt = buffer.BufferCountForLayer(layer);
                    std::function<void(int)> f([this, &buffer, layer, suppress, effectObj, reff, &SettingsMap, logger_render](int bufn) {
                        RenderBuffer* rb = &buffer.BufferForLayer(layer, bufn);

                        if (rb != nullptr) {
                            RenderBuffer* oldBuffer = nullptr;
                            RenderBuffer* newBuffer = nullptr;

                            // if we are suppressing then create a fake render buffer
                            if (suppress) {
                                newBuffer = new RenderBuffer(*rb);
                                oldBuffer = rb;
                                rb = newBuffer;
                                rb->needToInit = oldBuffer->needToInit;
                                rb->infoCache = oldBuffer->infoCache;
                            }

                            auto sw = std::chrono::steady_clock::now();
                            if (effectObj != nullptr && reff->SupportsRenderCache(SettingsMap) && _renderCache.IsEnabled()) {
                                if (!effectObj->GetFrame(*rb, _renderCache)) {
                                    reff->Render(effectObj, SettingsMap, *rb);
                                    GPURenderUtils::waitForRenderCompletion(rb);
                                    effectObj->AddFrame(*rb, _renderCache);
                                }
                            }
                            else {
                                reff->Render(effectObj, SettingsMap, *rb);
                            }

                            // Log slow render frames ... this takes time but at this point it is already slow
                            auto swElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - sw).count();
                            if (swElapsed > 150) {
                                logger_render->info("Frame #{} render on model {} ({}x{}) layer {} effect {} from {}ms (#{}) to {}ms (#{}) took more than 150 ms => {}ms.", rb->curPeriod, (const char*)buffer.GetModelName().c_str(), rb->BufferWi, rb->BufferHt, layer, (const char*)reff->Name().c_str(), effectObj->GetStartTimeMS(), rb->curEffStartPer, effectObj->GetEndTimeMS(), rb->curEffEndPer, swElapsed);
                            }

                            if (suppress && oldBuffer != nullptr) {
                                oldBuffer->needToInit = rb->needToInit;
                                oldBuffer->infoCache = rb->infoCache;
                                rb->infoCache.clear();
                                delete newBuffer;
                                rb = oldBuffer;
                                newBuffer = nullptr;
                                oldBuffer = nullptr;
                            }
                        }
                        });
                    if (bufCnt > 1) {
                        if (bgThread) {
                            static ParallelJobPool PER_MODEL_POOL("per_model_pool");
                            parallel_for(0, bufCnt, [&f](int x) {f(x); }, 1, &PER_MODEL_POOL);
                        }
                        else {
                            // if we are not on the bgThread, then assume this effect cannot render on background threads
                            // and thus cannot be done in parallel
                            for (int x = 0; x < bufCnt; x++) {
                                f(x);
                            }
                        }
                    }
                    else {
                        f(0);
                    }
                    buffer.MergeBuffersForLayer(layer);
                }
            }
        }
        else {
            retval = false;
        }
    }
    else {
        retval = false;
    }
    return retval;
}
