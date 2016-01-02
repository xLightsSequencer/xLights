#ifndef BARSEFFECT_H
#define BARSEFFECT_H

#include "RenderableEffect.h"


class BarsEffect : public RenderableEffect
{
    public:
        BarsEffect(int id);
        virtual ~BarsEffect();
        virtual wxPanel *CreatePanel(wxWindow *parent);
    
    protected:
    private:
};

#endif // BARSEFFECT_H
