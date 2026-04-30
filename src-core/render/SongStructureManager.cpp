/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SongStructureManager.h"

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstring>

// 8-color cycling palette at semi-transparent alpha (0x40 = 64/255 ~ 25%)
// blue, green, orange, purple, teal, red, slate, amber
static const uint32_t s_palette[SongStructureManager::PALETTE_SIZE] = {
    0x402196F3, // blue
    0x404CAF50, // green
    0x40FF9800, // orange
    0x409C27B0, // purple
    0x40009688, // teal
    0x40F44336, // red
    0x40607D8B, // slate
    0x40FFC107  // amber
};

std::vector<SongStructureRegion> SongStructureManager::s_emptyRegions;
int SongStructureManager::s_dummyNextId = 1;
const std::string SongStructureManager::s_emptyName;

SongStructureManager::SongStructureManager()
{
}

SongStructureManager::~SongStructureManager()
{
}

// --- Private helpers ---

std::vector<SongStructureRegion>& SongStructureManager::ActiveRegions()
{
    if (_activeViewIndex >= 0 && _activeViewIndex < (int)_views.size()) {
        return _views[_activeViewIndex].regions;
    }
    s_emptyRegions.clear();
    return s_emptyRegions;
}

const std::vector<SongStructureRegion>& SongStructureManager::ActiveRegions() const
{
    if (_activeViewIndex >= 0 && _activeViewIndex < (int)_views.size()) {
        return _views[_activeViewIndex].regions;
    }
    return s_emptyRegions;
}

int& SongStructureManager::ActiveNextId()
{
    if (_activeViewIndex >= 0 && _activeViewIndex < (int)_views.size()) {
        return _views[_activeViewIndex].nextId;
    }
    s_dummyNextId = 1;
    return s_dummyNextId;
}

// --- Region access (active view) ---

size_t SongStructureManager::GetRegionCount() const { return ActiveRegions().size(); }
const SongStructureRegion& SongStructureManager::GetRegion(size_t index) const { return ActiveRegions()[index]; }
SongStructureRegion& SongStructureManager::GetRegion(size_t index) { return ActiveRegions()[index]; }
const std::vector<SongStructureRegion>& SongStructureManager::GetRegions() const { return ActiveRegions(); }
bool SongStructureManager::HasRegions() const { return !ActiveRegions().empty(); }

int SongStructureManager::GetRegionIndexAtTime(int timeMS) const
{
    const auto& regions = ActiveRegions();
    for (size_t i = 0; i < regions.size(); i++) {
        if (timeMS >= regions[i].startTimeMS && timeMS < regions[i].endTimeMS) {
            return (int)i;
        }
    }
    return -1;
}

const SongStructureRegion* SongStructureManager::GetRegionAtTime(int timeMS) const
{
    int idx = GetRegionIndexAtTime(timeMS);
    if (idx >= 0) {
        return &ActiveRegions()[idx];
    }
    return nullptr;
}

void SongStructureManager::AddBoundary(int timeMS, int sequenceEndMS)
{
    // Auto-create default view if none exist
    if (_views.empty()) {
        _views.push_back(SongStructureView("Default"));
        _activeViewIndex = 0;
    }

    auto& regions = ActiveRegions();
    auto& nextId = ActiveNextId();

    if (regions.empty()) {
        // Reject boundaries that would create a zero/negative-length region
        if (timeMS <= 0 || timeMS >= sequenceEndMS) {
            return;
        }
        SongStructureRegion r1(nextId++, 0, timeMS, "Region 1", GetPaletteColor(0));
        SongStructureRegion r2(nextId++, timeMS, sequenceEndMS, "Region 2", GetPaletteColor(1));
        regions.push_back(r1);
        regions.push_back(r2);
    } else {
        int idx = GetRegionIndexAtTime(timeMS);
        if (idx < 0) {
            return;
        }
        SongStructureRegion& existing = regions[idx];

        if (timeMS <= existing.startTimeMS || timeMS >= existing.endTimeMS) {
            return;
        }

        SongStructureRegion newRegion(nextId++, timeMS, existing.endTimeMS,
            "Region " + std::to_string(nextId - 1),
            GetPaletteColor((regions.size()) % PALETTE_SIZE));
        existing.endTimeMS = timeMS;
        regions.insert(regions.begin() + idx + 1, newRegion);
    }
}

void SongStructureManager::DeleteBoundary(int timeMS)
{
    auto& regions = ActiveRegions();
    if (regions.size() < 2) {
        return;
    }

    for (size_t i = 1; i < regions.size(); i++) {
        if (regions[i].startTimeMS == timeMS) {
            regions[i - 1].endTimeMS = regions[i].endTimeMS;
            regions.erase(regions.begin() + i);

            if (regions.size() <= 1) {
                regions.clear();
            }
            return;
        }
    }
}

