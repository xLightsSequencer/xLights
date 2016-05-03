#ifndef CIRCLESEFFECT_H
#define CIRCLESEFFECT_H

#include "RenderableEffect.h"

class RgbBalls;
class MetaBall;

class CirclesEffect : public RenderableEffect
{
    public:
        CirclesEffect(int id);
        virtual ~CirclesEffect();
    
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
