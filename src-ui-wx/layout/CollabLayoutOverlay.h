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

// What the layout editor draws for a live collaboration session: the layout
// objects other people have selected (in their colour) and the ones someone
// holds in a modal editor. Built by DesktopCollab; read by ModelPreview and
// LayoutPanel. Null when no session is running.

#include <string>
#include <unordered_map>

#include "models/BaseObject.h"
#include "models/Model.h"
#include "utils/Color.h"

struct CollabLayoutOverlay {
    struct Lock {
        std::string text; // "Sam is editing Arch 1's submodels"
        xlColor colour;
        bool operator==(const Lock&) const = default;
    };
    // Layout object key (CollabLayoutOverlay::KeyOf) -> the colour of another
    // participant who has it selected.
    std::unordered_map<std::string, xlColor> selected;
    std::unordered_map<std::string, Lock> locks;

    bool operator==(const CollabLayoutOverlay&) const = default;

    // The id the session uses for a layout object: its layout id, else its
    // name (a submodel's full "Model/Sub" name).
    static std::string KeyOf(const BaseObject* o) {
        if (o == nullptr) {
            return {};
        }
        if (!o->GetObjectId().empty()) {
            return o->GetObjectId();
        }
        if (auto* m = dynamic_cast<const Model*>(o)) {
            return m->GetFullName();
        }
        return o->GetName();
    }
    const xlColor* SelectedColour(const BaseObject* o) const {
        if (selected.empty()) {
            return nullptr;
        }
        auto it = selected.find(KeyOf(o));
        return it == selected.end() ? nullptr : &it->second;
    }
    const Lock* LockOf(const BaseObject* o) const {
        if (locks.empty()) {
            return nullptr;
        }
        auto it = locks.find(KeyOf(o));
        return it == locks.end() ? nullptr : &it->second;
    }

    static CollabLayoutOverlay*& Current() {
        static CollabLayoutOverlay* current = nullptr;
        return current;
    }
};
