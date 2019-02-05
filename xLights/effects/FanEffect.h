#ifndef FANEFFECT_H
#define FANEFFECT_H

#include "RenderableEffect.h"

#define FAN_CENTREX_MIN 0
#define FAN_CENTREX_MAX 100

#define FAN_CENTREY_MIN 0
#define FAN_CENTREY_MAX 100

#define FAN_STARTRADIUS_MIN 0
#define FAN_STARTRADIUS_MAX 500

#define FAN_STARTANGLE_MIN 0
#define FAN_STARTANGLE_MAX 360

#define FAN_ENDRADIUS_MIN 0
#define FAN_ENDRADIUS_MAX 500

#define FAN_REVOLUTIONS_MIN 0
#define FAN_REVOLUTIONS_MAX 3600

#define FAN_BLADES_MIN 1
#define FAN_BLADES_MAX 16

#define FAN_BLADEWIDTH_MIN 5
#define FAN_BLADEWIDTH_MAX 100

#define FAN_BLADEANGLE_MIN -360
#define FAN_BLADEANGLE_MAX 360

#define FAN_NUMELEMENTS_MIN 1
#define FAN_NUMELEMENTS_MAX 4

#define FAN_ELEMENTWIDTH_MIN 5
#define FAN_ELEMENTWIDTH_MAX 100

#define FAN_DURATION_MIN 0
#define FAN_DURATION_MAX 100

#define FAN_ACCEL_MIN -10
#define FAN_ACCEL_MAX 10

class FanEffect : public RenderableEffect
{
    public:
        FanEffect(int id);
        virtual ~FanEffect();

        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual void SetDefaultParameters() override;
        virtual int DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                         DrawGLUtils::xlAccumulator &backgrounds, xlColor* colorMask, bool ramps) override;
        virtual bool SupportsRadialColorCurves(const SettingsMap &SettingsMap) override { return true; }
        virtual bool CanRenderPartialTimeInterval() const override { return true; }
        virtual bool SupportsRenderCache(const SettingsMap& settings) const override { return true; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
};

#endif // FANEFFECT_H
