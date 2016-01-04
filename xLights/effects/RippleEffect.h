#ifndef RIPPLEEFFECT_H
#define RIPPLEEFFECT_H

#include "RenderableEffect.h"


class RippleEffect : public RenderableEffect
{
    public:
        RippleEffect(int id);
        virtual ~RippleEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // RIPPLEEFFECT_H
