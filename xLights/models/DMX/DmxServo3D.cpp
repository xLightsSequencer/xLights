/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxServo3D.h"
#include "Mesh.h"
#include "Servo.h"
#include "ServoConfigDialog.h"
#include "../../ModelPreview.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"

enum MOTION_LINK {
    MOTION_LINK_MESH1,
    MOTION_LINK_MESH2,
    MOTION_LINK_MESH3,
    MOTION_LINK_MESH4,
    MOTION_LINK_MESH5,
    MOTION_LINK_MESH6,
    MOTION_LINK_MESH7,
    MOTION_LINK_MESH8,
    MOTION_LINK_MESH9,
    MOTION_LINK_MESH10,
    MOTION_LINK_MESH11,
    MOTION_LINK_MESH12,
    MOTION_LINK_MESH13,
    MOTION_LINK_MESH14,
    MOTION_LINK_MESH15,
    MOTION_LINK_MESH16,
    MOTION_LINK_MESH17,
    MOTION_LINK_MESH18,
    MOTION_LINK_MESH19,
    MOTION_LINK_MESH20,
    MOTION_LINK_MESH21,
    MOTION_LINK_MESH22,
    MOTION_LINK_MESH23,
    MOTION_LINK_MESH24
};

DmxServo3d::DmxServo3d(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
    : DmxModel(node, manager, zeroBased)
{
    SetFromXml(node, zeroBased);
}

DmxServo3d::~DmxServo3d()
{
    Clear();
}

void DmxServo3d::Clear() {
    for (auto it = servos.begin(); it != servos.end(); ++it) {
        if (*it != nullptr) {
            delete* it;
        }
    }
    servos.clear();
    for (auto it = motion_meshs.begin(); it != motion_meshs.end(); ++it) {
        if (*it != nullptr) {
            delete* it;
        }
    }
    motion_meshs.clear();
    for (auto it = static_meshs.begin(); it != static_meshs.end(); ++it) {
        if (*it != nullptr) {
            delete* it;
        }
    }
    static_meshs.clear();
}

static const std::string CLICK_TO_EDIT("--Click To Edit--");
class ServoConfigDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    ServoConfigDialogAdapter(DmxServo3d* model) :
        wxPGEditorDialogAdapter(), m_model(model)
    {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
                              wxPGProperty* WXUNUSED(property)) override
    {
        ServoConfigDialog dlg(propGrid);

        dlg.CheckBox_16bits->SetValue(m_model->Is16Bit());
        dlg.SpinCtrl_NumServos->SetValue(m_model->GetNumServos());
        dlg.SpinCtrl_NumStatic->SetValue(m_model->GetNumStatic());
        dlg.SpinCtrl_NumMotion->SetValue(m_model->GetNumMotion());
        dlg.SpinCtrl_NumMotion->SetRange(1, m_model->GetNumServos());

        if (dlg.ShowModal() == wxID_OK) {
            bool changed = false;

            int _num_servos = dlg.SpinCtrl_NumServos->GetValue();
            if (_num_servos != m_model->GetNumServos()) {
                m_model->GetModelXml()->DeleteAttribute("NumServos");
                m_model->GetModelXml()->AddAttribute("NumServos", std::to_string(_num_servos));
                changed = true;
                m_model->UpdateNodeNames();
            }
            int _num_static = dlg.SpinCtrl_NumStatic->GetValue();
            if (_num_static != m_model->GetNumStatic()) {
                m_model->GetModelXml()->DeleteAttribute("NumStatic");
                m_model->GetModelXml()->AddAttribute("NumStatic", std::to_string(_num_static));
                changed = true;
            }
            int _num_motion = dlg.SpinCtrl_NumMotion->GetValue();
            if (_num_motion != m_model->GetNumMotion()) {
                m_model->GetModelXml()->DeleteAttribute("NumMotion");
                m_model->GetModelXml()->AddAttribute("NumMotion", std::to_string(_num_motion));
                changed = true;
            }
            bool _16bit = dlg.CheckBox_16bits->GetValue();
            if (_16bit != m_model->Is16Bit()) {
                m_model->GetModelXml()->DeleteAttribute("Bits16");
                changed = true;
                if (_16bit) {
                    m_model->GetModelXml()->AddAttribute("Bits16", "1");
                } else {
                    m_model->GetModelXml()->AddAttribute("Bits16", "0");
                }
                m_model->UpdateBits();
                m_model->UpdateNodeNames();
            }
            if (changed) {
                m_model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo3d::ServoConfigDialogAdapter");
                m_model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo3d::ServoConfigDialogAdapter");
                m_model->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo3d::ServoConfigDialogAdapter");
                m_model->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo3d::ServoConfigDialogAdapter");
            }

            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            return true;
        }
        return false;
    }

protected:
    DmxServo3d* m_model;
};

