#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxServo3Axis.h"
#include "Servo.h"
#include "../../ModelPreview.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"

enum MESH_LINKS {
    MESH_LINK_MESH1,
    MESH_LINK_MESH2,
    MESH_NOT_LINKED
};

enum MOTION_LINKS {
    MOTION_LINK_MESH1,
    MOTION_LINK_MESH2,
    MOTION_LINK_MESH3
};

DmxServo3Axis::DmxServo3Axis(wxXmlNode* node, const ModelManager& manager, bool zeroBased)
    : DmxModel(node, manager, zeroBased), brightness(100.0f), static_mesh(nullptr),
    motion_mesh1(nullptr), motion_mesh2(nullptr), motion_mesh3(nullptr),
    servo1(nullptr), servo2(nullptr), servo3(nullptr),
    servo2_motion_link_val(MOTION_LINK_MESH2), servo3_motion_link_val(MOTION_LINK_MESH3),
    servo2_motion_link("Mesh 2"), servo3_motion_link("Mesh 3"),
    mesh2_motion_link_val(MESH_NOT_LINKED), mesh3_motion_link_val(MESH_NOT_LINKED),
    mesh2_motion_link("Not Linked"), mesh3_motion_link("Not Linked")
{
    wxXmlNode* n = node->GetChildren();
    while (n != nullptr) {
        if ("StaticMesh" == n->GetName()) {
            static_mesh = new Mesh(n, "StaticMesh");
        }
        else if ("MotionMesh1" == n->GetName()) {
            motion_mesh1 = new Mesh(n, "MotionMesh1");
        }
        else if ("MotionMesh2" == n->GetName()) {
            motion_mesh2 = new Mesh(n, "MotionMesh2");
        }
        else if ("MotionMesh3" == n->GetName()) {
            motion_mesh3 = new Mesh(n, "MotionMesh3");
        }
        else if ("Servo1" == n->GetName()) {
            servo1 = new Servo(n, "Servo1");
        }
        else if ("Servo2" == n->GetName()) {
            servo2 = new Servo(n, "Servo2");
        }
        else if ("Servo3" == n->GetName()) {
            servo3 = new Servo(n, "Servo3");
        }
        n = n->GetNext();
    }
    if (static_mesh == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "StaticMesh");
        node->AddChild(new_node);
        static_mesh = new Mesh(new_node, "StaticMesh");
    }
    if (motion_mesh1 == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "MotionMesh1");
        node->AddChild(new_node);
        motion_mesh1 = new Mesh(new_node, "MotionMesh1");
    }
    if (motion_mesh2 == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "MotionMesh2");
        node->AddChild(new_node);
        motion_mesh2 = new Mesh(new_node, "MotionMesh2");
    }
    if (motion_mesh3 == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "MotionMesh3");
        node->AddChild(new_node);
        motion_mesh3 = new Mesh(new_node, "MotionMesh3");
    }
    if (servo1 == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "Servo1");
        node->AddChild(new_node);
        servo1 = new Servo(new_node, "Servo1");
        servo1->SetChannel(1, this);
    }
    if (servo2 == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "Servo2");
        node->AddChild(new_node);
        servo2 = new Servo(new_node, "Servo2");
        servo2->SetChannel(3, this);
    }
    if (servo3 == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "Servo3");
        node->AddChild(new_node);
        servo3 = new Servo(new_node, "Servo3");
        servo3->SetChannel(5, this);
    }
}

DmxServo3Axis::~DmxServo3Axis()
{
    //dtor
}

static wxPGChoices MESH_LINKS;
static wxPGChoices MOTION2_LINKS;
static wxPGChoices MOTION3_LINKS;

