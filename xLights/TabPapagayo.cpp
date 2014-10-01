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
#include <algorithm> //sort
#include <limits> //max int, etc.

//#define WANT_DEBUG_IMPL
//#define WANT_DEBUG  -99 //unbuffered in case app crashes
//#include "djdebug.cpp"
#ifndef debug_function //dummy defs if debug cpp not included above
 #define debug(level, ...)
 #define debug_more(level, ...)
 #define debug_function(level)
#endif

//cut down on mem allocs outside debug() when WANT_DEBUG is off:
#ifdef WANT_DEBUG
#define IFDEBUG(stmt)  stmt
#else
#define IFDEBUG(stmt)
#endif // WANT_DEBUG

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


const wxString InactiveIndicator = "?";
static const wxString LastMode = wxT("last_mode");
static const wxString AutoFadeElement = wxT("auto_fade_element");
static const wxString AutoFadeAll = wxT("auto_fade_all");
static const wxString DelayElement = wxT("delay_element");
static const wxString DelayAll = wxT("delay_all");
static const wxString EyesBlink = wxT("eyes_blink");
static const wxString EyesLR = wxT("eyes_lr");
static const wxString Yes = wxT("Y");
static const wxString No = wxT("N");
static const wxString Name = wxT("name");


static wxString NoInactive(wxString name)
{
    return name.StartsWith(InactiveIndicator)? name.substr(InactiveIndicator.size()): name;
}


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

static const wxString SelectionHint = "(choose)", CreationHint = "(add new)", NoneHint = "(no choices)";


// Pgo notebook pages
#define AUTOTAB 0
#define COROTAB 1
#define IMAGETAB 2
#define MOVIETAB 3

//grid row#s:
//TODO: load these dynamically?
#define Model_Row  0
#define Outline_Row  1
#define AI_Row  2
#define E_Row  3
#define etc_Row  4
#define FV_Row  5
#define L_Row  6
#define MBP_Row  7
#define O_Row  8
#define rest_Row  9
#define U_Row  10
#define WQ_Row  11
#define Eyes_open_Row  12
#define Eyes_closed_Row  13
#define Eyes_left_Row  14
#define Eyes_right_Row  15
#define Eyes_up_Row  16
#define Eyes_down_Row  17



void xLightsFrame::OnButton_pgo_filenameClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Papagayo File", "", "", "", "Papagayo files (*.pgo)|*.pgo", wxFD_OPEN );
//  wxString filename = "this5.pgo";
    if (!filename.IsEmpty()) TextCtrl_pgo_filename->SetValue(filename);
    LoadPapagayoFile(filename);
//    LoadPgoSettings(); //reload in case models changed
}


void xLightsFrame::OnButton_PgoStitchClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Another Papagayo File", "", "", "", "Papagayo files (*.pgo)|*.pgo", wxFD_OPEN );
//  wxString filename = "this5.pgo";
    if (filename.IsEmpty()) return;

//    TextCtrl_pgo_filename->SetValue(filename);
    wxString stitch_frame;
    if (!EffectTreeDialog::PromptForName(this, &stitch_frame, wxT("Enter continuation frame#"), wxT("Frame# must not be empty"))) return;
    int start_frame;
    start_frame = wxAtoi(stitch_frame);
    if (!start_frame) return;
    LoadPapagayoFile(filename, start_frame);
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

class InfoChain
{
public:
    VoiceInfo* v;
    PhraseInfo* p;
    WordInfo* w;
    PhonemeInfo* q;
};
std::vector<std::pair<int, InfoChain>> phoemes_by_start_frame;

static int single_delay, all_delay, eyes_delay; //auto-fade or eye movement frame counts

void xLightsFrame::OnButtonStartPapagayoClick(wxCommandEvent& event)
{
    ButtonStartPapagayo->Enable(false);
    wxString OutputFormat = ChoiceOutputFormat->GetStringSelection();
    TextCtrlConversionStatus->Clear();
    ButtonStartPapagayo->Enable(true);

    all_delay = 0;
    single_delay = 0;
    eyes_delay = 0;
    if (CheckBox_AutoFadePgoElement->GetValue())
    {
        double val;
        if (!TextCtrl_AutoFadePgoElement->GetValue().ToDouble(&val))
            retmsg(wxString::Format("Invalid single element delay value: '%s'", TextCtrl_AutoFadePgoElement->GetValue()));
        single_delay = val * 20; //#sec => #frames
    }
    if (CheckBox_AutoFadePgoAll->GetValue())
    {
        double val;
        if (!TextCtrl_AutoFadePgoAll->GetValue().ToDouble(&val))
            retmsg(wxString::Format("Invalid all element delay value: '%s'", TextCtrl_AutoFadePgoAll->GetValue()));
        all_delay = val * 20; //#sec => #frames
    }
    if (CheckBox_AutoFadePgoAll->GetValue())
    {
        double val;
        if (!TextCtrl_AutoFadePgoAll->GetValue().ToDouble(&val))
            retmsg(wxString::Format("Invalid all element delay value: '%s'", TextCtrl_AutoFadePgoAll->GetValue()));
        all_delay = val * 20; //#sec => #frames
    }
    if (CheckBox_CoroEyesRandomBlink->GetValue() || CheckBox_CoroEyesRandomLR->GetValue()) eyes_delay = 100; //rand() % 30; //TODO: adjust value?
    debug(20, "auto-fade: single %d, all %d, eyes move? %d", single_delay, all_delay, eyes_delay);

//example code to iterate thru the data:
    wxString debug_msg, filename;
//mingw32-make -f makefile.gcc MONOLITHIC=1 SHARED=1 UNICODE=1 CXXFLAGS="-std=gnu++0x" BUILD=release
    filename = TextCtrl_papagayo_output_filename->GetValue();
//only open/close file once for better performance:
    wxFile f(filename);
    if (!f.Create(filename,true) || !f.IsOpened()) retmsg(wxString::Format("write_pgo_header: Unable to create file %s. Error %d\n",filename,f.GetLastError()));
    int pgofile_status = write_pgo_header(f, voices.size()); //, filename);

    int numwr = 0;
    phoemes_by_start_frame.clear();
    for (auto voice_it = voices.begin(); voice_it != voices.end(); ++voice_it)
    {
        IFDEBUG(debug_msg += wxString::Format(_("voice[%d/%d] '%s'\n"), voice_it - voices.begin(), voices.size(), voice_it->name.c_str()));
//        std::vector<PhraseInfo>& phrases = voice_it->phrases;
        for (auto phrase_it = voice_it->phrases.begin(); phrase_it != voice_it->phrases.end(); ++phrase_it)
        {
            IFDEBUG(debug_msg += wxString::Format(_("\tphrase[%d/%d] '%s'\n"), phrase_it - voice_it->phrases.begin(), voice_it->phrases.size(), phrase_it->name.c_str()));
//            std::vector<WordInfo>& words = phrase_it->words;
            for (auto word_it = phrase_it->words.begin(); word_it != phrase_it->words.end(); ++word_it)
            {
                StatusBar1->SetStatusText(wxString::Format("Writing pgo xml: voice [%d/%d] '%s', phrase[%d/%d] '%s', word[%d/%d] '%s'", voice_it - voices.begin(), voices.size(), voice_it->name.c_str(), phrase_it - voice_it->phrases.begin(), voice_it->phrases.size(), phrase_it->name.c_str(), word_it - phrase_it->words.begin(), phrase_it->words.size(), word_it->name.c_str()));
                IFDEBUG(debug_msg += wxString::Format(_("\t\tword[%d/%d] '%s'\n"), word_it - phrase_it->words.begin(), phrase_it->words.size(), word_it->name.c_str()));
//              std::vector<PhonemeInfo>& phonemes = word_it->phonemes;
                for (auto phoneme_it = word_it->phonemes.begin(); phoneme_it != word_it->phonemes.end(); ++phoneme_it)
                {
                    IFDEBUG(debug_msg += wxString::Format(_("\t\t\tV%d phoneme[%d/%d] '%s': call routine(start_frame %d, end_frame %d, phoneme '%s')\n"), (voice_it - voices.begin()),
                                                  phoneme_it - word_it->phonemes.begin(), word_it->phonemes.size(), phoneme_it->name.c_str(), phoneme_it->start_frame, phoneme_it->end_frame, phoneme_it->name));
//                  call routine(voice_it,phoneme_it->start_frame, phoneme_it->end_frame, phoneme_it->name);
//                    if (pgofile_status) AutoFace(f, voice_it - voices.begin(), phoneme_it->start_frame,phoneme_it->end_frame,
//                                                    phoneme_it->name, word_it->name.c_str());
                    if (pgofile_status) AutoFace(f, phoneme_it->start_frame, &*voice_it, &*phrase_it, &*word_it, &*phoneme_it);
                    ++numwr;

                    //if (xout) xout->alloff();

                }
            }
        }
    }
    StatusBar1->SetStatusText(wxString::Format("Wrote pgo xml: %d entries", numwr));
    if (pgofile_status) write_pgo_footer(f, voices.size()); //,filename);
    f.Close();
//    IFDEBUG(wxMessageBox(debug_msg, _("Papagayo Debug")));
}

