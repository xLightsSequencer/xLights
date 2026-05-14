/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "VendorCatalog.h"

#include "utils/CachedFileDownloader.h"
#include "utils/ExternalHooks.h"
#include "utils/string_utils.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <regex>

namespace vendor_catalog {

namespace {

constexpr const char* kVendorIndexURL =
    "https://raw.githubusercontent.com/xLightsSequencer/xLights/master/download/xlights_vendors.xml";
constexpr const char* kVendorIndexBackupURL =
    "https://nutcracker123.com/xlights/vendors/xlights_vendors.xml";

// Strip ASCII whitespace from a string (mirrors the legacy
// `std::remove_if(... isspace)` cleanup in `InterpretSize`).
std::string Trim(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            out += c;
        }
    }
    return out;
}

// Adapter around `CachedFileDownloader::GetFile` for the catalog
// fetches. Returns parsed pugi document (owned by caller) or nullptr.
std::unique_ptr<pugi::xml_document> FetchAndParse(const std::string& url) {
    if (url.empty()) return nullptr;
    std::string fname = CachedFileDownloader::GetDefaultCache().GetFile(url, CACHETIME_DAY);
    if (fname.empty() || !FileExists(fname)) return nullptr;
    auto doc = std::make_unique<pugi::xml_document>();
    if (!doc->load_file(fname.c_str())) return nullptr;
    if (!doc->document_element()) return nullptr;
    return doc;
}

} // namespace

// ---------------------------------------------------------------
// ModelWiring
// ---------------------------------------------------------------

ModelWiring::ModelWiring(pugi::xml_node n, Model* model,
                         int widthMM, int heightMM, int depthMM)
    : _model(model),
      _modelWidthMM(widthMM),
      _modelHeightMM(heightMM),
      _modelDepthMM(depthMM) {
    for (pugi::xml_node l = n.first_child(); l; l = l.next_sibling()) {
        if (l.type() == pugi::node_comment) continue;
        const std::string nn = ::Lower(l.name());
        if (nn == "name") {
            _name = l.text().get();
        } else if (nn == "description") {
            _wiringDescription = l.text().get();
        } else if (nn == "xmodellink") {
            _xmodelLink = l.text().get();
        } else if (nn == "imagefile") {
            std::string url = l.text().get();
            if (!url.empty()) _images.push_back(url);
        } else if (!nn.empty()) {
            spdlog::warn("ModelWiring: unknown element '{}'", nn);
        }
    }
}

void ModelWiring::AddImages(const std::list<std::string>& images) {
    for (const auto& u : images) _images.push_back(u);
}

void ModelWiring::DownloadImages() {
    if (_imageFiles.size() == _images.size()) return;
    _imageFiles.clear();
    for (const auto& url : _images) {
        std::string fn = CachedFileDownloader::GetDefaultCache().GetFile(url, CACHETIME_LONG);
        if (!fn.empty()) _imageFiles.push_back(fn);
    }
}

