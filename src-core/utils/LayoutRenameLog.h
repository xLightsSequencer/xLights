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

#include <mutex>
#include <string>
#include <utility>
#include <vector>

// The renames made in this process since the layout was last loaded or saved,
// in the order they happened. Each manager owns the log for the names it
// holds: ModelManager for models and groups, ViewObjectManager for view
// objects, OutputManager for controllers.
//
// Merging a show file uses the log to match a record whose name changed here
// when one of the two versions being compared has no layout id (a file written
// by a version that drops ids). The log describes this process's version
// only; whoever records the file's new base clears it.
class LayoutRenameLog {
public:
    using Entry = std::pair<std::string, std::string>; // old name, new name

    void Record(const std::string& oldName, const std::string& newName);
    void Clear();

    bool Empty() const;
    std::vector<Entry> Entries() const;

    // The name a record called `name` before every logged rename has now:
    // follows chains (A->B, B->C gives C) and returns `name` itself when it
    // was never renamed. A record created later with a name that had already
    // been renamed away is not confused with the original.
    std::string CurrentName(const std::string& name) const;

private:
    mutable std::mutex _lock;
    std::vector<Entry> _entries;
};
