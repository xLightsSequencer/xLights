/***************************************************************
 * Name:      xLightsMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-11-03
 * Copyright: Matt Brown ()
 * License:
 **************************************************************/

#include "xLightsMain.h"
#include <wx/msgdlg.h>
#include <wx/config.h>
#include <wx/dir.h>
#include <wx/textdlg.h>
#include <wx/numdlg.h>
#include <wx/valnum.h>

// dialogs
#include "SerialPortWithRate.h"
#include "E131Dialog.h"

// xml
#include "../include/xml-irr-1.2/irrXML.h"
#include "../include/xml-irr-1.2/irrXML.cpp"
using namespace irr;
using namespace io;

// scripting language
#include "xLightsBasic.cpp"

// image files
#include "../include/xlights.xpm"
#include "../include/open.xpm"
#include "../include/save.xpm"
#include "../include/insertrow.xpm"
#include "../include/deleterow.xpm"

//(*InternalHeaders(xLightsFrame)
#include <wx/intl.h>
#include <wx/string.h>
//*)


//helper functions
enum wxbuildinfoformat
{
    short_f, long_f
};

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

//(*IdInit(xLightsFrame)
//*)

const long xLightsFrame::ID_PLAYER_DIALOG = wxNewId();
const long xLightsFrame::ID_DELETE_EFFECT = wxNewId();
const long xLightsFrame::ID_IGNORE_CLICK = wxNewId();
const long xLightsFrame::ID_PROTECT_EFFECT = wxNewId();
const long xLightsFrame::ID_UNPROTECT_EFFECT = wxNewId();
const long xLightsFrame::ID_RANDOM_EFFECT = wxNewId();


BEGIN_EVENT_TABLE(xLightsFrame,wxFrame)
    //(*EventTable(xLightsFrame)
    //*)
END_EVENT_TABLE()

