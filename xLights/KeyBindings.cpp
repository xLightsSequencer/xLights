
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>

#include "KeyBindings.h"
#include "xLightsVersion.h"
#include "UtilFunctions.h"
#include "ExternalHooks.h"

#include <log4cpp/Category.hh>

#pragma region Constants

int KeyBinding::__nextid = 0;

static  std::vector<std::pair<std::string, KBSCOPE>> KeyBindingTypes =
{
    { "TIMING_ADD", KBSCOPE::Sequence },
    { "TIMING_SPLIT", KBSCOPE::Sequence },
    { "ZOOM_IN", KBSCOPE::Sequence },
    { "ZOOM_OUT", KBSCOPE::Sequence },
    { "ZOOM_SEL", KBSCOPE::Sequence },
    { "RANDOM", KBSCOPE::Sequence },
    { "RENDER_ALL", KBSCOPE::All },
    { "SAVE_CURRENT_TAB", KBSCOPE::All },
    { "LIGHTS_TOGGLE", KBSCOPE::All },
    { "OPEN_SEQUENCE", KBSCOPE::All },
    { "CLOSE_SEQUENCE", KBSCOPE::All },
    { "NEW_SEQUENCE", KBSCOPE::All },
    { "PASTE_BY_CELL", KBSCOPE::All },
    { "PASTE_BY_TIME", KBSCOPE::All },
    { "BACKUP", KBSCOPE::All },
    { "ALTERNATE_BACKUP", KBSCOPE::All },
    { "SELECT_SHOW_FOLDER", KBSCOPE::All },
    { "SAVEAS_SEQUENCE", KBSCOPE::Sequence },
    { "SAVE_SEQUENCE", KBSCOPE::Sequence },
    { "EFFECT_SETTINGS_TOGGLE", KBSCOPE::Sequence },
    { "EFFECT_ASSIST_TOGGLE", KBSCOPE::Sequence },
    { "COLOR_TOGGLE", KBSCOPE::Sequence },
    { "LAYER_SETTING_TOGGLE", KBSCOPE::Sequence },
    { "LAYER_BLENDING_TOGGLE", KBSCOPE::Sequence },
    { "MODEL_PREVIEW_TOGGLE", KBSCOPE::Sequence },
    { "HOUSE_PREVIEW_TOGGLE", KBSCOPE::Sequence },
    { "EFFECTS_TOGGLE", KBSCOPE::Sequence },
    { "DISPLAY_ELEMENTS_TOGGLE", KBSCOPE::Sequence },
    { "JUKEBOX_TOGGLE", KBSCOPE::Sequence },
    { "SEQUENCE_SETTINGS", KBSCOPE::All },
    { "LOCK_EFFECT", KBSCOPE::Sequence },
    { "UNLOCK_EFFECT", KBSCOPE::Sequence },
    { "MARK_SPOT", KBSCOPE::Sequence },
    { "RETURN_TO_SPOT", KBSCOPE::Sequence },
    { "EFFECT_DESCRIPTION", KBSCOPE::Sequence },
    { "EFFECT_ALIGN_START", KBSCOPE::Sequence },
    { "EFFECT_ALIGN_END", KBSCOPE::Sequence },
    { "EFFECT_ALIGN_BOTH", KBSCOPE::Sequence },
    { "INSERT_LAYER_ABOVE", KBSCOPE::Sequence },
    { "INSERT_LAYER_BELOW", KBSCOPE::Sequence },
    { "TOGGLE_ELEMENT_EXPAND", KBSCOPE::Sequence },
    { "SELECT_ALL", KBSCOPE::Sequence },
    { "SELECT_ALL_NO_TIMING", KBSCOPE::Sequence },
    { "SHOW_PRESETS", KBSCOPE::Sequence },
    { "SEARCH_TOGGLE", KBSCOPE::Sequence },
    { "PERSPECTIVES_TOGGLE", KBSCOPE::Sequence },
    { "EFFECT_UPDATE", KBSCOPE::Sequence },
    { "COLOR_UPDATE", KBSCOPE::Sequence },
    { "PLAY_LOOP", KBSCOPE::All },
    { "PLAY", KBSCOPE::All },
    { "TOGGLE_PLAY", KBSCOPE::All },
    { "START_OF_SONG", KBSCOPE::All },
    { "END_OF_SONG", KBSCOPE::All },
    { "STOP", KBSCOPE::All },
    { "PAUSE", KBSCOPE::All },
    { "EFFECT", KBSCOPE::Sequence },
    { "APPLYSETTING", KBSCOPE::Sequence },
    { "PRESET", KBSCOPE::Sequence },
    { "LOCK_MODEL", KBSCOPE::Layout },
    { "UNLOCK_MODEL", KBSCOPE::Layout },
    { "GROUP_MODELS", KBSCOPE::Layout },
    { "WIRING_VIEW", KBSCOPE::Layout },
    { "EXPORT_MODEL_CAD", KBSCOPE::Layout },
    { "EXPORT_LAYOUT_DXF", KBSCOPE::Layout },
    { "NODE_LAYOUT", KBSCOPE::Layout },
    { "SAVE_LAYOUT", KBSCOPE::Layout },
    { "SELECT_ALL_MODELS", KBSCOPE::Layout },
    { "MODEL_ALIGN_TOP", KBSCOPE::Layout },
    { "MODEL_ALIGN_BOTTOM", KBSCOPE::Layout },
    { "MODEL_ALIGN_LEFT", KBSCOPE::Layout },
    { "MODEL_ALIGN_RIGHT", KBSCOPE::Layout },
    { "MODEL_ALIGN_CENTER_VERT", KBSCOPE::Layout },
    { "MODEL_ALIGN_CENTER_HORIZ", KBSCOPE::Layout },
    { "MODEL_ALIGN_BACKS", KBSCOPE::Layout },
    { "MODEL_ALIGN_FRONTS", KBSCOPE::Layout },
    { "MODEL_ALIGN_GROUND", KBSCOPE::Layout },
    { "MODEL_DISTRIBUTE_HORIZ", KBSCOPE::Layout },
    { "MODEL_DISTRIBUTE_VERT", KBSCOPE::Layout },
    { "MODEL_FLIP_HORIZ", KBSCOPE::Layout },
    { "MODEL_FLIP_VERT", KBSCOPE::Layout },
    { "MODEL_SUBMODELS", KBSCOPE::Layout },
    { "MODEL_FACES", KBSCOPE::Layout },
    { "MODEL_STATES", KBSCOPE::Layout },
    { "MODEL_MODELDATA", KBSCOPE::Layout },
    { "CANCEL_RENDER", KBSCOPE::Sequence },
    { "TOGGLE_RENDER", KBSCOPE::Sequence },
    { "PRESETS_TOGGLE", KBSCOPE::Sequence },
    { "FOCUS_SEQUENCER", KBSCOPE::All }, // This forces focus to the sequencer for situations where keys dont seem to work. It must be mapped to function key
    { "VALUECURVES_TOGGLE", KBSCOPE::Sequence },
    { "COLOR_DROPPER_TOGGLE", KBSCOPE::Sequence },
    { "AUDIO_FULL_SPEED", KBSCOPE::Sequence },
    { "AUDIO_F_1_5_SPEED",  KBSCOPE::Sequence },
    { "AUDIO_F_2_SPEED", KBSCOPE::Sequence },
    { "AUDIO_F_3_SPEED", KBSCOPE::Sequence },
    { "AUDIO_F_4_SPEED", KBSCOPE::Sequence },
    { "AUDIO_S_3_4_SPEED", KBSCOPE::Sequence },
    { "AUDIO_S_1_2_SPEED", KBSCOPE::Sequence },
    { "AUDIO_S_1_4_SPEED", KBSCOPE::Sequence },
    { "PRIOR_TAG", KBSCOPE::Sequence },
    { "NEXT_TAG", KBSCOPE::Sequence },
    { "MODEL_TOGGLE", KBSCOPE::Sequence },
    { "MODEL_DISABLE", KBSCOPE::Sequence },
    { "MODEL_ENABLE", KBSCOPE::Sequence },
    { "EFFECT_TOGGLE", KBSCOPE::Sequence },
    { "EFFECT_DISABLE", KBSCOPE::Sequence },
    { "EFFECT_ENABLE", KBSCOPE::Sequence },
    { "MODEL_EFFECT_TOGGLE", KBSCOPE::Sequence }

};

