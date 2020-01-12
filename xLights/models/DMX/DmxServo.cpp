#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxServo.h"
#include "DmxImage.h"

#include "../../ModelPreview.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"
#include <log4cpp/Category.hh>

DmxServo::DmxServo(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
    : DmxModel(node, manager, zeroBased), static_image(nullptr), motion_image(nullptr), servo_style_val(0)
{
    wxXmlNode* n = node->GetChildren();
    while (n != nullptr) {
        if ("staticImage" == n->GetName()) {
            static_image = new DmxImage(n, "StaticImage");
        }
        else if ("motionImage" == n->GetName()) {
            motion_image = new DmxImage(n, "MotionImage");
        }
        n = n->GetNext();
    }
    if (static_image == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "staticImage");
        node->AddChild(new_node);
        static_image = new DmxImage(new_node, "StaticImage");
    }
    if (motion_image == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "motionImage");
        node->AddChild(new_node);
        motion_image = new DmxImage(new_node, "MotionImage");
    }
}

DmxServo::~DmxServo()
{
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

void DmxServo::AddTypeProperties(wxPropertyGridInterface* grid) {
    AddTypePropertiesSpecial(grid);
}

void DmxServo::AddTypePropertiesSpecial(wxPropertyGridInterface *grid, bool last) {

    DmxModel::AddTypeProperties(grid);

    wxPGProperty* p = grid->Append(new wxUIntProperty("Servo Channel", "DmxServoChannel", servo_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Min Limit", "DmxServoMinLimit", min_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 65535);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Max Limit", "DmxServoMaxLimit", max_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 65535);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Range of Motion", "DmxServoRangeOfMotion", range_of_motion));
    p->SetAttribute("Min", -65535);
    p->SetAttribute("Max", 65535);
    p->SetEditor("SpinCtrl");

    if (SERVO_STYLES.GetCount() == 0) {
        SERVO_STYLES.Add("Translate X");
        SERVO_STYLES.Add("Translate Y");
        SERVO_STYLES.Add("Translate Z");
        SERVO_STYLES.Add("Rotate X");
        SERVO_STYLES.Add("Rotate Y");
        SERVO_STYLES.Add("Rotate Z");
    }

    grid->Append(new wxEnumProperty("Servo Style", "ServoStyle", SERVO_STYLES, servo_style_val));
    
    switch (servo_style_val) {
    case SERVO_STYLE_ROTATEX:
    case SERVO_STYLE_ROTATEY:
    case SERVO_STYLE_ROTATEZ:
        p = grid->Append(new wxIntProperty("Pivot Offset X", "DmxPivotOffsetX", pivot_offset_x));
        p->SetAttribute("Min", -10000);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxIntProperty("Pivot Offset Y", "DmxPivotOffsetY", pivot_offset_y));
        p->SetAttribute("Min", -10000);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
        break;
    default:
        break;
    }

    p = grid->Append(new wxUIntProperty("Brightness", "Brightness", (int)brightness));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    if (last) {
        p = grid->Append(new wxUIntProperty("Transparency", "Transparency", transparency));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 100);
        p->SetEditor("SpinCtrl");

        if (static_image != nullptr) {
            static_image->AddTypeProperties(grid);
        }
        if (motion_image != nullptr) {
            motion_image->AddTypeProperties(grid);
        }
        grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
    }
}

