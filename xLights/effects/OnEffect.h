#ifndef ONEFFECT_H
#define ONEFFECT_H

#include "RenderableEffect.h"


class OnEffect : public RenderableEffect
{
    public:
        OnEffect(int id);
        virtual ~OnEffect();
        virtual wxPanel *CreatePanel(wxWindow *parent);
    
    protected:
    private:
};

#endif // ONEFFECT_H
