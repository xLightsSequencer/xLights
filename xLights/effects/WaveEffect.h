#ifndef WAVEEFFECT_H
#define WAVEEFFECT_H

#include "RenderableEffect.h"


class WaveEffect : public RenderableEffect
{
    public:
        WaveEffect(int id);
        virtual ~WaveEffect();
    
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // WAVEEFFECT_H
