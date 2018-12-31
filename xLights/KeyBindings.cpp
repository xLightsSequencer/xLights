#include <wx/xml/xml.h>

#include "KeyBindings.h"
#include "xLightsVersion.h"

#include <log4cpp/Category.hh>
#include "UtilFunctions.h"

bool KeyBinding::IsShiftedKey(wxKeyCode ch)
{
    wxString c;
    c += ch;
    return wxString("~!@#$%^&*()_+{}|\":<>?").Contains(c);
}

KeyBinding::KeyBinding(wxKeyCode k, bool disabled, const std::string& type, bool control, bool alt, bool shift):
    _key(k), _type(type), _effectName(""), _effectString(""), _effectDataVersion(""), _control(control), _alt(alt),
    _shift(shift), _disabled(disabled)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (KeyBindingTypes.find(type) == KeyBindingTypes.end())
    {
        // this should never happen
        wxASSERT(false);
        _disabled = true;
        _scope = KBSCOPE_INVALID;
        logger_base.error("Keybinding type '%s' not recognised", (const char *)type.c_str());
    }
    else
    {
        _scope = KeyBindingTypes.at(type);
    }
    if (IsShiftedKey(_key))
    {
        _shift = true;
    }
}

KeyBinding::KeyBinding(const std::string& k, bool disabled, const std::string& type, bool control, bool alt, bool shift):
    _type(type), _effectName(""), _effectString(""), _effectDataVersion(""), _control(control), _alt(alt),
    _shift(shift), _disabled(disabled)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _key = DecodeKey(k);
    if (_key == WXK_NONE) _disabled = true;
    if (KeyBindingTypes.find(type) == KeyBindingTypes.end())
    {
        // this should never happen
        wxASSERT(false);
        _disabled = true;
        _scope = KBSCOPE_INVALID;
        logger_base.error("Keybinding type '%s' not recognised", (const char *)type.c_str());
    }
    else
    {
        _scope = KeyBindingTypes.at(type);
    }
    if (IsShiftedKey(_key))
    {
        _shift = true;
    }
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

    while (res.size() < 22)
    {
        res += " ";
    }

    res += ": ";

    if (_type == "EFFECT" || _type == "PRESET")
    {
        res += _type + " " + _effectName;
    }
    else
    {
        wxString t = _type;
        t.Replace("_", " ");
        t = t.Capitalize();
        res += t;
    }
    return res;
}

struct KEYCODEMAP
{
    wxKeyCode _to;
    wxKeyCode _from;
    KEYCODEMAP(wxKeyCode to, wxKeyCode from) : _to(to), _from(from) {}
};

