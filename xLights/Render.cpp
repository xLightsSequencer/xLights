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
#include "BitmapCache.h"
#include <map>
#include <memory>
#include "effects/RenderableEffect.h"

#define END_OF_RENDER_FRAME INT_MAX


static const std::string SLIDER_SparkleFrequency("SLIDER_SparkleFrequency");
static const std::string SLIDER_Brightness("SLIDER_Brightness");
static const std::string SLIDER_Contrast("SLIDER_Contrast");
static const std::string CHOICE_LayerMethod("CHOICE_LayerMethod");
static const std::string SLIDER_EffectLayerMix("SLIDER_EffectLayerMix");
static const std::string CHECKBOX_LayerMorph("CHECKBOX_LayerMorph");
static const std::string TEXTCTRL_Fadein("TEXTCTRL_Fadein");
static const std::string TEXTCTRL_Fadeout("TEXTCTRL_Fadeout");
static const std::string SLIDER_EffectBlur("SLIDER_EffectBlur");

static const std::string CHECKBOX_OverlayBkg("CHECKBOX_OverlayBkg");
static const std::string CHOICE_BufferStyle("CHOICE_BufferStyle");
static const std::string CHOICE_BufferTransform("CHOICE_BufferTransform");
static const std::string STR_DEFAULT("Default");

