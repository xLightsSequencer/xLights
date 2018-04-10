#ifndef WARPEFFECT_H
#define WARPEFFECT_H

#include "RenderableEffect.h"

class WarpEffect : public RenderableEffect
{
    public:
        WarpEffect(int id);
        virtual ~WarpEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual int DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2, DrawGLUtils::xlAccumulator &backgrounds, xlColor* colorMask, bool ramps) override;
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) override { return false; }
        virtual void SetDefaultParameters(Model *cls) override;
        virtual std::string GetEffectString() override;

    protected:
        virtual void RemoveDefaults(const std::string &version, Effect *effect) override;
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // WARPEFFECT_H