void DmxServo3Axis::AddTypeProperties(wxPropertyGridInterface* grid) {
    DmxModel::AddTypeProperties(grid);

    servo1->AddTypeProperties(grid);
    servo2->AddTypeProperties(grid);
    servo3->AddTypeProperties(grid);
    static_mesh->AddTypeProperties(grid);
    motion_mesh1->AddTypeProperties(grid);
    motion_mesh2->AddTypeProperties(grid);
    motion_mesh3->AddTypeProperties(grid);

    grid->Append(new wxPropertyCategory("Motion Linkage", "MotionProperties"));

    if (MOTION2_LINKS.GetCount() == 0) {
        MOTION2_LINKS.Add("Mesh 1");
        MOTION2_LINKS.Add("Mesh 2");
    }

    if (MOTION3_LINKS.GetCount() == 0) {
        MOTION3_LINKS.Add("Mesh 1");
        MOTION3_LINKS.Add("Mesh 2");
        MOTION3_LINKS.Add("Mesh 3");
    }

    if (MESH_LINKS.GetCount() == 0) {
        MESH_LINKS.Add("Mesh 1");
        MESH_LINKS.Add("Mesh 2");
        MESH_LINKS.Add("Not Linked");
    }

    wxPGProperty* p = grid->Append(new wxEnumProperty("Servo1 Linkage", "Servo1Linkage", MOTION2_LINKS, 0));
    p->Enable(false);
    grid->Append(new wxEnumProperty("Servo2 Linkage", "Servo2Linkage", MOTION2_LINKS, servo2_motion_link_val));
    grid->Append(new wxEnumProperty("Servo3 Linkage", "Servo3Linkage", MOTION3_LINKS, servo3_motion_link_val));
    grid->Append(new wxEnumProperty("Mesh2 Linkage", "Mesh2Linkage", MESH_LINKS, mesh2_motion_link_val));
    grid->Append(new wxEnumProperty("Mesh3 Linkage", "Mesh3Linkage", MESH_LINKS, mesh3_motion_link_val));

    grid->Collapse("MotionProperties");

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

int DmxServo3Axis::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {

    if (servo1->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }
    if (servo2->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }
    if (servo3->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }
    if (static_mesh->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }
    if (motion_mesh1->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }
    if (motion_mesh2->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }
    if (motion_mesh3->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    std::string name = event.GetPropertyName().ToStdString();

    if ("Servo2Linkage" == name) {
        ModelXml->DeleteAttribute("Servo2Linkage");
        servo2_motion_link_val = event.GetPropertyValue().GetLong();
        if (servo2_motion_link_val == MOTION_LINK_MESH1) {
            servo2_motion_link = "Mesh 1";
        }
        else if (servo2_motion_link_val == MOTION_LINK_MESH2) {
            servo2_motion_link = "Mesh 2";
        }
        ModelXml->AddAttribute("Servo2Linkage", servo2_motion_link);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo3Axis::OnPropertyGridChange::Servo2Linkage");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo3Axis::OnPropertyGridChange::Servo2Linkage");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxServo3Axis::OnPropertyGridChange::Servo2Linkage");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "DmxServo3Axis::OnPropertyGridChange::Servo2Linkage");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo3Axis::OnPropertyGridChange::Servo2Linkage");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DmxServo3Axis::OnPropertyGridChange::Servo2Linkage");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo3Axis::OnPropertyGridChange::Servo2Linkage");
        return 0;
    }
    if ("Servo3Linkage" == name) {
        ModelXml->DeleteAttribute("Servo3Linkage");
        servo3_motion_link_val = event.GetPropertyValue().GetLong();
        if (servo3_motion_link_val == MOTION_LINK_MESH1) {
            servo3_motion_link = "Mesh 1";
        }
        else if (servo3_motion_link_val == MOTION_LINK_MESH2) {
            servo3_motion_link = "Mesh 2";
        }
        else if (servo3_motion_link_val == MOTION_LINK_MESH3) {
            servo3_motion_link = "Mesh 3";
        }
        ModelXml->AddAttribute("Servo3Linkage", servo3_motion_link);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo3Axis::OnPropertyGridChange::Servo3Linkage");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo3Axis::OnPropertyGridChange::Servo3Linkage");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxServo3Axis::OnPropertyGridChange::Servo3Linkage");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "DmxServo3Axis::OnPropertyGridChange::Servo3Linkage");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo3Axis::OnPropertyGridChange::Servo3Linkage");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DmxServo3Axis::OnPropertyGridChange::Servo3Linkage");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo3Axis::OnPropertyGridChange::Servo3Linkage");
        return 0;
    }
    if ("Mesh2Linkage" == name) {
        ModelXml->DeleteAttribute("Mesh2Linkage");
        mesh2_motion_link_val = event.GetPropertyValue().GetLong();
        if (mesh2_motion_link_val == MESH_NOT_LINKED) {
            mesh2_motion_link = "Not Linked";
        }
        else if (mesh2_motion_link_val == MESH_LINK_MESH1) {
            mesh2_motion_link = "Mesh 1";
        }
        else if (mesh2_motion_link_val == MESH_LINK_MESH2) {
            mesh2_motion_link = "Mesh 2";
        }
        ModelXml->AddAttribute("Mesh2Linkage", mesh2_motion_link);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo3Axis::OnPropertyGridChange::Mesh2Linkage");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo3Axis::OnPropertyGridChange::Mesh2Linkage");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxServo3Axis::OnPropertyGridChange::Mesh2Linkage");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "DmxServo3Axis::OnPropertyGridChange::Mesh2Linkage");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo3Axis::OnPropertyGridChange::Mesh2Linkage");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DmxServo3Axis::OnPropertyGridChange::Mesh2Linkage");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo3Axis::OnPropertyGridChange::Mesh2Linkage");
        return 0;
    }
    if ("Mesh3Linkage" == name) {
        ModelXml->DeleteAttribute("Mesh3Linkage");
        mesh3_motion_link_val = event.GetPropertyValue().GetLong();
        if (mesh3_motion_link_val == MESH_NOT_LINKED) {
            mesh3_motion_link = "Not Linked";
        }
        else if (mesh3_motion_link_val == MESH_LINK_MESH1) {
            mesh3_motion_link = "Mesh 1";
        }
        else if (mesh3_motion_link_val == MESH_LINK_MESH2) {
            mesh3_motion_link = "Mesh 2";
        }
        ModelXml->AddAttribute("Mesh3Linkage", mesh3_motion_link);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo3Axis::OnPropertyGridChange::Mesh3Linkage");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo3Axis::OnPropertyGridChange::Mesh3Linkage");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxServo3Axis::OnPropertyGridChange::Mesh3Linkage");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "DmxServo3Axis::OnPropertyGridChange::Mesh3Linkage");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo3Axis::OnPropertyGridChange::Mesh3Linkage");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DmxServo3Axis::OnPropertyGridChange::Mesh3Linkage");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo3Axis::OnPropertyGridChange::Mesh3Linkage");
        return 0;
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxServo3Axis::InitModel() {
    DmxModel::InitModel();
    DisplayAs = "DmxServo3Axis";

    servo1->Init(this);
    servo2->Init(this);
    servo3->Init(this);
    static_mesh->Init(this, true, !motion_mesh1->GetExists());
    motion_mesh1->Init(this, !static_mesh->GetExists(), !static_mesh->GetExists());
    motion_mesh2->Init(this, !static_mesh->GetExists() && !motion_mesh1->GetExists(), !static_mesh->GetExists() && !motion_mesh1->GetExists());
    motion_mesh3->Init(this, !static_mesh->GetExists() && !motion_mesh1->GetExists() && !motion_mesh2->GetExists(), !static_mesh->GetExists() && !motion_mesh1->GetExists() && !motion_mesh2->GetExists());

    servo2_motion_link = ModelXml->GetAttribute("Servo2Linkage", "Mesh 2");
    servo2_motion_link_val = MOTION_LINK_MESH2;
    if (servo2_motion_link == "Mesh 1") {
        servo2_motion_link_val = MOTION_LINK_MESH1;
    }

    servo3_motion_link = ModelXml->GetAttribute("Servo3Linkage", "Mesh 3");
    servo3_motion_link_val = MOTION_LINK_MESH3;
    if (servo3_motion_link == "Mesh 1") {
        servo3_motion_link_val = MOTION_LINK_MESH1;
    }
    else if (servo3_motion_link == "Mesh 2") {
        servo3_motion_link_val = MOTION_LINK_MESH2;
    }

    mesh2_motion_link = ModelXml->GetAttribute("Mesh2Linkage", "Not Linked");
    mesh2_motion_link_val = MESH_NOT_LINKED;
    if (mesh2_motion_link == "Mesh 1") {
        mesh2_motion_link_val = MESH_LINK_MESH1;
    }
    else if (mesh2_motion_link == "Mesh 2") {
        mesh2_motion_link_val = MESH_LINK_MESH2;
    }

    mesh3_motion_link = ModelXml->GetAttribute("Mesh3Linkage", "Not Linked");
    mesh3_motion_link_val = MESH_NOT_LINKED;
    if (mesh3_motion_link == "Mesh 1") {
        mesh3_motion_link_val = MESH_LINK_MESH1;
    }
    else if (mesh3_motion_link == "Mesh 2") {
        mesh3_motion_link_val = MESH_LINK_MESH2;
    }

    SetNodeNames("Axis1,-Axis1,Axis2,-Axis2,Axis3,-Axis3");
}

