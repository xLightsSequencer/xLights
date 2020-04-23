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

#include "DmxFloodlight.h"
#include "DmxColorAbility.h"
#include "DmxShutterAbility.h"

class DmxFloodArea : public DmxFloodlight
{
    public:
        DmxFloodArea(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxFloodArea();

    protected:
        virtual void InitModel() override;

        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active) override;
        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, const xlColor *c, float &sx, float &sy, float &sz, bool active) override;

    private:
};

