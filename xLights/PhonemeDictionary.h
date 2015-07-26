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

        void LoadDictionaries();
        void LoadDictionary(wxString filename);
        void BreakdownWord(const wxString& text, wxArrayString& phonemes);
        void InsertSpacesAfterPunctuation(wxString& text);

    protected:
    private:
        std::vector<wxString> phonemes;
        std::map<wxString, wxString> phoneme_map;
        std::map<wxString, wxArrayString> phoneme_dict;
};

#endif // PHONEMEDICTIONARY_H
