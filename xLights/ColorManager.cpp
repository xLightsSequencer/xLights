#include "ColorManager.h"
#include "xLightsMain.h"
#include <wx/xml/xml.h>

ColorManager::ColorManager(xLightsFrame* frame)
: xlights(frame)
{
    ResetDefaults();
}

ColorManager::~ColorManager()
{
    colors.clear();
    colors_backup.clear();
}

void ColorManager::ResetDefaults()
{
    colors["Timing1"] = xlCYAN;
    colors["Timing2"] = xlRED;
    colors["Timing3"] = xlGREEN;
    colors["Timing4"] = xlBLUE;
    colors["Timing5"] = xlYELLOW;

    colors["TimingDefault"] = xlWHITE;
    colors["EffectDefault"] = xlColor(192,192,192);
    colors["EffectSelected"] = xlColor(204, 102, 255);
    colors["ReferenceEffect"] = xlColor(255,0,255);
    colors["Gridlines"] = xlColor(40,40,40);
    colors["Labels"] = xlColor(255,255,204);
    colors["LabelOutline"] = xlColor(103, 103, 103);
    colors["Phrases"] = xlColor(153, 255, 153);
    colors["Words"] = xlColor(255, 218, 145);
    colors["Phonemes"] = xlColor(255, 181, 218);
    colors["RowHeader"] = xlColor(212,208,200);
    colors["RowHeaderSelected"] = xlColor(130,178,207);

    colors["ModelSelected"] = xlYELLOW;
}

void ColorManager::Snapshot()
{
	// store a new snapshot
    for (auto it = colors.begin(); it != colors.end(); ++it)
	{
        colors_backup[it->first] = it->second;
	}
}

void ColorManager::RestoreSnapshot()
{
    for (auto it = colors_backup.begin(); it != colors_backup.end(); ++it)
	{
        colors[it->first] = it->second;
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
}

xlColor ColorManager::GetColor(std::string name)
{
    return colors[name];
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

	for (auto it = colors.begin(); it != colors.end(); ++it)
	{
	    wxXmlNode* cnode = new wxXmlNode(wxXML_ELEMENT_NODE, it->first);
	    cnode->AddAttribute("Red", wxString::Format("%d", it->second.red));
	    cnode->AddAttribute("Green", wxString::Format("%d", it->second.green));
	    cnode->AddAttribute("Blue", wxString::Format("%d", it->second.blue));
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
            colors[name] = xlColor(wxAtoi(red_attr),wxAtoi(green_attr),wxAtoi(blue_attr));
        }
	}
}
