#ifndef OFFEFFECT_H
#define OFFEFFECT_H

#include "RenderableEffect.h"


class OffEffect : public RenderableEffect
{
    public:
        OffEffect(int id);
        virtual ~OffEffect();
    
        virtual wxPanel *CreatePanel(wxWindow *parent);

    protected:
    private:
};

#endif // OFFEFFECT_H
