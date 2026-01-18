/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/sstream.h>

#include "Servo.h"
#include "../BaseObject.h"
#include "../ModelScreenLocation.h"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Servo::Servo(const std::string& _name, bool _is2d)
    : base_name(_name), channel(0),
    min_limit(1), max_limit(65535), range_of_motion(180.0f),
    pivot_offset_x(0), pivot_offset_y(0), pivot_offset_z(0),
    servo_style_val(0), servo_style("Translate X"),
    _16bit(true), offset_scale(_is2d ? 100.0f : 1.0f),
    is_2d(_is2d), link(nullptr)
{
}

Servo::~Servo()
{
}

static wxPGChoices SERVO_STYLES;
static wxPGChoices ZERO_BEHAVIORS;
static wxPGChoices DATA_TYPES;

enum SERVO_STYLE {
    SERVO_STYLE_TRANSLATEX,
    SERVO_STYLE_TRANSLATEY,
    SERVO_STYLE_TRANSLATEZ,
    SERVO_STYLE_ROTATEX,
    SERVO_STYLE_ROTATEY,
    SERVO_STYLE_ROTATEZ
};

void Servo::Init(BaseObject* base) {
    this->base = base;

    lastValue = (max_limit + min_limit) / 2;

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
}

bool Servo::IsTranslate() const  {
    return (servo_style_val == SERVO_STYLE_TRANSLATEX ||
        servo_style_val == SERVO_STYLE_TRANSLATEY ||
        servo_style_val == SERVO_STYLE_TRANSLATEZ);
}

bool Servo::IsRotate() const {
    return (servo_style_val == SERVO_STYLE_ROTATEX ||
            servo_style_val == SERVO_STYLE_ROTATEY ||
            servo_style_val == SERVO_STYLE_ROTATEZ);
}

float Servo::GetPosition(int channel_value) {
    if (channel_value == 0) {
        channel_value = lastValue;
    }
    if (channel_value < min_limit) {
        channel_value = min_limit;
    }
    if (channel_value > max_limit) {
        channel_value = max_limit;
    }
    lastValue = channel_value;
    return ((1.0 - ((channel_value - min_limit) / (float)(max_limit - min_limit))) * range_of_motion - range_of_motion);
}

void Servo::FillMotionMatrix(float servo_pos, glm::mat4& motion_matrix) {
    glm::vec3 scale = base->GetBaseObjectScreenLocation().GetScaleMatrix();
    
    glm::mat4 Identity = glm::mat4(1.0f);
    switch(servo_style_val) {
    case SERVO_STYLE_TRANSLATEX:
        motion_matrix = glm::translate(Identity, glm::vec3(-servo_pos / scale.x, 0.0f, 0.0f));
        break;
    case SERVO_STYLE_TRANSLATEY:
        motion_matrix = glm::translate(Identity, glm::vec3(0.0f, -servo_pos / scale.y, 0.0f));
        break;
    case SERVO_STYLE_TRANSLATEZ:
        motion_matrix = glm::translate(Identity, glm::vec3(0.0f, 0.0f, -servo_pos / scale.z));
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
}

void Servo::AddTypeProperties(wxPropertyGridInterface *grid, bool pwm) {
    grid->Append(new wxPropertyCategory(base_name, base_name + "Properties"));
    
    wxPGProperty* p = grid->Append(new wxUIntProperty("Channel", base_name + "Channel", channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Min Limit", base_name + "MinLimit", min_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", _16bit ? 65535 : 255);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Max Limit", base_name + "MaxLimit", max_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", _16bit ? 65535 : 255);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxFloatProperty("Range of Motion", base_name + "RangeOfMotion", range_of_motion));
    p->SetAttribute("Precision", 1);
    p->SetAttribute("Step", 1.0);
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
    if (DATA_TYPES.GetCount() == 0) {
        DATA_TYPES.Add("Scaled");
        DATA_TYPES.Add("Absolute");
        DATA_TYPES.Add("1/2 Absolute");
        DATA_TYPES.Add("2x Absolute");
    }
    if (ZERO_BEHAVIORS.GetCount() == 0) {
        ZERO_BEHAVIORS.Add("Hold");
        ZERO_BEHAVIORS.Add("Min");
        ZERO_BEHAVIORS.Add("Max");
        ZERO_BEHAVIORS.Add("Center");
        ZERO_BEHAVIORS.Add("Stop PWM");
    }

    grid->Append(new wxEnumProperty("Servo Style", base_name + "ServoStyle", SERVO_STYLES, servo_style_val));

    switch (servo_style_val) {
    case SERVO_STYLE_ROTATEX:
    case SERVO_STYLE_ROTATEY:
    case SERVO_STYLE_ROTATEZ:
        p = grid->Append(new wxFloatProperty("Pivot Offset X", base_name + "PivotOffsetX", pivot_offset_x * offset_scale));
        p->SetAttribute("Precision", 1);
        p->SetAttribute("Step", 1.0);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxFloatProperty("Pivot Offset Y", base_name + "PivotOffsetY", pivot_offset_y * offset_scale));
        p->SetAttribute("Precision", 1);
        p->SetAttribute("Step", 1.0);
        p->SetEditor("SpinCtrl");

        if (!is_2d) {
            p = grid->Append(new wxFloatProperty("Pivot Offset Z", base_name + "PivotOffsetZ", pivot_offset_z * offset_scale));
            p->SetAttribute("Precision", 1);
            p->SetAttribute("Step", 1.0);
            p->SetEditor("SpinCtrl");
        }
        break;
    default:
        break;
    }
    if (pwm) {
        p = grid->Append(new wxUIntProperty("Contoller Min Limit", "Controller" + base_name + "MinLimit", controller_min));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 50000);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxUIntProperty("Controller Max Limit", "Controller" + base_name + "MaxLimit", controller_max));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 50000);
        p->SetEditor("SpinCtrl");

        grid->Append(new wxBoolProperty("Controller Reverse", "Controller" + base_name + "Reverse", controller_reverse))->SetAttribute("UseCheckbox", 1);
        
        
        int zbv = ZERO_BEHAVIORS.Index(controller_zero);
        int dtv = DATA_TYPES.Index(controller_dataType);
        grid->Append(new wxEnumProperty("Controller Zero Behavior", "Controller" + base_name + "ZeroBehavior", ZERO_BEHAVIORS, zbv));
        grid->Append(new wxEnumProperty("Controller DataType", "Controller" + base_name + "DataType", DATA_TYPES, dtv));
    }
    grid->Collapse(base_name + "Properties");
}

