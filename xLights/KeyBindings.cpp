#include <wx/xml/xml.h>

#include "KeyBindings.h"
#include "xLightsVersion.h"
#include "UtilFunctions.h"

#include <log4cpp/Category.hh>

#pragma region Constants

static  std::vector<std::pair<std::string, KBSCOPE>> KeyBindingTypes = 
{
    { _("TIMING_ADD"), KBSCOPE::Sequence },
    { _("TIMING_SPLIT"), KBSCOPE::Sequence },
    { _("ZOOM_IN"), KBSCOPE::Sequence },
    { _("ZOOM_OUT"), KBSCOPE::Sequence },
    { _("RANDOM"), KBSCOPE::Sequence },
    { _("RENDER_ALL"), KBSCOPE::All },
    { _("SAVE_CURRENT_TAB"), KBSCOPE::All },
    { _("LIGHTS_TOGGLE"), KBSCOPE::All },
    { _("OPEN_SEQUENCE"), KBSCOPE::All },
    { _("CLOSE_SEQUENCE"), KBSCOPE::All },
    { _("NEW_SEQUENCE"), KBSCOPE::All },
    { _("PASTE_BY_CELL"), KBSCOPE::All },
    { _("PASTE_BY_TIME"), KBSCOPE::All },
    { _("BACKUP"), KBSCOPE::All },
    { _("ALTERNATE_BACKUP"), KBSCOPE::All },
    { _("SELECT_SHOW_FOLDER"), KBSCOPE::All },
    { _("SAVEAS_SEQUENCE"), KBSCOPE::Sequence },
    { _("SAVE_SEQUENCE"), KBSCOPE::Sequence },
    { _("EFFECT_SETTINGS_TOGGLE"), KBSCOPE::Sequence },
    { _("EFFECT_ASSIST_TOGGLE"), KBSCOPE::Sequence },
    { _("COLOR_TOGGLE"), KBSCOPE::Sequence },
    { _("LAYER_SETTING_TOGGLE"), KBSCOPE::Sequence },
    { _("LAYER_BLENDING_TOGGLE"), KBSCOPE::Sequence },
    { _("MODEL_PREVIEW_TOGGLE"), KBSCOPE::Sequence },
    { _("HOUSE_PREVIEW_TOGGLE"), KBSCOPE::Sequence },
    { _("EFFECTS_TOGGLE"), KBSCOPE::Sequence },
    { _("DISPLAY_ELEMENTS_TOGGLE"), KBSCOPE::Sequence },
    { _("JUKEBOX_TOGGLE"), KBSCOPE::Sequence },
    { _("SEQUENCE_SETTINGS"), KBSCOPE::All },
    { _("LOCK_EFFECT"), KBSCOPE::Sequence },
    { _("UNLOCK_EFFECT"), KBSCOPE::Sequence },
    { _("MARK_SPOT"), KBSCOPE::Sequence },
    { _("RETURN_TO_SPOT"), KBSCOPE::Sequence },
    { _("EFFECT_DESCRIPTION"), KBSCOPE::Sequence },
    { _("EFFECT_ALIGN_START"), KBSCOPE::Sequence },
    { _("EFFECT_ALIGN_END"), KBSCOPE::Sequence },
    { _("EFFECT_ALIGN_BOTH"), KBSCOPE::Sequence },
    { _("INSERT_LAYER_ABOVE"), KBSCOPE::Sequence },
    { _("INSERT_LAYER_BELOW"), KBSCOPE::Sequence },
    { _("TOGGLE_ELEMENT_EXPAND"), KBSCOPE::Sequence },
    { _("SELECT_ALL"), KBSCOPE::Sequence },
    { _("SELECT_ALL_NO_TIMING"), KBSCOPE::Sequence },
    { _("SHOW_PRESETS"), KBSCOPE::Sequence },
    { _("SEARCH_TOGGLE"), KBSCOPE::Sequence },
    { _("PERSPECTIVES_TOGGLE"), KBSCOPE::Sequence },
    { _("EFFECT_UPDATE"), KBSCOPE::Sequence },
    { _("COLOR_UPDATE"), KBSCOPE::Sequence },
    { _("PLAY_LOOP"), KBSCOPE::All },
    { _("PLAY"), KBSCOPE::All },
    { _("TOGGLE_PLAY"), KBSCOPE::All },
    { _("START_OF_SONG"), KBSCOPE::All },
    { _("END_OF_SONG"), KBSCOPE::All },
    { _("STOP"), KBSCOPE::All },
    { _("PAUSE"), KBSCOPE::All },
    { _("EFFECT"), KBSCOPE::Sequence },
    { _("PRESET"), KBSCOPE::Sequence },
    { _("LOCK_MODEL"), KBSCOPE::Layout },
    { _("UNLOCK_MODEL"), KBSCOPE::Layout },
    { _("GROUP_MODELS"), KBSCOPE::Layout },
    { _("WIRING_VIEW"), KBSCOPE::Layout },
    { _("NODE_LAYOUT"), KBSCOPE::Layout },
    { _("SAVE_LAYOUT"), KBSCOPE::Layout },
    { _("SELECT_ALL_MODELS"), KBSCOPE::Layout },
    { _("MODEL_ALIGN_TOP"), KBSCOPE::Layout },
    { _("MODEL_ALIGN_BOTTOM"), KBSCOPE::Layout },
    { _("MODEL_ALIGN_LEFT"), KBSCOPE::Layout },
    { _("MODEL_ALIGN_RIGHT"), KBSCOPE::Layout },
    { _("MODEL_ALIGN_CENTER_VERT"), KBSCOPE::Layout },
    { _("MODEL_ALIGN_CENTER_HORIZ"), KBSCOPE::Layout },
    { _("MODEL_DISTRIBUTE_HORIZ"), KBSCOPE::Layout },
    { _("MODEL_DISTRIBUTE_VERT"), KBSCOPE::Layout },
    { _("CANCEL_RENDER"), KBSCOPE::Sequence },
    { _("TOGGLE_RENDER"), KBSCOPE::Sequence },
    { _("PRESETS_TOGGLE"), KBSCOPE::Sequence }
};

