#ifndef FILLEFFECT_H
#define FILLEFFECT_H

#include "RenderableEffect.h"

class FillEffect : public RenderableEffect
{
    public:
        FillEffect(int id);
        virtual ~FillEffect();

        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // FILLEFFECT_H