static void blank(wxFile& f)
{
    f.Write("    <td Protected=\"0\">Color Wash,None,Effect 1,ID_CHECKBOX_LayerMorph=0,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0,E1_SLIDER_ColorWash_Count=1,E1_CHECKBOX_ColorWash_HFade=0,E1_CHECKBOX_ColorWash_VFade=0,E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=0,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=0,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=1,E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0</td>\n");
}

// int Voice,int MaxVoice,int StartFrame, int EndFrame,wxString Phoneme
int xLightsFrame::write_pgo_header(wxFile& f, int MaxVoices)
{
//TODO: rewrite to use XmlDoc?  perf not too bad as is
    // wxFile f;
//    wxFile f(filename);
    // wxString filename=wxString::Format(("C:\\Vixen.2.1.1\\Sequences\\z.xml"));

    // retmsg(_("Filename: "+filename));
//    if (!f.Create(filename,true))
//    {
        //  retmsg(_("Unable to create file: "+filename));
//        wxMessageBox(wxString::Format("write_pgo_header: Unable to create file %s. Error %d\n",filename,f.GetLastError()));
//        return 0;
//    }


    //buff += wxString::Format("%d ",(*dataBuf)[seqidx]);

    //    buff += wxString::Format("\n");
    //    f.Write(buff);

// f.Write("\t\t\t</channels>\n");
//    if(f.IsOpened())
//    {
        f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        f.Write("<xsequence BaseChannel=\"0\" ChanCtrlBasic=\"0\" ChanCtrlColor=\"0\">\n");
        f.Write("<tr>\n");
        f.Write("    <td>Start Time</td>\n");
        f.Write("    <td>Label</td>\n");
        for(int voice=1; voice<=MaxVoices; voice++)
        {
            if (voice > GridCoroFaces->GetCols())
                f.Write(wxString::Format("    <td>VOICE%d</td>\n",voice)); //write dummy value
            else
            {
                wxString voice_name = GridCoroFaces->GetCellValue(Model_Row, voice - 1);
                f.Write(wxString::Format("    <td>%s</td>\n", NoInactive(voice_name))); //use actual voice model name
            }
        }
        f.Write("</tr>\n");


        f.Write("<tr>\n");
        f.Write("    <td Protected=\"0\">0.000</td>\n");
        f.Write("    <td Protected=\"0\">Blank</td>\n");
        for (int voice=1; voice<=MaxVoices; voice++) //use actual #voices
            blank(f);
        f.Write("</tr>\n");
//        f.Close();
        return 1; // good exit
//    }
//    return 0;   // bad exit
}

//sort by increasing frame# order; used by sort()
static bool Sorter(const std::pair<int, InfoChain>& lhs, const std::pair<int, InfoChain>& rhs)
{
    if (lhs.first != rhs.first) return (lhs.first < rhs.first);
//secondary sort to help trap dups; NOTE: strings are not unique, so deref ptrs and use string compares; case-sensitive is okay since we just need a unique sort order
    if (lhs.second.v->name != rhs.second.v->name) { /*debug(80, "sorter: v cmp %d", (lhs.second.v->name < rhs.second.v->name))*/; return (lhs.second.v->name < rhs.second.v->name); }
    if (lhs.second.p->name != rhs.second.p->name) { /*debug(80, "sorter: p cmp %d", (lhs.second.p->name < rhs.second.p->name))*/; return (lhs.second.p->name < rhs.second.p->name); }
    if (lhs.second.w->name != rhs.second.w->name) { /*debug(80, "sorter: w cmp %d", (lhs.second.w->name < rhs.second.w->name))*/; return (lhs.second.w->name < rhs.second.w->name); }
    if (lhs.second.q->name != rhs.second.q->name) { /*debug(80, "sorter: q cmp %d", (lhs.second.q->name < rhs.second.q->name))*/; return (lhs.second.q->name < rhs.second.q->name); }
//    debug(80, "sorter: all =");
    return true; //dup entry; no need to change position
}

