#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"
#include "../RenderBuffer.h"

#include <string>
#include <list>

#define VUMETER_OFFSET_MIN -100
#define VUMETER_OFFSET_MAX 100

#define VUMETER_GAIN_MIN -100
#define VUMETER_GAIN_MAX 100

struct NSVGimage;

class VUMeterEffect : public RenderableEffect
{
public:
    VUMeterEffect(int id);
    virtual ~VUMeterEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual void SetDefaultParameters() override;
    virtual void SetPanelStatus(Model* cls) override;
    virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual std::list<std::string> GetFileReferences(Model* model, const SettingsMap& SettingsMap) const override;
    virtual bool CleanupFileLocations(xLightsFrame* frame, SettingsMap& SettingsMap) override;

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_VUMeter_YOffset")
            return VUMETER_OFFSET_MIN;
        if (name == "E_VALUECURVE_VUMeter_Gain")
            return VUMETER_GAIN_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_VUMeter_YOffset")
            return VUMETER_OFFSET_MAX;
        if (name == "E_VALUECURVE_VUMeter_Gain")
            return VUMETER_GAIN_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
    static int DecodeType(const std::string& type);
    static int DecodeShape(const std::string& shape);

    void Render(RenderBuffer& buffer, SequenceElements* elements,
                int bars, const std::string& type, const std::string& timingtrack, int sensitivity, const std::string& shape, bool slowdownfalls, int startnote, int endnote,
                int xoffset, int yoffset, int gain, bool logarithmicX, const std::string& filter, bool regex, const std::string& svgFile, const std::string& audioFile);
    void RenderSpectrogramFrame(RenderBuffer& buffer, int bars, std::vector<float>& lastvalues, std::vector<float>& lastpeaks, std::list<int>& pauseuntilpeakfall, bool slowdownfalls, 
        int startnote, int endnote, int xoffset, int yoffset, bool peak, int peakhold, bool line, bool logarithmicX, bool circle, int gain, int sensitivity,
        std::list<std::vector<wxPoint>>& lineHistory, const std::string& audioFile) const;
    void RenderVolumeBarsFrame(RenderBuffer& buffer, int bars, int gain, const std::string& audioFile);
    void RenderWaveformFrame(RenderBuffer& buffer, int bars, int yoffset, int gain, bool frameDetail);
    void RenderTimingEventFrame(RenderBuffer& buffer, int bars, int type, std::string timingtrack, std::list<int>& timingmarks, const std::string& filter, bool regex);
    void RenderTimingEventTimedSweepFrame(RenderBuffer& buffer, int bars, int type, std::string timingtrack, int& nCount, const std::string& filter, bool regex);
    void RenderTimingEventTimedChaseFrame(RenderBuffer& buffer, int usebars, int nType, std::string timingtrack, int& nCount, const std::string& filter, bool regex);
    void RenderOnFrame(RenderBuffer& buffer, int gain, const std::string& audioFile);
    void RenderOnColourFrame(RenderBuffer& buffer, int gain, const std::string& audioFile);
    void RenderPulseFrame(RenderBuffer& buffer, int fadeframes, std::string timingtrack, int& lasttimingmark);
    void RenderTimingEventColourFrame(RenderBuffer& buffer, int& colourindex, std::string timingtrack, int sensitivity, const std::string& filter, bool regex);
    void RenderLevelColourFrame(RenderBuffer& buffer, int& colourindex, int sensitivity, int& lasttimingmark, int gain, const std::string& audioFile);
    void RenderIntensityWaveFrame(RenderBuffer& buffer, int bars, int gain, const std::string& audioFile);
    void RenderLevelPulseFrame(RenderBuffer& buffer, int fadeframes, int sensitivity, int& lasttimingmark, int gain, const std::string& audioFile);
    void RenderLevelJumpFrame(RenderBuffer& buffer, int fadeframes, int sensitivity, int& lasttimingmark, int gain, bool fullJump, float& lastVal, const std::string& audioFile);
    void RenderLevelBarFrame(RenderBuffer& buffer, int bars, int sensitivity, float& lastbar, int& colourindex, int gain, bool random, const std::string& audioFile);
    void RenderNoteLevelBarFrame(RenderBuffer& buffer, int bars, int sensitivity, float& lastbar, int& colourindex, int startNote, int endNote, int gain, bool random);
    void RenderLevelShapeFrame(RenderBuffer& buffer, const std::string& shape, float& lastsize, int scale, bool slowdownfalls, int xoffset, int yoffset, int usebars, int gain, NSVGimage* svgFile, const std::string& audioFile);
    void RenderTimingEventPulseFrame(RenderBuffer& buffer, int fadeframes, std::string timingtrack, float& lastsize, const std::string& filter, bool regex);
    void RenderTimingEventPulseColourFrame(RenderBuffer& buffer, int fadeframes, std::string timingtrack, float& lastsize, int& colourindex, const std::string& filter, bool regex);
    void RenderTimingEventBarFrame(RenderBuffer& buffer, int bars, std::string timingtrack, float& lastbar, int& colourindex, bool all, bool random, const std::string& filter, bool regex, bool bounce, int& lastDirection);
    void RenderNoteOnFrame(RenderBuffer& buffer, int startNote, int endNote, int gain, const std::string& audioFile);
    void RenderNoteLevelPulseFrame(RenderBuffer& buffer, int fadeframes, int sensitivity, int& lasttimingmark, int startNote, int endNote, int gain, const std::string& audioFile);
    void RenderNoteLevelJumpFrame(RenderBuffer& buffer, int fadeframes, int sensitivity, int& lasttimingmark, int startNote, int endNote, int gain, bool fullJump, float& lastsize, const std::string& audioFile);
    void RenderTimingEventJumpFrame(RenderBuffer& buffer, int fallframes, std::string timingtrack, float& lastval, bool useAudioLevel, int gain, const std::string& filter, bool regex, const std::string& audioFile);
    void RenderLevelPulseColourFrame(RenderBuffer& buffer, int fadeframes, int sensitivity, int& lasttimingmark, int& colourindex, int gain, const std::string& audioFile);
    void RenderDominantFrequencyColour(RenderBuffer& buffer, int sensitivity, int startNote, int endNote, bool gradient, const std::string& audioFile);

    void DrawBox(RenderBuffer& buffer, int startx, int endx, int starty, int endy, xlColor& color1);
    void DrawCircle(RenderBuffer& buffer, int x, int y, float radius, xlColor& color1);
    void DrawStar(RenderBuffer& buffer, int x, int y, float radius, xlColor& color1, int points);
    void DrawDiamond(RenderBuffer& buffer, int centerx, int centery, int size, xlColor& color1);
    void DrawSnowflake(RenderBuffer& buffer, int xc, int yc, double radius, int sides, xlColor color, double rotation = 0);
    void DrawHeart(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness = 1);
    void DrawTree(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness = 1);
    void DrawCandycane(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness = 1) const;
    void DrawCrucifix(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness = 1);
    void DrawPresent(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, int thickness = 1);
    void DrawSVG(RenderBuffer& buffer, int xc, int yc, double radius, xlColor color, NSVGimage* svgFile, int thickness = 1);

    Effect* GetTimingEvent(const std::string& timingTrack, uint32_t ms, const std::string& filter, bool regex);

    inline float ApplyGain(float value, int gain) const;
};
