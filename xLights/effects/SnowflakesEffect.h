#ifndef SNOWFLAKESEFFECT_H
#define SNOWFLAKESEFFECT_H

#include "RenderableEffect.h"


class SnowflakesEffect : public RenderableEffect
{
    public:
        SnowflakesEffect(int id);
        virtual ~SnowflakesEffect();

        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
        virtual bool needToAdjustSettings(const std::string &version);
        virtual void adjustSettings(const std::string &version, Effect *effect);
    private:
};

#endif // SNOWFLAKESEFFECT_H