void ModelWiring::DownloadXModel() {
    if (!_xmodelFile.empty() && FileExists(_xmodelFile)) return;
    if (_xmodelLink.empty()) return;

    // Zip-wrapped models exist; preserve the extension hint so the
    // cache stores the right file type.
    std::string ext = "xmodel";
    if (_xmodelLink.size() >= 4) {
        const std::string tail = ::Lower(_xmodelLink.substr(_xmodelLink.size() - 4));
        if (tail == ".zip") ext = "zip";
    }

    _xmodelFile = CachedFileDownloader::GetDefaultCache().GetFile(
        _xmodelLink, CACHETIME_LONG, ext);
    if (_xmodelFile.empty() || !FileExists(_xmodelFile)) return;

    // Patch in PixelType / PixelMinimumSpacingInches / PixelCount
    // attributes from the parent Model when the downloaded file
    // lacks them — desktop's MModelWiring::DownloadXModel did
    // this so the imported model has accurate pixel metadata.
    if (ext != "xmodel" || !_model) return;
    pugi::xml_document d;
    if (!d.load_file(_xmodelFile.c_str())) return;
    pugi::xml_node root = d.document_element();
    if (!root) return;
    bool changed = false;
    if (!root.attribute("PixelType") && !_model->_pixelDescription.empty()) {
        root.append_attribute("PixelType") = _model->_pixelDescription.c_str();
        changed = true;
    }
    if (!root.attribute("PixelMinimumSpacingInches")) {
        long spacing = std::strtol(_model->_pixelSpacing.c_str(), nullptr, 10);
        if (spacing != 0) {
            root.append_attribute("PixelMinimumSpacingInches") = static_cast<int>(spacing);
            changed = true;
        }
    }
    if (!root.attribute("PixelCount")) {
        long count = std::strtol(_model->_pixelCount.c_str(), nullptr, 10);
        if (count != 0) {
            root.append_attribute("PixelCount") = static_cast<int>(count);
            changed = true;
        }
    }
    if (changed) {
        d.save_file(_xmodelFile.c_str());
    }
}

std::string ModelWiring::GetDescription() const {
    std::string desc;
    if (_model) {
        desc = _model->GetDescription();
        desc += "\n";
    }
    if (!_name.empty()) desc += "Wiring Option: " + _name + "\n\n";
    if (!_wiringDescription.empty()) desc += _wiringDescription;
    return desc;
}

// ---------------------------------------------------------------
// Model
// ---------------------------------------------------------------

Model::Model(pugi::xml_node n, Vendor* vendor)
    : _vendor(vendor) {
    for (pugi::xml_node l = n.first_child(); l; l = l.next_sibling()) {
        if (l.type() == pugi::node_comment) continue;
        const std::string nn = ::Lower(l.name());
        if (nn == "id") _id = l.text().get();
        else if (nn == "categoryid") _categoryIds.push_back(l.text().get());
        else if (nn == "name") _name = l.text().get();
        else if (nn == "type") _type = l.text().get();
        else if (nn == "material") _material = l.text().get();
        else if (nn == "thickness") _thickness = l.text().get();
        else if (nn == "width") _width = l.text().get();
        else if (nn == "height") _height = l.text().get();
        else if (nn == "depth") _depth = l.text().get();
        else if (nn == "pixelcount") _pixelCount = l.text().get();
        else if (nn == "pixelspacing") _pixelSpacing = l.text().get();
        else if (nn == "pixeldescription") _pixelDescription = l.text().get();
        else if (nn == "notes") _notes = l.text().get();
        else if (nn == "weblink") _webpage = l.text().get();
        else if (nn == "imagefile") {
            std::string url = l.text().get();
            if (!url.empty()) _images.push_back(url);
        } else if (nn == "wiring") {
            // handled in second pass — dimensions need to be parsed first.
        } else if (!nn.empty()) {
            spdlog::warn("Model: unknown element '{}'", nn);
        }
    }
    const int widthMM = InterpretSize(_width);
    const int heightMM = InterpretSize(_height);
    const int depthMM = InterpretSize(_depth);
    for (pugi::xml_node l = n.first_child(); l; l = l.next_sibling()) {
        if (l.type() == pugi::node_comment) continue;
        if (::Lower(l.name()) == "wiring") {
            _wiring.push_back(new ModelWiring(l, this, widthMM, heightMM, depthMM));
        }
    }
    for (auto* w : _wiring) {
        w->AddImages(_images);
    }
}

Model::~Model() {
    for (auto* w : _wiring) delete w;
}

bool Model::InCategory(const std::string& categoryId) const {
    for (const auto& c : _categoryIds) {
        if (c == categoryId) return true;
    }
    return false;
}

bool Model::HasDownloadableXmodel() const {
    for (const auto* w : _wiring) {
        if (w->HasDownloadableXmodel()) return true;
    }
    return false;
}

