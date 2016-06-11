#include <map>
#include <wx/utils.h> //check keyboard state -DJ
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include <wx/tipwin.h>
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
#include "../DisplayElementsPanel.h"
#include "../effects/RenderableEffect.h"
#include "../xlCustomControl.h"




/************************************* New Sequencer Code*****************************************/
void xLightsFrame::CreateSequencer()
{
    // Lots of logging here as this function hard crashes
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    EffectsPanel1 = NULL;
    timingPanel = NULL;

    logger_base.debug("CreateSequencer: Creating Panels.");
    mainSequencer = new MainSequencer(PanelSequencer);
    mainSequencer->PanelEffectGrid->SetRenderDataSources(this, &SeqData);
    mainSequencer->SetSequenceElements(&mSequenceElements);

    mainSequencer->PanelWaveForm->SetTimeline(mainSequencer->PanelTimeLine);

    mainSequencer->PanelRowHeadings->SetSequenceElements(&mSequenceElements);
    mSequenceElements.SetMaxRowsDisplayed(mainSequencer->PanelRowHeadings->GetMaxRows());

    m_mgr->SetDockSizeConstraint(0.25, 0.15);

    sPreview1 = new ModelPreview(PanelSequencer);
    m_mgr->AddPane(sPreview1,wxAuiPaneInfo().Name(wxT("ModelPreview")).Caption(wxT("Model Preview")).
                   Left().Layer(1).PaneBorder(true));
    sPreview2 = new ModelPreview(PanelSequencer, PreviewModels, false);
    m_mgr->AddPane(sPreview2,wxAuiPaneInfo().Name(wxT("HousePreview")).Caption(wxT("House Preview")).
                   Left().Layer(1));

    effectsPnl = new TopEffectsPanel(PanelSequencer);
    effectsPnl->BitmapButtonSelectedEffect->SetEffect(effectManager[0], mIconSize);

    EffectsPanel1 = new EffectsPanel(effectsPnl, &effectManager);
    EffectsPanel1->SetSequenceElements(&mSequenceElements);
    effectsPnl->EffectSizer->Add(EffectsPanel1, wxEXPAND);
    effectsPnl->MainSizer->Fit(effectsPnl);
    effectsPnl->MainSizer->SetSizeHints(effectsPnl);

    sEffectAssist = new EffectAssist(PanelSequencer);
    m_mgr->AddPane(sEffectAssist,wxAuiPaneInfo().Name(wxT("EffectAssist")).Caption(wxT("Effect Assist")).
                   Left().Layer(1));
    sEffectAssist->Layout();

    colorPanel = new ColorPanel(PanelSequencer);
    timingPanel = new TimingPanel(PanelSequencer);
    bufferPanel = new BufferPanel(PanelSequencer);

    perspectivePanel = new PerspectivesPanel(PanelSequencer);

    effectPalettePanel = new EffectIconPanel(effectManager, PanelSequencer);

    // DisplayElements Panel
    displayElementsPanel = new DisplayElementsPanel(this, PanelSequencer);
    displayElementsPanel->SetViewChoice(mainSequencer->ViewChoice);
    displayElementsPanel->Fit();

    logger_base.debug("CreateSequencer: Hooking up the panes.");
    m_mgr->AddPane(displayElementsPanel,wxAuiPaneInfo().Name(wxT("DisplayElements")).Caption(wxT("Display Elements"))
                   .Float());
    // Hide the panel on start.
    m_mgr->GetPane("DisplayElements").Hide();


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
}


void xLightsFrame::InitSequencer()
{
    if(EffectsPanel1 == NULL || timingPanel == NULL)
    {
        return;
    }
    if(mSequencerInitialize)
    {
        return;
    }
    if(mCurrentPerpective!=NULL)
    {
        LoadPerspective(mCurrentPerpective);
    }
    mSequencerInitialize = true;
    sPreview2->InitializePreview(mBackgroundImage,mBackgroundBrightness);
    sPreview2->SetScaleBackgroundImage(mScaleBackgroundImage);
}

