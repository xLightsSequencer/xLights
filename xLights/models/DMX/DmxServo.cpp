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

static const int SUPPORTED_SERVOS = 24;

DmxServo::DmxServo(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
    : DmxModel(node, manager, zeroBased), transparency(0), brightness(100),
      update_node_names(false), num_servos(1), _16bit(true)
{
    InitModel();
}

DmxServo::~DmxServo()
{
    Clear();
}

void DmxServo::Clear() {
    for (auto it = servos.begin(); it != servos.end(); ++it) {
        if (*it != nullptr) {
            delete* it;
        }
    }
    servos.clear();
    for (auto it = motion_images.begin(); it != motion_images.end(); ++it) {
        if (*it != nullptr) {
            delete* it;
        }
    }
    motion_images.clear();
    for (auto it = static_images.begin(); it != static_images.end(); ++it) {
        if (*it != nullptr) {
            delete* it;
        }
    }
    static_images.clear();
}

void DmxServo::AddTypeProperties(wxPropertyGridInterface* grid) {
    DmxModel::AddTypeProperties(grid);

    wxPGProperty* p = grid->Append(new wxUIntProperty("Num Servos", "NumServos", (int)num_servos));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", SUPPORTED_SERVOS);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("16 Bit", "Bits16", _16bit));
    p->SetAttribute("UseCheckbox", true);

    p = grid->Append(new wxUIntProperty("Brightness", "Brightness", (int)brightness));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Transparency", "Transparency", transparency));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    for (auto it = servos.begin(); it != servos.end(); ++it) {
        (*it)->AddTypeProperties(grid);
    }

    for (auto it = static_images.begin(); it != static_images.end(); ++it) {
        (*it)->AddTypeProperties(grid);
    }

    for (auto it = motion_images.begin(); it != motion_images.end(); ++it) {
        (*it)->AddTypeProperties(grid);
    }

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

int DmxServo::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();

    if ("NumServos" == name) {
        update_node_names = true;
        num_servos = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("NumServos");
        ModelXml->AddAttribute("NumServos", wxString::Format("%d", num_servos));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::NumServos");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo::OnPropertyGridChange::NumServos");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::NumServos");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo::OnPropertyGridChange::NumServos");
        return 0;
    }
    else if (event.GetPropertyName() == "Bits16") {
        ModelXml->DeleteAttribute("Bits16");
        if (event.GetValue().GetBool()) {
            _16bit = true;
            ModelXml->AddAttribute("Bits16", "1");
        }
        else {
            _16bit = false;
            ModelXml->AddAttribute("Bits16", "0");
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::NumServos");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo::OnPropertyGridChange::NumServos");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::Transparency");
        return 0;
    }
    if ("Transparency" == name) {
        transparency = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Transparency");
        ModelXml->AddAttribute("Transparency", wxString::Format("%d", transparency));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::Transparency");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::Transparency");
        return 0;
    }
    if ("Brightness" == name) {
        brightness = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Brightness");
        ModelXml->AddAttribute("Brightness", wxString::Format("%d", (int)brightness));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::Brightness");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::Transparency");
        return 0;
    }

    for (auto it = servos.begin(); it != servos.end(); ++it) {
        if ((*it)->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
            return 0;
        }
    }

    for (auto it = static_images.begin(); it != static_images.end(); ++it) {
        if ((*it)->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
            return 0;
        }
    }

    for (auto it = motion_images.begin(); it != motion_images.end(); ++it) {
        if ((*it)->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
            return 0;
        }
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxServo::InitModel() {
    num_servos = wxAtoi(ModelXml->GetAttribute("NumServos", "1"));
    _16bit = wxAtoi(ModelXml->GetAttribute("Bits16", "1"));

    int min_channels = num_servos * (_16bit ? 2 : 1);
    if (parm1 < min_channels) {
        parm1 = min_channels;
    }

    DmxModel::InitModel();
    DisplayAs = "DmxServo";

    // clear any extras
    while (servos.size() > num_servos) {
        Servo* ptr = servos.back();
        if (ptr != nullptr) {
            delete ptr;
            ptr = nullptr;
        }
        servos.pop_back();
    }
    while (static_images.size() > num_servos) {
        DmxImage* ptr = static_images.back();
        if (ptr != nullptr) {
            delete ptr;
            ptr = nullptr;
        }
        static_images.pop_back();
    }
    while (motion_images.size() > num_servos) {
        DmxImage* ptr = motion_images.back();
        if (ptr != nullptr) {
            delete ptr;
            ptr = nullptr;
        }
        motion_images.pop_back();
    }

    // resize vector arrays
    if (static_images.size() < num_servos) {
        static_images.resize(num_servos);
    }
    if (motion_images.size() < num_servos) {
        motion_images.resize(num_servos);
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
        int static_idx = name.find("StaticImage");
        int motion_idx = name.find("MotionImage");

        if ("StaticImage" == name) {  // convert original name that had no number
            // copy attributes to new name
            wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "StaticImage1");
            ModelXml->AddChild(new_node);
            for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext())
            {
                new_node->AddAttribute(a->GetName(), a->GetValue());
            }
            snode = n;
            static_images[0] = new DmxImage(new_node, "StaticImage1");
        }
        else if ("MotionImage" == name) {  // convert original name that had no number
            // copy attributes to new name
            wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "MotionImage1");
            ModelXml->AddChild(new_node);
            for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext())
            {
                new_node->AddAttribute(a->GetName(), a->GetValue());
            }
            mnode = n;
            motion_images[0] = new DmxImage(new_node, "MotionImage1");
        }
        else if (static_idx != std::string::npos) {
            std::string num = name.substr(11, name.length());
            int id = atoi(num.c_str()) - 1;
            if (id < num_servos) {
                if (static_images[id] == nullptr) {
                    static_images[id] = new DmxImage(n, name);
                }
            }
        }
        else if (motion_idx != std::string::npos) {
            std::string num = name.substr(11, name.length());
            int id = atoi(num.c_str()) - 1;
            if (id < num_servos) {
                if (motion_images[id] == nullptr) {
                    motion_images[id] = new DmxImage(n, name);
                }
            }
        }
        else if (servo_idx != std::string::npos) {
            std::string num = name.substr(5, name.length());
            int id = atoi(num.c_str()) - 1;
            if (id < num_servos) {
                if (servos[id] == nullptr) {
                    servos[id] = new Servo(n, name);
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
        if (static_images[i] == nullptr) {
            std::string new_name = "StaticImage" + std::to_string(i + 1);
            wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
            ModelXml->AddChild(new_node);
            static_images[i] = new DmxImage(new_node, new_name);
        }
        if (motion_images[i] == nullptr) {
            std::string new_name = "MotionImage" + std::to_string(i + 1);
            wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
            ModelXml->AddChild(new_node);
            motion_images[i] = new DmxImage(new_node, new_name);
            motion_images[i]->SetOffsetZ(i * 0.01f, this);  // offset on creation so its not hidden
        }
        if (servos[i] == nullptr) {
            std::string new_name = "Servo" + std::to_string(i + 1);
            wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
            ModelXml->AddChild(new_node);
            servos[i] = new Servo(new_node, new_name);
            servos[i]->SetChannel(_16bit ? i*2+1 : i+1, this);
        }
    }

    transparency = wxAtoi(ModelXml->GetAttribute("Transparency", "0"));
    brightness = wxAtoi(ModelXml->GetAttribute("Brightness", "100"));

    for (auto it = servos.begin(); it != servos.end(); ++it) {
        (*it)->Init(this);
        (*it)->Set16Bit(_16bit);
    }

    bool last_exists = !motion_images[0]->GetExists();
    for (auto it = static_images.begin(); it != static_images.end(); ++it) {
        (*it)->Init(this, !last_exists);
        last_exists = (*it)->GetExists();
    }

    last_exists = static_images[0]->GetExists();
    for (auto it = motion_images.begin(); it != motion_images.end(); ++it) {
        (*it)->Init(this, !last_exists);
        last_exists = (*it)->GetExists();
    }

    // create node names
    std::string names = "";
    int index = 1;
    for (auto it = servos.begin(); it != servos.end(); ++it) {
        if (!names.empty()) {
            names += ",";
        }
        names += "Servo" + std::to_string(index) + ",-Servo" + std::to_string(index);
        index++;
    }
    SetNodeNames(names, update_node_names);
    update_node_names = false;
}

void DmxServo::DrawModel(ModelPreview* preview, DrawGLUtils::xlAccumulator& va, const xlColor* c, float& sx, float& sy, bool active)
{
    // crash protection
    if (motion_images.size() < num_servos) {
        return;
    }
    for (auto it = servos.begin(); it != servos.end(); ++it) {
        if ((*it)->GetChannel() > Nodes.size())
        {
            return;
        }
    }

    // this if statement scales the motion image(s) so that is has the same size ratio as the original images
    for (auto it = motion_images.begin(); it != motion_images.end(); ++it) {
        if ((*it)->ImageSelected() && (*it)->GetExists() && static_images[0]->GetExists()) {
            if (static_images[0]->GetWidth() != 0) {
                float new_scale = (float)(*it)->GetWidth() / (float)static_images[0]->GetWidth();
                (*it)->SetScaleX(new_scale, this);
                (*it)->SetScaleY(new_scale, this);
                (*it)->ClearImageSelected();
            }
        }
    }

    float servo_pos[SUPPORTED_SERVOS] = { 0.0f };
    glm::mat4 Identity = glm::mat4(1.0f);
    glm::mat4 scalingMatrix = glm::scale(Identity, GetModelScreenLocation().GetScaleMatrix());
    glm::mat4 translateMatrix = glm::translate(Identity, GetModelScreenLocation().GetWorldPosition());
    glm::quat rotateQuat = GetModelScreenLocation().GetRotationQuat();
    glm::mat4 base_matrix = translateMatrix * glm::toMat4(rotateQuat) * scalingMatrix;


    for (int i = 0; i < servos.size(); ++i) {
        glm::mat4 motion_matrix = Identity;
        static_images[i]->Draw(this, preview, va, base_matrix, motion_matrix, transparency, brightness, !motion_images[i]->GetExists(), 0, 0, false, false);
        if (servos[i]->GetChannel() > 0 && active) {
            servo_pos[i] = servos[i]->GetPosition(GetChannelValue(servos[i]->GetChannel() - 1));
            if (servos[i]->IsTranslate()) {
                glm::vec3 scale = GetBaseObjectScreenLocation().GetScaleMatrix();
                servo_pos[i] /= scale.x;
            }
        }
        servos[i]->FillMotionMatrix(servo_pos[i], motion_matrix);
        motion_images[i]->Draw(this, preview, va, base_matrix, motion_matrix, transparency, brightness, !static_images[i]->GetExists(),
            servos[i]->GetPivotOffsetX(), servos[i]->GetPivotOffsetY(), servos[i]->IsRotate(), !active);
    }

}

void DmxServo::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator& va, const xlColor* c, float& sx, float& sy, bool active)
{
    DrawModel(preview, va, c, sx, sy, active);

}

