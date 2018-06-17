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
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) override { return false; }
        virtual void SetDefaultParameters() override;

    protected:
        virtual void RemoveDefaults(const std::string &version, Effect *effect) override;
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
};

#endif // WARPEFFECT_H