int DmxServo::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {

    if ("DmxServoChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxServoChannel");
        ModelXml->AddAttribute("DmxServoChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::DmxServoChannel");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxServo::OnPropertyGridChange::DmxServoChannel");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo::OnPropertyGridChange::DmxServoChannel");
        return 0;
    } else if ("DmxServoMinLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxServoMinLimit");
        ModelXml->AddAttribute("DmxServoMinLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::DmxServoMinLimit");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxServo::OnPropertyGridChange::DmxServoMinLimit");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::DmxServoMinLimit");
        return 0;
    } else if ("DmxServoMaxLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxServoMaxLimit");
        ModelXml->AddAttribute("DmxServoMaxLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::DmxServoMaxLimit");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxServo::OnPropertyGridChange::DmxServoMaxLimit");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::DmxServoMaxLimit");
        return 0;
    } else if ("DmxServoRangeOfMotion" == event.GetPropertyName()) {
         ModelXml->DeleteAttribute("DmxServoRangeOfMotion");
         ModelXml->AddAttribute("DmxServoRangeOfMotion", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
         AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::DmxServoRangeOfMotion");
         AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxServo::OnPropertyGridChange::DmxServoRangeOfMotion");
         AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::DmxServoRangeOfMotion");
         return 0;
    } else if ("DmxPivotOffsetX" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxPivotOffsetX");
        ModelXml->AddAttribute("DmxPivotOffsetX", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::DmxPivotOffsetX");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxServo::OnPropertyGridChange::DmxPivotOffsetX");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::DmxPivotOffsetX");
        return 0;
    } else if ("DmxPivotOffsetY" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxPivotOffsetY");
        ModelXml->AddAttribute("DmxPivotOffsetY", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::DmxPivotOffsetY");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxServo::OnPropertyGridChange::DmxPivotOffsetY");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::DmxPivotOffsetY");
        return 0;
    } else if ("ServoStyle" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("ServoStyle");
        servo_style_val = event.GetPropertyValue().GetLong();
        if (servo_style_val == SERVO_STYLE_TRANSLATEX) {
            servo_style = "Translate X";
        }
        else if (servo_style_val == SERVO_STYLE_TRANSLATEY) {
            servo_style = "Translate Y";
        }
        else if (servo_style_val == SERVO_STYLE_TRANSLATEZ) {
            servo_style = "Translate Z";
        }
        else if (servo_style_val == SERVO_STYLE_ROTATEX) {
            servo_style = "Rotate X";
        }
        else if (servo_style_val == SERVO_STYLE_ROTATEY) {
            servo_style = "Rotate Y";
        }
        else if (servo_style_val == SERVO_STYLE_ROTATEZ) {
            servo_style = "Rotate Z";
        }
        ModelXml->AddAttribute("ServoStyle", servo_style);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::ServoStyle");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo::OnPropertyGridChange::ServoStyle");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxServo::OnPropertyGridChange::ServoStyle");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "DmxServo::OnPropertyGridChange::ServoStyle");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::ServoStyle");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DmxServo::OnPropertyGridChange::ServoStyle");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo::OnPropertyGridChange::ServoStyle");
        return 0;
    }
    else if ("Transparency" == event.GetPropertyName()) {
        transparency = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Transparency");
        ModelXml->AddAttribute("Transparency", wxString::Format("%d", transparency));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::Transparency");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::Transparency");
        return 0;
    }
    else if ("Brightness" == event.GetPropertyName()) {
        brightness = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Brightness");
        ModelXml->AddAttribute("Brightness", wxString::Format("%d", (int)brightness));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::Brightness");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::Transparency");
        return 0;
    }

    if (static_image != nullptr) {
        static_image->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked());
    }
    if (motion_image != nullptr) {
        motion_image->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked());
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxServo::InitModel() {
    DmxModel::InitModel();
    DisplayAs = "DmxServo";

    servo_channel = wxAtoi(ModelXml->GetAttribute("DmxServoChannel", "1"));
    min_limit = wxAtoi(ModelXml->GetAttribute("DmxServoMinLimit", "0"));
    max_limit = wxAtoi(ModelXml->GetAttribute("DmxServoMaxLimit", "65535"));
    range_of_motion = wxAtoi(ModelXml->GetAttribute("DmxServoRangeOfMotion", "180"));
    pivot_offset_x = wxAtoi(ModelXml->GetAttribute("DmxPivotOffsetX", "0"));
    pivot_offset_y = wxAtoi(ModelXml->GetAttribute("DmxPivotOffsetY", "0"));

    transparency = wxAtoi(ModelXml->GetAttribute("Transparency", "0"));
    brightness = wxAtoi(ModelXml->GetAttribute("Brightness", "100"));

    servo_style = ModelXml->GetAttribute("ServoStyle", "Translate X");
    servo_style_val = SERVO_STYLE_TRANSLATEX;
    if (servo_style == "Translate X") {
        servo_style_val = SERVO_STYLE_TRANSLATEX;
    }
    else if (servo_style == "Translate Y") {
        servo_style_val = SERVO_STYLE_TRANSLATEY;
    }
    else if (servo_style == "Translate Z") {
        servo_style_val = SERVO_STYLE_TRANSLATEZ;
    }
    else if (servo_style == "Rotate X") {
        servo_style_val = SERVO_STYLE_ROTATEX;
    }
    else if (servo_style == "Rotate Y") {
        servo_style_val = SERVO_STYLE_ROTATEY;
    }
    else if (servo_style == "Rotate Z") {
        servo_style_val = SERVO_STYLE_ROTATEZ;
    }

    if (static_image != nullptr) {
        static_image->Init(this, true, !motion_image->GetExists());
    }

    if (motion_image != nullptr) {
        motion_image->Init(this, !static_image->GetExists(), !static_image->GetExists());
    }

    wxString nn = ModelXml->GetAttribute("NodeNames", "");
    wxString tempstr = nn;
    nodeNames.clear();
    while (tempstr.size() > 0) {
        std::string t2 = tempstr.ToStdString();
        if (tempstr[0] == ',') {
            t2 = "";
            tempstr = tempstr(1, tempstr.length());
        }
        else if (tempstr.Contains(",")) {
            t2 = tempstr.SubString(0, tempstr.Find(",") - 1);
            tempstr = tempstr.SubString(tempstr.Find(",") + 1, tempstr.length());
        }
        else {
            tempstr = "";
        }
        nodeNames.push_back(t2);
    }
}

