#ifndef OFFEFFECT_H
#define OFFEFFECT_H

#include "RenderableEffect.h"

class OffEffect : public RenderableEffect
{
    public:
        OffEffect(int id);
        virtual ~OffEffect();
        virtual bool CanBeRandom() {return false;}
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // OFFEFFECT_H
