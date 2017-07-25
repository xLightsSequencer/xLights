#ifndef SHIMMEREFFECT_H
#define SHIMMEREFFECT_H

#include "RenderableEffect.h"


class ShimmerEffect : public RenderableEffect
{
    public:
        ShimmerEffect(int id);
        virtual ~ShimmerEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) override { return true; }
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        virtual bool needToAdjustSettings(const std::string& version) override;
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
private:
};

#endif // SHIMMEREFFECT_H
