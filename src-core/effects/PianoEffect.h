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

#include <list>
#include <map>

class PianoEffect : public RenderableEffect
{
public:
    PianoEffect(int id);
    virtual ~PianoEffect();
    virtual bool CanBeRandom() override
    {
        return false;
    }
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    static std::vector<float> Parse(const std::string& l);
    virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
    virtual int GetColorSupportedCount() const override
    {
        return 5;
    }
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    // Currently not possible but I think changes could be made to make it support partial
    // virtual bool CanRenderPartialTimeInterval() const override { return true; }

    // Cached from Piano.json by OnMetadataLoaded().
    static int sStartMIDIDefault;
    static int sEndMIDIDefault;
    static bool sShowSharpsDefault;
    static std::string sTypeDefault;
    static int sScaleDefault;
    static int sScaleMin;
    static int sScaleMax;
    static std::string sMIDITrackDefault;
    static int sXOffsetDefault;
    static bool sFadeNotesDefault;

protected:
    virtual void OnMetadataLoaded() override;

private:
    void RenderPiano(RenderBuffer& buffer, SequenceElements* elements, const int startmidi, const int endmidi, const bool sharps, const std::string type, int scale, std::string MIDITrack, int xoffset, bool fadeNotes);
    void ReduceChannels(std::list<std::pair<float, float>>* pdata, int start, int end, bool sharps);
    void DrawTruePiano(RenderBuffer& buffer, std::list<std::pair<float, float>>* pdata, bool sharps, int start, int end, int scale, int xoffset, bool fadeNotes);
    void DrawBarsPiano(RenderBuffer& buffer, std::list<std::pair<float, float>>* pdata, bool sharps, int start, int end, int scale, int xoffset, bool fadeNotes);
    bool IsSharp(float f);
    bool KeyDown(std::list<std::pair<float, float>>* pdata, int ch);
    float GetKeyBrightness(std::list<std::pair<float, float>>* pdata, int ch);
    std::map<int, std::list<std::pair<float, float>>> LoadTimingTrack(const std::string& track, int intervalMS, bool fadeNotes);
    std::list<std::string> ExtractNotes(const std::string& label);
    int ConvertNote(const std::string& note);
};
