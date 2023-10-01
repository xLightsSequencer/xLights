#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"

#include <list>
class PianoPanel;

#define PIANO_SCALE_MIN 0
#define PIANO_SCALE_MAX 100

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
    static std::vector<float> Parse(wxString& l);
    virtual void SetDefaultParameters() override;
    virtual void SetPanelStatus(Model* cls) override;
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

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Piano_Scale")
            return PIANO_SCALE_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }
    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Piano_Scale")
            return PIANO_SCALE_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
    virtual bool needToAdjustSettings(const std::string& version) override
    {
        return true;
    };
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    void SetPanelTimingTracks();

private:
    PianoPanel* _panel;
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
