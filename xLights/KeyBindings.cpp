#include <wx/xml/xml.h>

#include "KeyBindings.h"
#include "xLightsVersion.h"

#include <log4cpp/Category.hh>

bool KeyBinding::IsShiftedKey(unsigned char ch)
{
    wxString c;
    c += ch;
    return wxString("~!@#$%^&*()_+{}|\":<>?").Contains(c);
}

void KeyBindingMap::LoadDefaults() {
    bindings.push_back(KeyBinding('t', "TIMING_ADD"));
    bindings.push_back(KeyBinding('s', "TIMING_SPLIT"));
    bindings.push_back(KeyBinding('+', "ZOOM_IN"));
    bindings.push_back(KeyBinding('-', "ZOOM_OUT"));
    bindings.push_back(KeyBinding('R', "RANDOM", false, false));
    bindings.push_back(KeyBinding('l', "LIGHTS_TOGGLE", true));
    bindings.push_back(KeyBinding('o', "OPEN_SEQUENCE", true));
    bindings.push_back(KeyBinding('X', "CLOSE_SEQUENCE", true, false));
    bindings.push_back(KeyBinding('s', "SAVE_SEQUENCE", true));
    bindings.push_back(KeyBinding('n', "NEW_SEQUENCE", true));
    bindings.push_back(KeyBinding('S', "SAVEAS_SEQUENCE", true, false));
    bindings.push_back(KeyBinding('C', "PASTE_BY_CELL", true, false));
    bindings.push_back(KeyBinding('T', "PASTE_BY_TIME", true, false));
    bindings.push_back(KeyBinding('e', "EFFECT_SETTINGS_TOGGLE", true, true));
    bindings.push_back(KeyBinding('a', "EFFECT_ASSIST_TOGGLE", true, true));
    bindings.push_back(KeyBinding('c', "COLOR_TOGGLE", true, true));
    bindings.push_back(KeyBinding('l', "LAYER_SETTING_TOGGLE", true, true));
    bindings.push_back(KeyBinding('b', "LAYER_BLENDING_TOGGLE", true, true));
    bindings.push_back(KeyBinding('m', "MODEL_PREVIEW_TOGGLE", true, true));
    bindings.push_back(KeyBinding('h', "HOUSE_PREVIEW_TOGGLE", true, true));
    bindings.push_back(KeyBinding('E', "EFFECTS_TOGGLE", true, true));
    bindings.push_back(KeyBinding('d', "DISPLAY_ELEMENTS_TOGGLE", true, true));
    bindings.push_back(KeyBinding('j', "JUKEBOX_TOGGLE", true, true));
    bindings.push_back(KeyBinding('s', "SEQUENCE_SETTINGS", true, true));
    bindings.push_back(KeyBinding('l', "PLAY_LOOP"));
    bindings.push_back(KeyBinding(' ', "PLAY"));
    bindings.push_back(KeyBinding('t', "STOP", true));
    bindings.push_back(KeyBinding('p', "PAUSE", true));

    bindings.push_back(KeyBinding('o', "On", "E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=100", xlights_version_string));
    bindings.push_back(KeyBinding('u', "On", "E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=0", xlights_version_string));
    bindings.push_back(KeyBinding('d', "On", "E_TEXTCTRL_Eff_On_End=0,E_TEXTCTRL_Eff_On_Start=100", xlights_version_string));
    bindings.push_back(KeyBinding('m', "Morph", "", xlights_version_string));
    bindings.push_back(KeyBinding('c', "Curtain", "", xlights_version_string));
    bindings.push_back(KeyBinding('i', "Circles", "", xlights_version_string));
    bindings.push_back(KeyBinding('b', "Bars", "", xlights_version_string));
    bindings.push_back(KeyBinding('y', "Butterfly", "", xlights_version_string));
    bindings.push_back(KeyBinding('f', "Fire", "", xlights_version_string));
    bindings.push_back(KeyBinding('g', "Garlands", "", xlights_version_string));
    bindings.push_back(KeyBinding('p', "Pinwheel", "", xlights_version_string));
    bindings.push_back(KeyBinding('r', "Ripple", "", xlights_version_string));
    bindings.push_back(KeyBinding('x', "Text", "", xlights_version_string));
    bindings.push_back(KeyBinding('S', "Spirals", "", xlights_version_string, false, false));
    bindings.push_back(KeyBinding('w', "Color Wash", "", xlights_version_string));
    bindings.push_back(KeyBinding('n', "Snowflakes", "", xlights_version_string));
	bindings.push_back(KeyBinding('O', "Off", "", xlights_version_string, false, false));
	bindings.push_back(KeyBinding('F', "Fan", "", xlights_version_string, false, false));
}

