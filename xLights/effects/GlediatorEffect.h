#ifndef GLEDIATOREFFECT_H
#define GLEDIATOREFFECT_H

#include "RenderableEffect.h"


class GlediatorEffect : public RenderableEffect
{
    public:
        GlediatorEffect(int id);
        virtual ~GlediatorEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void SetSequenceElements(SequenceElements *els) override;
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // GLEDIATOREFFECT_H