// These are the keys that we we consider equivalent
const std::list<KEYCODEMAP> KeyEquivalents =
{
    { KEYCODEMAP((wxKeyCode)'0', WXK_NUMPAD0) },
    { KEYCODEMAP((wxKeyCode)'1', WXK_NUMPAD1) },
    { KEYCODEMAP((wxKeyCode)'2', WXK_NUMPAD2) },
    { KEYCODEMAP((wxKeyCode)'3', WXK_NUMPAD3) },
    { KEYCODEMAP((wxKeyCode)'4', WXK_NUMPAD4) },
    { KEYCODEMAP((wxKeyCode)'5', WXK_NUMPAD5) },
    { KEYCODEMAP((wxKeyCode)'6', WXK_NUMPAD6) },
    { KEYCODEMAP((wxKeyCode)'7', WXK_NUMPAD7) },
    { KEYCODEMAP((wxKeyCode)'8', WXK_NUMPAD8) },
    { KEYCODEMAP((wxKeyCode)'9', WXK_NUMPAD9) },
    { KEYCODEMAP((wxKeyCode)'*', WXK_MULTIPLY)},
    { KEYCODEMAP((wxKeyCode)'+', WXK_ADD)},
    { KEYCODEMAP((wxKeyCode)'-', WXK_SUBTRACT)},
    { KEYCODEMAP((wxKeyCode)'.', WXK_DECIMAL)},
    { KEYCODEMAP((wxKeyCode)'/', WXK_DIVIDE)},
    { KEYCODEMAP(WXK_TAB, WXK_NUMPAD_TAB)},
    { KEYCODEMAP(WXK_SPACE, WXK_NUMPAD_SPACE)},
    { KEYCODEMAP(WXK_RETURN, WXK_NUMPAD_ENTER)},
    { KEYCODEMAP(WXK_F1, WXK_NUMPAD_F1)},
    { KEYCODEMAP(WXK_F2, WXK_NUMPAD_F2)},
    { KEYCODEMAP(WXK_F3, WXK_NUMPAD_F3)},
    { KEYCODEMAP(WXK_F4, WXK_NUMPAD_F4)},
    { KEYCODEMAP(WXK_HOME, WXK_NUMPAD_HOME)},
    { KEYCODEMAP(WXK_LEFT, WXK_NUMPAD_LEFT)},
    { KEYCODEMAP(WXK_RIGHT, WXK_NUMPAD_RIGHT)},
    { KEYCODEMAP(WXK_UP, WXK_NUMPAD_UP)},
    { KEYCODEMAP(WXK_DOWN, WXK_NUMPAD_DOWN)},
    { KEYCODEMAP(WXK_PAGEUP, WXK_NUMPAD_PAGEUP)},
    { KEYCODEMAP(WXK_PAGEDOWN, WXK_NUMPAD_PAGEDOWN)},
    { KEYCODEMAP(WXK_END, WXK_NUMPAD_END)},
    { KEYCODEMAP(WXK_HOME, WXK_NUMPAD_BEGIN)},
    { KEYCODEMAP(WXK_DELETE, WXK_NUMPAD_DELETE)},
    { KEYCODEMAP((wxKeyCode)'=', WXK_NUMPAD_EQUAL)},
    { KEYCODEMAP((wxKeyCode)'*', WXK_NUMPAD_MULTIPLY)},
    { KEYCODEMAP((wxKeyCode)'+', WXK_NUMPAD_ADD)},
    { KEYCODEMAP((wxKeyCode)'-', WXK_NUMPAD_SUBTRACT)},
    { KEYCODEMAP((wxKeyCode)'.', WXK_NUMPAD_DECIMAL)},
    { KEYCODEMAP((wxKeyCode)'/', WXK_NUMPAD_DIVIDE)}
};

bool KeyBinding::IsKey(wxKeyCode key) const
{
    if (_key == key) return true;

    return false;
}

bool KeyBinding::IsEquivalentKey(wxKeyCode key) const
{
    for (auto it = KeyEquivalents.begin(); it != KeyEquivalents.end(); ++it)
    {
        if (it->_from == key && _key == it->_to)
        {
            return true;
        }
    }
    return false;
}

// These are the keys that were hard coded before 2018.28 and thus need to be inserted if they are not present
const std::map<std::string, std::string> ConvertKeys =
{
    {"OPEN_SEQUENCE", "CTRL+O"},
    {"NEW_SEQUENCE", "CTRL+N"},
    {"PAUSE", "CTRL+PAUSE"},
    {"START_OF_SONG", "+HOME"},
    {"END_OF_SONG", "+END"},
    {"SAVE_CURRENT_TAB", "CTRL+S"},
    {"MARK_SPOT", "CTRL+."},
    {"RETURN_TO_SPOT","CTRL+/"},
    {"SELECT_ALL","CTRL+SHIFT+A"},
    {"SELECT_ALL_NO_TIMING","CTRL+A"},
    {"TOGGLE_PLAY","+ "},
    {"BACKUP","+F10"},
    {"ALTERNATE_BACKUP","+F11"},
    {"SELECT_SHOW_FOLDER","+F9"},
    {"CANCEL_RENDER","+ESCAPE"}
};