static  std::vector<std::pair<std::string, std::string>> keyBindingTips =
{
    { "TIMING_ADD", "Add a timing mark." },
    { "TIMING_SPLIT", "Split a timing mark." },
    { "ZOOM_IN", "Zoom into the effects grid." },
    { "ZOOM_OUT", "Zoom out of the effects grid." },
    { "ZOOM_SEL", "Zoom so selected timeline fills the screen." },
    { "RANDOM", "Insert random effects." },
    { "RENDER_ALL", "Render all." },
    { "SAVE_CURRENT_TAB", "Save the currently selected tab." },
    { "LIGHTS_TOGGLE", "Toggle output to lights on/off." },
    { "OPEN_SEQUENCE", "Open a sequence." },
    { "CLOSE_SEQUENCE", "Close the open sequence." },
    { "NEW_SEQUENCE", "Create a new sequence." },
    { "PASTE_BY_CELL", "Put cut/copy/paste into Paste By Cell mode." },
    { "PASTE_BY_TIME", "Put cut/copy/paste into Paste By Time mode." },
    { "BACKUP", "Backup your show folder." },
    { "ALTERNATE_BACKUP", "Backup your show folder to the alternate backup location." },
    { "SELECT_SHOW_FOLDER", "Change your current show folder." },
    { "SAVEAS_SEQUENCE", "Save the current sequence to a new file." },
    { "SAVE_SEQUENCE", "Save the current sequence." },
    { "EFFECT_SETTINGS_TOGGLE", "Toggle display of the effect settings panel." },
    { "EFFECT_ASSIST_TOGGLE", "Toggle display of the effect assist panel." },
    { "COLOR_TOGGLE", "Toggle display of the color panel." },
    { "LAYER_SETTING_TOGGLE", "Toggle display of the layer settings panel." },
    { "LAYER_BLENDING_TOGGLE", "Toggle display of the layer blending panel." },
    { "MODEL_PREVIEW_TOGGLE", "Toggle display of the model preview panel." },
    { "HOUSE_PREVIEW_TOGGLE", "Toggle display of the house preview panel." },
    { "EFFECTS_TOGGLE", "Toggle display of the effect dropper panel." },
    { "DISPLAY_ELEMENTS_TOGGLE", "Toggle display of the display elements panel." },
    { "JUKEBOX_TOGGLE", "Toggle display of the jukebox panel." },
    { "SEQUENCE_SETTINGS", "Display the sequence settings." },
    { "LOCK_EFFECT", "Lock the selected effects." },
    { "UNLOCK_EFFECT", "Unlock the selected effects." },
    { "MARK_SPOT", "Mark the current spot in the sequencer so you can return to it." },
    { "RETURN_TO_SPOT", "Return to the previously marked spot in the sequencer." },
    { "EFFECT_DESCRIPTION", "Open the effect description dialog." },
    { "EFFECT_ALIGN_START", "Align the selected effects to have the same start times." },
    { "EFFECT_ALIGN_END", "Align the selected effects to have the same end times." },
    { "EFFECT_ALIGN_BOTH", "Align the selected effects to have the same start and end times." },
    { "INSERT_LAYER_ABOVE", "Insert a sequencing layer above the current row." },
    { "INSERT_LAYER_BELOW", "Insert a sequencing layer below the current row." },
    { "TOGGLE_ELEMENT_EXPAND", "Expand the current element." },
    { "SELECT_ALL", "Select all effects and timing marks." },
    { "SELECT_ALL_NO_TIMING", "Select all effects but not timing marks." },
    { "SHOW_PRESETS", "Show the effect presets panel." },
    { "SEARCH_TOGGLE", "Toggle display of the effect search panel." },
    { "PERSPECTIVES_TOGGLE", "Toggle display of the perspectives panel." },
    { "EFFECT_UPDATE", "Apply the current effect settings to all selected effects." },
    { "COLOR_UPDATE", "Apply the current colors to all selected effects." },
    { "PLAY_LOOP", "Play the selected part of the song repeatedly." },
    { "PLAY", "Play the song." },
    { "TOGGLE_PLAY", "Play/Stop playing the song." },
    { "START_OF_SONG", "Jump to the start of the song." },
    { "END_OF_SONG", "Jump to the end of the song." },
    { "STOP", "Stop sequence playback." },
    { "PAUSE", "Pause sequence playback." },
    { "EFFECT", "Insert an effect." },
    { "APPLYSETTING", "Apply setting to selected effects." },
    { "PRESET", "Insert a preset effect." },
    { "LOCK_MODEL", "Lock the selected models." },
    { "UNLOCK_MODEL", "Unlock the selected models." },
    { "GROUP_MODELS", "Create a group from the selected models." },
    { "WIRING_VIEW", "Display the wiring view for the selected model." },
    { "EXPORT_MODEL_CAD", "Export the selected model as a DXF or STL or VRML File." },
    { "EXPORT_LAYOUT_DXF", "Export the default layout as a DXF File." },
    { "NODE_LAYOUT", "Display the node layout for the selected model." },
    { "SAVE_LAYOUT", "Save the layout tab." },
    { "SELECT_ALL_MODELS", "Select all models." },
    { "MODEL_ALIGN_TOP", "Align the selected models to the top edge." },
    { "MODEL_ALIGN_BOTTOM", "Align the selected models to the bottom edge." },
    { "MODEL_ALIGN_LEFT", "Align the selected models to the left edge." },
    { "MODEL_ALIGN_RIGHT", "Align the selected models to the right edge." },
    { "MODEL_ALIGN_CENTER_VERT", "Align the selected models to be vertically centered." },
    { "MODEL_ALIGN_CENTER_HORIZ", "Align the selected models to be horizontally centered." },
    { "MODEL_ALIGN_FRONTS", "Align the selected models to the front edge." },
    { "MODEL_ALIGN_BACKS", "Align the selected models to the back edge." },
    { "MODEL_ALIGN_GROUND", "Align the selected models to the ground." },
    { "MODEL_DISTRIBUTE_HORIZ", "Distribute the selected model horizontally." },
    { "MODEL_DISTRIBUTE_VERT", "Distribute the selected models vertically." },
    { "MODEL_FLIP_HORIZ", "Flip the selected models horizontally." },
    { "MODEL_FLIP_VERT", "Flip the selected models vertically." },
    { "CANCEL_RENDER", "Cancel current rendering activity." },
    { "TOGGLE_RENDER", "Toggle background rendering." },
    { "PRESETS_TOGGLE", "Toggle display of the presets panel." },
    { "FOCUS_SEQUENCER", "Force keyboard focus to the effects gid." }, // This forces focus to the sequencer for situations where keys dont seem to work. It must be mapped to function key
    { "VALUECURVES_TOGGLE", "Toggle display of the value curves droppper panel." },
    { "COLOR_DROPPER_TOGGLE", "Toggle display of the color dropper panel." },
    { "AUDIO_FULL_SPEED", "Playback audio at normal speed." },
    { "AUDIO_F_1_5_SPEED", "Playback audio at 1.5 times speed." },
    { "AUDIO_F_2_SPEED", "Playback audio at 2 times speed." },
    { "AUDIO_F_3_SPEED", "Playback audio at 3 times speed." },
    { "AUDIO_F_4_SPEED", "Playback audio at 4 times speed." },
    { "AUDIO_S_3_4_SPEED", "Playback audio at 3/4 speed." },
    { "AUDIO_S_1_2_SPEED", "Playback audio at 1/2 speed." },
    { "AUDIO_S_1_4_SPEED", "Playback audio at 1/4 speed." },
    { "PRIOR_TAG", "Jump to prior audio tag." },
    { "NEXT_TAG", "Jump to next audio tag." },
    { "MODEL_SUBMODELS", "Edit model submodels." },
    { "MODEL_FACES", "Edit model faces." },
    { "MODEL_STATES", "Edit model states." },
    { "MODEL_MODELDATA", "Edit custom model data." },
    { "MODEL_TOGGLE", "Toggle (Enable/Disable) rendering of the selected model in the sequencer" },
    { "MODEL_DISABLE", "Disable rendering of the selected model in the sequencer" },
    { "MODEL_ENABLE", "Enable rendering of the selected model in the sequencer" },
    { "EFFECT_TOGGLE", "Toggle (Enable/Disable) rendering of the selected effects in the sequencer" },
    { "EFFECT_DISABLE", "Disable rendering of the selected effects in the sequencer" },
    { "EFFECT_ENABLE", "Enable rendering of the selected effects in the sequencer" },
    { "MODEL_EFFECT_TOGGLE", "Toggle (Enable/Disable) rendering of the selected model or the effects in the sequencer" }
};

