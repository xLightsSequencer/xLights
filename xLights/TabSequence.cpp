#include <wx/utils.h> //check keyboard state -DJ
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include "xLightsMain.h"
#include "heartbeat.h"

#include "SeqOpenDialog.h"
#include "SeqSettingsDialog.h"
#include "NewSequenceDialog.h"
#include "xLightsXmlFile.h"

void xLightsFrame::DisplayXlightsFilename(const wxString& filename)
{
    xlightsFilename=filename;
    StaticTextPreviewFileName->SetLabel(filename);
    bool EnableButtons=!filename.IsEmpty();
    bbPlayPause->Enable(EnableButtons);
    bbStop->Enable(EnableButtons);
    StatusBar1->SetStatusText(filename, 1);
}

void xLightsFrame::OnBitmapButtonOpenSeqClick(wxCommandEvent& event)
{
    OpenSequence();
}

void xLightsFrame::OnButtonNewSequenceClick(wxCommandEvent& event)
{
    NewSequence();
	EnableSequenceControls(true);
}


// load the specified .xseq binary file
void xLightsFrame::SeqLoadXlightsXSEQ(const wxString& filename)
{
    // read xlights file
    wxFileName fn(filename);
    if (fn.GetExt() == "xseq") {
        ReadXlightsFile(filename);
        fn.SetExt("fseq");
    } else {
        ReadFalconFile(filename);
    }
    DisplayXlightsFilename(fn.GetFullPath());
    SeqBaseChannel=1;
    SeqChanCtrlBasic=false;
    SeqChanCtrlColor=false;
}


void xLightsFrame::ResetEffectsXml()
{
    ModelsNode=NULL;
    EffectsNode=NULL;
    PalettesNode=NULL;
    ViewsNode=NULL;
    ModelGroupsNode=NULL;
    SettingsNode=NULL;
    PerspectivesNode = NULL;
}

wxString xLightsFrame::LoadEffectsFileNoCheck()
{
    ResetEffectsXml();
    wxFileName effectsFile;
    effectsFile.AssignDir( CurrentDir );
    effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
    wxString myString = "Hello";
    if (!effectsFile.FileExists())
    {
        // file does not exist, so create an empty xml doc
        CreateDefaultEffectsXml();
    }
    else if (!EffectsXml.Load( effectsFile.GetFullPath() ))
    {
        wxMessageBox(_("Unable to load RGB effects file"), _("Error"));
        CreateDefaultEffectsXml();
    }
    wxXmlNode* root=EffectsXml.GetRoot();
    if (root->GetName() != "xrgb")
    {
        wxMessageBox(_("Invalid RGB effects file. Press Save File button to start a new file."), _("Error"));
        CreateDefaultEffectsXml();
    }
    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "models") ModelsNode=e;
        if (e->GetName() == "effects") EffectsNode=e;
        if (e->GetName() == "palettes") PalettesNode=e;
        if (e->GetName() == "views") ViewsNode=e;
        if (e->GetName() == "modelGroups") ModelGroupsNode=e;
        if (e->GetName() == "settings") SettingsNode=e;
        if (e->GetName() == "perspectives") PerspectivesNode=e;
    }
    if (ModelsNode == 0)
    {
        ModelsNode = new wxXmlNode( wxXML_ELEMENT_NODE, "models" );
        root->AddChild( ModelsNode );
    }
    if (EffectsNode == 0)
    {
        EffectsNode = new wxXmlNode( wxXML_ELEMENT_NODE, "effects" );
        EffectsNode->AddAttribute("version", XLIGHTS_RGBEFFECTS_VERSION);
        root->AddChild( EffectsNode );
    }
    if (PalettesNode == 0)
    {
        PalettesNode = new wxXmlNode( wxXML_ELEMENT_NODE, "palettes" );
        root->AddChild( PalettesNode );
    }

    if (ViewsNode == 0)
    {
        ViewsNode = new wxXmlNode( wxXML_ELEMENT_NODE, "views" );
        root->AddChild( ViewsNode );
    }

    if (ModelGroupsNode == 0)
    {
        ModelGroupsNode = new wxXmlNode( wxXML_ELEMENT_NODE, "modelGroups" );
        root->AddChild( ModelGroupsNode );
    }

    if (PerspectivesNode == 0)
    {
        PerspectivesNode = new wxXmlNode( wxXML_ELEMENT_NODE, "perspectives" );
        root->AddChild( PerspectivesNode );
    }

    if(SettingsNode==0)
    {
        SettingsNode = new wxXmlNode( wxXML_ELEMENT_NODE, "settings" );
        root->AddChild( SettingsNode );
        SetXmlSetting("previewWidth","1280");
        SetXmlSetting("previewHeight","720");
    }
    int previewWidth=wxAtoi(GetXmlSetting("previewWidth","1280"));
    int previewHeight=wxAtoi(GetXmlSetting("previewHeight","720"));
    if (previewWidth==0 || previewHeight==0)
    {
        previewWidth = 1280;
        previewHeight = 720;
    }
    SetPreviewSize(previewWidth,previewHeight);

    mBackgroundImage = GetXmlSetting("backgroundImage","");
    mBackgroundBrightness = wxAtoi(GetXmlSetting("backgroundBrightness","100"));
    Slider_BackgroundBrightness->SetValue(mBackgroundBrightness);
    return effectsFile.GetFullPath();
}

void xLightsFrame::LoadEffectsFile()
{
    wxStopWatch sw; // start a stopwatch timer
    wxString filename=LoadEffectsFileNoCheck();
    // check version, do we need to convert?
    wxString version=EffectsNode->GetAttribute("version", "0000");
    if (version < XLIGHTS_RGBEFFECTS_VERSION)
    {
        // fix tags
        xLightsXmlFile::FixVersionDifferences(filename);

        // load converted file
        LoadEffectsFileNoCheck();
        // update version
        EffectsNode->DeleteAttribute("version");
        EffectsNode->AddAttribute("version", XLIGHTS_RGBEFFECTS_VERSION);
        // re-save
        EffectsXml.Save( filename );
    }

    UpdateModelsList();
    UpdateViewList();
    float elapsedTime = sw.Time()/1000.0; //msec => sec
    StatusBar1->SetStatusText(wxString::Format(_("'%s' loaded in %4.3f sec."), filename, elapsedTime));
}

// returns true on success
bool xLightsFrame::SaveEffectsFile()
{
    wxFileName effectsFile;
    effectsFile.AssignDir( CurrentDir );
    effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
    if (!EffectsXml.Save( effectsFile.GetFullPath() ))
    {
        wxMessageBox(_("Unable to save RGB effects file"), _("Error"));
        return false;
    }
    UnsavedChanges=false;
    return true;
}

void xLightsFrame::CreateDefaultEffectsXml()
{
    wxXmlNode* root = new wxXmlNode( wxXML_ELEMENT_NODE, "xrgb" );
    EffectsXml.SetRoot( root );
}

wxXmlNode* xLightsFrame::GetModelNode(const wxString& name)
{
    wxXmlNode* e;
    for(e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            if (name == e->GetAttribute("name")) return e;
        }
    }
    return NULL;
}

void xLightsFrame::ShowModelsDialog()
{
    ModelListDialog dialog(this);
    wxString name;
    wxXmlNode* e;
    for(e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            name=e->GetAttribute("name");
            if (!name.IsEmpty())
            {
                dialog.ListBox1->Append(name,e);
            }
        }
    }
    dialog.HtmlEasyPrint=HtmlEasyPrint;
    dialog.ShowModal();

    // append any new models to the main xml structure
    for(size_t i=0; i<dialog.ListBox1->GetCount(); i++)
    {
        e=(wxXmlNode*)dialog.ListBox1->GetClientData(i);
        if (!e->GetParent())
        {
            ModelsNode->AddChild(e);
        }
    }
    SaveEffectsFile();
    UpdateModelsList();
    EnableSequenceControls(true);
    UpdateChannelNames();
}

void xLightsFrame::SetChoicebook(wxChoicebook* cb, const wxString& PageName)
{
    for(size_t i=0; i<cb->GetPageCount(); i++)
    {
        if (cb->GetPageText(i) == PageName)
        {
            cb->SetSelection(i);
            return;
        }
    }
}

void xLightsFrame::OnBitmapButtonSaveSeqClick(wxCommandEvent& event)
{
    SaveSequence();
}

void xLightsFrame::UpdateModelsList()
{
    wxString name;
    ModelClass *model;
    ListBoxElementList->Clear();
    PreviewModels.clear();
    OtherModels.clear();
    for(wxXmlNode* e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            name=e->GetAttribute("name");
            if (!name.IsEmpty())
            {
                model=new ModelClass;
                model->SetFromXml(e);
                
                if (model->GetLastChannel() >= NetInfo.GetTotChannels()) {
                    wxMessageBox(wxString::Format("Model %s's last channel (%u) is beyond the end of the configured number of output channels (%u)",name, model->GetLastChannel(), NetInfo.GetTotChannels()));
                }
                if (ModelClass::IsMyDisplay(e))
                {
                    ListBoxElementList->Append(name,model);
                    PreviewModels.push_back(ModelClassPtr(model));
                }
                else //keep a list of non-preview models as well -DJ
                {
                    OtherModels.push_back(ModelClassPtr(model));
                }
            }
        }
    }
}

void xLightsFrame::SaveSequence()
{
    if (SeqData.NumFrames() == 0)
    {
        wxMessageBox("You must open a sequence first!", "Error");
        return;
    }
    if (xlightsFilename.IsEmpty())
    {
        int saved_text_entry_context = mTextEntryContext;
        mTextEntryContext = TEXT_ENTRY_DIALOG;
        wxString NewFilename;
        wxTextEntryDialog dialog(this,"Enter a name for the sequence:","Save As");
        dialog.SetValue(CurrentSeqXmlFile->GetName());
        bool ok = false;
        do
        {
            if (dialog.ShowModal() != wxID_OK)
            {
                return;
            }
            // validate inputs
            NewFilename=dialog.GetValue();
            NewFilename.Trim();
            ok=true;
            if (NewFilename.IsEmpty())
            {
                ok=false;
                wxMessageBox(_("File name cannot be empty"), _("ERROR"));
            }
        }
        while (!ok);
        wxFileName oName(NewFilename);
        oName.SetPath( CurrentDir );
        oName.SetExt("fseq");
        DisplayXlightsFilename(oName.GetFullPath());

        oName.SetExt("xml");
        CurrentSeqXmlFile->SetFullName(oName.GetFullName());

        mTextEntryContext = saved_text_entry_context;
    }

    EnableSequenceControls(false);
    wxStopWatch sw; // start a stopwatch timer
    StatusBar1->SetStatusText(_("Saving ")+xlightsFilename);

    CurrentSeqXmlFile->Save(mSequenceElements);
    RenderIseqData(true); // render ISEQ layers below the Nutcracker layer
    RenderGridToSeqData();
    RenderIseqData(false);  // render ISEQ layers above the Nutcracker layer
    WriteFalconPiFile(xlightsFilename);
    DisplayXlightsFilename(xlightsFilename);
    UnsavedChanges = false;
    float elapsedTime = sw.Time()/1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
    wxString displayBuff = wxString::Format(_("%s     Updated in %7.3f seconds"),xlightsFilename,elapsedTime);
    CallAfter(&xLightsFrame::SetStatusText, displayBuff);
    EnableSequenceControls(true);
}

