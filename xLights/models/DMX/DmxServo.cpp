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

    SetNodeNames("Axis1,-Axis1");
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
    if (!f.Create(filename, true) || !f.IsOpened()) DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dmxservo \n");

    ExportBaseParameters(f);

    f.Write(" >\n");

    wxString show_dir = GetModelManager().GetXLightsFrame()->GetShowDirectory();
    static_image->Serialise(ModelXml, f, show_dir);
    motion_image->Serialise(ModelXml, f, show_dir);
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
    f.Write("</dmxservo>");
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

        if (root->GetName() == "dmxservo")
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
            static_image->Serialise(root, ModelXml, show_dir);
            motion_image->Serialise(root, ModelXml, show_dir);
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
