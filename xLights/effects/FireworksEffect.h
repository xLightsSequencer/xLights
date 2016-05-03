#ifndef FIREWORKSEFFECT_H
#define FIREWORKSEFFECT_H

#include "RenderableEffect.h"


class FireworksEffect : public RenderableEffect
{
    public:
        FireworksEffect(int id);
        virtual ~FireworksEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;

    private:
};

#endif // FIREWORKSEFFECT_H