void xLightsFrame::RenderAll()
{
    EnableSequenceControls(false);
    wxStopWatch sw; // start a stopwatch timer
    StatusBar1->SetStatusText(_("Rendering all layers"));
    RenderIseqData(true); // render ISEQ layers below the Nutcracker layer
    RenderGridToSeqData();
    RenderIseqData(false);  // render ISEQ layers above the Nutcracker layer
    float elapsedTime = sw.Time()/1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
    wxString displayBuff = wxString::Format(_("Rendered in %7.3f seconds"),elapsedTime);
    CallAfter(&xLightsFrame::SetStatusText, displayBuff);
    EnableSequenceControls(true);
}

void xLightsFrame::GetModelNames(wxArrayString& a)
{
    wxString name;
    for(wxXmlNode* e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            name=e->GetAttribute("name");
            if (!name.IsEmpty())
            {
                a.Add(name);
            }
        }
    }
}


static void enableAllChildControls(wxWindow *parent, bool enable) {
    wxWindowList &ChildList = parent->GetChildren();
    for ( wxWindowList::Node *node = ChildList.GetFirst(); node; node = node->GetNext()) {
        wxWindow *ChildWin = (wxWindow *)node->GetData();
        ChildWin->Enable(enable);
    }
}
static void enableAllToolbarControls(wxAuiToolBar *parent, bool enable) {
    enableAllChildControls((wxWindow *)parent, enable);
    for (int x = 0; x < parent->GetToolCount(); x++) {
        wxAuiToolBarItem * item = parent->FindToolByIndex(x);
        parent->EnableTool(item->GetId(), enable);
    }
    parent->Refresh();
}
static void enableAllMenubarControls(wxMenuBar *parent, bool enable) {
    for (int x = 0; x < parent->GetMenuCount(); x++) {
        wxMenu * menu = parent->GetMenu(x);
        for (int y = 0; y < menu->GetMenuItemCount(); y++) {
            wxMenuItem *item = menu->FindItemByPosition(y);
            menu->Enable(item->GetId(), enable);
        }
    }
    parent->Refresh();
}

void xLightsFrame::EnableSequenceControls(bool enable)
{
    enableAllToolbarControls(MainToolBar, enable);
    enableAllToolbarControls(PlayToolBar, enable && SeqData.NumFrames() > 0);
    enableAllToolbarControls(EffectToolBar, enable && SeqData.NumFrames() > 0);
    enableAllToolbarControls(EffectPaletteToolBar, enable && SeqData.NumFrames() > 0);
    enableAllToolbarControls(ViewToolBar, enable);
    enableAllToolbarControls(OutputToolBar, enable);

    enableAllChildControls(EffectsPanel1, enable && SeqData.NumFrames() > 0);
    enableAllChildControls(timingPanel, enable && SeqData.NumFrames() > 0);
    enableAllChildControls(perspectivePanel, enable && SeqData.NumFrames() > 0);
    enableAllChildControls(colorPanel, enable && SeqData.NumFrames() > 0);
    enableAllChildControls(effectPalettePanel, enable && SeqData.NumFrames() > 0);

    enableAllMenubarControls(MenuBar, enable);

    if (enable && SeqData.NumFrames() == 0) {
        //no file is loaded, disable save/render buttons
        EnableToolbarButton(MainToolBar,ID_AUITOOLBAR_SAVE,false);
        EnableToolbarButton(MainToolBar,ID_AUITOOLBAR_SAVEAS,false);
        EnableToolbarButton(MainToolBar,ID_AUITOOLBAR_RENDERALL,false);
        Menu_Settings_Sequence->Enable(false);
        MenuItem_File_Save_Sequence->Enable(false);
        MenuItem_File_Close_Sequence->Enable(false);
    }
}


void xLightsFrame::CutOrCopyToClipboard(bool IsCut)
{

}

void xLightsFrame::GetSeqModelNames(wxArrayString& a)
{
}
void xLightsFrame::PlayEffect()
{
}
void xLightsFrame::UpdateViewList()
{
}
wxXmlNode* xLightsFrame::CreateEffectNode(wxString& name)
{
    return NULL;
}
void xLightsFrame::PasteFromClipboard()
{
}
void xLightsFrame::ResetEffectStates(bool *ResetEffectState)
{
    ResetEffectState[0]=true;
    ResetEffectState[1]=true;
}

void xLightsFrame::ResetSequenceGrid()
{
}


#if 0

const wxString& DefaultAs(const wxString& str, const wxString& defval)
{
    return !str.IsEmpty()? str: defval;
}

void xLightsFrame::OnButton_PlayAllClick(wxCommandEvent& event)
{
    PlayRgbSequence();
}

void xLightsFrame::PlayRgbSequence()
{
    if (SeqPlayerState == PLAYING_SEQ
            || SeqPlayerState == PLAYING_SEQ_ANIM)
    {
        StopNow();
        Button_PlayRgbSeq->SetLabel("Play (F4)");
        Button_PlayRgbSeq->Enable();
        return;
    }

    if (SeqData.NumFrames() == 0)
    {
        wxMessageBox("You must open a sequence first!", "Error");
        return;
    }
    SeqPlayColumn=Grid1->GetGridCursorCol();
    if (SeqPlayColumn < XLIGHTS_SEQ_STATIC_COLUMNS)
    {
        wxMessageBox("Select a cell in a display element column before clicking Play", "Error");
        return;
    }
    NextGridRowToPlay=Grid1->GetGridCursorRow();

    wxString ModelName=Grid1->GetColLabelValue(SeqPlayColumn);
    Choice_Models->SetStringSelection(ModelName);
    int sel=Choice_Models->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        wxMessageBox(_("Can not find model: ")+ModelName, _("Error"));
        return;
    }
    wxXmlNode* ModelXml=(wxXmlNode*)Choice_Models->GetClientData(sel);
    playBuffer.InitBuffer(ModelXml);
    ClearEffectWindow();
    StatusBar1->SetStatusText(_("Playback: RGB sequence"));
    PlayCurrentXlightsFile();
    Button_PlayRgbSeq->SetLabel("Stop (F4)");
    Button_PlayRgbSeq->Enable();
    Button_PlayEffect->Disable();
    heartbeat("playback seq", true); //tell fido to start watching -DJ
}



void xLightsFrame::PlayEffect()
{
    int sel=Choice_Models->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        wxMessageBox(_("No model is selected"), _("ERROR"));
        return;
    }
    if (EffectTreeDlg != NULL)
    {
        EffectTreeDlg->Show(false);
    }
    if (EffectTreeDlg != NULL)
    {
        EffectTreeDlg->Show(false);
    }
    wxXmlNode* ModelXml=(wxXmlNode*)Choice_Models->GetClientData(sel);
    playBuffer.InitBuffer(ModelXml);
    ResetEffectStates(playResetEffectState);
    ClearEffectWindow();
//~    playBuffer.SetMixType(Choice_LayerMethod->GetStringSelection());
    StatusBar1->SetStatusText(_("Playback: effect"));
    EnableSequenceControls(false);

    lastPlayEffect = CreateEffectString();
    MapStringString SettingsMap;
    LoadSettingsMap(lastPlayEffect, SettingsMap);
    ResetEffectStates(playResetEffectState);
    UpdateBuffersForNewMap(SettingsMap, playBuffer);
    UpdateEffectDuration(true, Grid1->GetCursorRow(), playBuffer, Grid1->GetCursorColumn());
    playPeriod = playBuffer.StartingPeriod();

    ResetTimer(PLAYING_EFFECT);
    Button_PlayEffect->SetLabel(_("Pause Effect (F3)")); //toggle label -DJ
    heartbeat("playback effect", true); //tell fido to start watching -DJ
}

void xLightsFrame::OnButton_PlayEffectClick(wxCommandEvent& event)
{
    if (SeqPlayerState == PLAYING_EFFECT)
    {
        StopNow();
    }
    else
    {
        PlayEffect();
    }
}

void xLightsFrame::OnButton_PresetsClick(wxCommandEvent& event)
{
    if (EffectTreeDlg==NULL)
    {
        EffectTreeDlg = new EffectTreeDialog(this);
        EffectTreeDlg->InitItems(EffectsNode);
    }
    EffectTreeDlg->Show();
}




wxXmlNode* xLightsFrame::CreateEffectNode(wxString& name)
{
    wxXmlNode* NewXml=new wxXmlNode(wxXML_ELEMENT_NODE, "effect");
    NewXml->AddAttribute("name", name);
    NewXml->AddAttribute("settings", CreateEffectString());
    return NewXml;
}

