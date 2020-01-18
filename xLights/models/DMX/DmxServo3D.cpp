#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxServo3D.h"
#include "Servo.h"
#include "../../ModelPreview.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"

DmxServo3d::DmxServo3d(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
    : DmxModel(node, manager, zeroBased), brightness(100.0f), static_mesh(nullptr), motion_mesh(nullptr), servo1(nullptr)
{
    wxXmlNode* n = node->GetChildren();
    while (n != nullptr) {
        if ("StaticMesh" == n->GetName()) {
            static_mesh = new Mesh(n, "StaticMesh");
        }
        else if ("MotionMesh" == n->GetName()) {
            motion_mesh = new Mesh(n, "MotionMesh");
        }
        else if ("Servo1" == n->GetName()) {
            servo1 = new Servo(n, "Servo1");
        }
        n = n->GetNext();
    }
    if (static_mesh == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "StaticMesh");
        node->AddChild(new_node);
        static_mesh = new Mesh(new_node, "StaticMesh");
    }
    if (motion_mesh == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "MotionMesh");
        node->AddChild(new_node);
        motion_mesh = new Mesh(new_node, "MotionMesh");
    }
    if (servo1 == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "Servo1");
        node->AddChild(new_node);
        servo1 = new Servo(new_node, "Servo1");
        servo1->SetChannel(1, this);
    }
}

DmxServo3d::~DmxServo3d()
{
    //dtor
}

void DmxServo3d::AddTypeProperties(wxPropertyGridInterface* grid) {
    DmxModel::AddTypeProperties(grid);

    servo1->AddTypeProperties(grid);
    static_mesh->AddTypeProperties(grid);
    motion_mesh->AddTypeProperties(grid);

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

int DmxServo3d::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {

    if (servo1->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }
    if (static_mesh->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }
    if (motion_mesh->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxServo3d::InitModel() {
    DmxModel::InitModel();
    DisplayAs = "DmxServo3d";

    servo1->Init(this);
    static_mesh->Init(this, true, !motion_mesh->GetExists());
    motion_mesh->Init(this, !static_mesh->GetExists(), !static_mesh->GetExists() );

    SetNodeNames("Axis1,-Axis1");
}

void DmxServo3d::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator& va, const xlColor* c, float& sx, float& sy, float& sz, bool active)
{
    if (servo1->GetChannel() > Nodes.size())
    {
        // crash protection
        return;
    }

    float servo_pos = 0.0;
    glm::mat4 Identity = glm::mat4(1.0f);
    glm::mat4 motion_matrix = Identity;
    glm::mat4 scalingMatrix = glm::scale(Identity, GetModelScreenLocation().GetScaleMatrix());
    glm::mat4 translateMatrix = glm::translate(Identity, GetModelScreenLocation().GetWorldPosition());
    glm::quat rotateQuat = GetModelScreenLocation().GetRotationQuat();
    glm::mat4 base_matrix = translateMatrix * glm::toMat4(rotateQuat) * scalingMatrix;

    static_mesh->Draw(this, preview, va, base_matrix, motion_matrix);

    if (servo1->GetChannel() > 0 && active) {
        servo_pos = servo1->GetPosition(GetChannelValue(servo1->GetChannel() - 1));
    }

    servo1->FillMotionMatrix(servo_pos, motion_matrix);
    motion_mesh->Draw(this, preview, va, base_matrix, motion_matrix, servo1->GetPivotOffsetX(), servo1->GetPivotOffsetY(), servo1->IsRotate(), !active);
}

void DmxServo3d::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator& va, const xlColor* c, float& sx, float& sy, bool active)
{
    float x1 = -0.5f;
    float x2 = -0.5f;
    float x3 = 0.5f;
    float x4 = 0.5f;
    float y1 = -0.5f;
    float y2 = 0.5f;
    float y3 = 0.5f;
    float y4 = -0.5f;
    float z1 = 0.0f;
    float z2 = 0.0f;
    float z3 = 0.0f;
    float z4 = 0.0f;

    GetBaseObjectScreenLocation().TranslatePoint(x1, y1, z1);
    GetBaseObjectScreenLocation().TranslatePoint(x2, y2, z2);
    GetBaseObjectScreenLocation().TranslatePoint(x3, y3, z3);
    GetBaseObjectScreenLocation().TranslatePoint(x4, y4, z4);

    va.AddVertex(x1, y1, z1, *wxRED);
    va.AddVertex(x2, y2, z2, *wxRED);
    va.AddVertex(x2, y2, z2, *wxRED);
    va.AddVertex(x3, y3, z3, *wxRED);
    va.AddVertex(x3, y3, z3, *wxRED);
    va.AddVertex(x4, y4, z4, *wxRED);
    va.AddVertex(x4, y4, z4, *wxRED);
    va.AddVertex(x1, y1, z1, *wxRED);
    va.AddVertex(x1, y1, z1, *wxRED);
    va.AddVertex(x3, y3, z3, *wxRED);
    va.AddVertex(x2, y2, z2, *wxRED);
    va.AddVertex(x4, y4, z4, *wxRED);
    va.Finish(GL_LINES, GL_LINE_SMOOTH, 5.0f);
}

void DmxServo3d::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dmxservo3d \n");

    ExportBaseParameters(f);

    f.Write(" >\n");

    wxString show_dir = GetModelManager().GetXLightsFrame()->GetShowDirectory();

    static_mesh->Serialise(ModelXml, f, show_dir);
    motion_mesh->Serialise(ModelXml, f, show_dir);
    servo1->Serialise(ModelXml, f, show_dir);

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
    f.Write("</dmxservo3d>");
    f.Close();
}

void DmxServo3d::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    // We have already loaded gdtf properties
    if (EndsWith(filename, "gdtf")) return;

    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "dmxservo3d")
        {
            ImportBaseParameters(root);

            wxString name = root->GetAttribute("name");
            wxString v = root->GetAttribute("SourceVersion");

            // Add any model version conversion logic here
            // Source version will be the program version that created the custom model

            wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
            GetModelScreenLocation().Write(ModelXml);
            SetProperty("name", newname, true);

            wxString show_dir = GetModelManager().GetXLightsFrame()->GetShowDirectory();
            static_mesh->Serialise(root, ModelXml, show_dir);
            motion_mesh->Serialise(root, ModelXml, show_dir);
            servo1->Serialise(root, ModelXml, show_dir);

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

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo3d::ImportXlightsModel");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo3d::ImportXlightsModel");
        }
        else
        {
            DisplayError("Failure loading DmxServo3d model file.");
        }
    }
    else
    {
        DisplayError("Failure loading DmxServo3d model file.");
    }
}
