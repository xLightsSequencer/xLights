#ifndef PLASMAEFFECT_H
#define PLASMAEFFECT_H

#include "RenderableEffect.h"

#define PLASMA_SPEED_MIN 0
#define PLASMA_SPEED_MAX 100

class PlasmaEffect : public RenderableEffect
{
    public:
        PlasmaEffect(int id);
        virtual ~PlasmaEffect();
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool CanRenderPartialTimeInterval() const override { return true; }
        virtual bool SupportsRenderCache() const override { return true; }
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // PLASMAEFFECT_H
