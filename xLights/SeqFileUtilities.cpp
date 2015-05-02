#include "xLightsMain.h"
#include "SeqSettingsDialog.h"
#include "FileConverter.h"
#include "DataLayer.h"

#include "LMSImportChannelMapDialog.h"

#include <wx/wfstream.h>

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
            UpdateModelsList();
            UpdatePreview();
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
    MenuSettings->Enable(ID_MENUITEM_RENDER_MODE, false);
    Menu_Settings_Sequence->Enable(false);
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
    if (bottom_layers && data_layers.GetNumLayers() == 1 &&
        data_layers.GetDataLayer(0)->GetName() == "Nutcracker") {
        DataLayer* nut_layer = data_layers.GetDataLayer(0);
        if( nut_layer->GetDataSource() == xLightsXmlFile::CANVAS_MODE ) {
            //Don't clear, v3 workflow of augmenting the existing fseq file
            return;
        }
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

#define MAXBUFSIZE 4096
class FixXMLInputStream : public wxInputStream {
public:
    FixXMLInputStream(wxInputStream & in) : wxInputStream(), bin(in) {
    }
    void fillBuf() {
        int pos = bufLen;
        int sz =  MAXBUFSIZE - bufLen;
        bin.Read(&buf[pos], sz);
        size_t ret = bin.LastRead();
        bufLen += ret;
        
        bool needToClose = false;
        for (int x = 7; x < bufLen; x++) {
            if (buf[x-7] == '<'
                && buf[x-6] == 'p'
                && buf[x-5] == 'i'
                && buf[x-4] == 'x'
                && buf[x-3] == 'e'
                && buf[x-2] == 'l'
                && buf[x-1] == 's'
                && buf[x] == '=') {
                buf[x-2] = ' ';
            } else if (buf[x-7] == '<'
                       && buf[x-6] == 't'
                       && buf[x-5] == 'i'
                       && buf[x-4] == 'm'
                       && buf[x-3] == 'i'
                       && buf[x-2] == 'n'
                       && buf[x-1] == 'g'
                       && buf[x] == ' ') {
                needToClose = true;
            } else if (x > 12 &&
                       buf[x-12] == '<'
                       && buf[x-11] == 'i'
                       && buf[x-10] == 'm'
                       && buf[x-9] == 'a'
                       && buf[x-8] == 'g'
                       && buf[x-7] == 'e'
                       && buf[x-6] == 'A'
                       && buf[x-5] == 'c'
                       && buf[x-4] == 't'
                       && buf[x-3] == 'i'
                       && buf[x-2] == 'o'
                       && buf[x-1] == 'n'
                       && buf[x] == ' ') {
                needToClose = true;
            } else if (buf[x-1] == '>' && needToClose) {
                if (buf[x-2] != '/') {
                    buf[x - 1] = '/';
                    buf[x] = '>';
                }
                needToClose = false;
            }
        }
    }
    
    virtual size_t OnSysRead(void *buffer, size_t bufsize) {
        unsigned char *b = (unsigned char *)buffer;
        if (bufsize > 1024) {
            bufsize = 1024;
        }
        size_t ret = 0;
        if (bufLen < 2000) {
            fillBuf();
        }
        
        if (bufLen) {
            ret = std::min(bufsize, bufLen);
            memcpy(b, buf, ret);
            for (int x = ret; x < bufLen; x++) {
                buf[x-ret] = buf[x];
            }
            bufLen -= ret;
            buf[bufLen] = 0;
            return ret;
        }
        return 0;
    }
    
private:
    wxBufferedInputStream bin;
    unsigned char buf[MAXBUFSIZE];
    size_t bufLen = 0;
};

void xLightsFrame::ImportSuperStar()
{
    wxString model_name = ChoiceSuperStarImportModel->GetStringSelection();
    if( model_name == "" )
    {
        wxMessageBox("Please select the target model!");
        return;
    }

    wxString wildcards = "SuperStar files (*.sup)|*.sup|LOR Sequence(*.lms)|*.lms";
    wxString filename = wxFileSelector("Choose SuperStar file to open", CurrentDir, wxEmptyString, "*.xml", wildcards, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if ( !filename.empty() )
    {
        wxStopWatch sw; // start a stopwatch timer
        bool model_found = false;

        // read v3 xml file into temporary document
        wxFileName xml_file(filename);
        wxXmlDocument input_xml;
        wxString xml_doc = xml_file.GetFullPath();
        wxFileInputStream fin(xml_doc);
        FixXMLInputStream bufIn(fin);
        
        if( !input_xml.Load(bufIn) )  return;

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
            if (input_xml.GetRoot()->GetName() == "sequence") {
                if (!ImportLMS(model, input_xml)) {
                    return;
                }
            } else if (!ImportSuperStar(model, input_xml)) {
                return;
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

bool findRGB(wxXmlNode *e, wxXmlNode *chan, wxXmlNode *&rchannel, wxXmlNode *&gchannel, wxXmlNode *&bchannel) {
    wxString idxs[3];
    int cnt = 0;
    for (wxXmlNode *n = chan->GetChildren(); n != nullptr; n = n->GetNext()) {
        if (n->GetName() == "channels") {
            for (wxXmlNode *n2 = n->GetChildren(); n2 != nullptr; n2 = n2->GetNext()) {
                if (n2->GetName() == "channel" && cnt < 3) {
                    idxs[cnt] = n2->GetAttribute("savedIndex");
                    cnt++;
                }
            }
        }
    }
    for (wxXmlNode* ch=e->GetChildren(); ch!=NULL; ch=ch->GetNext()) {
        if (ch->GetName() == "channel") {
            wxString idx = ch->GetAttribute("savedIndex");
            if (idx == idxs[0]) {
                rchannel = ch;
            }
            if (idx == idxs[1]) {
                gchannel = ch;
            }
            if (idx == idxs[2]) {
                bchannel = ch;
            }
        }
    }
    return true;
}

void GetRGBTimes(wxXmlNode *re, int &startms, int &endms) {
    if (re != nullptr) {
        startms = wxAtoi(re->GetAttribute("startCentisecond")) * 10;
        endms = wxAtoi(re->GetAttribute("endCentisecond")) * 10;
    } else {
        startms = 9999999;
        endms = 9999999;
    }
}
void GetIntensities(wxXmlNode *re, int &starti, int &endi) {
    wxString intensity = re->GetAttribute("intensity", "-1");
    if (intensity == "-1") {
        starti = wxAtoi(re->GetAttribute("startIntensity"));
        endi = wxAtoi(re->GetAttribute("endIntensity"));
    } else {
        starti = endi = wxAtoi(intensity);
    }
}
bool GetRGBEffectData(wxXmlNode* &re, wxXmlNode* &ge, wxXmlNode* &be, int &startms, int &endms, xlColor &sc, xlColor &ec) {
    int starts[3];
    int ends[3];
    bool isShimmer = false;
    wxXmlNode *nodes[3] = {re,ge,be};
    GetRGBTimes(re, starts[0], ends[0]);
    GetRGBTimes(ge, starts[1], ends[1]);
    GetRGBTimes(be, starts[2], ends[2]);
    startms = std::min(starts[0], std::min(starts[1], starts[2]));
    endms = 99999999;
    unsigned char startColor[3] = {0,0,0};
    unsigned char endColor[3] = {0,0,0};
    for (int x = 0; x < 3; x++) {
        if (startms == starts[x]) {
            endms = std::min(endms, ends[x]);
            int starti, endi;
            GetIntensities(nodes[x], starti, endi);
            isShimmer = nodes[x]->GetAttribute("type") == "shimmer";
            startColor[x] = starti * 255 / 100;
            endColor[x] = endi * 255 / 100;
            switch (x) {
                case 0:
                    re = re->GetNext();
                    break;
                case 1:
                    ge = ge->GetNext();
                    break;
                case 2:
                    be = be->GetNext();
                    break;
            }
        }
    }
    sc.Set(startColor[0], startColor[1], startColor[2]);
    ec.Set(endColor[0], endColor[1], endColor[2]);
    return isShimmer;
}

void MapRGBEffects(EffectLayer *layer, wxXmlNode *rchannel, wxXmlNode *gchannel, wxXmlNode *bchannel) {
    wxXmlNode* re=rchannel->GetChildren();
    while (re != nullptr && "effect" != re->GetName()) re = re->GetNext();
    wxXmlNode* ge=gchannel->GetChildren();
    while (ge != nullptr && "effect" != ge->GetName()) ge = ge->GetNext();
    wxXmlNode* be=bchannel->GetChildren();
    while (be != nullptr && "effect" != be->GetName()) be = be->GetNext();
    
    int cwIndex = Effect::GetEffectIndex("Color Wash");
    int shimmerIndex = Effect::GetEffectIndex("Shimmer");
    int onIndex = Effect::GetEffectIndex("On");

    while (re != nullptr || ge != nullptr || be != nullptr) {
        int start, end;
        xlColor sc, ec;
        bool isShimmer = GetRGBEffectData(re, ge, be, start, end, sc, ec);
        double starttime = ((double)start) / 1000.0;
        double endtime = ((double)end) / 1000.0;
        wxString palette = "C_BUTTON_Palette1=" + sc + ",C_CHECKBOX_Palette1=1,"
            + "C_BUTTON_Palette2=" + ec + ",C_CHECKBOX_Palette2=1,"
            + "C_CHECKBOX_Palette3=0,C_CHECKBOX_Palette4=0,C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,"
            + "C_SLIDER_Brightness=100,C_SLIDER_Contrast=0,C_SLIDER_SparkleFrequency=0";
        
        if (isShimmer && sc == ec) {
            wxString settings = _("E_CHECKBOX_Shimmer_Blink_Timing=0,E_CHECKBOX_Shimmer_Use_All_Colors=0,E_SLIDER_Shimmer_Blinks_Per_Row=1,")
                + "E_SLIDER_Shimmer_Duty_Factor=60,T_CHECKBOX_FitToTime=1,T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                + "T_CHOICE_LayerMethod=Effect 1,T_SLIDER_EffectLayerMix=0,T_SLIDER_Speed=30,T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00";
            layer->AddEffect(0, shimmerIndex, "Shimmer", settings, palette, starttime, endtime, false, false);
        //} else if (isShimmer) {  //what to do with a color wash with a shimmer?
        } else if (ec == sc) {
            wxString settings = _("E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=100")
                + ",T_CHECKBOX_FitToTime=1,T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                + "T_CHOICE_LayerMethod=Effect 1,T_SLIDER_EffectLayerMix=0,T_SLIDER_Speed=10,"
                + "T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00";
            layer->AddEffect(0, onIndex, "On", settings, palette, starttime, endtime, false, false);
        } else {
            wxString settings = _("E_CHECKBOX_ColorWash_HFade=0,E_CHECKBOX_ColorWash_VFade=0,E_SLIDER_ColorWash_Count=1,T_CHECKBOX_FitToTime=1,")
                + "T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,T_CHOICE_LayerMethod=Effect 1,"
                + "T_SLIDER_EffectLayerMix=0,T_SLIDER_Speed=10,T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00";
            layer->AddEffect(0, cwIndex, "Color Wash", settings, palette, starttime, endtime, false, false);
        }
    }
}
void MapOnEffects(EffectLayer *layer, wxXmlNode *channel, int chancountpernode) {
    wxString palette = _("C_BUTTON_Palette1=#FFFFFF,C_CHECKBOX_Palette1=1,")
        + "C_CHECKBOX_Palette2=0,C_CHECKBOX_Palette3=0,C_CHECKBOX_Palette4=0,C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,"
        + "C_SLIDER_Brightness=100,C_SLIDER_Contrast=0,C_SLIDER_SparkleFrequency=0";
    if (chancountpernode > 1) {
        int rgb = wxAtoi(channel->GetAttribute("color"));
        wxString c = wxString::Format("#%06lx",rgb);
        xlColor color(c);
        palette = "C_BUTTON_Palette1=" + color + ",C_CHECKBOX_Palette1=1,"
            + "C_CHECKBOX_Palette2=0,C_CHECKBOX_Palette3=0,C_CHECKBOX_Palette4=0,C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,"
            + "C_SLIDER_Brightness=100,C_SLIDER_Contrast=0,C_SLIDER_SparkleFrequency=0";
    }
    int on_index = Effect::GetEffectIndex("On");
    int shimmer_index = Effect::GetEffectIndex("Shimmer");

    for (wxXmlNode* ch=channel->GetChildren(); ch!=NULL; ch=ch->GetNext()) {
        if (ch->GetName() == "effect") {
            double starttime = ((double)wxAtoi(ch->GetAttribute("startCentisecond"))) / 100.0;
            double endtime = ((double)wxAtoi(ch->GetAttribute("endCentisecond"))) / 100.0;
            wxString intensity = ch->GetAttribute("intensity", "-1");
            wxString starti, endi;
            if (intensity == "-1") {
                starti = ch->GetAttribute("startIntensity");
                endi = ch->GetAttribute("endIntensity");
            } else {
                starti = endi = intensity;
            }
            if ("intensity" == ch->GetAttribute("type")) {
                wxString settings = "E_TEXTCTRL_Eff_On_End=" + endi +",E_TEXTCTRL_Eff_On_Start=" + starti
                    + ",T_CHECKBOX_FitToTime=1,T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                    + "T_CHOICE_LayerMethod=Effect 1,T_SLIDER_EffectLayerMix=0,T_SLIDER_Speed=10,"
                    + "T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00";
                layer->AddEffect(0, on_index, "On", settings, palette, starttime, endtime, false, false);
            } else if ("shimmer" == ch->GetAttribute("type")) {
                //FIXME - our shimmer doesn't really match  :(
                wxString settings = _("E_CHECKBOX_Shimmer_Blink_Timing=0,E_CHECKBOX_Shimmer_Use_All_Colors=0,E_SLIDER_Shimmer_Blinks_Per_Row=1,")
                    + "E_SLIDER_Shimmer_Duty_Factor=60,T_CHECKBOX_FitToTime=1,T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                    + "T_CHOICE_LayerMethod=Effect 1,T_SLIDER_EffectLayerMix=0,T_SLIDER_Speed=30,T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00";
                layer->AddEffect(0, shimmer_index, "Shimmer", settings, palette, starttime, endtime, false, false);
            }
        }
    }
}

bool xLightsFrame::ImportLMS(Element *model, wxXmlDocument &input_xml)
{
    wxArrayString channelNames;
    wxArrayString strandNames;
    wxArrayString nodeNames;
    wxArrayString ccrNames;
    ModelClass &mc = GetModelClass(model->GetName());
    nodeNames.push_back(model->GetName());
    for (int s = 0; s < model->getStrandLayerCount(); s++) {
        StrandLayer *sl = model->GetStrandLayer(s);
        wxString sname = sl->GetName();
        if ("" == sname) {
            sname = wxString::Format("Strand %d", (s + 1));
        }
        nodeNames.push_back(sname);
        strandNames.push_back(sname);
    }
    for (int s = 0; s < model->getStrandLayerCount(); s++) {
        StrandLayer *sl = model->GetStrandLayer(s);
                for (int n = 0; n < sl->GetNodeLayerCount(); n++) {
            int idx = mc.MapToNodeIndex(s,n);
            wxString name = mc.GetNodeName(idx);
            if ("" == name) {
                name = wxString::Format("Node %d", (nodeNames.size() + 2 - strandNames.size()));
            }
            nodeNames.push_back(name);
        }
    }
    for(wxXmlNode* e=input_xml.GetRoot()->GetChildren(); e!=NULL; e=e->GetNext()) {
        if (e->GetName() == "channels"){
            for (wxXmlNode* chan=e->GetChildren(); chan!=NULL; chan=chan->GetNext()) {
                if (chan->GetName() == "channel" || chan->GetName() == "rgbChannel") {
                    wxString name = chan->GetAttribute("name");
                    channelNames.push_back(name);
                    if (chan->GetName() == "rgbChannel" && name.Contains("-P")) {
                        int i = wxAtoi(name.SubString(name.Find("-P") + 2, name.size()));
                        if (i > 0
                            && (ccrNames.size() == 0 || ccrNames.back() != name.SubString(0, name.Find("-P") - 1))) {
                            ccrNames.push_back(name.SubString(0, name.Find("-P") - 1));
                        }
                    }
                }
            }
        }
    }
    channelNames.Sort();
    channelNames.Insert("", 0);
    ccrNames.Sort();
    ccrNames.Insert("", 0);
    LMSImportChannelMapDialog dlg(this);
    dlg.SetNames(&channelNames, &strandNames, &nodeNames, &ccrNames);
    
    if (dlg.ShowModal() == wxID_CANCEL) {
        return false;
    }
    bool mapByStrand = dlg.MapByStrand->GetValue();
    int cnt = 0;
    if (!mapByStrand) {
        for (int x = 0; x <= strandNames.size(); x++) {
            EffectLayer *layer = nullptr;
            if (cnt == 0) {
                layer = model->GetEffectLayer(0);
            } else {
                layer = model->GetStrandLayer(x - 1);
            }
            wxString nm = dlg.ChannelMapGrid->GetCellValue(cnt, 1);
            if (!nm.IsEmpty()) {
                wxXmlNode *channel = nullptr;
                wxXmlNode *rchannel = nullptr;
                wxXmlNode *gchannel = nullptr;
                wxXmlNode *bchannel = nullptr;
                for(wxXmlNode* e=input_xml.GetRoot()->GetChildren(); e!=NULL; e=e->GetNext()) {
                    if (e->GetName() == "channels"){
                        for (wxXmlNode* chan=e->GetChildren(); chan!=NULL; chan=chan->GetNext()) {
                            if ((chan->GetName() == "channel" || chan->GetName() == "rgbChannel")
                                && nm == chan->GetAttribute("name")) {
                                channel = chan;
                                if (chan->GetName() == "rgbChannel"
                                    && !findRGB(e, chan, rchannel, gchannel, bchannel)) {
                                    return false;
                                }
                                break;
                            }
                        }
                    }
                }
                if (channel == nullptr) {
                    return false;
                }
                if (channel->GetName() == "rgbChannel") {
                    MapRGBEffects(layer, rchannel, gchannel, bchannel);
                } else {
                    MapOnEffects(layer, channel, mc.GetChanCountPerNode());
                }
            }
            cnt++;
        }
    }
    
    
    for (int s = 0; s < model->getStrandLayerCount(); s++) {
        StrandLayer *sl = model->GetStrandLayer(s);
        wxString ccrName;
        if (mapByStrand) {
            ccrName = dlg.ChannelMapGrid->GetCellValue(s, 1);
            if (ccrName == "") {
                continue;
            }
        }
        
        for (int n = 0; n < sl->GetNodeLayerCount(); n++) {
            EffectLayer *layer = sl->GetNodeLayer(n);
            wxString nm;
            if (mapByStrand) {
                nm = ccrName + wxString::Format("-P%02d", (n + 1));
            } else {
                nm = dlg.ChannelMapGrid->GetCellValue(cnt, 1);
            }
            if (!nm.IsEmpty()) {
                wxXmlNode *channel = nullptr;
                wxXmlNode *rchannel = nullptr;
                wxXmlNode *gchannel = nullptr;
                wxXmlNode *bchannel = nullptr;
                for(wxXmlNode* e=input_xml.GetRoot()->GetChildren(); e!=NULL; e=e->GetNext()) {
                    if (e->GetName() == "channels"){
                        for (wxXmlNode* chan=e->GetChildren(); chan!=NULL; chan=chan->GetNext()) {
                            if ((chan->GetName() == "channel" || chan->GetName() == "rgbChannel")
                                && nm == chan->GetAttribute("name")) {
                                channel = chan;
                                if (chan->GetName() == "rgbChannel"
                                    && !findRGB(e, chan, rchannel, gchannel, bchannel)) {
                                    return false;
                                }
                                break;
                            }
                        }
                    }
                }
                if (channel == nullptr) {
                    return false;
                }
                if (channel->GetName() == "rgbChannel") {
                    MapRGBEffects(layer, rchannel, gchannel, bchannel);
                } else {
                    MapOnEffects(layer, channel, mc.GetChanCountPerNode());
                }
            }
            cnt++;
        }
    }
    
    return true;
}
bool xLightsFrame::ImportSuperStar(Element *model, wxXmlDocument &input_xml)
{
    double num_rows = 1.0;
    double num_columns = 1.0;
    bool reverse_rows = false;
    bool layout_defined = false;
    wxXmlNode* input_root=input_xml.GetRoot();
    int morph_index = Effect::GetEffectIndex("Morph");
    EffectLayer* layer = model->AddEffectLayer();
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
                return false;
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
                return false;
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
    return true;
}
