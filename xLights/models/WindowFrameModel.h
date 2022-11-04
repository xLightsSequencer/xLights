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

#include "Model.h"

class WindowFrameModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        WindowFrameModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~WindowFrameModel();
        virtual int GetNumPhysicalStrings() const override { return 1; }
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return true; }
        virtual int NodesPerString() const override;

    protected:
        virtual void InitModel() override;

        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        void GetCoordinates(int side, bool clockwise, bool LtoR, bool TtoB, float& x, float& y, float& screenx, float& screeny);

    private:
        void InitFrame();
        int rotation;
};