namespace {
std::string PadTitle(std::string t, size_t width) {
    while (t.size() < width) t += " ";
    return t;
}
} // namespace

std::string Model::GetDescription() const {
    std::string desc;
    auto pad = [](const std::string& s) { return PadTitle(s, 18); };
    if (!_name.empty())             desc += pad("Name:") + _name + "\n\n";
    if (!_type.empty())             desc += pad("Type:") + _type + "\n";
    if (!_material.empty())         desc += pad("Material:") + _material + "\n";
    if (!_thickness.empty())        desc += pad("Thickness:") + _thickness + "\n";
    if (!_width.empty())            desc += pad("Width:") + _width + "\n";
    if (!_height.empty())           desc += pad("Height:") + _height + "\n";
    if (!_depth.empty())            desc += pad("Depth:") + _depth + "\n";
    if (!_pixelCount.empty())       desc += pad("Pixel Count:") + _pixelCount + "\n";
    if (!_pixelSpacing.empty())     desc += pad("Minimum Pixel Spacing:") + _pixelSpacing + "\n";
    if (!_pixelDescription.empty()) desc += pad("Pixel Description:") + _pixelDescription + "\n";
    if (!_notes.empty())            desc += "\n" + _notes + "\n";
    return desc;
}

void Model::DownloadImages() {
    if (_imageFiles.size() == _images.size()) return;
    _imageFiles.clear();
    for (const auto& url : _images) {
        std::string fn = CachedFileDownloader::GetDefaultCache().GetFile(url, CACHETIME_LONG);
        if (!fn.empty()) _imageFiles.push_back(fn);
    }
}

int Model::InterpretSize(const std::string& size) {
    if (size.empty()) return -1;
    const std::string s = Trim(size);

    // Plain "<num>mm" or "<num.num>mm".
    if (std::regex_match(s, std::regex(R"(^\d+\.?\d*mm$)"))) {
        return static_cast<int>(std::strtod(s.c_str(), nullptr));
    }
    // "(xxmm)" embedded.
    {
        std::smatch m;
        if (std::regex_search(s, m, std::regex(R"(\((\d+\.?\d*)mm\))"))) {
            const double v = std::strtod(m[1].str().c_str(), nullptr);
            if (v != 0) return static_cast<int>(v);
        }
    }
    // "<num>cm" or "<num.num>cm".
    if (std::regex_match(s, std::regex(R"(^\d+\.?\d*cm$)"))) {
        const double v = std::strtod(s.c_str(), nullptr);
        if (v != 0) return static_cast<int>(v * 10);
    }
    // "(xxcm)" embedded.
    {
        std::smatch m;
        if (std::regex_search(s, m, std::regex(R"(\((\d+\.?\d*)cm\))"))) {
            const double v = std::strtod(m[1].str().c_str(), nullptr);
            if (v != 0) return static_cast<int>(v * 10);
        }
    }
    // Bare number → assume inches.
    if (std::regex_match(s, std::regex(R"(^\d+\.?\d*$)"))) {
        const double v = std::strtod(s.c_str(), nullptr);
        if (v != 0) return static_cast<int>(v * 25.4);
    }
    // Feet+inches: 5'2"
    {
        std::smatch m;
        if (std::regex_match(s, m, std::regex(R"(^(\d+\.?\d*)'(\d+\.?\d*)\"$)"))) {
            const double ft = std::strtod(m[1].str().c_str(), nullptr);
            const double in = std::strtod(m[2].str().c_str(), nullptr);
            if (ft != 0 || in != 0) return static_cast<int>(((ft * 12.0) + in) * 25.4);
        }
    }
    // Inches with quote.
    if (std::regex_match(s, std::regex(R"(^\d+\.?\d*\"$)"))) {
        const double v = std::strtod(s.c_str(), nullptr);
        if (v != 0) return static_cast<int>(v * 25.4);
    }
    spdlog::warn("VendorCatalog: unable to interpret size '{}'", size);
    return -1;
}

