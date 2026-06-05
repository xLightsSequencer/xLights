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

// Vendor catalog data + parser. Shared between desktop's
// `VendorModelDialog` and iPad's vendor-browser sheet — same XML
// schema, single parser. wx-free; URLs are plain `std::string`,
// local cache paths are plain `std::string`, size strings get
// parsed via `std::regex`. Network fetches go through
// `CachedFileDownloader` (already in `src-core/utils/`).
//
// Fields + ownership choices mirror the legacy `M*` classes that
// previously lived in desktop's VendorModelDialog.cpp (raw
// owning pointers, public fields) so the dialog can use the
// shared types via simple `using` aliases.

#include <functional>
#include <list>
#include <string>
#include <vector>

#include <pugixml.hpp>

namespace vendor_catalog {

class Vendor;
class Category;
class Model;
class ModelWiring;

// Wiring option on a model — owns the `.xmodel` file URL the
// user ultimately downloads.
class ModelWiring {
public:
    std::list<std::string> _images;        // image URLs
    std::list<std::string> _imageFiles;    // local cache paths
    std::string _name;
    std::string _wiringDescription;
    std::string _xmodelLink;
    std::string _xmodelFile;               // populated by DownloadXModel
    Model* _model = nullptr;
    int _modelWidthMM = -1;
    int _modelHeightMM = -1;
    int _modelDepthMM = -1;

    ModelWiring(pugi::xml_node n, Model* model,
                int widthMM, int heightMM, int depthMM);
    ~ModelWiring() = default;
    ModelWiring(const ModelWiring&) = delete;
    ModelWiring& operator=(const ModelWiring&) = delete;

    [[nodiscard]] int GetWidthMM() const { return _modelWidthMM; }
    [[nodiscard]] int GetHeightMM() const { return _modelHeightMM; }
    [[nodiscard]] int GetDepthMM() const { return _modelDepthMM; }
    [[nodiscard]] bool HasDownloadableXmodel() const { return !_xmodelLink.empty(); }
    [[nodiscard]] std::string GetDescription() const;

    void AddImages(const std::list<std::string>& images);
    void DownloadImages();
    // Fetch the wiring's `.xmodel` (or `.zip`) into the shared
    // file cache and patch in pixel-attribute metadata from the
    // parent Model when absent. Idempotent — second calls reuse
    // the cached path.
    void DownloadXModel();
};

// One catalog entry — a physical model with one or more wirings.
class Model {
public:
    std::string _id;
    std::list<std::string> _categoryIds;
    std::string _name;
    std::string _type;
    std::string _material;
    std::string _thickness;
    std::string _width;
    std::string _height;
    std::string _depth;
    std::string _pixelCount;
    std::string _pixelSpacing;
    std::string _pixelDescription;
    std::string _webpage;                  // URL
    std::list<std::string> _images;        // URLs
    std::list<std::string> _imageFiles;    // local cache paths
    std::string _notes;
    std::list<ModelWiring*> _wiring;       // owning — destructor deletes
    Vendor* _vendor = nullptr;

    Model(pugi::xml_node n, Vendor* vendor);
    ~Model();
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    [[nodiscard]] bool InCategory(const std::string& categoryId) const;
    [[nodiscard]] bool HasDownloadableXmodel() const;
    [[nodiscard]] std::string GetDescription() const;
    void DownloadImages();

    // Static helper for parsing human-typed size strings
    // (e.g. "10mm", "30 cm", "5'2\"", "2.5\"") into millimetres.
    // Returns -1 if the format isn't recognised.
    static int InterpretSize(const std::string& size);
};

// Hierarchical category. Vendors group their models into a tree;
// a `Model` records the leaf-category IDs it belongs to.
class Category {
public:
    std::string _id;
    std::string _name;
    Category* _parent = nullptr;
    std::list<Category*> _categories;     // child sub-categories (owning)
    Vendor* _vendor = nullptr;

    Category(pugi::xml_node n, Category* parent, Vendor* vendor);
    ~Category();
    Category(const Category&) = delete;
    Category& operator=(const Category&) = delete;

    [[nodiscard]] std::string GetPath() const;

private:
    void ParseCategories(pugi::xml_node n);
};

// One vendor's complete inventory (loaded from its per-vendor
// XML). Owns its category tree + model list.
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
    std::string _logoFile;                  // local cache path
    std::list<Category*> _categories;        // owning
    std::list<Model*> _models;               // owning
    int _maxModels = -1;

    // Suppressed vendor — only the name is known.
    explicit Vendor(const std::string& name);
    // Full parse from per-vendor XML.
    Vendor(pugi::xml_document& doc, int maxModels);
    ~Vendor();
    Vendor(const Vendor&) = delete;
    Vendor& operator=(const Vendor&) = delete;

    [[nodiscard]] std::string GetDescription() const;
    [[nodiscard]] std::list<Model*> GetModels(const std::string& categoryId) const;

private:
    void ParseCategories(pugi::xml_node n);
};

// Top-level catalog loader. Hits the master vendor index, then
// each vendor's URL (with cache + backup fallback). Suppressed
// vendors are kept as name-only entries so the UI can offer to
// unsuppress them.
class Catalog {
public:
    Catalog() = default;
    ~Catalog();
    Catalog(const Catalog&) = delete;
    Catalog& operator=(const Catalog&) = delete;

    // Per-step progress callback: (pct 0-100, label).
    using ProgressFn = std::function<bool(int, const std::string&)>;

    // Fetch the master vendor index + each vendor inventory.
    // `isSuppressed` lets callers consult app preferences for
    // hidden vendors. Returns true if at least one vendor parsed.
    bool Load(ProgressFn progress = nullptr,
              std::function<bool(const std::string&)> isSuppressed = nullptr);

    [[nodiscard]] const std::list<Vendor*>& Vendors() const { return _vendors; }

private:
    std::list<Vendor*> _vendors;            // owning
};

} // namespace vendor_catalog
