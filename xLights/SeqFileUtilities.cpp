#include "xLightsMain.h"
#include "SeqSettingsDialog.h"
#include "FileConverter.h"
#include "DataLayer.h"



void xLightsFrame::NewSequence()
{
    // close any open sequences
    CloseSequence();

    // assign global xml file object
    wxFileName xml_file;
    xml_file.SetPath(CurrentDir);
    CurrentSeqXmlFile = new xLightsXmlFile(xml_file);

    SeqSettingsDialog setting_dlg(this, CurrentSeqXmlFile, mediaDirectory, wxT(""), true);
    setting_dlg.Fit();
    int ret_code = setting_dlg.ShowModal();
    if( ret_code == wxID_CANCEL )
    {
        delete CurrentSeqXmlFile;
        CurrentSeqXmlFile = NULL;
        return;
    }

    // load media if available
    if( CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->HasAudioMedia() )
    {
        SetMediaFilename(CurrentSeqXmlFile->GetMediaFile());
    }

    wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
    int ms = atoi(mss.c_str());
    LoadSequencer(*CurrentSeqXmlFile);
    CurrentSeqXmlFile->SetSequenceLoaded(true);
    CurrentSeqXmlFile->AddNewTimingSection("New Timing", this);
    Menu_Settings_Sequence->Enable(true);
    MenuItem_File_Save_Sequence->Enable(true);
    MenuItem_File_Close_Sequence->Enable(true);

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
static wxFileName mapFileName(const wxFileName &orig) {
    if (orig.GetDirCount() == 0) {
        //likely a filename from windows on Mac/Linux or vice versa
        int idx = orig.GetFullName().Last('\\');
        if (idx == -1) {
            idx = orig.GetFullName().Last('/');
        }
        if (idx != -1) {
            return wxFileName(orig.GetFullName().Left(idx),
                              orig.GetFullName().Right(orig.GetFullName().Length() - idx - 1));
        }
    }
    return orig;
}

void xLightsFrame::OpenSequence()
{
    bool loaded_xml = false;
    bool loaded_fseq = false;
    wxString wildcards = "XML files (*.xml)|*.xml|FSEQ files (*.fseq)|*.fseq";
    wxString filename = wxFileSelector("Choose sequence file to open", CurrentDir, wxEmptyString, "*.xml", wildcards, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if ( !filename.empty() )
    {
        // close any open sequences
        CloseSequence();

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
                media_file = mapFileName(wxFileName::FileName(mf));
            }
            DisplayXlightsFilename(xlightsFilename);
            SeqBaseChannel=1;
            SeqChanCtrlBasic=false;
            SeqChanCtrlColor=false;
            loaded_fseq = true;
        }

        // assign global xml file object
        CurrentSeqXmlFile = new xLightsXmlFile(xml_file);

        // open the xml file so we can see if it has media
        CurrentSeqXmlFile->Open();

        // if fseq didn't have media check xml
        if( CurrentSeqXmlFile->HasAudioMedia()
           || !CurrentSeqXmlFile->GetMediaFile().IsEmpty())
        {
            media_file = mapFileName(CurrentSeqXmlFile->GetMediaFile());
        }


        // still no media file?  look for an XSEQ file and load if found
        if( !wxFileName(media_file).Exists() )
        {
            wxFileName xseq_file = selected_file;
            xseq_file.SetExt("xseq");
            if( xseq_file.FileExists() )
            {
                wxString mf;
                ReadXlightsFile(xseq_file.GetFullPath(), &mf);
                if( mf != "" )
                {
                    media_file = mapFileName(wxFileName::FileName(mf));
                }
                DisplayXlightsFilename(xlightsFilename);
                SeqBaseChannel=1;
                SeqChanCtrlBasic=false;
                SeqChanCtrlColor=false;
            }
        }

        // double-check file existence
        if( !wxFileName(media_file).Exists() )
        {
            wxFileName detect_media(media_file);

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
                if( detect_media.FileExists() )
                {
                    media_file = detect_media;
                }
            }
        }

        // search for missing media file in media directory and show directory
        if( !wxFileName(media_file).Exists() )
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
                if( detect_media.FileExists() )
                {
                    media_file = detect_media;
                }
            }
        }

        // if fseq or xseq had media update xml
        if( !CurrentSeqXmlFile->HasAudioMedia() && wxFileName(media_file).Exists() )
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
        EnableSequenceControls(true);
    }
}