const std::vector<KeyBinding> DefaultBindings =
{
    KeyBinding(_("a"), false, _("SELECT_ALL_MODELS"), true),
    KeyBinding(_("a"), false, _("SELECT_ALL"), true, true),
    KeyBinding(_("a"), false, _("SELECT_ALL_NO_TIMING"), true),
    KeyBinding(_("F10"), false, _("BACKUP")),
    KeyBinding(_("F11"), false, _("ALTERNATE_BACKUP")),
    KeyBinding(_("F9"), false, _("SELECT_SHOW_FOLDER")),
    KeyBinding(_(""), true, _("LIGHTS_TOGGLE")),
    KeyBinding(_("o"), false, _("OPEN_SEQUENCE"), true),
    KeyBinding(_(""), true, _("CLOSE_SEQUENCE")),
    KeyBinding(_("n"), false, _("NEW_SEQUENCE"), true),
    KeyBinding(_(""), true, _("RENDER_ALL")),
    KeyBinding(_(""), true, _("PASTE_BY_CELL")),
    KeyBinding(_(""), true, _("PASTE_BY_TIME")),
    KeyBinding(_(""), true, _("SEQUENCE_SETTINGS")),
    KeyBinding(_(""), true, _("PLAY_LOOP")),
    KeyBinding(_("HOME"), false, _("START_OF_SONG")),
    KeyBinding(_("END"), false, _("END_OF_SONG")),
    KeyBinding(_(""), true, _("PLAY")),
    KeyBinding(_("SPACE"), false, _("TOGGLE_PLAY")),
    KeyBinding(_(""), true, _("STOP")),
    KeyBinding(_("PAUSE"), false, _("PAUSE")),

    KeyBinding(_("s"), false, _("SAVE_CURRENT_TAB"), true),
    KeyBinding(_(""), true, _("SAVE_SEQUENCE"), true),
    KeyBinding(_(""), true, _("SAVEAS_SEQUENCE"), true, false, true),
    KeyBinding(_("t"), false, _("TIMING_ADD")),
    KeyBinding(_("s"), false, _("TIMING_SPLIT")),
    KeyBinding(_("+"), false, _("ZOOM_IN")),
    KeyBinding(_("-"), false, _("ZOOM_OUT")),
    KeyBinding(_("R"), false, _("RANDOM"), false, false, true),
    KeyBinding(_("F1"), false, _("EFFECT_SETTINGS_TOGGLE"), true),
    KeyBinding(_("F8"), false, _("EFFECT_ASSIST_TOGGLE"), true),
    KeyBinding(_("F2"), false, _("COLOR_TOGGLE"), true),
    KeyBinding(_("F3"), false, _("LAYER_SETTING_TOGGLE"), true),
    KeyBinding(_("F4"), false, _("LAYER_BLENDING_TOGGLE"), true),
    KeyBinding(_("F5"), false, _("MODEL_PREVIEW_TOGGLE"), true),
    KeyBinding(_("F6"), false, _("HOUSE_PREVIEW_TOGGLE"), true),
    KeyBinding(_("F9"), false, _("EFFECTS_TOGGLE"), true),
    KeyBinding(_("F7"), false, _("DISPLAY_ELEMENTS_TOGGLE"), true),
    KeyBinding(_("F8"), false, _("JUKEBOX_TOGGLE"), true, true),
    KeyBinding(_("l"), false, _("LOCK_EFFECT"), true),
    KeyBinding(_("u"), false, _("UNLOCK_EFFECT"), true),
    KeyBinding(_("."), false, _("MARK_SPOT"), true),
    KeyBinding(_("/"), false, _("RETURN_TO_SPOT"), true),
    KeyBinding(_(""), true, _("EFFECT_DESCRIPTION"), true),
    KeyBinding(_(""), true, _("EFFECT_ALIGN_START"), true, true),
    KeyBinding(_(""), true, _("EFFECT_ALIGN_END"), true, true),
    KeyBinding(_(""), true, _("EFFECT_ALIGN_BOTH"), true, true),
    KeyBinding(_("I"), false, _("INSERT_LAYER_ABOVE"), true, false, true),
    KeyBinding(_("A"), false, _("INSERT_LAYER_BELOW"), true, false, true),
    KeyBinding(_("X"), false, _("TOGGLE_ELEMENT_EXPAND"), true, false, true),
    KeyBinding(_(""), false, _("SHOW_PRESETS")),
    KeyBinding(_("F10"), false, _("PRESETS_TOGGLE"), true),
    KeyBinding(_("F11"), false, _("SEARCH_TOGGLE"), true),
    KeyBinding(_("F12"), false, _("PERSPECTIVES_TOGGLE"), true),
    KeyBinding(_("F5"), false, _("EFFECT_UPDATE")),
    KeyBinding(_(""), false, _("COLOR_UPDATE")),
    KeyBinding(_("ESCAPE"), false, _("CANCEL_RENDER")),
    KeyBinding(_(""), false, _("TOGGLE_RENDER")),

    KeyBinding(_("l"), false, _("LOCK_MODEL"), true),
    KeyBinding(_("u"), false, _("UNLOCK_MODEL"), true),
    KeyBinding(_("g"), false, _("GROUP_MODELS"), true),
    KeyBinding(_(""), true, _("WIRING_VIEW"), true),
    KeyBinding(_(""), true, _("NODE_LAYOUT"), true),
    KeyBinding(_(""), true, _("SAVE_LAYOUT"), true, false, true),
    KeyBinding(_(""), true, _("MODEL_ALIGN_TOP"), true, false, true),
    KeyBinding(_(""), true, _("MODEL_ALIGN_BOTTOM"), true, false, true),
    KeyBinding(_(""), true, _("MODEL_ALIGN_LEFT"), true, false, true),
    KeyBinding(_(""), true, _("MODEL_ALIGN_RIGHT"), true, false, true),
    KeyBinding(_(""), true, _("MODEL_ALIGN_CENTER_VERT"), true, false, true),
    KeyBinding(_(""), true, _("MODEL_ALIGN_CENTER_HORIZ"), true, false, true),
    KeyBinding(_(""), true, _("MODEL_DISTRIBUTE_HORIZ"), true, false, true),
    KeyBinding(_(""), true, _("MODEL_DISTRIBUTE_VERT"), true, false, true),

    KeyBinding(_("o"), false, _("On"), _("E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=100"), xlights_version_string),
    KeyBinding(_("u"), false, _("On"), _("E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=0"), xlights_version_string),
    KeyBinding(_("d"), false, _("On"), _("E_TEXTCTRL_Eff_On_End=0,E_TEXTCTRL_Eff_On_Start=100"), xlights_version_string),
    KeyBinding(_("m"), false, _("Morph"), _(""), xlights_version_string),
    KeyBinding(_("c"), false, _("Curtain"), _(""), xlights_version_string),
    KeyBinding(_("i"), false, _("Circles"), _(""), xlights_version_string),
    KeyBinding(_("b"), false, _("Bars"), _(""), xlights_version_string),
    KeyBinding(_("y"), false, _("Butterfly"), _(""), xlights_version_string),
    KeyBinding(_("f"), false, _("Fire"), _(""), xlights_version_string),
    KeyBinding(_("g"), false, _("Garlands"), _(""), xlights_version_string),
    KeyBinding(_("p"), false, _("Pinwheel"), _(""), xlights_version_string),
    KeyBinding(_("r"), false, _("Ripple"), _(""), xlights_version_string),
    KeyBinding(_("x"), false, _("Text"), _(""), xlights_version_string),
    KeyBinding(_("S"), false, _("Spirals"), _(""), xlights_version_string, false, false, true),
    KeyBinding(_("w"), false, _("Color Wash"), _(""), xlights_version_string),
    KeyBinding(_("n"), false, _("Snowflakes"), _(""), xlights_version_string),
    KeyBinding(_("O"), false, _("Off"), _(""), xlights_version_string, false, false, true),
    KeyBinding(_("F"), false, _("Fan"), _(""), xlights_version_string, false, false, true)
};