void KeyBindingMap::LoadDefaults() {
    bindings.push_back(KeyBinding("F10", false, "BACKUP"));
    bindings.push_back(KeyBinding("F11", false, "ALTERNATE_BACKUP"));
    bindings.push_back(KeyBinding("F9", false, "SELECT_SHOW_FOLDER"));
    bindings.push_back(KeyBinding("", true, "LIGHTS_TOGGLE"));
    bindings.push_back(KeyBinding("o", false, "OPEN_SEQUENCE", true));
    bindings.push_back(KeyBinding("", true, "CLOSE_SEQUENCE"));
    bindings.push_back(KeyBinding("n", false, "NEW_SEQUENCE", true));
    bindings.push_back(KeyBinding("", true, "RENDER_ALL"));
    bindings.push_back(KeyBinding("", true, "PASTE_BY_CELL"));
    bindings.push_back(KeyBinding("", true, "PASTE_BY_TIME"));
    bindings.push_back(KeyBinding("", true, "SEQUENCE_SETTINGS"));
    bindings.push_back(KeyBinding("", true, "PLAY_LOOP"));
    bindings.push_back(KeyBinding("HOME", false, "START_OF_SONG"));
    bindings.push_back(KeyBinding("END", false, "END_OF_SONG"));
    bindings.push_back(KeyBinding("", true, "PLAY"));
    bindings.push_back(KeyBinding("SPACE", false, "TOGGLE_PLAY"));
    bindings.push_back(KeyBinding("", true, "STOP"));
    bindings.push_back(KeyBinding("PAUSE", false, "PAUSE"));

    bindings.push_back(KeyBinding("s", false, "SAVE_CURRENT_TAB", true));
    bindings.push_back(KeyBinding("", true, "SAVE_SEQUENCE", true));
    bindings.push_back(KeyBinding("", true, "SAVEAS_SEQUENCE", true, false, true));
    bindings.push_back(KeyBinding("t", false, "TIMING_ADD"));
    bindings.push_back(KeyBinding("s", false, "TIMING_SPLIT"));
    bindings.push_back(KeyBinding("+", false, "ZOOM_IN"));
    bindings.push_back(KeyBinding("-", false, "ZOOM_OUT"));
    bindings.push_back(KeyBinding(std::string("R"), false, "RANDOM", false, false, true));
    bindings.push_back(KeyBinding("F1", false, "EFFECT_SETTINGS_TOGGLE", true));
    bindings.push_back(KeyBinding("F8", false, "EFFECT_ASSIST_TOGGLE", true));
    bindings.push_back(KeyBinding("F2", false, "COLOR_TOGGLE", true));
    bindings.push_back(KeyBinding("F3", false, "LAYER_SETTING_TOGGLE", true));
    bindings.push_back(KeyBinding("F4", false, "LAYER_BLENDING_TOGGLE", true));
    bindings.push_back(KeyBinding("F5", false, "MODEL_PREVIEW_TOGGLE", true));
    bindings.push_back(KeyBinding("F6", false, "HOUSE_PREVIEW_TOGGLE", true));
    bindings.push_back(KeyBinding("F9", false, "EFFECTS_TOGGLE", true, true));
    bindings.push_back(KeyBinding("F7", false, "DISPLAY_ELEMENTS_TOGGLE", true));
    bindings.push_back(KeyBinding("F8", false, "JUKEBOX_TOGGLE", true));
    bindings.push_back(KeyBinding("l", false, "LOCK_EFFECT", true));
    bindings.push_back(KeyBinding("u", false, "UNLOCK_EFFECT", true));
    bindings.push_back(KeyBinding(".", false, "MARK_SPOT", true));
    bindings.push_back(KeyBinding("/", false, "RETURN_TO_SPOT", true));
    bindings.push_back(KeyBinding("", true, "EFFECT_DESCRIPTION", true));
    bindings.push_back(KeyBinding("", true, "EFFECT_ALIGN_START", true, true));
    bindings.push_back(KeyBinding("", true, "EFFECT_ALIGN_END", true, true));
    bindings.push_back(KeyBinding("", true, "EFFECT_ALIGN_BOTH", true, true));
    bindings.push_back(KeyBinding(std::string("I"), false, "INSERT_LAYER_ABOVE", true, false, true));
    bindings.push_back(KeyBinding(std::string("A"), false, "INSERT_LAYER_BELOW", true, false, true));
    bindings.push_back(KeyBinding(std::string("X"), false, "TOGGLE_ELEMENT_EXPAND", true, false, true));
    bindings.push_back(KeyBinding("F10", false, "SHOW_PRESETS", true));
    bindings.push_back(KeyBinding("F11", false, "SEARCH_TOGGLE", true));
    bindings.push_back(KeyBinding("F12", false, "PERSPECTIVES_TOGGLE", true));
    bindings.push_back(KeyBinding("F5", false, "EFFECT_UPDATE"));
    bindings.push_back(KeyBinding("", false, "COLOR_UPDATE"));
    bindings.push_back(KeyBinding("ESCAPE", false, "CANCEL_RENDER"));

    bindings.push_back(KeyBinding("l", false, "LOCK_MODEL", true));
    bindings.push_back(KeyBinding("u", false, "UNLOCK_MODEL", true));
    bindings.push_back(KeyBinding("g", false, "GROUP_MODELS", true));
    bindings.push_back(KeyBinding("", true, "WIRING_VIEW", true));
    bindings.push_back(KeyBinding("", true, "NODE_LAYOUT", true));
    bindings.push_back(KeyBinding("", true, "SAVE_LAYOUT", true, false, true));
    bindings.push_back(KeyBinding("", true, "MODEL_ALIGN_TOP", true, false, true));
    bindings.push_back(KeyBinding("", true, "MODEL_ALIGN_BOTTOM", true, false, true));
    bindings.push_back(KeyBinding("", true, "MODEL_ALIGN_LEFT", true, false, true));
    bindings.push_back(KeyBinding("", true, "MODEL_ALIGN_RIGHT", true, false, true));
    bindings.push_back(KeyBinding("", true, "MODEL_ALIGN_CENTER_VERT", true, false, true));
    bindings.push_back(KeyBinding("", true, "MODEL_ALIGN_CENTER_HORIZ", true, false, true));
    bindings.push_back(KeyBinding("", true, wxString("MODEL_DISTRIBUTE_HORIZ"), true, false, true));
    bindings.push_back(KeyBinding("", true, wxString("MODEL_DISTRIBUTE_VERT"), true, false, true));

    bindings.push_back(KeyBinding("o", false, "On", "E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=100", xlights_version_string));
    bindings.push_back(KeyBinding("u", false, "On", "E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=0", xlights_version_string));
    bindings.push_back(KeyBinding("d", false, "On", "E_TEXTCTRL_Eff_On_End=0,E_TEXTCTRL_Eff_On_Start=100", xlights_version_string));
    bindings.push_back(KeyBinding("m", false, "Morph", "", xlights_version_string));
    bindings.push_back(KeyBinding("c", false, "Curtain", "", xlights_version_string));
    bindings.push_back(KeyBinding("i", false, "Circles", "", xlights_version_string));
    bindings.push_back(KeyBinding("b", false, "Bars", "", xlights_version_string));
    bindings.push_back(KeyBinding("y", false, "Butterfly", "", xlights_version_string));
    bindings.push_back(KeyBinding("f", false, "Fire", "", xlights_version_string));
    bindings.push_back(KeyBinding("g", false, "Garlands", "", xlights_version_string));
    bindings.push_back(KeyBinding("p", false, "Pinwheel", "", xlights_version_string));
    bindings.push_back(KeyBinding("r", false, "Ripple", "", xlights_version_string));
    bindings.push_back(KeyBinding("x", false, "Text", "", xlights_version_string));
    bindings.push_back(KeyBinding("S", false, "Spirals", "", xlights_version_string, false, false, true));
    bindings.push_back(KeyBinding("w", false, "Color Wash", "", xlights_version_string));
    bindings.push_back(KeyBinding("n", false, "Snowflakes", "", xlights_version_string));
    bindings.push_back(KeyBinding("O", false, "Off", "", xlights_version_string, false, false, true));
    bindings.push_back(KeyBinding("F", false, "Fan", "", xlights_version_string, false, false, true));
}

