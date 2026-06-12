/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MusicCatalog.h"

#include "utils/CachedFileDownloader.h"
#include "utils/ExternalHooks.h"
#include "utils/string_utils.h"

#include <spdlog/spdlog.h>

#include <algorithm>

namespace music_catalog {

namespace {

constexpr const char* kVendorIndexURL =
    "https://nutcracker123.com/xlights/vendors/xlights_vendors.xml";
constexpr const char* kVendorIndexBackupURL =
    "https://raw.githubusercontent.com/xLightsSequencer/xLights/master/download/xlights_vendors.xml";

std::unique_ptr<pugi::xml_document> FetchAndParse(const std::string& url) {
    if (url.empty()) return nullptr;
    std::string fname = CachedFileDownloader::GetDefaultCache().GetFile(url, CACHETIME_DAY);
    if (fname.empty() || !FileExists(fname)) return nullptr;
    auto doc = std::make_unique<pugi::xml_document>();
    if (!doc->load_file(fname.c_str())) return nullptr;
    if (!doc->document_element()) return nullptr;
    return doc;
}

std::string ReplaceLiteralNewlines(std::string s) {
    const std::string find = "\\n";
    for (std::string::size_type i = 0; (i = s.find(find, i)) != std::string::npos;) {
        s.replace(i, find.length(), "\n");
        i += 1;
    }
    return s;
}

} // namespace

// ---------------------------------------------------------------
// Item
// ---------------------------------------------------------------

Item::Item(pugi::xml_node n, Vendor* vendor)
    : _vendor(vendor) {
    for (pugi::xml_node l = n.first_child(); l; l = l.next_sibling()) {
        if (l.type() == pugi::node_comment) continue;
        const std::string nn = ::Lower(l.name());
        if (nn == "hash") _hashes.push_back(l.text().get());
        else if (nn == "categoryid") _categoryIds.push_back(l.text().get());
        else if (nn == "creator") _creator = l.text().get();
        else if (nn == "title") _title = l.text().get();
        else if (nn == "artist") _artist = l.text().get();
        else if (nn == "notes") _notes = l.text().get();
        else if (nn == "sequence") _type = ItemType::Sequence;
        else if (nn == "lyric") _type = ItemType::Lyric;
        else if (nn == "weblink") _webpage = l.text().get();
        else if (nn == "video") _video = l.text().get();
        else if (nn == "music") _music = l.text().get();
        else if (nn == "download") _download = l.text().get();
        else if (!nn.empty()) {
            spdlog::warn("MusicCatalog Item: unknown element '{}'", nn);
        }
    }
}

bool Item::InCategory(const std::string& categoryId) const {
    for (const auto& c : _categoryIds) {
        if (c == categoryId) return true;
    }
    return false;
}

std::string Item::GetTypeName() const {
    if (_type == ItemType::Sequence) return "Sequence";
    if (_type == ItemType::Lyric) return "Lyrics";
    return "Unknown";
}

std::string Item::GetExt() const {
    if (_type == ItemType::Lyric) return "xtiming";
    if (_type == ItemType::Sequence) return "zip";
    // Derive from the URL tail.
    auto dot = _download.find_last_of('.');
    auto slash = _download.find_last_of('/');
    if (dot != std::string::npos && (slash == std::string::npos || dot > slash)) {
        return _download.substr(dot + 1);
    }
    return "";
}

std::string Item::GetDownloadFileName() const {
    std::string name = _download;
    auto q = name.find('?');
    if (q != std::string::npos) name = name.substr(0, q);
    auto slash = name.find_last_of('/');
    if (slash != std::string::npos) name = name.substr(slash + 1);
    if (name.empty()) {
        name = _title;
        const std::string ext = GetExt();
        if (!ext.empty()) name += "." + ext;
    }
    return name;
}

std::string Item::GetDescription() const {
    auto pad = [](std::string s) {
        while (s.size() < 18) s += " ";
        return s;
    };
    std::string desc;
    if (!_title.empty())   desc += pad("Song:") + _title + "\n\n";
    if (!_artist.empty())  desc += pad("Artist:") + _artist + "\n\n";
    desc += pad("Type:") + GetTypeName() + "\n";
    if (!_creator.empty()) desc += pad("Creator:") + _creator + "\n";
    if (!_notes.empty())   desc += "\n" + _notes + "\n";
    return ReplaceLiteralNewlines(desc);
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
            spdlog::warn("MusicCatalog Category: unknown element '{}'", nn);
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

Vendor::Vendor(pugi::xml_document& doc, int maxItems) : _maxItems(maxItems) {
    pugi::xml_node root = doc.document_element();
    if (!root) return;
    if (::Lower(root.name()) != "musicinventory") return;
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
                } else if (!nn.empty()) {
                    spdlog::warn("MusicCatalog Vendor: unknown <vendor> child '{}'", nn);
                }
            }
        } else if (ename == "categories") {
            ParseCategories(e);
        } else if (ename == "music") {
            int count = 0;
            for (pugi::xml_node m = e.first_child(); m; m = m.next_sibling()) {
                if (m.type() == pugi::node_comment) continue;
                if (::Lower(m.name()) != "song") continue;
                count++;
                if (_maxItems < 1 || count <= _maxItems) {
                    _items.push_back(new Item(m, this));
                }
            }
        } else if (!ename.empty()) {
            spdlog::warn("MusicCatalog Vendor: unknown <musicinventory> child '{}'", ename);
        }
    }
    _items.sort([](const Item* a, const Item* b) {
        return ::Lower(a->_title) < ::Lower(b->_title);
    });
}