const std::vector<KeyBinding> DefaultBindings =
{
    KeyBinding("a", false, "SELECT_ALL_MODELS", true),
    KeyBinding("a", false, "SELECT_ALL", true, true),
    KeyBinding("a", false, "SELECT_ALL_NO_TIMING", true),
    KeyBinding("F10", false, "BACKUP"),
    KeyBinding("F11", false, "ALTERNATE_BACKUP"),
    KeyBinding("F9", false, "SELECT_SHOW_FOLDER"),
    KeyBinding("", true, "LIGHTS_TOGGLE"),
    KeyBinding("o", false, "OPEN_SEQUENCE", true),
    KeyBinding("w", false, "CLOSE_SEQUENCE", true),
    KeyBinding("n", false, "NEW_SEQUENCE", true),
    KeyBinding("", true, "RENDER_ALL"),
    KeyBinding("", true, "PASTE_BY_CELL"),
    KeyBinding("", true, "PASTE_BY_TIME"),
    KeyBinding("", true, "SEQUENCE_SETTINGS"),
    KeyBinding("", true, "PLAY_LOOP"),
    KeyBinding("HOME", false, "START_OF_SONG"),
    KeyBinding("END", false, "END_OF_SONG"),
    KeyBinding("", true, "PLAY"),
    KeyBinding("SPACE", false, "TOGGLE_PLAY"),
    KeyBinding("", true, "STOP"),
    KeyBinding("PAUSE", false, "PAUSE"),
    KeyBinding("", true, "AUDIO_FULL_SPEED", true, true),
    KeyBinding("", true, "AUDIO_F_1_5_SPEED", true, true),
    KeyBinding("", true, "AUDIO_F_2_SPEED", true, true),
    KeyBinding("", true, "AUDIO_F_3_SPEED", true, true),
    KeyBinding("", true, "AUDIO_F_4_SPEED", true, true),
    KeyBinding("", true, "AUDIO_S_3_4_SPEED", true, true),
    KeyBinding("", true, "AUDIO_S_1_2_SPEED", true, true),
    KeyBinding("", true, "AUDIO_S_1_4_SPEED", true, true),
    KeyBinding("", true, "PRIOR_TAG", true, true),
    KeyBinding("", true, "NEXT_TAG", true, true),

    KeyBinding("s", false, "SAVE_CURRENT_TAB", true),
    KeyBinding("", true, "SAVE_SEQUENCE", true),
    KeyBinding("", true, "SAVEAS_SEQUENCE", true, false, true),
    KeyBinding("t", false, "TIMING_ADD"),
    KeyBinding("s", false, "TIMING_SPLIT"),
    KeyBinding("+", false, "ZOOM_IN"),
    KeyBinding("-", false, "ZOOM_OUT"),
    KeyBinding("", false, "ZOOM_SEL"),
    KeyBinding(std::string("R"), false, "RANDOM", false, false, true),
    KeyBinding("F1", false, "EFFECT_SETTINGS_TOGGLE", true),
    KeyBinding("F8", false, "EFFECT_ASSIST_TOGGLE", true),
    KeyBinding("F2", false, "COLOR_TOGGLE", true),
    KeyBinding("F3", false, "LAYER_SETTING_TOGGLE", true),
    KeyBinding("F4", false, "LAYER_BLENDING_TOGGLE", true),
    KeyBinding("F5", false, "MODEL_PREVIEW_TOGGLE", true),
    KeyBinding("F6", false, "HOUSE_PREVIEW_TOGGLE", true),
    KeyBinding("F9", false, "EFFECTS_TOGGLE", true),
    KeyBinding("F7", false, "DISPLAY_ELEMENTS_TOGGLE", true),
    KeyBinding("F8", false, "JUKEBOX_TOGGLE", true, true),
    KeyBinding("l", false, "LOCK_EFFECT", true),
    KeyBinding("u", false, "UNLOCK_EFFECT", true),
    KeyBinding(".", false, "MARK_SPOT", true),
    KeyBinding("/", false, "RETURN_TO_SPOT", true),
    KeyBinding(std::string("."), false, "MARK_SPOT", false, false, false, true),
    KeyBinding(std::string("/"), false, "RETURN_TO_SPOT", false, false, false, true),
    KeyBinding("", true, "EFFECT_DESCRIPTION", true),
    KeyBinding("", true, "EFFECT_ALIGN_START", true, true),
    KeyBinding("", true, "EFFECT_ALIGN_END", true, true),
    KeyBinding("", true, "EFFECT_ALIGN_BOTH", true, true),
    KeyBinding(std::string("I"), false, "INSERT_LAYER_ABOVE", true, false, true),
    KeyBinding(std::string("A"), false, "INSERT_LAYER_BELOW", true, false, true),
    KeyBinding(std::string("X"), false, "TOGGLE_ELEMENT_EXPAND", true, false, true),
    KeyBinding("", false, "SHOW_PRESETS"),
    KeyBinding("F10", false, "PRESETS_TOGGLE", true),
    KeyBinding("F12", false, "VALUECURVES_TOGGLE", false, true),
    KeyBinding("F11", false, "COLOR_DROPPER_TOGGLE", false, true),
    KeyBinding("F12", false, "FOCUS_SEQUENCER"),
    KeyBinding("F11", false, "SEARCH_TOGGLE", true),
    KeyBinding("F12", false, "PERSPECTIVES_TOGGLE", true),
    KeyBinding("F5", false, "EFFECT_UPDATE"),
    KeyBinding("", false, "COLOR_UPDATE"),
    KeyBinding("ESCAPE", false, "CANCEL_RENDER"),
    KeyBinding("", false, "TOGGLE_RENDER"),

    KeyBinding("l", false, "LOCK_MODEL", true),
    KeyBinding("u", false, "UNLOCK_MODEL", true),
    KeyBinding("g", false, "GROUP_MODELS", true),
    KeyBinding("", true, "WIRING_VIEW", true),
    KeyBinding("", true, "EXPORT_MODEL_CAD", true),
    KeyBinding("", true, "EXPORT_LAYOUT_DXF", true),
    KeyBinding("", true, "NODE_LAYOUT", true),
    KeyBinding("", true, "SAVE_LAYOUT", true, false, true),
    KeyBinding("", true, "MODEL_ALIGN_TOP", true, false, true),
    KeyBinding("", true, "MODEL_ALIGN_BOTTOM", true, false, true),
    KeyBinding("", true, "MODEL_ALIGN_LEFT", true, false, true),
    KeyBinding("", true, "MODEL_ALIGN_RIGHT", true, false, true),
    KeyBinding("", true, "MODEL_ALIGN_CENTER_VERT", true, false, true),
    KeyBinding("", true, "MODEL_ALIGN_CENTER_HORIZ", true, false, true),
    KeyBinding("", true, "MODEL_ALIGN_BACKS", true, false, true),
    KeyBinding("", true, "MODEL_ALIGN_FRONTS", true, false, true),
    KeyBinding("", true, "MODEL_ALIGN_GROUND", true, false, true),
    KeyBinding("", true, "MODEL_DISTRIBUTE_HORIZ", true, false, true),
    KeyBinding("", true, "MODEL_DISTRIBUTE_VERT", true, false, true),
    KeyBinding("", true, "MODEL_FLIP_HORIZ", true, false, true),
    KeyBinding("", true, "MODEL_FLIP_VERT", true, false, true),
    KeyBinding("", true, "MODEL_SUBMODELS", true),
    KeyBinding("", true, "MODEL_FACES", true),
    KeyBinding("", true, "MODEL_STATES", true),
    KeyBinding("", true, "MODEL_MODELDATA", true),

    KeyBinding("o", false, "On", "E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=100", xlights_version_string),
    KeyBinding("u", false, "On", "E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=0", xlights_version_string),
    KeyBinding("d", false, "On", "E_TEXTCTRL_Eff_On_End=0,E_TEXTCTRL_Eff_On_Start=100", xlights_version_string),
    KeyBinding("m", false, "Morph", "", xlights_version_string),
    KeyBinding("c", false, "Curtain", "", xlights_version_string),
    KeyBinding("i", false, "Circles", "", xlights_version_string),
    KeyBinding("b", false, "Bars", "", xlights_version_string),
    KeyBinding("y", false, "Butterfly", "", xlights_version_string),
    KeyBinding("f", false, "Fire", "", xlights_version_string),
    KeyBinding("g", false, "Garlands", "", xlights_version_string),
    KeyBinding("p", false, "Pinwheel", "", xlights_version_string),
    KeyBinding("r", false, "Ripple", "", xlights_version_string),
    KeyBinding("x", false, "Text", "", xlights_version_string),
    KeyBinding("S", false, "Spirals", "", xlights_version_string, false, false, true),
    KeyBinding("w", false, "Color Wash", "", xlights_version_string),
    KeyBinding("n", false, "Snowflakes", "", xlights_version_string),
    KeyBinding("O", false, "Off", "", xlights_version_string, false, false, true),
    KeyBinding("F", false, "Fan", "", xlights_version_string, false, false, true),
    KeyBinding(false, std::string("U"), "T_TEXTCTRL_Fadein=1.00", xlights_version_string, false, false, true, false),
    KeyBinding(false, std::string("D"), "T_TEXTCTRL_Fadeout=1.00", xlights_version_string, false, false, true, false)
};