//modifed for partially random -DJ
//void djdebug(const char* fmt, ...); //_DJ
wxString xLightsFrame::CreateEffectStringRandom()
{
    int eff1, eff2, layerOp;
    wxString s;
    s.clear();

    /*
     enum RGB_EFFECTS_e
    {
        eff_NONE,
        eff_OFF,
        eff_ON,
        eff_BARS,
        eff_BUTTERFLY,
        eff_CIRCLES,
        eff_COLORWASH,
        eff_COROFACES,
        eff_CURTAIN,
        eff_FACES,
        eff_FIRE,
        eff_FIREWORKS,
        eff_GARLANDS,
        eff_GLEDIATOR,
        eff_LIFE,
        eff_METEORS,
        eff_PIANO,
        eff_PICTURES,
        eff_PINWHEEL,
        eff_RIPPLE,
        eff_SHIMMER,
        eff_SINGLESTRAND,
        eff_SNOWFLAKES,
        eff_SNOWSTORM,
        eff_SPIRALS,
        eff_SPIROGRAPH,
        eff_STROBE,
        eff_TEXT,
        eff_TREE,
        eff_TWINKLE,
        eff_WAVE,
        eff_LASTEFFECT //Always the last entry
    };
    */
    //   Old way
    eff1 = EffectsPanel1->isRandom_()? rand() % eff_LASTEFFECT: EffectsPanel1->Choicebook1->GetSelection();

    //  new way
    if (EffectsPanel1->isRandom_()) //avoid a few types of random effects
    {
        eff1 = CreateRandomEffect(eff_LASTEFFECT);
        /* eff1 = (eff_NONE == eff1 || eff_TEXT == eff1 || eff_PICTURES == eff1 || eff_PIANO == eff1
                 || eff_FACES == eff1 || eff_COROFACES == eff1 ||eff_GLEDIATOR == eff1
                 || eff_OFF == eff1)? eff_NONE:eff1;
         if(eff1 < eff_NONE || eff1 >= eff_LASTEFFECT) eff1 = eff_NONE;
             */
    }
    else
    {
        eff1 = EffectsPanel1->Choicebook1->GetSelection();
    }
//~    if (EffectsPanel2->isRandom_()) //avoid a few types of random effects
//    {
//        eff2 = CreateRandomEffect(eff_LASTEFFECT);
//        /*
//        eff2 = (eff_NONE == eff2|| eff_TEXT == eff2 || eff_PICTURES == eff2 || eff_PIANO == eff2 // if the above eff2+1 pushes into an effect
//                || eff_FACES == eff2 || eff_COROFACES == eff2 ||eff_GLEDIATOR == eff2
//                || eff_OFF == eff2)? eff_NONE:eff2;                  // we should skip, just set effect to NONE
//        if(eff2 < eff_NONE || eff2 >= eff_LASTEFFECT) eff2 = eff_NONE;
//            */
//    }
//    else
//    {
//        eff2 = EffectsPanel2->Choicebook1->GetSelection();
//    }

//~    layerOp = isRandom(Slider_EffectLayerMix)? rand() % LASTLAYER: Choice_LayerMethod->GetSelection();
    s = EffectNames[eff1] + ","+EffectNames[eff2] + "," + EffectLayerOptions[layerOp];
#if 0 // <SCM>
    s += ",ID_CHECKBOX_LayerMorph=0";
#else
//~    s += ",ID_CHECKBOX_LayerMorph=" + wxString::Format("%d", (isRandom(CheckBox_LayerMorph)? rand() & 1: CheckBox_LayerMorph->GetValue())? 1: 0);
#endif // 1
//~    s += ",ID_SLIDER_SparkleFrequency=" + wxString::Format("%d", isRandom(Slider_SparkleFrequency)? rand() % Slider_SparkleFrequency->GetMax(): Slider_SparkleFrequency->GetValue()); // max is actually all teh way left, ie no sparkles

    //  first calculate it the old way
//~    int newbrightness = isRandom(Slider_Brightness)? rand() % Slider_Brightness->GetMax(): Slider_Brightness->GetValue();
//~    newbrightness=100; // but instead overwrite it. no matter what we are creating, lets not mess with brightness
    //  s += ",ID_SLIDER_Brightness=" + wxString::Format("%d", isRandom(Slider_Brightness)? rand() % Slider_Brightness->GetMax(): Slider_Brightness->GetValue());
//~    s += ",ID_SLIDER_Brightness=" + wxString::Format("%d", newbrightness);

//~    s += ",ID_SLIDER_Contrast=" + wxString::Format("%d", isRandom(Slider_Contrast)? 0: Slider_Contrast->GetValue()); //use 0 instead of random value?
    s += EffectsPanel1->GetRandomEffectString(eff1);
//~    s += EffectsPanel2->GetRandomEffectString(eff2);
#if 0 //partially random -DJ
    int PageIdx1=EffectsPanel1->Choicebook1->GetSelection();
    int PageIdx2=EffectsPanel2->Choicebook1->GetSelection();
    // ID_CHOICEBOOK1, ID_CHOICEBOOK2, ID_CHOICE_LayerMethod
    wxString s=EffectsPanel1->Choicebook1->GetPageText(PageIdx1)+","+EffectsPanel2->Choicebook1->GetPageText(PageIdx2);
    s+=","+Choice_LayerMethod->GetStringSelection();
    s+=",ID_SLIDER_SparkleFrequency="+wxString::Format("%d",Slider_SparkleFrequency->GetValue());
    s+=",ID_SLIDER_Brightness="+wxString::Format("%d",Slider_Brightness->GetValue());
    s+=",ID_SLIDER_Contrast="+wxString::Format("%d",Slider_Contrast->GetValue());
    s+=",ID_SLIDER_EffectLayerMix="+wxString::Format("%d",Slider_EffectLayerMix->GetValue());
    s+=EffectsPanel1->GetEffectString();
    s+=EffectsPanel2->GetEffectString();
#elif 0
#define tostr(thing)  #thing
#define EFFECT "Color Wash"
#define SPFREQ  200
#define BRIGHT  109
#define CONTRAST  69
    s= EFFECT ",None,Effect 1,ID_SLIDER_SparkleFrequency= " tostr(SPREQ) ",ID_SLIDER_Brightness=" tostr(BRIGHT) ",ID_SLIDER_Contrast=" tostr(CONTRAST) ",ID_SLIDER_EffectLayerMix=0";
    s+=EffectsPanel1->GetRandomEffectString(eff1);
    s+=EffectsPanel2->GetRandomEffectString(eff2);
#endif
    return s;

}

int xLightsFrame::CreateRandomEffect(int eff_LASTEFFECT)
{
    bool BAD_CHOICE=1;
    int eff,count=0;
    int MAX_TRIES=10;

    //    srand (time(NULL));
    while (BAD_CHOICE && count<MAX_TRIES)
    {
        count++;
        eff=rand() % eff_LASTEFFECT;
        BAD_CHOICE = (eff_TEXT == eff || eff_PICTURES == eff || eff_PIANO == eff
                      || eff_FACES == eff || eff_COROFACES == eff || eff_GLEDIATOR == eff
                      || eff_OFF == eff || eff_ON == eff);
    }
    if(count==MAX_TRIES) eff=eff_OFF; // we failed to find a good effect after MAX_TRIES attempts
    return eff;
}

wxString xLightsFrame::CreateEffectString()
{
    int PageIdx1=EffectsPanel1->Choicebook1->GetSelection();
//~    int PageIdx2=EffectsPanel2->Choicebook1->GetSelection();
    // ID_CHOICEBOOK1, ID_CHOICEBOOK2, ID_CHOICE_LayerMethod
        wxString s;
//~    wxString s=EffectsPanel1->Choicebook1->GetPageText(PageIdx1)+","+EffectsPanel2->Choicebook1->GetPageText(PageIdx2);
//~    s+=","+Choice_LayerMethod->GetStringSelection();
#if 0 // <SCM>
    s += ",ID_CHECKBOX_LayerMorph=0";
#else
//~    s+=",ID_CHECKBOX_LayerMorph=" + wxString::Format("%d", CheckBox_LayerMorph->GetValue()? 1: 0);
#endif // 1


//~    s+=",ID_SLIDER_SparkleFrequency="+wxString::Format("%d",Slider_SparkleFrequency->GetValue());
//~    s+=",ID_SLIDER_Brightness="+wxString::Format("%d",Slider_Brightness->GetValue());
//~    s+=",ID_SLIDER_Contrast="+wxString::Format("%d",Slider_Contrast->GetValue());
//~    s+=",ID_SLIDER_EffectLayerMix="+wxString::Format("%d",Slider_EffectLayerMix->GetValue());
//~    s+=EffectsPanel1->GetEffectString();
//~    s+=EffectsPanel2->GetEffectString();
    return s;
}
void xLightsFrame::GridCellChanged(int row, int col)
{
    if (row < changedRow)
    {
        changedRow = row;
    }
    if (col < changedColumn)
    {
        changedColumn = col;
    }
    UnsavedChanges = true;
}


void xLightsFrame::UpdateGrid()
{
    int r,c;
    wxString v=CreateEffectString();
    WANT_FASTER_GRID(Grid1); //defer repaints until finished updating -DJ
    if ( Grid1->IsSelection() )
    {
        // iterate over entire grid looking for selected cells
        int nRows = Grid1->GetNumberRows();
        int nCols = Grid1->GetNumberCols();
        for (r=0; r<nRows; r++)
        {
            for (c=XLIGHTS_SEQ_STATIC_COLUMNS; c<nCols; c++)
            {
                if (Grid1->IsInSelection(r,c))
                {
                    Grid1->SetCellValue(r,c,v);
                    GridCellChanged(r, c);
                }
            }
        }
        Grid1->ForceRefresh();
    }
    else
    {
        // copy to current cell
        r=Grid1->GetGridCursorRow();
        c=Grid1->GetGridCursorCol();
        if (c >=XLIGHTS_SEQ_STATIC_COLUMNS)
        {
            Grid1->SetCellValue(r,c,v);
            GridCellChanged(r, c);
        }
    }
}
void xLightsFrame::OnButton_UpdateGridClick(wxCommandEvent& event)
{
    UpdateGrid();
}
void xLightsFrame::InsertRandomEffects(wxCommandEvent& event)
{
    int r,c;
    wxString v;

    WANT_FASTER_GRID(Grid1); //defer repaints until finished updating -DJ
    if ( Grid1->IsSelection() )
    {
        // iterate over entire grid looking for selected cells
        int nRows = Grid1->GetNumberRows();
        int nCols = Grid1->GetNumberCols();
        for (r=0; r<nRows; r++)
        {
            for (c=XLIGHTS_SEQ_STATIC_COLUMNS; c<nCols; c++)
            {
                if (Grid1->IsInSelection(r,c))
                {
                    v = CreateEffectStringRandom();
                    Grid1->SetCellValue(r,c,v);
                    GridCellChanged(r, c);
                }
            }
        }
    }
    else
    {
        // copy to current cell
        r=curCell->GetRow();
        c=curCell->GetCol();
        if (c >=XLIGHTS_SEQ_STATIC_COLUMNS)
        {
            v = CreateEffectStringRandom();
            Grid1->SetCellValue(r,c,v);
            GridCellChanged(r, c);
        }
    }
}

