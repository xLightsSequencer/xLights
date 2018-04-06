#ifndef COLORWASHEFFECT_H
#define COLORWASHEFFECT_H

#include "RenderableEffect.h"


class ColorWashEffect : public RenderableEffect
{
    public:
        ColorWashEffect(int id);
        virtual ~ColorWashEffect();
    
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual int DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2, DrawGLUtils::xlAccumulator &bg, xlColor* colorMask, bool ramps) override;
        virtual std::string GetEffectString() override;
        virtual bool needToAdjustSettings(const std::string &version) override;
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
        virtual bool CanRenderPartialTimeInterval() const override { return true; }

    protected:
        virtual void RemoveDefaults(const std::string &version, Effect *effect) override;
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // COLORWASHEFFECT_H
