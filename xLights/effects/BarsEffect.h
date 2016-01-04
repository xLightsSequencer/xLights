#ifndef BARSEFFECT_H
#define BARSEFFECT_H

#include "RenderableEffect.h"

class BarsEffect : public RenderableEffect
{
    public:
        BarsEffect(int id);
        virtual ~BarsEffect();
    
    
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // BARSEFFECT_H
