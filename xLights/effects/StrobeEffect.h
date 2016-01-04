#ifndef STROBEEFFECT_H
#define STROBEEFFECT_H

#include "RenderableEffect.h"


class StrobeEffect : public RenderableEffect
{
    public:
        StrobeEffect(int id);
        virtual ~StrobeEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // STROBEEFFECT_H
