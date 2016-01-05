#ifndef SPIROGRAPHEFFECT_H
#define SPIROGRAPHEFFECT_H

#include "RenderableEffect.h"


class SpirographEffect : public RenderableEffect
{
    public:
        SpirographEffect(int id);
        virtual ~SpirographEffect();
    
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // SPIROGRAPHEFFECT_H
