#ifndef TWINKLEEFFECT_H
#define TWINKLEEFFECT_H

#include "RenderableEffect.h"


class TwinkleEffect : public RenderableEffect
{
    public:
        TwinkleEffect(int id);
        virtual ~TwinkleEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // TWINKLEEFFECT_H
