/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <map>

#include <wx/utils.h>
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include <wx/filepicker.h>
#include <wx/fontpicker.h>
#include <wx/config.h>

#include "../xLightsMain.h"
#include "SequenceElements.h"
#include "../TopEffectsPanel.h"
#include "../EffectIconPanel.h"
#include "../ValueCurvesPanel.h"
#include "../ColoursPanel.h"
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
#include "../SeqElementMismatchDialog.h"
#include "../SequenceVideoPanel.h"
#include "../RenderCommandEvent.h"
#include "../xLightsVersion.h"
#include "../HousePreviewPanel.h"
#include "../UtilFunctions.h"
#include "../JukeboxPanel.h"
#include "../FindDataPanel.h"
#include "../EffectsPanel.h"
#include "../EffectAssist.h"
#include "../ColorPanel.h"
#include "../TimingPanel.h"
#include "../ModelPreview.h"
#include "MainSequencer.h"
#include "../PerspectivesPanel.h"
#include "../SelectPanel.h"
#include "../SearchPanel.h"
#include "../LayoutGroup.h"
#include "../ViewpointMgr.h"
#include "../LayoutPanel.h"
#include "../TraceLog.h"
#include "../effects/EffectPanelUtils.h"
#include "../UtilFunctions.h"
#include "../ExternalHooks.h"
#include "../models/ModelGroup.h"

#include <log4cpp/Category.hh>

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
    mainSequencer->PanelEffectGrid->SetRenderDataSources(this, &_seqData);
    mainSequencer->SetSequenceElements(&_sequenceElements);

    logger_base.debug("                Set timeline.");
    mainSequencer->PanelWaveForm->SetTimeline(mainSequencer->PanelTimeLine);
    _sequenceElements.SetTimeLine(mainSequencer->PanelTimeLine);

    logger_base.debug("                Set sequence elements.");
    mainSequencer->PanelRowHeadings->SetSequenceElements(&_sequenceElements);
    _sequenceElements.SetMaxRowsDisplayed(mainSequencer->PanelRowHeadings->GetMaxRows());

    logger_base.debug("                Set dock size constraints.");
    m_mgr->SetDockSizeConstraint(0.25, 0.15);

    logger_base.debug("        Model preview.");
    _modelPreviewPanel = new ModelPreview(PanelSequencer, this);
    m_mgr->AddPane(_modelPreviewPanel, wxAuiPaneInfo().Name(wxT("ModelPreview")).Caption(wxT("Model Preview")).
                   Left().Layer(1).PaneBorder(true).BestSize(250,250).MaximizeButton(true).Dockable(IsDockable("MP")));

    logger_base.debug("        House preview.");
    _housePreviewPanel = new HousePreviewPanel(PanelSequencer, this, _playControlsOnPreview, PreviewModels, LayoutGroups, false, 0, true);
    m_mgr->AddPane(_housePreviewPanel, wxAuiPaneInfo().Name(wxT("HousePreview")).Caption(wxT("House Preview")).
        Left().Layer(1).BestSize(250, 250).MaximizeButton(true).Dockable(IsDockable("HP")));

    logger_base.debug("        Effects.");
    effectsPnl = new TopEffectsPanel(PanelSequencer);
    effectsPnl->BitmapButtonSelectedEffect->SetEffect(effectManager[0], mIconSize);

    logger_base.debug("        Effect settings.");
    // This step takes about 5 seconds to create all the effects panels
    EffectsPanel1 = new EffectsPanel(effectsPnl, &effectManager, &EffectSettingsTimer);
    EffectsPanel1->SetSequenceElements(&_sequenceElements);
    effectsPnl->EffectSizer->Add(EffectsPanel1, wxEXPAND);
    effectsPnl->MainSizer->Fit(effectsPnl);
    effectsPnl->MainSizer->SetSizeHints(effectsPnl);

    logger_base.debug("        Effect assist.");
    sEffectAssist = new EffectAssist(PanelSequencer);
    m_mgr->AddPane(sEffectAssist,wxAuiPaneInfo().Name(wxT("EffectAssist")).Caption(wxT("Effect Assist")).
                   Left().Layer(1).BestSize(250,250));
    sEffectAssist->Layout();
    sEffectAssist->Hide();

    logger_base.debug("        Color.");
    colorPanel = new ColorPanel(PanelSequencer);
    colorPanel->AddChangeListeners(&EffectSettingsTimer);

    logger_base.debug("        Timing.");
    timingPanel = new TimingPanel(PanelSequencer);
    timingPanel->AddChangeListeners(&EffectSettingsTimer);

    logger_base.debug("        Buffer.");
    bufferPanel = new BufferPanel(PanelSequencer);
    bufferPanel->AddChangeListeners(&EffectSettingsTimer);

    logger_base.debug("        Perspective.");
    perspectivePanel = new PerspectivesPanel(PanelSequencer);

    logger_base.debug("        Effect Icons.");
    effectPalettePanel = new EffectIconPanel(effectManager, PanelSequencer);

    logger_base.debug("        Value Curves.");
    _valueCurvesPanel = new ValueCurvesPanel(PanelSequencer);

    logger_base.debug("        Colours.");
    _coloursPanel = new ColoursPanel(PanelSequencer);

    logger_base.debug("        Jukebox.");
    jukeboxPanel = new JukeboxPanel(PanelSequencer);
    
    logger_base.debug("        Find Data.");
    _findDataPanel = new FindDataPanel(PanelSequencer);

    // DisplayElements Panel
    logger_base.debug("        Display Elements.");
    displayElementsPanel = new ViewsModelsPanel(this, PanelSequencer);
    displayElementsPanel->SetViewChoice(mainSequencer->ViewChoice);
    displayElementsPanel->Fit();

    logger_base.debug("CreateSequencer: Hooking up the panes.");
    m_mgr->AddPane(displayElementsPanel,wxAuiPaneInfo().Name(wxT("DisplayElements")).Caption(wxT("Display Elements"))
                   .Float().MaximizeButton(true));
    // Hide the panel on start.
    wxAuiPaneInfo & info = m_mgr->GetPane("DisplayElements");
    info.BestSize(wxSize(750, 1050));
    int w, h;
    displayElementsPanel->GetSize(&w, &h);
    info.FloatingSize(std::max(750, w), std::max(1050, h));
    info.Hide();

    m_mgr->AddPane(perspectivePanel,wxAuiPaneInfo().Name(wxT("Perspectives")).Caption(wxT("Perspectives")).Left().Layer(1).Hide());
    m_mgr->AddPane(effectsPnl,wxAuiPaneInfo().Name(wxT("Effect")).Caption(wxT("Effect Settings")).
                   Left().Layer(0).Row(1));

    logger_base.debug("CreateSequencer: Adding Select Effects Panel.");
    _selectPanel = new SelectPanel(&_sequenceElements, mainSequencer, PanelSequencer);
    m_mgr->AddPane(_selectPanel, wxAuiPaneInfo().Name(wxT("SelectEffect")).Caption(wxT("Select Effects")).
        Left().Layer(1).Hide());

    logger_base.debug("CreateSequencer: Adding Search Panel.");
    _searchPanel = new SearchPanel(&_sequenceElements, mainSequencer, PanelSequencer);
    m_mgr->AddPane(_searchPanel, wxAuiPaneInfo().Name(wxT("SearchPanel")).Caption(wxT("Search Panel")).Left().Layer(1).Hide());

    m_mgr->AddPane(effectPalettePanel,wxAuiPaneInfo().Name(wxT("EffectDropper")).Caption(wxT("Effects")).Top().Layer(0).Hide());
    m_mgr->AddPane(_valueCurvesPanel, wxAuiPaneInfo().Name(wxT("ValueCurveDropper")).Caption(wxT("Value Curves")).Top().Layer(0).Hide());
    m_mgr->AddPane(_coloursPanel, wxAuiPaneInfo().Name(wxT("ColourDropper")).Caption(wxT("Colours")).Top().Layer(0).Hide());
    m_mgr->AddPane(jukeboxPanel,wxAuiPaneInfo().Name(wxT("Jukebox")).Caption(wxT("Jukebox")).Top().Layer(0).Hide());
    m_mgr->AddPane(_findDataPanel, wxAuiPaneInfo().Name(wxT("FindData")).Caption(wxT("Find Data")).Top().Layer(0).Hide());
    m_mgr->AddPane(colorPanel, wxAuiPaneInfo().Name(wxT("Color")).Caption(wxT("Color")).Top().Layer(0));
    m_mgr->AddPane(timingPanel,wxAuiPaneInfo().Name(wxT("LayerTiming")).Caption(wxT("Layer Blending")).Top().Layer(0));
    m_mgr->AddPane(bufferPanel,wxAuiPaneInfo().Name(wxT("LayerSettings")).Caption(wxT("Layer Settings")).Top().Layer(0));

    logger_base.debug( "        Sequence Video." );
    sequenceVideoPanel = new SequenceVideoPanel( this );
    m_mgr->AddPane(sequenceVideoPanel,wxAuiPaneInfo().Name(wxT("SequenceVideo")).Caption(wxT("Sequence Video")).Float().Hide() );

    m_mgr->AddPane(mainSequencer,wxAuiPaneInfo().Name(_T("Main Sequencer")).CenterPane().Caption(_("Main Sequencer")));

    logger_base.debug("CreateSequencer: Updating the layout.");
    m_mgr->Update();
    logger_base.debug("CreateSequencer: Resizing everything.");
    mainSequencer->Layout();
    logger_base.debug("CreateSequencer: Done.");

#ifdef __XLIGHTS_HAS_TOUCHBARS__
    mainSequencer->SetupTouchBar(effectManager, colorPanel->SetupTouchBar(mainSequencer->touchBarSupport));
#endif
}

void xLightsFrame::ResetWindowsToDefaultPositions(wxCommandEvent& event)
{
    m_mgr->GetPane("ModelPreview").Caption("Model Preview").Dock().Left().Layer(1).Show();
    m_mgr->GetPane("HousePreview").Caption("House Preview").Dock().Left().Layer(1).Show();
    m_mgr->GetPane("EffectAssist").Caption("Effect Assist").Dock().Left().Layer(1).Hide();

    m_mgr->GetPane("DisplayElements").Caption("Display Elements").Float().Hide();
    m_mgr->GetPane("Perspectives").Caption("Perspectives").Dock().Left().Layer(1).Hide();
    m_mgr->GetPane("Effect").Caption("Effect").Dock().Left().Layer(0).Show().Row(1);
    m_mgr->GetPane("SelectEffect").Caption("Select Effects").Dock().Left().Layer(1).Hide();

    m_mgr->GetPane("EffectDropper").Caption("Effects").Dock().Top().Layer(0).Hide();
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
    SaveWindowPosition("xLightsTipOfTheDay", nullptr);
    SaveWindowPosition("xLightsImportDialogPosition", nullptr);
    SaveWindowPosition("xLightsNodeSelectDialogPosition", nullptr);
    SaveWindowPosition("ControllerModelDialogPosition", nullptr);
    SaveWindowPosition("xLightsTestDialogPosition", nullptr);

    UpdateViewMenu();
}

void xLightsFrame::InitSequencer()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // check if sequence data is the right size
    if (CurrentSeqXmlFile != nullptr && CurrentSeqXmlFile->GetSequenceLoaded()) {
        if (_seqData.NumChannels() != roundTo4(GetMaxNumChannels())) {
            logger_base.info("Number of channels has changed ... reallocating sequence data memory.");

            AbortRender();

            wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
            int ms = wxAtoi(mss);

            _seqData.init(GetMaxNumChannels(), CurrentSeqXmlFile->GetSequenceDurationMS() / ms, ms);
            _sequenceElements.IncrementChangeCount(nullptr);

            SetStatusTextColor("Render buffer recreated. A render all is required.", *wxRED);
        }
    }

    if (EffectsPanel1 == nullptr || timingPanel == nullptr) {
        return;
    }

    if (mSequencerInitialize) {
        return;
    }

    // if we have a saved perspective on this machine then make that the current one
    if (_autoSavePerspecive) {
        wxConfigBase* config = wxConfigBase::Get();
        wxString machinePerspective = config->Read("xLightsMachinePerspective", "");
        if (machinePerspective != "") {
            if (!m_mgr->LoadPerspective(machinePerspective, true)) {
                logger_base.debug("Failed to load AutoSave perspective.");
            }
            else {
                logger_base.debug("Loaded AutoSave perspective.");
                ShowHideAllSequencerWindows(true);
                _modelPreviewPanel->Refresh(false);
                _housePreviewPanel->Refresh(false);
                m_mgr->Update();
            }
            LogPerspective(machinePerspective);
        } else {
            if (mCurrentPerpective != nullptr) {
                DoLoadPerspective(mCurrentPerpective);
            }
        }
    }
    else if (mCurrentPerpective != nullptr) {
        DoLoadPerspective(mCurrentPerpective);
    }

    mSequencerInitialize = true;
    _housePreviewPanel->GetModelPreview()->InitializePreview(mBackgroundImage, mBackgroundBrightness, mBackgroundAlpha, GetDisplay2DCenter0());
    _housePreviewPanel->GetModelPreview()->SetScaleBackgroundImage(mScaleBackgroundImage);
}

Model *xLightsFrame::GetModel(const std::string& name) const
{
    return AllModels[name];
}

bool xLightsFrame::InitPixelBuffer(const std::string &modelName, PixelBufferClass &buffer, int layerCount, bool zeroBased) {

    if (modelName == PRESET_MODEL_NAME && _presetModel != nullptr) {
        buffer.InitBuffer(*_presetModel, layerCount, 50, zeroBased);
    }
    else {
        Model* model = GetModel(modelName);
        if (model == nullptr || model->GetModelXml() == nullptr) {
            return false;
        }
        buffer.InitBuffer(*model, layerCount, _seqData.FrameTime(), zeroBased);
    }
    return true;
}