xLightsFrame::xLightsFrame(wxWindow* parent,wxWindowID id)
{

    //(*Initialize(xLightsFrame)
    Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("id"));
    //*)



    SetIcon(wxIcon(xlights_xpm));
    wxStandardPathsBase& stdp = wxStandardPaths::Get();
    wxConfig* config = new wxConfig(_(XLIGHTS_CONFIG_ID));

    effGridPrevX = 0;
    effGridPrevY = 0;

    // Load headings into network list
    wxListItem itemCol;
    itemCol.SetText(_T("Network Type"));
    itemCol.SetImage(-1);
    GridNetwork->InsertColumn(0, itemCol);

    itemCol.SetText(_T("Port"));
    itemCol.SetAlign(wxLIST_FORMAT_LEFT);
    GridNetwork->InsertColumn(1, itemCol);

    itemCol.SetText(_T("Baud Rate or E1.31 Univ"));
    itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
    GridNetwork->InsertColumn(2, itemCol);

    itemCol.SetText(_T("Last Channel"));
    itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
    GridNetwork->InsertColumn(3, itemCol);

    itemCol.SetText(_T("LOR Mapping"));
    itemCol.SetAlign(wxLIST_FORMAT_LEFT);
    GridNetwork->InsertColumn(4, itemCol);

    itemCol.SetText(_T("xLights/Vixen Mapping"));
    itemCol.SetAlign(wxLIST_FORMAT_LEFT);
    GridNetwork->InsertColumn(5, itemCol);

    GridNetwork->SetColumnWidth(0,wxLIST_AUTOSIZE_USEHEADER);
    GridNetwork->SetColumnWidth(1,100);
    GridNetwork->SetColumnWidth(2,wxLIST_AUTOSIZE_USEHEADER);
    GridNetwork->SetColumnWidth(3,100);
    GridNetwork->SetColumnWidth(4,150);
    GridNetwork->SetColumnWidth(5,150);

    Grid1->SetColFormatFloat(0,7,3);

    // get list of most recently used directories
    wxString dir,mru_name;
    int menuID, idx;
    for (int i=0; i<MRU_LENGTH; i++)
    {
        mru_name=wxString::Format(wxT("mru%d"),i);
        dir.clear();
        if ( config->Read(mru_name, &dir) )
        {
            if (!dir.IsEmpty())
            {
                idx=mru.Index(dir);
                if (idx == wxNOT_FOUND) mru.Add(dir);
            }
        }
        menuID = wxNewId();
        mru_MenuItem[i] = new wxMenuItem(MenuFile, menuID, mru_name);
        Connect(menuID,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuMRU);
    }
    dir.clear();
    bool ok = config->Read(_("LastDir"), &dir);
    wxString ConvertDir;
    if (ok && !config->Read(_("ConvertDir"), &ConvertDir))
    {
        ConvertDir=dir;
    }
    FileDialogConvert->SetDirectory(ConvertDir);

    // initialize all effect wxChoice lists

    BarEffectDirections.Add("up");
    BarEffectDirections.Add("down");
    BarEffectDirections.Add("expand");
    BarEffectDirections.Add("compress");

    ButterflyEffectColors.Add("Rainbow");
    ButterflyEffectColors.Add("Palette");

    MeteorsEffectTypes.Add("Rainbow");
    MeteorsEffectTypes.Add("Range");
    MeteorsEffectTypes.Add("Palette");

    MeteorsEffect.Add("Meteor");
    MeteorsEffect.Add("Swirl1");
    MeteorsEffect.Add("Swirl2");
    MeteorsEffect.Add("StarField");

    EffectDirections.Add("left");
    EffectDirections.Add("right");
    EffectDirections.Add("up");
    EffectDirections.Add("down");
    EffectDirections.Add("none");
    EffectDirections.Add("up-left");
    EffectDirections.Add("down-left");
    EffectDirections.Add("up-right");
    EffectDirections.Add("down-right");

    TextEffects.Add("normal");
    TextEffects.Add("vertical text up");
    TextEffects.Add("vertical dext down");
    TextEffects.Add("rotate up 45");
    TextEffects.Add("rotate up 90");
    TextEffects.Add("rotate down 45");
    TextEffects.Add("rotate down 90");

    TextCountDown.Add("none");
    TextCountDown.Add("count down seconds");
    TextCountDown.Add("count down to date");

    InitEffectsPanel(EffectsPanel1);
    InitEffectsPanel(EffectsPanel2);

    // Check if schedule should be running
    xout=0;
    long RunFlag=0;
    config->Read(_("RunSchedule"), &RunFlag);
    delete config;  // close config before calling SetDir, which will open config

    SetPlayMode(play_off);
    ResetEffectsXml();
    EnableSequenceControls(true);
    UpdateShowDates(wxDateTime::Now(),wxDateTime::Now());
    if (ok && !dir.IsEmpty())
    {
        SetDir(dir);
    }
    EffectsPanel1->PaletteChanged=true;
    EffectsPanel2->PaletteChanged=true;
    MixTypeChanged=true;
    HtmlEasyPrint=new wxHtmlEasyPrinting(wxT("xLights Printing"), this);
    basic.setFrame(this);
    PlayerDlg = new PlayerFrame(this, ID_PLAYER_DIALOG);

    /* The whole registration of effects is kind of a kludge but without moving all
     * of the unique panels out of wxSmith there really isn't a better way to do
     * this that I can think of.  This extension is to enable random effect generation
     */
    EffectNames.clear();
    EffectNames.resize(eff_LASTEFFECT); //
    EffectNames[eff_NONE]      =wxT("None");
    EffectNames[eff_BARS]      =wxT("Bars");
    EffectNames[eff_BUTTERFLY] =wxT("Butterfly");
    EffectNames[eff_COLORWASH] =wxT("Color Wash");
    EffectNames[eff_FIRE]      =wxT("Fire");
    EffectNames[eff_GARLANDS]  =wxT("Garlands");
    EffectNames[eff_LIFE]      =wxT("Life");
    EffectNames[eff_METEORS]   =wxT("Meteors");
    EffectNames[eff_PICTURES]  =wxT("Pictures");
    EffectNames[eff_SNOWFLAKES]=wxT("Snowflakes");
    EffectNames[eff_SNOWSTORM] =wxT("Snowstorm");
    EffectNames[eff_SPIRALS]   =wxT("Spirals");
    EffectNames[eff_TEXT]      =wxT("Text");
    EffectNames[eff_TWINKLE]   =wxT("Twinkle");
    EffectNames[eff_TREE]      =wxT("Tree");
    EffectNames[eff_SPIROGRAPH]=wxT("Spirograph");
    EffectNames[eff_FIREWORKS] =wxT("Fireworks");
    EffectNames[eff_CIRCLES]   =wxT("Circles");
    EffectNames[eff_PIANO]     =wxT("Piano");

    EffectLayerOptions.clear();
    EffectLayerOptions.resize(LASTLAYER);
    EffectLayerOptions[EFFECT1]     =wxT("Effect 1");
    EffectLayerOptions[EFFECT2]     =wxT("Effect 2");
    EffectLayerOptions[EFF1MASK]    =wxT("1 is Mask");
    EffectLayerOptions[EFF2MASK]    =wxT("2 is Mask");
    EffectLayerOptions[EFF1UNMASK]  =wxT("1 is Unmask");
    EffectLayerOptions[EFF2UNMASK]  =wxT("2 is Unmask");
    EffectLayerOptions[EFFLAYERED]  =wxT("Layered");
    EffectLayerOptions[EFFAVERAGED] =wxT("Average");

    if (RunFlag && !ShowEvents.IsEmpty())
    {
        // open ports
        Notebook1->ChangeSelection(SCHEDULETAB);
        CheckBoxLightOutput->SetValue(true);
        CheckBoxRunSchedule->SetValue(true);
        EnableOutputs();
        CheckRunSchedule();
    }
    else
    {
        Notebook1->ChangeSelection(SETUPTAB);
        EnableNetworkChanges();
    }
    wxImage::AddHandler(new wxGIFHandler);
    Timer1.Start(XTIMER_INTERVAL, wxTIMER_CONTINUOUS);