void xLightsFrame::write_pgo_footer(wxFile& f, int MaxVoices)
{
//TODO: rewrite to use XmlDoc?  perf not too bad as is
#if 1 //sort and write merged pgo events
//NOTE: dummy entry at end ensures that list is not empty
    debug(10, "sort %d frames", phoemes_by_start_frame.size());
    sort(phoemes_by_start_frame.begin(), phoemes_by_start_frame.end(), Sorter); //preserve array indexes and just sort tags
    debug(10, "... sorted %d frames, first frame %d, last frame %d", phoemes_by_start_frame.size(), phoemes_by_start_frame[0].first, phoemes_by_start_frame[std::max<int>(phoemes_by_start_frame.size() - 2, 0)].first);
    std::pair<int, InfoChain> eof;
    eof.first = std::numeric_limits<int>::max(); //MAXINT; //dummy entry to force last entry to be written
    eof.second.v = &voices[0];
    eof.second.p = &voices[0].phrases[0];
    eof.second.w = &voices[0].phrases[0].words[0];
    eof.second.q = &voices[0].phrases[0].words[0].phonemes[0];
    phoemes_by_start_frame.push_back(eof);
    int prev_frame = -1, err_frame = -1;
//TODO: do we need to keep end_frame?
    size_t numfr = 0;
    wxString frame_desc, shorter_desc;
    wxString errors;
    wxString frame_phonemes[voices.size()], frame_eyes[voices.size()];
    int all_fade_frame = std::numeric_limits<int>::max(), eyes_move_frame = std::numeric_limits<int>::max(); //, single_fade_frame = std::numeric_limits<int>::max(); //auto-fade frame counts
    std::unordered_map<std::string, int> single_fade_frame; //per-phoneme deadline
    if (eyes_delay) eyes_move_frame = rand() % eyes_delay;
#if 0 //obsolete
    struct
    {
        std::unordered_map<std::string, std::string> phon[4]; //(X,Y) values to use for face parts for each voice
        std::string outl[4], eyes[4];
    } parsed, oldxy;
    for (size_t v = 0; v < GridCoroFaces->GetCols(); ++v)
    {
        wxPoint xy;
        wxString str;
//TODO: make this code more compact
        if (ModelClass::ParseFaceElement(str = GridCoroFaces->GetCellValue(Outline_Row, v), &xy)) oldxy.outl[v] = wxString::Format(wxT("%d:%d"), xy.x, xy.y); if (!str.empty()) parsed.outl[v] = "+" + str;
        if (ModelClass::ParseFaceElement(str = GridCoroFaces->GetCellValue(Eyes_open_Row, v), &xy)) oldxy.eyes[v] = wxString::Format(wxT("%d:%d"), xy.x, xy.y); if (!str.empty()) parsed.eyes[v] = "+" + str;
        if (ModelClass::ParseFaceElement(str = GridCoroFaces->GetCellValue(Eyes_closed_Row, v), &xy)) oldxy.eyes[v] += wxString::Format(wxT(":%d:%d"), xy.x, xy.y); if (!str.empty() && (parsed.eyes[v].find(str) == std::string::npos)) parsed.eyes[v] += "+" + str;
        if (ModelClass::ParseFaceElement(GridCoroFaces->GetCellValue(AI_Row, v), &xy)) oldxy.phon[v]["AI"] = wxString::Format(wxT("%d:%d"), xy.x, xy.y); if (!str.empty()) parsed.phon[v]["AI"] = "+" + str;
        if (ModelClass::ParseFaceElement(GridCoroFaces->GetCellValue(E_Row, v), &xy)) oldxy.phon[v]["E"] = wxString::Format(wxT("%d:%d"), xy.x, xy.y); if (!str.empty()) parsed.phon[v]["E"] = "+" + str;
        if (ModelClass::ParseFaceElement(GridCoroFaces->GetCellValue(etc_Row, v), &xy)) oldxy.phon[v]["etc"] = wxString::Format(wxT("%d:%d"), xy.x, xy.y); if (!str.empty()) parsed.phon[v]["etc"] = "+" + str;
        if (ModelClass::ParseFaceElement(GridCoroFaces->GetCellValue(FV_Row, v), &xy)) oldxy.phon[v]["FV"] = wxString::Format(wxT("%d:%d"), xy.x, xy.y); if (!str.empty()) parsed.phon[v]["FV"] = "+" + str;
        if (ModelClass::ParseFaceElement(GridCoroFaces->GetCellValue(L_Row, v), &xy)) oldxy.phon[v]["L"] = wxString::Format(wxT("%d:%d"), xy.x, xy.y); if (!str.empty()) parsed.phon[v]["L"] = "+" + str;
        if (ModelClass::ParseFaceElement(GridCoroFaces->GetCellValue(MBP_Row, v), &xy)) oldxy.phon[v]["MBP"] = wxString::Format(wxT("%d:%d"), xy.x, xy.y); if (!str.empty()) parsed.phon[v]["MBP"] = "+" + str;
        if (ModelClass::ParseFaceElement(GridCoroFaces->GetCellValue(O_Row, v), &xy)) oldxy.phon[v]["O"] = wxString::Format(wxT("%d:%d"), xy.x, xy.y); if (!str.empty()) parsed.phon[v]["O"] = "+" + str;
        if (ModelClass::ParseFaceElement(GridCoroFaces->GetCellValue(rest_Row, v), &xy)) oldxy.phon[v]["rest"] = wxString::Format(wxT("%d:%d"), xy.x, xy.y); if (!str.empty()) parsed.phon[v]["rest"] = "+" + str;
        if (ModelClass::ParseFaceElement(GridCoroFaces->GetCellValue(U_Row, v), &xy)) oldxy.phon[v]["U"] = wxString::Format(wxT("%d:%d"), xy.x, xy.y); if (!str.empty()) parsed.phon[v]["U"] = "+" + str;
        if (ModelClass::ParseFaceElement(GridCoroFaces->GetCellValue(WQ_Row, v), &xy)) oldxy.phon[v]["WQ"] = wxString::Format(wxT("%d:%d"), xy.x, xy.y); if (!str.empty()) parsed.phon[v]["WQ"] = "+" + str;
    }
#endif // 0
    for (auto it = phoemes_by_start_frame.begin(); it != phoemes_by_start_frame.end(); ++it) //write out sorted entries
    {
        std::string phkey = (const char*)it->second.q->name.c_str();
        phkey += (const char*)it->second.v->name.c_str(); //tag phoneme with voice name so it can have different delay deadline
        debug(10, "compare frame %d v 0x%x '%s', phr 0x%x '%s', w 0x%x '%s', phon 0x%x '%s' vs. prev frame %d = %d v 0x%x '%s', phr 0x%x '%s', w 0x%x '%s', phon 0x%x '%s', auto-fade all %d, element %d", it->first, it->second.v, (const char*)it->second.v->name.c_str(), it->second.p, (const char*)it->second.p->name.c_str(), it->second.w, (const char*)it->second.w->name.c_str(), it->second.q, (const char*)it->second.q->name.c_str(), prev_frame, (it != phoemes_by_start_frame.begin())? (it - 1)->first: -2, (it != phoemes_by_start_frame.begin())? (it - 1)->second.v: 0, (it != phoemes_by_start_frame.begin())? (const char*)(it - 1)->second.v->name.c_str(): "(none)", (it != phoemes_by_start_frame.begin())? (it - 1)->second.p: 0, (it != phoemes_by_start_frame.begin())? (const char*)(it - 1)->second.p->name.c_str(): "(none)", (it != phoemes_by_start_frame.begin())? (it - 1)->second.w: 0, (it != phoemes_by_start_frame.begin())? (const char*)(it - 1)->second.w->name.c_str(): "(none)", (it != phoemes_by_start_frame.begin())? (it - 1)->second.q: 0, (it != phoemes_by_start_frame.begin())? (const char*)(it - 1)->second.q->name.c_str(): "(none)", all_fade_frame, (single_fade_frame.find(phkey) != single_fade_frame.end())? single_fade_frame[phkey]: 0);
        if (it != phoemes_by_start_frame.begin())
            if (Sorter(*(it - 1), *it) && Sorter(*it, *(it - 1))) continue; //skip duplicates; reuse Sorter lggic: lhs >= rhs && rhs >= lhs ==> lhs == rhs
        if (it->first != prev_frame)
        {
            if (prev_frame != -1) //flush prev frame
            {
                frame_desc = frame_desc.substr(2); //remove leading ", "
                if (!shorter_desc.empty()) frame_desc = shorter_desc; //reduce verbosity in grid
                else if (frame_desc.Find(',') == wxNOT_FOUND) //only one voice; remove voice name to cut down on verbosity
                    frame_desc = frame_desc.AfterFirst(':');
                debug(10, "footer: flush fr %d '%s'", prev_frame, (const char*)frame_desc.c_str());
                double seconds = (double) prev_frame * 0.050;  // assume 20fps fpr the papagayo file. not a good assumption
                f.Write(wxString::Format("<tr frame=\"%d\">\n", prev_frame)); //include a little debug info here (frame#)
                f.Write(wxString::Format("   <td Protected=\"0\">%7.3f</td>\n", seconds));
                f.Write(wxString::Format("   <td Protected=\"0\">%s</td>\n", frame_desc));
                for (int voice = 0; voice < voices.size(); voice++) //use actual #voices
                    if (!frame_phonemes[voice].empty())
//separated X:Y for phoneme, outlines, eyes
//                        f.Write(wxString::Format("   <td Protected=\"0\">CoroFaces,None,Effect 1,ID_CHECKBOX_LayerMorph=0,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0,E1_CHOICE_CoroFaces_Phoneme=%s,E1_TEXTCTRL_X_Y=%s,E1_TEXTCTRL_Outline_X_Y=%s,E1_TEXTCTRL_Eyes_X_Y=%s,E1_CHECKLISTBOX_CoroFaceElements=%s,E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=1,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=1,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=0,E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0</td>\n", frame_phonemes[voice], oldxy.phon[voice][std::string(frame_phonemes[voice].mb_str())], oldxy.outl[voice], oldxy.eyes[voice], wxEmptyString)); //"1: 91# @I1-W30+2: 53# @G5-W14+3: 20# @O9-Q17+4: 35# @D15-V25"));
//parsed (X,Y)
//                        f.Write(wxString::Format("   <td Protected=\"0\">CoroFaces,None,Effect 1,ID_CHECKBOX_LayerMorph=0,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0,E1_CHOICE_CoroFaces_Phoneme=%s,E1_TEXTCTRL_X_Y=%s,E1_TEXTCTRL_Outline_X_Y=%s,E1_TEXTCTRL_Eyes_X_Y=%s,E1_CHECKLISTBOX_CoroFaceElements=%s,E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=1,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=1,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=0,E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0</td>\n", frame_phonemes[voice], oldxy.phon[voice][std::string(frame_phonemes[voice].mb_str())], oldxy.outl[voice], oldxy.eyes[voice], (parsed.outl[voice] + parsed.eyes[voice] + parsed.phon[voice][std::string(frame_phonemes[voice].mb_str())]).substr(1))); //"1: 91# @I1-W30+2: 53# @G5-W14+3: 20# @O9-Q17+4: 35# @D15-V25"));
//phoneme name or eye position
                        f.Write(wxString::Format("   <td Protected=\"0\">CoroFaces,None,Effect 1,ID_CHECKBOX_LayerMorph=0,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0,E1_CHOICE_CoroFaces_Phoneme=%s,E1_CHOICE_CoroFaces_Eyes=%s,E1_CHECKBOX_CoroFaces_Outline=%s,E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=1,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=1,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=0,E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0</td>\n", frame_phonemes[voice], frame_eyes[voice], "Y"));
                    else
                        blank(f); //need placeholder in grid (xLights incorrectly handles missing column data)
                f.Write("</tr>\n");
                ++numfr;
            }
            if (all_fade_frame < it->first)
            {
                debug(10, "auto-fade all elements: next frame %d, deadline was %d (%7.3f sec)", it->first, all_fade_frame, all_fade_frame * 0.050);
                f.Write(wxString::Format("<tr frame=\"%d\">\n", all_fade_frame)); //include a little debug info here (frame#)
                f.Write(wxString::Format("   <td Protected=\"0\">%7.3f</td>\n", all_fade_frame * 0.050));
                f.Write("   <td Protected=\"0\">auto-fade all</td>\n");
                for (int voice = 0; voice < voices.size(); voice++) //use actual #voices
                    blank(f); //need placeholder in grid (xLights incorrectly handles missing column data)
                f.Write("</tr>\n");
                ++numfr;
            }
//TODO: sort this if perf becomes a problem; perf not too bad with simple loop so just leave it for now
            for (auto phit = single_fade_frame.begin(); phit != single_fade_frame.end(); ++phit)
                if (phit->second < it->first)
                {
                    debug(10, "auto-fade '%s' element: next frame %d, deadline was %d", it->first, all_fade_frame, phit->second);
                    f.Write(wxString::Format("<tr frame=\"%d\">\n", phit->second)); //include a little debug info here (frame#)
                    f.Write(wxString::Format("   <td Protected=\"0\">%7.3f</td>\n", phit->second * 0.050));
                    f.Write(wxString::Format("   <td Protected=\"0\">'%s' auto-fade single</td>\n", phit->first));
                    for (int voice = 0; voice < voices.size(); voice++) //use actual #voices
                        blank(f); //need placeholder in grid (xLights incorrectly handles missing column data)
                    f.Write("</tr>\n");
                    ++numfr;
                }
            debug(10, "footer: start new fr %d", it->first);
            prev_frame = it->first; //start new frame
            frame_desc.clear();
            shorter_desc = wxString::Format(wxT("'%s':%s"), it->second.w->name, it->second.q->name); //word:phoneme
            for (int voice = 0; voice < voices.size(); voice++) //use actual #voices
                frame_phonemes[voice].clear();
        }
//merge with current frame
//        debug(10, "here2: 0x%x vs v[0] 0x%x == %d", it->second.v, &voices[0], it->second.v - &voices[0]);
//        debug(10, "  0x%x vs phr[0] 0x%x == %d", it->second.p, &it->second.v->phrases[0], it->second.p - &it->second.v->phrases[0]);
//        debug(10, "  0x%x vs w[0] 0x%x == %d", it->second.w, &it->second.p->words[0], it->second.w - &it->second.p->words[0]);
//        debug(10, "  0x%x vs phon[0] 0x%x == %d", it->second.q, &it->second.w->phonemes[0], it->second.q - &it->second.w->phonemes[0]);
        frame_desc += wxString::Format(wxT(", '%s':'%s':%s"), it->second.v->name, it->second.w->name, it->second.q->name); //voice:word:phoneme
        wxString new_short_desc = wxString::Format(wxT("'%s':%s"), it->second.w->name, it->second.q->name); //word:phoneme
        if (new_short_desc != shorter_desc) shorter_desc.clear(); //can't use shorter desc (word or phoneme varies)
        debug(10, "footer: merge fr %d '%s'", it->first, (const char*)frame_desc.c_str());
        if (!frame_phonemes[it->second.v - &voices[0]].empty() && (err_frame != it->first))
        {
            errors += wxString::Format(wxT("Duplicate phoneme for '%s' in frame %d (%7.3f sec)\n"), it->second.v->name, it->first, (double)it->first * 0.050);
            err_frame = it->first; //only report each frame 1x
        }
        frame_phonemes[it->second.v - &voices[0]] = it->second.q->name; //phoneme
        if (all_delay) all_fade_frame = it->first + all_delay; //set next deadline
        if (single_delay) single_fade_frame[phkey] = it->first + single_delay; //set per-phoneme deadline
//TODO: need to improve random (random start time but somewhat fixed duration)
//            eyes_move_frame = it->first + rand() % eyes_delay; //set next deadline
//            frame_eyes[it->second.v - &voices[0]] = CheckBox_CoroEyesRandomBlink->GetValue()? ((rand() % 6)? "Open": "Closed"): CheckBox_CoroEyesRandomLR->GetValue()? "Left": "Open"; //eyes
        if (it->first < eyes_move_frame) continue;
        if (CheckBox_CoroEyesRandomBlink->GetValue())
        {
            if (frame_eyes[it->second.v - &voices[0]] != "Closed")
            {
                frame_eyes[it->second.v - &voices[0]] == "Closed";
                eyes_move_frame = it->first + 10; //1/2 sec blink
            }
            else
            {
                frame_eyes[it->second.v - &voices[0]] == "Open";
                eyes_move_frame = it->first + rand() % eyes_delay; //set next deadline
            }
        }
        else if (CheckBox_CoroEyesRandomLR->GetValue())
        {
            if (frame_eyes[it->second.v - &voices[0]] == "Open")
                frame_eyes[it->second.v - &voices[0]] == "Left";
            else if (frame_eyes[it->second.v - &voices[0]] == "Left")
                frame_eyes[it->second.v - &voices[0]] == "Right";
            else
                frame_eyes[it->second.v - &voices[0]] == "Open";
            eyes_move_frame = it->first + rand() % eyes_delay; //set next deadline
        }
    }
#endif // 1
    // wxFile f;
//    wxFile f(filename);
//    if (!f.Open(filename,wxFile::write_append))
//    {
//        retmsg(_("Unable to open for append, file: "+filename));

//    }
//    f.SeekEnd(0);
    errors += wxString::Format(wxT("Frames written: %d"), numfr);
    wxMessageBox(errors, wxT("File written"));
    f.Write("</xsequence>\n");
//    f.Close();

}

