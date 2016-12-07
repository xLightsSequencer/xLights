//
//  Render.cpp
//  xLights
//
//

#include <mutex>
#include <condition_variable>

#include "xLightsMain.h"
#include "xLightsXmlFile.h"
#include "RenderCommandEvent.h"
#include <map>
#include <memory>
#include "effects/RenderableEffect.h"
#include "RenderProgressDialog.h"
#include "SeqExportDialog.h"

#define END_OF_RENDER_FRAME INT_MAX

#include <log4cpp/Category.hh>

//other common strings
static const std::string STR_EMPTY("");

class EffectLayerInfo {
public:
    EffectLayerInfo() {
        numLayers = 0;
        buffer.reset(nullptr);
        strand = -1;
    }
    EffectLayerInfo(int l) {
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
        validLayers.resize(l);
    }
    int numLayers;
    int strand;
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
    RenderEvent() : mutex(), signal() {
    }
    std::mutex mutex;
    std::condition_variable signal;

    int layer;
    int period;
    Effect *effect;
    const SettingsMap *settingsMap;
    PixelBufferClass *buffer;
    bool *ResetEffectState;
    bool returnVal = true;
};


class NextRenderer {
public:
    NextRenderer() : nextLock(), nextSignal() {
        previousFrameDone = -1;
    }
    virtual ~NextRenderer() {
    }
    bool addNext(NextRenderer *n) {
        for (auto i = next.begin(); i < next.end(); i++) {
            if (*i == n) return false;
        }
        next.push_back(n);
        return true;
    }
    bool HasNext() {
        return !next.empty();
    }
    void FrameDone(int frame) {
        for (auto i = next.begin(); i < next.end(); i++) {
            (*i)->setPreviousFrameDone(frame);
        }
    }

    virtual void setPreviousFrameDone(int i) {
        std::unique_lock<std::mutex> lock(nextLock);
        previousFrameDone = i;
        nextSignal.notify_all();
    }
    int waitForFrame(int frame) {
        std::unique_lock<std::mutex> lock(nextLock);
        while (frame > previousFrameDone) {
            nextSignal.wait_for(lock, std::chrono::milliseconds(10));
        }
        return previousFrameDone;
    }
    bool checkIfDone(int frame, int timeout = 5) {
        std::unique_lock<std::mutex> lock(nextLock);
        return previousFrameDone >= frame;
    }
    int GetPreviousFrameDone()
    {
        return previousFrameDone;
    }
protected:
    std::mutex nextLock;
    std::condition_variable nextSignal;
    volatile long previousFrameDone;
private:
    std::vector<NextRenderer *> next;
};

class AggregatorRenderer: public NextRenderer {
public:
    AggregatorRenderer(int numFrames) : NextRenderer(), finalFrame(numFrames + 19) {
        data = new int[numFrames + 20];
        for (int x = 0; x < (numFrames + 20); x++) {
            data[x] = 0;
        }
        max = 0;
    }
    virtual ~AggregatorRenderer() {
        delete [] data;
    }
    void incNumAggregated() {
        max++;
    }
    int getNumAggregated() {
        return max;
    }
    virtual void setPreviousFrameDone(int frame) {
        if (max <= 1) {
            FrameDone(frame);
            return;
        }
        int idx = frame;
        if (idx == END_OF_RENDER_FRAME) {
            idx = finalFrame;
        }
        if (idx % 10 == 0 || idx == finalFrame) {
            //only record every 10th frame and the final frame to
            //avoid a lot of lock contention
            std::unique_lock<std::mutex> lock(nextLock);
            data[idx]++;
            if (data[idx] == max) {
                previousFrameDone = frame;
                FrameDone(previousFrameDone);
            }
        }
    }
private:
    int *data;
    int max;
    const int finalFrame;
};