wxString KeyBinding::EncodeKey(wxKeyCode key, bool shift)
{
    switch (key)
    {
    case WXK_ESCAPE:
        return "ESCAPE";
    case WXK_HOME:
        return "HOME";
    case WXK_END:
        return "END";
    case WXK_INSERT:
        return "INSERT";
    case WXK_SPACE:
        return "SPACE";
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
        wxASSERT(key > 32 && key < 128);
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
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

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
    if (k == "SPACE")
    {
        return WXK_SPACE;
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
    if (k == "HOME")
    {
        return WXK_HOME;
    }
    if (k == "END")
    {
        return WXK_END;
    }
    if (k == "INSERT")
    {
        return WXK_INSERT;
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

    if (k.length() != 1)
    {
        logger_base.error("KeyBinding decode key failed to decode '%s'. Taking the first character.", (const char *)k.c_str());
    }

    return (wxKeyCode)(int)k[0];
}

wxString KeyBindingMap::ParseKey(wxString key, bool& ctrl, bool& alt, bool& shift)
{
    if (key.Contains("ALT+")) alt = true;
    if (key.Contains("SHIFT+")) shift = true;
    if (key.Contains("CTRL+")) ctrl = true;
    return key.AfterLast('+');
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
                    if (k == "") disabled = true;
                    if (type == "EFFECT")
                    {
                        std::string effect = child->GetAttribute("effect").ToStdString();
                        std::string settings = "";
                        if (child->GetChildren() != nullptr) {
                            settings = child->GetChildren()->GetContent().ToStdString();
                        }
                        if (effect != "")
                        {
                            bindings.push_back(KeyBinding(k, disabled, effect, settings, child->GetAttribute("xLightsVersion", "4.0"), control, alt, shift));
                        }
                    }
                    else if (type == "PRESET")
                    {
                        std::string presetName = child->GetAttribute("effect").ToStdString();
                        bindings.push_back(KeyBinding(disabled, k, presetName, control, alt, shift));
                    }
                    else
                    {
                        bindings.push_back(KeyBinding(k, disabled, type, control, alt, shift));
                    }
                }

                child = child->GetNext();
            }

            // add in the missing essential key bindings if not present
            for (auto it = ConvertKeys.begin(); it != ConvertKeys.end(); ++it)
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
                    bool ctrl = false;
                    bool alt = false;
                    bool shift = false;
                    wxString key = ParseKey(it->second, ctrl, alt, shift);
                    bindings.push_back(KeyBinding(key, false, it->first, ctrl, alt, shift));
                }
            }

            // add in all the missing key bindings ... but disable them
            for (auto it = KeyBindingTypes.begin(); it != KeyBindingTypes.end(); ++it)
            {
                if (it->first != "EFFECT" && it->first != "PRESET")
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
                        bindings.push_back(KeyBinding(WXK_NONE, true, it->first, false, false, false));
                    }
                }
            }
        }
    }

    std::string invalid = "";
    for (auto kb : bindings)
    {
        if (kb.InScope(KBSCOPE_INVALID))
        {
            invalid = invalid + kb.GetType() + "\n";
        }
    }
    if (invalid != "")
    {
        DisplayError("Keybindings contains invalid bindings:\n\n" + invalid);
    }
}

