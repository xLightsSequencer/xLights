#pragma once
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

#include <wx/string.h>

extern const std::string xlEMPTY_STRING;
extern const wxString xlEMPTY_WXSTRING;

//namespace string_utils
//{
    inline bool isHexChar(char c)
    {
        return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
    }

    inline char HexToChar(char c)
    {
        if (c >= '0' && c <= '9')
        {
            return (int)c - 0x30;
        }
        if (c >= 'A' && c <= 'F')
        {
            return (int)c - 65 + 10;
        }
        return (int)c - 97 + 10;
    }

    inline int CountChar(const std::string& s, char c) {
        return std::count(begin(s), end(s), c);
    }

    inline size_t CountStrings(const std::string& what, const std::string& in)
    {
        size_t count = 0;
        size_t pos = 0;
        while ((pos = in.find(what, pos)) != std::string::npos) {
            count++;
            pos += what.size();
        }
        return count;
    }

    inline char HexToChar(char c1, char c2) {
        return (HexToChar(c1) << 4) + HexToChar(c2);
    }

    inline std::string PadLeft(const std::string& str, char with, size_t sz)
    {
        auto res = str;
        while (res.size() < sz) res = with + res;
        return res;
    }

    inline bool Contains(const std::string& in, const std::string& contains) noexcept {
        return in.find(contains) != std::string::npos;
    }

    inline bool ContainsBetweenCommas(const std::string& in, const std::string& contains) noexcept
    {
        size_t idx = 0;
        while (idx < in.length()) {
            size_t nidx = in.find(',', idx);
            if (nidx == std::string::npos) {
                nidx = in.length();
            }
            if (nidx - idx == contains.length() &&
                in.substr(idx, nidx - idx) == contains)
            {
                return true;
            }
            idx = nidx + 1;
        }
        return false;
    }

    inline bool Contains(const std::wstring& in, const std::wstring& contains) noexcept
    {
        return in.find(contains) != std::wstring::npos;
    }

    inline std::string StripAllBut(const std::string& in, const std::string& but)
    {
        std::string res;

        for (const auto c : in) {
            if (but.find(c) != std::string::npos) {
                res += c;
            }
        }

        return res;
    }

    inline bool StartsWith(const std::string& in, const std::string& startswith) noexcept
    {
        return in.size() >= startswith.size() && in.substr(0, startswith.size()) == startswith;
    }

    inline std::string AfterFirst(const std::string& in, char c)
    {
        auto pos = in.find(c);
        if (pos == std::string::npos) return "";
        return in.substr(pos+1);
    }

    inline std::string BeforeFirst(const std::string& in, char c)
    {
        auto pos = in.find(c);
        if (pos == std::string::npos) return in;
        return in.substr(0, pos);
    }

    inline bool EndsWith(const std::string& in, const std::string& endswith) noexcept
    {
        return in.size() >= endswith.size() && in.substr(in.size() - endswith.size()) == endswith;
    }

    inline std::string AfterLast(const std::string& in, char c)
    {
        auto pos = in.find_last_of(c);
        if (pos == std::string::npos) return "";
        return in.substr(pos+1);
    }

    inline std::string BeforeLast(const std::string& in, char c)
    {
        auto pos = in.find_last_of(c);
        if (pos == std::string::npos) return "";
        return in.substr(0, pos);
    }

    inline void Replace(std::string& in, const std::string& what, const std::string& with)
    {
        auto pos = in.find(what);

        // Repeat till end is reached
        while (pos != std::string::npos)
        {
            // Replace this occurrence of Sub String
            in.replace(pos, what.size(), with);
            // Get the next occurrence from the current position
            pos = in.find(what, pos + with.size());
        }
    }

    inline void Replace(std::wstring& in, const std::wstring& what, const std::wstring& with)
    {
        auto pos = in.find(what);

        // Repeat till end is reached
        while (pos != std::wstring::npos)
        {
            // Replace this occurrence of Sub String
            in.replace(pos, what.size(), with);
            // Get the next occurrence from the current position
            pos = in.find(what, pos + with.size());
        }
    }

    inline std::string Capitalise(const std::string& input) noexcept
    {
        std::string res = "";
        char last = ' ';
        for (const auto c : input)
        {
            if (last == ' ' || last == '\t' || last == '\n' || last == '\r')
            {
                res += ::toupper(c);
            }
            else
            {
                res += ::tolower(c);
            }

            last = c;
        }
        return res;
    }

    inline std::string Lower(const std::string& input) noexcept
    {
        std::string res = "";
        for (const auto c : input)
        {
            res += ::tolower(c);
        }
        return res;
    }

    inline std::string Trim(const std::string& input)
    {
        if (input.empty()) return "";

        size_t firstnonblank = 0;
        int lastnonblank = input.size()-1;

        while (firstnonblank < input.size() && (input[firstnonblank] == ' ' || input[firstnonblank] == '\t')) { firstnonblank++; }
        while (lastnonblank > 0 && (input[lastnonblank] == ' ' || input[lastnonblank] == '\t')) { --lastnonblank; }
        if (lastnonblank < firstnonblank) return "";
        return input.substr(firstnonblank, lastnonblank - firstnonblank + 1);
    }
    inline void Split(const std::string &frag, char splitBy, std::vector<std::string>& tokens, bool trim = false)
    {
        // Loop infinitely - break is internal.
        size_t lastIdx = 0;
        while (true) {
            size_t splitAt = frag.find_first_of(splitBy, lastIdx);
            // If we didn't find a new split point...
            if (splitAt == std::string::npos) {
                std::string f = frag.substr(lastIdx);
                tokens.push_back(trim ? Trim(f) : f);
                break;
            }
            std::string newf = frag.substr(lastIdx, splitAt - lastIdx);
            if (trim) {
                newf = Trim(newf);
            }
            tokens.push_back(newf);
            lastIdx = splitAt + 1;
        }
    }
    inline std::vector<std::string> Split(const std::string &frag, char splitBy, bool trim = false) {
        std::vector<std::string> r;
        Split(frag, splitBy, r, trim);
        return r;
    }

    inline std::string Join(std::vector<std::string> const &strings, std::string delim)
    {
        if (strings.empty()) {
            return std::string();
        }
     
        return std::accumulate(strings.begin() + 1, strings.end(), strings[0],
            [&delim](std::string x, std::string y) {
                return x + delim + y;
            }
        );
    }

//};
