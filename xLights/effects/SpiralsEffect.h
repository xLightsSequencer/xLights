#ifndef SPIRALSEFFECT_H
#define SPIRALSEFFECT_H

#include "RenderableEffect.h"


class SpiralsEffect : public RenderableEffect {
    public:
        SpiralsEffect(int id);
        virtual ~SpiralsEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // SPIRALSEFFECT_H