//   scm, causes crash if we remove this    Choicebook1->RemovePage(eff_CIRCLES);
//   Choicebook2->RemovePage(eff_CIRCLES);

}

xLightsFrame::~xLightsFrame()
{
    //(*Destroy(xLightsFrame)
    //*)
}

void xLightsFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void xLightsFrame::InitEffectsPanel(EffectsPanel* panel)
{
    panel->Choice_Bars_Direction->Set(BarEffectDirections);
    panel->Choice_Bars_Direction->SetSelection(0);
    panel->Choice_Butterfly_Colors->Set(ButterflyEffectColors);
    panel->Choice_Butterfly_Colors->SetSelection(0);
    panel->Choice_Meteors_Type->Set(MeteorsEffectTypes);
    panel->Choice_Meteors_Type->SetSelection(0);
    panel->Choice_Meteors_Effect->Set(MeteorsEffect);
    panel->Choice_Meteors_Effect->SetSelection(0);
    panel->Choice_Pictures_Direction->Set(EffectDirections);
    panel->Choice_Pictures_Direction->SetSelection(0);
    panel->Choice_Text_Dir1->Set(EffectDirections);
    panel->Choice_Text_Dir1->SetSelection(0);
    panel->Choice_Text_Dir2->Set(EffectDirections);
    panel->Choice_Text_Dir2->SetSelection(0);
    panel->Choice_Text_Effect1->Set(TextEffects);
    panel->Choice_Text_Effect1->SetSelection(0);
    panel->Choice_Text_Effect2->Set(TextEffects);
    panel->Choice_Text_Effect2->SetSelection(0);
    panel->Choice_Text_Count1->Set(TextCountDown);
    panel->Choice_Text_Count1->SetSelection(0);
    panel->Choice_Text_Count2->Set(TextCountDown);
    panel->Choice_Text_Count2->SetSelection(0);
    panel->CurrentDir = &CurrentDir;
}

void xLightsFrame::OnAbout(wxCommandEvent& event)
{
    wxString msg = _(XLIGHTS_LICENSE);
    wxString hdg = wxString::Format(_("About xLights %s"),_(XLIGHTS_VERSION));
    wxMessageBox(msg, hdg);
}

// return a random number between 0 and 1 inclusive
double xLightsFrame::rand01()
{
    return (double)rand()/(double)RAND_MAX;
}

void xLightsFrame::SetPlayMode(play_modes newmode)
{
    switch (newmode)
    {
    case play_off:
        StatusBar1->SetStatusText(_("Playback: off"));
        break;
    }

    ButtonGracefulStop->Enable(newmode == play_sched || newmode == play_list);
    ButtonChangeDir->Enable(newmode != play_sched && newmode != play_list && newmode != play_single);
    play_mode=newmode;
    starttime = wxDateTime::UNow();
#ifndef NDEBUG
    TextCtrlLog->AppendText(wxString::Format(_("SetPlayMode mode=%d state=%d\n"),play_mode,SeqPlayerState));
#endif
}