//copy random effect into other cells on this same row -DJ
void xLightsFrame::CopyEffectAcrossRow(wxCommandEvent& event)
{
    int nCols = Grid1->GetNumberCols();
    wxString v;
    int r, c;

    WANT_FASTER_GRID(Grid1); //defer repaints until finished updating -DJ
    if ( Grid1->IsSelection() )
    {
        // iterate over entire grid looking for selected cells
        int nRows = Grid1->GetNumberRows();
        for (r = 0; r < nRows; r++)
        {
            for (c = XLIGHTS_SEQ_STATIC_COLUMNS; c < nCols; c++) //find first selected cell
                if (Grid1->IsInSelection(r,c))
                {
                    v = Grid1->GetCellValue(r, c);
                    break;
                }
            if (c < nCols) //found a selected cell
                for (c = XLIGHTS_SEQ_STATIC_COLUMNS; c < nCols; c++)
                {
                    //copy it to other cells in this row
                    Grid1->SetCellValue(r, c, v);
                    GridCellChanged(r, c);
                }
        }
    }
    else
    {
        r = curCell->GetRow();
        c = curCell->GetCol();
        if (c >= XLIGHTS_SEQ_STATIC_COLUMNS)
        {
            v = Grid1->GetCellValue(r, c); //CreateEffectStringRandom(); //get selected cell text
//wxMessageBox(wxString::Format("col# %d of %d = %s", c, nCols, v));
            for (c = XLIGHTS_SEQ_STATIC_COLUMNS; c < nCols; c++)
            {
                //copy it to other cells in this row
                Grid1->SetCellValue(r, c, v);
                GridCellChanged(r, c);
            }
        }
    }
}

//clear all effects on this row -DJ
void xLightsFrame::ClearEffectRow(wxCommandEvent& event)
{
    int nCols = Grid1->GetNumberCols();
    wxString v;
    int r, c;

    WANT_FASTER_GRID(Grid1); //defer repaints until finished updating -DJ
    if ( Grid1->IsSelection() )
    {
        // iterate over entire grid looking for selected cells
        int nRows = Grid1->GetNumberRows();
        for (r = 0; r < nRows; r++)
        {
            for (c = XLIGHTS_SEQ_STATIC_COLUMNS; c < nCols; c++) //find first selected cell
                if (Grid1->IsInSelection(r,c)) break;
            if (c < nCols)   //found a selected cell
            {
                for (c = XLIGHTS_SEQ_STATIC_COLUMNS; c < nCols; c++)
                {
                    Grid1->SetCellValue(r, c, "");
                    GridCellChanged(r, c);
                }
            }
        }
    }
    else
    {
        r = curCell->GetRow();
        for (c = XLIGHTS_SEQ_STATIC_COLUMNS; c < nCols; c++)
        {
            Grid1->SetCellValue(r, c, "");
            GridCellChanged(r, c);
        }
    }
}

void xLightsFrame::DeleteSelectedEffects(wxCommandEvent& event)
{
    int r,c;
    wxString v;
    v.Clear();

    WANT_FASTER_GRID(Grid1); //defer repaints until finished updating -DJ
    if ( Grid1->IsSelection() )
    {
        // iterate over entire grid looking for selected cells
        int nRows = Grid1->GetNumberRows();
        int nCols = Grid1->GetNumberCols();
        for (r=0; r<nRows; r++)
        {
            for (c=XLIGHTS_SEQ_STATIC_COLUMNS; c<nCols; c++)
            {
                if (Grid1->IsInSelection(r,c))
                {
                    Grid1->SetCellValue(r,c,v);
                    Grid1->SetCellTextColour(r,c,*wxBLACK);
                    GridCellChanged(r, c);
                }
            }
        }
    }
    else
    {
        // copy to current cell
        r=curCell->GetRow();
        c=curCell->GetCol();
        if (c >=XLIGHTS_SEQ_STATIC_COLUMNS)
        {
            Grid1->SetCellValue(r,c,v);
            Grid1->SetCellTextColour(r,c,*wxBLACK);
            GridCellChanged(r, c);
        }
    }
}

void xLightsFrame::ProtectSelectedEffects(wxCommandEvent& event)
{
    int r,c;
    wxString v;
    v.Clear();

    WANT_FASTER_GRID(Grid1); //defer repaints until finished updating -DJ
    if ( Grid1->IsSelection() )
    {
        // iterate over entire grid looking for selected cells
        int nRows = Grid1->GetNumberRows();
        int nCols = Grid1->GetNumberCols();
        for (r=0; r<nRows; r++)
        {
            for (c=XLIGHTS_SEQ_STATIC_COLUMNS; c<nCols; c++)
            {
                if (Grid1->IsInSelection(r,c))
                {
                    Grid1->SetCellTextColour(r,c, *wxBLUE);
                }
            }
        }
    }
    else
    {
        // copy to current cell
        r=curCell->GetRow();
        c=curCell->GetCol();
        if (c >= XLIGHTS_SEQ_STATIC_COLUMNS)
        {
            Grid1->SetCellTextColour(r,c,*wxBLUE);
        }
    }
    Grid1->ForceRefresh();
}


void xLightsFrame::UnprotectSelectedEffects(wxCommandEvent& event)
{
    int r,c;
    wxString v;
    v.Clear();

    WANT_FASTER_GRID(Grid1); //defer repaints until finished updating -DJ
    if ( Grid1->IsSelection() )
    {
        // iterate over entire grid looking for selected cells
        int nRows = Grid1->GetNumberRows();
        int nCols = Grid1->GetNumberCols();
        for (r=0; r<nRows; r++)
        {
            for (c=XLIGHTS_SEQ_STATIC_COLUMNS; c<nCols; c++)
            {
                if (Grid1->IsInSelection(r,c))
                {
                    Grid1->SetCellTextColour(r,c, *wxBLACK);
                }
            }
        }
    }
    else
    {
        // copy to current cell
        r=curCell->GetRow();
        c=curCell->GetCol();
        if (c >= XLIGHTS_SEQ_STATIC_COLUMNS)
        {
            Grid1->SetCellTextColour(r,c,*wxBLACK);
        }
    }
    Grid1->ForceRefresh();
}

void xLightsFrame::OnChoice_LayerMethodSelect(wxCommandEvent& event)
{
    MixTypeChanged=true;
}


void xLightsFrame::OnButton_ModelsClick(wxCommandEvent& event)
{
    ShowModelsDialog();
}

void xLightsFrame::UpdateViewList()
{
    //TODO: Add code to read in model list with v2 values
    wxString name;
    ModelClass *model;
    Choice_Views->Clear();
    Choice_Views->Append("All Models");
    for(wxXmlNode* e=ViewsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "view")
        {
            name=e->GetAttribute("name");
            if (!name.IsEmpty())
            {
                Choice_Views->Append(name,e);
            }
        }
    }
    Choice_Views->SetSelection(0);
    ShowAllModelsView();
}

void xLightsFrame::OnChoice_ViewsSelect(wxCommandEvent& event)
{
    UpdateView();
}

void xLightsFrame::UpdateView()
{
    if(Choice_Views->GetSelection() == 0)
    {
        ShowAllModelsView();
    }
    else
    {
        ShowModelsView();
    }
}

void xLightsFrame::ShowAllModelsView()
{
    int cols = Grid1->GetCols();
    for(int col = XLIGHTS_SEQ_STATIC_COLUMNS; col<cols; col++)
    {
        Grid1->ShowCol(col);
    }
}

void xLightsFrame::ViewHideAllModels()
{
    int cols = Grid1->GetCols();
    for(int col = XLIGHTS_SEQ_STATIC_COLUMNS; col<cols; col++)
    {
        Grid1->HideCol(col);
    }
}

void xLightsFrame::ShowModelsView()
{
    ViewHideAllModels();
    for(wxXmlNode* e=ViewsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "view")
        {
            wxString name=e->GetAttribute("name");
            if(name == Choice_Views->GetString(Choice_Views->GetSelection()))
            {
                wxString views = e->GetAttribute("models");
                wxArrayString viewArr =wxSplit(views,',');
                int cols = Grid1->GetCols();
                for(int col = XLIGHTS_SEQ_STATIC_COLUMNS; col<cols; col++)
                {
                    wxString colModel = Grid1->GetColLabelValue(col);
                    if(viewArr.Index(colModel,false,false)!=wxNOT_FOUND)
                    {
                        Grid1->ShowCol(col);
                    }
                }
                break;
            }
        }
    }
}

void xLightsFrame::UpdateBufferFadesFromCtrl(PixelBufferClass &buffer)
{
    wxString tmpStr;
    double fadeIn, fadeOut;

//~    tmpStr = EffectsPanel1->TextCtrl_Fadein->GetValue();
    tmpStr.ToDouble(&fadeIn);
//~    tmpStr = EffectsPanel1->TextCtrl_Fadeout->GetValue();
    tmpStr.ToDouble(&fadeOut);
    buffer.SetFadeTimes(0, fadeIn, fadeOut);

//~    tmpStr = EffectsPanel2->TextCtrl_Fadein->GetValue();
    tmpStr.ToDouble(&fadeIn);
//~    tmpStr = EffectsPanel2->TextCtrl_Fadeout->GetValue();
    tmpStr.ToDouble(&fadeOut);
    buffer.SetFadeTimes(1, fadeIn, fadeOut);
}



void xLightsFrame::PlayRgbEffect(int EffectPeriod, MapStringString &SettingsMap)
{
    wxString effect1=SettingsMap["E1_Effect"];
    wxString effect2=SettingsMap["E1_Effect"];
    int persist1=wxAtoi(SettingsMap["E1_CHECKBOX_OverlayBkg"]); //NOTE: no SettingsMap for this value first time thru loop
    int persist2=wxAtoi(SettingsMap["E2_CHECKBOX_OverlayBkg"]);

    if (!persist1 || "None" == effect1)
    {
        playBuffer.Clear(0); //allow effects to overlay onto other effects (useful for composite models) -DJ
    }
    if (!persist2 || "None" == effect2)
    {
        playBuffer.Clear(1); //allow effects to overlay onto other effects (useful for composite models) -DJ
    }

    bool bufferClear = false;
    RenderEffectFromMap(EffectPeriod, SettingsMap,
                        playBuffer, playResetEffectState,
                        bufferClear, false);

    playBuffer.DisplayEffectOnWindow(seqPreview,mPointSize);
    size_t chnum;
    wxByte intensity;
    if (CheckBoxLightOutput->IsChecked() && xout)
    {
        size_t NodeCnt=playBuffer.GetNodeCount();
        size_t cn=playBuffer.ChannelsPerNode();
        for(size_t n=0; n<NodeCnt; n++)
        {
            for(size_t c=0; c<cn; c++)
            {
                playBuffer.GetChanIntensity(n,c,&chnum,&intensity);
                xout->SetIntensity(chnum,intensity);
            }
        }
    }
}



