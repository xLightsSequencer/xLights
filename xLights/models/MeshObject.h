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

#include <vector>
#include "ViewObject.h"
#include "BoxedScreenLocation.h"

class ModelPreview;
class xlMesh;

class MeshObject : public ObjectWithScreenLocation<BoxedScreenLocation>
{
    std::vector<std::string> _warnedTextures;

    public:
        MeshObject(wxXmlNode *node, const ViewObjectManager &manager);
        virtual ~MeshObject();

        virtual void InitModel() override;

        virtual void AddTypeProperties(wxPropertyGridInterface* grid) override;
        virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) override {}

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        virtual bool Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected = false) override;
    
        virtual std::list<std::string> GetFileReferences() override;
        virtual bool CleanupFileLocations(xLightsFrame* frame) override;
        virtual std::list<std::string> CheckModelSettings() override;

    protected:
        void checkAccessToFile(const std::string &url);
        void loadObject(xlGraphicsContext *ctx);
    private:
        std::string _objFile;
        float width;
        float height;
        float depth;
        float brightness;
        bool obj_loaded;
        bool mesh_only;

        xlMesh *mesh;
};

