#ifndef CIRCLESEFFECT_H
#define CIRCLESEFFECT_H

#include "RenderableEffect.h"

class RgbBalls;
class MetaBall;

#define CIRCLES_COUNT_MIN 1
#define CIRCLES_COUNT_MAX 10

#define CIRCLES_SIZE_MIN 1
#define CIRCLES_SIZE_MAX 20

#define CIRCLES_SPEED_MIN 1
#define CIRCLES_SPEED_MAX 30

class CirclesEffect : public RenderableEffect
{
    public:
        CirclesEffect(int id);
        virtual ~CirclesEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
    
        void RenderMetaBalls(RenderBuffer &buffer, int numBalls, MetaBall *metaballs);
        void RenderRadial(RenderBuffer &buffer, int start_x,int start_y,int radius,
                          int colorCnt, int number, bool radial_3D,
                          const int effectState);
        void RenderCirclesUpdate(RenderBuffer &buffer, int number, RgbBalls* effObjs, int circleSpeed);

};

#endif // CIRCLESEFFECT_H