// These are the keys that were hard coded before 2018.28 and thus need to be inserted if they are not present
const std::vector<std::pair<std::string, std::string>> ConvertKeys =
{
    { _("OPEN_SEQUENCE"), _("CTRL+O") },
    { _("NEW_SEQUENCE"), _("CTRL+N") },
    { _("PAUSE"), _("+PAUSE") },
    { _("START_OF_SONG"), _("+HOME") },
    { _("END_OF_SONG"), _("+END") },
    { _("SAVE_CURRENT_TAB"), _("CTRL+S") },
    { _("MARK_SPOT"), _("CTRL+.") },
    { _("RETURN_TO_SPOT"), _("CTRL+/") },
    { _("SELECT_ALL_MODELS"), _("CTRL+A") },
    { _("SELECT_ALL"), _("CTRL+SHIFT+A") },
    { _("SELECT_ALL_NO_TIMING"), _("CTRL+A") },
    { _("TOGGLE_PLAY"), _("+ ") },
    { _("BACKUP"), _("+F10") },
    { _("ALTERNATE_BACKUP"), _("+F11") },
    { _("SELECT_SHOW_FOLDER"), _("+F9") },
    { _("CANCEL_RENDER"), _("+ESCAPE") }
};

// These are the keys that we we consider equivalent
const std::vector<std::pair<wxKeyCode /*to*/, wxKeyCode/*from*/>> KeyEquivalents =
{
    { static_cast<wxKeyCode>('0'), WXK_NUMPAD0 },
    { static_cast<wxKeyCode>('1'), WXK_NUMPAD1 },
    { static_cast<wxKeyCode>('2'), WXK_NUMPAD2 },
    { static_cast<wxKeyCode>('3'), WXK_NUMPAD3 },
    { static_cast<wxKeyCode>('4'), WXK_NUMPAD4 },
    { static_cast<wxKeyCode>('5'), WXK_NUMPAD5 },
    { static_cast<wxKeyCode>('6'), WXK_NUMPAD6 },
    { static_cast<wxKeyCode>('7'), WXK_NUMPAD7 },
    { static_cast<wxKeyCode>('8'), WXK_NUMPAD8 },
    { static_cast<wxKeyCode>('9'), WXK_NUMPAD9 },
    { static_cast<wxKeyCode>('*'), WXK_MULTIPLY },
    { static_cast<wxKeyCode>('+'), WXK_ADD },
    { static_cast<wxKeyCode>('-'), WXK_SUBTRACT },
    { static_cast<wxKeyCode>('.'), WXK_DECIMAL },
    { static_cast<wxKeyCode>('/'), WXK_DIVIDE },
    { WXK_TAB, WXK_NUMPAD_TAB },
    { WXK_SPACE, WXK_NUMPAD_SPACE },
    { WXK_RETURN, WXK_NUMPAD_ENTER },
    { WXK_F1, WXK_NUMPAD_F1 },
    { WXK_F2, WXK_NUMPAD_F2 },
    { WXK_F3, WXK_NUMPAD_F3 },
    { WXK_F4, WXK_NUMPAD_F4 },
    { WXK_HOME, WXK_NUMPAD_HOME },
    { WXK_LEFT, WXK_NUMPAD_LEFT },
    { WXK_RIGHT, WXK_NUMPAD_RIGHT },
    { WXK_UP, WXK_NUMPAD_UP },
    { WXK_DOWN, WXK_NUMPAD_DOWN },
    { WXK_PAGEUP, WXK_NUMPAD_PAGEUP },
    { WXK_PAGEDOWN, WXK_NUMPAD_PAGEDOWN },
    { WXK_END, WXK_NUMPAD_END },
    { WXK_HOME, WXK_NUMPAD_BEGIN },
    { WXK_DELETE, WXK_NUMPAD_DELETE },
    { static_cast<wxKeyCode>('='), WXK_NUMPAD_EQUAL },
    { static_cast<wxKeyCode>('*'), WXK_NUMPAD_MULTIPLY },
    { static_cast<wxKeyCode>('+'), WXK_NUMPAD_ADD },
    { static_cast<wxKeyCode>('-'), WXK_NUMPAD_SUBTRACT },
    { static_cast<wxKeyCode>('.'), WXK_NUMPAD_DECIMAL },
    { static_cast<wxKeyCode>('/'), WXK_NUMPAD_DIVIDE }
};
#pragma endregion

