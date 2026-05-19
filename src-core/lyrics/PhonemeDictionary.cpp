/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PhonemeDictionary.h"

#include <filesystem>
#include <fstream>

#include <log.h>

#include "../utils/AppCallbacks.h"
#include "../utils/ExternalHooks.h"
#include "../utils/string_utils.h"

namespace {

std::string FindFile(const std::string& filename, const std::vector<std::string>& searchDirs)
{
    for (const auto& dir : searchDirs) {
        if (dir.empty()) {
            continue;
        }
        std::filesystem::path p = std::filesystem::path(dir) / filename;
        std::string full = p.string();
        if (FileExists(full)) {
            return full;
        }
    }
    return {};
}

void StripTrailingCR(std::string& line)
{
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
        line.pop_back();
    }
}

std::vector<std::string> SplitWhitespace(const std::string& line)
{
    std::vector<std::string> out;
    size_t i = 0;
    while (i < line.size()) {
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) {
            ++i;
        }
        size_t start = i;
        while (i < line.size() && line[i] != ' ' && line[i] != '\t') {
            ++i;
        }
        if (i > start) {
            out.emplace_back(line.substr(start, i - start));
        }
    }
    return out;
}

} // namespace

void PhonemeDictionary::LoadDictionaries(const std::vector<std::string>& searchDirs,
                                         const ProgressCallback& progress)
{
    if (!phoneme_dict.empty()) {
        return;
    }

    LoadDictionary("standard_dictionary", searchDirs, progress);
    LoadDictionary("extended_dictionary", searchDirs, progress);
    LoadDictionary("user_dictionary", searchDirs, progress); // load user last so it overrides the other dictionaries

    std::string mappingPath = FindFile("phoneme_mapping", searchDirs);
    if (mappingPath.empty()) {
        DisplayError("Failed to open Phoneme Mapping file!");
        return;
    }

    std::ifstream input(mappingPath);
    std::string line;
    while (std::getline(input, line)) {
        StripTrailingCR(line);
        line = Trim(line);
        if (line.empty() || StartsWith(line, "#") || StartsWith(line, ";;")) {
            continue;
        }
        auto strList = SplitWhitespace(line);
        if (strList.size() > 1) {
            if (strList[0] == ".") {
                phonemes.push_back(strList[1]);
            } else {
                phoneme_map.emplace(strList[0], strList[1]);
            }
        }
    }
}

void PhonemeDictionary::LoadDictionary(const std::string& filename,
                                       const std::vector<std::string>& searchDirs,
                                       const ProgressCallback& progress)
{
    std::string path = FindFile(filename, searchDirs);
    if (path.empty()) {
        spdlog::warn("Failed to open phoneme dictionary. '{}'", filename);
        DisplayError("Failed to open Phoneme dictionary!");
        return;
    }

    spdlog::debug("Loading phoneme dictionary. '{}'", path);

    std::error_code ec;
    auto totalSize = std::filesystem::file_size(path, ec);
    if (ec) {
        totalSize = 0;
    }

    std::ifstream input(path);
    std::string line;
    long linenum = 0;
    while (std::getline(input, line)) {
        StripTrailingCR(line);
        line = Trim(line);
        if (line.empty() || StartsWith(line, "##") || StartsWith(line, ";;")) {
            continue;
        }
        auto strList = SplitWhitespace(line);
        if (strList.size() > 1) {
            phoneme_dict[strList[0]] = strList;
        }
        ++linenum;
        if (progress && totalSize > 0 && (linenum % 1000) == 0) {
            auto pos = input.tellg();
            if (pos != std::streampos(-1)) {
                int pct = static_cast<int>(static_cast<uint64_t>(pos) * 100 / totalSize);
                if (pct < 0) pct = 0;
                if (pct > 100) pct = 100;
                progress(pct);
            }
        }
    }
    if (progress) {
        progress(100);
    }
}

void PhonemeDictionary::BreakdownWord(const std::string& text, std::vector<std::string>& phonemes) const
{
    static const std::string strip = "/#~@$%^*,!&-_+=[]{}\":;.<>?`";
    std::string word;
    word.reserve(text.size());
    for (char c : text) {
        if (c == '\t') {
            word.push_back(' ');
        } else if (strip.find(c) == std::string::npos) {
            word.push_back(c);
        }
    }

    phonemes.clear();

    std::string upper = Upper(word);
    auto it = phoneme_dict.find(upper);
    if (it == phoneme_dict.end()) {
        return;
    }

    const auto& pronunciation = it->second;
    if (pronunciation.size() <= 1) {
        return;
    }

    for (size_t i = 1; i < pronunciation.size(); ++i) {
        const std::string& p = pronunciation[i];
        if (p.empty()) {
            continue;
        }

        auto mapIt = phoneme_map.find(p);
        if (mapIt != phoneme_map.end()) {
            if (mapIt->second == "etc" && !phonemes.empty() && phonemes.back() == "etc") {
                continue;
            }
            phonemes.push_back(mapIt->second);
        } else {
            // Preserve legacy behavior: if the phoneme has no mapping, push
            // an empty string (phoneme_map[p] on a missing key) — unless the
            // previous emitted phoneme was "etc", in which case skip it.
            if (!phonemes.empty() && phonemes.back() == "etc") {
                continue;
            }
            phonemes.emplace_back();
        }
    }
}

void PhonemeDictionary::InsertSpacesAfterPunctuation(std::string& text)
{
    static const std::string punctuation = ".,!?;";
    bool repeat = true;
    while (repeat) {
        repeat = false;
        size_t n = text.size();
        for (size_t i = 0; i + 1 < n; ++i) {
            if (punctuation.find(text[i]) != std::string::npos && text[i + 1] != ' ') {
                text.insert(i + 1, 1, ' ');
                repeat = true;
                break;
            }
        }
    }
}

void PhonemeDictionary::InsertPhoneme(const std::vector<std::string>& phonemes)
{
    if (phonemes.empty()) {
        return;
    }
    phoneme_dict[phonemes[0]] = phonemes;
}

void PhonemeDictionary::RemovePhoneme(const std::string& text)
{
    phoneme_dict.erase(text);
}

std::vector<std::string> PhonemeDictionary::GetPhonemeList() const
{
    std::vector<std::string> keys;
    keys.reserve(phoneme_dict.size());
    for (const auto& kv : phoneme_dict) {
        keys.push_back(kv.first);
    }
    return keys;
}

std::vector<std::string> PhonemeDictionary::GetPhoneme(const std::string& word) const
{
    auto it = phoneme_dict.find(word);
    if (it == phoneme_dict.end()) {
        return {};
    }
    return it->second;
}