class ServoPopupDialogProperty : public wxStringProperty
{
public:
    ServoPopupDialogProperty(DmxServo3d* m,
                             const wxString& label,
                             const wxString& name,
                             const wxString& value,
                             int type) :
        wxStringProperty(label, name, value), m_model(m), m_tp(type)
    {
    }
    // Set editor to have button
    virtual const wxPGEditor* DoGetEditorClass() const override
    {
        return wxPGEditor_TextCtrlAndButton;
    }
    // Set what happens on button click
    virtual wxPGEditorDialogAdapter* GetEditorDialog() const override
    {
        switch (m_tp) {
        case 1:
            return new ServoConfigDialogAdapter(m_model);
        default:
            break;
        }
        return nullptr;
    }

protected:
    DmxServo3d* m_model = nullptr;
    int m_tp;
};

static wxPGChoices MOTION_LINKS;

void DmxServo3d::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    DmxModel::AddTypeProperties(grid, outputManager);

    wxPGProperty* p = grid->Append(new ServoPopupDialogProperty(this, "Servo Config", "ServoConfig", CLICK_TO_EDIT, 1));
    grid->LimitPropertyEditing(p);

    p = grid->Append(new wxBoolProperty("Show Pivot Axes", "PivotAxes", show_pivot));
    p->SetAttribute("UseCheckbox", true);

    for (const auto& it : servos) {
        it->AddTypeProperties(grid);
    }

    for (const auto& it : static_meshs) {
        it->AddTypeProperties(grid);
    }

    for (const auto& it : motion_meshs) {
        it->AddTypeProperties(grid);
    }

    if (MOTION_LINKS.GetCount() != (unsigned int)num_servos) {
        MOTION_LINKS.Clear();
    }
    if (MOTION_LINKS.GetCount() == 0) {
        for (int i = 0; i < num_servos; ++i) {
            MOTION_LINKS.Add("Mesh " + std::to_string(i + 1));
        }
    }

    grid->Append(new wxPropertyCategory("Servo Linkage", "ServoMotionProperties"));
    for (int i = 0; i < num_servos; ++i) {
        std::string linkage = "Servo " + std::to_string(i + 1) + " Linkage";
        std::string linkage2 = "Servo" + std::to_string(i + 1) + "Linkage";
        grid->Append(new wxEnumProperty(linkage, linkage2, MOTION_LINKS, servo_links[i] == -1 ? i : servo_links[i]));
    }
    grid->Collapse("ServoMotionProperties");

    grid->Append(new wxPropertyCategory("Mesh Linkage", "MeshMotionProperties"));
    for (int i = 0; i < num_servos; ++i) {
        std::string linkage = "Mesh " + std::to_string(i + 1) + " Linkage";
        std::string linkage2 = "Mesh" + std::to_string(i + 1) + "Linkage";
        grid->Append(new wxEnumProperty(linkage, linkage2, MOTION_LINKS, mesh_links[i] == -1 ? i : mesh_links[i]));
    }
    grid->Collapse("MeshMotionProperties");

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

