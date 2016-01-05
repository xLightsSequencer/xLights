#ifndef BUTTERFLYEFFECT_H
#define BUTTERFLYEFFECT_H

#include "RenderableEffect.h"


class ButterflyEffect : public RenderableEffect
{
    public:
        ButterflyEffect(int id);
        virtual ~ButterflyEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // BUTTERFLYEFFECT_H
