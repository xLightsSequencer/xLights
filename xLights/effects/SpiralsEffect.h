#ifndef SPIRALSEFFECT_H
#define SPIRALSEFFECT_H

#include "RenderableEffect.h"


class SpiralsEffect : public RenderableEffect {
    public:
        SpiralsEffect(int id);
        virtual ~SpiralsEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // SPIRALSEFFECT_H
