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

std::string KeyBinding::Description() const
{
    std::string res;
    if (_disabled)
    {
        res += "N/A";
    }
    else
    {
        if (_control)
        {
            res += "CTRL+";
        }
        if (_alt)
        {
            res += "ALT+";
        }
        if (!_shift && _key >= 'A' && _key <= 'Z')
        {
            res += "'";
            res += (_key + 32);
            res += "'";
        }
        else
        {
            res += "'";
            res += _key;
            res += "'";
        }
    }
    res += "   : ";

    if (_type == "EFFECT")
    {
        res += _effectName;
    }
    else
    {
        res += _type;
    }
    return res;
}

void KeyBindingMap::LoadDefaults() {
    bindings.push_back(KeyBinding('L', false, "LIGHTS_TOGGLE", true));
    bindings.push_back(KeyBinding('o', false, "OPEN_SEQUENCE", true));
    bindings.push_back(KeyBinding('X', false, "CLOSE_SEQUENCE", true, false));
    bindings.push_back(KeyBinding('n', false, "NEW_SEQUENCE", true));
    bindings.push_back(KeyBinding('C', false, "PASTE_BY_CELL", true, false));
    bindings.push_back(KeyBinding('T', false, "PASTE_BY_TIME", true, false));
    bindings.push_back(KeyBinding('s', false, "SEQUENCE_SETTINGS", true, true));
    bindings.push_back(KeyBinding('l', false, "PLAY_LOOP"));
    bindings.push_back(KeyBinding(' ', false, "PLAY"));
    bindings.push_back(KeyBinding('t', false, "STOP", true));
    bindings.push_back(KeyBinding('p', false, "PAUSE", true));

    bindings.push_back(KeyBinding('s', true, "SAVE_SEQUENCE", true));
    bindings.push_back(KeyBinding('S', true, "SAVEAS_SEQUENCE", true, false));
    bindings.push_back(KeyBinding('t', false, "TIMING_ADD"));
    bindings.push_back(KeyBinding('s', false, "TIMING_SPLIT"));
    bindings.push_back(KeyBinding('+', false, "ZOOM_IN"));
    bindings.push_back(KeyBinding('-', false, "ZOOM_OUT"));
    bindings.push_back(KeyBinding('R', false, "RANDOM", false, false));
    bindings.push_back(KeyBinding('e', false, "EFFECT_SETTINGS_TOGGLE", true, true));
    bindings.push_back(KeyBinding('a', false, "EFFECT_ASSIST_TOGGLE", true, true));
    bindings.push_back(KeyBinding('c', false, "COLOR_TOGGLE", true, true));
    bindings.push_back(KeyBinding('y', false, "LAYER_SETTING_TOGGLE", true, true));
    bindings.push_back(KeyBinding('b', false, "LAYER_BLENDING_TOGGLE", true, true));
    bindings.push_back(KeyBinding('m', false, "MODEL_PREVIEW_TOGGLE", true, true));
    bindings.push_back(KeyBinding('h', false, "HOUSE_PREVIEW_TOGGLE", true, true));
    bindings.push_back(KeyBinding('E', false, "EFFECTS_TOGGLE", true, true));
    bindings.push_back(KeyBinding('d', false, "DISPLAY_ELEMENTS_TOGGLE", true, true));
    bindings.push_back(KeyBinding('j', false, "JUKEBOX_TOGGLE", true, true));
    bindings.push_back(KeyBinding('L', false, "LOCK_EFFECT", true, false));
    bindings.push_back(KeyBinding('U', false, "UNLOCK_EFFECT", true, false));
    bindings.push_back(KeyBinding('.', false, "MARK_SPOT", true, false));
    bindings.push_back(KeyBinding('/', false, "RETURN_TO_SPOT", true, false));
    bindings.push_back(KeyBinding('~', false, "EFFECT_DESCRIPTION", true, false));
    bindings.push_back(KeyBinding('1', false, "EFFECT_ALIGN_START", true, true));
    bindings.push_back(KeyBinding('2', false, "EFFECT_ALIGN_END", true, true));
    bindings.push_back(KeyBinding('3', false, "EFFECT_ALIGN_BOTH", true, true));
    bindings.push_back(KeyBinding('4', false, "INSERT_LAYER_ABOVE", true, true));
    bindings.push_back(KeyBinding('5', false, "INSERT_LAYER_BELOW", true, true));
    bindings.push_back(KeyBinding('6', false, "TOGGLE_ELEMENT_EXPAND", true, true));
    bindings.push_back(KeyBinding('7', false, "SHOW_PRESETS", true, true));
    bindings.push_back(KeyBinding('8', false, "SEARCH_TOGGLE", true, true));
    bindings.push_back(KeyBinding('9', false, "PERSPECTIVES_TOGGLE", true, true));
 
    bindings.push_back(KeyBinding('L', false, "LOCK_MODEL", true, false));
    bindings.push_back(KeyBinding('U', false, "UNLOCK_MODEL", true, false));
    bindings.push_back(KeyBinding('g', false, "GROUP_MODELS", true, false));
    bindings.push_back(KeyBinding('w', false, "WIRING_VIEW", true, false));
    bindings.push_back(KeyBinding('N', false, "NODE_LAYOUT", true, false));
    bindings.push_back(KeyBinding('S', true, "SAVE_LAYOUT", true, false));
    bindings.push_back(KeyBinding('T', false, "MODEL_ALIGN_TOP", true, false));
    bindings.push_back(KeyBinding('B', false, "MODEL_ALIGN_BOTTOM", true, false));
    bindings.push_back(KeyBinding('L', false, "MODEL_ALIGN_LEFT", true, false));
    bindings.push_back(KeyBinding('R', false, "MODEL_ALIGN_RIGHT", true, false));
    bindings.push_back(KeyBinding('V', false, "MODEL_ALIGN_CENTER_VERT", true, false));
    bindings.push_back(KeyBinding('H', false, "MODEL_ALIGN_CENTER_HORIZ", true, false));
    bindings.push_back(KeyBinding('D', false, "MODEL_DISTRIBUTE_HORIZ", true, false));
    bindings.push_back(KeyBinding('E', false, "MODEL_DISTRIBUTE_VERT", true, false));

    bindings.push_back(KeyBinding('o', false, "On", "E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=100", xlights_version_string));
    bindings.push_back(KeyBinding('u', false, "On", "E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=0", xlights_version_string));
    bindings.push_back(KeyBinding('d', false, "On", "E_TEXTCTRL_Eff_On_End=0,E_TEXTCTRL_Eff_On_Start=100", xlights_version_string));
    bindings.push_back(KeyBinding('m', false, "Morph", "", xlights_version_string));
    bindings.push_back(KeyBinding('c', false, "Curtain", "", xlights_version_string));
    bindings.push_back(KeyBinding('i', false, "Circles", "", xlights_version_string));
    bindings.push_back(KeyBinding('b', false, "Bars", "", xlights_version_string));
    bindings.push_back(KeyBinding('y', false, "Butterfly", "", xlights_version_string));
    bindings.push_back(KeyBinding('f', false, "Fire", "", xlights_version_string));
    bindings.push_back(KeyBinding('g', false, "Garlands", "", xlights_version_string));
    bindings.push_back(KeyBinding('p', false, "Pinwheel", "", xlights_version_string));
    bindings.push_back(KeyBinding('r', false, "Ripple", "", xlights_version_string));
    bindings.push_back(KeyBinding('x', false, "Text", "", xlights_version_string));
    bindings.push_back(KeyBinding('S', false, "Spirals", "", xlights_version_string, false, false));
    bindings.push_back(KeyBinding('w', false, "Color Wash", "", xlights_version_string));
    bindings.push_back(KeyBinding('n', false, "Snowflakes", "", xlights_version_string));
    bindings.push_back(KeyBinding('O', false, "Off", "", xlights_version_string, false, false));
    bindings.push_back(KeyBinding('F', false, "Fan", "", xlights_version_string, false, false));
}

