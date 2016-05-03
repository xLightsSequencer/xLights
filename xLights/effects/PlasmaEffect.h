#ifndef PLASMAEFFECT_H
#define PLASMAEFFECT_H

#include "RenderableEffect.h"


class PlasmaEffect : public RenderableEffect
{
    public:
        PlasmaEffect(int id);
        virtual ~PlasmaEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // PLASMAEFFECT_H
