/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <string>
#include <vector>

// What each display can actually present, beyond the single number
// wxVideoMode::refresh carries.
//
// A sequence asking for more frames per second than the display can present
// cannot preview at its own rate - and on macOS the playback timer is driven by
// CADisplayLink, so it is hard limited by exactly this. Two things matter that
// wx does not report: the highest rate the panel offers at its current
// resolution (a 144Hz panel left set to 60 is a fixable user problem, a 60Hz
// panel is not), and whether the rate is variable.
//
// The caller passes what it already knows about each display so each platform
// can key on its strongest identifier rather than trusting enumeration order to
// line up: the device name on Windows, geometry on X11. macOS needs neither -
// it walks the same CGGetActiveDisplayList wxDisplay does.
struct xlDisplayQuery {
    std::string name;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

// One entry per input display, in the same order. An entry is empty when
// nothing could be determined, which leaves the caller on wxDisplay's number.
std::vector<std::string> GetDisplayRefreshInfo(const std::vector<xlDisplayQuery>& displays);

// Adapter-level, Windows only: whether DXGI allows presenting outside vblank,
// which is the flag a variable-refresh (G-Sync / FreeSync) present path gates
// on. Windows exposes no per-display VRR query, so this is the closest public
// signal there is - and it says nothing about the panel itself. Empty where it
// does not apply.
std::string GetPresentCapabilityDescription();
