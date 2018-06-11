#ifndef SPIRALSEFFECT_H
#define SPIRALSEFFECT_H

#include "RenderableEffect.h"

class SpiralsEffect : public RenderableEffect {
    public:
        SpiralsEffect(int id);
        virtual ~SpiralsEffect();
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) override;
        virtual bool CanRenderPartialTimeInterval() const override { return true; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // SPIRALSEFFECT_H
