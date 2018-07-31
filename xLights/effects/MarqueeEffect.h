#ifndef MARQUEEEFFECT_H
#define MARQUEEEFFECT_H

#include "RenderableEffect.h"

#define MARQUEE_XC_MIN -100
#define MARQUEE_XC_MAX 100
#define MARQUEE_YC_MIN -100
#define MARQUEE_YC_MAX 100
#define MARQUEE_SCALEX_MIN 1
#define MARQUEE_SCALEX_MAX 100
#define MARQUEE_SCALEY_MIN 1
#define MARQUEE_SCALEY_MAX 100
#define MARQUEE_BAND_SIZE_MIN 1
#define MARQUEE_BAND_SIZE_MAX 100
#define MARQUEE_SKIP_SIZE_MIN 0
#define MARQUEE_SKIP_SIZE_MAX 100
#define MARQUEE_SPEED_MIN 0
#define MARQUEE_SPEED_MAX 50
#define MARQUEE_STAGGER_MIN 0
#define MARQUEE_STAGGER_MAX 50
#define MARQUEE_START_MIN 0
#define MARQUEE_START_MAX 100
#define MARQUEE_THICKNESS_MIN 1
#define MARQUEE_THICKNESS_MAX 100

class MarqueeEffect : public RenderableEffect
{
    public:
        MarqueeEffect(int id);
        virtual ~MarqueeEffect();    
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool CanRenderPartialTimeInterval() const override { return true; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
};

#endif // MARQUEEEFFECT_H
