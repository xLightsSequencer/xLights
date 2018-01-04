#include <map>
#include <wx/utils.h> //check keyboard state -DJ
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include <wx/filepicker.h>
#include <wx/fontpicker.h>
#include "../xLightsMain.h"
#include "SequenceElements.h"
#include "../TopEffectsPanel.h"
#include "../EffectIconPanel.h"
#include "Element.h"
#include "Effect.h"
#include "../BufferPanel.h"
#include "../SeqSettingsDialog.h"
#include "../ViewsModelsPanel.h"
#include "../effects/RenderableEffect.h"
#include "../xlCustomControl.h"
#include "../NoteImportDialog.h"
#include "../MIDI/MidiFile.h"
#include "../MusicXML.h"
#include "../osxMacUtils.h"
#include "../SeqElementMismatchDialog.h"
#include "../RenderCommandEvent.h"
#include "../xLightsVersion.h"
#include <wx/config.h>
#include "HousePreviewPanel.h"
#include "UtilFunctions.h"

/************************************* New Sequencer Code*****************************************/
void xLightsFrame::CreateSequencer()
{
    // Lots of logging here as this function hard crashes
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    EffectsPanel1 = nullptr;
    timingPanel = nullptr;

    logger_base.debug("CreateSequencer: Creating Panels.");

    logger_base.debug("        Sequencer grid.");
    mainSequencer = new MainSequencer(PanelSequencer, _smallWaveform);

    logger_base.debug("                Set render data sources.");
    mainSequencer->PanelEffectGrid->SetRenderDataSources(this, &SeqData);
    mainSequencer->SetSequenceElements(&mSequenceElements);

    logger_base.debug("                Set timeline.");
    mainSequencer->PanelWaveForm->SetTimeline(mainSequencer->PanelTimeLine);
    mSequenceElements.SetTimeLine(mainSequencer->PanelTimeLine);

    logger_base.debug("                Set sequence elements.");
    mainSequencer->PanelRowHeadings->SetSequenceElements(&mSequenceElements);
    mSequenceElements.SetMaxRowsDisplayed(mainSequencer->PanelRowHeadings->GetMaxRows());

    logger_base.debug("                Set dock size constraints.");
    m_mgr->SetDockSizeConstraint(0.25, 0.15);

    logger_base.debug("        Model preview.");
    _modelPreviewPanel = new ModelPreview(PanelSequencer);
    m_mgr->AddPane(_modelPreviewPanel,wxAuiPaneInfo().Name(wxT("ModelPreview")).Caption(wxT("Model Preview")).
                   Left().Layer(1).PaneBorder(true).BestSize(250,250));

    logger_base.debug("        House preview.");
    _housePreviewPanel = new HousePreviewPanel(PanelSequencer, this, _playControlsOnPreview, PreviewModels, LayoutGroups, false, 0, true);
    m_mgr->AddPane(_housePreviewPanel, wxAuiPaneInfo().Name(wxT("HousePreview")).Caption(wxT("House Preview")).
        Left().Layer(1).BestSize(250, 250));

    logger_base.debug("        Effects.");
    effectsPnl = new TopEffectsPanel(PanelSequencer);
    effectsPnl->BitmapButtonSelectedEffect->SetEffect(effectManager[0], mIconSize);

    logger_base.debug("        Effect settings.");
    EffectsPanel1 = new EffectsPanel(effectsPnl, &effectManager);
    EffectsPanel1->SetSequenceElements(&mSequenceElements);
    effectsPnl->EffectSizer->Add(EffectsPanel1, wxEXPAND);
    effectsPnl->MainSizer->Fit(effectsPnl);
    effectsPnl->MainSizer->SetSizeHints(effectsPnl);

    logger_base.debug("        Effect assist.");
    sEffectAssist = new EffectAssist(PanelSequencer);
    m_mgr->AddPane(sEffectAssist,wxAuiPaneInfo().Name(wxT("EffectAssist")).Caption(wxT("Effect Assist")).
                   Left().Layer(1).BestSize(250,250));
    sEffectAssist->Layout();

    logger_base.debug("        Color.");
    colorPanel = new ColorPanel(PanelSequencer);

    logger_base.debug("        Timing.");
    timingPanel = new TimingPanel(PanelSequencer);

    logger_base.debug("        Buffer.");
    bufferPanel = new BufferPanel(PanelSequencer);

    logger_base.debug("        Persepctive.");
    perspectivePanel = new PerspectivesPanel(PanelSequencer);

    effectPalettePanel = new EffectIconPanel(effectManager, PanelSequencer);

    // DisplayElements Panel
    logger_base.debug("        Display Elements.");
    displayElementsPanel = new ViewsModelsPanel(this, PanelSequencer);
    displayElementsPanel->SetViewChoice(mainSequencer->ViewChoice);
    displayElementsPanel->Fit();

    logger_base.debug("CreateSequencer: Hooking up the panes.");
    m_mgr->AddPane(displayElementsPanel,wxAuiPaneInfo().Name(wxT("DisplayElements")).Caption(wxT("Display Elements"))
                   .Float());
    // Hide the panel on start.
    wxAuiPaneInfo & info = m_mgr->GetPane("DisplayElements");
    info.BestSize(displayElementsPanel->GetMinSize());
    info.Hide();

    m_mgr->AddPane(perspectivePanel,wxAuiPaneInfo().Name(wxT("Perspectives")).Caption(wxT("Perspectives")).Left().Layer(1).Hide());
    m_mgr->AddPane(effectsPnl,wxAuiPaneInfo().Name(wxT("Effect")).Caption(wxT("Effect Settings")).
                   Left().Layer(1));

    m_mgr->AddPane(effectPalettePanel,wxAuiPaneInfo().Name(wxT("EffectDropper")).Caption(wxT("Effects")).Top().Layer(0));
    m_mgr->AddPane(colorPanel,wxAuiPaneInfo().Name(wxT("Color")).Caption(wxT("Color")).Top().Layer(0));
    m_mgr->AddPane(timingPanel,wxAuiPaneInfo().Name(wxT("LayerTiming")).Caption(wxT("Layer Blending")).Top().Layer(0));
    m_mgr->AddPane(bufferPanel,wxAuiPaneInfo().Name(wxT("LayerSettings")).Caption(wxT("Layer Settings")).Top().Layer(0));

    m_mgr->AddPane(mainSequencer,wxAuiPaneInfo().Name(_T("Main Sequencer")).CenterPane().Caption(_("Main Sequencer")));

    logger_base.debug("CreateSequencer: Updating the layout.");
    m_mgr->Update(); // <== KW: I have seen crashes on this line -107374819
    logger_base.debug("CreateSequencer: Resizing everything.");
    mainSequencer->Layout();
    logger_base.debug("CreateSequencer: Done.");

    mainSequencer->SetupTouchBar(effectManager, colorPanel->SetupTouchBar(mainSequencer->touchBarSupport));
}

void xLightsFrame::ResetWindowsToDefaultPositions(wxCommandEvent& event)
{
    m_mgr->GetPane("ModelPreview").Caption("Model Preview").Dock().Left().Layer(1).Show();
    m_mgr->GetPane("HousePreview").Caption("House Preview").Dock().Left().Layer(1).Show();
    m_mgr->GetPane("EffectAssist").Caption("Effect Assist").Dock().Left().Layer(1).Show();

    m_mgr->GetPane("DisplayElements").Caption("Display Elements").Float().Hide();
    m_mgr->GetPane("Perspectives").Caption("Perspectives").Dock().Left().Layer(1).Hide();
    m_mgr->GetPane("Effect").Caption("Effect").Dock().Left().Layer(1).Show();

    m_mgr->GetPane("EffectDropper").Caption("Effects").Dock().Top().Layer(0).Show();
    m_mgr->GetPane("Color").Caption("Color").Top().Dock().Layer(0).Show();
    m_mgr->GetPane("LayerTiming").Caption("Layer Blending").Dock().Top().Layer(0).Show();
    m_mgr->GetPane("LayerSettings").Caption("Layer Settings").Dock().Top().Layer(0).Show();
    m_mgr->Update();

    // reset preview pane positions
    for (auto it = LayoutGroups.begin(); it != LayoutGroups.end(); ++it) {
        LayoutGroup* grp = (LayoutGroup*)(*it);
        if (grp != nullptr) {
            grp->ResetPositions();
        }
    }

    wxConfigBase* config = wxConfigBase::Get();
    config->DeleteEntry("ToolbarLocations");
    config->DeleteEntry("xLightsMachinePerspective");
    SaveWindowPosition("xLightsSubModelDialogPosition", nullptr);
}

void xLightsFrame::InitSequencer()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // check if sequence data is the right size
    if (CurrentSeqXmlFile != nullptr && CurrentSeqXmlFile->GetSequenceLoaded())
    {
        if (SeqData.NumChannels() != roundTo4(GetMaxNumChannels()))
        {
            logger_base.info("Number of channels has changed ... reallocating sequence data memory.");

            AbortRender();

            wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
            int ms = wxAtoi(mss);

            SeqData.init(GetMaxNumChannels(), CurrentSeqXmlFile->GetSequenceDurationMS() / ms, ms);
            mSequenceElements.IncrementChangeCount(nullptr);

            SetStatusTextColor("Render buffer recreated. A render all is required.", *wxRED);
        }
    }

    if(EffectsPanel1 == nullptr || timingPanel == nullptr)
    {
        return;
    }

    if(mSequencerInitialize)
    {
        return;
    }

    if(mCurrentPerpective!=nullptr)
    {
        DoLoadPerspective(mCurrentPerpective);
    }

    // if we have a saved perspective on this machine then make that the current one
    if (_autoSavePerspecive)
    {
        wxConfigBase* config = wxConfigBase::Get();
        wxString machinePerspective = config->Read("xLightsMachinePerspective", "");
        if (machinePerspective != "")
        {
            m_mgr->LoadPerspective(machinePerspective);
            logger_base.debug("Loaded machine perspective.");
        }
    }

    mSequencerInitialize = true;
    _housePreviewPanel->GetModelPreview()->InitializePreview(mBackgroundImage,mBackgroundBrightness);
    _housePreviewPanel->GetModelPreview()->SetScaleBackgroundImage(mScaleBackgroundImage);
}

Model *xLightsFrame::GetModel(const std::string& name) const
{
    return AllModels[name];
}

bool xLightsFrame::InitPixelBuffer(const std::string &modelName, PixelBufferClass &buffer, int layerCount, bool zeroBased) {
    Model *model = GetModel(modelName);
    if (model == nullptr || model->GetModelXml() == nullptr) {
        return false;
    }
    buffer.InitBuffer(*model, layerCount, SeqData.FrameTime(), zeroBased);
    return true;
}

void xLightsFrame::CheckForAndCreateDefaultPerpective()
{
    wxXmlNode* prospectives = PerspectivesNode->GetChildren();
    mCurrentPerpective = nullptr;
    if (prospectives==nullptr)
    {
        if(PerspectivesNode->HasAttribute("current"))
        {
            PerspectivesNode->DeleteAttribute("current");
        }
        PerspectivesNode->AddAttribute("current", "Default Perspective");
        wxXmlNode* p=new wxXmlNode(wxXML_ELEMENT_NODE, "perspective");
        p->AddAttribute("name", "Default Perspective");
        p->AddAttribute("settings", m_mgr->SavePerspective());
        p->AddAttribute("version", "2.0");
        PerspectivesNode->AddChild(p);
        mCurrentPerpective = p;
        UnsavedRgbEffectsChanges=true;
    }
    else
    {
        wxString currentName = PerspectivesNode->GetAttribute("current");
        for(wxXmlNode* p=PerspectivesNode->GetChildren(); p!=nullptr; p=p->GetNext())
        {
            if (p->GetName() == "perspective")
            {
                wxString name=p->GetAttribute("name");
                if (!name.IsEmpty() && currentName == name)
                {
                    mCurrentPerpective = p;
                }
            }
        }
    }
}

static wxArrayString ToArrayString(const std::vector<std::string> &names) {
    wxArrayString ret;
    for (auto it = names.begin(); it != names.end(); ++it) {
        ret.push_back(*it);
    }
    return ret;
}

static void Remove(std::vector<std::string> &names, const std::string &str) {
    names.erase(std::remove(names.begin(), names.end(), str), names.end());
}

static void HandleChoices(xLightsFrame *frame,
                          std::vector<std::string> &AllNames,
                          std::vector<std::string> &ModelNames,
                          Element *element,
                          const std::string &msg,
                          std::vector<Element*> &toMap,
                          std::vector<Element*> &ignore)
{
    wxArrayString choices;
    choices.push_back("Rename the model in the sequence");
    choices.push_back("Delete the model in the sequence");
    choices.push_back("Map the effects to different models");
    choices.push_back("Ignore (Handle Later) - Effects will not render");

    bool ok = false;
    wxSingleChoiceDialog dlg(frame, msg,
                             "Warning", choices);
    while (!ok) {
        ok = true;
        if (dlg.ShowModal() == wxID_OK) {
            switch (dlg.GetSelection()) {
                case 0: {
                    wxSingleChoiceDialog namedlg(frame, "Choose the model to use instead:",
                                                 "Select Model", ToArrayString(ModelNames));
                    if (namedlg.ShowModal() == wxID_OK) {
                        std::string newName = namedlg.GetStringSelection().ToStdString();
                        element->SetName(newName);
                        Remove(AllNames, newName);
                        Remove(ModelNames, newName);
                    } else {
                        ok = false;
                    }
                }
                    break;
                case 1:
                    if (dynamic_cast<SubModelElement*>(element) != nullptr) {
                        SubModelElement *sme = dynamic_cast<SubModelElement*>(element) ;
                        sme->GetModelElement()->RemoveSubModel(sme->GetName());
                    } else {
                        frame->GetSequenceElements().DeleteElement(element->GetName());
                    }
                    break;
                case 2:
                    if (dynamic_cast<SubModelElement*>(element) != nullptr) {
                        SubModelElement *sme = dynamic_cast<SubModelElement*>(element) ;
                        toMap.push_back(sme->GetModelElement());
                    } else {
                        toMap.push_back(element);
                    }
                    //relo
                    break;
                case 3:
                    ignore.push_back(element);
                    break;
            }
        }
    }
}

