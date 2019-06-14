#ifndef ONEFFECT_H
#define ONEFFECT_H

#include "RenderableEffect.h"

#define ON_TRANSPARENCY_MIN 0
#define ON_TRANSPARENCY_MAX 100

class OnEffect : public RenderableEffect
{
    public:
        OnEffect(int id);
        virtual ~OnEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual int DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2, DrawGLUtils::xlAccumulator &backgrounds, xlColor* colorMask, bool ramps) override;
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) const override { return true; }
        virtual void SetDefaultParameters() override;
        virtual std::string GetEffectString() override;
        virtual bool CanRenderPartialTimeInterval() const override { return true; }

    protected:
        virtual void RemoveDefaults(const std::string &version, Effect *effect) override;
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
};

#endif // ONEFFECT_H