int DmxServo3d::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    std::string name = event.GetPropertyName().ToStdString();

    if ("DmxChannelCount" == event.GetPropertyName()) {
        int channels = (int)event.GetPropertyValue().GetLong();
        int min_channels = num_servos * (_16bit ? 2 : 1);
        if (channels < min_channels) {
            wxPGProperty* p = grid->GetPropertyByName("DmxChannelCount");
            if (p != nullptr) {
                p->SetValue(min_channels);
            }
            std::string msg = wxString::Format("You have %d servos at %d bits so you need %d channels minimum.", num_servos, _16bit ? 16 : 8, min_channels);
            wxMessageBox(msg, "Minimum Channel Violation", wxOK | wxCENTER);
            return 0;
        }
    }

    if ("PivotAxes" == name) {
        if (event.GetValue().GetBool()) {
            show_pivot = true;
        } else {
            show_pivot = false;
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo3d::OnPropertyGridChange::PivotAxes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo3d::OnPropertyGridChange::PivotAxes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo3d::OnPropertyGridChange::PivotAxes");
        return 0;
    }

    for (const auto& it : servos) {
        if (it->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked() || IsFromBase()) == 0) {
            return 0;
        }
    }

    for (const auto& it : static_meshs) {
        if (it->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked() || IsFromBase()) == 0) {
            return 0;
        }
    }

    for (const auto& it : motion_meshs) {
        if (it->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked() || IsFromBase()) == 0) {
            return 0;
        }
    }

    for (int i = 0; i < num_servos; ++i) {
        std::string linkage = "Servo" + std::to_string(i + 1) + "Linkage";
        if (linkage == name) {
            ModelXml->DeleteAttribute(linkage);
            int link_num = event.GetPropertyValue().GetLong();
            if (link_num >= num_servos) {
                link_num = i;
            }
            if (link_num == i) {
                servo_links[i] = -1;
            } else {
                servo_links[i] = link_num;
            }
            ModelXml->AddAttribute(linkage, "Mesh " + std::to_string(link_num + 1));
            AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo3d::OnPropertyGridChange::ServoLinkage");
            AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo3d::OnPropertyGridChange::ServoLinkage");
            AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxServo3d::OnPropertyGridChange::ServoLinkage");
            AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "DmxServo3d::OnPropertyGridChange::ServoLinkage");
            AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo3d::OnPropertyGridChange::ServoLinkage");
            AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DmxServo3d::OnPropertyGridChange::ServoLinkage");
            AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo3d::OnPropertyGridChange::ServoLinkage");
            return 0;
        }
        linkage = "Mesh" + std::to_string(i + 1) + "Linkage";
        if (linkage == name) {
            ModelXml->DeleteAttribute(linkage);
            int link_num = event.GetPropertyValue().GetLong();
            if (link_num >= num_servos) {
                link_num = i;
            }
            if (link_num == i) {
                mesh_links[i] = -1;
            } else {
                mesh_links[i] = link_num;
            }
            ModelXml->AddAttribute(linkage, "Mesh " + std::to_string(link_num + 1));
            AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo3d::OnPropertyGridChange::MeshLinkage");
            AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo3d::OnPropertyGridChange::MeshLinkage");
            AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxServo3d::OnPropertyGridChange::MeshLinkage");
            AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "DmxServo3d::OnPropertyGridChange::MeshLinkage");
            AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo3d::OnPropertyGridChange::MeshLinkage");
            AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DmxServo3d::OnPropertyGridChange::MeshLinkage");
            AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo3d::OnPropertyGridChange::MeshLinkage");
            return 0;
        }
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxServo3d::InitModel()
{
    num_servos = wxAtoi(ModelXml->GetAttribute("NumServos", "1"));
    num_static = wxAtoi(ModelXml->GetAttribute("NumStatic", "1"));
    num_motion = wxAtoi(ModelXml->GetAttribute("NumMotion", "1"));
    _16bit = wxAtoi(ModelXml->GetAttribute("Bits16", "1"));

    int min_channels = num_servos * (_16bit ? 2 : 1);
    if (parm1 < min_channels) {
        UpdateChannelCount(min_channels, false);
        std::string msg = wxString::Format("Channel count increased to %d to accommodate %d servos at %d bits.", min_channels, num_servos, _16bit ? 16 : 8);
        wxMessageBox(msg, "Minimum Channel Violation", wxOK | wxCENTER);
    }

    DmxModel::InitModel();
    DisplayAs = "DmxServo3d";

    // clear links
    for (int i = 0; i < SUPPORTED_SERVOS; ++i) {
        servo_links[i] = -1;
        mesh_links[i] = -1;
    }

    // clear any extras
    while (servos.size() > num_servos) {
        Servo* ptr = servos.back();
        if (ptr != nullptr) {
            delete ptr;
            ptr = nullptr;
        }
        servos.pop_back();
    }
    while (static_meshs.size() > num_static) {
        Mesh* ptr = static_meshs.back();
        if (ptr != nullptr) {
            delete ptr;
            ptr = nullptr;
        }
        static_meshs.pop_back();
    }
    while (motion_meshs.size() > num_motion) {
        Mesh* ptr = motion_meshs.back();
        if (ptr != nullptr) {
            delete ptr;
            ptr = nullptr;
        }
        motion_meshs.pop_back();
    }

    // resize vector arrays
    if (static_meshs.size() < num_static) {
        static_meshs.resize(num_static);
    }
    if (motion_meshs.size() < num_motion) {
        motion_meshs.resize(num_motion);
    }
    if (servos.size() < num_servos) {
        servos.resize(num_servos);
    }

    wxXmlNode* n = ModelXml->GetChildren();
    wxXmlNode* snode = nullptr;
    wxXmlNode* mnode = nullptr;

    while (n != nullptr) {
        std::string name = n->GetName();
        int servo_idx = name.find("Servo");
        int static_idx = name.find("StaticMesh");
        int motion_idx = name.find("MotionMesh");

        if ("StaticMesh" == name) { // convert original name that had no number
            // copy attributes to new name
            wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "StaticMesh1");
            ModelXml->AddChild(new_node);
            for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext()) {
                new_node->AddAttribute(a->GetName(), a->GetValue());
            }
            snode = n;
            static_meshs[0] = new Mesh(new_node, "StaticMesh1");
        } else if ("MotionMesh" == name) { // convert original name that had no number
            // copy attributes to new name
            wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "MotionMesh1");
            ModelXml->AddChild(new_node);
            for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext()) {
                new_node->AddAttribute(a->GetName(), a->GetValue());
            }
            mnode = n;
            motion_meshs[0] = new Mesh(new_node, "MotionMesh1");
        } else if (static_idx != std::string::npos) {
            std::string num = name.substr(10, name.length());
            int id = atoi(num.c_str()) - 1;
            if (id < num_static) {
                if (static_meshs[id] == nullptr) {
                    static_meshs[id] = new Mesh(n, name);
                }
            }
        } else if (motion_idx != std::string::npos) {
            std::string num = name.substr(10, name.length());
            int id = atoi(num.c_str()) - 1;
            if (id < num_motion) {
                if (motion_meshs[id] == nullptr) {
                    motion_meshs[id] = new Mesh(n, name);
                }
            }
        } else if (servo_idx != std::string::npos) {
            std::string num = name.substr(5, name.length());
            int id = atoi(num.c_str()) - 1;
            if (id < num_servos) {
                if (servos[id] == nullptr) {
                    servos[id] = new Servo(n, name, false);
                }
            }
        }
        n = n->GetNext();
    }

    // clean up any old nodes from version 2020.3
    if (snode != nullptr) {
        ModelXml->RemoveChild(snode);
        delete snode;
    }
    if (mnode != nullptr) {
        ModelXml->RemoveChild(mnode);
        delete mnode;
    }

    // create any missing servos
    for (int i = 0; i < num_servos; ++i) {
        if (servos[i] == nullptr) {
            std::string new_name = "Servo" + std::to_string(i + 1);
            wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
            ModelXml->AddChild(new_node);
            servos[i] = new Servo(new_node, new_name, true);
            servos[i]->SetChannel(_16bit ? i * 2 + 1 : i + 1, this);
        }
    }

    // create any missing static meshes
    for (int i = 0; i < num_static; ++i) {
        if (static_meshs[i] == nullptr) {
            std::string new_name = "StaticMesh" + std::to_string(i + 1);
            wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
            ModelXml->AddChild(new_node);
            static_meshs[i] = new Mesh(new_node, new_name);
        }
    }

    // create any missing motion meshes
    for (int i = 0; i < num_motion; ++i) {
        if (motion_meshs[i] == nullptr) {
            std::string new_name = "MotionMesh" + std::to_string(i + 1);
            wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
            ModelXml->AddChild(new_node);
            motion_meshs[i] = new Mesh(new_node, new_name);
        }
    }

    // get servo linkages
    for (int i = 0; i < num_servos; ++i) {
        servo_links[i] = -1;
        std::string num = std::to_string(i + 1);
        std::string this_link = "Servo" + num + "Linkage";
        std::string this_default = "Mesh " + num;
        std::string link = ModelXml->GetAttribute(this_link, this_default);
        if (link.length() < 5) {
            link = "Mesh 1";
        }
        std::string num2 = link.substr(5, name.length());
        int link_id = atoi(num2.c_str());
        if (link_id < 1) {
            link_id = 1;
        }
        if (link_id != i + 1) {
            servo_links[i] = link_id - 1;
        }
    }

    // get mesh linkages
    for (int i = 0; i < num_servos; ++i) {
        mesh_links[i] = -1;
        std::string num = std::to_string(i + 1);
        std::string this_link = "Mesh" + num + "Linkage";
        std::string this_default = "Mesh " + num;
        std::string link = ModelXml->GetAttribute(this_link, this_default);
        if (link.length() < 5) {
            link = "Mesh 1";
        }
        std::string num2 = link.substr(5, name.length());
        int link_id = atoi(num2.c_str());
        if (link_id < 1) {
            link_id = 1;
        }
        if (link_id != i + 1) {
            mesh_links[i] = link_id - 1;
        }
    }

    brightness = wxAtoi(ModelXml->GetAttribute("Brightness", "100"));

    for (const auto& it : servos) {
        it->Init(this);
        it->Set16Bit(_16bit);
    }

    bool last_exists = false;
    for (const auto& it : static_meshs) {
        it->Init(this, !last_exists);
        last_exists = it->HasObjFile();
    }

    last_exists = num_static > 0 ? static_meshs[0]->HasObjFile() : false;
    for (const auto& it : motion_meshs) {
        it->Init(this, !last_exists);
        last_exists = it->HasObjFile();
    }

    // renumber servo changed if number of bits changed
    if (update_bits) {
        for (int i = 0; i < num_servos; ++i) {
            if (servos[i] != nullptr) {
                servos[i]->SetChannel(_16bit ? i * 2 + 1 : i + 1, this);
            }
        }
        update_bits = false;
    }

    // create node names
    std::string names = "";
    int index = 1;
    for (auto it = servos.begin(); it != servos.end(); ++it) {
        if (!names.empty()) {
            names += ",";
        }
        if (_16bit) {
            names += "Servo" + std::to_string(index) + ",-Servo" + std::to_string(index);
        } else {
            names += "Servo" + std::to_string(index);
        }
        index++;
    }
    SetNodeNames(names, update_node_names);
    update_node_names = false;
}

