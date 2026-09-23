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

#include <string>

#include <wx/intl.h>
#include <wx/string.h>

#include "utils/FilterMatch.h"

// wxString front end for the shared matcher (xl::FilterQuery). All the matching
// lives in core; this exists for the two things that need wx: case folding that
// covers non-ASCII, and UTF-8 conversion. Build one per keystroke and reuse it
// for every row.
class wxFilterQuery
{
public:
    wxFilterQuery() = default;

    explicit wxFilterQuery(const wxString& query)
    {
        wxString const trimmed = wxString(query).Trim(true).Trim(false);
        std::string pattern;
        // Case-fold the words-and-wildcards form only: lowering a pattern would
        // break character classes like [A-Z].
        _query = xl::AsRegexPattern(trimmed.utf8_string(), pattern)
                     ? xl::FilterQuery(trimmed.utf8_string())
                     : xl::FilterQuery(trimmed.Lower().utf8_string());
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return _query.IsEmpty();
    }

    [[nodiscard]] bool Matches(const wxString& text) const
    {
        // wxString::Lower folds non-ASCII, which the core matcher deliberately
        // does not; a regex gets the subject untouched.
        return _query.IsRegex() ? _query.Matches(text.utf8_string())
                                : _query.Matches(text.Lower().utf8_string());
    }

    [[nodiscard]] bool Matches(const std::string& text) const
    {
        return Matches(wxString::FromUTF8(text));
    }

    // For the filter boxes, so the syntax is not a secret.
    static wxString Hint()
    {
        return _("Words may be in any order and separators are optional. * and ? are wildcards. /pattern/ is a regular expression.");
    }

private:
    xl::FilterQuery _query;
};
