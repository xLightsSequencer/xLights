#ifndef SPIROGRAPHEFFECT_H
#define SPIROGRAPHEFFECT_H

#include "RenderableEffect.h"

#define SPIROGRAPH_ANIMATE_MIN -50
#define SPIROGRAPH_ANIMATE_MAX 50

#define SPIROGRAPH_LENGTH_MIN 0
#define SPIROGRAPH_LENGTH_MAX 50

#define SPIROGRAPH_R_MIN 1
#define SPIROGRAPH_R_MAX 100

#define SPIROGRAPH_r_MIN 1
#define SPIROGRAPH_r_MAX 100

#define SPIROGRAPH_SPEED_MIN 0
#define SPIROGRAPH_SPEED_MAX 50

#define SPIROGRAPH_d_MIN 1
#define SPIROGRAPH_d_MAX 100

class SpirographEffect : public RenderableEffect
{
    public:
        SpirographEffect(int id);
        virtual ~SpirographEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // SPIROGRAPHEFFECT_H
