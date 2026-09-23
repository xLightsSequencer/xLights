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
#include <regex>
#include <string>
#include <utility>
#include <vector>

// Shared matching for the filter/search boxes (model trees, controller lists,
// the visualiser, import mapping, pickers...). Every rule here is additive:
// anything a plain substring filter used to find is still found.
//
//   words in any order   "all house"   matches  grp_all_house_display
//   separators optional  "allhouse"    matches  grp_all_house_display
//                        "all-house"   matches  grp_all_house_display
//   wildcards            "grp_*_house" matches  grp_all_house_display
//                        "h?use"       matches  grp_all_house_display
//   regex                "/^grp_.*display$/"
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

        // ASCII only: std::tolower on individual UTF-8 bytes under a code-page
        // locale (e.g. Windows 1252) rewrites lead bytes. Non-ASCII folding is
        // the caller's job (wxFilterQuery uses wxString::Lower).
        inline std::string ToLower(const std::string& s)
        {
            std::string out;
            out.reserve(s.size());
            for (char c : s) {
                out += (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
            }
            return out;
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

    // An explicit /pattern/ is a regular expression; anything else is words and
    // wildcards, so a name full of regex punctuation is not treated as one.
    inline bool AsRegexPattern(const std::string& query, std::string& pattern)
    {
        if (query.size() < 2 || query.front() != '/' || query.back() != '/') {
            return false;
        }
        pattern = query.substr(1, query.size() - 2);
        return !pattern.empty();
    }

    class FilterQuery
    {
    public:
        FilterQuery() = default;

        explicit FilterQuery(const std::string& query)
        {
            std::string pattern;
            if (AsRegexPattern(query, pattern)) {
                try {
                    _regex = std::regex(pattern, std::regex::ECMAScript | std::regex::icase);
                    _isRegex = true;
                    return;
                } catch (const std::regex_error&) {
                    // Half-typed pattern: fall through to plain matching so the
                    // list does not blank out on every keystroke.
                }
            }

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
            return !_isRegex && _tokens.empty();
        }

        // The caller case-folds the subject for the token path; a pattern and
        // its subject must be left alone.
        [[nodiscard]] bool IsRegex() const
        {
            return _isRegex;
        }

        [[nodiscard]] bool Matches(const std::string& text) const
        {
            if (_isRegex) {
                return std::regex_search(text, _regex);
            }
            if (_tokens.empty()) {
                return true;
            }

            std::string const lowered = filter_detail::ToLower(text);
            // Only paid for when a token actually needs it.
            bool haveSquashed = false;
            std::string squashed;

            for (auto const& token : _tokens) {
                if (!token.hasWildcard && lowered.find(token.pattern) != std::string::npos) {
                    continue; // what a plain substring filter would have matched
                }
                if (!haveSquashed) {
                    squashed = filter_detail::WithoutSeparators(lowered);
                    haveSquashed = true;
                }
                if (!MatchesToken(token, lowered, squashed)) {
                    return false;
                }
            }
            return true;
        }

    private:
        struct Token
        {
            std::string pattern;
            // pattern with separators removed, so "all-house" and "allhouse"
            // both find all_house. Empty for a token made only of separators
            // (e.g. the leading "/" of a regex being typed), which then
            // constrains nothing rather than blanking the list.
            std::string squashed;
            bool hasWildcard = false;
        };

        void AddToken(const std::string& pattern)
        {
            Token token;
            token.pattern = pattern;
            token.squashed = filter_detail::WithoutSeparators(pattern);
            token.hasWildcard = pattern.find('*') != std::string::npos ||
                                pattern.find('?') != std::string::npos;
            if (token.hasWildcard) {
                // Anchored at neither end, so "grp_*_house" need not be the whole name.
                token.pattern = "*" + token.pattern + "*";
                token.squashed = "*" + token.squashed + "*";
            }
            _tokens.push_back(std::move(token));
        }

        static bool MatchesToken(const Token& token,
                                 const std::string& lowered,
                                 const std::string& squashed)
        {
            if (token.hasWildcard) {
                return filter_detail::GlobMatch(token.pattern, lowered) ||
                       filter_detail::GlobMatch(token.squashed, squashed);
            }
            return squashed.find(token.squashed) != std::string::npos;
        }

        std::vector<Token> _tokens;
        std::regex _regex;
        bool _isRegex = false;
    };

    // One-off convenience. Prefer building a FilterQuery when filtering a list.
    [[nodiscard]] inline bool MatchesFilter(const std::string& text, const std::string& query)
    {
        return FilterQuery(query).Matches(text);
    }
}
