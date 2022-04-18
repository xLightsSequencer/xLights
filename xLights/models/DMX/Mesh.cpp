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
#include <wx/sstream.h>

#include <algorithm>
#include <filesystem>

#include "DmxModel.h"
#include "Mesh.h"
#include "Servo.h"
#include "../../UtilFunctions.h"
#include "../../ExternalHooks.h"
#include "../../ModelPreview.h"
#include "../../xLightsMain.h"
#include <log4cpp/Category.hh>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../../graphics/tiny_obj_loader.h"
#include "../../graphics/xlMesh.h"
#include "../../graphics/xlGraphicsContext.h"

Mesh::Mesh(wxXmlNode* node, wxString _name)
 : node_xml(node), _objFile(""), base_name(_name)
{
}

Mesh::~Mesh()
{
    if (mesh3d) {
        delete mesh3d;
    }
}

void Mesh::SetRenderScaling(float s) {
    rscale = s;
}

void Mesh::Init(BaseObject* base, bool set_size) {
	_objFile = FixFile("", node_xml->GetAttribute("ObjFile", ""));
    if (_objFile != node_xml->GetAttribute("ObjFile", "")) {
        node_xml->DeleteAttribute("ObjFile");
        node_xml->AddAttribute("ObjFile", _objFile);
    }

    mesh_only = node_xml->GetAttribute("MeshOnly", "0") == "1";

    brightness = wxAtoi(node_xml->GetAttribute("Brightness", "100"));
    if (brightness > 100) {
        brightness = 100;
    } else if (brightness < 0) {
        brightness = 0;
    }

    offset_x = wxAtof(node_xml->GetAttribute("OffsetX", "0.0"));
    offset_y = wxAtof(node_xml->GetAttribute("OffsetY", "0.0"));
    offset_z = wxAtof(node_xml->GetAttribute("OffsetZ", "0.0"));

    scalex = wxAtof(node_xml->GetAttribute("ScaleX", "1.0"));
    scaley = wxAtof(node_xml->GetAttribute("ScaleY", "1.0"));
    scalez = wxAtof(node_xml->GetAttribute("ScaleZ", "1.0"));

    if (scalex < 0) {
        scalex = 1.0f;
    }
    if (scaley < 0) {
        scaley = 1.0f;
    }
    if (scalez < 0) {
        scalez = 1.0f;
    }

    rotatex = wxAtof(node_xml->GetAttribute("RotateX", "0.0f"));
    rotatey = wxAtof(node_xml->GetAttribute("RotateY", "0.0f"));
    rotatez = wxAtof(node_xml->GetAttribute("RotateZ", "0.0f"));

    if (rotatex < -180.0f || rotatex > 180.0f) {
        rotatex = 0.0f;
    }
    if (rotatey < -180.0f || rotatey > 180.0f) {
        rotatey = 0.0f;
    }
    if (rotatez < -180.0f || rotatez > 180.0f) {
        rotatez = 0.0f;
    }

    controls_size = set_size;
    if (controls_size) {
        width = wxAtof(node_xml->GetAttribute("Width", "1.0f"));
        height = wxAtof(node_xml->GetAttribute("Height", "1.0f"));
        depth = wxAtof(node_xml->GetAttribute("Depth", "1.0f"));
        base->GetBaseObjectScreenLocation().SetRenderSize(width, height, depth);
    }
}

void Mesh::AddTypeProperties(wxPropertyGridInterface *grid) {
    grid->Append(new wxPropertyCategory(base_name, base_name + "Properties"));
    
    wxPGProperty * prop = grid->Append(new wxFileProperty("ObjFile", base_name + "ObjFile", _objFile));
    prop->SetAttribute(wxPG_FILE_WILDCARD, "Wavefront files|*.obj|All files (*.*)|*.*");

    prop = grid->Append(new wxBoolProperty("Mesh Only", base_name + "MeshOnly", mesh_only));
    prop->SetAttribute("UseCheckbox", true);
    prop = grid->Append(new wxUIntProperty("Brightness", base_name + "Brightness", brightness));
    prop->SetAttribute("Min", 0);
    prop->SetAttribute("Max", 100);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("Offset X", base_name + "OffsetX", offset_x));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("Offset Y", base_name + "OffsetY", offset_y));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("Offset Z", base_name + "OffsetZ", offset_z));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleX", base_name + "ScaleX", scalex));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleY", base_name + "ScaleY", scaley));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleZ", base_name + "ScaleZ", scalez));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateX", base_name + "RotateX", rotatex));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateY", base_name + "RotateY", rotatey));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateZ", base_name + "RotateZ", rotatez));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");

    grid->Collapse(base_name + "Properties");
}

