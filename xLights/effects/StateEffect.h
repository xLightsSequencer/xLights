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
        virtual void SetPanelStatus(Model *cls) override;
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        std::list<std::string> GetStates(Model* cls, std::string model);
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
        void RenderState(RenderBuffer &buffer, SequenceElements *elements, const std::string &faceDefintion,
                         const std::string &Phoneme, const std::string &track, const std::string& mode, const std::string& colourmode);
    std::string FindState(std::map<std::string, std::string>& map, std::string name);
};

#endif // StateEFFECT_H
