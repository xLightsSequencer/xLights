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

#include <string>
#include <utility>
#include <vector>

class XLightsConfigAdapter;

// Persists an ordered (name, visible) list as a single JSON-array config
// string, e.g. [{"name":"Bars","visible":true}, ...]. Not toolbar-specific -
// callers pass their own config key and candidate name list, so this can be
// reused for a second customizable toolbar later without a redesign.

// Reads the saved layout for `key`, then reconciles it against
// `currentNames`: entries no longer present are dropped (e.g. a removed
// effect), and any name in `currentNames` not already accounted for is
// appended as visible, so new items don't silently disappear. Returns an
// all-visible layout in `currentNames` order if nothing was saved yet.
std::vector<std::pair<std::string, bool>> LoadToolbarLayout(XLightsConfigAdapter* cfg, const std::string& key,
                                                              const std::vector<std::string>& currentNames);

void SaveToolbarLayout(XLightsConfigAdapter* cfg, const std::string& key,
                        const std::vector<std::pair<std::string, bool>>& layout);