static bool HasEffects(ModelElement *el) {
    if (el->HasEffects()) {
        return true;
    }
    for (size_t sm = 0; sm < el->GetSubModelCount(); sm++) {
        SubModelElement *sme = el->GetSubModel(sm);

        if (sme->HasEffects()) {
            return true;
        }
        StrandElement *ste = dynamic_cast<StrandElement *>(sme);
        if (ste != nullptr) {
            for (size_t n = 0; n < ste->GetNodeLayerCount(); n++) {
                NodeLayer *nl = ste->GetNodeLayer(n, true);
                if (nl->GetEffectCount() > 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

void xLightsFrame::CheckForValidModels()
{
    //bool cancelled = cancelled_in;
    bool cancelled = false;

    std::vector<std::string> AllNames;
    std::vector<std::string> ModelNames;
    for (auto it = AllModels.begin(); it != AllModels.end(); ++it) {
        AllNames.push_back(it->first);
        if (it->second->GetDisplayAs() != "ModelGroup") {
            ModelNames.push_back(it->first);
        }
    }

    for (int x = mSequenceElements.GetElementCount()-1; x >= 0; x--) {
        if (ELEMENT_TYPE_MODEL == mSequenceElements.GetElement(x)->GetType()) {
            std::string name = mSequenceElements.GetElement(x)->GetModelName();
            //remove the current models from the list so we don't end up with the same model represented twice
            Remove(AllNames, name);
            Remove(ModelNames, name);
        }
    }

    std::vector<Element*> mapLater;
    SeqElementMismatchDialog dialog(this);
    for (int x = mSequenceElements.GetElementCount()-1; x >= 0; x--) {
        if (ELEMENT_TYPE_MODEL == mSequenceElements.GetElement(x)->GetType()) {
            ModelElement *me = static_cast<ModelElement*>(mSequenceElements.GetElement(x));
            std::string name = me->GetModelName();
            Model *m = AllModels[name];
            if (m == nullptr) {
                dialog.StaticTextMessage->SetLabel("Model '"+name+"'\ndoes not exist in your list of models");
                dialog.ChoiceModels->Set(ToArrayString(AllNames));
                dialog.Fit();

                if (!cancelled && HasEffects(me))
                {
                    cancelled = (dialog.ShowModal() == wxID_CANCEL);
                }

                if (cancelled || !HasEffects(me) || dialog.RadioButtonDelete->GetValue()) {
                    mSequenceElements.DeleteElement(name);
                } else if (dialog.RadioButtonMap->GetValue()) {
                    mapLater.push_back(me);
                } else {
                    std::string newName = dialog.ChoiceModels->GetStringSelection().ToStdString();
                    mSequenceElements.GetElement(x)->SetName(newName);
                    Remove(AllNames, newName);
                    Remove(ModelNames, newName);
                }
            }
        }
    }
    std::vector<Element*> toMap;
    std::vector<Element*> ignore;
    do {
        if (!toMap.empty()) {
            ImportXLights(mSequenceElements, toMap, wxFileName(), true, true);
        }
        toMap.clear();
        for (auto a = mapLater.begin(); a != mapLater.end(); ++a) {
            toMap.push_back(*a);
        }
        mapLater.clear();
        for (int x = mSequenceElements.GetElementCount()-1; x >= 0; x--) {
            if (ELEMENT_TYPE_MODEL == mSequenceElements.GetElement(x)->GetType()) {
                std::string name = mSequenceElements.GetElement(x)->GetModelName();
                ModelElement * el = dynamic_cast<ModelElement*>(mSequenceElements.GetElement(x));
                Model *m = AllModels[name];
                if (m == nullptr) {
                    if (HasEffects(el)) {
                        HandleChoices(this, AllNames, ModelNames, el,
                                      "Model " + name + " does not exist in your layout.\n"
                                      + "How should we handle this?",
                                      toMap, ignore);
                    } else {
                        mSequenceElements.DeleteElement(name);
                    }
                } else if (m->GetDisplayAs() == "ModelGroup") {
                    bool hasStrandEffects = false;
                    bool hasNodeEffects = false;
                    for (int l = 0; l < el->GetStrandCount(); l++) {
                        StrandElement *sl = el->GetStrand(l);
                        for (int l2 = 0; l2 < sl->GetEffectLayerCount(); l2++) {
                            if (sl->GetEffectLayer(l2)->GetEffectCount() > 0) {
                                hasStrandEffects = true;
                            }
                        }
                        for (int n = 0; n < sl->GetNodeLayerCount(); n++) {
                            if (sl->GetNodeLayer(n)->GetEffectCount()) {
                                hasNodeEffects = true;
                            }
                        }
                    }
                    for (auto a = ignore.begin(); a != ignore.end(); ++a) {
                        if (el == *a) {
                            hasNodeEffects = false;
                            hasStrandEffects = false;
                        }
                    }
                    if (hasNodeEffects || hasStrandEffects) {
                        HandleChoices(this, AllNames, ModelNames, el,
                                      "Model " + name + " is a Model Group but has Node/Strand effects.\n"
                                      + "How should we handle this?",
                                      toMap, ignore);
                    }
                } else {
                    for (int x1 = 0; x1 < el->GetSubModelCount(); x1++) {
                        SubModelElement *sme = el->GetSubModel(x1);
                        if (dynamic_cast<StrandElement*>(sme) == nullptr
                            && m->GetSubModel(sme->GetName()) == nullptr) {
                            std::vector<std::string> AllSMNames;
                            std::vector<std::string> ModelSMNames;
                            for (int z = 0; z < m->GetNumSubModels(); z++) {
                                AllSMNames.push_back(m->GetSubModel(z)->GetName());
                                ModelSMNames.push_back(m->GetSubModel(z)->GetName());
                            }
                            for (int z = 0; z < el->GetSubModelCount(); z++) {
                                Remove(AllSMNames, el->GetSubModel(z)->GetName());
                                Remove(ModelSMNames, el->GetSubModel(z)->GetName());
                            }
                            HandleChoices(this, AllSMNames, ModelSMNames, el,
                                          "SubModel " + sme->GetName() + " of Model " + m->GetName() + " does not exist.\n"
                                          + "How should we handle this?",
                                          toMap, ignore);
                        }
                    }
                }
            }
        }
    } while (!toMap.empty());
}

void xLightsFrame::LoadAudioData(xLightsXmlFile& xml_file)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // abort any in progress render ... as it may be using any already open media
    if (xml_file.GetMedia() != nullptr)
    {
        AbortRender();
    }

    mMediaLengthMS = xml_file.GetSequenceDurationMS();

    if(xml_file.GetSequenceType()=="Media")
    {
        int musicLength = 0;
		mediaFilename = wxEmptyString;
		if (xml_file.GetMedia() == nullptr)
		{
			mediaFilename = xml_file.GetMediaFile();
            ObtainAccessToURL(mediaFilename.ToStdString());
			if ((mediaFilename == wxEmptyString) || !wxFileExists(mediaFilename) || !wxIsReadable(mediaFilename))
			{
				SeqSettingsDialog setting_dlg(this, &xml_file, mediaDirectory, wxT(""));
				setting_dlg.Fit();
				int ret_val = setting_dlg.ShowModal();

                mediaFilename = xml_file.GetMediaFile();

                if (xml_file.GetMedia() != nullptr)
                {
                    mediaFilename = xml_file.GetMedia()->FileName();
                    ObtainAccessToURL(mediaFilename.ToStdString());
                    if (xml_file.GetMedia() != nullptr && xml_file.GetMedia()->GetFrameInterval() < 0)
                    {
                        xml_file.GetMedia()->SetFrameInterval(xml_file.GetFrameMS());
                    }
                    SetAudioControls();
                }

                if (ret_val == NEEDS_RENDER)
                {
                    RenderAll();
                }
            }
        } else {
            mediaFilename = xml_file.GetMediaFile();
            ObtainAccessToURL(mediaFilename.ToStdString());
        }

        if( mediaFilename != wxEmptyString )
        {
			wxString error;
            musicLength = mainSequencer->PanelWaveForm->OpenfileMedia(xml_file.GetMedia(), error);
            if (musicLength <= 0)
            {
                logger_base.warn("Media File Missing or Corrupted %s. Details: %s", (const char*) mediaFilename.c_str(), (const char *)error.c_str());
                wxMessageBox(wxString::Format("Media File Missing or Corrupted %s.\n\nDetails: %s", mediaFilename, error));
            }
        }
        else if (xml_file.GetSequenceType() == "Media")
        {
            logger_base.warn("Media File must be specified");
            wxMessageBox("Media File must be specified");
        }

        if (mMediaLengthMS == 0) {
            mMediaLengthMS = musicLength;
        }
    }

    mainSequencer->PanelTimeLine->SetTimeLength(mMediaLengthMS);
    mainSequencer->PanelTimeLine->Initialize();
    int maxZoom = mainSequencer->PanelTimeLine->GetMaxZoomLevel();
    mainSequencer->PanelTimeLine->SetZoomLevel(maxZoom);
    mainSequencer->PanelWaveForm->SetZoomLevel(maxZoom);
    mainSequencer->PanelTimeLine->RaiseChangeTimeline();  // force refresh when new media is loaded
    mainSequencer->PanelWaveForm->UpdatePlayMarker();
}

void xLightsFrame::LoadSequencer(xLightsXmlFile& xml_file)
{
    SetFrequency(xml_file.GetFrequency());
    mSequenceElements.SetViewsManager(GetViewsManager()); // This must come first before LoadSequencerFile.
    mSequenceElements.SetModelsNode(ModelsNode);
    mSequenceElements.SetEffectsNode(EffectsNode);
    mSequenceElements.LoadSequencerFile(xml_file, GetShowDirectory());
    xml_file.AdjustEffectSettingsForVersion(mSequenceElements, this);


    Menu_Settings_Sequence->Enable(true);
    MenuSettings->Enable(ID_MENUITEM_RENDER_MODE, true);
    if( xml_file.GetRenderMode() == xLightsXmlFile::CANVAS_MODE )
    {
        MenuItemRenderEraseMode->Check(false);
        MenuItemRenderCanvasMode->Check(true);
    }
    else
    {
        MenuItemRenderEraseMode->Check(true);
        MenuItemRenderCanvasMode->Check(false);
    }

    mSavedChangeCount = mSequenceElements.GetChangeCount();
    mLastAutosaveCount = mSavedChangeCount;
    CheckForValidModels();

    LoadAudioData(xml_file);


    mSequenceElements.PrepareViews(xml_file);
    mSequenceElements.PopulateRowInformation();

    mainSequencer->PanelEffectGrid->SetSequenceElements(&mSequenceElements);
    mainSequencer->PanelEffectGrid->SetTimeline(mainSequencer->PanelTimeLine);
    mainSequencer->PanelTimeLine->SetSequenceEnd(CurrentSeqXmlFile->GetSequenceDurationMS());
    _housePreviewPanel->SetDurationFrames(CurrentSeqXmlFile->GetSequenceDurationMS() / CurrentSeqXmlFile->GetFrameMS());
    mSequenceElements.SetSequenceEnd(CurrentSeqXmlFile->GetSequenceDurationMS());
    ResizeAndMakeEffectsScroll();
    ResizeMainSequencer();
    mainSequencer->PanelEffectGrid->Refresh();
    _modelPreviewPanel->Refresh();
    _housePreviewPanel->Refresh();
    m_mgr->Update();
}

void xLightsFrame::Zoom( wxCommandEvent& event)
{
    if(event.GetInt() == ZOOM_IN)
    {
        mainSequencer->PanelTimeLine->ZoomIn();
    }
    else
    {
        mainSequencer->PanelTimeLine->ZoomOut();
    }
}

void xLightsFrame::Scroll( wxCommandEvent& event)
{
    int position = mainSequencer->ScrollBarEffectsHorizontal->GetThumbPosition();
    int ts = mainSequencer->ScrollBarEffectsHorizontal->GetThumbSize() / 10;
    if (ts ==0) {
        ts = 1;
    }
    if(event.GetInt() == SCROLL_LEFT)
    {
        if (position > 0) {
            position -= ts;
        }
    }
    else
    {
        position += ts;
        if (position >= mainSequencer->ScrollBarEffectsHorizontal->GetRange()) {
            position = mainSequencer->ScrollBarEffectsHorizontal->GetRange() - 1;
        }
    }
    mainSequencer->ScrollBarEffectsHorizontal->SetThumbPosition(position);
    wxCommandEvent eventScroll(EVT_HORIZ_SCROLL);
    mainSequencer->HorizontalScrollChanged(eventScroll);
    mainSequencer->PanelEffectGrid->ClearSelection(true);
}


void xLightsFrame::TimeSelected( wxCommandEvent& event)
{
    mainSequencer->PanelTimeLine->SetSelectedPositionStartMS(event.GetInt());
    mainSequencer->PanelWaveForm->Refresh();
}

void xLightsFrame::MousePositionUpdated( wxCommandEvent& event)
{
    mainSequencer->PanelTimeLine->SetMousePositionMS(event.GetInt());
    mainSequencer->PanelTimeLine->Update();
    mainSequencer->PanelWaveForm->Refresh();
    mainSequencer->PanelWaveForm->Update();
}

void xLightsFrame::RowHeadingsChanged( wxCommandEvent& event)
{
    wxString s = event.GetString();
    if ("" != s) {
        for(wxXmlNode* e=ModelGroupsNode->GetChildren(); e!=nullptr; e=e->GetNext() ) {
            if (e->GetName() == "modelGroup") {
                if (s == e->GetAttribute("name")) {
                    std::string modelString = e->GetAttribute("models").ToStdString();
                    mSequenceElements.AddMissingModelsToSequence(modelString, false);
                }
            }
        }
    }
    mSequenceElements.PopulateRowInformation();
    displayElementsPanel->Initialize();
    ResizeMainSequencer();
}

void xLightsFrame::WindowResized( wxCommandEvent& event)
{
    ResizeAndMakeEffectsScroll();
    ResizeMainSequencer();
}

void xLightsFrame::ResizeAndMakeEffectsScroll()
{
    wxSize s;

    int w = effectsPnl->GetSize().GetWidth();
    int h = effectsPnl->GetSize().GetHeight();
    if(w>10 && h>50)
    {
        s = wxSize(w-10,h-60);
    }
    else
    {
        s = wxSize(200,200);
    }

    EffectsPanel1->SetSize(s);
    EffectsPanel1->SetMinSize(s);
    EffectsPanel1->SetMaxSize(s);


    EffectsPanel1->EffectChoicebook->SetSize(s);
    EffectsPanel1->EffectChoicebook->SetMinSize(s);
    EffectsPanel1->EffectChoicebook->SetMaxSize(s);

    EffectsPanel1->Refresh();
    EffectsPanel1->EffectChoicebook->Refresh();

    // Make effects window scroll by updating its container size
    wxScrolledWindow* sw = (wxScrolledWindow*)EffectsPanel1->EffectChoicebook->GetPage(EffectsPanel1->EffectChoicebook->GetSelection());
    sw->FitInside();
    sw->SetScrollRate(5, 5);
    sw->Refresh();

}

void xLightsFrame::ResizeMainSequencer()
{

    // Set max rows to determine correct row information size
    mSequenceElements.SetMaxRowsDisplayed(mainSequencer->PanelRowHeadings->GetMaxRows());
    mSequenceElements.PopulateVisibleRowInformation();

    mainSequencer->PanelWaveForm->Refresh();
    mainSequencer->PanelTimeLine->Refresh();
    mainSequencer->PanelRowHeadings->Refresh();
    mainSequencer->PanelEffectGrid->Refresh();
    mainSequencer->ScrollBarEffectsHorizontal->Refresh();
    mainSequencer->ScrollBarEffectsVertical->Refresh();

    colorPanel->Refresh();
    colorPanel->ColorScrollWindow->Refresh();
    mainSequencer->UpdateEffectGridVerticalScrollBar();
}

void xLightsFrame::OnPanelSequencerPaint(wxPaintEvent& event)
{
    mainSequencer->ScrollBarEffectsHorizontal->Update();
}


void xLightsFrame::UnselectedEffect(wxCommandEvent& event) {
    UnselectEffect();
}
void xLightsFrame::UnselectEffect(){
    if (playType != PLAY_TYPE_MODEL && playType != PLAY_TYPE_MODEL_PAUSED) {
        playType = PLAY_TYPE_STOPPED;
        playStartTime = -1;
        playEndTime = -1;
        playStartMS = -1;
    }
    sEffectAssist->SetPanel(nullptr);
    selectedEffect = nullptr;
    selectedEffectName = "";
    selectedEffectString = "";
    selectedEffectPalette = "";
    _modelPreviewPanel->Refresh();
    _housePreviewPanel->Refresh();
    mainSequencer->PanelEffectGrid->UnselectEffect(false);
}

void xLightsFrame::EffectChanged(wxCommandEvent& event)
{
    Effect* effect = (Effect*)event.GetClientData();
    SetEffectControls(effect->GetParentEffectLayer()->GetParentElement()->GetModelName(),
                      effect->GetEffectName(), effect->GetSettings(), effect->GetPaletteMap(),
                      true);
    selectedEffectString = "";  // force update to effect rendering
}

void xLightsFrame::SelectedEffectChanged(SelectedEffectChangedEvent& event)
{
    bool OnlyChoiceBookPage = event.effect==nullptr?true:false;
    Effect* effect = nullptr;
    if(OnlyChoiceBookPage)
    {
        int pageIndex = event.GetInt();
        // Dont change page if it is already on correct page
        if (EffectsPanel1->EffectChoicebook->GetSelection()!=pageIndex) {
            EffectsPanel1->EffectChoicebook->SetSelection(pageIndex);

            timingPanel->SetDefaultControls(nullptr, true);
            bufferPanel->SetDefaultControls(nullptr, true);
            colorPanel->SetDefaultSettings(true);
        } else {
            event.updateUI = false;
        }
    }
    else
    {
        effect = event.effect;
		bool resetStrings = false;
        if ("Random" == effect->GetEffectName()) {
            std::string settings, palette;
            std::string effectName = CreateEffectStringRandom(settings, palette);
            effect->SetPalette(palette);
            effect->SetSettings(settings, false);
            effect->SetEffectName(effectName);
            effect->SetEffectIndex(effectManager.GetEffectIndex(effectName));
            resetStrings = true;
        }
        SetEffectControls(effect->GetParentEffectLayer()->GetParentElement()->GetModelName(),
                          effect->GetEffectName(), effect->GetSettings(), effect->GetPaletteMap(),
                          !event.isNew);
        selectedEffectString = GetEffectTextFromWindows(selectedEffectPalette);
        selectedEffect = effect;
        if (effect->GetPaletteMap().empty() || resetStrings) {
            effect->SetPalette(selectedEffectPalette);
            effect->SetSettings(selectedEffectString, true);
            RenderEffectForModel(effect->GetParentEffectLayer()->GetParentElement()->GetModelName(),
                                 effect->GetStartTimeMS(),
                                 effect->GetEndTimeMS());
        }

        if (playType == PLAY_TYPE_MODEL_PAUSED) {
            DoStopSequence();
        }

        if (playType != PLAY_TYPE_MODEL) {
            playType = PLAY_TYPE_EFFECT;
            playStartTime = effect->GetStartTimeMS();
            playEndTime = effect->GetEndTimeMS();
            playStartMS = -1;
            playModel = GetModel(effect->GetParentEffectLayer()->GetParentElement()->GetModelName());
			SetAudioControls();
        }
    }
    if (event.updateUI || event.updateBtn) {
        RenderableEffect *eff = effectManager[EffectsPanel1->EffectChoicebook->GetSelection()];
        effectsPnl->SetDragIconBuffer(eff->GetEffectIcon(16));
        effectsPnl->BitmapButtonSelectedEffect->SetEffect(eff, mIconSize);
        if( effect != nullptr ) {
            UpdateEffectAssistWindow(effect, eff);
        }
        mainSequencer->PanelEffectGrid->SetFocus();
    }
}

void xLightsFrame::SelectedRowChanged(wxCommandEvent& event)
{
    mainSequencer->PanelRowHeadings->SetSelectedRow(event.GetInt());
    playModel = GetModel(event.GetString().ToStdString());
}

void xLightsFrame::EffectDroppedOnGrid(wxCommandEvent& event)
{
    int effectIndex = EffectsPanel1->EffectChoicebook->GetSelection();
    mSequenceElements.UnSelectAllEffects();
    std::string name = EffectsPanel1->EffectChoicebook->GetPageText(effectIndex).ToStdString();
    std::string palette;
    std::string settings = GetEffectTextFromWindows(palette);
    selectedEffect = nullptr;
    Effect* last_effect_created = nullptr;

    mSequenceElements.get_undo_mgr().CreateUndoStep();
    for(int i=0;i<mSequenceElements.GetSelectedRangeCount();i++)
    {
        EffectLayer* el = mSequenceElements.GetSelectedRange(i)->Layer;
        if (el->GetParentElement()->GetType() == ELEMENT_TYPE_TIMING) {
            continue;
        }
        // Delete Effects that are in same time range as dropped effect
        el->SelectEffectsInTimeRange(mSequenceElements.GetSelectedRange(i)->StartTime,
                                     mSequenceElements.GetSelectedRange(i)->EndTime);
        el->DeleteSelectedEffects(mSequenceElements.get_undo_mgr());
        // Add dropped effect
        Effect* effect = el->AddEffect(0,name,settings,palette,
                                       mSequenceElements.GetSelectedRange(i)->StartTime,
                                       mSequenceElements.GetSelectedRange(i)->EndTime,
                                       EFFECT_SELECTED,false);

        last_effect_created = effect;

        mSequenceElements.get_undo_mgr().CaptureAddedEffect( el->GetParentElement()->GetModelName(), el->GetIndex(), effect->GetID() );

        mainSequencer->PanelEffectGrid->ProcessDroppedEffect(effect);

        if (playType == PLAY_TYPE_MODEL_PAUSED) {
            DoStopSequence();
			SetAudioControls();
		}

        if (playType != PLAY_TYPE_MODEL) {
            playType = PLAY_TYPE_EFFECT;
            playStartTime = mSequenceElements.GetSelectedRange(i)->StartTime;
            playEndTime = mSequenceElements.GetSelectedRange(i)->EndTime;
            playStartMS = -1;
            RenderEffectForModel(el->GetParentElement()->GetModelName(),playStartTime,playEndTime);

            playModel = GetModel(el->GetParentElement()->GetModelName());

			SetAudioControls();
        }
    }

    if (playType != PLAY_TYPE_MODEL && last_effect_created != nullptr)
    {
        SetEffectControls(last_effect_created->GetParentEffectLayer()->GetParentElement()->GetModelName(),
                          last_effect_created->GetEffectName(), last_effect_created->GetSettings(),
                          last_effect_created->GetPaletteMap(), false);
        selectedEffectString = GetEffectTextFromWindows(selectedEffectPalette);
        selectedEffect = last_effect_created;
    }

	RenderableEffect *eff = effectManager[EffectsPanel1->EffectChoicebook->GetSelection()];

	if( last_effect_created != nullptr ) {
        UpdateEffectAssistWindow(last_effect_created, eff);
	}

    mainSequencer->PanelEffectGrid->Refresh(false);
}

void xLightsFrame::PlayModel(wxCommandEvent& event)
{
    std::string model = event.GetString().ToStdString();
    playModel = GetModel(model);
    if (playModel != nullptr
        && playType != PLAY_TYPE_MODEL) {
        wxCommandEvent playEvent(EVT_PLAY_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::CopyModelEffects(wxCommandEvent& event)
{
    mainSequencer->PanelEffectGrid->CopyModelEffects(event.GetInt());
}

void xLightsFrame::PasteModelEffects(wxCommandEvent& event)
{
    mainSequencer->PanelEffectGrid->PasteModelEffects(event.GetInt());
}

void xLightsFrame::ModelSelected(wxCommandEvent& event)
{
    if (playType == PLAY_TYPE_MODEL)
    {
        playModel = GetModel(event.GetString().ToStdString());
    }
}

void xLightsFrame::AutoShowHouse()
{
    if (_autoShowHousePreview)
    {
        bool visible = m_mgr->GetPane("HousePreview").IsShown();
        if (playType == PLAY_TYPE_MODEL || playType == PLAY_TYPE_MODEL_PAUSED)
        {
            if (!visible)
            {
                m_mgr->GetPane("HousePreview").Show();
                m_mgr->Update();
            }
        }
        else
        {
            if (visible)
            {
                m_mgr->GetPane("HousePreview").Hide();
                m_mgr->Update();
            }
        }
    }
}

void xLightsFrame::DoPlaySequence()
{
	if (CurrentSeqXmlFile != nullptr)
	{
		if (playType == PLAY_TYPE_EFFECT_PAUSED)
		{
			playType = PLAY_TYPE_EFFECT;
			playStartMS = -1;
		}
		else
		{
			playType = PLAY_TYPE_MODEL;
			playStartMS = -1;
			playStartTime = mainSequencer->PanelTimeLine->GetNewStartTimeMS();
			playEndTime = mainSequencer->PanelTimeLine->GetNewEndTimeMS();
			if (CurrentSeqXmlFile->GetSequenceType() == "Media") {
				if (CurrentSeqXmlFile->GetMedia() != nullptr)
				{
					CurrentSeqXmlFile->GetMedia()->Seek(playStartTime);
				}
			}
			if (playEndTime == -1 || playEndTime > CurrentSeqXmlFile->GetSequenceDurationMS()) {
				playEndTime = CurrentSeqXmlFile->GetSequenceDurationMS();
			}
			mainSequencer->PanelTimeLine->PlayStarted();
			if (CurrentSeqXmlFile->GetSequenceType() == "Media") {
				if (CurrentSeqXmlFile->GetMedia() != nullptr)
				{
					CurrentSeqXmlFile->GetMedia()->Play();
				}
			}
		}
	}
	SetAudioControls();
    AutoShowHouse();
}

void xLightsFrame::PlaySequence(wxCommandEvent& event)
{
	mLoopAudio = false;
	DoPlaySequence();
}

void xLightsFrame::PauseSequence(wxCommandEvent& event)
{
    if( CurrentSeqXmlFile->GetSequenceType() == "Media" )
    {
		if (CurrentSeqXmlFile->GetMedia() != nullptr)
		{
			if (CurrentSeqXmlFile->GetMedia()->GetPlayingState() == MEDIAPLAYINGSTATE::PLAYING)
			{
				CurrentSeqXmlFile->GetMedia()->Pause();
			}
			else if (CurrentSeqXmlFile->GetMedia()->GetPlayingState() == MEDIAPLAYINGSTATE::PAUSED)
			{
				CurrentSeqXmlFile->GetMedia()->Play();
			}
		}
	}

    if (playType == PLAY_TYPE_MODEL) {
        playType = PLAY_TYPE_MODEL_PAUSED;
    }
    else if (playType == PLAY_TYPE_MODEL_PAUSED) {
        mainSequencer->PanelTimeLine->CheckNeedToScrollToPlayStart(true);
        playType = PLAY_TYPE_MODEL;
    }
    else if (playType == PLAY_TYPE_EFFECT_PAUSED) {
        playType = PLAY_TYPE_EFFECT;
    }
    else {
        playType = PLAY_TYPE_EFFECT_PAUSED;
    }
	SetAudioControls();
    AutoShowHouse();
}

void xLightsFrame::SetAudioControls()
{
	if (CurrentSeqXmlFile == nullptr || mRendering)
	{
		EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, false);
		EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, false);
		EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, false);
		EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, false);
		EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, false);
		EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, false);
        _housePreviewPanel->EnablePlayControls("Play", false);
        _housePreviewPanel->EnablePlayControls("Pause", false);
        _housePreviewPanel->EnablePlayControls("Stop", false);
        _housePreviewPanel->EnablePlayControls("Rewind", false);
        _housePreviewPanel->EnablePlayControls("Rewind10", false);
        _housePreviewPanel->EnablePlayControls("FForward10", false);
        _housePreviewPanel->EnablePlayControls("Position", false);
	}
	else if (CurrentSeqXmlFile->GetSequenceType() != "Media")
	{
		if (playType == PLAY_TYPE_EFFECT_PAUSED || playType == PLAY_TYPE_MODEL_PAUSED)
		{
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, true);
            _housePreviewPanel->EnablePlayControls("Play", true);
            _housePreviewPanel->EnablePlayControls("Pause", true);
            _housePreviewPanel->EnablePlayControls("Stop", true);
            _housePreviewPanel->EnablePlayControls("Rewind", true);
            _housePreviewPanel->EnablePlayControls("Rewind10", false);
            _housePreviewPanel->EnablePlayControls("FForward10", false);
            _housePreviewPanel->EnablePlayControls("Position", true);
        }
		else if (playType == PLAY_TYPE_EFFECT || playType == PLAY_TYPE_MODEL)
		{
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, false);
            _housePreviewPanel->EnablePlayControls("Play", true);
            _housePreviewPanel->EnablePlayControls("Pause", true);
            _housePreviewPanel->EnablePlayControls("Stop", true);
            _housePreviewPanel->EnablePlayControls("Rewind", false);
            _housePreviewPanel->EnablePlayControls("Rewind10", true);
            _housePreviewPanel->EnablePlayControls("FForward10", true);
            _housePreviewPanel->EnablePlayControls("Position", true);
        }
		else
		{
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, true);
            _housePreviewPanel->EnablePlayControls("Play", true);
            _housePreviewPanel->EnablePlayControls("Pause", false);
            _housePreviewPanel->EnablePlayControls("Stop", false);
            _housePreviewPanel->EnablePlayControls("Rewind", true);
            _housePreviewPanel->EnablePlayControls("Rewind10", false);
            _housePreviewPanel->EnablePlayControls("FForward10", false);
            _housePreviewPanel->EnablePlayControls("Position", true);
        }
	}
	else
	{
		if (CurrentSeqXmlFile->GetMedia() == nullptr)
		{
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, false);
            _housePreviewPanel->EnablePlayControls("Play", false);
            _housePreviewPanel->EnablePlayControls("Pause", false);
            _housePreviewPanel->EnablePlayControls("Stop", false);
            _housePreviewPanel->EnablePlayControls("Rewind", false);
            _housePreviewPanel->EnablePlayControls("Rewind10", false);
            _housePreviewPanel->EnablePlayControls("FForward10", false);
            _housePreviewPanel->EnablePlayControls("Position", false);
        }
		else if (playType == PLAY_TYPE_STOPPED)
		{
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, true);
            _housePreviewPanel->EnablePlayControls("Play", true);
            _housePreviewPanel->EnablePlayControls("Pause", false);
            _housePreviewPanel->EnablePlayControls("Stop", false);
            _housePreviewPanel->EnablePlayControls("Rewind", true);
            _housePreviewPanel->EnablePlayControls("Rewind10", false);
            _housePreviewPanel->EnablePlayControls("FForward10", false);
            _housePreviewPanel->EnablePlayControls("Position", true);
        }
		else if (playType == PLAY_TYPE_EFFECT || playType == PLAY_TYPE_MODEL)
		{
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, false);
            _housePreviewPanel->EnablePlayControls("Play", true);
            _housePreviewPanel->EnablePlayControls("Pause", true);
            _housePreviewPanel->EnablePlayControls("Stop", true);
            _housePreviewPanel->EnablePlayControls("Rewind", false);
            _housePreviewPanel->EnablePlayControls("Rewind10", true);
            _housePreviewPanel->EnablePlayControls("FForward10", true);
            _housePreviewPanel->EnablePlayControls("Position", true);
        }
		else if (playType == PLAY_TYPE_EFFECT_PAUSED || playType == PLAY_TYPE_MODEL_PAUSED)
		{
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, true);
            _housePreviewPanel->EnablePlayControls("Play", true);
            _housePreviewPanel->EnablePlayControls("Pause", true);
            _housePreviewPanel->EnablePlayControls("Stop", true);
            _housePreviewPanel->EnablePlayControls("Rewind", true);
            _housePreviewPanel->EnablePlayControls("Rewind10", false);
            _housePreviewPanel->EnablePlayControls("FForward10", false);
            _housePreviewPanel->EnablePlayControls("Position", true);
        }
		else
		{
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, true);
            _housePreviewPanel->EnablePlayControls("Play", true);
            _housePreviewPanel->EnablePlayControls("Pause", false);
            _housePreviewPanel->EnablePlayControls("Stop", false);
            _housePreviewPanel->EnablePlayControls("Rewind", true);
            _housePreviewPanel->EnablePlayControls("Rewind10", false);
            _housePreviewPanel->EnablePlayControls("FForward10", false);
            _housePreviewPanel->EnablePlayControls("Position", true);
        }
	}
}