void KeyBindingMap::Save(wxFileName &fileName) {
    wxXmlDocument doc;
    wxXmlNode *root = new wxXmlNode(wxXML_ELEMENT_NODE, "keybindings");
    doc.SetRoot(root);

    for (auto it = bindings.begin(); it != bindings.end(); ++it) {

        const KeyBinding &binding = *it;

        wxKeyCode key = binding.GetKey();

// FIXME ... once I work out why this happens this can be removed
        if (binding.GetType() == "TIMING_ADD" && (key == WXK_NONE || KeyBinding::EncodeKey(key, binding.RequiresShift()) == ""))
        {
            log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.debug("TIMING_ADD: " + binding.Description());
            DisplayError("Your keybindings appear corrupt. Can you please use the tools menu Package Logs option and send us the file so we can work out why.");
        }
// END FIXME

        wxXmlNode *child = new wxXmlNode(wxXML_ELEMENT_NODE, "keybinding");

        if (binding.IsDisabled())
        {
            child->AddAttribute("keycode", "");
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
        else if (binding.GetType() == "PRESET")
        {
            child->AddAttribute("effect", binding.GetEffectName());
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
        if (bindings[x].InScope(KBSCOPE_ALL) && !bindings[x].IsDisabled())
        {
            auto s = bindings[x].Description();
            logger_base.debug("    %s", (const char*)s.c_str());
            res += s + "\n";
        }
    }
    logger_base.debug("Scope: Layout");
    res += "\n";
    for (size_t x = 0; x < bindings.size(); x++) {
        if (bindings[x].InScope(KBSCOPE_LAYOUT) && !bindings[x].IsDisabled())
        {
            auto s = bindings[x].Description();
            logger_base.debug("    %s", (const char*)s.c_str());
            res += s + "\n";
        }
    }
    logger_base.debug("Scope: Sequencer");
    res += "\n";
    for (size_t x = 0; x < bindings.size(); x++) {
        if (bindings[x].InScope(KBSCOPE_SEQUENCE) && !bindings[x].IsDisabled())
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
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool ctrl = event.CmdDown() || event.ControlDown();
    wxKeyCode key = (wxKeyCode)event.GetKeyCode();
    bool alt = event.AltDown();
    bool shift = event.ShiftDown();

    for (size_t x = 0; x < bindings.size(); x++) {
        if (!bindings[x].IsDisabled() &&
            bindings[x].RequiresAlt() == alt &&
            bindings[x].RequiresControl() == ctrl &&
            (
                (bindings[x].RequiresShift() == shift && bindings[x].IsKey(key)) ||
                (bindings[x].IsEquivalentKey(key))
            ) &&
            bindings[x].InScope(scope)) {
            // Once we get through a couple of releases and i know i am not getting crashes as a result of these i can comment this out
            logger_base.debug("Keybinding fired: %s %s", (const char *)bindings[x].GetType().c_str(), (const char *)bindings[x].GetEffectName().c_str());
            return &bindings[x];
        }
    }
    return nullptr;
}
