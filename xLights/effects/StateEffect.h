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

#include <string>
class SequenceElements;

class StateEffect : public RenderableEffect
{
    public:
        StateEffect(int id);
        virtual ~StateEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void SetDefaultParameters() override;
        virtual void SetPanelStatus(Model *cls) override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        std::list<std::string> GetStates(Model* cls, std::string model);
        virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
        virtual bool CanRenderPartialTimeInterval() const override { return true; }
        std::list<std::string> GetStatesUsed(const SettingsMap& SettingsMap);
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
        void RenderState(RenderBuffer &buffer, SequenceElements *elements, const std::string &faceDefintion,
                         const std::string &Phoneme, const std::string &track, const std::string& mode, const std::string& colourmode);
    std::string FindState(std::map<std::string, std::string>& map, std::string name);
};
