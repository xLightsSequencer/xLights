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


//somewhat deprecated as the ModelGroup is more likely to be used.  However, older rgbeffects.xml may have these in them
class WholeHouseModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        WholeHouseModel(wxXmlNode *node, const ModelManager &manager, bool zb = false);
        virtual ~WholeHouseModel();
        virtual bool SupportsExportAsCustom() const override { return false; } 
        virtual bool SupportsWiringView() const override { return false; }

    protected:
        WholeHouseModel(const ModelManager &manager);
        virtual void InitModel() override;
        void InitWholeHouse(const std::string &WholeHouseData);
    
        NodeBaseClass* createNode(int ns, const std::string &StringType, size_t NodesPerString, const std::string &rgbOrder) const;

    private:
};