#pragma region KeyBinding

#pragma region Constructors
KeyBinding::KeyBinding(wxKeyCode k, bool disabled, const std::string& type, bool control, bool alt, bool shift) :
    _type(type), _control(control), _alt(alt), _shift(shift),
    _disabled(disabled), _key(k)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxASSERT(KeyBindingTypes.size() > 0); // this can fail if someone reorders the constant creation so catch it
    auto it = std::find_if(begin(KeyBindingTypes), end(KeyBindingTypes), [type](const auto& kbt) { return kbt.first == type; });
    if (it == KeyBindingTypes.end())
    {
        // this should never happen
        wxASSERT(false);
        _disabled = true;
        _scope = KBSCOPE::Invalid;
        logger_base.error("Keybinding type '%s' not recognised", (const char *)type.c_str());
    }
    else
    {
        _scope = it->second;
    }
    _shift |= IsShiftedKey(_key);
}

KeyBinding::KeyBinding(const std::string& k, bool disabled, const std::string& type, bool control, bool alt, bool shift) :
    _type(type), _control(control), _alt(alt), _shift(shift), _disabled(disabled)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _key = DecodeKey(k);
    if (_key == WXK_NONE) _disabled = true;
    wxASSERT(KeyBindingTypes.size() > 0); // this can fail if someone reorders the constant creation so catch it
    auto it = std::find_if(begin(KeyBindingTypes), end(KeyBindingTypes), [type](const auto& kbt) { return kbt.first == type; });
    if (it == KeyBindingTypes.end())
    {
        // this should never happen
        wxASSERT(false);
        _disabled = true;
        _scope = KBSCOPE::Invalid;
        logger_base.error("Keybinding type '%s' not recognised", (const char *)type.c_str());
    }
    else
    {
        _scope = it->second;
    }
    _shift |= IsShiftedKey(_key);
}
#pragma endregion Constructors