void xLightsFrame::OnTimer1Trigger(wxTimerEvent& event)
{
    wxCriticalSectionLocker locker(gs_xoutCriticalSection);
    if (CheckBoxRunSchedule->IsChecked()) CheckSchedule();
    wxTimeSpan ts = wxDateTime::UNow() - starttime;
    long curtime = ts.GetMilliseconds().ToLong();
    if (xout) xout->TimerStart(curtime);
    switch (Notebook1->GetSelection())
    {
    case TESTTAB:
        OnTimerTest(curtime);
        break;
    case SEQUENCETAB:
        TimerRgbSeq(curtime);
        break;
    default:
        OnTimerPlaylist(curtime);
        break;
    }
    if (xout) xout->TimerEnd();
}

void xLightsFrame::ResetTimer(SeqPlayerStates newstate, long OffsetMsec)
{
    SeqPlayerState = newstate;
#ifndef NDEBUG
    TextCtrlLog->AppendText(wxString::Format(_("ResetTimer mode=%d state=%d\n"),play_mode,SeqPlayerState));
#endif
    //if (newstate == NO_SEQ) SetPlayMode(play_off);
    if (xout) xout->ResetTimer();
    wxTimeSpan offset(0,0,0,OffsetMsec);
    starttime = wxDateTime::UNow() - offset;
}

void xLightsFrame::OnBitmapButtonTabInfoClick(wxCommandEvent& event)
{
    wxString caption,msg,selectmethod;
#ifdef __WXOSX__
    selectmethod=_("cmd-click");
#else
    selectmethod=_("ctrl-click or shift-click");
#endif
    switch (Notebook1->GetSelection())
    {
    case SETUPTAB:
        caption=_("Setup Tab");
        msg=_("Show Directory\n\nThe first thing you need to know about xLights is that it expects you to organize all of your sequence files and associated audio or video files into a single directory. For example, you can have a directory called '2012 Show'. Once you have your show directory created and populated with the relevant files, you are ready to proceed. Tell xLights where your new show directory is by clicking the 'Change' button on the Setup tab, navigate to your show directory, then click 'OK'.\n\nLighting Networks\n\nThe next thing you will need to do is define your lighting network(s). xLights ignores most of the information about your lighting network contained in your LOR or Vixen sequence. Thus this step is very important! Add a row in the lower half of the Setup tab for each network used in your display. xLights can drive a mixture of network types (for example, the first network can be DMX, and the second one LOR, and the third one Renard). When you are finished, do not forget to SAVE YOUR CHANGES by clicking the 'Save Setup' button.");
        break;
    case TESTTAB:
        caption=_("Test Tab");
        msg=_("In order to use the xLights Test module, you must first define your lighting network(s) on the Setup tab. You should also click the 'Output to Lights' checkbox. Next you will need to select which channels you want to test. You can select channels one at a time by clicking on the checkbox next to each channel. You can select multiple channels at a time by using ")+selectmethod+_(", then clicking any checkbox in the selected range. Click the checkbox again to unselect all channels in the range. Once that is done, use the radio buttons and slide bars to control your testing. You can change channel selections while the test is running.");
        break;
    case CONVERTTAB:
        caption=_("Convert Tab");
        msg=_("Use the Convert tab to convert sequence files from one format to another. Available input formats include LOR (.lms or .las), Vixen (.vix), Lynx Conductor (.seq), and xLights (.xseq). Output formats include Vixen (.vix), Lynx Conductor (.seq), and xLights (.xseq). The xLights scheduler will only play xLights sequences and media files (music and video).\n\nThe lighting networks you define on the Setup tab will guide the conversion - make sure you have your lighting networks defined accurately before you start a conversion.\n\nAs the conversion takes place, messages will appear on the right side informing you about the progress and any errors found.");
        break;
    case SEQUENCETAB:
        caption=_("Sequence Tab");
        msg=_("The Sequence tab can be used to create RGB sequences. First, create a model of your RGB display element(s) by clicking on the Models button. Then try the different effects and settings until you create something you like. You can save the settings as a preset by clicking the New Preset button. From then on, that preset will be available in the presets drop-down list. You can combine effects by creating a second effect in the Effect 2 area, then choosing a Layering Method. To create a series of effects that will be used in a sequence, click the open file icon to open an xLights (.xseq) sequence. Choose which display elements/models you will use in this sequence. Then click the insert rows icon and type in the start time in seconds when that effect should begin. Rows will automatically sort by start time. To add an effect to the sequence, click on the grid cell in the desired display model column and the desired start time row, then click the Update button. When you are done creating effects for the sequence, click the save icon and the xLights sequence will be updated with the effects you stored in the grid.");
        break;
    case SCHEDULETAB:
        caption=_("Schedule Tab");
        msg=_("The xLights scheduler works by scheduling playlists. To create a playlist, select Playlist > Add from the menu. Once you have one or more playlists created, you are ready to build your schedule.\n\nStart building your schedule by defining your show start and end dates. Then click 'Schedule Playlist' to add playlists to the schedule. To alter the schedule, you may select multiple items using ")+selectmethod+_(". Don't worry if date headings get selected - they will be ignored. Then click 'Update Selected Items'.\n\nThe schedule is NOT SAVED until you click the 'Save Schedule' button (this also saves all playlists).\n\nThe schedule WILL NOT RUN unless the 'Run Schedule' checkbox is checked. You will also want to ensure 'Output to Lights' is checked.");
        break;
    default:
        // playlist
        caption=_("Playlist Tab");
        msg=_("Files in your show directory are listed on the left. Click the 'Audio', 'Video', or 'xLights' checkbox to see files of the desired type. Files in the playlist are listed on the right. To move files between the left and right lists, use the arrow buttons in the middle. You can rename or delete a list by using the Playlist menu. Selecting Playlist > Save Playlists from the menu will save all playlists as well as the schedule. You can play a single item on the playlist by selecting it and then clicking 'Play Item'. Use 'Play All' to play the entire playlist. You can reorder the playlist by dragging items up or down, or selecting items and using the up/down buttons.");
        break;
    }
    wxMessageBox(msg,caption);
}