// These are the keys that were hard coded before 2018.28 and thus need to be inserted if they are not present
const std::vector<std::pair<std::string, std::string>> ConvertKeys =
{
    { "OPEN_SEQUENCE", "CTRL+o" },
    { "NEW_SEQUENCE", "CTRL+n" },
    { "PAUSE", "PAUSE" },
    { "START_OF_SONG", "HOME" },
    { "END_OF_SONG", "END" },
    { "SAVE_CURRENT_TAB", "CTRL+s" },
    { "MARK_SPOT", "CTRL+." },
    { "RETURN_TO_SPOT", "CTRL+/" },
    { "MARK_SPOT", "RCTRL+." },
    { "RETURN_TO_SPOT", "RCTRL+/" },
    { "TOGGLE_PLAY", "SPACE" },
    { "BACKUP", "F10" },
    { "ALTERNATE_BACKUP", "F11" },
    { "SELECT_SHOW_FOLDER", "F9" },
    { "CANCEL_RENDER", "ESCAPE" },
    { "FOCUS_SEQUENCER", "F12" }
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
KeyBinding::KeyBinding(wxKeyCode k, bool disabled, const std::string& type, bool control, bool alt, bool shift, bool rcontrol) :
    _type(type), _control(control), _rcontrol(rcontrol), _alt(alt), _shift(shift),
    _disabled(disabled), _key(k)
{
    _id = __nextid++;

    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxASSERT(KeyBindingTypes.size() > 0); // this can fail if someone reorders the constant creation so catch it
    auto it = std::find_if(begin(KeyBindingTypes), end(KeyBindingTypes), [type](const auto& kbt) { return kbt.first == type; });
    if (it == KeyBindingTypes.end()) {
        // this should never happen
        wxASSERT(false);
        _disabled = true;
        _scope = KBSCOPE::Invalid;
        logger_base.error("Keybinding type '%s' not recognised", (const char *)type.c_str());
    } else {
        _scope = it->second;
    }
    auto it2 = std::find_if(begin(keyBindingTips), end(keyBindingTips), [type](const auto& kbt) { return kbt.first == type; });
    if (it2 != keyBindingTips.end()) {
        _tip = it2->second;
    }

    _shift |= IsShiftedKey(_key);
}

KeyBinding::KeyBinding(const std::string& k, bool disabled, const std::string& type, bool control, bool alt, bool shift, bool rcontrol) :
    _type(type), _control(control), _rcontrol(rcontrol), _alt(alt), _shift(shift), _disabled(disabled)
{
    _id = __nextid++;

    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _key = DecodeKey(k);
    if (_key == WXK_NONE) _disabled = true;
    wxASSERT(KeyBindingTypes.size() > 0); // this can fail if someone reorders the constant creation so catch it
    auto it = std::find_if(begin(KeyBindingTypes), end(KeyBindingTypes), [type](const auto& kbt) { return kbt.first == type; });
    if (it == KeyBindingTypes.end()) {
        // this should never happen
        wxASSERT(false);
        _disabled = true;
        _scope = KBSCOPE::Invalid;
        logger_base.error("Keybinding type '%s' not recognised", (const char *)type.c_str());
    } else {
        _scope = it->second;
    }
    auto it2 = std::find_if(begin(keyBindingTips), end(keyBindingTips), [type](const auto& kbt) { return kbt.first == type; });
    if (it2 != keyBindingTips.end()) {
        _tip = it2->second;
    }
    _shift |= IsShiftedKey(_key);
}
#pragma endregion Constructors

#pragma region Static functions
bool KeyBinding::IsShiftedKey(wxKeyCode ch) noexcept
{
    if (ch > 127) return false;

    static std::string shiftedChars = "~!@#$%^&*()_+{}|\":<>?";

    return (shiftedChars.find(static_cast<char>(ch)) != std::string::npos);
}

std::string KeyBinding::ParseKey(const std::string& k, bool& ctrl, bool& alt, bool& shift, bool &rctrl) noexcept
{
    std::string key = k;
    auto pos = key.find('+');
    while (pos != std::string::npos) {
        std::string prefix = key.substr(0, pos);
        if (prefix == "ALT") {
            alt = true;
        }
        if (prefix == "SHIFT") {
            shift = true;
        }
        if (prefix == "CTRL") {
            ctrl = true;
        }
        if (prefix == "RCTRL") {
            rctrl = true;
        }
        key = key.substr(pos + 1);
        pos = key.find('+');
    }
    return key;
}

std::vector<wxKeyCode>& KeyBinding::GetPossibleKeys()
{
    static bool init = false;
    static std::vector<wxKeyCode> keys =
    {
        WXK_ESCAPE,
        WXK_HOME,
        WXK_END,
        WXK_INSERT,
        WXK_SPACE,
        WXK_DELETE,
        WXK_BACK,
        WXK_TAB,
        WXK_DOWN,
        WXK_UP,
        WXK_LEFT,
        WXK_RIGHT,
        WXK_PAGEUP,
        WXK_PAGEDOWN,
        WXK_PAUSE,
        WXK_RETURN,
        WXK_F1,
        WXK_F2,
        WXK_F3,
        WXK_F4,
        WXK_F5,
        WXK_F6,
        WXK_F7,
        WXK_F8,
        WXK_F9,
        WXK_F10,
        WXK_F11,
        WXK_F12,
        WXK_F13,
        WXK_F14,
        WXK_F15,
        WXK_F16,
        WXK_F17,
        WXK_F18,
        WXK_F19,
        WXK_F20,
        WXK_F21,
        WXK_F22,
        WXK_F23,
        WXK_F24
    };

    if (!init) {
        init = true;
        for (int i = 33; i < 65; i++) {
            keys.push_back(wxKeyCode(i));
        }
        for (int i = 91; i < 127; i++) {
            keys.push_back(wxKeyCode(i));
        }
    }

    return keys;
}

std::string KeyBinding::EncodeKey(wxKeyCode key, bool shift) noexcept
{
    switch (key) {
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
    case WXK_F13:
        return "F13";
    case WXK_F14:
        return "F14";
    case WXK_F15:
        return "F15";
    case WXK_F16:
        return "F16";
    case WXK_F17:
        return "F17";
    case WXK_F18:
        return "F18";
    case WXK_F19:
        return "F19";
    case WXK_F20:
        return "F20";
    case WXK_F21:
        return "F21";
    case WXK_F22:
        return "F22";
    case WXK_F23:
        return "F23";
    case WXK_F24:
        return "F24";
    case WXK_NONE:
        return "";
    default:
        wxASSERT(key > 32 && key < 128);
        if (shift) {
            return std::string(1, ::toupper(key));
        } else {
            return std::string(1, ::tolower(key));
        }
    }
}

wxKeyCode KeyBinding::DecodeKey(std::string key) noexcept
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::transform(key.begin(), key.end(), key.begin(), ::toupper);
    if (key == "") {
        return WXK_NONE;
    }
    if (key == "ESC" || key == "ESCAPE") {
        return WXK_ESCAPE;
    }
    if (key == "DEL" || key == "DELETE") {
        return WXK_DELETE;
    }
    if (key == "BACK" || key == "BACKSPACE") {
        return WXK_BACK;
    }
    if (key == "TAB") {
        return WXK_TAB;
    }
    if (key == "SPACE") {
        return WXK_SPACE;
    }
    if (key == "DOWN") {
        return WXK_DOWN;
    }
    if (key == "UP") {
        return WXK_UP;
    }
    if (key == "LEFT") {
        return WXK_LEFT;
    }
    if (key == "RIGHT") {
        return WXK_RIGHT;
    }
    if (key == "HOME") {
        return WXK_HOME;
    }
    if (key == "END") {
        return WXK_END;
    }
    if (key == "INSERT") {
        return WXK_INSERT;
    }
    if (key == "PGUP" || key == "PAGEUP") {
        return WXK_PAGEUP;
    }
    if (key == "PGDN" || key == "PAGEDOWN") {
        return WXK_PAGEDOWN;
    }
    if (key == "PAUSE") {
        return WXK_PAUSE;
    }
    if (key == "RETURN" || key == "ENTER") {
        return WXK_RETURN;
    }
    if (key == "F1") {
        return WXK_F1;
    }
    if (key == "F2") {
        return WXK_F2;
    }
    if (key == "F3") {
        return WXK_F3;
    }
    if (key == "F4") {
        return WXK_F4;
    }
    if (key == "F5") {
        return WXK_F5;
    }
    if (key == "F6") {
        return WXK_F6;
    }
    if (key == "F7") {
        return WXK_F7;
    }
    if (key == "F8") {
        return WXK_F8;
    }
    if (key == "F9") {
        return WXK_F9;
    }
    if (key == "F10") {
        return WXK_F10;
    }
    if (key == "F11") {
        return WXK_F11;
    }
    if (key == "F12") {
        return WXK_F12;
    }
    if (key == "F13") {
        return WXK_F13;
    }
    if (key == "F14") {
        return WXK_F14;
    }
    if (key == "F15") {
        return WXK_F15;
    }
    if (key == "F16") {
        return WXK_F16;
    }
    if (key == "F17") {
        return WXK_F17;
    }
    if (key == "F18") {
        return WXK_F18;
    }
    if (key == "F19") {
        return WXK_F19;
    }
    if (key == "F20") {
        return WXK_F20;
    }
    if (key == "F21") {
        return WXK_F21;
    }
    if (key == "F22") {
        return WXK_F22;
    }
    if (key == "F23") {
        return WXK_F23;
    }
    if (key == "F24") {
        return WXK_F24;
    }

    if (key.size() != 1) {
        logger_base.error("KeyBinding decode key failed to decode '%s'. Taking the first character.", (const char *)key.c_str());
    }

    return static_cast<wxKeyCode>(static_cast<int8_t>(key[0]));
}

