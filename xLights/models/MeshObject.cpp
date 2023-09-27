/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/regex.h>

#include <algorithm>
#include <filesystem>

#include "MeshObject.h"
#include "UtilFunctions.h"
#include "ModelPreview.h"
#include "../ExternalHooks.h"
#include "xLightsMain.h"

#include <log4cpp/Category.hh>


#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "graphics/xlMesh.h"

MeshObject::MeshObject(wxXmlNode *node, const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager), _objFile(""),
    width(100), height(100), depth(100), brightness(100),
    obj_loaded(false), mesh_only(false),
    mesh(nullptr)
{
    SetFromXml(node);
    screenLocation.SetSupportsZScaling(true);
}

MeshObject::~MeshObject()
{
    if (mesh) {
        delete mesh;
    }
}

void MeshObject::InitModel() {
	_objFile = FixFile("", ModelXml->GetAttribute("ObjFile", ""));
    if (_objFile != ModelXml->GetAttribute("ObjFile", "")) {
        ModelXml->DeleteAttribute("ObjFile");
        ModelXml->AddAttribute("ObjFile", _objFile);
    }

    checkAccessToFile(_objFile);
    mesh_only = ModelXml->GetAttribute("MeshOnly", "0") == "1";

    if (ModelXml->HasAttribute("Brightness")) {
        brightness = wxAtoi(ModelXml->GetAttribute("Brightness"));
        if (brightness > 100) {
            brightness = 100;
        } else if (brightness < 0) {
            brightness = 0;
        }
    }

    screenLocation.SetRenderSize(width, height, depth);
}

void MeshObject::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
	wxPGProperty *p = grid->Append(new wxFileProperty("ObjFile",
                                             "ObjFile",
                                             _objFile));
    p->SetAttribute(wxPG_FILE_WILDCARD, "Wavefront files|*.obj|All files (*.*)|*.*");

    p = grid->Append(new wxUIntProperty("Brightness", "Brightness", brightness));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("Mesh Only", "MeshOnly", mesh_only));
    p->SetAttribute("UseCheckbox", true);
}