void DmxServo3Axis::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator& va, const xlColor* c, float& sx, float& sy, float& sz, bool active)
{
    if (servo1->GetChannel() > Nodes.size() ||
        servo2->GetChannel() > Nodes.size() || 
        servo3->GetChannel() > Nodes.size() )
    {
        // crash protection
        return;
    }

    float servo1_pos = 0.0;
    float servo2_pos = 0.0;
    float servo3_pos = 0.0;
    glm::mat4 Identity = glm::mat4(1.0f);
    glm::mat4 motion_matrix = Identity;
    glm::mat4 motion1_matrix = Identity;
    glm::mat4 motion2_matrix = Identity;
    glm::mat4 motion3_matrix = Identity;
    glm::mat4 scalingMatrix = glm::scale(Identity, GetModelScreenLocation().GetScaleMatrix());
    glm::mat4 translateMatrix = glm::translate(Identity, GetModelScreenLocation().GetWorldPosition());
    glm::quat rotateQuat = GetModelScreenLocation().GetRotationQuat();
    glm::mat4 base_matrix = translateMatrix * glm::toMat4(rotateQuat) * scalingMatrix;

    static_mesh->Draw(this, preview, va, base_matrix, motion_matrix);

    if (servo1->GetChannel() > 0 && active) {
        servo1_pos = servo1->GetPosition(GetChannelValue(servo1->GetChannel() - 1));
    }
    if (servo2->GetChannel() > 0 && active) {
        servo2_pos = servo2->GetPosition(GetChannelValue(servo2->GetChannel() - 1));
    }
    if (servo3->GetChannel() > 0 && active) {
        servo3_pos = servo3->GetPosition(GetChannelValue(servo3->GetChannel() - 1));
    }

    servo1->FillMotionMatrix(servo1_pos, motion1_matrix);
    servo2->FillMotionMatrix(servo2_pos, motion2_matrix);
    servo3->FillMotionMatrix(servo3_pos, motion3_matrix);

    // Draw Mesh 1
    if (motion_mesh1->GetExists()) {
        motion_matrix = motion1_matrix;
        if (servo2_motion_link_val == MOTION_LINK_MESH1) {
            motion_matrix = motion_matrix * motion2_matrix;
        }
        if (servo3_motion_link_val == MOTION_LINK_MESH1) {
            motion_matrix = motion_matrix * motion3_matrix;
        }
        motion_mesh1->Draw(this, preview, va, base_matrix, motion_matrix);
    }

    // Draw Mesh 2
    if (motion_mesh2->GetExists()) {
        motion_matrix = Identity;
        if (mesh2_motion_link_val == MOTION_LINK_MESH1) {
            motion_matrix = motion1_matrix;
        }
        if (servo2_motion_link_val == MOTION_LINK_MESH2) {
            motion_matrix = motion_matrix * motion2_matrix;
        }
        if (servo3_motion_link_val == MOTION_LINK_MESH2) {
            motion_matrix =  motion_matrix * motion3_matrix;
        }
        motion_mesh2->Draw(this, preview, va, base_matrix, motion_matrix);
    }

    // Draw Mesh 3
    if (motion_mesh3->GetExists()) {
        motion_matrix = Identity;
        if (mesh3_motion_link_val == MOTION_LINK_MESH1) {
            motion_matrix = motion1_matrix;
        }
        else if (mesh3_motion_link_val == MOTION_LINK_MESH2) {
            if (mesh2_motion_link_val == MOTION_LINK_MESH1) {
                motion_matrix = motion1_matrix;
            }
            motion_matrix = motion_matrix * motion2_matrix;
        }
        if (servo3_motion_link_val == MOTION_LINK_MESH3) {
            motion_matrix = motion_matrix * motion3_matrix;
        }
        motion_mesh3->Draw(this, preview, va, base_matrix, motion_matrix);
    }
}

