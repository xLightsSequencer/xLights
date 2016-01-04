#ifndef BUTTERFLYEFFECT_H
#define BUTTERFLYEFFECT_H

#include "RenderableEffect.h"


class ButterflyEffect : public RenderableEffect
{
    public:
        ButterflyEffect(int id);
        virtual ~ButterflyEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // BUTTERFLYEFFECT_H
