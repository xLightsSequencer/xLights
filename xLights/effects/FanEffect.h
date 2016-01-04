#ifndef FANEFFECT_H
#define FANEFFECT_H

#include "RenderableEffect.h"


class FanEffect : public RenderableEffect
{
    public:
        FanEffect(int id);
        virtual ~FanEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // FANEFFECT_H
