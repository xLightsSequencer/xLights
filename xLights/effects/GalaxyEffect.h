#ifndef GALAXYEFFECT_H
#define GALAXYEFFECT_H

#include "RenderableEffect.h"


class GalaxyEffect : public RenderableEffect
{
    public:
        GalaxyEffect(int id);
        virtual ~GalaxyEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);

    private:
};

#endif // GALAXYEFFECT_H
