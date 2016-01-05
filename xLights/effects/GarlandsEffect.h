#ifndef GARLANDSEFFECT_H
#define GARLANDSEFFECT_H

#include "RenderableEffect.h"


class GarlandsEffect : public RenderableEffect
{
    public:
        GarlandsEffect(int id);
        virtual ~GarlandsEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);

    private:
};

#endif // GARLANDSEFFECT_H