void KeyBindingMap::Load(wxFileName &fileName) {
    if (fileName.Exists()) {
        wxXmlDocument doc;
        if (doc.Load(fileName.GetFullPath())) {
            bindings.clear();
            wxXmlNode *root = doc.GetRoot();
            wxXmlNode *child = root->GetChildren();

            while (child != nullptr) {
                if ("keybinding" == child->GetName()) {
                    std::string type = child->GetAttribute("type").ToStdString();
                    bool disabled = false;
                    unsigned char k = '~';
                    if (child->GetAttribute("key") == "")
                    {
                        disabled = true;
                    }
                    else
                    {
                        k = child->GetAttribute("key")[0];
                    }
                    bool control = child->GetAttribute("control", "FALSE") == "TRUE";
                    bool alt = child->GetAttribute("alt", "FALSE") == "TRUE";
                    if (type == "EFFECT")
                    {
                        std::string effect = child->GetAttribute("effect").ToStdString();
                        std::string settings = "";
                        if (child->GetChildren() != nullptr) {
                            settings = child->GetChildren()->GetContent().ToStdString();
                        }
                        bindings.push_back(KeyBinding(k, disabled, effect, settings, child->GetAttribute("xLightsVersion", "4.0"), control, alt));
                    }
                    else
                    {
                        bindings.push_back(KeyBinding(k, disabled, type, control, alt));
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

        if (binding.IsDisabled())
        {
            child->AddAttribute("key", "");
        }
        else
        {
            child->AddAttribute("key", wxString::Format("%c", key));
        }
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

KeyBinding* KeyBindingMap::Find(unsigned char key, bool control, bool alt, KBSCOPE scope) {
    for (size_t x = 0; x < bindings.size(); x++) {
        if (!bindings[x].IsDisabled() && 
            bindings[x].GetKey() == key &&
            bindings[x].RequiresAlt() == alt && 
            bindings[x].RequiresControl() == control &&
            bindings[x].InScope(scope)) {
            return &bindings[x];
        }
    }
    return nullptr;
}

std::string KeyBindingMap::Dump()
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string res;
    logger_base.debug("Dumping key map");
    logger_base.debug("Scope: Everywhere");
    for (size_t x = 0; x < bindings.size(); x++) {
        if (bindings[x].InScope(KBSCOPE_ALL))
        {
            auto s = bindings[x].Description();
            logger_base.debug("    %s", (const char*)s.c_str());
            res += s + "\n";
        }
    }
    logger_base.debug("Scope: Layout");
    res += "\n";
    for (size_t x = 0; x < bindings.size(); x++) {
        if (bindings[x].InScope(KBSCOPE_LAYOUT))
        {
            auto s = bindings[x].Description();
            logger_base.debug("    %s", (const char*)s.c_str());
            res += s + "\n";
        }
    }
    logger_base.debug("Scope: Sequencer");
    res += "\n";
    for (size_t x = 0; x < bindings.size(); x++) {
        if (bindings[x].InScope(KBSCOPE_SEQUENCE))
        {
            auto s = bindings[x].Description();
            logger_base.debug("    %s", (const char*)s.c_str());
            res += s + "\n";
        }
    }
    return res;
}

KeyBinding* KeyBindingMap::Find(wxKeyEvent& event, KBSCOPE scope)
{
    bool ctrl = event.CmdDown() || event.ControlDown();
    auto key = event.GetUnicodeKey();
    bool alt = event.AltDown();
    bool shift = event.ShiftDown();

    for (size_t x = 0; x < bindings.size(); x++) {
        if (!bindings[x].IsDisabled() &&
            bindings[x].GetKey() == key &&
            bindings[x].RequiresAlt() == alt &&
            bindings[x].RequiresControl() == ctrl &&
            bindings[x].RequiresShift() == shift &&
            bindings[x].InScope(scope)) {
            return &bindings[x];
        }
    }
    return nullptr;
}
