#ifndef FIREWORKSEFFECT_H
#define FIREWORKSEFFECT_H

#include "RenderableEffect.h"


class FireworksEffect : public RenderableEffect
{
    public:
        FireworksEffect(int id);
        virtual ~FireworksEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);

    private:
};

#endif // FIREWORKSEFFECT_H
