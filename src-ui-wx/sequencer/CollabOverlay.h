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

// What the sequencer draws for a live collaboration session: other people's
// working areas, focused rows, selections and soft locks, this user's edits
// still waiting for the host, and remote edits to flash. Built by
// DesktopCollab; read by TimeLine, RowHeading and EffectsGrid while painting.
// Null when no session is running.

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "render/SequenceCommands.h"
#include "utils/Color.h"

struct CollabOverlay {
    struct Band {
        int startMS = 0;
        int endMS = 0;
        xlColor colour;
    };
    struct Lock {
        std::string initials;
        xlColor colour;
    };
    struct Ghost {
        EffectLocator effect; // layer to draw on, effect it stands for
        int startMS = 0;
        int endMS = 0;
        bool deleting = false;
    };

    std::vector<Band> bands;                                         // time ruler
    std::unordered_map<SeqObjectId, std::vector<xlColor>> rowStripes; // row element uid
    std::unordered_map<SeqObjectId, std::vector<xlColor>> outlines;   // effect uid
    std::unordered_map<SeqObjectId, Lock> locks;                      // effect uid
    std::vector<Ghost> ghosts;
    xlColor ghostColour;
    std::unordered_map<SeqObjectId, int64_t> flashUntil; // effect uid -> wxGetLocalTimeMillis
    int64_t now = 0;

    static CollabOverlay*& Current() {
        static CollabOverlay* current = nullptr;
        return current;
    }
};