void xLightsFrame::UpdateRgbPlaybackStatus(int seconds, long msec, int EffectPeriod, const wxString& seqtype)
{

    int s=seconds%60;
//    sparkle_count=0;
    msec = (EffectPeriod*50)%1000; // change frame into ms and then find the fractional part
    int minutes=seconds / 60;

    //  TextCtrlPreviewTime->SetValue(wxString::Format("%d:%02d.%03d",minutes,seconds,msec));
    StatusBar1->SetStatusText(wxString::Format("Playback: RGB "+seqtype+" sequence %d:%02d.%03ld   %d.%03ld ",minutes,s,msec,seconds,msec));
//  old way     StatusBar1->SetStatusText(wxString::Format("Playback: RGB "+seqtype+" sequence %d:%02d",m,s));
}

void xLightsFrame::TimerRgbSeq(long msec)
{
    long StartTime;
    int EffectPeriod;
    int rowcnt=Grid1->GetNumberRows();
    wxString EffectStr;
    switch (SeqPlayerState)
    {
    case PLAYING_EFFECT:
        {
            wxString v = CreateEffectString();
            MapStringString SettingsMap;
            LoadSettingsMap(v, SettingsMap);

            if (v != lastPlayEffect) {
                lastPlayEffect = v;
                ResetEffectStates(playResetEffectState);
                EffectPeriod = 0;
                UpdateBuffersForNewMap(SettingsMap, playBuffer);
                UpdateEffectDuration(true, Grid1->GetCursorRow(), playBuffer, Grid1->GetCursorColumn());
                playPeriod = playBuffer.StartingPeriod();
                playBuffer.SetFadeTimes(0,0.0,0.0);
                playBuffer.SetFadeTimes(1,0.0,0.0);
            }
            PlayRgbEffect(playPeriod, SettingsMap);
            playPeriod++;
        }
        break;
    case STARTING_SEQ_ANIM:
        ResetTimer(PLAYING_SEQ_ANIM, GetGridStartTimeMSec(NextGridRowToPlay));
        playBuffer.SetFadeTimes(0,0.0,0.0);
        playBuffer.SetFadeTimes(1,0.0,0.0);
        xLightsFrame::PlaybackMarker = "0,0"; //keep track of where we are within grid -DJ
        break;
    case PLAYING_SEQ_ANIM:
        if (xout && !xout->TxEmpty())
        {
            TxOverflowCnt++;
            TxOverflowTotal += xout->TxNonEmptyCount(); //show how much -DJ
//            break; //keep going; might catch up -DJ
        }
        EffectPeriod = msec / SeqData.FrameTime();
        if (EffectPeriod >= SeqData.NumFrames())
        {
            // sequence has finished
            if (xout) xout->alloff();
            ResetTimer(NO_SEQ);
            EnableSequenceControls(true);
            Button_PlayEffect->SetLabel("Play Effect (F3)");
            Button_PlayRgbSeq->SetLabel("Play (F4)");
        }
        else
        {
            MapStringString SettingsMap;
            if (NextGridRowToPlay < rowcnt && msec >= GetGridStartTimeMSec(NextGridRowToPlay))
            {
                // start next effect
                Grid1->GoToCell(NextGridRowToPlay,SeqPlayColumn);
                Grid1->SelectBlock(NextGridRowToPlay,SeqPlayColumn,NextGridRowToPlay,SeqPlayColumn);

                EffectStr=Grid1->GetCellValue(NextGridRowToPlay,SeqPlayColumn);
                EffectStr.Trim();
                if(!EffectStr.IsEmpty())
                {
//~                   SetEffectControls(EffectStr, Grid1->GetColLabelValue(SeqPlayColumn));
                    xLightsFrame::PlaybackMarker = wxString::Format("%d,%d", SeqPlayColumn, NextGridRowToPlay); //keep track of where we are within grid -DJ
                    LoadSettingsMap(EffectStr, SettingsMap);
                    lastPlayEffect = EffectStr;
                    ResetEffectStates(playResetEffectState);
                    UpdateBuffersForNewMap(SettingsMap, playBuffer);
                }
                UpdateEffectDuration(!EffectStr.IsEmpty(), NextGridRowToPlay, playBuffer, SeqPlayColumn);
                NextGridRowToPlay++;
            } else {
                LoadSettingsMap(lastPlayEffect, SettingsMap);
            }
            PlayRgbEffect(EffectPeriod, SettingsMap);
            UpdateRgbPlaybackStatus(EffectPeriod/20,msec,EffectPeriod,"animation");
//            if (EffectPeriod % 20 == 0) UpdateRgbPlaybackStatus(EffectPeriod/20,msec,"animation");
        }
        break;
    case STARTING_SEQ:
        StartTime=GetGridStartTimeMSec(NextGridRowToPlay);
        if(PlayerDlg->MediaCtrl->GetState() == wxMEDIASTATE_PLAYING)
        {
            ResetTimer(PLAYING_SEQ, StartTime);
            playBuffer.SetFadeTimes(0,0.0,0.0);
            playBuffer.SetFadeTimes(1,0.0,0.0);
        }
        else
        {
            PlayerDlg->MediaCtrl->Seek(StartTime);
            PlayerDlg->MediaCtrl->Play();
        }
        break;
    case PLAYING_SEQ:
        if (xout && !xout->TxEmpty())
        {
            TxOverflowCnt++;
            TxOverflowTotal += xout->TxNonEmptyCount(); //show how much -DJ
//            break; //keep going; might catch up -DJ
        }
        msec = PlayerDlg->MediaCtrl->Tell();
        EffectPeriod = msec / SeqData.FrameTime();
        if (EffectPeriod >= SeqData.NumFrames() || PlayerDlg->MediaCtrl->GetState() != wxMEDIASTATE_PLAYING)
        {
            // sequence has finished
            PlayerDlg->MediaCtrl->Stop();
            if (xout) xout->alloff();
            ResetTimer(NO_SEQ);
            EnableSequenceControls(true);
            Button_PlayEffect->SetLabel("Play Effect (F3)");
            Button_PlayRgbSeq->SetLabel("Play (F4)");
        }
        else
        {
            MapStringString SettingsMap;
            if (NextGridRowToPlay < rowcnt && msec >= GetGridStartTimeMSec(NextGridRowToPlay))
            {
                // start next effect
                Grid1->MakeCellVisible(NextGridRowToPlay,SeqPlayColumn);
                Grid1->SelectBlock(NextGridRowToPlay,SeqPlayColumn,NextGridRowToPlay,SeqPlayColumn);
                EffectStr=Grid1->GetCellValue(NextGridRowToPlay,SeqPlayColumn);
                EffectStr.Trim();
                if(!EffectStr.IsEmpty())
                {
//~                    SetEffectControls(EffectStr, Grid1->GetColLabelValue(SeqPlayColumn));

                    LoadSettingsMap(EffectStr, SettingsMap);
                    lastPlayEffect = EffectStr;
                    ResetEffectStates(playResetEffectState);
                    UpdateBuffersForNewMap(SettingsMap, playBuffer);
                }
                UpdateEffectDuration(!EffectStr.IsEmpty(), NextGridRowToPlay, playBuffer, SeqPlayColumn);
                NextGridRowToPlay++;
            } else {
                LoadSettingsMap(lastPlayEffect, SettingsMap);
            }
            PlayRgbEffect(EffectPeriod, SettingsMap);
            //TextCtrlLog->AppendText(wxString::Format("msec=%ld, period=%d\n",msec,EffectPeriod));
            UpdateRgbPlaybackStatus(EffectPeriod/20,msec,EffectPeriod,"music");
            //   if (EffectPeriod % 20 == 0) UpdateRgbPlaybackStatus(EffectPeriod/20,msec,"music");
        }
        break;
    }
}

void xLightsFrame::ResetEffectDuration(PixelBufferClass &buffer)
{
    buffer.SetTimes(0, 0, 0, 0, true);
    buffer.SetTimes(1, 0, 0, 0, true);
}
int xLightsFrame::UpdateEffectDuration(bool new_effect_starts, int startRow,
                                       PixelBufferClass &buffer, int playCol)
{
    int ii, curEffMsec, nextEffMsec, nextTimePeriodMsec;
    double val;
    int rowcnt=Grid1->GetNumberRows();
    wxString tmpStr;

    tmpStr = Grid1->GetCellValue(startRow,0);
    curEffMsec =tmpStr.ToDouble(&val )?(int)(val*1000):0;
    ii = 1;
    if (startRow+ii < rowcnt)
    {
        tmpStr = Grid1->GetCellValue(startRow+ii,0);
        nextTimePeriodMsec =tmpStr.ToDouble(&val )?(int)(val*1000):SeqData.NumFrames()*SeqData.FrameTime();
        do
        {
            tmpStr = Grid1->GetCellValue(startRow+ii, playCol);
        }
        while (tmpStr.IsEmpty() && ++ii && startRow+ii < rowcnt);
        //Really taking advantage of short circuit evluation here
        if (!tmpStr.IsEmpty())
        {
            tmpStr = Grid1->GetCellValue(startRow+ii,0);
            nextEffMsec = tmpStr.ToDouble(&val )?(int)(val*1000):SeqData.NumFrames()*SeqData.FrameTime();
        }
        else
        {
            nextEffMsec = SeqData.NumFrames()*SeqData.FrameTime();
        }
    }
    else
    {
        nextEffMsec = nextTimePeriodMsec = SeqData.NumFrames()*SeqData.FrameTime();
    }
    buffer.SetTimes(0, curEffMsec, nextEffMsec, nextTimePeriodMsec, new_effect_starts);
    buffer.SetTimes(1, curEffMsec, nextEffMsec, nextTimePeriodMsec, new_effect_starts);
    return startRow + ii;
}

void xLightsFrame::OnButton_PaletteClick(wxCommandEvent& event)
{
    PaletteMgmtDialog dialog(this);
 //~   dialog.initialize(PalettesNode,EffectsPanel1,EffectsPanel2);
    dialog.ShowModal();
    SaveEffectsFile();
}



