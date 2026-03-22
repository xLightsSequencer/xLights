/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ColorManager.h"
#include "xLightsMain.h"
#include "XmlSerializer/XmlSerializingVisitor.h"

#include <log4cpp/Category.hh>
#include "ui/wxUtilities.h"

ColorManager::ColorManager(xLightsFrame* frame)
: xlights(frame)
{
    ResetDefaults();
    pInstance = this;
}

ColorManager::~ColorManager()
{
    colors.clear();
    colors_backup.clear();
}

ColorManager* ColorManager::pInstance = nullptr;

ColorManager* ColorManager::instance()
{
    if( pInstance == nullptr ) {
        // this should not be possible since the main application should
        // always have constructed this object before any clients need it
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.error("Color Manager instance was still a nullptr.");
    }
    return pInstance;
}

void ColorManager::SysColorChanged() {
    for( size_t i = 0; i < ColorManager::ColorNames::NUM_COLORS; ++i ) {
        if (xLights_color[i].systemColor != wxSYS_COLOUR_MAX) {
            //using a system color, we need to reload it
            wxColour c = wxSystemSettings::GetColour(xLights_color[i].systemColor);
#ifdef __WXOSX__
            if (c.IsSolid()) {
                colors_system[xLights_color[i].name] = wxColourToXlColor(c);
            } else {
                colors_system[xLights_color[i].name] = xLights_color[i].color;
            }
#else
            colors_system[xLights_color[i].name] = wxColourToXlColor(c);
#endif
        }
	}
}


void ColorManager::ResetDefaults()
{
    colors.clear();
    colors_system.clear();
    for (size_t i = 0; i < ColorManager::ColorNames::NUM_COLORS; ++i) {
        if (xLights_color[i].systemColor == wxSYS_COLOUR_MAX) {
            colors[xLights_color[i].name] = xLights_color[i].color;
        } else {
            wxColour c = wxSystemSettings::GetColour(xLights_color[i].systemColor);
#ifdef __WXOSX__
            if (c.IsSolid()) {
                colors_system[xLights_color[i].name] = wxColourToXlColor(c);
            } else {
                colors_system[xLights_color[i].name] = xLights_color[i].color;
            }
#else
            colors_system[xLights_color[i].name] = wxColourToXlColor(c);
#endif
        }
	}
}

void ColorManager::Snapshot()
{
	// store a new snapshot
    colors_backup.clear();
    for (const auto& it: colors)
	{
        colors_backup[it.first] = it.second;
	}
}

void ColorManager::RestoreSnapshot()
{
    colors.clear();
    for (const auto& it : colors_backup)
	{
        colors[it.first] = it.second;
	}
}

wxColor ColorManager::CyanOrBlueOverride() {
    const xlColor* color = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_TEXT_HIGHLIGHTED);
    if (*color == xlBLACK) {
        return CyanOrBlue();
    } else {
        return xlColorToWxColour(*color);
    }
}

wxColor ColorManager::LightOrMediumGreyOverride() {
    const xlColor* color = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_TEXT_UNSELECTED);
    if (*color == xlBLACK) {
        return LightOrMediumGrey();
    } else {
        return xlColorToWxColour(*color);
    }
}

void ColorManager::SetNewColor(std::string name, xlColor& color)
{
    std::string color_name = name;
    if (color_name.find("ID_") != std::string::npos) {
        color_name.replace(0, 16, "");
    }

    colors[color_name] = color;
}

void ColorManager::SetDirty()
{
    xlights->UnsavedRgbEffectsChanges = true;
    xlights->UpdateLayoutSave();
    xlights->UpdateControllerSave();
}

xlColor ColorManager::GetColor(ColorManager::ColorNames name)
{
    auto search = colors.find(xLights_color[name].name);
    if(search != colors.end()) {
        return search->second;
    }
    auto search2 = colors_system.find(xLights_color[name].name);
    if(search2 != colors_system.end()) {
        return search2->second;
    }
    return xLights_color[name].color;
}

const xlColor* ColorManager::GetColorPtr(ColorNames name)
{
    auto const search = colors.find(xLights_color[name].name);
    if (search != colors.end()) {
        return &search->second;
    }
    return &colors_system[xLights_color[name].name];
}

void ColorManager::RefreshColors()
{
    xlights->RenderLayout();
}

const xlColor ColorManager::GetTimingColor(int colorIndex)
{
    xlColor value;
    switch(colorIndex%5)
    {
        case 0:
            //
            value = colors["Timing1"];
            break;
        case 1:
            value = colors["Timing2"];
            break;
        case 2:
            value = colors["Timing3"];
            break;
        case 3:
            value = colors["Timing4"];
            break;
        default:
            value = colors["Timing5"];
            break;
    }
    return value;
}

void ColorManager::Save(BaseSerializingVisitor& visitor) const
{
    BaseSerializingVisitor::AttrCollector emptyAttrs;
    visitor.WriteOpenTag("colors", emptyAttrs);
    for (const auto& it : colors) {
        BaseSerializingVisitor::AttrCollector attrs;
        attrs.Add("Red", std::to_string(it.second.red));
        attrs.Add("Green", std::to_string(it.second.green));
        attrs.Add("Blue", std::to_string(it.second.blue));
        visitor.WriteOpenTag(it.first, attrs, /*selfClose=*/true);
    }
    visitor.WriteCloseTag();
}

void ColorManager::Load(pugi::xml_node colors_node)
{
	if (colors_node)
	{
        ResetDefaults();
        for (pugi::xml_node c = colors_node.first_child(); c; c = c.next_sibling())
        {
            std::string name = c.name();
            int red = c.attribute("Red").as_int(0);
            int green = c.attribute("Green").as_int(0);
            int blue = c.attribute("Blue").as_int(0);
            colors[name] = xlColor(red, green, blue);
        }
	}
}
