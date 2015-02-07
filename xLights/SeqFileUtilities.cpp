#include "xLightsMain.h"
#include "SeqSettingsDialog.h"

void xLightsFrame::OpenSequence()
{
    bool loaded_xml = false;
    bool find_media = true;
    wxString wildcards = "XML files (*.xml)|*.xml|FSEQ files (*.fseq)|*.fseq";
    wxString filename = wxFileSelector("Choose sequence file to open", CurrentDir, wxEmptyString, "*.xml", wildcards, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if ( !filename.empty() )
    {
        // clear everything to prepare for new sequence
        mediaFilename.Clear();
        previewLoaded = false;
        previewPlaying = false;
        ResetTimer(NO_SEQ);
        ResetSequenceGrid();
        changedRow = 99999;
        changedColumn = 99999;
        wxStopWatch sw; // start a stopwatch timer

        wxFileName selected_file(filename);
        wxFileName fseq_file = selected_file;
        fseq_file.SetExt("fseq");
        wxFileName xml_file = selected_file;
        xml_file.SetExt("xml");
        wxFileName media_file;

        // load the fseq data file if it exists
        xlightsFilename = fseq_file.GetFullPath();
        if( fseq_file.FileExists() )
        {
            wxString mf;
            ReadFalconFile(xlightsFilename, &mf);
            if( mf != "" )
            {
                media_file = mf;
                find_media = false;
            }
            DisplayXlightsFilename(xlightsFilename);
            SeqBaseChannel=1;
            SeqChanCtrlBasic=false;
            SeqChanCtrlColor=false;
        }

        // assign global xml file object
        delete CurrentSeqXmlFile;
        CurrentSeqXmlFile = new xLightsXmlFile(xml_file);

        // open the xml file so we can see if it has media
        CurrentSeqXmlFile->Open();

        // if fseq didn't have media check xml
        if( CurrentSeqXmlFile->HasAudioMedia() )
        {
            media_file = CurrentSeqXmlFile->GetMediaFile();
            find_media = false;
        }

        // still no media file?  look for an XSEQ file and load if found
        if( find_media )
        {
            wxFileName xseq_file = selected_file;
            xseq_file.SetExt("xseq");
            if( xseq_file.FileExists() )
            {
                wxString mf;
                ReadXlightsFile(xseq_file.GetFullPath(), &mf);
                if( mf != "" )
                {
                    media_file = mf;
                    find_media = false;
                }
                DisplayXlightsFilename(xlightsFilename);
                SeqBaseChannel=1;
                SeqChanCtrlBasic=false;
                SeqChanCtrlColor=false;
            }
        }

        // if fseq or xseq had media update xml
        if( !CurrentSeqXmlFile->HasAudioMedia() && !find_media )
        {
            CurrentSeqXmlFile->SetMediaFile(media_file.GetFullPath(), true);
            CurrentSeqXmlFile->SetSequenceDurationMS(mMediaLengthMS);
        }

        if( CurrentSeqXmlFile->WasConverted() )
        {
            SeqSettingsDialog setting_dlg(this, CurrentSeqXmlFile, mediaDirectory, wxT("V3 file was converted. Please check settings!"));
            setting_dlg.Fit();
            setting_dlg.ShowModal();
            if( CurrentSeqXmlFile->HasAudioMedia() )  find_media = false;  // user may have set audio file in dialog
        }

        // search for missing media file
        if( find_media )
        {
            if( !media_file.FileExists() )
            {
                wxFileName detect_media(selected_file);
                detect_media.SetExt("mp3");

                // search media directory
                detect_media.SetPath(mediaDirectory);
                if( detect_media.FileExists() )
                {
                    media_file = detect_media;
                }
                else
                {
                    // search selected file directory
                    detect_media.SetPath(selected_file.GetPath());
                    if( detect_media.FileExists() ) media_file = detect_media;
                }
                if( media_file.FileExists() )  // if media was found update xml data
                {
                    CurrentSeqXmlFile->SetMediaFile(media_file.GetFullPath(), true);
                    CurrentSeqXmlFile->SetSequenceDurationMS(mMediaLengthMS);
                }
            }
        }

        // load media if available
        if( CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->HasAudioMedia() )
        {
            SetMediaFilename(CurrentSeqXmlFile->GetMediaFile());
        }

        wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
        int ms = atoi(mss.c_str());
        loaded_xml = SeqLoadXlightsFile(*CurrentSeqXmlFile, true);

        if( (NetInfo.GetTotChannels() > SeqData.NumChannels()) ||
            (CurrentSeqXmlFile->GetSequenceDurationMS() / ms) > SeqData.NumFrames() )
        {
            SeqData.init(NetInfo.GetTotChannels(), mMediaLengthMS / ms, ms);
        }

        if( loaded_xml )
        {
            bbPlayPause->SetBitmap(playIcon);
            SliderPreviewTime->SetValue(0);
            TextCtrlPreviewTime->Clear();
            CompareMyDisplayToSeq();
            Timer1.Start(SeqData.FrameTime());
            float elapsedTime = sw.Time()/1000.0; //msec => sec
            StatusBar1->SetStatusText(wxString::Format("'%s' loaded in %4.3f sec.", filename, elapsedTime));
        }
        else
        {
            StatusBar1->SetStatusText(wxString::Format("Failed to load: '%s'.", filename));
        }
    }
}

bool xLightsFrame::SeqLoadXlightsFile(const wxString& filename, bool ChooseModels)
{
    delete xLightsFrame::CurrentSeqXmlFile;
    xLightsFrame::CurrentSeqXmlFile = new xLightsXmlFile(filename);
    return SeqLoadXlightsFile(*xLightsFrame::CurrentSeqXmlFile, ChooseModels);
}

// Load the xml file containing effects for a particular sequence
// Returns true if file exists and was read successfully
bool xLightsFrame::SeqLoadXlightsFile(xLightsXmlFile& xml_file, bool ChooseModels )
{
    if( xml_file.IsOpen() )
    {
        LoadSequencer(xml_file);
        Menu_Settings_Sequence->Enable(true);
        return true;
    }

    return false;
}
