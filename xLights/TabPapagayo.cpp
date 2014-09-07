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

//#define WANT_DEBUG_IMPL
//#define WANT_DEBUG  99
//#include "djdebug.cpp"
#define debug(level, ...)
#define debug_more(level, ...)
#define debug_function(level)

//#define numents(ary)  (sizeof(ary)/sizeof(ary[0]))


//cut down on verbosity:
//use for fatal errors only
#define retmsg(msg)  \
{ \
    wxMessageBox(msg, _("Papagayo Error")); \
    debug(1, "RETERR: %s", (const char*)msg.c_str()); \
    return; \
}

#define warnmsg(msg)  warnings += msg + "\n"



void xLightsFrame::PapagayoError(const wxString& msg)
{
    debug(1, "ERROR: %s", (const char*)msg.c_str());
    wxMessageBox(msg, _("Error"), wxOK | wxICON_EXCLAMATION);
}


//template<typename T, int InitValue>
//class Initialized
//{
//public:
//    T value;
//    Initialized(void): value(InitValue) {}; //ctor to init value
//    T& T() { return value; }
//};

//coro face values for one column (voice):
//struct CoroVoice
//{
////    Initialized<bool, true> IsEmpty;
//    wxString Outline, EyesOpen, EyesClosed, Phon_AI, Phon_E, Phon_etc, Phon_FV, Phon_L, Phon_MBP, Phon_O, Phon_rest, Phon_U;
//    bool IsEmpty(void) const { return Outline.empty() && EyesOpen.empty() && EyesClosed.empty() && Phon_AI.empty() && Phon_E.empty() && Phon_etc.empty() && Phon_FV.empty() && Phon_L.empty() && Phon_MBP.empty() && Phon_O.empty() && Phon_rest.empty() && Phon_U.empty(); }
//};

//typedef CoroVoice[4] CoroGroup; //std::vector<CoroVoice>
//coro face settings for 4 voices on multiple groups:
//std::unordered_map<wxString&, CoroGroup> coro_groups;
//static std::vector<std::string> groups;


//this is more concise but harder to read:
//    std::vector<std::pair<wxString, //voices at outer level
//        std::vector<std::pair<wxString, //phrases
//            std::vector<std::pair<wxString, //words
//                std::vector<std::pair<const Phoneme&, int>> >> >> >> voices; //phonemes and start frame#
struct PhonemeInfo
{
    wxString name;
    int start_frame, end_frame;
};

struct WordInfo
{
    wxString name;
    std::vector<PhonemeInfo> phonemes;
};

struct PhraseInfo
{
    wxString name;
    std::vector<WordInfo> words;
};

struct VoiceInfo
{
    wxString name;
    std::vector<PhraseInfo> phrases;
};

struct SongInfo
{
    wxString name;
    int samppersec;
    int numsamp ;
    int numvoices;
};

static std::vector<VoiceInfo> voices; //TODO: move this to xLightsMain.h?
static std::vector<SongInfo> songs; //TODO: move this to xLightsMain.h?

//list of allowed phonemes
static const wxString AllowedPhonemes = _(",AI,E,FV,L,MBP,O,U,WQ,etc,rest,"); //TODO: change to hash<string> if performance is bad

static const wxString SelectionHint = "(choose)", CreationHint = "(add new)";


void xLightsFrame::OnButton_pgo_filenameClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Papagayo File", "", "", "", "Papagayo files (*.pgo)|*.pgo", wxFD_OPEN );
//  wxString filename = "this5.pgo";
    if (!filename.IsEmpty()) TextCtrl_pgo_filename->SetValue(filename);
    LoadPapagayoFile(filename);
}

void xLightsFrame::OnButton_papagayo_output_sequenceClick1(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Output Sequence", "", "", "", "Sequence files (*.xml)|*.xml", wxFD_OPEN );
//  wxString filename = "this5.pgo";
    if (!filename.IsEmpty()) TextCtrl_papagayo_output_filename->SetValue(filename);

}

