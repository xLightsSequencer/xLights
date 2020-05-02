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

#define SPIROGRAPH_ANIMATE_MIN -50
#define SPIROGRAPH_ANIMATE_MAX 50

#define SPIROGRAPH_LENGTH_MIN 0
#define SPIROGRAPH_LENGTH_MAX 50

#define SPIROGRAPH_WIDTH_MIN 1
#define SPIROGRAPH_WIDTH_MAX 50

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
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool AppropriateOnNodes() const override { return false; }
        virtual bool CanRenderPartialTimeInterval() const override { return true; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
};
