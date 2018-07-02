#include "PhonemeDictionary.h"
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/msgdlg.h>

PhonemeDictionary::PhonemeDictionary()
{

}

PhonemeDictionary::~PhonemeDictionary()
{

}

void PhonemeDictionary::LoadDictionaries(const wxString &showDir)
{
	if (phoneme_dict.size() > 0)
		return;

    LoadDictionary("user_dictionary", showDir);
    LoadDictionary("standard_dictionary", showDir);
    LoadDictionary("extended_dictionary", showDir);

    wxFileName phonemeFile = wxFileName::FileName(wxStandardPaths::Get().GetExecutablePath());
    phonemeFile.SetFullName("phoneme_mapping");
    if (!wxFile::Exists(phonemeFile.GetFullPath())) {
        phonemeFile = wxFileName(wxStandardPaths::Get().GetResourcesDir(), "phoneme_mapping");
    }
    if (!wxFile::Exists(phonemeFile.GetFullPath()))
    {
        wxMessageBox("Failed to open Phoneme Mapping file!");
        return;
    }

    wxFileInputStream input(phonemeFile.GetFullPath());
    wxTextInputStream text(input);

	while(input.IsOk() && !input.Eof())
    {
        wxString line = text.ReadLine();
        line = line.Trim();
        if (line.Left(1) == "#" || line.Length() == 0)
            continue; // skip comments

		wxArrayString strList = wxSplit(line,' ');
        if (strList.size() > 1)
        {
            if (strList[0] == ".")
                phonemes.push_back(strList[1]);
            else
				phoneme_map.insert( std::pair<wxString, wxString>(strList[0], strList[1]));
        }
    }
}

void PhonemeDictionary::LoadDictionary(const wxString &filename, const wxString &showDir)
{
    wxFileName phonemeFile = wxFileName::DirName(showDir);
    phonemeFile.SetFullName(filename);
    if (!wxFile::Exists(phonemeFile.GetFullPath())) {
        phonemeFile = wxFileName::FileName(wxStandardPaths::Get().GetExecutablePath());
        phonemeFile.SetFullName(filename);
    }
    if (!wxFile::Exists(phonemeFile.GetFullPath())) {
        phonemeFile = wxFileName(wxStandardPaths::Get().GetResourcesDir(), filename);
    }
    if (!wxFile::Exists(phonemeFile.GetFullPath()))
    {
        wxMessageBox("Failed to open Phoneme dictionary!");
        return;
    }

    wxFileInputStream input(phonemeFile.GetFullPath());
    wxTextInputStream text(input);

	while(input.IsOk() && !input.Eof())
	{
		wxString line = text.ReadLine();
		line = line.Trim();
		if (line.Left(1) == "#" || line.Length() == 0)
			continue; // skip comments

		wxArrayString strList = wxSplit(line,' ');
		if (strList.size() > 1)
		{
			if (!phoneme_dict.count(strList[0]))
				phoneme_dict.insert( std::pair<wxString, wxArrayString>(strList[0], strList));
		}
	}
}

void PhonemeDictionary::BreakdownWord(const wxString& text, wxArrayString& phonemes)
{
    wxString word = text;
    word.Replace("[", "");
    word.Replace(".", "");
    word.Replace(",", "");
    word.Replace("!", "");
    word.Replace("?", "");
    word.Replace(";", "");
    word.Replace("-", "");
    word.Replace("/", "");
    word.Replace("(", "");
    word.Replace(")", "");
    word.Replace("]", "");
    phonemes.Clear();
	wxArrayString pronunciation;
    pronunciation = phoneme_dict[word.Upper()];
    if (pronunciation.size() > 1)
    {
        for (int i = 1; i < pronunciation.size(); i++)
        {
            wxString p = pronunciation[i];
            if (p.length() == 0)
                continue;
            if (phoneme_map.count(p))
            {
                bool skip = false;
                if( phoneme_map[p] == "etc" )
                {
                    if( phonemes.Count() > 0 )
                    {
                        if( phonemes.Last() == "etc" )
                        {
                            skip = true;
                        }
                    }
                }
                if( !skip )
                {
                    phonemes.push_back(phoneme_map[p]);
                }
            }
            else
            {
                bool skip = false;
                if( phonemes.Count() > 0 )
                {
                    if( phonemes.Last() == "etc" )
                    {
                        skip = true;
                    }
                }
                if( !skip )
                {
                    phonemes.push_back(phoneme_map[p]);
                }
            }
        }
    }
}

void PhonemeDictionary::InsertSpacesAfterPunctuation(wxString& text)
{
    // make sure there is a space after all punctuation marks
    wxString punctuation = ".,!?;";
    bool repeatLoop = true;
    while (repeatLoop)
    {
        repeatLoop = false;
        int n = text.Length();
        for (int i = 0; i < n - 1; i++)
        {
            if (punctuation.Contains(text[i]) && text[i + 1] != ' ')
            {
                text.insert(i + 1,	' ');
                repeatLoop = true;
                break;
            }
        }
    }
}

void PhonemeDictionary::InsertPhoneme(const wxArrayString& phonemes)
{
    if (!phoneme_dict.count(phonemes[0]))
    {
        phoneme_dict.insert(std::pair<wxString, wxArrayString>(phonemes[0], phonemes));
    }
}

std::vector<wxString> PhonemeDictionary::GetPhonemeList()
{
    std::vector<wxString> keys;
    std::transform(std::begin(phoneme_dict), std::end(phoneme_dict), std::back_inserter(keys),
        [](auto const& val) { return wxString(val.first); });
    return keys;
}