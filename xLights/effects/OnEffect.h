#ifndef ONEFFECT_H
#define ONEFFECT_H

#include "RenderableEffect.h"


class OnEffect : public RenderableEffect
{
    public:
        OnEffect(int id);
        virtual ~OnEffect();
    
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // ONEFFECT_H