void xLightsFrame::CheckForAndCreateDefaultPerpective()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxXmlNode* prospectives = PerspectivesNode->GetChildren();
    mCurrentPerpective = nullptr;
    if (prospectives == nullptr) {
        if (PerspectivesNode->HasAttribute("current")) {
            PerspectivesNode->DeleteAttribute("current");
        }
        PerspectivesNode->AddAttribute("current", "Default Perspective");
        wxXmlNode* p = new wxXmlNode(wxXML_ELEMENT_NODE, "perspective");
        p->AddAttribute("name", "Default Perspective");
        wxString perspective = m_mgr->SavePerspective();
        p->AddAttribute("settings", perspective);
        logger_base.debug("Saved perspective.");
        LogPerspective(perspective);

        p->AddAttribute("version", "2.0");
        PerspectivesNode->AddChild(p);
        mCurrentPerpective = p;
        UnsavedRgbEffectsChanges = true;
        UpdateLayoutSave();
        UpdateControllerSave();
    } else {
        wxString currentName = PerspectivesNode->GetAttribute("current");
        for (wxXmlNode* p = PerspectivesNode->GetChildren(); p != nullptr; p = p->GetNext()) {
            if (p->GetName() == "perspective") {
                wxString name = p->GetAttribute("name");
                if (!name.IsEmpty() && currentName == name) {
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
                          std::vector<Element*> &ignore,
                          bool mapall)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxArrayString choices;
    choices.push_back("Rename the model in the sequence");
    choices.push_back("Delete the model in the sequence");
    choices.push_back("Map the effects to different models");
    choices.push_back("Ignore (Handle Later) - Effects will not render");

    if (mapall) {
        toMap.push_back(element);
    }
    else {
        bool ok = false;
        wxSingleChoiceDialog dlg(frame, msg,
            "Warning", choices);
        while (!ok) {
            ok = true;
            if (dlg.ShowModal() == wxID_OK) {
                switch (dlg.GetSelection()) {
                case 0: {
                    // Rename Model
                    wxSingleChoiceDialog namedlg(frame, "Choose the model to use instead:",
                        "Select Model", ToArrayString(ModelNames));
                    if (namedlg.ShowModal() == wxID_OK) {
                        std::string newName = namedlg.GetStringSelection().ToStdString();

                        logger_base.debug("Sequence Element Mismatch 2: rename '%s' to '%s'", (const char*)element->GetFullName().c_str(), (const char*)newName.c_str());

                        // remove the existing element before we rename
                        if (dynamic_cast<SubModelElement*>(element) != nullptr) {
                            SubModelElement* sme = dynamic_cast<SubModelElement*>(element);
                            sme->GetModelElement()->RemoveSubModel(newName);
                        }
                        else {
                            frame->GetSequenceElements().DeleteElement(newName);
                        }

                        element->SetName(newName);
                        Remove(AllNames, newName);
                        Remove(ModelNames, newName);
                    }
                    else {
                        ok = false;
                    }
                }
                      break;
                case 1:
                    // Delete the model
                    if (dynamic_cast<SubModelElement*>(element) != nullptr) {
                        SubModelElement* sme = dynamic_cast<SubModelElement*>(element);
                        logger_base.debug("Sequence Element Mismatch 2: delete '%s'", (const char*)sme->GetFullName().c_str());
                        sme->GetModelElement()->RemoveSubModel(sme->GetName());
                    }
                    else {
                        logger_base.debug("Sequence Element Mismatch 2: delete '%s'", (const char*)element->GetFullName().c_str());
                        frame->GetSequenceElements().DeleteElement(element->GetName());
                    }
                    break;
                case 2:
                    // Map effects
                    logger_base.debug("Sequence Element Mismatch 2: map '%s'", (const char*)element->GetFullName().c_str());
                    toMap.push_back(element);
                    //relo
                    break;
                case 3:
                    // Handle later
                    logger_base.debug("Sequence Element Mismatch 2: handle later '%s'", (const char*)element->GetFullName().c_str());
                    ignore.push_back(element);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

static bool HasEffects(ModelElement *el) {
    if (el->HasEffects()) {
        return true;
    }
    for (size_t sm = 0; sm < el->GetSubModelAndStrandCount(); sm++) {
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
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    //bool cancelled = cancelled_in;
    bool cancelled = false;

    logger_base.debug("CheckForValidModels: building model list.");

    std::vector<std::string> AllNames;
    std::vector<std::string> ModelNames;
    for (const auto& it : AllModels) {
        if (it.second != nullptr) {
            AllNames.push_back(it.first);
            if (it.second->GetDisplayAs() != "ModelGroup") {
                ModelNames.push_back(it.first);
            }
        }
    }

    logger_base.debug("CheckForValidModels: Remove models that already exist.");

    std::vector<std::string> missingModels;

    if ((!_renderMode && !_checkSequenceMode) || _promptBatchRenderIssues) {
        for (int x = _sequenceElements.GetElementCount() - 1; x >= 0; x--) {
            Element* element = _sequenceElements.GetElement(x);
            if (element && ElementType::ELEMENT_TYPE_MODEL == element->GetType()) {
                std::string name = element->GetModelName();
                
                if (AllModels[name] == nullptr && element->GetEffectCount()>0) {
                    //check to see if we have an alias
                    for (const auto& it : AllModels) {
                        if (it.second->IsAlias(name, true)) {
                            //this will map to an alias later, skip it
                        } else if (it.second->IsAlias(name, false)) {
                            int numfx = element->GetEffectCount(); // useful info for user
                            std::string desc = name + "(" + std::to_string(numfx) + ")";
                            missingModels.push_back(desc); // show which ones have effects (tells user how important)
                        }
                    }
                    
                }
                //remove the current models from the list so we don't end up with the same model represented twice
                Remove(AllNames, name);
                Remove(ModelNames, name);
            }
        }
    }

    bool mapall = false;
    if (missingModels.size() > 7) {
        std::string seqName = "No Name";
        if (CurrentSeqXmlFile != nullptr) {
            seqName = CurrentSeqXmlFile->GetFullName();
        }
        std::string missings;
        for (const auto &name: missingModels) {
            missings += name + ", ";
        }
        missings.pop_back();
        missings.pop_back(); //drop last delimiter
        auto msg = wxString::Format("The sequence you are opening '%s' contains %d models which are not in your layout (%s). We suggest you import this sequence instead. Do you want to continue to open it?", seqName, (int)missingModels.size(), missings.c_str());
        if (wxMessageBox(msg, "Many missing models in this sequence", wxYES_NO) == wxNO) {
            mapall = true;
        }
    }

    std::vector<Element*> mapLater;
    SeqElementMismatchDialog dialog(this);

    logger_base.debug("CheckForValidModels: Remmap missing models.");

    // Check each model element in the sequence
    // We do this because we can just rename them so it is easy
    // Strands, nodes and submodels are not so easy ... we have to delete them or map them
    for (int x = _sequenceElements.GetElementCount() - 1; x >= 0; x--) {
        if (ElementType::ELEMENT_TYPE_MODEL == _sequenceElements.GetElement(x)->GetType()) {

            // Find the model/model group for the element
            Element *el = _sequenceElements.GetElement(x);
            ModelElement* me = static_cast<ModelElement*>(el);
            if (me != nullptr) {
                std::string name = me->GetModelName();
                Model* m = AllModels[name];

                // If model is not found we need to remap
                if (m == nullptr) {
                    logger_base.debug("CheckForValidModels:    Missing model: %s.", (const char*)name.c_str());
                    if (!mapall) {
                        dialog.StaticTextMessage->SetLabel("Model '" + name + "'\ndoes not exist in your list of models");
                        dialog.ChoiceModels->Set(ToArrayString(AllNames));
                        bool renameAlias = false;
                        if (AllNames.size() > 0) {

                            std::string mapto = "";
                            // go through all the models looking for an alias match
                            for (const auto& it : AllModels)
                            {
                                if (it.second->IsAlias(name, true)) {
                                    renameAlias = true;
                                    mapto = it.first;
                                    break;
                                } else if (it.second->IsAlias(name, false)) {
                                    // this is an alias but not a rename one
                                    mapto = it.first;                                
                                }
                            }

                            if (mapto == "") {
                                dialog.ChoiceModels->SetSelection(0);
                            } else {
                                dialog.ChoiceModels->SetStringSelection(mapto);
                                dialog.RadioButtonRename->SetValue(true);
                            }
                        }
                        else {
                            dialog.ChoiceModels->Hide();
                            dialog.RadioButtonRename->Hide();
                            dialog.Layout();
                        }
                        dialog.Fit();

                        // if mapto is not blank then we can use an oldname alias to remap automagically
                        if (!renameAlias && ((!_renderMode && !_checkSequenceMode) || _promptBatchRenderIssues) && !cancelled && HasEffects(me)) {
                            cancelled = (dialog.ShowModal() == wxID_CANCEL);
                        }
                    }

                    if (cancelled || (!_promptBatchRenderIssues && (_renderMode || _checkSequenceMode)) || !HasEffects(me) || (!mapall && dialog.RadioButtonDelete->GetValue())) {
                        // Just delete the element from the sequence we are opening
                        logger_base.debug("Sequence Element Mismatch: deleting '%s'", (const char*)name.c_str());
                        _sequenceElements.DeleteElement(name);
                    }
                    else if (mapall || dialog.RadioButtonMap->GetValue()) {
                        // add it to the list of things we will map later
                        logger_base.debug("Sequence Element Mismatch: map later '%s'", (const char*)name.c_str());
                        mapLater.push_back(me);
                    }
                    else {
                        // change the name of the element to the new name
                        std::string newName = dialog.ChoiceModels->GetStringSelection().ToStdString();
                        if (newName != "") {
                            logger_base.debug("Sequence Element Mismatch: rename '%s' to '%s'", (const char*)name.c_str(), (const char*)newName.c_str());
                            // This does not seem to be necessary and it has some bad side effects such as removing the model from all views
                            //_sequenceElements.DeleteElement(newName);
                            _sequenceElements.GetElement(x)->SetName(newName);
                            if (AllModels[newName] == nullptr) {
                                logger_base.crit("Sequence Element Mismatch: rename '%s' to '%s' AllModels[newName] returned nullptr ... this is going to crash", (const char*)name.c_str(), (const char*)newName.c_str());
                            }
                            ((ModelElement*)_sequenceElements.GetElement(x))->Init(*AllModels[newName]);
                            Remove(AllNames, newName);
                            Remove(ModelNames, newName);
                        }
                        else {
                            logger_base.error("Sequence Element Mismatch: rename '%s' to '%s' tried to rename to blank.", (const char*)name.c_str(), (const char*)newName.c_str());
                        }
                    }
                }
            }
        }
    }

    logger_base.debug("CheckForValidModels: Prepare for map later.");

    std::vector<Element*> toMap;
    std::vector<Element*> ignore;

    // build the list of models to map
    for (const auto& a : mapLater) {
        toMap.push_back(a);
    }
    mapLater.clear();

    do {
        // If we have something to map ask the user to do so
        if (!toMap.empty()) {

            // we only map at the model level so we need to build a list of them only
            std::vector<Element*> modelElements;
            for (const auto& it : toMap) {
                Element* me = it;
                if (it->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL) {
                    auto sme = dynamic_cast<SubModelElement*>(it);
                    if (sme != nullptr) {
                        me = sme->GetModelElement();
                    }
                }

                if (std::find(modelElements.begin(), modelElements.end(), me) == modelElements.end()) {
                    modelElements.push_back(me);
                }
            }

            ImportXLights(_sequenceElements, modelElements, wxFileName(), false, false, true, true);

            for (const auto& it : toMap) {
                if (it->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                    _sequenceElements.DeleteElement(it->GetName());
                }
                else if (it->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL) {
                    SubModelElement* sme = dynamic_cast<SubModelElement*>(it);
                    if (sme != nullptr) {
                        sme->GetModelElement()->RemoveSubModel(sme->GetName());
                    }
                }
            }
        }

        toMap.clear();

        // Now we go through everything again ... but we also look at strands and submodels and nodes
        for (int x = _sequenceElements.GetElementCount() - 1; x >= 0; x--) {

            if (ElementType::ELEMENT_TYPE_MODEL == _sequenceElements.GetElement(x)->GetType()) {

                std::string name = _sequenceElements.GetElement(x)->GetModelName();
                ModelElement* el = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(x));
                if (el != nullptr) {
                    Model* m = AllModels[name];

                    // model still doesnt exist
                    if (m == nullptr) {
                        // If we have effects at any level
                        if (((!_renderMode && !_checkSequenceMode) || _promptBatchRenderIssues) && HasEffects(el)) {
                            HandleChoices(this, AllNames, ModelNames, el,
                                "Model " + name + " does not exist in your layout.\n"
                                + "How should we handle this?",
                                toMap, ignore, mapall);
                        }
                        else {
                            // no effects at any level so just remove it
                            logger_base.debug("Sequence Element Mismatch 2: deleting '%s'", (const char*)name.c_str());
                            _sequenceElements.DeleteElement(name);
                        }
                    }
                    else if (m->GetDisplayAs() == "ModelGroup") {
                        bool hasStrandEffects = false;
                        bool hasNodeEffects = false;
                        for (int l = 0; l < el->GetStrandCount(); l++) {
                            StrandElement* sl = el->GetStrand(l);
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
                        for (const auto& a : ignore) {
                            if (el == a) {
                                hasNodeEffects = false;
                                hasStrandEffects = false;
                            }
                        }
                        if (((!_renderMode && !_checkSequenceMode) || _promptBatchRenderIssues) && (hasNodeEffects || hasStrandEffects)) {
                            HandleChoices(this, AllNames, ModelNames, el,
                                "Model " + name + " is a Model Group but has Node/Strand effects.\n"
                                + "How should we handle this?",
                                toMap, ignore, mapall);
                        }
                    }
                    else {
                        for (int x1 = 0; x1 < el->GetSubModelAndStrandCount(); x1++) {
                            SubModelElement* sme = el->GetSubModel(x1);
                            if (sme != nullptr &&
                                dynamic_cast<StrandElement*>(sme) == nullptr &&
                                m->GetSubModel(sme->GetName()) == nullptr) {
                                std::vector<std::string> AllSMNames;
                                std::vector<std::string> ModelSMNames;
                                for (int z = 0; z < m->GetNumSubModels(); z++) {
                                    AllSMNames.push_back(m->GetSubModel(z)->GetName());
                                    ModelSMNames.push_back(m->GetSubModel(z)->GetName());
                                }
                                if ((!_renderMode && !_checkSequenceMode) || _promptBatchRenderIssues) {
                                    int priorCnt = el->GetSubModelAndStrandCount();
                                    HandleChoices(this, AllSMNames, ModelSMNames, sme,
                                        "SubModel " + sme->GetName() + " of Model " + m->GetName() + " does not exist.\n"
                                        + "How should we handle this?",
                                        toMap, ignore, mapall);
                                    // if count after is less than the count before then the submodel list is shorter, so rewind the index
                                    if (priorCnt != el->GetSubModelAndStrandCount()) {
                                        --x1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    } while (!toMap.empty());
}

void xLightsFrame::LoadAudioData(xLightsXmlFile& xml_file)
{
    // abort any in progress render ... as it may be using any already open media
    if (xml_file.GetMedia() != nullptr) {
        AbortRender();
    }

    mMediaLengthMS = xml_file.GetSequenceDurationMS();

    if (xml_file.GetSequenceType() == "Media") {
        int musicLength = 0;
        mediaFilename = "";
        if (xml_file.GetMedia() == nullptr) {
            mediaFilename = xml_file.GetMediaFile();
            ObtainAccessToURL(mediaFilename);
            if (mediaFilename.empty() || !FileExists(mediaFilename) || !wxIsReadable(mediaFilename)) {
                SeqSettingsDialog setting_dlg(this, &xml_file, mediaDirectories, wxT(""), wxEmptyString);
                setting_dlg.Fit();
                int ret_val = setting_dlg.ShowModal();

                mediaFilename = xml_file.GetMediaFile();

                if (xml_file.GetMedia() != nullptr) {
                    mediaFilename = xml_file.GetMedia()->FileName();
                    ObtainAccessToURL(mediaFilename);
                    if (xml_file.GetMedia() != nullptr && xml_file.GetMedia()->GetFrameInterval() < 0) {
                        xml_file.GetMedia()->SetFrameInterval(xml_file.GetFrameMS());
                    }
                    SetAudioControls();
                }

                if (ret_val == NEEDS_RENDER) {
                    RenderAll();
                }
            }
        }
        else {
            mediaFilename = xml_file.GetMediaFile();
            ObtainAccessToURL(mediaFilename);
        }

        if (!mediaFilename.empty()) {
            wxString error;
            musicLength = mainSequencer->PanelWaveForm->OpenfileMedia(xml_file.GetMedia(), error);
            if (musicLength <= 0) {
                DisplayWarning(wxString::Format("Media File Missing or Corrupted %s.\n\nDetails: %s", mediaFilename, error).ToStdString());
            }
            else {
                sequenceVideoPanel->SetMediaPath(mediaFilename);
            }
        }
        else if (xml_file.GetSequenceType() == "Media") {
            DisplayWarning("Media File must be specified");
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
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Load sequence %s", (const char*)xml_file.GetFullPath().c_str());

    PushTraceContext();
    SetFrequency(xml_file.GetFrequency());
    _sequenceElements.SetViewsManager(GetViewsManager()); // This must come first before LoadSequencerFile.
    _sequenceElements.SetModelsNode(ModelsNode);
    _sequenceElements.SetEffectsNode(EffectsNode);

    AddTraceMessage("loading");
    _sequenceElements.LoadSequencerFile(xml_file, GetShowDirectory());

    logger_base.debug("Upgrading sequence");
    xml_file.AdjustEffectSettingsForVersion(_sequenceElements, this);

    Menu_Settings_Sequence->Enable(true);

    mSavedChangeCount = _sequenceElements.GetChangeCount();
    mLastAutosaveCount = mSavedChangeCount;

    logger_base.debug("Checking for valid models");
    CheckForValidModels();

    logger_base.debug("Loading the audio data");
    LoadAudioData(xml_file);

    logger_base.debug("Preparing views");
    _sequenceElements.PrepareViews(xml_file);

    logger_base.debug("Populating row information");
    _sequenceElements.PopulateRowInformation();

    mainSequencer->PanelEffectGrid->SetSequenceElements(&_sequenceElements);
    mainSequencer->PanelEffectGrid->SetTimeline(mainSequencer->PanelTimeLine);

    logger_base.debug("Updating the timeline");
    mainSequencer->PanelTimeLine->SetSequenceEnd(CurrentSeqXmlFile->GetSequenceDurationMS());

    logger_base.debug("Updating the house preview");
    _housePreviewPanel->SetDurationFrames(CurrentSeqXmlFile->GetSequenceDurationMS() / CurrentSeqXmlFile->GetFrameMS());
    _sequenceElements.SetSequenceEnd(CurrentSeqXmlFile->GetSequenceDurationMS());
    ResizeAndMakeEffectsScroll();
    ResizeMainSequencer();
    mainSequencer->PanelEffectGrid->Refresh();
    _modelPreviewPanel->Refresh();
    _housePreviewPanel->Refresh();
    m_mgr->Update();

    _coloursPanel->UpdateColourButtons(true, this);

    logger_base.debug("Sequence all loaded.");

    if ((!_renderMode && !_checkSequenceMode) || _promptBatchRenderIssues) {
        ValidateEffectAssets();
    }

    PopTraceContext();
}

void xLightsFrame::Zoom(wxCommandEvent& event)
{
    if (event.GetInt() == ZOOM_IN) {
        mainSequencer->PanelTimeLine->ZoomIn();
    }
    else {
        mainSequencer->PanelTimeLine->ZoomOut();
    }
}

void xLightsFrame::Scrub(wxCommandEvent& event)
{
    // stop any playing so we can scrub
    playType = PLAY_TYPE_STOPPED;

    int ms = event.GetInt();
    if (ms < 0) ms = 0;
    unsigned int frame = ms / CurrentSeqXmlFile->GetFrameMS();

    if (ms > CurrentSeqXmlFile->GetSequenceDurationMS()) ms = CurrentSeqXmlFile->GetSequenceDurationMS();
    if (frame >= _seqData.NumFrames()) frame = _seqData.NumFrames();

    for (auto &f : _fps) {
        f = -1;
    }
    playCurFrame = -1;
    mainSequencer->UpdateTimeDisplay(ms, _fps);

    // update any video diaplay
    sequenceVideoPanel->UpdateVideo(ms);

    //have the frame, copy from SeqData
    TimerOutput(frame);
    if (playModel != nullptr) {
        int nn = playModel->GetNodeCount();
        for (int node = 0; node < nn; node++) {
            int start = playModel->NodeStartChannel(node);
            playModel->SetNodeChannelValues(node, &_seqData[frame][start]);
        }
        _modelPreviewPanel->setCurrentFrameTime(ms);
        playModel->DisplayEffectOnWindow(_modelPreviewPanel, mPointSize);
    }
    _housePreviewPanel->GetModelPreview()->Render(ms, &_seqData[frame][0]);
    for (const auto& it : PreviewWindows) {
        ModelPreview* preview = it;
        if (preview->GetActive()) {
            preview->Render(ms, &_seqData[frame][0]);
        }
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
    mainSequencer->PanelEffectGrid->ClearSelection();
}

void xLightsFrame::TimeSelected( wxCommandEvent& event)
{
    mainSequencer->PanelTimeLine->SetSelectedPositionStartMS(event.GetInt());
    mainSequencer->PanelWaveForm->Refresh();
}

void xLightsFrame::MousePositionUpdated( wxCommandEvent& event)
{
    if (mainSequencer) {
        mainSequencer->PanelTimeLine->SetMousePositionMS(event.GetInt());
        mainSequencer->PanelTimeLine->Update();
        mainSequencer->PanelWaveForm->Refresh();
        mainSequencer->PanelWaveForm->Update();
    }
}

void xLightsFrame::RowHeadingsChanged( wxCommandEvent& event)
{
    wxString s = event.GetString();
    if ("" != s) {
        for(wxXmlNode* e=ModelGroupsNode->GetChildren(); e!=nullptr; e=e->GetNext() ) {
            if (e->GetName() == "modelGroup") {
                if (s == e->GetAttribute("name")) {
                    std::string modelString = e->GetAttribute("models").ToStdString();
                    _sequenceElements.AddMissingModelsToSequence(modelString, false);
                }
            }
        }
    }
    _sequenceElements.PopulateRowInformation();
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
    _sequenceElements.SetMaxRowsDisplayed(mainSequencer->PanelRowHeadings->GetMaxRows());
    _sequenceElements.PopulateVisibleRowInformation();

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
        SetPlayStatus(PLAY_TYPE_STOPPED);
        playStartTime = -1;
        playEndTime = -1;
        playStartMS = -1;
        _outputManager.AllOff(); // Force clear all outputs just in case we are sending data
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
    SetEffectControls(effect->GetParentEffectLayer()->GetParentElement()->GetFullName(),
                      effect->GetEffectName(), effect->GetSettings(), effect->GetPaletteMap(),
                      effect->GetStartTimeMS(), effect->GetEndTimeMS(), true);
    selectedEffectString = "";  // force update to effect rendering
}

// flags something has changed in an effect but does not send the effect
void xLightsFrame::EffectUpdated(wxCommandEvent& event)
{
    if (selectedEffect != nullptr) {
        // For canvas mode the timing panel needs to know how many layers are under this effect
        int layers = selectedEffect->GetParentEffectLayer()->GetParentElement()->GetEffectLayerCount();
        int start = selectedEffect->GetParentEffectLayer()->GetLayerNumber() + 1;
		std::vector<int> effectLayers = selectedEffect->GetParentEffectLayer()->GetParentElement()->GetLayersWithEffectsByTime(selectedEffect->GetStartTimeMS(), selectedEffect->GetEndTimeMS());
        if (start > layers) start = -1;
        timingPanel->SetLayersBelow(start, layers, effectLayers, _sequenceElements.SupportsModelBlending());
    }
}

void xLightsFrame::SelectedEffectChanged(SelectedEffectChangedEvent& event)
{
    EffectSettingsTimer.Start(25);
    
    // prevent re-entry notification of effect selected changed
    static bool reentry = false;
    if (reentry) {
        return;
    }
    reentry = true;

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool OnlyChoiceBookPage = event.effect==nullptr?true:false;
    Effect* effect = nullptr;
    if(OnlyChoiceBookPage)
    {
        int pageIndex = event.GetInt();
        // Dont change page if it is already on correct page
        if (EffectsPanel1->EffectChoicebook->GetSelection()!=pageIndex) {
            EffectsPanel1->SetEffectType(pageIndex);
            ResetPanelDefaultSettings(EffectsPanel1->EffectChoicebook->GetChoiceCtrl()->GetStringSelection(), nullptr, true);
        } else {
            event.updateUI = false;
        }
    }
    else
    {
        Element* element = _sequenceElements.GetElement(event._elementName);
        if (element != nullptr)
        {
            if (event._node != -1)
            {
                StrandElement* se = (StrandElement*)element;
                NodeLayer* nodeLayer = se->GetNodeLayer(event._node - 1); // not sure why -2
                if (nodeLayer != nullptr)
                {
                    // The +1 guarantees we get the right one
                    effect = nodeLayer->GetEffectAtTime(event._startTime + 1);
                }
                else
                {
                    logger_base.error("SelectedEffectChanged ... node layer no longer exists %s %d", (const char *)event._elementName.c_str(), event._node);
                }
            }
            else
            {
                EffectLayer* effectLayer = element->GetEffectLayer(event._layer - 1);
                if (effectLayer != nullptr)
                {
                    // The +1 guarantees we get the right one
                    effect = effectLayer->GetEffectAtTime(event._startTime + 1);
                }
                else
                {
                    logger_base.error("SelectedEffectChanged ... element layer no longer exists %s %d", (const char *)event._elementName.c_str(), event._layer);
                }
            }
        }
        else
        {
            logger_base.error("SelectedEffectChanged ... element no longer exists %s", (const char *)event._elementName.c_str());
        }

        //effect = event.effect;
        wxASSERT(event.effect == effect);

        if (effect == nullptr)
        {
            logger_base.error("SelectedEffectChanged ... effect no longer exists %s %d %dms", (const char *)event._elementName.c_str(), event._layer, event._startTime);
        }
        else
        {
            if (effect->GetParentEffectLayer()->GetParentElement()->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
                if (event.effect != effect) {
                    logger_base.warn("SelectedEffectChanged ... effect didnt match");
                }

                // For canvas mode the timing panel needs to know how many layers are under this effect
                int layers = effect->GetParentEffectLayer()->GetParentElement()->GetEffectLayerCount();
                int start = effect->GetParentEffectLayer()->GetLayerNumber() + 1;
                std::vector<int> effectLayers = effect->GetParentEffectLayer()->GetParentElement()->GetLayersWithEffectsByTime(effect->GetStartTimeMS(), effect->GetEndTimeMS());
                if (start > layers) start = -1;
                timingPanel->SetLayersBelow(start, layers, effectLayers, _sequenceElements.SupportsModelBlending());

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
                SetEffectControls(effect->GetParentEffectLayer()->GetParentElement()->GetFullName(),
                    effect->GetEffectName(), effect->GetSettings(), effect->GetPaletteMap(),
                    effect->GetStartTimeMS(), effect->GetEndTimeMS(), !event.isNew);
                selectedEffectString = GetEffectTextFromWindows(selectedEffectPalette);
                selectedEffect = effect;

                if (effect->GetPaletteMap().empty() || resetStrings || effect->SettingsChanged(selectedEffectString)) { // the settings changed is necessary because there are some situations where a new effect wont render because somehow the selectedEffectString changes without triggering a render. I think it relates to effects that auto-set certain values based on the model such as faces and states
                    effect->SetPalette(selectedEffectPalette);
                    effect->SetSettings(selectedEffectString, true);
                    if (!_suspendRender) {
                        RenderEffectForModel(effect->GetParentEffectLayer()->GetParentElement()->GetModelName(),
                            effect->GetStartTimeMS(),
                            effect->GetEndTimeMS());
                    }
                }

                if (playType == PLAY_TYPE_MODEL_PAUSED) {
                    DoStopSequence();
                }

                if (playType != PLAY_TYPE_MODEL) {
                    SetPlayStatus(PLAY_TYPE_EFFECT);
                    playStartTime = effect->GetStartTimeMS();
                    playEndTime = effect->GetEndTimeMS();
                    playStartMS = -1;
                    playModel = GetModel(effect->GetParentEffectLayer()->GetParentElement()->GetModelName());
                    SetAudioControls();
                }
            }
        }
    }
    if (event.updateUI || event.updateBtn) {
        RenderableEffect *eff = effectManager[EffectsPanel1->EffectChoicebook->GetSelection()];
        effectsPnl->SetDragIconBuffer(eff->GetEffectIcon());
        effectsPnl->BitmapButtonSelectedEffect->SetEffect(eff, mIconSize);
        if( effect != nullptr ) {
            UpdateEffectAssistWindow(effect, eff);
        }
        mainSequencer->PanelEffectGrid->SetFocus();
    }

    reentry = false;
}

void xLightsFrame::SelectedRowChanged(wxCommandEvent& event)
{
    mainSequencer->PanelRowHeadings->SetSelectedRow(event.GetInt());
    playModel = GetModel(event.GetString().ToStdString());
}

void xLightsFrame::EffectDroppedOnGrid(wxCommandEvent& event)
{
    int effectIndex = EffectsPanel1->EffectChoicebook->GetSelection();
    _sequenceElements.UnSelectAllEffects();
    std::string name = EffectsPanel1->EffectChoicebook->GetPageText(effectIndex).ToStdString();
    std::string palette;
    std::string settings = GetEffectTextFromWindows(palette);
    selectedEffect = nullptr;
    Effect* last_effect_created = nullptr;

    _sequenceElements.get_undo_mgr().CreateUndoStep();
    for(size_t i=0;i<_sequenceElements.GetSelectedRangeCount();i++)
    {
        EffectLayer* el = _sequenceElements.GetSelectedRange(i)->Layer;
        if (el->GetParentElement()->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            continue;
        }
        // Delete Effects that are in same time range as dropped effect
        el->SelectEffectsInTimeRange(_sequenceElements.GetSelectedRange(i)->StartTime,
                                     _sequenceElements.GetSelectedRange(i)->EndTime);
        el->DeleteSelectedEffects(_sequenceElements.get_undo_mgr());
        // Add dropped effect
        Effect* effect = el->AddEffect(0,name,settings,palette,
                                       _sequenceElements.GetSelectedRange(i)->StartTime,
                                       _sequenceElements.GetSelectedRange(i)->EndTime,
                                       EFFECT_SELECTED,false);

        // Change render buffer to Per Model for models that need it
        Model* m = AllModels[el->GetParentElement()->GetModelName()];
        if( m->GetDisplayAs() == "ModelGroup" ) {
            auto mg = dynamic_cast<ModelGroup*>(m);
            if (mg != nullptr) {
                // see if all models in the group match the desired model types
                bool all_good = true;
                for (const auto& it : mg->GetFlatModels(true, false)) {
                    if (it->GetDisplayAs() != "DmxMovingHeadAdv" && it->GetDisplayAs() != "DmxMovingHead") {
                        all_good = false;
                        break;
                    }
                }
                if( all_good ) {
                    effect->GetSettings()["B_CHOICE_BufferStyle"] = "Per Model Default";
                }
            }
        }

        last_effect_created = effect;

        _sequenceElements.get_undo_mgr().CaptureAddedEffect( el->GetParentElement()->GetFullName(), el->GetIndex(), effect->GetID() );

        mainSequencer->PanelEffectGrid->ProcessDroppedEffect(effect);

        // need to do this otherwise they dont update when we drop the model
        bufferPanel->UpdateBufferStyles(AllModels[el->GetParentElement()->GetFullName()]);
        bufferPanel->UpdateCamera(AllModels[el->GetParentElement()->GetModelName()]); //need the full model, not submodel, for the camera

        if (playType == PLAY_TYPE_MODEL_PAUSED) {
            DoStopSequence();
			SetAudioControls();
		}

        if (playType != PLAY_TYPE_MODEL) {
            SetPlayStatus(PLAY_TYPE_EFFECT);
            playStartTime = _sequenceElements.GetSelectedRange(i)->StartTime;
            playEndTime = _sequenceElements.GetSelectedRange(i)->EndTime;
            playStartMS = -1;
            if (!_suspendRender)
            {
                RenderEffectForModel(el->GetParentElement()->GetModelName(), playStartTime, playEndTime);
            }

            playModel = GetModel(el->GetParentElement()->GetModelName());

			SetAudioControls();
        }
    }

    if (playType != PLAY_TYPE_MODEL && last_effect_created != nullptr)
    {
        SetEffectControls(last_effect_created->GetParentEffectLayer()->GetParentElement()->GetFullName(),
                          last_effect_created->GetEffectName(), last_effect_created->GetSettings(),
                          last_effect_created->GetPaletteMap(), last_effect_created->GetStartTimeMS(),
                          last_effect_created->GetEndTimeMS(), false);
        selectedEffectString = GetEffectTextFromWindows(selectedEffectPalette);
        selectedEffect = last_effect_created;
    }

	RenderableEffect *eff = effectManager[EffectsPanel1->EffectChoicebook->GetSelection()];

	if( last_effect_created != nullptr ) {
        UpdateEffectAssistWindow(last_effect_created, eff);
	}

    mainSequencer->PanelRowHeadings->Refresh(false);
    mainSequencer->PanelEffectGrid->Refresh(false);
}

void xLightsFrame::EffectFileDroppedOnGrid(wxCommandEvent& event)
{
    auto parms = wxSplit(event.GetString(), '|');

    if (parms.size() != 2) return;

    std::string effectName = parms[0].ToStdString();
    std::string filename = parms[1].ToStdString();

    int effectIndex = 0;
    for (size_t i = 0; i < EffectsPanel1->EffectChoicebook->GetChoiceCtrl()->GetCount(); i++)
    {
        if (EffectsPanel1->EffectChoicebook->GetChoiceCtrl()->GetString(i) == effectName)
        {
            EffectsPanel1->EffectChoicebook->SetSelection(i);
            effectIndex = i;
            break;
        }
    }

    wxASSERT(effectIndex != 0);

    _sequenceElements.UnSelectAllEffects();
    std::string palette;
    std::string settings = GetEffectTextFromWindows(palette);
    selectedEffect = nullptr;
    Effect* last_effect_created = nullptr;

    _sequenceElements.get_undo_mgr().CreateUndoStep();
    for (size_t i = 0; i < _sequenceElements.GetSelectedRangeCount(); i++)
    {
        EffectLayer* el = _sequenceElements.GetSelectedRange(i)->Layer;
        if (el->GetParentElement()->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            continue;
        }
        // Delete Effects that are in same time range as dropped effect
        el->SelectEffectsInTimeRange(_sequenceElements.GetSelectedRange(i)->StartTime,
            _sequenceElements.GetSelectedRange(i)->EndTime);
        el->DeleteSelectedEffects(_sequenceElements.get_undo_mgr());
        // Add dropped effect
        Effect* effect = el->AddEffect(0, effectName, settings, palette,
            _sequenceElements.GetSelectedRange(i)->StartTime,
            _sequenceElements.GetSelectedRange(i)->EndTime,
            EFFECT_SELECTED, false);

        // Now set the filename
        if (effectName == "Video")
        {
            effect->GetSettings()["E_FILEPICKERCTRL_Video_Filename"] = filename;
        }
        else if (effectName == "Pictures")
        {
            effect->GetSettings()["E_FILEPICKER_Pictures_Filename"] = filename;
        }
        else if (effectName == "Glediator")
        {
            effect->GetSettings()["E_FILEPICKERCTRL_Glediator_Filename"] = filename;
        }
        else if (effectName == "Shader") {
            effect->GetSettings()["E_0FILEPICKERCTRL_IFS"] = filename;
        }

        last_effect_created = effect;

        _sequenceElements.get_undo_mgr().CaptureAddedEffect(el->GetParentElement()->GetModelName(), el->GetIndex(), effect->GetID());

        mainSequencer->PanelEffectGrid->ProcessDroppedEffect(effect);

        if (playType == PLAY_TYPE_MODEL_PAUSED) {
            DoStopSequence();
            SetAudioControls();
        }

        if (playType != PLAY_TYPE_MODEL) {
            SetPlayStatus(PLAY_TYPE_EFFECT);
            playStartTime = _sequenceElements.GetSelectedRange(i)->StartTime;
            playEndTime = _sequenceElements.GetSelectedRange(i)->EndTime;
            playStartMS = -1;
            if (!_suspendRender)
            {
                RenderEffectForModel(el->GetParentElement()->GetModelName(), playStartTime, playEndTime);
            }

            playModel = GetModel(el->GetParentElement()->GetModelName());

            SetAudioControls();
        }
    }

    if (playType != PLAY_TYPE_MODEL && last_effect_created != nullptr)
    {
        SetEffectControls(last_effect_created->GetParentEffectLayer()->GetParentElement()->GetFullName(),
            last_effect_created->GetEffectName(), last_effect_created->GetSettings(),
            last_effect_created->GetPaletteMap(), last_effect_created->GetStartTimeMS(),
            last_effect_created->GetEndTimeMS(), false);
        selectedEffectString = GetEffectTextFromWindows(selectedEffectPalette);
        selectedEffect = last_effect_created;
    }

    RenderableEffect *eff = effectManager[EffectsPanel1->EffectChoicebook->GetSelection()];

    if (last_effect_created != nullptr) {
        UpdateEffectAssistWindow(last_effect_created, eff);
    }

    mainSequencer->PanelRowHeadings->Refresh(false);
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

void xLightsFrame::CutModelEffects(wxCommandEvent& event)
{
    mainSequencer->PanelEffectGrid->CutModelEffects(event.GetInt(), event.GetString() == "All");
    UnselectEffect();
}

void xLightsFrame::CopyModelEffects(wxCommandEvent& event)
{
    mainSequencer->PanelEffectGrid->CopyModelEffects(event.GetInt(), event.GetString().StartsWith("All"), event.GetString() == "AllInclSub");
}

void xLightsFrame::PasteModelEffects(wxCommandEvent& event)
{
    mainSequencer->PanelEffectGrid->PasteModelEffects(event.GetInt(), event.GetString() == "All");
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
                auto& hp = m_mgr->GetPane("HousePreview");
                hp.Show();
                if (_wasMaximised)
                {
                    m_mgr->MaximizePane(hp);
                }
                m_mgr->Update();
            }
        }
        else
        {
            _wasMaximised = false;
            if (visible)
            {
                auto& hp = m_mgr->GetPane("HousePreview");
                if (hp.IsMaximized() && hp.IsDocked())
                {
                    _wasMaximised = true;
                    m_mgr->RestoreMaximizedPane();
                }
                hp.Hide();
                m_mgr->Update();
            }
        }
    }
    UpdateViewMenu();
}

void xLightsFrame::DoPlaySequence()
{
	if (CurrentSeqXmlFile != nullptr) {
        if (playType == PLAY_TYPE_EFFECT_PAUSED) {
            SetPlayStatus(PLAY_TYPE_EFFECT);
			playStartMS = -1;
        } else {
            SetPlayStatus(PLAY_TYPE_MODEL);
			playStartMS = -1;
			playStartTime = mainSequencer->PanelTimeLine->GetNewStartTimeMS();
			playEndTime = mainSequencer->PanelTimeLine->GetNewEndTimeMS();
			if (CurrentSeqXmlFile->GetSequenceType() == "Media") {
				if (CurrentSeqXmlFile->GetMedia() != nullptr) {
					CurrentSeqXmlFile->GetMedia()->Seek(playStartTime);
				}
			}
			if (playEndTime == -1 || playEndTime > CurrentSeqXmlFile->GetSequenceDurationMS()) {
				playEndTime = CurrentSeqXmlFile->GetSequenceDurationMS();
			}
			mainSequencer->PanelTimeLine->PlayStarted();
			if (CurrentSeqXmlFile->GetSequenceType() == "Media") {
				if (CurrentSeqXmlFile->GetMedia() != nullptr) {
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
    if (CurrentSeqXmlFile == nullptr) return;

    if( CurrentSeqXmlFile->GetSequenceType() == "Media" ) {
        if (CurrentSeqXmlFile->GetMedia() != nullptr) {
			if (CurrentSeqXmlFile->GetMedia()->GetPlayingState() == MEDIAPLAYINGSTATE::PLAYING) {
				CurrentSeqXmlFile->GetMedia()->Pause();
            } else if (CurrentSeqXmlFile->GetMedia()->GetPlayingState() == MEDIAPLAYINGSTATE::PAUSED) {
				CurrentSeqXmlFile->GetMedia()->Play();
			}
		}
	}

    if (playType == PLAY_TYPE_MODEL) {
        SetPlayStatus(PLAY_TYPE_MODEL_PAUSED);
    } else if (playType == PLAY_TYPE_MODEL_PAUSED) {
        mainSequencer->PanelTimeLine->CheckNeedToScrollToPlayStart(true);
        SetPlayStatus(PLAY_TYPE_MODEL);
    } else if (playType == PLAY_TYPE_EFFECT_PAUSED) {
        SetPlayStatus(PLAY_TYPE_EFFECT);
    } else {
        SetPlayStatus(PLAY_TYPE_EFFECT_PAUSED);
    }
	SetAudioControls();
    AutoShowHouse();
}

void xLightsFrame::SetAudioControls()
{
    if (_housePreviewPanel == nullptr) return;

    if (Notebook1->GetSelection() != NEWSEQUENCER)
    {
        if (playType == PLAY_TYPE_MODEL)
        {
            EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, true);
        }
        else
        {
            EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_STOP, false);
        }
        EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PLAY_NOW, false);
        EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_PAUSE, false);
        EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_REPLAY_SECTION, false);
        EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_FIRST_FRAME, false);
        EnableToolbarButton(PlayToolBar, ID_AUITOOLBAR_LAST_FRAME, false);
    }
    else if (CurrentSeqXmlFile == nullptr || mRendering)
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
    if (CurrentSeqXmlFile == nullptr) return;

    for (auto &f : _fps) {
        f = -1;
    }
    playCurFrame = -1;
	mLoopAudio = false;
    if( playType == PLAY_TYPE_MODEL || playType == PLAY_TYPE_MODEL_PAUSED ) {
        if( CurrentSeqXmlFile->GetSequenceType() == "Media" ) {
			if (CurrentSeqXmlFile->GetMedia() != nullptr) {
				CurrentSeqXmlFile->GetMedia()->Stop();
				CurrentSeqXmlFile->GetMedia()->Seek(playStartTime);
			}
        }
        mainSequencer->PanelTimeLine->PlayStopped();
        mainSequencer->PanelWaveForm->UpdatePlayMarker();
        mainSequencer->PanelEffectGrid->ForceRefresh();
        mainSequencer->UpdateTimeDisplay(playStartTime, _fps);
    }
    SetPlayStatus(PLAY_TYPE_STOPPED);
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
    if (CurrentSeqXmlFile == nullptr) return;

    for (auto &f : _fps) {
        f = -1;
    }
    playCurFrame = -1;
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
    if (CurrentSeqXmlFile == nullptr) return;

    for (auto &f : _fps) {
        f = -1;
    }
    playCurFrame = -1;
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
    if (CurrentSeqXmlFile == nullptr) return;

    int current_play_time;
    if (CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->GetMedia() != nullptr)
    {
        current_play_time = CurrentSeqXmlFile->GetMedia()->Tell();
    }
    else
    {
        wxTimeSpan ts = wxDateTime::UNow() - starttime;
        long curtime = ts.GetMilliseconds().ToLong();
        int msec;
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
    if (CurrentSeqXmlFile == nullptr) return;

    int current_play_time;
    if (CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->GetMedia() != nullptr)
    {
        current_play_time = CurrentSeqXmlFile->GetMedia()->Tell();
    }
    else
    {
        wxTimeSpan ts = wxDateTime::UNow() - starttime;
        long curtime = ts.GetMilliseconds().ToLong();
        int msec;
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

void xLightsFrame::SequencePriorTag(wxCommandEvent& event) {
    if (CurrentSeqXmlFile == nullptr)
        return;

    int current_play_time;
    if (CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->GetMedia() != nullptr) {
        current_play_time = CurrentSeqXmlFile->GetMedia()->Tell();
    } else {
        wxTimeSpan ts = wxDateTime::UNow() - starttime;
        long curtime = ts.GetMilliseconds().ToLong();
        int msec;
        if (playAnimation) {
            msec = curtime * playSpeed;
        } else {
            msec = curtime;
        }
        current_play_time = (playStartTime + msec - playStartMS);
    }

    long origtime = current_play_time;
    current_play_time = mainSequencer->PanelTimeLine->GetPriorTag(origtime - 500);

    int end_ms = CurrentSeqXmlFile->GetSequenceDurationMS();
    if (current_play_time > end_ms)
        current_play_time = end_ms;

    if (CurrentSeqXmlFile->GetSequenceType() == "Media") {
        if (CurrentSeqXmlFile->GetMedia() != nullptr) {
            CurrentSeqXmlFile->GetMedia()->Seek(current_play_time);
        }
    } else {
        starttime += wxTimeSpan(0, 0, (origtime - current_play_time) / 1000, (origtime - current_play_time) % 1000);
    }

    mainSequencer->PanelWaveForm->UpdatePlayMarker();
    mainSequencer->PanelEffectGrid->ForceRefresh();
    mainSequencer->UpdateTimeDisplay(current_play_time, _fps);
}

void xLightsFrame::SequenceNextTag(wxCommandEvent& event) {
    if (CurrentSeqXmlFile == nullptr)
        return;

    int current_play_time;
    if (CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->GetMedia() != nullptr) {
        current_play_time = CurrentSeqXmlFile->GetMedia()->Tell();
    } else {
        wxTimeSpan ts = wxDateTime::UNow() - starttime;
        long curtime = ts.GetMilliseconds().ToLong();
        int msec;
        if (playAnimation) {
            msec = curtime * playSpeed;
        } else {
            msec = curtime;
        }

        current_play_time = (playStartTime + msec - playStartMS);
    }

    long origtime = current_play_time;
    current_play_time = mainSequencer->PanelTimeLine->GetNextTag(origtime);
    int end_ms = CurrentSeqXmlFile->GetSequenceDurationMS();
    if (current_play_time > end_ms)
        current_play_time = end_ms;

    if (CurrentSeqXmlFile->GetSequenceType() == "Media") {
        if (CurrentSeqXmlFile->GetMedia() != nullptr) {
            CurrentSeqXmlFile->GetMedia()->Seek(current_play_time);
        }
    } else {
        starttime += wxTimeSpan(0, 0, (origtime - current_play_time) / 1000, (origtime - current_play_time) % 1000);
    }

    mainSequencer->PanelWaveForm->UpdatePlayMarker();
    mainSequencer->PanelEffectGrid->ForceRefresh();
    mainSequencer->UpdateTimeDisplay(current_play_time, _fps);
}

void xLightsFrame::SequenceSeekTo(wxCommandEvent& event)
{
    if (CurrentSeqXmlFile == nullptr) return;

    int pos = event.GetInt();
    int current_play_time;
    if (CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->GetMedia() != nullptr) {
        current_play_time = CurrentSeqXmlFile->GetMedia()->Tell();
    } else {
        wxTimeSpan ts = wxDateTime::UNow() - starttime;
        long curtime = ts.GetMilliseconds().ToLong();
        int msec;
        if (playAnimation) {
            msec = curtime * playSpeed;
        } else {
            msec = curtime;
        }

        current_play_time = (playStartTime + msec - playStartMS);
    }

    long origtime = current_play_time;
    current_play_time += (pos - current_play_time);
    int end_ms = CurrentSeqXmlFile->GetSequenceDurationMS();
    if (current_play_time > end_ms) {
        current_play_time = end_ms;
    }

    if (CurrentSeqXmlFile->GetSequenceType() == "Media") {
        if (CurrentSeqXmlFile->GetMedia() != nullptr) {
            CurrentSeqXmlFile->GetMedia()->Seek(current_play_time);
        }
    } else {
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
            SetPlayStatus(PLAY_TYPE_EFFECT);
            playStartTime = (int)(args->effect->GetStartTimeMS());
            playEndTime = (int)(args->effect->GetEndTimeMS());
            if(args->renderEffect && !_suspendRender)
            {
                RenderEffectForModel(args->element->GetModelName(),playStartTime,playEndTime);
            }

            playStartMS = -1;
        } else {
            SetPlayStatus(PLAY_TYPE_STOPPED);
        }
    }
}

void xLightsFrame::UpdateEffectPalette(wxCommandEvent& event) {

    // Get only the colours from the colour panel ... ignore all the other settings
    std::string palette = colorPanel->GetColorString(true);

    _sequenceElements.get_undo_mgr().CreateUndoStep();
    for (size_t i = 0; i < _sequenceElements.GetRowInformationSize(); i++) {
        Element* element = _sequenceElements.GetRowInformation(i)->element;
        EffectLayer* el = _sequenceElements.GetEffectLayer(i);

        int startms = 99999999;
        int endms = -1;
        for (int j = 0; j < el->GetEffectCount(); j++) {
            if (el->GetEffect(j)->GetSelected() != EFFECT_NOT_SELECTED) {
                Effect* ef = el->GetEffect(j);
                _sequenceElements.get_undo_mgr().CaptureModifiedEffect(element->GetModelName(),
                    el->GetIndex(),
                    ef);
                // only set the colours ... not other settings like sparkles
                ef->SetColourOnlyPalette(palette);
                startms = std::min(startms, ef->GetStartTimeMS());
                endms = std::max(endms, ef->GetEndTimeMS());
            }
        }
        if (startms <= endms) {
            SetPlayStatus(PLAY_TYPE_EFFECT);
            playStartMS = -1;
            if (!_suspendRender) {
                RenderEffectForModel(element->GetModelName(), startms, endms);
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

    _sequenceElements.get_undo_mgr().CreateUndoStep();

    for(size_t i=0;i<_sequenceElements.GetVisibleRowInformationSize();i++) {
        Element* element = _sequenceElements.GetVisibleRowInformation(i)->element;
        EffectLayer* el = _sequenceElements.GetVisibleEffectLayer(i);

        int startms = 99999999;
        int endms = -1;

        for(int j=0;j< el->GetEffectCount();j++) {
            if(el->GetEffect(j)->GetSelected() != EFFECT_NOT_SELECTED) {
                Effect *ef = el->GetEffect(j);
                _sequenceElements.get_undo_mgr().CaptureModifiedEffect(element->GetModelName(),
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
            SetPlayStatus(PLAY_TYPE_EFFECT);
            playStartMS = -1;
            if (!_suspendRender) {
                RenderEffectForModel(element->GetModelName(), startms, endms);
            }
        }
    }
    mainSequencer->PanelEffectGrid->ForceRefresh();
}

void xLightsFrame::RandomizeEffect(wxCommandEvent& event)
{
    _sequenceElements.get_undo_mgr().CreateUndoStep();

    for(size_t i=0;i<_sequenceElements.GetVisibleRowInformationSize();i++) {
        Element* element = _sequenceElements.GetVisibleRowInformation(i)->element;
        EffectLayer* el = _sequenceElements.GetVisibleEffectLayer(i);

        int startms = 99999999;
        int endms = -1;

        for(int j=0;j< el->GetEffectCount();j++) {
            if(el->GetEffect(j)->GetSelected() != EFFECT_NOT_SELECTED) {
                std::string effectName = el->GetEffect(j)->GetEffectName();
                int effectIndex = el->GetEffect(j)->GetEffectIndex();

                auto oldSettings = el->GetEffect(j)->GetSettings();

                std::string settings = EffectsPanel1->GetRandomEffectString(effectIndex).ToStdString();
                std::string palette = colorPanel->GetRandomColorString().ToStdString();

                _sequenceElements.get_undo_mgr().CaptureModifiedEffect(element->GetModelName(),
                                                                       el->GetIndex(),
                                                                       el->GetEffect(j));

                // Keep canvas mode if it was set as the effect is unlikely to work
                // properly without it
                for (const auto& it : oldSettings)
                {
                    //if (StartsWith(it.first, "B_") || StartsWith(it.first, "T_"))
                    //{
                    //    settings += "," + it.first + "=" + it.second;
                    //}
                    if (it.first == "T_CHECKBOX_Canvas")
                    {
                        settings += "," + it.first + "=" + it.second;
                    }
                }

                el->GetEffect(j)->SetSettings(settings, true);
                el->GetEffect(j)->SetEffectIndex(effectIndex);
                el->GetEffect(j)->SetEffectName(effectName);
                el->GetEffect(j)->SetPalette(palette);

                SetEffectControls(el->GetEffect(j)->GetParentEffectLayer()->GetParentElement()->GetFullName(),
                                  el->GetEffect(j)->GetEffectName(),
                                  el->GetEffect(j)->GetSettings(),
                                  el->GetEffect(j)->GetPaletteMap(),
                                  el->GetEffect(j)->GetStartTimeMS(),
                                  el->GetEffect(j)->GetEndTimeMS(),
                                  true);
                selectedEffectString = GetEffectTextFromWindows(selectedEffectPalette);
                selectedEffect = el->GetEffect(j);
                startms = std::min(startms, el->GetEffect(j)->GetStartTimeMS());
                endms = std::max(endms, el->GetEffect(j)->GetEndTimeMS());
            }
        }

        if(startms <= endms) {
            SetPlayStatus(PLAY_TYPE_EFFECT);
            playStartMS = -1;
            if (!_suspendRender) {
                RenderEffectForModel(element->GetModelName(), startms, endms);
            }
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
    
    PushTraceContext();
    AddTraceMessage("In OnEffectSettingsTimerTrigger");
    if (Notebook1->GetSelection() != NEWSEQUENCER) {
        PopTraceContext();
        return;
    }

    // grab a copy of the pointer in case user clicks off the effect
    Effect* eff = selectedEffect;

    // This should not be necessary but i have seen enough crashes where accessing the eff at this point bombs out that I want to check it is valid
    if (eff != nullptr)
    {
        if (!_sequenceElements.IsValidEffect(eff))
        {
            AddTraceMessage("Effect not null but when we checked validity it failed. THIS WOULD HAVE CRASHED.");
            wxASSERT(false);
            eff = nullptr;
            logger_base.error("OnEffectSettingsTimerTrigger went to use the selectedEffect but the pointer did not point to a valid effect!!!");
        }
    }

    if (eff != nullptr && timingPanel->BitmapButton_CheckBox_LayerMorph->IsEnabled()) {
        AddTraceMessage("Effect not null and enabled");

        std::string palette;
        std::string effectText = GetEffectTextFromWindows(palette);
        AddTraceMessage("Selected Effect text: " + selectedEffectString);
        AddTraceMessage("Effect text         : " + effectText);
        AddTraceMessage("Selected Effect palette: " + selectedEffectPalette);
        AddTraceMessage("Effect palette         : " + palette);
        if (effectText != selectedEffectString
            || palette != selectedEffectPalette
            || eff->GetEffectIndex() != EffectsPanel1->EffectChoicebook->GetSelection()) {

            AddTraceMessage("  Something changed");
            int effectIndex = EffectsPanel1->EffectChoicebook->GetSelection();
            wxString name = EffectsPanel1->EffectChoicebook->GetPageText(effectIndex);
            if (name !=  eff->GetEffectName()) {
                AddTraceMessage("  Effect name changed");
                eff->SetEffectName(name.ToStdString());
                eff->SetEffectIndex(EffectsPanel1->EffectChoicebook->GetSelection());
            }
            AddTraceMessage("  Effect name " + name);

            EffectLayer* el = eff->GetParentEffectLayer();

            // TEMPORARY - THIS SHOULD BE REMOVED BUT I WANT TO SEE WHAT IS CAUSING SOME RANDOM CRASHES - KW - 2017.7
            if (el == nullptr)
            {
                logger_base.crit("OnEffectSettingsTimerTrigger el is nullptr ... this is going to crash.");
                wxASSERT(false);
            }

            Element *elem = el->GetParentElement();

            // TEMPORARY - THIS SHOULD BE REMOVED BUT I WANT TO SEE WHAT IS CAUSING SOME RANDOM CRASHES - KW - 2017.7
            if (elem == nullptr)
            {
                logger_base.crit("OnEffectSettingsTimerTrigger elem is nullptr ... this is going to crash.");
                wxASSERT(false);
            }

            //check for undo capture
            if( selectedEffectName != eff->GetEffectName() )
            {
                _sequenceElements.get_undo_mgr().CreateUndoStep();
                _sequenceElements.get_undo_mgr().CaptureModifiedEffect( elem->GetModelName(), el->GetIndex(), eff->GetID(), selectedEffectString, selectedEffectPalette );
                AddTraceMessage("  Undo step created\n");
            }

            if (el->GetParentElement()->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
                eff->SetSettings(effectText, true);
                eff->SetPalette(palette);
                AddTraceMessage("  Effect settings updated");

                selectedEffectName = eff->GetEffectName();
                selectedEffectString = effectText;
                selectedEffectPalette = palette;
                AddTraceMessage("  SEN: " + selectedEffectName);
            }

            playStartTime = eff->GetStartTimeMS();
            playEndTime = eff->GetEndTimeMS();
            playStartMS = -1;

            // Update if effect has been modified
            if (m_mgr->GetPane("EffectAssist").IsShown()) {
                AddTraceMessage("  EffectAssist was shown");
                sEffectAssist->ForceRefresh();
                AddTraceMessage("   and refreshed");
            }

            if (!_suspendRender) {
                AddTraceMessage("  Triggering a render\n");
                RenderEffectForModel(elem->GetModelName(), playStartTime, playEndTime);
            }
            AddTraceMessage("  Triggering a refresh\n");
            mainSequencer->PanelEffectGrid->ForceRefresh();

            // This ensures colour curves which can be dependent on effect settings are correct
            RenderableEffect *ef = GetEffectManager().GetEffect(selectedEffectName);
            if (ef != nullptr) {
                AddTraceMessage("  Resetting color panel\n");
                colorPanel->SetSupports(ef->SupportsLinearColorCurves(eff->GetSettings()), ef->SupportsRadialColorCurves(eff->GetSettings()));
            }
        } else {
            AddTraceMessage("  Nothing changed");
        }
    }
    PopTraceContext();
}

int xLightsFrame::GetCurrentPlayTime()
{
    wxTimeSpan ts = wxDateTime::UNow() - starttime;
    long msec = ts.GetMilliseconds().ToLong();

    // record current time
    int curt = (playStartTime + msec - playStartMS);

    if (playType == PLAY_TYPE_MODEL) {

        if (CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->GetMedia() != nullptr && CurrentSeqXmlFile->GetMedia()->GetPlayingState() == MEDIAPLAYINGSTATE::PLAYING)
        {
            curt = CurrentSeqXmlFile->GetMedia()->Tell();
        }
    }

    return curt;
}

void xLightsFrame::SetPlayStatus(int status) {
    playType = status;
    if (playType != PLAY_TYPE_STOPPED) {
        StartOutputTimer();
        //printf("Timer started - SetPlayStatus %d\n", status);
    }
}
void xLightsFrame::StartOutputTimer() {
    GPURenderUtils::prioritizeGraphics(true);
    playCurFrame = -1;
    OutputTimer.Start(_seqData.FrameTime(), wxTIMER_CONTINUOUS);
}
void xLightsFrame::StopOutputTimer() {
    OutputTimer.Stop();
    playCurFrame = -1;
    GPURenderUtils::prioritizeGraphics(false);
}

#ifdef XL_DRAWING_WITH_METAL
static bool NeedToRenderFrame(wxMetalCanvas *w, const xLightsTimer& t, std::vector<bool> &didRender) {
    int i = w->getScreenIndex();
    if (i >= 0) {
        if (i >= didRender.size()) {
            didRender.resize(i + 1);
        }
        double f = t.presentTimeForScreen(i);
        if (f > 0) {
            bool b = w->startFrameForTime(f);
            if (!b) {
                return false;
            }
        }
        didRender[i] = true;
        return true;
    }
    return false;
}
#else
static bool NeedToRenderFrame(wxWindow *w, const xLightsTimer& t, std::vector<bool> &didRender) {
    didRender[0] = true;
    return true;
}
#endif

bool xLightsFrame::TimerRgbSeq(long msec)
{
    //static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // check if there are models that depend on timing tracks or similar that need to be rendered
    std::vector<Element *> elsToRender;
    if (_sequenceElements.GetElementsToRender(elsToRender)) {
        for (const auto& it : elsToRender) {
            int ss, es;
            it->GetDirtyRange(ss, es);
            if (!_suspendRender) {
                RenderEffectForModel(it->GetModelName(), ss, es);
            }
        }
    }

    // return if play is stopped
    if (playType == PLAY_TYPE_STOPPED || CurrentSeqXmlFile == nullptr) {
        return false;
    }

    // return if paused
    if (playType == PLAY_TYPE_EFFECT_PAUSED || playType == PLAY_TYPE_MODEL_PAUSED) {
        playStartMS = msec - playOffsetTime;  // maintain offset so we can restart where we paused
        return false;
    }

    // return if we have reset play times
    if (playEndTime == 0) {
        return false;
    }

    // capture start time if necessary
    if (playStartMS == -1) {
        playStartMS = msec;
        for (auto &f : fpsEvents) {
            f.clear();
        }
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
            return false;
        }
        return true;
    }

    int current_play_time = 0;
    if (playType == PLAY_TYPE_MODEL) {
        if (CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->GetMedia() != nullptr && CurrentSeqXmlFile->GetMedia()->GetPlayingState() == MEDIAPLAYINGSTATE::PLAYING) {
            current_play_time = CurrentSeqXmlFile->GetMedia()->Tell();
            curt = current_play_time;
        } else {
            current_play_time = curt;
        }

        // see if its time to stop model play
        if (curt >= playEndTime) {
            if (mLoopAudio) {
                DoPlaySequence();
                curt = playStartTime;
                current_play_time = curt;
                for (auto &f : fpsEvents) {
                    f.clear(); // we need to clear FPS data
                }
                // return true;
            } else {
                playStartTime = playEndTime = 0;
                playStartMS = -1;
                wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
                wxPostEvent(this, playEvent);
                //logger_base.debug("Stopping play");
                return true;
            }
        }
    }
    
    int frame = curt / _seqData.FrameTime();
    if (frame == playCurFrame) {
        // same frame as before, no need to update anything
        return true;
    }
    playCurFrame = frame;
    
    if (_outputManager.IsOutputting()) {
        _outputManager.StartFrame(msec);
    }
    std::vector<bool> didRender(8);
    if (frame < _seqData.NumFrames()) {
        //logger_base.debug("Outputting Frame %d", frame);
        // have the frame, copy from SeqData
        TimerOutput(frame);
        if (playModel != nullptr && NeedToRenderFrame(_modelPreviewPanel, OutputTimer, didRender)) {
            int nn = playModel->GetNodeCount();
            for (int node = 0; node < nn; node++) {
                int start = playModel->NodeStartChannel(node);
                wxASSERT(start < _seqData.NumChannels());
                playModel->SetNodeChannelValues(node, &_seqData[frame][start]);
            }
            _modelPreviewPanel->setCurrentFrameTime(curt);
            playModel->DisplayEffectOnWindow(_modelPreviewPanel, mPointSize);
        }
        if (NeedToRenderFrame(_housePreviewPanel->GetModelPreview(), OutputTimer, didRender)) {
            _housePreviewPanel->GetModelPreview()->Render(curt, &_seqData[frame][0]);
        }

        for (const auto& it : PreviewWindows) {
            if (it->GetActive() && NeedToRenderFrame(it, OutputTimer, didRender)) {
                it->Render(curt, &_seqData[frame][0]);
            }
        }
    }

    if (playType == PLAY_TYPE_MODEL) {
        wxASSERT(_seqData.FrameTime() != 0);
        if (NeedToRenderFrame(mainSequencer->PanelEffectGrid, OutputTimer, didRender)) {
            bool reRender = mainSequencer->UpdateTimeDisplay(current_play_time, _fps, false);
            if (reRender) {
                GRAPHICS_BASE_CLASS *td = (GRAPHICS_BASE_CLASS*)mainSequencer->timeDisplay;
                if (NeedToRenderFrame(td, OutputTimer, didRender)) {
                    td->render();
                }
            }
            if (mainSequencer->PanelTimeLine->SetPlayMarkerMS(current_play_time)) {
                if (NeedToRenderFrame(mainSequencer->PanelWaveForm, OutputTimer, didRender)) {
                    mainSequencer->PanelWaveForm->UpdatePlayMarker();
                    mainSequencer->PanelWaveForm->CheckNeedToScroll();
                    mainSequencer->PanelEffectGrid->ForceRefresh();
                }
                wxASSERT(CurrentSeqXmlFile->GetFrameMS() != 0);
                _housePreviewPanel->SetPositionFrames(current_play_time / CurrentSeqXmlFile->GetFrameMS());
            }
        }
        sequenceVideoPanel->UpdateVideo( current_play_time );
    }
// #define DO_FPS
#ifdef DO_FPS
    for (int x = 0; x < didRender.size(); x++) {
        if (didRender[x]) {
            if (x >= _fps.size()) {
                fpsEvents.resize(x + 1);
                _fps.resize(x + 1);
            }
            fpsEvents[x].push_back(FPSEvent(frame));
            size_t fpsSize = fpsEvents[x].size();
            while (fpsSize > (2000 / _seqData.FrameTime())) {
                fpsEvents[x].pop_front();
                fpsSize--;
            }
            if (!fpsEvents[x].empty()) {
                FPSEvent b = fpsEvents[x].front();
                FPSEvent e = fpsEvents[x].back();
                if (e.when == b.when) {
                    _fps[x] = 0;
                } else {
                    _fps[x] = (float)((double)(fpsSize-1) * 1000.0) / ((e.when - b.when).GetMilliseconds().ToDouble());
                }
            }

        }
    }
#endif
    if (_outputManager.IsOutputting()) {
        _outputManager.EndFrame();
    }
    return true;
}

void xLightsFrame::SetEffectControls(const std::string &modelName, const std::string &effectName,
                                     const SettingsMap &settings, const SettingsMap &palette,
                                     int startTimeMs, int endTimeMs, bool setDefaults) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (CurrentSeqXmlFile == nullptr) return;
    //timingPanel->Freeze();
    //bufferPanel->Freeze();
    //colorPanel->Freeze();
    SetChoicebook(EffectsPanel1->EffectChoicebook, effectName);
    //wxWindow* p = EffectsPanel1->GetSelectedPanel();
    //p->Freeze();
    Model *model = GetModel(modelName);
    if (setDefaults) {
        ResetPanelDefaultSettings(effectName, model, false);
    }

    EffectsPanel1->SetEffectPanelStatus(model, effectName, startTimeMs, endTimeMs);
    SetEffectControls(settings);
    SetEffectControls(palette);
    RenderableEffect *ef = GetEffectManager().GetEffect(effectName);
    if (ef != nullptr) {
        colorPanel->SetColorCount(ef->GetColorSupportedCount());
        colorPanel->SetSupports(ef->SupportsLinearColorCurves(settings), ef->SupportsRadialColorCurves(settings));
    } else {
        colorPanel->SetColorCount(8);
        logger_base.warn("Setting effect controls for unknown effect type: %s", (const char *)effectName.c_str());
    }
    //p->Thaw();
    //timingPanel->Thaw();
    //bufferPanel->Thaw();
    //colorPanel->Thaw();
}

bool xLightsFrame::ApplySetting(wxString name, const wxString &value, int count)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool res = true;
    auto orig = name;
    wxWindow* ContextWin = nullptr;
	if (name.StartsWith("E_")) {
		ContextWin = EffectsPanel1;
	} else if (name.StartsWith("T_")) {
        // Layers selected is not stored in a control so we handle it here
        if (name == "T_LayersSelected") {
            timingPanel->SetLayersSelected(value.ToStdString());
            return res;
        }

	    if (name == "T_CHECKBOX_OverlayBkg") {
			//temporary until this key is remapped
			ContextWin = bufferPanel;
		} else {
			ContextWin = timingPanel;
		}
	} else if (name.StartsWith("B_")) {
	    ContextWin = bufferPanel;
	} else if (name.StartsWith("C_")) {
		ContextWin = colorPanel;
	} else if (name.StartsWith("X_")) {
        // This is used for properties that are not displayed on a panel ... but are typically accessed via the right click menu on an effect
        return res;
    } else {
        logger_base.error("ApplySetting: Unable to panel type for: %s", (const char*)name.c_str());
        return false;
	}

    name = "ID_" + name.Mid(2);
	wxWindow *CtrlWin = wxWindow::FindWindowByName(name, ContextWin);

    if (CtrlWin != nullptr) {
		if (name.StartsWith("ID_SLIDER")) {
			wxSlider* ctrl = (wxSlider*)CtrlWin;
            long tempLong;
			if (value.ToLong(&tempLong)) {
				ctrl->SetValue(tempLong);

				wxScrollEvent event(wxEVT_SLIDER, ctrl->GetId());
				event.SetEventObject(ctrl);
				event.SetInt(tempLong);
				ctrl->ProcessWindowEvent(event);
			}
        } else if (name.StartsWith("ID_TEXTCTRL")) {
			wxTextCtrl* ctrl = dynamic_cast<wxTextCtrl*>(CtrlWin);
            if (ctrl != nullptr) {
                ctrl->SetValue(value);
            } else {
                // some text ctrls have been replace with combo boxes ... maybe this is one of those
                wxComboBox* ctrl = dynamic_cast<wxComboBox*>(CtrlWin);
                if (ctrl != nullptr) {
                    ctrl->SetValue(value);
                } else {
                    wxASSERT(false);
                }
            }
		} else if (name.StartsWith("ID_SPINCTRL")) {
			wxSpinCtrl* ctrl = (wxSpinCtrl*)CtrlWin;
			ctrl->SetValue(wxAtoi(value));
		} else if (name.StartsWith("ID_CHOICE")) {
			wxString nn = "IDD_RADIOBUTTON" + name.SubString(9, name.size());
			wxRadioButton *b = (wxRadioButton*)wxWindow::FindWindowByName(nn, ContextWin);
			if (b != nullptr) {
				b->SetValue(true);
				wxCommandEvent evt(wxEVT_RADIOBUTTON, b->GetId());
				evt.SetEventObject(b);
                b->ProcessWindowEvent(evt); // dont post ... if we post it gets processed in the wrong order
			}

			wxChoice* ctrl = (wxChoice*)CtrlWin;
            if (!ctrl->SetStringSelection(value) && value.StartsWith("**")) {
                ctrl->Append(value);
                ctrl->SetStringSelection(value);
            }
            if (ctrl->GetStringSelection() != value && count < 10) {
                // if did not take ... possibly because it has not loaded the values yet
                // If it doesn't take after 10 attempts, the "value" is not in the list of possible values
                // so we'll just use whatever the default/last value is to prevent an infinite loop
                // waiting for the value to to be added
                wxCommandEvent event(EVT_SETEFFECTCHOICE);
                event.SetString(orig + "|" + value);
                event.SetInt(count + 1);
                wxPostEvent(this, event);
            } else {
                wxCommandEvent event(wxEVT_CHOICE, ctrl->GetId());
                event.SetEventObject(ctrl);
                event.SetString(ctrl->GetStringSelection());
                ctrl->ProcessWindowEvent(event);
            }
        } else if (name.StartsWith("ID_BUTTON")) {
            if (name.StartsWith("ID_BUTTON_Palette")) {
                colorPanel->SetButtonColor((ColorCurveButton*)CtrlWin, value.ToStdString());
            }
		} else if (name.StartsWith("ID_CHECKBOX")) {
			wxCheckBox* ctrl = (wxCheckBox*)CtrlWin;
            long tempLong;
			if (value.ToLong(&tempLong)) {
				ctrl->SetValue(tempLong != 0);
				wxCommandEvent evt(wxEVT_COMMAND_CHECKBOX_CLICKED, ctrl->GetId());
				evt.SetEventObject(ctrl);
				evt.SetInt(tempLong != 0);
				ctrl->ProcessWindowEvent(evt);
			}
		} else if (name.StartsWith("ID_NOTEBOOK")) {
			wxNotebook* ctrl = (wxNotebook*)CtrlWin;
			for (size_t z = 0; z < ctrl->GetPageCount(); z++) {
				if (value == ctrl->GetPageText(z)) {
					ctrl->SetSelection(z);
				}
			}
		} else if (name.StartsWith("ID_FILEPICKER") || name.StartsWith("ID_0FILEPICKER")) {
			wxFilePickerCtrl *picker = (wxFilePickerCtrl*)CtrlWin;
			picker->SetFileName(value);

			wxFileDirPickerEvent evt(wxEVT_FILEPICKER_CHANGED, picker, picker->GetId(), value);
			evt.SetEventObject(picker);
			picker->ProcessWindowEvent(evt);
		} else if (name.StartsWith("ID_FONTPICKER")) {
			wxFontPickerCtrl *picker = (wxFontPickerCtrl*)CtrlWin;
			wxFont oldfont;
			oldfont.SetNativeFontInfoUserDesc(value);
			picker->SetSelectedFont(oldfont);
		} else if (name.StartsWith("ID_COLOURPICKER")) {
            wxColourPickerCtrl* picker = (wxColourPickerCtrl*)CtrlWin;
            wxColour c(value);
            picker->SetColour(c);
        } else if (name.StartsWith("ID_CUSTOM")) {
            xlCustomControl *custom = dynamic_cast<xlCustomControl *>(CtrlWin);
            custom->SetValue(value.ToStdString());
        } else if (name.StartsWith("ID_VALUECURVE")) {
            ValueCurveButton *vcb = dynamic_cast<ValueCurveButton *>(CtrlWin);
            vcb->SetValue(value.ToStdString());
        } else if (name.StartsWith("ID_TOGGLEBUTTON")) {
            wxToggleButton *vcb = dynamic_cast<wxToggleButton *>(CtrlWin);
            vcb->SetValue(wxAtoi(value) != 0);
        } else {
			logger_base.error("ApplySetting: Unknown type: %s", (const char*)name.c_str());
            res = false;
            wxASSERT(false);
        }
    } else {
		if (name.StartsWith("ID_")) {
			//check if the control has been renamed to be ignored
			wxString nn = "IDD_" + name.SubString(3, name.size());
			CtrlWin = wxWindow::FindWindowByName(nn, ContextWin);
		}
		if (CtrlWin == nullptr) {
            logger_base.error("ApplySetting: Unable to find: %s", (const char*)name.c_str());
            res = false;
            wxASSERT(false);
        }
	}
    return res;
}

void xLightsFrame::SetEffectChoice(wxCommandEvent& event)
{
    auto v = wxSplit(event.GetString(), '|');
    if (v.size() == 2) {
        ApplySetting(v[0], v[1], event.GetInt());
    } else {
        wxASSERT(false);
    }
}

void xLightsFrame::TipOfDayReady(wxCommandEvent& event)
{
    // only show tip of the day if show directory is set
    if (CurrentDir != "") {
        // at this point if we are downloading tip of day content then the tip of day content is downloaded and ready to go
#ifdef __WXMSW__
        _tod.SetTODXMLFile(event.GetString());
        _tod.DoTipOfDay(false);
#else
        _tod->SetTODXMLFile(event.GetString());
        _tod->DoTipOfDay(false);
#endif
    }
}

void xLightsFrame::SetEffectDuration(wxCommandEvent& event)
{
    mainSequencer->SetEffectDuration(event.GetString(), event.GetInt());
}

void xLightsFrame::ApplyLast(wxCommandEvent& event)
{
    // Handle the EVT_APPLYLAST event
    SettingsMap* pSettingsMap = (SettingsMap*)event.GetClientData();
    SetEffectControlsApplyLast(*pSettingsMap);
    delete pSettingsMap;

    ValidatePanels();
}

void xLightsFrame::SetEffectControlsApplyLast(const SettingsMap &settings) {
    // Now Apply those settings with APPLYLAST in their name ... last
    for (const auto& it : settings) {
        if (it.first.find("APPLYLAST") != std::string::npos)
        {
            ApplySetting(wxString(it.first.c_str()), ToWXString(it.second));
        }
    }

    ValidatePanels();
}

void xLightsFrame::ResetPanelDefaultSettings(const std::string& effect, const Model* model, bool optionbased)
{
    SetChoicebook(EffectsPanel1->EffectChoicebook, effect);
    timingPanel->SetDefaultControls(model, optionbased);
    bufferPanel->SetDefaultControls(model, optionbased);
    colorPanel->SetDefaultSettings(optionbased);

    // do the effect setting last as it may want to override some of the above
    // this should be used sparingly ...
    EffectsPanel1->SetDefaultEffectValues(effect);
}

void xLightsFrame::SetEffectControls(const SettingsMap &settings) {

    bool applylast = false;

	// Apply those settings without APPLYLAST in their name first
    for (const auto& it : settings) {
		if (it.first.find("APPLYLAST") == std::string::npos) {
			ApplySetting(it.first, ToWXString(it.second));
        } else {
            applylast = true;
        }
    }

    MixTypeChanged = true;
    FadesChanged = true;

    if (applylast) {
        // we do this asynchronously as we tyically need other events to process first
        wxCommandEvent event(EVT_APPLYLAST);
        event.SetClientData(new SettingsMap(settings));
        wxPostEvent(this, event);
    } else {
        ValidatePanels();
    }
}

void xLightsFrame::ValidatePanels()
{
    timingPanel->ValidateWindow();
    bufferPanel->ValidateWindow();
    colorPanel->ValidateWindow();
    wxCommandEvent e(EVT_VALIDATEWINDOW);
    wxPostEvent(EffectsPanel1->GetSelectedPanel(), e);
}

std::string xLightsFrame::GetEffectTextFromWindows(std::string &palette) const
{
    RenderableEffect *eff = effectManager[EffectsPanel1->EffectChoicebook->GetSelection()];
    if (eff == nullptr) {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.crit("xLightsFrame::GetEffectTextFromWindows eff returned nullptr for effect %d. This is going to crash.", EffectsPanel1->EffectChoicebook->GetSelection());
    }
    wxString effectText = eff->GetEffectString();
    if (effectText.size() > 0 && effectText[effectText.size()-1] != ',') {
        effectText += ",";
    }
    effectText += timingPanel->GetTimingString();
    effectText += bufferPanel->GetBufferString();
    palette = colorPanel->GetColorString();
    return ToStdString(effectText);
}

void xLightsFrame::ForceSequencerRefresh(wxCommandEvent& event)
{
    DoForceSequencerRefresh();
}

void xLightsFrame::DoForceSequencerRefresh()
{
    _sequenceElements.PopulateRowInformation();
    ResizeMainSequencer();
}

void xLightsFrame::DoLoadPerspective(wxXmlNode *perspective)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (perspective == nullptr) {
        logger_base.warn("xLightsFrame::LoadPerspective Null perspective node.");
        return;
    }
    if (PerspectivesNode == nullptr) {
        logger_base.warn("xLightsFrame::LoadPerspective Null PerspectivesNode.");
        return;
    }

    wxString name = perspective->GetAttribute("name");
    wxString settings = perspective->GetAttribute("settings");
    if (name != PerspectivesNode->GetAttribute("current")) {
        PerspectivesNode->DeleteAttribute("current");
        PerspectivesNode->AddAttribute("current", name);
        UnsavedRgbEffectsChanges = true;
        mCurrentPerpective = perspective;
    }
    if (settings.size() == 0) {
        settings = m_mgr->SavePerspective();
        mCurrentPerpective->DeleteAttribute("settings");
        mCurrentPerpective->AddAttribute("settings", settings);
        mCurrentPerpective->AddAttribute("version", "2.0");
        logger_base.debug("Saved perspective.");
        LogPerspective(settings);
    }
    logger_base.debug("Loading perspective %s", (const char *)name.c_str());
    PushTraceContext();
    AddTraceMessage(settings);
    LogPerspective(settings);
    m_mgr->LoadPerspective(settings, true);
    PopTraceContext();

    //perspectives may have been saved without the maximize button flag, we'll
    //make sure it's turned on.  Make sure Dockable state matches menu options/configuration
    m_mgr->GetPane("ModelPreview").MaximizeButton(true).Dockable(IsDockable("MP"));
    m_mgr->GetPane("HousePreview").MaximizeButton(true).Dockable(IsDockable("HP"));
    m_mgr->GetPane("DisplayElements").MaximizeButton(true);

    ShowHideAllSequencerWindows(true);

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
        wxString p = m_mgr->SavePerspective();
        perspective->AddAttribute("settings", p);
        logger_base.debug("Saved perspective.");
        LogPerspective(p);
    } else {
        _modelPreviewPanel->Refresh(false);
        _housePreviewPanel->Refresh(false);
        m_mgr->Update();
    }

    if (tempEffectAssistMode == EFFECT_ASSIST_ALWAYS_OFF) {
        SetEffectAssistWindowState(false);
    } else if (tempEffectAssistMode == EFFECT_ASSIST_ALWAYS_ON) {
        bool visible = m_mgr->GetPane("EffectAssist").IsShown();
        if (!visible) {
            tempEffectAssistMode = EFFECT_ASSIST_NOT_IN_PERSPECTIVE;
        }
    }

    for (int i = 0; i < 10; i++) {
        if (perspectives[i].p == perspective) {
            MenuItemPerspectives->Check(perspectives[i].id, true);
        }
    }
    UpdateLayoutSave();
    UpdateControllerSave();
    UpdateViewMenu();
}

void xLightsFrame::LoadPerspective(wxCommandEvent& event)
{
    wxXmlNode* perspective = (wxXmlNode*)(event.GetClientData());
    mCurrentPerpective = perspective;
    DoLoadPerspective(mCurrentPerpective);
}

void xLightsFrame::OnMenuItemViewSavePerspectiveSelected(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (mCurrentPerpective != nullptr)
    {
        wxMessageDialog confirm(this, _("Are you sure you want to save the current view as perpective \"") + mCurrentPerpective->GetAttribute("name") + "\"?", _("Confirm"), wxYES | wxNO);
        if (confirm.ShowModal() == wxID_YES)
        {
            if (mCurrentPerpective->HasAttribute("settings"))
            {
                mCurrentPerpective->DeleteAttribute("settings");
            }
            wxString p = m_mgr->SavePerspective();
            mCurrentPerpective->AddAttribute("settings", p);
            mCurrentPerpective->DeleteAttribute("version");
            mCurrentPerpective->AddAttribute("version", "2.0");
            logger_base.debug("Saved perspective.");
            LogPerspective(p);
            SaveEffectsFile();
        }
    }
}

void xLightsFrame::OnMenuItemViewSaveAsPerspectiveSelected(wxCommandEvent& event)
{
    wxString name = wxGetTextFromUser("Enter name of perspective", "Perspective Name");
    if (name.size() > 0) {
        for (wxXmlNode* p = PerspectivesNode->GetChildren(); p != nullptr; p = p->GetNext())
        {
            if (p->GetName() == "perspective")
            {
                wxString check_name = p->GetAttribute("name");
                if (check_name == name)
                {
                    int answer = wxMessageBox("Enter new name?", "Duplicate Name", wxYES_NO);
                    if (answer == wxYES) {
                        OnMenuItemViewSaveAsPerspectiveSelected(event);
                    }
                    return;
                }
            }
        }

        wxXmlNode* p = new wxXmlNode(wxXML_ELEMENT_NODE, "perspective");
        p->AddAttribute("name", name);
        p->AddAttribute("settings", "");
        PerspectivesNode->AddChild(p);
        mCurrentPerpective = p;
        OnMenuItemViewSavePerspectiveSelected(event);
        PerspectivesChanged(event);
        DoLoadPerspective(mCurrentPerpective);
        wxCommandEvent eventPerspectivesChanged(EVT_PERSPECTIVES_CHANGED);
        wxPostEvent(this, eventPerspectivesChanged);
    }
}

void xLightsFrame::TurnOnOutputToLights(wxCommandEvent& event)
{
    EnableOutputs(true);
}

void xLightsFrame::PlayJukeboxItem(wxCommandEvent& event)
{
    jukeboxPanel->PlayItem(event.GetInt());
}

void xLightsFrame::PerspectivesChanged(wxCommandEvent& event)
{
    LoadPerspectivesMenu(PerspectivesNode);
    UnsavedRgbEffectsChanges = true;
    UpdateLayoutSave();
    UpdateControllerSave();
    UpdateViewMenu();
}

void xLightsFrame::ShowDisplayElements(wxCommandEvent& event)
{
    displayElementsPanel->Initialize();
    wxAuiPaneInfo & info = m_mgr->GetPane("DisplayElements");
    info.BestSize(wxSize(750, 1050));
    int w, h;
    displayElementsPanel->GetSize(&w, &h);
    info.FloatingSize(std::max(750, w), std::max(1050, h));
    info.Show();
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::ShowHideSelectEffectsWindow(wxCommandEvent& event)
{
    InitSequencer();
    bool visible = m_mgr->GetPane("SelectEffect").IsShown();
    if (visible) {
        m_mgr->GetPane("SelectEffect").Hide();
    } else {
        m_mgr->GetPane("SelectEffect").Show();
    }
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::OnMenuDockAllSelected(wxCommandEvent& event)
{
    ResetAllSequencerWindows();
}

void xLightsFrame::ShowHideBufferSettingsWindow(wxCommandEvent& event)
{
    InitSequencer();

    bool visible = m_mgr->GetPane("LayerSettings").IsShown();
    if (visible) {
        m_mgr->GetPane("LayerSettings").Hide();
    } else {
        m_mgr->GetPane("LayerSettings").Show();
    }
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::ShowHideDisplayElementsWindow(wxCommandEvent& event)
{
    InitSequencer();

    wxAuiPaneInfo& info = m_mgr->GetPane("DisplayElements");

    if (!info.IsOk()) return;

    bool visible = info.IsShown();
    if (visible) {
        info.Hide();
    } else {
        info.BestSize(wxSize(750, 1050));
        int w, h;
        displayElementsPanel->GetSize(&w, &h);
        info.FloatingSize(std::max(750, w), std::max(1050, h));
        info.Show();
    }
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::ShowHideEffectSettingsWindow(wxCommandEvent& event)
{
    InitSequencer();
    bool visible = m_mgr->GetPane("Effect").IsShown();
    if (visible) {
        m_mgr->GetPane("Effect").Hide();
    } else {
        m_mgr->GetPane("Effect").Show();
    }
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::ShowHideColorWindow(wxCommandEvent& event)
{
    InitSequencer();
    bool visible = m_mgr->GetPane("Color").IsShown();
    if (visible) {
        m_mgr->GetPane("Color").Hide();
    } else {
        m_mgr->GetPane("Color").Show();
    }
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::ShowHideLayerTimingWindow(wxCommandEvent& event)
{
    InitSequencer();
    bool visible = m_mgr->GetPane("LayerTiming").IsShown();
    if (visible) {
        m_mgr->GetPane("LayerTiming").Hide();
    } else {
        m_mgr->GetPane("LayerTiming").Show();
    }
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::ShowHideModelPreview(wxCommandEvent& event)
{
    InitSequencer();
    bool visible = m_mgr->GetPane("ModelPreview").IsShown();
    if (visible) {
        m_mgr->GetPane("ModelPreview").Hide();
    } else {
        m_mgr->GetPane("ModelPreview").Show();
    }
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::ShowHideHousePreview(wxCommandEvent& event)
{
    InitSequencer();
    bool visible = m_mgr->GetPane("HousePreview").IsShown();
    if (visible) {
        m_mgr->GetPane("HousePreview").Hide();
    } else {
        m_mgr->GetPane("HousePreview").Show();
    }
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::ShowHideEffectDropper(wxCommandEvent& event)
{
    InitSequencer();
    bool visible = m_mgr->GetPane("EffectDropper").IsShown();
    if (visible) {
        m_mgr->GetPane("EffectDropper").Hide();
    } else {
        m_mgr->GetPane("EffectDropper").Show();
    }
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::ShowHidePerspectivesWindow(wxCommandEvent& event)
{
    InitSequencer();
    bool visible = m_mgr->GetPane("Perspectives").IsShown();
    if (visible) {
        m_mgr->GetPane("Perspectives").Hide();
    } else {
        m_mgr->GetPane("Perspectives").Show();
    }
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::ShowHideEffectAssistWindow(wxCommandEvent& event)
{
    InitSequencer();
    bool visible = m_mgr->GetPane("EffectAssist").IsShown();
    if (visible) {
        m_mgr->GetPane("EffectAssist").Hide();
        // Dont set it permanently
        //mEffectAssistMode = EFFECT_ASSIST_ALWAYS_OFF;
        tempEffectAssistMode = EFFECT_ASSIST_ALWAYS_OFF;
    } else {
        m_mgr->GetPane("EffectAssist").Show();
        // Dont set it permanently
        // mEffectAssistMode = EFFECT_ASSIST_ALWAYS_ON;
        tempEffectAssistMode = EFFECT_ASSIST_ALWAYS_ON;
    }
    m_mgr->Update();
    UpdateViewMenu();
}

TimingElement* xLightsFrame::AddTimingElement(const std::string& name, const std::string &subType)
{
    std::string n = RemoveUnsafeXmlChars(name);
    int nn = 1;

    // make sure the name is unique ... adding a sequence number if it isnt
    while (_sequenceElements.GetElement(n) != nullptr) {
        n = wxString::Format("%s_%d", name.c_str(), nn++).ToStdString();
    }

    // Deactivate active timing mark so new one is selected;
    _sequenceElements.DeactivateAllTimingElements();
    int timingCount = _sequenceElements.GetNumberOfTimingElements();
    std::string type = "timing";
    TimingElement* e = dynamic_cast<TimingElement*>(_sequenceElements.AddElement(timingCount, n, type, true, false, true, false, false));
    e->SetSubType(subType);
    e->AddEffectLayer();
    _sequenceElements.AddTimingToCurrentView(n);
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(this, eventRowHeaderChanged);
    return e;
}

void xLightsFrame::DeleteTimingElement(const std::string& name)
{
    displayElementsPanel->RemoveModelFromLists(name);
    _sequenceElements.DeleteElement(name);
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(this, eventRowHeaderChanged);
}

void xLightsFrame::RenameTimingElement(const std::string& old_name, const std::string& new_name)
{
    Element* element = _sequenceElements.GetElement(old_name);
    if( element ) element->SetName(new_name);
    _sequenceElements.RenameTimingTrack(old_name, new_name);
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

std::map<int, std::vector<float>> xLightsFrame::LoadPolyphonicTranscription(AudioManager* audio, int intervalMS)
{
    static log4cpp::Category &logger_pianodata = log4cpp::Category::getInstance(std::string("log_pianodata"));
    std::map<int, std::vector<float>> res;

    if (audio != nullptr) {
        try {
            if (!audio->IsPolyphonicTranscriptionDone()) {
                wxProgressDialog pd("Processing Audio", "");
                logger_pianodata.info("Processing Polyphonic Transcription to produce notes");
                audio->DoPolyphonicTranscription(&pd, &PTProgress);
                logger_pianodata.info("Processing Polyphonic Transcription - DONE");
            }
        } catch (...) {
            logger_pianodata.warn("Exception caught processing Polyphonic Transcription");
        }

        logger_pianodata.debug("Interval %d.", intervalMS);

        int frames = audio->LengthMS() / intervalMS;

        for (size_t i = 0; i < frames; i++) {
            auto pdata = audio->GetFrameData(i, "", true);
            if (pdata != nullptr) {
                res[i*intervalMS] = pdata->notes;
            }
        }

        if (logger_pianodata.isDebugEnabled()) {
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

std::map<int, std::vector<float>> xLightsFrame::LoadAudacityFile(std::string file, int intervalMS)
{
    static log4cpp::Category &logger_pianodata = log4cpp::Category::getInstance(std::string("log_pianodata"));
    std::map<int, std::vector<float>> res;

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
                            std::vector<float> ff;
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

std::map<int, std::vector<float>> xLightsFrame::LoadMusicXMLFile(std::string file, int intervalMS, int speedAdjust, int startAdjustMS, std::string track)
{
    static log4cpp::Category &logger_pianodata = log4cpp::Category::getInstance(std::string("log_pianodata"));
    std::map<int, std::vector<float>> res;

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
                            std::vector<float> ff;
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

std::map<int, std::vector<float>> xLightsFrame::LoadMIDIFile(std::string file, int intervalMS, int speedAdjust, int startAdjustMS, std::string track)
{
    static log4cpp::Category& logger_pianodata = log4cpp::Category::getInstance(std::string("log_pianodata"));
    std::map<int, std::vector<float>> res;

    float speedadjust = speedAdjust / 100.0;

    int notestate[128] = {0};
//    for (int i = 0; i <= 127; ++i) {
//        notestate[i] = 0;
//    }

    logger_pianodata.debug("Processing midi file " + file);
    logger_pianodata.debug("Interval %d.", intervalMS);
    logger_pianodata.debug("SpeedAdjust %d.", speedAdjust);
    logger_pianodata.debug("StartAdjustMS %d.", startAdjustMS);

    MidiFile midifile;
    float lasttime = -1;
    if (midifile.read(file) != 0) {
        int ntrack = 0;
        if (track == "All" || track == "") {
            midifile.joinTracks();
        } else {
            ntrack = wxAtoi(track) - 1;
            if (ntrack >= midifile.getNumTracks()) {
                ntrack = 0;
            }
        }
        midifile.doTimeAnalysis();

        logger_pianodata.debug("Processing midi track %d.", ntrack);
        logger_pianodata.debug("Event,time(s)->ms,adjustedtime(s)->ms,isnote,isnoteon,isnoteoff,midinote");

        // process each event
        for (int i = 0; i < midifile.getNumEvents(ntrack); ++i) {
            MidiEvent e = midifile.getEvent(ntrack, i);

            if (e.isNote()) {
                float time = (float)startAdjustMS / 100.0 + midifile.getTimeInSeconds(ntrack, i) * speedadjust;

                if (logger_pianodata.isDebugEnabled()) {
                    logger_pianodata.debug("%d,%f->%d,%f->%d,%d,%d,%d,%d", i, midifile.getTimeInSeconds(ntrack, i), LowerTS(midifile.getTimeInSeconds(ntrack, i), intervalMS), time, LowerTS(time, intervalMS), e.isNote(), e.isNoteOn(), e.isNoteOff(), e.getKeyNumber());
                }
                if (time != lasttime) {
                    if (lasttime >= 0) {
                        // we can update things now
                        int start = LowerTS(lasttime, intervalMS);
                        int end = UpperTS(time, intervalMS);

                        for (int j = start; j < end; j += intervalMS) {
                            std::vector<float> f;
                            for (int k = 0; k <= 127; ++k) {
                                if (notestate[k] > 0) {
                                    f.push_back(k);
                                }
                            }
                            res[j] = f;
                        }
                    }

                    lasttime = time;
                }

                if (e.isNoteOn()) {
                    notestate[e.getKeyNumber()]++;
                } else if (e.isNoteOff()) {
                    notestate[e.getKeyNumber()]--;
                    if (notestate[e.getKeyNumber()] < 0) {
                        // this should never happen
                    }
                }
            }
        }

        if (logger_pianodata.isDebugEnabled()) {
            logger_pianodata.debug("Note data calculated:");
            logger_pianodata.debug("Time MS, Keys");
            for (auto it = res.begin(); it != res.end(); ++it) {
                std::string keys = "";
                for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
                    keys += " " + std::string(wxString::Format("%f", *it2).c_str());
                }
                logger_pianodata.debug("%d,%s", it->first, (const char*)keys.c_str());
            }
        }
    } else {
        logger_pianodata.warn("Invalid MIDI file " + file);
    }

    return res;
}

void xLightsFrame::ExecuteImportNotes(wxCommandEvent& command)
{
    NoteImportDialog dlgNoteImport(this, _sequenceElements, (CurrentSeqXmlFile->GetMedia() != nullptr));

    if (dlgNoteImport.ShowModal() == wxID_OK) {
        wxString name = dlgNoteImport.TextCtrl_TimingName->GetValue();
        _sequenceElements.DeactivateAllTimingElements();
        Element* element = AddTimingElement(std::string(name.ToStdString()));
        EffectLayer* effectLayer = element->GetEffectLayer(0);
        // _sequenceElements.AddTimingToCurrentView(name.ToStdString()); I dont think this is necessary

        int interval = CurrentSeqXmlFile->GetFrameMS();
        wxString type = dlgNoteImport.Choice_Piano_Notes_Source->GetStringSelection();
        std::map<int, std::vector<float>> notes;
        if (type == "Audacity Timing File") {
            notes = LoadAudacityFile(dlgNoteImport.TextCtrl_Piano_File->GetValue().ToStdString(), interval);
        } else if (type == "Music XML File") {
            notes = LoadMusicXMLFile(dlgNoteImport.TextCtrl_Piano_File->GetValue().ToStdString(), interval, dlgNoteImport.Slider_Piano_MIDI_Speed->GetValue(), dlgNoteImport.Slider_Piano_MIDI_Start->GetValue(), dlgNoteImport.Choice_Piano_MIDITrack_APPLYLAST->GetStringSelection().ToStdString());
        } else if (type == "MIDI File") {
            notes = LoadMIDIFile(dlgNoteImport.TextCtrl_Piano_File->GetValue().ToStdString(), interval, dlgNoteImport.Slider_Piano_MIDI_Speed->GetValue(), dlgNoteImport.Slider_Piano_MIDI_Start->GetValue(), dlgNoteImport.Choice_Piano_MIDITrack_APPLYLAST->GetStringSelection().ToStdString());
        } else if (type == "Polyphonic Transcription") {
            notes = LoadPolyphonicTranscription(CurrentSeqXmlFile->GetMedia(), interval);
        }

        CreateNotes(effectLayer, notes, interval, CurrentSeqXmlFile->GetSequenceDurationMS() / interval);

        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(this, eventRowHeaderChanged);
    }
}

std::string xLightsFrame::CreateNotesLabel(const std::vector<float>& notes) const
{
    std::string res;

    for (auto f : notes) {
        if (res != "") {
            res += ",";
        }
        res += DecodeMidi(f);
    }

    return res;
}

void xLightsFrame::CreateNotes(EffectLayer* el, std::map<int, std::vector<float>>& notes, int interval, int frames)
{
    size_t last = 0;
    std::string lastLabel = "";
    for (size_t i = 0; i <= frames; ++i) {
        std::string label = "";
        if (notes.find((float)(i * interval)) != notes.end()) {
            label = CreateNotesLabel(notes[i * interval]);
        }

        if (label == lastLabel) {
            // do nothing
        } else {
            if (i != 0) {
                el->AddEffect(0, lastLabel, "", "", last * interval, i * interval, EFFECT_NOT_SELECTED, false);
            }
            last = i;
            lastLabel = label;
        }
    }
    el->AddEffect(0, lastLabel, "", "", last * interval, frames * interval, EFFECT_NOT_SELECTED, false);
}

void xLightsFrame::ExecuteImportTimingElement(wxCommandEvent& command)
{
    ImportTimingElement();
}

void xLightsFrame::ImportTimingElement()
{
    wxFileDialog* OpenDialog = new wxFileDialog(this, "Choose Timing file(s)", wxEmptyString, wxEmptyString,
                                                "Timing files (*.xtiming)|*.xtiming|Papagayo files (*.pgo)|*.pgo|Subrip Subtitle File (*.srt)|*.srt|Text files (*.txt)|*.txt|Vixen 3 (*.tim)|*.tim|LOR (*.lms)|*.lms|LOR (*.las)|*.las|LSP (*.msq)|*.msq|xLights (*.xsq)|*.xsq|Old xLights (*.xml)|*.xml",
                                                wxFD_OPEN | wxFD_MULTIPLE, wxDefaultPosition);
    wxString fDir;
    if (OpenDialog->ShowModal() == wxID_OK) {
        fDir = OpenDialog->GetDirectory();
        wxArrayString filenames;
        OpenDialog->GetFilenames(filenames);
        if (filenames.size() > 0) {
            wxFileName file1(filenames[0]);
            if (file1.GetExt().Lower() == "lms" || file1.GetExt().Lower() == "las") {
                CurrentSeqXmlFile->ProcessLorTiming(fDir, filenames, this);
            } else if (file1.GetExt().Lower() == "xtiming") {
                CurrentSeqXmlFile->ProcessXTiming(fDir, filenames, this);
            } else if (file1.GetExt().Lower() == "pgo") {
                CurrentSeqXmlFile->ProcessPapagayo(fDir, filenames, this);
            } else if (file1.GetExt().Lower() == "srt") {
                CurrentSeqXmlFile->ProcessSRT(fDir, filenames, this);
            } else if (file1.GetExt().Lower() == "msq") {
                CurrentSeqXmlFile->ProcessLSPTiming(fDir, filenames, this);
            } else if (file1.GetExt().Lower() == "xml" || file1.GetExt().Lower() == "xsq") {
                CurrentSeqXmlFile->ProcessXLightsTiming(fDir, filenames, this);
            } else if (file1.GetExt().Lower() == "tim") {
                CurrentSeqXmlFile->ProcessVixen3Timing(fDir, filenames, this);
            } else {
                CurrentSeqXmlFile->ProcessAudacityTimingFiles(fDir, filenames, this);
            }
        }
    }

    OpenDialog->Destroy();
}

void xLightsFrame::OnMenuItemLoadEditPerspectiveSelected(wxCommandEvent& event)
{
    InitSequencer();
    m_mgr->GetPane(wxT("Perspectives")).Show(true);
    m_mgr->Update();
}

bool isOnLine(int x1, int y1, int x2, int y2, int x3, int y3)
{
    double diffx = x2 - x1;
    double diffy = y2 - y1;
    double b = y1 - diffy / diffx * x1;
    double ye1 = diffy / diffx * x3 + b;

    return (y3 + 1) >= ye1 && (y3 - 1) <= ye1;
}

bool isOnLineColor(const xlColor& v1, const xlColor& v2, const xlColor& v3,
                   int x, int x2, int x3)
{
    return isOnLine(x, v1.Red(), x2, v2.Red(), x3, v3.Red()) && isOnLine(x, v1.Green(), x2, v2.Green(), x3, v3.Green()) && isOnLine(x, v1.Blue(), x2, v2.Blue(), x3, v3.Blue());
}

int RampLenColor(int start, std::vector<xlColor>& colors)
{
    int s = start + 2;
    for (; s < colors.size(); s++) {
        if (!isOnLineColor(colors[start], colors[s - 1], colors[s],
                           start, s - 1, s)) {
            return s - start;
        }
    }
    if (s == colors.size()) {
        return s - start;
    }
    return 0;
}

void xLightsFrame::DoConvertDataRowToEffects(EffectLayer* layer, xlColorVector& colors, int frameTime, bool eraseExisting)
{
    if (eraseExisting)
        layer->DeleteAllEffects();

    colors.push_back(xlBLACK);
    int startTime = 0;
    xlColor lastColor(xlBLACK);

    for (size_t x = 0; x < colors.size() - 3; ++x) {
        if (colors[x] != colors[x + 1]) {
            int len = RampLenColor(x, colors);
            if (len >= 3) {
                HSVValue v1 = colors[x].asHSV();
                HSVValue v2 = colors[x + len - 1].asHSV();

                int stime = x * frameTime;
                int etime = (x + len) * frameTime;
                if (colors[x] == xlBLACK || colors[x + len - 1] == xlBLACK || (v1.hue == v2.hue)) {
                    HSVValue c = colors[x].asHSV();
                    if (colors[x] == xlBLACK) {
                        c = colors[x + len - 1].asHSV();
                    }
                    c.value = 1.0;
                    xlColor c2(c);

                    int i = v1.value * 100.0;
                    int i2 = v2.value * 100.0;
                    std::string settings = wxString::Format("E_TEXTCTRL_Eff_On_Start=%d,E_TEXTCTRL_Eff_On_End=%d", i, i2).ToStdString();
                    std::string palette = "C_BUTTON_Palette1=" + (std::string)c2 + ",C_CHECKBOX_Palette1=1";
                    if (!layer->HasEffectsInTimeRange(stime, etime)) {
                        layer->AddEffect(0, "On", settings, palette, stime, etime, false, false);
                    }
                } else {
                    std::string palette = "C_BUTTON_Palette1=" + (std::string)colors[x] + ",C_CHECKBOX_Palette1=1,"
                                                                                          "C_BUTTON_Palette2=" +
                                          (std::string)colors[x + len - 1] + ",C_CHECKBOX_Palette2=1";
                    if (!layer->HasEffectsInTimeRange(stime, etime)) {
                        layer->AddEffect(0, "Color Wash", "", palette, stime, etime, false, false);
                    }
                }
                for (int z = 0; z < len; ++z) {
                    // clear it
                    colors[x + z] = xlBLACK;
                }
            }
        }
    }

    for (size_t x = 0; x < colors.size(); ++x) {
        if (lastColor != colors[x]) {
            int time = x * frameTime;
            if (lastColor != xlBLACK) {
                std::string palette = "C_BUTTON_Palette1=" + (std::string)lastColor + ",C_CHECKBOX_Palette1=1";

                if (time != startTime) {
                    if (!layer->HasEffectsInTimeRange(startTime, time)) {
                        layer->AddEffect(0, "On", "", palette, startTime, time, false, false);
                    }
                }
            }
            startTime = time;
            lastColor = colors[x];
        }
    }
}

void xLightsFrame::ConvertDataRowToEffects(wxCommandEvent& event)
{
    Element* e = (Element*)event.GetClientData();
    if (e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
        ModelElement* el = dynamic_cast<ModelElement*>(e);
        Model* model = GetModel(el->GetModelName());

        // we cant do this on model groups & submodels
        if (model->GetDisplayAs() != "ModelGroup" && model->GetDisplayAs() != "SubModel") {
            for (int i = 0; i < el->GetStrandCount(); ++i) {
                StrandElement* se = el->GetStrand(i);
                for (int j = 0; j < se->GetNodeLayerCount(); ++j) {
                    EffectLayer* layer = se->GetNodeLayer(j);
                    xlColorVector colors;
                    colors.reserve(_seqData.NumFrames());
                    PixelBufferClass ncls(this);
                    if (model != nullptr) {
                        SingleLineModel* ssModel = new SingleLineModel(model->GetModelManager());
                        ssModel->Reset(1, *model, i, j);

                        for (size_t f = 0; f < _seqData.NumFrames(); ++f) {
                            ssModel->SetNodeChannelValues(0, &_seqData[f][ssModel->NodeStartChannel(0)]);
                            xlColor c = ssModel->GetNodeColor(0);
                            colors.push_back(c);
                        }
                        DoConvertDataRowToEffects(layer, colors, _seqData.FrameTime(), false);
                        delete ssModel;
                    }
                }
            }
        }
    } else if (e->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
        StrandElement* el = dynamic_cast<StrandElement*>(e);
        Model* model = GetModel(el->GetModelName());
        int node = event.GetInt() & 0xFFFF;
        int strand = (event.GetInt() >> 16) & 0xFFFF;

        if (node == 0xFFFF) {
            // this is a strand
            for (int i = 0; i < el->GetNodeLayerCount(); ++i) {
                EffectLayer* layer = el->GetNodeLayer(i);
                xlColorVector colors;
                colors.reserve(_seqData.NumFrames());
                PixelBufferClass ncls(this);
                if (model != nullptr) {
                    SingleLineModel* ssModel = new SingleLineModel(model->GetModelManager());
                    ssModel->Reset(1, *model, strand, i);

                    for (size_t f = 0; f < _seqData.NumFrames(); ++f) {
                        ssModel->SetNodeChannelValues(0, &_seqData[f][ssModel->NodeStartChannel(0)]);
                        xlColor c = ssModel->GetNodeColor(0);
                        colors.push_back(c);
                    }
                    DoConvertDataRowToEffects(layer, colors, _seqData.FrameTime(), false);
                    delete ssModel;
                }
            }
        } else {
            EffectLayer* layer = el->GetNodeLayer(node);
            xlColorVector colors;
            colors.reserve(_seqData.NumFrames());
            PixelBufferClass ncls(this);
            if (model != nullptr) {
                SingleLineModel* ssModel = new SingleLineModel(model->GetModelManager());
                ssModel->Reset(1, *model, strand, node);

                for (size_t f = 0; f < _seqData.NumFrames(); ++f) {
                    ssModel->SetNodeChannelValues(0, &_seqData[f][ssModel->NodeStartChannel(0)]);
                    xlColor c = ssModel->GetNodeColor(0);
                    colors.push_back(c);
                }
                DoConvertDataRowToEffects(layer, colors, _seqData.FrameTime(), false);
                delete ssModel;
            }
        }
    }
}

wxXmlNode* xLightsFrame::CreateEffectNode(wxString& name)
{
    wxXmlNode* NewXml = new wxXmlNode(wxXML_ELEMENT_NODE, "effect");
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

std::vector<std::string> GetPresets(wxXmlNode* node, const std::string& path)
{
    std::vector<std::string> res;

    if (node == nullptr)
        return res;

    for (auto n = node->GetChildren(); n != nullptr; n = n->GetNext()) {
        if (n->GetName() == "effect") {
            auto name = UnXmlSafe(n->GetAttribute("name", ""));
            if (path != "")
                name = path + "\\" + name;
            if (name != "")
                res.push_back(name);
        } else if (n->GetName() == "effectGroup") {
            auto name = UnXmlSafe(n->GetAttribute("name", ""));
            auto p = path;
            if (p != "")
                p = p + "\\";
            p = p + name;
            auto toadd = GetPresets(n, p);
            res.reserve(res.size() + toadd.size()); // preallocate memory
            res.insert(res.end(), toadd.begin(), toadd.end());
        }
    }

    return res;
}

std::vector<std::string> xLightsFrame::GetPresets() const
{
    return ::GetPresets(_sequenceElements.GetEffectsNode(), "");
}

wxXmlNode* xLightsFrame::FindPreset(wxXmlNode* node, wxArrayString& path, int level) const
{
    for (auto n = node->GetChildren(); n != nullptr; n = n->GetNext()) {
        if (n->GetName() == "effect") {
            if (UnXmlSafe(n->GetAttribute("name", "")) == path[level]) {
                return n;
            }
        } else if (n->GetName() == "effectGroup" && level < path.size() - 1) {
            if (UnXmlSafe(n->GetAttribute("name", "")) == path[level]) {
                return FindPreset(n, path, level + 1);
            }
        } else {
            wxASSERT(false);
        }
    }
    return nullptr;
}

Effect* xLightsFrame::ApplyEffectsPreset(const std::string& presetName)
{
    Effect* res = nullptr;
    wxXmlNode* ele = nullptr;

    auto path = wxSplit(presetName, '/');

    ele = FindPreset(_sequenceElements.GetEffectsNode(), path, 0);

    if (ele != nullptr) {
        res = mainSequencer->PanelEffectGrid->Paste(ele->GetAttribute("settings"), ele->GetAttribute("xLightsVersion", "4.0"));
    }

    return res;
}

void xLightsFrame::ApplyEffectsPreset(wxString& data, const wxString& pasteDataVersion)
{
    mainSequencer->PanelEffectGrid->Paste(data, pasteDataVersion);
}

void xLightsFrame::PromoteEffects(wxCommandEvent& command)
{
    ModelElement* el = dynamic_cast<ModelElement*>((Element*)command.GetClientData());
    DoPromoteEffects(el);
}

bool equals(Effect* e, Effect* e2, const wxString& pal, const wxString& set)
{
    if (e->GetEffectIndex() != e2->GetEffectIndex() || e->GetStartTimeMS() != e2->GetStartTimeMS() || e->GetEndTimeMS() != e2->GetEndTimeMS()) {
        return false;
    }
    if (pal != e2->GetPaletteAsString() || set != e2->GetSettingsAsString()) {
        return false;
    }
    return true;
}

void xLightsFrame::DoPromoteEffects(ModelElement* element)
{
    // first promote from nodes to strands
    for (int x = 0; x < element->GetStrandCount(); x++) {
        StrandElement* se = element->GetStrand(x);
        EffectLayer* target = se->GetEffectLayer(0);
        if (element->GetStrandCount() <= 1) {
            if (element->GetEffectLayer(0)->GetEffectCount() != 0) {
                element->InsertEffectLayer(0);
            }
            target = element->GetEffectLayer(0);
        }
        if (se->GetNodeLayerCount() > 0) {
            NodeLayer* base = se->GetNodeLayer(0);
            for (int e = base->GetEffectCount() - 1; e >= 0; e--) {
                Effect* eff = base->GetEffect(e);
                const std::string& name = eff->GetEffectName();

                if (target->HasEffectsInTimeRange(eff->GetStartTimeMS(), eff->GetEndTimeMS())) {
                    // cannot promote, already an effect there
                    continue;
                }
                if (name == "On" || name == "Color Wash") {
                    const std::string pal = eff->GetPaletteAsString();
                    const std::string set = eff->GetSettingsAsString();
                    int mp = (eff->GetStartTimeMS() + eff->GetEndTimeMS()) / 2;
                    bool collapse = true;

                    for (int n = 1; n < se->GetNodeLayerCount() && collapse; n++) {
                        NodeLayer* node = se->GetNodeLayer(n);
                        int nodeIndex = 0;
                        if (node->HitTestEffectByTime(mp, nodeIndex)) {
                            Effect* nf = node->GetEffect(nodeIndex);
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
                            NodeLayer* node = se->GetNodeLayer(n);
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
    if (element->GetStrandCount() <= 1)
        return;
    // OK, we're now promoted to strand level effects, try and promote to Model level
    EffectLayer* base = element->GetStrand(0)->GetEffectLayer(0);
    if (element->GetEffectLayer(0)->GetEffectCount() != 0) {
        element->InsertEffectLayer(0);
    }
    EffectLayer* target = element->GetEffectLayer(0);
    for (int e = base->GetEffectCount() - 1; e >= 0; e--) {
        Effect* eff = base->GetEffect(e);
        const std::string& name = eff->GetEffectName();

        if (name == "On" || name == "Color Wash") {
            const std::string pal = eff->GetPaletteAsString();
            const std::string set = eff->GetSettingsAsString();
            int mp = (eff->GetStartTimeMS() + eff->GetEndTimeMS()) / 2;
            bool collapse = true;

            for (int n = 0; n < element->GetStrandCount() && collapse; n++) {
                StrandElement* se = element->GetStrand(n);
                for (int l = 0; l < se->GetEffectLayerCount(); l++) {
                    EffectLayer* node = se->GetEffectLayer(l);
                    if (node == base) {
                        continue;
                    }
                    int nodeIndex = 0;
                    if (node->HitTestEffectByTime(mp, nodeIndex)) {
                        Effect* nf = node->GetEffect(nodeIndex);
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
                    StrandElement* se = element->GetStrand(n);
                    for (int l = 0; l < se->GetEffectLayerCount(); l++) {
                        EffectLayer* node = se->GetEffectLayer(l);
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
    InitSequencer();
    bool visible = m_mgr->GetPane("EffectDropper").IsShown();
    if (visible) {
        m_mgr->GetPane("EffectDropper").Hide();
    } else {
        m_mgr->GetPane("EffectDropper").Show();
    }
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::UpdateSequenceVideoPanel(const wxString& path)
{
    if (sequenceVideoPanel != nullptr) {
        std::string spath(path.ToStdString());
        ObtainAccessToURL(spath);
        sequenceVideoPanel->SetMediaPath(spath);
    }
}


void xLightsFrame::CallOnEffectBeforeSelected(std::function<bool(Effect *)> &&cb) {
    if (selectedEffect != nullptr) {
        Effect *ef = selectedEffect->GetParentEffectLayer()->GetEffectAtTime(selectedEffect->GetStartTimeMS() - 1);
        if (ef != nullptr) {
            _sequenceElements.get_undo_mgr().CaptureModifiedEffect(ef->GetParentEffectLayer()->GetParentElement()->GetFullName(),
                                                                   ef->GetParentEffectLayer()->GetIndex(), ef);
            bool changed = cb(ef);
            if (!changed) {
                //didn't change, cancel the undo step
                _sequenceElements.get_undo_mgr().CancelLastStep();
            }
        }
    }
}
void xLightsFrame::CallOnEffectAfterSelected(std::function<bool(Effect *)> &&cb) {
    if (selectedEffect != nullptr) {
        Effect *ef = selectedEffect->GetParentEffectLayer()->GetEffectAtTime(selectedEffect->GetEndTimeMS() + 1);
        if (ef != nullptr) {
            _sequenceElements.get_undo_mgr().CaptureModifiedEffect(ef->GetParentEffectLayer()->GetParentElement()->GetFullName(),
                                                                   ef->GetParentEffectLayer()->GetIndex(), ef);
            bool changed = cb(ef);
            if (!changed) {
                //didn't change, cancel the undo step
                _sequenceElements.get_undo_mgr().CancelLastStep();
            }
        }
    }
}
