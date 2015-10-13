//
//  Render.cpp
//  xLights
//
//

#include "xLightsMain.h"
#include "RenderCommandEvent.h"

#include <map>
#include <memory>

#define END_OF_RENDER_FRAME INT_MAX

class RenderEvent {
public:
    RenderEvent() : mutex(), signal(mutex) {
    }
    wxMutex mutex;
    wxCondition signal;

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
    NextRenderer() : nextLock(), nextSignal(nextLock) {
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
        wxMutexLocker lock(nextLock);
        previousFrameDone = i;
        nextSignal.Broadcast();
    }
    int waitForFrame(int frame) {
        wxMutexLocker lock(nextLock);
        while (frame > previousFrameDone) {
            nextSignal.WaitTimeout(5);
        }
        return previousFrameDone;
    }
    bool checkIfDone(int frame, int timeout = 5) {
        wxMutexLocker lock(nextLock);
        return previousFrameDone >= frame;
    }
protected:
    wxMutex nextLock;
    wxCondition nextSignal;
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
            wxMutexLocker lock(nextLock);
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
            mainBuffer = new PixelBufferClass();
            if (xframe->InitPixelBuffer(name, *mainBuffer, rowToRender->GetEffectLayerCount(), zeroBased)) {

                for (int x = 0; x < row->getStrandLayerCount(); x++) {
                    StrandLayer *sl = row->GetStrandLayer(x);
                    if (sl -> GetEffectCount() > 0) {
                        strandBuffers[x].reset(new PixelBufferClass());
                        strandBuffers[x]->InitStrandBuffer(*mainBuffer, x, data.FrameTime());
                    }
                    for (int n = 0; n < sl->GetNodeLayerCount(); n++) {
                        if (n < mainBuffer->GetStrandLength(x)) {
                            EffectLayer *nl = sl->GetNodeLayer(n);
                            if (nl -> GetEffectCount() > 0) {
                                nodeBuffers[SNPair(x, n)].reset(new PixelBufferClass());
                                nodeBuffers[SNPair(x, n)]->InitNodeBuffer(*mainBuffer, x, n, data.FrameTime());
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
    void SetGenericStatus(const wxString &msg, int frame, int layer) {
        statusType = 5;
        statusMsg = msg;
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
    wxString GetStatus() {
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
        }
        return statusMsg;
    }
    SequenceData *createExportBuffer() {
        SequenceData *sb = new SequenceData();
        sb->init(mainBuffer->GetChanCount(), seqData->NumFrames(), seqData->FrameTime());
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
        wxMutexLocker lock(rowToRender->GetRenderLock());
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
                int persist=wxAtoi(settingsMaps[layer]["CHECKBOX_OverlayBkg"]);
                if (!persist || "None" == settingsMaps[layer]["Effect"]) {
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
                    int persist=wxAtoi(strandSettingsMaps[strand]["CHECKBOX_OverlayBkg"]);
                    if (!persist || "None" == strandSettingsMaps[strand]["Effect"]) {
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
                    int persist=wxAtoi(nodeSettingsMaps[node]["CHECKBOX_OverlayBkg"]);
                    if (!persist || "None" == nodeSettingsMaps[node]["Effect"]) {
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
        if (next) {
            //let the next know we're done
            SetGenericStatus("%s: Notifying next renderer of final frame\n", 0);
            next->setPreviousFrameDone(END_OF_RENDER_FRAME);
            //make sure the previous has told us we're at the end.  If we return before waiting, the previous
            //may try sending the END_OF_RENDER_FRAME to us and we'll have been deleted
            SetGenericStatus("%s: Waiting on previous renderer for final frame\n", 0);
            waitForFrame(END_OF_RENDER_FRAME);
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
            settingsMap["Effect"]="None";
        } else {
            loadSettingsMap(el->GetEffectName(),
                            el,
                            settingsMap);
        }
        updateBufferPaletteFromMap(layer, settingsMap, buffer);
        updateBufferFadesFromMap(layer, settingsMap, buffer);

        if (el != NULL) {
            buffer->SetTimes(layer, el->GetStartTimeMS(), el->GetEndTimeMS());
        }

        int freq=wxAtoi(settingsMap["SLIDER_SparkleFrequency"]);
        buffer->SetSparkle(layer, freq);

        wxString s = settingsMap["SLIDER_Brightness"];
        if (s == "") {
            buffer->SetBrightness(layer, 100);
        } else {
            int brightness = wxAtoi(s);
            buffer->SetBrightness(layer, brightness);
        }

        int contrast=wxAtoi(settingsMap["SLIDER_Contrast"]);
        buffer->SetContrast(layer, contrast);
        buffer->SetMixType(layer, settingsMap["CHOICE_LayerMethod"]);
        int effectMixThreshold=wxAtoi(settingsMap["SLIDER_EffectLayerMix"]);
        buffer->SetMixThreshold(layer, effectMixThreshold, wxAtoi(settingsMap["CHECKBOX_LayerMorph"]) != 0); //allow threshold to vary -DJ
    }

    void updateBufferPaletteFromMap(int layer, SettingsMap& settingsMap, PixelBufferClass *buffer) {
        xlColorVector newcolors;
        for (int i = 1; i <= 6; i++) {
            if (settingsMap[wxString::Format("CHECKBOX_Palette%d",i)] ==  "1") {
                newcolors.push_back(xlColor(settingsMap[wxString::Format("BUTTON_Palette%d",i)]));
            }
        }
        buffer->SetPalette(layer, newcolors);
    }
    void updateBufferFadesFromMap(int layer, SettingsMap& settingsMap, PixelBufferClass *buffer) {
        wxString tmpStr;
        double fadeIn, fadeOut;

        tmpStr = settingsMap["TEXTCTRL_Fadein"];
        tmpStr.ToDouble(&fadeIn);
        tmpStr = settingsMap["TEXTCTRL_Fadeout"];
        tmpStr.ToDouble(&fadeOut);

        buffer->SetFadeTimes(layer, fadeIn, fadeOut);
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
        settingsMap["Effect"]=effectName;


        effect->CopySettingsMap(settingsMap, true);
        effect->CopyPaletteMap(settingsMap, true);
    }


    Element *rowToRender;
    wxString name;
    int startFrame;
    int endFrame;
    PixelBufferClass *mainBuffer;
    xLightsFrame *xLights;
    SequenceData *seqData;
    bool clearAllFrames;
    RenderEvent renderEvent;

    //stuff for handling the status;
    wxString statusMsg;
    int statusType;
    int statusFrame;
    SettingsMap *statusMap;
    int statusLayer;
    int statusStrand;
    int statusNode;

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
    ev->signal.Broadcast();
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
            size_t cn = buffer->ChannelsPerNode();
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
void xLightsFrame::RenderEffectForModel(const wxString &model, int startms, int endms, bool clear) {
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
    wxString model = command.GetString();


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
        ModelClass *m = GetModelClass(model);
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
        wxString tempstr;
        long stChan = 1;
        wxXmlNode *modelNode = GetModelNode(model);
        if (modelNode != NULL) {
            tempstr=modelNode->GetAttribute("StartChannel","1");
            tempstr.ToLong(&stChan);
        }
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
    wxString effect=SettingsMap["Effect"];
    if (effect == "None") {
        retval = false;
    } else if (effect == "Off") {
        buffer.RenderOff();
    } else if (effect == "On") {
        buffer.RenderOn(effectObj,
                        wxAtoi(SettingsMap.Get("TEXTCTRL_Eff_On_Start", "100")),
                        wxAtoi(SettingsMap.Get("TEXTCTRL_Eff_On_End", "100")),
                        wxAtoi(SettingsMap.Get("CHECKBOX_On_Shimmer", "0")),
                        wxAtof(SettingsMap.Get("TEXTCTRL_On_Cycles", "1.0")));
    } else if (effect == "Bars") {
        buffer.RenderBars(wxAtoi(SettingsMap["SLIDER_Bars_BarCount"]),
                          BarEffectDirections.Index(SettingsMap["CHOICE_Bars_Direction"]),
                          SettingsMap["CHECKBOX_Bars_Highlight"]=="1",
                          SettingsMap["CHECKBOX_Bars_3D"]=="1",
                          wxAtof(SettingsMap.Get("TEXTCTRL_Bars_Cycles", "1.0")));
    } else if (effect == "Butterfly") {
        buffer.RenderButterfly(ButterflyEffectColors.Index(SettingsMap["CHOICE_Butterfly_Colors"]),
                               wxAtoi(SettingsMap["SLIDER_Butterfly_Style"]),
                               wxAtoi(SettingsMap["SLIDER_Butterfly_Chunks"]),
                               wxAtoi(SettingsMap["SLIDER_Butterfly_Skip"]),
                               ButterflyDirection.Index(SettingsMap["CHOICE_Butterfly_Direction"]),
                               wxAtoi(SettingsMap.Get("SLIDER_Butterfly_Speed", "10")));
    } else if (effect == "Circles") {
        buffer.RenderCircles(wxAtoi(SettingsMap["SLIDER_Circles_Count"]),
                             wxAtoi(SettingsMap["SLIDER_Circles_Size"]),
                             SettingsMap["CHECKBOX_Circles_Bounce"]=="1",
                             SettingsMap["CHECKBOX_Circles_Collide"]=="1",
                             SettingsMap["CHECKBOX_Circles_Random_m"]=="1",
                             SettingsMap["CHECKBOX_Circles_Radial"]=="1",
                             SettingsMap["CHECKBOX_Circles_Radial_3D"]=="1",
                             SettingsMap["CHECKBOX_Circles_Bubbles"]=="1",
                             buffer.BufferWi/2, buffer.BufferHt/2,
                             SettingsMap["CHECKBOX_Circles_Plasma"]=="1",
                             SettingsMap["CHECKBOX_Circles_Linear_Fade"]=="1",
                             wxAtoi(SettingsMap.Get("SLIDER_Circles_Speed", "10"))
                            );

    } else if (effect == "Color Wash") {
        buffer.RenderColorWash(effectObj,
                               SettingsMap["CHECKBOX_ColorWash_HFade"]=="1",
                               SettingsMap["CHECKBOX_ColorWash_VFade"]=="1",
                               wxAtof(SettingsMap.Get("TEXTCTRL_ColorWash_Cycles", "1.0")),
                               wxAtoi(SettingsMap.Get("CHECKBOX_ColorWash_EntireModel", "1")),
                               wxAtoi(SettingsMap.Get("SLIDER_ColorWash_X1", "-50")),
                               wxAtoi(SettingsMap.Get("SLIDER_ColorWash_Y1", "-50")),
                               wxAtoi(SettingsMap.Get("SLIDER_ColorWash_X2", "50")),
                               wxAtoi(SettingsMap.Get("SLIDER_ColorWash_Y2", "50")),
                               wxAtoi(SettingsMap.Get("CHECKBOX_ColorWash_Shimmer", "0")),
                               wxAtoi(SettingsMap.Get("CHECKBOX_ColorWash_CircularPalette", "0"))
                              );
    } else if (effect == "Curtain") {
        buffer.RenderCurtain(CurtainEdge.Index(SettingsMap["CHOICE_Curtain_Edge"]),
                             CurtainEffect.Index(SettingsMap["CHOICE_Curtain_Effect"]),
                             wxAtoi(SettingsMap["SLIDER_Curtain_Swag"]),
                             SettingsMap["CHECKBOX_Curtain_Repeat"]=="1",
                             wxAtof(SettingsMap.Get("TEXTCTRL_Curtain_Speed", "1.0")));
    } else if (effect == "Faces") {
        if (SettingsMap.Get("CHOICE_Faces_FaceDefinition", "Default") == "Rendered"
                && SettingsMap.Get("CHECKBOX_Faces_Outline", "") == "") {
            //3.x style Faces effect
            buffer.RenderFaces(SettingsMap["CHOICE_Faces_Phoneme"], "Auto", true);
        } else if (SettingsMap.Get("CHOICE_Faces_FaceDefinition", "Default") == XLIGHTS_PGOFACES_FILE) {
            buffer.RenderCoroFacesFromPGO(SettingsMap["CHOICE_Faces_Phoneme"],
                                          SettingsMap.Get("CHOICE_Faces_Eyes", "Auto"),
                                          SettingsMap.Get("CHECKBOX_Faces_Outline", "0") == "1");
        } else {
            buffer.RenderFaces(&mSequenceElements,
                               SettingsMap.Get("CHOICE_Faces_FaceDefinition", "Default"),
                               SettingsMap["CHOICE_Faces_Phoneme"],
                               SettingsMap["CHOICE_Faces_TimingTrack"],
                               SettingsMap["CHOICE_Faces_Eyes"],
                               SettingsMap["CHECKBOX_Faces_Outline"] == "1");
        }
    } else if (effect == "Fan") {
        buffer.RenderFan(wxAtoi(SettingsMap["SLIDER_Fan_CenterX"]),
                         wxAtoi(SettingsMap["SLIDER_Fan_CenterY"]),
                         wxAtoi(SettingsMap["SLIDER_Fan_Start_Radius"]),
                         wxAtoi(SettingsMap["SLIDER_Fan_End_Radius"]),
                         wxAtoi(SettingsMap["SLIDER_Fan_Start_Angle"]),
                         wxAtoi(SettingsMap["SLIDER_Fan_Revolutions"]),
                         wxAtoi(SettingsMap["SLIDER_Fan_Duration"]),
                         wxAtoi(SettingsMap["SLIDER_Fan_Accel"]),
                         SettingsMap["CHECKBOX_Fan_Reverse"]=="1",
                         SettingsMap["CHECKBOX_Fan_Blend_Edges"]=="1",
                         wxAtoi(SettingsMap["SLIDER_Fan_Num_Blades"]),
                         wxAtoi(SettingsMap["SLIDER_Fan_Blade_Width"]),
                         wxAtoi(SettingsMap["SLIDER_Fan_Blade_Angle"]),
                         wxAtoi(SettingsMap["SLIDER_Fan_Num_Elements"]),
                         wxAtoi(SettingsMap["SLIDER_Fan_Element_Width"]));
    } else if (effect == "Fire") {
        buffer.RenderFire(wxAtoi(SettingsMap["SLIDER_Fire_Height"]),
                          wxAtoi(SettingsMap["SLIDER_Fire_HueShift"]),
                          wxAtof(SettingsMap.Get("TEXTCTRL_Fire_GrowthCycles", "0.0")),
                          SettingsMap.Get("CHOICE_Fire_Location", "Bottom"));
    } else if (effect == "Fireworks") {
        buffer.RenderFireworks(wxAtoi(SettingsMap["SLIDER_Fireworks_Explosions"]),
                               wxAtoi(SettingsMap["SLIDER_Fireworks_Count"]),
                               wxAtoi(SettingsMap["SLIDER_Fireworks_Velocity"]),
                               wxAtoi(SettingsMap["SLIDER_Fireworks_Fade"]));
    } else if (effect == "Galaxy") {
        buffer.RenderGalaxy(wxAtoi(SettingsMap["SLIDER_Galaxy_CenterX"]),
                            wxAtoi(SettingsMap["SLIDER_Galaxy_CenterY"]),
                            wxAtoi(SettingsMap["SLIDER_Galaxy_Start_Radius"]),
                            wxAtoi(SettingsMap["SLIDER_Galaxy_End_Radius"]),
                            wxAtoi(SettingsMap["SLIDER_Galaxy_Start_Angle"]),
                            wxAtoi(SettingsMap["SLIDER_Galaxy_Revolutions"]),
                            wxAtoi(SettingsMap["SLIDER_Galaxy_Start_Width"]),
                            wxAtoi(SettingsMap["SLIDER_Galaxy_End_Width"]),
                            wxAtoi(SettingsMap["SLIDER_Galaxy_Duration"]),
                            wxAtoi(SettingsMap["SLIDER_Galaxy_Accel"]),
                            SettingsMap["CHECKBOX_Galaxy_Reverse"]=="1",
                            SettingsMap["CHECKBOX_Galaxy_Blend_Edges"]=="1",
                            SettingsMap["CHECKBOX_Galaxy_Inward"]=="1");
    } else if (effect == "Garlands") {
        buffer.RenderGarlands(wxAtoi(SettingsMap["SLIDER_Garlands_Type"]),
                              wxAtoi(SettingsMap["SLIDER_Garlands_Spacing"]),
                              wxAtof(SettingsMap.Get("TEXTCTRL_Garlands_Cycles", "1.0")),
                              SettingsMap.Get("CHOICE_Garlands_Direction", "Up"));
    } else if (effect == "Glediator") {
        buffer.RenderGlediator(SettingsMap["TEXTCTRL_Glediator_Filename"]);
    } else if (effect == "Life") {
        buffer.RenderLife(wxAtoi(SettingsMap["SLIDER_Life_Count"]),
                          wxAtoi(SettingsMap["SLIDER_Life_Seed"]),
                          wxAtoi(SettingsMap.Get("SLIDER_Life_Speed", "10")));
    } else if (effect == "Lightning") {
        buffer.RenderLightning(wxAtoi(SettingsMap["SLIDER_Number_Bolts"]),
                               wxAtoi(SettingsMap["SLIDER_Number_Segments"]),
                               SettingsMap["CHECKBOX_ForkedLightning"]=="1",
                               wxAtoi(SettingsMap["SLIDER_Lightning_TopX"]),
                               wxAtoi(SettingsMap["SLIDER_Lightning_TopY"]),
                               wxAtoi(SettingsMap["SLIDER_Lightning_BOTX"]),
                               wxAtoi(SettingsMap["SLIDER_Lightning_BOTY"]));
    } else if (effect == "Marquee") {
        buffer.RenderMarquee(wxAtoi(SettingsMap["SLIDER_Marquee_Band_Size"]),
                             wxAtoi(SettingsMap["SLIDER_Marquee_Skip_Size"]),
                             wxAtoi(SettingsMap["SLIDER_Marquee_Thickness"]),
                             wxAtoi(SettingsMap["SLIDER_Marquee_Stagger"]),
                             wxAtoi(SettingsMap.Get("SLIDER_Marquee_Speed", "1")),
                             SettingsMap["CHECKBOX_Marquee_Reverse"] == "1",
                             wxAtoi(SettingsMap["SLIDER_Marquee_ScaleX"]),
                             wxAtoi(SettingsMap["SLIDER_Marquee_ScaleY"]),
                             wxAtoi(SettingsMap["SLIDER_MarqueeXC"]),
                             wxAtoi(SettingsMap["SLIDER_MarqueeYC"]),
                             SettingsMap["CHECKBOX_Marquee_PixelOffsets"] == "1",
                             SettingsMap["CHECKBOX_Marquee_WrapX"] == "1");
    } else if (effect == "Meteors") {
        buffer.RenderMeteors(MeteorsEffectTypes.Index(SettingsMap["CHOICE_Meteors_Type"]),
                             wxAtoi(SettingsMap["SLIDER_Meteors_Count"]),
                             wxAtoi(SettingsMap["SLIDER_Meteors_Length"]),
                             MeteorsEffect.Index(SettingsMap["CHOICE_Meteors_Effect"]),
                             wxAtoi(SettingsMap["SLIDER_Meteors_Swirl_Intensity"]),
                             wxAtoi(SettingsMap.Get("SLIDER_Meteors_Speed", "10")));
    } else if (effect == "Morph") {
        buffer.RenderMorph( wxAtoi(SettingsMap["SLIDER_Morph_Start_X1"]),
                            wxAtoi(SettingsMap["SLIDER_Morph_Start_Y1"]),
                            wxAtoi(SettingsMap["SLIDER_Morph_Start_X2"]),
                            wxAtoi(SettingsMap["SLIDER_Morph_Start_Y2"]),
                            wxAtoi(SettingsMap["SLIDER_Morph_End_X1"]),
                            wxAtoi(SettingsMap["SLIDER_Morph_End_Y1"]),
                            wxAtoi(SettingsMap["SLIDER_Morph_End_X2"]),
                            wxAtoi(SettingsMap["SLIDER_Morph_End_Y2"]),
                            wxAtoi(SettingsMap["SLIDER_MorphStartLength"]),
                            wxAtoi(SettingsMap["SLIDER_MorphEndLength"]),
                            SettingsMap["CHECKBOX_Morph_Start_Link"] == "1",
                            SettingsMap["CHECKBOX_Morph_End_Link"] == "1",
                            wxAtoi(SettingsMap["SLIDER_MorphDuration"]),
                            wxAtoi(SettingsMap["SLIDER_MorphAccel"]),
                            SettingsMap["CHECKBOX_ShowHeadAtStart"]=="1",
                            wxAtoi(SettingsMap["SLIDER_Morph_Repeat_Count"]),
                            wxAtoi(SettingsMap["SLIDER_Morph_Repeat_Skip"]),
                            wxAtoi(SettingsMap["SLIDER_Morph_Stagger"]));
    } else if(effect == "Piano") {
        buffer.RenderPiano(PianoEffectStyles.Index(SettingsMap["CHOICE_Piano_Style"]),
                           wxAtoi(SettingsMap["SLIDER_Piano_NumKeys"]),
                           wxAtoi(SettingsMap["SLIDER_Piano_NumRows"]),
                           PianoKeyPlacement.Index(SettingsMap["CHOICE_Piano_Placement"]),
                           SettingsMap["CHECKBOX_Piano_Clipping"] == "1",
                           SettingsMap["TEXTCTRL_Piano_CueFilename"],
                           SettingsMap["TEXTCTRL_Piano_MapFilename"],
                           SettingsMap["TEXTCTRL_Piano_ShapeFilename"]);
    } else if (effect == "Pictures") {
        buffer.RenderPictures(PictureEffectDirections.Index(SettingsMap["CHOICE_Pictures_Direction"]),
                              SettingsMap["FILEPICKER_Pictures_Filename"],
                              wxAtof(SettingsMap.Get("TEXTCTRL_Pictures_Speed", "1.0")),
                              wxAtof(SettingsMap.Get("TEXTCTRL_Pictures_FrameRateAdj", "1.0")),
                              wxAtoi(SettingsMap["SLIDER_PicturesXC"]),
                              wxAtoi(SettingsMap["SLIDER_PicturesYC"]),
                              wxAtoi(SettingsMap["SLIDER_PicturesEndXC"]),
                              wxAtoi(SettingsMap["SLIDER_PicturesEndYC"]),
                              SettingsMap["CHECKBOX_Pictures_PixelOffsets"] == "1",
                              SettingsMap["CHECKBOX_Pictures_WrapX"] == "1"
                             );
    } else if (effect == "Pinwheel") {
        buffer.RenderPinwheel(wxAtoi(SettingsMap["SLIDER_Pinwheel_Arms"]),
                              wxAtoi(SettingsMap["SLIDER_Pinwheel_Twist"]),
                              wxAtoi(SettingsMap["SLIDER_Pinwheel_Thickness"]),
                              SettingsMap["CHECKBOX_Pinwheel_Rotation"] == "1",
                              SettingsMap["CHOICE_Pinwheel_3D"],
                              wxAtoi(SettingsMap["SLIDER_PinwheelXC"]),
                              wxAtoi(SettingsMap["SLIDER_PinwheelYC"]),
                              wxAtoi(SettingsMap["SLIDER_Pinwheel_ArmSize"]),
                              wxAtoi(SettingsMap["TEXTCTRL_Pinwheel_Speed"]));
    } else if (effect == "Plasma") {
        buffer.RenderPlasma(PlasmaEffectColors.Index(SettingsMap["CHOICE_Plasma_Color"]),
                            wxAtoi(SettingsMap["SLIDER_Plasma_Style"]),
                            wxAtoi(SettingsMap["SLIDER_Plasma_Line_Density"]),
                            ButterflyDirection.Index(SettingsMap["CHOICE_Plasma_Direction"]),
                            wxAtoi(SettingsMap.Get("SLIDER_Plasma_Speed", "10")));
    } else if (effect == "Ripple") {
        buffer.RenderRipple(RippleObjectToDraw.Index(SettingsMap["CHOICE_Ripple_Object_To_Draw"]),
                            RippleMovement.Index(SettingsMap["CHOICE_Ripple_Movement"]),
                            wxAtoi(SettingsMap["SLIDER_Ripple_Thickness"]),
                            SettingsMap["CHECKBOX_Ripple3D"] == "1" ,
                            wxAtof(SettingsMap.Get("TEXTCTRL_Ripple_Cycles", "1.0")));
    } else if (effect == "Shimmer") {
        buffer.RenderShimmer(wxAtoi(SettingsMap["SLIDER_Shimmer_Duty_Factor"]),
                             SettingsMap["CHECKBOX_Shimmer_Use_All_Colors"]=="1",
                             wxAtof(SettingsMap.Get("TEXTCTRL_Shimmer_Cycles", "1.0")));
    } else if (effect == "Shockwave") {
        buffer.RenderShockwave(wxAtoi(SettingsMap["SLIDER_Shockwave_CenterX"]),
                               wxAtoi(SettingsMap["SLIDER_Shockwave_CenterY"]),
                               wxAtoi(SettingsMap["SLIDER_Shockwave_Start_Radius"]),
                               wxAtoi(SettingsMap["SLIDER_Shockwave_End_Radius"]),
                               wxAtoi(SettingsMap["SLIDER_Shockwave_Start_Width"]),
                               wxAtoi(SettingsMap["SLIDER_Shockwave_End_Width"]),
                               wxAtoi(SettingsMap["SLIDER_Shockwave_Accel"]),
                               SettingsMap["CHECKBOX_Shockwave_Blend_Edges"]=="1");
    } else if (effect == "SingleStrand") {
        if ("Skips" == SettingsMap["NOTEBOOK_SSEFFECT_TYPE"]) {
            buffer.RenderSingleStrandSkips(effectObj,
                                           wxAtoi(SettingsMap["SLIDER_Skips_BandSize"]),
                                           wxAtoi(SettingsMap["SLIDER_Skips_SkipSize"]),
                                           wxAtoi(SettingsMap["SLIDER_Skips_StartPos"]),
                                           SettingsMap["CHOICE_Skips_Direction"],
                                           wxAtoi(SettingsMap.Get("SLIDER_Skips_Advance", "0")));
        } else {
            buffer.RenderSingleStrandChase(SettingsMap.Get("CHOICE_SingleStrand_Colors", "Palette"),
                                           wxAtoi(SettingsMap["SLIDER_Number_Chases"]),
                                           wxAtoi(SettingsMap["SLIDER_Color_Mix1"]),
                                           SettingsMap.Get("CHOICE_Chase_Type1", "Left-Right"),
                                           SettingsMap["CHECKBOX_Chase_3dFade1"]=="1",
                                           SettingsMap["CHECKBOX_Chase_Group_All"]=="1",
                                           wxAtof(SettingsMap.Get("TEXTCTRL_Chase_Rotations", "1.0")));
        }
    } else if (effect == "Snowflakes") {
        buffer.RenderSnowflakes(wxAtoi(SettingsMap["SLIDER_Snowflakes_Count"]),
                                wxAtoi(SettingsMap["SLIDER_Snowflakes_Type"]),
                                wxAtoi(SettingsMap.Get("SLIDER_Snowflakes_Speed", "10")),
                                SettingsMap["CHECKBOX_Snowflakes_Accumulate"]=="1");
    } else if (effect == "Snowstorm") {
        buffer.RenderSnowstorm(wxAtoi(SettingsMap["SLIDER_Snowstorm_Count"]),
                               wxAtoi(SettingsMap["SLIDER_Snowstorm_Length"]),
                               wxAtoi(SettingsMap.Get("SLIDER_Snowstorm_Speed", "10")));
    } else if (effect == "Spirals") {
        buffer.RenderSpirals(wxAtoi(SettingsMap["SLIDER_Spirals_Count"]),
                             wxAtof(SettingsMap.Get("TEXTCTRL_Spirals_Movement", "1.0")),
                             wxAtoi(SettingsMap["SLIDER_Spirals_Rotation"]),
                             wxAtoi(SettingsMap["SLIDER_Spirals_Thickness"]),
                             SettingsMap["CHECKBOX_Spirals_Blend"]=="1",
                             SettingsMap["CHECKBOX_Spirals_3D"]=="1",
                             SettingsMap["CHECKBOX_Spirals_Grow"]=="1",
                             SettingsMap["CHECKBOX_Spirals_Shrink"]=="1"
                            );
    } else if (effect == "Spirograph") {
        buffer.RenderSpirograph(wxAtoi(SettingsMap["SLIDER_Spirograph_R"]),
                                wxAtoi(SettingsMap["SLIDER_Spirograph_r"]),
                                wxAtoi(SettingsMap["SLIDER_Spirograph_d"]),
                                wxAtoi(SettingsMap.Get("TEXTCTRL_Spirograph_Animate", "0")),
                                wxAtoi(SettingsMap.Get("TEXTCTRL_Spirograph_Speed", "10")),
                                wxAtoi(SettingsMap.Get("TEXTCTRL_Spirograph_Length", "20")));
    } else if (effect == "Strobe") {
        buffer.RenderStrobe(wxAtoi(SettingsMap["SLIDER_Number_Strobes"]),
                            wxAtoi(SettingsMap["SLIDER_Strobe_Duration"]),
                            wxAtoi(SettingsMap["SLIDER_Strobe_Type"]));
    } else if (effect == "Text") {
        // this needs to be on the primary thread due to GDI calls
#ifdef LINUX
        if (bgThread) {
#else
        if (bgThread && false) {
#endif // LINUX
            event->effect = effectObj;
            event->layer = layer;
            event->period = period;
            event->settingsMap = &SettingsMap;
            event->ResetEffectState = &resetEffectState;

            event->mutex.Lock();
            CallAfter(&xLightsFrame::RenderEffectOnMainThread, event);
            if (event->signal.Wait() == wxCOND_NO_ERROR) {
                retval = event->returnVal;
                event->mutex.Unlock();
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
        } else {
            buffer.RenderText(wxAtoi(SettingsMap["SLIDER_Text_Position1"]),
                              SettingsMap["TEXTCTRL_Text_Line1"],
                              SettingsMap["FONTPICKER_Text_Font1"],
                              TextEffectDirections.Index(SettingsMap["CHOICE_Text_Dir1"]),
                              wxAtoi(SettingsMap["CHECKBOX_TextToCenter1"]) != 0,
                              TextEffects.Index(SettingsMap["CHOICE_Text_Effect1"]),
                              TextCountDown.Index(SettingsMap["CHOICE_Text_Count1"]),
                              wxAtoi(SettingsMap.Get("TEXTCTRL_Text_Speed1", "10")),
                              //
                              wxAtoi(SettingsMap["SLIDER_Text_Position2"]),
                              SettingsMap["TEXTCTRL_Text_Line2"],
                              SettingsMap["FONTPICKER_Text_Font2"],
                              TextEffectDirections.Index(SettingsMap["CHOICE_Text_Dir2"]),
                              wxAtoi(SettingsMap["CHECKBOX_TextToCenter2"]) != 0,
                              TextEffects.Index(SettingsMap["CHOICE_Text_Effect2"]),
                              TextCountDown.Index(SettingsMap["CHOICE_Text_Count2"]),
                              wxAtoi(SettingsMap.Get("TEXTCTRL_Text_Speed2", "10")),
                              //
                              wxAtoi(SettingsMap["SLIDER_Text_Position3"]),
                              SettingsMap["TEXTCTRL_Text_Line3"],
                              SettingsMap["FONTPICKER_Text_Font3"],
                              TextEffectDirections.Index(SettingsMap["CHOICE_Text_Dir3"]),
                              wxAtoi(SettingsMap["CHECKBOX_TextToCenter3"]) != 0,
                              TextEffects.Index(SettingsMap["CHOICE_Text_Effect3"]),
                              TextCountDown.Index(SettingsMap["CHOICE_Text_Count3"]),
                              wxAtoi(SettingsMap.Get("TEXTCTRL_Text_Speed3", "10")),
                              //
                              wxAtoi(SettingsMap["SLIDER_Text_Position4"]),
                              SettingsMap["TEXTCTRL_Text_Line4"],
                              SettingsMap["FONTPICKER_Text_Font4"],
                              TextEffectDirections.Index(SettingsMap["CHOICE_Text_Dir4"]),
                              wxAtoi(SettingsMap["CHECKBOX_TextToCenter4"]) != 0,
                              TextEffects.Index(SettingsMap["CHOICE_Text_Effect4"]),
                              TextCountDown.Index(SettingsMap["CHOICE_Text_Count4"]),
                              wxAtoi(SettingsMap.Get("TEXTCTRL_Text_Speed4", "10"))
                             );
        }
    } else if (effect == "Tree") {
        buffer.RenderTree(wxAtoi(SettingsMap["SLIDER_Tree_Branches"]),
                          wxAtoi(SettingsMap.Get("SLIDER_Tree_Speed", "10")));
    } else if (effect == "Twinkle") {
        buffer.RenderTwinkle(wxAtoi(SettingsMap["SLIDER_Twinkle_Count"]),
                             wxAtoi(SettingsMap["SLIDER_Twinkle_Steps"]),
                             SettingsMap["CHECKBOX_Twinkle_Strobe"]=="1",
                             SettingsMap["CHECKBOX_Twinkle_ReRandom"] == "1");
    } else if (effect == "Wave") {
        buffer.RenderWave(WaveType.Index(SettingsMap["CHOICE_Wave_Type"]), //
                          FillColors.Index(SettingsMap["CHOICE_Fill_Colors"]),
                          SettingsMap["CHECKBOX_Mirror_Wave"]=="1",
                          wxAtoi(SettingsMap["SLIDER_Number_Waves"]),
                          wxAtoi(SettingsMap["SLIDER_Thickness_Percentage"]),
                          wxAtoi(SettingsMap["SLIDER_Wave_Height"]),
                          WaveDirection.Index(SettingsMap["CHOICE_Wave_Direction"]),
                          wxAtoi(SettingsMap.Get("TEXTCTRL_Wave_Speed", "10")));
    }

    return retval;
}
