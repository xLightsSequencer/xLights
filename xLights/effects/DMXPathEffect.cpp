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
#include "../models/DMX/DmxMovingHead.h"
#include "assist/AssistPanel.h"
#include "assist/DMXPathAssistPanel.h"

#include "../../include/dmxpath-16.xpm"
#include "../../include/dmxpath-24.xpm"
#include "../../include/dmxpath-32.xpm"
#include "../../include/dmxpath-48.xpm"
#include "../../include/dmxpath-64.xpm"
#include "UtilFunctions.h"



DMXPathEffect::DMXPathEffect(int id) : RenderableEffect(id, "DMX Path", dmxpath_16, dmxpath_24, dmxpath_32, dmxpath_48, dmxpath_64)
{
    //ctor
}

DMXPathEffect::~DMXPathEffect()
{
    //dtor
}

xlEffectPanel *DMXPathEffect::CreatePanel(wxWindow *parent) {
    m_panel = new DMXPathPanel(parent);
    return m_panel;
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
    buffer.SetPixel(chan - 1, 0, color, false, false, true);
}

void DMXPathEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    double eff_pos = buffer.GetEffectTimeIntervalPosition(1.0F);

    auto startTm = buffer.GetStartTimeMS();
    auto endTm = buffer.GetEndTimeMS();
    auto length = endTm - startTm;
    std::string type_Str = SettingsMap["CHOICE_DMXPath_Type"];
    auto width = SettingsMap.GetInt("SLIDER_DMXPath_Width", 30);
    auto height = SettingsMap.GetInt("SLIDER_DMXPath_Height", 30);
    auto x_offset = SettingsMap.GetInt("SLIDER_DMXPath_X_Off", 0);
    auto y_offset = SettingsMap.GetInt("SLIDER_DMXPath_Y_Off", 0);
    auto distance = SettingsMap.GetInt("SLIDER_DMXPath_Dist", 0);

    if(nullptr != m_dmxAssistPanel && nullptr != effect ) {
        if (!m_dmxAssistPanel->IsBeingDeleted() )
            m_dmxAssistPanel->SetSettingDef(SettingsMap);
    }

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
    int panChan { -1 };
    int tiltChan{ -1 };
    int panDegrees{ 360 };
    int tiltDegrees{ 150 };

    if (model_info->IsDMXModel()) {
        auto dmxTilt = static_cast<DmxMovingHead*>(model_info);
        if (nullptr != dmxTilt) {
            panChan = dmxTilt->GetPanChannel();
            tiltChan = dmxTilt->GetTiltChannel();
            panDegrees = dmxTilt->GetPanDegOfRot();
            tiltDegrees = dmxTilt->GetTiltDegOfRot();
        }
    }

    if (panChan == -1 && tiltChan == -1) {
        int num_channels = model_info->GetNumChannels();
        for (int i = 0; i < num_channels; ++i) {
            std::string name = model_info->GetNodeName(i);
            if (StartsWith(name, "Tilt")) {
                tiltChan = i;
            }
            if (StartsWith(name, "Pan")) {
                panChan = i;
            }
        }
    }

    auto pathType = DecodeType(type_Str);

    auto [x, y] = RenderPath(pathType, eff_pos, height, width, x_offset, y_offset, rotation);

    if (panChan != -1) {
        SetDMXColorPixel(panChan, ScaleToDMX(x, panDegrees), buffer);
    }
    if (tiltChan != -1) {
        SetDMXColorPixel(tiltChan, ScaleToDMX(y, tiltDegrees), buffer);
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

std::pair<int, int> DMXPathEffect::RenderPath(DMXPathType effectType, double eff_pos, int height, int width, int x_off, int y_off, int rot)
{
    float position = 360.0 * (eff_pos);


    auto [x, y] = CalcLocation(effectType, position);

    x = width * x;
    y = height * y;

    double radRot = (rot) * (M_PI / 180.0);
    double rx = (x * cos(radRot)) - (y * sin(radRot));
    double ry = (y * cos(radRot)) + (x * sin(radRot));

    int xx = std::round(rx) + x_off;
    int yy = std::round(ry) + y_off;

    //x += x_off;
    //y += y_off;
    return {xx,yy};
}

std::pair<float, float> DMXPathEffect::CalcLocation(DMXPathType effectType, float degpos)
{
    float x;
    float y;

    double radpos = degpos * M_PI / 180.0;

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


int DMXPathEffect::ScaleToDMX(float value, float degresOfMovement) const
{
    float MinMax = degresOfMovement / 2.0F;
    value = std::min(MinMax, value);
    value = std::max(-MinMax, value);
    return (double(value + MinMax) / degresOfMovement) * 255.0F;
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

AssistPanel* DMXPathEffect::GetAssistPanel(wxWindow* parent, xLightsFrame* /*xl_frame*/)
{
    if (m_panel == nullptr)
      return nullptr;

    AssistPanel* assistPanel = new AssistPanel(parent);

    auto dmxAssistPanel = new DMXPathAssistPanel(assistPanel->GetCanvasParent());
    // sketchAssistPanel->SetxLightsFrame(xl_frame);
    assistPanel->AddPanel(dmxAssistPanel, wxALL | wxEXPAND);

    m_dmxAssistPanel = dmxAssistPanel;
    
    return assistPanel;
}