#pragma region Static functions
bool KeyBinding::IsShiftedKey(wxKeyCode ch) noexcept
{
    static std::string shiftedChars = "~!@#$%^&*()_+{}|\":<>?";

    return (shiftedChars.find(static_cast<char>(ch)) != std::string::npos);
}

std::string KeyBinding::ParseKey(const std::string& key, bool& ctrl, bool& alt, bool& shift) noexcept
{
    alt = Contains(key, "ALT+");
    shift = Contains(key, "SHIFT+");
    ctrl = Contains(key, "CTRL+");
    // the key is whatever exists after the last '+'
    auto pos = key.find('+');
    if (pos != std::string::npos)
    {
        return key.substr(pos + 1);
    }
    return "";
}

std::string KeyBinding::EncodeKey(wxKeyCode key, bool shift) noexcept
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
        if (shift)
        {
            return std::string(1, ::toupper(key));
        }
        else
        {
            return std::string(1, ::tolower(key));
        }
    }
}

wxKeyCode KeyBinding::DecodeKey(std::string key) noexcept
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::transform(key.begin(), key.end(), key.begin(), ::toupper);
    if (key == "")
    {
        return WXK_NONE;
    }
    if (key == "ESC" || key == "ESCAPE")
    {
        return WXK_ESCAPE;
    }
    if (key == "DEL" || key == "DELETE")
    {
        return WXK_DELETE;
    }
    if (key == "BACK" || key == "BACKSPACE")
    {
        return WXK_BACK;
    }
    if (key == "TAB")
    {
        return WXK_TAB;
    }
    if (key == "SPACE")
    {
        return WXK_SPACE;
    }
    if (key == "DOWN")
    {
        return WXK_DOWN;
    }
    if (key == "UP")
    {
        return WXK_UP;
    }
    if (key == "LEFT")
    {
        return WXK_LEFT;
    }
    if (key == "RIGHT")
    {
        return WXK_RIGHT;
    }
    if (key == "HOME")
    {
        return WXK_HOME;
    }
    if (key == "END")
    {
        return WXK_END;
    }
    if (key == "INSERT")
    {
        return WXK_INSERT;
    }
    if (key == "PGUP" || key == "PAGEUP")
    {
        return WXK_PAGEUP;
    }
    if (key == "PGDN" || key == "PAGEDOWN")
    {
        return WXK_PAGEDOWN;
    }
    if (key == "PAUSE")
    {
        return WXK_PAUSE;
    }
    if (key == "RETURN" || key == "ENTER")
    {
        return WXK_RETURN;
    }
    if (key == "F1")
    {
        return WXK_F1;
    }
    if (key == "F2")
    {
        return WXK_F2;
    }
    if (key == "F3")
    {
        return WXK_F3;
    }
    if (key == "F4")
    {
        return WXK_F4;
    }
    if (key == "F5")
    {
        return WXK_F5;
    }
    if (key == "F6")
    {
        return WXK_F6;
    }
    if (key == "F7")
    {
        return WXK_F7;
    }
    if (key == "F8")
    {
        return WXK_F8;
    }
    if (key == "F9")
    {
        return WXK_F9;
    }
    if (key == "F10")
    {
        return WXK_F10;
    }
    if (key == "F11")
    {
        return WXK_F11;
    }
    if (key == "F12")
    {
        return WXK_F12;
    }

    if (key.size() != 1)
    {
        logger_base.error("KeyBinding decode key failed to decode '%s'. Taking the first character.", (const char *)key.c_str());
    }

    return static_cast<wxKeyCode>(static_cast<int8_t>(key[0]));
}
#pragma endregion Static functions

