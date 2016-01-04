#ifndef WAVEEFFECT_H
#define WAVEEFFECT_H

#include "RenderableEffect.h"


class WaveEffect : public RenderableEffect
{
    public:
        WaveEffect(int id);
        virtual ~WaveEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // WAVEEFFECT_H
