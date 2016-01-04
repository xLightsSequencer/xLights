#ifndef DMXEFFECT_H
#define DMXEFFECT_H

#include "RenderableEffect.h"


class DMXEffect : public RenderableEffect
{
    public:
        DMXEffect(int id);
        virtual ~DMXEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // DMXEFFECT_H
