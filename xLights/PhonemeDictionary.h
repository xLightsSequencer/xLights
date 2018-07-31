#ifndef PHONEMEDICTIONARY_H
#define PHONEMEDICTIONARY_H

#include <vector>
#include <map>
#include <wx/string.h>
#include <wx/arrstr.h>

class PhonemeDictionary
{
    public:
        PhonemeDictionary();
        virtual ~PhonemeDictionary();

        void LoadDictionaries(const wxString &showDir);
        void LoadDictionary(const wxString &filename, const wxString &showDir);
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

#endif // PHONEMEDICTIONARY_H
