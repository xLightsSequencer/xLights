/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "DMXPathEffect.h"
#include "DMXPathPanel.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../models/Model.h"
#include "../models/ModelGroup.h"

#include "../../include/dmxpath-16.xpm"
#include "../../include/dmxpath-24.xpm"
#include "../../include/dmxpath-32.xpm"
#include "../../include/dmxpath-48.xpm"
#include "../../include/dmxpath-64.xpm"
#include "UtilFunctions.h"

#define DMXPATH_ROTATION_MIN 0
#define DMXPATH_ROTATION_MAX 360

DMXPathEffect::DMXPathEffect(int id) : RenderableEffect(id, "DMX Path", dmxpath_16, dmxpath_24, dmxpath_32, dmxpath_48, dmxpath_64)
{
    //ctor
}

DMXPathEffect::~DMXPathEffect()
{
    //dtor
}

xlEffectPanel *DMXPathEffect::CreatePanel(wxWindow *parent) {
    return new DMXPathPanel(parent);
}

static int GetPct(wxString val)
{
    int value = wxAtoi(val);
    return (value * 100) / 255;
}

void DMXPathEffect::SetDefaultParameters() {
    DMXPathPanel *dp = (DMXPathPanel*)panel;
    if (dp == nullptr) {
        return;
    }


}

void DMXPathEffect::SetDMXColorPixel(int chan, uint8_t value, RenderBuffer &buffer)
{
    xlColor color(value, value, value);
    buffer.SetPixel(chan, 0, color, false, false, true);
}

void DMXPathEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    double eff_pos = buffer.GetEffectTimeIntervalPosition(1.0F);

    if (eff_pos > 1.0) {
        auto t = "arrrrr";
    }
    auto startTm = buffer.GetStartTimeMS();
    auto endTm = buffer.GetEndTimeMS();
    auto length = endTm - startTm;
    std::string type_Str = SettingsMap["CHOICE_DMXPath_Type"];
    auto width = SettingsMap.GetInt("SLIDER_DMXPath_Width", 30);
    auto heigth = SettingsMap.GetInt("SLIDER_DMXPath_Height", 30);
    auto x_offset = SettingsMap.GetInt("SLIDER_DMXPath_X", 127);
    auto y_offset = SettingsMap.GetInt("SLIDER_DMXPath_Y", 127);

    int rotation = GetValueCurveInt("DMXPath_Rotation", 0, SettingsMap, eff_pos, DMXPATH_ROTATION_MIN, DMXPATH_ROTATION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    if (buffer.cur_model == "") {
        return;
    }
    Model* model_info = buffer.GetModel();
    if (model_info == nullptr) {
        return;
    }
    const std::string& string_type = model_info->GetStringType();
    if (!StartsWith(string_type, "Single Color")) {
        return;
    }

    auto pathType = DecodeType(type_Str);
    auto [x, y] = renderPath(pathType, eff_pos, length, heigth,width,x_offset,y_offset,rotation);

    int num_channels = model_info->GetNumChannels();

    for (int i = 0; i < num_channels; ++i) {
        std::string name = model_info->GetNodeName(i);
        if (StartsWith(name, "Tilt")) {
            SetDMXColorPixel(i,x,buffer);
        }
        if (StartsWith(name, "Pan")) {
            SetDMXColorPixel(i, y, buffer);
        }
    }
}

void DMXPathEffect::SetPanelStatus(Model *cls) {
    DMXPathPanel *p = (DMXPathPanel*)panel;
    if (p == nullptr) {
        return;
    }
    if (cls == nullptr) {
        return;
    }

    Model* m = cls;
    if (cls->GetDisplayAs() == "ModelGroup") {
        m = dynamic_cast<ModelGroup*>(cls)->GetFirstModel();
        if (m == nullptr) m = cls;
    }

    //int num_channels = m->GetNumChannels();

    //for(int i = 0; i <= num_channels; ++i) {
    //    std::string name = m->GetNodeName(i);
    //}
    p->FlexGridSizer_Main->Layout();
    p->Refresh();
}

std::pair<uint8_t, uint8_t> DMXPathEffect::renderPath(DMXPathType effectType, double eff_pos,long length, int height, int width, int x_off, int y_off, int rot)
{
    float position = 360.0 * (eff_pos);


    auto [x, y] = calcLocation(effectType, position);

    x = width * x;
    y = height * y;

    double radRot = (rot) * (M_PI / 180.0);
    double rx = (x * cos(radRot)) - (y * sin(radRot));
    double ry = (y * cos(radRot)) + (x * sin(radRot));

    int xx = std::round(rx) + x_off;
    int yy = std::round(ry) + y_off;

    //x += x_off;
    //y += y_off;
    xx = std::min(DMX_MAX, xx);
    xx = std::max(DMX_MIN, xx);
    yy = std::min(DMX_MAX, yy);
    yy = std::max(DMX_MIN, yy);

    return {xx,yy};
}

std::pair<float, float> DMXPathEffect::calcLocation(DMXPathType effectType, float degpos)
{
    float x;
    float y;

    double radpos = degpos * PI / 180.0;

    switch (effectType) {
    case DMXPathType::Circle:
        x = sin(radpos);
        y = cos(radpos);

        break;
    case DMXPathType::Square:
        if (radpos < M_PI / 2) {
            x = (radpos * 2 / M_PI) * 2 - 1;
            y = 1.0F;
        } else if (M_PI / 2 <= radpos && radpos < M_PI) {
            x = 1.0F;
            y = (1 - (radpos - M_PI / 2) * 2 / M_PI) * 2 - 1;
        } else if (M_PI <= radpos && radpos < M_PI * 3 / 2) {
            x = (1 - (radpos - M_PI) * 2 / M_PI) * 2 - 1;
            y = -1.0F;
        } else // M_PI * 3 / 2 <= iterator
        {
            x = -1.0F;
            y = ((radpos - M_PI * 3 / 2) * 2 / M_PI) * 2 - 1;
        }
        break;
    case DMXPathType::Leaf:
        x = pow(cos(radpos + (M_PI / 2.0)), 5);
        y = cos(radpos);
        break;
    case DMXPathType::Line:
        x = cos(radpos);
        y = cos(radpos);
        break;
    case DMXPathType::Diamond:
        x = pow(cos(radpos - (M_PI / 2.0)), 3);
        y = pow(cos(radpos), 3);
        break;
    case DMXPathType::Eight:
        x = cos((radpos * 2) + (M_PI/2.0));
        y = cos(radpos);
        break;
    default:
        wxASSERT(false);
        break;
    }

    return { x, y };
}

DMXPathType DMXPathEffect::DecodeType(const std::string& shape)
{
    if (shape == "Circle") {
        return DMXPathType::Circle;
    } else if (shape == "Square") {
        return DMXPathType::Square;
    } else if (shape == "Diamond") {
        return DMXPathType::Diamond;
    } else if (shape == "Line") {
        return DMXPathType::Line;
    } else if (shape == "Leaf") {
        return DMXPathType::Leaf;
    } else if (shape == "Eight") {
        return DMXPathType::Eight;
    }
    return DMXPathType::Unknown;
}
