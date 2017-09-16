#ifndef TWINKLEEFFECT_H
#define TWINKLEEFFECT_H

#include "RenderableEffect.h"


class TwinkleEffect : public RenderableEffect
{
    public:
        TwinkleEffect(int id);
        virtual ~TwinkleEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual int DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2, DrawGLUtils::xlAccumulator &backgrounds, xlColor* colorMask, bool ramps) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // TWINKLEEFFECT_H
