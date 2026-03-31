/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <algorithm>
#include <format>
#include <filesystem>

#include "MeshObject.h"
#include "UtilFunctions.h"
#include "ModelPreview.h"
#include "../ExternalHooks.h"
#include "../render/RenderContext.h"

#include "../render/UICallbacks.h"

#include <log.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "graphics/xlMesh.h"

MeshObject::MeshObject(const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::Mesh;
    screenLocation.SetSupportsZScaling(true);
}

MeshObject::~MeshObject()
{
}

void MeshObject::InitModel() {
    if (brightness > 100) {
        brightness = 100;
    } else if (brightness < 0) {
        brightness = 0;
    }
    screenLocation.SetRenderSize(width, height, depth);
}

void MeshObject::SetObjectFile(const std::string & objFile)
{
    _objFile = FixFile("", objFile);
    checkAccessToFile(_objFile);
}


bool MeshObject::CleanupFileLocations(RenderContext* ctx)
{
    bool rc = false;
    if (FileExists(_objFile)) {
        if (!ctx->IsInShowFolder(_objFile)) {
            auto fr = GetFileReferences();
            for (auto f: fr) {
                if (f != _objFile) {
                    ctx->MoveToShowFolder(f, std::string(1, std::filesystem::path::preferred_separator) + "3D");
                }
            }

            _objFile = ctx->MoveToShowFolder(_objFile, std::string(1, std::filesystem::path::preferred_separator) + "3D");
            rc = true;
        }
    }
    return BaseObject::CleanupFileLocations(ctx) || rc;
}

void MeshObject::checkAccessToFile(const std::string &url) {
    if (FileExists(url) && !ObtainAccessToURL(url)) {
        if (auto* ui = GetObjectManager().GetUICallbacks()) {
            ui->ShowMessage("Could not obtain access to " + url + "\n\nTry giving xLights permission to access to the directory.",
                           "Access Denied");
            std::string dirPath = std::filesystem::path(url).parent_path().string();
            std::string chosen = ui->PromptForDirectory("Select Directory For Mesh Resources", dirPath);
            if (!chosen.empty()) {
                ObtainAccessToURL(url);
            }
        }
    }
}

std::list<std::string> MeshObject::CheckModelSettings()
{
    std::list<std::string> res;

    if (_objFile == "" || !FileExists(_objFile)) {
        res.push_back(std::format("    ERR: Mesh object '{}' cant find obj file '{}'", GetName(), _objFile));
    } else {
        if (!IsFileInShowDir(std::string(), _objFile)) {
            res.push_back(std::format("    WARN: Mesh object '{}' obj file '{}' not under show/media/resource directories.", GetName(), _objFile));
        }

        std::filesystem::path objPath(_objFile);
        std::string objDir = objPath.parent_path().string();
        std::string objStem = objPath.stem().string();
        checkAccessToFile(_objFile);
        auto mtfs = xlMesh::GetMaterialFilenamesFromOBJ(_objFile);

        if (mtfs.empty() || xlMesh::InvalidMaterialsList(mtfs)) {
            mtfs = xlMesh::GetMaterialFilenamesFromOBJ(_objFile, false);
            if (!mtfs.empty()) {
                res.push_back(std::format("    WARN: Mesh object '{}' obj file '{}' has a space in the mesh file name : {}.", GetName(), _objFile, mtfs.front()));
            }
        }

        if (!mtfs.empty()) {
            for (auto & mtf : mtfs) {
                mtf = (std::filesystem::path(objDir) / mtf).string();
                if (!FileExists(mtf)) {
                    // we should also check in a folder with the same name as the mesh file
                    auto mtf2 = (std::filesystem::path(objDir) / objStem / mtf).string();
                    if (!FileExists(mtf2)) {
                        // still not there so report the warning
                        res.push_back(std::format("    WARN: Mesh object '{}' is missing material file '{}'.", GetName(), mtf));
                    }
                }
            }
        } else {
            res.push_back(std::format("    WARN: Mesh object '{}' does not have a material file '{}'.", GetName(), _objFile));
        }

        std::string base_path = objDir;
        tinyobj::attrib_t attr;
        std::vector<int> lin;
        std::vector<tinyobj::shape_t> shap;
        std::vector<tinyobj::material_t> mater;
        std::string err, warn;
        tinyobj::LoadObj(&attr, &shap, &mater, &warn, &err, (char *)_objFile.c_str(), (char *)base_path.c_str());

        for (auto m : mater) {
            if (m.diffuse_texname.length() > 0) {
                std::string texPath = (std::filesystem::path(objDir) / m.diffuse_texname).string();
                if (!FileExists(texPath)) {
                    std::string texPath2 = (std::filesystem::path(objDir) / m.diffuse_texname).string();
                    if (!FileExists(texPath2)) {
                        std::string texPath3 = (std::filesystem::path(objDir) / objStem / m.diffuse_texname).string();
                        if (!FileExists(texPath3)) {
                            res.push_back(std::format("    ERR: Mesh object '{}' cant find texture file '{}'", GetName(), texPath));
                        }
                    }
                }
            }
        }
    }

    res.splice(res.end(), BaseObject::CheckModelSettings());
    return res;
}

