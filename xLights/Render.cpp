/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <mutex>
#include <condition_variable>
#include <map>
#include <memory>

#include "xLightsMain.h"
#include "xLightsXmlFile.h"
#include "RenderCommandEvent.h"
#include "effects/RenderableEffect.h"
#include "RenderProgressDialog.h"
#include "SeqExportDialog.h"
#include "RenderUtils.h"
#include "models/ModelGroup.h"
#include "sequencer/MainSequencer.h"
#include "UtilFunctions.h"
#include "PixelBuffer.h"
#include "Parallel.h"
#include "ExternalHooks.h"
#include "GPURenderUtils.h"

#include <log4cpp/Category.hh>

#define END_OF_RENDER_FRAME INT_MAX

//other common strings
static const std::string STR_EMPTY("");

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
    bool returnVal{ true };
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

    AggregatorRenderer(int numFrames) : NextRenderer(), finalFrame(numFrames + 19), data(numFrames + 20) {
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


class RenderJob: public Job, public NextRenderer {
public:
    RenderJob(ModelElement *row, SequenceData &data, xLightsFrame *xframe, bool zeroBased = false)
        : Job(), NextRenderer(), rowToRender(row), seqData(&data), xLights(xframe),
            gauge(nullptr), currentFrame(0), renderLog(log4cpp::Category::getInstance(std::string("log_render"))),
            supportsModelBlending(false), abort(false), statusMap(nullptr)
    {
        name = "";
        if (row != nullptr) {
            name = row->GetModelName();
            mainBuffer = new PixelBufferClass(xframe);
            numLayers = rowToRender->GetEffectLayerCount();

            if (xframe->InitPixelBuffer(name, *mainBuffer, numLayers, zeroBased)) {
                const Model *model = mainBuffer->GetModel();
                if ("ModelGroup" == model->GetDisplayAs()) {
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
                                subModelInfos.back()->buffer.reset(new PixelBufferClass(xframe));
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
                                subModelInfos.back()->buffer.reset(new PixelBufferClass(xframe));
                                subModelInfos.back()->buffer->InitBuffer(*subModel, se->GetEffectLayerCount() + 1, data.FrameTime(), false);
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
                                        nodeBuffers[SNPair(ste->GetStrand(), n)].reset(new PixelBufferClass(xframe));
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

    wxGauge *GetGauge() const { return gauge;}
    void SetGauge(wxGauge *g) { gauge = g;}
    int GetCurrentFrame() const { return currentFrame;}
    int GetEndFrame() const { return endFrame;}
    int GetStartFrame() const { return startFrame;}

    const std::string GetName() const override {
        return name;
    }

    virtual bool DeleteWhenComplete() override {
        return false;
    }

    void LogToLogger(int logLevel) {
        // these can only be set at start time
        static bool debug = renderLog.isPriorityEnabled((log4cpp::Priority::DEBUG));
        static bool info = renderLog.isPriorityEnabled((log4cpp::Priority::INFO));

        if ((debug && logLevel == log4cpp::Priority::DEBUG) ||
            (info && logLevel == log4cpp::Priority::INFO))
        {
            renderLog.log(logLevel, (const char *)GetwxStatus().c_str());
        }
    }

    void SetGenericStatus(const wxString &msg, int frame, bool debugLog = false, bool includeStatusMap = false) {
        statusType = includeStatusMap ? 8 : 4;
        statusMsg = msg;
        statusFrame = frame;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }

    void SetGenericStatus(const char *msg, int frame, bool debugLog = false, bool includeStatusMap = false) {
        statusType = includeStatusMap ? 10 : 6;
        statusMsgChars = msg;
        statusFrame = frame;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }
    void SetGenericStatus(const wxString &msg, int frame, int layer, bool debugLog = false, bool includeStatusMap = false) {
        statusType = includeStatusMap ? 9 : 5;
        statusMsg = msg;
        statusFrame = frame;
        statusLayer = layer;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }

    void SetGenericStatus(const char *msg, int frame, int layer, bool debugLog = false, bool includeStatusMap = false) {
        statusType = includeStatusMap ? 11 : 7;
        statusMsgChars = msg;
        statusFrame = frame;
        statusLayer = layer;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }

    wxString PctSafe(const wxString& s) {
        wxString res(s);
        res.Replace("%", "%%");
        return res;
    }

    wxString PrintStatusMap() {
        if (statusMap == nullptr) return "";
        return PctSafe(statusMap->AsString());
    }

    void SetRenderingStatus(int frame, SettingsMap*map, int layer, int submodel, int strand, int node, bool debugLog = false) {
        statusType = 2;
        statusFrame = frame;
        statusLayer = layer;
        statusStrand = strand;
        statusSubmodel = submodel;
        statusNode = node;
        statusMap = map;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }

    void SetCalOutputStatus(int frame, int submodel, int strand, int node, bool debugLog = true) {
        statusType = 3;
        statusFrame = frame;
        statusStrand = strand;
        statusSubmodel = submodel;
        statusNode = node;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }

    void SetInializingStatus(int frame, int layer, int submodel, int strand, int node, bool debugLog = false) {
        statusType = 1;
        statusFrame = frame;
        statusLayer = layer;
        statusStrand = strand;
        statusSubmodel = submodel;
        statusNode = node;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }
    void SetWaitingStatus(int frame, bool debugLog = false) {
        statusType = 13;
        statusFrame = frame;
    }
    void SetStatus(const wxString &st, bool debugLog = false) {
        statusMsg = st;
        statusType = 0;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }

    void SetStatus(const char *st, bool debugLog = false) {
        statusMsgChars = st;
        statusType = 12;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }

    std::string GetStatus() override {
        return GetwxStatus().ToStdString();
    }

    wxString GetStatusForUser()
    {
        int lastIdx = 0;
        int submodel = -1;
        if (statusType >= 1 && statusType <= 3) {
            submodel = statusSubmodel;
        }
        Effect* effect = findEffectForFrame(this->statusLayer, GetCurrentFrame(), submodel, lastIdx);

        if (effect != nullptr) {
            std::string mname = "";
            if (submodel >= 0) {
                mname = "Submodel: " + subModelInfos[submodel]->element->GetName() + " ";
            }
            return wxString::Format("%sEffect: %s Start: %s End %s", mname.c_str(), effect->GetEffectName().c_str(),
                                    FORMATTIME(effect->GetStartTimeMS()), FORMATTIME(effect->GetEndTimeMS()));
        }
        if (statusType == 13) {
            return wxString::Format("Waiting to start frame %d", statusFrame);
        }

        return "";
    }

    wxString GetwxStatus() {
        std::string n = (statusSubmodel == -1 || statusSubmodel >= subModelInfos.size()) ? name : subModelInfos[statusSubmodel]->element->GetFullName();
        switch (statusType) {
        case 0:
            return statusMsg;
        case 1:
            if (statusStrand == -1) {
                return wxString::Format("Initializing effect at frame %d for %s, layer %d", statusFrame, n, statusLayer);
            } else if (statusNode == -1) {
                return wxString::Format("Initializing strand effect at frame %d for %s, strand %d", statusFrame, n, statusStrand);
            } else {
                return wxString::Format("Initializing node effect at frame %d for %s, strand %d, node %d", statusFrame, n, statusStrand, statusNode);
            }
        case 2:
            if (statusStrand == -1) {
                return wxString::Format("Rendering layer effect for frame %d of %s, layer %d: ", statusFrame, n, statusLayer) + PrintStatusMap();
            } else if (statusNode == -1) {
                return wxString::Format("Rendering strand effect for frame %d of %s, strand %d: ", statusFrame, n, statusStrand) + PrintStatusMap();
            } else {
                return wxString::Format("Rendering node effect for frame %d of %s, strand %d, node %d: ", statusFrame, n, statusLayer, statusNode) + PrintStatusMap();
            }
        case 3:
            if (statusStrand == -1) {
                return wxString::Format("Calculating output at frame %d for %s: ", statusFrame, n) + PrintStatusMap();
            } else if (statusNode == -1) {
                return wxString::Format("Calculating output at frame %d for %s, strand %d: ", statusFrame, n, statusStrand) + PrintStatusMap();
            } else {
                return wxString::Format("Calculating output at frame %d for %s, strand %d, node %d: ", statusFrame, n, statusStrand, statusNode) + PrintStatusMap();
            }
        case 4:
            return wxString::Format(statusMsg, name, statusFrame);
        case 5:
            return wxString::Format(statusMsg, name, statusFrame, statusLayer);
        case 6:
            return wxString::Format(statusMsgChars, name, statusFrame);
        case 7:
            return wxString::Format(statusMsgChars, name, statusFrame, statusLayer);
        case 8:
            return wxString::Format(statusMsg, name, statusFrame) + PrintStatusMap();
        case 9:
            return wxString::Format(statusMsg, name, statusFrame, statusLayer) + PrintStatusMap();
        case 10:
            return wxString::Format(statusMsgChars, name, statusFrame) + PrintStatusMap();
        case 11:
            return wxString::Format(statusMsgChars, name, statusFrame, statusLayer) + PrintStatusMap();
        case 12:
            return statusMsgChars;
        case 13:
            return wxString::Format("Waiting to start frame %d for %s", statusFrame, n);
        }
        return statusMsg;
    }

    SequenceData *createExportBuffer() {
        SequenceData *sb = new SequenceData();
        sb->init(xLights->GetModel(mainBuffer->GetModelName())->GetActChanCount(), seqData->NumFrames(), seqData->FrameTime(), false);
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
        wxStopWatch sw;
        bool effectsToUpdate = false;
        Effect* tempEffect = nullptr;
        int numLayers = el->GetEffectLayerCount();

        for (int x = 0; x < info.validLayers.size(); x++) {
            info.validLayers[x] = false;
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
                if (buffer->IsCanvasMix(layer) && layer < numLayers - 1) {
                    maybeWaitForFrame(frame);

                    auto vl = info.validLayers;
                    if (info.settingsMaps[layer].Get("LayersSelected", "") != "") {
                        // remove from valid layers any layers we dont need to include
                        wxArrayString ls = wxSplit(info.settingsMaps[layer].Get("LayersSelected", ""), '|');
                        for (int i = layer + 1; i < vl.size(); i++) {
                            if (vl[i]) {
                                bool found = false;
                                for (auto it = ls.begin(); !found && it != ls.end(); ++it) {
                                    if (wxAtoi(*it) + layer + 1 == i) {
                                        found = true;
                                    }
                                }
                                if (!found) {
                                    vl[i] = false;
                                }
                            }
                        }
                    }

                    // preload the buffer with the output from the lower layers
                    RenderBuffer& rb = buffer->BufferForLayer(layer, -1);

                    // I have to calc the output here to apply blend, rotozoom and transitions
                    buffer->CalcOutput(frame, vl, layer);
                    std::vector<uint8_t> done(rb.GetPixelCount());
                    rb.CopyNodeColorsToPixels(done);
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

                info.validLayers[layer] = xLights->RenderEffectFromMap(suppress, ef, layer, frame, info.settingsMaps[layer], *buffer, b, true, &renderEvent);
                effectsToUpdate |= info.validLayers[layer];
                info.effectStates[layer] = b;

                if (suppress) {
                    info.validLayers[layer] = false;
                } else {
                    buffer->HandleLayerBlurZoom(frame, layer);
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
            if (blend) {
                buffer->SetColors(numLayers, &((*seqData)[frame][0]));
                info.validLayers[numLayers] = true;
            }
            buffer->CalcOutput(frame, info.validLayers);
            buffer->GetColors(&((*seqData)[frame][0]), rangeRestriction);
        }

        if (sw.Time() > 500) {
            RenderBuffer& b = buffer->BufferForLayer(0, -1);
            renderLog.info("*** Frame #%d at %dms render on model %s (%dx%d) took more than 1/2s => %dms.", frame, frame * b.frameTimeInMs, (const char *)el->GetName().c_str(), b.BufferWi, b.BufferHt, sw.Time());
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
            SetGenericStatus("%s: Processing frame %d ", frame, true, true);
        }
    }
    virtual void Process() override {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        static log4cpp::Category& logger_jobpool = log4cpp::Category::getInstance(std::string("log_jobpool"));
        logger_jobpool.debug("Render job thread id 0x%x or %d", wxThread::GetCurrentId(), wxThread::GetCurrentId());

        SetGenericStatus("Initializing rendering thread for %s", 0);
        int origChangeCount;
        int ss, es;

        rowToRender->IncWaitCount();
        std::unique_lock<std::recursive_timed_mutex> lock(rowToRender->GetRenderLock());
        if (rowToRender->DecWaitCount() && !HasNext()) {
            // other threads for this model waiting, we'll bail fast and let them handle this
            renderLog.debug("Rendering thread exiting early.");
            currentFrame = END_OF_RENDER_FRAME; // this is needed otherwise the job does not look done
            return;
        }
        SetGenericStatus("Got lock on rendering thread for %s", 0);

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
                SetGenericStatus("Finding starting effect for %s, startFrame %d, and layer %d ", (int)startFrame, layer, false, true);
                EffectLayer *elayer = rowToRender->GetEffectLayer(layer);
                std::unique_lock<std::recursive_mutex> elock(elayer->GetLock());
                mainModelInfo.currentEffects[layer] = findEffectForFrame(elayer, startFrame, mainModelInfo.currentEffectIdxs[layer]);
                SetGenericStatus("Initializing starting effect for %s, startFrame %d, and layer %d ", (int)startFrame, layer, false, true);
                initialize(layer, startFrame, mainModelInfo.currentEffects[layer], mainModelInfo.settingsMaps[layer], mainBuffer);
                mainModelInfo.effectStates[layer] = true;
            }

            for (int frame = startFrame; frame <= endFrame; ++frame) {
                currentFrame = frame;
                SetGenericStatus("%s: Starting frame %d ", frame, true, true);

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
                            logger_base.crit("RenderJob::Process PixelBufferPointer is null ... this is going to crash.");
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
                        if (xLights->RenderEffectFromMap(false, el, 0, frame, nodeSettingsMaps[node], *buffer, nodeEffectStates[node], true, &renderEvent)) {
                            SetCalOutputStatus(frame, -1, strand, inode);
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
                    SetGenericStatus("%s: Notifying next renderer of frame %d done", frame, true);
                    FrameDone(frame);
                }
            }
            SetGenericStatus("%s: All done - Completed frame %d ", endFrame, true, false);
        } catch ( std::exception &ex) {
            wxASSERT(false); // so when we debug we catch them
            printf("Caught an exception %s", ex.what());
			renderLog.error("Caught an exception on rendering thread: " + std::string(ex.what()));
            logger_base.error("Caught an exception on rendering thread: %s", ex.what());
		} catch ( ... ) {
            wxASSERT(false); // so when we debug we catch them
            printf("Caught an unknown exception");
			renderLog.error("Caught an unknown exception on rendering thread.");
            logger_base.error("Caught an unknown exception on rendering thread.");
        }
        if (HasNext()) {
            //make sure the previous has told us we're at the end.  If we return before waiting, the previous
            //may try sending the END_OF_RENDER_FRAME to us and we'll have been deleted
            SetGenericStatus("%s: Waiting on previous renderer for final frame", 0, true);
            waitForFrame(END_OF_RENDER_FRAME);

            //let the next know we're done
            SetGenericStatus("%s: Notifying next renderer of final frame", 0, true);
            FrameDone(END_OF_RENDER_FRAME);
            xLights->CallAfter(&xLightsFrame::SetStatusText, wxString("Done Rendering \"" + rowToRender->GetModelName() + "\""), 0);
            SetGenericStatus("%s: All done - Completed frame %d ", endFrame, true, false);
        } else {
            xLights->CallAfter(&xLightsFrame::RenderDone);
        }
        rowToRender->CleanupAfterRender();
        currentFrame = END_OF_RENDER_FRAME;
        //printf("Done rendering %lx (next %lx)\n", (unsigned long)this, (unsigned long)next);
		renderLog.debug("Rendering thread exiting.");
	}

    void AbortRender() {
        abort = true;
    }

    ModelElement* GetModelElement() const { return rowToRender; }

private:

    void initialize(int layer, int frame, Effect *el, SettingsMap &settingsMap, PixelBufferClass *buffer) {
        if (el == nullptr || el->GetEffectIndex() == -1) {
            settingsMap.clear();
        } else {
            loadSettingsMap(el->GetEffectName(),
                            el,
                            settingsMap);
        }
        buffer->SetLayerSettings(layer, settingsMap);
        if (el != nullptr) {
            xlColorVector newcolors;
            xlColorCurveVector newcc;
            el->CopyPalette(newcolors, newcc);
            buffer->SetPalette(layer, newcolors, newcc);
            buffer->SetTimes(layer, el->GetStartTimeMS(), el->GetEndTimeMS());
        }
    }

    Effect* findEffectForFrame(const wxString& model, const int layer, int frame)
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
    xLightsFrame *xLights;
    SequenceData *seqData;
    std::vector<bool> rangeRestriction;
    bool supportsModelBlending;
    RenderEvent renderEvent;

    //stuff for handling the status;
    wxString statusMsg;
    const char *statusMsgChars;
    volatile int statusType;
    volatile int statusFrame;
    SettingsMap *statusMap;
    volatile int statusLayer;
    volatile int statusSubmodel = -1;
    volatile int statusStrand = -1;
    volatile int statusNode = -1;
    log4cpp::Category &renderLog;

    wxGauge *gauge;
    std::atomic_int currentFrame;
    std::atomic_bool abort;

    std::vector<EffectLayerInfo *> subModelInfos;

    std::map<SNPair, PixelBufferClassPtr> nodeBuffers;
};


IMPLEMENT_DYNAMIC_CLASS(RenderCommandEvent, wxCommandEvent)
IMPLEMENT_DYNAMIC_CLASS(SelectedEffectChangedEvent, wxCommandEvent)

void xLightsFrame::RenderRange(RenderCommandEvent &evt) {
    if (evt.deleted) {
        selectedEffect = 0;
    }
    if (evt.model == "") {
        if( (evt.start != -1) && (evt.end != -1) ) {
            RenderTimeSlice(evt.start, evt.end, evt.clear);
        } else {
            //render all dirty models
            if (!_suspendRender)
                RenderDirtyModels();
        }
    } else {
        if (!_suspendRender)
            RenderEffectForModel(evt.model, evt.start,  evt.end, evt.clear);
    }
}

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
    if (_sequenceElements.IsValidEffect(ev->effect))
    {
        ev->returnVal = RenderEffectFromMap(ev->suppress, ev->effect,
            ev->layer,
            ev->period,
            *ev->settingsMap,
            *ev->buffer, *ev->ResetEffectState, false, ev);
    }
    else
    {
        wxASSERT(false);
    }
    ev->signal.notify_all();
}

class RenderProgressInfo {
public:
    RenderProgressInfo(std::function<void(bool)>&& cb) : callback(cb)
    {
        numRows = 0;
        startFrame = 0;
        endFrame = 0;
        jobs = nullptr;
        aggregators = nullptr;
        renderProgressDialog = nullptr;
    };
    std::function<void(bool)> callback;
    int numRows;
    int startFrame;
    int endFrame;
    RenderJob **jobs;
    AggregatorRenderer **aggregators;
    RenderProgressDialog *renderProgressDialog;
    std::list<Model *> restriction;
};

void xLightsFrame::LogRenderStatus()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Logging render status ***************");
    logger_base.debug("Render tree size. %d entries.", renderTree.data.size());
    logger_base.debug("Render Thread status:\n%s", (const char *)GetThreadStatusReport().c_str());
    for (const auto& it : renderProgressInfo) {
        int frames = it->endFrame - it->startFrame + 1;
        logger_base.debug("Render progress rows %d, start frame %d, end frame %d, frames %d.", it->numRows, it->startFrame, it->endFrame, frames);
        for (int i = 0; i < it->numRows; i++) {
            if (it->jobs[i] != nullptr) {
                auto job = it->jobs[i];
                int curFrame = job->GetCurrentFrame();
                if (curFrame > it->endFrame || curFrame == END_OF_RENDER_FRAME) {
                    curFrame = it->endFrame;
                }

                logger_base.debug("    Progress %s - %ld%%.", (const char *)job->GetName().c_str(), (long)(curFrame - it->startFrame + 1) * 100 / frames);
                std::string su = job->GetStatusForUser();
                if (!su.empty()) {
                    logger_base.debug("             %s.", (const char *)su.c_str());
                }
                su = job->GetStatus();
                if (!su.empty()) {
                    logger_base.debug("             %s.", (const char *)su.c_str());
                }

                auto row = job->GetModelElement();
                if (row != nullptr) {
                    bool blocked = job->GetwxStatus().StartsWith("Initializing rendering thread");
                    if (blocked || row->GetWaitCount()) {
                        logger_base.debug("             Element %s, Blocked %d, Wait Count %d.",
                                          (const char *)row->GetModelName().c_str(), blocked,
                                          row->GetWaitCount());
                    }
                }
            }
        }
    }
    logger_base.debug("*************************************");
}

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

void xLightsFrame::OnProgressBarDoubleClick(wxMouseEvent &evt) {
    if (renderProgressInfo.empty()) {
        return;
    }
    for (auto it : renderProgressInfo) {
        if (it->renderProgressDialog) {
            it->renderProgressDialog->Show();
            return;
        }
    }
}
void xLightsFrame::OnRenderStatusTimerTrigger(wxTimerEvent& event) {
    UpdateRenderStatus();
}

void xLightsFrame::UpdateRenderStatus() {
    if (renderProgressInfo.empty()) {
        RenderStatusTimer.Stop();
        return;
    }

    RenderMainThreadEffects();

    for (auto it = renderProgressInfo.begin(); it != renderProgressInfo.end();) {
        int countModels = 0;
        int countFrames = 0;

        bool done = true;
        RenderProgressInfo *rpi = *it;
        bool shown = rpi->renderProgressDialog == nullptr ? false : rpi->renderProgressDialog->IsShown();

        int frames = rpi->endFrame - rpi->startFrame + 1;
        if( frames <= 0 ) frames = 1;
        for (size_t row = 0; row < rpi->numRows; ++row) {

            if (rpi->jobs[row]) {
                int i = rpi->jobs[row]->GetCurrentFrame();
                if (i > rpi->jobs[row]->GetEndFrame()) {
                    i = END_OF_RENDER_FRAME;
                }
                if (i != END_OF_RENDER_FRAME) {
                    done = false;
                }
                if (rpi->jobs[row]->GetEndFrame() > rpi->endFrame) {
                    frames += rpi->jobs[row]->GetEndFrame() - rpi->endFrame;
                }
                if (rpi->jobs[row]->GetStartFrame() < rpi->startFrame) {
                    frames += rpi->startFrame - rpi->jobs[row]->GetStartFrame();
                }
                ++countModels;
                if (i == END_OF_RENDER_FRAME) {
                    countFrames += rpi->jobs[row]->GetEndFrame() - rpi->jobs[row]->GetStartFrame() + 1;
                    if (shown) {
                        wxGauge *g = rpi->jobs[row]->GetGauge();
                        if (g != nullptr && g->GetValue() != 100) {
                            g->SetValue(100);
                        }
                    }
                } else {
                    i -= rpi->jobs[row]->GetStartFrame();
                    if (shown) {
                        int val = (i > rpi->endFrame) ? 100 : (100 * i)/frames;
                        wxGauge *g = rpi->jobs[row]->GetGauge();
                        if (g != nullptr && g->GetValue() != val) {
                            g->SetValue(val);
                            g->SetToolTip(rpi->jobs[row]->GetStatusForUser());
                        }
                    }
                    countFrames += i;
                }
            }
        }
        if (countFrames > 0 && countModels > 0) {
            int pct = (countFrames * 80) / (countModels * frames);
            static int lastVal = 0;
            if (lastVal != pct) {
                if (ProgressBar->GetValue() != (10 + pct)) {
                    ProgressBar->SetValue(10 + pct);
                    _appProgress->SetValue(10 + pct);
                }
                lastVal = pct;
            }
        }

        if (done) {
            for (size_t row = 0; row < rpi->numRows; ++row) {
                if (rpi->jobs[row]) {
                    delete rpi->jobs[row];
                }
                delete rpi->aggregators[row];
            }
            if (rpi->renderProgressDialog) {
                delete rpi->renderProgressDialog;
                rpi->renderProgressDialog = nullptr;
            }
            _appProgress->SetValue(0);
            _appProgress->Reset();
            RenderDone();
            delete []rpi->jobs;
            delete []rpi->aggregators;
            rpi->callback(abortedRenderJobs > 0);
            delete rpi;
            rpi = nullptr;
            it = renderProgressInfo.erase(it);
        } else {
            ++it;
        }
    }
}

void xLightsFrame::RenderDone()
{
    mainSequencer->PanelEffectGrid->Refresh();
}

class RenderTreeData {
public:
    RenderTreeData(Model *e): model(e) {

        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        if (e == nullptr) {
            logger_base.crit("Render tree has a null model ... this is not going to end well.");
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
    static log4cpp::Category &logger_render = log4cpp::Category::getInstance(std::string("log_render"));
    logger_render.debug("========== RENDER TREE");
    for (const auto& it : data) {
        //printf("%s:   (%d)\n", (*it)->model->GetName().c_str(), (int)(*it)->ranges.size());
        logger_render.debug("   %s:   (%d)", (const char *)it->model->GetName().c_str(), (int)it->ranges.size());
        for (const auto& it2 : it->renderOrder) {
            //printf("    %s     \n", it2->GetName().c_str());
            logger_render.debug("        %s", (const char *)it2->GetName().c_str());
        }
    }
    logger_render.debug("========== END RENDER TREE");
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
        for (size_t row = 0; row < numEls; ++row) {
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
                          bool progressDialog, bool clear,
                          std::function<void(bool)>&& callback)
{
    abortedRenderJobs = 0;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static log4cpp::Category &logger_render = log4cpp::Category::getInstance(std::string("log_render"));

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
            //logger_base.crit("xLightsFrame::Render rowEl is nullptr ... this is going to crash looking for '%s'.", (const char *)(*it)->GetName().c_str());
        } else {
            if (rowEl->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                ModelElement *me = dynamic_cast<ModelElement *>(rowEl);

                if (me == nullptr) {
                    logger_base.crit("xLightsFrame::Render me is nullptr ... this is going to crash.");
                }

                bool hasEffects = HasEffects(me);
                bool isRestricted = std::find(restrictToModels.begin(), restrictToModels.end(), *it) != restrictToModels.end();
                if (hasEffects || (isRestricted && clear)) {
                    RenderJob *job = new RenderJob(me, seqData, this, false);

                    if (job == nullptr) {
                        logger_base.crit("xLightsFrame::Render job is nullptr ... this is going to crash.");
                    }

                    job->setRenderRange(startFrame, endFrame);
                    job->SetRangeRestriction(ranges);
                    if (seqElements.SupportsModelBlending()) {
                        job->SetModelBlending();
                    }
                    PixelBufferClass *buffer = job->getBuffer();
                    if (buffer == nullptr) {
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
                                    if (idx != row) {
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

    logger_render.debug("Aggregators created.");

    channelMaps.clear();
    RenderProgressDialog *renderProgressDialog = nullptr;
    if (progressDialog) {
        renderProgressDialog = new RenderProgressDialog(this);
    }
    unsigned int count = 0;
    if (clear) {
        for (int f = startFrame; f <= endFrame; f++) {
            for (const auto& it : ranges) {
                seqData[f].Zero(it.start, it.end - it.start + 1);
            }
        }
    }

    logger_render.debug("Data cleared.");

    for (row = 0; row < numRows; ++row) {
        if (jobs[row]) {
            if (aggregators[row]->getNumAggregated() == 0) {
                //start all the jobs that don't depend on anything above them
                //get them rendering while we setup the rest
                jobs[row]->setPreviousFrameDone(END_OF_RENDER_FRAME);
                jobPool.PushJob(jobs[row]);
                ++count;
            }
            if (progressDialog) {
                wxStaticText *label = new wxStaticText(renderProgressDialog->scrolledWindow, wxID_ANY, jobs[row]->GetName());
                renderProgressDialog->scrolledWindowSizer->Add(label,1, wxALL |wxEXPAND,3);
                wxGauge *g = new wxGauge(renderProgressDialog->scrolledWindow, wxID_ANY, 100);
                g->SetValue(0);
                g->SetMinSize(wxSize(200, -1));
                renderProgressDialog->scrolledWindowSizer->Add(g, 1, wxALL |wxEXPAND,3);
                jobs[row]->SetGauge(g);
            }
        }
    }

    logger_render.debug("Job pool start size %d.", (int)jobPool.size());
    for (row = 0; row < numRows; ++row) {
        if (jobs[row] && aggregators[row]->getNumAggregated() != 0) {
            //now start the rest
            jobPool.PushJob(jobs[row]);
            ++count;
        }
    }
    logger_base.debug("Job pool new size %d.", (int)jobPool.size());

    if (count) {
        if (progressDialog) {
            renderProgressDialog->SetSize(450, 400);
            renderProgressDialog->scrolledWindow->SetSizer(renderProgressDialog->scrolledWindowSizer);
            renderProgressDialog->scrolledWindow->FitInside();
            renderProgressDialog->scrolledWindow->SetScrollRate(5, 5);
        }

        RenderProgressInfo *pi = new RenderProgressInfo(std::move(callback));
        pi->numRows = numRows;
        pi->startFrame = startFrame;
        pi->endFrame = endFrame;
        pi->jobs = jobs;
        pi->renderProgressDialog = renderProgressDialog;
        pi->restriction = restrictToModels;
        pi->aggregators = aggregators;

        renderProgressInfo.push_back(pi);
        RenderStatusTimer.Start(100, false);
    } else {
        callback(abortedRenderJobs > 0);
        if (progressDialog) {
            delete renderProgressDialog;
        }
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
    Render(_sequenceElements, _seqData, models, restricts, startframe, endframe, false, true, [] (bool) {});
}

bool xLightsFrame::AbortRender(int maxTimeMS, int* numThreadsAborted)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static bool inAbort = false;
    if (inAbort) {
        return false;
    }
    inAbort = true;
    logger_base.info("Aborting rendering ...");
    int abortCount = 0;
    for (auto rpi : renderProgressInfo) {
        //abort whatever is rendering
        for (size_t row = 0; row < rpi->numRows; ++row) {
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
    logger_base.info("Aborting %d renderers", abortCount);
    int loops = 0;
    while (!renderProgressInfo.empty() && loops < maxLoops) {
        loops++;
        RenderMainThreadEffects(); // make sure main thread effects are rendered
        wxMilliSleep(10);
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
            logger_base.info("    Waiting for renderers to abort. %d left.", (int)renderProgressInfo.size());
        }
    }
    logger_base.info("    Aborting renderers ... Done");
    inAbort = false;
    if( numThreadsAborted != nullptr ) {
        *numThreadsAborted = abortCount;
    }
    return renderProgressInfo.empty();
}

void xLightsFrame::RenderGridToSeqData(std::function<void(bool)>&& callback) {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    BuildRenderTree();
    if (renderTree.data.empty()) {
        //nothing to do....
        callback(false);
        return;
    }

    logger_base.debug("Render tree built. %d entries.", renderTree.data.size());

    const int numRows = _sequenceElements.GetElementCount();
    if (numRows == 0) {
        callback(false);
        return;
    }
    std::list<Model *> models;
    for (const auto& it : renderTree.data) {
        models.push_back(it->model);
    }
    for (auto it : renderProgressInfo) {
        //we're going to render EVERYTHING, abort whatever is rendering
        for (size_t row = 0; row < it->numRows; ++row) {
            if (it->jobs[row]) {
               it->jobs[row]->AbortRender();
            }
        }
    }
    std::list<Model*> restricts;

    logger_base.debug("Rendering %d models %d frames.", models.size(), _seqData.NumFrames());


#ifdef DOTIMING
    wxStopWatch sw;
    Render(_sequenceElements, _seqData, models, restricts, 0, SeqData.NumFrames() - 1, true, false, [this, models, restricts, sw, callback] {
        printf("%s  Render 1:  %ld ms\n", (const char *)xlightsFilename.c_str(), sw.Time());
        wxStopWatch sw2;
        Render(_sequenceElements, _seqData, models, restricts, 0, SeqData.NumFrames() - 1, true, false, [this, models, restricts, sw2, callback] {
            printf("%s  Render 2:  %ld ms\n", (const char *)xlightsFilename.c_str(), sw2.Time());
            wxStopWatch sw3;
            Render(_sequenceElements, _seqData, models, restricts, 0, SeqData.NumFrames() - 1, true, false, [sw3, callback] {
                printf("%s  Render 3:  %ld ms\n", (const char *)xlightsFilename.c_str(), sw3.Time());
                callback(abortedRenderJobs > 0);
            } );
        });
    });
#else
    Render(_sequenceElements, _seqData, models, restricts, 0, _seqData.NumFrames() - 1, true, false, std::move(callback));
#endif
}

void xLightsFrame::RenderEffectForModel(const std::string &model, int startms, int endms, bool clear) {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_suspendRender) return;

    BuildRenderTree();

    logger_base.debug("Render tree built for model %s %dms-%dms. %d entries.",
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
                    for (size_t row = 0; row < rpi->numRows; ++row) {
                        if (rpi->jobs[row]) {
                            rpi->jobs[row]->AbortRender();
                        }
                    }
                }
            }
            std::list<Model *> m;
            m.push_back(it->model);

            logger_base.debug("Rendering %d models %d frames.", m.size(), endframe - startframe + 1);

            Render(_sequenceElements, _seqData, it->renderOrder, m, startframe, endframe, false, true, [] (bool) {});
        }
    }
}

void xLightsFrame::RenderTimeSlice(int startms, int endms, bool clear) {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    BuildRenderTree();
    logger_base.debug("Render tree built for time slice %dms-%dms. %d entries.",
        startms,
        endms,
        renderTree.data.size());

    if (renderTree.data.empty()) {
        //nothing to do....
        return;
    }
    const int numRows = _sequenceElements.GetElementCount();
    if (numRows == 0) {
        return;
    }
    std::list<Model *> models;
    std::list<Model *> restricts;
    for (auto it = renderTree.data.begin(); it != renderTree.data.end(); ++it) {
        models.push_back((*it)->model);
    }
    if (startms < 0) {
        startms = 0;
    }
    if (endms < 0) {
        endms = 0;
    }
    int startframe = startms / _seqData.FrameTime() - 1;
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

    EnableSequenceControls(false);
    mRendering = true;
    ProgressBar->Show();
    GaugeSizer->Layout();
    SetStatusText(_("Rendering all layers for time slice"));
    ProgressBar->SetValue(0);
    _appProgress->SetValue(0);
    _appProgress->Reset();
    wxStopWatch sw; // start a stopwatch timer
    Render(_sequenceElements, _seqData, models, restricts, startframe, endframe, true, clear, [this, sw] (bool aborted) {
        static log4cpp::Category &logger_base2 = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base2.info("   Effects done.");
        ProgressBar->SetValue(100);
        float elapsedTime = sw.Time()/1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
        wxString displayBuff = wxString::Format(_("Rendered in %7.3f seconds"),elapsedTime);
        CallAfter(&xLightsFrame::SetStatusText, displayBuff, 0);
        mRendering = false;
        EnableSequenceControls(true);
        ProgressBar->Hide();
        _appProgress->SetValue(0);
        _appProgress->Reset();
        GaugeSizer->Layout();
    });
}

bool xLightsFrame::DoExportModel(unsigned int startFrame, unsigned int endFrame, const std::string& model, const std::string& fn, const std::string& fmt, bool doRender)
{
    if (endFrame == 0)
        endFrame = _seqData.NumFrames();

    Model* m = GetModel(model);
    if (m == nullptr)
        return false;

    if (m->GetDisplayAs() == "ModelGroup")
        return false;

    wxString filename(fn);
    wxString format(fmt);

    wxStopWatch sw;
    wxString Out3 = format.Left(3);

    if (Out3 == "LSP") {
        filename = filename + "_USER";
    }
    wxFileName oName(filename);

    if (oName.GetPathWithSep() == "") {
        oName.SetPath(CurrentDir);
    }
    wxString fullpath;

    SetStatusText(wxString::Format("Starting Export for %s - %s", format, Out3));
    wxYield();

    NextRenderer wait;
    Element* el = _sequenceElements.GetElement(model);
    if (el == nullptr)
        return false;
    RenderJob* job = new RenderJob(dynamic_cast<ModelElement*>(el), _seqData, this, true);
    wxASSERT(job != nullptr);
    SequenceData* data = job->createExportBuffer();
    wxASSERT(data != nullptr);
    int cpn = job->getBuffer()->GetChanCountPerNode();

    if (doRender) {
        // always render the whole model
        job->setRenderRange(0, _seqData.NumFrames());
        job->setPreviousFrameDone(END_OF_RENDER_FRAME);
        job->addNext(&wait);
        jobPool.PushJob(job);
        //wait to complete
        while (!wait.checkIfDone(_seqData.NumFrames())) {
            wxYield();
        }
    } else {
        Model* m2 = GetModel(model);
        for (size_t frame = 0; frame < _seqData.NumFrames(); ++frame) {
            for (size_t x = 0; x < job->getBuffer()->GetNodeCount(); ++x) {
                //chan in main buffer
                int ostart = m2->NodeStartChannel(x);
                int nstart = job->getBuffer()->NodeStartChannel(x);
                //copy to render buffer for export
                job->getBuffer()->SetNodeChannelValues(x, &_seqData[frame][ostart]);
                job->getBuffer()->GetNodeChannelValues(x, &((*data)[frame][nstart]));
            }
        }
    }
    delete job;

    if (Out3 == "Lcb") {
        oName.SetExt(_("lcb"));
        fullpath = oName.GetFullPath();
        int lcbVer = 1;
        if (format.Contains("S5")) {
            lcbVer = 2;
        }
        WriteLcbFile(fullpath, data->NumChannels(), startFrame, endFrame, data, lcbVer, cpn);
    } else if (Out3 == "Vir") {
        oName.SetExt(_("vir"));
        fullpath = oName.GetFullPath();
        WriteVirFile(fullpath, data->NumChannels(), startFrame, endFrame, data);
    } else if (Out3 == "LSP") {
        oName.SetExt(_("xml"));
        fullpath = oName.GetFullPath();
        WriteLSPFile(fullpath, data->NumChannels(), startFrame, endFrame, data, cpn);
    } else if (Out3 == "HLS") {
        oName.SetExt(_("hlsnc"));
        fullpath = oName.GetFullPath();
        WriteHLSFile(fullpath, data->NumChannels(), startFrame, endFrame, data);
    } else if (Out3 == "FPP") {
        int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
        oName.SetExt(_("eseq"));
        fullpath = oName.GetFullPath();
        bool v2 = format.Contains("Compressed");
        WriteFalconPiModelFile(fullpath, data->NumChannels(), startFrame, endFrame, data, stChan, data->NumChannels(), v2);
    } else if (Out3 == "Com") {
        int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
        oName.SetExt(_("mp4"));
        fullpath = oName.GetFullPath();
        WriteVideoModelFile(fullpath, data->NumChannels(), startFrame, endFrame, data, stChan, data->NumChannels(), GetModel(model), true);
    } else if (Out3 == "Unc") {
        int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
        fullpath = oName.GetFullPath();
        WriteVideoModelFile(fullpath, data->NumChannels(), startFrame, endFrame, data, stChan, data->NumChannels(), GetModel(model), false);
    } else if (Out3 == "Min") {
        int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
        oName.SetExt(_("bin"));
        fullpath = oName.GetFullPath();
        WriteMinleonNECModelFile(fullpath, data->NumChannels(), startFrame, endFrame, data, stChan, data->NumChannels(), GetModel(model));
    } else if (Out3 == "GIF") {
        int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
        oName.SetExt(_("gif"));
        fullpath = oName.GetFullPath();
        WriteGIFModelFile(fullpath, data->NumChannels(), startFrame, endFrame, data, stChan, data->NumChannels(), GetModel(model), _seqData.FrameTime());
    }
    float s = sw.Time();
    s /= 1000;
    SetStatusText(wxString::Format("Finished writing model: %s in %0.3fs ", fullpath, s));

    delete data;
    EnableSequenceControls(true);

    return true;
}

void xLightsFrame::ExportModel(wxCommandEvent& command)
{
    unsigned int startFrame = 0;
    unsigned int endFrame = _seqData.NumFrames();
    if (command.GetString().Contains('|')) {
        auto as = wxSplit(command.GetString(), '|');
        if (as.size() == 3) {
            startFrame = wxAtoi(as[1]);
            endFrame = wxAtoi(as[2]);
        }
    }

    std::string model = command.GetString().BeforeFirst('|').ToStdString();
    Model* m = GetModel(model);
    if (m == nullptr)
        return;

    bool isgroup = (m->GetDisplayAs() == "ModelGroup");

    bool isboxed = false;
    if (dynamic_cast<ModelWithScreenLocation<BoxedScreenLocation>*>(m) != nullptr) {
        // line models, arches etc make no sense for videos
        isboxed = true;
    }

    SeqExportDialog dialog(this, m->GetName());
    dialog.ModelExportTypes(isgroup || !isboxed);
    dialog.SetExportType(command.GetString().Contains('|'), command.GetInt() == 1);

    if (dialog.ShowModal() == wxID_OK) {
        wxString filename = dialog.TextCtrlFilename->GetValue();
        ObtainAccessToURL(filename);
        EnableSequenceControls(false);
        wxString format = dialog.ChoiceFormat->GetStringSelection();

        DoExportModel(startFrame, endFrame, model, filename, format, command.GetInt() == 1);
    }
}

bool xLightsFrame::RenderEffectFromMap(bool suppress, Effect* effectObj, int layer, int period, SettingsMap& SettingsMap,
    PixelBufferClass& buffer, bool& resetEffectState,
    bool bgThread, RenderEvent* event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static log4cpp::Category& logger_render = log4cpp::Category::getInstance(std::string("log_render"));

    // dont render disabled effects
    if (effectObj == nullptr) return false;
    if (effectObj->IsRenderDisabled()) return false;

    if (layer >= buffer.GetLayerCount()) {
        logger_base.error("Model %s Effect %s at frame %d tried to render on a layer %d that does not exist (Only %d found).",
            (const char*)buffer.GetModel()->GetName().c_str(), (const char*)effectObj->GetEffectName().c_str(), period, layer + 1, buffer.GetLayerCount());
        wxASSERT(false);
        return false;
    }

    if (buffer.BufferForLayer(layer, -1).BufferHt == 0 || buffer.BufferForLayer(layer, -1).BufferWi == 0) {
        return false;
    }

    if (buffer.GetModel() != nullptr && buffer.GetModel()->GetNodeCount() == 0) {
        // this happens with custom models with no nodes defined
        if (buffer.BufferForLayer(layer, 0).curEffStartPer == period) {
            logger_base.warn("Model %s has no nodes so skipping rendering.", (const char*)buffer.GetModel()->GetName().c_str());
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
            m = GetSequenceElements().GetXLightsFrame()->GetModel(buffer.GetModelName());
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
                logger_base.warn("render on model %s layer %d effect %s from %dms returned no buffer ... skipping rendering.", (const char*)buffer.GetModelName().c_str(), layer, (const char*)reff->Name().c_str(), effectObj->GetStartTimeMS());
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
                        retval = event->returnVal;
                    }
                    else {
                        logger_base.warn("HELP!!!!   Frame #%d render on model %s (%dx%d) layer %d effect %s from %dms (#%d) to %dms (#%d) timed out 10 secs.", b->curPeriod, (const char*)buffer.GetModelName().c_str(), b->BufferWi, b->BufferHt, layer, (const char*)reff->Name().c_str(), effectObj->GetStartTimeMS(), b->curEffStartPer, effectObj->GetEndTimeMS(), b->curEffEndPer);
                        printf("HELP!!!!   Frame #%d render on model %s (%dx%d) layer %d effect %s from %dms (#%d) to %dms (#%d) timed out 10 secs.\n", b->curPeriod, (const char*)buffer.GetModelName().c_str(), b->BufferWi, b->BufferHt, layer, (const char*)reff->Name().c_str(), effectObj->GetStartTimeMS(), b->curEffStartPer, effectObj->GetEndTimeMS(), b->curEffEndPer);

                        // Give it one more chance
                        if (event->signal.wait_for(lock, std::chrono::seconds(60)) == std::cv_status::no_timeout) {
                            retval = event->returnVal;
                        }
                        else {
                            logger_base.warn("DOUBLE HELP!!!!   Frame #%d render on model %s (%dx%d) layer %d effect %s from %dms (#%d) to %dms (#%d) timed out 70 secs.", b->curPeriod, (const char*)buffer.GetModelName().c_str(), b->BufferWi, b->BufferHt, layer, (const char*)reff->Name().c_str(), effectObj->GetStartTimeMS(), b->curEffStartPer, effectObj->GetEndTimeMS(), b->curEffEndPer);
                            printf("DOUBLE HELP!!!!   Frame #%d render on model %s (%dx%d) layer %d effect %s from %dms (#%d) to %dms (#%d) timed out 70 secs.\n", b->curPeriod, (const char*)buffer.GetModelName().c_str(), b->BufferWi, b->BufferHt, layer, (const char*)reff->Name().c_str(), effectObj->GetStartTimeMS(), b->curEffStartPer, effectObj->GetEndTimeMS(), b->curEffEndPer);
                        }
                    }
                    if (period % 10 == 0) {
                        //constantly putting stuff on CallAfter can result in the main
                        //dispatch thread never being able to empty the CallAfter
                        //queue and thus effectively blocking.   We'll yield periodically to
                        //allow the main thread to hopefully continue
                        wxThread::Yield();

                        // After yield who knows what may or may not be valid so we need to revalidate it
                        if (!_sequenceElements.IsValidEffect(event->effect)) {
                            logger_base.error("In RenderEffectFromMap after Yield() call checked the effect was still valid ... and it isnt ... this would likely have crashed.");
                        }
                    }
                }
                else {
                    int bufCnt = buffer.BufferCountForLayer(layer);
                    std::function<void(int)> f([this, &buffer, layer, suppress, effectObj, reff, &SettingsMap](int bufn) {
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

                            wxStopWatch sw;
                            if (effectObj != nullptr && reff->SupportsRenderCache(SettingsMap)) {
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
                            if (sw.Time() > 150) {
                                logger_render.info("Frame #%d render on model %s (%dx%d) layer %d effect %s from %dms (#%d) to %dms (#%d) took more than 150 ms => %dms.", rb->curPeriod, (const char*)buffer.GetModelName().c_str(), rb->BufferWi, rb->BufferHt, layer, (const char*)reff->Name().c_str(), effectObj->GetStartTimeMS(), rb->curEffStartPer, effectObj->GetEndTimeMS(), rb->curEffEndPer, sw.Time());
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
