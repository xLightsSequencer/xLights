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

class IModelPreview;
class xlMesh;

class MeshObject : public ObjectWithScreenLocation<BoxedScreenLocation>
{
    std::vector<std::string> _warnedTextures;

public:
    MeshObject(const ViewObjectManager &manager);
    virtual ~MeshObject();

    virtual void InitModel() override;


    virtual bool Draw(IModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected = false) override;

    virtual std::list<std::string> GetFileReferences() override;
    virtual bool CleanupFileLocations(RenderContext* ctx) override;
    virtual std::list<std::string> CheckModelSettings() override;

    void SetObjectFile(const std::string & objFile);
    void SetObjFile(const std::string& file) { _objFile = file; }
    void SetMeshOnly(bool val) { mesh_only = val; }
    void SetBrightness(int val) { brightness = val; }
    void SetObjLoaded(bool val) { obj_loaded = val; }

    const std::string GetObjFile() const { return _objFile; }
    bool IsMeshOnly() const { return mesh_only; }
    int GetBrightness() const { return brightness; }

    void checkAccessToFile(const std::string &url);
    
    void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

protected:
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

