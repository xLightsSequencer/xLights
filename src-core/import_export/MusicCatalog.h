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

// Music / lyric vendor catalog data + parser. The desktop's
// `VendorMusicDialog` (`MSLVendor` / `MSLSequenceLyric`) is wx-only
// and the shared `vendor_catalog::Catalog` is models-only — this is
// the wx-free core equivalent for the iPad vendor browser's
// sequences/lyrics mode.
//
// Schema is intentionally duplicated here from desktop's
// `VendorMusicHelpers.cpp` (the `<musicvendor>` master-index element
// + the per-vendor `<musicinventory>` / `<song>` schema). The two
// should eventually be unified the way `VendorCatalog` unified the
// model catalog — desktop's `MSL*` classes still parse the same XML
// independently today.
//
// wx-free: URLs / cache paths are `std::string`, fetches go through
// `CachedFileDownloader` (already in `src-core/utils/`).

#include <functional>
#include <list>
#include <string>
#include <vector>

#include <pugixml.hpp>

namespace music_catalog {

class Vendor;
class Category;

enum class ItemType { Unknown,
                       Sequence,
                       Lyric };

// One downloadable sequence or lyric entry (`<song>` in the schema).
class Item {
public:
    std::list<std::string> _hashes;
    std::list<std::string> _categoryIds;
    std::string _title;
    std::string _creator;
    std::string _artist;
    std::string _notes;
    ItemType _type = ItemType::Unknown;
    std::string _webpage;      // URL
    std::string _download;     // URL
    std::string _video;        // URL
    std::string _music;        // URL
    Vendor* _vendor = nullptr;

    Item(pugi::xml_node n, Vendor* vendor);
    ~Item() = default;
    Item(const Item&) = delete;
    Item& operator=(const Item&) = delete;

    [[nodiscard]] bool InCategory(const std::string& categoryId) const;
    [[nodiscard]] std::string GetTypeName() const;
    [[nodiscard]] std::string GetDescription() const;
    // File extension the download should be saved under: "xtiming"
    // for lyrics, "zip" for sequences, else derived from the URL.
    [[nodiscard]] std::string GetExt() const;
    // Filename portion of the download URL (after the last '/'),
    // falling back to "<title>.<ext>" when the URL has none.
    [[nodiscard]] std::string GetDownloadFileName() const;
};

// Hierarchical category (same tree shape as the model catalog).
class Category {
public:
    std::string _id;
    std::string _name;
    Category* _parent = nullptr;
    std::list<Category*> _categories;     // owning
    Vendor* _vendor = nullptr;

    Category(pugi::xml_node n, Category* parent, Vendor* vendor);
    ~Category();
    Category(const Category&) = delete;
    Category& operator=(const Category&) = delete;

    [[nodiscard]] std::string GetPath() const;

private:
    void ParseCategories(pugi::xml_node n);
};

// One vendor's complete sequence/lyric inventory.
class Vendor {
public:
    std::string _name;
    std::string _contact;
    std::string _email;
    std::string _phone;
    std::string _website;
    std::string _facebook;
    std::string _twitter;
    std::string _notes;
    std::string _logoFile;                 // local cache path
    std::list<Category*> _categories;       // owning
    std::list<Item*> _items;                // owning
    int _maxItems = -1;

    explicit Vendor(const std::string& name);
    Vendor(pugi::xml_document& doc, int maxItems);
    ~Vendor();
    Vendor(const Vendor&) = delete;
    Vendor& operator=(const Vendor&) = delete;

    [[nodiscard]] std::string GetDescription() const;

private:
    void ParseCategories(pugi::xml_node n);
};

// Top-level music/lyric catalog loader. Hits the same master vendor
// index as the model catalog but reads `<musicvendor>` entries
// instead of `<vendor>` entries.
class Catalog {
public:
    Catalog() = default;
    ~Catalog();
    Catalog(const Catalog&) = delete;
    Catalog& operator=(const Catalog&) = delete;

    using ProgressFn = std::function<bool(int, const std::string&)>;

    bool Load(ProgressFn progress = nullptr);

    [[nodiscard]] const std::list<Vendor*>& Vendors() const { return _vendors; }

    // Download `url` into `destFolder` as `fileName` (skipping the
    // fetch if it already exists). Returns the saved path, or "" on
    // failure. Used by the bridge to land sequences/lyrics straight
    // in the show folder rather than the shared cache.
    static std::string DownloadTo(const std::string& url,
                                  const std::string& destFolder,
                                  const std::string& fileName);

private:
    std::list<Vendor*> _vendors;            // owning
};

} // namespace music_catalog