void SongStructureManager::MoveBoundary(int oldTimeMS, int newTimeMS)
{
    auto& regions = ActiveRegions();
    if (regions.size() < 2) {
        return;
    }

    for (size_t i = 1; i < regions.size(); i++) {
        if (regions[i].startTimeMS == oldTimeMS) {
            int minTime = regions[i - 1].startTimeMS + 1;
            int maxTime = regions[i].endTimeMS - 1;
            int clampedTime = std::max(minTime, std::min(maxTime, newTimeMS));

            regions[i - 1].endTimeMS = clampedTime;
            regions[i].startTimeMS = clampedTime;
            return;
        }
    }
}

int SongStructureManager::FindNearestBoundary(int timeMS, int toleranceMS) const
{
    const auto& regions = ActiveRegions();
    if (regions.size() < 2) {
        return -1;
    }

    int nearestTime = -1;
    int nearestDist = toleranceMS + 1;

    for (size_t i = 1; i < regions.size(); i++) {
        int dist = std::abs(regions[i].startTimeMS - timeMS);
        if (dist < nearestDist) {
            nearestDist = dist;
            nearestTime = regions[i].startTimeMS;
        }
    }

    return nearestTime;
}

void SongStructureManager::SetRegionName(size_t index, const std::string& name)
{
    auto& regions = ActiveRegions();
    if (index < regions.size()) {
        regions[index].name = name;
    }
}

void SongStructureManager::SetRegionColor(size_t index, uint32_t colorARGB)
{
    auto& regions = ActiveRegions();
    if (index < regions.size()) {
        regions[index].colorARGB = colorARGB;
    }
}

void SongStructureManager::SetRegions(const std::vector<SongStructureRegion>& regions)
{
    if (_activeViewIndex < 0 || _activeViewIndex >= (int)_views.size()) {
        return;
    }
    auto& view = _views[_activeViewIndex];
    view.regions = regions;
    // Update nextId to be past any existing IDs
    int maxId = 0;
    for (const auto& r : view.regions) {
        if (r.id > maxId) maxId = r.id;
    }
    view.nextId = maxId + 1;
}

void SongStructureManager::Clear()
{
    _views.clear();
    _activeViewIndex = -1;
}

// --- View management ---

const std::string& SongStructureManager::GetActiveViewName() const
{
    if (_activeViewIndex >= 0 && _activeViewIndex < (int)_views.size()) {
        return _views[_activeViewIndex].name;
    }
    return s_emptyName;
}

const std::string& SongStructureManager::GetViewName(size_t index) const
{
    if (index < _views.size()) {
        return _views[index].name;
    }
    return s_emptyName;
}

void SongStructureManager::SetActiveView(int index)
{
    if (index >= 0 && index < (int)_views.size()) {
        _activeViewIndex = index;
    } else if (_views.empty()) {
        _activeViewIndex = -1;
    } else {
        _activeViewIndex = 0;
    }
}

int SongStructureManager::AddView(const std::string& name)
{
    _views.push_back(SongStructureView(name));
    return (int)_views.size() - 1;
}

int SongStructureManager::DuplicateView(int sourceIndex, const std::string& newName)
{
    if (sourceIndex < 0 || sourceIndex >= (int)_views.size()) {
        return AddView(newName);
    }

    SongStructureView newView(newName);
    newView.regions = _views[sourceIndex].regions;
    newView.nextId = _views[sourceIndex].nextId;
    _views.push_back(newView);
    return (int)_views.size() - 1;
}

void SongStructureManager::RenameView(int index, const std::string& name)
{
    if (index >= 0 && index < (int)_views.size()) {
        _views[index].name = name;
    }
}

void SongStructureManager::DeleteView(int index)
{
    if (index < 0 || index >= (int)_views.size()) return;

    _views.erase(_views.begin() + index);

    if (_views.empty()) {
        _activeViewIndex = -1;
    } else if (_activeViewIndex >= (int)_views.size()) {
        _activeViewIndex = (int)_views.size() - 1;
    } else if (_activeViewIndex > index) {
        _activeViewIndex--;
    } else if (_activeViewIndex == index) {
        if (_activeViewIndex >= (int)_views.size()) {
            _activeViewIndex = (int)_views.size() - 1;
        }
    }
}

bool SongStructureManager::AnyViewHasRegions() const
{
    for (const auto& view : _views) {
        if (!view.regions.empty()) return true;
    }
    return false;
}

