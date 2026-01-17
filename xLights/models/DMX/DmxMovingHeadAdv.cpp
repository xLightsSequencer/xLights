/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>
#include <wx/stdpaths.h>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxColorAbility.h"
#include "DmxColorAbilityRGB.h"
#include "DmxColorAbilityCMY.h"
#include "DmxColorAbilityWheel.h"
#include "DmxMotor.h"
#include "DmxMovingHeadAdv.h"
#include "DmxPresetAbility.h"
#include "Mesh.h"
#include "MovingHeads/MhFeature.h"
#include "MovingHeads/MhFeatureDialog.h"
#include "../../controllers/ControllerCaps.h"
#include "../../ModelPreview.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"

enum MOTION_LINK {
    MOTION_LINK_MESH1,
    MOTION_LINK_MESH2
};

#define ToRadians(x) ((double)x * PI / (double)180.0)

class dmxPoint3 {

public:
    float x;
    float y;
    float z;

    dmxPoint3(float x_, float y_, float z_, float pan_angle_, float tilt_angle_ = 0)
        : x(x_), y(y_), z(z_)
    {
        float pan_angle = wxDegToRad(pan_angle_);
        float tilt_angle = wxDegToRad(tilt_angle_);

        glm::vec4 position = glm::vec4(glm::vec3(x_, y_, z_), 1.0);

        glm::mat4 rotationMatrixPan = glm::rotate(glm::mat4(1.0f), pan_angle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotationMatrixTilt = glm::rotate(glm::mat4(1.0f), tilt_angle, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::vec4 model_position = position * rotationMatrixTilt * rotationMatrixPan;
        x = model_position.x;
        y = model_position.y;
        z = model_position.z;
    }
};

// These recreate the DMX color types from DmxModel so that the "Unused" option could be added
static const char* DMX_COLOR_TYPES_ADV_VALUES[] = {
    "RGBW",
    "ColorWheel",
    "CMYW",
    "Unused"
};

static wxPGChoices DMX_COLOR_TYPES_ADV(wxArrayString(4, DMX_COLOR_TYPES_ADV_VALUES));

DmxMovingHeadAdv::DmxMovingHeadAdv(const ModelManager &manager) :
    DmxMovingHeadComm(manager)
{
    wxStandardPaths stdp = wxStandardPaths::Get();
#ifndef __WXMSW__
    obj_path = wxStandardPaths::Get().GetResourcesDir() + "/meshobjects/SimpleMovingHead/";
#else
#ifdef _DEBUG
    obj_path = wxFileName(stdp.GetExecutablePath()).GetPath() + "/../../../meshobjects/SimpleMovingHead/";
#else
    obj_path = wxFileName(stdp.GetExecutablePath()).GetPath() + "/meshobjects/SimpleMovingHead/";
#endif
#endif
    beam_width = GetDefaultBeamWidth();
}

DmxMovingHeadAdv::~DmxMovingHeadAdv()
{
    Clear();
}

void DmxMovingHeadAdv::Clear() {
    pan_motor.reset();
    tilt_motor.reset();
    //if (pan_motor != nullptr) {
    //    delete pan_motor;
    //}
    //
    //if (tilt_motor != nullptr) {
    //    delete tilt_motor;
    //}

    if (base_mesh != nullptr) {
        delete base_mesh;
    }

    if (yoke_mesh != nullptr) {
        delete yoke_mesh;
    }

    if (head_mesh != nullptr) {
        delete head_mesh;
    }
}

static wxPGChoices DMX_FIXTURES;

static const std::string CLICK_TO_EDIT("--Click To Edit--");
class MhConfigDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    MhConfigDialogAdapter(DmxMovingHeadAdv* model, std::vector<std::unique_ptr<MhFeature>>& _features, wxXmlNode* _node_xml) :
        wxPGEditorDialogAdapter(), m_model(model), features(_features), node_xml(_node_xml)
    {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
                              wxPGProperty* WXUNUSED(property)) override
    {
        MhFeatureDialog dlg(features, node_xml, propGrid);

        if (dlg.ShowModal() == wxID_OK) {
            bool changed = true;

            if (changed) {
                m_model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::MhConfigDialogAdapter");
                m_model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxMovingHeadAdv::MhConfigDialogAdapter");
                m_model->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::MhConfigDialogAdapter");
                m_model->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMovingHeadAdv::MhConfigDialogAdapter");
            }

            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            return true;
        }
        return false;
    }

protected:
    DmxMovingHeadAdv* m_model;
    std::vector<std::unique_ptr<MhFeature>>& features;
    wxXmlNode* node_xml;
};

class MhPopupDialogProperty : public wxStringProperty
{
public:
    MhPopupDialogProperty(DmxMovingHeadAdv* m,
                          std::vector<std::unique_ptr<MhFeature>>& _features,
                          wxXmlNode* _node_xml,
                          const wxString& label,
                          const wxString& name,
                          const wxString& value,
                          int type) :
        wxStringProperty(label, name, value), m_model(m), features(_features), features_xml_node(_node_xml), m_tp(type)
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
            return new MhConfigDialogAdapter(m_model, features, features_xml_node);
        default:
            break;
        }
        return nullptr;
    }