void xLightsFrame::AllLightsOff()
{
    TestButtonsOff();
    if (xout) xout->alloff();
}

void xLightsFrame::OnNotebook1PageChanged(wxNotebookEvent& event)
{
    if (Notebook1->GetSelection() == TESTTAB && !xout)
    {
        StatusBar1->SetStatusText(_("Testing disabled - Output to Lights is not checked"));
    }
    if (event.GetOldSelection() == TESTTAB)
    {
        AllLightsOff();
    }
}


void xLightsFrame::OnButtonLightsOffClick(wxCommandEvent& event)
{
    AllLightsOff();
}

bool xLightsFrame::EnableOutputs()
{
    wxCriticalSectionLocker locker(gs_xoutCriticalSection);
    long MaxChan;
    bool ok=true;
    if (CheckBoxLightOutput->IsChecked() && xout==0)
    {
        xout = new xOutput();


        for( wxXmlNode* e=NetworkXML.GetRoot()->GetChildren(); e!=NULL && ok; e=e->GetNext() )
        {
            wxString tagname=e->GetName();
            if (tagname == wxT("network"))
            {
                wxString tempstr=e->GetAttribute(wxT("MaxChannels"), wxT("0"));
                tempstr.ToLong(&MaxChan);
                wxString NetworkType=e->GetAttribute(wxT("NetworkType"), wxT(""));
                wxString ComPort=e->GetAttribute(wxT("ComPort"), wxT(""));
                wxString BaudRate=e->GetAttribute(wxT("BaudRate"), wxT(""));
                int baud = (BaudRate == _("n/a")) ? 115200 : wxAtoi(BaudRate);
                wxString msg = _("Error occurred while connecting to ") + NetworkType+ _(" network on ") + ComPort +
                               _("\n\nThings to check:\n1. Are all required cables plugged in?") +
                               _("\n2. Is there another program running that is accessing the port (like the LOR Control Panel)? If so, then you must close the other program and then restart xLights.") +
                               _("\n3. If this is a USB dongle, are the FTDI Virtual COM Port drivers loaded?\n\n");
                try
                {
                    xout->addnetwork(NetworkType,MaxChan,ComPort,baud);
                    //TextCtrlLog->AppendText(_("Successfully initialized ") + NetworkType + _(" network on ") + ComPort + _("\n"));
                }
                catch (const char *str)
                {
                    wxString errmsg(str,wxConvUTF8);
                    wxMessageBox(msg + errmsg, _("Communication Error"));
                    ok = false;
                }
                catch (char *str)
                {
                    wxString errmsg(str,wxConvUTF8);
                    wxMessageBox(msg + errmsg, _("Communication Error"));
                    ok = false;
                }
            }
        }
        if (!ok)
        {
            // uncheck checkbox since we were not able to initialize the port(s) successfully
            delete xout;
            xout=0;
            CheckBoxLightOutput->SetValue(false);
            //CheckBoxLightOutput->Enable(false);
        }
    }
    else if (!CheckBoxLightOutput->IsChecked() && xout)
    {
        delete xout;
        xout=0;
    }
    EnableNetworkChanges();
    return ok;
}

