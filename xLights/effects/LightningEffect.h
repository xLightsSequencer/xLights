#ifndef LIGHTNINGEFFECT_H
#define LIGHTNINGEFFECT_H

#include "RenderableEffect.h"

class xlColor;

class LightningEffect : public RenderableEffect
{
    public:
        LightningEffect(int id);
        virtual ~LightningEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
        void LightningDrawBolt(RenderBuffer &buffer, const int x0_, const int y0_, const int x1_, const int y1_,  xlColor& color ,int curState);
};

#endif // LIGHTNINGEFFECT_H