void xLightsFrame::CloseSequence()
{
    if (UnsavedChanges && wxNO == wxMessageBox("Sequence changes will be lost.  Do you wish to continue?",
                                               "Sequence Changed Confirmation", wxICON_QUESTION | wxYES_NO))
    {
        return;
    }

    // clear everything to prepare for new sequence
    xlightsFilename = "";
    mediaFilename.Clear();
    previewLoaded = false;
    previewPlaying = false;
    ResetTimer(NO_SEQ);
    ResetSequenceGrid();
    changedRow = 99999;
    changedColumn = 99999;
    playType = 0;
    selectedEffect = NULL;
    if( CurrentSeqXmlFile )
    {
        delete CurrentSeqXmlFile;
        CurrentSeqXmlFile = NULL;
    }
    mSequenceElements.Clear();

    mainSequencer->PanelWaveForm->CloseMediaFile();

    EnableSequenceControls(true);  // let it re-evaluate menu state
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
        xml_file.SetSequenceLoaded(true);
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
    if (bottom_layers &&  data_layers.GetNumLayers() == 1 &&
        data_layers.GetDataLayer(0)->GetName() == "Nutcracker") {
        //Don't clear, v3 workflow of augmenting the existing fseq file
        return;
    }

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

void xLightsFrame::SetSequenceEnd(int ms)
{
    mainSequencer->PanelTimeLine->SetSequenceEnd(CurrentSeqXmlFile->GetSequenceDurationMS());
}

static void CalcPercentage(wxString& value, double base, bool reverse)
{
    double val;
    value.ToDouble(&val);
    double percent = val/(base-1)*100.0;
    if( reverse )
    {
        percent = 100.0 - percent;
    }
    value = wxString::Format("%d",(int)percent);
}

static wxString GetColorString(wxString& sRed, wxString& sGreen, wxString& sBlue)
{
    double val,red,green,blue;
    sRed.ToDouble(&red);
    red = red / 100.0 * 255.0;
    sGreen.ToDouble(&green);
    green = green / 100.0 * 255.0;
    sBlue.ToDouble(&blue);
    blue = blue / 100.0 * 255.0;
    xlColor color(red, green, blue);
    return (wxString)color;
}

static EffectLayer* FindOpenLayer(Element* model, int layer_index, double start_time, double end_time)
{
    EffectLayer* layer;
    int index = layer_index-1;

    layer = model->GetEffectLayer(index);
    if( layer->GetRangeIsClear(start_time, end_time) )
    {
        return layer;
    }

    // need to search for open layer
    for( int i = 0; i < model->GetEffectLayerCount(); i++ )
    {
        layer = model->GetEffectLayer(i);
        if( layer->GetRangeIsClear(start_time, end_time) )
        {
            return layer;
        }
    }

    // empty layer not found so create a new one
    layer = model->AddEffectLayer();
    return layer;
}

void xLightsFrame::ImportSuperStar()
{
    wxString model_name = ChoiceSuperStarImportModel->GetStringSelection();
    if( model_name == "" )
    {
        wxMessageBox("Please select the target model!");
        return;
    }

    wxString wildcards = "SuperStar files (*.sup)|*.sup";
    wxString filename = wxFileSelector("Choose SuperStar file to open", CurrentDir, wxEmptyString, "*.xml", wildcards, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if ( !filename.empty() )
    {
        wxStopWatch sw; // start a stopwatch timer
        double num_rows = 1.0;
        double num_columns = 1.0;
        bool reverse_rows = false;
        bool layout_defined = false;
        bool model_found = false;

        // read v3 xml file into temporary document
        wxFileName xml_file(filename);
        wxXmlDocument input_xml;
        wxString xml_doc = xml_file.GetFullPath();
        if( !input_xml.Load(xml_doc) )  return;

        Element* model = nullptr;

        for(int i=0;i<mSequenceElements.GetElementCount();i++)
        {
            if(mSequenceElements.GetElement(i)->GetType()== "model")
            {
                model = mSequenceElements.GetElement(i);
                if( model->GetName() == model_name )
                {
                    model_found = true;
                    break;
                }
            }
        }
        if( model != nullptr && model_found )
        {
            int morph_index = Effect::GetEffectIndex("Morph");
            EffectLayer* layer = model->AddEffectLayer();
            wxXmlNode* input_root=input_xml.GetRoot();

            for(wxXmlNode* e=input_root->GetChildren(); e!=NULL; e=e->GetNext() )
            {
                if (e->GetName() == "layouts")
                {
                    wxXmlNode* element=e->GetChildren();
                    double value;
                    wxString attr;
                    element->GetAttribute("nbrOfRibbons", &attr);
                    attr.ToDouble(&num_columns);
                    element->GetAttribute("ribbonType", &attr);
                    if( attr == "CCR" )
                    {
                        num_rows = 50.0;
                    }
                    else
                    {
                        wxMessageBox("Currently only CCRs are supported!");
                        return;
                    }
                    element->GetAttribute("controllerLocation", &attr);
                    if( attr == "bottom" )
                    {
                        reverse_rows = true;
                    }
                    layout_defined = true;
                }
                if (e->GetName() == "morphs")
                {
                    if( !layout_defined )
                    {
                        wxMessageBox("The layouts section was not found in the SuperStar file!");
                        return;
                    }
                    for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
                    {
                        int layer_index;
                        double layer_val;
                        wxString name_attr;
                        wxString acceleration;
                        wxString state1_time, state2_time, ramp_time_ext, attr;
                        double start_time, end_time, ramp_time;
                        element->GetAttribute("name", &name_attr);
                        element->GetAttribute("acceleration", &acceleration);
                        element->GetAttribute("layer", &attr);
                        attr.ToDouble(&layer_val);
                        layer_index = (int)layer_val;
                        wxXmlNode* state1=element->GetChildren();
                        wxXmlNode* state2=state1->GetNext();
                        wxXmlNode* ramp=state2->GetNext();
                        state1->GetAttribute("time", &state1_time);
                        state2->GetAttribute("time", &state2_time);
                        ramp->GetAttribute("timeExt", &ramp_time_ext);
                        state1_time.ToDouble(&start_time);
                        state2_time.ToDouble(&end_time);
                        ramp_time_ext.ToDouble(&ramp_time);
                        start_time /= 100.0;
                        end_time /= 100.0;
                        ramp_time /= 100.0;
                        end_time += ramp_time;
                        double head_duration = (1.0 - ramp_time/(end_time-start_time)) * 100.0;
                        wxString settings = "E_CHECKBOX_MorphUseHeadEndColor=0,E_CHECKBOX_MorphUseHeadStartColor=0,E_CHECKBOX_Morph_End_Link=0,E_CHECKBOX_Morph_Start_Link=0,E_CHECKBOX_ShowHeadAtStart=0,E_NOTEBOOK_Morph=Start,E_SLIDER_MorphAccel=";
                        settings += acceleration + ",";
                        wxString duration = wxString::Format("E_SLIDER_MorphDuration=%d,",(int)head_duration);
                        settings += duration;
                        state2->GetAttribute("trailLen", &attr);
                        settings += "E_SLIDER_MorphEndLength=" + attr + ",";
                        state1->GetAttribute("trailLen", &attr);
                        settings += "E_SLIDER_MorphStartLength=" + attr + ",";
                        state2->GetAttribute("x1", &attr);
                        CalcPercentage(attr, num_columns, false);
                        settings += "E_SLIDER_Morph_End_X1=" + attr + ",";
                        state2->GetAttribute("x2", &attr);
                        CalcPercentage(attr, num_columns, false);
                        settings += "E_SLIDER_Morph_End_X2=" + attr + ",";
                        state2->GetAttribute("y1", &attr);
                        CalcPercentage(attr, num_rows, reverse_rows);
                        settings += "E_SLIDER_Morph_End_Y1=" + attr + ",";
                        state2->GetAttribute("y2", &attr);
                        CalcPercentage(attr, num_rows, reverse_rows);
                        settings += "E_SLIDER_Morph_End_Y2=" + attr + ",";
                        state1->GetAttribute("x1", &attr);
                        CalcPercentage(attr, num_columns, false);
                        settings += "E_SLIDER_Morph_Start_X1=" + attr + ",";
                        state1->GetAttribute("x2", &attr);
                        CalcPercentage(attr, num_columns, false);
                        settings += "E_SLIDER_Morph_Start_X2=" + attr + ",";
                        state1->GetAttribute("y1", &attr);
                        CalcPercentage(attr, num_rows, reverse_rows);
                        settings += "E_SLIDER_Morph_Start_Y1=" + attr + ",";
                        state1->GetAttribute("y2", &attr);
                        CalcPercentage(attr, num_rows, reverse_rows);
                        settings += "E_SLIDER_Morph_Start_Y2=" + attr + ",";
                        settings += "T_CHECKBOX_FitToTime=0,T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,T_CHOICE_LayerMethod=1 reveals 2,T_SLIDER_EffectLayerMix=0,T_SLIDER_Speed=10,T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00";
                        wxString sRed, sGreen, sBlue,color;
                        state1->GetAttribute("red", &sRed);
                        state1->GetAttribute("green", &sGreen);
                        state1->GetAttribute("blue", &sBlue);
                        color = GetColorString(sRed, sGreen, sBlue);
                        wxString palette = "C_BUTTON_Palette1=" + color + ",";
                        state2->GetAttribute("red", &sRed);
                        state2->GetAttribute("green", &sGreen);
                        state2->GetAttribute("blue", &sBlue);
                        color = GetColorString(sRed, sGreen, sBlue);
                        palette += "C_BUTTON_Palette2=" + color + ",";
                        ramp->GetAttribute("red1", &sRed);
                        ramp->GetAttribute("green1", &sGreen);
                        ramp->GetAttribute("blue1", &sBlue);
                        color = GetColorString(sRed, sGreen, sBlue);
                        palette += "C_BUTTON_Palette3=" + color + ",";
                        ramp->GetAttribute("red2", &sRed);
                        ramp->GetAttribute("green2", &sGreen);
                        ramp->GetAttribute("blue2", &sBlue);
                        color = GetColorString(sRed, sGreen, sBlue);
                        palette += "C_BUTTON_Palette4=" + color + ",";
                        palette += "C_BUTTON_Palette5=#FFFFFF,C_BUTTON_Palette6=#000000,C_CHECKBOX_Palette1=1,C_CHECKBOX_Palette2=1,C_CHECKBOX_Palette3=1,C_CHECKBOX_Palette4=1,";
                        palette += "C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,C_SLIDER_Brightness=100,C_SLIDER_Contrast=0,C_SLIDER_SparkleFrequency=0";
                        while( model->GetEffectLayerCount() < layer_index )
                        {
                            model->AddEffectLayer();
                        }
                        layer = FindOpenLayer(model, layer_index, start_time, end_time);
                        layer->AddEffect(0, morph_index, "Morph", settings, palette, start_time, end_time, false, false);
                    }
                }
            }
            float elapsedTime = sw.Time()/1000.0; //msec => sec
            StatusBar1->SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename, elapsedTime));
        }
        else
        {
            wxMessageBox(wxString::Format("Model %s was not found. Check that sequence is open and model is in the grid.", model_name));
        }
    }
}
