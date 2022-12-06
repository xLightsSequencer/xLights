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

#include "DmxMovingHead.h"

class Mesh;
class wxXmlNode;

class DmxMovingHead3D : public DmxMovingHead
{
    public:
        DmxMovingHead3D(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxMovingHead3D();


        virtual void DrawModel(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *sprogram, xlGraphicsProgram *tprogram, bool is3d, bool active, const xlColor *c) override;

    protected:
        virtual void SetFromXml(wxXmlNode* ModelNode, bool zb) override;
        virtual void InitModel() override;

        virtual void ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        virtual void DisableUnusedProperties(wxPropertyGridInterface* grid) override;
        virtual float GetDefaultBeamWidth() const override { return 1.5f; }
        virtual std::list<std::string> CheckModelSettings() override;

    private:

        Mesh* base_mesh = nullptr;
        Mesh* head_mesh = nullptr;
        wxXmlNode* base_node = nullptr;
        wxXmlNode* head_node = nullptr;
        wxString obj_path = "";
};