void DmxServo3d::DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext* ctx,
                                      xlGraphicsProgram* sprogram, xlGraphicsProgram* tprogram, bool is_3d,
                                      const xlColor* c, bool allowSelected, bool wiring,
                                      bool highlightFirst, int highlightpixel,
                                      float* boundingBox)
{
    if (!IsActive())
        return;

    screenLocation.PrepareToDraw(is_3d, allowSelected);
    screenLocation.UpdateBoundingBox(Nodes);
    if (boundingBox) {
        boundingBox[0] = -0.5;
        boundingBox[1] = -0.5;
        boundingBox[2] = -0.5;
        boundingBox[3] = 0.5;
        boundingBox[4] = 0.5;
        boundingBox[5] = 0.5;
    }
    sprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0 plane
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    tprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0 plane
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    DrawModel(preview, ctx, sprogram, tprogram, !allowSelected);
    sprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->PopMatrix();
    });
    tprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->PopMatrix();
    });
    if ((Selected || (Highlighted && is_3d)) && c != nullptr && allowSelected) {
        if (is_3d) {
            GetModelScreenLocation().DrawHandles(tprogram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), Highlighted, IsFromBase());
        } else {
            GetModelScreenLocation().DrawHandles(tprogram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), IsFromBase());
        }
    }
}

