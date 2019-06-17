#ifndef SPIRALSEFFECT_H
#define SPIRALSEFFECT_H

#include "RenderableEffect.h"

#define SPIRALS_COUNT_MIN 1
#define SPIRALS_COUNT_MAX 5

#define SPIRALS_MOVEMENT_MIN -200
#define SPIRALS_MOVEMENT_MAX 200
#define SPIRALS_MOVEMENT_DIVISOR 10

#define SPIRALS_ROTATION_MIN -300
#define SPIRALS_ROTATION_MAX 300
#define SPIRALS_ROTATION_DIVISOR 10

#define SPIRALS_THICKNESS_MIN 0
#define SPIRALS_THICKNESS_MAX 100

class SpiralsEffect : public RenderableEffect {
    public:
        SpiralsEffect(int id);
        virtual ~SpiralsEffect();
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) const override;
        virtual bool CanRenderPartialTimeInterval() const override { return true; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // SPIRALSEFFECT_H