void DmxServo3Axis::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator& va, const xlColor* c, float& sx, float& sy, bool active)
{
}

void DmxServo3Axis::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dmxservo3axis \n");

    ExportBaseParameters(f);

    wxString ml2 = ModelXml->GetAttribute("Mesh2Linkage");
    wxString ml3 = ModelXml->GetAttribute("Mesh3Linkage");
    wxString sl1 = ModelXml->GetAttribute("Servo1Linkage");
    wxString sl2 = ModelXml->GetAttribute("Servo2Linkage");
    wxString sl3 = ModelXml->GetAttribute("Servo3Linkage");
    f.Write(wxString::Format("Mesh2Linkage=\"%s\" ", ml2));
    f.Write(wxString::Format("Mesh3Linkage=\"%s\" ", ml3));
    f.Write(wxString::Format("Servo1Linkage=\"%s\" ", sl1));
    f.Write(wxString::Format("Servo2Linkage=\"%s\" ", sl2));
    f.Write(wxString::Format("Servo3Linkage=\"%s\" ", sl3));

    f.Write(" >\n");

    
    wxString show_dir = GetModelManager().GetXLightsFrame()->GetShowDirectory();

    static_mesh->Serialise(ModelXml, f, show_dir);
    motion_mesh1->Serialise(ModelXml, f, show_dir);
    motion_mesh2->Serialise(ModelXml, f, show_dir);
    motion_mesh3->Serialise(ModelXml, f, show_dir);
    servo1->Serialise(ModelXml, f, show_dir);
    servo2->Serialise(ModelXml, f, show_dir);
    servo3->Serialise(ModelXml, f, show_dir);

    wxString submodel = SerialiseSubmodel();
    if (submodel != "")
    {
        f.Write(submodel);
    }
    wxString state = SerialiseState();
    if (state != "")
    {
        f.Write(state);
    }

    f.Write("</dmxservo3axis>");
    f.Close();
}

