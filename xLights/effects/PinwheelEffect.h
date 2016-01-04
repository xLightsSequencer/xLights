#ifndef PINWHEELEFFECT_H
#define PINWHEELEFFECT_H

#include "RenderableEffect.h"


class PinwheelEffect : public RenderableEffect
{
    public:
        PinwheelEffect(int id);
        virtual ~PinwheelEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // PINWHEELEFFECT_H