void DmxServo3d::DisplayEffectOnWindow(ModelPreview* preview, double pointSize)
{
    if (!IsActive() && preview->IsNoCurrentModel()) {
        return;
    }

    bool mustEnd = false;
    xlGraphicsContext* ctx = preview->getCurrentGraphicsContext();
    if (ctx == nullptr) {
        bool success = preview->StartDrawing(pointSize);
        if (success) {
            ctx = preview->getCurrentGraphicsContext();
            mustEnd = true;
        }
    }
    if (ctx) {
        int w, h;
        preview->GetSize(&w, &h);
        float scaleX = float(w) * 0.95f / GetModelScreenLocation().RenderWi;
        float scaleY = float(h) * 0.95f / GetModelScreenLocation().RenderHt;

        float aspect = screenLocation.GetScaleX();
        aspect /= screenLocation.GetScaleY();
        if (scaleY < scaleX) {
            scaleX = scaleY * aspect;
        } else {
            scaleY = scaleX / aspect;
        }
        float ml, mb;
        GetMinScreenXY(ml, mb);
        ml += GetModelScreenLocation().RenderWi / 2;
        mb += GetModelScreenLocation().RenderHt / 2;

        preview->getCurrentTransparentProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PushMatrix();
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
            ctx->TranslateViewMatrix(w / 2.0f - (ml < 0.0f ? ml : 0.0f),
                                     h / 2.0f - (mb < 0.0f ? mb : 0.0f), 0.0f);
            ctx->ScaleViewMatrix(scaleX, scaleY, 1.0);
        });
        preview->getCurrentSolidProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PushMatrix();
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
            ctx->TranslateViewMatrix(w / 2.0f - (ml < 0.0f ? ml : 0.0f),
                                     h / 2.0f - (mb < 0.0f ? mb : 0.0f), 0.0f);
            ctx->ScaleViewMatrix(scaleX, scaleY, 1.0f);
        });
        DrawModel(preview, ctx, preview->getCurrentSolidProgram(), preview->getCurrentTransparentProgram(), true);
        preview->getCurrentTransparentProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PopMatrix();
        });
        preview->getCurrentSolidProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PopMatrix();
        });
    }
    if (mustEnd) {
        preview->EndDrawing();
    }
}

