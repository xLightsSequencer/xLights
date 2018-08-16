#include <wx/xml/xml.h>

#include "KeyBindings.h"
#include "xLightsVersion.h"

#include <log4cpp/Category.hh>

bool KeyBinding::IsShiftedKey(wxKeyCode ch)
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
        bool s = _shift;
        if (s && IsShiftedKey(_key)) s = false;
        if (s)
        {
            res += "SHIFT+";
        }
        res += "'";
        res += EncodeKey(_key, _shift);
        res += "'";
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
    bindings.push_back(KeyBinding('l', false, "LIGHTS_TOGGLE", true));
    bindings.push_back(KeyBinding('o', false, "OPEN_SEQUENCE", true));
    bindings.push_back(KeyBinding('X', false, "CLOSE_SEQUENCE", true, false, true));
    bindings.push_back(KeyBinding('n', false, "NEW_SEQUENCE", true));
    bindings.push_back(KeyBinding('C', false, "PASTE_BY_CELL", true, false, true));
    bindings.push_back(KeyBinding('T', false, "PASTE_BY_TIME", true, false, true));
    bindings.push_back(KeyBinding('s', false, "SEQUENCE_SETTINGS", true, true));
    bindings.push_back(KeyBinding('l', false, "PLAY_LOOP"));
    bindings.push_back(KeyBinding(' ', false, "PLAY"));
    bindings.push_back(KeyBinding('t', false, "STOP", true));
    bindings.push_back(KeyBinding('p', false, "PAUSE", true));

    bindings.push_back(KeyBinding('s', false, "SAVE_CURRENT_TAB", true));
    bindings.push_back(KeyBinding('s', true, "SAVE_SEQUENCE", true));
    bindings.push_back(KeyBinding('S', true, "SAVEAS_SEQUENCE", true, false, true));
    bindings.push_back(KeyBinding('t', false, "TIMING_ADD"));
    bindings.push_back(KeyBinding('s', false, "TIMING_SPLIT"));
    bindings.push_back(KeyBinding('+', false, "ZOOM_IN"));
    bindings.push_back(KeyBinding('-', false, "ZOOM_OUT"));
    bindings.push_back(KeyBinding('R', false, "RANDOM", false, false, true));
    bindings.push_back(KeyBinding('e', false, "EFFECT_SETTINGS_TOGGLE", true, true));
    bindings.push_back(KeyBinding('a', false, "EFFECT_ASSIST_TOGGLE", true, true));
    bindings.push_back(KeyBinding('c', false, "COLOR_TOGGLE", true, true));
    bindings.push_back(KeyBinding('y', false, "LAYER_SETTING_TOGGLE", true, true));
    bindings.push_back(KeyBinding('b', false, "LAYER_BLENDING_TOGGLE", true, true));
    bindings.push_back(KeyBinding('m', false, "MODEL_PREVIEW_TOGGLE", true, true));
    bindings.push_back(KeyBinding('h', false, "HOUSE_PREVIEW_TOGGLE", true, true));
    bindings.push_back(KeyBinding('E', false, "EFFECTS_TOGGLE", true, true, true));
    bindings.push_back(KeyBinding('d', false, "DISPLAY_ELEMENTS_TOGGLE", true, true));
    bindings.push_back(KeyBinding('j', false, "JUKEBOX_TOGGLE", true, true));
    bindings.push_back(KeyBinding('L', false, "LOCK_EFFECT", true, false, true));
    bindings.push_back(KeyBinding('U', false, "UNLOCK_EFFECT", true, false, true));
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
 
    bindings.push_back(KeyBinding('L', false, "LOCK_MODEL", true, false, true));
    bindings.push_back(KeyBinding('U', false, "UNLOCK_MODEL", true, false, true));
    bindings.push_back(KeyBinding('g', false, "GROUP_MODELS", true, false));
    bindings.push_back(KeyBinding('w', false, "WIRING_VIEW", true, false, true));
    bindings.push_back(KeyBinding('N', false, "NODE_LAYOUT", true, false, true));
    bindings.push_back(KeyBinding('S', true, "SAVE_LAYOUT", true, false, true));
    bindings.push_back(KeyBinding('T', false, "MODEL_ALIGN_TOP", true, false, true));
    bindings.push_back(KeyBinding('B', false, "MODEL_ALIGN_BOTTOM", true, false, true));
    bindings.push_back(KeyBinding('F', false, "MODEL_ALIGN_LEFT", true, false, true));
    bindings.push_back(KeyBinding('R', false, "MODEL_ALIGN_RIGHT", true, false, true));
    bindings.push_back(KeyBinding('V', false, "MODEL_ALIGN_CENTER_VERT", true, false, true));
    bindings.push_back(KeyBinding('H', false, "MODEL_ALIGN_CENTER_HORIZ", true, false, true));
    bindings.push_back(KeyBinding('D', false, "MODEL_DISTRIBUTE_HORIZ", true, false, true));
    bindings.push_back(KeyBinding('E', false, "MODEL_DISTRIBUTE_VERT", true, false, true));

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
    bindings.push_back(KeyBinding('S', false, "Spirals", "", xlights_version_string, false, false, true));
    bindings.push_back(KeyBinding('w', false, "Color Wash", "", xlights_version_string));
    bindings.push_back(KeyBinding('n', false, "Snowflakes", "", xlights_version_string));
    bindings.push_back(KeyBinding('O', false, "Off", "", xlights_version_string, false, false, true));
    bindings.push_back(KeyBinding("F", false, "Fan", "", xlights_version_string, false, false, true));
}