int Mesh::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked) {
    std::string name = event.GetPropertyName().ToStdString();
    if (!locked && base_name + "ObjFile" == name) {
        obj_loaded = false;
        obj_exists = false;
        if (mesh3d) {
            delete mesh3d;
            mesh3d = nullptr;
        }
        if (controls_size) {
            node_xml->DeleteAttribute("Width");
            node_xml->DeleteAttribute("Height");
            node_xml->DeleteAttribute("Depth");
        }
        uncacheDisplayObjects();
        _objFile = event.GetValue().GetString();
        ObtainAccessToURL(_objFile);
        node_xml->DeleteAttribute("ObjFile");
        node_xml->AddAttribute("ObjFile", _objFile);
        
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

        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::ObjFile");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::ObjFile");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::ObjFile");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::ObjFile");
        return 0;
    }
    else if (locked && base_name + "ObjFile" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "Brightness" == name) {
        brightness = (int)event.GetPropertyValue().GetLong();
        node_xml->DeleteAttribute("Brightness");
        node_xml->AddAttribute("Brightness", wxString::Format("%d", (int)brightness));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::Brightness");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::Brightness");
        return 0;
    }
    else if (locked && base_name + "Brightness" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "MeshOnly" == name) {
        node_xml->DeleteAttribute("MeshOnly");
        mesh_only = event.GetValue().GetBool();
        if (mesh_only) {
            node_xml->AddAttribute("MeshOnly", "1");
        }
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::MeshOnly");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::MeshOnly");
        return 0;
    }
    else if (locked && base_name + "MeshOnly" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "ScaleX" == name) {
        scalex = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("ScaleX");
        node_xml->AddAttribute("ScaleX", wxString::Format("%6.4f", scalex));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::ScaleX");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::ScaleX");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::ScaleX");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::ScaleX");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (locked && base_name + "ScaleX" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "ScaleY" == name) {
        scaley = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("ScaleY");
        node_xml->AddAttribute("ScaleY", wxString::Format("%6.4f", scaley));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::ScaleY");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::ScaleY");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::ScaleY");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::ScaleY");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (locked && base_name + "ScaleY" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "ScaleZ" == name) {
        scalez = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("ScaleZ");
        node_xml->AddAttribute("ScaleZ", wxString::Format("%6.4f", scalez));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::ScaleZ");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::ScaleZ");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::ScaleZ");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::ScaleZ");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (locked && base_name + "ScaleZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "OffsetX" == name) {
        offset_x = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("OffsetX");
        node_xml->AddAttribute("OffsetX", wxString::Format("%6.4f", offset_x));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::ModelX");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::ModelX");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::ModelX");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::ModelX");
        return 0;
    }
    else if (locked && base_name + "OffsetX" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "OffsetY" == name) {
        offset_y = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("OffsetY");
        node_xml->AddAttribute("OffsetY", wxString::Format("%6.4f", offset_y));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::ModelY");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::ModelY");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::ModelY");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::ModelY");
        return 0;
    }
    else if (locked && base_name + "OffsetY" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "OffsetZ" == name) {
        offset_z = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("OffsetZ");
        node_xml->AddAttribute("OffsetZ", wxString::Format("%6.4f", offset_z));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::ModelZ");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::ModelZ");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::ModelZ");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::ModelZ");
        return 0;
    }
    else if (locked && base_name + "OffsetZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "RotateX" == name) {
        rotatex = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("RotateX");
        node_xml->AddAttribute("RotateX", wxString::Format("%4.8f", rotatex));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::RotateX");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::RotateX");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::RotateX");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::RotateX");
        return 0;
    }
    else if (locked && base_name + "RotateX" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "RotateY" == name) {
        rotatey = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("RotateY");
        node_xml->AddAttribute("RotateY", wxString::Format("%4.8f", rotatey));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::RotateY");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::RotateY");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::RotateY");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::RotateY");
        return 0;
    }
    else if (locked && base_name + "RotateY" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "RotateZ" == name) {
        rotatez = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("RotateZ");
        node_xml->AddAttribute("RotateZ", wxString::Format("%4.8f", rotatez));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::RotateZ");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::RotateZ");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::RotateZ");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::RotateZ");
        return 0;
    }
    else if (locked && base_name + "RotateZ" == name) {
        event.Veto();
        return 0;
    }

    return -1;
}

