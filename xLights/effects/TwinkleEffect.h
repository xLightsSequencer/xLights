#ifndef TWINKLEEFFECT_H
#define TWINKLEEFFECT_H

#include "RenderableEffect.h"


class TwinkleEffect : public RenderableEffect
{
    public:
        TwinkleEffect(int id);
        virtual ~TwinkleEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // TWINKLEEFFECT_H
