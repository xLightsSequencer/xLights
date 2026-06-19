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

#include "ai/ServiceProperty.h"

#include <functional>
#include <string>
#include <vector>

class wxPropertyGrid;
class wxSizer;
class wxWindow;
class wxPanel;
class wxPropertyGridEvent;

namespace PropertyGridBuilder {

// Append each ServiceProperty to `page` as the appropriate wxPG property row.
// Ids are used verbatim as the property name so the host can route
// wxEVT_PG_CHANGED events back to the right service.
void Append(wxPropertyGrid* page, const std::vector<ServiceProperty>& props);

// Build a standalone wxPanel containing a wxPropertyGrid populated with the
// given properties. Used by AIImageDialog to embed a per-generator settings
// block inline (in place of the old AIImageGenerator::addControls hook).
// When a property changes, `onChanged` is invoked with the event so the
// caller can forward to SetProperty on the right object.
wxPanel* MakePanel(wxWindow* parent,
                   const std::vector<ServiceProperty>& props,
                   std::function<void(const wxPropertyGridEvent&)> onChanged);

// Dispatch a wxPropertyGrid change event to a SetProperty-style callable.
// `setBool`/`setInt`/`setString` each take (id, value).
void Dispatch(const wxPropertyGridEvent& event,
              const std::function<void(const std::string&, bool)>& setBool,
              const std::function<void(const std::string&, int)>& setInt,
              const std::function<void(const std::string&, const std::string&)>& setString);

} // namespace PropertyGridBuilder