void xLightsFrame::GetSeqModelNames(wxArrayString& a)
{
    wxString name;
    for(int i=XLIGHTS_SEQ_STATIC_COLUMNS; i < Grid1->GetCols(); i++)
    {
        name=Grid1->GetColLabelValue(i);
        a.Add(name);
    }
}



void xLightsFrame::GetGridColumnLabels(wxArrayString& a)
{
    int n=Grid1->GetNumberCols();
    for(int i=XLIGHTS_SEQ_STATIC_COLUMNS; i < n; i++)
    {
        a.Add(Grid1->GetColLabelValue(i));
    }
}

void xLightsFrame::ChooseModelsForSequence()
{
    if (SeqData.NumFrames() == 0)
    {
        wxMessageBox("You must open a sequence first!", "Error");
        return;
    }
    SeqParmsDialog dialog(this);
    wxXmlNode* e;
    wxString name;
    int idx;

    // get list of models that are already in the grid
    wxArrayString labels;
    GetGridColumnLabels(labels);

    // populate the listbox with all models
    for(e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            name=e->GetAttribute("name");
            // allow only models where MyDisplay is set?
            //if (!name.IsEmpty() && e->GetAttribute("MyDisplay","0")=="1") {
            if (!name.IsEmpty())
            {
                dialog.CheckListBox1->Append(name);
                idx=dialog.CheckListBox1->FindString(name);
                if (idx != wxNOT_FOUND && labels.Index(name,false) != wxNOT_FOUND)
                {
                    dialog.CheckListBox1->Check(idx,true);
                }
            }
        }
    }


    dialog.StaticText_Filename->SetLabel(xlightsFilename);
    if (dialog.ShowModal() != wxID_OK) return;

    // add checked models to grid

    size_t cnt = dialog.CheckListBox1->GetCount();
    WANT_FASTER_GRID(Grid1); //defer repaints until finished updating -DJ
    for (size_t i=0; i < cnt; i++)
    {
        if (dialog.CheckListBox1->IsChecked(i))
        {
            name=dialog.CheckListBox1->GetString(i);
            idx=labels.Index(name,false);
            if (idx == wxNOT_FOUND)
            {
                // newly checked item, so add it
                Grid1->AppendCols();
                int colnum=Grid1->GetNumberCols()-1;
                Grid1->SetColLabelValue(colnum,name);
                GridCellChanged(0, colnum);

                for(int j = 0; j < Grid1->GetNumberRows(); j++)
                {
                    Grid1->SetCellOverflow(j,colnum, false);
                    Grid1->SetReadOnly(j,colnum,false);
                }
            }
            else
            {
                // item already exists
                labels[idx].Clear();
            }
        }
    }

    // any non-empty entries in labels represent items that were unchecked - so delete them from grid
    for(idx=labels.GetCount()-1; idx >= 0; idx--)
    {
        if (!labels[idx].IsEmpty())
        {
            Grid1->DeleteCols(idx+XLIGHTS_SEQ_STATIC_COLUMNS);
            GridCellChanged(0, idx+XLIGHTS_SEQ_STATIC_COLUMNS - 1);
        }
    }
    EnableSequenceControls(true);
}


void xLightsFrame::OnButton_ChannelMapClick(wxCommandEvent& event)
{
    if (SeqData.NumFrames() == 0)
    {
        wxMessageBox("You must open a sequence first!", "Error");
        return;
    }
    ChannelMapDialog dialog(this);
    dialog.SpinCtrlBaseChannel->SetValue(SeqBaseChannel);
    dialog.CheckBox_EnableBasic->SetValue(SeqChanCtrlBasic);
    dialog.CheckBox_EnableColor->SetValue(SeqChanCtrlColor);
    dialog.SetNetInfo(&NetInfo);
    if (dialog.ShowModal() != wxID_OK) return;
    SeqBaseChannel=dialog.SpinCtrlBaseChannel->GetValue();
    SeqChanCtrlBasic=dialog.CheckBox_EnableBasic->GetValue();
    SeqChanCtrlColor=dialog.CheckBox_EnableColor->GetValue();
}

//void djdebug(const char* fmt, ...); //_DJ
void xLightsFrame::ProcessAudacityTimingFile(const wxString& filename)
{
    wxTextFile f;
    wxString line;

    int r;

    if (!f.Open(filename.c_str()))
    {
        //Add error dialog if open file failed
        return;
    }

    WANT_FASTER_GRID(Grid1); //defer repaints until finished updating -DJ
    for(r=0, line = f.GetFirstLine(); !f.Eof(); line = f.GetNextLine(), r++)
    {
        std::string::size_type ofs;
        if ((ofs = line.find("#")) != std::string::npos) line.erase(ofs); //remove comments
//        while (!linebuf.empty() && (linebuf.back() == '\\')) //line continuation
//        {
//            linebuf.pop_back(); //remove trailing "\"
//            std::string morebuf;
//            if (!std::getline(infile.stream, morebuf)) break;
//            linebuf += morebuf;
//        }
        while (!line.empty() && (line.Last() == ' ')) line.RemoveLast(); //trim trailing spaces
        if (line.empty())
        {
            --r;    //skip blank lines; don't add grid row
            continue;
        }

        wxStringTokenizer tkz(line, "\t");
        wxString token = tkz.GetNextToken(); //first column = start time
#if 1 //pull in lyrics or other label text -DJ
        tkz.GetNextToken(); //second column = end time; ignored by Nutcracker
        wxString label = tkz.GetNextToken(); //third column = label/text -DJ
        for (;;) //collect remaining tokens into label
        {
            wxString more = tkz.GetNextToken();
            if (more.empty()) break;
            label += " " + more;
        }
#endif // 1
        Grid1->AppendRows();
//        djdebug("line last token = '%s', t2 = %s, t3 %s", (const char*)token, (const char*)token2, (const char*)token3); //-DJ
        Grid1->SetCellValue(r, 0, token);
#if 1
        Grid1->SetCellValue(r, 1, label); //add label text -DJ
#endif // 1
    }
}


void xLightsFrame::ResetSequenceGrid()
{
    int n;
    n=Grid1->GetNumberCols();
    if (n > XLIGHTS_SEQ_STATIC_COLUMNS) Grid1->DeleteCols(XLIGHTS_SEQ_STATIC_COLUMNS, n-XLIGHTS_SEQ_STATIC_COLUMNS);
    n=Grid1->GetNumberRows();
    if (n > 0) Grid1->DeleteRows(0, n);
}


bool xLightsFrame::RenderEffectFromMap(int period, MapStringString& SettingsMap,
                                       PixelBufferClass &buffer, bool *ResetEffectState,
                                       bool &bufferClear, bool bgThread) {

    bool effectsToUpdate = RenderEffectFromMap(0, period, SettingsMap, buffer, ResetEffectState, bgThread);
    effectsToUpdate |= RenderEffectFromMap(1, period, SettingsMap,buffer, ResetEffectState, bgThread);

    if (effectsToUpdate)
    {
        bufferClear = false;
        //TextCtrlLog->AppendText(wxString::Format("  period %d\n",p));
        buffer.CalcOutput(period);
        // update SeqData with contents of buffer
        return true;
    }//if (effectsToUpdate)
    return false;
}

void xLightsFrame::OnBitmapButtonInsertRowClick(wxCommandEvent& event)
{
    InsertRow();
}

void xLightsFrame::InsertRow()
{
    if (SeqData.NumFrames() == 0)
    {
        wxMessageBox("You must open a sequence first!", "Error");
        return;
    }
    int r=Grid1->GetGridCursorRow();
    GridCellChanged(r + 1, 0);
    Grid1->InsertRows(r + 1, 1);
    // only the first 2 columns are editable; set everything else to read-only
    int n=Grid1->GetNumberCols();
    for (int c=XLIGHTS_SEQ_STATIC_COLUMNS; c < n; c++)
    {
        Grid1->SetReadOnly(r,c);
    }
    UnsavedChanges = true;
}

void xLightsFrame::OnBitmapButtonDeleteRowClick(wxCommandEvent& event)
{
    if (SeqData.NumFrames() == 0)
    {
        wxMessageBox("You must open a sequence first!", "Error");
        return;
    }
    if ( Grid1->IsSelection() )
    {
        wxGridUpdateLocker locker(Grid1);
        int c=Grid1->GetGridCursorCol();
        for (int n = Grid1->GetNumberRows() - 1; n >= 0; n--)
        {
            if ( Grid1->IsInSelection( n , c ) )
            {
                Grid1->DeleteRows( n, 1 );
                GridCellChanged(n, 0);
                UnsavedChanges = true;
            }
        }
    }
    else
    {
        int r=Grid1->GetGridCursorRow();
        Grid1->DeleteRows( r, 1 );
        GridCellChanged(r, 0);
        UnsavedChanges = true;
    }
}

void xLightsFrame::OnButtonDisplayElementsClick(wxCommandEvent& event)
{
    if (SeqData.NumFrames() == 0)
    {
        wxMessageBox("You must open a sequence first!", "Error");
        return;
    }
    ChooseModelsForSequence();
}

//copy row2 to row1
void xLightsFrame::CopyRow(int row1, int row2)
{
    int i, iMax;
    iMax = Grid1->GetNumberCols();
    WANT_FASTER_GRID(Grid1); //defer repaints until finished updating -DJ
    for(i=0; i<iMax; i++)
    {
        Grid1->SetCellValue(row1,i,Grid1->GetCellValue(row2,i));
    }
    GridCellChanged(row1, 0);
    GridCellChanged(row2, 0);
}

// returns time in milliseconds
long xLightsFrame::GetGridStartTimeMSec(int row)
{
    double d=GetGridStartTime(row);
    return long(d*1000.0);
}

// returns time in seconds
double xLightsFrame::GetGridStartTime(int row)
{
    double d;
    wxString s = Grid1->GetCellValue(row,0);
    if (!s.IsEmpty() && s.ToDouble(&d)) return d;
    return 999999.9;
}