void Mesh::uncacheDisplayObjects() {
    if (mesh3d) {
        delete mesh3d;
        mesh3d = nullptr;
    }
}

void Mesh::loadObject(BaseObject* base, xlGraphicsContext *ctx) {
    if (FileExists(_objFile)) {
        uncacheDisplayObjects();
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        obj_exists = true;
                
        logger_base.debug("Loading mesh model file %s.",
                          (const char *)_objFile.c_str());
        mesh3d = ctx->loadMeshFromObjFile(_objFile);
        width = mesh3d->GetXMax() - mesh3d->GetXMin();
        height = mesh3d->GetYMax() - mesh3d->GetYMin();
        depth = mesh3d->GetZMax() - mesh3d->GetZMin();
        obj_loaded = true;
        
        std::string name = base->GetName() + ":" + base_name;
        mesh3d->SetName(name);
    }
}
bool Mesh::GetExists(BaseObject* base, xlGraphicsContext *ctx) {
    if (!obj_loaded) {
        loadObject(base, ctx);
        base->GetBaseObjectScreenLocation().SetSupportsZScaling(true);  // set here instead of constructor so model creation doesn't go up like a tree
    }
    return mesh3d != nullptr;
}

void Mesh::Draw(BaseObject* base, ModelPreview* preview, xlGraphicsProgram *sprogram, xlGraphicsProgram *tprogram,
                glm::mat4& base_matrix, glm::mat4& motion_matrix,
                bool show_empty, float pivot_offset_x, float pivot_offset_y, float pivot_offset_z, bool rotation, bool use_pivot)
{
    if (!obj_loaded) {
        loadObject(base, preview->getCurrentGraphicsContext());
        base->GetBaseObjectScreenLocation().SetSupportsZScaling(true);  // set here instead of constructor so model creation doesn't go up like a tree
    }

    if (obj_loaded) {
        glm::mat4 Identity = glm::mat4(1.0f);
        glm::mat4 scalingMatrix = glm::scale(Identity, glm::vec3(scalex * rscale, scaley * rscale, scalez * rscale));
        glm::mat4 rx = glm::rotate(Identity, glm::radians(rotatex), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ry = glm::rotate(Identity, glm::radians(rotatey), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rz = glm::rotate(Identity, glm::radians(rotatez), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::quat rotate_quat = glm::quat_cast(rx * ry * rz);
        glm::mat4 translationMatrix = glm::translate(Identity, glm::vec3(offset_x, offset_y, offset_z));
        glm::mat4 m;
        if (rotation) {
            glm::mat4 pivotToZero = glm::translate(Identity, glm::vec3(-pivot_offset_x, -pivot_offset_y, -pivot_offset_z));
            glm::mat4 pivotBack = glm::translate(Identity, glm::vec3(pivot_offset_x, pivot_offset_y, pivot_offset_z));
            m = base_matrix * translationMatrix * pivotBack * motion_matrix * pivotToZero * glm::toMat4(rotate_quat) * scalingMatrix;
        } else {
            m = base_matrix * translationMatrix * motion_matrix * glm::toMat4(rotate_quat) * scalingMatrix;
        }

        if (controls_size) {
            if (node_xml != nullptr && !node_xml->HasAttribute("Width")) {
                base->GetBaseObjectScreenLocation().AdjustRenderSize(width * scalex, height * scaley * half_height, depth * scalez, base->GetModelXml());
                node_xml->AddAttribute("Width", std::to_string(width));
                node_xml->AddAttribute("Height", std::to_string(height));
                node_xml->AddAttribute("Depth", std::to_string(depth));
            }
            base->GetBaseObjectScreenLocation().UpdateBoundingBox(width, height, depth);  // FIXME: Modify to only call this when position changes
        }

        // skip updating color if it hasn't changed.
        if (update_color) {
            if (new_color != last_color) {
                last_color = new_color;
            } else {
                update_color = false;
            }
        }
        if (update_color) {
            mesh3d->SetMaterialColor(color_name, &new_color);
        }

        auto vac = sprogram->getAccumulator();
        int start = vac->getCount();
        if (use_pivot && rotation) {
            /*
            glm::vec3 scale = base->GetBaseObjectScreenLocation().GetScaleMatrix();
            float mw = std::min(std::min(4.0f / scale.x, 4.0f / scale.y);
            float mw2 = std::min(std::min(3.0f / scale.x, 3.0f / scale.y);

            
            xlColor pink = xlColor(255, 0, 255);
            vac->AddCircleAsTriangles(0, 0, 0, mw, xlBLACK);
            vac->AddCircleAsTriangles(0, 0, 0, mw2, pink);
            vac->AddVertex(0.0-mw/2, 0.0-mw/2, 0, xlBLACK);
            vac->AddVertex(0.0+mw/2, 0.0+mw/2, 0, xlBLACK);
            vac->AddVertex(0.0-mw/2, 0.0+mw/2, 0, xlBLACK);
            vac->AddVertex(0.0+mw/2, 0.0-mw/2, 0, xlBLACK);
             
             
             
             glm::vec3 scale = base->GetBaseObjectScreenLocation().GetScaleMatrix();
             float x1 = pivot_offset_x;
             float y1 = pivot_offset_y;
             float z1 = pivot_offset_z;
             float offx = width * scalex * scale.x * rscale;
             float offz = depth * scalez * scale.z * rscale;
             glm::vec4 v = base_matrix * translationMatrix * glm::vec4(glm::vec3(x1, y1, z1), 1.0f);
             x1 = v.x; y1 = v.y; z1 = v.z;
             va.AddVertex(x1 - offx, y1, z1, pink);
             va.AddVertex(x1 + offx, y1, z1, pink);
             va.AddVertex(x1, y1, z1 - offz, pink);
             va.AddVertex(x1, y1, z1 + offz, pink);
             va.Finish(GL_LINES);
            */
        }

        int end = vac->getCount();
        sprogram->addStep([=](xlGraphicsContext *ctx) {
            ctx->PushMatrix();
            ctx->ApplyMatrix(m);
            if (mesh_only) {
                ctx->drawMeshWireframe(mesh3d, this->brightness);
            } else {
                ctx->drawMeshSolids(mesh3d, this->brightness, preview->Is3D());
            }
            if (end != start) {
                if (rotation) {
                    ctx->Translate(pivot_offset_x, pivot_offset_y, 0.0f);
                }
                ctx->drawTriangles(vac, start, end - 4 - start);
                ctx->drawLines(vac, end - 4, 4);
            }
            ctx->PopMatrix();
        });
        if (!mesh_only) {
            tprogram->addStep([=](xlGraphicsContext *ctx) {
                ctx->PushMatrix();
                ctx->ApplyMatrix(m);
                ctx->drawMeshTransparents(mesh3d, this->brightness);
                if (end != start) {
                    if (rotation) {
                        ctx->Translate(pivot_offset_x, pivot_offset_y, 0.0f);
                    }
                    ctx->drawTriangles(vac, start, end - 4 - start);
                    ctx->drawLines(vac, end - 4, 4);
                }
                ctx->PopMatrix();
            });
        }
    } else if( show_empty ) {
        DmxModel::DrawInvalid(sprogram, nullptr, false, false);
    }
}

// Serialize for output
void Mesh::Serialise(wxXmlNode* root, wxFile& f, const wxString& show_dir) const
{
    wxString res = "";

    wxXmlNode* child = root->GetChildren();
    while (child != nullptr) {
        if (child->GetName() == base_name) {
            wxXmlDocument new_doc;
            new_doc.SetRoot(new wxXmlNode(*child));
            wxStringOutputStream stream;
            new_doc.Save(stream);
            wxString s = stream.GetString();
            s = s.SubString(s.Find("\n") + 1, s.Length()); // skip over xml format header
            int index = s.Find(show_dir);
            while (index != wxNOT_FOUND) {
                s = s.SubString(0, index-1) + s.SubString(index + show_dir.Length() + 1, s.Length());
                index = s.Find(show_dir);
            }
            res += s;
        }
        child = child->GetNext();
    }

    if (res != "")
    {
        f.Write(res);
    }
}

// Serialise for input
void Mesh::Serialise(wxXmlNode* root, wxXmlNode* model_xml, const wxString& show_dir) const
{
    wxXmlNode* node = nullptr;
    for (wxXmlNode* n = model_xml->GetChildren(); n != nullptr; n = n->GetNext())
    {
        if (n->GetName() == base_name)
        {
            node = n;
            break;
        }
    }

    if (node != nullptr) {
        // add new attributes from import
        for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
        {
            if (n->GetName() == base_name)
            {
                for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext())
                {
                    wxString s = a->GetValue();
                    if (a->GetName() == "ObjFile") {
                        s = show_dir + wxFileName::GetPathSeparator() + s;
                    }
                    if (node->HasAttribute(a->GetName())) {
                        node->DeleteAttribute(a->GetName());
                    }
                    node->AddAttribute(a->GetName(), s);
                }
                return;
            }
        }
    }
}
