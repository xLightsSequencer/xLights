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

#include "DmxServo.h"
#include "DmxImage.h"
#include "Servo.h"

#include "../../ModelPreview.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"
#include <log4cpp/Category.hh>
#include "../../ModelPreview.h"

static const int SUPPORTED_SERVOS = 24;

DmxServo::DmxServo(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
    : DmxModel(node, manager, zeroBased), transparency(0), brightness(100),
      update_node_names(false), num_servos(1), _16bit(true)
{
    SetFromXml(node, zeroBased);
}

DmxServo::~DmxServo()
{
    Clear();
}

void DmxServo::Clear()
{
    for (auto it = servos.begin(); it != servos.end(); ++it) {
        if (*it != nullptr) {
            delete *it;
        }
    }
    servos.clear();
    for (auto it = motion_images.begin(); it != motion_images.end(); ++it) {
        if (*it != nullptr) {
            delete *it;
        }
    }
    motion_images.clear();
    for (auto it = static_images.begin(); it != static_images.end(); ++it) {
        if (*it != nullptr) {
            delete *it;
        }
    }
    static_images.clear();
}

void DmxServo::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    DmxModel::AddTypeProperties(grid, outputManager);

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

    for (const auto& it : servos) {
        it->AddTypeProperties(grid);
    }

    for (const auto& it : static_images) {
        it->AddTypeProperties(grid);
    }

    for (const auto& it : motion_images) {
        it->AddTypeProperties(grid);
    }

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

int DmxServo::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
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
    if (event.GetPropertyName() == "Bits16") {
        ModelXml->DeleteAttribute("Bits16");
        if (event.GetValue().GetBool()) {
            _16bit = true;
            ModelXml->AddAttribute("Bits16", "1");
        } else {
            _16bit = false;
            ModelXml->AddAttribute("Bits16", "0");
        }

        for (int i = 0; i < num_servos; ++i) {
            if (servos[i] != nullptr) {
                servos[i]->SetChannel(_16bit ? i * 2 + 1 : i + 1, this);
            }
        }

        int min_channels = num_servos * (_16bit ? 2 : 1);
        if (parm1 < min_channels) {
            UpdateChannelCount(min_channels, true);
        }
        update_node_names = true;
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::Bits16");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo::OnPropertyGridChange::Bits16");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::Bits16");
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
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::Brightness");
        return 0;
    }

    for (const auto& it : servos) {
        if (it->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked() || IsFromBase()) == 0) {
            return 0;
        }
    }

    for (const auto& it : static_images) {
        if (it->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked() || IsFromBase()) == 0) {
            return 0;
        }
    }

    for (const auto& it : motion_images) {
        if (it->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked() || IsFromBase()) == 0) {
            return 0;
        }
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxServo::InitModel()
{
    num_servos = wxAtoi(ModelXml->GetAttribute("NumServos", "1"));
    _16bit = wxAtoi(ModelXml->GetAttribute("Bits16", "1"));

    int min_channels = num_servos * (_16bit ? 2 : 1);
    if (parm1 < min_channels) {
        UpdateChannelCount(min_channels, false);
        std::string msg = wxString::Format("Channel count increased to %d to accommodate %d servos at %d bits.", min_channels, num_servos, _16bit ? 16 : 8);
        wxMessageBox(msg, "Minimum Channel Violation", wxOK | wxCENTER);
    }

    DmxModel::InitModel();
    DisplayAs = "DmxServo";
    screenLocation.SetRenderSize(1, 1, 1);

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

        if ("StaticImage" == name) { // convert original name that had no number
            // copy attributes to new name
            wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "StaticImage1");
            ModelXml->AddChild(new_node);
            for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext()) {
                new_node->AddAttribute(a->GetName(), a->GetValue());
            }
            snode = n;
            static_images[0] = new DmxImage(new_node, "StaticImage1");
        } else if ("MotionImage" == name) { // convert original name that had no number
            // copy attributes to new name
            wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "MotionImage1");
            ModelXml->AddChild(new_node);
            for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext()) {
                new_node->AddAttribute(a->GetName(), a->GetValue());
            }
            mnode = n;
            motion_images[0] = new DmxImage(new_node, "MotionImage1");
        } else if (static_idx != std::string::npos) {
            std::string num = name.substr(11, name.length());
            int id = atoi(num.c_str()) - 1;
            if (id < num_servos) {
                if (static_images[id] == nullptr) {
                    static_images[id] = new DmxImage(n, name);
                }
            }
        } else if (motion_idx != std::string::npos) {
            std::string num = name.substr(11, name.length());
            int id = atoi(num.c_str()) - 1;
            if (id < num_servos) {
                if (motion_images[id] == nullptr) {
                    motion_images[id] = new DmxImage(n, name);
                }
            }
        } else if (servo_idx != std::string::npos) {
            std::string num = name.substr(5, name.length());
            int id = atoi(num.c_str()) - 1;
            if (id < num_servos) {
                if (servos[id] == nullptr) {
                    servos[id] = new Servo(n, name, true);
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
            motion_images[i]->SetOffsetZ(i * 0.01f, this); // offset on creation so its not hidden
        }
        if (servos[i] == nullptr) {
            std::string new_name = "Servo" + std::to_string(i + 1);
            wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
            ModelXml->AddChild(new_node);
            servos[i] = new Servo(new_node, new_name, true);
            servos[i]->SetChannel(_16bit ? i * 2 + 1 : i + 1, this);
        }
    }

    transparency = wxAtoi(ModelXml->GetAttribute("Transparency", "0"));
    brightness = wxAtoi(ModelXml->GetAttribute("Brightness", "100"));

    for (auto it = servos.begin(); it != servos.end(); ++it) {
        (*it)->Init(this);
        (*it)->Set16Bit(_16bit);
    }

    for (auto it = static_images.begin(); it != static_images.end(); ++it) {
        (*it)->Init(this);
    }

    for (auto it = motion_images.begin(); it != motion_images.end(); ++it) {
        (*it)->Init(this);
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

void DmxServo::DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext* ctx,
                                    xlGraphicsProgram* solidProgram, xlGraphicsProgram* transparentProgram, bool is_3d,
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

    xlGraphicsProgram* program = transparentProgram;
    program->addStep([=](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0 plane
            ctx->Scale(1.0, 1.0, 0.0);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    DrawModel(preview, ctx, program, c, !allowSelected);
    program->addStep([=](xlGraphicsContext* ctx) {
        ctx->PopMatrix();
    });
    if ((Selected || (Highlighted && is_3d)) && c != nullptr && allowSelected) {
        if (is_3d) {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), Highlighted, IsFromBase());
        } else {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), IsFromBase());
        }
    }
}

