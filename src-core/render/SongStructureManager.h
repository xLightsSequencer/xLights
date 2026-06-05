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

#include <vector>
#include <string>
#include <cstdint>

#include <pugixml.hpp>

struct SongStructureRegion
{
    int id;
    int startTimeMS;
    int endTimeMS;
    std::string name;
    uint32_t colorARGB; // 0xAARRGGBB

    SongStructureRegion() : id(0), startTimeMS(0), endTimeMS(0), colorARGB(0x40808080) {}
    SongStructureRegion(int _id, int _start, int _end, const std::string& _name, uint32_t _color)
        : id(_id), startTimeMS(_start), endTimeMS(_end), name(_name), colorARGB(_color) {}
};

struct SongStructureView
{
    std::string name;
    std::vector<SongStructureRegion> regions;
    int nextId = 1;

    SongStructureView() = default;
    SongStructureView(const std::string& _name) : name(_name) {}
};

class SongStructureManager
{
public:
    SongStructureManager();
    ~SongStructureManager();

    // Region access (operates on active view)
    size_t GetRegionCount() const;
    const SongStructureRegion& GetRegion(size_t index) const;
    SongStructureRegion& GetRegion(size_t index);
    const std::vector<SongStructureRegion>& GetRegions() const;
    bool HasRegions() const;

    // Find region at a given time
    int GetRegionIndexAtTime(int timeMS) const;
    const SongStructureRegion* GetRegionAtTime(int timeMS) const;

    // Boundary operations
    void AddBoundary(int timeMS, int sequenceEndMS);
    void DeleteBoundary(int timeMS);
    void MoveBoundary(int oldTimeMS, int newTimeMS);
    int FindNearestBoundary(int timeMS, int toleranceMS) const;

    // Region editing
    void SetRegionName(size_t index, const std::string& name);
    void SetRegionColor(size_t index, uint32_t colorARGB);

    // Bulk region creation — replaces all regions in the active view
    void SetRegions(const std::vector<SongStructureRegion>& regions);

    // Clear all views
    void Clear();

    // View management
    size_t GetViewCount() const { return _views.size(); }
    int GetActiveViewIndex() const { return _activeViewIndex; }
    const std::string& GetActiveViewName() const;
    const std::string& GetViewName(size_t index) const;
    void SetActiveView(int index);
    int AddView(const std::string& name);
    int DuplicateView(int sourceIndex, const std::string& newName);
    void RenameView(int index, const std::string& name);
    void DeleteView(int index);
    bool AnyViewHasRegions() const;

    // XML persistence (pugixml — wx-free)
    void LoadFromXml(const pugi::xml_node& node);
    void SaveToXml(pugi::xml_node parent) const;

    // Get color palette for auto-assignment
    static uint32_t GetPaletteColor(int index);
    static const int PALETTE_SIZE = 8;

private:
    void SortRegions();
    std::vector<SongStructureRegion>& ActiveRegions();
    const std::vector<SongStructureRegion>& ActiveRegions() const;
    int& ActiveNextId();

    std::vector<SongStructureView> _views;
    int _activeViewIndex = -1;

    // Used when no active view exists
    static std::vector<SongStructureRegion> s_emptyRegions;
    static int s_dummyNextId;
    static const std::string s_emptyName;
};
