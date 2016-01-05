#ifndef STROBEEFFECT_H
#define STROBEEFFECT_H

#include "RenderableEffect.h"


class StrobeEffect : public RenderableEffect
{
    public:
        StrobeEffect(int id);
        virtual ~StrobeEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // STROBEEFFECT_H