void DmxServo::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator& va, const xlColor* c, float& sx, float& sy, float& sz, bool active)
{
    DrawModel(preview, va, c, sx, sy, active);
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

    wxString bits = ModelXml->GetAttribute("Bits16");
    wxString brt = ModelXml->GetAttribute("Brightness");
    wxString trans = ModelXml->GetAttribute("Transparency");
    f.Write(wxString::Format("Bits16=\"%s\" ", bits));
    f.Write(wxString::Format("Brightness=\"%s\" ", brt));
    f.Write(wxString::Format("Transparency=\"%s\" ", trans));

    f.Write(" >\n");

    wxString show_dir = GetModelManager().GetXLightsFrame()->GetShowDirectory();
    for (auto it = static_images.begin(); it != static_images.end(); ++it) {
        (*it)->Serialise(ModelXml, f, show_dir);
    }
    for (auto it = motion_images.begin(); it != motion_images.end(); ++it) {
        (*it)->Serialise(ModelXml, f, show_dir);
    }
    for (auto it = servos.begin(); it != servos.end(); ++it) {
        (*it)->Serialise(ModelXml, f, show_dir);
    }

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
            wxString bits = ModelXml->GetAttribute("Bits16");
            wxString brt = root->GetAttribute("Brightness");
            wxString trans = root->GetAttribute("Transparency");

            // Add any model version conversion logic here
            // Source version will be the program version that created the custom model

            SetProperty("Bits16", bits);
            SetProperty("Brightness", brt);
            SetProperty("Transparency", trans);

            wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
            GetModelScreenLocation().Write(ModelXml);
            SetProperty("name", newname, true);

            wxString show_dir = GetModelManager().GetXLightsFrame()->GetShowDirectory();
            for (auto it = static_images.begin(); it != static_images.end(); ++it) {
                (*it)->Serialise(root, ModelXml, show_dir);
            }
            for (auto it = motion_images.begin(); it != motion_images.end(); ++it) {
                (*it)->Serialise(root, ModelXml, show_dir);
            }
            for (auto it = servos.begin(); it != servos.end(); ++it) {
                (*it)->Serialise(root, ModelXml, show_dir);
            }

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
