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

DmxServo3Axis::DmxServo3Axis(wxXmlNode* node, const ModelManager& manager, bool zeroBased)
    : DmxModel(node, manager, zeroBased), brightness(100.0f), static_mesh(nullptr),
    motion_mesh1(nullptr), motion_mesh2(nullptr), motion_mesh3(nullptr),
    servo1(nullptr), servo2(nullptr), servo3(nullptr)
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
            servo1 = new Servo(n, "Servo1", 1);
        }
        else if ("Servo2" == n->GetName()) {
            servo2 = new Servo(n, "Servo2", 1);
        }
        else if ("Servo3" == n->GetName()) {
            servo3 = new Servo(n, "Servo3", 1);
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
        servo1 = new Servo(new_node, "Servo1", 1);
    }
    if (servo2 == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "Servo2");
        node->AddChild(new_node);
        servo2 = new Servo(new_node, "Servo2", 3);
    }
    if (servo3 == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "Servo3");
        node->AddChild(new_node);
        servo3 = new Servo(new_node, "Servo3", 5);
    }
}

DmxServo3Axis::~DmxServo3Axis()
{
    //dtor
}

void DmxServo3Axis::AddTypeProperties(wxPropertyGridInterface* grid) {
    DmxModel::AddTypeProperties(grid);

    servo1->AddTypeProperties(grid);
    servo2->AddTypeProperties(grid);
    servo3->AddTypeProperties(grid);
    static_mesh->AddTypeProperties(grid);
    motion_mesh1->AddTypeProperties(grid);
    motion_mesh2->AddTypeProperties(grid);
    motion_mesh3->AddTypeProperties(grid);

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
    glm::mat4 Identity = glm::mat4(1.0f);
    glm::mat4 motion_matrix = Identity;
    glm::mat4 scalingMatrix = glm::scale(Identity, GetModelScreenLocation().GetScaleMatrix());
    glm::mat4 translateMatrix = glm::translate(Identity, GetModelScreenLocation().GetWorldPosition());
    glm::quat rotateQuat = GetModelScreenLocation().GetRotationQuat();
    glm::mat4 base_matrix = translateMatrix * glm::toMat4(rotateQuat) * scalingMatrix;

    static_mesh->Draw(this, preview, va, base_matrix, motion_matrix);

    if (servo1->GetChannel() > 0 && active) {
        servo1_pos = servo1->GetPosition(GetChannelValue(servo1->GetChannel() - 1));
    }

    servo1->FillMotionMatrix(servo1_pos, motion_matrix);
    motion_mesh1->Draw(this, preview, va, base_matrix, motion_matrix);
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
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
    wxString p1 = ModelXml->GetAttribute("parm1");
    wxString p2 = ModelXml->GetAttribute("parm2");
    wxString p3 = ModelXml->GetAttribute("parm3");
    wxString st = ModelXml->GetAttribute("StringType");
    wxString ps = ModelXml->GetAttribute("PixelSize");
    wxString t = ModelXml->GetAttribute("Transparency");
    wxString mb = ModelXml->GetAttribute("ModelBrightness");
    wxString a = ModelXml->GetAttribute("Antialias");
    wxString ss = ModelXml->GetAttribute("StartSide");
    wxString dir = ModelXml->GetAttribute("Dir");
    wxString sn = ModelXml->GetAttribute("StrandNames");
    wxString nn = ModelXml->GetAttribute("NodeNames");
    wxString da = ModelXml->GetAttribute("DisplayAs");

    wxString svc = ModelXml->GetAttribute("DmxServoChannel", "1");
    wxString sml = ModelXml->GetAttribute("DmxServoMinLimit", "0");
    wxString smxl = ModelXml->GetAttribute("DmxServoMaxLimit", "65535");
    wxString rom = ModelXml->GetAttribute("DmxServoRangeOfMotion", "100");
    wxString s = ModelXml->GetAttribute("ServoStyle");

    wxString v = xlights_version_string;

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dmxmodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("parm1=\"%s\" ", p1));
    f.Write(wxString::Format("parm2=\"%s\" ", p2));
    f.Write(wxString::Format("parm3=\"%s\" ", p3));
    f.Write(wxString::Format("DisplayAs=\"%s\" ", da));
    f.Write(wxString::Format("StringType=\"%s\" ", st));
    f.Write(wxString::Format("Transparency=\"%s\" ", t));
    f.Write(wxString::Format("PixelSize=\"%s\" ", ps));
    f.Write(wxString::Format("ModelBrightness=\"%s\" ", mb));
    f.Write(wxString::Format("Antialias=\"%s\" ", a));
    f.Write(wxString::Format("StartSide=\"%s\" ", ss));
    f.Write(wxString::Format("Dir=\"%s\" ", dir));
    f.Write(wxString::Format("StrandNames=\"%s\" ", sn));
    f.Write(wxString::Format("NodeNames=\"%s\" ", nn));
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));

    f.Write(wxString::Format("DmxServoChannel=\"%s\" ", svc));
    f.Write(wxString::Format("DmxServoMinLimit=\"%s\" ", sml));
    f.Write(wxString::Format("DmxServoMaxLimit=\"%s\" ", smxl));
    f.Write(wxString::Format("DmxServoRangeOfMotion=\"%s\" ", rom));
    f.Write(wxString::Format("ServoStyle=\"%s\" ", s));
    f.Write(" >\n");
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
    f.Write("</dmxmodel>");
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

        if (root->GetName() == "dmxmodel")
        {
            wxString name = root->GetAttribute("name");
            wxString p1 = root->GetAttribute("parm1");
            wxString p2 = root->GetAttribute("parm2");
            wxString p3 = root->GetAttribute("parm3");
            wxString st = root->GetAttribute("StringType");
            wxString ps = root->GetAttribute("PixelSize");
            wxString t = root->GetAttribute("Transparency");
            wxString mb = root->GetAttribute("ModelBrightness");
            wxString a = root->GetAttribute("Antialias");
            wxString ss = root->GetAttribute("StartSide");
            wxString dir = root->GetAttribute("Dir");
            wxString sn = root->GetAttribute("StrandNames");
            wxString nn = root->GetAttribute("NodeNames");
            wxString v = root->GetAttribute("SourceVersion");
            wxString da = root->GetAttribute("DisplayAs");

            wxString svc = root->GetAttribute("DmxServoChannel");
            wxString sml = root->GetAttribute("DmxServoMinLimit");
            wxString smxl = root->GetAttribute("DmxServoMaxLimit");
            wxString rom = root->GetAttribute("DmxServoRangeOfMotion");
            wxString s = root->GetAttribute("ServoStyle");

            // Add any model version conversion logic here
            // Source version will be the program version that created the custom model

            SetProperty("parm1", p1);
            SetProperty("parm2", p2);
            SetProperty("parm3", p3);
            SetProperty("StringType", st);
            SetProperty("PixelSize", ps);
            SetProperty("Transparency", t);
            SetProperty("ModelBrightness", mb);
            SetProperty("Antialias", a);
            SetProperty("StartSide", ss);
            SetProperty("Dir", dir);
            SetProperty("StrandNames", sn);
            SetProperty("NodeNames", nn);
            SetProperty("DisplayAs", da);

            SetProperty("DmxServoChannel", svc);
            SetProperty("DmxServoMinLimit", sml);
            SetProperty("DmxServoMaxLimit", smxl);
            SetProperty("DmxServoRangeOfMotion", rom);
            SetProperty("ServoStyle", s);

            wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
            GetModelScreenLocation().Write(ModelXml);
            SetProperty("name", newname, true);

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

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::ImportXlightsModel");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo::ImportXlightsModel");
        }
        else
        {
            DisplayError("Failure loading DmxServo model file.");
        }
    }
    else
    {
        DisplayError("Failure loading DmxServo model file.");
    }
}