std::list<std::string> DmxServo3d::CheckModelSettings()
{
    std::list<std::string> res;

    int nodeCount = Nodes.size();
    int min_channels = num_servos * (_16bit ? 2 : 1);

    if (min_channels > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s requires more channels %d than have been allocated to it %d.", GetName(), min_channels, nodeCount));
    }
    int i = 1;
    for (const auto& it : servos) {
        if (it->GetChannel() > nodeCount) {
            res.push_back(wxString::Format("    ERR: Model %s servo %d is assigned to channel %d but the model only has %d channels.", GetName(), i, it->GetChannel(), nodeCount));
        }
        i++;
    }

    res.splice(res.end(), Model::CheckModelSettings());
    return res;
}

void DmxServo3d::DrawModel(ModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* sprogram, xlGraphicsProgram* tprogram, bool active)
{
    // crash protection
    int min_channels = num_servos * (_16bit ? 2 : 1);
    if (min_channels > Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    for (const auto& it : servos) {
        if (it->GetChannel() > Nodes.size()) {
            DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
            return;
        }
    }

    float servo_pos[SUPPORTED_SERVOS] = { 0.0f };
    glm::mat4 Identity = glm::mat4(1.0f);
    glm::mat4 servo_matrix[SUPPORTED_SERVOS];
    glm::mat4 motion_matrix[SUPPORTED_SERVOS];

    // Draw Static Meshs
    for (int i = 0; i < num_static; ++i) {
        static_meshs[i]->Draw(this, preview, sprogram, tprogram, Identity, Identity, i < num_motion ? !motion_meshs[i]->GetExists(this, ctx) : false, 0, 0, 0, false, false);
    }

    // Get servo positions and fill motion matrices
    for (int i = 0; i < servos.size(); ++i) {
        if (servos[i]->GetChannel() > 0 && active) {
            servo_pos[i] = servos[i]->GetPosition(GetChannelValue(servos[i]->GetChannel() - 1, servos[i]->Is16Bit()));
            if (servos[i]->IsTranslate()) {
                glm::vec3 scale = GetBaseObjectScreenLocation().GetScaleMatrix();
                servo_pos[i] /= scale.x;
            }
        }
        servo_matrix[i] = Identity;
        motion_matrix[i] = Identity;
        servos[i]->FillMotionMatrix(servo_pos[i], servo_matrix[i]);
    }

    // Determine motion mesh linkages
    for (int i = 0; i < num_motion; ++i) {
        int link = mesh_links[i];
        int nesting = num_motion;
        std::vector<int> link_list;
        // if mesh does not link to himself
        if (link != i) {
            // iterate through pushing mesh links into a list
            while ((link != -1) && (nesting > 0)) {
                link_list.push_back(link);
                link = mesh_links[link];
                nesting--; // prevents circular loops from hanging things up
            }
            // multiply motion matrix based on list in reverse order
            while (!link_list.empty()) {
                link = link_list.back();
                link_list.pop_back();
                motion_matrix[i] = motion_matrix[i] * servo_matrix[link];
            }
        }
    }

    // add motion based on servo mapping
    for (int i = 0; i < servos.size(); ++i) {
        // see if servo links to his own mesh
        if (servo_links[i] == -1) {
            motion_matrix[i] = motion_matrix[i] * servo_matrix[i];
        }
        // check if any other servos map to this mesh
        for (int j = 0; j < servos.size(); ++j) {
            if (j != i) {
                if (servo_links[j] == i) {
                    motion_matrix[i] = motion_matrix[i] * servo_matrix[j];
                }
            }
        }
    }

    // Draw Motion Meshs
    for (int i = 0; i < num_motion; ++i) {
        motion_meshs[i]->Draw(this, preview, sprogram, tprogram, Identity, motion_matrix[i], i < num_static ? !static_meshs[i]->GetExists(this, ctx) : false,
                              servos[i]->GetPivotOffsetX(), servos[i]->GetPivotOffsetY(), servos[i]->GetPivotOffsetZ(), servos[i]->IsRotate() && show_pivot, !active);
    }
}

void DmxServo3d::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty())
        return;
    wxFile f(filename);
    if (!f.Create(filename, true) || !f.IsOpened())
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dmxservo3d \n");

    ExportBaseParameters(f);

    f.Write(wxString::Format("NumServos=\"%i\" ", num_servos));
    f.Write(wxString::Format("NumStatic=\"%i\" ", num_static));
    f.Write(wxString::Format("NumMotion=\"%i\" ", num_motion));

    wxString bits = ModelXml->GetAttribute("Bits16");
    f.Write(wxString::Format("Bits16=\"%s\" ", bits));

    // servo linkages
    for (int i = 0; i < num_servos; ++i) {
        std::string num = std::to_string(i + 1);
        std::string this_link = "Servo" + num + "Linkage";
        if( ModelXml->HasAttribute(this_link)) {
            std::string link = ModelXml->GetAttribute(this_link, "");
            f.Write(wxString::Format("%s=\"%s\" ", this_link,link));
        }
    }

    // mesh linkages
    for (int i = 0; i < num_servos; ++i) {
        std::string num = std::to_string(i + 1);
        std::string this_link = "Mesh" + num + "Linkage";
        if( ModelXml->HasAttribute(this_link)) {
            std::string link = ModelXml->GetAttribute(this_link, "");
            f.Write(wxString::Format("%s=\"%s\" ", this_link,link));
        }
    }

    f.Write(" >\n");

    wxString show_dir = GetModelManager().GetXLightsFrame()->GetShowDirectory();

    for (auto it = static_meshs.begin(); it != static_meshs.end(); ++it) {
        (*it)->Serialise(ModelXml, f, show_dir);
    }
    for (auto it = motion_meshs.begin(); it != motion_meshs.end(); ++it) {
        (*it)->Serialise(ModelXml, f, show_dir);
    }
    for (auto it = servos.begin(); it != servos.end(); ++it) {
        (*it)->Serialise(ModelXml, f, show_dir);
    }

    wxString submodel = SerialiseSubmodel();
    if (submodel != "") {
        f.Write(submodel);
    }
    wxString state = SerialiseState();
    if (state != "") {
        f.Write(state);
    }
    wxString groups = SerialiseGroups();
    if (groups != "") {
        f.Write(groups);
    }
    //ExportDimensions(f);
    f.Write("</dmxservo3d>");
    f.Close();
}

