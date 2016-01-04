#ifndef OFFEFFECT_H
#define OFFEFFECT_H

#include "RenderableEffect.h"

class OffEffect : public RenderableEffect
{
    public:
        OffEffect(int id);
        virtual ~OffEffect();
    
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // OFFEFFECT_H
