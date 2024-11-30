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

#include "ColorManager.h"
#include "xLightsMain.h"

#include <log4cpp/Category.hh>

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
                colors_system[xLights_color[i].name] = c;
            } else {
                colors_system[xLights_color[i].name] = xLights_color[i].color;
            }
#else
            colors_system[xLights_color[i].name] = c;
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
                colors_system[xLights_color[i].name] = c;
            } else {
                colors_system[xLights_color[i].name] = xLights_color[i].color;
            }
#else
            colors_system[xLights_color[i].name] = c;
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
    if (color->asWxColor() == *wxBLACK) {
        return CyanOrBlue();
    } else {
        return color->asWxColor();
    }
}

wxColor ColorManager::LightOrMediumGreyOverride() {
    const xlColor* color = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_TEXT_UNSELECTED);
    if (color->asWxColor() == *wxBLACK) {
        return LightOrMediumGrey();
    } else {
        return color->asWxColor();
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

void ColorManager::Save(wxXmlDocument* doc)
{
	wxXmlNode* colors_node = nullptr;

	// find an existing view node in the document and delete it
	for (wxXmlNode* e = doc->GetRoot()->GetChildren(); e != nullptr; e = e->GetNext())
	{
		if (e->GetName() == "colors") colors_node = e;
	}
	if (colors_node != nullptr)
	{
		doc->GetRoot()->RemoveChild(colors_node);
	}

	wxXmlNode* newnode = Save();
    doc->GetRoot()->AddChild(newnode);
}

wxXmlNode* ColorManager::Save() const
{
	wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "colors");

	for (const auto& it : colors)
	{
	    wxXmlNode* cnode = new wxXmlNode(wxXML_ELEMENT_NODE, it.first);
	    cnode->AddAttribute("Red", wxString::Format("%d", it.second.red));
	    cnode->AddAttribute("Green", wxString::Format("%d", it.second.green));
	    cnode->AddAttribute("Blue", wxString::Format("%d", it.second.blue));
        node->AddChild(cnode);
	}

	return node;
}

void ColorManager::Load(wxXmlNode* colors_node)
{
	if (colors_node != nullptr)
	{
        ResetDefaults();
        for (wxXmlNode* c = colors_node->GetChildren(); c != nullptr; c = c->GetNext())
        {
            std::string name = c->GetName().ToStdString();
            wxString red_attr;
            c->GetAttribute("Red", &red_attr);
            wxString green_attr;
            c->GetAttribute("Green", &green_attr);
            wxString blue_attr;
            c->GetAttribute("Blue", &blue_attr);
            colors[name] = xlColor(wxAtoi(red_attr),wxAtoi(green_attr),wxAtoi(blue_attr));
        }
	}
}
