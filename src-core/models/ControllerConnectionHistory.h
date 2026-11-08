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

#include <map>
#include <string>
#include <vector>

#include "ControllerConnection.h"

class Model;
class ModelManager;

// Undo / cancel for the controller visualiser: snapshots every model's controller
// connection and model chain. Restore calls return the names of the models they
// changed; the caller recalculates start channels and marks the layout dirty.
class ControllerConnectionHistory
{
public:
    explicit ControllerConnectionHistory(ModelManager& mm) : _mm(mm) {}

    // Takes the state Revert() returns to and clears the undo steps.
    void Begin();
    // Makes the current state the undo baseline without adding a step.
    void SkipStep();
    // Adds an undo step if anything changed since the last call.
    bool Record();
    [[nodiscard]] bool CanUndo() const { return !_undo.empty(); }
    // True when the state differs from the undo baseline (so undoing every edit means no changes).
    [[nodiscard]] bool HasChanges() const;
    std::vector<std::string> Undo();
    std::vector<std::string> Revert();

private:
    struct State {
        const Model* model;
        ControllerConnection conn;
        std::string chain;
        std::string key;
    };
    using Snapshot = std::map<std::string, State>;

    static std::string Key(const Model* m);
    [[nodiscard]] Snapshot Capture() const;
    static bool Same(const Snapshot& a, const Snapshot& b);
    std::vector<std::string> Restore(const Snapshot& snap);

    ModelManager& _mm;
    Snapshot _original;
    Snapshot _baseline;
    Snapshot _last;
    std::vector<Snapshot> _undo;
};
