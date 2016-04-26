#ifndef FIREEFFECT_H
#define FIREEFFECT_H

#include "RenderableEffect.h"


class FireEffect : public RenderableEffect
{
    public:
        FireEffect(int id);
        virtual ~FireEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // FIREEFFECT_H
