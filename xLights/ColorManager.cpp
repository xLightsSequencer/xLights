#include "ColorManager.h"
#include "xLightsMain.h"
#include <wx/xml/xml.h>

ColorManager::ColorManager(xLightsFrame* frame)
: xlights(frame)
{
    colors["Timing1"] = new xlColor(  0, 255, 255); // xlCYAN
    colors["Timing2"] = new xlColor(255,   0,   0); // xlRED
    colors["Timing3"] = new xlColor(  0, 255,   0); // xlGREEN
    colors["Timing4"] = new xlColor(  0,   0, 255); // xlBLUE
    colors["Timing5"] = new xlColor(255, 255,   0); // xlYELLOW

    colors["EffectSelected"] = new xlColor(204, 102, 255);
    colors["ReferenceEffect"] = new xlColor(255,0,255);
    colors["ModelSelected"] = new xlColor(255, 255, 0);     // xlYELLOW

    colors["HeaderColor"] = new xlColor(212,208,200);
    colors["HeaderSelectedColor"] = new xlColor(130,178,207);
}

ColorManager::~ColorManager()
{
    for(std::map<std::string, xlColor*>::iterator itr = colors.begin(); itr != colors.end(); itr++)
    {
        delete itr->second;
    }
}

void ColorManager::SetNewColor(std::string name, xlColor* color)
{
    std::string color_name = name;
    if (color_name.find("ID_") != std::string::npos) {
        color_name.replace(0, 16, "");
    }

    *colors[color_name] = *color;
    xlights->UnsavedRgbEffectsChanges = true;
}

xlColor* ColorManager::GetColor(std::string name)
{
    return colors[name];
}

const xlColor* ColorManager::GetTimingColor(int colorIndex)
{
    const xlColor* value;
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

	for (auto it = colors.begin(); it != colors.end(); ++it)
	{
	    wxXmlNode* cnode = new wxXmlNode(wxXML_ELEMENT_NODE, it->first);
	    cnode->AddAttribute("Red", wxString::Format("%d", ((xlColor*)(it->second))->red));
	    cnode->AddAttribute("Green", wxString::Format("%d", ((xlColor*)(it->second))->green));
	    cnode->AddAttribute("Blue", wxString::Format("%d", ((xlColor*)(it->second))->blue));
        node->AddChild(cnode);
	}

	return node;
}

void ColorManager::Load(wxXmlNode* colors_node)
{
	if (colors_node != nullptr)
	{
        for (wxXmlNode* c = colors_node->GetChildren(); c != nullptr; c = c->GetNext())
        {
            std::string name = c->GetName().ToStdString();
            wxString red_attr;
            c->GetAttribute("Red", &red_attr);
            wxString green_attr;
            c->GetAttribute("Green", &green_attr);
            wxString blue_attr;
            c->GetAttribute("Blue", &blue_attr);
            if( colors[name] != nullptr ) {
                delete colors[name];
            }
            colors[name] = new xlColor(wxAtoi(red_attr),wxAtoi(green_attr),wxAtoi(blue_attr));
        }
	}
}
