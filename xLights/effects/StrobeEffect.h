#ifndef STROBEEFFECT_H
#define STROBEEFFECT_H

#include "RenderableEffect.h"


class StrobeEffect : public RenderableEffect
{
    public:
        StrobeEffect(int id);
        virtual ~StrobeEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // STROBEEFFECT_H