class SNPair {
public:
    SNPair(int s, int n) : strand(s), node(n) {
    }
    SNPair(const SNPair &p) : strand(p.strand), node(p.node) {
    }

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
    RenderJob(ModelElement *row, SequenceData &data, xLightsFrame *xframe, bool zeroBased = false, bool clear = false)
        : Job(), NextRenderer(), rowToRender(row), seqData(&data), xLights(xframe), deleteWhenComplete(false),
            gauge(nullptr), currentFrame(0), renderLog(log4cpp::Category::getInstance(std::string("log_render")))
    {
        if (row != NULL) {
            name = row->GetModelName();
            mainBuffer = new PixelBufferClass(xframe, false);
            Model *model = xframe->GetModel(name);
            numLayers = rowToRender->GetEffectLayerCount();

            if (xframe->InitPixelBuffer(name, *mainBuffer, numLayers, zeroBased)) {

                for (int x = 0; x < row->GetSubModelCount(); x++) {
                    SubModelElement *se = row->GetSubModel(x);
                    if (se->HasEffects() > 0) {
                        if (se->GetType() == ELEMENT_TYPE_STRAND) {
                            StrandElement *ste = (StrandElement*)se;
                            if (ste->GetStrand() < model->GetNumStrands()) {
                                subModelInfos.push_back(new EffectLayerInfo(se->GetEffectLayerCount() + 1));
                                subModelInfos.back()->element = se;
                                subModelInfos.back()->buffer.reset(new PixelBufferClass(xframe, false));
                                subModelInfos.back()->strand = ste->GetStrand();
                                subModelInfos.back()->buffer->InitStrandBuffer(*model, ste->GetStrand(), data.FrameTime(), se->GetEffectLayerCount());
                            }
                        } else {
                            Model *subModel = model->GetSubModel(se->GetName());
                            if (subModel != nullptr) {
                                subModelInfos.push_back(new EffectLayerInfo(se->GetEffectLayerCount() + 1));
                                subModelInfos.back()->element = se;
                                subModelInfos.back()->buffer.reset(new PixelBufferClass(xframe, false));
                                subModelInfos.back()->buffer->InitBuffer(*subModel, se->GetEffectLayerCount() + 1, data.FrameTime(), false);
                            }
                        }
                    }
                    if (se->GetType() == ELEMENT_TYPE_STRAND) {
                        StrandElement *ste = (StrandElement*)se;
                        if (ste->GetStrand() < model->GetNumStrands()) {
                            for (int n = 0; n < ste->GetNodeLayerCount(); n++) {
                                if (n < model->GetStrandLength(ste->GetStrand())) {
                                    EffectLayer *nl = ste->GetNodeLayer(n);
                                    if (nl -> GetEffectCount() > 0) {
                                        nodeBuffers[SNPair(ste->GetStrand(), n)].reset(new PixelBufferClass(xframe, false));
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
            mainBuffer = NULL;
            name = "";
        }
        startFrame = 0;
        clearAllFrames = clear;
        renderEvent.buffer = mainBuffer;
    }

    virtual ~RenderJob() {
        if (mainBuffer != NULL) {
            delete mainBuffer;
        }
        for (auto a = subModelInfos.begin(); a != subModelInfos.end(); a++) {
            EffectLayerInfo *info = *a;
            delete info;
        }
    }

    wxGauge *GetGauge() const { return gauge;}
    void SetGauge(wxGauge *g) { gauge = g;}
    int GetCurrentFrame() const { return currentFrame;}

    const std::string &GetName() const {
        return name;
    }

    virtual bool DeleteWhenComplete() override {
        return deleteWhenComplete;
    }
    void SetDeleteWhenComplete() {
        deleteWhenComplete = true;
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
    void SetGenericStatus(const wxString &msg, int frame, bool debugLog = false) {
        statusType = 4;
        statusMsg = msg;
        statusFrame = frame;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }
    void SetGenericStatus(const char *msg, int frame, bool debugLog = false) {
        statusType = 6;
        statusMsgChars = msg;
        statusFrame = frame;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }
    void SetGenericStatus(const wxString &msg, int frame, int layer, bool debugLog = false) {
        statusType = 5;
        statusMsg = msg;
        statusFrame = frame;
        statusLayer = layer;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }
    void SetGenericStatus(const char *msg, int frame, int layer, bool debugLog = false) {
        statusType = 7;
        statusMsgChars = msg;
        statusFrame = frame;
        statusLayer = layer;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }
    void SetRenderingStatus(int frame, SettingsMap*map, int layer, int strand, int node, bool debugLog = false) {
        statusType = 2;
        statusFrame = frame;
        statusLayer = layer;
        statusStrand = strand;
        statusNode = node;
        statusMap = map;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }
    void SetCalOutputStatus(int frame, int strand = -1, int node = -1, bool debugLog = true) {
        statusType = 3;
        statusFrame = frame;
        statusStrand = strand;
        statusNode = node;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }
    void SetInializingStatus(int frame, int layer, int strand = -1, int node = -1, bool debugLog = false) {
        statusType = 1;
        statusFrame = frame;
        statusLayer = layer;
        statusStrand = strand;
        statusNode = node;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }
    void SetStatus(const wxString &st, bool debugLog = false) {
        statusMsg = st;
        statusType = 0;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }
    void SetStatus(const char *st, bool debugLog = false) {
        statusMsgChars = st;
        statusType = 8;
        LogToLogger(debugLog ? log4cpp::Priority::DEBUG : log4cpp::Priority::INFO);
    }
    std::string GetStatus() override {
        return GetwxStatus().ToStdString();
    }
    wxString GetwxStatus() {
        switch (statusType) {
        case 0:
            return statusMsg;
        case 1:
            if (statusStrand == -1) {
                return wxString::Format("Initializing effect at frame %d for %s, layer %d.", statusFrame, name, statusLayer);
            } else if (statusNode == -1) {
                return wxString::Format("Initializing strand effect at frame %d for %s, strand %d.", statusFrame, name, statusStrand);
            } else {
                return wxString::Format("Initializing node effect at frame %d for %s, strand %d, node %d.", statusFrame, name, statusStrand, statusNode);
            }
        case 2:
            if (statusStrand == -1) {
                return wxString::Format("Rendering layer effect for frame %d of %s, layer %d.", statusFrame, name, statusLayer) + statusMap->AsString();
            } else if (statusNode == -1) {
                return wxString::Format("Rendering strand effect for frame %d of %s, strand %d.", statusFrame, name, statusStrand) + statusMap->AsString();
            } else {
                return wxString::Format("Rendering node effect for frame %d of %s, strand %d, node %d.", statusFrame, name, statusLayer, statusNode) + statusMap->AsString();
            }
        case 3:
            if (statusStrand == -1) {
                return wxString::Format("Calculating output at frame %d for %s.", statusFrame, name);
            } else if (statusNode == -1) {
                return wxString::Format("Calculating output at frame %d for %s, strand %d.", statusFrame, name, statusStrand);
            } else {
                return wxString::Format("Calculating output at frame %d for %s, strand %d, node %d.", statusFrame, name, statusStrand, statusNode);
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
            return statusMsgChars;
        }
        return statusMsg;
    }
    SequenceData *createExportBuffer() {
        SequenceData *sb = new SequenceData();
        sb->init(xLights->GetModel(mainBuffer->GetModelName())->GetActChanCount(), seqData->NumFrames(), seqData->FrameTime());
        seqData = sb;
        return sb;
    }
    PixelBufferClass *getBuffer() {
        return mainBuffer;
    }

    void setRenderRange(int start, int end) {
        startFrame = start;
        endFrame = end;
    }
    

    bool ProcessFrame(int frame, Element *el, EffectLayerInfo &info, PixelBufferClass *buffer, int strand = -1, bool blend = false) {
        wxStopWatch sw;
        bool effectsToUpdate = false;
        int numLayers = el->GetEffectLayerCount();
        for (int layer = 0; layer < info.validLayers.size(); layer++) {
            info.validLayers[layer] = false;
        }
        for (int layer = 0; layer < numLayers; layer++) {
            EffectLayer *elayer = el->GetEffectLayer(layer);
            Effect *ef = findEffectForFrame(elayer, frame, info.currentEffectIdxs[layer]);
            if (ef != info.currentEffects[layer]) {
                info.currentEffects[layer] = ef;
                SetInializingStatus(frame, layer, strand);
                initialize(layer, frame, ef, info.settingsMaps[layer], buffer);
                info.effectStates[layer] = true;
            }
            bool persist = buffer->IsPersistent(layer);
            if (!persist || info.currentEffects[layer] == nullptr || info.currentEffects[layer]->GetEffectIndex() == -1) {
                buffer->Clear(layer);
            }
            SetRenderingStatus(frame, &info.settingsMaps[layer], layer, strand, -1, true);
            bool b = info.effectStates[layer];
            info.validLayers[layer] = xLights->RenderEffectFromMap(ef, layer, frame, info.settingsMaps[layer], *buffer, b, true, &renderEvent);
            info.effectStates[layer] = b;
            effectsToUpdate |= info.validLayers[layer];
        }
        if (!effectsToUpdate && clearAllFrames) {
            info.validLayers[0] = true;
            effectsToUpdate = true;
        }
        if (effectsToUpdate) {
            SetCalOutputStatus(frame, strand);
            if (blend) {
                buffer->SetColors(numLayers, &((*seqData)[frame][0]));
                info.validLayers[numLayers] = true;
            }
            buffer->CalcOutput(frame, info.validLayers);
            buffer->GetColors(&((*seqData)[frame][0]));
        }
        if (sw.Time() > 1000)
        {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            RenderBuffer& b = buffer->BufferForLayer(0);
            logger_base.warn("*** Frame #%d at %dms render on model %s (%dx%d) took more than 1s => %dms.", frame, frame * b.frameTimeInMs, (const char *)el->GetName().c_str(), b.BufferWi, b.BufferHt, sw.Time());
        }
        return effectsToUpdate;
    }
    
    virtual void Process() override {
        SetGenericStatus("Initializing rendering thread for %s", 0);
        int maxFrameBeforeCheck = -1;
        int origChangeCount;
        int ss, es;

        rowToRender->IncWaitCount();
        std::unique_lock<std::recursive_mutex> lock(rowToRender->GetRenderLock());
        rowToRender->DecWaitCount();
        SetGenericStatus("Got lock on rendering thread for %s", 0);

        rowToRender->GetAndResetDirtyRange(origChangeCount, ss, es);
        if (ss != -1) {
            //expand to cover the whole dirty range
            ss = ss / seqData->FrameTime();
            if (ss < 0) {
                ss = 0;
            }
            es = es / seqData->FrameTime();
            if (es > seqData->NumFrames()) {
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
        if (endFrame > seqData->NumFrames()) endFrame = seqData->NumFrames();

        EffectLayerInfo mainModelInfo(numLayers);
        std::map<SNPair, Effect*> nodeEffects;
        std::map<SNPair, SettingsMap> nodeSettingsMaps;
        std::map<SNPair, bool> nodeEffectStates;
        std::map<SNPair, int> nodeEffectIdxs;

        if (clearAllFrames && mainBuffer != NULL) {
            mainBuffer->Clear(0);
        }

        try {
            for (int layer = 0; layer < numLayers; layer++) {
                wxString msg = wxString::Format("Finding starting effect for %s, layer %d and startFrame %d", name, layer, startFrame);
                SetStatus(msg);
                mainModelInfo.currentEffects[layer] = findEffectForFrame(layer, startFrame, mainModelInfo.currentEffectIdxs[layer]);
                msg = wxString::Format("Initializing starting effect for %s, layer %d and startFrame %d", name, layer, startFrame);
                SetStatus(msg);
                initialize(layer, startFrame, mainModelInfo.currentEffects[layer], mainModelInfo.settingsMaps[layer], mainBuffer);
                mainModelInfo.effectStates[layer] = true;
            }

            for (int frame = startFrame; frame < endFrame; frame++) {
                currentFrame = frame;
                SetGenericStatus("%s: Starting frame %d", frame, true);

                if (!HasNext() &&
                        (origChangeCount != rowToRender->getChangeCount()
                         || rowToRender->GetWaitCount())) {
                    //we're bailing out but make sure this range is reconsidered
                    rowToRender->SetDirtyRange(frame * seqData->FrameTime(), endFrame * seqData->FrameTime());
                    break;
                }
                //make sure we can do this frame
                if (frame >= maxFrameBeforeCheck) {
                    maxFrameBeforeCheck = waitForFrame(frame);
                }
                bool cleared = ProcessFrame(frame, rowToRender, mainModelInfo, mainBuffer);
                if (!subModelInfos.empty()) {
                    for (auto a = subModelInfos.begin(); a != subModelInfos.end(); a++) {
                        EffectLayerInfo *info = *a;
                        cleared |= ProcessFrame(frame, info->element, *info, info->buffer.get(), info->strand, cleared);
                    }
                }
                if (!nodeBuffers.empty()) {
                    for (std::map<SNPair, PixelBufferClassPtr>::iterator it = nodeBuffers.begin(); it != nodeBuffers.end(); it++) {
                        SNPair node = it->first;
                        PixelBufferClass *buffer = it->second.get();
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
                        
                        Effect *el = findEffectForFrame(nlayer, frame, nodeEffectIdxs[node]);
                        if (el != nodeEffects[node] || frame == startFrame) {
                            nodeEffects[node] = el;
                            SetInializingStatus(frame, -1, strand, inode);
                            initialize(0, frame, el, nodeSettingsMaps[node], buffer);
                            nodeEffectStates[node] = true;
                        }
                        bool persist=buffer->IsPersistent(0);
                        if (!persist || nodeEffects[node] == nullptr || nodeEffects[node]->GetEffectIndex() == -1) {
                            buffer->Clear(0);
                        }

                        SetRenderingStatus(frame, &nodeSettingsMaps[node], -1, strand, inode, cleared);
                        if (xLights->RenderEffectFromMap(el, 0, frame, nodeSettingsMaps[node], *buffer, nodeEffectStates[node], true, &renderEvent)) {
                            SetCalOutputStatus(frame, strand, inode);
                            //copy to output
                            std::vector<bool> valid(2, true);
                            buffer->SetColors(1, &((*seqData)[frame][0]));
                            buffer->CalcOutput(frame, valid);
                            buffer->GetColors(&((*seqData)[frame][0]));
                        }
                    }
                }
                //mainBuffer->ApplyDimmingCurves(&((*seqData)[frame][0]));
                if (HasNext()) {
                    SetGenericStatus("%s: Notifying next renderer of frame %d done", frame);
                    FrameDone(frame);
                }
            }
        } catch ( std::exception &ex) {
            printf("Caught an exception %s", ex.what());
			renderLog.error("Caught an exception on rendering thread: " + std::string(ex.what()));
		} catch ( ... ) {
            printf("Caught an unknown exception");
			renderLog.error("Caught an unknown exception on rendering thread.");
		}
        if (HasNext()) {
            //make sure the previous has told us we're at the end.  If we return before waiting, the previous
            //may try sending the END_OF_RENDER_FRAME to us and we'll have been deleted
            SetGenericStatus("%s: Waiting on previous renderer for final frame", 0);
            waitForFrame(END_OF_RENDER_FRAME);

            //let the next know we're done
            SetGenericStatus("%s: Notifying next renderer of final frame", 0);
            FrameDone(END_OF_RENDER_FRAME);
            xLights->CallAfter(&xLightsFrame::SetStatusText, wxString("Done Rendering " + rowToRender->GetModelName()), 0);
        } else {
            xLights->CallAfter(&xLightsFrame::RenderDone);
        }
        currentFrame = END_OF_RENDER_FRAME;
        //printf("Done rendering %lx (next %lx)\n", (unsigned long)this, (unsigned long)next);
		renderLog.debug("Rendering thread exiting.");
	}

private:

    void initialize(int layer, int frame, Effect *el, SettingsMap &settingsMap, PixelBufferClass *buffer) {
        if (el == NULL || el->GetEffectIndex() == -1) {
            settingsMap.clear();
        } else {
            loadSettingsMap(el->GetEffectName(),
                            el,
                            settingsMap);
        }
        buffer->SetLayerSettings(layer, settingsMap);
        updateBufferPaletteFromMap(layer, el, buffer);

        if (el != NULL) {
            buffer->SetTimes(layer, el->GetStartTimeMS(), el->GetEndTimeMS());
        }
    }

    void updateBufferPaletteFromMap(int layer, Effect *effect, PixelBufferClass *buffer) {
        xlColorVector newcolors;
        xlColorCurveVector newcc;
        if (effect != nullptr) {
            effect->CopyPalette(newcolors, newcc);
        }
        buffer->SetPalette(layer, newcolors, newcc);
    }

    Effect *findEffectForFrame(EffectLayer* layer, int frame, int &lastIdx) {
        if (layer == nullptr) {
            return nullptr;
        }
        int time = frame * seqData->FrameTime();
        for (int e = lastIdx; e < layer->GetEffectCount(); e++) {
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
    void loadSettingsMap(const std::string &effectName,
                         Effect *effect,
                         SettingsMap& settingsMap) {
        settingsMap.clear();
        effect->CopySettingsMap(settingsMap, true);
    }


    ModelElement *rowToRender;
    std::string name;
    int startFrame;
    int endFrame;
    PixelBufferClass *mainBuffer;
    int numLayers;
    xLightsFrame *xLights;
    SequenceData *seqData;
    bool clearAllFrames;
    RenderEvent renderEvent;
    bool deleteWhenComplete;

    //stuff for handling the status;
    wxString statusMsg;
    const char *statusMsgChars;
    volatile int statusType;
    volatile int statusFrame;
    SettingsMap *statusMap;
    volatile int statusLayer;
    volatile int statusStrand;
    volatile int statusNode;
    log4cpp::Category &renderLog;

    wxGauge *gauge;
    int currentFrame;

    std::vector<EffectLayerInfo *> subModelInfos;

    std::map<SNPair, PixelBufferClassPtr> nodeBuffers;
};


IMPLEMENT_DYNAMIC_CLASS(RenderCommandEvent, wxCommandEvent)
IMPLEMENT_DYNAMIC_CLASS(SelectedEffectChangedEvent, wxCommandEvent)

void xLightsFrame::RenderRange(RenderCommandEvent &evt) {
    if (evt.deleted) {
        selectedEffect = 0;
    }
    RenderEffectForModel(evt.model, evt.start,  evt.end, evt.clear);
}

void xLightsFrame::RenderEffectOnMainThread(RenderEvent *ev) {
    ev->returnVal = RenderEffectFromMap(ev->effect,
                                        ev->layer,
                                        ev->period,
                                        *ev->settingsMap,
                                        *ev->buffer, *ev->ResetEffectState, false, ev);
    ev->signal.notify_all();
}

class RenderProgressInfo {
public:
    RenderProgressInfo(std::function<void()>&& cb) : callback(cb) {};
    std::function<void()> callback;
    int numRows;
    int frames;
    RenderJob **jobs;
    AggregatorRenderer **aggregators;
};

void xLightsFrame::RenderGridToSeqData(std::function<void()>&& callback) {
    const int numRows = mSequenceElements.GetElementCount();
    if (numRows == 0) {
        //nothing to do....
        return;
    }
    RenderJob **jobs = new RenderJob*[numRows];
    AggregatorRenderer **aggregators = new AggregatorRenderer*[numRows];
    std::vector<std::set<int>> channelMaps(SeqData.NumChannels());
    ModelElement *lastRowEl = NULL;

    for (size_t row = 0; row < numRows; row++) {
        jobs[row] = nullptr;
        aggregators[row] = new AggregatorRenderer(SeqData.NumFrames());
    }

    for (size_t row = 0; row < numRows; row++) {
        Element *rowEl = mSequenceElements.GetElement(row);
        if (rowEl->GetType() == ELEMENT_TYPE_MODEL && rowEl != lastRowEl) {
            ModelElement *me = dynamic_cast<ModelElement *>(rowEl);
            lastRowEl = me;
            bool hasEffects = me->HasEffects();
            if (!hasEffects) {
                for (int x = 0; x < me->GetSubModelCount(); x++) {
                    hasEffects |= me->GetSubModel(x)->HasEffects();
                }
            }
            if (!hasEffects) {
                for (int x = 0; x < me->GetStrandCount(); x++) {
                    StrandElement *se = me->GetStrand(x);
                    for (int n = 0; n < se->GetNodeLayerCount(); n++) {
                        hasEffects |= se->GetNodeLayer(n)->GetEffectCount() > 0;
                    }
                }
            }
            if (!hasEffects) {
                //there are no effects anywhere on this model, we dont need to consider this at all
                continue;
            }
            RenderJob *job = new RenderJob(me, SeqData, this);
            job->setRenderRange(0, SeqData.NumFrames());

            PixelBufferClass *buffer = job->getBuffer();
            if (buffer == nullptr) {
                delete job;
                continue;
            }

            jobs[row] = job;
            aggregators[row]->addNext(job);
            size_t cn = buffer->GetChanCountPerNode();
            for (int node = 0; node < buffer->GetNodeCount(); node++) {
                int start = buffer->NodeStartChannel(node);
                for (int c = 0; c < cn; c++) {
                    int cnum = start + c;
                    if (cnum < SeqData.NumChannels()) {
                        for (auto i = channelMaps[cnum].begin(); i != channelMaps[cnum].end(); i++) {
                            int idx = *i;
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
    channelMaps.clear();

    renderProgressDialog = new RenderProgressDialog(this);
    for (size_t row = 0; row < numRows; row++) {
        if (jobs[row]) {
            if (aggregators[row]->getNumAggregated() == 0) {
                //start all the jobs that don't depend on anything above them
                //get them rendering while we setup the rest
                jobs[row]->setPreviousFrameDone(END_OF_RENDER_FRAME);
                jobPool.PushJob(jobs[row]);
            }

            wxStaticText *label = new wxStaticText(renderProgressDialog->scrolledWindow, wxID_ANY, jobs[row]->GetName());
            renderProgressDialog->scrolledWindowSizer->Add(label,1, wxALL |wxEXPAND,3);
            wxGauge *g = new wxGauge(renderProgressDialog->scrolledWindow, wxID_ANY, 99);
            g->SetValue(0);
            g->SetMinSize(wxSize(100, -1));
            renderProgressDialog->scrolledWindowSizer->Add(g, 1, wxALL |wxEXPAND,3);
            jobs[row]->SetGauge(g);
        }
    }
    for (size_t row = 0; row < numRows; row++) {
        if (jobs[row] && aggregators[row]->getNumAggregated() != 0) {
            //now start the rest
            jobPool.PushJob(jobs[row]);
        }
    }
    renderProgressDialog->SetSize(250, 400);
    renderProgressDialog->scrolledWindow->SetSizer(renderProgressDialog->scrolledWindowSizer);
    renderProgressDialog->scrolledWindow->FitInside();
    renderProgressDialog->scrolledWindow->SetScrollRate(5, 5);

    renderProgressInfo = new RenderProgressInfo(std::move(callback));
    renderProgressInfo->numRows = numRows;
    renderProgressInfo->frames = SeqData.NumFrames();
    renderProgressInfo->jobs = jobs;
    renderProgressInfo->aggregators = aggregators;
}
void xLightsFrame::UpdateRenderStatus() {
    if (renderProgressInfo == nullptr) {
        return;
    }
    bool done = true;
    int countModels = 0;
    int countFrames = 0;
    bool shown = renderProgressDialog->IsShown();
    for (size_t row = 0; row < renderProgressInfo->numRows; row++) {
        if (renderProgressInfo->jobs[row]) {
            int i = renderProgressInfo->jobs[row]->GetCurrentFrame();
            if (i >= renderProgressInfo->frames) {
                i = renderProgressInfo->frames;
            } else {
                done = false;
            }
            countModels++;
            if (shown) {
                int val = (100 * i)/renderProgressInfo->frames;
                wxGauge *g = renderProgressInfo->jobs[row]->GetGauge();
                if (g->GetValue() != val) {
                    g->SetValue(val);
                }
            }
            countFrames += i;
        }
    }
    if (countFrames > 0) {
        int pct = (countFrames * 80) / (countModels * renderProgressInfo->frames);
        static int lastVal = 0;
        if (lastVal != pct) {
            if (ProgressBar->GetValue() != (10 + pct)) {
                ProgressBar->SetValue(10 + pct);
            }
            lastVal = pct;
        }
    }

    if (done) {
        for (size_t row = 0; row < renderProgressInfo->numRows; row++) {
            if (renderProgressInfo->jobs[row]) {
                delete renderProgressInfo->jobs[row];
            }
            delete renderProgressInfo->aggregators[row];
        }
        delete renderProgressDialog;
        renderProgressDialog = nullptr;
        delete []renderProgressInfo->jobs;
        delete []renderProgressInfo->aggregators;
        RenderDone();
        renderProgressInfo->callback();
        delete renderProgressInfo;
        renderProgressInfo = nullptr;
    }
}


void xLightsFrame::RenderDone() {
    mainSequencer->PanelEffectGrid->Refresh();
}
void xLightsFrame::RenderEffectForModel(const std::string &model, int startms, int endms, bool clear) {
	//printf("render model %s   %d %d   %d\n", model.c_str(), startms, endms, clear);
    RenderJob *job = NULL;
    Element * el = mSequenceElements.GetElement(model);
    if( el->GetType() != ELEMENT_TYPE_TIMING) {
        job = new RenderJob(dynamic_cast<ModelElement*>(el), SeqData, this, false, clear);
        job->SetDeleteWhenComplete();
        if (job->getBuffer() == nullptr) {
            delete job;
            return;
        }

        //account for some rounding by rendering before/after
        int startframe = startms / SeqData.FrameTime() - 1;
        if (startframe < 0) {
            startframe = 0;
        }
        int endframe = endms / SeqData.FrameTime() + 1;
        if (endframe >= SeqData.NumFrames()) {
            endframe = SeqData.NumFrames() - 1;
        }
        job->setRenderRange(startframe, endframe);
        job->setPreviousFrameDone(END_OF_RENDER_FRAME);
        jobPool.PushJob(job);
    }
}


void xLightsFrame::ExportModel(wxCommandEvent &command) {
     
    std::string model = command.GetString().ToStdString();
    Model *m = GetModel(model);

    bool isgroup = (m->GetDisplayAs() == "ModelGroup"); 

    bool isboxed = false;
    if (dynamic_cast<ModelWithScreenLocation<BoxedScreenLocation>*>(m) != nullptr)
    {
        // line models, arches etc make no sense for videos
        isboxed = true;
    }

    SeqExportDialog dialog(this);
    dialog.ModelExportTypes(isgroup || !isboxed);

    if (dialog.ShowModal() == wxID_OK) {
        wxString filename = dialog.TextCtrlFilename->GetValue();
        EnableSequenceControls(false);
        wxString format = dialog.ChoiceFormat->GetStringSelection();
        wxStopWatch sw;
        wxString Out3 = format.Left(3);

        if (Out3 == "LSP") {
            filename = filename + "_USER";
        }
        wxFileName oName(filename);

        if (oName.GetPathWithSep() == "")
        {
            oName.SetPath(CurrentDir);
        }
        wxString fullpath;

        SetStatusText(_("Starting Export for ") + format + "-" + Out3);
        wxYield();

        NextRenderer wait;
        Element * el = mSequenceElements.GetElement(model);
        RenderJob *job = new RenderJob(dynamic_cast<ModelElement*>(el), SeqData, this, true);
        SequenceData *data = job->createExportBuffer();
        int cpn = job->getBuffer()->GetChanCountPerNode();

        if (command.GetInt()) {
            job->setRenderRange(0, SeqData.NumFrames());
            job->setPreviousFrameDone(END_OF_RENDER_FRAME);
            job->addNext(&wait);
            jobPool.PushJob(job);
            //wait to complete
            while (!wait.checkIfDone(SeqData.NumFrames())) {
                wxYield();
            }
        }
        else {
            Model *m = GetModel(model);
            for (int frame = 0; frame < SeqData.NumFrames(); frame++) {
                for (int x = 0; x < job->getBuffer()->GetNodeCount(); x++) {
                    //chan in main buffer
                    int ostart = m->NodeStartChannel(x);
                    int nstart = job->getBuffer()->NodeStartChannel(x);
                    //copy to render buffer for export
                    job->getBuffer()->SetNodeChannelValues(x, &SeqData[frame][ostart]);
                    job->getBuffer()->GetNodeChannelValues(x, &((*data)[frame][nstart]));
                }
            }
        }
        delete job;


        if (Out3 == "Lcb") {
            oName.SetExt(_("lcb"));
            fullpath = oName.GetFullPath();
            WriteLcbFile(fullpath, data->NumChannels(), SeqData.NumFrames(), data);
        }
        else if (Out3 == "Vir") {
            oName.SetExt(_("vir"));
            fullpath = oName.GetFullPath();
            WriteVirFile(fullpath, data->NumChannels(), SeqData.NumFrames(), data);
        }
        else if (Out3 == "LSP") {
            oName.SetExt(_("xml"));
            fullpath = oName.GetFullPath();
            WriteLSPFile(fullpath, data->NumChannels(), SeqData.NumFrames(), data, cpn);
        }
        else if (Out3 == "HLS") {
            oName.SetExt(_("hlsnc"));
            fullpath = oName.GetFullPath();
            WriteHLSFile(fullpath, data->NumChannels(), SeqData.NumFrames(), data);
        }
        else if (Out3 == "Fal") {
            int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
            oName.SetExt(_("eseq"));
            fullpath = oName.GetFullPath();
            WriteFalconPiModelFile(fullpath, data->NumChannels(), SeqData.NumFrames(), data, stChan, data->NumChannels());
        }
        else if (Out3 == "Com")
        {
            int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
            oName.SetExt(_("avi"));
            fullpath = oName.GetFullPath();
            WriteVideoModelFile(fullpath, data->NumChannels(), SeqData.NumFrames(), data, stChan, data->NumChannels(), GetModel(model), true);
        }
        else if (Out3 == "Unc")
        {
            int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
            oName.SetExt(_("avi"));
            fullpath = oName.GetFullPath();
            WriteVideoModelFile(fullpath, data->NumChannels(), SeqData.NumFrames(), data, stChan, data->NumChannels(), GetModel(model), false);
        }
        SetStatusText(_("Finished writing model: ") + fullpath + wxString::Format(" in %ld ms ", sw.Time()));

        delete data;
        EnableSequenceControls(true);
    }
}


bool xLightsFrame::RenderEffectFromMap(Effect *effectObj, int layer, int period, const SettingsMap& SettingsMap,
                                       PixelBufferClass &buffer, bool &resetEffectState,
                                       bool bgThread, RenderEvent *event) {
    
    if (buffer.BufferForLayer(layer).BufferHt == 0 || buffer.BufferForLayer(layer).BufferWi == 0) {
        return false;
    }
    bool retval=true;

    buffer.SetLayer(layer, period, resetEffectState);
    resetEffectState = false;
    int eidx = -1;
    if (effectObj != nullptr) {
        eidx = effectObj->GetEffectIndex();
    }
    if (eidx >= 0) {
        RenderableEffect *reff = effectManager.GetEffect(eidx);
        if (reff == nullptr) {
            retval= false;
        } else if (!bgThread || reff->CanRenderOnBackgroundThread(effectObj, SettingsMap, buffer.BufferForLayer(layer))) {
            wxStopWatch sw;
            reff->Render(effectObj, SettingsMap, buffer.BufferForLayer(layer));
            // Log slow render frames ... this takes time but at this point it is already slow
            if (sw.Time() > 150)
            {
                static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
                RenderBuffer& b = buffer.BufferForLayer(layer);
                logger_base.warn("Frame #%d render on model %s (%dx%d) layer %d effect %s from %dms (#%d) to %dms (#%d) took more than 150 ms => %dms.", b.curPeriod, (const char *)buffer.GetModelName().c_str(),b.BufferWi, b.BufferHt, layer, (const char *)reff->Name().c_str(), effectObj->GetStartTimeMS(), b.curEffStartPer, effectObj->GetEndTimeMS(), b.curEffEndPer, sw.Time());
            }
        } else {
            event->effect = effectObj;
            event->layer = layer;
            event->period = period;
            event->settingsMap = &SettingsMap;
            event->ResetEffectState = &resetEffectState;

            std::unique_lock<std::mutex> lock(event->mutex);
            CallAfter(&xLightsFrame::RenderEffectOnMainThread, event);
            if (event->signal.wait_for(lock, std::chrono::seconds(60)) == std::cv_status::no_timeout) {
                retval = event->returnVal;
            } else {
                static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
                RenderBuffer& b = buffer.BufferForLayer(layer);
                logger_base.warn("Frame #%d render on model %s (%dx%d) layer %d effect %s from %dms (#%d) to %dms (#%d) timed out.", b.curPeriod, (const char *)buffer.GetModelName().c_str(), b.BufferWi, b.BufferHt, layer, (const char *)reff->Name().c_str(), effectObj->GetStartTimeMS(), b.curEffStartPer, effectObj->GetEndTimeMS(), b.curEffEndPer);
                printf("HELP!!!!\n");
			}
            if (period % 10 == 0) {
                //constantly putting stuff on CallAfter can result in the main
                //dispatch thread never being able to empty the CallAfter
                //queue and thus effectively blocking.   We'll yield periodically to
                //allow the main thread to hopefully continue
                wxThread::Yield();
            }
        }
    } else {
        retval = false;
    }
    return retval;
}