protected:
    DmxMovingHeadAdv* m_model = nullptr;
    std::vector<std::unique_ptr<MhFeature>>& features;
    wxXmlNode* features_xml_node;
    int m_tp;
};


void DmxMovingHeadAdv::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    // Disable until features are active
    //auto p = grid->Append(new MhPopupDialogProperty(this, features, features_xml_node, "Moving Head Config", "MHConfig", CLICK_TO_EDIT, 1));
    //grid->LimitPropertyEditing(p);

    if (DMX_FIXTURES.GetCount() == 0) {
        DMX_FIXTURES.Add("MH1");
        DMX_FIXTURES.Add("MH2");
        DMX_FIXTURES.Add("MH3");
        DMX_FIXTURES.Add("MH4");
        DMX_FIXTURES.Add("MH5");
        DMX_FIXTURES.Add("MH6");
        DMX_FIXTURES.Add("MH7");
        DMX_FIXTURES.Add("MH8");
    }

    auto p = grid->Append(new wxEnumProperty("Fixture", "DmxFixture", DMX_FIXTURES, fixture_val));

    DmxModel::AddTypeProperties(grid, outputManager);

    pan_motor->AddTypeProperties(grid);
    tilt_motor->AddTypeProperties(grid);

    grid->Append(new wxPropertyCategory("Color Properties", "DmxColorAbility"));
    int selected = 3; // show Unused if not selected
    if (nullptr != color_ability) {
        selected = DMX_COLOR_TYPES_ADV.Index(color_ability->GetTypeName());
    }
    grid->Append(new wxEnumProperty("Color Type", "DmxColorType", DMX_COLOR_TYPES_ADV, selected));
    if (nullptr != color_ability) {
        ControllerCaps *caps = GetControllerCaps();
        color_ability->AddColorTypeProperties(grid, IsPWMProtocol() && caps && caps->SupportsPWM());
    }
    grid->Collapse("DmxColorAbility");

    AddDimmerTypeProperties(grid);
    AddShutterTypeProperties(grid);
    grid->Collapse("DmxDimmerProperties");
    grid->Collapse("DmxShutterProperties");

    p = grid->Append(new wxPropertyCategory("Beam Properties", "BeamProperties"));

    p = grid->Append(new wxFloatProperty("Beam Display Length", "DmxBeamLength", beam_length));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxFloatProperty("Beam Display Width", "DmxBeamWidth", beam_width));
    p->SetAttribute("Min", 0.01);
    p->SetAttribute("Max", 150);
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Beam Orientation", "DmxBeamOrient", beam_orient));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 360);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxFloatProperty("Beam Y Offset", "DmxBeamYOffset", beam_y_offset));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 500);
    p->SetAttribute("Precision", 1);
    p->SetAttribute("Step", 1);
    p->SetEditor("SpinCtrl");

    grid->Collapse("BeamProperties");

    base_mesh->AddTypeProperties(grid);
    yoke_mesh->AddTypeProperties(grid);
    head_mesh->AddTypeProperties(grid);

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

void DmxMovingHeadAdv::DisableUnusedProperties(wxPropertyGridInterface* grid)
{
    // rotation around the Z axis causes issues when the pan and tilt rotations are applied
    // users should be able to achieve any desired position with only X and Y rotations
    auto p = grid->GetPropertyByName("c");
    if (p != nullptr) {
        p->Hide(true);
    }
    p = grid->GetPropertyByName("YokeMeshRotateZ");
    if (p != nullptr) {
        p->Hide(true);
    }
    p = grid->GetPropertyByName("HeadMeshRotateZ");
    if (p != nullptr) {
        p->Hide(true);
    }

    DmxModel::DisableUnusedProperties(grid);
}

