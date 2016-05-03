#ifndef SNOWSTORMEFFECT_H
#define SNOWSTORMEFFECT_H

#include "RenderableEffect.h"


class SnowstormEffect : public RenderableEffect
{
    public:
        SnowstormEffect(int id);
        virtual ~SnowstormEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // SNOWSTORMEFFECT_H