int MeshObject::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("ObjFile" == event.GetPropertyName()) {
        obj_loaded = false;
        _objFile = event.GetValue().GetString();
        auto mtfs = xlMesh::GetMaterialFilenamesFromOBJ(_objFile, false);
        bool hasSpaces = false;
        std::filesystem::path path(_objFile);
        for (auto &mtf : mtfs) {
            if (mtf.find(' ') != std::string::npos) {
                std::filesystem::path mtlpath(path);
                mtlpath.replace_filename(mtf);
                if (std::filesystem::exists(mtlpath)) {
                    // has spaces, but is found so we can fix it
                    hasSpaces = true;
                }
            }
        }
        if (hasSpaces) {
            if (wxMessageBox("The OBJ file contains materials with spaces in the filename.  This will prevent the materials from working.  Should we attempt to fix the file?",
                         "Files with spaces",
                             wxYES_NO | wxCENTRE | wxICON_WARNING) == wxYES) {
                
                xlMesh::FixMaterialFilenamesInOBJ(_objFile);
            }
        }
        checkAccessToFile(_objFile);
        ModelXml->DeleteAttribute("ObjFile");
        ModelXml->AddAttribute("ObjFile", _objFile);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MeshObject::OnPropertyGridChange::ObjFile");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MeshObject::OnPropertyGridChange::ObjFile");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "MeshObject::OnPropertyGridChange::ObjFile");
        return 0;
    } else if ("Brightness" == event.GetPropertyName()) {
        brightness = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Brightness");
        ModelXml->AddAttribute("Brightness", wxString::Format("%d", (int)brightness));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MeshObject::OnPropertyGridChange::Brightness");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MeshObject::OnPropertyGridChange::Brightness");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "MeshObject::OnPropertyGridChange::Brightness");
        return 0;
    } else if ("MeshOnly" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("MeshOnly");
        mesh_only = event.GetValue().GetBool();
        if (mesh_only) {
            ModelXml->AddAttribute("MeshOnly", "1");
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MeshObject::OnPropertyGridChange::MeshOnly");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MeshObject::OnPropertyGridChange::MeshOnly");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "MeshObject::OnPropertyGridChange::MeshOnly");
        return 0;
    }

    return ViewObject::OnPropertyGridChange(grid, event);
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

            ModelXml->DeleteAttribute("ObjFile");
            ModelXml->AddAttribute("ObjFile", _objFile);
            SetFromXml(ModelXml);
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
        res.push_back(wxString::Format("    ERR: Mesh object '%s' cant find obj file '%s'", GetName(), _objFile).ToStdString());
    } else {
        if (!IsFileInShowDir(xLightsFrame::CurrentDir, _objFile)) {
            res.push_back(wxString::Format("    WARN: Mesh object '%s' obj file '%s' not under show/media/resource directories.", GetName(), _objFile).ToStdString());
        }

        wxFileName fn(_objFile);
        checkAccessToFile(_objFile);
        auto mtfs = xlMesh::GetMaterialFilenamesFromOBJ(_objFile);

        if (mtfs.empty() || xlMesh::InvalidMaterialsList(mtfs)) {
            mtfs = xlMesh::GetMaterialFilenamesFromOBJ(_objFile, false);
            if (!mtfs.empty()) {
                res.push_back(wxString::Format("    WARN: Mesh object '%s' obj file '%s' has a space in the mesh file name : %s.", GetName(), _objFile, mtfs.front()).ToStdString());
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
                        res.push_back(wxString::Format("    WARN: Mesh object '%s' is missing material file '%s'.", GetName(), fn2.GetFullPath()).ToStdString());
                    }
                }
            }
        } else {
            res.push_back(wxString::Format("    WARN: Mesh object '%s' does not have a material file '%s'.", GetName(), fn.GetFullPath()).ToStdString());
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
                            res.push_back(wxString::Format("    ERR: Mesh object '%s' cant find texture file '%s'", GetName(), tex.GetFullPath()).ToStdString());
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
            }
        }
    }
    return res;
}

void MeshObject::loadObject(xlGraphicsContext *ctx)
{
    if (FileExists(_objFile)) {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        logger_base.debug("Loading mesh model '%s' file '%s'.",
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

        
        solid->addStep([=](xlGraphicsContext *ctx) {
            ctx->PushMatrix()
                ->Translate(GetObjectScreenLocation().GetHcenterPos(),
                                                   GetObjectScreenLocation().GetVcenterPos(),
                                                   GetObjectScreenLocation().GetDcenterPos())
                ->Rotate(rotations.z, 0, 0, 1)
                ->Rotate(rotations.y, 0, 1, 0)
                ->Rotate(rotations.x, 1, 0, 0)
                ->Scale(scalingMatrix.x, scalingMatrix.y, scalingMatrix.z);
            if (mesh_only) {
                ctx->drawMeshWireframe(mesh, brightness);
            } else {
                ctx->drawMeshSolids(mesh, brightness, true);
            }
            ctx->PopMatrix();
        });
        if (!mesh_only) {
            transparent->addStep([=](xlGraphicsContext *ctx) {
                ctx ->PushMatrix()
                    ->Translate(GetObjectScreenLocation().GetHcenterPos(),
                                                       GetObjectScreenLocation().GetVcenterPos(),
                                                       GetObjectScreenLocation().GetDcenterPos())
                    ->Rotate(rotations.z, 0, 0, 1)
                    ->Rotate(rotations.y, 0, 1, 0)
                    ->Rotate(rotations.x, 1, 0, 0)
                    ->Scale(scalingMatrix.x, scalingMatrix.y, scalingMatrix.z)
                    ->drawMeshTransparents(mesh, brightness)
                    ->PopMatrix();
            });
        }
    }
    
    if ((Selected || Highlighted) && allowSelected) {
        GetObjectScreenLocation().DrawHandles(solid, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), true, IsFromBase());
    }
    return false;
}