int Servo::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked) {
    std::string name = event.GetPropertyName().ToStdString();
    std::string cname = "Controller" + base_name;
    if (base_name + "Channel" == name) {
        channel = (int)event.GetPropertyValue().GetLong();
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Servo::OnPropertyGridChange::Channel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Servo::OnPropertyGridChange::Channel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Servo::OnPropertyGridChange::Channel");
        return 0;
    } else if (base_name + "MinLimit" == name) {
        min_limit = (int)event.GetPropertyValue().GetLong();
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Servo::OnPropertyGridChange::MinLimit");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Servo::OnPropertyGridChange::MinLimit");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Servo::OnPropertyGridChange::MinLimit");
        return 0;
    } else if (base_name + "MaxLimit" == name) {
        max_limit = (int)event.GetPropertyValue().GetLong();
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Servo::OnPropertyGridChange::MaxLimit");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Servo::OnPropertyGridChange::MaxLimit");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Servo::OnPropertyGridChange::MaxLimit");
        return 0;
    } else if (base_name + "RangeOfMotion" == name) {
        range_of_motion = (int)event.GetPropertyValue().GetLong();
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Servo::OnPropertyGridChange::RangeOfMotion");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Servo::OnPropertyGridChange::RangeOfMotion");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Servo::OnPropertyGridChange::RangeOfMotion");
        return 0;
    } else if (base_name + "PivotOffsetX" == name) {
        pivot_offset_x = event.GetValue().GetDouble() / offset_scale;
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Servo::OnPropertyGridChange::PivotOffsetX");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Servo::OnPropertyGridChange::PivotOffsetX");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Servo::OnPropertyGridChange::PivotOffsetX");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Servo::OnPropertyGridChange::PivotOffsetX");
        return 0;
    } else if (base_name + "PivotOffsetY" == name) {
        pivot_offset_y = event.GetValue().GetDouble() / offset_scale;
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Servo::OnPropertyGridChange::PivotOffsetY");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Servo::OnPropertyGridChange::PivotOffsetY");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Servo::OnPropertyGridChange::PivotOffsetY");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Servo::OnPropertyGridChange::PivotOffsetY");
        return 0;
    } else if (base_name + "PivotOffsetZ" == name) {
        pivot_offset_z = event.GetValue().GetDouble() / offset_scale;
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Servo::OnPropertyGridChange::PivotOffsetZ");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Servo::OnPropertyGridChange::PivotOffsetZ");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Servo::OnPropertyGridChange::PivotOffsetZ");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Servo::OnPropertyGridChange::PivotOffsetZ");
        return 0;
    } else if (base_name + "ServoStyle" == name) {
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
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Servo::OnPropertyGridChange::ServoStyle");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Servo::OnPropertyGridChange::ServoStyle");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Servo::OnPropertyGridChange::ServoStyle");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Servo::OnPropertyGridChange::ServoStyle");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Servo::OnPropertyGridChange::ServoStyle");
        base->AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Servo::OnPropertyGridChange::ServoStyle");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Servo::OnPropertyGridChange::ServoStyle");
        return 0;
    } else if (name.starts_with(cname)) {
        if (name.ends_with("MinLimit")) {
            controller_min = event.GetValue().GetLong();
        } else if (name.ends_with("MaxLimit")) {
            controller_max = event.GetValue().GetLong();
        } else if (name.ends_with("Reverse")) {
            controller_reverse = event.GetValue().GetBool();
        } else if (name.ends_with("ZeroBehavior")) {
            controller_zero = ZERO_BEHAVIORS.GetLabel(event.GetValue().GetLong());
        } else if (name.ends_with("DataType")) {
            controller_dataType = DATA_TYPES.GetLabel(event.GetValue().GetLong());
        }
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Servo::OnPropertyGridChange::" + name);
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Servo::OnPropertyGridChange::" + name);
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Servo::OnPropertyGridChange::" + name);
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Servo::OnPropertyGridChange::" + name);
    }

    return -1;
}

void Servo::Set16Bit(bool value)
{
    _16bit = value;
    if (!_16bit) {
        if (min_limit > 255) {
            min_limit = 255;
        }
        if (max_limit > 255) {
            max_limit = 255;
        }
    }
}

void Servo::GetPWMOutputs(std::map<uint32_t, PWMOutput> &channels) const {
    if (channel > 0) {
        channels[channel] = PWMOutput(channel, PWMOutput::Type::SERVO, _16bit ? 2 : 1, base_name, controller_min, controller_max,
                                      controller_reverse, controller_zero, controller_dataType);
    }
}
