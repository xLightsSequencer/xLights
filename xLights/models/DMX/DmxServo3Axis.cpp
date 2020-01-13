#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxServo3Axis.h"
#include "../../ModelPreview.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"

DmxServo3Axis::DmxServo3Axis(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
    : DmxServo3d(node, manager, zeroBased), static_mesh(nullptr), motion_mesh(nullptr)
{
    wxXmlNode* n = node->GetChildren();
    while (n != nullptr) {
        if ("staticMesh" == n->GetName()) {
            static_mesh = new Mesh(n, "StaticMesh");
        }
        else if ("motionMesh" == n->GetName()) {
            motion_mesh = new Mesh(n, "MotionMesh");
        }
        n = n->GetNext();
    }
    if (static_mesh == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "staticMesh");
        node->AddChild(new_node);
        static_mesh = new Mesh(new_node, "StaticMesh");
    }
    if (motion_mesh == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "motionMesh");
        node->AddChild(new_node);
        motion_mesh = new Mesh(new_node, "MotionMesh");
    }
}

DmxServo3Axis::~DmxServo3Axis()
{
    //dtor
}

static wxPGChoices SERVO_STYLES;

enum SERVO_STYLE {
    SERVO_STYLE_TRANSLATEX,
    SERVO_STYLE_TRANSLATEY,
    SERVO_STYLE_TRANSLATEZ,
    SERVO_STYLE_ROTATEX,
    SERVO_STYLE_ROTATEY,
    SERVO_STYLE_ROTATEZ
};

void DmxServo3Axis::AddTypeProperties(wxPropertyGridInterface *grid) {

    DmxServo3d::AddTypePropertiesSpecial(grid, false);

    if (static_mesh != nullptr) {
        static_mesh->AddTypeProperties(grid);
    }
    if (motion_mesh != nullptr) {
        motion_mesh->AddTypeProperties(grid);
    }

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

int DmxServo3Axis::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {

    if (static_mesh != nullptr) {
        static_mesh->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked());
    }
    if (motion_mesh != nullptr) {
        motion_mesh->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked());
    }

    return DmxServo3d::OnPropertyGridChange(grid, event);
}

void DmxServo3Axis::InitModel() {
    DmxServo3d::InitModel();
    DisplayAs = "DmxServo3d";

    if (static_mesh != nullptr) {
        static_mesh->Init(this, true, !motion_mesh->GetExists());
    }

    if (motion_mesh != nullptr) {
        motion_mesh->Init(this, !static_mesh->GetExists(), !static_mesh->GetExists() );
    }
}

void DmxServo3Axis::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, const xlColor *c, float &sx, float &sy, float &sz, bool active)
{
    int channel_value;
    float servo_pos = 0.0;
    glm::mat4 Identity = glm::mat4(1.0f);
    glm::mat4 motion_matrix = Identity;
    glm::mat4 scalingMatrix = glm::scale(Identity, GetModelScreenLocation().GetScaleMatrix());
    glm::mat4 translateMatrix = glm::translate(Identity, GetModelScreenLocation().GetWorldPosition());
    glm::quat rotateQuat = GetModelScreenLocation().GetRotationQuat();
    glm::mat4 base_matrix = translateMatrix * glm::toMat4(rotateQuat) * scalingMatrix;

    if (static_mesh != nullptr) {
        static_mesh->Draw(this, preview, va, base_matrix, motion_matrix);
    }

    if (servo_channel > 0 && active) {
        channel_value = GetChannelValue(servo_channel - 1);
        switch (servo_style_val) {
        case SERVO_STYLE_TRANSLATEX:
        case SERVO_STYLE_TRANSLATEY:
        case SERVO_STYLE_TRANSLATEZ:
            servo_pos = (1.0 - ((channel_value - min_limit) / (float)(max_limit - min_limit))) * -range_of_motion + range_of_motion;
            break;
        case SERVO_STYLE_ROTATEX:
        case SERVO_STYLE_ROTATEY:
        case SERVO_STYLE_ROTATEZ:
            servo_pos = (1.0 - ((channel_value - min_limit) / (float)(max_limit - min_limit))) * -range_of_motion + range_of_motion / 2.0;
            break;
        }
    }
    else {
        servo_pos = 0.0f;
    }

    switch (servo_style_val) {
    case SERVO_STYLE_TRANSLATEX:
        motion_matrix = glm::translate(Identity, glm::vec3(servo_pos, 0.0f, 0.0f));
        break;
    case SERVO_STYLE_TRANSLATEY:
        motion_matrix = glm::translate(Identity, glm::vec3(0.0f, servo_pos, 0.0f));
        break;
    case SERVO_STYLE_TRANSLATEZ:
        motion_matrix = glm::translate(Identity, glm::vec3(0.0f, 0.0f, servo_pos));
        break;
    case SERVO_STYLE_ROTATEX:
        motion_matrix = glm::rotate(Identity, glm::radians(servo_pos), glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    case SERVO_STYLE_ROTATEY:
        motion_matrix = glm::rotate(Identity, glm::radians(servo_pos), glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    case SERVO_STYLE_ROTATEZ:
        motion_matrix = glm::rotate(Identity, glm::radians(servo_pos), glm::vec3(0.0f, 0.0f, 1.0f));
        break;
    }

    if (motion_mesh != nullptr) {
        motion_mesh->Draw(this, preview, va, base_matrix, motion_matrix);
    }
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
