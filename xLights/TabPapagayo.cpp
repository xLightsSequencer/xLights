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

//cut down on verbosity:
//use for fatal errors only
#define retmsg(msg)  \
{ \
    wxMessageBox(msg, _("Papagayo Error")); \
    return; \
}

#define warnmsg(msg)  warnings += msg + "\n"



void xLightsFrame::PapagayoError(const wxString& msg)
{
    wxMessageBox(msg, _("Error"), wxOK | wxICON_EXCLAMATION);
}



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

    filename=TextCtrl_papagayo_output_filename->GetValue();
    int pgofile_status=write_pgo_header(voices.size(),filename);

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
                debug_msg += wxString::Format(_("\t\tword[%d/%d] '%s'\n"), word_it - phrase_it->words.begin(), phrase_it->words.size(), word_it->name.c_str());
//              std::vector<PhonemeInfo>& phonemes = word_it->phonemes;
                for (auto phoneme_it = word_it->phonemes.begin(); phoneme_it != word_it->phonemes.end(); ++phoneme_it)
                {
                    debug_msg += wxString::Format(_("\t\t\tV%d phoneme[%d/%d] '%s': call routine(start_frame %d, end_frame %d, phoneme '%s')\n"), (voice_it - voices.begin()),
                                                  phoneme_it - word_it->phonemes.begin(), word_it->phonemes.size(), phoneme_it->name.c_str(), phoneme_it->start_frame, phoneme_it->end_frame, phoneme_it->name);
//                  call routine(voice_it,phoneme_it->start_frame, phoneme_it->end_frame, phoneme_it->name);
                    if(pgofile_status) AutoFace((voice_it - voices.begin()),filename, phoneme_it->start_frame,phoneme_it->end_frame,
                                                 phoneme_it->name, word_it->name.c_str());

                    //if (xout) xout->alloff();

                }
            }
        }
    }
    if(pgofile_status)  write_pgo_footer(voices.size(),filename);
    wxMessageBox(debug_msg, _("Papagayo Debug"));
}
// int Voice,int MaxVoice,int StartFrame, int EndFrame,wxString Phoneme
int xLightsFrame::write_pgo_header(int MaxVoices,const wxString& filename)
{

    // wxFile f;
    wxFile f(filename);
    int voice;
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
        for(voice=1; voice<=MaxVoices; voice++)
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
    int voice;
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
    seconds = (double) start_frame * 0.050;
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
// scm, we wont be using the wav file in *.pgo files.
// no need to check for it
//   if (soundpath.IsEmpty() || !wxFileExists(soundpath)) warnmsg(wxString::Format(_("Sound file '%s' is missing"), soundpath.c_str()));
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
        if (!numphrases) retmsg(wxString::Format(_("Invalid file @line %d ('%s' phrases for %s)"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str(), desc.c_str()));
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
            if (!numwords) retmsg(wxString::Format(_("Invalid file @line %d ('%s' words for %s)"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str(), desc.c_str()));
            for (int w = 1; w <= numwords; ++w)
            {
                newphrase.words.emplace_back();
                WordInfo& newword = newphrase.words.back();
                wxStringTokenizer wtkz(readline(), " ");
                newword.name = wtkz.GetNextToken();
                if (newword.name.empty())
                {
                    warnmsg(wxString::Format(_("Missing word# %d of %d for %s"), w, numwords, desc.c_str()));
                    newphrase.words.pop_back(); //get rid of unneeded entry
                    break;
                }
                wtkz.GetNextToken(); //start frame TODO: do we need to save this?
                wxString endfr = wtkz.GetNextToken(); //end frame TODO: do we need to save this?
//                voices.back().phrases.push_back(newphrase);
                wxString syllcount = wtkz.GetNextToken();
                wxString junk = wtkz.GetNextToken();
//                wxMessageBox(wxString::Format(_("word '%s', end fr %s, #syll %s, junk %s"), newword.name.c_str(), endfr.c_str(), syllcount.c_str(), junk.c_str()));
                desc = wxString::Format(_("voice# %d, phrase %d, word %d '%s' @line %d"), v, phr, w, newword.name, PapagayoFileInfo.linenum);
                if (!junk.empty()) warnmsg(wxString::Format(_("Ignoring junk '%s' at end of %s @line %d"), junk.c_str(), desc.c_str(), PapagayoFileInfo.linenum));
                debug(10, (const char*)desc.c_str());

                int end_frame = number.Matches(endfr)? wxAtoi(endfr): 0;
                if (!end_frame) warnmsg(wxString::Format(_("Invalid file @line %d ('%s' end frame for %s)"), PapagayoFileInfo.linenum, endfr.c_str(), desc.c_str()));

                int numsylls = number.Matches(syllcount)? wxAtoi(syllcount): 0;
                if (!numsylls) retmsg(wxString::Format(_("Invalid file @line %d ('%s' phonemes for %s)"), PapagayoFileInfo.linenum, syllcount.c_str(), desc.c_str()));
                for (int syll = 1; syll <= numsylls; ++syll)
                {
                    newword.phonemes.emplace_back();
                    PhonemeInfo& newsyll = newword.phonemes.back();
                    wxStringTokenizer stkz = wxStringTokenizer(readline(), " ");
                    wxString stframe = stkz.GetNextToken();
                    newsyll.name = stkz.GetNextToken();
//                    wxMessageBox(wxString::Format(_("get syll %s, st fr %s, syll %s, allowed in %s? %d"), PapagayoFileInfo.linebuf, stframe, _(",") + newsyll.name + _(","), AllowedPhonemes, AllowedPhonemes.find(_(",") + newsyll.name + _(","))));
                    if (newsyll.name.empty() || (AllowedPhonemes.find(_(",") + newsyll.name + _(",")) == -1))
                    {
                        retmsg(wxString::Format(_("Missing phoneme# %d of %d for %s"), syll, numsylls, desc.c_str()));
                        newword.phonemes.pop_back(); //get rid of unneeded entry
                        break;
                    }
                    /*wxString*/ junk = stkz.GetNextToken();
                    desc = wxString::Format(_("voice# %d, phrase %d, word %d, phoneme %d '%s' @line %d"), v, phr, w, syll, newsyll.name, PapagayoFileInfo.linenum);
                    if (!junk.empty()) warnmsg(wxString::Format(_("Ignoring junk '%s' at end of %s @line %d"), junk.c_str(), desc.c_str(), PapagayoFileInfo.linenum));
                    debug(10, (const char*)desc.c_str());

                    newsyll.start_frame = number.Matches(stframe)? wxAtoi(stframe): -1;
                    if (newsyll.start_frame == -1) retmsg(wxString::Format(_("Invalid file @line %d ('%s' start frame for %s)"), PapagayoFileInfo.linenum, stframe.c_str(), desc.c_str()));
                    newsyll.end_frame = end_frame; //assume end of phrase until another phoneme is found
                    if (syll > 1) (&newsyll)[-1].end_frame = newsyll.start_frame; //don't overlap?
                }
            }
        }
    }

    if (!warnings.empty()) wxMessageBox(warnings, _("Papagayo Warning"));
    debug(3, "file loaded");
}