void xLightsFrame::TogglePlay(wxCommandEvent& event)
{
    if( playType == PLAY_TYPE_MODEL )
    {
        wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
    else if( playType == PLAY_TYPE_MODEL_PAUSED )
    {
        wxCommandEvent playEvent(EVT_PAUSE_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
    else
    {
        wxCommandEvent playEvent(EVT_PLAY_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::DoStopSequence()
{
    _fps = -1;
	mLoopAudio = false;
    if( playType == PLAY_TYPE_MODEL || playType == PLAY_TYPE_MODEL_PAUSED )
    {
        if( CurrentSeqXmlFile->GetSequenceType() == "Media" ) {
			if (CurrentSeqXmlFile->GetMedia() != nullptr)
			{
				CurrentSeqXmlFile->GetMedia()->Stop();
				CurrentSeqXmlFile->GetMedia()->Seek(playStartTime);
			}
        }
        mainSequencer->PanelTimeLine->PlayStopped();
        mainSequencer->PanelWaveForm->UpdatePlayMarker();
        mainSequencer->PanelEffectGrid->ForceRefresh();
        mainSequencer->UpdateTimeDisplay(playStartTime, _fps);
    }
    playType = PLAY_TYPE_STOPPED;
    if( CheckBoxLightOutput->IsChecked()) {
        _outputManager.AllOff();
    }
	SetAudioControls();
    AutoShowHouse();
}

void xLightsFrame::StopSequence(wxCommandEvent& event)
{
    DoStopSequence();
}

void xLightsFrame::SequenceFirstFrame(wxCommandEvent& event)
{
    _fps = -1;
    if( playType == PLAY_TYPE_EFFECT_PAUSED || playType == PLAY_TYPE_EFFECT ) {
        playStartMS = -1;
    }
    else
    {
        mainSequencer->ScrollBarEffectsHorizontal->SetThumbPosition(0);
        wxCommandEvent eventScroll(EVT_HORIZ_SCROLL);
        mainSequencer->HorizontalScrollChanged(eventScroll);

        mainSequencer->PanelTimeLine->ResetMarkers(0);
        mainSequencer->PanelWaveForm->UpdatePlayMarker();
        mainSequencer->PanelEffectGrid->ForceRefresh();
        mainSequencer->UpdateTimeDisplay(0, _fps);
    }
}

void xLightsFrame::SequenceLastFrame(wxCommandEvent& event)
{
    _fps = -1;
    int limit = mainSequencer->ScrollBarEffectsHorizontal->GetRange();
    mainSequencer->ScrollBarEffectsHorizontal->SetThumbPosition(limit-1);
    wxCommandEvent eventScroll(EVT_HORIZ_SCROLL);
    mainSequencer->HorizontalScrollChanged(eventScroll);

    int end_ms = CurrentSeqXmlFile->GetSequenceDurationMS();
    mainSequencer->PanelTimeLine->ResetMarkers(end_ms);
    mainSequencer->PanelWaveForm->UpdatePlayMarker();
    mainSequencer->PanelEffectGrid->ForceRefresh();
    mainSequencer->UpdateTimeDisplay(end_ms, _fps);
}

void xLightsFrame::SequenceRewind10(wxCommandEvent& event)
{
    int current_play_time;
    if (CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->GetMedia() != nullptr)
    {
        current_play_time = CurrentSeqXmlFile->GetMedia()->Tell();
    }
    else
    {
        wxTimeSpan ts = wxDateTime::UNow() - starttime;
        long curtime = ts.GetMilliseconds().ToLong();
        int msec = 0;
        if (playAnimation) {
            msec = curtime * playSpeed;
        }
        else {
            msec = curtime;
        }

        current_play_time = (playStartTime + msec - playStartMS);
    }

    long origtime = current_play_time;
    current_play_time -= 10000;
    if (current_play_time < 0) current_play_time = 0;

    if (CurrentSeqXmlFile->GetSequenceType() == "Media") {
        if (CurrentSeqXmlFile->GetMedia() != nullptr)
        {
            CurrentSeqXmlFile->GetMedia()->Seek(current_play_time);
        }
    }
    else
    {
        starttime += wxTimeSpan(0, 0, (origtime - current_play_time) / 1000, (origtime - current_play_time) % 1000);
    }

    mainSequencer->PanelWaveForm->UpdatePlayMarker();
    mainSequencer->PanelEffectGrid->ForceRefresh();
    mainSequencer->UpdateTimeDisplay(current_play_time, _fps);
}

void xLightsFrame::SequenceFForward10(wxCommandEvent& event)
{
    int current_play_time;
    if (CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->GetMedia() != nullptr)
    {
        current_play_time = CurrentSeqXmlFile->GetMedia()->Tell();
    }
    else
    {
        wxTimeSpan ts = wxDateTime::UNow() - starttime;
        long curtime = ts.GetMilliseconds().ToLong();
        int msec = 0;
        if (playAnimation) {
            msec = curtime * playSpeed;
        }
        else {
            msec = curtime;
        }

        current_play_time = (playStartTime + msec - playStartMS);
    }

    long origtime = current_play_time;
    current_play_time += 10000;
    int end_ms = CurrentSeqXmlFile->GetSequenceDurationMS();
    if (current_play_time > end_ms) current_play_time = end_ms;

    if (CurrentSeqXmlFile->GetSequenceType() == "Media") {
        if (CurrentSeqXmlFile->GetMedia() != nullptr)
        {
            CurrentSeqXmlFile->GetMedia()->Seek(current_play_time);
        }
    }
    else
    {
        starttime += wxTimeSpan(0, 0, (origtime - current_play_time) / 1000, (origtime - current_play_time) % 1000);
    }

    mainSequencer->PanelWaveForm->UpdatePlayMarker();
    mainSequencer->PanelEffectGrid->ForceRefresh();
    mainSequencer->UpdateTimeDisplay(current_play_time, _fps);
}

void xLightsFrame::SequenceSeekTo(wxCommandEvent& event)
{
    int pos = event.GetInt();
    int current_play_time;
    if (CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->GetMedia() != nullptr)
    {
        current_play_time = CurrentSeqXmlFile->GetMedia()->Tell();
    }
    else
    {
        wxTimeSpan ts = wxDateTime::UNow() - starttime;
        long curtime = ts.GetMilliseconds().ToLong();
        int msec = 0;
        if (playAnimation) {
            msec = curtime * playSpeed;
        }
        else {
            msec = curtime;
        }

        current_play_time = (playStartTime + msec - playStartMS);
    }

    long origtime = current_play_time;
    current_play_time += (pos - current_play_time);
    int end_ms = CurrentSeqXmlFile->GetSequenceDurationMS();
    if (current_play_time > end_ms) current_play_time = end_ms;

    if (CurrentSeqXmlFile->GetSequenceType() == "Media") {
        if (CurrentSeqXmlFile->GetMedia() != nullptr)
        {
            CurrentSeqXmlFile->GetMedia()->Seek(current_play_time);
        }
    }
    else
    {
        starttime += wxTimeSpan(0, 0, (origtime - current_play_time) / 1000, (origtime - current_play_time) % 1000);
    }

    mainSequencer->PanelWaveForm->UpdatePlayMarker();
    mainSequencer->PanelEffectGrid->ForceRefresh();
    mainSequencer->UpdateTimeDisplay(current_play_time, _fps);
}

void xLightsFrame::SequenceReplaySection(wxCommandEvent& event)
{
	mLoopAudio = true;
	DoPlaySequence();
}

void xLightsFrame::PlayModelEffect(wxCommandEvent& event)
{
    if( playType != PLAY_TYPE_MODEL && playType != PLAY_TYPE_MODEL_PAUSED)
    {
        EventPlayEffectArgs* args = (EventPlayEffectArgs*)event.GetClientData();
        playModel = GetModel(args->element->GetModelName());
        if (playModel != nullptr) {
            playType = PLAY_TYPE_EFFECT;
            playStartTime = (int)(args->effect->GetStartTimeMS());
            playEndTime = (int)(args->effect->GetEndTimeMS());
            if(args->renderEffect)
            {
                RenderEffectForModel(args->element->GetModelName(),playStartTime,playEndTime);
            }
            playStartMS = -1;
        } else {
            playType = PLAY_TYPE_STOPPED;
        }
    }
}

void xLightsFrame::UpdateEffectPalette(wxCommandEvent& event) {
    std::string palette;
    std::string effectText = GetEffectTextFromWindows(palette);
    mSequenceElements.get_undo_mgr().CreateUndoStep();
    for(size_t i=0;i<mSequenceElements.GetVisibleRowInformationSize();i++) {
        Element* element = mSequenceElements.GetVisibleRowInformation(i)->element;
        EffectLayer* el = mSequenceElements.GetVisibleEffectLayer(i);

        int startms = 99999999;
        int endms = -1;
        for(int j=0;j< el->GetEffectCount();j++) {
            if(el->GetEffect(j)->GetSelected() != EFFECT_NOT_SELECTED) {
                Effect *ef = el->GetEffect(j);
                mSequenceElements.get_undo_mgr().CaptureModifiedEffect(element->GetModelName(),
                                                                       el->GetIndex(),
                                                                       ef);
                ef->SetPalette(palette);
                startms = std::min(startms, ef->GetStartTimeMS());
                endms = std::max(endms, ef->GetEndTimeMS());
            }
        }
        if(startms <= endms) {
            playType = PLAY_TYPE_EFFECT;
            playStartMS = -1;
            RenderEffectForModel(element->GetModelName(),startms,endms);
        }

    }
    mainSequencer->PanelEffectGrid->ForceRefresh();
}

void xLightsFrame::UpdateEffect(wxCommandEvent& event)
{
    std::string palette;
    std::string effectText = GetEffectTextFromWindows(palette);
    int effectIndex = EffectsPanel1->EffectChoicebook->GetSelection();
    std::string effectName = EffectsPanel1->EffectChoicebook->GetPageText(EffectsPanel1->EffectChoicebook->GetSelection()).ToStdString();

    mSequenceElements.get_undo_mgr().CreateUndoStep();

    for(size_t i=0;i<mSequenceElements.GetVisibleRowInformationSize();i++) {
        Element* element = mSequenceElements.GetVisibleRowInformation(i)->element;
        EffectLayer* el = mSequenceElements.GetVisibleEffectLayer(i);

        int startms = 99999999;
        int endms = -1;

        for(int j=0;j< el->GetEffectCount();j++) {
            if(el->GetEffect(j)->GetSelected() != EFFECT_NOT_SELECTED)  {
                Effect *ef = el->GetEffect(j);
                mSequenceElements.get_undo_mgr().CaptureModifiedEffect(element->GetModelName(),
                                                                       el->GetIndex(),
                                                                       ef);

                el->GetEffect(j)->SetSettings(effectText, true);
                el->GetEffect(j)->SetEffectIndex(effectIndex);
                el->GetEffect(j)->SetEffectName(effectName);
                el->GetEffect(j)->SetPalette(palette);

                startms = std::min(startms, el->GetEffect(j)->GetStartTimeMS());
                endms = std::max(endms, el->GetEffect(j)->GetEndTimeMS());
            }
        }

        if(startms <= endms) {
            playType = PLAY_TYPE_EFFECT;
            playStartMS = -1;
            RenderEffectForModel(element->GetModelName(),startms,endms);
        }
    }
    mainSequencer->PanelEffectGrid->ForceRefresh();
}

void xLightsFrame::RandomizeEffect(wxCommandEvent& event)
{
    mSequenceElements.get_undo_mgr().CreateUndoStep();

    for(size_t i=0;i<mSequenceElements.GetVisibleRowInformationSize();i++) {
        Element* element = mSequenceElements.GetVisibleRowInformation(i)->element;
        EffectLayer* el = mSequenceElements.GetVisibleEffectLayer(i);

        int startms = 99999999;
        int endms = -1;

        for(int j=0;j< el->GetEffectCount();j++) {
            if(el->GetEffect(j)->GetSelected() != EFFECT_NOT_SELECTED)  {
                std::string effectName = el->GetEffect(j)->GetEffectName();
                int effectIndex = el->GetEffect(j)->GetEffectIndex();

                std::string settings = EffectsPanel1->GetRandomEffectString(effectIndex).ToStdString();
                std::string palette = colorPanel->GetRandomColorString().ToStdString();

                mSequenceElements.get_undo_mgr().CaptureModifiedEffect(element->GetModelName(),
                                                                       el->GetIndex(),
                                                                       el->GetEffect(j));

                el->GetEffect(j)->SetSettings(settings, true);
                el->GetEffect(j)->SetEffectIndex(effectIndex);
                el->GetEffect(j)->SetEffectName(effectName);
                el->GetEffect(j)->SetPalette(palette);

                SetEffectControls(el->GetEffect(j)->GetParentEffectLayer()->GetParentElement()->GetModelName(),
                                  el->GetEffect(j)->GetEffectName(),
                                  el->GetEffect(j)->GetSettings(),
                                  el->GetEffect(j)->GetPaletteMap(),
                                  true);
                selectedEffectString = GetEffectTextFromWindows(selectedEffectPalette);
                selectedEffect = el->GetEffect(j);
                startms = std::min(startms, el->GetEffect(j)->GetStartTimeMS());
                endms = std::max(endms, el->GetEffect(j)->GetEndTimeMS());
            }
        }

        if(startms <= endms) {
            playType = PLAY_TYPE_EFFECT;
            playStartMS = -1;
            RenderEffectForModel(element->GetModelName(),startms,endms);
        }
    }

    mainSequencer->PanelEffectGrid->ForceRefresh();
    if( m_mgr->GetPane("EffectAssist").IsShown())
    {
        sEffectAssist->ForceRefresh();
    }
}

void xLightsFrame::OnEffectSettingsTimerTrigger(wxTimerEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (CurrentSeqXmlFile == nullptr) {
        return;
    }

    UpdateRenderStatus();
    if (Notebook1->GetSelection() != NEWSEQUENCER) {
        return;
    }

    // grab a copy of the pointer in case user clicks off the effect
    Effect* eff = selectedEffect;

    if (eff != nullptr && timingPanel->BitmapButton_CheckBox_LayerMorph->IsEnabled()) {
        std::string palette;
        std::string effectText = GetEffectTextFromWindows(palette);
        if (effectText != selectedEffectString
            || palette != selectedEffectPalette) {

            int effectIndex = EffectsPanel1->EffectChoicebook->GetSelection();
            wxString name = EffectsPanel1->EffectChoicebook->GetPageText(effectIndex);
            if (name !=  eff->GetEffectName()) {
                eff->SetEffectName(name.ToStdString());
                eff->SetEffectIndex(EffectsPanel1->EffectChoicebook->GetSelection());
            }

            EffectLayer* el = eff->GetParentEffectLayer();

            // TEMPORARY - THIS SHOULD BE REMOVED BUT I WANT TO SEE WHAT IS CAUSING SOME RANDOM CRASHES - KW - 2017.7
            if (el == nullptr)
            {
                logger_base.crit("OnEffectSettingsTimerTrigger el is nullptr ... this is going to crash.");
            }

            Element *elem = el->GetParentElement();

            // TEMPORARY - THIS SHOULD BE REMOVED BUT I WANT TO SEE WHAT IS CAUSING SOME RANDOM CRASHES - KW - 2017.7
            if (elem == nullptr)
            {
                logger_base.crit("OnEffectSettingsTimerTrigger elem is nullptr ... this is going to crash.");
            }

            //check for undo capture
            if( selectedEffectName != eff->GetEffectName() )
            {
                mSequenceElements.get_undo_mgr().CreateUndoStep();
                mSequenceElements.get_undo_mgr().CaptureModifiedEffect( elem->GetModelName(), el->GetIndex(), eff->GetID(), selectedEffectString, selectedEffectPalette );
            }

            eff->SetSettings(effectText, true);
            eff->SetPalette(palette);

            selectedEffectName = eff->GetEffectName();
            selectedEffectString = effectText;
            selectedEffectPalette = palette;

            playStartTime = eff->GetStartTimeMS();
            playEndTime = eff->GetEndTimeMS();
            playStartMS = -1;

            // Update if effect has been modified
            if( m_mgr->GetPane("EffectAssist").IsShown() )
            {
                sEffectAssist->ForceRefresh();
            }

            RenderEffectForModel(elem->GetModelName(),playStartTime,playEndTime);
            mainSequencer->PanelEffectGrid->ForceRefresh();

            // This ensures colour curves which can be dependent on effect settings are correct
            RenderableEffect *ef = GetEffectManager().GetEffect(selectedEffectName);
            if (ef != nullptr)
            {
                colorPanel->SetSupports(ef->SupportsLinearColorCurves(eff->GetSettings()), ef->SupportsRadialColorCurves(eff->GetSettings()));
            }

            return;
        }
    }
}

void xLightsFrame::TimerRgbSeq(long msec)
{
    //check if there are models that depend on timing tracks or similar that need to be rendered
    std::vector<Element *> elsToRender;
    if (mSequenceElements.GetElementsToRender(elsToRender)) {
        for (std::vector<Element *>::iterator it = elsToRender.begin(); it != elsToRender.end(); ++it) {
            int ss, es;
            (*it)->GetDirtyRange(ss, es);
            RenderEffectForModel((*it)->GetModelName(), ss, es);
        }
    }

    // Update play status so sequencer grid can allow dropping timings during playback
    mainSequencer->SetPlayStatus(playType);

    // return if play is stopped
    if (playType == PLAY_TYPE_STOPPED || CurrentSeqXmlFile == nullptr) {
        return;
    }

    // return if paused
    if (playType == PLAY_TYPE_EFFECT_PAUSED || playType == PLAY_TYPE_MODEL_PAUSED) {
        playStartMS = msec - playOffsetTime;  // maintain offset so we can restart where we paused
        return;
    }

    // return if we have reset play times
    if (playEndTime == 0) {
        return;
    }

    // capture start time if necessary
    if (playStartMS == -1) {
        playStartMS = msec;
        fpsEvents.clear();
    }

    // record current time
    int curt = (playStartTime + msec - playStartMS);
    playOffsetTime = msec - playStartMS;

    // repeat loop if in play effect mode
    if (curt >= playEndTime && playType == PLAY_TYPE_EFFECT) {
        playStartMS = msec;
        curt = playStartTime;
    }

    // reset all if current time is invalid
    if (curt < 0) {
        playStartMS = -1;
        if (playType == PLAY_TYPE_MODEL) {
            playStartTime = playEndTime = 0;
            playType = PLAY_TYPE_STOPPED;
        }
        return;
    }

    if (playType == PLAY_TYPE_MODEL) {

        int current_play_time;
		if (CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->GetMedia() != nullptr && CurrentSeqXmlFile->GetMedia()->GetPlayingState() == MEDIAPLAYINGSTATE::PLAYING)
		{
			current_play_time = CurrentSeqXmlFile->GetMedia()->Tell();
            curt = current_play_time;
        } else
		{
            current_play_time = curt;
        }
        // see if its time to stop model play
        if (curt >= playEndTime) {
			if (mLoopAudio)
			{
				DoPlaySequence();
				return;
			}
			else
			{
				playStartTime = playEndTime = 0;
				playStartMS = -1;
				wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
				wxPostEvent(this, playEvent);
				return;
			}
        }

        int frame = curt / SeqData.FrameTime();
        fpsEvents.push_back(FPSEvent(frame));
        size_t fpsSize = fpsEvents.size();
        while (fpsSize > (2000 / SeqData.FrameTime()))
        {
            fpsEvents.pop_front();
            fpsSize--;
        }
        if (!fpsEvents.empty())
        {
            FPSEvent b = fpsEvents.front();
            FPSEvent e = fpsEvents.back();
            _fps = (float)((double)(fpsSize-1) * 1000.0) / ((e.when - b.when).GetMilliseconds().ToDouble());
            if ((frame % 200) == 0) {
                static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
                logger_opengl.debug("Play fps  %f   (%d ms)", _fps, SeqData.FrameTime());
            }
        }

        //static wxLongLong ms = wxGetUTCTimeMillis();
        mainSequencer->UpdateTimeDisplay(current_play_time, _fps);
        if (mainSequencer->PanelTimeLine->SetPlayMarkerMS(current_play_time)) {
            mainSequencer->PanelWaveForm->UpdatePlayMarker();
            mainSequencer->PanelWaveForm->CheckNeedToScroll();
            mainSequencer->PanelEffectGrid->ForceRefresh();
            _housePreviewPanel->SetPositionFrames(current_play_time / CurrentSeqXmlFile->GetFrameMS());
        }

        //wxLongLong me = wxGetUTCTimeMillis();
        //printf("%d     %d    %d\n", (me-ms).GetLo(), SeqData.FrameTime(), Timer1.GetInterval());
        //ms = me;
    }

    int frame = curt / SeqData.FrameTime();
    //have the frame, copy from SeqData
    if (playModel != nullptr) {
        int nn = playModel->GetNodeCount();
        for (int node = 0; node < nn; node++) {
            int start = playModel->NodeStartChannel(node);
            playModel->SetNodeChannelValues(node, &SeqData[frame][start]);
        }
    }
    TimerOutput(frame);
    if (playModel != nullptr) {
        playModel->DisplayEffectOnWindow(_modelPreviewPanel, mPointSize);
    }
    _housePreviewPanel->GetModelPreview()->Render(&SeqData[frame][0]);
    for (auto it = PreviewWindows.begin(); it != PreviewWindows.end(); ++it) {
        ModelPreview* preview = *it;
        if( preview->GetActive() ) {
            preview->Render(&SeqData[frame][0]);
        }
    }
}

void xLightsFrame::SetEffectControls(const std::string &modelName, const std::string &effectName,
                                     const SettingsMap &settings, const SettingsMap &palette,
                                     bool setDefaults) {
    SetChoicebook(EffectsPanel1->EffectChoicebook, effectName);
    Model *model = GetModel(modelName);
    if (setDefaults) {
        if (modelName == "") {
            EffectsPanel1->SetDefaultEffectValues(nullptr, CurrentSeqXmlFile->GetMedia(), effectName);
            timingPanel->SetDefaultControls(nullptr);
            bufferPanel->SetDefaultControls(nullptr);
            colorPanel->SetDefaultSettings();
        } else {
            EffectsPanel1->SetDefaultEffectValues(model, CurrentSeqXmlFile->GetMedia(), effectName);
            timingPanel->SetDefaultControls(model);
            bufferPanel->SetDefaultControls(model);
            colorPanel->SetDefaultSettings();
        }
    }

    EffectsPanel1->SetEffectPanelStatus(model, effectName);
    SetEffectControls(settings);
    SetEffectControls(palette);
    RenderableEffect *ef = GetEffectManager().GetEffect(effectName);
    if (ef != nullptr) {
        colorPanel->SetColorCount(ef->GetColorSupportedCount());
    } else {
        colorPanel->SetColorCount(8);
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.warn("Setting effect controls for unknown effect type: %s", (const char *)effectName.c_str());
    }
    colorPanel->SetSupports(ef->SupportsLinearColorCurves(settings), ef->SupportsRadialColorCurves(settings));
}

void xLightsFrame::ApplySetting(wxString name, const wxString &value)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxWindow* ContextWin;
	if (name.StartsWith("E_"))
	{
		ContextWin = EffectsPanel1;
	}
	else if (name.StartsWith("T_"))
	{
		if (name == "T_CHECKBOX_OverlayBkg") {
			//temporary until this key is remapped
			ContextWin = bufferPanel;
		}
		else {
			ContextWin = timingPanel;
		}
	}
	else if (name.StartsWith("B_"))
	{
		ContextWin = bufferPanel;
	}
	else if (name.StartsWith("C_"))
	{
		ContextWin = colorPanel;
	}
    else if (name.StartsWith("X_"))
    {
        return;
    }
    else
	{
		return;
		//efPanel = nullptr;
		//ContextWin=SeqPanelLeft;
	}
	name = "ID_" + name.Mid(2);

	wxWindow *CtrlWin = wxWindow::FindWindowByName(name, ContextWin);
	if (CtrlWin)
	{
		if (name.StartsWith("ID_SLIDER"))
		{
			wxSlider* ctrl = (wxSlider*)CtrlWin;
            long tempLong;
			if (value.ToLong(&tempLong)) {
				ctrl->SetValue(tempLong);

				wxScrollEvent event(wxEVT_SLIDER, ctrl->GetId());
				event.SetEventObject(ctrl);
				event.SetInt(tempLong);
				ctrl->ProcessWindowEvent(event);
			}
		}
		else if (name.StartsWith("ID_TEXTCTRL"))
		{
			wxTextCtrl* ctrl = (wxTextCtrl*)CtrlWin;
			ctrl->SetValue(value);
		}
		else if (name.StartsWith("ID_SPINCTRL"))
		{
			wxSpinCtrl* ctrl = (wxSpinCtrl*)CtrlWin;
			ctrl->SetValue(wxAtoi(value));
		}
		else if (name.StartsWith("ID_CHOICE"))
		{
			wxString nn = "IDD_RADIOBUTTON" + name.SubString(9, name.size());
			wxRadioButton *b = (wxRadioButton*)wxWindow::FindWindowByName(nn, ContextWin);
			if (b != nullptr) {
				b->SetValue(true);
				wxCommandEvent evt(wxEVT_RADIOBUTTON, b->GetId());
				evt.SetEventObject(b);
				wxPostEvent(b->GetEventHandler(), evt);
			}

			wxChoice* ctrl = (wxChoice*)CtrlWin;
			ctrl->SetStringSelection(value);

			wxCommandEvent event(wxEVT_CHOICE, ctrl->GetId());
			event.SetEventObject(ctrl);
			event.SetString(value);
			ctrl->ProcessWindowEvent(event);
		}
		else if (name.StartsWith("ID_BUTTON"))
		{
            if (name.StartsWith("ID_BUTTON_Palette"))
            {
                colorPanel->SetButtonColor((ColorCurveButton*)CtrlWin, value.ToStdString());
            }
		}
		else if (name.StartsWith("ID_CHECKBOX"))
		{
			wxCheckBox* ctrl = (wxCheckBox*)CtrlWin;
            long tempLong;
			if (value.ToLong(&tempLong)) {
				ctrl->SetValue(tempLong != 0);
				wxCommandEvent evt(wxEVT_COMMAND_CHECKBOX_CLICKED, ctrl->GetId());
				evt.SetEventObject(ctrl);
				evt.SetInt(tempLong != 0);
				ctrl->ProcessWindowEvent(evt);
			}
		}
		else if (name.StartsWith("ID_NOTEBOOK"))
		{
			wxNotebook* ctrl = (wxNotebook*)CtrlWin;
			for (size_t z = 0; z < ctrl->GetPageCount(); z++)
			{
				if (value == ctrl->GetPageText(z))
				{
					ctrl->SetSelection(z);
				}
			}
		}
		else if (name.StartsWith("ID_FILEPICKER"))
		{
			wxFilePickerCtrl *picker = (wxFilePickerCtrl*)CtrlWin;
			picker->SetFileName(value);

			wxFileDirPickerEvent evt(wxEVT_FILEPICKER_CHANGED, picker, picker->GetId(), value);
			evt.SetEventObject(picker);
			picker->ProcessWindowEvent(evt);
		}
		else if (name.StartsWith("ID_FONTPICKER"))
		{
			wxFontPickerCtrl *picker = (wxFontPickerCtrl*)CtrlWin;
			wxFont oldfont;
			oldfont.SetNativeFontInfoUserDesc(value);
			picker->SetSelectedFont(oldfont);
		}
        else if (name.StartsWith("ID_CUSTOM"))
        {
            xlCustomControl *custom = dynamic_cast<xlCustomControl *>(CtrlWin);
            custom->SetValue(value.ToStdString());
        }
        else if (name.StartsWith("ID_VALUECURVE"))
        {
            ValueCurveButton *vcb = dynamic_cast<ValueCurveButton *>(CtrlWin);
            vcb->SetValue(value.ToStdString());
        }
        else
		{
			wxMessageBox("Unknown type: " + name, "Internal Error");
            logger_base.warn("Unknown type: " + name);
        }
	}
	else
	{
		if (name.StartsWith("ID_")) {
			//check if the control has been renamed to be ignored
			wxString nn = "IDD_" + name.SubString(3, name.size());
			CtrlWin = wxWindow::FindWindowByName(nn, ContextWin);
		}
		if (CtrlWin == nullptr) {
			wxMessageBox("Unable to find: " + name, "Internal Error");
            logger_base.warn("Unable to find : " + name);
        }
	}
}

void xLightsFrame::ApplyLast(wxCommandEvent& event)
{
    // Handle the EVT_APPLYLAST event
    SettingsMap* pSettingsMap = (SettingsMap*)event.GetClientData();
    SetEffectControlsApplyLast(*pSettingsMap);
    delete pSettingsMap;
}

void xLightsFrame::SetEffectControlsApplyLast(const SettingsMap &settings) {
    // Now Apply those settings with APPLYLAST in their name ... last
    for (std::map<std::string, std::string>::const_iterator it = settings.begin(); it != settings.end(); ++it) {
        if (it->first.find("APPLYLAST") != std::string::npos)
        {
            ApplySetting(wxString(it->first.c_str()), wxString(it->second.c_str()));
        }
    }
}

void xLightsFrame::SetEffectControls(const SettingsMap &settings) {

    bool applylast = false;

	// Apply those settings without APPLYLAST in their name first
    for (std::map<std::string,std::string>::const_iterator it=settings.begin(); it!=settings.end(); ++it) {
		if (it->first.find("APPLYLAST") == std::string::npos)
		{
			ApplySetting(wxString(it->first.c_str()), wxString(it->second.c_str()));
		}
        else
        {
            applylast = true;
        }
    }

    if (applylast)
    {
        // we do this asynchronously as we tyically need other events to process first
        wxCommandEvent event(EVT_APPLYLAST);
        event.SetClientData(new SettingsMap(settings));
        wxPostEvent(this, event);
    }

    MixTypeChanged=true;
    FadesChanged=true;
}

std::string xLightsFrame::GetEffectTextFromWindows(std::string &palette)
{
    RenderableEffect *eff = effectManager[EffectsPanel1->EffectChoicebook->GetSelection()];
    if (eff == nullptr)
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.crit("xLightsFrame::GetEffectTextFromWindows eff returned nullptr for effect %d. This is going to crash.", EffectsPanel1->EffectChoicebook->GetSelection());
    }
    std::string effectText= eff->GetEffectString();
    if (effectText.size() > 0 && effectText[effectText.size()-1] != ',') {
        effectText += ",";
    }
    effectText += timingPanel->GetTimingString();
    effectText += bufferPanel->GetBufferString();
    palette = colorPanel->GetColorString();
    return effectText;
}

void xLightsFrame::ForceSequencerRefresh(wxCommandEvent& event)
{
    DoForceSequencerRefresh();
}

void xLightsFrame::DoForceSequencerRefresh()
{
    mSequenceElements.PopulateRowInformation();
    ResizeMainSequencer();
}

void xLightsFrame::DoLoadPerspective(wxXmlNode *perspective)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (perspective == nullptr)
    {
        logger_base.warn("xLightsFrame::LoadPerspective Null perspective node.");
        return;
    }
    if (PerspectivesNode == nullptr)
    {
        logger_base.warn("xLightsFrame::LoadPerspective Null PerspectivesNode.");
        return;
    }

    wxString name = perspective->GetAttribute("name");
    wxString settings = perspective->GetAttribute("settings");
    if (name != PerspectivesNode->GetAttribute("current")) {
        PerspectivesNode->DeleteAttribute("current");
        PerspectivesNode->AddAttribute("current", name);
        UnsavedRgbEffectsChanges=true;
        mCurrentPerpective = perspective;
    }
    if (settings.size() == 0)
    {
        settings = m_mgr->SavePerspective();
        mCurrentPerpective->DeleteAttribute("settings");
        mCurrentPerpective->AddAttribute("settings", settings);
        mCurrentPerpective->AddAttribute("version", "2.0");
    }

    m_mgr->LoadPerspective(settings,true);
    if (perspective->GetAttribute("version", "1.0") == "1.0") {
        //title on Layer Timing panel changed
        m_mgr->GetPane("LayerTiming").Caption("Layer Blending");

        //did not have the layer settings panel
        m_mgr->GetPane("LayerSettings").Caption("Layer Settings").Dock().Left().Show();

        _modelPreviewPanel->Refresh(false);
        _housePreviewPanel->Refresh(false);
        m_mgr->Update();

        perspective->DeleteAttribute("settings");
        perspective->DeleteAttribute("version");
        perspective->AddAttribute("version", "2.0");
        perspective->AddAttribute("settings", m_mgr->SavePerspective());
    } else {
        _modelPreviewPanel->Refresh(false);
        _housePreviewPanel->Refresh(false);
        m_mgr->Update();
    }

    if( mEffectAssistMode == EFFECT_ASSIST_ALWAYS_OFF )
    {
        SetEffectAssistWindowState(false);
    }
    else if( mEffectAssistMode == EFFECT_ASSIST_ALWAYS_ON )
    {
        bool visible = m_mgr->GetPane("EffectAssist").IsShown();
        if( !visible )
        {
            mEffectAssistMode = EFFECT_ASSIST_NOT_IN_PERSPECTIVE;
            MenuItemEffectAssistAlwaysOn->Check(false);
        }
    }

    for (int i=0; i<10; i++) {
        if (perspectives[i].p == perspective) {
            MenuItemPerspectives->Check(perspectives[i].id, true);
        }
    }

}

void xLightsFrame::LoadPerspective(wxCommandEvent& event)
{
    wxXmlNode* perspective = (wxXmlNode*)(event.GetClientData());
    mCurrentPerpective = perspective;
    DoLoadPerspective(mCurrentPerpective);
}

void xLightsFrame::OnMenuItemViewSavePerspectiveSelected(wxCommandEvent& event)
{
    if(mCurrentPerpective != nullptr)
    {
        wxMessageDialog confirm(this, _("Are you sure you want to save the current view as perpective \"") + mCurrentPerpective->GetAttribute("name") + "\"?", _("Confirm"), wxYES|wxNO);
        if (confirm.ShowModal() == wxID_YES)
        {
            if(mCurrentPerpective->HasAttribute("settings"))
            {
                mCurrentPerpective->DeleteAttribute("settings");
            }
            mCurrentPerpective->AddAttribute("settings",m_mgr->SavePerspective());
            mCurrentPerpective->DeleteAttribute("version");
            mCurrentPerpective->AddAttribute("version", "2.0");
            SaveEffectsFile();
        }
    }
}

void xLightsFrame::OnMenuItemViewSaveAsPerspectiveSelected(wxCommandEvent& event)
{
    wxString name = wxGetTextFromUser("Enter name of perspective","Perspective Name");
    if(name.size()>0) {
        for(wxXmlNode* p=PerspectivesNode->GetChildren(); p!=nullptr; p=p->GetNext() )
        {
            if (p->GetName() == "perspective")
            {
                wxString check_name=p->GetAttribute("name");
                if (check_name == name)
                {
                    int answer = wxMessageBox("Enter new name?", "Duplicate Name", wxYES_NO );
                    if (answer == wxYES) {
                        OnMenuItemViewSaveAsPerspectiveSelected(event);
                    }
                    return;
                }
            }
        }

        wxXmlNode* p=new wxXmlNode(wxXML_ELEMENT_NODE, "perspective");
        p->AddAttribute("name", name);
        p->AddAttribute("settings","");
        PerspectivesNode->AddChild(p);
        mCurrentPerpective=p;
        OnMenuItemViewSavePerspectiveSelected(event);
        PerspectivesChanged(event);
        DoLoadPerspective(mCurrentPerpective);
        wxCommandEvent eventPerspectivesChanged(EVT_PERSPECTIVES_CHANGED);
        wxPostEvent(this, eventPerspectivesChanged);
    }
}

void xLightsFrame::PerspectivesChanged(wxCommandEvent& event)
{
    LoadPerspectivesMenu(PerspectivesNode);
    UnsavedRgbEffectsChanges = true;
}

void xLightsFrame::ShowDisplayElements(wxCommandEvent& event)
{
    displayElementsPanel->Initialize();
    wxAuiPaneInfo & info = m_mgr->GetPane("DisplayElements");
    info.BestSize(displayElementsPanel->GetMinSize());
    info.Show();
    m_mgr->Update();
}

void xLightsFrame::OnMenuDockAllSelected(wxCommandEvent& event)
{
    ResetAllSequencerWindows();
}

void xLightsFrame::ShowHideBufferSettingsWindow(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("LayerSettings").IsShown();
    if (visible) {
        m_mgr->GetPane("LayerSettings").Hide();
    } else {
        m_mgr->GetPane("LayerSettings").Show();
    }
    m_mgr->Update();
}

void xLightsFrame::ShowHideDisplayElementsWindow(wxCommandEvent& event)
{
    if (!m_mgr->GetPane("DisplayElements").IsOk()) return;

    bool visible = m_mgr->GetPane("DisplayElements").IsShown();
    if (visible) {
        m_mgr->GetPane("DisplayElements").Hide();
    } else {
        m_mgr->GetPane("DisplayElements").Show();
    }
    m_mgr->Update();
}

void xLightsFrame::ShowHideEffectSettingsWindow(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("Effect").IsShown();
    if (visible) {
        m_mgr->GetPane("Effect").Hide();
    } else {
        m_mgr->GetPane("Effect").Show();
    }
    m_mgr->Update();
}

void xLightsFrame::ShowHideColorWindow(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("Color").IsShown();
    if (visible) {
        m_mgr->GetPane("Color").Hide();
    } else {
        m_mgr->GetPane("Color").Show();
    }
    m_mgr->Update();
}

void xLightsFrame::ShowHideLayerTimingWindow(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("LayerTiming").IsShown();
    if (visible) {
        m_mgr->GetPane("LayerTiming").Hide();
    } else {
        m_mgr->GetPane("LayerTiming").Show();
    }
    m_mgr->Update();
}

void xLightsFrame::ShowHideModelPreview(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("ModelPreview").IsShown();
    if (visible) {
        m_mgr->GetPane("ModelPreview").Hide();
    } else {
        m_mgr->GetPane("ModelPreview").Show();
    }
    m_mgr->Update();
}

void xLightsFrame::ShowHideHousePreview(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("HousePreview").IsShown();
    if (visible) {
        m_mgr->GetPane("HousePreview").Hide();
    } else {
        m_mgr->GetPane("HousePreview").Show();
    }
    m_mgr->Update();
}

void xLightsFrame::ShowHideEffectDropper(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("EffectDropper").IsShown();
    if (visible) {
        m_mgr->GetPane("EffectDropper").Hide();
    } else {
        m_mgr->GetPane("EffectDropper").Show();
    }
    m_mgr->Update();
}

void xLightsFrame::ShowHidePerspectivesWindow(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("Perspectives").IsShown();
    if (visible) {
        m_mgr->GetPane("Perspectives").Hide();
    } else {
        m_mgr->GetPane("Perspectives").Show();
    }
    m_mgr->Update();
}

void xLightsFrame::ShowHideEffectAssistWindow(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("EffectAssist").IsShown();
    if (visible) {
        m_mgr->GetPane("EffectAssist").Hide();
        mEffectAssistMode = EFFECT_ASSIST_ALWAYS_OFF;
        MenuItemEffectAssistAlwaysOn->Check(false);
        MenuItemEffectAssistAlwaysOff->Check(true);
        MenuItemEffectAssistToggleMode->Check(false);
    } else {
        m_mgr->GetPane("EffectAssist").Show();
        mEffectAssistMode = EFFECT_ASSIST_ALWAYS_ON;
        MenuItemEffectAssistAlwaysOn->Check(true);
        MenuItemEffectAssistAlwaysOff->Check(false);
        MenuItemEffectAssistToggleMode->Check(false);
    }
    m_mgr->Update();
}

TimingElement* xLightsFrame::AddTimingElement(const std::string& name)
{
    std::string n = name;
    int nn = 1;

    // make sure the name is unique ... adding a sequence number if it isnt
    while(mSequenceElements.GetElement(n) != nullptr)
    {
        n = wxString::Format("%s-%d", name.c_str(), nn++).ToStdString();
    }

    // Deactivate active timing mark so new one is selected;
    mSequenceElements.DeactivateAllTimingElements();
    int timingCount = mSequenceElements.GetNumberOfTimingElements();
    std::string type = "timing";
    TimingElement* e = dynamic_cast<TimingElement*>(mSequenceElements.AddElement(timingCount,n,type,true,false,true,false));
    e->AddEffectLayer();
    mSequenceElements.AddTimingToCurrentView(n);
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(this, eventRowHeaderChanged);
    return e;
}

void xLightsFrame::DeleteTimingElement(const std::string& name)
{
    mSequenceElements.DeleteElement(name);
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(this, eventRowHeaderChanged);
}

void xLightsFrame::RenameTimingElement(const std::string& old_name, const std::string& new_name)
{
    Element* element = mSequenceElements.GetElement(old_name);
    if( element ) element->SetName(new_name);
    mSequenceElements.RenameTimingTrack(old_name, new_name);
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(this, eventRowHeaderChanged);
    CurrentSeqXmlFile->SetTimingSectionName(old_name, new_name);
}

int LowerTS(float t, int intervalMS)
{
    int res = t * 1000;
    res += intervalMS / 2;
    res /= intervalMS;
    res *= intervalMS;
    return res;
}

int UpperTS(float t, int intervalMS)
{
    int res = t * 1000;
    res += intervalMS / 2;
    res /= intervalMS;
    res *= intervalMS;
    return res;
}

void PTProgress(wxProgressDialog* pd, int p)
{
    // Update progress dialog
    if (pd != nullptr)
    {
        pd->Update(p);
    }
}

std::map<int, std::list<float>> xLightsFrame::LoadPolyphonicTranscription(AudioManager* audio, int intervalMS)
{
    static log4cpp::Category &logger_pianodata = log4cpp::Category::getInstance(std::string("log_pianodata"));
    std::map<int, std::list<float>> res;

    if (audio != nullptr)
    {
        try
        {
            if (!audio->IsPolyphonicTranscriptionDone())
            {
                wxProgressDialog pd("Processing Audio", "");
                logger_pianodata.info("Processing Polyphonic Transcription to produce notes");
                audio->DoPolyphonicTranscription(&pd, &PTProgress);
                logger_pianodata.info("Processing Polyphonic Transcription - DONE");
            }
        }
        catch (...)
        {
            logger_pianodata.warn("Exception caught processing Polyphonic Transcription");
        }

        logger_pianodata.debug("Interval %d.", intervalMS);

        int frames = audio->LengthMS() / intervalMS;

        for (size_t i = 0; i < frames; i++)
        {
            std::list<float>* pdata = audio->GetFrameData(i, FRAMEDATA_NOTES, "");
            if (pdata != nullptr)
            {
                res[i*intervalMS] = *pdata;
            }
        }

        if (logger_pianodata.isDebugEnabled())
        {
            logger_pianodata.debug("Note data calculated:");
            logger_pianodata.debug("Time MS, Keys");
            for (auto it = res.begin(); it != res.end(); ++it)
            {
                std::string keys = "";
                for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
                {
                    keys += " " + std::string(wxString::Format("%f", *it2).c_str());
                }
                logger_pianodata.debug("%d,%s", it->first, (const char *)keys.c_str());
            }
        }

    }
    else
    {
        logger_pianodata.warn("Polyphonic Transcription requires a media file to scan.");
    }

    return res;
}

std::map<int, std::list<float>> xLightsFrame::LoadAudacityFile(std::string file, int intervalMS)
{
    static log4cpp::Category &logger_pianodata = log4cpp::Category::getInstance(std::string("log_pianodata"));
    std::map<int, std::list<float>> res;

    logger_pianodata.debug("Processing audacity file " + file);
    logger_pianodata.debug("Interval %d.", intervalMS);
    logger_pianodata.debug("Start,End,midinote");

    wxTextFile f(file);

    if (f.Open())
    {
        wxString l = f.GetFirstLine();
        while (!f.Eof())
        {
            if (l != "") // skip blank lines
            {
                std::vector<float> components = NoteImportDialog::Parse(l);
                if (components.size() != 3)
                {
                    // this is a problem ... there should be 3 floating point numbers
                    logger_pianodata.warn("Invalid data in audacity file - 3 tab separated floating point values expected: '" + l + "'");
                    break;
                }
                else
                {
                    int start = LowerTS(components[0], intervalMS);
                    int end = UpperTS(components[1], intervalMS);
                    logger_pianodata.debug("%f,%f,%f -> %d,%d", components[0], components[1], components[2], start, end);
                    for (int i = start; i < end; i += intervalMS)
                    {
                        if (res.find(i) == res.end())
                        {
                            std::list<float> ff;
                            ff.push_back(components[2]);
                            res[i] = ff;
                        }
                        else
                        {
                            bool found = false;
                            for (auto it = res[i].begin(); it != res[i].end(); ++it)
                            {
                                if (*it == components[2])
                                {
                                    found = true;
                                    break;
                                }
                            }
                            if (!found)
                            {
                                res[i].push_back(components[2]);
                            }
                        }
                    }
                }
            }
            l = f.GetNextLine();
        }

        if (logger_pianodata.isDebugEnabled())
        {
            logger_pianodata.debug("Note data calculated:");
            logger_pianodata.debug("Time MS, Keys");
            for (auto it = res.begin(); it != res.end(); ++it)
            {
                std::string keys = "";
                for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
                {
                    keys += " " + std::string(wxString::Format("%f", *it2).c_str());
                }
                logger_pianodata.debug("%d,%s", it->first, (const char *)keys.c_str());
            }
        }
    }

    return res;
}

std::map<int, std::list<float>> xLightsFrame::LoadMusicXMLFile(std::string file, int intervalMS, int speedAdjust, int startAdjustMS, std::string track)
{
    static log4cpp::Category &logger_pianodata = log4cpp::Category::getInstance(std::string("log_pianodata"));
    std::map<int, std::list<float>> res;

    float speedadjust = speedAdjust / 100.0;

    MusicXML musicXML(file);

    if (musicXML.IsOk())
    {
        std::list<MusicXmlNote> notes = musicXML.GetNotes(track);

        for (auto it = notes.begin(); it != notes.end(); ++it)
        {
            if (!it->IsRest())
            {
                float startt = (float)startAdjustMS / 100.0 + (float)it->startMS / 1000.0 * speedadjust;
                float endt= (float)startAdjustMS / 100.0 + ((float)it->startMS + (float)it->durationMS) / 1000.0 * speedadjust;
                int start = LowerTS(startt, intervalMS);
                int end = UpperTS(endt, intervalMS);

                for (int i = start; i < end; i += intervalMS)
                {
                    if (i >= 0)
                    {
                        if (res.find(i) == res.end())
                        {
                            std::list<float> ff;
                            ff.push_back(it->midi);
                            res[i] = ff;
                        }
                        else
                        {
                            bool found = false;
                            for (auto it2 = res[i].begin(); it2 != res[i].end(); ++it2)
                            {
                                if (*it2 == it->midi)
                                {
                                    found = true;
                                    break;
                                }
                            }
                            if (!found)
                            {
                                res[i].push_back(it->midi);
                            }
                        }
                    }
                }
            }
        }

        if (logger_pianodata.isDebugEnabled())
        {
            logger_pianodata.debug("Note data calculated:");
            logger_pianodata.debug("Time MS, Keys");
            for (auto it = res.begin(); it != res.end(); ++it)
            {
                std::string keys = "";
                for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
                {
                    keys += " " + std::string(wxString::Format("%f", *it2).c_str());
                }
                logger_pianodata.debug("%d,%s", it->first, (const char *)keys.c_str());
            }
        }
    }
    else
    {
        logger_pianodata.warn("Invalid MusicXML file " + file);
    }

    return res;
}

std::map<int, std::list<float>> xLightsFrame::LoadMIDIFile(std::string file, int intervalMS, int speedAdjust, int startAdjustMS, std::string track)
{
    static log4cpp::Category &logger_pianodata = log4cpp::Category::getInstance(std::string("log_pianodata"));
    std::map<int, std::list<float>> res;

    float speedadjust = speedAdjust / 100.0;

    int notestate[128];
    for (int i = 0; i <= 127; i++)
    {
        notestate[i] = 0;
    }

    logger_pianodata.debug("Processing midi file " + file);
    logger_pianodata.debug("Interval %d.", intervalMS);
    logger_pianodata.debug("SpeedAdjust %d.", speedAdjust);
    logger_pianodata.debug("StartAdjustMS %d.", startAdjustMS);

    MidiFile midifile;
    float lasttime = -1;
    if (midifile.read(file) != 0)
    {
        int ntrack = 0;
        if (track == "All" || track == "")
        {
            midifile.joinTracks();
        }
        else
        {
            ntrack = wxAtoi(track) - 1;
            if (ntrack >= midifile.getNumTracks())
            {
                ntrack = 0;
            }
        }
        midifile.doTimeAnalysis();

        logger_pianodata.debug("Processing midi track %d.", ntrack);
        logger_pianodata.debug("Event,time(s)->ms,adjustedtime(s)->ms,isnote,isnoteon,isnoteoff,midinote");

        // process each event
        for (int i = 0; i < midifile.getNumEvents(ntrack); i++)
        {
            MidiEvent e = midifile.getEvent(ntrack, i);

            if (e.isNote())
            {
                float time = (float)startAdjustMS / 100.0 + midifile.getTimeInSeconds(ntrack, i) * speedadjust;

                if (logger_pianodata.isDebugEnabled())
                {
                    logger_pianodata.debug("%d,%f->%d,%f->%d,%d,%d,%d,%d", i, midifile.getTimeInSeconds(ntrack, i), LowerTS(midifile.getTimeInSeconds(ntrack, i), intervalMS), time, LowerTS(time, intervalMS), e.isNote(), e.isNoteOn(), e.isNoteOff(), e.getKeyNumber());
                }
                if (time != lasttime)
                {
                    if (lasttime >= 0)
                    {
                        // we can update things now
                        int start = LowerTS(lasttime, intervalMS);
                        int end = UpperTS(time, intervalMS);

                        for (int j = start; j < end; j += intervalMS)
                        {
                            std::list<float> f;
                            for (int k = 0; k <= 127; k++)
                            {
                                if (notestate[k] > 0)
                                {
                                    f.push_back(k);
                                }
                            }
                            res[j] = f;
                        }
                    }

                    lasttime = time;
                }

                if (e.isNoteOn())
                {
                    notestate[e.getKeyNumber()]++;
                }
                else if (e.isNoteOff())
                {
                    notestate[e.getKeyNumber()]--;
                    if (notestate[e.getKeyNumber()] < 0)
                    {
                        // this should never happen
                    }
                }
            }
        }

        if (logger_pianodata.isDebugEnabled())
        {
            logger_pianodata.debug("Note data calculated:");
            logger_pianodata.debug("Time MS, Keys");
            for (auto it = res.begin(); it != res.end(); ++it)
            {
                std::string keys = "";
                for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
                {
                    keys += " " + std::string(wxString::Format("%f", *it2).c_str());
                }
                logger_pianodata.debug("%d,%s", it->first, (const char *)keys.c_str());
            }
        }
    }
    else
    {
        logger_pianodata.warn("Invalid MIDI file " + file);
    }

    return res;
}

void xLightsFrame::ExecuteImportNotes(wxCommandEvent& command)
{
    NoteImportDialog dlgNoteImport(this, mSequenceElements, (CurrentSeqXmlFile->GetMedia() != nullptr));

    if (dlgNoteImport.ShowModal() == wxID_OK)
    {
        wxString name = dlgNoteImport.TextCtrl_TimingName->GetValue();
        mSequenceElements.DeactivateAllTimingElements();
        Element* element = AddTimingElement(std::string(name.ToStdString()));
        EffectLayer* effectLayer = element->GetEffectLayer(0);
        // mSequenceElements.AddTimingToCurrentView(name.ToStdString()); I dont think this is necessary

        int interval = CurrentSeqXmlFile->GetFrameMS();
        wxString type = dlgNoteImport.Choice_Piano_Notes_Source->GetStringSelection();
        std::map<int, std::list<float>> notes;
        if (type == "Audacity Timing File")
        {
            notes = LoadAudacityFile(dlgNoteImport.TextCtrl_Piano_File->GetValue().ToStdString(), interval);
        }
        else if (type == "Music XML File")
        {
            notes = LoadMusicXMLFile(dlgNoteImport.TextCtrl_Piano_File->GetValue().ToStdString(), interval, dlgNoteImport.Slider_Piano_MIDI_Speed->GetValue(), dlgNoteImport.Slider_Piano_MIDI_Start->GetValue(), dlgNoteImport.Choice_Piano_MIDITrack_APPLYLAST->GetStringSelection().ToStdString());
        }
        else if (type == "MIDI File")
        {
            notes = LoadMIDIFile(dlgNoteImport.TextCtrl_Piano_File->GetValue().ToStdString(), interval, dlgNoteImport.Slider_Piano_MIDI_Speed->GetValue(), dlgNoteImport.Slider_Piano_MIDI_Start->GetValue(), dlgNoteImport.Choice_Piano_MIDITrack_APPLYLAST->GetStringSelection().ToStdString());
        }
        else if (type == "Polyphonic Transcription")
        {
            notes = LoadPolyphonicTranscription(CurrentSeqXmlFile->GetMedia(), interval);
        }

        CreateNotes(effectLayer, notes, interval, CurrentSeqXmlFile->GetSequenceDurationMS() / interval);

        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(this, eventRowHeaderChanged);
    }
}

std::string xLightsFrame::DecodeMidi(int midi)
{
    int n = midi % 12;
    int o = midi / 12 - 1;
    // dont go below zero ... if so move it up an octave ... the user will never know
    while (o < 0)
    {
        o++;
    }

    bool sharp = false;
    char note = '?';
    switch(n)
    {
    case 9:
        note = 'A';
        break;
    case 10:
        note = 'A';
        sharp = true;
        break;
    case 11:
        note = 'B';
        break;
    case 0:
        note = 'C';
        break;
    case 1:
        note = 'C';
        sharp = true;
        break;
    case 2:
        note = 'D';
        break;
    case 3:
        note = 'D';
        sharp = true;
        break;
    case 4:
        note = 'E';
        break;
    case 5:
        note = 'F';
        break;
    case 6:
        note = 'F';
        sharp = true;
        break;
    case 7:
        note = 'G';
        break;
    case 8:
        note = 'G';
        sharp = true;
        break;
    }

    if (sharp)
    {
        return wxString::Format("%c#%d", note, o).ToStdString();
    }
    else
    {
        return wxString::Format("%c%d", note, o).ToStdString();
    }
}

std::string xLightsFrame::CreateNotesLabel(const std::list<float>& notes) const
{
    std::string res;

    for (auto it = notes.begin(); it != notes.end(); ++it)
    {
        if (res != "")
        {
            res += ",";
        }
        res += xLightsFrame::DecodeMidi((int)*it);
    }

    return res;
}

void xLightsFrame::CreateNotes(EffectLayer* el, std::map<int, std::list<float>>& notes, int interval, int frames)
{
    size_t last = 0;
    std::string lastLabel = "";
    for (size_t i = 0; i <= frames; i++)
    {
        std::string label = "";
        if (notes.find((float)(i*interval)) != notes.end())
        {
            label = CreateNotesLabel(notes[i*interval]);
        }

        if (label == lastLabel)
        {
            // do nothing
        }
        else
        {
            if (i != 0)
            {
                el->AddEffect(0, lastLabel, "", "", last * interval, i*interval, EFFECT_NOT_SELECTED, false);
            }
            last = i;
            lastLabel = label;
        }
    }
    el->AddEffect(0, lastLabel, "", "", last * interval, frames*interval, EFFECT_NOT_SELECTED, false);
}

void xLightsFrame::ExecuteImportTimingElement(wxCommandEvent &command) {
    ImportTimingElement();
}

void xLightsFrame::ImportTimingElement()
{
    wxFileDialog* OpenDialog = new wxFileDialog( this, "Choose Timing file(s)", wxEmptyString, wxEmptyString,
        "Timing files (*.xtiming)|*.xtiming|Papagayo files (*.pgo)|*.pgo|Text files (*.txt)|*.txt|LOR (*.lms)|*.lms|LOR (*.las)|*.las|LSP (*.msq)|*.msq|xLights (*.xml)|*.xml",
                                                wxFD_OPEN | wxFD_MULTIPLE, wxDefaultPosition);
    wxString fDir;
    if (OpenDialog->ShowModal() == wxID_OK)
    {
        fDir =	OpenDialog->GetDirectory();
        wxArrayString filenames;
        OpenDialog->GetFilenames(filenames);
        if(filenames.size() > 0)
        {
            wxFileName file1(filenames[0]);
            if( file1.GetExt().Lower() == "lms"|| file1.GetExt().Lower() == "las" )
            {
                CurrentSeqXmlFile->ProcessLorTiming(fDir, filenames, this);
            }
            else if (file1.GetExt().Lower() == "xtiming")
            {
                CurrentSeqXmlFile->ProcessXTiming(fDir, filenames, this);
            }
            else if (file1.GetExt().Lower() == "pgo")
            {
                CurrentSeqXmlFile->ProcessPapagayo(fDir, filenames, this);
            }
            else if (file1.GetExt().Lower() == "msq")
            {
                CurrentSeqXmlFile->ProcessLSPTiming(fDir, filenames, this);
            }
            else if (file1.GetExt().Lower() == "xml")
            {
                CurrentSeqXmlFile->ProcessXLightsTiming(fDir, filenames, this);
            }
            else
            {
                CurrentSeqXmlFile->ProcessAudacityTimingFiles(fDir, filenames, this);
            }
        }
    }

    OpenDialog->Destroy();
}

void xLightsFrame::OnMenuItemLoadEditPerspectiveSelected(wxCommandEvent& event)
{
    m_mgr->GetPane(wxT("Perspectives")).Show(true);
    m_mgr->Update();
}


bool isOnLine(int x1, int y1, int x2, int y2, int x3, int y3) {
    double diffx = x2 - x1;
    double diffy = y2 - y1;
    double b = y1 - diffy/diffx*x1;
    double ye1 = diffy/diffx*x3 + b;

    return (y3 + 1) >= ye1
        && (y3 - 1) <= ye1;
}

bool isOnLineColor(const xlColor &v1, const xlColor &v2, const xlColor &v3,
              int x, int x2, int x3) {
    return isOnLine(x, v1.Red(), x2, v2.Red(), x3, v3.Red())
    && isOnLine(x, v1.Green(), x2, v2.Green(), x3, v3.Green())
    && isOnLine(x, v1.Blue(), x2, v2.Blue(), x3, v3.Blue());
}
int RampLenColor(int start, std::vector<xlColor> &colors) {

    for (int s = start + 2; s < colors.size(); s++) {
        if (!isOnLineColor(colors[start], colors[s-1], colors[s],
                      start, s-1, s)) {
            return s - start;
        }
    }
    return 0;
}


void xLightsFrame::DoConvertDataRowToEffects(EffectLayer *layer, xlColorVector &colors, int frameTime) {
    colors.push_back(xlBLACK);
    int startTime = 0;
    xlColor lastColor(xlBLACK);

    for (size_t x = 0; x < colors.size()-3; x++) {
        if (colors[x] != colors[x + 1]) {
            int len = RampLenColor(x, colors);
            if (len >= 3) {

                int stime = x * frameTime;
                int etime = (x+len)*frameTime;
                if (colors[x] == xlBLACK || colors[x + len - 1] == xlBLACK) {
                    HSVValue c = colors[x].asHSV();
                    if (colors[x] == xlBLACK) {
                        c = colors[x + len - 1].asHSV();
                    }
                    c.value = 1.0;
                    xlColor c2(c);

                    int i = colors[x].asHSV().value * 100.0;
                    int i2 = colors[x + len - 1].asHSV().value * 100.0;
                    std::string settings = wxString::Format("E_TEXTCTRL_Eff_On_Start=%d,E_TEXTCTRL_Eff_On_End=%d", i, i2).ToStdString();
                    std::string palette = "C_BUTTON_Palette1=" + (std::string)c2 + ",C_CHECKBOX_Palette1=1";
                    if (!layer->HasEffectsInTimeRange(stime, etime))
                    {
                        layer->AddEffect(0, "On", settings, palette, stime, etime, false, false);
                    }
                } else {
                    std::string palette = "C_BUTTON_Palette1=" + (std::string)colors[x] + ",C_CHECKBOX_Palette1=1,"
                        "C_BUTTON_Palette2=" + (std::string)colors[x + len - 1] + ",C_CHECKBOX_Palette2=1";
                    if (!layer->HasEffectsInTimeRange(stime, etime))
                    {
                        layer->AddEffect(0, "Color Wash", "", palette, stime, etime, false, false);
                    }
                }
                for (int z = 0; z < len; z++) {
                    //clear it
                    colors[x + z] = xlBLACK;
                }
            }
        }
    }

    for (size_t x = 0; x < colors.size(); x++) {
        if (lastColor != colors[x]) {
            int time = x * frameTime;
            if (lastColor != xlBLACK) {
                std::string palette = "C_BUTTON_Palette1=" + (std::string)lastColor + ",C_CHECKBOX_Palette1=1";

                if (time != startTime) {
                    if (!layer->HasEffectsInTimeRange(startTime, time))
                    {
                        layer->AddEffect(0, "On", "", palette, startTime, time, false, false);
                    }
                }
            }
            startTime = time;
            lastColor = colors[x];
        }
    }
}

void xLightsFrame::ConvertDataRowToEffects(wxCommandEvent &event) {
    StrandElement *el = dynamic_cast<StrandElement*>((Element*)event.GetClientData());
    int node = event.GetInt() & 0xFFFF;
    EffectLayer *layer = el->GetNodeLayer(node);

    xlColorVector colors;
    PixelBufferClass ncls(this);
    Model *model = GetModel(el->GetModelName());
    for (size_t f = 0; f < SeqData.NumFrames(); f++) {
        model->SetNodeChannelValues(0, &SeqData[f][model->NodeStartChannel(0)]);
        xlColor c = model->GetNodeColor(0);
        colors.push_back(c);
    }
    DoConvertDataRowToEffects(layer, colors, SeqData.FrameTime());
}

wxXmlNode* xLightsFrame::CreateEffectNode(wxString& name)
{
    wxXmlNode* NewXml=new wxXmlNode(wxXML_ELEMENT_NODE, "effect");
    NewXml->AddAttribute("name", name);
    wxString copy_data;
    mainSequencer->GetPresetData(copy_data);
    NewXml->AddAttribute("settings", copy_data);
    NewXml->AddAttribute("version", XLIGHTS_RGBEFFECTS_VERSION);
    NewXml->AddAttribute("xLightsVersion", xlights_version_string);
    return NewXml;
}

void xLightsFrame::UpdateEffectNode(wxXmlNode* node)
{
    wxString copy_data;
    mainSequencer->GetSelectedEffectsData(copy_data);
    node->AddAttribute("settings", copy_data);
    node->DeleteAttribute("xLightsVersion");
    node->AddAttribute("xLightsVersion", xlights_version_string);
}

void xLightsFrame::ApplyEffectsPreset(wxString& data, const wxString &pasteDataVersion)
{
    mainSequencer->PanelEffectGrid->Paste(data, pasteDataVersion);
}
void xLightsFrame::PromoteEffects(wxCommandEvent &command) {
    ModelElement *el = dynamic_cast<ModelElement*>((Element*)command.GetClientData());
    DoPromoteEffects(el);
}

bool equals(Effect *e, Effect *e2, const wxString &pal, const wxString &set) {
    if (e->GetEffectIndex() != e2->GetEffectIndex()
        || e->GetStartTimeMS() != e2->GetStartTimeMS()
        || e->GetEndTimeMS() != e2->GetEndTimeMS()) {
        return false;
    }
    if (pal != e2->GetPaletteAsString()
        || set != e2->GetSettingsAsString()) {
        return false;
    }
    return true;
}
void xLightsFrame::DoPromoteEffects(ModelElement *element) {
    //first promote from nodes to strands
    for (int x = 0;  x < element->GetStrandCount(); x++) {
        StrandElement *se = element->GetStrand(x);
        EffectLayer *target = se->GetEffectLayer(0);
        if (element->GetStrandCount() <= 1) {
            if (element->GetEffectLayer(0)->GetEffectCount() != 0) {
                element->InsertEffectLayer(0);
            }
            target = element->GetEffectLayer(0);
        }
        if (se->GetNodeLayerCount() > 0) {
            NodeLayer *base = se->GetNodeLayer(0);
            for (int e = base->GetEffectCount() - 1; e >= 0; e--) {
                Effect *eff = base->GetEffect(e);
                const std::string &name = eff->GetEffectName();

                if (target->HasEffectsInTimeRange(eff->GetStartTimeMS(), eff->GetEndTimeMS())) {
                    //cannot promote, already an effect there
                    continue;
                }
                if (name == "On" || name == "Color Wash") {
                    const std::string pal = eff->GetPaletteAsString();
                    const std::string set = eff->GetSettingsAsString();
                    int mp = (eff->GetStartTimeMS() + eff->GetEndTimeMS()) / 2;
                    bool collapse = true;

                    for (int n = 1; n < se->GetNodeLayerCount() && collapse; n++) {
                        NodeLayer *node = se->GetNodeLayer(n);
                        int nodeIndex = 0;
                        if (node->HitTestEffectByTime(mp, nodeIndex)) {
                            Effect *nf = node->GetEffect(nodeIndex);
                            if (!equals(eff, nf, pal, set)) {
                                collapse = false;
                            }
                        } else {
                            collapse = false;
                        }
                    }
                    if (collapse) {
                        target->AddEffect(0, eff->GetEffectName(), set, pal, eff->GetStartTimeMS(), eff->GetEndTimeMS(), false, false);
                        for (int n = 0; n < se->GetNodeLayerCount() && collapse; n++) {
                            NodeLayer *node = se->GetNodeLayer(n);
                            int nodeIndex = 0;
                            if (node->HitTestEffectByTime(mp, nodeIndex)) {
                                node->DeleteEffectByIndex(nodeIndex);
                            }
                        }
                    }
                }
            }
        }
    }
    if (element->GetStrandCount() <= 1) return;
    //OK, we're now promoted to strand level effects, try and promote to Model level
    EffectLayer *base = element->GetStrand(0)->GetEffectLayer(0);
    if (element->GetEffectLayer(0)->GetEffectCount() != 0) {
        element->InsertEffectLayer(0);
    }
    EffectLayer *target = element->GetEffectLayer(0);
    for (int e = base->GetEffectCount() - 1; e >= 0; e--) {
        Effect *eff = base->GetEffect(e);
        const std::string &name = eff->GetEffectName();

        if (name == "On" || name == "Color Wash") {
            const std::string pal = eff->GetPaletteAsString();
            const std::string set = eff->GetSettingsAsString();
            int mp = (eff->GetStartTimeMS() + eff->GetEndTimeMS()) / 2;
            bool collapse = true;

            for (int n = 0; n < element->GetStrandCount() && collapse; n++) {
                StrandElement *se = element->GetStrand(n);
                for (int l = 0; l < se->GetEffectLayerCount(); l++) {
                    EffectLayer *node = se->GetEffectLayer(l);
                    if (node == base) {
                        continue;
                    }
                    int nodeIndex = 0;
                    if (node->HitTestEffectByTime(mp, nodeIndex)) {
                        Effect *nf = node->GetEffect(nodeIndex);
                        if (!equals(eff, nf, pal, set)) {
                            collapse = false;
                        }
                    } else {
                        collapse = false;
                    }
                }
            }
            if (collapse) {
                target->AddEffect(0, eff->GetEffectName(), set, pal, eff->GetStartTimeMS(), eff->GetEndTimeMS(), false, false);
                for (int n = 0; n < element->GetStrandCount() && collapse; n++) {
                    StrandElement *se = element->GetStrand(n);
                    for (int l = 0; l < se->GetEffectLayerCount(); l++) {
                        EffectLayer *node = se->GetEffectLayer(l);
                        int nodeIndex = 0;
                        if (node->HitTestEffectByTime(mp, nodeIndex)) {
                            node->DeleteEffectByIndex(nodeIndex);
                        }
                    }
                }
            }
        }
    }
}

void xLightsFrame::OnAuiToolBarItemShowHideEffects(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("EffectDropper").IsShown();
    if (visible) {
        m_mgr->GetPane("EffectDropper").Hide();
    }
    else {
        m_mgr->GetPane("EffectDropper").Show();
    }
    m_mgr->Update();
}