bool KeyBinding::IsControlEqual(KeyBinding const& key, const bool& ctrl, const bool& rctrl)
{
#if !defined(__WXOSX__)
    if (ctrl && key.RequiresControl() == ctrl) {
        return true;
    }
    if (rctrl && key.RequiresRawControl() == rctrl) {
        return true;
    }
#endif
    return key.RequiresControl() == ctrl && key.RequiresRawControl() == rctrl;
}
#pragma endregion Static functions

std::string KeyBinding::KeyDescription() const noexcept
{
    std::string res = "";

    if (_disabled) {
        res += "N/A";
    } else {
        if (_control) {
            res += "CTRL+";
        }
        if (_rcontrol) {
            res += "RCTRL+";
        }
        if (_alt) {
            res += "ALT+";
        }
        bool s = _shift;
        if (s && IsShiftedKey(_key)) s = false;
        if (s) {
            res += "SHIFT+";
        }
        res += EncodeKey(_key, _shift);
    }
    return res;
}
std::string KeyBinding::Description() const noexcept
{
    std::string res = "";

    if (_disabled) {
        res += "N/A";
    } else {
        if (_control) {
            res += "CTRL+";
        }
        if (_rcontrol) {
            res += "RCTRL+";
        }
        if (_alt) {
            res += "ALT+";
        }
        bool s = _shift;
        if (s && IsShiftedKey(_key)) s = false;
        if (s) {
            res += "SHIFT+";
        }
        res += "'";
        res += EncodeKey(_key, _shift);
        res += "'";
    }

    while (res.size() < 22) {
        res += " ";
    }

    res += ": ";

    if (_type == "EFFECT" || _type == "PRESET") {
        res += _type + " " + _effectName;
    } else {
        std::string t = Capitalise(_type);
        std::replace(t.begin(), t.end(), '_', ' ');
        res += t;
    }
    return res;
}