wxString KeyBinding::EncodeKey(wxKeyCode key, bool shift)
{
    switch (key)
    {
    case WXK_ESCAPE:
        return "ESCAPE";
    case WXK_DELETE:
        return "DELETE";
    case WXK_BACK:
        return "BACKSPACE";
    case WXK_TAB:
        return "TAB";
    case WXK_DOWN:
        return "DOWN";
    case WXK_UP:
        return "UP";
    case WXK_LEFT:
        return "LEFT";
    case WXK_RIGHT:
        return "RIGHT";
    case WXK_PAGEUP:
        return "PGUP";
    case WXK_PAGEDOWN:
        return "PGDOWN";
    case WXK_PAUSE:
        return "PAUSE";
    case WXK_RETURN:
        return "ENTER";
    case WXK_F1:
        return "F1";
    case WXK_F2:
        return "F2";
    case WXK_F3:
        return "F3";
    case WXK_F4:
        return "F4";
    case WXK_F5:
        return "F5";
    case WXK_F6:
        return "F6";
    case WXK_F7:
        return "F7"; 
    case WXK_F8:
        return "F8";
    case WXK_F9:
        return "F9"; 
    case WXK_F10:
        return "F10"; 
    case WXK_F11:
        return "F11";
    case WXK_F12:
        return "F12"; 
    default:
        auto s = wxString((char)key);
        if (shift)
        {
            return s.Upper();
        }
        else
        {
            return s.Lower();
        }
    }
}

