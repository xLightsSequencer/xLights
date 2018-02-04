#include "DmxModel.h"
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>
#include "ModelScreenLocation.h"
#include "../ModelPreview.h"
#include "../RenderBuffer.h"
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <wx/log.h>
#include <wx/filedlg.h>
#include "../xLightsVersion.h"
#include <wx/msgdlg.h>
#include "../xLightsMain.h"

DmxModel::DmxModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    style_changed = false;
    SetFromXml(node, zeroBased);
}
DmxModel::DmxModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    //ctor
}

DmxModel::~DmxModel()
{
    //dtor
}

const double PI  =3.141592653589793238463;
#define ToRadians(x) ((double)x * PI / (double)180.0)

class dmxPoint {

public:
    float x;
    float y;

    dmxPoint( float x_, float y_, int cx_, int cy_, float scale_, float angle_ )
    : x(x_), y(y_), cx(cx_), cy(cy_), scale(scale_)
    {
        float s = RenderBuffer::sin(ToRadians(angle_));
        float c = RenderBuffer::cos(ToRadians(angle_));

        // scale point
        x *= scale;
        y *= scale;

        // rotate point
        float xnew = x * c - y * s;
        float ynew = x * s + y * c;

        // translate point
        x = xnew + cx;
        y = ynew + cy;
    }

private:
    float cx;
    float cy;
    float scale;
};

class dmxPoint3 {

public:
    float x;
    float y;
    float z;

    dmxPoint3( float x_, float y_, float z_, int cx_, int cy_, float scale_, float pan_angle_, float tilt_angle_, float nod_angle_ = 0.0 )
    : x(x_), y(y_), z(z_)
    {
        float pan_angle = wxDegToRad(pan_angle_);
        float tilt_angle = wxDegToRad(tilt_angle_);
        float nod_angle = wxDegToRad(nod_angle_);

        glm::vec4 position = glm::vec4(glm::vec3(x_,y_,z_), 1.0);

        glm::mat4 rotationMatrixPan = glm::rotate(glm::mat4(1.0f), pan_angle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotationMatrixTilt = glm::rotate(glm::mat4(1.0f), tilt_angle, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 rotationMatrixNod = glm::rotate(glm::mat4(1.0f), nod_angle, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3((float)cx_, (float)cy_, 0.0f));
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale_));
        glm::vec4 model_position = translateMatrix * rotationMatrixPan * rotationMatrixTilt * rotationMatrixNod * scaleMatrix * position;
        x = model_position.x;
        y = model_position.y;
    }
};

static wxPGChoices DMX_STYLES;

enum DMX_STYLE {
    DMX_STYLE_MOVING_HEAD_TOP,
    DMX_STYLE_MOVING_HEAD_SIDE,
    DMX_STYLE_MOVING_HEAD_BARS,
    DMX_STYLE_MOVING_HEAD_TOP_BARS,
    DMX_STYLE_MOVING_HEAD_SIDE_BARS,
    DMX_STYLE_MOVING_HEAD_3D,
    DMX_STYLE_BASIC_FLOOD,
    DMX_STYLE_SKULLTRONIX_SKULL
};

void DmxModel::GetBufferSize(const std::string &type, const std::string &transform, int &BufferWi, int &BufferHi) const {
        BufferHi = 1;
        BufferWi = GetNodeCount();
}
void DmxModel::InitRenderBufferNodes(const std::string &type,  const std::string &transform,
                                     std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi) const {
    BufferHi = 1;
    BufferWi = GetNodeCount();

    for (int cur=0; cur < BufferWi; cur++) {
        newNodes.push_back(NodeBaseClassPtr(Nodes[cur]->clone()));
        for(size_t c=0; c < newNodes[cur]->Coords.size(); c++) {
            newNodes[cur]->Coords[c].bufX=cur;
            newNodes[cur]->Coords[c].bufY=0;
        }
    }
}

void DmxModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    if (DMX_STYLES.GetCount() == 0) {
        DMX_STYLES.Add("Moving Head Top");
        DMX_STYLES.Add("Moving Head Side");
        DMX_STYLES.Add("Moving Head Bars");
        DMX_STYLES.Add("Moving Head Top Bars");
        DMX_STYLES.Add("Moving Head Side Bars");
        DMX_STYLES.Add("Moving Head 3D");
        DMX_STYLES.Add("Flood Light");
        DMX_STYLES.Add("Skulltronix Skull");
    }

    AddStyleProperties(grid);

    wxPGProperty *p = grid->Append(new wxUIntProperty("# Channels", "DmxChannelCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    if( dmx_style_val != DMX_STYLE_BASIC_FLOOD ) {
        p = grid->Append(new wxUIntProperty("Pan Channel", "DmxPanChannel", pan_channel));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 512);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxUIntProperty("Pan Orientation", "DmxPanOrient", pan_orient));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 360);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxUIntProperty("Pan Deg of Rot", "DmxPanDegOfRot", pan_deg_of_rot));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 1000);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxFloatProperty("Pan Slew Limit (deg/sec)", "DmxPanSlewLimit", pan_slew_limit));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 500);
        p->SetAttribute("Precision", 2);
        p->SetAttribute("Step", 0.1);
        p->SetEditor("SpinCtrl");

        if( dmx_style_val == DMX_STYLE_SKULLTRONIX_SKULL ) {
            p = grid->Append(new wxUIntProperty("Pan Min Limit", "DmxPanMinLimit", pan_min_limit));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 2500);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Pan Max Limit", "DmxPanMaxLimit", pan_max_limit));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 2500);
            p->SetEditor("SpinCtrl");
        }

        p = grid->Append(new wxUIntProperty("Tilt Channel", "DmxTiltChannel", tilt_channel));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 512);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxUIntProperty("Tilt Orientation", "DmxTiltOrient", tilt_orient));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 360);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxUIntProperty("Tilt Deg of Rot", "DmxTiltDegOfRot", tilt_deg_of_rot));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 1000);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxFloatProperty("Tilt Slew Limit (deg/sec)", "DmxTiltSlewLimit", tilt_slew_limit));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 500);
        p->SetAttribute("Precision", 2);
        p->SetAttribute("Step", 0.1);
        p->SetEditor("SpinCtrl");

        if( dmx_style_val == DMX_STYLE_SKULLTRONIX_SKULL ) {
            p = grid->Append(new wxUIntProperty("Tilt Min Limit", "DmxTiltMinLimit", tilt_min_limit));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 2500);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Tilt Max Limit", "DmxTiltMaxLimit", tilt_max_limit));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 2500);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Nod Channel", "DmxNodChannel", nod_channel));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 512);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Nod Orientation", "DmxNodOrient", nod_orient));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 360);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Nod Deg of Rot", "DmxNodDegOfRot", nod_deg_of_rot));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 1000);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Nod Min Limit", "DmxNodMinLimit", nod_min_limit));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 2500);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Nod Max Limit", "DmxNodMaxLimit", nod_max_limit));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 2500);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Jaw Channel", "DmxJawChannel", jaw_channel));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 512);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Jaw Min Limit", "DmxJawMinLimit", jaw_min_limit));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 2500);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Jaw Max Limit", "DmxJawMaxLimit", jaw_max_limit));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 2500);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Eye UD Channel", "DmxEyeUDChannel", eye_ud_channel));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 512);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Eye UD Min Limit", "DmxEyeUDMinLimit", eye_ud_min_limit));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 2500);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Eye UD Max Limit", "DmxEyeUDMaxLimit", eye_ud_max_limit));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 2500);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Eye LR Channel", "DmxEyeLRChannel", eye_lr_channel));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 512);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Eye LR Min Limit", "DmxEyeLRMinLimit", eye_lr_min_limit));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 2500);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Eye LR Max Limit", "DmxEyeLRMaxLimit", eye_lr_max_limit));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 2500);
            p->SetEditor("SpinCtrl");

            p = grid->Append(new wxUIntProperty("Eye Brightness Channel", "DmxEyeBrtChannel", eye_brightness_channel));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 512);
            p->SetEditor("SpinCtrl");
        }
    }

    p = grid->Append(new wxUIntProperty("Red Channel", "DmxRedChannel", red_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Green Channel", "DmxGreenChannel", green_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Blue Channel", "DmxBlueChannel", blue_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    if( dmx_style_val != DMX_STYLE_SKULLTRONIX_SKULL && dmx_style_val != DMX_STYLE_BASIC_FLOOD ) {
        p = grid->Append(new wxUIntProperty("Shutter Channel", "DmxShutterChannel", shutter_channel));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 512);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxIntProperty("Shutter Open Threshold", "DmxShutterOpen", shutter_threshold));
        p->SetAttribute("Min", -255);
        p->SetAttribute("Max", 255);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxFloatProperty("Beam Display Length", "DmxBeamLength", beam_length));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 10);
        p->SetAttribute("Precision", 2);
        p->SetAttribute("Step", 0.1);
        p->SetEditor("SpinCtrl");
    }
}
void DmxModel::AddStyleProperties(wxPropertyGridInterface *grid) {
    grid->Append(new wxEnumProperty("DMX Style", "DmxStyle", DMX_STYLES, dmx_style_val));
}

void DmxModel::DisableUnusedProperties(wxPropertyGridInterface *grid)
{
    // disable string type properties.  Only Single Color White allowed.
    wxPGProperty *p = grid->GetPropertyByName("ModelStringType");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelStringColor");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelFaces");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelDimmingCurves");
    if (p != nullptr) {
        p->Enable(false);
    }

    // Don't remove ModelStates ... these can be used for DMX devices that use a value range to set a colour or behaviour
}

int DmxModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {

    if ("DmxStyle" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxStyle");
        dmx_style_val = event.GetPropertyValue().GetLong();
        if( dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP ) {
            dmx_style = "Moving Head Top";
        } else if( dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE ) {
            dmx_style = "Moving Head Side";
        } else if( dmx_style_val == DMX_STYLE_MOVING_HEAD_BARS ) {
            dmx_style = "Moving Head Bars";
        } else if( dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP_BARS ) {
            dmx_style = "Moving Head TopBars";
        } else if( dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE_BARS ) {
            dmx_style = "Moving Head SideBars";
        } else if( dmx_style_val == DMX_STYLE_MOVING_HEAD_3D ) {
            dmx_style = "Moving Head 3D";
        } else if( dmx_style_val == DMX_STYLE_BASIC_FLOOD ) {
            dmx_style = "Flood Light";
			style_changed = true;
		} else if( dmx_style_val == DMX_STYLE_SKULLTRONIX_SKULL ) {
            dmx_style = "Skulltronix Skull";
            style_changed = true;
        }
        ModelXml->AddAttribute("DmxStyle", dmx_style );
        SetFromXml(ModelXml, zeroBased);
        return 7;
    } else if ("DmxChannelCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("DmxPanChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxPanChannel");
        ModelXml->AddAttribute("DmxPanChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxPanOrient" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxPanOrient");
        ModelXml->AddAttribute("DmxPanOrient", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxPanDegOfRot" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxPanDegOfRot");
        ModelXml->AddAttribute("DmxPanDegOfRot", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxPanSlewLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxPanSlewLimit");
        ModelXml->AddAttribute("DmxPanSlewLimit", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxPanMinLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxPanMinLimit");
        ModelXml->AddAttribute("DmxPanMinLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxPanMaxLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxPanMaxLimit");
        ModelXml->AddAttribute("DmxPanMaxLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxTiltChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxTiltChannel");
        ModelXml->AddAttribute("DmxTiltChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxTiltOrient" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxTiltOrient");
        ModelXml->AddAttribute("DmxTiltOrient", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxTiltDegOfRot" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxTiltDegOfRot");
        ModelXml->AddAttribute("DmxTiltDegOfRot", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxTiltSlewLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxTiltSlewLimit");
        ModelXml->AddAttribute("DmxTiltSlewLimit", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxTiltMinLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxTiltMinLimit");
        ModelXml->AddAttribute("DmxTiltMinLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxTiltMaxLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxTiltMaxLimit");
        ModelXml->AddAttribute("DmxTiltMaxLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxNodChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxNodChannel");
        ModelXml->AddAttribute("DmxNodChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxNodOrient" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxNodOrient");
        ModelXml->AddAttribute("DmxNodOrient", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxNodDegOfRot" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxNodDegOfRot");
        ModelXml->AddAttribute("DmxNodDegOfRot", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxNodMinLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxNodMinLimit");
        ModelXml->AddAttribute("DmxNodMinLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxNodMaxLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxNodMaxLimit");
        ModelXml->AddAttribute("DmxNodMaxLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxJawChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxJawChannel");
        ModelXml->AddAttribute("DmxJawChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxJawMinLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxJawMinLimit");
        ModelXml->AddAttribute("DmxJawMinLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxJawMaxLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxJawMaxLimit");
        ModelXml->AddAttribute("DmxJawMaxLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxEyeBrtChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxEyeBrtChannel");
        ModelXml->AddAttribute("DmxEyeBrtChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxEyeUDChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxEyeUDChannel");
        ModelXml->AddAttribute("DmxEyeUDChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxEyeUDMinLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxEyeUDMinLimit");
        ModelXml->AddAttribute("DmxEyeUDMinLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxEyeUDMaxLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxEyeUDMaxLimit");
        ModelXml->AddAttribute("DmxEyeUDMaxLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxEyeLRChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxEyeLRChannel");
        ModelXml->AddAttribute("DmxEyeLRChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxEyeLRMinLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxEyeLRMinLimit");
        ModelXml->AddAttribute("DmxEyeLRMinLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxEyeLRMaxLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxEyeLRMaxLimit");
        ModelXml->AddAttribute("DmxEyeLRMaxLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxRedChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxRedChannel");
        ModelXml->AddAttribute("DmxRedChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxGreenChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxGreenChannel");
        ModelXml->AddAttribute("DmxGreenChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxBlueChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxBlueChannel");
        ModelXml->AddAttribute("DmxBlueChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxShutterChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxShutterChannel");
        ModelXml->AddAttribute("DmxShutterChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxShutterOpen" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxShutterOpen");
        ModelXml->AddAttribute("DmxShutterOpen", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("DmxBeamLength" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxBeamLength");
        ModelXml->AddAttribute("DmxBeamLength", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    }

    return Model::OnPropertyGridChange(grid, event);
}

void DmxModel::InitModel() {
    DisplayAs = "DMX";
    StringType = "Single Color White";
    parm2 = 1;
    parm3 = 1;

	dmx_style = ModelXml->GetAttribute("DmxStyle", "Moving Head Top");
    if( style_changed ) {
       if( dmx_style == "Skulltronix Skull" ) {
            if (wxMessageBox("Would you like to reset to default Skulltronix settings?", "Restore Defaults?", wxYES_NO | wxCENTER) == wxNO) {
                style_changed = false;
            } else {
                parm1 = 18;
                ModelXml->DeleteAttribute("parm1");
                ModelXml->AddAttribute("parm1", wxString::Format("%d", parm1));
            }
       } else if( dmx_style == "Flood Light" ) {
           pixelSize = 200;
           ModelXml->DeleteAttribute("PixelSize");
           ModelXml->AddAttribute("PixelSize", wxString::Format(wxT("%i"), pixelSize));
           pixelStyle = 3;
           ModelXml->DeleteAttribute("Antialias");
           ModelXml->AddAttribute("Antialias", wxString::Format(wxT("%i"), pixelStyle));
       }
    }

    int numChannels = parm1;
    SetNodeCount(numChannels, 1, rgbOrder);

    int curNode = 0;
    for (int x = 0; x < numChannels; x++) {
        Nodes[curNode]->ActChan = stringStartChan[0] + curNode*GetNodeChannelCount(StringType);
        Nodes[curNode]->StringNum=0;
        // the screenx/screeny positions are used to fake it into giving a bigger selection area
        if( x == 1 ) {
            Nodes[curNode]->Coords[0].screenX = -0.65f;
            Nodes[curNode]->Coords[0].screenY = -1.0f;
        } else if( x == 2 ) {
            Nodes[curNode]->Coords[0].screenX = 0.65f;
            Nodes[curNode]->Coords[0].screenY = 1.0f;
        } else {
            Nodes[curNode]->Coords[0].screenX = 0;
            Nodes[curNode]->Coords[0].screenY = 0;
        }
        Nodes[curNode]->Coords[0].bufX = 0;
        Nodes[curNode]->Coords[0].bufY = 0;
        curNode++;
    }
    SetBufferSize(1,parm1);
    screenLocation.SetRenderSize(1, 1);

	pan_channel = wxAtoi(ModelXml->GetAttribute("DmxPanChannel", "0"));
	pan_orient = wxAtoi(ModelXml->GetAttribute("DmxPanOrient", "0"));
	pan_deg_of_rot = wxAtoi(ModelXml->GetAttribute("DmxPanDegOfRot", "540"));
	pan_slew_limit = wxAtof(ModelXml->GetAttribute("DmxPanSlewLimit", "0"));
	tilt_channel = wxAtoi(ModelXml->GetAttribute("DmxTiltChannel", "0"));
	tilt_orient = wxAtoi(ModelXml->GetAttribute("DmxTiltOrient", "0"));
	tilt_deg_of_rot = wxAtoi(ModelXml->GetAttribute("DmxTiltDegOfRot", "180"));
	tilt_slew_limit = wxAtof(ModelXml->GetAttribute("DmxTiltSlewLimit", "0"));
	red_channel = wxAtoi(ModelXml->GetAttribute("DmxRedChannel", "0"));
	green_channel = wxAtoi(ModelXml->GetAttribute("DmxGreenChannel", "0"));
	blue_channel = wxAtoi(ModelXml->GetAttribute("DmxBlueChannel", "0"));
	shutter_channel = wxAtoi(ModelXml->GetAttribute("DmxShutterChannel", "0"));
	shutter_threshold = wxAtoi(ModelXml->GetAttribute("DmxShutterOpen", "1"));
	beam_length = wxAtof(ModelXml->GetAttribute("DmxBeamLength", "4.0"));

    dmx_style_val = DMX_STYLE_MOVING_HEAD_TOP;
    if( dmx_style == "Moving Head Side" ) {
        dmx_style_val = DMX_STYLE_MOVING_HEAD_SIDE;
    } else if( dmx_style == "Moving Head Bars" ) {
        dmx_style_val = DMX_STYLE_MOVING_HEAD_BARS;
    } else if( dmx_style == "Moving Head TopBars" ) {
        dmx_style_val = DMX_STYLE_MOVING_HEAD_TOP_BARS;
    } else if( dmx_style == "Moving Head SideBars" ) {
        dmx_style_val = DMX_STYLE_MOVING_HEAD_SIDE_BARS;
    } else if( dmx_style == "Moving Head 3D" ) {
        dmx_style_val = DMX_STYLE_MOVING_HEAD_3D;
    } else if( dmx_style == "Flood Light" ) {
        dmx_style_val = DMX_STYLE_BASIC_FLOOD;
    } else if( dmx_style == "Skulltronix Skull" ) {
        dmx_style_val = DMX_STYLE_SKULLTRONIX_SKULL;
        if( !style_changed ) {
            pan_channel = wxAtoi(ModelXml->GetAttribute("DmxPanChannel", "5"));
            pan_orient = wxAtoi(ModelXml->GetAttribute("DmxPanOrient", "90"));
            pan_deg_of_rot = wxAtoi(ModelXml->GetAttribute("DmxPanDegOfRot", "180"));
            pan_slew_limit = wxAtof(ModelXml->GetAttribute("DmxPanSlewLimit", "0"));
            tilt_channel = wxAtoi(ModelXml->GetAttribute("DmxTiltChannel", "11"));
            tilt_orient = wxAtoi(ModelXml->GetAttribute("DmxTiltOrient", "315"));
            tilt_deg_of_rot = wxAtoi(ModelXml->GetAttribute("DmxTiltDegOfRot", "90"));
            tilt_slew_limit = wxAtof(ModelXml->GetAttribute("DmxTiltSlewLimit", "0"));
            red_channel = wxAtoi(ModelXml->GetAttribute("DmxRedChannel", "16"));
            green_channel = wxAtoi(ModelXml->GetAttribute("DmxGreenChannel", "17"));
            blue_channel = wxAtoi(ModelXml->GetAttribute("DmxBlueChannel", "18"));
            tilt_min_limit = wxAtoi(ModelXml->GetAttribute("DmxTiltMinLimit", "442"));
            tilt_max_limit = wxAtoi(ModelXml->GetAttribute("DmxTiltMaxLimit", "836"));
            pan_min_limit = wxAtoi(ModelXml->GetAttribute("DmxPanMinLimit", "250"));
            pan_max_limit = wxAtoi(ModelXml->GetAttribute("DmxPanMaxLimit", "1250"));
            nod_channel = wxAtoi(ModelXml->GetAttribute("DmxNodChannel", "3"));
            nod_orient = wxAtoi(ModelXml->GetAttribute("DmxNodOrient", "331"));
            nod_deg_of_rot = wxAtoi(ModelXml->GetAttribute("DmxNodDegOfRot", "58"));
            nod_min_limit = wxAtoi(ModelXml->GetAttribute("DmxNodMinLimit", "452"));
            nod_max_limit = wxAtoi(ModelXml->GetAttribute("DmxNodMaxLimit", "745"));
            jaw_channel = wxAtoi(ModelXml->GetAttribute("DmxJawChannel", "1"));
            jaw_min_limit = wxAtoi(ModelXml->GetAttribute("DmxJawMinLimit", "500"));
            jaw_max_limit = wxAtoi(ModelXml->GetAttribute("DmxJawMaxLimit", "750"));
            eye_brightness_channel = wxAtoi(ModelXml->GetAttribute("DmxEyeBrtChannel", "15"));
            eye_ud_channel = wxAtoi(ModelXml->GetAttribute("DmxEyeUDChannel", "7"));
            eye_ud_min_limit = wxAtoi(ModelXml->GetAttribute("DmxEyeUDMinLimit", "575"));
            eye_ud_max_limit = wxAtoi(ModelXml->GetAttribute("DmxEyeUDMaxLimit", "1000"));
            eye_lr_channel = wxAtoi(ModelXml->GetAttribute("DmxEyeLRChannel", "9"));
            eye_lr_min_limit = wxAtoi(ModelXml->GetAttribute("DmxEyeLRMinLimit", "499"));
            eye_lr_max_limit = wxAtoi(ModelXml->GetAttribute("DmxEyeLRMaxLimit", "878"));
        } else {
            style_changed = false;
            pan_channel = 5;
            pan_orient = 90;
            pan_deg_of_rot = 180;
            pan_slew_limit = 0;
            tilt_channel = 11;
            tilt_orient = 315;
            tilt_deg_of_rot = 90;
            tilt_slew_limit = 0;
            red_channel = 16;
            green_channel = 17;
            blue_channel = 18;
            tilt_min_limit = 442;
            tilt_max_limit = 836;
            pan_min_limit = 250;
            pan_max_limit = 1250;
            nod_channel = 3;
            nod_orient = 331;
            nod_deg_of_rot = 58;
            nod_min_limit = 452;
            nod_max_limit = 745;
            jaw_channel = 1;
            jaw_min_limit = 500;
            jaw_max_limit = 750;
            eye_ud_channel = 7;
            eye_ud_min_limit = 575;
            eye_ud_max_limit = 1000;
            eye_lr_channel = 9;
            eye_lr_min_limit = 499;
            eye_lr_max_limit = 878;

            ModelXml->DeleteAttribute("DmxPanChannel");
            ModelXml->AddAttribute("DmxPanChannel", wxString::Format("%d", pan_channel));
            ModelXml->DeleteAttribute("DmxPanOrient");
            ModelXml->AddAttribute("DmxPanOrient", wxString::Format("%d", pan_orient));
            ModelXml->DeleteAttribute("DmxPanDegOfRot");
            ModelXml->AddAttribute("DmxPanDegOfRot", wxString::Format("%d", pan_deg_of_rot));
            ModelXml->DeleteAttribute("DmxPanSlewLimit");
            ModelXml->AddAttribute("DmxPanSlewLimit", wxString::Format("%6.4f", pan_slew_limit));
            ModelXml->DeleteAttribute("DmxPanMinLimit");
            ModelXml->AddAttribute("DmxPanMinLimit", wxString::Format("%d", pan_min_limit));
            ModelXml->DeleteAttribute("DmxPanMaxLimit");
            ModelXml->AddAttribute("DmxPanMaxLimit", wxString::Format("%d", pan_max_limit));
            ModelXml->DeleteAttribute("DmxTiltChannel");
            ModelXml->AddAttribute("DmxTiltChannel", wxString::Format("%d",tilt_channel));
            ModelXml->DeleteAttribute("DmxTiltOrient");
            ModelXml->AddAttribute("DmxTiltOrient", wxString::Format("%d", tilt_orient));
            ModelXml->DeleteAttribute("DmxTiltDegOfRot");
            ModelXml->AddAttribute("DmxTiltDegOfRot", wxString::Format("%d", tilt_deg_of_rot));
            ModelXml->DeleteAttribute("DmxTiltSlewLimit");
            ModelXml->AddAttribute("DmxTiltSlewLimit", wxString::Format("%6.4f", tilt_slew_limit));
            ModelXml->DeleteAttribute("DmxTiltMinLimit");
            ModelXml->AddAttribute("DmxTiltMinLimit", wxString::Format("%d", tilt_min_limit));
            ModelXml->DeleteAttribute("DmxTiltMaxLimit");
            ModelXml->AddAttribute("DmxTiltMaxLimit", wxString::Format("%d", tilt_max_limit));
            ModelXml->DeleteAttribute("DmxNodChannel");
            ModelXml->AddAttribute("DmxNodChannel", wxString::Format("%d", nod_channel));
            ModelXml->DeleteAttribute("DmxNodOrient");
            ModelXml->AddAttribute("DmxNodOrient", wxString::Format("%d", nod_orient));
            ModelXml->DeleteAttribute("DmxNodDegOfRot");
            ModelXml->AddAttribute("DmxNodDegOfRot", wxString::Format("%d", nod_deg_of_rot));
            ModelXml->DeleteAttribute("DmxNodMinLimit");
            ModelXml->AddAttribute("DmxNodMinLimit", wxString::Format("%d", nod_min_limit));
            ModelXml->DeleteAttribute("DmxNodMaxLimit");
            ModelXml->AddAttribute("DmxNodMaxLimit", wxString::Format("%d", nod_max_limit));
            ModelXml->DeleteAttribute("DmxJawChannel");
            ModelXml->AddAttribute("DmxJawChannel", wxString::Format("%d", jaw_channel));
            ModelXml->DeleteAttribute("DmxJawMinLimit");
            ModelXml->AddAttribute("DmxJawMinLimit", wxString::Format("%d", jaw_min_limit));
            ModelXml->DeleteAttribute("DmxJawMaxLimit");
            ModelXml->AddAttribute("DmxJawMaxLimit", wxString::Format("%d", jaw_max_limit));
            ModelXml->DeleteAttribute("DmxEyeBrtChannel");
            ModelXml->AddAttribute("DmxEyeBrtChannel", wxString::Format("%d", eye_brightness_channel));
            ModelXml->DeleteAttribute("DmxEyeUDChannel");
            ModelXml->AddAttribute("DmxEyeUDChannel", wxString::Format("%d", eye_ud_channel));
            ModelXml->DeleteAttribute("DmxEyeUDMinLimit");
            ModelXml->AddAttribute("DmxEyeUDMinLimit", wxString::Format("%d", eye_ud_min_limit));
            ModelXml->DeleteAttribute("DmxEyeUDMaxLimit");
            ModelXml->AddAttribute("DmxEyeUDMaxLimit", wxString::Format("%d", eye_ud_max_limit));
            ModelXml->DeleteAttribute("DmxEyeLRChannel");
            ModelXml->AddAttribute("DmxEyeLRChannel", wxString::Format("%d", eye_lr_channel));
            ModelXml->DeleteAttribute("DmxEyeLRMinLimit");
            ModelXml->AddAttribute("DmxEyeLRMinLimit", wxString::Format("%d", eye_lr_min_limit));
            ModelXml->DeleteAttribute("DmxEyeLRMaxLimit");
            ModelXml->AddAttribute("DmxEyeLRMaxLimit", wxString::Format("%d", eye_lr_max_limit));
            ModelXml->DeleteAttribute("DmxRedChannel");
            ModelXml->AddAttribute("DmxRedChannel", wxString::Format("%d", red_channel));
            ModelXml->DeleteAttribute("DmxGreenChannel");
            ModelXml->AddAttribute("DmxGreenChannel", wxString::Format("%d", green_channel));
            ModelXml->DeleteAttribute("DmxBlueChannel");
            ModelXml->AddAttribute("DmxBlueChannel", wxString::Format("%d", blue_channel));
        }
    }
}

void DmxModel::DisplayEffectOnWindow(ModelPreview* preview, double pointSize)
{
    bool success = preview->StartDrawing(pointSize);

    if(success) {
        DrawGLUtils::xlAccumulator va(maxVertexCount);

        float sx,sy;
        xlColor color, proxy;
        int w, h;

        GetModelScreenLocation().PrepareToDraw();

        va.PreAlloc(maxVertexCount);

        preview->GetSize(&w, &h);

        sx=w/2;
        sy=h/2;

        if( dmx_style_val == DMX_STYLE_SKULLTRONIX_SKULL ) {
            DrawSkullModelOnWindow(preview, va, nullptr, sx, sy, true);
        } else if( dmx_style_val == DMX_STYLE_BASIC_FLOOD ) {
            DrawFloodOnWindow(preview, va, nullptr, sx, sy, true);
        } else {
            DrawModelOnWindow(preview, va, nullptr, sx, sy, true);
        }

        DrawGLUtils::Draw(va);

        preview->EndDrawing();

    }

}

// display model using colors
void DmxModel::DisplayModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, bool allowSelected) {
    float sx,sy;
    int w, h;
    preview->GetVirtualCanvasSize(w, h);

    GetModelScreenLocation().PrepareToDraw();

    va.PreAlloc(maxVertexCount);

    sx=0;
    sy=0;
    GetModelScreenLocation().TranslatePoint(sx, sy);

    if( dmx_style_val == DMX_STYLE_SKULLTRONIX_SKULL ) {
        DrawSkullModelOnWindow(preview, va, c, sx, sy, !allowSelected);
    } else if( dmx_style_val == DMX_STYLE_BASIC_FLOOD ) {
        DrawFloodOnWindow(preview, va, c, sx, sy, !allowSelected);
    } else {
        DrawModelOnWindow(preview, va, c, sx, sy, !allowSelected);
    }

    if (Selected && c != nullptr && allowSelected) {
        GetModelScreenLocation().DrawHandles(va);
    }
}

void DmxModel::DrawFloodOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active)
{
    size_t NodeCount=Nodes.size();

    if( red_channel > NodeCount ||
        green_channel > NodeCount ||
        blue_channel > NodeCount )
    {
        return;
    }

	xlColor color;
	xlColor ecolor;
	xlColor beam_color(xlWHITE);
	if (c != nullptr) {
		color = *c;
		ecolor = *c;
	}

	int trans = color == xlBLACK ? blackTransparency : transparency;
	if (red_channel > 0 && green_channel > 0 && blue_channel > 0) {
		xlColor proxy;
		Nodes[red_channel - 1]->GetColor(proxy);
		beam_color.red = proxy.red;
		Nodes[green_channel - 1]->GetColor(proxy);
		beam_color.green = proxy.red;
		Nodes[blue_channel - 1]->GetColor(proxy);
		beam_color.blue = proxy.red;
	}
	if (!active) {
		beam_color = color;
	}
	if (beam_color.red == 0 && beam_color.green == 0 && beam_color.blue == 0) {
		beam_color = xlWHITE;
	}
	ApplyTransparency(beam_color, trans);
    ApplyTransparency(ecolor, pixelStyle == 2 ? trans : 100);

	float min_size = (float)(std::min(GetWidth(preview), GetHeight(preview)));
    va.AddTrianglesCircle(sx, sy, min_size/2.0f, beam_color, ecolor);
    va.Finish(GL_TRIANGLES);
}

void DmxModel::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active)
{
    static wxStopWatch sw;
    float angle, pan_angle, pan_angle_raw, tilt_angle, angle1, angle2, beam_length_displayed;
    int x1, x2, y1, y2;
    size_t NodeCount=Nodes.size();

    if( pan_channel > NodeCount ||
        tilt_channel > NodeCount ||
        red_channel > NodeCount ||
        green_channel > NodeCount ||
        blue_channel > NodeCount )
    {
        return;
    }

    xlColor ccolor(xlWHITE);
    xlColor pnt_color(xlRED);
    xlColor beam_color(xlWHITE);
    xlColor marker_color(xlBLACK);
    xlColor black(xlBLACK);
    xlColor base_color(200, 200, 200);
    xlColor base_color2(150, 150, 150);
    xlColor color;
    if (c != nullptr) {
        color = *c;
    }

    int dmx_size = ((BoxedScreenLocation)screenLocation).GetScaleX() * screenLocation.previewW;
    float radius = (float)(dmx_size) / 2.0f;
    xlColor color_angle;

    int trans = color == xlBLACK ? blackTransparency : transparency;
    if( red_channel > 0 && green_channel > 0 && blue_channel > 0 ) {
        xlColor proxy;
        Nodes[red_channel-1]->GetColor(proxy);
        beam_color.red = proxy.red;
        Nodes[green_channel-1]->GetColor(proxy);
        beam_color.green = proxy.red;
        Nodes[blue_channel-1]->GetColor(proxy);
        beam_color.blue = proxy.red;
    }
    if( (beam_color.red == 0 && beam_color.green == 0 && beam_color.blue == 0) || !active ) {
        beam_color = xlWHITE;
    } else {
        marker_color = beam_color;
    }
    ApplyTransparency(beam_color, trans);
    ApplyTransparency(ccolor, trans);
    ApplyTransparency(base_color, trans);
    ApplyTransparency(base_color2, trans);
    ApplyTransparency(pnt_color, trans);

    // retrieve the model state
    float old_pan_angle = 0.0f;
    float old_tilt_angle = 0.0f;
    long old_ms = 0;
    float rot_angle = (float)(((BoxedScreenLocation)screenLocation).GetRotation());

    std::vector<std::string> old_state = GetModelState();
    if( old_state.size() > 0  && active ) {
        old_ms = std::atol(old_state[0].c_str());
        old_pan_angle = std::atof(old_state[1].c_str());
        old_tilt_angle = std::atof(old_state[2].c_str());
    }

    if( pan_channel > 0 && active) {
        Nodes[pan_channel-1]->GetColor(color_angle);
        pan_angle = (color_angle.red / 255.0f) * pan_deg_of_rot + pan_orient;
    } else {
        pan_angle = pan_orient;
    }

    long ms = sw.Time();
    long time_delta = ms - old_ms;

    if( time_delta != 0 && old_state.size() > 0 && active ) {
        // pan slew limiting
        if( pan_slew_limit > 0.0f ) {
            float slew_limit = pan_slew_limit * (float)time_delta / 1000.0f;
            float pan_delta = pan_angle - old_pan_angle;
            if( std::abs(pan_delta) > slew_limit ) {
                if( pan_delta < 0 ) {
                    slew_limit *= -1.0f;
                }
                pan_angle =  old_pan_angle + slew_limit;
            }
        }
    }

    pan_angle_raw = pan_angle;
    if( tilt_channel > 0 && active ) {
        Nodes[tilt_channel-1]->GetColor(color_angle);
        tilt_angle = (color_angle.red / 255.0f) * tilt_deg_of_rot + tilt_orient;
    } else {
        tilt_angle = tilt_orient;
    }

    if( time_delta != 0 && old_state.size() > 0 && active ) {
        // tilt slew limiting
        if( tilt_slew_limit > 0.0f ) {
            float slew_limit = tilt_slew_limit * (float)time_delta / 1000.0f;
            float tilt_delta = tilt_angle - old_tilt_angle;
            if( std::abs(tilt_delta) > slew_limit ) {
                if( tilt_delta < 0 ) {
                    slew_limit *= -1.0f;
                }
                tilt_angle =  old_tilt_angle + slew_limit;
            }
        }
    }

    // Determine if we need to flip the beam
    int tilt_pos = (int)(RenderBuffer::cos(ToRadians(tilt_angle))*radius*0.8);
    if( tilt_pos < 0 ) {
        if( pan_angle >= 180.0f ) {
            pan_angle -= 180.0f;
        } else {
            pan_angle += 180.0f;
        }
        tilt_pos *= -1;
    }

    if( dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP || dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP_BARS ) {
        angle = pan_angle;
    } else {
        angle = tilt_angle;
    }

    // save the model state
    std::vector<std::string> state;
    state.push_back(std::to_string(ms));
    state.push_back(std::to_string(pan_angle_raw));
    state.push_back(std::to_string(tilt_angle));
    SaveModelState(state);

    float sf = 12.0f;
    float scale = radius / sf;

    int bars_deltax = (int)(scale * sf * 1.0f);
    int bars_deltay = (int)(scale * sf * 1.1f);

    if( dmx_style_val == DMX_STYLE_MOVING_HEAD_BARS ||
        dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP_BARS ||
        dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE_BARS ) {
            scale /= 2.0f;
            tilt_pos /= 2;
    }

    float beam_width = 30.0f;
    beam_length_displayed = scale * sf * beam_length;
    angle1 = angle - beam_width / 2.0f;
    angle2 = angle + beam_width / 2.0f;
    if( angle1 < 0.0f ) {
        angle1 += 360.0f;
    }
    if( angle2 > 360.f ) {
        angle2 -= 360.0f;
    }
    x1 = (int)(RenderBuffer::cos(ToRadians(angle1))*beam_length_displayed);
    y1 = (int)(RenderBuffer::sin(ToRadians(angle1))*beam_length_displayed);
    x2 = (int)(RenderBuffer::cos(ToRadians(angle2))*beam_length_displayed);
    y2 = (int)(RenderBuffer::sin(ToRadians(angle2))*beam_length_displayed);

    // determine if shutter is open for heads that support it
    bool shutter_open = true;
    if( shutter_channel > 0 && active ) {
        xlColor proxy;
        Nodes[shutter_channel-1]->GetColor(proxy);
        int shutter_value = proxy.red;
        if( shutter_value >= 0 ) {
            shutter_open = shutter_value >= shutter_threshold;
        } else {
            shutter_open = shutter_value <= std::abs(shutter_threshold);
        }
    }

    // Draw the light beam
    if( dmx_style_val != DMX_STYLE_MOVING_HEAD_BARS && dmx_style_val != DMX_STYLE_MOVING_HEAD_3D && shutter_open ) {
        va.AddVertex(sx, sy, beam_color);
        ApplyTransparency(beam_color, 100);
        va.AddVertex(sx+x1, sy+y1, beam_color);
        va.AddVertex(sx+x2, sy+y2, beam_color);
    }

    if( dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP || dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP_BARS) {
        va.AddTrianglesCircle(sx, sy, scale*sf, ccolor, ccolor);

        // draw angle line
        dmxPoint p1(0, -1, sx, sy, scale, angle);
        dmxPoint p2(12, -1, sx, sy, scale, angle);
        dmxPoint p3(12, 1, sx, sy, scale, angle);
        dmxPoint p4(0, 1, sx, sy, scale, angle);

        va.AddVertex(p1.x, p1.y, pnt_color);
        va.AddVertex(p2.x, p2.y, pnt_color);
        va.AddVertex(p3.x, p3.y, pnt_color);

        va.AddVertex(p1.x, p1.y, pnt_color);
        va.AddVertex(p3.x, p3.y, pnt_color);
        va.AddVertex(p4.x, p4.y, pnt_color);

        // draw tilt marker
        dmxPoint marker(tilt_pos, 0, sx, sy, 1.0, angle);
        va.AddTrianglesCircle(marker.x, marker.y, scale*sf*0.22, black, black);
        va.AddTrianglesCircle(marker.x, marker.y, scale*sf*0.20, marker_color, marker_color);
    } else if( dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE || dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE_BARS) {
        // draw head
        dmxPoint p1(12, -13, sx, sy, scale, angle);
        dmxPoint p2(12, +13, sx, sy, scale, angle);
        dmxPoint p3(-12, +10, sx, sy, scale, angle);
        dmxPoint p4(-15, +5, sx, sy, scale, angle);
        dmxPoint p5(-15, -5, sx, sy, scale, angle);
        dmxPoint p6(-12, -10, sx, sy, scale, angle);

        va.AddVertex(p1.x, p1.y, ccolor);
        va.AddVertex(p2.x, p2.y, ccolor);
        va.AddVertex(p6.x, p6.y, ccolor);

        va.AddVertex(p2.x, p2.y, ccolor);
        va.AddVertex(p3.x, p3.y, ccolor);
        va.AddVertex(p6.x, p6.y, ccolor);

        va.AddVertex(p3.x, p3.y, ccolor);
        va.AddVertex(p5.x, p5.y, ccolor);
        va.AddVertex(p6.x, p6.y, ccolor);

        va.AddVertex(p3.x, p3.y, ccolor);
        va.AddVertex(p4.x, p4.y, ccolor);
        va.AddVertex(p5.x, p5.y, ccolor);

        // draw base
        va.AddTrianglesCircle(sx, sy, scale*sf*0.6, base_color, base_color);
        va.AddRect(sx-scale*sf*0.6, sy, sx+scale*sf*0.6, sy-scale*sf*2, base_color);

        // draw pan marker
        dmxPoint p7(7, 2, sx, sy, scale, pan_angle);
        dmxPoint p8(7, -2, sx, sy, scale, pan_angle);
        va.AddVertex(sx, sy, marker_color);
        va.AddVertex(p7.x, p7.y, marker_color);
        va.AddVertex(p8.x, p8.y, marker_color);
    }
    if( dmx_style_val == DMX_STYLE_MOVING_HEAD_BARS ||
        dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP_BARS ||
        dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE_BARS ) {

        if( dmx_style_val == DMX_STYLE_MOVING_HEAD_BARS ) {
            bars_deltax = 0;
        }
        // draw the bars
        xlColor proxy;
        xlColor red(xlRED);
        xlColor green(xlGREEN);
        xlColor blue(xlBLUE);
        xlColor pink(255,51,255);
        xlColor turqoise(64,224,208);
        ApplyTransparency(red, trans);
        ApplyTransparency(green, trans);
        ApplyTransparency(blue, trans);
        ApplyTransparency(pink, trans);
        ApplyTransparency(turqoise, trans);
        float offsetx = 0.0f;
        int stepy = (int)(radius * 0.15f);
        int gapy = (int)(radius * 0.1f);
        if( gapy < 1 ) gapy = 1;
        va.AddRect(sx+bars_deltax-gapy-2, sy+bars_deltay+gapy+2, sx+bars_deltax+radius+gapy+2, sy+bars_deltay-(stepy+gapy)*(NodeCount-1)-stepy-gapy-2, ccolor);
        va.AddRect(sx+bars_deltax-gapy, sy+bars_deltay+gapy, sx+bars_deltax+radius+gapy, sy+bars_deltay-(stepy+gapy)*(NodeCount-1)-stepy-gapy, black);
        for( int i = 1; i <= NodeCount; ++i ) {
            Nodes[i-1]->GetColor(proxy);
            float val = (float)proxy.red;
            offsetx = (val / 255.0 * radius);
            if( i == pan_channel ) {
                proxy = pink;
            } else if( i == tilt_channel ) {
                proxy = turqoise;
            } else if( i == red_channel ) {
                proxy = red;
            } else if( i == green_channel ) {
                proxy = green;
            } else if( i == blue_channel ) {
                proxy = blue;
            } else {
                proxy = ccolor;
            }
            va.AddRect(sx+bars_deltax, sy+bars_deltay-(stepy+gapy)*(i-1), sx+bars_deltax+offsetx, sy+bars_deltay-(stepy+gapy)*(i-1)-stepy, proxy);
        }
    }

    if( dmx_style_val == DMX_STYLE_MOVING_HEAD_3D ) {
        xlColor beam_color_end(beam_color);
        ApplyTransparency(beam_color_end, 100);

        while (pan_angle_raw > 360.0f ) pan_angle_raw -= 360.0f;
        pan_angle_raw = 360.0f - pan_angle_raw;
        bool facing_right = pan_angle_raw <= 90.0f || pan_angle_raw >= 270.0f;

        float combined_angle = tilt_angle + rot_angle;
        if( shutter_open ) {
            dmxPoint3 p1(beam_length_displayed,-5,-5, sx, sy, scale, pan_angle_raw, combined_angle);
            dmxPoint3 p2(beam_length_displayed,-5,5, sx, sy, scale, pan_angle_raw, combined_angle);
            dmxPoint3 p3(beam_length_displayed,5,-5, sx, sy, scale, pan_angle_raw, combined_angle);
            dmxPoint3 p4(beam_length_displayed,5,5, sx, sy, scale, pan_angle_raw, combined_angle);
            dmxPoint3 p0(0,0,0, sx, sy, scale, pan_angle_raw, combined_angle);


            if( facing_right ) {
                va.AddVertex(p2.x, p2.y, beam_color_end);
                va.AddVertex(p4.x, p4.y, beam_color_end);
                va.AddVertex(p0.x, p0.y, beam_color);
            } else {
                va.AddVertex(p1.x, p1.y, beam_color_end);
                va.AddVertex(p3.x, p3.y, beam_color_end);
                va.AddVertex(p0.x, p0.y, beam_color);
            }

            va.AddVertex(p1.x, p1.y, beam_color_end);
            va.AddVertex(p2.x, p2.y, beam_color_end);
            va.AddVertex(p0.x, p0.y, beam_color);

            va.AddVertex(p3.x, p3.y, beam_color_end);
            va.AddVertex(p4.x, p4.y, beam_color_end);
            va.AddVertex(p0.x, p0.y, beam_color);

            if( !facing_right ) {
                va.AddVertex(p2.x, p2.y, beam_color_end);
                va.AddVertex(p4.x, p4.y, beam_color_end);
                va.AddVertex(p0.x, p0.y, beam_color);
            } else {
                va.AddVertex(p1.x, p1.y, beam_color_end);
                va.AddVertex(p3.x, p3.y, beam_color_end);
                va.AddVertex(p0.x, p0.y, beam_color);
            }
        }

        if( facing_right ) {
            Draw3DDMXBaseRight(va, base_color, sx, sy, scale, pan_angle_raw, rot_angle);
            Draw3DDMXHead(va, base_color2, sx, sy, scale, pan_angle_raw, combined_angle);
            Draw3DDMXBaseLeft(va, base_color, sx, sy, scale, pan_angle_raw, rot_angle);
        } else {
            Draw3DDMXBaseLeft(va, base_color, sx, sy, scale, pan_angle_raw, rot_angle);
            Draw3DDMXHead(va, base_color2, sx, sy, scale, pan_angle_raw, combined_angle);
            Draw3DDMXBaseRight(va, base_color, sx, sy, scale, pan_angle_raw, rot_angle);
        }
    }

    va.Finish(GL_TRIANGLES);
}

int DmxModel::GetChannelValue( int channel )
{
    xlColor color_angle;
    int lsb = 0;
    int msb = 0;
    if( dmx_style_val == DMX_STYLE_SKULLTRONIX_SKULL ) {
        Nodes[channel]->GetColor(color_angle);
        msb = color_angle.red;
        Nodes[channel+1]->GetColor(color_angle);
        lsb = color_angle.red;
    } else {
        Nodes[channel]->GetColor(color_angle);
        lsb = color_angle.red;
   }
   return ((msb << 8) | lsb);
}

void DmxModel::DrawSkullModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active)
{
    float pan_angle, pan_angle_raw, tilt_angle, nod_angle, jaw_pos, eye_x, eye_y;
    float jaw_range_of_motion = -4.0f;
    float eye_range_of_motion = 3.8f;
    int channel_value;
    size_t NodeCount=Nodes.size();
    bool beam_off = false;

    if( pan_channel > NodeCount ||
        tilt_channel > NodeCount ||
        red_channel > NodeCount ||
        green_channel > NodeCount ||
        blue_channel > NodeCount )
    {
        return;
    }

    xlColor ccolor(xlWHITE);
    xlColor pnt_color(xlRED);
    xlColor eye_color(xlWHITE);
    xlColor marker_color(xlBLACK);
    xlColor black(xlBLACK);
    xlColor base_color(200, 200, 200);
    xlColor base_color2(150, 150, 150);
    xlColor color;
    if (c != nullptr) {
        color = *c;
    }

    int dmx_size = ((BoxedScreenLocation)screenLocation).GetScaleX() * screenLocation.previewW;
    float radius = (float)(dmx_size) / 2.0f;
    xlColor color_angle;

    int trans = color == xlBLACK ? blackTransparency : transparency;
    if( red_channel > 0 && green_channel > 0 && blue_channel > 0 ) {
        xlColor proxy;
        Nodes[red_channel-1]->GetColor(proxy);
        eye_color.red = proxy.red;
        Nodes[green_channel-1]->GetColor(proxy);
        eye_color.green = proxy.red;
        Nodes[blue_channel-1]->GetColor(proxy);
        eye_color.blue = proxy.red;
    }
    if( (eye_color.red == 0 && eye_color.green == 0 && eye_color.blue == 0) || !active ) {
        eye_color = xlWHITE;
        beam_off = true;
    } else {
        ApplyTransparency(eye_color, trans);
        marker_color = eye_color;
    }
    ApplyTransparency(ccolor, trans);
    ApplyTransparency(base_color, trans);
    ApplyTransparency(base_color2, trans);
    ApplyTransparency(pnt_color, trans);

    if( pan_channel > 0 ) {
        channel_value = GetChannelValue(pan_channel-1);
        pan_angle = ((channel_value - pan_min_limit) / (double)(pan_max_limit - pan_min_limit)) * pan_deg_of_rot + pan_orient;
    } else {
        pan_angle = 0.0f;
    }
    pan_angle_raw = pan_angle;
    if( tilt_channel > 0 ) {
        channel_value = GetChannelValue(tilt_channel-1);
        tilt_angle = (1.0 - ((channel_value - tilt_min_limit) / (double)(tilt_max_limit - tilt_min_limit))) * tilt_deg_of_rot + tilt_orient;
    } else {
        tilt_angle = 0.0f;
    }
    if( nod_channel > 0 ) {
        channel_value = GetChannelValue(nod_channel-1);
        nod_angle = (1.0 - ((channel_value - nod_min_limit) / (double)(nod_max_limit - nod_min_limit))) * nod_deg_of_rot + nod_orient;
    } else {
        nod_angle = 0.0f;
    }
    if( jaw_channel > 0 ) {
        channel_value = GetChannelValue(jaw_channel-1);
        jaw_pos = ((channel_value - jaw_min_limit) / (double)(jaw_max_limit - jaw_min_limit)) * jaw_range_of_motion - 0.5f;
    } else {
        jaw_pos = -0.5f;
    }
    if( eye_lr_channel > 0 ) {
        channel_value = GetChannelValue(eye_lr_channel-1);
        eye_x = (1.0 - ((channel_value - eye_lr_min_limit) / (double)(eye_lr_max_limit - eye_lr_min_limit))) * eye_range_of_motion - eye_range_of_motion/2.0;
    } else {
        eye_x = 0.0f;
    }
    if( eye_ud_channel > 0 ) {
        channel_value = GetChannelValue(eye_ud_channel-1);
        eye_y = ((channel_value - eye_ud_min_limit) / (double)(eye_ud_max_limit - eye_ud_min_limit)) * eye_range_of_motion - eye_range_of_motion/2.0;
    } else {
        eye_y = 0.0f;
    }

    if( !active ) {
        pan_angle = 0.5f * 180 + 90;
        tilt_angle = 0.5f * 90 + 315;
        nod_angle = 0.5f * 58 + 331;
        jaw_pos = -0.5f;
        eye_x = 0.5f * eye_range_of_motion - eye_range_of_motion/2.0;
        eye_y = 0.5f * eye_range_of_motion - eye_range_of_motion/2.0;
    }

    float sf = 12.0f;
    float scale = radius / sf;

    // Create Head
    dmxPoint3 p1(-7.5f, 13.7f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p2(7.5f, 13.7f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p3(13.2f, 6.0f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p8(-13.2f, 6.0f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p4(9, -11.4f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p7(-9, -11.4f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p5(6.3f, -16, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p6(-6.3f, -16, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);

    dmxPoint3 p9(0, 3.5f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p10(-2.5f, -1.7f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p11(2.5f, -1.7f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);

    dmxPoint3 p14(0, -6.5f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p12(-6, -6.5f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p16(6, -6.5f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p13(-3, -11.4f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p15(3, -11.4f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);

    // Create Back of Head
    dmxPoint3 p1b(-7.5f, 13.7f, -3, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p2b(7.5f, 13.7f, -3, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p3b(13.2f, 6.0f, -3, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p8b(-13.2f, 6.0f, -3, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p4b(9, -11.4f, -3, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p7b(-9, -11.4f, -3, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p5b(6.3f, -16, -3, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p6b(-6.3f, -16, -3, sx, sy, scale, pan_angle, tilt_angle, nod_angle);

    // Create Lower Mouth
    dmxPoint3 p4m(9, -11.4f+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p7m(-9, -11.4f+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p5m(6.3f, -16+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p6m(-6.3f, -16+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p14m(0, -6.5f+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p12m(-6, -6.5f+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p16m(6, -6.5f+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p13m(-3, -11.4f+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p15m(3, -11.4f+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);

    // Create Eyes
    dmxPoint3 left_eye_socket(-5, 7.5f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 right_eye_socket(5, 7.5f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 left_eye(-5+eye_x, 7.5f+eye_y, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 right_eye(5+eye_x, 7.5f+eye_y, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);

    // Draw Back of Head
    va.AddVertex(p1.x, p1.y, base_color2);
    va.AddVertex(p1b.x, p1b.y, base_color2);
    va.AddVertex(p2.x, p2.y, base_color2);
    va.AddVertex(p2b.x, p2b.y, base_color2);
    va.AddVertex(p1b.x, p1b.y, base_color2);
    va.AddVertex(p2.x, p2.y, base_color2);

    va.AddVertex(p2.x, p2.y, base_color);
    va.AddVertex(p2b.x, p2b.y, base_color);
    va.AddVertex(p3.x, p3.y, base_color);
    va.AddVertex(p3b.x, p3b.y, base_color);
    va.AddVertex(p2b.x, p2b.y, base_color);
    va.AddVertex(p3.x, p3.y, base_color);

    va.AddVertex(p3.x, p3.y, base_color2);
    va.AddVertex(p3b.x, p3b.y, base_color2);
    va.AddVertex(p4.x, p4.y, base_color2);
    va.AddVertex(p4b.x, p4b.y, base_color2);
    va.AddVertex(p3b.x, p3b.y, base_color2);
    va.AddVertex(p4.x, p4.y, base_color2);

    va.AddVertex(p4.x, p4.y, base_color);
    va.AddVertex(p4b.x, p4b.y, base_color);
    va.AddVertex(p5.x, p5.y, base_color);
    va.AddVertex(p5b.x, p5b.y, base_color);
    va.AddVertex(p4b.x, p4b.y, base_color);
    va.AddVertex(p5.x, p5.y, base_color);

    va.AddVertex(p5.x, p5.y, base_color2);
    va.AddVertex(p5b.x, p5b.y, base_color2);
    va.AddVertex(p6.x, p6.y, base_color2);
    va.AddVertex(p6b.x, p6b.y, base_color2);
    va.AddVertex(p5b.x, p5b.y, base_color2);
    va.AddVertex(p6.x, p6.y, base_color2);

    va.AddVertex(p6.x, p6.y, base_color);
    va.AddVertex(p6b.x, p6b.y, base_color);
    va.AddVertex(p7.x, p7.y, base_color);
    va.AddVertex(p7b.x, p7b.y, base_color);
    va.AddVertex(p6b.x, p6b.y, base_color);
    va.AddVertex(p7.x, p7.y, base_color);

    va.AddVertex(p7.x, p7.y, base_color2);
    va.AddVertex(p7b.x, p7b.y, base_color2);
    va.AddVertex(p8.x, p8.y, base_color2);
    va.AddVertex(p8b.x, p8b.y, base_color2);
    va.AddVertex(p7b.x, p7b.y, base_color2);
    va.AddVertex(p8.x, p8.y, base_color2);

    va.AddVertex(p8.x, p8.y, base_color);
    va.AddVertex(p8b.x, p8b.y, base_color);
    va.AddVertex(p1.x, p1.y, base_color);
    va.AddVertex(p1b.x, p1b.y, base_color);
    va.AddVertex(p8b.x, p8b.y, base_color);
    va.AddVertex(p1.x, p1.y, base_color);

    // Draw Front of Head
    va.AddVertex(p1.x, p1.y, ccolor);
    va.AddVertex(p2.x, p2.y, ccolor);
    va.AddVertex(p9.x, p9.y, ccolor);

    va.AddVertex(p2.x, p2.y, ccolor);
    va.AddVertex(p9.x, p9.y, ccolor);
    va.AddVertex(p11.x, p11.y, ccolor);

    va.AddVertex(p1.x, p1.y, ccolor);
    va.AddVertex(p9.x, p9.y, ccolor);
    va.AddVertex(p10.x, p10.y, ccolor);

    va.AddVertex(p1.x, p1.y, ccolor);
    va.AddVertex(p8.x, p8.y, ccolor);
    va.AddVertex(p10.x, p10.y, ccolor);

    va.AddVertex(p2.x, p2.y, ccolor);
    va.AddVertex(p3.x, p3.y, ccolor);
    va.AddVertex(p11.x, p11.y, ccolor);

    va.AddVertex(p8.x, p8.y, ccolor);
    va.AddVertex(p10.x, p10.y, ccolor);
    va.AddVertex(p12.x, p12.y, ccolor);

    va.AddVertex(p3.x, p3.y, ccolor);
    va.AddVertex(p11.x, p11.y, ccolor);
    va.AddVertex(p16.x, p16.y, ccolor);

    va.AddVertex(p7.x, p7.y, ccolor);
    va.AddVertex(p8.x, p8.y, ccolor);
    va.AddVertex(p12.x, p12.y, ccolor);

    va.AddVertex(p3.x, p3.y, ccolor);
    va.AddVertex(p4.x, p4.y, ccolor);
    va.AddVertex(p16.x, p16.y, ccolor);

    va.AddVertex(p10.x, p10.y, ccolor);
    va.AddVertex(p12.x, p12.y, ccolor);
    va.AddVertex(p14.x, p14.y, ccolor);

    va.AddVertex(p10.x, p10.y, ccolor);
    va.AddVertex(p11.x, p11.y, ccolor);
    va.AddVertex(p14.x, p14.y, ccolor);

    va.AddVertex(p11.x, p11.y, ccolor);
    va.AddVertex(p14.x, p14.y, ccolor);
    va.AddVertex(p16.x, p16.y, ccolor);

    va.AddVertex(p12.x, p12.y, ccolor);
    va.AddVertex(p13.x, p13.y, ccolor);
    va.AddVertex(p14.x, p14.y, ccolor);

    va.AddVertex(p14.x, p14.y, ccolor);
    va.AddVertex(p15.x, p15.y, ccolor);
    va.AddVertex(p16.x, p16.y, ccolor);

    // Draw Lower Mouth
    va.AddVertex(p4m.x, p4m.y, ccolor);
    va.AddVertex(p6m.x, p6m.y, ccolor);
    va.AddVertex(p7m.x, p7m.y, ccolor);

    va.AddVertex(p4m.x, p4m.y, ccolor);
    va.AddVertex(p5m.x, p5m.y, ccolor);
    va.AddVertex(p6m.x, p6m.y, ccolor);

    va.AddVertex(p7m.x, p7m.y, ccolor);
    va.AddVertex(p12m.x, p12m.y, ccolor);
    va.AddVertex(p13m.x, p13m.y, ccolor);

    va.AddVertex(p13m.x, p13m.y, ccolor);
    va.AddVertex(p14m.x, p14m.y, ccolor);
    va.AddVertex(p15m.x, p15m.y, ccolor);

    va.AddVertex(p4m.x, p4m.y, ccolor);
    va.AddVertex(p15m.x, p15m.y, ccolor);
    va.AddVertex(p16m.x, p16m.y, ccolor);

    // Draw Eyes
    va.AddTrianglesCircle(left_eye_socket.x, left_eye_socket.y, scale*sf*0.25, black, black);
    va.AddTrianglesCircle(right_eye_socket.x, right_eye_socket.y, scale*sf*0.25, black, black);
    va.AddTrianglesCircle(left_eye.x, left_eye.y, scale*sf*0.10, eye_color, eye_color);
    va.AddTrianglesCircle(right_eye.x, right_eye.y, scale*sf*0.10, eye_color, eye_color);

    va.Finish(GL_TRIANGLES);
}

void DmxModel::Draw3DDMXBaseLeft(DrawGLUtils::xlAccumulator &va, const xlColor &c, float &sx, float &sy, float &scale, float &pan_angle, float& rot_angle)
{
    dmxPoint3 p10(-3,-1,-5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p11(3,-1,-5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p12(-3,-5,-5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p13(3,-5,-5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p14(0,-1,-5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p15(-1,1,-5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p16(1,1,-5, sx, sy, scale, pan_angle, rot_angle);

    va.AddVertex(p10.x, p10.y, c);
    va.AddVertex(p11.x, p11.y, c);
    va.AddVertex(p12.x, p12.y, c);
    va.AddVertex(p11.x, p11.y, c);
    va.AddVertex(p12.x, p12.y, c);
    va.AddVertex(p13.x, p13.y, c);
    va.AddVertex(p10.x, p10.y, c);
    va.AddVertex(p14.x, p14.y, c);
    va.AddVertex(p15.x, p15.y, c);
    va.AddVertex(p11.x, p11.y, c);
    va.AddVertex(p14.x, p14.y, c);
    va.AddVertex(p16.x, p16.y, c);
    va.AddVertex(p15.x, p15.y, c);
    va.AddVertex(p14.x, p14.y, c);
    va.AddVertex(p16.x, p16.y, c);

    dmxPoint3 p210(-3,-1,-3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p211(3,-1,-3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p212(-3,-5,-3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p213(3,-5,-3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p214(0,-1,-3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p215(-1,1,-3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p216(1,1,-3, sx, sy, scale, pan_angle, rot_angle);

    va.AddVertex(p210.x, p210.y, c);
    va.AddVertex(p211.x, p211.y, c);
    va.AddVertex(p212.x, p212.y, c);
    va.AddVertex(p211.x, p211.y, c);
    va.AddVertex(p212.x, p212.y, c);
    va.AddVertex(p213.x, p213.y, c);
    va.AddVertex(p210.x, p210.y, c);
    va.AddVertex(p214.x, p214.y, c);
    va.AddVertex(p215.x, p215.y, c);
    va.AddVertex(p211.x, p211.y, c);
    va.AddVertex(p214.x, p214.y, c);
    va.AddVertex(p216.x, p216.y, c);
    va.AddVertex(p215.x, p215.y, c);
    va.AddVertex(p214.x, p214.y, c);
    va.AddVertex(p216.x, p216.y, c);

    va.AddVertex(p10.x, p10.y, c);
    va.AddVertex(p210.x, p210.y, c);
    va.AddVertex(p212.x, p212.y, c);
    va.AddVertex(p10.x, p10.y, c);
    va.AddVertex(p12.x, p12.y, c);
    va.AddVertex(p212.x, p212.y, c);
    va.AddVertex(p10.x, p10.y, c);
    va.AddVertex(p210.x, p210.y, c);
    va.AddVertex(p215.x, p215.y, c);
    va.AddVertex(p10.x, p10.y, c);
    va.AddVertex(p15.x, p15.y, c);
    va.AddVertex(p215.x, p215.y, c);
    va.AddVertex(p15.x, p15.y, c);
    va.AddVertex(p16.x, p16.y, c);
    va.AddVertex(p215.x, p215.y, c);
    va.AddVertex(p16.x, p16.y, c);
    va.AddVertex(p216.x, p216.y, c);
    va.AddVertex(p215.x, p215.y, c);
    va.AddVertex(p16.x, p16.y, c);
    va.AddVertex(p11.x, p11.y, c);
    va.AddVertex(p211.x, p211.y, c);
    va.AddVertex(p16.x, p16.y, c);
    va.AddVertex(p211.x, p211.y, c);
    va.AddVertex(p216.x, p216.y, c);
    va.AddVertex(p13.x, p13.y, c);
    va.AddVertex(p211.x, p211.y, c);
    va.AddVertex(p213.x, p213.y, c);
    va.AddVertex(p13.x, p13.y, c);
    va.AddVertex(p211.x, p211.y, c);
    va.AddVertex(p11.x, p11.y, c);
}

void DmxModel::Draw3DDMXBaseRight(DrawGLUtils::xlAccumulator &va, const xlColor &c, float &sx, float &sy, float &scale, float &pan_angle, float& rot_angle)
{
    dmxPoint3 p20(-3,-1,5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p21(3,-1,5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p22(-3,-5,5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p23(3,-5,5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p24(0,-1,5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p25(-1,1,5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p26(1,1,5, sx, sy, scale, pan_angle, rot_angle);

    va.AddVertex(p20.x, p20.y, c);
    va.AddVertex(p21.x, p21.y, c);
    va.AddVertex(p22.x, p22.y, c);
    va.AddVertex(p21.x, p21.y, c);
    va.AddVertex(p22.x, p22.y, c);
    va.AddVertex(p23.x, p23.y, c);
    va.AddVertex(p20.x, p20.y, c);
    va.AddVertex(p24.x, p24.y, c);
    va.AddVertex(p25.x, p25.y, c);
    va.AddVertex(p21.x, p21.y, c);
    va.AddVertex(p24.x, p24.y, c);
    va.AddVertex(p26.x, p26.y, c);
    va.AddVertex(p25.x, p25.y, c);
    va.AddVertex(p24.x, p24.y, c);
    va.AddVertex(p26.x, p26.y, c);

    dmxPoint3 p220(-3,-1,3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p221(3,-1,3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p222(-3,-5,3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p223(3,-5,3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p224(0,-1,3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p225(-1,1,3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p226(1,1,3, sx, sy, scale, pan_angle, rot_angle);

    va.AddVertex(p220.x, p220.y, c);
    va.AddVertex(p221.x, p221.y, c);
    va.AddVertex(p222.x, p222.y, c);
    va.AddVertex(p221.x, p221.y, c);
    va.AddVertex(p222.x, p222.y, c);
    va.AddVertex(p223.x, p223.y, c);
    va.AddVertex(p220.x, p220.y, c);
    va.AddVertex(p224.x, p224.y, c);
    va.AddVertex(p225.x, p225.y, c);
    va.AddVertex(p221.x, p221.y, c);
    va.AddVertex(p224.x, p224.y, c);
    va.AddVertex(p226.x, p226.y, c);
    va.AddVertex(p225.x, p225.y, c);
    va.AddVertex(p224.x, p224.y, c);
    va.AddVertex(p226.x, p226.y, c);

    va.AddVertex(p20.x, p20.y, c);
    va.AddVertex(p220.x, p220.y, c);
    va.AddVertex(p222.x, p222.y, c);
    va.AddVertex(p20.x, p20.y, c);
    va.AddVertex(p22.x, p22.y, c);
    va.AddVertex(p222.x, p222.y, c);
    va.AddVertex(p20.x, p20.y, c);
    va.AddVertex(p220.x, p220.y, c);
    va.AddVertex(p225.x, p225.y, c);
    va.AddVertex(p20.x, p20.y, c);
    va.AddVertex(p25.x, p25.y, c);
    va.AddVertex(p225.x, p225.y, c);
    va.AddVertex(p25.x, p25.y, c);
    va.AddVertex(p26.x, p26.y, c);
    va.AddVertex(p225.x, p225.y, c);
    va.AddVertex(p26.x, p26.y, c);
    va.AddVertex(p226.x, p226.y, c);
    va.AddVertex(p225.x, p225.y, c);
    va.AddVertex(p26.x, p26.y, c);
    va.AddVertex(p21.x, p21.y, c);
    va.AddVertex(p221.x, p221.y, c);
    va.AddVertex(p26.x, p26.y, c);
    va.AddVertex(p221.x, p221.y, c);
    va.AddVertex(p226.x, p226.y, c);
    va.AddVertex(p23.x, p23.y, c);
    va.AddVertex(p221.x, p221.y, c);
    va.AddVertex(p223.x, p223.y, c);
    va.AddVertex(p23.x, p23.y, c);
    va.AddVertex(p221.x, p221.y, c);
    va.AddVertex(p21.x, p21.y, c);
}

void DmxModel::Draw3DDMXHead(DrawGLUtils::xlAccumulator &va, const xlColor &c, float &sx, float &sy, float &scale, float &pan_angle, float &tilt_angle)
{
    // draw the head
    float pan_angle1 = pan_angle + 270.0f;  // needs to be rotated from reference we drew it
    dmxPoint3 p31(-2,3.45f,-4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p32(2,3.45f,-4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p33(4,0,-4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p34(2,-3.45f,-4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p35(-2,-3.45f,-4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p36(-4,0,-4, sx, sy, scale, pan_angle1, 0, tilt_angle);

    dmxPoint3 p41(-1,1.72f,4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p42(1,1.72f,4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p43(2,0,4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p44(1,-1.72f,4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p45(-1,-1.72f,4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p46(-2,0,4, sx, sy, scale, pan_angle1, 0, tilt_angle);

    va.AddVertex(p31.x, p31.y, c);
    va.AddVertex(p32.x, p32.y, c);
    va.AddVertex(p35.x, p35.y, c);
    va.AddVertex(p34.x, p34.y, c);
    va.AddVertex(p32.x, p32.y, c);
    va.AddVertex(p35.x, p35.y, c);
    va.AddVertex(p32.x, p32.y, c);
    va.AddVertex(p33.x, p33.y, c);
    va.AddVertex(p34.x, p34.y, c);
    va.AddVertex(p31.x, p31.y, c);
    va.AddVertex(p36.x, p36.y, c);
    va.AddVertex(p35.x, p35.y, c);

    va.AddVertex(p41.x, p41.y, c);
    va.AddVertex(p42.x, p42.y, c);
    va.AddVertex(p45.x, p45.y, c);
    va.AddVertex(p44.x, p44.y, c);
    va.AddVertex(p42.x, p42.y, c);
    va.AddVertex(p45.x, p45.y, c);
    va.AddVertex(p42.x, p42.y, c);
    va.AddVertex(p43.x, p43.y, c);
    va.AddVertex(p44.x, p44.y, c);
    va.AddVertex(p41.x, p41.y, c);
    va.AddVertex(p46.x, p46.y, c);
    va.AddVertex(p45.x, p45.y, c);

    va.AddVertex(p31.x, p31.y, c);
    va.AddVertex(p41.x, p41.y, c);
    va.AddVertex(p42.x, p42.y, c);
    va.AddVertex(p31.x, p31.y, c);
    va.AddVertex(p32.x, p32.y, c);
    va.AddVertex(p42.x, p42.y, c);

    va.AddVertex(p32.x, p32.y, c);
    va.AddVertex(p42.x, p42.y, c);
    va.AddVertex(p43.x, p43.y, c);
    va.AddVertex(p32.x, p32.y, c);
    va.AddVertex(p33.x, p33.y, c);
    va.AddVertex(p43.x, p43.y, c);

    va.AddVertex(p33.x, p33.y, c);
    va.AddVertex(p43.x, p43.y, c);
    va.AddVertex(p44.x, p44.y, c);
    va.AddVertex(p33.x, p33.y, c);
    va.AddVertex(p34.x, p34.y, c);
    va.AddVertex(p44.x, p44.y, c);

    va.AddVertex(p34.x, p34.y, c);
    va.AddVertex(p44.x, p44.y, c);
    va.AddVertex(p45.x, p45.y, c);
    va.AddVertex(p34.x, p34.y, c);
    va.AddVertex(p35.x, p35.y, c);
    va.AddVertex(p45.x, p45.y, c);

    va.AddVertex(p35.x, p35.y, c);
    va.AddVertex(p45.x, p45.y, c);
    va.AddVertex(p46.x, p46.y, c);
    va.AddVertex(p35.x, p35.y, c);
    va.AddVertex(p36.x, p36.y, c);
    va.AddVertex(p46.x, p46.y, c);

    va.AddVertex(p36.x, p36.y, c);
    va.AddVertex(p46.x, p46.y, c);
    va.AddVertex(p41.x, p41.y, c);
    va.AddVertex(p36.x, p36.y, c);
    va.AddVertex(p31.x, p31.y, c);
    va.AddVertex(p41.x, p41.y, c);
}

#define retmsg(msg)  \
{ \
wxMessageBox(msg, _("Export Error")); \
return; \
}

void DmxModel::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) retmsg(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()));
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

    wxString pdr = ModelXml->GetAttribute("DmxPanDegOfRot", "540");
    wxString tdr = ModelXml->GetAttribute("DmxTiltDegOfRot", "180");
    wxString s = ModelXml->GetAttribute("DmxStyle");
    wxString pc = ModelXml->GetAttribute("DmxPanChannel", "0");
    wxString po = ModelXml->GetAttribute("DmxPanOrient", "0");
    wxString tc = ModelXml->GetAttribute("DmxTiltChannel", "0");
    wxString to = ModelXml->GetAttribute("DmxTiltOrient","0");
    wxString rc = ModelXml->GetAttribute("DmxRedChannel","0");
    wxString gc = ModelXml->GetAttribute("DmxGreenChannel","0");
    wxString bc = ModelXml->GetAttribute("DmxBlueChannel","0");
    wxString sc = ModelXml->GetAttribute("DmxShutterChannel","0");
    wxString so = ModelXml->GetAttribute("DmxShutterOpen","1");

    // I need to do this because there are different defaults
    if (s == "Skulltronix Skull")
    {
        pc = ModelXml->GetAttribute("DmxPanChannel", "5");
        po = ModelXml->GetAttribute("DmxPanOrient", "90");
        pdr = ModelXml->GetAttribute("DmxPanDegOfRot", "180");
        tc = ModelXml->GetAttribute("DmxTiltChannel", "11");
        to = ModelXml->GetAttribute("DmxTiltOrient", "315");
        tdr = ModelXml->GetAttribute("DmxTiltDegOfRot", "90");
        rc = ModelXml->GetAttribute("DmxRedChannel", "16");
        gc = ModelXml->GetAttribute("DmxGreenChannel", "17");
        bc = ModelXml->GetAttribute("DmxBlueChannel", "18");
    }

    wxString tml = ModelXml->GetAttribute("DmxTiltMinLimit", "442");
    wxString tmxl = ModelXml->GetAttribute("DmxTiltMaxLimit", "836");
    wxString pml = ModelXml->GetAttribute("DmxPanMinLimit", "250");
    wxString pmxl = ModelXml->GetAttribute("DmxPanMaxLimit", "1250");
    wxString nc = ModelXml->GetAttribute("DmxNodChannel", "3");
    wxString no = ModelXml->GetAttribute("DmxNodOrient", "331");
    wxString ndr = ModelXml->GetAttribute("DmxNodDegOfRot", "58");
    wxString nml = ModelXml->GetAttribute("DmxNodMinLimit", "452");
    wxString nmxl = ModelXml->GetAttribute("DmxNodMaxLimit", "745");
    wxString jc = ModelXml->GetAttribute("DmxJawChannel", "1");
    wxString jml = ModelXml->GetAttribute("DmxJawMinLimit", "500");
    wxString jmxl = ModelXml->GetAttribute("DmxJawMaxLimit", "750");
    wxString eb = ModelXml->GetAttribute("DmxEyeBrtChannel","15");
    wxString eudc = ModelXml->GetAttribute("DmxEyeUDChannel", "7");
    wxString eudml = ModelXml->GetAttribute("DmxEyeUDMinLimit", "575");
    wxString eudmxl = ModelXml->GetAttribute("DmxEyeUDMaxLimit", "1000");
    wxString elrc = ModelXml->GetAttribute("DmxEyeLRChannel", "9");
    wxString elml = ModelXml->GetAttribute("DmxEyeLRMinLimit", "499");
    wxString elrmxl = ModelXml->GetAttribute("DmxEyeLRMaxLimit", "878");

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
    f.Write(wxString::Format("DmxPanDegOfRot=\"%s\" ", pdr));
    f.Write(wxString::Format("DmxTiltDegOfRot=\"%s\" ", tdr));
    f.Write(wxString::Format("DmxStyle=\"%s\" ", s));
    f.Write(wxString::Format("DmxPanChannel=\"%s\" ", pc));
    f.Write(wxString::Format("DmxPanOrient=\"%s\" ", po));
    f.Write(wxString::Format("DmxTiltChannel=\"%s\" ", tc));
    f.Write(wxString::Format("DmxTiltOrient=\"%s\" ", to));
    f.Write(wxString::Format("DmxRedChannel=\"%s\" ", rc));
    f.Write(wxString::Format("DmxGreenChannel=\"%s\" ", gc));
    f.Write(wxString::Format("DmxBlueChannel=\"%s\" ", bc));
    f.Write(wxString::Format("DmxShutterChannel=\"%s\" ", sc));
    f.Write(wxString::Format("DmxShutterOpen=\"%s\" ", so));

    f.Write(wxString::Format("DmxTiltMinLimit=\"%s\" ", tml));
    f.Write(wxString::Format("DmxTiltMaxLimit=\"%s\" ", tmxl));
    f.Write(wxString::Format("DmxPanMinLimit=\"%s\" ", pml));
    f.Write(wxString::Format("DmxPanMaxLimit=\"%s\" ", pmxl));
    f.Write(wxString::Format("DmxNodChannel=\"%s\" ", nc));
    f.Write(wxString::Format("DmxNodOrient=\"%s\" ", no));
    f.Write(wxString::Format("DmxNodDegOfRot=\"%s\" ", ndr));
    f.Write(wxString::Format("DmxNodMinLimit=\"%s\" ", nml));
    f.Write(wxString::Format("DmxNodMaxLimit=\"%s\" ", nmxl));
    f.Write(wxString::Format("DmxJawChannel=\"%s\" ", jc));
    f.Write(wxString::Format("DmxJawMinLimit=\"%s\" ", jml));
    f.Write(wxString::Format("DmxJawMaxLimit=\"%s\" ", jmxl));
    f.Write(wxString::Format("DmxEyeBrtChannel=\"%s\" ", eb));
    f.Write(wxString::Format("DmxEyeUDChannel=\"%s\" ", eudc));
    f.Write(wxString::Format("DmxEyeUDMinLimit=\"%s\" ", eudml));
    f.Write(wxString::Format("DmxEyeUDMaxLimit=\"%s\" ", eudmxl));
    f.Write(wxString::Format("DmxEyeLRChannel=\"%s\" ", elrc));
    f.Write(wxString::Format("DmxEyeLRMinLimit=\"%s\" ", elml));
    f.Write(wxString::Format("DmxEyeLRMaxLimit=\"%s\" ", elrmxl));
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

void DmxModel::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
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

            wxString pdr = root->GetAttribute("DmxPanDegOfRot");
            wxString tdr = root->GetAttribute("DmxTiltDegOfRot");
            wxString s = root->GetAttribute("DmxStyle");
            wxString pc = root->GetAttribute("DmxPanChannel");
            wxString po = root->GetAttribute("DmxPanOrient");
            wxString psl = root->GetAttribute("DmxPanSlewLimit");
            wxString tc = root->GetAttribute("DmxTiltChannel");
            wxString to = root->GetAttribute("DmxTiltOrient");
            wxString tsl = root->GetAttribute("DmxTiltSlewLimit");
            wxString rc = root->GetAttribute("DmxRedChannel");
            wxString gc = root->GetAttribute("DmxGreenChannel");
            wxString bc = root->GetAttribute("DmxBlueChannel");
            wxString sc = root->GetAttribute("DmxShutterChannel");
            wxString so = root->GetAttribute("DmxShutterOpen");
            wxString bl = root->GetAttribute("DmxBeamLimit");

            wxString tml = root->GetAttribute("DmxTiltMinLimit");
            wxString tmxl = root->GetAttribute("DmxTiltMaxLimit");
            wxString pml = root->GetAttribute("DmxPanMinLimit");
            wxString pmxl = root->GetAttribute("DmxPanMaxLimit");
            wxString nc = root->GetAttribute("DmxNodChannel");
            wxString no = root->GetAttribute("DmxNodOrient");
            wxString ndr = root->GetAttribute("DmxNodDegOfRot");
            wxString nml = root->GetAttribute("DmxNodMinLimit");
            wxString nmxl = root->GetAttribute("DmxNodMaxLimit");
            wxString jc = root->GetAttribute("DmxJawChannel");
            wxString jml = root->GetAttribute("DmxJawMinLimit");
            wxString jmxl = root->GetAttribute("DmxJawMaxLimit");
            wxString eb = root->GetAttribute("DmxEyeBrtChannel");
            wxString eudc = root->GetAttribute("DmxEyeUDChannel");
            wxString eudml = root->GetAttribute("DmxEyeUDMinLimit");
            wxString eudmxl = root->GetAttribute("DmxEyeUDMaxLimit");
            wxString elrc = root->GetAttribute("DmxEyeLRChannel");
            wxString elml = root->GetAttribute("DmxEyeLRMinLimit");
            wxString elrmxl = root->GetAttribute("DmxEyeLRMaxLimit");

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

            SetProperty("DmxPanDegOfRot", pdr);
            SetProperty("DmxTiltDegOfRot", tdr);
            SetProperty("DmxStyle", s);
            SetProperty("DmxPanChannel", pc);
            SetProperty("DmxPanOrient", po);
            SetProperty("DmxPanSlewLimit", psl);
            SetProperty("DmxTiltChannel", tc);
            SetProperty("DmxTiltOrient", to);
            SetProperty("DmxTiltSlewLimit", tsl);
            SetProperty("DmxRedChannel", rc);
            SetProperty("DmxGreenChannel", gc);
            SetProperty("DmxBlueChannel", bc);
            SetProperty("DmxShutterChannel", sc);
            SetProperty("DmxShutterOpen", so);
            SetProperty("DmxBeamLimit", bl);

            SetProperty("DmxTiltMinLimit", tml);
            SetProperty("DmxTiltMaxLimit", tmxl);
            SetProperty("DmxPanMinLimit", pml);
            SetProperty("DmxPanMaxLimit", pmxl);
            SetProperty("DmxNodChannel", nc);
            SetProperty("DmxNodOrient", no);
            SetProperty("DmxNodDegOfRot", ndr);
            SetProperty("DmxNodMinLimit", nml);
            SetProperty("DmxNodMaxLimit", nmxl);
            SetProperty("DmxJawChannel", jc);
            SetProperty("DmxJawMinLimit", jml);
            SetProperty("DmxJawMaxLimit", jmxl);
            SetProperty("DmxEyeBrtChannel", eb);
            SetProperty("DmxEyeUDChannel", eudc);
            SetProperty("DmxEyeUDMinLimit", eudml);
            SetProperty("DmxEyeUDMaxLimit", eudmxl);
            SetProperty("DmxEyeLRChannel", elrc);
            SetProperty("DmxEyeLRMinLimit", elml);
            SetProperty("DmxEyeLRMaxLimit", elrmxl);

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

            xlights->MarkEffectsFileDirty(true);
        }
        else
        {
            wxMessageBox("Failure loading DMX model file.");
        }
    }
    else
    {
        wxMessageBox("Failure loading DMX model file.");
    }
}