int DmxMovingHeadAdv::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    std::string name = event.GetPropertyName().ToStdString();

    if (nullptr != color_ability && color_ability->OnColorPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    if (OnShutterPropertyGridChange(grid, event, this) == 0) {
        return 0;
    }

    if (OnDimmerPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    if ("DmxFixture" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxFixture");
        fixture_val = event.GetPropertyValue().GetLong();
        dmx_fixture = FixtureIDtoString(fixture_val); 
        ModelXml->AddAttribute("DmxFixture", dmx_fixture);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::DmxFixture");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::DmxFixture");
        return 0;
    } else if ("DmxBeamLength" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxBeamLength");
        ModelXml->AddAttribute("DmxBeamLength", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::DMXBeamLength");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHeadAdv::OnPropertyGridChange::DMXBeamLength");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::DMXBeamLength");
        return 0;
    }
    else if ("DmxBeamWidth" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxBeamWidth");
        ModelXml->AddAttribute("DmxBeamWidth", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::DMXBeamWidth");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHeadAdv::OnPropertyGridChange::DMXBeamWidth");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::DMXBeamWidth");
        return 0;
    }
    else if ("DmxBeamOrient" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxBeamOrient");
        ModelXml->AddAttribute("DmxBeamOrient", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamOrient");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamOrient");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamOrient");
        return 0;
    }
    else if ("DmxBeamYOffset" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxBeamYOffset");
        ModelXml->AddAttribute("DmxBeamYOffset", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamYOffset");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamYOffset");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamYOffset");
        return 0;
    }
    else if ("DmxColorType" == event.GetPropertyName()) {
        int color_type = event.GetPropertyValue().GetInteger();

        ModelXml->DeleteAttribute("DmxColorType");
        ModelXml->AddAttribute("DmxColorType", wxString::Format("%d", color_type));

        if (color_type == 0) {
            color_ability = std::make_unique<DmxColorAbilityRGB>();
        } else  if (color_type == 1) {
            color_ability = std::make_unique<DmxColorAbilityWheel>();
        } else  if (color_type == 2) {
            color_ability = std::make_unique<DmxColorAbilityCMY>();
        }
        else {
            color_ability = nullptr;
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::DmxColorType");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHeadAdv::OnPropertyGridChange::DmxColorType");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::DmxColorType");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMovingHeadAdv::OnPropertyGridChange::DmxColorType");
        return 0;
    }

    if (pan_motor->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    if (tilt_motor->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    if (base_mesh->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    if (yoke_mesh->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    if (head_mesh->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxMovingHeadAdv::MapChannelName(wxArrayString& array, int chan, std::string name)
{
    if (chan > 0) {
        array[chan-1] = name;
    }
}

void DmxMovingHeadAdv::InitModel()
{
    DmxModel::InitModel();
    DisplayAs = "DmxMovingHeadAdv";

    dimmer_channel = wxAtoi(ModelXml->GetAttribute("MhDimmerChannel", "0"));
    shutter_channel = wxAtoi(ModelXml->GetAttribute("DmxShutterChannel", "0"));
    shutter_threshold = wxAtoi(ModelXml->GetAttribute("DmxShutterOpen", "1"));
    shutter_on_value = wxAtoi(ModelXml->GetAttribute("DmxShutterOnValue", "0"));
    
    int color_type = wxAtoi(ModelXml->GetAttribute("DmxColorType", "0"));
    if (color_type == 0) {
        color_ability = std::make_unique<DmxColorAbilityRGB>();
    } else if (color_type == 1) {
        color_ability = std::make_unique<DmxColorAbilityWheel>();
    } else if (color_type == 2) {
        color_ability = std::make_unique<DmxColorAbilityCMY>();
    }
    else {
        color_ability = nullptr;
    }

    beam_length = wxAtof(ModelXml->GetAttribute("DmxBeamLength", "4.0"));
    if (ModelXml->HasAttribute("DmxBeamWidth")) {
        beam_width = wxAtof(ModelXml->GetAttribute("DmxBeamWidth"));
    }
    beam_orient = wxAtoi(ModelXml->GetAttribute("DmxBeamOrient", "0"));

    // Setup some reasonable defaults for new model creation
    if (ModelXml->HasAttribute("DmxBeamYOffset")) {
        beam_y_offset = wxAtof(ModelXml->GetAttribute("DmxBeamYOffset"));
    } else {
        ModelXml->AddAttribute("DmxBeamYOffset", "17");
        beam_y_offset = 17.0f;
    }
    if (!ModelXml->HasAttribute("DmxRedChannel")) {
        ModelXml->AddAttribute("DmxRedChannel", "5");
    }
    if (!ModelXml->HasAttribute("DmxGreenChannel")) {
        ModelXml->AddAttribute("DmxGreenChannel", "6");
    }
    if (!ModelXml->HasAttribute("DmxBlueChannel")) {
        ModelXml->AddAttribute("DmxBlueChannel", "7");
    }

    wxXmlNode* n = ModelXml->GetChildren();
    while (n != nullptr) {
        std::string name = n->GetName();

        if ("PanMotor" == name) {
             if (pan_motor == nullptr) {
                pan_motor = std::make_unique<DmxMotor>(n, name);
             }
        } else if ("TiltMotor" == name) {
            if (tilt_motor == nullptr) {
                tilt_motor = std::make_unique<DmxMotor>(n, name);
            }
        } else if ("BaseMesh" == name) {
            if (base_mesh == nullptr) {
                base_mesh = new Mesh(n, name);
            }
        } else if ("YokeMesh" == name) {
            if (yoke_mesh == nullptr) {
                yoke_mesh = new Mesh(n, name);
            }
        } else if ("HeadMesh" == name) {
            if (head_mesh == nullptr) {
                head_mesh = new Mesh(n, name);
            }
        } else if ("Features" == name) {
            if (features_xml_node == nullptr) {
                features_xml_node = n;
            }
        }
        n = n->GetNext();
    }

    // create pan motor
    if (pan_motor == nullptr) {
        std::string new_name = "PanMotor";
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
        ModelXml->AddChild(new_node);
        pan_motor = std::make_unique<DmxMotor>(new_node, new_name);
        pan_motor->SetChannelCoarse(1);
        new_node->AddAttribute("RangeOfMotion", "540");
        new_node->AddAttribute("OrientHome", "90");
        new_node->AddAttribute("SlewLimit", "180");
    }

    // create tilt motor
    if (tilt_motor == nullptr) {
        std::string new_name = "TiltMotor";
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
        ModelXml->AddChild(new_node);
        new_node->AddAttribute("OrientHome", "90");
        new_node->AddAttribute("SlewLimit", "180");
        tilt_motor = std::make_unique<DmxMotor>(new_node, new_name);
        tilt_motor->SetChannelCoarse(3);
    }

    // create base mesh
    if (base_mesh == nullptr) {
        std::string new_name = "BaseMesh";
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
        ModelXml->AddChild(new_node);
        wxString f = obj_path + "MovingHeadBase.obj";
        new_node->AddAttribute("ObjFile", f);
        new_node->AddAttribute("Brightness", "40");
        base_mesh = new Mesh(new_node, new_name);
    }

    // create yoke mesh
    if (yoke_mesh == nullptr) {
        std::string new_name = "YokeMesh";
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
        ModelXml->AddChild(new_node);
        wxString f = obj_path + "MovingHeadYoke.obj";
        new_node->AddAttribute("ObjFile", f);
        new_node->AddAttribute("RotateY", "90");
        new_node->AddAttribute("Brightness", "50");
        yoke_mesh = new Mesh(new_node, new_name);
    }

    // create head mesh
    if (head_mesh == nullptr) {
        std::string new_name = "HeadMesh";
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
        ModelXml->AddChild(new_node);
        wxString f = obj_path + "MovingHead.obj";
        new_node->AddAttribute("ObjFile", f);
        new_node->AddAttribute("RotateX", "90");
        new_node->AddAttribute("RotateY", "90");
        new_node->AddAttribute("OffsetY", "17");
        new_node->AddAttribute("Brightness", "80");
        head_mesh = new Mesh(new_node, new_name);
    }

    brightness = wxAtoi(ModelXml->GetAttribute("Brightness", "100"));

    bool base_defined = base_mesh->HasObjFile();
    bool yoke_defined = yoke_mesh->HasObjFile();
    bool head_defined = head_mesh->HasObjFile();

    pan_motor->Init(this);
    tilt_motor->Init(this);
    base_mesh->Init(this, head_defined ? false : (yoke_defined ? false : true));
    yoke_mesh->Init(this, head_defined ? false : (base_defined ? (yoke_defined ? true : false) : true));
    head_mesh->Init(this, head_defined ? true : (yoke_defined ? true : false));

    // I'd really like a better way to do this...colors are defaulting to
    // channels 1, 2, 3 and conflicting with Pan and Tilt defaults of 1 and 3
    CorrectDefaultColorChannels();

    // create node names
    std::string names = "";
    wxString nn = ModelXml->GetAttribute("NodeNames", "");
    wxArrayString nodestrings;
    nodestrings.resize(GetNumChannels());
    if (nn == "") {
        int chan = pan_motor->GetChannelCoarse();
        MapChannelName(nodestrings, chan, "Pan");
        if (pan_motor->Is16Bit()) {
            chan = pan_motor->GetChannelFine();
            MapChannelName(nodestrings, chan, "Pan Fine");
        }
        chan = tilt_motor->GetChannelCoarse();
        MapChannelName(nodestrings, chan, "Tilt");
        if (tilt_motor->Is16Bit()) {
            chan = tilt_motor->GetChannelFine();
            nodestrings[chan] = "Tilt Fine";
        }
        if (nullptr != color_ability) {
            if (color_ability->GetColorType() == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_RGBW) {
                DmxColorAbilityRGB* crgb = dynamic_cast<DmxColorAbilityRGB*>(color_ability.get());
                chan = crgb->GetRedChannel();
                MapChannelName(nodestrings, chan, "Red");
                chan = crgb->GetGreenChannel();
                MapChannelName(nodestrings, chan, "Green");
                chan = crgb->GetBlueChannel();
                MapChannelName(nodestrings, chan, "Blue");
                chan = crgb->GetWhiteChannel();
                MapChannelName(nodestrings, chan, "White");
            }
            else if (color_ability->GetColorType() == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_CMYW) {
                DmxColorAbilityCMY* ccmy = dynamic_cast<DmxColorAbilityCMY*>(color_ability.get());
                chan = ccmy->GetCyanChannel();
                MapChannelName(nodestrings, chan, "Cyan");
                chan = ccmy->GetMagentaChannel();
                MapChannelName(nodestrings, chan, "Magenta");
                chan = ccmy->GetYellowChannel();
                MapChannelName(nodestrings, chan, "Yellow");
                chan = ccmy->GetWhiteChannel();
                MapChannelName(nodestrings, chan, "White");
            }
        }
        names = wxJoin( nodestrings, ',');
    }
    SetNodeNames(names, update_node_names);
    
    dmx_fixture = ModelXml->GetAttribute("DmxFixture", "MH1");
    fixture_val = FixtureStringtoID(dmx_fixture);

    if (dmx_fixture.empty()) {
        dmx_fixture = "MH1";
    }

    // process features
    if( features_xml_node == nullptr ) {
        features_xml_node = new wxXmlNode(wxXML_ELEMENT_NODE, "Features");
        ModelXml->AddChild(features_xml_node);
    } else {
        n = features_xml_node->GetChildren();
        while (n != nullptr) {
            std::string node_name = n->GetName();
            std::string feature_name = n->GetAttribute("Name", node_name);
            bool feature_found {false};
            for (auto it = features.begin(); it != features.end(); ++it) {
                if( (*it)->GetName() == feature_name ) {
                    feature_found = true;
                    (*it)->Init();
                    break;
                }
            }
            if( !feature_found ) {
                std::unique_ptr<MhFeature> newFeature(new MhFeature(n, node_name, feature_name));
                newFeature->Init();
                features.push_back(std::move(newFeature));
            }
            n = n->GetNext();
        }
    }
}

void DmxMovingHeadAdv::CorrectDefaultColorChannels()
{
    if (nullptr != color_ability) {
        if (color_ability->GetColorType() == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_RGBW) {
            DmxColorAbilityRGB* crgb = dynamic_cast<DmxColorAbilityRGB*>(color_ability.get());
            if (pan_motor->GetChannelCoarse() == crgb->GetRedChannel() && crgb->GetRedChannel() == 1) {
                crgb->SetRedChannel(5);
                crgb->SetGreenChannel(6);
                crgb->SetBlueChannel(7);
            }
        }
    }
}

void DmxMovingHeadAdv::DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext* ctx,
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
    sprogram->addStep([=, this](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0.5 plane
            ctx->Translate(0, 0, 0.5f);
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    tprogram->addStep([=, this](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0.5 plane
            ctx->Translate(0, 0, 0.5f);
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    DrawModel(preview, ctx, sprogram, tprogram, !allowSelected, c);
    sprogram->addStep([=, this](xlGraphicsContext* ctx) {
        ctx->PopMatrix();
    });
    tprogram->addStep([=, this](xlGraphicsContext* ctx) {
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

void DmxMovingHeadAdv::DisplayEffectOnWindow(ModelPreview* preview, double pointSize)
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
        float scaleX = float(w) * 0.95f / float(GetModelScreenLocation().RenderWi);
        float scaleY = float(h) * 0.95f / float(GetModelScreenLocation().RenderHt);
        if (GetModelScreenLocation().RenderDp > 1) {
            float scaleZ = float(w) * 0.95f / float(GetModelScreenLocation().RenderDp);
            scaleX = std::min(scaleX, scaleZ);
        }

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
                           h / 2.0f - (mb < 0.0f ? mb : 0.0f), 0.5f);
            ctx->Scale(scaleX, scaleY, 0.001f);
        });
        preview->getCurrentSolidProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PushMatrix();
            ctx->Translate(w / 2.0f - (ml < 0.0f ? ml : 0.0f),
                           h / 2.0f - (mb < 0.0f ? mb : 0.0f), 0.5f);
            ctx->Scale(scaleX, scaleY, 0.001f);
        });
        DrawModel(preview, ctx, preview->getCurrentSolidProgram(), preview->getCurrentTransparentProgram(), true, nullptr);
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

int DmxMovingHeadAdv::GetMinChannels()
{
    int min_channels = (pan_motor->Is16Bit() ? 2 : 1) + (tilt_motor->Is16Bit() ? 2 : 1);
    if( nullptr != color_ability ) {
        min_channels += color_ability->GetNumChannels();
    }
    min_channels += shutter_channel > 0 ? 1 : 0;
    return min_channels;
}

std::list<std::string> DmxMovingHeadAdv::CheckModelSettings()
{
    std::list<std::string> res;

    int nodeCount = Nodes.size();
    int min_channels = GetMinChannels();
    if (min_channels > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s requires more channels %d than have been allocated to it %d.", GetName(), min_channels, nodeCount));
    }
    
    if (pan_motor->GetChannelCoarse() > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s pan motor coarse is assigned to channel %d but the model only has %d channels.", GetName(), pan_motor->GetChannelCoarse(), nodeCount));
    }

    if (pan_motor->GetChannelFine() > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s pan motor fine is assigned to channel %d but the model only has %d channels.", GetName(), pan_motor->GetChannelFine(), nodeCount));
    }

    if (tilt_motor->GetChannelCoarse() > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s tilt motor coarse is assigned to channel %d but the model only has %d channels.", GetName(), tilt_motor->GetChannelCoarse(), nodeCount));
    }

    if (tilt_motor->GetChannelFine() > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s tilt motor fine is assigned to channel %d but the model only has %d channels.", GetName(), tilt_motor->GetChannelFine(), nodeCount));
    }

    res.splice(res.end(), Model::CheckModelSettings());
    return res;
}

void DmxMovingHeadAdv::DrawModel(ModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* sprogram, xlGraphicsProgram* tprogram, bool active, const xlColor* c)
{
    // crash protection
    int min_channels = GetMinChannels();
    if (min_channels > Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (pan_motor->GetChannelCoarse() > Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (pan_motor->GetChannelFine() > Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (tilt_motor->GetChannelCoarse() > Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (tilt_motor->GetChannelFine() > Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    size_t NodeCount = Nodes.size();
    if ((( nullptr != color_ability ) && !color_ability->IsValidModelSettings(this)) ||
        !preset_ability->IsValidModelSettings(this) ||
        shutter_channel > NodeCount ||
        dimmer_channel > NodeCount)
    {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }

    // retrieve the model state
    float old_pan_angle = 0.0f;
    float old_tilt_angle = 0.0f;
    uint32_t old_ms = 0;

    PanTiltState &st = panTiltStates[preview->GetName().ToStdString()];
    if (active) {
        old_ms = st.ms;
        old_pan_angle = st.pan_angle;
        old_tilt_angle = st.tilt_angle;
    }

    float pan_angle = 0;
    if (pan_motor->GetChannelCoarse() > 0 && active) {
        pan_angle = pan_motor->GetPosition(GetChannelValue(pan_motor->GetChannelCoarse() - 1, pan_motor->GetChannelFine() - 1));
    }
    pan_angle += pan_motor->GetOrientZero();

    float tilt_angle = 0;
    if (tilt_motor->GetChannelCoarse() > 0 && active) {
        tilt_angle = tilt_motor->GetPosition(GetChannelValue(tilt_motor->GetChannelCoarse() - 1, tilt_motor->GetChannelFine() - 1));
    }
    tilt_angle += tilt_motor->GetOrientZero();

    uint32_t ms = preview->getCurrentFrameTime();
    uint32_t time_delta = 0;
    if (ms > old_ms) {
        time_delta = ms - old_ms;
    } else if (ms == old_ms && active) {
        pan_angle = old_pan_angle;
        tilt_angle = old_tilt_angle;
    }
    if (time_delta > 500) {
        // more than 1/2 second off, assume a jump of some sort
        time_delta = 0;
    }

    if (time_delta != 0 && active) {
        // pan slew limiting
        if (pan_motor->GetSlewLimit() > 0.0f) {
            float slew_limit = pan_motor->GetSlewLimit() * (float)time_delta / 1000.0f;
            float pan_delta = pan_angle - old_pan_angle;
            if (std::abs(pan_delta) > slew_limit) {
                if (pan_delta < 0) {
                    slew_limit *= -1.0f;
                }
                pan_angle = old_pan_angle + slew_limit;
            }
        }
    }

    float pan_angle_raw = pan_angle;

    if (time_delta != 0 && active) {
        // tilt slew limiting
        if (tilt_motor->GetSlewLimit() > 0.0f) {
            float slew_limit = tilt_motor->GetSlewLimit() * (float)time_delta / 1000.0f;
            float tilt_delta = tilt_angle - old_tilt_angle;
            if (std::abs(tilt_delta) > slew_limit) {
                if (tilt_delta < 0) {
                    slew_limit *= -1.0f;
                }
                tilt_angle = old_tilt_angle + slew_limit;
            }
        }
    }

    // Determine if we need to flip the beam
    int tilt_pos = (int)(RenderBuffer::cos(ToRadians(tilt_angle)));
    if (tilt_pos < 0) {
        if (pan_angle >= 180.0f) {
            pan_angle -= 180.0f;
        } else {
            pan_angle += 180.0f;
        }
    }

    // save the model state
    st.ms = ms;
    st.pan_angle = pan_angle_raw;
    st.tilt_angle = tilt_angle;

    glm::mat4 Identity = glm::mat4(1.0f);

    // Draw Meshs
    base_mesh->Draw(this, preview, sprogram, tprogram, Identity, Identity, false, 0, 0, 0, false, false);
    yoke_mesh->Draw(this, preview, sprogram, tprogram, Identity, Identity, 0, pan_angle_raw, 0, false, 0, 0, 0, false, !active);
    head_mesh->Draw(this, preview, sprogram, tprogram, Identity, Identity, tilt_angle, pan_angle_raw, 0, false, 0, 0, 0, false, !active);

    // Everything below here is for drawing the light beam
    float beam_length_displayed = beam_length;
    
    float scw = screenLocation.GetRenderWi() * screenLocation.GetScaleX();
    float sch = screenLocation.GetRenderHt() * screenLocation.GetScaleY();
    float scd = screenLocation.GetRenderDp() * screenLocation.GetScaleZ();
    float sbl = std::max(scw, std::max(sch, scd));
    beam_length_displayed *= sbl;

    // determine if shutter is open for heads that support it
    bool shutter_open = true;
    if (shutter_channel > 0 && shutter_channel <= NodeCount && active) {
        xlColor proxy;
        Nodes[shutter_channel - 1]->GetColor(proxy);
        int shutter_value = proxy.red;
        if (shutter_value >= 0) {
            shutter_open = shutter_value >= shutter_threshold;
        } else {
            shutter_open = shutter_value <= std::abs(shutter_threshold);
        }
    }

    xlColor color;
    if (c != nullptr) {
        color = *c;
    }
    int trans = color == xlBLACK ? blackTransparency : transparency;
    xlColor beam_color = color_ability == nullptr ? xlWHITE : color_ability->GetBeamColor(Nodes);
    if (!active) {
        beam_color = xlWHITE;
    }

    // apply dimmer to beam
    if (dimmer_channel > 0 && active) {
        xlColor proxy;
        Nodes[dimmer_channel - 1]->GetColor(proxy);
        HSVValue hsv = proxy.asHSV();
        beam_color.red = (beam_color.red * hsv.value);
        beam_color.blue = (beam_color.blue * hsv.value);
        beam_color.green = (beam_color.green * hsv.value);
    }

    ApplyTransparency(beam_color, trans, trans);

    pan_angle_raw += beam_orient;
    while (pan_angle_raw > 360.0f)
        pan_angle_raw -= 360.0f;
    pan_angle_raw = 360.0f - pan_angle_raw;

    auto vac = tprogram->getAccumulator();
    int start = vac->getCount();
    Draw3DBeam(vac, beam_color, beam_length_displayed, pan_angle_raw, tilt_angle, shutter_open, beam_y_offset);
    int end = vac->getCount();
    tprogram->addStep([=](xlGraphicsContext *ctx) {
        ctx->drawTriangles(vac, start, end - start);
    });
}

void DmxMovingHeadAdv::Draw3DBeam(xlVertexColorAccumulator* tvac, xlColor beam_color, float beam_length_displayed, float pan_angle_raw, float tilt_angle, bool shutter_open, float beam_offset)
{
    xlColor beam_color_end(beam_color);
    beam_color_end.alpha = 0;

    bool facing_right = pan_angle_raw <= 90.0f || pan_angle_raw >= 270.0f;

    if (beam_color.red != 0 || beam_color.green != 0 || beam_color.blue != 0) {
        if (shutter_open) {
            float angle1 = float(beam_width) / 2.0f;
            if (angle1 < 0.0f) {
                angle1 += 360.0f;
            }
            float x1 = (RenderBuffer::cos(ToRadians(angle1)) * beam_length_displayed);
            float y1 = (RenderBuffer::sin(ToRadians(angle1)) * beam_length_displayed);

            dmxPoint3 p1(x1, -y1, -y1, pan_angle_raw, tilt_angle);
            dmxPoint3 p2(x1, -y1, y1, pan_angle_raw, tilt_angle);
            dmxPoint3 p3(x1, y1, -y1, pan_angle_raw, tilt_angle);
            dmxPoint3 p4(x1, y1, y1, pan_angle_raw, tilt_angle);
            dmxPoint3 p0(0, 0, 0, pan_angle_raw, tilt_angle);
            p0.y += beam_offset;
            p1.y += beam_offset;
            p2.y += beam_offset;
            p3.y += beam_offset;
            p4.y += beam_offset;

            if (!facing_right) {
                tvac->AddVertex(p2.x, p2.y, p2.z, beam_color_end);
                tvac->AddVertex(p4.x, p4.y, p4.z, beam_color_end);
                tvac->AddVertex(p0.x, p0.y, p0.z, beam_color);
            } else {
                tvac->AddVertex(p1.x, p1.y, p1.z, beam_color_end);
                tvac->AddVertex(p3.x, p3.y, p3.z, beam_color_end);
                tvac->AddVertex(p0.x, p0.y, p0.z, beam_color);
            }

            tvac->AddVertex(p1.x, p1.y, p1.z, beam_color_end);
            tvac->AddVertex(p2.x, p2.y, p2.z, beam_color_end);
            tvac->AddVertex(p0.x, p0.y, p0.z, beam_color);

            tvac->AddVertex(p3.x, p3.y, p3.z, beam_color_end);
            tvac->AddVertex(p4.x, p4.y, p4.z, beam_color_end);
            tvac->AddVertex(p0.x, p0.y, p0.z, beam_color);

            if (facing_right) {
                tvac->AddVertex(p2.x, p2.y, p2.z, beam_color_end);
                tvac->AddVertex(p4.x, p4.y, p4.z, beam_color_end);
                tvac->AddVertex(p0.x, p0.y, p0.z, beam_color);
            } else {
                tvac->AddVertex(p1.x, p1.y, p1.z, beam_color_end);
                tvac->AddVertex(p3.x, p3.y, p3.z, beam_color_end);
                tvac->AddVertex(p0.x, p0.y, p0.z, beam_color);
            }
        }
    }
}

std::vector<std::string> DmxMovingHeadAdv::GenerateNodeNames() const {
    std::vector<std::string> names = DmxModel::GenerateNodeNames();

    if (0 != shutter_channel && shutter_channel < names.size()) {
        names[shutter_channel - 1] = "Shutter";
    }
    if (0 != pan_motor->GetChannelCoarse() && pan_motor->GetChannelCoarse() < names.size()) {
        names[pan_motor->GetChannelCoarse() - 1] = "Pan";
    }
    if (0 != tilt_motor->GetChannelCoarse() && tilt_motor->GetChannelCoarse() < names.size()) {
        names[tilt_motor->GetChannelCoarse() - 1] = "Tilt";
    }
    if (0 != pan_motor->GetChannelFine() && pan_motor->GetChannelFine() < names.size()) {
        names[pan_motor->GetChannelFine() - 1] = "Pan Fine";
    }
    if (0 != tilt_motor->GetChannelFine() && tilt_motor->GetChannelFine() < names.size()) {
        names[tilt_motor->GetChannelFine() - 1] = "Tilt Fine";
    }

    return names;
}
