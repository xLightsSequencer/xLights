#ifndef GALAXYEFFECT_H
#define GALAXYEFFECT_H

#include "RenderableEffect.h"


class GalaxyEffect : public RenderableEffect
{
    public:
        GalaxyEffect(int id);
        virtual ~GalaxyEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
        virtual int DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);

    private:
};

#endif // GALAXYEFFECT_H
