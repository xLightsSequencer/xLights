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

#include <vector>
#include <map>
#include <wx/string.h>
#include <wx/arrstr.h>

class wxWindow;

class PhonemeDictionary
{
    public:
        PhonemeDictionary() {}
        virtual ~PhonemeDictionary() {}

        void LoadDictionaries(const wxString &showDir, wxWindow* parent);
        void LoadDictionary(const wxString &filename, const wxString &showDir, wxWindow* parent, wxFontEncoding defEnc = wxFONTENCODING_UTF8);
        void BreakdownWord(const wxString& text, wxArrayString& phonemes);
        void InsertSpacesAfterPunctuation(wxString& text);
        void InsertPhoneme(const wxArrayString& phonemes);
        void RemovePhoneme(const wxString& text);
        bool ContainsPhoneme(const wxString& text) { return phoneme_dict.count(text); }
        bool ContainsPhonemeMap(const wxString& text) { return phoneme_map.count(text); }
        wxArrayString GetPhonemeList();
        wxArrayString GetPhoneme(const wxString& word) { return phoneme_dict[word.Upper()]; }

    protected:
    private:
        std::vector<wxString> phonemes;
        std::map<wxString, wxString> phoneme_map;
        std::map<wxString, wxArrayString> phoneme_dict;
};
