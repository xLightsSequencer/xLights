#ifndef FANEFFECT_H
#define FANEFFECT_H

#include "RenderableEffect.h"

class FanEffect : public RenderableEffect
{
    public:
        FanEffect(int id);
        virtual ~FanEffect();

        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        virtual void SetDefaultParameters(Model *cls) override;
        virtual int DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                         DrawGLUtils::xlVertexColorAccumulator &backgrounds, xlColor* colorMask) override;
        virtual bool SupportsRadialColorCurves(const SettingsMap &SettingsMap) override { return true; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
};

#endif // FANEFFECT_H
