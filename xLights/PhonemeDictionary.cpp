/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/progdlg.h>

#include "PhonemeDictionary.h"

#include <log4cpp/Category.hh>
#include "UtilFunctions.h"

void PhonemeDictionary::LoadDictionaries(const wxString& showDir, wxWindow* parent)
{
    if (phoneme_dict.size() > 0)
        return;

    LoadDictionary("user_dictionary", showDir, parent);
    LoadDictionary("standard_dictionary", showDir, parent, wxFONTENCODING_ISO8859_1);
    LoadDictionary("extended_dictionary", showDir, parent, wxFONTENCODING_ISO8859_1);

    wxFileName phonemeFile = wxFileName::FileName(wxStandardPaths::Get().GetExecutablePath());
    phonemeFile.SetFullName("phoneme_mapping");
    if (!wxFile::Exists(phonemeFile.GetFullPath())) {
        phonemeFile = wxFileName(wxStandardPaths::Get().GetResourcesDir(), "phoneme_mapping");
    }
    if (!wxFile::Exists(phonemeFile.GetFullPath())) {
        DisplayError("Failed to open Phoneme Mapping file!");
        return;
    }

    wxFileInputStream input(phonemeFile.GetFullPath());
    wxTextInputStream text(input, " \t", wxConvAuto(wxFONTENCODING_UTF8));

    while (input.IsOk() && !input.Eof()) {
        wxString line = text.ReadLine();
        line = line.Trim();
        if (line.Length() == 0 || line.Left(1) == "#" || line.Left(2) == ";;")
            continue; // skip comments

        wxArrayString strList = wxSplit(line, ' ');
        if (strList.size() > 1) {
            if (strList[0] == ".")
                phonemes.push_back(strList[1]);
            else
                phoneme_map.insert(std::pair<wxString, wxString>(strList[0], strList[1]));
        }
    }
}

void PhonemeDictionary::LoadDictionary(const wxString &filename, const wxString &showDir, wxWindow* parent, wxFontEncoding defEnc)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // start looking for dictionary in the show folder
    wxFileName phonemeFile = wxFileName::DirName(showDir);
    phonemeFile.SetFullName(filename);

    // if not there then look were the exe is
    if (!wxFile::Exists(phonemeFile.GetFullPath())) {
        phonemeFile = wxFileName::FileName(wxStandardPaths::Get().GetExecutablePath());
        phonemeFile.SetFullName(filename);
    }

    // if not there look in the resources location (OSX/Linux keeps it there)
    if (!wxFile::Exists(phonemeFile.GetFullPath())) {
        phonemeFile = wxFileName(wxStandardPaths::Get().GetResourcesDir(), filename);
    }

    if (!wxFile::Exists(phonemeFile.GetFullPath())) {
        logger_base.warn("Failed to open phoneme dictionary. '%s'", (const char *)filename.c_str());
        DisplayError("Failed to open Phoneme dictionary!");
        return;
    }

    logger_base.debug("Loading phoneme dictionary. '%s'", (const char *)phonemeFile.GetFullPath().c_str());

    wxProgressDialog dlg("Loading", "Loading dictionary " + phonemeFile.GetName(), 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE);

    wxFileInputStream input(phonemeFile.GetFullPath());
    wxTextInputStream text(input, " \t", wxConvAuto(defEnc));

    auto size = input.GetSize();
    long linenum = 0;
    while(input.IsOk() && !input.Eof()) {
		wxString line = text.ReadLine();
		line = line.Trim();
        if (line.Length() == 0 || line.Left(2) == "##" || line.Left(2) == ";;")
			continue; // skip comments

		wxArrayString strList = wxSplit(line,' ');
		if (strList.size() > 1) {
			//if (phoneme_dict.find(strList[0]) == phoneme_dict.end())
			//	phoneme_dict.emplace(std::pair<wxString, wxArrayString>(strList[0], strList));
            phoneme_dict[strList[0]] = strList;
		}
        linenum++;
        if (linenum % 1000 == 0)
        {
            dlg.Update(input.TellI() * 100 / size);
        }
    }
    dlg.Update(100);
}

void PhonemeDictionary::BreakdownWord(const wxString& text, wxArrayString& phonemes)
{
    wxString word = text;
    word.Replace("/", "");
    word.Replace("#", "");
    word.Replace("~", "");
    word.Replace("@", "");
    word.Replace("$", "");
    word.Replace("%", "");
    word.Replace("^", "");
    word.Replace("*", "");
    word.Replace(",", "");
    word.Replace("!", "");
    word.Replace("&", "");
    word.Replace("-", "");
    word.Replace("_", "");
    word.Replace("+", "");
    word.Replace("=", "");
    word.Replace("[", "");
    word.Replace("]", "");
    word.Replace("{", "");
    word.Replace("}", "");
    word.Replace("\"", "");
    word.Replace(":", "");
    word.Replace(";", "");
    word.Replace(".", "");
    word.Replace("<", "");
    word.Replace(">", "");
    word.Replace("/", "");
    word.Replace("?", "");
    word.Replace("`", "");
    word.Replace("\t", " ");

    phonemes.Clear();

    if (!phoneme_dict.count(word.Upper())) return;

    wxArrayString pronunciation = phoneme_dict.at(word.Upper());
    if (pronunciation.size() > 1) {
        for (int i = 1; i < pronunciation.size(); i++) {

            wxString p = pronunciation[i];
            if (p.length() == 0) continue;
            
            if (phoneme_map.count(p)) {
                bool skip = false;
                if (phoneme_map[p] == "etc") {
                    if (phonemes.Count() > 0) {
                        if (phonemes.Last() == "etc") {
                            skip = true;
                        }
                    }
                }
                if (!skip) {
                    phonemes.push_back(phoneme_map[p]);
                }
            }
            else {
                bool skip = false;
                if (phonemes.Count() > 0) {
                    if (phonemes.Last() == "etc") {
                        skip = true;
                    }
                }
                if (!skip) {
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
    if (phoneme_dict.count(phonemes[0]))
    {
        phoneme_dict.erase(phonemes[0]);
    }
    phoneme_dict.insert(std::pair<wxString, wxArrayString>(phonemes[0], phonemes));
}

void PhonemeDictionary::RemovePhoneme(const wxString & text)
{
    phoneme_dict.erase(text);
}

wxArrayString PhonemeDictionary::GetPhonemeList()
{
    wxArrayString keys;
    std::transform(std::begin(phoneme_dict), std::end(phoneme_dict), std::back_inserter(keys),
        [](auto const& val) { return wxString(val.first); });
    return keys;
}
