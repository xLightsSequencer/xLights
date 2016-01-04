#ifndef SHOCKWAVEEFFECT_H
#define SHOCKWAVEEFFECT_H

#include "RenderableEffect.h"


class ShockwaveEffect : public RenderableEffect
{
    public:
        ShockwaveEffect(int id);
        virtual ~ShockwaveEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // SHOCKWAVEEFFECT_H