bool KeyBinding::IsEquivalentKey(wxKeyCode key) const noexcept
{
    for (auto ke : KeyEquivalents) {
        // auto [to, from] = ke;
        auto to = ke.second;
        auto from = ke.first;

        if (to == key && _key == from) {
            return true;
        }
    }
    return false;
}

void KeyBinding::SetKey(const std::string& key)
{
    if (key == "") {
        _key = WXK_NONE;
        _disabled = true;
    } else {
        _key = DecodeKey(key);
    }
    _disabled = (_key == WXK_NONE);
}

void KeyBinding::SetKey(const wxKeyCode key)
{
    _key = key;
    if (_key >= 97 && _key <= 122) _key = (wxKeyCode)(_key - 32);
    if (_key == WXK_NONE) _disabled = true; else _disabled = false; }

bool KeyBinding::IsDuplicateKey(const KeyBinding& b) const
{
    if (_id == b.GetId()) return false;

    if (b.GetScope() == GetScope() || GetScope() == KBSCOPE::All || b.GetScope() == KBSCOPE::All) {
        if (b.GetKey() == GetKey() && b.RequiresAlt() == RequiresAlt() && KeyBinding::IsControlEqual(b, RequiresRawControl(), RequiresControl()) && b.RequiresShift() == RequiresShift()) {
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

    _openedFile = fileName; // even if the file does not exist I assume this is where we want to save it

    if (FileExists(fileName)) {
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
                    bool rcontrol = child->GetAttribute("rawControl", "FALSE") == "TRUE";
                    if (oldk != "") {
                        k = oldk;
                        if (k >= 'A' && k <= 'Z') shift = true; else shift = false;
                    }
                    if (k == "") disabled = true;
                    if (type == "EFFECT") {
                        std::string effect = child->GetAttribute("effect").ToStdString();
                        std::string settings = "";
                        if (child->GetChildren() != nullptr) {
                            settings = child->GetChildren()->GetContent().ToStdString();
                        }
                        if (effect != "")
                        {
                            _bindings.emplace_back(KeyBinding(k, disabled, effect, settings, child->GetAttribute("xLightsVersion", "4.0"), control, alt, shift, rcontrol));
                        }
                    } else if (type == "PRESET") {
                        std::string presetName = child->GetAttribute("effect").ToStdString();
                        _bindings.emplace_back(KeyBinding(disabled, k, presetName, control, alt, shift, rcontrol));
                    } else if (type == "APPLYSETTING") {
						std::string settings = "";
						if (child->GetChildren() != nullptr) {
							settings = child->GetChildren()->GetContent().ToStdString();
						}
						if (settings != "")	{
							_bindings.emplace_back(KeyBinding(disabled, k, settings, child->GetAttribute("xLightsVersion", "4.0"), control, alt, shift, rcontrol));
						}
                    } else {
                        _bindings.emplace_back(KeyBinding(k, disabled, type, control, alt, shift, rcontrol));
                    }
                }

                child = child->GetNext();
            }

            // add in the missing essential key bindings if not present
            for (auto ck : ConvertKeys) {
                // const auto&[type, key] = ck;
                auto type = ck.first;
                auto key = ck.second;

                bool found = std::find_if(begin(_bindings), end(_bindings), [type, key](const KeyBinding& b) {return b.GetType() == type && b.KeyDescription() == key; }) != _bindings.end();
                if (!found) {
                    bool ctrl = false;
                    bool rctrl = false;
                    bool alt = false;
                    bool shift = false;
                    logger_base.debug("Adding essential keybinding %s.", (const char *)type.c_str());
                    std::string k = KeyBinding::ParseKey(key, ctrl, alt, shift, rctrl);
                    _bindings.emplace_back(KeyBinding(k, false, type, ctrl, alt, shift, rctrl));
                }
            }

            // add in all the missing key bindings ... but disable them
            for (auto kbt : KeyBindingTypes) {
                // const auto&[type, scope] = kbt;
                auto type = kbt.first;
                // auto scope = kbt.second;

                if (type != "EFFECT" && type != "PRESET" && type != "APPLYSETTING") {
                    bool found = std::find_if(begin(_bindings), end(_bindings), [type](const KeyBinding& b) {return b.GetType() == type; }) != _bindings.end();
                    if (!found) {
                        logger_base.debug("Adding missing keybinding %s.", (const char *)type.c_str());
                        _bindings.emplace_back(KeyBinding(WXK_NONE, true, type, false, false, false, false));
                    }
                }
            }
        }
        logger_base.debug("Keybindings loaded.");
    } else {
        logger_base.debug("Keybinding file not found, Creating Default File.");
        Save();
    }

    std::string invalid = "";
    for (const auto& kb : _bindings) {
        if (kb.InScope(KBSCOPE::Invalid)) {
            invalid = invalid + kb.GetType() + "\n";
        }
    }
    if (invalid != "") {
        DisplayError("Keybindings contains invalid bindings:\n\n" + invalid);
    }
}

