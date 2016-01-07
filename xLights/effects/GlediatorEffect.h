#ifndef GLEDIATOREFFECT_H
#define GLEDIATOREFFECT_H

#include "RenderableEffect.h"


class GlediatorEffect : public RenderableEffect
{
    public:
        GlediatorEffect(int id);
        virtual ~GlediatorEffect();
        virtual bool CanBeRandom() {return false;}
        virtual void SetSequenceElements(SequenceElements *els);
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // GLEDIATOREFFECT_H
