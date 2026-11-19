/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LayoutRenameLog.h"

void LayoutRenameLog::Record(const std::string& oldName, const std::string& newName, const std::string& id) {
    if (oldName.empty() || newName.empty() || oldName == newName) {
        return;
    }
    std::lock_guard<std::mutex> lock(_lock);
    _entries.emplace_back(oldName, newName);
    _unpersisted.push_back({ oldName, newName, id, std::time(nullptr) });
}

void LayoutRenameLog::Clear() {
    std::lock_guard<std::mutex> lock(_lock);
    _entries.clear();
}

void LayoutRenameLog::Reset() {
    std::lock_guard<std::mutex> lock(_lock);
    _entries.clear();
    _unpersisted.clear();
}

std::vector<LayoutRenameLog::TimedEntry> LayoutRenameLog::TakeUnpersisted() {
    std::lock_guard<std::mutex> lock(_lock);
    std::vector<TimedEntry> out;
    out.swap(_unpersisted);
    return out;
}

bool LayoutRenameLog::Empty() const {
    std::lock_guard<std::mutex> lock(_lock);
    return _entries.empty();
}

std::vector<LayoutRenameLog::Entry> LayoutRenameLog::Entries() const {
    std::lock_guard<std::mutex> lock(_lock);
    return _entries;
}

std::string LayoutRenameLog::CurrentName(const std::string& name) const {
    std::lock_guard<std::mutex> lock(_lock);
    std::string current = name;
    for (const auto& [from, to] : _entries) {
        if (from == current) {
            current = to;
        }
    }
    return current;
}