void KeyBindingMap::Save() const noexcept
{
    Save(_openedFile);
}

int KeyBindingMap::AddKey(const KeyBinding& kb)
{
    _bindings.emplace_back(kb);
    return kb.GetId();
}

void KeyBindingMap::DeleteKey(int id)
{
    for (auto it = _bindings.begin(); it != _bindings.end(); ++it) {
        if (it->GetId() == id) {
            _bindings.erase(it);
            break;
        }
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
        if (binding.GetType() == "TIMING_ADD" && (key == WXK_NONE || KeyBinding::EncodeKey(key, binding.RequiresShift()) == "")) {
            logger_base.debug("TIMING_ADD: " + binding.Description());
            logger_base.warn("Your keybindings appear corrupt. Resetting key bindings.");
            corrupt = true;
            break;
        }
    }

    auto bindings = _bindings;
    if (corrupt) {
        wxASSERT(false);
        bindings = DefaultBindings;
    }

    for (const auto& binding : bindings) {

        wxKeyCode key = binding.GetKey();

        wxXmlNode *child = new wxXmlNode(wxXML_ELEMENT_NODE, "keybinding");

        if (binding.IsDisabled()) {
            child->AddAttribute("keycode", "");
        } else {
            child->AddAttribute("keycode", KeyBinding::EncodeKey(key, binding.RequiresShift()));
        }
        child->AddAttribute("alt", binding.RequiresAlt() ? "TRUE" : "FALSE");
        child->AddAttribute("control", binding.RequiresControl() ? "TRUE" : "FALSE");
        if (binding.RequiresRawControl()) {
            child->AddAttribute("rawControl", "TRUE");
        }
        child->AddAttribute("shift", binding.RequiresShift() ? "TRUE" : "FALSE");
        child->AddAttribute("type", binding.GetType());
        if (binding.GetType() == "EFFECT") {
            child->AddAttribute("effect", binding.GetEffectName());
            child->AddAttribute("xLightsVersion", binding.GetEffectDataVersion());
            child->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", binding.GetEffectString()));
        } else if (binding.GetType() == "PRESET") {
            child->AddAttribute("effect", binding.GetEffectName());
        } else if (binding.GetType() == "APPLYSETTING") {
            child->AddAttribute("xLightsVersion", binding.GetEffectDataVersion());
            child->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", binding.GetEffectString()));
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
        if (b.InScope(KBSCOPE::All) && !b.IsDisabled()) {
            auto s = b.Description();
            logger_base.debug("    %s", (const char*)s.c_str());
            res += s + "\n";
        }
    }
    logger_base.debug("Scope: Layout");
    res += "\n";
    for (const auto& b : _bindings) {
        if (b.InScope(KBSCOPE::Layout) && !b.IsDisabled()) {
            auto s = b.Description();
            logger_base.debug("    %s", (const char*)s.c_str());
            res += s + "\n";
        }
    }
    logger_base.debug("Scope: Sequencer");
    res += "\n";
    for (const auto& b : _bindings) {
        if (b.InScope(KBSCOPE::Sequence) && !b.IsDisabled()) {
            auto s = b.Description();
            logger_base.debug("    %s", (const char*)s.c_str());
            res += s + "\n";
        }
    }
    return res;
}

KeyBinding& KeyBindingMap::GetBinding(int id)
{
    for (auto& b : _bindings) {
        if (b.GetId() == id) return b;
    }

    // this is bad
    wxASSERT(false);
    return *(new KeyBinding("", true, ""));
}

bool KeyBindingMap::IsDuplicateKey(const KeyBinding& b) const
{
    for (const auto& it : _bindings) {
        if (it.IsDuplicateKey(b)) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<const KeyBinding> KeyBindingMap::Find(const wxKeyEvent& event, KBSCOPE scope) const noexcept
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));


    wxKeyCode key = static_cast<wxKeyCode>(event.GetKeyCode());
    bool alt = event.AltDown();
    bool shift = event.ShiftDown();
    bool rctrl = event.RawControlDown();
    bool ctrl = event.CmdDown() || event.ControlDown();

    for (const auto& b : _bindings) {
        if (!b.IsDisabled() &&
            b.RequiresAlt() == alt &&
            KeyBinding::IsControlEqual(b, ctrl, rctrl) &&
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