void KeyBindingMap::Load(wxFileName &fileName) {
    if (fileName.Exists()) {
        wxXmlDocument doc;
        if (doc.Load(fileName.GetFullPath())) {
            bindings.clear();
            wxXmlNode *root = doc.GetRoot();
            wxXmlNode *child = root->GetChildren();

            while (child != nullptr) {
                if ("keybinding" == child->GetName() && child->GetAttribute("key") != "") {
                    std::string type = child->GetAttribute("type").ToStdString();
                    unsigned char k = child->GetAttribute("key")[0];
                    bool control = child->GetAttribute("control", "FALSE") == "TRUE";
                    bool alt = child->GetAttribute("alt", "FALSE") == "TRUE";
                    if (type == "EFFECT")
                    {
                        std::string effect = child->GetAttribute("effect").ToStdString();
                        std::string settings = "";
                        if (child->GetChildren() != nullptr) {
                            settings = child->GetChildren()->GetContent().ToStdString();
                        }
                        bindings.push_back(KeyBinding(k, effect, settings, child->GetAttribute("xLightsVersion", "4.0"), control, alt));
                    }
                    else
                    {
                        bindings.push_back(KeyBinding(k, type, control, alt));
                    } 
                }

                child = child->GetNext();
            }
        }
    }
}

void KeyBindingMap::Save(wxFileName &fileName) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxXmlDocument doc;
    wxXmlNode *root = new wxXmlNode(wxXML_ELEMENT_NODE, "keybindings");
    doc.SetRoot(root);

    for (auto it = bindings.begin(); it != bindings.end(); ++it) {

        const KeyBinding &binding = *it;

        if (binding.GetKey() < 32 || binding.GetKey() > 127)
        {
            // This looks extremely suspicious
            logger_base.warn("KeyBindings saving element type %s has odd looking key assigned 0x%02x.", (const char*)binding.GetType().c_str(), (int)binding.GetKey());
        }
        
        wxXmlNode *child = new wxXmlNode(wxXML_ELEMENT_NODE, "keybinding");

        unsigned char key = binding.GetKey();
        if (binding.RequiresShift() && key >= 'a' && key <= 'z')
        {
            key -= 32;
        }
        else if (!binding.RequiresShift() && key >= 'A' && key <= 'Z')
        {
            key += 32;
        }

        child->AddAttribute("key", wxString::Format("%c", key));
        child->AddAttribute("alt", binding.RequiresAlt() ? "TRUE" : "FALSE");
        child->AddAttribute("control", binding.RequiresControl() ? "TRUE" : "FALSE");
        child->AddAttribute("type", binding.GetType());
        if (binding.GetType() == "EFFECT")
        {
            child->AddAttribute("type", "EFFECT");
            child->AddAttribute("effect", binding.GetEffectName());
            child->AddAttribute("xLightsVersion", binding.GetEffectDataVersion());
            child->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", binding.GetEffectString()));
        }
        root->AddChild(child);
    }
    doc.Save(fileName.GetFullPath());
}

KeyBinding* KeyBindingMap::Find(unsigned char key, bool control, bool alt) {
    for (size_t x = 0; x < bindings.size(); x++) {
        if (bindings[x].GetKey() == key && 
            bindings[x].RequiresAlt() == alt && 
            bindings[x].RequiresControl() == control) {
            return &bindings[x];
        }
    }
    return nullptr;
}

KeyBinding* KeyBindingMap::Find(wxKeyEvent& event)
{
    bool ctrl = event.CmdDown() || event.ControlDown();
    auto key = event.GetUnicodeKey();
    bool alt = event.AltDown();
    bool shift = event.ShiftDown();

    for (size_t x = 0; x < bindings.size(); x++) {
        if (bindings[x].GetKey() == key &&
            bindings[x].RequiresAlt() == alt &&
            bindings[x].RequiresControl() == ctrl &&
            bindings[x].RequiresShift() == shift) {
            return &bindings[x];
        }
    }
    return nullptr;
}