// see http://en.wikipedia.org/wiki/Insertion_sort
void xLightsFrame::NumericSort()
{
    //sort the table
    int i, iHole;
    double d;

    Grid1->BeginBatch();
    int rowcnt = Grid1->GetNumberRows();
    Grid1->AppendRows(1); // temp swap row

    for (i=1; i < rowcnt; i++)
    {
        d=GetGridStartTime(i);
        CopyRow(rowcnt,i);
        iHole = i;
        while ((iHole > 0) && (GetGridStartTime(iHole-1) > d))
        {
            CopyRow(iHole,iHole-1);
            iHole--;
        }
        CopyRow(iHole,rowcnt);
    }
    Grid1->DeleteRows(rowcnt,1);
    Grid1->EndBatch();
    Grid1->ForceRefresh();
    GridCellChanged(0, 0);
}

void xLightsFrame::OnGrid1CellChange(wxGridEvent& event)
{
    int col = event.GetCol();
    if (col==0)
    {
        // re-order table by start time (column 0)
        Grid1->EnableEditing(false);
        //StatusBar1->SetStatusText(_("New grid value=")+Grid1->GetCellValue(row,col));
        NumericSort();
        Grid1->EnableEditing(true);
    }
    UnsavedChanges = true;
    event.Skip();
}

void xLightsFrame::OnGrid1CellLeftClick(wxGridEvent& event)
{
    int row = event.GetRow(),
        col = event.GetCol();

    if ( row != effGridPrevY || col != effGridPrevX)
    {
        //set selected cell background
        Grid1->SetCellBackgroundColour( *wxYELLOW, row, col);
        Grid1->SetCellBackgroundColour(Grid1->GetDefaultCellBackgroundColour(), effGridPrevY, effGridPrevX);
    }
    effGridPrevY = row;
    effGridPrevX = col;

    if (col >= XLIGHTS_SEQ_STATIC_COLUMNS)
    {
        wxString EffectString=Grid1->GetCellValue(row,col);
        if (!EffectString.IsEmpty())
        {
            //Choice_Presets->SetSelection(0);  // set to <grid>
//~            SetEffectControls(EffectString, Grid1->GetColLabelValue(col));
        }
    }
    Grid1->ForceRefresh();
    event.Skip();
}

void xLightsFrame::ClearEffectWindow()
{
    //wxClientDC dc(ScrolledWindow1); //dp
    //dc.Clear();
}