Model *xLightsFrame::GetModel(const std::string& name) {
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
    mCurrentPerpective = NULL;
    if (prospectives==NULL)
    {
        if(PerspectivesNode->HasAttribute("current"))
        {
            PerspectivesNode->DeleteAttribute("current");
        }
        PerspectivesNode->AddAttribute("current","Default Perspective");
        wxXmlNode* p=new wxXmlNode(wxXML_ELEMENT_NODE, "perspective");
        p->AddAttribute("name", "Default Perspective");
        p->AddAttribute("settings",m_mgr->SavePerspective());
        p->AddAttribute("version", "2.0");
        PerspectivesNode->AddChild(p);
        mCurrentPerpective = p;
        UnsavedRgbEffectsChanges=true;
    }
    else
    {
        wxString currentName = PerspectivesNode->GetAttribute("current");
        for(wxXmlNode* p=PerspectivesNode->GetChildren(); p!=NULL; p=p->GetNext())
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
    for (auto it = names.begin(); it != names.end(); it++) {
        ret.push_back(*it);
    }
    return ret;
}
static void Remove(std::vector<std::string> &names, const std::string &str) {
    names.erase(std::remove(names.begin(), names.end(), str), names.end());
}

void xLightsFrame::CheckForValidModels()
{
    //bool cancelled = cancelled_in;
    bool cancelled = false;

    std::vector<std::string> AllNames;
    std::vector<std::string> ModelNames;
    for (auto it = AllModels.begin(); it != AllModels.end(); it++) {
        AllNames.push_back(it->first);
        if (it->second->GetDisplayAs() != "ModelGroup") {
            ModelNames.push_back(it->first);
        }
    }

    for (int x = mSequenceElements.GetElementCount()-1; x >= 0; x--) {
        if ("model" == mSequenceElements.GetElement(x)->GetType()) {
            std::string name = mSequenceElements.GetElement(x)->GetName();
            //remove the current models from the list so we don't end up with the same model represented twice
            Remove(AllNames, name);
            Remove(ModelNames, name);
        }
    }

    SeqElementMismatchDialog dialog(this);
    for (int x = mSequenceElements.GetElementCount()-1; x >= 0; x--) {
        if ("model" == mSequenceElements.GetElement(x)->GetType()) {
            std::string name = mSequenceElements.GetElement(x)->GetName();
            Model *m = AllModels[name];
            if (m == nullptr) {
                dialog.StaticTextMessage->SetLabel("Model '"+name+"'\ndoes not exist in your list of models");
                dialog.ChoiceModels->Set(ToArrayString(AllNames));
                dialog.Fit();

                if (!cancelled)
                {
                    cancelled = (dialog.ShowModal() == wxID_CANCEL);
                }

                if (cancelled || dialog.RadioButtonDelete->GetValue()) {
                    mSequenceElements.DeleteElement(name);
                    x = mSequenceElements.GetElementCount();
                } else {
                    std::string newName = dialog.ChoiceModels->GetStringSelection().ToStdString();
                    mSequenceElements.GetElement(x)->SetName(newName);
                    Remove(AllNames, newName);
                    Remove(ModelNames, newName);
                    x = mSequenceElements.GetElementCount();
                }
            }
        }
    }
    for (int x = mSequenceElements.GetElementCount()-1; x >= 0; x--) {
        if ("model" == mSequenceElements.GetElement(x)->GetType()) {
            std::string name = mSequenceElements.GetElement(x)->GetName();
            Model *m = AllModels[name];
            if (m->GetDisplayAs() == "ModelGroup") {
                Element * el = mSequenceElements.GetElement(x);
                bool hasStrandEffects = false;
                bool hasNodeEffects = false;
                for (int l = 0; l < el->getStrandLayerCount(); l++) {
                    StrandLayer *sl = el->GetStrandLayer(l);
                    if (sl->GetEffectCount() > 0) {
                        hasStrandEffects = true;
                    }
                    for (int n = 0; n < sl->GetNodeLayerCount(); n++) {
                        if (sl->GetNodeLayer(n)->GetEffectCount()) {
                            hasNodeEffects = true;
                        }
                    }
                }
                if (hasNodeEffects || hasStrandEffects) {
                    wxArrayString choices;
                    choices.push_back("Rename the model in the sequence");
                    choices.push_back("Delete the model in the sequence");
                    choices.push_back("Map the Strand/Node effects to different models");
                    choices.push_back("Ignore (Handle Later) - Effects will not render");

                    wxSingleChoiceDialog dlg(this, "Model " + name + " is a Model Group but has Node/Strand effects.\n"
                                             + "How should we handle this?",
                                             "Warning", choices);
                    if (dlg.ShowModal() == wxID_OK) {
                        switch (dlg.GetSelection()) {
                            case 0: {
                                    wxSingleChoiceDialog namedlg(this, "Choose the model to use instead:",
                                                             "Select Model", ToArrayString(ModelNames));
                                    if (namedlg.ShowModal() == wxID_OK) {
                                        std::string newName = namedlg.GetStringSelection().ToStdString();
                                        mSequenceElements.GetElement(x)->SetName(newName);
                                        Remove(AllNames, newName);
                                        Remove(ModelNames, newName);
                                    }
                                }
                                break;
                            case 1:
                                mSequenceElements.DeleteElement(name);
                                break;
                            case 2:
                                ImportXLights(mSequenceElements, std::vector<Element *> {el}, wxFileName(), true, true);
                                //relo
                                x++;
                                break;
                            case 3:
                                break;
                        }
                    }
                }
            }
        }
    }
}

void xLightsFrame::LoadAudioData(xLightsXmlFile& xml_file)
{
    mMediaLengthMS = xml_file.GetSequenceDurationMS();

    if(xml_file.GetSequenceType()=="Media")
    {
        int musicLength = 0;
		mediaFilename = wxEmptyString;
		if (xml_file.GetMedia() != NULL)
		{
			mediaFilename = xml_file.GetMedia()->FileName();
			if ((mediaFilename == wxEmptyString) || (!wxFileExists(mediaFilename)))
			{
				SeqSettingsDialog setting_dlg(this, &xml_file, mediaDirectory, wxT("Please select Media file!!!"));
				setting_dlg.Fit();
				setting_dlg.ShowModal();
				mediaFilename = xml_file.GetMedia()->FileName();
				if (xml_file.GetMedia()->GetFrameInterval() < 0)
				{
					xml_file.GetMedia()->SetFrameInterval(xml_file.GetSequenceTimingAsInt());
				}
				SetAudioControls();
			}
		}
        if( mediaFilename != wxEmptyString )
        {
			wxString error;
            musicLength = mainSequencer->PanelWaveForm->OpenfileMedia(xml_file.GetMedia(), error);
            if(musicLength <=0)
            {
                wxMessageBox(wxString::Format("Media File Missing or Corrupted.\n\nDetails: %s", error));
            }
        }
        else if (xml_file.GetSequenceType() == "Media")
        {
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
    mSequenceElements.SetViewsNode(ViewsNode); // This must come first before LoadSequencerFile.
    mSequenceElements.SetModelsNode(ModelsNode);
    mSequenceElements.SetEffectsNode(EffectsNode);
    mSequenceElements.LoadSequencerFile(xml_file, GetShowDirectory());
    xml_file.AdjustEffectSettingsForVersion(mSequenceElements, this);
    mSequenceElements.PopulateRowInformation();

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
    CheckForValidModels();

    LoadAudioData(xml_file);

    mainSequencer->PanelEffectGrid->SetSequenceElements(&mSequenceElements);
    mainSequencer->PanelEffectGrid->SetTimeline(mainSequencer->PanelTimeLine);
    mainSequencer->PanelTimeLine->SetSequenceEnd(CurrentSeqXmlFile->GetSequenceDurationMS());
    mSequenceElements.SetSequenceEnd(CurrentSeqXmlFile->GetSequenceDurationMS());
    ResizeAndMakeEffectsScroll();
    ResizeMainSequencer();
    mainSequencer->PanelEffectGrid->Refresh();
    sPreview1->Refresh();
    sPreview2->Refresh();
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
        for(wxXmlNode* e=ModelGroupsNode->GetChildren(); e!=NULL; e=e->GetNext() ) {
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
    if (playType != PLAY_TYPE_MODEL && playType != PLAY_TYPE_MODEL_PAUSED) {
        playType = PLAY_TYPE_STOPPED;
        playStartTime = -1;
        playEndTime = -1;
        playStartMS = -1;
    }
    selectedEffect = NULL;
    selectedEffectName = "";
    selectedEffectString = "";
    selectedEffectPalette = "";
    sPreview1->Refresh();
    sPreview2->Refresh();
}

void xLightsFrame::EffectChanged(wxCommandEvent& event)
{
    Effect* effect = (Effect*)event.GetClientData();
    SetEffectControls(effect->GetParentEffectLayer()->GetParentElement()->GetName(),
                      effect->GetEffectName(), effect->GetSettings(), effect->GetPaletteMap(),
                      true);
    selectedEffectString = "";  // force update to effect rendering
}

void xLightsFrame::SelectedEffectChanged(wxCommandEvent& event)
{
    bool OnlyChoiceBookPage = event.GetClientData()==nullptr?true:false;
    Effect* effect = nullptr;
    if(OnlyChoiceBookPage)
    {
        int pageIndex = event.GetInt();
        // Dont change page if it is already on correct page
        if (EffectsPanel1->EffectChoicebook->GetSelection()!=pageIndex)
        {
            EffectsPanel1->EffectChoicebook->SetSelection(pageIndex);
        }
    }
    else
    {
        effect = (Effect*)event.GetClientData();
		bool resetStrings = false;
        if ("Random" == effect->GetEffectName()) {
            std::string settings, palette;
            std::string effectName = CreateEffectStringRandom(settings, palette);
            effect->SetPalette(palette);
            effect->SetSettings(settings);
            effect->SetEffectName(effectName);
            effect->SetEffectIndex(effectManager.GetEffectIndex(effectName));
            resetStrings = true;
        }
        SetEffectControls(effect->GetParentEffectLayer()->GetParentElement()->GetName(),
                          effect->GetEffectName(), effect->GetSettings(), effect->GetPaletteMap(),
                          !event.GetInt());
        selectedEffectString = GetEffectTextFromWindows(selectedEffectPalette);
        selectedEffect = effect;
        if (effect->GetPaletteMap().empty() || resetStrings) {
            effect->SetPalette(selectedEffectPalette);
            effect->SetSettings(selectedEffectString);
            RenderEffectForModel(effect->GetParentEffectLayer()->GetParentElement()->GetName(),
                                 effect->GetStartTimeMS(),
                                 effect->GetEndTimeMS());
        }

        if (playType == PLAY_TYPE_MODEL_PAUSED) {
            StopSequence();
        }

        if (playType != PLAY_TYPE_MODEL) {
            playType = PLAY_TYPE_EFFECT;
            playStartTime = effect->GetStartTimeMS();
            playEndTime = effect->GetEndTimeMS();
            playStartMS = -1;
            playModel = GetModel(effect->GetParentEffectLayer()->GetParentElement()->GetName());
			SetAudioControls();
        }
    }
    RenderableEffect *eff = effectManager[EffectsPanel1->EffectChoicebook->GetSelection()];
    effectsPnl->SetDragIconBuffer(eff->GetEffectIcon(16));
    effectsPnl->BitmapButtonSelectedEffect->SetEffect(eff, mIconSize);
    if( effect != nullptr ) {
        UpdateEffectAssistWindow(effect, eff);
    }
    mainSequencer->PanelEffectGrid->SetFocus();
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
    selectedEffect = NULL;
    Effect* last_effect_created = NULL;

    mSequenceElements.get_undo_mgr().CreateUndoStep();
    for(int i=0;i<mSequenceElements.GetSelectedRangeCount();i++)
    {
        EffectLayer* el = mSequenceElements.GetSelectedRange(i)->Layer;
        if (el->GetParentElement()->GetType() != "model") {
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

        mSequenceElements.get_undo_mgr().CaptureAddedEffect( el->GetParentElement()->GetName(), el->GetIndex(), effect->GetID() );

        mainSequencer->PanelEffectGrid->ProcessDroppedEffect(effect);

        if (playType == PLAY_TYPE_MODEL_PAUSED) {
            StopSequence();
			SetAudioControls();
		}

        if (playType != PLAY_TYPE_MODEL) {
            playType = PLAY_TYPE_EFFECT;
            playStartTime = mSequenceElements.GetSelectedRange(i)->StartTime;
            playEndTime = mSequenceElements.GetSelectedRange(i)->EndTime;
            playStartMS = -1;
            RenderEffectForModel(el->GetParentElement()->GetName(),playStartTime,playEndTime);

            playModel = GetModel(el->GetParentElement()->GetName());

			SetAudioControls();
        }
    }

    if (playType != PLAY_TYPE_MODEL && last_effect_created != NULL)
    {
        SetEffectControls(last_effect_created->GetParentEffectLayer()->GetParentElement()->GetName(),
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

void xLightsFrame::PlaySequence()
{
	if (CurrentSeqXmlFile != NULL)
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
				if (CurrentSeqXmlFile->GetMedia() != NULL)
				{
					CurrentSeqXmlFile->GetMedia()->Seek(playStartTime);
				}
			}
			if (playEndTime == -1 || playEndTime > CurrentSeqXmlFile->GetSequenceDurationMS()) {
				playEndTime = CurrentSeqXmlFile->GetSequenceDurationMS();
			}
			mainSequencer->PanelTimeLine->PlayStarted();
			if (CurrentSeqXmlFile->GetSequenceType() == "Media") {
				if (CurrentSeqXmlFile->GetMedia() != NULL)
				{
					CurrentSeqXmlFile->GetMedia()->Play();
				}
			}
		}
	}
	SetAudioControls();
}

void xLightsFrame::PlaySequence(wxCommandEvent& event)
{
	mLoopAudio = false;
	PlaySequence();
}

void xLightsFrame::PauseSequence(wxCommandEvent& event)
{
    if( CurrentSeqXmlFile->GetSequenceType() == "Media" )
    {
		if (CurrentSeqXmlFile->GetMedia() != NULL)
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
}

void xLightsFrame::SetAudioControls()
{
	if (CurrentSeqXmlFile == NULL || mRendering)
	{
		EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, false);
		EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, false);
		EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, false);
		EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, false);
		EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, false);
		EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, false);
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
		}
		else if (playType == PLAY_TYPE_EFFECT || playType == PLAY_TYPE_MODEL)
		{
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, false);
		}
		else
		{
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, true);
		}
	}
	else
	{
		if (CurrentSeqXmlFile->GetMedia() == NULL)
		{
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, false);
		}
		else if (playType == PLAY_TYPE_STOPPED)
		{
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, true);
		}
		else if (playType == PLAY_TYPE_EFFECT || playType == PLAY_TYPE_MODEL)
		{
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, false);
		}
		else if (playType == PLAY_TYPE_EFFECT_PAUSED || playType == PLAY_TYPE_MODEL_PAUSED)
		{
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, true);
		}
		else
		{
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, false);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, true);
			EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, true);
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

