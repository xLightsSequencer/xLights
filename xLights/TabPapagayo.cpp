/***************************************************************
 * Name:      TabPapagayo.cpp
 * Purpose:   Code for Application Frame
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-11-03
 * Copyright: Matt Brown ()
 * License:
 **************************************************************/

#include "xLightsMain.h"

#include <wx/tokenzr.h>
#include <wx/msgdlg.h>
#include <wx/regex.h>

//this is more concise but harder to read:
//    std::vector<std::pair<wxString, //voices at outer level
//        std::vector<std::pair<wxString, //phrases
//            std::vector<std::pair<wxString, //words
//                std::vector<std::pair<const Phoneme&, int>> >> >> >> voices; //phonemes and start frame#
typedef struct
{
    wxString name;
    int start_frame, end_frame;
} PhonemeInfo;

typedef struct
{
    wxString name;
    std::vector<PhonemeInfo> phonemes;
} WordInfo;

typedef struct
{
    wxString name;
    std::vector<WordInfo> words;
} PhraseInfo;

typedef struct
{
    wxString name;
    std::vector<PhraseInfo> phrases;
} VoiceInfo;

static std::vector<VoiceInfo> voices; //TODO: move this to xLightsMain.h?

//list of allowed phonemes
static const wxString AllowedPhonemes = _(",AI,E,FV,L,MBP,O,U,WQ,etc,rest,"); //TODO: change to hash<string> if performance is bad

void xLightsFrame::OnButton_pgo_filenameClick(wxCommandEvent& event)
{
     wxString filename = wxFileSelector( "Choose Papagayo File", "", "", "", "Papagayo files (*.pgo)|*.pgo", wxFD_OPEN );
 //  wxString filename = "this5.pgo";
    if (!filename.IsEmpty()) TextCtrl_pgo_filename->SetValue(filename);
    LoadPapagayoFile(filename);
}

void xLightsFrame::OnButton_papagayo_output_sequenceClick(wxCommandEvent& event)
{
   wxString filename = wxFileSelector( "Choose Output xLights Sequence File", "", "", "", "xLights files (*.xml)|*.xml", wxFD_OPEN );

    if (!filename.IsEmpty()) TextCtrl_papagayo_output_filename->SetValue(filename);
}

void xLightsFrame::OnButtonStartPapagayoClick(wxCommandEvent& event)
{
    ButtonStartPapagayo->Enable(false);
    wxString OutputFormat = ChoiceOutputFormat->GetStringSelection();
    TextCtrlConversionStatus->Clear();

    // check inputs
    if (FileNames.IsEmpty()) {
        wxMessageBox(_("Please select one or more sequence files"), _("Error"));
    } else if (OutputFormat.IsEmpty()) {
        wxMessageBox(_("Please select an output format"), _("Error"));
    } else {
        for (size_t i=0; i < FileNames.GetCount(); i++) {
            DoConversion(FileNames[i], OutputFormat);
        }
        TextCtrlConversionStatus->AppendText(_("Finished converting all files\n"));
    }

    ButtonStartPapagayo->Enable(true);

//example code to iterate thru the data:
    wxString debug_msg;
    for (auto voice_it = voices.begin(); voice_it != voices.end(); ++voice_it)
    {
        debug_msg += wxString::Format(_("voice[%d/%d] '%s'"), voice_it - voices.begin(), voices.size(), voice_it->name.c_str());
//        std::vector<PhraseInfo>& phrases = voice_it->phrases;
        for (auto phrase_it = voice_it->phrases.begin(); phrase_it != voice_it->phrases.end(); ++phrase_it)
        {
            debug_msg += wxString::Format(_("  phrase[%d/%d] '%s'"), phrase_it - voice_it->phrases.begin(), voice_it->phrases.size(), phrase_it->name.c_str());
//            std::vector<WordInfo>& words = phrase_it->words;
            for (auto word_it = phrase_it->words.begin(); word_it != phrase_it->words.end(); ++word_it)
            {
                debug_msg += wxString::Format(_("    word[%d/%d] '%s'"), word_it - phrase_it->words.begin(), phrase_it->words.size(), word_it->name.c_str());
//              std::vector<PhonemeInfo>& phonemes = word_it->phonemes;
                for (auto phoneme_it = word_it->phonemes.begin(); phoneme_it != word_it->phonemes.end(); ++phoneme_it)
                {
                    debug_msg += wxString::Format(_("      phoneme[%d/%d] '%s': call routine(start_frame %d, end_frame %d, phoneme '%s')"), phoneme_it - word_it->phonemes.begin(), word_it->phonemes.size(), phoneme_it->name.c_str(), phoneme_it->start_frame, phoneme_it->end_frame, phoneme_it->name);
//                  call routine(phoneme_it->start_frame, phoneme_it->end_frame, phoneme_it->name);
                }
            }
        }
    }
    wxMessageBox(debug_msg, _("Papagayo Debug"));
}


