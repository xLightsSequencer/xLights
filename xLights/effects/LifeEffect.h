#ifndef LIFEEFFECT_H
#define LIFEEFFECT_H

#include "RenderableEffect.h"


class LifeEffect : public RenderableEffect
{
    public:
        LifeEffect(int id);
        virtual ~LifeEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // LIFEEFFECT_H
