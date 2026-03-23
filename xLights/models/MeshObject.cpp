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
#include "xLightsMain.h"

#include "spdlog/spdlog.h"

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


bool MeshObject::CleanupFileLocations(xLightsFrame* frame)
{
    bool rc = false;
    if (FileExists(_objFile)) {
        if (!frame->IsInShowFolder(_objFile)) {
            auto fr = GetFileReferences();
            for (auto f: fr) {
                if (f != _objFile) {
                    frame->MoveToShowFolder(f, wxString(wxFileName::GetPathSeparator()) + "3D");
                }
            }

            _objFile = frame->MoveToShowFolder(_objFile, wxString(wxFileName::GetPathSeparator()) + "3D");
            rc = true;
        }
    }
    return BaseObject::CleanupFileLocations(frame) || rc;
}

void MeshObject::checkAccessToFile(const std::string &url) {
    if (FileExists(url) && !ObtainAccessToURL(url)) {
        wxMessageBox("Could not obtain access to " + url + "\n\nTry giving xLights permission to access to the directory.",
                     "Access Denied");
        wxFileName fn(url);
        wxDirDialog dlg(nullptr, "Select Directory For Mesh Resources", fn.GetPath());
        if (dlg.ShowModal()) {
            ObtainAccessToURL(url);
        }
    }
}

std::list<std::string> MeshObject::CheckModelSettings()
{
    std::list<std::string> res;

    if (_objFile == "" || !FileExists(_objFile)) {
        res.push_back(std::format("    ERR: Mesh object '{}' cant find obj file '{}'", GetName(), _objFile));
    } else {
        if (!IsFileInShowDir(xLightsFrame::CurrentDir, _objFile)) {
            res.push_back(std::format("    WARN: Mesh object '{}' obj file '{}' not under show/media/resource directories.", GetName(), _objFile));
        }

        wxFileName fn(_objFile);
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
                mtf = fn.GetPath() + wxFileName::GetPathSeparator() + mtf;
                wxFileName fn2 = wxFileName(mtf);
                if (!FileExists(fn2)) {
                    // we should also check in a folder with the same name as the mesh file
                    auto mtf2 = fn.GetPath() + wxFileName::GetPathSeparator() + fn.GetName() + wxFileName::GetPathSeparator() + mtf;
                    wxFileName fn3 = wxFileName(mtf2);
                    if (!FileExists(fn3)) {
                        // still not there so report the warning
                        res.push_back(std::format("    WARN: Mesh object '{}' is missing material file '{}'.", GetName(), fn2.GetFullPath().ToStdString()));
                    }
                }
            }
        } else {
            res.push_back(std::format("    WARN: Mesh object '{}' does not have a material file '{}'.", GetName(), fn.GetFullPath().ToStdString()));
        }

        std::string base_path = fn.GetPath();
        tinyobj::attrib_t attr;
        std::vector<int> lin;
        std::vector<tinyobj::shape_t> shap;
        std::vector<tinyobj::material_t> mater;
        std::string err, warn;
        tinyobj::LoadObj(&attr, &shap, &mater, &warn, &err, (char *)_objFile.c_str(), (char *)base_path.c_str());

        for (auto m : mater) {
            if (m.diffuse_texname.length() > 0) {
                wxFileName tex(m.diffuse_texname);
                tex.SetPath(fn.GetPath());
                if (!FileExists(tex)) {
                    wxFileName tex2(fn.GetPath() + wxFileName::GetPathSeparator() + m.diffuse_texname);
                    if (!FileExists(tex2)) {
                        wxFileName tex3(fn.GetPath() + wxFileName::GetPathSeparator() + fn.GetName() + wxFileName::GetPathSeparator() + m.diffuse_texname);
                        if (!FileExists(tex3)) {
                            res.push_back(std::format("    ERR: Mesh object '{}' cant find texture file '{}'", GetName(), tex.GetFullPath().ToStdString()));
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

        wxFileName path(_objFile);
        auto mtfs = xlMesh::GetMaterialFilenamesFromOBJ(_objFile);

        if (mtfs.empty() || xlMesh::InvalidMaterialsList(mtfs)) {
            mtfs = xlMesh::GetMaterialFilenamesFromOBJ(_objFile, false);
        }

        for (auto &mtf : mtfs) {
            mtf = path.GetPath() + wxFileName::GetPathSeparator() + mtf;
            wxFileName mtl = wxFileName(mtf);
            if (FileExists(mtl)) {
                res.push_back(mtl.GetFullPath());
                checkAccessToFile(mtl.GetFullPath());
                
                auto txts = xlMesh::GetTextureFilenamesFromMTL(mtl.GetFullPath());
                for (auto t : txts) {
                    t = path.GetPath() + wxFileName::GetPathSeparator() + t;
                    wxFileName tfn = wxFileName(t);
                    if (FileExists(tfn)) {
                        res.push_back(tfn.GetFullPath());
                        checkAccessToFile(tfn.GetFullPath());
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

        wxFileName fn(_objFile);

        wxFileName mtl(_objFile);
        mtl.SetExt("mtl");
        if (FileExists(mtl)) {
            checkAccessToFile(mtl.GetFullPath());
        }
        mesh = ctx->loadMeshFromObjFile(fn.GetFullPath());
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