int DmxServo::GetChannelValue( int channel )
{
    xlColor color_angle;
    int lsb = 0;
    int msb = 0;
    Nodes[channel]->GetColor(color_angle);
    msb = color_angle.red;
    Nodes[channel+1]->GetColor(color_angle);
    lsb = color_angle.red;
    return ((msb << 8) | lsb);
}

void DmxServo::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator& va, const xlColor* c, float& sx, float& sy, bool active)
{
    int channel_value;
    float servo_pos = 0.0;
    glm::mat4 Identity = glm::mat4(1.0f);
    glm::mat4 motion_matrix = Identity;
    glm::mat4 scalingMatrix = glm::scale(Identity, GetModelScreenLocation().GetScaleMatrix());
    glm::mat4 translateMatrix = glm::translate(Identity, GetModelScreenLocation().GetWorldPosition());
    glm::quat rotateQuat = GetModelScreenLocation().GetRotationQuat();
    glm::mat4 base_matrix = translateMatrix * glm::toMat4(rotateQuat) * scalingMatrix;

    if (static_image != nullptr) {
        static_image->Draw(this, preview, va, base_matrix, motion_matrix, transparency, brightness, 0, 0, false);
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

    bool use_pivot = false;

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
        use_pivot = true;
        motion_matrix = glm::rotate(Identity, glm::radians(servo_pos), glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    case SERVO_STYLE_ROTATEY:
        use_pivot = true;
        motion_matrix = glm::rotate(Identity, glm::radians(servo_pos), glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    case SERVO_STYLE_ROTATEZ:
        use_pivot = true;
        motion_matrix = glm::rotate(Identity, glm::radians(servo_pos), glm::vec3(0.0f, 0.0f, 1.0f));
        break;
    }

    if (motion_image != nullptr) {
        use_pivot = use_pivot & !active;
        motion_image->Draw(this, preview, va, base_matrix, motion_matrix, transparency, brightness, pivot_offset_x, pivot_offset_y, use_pivot);
    }
}

void DmxServo::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator& va, const xlColor* c, float& sx, float& sy, float& sz, bool active)
{
    int channel_value;
    float servo_pos = 0.0;
    glm::mat4 Identity = glm::mat4(1.0f);
    glm::mat4 motion_matrix = Identity;
    glm::mat4 scalingMatrix = glm::scale(Identity, GetModelScreenLocation().GetScaleMatrix());
    glm::mat4 translateMatrix = glm::translate(Identity, GetModelScreenLocation().GetWorldPosition());
    glm::quat rotateQuat = GetModelScreenLocation().GetRotationQuat();
    glm::mat4 base_matrix = translateMatrix * glm::toMat4(rotateQuat) * scalingMatrix;

    if (static_image != nullptr) {
        static_image->Draw(this, preview, va, base_matrix, motion_matrix, transparency, brightness, 0, 0, false);
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

    bool use_pivot = false;

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
        use_pivot = true;
        motion_matrix = glm::rotate(Identity, glm::radians(servo_pos), glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    case SERVO_STYLE_ROTATEY:
        use_pivot = true;
        motion_matrix = glm::rotate(Identity, glm::radians(servo_pos), glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    case SERVO_STYLE_ROTATEZ:
        use_pivot = true;
        motion_matrix = glm::rotate(Identity, glm::radians(servo_pos), glm::vec3(0.0f, 0.0f, 1.0f));
        break;
    }

    if (motion_image != nullptr) {
        use_pivot = use_pivot & !active;
        motion_image->Draw(this, preview, va, base_matrix, motion_matrix, transparency, brightness, pivot_offset_x, pivot_offset_y, use_pivot);
    }
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
