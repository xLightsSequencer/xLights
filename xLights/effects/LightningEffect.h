#ifndef LIGHTNINGEFFECT_H
#define LIGHTNINGEFFECT_H

#include "RenderableEffect.h"

class xlColor;

#define LIGHTNING_TOPX_MIN -50
#define LIGHTNING_TOPX_MAX 50

#define LIGHTNING_TOPY_MIN 0
#define LIGHTNING_TOPY_MAX 100

#define LIGHTNING_BOLTS_MIN 1
#define LIGHTNING_BOLTS_MAX 50

#define LIGHTNING_SEGMENTS_MIN 1
#define LIGHTNING_SEGMENTS_MAX 20

class LightningEffect : public RenderableEffect
{
    public:
        LightningEffect(int id);
        virtual ~LightningEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
        void LightningDrawBolt(RenderBuffer &buffer, const int x0_, const int y0_, const int x1_, const int y1_,  xlColor& color ,int curState);
};

#endif // LIGHTNINGEFFECT_H
