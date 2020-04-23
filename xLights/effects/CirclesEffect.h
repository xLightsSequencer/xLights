#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

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
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool AppropriateOnNodes() const override { return false; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
    
        void RenderMetaBalls(RenderBuffer &buffer, int numBalls, MetaBall *metaballs);
        void RenderRadial(RenderBuffer &buffer, int start_x,int start_y,int radius,
                          int colorCnt, int number, bool radial_3D,
                          const int effectState);
        void RenderCirclesUpdate(RenderBuffer &buffer, int number, RgbBalls* effObjs, int circleSpeed);

};
