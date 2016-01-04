#ifndef GALAXYEFFECT_H
#define GALAXYEFFECT_H

#include "RenderableEffect.h"


class GalaxyEffect : public RenderableEffect
{
    public:
        GalaxyEffect(int id);
        virtual ~GalaxyEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);

    private:
};

#endif // GALAXYEFFECT_H
