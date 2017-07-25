#ifndef SNOWFLAKESEFFECT_H
#define SNOWFLAKESEFFECT_H

#include "RenderableEffect.h"


class SnowflakesEffect : public RenderableEffect
{
    public:
        SnowflakesEffect(int id);
        virtual ~SnowflakesEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        virtual bool needToAdjustSettings(const std::string &version) override;
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
    private:
};

#endif // SNOWFLAKESEFFECT_H