#if 0 //huh??  this looks wrong!  needs to be sorted by interval and write all applicable voices, not sorted by voice/phrase/word/phoneme which leads to duplicate intervals
void xLightsFrame::AutoFace(wxFile& f, int MaxVoices,int start_frame,int end_frame,const wxString& phoneme,const wxString& word)
{
//TODO: rewrite to use XmlDoc?  perf not too bad as is
    wxString label;
    double seconds;
//    wxFile f;

    // retmsg(_("Filename: "+filename));
//    if (!f.Open(filename,wxFile::write_append))
//    {
//        retmsg(_("Unable to open for append, file: "+filename));
//    }
//    f.SeekEnd(0);
    label = "'" + word + "':'" + phoneme + "'";
    seconds = (double) start_frame * 0.050;  // assume 20fps fpr the papagayo file. not a good assumption
    f.Write("<tr>\n");
    f.Write(wxString::Format("   <td Protected=\"0\">%7.3f</td>\n",seconds));
    f.Write(wxString::Format("   <td Protected=\"0\">%s</td>\n",label));
    for (int voice=1; voice<=MaxVoices; voice++) //use actual #voices
        f.Write(wxString::Format("   <td Protected=\"0\">Faces,None,Effect 1,ID_CHECKBOX_LayerMorph=0,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0,E1_CHOICE_Faces_Phoneme=%s,E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=1,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=1,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=0,E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0</td>\n", phoneme));
    f.Write("</tr>\n");
}
#else //sort by interval
void xLightsFrame::AutoFace(wxFile& f, int start_frame, void* voice_ptr, void* phrase_ptr, void* word_ptr, void* phoneme_ptr)
{
    std::pair<int, InfoChain> newent;
    newent.first = start_frame; //voices[voice_inx].phrases[phrase_inx].words[word_inx].phonemes[phoneme_inx].start_frame; //sort key
    newent.second.v = (VoiceInfo*)voice_ptr;
    newent.second.p = (PhraseInfo*)phrase_ptr;
    newent.second.w = (WordInfo*)word_ptr;
    newent.second.q = (PhonemeInfo*)phoneme_ptr;
    phoemes_by_start_frame.push_back(newent); //build a list for sorting before writing to file
    debug(10, "AutoFace: fr %d, v# %d '%s', phr# %d '%s', w# %d '%s', phon# %d '%s'", start_frame, ((VoiceInfo*)voice_ptr) - &voices[0], (const char*)((VoiceInfo*)voice_ptr)->name.c_str(), ((PhraseInfo*)phrase_ptr) - &((VoiceInfo*)voice_ptr)->phrases[0], (const char*)((PhraseInfo*)phrase_ptr)->name.c_str(), ((WordInfo*)word_ptr) - &((PhraseInfo*)phrase_ptr)->words[0], (const char*)((WordInfo*)word_ptr)->name.c_str(), ((PhonemeInfo*)phoneme_ptr) - &((WordInfo*)word_ptr)->phonemes[0], (const char*)((PhonemeInfo*)phoneme_ptr)->name.c_str());
}
#endif // 1


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

void xLightsFrame::LoadPapagayoFile(const wxString& filename, int frame_offset /*= 0*/)
{
    wxString warnings;
//    if (!CachedCueFilename.CmpNoCase(filename)) { debug_more(2, ", no change"); return; } //no change
    if (!frame_offset) voices.clear(); //clean out prev file
    if (!wxFileExists(filename)) retmsg(wxString::Format(_("File '%s' does not exist."), filename));
    if (!PapagayoFileInfo.file.Open(filename)) retmsg(wxString::Format(_("Can't open file '%s'."), filename));
    debug(3, "read file '%s', frame offset %d", (const char*)filename.c_str(), frame_offset);

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

    int total_voices = 0, total_phrases = 0, total_words = 0, total_syllables = 0;
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
//        if (!numphrases) warnmsg(wxString::Format(_("Suspicious file @line %d ('%s' phrases for %s)"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str(), desc.c_str()));
        if (numphrases > 0) ++total_voices;

        VoiceInfo emptyvoice, *newvoice = &emptyvoice;
        for (auto it = voices.begin(); it != voices.end(); ++it)
            if (it->name == voicename)
            {
                debug(10, "found dup voice '%s', stitch? %d", (const char*)voicename.c_str(), frame_offset);
                if (!frame_offset) warnmsg(wxString::Format(_("Duplicate voice name: %s"), voicename.c_str()));
                else newvoice = &*it; //add to previous info
                break;
            }
        debug(10, "new voice '%s' had %d phrases", (const char*)voicename.c_str(), newvoice->phrases.size());
        newvoice->name = voicename;
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
//            if (!numwords) warnmsg(wxString::Format(_("Suspicious file @line %d ('%s' words for %s)"), PapagayoFileInfo.linenum, PapagayoFileInfo.linebuf.c_str(), desc.c_str()));
            if (numwords > 0) ++total_phrases;

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
//                if (!numsylls) warnmsg(wxString::Format(_("Suspicious file @line %d ('%s' phonemes for %s)"), PapagayoFileInfo.linenum, syllcount.c_str(), desc.c_str()));
                if (numsylls > 0) ++total_words;

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
                    ++total_syllables;

                    newsyll.start_frame = number.Matches(stframe)? wxAtoi(stframe): -1;
                    if (newsyll.start_frame == -1) retmsg(wxString::Format(_("Invalid file @line %d ('%s' start frame for %s)"), PapagayoFileInfo.linenum, stframe.c_str(), desc.c_str()));
                    newsyll.end_frame = end_frame; //assume end of phrase until another phoneme is found
                    newsyll.start_frame += frame_offset; //stitch
                    newsyll.end_frame += frame_offset;
                    if (syll > 1) (&newsyll)[-1].end_frame = newsyll.start_frame; //don't overlap?
                    newword.phonemes.push_back(newsyll);
                }
                newphrase.words.push_back(newword);

            }
            newvoice->phrases.push_back(newphrase);

        }
        if (newvoice == &emptyvoice) voices.push_back(*newvoice);
    }
    if (!readline().empty()) warnmsg(wxString::Format(_("Ignoring junk at eof ('%s' found on line %d)"), PapagayoFileInfo.linebuf.c_str(), PapagayoFileInfo.linenum));
    PapagayoFileInfo.file.Close();

    if (!warnings.empty()) wxMessageBox(warnings, _("Papagayo Warning"));
    StatusBar1->SetStatusText(wxString::Format(wxT("Pgo voices loaded: %d, phrases: %d, words: %d, syllables: %d"), total_voices, total_phrases, total_words, total_syllables));
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
    void AddChild(myXmlNode*& child)
    {
        wxXmlNode::AddChild(child);
    };
    bool RemoveChild (myXmlNode* child)
    {
        return wxXmlNode::RemoveChild(child);
    }
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
    wxString last_mode = pgoXml.GetRoot()->GetAttribute(LastMode);