std::list<std::string> MeshObject::GetFileReferences()
{
    std::list<std::string> res;
    if (FileExists(_objFile)) {
        res.push_back(_objFile);

        std::string objDir = std::filesystem::path(_objFile).parent_path().string();
        auto mtfs = xlMesh::GetMaterialFilenamesFromOBJ(_objFile);

        if (mtfs.empty() || xlMesh::InvalidMaterialsList(mtfs)) {
            mtfs = xlMesh::GetMaterialFilenamesFromOBJ(_objFile, false);
        }

        for (auto &mtf : mtfs) {
            mtf = (std::filesystem::path(objDir) / mtf).string();
            if (FileExists(mtf)) {
                res.push_back(mtf);
                checkAccessToFile(mtf);

                auto txts = xlMesh::GetTextureFilenamesFromMTL(mtf);
                for (auto t : txts) {
                    t = (std::filesystem::path(objDir) / t).string();
                    if (FileExists(t)) {
                        res.push_back(t);
                        checkAccessToFile(t);
                    }
                }
            }
        }
    }
    return res;
}

void MeshObject::loadObject(xlGraphicsContext *ctx)
{
    if (FileExists(_objFile)) {
        

        spdlog::debug("Loading mesh model '{}' file '{}'.",
            (const char*)GetName().c_str(),
            (const char*)_objFile.c_str());

        std::string mtlPath = std::filesystem::path(_objFile).replace_extension(".mtl").string();
        if (FileExists(mtlPath)) {
            checkAccessToFile(mtlPath);
        }
        mesh = ctx->loadMeshFromObjFile(_objFile);
        if (mesh) {
            obj_loaded = true;
            width = std::max(std::abs(mesh->GetXMax()), std::abs(mesh->GetXMin())) * 2;
            height = std::max(std::abs(mesh->GetYMax()), std::abs(mesh->GetYMin())) * 2;
            depth = std::max(std::abs(mesh->GetZMax()), std::abs(mesh->GetZMin())) * 2;
            
            screenLocation.SetRenderSize(width, height, depth);
            mesh->SetName(GetName());
        }
    }
}

bool MeshObject::Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected) {
    if (!IsActive()) { return true; }

    GetObjectScreenLocation().PrepareToDraw(true, allowSelected);

    if (!obj_loaded) {
        loadObject(ctx);
    }

    GetObjectScreenLocation().UpdateBoundingBox(width, height, depth);  // FIXME: Modify to only call this when position changes
    if (mesh) {
        glm::vec3 scalingMatrix = GetObjectScreenLocation().GetScaleMatrix();
        glm::vec3 rotations = GetObjectScreenLocation().GetRotation();

        
        solid->addStep([=, this](xlGraphicsContext *ctx) {
            ctx->PushMatrix()
                ->Translate(GetObjectScreenLocation().GetHcenterPos(),
                                                   GetObjectScreenLocation().GetVcenterPos(),
                                                   GetObjectScreenLocation().GetDcenterPos())
                ->Rotate(rotations.z, 0, 0, 1)
                ->Rotate(rotations.y, 0, 1, 0)
                ->Rotate(rotations.x, 1, 0, 0)
                ->Scale(scalingMatrix.x, scalingMatrix.y, scalingMatrix.z);
            if (mesh_only) {
                ctx->drawMeshWireframe(mesh.get(), brightness);
            } else {
                ctx->drawMeshSolids(mesh.get(), brightness, true);
            }
            ctx->PopMatrix();
        });
        if (!mesh_only) {
            transparent->addStep([=, this](xlGraphicsContext *ctx) {
                ctx ->PushMatrix()
                    ->Translate(GetObjectScreenLocation().GetHcenterPos(),
                                                       GetObjectScreenLocation().GetVcenterPos(),
                                                       GetObjectScreenLocation().GetDcenterPos())
                    ->Rotate(rotations.z, 0, 0, 1)
                    ->Rotate(rotations.y, 0, 1, 0)
                    ->Rotate(rotations.x, 1, 0, 0)
                    ->Scale(scalingMatrix.x, scalingMatrix.y, scalingMatrix.z)
                    ->drawMeshTransparents(mesh.get(), brightness)
                    ->PopMatrix();
            });
        }
    }
    
    if ((Selected() || Highlighted()) && allowSelected) {
        GetObjectScreenLocation().DrawHandles(solid, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), true, IsFromBase());
    }
    return false;
}