std::string KeyBinding::Description() const noexcept
{
    std::string res = "";

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
        std::string t = Capitalise(_type);
        std::replace(t.begin(), t.end(), '_', ' ');
        res += t;
    }
    return res;
}

bool KeyBinding::IsEquivalentKey(wxKeyCode key) const noexcept
{
    for (auto ke : KeyEquivalents)
    {
        // auto [to, from] = ke;
        auto to = ke.second;
        auto from = ke.first;

        if (to == key && _key == from)
        {
            return true;
        }
    }
    return false;
}

#pragma endregion KeyBinding

#pragma region KeyBindingMap
void KeyBindingMap::LoadDefaults() noexcept
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Loading default keybindings.");

    _bindings = DefaultBindings;
}

void KeyBindingMap::Load(const wxFileName &fileName) noexcept
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (fileName.Exists()) {
        logger_base.debug("Loading keybindings.");
        wxXmlDocument doc;
        if (doc.Load(fileName.GetFullPath())) {
            _bindings.clear();
            wxXmlNode *root = doc.GetRoot();
            wxXmlNode *child = root->GetChildren();

            while (child != nullptr) {
                if ("keybinding" == child->GetName()) {
                    std::string type = child->GetAttribute("type").ToStdString();
                    bool disabled = false;
                    std::string k = child->GetAttribute("keycode").ToStdString();
                    std::string oldk = child->GetAttribute("key").ToStdString();
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
                            _bindings.emplace_back(KeyBinding(k, disabled, effect, settings, child->GetAttribute("xLightsVersion", "4.0"), control, alt, shift));
                        }
                    }
                    else if (type == "PRESET")
                    {
                        std::string presetName = child->GetAttribute("effect").ToStdString();
                        _bindings.emplace_back(KeyBinding(disabled, k, presetName, control, alt, shift));
                    }
                    else
                    {
                        _bindings.emplace_back(KeyBinding(k, disabled, type, control, alt, shift));
                    }
                }

                child = child->GetNext();
            }

            // add in the missing essential key bindings if not present
            for (auto ck : ConvertKeys)
            {
                // const auto&[type, key] = ck;
                auto type = ck.first;
                auto key = ck.second;

                bool found = std::find_if(begin(_bindings), end(_bindings), [type](const KeyBinding& b) {return b.GetType() == type; }) != _bindings.end();
                if (!found)
                {
                    bool ctrl = false;
                    bool alt = false;
                    bool shift = false;
                    logger_base.debug("Adding essential keybinding %s.", (const char *)type.c_str());
                    std::string k = KeyBinding::ParseKey(key, ctrl, alt, shift);
                    _bindings.emplace_back(KeyBinding(k, false, type, ctrl, alt, shift));
                }
            }

            // add in all the missing key bindings ... but disable them
            for (auto kbt : KeyBindingTypes)
            {
                // const auto&[type, scope] = kbt;
                auto type = kbt.first;
                // auto scope = kbt.second;

                if (type != "EFFECT" && type != "PRESET")
                {
                    bool found = std::find_if(begin(_bindings), end(_bindings), [type](const KeyBinding& b) {return b.GetType() == type; }) != _bindings.end();
                    if (!found)
                    {
                        logger_base.debug("Adding missing keybinding %s.", (const char *)type.c_str());
                        _bindings.emplace_back(KeyBinding(WXK_NONE, true, type, false, false, false));
                    }
                }
            }
        }
        logger_base.debug("Keybindings loaded.");
    }

    std::string invalid = "";
    for (const auto& kb : _bindings)
    {
        if (kb.InScope(KBSCOPE::Invalid))
        {
            invalid = invalid + kb.GetType() + "\n";
        }
    }
    if (invalid != "")
    {
        DisplayError("Keybindings contains invalid bindings:\n\n" + invalid);
    }
}

