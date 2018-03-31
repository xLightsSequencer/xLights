#ifndef STATEEFFECT_H
#define STATEEFFECT_H

#include "RenderableEffect.h"

#include <string>
class SequenceElements;

class StateEffect : public RenderableEffect
{
    public:
        StateEffect(int id);
        virtual ~StateEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void SetPanelStatus(Model *cls) override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        std::list<std::string> GetStates(Model* cls, std::string model);
        virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
        virtual bool CanRenderPartialTimeInterval() const override { return true; }
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
        void RenderState(RenderBuffer &buffer, SequenceElements *elements, const std::string &faceDefintion,
                         const std::string &Phoneme, const std::string &track, const std::string& mode, const std::string& colourmode);
    std::string FindState(std::map<std::string, std::string>& map, std::string name);
};

#endif // StateEFFECT_H