Vendor::~Vendor() {
    for (auto* c : _categories) delete c;
    for (auto* i : _items) delete i;
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
    auto pad = [](std::string s) {
        while (s.size() < 9) s += " ";
        return s;
    };
    std::string desc;
    if (!_name.empty())    desc += pad("Name:") + _name + "\n\n";
    if (!_contact.empty()) desc += pad("Contact:") + _contact + "\n";
    if (!_phone.empty())   desc += pad("Phone:") + _phone + "\n";
    if (!_email.empty())   desc += pad("Email:") + _email + "\n";
    if (!_twitter.empty()) desc += pad("Twitter:") + _twitter + "\n";
    if (!_notes.empty())   desc += "\n" + _notes + "\n";
    return ReplaceLiteralNewlines(desc);
}

// ---------------------------------------------------------------
// Catalog
// ---------------------------------------------------------------

bool Catalog::Load(ProgressFn progress) {
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
        spdlog::warn("MusicCatalog: master + backup vendor index unreachable");
        return false;
    }
    report(10, "Parsing vendor list");

    struct Entry { std::string url; int maxItems; };
    std::vector<Entry> entries;
    pugi::xml_node root = indexDoc->document_element();
    for (pugi::xml_node v = root.first_child(); v; v = v.next_sibling()) {
        if (v.type() == pugi::node_comment) continue;
        if (::Lower(v.name()) != "musicvendor") continue;
        Entry e{};
        e.maxItems = -1;
        for (pugi::xml_node l = v.first_child(); l; l = l.next_sibling()) {
            const std::string nn = ::Lower(l.name());
            if (nn == "link") e.url = l.text().get();
            else if (nn == "maxitems") e.maxItems = l.text().as_int();
        }
        if (!e.url.empty()) entries.push_back(std::move(e));
    }

    int idx = 0;
    for (const auto& e : entries) {
        ++idx;
        const int basePct = 10 + (idx * 85 / std::max<int>(1, static_cast<int>(entries.size())));
        report(basePct, "Downloading vendor inventory");
        auto vdoc = FetchAndParse(e.url);
        if (!vdoc) {
            spdlog::debug("MusicCatalog: vendor inventory '{}' failed to load", e.url);
            continue;
        }
        _vendors.push_back(new Vendor(*vdoc, e.maxItems));
    }
    report(100, "Catalog ready");
    return !_vendors.empty();
}

std::string Catalog::DownloadTo(const std::string& url,
                                const std::string& destFolder,
                                const std::string& fileName) {
    if (url.empty() || destFolder.empty() || fileName.empty()) return "";
    std::string dest = destFolder;
    if (dest.back() != '/' && dest.back() != '\\') dest += "/";
    dest += fileName;
    if (FileExists(dest)) return dest;
    if (!FileCacheItem::DownloadURL(url, dest)) return "";
    if (!FileExists(dest)) return "";
    return dest;
}

Catalog::~Catalog() {
    for (auto* v : _vendors) delete v;
}

} // namespace music_catalog