//    SetChoicebook(NotebookPgoParms, last_mode);
    for (int i = 0; i < NotebookPgoParms->GetPageCount(); ++i)
        if (last_mode == NotebookPgoParms->GetPageText(i))
        {
            debug(10, "LoadPgoSetting: last mode '%s' matches page# %d/%d", (const char*)last_mode.c_str(), i, NotebookPgoParms->GetPageCount());
            NotebookPgoParms->SetSelection(i);
            break;
        }
    CheckBox_AutoFadePgoElement->SetValue(pgoXml.GetRoot()->GetAttribute(AutoFadeElement) == Yes);
    CheckBox_AutoFadePgoAll->SetValue(pgoXml.GetRoot()->GetAttribute(AutoFadeAll) == Yes);
    TextCtrl_AutoFadePgoElement->SetValue(pgoXml.GetRoot()->GetAttribute(DelayElement));
    TextCtrl_AutoFadePgoAll->SetValue(pgoXml.GetRoot()->GetAttribute(DelayAll));
    CheckBox_CoroEyesRandomBlink->SetValue(pgoXml.GetRoot()->GetAttribute(EyesBlink) == Yes);
    CheckBox_CoroEyesRandomLR->SetValue(pgoXml.GetRoot()->GetAttribute(EyesLR) == Yes);
    wxXmlNode* AutoFace = FindNode(pgoXml.GetRoot(), wxT("autoface"), Name, wxEmptyString, true);
//    XmlNode* first_face = parent->GetChildren(); //TODO: look at multiple children?
    wxXmlNode* any_node = FindNode(AutoFace, wxT("auto"), Name, wxEmptyString, false); //TODO: not sure which child node to use; there are no group names on this tab
//    any_node->AddAttribute(wxT("face_shape"), RadioButton_PgoFaceRound->GetValue()? Yes: No);
    if (any_node)
    {
        wxString shape = any_node->GetAttribute(wxT("face_shape"), Yes); //TODO: Y/N doesn't seem like the best choice here
        RadioButton_PgoFaceRound->SetValue(shape == Yes);
        RadioButton_PgoFaceRect->SetValue(shape != Yes);
        CheckBox_PgoFaceOutline->SetValue(any_node->GetAttribute(wxT("outline"), Yes) == Yes);
    }

//load corofaces settings:
//individual UI controls are loaded when the user chooses a group name later
//only the list of available groups is populated here
    Choice_PgoGroupName->Clear();
    Choice_PgoGroupName->Append(SelectionHint); //wxT("(choose one)"));
    Choice_PgoGroupName->Append(CreationHint); //wxT("(add new)"));
    wxXmlNode* CoroFaces = FindNode(pgoXml.GetRoot(), wxT("corofaces"), Name, wxEmptyString, true);
//    wxString buf;
    for (wxXmlNode* group = CoroFaces->GetChildren(); group != NULL; group = group->GetNext())
    {
        wxString grpname = group->GetAttribute(Name);
        debug(15, "found %s group '%s'", (const char*)group->GetName().c_str(), (const char*)group->GetAttribute(Name, wxT("??")).c_str());
        if (group->GetName() != "coro") continue;
        if (grpname.IsEmpty()) continue;
//        CoroGroup newgrp;
//        buf += _(", "); buf += grpname;
        Choice_PgoGroupName->Append(grpname); //build user's choice list for group names
//        std::pair<iterator, bool> newgrp = coro_groups.emplace(grpname);
//        for (int i = 0; i < numents(CoroGroup); ++i)
//            newgrp.first[i].empty = true;
//        coro_groups.emplace(grpname, newgrp);
    }

//    wxMessageBox(wxString::Format(_("found %d grps: %s"), Choice_PgoGroupName->GetCount(), buf));
//load image settings:
   wxXmlNode* Images = FindNode(pgoXml.GetRoot(), wxT("images"), Name, wxEmptyString, true);
   for (wxXmlNode* image = Images->GetChildren(); image != NULL; image = image->GetNext())
    {
        wxString grpname = image->GetAttribute(Name);
        debug(5, "found %s image '%s'", (const char*)image->GetName().c_str(), (const char*)image->GetAttribute(Name, wxT("??")).c_str());
        if (image->GetName() != "coro") continue;
//TODO: set group name choices
    }

//load mp4 settings:
    wxXmlNode* Mp4 = FindNode(pgoXml.GetRoot(), wxT("mp4"), Name, wxEmptyString, true);
    for (wxXmlNode* image = Mp4->GetChildren(); image != NULL; image = image->GetNext())
    {
        wxString grpname = image->GetAttribute(Name);
        debug(5, "found %s image '%s'", (const char*)image->GetName().c_str(), (const char*)image->GetAttribute(Name, wxT("??")).c_str());
        if (image->GetName() != "coro") continue;
//TODO: set group name choices
    }

#if 1 //do this whenever file changes (avoid caching too long?)
    Choice_PgoModelVoiceEdit->Clear();
    Choice_PgoModelVoiceEdit->Append(SelectionHint); //wxT("(choose)"));
    Choice_PgoModelVoiceEdit->SetSelection(0);
    for (auto it = xLightsFrame::PreviewModels.begin(); it != xLightsFrame::PreviewModels.end(); ++it)
    {
        if ((*it)->name.IsEmpty()) continue;
        if (!(*it)->IsCustom()) continue; //only want custom models for now
        Choice_PgoModelVoiceEdit->Append((*it)->name);
    }
//also list non-preview models:
    for (auto it = xLightsFrame::OtherModels.begin(); it != xLightsFrame::OtherModels.end(); ++it)
    {
        if ((*it)->name.IsEmpty()) continue;
        if (!(*it)->IsCustom()) continue;
        Choice_PgoModelVoiceEdit->Append(InactiveIndicator + (*it)->name); //show non-active models in parens
    }
//tell user there are none to choose from:
    if (Choice_PgoModelVoiceEdit->GetCount() < 2)
    {
        Choice_PgoModelVoiceEdit->Clear();
        Choice_PgoModelVoiceEdit->Append(NoneHint); //wxT("(no choices)"));
        Choice_PgoModelVoiceEdit->SetSelection(0);
    }
#endif // 0
    for (int i = 0; i < GridCoroFaces->GetCols(); ++i)
        GridCoroFaces->SetCellValue(Model_Row, i, SelectionHint);
    debug(10, "set selection hint on model row");

    if (Choice_PgoGroupName->GetCount() > 2)
    {
        Choice_PgoGroupName->SetSelection(2); //choose first one found instead of "choose"
        wxCommandEvent non_evt;
        OnChoice_PgoGroupNameSelect(non_evt); //kludge: force UI to update
    }
    else
        Choice_PgoGroupName->SetSelection(0); //"choose one" hint

//    wxMessageBox(wxString::Format(_("load settings: %d active models, %d inactive models, choice %d of %d"), xLightsFrame::PreviewModels.end() - xLightsFrame::PreviewModels.begin(), xLightsFrame::OtherModels.end() - xLightsFrame::OtherModels.begin(), Choice_PgoGroupName->GetSelection(), Choice_PgoGroupName->GetCount()), wxT("Debug info"));
    StatusBar1->SetStatusText(wxString::Format(_("Loaded pgo settings: %d active models, %d inactive models, choice %d of %d"), xLightsFrame::PreviewModels.size(), xLightsFrame::OtherModels.size(), Choice_PgoGroupName->GetSelection(), Choice_PgoGroupName->GetCount()));
    debug(10, "loaded pgo settings: %d active models, %d inactive models, choice %d of %d", xLightsFrame::PreviewModels.size(), xLightsFrame::OtherModels.size(), Choice_PgoGroupName->GetSelection(), Choice_PgoGroupName->GetCount());
    return true;
}

