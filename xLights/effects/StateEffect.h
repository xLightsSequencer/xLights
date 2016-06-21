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
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
        void RenderState(RenderBuffer &buffer, SequenceElements *elements, const std::string &faceDefintion,
                         const std::string &Phoneme, const std::string &track);
};

#endif // StateEFFECT_H
