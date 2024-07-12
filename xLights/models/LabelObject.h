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

#include "ViewObject.h"
#include "BoxedScreenLocation.h"
#include "../graphics/xlGraphicsAccumulators.h"
#include "../Color.h"

class ModelPreview;

class LabelObject : public ObjectWithScreenLocation<BoxedScreenLocation>
{
    public:
        LabelObject(wxXmlNode *node, const ViewObjectManager &manager);
        virtual ~LabelObject();

        virtual void InitModel() override;

        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) override {}

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        virtual bool Draw(ModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* solid, xlGraphicsProgram* transparent, bool allowSelected = false) override;

    protected:

    private:
        std::string _label;
        xlColor _color = xlGREEN;
        float _curFontSize = 12;
        xlTexture* _fontTexture = nullptr;
};