wxKeyCode KeyBinding::DecodeKey(wxString key)
{
    wxString k = key.Upper();
    if (k == "")
    {
        return WXK_NONE;
    }
    if (k == "ESC" || k == "ESCAPE")
    {
        return WXK_ESCAPE;
    }
    if (k == "DEL" || k == "DELETE")
    {
        return WXK_DELETE;
    }
    if (k == "BACK" || k == "BACKSPACE")
    {
        return WXK_BACK;
    }
    if (k == "TAB")
    {
        return WXK_TAB;
    }
    if (k == "DOWN")
    {
        return WXK_DOWN;
    }
    if (k == "UP")
    {
        return WXK_UP;
    }
    if (k == "LEFT")
    {
        return WXK_LEFT;
    }
    if (k == "RIGHT")
    {
        return WXK_RIGHT;
    }
    if (k == "PGUP" || k == "PAGEUP")
    {
        return WXK_PAGEUP;
    }
    if (k == "PGDN" || k == "PAGEDOWN")
    {
        return WXK_PAGEDOWN;
    }
    if (k == "PAUSE")
    {
        return WXK_PAUSE;
    }
    if (k == "RETURN" || k == "ENTER")
    {
        return WXK_RETURN;
    }
    if (k == "F1")
    {
        return WXK_F1;
    }
    if (k == "F2")
    {
        return WXK_F2;
    }
    if (k == "F3")
    {
        return WXK_F3;
    }
    if (k == "F4")
    {
        return WXK_F4;
    }
    if (k == "F5")
    {
        return WXK_F5;
    }
    if (k == "F6")
    {
        return WXK_F6;
    }
    if (k == "F7")
    {
        return WXK_F7;
    }
    if (k == "F8")
    {
        return WXK_F8;
    }
    if (k == "F9")
    {
        return WXK_F9;
    }
    if (k == "F10")
    {
        return WXK_F10;
    }
    if (k == "F11")
    {
        return WXK_F11;
    }
    if (k == "F12")
    {
        return WXK_F12;
    }

    return (wxKeyCode)(int)k[0];
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
                    wxString k = child->GetAttribute("keycode");
                    wxString oldk = child->GetAttribute("key");
                    bool control = child->GetAttribute("control", "FALSE") == "TRUE";
                    bool alt = child->GetAttribute("alt", "FALSE") == "TRUE";
                    bool shift = child->GetAttribute("shift", "FALSE") == "TRUE";
                    if (oldk != "")
                    {
                        k = oldk;
                        if (k >= 'A' && k <= 'Z') shift = true; else shift = false;
                    }
                    if (type == "EFFECT")
                    {
                        std::string effect = child->GetAttribute("effect").ToStdString();
                        std::string settings = "";
                        if (child->GetChildren() != nullptr) {
                            settings = child->GetChildren()->GetContent().ToStdString();
                        }
                        bindings.push_back(KeyBinding(k, disabled, effect, settings, child->GetAttribute("xLightsVersion", "4.0"), control, alt, shift));
                    }
                    else
                    {
                        bindings.push_back(KeyBinding(k, disabled, type, control, alt, shift));
                    } 
                }

                child = child->GetNext();
            }

            // add in all the missing key bindings ... but disable them
            for (auto it = KeyBindingTypes.begin(); it != KeyBindingTypes.end(); ++it)
            {
                if (it->first != "EFFECT")
                {
                    bool found = false;
                    for (auto it2 = bindings.begin(); it2 != bindings.end(); ++it2)
                    {
                        if (it2->GetType() == it->first)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        bindings.push_back(KeyBinding(WXK_F1, true, it->first, false, false, false));
                    }
                }
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

        wxXmlNode *child = new wxXmlNode(wxXML_ELEMENT_NODE, "keybinding");

        wxKeyCode key = binding.GetKey();
        if (binding.IsDisabled())
        {
            child->AddAttribute("key", "");
        }
        else
        {
            child->AddAttribute("keycode", KeyBinding::EncodeKey(key, binding.RequiresShift()));
        }
        child->AddAttribute("alt", binding.RequiresAlt() ? "TRUE" : "FALSE");
        child->AddAttribute("control", binding.RequiresControl() ? "TRUE" : "FALSE");
        child->AddAttribute("shift", binding.RequiresShift() ? "TRUE" : "FALSE");
        child->AddAttribute("type", binding.GetType());
        if (binding.GetType() == "EFFECT")
        {
            child->AddAttribute("effect", binding.GetEffectName());
            child->AddAttribute("xLightsVersion", binding.GetEffectDataVersion());
            child->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", binding.GetEffectString()));
        }
        root->AddChild(child);
    }
    doc.Save(fileName.GetFullPath());
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
    auto key = event.GetKeyCode();
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