/*
void xLightsFrame::OnButton_papagayo_output_sequenceClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Output xLights Sequence File", "", "", "", "xLights files (*.xml)|*.xml", wxFD_OPEN );

    if (!filename.IsEmpty()) TextCtrl_papagayo_output_filename->SetValue(filename);
}
*/
void xLightsFrame::OnButtonStartPapagayoClick(wxCommandEvent& event)
{
    ButtonStartPapagayo->Enable(false);
    wxString OutputFormat = ChoiceOutputFormat->GetStringSelection();
    TextCtrlConversionStatus->Clear();


    ButtonStartPapagayo->Enable(true);

//example code to iterate thru the data:
    wxString debug_msg,filename;
//mingw32-make -f makefile.gcc MONOLITHIC=1 SHARED=1 UNICODE=1 CXXFLAGS="-std=gnu++0x" BUILD=release
    filename=TextCtrl_papagayo_output_filename->GetValue();
    int pgofile_status=write_pgo_header(voices.size(),filename);

    int numwr = 0;
    for (auto voice_it = voices.begin(); voice_it != voices.end(); ++voice_it)
    {
        debug_msg += wxString::Format(_("voice[%d/%d] '%s'\n"), voice_it - voices.begin(), voices.size(), voice_it->name.c_str());
//        std::vector<PhraseInfo>& phrases = voice_it->phrases;
        for (auto phrase_it = voice_it->phrases.begin(); phrase_it != voice_it->phrases.end(); ++phrase_it)
        {
            debug_msg += wxString::Format(_("\tphrase[%d/%d] '%s'\n"), phrase_it - voice_it->phrases.begin(), voice_it->phrases.size(), phrase_it->name.c_str());
//            std::vector<WordInfo>& words = phrase_it->words;
            for (auto word_it = phrase_it->words.begin(); word_it != phrase_it->words.end(); ++word_it)
            {
                StatusBar1->SetStatusText(wxString::Format("Writing pgo xml: voice [%d/%d] '%s', phrase[%d/%d] '%s', word[%d/%d] '%s'", voice_it - voices.begin(), voices.size(), voice_it->name.c_str(), phrase_it - voice_it->phrases.begin(), voice_it->phrases.size(), phrase_it->name.c_str(), word_it - phrase_it->words.begin(), phrase_it->words.size(), word_it->name.c_str()));
                debug_msg += wxString::Format(_("\t\tword[%d/%d] '%s'\n"), word_it - phrase_it->words.begin(), phrase_it->words.size(), word_it->name.c_str());
//              std::vector<PhonemeInfo>& phonemes = word_it->phonemes;
                for (auto phoneme_it = word_it->phonemes.begin(); phoneme_it != word_it->phonemes.end(); ++phoneme_it)
                {
                    debug_msg += wxString::Format(_("\t\t\tV%d phoneme[%d/%d] '%s': call routine(start_frame %d, end_frame %d, phoneme '%s')\n"), (voice_it - voices.begin()),
                                                  phoneme_it - word_it->phonemes.begin(), word_it->phonemes.size(), phoneme_it->name.c_str(), phoneme_it->start_frame, phoneme_it->end_frame, phoneme_it->name);
//                  call routine(voice_it,phoneme_it->start_frame, phoneme_it->end_frame, phoneme_it->name);
                    if(pgofile_status) AutoFace((voice_it - voices.begin()),filename, phoneme_it->start_frame,phoneme_it->end_frame,
                                                 phoneme_it->name, word_it->name.c_str());
                    ++numwr;

                    //if (xout) xout->alloff();

                }
            }
        }
    }
    StatusBar1->SetStatusText(wxString::Format("Wrote pgo xml: %d entries", numwr));
    if(pgofile_status)  write_pgo_footer(voices.size(),filename);
    wxMessageBox(debug_msg, _("Papagayo Debug"));
}
// int Voice,int MaxVoice,int StartFrame, int EndFrame,wxString Phoneme
int xLightsFrame::write_pgo_header(int MaxVoices,const wxString& filename)
{

    // wxFile f;
    wxFile f(filename);
    // wxString filename=wxString::Format(("C:\\Vixen.2.1.1\\Sequences\\z.xml"));

    // retmsg(_("Filename: "+filename));
    if (!f.Create(filename,true))
    {
        //  retmsg(_("Unable to create file: "+filename));
        wxMessageBox(wxString::Format("write_pgo_header: Unable to create file %s. Error %d\n",filename,f.GetLastError()));
        return 0;
    }


    //buff += wxString::Format("%d ",(*dataBuf)[seqidx]);

    //    buff += wxString::Format("\n");
    //    f.Write(buff);

// f.Write("\t\t\t</channels>\n");
    if(f.IsOpened())
    {
        f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        f.Write("<xsequence BaseChannel=\"0\" ChanCtrlBasic=\"0\" ChanCtrlColor=\"0\">\n");
        f.Write("<tr>\n");
        f.Write("<td>Start Time</td>\n");
        f.Write("<td>Label</td>\n");
        for(int voice=1; voice<=MaxVoices; voice++)
        {
            f.Write(wxString::Format("<td>VOICE%d</td>\n",voice));
        }


        f.Write("</tr>\n");
        f.Write("<tr>\n");
        f.Write("    <td Protected=\"0\">0.000</td>\n");
        f.Write("    <td Protected=\"0\">Blank</td>\n");
        f.Write("    <td Protected=\"0\">Color Wash,None,Effect 1,ID_CHECKBOX_LayerMorph=0,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0,E1_SLIDER_ColorWash_Count=1,E1_CHECKBOX_ColorWash_HFade=0,E1_CHECKBOX_ColorWash_VFade=0,E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=0,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=0,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=1,E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0</td>\n");
        f.Write("    <td Protected=\"0\">Color Wash,None,Effect 1,ID_CHECKBOX_LayerMorph=0,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0,E1_SLIDER_ColorWash_Count=1,E1_CHECKBOX_ColorWash_HFade=0,E1_CHECKBOX_ColorWash_VFade=0,E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=0,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=0,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=1,E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0</td>\n");
        f.Write("</tr>\n");
        f.Close();
        return 1; // good exit
    }
    return 0;   // bad exit
}

void xLightsFrame::write_pgo_footer(int MaxVoices,const wxString& filename)
{

    // wxFile f;
    wxFile f(filename);
    if (!f.Open(filename,wxFile::write_append))
    {
        retmsg(_("Unable to open for append, file: "+filename));

    }
    f.SeekEnd(0);

    f.Write("</xsequence>\n");
    f.Close();

}

void xLightsFrame::AutoFace(int MaxVoices,const wxString& filename,int start_frame,int end_frame,const wxString& phoneme,const wxString& word)
{
    wxString label;
    double seconds;
    wxFile f;

    // retmsg(_("Filename: "+filename));
    if (!f.Open(filename,wxFile::write_append))
    {
        retmsg(_("Unable to open for append, file: "+filename));
    }
    f.SeekEnd(0);
    label = "'" + word + "':'" + phoneme + "'";
    seconds = (double) start_frame * 0.050;  // assume 20fps fpr the papagayo file. not a good assumption
    f.Write("<tr>\n");
    f.Write(wxString::Format("   <td Protected=\"0\">%7.3f</td>\n",seconds));
    f.Write(wxString::Format("   <td Protected=\"0\">%s</td>\n",label));
    f.Write(wxString::Format("   <td Protected=\"0\">Faces,None,Effect 1,ID_CHECKBOX_LayerMorph=0,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0,E1_CHOICE_Faces_Phoneme=%s,E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=1,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=1,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=0,E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0</td>\n", phoneme));
    f.Write(wxString::Format("   <td Protected=\"0\">Faces,None,Effect 1,ID_CHECKBOX_LayerMorph=0,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0,E1_CHOICE_Faces_Phoneme=%s,E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=1,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=1,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=0,E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0</td>\n", phoneme));
    f.Write("</tr>\n");
}


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
        if ((ofs = PapagayoFileInfo.linebuf.find("#")) != std::string::npos) PapagayoFileInfo.linebuf.erase(ofs); //allow comments
        while (!PapagayoFileInfo.linebuf.empty() && (PapagayoFileInfo.linebuf.Last() == '\\')) //allow line continuation
        {
            PapagayoFileInfo.linebuf.RemoveLast(); //remove trailing "\"
            /*std::*/wxString morebuf = PapagayoFileInfo.file.GetNextLine();
            if (PapagayoFileInfo.file.Eof()) break;
            PapagayoFileInfo.linebuf += morebuf;
            ++PapagayoFileInfo.linenum;
        }
        while (!PapagayoFileInfo.linebuf.empty() && isspace(PapagayoFileInfo.linebuf[0])) PapagayoFileInfo.linebuf = PapagayoFileInfo.linebuf.substr(1); //.RemoveFirst(); //trim leading spaces
        while (!PapagayoFileInfo.linebuf.empty() && isspace(PapagayoFileInfo.linebuf.Last())) PapagayoFileInfo.linebuf.RemoveLast(); //trim trailing spaces
