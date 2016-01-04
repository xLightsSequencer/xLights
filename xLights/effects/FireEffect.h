#ifndef FIREEFFECT_H
#define FIREEFFECT_H

#include "RenderableEffect.h"


class FireEffect : public RenderableEffect
{
    public:
        FireEffect(int id);
        virtual ~FireEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // FIREEFFECT_H