//#define WANT_DEBUG_IMPL
//#define WANT_DEBUG  99
//#include "djdebug.cpp"
 #define debug(level, ...)
 #define debug_more(level, ...)
 #define debug_function(level)

//cut down on verbosity:
//use for fatal errors only
#define retmsg(msg)  \
{ \
    wxMessageBox(msg, _("Papagayo Error")); \
    return; \
}

#define warnmsg(msg)  warnings += msg + "\n"

//TODO: move this into RgbEffects.h; this will force recompile of most .cpp files
static struct
{
    int linenum;
    wxTextFile file;
    wxString linebuf;
} PapagayoFileInfo;

static const wxString& readline(bool first = false)
{
    if (first) PapagayoFileInfo.linenum = 0;
    for (PapagayoFileInfo.linebuf = first? PapagayoFileInfo.file.GetFirstLine(): PapagayoFileInfo.file.GetNextLine(); !PapagayoFileInfo.file.Eof(); PapagayoFileInfo.linebuf = PapagayoFileInfo.file.GetNextLine(), ++PapagayoFileInfo.linenum)
    {
        ++PapagayoFileInfo.linenum;
        std::string::size_type ofs;
        if ((ofs = PapagayoFileInfo.linebuf.find("#")) != std::string::npos) PapagayoFileInfo.linebuf.erase(ofs); //remove comments
        while (!PapagayoFileInfo.linebuf.empty() && (PapagayoFileInfo.linebuf.Last() == '\\')) //line continuation
        {
            PapagayoFileInfo.linebuf.RemoveLast(); //remove trailing "\"
            /*std::*/wxString morebuf = PapagayoFileInfo.file.GetNextLine();
            if (PapagayoFileInfo.file.Eof()) break;
            PapagayoFileInfo.linebuf += morebuf;
            ++PapagayoFileInfo.linenum;
        }
        while (!PapagayoFileInfo.linebuf.empty() && isspace(PapagayoFileInfo.linebuf[0])) PapagayoFileInfo.linebuf = PapagayoFileInfo.linebuf.substr(1); //.RemoveFirst(); //trim leading spaces
        while (!PapagayoFileInfo.linebuf.empty() && isspace(PapagayoFileInfo.linebuf.Last())) PapagayoFileInfo.linebuf.RemoveLast(); //trim trailing spaces
        if (PapagayoFileInfo.linebuf.empty()) continue; //skip blank lines
        debug(20, "got line[%d] '%s'", PapagayoFileInfo.linenum, (const char*)PapagayoFileInfo.linebuf.c_str());
        return PapagayoFileInfo.linebuf;
    }
    return _("");
}

