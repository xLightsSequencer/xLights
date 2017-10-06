#ifndef SNOWSTORMEFFECT_H
#define SNOWSTORMEFFECT_H

#include "RenderableEffect.h"


class SnowstormEffect : public RenderableEffect
{
    public:
        SnowstormEffect(int id);
        virtual ~SnowstormEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool AppropriateOnNodes() const override { return false; }
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // SNOWSTORMEFFECT_H