void KeyBindingMap::Save(const wxFileName &fileName) const noexcept
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Saving keybindings.");

    wxXmlDocument doc;
    wxXmlNode *root = new wxXmlNode(wxXML_ELEMENT_NODE, "keybindings");
    doc.SetRoot(root);

    bool corrupt = false;
    for (const auto& binding : _bindings) {
        wxKeyCode key = binding.GetKey();
        if (binding.GetType() == "TIMING_ADD" && (key == WXK_NONE || KeyBinding::EncodeKey(key, binding.RequiresShift()) == ""))
        {
            logger_base.debug("TIMING_ADD: " + binding.Description());
            logger_base.warn("Your keybindings appear corrupt. Resetting key bindings.");
            corrupt = true;
            break;
        }
    }

    auto bindings = _bindings;
    if (corrupt)
    {
        wxASSERT(false);
        bindings = DefaultBindings;
    }

    for (const auto& binding : bindings) {

        wxKeyCode key = binding.GetKey();

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

    logger_base.debug("Keybindings saved.");
}

std::string KeyBindingMap::Dump() const noexcept
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string res;
    logger_base.debug("Dumping key map");
    logger_base.debug("Scope: Everywhere");
    for (const auto& b : _bindings) {
        if (b.InScope(KBSCOPE::All) && !b.IsDisabled())
        {
            auto s = b.Description();
            logger_base.debug("    %s", (const char*)s.c_str());
            res += s + "\n";
        }
    }
    logger_base.debug("Scope: Layout");
    res += "\n";
    for (const auto& b : _bindings) {
        if (b.InScope(KBSCOPE::Layout) && !b.IsDisabled())
        {
            auto s = b.Description();
            logger_base.debug("    %s", (const char*)s.c_str());
            res += s + "\n";
        }
    }
    logger_base.debug("Scope: Sequencer");
    res += "\n";
    for (const auto& b : _bindings) {
        if (b.InScope(KBSCOPE::Sequence) && !b.IsDisabled())
        {
            auto s = b.Description();
            logger_base.debug("    %s", (const char*)s.c_str());
            res += s + "\n";
        }
    }
    return res;
}

std::shared_ptr<const KeyBinding> KeyBindingMap::Find(const wxKeyEvent& event, KBSCOPE scope) const noexcept
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool ctrl = event.CmdDown() || event.ControlDown();
    wxKeyCode key = static_cast<wxKeyCode>(event.GetKeyCode());
    bool alt = event.AltDown();
    bool shift = event.ShiftDown();

    for (const auto& b : _bindings) {
        if (!b.IsDisabled() &&
            b.RequiresAlt() == alt &&
            b.RequiresControl() == ctrl &&
            (
            (b.RequiresShift() == shift && b.IsKey(key)) ||
                (b.IsEquivalentKey(key))
                ) &&
            b.InScope(scope)) {
            // Once we get through a couple of releases and i know i am not getting crashes as a result of these i can comment this out
            logger_base.debug("Keybinding fired: %s %s", (const char *)b.GetType().c_str(), (const char *)b.GetEffectName().c_str());
            return std::make_shared<const KeyBinding>(b);
        }
    }
    return nullptr;
}
#pragma endregion KeyBindingMap