//NO        if (PapagayoFileInfo.linebuf.empty()) continue; //skip blank lines; NOTE: line could be blank for empty voice
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
// scm, we wont be using the wav file in *.pgo files.
// no need to check for it
//   if (soundpath.IsEmpty() || !wxFileExists(soundpath)) warnmsg(wxString::Format(_("Sound file '%s' is missing"), soundpath.c_str()));
//    wxFile soundfile(soundpath);
//    if (!soundfile.Open(filename.c_str())) retmsg(_("Can't open file"));
//    if (ext != ".wav") error?


    wxRegEx number("^[0-9]+$");
    int samppersec = number.Matches(readline())? wxAtoi(PapagayoFileInfo.linebuf): -1;
    if (samppersec < 1) warnmsg(wxString::Format(_("Invalid file @line %d ('%s' samples per sec)"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str()));

    int numsamp = number.Matches(readline())? wxAtoi(PapagayoFileInfo.linebuf): -1;
    if (numsamp < 1) warnmsg(wxString::Format(_("Invalid file @line %d ('%s' song samples)"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str()));

    wxString desc;
    int numvoices = number.Matches(readline())? wxAtoi(PapagayoFileInfo.linebuf): -1;
    if (numvoices < 1) retmsg(wxString::Format(_("Invalid file @line %d ('%s' voices)"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str()));


  /* struct SongInfo
{
    wxString name;
    int samppersec;
    int numsamp ;
    int numvoices;
   };
   */
  // songs.samppersec=samppersec;

    for (int v = 1; v <= numvoices; ++v)
    {
        wxString voicename = readline();
        if (voicename.empty())
        {
            warnmsg(wxString::Format(_("Missing voice# %d of %d"), v, numvoices));
            break;
        }
        readline(); //all phrases for voice, "|" delimiter; TODO: do we need to save this?
        desc = wxString::Format(_("voice# %d '%s' @line %d"), v, voicename, PapagayoFileInfo.linenum);
        debug(10, (const char*)desc.c_str());
        int numphrases = number.Matches(readline())? wxAtoi(PapagayoFileInfo.linebuf): -1;
        if (numphrases < 0) retmsg(wxString::Format(_("Invalid file @line %d ('%s' phrases for %s)"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str(), desc.c_str()));
        if (!numphrases) warnmsg(wxString::Format(_("Suspicious file @line %d ('%s' phrases for %s)"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str(), desc.c_str()));

        VoiceInfo newvoice;
        newvoice.name = voicename;
        for (int phr = 1; phr <= numphrases; ++phr)
        {
            wxString phrasename = readline();
            if (phrasename.empty())
            {
                warnmsg(wxString::Format(_("Missing phrase# %d of %d for %s"), phr, numphrases, desc.c_str()));
                break;
            }
            readline(); //start frame TODO: do we need to save this?
            readline(); //end frame TODO: do we need to save this?
            desc = wxString::Format(_("voice# %d, phrase %d '%s' @line %d"), v, phr, phrasename, PapagayoFileInfo.linenum);
            debug(10, (const char*)desc.c_str());
            PhraseInfo newphrase;
            newphrase.name = phrasename;
            int numwords = number.Matches(readline())? wxAtoi(PapagayoFileInfo.linebuf): -1;
            if (numwords < 0) retmsg(wxString::Format(_("Invalid file @line %d ('%s' words for %s)"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str(), desc.c_str()));
            if (!numwords) warnmsg(wxString::Format(_("Suspicious file @line %d ('%s' words for %s)"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str(), desc.c_str()));
            for (int w = 1; w <= numwords; ++w)
            {
                wxStringTokenizer wtkz(readline(), " ");
                wxString wordname = wtkz.GetNextToken();
                if (wordname.empty())
                {
                    warnmsg(wxString::Format(_("Missing word# %d of %d for %s"), w, numwords, desc.c_str()));
                    break;
                }
                WordInfo newword;
                newword.name = wordname;
                wtkz.GetNextToken(); //start frame TODO: do we need to save this?
                wxString endfr = wtkz.GetNextToken(); //end frame TODO: do we need to save this?
//                voices.back().phrases.push_back(newphrase);
                wxString syllcount = wtkz.GetNextToken();
                wxString junk = wtkz.GetNextToken();
//                wxMessageBox(wxString::Format(_("word '%s', end fr %s, #syll %s, junk %s"), wordname.c_str(), endfr.c_str(), syllcount.c_str(), junk.c_str()));
                desc = wxString::Format(_("voice# %d, phrase %d, word %d '%s' @line %d"), v, phr, w, wordname, PapagayoFileInfo.linenum);
                if (!junk.empty()) warnmsg(wxString::Format(_("Ignoring junk '%s' at end of %s @line %d"), junk.c_str(), desc.c_str(), PapagayoFileInfo.linenum));
                debug(10, (const char*)desc.c_str());

                int end_frame = number.Matches(endfr)? wxAtoi(endfr): -1;
                if (end_frame < 1) warnmsg(wxString::Format(_("Invalid file @line %d ('%s' end frame for %s)"), PapagayoFileInfo.linenum, endfr.c_str(), desc.c_str()));

                int numsylls = number.Matches(syllcount)? wxAtoi(syllcount): -1;
                if (numsylls < 0) retmsg(wxString::Format(_("Invalid file @line %d ('%s' phonemes for %s)"), PapagayoFileInfo.linenum, syllcount.c_str(), desc.c_str()));
                if (!numsylls) warnmsg(wxString::Format(_("Suspicious file @line %d ('%s' phonemes for %s)"), PapagayoFileInfo.linenum, syllcount.c_str(), desc.c_str()));
                for (int syll = 1; syll <= numsylls; ++syll)
                {
                    wxStringTokenizer stkz = wxStringTokenizer(readline(), " ");
                    wxString stframe = stkz.GetNextToken();
                    wxString syllname = stkz.GetNextToken();
//                    wxMessageBox(wxString::Format(_("get syll %s, st fr %s, syll %s, allowed in %s? %d"), PapagayoFileInfo.linebuf, stframe, _(",") + newsyll.name + _(","), AllowedPhonemes, AllowedPhonemes.find(_(",") + newsyll.name + _(","))));
                    if (syllname.empty() || (AllowedPhonemes.find(_(",") + syllname + _(",")) == -1))
                    {
                        retmsg(wxString::Format(_("Missing phoneme# %d of %d for %s"), syll, numsylls, desc.c_str()));
                        break;
                    }
                    /*wxString*/ junk = stkz.GetNextToken();
                    desc = wxString::Format(_("voice# %d, phrase %d, word %d, phoneme %d '%s' @line %d"), v, phr, w, syll, syllname, PapagayoFileInfo.linenum);
                    if (!junk.empty()) warnmsg(wxString::Format(_("Ignoring junk '%s' at end of %s @line %d"), junk.c_str(), desc.c_str(), PapagayoFileInfo.linenum));
                    debug(10, (const char*)desc.c_str());
                    PhonemeInfo newsyll;
                    newsyll.name = syllname;

                    newsyll.start_frame = number.Matches(stframe)? wxAtoi(stframe): -1;
                    if (newsyll.start_frame == -1) retmsg(wxString::Format(_("Invalid file @line %d ('%s' start frame for %s)"), PapagayoFileInfo.linenum, stframe.c_str(), desc.c_str()));
                    newsyll.end_frame = end_frame; //assume end of phrase until another phoneme is found
                    if (syll > 1) (&newsyll)[-1].end_frame = newsyll.start_frame; //don't overlap?
                    newword.phonemes.push_back(newsyll);
                }
                newphrase.words.push_back(newword);

            }
            newvoice.phrases.push_back(newphrase);

        }
        voices.push_back(newvoice);
    }
    if (!readline().empty()) warnmsg(wxString::Format(_("Ignoring junk at eof ('%s' found on line %d)"), PapagayoFileInfo.linebuf.c_str(), PapagayoFileInfo.linenum));

    if (!warnings.empty()) wxMessageBox(warnings, _("Papagayo Warning"));
    debug(3, "file loaded %s", (const char*)warnings.c_str());
}

//don't create dup attrs:
#if 0 //broken
class myXmlNode: public wxXmlNode
{
public:
    myXmlNode(wxXmlNodeType type, const wxString& value): wxXmlNode(type, value) {};
    myXmlNode(const wxXmlNode& node): wxXmlNode(node) {};
public:
    void AddChild(myXmlNode*& child) { wxXmlNode::AddChild(child); };
    bool RemoveChild (myXmlNode* child) { return wxXmlNode::RemoveChild(child); }
    void AddAttribute(const wxString& name, const wxString& value)
    {
        wxString junk;
        if (GetAttribute(name, &junk)) DeleteAttribute(name); //kludge: avoid dups
        if (!value.empty()) wxXmlNode::AddAttribute(name, value);
    }
};
#else
void AddNonDupAttr(wxXmlNode* node, const wxString& name, const wxString& value)
{
    wxString junk;
    if (node->GetAttribute(name, &junk)) node->DeleteAttribute(name); //kludge: avoid dups
    if (!value.empty()) node->AddAttribute(name, value);
}
#endif

//sigh; a function like this should have been built into wxWidgets
wxXmlNode* FindNode(wxXmlNode* parent, const wxString& tag, const wxString& attr, const wxString& value, bool create = false)
{
#if 0
    static struct
    {
        std::unordered_map<const char*, wxXmlNode*> nodes;
        std::string parent, child;
    } cached_names;

    if (parent->GetName() != cached_names.parent) //reload cache
    {
        cached_names.nodes.clear();
        for (wxXmlNode* node = parent->GetChildren(); node != NULL; node = node->GetNext())
            cached_names.nodes[node->GetName()] = node;
        cached_names.parent = parent;
    }
    if (cached_names.nodes.find(tag) == cached_names.nodes.end()) //not found
    {
        if (!create) return 0;
        parent->AddChild(cached_names.nodes[tag] = new wxXmlNode(wxXML_ELEMENT_NODE, tag));
    }
    return cached_names.nodes[tag];
#endif // 0
    for (wxXmlNode* node = parent->GetChildren(); node != NULL; node = node->GetNext())
    {
        if (!tag.empty() && (node->GetName() != tag)) continue;
        if (!value.empty() && (node->GetAttribute(attr) != value)) continue;
        return node;
    }
    if (!create) return 0; //CAUTION: this will give null ptr exc if caller does not check
    wxXmlNode* retnode = new wxXmlNode(wxXML_ELEMENT_NODE, tag); //NOTE: assumes !tag.empty()
    parent->AddChild(retnode);
    if (!value.empty()) AddNonDupAttr(retnode, attr, value);
    return retnode;
}


#if 0 //example file:
<papagayo>
    <autoface>
        <auto name="MTREE" face_shape="Y" outline="Y" />
    </autoface>
    <corofaces>
        <coro name="HALLOWEEN">
            <voice  voiceNumber="1" name="FACE1" AI="100" E="100" etc="100" FV="100" L="100" MBP="100" O="100" rest="100" U="100" WQ="100" Outline="100" Eyes_open="0" Eyes_Closed="0" />
            <voice  voiceNumber="2" name="FACE2" AI="100" E="100" etc="100" FV="100" L="100" MBP="100" O="100" rest="100" U="100" WQ="100" Outline="100" Eyes_open="0" Eyes_Closed="0" />
            <voice  voiceNumber="3" name="FACE3" AI="100" E="100" etc="100" FV="100" L="100" MBP="100" O="100" rest="100" U="100" WQ="100" Outline="100" Eyes_open="0" Eyes_Closed="0" />
            <voice  voiceNumber="4" name="FACE4" AI="100" E="100" etc="100" FV="100" L="100" MBP="100" O="100" rest="100" U="100" WQ="100" Outline="100" Eyes_open="0" Eyes_Closed="0" />
        </coro>
    </corofaces>
    <images>
        <coro name="PUMPKINS">
            <voice  voiceNumber="1" name="PUMPKIN1" AI="C:\Users\sean\Pictures\black.jpg" E="C:\Users\sean\Pictures\black.jpg"
            etc="C:\Users\sean\Pictures\black.jpg" FV="C:\Users\sean\Pictures\black.jpg" L="C:\Users\sean\Pictures\black.jpg"
            MBP="C:\Users\sean\Pictures\black.jpg" O="C:\Users\sean\Pictures\black.jpg" rest="C:\Users\sean\Pictures\black.jpg"
            U="C:\Users\sean\Pictures\black.jpg" WQ="C:\Users\sean\Pictures\black.jpg" Outline="C:\Users\sean\Pictures\black.jpg"
            Eyes_open="C:\Users\sean\Pictures\black.jpg" Eyes_Closed="C:\Users\sean\Pictures\black.jpg" />
        </coro>
    </images>
</papagayo>
#endif // 0

bool xLightsFrame::LoadPgoSettings(void)
{
    enum {NoFile, LoadErr, BadRoot, Okay} status = Okay;
//    std::unordered_map<const char*, wxXmlNode*> nodes;
//    wxXmlDocument pgoXml;
    wxFileName pgoFile;
//    wxXmlNode* root = 0;
//    wxXmlNode* AutoFace = 0;
//    wxXmlNode* CoroFaces = 0;
//    wxXmlNode* Images = 0;

    pgoFile.AssignDir(CurrentDir);
    pgoFile.SetFullName(_(XLIGHTS_PGOFACES_FILE));
    if (!pgoFile.FileExists()) status = NoFile;
    else if (!pgoXml.Load(pgoFile.GetFullPath()))
    {
        wxMessageBox(wxT("Unable to load Papagayo settings file."), wxT("Error"));
        status = LoadErr;
    }
    else
    {
        wxXmlNode* root = pgoXml.GetRoot();
        if (!root || (root->GetName() != "papagayo"))
        {
            wxMessageBox(wxT("Invalid Papagayo settings file.  Press Save button to create a new file."), wxT("Error"));
            status = BadRoot;
        }
    }
    if (status != Okay) pgoXml.SetRoot(new wxXmlNode(wxXML_ELEMENT_NODE, "papagayo"));
    debug(10, "LoadPgoSetting: load file %s, status %d", (const char*)pgoFile.GetFullPath().c_str(), status);
//    for (wxXmlNode* node = root->GetChildren(); node != NULL; node = node->GetNext())
//        nodes[node->GetName()] = node;
//    const char* branches[] = {"autoface", "images", "corofaces"};
//    for (int i = 0; i < numents(branches); ++i)
//        if (!root->FindNode(branches[i]]) nodes["root"]->AddChild(nodes[branches[i] = new wxXmlNode(wxXML_ELEMENT_NODE, branches[i]));

//load autoface settings:
//there's no group name on this tab, so set all the UI controls here
    wxXmlNode* AutoFace = FindNode(pgoXml.GetRoot(), wxT("autoface"), wxT("name"), wxEmptyString, true);
//    XmlNode* first_face = parent->GetChildren(); //TODO: look at multiple children?
    wxXmlNode* any_node = FindNode(AutoFace, wxT("auto"), wxT("name"), wxEmptyString, false); //TODO: not sure which child node to use; there are no group names on this tab
//    any_node->AddAttribute(wxT("face_shape"), RadioButton_PgoFaceRound->GetValue()? wxT("Y"): wxT("N"));
    if (any_node)
    {
        wxString shape = any_node->GetAttribute(wxT("face_shape"), wxT("Y")); //TODO: Y/N doesn't seem like the best choice here
        RadioButton_PgoFaceRound->SetValue(shape == wxT("Y"));
        RadioButton_PgoFaceRect->SetValue(shape != wxT("Y"));
        CheckBox_PgoFaceOutline->SetValue(any_node->GetAttribute(wxT("outline"), wxT("Y")) == wxT("Y"));
    }

//load corofaces settings:
//individual UI controls are loaded when the user chooses a group name later
//only the list of available groups is populated here
    Choice_PgoGroupName->Clear();
    Choice_PgoGroupName->Append(SelectionHint); //wxT("(choose one)"));
    wxXmlNode* CoroFaces = FindNode(pgoXml.GetRoot(), wxT("corofaces"), wxT("name"), wxEmptyString, true);
    for (wxXmlNode* group = CoroFaces->GetChildren(); group != NULL; group = group->GetNext())
    {
        wxString grpname = group->GetAttribute(wxT("name"));
        debug(15, "found %s group '%s'", (const char*)group->GetName().c_str(), (const char*)group->GetAttribute(wxT("name"), wxT("??")).c_str());
        if (group->GetName() != "coro") continue;
        if (grpname.IsEmpty()) continue;
//        CoroGroup newgrp;
        Choice_PgoGroupName->Append(grpname); //build user's choice list for group names
//        std::pair<iterator, bool> newgrp = coro_groups.emplace(grpname);
//        for (int i = 0; i < numents(CoroGroup); ++i)
//            newgrp.first[i].empty = true;
//        coro_groups.emplace(grpname, newgrp);
#if 0 //don't need this until user selects this group
        for (wxXmlNode* voice = group->GetChildren(); voice != NULL; voice = voice->GetNext())
        {
            int inx = wxAtoi(voice->GetAttribute(wxT("voiceNumber"), wxT("-1")));
            debug(5, "found %s voice '%s' inx %d", voice->GetName(), voice->GetAttribute(wxT("name"), wxT("??")), inx);
            if (voice->GetName() != "voice") continue;
            if ((inx < 1) || (inx > GridCoroFaces->GetCols())) continue; //out of bounds
//            wxMessageBox(wxT("Invalid Papagayo settings file.  Press Save button to create a new file."), wxT("Error"));
//            if (!(*newgrp.first)[inx - 1].IsEmpty()) continue; //duplicate entry?
//TODO: what to do with name attr?
            (*newgrp.first)[inx - 1].Outline = voice->GetAttribute(wxT("Outline")
            (*newgrp.first)[inx - 1].EyesOpen = voice->GetAttribute(wxT("Eyes_open")
            (*newgrp.first)[inx - 1].EyesClosed = voice->GetAttribute(wxT("Eyes_closed")
            (*newgrp.first)[inx - 1].Phon_AI = voice->GetAttribute(wxT("AI")
            (*newgrp.first)[inx - 1].Phon_E = voice->GetAttribute(wxT("E")
            (*newgrp.first)[inx - 1].Phon_etc = voice->GetAttribute(wxT("etc")
            (*newgrp.first)[inx - 1].Phon_FV = voice->GetAttribute(wxT("FV")
            (*newgrp.first)[inx - 1].Phon_L = voice->GetAttribute(wxT("L")
            (*newgrp.first)[inx - 1].Phon_MBP = voice->GetAttribute(wxT("MBP")
            (*newgrp.first)[inx - 1].Phon_O = voice->GetAttribute(wxT("O")
            (*newgrp.first)[inx - 1].Phon_rest = voice->GetAttribute(wxT("rest")
            (*newgrp.first)[inx - 1].Phon_U = voice->GetAttribute(wxT("U")
            (*newgrp.first)[inx - 1].Phon_WQ = voice->GetAttribute(wxT("WQ")
        }
#endif // 0
    }

//load image settings:
    wxXmlNode* Images = FindNode(pgoXml.GetRoot(), wxT("images"), wxT("name"), wxEmptyString, true);
    for (wxXmlNode* image = Images->GetChildren(); image != NULL; image = image->GetNext())
    {
        wxString grpname = image->GetAttribute(wxT("name"));
        debug(5, "found %s image '%s'", (const char*)image->GetName().c_str(), (const char*)image->GetAttribute(wxT("name"), wxT("??")).c_str());
        if (image->GetName() != "coro") continue;
//TODO: set group name choices
    }

//load mp4 settings:
    wxXmlNode* Mp4 = FindNode(pgoXml.GetRoot(), wxT("mp4"), wxT("name"), wxEmptyString, true);
    for (wxXmlNode* image = Mp4->GetChildren(); image != NULL; image = image->GetNext())
    {
        wxString grpname = image->GetAttribute(wxT("name"));
        debug(5, "found %s image '%s'", (const char*)image->GetName().c_str(), (const char*)image->GetAttribute(wxT("name"), wxT("??")).c_str());
        if (image->GetName() != "coro") continue;
//TODO: set group name choices
    }

    return true;
}

bool xLightsFrame::GetGroupName(wxString& grpname)
{
    grpname = Choice_PgoGroupName->GetString(Choice_PgoGroupName->GetSelection());
    if (grpname == SelectionHint)
    {
        wxMessageBox(wxT("Please select a group name."), wxT("Required Input"));
//        Choice_PgoGroupName->SetSelection(-1); //force event handler to fire again next time
        return false;
    }
    if (grpname == CreationHint)
    {
        if (!EffectTreeDialog::PromptForName(this, &grpname, wxT("Enter new group name"), wxT("Group name must not be empty")))
        {
            Choice_PgoGroupName->SetSelection(-1); //force event handler to fire again next time
            return false;
        }
        Choice_PgoGroupName->Delete(Choice_PgoGroupName->GetCount() - 1); //remove "add new"
        Choice_PgoGroupName->Append(grpname);
        Choice_PgoGroupName->Append(CreationHint);
        Choice_PgoGroupName->SetSelection(Choice_PgoGroupName->GetCount() - 2); //select new group
    }
    return true;
}

bool xLightsFrame::SavePgoSettings(void)
{
    wxFileName pgoFile;
//    wxXmlDocument pgoXml;
    pgoFile.AssignDir(CurrentDir);
    pgoFile.SetFullName(wxT(XLIGHTS_PGOFACES_FILE));
    debug(10, "SavePgoSettings: write xmldoc to '%s'", (const char*)pgoFile.GetFullPath().c_str());

//save autoface settings:
//no group name on this tab, so save all the UI controls here
    wxXmlNode* AutoFace = FindNode(pgoXml.GetRoot(), wxT("autoface"), wxT("name"), wxEmptyString, true);
    wxXmlNode* node = FindNode(AutoFace, wxT("auto"), wxT("name"), wxEmptyString, true); //TODO: not sure which child node to use; there are no group names on this tab
    if (node->GetAttribute(wxT("name")).empty()) AddNonDupAttr(node, wxT("name"), wxT("NAME?")); //give it a name (not sure what name to use)
    AddNonDupAttr(node, wxT("face_shape"), RadioButton_PgoFaceRound->GetValue()? wxT("Y"): wxT("N"));
    AddNonDupAttr(node, wxT("outline"), CheckBox_PgoFaceOutline->GetValue()? wxT("Y"): wxT("N"));

//save corofaces settings:
//individual UI controls are saved for each group
//the entire xmldoc is saved here
#if 0
    bool isempty = true;
    CoroVoice coro_voice;
    wxXmlNode* CoroFaces = FindNode(pgoXml->GetRoot(), "corofaces", true);
    node = FindNode(CoroFaces, "coro", grpname, true);
    for (int i = 0; i < GridCoroFaces->GetCols(); ++i)
    {
        XmlNode* voice = FindNode(node, "voice", wxString::Format(wxT("FACE%d"), i + 1), true);
        voice->AddAttribute(wxT("voiceNumber"), wxString::Format(wxT("%d"), i + 1));
//CAUTION: code below assumes a specific layout in the grid
        voice->AddAttribute(wxT("Outline"), coro_voice.Outline = GridCoroFaces->GetCellValue(0, i));
        voice->AddAttribute(wxT("Eyes_open"), coro_voice.EyesOpen = GridCoroFaces->GetCellValue(1, i));
        voice->AddAttribute(wxT("Eyes_closed"), coro_voice.EyesClosed = GridCoroFaces->GetCellValue(2, i));
        voice->AddAttribute(wxT("AI"), coro_voice.Phon_AI = GridCoroFaces->GetCellValue(3, i));
        voice->AddAttribute(wxT("E"), coro_voice.Phon_E = GridCoroFaces->GetCellValue(4, i));
        voice->AddAttribute(wxT("etc"), coro_voice.Phon_etc = GridCoroFaces->GetCellValue(5, i));
        voice->AddAttribute(wxT("FV"), coro_voice.Phon_FV = GridCoroFaces->GetCellValue(6, i));
        voice->AddAttribute(wxT("L"), coro_voice.Phon_L = GridCoroFaces->GetCellValue(7, i));
        voice->AddAttribute(wxT("MBP"), coro_voice.Phon_MBP = GridCoroFaces->GetCellValue(8, i));
        voice->AddAttribute(wxT("O"), coro_voice.Phon_O = GridCoroFaces->GetCellValue(9, i));
        voice->AddAttribute(wxT("rest"), coro_voice.Phon_rest = GridCoroFaces->GetCellValue(10, i));
        voice->AddAttribute(wxT("U"), coro_voice.Phon_U = GridCoroFaces->GetCellValue(11, i));
        voice->AddAttribute(wxT("WQ"), coro_voice.Phon_WQ = GridCoroFaces->GetCellValue(12, i));
        if (!coro_voice.IsEmpty()) isempty = false;
        debug(15, "SavePgoSettings: group '%s', voice# %d empty? %d", grpname, i + 1, coro_voice.IsEmpty());
    }
    if (isempty) CoroFaces->RemoveChild(node); //delete group; nothing to save
#endif // 0

//load image settings:
//    wxXmlNode* Images = FindNode(pgoXml->GetRoot(), "images", true);
//TODO
//individual UI controls are saved for each group
//the entire xmldoc is saved here

//load mp4 settings:
//    wxXmlNode* Mp4 = FindNode(pgoXml->GetRoot(), "mp4", true);
//TODO
//individual UI controls are saved for each group
//the entire xmldoc is saved here

    if (!pgoXml.Save(pgoFile.GetFullPath()))
    {
        wxMessageBox(wxT("Unable to save Papagayo settings"), wxT("Error"));
        return false;
    }
//    UnsavedChanges=false;
    debug(10, "SavePgoSettings: saved ok");
    return true;
}

void xLightsFrame::OnButtonPgoImageClick(wxCommandEvent& event)
{
    wxString ButtonWindowName=((wxWindow*)event.GetEventObject())->GetName();
    wxString TextCtrlName=ButtonWindowName;
    TextCtrlName.Replace("BUTTON","TEXTCTRL");
    wxTextCtrl* TargetTextBox=(wxTextCtrl*)wxWindow::FindWindowByName(TextCtrlName,NotebookPgoParms);
    if (TargetTextBox!=NULL && FileDialogPgoImage->ShowModal() == wxID_OK)
    {
        TargetTextBox->SetValue(FileDialogPgoImage->GetPath());
    }
}

//allow array-like access to Voice drop-down lists:
//NOTE: must have same number of entries as columns in grid
wxChoice* xLightsFrame::Voice(int inx)
{
    switch (inx)
    {
        case 0: return Choice_PgoModelVoice1;
        case 1: return Choice_PgoModelVoice2;
        case 2: return Choice_PgoModelVoice3;
        case 3: return Choice_PgoModelVoice4;
        default: return 0;
    }
}

//populate choice lists with model names, etc.
void xLightsFrame::InitPapagayoTab(void)
{

//??    if (Choice_PgoGroupName->GetCount() < 1)
//    {
//        Choice_PgoGroupName->Clear();
//        Choice_PgoGroupName->Append(SelectionHint); //wxT("(choose one)"));
//    }
    LoadPgoSettings();
    Choice_PgoGroupName->Append(CreationHint); //wxT("(add new)"));
    Choice_PgoGroupName->SetSelection(0); //this will be "(choose)" if any groups exist, or "(add new)" otherwise

//??    if (Choice_PgoModelVoice1->GetCount()) return;
    for (int i = 0; i < GridCoroFaces->GetCols() /*numents(voices)*/; ++i)
    {
        Voice(i)->Clear();
        Voice(i)->Append(SelectionHint); //wxT("(choose)"));
        Voice(i)->SetSelection(0);
    }
    for (auto it = xLightsFrame::PreviewModels.begin(); it != xLightsFrame::PreviewModels.end(); ++it)
    {
        if ((*it)->name.IsEmpty()) continue;
        for (int i = 0; i < GridCoroFaces->GetCols() /*numents(voices)*/; ++i)
            Voice(i)->Append((*it)->name);
    }
}

//NOTE: this only saves one group name at a time to the xmldoc, then saves entire xmldoc to file
void xLightsFrame::OnBitmapButton_SaveCoroGroupClick(wxCommandEvent& event)
{
    wxString grpname;
    if (!GetGroupName(grpname)) return;
    debug(10, "SaveCoroGroupClick: save group '%s' to xmldoc", (const char*)grpname.c_str());
    wxXmlNode* CoroFaces = FindNode(pgoXml.GetRoot(), wxT("corofaces"), wxT("name"), wxEmptyString, true);
    wxXmlNode* node = FindNode(CoroFaces, wxT("coro"), wxT("name"), grpname, true);
    int num_voice = -1;
    wxString warnings;
    for (int i = 0; i < GridCoroFaces->GetCols(); ++i)
    {
        bool non_empty = false;
        for (int r = 0; r <= 12; ++r)
        {
            if (GridCoroFaces->GetCellValue(r, i).empty()) continue;
            non_empty = true;
            break;
        }
        wxString voice_model;
        if (Voice(i)->GetSelection() >= 0) voice_model = Voice(i)->GetString(Voice(i)->GetSelection());
        if (voice_model == SelectionHint) //warn if user forgot to set model
        {
            if (non_empty) warnings += wxString::Format(wxT("Voice# %d not saved (no model selected).\n"), i + 1);
            continue;
        }
        wxXmlNode* voice = FindNode(node, "voice", wxT("voiceNumber"), wxString::Format(wxT("%d"), i + 1), true);
//        voice->AddAttribute(wxT("voiceNumber"), wxString::Format(wxT("%d"), i + 1));
        AddNonDupAttr(voice, wxT("name"), voice_model);
        AddNonDupAttr(voice, wxT("Outline"), GridCoroFaces->GetCellValue(0, i));
        AddNonDupAttr(voice, wxT("Eyes_open"), GridCoroFaces->GetCellValue(1, i));
        AddNonDupAttr(voice, wxT("Eyes_closed"), GridCoroFaces->GetCellValue(2, i));
        AddNonDupAttr(voice, wxT("AI"), GridCoroFaces->GetCellValue(3, i));
        AddNonDupAttr(voice, wxT("E"), GridCoroFaces->GetCellValue(4, i));
        AddNonDupAttr(voice, wxT("etc"), GridCoroFaces->GetCellValue(5, i));
        AddNonDupAttr(voice, wxT("FV"), GridCoroFaces->GetCellValue(6, i));
        AddNonDupAttr(voice, wxT("L"), GridCoroFaces->GetCellValue(7, i));
        AddNonDupAttr(voice, wxT("MBP"), GridCoroFaces->GetCellValue(8, i));
        AddNonDupAttr(voice, wxT("O"), GridCoroFaces->GetCellValue(9, i));
        AddNonDupAttr(voice, wxT("rest"), GridCoroFaces->GetCellValue(10, i));
        AddNonDupAttr(voice, wxT("U"), GridCoroFaces->GetCellValue(11, i));
        AddNonDupAttr(voice, wxT("WQ"), GridCoroFaces->GetCellValue(12, i));

        if (num_voice < 0) ++num_voice; //remember that a voice was selected
        if (non_empty) ++num_voice;
    }
    if (num_voice < 0)
    {
        wxMessageBox(wxT("Please select one or more voice models."), wxT("Missing data"));
        return;
    }
    if (!SavePgoSettings()) return; //TODO: this should be called from somewhere else as well
    wxMessageBox(warnings + wxString::Format(wxT("Papagayo settings (%d %s) saved for group '%s'."), num_voice, (num_voice == 1)? wxT("voice"): wxT("voices"), grpname), wxT("Success"));
}

//just use choice list event instead of explicit Open button:
//void xLightsFrame::OnBitmapButton_OpenCoroGroupClick(wxCommandEvent& event)
//{
//}
//this loads one group name at a time from the xmldoc
void xLightsFrame::OnChoice_PgoGroupNameSelect(wxCommandEvent& event)
{
    wxString grpname;
    if (!GetGroupName(grpname)) return;
    debug(10, "PgoGroupNameSelect: load group '%s' from xmldoc", (const char*)grpname.c_str());
    wxXmlNode* CoroFaces = FindNode(pgoXml.GetRoot(), wxT("corofaces"), wxT("name"), wxEmptyString, true);
    wxXmlNode* node = FindNode(CoroFaces, wxT("coro"), wxT("name"), grpname, true);
    for (int i = 0; i < GridCoroFaces->GetCols(); ++i)
//    for (wxXmlNode* voice = node->GetChildren(); voice != NULL; voice = voice->GetNext())
    {
        wxXmlNode* voice = FindNode(node, "voice", wxT("voiceNumber"), wxString::Format(wxT("%d"), i + 1), true);
//        int voice_num = wxAtoi(voice->GetAttribute(wxT("voiceNumber"), wxT("0")));
//        if ((voice_num < 1) || (voice_num > GridCoroFaces->GetCols())) continue; //bad voice#
        int inx = Voice(i)->FindString(voice->GetAttribute(wxT("name"), wxEmptyString));
        if ((inx < 0) && !voice->GetAttribute(wxT("name"), wxEmptyString).empty()) wxMessageBox(wxString::Format(wxT("Model '%s' not found for voice# %d."), voice->GetAttribute(wxT("name")), i), wxT("Bad config setting"));
        if (inx < 0) inx = 0; //default to "(choose)"
        Voice(i)->SetSelection(inx);
//        voice->AddAttribute(wxT("voiceNumber"), wxString::Format(wxT("%d"), i + 1));
        GridCoroFaces->SetCellValue(0, i, voice->GetAttribute(wxT("Outline")));
        GridCoroFaces->SetCellValue(1, i, voice->GetAttribute(wxT("Eyes_open")));
        GridCoroFaces->SetCellValue(2, i, voice->GetAttribute(wxT("Eyes_closed")));
        GridCoroFaces->SetCellValue(3, i, voice->GetAttribute(wxT("AI")));
        GridCoroFaces->SetCellValue(4, i, voice->GetAttribute(wxT("E")));
        GridCoroFaces->SetCellValue(5, i, voice->GetAttribute(wxT("etc")));
        GridCoroFaces->SetCellValue(6, i, voice->GetAttribute(wxT("FV")));
        GridCoroFaces->SetCellValue(7, i, voice->GetAttribute(wxT("L")));
        GridCoroFaces->SetCellValue(8, i, voice->GetAttribute(wxT("MBP")));
        GridCoroFaces->SetCellValue(9, i, voice->GetAttribute(wxT("O")));
        GridCoroFaces->SetCellValue(10, i, voice->GetAttribute(wxT("rest")));
        GridCoroFaces->SetCellValue(11, i, voice->GetAttribute(wxT("U")));
        GridCoroFaces->SetCellValue(12, i, voice->GetAttribute(wxT("WQ")));
    }
}

//TODO: use Save for Delete as well?
void xLightsFrame::OnButton_CoroGroupDeleteClick(wxCommandEvent& event)
{
    wxString grpname;
    if (!GetGroupName(grpname)) return;
    debug(10, "CoroGroupDeleteClick: delete group '%s' from xmldoc", (const char*)grpname.c_str());
    wxXmlNode* CoroFaces = FindNode(pgoXml.GetRoot(), wxT("corofaces"), wxT("name"), wxEmptyString, true);
    wxXmlNode* node = FindNode(CoroFaces, "coro", wxT("name"), grpname, false);
    if (node) CoroFaces->RemoveChild(node); //delete group
    Choice_PgoGroupName->SetSelection(0); //"choose"
    GridCoroFaces->ClearGrid();
}

//NOTE: doesn't save
void xLightsFrame::OnButton_CoroGroupClearClick(wxCommandEvent& event)
{
    GridCoroFaces->ClearGrid();
}
