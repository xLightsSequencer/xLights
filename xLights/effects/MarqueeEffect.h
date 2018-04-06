#ifndef MARQUEEEFFECT_H
#define MARQUEEEFFECT_H

#include "RenderableEffect.h"

class MarqueeEffect : public RenderableEffect
{
    public:
        MarqueeEffect(int id);
        virtual ~MarqueeEffect();    
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool CanRenderPartialTimeInterval() const override { return true; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
};

#endif // MARQUEEEFFECT_H
