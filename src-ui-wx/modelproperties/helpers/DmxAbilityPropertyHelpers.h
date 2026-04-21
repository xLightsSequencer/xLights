#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class DmxShutterAbility;
class DmxDimmerAbility;
class DmxBeamAbility;
class DmxPresetAbility;
class DmxColorAbility;
class DmxColorAbilityRGB;
class DmxColorAbilityCMY;
class DmxColorAbilityWheel;

namespace DmxAbilityPropertyHelpers {

// Polymorphic color dispatch — calls the appropriate typed function based on DmxColorAbility subclass
void AddColorProperties(wxPropertyGridInterface* grid, DmxColorAbility* color, bool pwm);
int OnColorPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxColorAbility* color, BaseObject* base);

void AddShutterProperties(wxPropertyGridInterface* grid, const DmxShutterAbility& shutter);
int OnShutterPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxShutterAbility& shutter, BaseObject* base);

void AddDimmerProperties(wxPropertyGridInterface* grid, const DmxDimmerAbility& dimmer);
int OnDimmerPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxDimmerAbility& dimmer, BaseObject* base);

void AddBeamProperties(wxPropertyGridInterface* grid, const DmxBeamAbility& beam);
int OnBeamPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxBeamAbility& beam, BaseObject* base);

void AddPresetProperties(wxPropertyGridInterface* grid, DmxPresetAbility& preset, int num_channels);
int OnPresetPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxPresetAbility& preset, int num_channels, BaseObject* base);

void AddColorRGBProperties(wxPropertyGridInterface* grid, const DmxColorAbilityRGB& color, bool pwm);
int OnColorRGBPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxColorAbilityRGB& color, BaseObject* base);

void AddColorCMYProperties(wxPropertyGridInterface* grid, const DmxColorAbilityCMY& color, bool pwm);
int OnColorCMYPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxColorAbilityCMY& color, BaseObject* base);

void AddColorWheelProperties(wxPropertyGridInterface* grid, const DmxColorAbilityWheel& color, bool pwm);
int OnColorWheelPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxColorAbilityWheel& color, BaseObject* base);

} // namespace DmxAbilityPropertyHelpers