void xLightsFrame::LoadPapagayoFile(const wxString& filename)
{
    wxString warnings;
//    if (!CachedCueFilename.CmpNoCase(filename)) { debug_more(2, ", no change"); return; } //no change
    if (!wxFileExists(filename)) retmsg(_("File does not exist"));
    if (!PapagayoFileInfo.file.Open(filename)) retmsg(_("Can't open file"));
    debug(3, "read file '%s'", (const char*)filename.c_str());

//        wxStringTokenizer tkz(linebuf, "\t");
//        linebuf += "\teol"; //end-of-line check for missing params
//        if ((tkz.GetNextToken().LowerCase() != "ipsync")
//            || (tkz.GetNextToken().LowerCase() != "version")

//    wxRegEx ipsync("^ipsync version 1$");
    if (readline(true).CmpNoCase("lipsync version 1")) retmsg(wxString::Format(_("Invalid file @line %d (header '%s')"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str()));
//    if (ipsync.GetMatch(linebuf, 1) != "1") retmsg(wxString::Format(_("Bad ipsync version: got %s, expected 1"), ))

    wxString soundpath = readline();
    if (soundpath.IsEmpty() || !wxFileExists(soundpath)) warnmsg(wxString::Format(_("Sound file '%s' is missing"), soundpath.c_str()));
//    wxFile soundfile(soundpath);
//    if (!soundfile.Open(filename.c_str())) retmsg(_("Can't open file"));
//    if (ext != ".wav") error?

    wxRegEx number("^[0-9]+$");
    int samppersec = number.Matches(readline())? wxAtoi(PapagayoFileInfo.linebuf): 0;
    if (!samppersec) warnmsg(wxString::Format(_("Invalid file @line %d ('%s' samples per sec)"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str()));

    int numsamp = number.Matches(readline())? wxAtoi(PapagayoFileInfo.linebuf): 0;
    if (!numsamp) warnmsg(wxString::Format(_("Invalid file @line %d ('%s' song samples)"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str()));

    wxString desc;
    int numvoices = number.Matches(readline())? wxAtoi(PapagayoFileInfo.linebuf): 0;
    if (!numvoices) retmsg(wxString::Format(_("Invalid file @line %d ('%s' voices)"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str()));
    for (int v = 1; v <= numvoices; ++v)
    {
        voices.emplace_back();
        VoiceInfo& newvoice = voices.back();
        newvoice.name = readline();
        if (newvoice.name.empty())
        {
            warnmsg(wxString::Format(_("Missing voice# %d of %d"), v, numvoices));
            voices.pop_back(); //get rid of unneeded entry
            break;
        }
        readline(); //all phrases for voice, "|" delimiter; TODO: do we need to save this?
//        voices.push_back(newvoice);
        desc = wxString::Format(_("voice# %d '%s' @line %d"), v, newvoice.name, PapagayoFileInfo.linenum);
        debug(10, (const char*)desc.c_str());

        int numphrases = number.Matches(readline())? wxAtoi(PapagayoFileInfo.linebuf): 0;
        if (!numphrases) retmsg(wxString::Format(_("Invalid file @line %d (%s phrases for %s)"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str(), desc.c_str()));
        for (int phr = 1; phr <= numphrases; ++phr)
        {
            newvoice.phrases.emplace_back();
            PhraseInfo& newphrase = newvoice.phrases.back();
            newphrase.name = readline();
            if (newphrase.name.empty())
            {
                warnmsg(wxString::Format(_("Missing phrase# %d of %d for %s"), phr, numphrases, desc.c_str()));
                newvoice.phrases.pop_back(); //get rid of unneeded entry
                break;
            }
            readline(); //start frame TODO: do we need to save this?
            readline(); //end frame TODO: do we need to save this?
//            voices.back().phrases.push_back(newphrase);
            desc = wxString::Format(_("voice# %d, phrase %d '%s' @line %d"), v, phr, newphrase.name, PapagayoFileInfo.linenum);
            debug(10, (const char*)desc.c_str());

            int numwords = number.Matches(readline())? wxAtoi(PapagayoFileInfo.linebuf): 0;
            if (!numwords) retmsg(wxString::Format(_("Invalid file @line %d (%s words for %s)"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str(), desc.c_str()));
            for (int w = 1; w <= numwords; ++w)
            {
                newphrase.words.emplace_back();
                WordInfo& newword = newphrase.words.back();
                wxStringTokenizer tkz(readline(), " ");
                newword.name = tkz.GetNextToken();
                if (newword.name.empty())
                {
                    warnmsg(wxString::Format(_("Missing word# %d of %d for %s"), w, numwords, desc.c_str()));
                    newphrase.words.pop_back(); //get rid of unneeded entry
                    break;
                }
                tkz.GetNextToken(); //start frame TODO: do we need to save this?
                wxString endfr = tkz.GetNextToken(); //end frame TODO: do we need to save this?
//                voices.back().phrases.push_back(newphrase);
                wxString syllcount = tkz.GetNextToken();
                wxString junk = tkz.GetNextToken();
                desc = wxString::Format(_("voice# %d, phrase %d, word %d '%s' @line %d"), v, phr, w, newword.name, PapagayoFileInfo.linenum);
                if (!junk.empty()) warnmsg(wxString::Format(_("Ignoring junk '%s' at end of %s @line %d"), junk.c_str(), desc.c_str(), PapagayoFileInfo.linenum));
                debug(10, (const char*)desc.c_str());

                int end_frame = number.Matches(endfr)? wxAtoi(endfr): 0;
                if (!end_frame) warnmsg(wxString::Format(_("Invalid file @line %d (%s end frame for %s)"), PapagayoFileInfo.linenum, endfr.c_str(), desc.c_str()));

                int numsylls = number.Matches(syllcount)? wxAtoi(syllcount): 0;
                if (!numsylls) retmsg(wxString::Format(_("Invalid file @line %d (%s phonemes for %s)"), PapagayoFileInfo.linenum, syllcount.c_str(), desc.c_str()));
                for (int syll = 1; syll <= numsylls; ++syll)
                {
                    newword.phonemes.emplace_back();
                    PhonemeInfo& newsyll = newword.phonemes.back();
                    /*wxStringTokenizer*/ tkz = wxStringTokenizer(readline(), " ");
                    wxString stframe = tkz.GetNextToken();
                    newsyll.name = tkz.GetNextToken();
                    if (newsyll.name.empty() || (AllowedPhonemes.find(newsyll.name) == -1))
                    {
                        warnmsg(wxString::Format(_("Missing phoneme# %d of %d for %s"), syll, numsylls, desc.c_str()));
                        newword.phonemes.pop_back(); //get rid of unneeded entry
                        break;
                    }
                    /*wxString*/ junk = tkz.GetNextToken();
                    desc = wxString::Format(_("voice# %d, phrase %d, word %d, phoneme %d '%s' @line %d"), v, phr, w, syll, newsyll.name, PapagayoFileInfo.linenum);
                    if (!junk.empty()) warnmsg(wxString::Format(_("Ignoring junk '%s' at end of %s @line %d"), junk.c_str(), desc.c_str(), PapagayoFileInfo.linenum));
                    debug(10, (const char*)desc.c_str());

                    newsyll.start_frame = number.Matches(stframe)? wxAtoi(stframe): 0;
                    if (!newsyll.start_frame) retmsg(wxString::Format(_("Invalid file @line %d (%s start frame for %s)"), PapagayoFileInfo.linenum, stframe.c_str(), desc.c_str()));
                    newsyll.end_frame = end_frame; //assume end of phrase until another phoneme is found
                    if (syll > 1) (&newsyll)[-1].end_frame = newsyll.start_frame; //don't overlap?
                }
            }
        }
    }

    if (!warnings.empty()) wxMessageBox(warnings, _("Papagayo Warning"));
    debug(3, "file loaded");
}
