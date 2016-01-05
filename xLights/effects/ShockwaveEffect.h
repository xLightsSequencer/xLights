#ifndef SHOCKWAVEEFFECT_H
#define SHOCKWAVEEFFECT_H

#include "RenderableEffect.h"


class ShockwaveEffect : public RenderableEffect
{
    public:
        ShockwaveEffect(int id);
        virtual ~ShockwaveEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // SHOCKWAVEEFFECT_H