//other common strings
static const std::string STR_NORMAL("Normal");
static const std::string STR_EFFECT("Effect");
static const std::string STR_NONE("None");
static const std::string STR_EMPTY("");


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
        next = NULL;
    }
    virtual ~NextRenderer() {}
    void setNext(NextRenderer *n) {
        next = n;
    }
    NextRenderer *getNext() {
        return next;
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
protected:
    std::mutex nextLock;
    std::condition_variable nextSignal;
    volatile long previousFrameDone;
    NextRenderer *next;
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
    virtual void setPreviousFrameDone(int frame) {
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
                if (next != NULL) {
                    next->setPreviousFrameDone(previousFrameDone);
                }
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
    RenderJob(Element *row, SequenceData &data, xLightsFrame *xframe, bool zeroBased = false, bool clear = false)
        : Job(), NextRenderer(), rowToRender(row), seqData(&data), xLights(xframe) {
        if (row != NULL) {
            name = row->GetName();
            mainBuffer = new PixelBufferClass(false);
            Model *model = xframe->GetModel(name);
            if (xframe->InitPixelBuffer(name, *mainBuffer, rowToRender->GetEffectLayerCount(), zeroBased)) {

                for (int x = 0; x < row->getStrandLayerCount(); x++) {
                    StrandLayer *sl = row->GetStrandLayer(x);
                    if (sl -> GetEffectCount() > 0) {
                        strandBuffers[x].reset(new PixelBufferClass(false));
                        strandBuffers[x]->InitStrandBuffer(*model, x, data.FrameTime());
                    }
                    for (int n = 0; n < sl->GetNodeLayerCount(); n++) {
                        if (n < model->GetStrandLength(x)) {
                            EffectLayer *nl = sl->GetNodeLayer(n);
                            if (nl -> GetEffectCount() > 0) {
                                nodeBuffers[SNPair(x, n)].reset(new PixelBufferClass(false));
                                nodeBuffers[SNPair(x, n)]->InitNodeBuffer(*model, x, n, data.FrameTime());
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
    }
    void SetGenericStatus(const wxString &msg, int frame) {
        statusType = 4;
        statusMsg = msg;
        statusFrame = frame;
    }
    void SetGenericStatus(const char *msg, int frame) {
        statusType = 6;
        statusMsgChars = msg;
        statusFrame = frame;
    }
    void SetGenericStatus(const wxString &msg, int frame, int layer) {
        statusType = 5;
        statusMsg = msg;
        statusFrame = frame;
        statusLayer = layer;
    }
    void SetGenericStatus(const char *msg, int frame, int layer) {
        statusType = 7;
        statusMsgChars = msg;
        statusFrame = frame;
        statusLayer = layer;
    }
    void SetRenderingStatus(int frame, SettingsMap*map, int layer, int strand = -1, int node = -1) {
        statusType = 2;
        statusFrame = frame;
        statusLayer = layer;
        statusStrand = strand;
        statusNode = node;
        statusMap = map;
    }
    void SetCalOutputStatus(int frame, int strand = -1, int node = -1) {
        statusType = 3;
        statusFrame = frame;
        statusStrand = strand;
        statusNode = node;
    }
    void SetInializingStatus(int frame, int layer, int strand = -1, int node = -1) {
        statusType = 1;
        statusFrame = frame;
        statusLayer = layer;
        statusStrand = strand;
        statusNode = node;
    }
    void SetStatus(const wxString &st) {
        statusMsg = st;
        statusType = 0;
    }
    void SetStatus(const char *st) {
        statusMsgChars = st;
        statusType = 8;
    }
    std::string GetStatus() {
        return GetwxStatus().ToStdString();
    }
    wxString GetwxStatus() {
        switch (statusType) {
        case 0:
            return statusMsg;
        case 1:
            if (statusStrand == -1) {
                return wxString::Format("Initializing effect at frame %d for %s, layer %d.\n    ", statusFrame, name, statusLayer);
            } else if (statusNode == -1) {
                return wxString::Format("Initializing strand effect at frame %d for %s, strand %d.\n    ", statusFrame, name, statusStrand);
            } else {
                return wxString::Format("Initializing node effect at frame %d for %s, strand %d, node %d.\n    ", statusFrame, name, statusStrand, statusNode);
            }
        case 2:
            if (statusStrand == -1) {
                return wxString::Format("Rendering layer effect for frame %d of %s, layer %d.\n    ", statusFrame, name, statusLayer) + statusMap->AsString();
            } else if (statusNode == -1) {
                return wxString::Format("Rendering strand effect for frame %d of %s, strand %d.\n    ", statusFrame, name, statusStrand) + statusMap->AsString();
            } else {
                return wxString::Format("Rendering node effect for frame %d of %s, strand %d, node %d.\n    ", statusFrame, name, statusLayer, statusNode) + statusMap->AsString();
            }
        case 3:
            if (statusStrand == -1) {
                return wxString::Format("Calculating output at frame %d for %s.\n    ", statusFrame, name);
            } else if (statusNode == -1) {
                return wxString::Format("Calculating output at frame %d for %s, strand %d.\n    ", statusFrame, name, statusStrand);
            } else {
                return wxString::Format("Calculating output at frame %d for %s, strand %d, node %d.\n    ", statusFrame, name, statusStrand, statusNode);
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
        sb->init(xLights->GetModel(mainBuffer->GetModelName())->GetChanCount(), seqData->NumFrames(), seqData->FrameTime());
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

    virtual void Process() {
        SetGenericStatus("Initializing rendering thread for %s\n", 0);
        //printf("Starting rendering %lx (no next)\n", (unsigned long)this);
        int maxFrameBeforeCheck = -1;
        int origChangeCount;
        int ss, es;

        rowToRender->IncWaitCount();
        std::unique_lock<std::recursive_mutex> lock(rowToRender->GetRenderLock());
        rowToRender->DecWaitCount();
        SetGenericStatus("Got lock on rendering thread for %s\n", 0);

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

        int numLayers = rowToRender->GetEffectLayerCount();
        std::vector<Effect*> currentEffects(numLayers, nullptr);
        std::vector<SettingsMap> settingsMaps(numLayers);
        std::vector<bool> effectStates(numLayers);
        std::vector<bool> validLayers(numLayers);
        std::map<int, Effect*> strandEffects;
        std::map<int, SettingsMap> strandSettingsMaps;
        std::map<int, bool> strandEffectStates;
        std::map<SNPair, Effect*> nodeEffects;
        std::map<SNPair, SettingsMap> nodeSettingsMaps;
        std::map<SNPair, bool> nodeEffectStates;

        if (clearAllFrames && mainBuffer != NULL) {
            mainBuffer->Clear(0);
        }

        try {
            for (int layer = 0; layer < numLayers; layer++) {
                SetStatus(wxString::Format("Finding starting effect for %s, layer %d and startFrame %d", name, layer, startFrame));
                currentEffects[layer] = findEffectForFrame(layer, startFrame);
                SetStatus(wxString::Format("Initializing starting effect for %s, layer %d and startFrame %d", name, layer, startFrame));
                initialize(layer, startFrame, currentEffects[layer], settingsMaps[layer], mainBuffer);
                effectStates[layer] = true;
            }

            for (int frame = startFrame; frame < endFrame; frame++) {
                SetGenericStatus("%s: Starting frame %d\n", frame);

                if (next == nullptr &&
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
                bool effectsToUpdate = false;
                for (int layer = 0; layer < numLayers; layer++) {
                    SetGenericStatus("%s: Starting frame %d, layer %d\n", frame, layer);
                    Effect *el = findEffectForFrame(layer, frame);
                    if (el != currentEffects[layer]) {
                        currentEffects[layer] = el;
                        SetInializingStatus(frame, layer);
                        initialize(layer, frame, el, settingsMaps[layer], mainBuffer);
                        effectStates[layer] = true;
                    }
                    bool persist = settingsMaps[layer].GetBool(CHECKBOX_OverlayBkg);
                    if (!persist || STR_NONE == settingsMaps[layer][STR_EFFECT]) {
                        mainBuffer->Clear(layer);
                    }
                    SetRenderingStatus(frame, &settingsMaps[layer], layer);
                    bool b = effectStates[layer];
                    validLayers[layer] = xLights->RenderEffectFromMap(el, layer, frame, settingsMaps[layer], *mainBuffer, b, true, &renderEvent);
                    effectStates[layer] = b;
                    effectsToUpdate |= validLayers[layer];
                }
                if (!effectsToUpdate && clearAllFrames) {
                    validLayers[0] = true;
                    effectsToUpdate = true;
                }
                if (effectsToUpdate) {
                    SetCalOutputStatus(frame);
                    mainBuffer->CalcOutput(frame, validLayers);
                    size_t nodeCnt = mainBuffer->GetNodeCount();
                    for(size_t n = 0; n < nodeCnt; n++) {
                        int start = mainBuffer->NodeStartChannel(n);
                        mainBuffer->GetNodeChannelValues(n, &((*seqData)[frame][start]));
                    }
                }
                if (!strandBuffers.empty()) {
                    for (std::map<int, PixelBufferClassPtr>::iterator it = strandBuffers.begin(); it != strandBuffers.end(); it++) {
                        int strand = it->first;

                        PixelBufferClass *buffer = it->second.get();
                        StrandLayer *slayer = rowToRender->GetStrandLayer(strand);
                        Effect *el = findEffectForFrame(slayer, frame);
                        if (el != strandEffects[strand] || frame == startFrame) {
                            strandEffects[strand] = el;
                            SetInializingStatus(frame, -1, strand);
                            initialize(0, frame, el, strandSettingsMaps[strand], buffer);
                            strandEffectStates[strand] = true;
                        }
                        bool persist=strandSettingsMaps[strand].GetBool(CHECKBOX_OverlayBkg);
                        if (!persist || STR_NONE == strandSettingsMaps[strand][STR_EFFECT]) {
                            buffer->Clear(0);
                        }
                        SetRenderingStatus(frame, &strandSettingsMaps[strand], -1, strand);

                        if (xLights->RenderEffectFromMap(el, 0, frame, strandSettingsMaps[strand], *buffer, strandEffectStates[strand], true, &renderEvent)) {
                            //copy to output
                            std::vector<bool> valid(2, true);
                            SetCalOutputStatus(frame, strand);
                            buffer->SetColors(1, &((*seqData)[frame][0]));
                            buffer->CalcOutput(frame, valid);
                            size_t nodeCnt = buffer->GetNodeCount();
                            for(size_t n = 0; n < nodeCnt; n++) {
                                int start = buffer->NodeStartChannel(n);
                                buffer->GetNodeChannelValues(n, &((*seqData)[frame][start]));
                            }
                        }
                    }
                }
                if (!nodeBuffers.empty()) {
                    for (std::map<SNPair, PixelBufferClassPtr>::iterator it = nodeBuffers.begin(); it != nodeBuffers.end(); it++) {
                        SNPair node = it->first;
                        PixelBufferClass *buffer = it->second.get();
                        int strand = node.strand;
                        int inode = node.node;
                        StrandLayer *slayer = rowToRender->GetStrandLayer(strand);
                        if (slayer == nullptr) {
                            //deleted strand
                            continue;
                        }
                        EffectLayer *nlayer = slayer->GetNodeLayer(inode, false);
                        if (nlayer == nullptr) {
                            //deleted node
                            continue;
                        }
                        Effect *el = findEffectForFrame(nlayer, frame);
                        if (el != nodeEffects[node] || frame == startFrame) {
                            nodeEffects[node] = el;
                            SetInializingStatus(frame, -1, strand, inode);
                            initialize(0, frame, el, nodeSettingsMaps[node], buffer);
                            nodeEffectStates[node] = true;
                        }
                        bool persist=nodeSettingsMaps[node].GetBool(CHECKBOX_OverlayBkg);
                        if (!persist || STR_NONE == nodeSettingsMaps[node][STR_EFFECT]) {
                            buffer->Clear(0);
                        }

                        SetRenderingStatus(frame, &nodeSettingsMaps[node], -1, strand, inode);
                        if (xLights->RenderEffectFromMap(el, 0, frame, nodeSettingsMaps[node], *buffer, nodeEffectStates[node], true, &renderEvent)) {
                            SetCalOutputStatus(frame, strand, inode);
                            //copy to output
                            std::vector<bool> valid(2, true);
                            buffer->SetColors(1, &((*seqData)[frame][0]));
                            buffer->CalcOutput(frame, valid);
                            size_t nodeCnt = buffer->GetNodeCount();
                            for(size_t n = 0; n < nodeCnt; n++) {
                                int start = buffer->NodeStartChannel(n);
                                buffer->GetNodeChannelValues(n, &((*seqData)[frame][start]));
                            }
                        }
                    }
                }
                if (next) {
                    SetGenericStatus("%s: Notifying next renderer of frame %d done\n", frame);
                    next->setPreviousFrameDone(frame);
                }
            }
        } catch ( std::exception &ex) {
            printf("Caught an exception %s\n", ex.what());
        } catch ( ... ) {
            printf("Caught an unknown exception\n");
        }
        if (next) {
            //make sure the previous has told us we're at the end.  If we return before waiting, the previous
            //may try sending the END_OF_RENDER_FRAME to us and we'll have been deleted
            SetGenericStatus("%s: Waiting on previous renderer for final frame\n", 0);
            waitForFrame(END_OF_RENDER_FRAME);

            //let the next know we're done
            SetGenericStatus("%s: Notifying next renderer of final frame\n", 0);
            next->setPreviousFrameDone(END_OF_RENDER_FRAME);
            xLights->CallAfter(&xLightsFrame::SetStatusText, wxString("Done Rendering " + rowToRender->GetName()));

        } else {
            xLights->CallAfter(&xLightsFrame::RenderDone);
        }
        //printf("Done rendering %lx (next %lx)\n", (unsigned long)this, (unsigned long)next);
    }

private:

    void initialize(int layer, int frame, Effect *el, SettingsMap &settingsMap, PixelBufferClass *buffer) {
        if (el == NULL) {
            settingsMap.clear();
            settingsMap[STR_EFFECT]=STR_NONE;
        } else {
            loadSettingsMap(el->GetEffectName(),
                            el,
                            settingsMap);
        }
        updateBufferPaletteFromMap(layer, el, buffer);
        updateBufferFromMap(layer, settingsMap, buffer);

        if (el != NULL) {
            buffer->SetTimes(layer, el->GetStartTimeMS(), el->GetEndTimeMS());
        }

    }
    void updateBufferPaletteFromMap(int layer, Effect *effect, PixelBufferClass *buffer) {
        xlColorVector newcolors;
        if (effect != nullptr) {
            effect->CopyPalette(newcolors);
        }
        buffer->SetPalette(layer, newcolors);
    }
    void updateBufferFromMap(int layer, SettingsMap& settingsMap, PixelBufferClass *buffer) {
        double fadeIn, fadeOut;
        fadeIn = settingsMap.GetDouble(TEXTCTRL_Fadein, 0.0);
        fadeOut = settingsMap.GetDouble(TEXTCTRL_Fadeout, 0.0);
        buffer->SetFadeTimes(layer, fadeIn, fadeOut);

		int effectBlur = settingsMap.GetInt(SLIDER_EffectBlur, 1);
		buffer->SetBlur(layer, effectBlur);

        int freq=settingsMap.GetInt(SLIDER_SparkleFrequency, 0);
        buffer->SetSparkle(layer, freq);
        
        int brightness = settingsMap.GetInt(SLIDER_Brightness, 100);
        buffer->SetBrightness(layer, brightness);
        
        int contrast=settingsMap.GetInt(SLIDER_Contrast, 0);
        buffer->SetContrast(layer, contrast);
        buffer->SetMixType(layer, settingsMap.Get(CHOICE_LayerMethod, STR_NORMAL));
        int effectMixThreshold=settingsMap.GetInt(SLIDER_EffectLayerMix, 0);
        buffer->SetMixThreshold(layer, effectMixThreshold, settingsMap.GetInt(CHECKBOX_LayerMorph, 0) != 0); //allow threshold to vary -DJ
        
        buffer->SetBufferType(layer,
                              settingsMap.Get(CHOICE_BufferStyle, STR_DEFAULT),
                              settingsMap.Get(CHOICE_BufferTransform, STR_NONE));
    }

    Effect *findEffectForFrame(EffectLayer* layer, int frame) {
        int time = frame * seqData->FrameTime();
        for (int e = 0; e < layer->GetEffectCount(); e++) {
            Effect *effect = layer->GetEffect(e);
            int st = effect->GetStartTimeMS();
            int et = effect->GetEndTimeMS();
            if (et > time && st <= time) {
                return effect;
            }
        }
        return NULL;
    }
    Effect *findEffectForFrame(int layer, int frame) {
        return findEffectForFrame(rowToRender->GetEffectLayer(layer), frame);
    }
    void loadSettingsMap(const wxString &effectName,
                         Effect *effect,
                         SettingsMap& settingsMap) {
        settingsMap.clear();
        settingsMap[STR_EFFECT]=effectName;


        effect->CopySettingsMap(settingsMap, true);
    }


    Element *rowToRender;
    std::string name;
    int startFrame;
    int endFrame;
    PixelBufferClass *mainBuffer;
    xLightsFrame *xLights;
    SequenceData *seqData;
    bool clearAllFrames;
    RenderEvent renderEvent;

    //stuff for handling the status;
    wxString statusMsg;
    const char *statusMsgChars;
    volatile int statusType;
    volatile int statusFrame;
    SettingsMap *statusMap;
    volatile int statusLayer;
    volatile int statusStrand;
    volatile int statusNode;

    std::map<int, PixelBufferClassPtr> strandBuffers;
    std::map<SNPair, PixelBufferClassPtr> nodeBuffers;
};


IMPLEMENT_DYNAMIC_CLASS(RenderCommandEvent, wxCommandEvent)

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

void xLightsFrame::RenderGridToSeqData() {
    int numRows = mSequenceElements.GetElementCount();
    if (numRows == 0) {
        //nothing to do....
        return;
    }
    RenderJob **noDepJobs = new RenderJob*[numRows];
    RenderJob **depJobs = new RenderJob*[numRows];
    NextRenderer wait;
    Element *lastRowEl = NULL;
    AggregatorRenderer aggregator(SeqData.NumFrames());
    int *channelsRendered = new int[SeqData.NumChannels()];
    for (int x = 0; x < SeqData.NumChannels(); x++) {
        channelsRendered[x] = -1;
    }

    int noDepsCount = 0;
    int depsCount = 0;
    aggregator.setNext(&wait);


    for (int row = 0; row < mSequenceElements.GetElementCount(); row++) {
        Element *rowEl = mSequenceElements.GetElement(row);
        if (rowEl->GetType() == "model" && rowEl != lastRowEl) {
            lastRowEl = rowEl;
            RenderJob *job = new RenderJob(rowEl, SeqData, this);
            job->setRenderRange(0, SeqData.NumFrames());

            bool hasDep = false;
            PixelBufferClass *buffer = job->getBuffer();
            if (buffer == nullptr) {
                delete job;
                continue;
            }
            size_t cn = buffer->GetChanCountPerNode();
            for (int node = 0; node < buffer->GetNodeCount(); node++) {
                int start = buffer->NodeStartChannel(node);
                for (int c = 0; c < cn; c++) {
                    int cnum = start + c;
                    if (cnum < SeqData.NumChannels()) {
                        if (channelsRendered[cnum] >= 0
                                && channelsRendered[cnum] != row) {
                            hasDep = true;
                        } else {
                            channelsRendered[cnum] = row;
                        }
                    }
                }
            }
            if (!hasDep) {
                //does not depend on anything above it, reorder to the beginning and let it render at full
                //speed with no waits
                noDepJobs[noDepsCount] = job;
                noDepsCount++;
                job->setPreviousFrameDone(END_OF_RENDER_FRAME);
                job->setNext(&aggregator);
                aggregator.incNumAggregated();
            } else {
                depJobs[depsCount] = job;
                job->setNext(&wait);
                if (depsCount > 0) {
                    depJobs[depsCount - 1]->setNext(depJobs[depsCount]);
                } else {
                    aggregator.setNext(job);
                }
                depsCount++;
            }

        }
    }
    for (int row = 0; row < noDepsCount; row++) {
        if (noDepJobs[row]) {
            jobPool.PushJob(noDepJobs[row]);
        }
    }
    for (int row = 0; row < depsCount; row++) {
        if (depJobs[row]) {
            jobPool.PushJob(depJobs[row]);
        }
    }
    if (depsCount > 0 || noDepsCount > 0) {
        //wait to complete
        while (!wait.checkIfDone(SeqData.NumFrames())) {
            wxYield();
        }
    }

    delete []channelsRendered;
    delete []depJobs;
    delete []noDepJobs;
    RenderDone();
}
void xLightsFrame::RenderDone() {
    mainSequencer->PanelEffectGrid->Refresh();
}
void xLightsFrame::RenderEffectForModel(const std::string &model, int startms, int endms, bool clear) {
    //printf("render model %d %d   %d\n", startms,endms, clear);
    RenderJob *job = NULL;
    Element * el = mSequenceElements.GetElement(model);
    if( el->GetType() != "timing") {
        job = new RenderJob(el, SeqData, this, false, clear);
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


    SeqExportDialog dialog(this);
    dialog.ModelExportTypes();
    bool ok;
    wxString filename;
    do {
        ok = true;
        if (dialog.ShowModal() == wxID_OK) {
            // validate inputs
            filename = dialog.TextCtrlFilename->GetValue();
            filename.Trim();
            if (filename.IsEmpty()) {
                ok = false;
                wxMessageBox(_("The file name cannot be empty"), _("ERROR"));
            }
        } else {
            return;
        }
    } while (!ok);
    EnableSequenceControls(false);
    wxString format=dialog.ChoiceFormat->GetStringSelection();
    wxStopWatch sw;
    wxString Out3=format.Left(3);

    if (Out3 == "LSP") {
        filename = filename + "_USER";
    }
    wxFileName oName(filename);
    oName.SetPath( CurrentDir );
    wxString fullpath;

    StatusBar1->SetStatusText(_("Starting Export for ") + format + "-" + Out3);
    wxYield();

    Element * el = mSequenceElements.GetElement(model);
    NextRenderer wait;
    RenderJob *job = new RenderJob(el, SeqData, this, true);
    SequenceData *data = job->createExportBuffer();
    int cpn = job->getBuffer()->GetChanCountPerNode();

    if (command.GetInt()) {
        job->setRenderRange(0, SeqData.NumFrames());
        job->setPreviousFrameDone(END_OF_RENDER_FRAME);
        job->setNext(&wait);
        jobPool.PushJob(job);
        //wait to complete
        while (!wait.checkIfDone(SeqData.NumFrames())) {
            wxYield();
        }
    } else {
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
        delete job;
    }



    if (Out3 == "Lcb") {
        oName.SetExt(_("lcb"));
        fullpath=oName.GetFullPath();
        WriteLcbFile(fullpath, data->NumChannels(), SeqData.NumFrames(), data);
    } else if (Out3 == "Vir") {
        oName.SetExt(_("vir"));
        fullpath=oName.GetFullPath();
        WriteVirFile(fullpath, data->NumChannels(), SeqData.NumFrames(), data);
    } else if (Out3 == "LSP") {
        oName.SetExt(_("xml"));
        fullpath=oName.GetFullPath();
        WriteLSPFile(fullpath, data->NumChannels(), SeqData.NumFrames(), data, cpn);
    } else if (Out3 == "HLS") {
        oName.SetExt(_("hlsnc"));
        fullpath=oName.GetFullPath();
        WriteHLSFile(fullpath, data->NumChannels(), SeqData.NumFrames(), data);
    } else if (Out3 == "Fal") {
        wxString tempstr = GetModel(model)->ModelStartChannel;
        int stChan = wxAtoi(tempstr);
        oName.SetExt(_("eseq"));
        fullpath=oName.GetFullPath();
        WriteFalconPiModelFile(fullpath, data->NumChannels(), SeqData.NumFrames(), data, stChan, data->NumChannels());
    }
    StatusBar1->SetStatusText(_("Finished writing model: " )+fullpath + wxString::Format(" in %ld ms ",sw.Time()));

    delete data;
    EnableSequenceControls(true);
}


bool xLightsFrame::RenderEffectFromMap(Effect *effectObj, int layer, int period, const SettingsMap& SettingsMap,
                                       PixelBufferClass &buffer, bool &resetEffectState,
                                       bool bgThread, RenderEvent *event) {
    bool retval=true;

    buffer.SetLayer(layer, period, resetEffectState);
    resetEffectState = false;
    int eidx = -1;
    if (effectObj != nullptr) {
        eidx = effectObj->GetEffectIndex();
    } else {
        const std::string &effectName = SettingsMap.Get(STR_EFFECT, STR_NONE);
        if (effectName != STR_NONE) {
            eidx = effectManager.GetEffectIndex(effectName);
        }
    }
    if (eidx >= 0) {
        RenderableEffect *reff = effectManager.GetEffect(eidx);
        if (reff == nullptr) {
            retval= false;
        } else if (!bgThread || reff->CanRenderOnBackgroundThread()) {
            reff->Render(effectObj, SettingsMap, buffer.BufferForLayer(layer));
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
