#ifndef LINESEFFECT_H
#define LINESEFFECT_H

#include "RenderableEffect.h"
#include "../RenderBuffer.h"

#define LINES_SPEED_MIN 1
#define LINES_SPEED_MAX 10

#define LINES_THICKNESS_MIN 1
#define LINES_THICKNESS_MAX 10

class LinesEffect : public RenderableEffect
{
    public:
        LinesEffect(int id);
        virtual ~LinesEffect();
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        void Render(RenderBuffer &buffer, 
            int objects, int segments, int thickness, int speed, int trails, bool fadeTrails);
		virtual void SetDefaultParameters() override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
};

#endif // LINESEFFECT_H