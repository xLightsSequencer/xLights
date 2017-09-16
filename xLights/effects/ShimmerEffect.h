#ifndef SHIMMEREFFECT_H
#define SHIMMEREFFECT_H

#include "RenderableEffect.h"

#define SHIMMER_CYCLES_MIN 0
#define SHIMMER_CYCLES_MAX 6000

#define SHIMMER_DUTYFACTOR_MIN 1
#define SHIMMER_DUTYFACTOR_MAX 100

class ShimmerEffect : public RenderableEffect
{
    public:
        ShimmerEffect(int id);
        virtual ~ShimmerEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) override { return true; }
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        virtual bool needToAdjustSettings(const std::string& version) override;
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
private:
};

#endif // SHIMMEREFFECT_H