void DmxServo3d::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    if (root->GetName() == "dmxservo3d" || root->GetName() == "dmxservo3axis") {
        ImportBaseParameters(root);

        wxString name = root->GetAttribute("name");
        wxString v = root->GetAttribute("SourceVersion");
        wxString bits = root->GetAttribute("Bits16", "1");

        // Add any model version conversion logic here
        // Source version will be the program version that created the custom model

        wxXmlNode* n = root->GetChildren();
        while (n != nullptr) {
            std::string name = n->GetName();

            if ("StaticMesh" == name) { // convert original name that had no number
                // copy attributes to new name
                wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "StaticMesh1");
                root->AddChild(new_node);
                for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext()) {
                    new_node->AddAttribute(a->GetName(), a->GetValue());
                }
            } else if ("MotionMesh" == name) { // convert original name that had no number
                // copy attributes to new name
                wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "MotionMesh1");
                root->AddChild(new_node);
                for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext()) {
                    new_node->AddAttribute(a->GetName(), a->GetValue());
                }
            }
            n = n->GetNext();
        }

        wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
        GetModelScreenLocation().Write(ModelXml);
        SetProperty("name", newname, true);
        SetProperty("Bits16", bits);

        num_servos = wxAtoi(root->GetAttribute("NumServos", "1"));
        num_static = wxAtoi(root->GetAttribute("NumStatic", "1"));
        num_motion = wxAtoi(root->GetAttribute("NumMotion", "1"));
        SetProperty("NumServos", std::to_string(num_servos));
        SetProperty("NumStatic", std::to_string(num_static));
        SetProperty("NumMotion", std::to_string(num_motion));

        // resize vector arrays
        if (static_meshs.size() < num_static) {
            static_meshs.resize(num_static);
        }
        if (motion_meshs.size() < num_motion) {
            motion_meshs.resize(num_motion);
        }
        if (servos.size() < num_servos) {
            servos.resize(num_servos);
        }

        // create any missing servos
        for (int i = 0; i < num_servos; ++i) {
            if (servos[i] == nullptr) {
                std::string new_name = "Servo" + std::to_string(i + 1);
                wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
                ModelXml->AddChild(new_node);
                servos[i] = new Servo(new_node, new_name, true);
                servos[i]->SetChannel(_16bit ? i * 2 + 1 : i + 1, this);
            }
        }

        // create any missing static meshes
        for (int i = 0; i < num_static; ++i) {
            if (static_meshs[i] == nullptr) {
                std::string new_name = "StaticMesh" + std::to_string(i + 1);
                wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
                ModelXml->AddChild(new_node);
                static_meshs[i] = new Mesh(new_node, new_name);
            }
        }

        // create any missing motion meshes
        for (int i = 0; i < num_motion; ++i) {
            if (motion_meshs[i] == nullptr) {
                std::string new_name = "MotionMesh" + std::to_string(i + 1);
                wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
                ModelXml->AddChild(new_node);
                motion_meshs[i] = new Mesh(new_node, new_name);
            }
        }

        // servo linkages
        for (int i = 0; i < num_servos; ++i) {
            std::string num = std::to_string(i + 1);
            std::string this_link = "Servo" + num + "Linkage";
            if( root->HasAttribute(this_link)) {
                wxString link = root->GetAttribute(this_link);
                SetProperty(this_link, link);
            }
        }

        // mesh linkages
        for (int i = 0; i < num_servos; ++i) {
            std::string num = std::to_string(i + 1);
            std::string this_link = "Mesh" + num + "Linkage";
            if( root->HasAttribute(this_link)) {
                wxString link = root->GetAttribute(this_link);
                SetProperty(this_link, link);
            }
        }

        wxString show_dir = GetModelManager().GetXLightsFrame()->GetShowDirectory();
        for (auto it = static_meshs.begin(); it != static_meshs.end(); ++it) {
            (*it)->Serialise(root, ModelXml, show_dir);
        }
        for (auto it = motion_meshs.begin(); it != motion_meshs.end(); ++it) {
            (*it)->Serialise(root, ModelXml, show_dir);
        }
        for (auto it = servos.begin(); it != servos.end(); ++it) {
            (*it)->Serialise(root, ModelXml, show_dir);
        }

        ImportModelChildren(root, xlights, newname, min_x, max_x, min_y, max_y);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo3d::ImportXlightsModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo3d::ImportXlightsModel");
    } else {
        DisplayError("Failure loading DmxServo3d model file.");
    }
}
