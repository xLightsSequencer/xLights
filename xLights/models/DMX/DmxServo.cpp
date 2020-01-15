#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxServo.h"
#include "DmxImage.h"
#include "Servo.h"

#include "../../ModelPreview.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"
#include <log4cpp/Category.hh>

DmxServo::DmxServo(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
    : DmxModel(node, manager, zeroBased), transparency(0), brightness(100),
      static_image(nullptr), motion_image(nullptr), servo1(nullptr)
{
    wxXmlNode* n = node->GetChildren();
    while (n != nullptr) {
        if ("StaticImage" == n->GetName()) {
            static_image = new DmxImage(n, "StaticImage");
        }
        else if ("MotionImage" == n->GetName()) {
            motion_image = new DmxImage(n, "MotionImage");
        }
        else if ("Servo1" == n->GetName()) {
            servo1 = new Servo(n, "Servo1");
        }
        n = n->GetNext();
    }
    if (static_image == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "StaticImage");
        node->AddChild(new_node);
        static_image = new DmxImage(new_node, "StaticImage");
    }
    if (motion_image == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "MotionImage");
        node->AddChild(new_node);
        motion_image = new DmxImage(new_node, "MotionImage");
        motion_image->SetOffsetZ(0.01f, this);  // offset on creation so its not hidden
    }
    if (servo1 == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "Servo1");
        node->AddChild(new_node);
        servo1 = new Servo(new_node, "Servo1");
        servo1->SetChannel(1, this);
    }
}

DmxServo::~DmxServo()
{
}

void DmxServo::AddTypeProperties(wxPropertyGridInterface* grid) {
    DmxModel::AddTypeProperties(grid);

    wxPGProperty* p = grid->Append(new wxUIntProperty("Brightness", "Brightness", (int)brightness));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Transparency", "Transparency", transparency));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    servo1->AddTypeProperties(grid);
    static_image->AddTypeProperties(grid);
    motion_image->AddTypeProperties(grid);

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

int DmxServo::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();

    if ("Transparency" == name) {
        transparency = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Transparency");
        ModelXml->AddAttribute("Transparency", wxString::Format("%d", transparency));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::Transparency");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::Transparency");
        return 0;
    }
    else if ("Brightness" == name) {
        brightness = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Brightness");
        ModelXml->AddAttribute("Brightness", wxString::Format("%d", (int)brightness));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::Brightness");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::Transparency");
        return 0;
    }

    if (servo1->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }
    if (static_image->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }
    if (motion_image->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxServo::InitModel() {
    DmxModel::InitModel();
    DisplayAs = "DmxServo";

    transparency = wxAtoi(ModelXml->GetAttribute("Transparency", "0"));
    brightness = wxAtoi(ModelXml->GetAttribute("Brightness", "100"));

    servo1->Init(this);
    static_image->Init(this, !motion_image->GetExists());
    motion_image->Init(this, !static_image->GetExists());

    SetNodeNames(wxString::Format("%s", "Axis1,-Axis1"));
}

void DmxServo::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator& va, const xlColor* c, float& sx, float& sy, bool active)
{
    if (servo1->GetChannel() > Nodes.size())
    {
        // crash protection
        return;
    }

    // this if statement scales the motion image so that is has the same size ratio as the original images
    if (motion_image->ImageSelected() && motion_image->GetExists() && static_image->GetExists()) {
        if (static_image->GetWidth() != 0) {
            float new_scale = (float)motion_image->GetWidth() / (float)static_image->GetWidth();
            motion_image->SetScaleX(new_scale, this);
            motion_image->SetScaleY(new_scale, this);
            motion_image->ClearImageSelected();
        }
    }

    float servo_pos = 0.0;
    glm::mat4 Identity = glm::mat4(1.0f);
    glm::mat4 motion_matrix = Identity;
    glm::mat4 scalingMatrix = glm::scale(Identity, GetModelScreenLocation().GetScaleMatrix());
    glm::mat4 translateMatrix = glm::translate(Identity, GetModelScreenLocation().GetWorldPosition());
    glm::quat rotateQuat = GetModelScreenLocation().GetRotationQuat();
    glm::mat4 base_matrix = translateMatrix * glm::toMat4(rotateQuat) * scalingMatrix;

    static_image->Draw(this, preview, va, base_matrix, motion_matrix, transparency, brightness, !motion_image->GetExists(), 0, 0, false, false);

    if (servo1->GetChannel() > 0 && active) {
        servo_pos = servo1->GetPosition(GetChannelValue(servo1->GetChannel() - 1));
        if (servo1->IsTranslate()) {
            glm::vec3 scale = GetBaseObjectScreenLocation().GetScaleMatrix();
            servo_pos /= scale.x;
        }
    }

    servo1->FillMotionMatrix(servo_pos, motion_matrix);
    motion_image->Draw(this, preview, va, base_matrix, motion_matrix, transparency, brightness, !static_image->GetExists(),
                       servo1->GetPivotOffsetX(), servo1->GetPivotOffsetY(), servo1->IsRotate(), !active);
}

void DmxServo::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator& va, const xlColor* c, float& sx, float& sy, float& sz, bool active)
{
    if (servo1->GetChannel() > Nodes.size())
    {
        // crash protection
        return;
    }

    // this if statement scales the motion image so that is has the same size ratio as the original images
    if (motion_image->ImageSelected() && motion_image->GetExists() && static_image->GetExists()) {
        if (static_image->GetWidth() != 0) {
            float new_scale = (float)motion_image->GetWidth() / (float)static_image->GetWidth();
            motion_image->SetScaleX(new_scale, this);
            motion_image->SetScaleY(new_scale, this);
            motion_image->ClearImageSelected();
        }
    }

    float servo_pos = 0.0;
    glm::mat4 Identity = glm::mat4(1.0f);
    glm::mat4 motion_matrix = Identity;
    glm::mat4 scalingMatrix = glm::scale(Identity, GetModelScreenLocation().GetScaleMatrix());
    glm::mat4 translateMatrix = glm::translate(Identity, GetModelScreenLocation().GetWorldPosition());
    glm::quat rotateQuat = GetModelScreenLocation().GetRotationQuat();
    glm::mat4 base_matrix = translateMatrix * glm::toMat4(rotateQuat) * scalingMatrix;

    static_image->Draw(this, preview, va, base_matrix, motion_matrix, transparency, brightness, !motion_image->GetExists(), 0, 0, false, false);

    if (servo1->GetChannel() > 0 && active) {
        servo_pos = servo1->GetPosition(GetChannelValue(servo1->GetChannel() - 1));
        if (servo1->IsTranslate()) {
            glm::vec3 scale = GetBaseObjectScreenLocation().GetScaleMatrix();
            servo_pos /= scale.x;
        }
    }

    servo1->FillMotionMatrix(servo_pos, motion_matrix);
    motion_image->Draw(this, preview, va, base_matrix, motion_matrix, transparency, brightness, !static_image->GetExists(),
        servo1->GetPivotOffsetX(), servo1->GetPivotOffsetY(), servo1->IsRotate(), !active);
}

void DmxServo::ExportXlightsModel()
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

void DmxServo::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
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
