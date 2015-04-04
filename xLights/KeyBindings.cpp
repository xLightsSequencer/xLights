//
//  KeyBindings.cpp
//  xLights

#include "KeyBindings.h"


void KeyBindingMap::LoadDefaults() {
    bindings.push_back(KeyBinding('t', TIMING_ADD));
    bindings.push_back(KeyBinding('s', TIMING_SPLIT));
    bindings.push_back(KeyBinding('+', KEY_ZOOM_IN));
    bindings.push_back(KeyBinding('-', KEY_ZOOM_OUT));
    bindings.push_back(KeyBinding('o', "On", "E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=100"));
    bindings.push_back(KeyBinding('u', "On", "E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=0"));
    bindings.push_back(KeyBinding('d', "On", "E_TEXTCTRL_Eff_On_End=0,E_TEXTCTRL_Eff_On_Start=100"));
}

void KeyBindingMap::Load(wxString &file) {
    
}

void KeyBindingMap::Save(wxString &file) {
    
}

KeyBinding *KeyBindingMap::Find(unsigned char ch) {
    for (int x = 0; x < bindings.size(); x++) {
        if (bindings[x].GetKey() == ch) {
            return &bindings[x];
        }
    }
    return NULL;
}