// ---------------------------------------------------------------
// Category
// ---------------------------------------------------------------

Category::Category(pugi::xml_node n, Category* parent, Vendor* vendor)
    : _parent(parent), _vendor(vendor) {
    for (pugi::xml_node e = n.first_child(); e; e = e.next_sibling()) {
        if (e.type() == pugi::node_comment) continue;
        const std::string nn = ::Lower(e.name());
        if (nn == "id") _id = e.text().get();
        else if (nn == "name") _name = e.text().get();
        else if (nn == "categories") ParseCategories(e);
        else if (!nn.empty()) {
            spdlog::warn("Category: unknown element '{}'", nn);
        }
    }
}

void Category::ParseCategories(pugi::xml_node n) {
    for (pugi::xml_node l = n.first_child(); l; l = l.next_sibling()) {
        if (l.type() == pugi::node_comment) continue;
        if (::Lower(l.name()) == "category") {
            _categories.push_back(new Category(l, this, _vendor));
        }
    }
}

Category::~Category() {
    for (auto* c : _categories) delete c;
}

std::string Category::GetPath() const {
    if (_parent) return _parent->GetPath() + "/" + _name;
    return _name;
}

// ---------------------------------------------------------------
// Vendor
// ---------------------------------------------------------------

Vendor::Vendor(const std::string& name) : _name(name) {}

Vendor::Vendor(pugi::xml_document& doc, int maxModels) : _maxModels(maxModels) {
    pugi::xml_node root = doc.document_element();
    if (!root) return;
    const std::string rn = ::Lower(root.name());
    if (rn != "modelinventory") return;
    // Schema (xlights.org per-vendor XML):
    //   <modelinventory>
    //     <vendor>      — name / contact / website / ...
    //     <categories>  — hierarchical category tree (sibling)
    //     <models>      — model list (sibling)
    //   </modelinventory>
    // i.e. categories/models are NOT inside <vendor>.
    for (pugi::xml_node e = root.first_child(); e; e = e.next_sibling()) {
        if (e.type() == pugi::node_comment) continue;
        const std::string ename = ::Lower(e.name());
        if (ename == "vendor") {
            for (pugi::xml_node v = e.first_child(); v; v = v.next_sibling()) {
                if (v.type() == pugi::node_comment) continue;
                const std::string nn = ::Lower(v.name());
                if (nn == "name") _name = v.text().get();
                else if (nn == "contact") _contact = v.text().get();
                else if (nn == "email") _email = v.text().get();
                else if (nn == "phone") _phone = v.text().get();
                else if (nn == "website") _website = v.text().get();
                else if (nn == "facebook") _facebook = v.text().get();
                else if (nn == "twitter") _twitter = v.text().get();
                else if (nn == "notes") _notes = v.text().get();
                else if (nn == "logolink") {
                    const std::string url = v.text().get();
                    if (!url.empty()) {
                        _logoFile = CachedFileDownloader::GetDefaultCache().GetFile(
                            url, CACHETIME_LONG);
                    }
                }
                else if (!nn.empty()) {
                    spdlog::warn("Vendor: unknown <vendor> child element '{}'", nn);
                }
            }
        } else if (ename == "categories") {
            ParseCategories(e);
        } else if (ename == "models") {
            int count = 0;
            for (pugi::xml_node m = e.first_child(); m; m = m.next_sibling()) {
                if (m.type() == pugi::node_comment) continue;
                if (::Lower(m.name()) != "model") continue;
                count++;
                if (_maxModels < 1 || count <= _maxModels) {
                    _models.push_back(new Model(m, this));
                }
            }
        } else if (!ename.empty()) {
            spdlog::warn("Vendor: unknown <modelinventory> child element '{}'", ename);
        }
    }
}

Vendor::~Vendor() {
    for (auto* c : _categories) delete c;
    for (auto* m : _models) delete m;
}

