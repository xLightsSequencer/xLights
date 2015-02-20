#include "xLightsMain.h"
#include "SeqSettingsDialog.h"
#include "FileConverter.h"
#include "DataLayer.h"



void xLightsFrame::NewSequence()
{
    if (UnsavedChanges && wxNO == wxMessageBox("Sequence changes will be lost.  Do you wish to continue?",
                                               "Sequence Changed Confirmation", wxICON_QUESTION | wxYES_NO))
    {
        return;
    }

    // clear everything to prepare for new sequence
    mediaFilename.Clear();
    previewLoaded = false;
    previewPlaying = false;
    ResetTimer(NO_SEQ);
    ResetSequenceGrid();
    changedRow = 99999;
    changedColumn = 99999;

    // assign global xml file object
    wxFileName xml_file;
    delete CurrentSeqXmlFile;
    CurrentSeqXmlFile = new xLightsXmlFile(xml_file);

    SeqSettingsDialog setting_dlg(this, CurrentSeqXmlFile, mediaDirectory, wxT(""), true);
    setting_dlg.Fit();
    setting_dlg.ShowModal();

    // load media if available
    if( CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->HasAudioMedia() )
    {
        SetMediaFilename(CurrentSeqXmlFile->GetMediaFile());
    }

    wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
    int ms = atoi(mss.c_str());
    //SeqLoadXlightsFile(*CurrentSeqXmlFile, true);
    LoadSequencer(*CurrentSeqXmlFile);
    Menu_Settings_Sequence->Enable(true);

    if( (NetInfo.GetTotChannels() > SeqData.NumChannels()) ||
        (CurrentSeqXmlFile->GetSequenceDurationMS() / ms) > SeqData.NumFrames() )
    {
        SeqData.init(NetInfo.GetTotChannels(), mMediaLengthMS / ms, ms);
    }
    else
    {
        SeqData.init(NetInfo.GetTotChannels(), CurrentSeqXmlFile->GetSequenceDurationMS() / ms, ms);
    }
}

void xLightsFrame::OpenSequence()
{
    bool loaded_xml = false;
    bool loaded_fseq = false;
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
            ConvertParameters read_params(xlightsFilename,                              // input filename
                                          SeqData,                                      // sequence data object
                                          GetNetInfo(),                                 // global network info
                                          ConvertParameters::READ_MODE_LOAD_MAIN,       // file read mode
                                          this,                                         // xLights main frame
                                          &mf );                                        // media filename

            FileConverter::ReadFalconFile(read_params);
            if( mf != "" )
            {
                media_file = mf;
                find_media = false;
            }
            DisplayXlightsFilename(xlightsFilename);
            SeqBaseChannel=1;
            SeqChanCtrlBasic=false;
            SeqChanCtrlColor=false;
            loaded_fseq = true;
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

        // search for missing media file in media directory and show directory
        if( find_media )
        {
            wxFileName detect_media(selected_file);
            detect_media.SetExt("mp3");

            // search media directory
            detect_media.SetPath(mediaDirectory);
            if( detect_media.FileExists() )
            {
                media_file = detect_media;
                find_media = false;
            }
            else
            {
                // search selected file directory
                detect_media.SetPath(selected_file.GetPath());
                if( detect_media.FileExists() )
                {
                    media_file = detect_media;
                    find_media = false;
                }
            }
        }

        // if fseq or xseq had media update xml
        if( !CurrentSeqXmlFile->HasAudioMedia() && !find_media )
        {
            CurrentSeqXmlFile->SetMediaFile(media_file.GetFullPath(), true);
            int length_ms = Waveform::GetLengthOfMusicFileInMS(media_file.GetFullPath());
            CurrentSeqXmlFile->SetSequenceDurationMS(length_ms);
        }

        if( CurrentSeqXmlFile->WasConverted() )
        {
            SeqSettingsDialog setting_dlg(this, CurrentSeqXmlFile, mediaDirectory, wxT("V3 file was converted. Please check settings!"));
            setting_dlg.Fit();
            setting_dlg.ShowModal();
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
        else if( !loaded_fseq )
        {
            SeqData.init(NetInfo.GetTotChannels(), CurrentSeqXmlFile->GetSequenceDurationMS() / ms, ms);
        }

        if( loaded_fseq )
        {
            bbPlayPause->SetBitmap(playIcon);
            SliderPreviewTime->SetValue(0);
            TextCtrlPreviewTime->Clear();
            CompareMyDisplayToSeq();
            Timer1.Start(SeqData.FrameTime());
        }
        else if( !loaded_xml )
        {
            StatusBar1->SetStatusText(wxString::Format("Failed to load: '%s'.", filename));
            return;
        }

        float elapsedTime = sw.Time()/1000.0; //msec => sec
        StatusBar1->SetStatusText(wxString::Format("'%s' loaded in %4.3f sec.", filename, elapsedTime));
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

void xLightsFrame::ClearSequenceData()
{
    for( int i = 0; i < SeqData.NumFrames(); ++i)
        for( int j = 0; j < SeqData.NumChannels(); ++j )
            SeqData[i][j] = 0;
}

void xLightsFrame::RenderIseqData(bool bottom_layers)
{
    DataLayerSet& data_layers = CurrentSeqXmlFile->GetDataLayers();
    ConvertParameters::ReadMode read_mode;
    if( bottom_layers )
    {
        ClearSequenceData();
        read_mode = ConvertParameters::READ_MODE_NORMAL;
    }
    else
    {
        read_mode = ConvertParameters::READ_MODE_IGNORE_BLACK;
    }
    int layers_rendered = 0;
    bool start_rendering = bottom_layers;
    for( int i = data_layers.GetNumLayers() - 1; i >= 0; --i )  // build layers bottom up
    {
        DataLayer* data_layer = data_layers.GetDataLayer(i);

        if( data_layer->GetName() != "Nutcracker" )
        {
            if( start_rendering )
            {
                ConvertParameters read_params(data_layer->GetDataSource(),                // input filename
                                              SeqData,                                    // sequence data object
                                              GetNetInfo(),                               // global network info
                                              read_mode,                                  // file read mode
                                              this,                                       // xLights main frame
                                              nullptr,                                    // filename not needed
                                              data_layer );                               // provide data layer for channel offsets

                FileConverter::ReadFalconFile(read_params);
                read_mode = ConvertParameters::READ_MODE_IGNORE_BLACK;
                layers_rendered++;
            }
        }
        else
        {
            if( bottom_layers ) break;  // exit after Nutcracker layer if rendering bottom layers only
            start_rendering = true;
        }
    }
}
