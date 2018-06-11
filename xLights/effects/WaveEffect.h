#ifndef WAVEEFFECT_H
#define WAVEEFFECT_H

#include "RenderableEffect.h"

#define WAVE_NUMBER_MIN 180
#define WAVE_NUMBER_MAX 3600

#define WAVE_THICKNESS_MIN 0
#define WAVE_THICKNESS_MAX 100

#define WAVE_HEIGHT_MIN 0
#define WAVE_HEIGHT_MAX 100

#define WAVE_SPEED_MIN 0
#define WAVE_SPEED_MAX 50

class WaveEffect : public RenderableEffect
{
    public:
        WaveEffect(int id);
        virtual ~WaveEffect();
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool AppropriateOnNodes() const override { return false; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // WAVEEFFECT_H