void xLightsFrame::EnableNetworkChanges()
{
    bool flag=(xout==0 && !CurrentDir.IsEmpty());
    ButtonAddDongle->Enable(flag);
    ButtonAddE131->Enable(flag);
    ButtonNetworkChange->Enable(flag);
    ButtonNetworkDelete->Enable(flag);
    ButtonNetworkDeleteAll->Enable(flag);
    BitmapButtonMoveNetworkUp->Enable(flag);
    BitmapButtonMoveNetworkDown->Enable(flag);
    ButtonSaveSetup->Enable(!CurrentDir.IsEmpty());
    ButtonSaveSchedule->Enable(!CurrentDir.IsEmpty());
    CheckBoxLightOutput->Enable(!CurrentDir.IsEmpty());
    CheckBoxRunSchedule->Enable(!CurrentDir.IsEmpty());
}

void xLightsFrame::OnCheckBoxLightOutputClick(wxCommandEvent& event)
{
    EnableOutputs();
    CheckChannelList=true;  // cause status bar to be updated if in test mode
}


void xLightsFrame::OnButtonStopNowClick(wxCommandEvent& event)
{
    PlayerDlg->MediaCtrl->Stop();
    if (play_mode == play_sched)
    {
        CheckBoxRunSchedule->SetValue(false);
        CheckRunSchedule();
    }
    if (basic.IsRunning()) basic.halt();
    SetPlayMode(play_off);
    ResetTimer(NO_SEQ);
    switch (Notebook1->GetSelection())
    {
    case TESTTAB:
        TestButtonsOff();
        break;
    case SEQUENCETAB:
        EnableSequenceControls(true);
        break;
    }
}

void xLightsFrame::OnButtonGracefulStopClick(wxCommandEvent& event)
{
    if (play_mode == play_sched)
    {
        EndTimeSec = 0;
    }
    else if (basic.IsRunning())
    {
        SecondsRemaining = 0;
        StatusBar1->SetStatusText(_("Finishing playlist"));
    }
    else
    {
        wxMessageBox(_("Graceful Stop is only useful when a schedule or playlist is running"));
    }
}

void xLightsFrame::OnButtonSaveScheduleClick(wxCommandEvent& event)
{
    SaveScheduleFile();
}

void xLightsFrame::OnMenuItemSavePlaylistsSelected(wxCommandEvent& event)
{
    SaveScheduleFile();
}



#include "TabSetup.cpp"
#include "TabTest.cpp"
#include "TabConvert.cpp"
#include "TabSchedule.cpp"
#include "TabSequence.cpp"

void xLightsFrame::OnClose(wxCloseEvent& event)
{
    if (UnsavedChanges && wxNO == wxMessageBox("Quit without saving?",
            "Unsaved Changes", wxICON_QUESTION | wxYES_NO))
    {
        return;
    }
    Destroy();
}

void xLightsFrame::OnMenuItemBackupSelected(wxCommandEvent& event)
{
    wxString folderName;
    time_t cur;
    time(&cur);
    wxFileName newDirH;
    wxDateTime curTime(cur);
    wxString newDir = wxString::Format(wxT("Backup-%s-%s"),curTime.FormatISODate(),curTime.FormatISOTime());

    if ( wxNO == wxMessageBox(wxT("All xml files in your xlights directory will be backed up to \"")+CurrentDir+wxT("\\")+
                              newDir+wxT("\". Proceed?"),wxT("Backup"),wxICON_QUESTION | wxYES_NO))
    {
        return;
    }
    if (!newDirH.Mkdir(newDir))
    {
        wxMessageBox(wxT("Unable to create directory!"),"Error", wxICON_ERROR);
        return;
    }


    //CurrentDir
}
