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

#include "Servo.h"
#include "../BaseObject.h"
#include "../ModelScreenLocation.h"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Servo::Servo(const std::string& _name, bool _is2d)
    : base_name(_name), offset_scale(_is2d ? 100.0f : 1.0f), is_2d(_is2d)
{
}

Servo::~Servo()
{
}

void Servo::Init(BaseObject* base) {
    this->base = base;
}

void Servo::SetStyle(const std::string& style) {
    servo_style = style;
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
    } else {
        servo_style_val = SERVO_STYLE_TRANSLATEX;
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
