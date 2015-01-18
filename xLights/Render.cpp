//
//  Render.cpp
//  xLights
//
//


#include "xLightsMain.h"


class RenderEvent
{
public:
    RenderEvent(int l, int p, MapStringString& sm,
                PixelBufferClass &b, bool *res) : mutex(), signal(mutex)
    {
        layer = l;
        period = p;
        SettingsMap = &sm;
        buffer = &b;
        ResetEffectState = res;
    }
    wxMutex mutex;
    wxCondition signal;
    volatile bool done = false;
    int layer;
    int period;
    MapStringString *SettingsMap;
    PixelBufferClass *buffer;
    bool *ResetEffectState;
    bool returnVal = false;
};


class RenderJob: public Job {
public:
    RenderJob(Element *row, wxXmlNode *modelNode, SequenceData &data, xLightsFrame *xframe)
        : Job(), rowToRender(*row), nextLock(), nextSignal(nextLock), seqData(data), xLights(xframe) {
        previousFrameDone = -1;
        if (row != NULL) {
            buffer = new PixelBufferClass();
            buffer->InitBuffer(modelNode, rowToRender.GetEffectLayerCount(), seqData.FrameTime(), false);
        } else {
            buffer = NULL;
        }
        startFrame = 0;
    }
    
    virtual ~RenderJob() {
        if (buffer != NULL) {
            delete buffer;
        }
    }
    
    void setRenderRange(int start, int end) {
        startFrame = start;
        endFrame = end;
    }
    void setPreviousFrameDone(int i) {
        wxMutexLocker lock(nextLock);
        previousFrameDone = i;
        nextSignal.Broadcast();
    }
    void setNext(RenderJob *n) {
        next = n;
    }
    
