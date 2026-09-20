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

#include <cctype>
#include <string>
#include <vector>

// Shared matching for the filter/search boxes (model trees, controller lists,
// the visualiser, import mapping, pickers...). Every rule here is additive:
// anything a plain substring filter used to find is still found.
//
//   words in any order   "all house"   matches  grp_all_house_display
//   separators optional  "allhouse"    matches  grp_all_house_display
//   wildcards            "grp_*_house" matches  grp_all_house_display
//                        "h?use"       matches  grp_all_house_display
//
// Build a FilterQuery once per keystroke and reuse it for every row -- parsing
// it per row is what makes a filter feel slow on a large show.
namespace xl
{
    namespace filter_detail
    {
        inline bool IsSeparator(char c)
        {
            return c == '_' || c == '-' || c == ' ' || c == '/' || c == '\\' ||
                   c == '.' || c == ',' || c == ':' || c == ';' || c == '+' ||
                   c == '(' || c == ')' || c == '[' || c == ']';
        }

        inline std::string ToLower(const std::string& s)
        {
            std::string out;
            out.reserve(s.size());
            for (char c : s) {
                out += (char)std::tolower((unsigned char)c);
            }
            return out;
        }

        // "grp_all_house_display" -> { "grp", "all", "house", "display" }
        inline std::vector<std::string> Segments(const std::string& lowered)
        {
            std::vector<std::string> segments;
            std::string current;
            for (char c : lowered) {
                if (IsSeparator(c)) {
                    if (!current.empty()) {
                        segments.push_back(current);
                        current.clear();
                    }
                } else {
                    current += c;
                }
            }
            if (!current.empty()) {
                segments.push_back(current);
            }
            return segments;
        }

        inline std::string WithoutSeparators(const std::string& lowered)
        {
            std::string out;
            out.reserve(lowered.size());
            for (char c : lowered) {
                if (!IsSeparator(c)) {
                    out += c;
                }
            }
            return out;
        }

        // '*' any run (separators included), '?' exactly one character.
        inline bool GlobMatch(const std::string& pattern, const std::string& text)
        {
            size_t p = 0;
            size_t t = 0;
            size_t starP = std::string::npos;
            size_t starT = 0;
            while (t < text.size()) {
                if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == text[t])) {
                    ++p;
                    ++t;
                } else if (p < pattern.size() && pattern[p] == '*') {
                    starP = p++;
                    starT = t;
                } else if (starP != std::string::npos) {
                    // Backtrack: let the last '*' swallow one more character.
                    p = starP + 1;
                    t = ++starT;
                } else {
                    return false;
                }
            }
            while (p < pattern.size() && pattern[p] == '*') {
                ++p;
            }
            return p == pattern.size();
        }
    }

    class FilterQuery
    {
    public:
        FilterQuery() = default;

        explicit FilterQuery(const std::string& query)
        {
            std::string const lowered = filter_detail::ToLower(query);
            std::string current;
            for (char c : lowered) {
                // Only whitespace separates tokens; a user typing "grp_all" means
                // one token, separators and all.
                if (std::isspace((unsigned char)c)) {
                    if (!current.empty()) {
                        AddToken(current);
                        current.clear();
                    }
                } else {
                    current += c;
                }
            }
            if (!current.empty()) {
                AddToken(current);
            }
        }

        [[nodiscard]] bool IsEmpty() const
        {
            return _tokens.empty();
        }

        [[nodiscard]] bool Matches(const std::string& text) const
        {
            if (_tokens.empty()) {
                return true;
            }

            std::string const lowered = filter_detail::ToLower(text);
            // Only paid for when a token actually needs them.
            bool haveExtras = false;
            std::vector<std::string> segments;
            std::string squashed;

            for (auto const& token : _tokens) {
                if (!token.hasWildcard && lowered.find(token.pattern) != std::string::npos) {
                    continue; // what a plain substring filter would have matched
                }
                if (!haveExtras) {
                    segments = filter_detail::Segments(lowered);
                    squashed = filter_detail::WithoutSeparators(lowered);
                    haveExtras = true;
                }
                if (!MatchesToken(token, lowered, segments, squashed)) {
                    return false;
                }
            }
            return true;
        }

    private:
        struct Token
        {
            std::string pattern;
            bool hasWildcard = false;
        };

        void AddToken(const std::string& pattern)
        {
            Token token;
            token.pattern = pattern;
            token.hasWildcard = pattern.find('*') != std::string::npos ||
                                pattern.find('?') != std::string::npos;
            _tokens.push_back(token);
        }

        static bool MatchesToken(const Token& token,
                                 const std::string& lowered,
                                 const std::vector<std::string>& segments,
                                 const std::string& squashed)
        {
            if (token.hasWildcard) {
                // Anchored at neither end, so "grp_*_house" need not be the whole name.
                if (filter_detail::GlobMatch("*" + token.pattern + "*", lowered)) {
                    return true;
                }
                for (auto const& segment : segments) {
                    if (filter_detail::GlobMatch(token.pattern, segment)) {
                        return true;
                    }
                }
                return false;
            }

            for (auto const& segment : segments) {
                if (segment.find(token.pattern) != std::string::npos) {
                    return true;
                }
            }
            // "allhouse" for all_house: the user left the separators out.
            return squashed.find(token.pattern) != std::string::npos;
        }

        std::vector<Token> _tokens;
    };

    // One-off convenience. Prefer building a FilterQuery when filtering a list.
    [[nodiscard]] inline bool MatchesFilter(const std::string& text, const std::string& query)
    {
        return FilterQuery(query).Matches(text);
    }
}
