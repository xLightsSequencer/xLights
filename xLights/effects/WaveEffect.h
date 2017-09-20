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

#define WAVE_Y_POSITION_MIN -250
#define WAVE_Y_POSITION_MAX 250

class WaveEffect : public RenderableEffect
{
    public:
        WaveEffect(int id);
        virtual ~WaveEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // WAVEEFFECT_H