void DmxServo3Axis::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    // We have already loaded gdtf properties
    if (EndsWith(filename, "gdtf")) return;

    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "dmxservo3axis")
        {
            ImportBaseParameters(root);

            wxString name = root->GetAttribute("name");
            wxString v = root->GetAttribute("SourceVersion");
 
            wxString ml2 = root->GetAttribute("Mesh2Linkage");
            wxString ml3 = root->GetAttribute("Mesh3Linkage");
            wxString sl1 = root->GetAttribute("Servo1Linkage");
            wxString sl2 = root->GetAttribute("Servo2Linkage");
            wxString sl3 = root->GetAttribute("Servo3Linkage");
            SetProperty("Mesh2Linkage", ml2);
            SetProperty("Mesh3Linkage", ml3);
            SetProperty("Servo1Linkage", sl1);
            SetProperty("Servo2Linkage", sl2);
            SetProperty("Servo3Linkage", sl3);

            // Add any model version conversion logic here
            // Source version will be the program version that created the custom model

            wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
            GetModelScreenLocation().Write(ModelXml);
            SetProperty("name", newname, true);

            wxString show_dir = GetModelManager().GetXLightsFrame()->GetShowDirectory();
            static_mesh->Serialise(root, ModelXml, show_dir);
            motion_mesh1->Serialise(root, ModelXml, show_dir);
            motion_mesh2->Serialise(root, ModelXml, show_dir);
            motion_mesh3->Serialise(root, ModelXml, show_dir);
            servo1->Serialise(root, ModelXml, show_dir);
            servo2->Serialise(root, ModelXml, show_dir);
            servo3->Serialise(root, ModelXml, show_dir);

            for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
            {
                if (n->GetName() == "subModel")
                {
                    AddSubmodel(n);
                }
                else if (n->GetName() == "stateInfo")
                {
                    AddState(n);
                }
            }

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo3Axis::ImportXlightsModel");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo3Axis::ImportXlightsModel");
        }
        else
        {
            DisplayError("Failure loading DmxServo3Axis model file.");
        }
    }
    else
    {
        DisplayError("Failure loading DmxServo3Axis model file.");
    }
}