void xLightsFrame::OnButtonSeqExportClick(wxCommandEvent& event)
{
    if (SeqData.NumFrames() == 0)
    {
        wxMessageBox("You must open a sequence first!", "Error");
        return;
    }
    if (Grid1->GetNumberCols() <= XLIGHTS_SEQ_STATIC_COLUMNS)
    {
        wxMessageBox("No models in the grid!", "Error");
        return;
    }
    if (Grid1->GetNumberRows() == 0)
    {
        wxMessageBox("No grid rows to save!", "Error");
        return;
    }
    int DlgResult;
    bool ok;
    wxString filename;
    SeqExportDialog dialog(this);
    do
    {
        ok=true;
        DlgResult=dialog.ShowModal();
        if (DlgResult == wxID_OK)
        {
            // validate inputs
            filename=dialog.TextCtrlFilename->GetValue();
            filename.Trim();
            if (filename.IsEmpty())
            {
                ok=false;
                wxMessageBox(_("The file name cannot be empty"), _("ERROR"));
            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
    if (DlgResult != wxID_OK) return;

    RenderGridToSeqData();


    wxString format=dialog.ChoiceFormat->GetStringSelection();
    wxStopWatch sw;
    wxString Out3=format.Left(3);
    StatusBar1->SetStatusText(_("Starting Export for ") + format + "-" + Out3);
    wxYield();

    if (Out3 == "LSP")
    {
        filename = filename + "_USER";
    }
    wxFileName oName(filename);
    oName.SetPath( CurrentDir );
    wxString fullpath;


    // REFACTOR -- FR: These extensions should all be based on Macros. For that matter all these compares should be as well.
    if (Out3 == "LOR")
    {
        if (mediaFilename.IsEmpty())
        {
            oName.SetExt(_("las"));
        }
        else
        {
            oName.SetExt(_("lms"));
        }
        fullpath=oName.GetFullPath();
        WriteLorFile(fullpath);
    }
    else if (Out3 == "Lcb")
    {
        oName.SetExt(_("lcb"));
        fullpath=oName.GetFullPath();
        WriteLcbFile(fullpath);
    }
    else if (Out3 == "Vix")
    {
        oName.SetExt(_("vix"));
        fullpath=oName.GetFullPath();
        WriteVixenFile(fullpath);
    }
    else if (Out3 == "Vir")
    {
        oName.SetExt(_("vir"));
        fullpath=oName.GetFullPath();
        WriteVirFile(fullpath);
    }
    else if (Out3 == "LSP")
    {
        oName.SetExt(_("xml"));
        fullpath=oName.GetFullPath();
        WriteLSPFile(fullpath);
        return;
    }
    else if (Out3 == "HLS")
    {
        oName.SetExt(_("hlsnc"));
        fullpath=oName.GetFullPath();
        WriteHLSFile(fullpath);
    }
    else if (Out3 == "xLi")
    {
        oName.SetExt(_(XLIGHTS_SEQUENCE_EXT));
        fullpath=oName.GetFullPath();
        WriteXLightsFile(fullpath);
    }
    else if (Out3 == "Fal")
    {
        oName.SetExt(_("fseq"));
        fullpath=oName.GetFullPath();
        WriteFalconPiFile(fullpath);
    }

    StatusBar1->SetStatusText(_("Finished writing: " )+fullpath + wxString::Format(" in %ld ms ",sw.Time()));
}

void xLightsFrame::OnbtEditViewsClick(wxCommandEvent& event)
{
    int DlgResult;
    ViewsDialog dialog(this);
    dialog.SetModelAndViewNodes(ModelsNode,ViewsNode);
    dialog.CenterOnParent();
    dialog.ShowModal();
    SaveEffectsFile();
    UpdateViewList();
}


wxXmlNode* xLightsFrame::SelectModelToExport()
{
    ExportModelSelect dialog(this);
    wxString modelName;
    int colcnt = Grid1->GetNumberCols();
    wxASSERT(colcnt > XLIGHTS_SEQ_STATIC_COLUMNS);
    for( int col=XLIGHTS_SEQ_STATIC_COLUMNS; col<colcnt; col++)
    {
        dialog.ModelChoice->Append(Grid1->GetColLabelValue(col));
    }
    dialog.ModelChoice->SetSelection(0);
    if (dialog.ShowModal() != wxID_OK) return NULL;
    return GetModelNode(dialog.ModelChoice->GetStringSelection());
}


void xLightsFrame::OnButtonModelExportClick(wxCommandEvent& event)
{
    if (SeqData.NumFrames() == 0)
    {
        wxMessageBox("You must open a sequence first!", "Error");
        return;
    }
    if (Grid1->GetNumberCols() <= XLIGHTS_SEQ_STATIC_COLUMNS)
    {
        wxMessageBox("No models in the grid!", "Error");
        return;
    }
    if (Grid1->GetNumberRows() == 0)
    {
        wxMessageBox("No grid rows to save!", "Error");
        return;
    }
    int DlgResult;
    bool ok;
    wxString filename;
    wxXmlNode* modelNode;
    SeqDataType* dataBuf;
    int numChan;
    SeqExportDialog dialog(this);
    dialog.ModelExportTypes();
    do
    {
        ok=true;
        DlgResult=dialog.ShowModal();
        if (DlgResult == wxID_OK)
        {
            // validate inputs
            filename=dialog.TextCtrlFilename->GetValue();
            filename.Trim();
            if (filename.IsEmpty())
            {
                ok=false;
                wxMessageBox(_("The file name cannot be empty"), _("ERROR"));
            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
    if (DlgResult != wxID_OK) return;

    modelNode = SelectModelToExport();

    if(!modelNode) return;

    PixelBufferClass *buffer = new PixelBufferClass();
    buffer->InitBuffer(modelNode,true);
    numChan = buffer->GetChanCount();
    int cpn = buffer->GetChanCountPerNode();
    dataBuf = RenderModelToData(modelNode, *buffer);
    delete buffer;

    wxString format=dialog.ChoiceFormat->GetStringSelection();
    wxStopWatch sw;
    wxString Out3=format.Left(3);

    if (Out3 == "LSP")
    {
        filename = filename + "_USER";
    }
    wxFileName oName(filename);
    oName.SetPath( CurrentDir );
    wxString fullpath;

    StatusBar1->SetStatusText(_("Starting Export for ") + format + "-" + Out3);
    wxYield();

    if (Out3 == "Lcb")
    {
        oName.SetExt(_("lcb"));
        fullpath=oName.GetFullPath();
        WriteLcbFile(fullpath, numChan, SeqData.NumFrames(), dataBuf);
    }
    else if (Out3 == "Vir")
    {
        oName.SetExt(_("vir"));
        fullpath=oName.GetFullPath();
        WriteVirFile(fullpath, numChan, SeqData.NumFrames(), dataBuf);
    }
    else if (Out3 == "LSP")
    {
        oName.SetExt(_("xml"));
        fullpath=oName.GetFullPath();
        //    int cpn = ChannelsPerNode();
        WriteLSPFile(fullpath, numChan, SeqData.NumFrames(), dataBuf, cpn);
        return;
    }
    else if (Out3 == "HLS")
    {
        oName.SetExt(_("hlsnc"));
        fullpath=oName.GetFullPath();
        WriteHLSFile(fullpath, numChan, SeqData.NumFrames(), dataBuf);
    }
    else if (Out3 == "Fal")
    {
        wxString tempstr;
        long stChan;
        tempstr=modelNode->GetAttribute("StartChannel","1");
        tempstr.ToLong(&stChan);
        oName.SetExt(_("eseq"));
        fullpath=oName.GetFullPath();
        WriteFalconPiModelFile(fullpath, numChan, SeqData.NumFrames(), dataBuf, stChan, numChan);
    }

    delete dataBuf;
    StatusBar1->SetStatusText(_("Finished writing model: " )+fullpath + wxString::Format(" in %ld ms ",sw.Time()));
}

void xLightsFrame::OnGrid1CellRightClick(wxGridEvent& event)
{
    wxMenu mnu;
    curCell->Set(event.GetRow(), event.GetCol());

    mnu.Append(ID_PROTECT_EFFECT, 	"Protect Effect");
    mnu.Append(ID_UNPROTECT_EFFECT, "Unprotect Effect");
    mnu.AppendSeparator();
    mnu.Append(ID_RANDOM_EFFECT, 	"Create Random Effect");
    mnu.Append(ID_IGNORE_CLICK, 	"Ignore Click");
    mnu.AppendSeparator();
    mnu.Append(ID_DELETE_EFFECT, 	"Delete Highlighted Effect");
    mnu.Append(ID_COPYROW_EFFECT, 	"Copy Effect Across Row"); //requested by Sean -DJ
    mnu.Append(ID_CLEARROW_EFFECT, 	"Clear Row"); //requested by Sean -DJ
    mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnPopupClick, NULL, this);
    PopupMenu(&mnu);
}


static const long ID_SHIFT_COL_LEFT = wxNewId();
static const long ID_SHIFT_COL_RIGHT = wxNewId();

void xLightsFrame::OnGrid1LabelRightClick(wxGridEvent& event)
{
    if (event.GetCol() > 1)
    {
        wxMenu mnu;
        curCell->Set(event.GetRow(), event.GetCol());
        if (event.GetCol() > 2)
        {
            mnu.Append(ID_SHIFT_COL_LEFT, "Shift Left");
        }
        if (event.GetCol() != Grid1->GetNumberCols() - 1)
        {
            mnu.Append(ID_SHIFT_COL_RIGHT, "Shift Right");
        }
        Grid1->SelectCol(event.GetCol());;

        mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnPopupClick, NULL, this);
        PopupMenu(&mnu);
    }
}

void xLightsFrame::OnPopupClick(wxCommandEvent &event)
{
    if(event.GetId() == ID_DELETE_EFFECT)
    {
        DeleteSelectedEffects(event);
    }
    if(event.GetId() == ID_RANDOM_EFFECT)
    {
        InsertRandomEffects(event);
    }
    if(event.GetId() == ID_PROTECT_EFFECT)
    {
        ProtectSelectedEffects(event);
    }
    if(event.GetId() == ID_UNPROTECT_EFFECT)
    {
        UnprotectSelectedEffects(event);
    }
    if (event.GetId() == ID_COPYROW_EFFECT)
        CopyEffectAcrossRow(event); //-DJ
    if (event.GetId() == ID_CLEARROW_EFFECT)
        ClearEffectRow(event); //-DJ

    if (event.GetId() == ID_SHIFT_COL_LEFT)
    {
        SwapCols(curCell->GetCol() - 1, curCell->GetCol());
        Grid1->SelectCol(curCell->GetCol() - 1);
    }
    if (event.GetId() == ID_SHIFT_COL_RIGHT)
    {
        SwapCols(curCell->GetCol(), curCell->GetCol() + 1);
        Grid1->SelectCol(curCell->GetCol() + 1);
    }
}

//void djdebug(const char* fmt, ...); //_DJ
void xLightsFrame::OnbtRandomEffectClick(wxCommandEvent& event)
{
    int r,c;
    wxString v;

    int nRows = Grid1->GetNumberRows();
    int nCols = Grid1->GetNumberCols();
    bool apply_horiz = wxGetKeyState(WXK_SHIFT); //use Shift state to apply horizontally -DJ

#if 0 //debug
    wxString buf;
    for (auto it = buttonState.begin(); it != buttonState.end(); ++it)
        buf += it->first + wxString::Format("=%d,", it->second);
    for (auto it = EffectsPanel1->buttonState.begin(); it != EffectsPanel1->buttonState.end(); ++it)
        buf += "FX1:" + it->first + wxString::Format("=%d,", it->second);
    for (auto it = EffectsPanel2->buttonState.begin(); it != EffectsPanel2->buttonState.end(); ++it)
        buf += "FX2:" + it->first + wxString::Format("=%d,", it->second);
//    djdebug("InsertRandomEffects: %s", (const char*)buf.c_str());
//    djdebug("GetRandomEffectString: %s rnd? %d", (const char*)Slider_Speed->GetName().c_str(), isRandom(Slider_Speed));
#endif

//    if (apply_horiz) wxMessageBox(_("Apply horiz."), _("DEBUG")); //-DJ
//    for (c=XLIGHTS_SEQ_STATIC_COLUMNS; c<nCols; c++)
    WANT_FASTER_GRID(Grid1); //defer repaints until finished updating -DJ
    for (r=0; r<nRows; r++) //reversed order of loops -DJ
    {
        v.Empty(); //no random effect for this row yet -DJ
//        for (r=0; r<nRows; r++)
        for (c=XLIGHTS_SEQ_STATIC_COLUMNS; c<nCols; c++)
        {
            if( Grid1->GetCellTextColour(r,c) == *wxBLACK )
            {
                if (!apply_horiz || v.empty()) v = CreateEffectStringRandom();
                Grid1->SetCellValue(r,c,v);
                GridCellChanged(r,c);
            }
        }
    }
    UnsavedChanges = true;
}

void xLightsFrame::OnSlider_EffectLayerMixCmdScroll(wxScrollEvent& event)
{
//~    txtCtlEffectMix->SetValue(wxString::Format( "%d",Slider_EffectLayerMix->GetValue()));
}

void xLightsFrame::OnSlider_SparkleFrequencyCmdScroll(wxScrollEvent& event)
{
//~    txtCtrlSparkleFreq->SetValue(wxString::Format("%d",Slider_SparkleFrequency->GetValue()));
}



void xLightsFrame::OnSlider_BrightnessCmdScroll(wxScrollEvent& event)
{
//~    txtCtlBrightness->SetValue(wxString::Format("%d",Slider_Brightness->GetValue()));
}

void xLightsFrame::OnSlider_ContrastCmdScroll(wxScrollEvent& event)
{
//~    txtCtlContrast->SetValue(wxString::Format("%d",Slider_Contrast->GetValue()));
}

void xLightsFrame::OnScrolledWindow1Resize(wxSizeEvent& event)
{
    //ScrolledWindow1->ClearBackground();
}

// pass true for cutting, false for copying
void xLightsFrame::CutOrCopyToClipboard(bool IsCut)
{
    int i,k;
    wxString copy_data;
    bool something_in_this_line;

    WANT_FASTER_GRID(Grid1); //defer repaints until finished updating -DJ
    if (Grid1->IsSelection())
    {
        // some cells are selected
        for (i=0; i< Grid1->GetRows(); i++)        // step through all lines
        {
            something_in_this_line = false;             // nothing found yet
            for (k=0; k<Grid1->GetCols(); k++)     // step through all colums
            {
                if (Grid1->IsInSelection(i,k))     // this field is selected!!!
                {
                    if (!something_in_this_line)        // first field in this line => may need a linefeed
                    {
                        if (!copy_data.IsEmpty())       // ... if it is not the very first field
                        {
                            copy_data += "\n";     // next LINE
                        }
                        something_in_this_line = true;
                    }
                    else                                    // if not the first field in this line we need a field seperator (TAB)
                    {
                        copy_data += "\t";  // next COLUMN
                    }
                    copy_data += Grid1->GetCellValue(i,k);    // finally we need the field value
                    if (IsCut)
                    {
                        Grid1->SetCellValue(i,k,wxEmptyString);
                        GridCellChanged(i,k);
                        UnsavedChanges=true;
                    }
                }
            }
        }
    }
    else
    {
        // no cells selected, so copy current cell
        i = Grid1->GetGridCursorRow();
        k = Grid1->GetGridCursorCol();
        copy_data = Grid1->GetCellValue(i,k);
        if (IsCut)
        {
            Grid1->SetCellValue(i,k,wxEmptyString);
            GridCellChanged(i,k);
            UnsavedChanges=true;
        }
    }

    if (wxTheClipboard->Open())
    {
        if (!wxTheClipboard->SetData(new wxTextDataObject(copy_data)))
        {
            wxMessageBox(_("Unable to copy data to clipboard."), _("Error"));
        }
        wxTheClipboard->Close();
    }
    else
    {
        wxMessageBox(_("Error opening clipboard."), _("Error"));
    }
}

void xLightsFrame::OnBitmapButtonGridCutClick(wxCommandEvent& event)
{
    CutOrCopyToClipboard(true);
}

void xLightsFrame::OnBitmapButtonGridCopyClick(wxCommandEvent& event)
{
    CutOrCopyToClipboard(false);
}

// validate that s contains a valid effect
// just a placeholder for now
bool xLightsFrame::IsValidEffectString(wxString& s)
{
    return true;
}

void xLightsFrame::OnBitmapButtonGridPasteClick(wxCommandEvent& event)
{
    PasteFromClipboard();
}

void xLightsFrame::PasteFromClipboard()
{
    wxString copy_data;
    wxString cur_line;
    wxArrayString fields;
    int i,k,fieldnum;

    if (wxTheClipboard->Open())
    {
        if (wxTheClipboard->IsSupported(wxDF_TEXT))
        {
            wxTextDataObject data;

            if (wxTheClipboard->GetData(data))
            {
                copy_data = data.GetText();
            }
            else
            {
                wxMessageBox(_("Unable to copy data from clipboard."), _("Error"));
            }
        }
        else
        {
            wxMessageBox(_("Non-Text data in clipboard."), _("Error"));
        }
        wxTheClipboard->Close();
    }
    else
    {
        wxMessageBox(_("Error opening clipboard."), _("Error"));
        return;
    }

    i = Grid1->GetGridCursorRow();
    k = Grid1->GetGridCursorCol();
    int numrows=Grid1->GetNumberRows();
    int numcols=Grid1->GetNumberCols();
    bool errflag=false;
#ifdef __WXOSX__
    copy_data.Replace("\r", "\n");
#endif
    WANT_FASTER_GRID(Grid1); //defer repaints until finished updating -DJ
    do
    {
        cur_line = copy_data.BeforeFirst('\n');
        copy_data = copy_data.AfterFirst('\n');
        fields=wxSplit(cur_line,'\t');
        for(fieldnum=0; fieldnum<fields.Count(); fieldnum++)
        {
            if (i < numrows && k+fieldnum < numcols /*&& k+fieldnum >=XLIGHTS_SEQ_STATIC_COLUMNS*/)
            {
                if (fields[fieldnum].IsEmpty() || IsValidEffectString(fields[fieldnum]))
                {
                    Grid1->SetCellValue(i,k+fieldnum,fields[fieldnum]);
                    GridCellChanged(i,k+fieldnum);
                    UnsavedChanges=true;
                }
                else
                {
                    errflag=true;
                }
            }
        }
        i++;
    }
    while (copy_data.IsEmpty() == false);
    if (errflag)
    {
        wxMessageBox(_("One or more of the values were not pasted because they did not contain a number"),_("Paste Error"));
    }
}




#endif