void xLightsFrame::StopSequence()
{
    _fps = -1;
	mLoopAudio = false;
    if( playType == PLAY_TYPE_MODEL || playType == PLAY_TYPE_MODEL_PAUSED )
    {
        if( CurrentSeqXmlFile->GetSequenceType() == "Media" ) {
			if (CurrentSeqXmlFile->GetMedia() != NULL)
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
    if( CheckBoxLightOutput->IsChecked() && xout ) {
        xout->alloff();
    }
	SetAudioControls();
}

void xLightsFrame::StopSequence(wxCommandEvent& event)
{
    StopSequence();
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

void xLightsFrame::SequenceReplaySection(wxCommandEvent& event)
{
    //FIXME implement this:  use as a loop flag?
	mLoopAudio = true;
	PlaySequence();
}

void xLightsFrame::PlayModelEffect(wxCommandEvent& event)
{
    if( playType != PLAY_TYPE_MODEL && playType != PLAY_TYPE_MODEL_PAUSED)
    {
        EventPlayEffectArgs* args = (EventPlayEffectArgs*)event.GetClientData();
        playModel = GetModel(args->element->GetName());
        if (playModel != nullptr) {
            playType = PLAY_TYPE_EFFECT;
            playStartTime = (int)(args->effect->GetStartTimeMS());
            playEndTime = (int)(args->effect->GetEndTimeMS());
            if(args->renderEffect)
            {
                RenderEffectForModel(args->element->GetName(),playStartTime,playEndTime);
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
    for(size_t i=0;i<mSequenceElements.GetVisibleRowInformationSize();i++)
    {
        Element* element = mSequenceElements.GetVisibleRowInformation(i)->element;
        if(element->GetType() == "model" || element->GetType() == "timing")
        {
            EffectLayer* el = mSequenceElements.GetVisibleEffectLayer(i);
            for(int j=0;j< el->GetEffectCount();j++)
            {
                if(el->GetEffect(j)->GetSelected() != EFFECT_NOT_SELECTED)
                {
                    el->GetEffect(j)->SetPalette(palette);

                    if(playType != PLAY_TYPE_MODEL && playType != PLAY_TYPE_MODEL_PAUSED)
                    {
                        playType = PLAY_TYPE_EFFECT;
                        playStartTime = (int)(el->GetEffect(j)->GetStartTimeMS());
                        playEndTime = (int)(el->GetEffect(j)->GetEndTimeMS());
                        playStartMS = -1;
                        RenderEffectForModel(element->GetName(),playStartTime,playEndTime);
                    }
                }
            }
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

    for(size_t i=0;i<mSequenceElements.GetVisibleRowInformationSize();i++)
    {
        Element* element = mSequenceElements.GetVisibleRowInformation(i)->element;
        if(element->GetType() == "model" || element->GetType() == "timing")
        {
            EffectLayer* el = mSequenceElements.GetVisibleEffectLayer(i);
            for(int j=0;j< el->GetEffectCount();j++)
            {
                if(el->GetEffect(j)->GetSelected() != EFFECT_NOT_SELECTED)
                {
                    el->GetEffect(j)->SetSettings(effectText);
                    el->GetEffect(j)->SetEffectIndex(effectIndex);
                    el->GetEffect(j)->SetEffectName(effectName);
                    el->GetEffect(j)->SetPalette(palette);

                    if(playType != PLAY_TYPE_MODEL && playType != PLAY_TYPE_MODEL_PAUSED)
                    {
                        playType = PLAY_TYPE_EFFECT;
                        playStartTime = (int)(el->GetEffect(j)->GetStartTimeMS());
                        playEndTime = (int)(el->GetEffect(j)->GetEndTimeMS());
                        playStartMS = -1;
                        RenderEffectForModel(element->GetName(),playStartTime,playEndTime);
                    }
                }
            }
        }
    }
    mainSequencer->PanelEffectGrid->ForceRefresh();
}


void xLightsFrame::OnEffectSettingsTimerTrigger(wxTimerEvent& event)
{
    if (CurrentSeqXmlFile == NULL) {
        return;
    }
    if (Notebook1->GetSelection() != NEWSEQUENCER) {
        return;
    }

    if (selectedEffect != NULL && timingPanel->BitmapButton_CheckBox_LayerMorph->IsEnabled()) {
        std::string palette;
        std::string effectText = GetEffectTextFromWindows(palette);
        if (effectText != selectedEffectString
            || palette != selectedEffectPalette) {

            int effectIndex = EffectsPanel1->EffectChoicebook->GetSelection();
            wxString name = EffectsPanel1->EffectChoicebook->GetPageText(effectIndex);
            if (name !=  selectedEffect->GetEffectName()) {
                selectedEffect->SetEffectName(name.ToStdString());
                selectedEffect->SetEffectIndex(EffectsPanel1->EffectChoicebook->GetSelection());
            }

            EffectLayer* el = selectedEffect->GetParentEffectLayer();
            Element *elem = el->GetParentElement();

            //check for undo capture
            if( selectedEffectName != selectedEffect->GetEffectName() )
            {
                mSequenceElements.get_undo_mgr().CreateUndoStep();
                mSequenceElements.get_undo_mgr().CaptureModifiedEffect( elem->GetName(), el->GetIndex(), selectedEffect->GetID(), selectedEffectString, selectedEffectPalette );
            }

            selectedEffect->SetSettings(effectText);
            selectedEffect->SetPalette(palette);

            selectedEffectName = selectedEffect->GetEffectName();
            selectedEffectString = effectText;
            selectedEffectPalette = palette;

            playStartTime = selectedEffect->GetStartTimeMS();
            playEndTime = selectedEffect->GetEndTimeMS();
            playStartMS = -1;

            // Update if effect has been modified
            if( m_mgr->GetPane("EffectAssist").IsShown() )
            {
                sEffectAssist->ForceRefresh();
            }

            RenderEffectForModel(elem->GetName(),playStartTime,playEndTime);
            mainSequencer->PanelEffectGrid->ForceRefresh();
            return;
        }
    }
}

void xLightsFrame::TimerRgbSeq(long msec)
{
    //check if there are models that depend on timing tracks or similar that need to be rendered
    std::vector<Element *> elsToRender;
    if (mSequenceElements.GetElementsToRender(elsToRender)) {
        for (std::vector<Element *>::iterator it = elsToRender.begin(); it != elsToRender.end(); it++) {
            int ss, es;
            (*it)->GetDirtyRange(ss, es);
            RenderEffectForModel((*it)->GetName(), ss, es);
        }
    }

    // Update play status so sequencer grid can allow dropping timings during playback
    mainSequencer->SetPlayStatus(playType);

    // return if play is stopped
    if (playType == PLAY_TYPE_STOPPED || CurrentSeqXmlFile == NULL) {
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

        int current_play_time = 0;
		if (CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->GetMedia() != NULL && CurrentSeqXmlFile->GetMedia()->GetPlayingState() == MEDIAPLAYINGSTATE::PLAYING)
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
				PlaySequence();
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
        playModel->DisplayEffectOnWindow(sPreview1, mPointSize);
    }
    sPreview2->Render(&SeqData[frame][0]);
    for (auto it = PreviewWindows.begin(); it != PreviewWindows.end(); it++) {
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
    if (setDefaults) {
        if (modelName == "") {
            EffectsPanel1->SetDefaultEffectValues(nullptr, CurrentSeqXmlFile->GetMedia(), effectName);
            timingPanel->SetDefaultControls(nullptr);
            bufferPanel->SetDefaultControls(nullptr);
            colorPanel->SetDefaultSettings();
        } else {
            Model *model = GetModel(modelName);
            EffectsPanel1->SetDefaultEffectValues(model, CurrentSeqXmlFile->GetMedia(), effectName);
            timingPanel->SetDefaultControls(model);
            bufferPanel->SetDefaultControls(model);
            colorPanel->SetDefaultSettings();
        }
    }
    SetEffectControls(settings);
    SetEffectControls(palette);
}

void xLightsFrame::ApplySetting(wxString name, wxString value)
{
	long TempLong;
	wxWindow *CtrlWin, *ContextWin;

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
	else
	{
		return;
		//efPanel = NULL;
		//ContextWin=SeqPanelLeft;
	}
	name = "ID_" + name.Mid(2);

	CtrlWin = wxWindow::FindWindowByName(name, ContextWin);
	if (CtrlWin)
	{
		if (name.StartsWith("ID_SLIDER"))
		{
			wxSlider* ctrl = (wxSlider*)CtrlWin;
			if (value.ToLong(&TempLong)) {
				ctrl->SetValue(TempLong);

				wxScrollEvent event(wxEVT_SLIDER, ctrl->GetId());
				event.SetEventObject(ctrl);
				event.SetInt(TempLong);
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
			colorPanel->SetButtonColor((wxBitmapButton*)CtrlWin, value.ToStdString());
		}
		else if (name.StartsWith("ID_CHECKBOX"))
		{
			wxCheckBox* ctrl = (wxCheckBox*)CtrlWin;
			if (value.ToLong(&TempLong)) {
				ctrl->SetValue(TempLong != 0);
				wxCommandEvent evt(wxEVT_COMMAND_CHECKBOX_CLICKED, ctrl->GetId());
				evt.SetEventObject(ctrl);
				evt.SetInt(TempLong != 0);
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
		}
	}
}

void xLightsFrame::SetEffectControls(const SettingsMap &settings) {
    int cnt=0;

	// Apply those settings without APPLYLAST in their name first
    for (std::map<std::string,std::string>::const_iterator it=settings.begin(); it!=settings.end(); ++it) {
		if (it->first.find("APPLYLAST") == std::string::npos)
		{
			ApplySetting(wxString(it->first.c_str()), wxString(it->second.c_str()));
			cnt++;
		}
    }

	// Now Apply those settings with APPLYLAST in their name ... last
	for (std::map<std::string, std::string>::const_iterator it = settings.begin(); it != settings.end(); ++it) {
		if (it->first.find("APPLYLAST") != std::string::npos)
		{
			ApplySetting(wxString(it->first.c_str()), wxString(it->second.c_str()));
			cnt++;
		}
	}

    MixTypeChanged=true;
    FadesChanged=true;
}

std::string xLightsFrame::GetEffectTextFromWindows(std::string &palette)
{
    RenderableEffect *eff = effectManager[EffectsPanel1->EffectChoicebook->GetSelection()];
    std::string effectText = eff->GetEffectString();
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
    ForceSequencerRefresh();
}
void xLightsFrame::ForceSequencerRefresh()
{
    mSequenceElements.PopulateRowInformation();
    ResizeMainSequencer();
}

void xLightsFrame::LoadPerspective(wxXmlNode *perspective) {
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

        sPreview1->Refresh(false);
        sPreview2->Refresh(false);
        m_mgr->Update();

        perspective->DeleteAttribute("settings");
        perspective->DeleteAttribute("version");
        perspective->AddAttribute("version", "2.0");
        perspective->AddAttribute("settings", m_mgr->SavePerspective());
    } else {
        sPreview1->Refresh(false);
        sPreview2->Refresh(false);
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

    for (int i=0;i<10;i++) {
        if (perspectives[i].p == perspective) {
            MenuItemPerspectives->Check(perspectives[i].id,true);
        }
    }

}

void xLightsFrame::LoadPerspective(wxCommandEvent& event)
{
    wxXmlNode* perspective = (wxXmlNode*)(event.GetClientData());
    mCurrentPerpective = perspective;
    LoadPerspective(mCurrentPerpective);
}

void xLightsFrame::OnMenuItemViewSavePerspectiveSelected(wxCommandEvent& event)
{
    if(mCurrentPerpective != NULL)
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
        for(wxXmlNode* p=PerspectivesNode->GetChildren(); p!=NULL; p=p->GetNext() )
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
        LoadPerspective(mCurrentPerpective);
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

Element* xLightsFrame::AddTimingElement(const std::string& name)
{
    // Deactivate active timing mark so new one is selected;
    mSequenceElements.DeactivateAllTimingElements();
    int timingCount = mSequenceElements.GetNumberOfTimingElements();
    std::string type = "timing";
    Element* e = mSequenceElements.AddElement(timingCount,name,type,true,false,true,false);
    e->AddEffectLayer();
    mSequenceElements.AddTimingToAllViews(name);
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
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(this, eventRowHeaderChanged);
}

void xLightsFrame::ExecuteImportTimingElement(wxCommandEvent &command) {
    ImportTimingElement();
}

void xLightsFrame::ImportTimingElement()
{
    wxFileDialog* OpenDialog = new wxFileDialog( this, "Choose Timing file(s)", wxEmptyString, wxEmptyString, "Timing files (*.xtiming)|*.xtiming|Papagayo files (*.pgo)|*.pgo|Text files (*.txt)|*.txt|LOR (*.lms)|*.lms|LOR (*.las)|*.las|LSP (*.msq)|*.msq", wxFD_OPEN | wxFD_MULTIPLE, wxDefaultPosition);
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


void xLightsFrame::ConvertDataRowToEffects(EffectLayer *layer, xlColorVector &colors, int frameTime) {
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
                    layer->AddEffect(0, "On", settings, palette, stime, etime, false, false);
                } else {
                    std::string palette = "C_BUTTON_Palette1=" + (std::string)colors[x] + ",C_CHECKBOX_Palette1=1,"
                        "C_BUTTON_Palette2=" + (std::string)colors[x + len - 1] + ",C_CHECKBOX_Palette2=1";
                    layer->AddEffect(0, "Color Wash", "", palette, stime, etime, false, false);
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
                    layer->AddEffect(0, "On", "", palette, startTime, time, false, false);
                }
            }
            startTime = time;
            lastColor = colors[x];
        }
    }
}

void xLightsFrame::ConvertDataRowToEffects(wxCommandEvent &event) {
    Element *el = (Element*)event.GetClientData();
    int strand = event.GetInt() >> 16;
    int node = event.GetInt() & 0xFFFF;
    EffectLayer *layer = el->GetStrandLayer(strand)->GetNodeLayer(node);

    xlColorVector colors;
    PixelBufferClass ncls(this, true);
    Model *model = GetModel(el->GetName());
    for (size_t f = 0; f < SeqData.NumFrames(); f++) {
        model->SetNodeChannelValues(0, &SeqData[f][model->NodeStartChannel(0)]);
        xlColor c = model->GetNodeColor(0);
        colors.push_back(c);
    }
    ConvertDataRowToEffects(layer, colors, SeqData.FrameTime());
}

wxXmlNode* xLightsFrame::CreateEffectNode(wxString& name)
{
    wxXmlNode* NewXml=new wxXmlNode(wxXML_ELEMENT_NODE, "effect");
    NewXml->AddAttribute("name", name);
    wxString copy_data;
    mainSequencer->GetSelectedEffectsData(copy_data);
    NewXml->AddAttribute("settings", copy_data);
    NewXml->AddAttribute("version", XLIGHTS_RGBEFFECTS_VERSION);
    return NewXml;
}

void xLightsFrame::UpdateEffectNode(wxXmlNode* node)
{
    wxString copy_data;
    mainSequencer->GetSelectedEffectsData(copy_data);
    node->AddAttribute("settings", copy_data);
}

void xLightsFrame::ApplyEffectsPreset(wxString& data)
{
    mainSequencer->PanelEffectGrid->Paste(data);
}
void xLightsFrame::PromoteEffects(wxCommandEvent &command) {
    Element *el = (Element*)command.GetClientData();
    PromoteEffects(el);
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
void xLightsFrame::PromoteEffects(Element *element) {
    //first promote from nodes to strands
    for (int x = 0;  x < element->getStrandLayerCount(); x++) {
        StrandLayer *layer = element->GetStrandLayer(x);
        EffectLayer *target = layer;
        if (element->getStrandLayerCount() <= 1) {
            if (element->GetEffectLayer(0)->GetEffectCount() != 0) {
                element->InsertEffectLayer(0);
            }
            target = element->GetEffectLayer(0);
        }
        if (layer->GetNodeLayerCount() > 0) {
            NodeLayer *base = layer->GetNodeLayer(0);
            for (int e = base->GetEffectCount() - 1; e >= 0; e--) {
                Effect *eff = base->GetEffect(e);
                const std::string &name = eff->GetEffectName();

                if (layer->HasEffectsInTimeRange(eff->GetStartTimeMS(), eff->GetEndTimeMS())) {
                    //cannot promote, already an effect there
                    continue;
                }
                if (name == "On" || name == "Color Wash") {
                    const std::string pal = eff->GetPaletteAsString();
                    const std::string set = eff->GetSettingsAsString();
                    int mp = (eff->GetStartTimeMS() + eff->GetEndTimeMS()) / 2;
                    bool collapse = true;

                    for (int n = 1; n < layer->GetNodeLayerCount() && collapse; n++) {
                        NodeLayer *node = layer->GetNodeLayer(n);
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
                        for (int n = 0; n < layer->GetNodeLayerCount() && collapse; n++) {
                            NodeLayer *node = layer->GetNodeLayer(n);
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
    if (element->getStrandLayerCount() <= 1) {
        return;
    }
    //OK, we're now promoted to strand level effects, try and promote to Model level
    StrandLayer *base = element->GetStrandLayer(0);
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

            for (int n = 1; n < element->getStrandLayerCount() && collapse; n++) {
                StrandLayer *node = element->GetStrandLayer(n);
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
                for (int n = 0; n < element->getStrandLayerCount() && collapse; n++) {
                    StrandLayer *node = element->GetStrandLayer(n);
                    int nodeIndex = 0;
                    if (node->HitTestEffectByTime(mp, nodeIndex)) {
                        node->DeleteEffectByIndex(nodeIndex);
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

