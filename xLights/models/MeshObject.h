#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
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
    MeshObject(const ViewObjectManager &manager);
    virtual ~MeshObject();

    virtual void InitModel() override;

    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) override {}

    int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    virtual bool Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected = false) override;

    virtual std::list<std::string> GetFileReferences() override;
    virtual bool CleanupFileLocations(xLightsFrame* frame) override;
    virtual std::list<std::string> CheckModelSettings() override;

    void SetObjectFile(const std::string & objFile);
    void SetMeshOnly(bool val) { mesh_only = val; }
    void SetBrightness(int val) {brightness = val; }

protected:
    void checkAccessToFile(const std::string &url);
    void loadObject(xlGraphicsContext *ctx);

private:
    std::string _objFile {""};
    float width {100};
    float height {100};
    float depth {100};
    int brightness {100};
    bool obj_loaded {false};
    bool mesh_only {false};

    std::unique_ptr<xlMesh> mesh {nullptr};
};

