#ifndef LIGHTNINGEFFECT_H
#define LIGHTNINGEFFECT_H

#include "RenderableEffect.h"


class LightningEffect : public RenderableEffect
{
    public:
        LightningEffect(int id);
        virtual ~LightningEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // LIGHTNINGEFFECT_H