    virtual void Process() {
        int numLayers = rowToRender.GetEffectLayerCount();
        Effect *currentEffects[numLayers];
        MapStringString *settingsMaps = new MapStringString[numLayers];
        bool effectStates[numLayers];

        for (int layer = 0; layer < numLayers; layer++) {
            currentEffects[layer] = findEffectForFrame(layer, startFrame);
            initialize(layer, startFrame, currentEffects[layer], settingsMaps[layer]);
            effectStates[layer] = false;
        }
        
        for (int frame = startFrame; frame < endFrame; frame++) {
            bool validLayers[numLayers];
            //make sure we can do this frame
            waitForFrame(frame);
            bool effectsToUpdate = false;
            for (int layer = 0; layer < numLayers; layer++) {
                Effect *el = findEffectForFrame(layer, frame);
                if (el != currentEffects[layer]) {
                    currentEffects[layer] = el;
                    initialize(layer, frame, el, settingsMaps[layer]);
                    effectStates[layer] = false;
                }
                int persist=wxAtoi(settingsMaps[layer]["CHECKBOX_OverlayBkg"]);
                if (!persist || "None" == settingsMaps[layer]["Effect"]) {
                    buffer->Clear(layer);
                }
                
                validLayers[layer] = xLights->RenderEffectFromMap(layer, frame, settingsMaps[layer], *buffer, effectStates[layer], true);
                effectsToUpdate |= validLayers[layer];
            }
            
            if (effectsToUpdate) {
                buffer->CalcOutput(frame, validLayers);
                size_t chnum;
                wxByte intensity;
                size_t nodeCnt = buffer->GetNodeCount();
                size_t cn = buffer->ChannelsPerNode();
                for(size_t n = 0; n < nodeCnt; n++) {
                    for(size_t c = 0; c < cn; c++) {
                        buffer->GetChanIntensity(n, c, &chnum, &intensity);
                        seqData[frame][chnum]=intensity;
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
            xLights->CallAfter(&xLightsFrame::SetStatusText, wxString("Done Rendering " + rowToRender.GetName()));
        }
    }
    void waitForFrame(int frame) {
        wxMutexLocker lock(nextLock);
        while (frame >= previousFrameDone) {
            nextSignal.WaitTimeout(5);
        }
    }
    bool checkIfDone(int frame, int timeout = 5) {
        wxMutexLocker lock(nextLock);
        return previousFrameDone >= frame;
    }
    
private:
    void initialize(int layer, int frame, Effect *el, MapStringString &settingsMap) {
        if (el == NULL) {
            loadSettingsMap("None", "", settingsMap);
        } else {
            loadSettingsMap(el->GetEffectName(), el->GetSettings(), settingsMap);
        }
        updateBufferPaletteFromMap(layer, settingsMap);
        updateBufferFadesFromMap(layer, settingsMap);
        updateFitToTimeFromMap(layer, settingsMap);
        
        if (el != NULL) {
            buffer->SetTimes(layer, el->GetStartTime(), el->GetEndTime());
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
    
    void updateBufferPaletteFromMap(int layer, MapStringString& settingsMap) {
        wxColourVector newcolors;
        for (int i = 1; i <= 6; i++) {
            if (settingsMap[wxString::Format("CHECKBOX_Palette%d",i)] ==  "1") {
                newcolors.push_back(wxColour(settingsMap[wxString::Format("BUTTON_Palette%d",i)]));
            }
        }
        buffer->SetPalette(layer, newcolors);
    }
    void updateBufferFadesFromMap(int layer, MapStringString& settingsMap) {
        wxString tmpStr;
        double fadeIn, fadeOut;
        
        tmpStr = settingsMap["TEXTCTRL_Fadein"];
        tmpStr.ToDouble(&fadeIn);
        tmpStr = settingsMap["TEXTCTRL_Fadeout"];
        tmpStr.ToDouble(&fadeOut);
        
        buffer->SetFadeTimes(layer, fadeIn, fadeOut);
    }
    
    void updateFitToTimeFromMap(int layer, MapStringString& settingsMap) {
        bool fitToTime = settingsMap["CHECKBOX_FitToTime"] == "1";
        buffer->SetFitToTime(layer, fitToTime);
    }
    
    Effect *findEffectForFrame(int layer, int frame) {
        int time = frame * seqData.FrameTime();
        for (int e = 0; e < rowToRender.GetEffectLayer(layer)->GetEffectCount(); e++) {
            Effect *effect = rowToRender.GetEffectLayer(layer)->GetEffect(e);
            if ((effect->GetEndTime() * 1000) > time && (effect->GetStartTime() * 1000) <= time) {
                return effect;
            }
        }
        return NULL;
    }
    void loadSettingsMap(const wxString &effectName, wxString settings, MapStringString& settingsMap) {
        settingsMap.clear();
        settingsMap["Effect"]=effectName;

        wxString before,after,name,value;
        while (!settings.IsEmpty()) {
            before=settings.BeforeFirst(',');
            settings=settings.AfterFirst(',');
            
            name=before.BeforeFirst('=');
            if (name[1] == '_') {
                name = name.AfterFirst('_');
            }
            value=before.AfterFirst('=');
            settingsMap[name]=value;
        }
    }

    
    Element &rowToRender;
    volatile long previousFrameDone;
    
    int startFrame;
    int endFrame;
    PixelBufferClass *buffer;
    xLightsFrame *xLights;
    RenderJob *next;
    SequenceData &seqData;
    
    wxMutex nextLock;
    wxCondition nextSignal;
};

void xLightsFrame::RenderEffectOnMainThread(RenderEvent *ev) {
    wxMutexLocker(ev->mutex);
    ev->returnVal = RenderEffectFromMap(ev->layer, ev->period,
                                        *ev->SettingsMap,
                                        *ev->buffer, *ev->ResetEffectState, false);
    ev->done = true;
    ev->signal.Broadcast();
}

void xLightsFrame::RenderGridToSeqData() {
    int numRows = mSequenceElements.GetRowInformationSize();
    RenderJob **jobs = new RenderJob*[numRows];
    RenderJob wait(NULL, NULL, SeqData, this);
    RenderJob *last = NULL;
    for (int row = 0; row < mSequenceElements.GetRowInformationSize(); row++) {
        Row_Information_Struct* rowInfo = mSequenceElements.GetRowInformation(row);
        Element *rowEl = rowInfo->element;
        if (rowEl->GetType() == "model") {
            wxXmlNode *modelNode = GetModelNode(rowEl->GetName());
            jobs[row] = new RenderJob(rowEl, modelNode, SeqData, this);
            jobs[row]->setRenderRange(0, SeqData.NumFrames());
            if (last == NULL) {
                jobs[row]->setPreviousFrameDone(SeqData.NumFrames() + 1);
            } else {
                last->setNext(jobs[row]);
            }
            last = jobs[row];
        } else {
            jobs[row] = NULL;
        }
    }
    if (last) {
        last->setNext(&wait);
    }
    for (int row = 0; row < numRows; row++) {
        if (jobs[row]) {
            jobPool.PushJob(jobs[row]);
        }
    }
    //wait to complete
    while (!wait.checkIfDone(SeqData.NumFrames())) {
        wxYield();
    }
    
    delete []jobs;
}

void xLightsFrame::RenderEffectForModel(const wxString &model, int startms, int endms) {
    RenderJob *job = NULL;
    Element * el = mSequenceElements.GetElement(model);
    wxXmlNode *modelNode = GetModelNode(model);
    job = new RenderJob(el, modelNode, SeqData, this);
    job->setRenderRange(0, SeqData.NumFrames());
    job->setPreviousFrameDone(SeqData.NumFrames() + 1);
    jobPool.PushJob(job);
}




bool xLightsFrame::RenderEffectFromMap(int layer, int period, MapStringString& SettingsMap,
                                       PixelBufferClass &buffer, bool &resetEffectState,
                                       bool bgThread) {
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
        buffer.RenderOn(wxAtoi(SettingsMap["SLIDER_Eff_On_Red"]),
                        wxAtoi(SettingsMap["SLIDER_Eff_On_Grn"]),
                        wxAtoi(SettingsMap["SLIDER_Eff_On_Blu"]) );
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
                             SettingsMap["CHECKBOX_Circles_Plasma"]=="1"
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
    } else if (effect == "Fire") {
        buffer.RenderFire(wxAtoi(SettingsMap["SLIDER_Fire_Height"]),
                          wxAtoi(SettingsMap["SLIDER_Fire_HueShift"]),
                          SettingsMap["CHECKBOX_Fire_GrowFire"]=="1");
    } else if (effect == "Fireworks") {
        buffer.RenderFireworks(wxAtoi(SettingsMap["SLIDER_Fireworks_Number_Explosions"]),
                               wxAtoi(SettingsMap["SLIDER_Fireworks_Count"]),
                               wxAtoi(SettingsMap["SLIDER_Fireworks_Velocity"]),
                               wxAtoi(SettingsMap["SLIDER_Fireworks_Fade"]));
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
    } else if (effect == "Piano") {
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
                              wxAtoi(SettingsMap["SLIDER_Pictures_GifType"]),
                              SettingsMap["CHECKBOX_MovieIs20FPS"] == "1"
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
            RenderEvent ev(layer, period, SettingsMap, buffer, &resetEffectState);
            GetEventHandler()->CallAfter(&xLightsFrame::RenderEffectOnMainThread, &ev);
            wxMutexLocker lock(ev.mutex);
            int cnt = 0;
            while (cnt < 50 && !ev.done) {
                ev.signal.WaitTimeout(2);
                cnt++;
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
