//
//  Render.cpp
//  xLights
//
//

#include "xLightsMain.h"
#include "RenderCommandEvent.h"

#include <map>
#include <memory>


class RenderEvent
{
public:
    RenderEvent() : mutex(), signal(mutex)
    {
    }
    wxMutex mutex;
    wxCondition signal;

    int layer;
    int period;
    const MapStringString *settingsMap;
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
        while (frame >= previousFrameDone) {
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
    AggregatorRenderer(int numFrames) : NextRenderer() {
        data = new int[numFrames + 20];
        for (int x = 0; x < (numFrames + 20); x++) {
            data[x] = 0;
        }
        max = 0;
        maxFrameSoFar = 0;
    }
    ~AggregatorRenderer() {
        delete [] data;
    }
    void incNumAggregated() {
        max++;
    }
    virtual void setPreviousFrameDone(int i) {
        if (i % 10 != 0 && i < maxFrameSoFar) {
            return;
        }
        wxMutexLocker lock(nextLock);
        if (i > maxFrameSoFar) {
            maxFrameSoFar = i;
        }
        data[i]++;
        if (data[i] == max) {
            previousFrameDone = i;
            if (next != NULL) {
                next->setPreviousFrameDone(previousFrameDone);
            }
        }
    }
private:
    int *data;
    int max;
    int maxFrameSoFar;
};

class RenderJob: public Job, public NextRenderer {
public:
    RenderJob(Element *row, SequenceData &data, xLightsFrame *xframe, bool zeroBased = false, bool clear = false)
        : Job(), NextRenderer(), rowToRender(row), seqData(&data), xLights(xframe) {
        if (row != NULL) {
            name = row->GetName();
            mainBuffer = new PixelBufferClass();
            xframe->InitPixelBuffer(name, *mainBuffer, rowToRender->GetEffectLayerCount(), zeroBased);
            
            for (int x = 0; x < row->getStrandLayerCount(); x++) {
                StrandLayer *sl = row->GetStrandLayer(x);
                if (sl -> GetEffectCount() > 0) {
                    strandBuffers[x].reset(new PixelBufferClass());
                    strandBuffers[x]->InitStrandBuffer(*mainBuffer, x);
                }
                for (int n = 0; n < sl->GetNodeLayerCount(); n++) {
                    EffectLayer *nl = sl->GetNodeLayer(n);
                    if (nl -> GetEffectCount() > 0) {
                        nodeBuffers[x * mainBuffer->GetNodeCount() + n].reset(new PixelBufferClass());
                        nodeBuffers[x * mainBuffer->GetNodeCount() + n]->InitNodeBuffer(*mainBuffer, x, n);
                    }
                }
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
        //printf("Starting rendering %lx (no next)\n", (unsigned long)this);
        int maxFrameBeforeCheck = -1;
        int origChangeCount = rowToRender->getChangeCount();
        int numLayers = rowToRender->GetEffectLayerCount();
        Effect *currentEffects[numLayers];
        MapStringString *settingsMaps = new MapStringString[numLayers];
        bool effectStates[numLayers];
        
        std::map<int, Effect*> strandEffects;
        std::map<int, MapStringString> strandSettingsMaps;
        std::map<int, bool> strandEffectStates;
        std::map<int, Effect*> nodeEffects;
        std::map<int, MapStringString> nodeSettingsMaps;
        std::map<int, bool> nodeEffectStates;
        
        if (clearAllFrames && mainBuffer != NULL) {
            mainBuffer->Clear(0);
        }

        for (int layer = 0; layer < numLayers; layer++) {
            currentEffects[layer] = findEffectForFrame(layer, startFrame);
            initialize(layer, startFrame, currentEffects[layer], settingsMaps[layer], mainBuffer);
            effectStates[layer] = true;
        }

        for (int frame = startFrame; frame < endFrame; frame++) {
            if (origChangeCount != rowToRender->getChangeCount()) {
                break;
            }
            bool validLayers[numLayers];
            //make sure we can do this frame
            if (frame >= maxFrameBeforeCheck) {
                maxFrameBeforeCheck = waitForFrame(frame);
            }
            bool effectsToUpdate = false;
            for (int layer = 0; layer < numLayers; layer++) {
                Effect *el = findEffectForFrame(layer, frame);
                if (el != currentEffects[layer]) {
                    currentEffects[layer] = el;
                    initialize(layer, frame, el, settingsMaps[layer], mainBuffer);
                    effectStates[layer] = true;
                }
                int persist=wxAtoi(settingsMaps[layer]["CHECKBOX_OverlayBkg"]);
                if (!persist || "None" == settingsMaps[layer]["Effect"]) {
                    mainBuffer->Clear(layer);
                }

                validLayers[layer] = xLights->RenderEffectFromMap(layer, frame, settingsMaps[layer], *mainBuffer, effectStates[layer], true, &renderEvent);
                effectsToUpdate |= validLayers[layer];
            }
            if (!effectsToUpdate && clearAllFrames) {
                validLayers[0] = true;
                effectsToUpdate = true;
            }
            if (effectsToUpdate) {
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
                        initialize(0, frame, el, strandSettingsMaps[strand], buffer);
                        strandEffectStates[strand] = true;
                    }
                    int persist=wxAtoi(strandSettingsMaps[strand]["CHECKBOX_OverlayBkg"]);
                    if (!persist || "None" == strandSettingsMaps[strand]["Effect"]) {
                        buffer->Clear(0);
                    }
                    
                    if (xLights->RenderEffectFromMap(0, frame, strandSettingsMaps[strand], *buffer, strandEffectStates[strand], true, &renderEvent)) {
                        //copy to output
                        bool valid[1] = {true};
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
                for (std::map<int, PixelBufferClassPtr>::iterator it = nodeBuffers.begin(); it != nodeBuffers.end(); it++) {
                    int node = it->first;
                    PixelBufferClass *buffer = it->second.get();
                    int strand = node / mainBuffer->GetNodeCount();
                    int inode = node % mainBuffer->GetNodeCount();
                    StrandLayer *slayer = rowToRender->GetStrandLayer(strand);
                    EffectLayer *nlayer = slayer->GetNodeLayer(inode);
                    Effect *el = findEffectForFrame(nlayer, frame);
                    if (el != nodeEffects[node] || frame == startFrame) {
                        nodeEffects[node] = el;
                        initialize(0, frame, el, nodeSettingsMaps[strand], buffer);
                        nodeEffectStates[strand] = true;
                    }
                    int persist=wxAtoi(nodeSettingsMaps[strand]["CHECKBOX_OverlayBkg"]);
                    if (!persist || "None" == nodeSettingsMaps[strand]["Effect"]) {
                        buffer->Clear(0);
                    }
                    
                    if (xLights->RenderEffectFromMap(0, frame, nodeSettingsMaps[strand], *buffer, nodeEffectStates[strand], true, &renderEvent)) {
                        //copy to output
                        bool valid[1] = {true};
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
                next->setPreviousFrameDone(frame);
            }
        }
        delete [] settingsMaps;
        if (next) {
            next->setPreviousFrameDone(endFrame);
            xLights->CallAfter(&xLightsFrame::SetStatusText, wxString("Done Rendering " + rowToRender->GetName()));
        }
        xLights->CallAfter(&xLightsFrame::RenderDone);
        //printf("Done rendering %lx (next %lx)\n", (unsigned long)this, (unsigned long)next);
    }

private:
    void initialize(int layer, int frame, Effect *el, MapStringString &settingsMap, PixelBufferClass *buffer) {
        if (el == NULL) {
            settingsMap.clear();
            settingsMap["Effect"]="None";
        } else {
            loadSettingsMap(el->GetEffectName(),
                            el->GetSettings(),
                            el->GetPaletteMap(),
                            settingsMap);
        }
        updateBufferPaletteFromMap(layer, settingsMap, buffer);
        updateBufferFadesFromMap(layer, settingsMap, buffer);
        updateFitToTimeFromMap(layer, settingsMap, buffer);

        if (el != NULL) {
            buffer->SetTimes(layer, el->GetStartTime() * 1000, el->GetEndTime() * 1000);
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

    void updateBufferPaletteFromMap(int layer, MapStringString& settingsMap, PixelBufferClass *buffer) {
        xlColorVector newcolors;
        for (int i = 1; i <= 6; i++) {
            if (settingsMap[wxString::Format("CHECKBOX_Palette%d",i)] ==  "1") {
                newcolors.push_back(xlColor(settingsMap[wxString::Format("BUTTON_Palette%d",i)]));
            }
        }
        buffer->SetPalette(layer, newcolors);
    }
    void updateBufferFadesFromMap(int layer, MapStringString& settingsMap, PixelBufferClass *buffer) {
        wxString tmpStr;
        double fadeIn, fadeOut;

        tmpStr = settingsMap["TEXTCTRL_Fadein"];
        tmpStr.ToDouble(&fadeIn);
        tmpStr = settingsMap["TEXTCTRL_Fadeout"];
        tmpStr.ToDouble(&fadeOut);

        buffer->SetFadeTimes(layer, fadeIn, fadeOut);
    }

    void updateFitToTimeFromMap(int layer, MapStringString& settingsMap, PixelBufferClass *buffer) {
        bool fitToTime = settingsMap["CHECKBOX_FitToTime"] == "1";
        buffer->SetFitToTime(layer, fitToTime);
    }

    Effect *findEffectForFrame(EffectLayer* layer, int frame) {
        int time = frame * seqData->FrameTime();
        for (int e = 0; e < layer->GetEffectCount(); e++) {
            Effect *effect = layer->GetEffect(e);
            if ((effect->GetEndTime() * 1000) > time && (effect->GetStartTime() * 1000) <= time) {
                return effect;
            }
        }
        return NULL;
    }
    Effect *findEffectForFrame(int layer, int frame) {
        return findEffectForFrame(rowToRender->GetEffectLayer(layer), frame);
    }
    void loadSettingsMap(const wxString &effectName,
                         const MapStringString &settings,
                         const MapStringString &colorPalette,
                         MapStringString& settingsMap) {
        settingsMap.clear();
        settingsMap["Effect"]=effectName;

        for (std::map<wxString,wxString>::const_iterator it=settings.begin(); it!=settings.end(); ++it) {
            wxString name = it->first;
            if (name[1] == '_') {
                name = name.AfterFirst('_');
            }
            settingsMap[name] = it->second;
        }
        for (std::map<wxString,wxString>::const_iterator it=colorPalette.begin(); it!=colorPalette.end(); ++it) {
            wxString name = it->first;
            if (name[1] == '_') {
                name = name.AfterFirst('_');
            }
            settingsMap[name] = it->second;
        }
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
    
    std::map<int, PixelBufferClassPtr> strandBuffers;
    std::map<int, PixelBufferClassPtr> nodeBuffers;
};


IMPLEMENT_DYNAMIC_CLASS(RenderCommandEvent, wxCommandEvent)

void xLightsFrame::RenderRange(RenderCommandEvent &evt) {
    if (evt.deleted) {
        selectedEffect = 0;
    }
    RenderEffectForModel(evt.model, evt.start * 1000,  evt.end * 1000, evt.clear);
}

void xLightsFrame::RenderEffectOnMainThread(RenderEvent *ev) {
    ev->returnVal = RenderEffectFromMap(ev->layer, ev->period,
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
            size_t cn = buffer->ChannelsPerNode();
            for (int node = 0; node < buffer->GetNodeCount(); node++) {
                int start = buffer->NodeStartChannel(node);
                for (int c = 0; c < cn; c++) {
                    int cnum = start + c;
                    if (cnum < SeqData.NumChannels()
                        && channelsRendered[cnum] >= 0
                        && channelsRendered[cnum] != row) {
                        hasDep = true;
                    } else {
                        channelsRendered[cnum] = row;
                    }
                }
            }
            if (!hasDep) {
                //does not depend on anything above it, reorder to the beginning and let it render at full
                //speed with no waits
                noDepJobs[noDepsCount] = job;
                noDepsCount++;
                job->setPreviousFrameDone(SeqData.NumFrames());
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
}
void xLightsFrame::RenderDone() {
    mainSequencer->PanelEffectGrid->Refresh();
}
void xLightsFrame::RenderEffectForModel(const wxString &model, int startms, int endms, bool clear) {
    //printf("render model %d %d   %d\n", startms,endms, clear);
    RenderJob *job = NULL;
    Element * el = mSequenceElements.GetElement(model);
    if( el->GetType() != "timing")
    {
        job = new RenderJob(el, SeqData, this, false, clear);

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
        job->setPreviousFrameDone(SeqData.NumFrames() + 1);
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

    if (Out3 == "LSP")
    {
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
    job->setRenderRange(0, SeqData.NumFrames());
    job->setPreviousFrameDone(SeqData.NumFrames() + 1);
    job->setNext(&wait);
    int cpn = job->getBuffer()->GetChanCountPerNode();
    jobPool.PushJob(job);

    //wait to complete
    while (!wait.checkIfDone(SeqData.NumFrames())) {
        wxYield();
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
        //    int cpn = ChannelsPerNode();
        WriteLSPFile(fullpath, data->NumChannels(), SeqData.NumFrames(), data, cpn);
        return;
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


bool xLightsFrame::RenderEffectFromMap(int layer, int period, const MapStringString& SettingsMap,
                                       PixelBufferClass &buffer, bool &resetEffectState,
                                       bool bgThread, RenderEvent *event) {
    bool retval=true;

    wxString SpeedStr=SettingsMap["SLIDER_Speed"];
    buffer.SetLayer(layer, period, wxAtoi(SpeedStr), resetEffectState);
    resetEffectState = false;
    wxString effect=SettingsMap["Effect"];
    if (effect == "None") {
        retval = false;
    } else if (effect == "Off") {
        buffer.RenderOff();
    } else if (effect == "On") {
        buffer.RenderOn(wxAtoi(SettingsMap.Get("TEXTCTRL_Eff_On_Start", "100")),
                        wxAtoi(SettingsMap.Get("TEXTCTRL_Eff_On_End", "100")));
    } else if (effect == "Bars") {
        buffer.RenderBars(wxAtoi(SettingsMap["SLIDER_Bars_BarCount"]),
                          BarEffectDirections.Index(SettingsMap["CHOICE_Bars_Direction"]),
                          SettingsMap["CHECKBOX_Bars_Highlight"]=="1",
                          SettingsMap["CHECKBOX_Bars_3D"]=="1");
    } else if (effect == "Butterfly") {
        buffer.RenderButterfly(ButterflyEffectColors.Index(SettingsMap["CHOICE_Butterfly_Colors"]),
                               wxAtoi(SettingsMap["SLIDER_Butterfly_Style"]),
                               wxAtoi(SettingsMap["SLIDER_Butterfly_Chunks"]),
                               wxAtoi(SettingsMap["SLIDER_Butterfly_Skip"]),
                               ButterflyDirection.Index(SettingsMap["CHOICE_Butterfly_Direction"]));
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
                             SettingsMap["CHECKBOX_Circles_Linear_Fade"]=="1"
                             );

    } else if (effect == "Color Wash") {
        buffer.RenderColorWash(SettingsMap["CHECKBOX_ColorWash_HFade"]=="1",
                               SettingsMap["CHECKBOX_ColorWash_VFade"]=="1",
                               wxAtoi(SettingsMap["SLIDER_ColorWash_Count"]));
    } else if (effect == "Curtain") {
        buffer.RenderCurtain(CurtainEdge.Index(SettingsMap["CHOICE_Curtain_Edge"]),
                             CurtainEffect.Index(SettingsMap["CHOICE_Curtain_Effect"]),
                             wxAtoi(SettingsMap["SLIDER_Curtain_Swag"]),
                             SettingsMap["CHECKBOX_Curtain_Repeat"]=="1");
    } else if (effect == "Faces") {
        buffer.RenderFaces(FacesPhoneme.Index(SettingsMap["CHOICE_Faces_Phoneme"]));
    } else if (effect == "CoroFaces") {
        buffer.RenderCoroFaces(SettingsMap["CHOICE_CoroFaces_Phoneme"],
                               SettingsMap["CHOICE_CoroFaces_Eyes"],
                               SettingsMap["CHECKBOX_CoroFaces_Outline"] == "1");
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
                          SettingsMap["CHECKBOX_Fire_GrowFire"]=="1");
    } else if (effect == "Fireworks") {
        buffer.RenderFireworks(wxAtoi(SettingsMap["SLIDER_Fireworks_Number_Explosions"]),
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
                              wxAtoi(SettingsMap["SLIDER_Garlands_Spacing"]));
    } else if (effect == "Glediator") {
        buffer.RenderGlediator(SettingsMap["TEXTCTRL_Glediator_Filename"]);
    } else if (effect == "Life") {
        buffer.RenderLife(wxAtoi(SettingsMap["SLIDER_Life_Count"]),
                          wxAtoi(SettingsMap["SLIDER_Life_Seed"]));
    } else if (effect == "Meteors") {
        buffer.RenderMeteors(MeteorsEffectTypes.Index(SettingsMap["CHOICE_Meteors_Type"]),
                             wxAtoi(SettingsMap["SLIDER_Meteors_Count"]),
                             wxAtoi(SettingsMap["SLIDER_Meteors_Length"]),
                             MeteorsEffect.Index(SettingsMap["CHOICE_Meteors_Effect"]),
                             wxAtoi(SettingsMap["SLIDER_Meteors_Swirl_Intensity"]));
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
                            0,                      // int tail_style,
                            SettingsMap["CHECKBOX_MorphUseHeadStartColor"]=="1",
                            SettingsMap["CHECKBOX_MorphUseHeadEndColor"]=="1",
                            SettingsMap["CHECKBOX_ShowHeadAtStart"]=="1");
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
                              SettingsMap["TEXTCTRL_Pictures_Filename"],
                              wxAtoi(SettingsMap["SLIDER_Pictures_GifSpeed"]),
                              SettingsMap["CHECKBOX_MovieIs20FPS"] == "1",
                              wxAtoi(SettingsMap["SLIDER_PicturesXC"]),
                              wxAtoi(SettingsMap["SLIDER_PicturesYC"]),
                              SettingsMap["CHECKBOX_Pictures_WrapX"] == "1"
                              );
    } else if (effect == "Pinwheel") {
        buffer.RenderPinwheel(wxAtoi(SettingsMap["SLIDER_Pinwheel_Arms"]),
                              wxAtoi(SettingsMap["SLIDER_Pinwheel_Twist"]),
                              wxAtoi(SettingsMap["SLIDER_Pinwheel_Thickness"]),
                              SettingsMap["CHECKBOX_Pinwheel_Rotation"] == "1",
                              wxAtoi(SettingsMap["CHOICE_Pinwheel_3D"]),
                              wxAtoi(SettingsMap["SLIDER_PinwheelXC"]),
                              wxAtoi(SettingsMap["SLIDER_PinwheelYC"]),
                              wxAtoi(SettingsMap["SLIDER_Pinwheel_ArmSize"]));
    } else if (effect == "Ripple") {
        buffer.RenderRipple(RippleObjectToDraw.Index(SettingsMap["CHOICE_Ripple_Object_To_Draw"]),
                            RippleMovement.Index(SettingsMap["CHOICE_Ripple_Movement"]),
                            wxAtoi(SettingsMap["SLIDER_Ripple_Thickness"]),
                            SettingsMap["CHECKBOX_Ripple3D"] == "1" );
    } else if (effect == "Shimmer") {
        buffer.RenderShimmer(wxAtoi(SettingsMap["SLIDER_Shimmer_Duty_Factor"]),
                             SettingsMap["CHECKBOX_Shimmer_Use_All_Colors"]=="1",
                             SettingsMap["CHECKBOX_Shimmer_Blink_Timing"]=="1",
                             wxAtoi(SettingsMap["SLIDER_Shimmer_Blinks_Per_Row"]));
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
            buffer.RenderSingleStrandSkips(
                                           wxAtoi(SettingsMap["SLIDER_Skips_BandSize"]),
                                           wxAtoi(SettingsMap["SLIDER_Skips_SkipSize"]),
                                           wxAtoi(SettingsMap["SLIDER_Skips_StartPos"]),
                                           SettingsMap["CHOICE_Skips_Direction"]);
        } else {
            buffer.RenderSingleStrandChase(
                                           SingleStrandColors.Index(SettingsMap["CHOICE_SingleStrand_Colors"]),
                                           wxAtoi(SettingsMap["SLIDER_Number_Chases"]),
                                           wxAtoi(SettingsMap["SLIDER_Color_Mix1"]),
                                           wxAtoi(SettingsMap["SLIDER_Chase_Spacing1"]),
                                           SingleStrandTypes.Index(SettingsMap["CHOICE_Chase_Type1"]),
                                           SettingsMap["CHECKBOX_Chase_3dFade1"]=="1",
                                           SettingsMap["CHECKBOX_Chase_Group_All"]=="1");
        }
    } else if (effect == "Snowflakes") {
        buffer.RenderSnowflakes(wxAtoi(SettingsMap["SLIDER_Snowflakes_Count"]),
                                wxAtoi(SettingsMap["SLIDER_Snowflakes_Type"]));
    } else if (effect == "Snowstorm") {
        buffer.RenderSnowstorm(wxAtoi(SettingsMap["SLIDER_Snowstorm_Count"]),
                               wxAtoi(SettingsMap["SLIDER_Snowstorm_Length"]));
    } else if (effect == "Spirals") {
        buffer.RenderSpirals(wxAtoi(SettingsMap["SLIDER_Spirals_Count"]),
                             wxAtoi(SettingsMap["SLIDER_Spirals_Direction"]),
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
                                wxAtoi(SettingsMap["SLIDER_Spirograph_Animate"]));
    } else if (effect == "Strobe") {
        buffer.RenderStrobe(wxAtoi(SettingsMap["SLIDER_Number_Strobes"]),
                            wxAtoi(SettingsMap["SLIDER_Strobe_Duration"]),
                            wxAtoi(SettingsMap["SLIDER_Strobe_Type"]));
    } else if (effect == "Text") {
        // this needs to be on the primary thread due to GDI calls
        if (bgThread) {
            event->layer = layer;
            event->period = period;
            event->settingsMap = &SettingsMap;
            event->ResetEffectState = &resetEffectState;

            event->mutex.Lock();
            CallAfter(&xLightsFrame::RenderEffectOnMainThread, event);
            if (event->signal.WaitTimeout(500) == wxCOND_NO_ERROR) {
                retval = event->returnVal;
                event->mutex.Unlock();
            } else {
                printf("HELP!!!!\n");
            }
        } else {
            buffer.RenderText(wxAtoi(SettingsMap["SLIDER_Text_Position1"]),
                              SettingsMap["TEXTCTRL_Text_Line1"],
                              SettingsMap["TEXTCTRL_Text_Font1"],
                              TextEffectDirections.Index(SettingsMap["CHOICE_Text_Dir1"]),
                              wxAtoi(SettingsMap["CHECKBOX_TextToCenter1"]) != 0,
                              TextEffects.Index(SettingsMap["CHOICE_Text_Effect1"]),
                              TextCountDown.Index(SettingsMap["CHOICE_Text_Count1"]),
                              //
                              wxAtoi(SettingsMap["SLIDER_Text_Position2"]),
                              SettingsMap["TEXTCTRL_Text_Line2"],
                              SettingsMap["TEXTCTRL_Text_Font2"],
                              TextEffectDirections.Index(SettingsMap["CHOICE_Text_Dir2"]),
                              wxAtoi(SettingsMap["CHECKBOX_TextToCenter2"]) != 0,
                              TextEffects.Index(SettingsMap["CHOICE_Text_Effect2"]),
                              TextCountDown.Index(SettingsMap["CHOICE_Text_Count2"]),
                              //
                              wxAtoi(SettingsMap["SLIDER_Text_Position3"]),
                              SettingsMap["TEXTCTRL_Text_Line3"],
                              SettingsMap["TEXTCTRL_Text_Font3"],
                              TextEffectDirections.Index(SettingsMap["CHOICE_Text_Dir3"]),
                              wxAtoi(SettingsMap["CHECKBOX_TextToCenter3"]) != 0,
                              TextEffects.Index(SettingsMap["CHOICE_Text_Effect3"]),
                              TextCountDown.Index(SettingsMap["CHOICE_Text_Count3"]),
                              //
                              wxAtoi(SettingsMap["SLIDER_Text_Position4"]),
                              SettingsMap["TEXTCTRL_Text_Line4"],
                              SettingsMap["TEXTCTRL_Text_Font4"],
                              TextEffectDirections.Index(SettingsMap["CHOICE_Text_Dir4"]),
                              wxAtoi(SettingsMap["CHECKBOX_TextToCenter4"]) != 0,
                              TextEffects.Index(SettingsMap["CHOICE_Text_Effect4"]),
                              TextCountDown.Index(SettingsMap["CHOICE_Text_Count4"]));
        }
    } else if (effect == "Tree") {
        buffer.RenderTree(wxAtoi(SettingsMap["SLIDER_Tree_Branches"]));
    } else if (effect == "Twinkle") {
        buffer.RenderTwinkle(wxAtoi(SettingsMap["SLIDER_Twinkle_Count"]),
                             wxAtoi(SettingsMap["SLIDER_Twinkle_Steps"]),
                             SettingsMap["CHECKBOX_Twinkle_Strobe"]=="1");
    } else if (effect == "Wave") {
        buffer.RenderWave(WaveType.Index(SettingsMap["CHOICE_Wave_Type"]), //
                          FillColors.Index(SettingsMap["CHOICE_Fill_Colors"]),
                          SettingsMap["CHECKBOX_Mirror_Wave"]=="1",
                          wxAtoi(SettingsMap["SLIDER_Number_Waves"]),
                          wxAtoi(SettingsMap["SLIDER_Thickness_Percentage"]),
                          wxAtoi(SettingsMap["SLIDER_Wave_Height"]),
                          WaveDirection.Index(SettingsMap["CHOICE_Wave_Direction"]));
    }

    return retval;
}
