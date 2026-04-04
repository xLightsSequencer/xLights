/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/msgdlg.h>
#include <filesystem>

#include "MeshObjectPropertyAdapter.h"
#include "../../../models/MeshObject.h"
#include "../../../graphics/xlMesh.h"

MeshObjectPropertyAdapter::MeshObjectPropertyAdapter(MeshObject& obj)
    : ViewObjectPropertyAdapter(obj), _mesh(obj) {}

void MeshObjectPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty *p = grid->Append(new wxFileProperty("ObjFile",
                                             "ObjFile",
                                             _mesh.GetObjFile()));
    p->SetAttribute(wxPG_FILE_WILDCARD, "Wavefront files|*.obj|All files (*.*)|*.*");

    p = grid->Append(new wxUIntProperty("Brightness", "Brightness", _mesh.GetBrightness()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("Mesh Only", "MeshOnly", _mesh.IsMeshOnly()));
    p->SetAttribute("UseCheckbox", true);
}

int MeshObjectPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("ObjFile" == event.GetPropertyName()) {
        _mesh.SetObjLoaded(false);
        std::string objFile = event.GetValue().GetString().ToStdString();
        _mesh.SetObjFile(objFile);
        auto mtfs = xlMesh::GetMaterialFilenamesFromOBJ(objFile, false);
        bool hasSpaces = false;
        std::filesystem::path path(objFile);
        for (auto &mtf : mtfs) {
            if (mtf.find(' ') != std::string::npos) {
                std::filesystem::path mtlpath(path);
                mtlpath.replace_filename(mtf);
                if (std::filesystem::exists(mtlpath)) {
                    hasSpaces = true;
                }
            }
        }
        if (hasSpaces) {
            if (wxMessageBox("The OBJ file contains materials with spaces in the filename.  This will prevent the materials from working.  Should we attempt to fix the file?",
                         "Files with spaces",
                             wxYES_NO | wxCENTRE | wxICON_WARNING) == wxYES) {
                xlMesh::FixMaterialFilenamesInOBJ(objFile);
            }
        }
        _mesh.checkAccessToFile(objFile);
        _mesh.IncrementChangeCount();
        _mesh.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MeshObject::OnPropertyGridChange::ObjFile");
        _mesh.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "MeshObject::OnPropertyGridChange::ObjFile");
        return 0;
    } else if ("Brightness" == event.GetPropertyName()) {
        _mesh.SetBrightness((int)event.GetPropertyValue().GetLong());
        _mesh.IncrementChangeCount();
        _mesh.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MeshObject::OnPropertyGridChange::Brightness");
        _mesh.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "MeshObject::OnPropertyGridChange::Brightness");
        return 0;
    } else if ("MeshOnly" == event.GetPropertyName()) {
        _mesh.SetMeshOnly(event.GetValue().GetBool());
        _mesh.IncrementChangeCount();
        _mesh.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MeshObject::OnPropertyGridChange::MeshOnly");
        _mesh.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "MeshObject::OnPropertyGridChange::MeshOnly");
        return 0;
    }

    return ViewObjectPropertyAdapter::OnPropertyGridChange(grid, event);
}