// --- XML persistence (pugixml) ---

static int ParseIntAttr(const pugi::xml_node& node, const char* name, int defaultVal = 0)
{
    pugi::xml_attribute a = node.attribute(name);
    if (!a) return defaultVal;
    return (int)std::strtol(a.value(), nullptr, 10);
}

static uint32_t ParseHexColorAttr(const pugi::xml_node& node, const char* name, uint32_t defaultVal = 0x40808080)
{
    pugi::xml_attribute a = node.attribute(name);
    if (!a) return defaultVal;
    const char* s = a.value();
    if (s == nullptr || *s == '\0') return defaultVal;
    return (uint32_t)std::strtoul(s, nullptr, 16);
}

static void ParseRegionsFromXml(const pugi::xml_node& parent, std::vector<SongStructureRegion>& regions, int& maxId)
{
    for (pugi::xml_node child = parent.first_child(); child; child = child.next_sibling()) {
        if (std::strcmp(child.name(), "Region") == 0) {
            SongStructureRegion region;
            region.startTimeMS = ParseIntAttr(child, "startTimeMS", 0);
            region.endTimeMS = ParseIntAttr(child, "endTimeMS", 0);
            region.name = child.attribute("name").as_string("");
            region.colorARGB = ParseHexColorAttr(child, "color", 0x40808080);
            region.id = ParseIntAttr(child, "id", 0);
            if (region.id > maxId) {
                maxId = region.id;
            }
            regions.push_back(region);
        }
    }
}

void SongStructureManager::LoadFromXml(const pugi::xml_node& node)
{
    Clear();

    if (!node) return;

    // Detect format: new format has <View> children, old format has <Region> directly
    bool hasViewChildren = false;
    for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
        if (std::strcmp(child.name(), "View") == 0) {
            hasViewChildren = true;
            break;
        }
    }

    if (hasViewChildren) {
        // New format
        for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
            if (std::strcmp(child.name(), "View") == 0) {
                SongStructureView view;
                view.name = child.attribute("name").as_string("Default");
                int maxId = 0;
                ParseRegionsFromXml(child, view.regions, maxId);
                view.nextId = maxId + 1;
                _views.push_back(view);
            }
        }
        int activeIdx = ParseIntAttr(node, "activeView", 0);
        _activeViewIndex = _views.empty() ? -1 : std::clamp(activeIdx, 0, (int)_views.size() - 1);
    } else {
        // Old format: <Region> directly under <SongStructure>
        SongStructureView view("Default");
        int maxId = 0;
        ParseRegionsFromXml(node, view.regions, maxId);
        view.nextId = maxId + 1;
        if (!view.regions.empty()) {
            _views.push_back(view);
            _activeViewIndex = 0;
        }
    }

    // Sort regions in all views
    for (auto& view : _views) {
        std::sort(view.regions.begin(), view.regions.end(),
            [](const SongStructureRegion& a, const SongStructureRegion& b) {
                return a.startTimeMS < b.startTimeMS;
            });
    }
}

void SongStructureManager::SaveToXml(pugi::xml_node parent) const
{
    pugi::xml_node node = parent.append_child("SongStructure");

    char buf[32];
    std::snprintf(buf, sizeof(buf), "%d", _activeViewIndex);
    node.append_attribute("activeView") = buf;

    for (const auto& view : _views) {
        pugi::xml_node viewNode = node.append_child("View");
        viewNode.append_attribute("name") = view.name.c_str();
        for (const auto& region : view.regions) {
            pugi::xml_node regionNode = viewNode.append_child("Region");
            std::snprintf(buf, sizeof(buf), "%d", region.id);
            regionNode.append_attribute("id") = buf;
            std::snprintf(buf, sizeof(buf), "%d", region.startTimeMS);
            regionNode.append_attribute("startTimeMS") = buf;
            std::snprintf(buf, sizeof(buf), "%d", region.endTimeMS);
            regionNode.append_attribute("endTimeMS") = buf;
            regionNode.append_attribute("name") = region.name.c_str();
            std::snprintf(buf, sizeof(buf), "%08X", region.colorARGB);
            regionNode.append_attribute("color") = buf;
        }
    }
}

uint32_t SongStructureManager::GetPaletteColor(int index)
{
    return s_palette[index % PALETTE_SIZE];
}

void SongStructureManager::SortRegions()
{
    auto& regions = ActiveRegions();
    std::sort(regions.begin(), regions.end(),
        [](const SongStructureRegion& a, const SongStructureRegion& b) {
            return a.startTimeMS < b.startTimeMS;
        });
}
