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

#include <functional>
#include <map>
#include <string>
#include <vector>

class PhonemeDictionary
{
public:
    using ProgressCallback = std::function<void(int pct)>;

    void LoadDictionaries(const std::vector<std::string>& searchDirs,
                          const ProgressCallback& progress = {});
    void LoadDictionary(const std::string& filename,
                        const std::vector<std::string>& searchDirs,
                        const ProgressCallback& progress = {});

    void BreakdownWord(const std::string& text, std::vector<std::string>& phonemes) const;
    static void InsertSpacesAfterPunctuation(std::string& text);

    void InsertPhoneme(const std::vector<std::string>& phonemes);
    void RemovePhoneme(const std::string& text);

    bool ContainsPhoneme(const std::string& text) const { return phoneme_dict.count(text) > 0; }
    bool ContainsPhonemeMap(const std::string& text) const { return phoneme_map.count(text) > 0; }

    std::vector<std::string> GetPhonemeList() const;
    std::vector<std::string> GetPhoneme(const std::string& word) const;

private:
    std::vector<std::string> phonemes;
    std::map<std::string, std::string> phoneme_map;
    std::map<std::string, std::vector<std::string>> phoneme_dict;
};
