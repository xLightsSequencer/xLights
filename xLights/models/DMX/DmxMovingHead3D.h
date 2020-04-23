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

    protected:
        virtual void InitModel() override;

        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active) override;
        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, const xlColor *c, float &sx, float &sy, float &sz, bool active) override;

        virtual void DisableUnusedProperties(wxPropertyGridInterface* grid) override;

    private:
        void DrawModel(ModelPreview* preview, DrawGLUtils::xlAccumulator& va2, DrawGLUtils::xl3Accumulator& va3, const xlColor* c, float& sx, float& sy, float& sz, bool active, bool is_3d);

        Mesh* base_mesh = nullptr;
        Mesh* head_mesh = nullptr;
        wxXmlNode* base_node = nullptr;
        wxXmlNode* head_node = nullptr;
        wxString obj_path = "";
};