void DmxServo::DisplayEffectOnWindow(ModelPreview* preview, double pointSize)
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
        float scaleX = float(w) * 0.95 / GetModelScreenLocation().RenderWi;
        float scaleY = float(h) * 0.95 / GetModelScreenLocation().RenderHt;

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
            ctx->Translate(w / 2.0f - (ml < 0.0f ? ml : 0.0f),
                           h / 2.0f - (mb < 0.0f ? mb : 0.0f), 0.0f);
            ctx->Scale(scaleX, scaleY, 1.0);
        });
        DrawModel(preview, ctx, preview->getCurrentTransparentProgram(), nullptr, true);
        preview->getCurrentTransparentProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PopMatrix();
        });
    }
    if (mustEnd) {
        preview->EndDrawing();
    }
}

std::list<std::string> DmxServo::CheckModelSettings()
{
    std::list<std::string> res;

    int nodeCount = Nodes.size();
    int min_channels = num_servos * (_16bit ? 2 : 1);

    if (min_channels > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s requires more channels %d than have been allocated to it %d.", GetName(), min_channels, nodeCount));
    }
    if (motion_images.size() < num_servos) {
        res.push_back(wxString::Format("    ERR: Model %s Insufficient images defined %d when %d required.", GetName(), motion_images.size(), num_servos));
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

void DmxServo::DrawModel(ModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* program, const xlColor* c, bool active)
{
    // crash protection
    int min_channels = num_servos * (_16bit ? 2 : 1);
    if (min_channels > Nodes.size()) {
        DmxModel::DrawInvalid(program, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (motion_images.size() < num_servos) {
        DmxModel::DrawInvalid(program, &(GetModelScreenLocation()), false, false);
        return;
    }
    for (const auto& it : servos) {
        if (it->GetChannel() > Nodes.size()) {
            DmxModel::DrawInvalid(program, &(GetModelScreenLocation()), false, false);
            return;
        }
    }

    float servo_pos[SUPPORTED_SERVOS] = { 0.0f };

    for (int i = 0; i < servos.size(); ++i) {
        //need to layer the images via slight Z offsets or they blend together and don't appear.
        glm::mat4 motionMatrix = glm::mat4(1.0f);
        motionMatrix = glm::translate(motionMatrix, glm::vec3(0, 0, 0.2 * float(i)));
        static_images[i]->Draw(this, preview, program, motionMatrix, transparency, brightness, !motion_images[i]->GetExists(), 0, 0, false, false);
        if (servos[i]->GetChannel() > 0 && active) {
            servo_pos[i] = servos[i]->GetPosition(GetChannelValue(servos[i]->GetChannel() - 1, servos[i]->Is16Bit()));
        }
        servos[i]->FillMotionMatrix(servo_pos[i], motionMatrix);
        motionMatrix = glm::translate(motionMatrix, glm::vec3(0, 0, 0.2 * float(i) + 0.1));
        motion_images[i]->Draw(this, preview, program, motionMatrix,
                               transparency, brightness, !static_images[i]->GetExists(),
                               servos[i]->GetPivotOffsetX(), servos[i]->GetPivotOffsetY(),
                               servos[i]->IsRotate(), !active);
    }
}

void DmxServo::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty())
        return;
    wxFile f(filename);
    if (!f.Create(filename, true) || !f.IsOpened())
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dmxservo \n");

    ExportBaseParameters(f);

    wxString bits = ModelXml->GetAttribute("Bits16", "1");
    wxString brt = ModelXml->GetAttribute("Brightness", "100");
    wxString trans = ModelXml->GetAttribute("Transparency", "0");
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
    f.Write("</dmxservo>");
    f.Close();
}

void DmxServo::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    if (root->GetName() == "dmxservo") {
        ImportBaseParameters(root);

        wxString name = root->GetAttribute("name");
        wxString v = root->GetAttribute("SourceVersion");
        wxString bits = ModelXml->GetAttribute("Bits16", "1");
        wxString brt = root->GetAttribute("Brightness", "100");
        wxString trans = root->GetAttribute("Transparency", "0");

        // Add any model version conversion logic here
        // Source version will be the program version that created the custom model

        wxXmlNode* n = root->GetChildren();
        while (n != nullptr) {
            std::string name = n->GetName();

            if ("StaticImage" == name) { // convert original name that had no number
                // copy attributes to new name
                wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "StaticImage1");
                root->AddChild(new_node);
                for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext()) {
                    new_node->AddAttribute(a->GetName(), a->GetValue());
                }
            } else if ("MotionImage" == name) { // convert original name that had no number
                // copy attributes to new name
                wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "MotionImage1");
                root->AddChild(new_node);
                for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext()) {
                    new_node->AddAttribute(a->GetName(), a->GetValue());
                }
            }
            n = n->GetNext();
        }

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

        ImportModelChildren(root, xlights, newname, min_x, max_x, min_y, max_y);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::ImportXlightsModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo::ImportXlightsModel");
    } else {
        DisplayError("Failure loading DmxServo model file.");
    }
}
