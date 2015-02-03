#include "xLightsMain.h"
#include "SeqSettingsDialog.h"

static bool isXmlSequence(wxFileName &fname) {
    char buf[1024];
    wxFile file(fname.GetFullPath());
    int i = file.Read(buf, 1024);
    file.Close();
    if (wxString(buf, 0 , i).Contains("<xsequence")) {
        return true;
    }
    return false;
}

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
        wxFileName fseq_file;
        wxFileName xml_file;
        wxFileName media_file;

        if( selected_file.GetExt() == "xml" )
        {
            xml_file = selected_file;
            if( isXmlSequence(xml_file) ) find_media = false;
            fseq_file = filename;
            fseq_file.SetExt("fseq");
        }
        else if( selected_file.GetExt() == "fseq" )
        {
            fseq_file = selected_file;

            // search for matching xml file
            xml_file = filename;
            xml_file.SetExt("xml");
            if( xml_file.FileExists() )
            {
                if( isXmlSequence(xml_file) ) find_media = false;
            }
            else
            {
                // no xml exists so create a new one
                xLightsXmlFile new_xml_file;
                new_xml_file = xml_file;
                new_xml_file.CreateNew();
            }
        }

        // load the fseq data file if it exits
        xlightsFilename = fseq_file.GetFullPath();
        if( fseq_file.FileExists() )
        {
            xlightsFilename = fseq_file.GetFullPath();
            LoadFSEQ(xlightsFilename, media_file);
            DisplayXlightsFilename(xlightsFilename);
            SeqBaseChannel=1;
            SeqChanCtrlBasic=false;
            SeqChanCtrlColor=false;
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
            }
        }

        // assign global xml file object
        delete CurrentSeqXmlFile;
        CurrentSeqXmlFile = new xLightsXmlFile(xml_file);

        if( find_media && media_file.FileExists() )
        {
            // this dialog is not going to be displayed we only use the object for the MP3 extract info feature
            SeqSettingsDialog setting_dlg(this, CurrentSeqXmlFile, mediaDirectory, "");
            setting_dlg.SetMediaFilename(media_file.GetFullPath(), true);
            setting_dlg.SaveAll();
        }

        // setup media file
        SetMediaFilename(CurrentSeqXmlFile->GetMediaFile());
        if( find_media && media_file.FileExists() )
        {
            CurrentSeqXmlFile->SetSequenceDurationMS(mMediaLengthMS);
            CurrentSeqXmlFile->Save();
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
    bool loaded = false;
    bool save_media = false;

    SeqXmlFileName=xml_file.GetFullPath();

    // read xml
    xml_file.Load();
    if (xml_file.NeedsConversion())
    {
        if( !xml_file.Convert() )
        {
            wxMessageBox(wxString::Format("Failed to convert XML file: %s", xml_file.GetFullPath()),"Error");
            return false;
        }
        SeqSettingsDialog setting_dlg(this, &xml_file, mediaDirectory, wxT("Your XML file has been converted!"));
        if( mediaFilename != wxEmptyString )
        {
            save_media = true;
        }
        else
        {
            wxFileName detect_media(xml_file.GetFullPath());
            detect_media.SetExt("mp3");
            if( detect_media.FileExists() )
            {
                mediaFilename = detect_media.GetFullPath();
                save_media = true;
            }
        }
        if( save_media )
        {
            setting_dlg.SetMediaFilename(mediaFilename, true);
            setting_dlg.SaveAll();
        }
        setting_dlg.Fit();
        setting_dlg.ShowModal();
    }

    if( xml_file.IsLoaded() )
    {
        LoadSequencer(xml_file);
        Menu_Settings_Sequence->Enable(true);
        loaded = true;
    }
    return loaded;
}

void xLightsFrame::LoadFSEQ(const wxString& FileName, wxFileName& media_file)
{
    wxString mf;
    ReadFalconFile(FileName, &mf);
    media_file = mf;
}
