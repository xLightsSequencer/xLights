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

class LifeEffect : public RenderableEffect
{
public:
    LifeEffect(int id);
    virtual ~LifeEffect();
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool AppropriateOnNodes() const override { return false; }
protected:
    virtual wxPanel* CreatePanel(wxWindow* parent) override;
private:
};
