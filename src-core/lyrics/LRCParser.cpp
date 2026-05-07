/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LRCParser.h"
#include "PhonemeDictionary.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>

namespace lrc {

std::vector<std::pair<int, std::string>> ParseLRC(const std::string& syncedLyrics) {
    std::vector<std::pair<int, std::string>> result;
    std::istringstream stream(syncedLyrics);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty() || line[0] != '[')
            continue;

        size_t closeBracket = line.find(']');
        if (closeBracket == std::string::npos || closeBracket < 5)
            continue;

        std::string timestamp = line.substr(1, closeBracket - 1);

        // Skip metadata tags like `[ti:]`, `[ar:]`, `[al:]`, `[by:]`.
        // Numeric "mm:ss" timestamps have only digits before the colon.
        size_t colonPos = timestamp.find(':');
        if (colonPos == std::string::npos)
            continue;

        bool isMetadata = false;
        for (size_t i = 0; i < colonPos; ++i) {
            if (!std::isdigit(static_cast<unsigned char>(timestamp[i]))) {
                isMetadata = true;
                break;
            }
        }
        if (isMetadata)
            continue;

        std::string text = (closeBracket + 1 < line.size()) ? line.substr(closeBracket + 1) : "";
        // Trim a single leading space — typical LRC files use `[mm:ss.xx] text`.
        if (!text.empty() && text.front() == ' ')
            text.erase(text.begin());

        size_t dotPos = timestamp.find('.');
        // Use strtol/strtod (no exceptions; CLAUDE.md rule).
        int minutes = static_cast<int>(std::strtol(timestamp.substr(0, colonPos).c_str(), nullptr, 10));
        size_t secondsLen = (dotPos != std::string::npos ? dotPos : timestamp.size()) - colonPos - 1;
        int seconds = static_cast<int>(std::strtol(timestamp.substr(colonPos + 1, secondsLen).c_str(), nullptr, 10));
        int ms = 0;
        if (dotPos != std::string::npos) {
            std::string frac = timestamp.substr(dotPos + 1);
            int fracVal = static_cast<int>(std::strtol(frac.c_str(), nullptr, 10));
            switch (frac.size()) {
            case 1: ms = fracVal * 100; break;   // tenths
            case 2: ms = fracVal * 10;  break;   // centiseconds
            case 3: ms = fracVal;       break;   // milliseconds
            default: ms = 0;            break;
            }
        }
        if (minutes < 0 || seconds < 0 || ms < 0)
            continue;
        int totalMs = minutes * 60000 + seconds * 1000 + ms;
        result.push_back({ totalMs, text });
    }
    return result;
}

namespace {
// Replace each occurrence of `from` (multi-byte UTF-8 sequence) with `to`.
// Plain `std::string::replace`-driven loop — small inputs, doesn't justify
// pulling in a regex engine.
void ReplaceAll(std::string& s, const std::string& from, const std::string& to) {
    if (from.empty()) return;
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
}
} // namespace

bool SanitizePhraseText(std::string& text) {
    // Common Unicode quote-equivalents that the LRCLIB / vendor
    // text often contains. Replace with ASCII so the downstream
    // XML write doesn't have to worry about escapes.
    ReplaceAll(text, "\xE2\x80\x99", "'");  // U+2019 RIGHT SINGLE QUOTATION MARK
    ReplaceAll(text, "\xC8\x98",     "'");  // U+0218 LATIN CAPITAL S WITH COMMA BELOW (LRC mojibake guard)
    ReplaceAll(text, "\xE2\x80\x9C", "\""); // U+201C LEFT DOUBLE QUOTATION MARK
    ReplaceAll(text, "\xE2\x80\x9D", "\""); // U+201D RIGHT DOUBLE QUOTATION MARK

    // Strip out characters that are illegal in phrase labels
    // (XML-attribute-hostile + the renderer's parser doesn't like them).
    text.erase(std::remove(text.begin(), text.end(), '"'), text.end());
    text.erase(std::remove(text.begin(), text.end(), '<'), text.end());
    text.erase(std::remove(text.begin(), text.end(), '>'), text.end());

    // Trim leading + trailing whitespace.
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    auto first = std::find_if(text.begin(), text.end(), notSpace);
    auto last = std::find_if(text.rbegin(), text.rend(), notSpace).base();
    if (first >= last) {
        text.clear();
    } else {
        text = std::string(first, last);
    }

    if (text.empty()) return false;

    PhonemeDictionary::InsertSpacesAfterPunctuation(text);
    return !text.empty();
}

} // namespace lrc