bool xLightsFrame::GetGroupName(wxString& grpname)
{
    grpname = Choice_PgoGroupName->GetString(Choice_PgoGroupName->GetSelection());
    if ((grpname == SelectionHint) || (grpname == NoneHint))
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
    wxXmlNode* AutoFace = FindNode(pgoXml.GetRoot(), wxT("autoface"), Name, wxEmptyString, true);
    debug(10, "mode = %d '%s'", NotebookPgoParms->GetSelection(), (NotebookPgoParms->GetSelection() != -1)? (const char*)NotebookPgoParms->GetPageText(NotebookPgoParms->GetSelection()).c_str(): "(none)");
    if (NotebookPgoParms->GetSelection() != -1) //remember last-used tab (user friendly, not critical)
        AddNonDupAttr(pgoXml.GetRoot(), LastMode, NotebookPgoParms->GetPageText(NotebookPgoParms->GetSelection()));
    AddNonDupAttr(pgoXml.GetRoot(), AutoFadeElement, CheckBox_AutoFadePgoElement->GetValue()? Yes: No);
    AddNonDupAttr(pgoXml.GetRoot(), AutoFadeAll, CheckBox_AutoFadePgoAll->GetValue()? Yes: No);
    AddNonDupAttr(pgoXml.GetRoot(), DelayElement, TextCtrl_AutoFadePgoElement->GetValue());
    AddNonDupAttr(pgoXml.GetRoot(), DelayAll, TextCtrl_AutoFadePgoAll->GetValue());
    AddNonDupAttr(pgoXml.GetRoot(), EyesBlink, CheckBox_CoroEyesRandomBlink->GetValue()? Yes: No);
    AddNonDupAttr(pgoXml.GetRoot(), EyesLR, CheckBox_CoroEyesRandomLR->GetValue()? Yes: No);
    wxXmlNode* node = FindNode(AutoFace, wxT("auto"), Name, wxEmptyString, true); //TODO: not sure which child node to use; there are no group names on this tab
    if (node->GetAttribute(Name).empty()) AddNonDupAttr(node, Name, wxT("NAME?")); //give it a name (not sure what name to use)
    AddNonDupAttr(node, wxT("face_shape"), RadioButton_PgoFaceRound->GetValue()? Yes: No);
    AddNonDupAttr(node, wxT("outline"), CheckBox_PgoFaceOutline->GetValue()? Yes: No);

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

void xLightsFrame::OnNotebookPgoParmsPageChanged(wxNotebookEvent& event)
{
//    wxMessageBox(wxString::Format("pgo tab now = %d vs. %d", NotebookPgoParms->GetSelection(), COROTAB));
    debug(10, "grid cell sel notebook %d, %d", GridCoroFaces->GetCursorRow(), GridCoroFaces->GetCursorColumn());
//    int row = GridCoroFaces->GetCursorRow(), col = GridCoroFaces->GetCursorColumn();
//    if (row < 0) row = 0;
//    if (col < 0) col = 0;
    if (event.GetSelection() == COROTAB) Timer2.StartOnce(10); //show drop-down after UI stabilizes
    else //de-select row/col for clean re-entry
    {
        PgoGridCellSelect(-1, -1, __LINE__); //hide drop-down
        GridCoroFaces->SelectRow(-1);
        GridCoroFaces->SelectCol(-1);
    }
//        PgoGridCellSelect(row, col, __LINE__); //(0, 0); //show drop-down to make ui more obvious
}


//kludge: delay a little before showing drop-down list on grid
//otherwise drop-down list doesn't display reliably
void xLightsFrame::OnTimer2Trigger(wxTimerEvent& event)
{
    if (Choice_PgoModelVoiceEdit->GetCount() < 1) return; //settings not loaded yet
    int row = GridCoroFaces->GetCursorRow(), col = GridCoroFaces->GetCursorColumn();
    if (row < 0) row = 0; //default to first cell if none selected
    if (col < 0) col = 0;
    PgoGridCellSelect(row, col, __LINE__); //(0, 0); //show drop-down to make ui more obvious
//    Timer2.Stop();
}

//populate choice lists with model names, etc.
void xLightsFrame::InitPapagayoTab(bool tab_changed)
{

//??    if (Choice_PgoGroupName->GetCount() < 1)
//    {
//        Choice_PgoGroupName->Clear();
//        Choice_PgoGroupName->Append(SelectionHint); //wxT("(choose one)"));
//    }
    LoadPgoSettings();
#if 0 //do this whenever file changes (avoid caching too long?)
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
#endif // 0
}

//add (X,Y) info back into settings file for easier reference
static wxString addxy(ModelClass* model, wxString nodestr)
{
    long node;
    debug(10, "addxy: model? %d, node %s", model, (const char*)nodestr.c_str());
    if (!model || !nodestr.ToLong(&node)) return nodestr;
    debug(10, " => xy info '%s'", (const char*)model->GetNodeXY(node).c_str());
    return model->GetNodeXY(node);
}

//NOTE: this only saves one group name at a time to the xmldoc, then saves entire xmldoc to file
void xLightsFrame::OnBitmapButton_SaveCoroGroupClick(wxCommandEvent& event)
{
    wxString grpname;
    if (!GetGroupName(grpname)) return;
    PgoGridCellSelect(GridCoroFaces->GetCursorRow(), GridCoroFaces->GetCursorColumn(), __LINE__); //force cell update if edit in progress
    wxDateTime now = wxDateTime::Now(); //NOTE: now.Format("%F %T") seems to be broken
    debug(10, "SaveCoroGroupClick: save group '%s' to xmldoc, timestamp '%s %s'", (const char*)grpname.c_str(), (const char*)now.FormatDate().c_str(), (const char*)now.FormatTime().c_str()); //Format(wxT("%F %T")).c_str());
    AddNonDupAttr(pgoXml.GetRoot(), wxT("last_mod"), now.FormatDate() + wxT(" ") + now.FormatTime()); //wxT("%F %T"))); //useful for audit trail or debug
    wxXmlNode* CoroFaces = FindNode(pgoXml.GetRoot(), wxT("corofaces"), Name, wxEmptyString, true);
    wxXmlNode* node = FindNode(CoroFaces, wxT("coro"), Name, grpname, true);
    int num_voice = -1;
    wxString warnings;
    for (int i = 0; i < GridCoroFaces->GetCols(); ++i)
    {
        bool non_empty = false;
        for (int r = 0; r < GridCoroFaces->GetRows(); ++r)
        {
            if (GridCoroFaces->GetCellValue(r, i).empty()) continue;
            non_empty = true;
            break;
        }
        wxString voice_model = NoInactive(GridCoroFaces->GetCellValue(Model_Row, i));
        ModelClass* model_info = ModelClass::FindModel(voice_model);
//        if (Voice(i)->GetSelection() >= 0) voice_model = Voice(i)->GetString(Voice(i)->GetSelection());
//        if (Choice_PgoModelVoiceEdit->GetSelection() >= 0) voice_model = Choice_PgoModelVoiceEdit->GetString(Choice_PgoModelVoiceEdit->GetSelection());
        if ((voice_model == SelectionHint) || (voice_model == NoneHint)) //warn if user forgot to set model
        {
            if (non_empty) warnings += wxString::Format(wxT("Voice# %d not saved (no model selected).\n"), i + 1);
            continue;
        }
        wxXmlNode* voice = FindNode(node, "voice", wxT("voiceNumber"), wxString::Format(wxT("%d"), i + 1), true);
//        voice->AddAttribute(wxT("voiceNumber"), wxString::Format(wxT("%d"), i + 1));
        AddNonDupAttr(voice, Name, voice_model);
        AddNonDupAttr(voice, wxT("Outline"), addxy(model_info, GridCoroFaces->GetCellValue(Outline_Row, i)));
        AddNonDupAttr(voice, wxT("Eyes_open"), addxy(model_info, GridCoroFaces->GetCellValue(Eyes_open_Row, i)));
        AddNonDupAttr(voice, wxT("Eyes_closed"), addxy(model_info, GridCoroFaces->GetCellValue(Eyes_closed_Row, i)));
        AddNonDupAttr(voice, wxT("Eyes_up"), addxy(model_info, GridCoroFaces->GetCellValue(Eyes_up_Row, i)));
        AddNonDupAttr(voice, wxT("Eyes_down"), addxy(model_info, GridCoroFaces->GetCellValue(Eyes_down_Row, i)));
        AddNonDupAttr(voice, wxT("Eyes_left"), addxy(model_info, GridCoroFaces->GetCellValue(Eyes_left_Row, i)));
        AddNonDupAttr(voice, wxT("Eyes_right"), addxy(model_info, GridCoroFaces->GetCellValue(Eyes_right_Row, i)));
        AddNonDupAttr(voice, wxT("AI"), addxy(model_info, GridCoroFaces->GetCellValue(AI_Row, i)));
        AddNonDupAttr(voice, wxT("E"), addxy(model_info, GridCoroFaces->GetCellValue(E_Row, i)));
        AddNonDupAttr(voice, wxT("etc"), addxy(model_info, GridCoroFaces->GetCellValue(etc_Row, i)));
        AddNonDupAttr(voice, wxT("FV"), addxy(model_info, GridCoroFaces->GetCellValue(FV_Row, i)));
        AddNonDupAttr(voice, wxT("L"), addxy(model_info, GridCoroFaces->GetCellValue(L_Row, i)));
        AddNonDupAttr(voice, wxT("MBP"), addxy(model_info, GridCoroFaces->GetCellValue(MBP_Row, i)));
        AddNonDupAttr(voice, wxT("O"), addxy(model_info, GridCoroFaces->GetCellValue(O_Row, i)));
        AddNonDupAttr(voice, wxT("rest"), addxy(model_info, GridCoroFaces->GetCellValue(rest_Row, i)));
        AddNonDupAttr(voice, wxT("U"), addxy(model_info, GridCoroFaces->GetCellValue(U_Row, i)));
        AddNonDupAttr(voice, wxT("WQ"), addxy(model_info, GridCoroFaces->GetCellValue(WQ_Row, i)));
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

static wxString ExtractNodes(wxString parsed_info)
{
    wxString nodestr;
    wxStringTokenizer wtkz(parsed_info, "+");
    while (wtkz.HasMoreTokens())
    {
        wxString nextnode = wtkz.GetNextToken().BeforeFirst(':');
        if (nextnode.empty()) continue;
        if (!nodestr.empty()) nodestr += wxT("+");
        nodestr += nextnode;
    }
    debug(10, "extracted nodes '%s' from '%s'", (const char*)nodestr.c_str(), (const char*)parsed_info.c_str());
    return nodestr;
}

//this loads one group name at a time from the xmldoc
void xLightsFrame::OnChoice_PgoGroupNameSelect(wxCommandEvent& event)
{
    debug(10, "PgoGroupNameSelect selection %d, count %d", Choice_PgoGroupName->GetSelection(), Choice_PgoModelVoiceEdit->GetCount());
    if (Choice_PgoModelVoiceEdit->GetCount() < 1) return; //settings not loaded yet
    wxString grpname;
    if (!GetGroupName(grpname)) return;
    debug(10, "PgoGroupNameSelect: load group '%s' from xmldoc", (const char*)grpname.c_str());
    wxXmlNode* CoroFaces = FindNode(pgoXml.GetRoot(), wxT("corofaces"), Name, wxEmptyString, true);
    wxXmlNode* node = FindNode(CoroFaces, wxT("coro"), Name, grpname, true);
    wxString errors;
    for (int i = 0; i < GridCoroFaces->GetCols(); ++i)
//    for (wxXmlNode* voice = node->GetChildren(); voice != NULL; voice = voice->GetNext())
    {
        wxXmlNode* voice = FindNode(node, "voice", wxT("voiceNumber"), wxString::Format(wxT("%d"), i + 1), true);
//        int voice_num = wxAtoi(voice->GetAttribute(wxT("voiceNumber"), wxT("0")));
//        if ((voice_num < 1) || (voice_num > GridCoroFaces->GetCols())) continue; //bad voice#
//        int inx = Voice(i)->FindString(voice->GetAttribute(Name));
        wxString voice_name = NoInactive(voice->GetAttribute(Name));
        int inx = Choice_PgoModelVoiceEdit->FindString(voice_name);
        if ((inx < 0) && !voice_name.empty())
        {
            inx = Choice_PgoModelVoiceEdit->FindString(InactiveIndicator + voice_name);
            errors += wxString::Format(wxT("Saved model name '%s' %s for voice# %d.\n"), voice_name, (inx == wxNOT_FOUND)? wxT("not found"): wxT("not marked 'my display'"), i + 1); //, Choice_PgoModelVoiceEdit->GetCount());
        }
        debug(10, "grp name sel[%d] '%s' => inx %d", i, (const char*)voice_name.c_str(), inx);
        if (inx < 0) inx = 0; //default to "(choose)" hint
//        Voice(i)->SetSelection(inx);
//        Choice_PgoModelVoiceEdit->SetSelection(inx);
//        voice->AddAttribute(wxT("voiceNumber"), wxString::Format(wxT("%d"), i + 1));

        GridCoroFaces->SetCellValue(Model_Row, i, Choice_PgoModelVoiceEdit->GetString(inx));
        debug(10, "model name[%d] now = '%s' from inx %d", i, (const char*)GridCoroFaces->GetCellValue(Model_Row, i).c_str(), inx);
        GridCoroFaces->SetCellValue(Outline_Row, i, ExtractNodes(voice->GetAttribute(wxT("Outline"))));
        GridCoroFaces->SetCellValue(Eyes_open_Row, i, ExtractNodes(voice->GetAttribute(wxT("Eyes_open"))));
        GridCoroFaces->SetCellValue(Eyes_closed_Row, i, ExtractNodes(voice->GetAttribute(wxT("Eyes_closed"))));
        GridCoroFaces->SetCellValue(Eyes_up_Row, i, ExtractNodes(voice->GetAttribute(wxT("Eyes_up"))));
        GridCoroFaces->SetCellValue(Eyes_down_Row, i, ExtractNodes(voice->GetAttribute(wxT("Eyes_down"))));
        GridCoroFaces->SetCellValue(Eyes_left_Row, i, ExtractNodes(voice->GetAttribute(wxT("Eyes_left"))));
        GridCoroFaces->SetCellValue(Eyes_right_Row, i, ExtractNodes(voice->GetAttribute(wxT("Eyes_right"))));
        GridCoroFaces->SetCellValue(AI_Row, i, ExtractNodes(voice->GetAttribute(wxT("AI"))));
        GridCoroFaces->SetCellValue(E_Row, i, ExtractNodes(voice->GetAttribute(wxT("E"))));
        GridCoroFaces->SetCellValue(etc_Row, i, ExtractNodes(voice->GetAttribute(wxT("etc"))));
        GridCoroFaces->SetCellValue(FV_Row, i, ExtractNodes(voice->GetAttribute(wxT("FV"))));
        GridCoroFaces->SetCellValue(L_Row, i, ExtractNodes(voice->GetAttribute(wxT("L"))));
        GridCoroFaces->SetCellValue(MBP_Row, i, ExtractNodes(voice->GetAttribute(wxT("MBP"))));
        GridCoroFaces->SetCellValue(O_Row, i, ExtractNodes(voice->GetAttribute(wxT("O"))));
        GridCoroFaces->SetCellValue(rest_Row, i, ExtractNodes(voice->GetAttribute(wxT("rest"))));
        GridCoroFaces->SetCellValue(U_Row, i, ExtractNodes(voice->GetAttribute(wxT("U"))));
        GridCoroFaces->SetCellValue(WQ_Row, i, ExtractNodes(voice->GetAttribute(wxT("WQ"))));
        debug(10, "set grid cells from voice attrs");
    }
    if (!errors.empty()) wxMessageBox(errors, wxT("Bad config settings")); //only show one message
}

//TODO: use Save for Delete as well?
void xLightsFrame::OnButton_CoroGroupDeleteClick(wxCommandEvent& event)
{
    wxString grpname;
    if (!GetGroupName(grpname)) return;
    debug(10, "CoroGroupDeleteClick: delete group '%s' from xmldoc", (const char*)grpname.c_str());
    wxXmlNode* CoroFaces = FindNode(pgoXml.GetRoot(), wxT("corofaces"), Name, wxEmptyString, true);
    wxXmlNode* node = FindNode(CoroFaces, "coro", Name, grpname, false);
    if (node) CoroFaces->RemoveChild(node); //delete group
    Choice_PgoGroupName->SetSelection(0); //"choose"
    GridCoroFaces->ClearGrid();
}

//NOTE: doesn't save
void xLightsFrame::OnButton_CoroGroupClearClick(wxCommandEvent& event)
{
    GridCoroFaces->ClearGrid();
}

//#include <wx/wxprec.h>
//#include <wx/utils.h>
static wxSize prevcell(-1, Model_Row);

//kludge: expose additional methods
//CAUTION: data members must remain the same for safe cast
class myGrid: public wxGrid
{
public:
    int GetColWidth(int col) const { return wxGrid::GetColWidth(col); }
    int GetColLeft(int col) const { return wxGrid::GetColLeft(col); }
    int GetColRight(int col) const { return wxGrid::GetColRight(col); }
    int GetRowTop(int row) const { return wxGrid::GetRowTop(row); }
    int GetRowBottom(int row) const { return wxGrid::GetRowBottom(row); }
    int GetRowHeight(int row) const { return wxGrid::GetRowHeight(row); }
};

void xLightsFrame::OnGridCoroFacesCellSelect(wxGridEvent& event)
{
    PgoGridCellSelect(event.GetRow(), event.GetCol(), __LINE__);
}

#define nodelist  Choice_RelativeNodes
//#define nodelist  ChoiceListBox_RelativeNodes
//#define nodelist  ListBox_RelativeNodes

//find unique node#s and associated (X,Y) for a model:
void xLightsFrame::GetMouthNodes(const wxString& model_name)
{
    debug(10, "get mouth nodes '%s'", (const char*)model_name.c_str());
//    StatusBar1->SetStatusText(wxT("get mouth nodes ..."));
    for (auto it = xLightsFrame::PreviewModels.begin(); it != xLightsFrame::PreviewModels.end(); ++it)
    {
        if (model_name.CmpNoCase((*it)->name)) continue; //don't check this model
        debug(10, "parse model '%s'", (const char*)(*it)->name.c_str());
        if (!(*it)->GetChannelCoords(Choice_RelativeNodes, 0, 0)) nodelist->Append(NoneHint);
//    StatusBar1->SetStatusText(wxT("...get mouth nodes"));
        debug(10, "got %d ents, active", nodelist->GetCount());
        return;
    }
    for (auto it = xLightsFrame::OtherModels.begin(); it != xLightsFrame::OtherModels.end(); ++it)
    {
        if (model_name.CmpNoCase((*it)->name)) continue; //don't check this model
        debug(10, "parse model '%s'", (const char*)(*it)->name.c_str());
        if (!(*it)->GetChannelCoords(Choice_RelativeNodes, 0, 0)) nodelist->Append(NoneHint);
//    StatusBar1->SetStatusText(wxT("...get mouth nodes"));
        debug(10, "got %d ents, inactive", nodelist->GetCount());
        return;
    }
//    StatusBar1->SetStatusText(wxT("...get mouth nodes"));
    debug(10, "!found model '%s', got %d ents, active", (const char*)model_name.c_str(), nodelist->GetCount());
}

static void MultiSelectNodes(wxChoice* choices, wxString nodestr)
{
    wxStringTokenizer wtkz(nodestr, "+");
    while (wtkz.HasMoreTokens())
    {
        wxString nextnode = wtkz.GetNextToken();
//        choices->SetSelection(choices->FindString(nextnode)); //load value from new cell
        for (size_t i = 0; i < choices->GetCount(); ++i)
            if (choices->GetString(i).BeforeFirst(':') == nextnode)
            {
                debug(10, "select entry '%s' using value '%s'", (const char*)choices->GetString(i).c_str(), (const char*)nextnode.c_str());
                choices->SetSelection(i);
                break;
            }
//TODO: need multi-select here; for now it just keeps the last one
    }
}

//kludgey drop-down edit for grid cells:
//TODO: maybe can use custom grid cell editor in wxWidgets 3.1
//TODO: multi-select elements in Pgo grid cells
void xLightsFrame::PgoGridCellSelect(int row, int col, int where)
{
//    wxMessageBox(wxT("editor shown"));
//    StatusBar1->SetStatusText(wxString::Format(wxT("cell sel %d, %d"), event.GetCol(), event.GetRow())); //GridCoroFaces->GetGridCursorCol(), GridCoroFaces->GetGridCursorRow()));
//    wxBell();
//    StatusBar1->SetStatusText(wxString::Format(wxT("grid x %d, y %d, scroll %d %d, cell: c %d, r %d, x %d, l %d, r %d, y %d, b %d, w %d %d, h %d, rlw %d"), GridCoroFaces->GetPosition().x, GridCoroFaces->GetPosition().y, GridCoroFaces->GetScrollPosX(), GridCoroFaces->GetScrollPosY(), col, row, ((myGrid*)GridCoroFaces)->GetColPos(col), ((myGrid*)GridCoroFaces)->GetColLeft(col), ((myGrid*)GridCoroFaces)->GetColRight(col), ((myGrid*)GridCoroFaces)->GetRowTop(row), ((myGrid*)GridCoroFaces)->GetRowBottom(row), ((myGrid*)GridCoroFaces)->GetColWidth(col), ((myGrid*)GridCoroFaces)->GetColSize(col), ((myGrid*)GridCoroFaces)->GetRowHeight(row), GridCoroFaces->GetRowLabelSize()));
    bool node_edit = nodelist->Hide(); //was editing node#s
    bool model_edit = Choice_PgoModelVoiceEdit->Hide(); //was editing a model name
    if (node_edit || model_edit)
    {
        debug(10, "cell sel(%d, %d) was vis from (%d, %d) @%d", row, col, prevcell.y, prevcell.x, where);
        if (prevcell.x != -1) //update previous cell
            GridCoroFaces->SetCellValue(prevcell.y, prevcell.x, node_edit? ExtractNodes(nodelist->GetString(nodelist->GetSelection())): Choice_PgoModelVoiceEdit->GetString(Choice_PgoModelVoiceEdit->GetSelection()));
        prevcell.x = -1;
    }
    if (col != prevcell.x)  //analyze face element node#s in new model
        GetMouthNodes(NoInactive(GridCoroFaces->GetCellValue(Model_Row, col)));
    int destx = ((myGrid*)GridCoroFaces)->GetColLeft(col), destw = ((myGrid*)GridCoroFaces)->GetColWidth(col);
    int desty = ((myGrid*)GridCoroFaces)->GetRowTop(row), desth = ((myGrid*)GridCoroFaces)->GetRowHeight(row);
    destx += GridCoroFaces->GetPosition().x + GridCoroFaces->GetRowLabelSize();
    desty += GridCoroFaces->GetPosition().y + GridCoroFaces->GetRowHeight(0); //kludge: assume col labels are same height as row
#if 0 //wxWidgets BROKEN
    int srcollx = GridCoroFaces->GetScrollPosX(); //TODO: scroll position BROKEN
    int scrolly = GridCoroFaces->GetScrollPosY();
#else //kludgey way to find approx scroll position
    int scrollx = 0, scrolly = 0;
//GetScrollLineY() seems to be more accurate than GetRowHeight()
//check col 2 in case col 0 !vis
    for (int r = 0; r < GridCoroFaces->GetRows(); ++r)
        if (!GridCoroFaces->IsVisible(r, 0+2)) // && !GridCoroFaces->IsVisible(r - 1, 0))
        {
            scrolly += GridCoroFaces->GetScrollLineY(); //((myGrid*)GridCoroFaces)->GetRowHeight(r);
            if (!(r % 5)) scrolly += 3; //FUD factor
        }
        else break;
//X not reliable
//    for (int c = 0; c < GridCoroFaces->GetCols(); ++c)
//        if (!GridCoroFaces->IsVisible(scrolly, c))
//            scrollx += GridCoroFaces->GetScrollLineX(); //((myGrid*)GridCoroFaces)->GetColWidth(c);
//        else break;
#endif // 0
    destx -= scrollx; //GridCoroFaces->GetScrollPosX(); //TODO: scroll position BROKEN
    desty -= scrolly; //GridCoroFaces->GetScrollPosY();
    debug(10, "coro grid scrolled at (x %d, y %d), detected at (%d, %d), scr inc %d, %d", GridCoroFaces->GetScrollPosX(), GridCoroFaces->GetScrollPosY(), scrollx, scrolly, GridCoroFaces->GetScrollLineX(), GridCoroFaces->GetScrollLineY());
    debug(10, "cell (0,0) vis? %d, (4,0) vis? %d, (12, 0) vis? %d", GridCoroFaces->IsVisible(0, 0), GridCoroFaces->IsVisible(4, 0), GridCoroFaces->IsVisible(12, 0));
//    if (row != Model_Row) desth *= 5; //give ListBox some room
#if 1 //broken?
    wxControl* choices = (row == Model_Row)? (wxControl*)Choice_PgoModelVoiceEdit: (wxControl*)nodelist; //start model name vs. node#s
//TODO: some of the below calls might be redundant; had a hard time getting it to display consistently at first, so just leave them all in for now
    choices->Show();
    choices->Move(destx, desty);
    choices->SetSize(destw, desth);
    choices->Raise(); //put it on top of grid
//    choices->GetSizer().Layout(); //FlexGridSizer51
//    GridCoroFaces->Hide();
//    GridCoroFaces->Show();
    choices->Update();
    choices->Refresh();
#else
    if (row == Model_Row) //start model name vs. node#s
    {
        Choice_PgoModelVoiceEdit->Show();
        Choice_PgoModelVoiceEdit->Move(destx, desty);
        Choice_PgoModelVoiceEdit->SetSize(destw, desth);
        Choice_PgoModelVoiceEdit->Raise(); //put it on top of grid
        Choice_PgoModelVoiceEdit->Update();
        Choice_PgoModelVoiceEdit->Refresh();
    }
    else
    {
        ListBox_RelativeNodes->Show();
        ListBox_RelativeNodes->Move(destx, desty);
        ListBox_RelativeNodes->SetSize(destw, desth);
        ListBox_RelativeNodes->Raise(); //put it on top of grid
        ListBox_RelativeNodes->Update();
        ListBox_RelativeNodes->Refresh();
    }
//TODO: some of the below calls might be redundant; had a hard time getting it to display consistently at first, so just leave them all in for now
#endif
    debug(1, "cell sel (%d, %d) made vis '%s' @%d", row, col, (const char*)GridCoroFaces->GetCellValue(row, col).c_str(), where);
    if (row == Model_Row)
        Choice_PgoModelVoiceEdit->SetSelection(Choice_PgoModelVoiceEdit->FindString(GridCoroFaces->GetCellValue(row, col))); //load value from new cell
    else //multi-select and match on node#
        MultiSelectNodes(nodelist, GridCoroFaces->GetCellValue(row, col));
    prevcell.x = col; //remember where to put back new value
    prevcell.y = row;
//    Choice_PgoModelVoiceEdit = new wxChoice(PGO_COROFACES, ID_CHOICE_PgoModelVoiceEdit, wxDefaultPosition, wxSize(86,21), 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE_PgoModelVoiceEdit"));
//FlexGridSizer51
}
