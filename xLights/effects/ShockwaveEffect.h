#ifndef SHOCKWAVEEFFECT_H
#define SHOCKWAVEEFFECT_H

#include "RenderableEffect.h"

class ShockwaveEffect : public RenderableEffect
{
    public:
        ShockwaveEffect(int id);
        virtual ~ShockwaveEffect();

        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        virtual int DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                         DrawGLUtils::xlVertexColorAccumulator &backgrounds, xlColor* colorMask, bool ramps) override;
        virtual void SetDefaultParameters(Model *cls) override;
        virtual bool SupportsRadialColorCurves(const SettingsMap &SettingsMap) override { return true; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
};

#endif // SHOCKWAVEEFFECT_H
