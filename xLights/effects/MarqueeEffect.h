#ifndef MARQUEEEFFECT_H
#define MARQUEEEFFECT_H

#include "RenderableEffect.h"


class MarqueeEffect : public RenderableEffect
{
    public:
        MarqueeEffect(int id);
        virtual ~MarqueeEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // MARQUEEEFFECT_H