void Vendor::ParseCategories(pugi::xml_node n) {
    for (pugi::xml_node l = n.first_child(); l; l = l.next_sibling()) {
        if (l.type() == pugi::node_comment) continue;
        if (::Lower(l.name()) == "category") {
            _categories.push_back(new Category(l, nullptr, this));
        }
    }
}

std::string Vendor::GetDescription() const {
    auto pad = [](const std::string& s) { return PadTitle(s, 9); };
    std::string desc;
    if (!_name.empty())    desc += pad("Name:") + _name + "\n\n";
    if (!_contact.empty()) desc += pad("Contact:") + _contact + "\n";
    if (!_phone.empty())   desc += pad("Phone:") + _phone + "\n";
    if (!_email.empty())   desc += pad("Email:") + _email + "\n";
    if (!_twitter.empty()) desc += pad("Twitter:") + _twitter + "\n";
    if (!_notes.empty())   desc += "\n" + _notes + "\n";
    return desc;
}

std::list<Model*> Vendor::GetModels(const std::string& categoryId) const {
    std::list<Model*> out;
    for (auto* m : _models) {
        if (m->InCategory(categoryId)) out.push_back(m);
    }
    return out;
}

// ---------------------------------------------------------------
// Catalog
// ---------------------------------------------------------------

bool Catalog::Load(ProgressFn progress,
                   std::function<bool(const std::string&)> isSuppressed) {
    auto report = [&](int pct, const std::string& label) {
        if (progress) progress(pct, label);
    };

    report(0, "Downloading vendor list");
    auto indexDoc = FetchAndParse(kVendorIndexURL);
    if (!indexDoc) {
        report(5, "Trying backup vendor list");
        indexDoc = FetchAndParse(kVendorIndexBackupURL);
    }
    if (!indexDoc) {
        spdlog::warn("VendorCatalog: master + backup vendor index unreachable");
        return false;
    }
    report(10, "Parsing vendor list");

    // Collect vendor index entries first so we can compute progress.
    struct Entry { std::string name; std::string url; int maxModels; };
    std::vector<Entry> entries;
    pugi::xml_node root = indexDoc->document_element();
    for (pugi::xml_node v = root.first_child(); v; v = v.next_sibling()) {
        if (v.type() == pugi::node_comment) continue;
        if (::Lower(v.name()) != "vendor") continue;
        Entry e{};
        e.maxModels = -1;
        for (pugi::xml_node l = v.first_child(); l; l = l.next_sibling()) {
            const std::string nn = ::Lower(l.name());
            if (nn == "link") e.url = l.text().get();
            else if (nn == "maxmodels") e.maxModels = l.text().as_int();
            else if (nn == "name") e.name = l.text().get();
        }
        if (!e.name.empty()) entries.push_back(std::move(e));
    }

    int idx = 0;
    for (const auto& e : entries) {
        ++idx;
        const int basePct = 10 + (idx * 85 / static_cast<int>(entries.size()));
        if (isSuppressed && isSuppressed(e.name)) {
            _vendors.push_back(new Vendor(e.name));
            spdlog::debug("VendorCatalog: vendor '{}' suppressed by preference", e.name);
            report(basePct, e.name + " (suppressed)");
            continue;
        }
        if (e.url.empty()) {
            spdlog::debug("VendorCatalog: vendor '{}' has no url", e.name);
            continue;
        }
        report(basePct, "Downloading " + e.name);
        auto vdoc = FetchAndParse(e.url);
        if (!vdoc) {
            spdlog::debug("VendorCatalog: vendor '{}' inventory failed to load", e.name);
            continue;
        }
        _vendors.push_back(new Vendor(*vdoc, e.maxModels));
    }
    report(100, "Catalog ready");
    return !_vendors.empty();
}

Catalog::~Catalog() {
    for (auto* v : _vendors) delete v;
}

} // namespace vendor_catalog
