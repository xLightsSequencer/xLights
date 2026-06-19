/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MapHintsIO.h"

#include "utils/ExternalHooks.h"
#include "utils/UtilFunctions.h"

#include <pugixml.hpp>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <system_error>

std::vector<MapHintEntry> LoadMapHintsFile(const std::string& path) {
    std::vector<MapHintEntry> entries;

    if (!FileExists(path)) {
        return entries;
    }

    pugi::xml_document doc;
    if (!doc.load_file(path.c_str())) {
        return entries;
    }

    auto root = doc.document_element();
    for (pugi::xml_node n = root.first_child(); n; n = n.next_sibling()) {
        std::string nodeName = n.name();
        std::transform(nodeName.begin(), nodeName.end(), nodeName.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (nodeName != "map") {
            continue;
        }

        std::string toRegex = n.attribute("ToRegex").as_string();
        std::string fromModel = n.attribute("FromModel").as_string();
        std::string applyTo = n.attribute("ApplyTo").as_string("B");
        if (toRegex.empty() || fromModel.empty()) {
            continue;
        }

        entries.push_back({ toRegex, fromModel, applyTo });
    }

    return entries;
}

std::vector<MapHintEntry> LoadMapHintsFromShowDir(const std::string& showDir) {
    std::vector<MapHintEntry> entries;
    if (showDir.empty()) {
        return entries;
    }

    std::filesystem::path hintsDir = std::filesystem::path(showDir) / "maphints";
    std::error_code ec;
    if (!std::filesystem::exists(hintsDir, ec) || ec) {
        return entries;
    }

    for (const auto& dent : std::filesystem::directory_iterator(hintsDir, ec)) {
        if (ec) break;
        if (!dent.is_regular_file(ec)) continue;
        auto p = dent.path();
        if (p.extension() != ".xmaphint") continue;
        auto fileEntries = LoadMapHintsFile(p.string());
        entries.insert(entries.end(), fileEntries.begin(), fileEntries.end());
    }

    return entries;
}

bool WriteMapHintsFile(const std::string& path, const std::vector<MapHintEntry>& entries) {
    std::FILE* f = std::fopen(path.c_str(), "wb");
    if (f == nullptr) {
        return false;
    }

    auto writeStr = [&](const std::string& s) {
        std::fwrite(s.data(), 1, s.size(), f);
    };

    writeStr("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<MapHints>\n");
    for (const auto& e : entries) {
        writeStr("    <Map ToRegex=\"");
        writeStr(e.toRegex);
        writeStr("\" FromModel=\"");
        writeStr(e.fromModel);
        writeStr("\" ApplyTo=\"");
        writeStr(e.applyTo.empty() ? std::string("B") : e.applyTo);
        writeStr("\" />\n");
    }
    writeStr("<!-- Samples\n");
    writeStr("    <Map ToRegex=\"^Star(s?)(\\sGroup)?$\" FromModel=\"Star\" ApplyTo=\"B\" />\n");
    writeStr("    <Map ToRegex=\"(.*Mega|^Pixel)\\sTree$\" FromModel=\"Mega Tree\" ApplyTo=\"M\" />\n");
    writeStr("    <Map ToRegex=\"(^ALL|^Whole)\\sHouse$\" FromModel=\"ALL House\" ApplyTo=\"G\" />\n");
    writeStr("-->\n");
    writeStr("</MapHints>");

    std::fclose(f);
    return true;
}
