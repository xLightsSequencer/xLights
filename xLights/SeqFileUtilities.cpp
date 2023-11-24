/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/config.h>
#include <wx/regex.h>
#include <wx/tokenzr.h>
#include <wx/uri.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

#include "BufferPanel.h"
#include "ConvertLogDialog.h"
#include "DataLayer.h"
#include "EffectAssist.h"
#include "ExternalHooks.h"
#include "FileConverter.h"
#include "FontManager.h"
#include "HousePreviewPanel.h"
#include "LMSImportChannelMapDialog.h"
#include "LOREdit.h"
#include "ModelPreview.h"
#include "SaveChangesDialog.h"
#include "SearchPanel.h"
#include "SelectPanel.h"
#include "SeqSettingsDialog.h"
#include "SequenceVideoPanel.h"
#include "SuperStarImportDialog.h"
#include "UtilFunctions.h"
#include "VSAFile.h"
#include "ViewsModelsPanel.h"
#include "VsaImportDialog.h"
#include "xLightsImportChannelMapDialog.h"
#include "xLightsMain.h"
#include "xLightsVersion.h"
#include "models/DMX/DmxModel.h"
#include "models/ModelGroup.h"
#include "sequencer/MainSequencer.h"

#include "SequencePackage.h"
#include "Vixen3.h"
#include "effects/BarsEffect.h"
#include "effects/ButterflyEffect.h"
#include "effects/CurtainEffect.h"
#include "effects/FireEffect.h"
#include "effects/GarlandsEffect.h"
#include "effects/MeteorsEffect.h"
#include "effects/PinwheelEffect.h"
#include "effects/SnowflakesEffect.h"
#include "effects/SpiralsEffect.h"

#include "xLightsApp.h"
#include "xLightsMain.h"

#include <log4cpp/Category.hh>

void xLightsFrame::AddAllModelsToSequence()
{
    if (ModelGroupsNode == nullptr)
        return;
    if (ModelsNode == nullptr)
        return;

    std::string models_to_add = "";
    bool first_model = true;
    for (wxXmlNode* e = ModelGroupsNode->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "modelGroup") {
            wxString name = e->GetAttribute("name");
            if (!_sequenceElements.ElementExists(name.ToStdString(), 0)) {
                if (!first_model) {
                    models_to_add += ",";
                }
                models_to_add += name;
                first_model = false;
            }
        }
    }

    for (wxXmlNode* e = ModelsNode->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "model") {
            wxString name = e->GetAttribute("name");
            if (!_sequenceElements.ElementExists(name.ToStdString(), 0)) {
                if (!first_model) {
                    models_to_add += ",";
                }
                models_to_add += name;
                first_model = false;
            }
        }
    }

    _sequenceElements.AddMissingModelsToSequence(models_to_add);
}

void xLightsFrame::NewSequence(const std::string& media, uint32_t durationMS, uint32_t frameMS, const std::string& defView)
{
    // close any open sequences
    if (!CloseSequence()) {
        return;
    }

    // assign global xml file object
    wxFileName xml_file;
    xml_file.SetPath(CurrentDir);
    CurrentSeqXmlFile = new xLightsXmlFile(xml_file, frameMS);

    if (_modelBlendDefaultOff) {
        CurrentSeqXmlFile->setSupportsModelBlending(false);
    }

    SeqSettingsDialog setting_dlg(this, CurrentSeqXmlFile, mediaDirectories, wxT(""), _defaultSeqView, true, media, durationMS);
    setting_dlg.Fit();
    int ret_code = setting_dlg.ShowModal();
    if (ret_code == wxID_CANCEL) {
        delete CurrentSeqXmlFile;
        CurrentSeqXmlFile = nullptr;
        return;
    } else {
        if (CurrentSeqXmlFile->GetMedia() != nullptr) {
            if (CurrentSeqXmlFile->GetMedia()->GetFrameInterval() < 0) {
                CurrentSeqXmlFile->GetMedia()->SetFrameInterval(CurrentSeqXmlFile->GetFrameMS());
            }
        }
    }

    // load media if available
    if (CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->HasAudioMedia()) {
        SetMediaFilename(CurrentSeqXmlFile->GetMedia()->FileName());
    }

    wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
    int ms = atoi(mss.c_str());

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("New sequence created Type %s Timing %dms.", (const char*)(CurrentSeqXmlFile->GetSequenceType().c_str()), ms);

    LoadSequencer(*CurrentSeqXmlFile);
    CurrentSeqXmlFile->SetSequenceLoaded(true);
    if (_sequenceElements.GetNumberOfTimingElements() == 0) {
        // only add timing if the user didnt set up timings
        std::string new_timing = "New Timing";
        CurrentSeqXmlFile->AddNewTimingSection(new_timing, this);
        _sequenceElements.AddTimingToAllViews(new_timing);
    } else {
        _sequenceElements.GetTimingElement(0)->SetActive(true);
    }
    MenuItem_File_Save->Enable(true);
    MenuItem_File_SaveAs_Sequence->Enable(true);
    MenuItem_File_Close_Sequence->Enable(true);
    MenuItem_File_Export_Video->Enable(true);
    MenuItem_PackageSequence->Enable(true);
    MenuItem_GenerateLyrics->Enable(true);
    MenuItem_ExportEffects->Enable(true);
    MenuItem_ImportEffects->Enable(true);
    MenuItem_PurgeRenderCache->Enable(true);

    unsigned int max = GetMaxNumChannels();
    size_t memRequired = std::max(CurrentSeqXmlFile->GetSequenceDurationMS(), mMediaLengthMS) / ms;
    memRequired *= max;
    memRequired /= 1024; // ->kb
    memRequired /= 1024; // ->mb
    if (memRequired > (GetPhysicalMemorySizeMB() - 1024) && (_promptBatchRenderIssues || (!_renderMode && !_checkSequenceMode))) {
        DisplayWarning(wxString::Format("The setup requires a large amount of memory (%lu MB) which could result in performance issues.", (unsigned long)memRequired), this);
    }

    if ((max > _seqData.NumChannels()) ||
        (CurrentSeqXmlFile->GetSequenceDurationMS() / ms) > (long)_seqData.NumFrames()) {
        _seqData.init(max, mMediaLengthMS / ms, ms);
    } else {
        _seqData.init(max, CurrentSeqXmlFile->GetSequenceDurationMS() / ms, ms);
    }

    // we can render now the sequence data buffers are initialised
    if (ret_code == NEEDS_RENDER) {
        RenderAll();
    }

    StartOutputTimer();
    displayElementsPanel->Initialize();
    std::string view = setting_dlg.GetView();
    if (defView != "" && (defView == "All Models" || defView == "Empty" || displayElementsPanel->HasView(defView))) {
        view = defView;
    }
    if (view == "All Models") {
        AddAllModelsToSequence();
        displayElementsPanel->SelectView("Master View");
    } else if (view != "Empty") {
        displayElementsPanel->SelectView(view);
    }
    SetAudioControls();
    Notebook1->SetSelection(Notebook1->GetPageIndex(PanelSequencer));
}

static wxFileName mapFileName(const wxFileName& orig)
{
    if (orig.GetDirCount() == 0) {
        // likely a filename from windows on Mac/Linux or vice versa
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

void xLightsFrame::SetPanelSequencerLabel(const std::string& sequence)
{
    PanelSequencer->SetLabel("XLIGHTS_SEQUENCER_TAB:" + sequence);
}

void xLightsFrame::OpenSequence(const wxString& passed_filename, ConvertLogDialog* plog)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    ClearNonExistentFiles();

    bool loaded_fseq = false;
    wxString filename;
    wxString wildcards = "xLights Sequence files (*.xsq;*.xml)|*.xsq;*.xml|Old xLights Sequence files (*.xml)|*.xml|FSEQ files (*.fseq)|*.fseq|Sequence Backups (*.xbkp)|*.xbkp";
    if (passed_filename.IsEmpty()) {
        filename = wxFileSelector("Choose sequence file to open", CurrentDir, wxEmptyString, "*.xsq", wildcards, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    } else {
        filename = passed_filename;
    }
    logger_base.debug("Opening File: %s", (const char*)filename.ToStdString().c_str());
    if (!filename.empty()) {
        if (filename.Contains(XLIGHTS_RGBEFFECTS_FILE) || filename.Contains(XLIGHTS_NETWORK_FILE) || filename.Contains(XLIGHTS_KEYBINDING_FILE)) {
            wxMessageBox("the 'xlights_rgbeffects.xml', 'xlights_networks.xml' or 'xlights_keybindings.xml' files are not valid sequence files", "Error");
            return;
        }

        // close any open sequences
        if (!CloseSequence()) {
            return;
        }

        if (wxFileName(filename).GetExt().Lower() == "xbkp") {
            wxMessageBox("NOTE: When you save this .xbkp file it will save as a .xsq file overwriting any existing sequence .xsq file", "Warning");
        }

        // check if there is a autosave backup file which is newer than the file we have been asked to open
        if (((!_renderMode && !_checkSequenceMode) || _promptBatchRenderIssues) && wxFileName(filename).GetExt().Lower() != "xbkp" && wxFileName(filename).GetExt().Lower() != "fseq") {
            wxFileName fn(filename);
            wxFileName xx = fn;
            xx.SetExt("xbkp");
            wxString asfile = xx.GetLongPath();

            if (FileExists(asfile)) {
                // the autosave file exists
                wxDateTime xmltime = fn.GetModificationTime();
                wxFileName asfn(asfile);
                wxDateTime xbkptime = asfn.GetModificationTime();

                if (xbkptime.IsValid() && xmltime.IsValid() && (xbkptime > xmltime)) {
                    // autosave file is newer
                    if (wxMessageBox("Autosaved file found which seems to be newer than your sequence file ... would you like to open that instead and replace your xml file?", "Newer file found", wxYES_NO) == wxYES) {
                        // run a backup ... equivalent of a F10
                        DoBackup(false, false, true);

                        // delete the old xml file
                        wxRemoveFile(filename);

                        // rename the autosave file
                        wxRenameFile(asfile, filename);
                    } else {
                        if (FileExists(fn)) {
                            // set the backup to be older than the XML files to avoid re-promting
                            xmltime -= wxTimeSpan(0, 0, 3, 0); // subtract 2 seconds as FAT time resulution is 2 seconds
                            asfn.SetTimes(&xmltime, &xmltime, &xmltime);
                        }
                    }
                }
            }
        }

        wxStopWatch sw; // start a stopwatch timer

        wxFileName selected_file(filename);

        SetPanelSequencerLabel(selected_file.GetName().ToStdString());

        wxFileName xml_file = selected_file;
        if (xml_file.GetExt() != "xml") {
            xml_file.SetExt("xsq");
            // maybe the filename has not changed
            if (!FileExists(xml_file)) {
                // xsq not found ... maybe it is xbkp
                xml_file.SetExt("xbkp");
                if (!FileExists(xml_file)) {
                    xml_file.SetExt("xml");
                }
            }
        }
        wxFileName media_file;

        wxFileName fseq_file = selected_file;
        fseq_file.SetExt("fseq");

        wxFileName fseq_file_SEQ_fold = selected_file;
        fseq_file_SEQ_fold.SetExt("fseq");

        // Only Look for FSEQ file in FSEQ FOLDER, if folder are unlinked
        if (wxFileName(fseqDirectory) != wxFileName(showDirectory)) {
            ObtainAccessToURL(fseqDirectory);
            fseq_file.SetPath(fseqDirectory);
            if (!FileExists(fseq_file)) {
                // no FSEQ file found in FSEQ Folder, look for it next to the SEQ File
                if (FileExists(fseq_file_SEQ_fold)) {
                    // if found, move file to fseq folder
                    logger_base.debug("Moving FSEQ File: '%s' to '%s'", (const char*)fseq_file_SEQ_fold.GetPath().c_str(), (const char*)fseq_file.GetPath().c_str());
                    wxRenameFile(fseq_file_SEQ_fold.GetFullPath(), fseq_file.GetFullPath());
                }
            } else {
                // if FSEQ File is Found in FSEQ Folder, remove old file next to the Seq File
                /***************************/
                // TODO: Maybe remove this if Keith/Gil/Dan think it's bad - Scott
                if (FileExists(fseq_file_SEQ_fold)) {
                    // remove FSEQ file next to seg file
                    logger_base.debug("Deleting old FSEQ File: '%s'", (const char*)fseq_file_SEQ_fold.GetPath().c_str());
                    wxRemoveFile(fseq_file_SEQ_fold.GetFullPath()); //
                }
            }
        }

        xlightsFilename = fseq_file.GetFullPath(); // this need to be set , as it is checked when saving is triggered

        // load the fseq data file if it exists
        if (FileExists(fseq_file)) {
            logger_base.debug("Opening FSEQ File at: '%s'", (const char*)fseq_file.GetFullPath().c_str());
            if (plog != nullptr) {
                plog->Show(true);
            }
            std::string mf;
            ConvertParameters read_params(xlightsFilename,                        // input filename
                                          _seqData,                               // sequence data object
                                          &_outputManager,                        // global network info
                                          ConvertParameters::READ_MODE_LOAD_MAIN, // file read mode
                                          this,                                   // xLights main frame
                                          nullptr,
                                          plog,
                                          &mf); // media filename

            FileConverter::ReadFalconFile(read_params);
            if (mf != "") {
                media_file = mapFileName(wxFileName::FileName(mf));
            }
            DisplayXlightsFilename(xlightsFilename);
            SeqBaseChannel = 1;
            SeqChanCtrlBasic = false;
            SeqChanCtrlColor = false;
            loaded_fseq = true;

            logger_base.debug("    Fseq file loaded.");
            logger_base.debug("        Channels %u", _seqData.NumChannels());
            logger_base.debug("        Frame Time %u", _seqData.FrameTime());
            logger_base.debug("        Frames %u", _seqData.NumFrames());
            logger_base.debug("        Length %u", _seqData.TotalTime());
        } else {
            logger_base.debug("Could not Find FSEQ File at: '%s'", (const char*)fseq_file.GetFullPath().c_str());
        }

        // assign global xml file object
        CurrentSeqXmlFile = new xLightsXmlFile(xml_file);

        // open the xml file so we can see if it has media
        CurrentSeqXmlFile->Open(GetShowDirectory());

        _renderCache.SetSequence(renderCacheDirectory, CurrentSeqXmlFile->GetName().ToStdString());

        // if fseq didn't have media check xml
        if (CurrentSeqXmlFile->GetMediaFile() != "") {
            media_file = mapFileName(CurrentSeqXmlFile->GetMediaFile());
            ObtainAccessToURL(media_file.GetFullPath().ToStdString());
        }

        if (media_file.GetName() != "") {
            logger_base.debug("Media file from sequence: '%s'", (const char*)media_file.GetFullPath().c_str());

            // double-check file existence
            if (!FileExists(media_file) || !wxFileName(media_file).IsFileReadable()) {
                wxFileName detect_media(media_file);

                // search media directory
                for (auto& mediaDirectory : mediaDirectories) {
                    detect_media.SetPath(mediaDirectory);
                    if (FileExists(detect_media)) {
                        media_file = detect_media;
                        ObtainAccessToURL(media_file.GetFullPath().ToStdString());
                        break;
                    } else {
                        // search selected file directory
                        detect_media.SetPath(selected_file.GetPath());
                        if (FileExists(detect_media)) {
                            media_file = detect_media;
                            ObtainAccessToURL(media_file.GetFullPath().ToStdString());
                            break;
                        }
                    }
                }
                logger_base.debug("    Did not exist, attepting to map to: '%s'", (const char*)media_file.GetFullPath().c_str());
            }

            // search for missing media file in media directory and show directory
            if (!FileExists(media_file) || !wxFileName(media_file).IsFileReadable()) {
                wxFileName detect_media(selected_file);
                detect_media.SetExt("mp3");

                // search media directory
                for (auto& mediaDirectory : mediaDirectories) {
                    detect_media.SetPath(mediaDirectory);
                    if (FileExists(detect_media)) {
                        media_file = detect_media;
                        ObtainAccessToURL(media_file.GetFullPath().ToStdString());
                        break;
                    } else {
                        // search selected file directory
                        detect_media.SetPath(selected_file.GetPath());
                        if (FileExists(detect_media)) {
                            media_file = detect_media;
                            ObtainAccessToURL(media_file.GetFullPath().ToStdString());
                            break;
                        }
                    }
                }
                logger_base.debug("    Still did not exist, attepting to map to: '%s'", (const char*)media_file.GetFullPath().c_str());
            }

            // search for missing media file in the show directory one folder deep
            if (!FileExists(media_file) || !wxFileName(media_file).IsFileReadable()) {
                wxFileName detect_audio(CurrentSeqXmlFile->GetMediaFile());
                wxDir audDirectory;
                wxString audFile;
                audDirectory.Open(GetShowDirectory());
                bool fcont = audDirectory.GetFirst(&audFile, wxEmptyString, wxDIR_DIRS);
                while (fcont) {
                    if (audFile != "Backup") {
                        // search directory
                        detect_audio.SetPath(GetShowDirectory() + wxFileName::GetPathSeparator() + audFile);
                        if (FileExists(detect_audio)) {
                            media_file = detect_audio;
                            ObtainAccessToURL(media_file.GetFullPath().ToStdString());
                            break;
                        }
                    }
                    fcont = audDirectory.GetNext(&audFile);
                }
                logger_base.debug("    Still did not exist, attepting to map to: '%s'", (const char*)media_file.GetFullPath().c_str());
            }
        }

        // if fseq had media update xml
        if (!CurrentSeqXmlFile->HasAudioMedia() && FileExists(media_file) && wxFileName(media_file).IsFileReadable()) {
            CurrentSeqXmlFile->SetMediaFile(GetShowDirectory(), media_file.GetFullPath(), true);
            int length_ms = CurrentSeqXmlFile->GetMedia()->LengthMS();
            CurrentSeqXmlFile->SetSequenceDurationMS(length_ms);
            playAnimation = false;
        } else {
            playAnimation = true;
        }

        if (CurrentSeqXmlFile->WasConverted()) {
            logger_base.debug("Loaded Sequence was Converted, need to check settings");
            // abort any in progress render ... as it may be using any already open media
            bool aborted = false;
            if (CurrentSeqXmlFile->GetMedia() != nullptr) {
                aborted = AbortRender();
            }

            SeqSettingsDialog setting_dlg(this, CurrentSeqXmlFile, mediaDirectories, wxT("V3 file was converted. Please check settings!"), wxEmptyString);
            setting_dlg.Fit();
            int ret_code = setting_dlg.ShowModal();

            if (ret_code == NEEDS_RENDER || aborted) {
                RenderAll();
            }

            if (CurrentSeqXmlFile->GetMedia() != nullptr && CurrentSeqXmlFile->GetMedia()->GetFrameInterval() < 0) {
                CurrentSeqXmlFile->GetMedia()->SetFrameInterval(CurrentSeqXmlFile->GetFrameMS());
            }
            SetAudioControls();
        }

        wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
        int ms = atoi(mss.c_str());
        logger_base.debug("Sequence Timing: %d", ms);
        bool loaded_xml = SeqLoadXlightsFile(*CurrentSeqXmlFile, true);

        unsigned int numChan = GetMaxNumChannels();
        size_t memRequired = std::max(CurrentSeqXmlFile->GetSequenceDurationMS(), mMediaLengthMS) / ms;
        memRequired *= numChan;
        memRequired /= 1024; // ->kb
        memRequired /= 1024; // ->mb
        if (memRequired > (GetPhysicalMemorySizeMB() - 1024) && (_promptBatchRenderIssues || (!_renderMode && !_checkSequenceMode))) {
            DisplayWarning(wxString::Format("The setup requires a large amount of memory (%lu MB) which could result in performance issues.", (unsigned long)memRequired), this);
        }

        logger_base.debug("Sequence Num Channels: %d or %d", numChan, _seqData.NumChannels());
        logger_base.debug("Sequence Num Frames: %d", (int)(CurrentSeqXmlFile->GetSequenceDurationMS() / ms));

        if ((numChan > _seqData.NumChannels()) ||
            (CurrentSeqXmlFile->GetSequenceDurationMS() / ms) > (long)_seqData.NumFrames()) {
            if (_seqData.NumChannels() > 0) {
                if (numChan > _seqData.NumChannels()) {
                    logger_base.warn("Fseq file had %u channels but sequence has %u channels so dumping the fseq data.", _seqData.NumChannels(), numChan);
                } else {
                    if ((CurrentSeqXmlFile->GetSequenceDurationMS() / ms) > (long)_seqData.NumFrames()) {
                        logger_base.warn("Fseq file had %u frames but sequence has %u frames so dumping the fseq data.",
                                         CurrentSeqXmlFile->GetSequenceDurationMS() / ms,
                                         _seqData.NumFrames());
                    }
                }
            }
            _seqData.init(numChan, mMediaLengthMS / ms, ms);
        } else if (!loaded_fseq) {
            _seqData.init(numChan, CurrentSeqXmlFile->GetSequenceDurationMS() / ms, ms);
        }

        logger_base.debug("Initializeing Display Elements");
        displayElementsPanel->Initialize();

        // if we loaded the fseq but not the xml then we need to populate views
        if (!CurrentSeqXmlFile->IsOpen()) {
            std::string new_timing = "New Timing";
            CurrentSeqXmlFile->AddNewTimingSection(new_timing, this);
            _sequenceElements.AddTimingToAllViews(new_timing);
            AddAllModelsToSequence();
            displayElementsPanel->SelectView("Master View");
        }

        logger_base.debug("Starting timers");
        StartOutputTimer();
        if (loaded_fseq) {
            GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LayoutPanel::HandleLayoutKey::OpenSequence");
        } else if (!loaded_xml) {
            SetStatusText(wxString::Format("Failed to load: '%s'.", filename));
            return;
        }

        float elapsedTime = sw.Time() / 1000.0; // msec => sec
        SetStatusText(wxString::Format("'%s' loaded in %4.3f sec.", filename, elapsedTime));
        SetTitle(xlights_base_name + xlights_qualifier + " - " + filename);

        EnableSequenceControls(true);
        Notebook1->SetSelection(Notebook1->GetPageIndex(PanelSequencer));

        AddToMRU(filename);
        UpdateRecentFilesList(false);
    }
}

void xLightsFrame::AddToMRU(const std::string& filename)
{
    if (mruFiles.Index(filename) != wxNOT_FOUND) {
        mruFiles.Remove(filename);
    }
    if (mruFiles.empty()) {
        mruFiles.push_back(filename);
    } else {
        mruFiles.Insert(filename, 0);
    }
}

bool xLightsFrame::CloseSequence()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Closing sequence.");

    if (_autoSavePerspecive && CurrentSeqXmlFile != nullptr) {
        // save perspective on this machine so we can restore it next time
        wxConfigBase* config = wxConfigBase::Get();
        wxString machinePerspective = m_mgr->SavePerspective();
        config->Write("xLightsMachinePerspective", machinePerspective);
        logger_base.debug("AutoSave perspective");
        LogPerspective(machinePerspective);
    }

    if (mSavedChangeCount != _sequenceElements.GetChangeCount() && !_renderMode && !_checkSequenceMode) {
        SaveChangesDialog* dlg = new SaveChangesDialog(this);
        if (dlg->ShowModal() == wxID_CANCEL) {
            return false;
        }
        if (dlg->GetSaveChanges()) {
            SaveSequence();
            // must wait for the rendering to complete
            while (!renderProgressInfo.empty()) {
                wxMilliSleep(10);
                wxYield();
            }
        } else {
            if (CurrentSeqXmlFile != nullptr) {
                // We discarded the sequence so make sure the sequence file is newer than the backup
                wxFileName fn(CurrentSeqXmlFile->GetLongPath());
                wxFileName xx = fn;
                xx.SetExt("xbkp");
                wxString asfile = xx.GetLongPath();

                if (FileExists(asfile)) {
                    // the autosave file exists
                    wxDateTime xmltime = fn.GetModificationTime();
                    wxFileName asfn(asfile);
                    wxDateTime xbkptime = asfn.GetModificationTime();

                    if (xbkptime > xmltime) {
                        // set the backup to be older than the XML files to avoid re-promting
                        xmltime -= wxTimeSpan(0, 0, 3, 0); // subtract 2 seconds as FAT time resulution is 2 seconds
                        asfn.SetTimes(&xmltime, &xmltime, &xmltime);
                    }
                }
            }
        }
    }

    // just in case there is still rendering going on
    AbortRender();

    _renderCache.CleanupCache(&_sequenceElements);
    _renderCache.SetSequence(renderCacheDirectory, "");

    // clear everything to prepare for new sequence
    if (displayElementsPanel != nullptr)
        displayElementsPanel->Clear();
    if (sEffectAssist != nullptr)
        sEffectAssist->SetPanel(nullptr);
    if (sequenceVideoPanel != nullptr)
        sequenceVideoPanel->SetMediaPath("");
    xlightsFilename = "";
    mediaFilename = "";
    previewLoaded = false;
    previewPlaying = false;
    playType = 0;
    selectedEffect = nullptr;
    if (CurrentSeqXmlFile) {
        delete CurrentSeqXmlFile;
        CurrentSeqXmlFile = nullptr;
    }

    if (mainSequencer != nullptr) {
        if (mainSequencer->PanelEffectGrid != nullptr)
            mainSequencer->PanelEffectGrid->ClearSelection();
    }

    _sequenceElements.Clear();
    mSavedChangeCount = _sequenceElements.GetChangeCount();
    mLastAutosaveCount = mSavedChangeCount;

    SetPanelSequencerLabel("");

    if (_selectPanel != nullptr) {
        _selectPanel->ClearData();
    }
    if (_searchPanel != nullptr) {
        _searchPanel->ClearData();
    }
    if (mainSequencer != nullptr) {
        if (mainSequencer->PanelWaveForm != nullptr)
            mainSequencer->PanelWaveForm->CloseMedia();
        if (mainSequencer->ViewChoice != nullptr)
            mainSequencer->ViewChoice->Clear();
    }
    _seqData.init(0, 0, 50);
    EnableSequenceControls(true); // let it re-evaluate menu state
    SetStatusText("");
    SetStatusText(CurrentDir, true);
    if (_modelPreviewPanel != nullptr)
        _modelPreviewPanel->Refresh();
    if (_housePreviewPanel != nullptr)
        _housePreviewPanel->Refresh();

    SetTitle(xlights_base_name + xlights_qualifier + " (Ver " + GetDisplayVersionString() + ") " + xlights_build_date);

    return true;
}

bool xLightsFrame::SeqLoadXlightsFile(const wxString& filename, bool ChooseModels)
{
    delete xLightsFrame::CurrentSeqXmlFile;
    xLightsFrame::CurrentSeqXmlFile = new xLightsXmlFile(filename);
    return SeqLoadXlightsFile(*xLightsFrame::CurrentSeqXmlFile, ChooseModels);
}

// Load the xml file containing effects for a particular sequence
// Returns true if file exists and was read successfully
bool xLightsFrame::SeqLoadXlightsFile(xLightsXmlFile& xml_file, bool ChooseModels)
{
    LoadSequencer(xml_file);
    xml_file.SetSequenceLoaded(true);
    return true;
}

void xLightsFrame::ClearSequenceData()
{
    wxASSERT(_seqData.IsValidData());
    for (size_t i = 0; i < _seqData.NumFrames(); ++i)
        _seqData[i].Zero();
}

void xLightsFrame::RenderIseqData(bool bottom_layers, ConvertLogDialog* plog)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("xLightsFrame::RenderIseqData bottom_layers %d", bottom_layers);

    DataLayerSet& data_layers = CurrentSeqXmlFile->GetDataLayers();
    ConvertParameters::ReadMode read_mode;
    if (bottom_layers && data_layers.GetNumLayers() == 1 &&
        data_layers.GetDataLayer(0)->GetName() == "Nutcracker") {
        DataLayer* nut_layer = data_layers.GetDataLayer(0);
        if (nut_layer->GetDataSource() == xLightsXmlFile::CANVAS_MODE) {
            // Don't clear, v3 workflow of augmenting the existing fseq file
            return;
        }
    }

    if (bottom_layers) {
        logger_base.debug("xLightsFrame::RenderIseqData clearing sequence data.");
        ClearSequenceData();
        read_mode = ConvertParameters::READ_MODE_NORMAL;
    } else {
        read_mode = ConvertParameters::READ_MODE_IGNORE_BLACK;
    }

    int layers_rendered = 0;
    bool start_rendering = bottom_layers;
    for (int i = data_layers.GetNumLayers() - 1; i >= 0; --i) // build layers bottom up
    {
        DataLayer* data_layer = data_layers.GetDataLayer(i);
        if (data_layer->GetName() != "Nutcracker") {
            if (start_rendering) {
                logger_base.debug("xLightsFrame::RenderIseqData rendering %s.", (const char*)data_layer->GetDataSource().c_str());
                if (plog != nullptr) {
                    plog->Show(true);
                }
                ConvertParameters read_params(data_layer->GetDataSource(), // input filename
                                              _seqData,                    // sequence data object
                                              &_outputManager,             // global network info
                                              read_mode,                   // file read mode
                                              this,                        // xLights main frame
                                              nullptr,
                                              plog,
                                              nullptr,     // filename not needed
                                              data_layer); // provide data layer for channel offsets

                FileConverter::ReadFalconFile(read_params);
                read_mode = ConvertParameters::READ_MODE_IGNORE_BLACK;
                ++layers_rendered;
            }
        } else {
            if (bottom_layers)
                break; // exit after Nutcracker layer if rendering bottom layers only
            start_rendering = true;
        }
    }
}

void xLightsFrame::SetSequenceEnd(int ms)
{
    mainSequencer->PanelTimeLine->SetSequenceEnd(CurrentSeqXmlFile->GetSequenceDurationMS());
    _sequenceElements.SetSequenceEnd(CurrentSeqXmlFile->GetSequenceDurationMS());
    _housePreviewPanel->SetDurationFrames(CurrentSeqXmlFile->GetSequenceDurationMS() / CurrentSeqXmlFile->GetFrameMS());
}

static bool CalcPercentage(std::string& value, double base, bool reverse, int offset)
{
    int val = wxAtoi(value);
    val -= offset;
    val %= (int)base;
    if (val < 0)
        return false;
    double half_width = 1.0 / base * 50.0;
    double percent = (double)val / base * 100.0 + half_width;
    if (reverse) {
        percent = 100.0 - percent;
    }
    value = wxString::Format(wxT("%i"), (int)percent);
    return true;
}

static bool CalcBoundedPercentage(std::string& value, int base, bool reverse, int offset)
{
    int val = wxAtoi(value);
    val -= offset;
    val %= (int)base;
    if (val < 0)
        return false;
    if (val == 0) {
        value = reverse ? "99.9" : "0.0";
    } else if (val == (base - 1)) {
        value = reverse ? "0.0" : "99.9";
    } else {
        return CalcPercentage(value, base, reverse, offset);
    }
    return true;
}

static int CalcUnBoundedPercentage(int val, int base)
{
    double half_width = 50.0 / base;
    double percent = (double)val / base * 100.0 + half_width;

    return percent;
}

static xlColor GetColor(const std::string& sRed, const std::string& sGreen, const std::string& sBlue)
{
    double red = atof(sRed.c_str());
    red = red / 100.0 * 255.0;
    double green = atof(sGreen.c_str());
    green = green / 100.0 * 255.0;
    double blue = atof(sBlue.c_str());
    blue = blue / 100.0 * 255.0;
    xlColor color(red, green, blue);
    return color;
}

static wxString GetColorString(const std::string& sRed, const std::string& sGreen, const std::string& sBlue)
{
    return GetColor(sRed, sGreen, sBlue);
}

static xlColor GetColor(const std::string& rgb)
{
    int i = wxAtoi(rgb);
    xlColor cl;
    cl.red = (i & 0xff);
    cl.green = ((i >> 8) & 0xFF);
    cl.blue = ((i >> 16) & 0xff);
    return cl;
}

static EffectLayer* FindOpenLayer(Element* model, int layer_index, int startTimeMS, int endTimeMS, std::vector<bool>& reserved)
{
    int index = layer_index - 1;

    EffectLayer* layer = model->GetEffectLayer(index);
    if (layer != nullptr && layer->GetRangeIsClearMS(startTimeMS, endTimeMS)) {
        return layer;
    }

    // need to search for open layer
    for (size_t i = 0; i < model->GetEffectLayerCount(); ++i) {
        if (i >= reserved.size() || !reserved[i]) {
            layer = model->GetEffectLayer(i);
            if (layer->GetRangeIsClearMS(startTimeMS, endTimeMS)) {
                return layer;
            }
        }
    }

    // empty layer not found so create a new one
    layer = model->AddEffectLayer();
    if (model->GetEffectLayerCount() > reserved.size()) {
        reserved.resize(model->GetEffectLayerCount(), false);
    }
    return layer;
}

#define MAXBUFSIZE 4096
class FixXMLInputStream : public wxInputStream
{
public:
    FixXMLInputStream(wxInputStream& in) :
        wxInputStream(), bin(in)
    {
    }
    void fillBuf()
    {
        int pos = bufLen;
        int sz = MAXBUFSIZE - bufLen;
        bin.Read(&buf[pos], sz);
        size_t ret = bin.LastRead();
        bufLen += ret;

        bool needToClose = false;
        for (size_t x = 7; x < bufLen; ++x) {
            if (buf[x - 7] == '<' && buf[x - 6] == 'p' && buf[x - 5] == 'i' && buf[x - 4] == 'x' && buf[x - 3] == 'e' && buf[x - 2] == 'l' && buf[x - 1] == 's' && buf[x] == '=') {
                buf[x - 2] = ' ';
            } else if (buf[x - 7] == '<' && buf[x - 6] == 't' && buf[x - 5] == 'i' && buf[x - 4] == 'm' && buf[x - 3] == 'i' && buf[x - 2] == 'n' && buf[x - 1] == 'g' && buf[x] == ' ') {
                needToClose = true;
            } else if (x > 6 &&
                       buf[x - 6] == '<' && buf[x - 5] == 'g' && buf[x - 4] == 'r' && buf[x - 3] == 'o' && buf[x - 2] == 'u' && buf[x - 1] == 'p' && buf[x] == ' ') {
                needToClose = true;
            } else if (x > 12 &&
                       buf[x - 12] == '<' && buf[x - 11] == 'i' && buf[x - 10] == 'm' && buf[x - 9] == 'a' && buf[x - 8] == 'g' && buf[x - 7] == 'e' && buf[x - 6] == 'A' && buf[x - 5] == 'c' && buf[x - 4] == 't' && buf[x - 3] == 'i' && buf[x - 2] == 'o' && buf[x - 1] == 'n' && buf[x] == ' ') {
                needToClose = true;
            } else if (x > 11 &&
                       buf[x - 11] == '<' && buf[x - 10] == 't' && buf[x - 9] == 'e' && buf[x - 8] == 'x' && buf[x - 7] == 't' && buf[x - 6] == 'A' && buf[x - 5] == 'c' && buf[x - 4] == 't' && buf[x - 3] == 'i' && buf[x - 2] == 'o' && buf[x - 1] == 'n' && buf[x] == ' ') {
                needToClose = true;
            } else if (x > 14 &&
                       buf[x - 14] == '<' && buf[x - 13] == 'c' && buf[x - 12] == 'o' && buf[x - 11] == 'n' && buf[x - 10] == 'f' && buf[x - 9] == 'i' && buf[x - 8] == 'g' && buf[x - 7] == 'u' && buf[x - 6] == 'r' && buf[x - 5] == 'a' && buf[x - 4] == 't' && buf[x - 3] == 'i' && buf[x - 2] == 'o' && buf[x - 1] == 'n' && buf[x] == ' ') {
                needToClose = true;
            } else if (x > 15 &&
                       buf[x - 15] == '<' && buf[x - 14] == '/' && buf[x - 13] == 'c' && buf[x - 12] == 'o' && buf[x - 11] == 'n' && buf[x - 10] == 'f' && buf[x - 9] == 'i' && buf[x - 8] == 'g' && buf[x - 7] == 'u' && buf[x - 6] == 'r' && buf[x - 5] == 'a' && buf[x - 4] == 't' && buf[x - 3] == 'i' && buf[x - 2] == 'o' && buf[x - 1] == 'n' && buf[x] == '>') {
                for (int y = x - 15; y <= x; ++y) {
                    buf[y] = ' ';
                }
            } else if (buf[x - 1] == '>' && needToClose) {
                if (buf[x - 2] != '/') {
                    buf[x - 1] = '/';
                    buf[x] = '>';
                }
                needToClose = false;
            }
        }
    }

    virtual size_t OnSysRead(void* buffer, size_t bufsize)
    {
        unsigned char* b = (unsigned char*)buffer;
        if (bufsize > 1024) {
            bufsize = 1024;
        }

        if (bufLen < 2000) {
            fillBuf();
        }

        if (bufLen) {
            size_t ret = std::min(bufsize, bufLen);
            memcpy(b, buf, ret);
            for (int x = ret; x < bufLen; ++x) {
                buf[x - ret] = buf[x];
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

void xLightsFrame::OnMenuItemImportEffects(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxArrayString filters;
    filters.push_back("All|*.xsq;*.sup;*.lms;*.lpe;*.las;*.loredit;*.xml;*.hlsdata;*.vix;*.tim;*.msq;*.vsa;*.zip;*.piz");
    filters.push_back("SuperStar File (*.sup)|*.sup");
    filters.push_back("LOR Music Sequences (*.lms)|*.lms");
    filters.push_back("LOR Pixel Editor Sequences (*.lpe)|*.lpe");
    filters.push_back("LOR Animation Sequences (*.las)|*.las");
    filters.push_back("LOR S5(*.loredit)|*.loredit");
    filters.push_back("xLights Sequence Package (*.zip;*.piz)|*.zip;*.piz");
    filters.push_back("xLights Sequence (*.xsq)|*.xsq");
    filters.push_back("Old xLights Sequence (*.xml)|*.xml");
    filters.push_back("HLS hlsIdata Sequences(*.hlsIdata)|*.hlsIdata");
    filters.push_back("Vixen 2.x Sequence(*.vix)|*.vix");
    filters.push_back("Vixen 3.x Sequence(*.tim)|*.tim");
    filters.push_back("LSP 2.x Sequence(*.msq)|*.msq");
    filters.push_back("VSA Files(*.vsa)|*.vsa");

    wxString filter;
    for (auto it = filters.begin(); it != filters.end(); ++it) {
        if (filter != "") {
            filter += "|";
        }
        filter += *it;
    }

    wxFileDialog file(this, "Choose file to import", "", "", filter);

    wxString lit = "";
    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        config->Read("xLightsLastImportType", &lit, "");
    }
    if (lit != "") {
        int index = 0;

        for (auto it = filters.begin(); it != filters.end(); ++it) {
            if (lit == *it) {
                file.SetFilterIndex(index);
                break;
            }
            index++;
        }
    }

    if (file.ShowModal() == wxID_OK) {
        if (config != nullptr && file.GetFilterIndex() >= 0 && file.GetFilterIndex() < filters.size()) {
            config->Write("xLightsLastImportType", filters[file.GetFilterIndex()]);
        } else {
            logger_base.warn("XLightsLastImportType not saved due to invalid filter index %d.", file.GetFilterIndex());
        }

        wxFileName fn = file.GetPath();
        if (!FileExists(fn)) {
            return;
        }
        wxString ext = fn.GetExt().Lower();
        if (ext == "lms" || ext == "las") {
            ImportLMS(fn);
        } else if (ext == "lpe") {
            ImportLPE(fn);
        } else if (ext == "loredit") {
            ImportS5(fn);
        } else if (ext == "hlsidata") {
            ImportHLS(fn);
        } else if (ext == "sup") {
            ImportSuperStar(fn);
        } else if (ext == "tim") {
            ImportVixen3(fn);
        } else if (ext == "vix") {
            ImportVix(fn);
        } else if (ext == "xml" || ext == "xsq" || ext == "zip" || ext == "piz") {
            ImportXLights(fn);
        } else if (ext == "msq") {
            ImportLSP(fn);
        } else if (ext == "vsa") {
            ImportVsa(fn);
        }
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(this, eventRowHeaderChanged);
        mainSequencer->PanelEffectGrid->Refresh();
    }
}

void MapXLightsEffects(EffectLayer* target, EffectLayer* src, std::vector<EffectLayer*>& mapped, bool eraseExisting, SequencePackage& xsqPkg, bool lock, const std::map<std::string, std::string>& mapping)
{
    if (eraseExisting)
        target->DeleteAllEffects();

    for (int x = 0; x < src->GetEffectCount(); ++x) {
        Effect* ef = src->GetEffect(x);
        if (!target->HasEffectsInTimeRange(ef->GetStartTimeMS(), ef->GetEndTimeMS())) {
            std::string settings;
            if (xsqPkg.HasMedia() && xsqPkg.GetImportOptions()->IsImportActive()) {
                // attempt to import it and fix settings
                settings = xsqPkg.FixAndImportMedia(ef, target);
            } else {
                settings = ef->GetSettingsAsString();
            }

            // remove lock if it is there
            Replace(settings, ",X_Effect_Locked=True", "");

            // If this is a duplicate effect map the duplicate to the model the original model was mapped to
            if (ef->GetEffectIndex() == EffectManager::eff_DUPLICATE && Contains(settings, "E_CHOICE_Duplicate_Model")) {
                auto dupModel = ef->GetSettings()["E_CHOICE_Duplicate_Model"];
                auto it = mapping.find(dupModel);
                if (it != mapping.end()) {
                    Replace(settings, ",E_CHOICE_Duplicate_Model=" + dupModel, ",E_CHOICE_Duplicate_Model=" + it->second);
                }
            }

            // if we are mapping the effect onto a group and it is a per preview render buffer then use the group's default camera
            //   unless there is a non-default 3D camera assigned to the effect, and it exists in the target layout
            if (!target->IsTimingLayer()) {
                Model* m = target->GetParentElement()->GetSequenceElements()->GetXLightsFrame()->GetModel(target->GetParentElement()->GetModelName());
                if (m != nullptr) {
                    auto mg = dynamic_cast<const ModelGroup*>(m);
                    if (mg != nullptr) {
                        // so is it a per preview render buffer
                        auto rb = ef->GetSettings()["B_CHOICE_BufferStyle"];
                        if (BufferPanel::CanRenderBufferUseCamera(rb)) {
                            if (Contains(settings, "B_CHOICE_PerPreviewCamera")) {
                                // MoC - There isn't a way to just indicate "use group's default", so instead we grab it as
                                //   a setting for the effect.
                                // That way if the group default changes, there is no effect on old / mapped effects
                                auto newCamera = mg->GetDefaultCamera();
                                auto effCamera = ef->GetSettings()["B_CHOICE_PerPreviewCamera"];
                                xLightsFrame* frame = xLightsApp::GetFrame();
                                if (effCamera != "2D" && effCamera != "Default" && frame->viewpoint_mgr.GetNamedCamera3D(effCamera)) {
                                    newCamera = effCamera;
                                }
                                Replace(settings, ",B_CHOICE_PerPreviewCamera=" + effCamera,
                                        ",B_CHOICE_PerPreviewCamera=" + newCamera);
                            } else {
                                settings += ",B_CHOICE_PerPreviewCamera=" + mg->GetDefaultCamera();
                            }
                        }
                    }
                } else {
                    // cant see why this would happen
                    // gjones - I found this happening when importing a sequence along with timing layers...added code to skip timing layers to          avoid this assert in the debugger.
                    wxASSERT(false);
                }
            }

            Effect* ne = target->AddEffect(0, ef->GetEffectName(), settings, ef->GetPaletteAsString(),
                                           ef->GetStartTimeMS(), ef->GetEndTimeMS(), 0, false);
            if (lock) {
                ne->SetLocked(true);
            }
        }
    }
    mapped.push_back(src);
}

void MapXLightsStrandEffects(EffectLayer* target, const std::string& name,
                             std::map<std::string, EffectLayer*>& layerMap,
                             SequenceElements& seqEl,
                             std::vector<EffectLayer*>& mapped, bool eraseExisting,
                             SequencePackage& xsqPkg, bool lock, const std::map<std::string, std::string>& mapping)
{
    EffectLayer* src = layerMap[name];
    if (src == nullptr) {
        Element* srcEl = seqEl.GetElement(name);
        if (srcEl == nullptr) {
            printf("Source strand %s doesn't exist\n", name.c_str());
            return;
        }
        src = srcEl->GetEffectLayer(0);
    }
    if (src != nullptr) {
        MapXLightsEffects(target, src, mapped, eraseExisting, xsqPkg, lock, mapping);
    } else {
        printf("Source strand %s doesn't exist\n", name.c_str());
    }
}

void MapXLightsEffects(Element* target,
                       const std::string& name,
                       SequenceElements& seqEl,
                       std::map<std::string, Element*>& elementMap,
                       std::map<std::string, EffectLayer*>& layerMap,
                       std::vector<EffectLayer*>& mapped, bool eraseExisting,
                       SequencePackage& xsqPkg, bool lock, const std::map<std::string, std::string>& mapping)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (target->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
        wxString strandName = wxString::Format("Strand %d", ((StrandElement*)target)->GetStrand() + 1);
        logger_base.debug("Mapping xLights effect from %s to %s%s.", (const char*)name.c_str(), (const char*)target->GetFullName().c_str(), (const char*)strandName.c_str());
    } else {
        logger_base.debug("Mapping xLights effect from %s to %s.", (const char*)name.c_str(), (const char*)target->GetFullName().c_str());
    }

    EffectLayer* src = layerMap[name];
    Element* el = elementMap[name];

    if (src != nullptr) {
        MapXLightsEffects(target->GetEffectLayer(0), src, mapped, eraseExisting, xsqPkg, lock, mapping);
        return;
    }

    if (el == nullptr) {
        el = seqEl.GetElement(name);
    }

    if (el == nullptr) {
        logger_base.debug("Mapping xLights effect from %s to %s failed as the effect was not found in the source sequence.", (const char*)name.c_str(), (const char*)target->GetName().c_str());
        // printf("Source element %s doesn't exist\n", name.c_str());
        return;
    }

    while (target->GetEffectLayerCount() < el->GetEffectLayerCount()) {
        target->AddEffectLayer();
    }
    for (size_t x = 0; x < el->GetEffectLayerCount(); ++x) {
        MapXLightsEffects(target->GetEffectLayer(x), el->GetEffectLayer(x), mapped, eraseExisting, xsqPkg, lock, mapping);
    }
}

void xLightsFrame::ImportXLights(const wxFileName& filename, std::string const& mapFile)
{
    wxStopWatch sw; // start a stopwatch timer

    SequencePackage xsqPkg(filename, this);

    if (xsqPkg.IsPkg()) {
        xsqPkg.Extract();
    } else {
        xsqPkg.FindRGBEffectsFile();
    }

    if (!xsqPkg.IsValid() && xsqPkg.IsPkg()) {
        wxMessageBox("The file you are attempting to import doesn't appear to be a valid xLights Sequence Package.", "Invalid Sequence Package", wxICON_ERROR | wxOK);
        return;
    }

    xLightsXmlFile xlf(xsqPkg.GetXsqFile());
    xlf.Open(GetShowDirectory(), true);
    SequenceElements se(this);
    se.SetFrequency(_sequenceElements.GetFrequency());
    se.SetViewsManager(GetViewsManager()); // This must come first before LoadSequencerFile.
    se.LoadSequencerFile(xlf, GetShowDirectory(), true);
    xlf.AdjustEffectSettingsForVersion(se, this);

    bool supportsModelBlending = xlf.supportsModelBlending();

    std::vector<Element*> elements;
    for (size_t e = 0; e < se.GetElementCount(); ++e) {
        Element* el = se.GetElement(e);
        elements.push_back(el);
    }
    ImportXLights(se, elements, xsqPkg, supportsModelBlending, true, false, false, mapFile);

    float elapsedTime = sw.Time() / 1000.0; // msec => sec
    SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}

ModelElement* AddModel(Model* m, SequenceElements& se)
{
    if (m != nullptr) {
        se.AddMissingModelsToSequence(m->GetName(), false);
        ModelElement* model = dynamic_cast<ModelElement*>(se.GetElement(m->GetName()));
        model->Init(*m);
        return model;
    }
    return nullptr;
}

// backwards compatible for tabSequencer call
void xLightsFrame::ImportXLights(SequenceElements& se, const std::vector<Element*>& elements, const wxFileName& filename,
                                 bool modelBlending, bool showModelBlending, bool allowAllModels, bool clearSrc)
{
    SequencePackage xsqPkg(filename, this);
    ImportXLights(se, elements, xsqPkg, modelBlending, showModelBlending, allowAllModels, clearSrc);
}

void xLightsFrame::ImportXLights(SequenceElements& se, const std::vector<Element*>& elements, SequencePackage& xsqPkg,
                                 bool modelBlending, bool showModelBlending, bool allowAllModels, bool clearSrc, std::string const& mapFile)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    std::map<std::string, EffectLayer*> layerMap;
    std::map<std::string, Element*> elementMap;
    xLightsImportChannelMapDialog dlg(this, xsqPkg.GetXsqFile(), false, true, false, false, showModelBlending);
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;
    if (showModelBlending) {
        dlg.SetModelBlending(modelBlending);
    }

    dlg.SetXsqPkg(&xsqPkg);

    std::vector<EffectLayer*> mapped;
    std::vector<std::string> timingTrackNames;
    std::map<std::string, bool> timingTrackAlreadyExists;
    std::map<std::string, TimingElement*> timingTracks;

    for (const auto& it : elements) {
        Element* e = it;
        if (e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            ModelElement* el = dynamic_cast<ModelElement*>(e);
            bool hasEffects{ false };

            for (size_t l = 0; l < el->GetEffectLayerCount(); ++l) {
                hasEffects |= el->GetEffectLayer(l)->GetEffectCount() > 0;
            }
            if (hasEffects) {
                dlg.AddChannel(el->GetName(), el->GetEffectCount());
            }
            elementMap[el->GetName()] = el;
            int s = 0;
            for (size_t sm = 0; sm < el->GetSubModelAndStrandCount(); ++sm) {
                SubModelElement* sme = el->GetSubModel(sm);

                StrandElement* ste = dynamic_cast<StrandElement*>(sme);
                std::string smName = sme->GetName();
                if (ste != nullptr) {
                    ++s;
                    if (smName == "") {
                        smName = wxString::Format("Strand %d", s);
                    }
                }
                if (sme->HasEffects()) {
                    elementMap[el->GetName() + "/" + smName] = sme;
                    dlg.AddChannel(el->GetName() + "/" + smName, sme->GetEffectCount());
                }
                if (ste != nullptr) {
                    for (size_t n = 0; n < ste->GetNodeLayerCount(); ++n) {
                        NodeLayer* nl = ste->GetNodeLayer(n, true);
                        if (nl->GetEffectCount() > 0) {
                            std::string nodeName = nl->GetName();
                            if (nodeName == "") {
                                nodeName = wxString::Format("Node %d", (int)(n + 1));
                            }
                            dlg.AddChannel(el->GetName() + "/" + smName + "/" + nodeName, nl->GetEffectCount());
                            layerMap[el->GetName() + "/" + smName + "/" + nodeName] = nl;
                        }
                    }
                }
            }
        } else if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            TimingElement* tel = dynamic_cast<TimingElement*>(e);
            bool hasEffects{ false };
            for (size_t n = 0; n < tel->GetEffectLayerCount(); ++n) {
                hasEffects |= tel->GetEffectLayer(n)->GetEffectCount() > 0;
            }
            if (hasEffects) {
                timingTrackNames.push_back(tel->GetName());
                timingTracks[tel->GetName()] = tel;

                // we want to know which timing tracks exist so we can preselect the ones which are not already present
                // a timing track is only considered to exist if it has at least one timing mark
                timingTrackAlreadyExists[tel->GetName()] = (_sequenceElements.GetTimingElement(tel->GetName()) != nullptr && _sequenceElements.GetTimingElement(tel->GetName())->HasEffects());
            }
        }
    }

    dlg.SortChannels();
    dlg.timingTracks = timingTrackNames;
    dlg.timingTrackAlreadyExists = timingTrackAlreadyExists;
    bool ok = dlg.InitImport();

    if (mapFile.empty()) {
        if (!ok || dlg.ShowModal() != wxID_OK) {
            return;
        }
    } else {
        dlg.LoadMappingFile(mapFile, true);
    }

    if (showModelBlending && dlg.GetImportModelBlending()) {
        CurrentSeqXmlFile->setSupportsModelBlending(modelBlending);
        GetSequenceElements().SetSupportsModelBlending(modelBlending);
    }

    bool lock = dlg.IsLockEffects();

    // if the user is importing at least one timing element and the current sequence only has one timing track called New Timing with no timing marks in it ...
    if (dlg.TimingTrackListBox->GetCount() > 0 && _sequenceElements.GetNumberOfTimingElements() == 1 && _sequenceElements.GetTimingElement("New Timing") != nullptr && !_sequenceElements.GetTimingElement("New Timing")->HasEffects()) {
        // Delete the New Timing timing track
        _sequenceElements.DeleteElement("New Timing");
    }

    // build a mapping table so we can use it to fix up any 'Duplicate' Effects
    std::map<std::string, std::string> mapping;
    for (size_t i = 0; i < dlg._dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = dlg._dataModel->GetNthChild(i);
        if (m->_mapping != "") {
            mapping[m->_mapping] = m->_model.ToStdString();
        }
    }

    for (size_t tt = 0; tt < dlg.TimingTrackListBox->GetCount(); ++tt) {
        if (dlg.TimingTrackListBox->IsChecked(tt)) {
            TimingElement* tel = timingTracks[timingTrackNames[tt]];
            TimingElement* target = nullptr;
            if (_sequenceElements.GetTimingElement(tel->GetName()) != nullptr) {
                if (_sequenceElements.GetTimingElement(tel->GetName())->GetEffectLayer(0)->GetEffectCount() == 0) {
                    target = _sequenceElements.GetTimingElement(tel->GetName());
                }
            }

            if (target == nullptr) {
                target = static_cast<TimingElement*>(_sequenceElements.AddElement(tel->GetName(), "timing", true, tel->GetCollapsed(), tel->GetActive(), false, false));
                char cnt = '1';
                while (target == nullptr) {
                    target = static_cast<TimingElement*>(_sequenceElements.AddElement(tel->GetName() + "-" + cnt++, "timing", true, tel->GetCollapsed(), tel->GetActive(), false, false));
                }
            }

            for (int l = 0; l < tel->GetEffectLayerCount(); ++l) {
                EffectLayer* src = tel->GetEffectLayer(l);
                while (l >= target->GetEffectLayerCount()) {
                    target->AddEffectLayer();
                }
                EffectLayer* dst = target->GetEffectLayer(l);
                std::vector<EffectLayer*> mapped2;
                MapXLightsEffects(dst, src, mapped2, dlg.CheckBox_EraseExistingEffects->GetValue(), xsqPkg, lock, mapping);
            }
        }
    }

    for (size_t i = 0; i < dlg._dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = dlg._dataModel->GetNthChild(i);
        std::string modelName = m->_model.ToStdString();
        ModelElement* model = nullptr;
        for (size_t x = 0; x < _sequenceElements.GetElementCount(); ++x) {
            if (_sequenceElements.GetElement(x)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(x)->GetName()) {
                model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(x));
                break;
            }
        }
        if (m->_mapping != "") {
            if (model == nullptr) {
                model = AddModel(GetModel(modelName), _sequenceElements);
            }
            if (model == nullptr) {
                logger_base.error("Attempt to add model %s during xLights import failed.", (const char*)modelName.c_str());
            } else {
                MapXLightsEffects(model, m->_mapping.ToStdString(), se, elementMap, layerMap, mapped, dlg.CheckBox_EraseExistingEffects->GetValue(), xsqPkg, lock, mapping);
            }
        }

        int str = 0;
        for (size_t j = 0; j < m->GetChildCount(); j++) {
            xLightsImportModelNode* s = m->GetNthChild(j);

            if ("" != s->_mapping) {
                if (model == nullptr) {
                    model = AddModel(GetModel(modelName), _sequenceElements);
                }
                if (model == nullptr) {
                    logger_base.error("Attempt to add model %s during xLights import failed.", (const char*)modelName.c_str());
                } else {
                    SubModelElement* ste = model->GetSubModel(str);
                    if (ste != nullptr) {
                        MapXLightsEffects(ste, s->_mapping.ToStdString(), se, elementMap, layerMap, mapped, dlg.CheckBox_EraseExistingEffects->GetValue(), xsqPkg, lock, mapping);
                    }
                }
            }
            for (size_t n = 0; n < s->GetChildCount(); n++) {
                xLightsImportModelNode* ns = s->GetNthChild(n);
                if ("" != ns->_mapping) {
                    if (model == nullptr) {
                        model = AddModel(GetModel(modelName), _sequenceElements);
                    }
                    if (model == nullptr) {
                        logger_base.error("Attempt to add model %s during xLights import failed.", (const char*)modelName.c_str());
                    } else {
                        SubModelElement* ste = model->GetSubModel(str);
                        StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                        if (stre != nullptr) {
                            NodeLayer* nl = stre->GetNodeLayer(n, true);
                            if (nl != nullptr) {
                                MapXLightsStrandEffects(nl, ns->_mapping.ToStdString(), layerMap, se, mapped, dlg.CheckBox_EraseExistingEffects->GetValue(), xsqPkg, lock, mapping);
                            }
                        }
                    }
                }
            }
            str++;
        }
    }

    if (clearSrc) {
        for (const auto& it : mapped) {
            it->RemoveAllEffects(nullptr);
        }
    }

    if (xsqPkg.IsPkg()) {
        if (xsqPkg.HasMissingMedia()) {
            wxString missingAssets;
            for (const auto& missingAsset : xsqPkg.GetMissingMedia()) {
                missingAssets = missingAssets + wxString::Format("%s- %s\n", "    ", missingAsset);
            }

            wxString msgP1 = "The following assets were missing from the Sequence Package and could not be imported.";
            wxString msgP2 = "Once you source them, place them in your show folder and use 'Import Effects' again making sure to select 'Erase existing effects on imported models'";
            wxString msgP3 = "or update the effects individually.";

            wxMessageBox(wxString::Format("%s %s %s\n\n%s", msgP1, msgP2, msgP3, missingAssets), "Missing Assets", wxICON_WARNING | wxOK, this);
        }

        if (xsqPkg.ModelsChanged()) {
            GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "xLightsFrame::ImportXLights");
        }
    }
}

void MapToStrandName(const std::string& name, std::vector<std::string>& strands)
{
    if (name.find("_") != name.npos) {
        int idx = name.find("_") + 1;
        // maybe map to a strand?  Name_0001, Name_0002... etc...

        int ppos = -1;
        int spos = -1;
        for (int x = idx; x < name.size(); x++) {
            if (name[x] == 'P') {
                ppos = x;
            } else if (name[x] == 'S') {
                spos = x;
            } else if (name[x] < '0' || name[x] > '9') {
                return;
            }
        }
        std::string strandName;
        if (spos == -1 && ppos == -1) {
            // simple "strand" names of _#####
            strandName = name.substr(0, name.find("_") - 1);
        } else if (spos >= 0 && ppos > spos) {
            // more complex of _S###P###
            strandName = name.substr(0, ppos - 1);
        }
        if ("" != strandName && std::find(strands.begin(), strands.end(), strandName) == strands.end()) {
            strands.push_back(strandName);
        }
    }
}

void ReadHLSData(wxXmlNode* chand, std::vector<unsigned char>& data)
{
    for (wxXmlNode* chani = chand->GetChildren(); chani != nullptr; chani = chani->GetNext()) {
        if ("IlluminationData" == chani->GetName()) {
            for (wxXmlNode* block = chani->GetChildren(); block != nullptr; block = block->GetNext()) {
                wxString vals = block->GetChildren()->GetContent();
                int offset = wxAtoi(vals.SubString(0, vals.Find("-")));
                vals = vals.SubString(vals.Find("-") + 1, vals.size());
                while (!vals.IsEmpty()) {
                    wxString v = vals.BeforeFirst(',');
                    vals = vals.AfterFirst(',');
                    long iv = 0;
                    v.ToLong(&iv, 16);
                    data[offset] = iv;
                    offset++;
                }
            }
        }
    }
}

void MapHLSChannelInformation(xLightsFrame* xlights, EffectLayer* layer, wxXmlNode* tuniv, int frames, int frameTime,
                              const wxString& cn, wxColor color, Model& mc, bool byStrand, bool eraseExisting)
{
    if (cn == "") {
        return;
    }
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxXmlNode* redNode = nullptr;
    wxXmlNode* greenNode = nullptr;
    wxXmlNode* blueNode = nullptr;

    for (wxXmlNode* univ = tuniv->GetChildren(); univ != nullptr; univ = univ->GetNext()) {
        if (univ->GetName() == "Universe") {
            for (wxXmlNode* channels = univ->GetChildren(); channels != nullptr; channels = channels->GetNext()) {
                if (channels->GetName() == "Channels") {
                    for (wxXmlNode* chand = channels->GetChildren(); chand != nullptr; chand = chand->GetNext()) {
                        if (chand->GetName() == "ChannelData") {
                            for (wxXmlNode* chani = chand->GetChildren(); chani != nullptr; chani = chani->GetNext()) {
                                if (chani->GetName() == "ChanInfo") {
                                    wxString info = chani->GetChildren()->GetContent();
                                    if (info == cn + ", Normal") {
                                        // single channel, easy
                                        redNode = chand;
                                    } else if (info == cn + ", RGB-R") {
                                        redNode = chand;
                                    } else if (info == cn + ", RGB-G") {
                                        greenNode = chand;
                                    } else if (info == cn + ", RGB-B") {
                                        blueNode = chand;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (redNode == nullptr) {
        printf("Did not map %s\n", (const char*)cn.c_str());
        logger_base.info("Did not map " + cn);
        return;
    }
    std::vector<unsigned char> redData(frames);
    std::vector<unsigned char> greenData(frames);
    std::vector<unsigned char> blueData(frames);
    std::vector<xlColor> colors(frames);
    ReadHLSData(redNode, redData);
    if (greenNode != nullptr) {
        ReadHLSData(greenNode, greenData);
        ReadHLSData(blueNode, blueData);
        for (int x = 0; x < frames; x++) {
            colors[x].Set(redData[x], greenData[x], blueData[x]);
        }
    } else {
        xlColor c(color.Red(), color.Green(), color.Blue());
        HSVValue hsv = c.asHSV();
        for (int x = 0; x < frames; x++) {
            int i = redData[x];
            // for ramps up/down, HLS does a 1%-100% so the first cell is not linear and
            // we end up not able to map the ramps, we'll try and detect that here
            // and change to 0
            if (i <= 3 && i > 0) {
                if (x < (frames - 4)) {
                    if (i < redData[x + 1] && redData[x + 1] < redData[x + 2] && redData[x + 2] < redData[x + 3]) {
                        i = 0;
                    }
                }
                if (x > 4) {
                    if (i < redData[x - 1] && redData[x - 1] < redData[x - 2] && redData[x - 2] < redData[x - 3]) {
                        i = 0;
                    }
                }
            }
            hsv.value = ((double)i) / 255.0;
            colors[x] = hsv;
        }
    }
    xlights->DoConvertDataRowToEffects(layer, colors, frameTime, eraseExisting);
}

std::string FindHLSStrandName(const std::string& ccrName, int node, const std::vector<std::string>& channelNames)
{
    std::string r = ccrName + wxString::Format("P%03d", node).ToStdString();
    if (std::find(channelNames.begin(), channelNames.end(), r) == channelNames.end()) {
        r = ccrName + wxString::Format("P%04d", node);
    } else {
        return r;
    }
    if (std::find(channelNames.begin(), channelNames.end(), r) == channelNames.end()) {
        r = ccrName + wxString::Format("P%02d", node);
    } else {
        return r;
    }
    if (std::find(channelNames.begin(), channelNames.end(), r) == channelNames.end()) {
        r = ccrName + wxString::Format("_%04d", node);
    } else {
        return r;
    }
    if (std::find(channelNames.begin(), channelNames.end(), r) == channelNames.end()) {
        r = ccrName + wxString::Format("_%03d", node);
    } else {
        return r;
    }
    if (std::find(channelNames.begin(), channelNames.end(), r) == channelNames.end()) {
        return r;
    }
    return "";
}

bool Contains(const std::vector<std::string>& array, const std::string& str)
{
    return std::find(array.begin(), array.end(), str) != array.end();
}

int Index(const std::vector<std::string>& array, const std::string& str)
{
    auto it = std::find(array.begin(), array.end(), str);
    if (it == array.end()) {
        return -1;
    }
    return it - array.begin();
}
void MapVixChannelInformation(xLightsFrame* xlights, EffectLayer* layer,
                              std::vector<unsigned char>& data,
                              int frameTime,
                              int numFrames,
                              const std::string& channelName,
                              const std::vector<std::string>& channels,
                              wxColor color,
                              Model& mc, bool eraseExisting)
{
    if (channelName == "") {
        return;
    }
    int channel = Index(channels, channelName);
    xlColorVector colors(numFrames);
    if (channel == wxNOT_FOUND) {
        int rchannel = Index(channels, channelName + "Red");
        if (rchannel == wxNOT_FOUND) {
            rchannel = Index(channels, channelName + "-R");
        }
        int gchannel = Index(channels, channelName + "Green");
        if (gchannel == wxNOT_FOUND) {
            gchannel = Index(channels, channelName + "-G");
        }
        int bchannel = Index(channels, channelName + "Blue");
        if (bchannel == wxNOT_FOUND) {
            bchannel = Index(channels, channelName + "-B");
        }
        if (rchannel == wxNOT_FOUND || gchannel == wxNOT_FOUND || bchannel == wxNOT_FOUND) {
            return;
        }
        for (int x = 0; x < numFrames; x++) {
            colors[x].Set(data[x + numFrames * rchannel], data[x + numFrames * gchannel], data[x + numFrames * bchannel]);
        }
    } else {
        xlColor c(color.Red(), color.Green(), color.Blue());
        HSVValue hsv = c.asHSV();
        for (int x = 0; x < numFrames; x++) {
            hsv.value = ((double)data[x + numFrames * channel]) / 255.0;
            colors[x] = hsv;
        }
    }
    xlights->DoConvertDataRowToEffects(layer, colors, frameTime, eraseExisting);
}

// xml
#include "../include/spxml-0.5/spxmlevent.hpp"
#include "../include/spxml-0.5/spxmlparser.hpp"
#ifndef MAX_READ_BLOCK_SIZE
#define MAX_READ_BLOCK_SIZE 4096 * 1024
#endif

static void CheckForVixenRGB(const std::string& name, xlColor& c, xLightsImportChannelMapDialog& dlg)
{
    auto const channelNames{ dlg.GetChannelNames() };
    bool addRGB = false;
    std::string base;
    if (EndsWith(name, "Red") || EndsWith(name, "-R")) {
        c = xlRED;
        if (EndsWith(name, "-R")) {
            base = name.substr(0, name.size() - 2);
        } else {
            base = name.substr(0, name.size() - 3);
        }
        if ((Contains(channelNames, base + "Blue") && Contains(channelNames, base + "Green")) || (Contains(channelNames, base + "-B") && Contains(channelNames, base + "-G"))) {
            addRGB = true;
        }
    } else if (EndsWith(name, "Blue") || EndsWith(name, "-B")) {
        c = xlBLUE;
        if (EndsWith(name, "-B")) {
            base = name.substr(0, name.size() - 2);
        } else {
            base = name.substr(0, name.size() - 4);
        }
        if ((Contains(channelNames, base + "Red") && Contains(channelNames, base + "Green")) || (Contains(channelNames, base + "-R") && Contains(channelNames, base + "-G"))) {
            addRGB = true;
        }
    } else if (EndsWith(name, "Green") || EndsWith(name, "-G")) {
        c = xlGREEN;
        if (EndsWith(name, "-G")) {
            base = name.substr(0, name.size() - 2);
        } else {
            base = name.substr(0, name.size() - 5);
        }
        if ((Contains(channelNames, base + "Blue") && Contains(channelNames, base + "Red")) || (Contains(channelNames, base + "-B") && Contains(channelNames, base + "-R"))) {
            addRGB = true;
        }
    }
    dlg.channelColors[name] = c;
    if (addRGB) {
        dlg.channelColors[base] = xlBLACK;
        dlg.AddChannel(base);
    }
}

std::string SafeGetAttrValue(SP_XmlStartTagEvent* event, const char* name)
{
    const char* p = event->getAttrValue(name);

    if (p == nullptr) {
        return "";
    } else {
        return std::string(p);
    }
}

void xLightsFrame::ImportVix(const wxFileName& filename)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxStopWatch sw; // start a stopwatch timer

    logger_base.debug("Importing vixen file %s.", (const char*)filename.GetFullName().c_str());

    std::vector<unsigned char> VixSeqData;
    std::vector<std::string> context;

    int time = 0;
    int frameTime = 50;

    xLightsImportChannelMapDialog dlg(this, filename, false, false, true, true, false);
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;

    SP_XmlPullParser* parser = new SP_XmlPullParser();
    parser->setMaxTextSize(MAX_READ_BLOCK_SIZE / 2);
    wxFile file(filename.GetFullPath());
    char* bytes = new char[MAX_READ_BLOCK_SIZE];
    size_t read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
    parser->append(bytes, read);
    wxString carryOver;

    std::vector<std::string> unsortedChannels;

    int chanColor = -1;

    // pass 1, read the length, determine number of networks, units/network, channels per unit
    logger_base.debug("Reading vixen file.");
    SP_XmlPullEvent* event = parser->getNext();
    int done = 0;
    long cnt = 0;
    while (!done) {
        if (!event) {
            read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
            if (read == 0) {
                done = true;
            } else {
                parser->append(bytes, read);
            }
        } else {
            switch (event->getEventType()) {
            case SP_XmlPullEvent::eEndDocument:
                done = true;
                break;
            case SP_XmlPullEvent::eStartTag: {
                SP_XmlStartTagEvent* stagEvent = (SP_XmlStartTagEvent*)event;
                std::string NodeName = stagEvent->getName();
                context.push_back(NodeName);
                cnt++;
                if (cnt > 1 && context[1] == "Channels" && NodeName == "Channel") {
                    chanColor = wxAtoi(stagEvent->getAttrValue("color")) & 0xFFFFFF;
                    NodeName = SafeGetAttrValue(stagEvent, "name");
                    if (NodeName != "") {
                        dlg.AddChannel(NodeName);
                        unsortedChannels.push_back(NodeName);

                        xlColor c(chanColor, false);
                        CheckForVixenRGB(NodeName, c, dlg);

                        context.pop_back();
                        context.push_back("IgnoreChannelElement");
                    }
                }
            } break;
            case SP_XmlPullEvent::eCData: {
                SP_XmlCDataEvent* stagEvent = (SP_XmlCDataEvent*)event;
                if (cnt >= 2) {
                    std::string NodeValue = stagEvent->getText();
                    if (context[1] == "MaximumLevel") {
                        // MaxIntensity = wxAtoi(NodeValue);
                    } else if (context[1] == "EventPeriodInMilliseconds") {
                        frameTime = wxAtoi(NodeValue);
                    } else if (context[1] == "Time") {
                        time = wxAtoi(NodeValue);
                    } else if (context[1] == "Profile") {
                        wxArrayInt VixChannels;
                        wxArrayString VixChannelNames;
                        SequenceData seqData;
                        ConvertParameters params(filename.GetFullPath(),
                                                 seqData,
                                                 nullptr,
                                                 ConvertParameters::ReadMode::READ_MODE_NORMAL,
                                                 this,
                                                 nullptr,
                                                 nullptr);

                        std::vector<xlColor> colors;
                        FileConverter::LoadVixenProfile(params, NodeValue, VixChannels, VixChannelNames, colors);
                        for (int x = 0; x < VixChannelNames.size(); x++) {
                            std::string name = VixChannelNames[x].ToStdString();
                            xlColor c = colors[x];
                            dlg.AddChannel(name);
                            unsortedChannels.push_back(name);

                            CheckForVixenRGB(name, c, dlg);
                        }

                    } else if (context[1] == "EventValues") {
                        // AppendConvertStatus(string_format(wxString("Chunk Size=%d\n"), NodeValue.size()));
                        if (carryOver.size() > 0) {
                            NodeValue.insert(0, carryOver);
                        }
                        int i = base64_decode(NodeValue, VixSeqData);
                        if (i != 0) {
                            int start = NodeValue.size() - i - 1;
                            carryOver = NodeValue.substr(start, start + i);
                        } else {
                            carryOver.clear();
                        }
                    } else if (context[1] == "Channels" && context[2] == "Channel") {
                        dlg.AddChannel(NodeValue);
                        unsortedChannels.push_back(NodeValue);

                        xlColor c(chanColor, false);
                        CheckForVixenRGB(NodeValue, c, dlg);
                    }
                }
                break;
            }
            case SP_XmlPullEvent::eEndTag:
                if (cnt > 0) {
                    context.pop_back();
                }
                cnt = context.size();
                break;
            }
            delete event;
        }
        if (!done) {
            event = parser->getNext();
        }
    }
    delete[] bytes;
    delete parser;
    file.Close();

    // I added the ceiling command because i had an example file that ended up one calculating number of frames one less than
    // the previous calculation because it had a partial last frame
    int numFrames = (int)std::ceil((float)time / (float)frameTime);

    dlg.SortChannels();

    logger_base.debug("Showing mapping dialog.");
    dlg.InitImport();

    if (dlg.ShowModal() != wxID_OK || dlg._dataModel == nullptr) {
        return;
    }

    logger_base.debug("Doing the import of the mapped channels.");
    for (size_t i = 0; i < dlg._dataModel->GetChildCount(); i++) {
        xLightsImportModelNode* m = dlg._dataModel->GetNthChild(i);
        std::string modelName = m->_model.ToStdString();
        Model* mc = GetModel(modelName);
        ModelElement* model = nullptr;
        for (size_t x = 0; x < _sequenceElements.GetElementCount(); x++) {
            if (_sequenceElements.GetElement(x)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(x)->GetName()) {
                model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(x));
                break;
            }
        }
        if (m->_mapping != "") {
            if (model == nullptr) {
                model = AddModel(GetModel(modelName), _sequenceElements);
            }
            if (model == nullptr) {
                logger_base.error("Attempt to add model %s during Vixen import failed.", (const char*)modelName.c_str());
            } else {
                MapVixChannelInformation(this, model->GetEffectLayer(0),
                                         VixSeqData, frameTime, numFrames,
                                         m->_mapping.ToStdString(),
                                         unsortedChannels,
                                         m->_color,
                                         *mc, dlg.CheckBox_EraseExistingEffects->GetValue());
            }
        }

        int str = 0;
        for (size_t j = 0; j < m->GetChildCount(); j++) {
            xLightsImportModelNode* s = m->GetNthChild(j);

            if ("" != s->_mapping) {
                if (model == nullptr) {
                    model = AddModel(GetModel(modelName), _sequenceElements);
                }
                if (model == nullptr) {
                    logger_base.error("Attempt to add model %s during Vixen import failed.", (const char*)modelName.c_str());
                } else {
                    SubModelElement* ste = model->GetSubModel(str);
                    if (ste != nullptr) {
                        MapVixChannelInformation(this, ste->GetEffectLayer(0),
                                                 VixSeqData, frameTime, numFrames,
                                                 s->_mapping.ToStdString(),
                                                 unsortedChannels,
                                                 s->_color, *mc, dlg.CheckBox_EraseExistingEffects->GetValue());
                    }
                }
            }
            for (size_t n = 0; n < s->GetChildCount(); n++) {
                xLightsImportModelNode* ns = s->GetNthChild(n);
                if ("" != ns->_mapping) {
                    if (model == nullptr) {
                        model = AddModel(GetModel(modelName), _sequenceElements);
                    }
                    if (model == nullptr) {
                        logger_base.error("Attempt to add model %s during Vixen import failed.", (const char*)modelName.c_str());
                    } else {
                        SubModelElement* ste = model->GetSubModel(str);
                        StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                        if (stre != nullptr) {
                            NodeLayer* nl = stre->GetNodeLayer(n, true);
                            if (nl != nullptr) {
                                MapVixChannelInformation(this, nl,
                                                         VixSeqData, frameTime, numFrames,
                                                         ns->_mapping.ToStdString(),
                                                         unsortedChannels,
                                                         ns->_color, *mc, dlg.CheckBox_EraseExistingEffects->GetValue());
                            }
                        }
                    }
                }
            }
            str++;
        }
    }

    float elapsedTime = sw.Time() / 1000.0; // msec => sec
    SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}

void xLightsFrame::ImportHLS(const wxFileName& filename)
{
    wxStopWatch sw; // start a stopwatch timer

    wxFileName xml_file(filename);
    wxXmlDocument input_xml;
    wxString xml_doc = xml_file.GetFullPath();
    wxFileInputStream fin(xml_doc);

    if (!input_xml.Load(fin))
        return;

    LMSImportChannelMapDialog dlg(this, filename);
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;

    /*
     </ChannelData> | </IlluminationData>
     </Channels>
     </Universe>
     </TotalUniverses> | NumberOfTimeCells | MilliSecPerTimeUnit
     </HLS_OutputSequence>
     */
    int frames = 0;
    int frameTime = 0;
    wxXmlNode* totalUniverses = nullptr;
    for (wxXmlNode* tuniv = input_xml.GetRoot()->GetChildren(); tuniv != nullptr; tuniv = tuniv->GetNext()) {
        if (tuniv->GetName() == "NumberOfTimeCells") {
            frames = wxAtoi(tuniv->GetChildren()->GetContent());
        } else if (tuniv->GetName() == "MilliSecPerTimeUnit") {
            frameTime = wxAtoi(tuniv->GetChildren()->GetContent());
        } else if (tuniv->GetName() == "TotalUniverses") {
            totalUniverses = tuniv;
            for (wxXmlNode* univ = tuniv->GetChildren(); univ != nullptr; univ = univ->GetNext()) {
                if (univ->GetName() == "Universe") {
                    for (wxXmlNode* channels = univ->GetChildren(); channels != nullptr; channels = channels->GetNext()) {
                        if (channels->GetName() == "Channels") {
                            for (wxXmlNode* chand = channels->GetChildren(); chand != nullptr; chand = chand->GetNext()) {
                                if (chand->GetName() == "ChannelData") {
                                    for (wxXmlNode* chani = chand->GetChildren(); chani != nullptr; chani = chani->GetNext()) {
                                        if (chani->GetName() == "ChanInfo") {
                                            std::string info = chani->GetChildren()->GetContent().ToStdString();
                                            if (info.find(", Normal") != info.npos) {
                                                std::string name = info.substr(0, info.find(", Normal"));
                                                dlg.channelNames.push_back(name);
                                                dlg.channelColors[name] = xlWHITE;
                                                MapToStrandName(name, dlg.ccrNames);
                                            } else if (info.find(", RGB-") != info.npos) {
                                                std::string name = info.substr(0, info.find(", RGB-"));
                                                std::string color = info.substr(info.size() - 1, 1);
                                                if (color == "R") {
                                                    dlg.channelNames.push_back(name);
                                                    dlg.channelColors[name] = xlBLACK;
                                                }
                                                dlg.channelNames.push_back(info);
                                                if (color == "R") {
                                                    dlg.channelColors[info] = xlRED;
                                                } else if (color == "G") {
                                                    dlg.channelColors[info] = xlGREEN;
                                                } else {
                                                    dlg.channelColors[info] = xlBLUE;
                                                }
                                                MapToStrandName(name, dlg.ccrNames);
                                            } // else "Dead Channel"
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    std::sort(dlg.channelNames.begin(), dlg.channelNames.end(), stdlistNumberAwareStringCompare);
    dlg.channelNames.insert(dlg.channelNames.begin(), "");
    std::sort(dlg.ccrNames.begin(), dlg.ccrNames.end(), stdlistNumberAwareStringCompare);
    dlg.ccrNames.insert(dlg.ccrNames.begin(), "");

    dlg.Init();

    if (dlg.ShowModal() != wxID_OK) {
        return;
    }

    int row = 0;
    for (size_t m = 0; m < dlg.modelNames.size(); m++) {
        std::string modelName = dlg.modelNames[m];
        Model* mc = GetModel(modelName);
        ModelElement* model = nullptr;
        for (size_t i = 0; i < _sequenceElements.GetElementCount(); i++) {
            if (_sequenceElements.GetElement(i)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(i)->GetName()) {
                model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(i));
            }
        }
        MapHLSChannelInformation(this, model->GetEffectLayer(0),
                                 totalUniverses, frames, frameTime,
                                 dlg.ChannelMapGrid->GetCellValue(row, 3),
                                 dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                 *mc, dlg.MapByStrand->GetValue(), false /*dlg.CheckBox_EraseExisting()->GetValue()*/);
        row++;

        if (!dlg.MapByStrand->GetValue()) {
            for (int str = 0; str < mc->GetNumSubModels(); str++) {
                SubModelElement* se = model->GetSubModel(str);
                EffectLayer* sl = se->GetEffectLayer(0);

                MapHLSChannelInformation(this, sl,
                                         totalUniverses, frames, frameTime,
                                         dlg.ChannelMapGrid->GetCellValue(row, 3),
                                         dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                         *mc, false, false /*dlg.CheckBox_EraseExisting()->GetValue()*/);
                row++;
            }
        }
        for (int str = 0; str < mc->GetNumStrands(); str++) {
            StrandElement* se = model->GetStrand(str, true);
            EffectLayer* sl = se->GetEffectLayer(0);

            if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                if (!dlg.MapByStrand->GetValue()) {
                    MapHLSChannelInformation(this, sl,
                                             totalUniverses, frames, frameTime,
                                             dlg.ChannelMapGrid->GetCellValue(row, 3),
                                             dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                             *mc, false, false /*dlg.CheckBox_EraseExisting()->GetValue()*/);
                } else {
                    std::string ccrName = dlg.ChannelMapGrid->GetCellValue(row, 3).ToStdString();
                    for (int n = 0; n < se->GetNodeLayerCount(); n++) {
                        EffectLayer* layer = se->GetNodeLayer(n, true);

                        wxString nm = FindHLSStrandName(ccrName, n + 1, dlg.channelNames);

                        MapHLSChannelInformation(this, layer,
                                                 totalUniverses, frames, frameTime,
                                                 nm,
                                                 dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                                 *mc, true, false /*dlg.CheckBox_EraseExisting()->GetValue()*/);
                    }
                }
            }
            row++;
            if (!dlg.MapByStrand->GetValue()) {
                for (int n = 0; n < mc->GetStrandLength(str); n++) {
                    if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                        MapHLSChannelInformation(this, se->GetNodeLayer(n, true),
                                                 totalUniverses, frames, frameTime,
                                                 dlg.ChannelMapGrid->GetCellValue(row, 3),
                                                 dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                                 *mc, false, false /*dlg.CheckBox_EraseExisting()->GetValue()*/);
                    }
                    row++;
                }
            }
        }
    }

    float elapsedTime = sw.Time() / 1000.0; // msec => sec
    SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}

void xLightsFrame::ImportLMS(const wxFileName& filename)
{
    wxStopWatch sw; // start a stopwatch timer

    wxFileName xml_file(filename);
    wxXmlDocument input_xml;
    wxString xml_doc = xml_file.GetFullPath();
    wxFileInputStream fin(xml_doc);

    if (!input_xml.Load(fin))
        return;
    ImportLMS(input_xml, filename);
    float elapsedTime = sw.Time() / 1000.0; // msec => sec
    SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}

void xLightsFrame::ImportLPE(const wxFileName& filename)
{
    wxStopWatch sw; // start a stopwatch timer

    wxFileName xml_file(filename);
    wxXmlDocument input_xml;
    wxString xml_doc = xml_file.GetFullPath();
    wxFileInputStream fin(xml_doc);

    if (!input_xml.Load(fin))
        return;
    ImportLPE(input_xml, filename);
    float elapsedTime = sw.Time() / 1000.0; // msec => sec
    SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}

void xLightsFrame::ImportS5(const wxFileName& filename)
{
    wxStopWatch sw; // start a stopwatch timer

    wxFileName xml_file(filename);
    wxXmlDocument input_xml;
    wxString xml_doc = xml_file.GetFullPath();
    wxFileInputStream fin(xml_doc);

    if (!input_xml.Load(fin))
        return;
    ImportS5(input_xml, filename);
    float elapsedTime = sw.Time() / 1000.0; // msec => sec
    SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}

void AdjustAllTimings(wxXmlNode* input_xml, int offset)
{
    if (input_xml->HasAttribute("startCentisecond")) {
        int i = wxAtoi(input_xml->GetAttribute("startCentisecond"));
        input_xml->DeleteAttribute("startCentisecond");
        input_xml->AddAttribute("startCentisecond", wxString::Format(wxT("%i"), i + offset));
    }
    if (input_xml->HasAttribute("endCentisecond")) {
        int i = wxAtoi(input_xml->GetAttribute("endCentisecond"));
        input_xml->DeleteAttribute("endCentisecond");
        input_xml->AddAttribute("endCentisecond", wxString::Format(wxT("%i"), i + offset));
    }
    if (input_xml->GetName() == "flowy") {
        if (input_xml->HasAttribute("startTime")) {
            int i = wxAtoi(input_xml->GetAttribute("startTime"));
            input_xml->DeleteAttribute("startTime");
            input_xml->AddAttribute("startTime", wxString::Format("%d", i + offset));
        }
        if (input_xml->HasAttribute("endTime")) {
            int i = wxAtoi(input_xml->GetAttribute("endTime"));
            input_xml->DeleteAttribute("endTime");
            input_xml->AddAttribute("endTime", wxString::Format("%d", i + offset));
        }
    } else if (input_xml->GetName() == "state1" || input_xml->GetName() == "state2") {
        if (input_xml->HasAttribute("time")) {
            int i = wxAtoi(input_xml->GetAttribute("time"));
            input_xml->DeleteAttribute("time");
            input_xml->AddAttribute("time", wxString::Format("%d", i + offset));
        }
    }
    for (wxXmlNode* chan = input_xml->GetChildren(); chan != nullptr; chan = chan->GetNext()) {
        AdjustAllTimings(chan, offset);
    }
}

void xLightsFrame::ImportSuperStar(const wxFileName& filename)
{
    SuperStarImportDialog dlg(this);

    for (size_t i = 0; i < _sequenceElements.GetElementCount(); i++) {
        if (_sequenceElements.GetElement(i)->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            dlg.ChoiceSuperStarImportModel->Append(_sequenceElements.GetElement(i)->GetName());

            ModelElement* model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(i));
            for (int x = 0; x < model->GetSubModelAndStrandCount(); x++) {
                std::string fname = model->GetSubModel(x)->GetFullName();
                const std::string& name = model->GetSubModel(x)->GetName();
                if (name != "") {
                    dlg.ChoiceSuperStarImportModel->Append(fname);
                }
            }
        }
    }

    if (dlg.ShowModal() == wxID_CANCEL) {
        return;
    }

    wxString model_name = dlg.ChoiceSuperStarImportModel->GetStringSelection();
    if (model_name == "") {
        DisplayError("Please select the target model!");
        return;
    }

    wxStopWatch sw; // start a stopwatch timer
    bool model_found = false;

    // read v3 xml file into temporary document
    wxFileName xml_file(filename);
    wxXmlDocument input_xml;
    wxString xml_doc = xml_file.GetFullPath();
    wxFileInputStream fin(xml_doc);
    FixXMLInputStream bufIn(fin);

    if (!input_xml.Load(bufIn)) {
        DisplayError("Problem loading superstar file.");
        return;
    }

    if (dlg.TimeAdjSpinCtrl->GetValue() != 0) {
        int offset = dlg.TimeAdjSpinCtrl->GetValue();
        AdjustAllTimings(input_xml.GetRoot(), offset / 10);
    }

    Element* model = nullptr;

    for (size_t i = 0; i < _sequenceElements.GetElementCount(); i++) {
        if (_sequenceElements.GetElement(i)->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            model = _sequenceElements.GetElement(i);
            if (model->GetName() == model_name) {
                model_found = true;
                break;
            } else {
                ModelElement* modelEl = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(i));
                for (int x = 0; x < modelEl->GetSubModelAndStrandCount(); x++) {
                    std::string name = modelEl->GetSubModel(x)->GetFullName();
                    if (name == model_name) {
                        model = modelEl->GetSubModel(x);
                        model_found = true;
                    }
                }
                if (model_found) {
                    break;
                }
            }
        }
    }
    if (model != nullptr && model_found) {
        int x_size = wxAtoi(dlg.TextCtrl_SS_X_Size->GetValue());
        int y_size = wxAtoi(dlg.TextCtrl_SS_Y_Size->GetValue());
        int x_offset = wxAtoi(dlg.TextCtrl_SS_X_Offset->GetValue());
        int y_offset = wxAtoi(dlg.TextCtrl_SS_Y_Offset->GetValue());
        wxString blend = dlg.Choice_LayerBlend->GetStringSelection();
        Model* cls = GetModel(model->GetFullName());
        int bw, bh;
        cls->GetBufferSize("Default", "2D", "None", bw, bh, 0);
        wxSize modelSize(bw, bh);
        ImportSuperStar(model, input_xml, x_size, y_size, x_offset, y_offset, dlg.ImageResizeChoice->GetSelection(), modelSize, blend);
    }
    float elapsedTime = sw.Time() / 1000.0; // msec => sec
    SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}

bool findRGB(wxXmlNode* e, wxXmlNode* chan, wxXmlNode*& rchannel, wxXmlNode*& gchannel, wxXmlNode*& bchannel)
{
    wxString idxs[3];
    int cnt = 0;
    for (wxXmlNode* n = chan->GetChildren(); n != nullptr; n = n->GetNext()) {
        if (n->GetName() == "channels") {
            for (wxXmlNode* n2 = n->GetChildren(); n2 != nullptr; n2 = n2->GetNext()) {
                if (n2->GetName() == "channel" && cnt < 3) {
                    idxs[cnt] = n2->GetAttribute("savedIndex");
                    cnt++;
                }
            }
        }
    }
    for (wxXmlNode* ch = e->GetChildren(); ch != nullptr; ch = ch->GetNext()) {
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

void GetRGBTimes(wxXmlNode* re, int& startms, int& endms)
{
    if (re != nullptr) {
        startms = wxAtoi(re->GetAttribute("startCentisecond")) * 10;
        endms = wxAtoi(re->GetAttribute("endCentisecond")) * 10;
    } else {
        startms = 9999999;
        endms = 9999999;
    }
}
void GetIntensities(wxXmlNode* re, int& starti, int& endi)
{
    wxString intensity = re->GetAttribute("intensity", "-1");
    if (intensity == "-1") {
        starti = wxAtoi(re->GetAttribute("startIntensity"));
        endi = wxAtoi(re->GetAttribute("endIntensity"));
    } else {
        starti = endi = wxAtoi(intensity);
    }
}

class RGBData
{
public:
    int startms, endms;
    int starti, endi;
    bool shimmer;
};

void FillData(wxXmlNode* nd, RGBData& data)
{
    GetIntensities(nd, data.starti, data.endi);
    GetRGBTimes(nd, data.startms, data.endms);
    data.shimmer = nd->GetAttribute("type") == "shimmer";
}
void Insert(int x, std::vector<RGBData>& v, int startms)
{
    v.insert(v.begin() + x, 1, RGBData());
    v[x].startms = startms;
    v[x].endms = v[x + 1].startms;
    v[x].endi = v[x].starti = 0;
    v[x].shimmer = false;
}
void Split(int x, std::vector<RGBData>& v, int endms)
{
    v.insert(v.begin() + x, 1, RGBData());
    v[x].startms = v[x + 1].startms;
    v[x].endms = endms;
    v[x + 1].startms = endms;
    v[x].shimmer = v[x + 1].shimmer;
    v[x].starti = v[x + 1].starti;
    double d = endms - v[x].startms;
    d = d / double(v[x + 1].endms - v[x].startms);
    double newi = (v[x + 1].endi - v[x].starti) * d + v[x].starti;
    v[x].endi = v[x + 1].starti = newi;
}
#define MAXMS 99999999
int GetStartMS(int x, std::vector<RGBData>& v)
{
    if (x < v.size()) {
        return v[x].startms;
    }
    return MAXMS;
}
int GetEndMS(int x, std::vector<RGBData>& v)
{
    if (x < v.size()) {
        return v[x].endms;
    }
    return MAXMS;
}
void Resize(int x,
            std::vector<RGBData>& v, int startms)
{
    while (x >= v.size()) {
        int i = v.size();
        v.push_back(RGBData());
        v[i].endms = MAXMS;
        v[i].startms = startms;
        v[i].starti = v[i].endi = 0;
        v[i].shimmer = false;
    }
}

void UnifyData(int x,
               std::vector<RGBData>& red,
               std::vector<RGBData>& green,
               std::vector<RGBData>& blue)
{
    int min = std::min(GetStartMS(x, red), std::min(GetStartMS(x, green), GetStartMS(x, blue)));
    Resize(x, red, min);
    Resize(x, green, min);
    Resize(x, blue, min);
    if (red[x].startms != min) {
        Insert(x, red, min);
    }
    if (green[x].startms != min) {
        Insert(x, green, min);
    }
    if (blue[x].startms != min) {
        Insert(x, blue, min);
    }
    min = std::min(GetEndMS(x, red), std::min(GetEndMS(x, green), GetEndMS(x, blue)));
    if (min == MAXMS) {
        return;
    }
    if (red[x].endms != min) {
        Split(x, red, min);
    }
    if (green[x].endms != min) {
        Split(x, green, min);
    }
    if (blue[x].endms != min) {
        Split(x, blue, min);
    }
}
bool GetRGBEffectData(RGBData& red, RGBData& green, RGBData& blue, xlColor& sc, xlColor& ec)
{
    sc.red = red.starti * 255 / 100;
    sc.green = green.starti * 255 / 100;
    sc.blue = blue.starti * 255 / 100;

    ec.red = red.endi * 255 / 100;
    ec.green = green.endi * 255 / 100;
    ec.blue = blue.endi * 255 / 100;

    return red.shimmer | blue.shimmer | green.shimmer;
}

void LoadRGBData(EffectManager& effectManager, EffectLayer* layer, wxXmlNode* rchannel, wxXmlNode* gchannel, wxXmlNode* bchannel)
{
    std::vector<RGBData> red, green, blue;
    while (rchannel != nullptr) {
        int startms, endms;
        GetRGBTimes(rchannel, startms, endms);
        if (startms < endms) {
            red.resize(red.size() + 1);
            FillData(rchannel, red[red.size() - 1]);
        }
        rchannel = rchannel->GetNext();
    }
    while (gchannel != nullptr) {
        int startms, endms;
        GetRGBTimes(gchannel, startms, endms);
        if (startms < endms) {
            green.resize(green.size() + 1);
            FillData(gchannel, green[green.size() - 1]);
        }
        gchannel = gchannel->GetNext();
    }
    while (bchannel != nullptr) {
        int startms, endms;
        GetRGBTimes(bchannel, startms, endms);
        if (startms < endms) {
            blue.resize(blue.size() + 1);
            FillData(bchannel, blue[blue.size() - 1]);
        }
        bchannel = bchannel->GetNext();
    }
    // have the data, now need to split it so common start/end times
    for (size_t x = 0; x < red.size() || x < green.size() || x < blue.size(); x++) {
        UnifyData(x, red, green, blue);
    }

    for (size_t x = 0; x < red.size() || x < green.size() || x < blue.size(); x++) {
        xlColor sc, ec;
        bool isShimmer = GetRGBEffectData(red[x], green[x], blue[x], sc, ec);

        int starttime = red[x].startms;
        int endtime = red[x].endms;

        if (ec == sc) {
            if (ec != xlBLACK) {
                std::string palette = "C_BUTTON_Palette1=" + (std::string)sc + ",C_CHECKBOX_Palette1=1," + "C_BUTTON_Palette2=#000000,C_CHECKBOX_Palette2=0";
                std::string settings = (isShimmer ? "E_CHECKBOX_On_Shimmer=1" : "");
                layer->AddEffect(0, "On", settings, palette, starttime, endtime, false, false);
            }
        } else if (sc == xlBLACK) {
            std::string palette = "C_BUTTON_Palette1=" + (std::string)ec + ",C_CHECKBOX_Palette1=1,"
                                                                           "C_BUTTON_Palette2=#000000,C_CHECKBOX_Palette2=0";
            std::string settings = "E_TEXTCTRL_Eff_On_Start=0";
            if (isShimmer) {
                settings += ",E_CHECKBOX_On_Shimmer=1";
            }
            layer->AddEffect(0, "On", settings, palette, starttime, endtime, false, false);
        } else if (ec == xlBLACK) {
            std::string palette = "C_BUTTON_Palette1=" + (std::string)sc + ",C_CHECKBOX_Palette1=1,"
                                                                           "C_BUTTON_Palette2=#000000,C_CHECKBOX_Palette2=0";
            std::string settings = "E_TEXTCTRL_Eff_On_End=0";
            if (isShimmer) {
                settings += ",E_CHECKBOX_On_Shimmer=1";
            }
            layer->AddEffect(0, "On", settings, palette, starttime, endtime, false, false);
        } else {
            std::string palette = "C_BUTTON_Palette1=" + (std::string)sc + ",C_CHECKBOX_Palette1=1,"
                                                                           "C_BUTTON_Palette2=" +
                                  (std::string)ec + ",C_CHECKBOX_Palette2=1";
            std::string settings = (isShimmer ? "E_CHECKBOX_ColorWash_Shimmer=1," : "");
            layer->AddEffect(0, "Color Wash", settings, palette, starttime, endtime, false, false);
        }
    }
}

void MapRGBEffects(EffectManager& effectManager, EffectLayer* layer, wxXmlNode* rchannel, wxXmlNode* gchannel, wxXmlNode* bchannel)
{
    wxXmlNode* re = rchannel->GetChildren();
    while (re != nullptr && "effect" != re->GetName())
        re = re->GetNext();
    wxXmlNode* ge = gchannel->GetChildren();
    while (ge != nullptr && "effect" != ge->GetName())
        ge = ge->GetNext();
    wxXmlNode* be = bchannel->GetChildren();
    while (be != nullptr && "effect" != be->GetName())
        be = be->GetNext();
    LoadRGBData(effectManager, layer, re, ge, be);
}

std::string Scale255To100(wxString s, bool doscale)
{
    if (doscale)
        return wxString::Format("%d", wxAtoi(s) * 100 / 255).ToStdString();

    return s.ToStdString();
}

void MapOnEffects(EffectManager& effectManager, EffectLayer* layer, wxXmlNode* channel, int chancountpernode, const wxColor& color)
{
    std::string palette = "C_BUTTON_Palette1=#FFFFFF,C_CHECKBOX_Palette1=1";
    if (chancountpernode > 1) {
        xlColor color1(color);
        palette = "C_BUTTON_Palette1=" + color1 + ",C_CHECKBOX_Palette1=1";
    }

    for (wxXmlNode* ch = channel->GetChildren(); ch != nullptr; ch = ch->GetNext()) {
        if (ch->GetName() == "effect") {
            std::string type = ch->GetAttribute("type", "");
            bool doscale = (type == "DMX intensity");
            int starttime = (wxAtoi(ch->GetAttribute("startCentisecond"))) * 10;
            int endtime = (wxAtoi(ch->GetAttribute("endCentisecond"))) * 10;
            std::string intensity = ch->GetAttribute("intensity", "-1").ToStdString();
            std::string starti, endi;
            if (intensity == "-1") {
                starti = Scale255To100(ch->GetAttribute("startIntensity"), doscale);
                endi = Scale255To100(ch->GetAttribute("endIntensity"), doscale);
            } else {
                starti = endi = Scale255To100(intensity, doscale);
            }

            if (type == "twinkle") {
                std::string efpalette = palette;
                int steps = std::max((float)(endtime - starttime - 1000) / 100.0, 0.0) + (rand01() * 10.0 - 5.0);
                steps = std::max(steps, 2);
                steps = std::min(steps, 200);
                std::string settings = "E_CHECKBOX_Twinkle_Strobe=1,E_SLIDER_Twinkle_Count=2,E_SLIDER_Twinkle_Steps=" + wxString::Format("%d", steps);

                if (starti == endi) {
                    if (starti != "100") {
                        efpalette += ",C_SLIDER_Brightness=" + starti;
                    }
                } else {
                    efpalette += wxString::Format(",C_VALUECURVE_Brightness=Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Ramp|Min=0.00|Max=400.00|P1=%s.00|P2=%s.00|RV=TRUE|", starti, endi);
                }

                layer->AddEffect(0, "Twinkle", settings, efpalette, starttime, endtime, false, false);
            } else {
                std::string settings;
                if ("100" != starti) {
                    settings += "E_TEXTCTRL_Eff_On_Start=" + starti;
                }
                if ("100" != endi) {
                    if (!settings.empty()) {
                        settings += ",";
                    }
                    settings += "E_TEXTCTRL_Eff_On_End=" + endi;
                }
                if (("intensity" != ch->GetAttribute("type")) && ("DMX intensity" != ch->GetAttribute("type"))) {
                    if (!settings.empty()) {
                        settings += ",";
                    }
                    settings += "E_CHECKBOX_On_Shimmer=1";
                }
                layer->AddEffect(0, "On", settings, palette, starttime, endtime, false, false);
            }
        }
    }
}

bool MapChannelInformation(EffectManager& effectManager, EffectLayer* layer, wxXmlDocument& input_xml, const wxString& nm, const wxColor& color, const Model& mc, bool eraseExisting)
{
    if ("" == nm) {
        return false;
    }

    if (eraseExisting)
        layer->DeleteAllEffects();

    wxXmlNode* channel = nullptr;
    wxXmlNode* rchannel = nullptr;
    wxXmlNode* gchannel = nullptr;
    wxXmlNode* bchannel = nullptr;
    for (wxXmlNode* e = input_xml.GetRoot()->GetChildren(); channel == nullptr && e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "channels") {
            for (wxXmlNode* chan = e->GetChildren(); channel == nullptr && chan != nullptr; chan = chan->GetNext()) {
                std::string unit = chan->GetAttribute("unit");
                std::string circuit = chan->GetAttribute("circuit");
                std::string dedupname = chan->GetAttribute("name") + "_Unit_" + unit + "_Circuit_" + circuit;
                if ((chan->GetName() == "channel" || chan->GetName() == "rgbChannel") && (nm == chan->GetAttribute("name") || nm == dedupname)) {
                    channel = chan;
                    if (chan->GetName() == "rgbChannel" && !findRGB(e, chan, rchannel, gchannel, bchannel)) {
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
        MapRGBEffects(effectManager, layer, rchannel, gchannel, bchannel);
    } else {
        MapOnEffects(effectManager, layer, channel, mc.GetChanCountPerNode(), color);
    }
    return true;
}

void MapCCRModel(int& node, const std::vector<std::string>& channelNames, ModelElement* model, xLightsImportModelNode* m, Model* mc, wxXmlDocument& input_xml, EffectManager& effectManager, bool eraseExisting)
{
    wxString ccrName = m->_mapping;

    for (int str = 0; str < mc->GetNumStrands(); ++str) {
        StrandElement* se = model->GetStrand(str, true);

        for (int n = 0; n < se->GetNodeLayerCount(); n++) {
            EffectLayer* layer = se->GetNodeLayer(n, true);
            wxString nm = ccrName + wxString::Format("-P%02d", (node + 1));
            if (std::find(channelNames.begin(), channelNames.end(), nm) == channelNames.end()) {
                nm = ccrName + wxString::Format(" p%02d", (node + 1));
            }
            if (std::find(channelNames.begin(), channelNames.end(), nm) == channelNames.end()) {
                nm = ccrName + wxString::Format("-P%d", (node + 1));
            }
            if (std::find(channelNames.begin(), channelNames.end(), nm) == channelNames.end()) {
                nm = ccrName + wxString::Format(" p%d", (node + 1));
            }
            if (std::find(channelNames.begin(), channelNames.end(), nm) == channelNames.end()) {
                nm = ccrName + wxString::Format(" P %02d", (node + 1));
            }
            MapChannelInformation(effectManager,
                                  layer, input_xml,
                                  nm, m->_color,
                                  *mc, eraseExisting);
            node++;
        }
    }
}

void MapCCRStrand(const std::vector<std::string>& channelNames, StrandElement* se, xLightsImportModelNode* s, Model* mc, wxXmlDocument& input_xml, EffectManager& effectManager, bool eraseExisting)
{
    int node = 0;
    wxString ccrName = s->_mapping;

    for (int n = 0; n < se->GetNodeLayerCount(); n++) {
        EffectLayer* layer = se->GetNodeLayer(n, true);
        wxString nm = ccrName + wxString::Format("-P%02d", (node + 1));
        if (std::find(channelNames.begin(), channelNames.end(), nm) == channelNames.end()) {
            nm = ccrName + wxString::Format(" p%02d", (node + 1));
        }
        if (std::find(channelNames.begin(), channelNames.end(), nm) == channelNames.end()) {
            nm = ccrName + wxString::Format("-P%d", (node + 1));
        }
        if (std::find(channelNames.begin(), channelNames.end(), nm) == channelNames.end()) {
            nm = ccrName + wxString::Format(" p%d", (node + 1));
        }
        if (std::find(channelNames.begin(), channelNames.end(), nm) == channelNames.end()) {
            nm = ccrName + wxString::Format(" P %02d", (node + 1));
        }
        MapChannelInformation(effectManager,
                              layer, input_xml,
                              nm, s->_color,
                              *mc, eraseExisting);
        node++;
    }
}

void MapCCR(const std::vector<std::string>& channelNames, ModelElement* model, xLightsImportModelNode* m, Model* mc, wxXmlDocument& input_xml, EffectManager& effectManager, bool eraseExisting)
{
    if (mc->GetDisplayAs() == "ModelGroup") {
        ModelGroup* mg = (ModelGroup*)mc;
        int node = 0;
        for (auto it = mg->Models().begin(); it != mg->Models().end(); ++it) {
            MapCCRModel(node, channelNames, model, m, *it, input_xml, effectManager, eraseExisting);
        }
    } else {
        int node = 0;
        MapCCRModel(node, channelNames, model, m, mc, input_xml, effectManager, eraseExisting);
    }
}

bool xLightsFrame::ImportLMS(wxXmlDocument& input_xml, const wxFileName& filename)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    xLightsImportChannelMapDialog dlg(this, filename, true, true, true, true, false);
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;
    std::vector<std::string> timingTrackNames;
    std::map<std::string, wxXmlNode*> timingTracks;

    for (wxXmlNode* e = input_xml.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "channels") {
            for (wxXmlNode* chan = e->GetChildren(); chan != nullptr; chan = chan->GetNext()) {
                if (chan->GetName() == "channel" || chan->GetName() == "rgbChannel") {
                    std::string name = chan->GetAttribute("name").ToStdString();
                    if (chan->GetName() == "rgbChannel") {
                        dlg.channelColors[name] = xlBLACK;
                    } else {
                        std::string color = chan->GetAttribute("color").ToStdString();
                        std::string unit = chan->GetAttribute("unit").ToStdString();
                        std::string circuit = chan->GetAttribute("circuit").ToStdString();
                        if (dlg.GetImportChannel(name)) {
                            name += "_Unit_" + unit + "_Circuit_" + circuit;
                        }
                        dlg.channelColors[name] = GetColor(color);
                    }

                    bool ccr = false;
                    if (chan->GetName() == "rgbChannel") {
                        int idxDP = name.find("-P");
                        int idxUP = name.find(" P");
                        int idxSP = name.find(" p");
                        if (idxUP > idxSP) {
                            idxSP = idxUP;
                        }
                        if (idxDP > idxSP) {
                            idxSP = idxDP;
                        }
                        if (idxSP != wxNOT_FOUND) {
                            int i = wxAtoi(name.substr(idxSP + 2, name.size()));
                            if (i > 0 && name != "") {
                                ccr = true;
                                dlg.AddChannel(name);
                                // if (std::find(dlg.ccrNames.begin(), dlg.ccrNames.end(), name.substr(0, idxSP - 1)) == dlg.ccrNames.end())
                                if (name.substr(0, idxSP) != "" && std::find(dlg.ccrNames.begin(), dlg.ccrNames.end(), name.substr(0, idxSP)) == dlg.ccrNames.end()) {
                                    // dlg.ccrNames.push_back(name.substr(0, idxSP - 1));
                                    dlg.ccrNames.push_back(name.substr(0, idxSP));
                                }
                            }
                        }
                    }

                    if (!ccr && name != "") {
                        dlg.AddChannel(name);
                    }
                }
            }
        } else if (e->GetName() == "timingGrids") {
            for (wxXmlNode* timing = e->GetChildren(); timing != nullptr; timing = timing->GetNext()) {
                if (timing->GetName() == "timingGrid") {
                    wxString type = timing->GetAttribute("type", "");
                    if (type != "fixed") {
                        std::string name = timing->GetAttribute("name", "").ToStdString();
                        if (name != "") {
                            timingTrackNames.push_back(name);
                            timingTracks[name] = timing;
                        }
                    }
                }
            }
        }
    }

    dlg.SortChannels();
    std::sort(dlg.ccrNames.begin(), dlg.ccrNames.end(), stdlistNumberAwareStringCompare);
    dlg.timingTracks = timingTrackNames;

    dlg.InitImport();

    if (dlg.ShowModal() != wxID_OK || dlg._dataModel == nullptr) {
        return false;
    }

    if (dlg.TimeAdjustSpinCtrl->GetValue() != 0) {
        int offset = dlg.TimeAdjustSpinCtrl->GetValue();
        AdjustAllTimings(input_xml.GetRoot(), offset / 10);
    }

    for (size_t tt = 0; tt < dlg.TimingTrackListBox->GetCount(); ++tt) {
        if (dlg.TimingTrackListBox->IsChecked(tt)) {
            std::string name = dlg.TimingTrackListBox->GetString(tt).ToStdString();
            TimingElement* target = (TimingElement*)_sequenceElements.AddElement(name, "timing", true, true, false, false, false);
            char cnt = '1';
            while (target == nullptr) {
                target = (TimingElement*)_sequenceElements.AddElement(name + "-" + cnt++, "timing", true, true, false, false, false);
            }
            if (target->GetEffectLayerCount() == 0) {
                target->AddEffectLayer();
            }

            int offset = dlg.TimeAdjustSpinCtrl->GetValue();
            EffectLayer* targetLayer = target->GetEffectLayer(0);
            long last = offset;
            for (wxXmlNode* t = timingTracks[name]->GetChildren(); t != nullptr; t = t->GetNext()) {
                if (t->GetName() == "timing") {
                    int time = wxAtoi(t->GetAttribute("centisecond")) * 10 + offset;
                    int adjTime = TimeLine::RoundToMultipleOfPeriod(time, CurrentSeqXmlFile->GetFrequency());
                    if (adjTime > last) {
                        targetLayer->AddEffect(0, "", "", "", last, adjTime, false, false);
                        last = adjTime;
                    }
                }
            }
        }
    }

    for (size_t i = 0; i < dlg._dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = dlg._dataModel->GetNthChild(i);
        std::string modelName = m->_model.ToStdString();
        Model* mc = GetModel(modelName);
        ModelElement* model = nullptr;
        for (size_t x = 0; x < _sequenceElements.GetElementCount(); x++) {
            if (_sequenceElements.GetElement(x)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(x)->GetName()) {
                model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(x));
                break;
            }
        }

        if (m->_mapping != "") {
            if (model == nullptr) {
                model = AddModel(GetModel(modelName), _sequenceElements);
            }
            if (model == nullptr) {
                logger_base.error("Attempt to add model %s during LMS import failed.", (const char*)modelName.c_str());
            } else {
                if (std::find(dlg.ccrNames.begin(), dlg.ccrNames.end(), m->_mapping) != dlg.ccrNames.end()) {
                    MapCCR(dlg.GetChannelNames(), model, m, mc, input_xml, effectManager, dlg.CheckBox_EraseExistingEffects->GetValue());
                } else {
                    MapChannelInformation(effectManager,
                                          model->GetEffectLayer(0), input_xml,
                                          m->_mapping,
                                          m->_color, *mc, dlg.CheckBox_EraseExistingEffects->GetValue());
                }
            }
        }

        int str = 0;
        for (size_t j = 0; j < m->GetChildCount(); j++) {
            xLightsImportModelNode* s = m->GetNthChild(j);

            if ("" != s->_mapping) {
                if (model == nullptr) {
                    model = AddModel(GetModel(modelName), _sequenceElements);
                }
                if (model == nullptr) {
                    logger_base.error("Attempt to add model %s during LMS import failed.", (const char*)modelName.c_str());
                } else {
                    if (std::find(dlg.ccrNames.begin(), dlg.ccrNames.end(), s->_mapping) != dlg.ccrNames.end()) {
                        StrandElement* se = model->GetStrand(str);
                        if (se != nullptr) {
                            MapCCRStrand(dlg.GetChannelNames(), se, s, mc, input_xml, effectManager, dlg.CheckBox_EraseExistingEffects->GetValue());
                        } else {
                            logger_base.debug("LMS Import: Strand %d not found.", str);
                        }
                    } else {
                        SubModelElement* ste = model->GetSubModel(str);
                        if (ste != nullptr) {
                            MapChannelInformation(effectManager,
                                                  ste->GetEffectLayer(0), input_xml,
                                                  s->_mapping,
                                                  s->_color, *mc, dlg.CheckBox_EraseExistingEffects->GetValue());
                        } else {
                            logger_base.debug("LMS Import: SubModel %d not found.", str);
                        }
                    }
                }
            }
            for (size_t n = 0; n < s->GetChildCount(); n++) {
                xLightsImportModelNode* ns = s->GetNthChild(n);
                if ("" != ns->_mapping) {
                    if (model == nullptr) {
                        model = AddModel(GetModel(modelName), _sequenceElements);
                    }
                    if (model == nullptr) {
                        logger_base.error("Attempt to add model %s during LMS import failed.", (const char*)modelName.c_str());
                    } else {
                        SubModelElement* ste = model->GetSubModel(str);
                        StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                        if (stre != nullptr) {
                            NodeLayer* nl = stre->GetNodeLayer(n, true);
                            if (nl != nullptr) {
                                MapChannelInformation(effectManager,
                                                      nl, input_xml,
                                                      ns->_mapping,
                                                      ns->_color, *mc, dlg.CheckBox_EraseExistingEffects->GetValue());
                            }
                        }
                    }
                }
            }
            str++;
        }
    }

    return true;
}

bool LPEHasEffects(const wxXmlDocument& input_xml, const wxString& model, int layer, bool left)
{
    for (wxXmlNode* e = input_xml.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "SequenceProps" || e->GetName() == "ArchivedProps") {
            for (wxXmlNode* prop = e->GetChildren(); prop != nullptr; prop = prop->GetNext()) {
                if (prop->GetName() == "SeqProp" || prop->GetName() == "ArchiveProp") {
                    std::string name = prop->GetAttribute("name").ToStdString();
                    if (name == "") {
                        for (wxXmlNode* ap = prop->GetChildren(); ap != nullptr; ap = ap->GetNext()) {
                            if (ap->GetName() == "PropClass") {
                                name = ap->GetAttribute("Name");
                            }
                        }
                    }
                    if (name == model) {
                        for (wxXmlNode* track = prop->GetChildren(); track != nullptr; track = track->GetNext()) {
                            int id = wxAtoi(track->GetAttribute("id"));
                            if (id == layer) {
                                for (wxXmlNode* eff = track->GetChildren(); eff != nullptr; eff = eff->GetNext()) {
                                    if (eff->GetName() == "effect" && eff->GetAttribute("type") == "pixelEffect") {
                                        wxString settings = eff->GetAttribute("pixelEffect");
                                        wxArrayString as = wxSplit(settings, '|');
                                        if (as.size() == 7) {
                                            wxString s;
                                            if (left) {
                                                s = as[5];
                                            } else {
                                                s = as[6];
                                            }
                                            wxArrayString ss = wxSplit(s, ':');
                                            if (ss[0] != "none")
                                                return true;
                                        } else if (as.size() == 5) {
                                            wxString s;
                                            if (left) {
                                                s = as[3];
                                            } else {
                                                s = as[4];
                                            }
                                            wxArrayString ss = wxSplit(s, ':');
                                            if (ss[0] != "none")
                                                return true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

wxString MapLPEEffectType(const wxString& effect)
{
    if (effect == "colorwash")
        return "Color Wash";
    if (effect == "picture")
        return "Pictures";
    if (effect == "lineshorizontal")
        return "";
    if (effect == "linesvertical")
        return "";
    if (effect == "countdown")
        return "Text"; // we dont support countdown

    return effect.Capitalize();
}

wxXmlNode* FindLastLPEEffectNode(wxXmlNode* effect, int start_centisecond, int end_centisecond, int end_intensity, int intensityChange, const wxString& settings, int& fadeInCS, int& fadeOutCS)
{
    int originalIntensity = end_intensity - intensityChange;

    fadeInCS = 0;
    fadeOutCS = 0;

    wxXmlNode* res = effect;
    wxXmlNode* n = effect->GetNext();
    while (n != nullptr) {
        int newstartCentisecond = wxAtoi(n->GetAttribute("startCentisecond"));
        // int newendCentisecond = wxAtoi(n->GetAttribute("endCentisecond"));
        int newstartIntensity = wxAtoi(n->GetAttribute("startIntensity", "100"));
        int newendIntensity = wxAtoi(n->GetAttribute("endIntensity", "100"));
        wxString newsettings = n->GetAttribute("pixelEffect", "");
        if (settings != newsettings) {
            break;
        }

        int newintensityChange = newendIntensity - newstartIntensity;
        if (abs(intensityChange - newintensityChange) > 1) {
            // significant change in the amount of intensity change

            if (intensityChange > 0 && newintensityChange == 0 && fadeInCS == 0) {
                fadeInCS = newstartCentisecond;
                intensityChange = newintensityChange;
            } else if (intensityChange == 0 && newintensityChange < 0 && fadeOutCS == 0) {
                fadeOutCS = newstartCentisecond;
                intensityChange = newintensityChange;
            } else {
                // weird intensity change ... lets give up
                break;
            }
        }

        res = n;
        n = n->GetNext();
    }

    // handle effects that are all fade in or all fade out
    // I use 5-95 as a proxy for 0-100 to maximise the use of fade in/out
    int newendCentisecond = wxAtoi(res->GetAttribute("endCentisecond"));
    int newendIntensity = wxAtoi(res->GetAttribute("endIntensity", "100"));
    if (fadeOutCS == 0 && fadeInCS == 0 && originalIntensity < newendIntensity && newendIntensity > 95 && originalIntensity < 5) {
        // all fade in
        fadeInCS = newendCentisecond;
    } else if (fadeOutCS == 0 && fadeInCS == 0 && originalIntensity > newendIntensity && newendIntensity < 5 && originalIntensity > 95) {
        // all fade out
        fadeOutCS = start_centisecond;
    }

    return res;
}

wxString MapLPEBlend(const wxString& blend, bool left)
{
    if (!left)
        return "Normal";

    if (blend == "Mix_Average")
        return "Average";
    if (blend == "Mix_Overlay")
        return "Normal";
    if (blend == "Mix_Maximum")
        return "Max";
    if (blend == "Mix_Bottom_Top")
        return "Bottom-Top";
    if (blend == "Mix_Left-Right")
        return "Left-Right";
    if (blend == "Mix_Rt_Hides_Lt")
        return "1 is Mask";
    if (blend == "Mix_Rt_Reveals_Lt")
        return "2 reveals 1";

    return "Normal";
}

std::string ExtractLPEPallette(const wxArrayString& ps)
{
    std::string palette;

    int cnum = 0;
    wxArrayString c = wxSplit(ps[1], ';');
    for (int i = 0; i < c.size(); i++) {
        wxString n = wxString::Format("%d", cnum + 1);

        wxArrayString cc = wxSplit(c[i], ',');
        if (cc.size() == 2) {
            wxString c1 = cc[0].substr(2); // drop transparency
            wxString active = cc[1];

            palette += ",C_BUTTON_Palette" + n + "=#" + c1;
            if (active == "1") {
                palette += ",C_CHECKBOX_Palette" + n + "=" + active;
            }
            cnum++;
        } else if (cc.size() == 3) {
            wxString c1 = cc[0].substr(2); // drop transparency
            wxString c2 = cc[1].substr(2); // drop transparency
            wxString active = cc[2];

            if (c1 == c2) {
                palette += ",C_BUTTON_Palette" + n + "=#" + c1;
                if (active == "1") {
                    palette += ",C_CHECKBOX_Palette" + n + "=" + active;
                }
            } else {
                palette += "C_BUTTON_Palette" + n + "=Active=TRUE|Id=ID_BUTTON_Palette" + n + "|Values=x=0.000^c=#" + c1 + ";x=1.000^c=#" + c2 + "|";
                if (active == "1") {
                    palette += ",C_CHECKBOX_Palette" + n + "=" + active;
                }
            }
            cnum++;
        } else {
            // Not sure what the last value is so ignoring it
            // int unknown1 = wxAtoi(c[i]);
            break;
        }
    }

    return palette;
}

// Used to rescale a parameter to a broader scale.
// Assumes the range is different but the translation is direct.
// If this is not the case then set the source Min/Max to the range that does map to the targetMin/Max and the conversion will
// clamp original values outside the supported range to the largest practical in the target
float Rescale(float original, float sourceMin, float sourceMax, float targetMin, float targetMax)

{
    if (original < sourceMin)
        original = sourceMin;
    if (original > sourceMax)
        original = sourceMax;

    return ((original - sourceMin) / (sourceMax - sourceMin)) * (targetMax - targetMin) + targetMin;
}

wxString RescaleWithRangeI(wxString r, wxString vcName, float sourceMin, float sourceMax, float targetMin, float targetMax, wxString& vc, float targetRealMin, float targetRealMax)
{
    if (r.Contains("R")) {
        // it is a range
        wxArrayString rr = wxSplit(r, 'R');
        vc = "," + vcName + "=Active=TRUE|Id=ID_" + vcName.substr(2) + "|Type=Ramp|Min=" + wxString::Format("%.2f", targetRealMin) +
             "|Max=" + wxString::Format("%.2f", targetRealMax) +
             "|P1=" + wxString::Format("%.2f", Rescale(wxAtof(rr[0]), sourceMin, sourceMax, targetMin, targetMax)) +
             "|P2=" + wxString::Format("%.2f", Rescale(wxAtof(rr[1]), sourceMin, sourceMax, targetMin, targetMax)) +
             "|RV=TRUE|";
        return wxString::Format("%d", (int)Rescale(wxAtof(rr[0]), sourceMin, sourceMax, targetMin, targetMax));
    } else {
        vc = "";
        return wxString::Format("%d", (int)Rescale(wxAtof(r), sourceMin, sourceMax, targetMin, targetMax));
    }
}

wxString RescaleWithRangeF(wxString r, wxString vcName, float sourceMin, float sourceMax, float targetMin, float targetMax, wxString& vc, float targetRealMin, float targetRealMax)
{
    if (r.Contains("R")) {
        // it is a range
        wxArrayString rr = wxSplit(r, 'R');
        vc = "," + vcName + "=Active=TRUE|Id=ID_" + vcName.substr(2) + "|Type=Ramp|Min=" + wxString::Format("%.2f", targetRealMin) +
             "|Max=" + wxString::Format("%.2f", targetRealMax) +
             "|P1=" + wxString::Format("%.2f", Rescale(wxAtof(rr[0]), sourceMin, sourceMax, targetMin, targetMax)) +
             "|P2=" + wxString::Format("%.2f", Rescale(wxAtof(rr[1]), sourceMin, sourceMax, targetMin, targetMax)) +
             "|RV=TRUE|";
    } else {
        vc = "";
    }
    return wxString::Format("%.1f", Rescale(wxAtof(r), sourceMin, sourceMax, targetMin, targetMax));
}

std::string LPEParseEffectSettings(const wxString& effectType, const wxArrayString& arrSettings, std::string& palette, int durationMS)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string settings;

    if (arrSettings.size() > 1) {
        wxArrayString parms = wxSplit(arrSettings[2], ',');
        if (effectType == "butterfly") {
            wxString style = parms[0];
            wxString chunks = parms[1];
            wxString vcChunks;
            chunks = RescaleWithRangeI(chunks, "E_VALUECURVE_Butterfly_Chunks", 1, 10, 1, 10, vcChunks, BUTTERFLY_CHUNKS_MIN, BUTTERFLY_CHUNKS_MAX);
            wxString skip = parms[2];
            wxString vcSkip;
            skip = RescaleWithRangeI(skip, "E_VALUECURVE_Butterfly_Skip", 2, 10, 2, 10, vcSkip, BUTTERFLY_SKIP_MIN, BUTTERFLY_SKIP_MAX);
            wxString direction = parms[3];
            wxString hue = parms[4];
            wxString vcHue;
            hue = RescaleWithRangeI(hue, "C_VALUECURVE_Color_HueAdjust", 0, 359, -100, 100, vcHue, -100, 100);
            wxString speed = parms[5];
            wxString vcSpeed;
            speed = RescaleWithRangeI(speed, "E_VALUECURVE_Butterfly_Speed", 0, 50, 0, 50, vcSpeed, BUTTERFLY_SPEED_MIN, BUTTERFLY_SPEED_MAX);
            wxString colours = parms[6];

            if (style == "linear") {
                settings += ",E_SLIDER_Butterfly_Style=1";
            } else if (style == "radial") {
                settings += ",E_SLIDER_Butterfly_Style=2";
            } else if (style == "blocks") {
                settings += ",E_SLIDER_Butterfly_Style=3";
            } else if (style == "corner") {
                settings += ",E_SLIDER_Butterfly_Style=5";
            }
            settings += ",E_CHOICE_Butterfly_Colors=" + colours.Capitalize();
            settings += ",E_CHOICE_Butterfly_Direction=" + direction.Capitalize();
            settings += ",E_SLIDER_Butterfly_Chunks=" + chunks;
            settings += vcChunks;
            settings += ",E_SLIDER_Butterfly_Skip=" + skip;
            settings += vcSkip;
            settings += ",E_SLIDER_Butterfly_Speed=" + speed;
            settings += vcSpeed;

            if (hue != "0") {
                palette += ",C_SLIDER_Color_HueAdjust=" + hue;
                palette += vcHue;
            }
        } else if (effectType == "colorwash") {
            // full, full, none, 12
            wxString horizontalFade = parms[0];
            wxString verticalFade = parms[1];

            if (horizontalFade == "full") {
            } else if (horizontalFade == "left_to_right") {
                settings += ",E_CHECKBOX_ColorWash_HFade=1";
            } else if (horizontalFade == "right_to_left") {
                settings += ",E_CHECKBOX_ColorWash_HFade=1";
            } else if (horizontalFade == "center_on") {
                settings += ",E_CHECKBOX_ColorWash_HFade=1";
            } else if (horizontalFade == "center_off") {
                settings += ",E_CHECKBOX_ColorWash_HFade=1";
            }

            if (verticalFade == "full") {
            } else if (verticalFade == "top_to_bottom") {
                settings += ",E_CHECKBOX_ColorWash_VFade=1";
            } else if (verticalFade == "bottom_to_top") {
                settings += ",E_CHECKBOX_ColorWash_VFade=1";
            } else if (verticalFade == "center_on") {
                settings += ",E_CHECKBOX_ColorWash_VFade=1";
            } else if (verticalFade == "center_off") {
                settings += ",E_CHECKBOX_ColorWash_VFade=1";
            }
        } else if (effectType == "spirals") {
            // 1, left_to_right, 20, 50, 0, False, none, 12
            wxString repeat = parms[0];
            wxString vcRepeat;
            repeat = RescaleWithRangeI(repeat, "E_VALUECURVE_Spirals_Count", 1, 5, 1, 5, vcRepeat, SPIRALS_COUNT_MIN, SPIRALS_COUNT_MAX);
            wxString direction = parms[1];
            wxString rotation = parms[2];
            rotation = wxString::Format("%.2f", wxAtof(rotation) / 60.0);
            wxString vcRotation;
            rotation = RescaleWithRangeF(rotation, "E_VALUECURVE_Spirals_Rotation", 0, 50, 0, 50, vcRotation, SPIRALS_ROTATION_MIN, SPIRALS_ROTATION_MAX);
            rotation = wxString::Format("%d", (int)(wxAtof(rotation) * 10.0));
            wxString thickness = parms[3];
            wxString vcThickness;
            thickness = RescaleWithRangeI(thickness, "E_VALUECURVE_Spirals_Thickness", 0, 100, 0, 100, vcThickness, SPIRALS_THICKNESS_MIN, SPIRALS_THICKNESS_MAX);
            wxString thicknessChange = parms[4];
            wxString blend = parms[5];
            wxString show3d = parms[6];
            wxString speed = parms[7];
            speed = wxString::Format("%d", (int)(wxAtof(speed) / (20.0 / ((float)durationMS / 1000.0))));
            wxString vcSpeed;
            if (direction == "right_to_left") {
                speed = RescaleWithRangeF(speed, "E_VALUECURVE_Spirals_Movement", 0, 50, 0, -50, vcSpeed, SPIRALS_MOVEMENT_MIN, SPIRALS_MOVEMENT_MAX);
            } else {
                speed = RescaleWithRangeF(speed, "E_VALUECURVE_Spirals_Movement", 0, 50, 0, 50, vcSpeed, SPIRALS_MOVEMENT_MIN, SPIRALS_MOVEMENT_MAX);
            }

            settings += ",E_SLIDER_Spirals_Count=" + repeat;
            settings += vcRepeat;

            settings += ",E_TEXTCTRL_Spirals_Movement=" + speed;
            settings += vcSpeed;

            settings += ",E_SLIDER_Spirals_Rotation=" + rotation;
            settings += vcRotation;

            settings += ",E_SLIDER_Spirals_Thickness=" + thickness;
            settings += vcThickness;

            // dont know what to do with thickness change

            if (blend == "True") {
                settings += ",E_CHECKBOX_Spirals_Blend=1,";
            }

            if (show3d == "none") {
            } else if (show3d == "trail_left") {
                settings += ",E_CHECKBOX_Spirals_3D=1";
            } else if (show3d == "trail_right") {
                settings += ",E_CHECKBOX_Spirals_3D=1";
            }
        } else if (effectType == "bars") {
            // down,2,False,False,8,0
            wxString direction = parms[0];
            wxString repeat = parms[1];
            wxString vcRepeat;
            repeat = RescaleWithRangeI(repeat, "E_VALUECURVE_Bars_BarCount", 1, 5, 1, 5, vcRepeat, BARCOUNT_MIN, BARCOUNT_MAX);
            wxString highlight = parms[2];
            wxString show3d = parms[3];
            wxString speed = parms[4];
            speed = wxString::Format("%d", (int)(wxAtof(speed) / (20.0 / ((float)durationMS / 1000.0))));
            wxString vcSpeed;
            speed = RescaleWithRangeF(speed, "E_VALUECURVE_Bars_Cycles", 0, 50, 0, 30, vcSpeed, BARCYCLES_MIN, BARCYCLES_MAX);
            wxString centre = parms[5];
            wxString vcCentre;
            centre = RescaleWithRangeI(centre, "E_VALUECURVE_Bars_Center", -50, 50, -100, 100, vcCentre, BARCENTER_MIN, BARCENTER_MAX);

            settings += ",E_SLIDER_Bars_BarCount=" + repeat;
            settings += vcRepeat;

            if (direction == "V_expand")
                direction = "expand";
            if (direction == "V_compress")
                direction = "compress";
            if (direction == "H_expand")
                direction = "H-expand";
            if (direction == "H_compress")
                direction = "H-compress";
            if (direction == "left")
                direction = "Left";
            if (direction == "right")
                direction = "Right";
            if (direction == "block_up")
                direction = "Alternate Up";
            if (direction == "block_down")
                direction = "Alternate Down";
            if (direction == "block_left")
                direction = "Alternate Right";
            if (direction == "block_right")
                direction = "Alternate Right";
            settings += ",E_CHOICE_Bars_Direction=" + direction;

            if (show3d == "True") {
                settings += ",E_CHECKBOX_Bars_3D=1";
            }

            if (highlight == "True") {
                settings += "E_CHECKBOX_Bars_Highlight=1";
            }

            settings += ",E_TEXTCTRL_Bars_Cycles=" + speed;
            settings += vcSpeed;

            settings += ",E_TEXTCTRL_Bars_Center=" + centre;
            settings += vcCentre;
        } else if (effectType == "countdown") {
            // 0,Arial,75,7
            wxString seconds = parms[0];
            wxString font = parms[1];
            wxString fontSize = parms[2];
            wxString vcCrap;
            fontSize = RescaleWithRangeI(fontSize, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
            wxString position = parms[3];
            position = RescaleWithRangeI(position, "IGNORE", -50, 50, -200, 200, vcCrap, -1, -1);

            settings += ",E_TEXTCTRL_Text=" + seconds;
            settings += ",E_CHOICE_Text_Count=seconds";
            settings += ",E_CHOICE_Text_Font=Use OS Fonts";
            settings += ",E_FONTPICKER_Text_Font='" + font + "' " + fontSize;
            settings += ",E_SLIDER_Text_XStart=" + position;
        } else if (effectType == "lineshorizontal") {
            // Bottom_to_Top,8,38

            // No xLights equivalent

            logger_base.warn("LPE conversion for Lines Horizontal does not exist.");
        } else if (effectType == "linesvertical") {
            // Left_to_Right,4,32

            // No xLights equivalent

            logger_base.warn("LPE conversion for Lines Vertical does not exist.");
        } else if (effectType == "curtain") {
            // center,open,0,once_at_speed,12
            wxString edge = parms[0];
            wxString movement = parms[1];
            wxString swag = parms[2];
            wxString vcSwag;
            swag = RescaleWithRangeF(swag, "E_VALUECURVE_Curtain_Swag", 0, 10, 0, 10, vcSwag, CURTAIN_SWAG_MIN, CURTAIN_SWAG_MAX);
            wxString repeat = parms[3];
            wxString speed = parms[4];
            wxString vcSpeed;
            speed = RescaleWithRangeF(speed, "E_VALUECURVE_Curtain_Speed", 0, 50, 0, 10, vcSpeed, CURTAIN_SPEED_MIN, CURTAIN_SPEED_MAX);

            settings += ",E_CHOICE_Curtain_Edge=" + edge;
            movement.Replace("_", " ");
            settings += ",E_CHOICE_Curtain_Effect=" + movement;
            settings += ",E_SLIDER_Curtain_Swag=" + swag;
            settings += vcSwag;

            if (repeat == "once_at_speed") {
                settings += ",E_CHECKBOX_Curtain_Repeat=0";
            } else if (repeat == "once_fit_to_duration") {
                settings += ",E_CHECKBOX_Curtain_Repeat=0";
            } else if (repeat == "repeat_at_speed_rotate_colors") {
                settings += ",E_CHECKBOX_Curtain_Repeat=1";
            } else if (repeat == "repeat_at_speed_blend_colors") {
                settings += ",E_CHECKBOX_Curtain_Repeat=1";
            }
            settings += ",E_TEXTCTRL_Curtain_Speed=" + speed;
            settings += vcSpeed;
        } else if (effectType == "fire") {
            // 50,0
            wxString height = parms[0];
            wxString vcHeight;
            height = RescaleWithRangeI(height, "E_VALUECURVE_Fire_Height", 10, 100, 0, 100, vcHeight, FIRE_HEIGHT_MIN, FIRE_HEIGHT_MAX);
            wxString hueShift = parms[1];
            wxString vcHueShift;
            hueShift = RescaleWithRangeI(hueShift, "E_VALUECURVE_Fire_HueShift", 0, 359, 0, 100, vcHueShift, FIRE_HUE_MIN, FIRE_HUE_MAX);

            settings += ",E_SLIDER_Fire_Height=" + height;
            settings += vcHeight;
            settings += ",E_SLIDER_Fire_HueShift=" + hueShift;
            settings += vcHueShift;
        } else if (effectType == "fireworks") {
            // 10,50,2,30,normal,continuous
            wxString explosionRate = parms[0];
            wxString vcCrap;
            explosionRate = RescaleWithRangeI(explosionRate, "IGNORE", 1, 95, 1, 50, vcCrap, -1, -1);
            wxString particles = parms[1];
            particles = RescaleWithRangeI(particles, "IGNORE", 1, 100, 1, 100, vcCrap, -1, -1);
            wxString velocity = parms[2];
            velocity = RescaleWithRangeI(velocity, "IGNORE", 1, 10, 1, 10, vcCrap, -1, -1);
            wxString fade = parms[3];
            fade = RescaleWithRangeI(fade, "IGNORE", 1, 100, 1, 100, vcCrap, -1, -1);
            wxString pattern = parms[4];    // not used
            wxString rateChange = parms[5]; // not used
            settings += ",E_SLIDER_Fireworks_Explosions=" + explosionRate;
            settings += ",E_SLIDER_Fireworks_Count=" + particles;
            settings += ",E_SLIDER_Fireworks_Fade=" + fade;
            settings += ",E_SLIDER_Fireworks_Velocity=" + velocity;
        } else if (effectType == "garland") {
            // 3,34,once_at_speed,12,bottom_to_top
            wxString type = parms[0];
            wxString vcCrap;
            type = RescaleWithRangeI(type, "IGNORE", 0, 4, 0, 4, vcCrap, -1, -1);
            wxString spacing = parms[1];
            wxString vcSpacing;
            spacing = RescaleWithRangeI(spacing, "E_VALUECURVE_Garlands_Spacing", 0, 100, 1, 100, vcSpacing, GARLANDS_SPACING_MIN, GARLANDS_SPACING_MAX);
            wxString repeat = parms[2];
            wxString speed = parms[3];
            wxString vcSpeed;
            speed = RescaleWithRangeF(speed, "E_VALUECURVE_Garlands_Cycles", 0, 50, 0, 20, vcSpeed, GARLANDS_CYCLES_MIN, GARLANDS_CYCLES_MAX);
            wxString fill = parms[4];

            settings += ",E_SLIDER_Garlands_Type=" + type;

            if (fill == "bottom_to_top") {
                settings += ",E_CHOICE_Garlands_Direction=Up";
            } else if (fill == "top_to_bottom") {
                settings += ",E_CHOICE_Garlands_Direction=Down";
            } else if (fill == "left_to_right") {
                settings += ",E_CHOICE_Garlands_Direction=Right";
            } else if (fill == "right_to_left") {
                settings += ",E_CHOICE_Garlands_Direction=Left";
            }

            settings += ",E_SLIDER_Garlands_Spacing=" + spacing;
            settings += vcSpacing;

            if (repeat == "repeat_at_speed") {
                settings += ",E_TEXTCTRL_Garlands_Cycles=" + speed;
                settings += vcSpeed;
            } else if (repeat == "once_at_speed") {
                settings += ",E_TEXTCTRL_Garlands_Cycles=1.0";
            } else if (repeat == "once_fit_to_duration") {
                settings += ",E_TEXTCTRL_Garlands_Cycles=1.0";
            }
        } else if (effectType == "meteors") {
            // rainbow,10,25,down,0,12
            wxString colourScheme = parms[0];
            wxString count = parms[1];
            wxString vcCount;
            count = RescaleWithRangeI(count, "E_VALUECURVE_Meteors_Count", 1, 100, 1, 100, vcCount, METEORS_COUNT_MIN, METEORS_COUNT_MAX);
            wxString length = parms[2];
            wxString vcLength;
            length = RescaleWithRangeI(length, "E_VALUECURVE_Meteors_Length", 1, 100, 1, 100, vcLength, METEORS_LENGTH_MIN, METEORS_LENGTH_MAX);
            wxString effect = parms[3];
            wxString swirl = parms[4];
            wxString vcSwirl;
            swirl = RescaleWithRangeI(swirl, "E_VALUECURVE_Meteors_Swirl_Intensity", 0, 20, 0, 20, vcSwirl, METEORS_SWIRL_MIN, METEORS_SWIRL_MAX);
            wxString speed = parms[5];
            wxString vcSpeed;
            speed = RescaleWithRangeI(speed, "E_VALUECURVE_Meteors_Speed", 1, 50, 1, 50, vcSpeed, METEORS_SPEED_MIN, METEORS_SPEED_MAX);

            settings += ",E_CHOICE_Meteors_Type=" + colourScheme.Lower();
            settings += ",E_SLIDER_Meteors_Count=" + count;
            settings += vcCount;
            settings += ",E_SLIDER_Meteors_Length=" + length;
            settings += vcLength;
            settings += ",E_CHOICE_Meteors_Effect=" + effect.Capitalize();
            settings += ",E_SLIDER_Meteors_Swirl_Intensity=" + swirl;
            settings += vcSwirl;
            settings += ",E_SLIDER_Meteors_Speed=" + speed;
            settings += vcSpeed;
        } else if (effectType == "movie") {
            // xxx.avi,True,False
            wxString file = parms[0];
            wxString scale = parms[1];
            wxString fullLength = parms[2];

            settings += ",E_FILEPICKERCTRL_Video_Filename=" + file;

            if (scale == "True") {
                settings += ",E_CHECKBOX_Video_AspectRatio=0";
            } else {
                settings += ",E_CHECKBOX_Video_AspectRatio=1";
            }
            if (fullLength == "True") {
                settings += ",E_CHOICE_Video_DurationTreatment=Slow/Accelerate";
            } else {
                settings += ",E_CHOICE_Video_DurationTreatment=Normal";
            }
        } else if (effectType == "picture") {
            // file.jpg,True,none,0,10,19,12
            wxString file = parms[0];
            wxString scale = parms[1];
            wxString movement = parms[2];
            wxString x = parms[3];
            wxString vcCrap;
            x = RescaleWithRangeI(x, "IGNORE", -50, 50, -100, 100, vcCrap, -1, -1);
            wxString peekabooHoldTime = parms[4]; // not used
            peekabooHoldTime = RescaleWithRangeI(peekabooHoldTime, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
            wxString wiggle = parms[5]; // not used
            wiggle = RescaleWithRangeI(wiggle, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
            wxString speed = parms[6];
            speed = RescaleWithRangeF(speed, "IGNORE", 0, 50, 0, 20, vcCrap, -1, -1);

            settings += ",E_FILEPICKER_Pictures_Filename=" + file;
            if (scale == "True") {
                settings += ",E_CHOICE_Scaling=Scale To Fit";
            } else {
                settings += ",E_CHOICE_Scaling=No Scaling";
            }

            movement.Replace("_", "-");
            if (movement == "peekaboo-bottom") {
                movement = "peekaboo";
            } else if (movement == "peekaboo-top") {
                movement = "peekaboo 180";
            } else if (movement == "peekaboo-left") {
                movement = "peekaboo 90";
            } else if (movement == "peekaboo-right") {
                movement = "peekaboo 270";
            }
            settings += ",E_CHOICE_Pictures_Direction=" + movement;
            settings += ",E_SLIDER_PicturesXC=" + x;
            settings += ",E_TEXTCTRL_Pictures_Speed=" + speed;
        } else if (effectType == "pinwheel") {
            // 3,1,6,color_per_arm,True,12,100,10,-23

            wxString arms = parms[0];
            wxString vcCrap;
            arms = RescaleWithRangeI(arms, "IGNORE", 1, 10, 1, 10, vcCrap, -1, -1);
            wxString width = parms[1];
            wxString vcWidth;
            width = RescaleWithRangeI(width, "E_VALUECURVE_Pinwheel_Thickness", 1, 10, 0, 100, vcWidth, PINWHEEL_THICKNESS_MIN, PINWHEEL_THICKNESS_MAX);
            wxString bend = parms[2];
            wxString vcBend;
            bend = RescaleWithRangeI(bend, "E_VALUECURVE_Pinwheel_Twist", -10, 10, -360, 360, vcBend, PINWHEEL_TWIST_MIN, PINWHEEL_TWIST_MAX);
            wxString colour = parms[3]; // not used
            wxString CCW = parms[4];
            wxString speed = parms[5];
            wxString vcSpeed;
            speed = RescaleWithRangeI(speed, "E_VALUECURVE_Pinwheel_Speed", 0, 50, 0, 50, vcSpeed, PINWHEEL_SPEED_MIN, PINWHEEL_SPEED_MAX);
            wxString length = parms[6];
            wxString vcLength;
            length = RescaleWithRangeI(length, "E_VALUECURVE_Pinwheel_ArmSize", 1, 100, 0, 400, vcLength, PINWHEEL_ARMSIZE_MIN, PINWHEEL_ARMSIZE_MAX);
            wxString x = parms[7];
            wxString vcX;
            x = RescaleWithRangeI(x, "E_VALUECURVE_PinwheelXC", -50, 50, -100, 100, vcX, PINWHEEL_X_MIN, PINWHEEL_X_MAX);
            wxString y = parms[8];
            wxString vcY;
            y = RescaleWithRangeI(y, "E_VALUECURVE_PinwheelYC", -50, 50, -100, 100, vcY, PINWHEEL_Y_MIN, PINWHEEL_Y_MAX);

            settings += ",E_SLIDER_Pinwheel_Arms=" + arms;
            settings += ",E_SLIDER_Pinwheel_Thickness=" + width;
            settings += vcWidth;
            settings += ",E_SLIDER_Pinwheel_Twist=" + bend;
            settings += vcBend;
            if (CCW == "True") {
                settings += ",E_CHECKBOX_Pinwheel_Rotation=1";
            } else {
                settings += ",E_CHECKBOX_Pinwheel_Rotation=0";
            }
            settings += ",E_SLIDER_Pinwheel_Speed=" + speed;
            settings += vcSpeed;
            settings += ",E_SLIDER_Pinwheel_ArmSize=" + length;
            settings += vcLength;
            settings += ",E_CHOICE_Pinwheel_Style=New Render Method";
            settings += ",E_SLIDER_PinwheelXC=" + x;
            settings += vcX;
            settings += ",E_SLIDER_PinwheelYC=" + y;
            settings += vcY;
        } else if (effectType == "snowflakes") {
            // 5,1,0,12,60
            wxString count = parms[0];
            wxString vcCount;
            count = RescaleWithRangeI(count, "E_VALUECURVE_Snowflakes_Count", 1, 20, 1, 20, vcCount, SNOWFLAKES_COUNT_MIN, SNOWFLAKES_COUNT_MAX);
            wxString type = parms[1];
            wxString vcCrap;
            type = RescaleWithRangeI(type, "IGNORE", 0, 5, 0, 5, vcCrap, -1, -1);
            wxString direction = parms[2]; // not used
            direction = RescaleWithRangeI(direction, "IGNORE", -8, 8, -8, 8, vcCrap, -1, -1);
            wxString speed = parms[3];
            wxString vcSpeed;
            speed = RescaleWithRangeI(speed, "E_VALUECURVE_Snowflakes_Speed", 0, 50, 0, 50, vcSpeed, SNOWFLAKES_SPEED_MIN, SNOWFLAKES_SPEED_MAX);
            wxString accumulation = parms[4];
            accumulation = RescaleWithRangeI(accumulation, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);

            settings += ",E_SLIDER_Snowflakes_Count=" + count;
            settings += ",E_SLIDER_Snowflakes_Type=" + type;
            settings += ",E_SLIDER_Snowflakes_Speed=" + speed;
            if (accumulation == "0") {
                settings += ",E_CHOICE_Falling=Falling";
                settings += vcCount;
                settings += vcSpeed;
            } else {
                settings += ",E_CHOICE_Falling=Falling & Accumulating";
                settings += vcCount;
                settings += vcSpeed;
            }
        } else if (effectType == "text") {
            // Hello%26nbsp%3B%20Keith,50,left,0,10,0,4,True
            wxString text = wxURI::Unescape(parms[0]);
            text.Replace("&gt;", ">");
            text.Replace("&lt;", "<");
            text.Replace("&nbsp;", " ");
            text.Replace("&amp;", "&");
            wxString fontSize = parms[1];
            wxString vcCrap;
            fontSize = RescaleWithRangeI(fontSize, "IGNORE", 0, 149, 0, 149, vcCrap, -1, -1);
            wxString movement = parms[2];
            wxString position = parms[3];
            position = RescaleWithRangeI(position, "IGNORE", -50, 49, -200, 200, vcCrap, -1, -1);
            wxString peekabooHoldTime = parms[4]; // unused
            peekabooHoldTime = RescaleWithRangeI(peekabooHoldTime, "IGNORE", 0, 99, 0, 99, vcCrap, -1, -1);
            wxString bounce = parms[5]; // unused
            bounce = RescaleWithRangeI(bounce, "IGNORE", 0, 99, 0, 99, vcCrap, -1, -1);
            wxString speed = parms[6];
            speed = RescaleWithRangeI(speed, "IGNORE", 0, 50, 0, 50, vcCrap, -1, -1);
            if (parms.size() > 7) {
                wxString unknown1 = parms[7]; // unused
            }

            settings += ",E_TEXTCTRL_Text=" + text;
            settings += ",E_CHOICE_Text_Font=Use OS Fonts";
            settings += ",E_FONTPICKER_Text_Font='segoe ui' " + fontSize;

            if (movement == "peekaboo_bottom") {
                settings += ",E_CHECKBOX_TextToCenter=1";
                movement = "up";
            }
            if (movement == "peekaboo_top") {
                settings += ",E_CHECKBOX_TextToCenter=1";
                movement = "down";
            }
            if (movement == "peekaboo_left") {
                settings += ",E_CHECKBOX_TextToCenter=1";
                movement = "left";
            }
            if (movement == "peekaboo_right") {
                settings += ",E_CHECKBOX_TextToCenter=1";
                movement = "right";
            }
            settings += ",E_CHOICE_Text_Dir=" + movement;
            settings += ",E_SLIDER_Text_XStart=" + position;
            settings += ",E_TEXTCTRL_Text_Speed=" + speed;
        } else if (effectType == "twinkle") {
            // 50,25,twinkle,random
            wxString rate = parms[0];
            wxString vcCrap;
            rate = RescaleWithRangeI(rate, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
            wxString density = parms[1];
            density = RescaleWithRangeI(density, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
            wxString mode = parms[2];
            wxString layout = parms[3];

            settings += ",E_SLIDER_Twinkle_Count=" + density;
            settings += ",E_SLIDER_Twinkle_Steps=" + rate;
            if (layout == "interval") {
                settings += ",E_CHECKBOX_Twinkle_ReRandom=0";
            } else if (layout == "random") {
                settings += ",E_CHECKBOX_Twinkle_ReRandom=1";
            }

            if (mode == "twinkle") {
                settings += ",E_CHECKBOX_Twinkle_Strobe=0";
            } else // pulse/flash
            {
                settings += ",E_CHECKBOX_Twinkle_Strobe=1";
            }

            logger_base.warn("LPE conversion for Twinkle not created yet.");
        } else {
            logger_base.warn("LPE conversion for %s not created yet.", (const char*)effectType.c_str());
            wxASSERT(false);
        }
    }

    return settings;
}

void MapLPE(const EffectManager& effect_manager, int i, EffectLayer* layer, const wxXmlDocument& input_xml, const wxString& model, bool left, int frequency, bool eraseExisting)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (eraseExisting)
        layer->DeleteAllEffects();

    for (wxXmlNode* e = input_xml.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "SequenceProps" || e->GetName() == "ArchivedProps") {
            for (wxXmlNode* prop = e->GetChildren(); prop != nullptr; prop = prop->GetNext()) {
                if (prop->GetName() == "SeqProp" || prop->GetName() == "ArchiveProp") {
                    std::string name = prop->GetAttribute("name").ToStdString();
                    if (name == "") {
                        for (wxXmlNode* ap = prop->GetChildren(); ap != nullptr; ap = ap->GetNext()) {
                            if (ap->GetName() == "PropClass") {
                                name = ap->GetAttribute("Name");
                            }
                        }
                    }
                    if (name == model) {
                        for (wxXmlNode* track = prop->GetChildren(); track != nullptr; track = track->GetNext()) {
                            int id = wxAtoi(track->GetAttribute("id"));
                            if (id == i) {
                                // now to add effects
                                for (wxXmlNode* effect = track->GetChildren(); effect != nullptr; effect = effect->GetNext()) {
                                    wxString type = effect->GetAttribute("type");

                                    if (effect->GetName() != "effect" || type != "pixelEffect") {
                                        logger_base.warn("LPE import node %s type %s not known.", (const char*)effect->GetName().c_str(), (const char*)type.c_str());
                                    } else {
                                        int startCentisecond = wxAtoi(effect->GetAttribute("startCentisecond"));
                                        int endCentisecond = wxAtoi(effect->GetAttribute("endCentisecond"));
                                        int startIntensity = wxAtoi(effect->GetAttribute("startIntensity", "100"));
                                        int endIntensity = wxAtoi(effect->GetAttribute("endIntensity", "100"));
                                        wxString settings = effect->GetAttribute("pixelEffect", "");
                                        wxArrayString settingsArray = wxSplit(settings, '|');
                                        wxString sideSettings;
                                        if (left) {
                                            if (settingsArray.size() == 7) {
                                                sideSettings = settingsArray[5];
                                            } else {
                                                sideSettings = settingsArray[3];
                                            }
                                        } else {
                                            if (settingsArray.size() == 7) {
                                                sideSettings = settingsArray[6];
                                            } else {
                                                sideSettings = settingsArray[4];
                                            }
                                        }
                                        wxArrayString effSettings = wxSplit(sideSettings, ':');
                                        wxString effectType = effSettings[0];
                                        if (effectType == "none") {
                                            // nothing to do
                                        } else {
                                            wxString ourEffectType = MapLPEEffectType(effectType);

                                            if (ourEffectType == "") {
                                                logger_base.warn("LPE import effect %s not known.", (const char*)effectType.c_str());
                                            } else {
                                                // skip over the multiple nodes PE creates when fading isnt perfectly even
                                                int fadeInCS, fadeOutCS;
                                                wxXmlNode* lastnode = FindLastLPEEffectNode(effect, startCentisecond, endCentisecond, endIntensity, endIntensity - startIntensity, settings, fadeInCS, fadeOutCS);
                                                if (lastnode != effect) {
                                                    endCentisecond = wxAtoi(lastnode->GetAttribute("endCentisecond"));
                                                    endIntensity = wxAtoi(lastnode->GetAttribute("endIntensity", "100"));
                                                    effect = lastnode;
                                                }

                                                // only create effect if there is nothing there
                                                if (!layer->HasEffectsInTimeRange(TimeLine::RoundToMultipleOfPeriod(startCentisecond * 10, frequency), TimeLine::RoundToMultipleOfPeriod(endCentisecond * 10, frequency))) {
                                                    wxString blend = MapLPEBlend(settingsArray[0], left);
                                                    int blendPos = wxAtoi(settingsArray[1]);
                                                    int sparkle = wxAtoi(settingsArray[2]);

                                                    // now we need to create the effect
                                                    std::string newpalette = ExtractLPEPallette(effSettings);
                                                    std::string newsettings = "T_CHOICE_LayerMethod=" + blend;

                                                    if (sparkle > 0) {
                                                        newpalette += ",C_SLIDER_SparkleFrequency=" + wxString::Format("%d", sparkle);
                                                    }
                                                    if (left && blendPos > 0) {
                                                        newsettings += ",T_SLIDER_EffectLayerMix=" + wxString::Format("%d", blendPos);
                                                    }

                                                    if (startIntensity == 100 && endIntensity == 100 && fadeInCS == 0 && fadeOutCS == 0) {
                                                        // dont need to do anything
                                                    } else if (startIntensity == endIntensity && fadeInCS == 0 && fadeOutCS == 0) {
                                                        // need to set brightness
                                                        newpalette += ",C_SLIDER_Brightness=" + wxString::Format("%d", startIntensity);
                                                    } else {
                                                        // need to set a brightness value curve
                                                        if (fadeInCS > 0) {
                                                            newsettings += ",T_TEXTCTRL_Fadein=" + wxString::Format("%.2f", (float)(fadeInCS - startCentisecond) / 100.0);
                                                        }
                                                        if (fadeOutCS > 0) {
                                                            newsettings += ",T_TEXTCTRL_Fadeout=" + wxString::Format("%.2f", (float)(endCentisecond - fadeOutCS) / 100.0);
                                                        }

                                                        if (fadeInCS == 0 && fadeOutCS == 0) {
                                                            newpalette += ",C_VALUECURVE_Brightness=Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Ramp|Min=0.00|Max=400.00|P1=" + wxString::Format("%d", startIntensity) + "|P2=" + wxString::Format("%d", endIntensity) + "|RV=TRUE|";
                                                        }
                                                    }

                                                    newsettings += LPEParseEffectSettings(effectType, effSettings, newpalette, (endCentisecond - startCentisecond) * 10);

                                                    layer->AddEffect(0, ourEffectType, newsettings, newpalette, TimeLine::RoundToMultipleOfPeriod(startCentisecond * 10, frequency), TimeLine::RoundToMultipleOfPeriod(endCentisecond * 10, frequency), false, false);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        return;
                    }
                }
            }
        }
    }
}

void MapLPEEffects(const EffectManager& effectManager, Element* model, const wxXmlDocument& input_xml, const wxString& mapping, int frequency, bool eraseExisting)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int layer = 0;
    if (LPEHasEffects(input_xml, mapping, 0, true)) {
        logger_base.debug("Creating effects on model %s layer %d from %s layer 0 left hand side",
                          (const char*)model->GetFullName().c_str(), layer + 1, (const char*)mapping.c_str());
        MapLPE(effectManager, 0, model->GetEffectLayer(layer), input_xml, mapping, true, frequency, eraseExisting);
    }
    if (LPEHasEffects(input_xml, mapping, 0, false)) {
        layer++;
        if (model->GetEffectLayerCount() < layer + 1) {
            model->AddEffectLayer();
        }
        logger_base.debug("Creating effects on model %s layer %d from %s layer 0 right hand side",
                          (const char*)model->GetFullName().c_str(), layer + 1, (const char*)mapping.c_str());
        MapLPE(effectManager, 0, model->GetEffectLayer(layer), input_xml, mapping, false, frequency, eraseExisting);
    }
    if (LPEHasEffects(input_xml, mapping, 1, true)) {
        layer++;
        if (model->GetEffectLayerCount() < layer + 1) {
            model->AddEffectLayer();
        }
        logger_base.debug("Creating effects on model %s layer %d from %s layer 1 left hand side",
                          (const char*)model->GetFullName().c_str(), layer + 1, (const char*)mapping.c_str());
        MapLPE(effectManager, 1, model->GetEffectLayer(layer), input_xml, mapping, true, frequency, eraseExisting);
    }
    if (LPEHasEffects(input_xml, mapping, 1, false)) {
        layer++;
        if (model->GetEffectLayerCount() < layer + 1) {
            model->AddEffectLayer();
        }
        logger_base.debug("Creating effects on model %s layer %d from %s layer 1 right hand side",
                          (const char*)model->GetFullName().c_str(), layer + 1, (const char*)mapping.c_str());
        MapLPE(effectManager, 1, model->GetEffectLayer(layer), input_xml, mapping, false, frequency, eraseExisting);
    }
}

void MapS5(const EffectManager& effect_manager, int layer, EffectLayer* el, const LOREdit& lorEdit, const wxString& model, Model* m, int frequency, int offset, bool eraseExisting)
{
    if (el == nullptr)
        return;

    if (eraseExisting)
        el->DeleteAllEffects();

    bool channelBlock = (m != nullptr && m->GetDisplayAs() == "Channel Block");

    auto st = lorEdit.GetSequencingType(model);

    if (st == loreditType::CHANNELS) {
        auto effects = lorEdit.GetChannelEffects(model, 0, m, offset);

        for (const auto& it : effects) {
            if (!el->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                // for channel blocks we always use white as the colour comes from the channel block
                if (channelBlock)
                    palette = "C_BUTTON_Palette1=#ffffff,C_CHECKBOX_Palette1=1";
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    el->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        // pixel effects on a node ... not useful but whatever
        auto effects = lorEdit.GetTrackEffects(model, layer, offset);

        for (const auto& it : effects) {
            if (!el->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    el->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    }
}

void MapS5ChannelEffects(const EffectManager& effectManager, int node, EffectLayer* nl, Model* m, const LOREdit& lorEdit, const wxString& mapping, int frequency, int offset, bool eraseExisting)
{
    if (nl == nullptr)
        return;

    if (eraseExisting)
        nl->DeleteAllEffects();

    bool channelBlock = (m != nullptr && m->GetDisplayAs() == "Channel Block");

    auto st = lorEdit.GetSequencingType(mapping);

    if (st == loreditType::CHANNELS) {
        auto effects = lorEdit.GetChannelEffects(mapping, node, m, offset);

        for (const auto& it : effects) {
            if (!nl->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                // for channel blocks we always use white as the colour comes from the channel block
                if (channelBlock)
                    palette = "C_BUTTON_Palette1=#ffffff,C_CHECKBOX_Palette1=1";
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    nl->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        // pixel effects on a node ... not useful but whatever
        auto effects = lorEdit.GetTrackEffects(mapping, 0, offset);

        for (const auto& it : effects) {
            if (!nl->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    nl->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    }
}

void MapS5ChannelEffects(const EffectManager& effectManager, EffectLayer* layer, const LOREdit& lorEdit, const wxString& mapping, int frequency, int offset, bool eraseExisting)
{
    if (eraseExisting)
        layer->DeleteAllEffects();

    Model* m = layer->GetParentElement()->GetSequenceElements()->GetXLightsFrame()->AllModels[layer->GetParentElement()->GetModelName()];
    bool channelBlock = (m != nullptr && m->GetDisplayAs() == "Channel Block");

    static wxRegEx regex("\\[(\\d+),(\\d+),(\\d+)\\]\\[(.*)\\]", wxRE_ADVANCED | wxRE_NEWLINE);
    if (regex.Matches(mapping)) {
        int const row = wxAtoi(regex.GetMatch(mapping, 1).ToStdString());
        int const col = wxAtoi(regex.GetMatch(mapping, 2).ToStdString());
        int const color = wxAtoi(regex.GetMatch(mapping, 3).ToStdString());
        std::string strColor = regex.GetMatch(mapping, 4).ToStdString();
        wxString name = mapping;
        wxString const end = regex.GetMatch(mapping, 0);
        name.Replace(end, "");

        auto effects = lorEdit.GetChannelEffects(name, row, col, color, offset);

        for (auto& it : effects) {
            if (!layer->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                LOREdit::setNodeColor(strColor, it);
                std::string palette = it.GetPalette();
                // for channel blocks we always use white as the colour comes from the channel block
                if (channelBlock)
                    palette = "C_BUTTON_Palette1=#ffffff,C_CHECKBOX_Palette1=1";
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    layer->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    }
}

void MapS5ChannelEffects(const EffectManager& effectManager, int node, EffectLayer* nl, int nodes, const LOREdit& lorEdit, const wxString& mapping, int frequency, int offset, bool eraseExisting)
{
    if (nl == nullptr)
        return;

    if (eraseExisting)
        nl->DeleteAllEffects();

    Model* m = nl->GetParentElement()->GetSequenceElements()->GetXLightsFrame()->AllModels[nl->GetParentElement()->GetModelName()];
    bool channelBlock = (m != nullptr && m->GetDisplayAs() == "Channel Block");

    auto st = lorEdit.GetSequencingType(mapping);

    if (st == loreditType::CHANNELS) {
        auto effects = lorEdit.GetChannelEffects(mapping, node, nodes, offset);

        for (const auto& it : effects) {
            if (!nl->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                // for channel blocks we always use white as the colour comes from the channel block
                if (channelBlock)
                    palette = "C_BUTTON_Palette1=#ffffff,C_CHECKBOX_Palette1=1";
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    nl->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        // pixel effects on a node ... not useful but whatever
        auto effects = lorEdit.GetTrackEffects(mapping, 0, offset);

        for (const auto& it : effects) {
            if (!nl->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    nl->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    }
}

void MapS5Effects(const EffectManager& effectManager, Element* model, const LOREdit& lorEdit, const wxString& mapping, int frequency, int offset, bool eraseExisting)
{
    // static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    auto st = lorEdit.GetSequencingType(mapping);
    Model* m = model->GetSequenceElements()->GetXLightsFrame()->AllModels[model->GetModelName()];

    if (st == loreditType::CHANNELS) {
        if (m->GetNodeCount() == 1) {
            MapS5ChannelEffects(effectManager, 0, model->GetEffectLayer(0), m, lorEdit, mapping, frequency, offset, eraseExisting);
        } else {
            int lr, lc;
            lorEdit.GetModelChannels(mapping, lr, lc);

            if (lr == 1 && lc == 1) {
                MapS5ChannelEffects(effectManager, 0, model->GetEffectLayer(0), m, lorEdit, mapping, frequency, offset, eraseExisting);
            } else {
                for (uint32_t i = 0; i < m->GetNodeCount(); i++) {
                    NodeLayer* nl = model->GetNodeEffectLayer(i);
                    if (nl != nullptr) {
                        MapS5ChannelEffects(effectManager, i, nl, m, lorEdit, mapping, frequency, offset, eraseExisting);
                    }
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        for (int i = 0; i < lorEdit.GetModelLayers(mapping); i++) {
            if (model->GetEffectLayerCount() < i + 1) {
                model->AddEffectLayer();
            }
            MapS5(effectManager, i, model->GetEffectLayer(i), lorEdit, mapping, m, frequency, offset, eraseExisting);
        }
    }
}

void MapS5Effects(const EffectManager& effectManager, StrandElement* se, const LOREdit& lorEdit, const wxString& mapping, int frequency, int offset, bool eraseExisting)
{
    // static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    auto st = lorEdit.GetSequencingType(mapping);
    Model* m = se->GetSequenceElements()->GetXLightsFrame()->AllModels[se->GetModelName()];

    if (st == loreditType::CHANNELS) {
        if (se->GetNodeLayerCount() == 1) {
            MapS5ChannelEffects(effectManager, 0, se->GetEffectLayer(0), 1, lorEdit, mapping, frequency, offset, eraseExisting);
        } else {
            int lr, lc;
            lorEdit.GetModelChannels(mapping, lr, lc);

            if (lr == 1 && lc == 1) {
                MapS5ChannelEffects(effectManager, 0, se->GetEffectLayer(0), 1, lorEdit, mapping, frequency, offset, eraseExisting);
            } else {
                int nodes = se->GetNodeLayerCount();
                for (int i = 0; i < nodes; i++) {
                    NodeLayer* nl = se->GetNodeEffectLayer(i);
                    if (nl != nullptr) {
                        MapS5ChannelEffects(effectManager, i, nl, nodes, lorEdit, mapping, frequency, offset, eraseExisting);
                    }
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        for (int i = 0; i < lorEdit.GetModelLayers(mapping); i++) {
            if (se->GetEffectLayerCount() < i + 1) {
                se->AddEffectLayer();
            }
            MapS5(effectManager, i, se->GetEffectLayer(i), lorEdit, mapping, m, frequency, offset, eraseExisting);
        }
    }
}

void MapS5Effects(const EffectManager& effectManager, SubModelElement* se, const LOREdit& lorEdit, const wxString& mapping, int frequency, int offset, bool eraseExisting)
{
    if (dynamic_cast<StrandElement*>(se) != nullptr) {
        return MapS5Effects(effectManager, dynamic_cast<StrandElement*>(se), lorEdit, mapping, frequency, offset, eraseExisting);
    }

    // static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    auto st = lorEdit.GetSequencingType(mapping);
    Model* m = se->GetSequenceElements()->GetXLightsFrame()->AllModels[se->GetModelName()];

    if (st == loreditType::CHANNELS) {
        if (m->GetNodeCount() == 1) {
            MapS5ChannelEffects(effectManager, 0, se->GetEffectLayer(0), m, lorEdit, mapping, frequency, offset, eraseExisting);
        } else {
            int lr, lc;
            lorEdit.GetModelChannels(mapping, lr, lc);

            if (lr == 1 && lc == 1) {
                MapS5ChannelEffects(effectManager, 0, se->GetEffectLayer(0), m, lorEdit, mapping, frequency, offset, eraseExisting);
            } else {
                for (uint32_t i = 0; i < m->GetNodeCount(); i++) {
                    NodeLayer* nl = se->GetNodeEffectLayer(i);
                    if (nl != nullptr) {
                        MapS5ChannelEffects(effectManager, i, nl, m, lorEdit, mapping, frequency, offset, eraseExisting);
                    }
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        for (int i = 0; i < lorEdit.GetModelLayers(mapping); i++) {
            if (se->GetEffectLayerCount() < i + 1) {
                se->AddEffectLayer();
            }
            MapS5(effectManager, i, se->GetEffectLayer(i), lorEdit, mapping, m, frequency, offset, eraseExisting);
        }
    }
}

bool xLightsFrame::ImportS5(wxXmlDocument& input_xml, const wxFileName& filename)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    DisplayWarning(
        "WARNING: As at this release S5 import is experimental and its improvement relies on your feedback.\nIf it doesnt do a good job let us know by telling us:\n\
        - which effect\n\
        - which setting you had to fine tune\n\
        - what it was when it was converted\n\
        - what you changed it to.\n",
        this);

    LOREdit lorEdit(input_xml, CurrentSeqXmlFile->GetFrequency());

    xLightsImportChannelMapDialog dlg(this, filename, true, true, false, true, false);
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;

    dlg.timingTracks = lorEdit.GetTimingTracks();
    for (auto const& m : lorEdit.GetModelsWithEffects()) {
        dlg.AddChannel(m);
    }
    dlg.ccrNames = lorEdit.GetNodesWithEffects();

    dlg.SortChannels();

    dlg.InitImport("Stands and Channels");

    if (dlg.ShowModal() != wxID_OK || dlg._dataModel == nullptr) {
        return false;
    }

    logger_base.debug("Importing S5 effects from %s.", (const char*)filename.GetFullPath().c_str());

    int offset = dlg.TimeAdjustSpinCtrl->GetValue();

    for (size_t tt = 0; tt < dlg.TimingTrackListBox->GetCount(); ++tt) {
        if (dlg.TimingTrackListBox->IsChecked(tt)) {
            std::string name = dlg.TimingTrackListBox->GetString(tt).ToStdString();

            auto timings = lorEdit.GetTimings(name, offset);

            TimingElement* target = (TimingElement*)_sequenceElements.AddElement(name, "timing", true, true, false, false, false);
            char cnt = '1';
            while (target == nullptr) {
                target = (TimingElement*)_sequenceElements.AddElement(name + "-" + cnt++, "timing", true, true, false, false, false);
            }
            if (target->GetEffectLayerCount() == 0) {
                target->AddEffectLayer();
            }

            EffectLayer* targetLayer = target->GetEffectLayer(0);

            for (auto t : timings) {
                targetLayer->AddEffect(0, "", "", "", t.first, t.second, false, false);
            }
        }
    }

    for (size_t i = 0; i < dlg._dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = dlg._dataModel->GetNthChild(i);
        std::string modelName = m->_model.ToStdString();
        ModelElement* model = nullptr;
        for (size_t x = 0; x < _sequenceElements.GetElementCount(); x++) {
            if (_sequenceElements.GetElement(x)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(x)->GetName()) {
                model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(x));
                break;
            }
        }
        if (model != nullptr) {
            Model* mdl = model->GetSequenceElements()->GetXLightsFrame()->AllModels[model->GetModelName()];

            if (m->_mapping != "") {
                if (model == nullptr) {
                    model = AddModel(GetModel(modelName), _sequenceElements);
                }
                if (model == nullptr) {
                    logger_base.error("Attempt to add model %s during S5 import failed.", (const char*)modelName.c_str());
                } else {
                    if (!LOREdit::IsNodeStrandMapping(m->_mapping))
                        MapS5Effects(effectManager, model, lorEdit, m->_mapping, CurrentSeqXmlFile->GetFrequency(), offset, dlg.CheckBox_EraseExistingEffects->GetValue());
                    else
                        MapS5ChannelEffects(effectManager, model->GetEffectLayer(0), lorEdit, m->_mapping, CurrentSeqXmlFile->GetFrequency(), offset, dlg.CheckBox_EraseExistingEffects->GetValue());
                }
            }

            int str = 0;
            for (size_t j = 0; j < m->GetChildCount(); j++) {
                xLightsImportModelNode* s = m->GetNthChild(j);

                if ("" != s->_mapping) {
                    if (model == nullptr) {
                        model = AddModel(GetModel(modelName), _sequenceElements);
                    }
                    if (model == nullptr) {
                        logger_base.error("Attempt to add model %s during S5 import failed.", (const char*)modelName.c_str());
                    } else {
                        SubModelElement* ste = model->GetSubModel(str);
                        if (ste != nullptr) {
                            if (!LOREdit::IsNodeStrandMapping(s->_mapping))
                                MapS5Effects(effectManager, ste, lorEdit, s->_mapping, CurrentSeqXmlFile->GetFrequency(), offset, dlg.CheckBox_EraseExistingEffects->GetValue());
                            else
                                MapS5ChannelEffects(effectManager, ste->GetEffectLayer(0), lorEdit, s->_mapping, CurrentSeqXmlFile->GetFrequency(), offset, dlg.CheckBox_EraseExistingEffects->GetValue());
                        }
                    }
                }
                for (size_t n = 0; n < s->GetChildCount(); n++) {
                    xLightsImportModelNode* ns = s->GetNthChild(n);
                    if ("" != ns->_mapping) {
                        if (model == nullptr) {
                            model = AddModel(GetModel(modelName), _sequenceElements);
                        }
                        if (model == nullptr) {
                            logger_base.error("Attempt to add model %s during S5 import failed.", (const char*)modelName.c_str());
                        } else {
                            SubModelElement* ste = model->GetSubModel(str);
                            StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                            if (stre != nullptr) {
                                NodeLayer* nl = stre->GetNodeLayer(n, true);
                                if (nl != nullptr) {
                                    if (LOREdit::IsNodeStrandMapping(s->_mapping)) {
                                        MapS5ChannelEffects(effectManager, nl, lorEdit, s->_mapping, CurrentSeqXmlFile->GetFrequency(), offset, dlg.CheckBox_EraseExistingEffects->GetValue());
                                    } else {
                                        auto st = lorEdit.GetSequencingType(ns->_mapping);
                                        if (st == loreditType::CHANNELS) {
                                            MapS5ChannelEffects(effectManager, i, nl, mdl, lorEdit, ns->_mapping, CurrentSeqXmlFile->GetFrequency(), offset, dlg.CheckBox_EraseExistingEffects->GetValue());
                                        } else if (st == loreditType::TRACKS) {
                                            // no layers so we just map the first
                                            MapS5(effectManager, 0, nl, lorEdit, ns->_mapping, mdl, CurrentSeqXmlFile->GetFrequency(), offset, dlg.CheckBox_EraseExistingEffects->GetValue());
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                str++;
            }
        }
    }

    logger_base.debug("    Importing S5 effects done.");

    return true;
}

bool xLightsFrame::ImportLPE(wxXmlDocument& input_xml, const wxFileName& filename)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    DisplayWarning(
        "WARNING: As at this release PixelEditor import is experimental and its improvement relies on your feedback.\nIf it doesnt do a good job let us know by telling us:\n\
        - which effect\n\
        - which setting you had to fine tune\n\
        - what it was when it was converted\n\
        - what you changed it to.\n",
        this);

    xLightsImportChannelMapDialog dlg(this, filename, true, false, false, false, false);
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;
    std::vector<std::string> timingTrackNames;
    std::map<std::string, wxXmlNode*> timingTracks;

    for (wxXmlNode* e = input_xml.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "SequenceProps" || e->GetName() == "ArchivedProps") {
            for (wxXmlNode* prop = e->GetChildren(); prop != nullptr; prop = prop->GetNext()) {
                if (prop->GetName() == "SeqProp" || prop->GetName() == "ArchiveProp") {
                    std::string name = prop->GetAttribute("name").ToStdString();
                    if (name == "") {
                        for (wxXmlNode* ap = prop->GetChildren(); ap != nullptr; ap = ap->GetNext()) {
                            if (ap->GetName() == "PropClass") {
                                name = ap->GetAttribute("Name");
                            }
                        }
                    }
                    dlg.AddChannel(name);
                    dlg.channelColors[name] = xlBLACK;
                }
            }
        }
    }

    dlg.SortChannels();
    dlg.timingTracks = timingTrackNames;

    dlg.InitImport();

    if (dlg.ShowModal() != wxID_OK || dlg._dataModel == nullptr) {
        return false;
    }

    logger_base.debug("Importing LPE effects from %s.", (const char*)filename.GetFullPath().c_str());

    if (dlg.TimeAdjustSpinCtrl->GetValue() != 0) {
        int offset = dlg.TimeAdjustSpinCtrl->GetValue();
        AdjustAllTimings(input_xml.GetRoot(), offset / 10);
    }

    for (size_t i = 0; i < dlg._dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = dlg._dataModel->GetNthChild(i);
        std::string modelName = m->_model.ToStdString();
        ModelElement* model = nullptr;
        for (size_t x = 0; x < _sequenceElements.GetElementCount(); x++) {
            if (_sequenceElements.GetElement(x)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(x)->GetName()) {
                model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(x));
                break;
            }
        }

        if (m->_mapping != "") {
            if (model == nullptr) {
                model = AddModel(GetModel(modelName), _sequenceElements);
            }
            if (model == nullptr) {
                logger_base.error("Attempt to add model %s during LPE import failed.", (const char*)modelName.c_str());
            } else {
                MapLPEEffects(effectManager, model, input_xml, m->_mapping, CurrentSeqXmlFile->GetFrequency(), dlg.CheckBox_EraseExistingEffects->GetValue());
            }
        }

        int str = 0;
        for (size_t j = 0; j < m->GetChildCount(); j++) {
            xLightsImportModelNode* s = m->GetNthChild(j);

            if ("" != s->_mapping) {
                if (model == nullptr) {
                    model = AddModel(GetModel(modelName), _sequenceElements);
                }
                if (model == nullptr) {
                    logger_base.error("Attempt to add model %s during LPE import failed.", (const char*)modelName.c_str());
                } else {
                    SubModelElement* ste = model->GetSubModel(str);
                    if (ste != nullptr) {
                        MapLPEEffects(effectManager, ste, input_xml, s->_mapping, CurrentSeqXmlFile->GetFrequency(), dlg.CheckBox_EraseExistingEffects->GetValue());
                    }
                }
            }
            for (size_t n = 0; n < s->GetChildCount(); n++) {
                xLightsImportModelNode* ns = s->GetNthChild(n);
                if ("" != ns->_mapping) {
                    if (model == nullptr) {
                        model = AddModel(GetModel(modelName), _sequenceElements);
                    }
                    if (model == nullptr) {
                        logger_base.error("Attempt to add model %s during LPE import failed.", (const char*)modelName.c_str());
                    } else {
                        SubModelElement* ste = model->GetSubModel(str);
                        StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                        if (stre != nullptr) {
                            NodeLayer* nl = stre->GetNodeLayer(n, true);
                            if (nl != nullptr) {
                                MapLPE(effectManager, 0, nl, input_xml, ns->_mapping, true, CurrentSeqXmlFile->GetFrequency(), dlg.CheckBox_EraseExistingEffects->GetValue());
                            }
                        }
                    }
                }
            }
            str++;
        }
    }

    logger_base.debug("    Importing LPE effects done.");

    return true;
}

void MapVixen3(Element* model, const Vixen3& vixen, const wxString& modelName, long offset, int frameMS, bool eraseExisting)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (eraseExisting) {
        for (const auto& it : model->GetEffectLayers()) {
            it->DeleteAllEffects();
        }
    }

    auto effects = vixen.GetEffects(modelName.ToStdString());

    for (const auto& it : effects) {
        long s = Vixen3::ConvertTiming(it.start + offset, frameMS);
        long e = Vixen3::ConvertTiming(it.end + offset, frameMS);

        // Vixen can have multiple effects in one time slot so add layers as needed
        EffectLayer* layer = nullptr;
        for (const auto& it : model->GetEffectLayers()) {
            if (!it->HasEffectsInTimeRange(s, e)) {
                layer = it;
                break;
            }
        }

        if (layer == nullptr)
            layer = model->AddEffectLayer();

        // now we need to create the effect
        std::string newpalette = it.GetPalette();
        std::string newsettings = it.GetSettings();
        std::string type = it.GetXLightsType();
        if (type != "") {
            if (layer->GetParentElement()->GetSequenceElements()->GetEffectManager().GetEffectIndex(type) < 0) {
                logger_base.debug("Vixen 3 import %s -> %s is not a valid effect.", (const char*)it.type.c_str(), (const char*)type.c_str());
            } else {
                layer->AddEffect(0, type, newsettings, newpalette, s, e, false, false);
            }
        }
    }
}

void MapVixen3Effects(const EffectManager& effectManager, Element* model, const Vixen3& vixen, const wxString& mapping, long offset, int frameMS, bool eraseExisting)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Creating effects on model %s from %s",
                      (const char*)model->GetFullName().c_str(), (const char*)mapping.c_str());
    MapVixen3(model, vixen, mapping, offset, frameMS, eraseExisting);
}

bool xLightsFrame::ImportVixen3(const wxFileName& filename)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxMessageBox(
        "WARNING: As at this release Vixen3 import is experimental and its improvement relies on your feedback.\nIf it doesnt do a good job let us know by telling us:\n\
        - which effect\n\
        - which setting you had to fine tune\n\
        - what it was when it was converted\n\
        - what you changed it to.\n\n\n\
AT THIS POINT IT JUST BRINGS IN THE EFFECTS. WE MAKE NO EFFORT TO GET THE SETTINGS RIGHT!");

    Vixen3 vixen(filename.GetFullPath().ToStdString());

    if (!vixen.IsSystemFound()) {
        wxMessageBox("SystemConfig.xml could not be found. Import impossible.");
        return false;
    }

    xLightsImportChannelMapDialog dlg(this, filename, true, true, false, false, false);
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;

    auto models = vixen.GetModelsWithEffects();
    for (auto [model, numEff] : models) {
        auto effects = vixen.GetEffects(model);

        if (effects.front().type == "Data") {
            dlg.timingTracks.push_back(model);
        } else {
            dlg.AddChannel(model, numEff);
        }
    }
    dlg.SortChannels();
    auto timings = vixen.GetTimings();
    for (const auto& it : timings) {
        dlg.timingTracks.push_back(it);
    }

    dlg.InitImport();

    if (dlg.ShowModal() != wxID_OK || dlg._dataModel == nullptr) {
        return false;
    }

    logger_base.debug("Importing Vixen 3 effects from %s.", (const char*)filename.GetFullPath().c_str());

    int offset = dlg.TimeAdjustSpinCtrl->GetValue();

    for (size_t tt = 0; tt < dlg.TimingTrackListBox->GetCount(); ++tt) {
        if (dlg.TimingTrackListBox->IsChecked(tt)) {
            std::string name = dlg.TimingTrackListBox->GetString(tt).ToStdString();

            if (vixen.GetTimingType(name) == "Phrase") {
                TimingElement* element = AddTimingElement(name);
                EffectLayer* effectLayer = element->GetEffectLayer(0);
                if (effectLayer == nullptr) {
                    effectLayer = element->AddEffectLayer();
                }

                xLightsXmlFile::AddMarksToLayer(vixen.GetTimings(name), effectLayer, CurrentSeqXmlFile->GetFrameMS());
                effectLayer = element->AddEffectLayer();
                xLightsXmlFile::AddMarksToLayer(vixen.GetRelatedTiming(name, "Word"), effectLayer, CurrentSeqXmlFile->GetFrameMS());
                effectLayer = element->AddEffectLayer();
                xLightsXmlFile::AddMarksToLayer(vixen.GetRelatedTiming(name, "Phoneme"), effectLayer, CurrentSeqXmlFile->GetFrameMS());
            } else {
                TimingElement* element = AddTimingElement(name);
                EffectLayer* effectLayer = element->GetEffectLayer(0);
                if (effectLayer == nullptr) {
                    effectLayer = element->AddEffectLayer();
                }

                xLightsXmlFile::AddMarksToLayer(vixen.GetTimings(name), effectLayer, CurrentSeqXmlFile->GetFrameMS());
            }
        }
    }

    for (size_t i = 0; i < dlg._dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = dlg._dataModel->GetNthChild(i);
        std::string modelName = m->_model.ToStdString();
        ModelElement* model = nullptr;
        for (size_t x = 0; x < _sequenceElements.GetElementCount(); x++) {
            if (_sequenceElements.GetElement(x)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(x)->GetName()) {
                model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(x));
                break;
            }
        }

        if (m->_mapping != "") {
            if (model == nullptr) {
                model = AddModel(GetModel(modelName), _sequenceElements);
            }
            if (model == nullptr) {
                logger_base.error("Attempt to add model %s during Vixen 3 import failed.", (const char*)modelName.c_str());
            } else {
                MapVixen3Effects(effectManager, model, vixen, m->_mapping, offset, CurrentSeqXmlFile->GetFrameMS(), dlg.CheckBox_EraseExistingEffects->GetValue());
            }
        }

        int str = 0;
        for (size_t j = 0; j < m->GetChildCount(); j++) {
            xLightsImportModelNode* s = m->GetNthChild(j);

            if ("" != s->_mapping) {
                if (model == nullptr) {
                    model = AddModel(GetModel(modelName), _sequenceElements);
                }
                if (model == nullptr) {
                    logger_base.error("Attempt to add model %s during Vixen 3 import failed.", (const char*)modelName.c_str());
                } else {
                    SubModelElement* ste = model->GetSubModel(str);
                    if (ste != nullptr) {
                        MapVixen3Effects(effectManager, ste, vixen, s->_mapping, offset, CurrentSeqXmlFile->GetFrameMS(), dlg.CheckBox_EraseExistingEffects->GetValue());
                    }
                }
            }
            for (size_t n = 0; n < s->GetChildCount(); n++) {
                xLightsImportModelNode* ns = s->GetNthChild(n);
                if ("" != ns->_mapping) {
                    if (model == nullptr) {
                        model = AddModel(GetModel(modelName), _sequenceElements);
                    }
                    if (model == nullptr) {
                        logger_base.error("Attempt to add model %s during Vixen 3 import failed.", (const char*)modelName.c_str());
                    } else {
                        SubModelElement* ste = model->GetSubModel(str);
                        StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                        if (stre != nullptr) {
                            NodeLayer* nl = stre->GetNodeLayer(n, true);
                            if (nl != nullptr) {
                                MapVixen3(model, vixen, ns->_mapping, offset, CurrentSeqXmlFile->GetFrameMS(), dlg.CheckBox_EraseExistingEffects->GetValue());
                            }
                        }
                    }
                }
            }
            str++;
        }
    }

    logger_base.debug("    Importing Vixen 3 effects done.");

    return true;
}

unsigned char ChannelBlend(unsigned char c1, unsigned char c2, double ratio)
{
    return c1 + floor(ratio * (c2 - c1) + 0.5);
}

class ImageInfo
{
public:
    int xOffset;
    int yOffset;
    int width;
    int height;
    double scaleY;
    double scaleX;
    std::string imageName;

    void Set(int x, int y, int w, int h, const std::string& n)
    {
        xOffset = x;
        yOffset = y;
        width = w;
        height = h;
        imageName = n;
        scaleX = 1.0;
        scaleY = 1.0;
    }
};

void ScaleImage(wxImage& img, int type,
                const wxSize& modelSize,
                int numCol, int numRow,
                ImageInfo& imgInfo,
                bool reverse)
{
    bool scale = false;

    imgInfo.xOffset = imgInfo.xOffset + (imgInfo.width - numCol) / 2;
    if (reverse) {
        imgInfo.yOffset = imgInfo.yOffset + (imgInfo.height + 0.5) / 2 - numRow / 2;
    } else {
        imgInfo.yOffset = numRow - imgInfo.yOffset - (numRow + imgInfo.height) / 2;
    }

    switch (type) {
    case 0: // NONE
        return;
    case 1: // Exact width
        if (numCol == imgInfo.width) {
            imgInfo.width = modelSize.GetWidth();
            imgInfo.scaleX = ((double)modelSize.GetWidth()) / ((double)numCol);
            imgInfo.xOffset = round((double)imgInfo.xOffset * imgInfo.scaleX);
            scale = true;
        }
        break;
    case 2: // Exact height
        if (numRow == imgInfo.height) {
            imgInfo.height = modelSize.GetHeight();
            imgInfo.scaleY = ((double)modelSize.GetHeight()) / ((double)numRow);
            imgInfo.yOffset = round((double)imgInfo.yOffset * imgInfo.scaleY);
            scale = true;
        }
        break;
    case 3: // Exact width or height
        if (numCol == imgInfo.width) {
            imgInfo.width = modelSize.GetWidth();
            imgInfo.scaleX = ((double)modelSize.GetWidth()) / ((double)numCol);
            imgInfo.xOffset = round((double)imgInfo.xOffset * imgInfo.scaleX);
            scale = true;
        }
        if (numRow == imgInfo.height) {
            imgInfo.height = modelSize.GetHeight();
            imgInfo.scaleY = ((double)modelSize.GetHeight()) / ((double)numRow);
            imgInfo.yOffset = round((double)imgInfo.yOffset * imgInfo.scaleY);
            scale = true;
        }
        break;
    case 4: // everything
        imgInfo.scaleX = ((double)modelSize.GetWidth()) / ((double)numCol);
        imgInfo.scaleY = ((double)modelSize.GetHeight()) / ((double)numRow);
        int newW = round((double)imgInfo.width * imgInfo.scaleX);
        int newH = round((double)imgInfo.height * imgInfo.scaleY);
        if (newH != imgInfo.height || newW != imgInfo.width) {
            scale = true;
            imgInfo.height = newH;
            imgInfo.width = newW;
            imgInfo.yOffset = round((double)imgInfo.yOffset * imgInfo.scaleY);
            imgInfo.xOffset = round((double)imgInfo.xOffset * imgInfo.scaleX);
        }
        break;
    }
    if (scale) {
        img.Rescale(imgInfo.width, imgInfo.height);
    }
}

wxString CreateSceneImage(const std::string& imagePfx, const std::string& postFix,
                          wxXmlNode* element, int numCols,
                          int numRows, bool reverse, bool rotate, const xlColor& color, int y_offset,
                          int resizeType, const wxSize& modelSize)
{
    wxImage i;
    i.Create(numCols, numRows);
    i.InitAlpha();
    for (int x = 0; x < numCols; x++) {
        for (int y = 0; y < numRows; y++) {
            i.SetAlpha(x, y, wxALPHA_TRANSPARENT);
        }
    }
    for (wxXmlNode* e = element->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "element") {
            int x = wxAtoi(e->GetAttribute("ribbonIndex"));
            int y = wxAtoi(e->GetAttribute("pixelIndex")) - y_offset;
            if (rotate) {
                std::swap(x, y);
            }
            if (rotate ^ reverse) {
                y = numRows - y;
            }
            if (x < numCols && y >= 0 && y < numRows) {
                i.SetRGB(x, y, color.Red(), color.Green(), color.Blue());
                i.SetAlpha(x, y, wxALPHA_OPAQUE);
            }
        }
    }
    std::string name = imagePfx + "_s" + element->GetAttribute("savedIndex").ToStdString() + postFix + ".png";
    ImageInfo im;
    im.Set(0, 0, numCols, numRows, name);
    ScaleImage(i, resizeType, modelSize, numCols, numRows, im, false);
    i.SaveFile(name);
    return name;
}

bool IsPartOfModel(wxXmlNode* element, int num_rows, int num_columns, bool& isFull, wxRect& rect, bool reverse)
{
    if (element == nullptr)
        return false;

    std::vector<std::vector<bool>> data(num_columns, std::vector<bool>(num_rows));
    int maxCol = -1;
    int maxRow = -1;
    int minCol = 9999999;
    int minRow = 9999999;
    isFull = true;
    for (wxXmlNode* e = element->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "element") {
            int x = wxAtoi(e->GetAttribute("ribbonIndex"));
            int y = wxAtoi(e->GetAttribute("pixelIndex"));
            if (reverse) {
                std::swap(x, y);
            }
            if (x < num_columns && y < num_rows) {
                data[x][y] = true;
                if (x > maxCol)
                    maxCol = x;
                if (x < minCol)
                    minCol = x;
                if (y > maxRow)
                    maxRow = y;
                if (y < minRow)
                    minRow = y;
            } else {
                return false;
            }
        }
    }
    isFull = minCol == 0 && minRow == 0 && maxRow == (num_rows - 1) && maxCol == (num_columns - 1);
    bool isRect = true;
    for (int x = minCol; x <= maxCol; x++) {
        for (int y = minRow; y <= maxRow; y++) {
            if (!data[x][y]) {
                isFull = false;
                isRect = false;
            }
        }
    }
    if (isRect) {
        rect.x = minCol;
        rect.y = minRow;
        rect.width = maxCol;
        rect.height = maxRow;
    } else {
        rect.x = -1;
        rect.y = -1;
    }
    return true;
}

bool xLightsFrame::ImportSuperStar(Element* model, wxXmlDocument& input_xml, int x_size, int y_size,
                                   int x_offset, int y_offset,
                                   int imageResizeType, const wxSize& modelSize, const wxString& layerBlend)
{
    double num_rows = 1.0;
    double num_columns = 1.0;
    bool reverse_rows = false;
    bool reverse_xy = false;
    bool layout_defined = false;
    wxXmlNode* input_root = input_xml.GetRoot();
    EffectLayer* layer = model->AddEffectLayer();
    std::map<int, ImageInfo> imageInfo;
    std::string imagePfx;
    std::vector<bool> reserved;
    std::string blend_string = "";
    if (layerBlend != "Normal") {
        blend_string = ",T_CHOICE_LayerMethod=" + layerBlend + ",";
    }
    for (wxXmlNode* e = input_root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if ("imageActions" == e->GetName()) {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if ("imageAction" == element->GetName()) {
                    int layer_index = wxAtoi(element->GetAttribute("layer"));
                    if (layer_index > 0)
                        layer_index--;
                    if (layer_index >= reserved.size()) {
                        reserved.resize(layer_index + 1, false);
                    }
                    reserved[layer_index] = true;
                }
            }
        } else if ("scenes" == e->GetName() || "images" == e->GetName()) {
            for (wxXmlNode* element = e->GetChildren(); (element != nullptr) && ("" == imagePfx); element = element->GetNext()) {
                if ("image" == element->GetName() || "scene" == element->GetName()) {
                    if ("" == imagePfx) {
                        wxFileDialog fd(this,
                                        "Choose location and base name for image files",
                                        showDirectory,
                                        wxEmptyString,
                                        wxFileSelectorDefaultWildcardStr,
                                        wxFD_SAVE);
                        if (fd.ShowModal() == wxID_CANCEL) {
                            return false;
                        }
                        imagePfx = fd.GetPath().ToStdString();
                    }
                }
            }
        } else if (e->GetName() == "layouts") {
            wxXmlNode* element = e->GetChildren();
            wxString attr;
            element->GetAttribute("visualizationMode", &attr);
            if (attr == "false") {
                element->GetAttribute("nbrOfRibbons", &attr);
                attr.ToDouble(&num_columns);
                num_rows = 50.0;
                element->GetAttribute("ribbonLength", &attr);
                if (attr == "half") {
                    num_rows /= 2.0;
                    num_columns *= 2.0;
                }
            } else {
                num_rows = (double)y_size;
                num_columns = (double)x_size;
            }
            element->GetAttribute("ribbonOrientation", &attr);
            if (attr == "horizontal") {
                reverse_xy = true;
                std::swap(num_columns, num_rows);
            } else {
                reverse_rows = true;
            }
            layout_defined = true;
        }
    }
    for (wxXmlNode* e = input_root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "morphs") {
            if (!layout_defined) {
                DisplayError("The layouts section was not found in the SuperStar file!", this);
                return false;
            }
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                wxString name_attr;
                wxString acceleration;
                wxString state1_time, state2_time, ramp_time_ext;
                element->GetAttribute("name", &name_attr);
                element->GetAttribute("acceleration", &acceleration);
                std::string attr = element->GetAttribute("layer").ToStdString();
                double layer_val = atof(attr.c_str());
                int layer_index = (int)layer_val;
                wxXmlNode* state1 = element->GetChildren();
                wxXmlNode* state2 = state1->GetNext();
                wxXmlNode* ramp = state2->GetNext();
                state1->GetAttribute("time", &state1_time);
                state2->GetAttribute("time", &state2_time);
                ramp->GetAttribute("timeExt", &ramp_time_ext);
                int start_time = wxAtoi(state1_time) * 10;
                int end_time = wxAtoi(state2_time) * 10;
                int ramp_time = wxAtoi(ramp_time_ext) * 10;
                end_time += ramp_time;
                double head_duration = (1.0 - (double)ramp_time / ((double)end_time - (double)start_time)) * 100.0;
                std::string settings = "E_CHECKBOX_Morph_End_Link=0,E_CHECKBOX_Morph_Start_Link=0,E_CHECKBOX_ShowHeadAtStart=0,E_NOTEBOOK_Morph=Start,E_SLIDER_MorphAccel=0,E_SLIDER_Morph_Repeat_Count=0,E_SLIDER_Morph_Repeat_Skip=1,E_SLIDER_Morph_Stagger=0";
                settings += acceleration + ",";
                wxString duration = wxString::Format("E_SLIDER_MorphDuration=%d,", (int)head_duration);
                settings += duration;
                attr = state2->GetAttribute("trailLen");
                settings += "E_SLIDER_MorphEndLength=" + attr + ",";
                attr = state1->GetAttribute("trailLen");
                settings += "E_SLIDER_MorphStartLength=" + attr + ",";
                if (!reverse_xy)
                    attr = state2->GetAttribute("x1");
                else
                    attr = state2->GetAttribute("y1");
                if (!CalcPercentage(attr, num_columns, false, x_offset))
                    continue;
                settings += "E_SLIDER_Morph_End_X1=" + attr + ",";
                if (!reverse_xy)
                    attr = state2->GetAttribute("x2");
                else
                    attr = state2->GetAttribute("y2");
                if (!CalcPercentage(attr, num_columns, false, x_offset))
                    continue;
                settings += "E_SLIDER_Morph_End_X2=" + attr + ",";
                if (!reverse_xy)
                    attr = state2->GetAttribute("y1");
                else
                    attr = state2->GetAttribute("x1");
                if (!CalcPercentage(attr, num_rows, reverse_rows, y_offset))
                    continue;
                settings += "E_SLIDER_Morph_End_Y1=" + attr + ",";
                if (!reverse_xy)
                    attr = state2->GetAttribute("y2");
                else
                    attr = state2->GetAttribute("x2");
                if (!CalcPercentage(attr, num_rows, reverse_rows, y_offset))
                    continue;
                settings += "E_SLIDER_Morph_End_Y2=" + attr + ",";
                if (!reverse_xy)
                    attr = state1->GetAttribute("x1");
                else
                    attr = state1->GetAttribute("y1");
                if (!CalcPercentage(attr, num_columns, false, x_offset))
                    continue;
                settings += "E_SLIDER_Morph_Start_X1=" + attr + ",";
                if (!reverse_xy)
                    attr = state1->GetAttribute("x2");
                else
                    attr = state1->GetAttribute("y2");
                if (!CalcPercentage(attr, num_columns, false, x_offset))
                    continue;
                settings += "E_SLIDER_Morph_Start_X2=" + attr + ",";
                if (!reverse_xy)
                    attr = state1->GetAttribute("y1");
                else
                    attr = state1->GetAttribute("x1");
                if (!CalcPercentage(attr, num_rows, reverse_rows, y_offset))
                    continue;
                settings += "E_SLIDER_Morph_Start_Y1=" + attr + ",";
                if (!reverse_xy)
                    attr = state1->GetAttribute("y2");
                else
                    attr = state1->GetAttribute("x2");
                if (!CalcPercentage(attr, num_rows, reverse_rows, y_offset))
                    continue;
                settings += "E_SLIDER_Morph_Start_Y2=" + attr + ",";
                std::string sRed = state1->GetAttribute("red").ToStdString();
                std::string sGreen = state1->GetAttribute("green").ToStdString();
                std::string sBlue = state1->GetAttribute("blue").ToStdString();
                std::string color = GetColorString(sRed, sGreen, sBlue).ToStdString();
                std::string palette = "C_BUTTON_Palette1=" + (std::string)color + ",";
                sRed = state2->GetAttribute("red");
                sGreen = state2->GetAttribute("green");
                sBlue = state2->GetAttribute("blue");
                color = GetColorString(sRed, sGreen, sBlue);
                palette += "C_BUTTON_Palette2=" + color + ",";
                sRed = ramp->GetAttribute("red1");
                sGreen = ramp->GetAttribute("green1");
                sBlue = ramp->GetAttribute("blue1");
                color = GetColorString(sRed, sGreen, sBlue);
                palette += "C_BUTTON_Palette3=" + color + ",";
                sRed = ramp->GetAttribute("red2");
                sGreen = ramp->GetAttribute("green2");
                sBlue = ramp->GetAttribute("blue2");
                color = GetColorString(sRed, sGreen, sBlue);
                palette += "C_BUTTON_Palette4=" + color + ",";
                palette += "C_BUTTON_Palette5=#FFFFFF,C_BUTTON_Palette6=#000000,C_CHECKBOX_Palette1=1,C_CHECKBOX_Palette2=1,C_CHECKBOX_Palette3=1,C_CHECKBOX_Palette4=1,";
                settings += blend_string;
                while (model->GetEffectLayerCount() < layer_index) {
                    model->AddEffectLayer();
                }
                layer = FindOpenLayer(model, layer_index, start_time, end_time, reserved);
                layer->AddEffect(0, "Morph", settings, palette, start_time, end_time, false, false);
            }
        } else if ("images" == e->GetName()) {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if ("image" == element->GetName()) {
                    for (wxXmlNode* i = element->GetChildren(); i != nullptr; i = i->GetNext()) {
                        if ("pixe" == i->GetName()) {
                            wxString data = i->GetAttribute("s");
                            int w = wxAtoi(element->GetAttribute("width"));
                            int h = wxAtoi(element->GetAttribute("height"));

                            int idx = wxAtoi(element->GetAttribute("savedIndex"));
                            int xOffset = wxAtoi(element->GetAttribute("xOffset"));
                            int yOffset = wxAtoi(element->GetAttribute("yOffset"));

                            unsigned char* bytes = (unsigned char*)malloc(w * h * 3);
                            unsigned char* alpha = (unsigned char*)malloc(w * h);
                            int cnt = 0;
                            int p = 0;
                            wxStringTokenizer tokenizer(data, ",");
                            while (tokenizer.HasMoreTokens()) {
                                unsigned int ii = wxAtoi(tokenizer.GetNextToken());
                                unsigned int v = (ii >> 16) & 0xff;
                                v *= 255;
                                v /= 100;
                                bytes[cnt] = v;
                                v = (ii >> 8) & 0xff;
                                v *= 255;
                                v /= 100;
                                bytes[cnt + 1] = v;
                                v = ii & 0xff;
                                v *= 255;
                                v /= 100;
                                bytes[cnt + 2] = v;

                                alpha[p] = wxALPHA_OPAQUE;
                                if (ii == 0) {
                                    alpha[p] = wxALPHA_TRANSPARENT;
                                }
                                p++;
                                cnt += 3;
                            }

                            wxImage image(w, h, bytes, alpha);
                            if ("" == imagePfx) {
                                wxFileDialog fd(this,
                                                "Choose location and base name for image files",
                                                showDirectory,
                                                wxEmptyString,
                                                wxFileSelectorDefaultWildcardStr,
                                                wxFD_SAVE);
                                while (fd.ShowModal() == wxID_CANCEL || fd.GetFilename() == "") {
                                }
                                imagePfx = fd.GetPath();
                            }
                            std::string fname = imagePfx + "_" + wxString::Format("%d.png", idx).ToStdString();
                            if (reverse_xy) {
                                image = image.Rotate90(false);
                                imageInfo[idx].Set(yOffset, xOffset, h, w, fname);
                            } else {
                                imageInfo[idx].Set(xOffset, yOffset, w, h, fname);
                            }
                            ScaleImage(image, imageResizeType, modelSize, num_columns, num_rows, imageInfo[idx], reverse_xy);
                            image.SaveFile(fname);
                        }
                    }
                }
            }
        } else if ("flowys" == e->GetName()) {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if ("flowy" == element->GetName()) {
                    std::string centerX, centerY;
                    int startms = wxAtoi(element->GetAttribute("startTime")) * 10;
                    int endms = wxAtoi(element->GetAttribute("endTime")) * 10;
                    wxString type = element->GetAttribute("flowyType");
                    wxString color_string = element->GetAttribute("Colors");
                    std::string color;
                    std::string palette = "C_BUTTON_Palette1=" + (std::string)color + ",";
                    int cnt = 1;
                    wxStringTokenizer tokenizer(color_string, " ");
                    while (tokenizer.HasMoreTokens() && cnt <= 6) {
                        wxStringTokenizer tokenizer2(tokenizer.GetNextToken(), ",");
                        std::string sRed = tokenizer2.GetNextToken().ToStdString();
                        std::string sGreen = tokenizer2.GetNextToken().ToStdString();
                        std::string sBlue = tokenizer2.GetNextToken().ToStdString();
                        color = GetColorString(sRed, sGreen, sBlue);
                        if (cnt > 1) {
                            palette += ",";
                        }
                        palette += "C_BUTTON_Palette" + wxString::Format("%d", cnt) + "=" + color;
                        palette += ",C_CHECKBOX_Palette" + wxString::Format("%d", cnt) + "=1";
                        cnt++;
                    }
                    while (cnt <= 6) {
                        if (cnt > 1) {
                            palette += ",";
                        }
                        palette += "C_BUTTON_Palette" + wxString::Format("%d", cnt) + "=#000000";
                        palette += ",C_CHECKBOX_Palette" + wxString::Format("%d", cnt) + "=0";
                        cnt++;
                    }

                    int layer_index = wxAtoi(element->GetAttribute("layer"));
                    int acceleration = wxAtoi(element->GetAttribute("acceleration"));
                    if (!reverse_xy)
                        centerX = element->GetAttribute("centerX").ToStdString();
                    else
                        centerX = element->GetAttribute("centerY").ToStdString();
                    if (!CalcPercentage(centerX, num_columns, false, x_offset))
                        continue;
                    if (!reverse_xy)
                        centerY = element->GetAttribute("centerY").ToStdString();
                    else
                        centerY = element->GetAttribute("centerX").ToStdString();
                    if (!CalcPercentage(centerY, num_rows, reverse_rows, y_offset))
                        continue;
                    int startAngle = wxAtoi(element->GetAttribute("startAngle"));
                    int endAngle = wxAtoi(element->GetAttribute("endAngle"));
                    if (reverse_xy) {
                        startAngle -= 90;
                        endAngle -= 90;
                        if (startAngle < 0) {
                            startAngle += 360;
                        }
                        if (endAngle < 0) {
                            endAngle += 360;
                        }
                    }
                    int revolutions = std::abs(endAngle - startAngle);
                    if (revolutions == 0)
                        revolutions = 3; // algorithm needs non-zero value until we figure out better way to draw effect
                    int startRadius = wxAtoi(element->GetAttribute("startRadius"));
                    int endRadius = wxAtoi(element->GetAttribute("endRadius"));
                    if (type == "Spiral") {
                        int tailms = wxAtoi(element->GetAttribute("tailTimeLength")) * 10;
                        endms += tailms;
                        double duration = (1.0 - (double)tailms / ((double)endms - (double)startms)) * 100.0;
                        int startWidth = wxAtoi(element->GetAttribute("startDotSize"));
                        int endWidth = wxAtoi(element->GetAttribute("endDotSize"));
                        std::string settings = "E_CHECKBOX_Galaxy_Reverse=" + wxString::Format("%d", endAngle < startAngle).ToStdString() + ",E_CHECKBOX_Galaxy_Blend_Edges=1" + ",E_CHECKBOX_Galaxy_Inward=1" + ",E_NOTEBOOK_Galaxy=Start,E_SLIDER_Galaxy_Accel=" + wxString::Format("%d", acceleration).ToStdString() + ",E_SLIDER_Galaxy_CenterX=" + centerX + ",E_SLIDER_Galaxy_CenterY=" + centerY + ",E_SLIDER_Galaxy_Duration=" + wxString::Format("%d", (int)duration).ToStdString() + ",E_SLIDER_Galaxy_End_Radius=" + wxString::Format("%d", endRadius).ToStdString() + ",E_SLIDER_Galaxy_End_Width=" + wxString::Format("%d", endWidth).ToStdString() + ",E_SLIDER_Galaxy_Revolutions=" + wxString::Format("%d", revolutions).ToStdString() + ",E_SLIDER_Galaxy_Start_Angle=" + wxString::Format("%d", startAngle).ToStdString() + ",E_SLIDER_Galaxy_Start_Radius=" + wxString::Format("%d", startRadius).ToStdString() + ",E_SLIDER_Galaxy_Start_Width=" + wxString::Format("%d", startWidth).ToStdString() + blend_string;

                        layer = FindOpenLayer(model, layer_index, startms, endms, reserved);
                        layer->AddEffect(0, "Galaxy", settings, palette, startms, endms, false, false);
                    } else if (type == "Shockwave") {
                        int startWidth = wxAtoi(element->GetAttribute("headWidth"));
                        int endWidth = wxAtoi(element->GetAttribute("tailWidth"));
                        std::string settings = "E_CHECKBOX_Shockwave_Blend_Edges=1,E_NOTEBOOK_Shockwave=Position,E_SLIDER_Shockwave_Accel=" + wxString::Format("%d", acceleration).ToStdString() + ",E_SLIDER_Shockwave_CenterX=" + centerX + ",E_SLIDER_Shockwave_CenterY=" + centerY + ",E_SLIDER_Shockwave_End_Radius=" + wxString::Format("%d", endRadius).ToStdString() + ",E_SLIDER_Shockwave_End_Width=" + wxString::Format("%d", endWidth).ToStdString() + ",E_SLIDER_Shockwave_Start_Radius=" + wxString::Format("%d", startRadius).ToStdString() + ",E_SLIDER_Shockwave_Start_Width=" + wxString::Format("%d", startWidth).ToStdString() + blend_string;
                        layer = FindOpenLayer(model, layer_index, startms, endms, reserved);
                        layer->AddEffect(0, "Shockwave", settings, palette, startms, endms, false, false);
                    } else if (type == "Fan") {
                        int revolutionsPerSecond = wxAtoi(element->GetAttribute("revolutionsPerSecond"));
                        int blades = wxAtoi(element->GetAttribute("blades"));
                        int blade_width = wxAtoi(element->GetAttribute("width"));
                        int elementAngle = wxAtoi(element->GetAttribute("elementAngle"));
                        int elementStepAngle = wxAtoi(element->GetAttribute("elementStepAngle"));
                        int numElements = elementAngle / elementStepAngle;
                        numElements = std::max(1, numElements);
                        numElements = std::min(numElements, 4);
                        blades = std::max(1, blades);
                        blades = std::min(blades, 16);
                        int tailms = int(double(endms - startms) * (blade_width * 2.0) / 100.0) + 35;
                        endms += tailms;
                        int duration = 100 - int(tailms * 100.0 / (endms - startms));
                        std::string settings = "E_CHECKBOX_Fan_Reverse=" + wxString::Format("%d", startAngle > endAngle).ToStdString() + ",E_CHECKBOX_Fan_Blend_Edges=1" + ",E_NOTEBOOK_Fan=Position,E_SLIDER_Fan_Accel=" + wxString::Format("%d", acceleration).ToStdString() + ",E_SLIDER_Fan_Blade_Angle=" + wxString::Format("%d", elementAngle).ToStdString() + ",E_SLIDER_Fan_Blade_Width=" + wxString::Format("%d", blade_width).ToStdString() + ",E_SLIDER_Fan_CenterX=" + centerX + ",E_SLIDER_Fan_CenterY=" + centerY + ",E_SLIDER_Fan_Duration=" + wxString::Format("%d", duration).ToStdString() + ",E_SLIDER_Fan_Element_Width=" + wxString::Format("%d", 100).ToStdString() + ",E_SLIDER_Fan_Num_Blades=" + wxString::Format("%d", blades).ToStdString() + ",E_SLIDER_Fan_Num_Elements=" + wxString::Format("%d", numElements).ToStdString() + ",E_SLIDER_Fan_End_Radius=" + wxString::Format("%d", endRadius).ToStdString() + ",E_SLIDER_Fan_Revolutions=" + wxString::Format("%d", (int)((double)revolutionsPerSecond * ((double)(endms - startms) / 1000.0) * 3.6)).ToStdString() + ",E_SLIDER_Fan_Start_Angle=" + wxString::Format("%d", startAngle).ToStdString() + ",E_SLIDER_Fan_Start_Radius=" + wxString::Format("%d", startRadius).ToStdString() + blend_string;
                        layer = FindOpenLayer(model, layer_index, startms, endms, reserved);
                        layer->AddEffect(0, "Fan", settings, palette, startms, endms, false, false);
                    }
                }
            }
        } else if ("scenes" == e->GetName()) {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if ("scene" == element->GetName()) {
                    wxString startms = element->GetAttribute("startCentisecond") + "0";
                    wxString endms = element->GetAttribute("endCentisecond") + "0";
                    wxString type = element->GetAttribute("type");
                    int layer_index = wxAtoi(element->GetAttribute("layer"));
                    xlColor startc = GetColor(element->GetAttribute("red1").ToStdString(),
                                              element->GetAttribute("green1").ToStdString(),
                                              element->GetAttribute("blue1").ToStdString());
                    xlColor endc = GetColor(element->GetAttribute("red2").ToStdString(),
                                            element->GetAttribute("green2").ToStdString(),
                                            element->GetAttribute("blue2").ToStdString());
                    while (model->GetEffectLayerCount() < layer_index) {
                        model->AddEffectLayer();
                    }

                    int start_time = wxAtoi(startms);
                    int end_time = wxAtoi(endms);
                    layer = FindOpenLayer(model, layer_index, start_time, end_time, reserved);
                    if ("" == imagePfx) {
                        wxFileDialog fd(this,
                                        "Choose location and base name for image files",
                                        showDirectory,
                                        wxEmptyString,
                                        wxFileSelectorDefaultWildcardStr,
                                        wxFD_SAVE);
                        while (fd.ShowModal() == wxID_CANCEL || fd.GetFilename() == "") {
                        }
                        imagePfx = fd.GetPath();
                    }

                    std::string ru = "0.0";
                    std::string rd = "0.0";
                    std::string imageName;
                    bool isFull = false;
                    wxRect rect;

                    bool isPartOfModel = IsPartOfModel(element, num_rows, num_columns, isFull, rect, reverse_xy);

                    if (isPartOfModel && isFull) {
                        // Every pixel in the model is specified, we can use a color wash or on instead of images
                        std::string palette = "C_BUTTON_Palette1=" + (std::string)startc + ",C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=" + (std::string)endc + ",C_CHECKBOX_Palette2=1,";

                        std::string settings = blend_string;
                        if (startc == endc) {
                            layer->AddEffect(0, "On", settings, palette, start_time, end_time, false, false);
                        } else if (startc == xlBLACK) {
                            std::string palette1 = "C_BUTTON_Palette1=" + (std::string)endc + ",C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=" + (std::string)startc +
                                                   ",C_CHECKBOX_Palette2=1";
                            settings += ",E_TEXTCTRL_Eff_On_Start=0";
                            layer->AddEffect(0, "On", settings, palette1, start_time, end_time, false, false);
                        } else if (endc == xlBLACK) {
                            settings += ",E_TEXTCTRL_Eff_On_End=0";
                            layer->AddEffect(0, "On", "E_TEXTCTRL_Eff_On_End=0", palette, start_time, end_time, false, false);
                        } else {
                            layer->AddEffect(0, "Color Wash", settings, palette, start_time, end_time, false, false);
                        }
                    } else if (isPartOfModel && rect.x != -1) {
                        // forms a simple rectangle, we can use a ColorWash affect for this with a partial rectangle
                        std::string palette = "C_BUTTON_Palette1=" + (std::string)startc + ",C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=" + (std::string)endc + ",C_CHECKBOX_Palette2=1";

                        std::string settings = "B_CUSTOM_SubBuffer=";
                        std::string val = wxString::Format("%d", rect.x).ToStdString();
                        if (!CalcBoundedPercentage(val, num_columns, false, x_offset))
                            continue;
                        settings += val;
                        settings += "x";
                        val = wxString::Format("%d", rect.y);
                        if (!CalcBoundedPercentage(val, num_rows, reverse_rows ^ reverse_xy, y_offset))
                            continue;
                        settings += val;
                        settings += "x";
                        val = wxString::Format("%d", rect.width);
                        if (!CalcBoundedPercentage(val, num_columns, false, x_offset))
                            continue;
                        settings += val;
                        settings += "x";
                        val = wxString::Format("%d", rect.height);
                        if (!CalcBoundedPercentage(val, num_rows, reverse_rows ^ reverse_xy, y_offset))
                            continue;
                        settings += val;
                        settings += blend_string;

                        layer->AddEffect(0, "Color Wash", settings, palette, start_time, end_time, false, false);
                    } else if (isPartOfModel) {
                        if (startc == xlBLACK || endc == xlBLACK || endc == startc) {
                            imageName = CreateSceneImage(imagePfx, "", element, num_columns, num_rows, false, reverse_xy,
                                                         (startc == xlBLACK) ? endc : startc, y_offset,
                                                         imageResizeType, modelSize);
                            wxString ramp = wxString::Format("%lf", (double)(end_time - start_time) / 1000.0);
                            if (endc == xlBLACK) {
                                rd = ramp;
                            }
                            if (startc == xlBLACK) {
                                ru = ramp;
                            }
                        } else {
                            int time = wxAtoi(endms) - wxAtoi(startms);
                            int numFrames = time / _seqData.FrameTime();
                            xlColor color;
                            for (int x = 0; x < numFrames; x++) {
                                double ratio = x;
                                ratio /= numFrames;
                                color.Set(ChannelBlend(startc.Red(), endc.Red(), ratio),
                                          ChannelBlend(startc.Green(), endc.Green(), ratio),
                                          ChannelBlend(startc.Blue(), endc.Blue(), ratio));
                                wxString s = CreateSceneImage(imagePfx, wxString::Format("-%d", x + 1).ToStdString(),
                                                              element,
                                                              num_columns, num_rows, false, reverse_xy,
                                                              color, y_offset,
                                                              imageResizeType, modelSize);
                                if (x == 0) {
                                    imageName = s;
                                }
                            }
                        }

                        std::string settings = "E_CHECKBOX_Pictures_WrapX=0,E_CHOICE_Pictures_Direction=none,"
                                               "E_SLIDER_PicturesXC=0"
                                               ",E_SLIDER_PicturesYC=0"
                                               ",E_CHOICE_Scaling=Scale To Fit"
                                               ",E_SLIDER_Pictures_StartScale=100"
                                               ",E_SLIDER_Pictures_EndScale=100"
                                               ",E_CHECKBOX_Pictures_PixelOffsets=1"
                                               ",E_TEXTCTRL_Pictures_Speed=1.0"
                                               ",E_TEXTCTRL_Pictures_FrameRateAdj=1.0"
                                               ",E_TEXTCTRL_Pictures_Filename=" +
                                               imageName;
                        if (type == "shimmer") {
                            settings += ",E_CHECKBOX_Pictures_Shimmer=1";
                        }
                        settings += blend_string;
                        if (ru != "0.0") {
                            settings += ",T_TEXTCTRL_Fadein=" + ru;
                        }
                        if (rd != "0.0") {
                            settings += ",T_TEXTCTRL_Fadeout=" + rd;
                        }
                        layer->AddEffect(0, "Pictures", settings, "", start_time, end_time, false, false);
                    }
                }
            }
        } else if ("textActions" == e->GetName()) {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if ("textAction" == element->GetName()) {
                    wxString startms = element->GetAttribute("startCentisecond") + "0";
                    wxString endms = element->GetAttribute("endCentisecond") + "0";
                    std::string text = element->GetAttribute("text").ToStdString();
                    wxString fontName = element->GetAttribute("fontName");
                    int fontSize = wxAtoi(element->GetAttribute("fontCapsHeight", "6"));
                    int fontCellWidth = wxAtoi(element->GetAttribute("fontCellWidth", "6"));
                    int fontCellHeight = wxAtoi(element->GetAttribute("fontCellHeight", "6"));
                    wxString colorType = element->GetAttribute("colorType", "chooseColor");
                    int fCI = wxAtoi(element->GetAttribute("firstColorIndex", "0"));
                    wxString mask = element->GetAttribute("maskType");
                    bool use_xl_font = true;
                    wxString xl_font_name = wxString::Format("%d-%dx%d %s", fontSize, fontCellWidth, fontCellHeight, fontName);
                    xl_font_name.Replace('_', ' ');
                    xlFont* xl_font = FontManager::get_font(xl_font_name);
                    if (xl_font == nullptr) {
                        xl_font_name = "Use OS Fonts";
                        use_xl_font = false;
                    }

                    // SuperStar fonts are not as wide as they are listed.  This gets us closer to reality.
                    fontCellWidth = (fontCellWidth * 2) / 3;
                    if (!use_xl_font) {
                        fontSize += 4;
                    }

                    int rotation = wxAtoi(element->GetAttribute("rotation", "90"));
                    if (reverse_xy) {
                        rotation -= 90;
                    }
                    // int direction = wxAtoi(element->GetAttribute("direction", "0"));
                    int xStart = wxAtoi(element->GetAttribute("xStart", "0"));
                    int yStart = wxAtoi(element->GetAttribute("yStart", "0"));
                    int xEnd = wxAtoi(element->GetAttribute("xEnd", "0"));
                    int yEnd = wxAtoi(element->GetAttribute("yEnd", "0"));
                    if (reverse_xy) {
                        std::swap(xStart, yStart);
                        std::swap(xEnd, yEnd);
                    }

                    xlColor color = GetColor(element->GetAttribute("red").ToStdString(),
                                             element->GetAttribute("green").ToStdString(),
                                             element->GetAttribute("blue").ToStdString());

                    int layer_index = wxAtoi(element->GetAttribute("layer"));
                    while (model->GetEffectLayerCount() < layer_index) {
                        model->AddEffectLayer();
                    }
                    int start_time = wxAtoi(startms);
                    int end_time = wxAtoi(endms);
                    layer = FindOpenLayer(model, layer_index, start_time, end_time, reserved);
                    int lorWidth = text.size() * fontCellWidth;
                    int lorHeight = fontSize;

                    if (use_xl_font) {
                        wxString xl_text(text);
                        lorWidth = FontManager::get_length(xl_font, xl_text) - 2;
                    }

                    std::string font = "arial " + wxString::Format("%s%d", (fontName.Contains("Bold") ? "bold " : ""), fontSize).ToStdString();
                    std::string eff = "normal";

                    if (use_xl_font) {
                        if (rotation == 90) {
                            eff = "rotate down 90";
                            lorHeight = lorWidth;
                            lorWidth = fontSize - 2;
                        } else if (rotation == -90 || rotation == 270) {
                            eff = "rotate up 90";
                            lorHeight = lorWidth - 2;
                            lorWidth = fontSize - 2;
                        }
                        if (fontName.Contains("Vertical")) {
                            lorHeight += 4;
                        }
                    } else {
                        if (fontName.Contains("Vertical")) {
                            eff = "vert text down";
                            lorWidth = fontCellWidth;
                            lorHeight = text.size() * fontSize;
                        } else if (rotation == 90) {
                            eff = "rotate down 90";
                            lorWidth = fontSize;
                            lorHeight = text.size() * fontCellWidth;
                        } else if (rotation == 270 || rotation == -90) {
                            eff = "rotate up 90";
                            lorWidth = fontSize;
                            lorHeight = text.size() * fontCellWidth;
                        }
                    }

                    // calculate everything off the LOR center
                    xStart += round((double)lorWidth / 2.0);
                    xEnd += round((double)lorWidth / 2.0);
                    yStart += round((double)lorHeight / 2.0);
                    yEnd += round((double)lorHeight / 2.0);

                    yStart = -yStart + num_rows;
                    yEnd = -yEnd + num_rows;

                    xStart = CalcUnBoundedPercentage(xStart, num_columns) - 50;
                    xEnd = CalcUnBoundedPercentage(xEnd, num_columns) - 50;
                    yStart = CalcUnBoundedPercentage(yStart, num_rows) - 50;
                    yEnd = CalcUnBoundedPercentage(yEnd, num_rows) - 50;

                    std::string palette = "C_BUTTON_Palette1=" + (std::string)color + ",C_CHECKBOX_Palette1=1" + ",C_CHECKBOX_Palette2=0,C_CHECKBOX_Palette3=0,C_CHECKBOX_Palette4=0,C_CHECKBOX_Palette5=0" + ",C_CHECKBOX_Palette6=0,C_CHECKBOX_Palette7=0,C_CHECKBOX_Palette8=0";
                    if (use_xl_font) {
                        if (colorType == "rainbow") {
                            const wxString colors[] = { wxT("#FF0000"), wxT("#FF8000"), wxT("#FFFF00"), wxT("#00FF00"), wxT("#0000FF"), wxT("#8000FF") };
                            palette = "C_BUTTON_Palette1=" + std::string(colors[fCI].mb_str());
                            palette += ",C_BUTTON_Palette2=" + std::string(colors[(fCI + 1) % 6].mb_str());
                            palette += ",C_BUTTON_Palette3=" + std::string(colors[(fCI + 2) % 6].mb_str());
                            palette += ",C_BUTTON_Palette4=" + std::string(colors[(fCI + 3) % 6].mb_str());
                            palette += ",C_BUTTON_Palette5=" + std::string(colors[(fCI + 4) % 6].mb_str());
                            palette += ",C_BUTTON_Palette6=" + std::string(colors[(fCI + 5) % 6].mb_str());
                            palette += ",C_CHECKBOX_Palette1=1,C_CHECKBOX_Palette2=1,C_CHECKBOX_Palette3=1,C_CHECKBOX_Palette4=1";
                            palette += ",C_CHECKBOX_Palette5=1,C_CHECKBOX_Palette6=1,C_CHECKBOX_Palette7=0,C_CHECKBOX_Palette8=0";
                        } else if (colorType == "redGreenBlue") {
                            const wxString colors[] = { wxT("#FF0000"), wxT("#00FF00"), wxT("#0000FF") };
                            palette = "C_BUTTON_Palette1=" + std::string(colors[fCI].mb_str());
                            palette += ",C_BUTTON_Palette2=" + std::string(colors[(fCI + 1) % 3].mb_str());
                            palette += ",C_BUTTON_Palette3=" + std::string(colors[(fCI + 2) % 3].mb_str());
                            palette += ",C_CHECKBOX_Palette1=1,C_CHECKBOX_Palette2=1,C_CHECKBOX_Palette3=1,C_CHECKBOX_Palette4=0";
                            palette += ",C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,C_CHECKBOX_Palette7=0,C_CHECKBOX_Palette8=0";
                        }
                    }
                    std::string settings =
                        "E_CHECKBOX_TextToCenter=0,E_TEXTCTRL_Text=" + text + ",E_TEXTCTRL_Text_Speed=26," + "E_CHOICE_Text_Count=none," + "E_CHOICE_Text_Dir=vector,E_CHECKBOX_Text_PixelOffsets=0," + "E_CHOICE_Text_Effect=" + eff + "," + "E_FONTPICKER_Text_Font=" + font + "," + "E_CHOICE_Text_Font=" + xl_font_name.ToStdString() + "," + "E_SLIDER_Text_XStart=" + wxString::Format("%d", xStart).ToStdString() + "," + "E_SLIDER_Text_YStart=" + wxString::Format("%d", yStart).ToStdString() + "," + "E_SLIDER_Text_XEnd=" + wxString::Format("%d", xEnd).ToStdString() + "," + "E_SLIDER_Text_YEnd=" + wxString::Format("%d", yEnd).ToStdString();
                    if (mask == "positiveMask") {
                        if (blend_string != "") {
                            settings += blend_string;
                        }
                    } else if (mask == "negativeMask") {
                        settings += ",T_CHOICE_LayerMethod=1 is Mask";
                    } else {
                        settings += blend_string;
                    }

                    layer->AddEffect(0, "Text", settings, palette, start_time, end_time, false, false);
                }
            }

        } else if ("imageActions" == e->GetName()) {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if ("imageAction" == element->GetName()) {
                    //<imageAction name="Image Action 14" colorType="nativeColor" maskType="normal" rotation="0" direction="8"
                    //  stopAtEdge="0" layer="3" xStart="-1" yStart="0" xEnd="0" yEnd="0" startCentisecond="115" endCentisecond="145"
                    //  preRampTime="0" rampTime="0" fadeToBright="0" fadeFromBright="0" imageIndex="5" savedIndex="0">

                    wxString name = element->GetAttribute("name");
                    int idx = wxAtoi(element->GetAttribute("imageIndex"));
                    int startms = wxAtoi(element->GetAttribute("startCentisecond")) * 10;
                    int endms = wxAtoi(element->GetAttribute("endCentisecond")) * 10;
                    int layer_index = wxAtoi(element->GetAttribute("layer"));
                    int rampDownTime = wxAtoi(element->GetAttribute("rampTime")) * 10;
                    int rampUpTime = wxAtoi(element->GetAttribute("preRampTime")) * 10;
                    while (model->GetEffectLayerCount() <= layer_index) {
                        model->AddEffectLayer();
                    }
                    std::string rampUpTimeString = "0";
                    if (rampUpTime) {
                        double fadeIn = rampUpTime;
                        fadeIn /= 1000; // FadeIn is in seconds
                        rampUpTimeString = wxString::Format("%lf", fadeIn);
                    }
                    std::string rampDownTimeString = "0";
                    if (rampDownTime) {
                        double fade = rampDownTime;
                        fade /= 1000; // FadeIn is in seconds
                        rampDownTimeString = wxString::Format("%lf", fade);
                    }

                    int startx = wxAtoi(element->GetAttribute("xStart"));
                    int starty = wxAtoi(element->GetAttribute("yStart"));
                    int endx = wxAtoi(element->GetAttribute("xEnd"));
                    int endy = wxAtoi(element->GetAttribute("yEnd"));
                    if (reverse_xy) {
                        std::swap(startx, starty);
                        std::swap(endx, endy);
                        starty = -starty;
                        endy = -endy;
                    }
                    ImageInfo& imgInfo = imageInfo[idx];
                    int x = imgInfo.xOffset;
                    int y = imgInfo.yOffset;

                    layer = FindOpenLayer(model, layer_index, startms, endms, reserved);
                    if (endy == starty && endx == startx) {
                        x += round((double)startx * imgInfo.scaleX);
                        y -= round((double)starty * imgInfo.scaleY);
                        std::string settings = "E_CHECKBOX_Pictures_WrapX=0,E_CHOICE_Pictures_Direction=none,"
                                               "E_SLIDER_PicturesXC=" +
                                               wxString::Format("%d", x).ToStdString() + ",E_SLIDER_PicturesYC=" + wxString::Format("%d", y).ToStdString() + ",E_CHOICE_Scaling=No Scaling" + ",X_Effect_Description=" + name + ",E_SLIDER_Pictures_StartScale=100" + ",E_SLIDER_Pictures_EndScale=100" + ",E_CHECKBOX_Pictures_PixelOffsets=1" + ",E_TEXTCTRL_Pictures_Filename=" + imgInfo.imageName + ",E_TEXTCTRL_Pictures_Speed=1.0" + ",E_TEXTCTRL_Pictures_FrameRateAdj=1.0";

                        if ("0" != rampUpTimeString) {
                            settings += ",T_TEXTCTRL_Fadein=" + rampUpTimeString;
                        }
                        if ("0" != rampDownTimeString) {
                            settings += ",T_TEXTCTRL_Fadeout=" + rampDownTimeString;
                        }
                        settings += blend_string;

                        layer->AddEffect(0, "Pictures", settings, "", startms, endms, false, false);
                    } else {
                        std::string settings = "E_CHECKBOX_Pictures_WrapX=0,E_CHOICE_Pictures_Direction=vector,"
                                               "E_SLIDER_PicturesXC=" +
                                               wxString::Format("%d", x + (int)round((double)startx * imgInfo.scaleX)).ToStdString() + ",E_SLIDER_PicturesYC=" + wxString::Format("%d", y - (int)round((double)starty * imgInfo.scaleY)).ToStdString() + ",E_SLIDER_PicturesEndXC=" + wxString::Format("%d", x + (int)round((double)endx * imgInfo.scaleX)).ToStdString() + ",E_SLIDER_PicturesEndYC=" + wxString::Format("%d", y - (int)round((double)endy * imgInfo.scaleY)).ToStdString() + ",E_CHOICE_Scaling=No Scaling" + ",X_Effect_Description=" + name + ",E_SLIDER_Pictures_StartScale=100" + ",E_SLIDER_Pictures_EndScale=100" + ",E_TEXTCTRL_Pictures_Speed=1.0" + ",E_TEXTCTRL_Pictures_FrameRateAdj=1.0" + ",E_CHECKBOX_Pictures_PixelOffsets=1" + ",E_TEXTCTRL_Pictures_Filename=" + imgInfo.imageName;

                        if ("0" != rampUpTimeString) {
                            settings += ",T_TEXTCTRL_Fadein=" + rampUpTimeString;
                        }
                        if ("0" != rampDownTimeString) {
                            settings += ",T_TEXTCTRL_Fadeout=" + rampDownTimeString;
                        }
                        settings += blend_string;

                        layer->AddEffect(0, "Pictures", settings, "", startms, endms, false, false);
                    }
                }
            }
        }
    }
    return true;
}

void AddLSPEffect(EffectLayer* layer, int pos, int epos, int in, int out, int eff, const wxColor& c, int bst, int ben)
{
    if (eff == 4) {
        // off
        return;
    }
    xlColor color(c);
    xlColor color2(xlBLACK);

    bool isShimmer = eff == 5 || eff == 6;
    std::string effect = "On";
    std::string settings = wxString::Format("E_TEXTCTRL_Eff_On_End=%d,E_TEXTCTRL_Eff_On_Start=%d", out, in).ToStdString();
    if (isShimmer) {
        settings += ",E_CHECKBOX_On_Shimmer=1";
    }

    if (bst != 0 && ben != 0) {
        color = xlColor(bst & 0xFFFFFF, false);
        color2 = xlColor(ben & 0xFFFFFF, false);
        if (color == color2) {
            color2 = xlBLACK;
        } else {
            effect = "Color Wash";
            settings = isShimmer ? "E_CHECKBOX_ColorWash_Shimmer=1" : "";
        }
    }

    if (xlBLACK == color && xlBLACK == color2) {
        // nutcracker or other effects imported into LSP generate "BLACK" effects in the sequence.  Don't import them.
        return;
    }

    std::string palette = "C_BUTTON_Palette1=" + (std::string)color + ",C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=" + (std::string)color2 + ",C_CHECKBOX_Palette2=1";

    int start_time = (int)(pos * 50.0 / 4410.0);
    int end_time = (int)((epos - 1) * 50.0 / 4410.0);
    layer->AddEffect(0, effect, settings, palette, start_time, end_time, false, false);
}

void MapLSPEffects(EffectLayer* layer, wxXmlNode* node, const wxColor& c)
{
    if (node == nullptr) {
        return;
    }
    int eff = -1;
    int in = 1, out = 1, pos = 1;

    int bst = 0, ben = 0;

    for (wxXmlNode* cnd = node->GetChildren(); cnd != nullptr; cnd = cnd->GetNext()) {
        if (cnd->GetName() == "Tracks") {
            for (wxXmlNode* cnnd = cnd->GetChildren(); cnnd != nullptr; cnnd = cnnd->GetNext()) {
                if (cnnd->GetName() == "Track") {
                    for (wxXmlNode* ind = cnnd->GetChildren(); ind != nullptr; ind = ind->GetNext()) {
                        if (ind->GetName() == "Intervals") {
                            for (wxXmlNode* ti = ind->GetChildren(); ti != nullptr; ti = ti->GetNext()) {
                                if (ti->GetName() == "TimeInterval") {
                                    int neff = wxAtoi(ti->GetAttribute("eff", "4"));
                                    if (eff != -1 && neff != 7) {
                                        int npos = wxAtoi(ti->GetAttribute("pos", "1"));
                                        AddLSPEffect(layer, pos, npos, in, out, eff, c, bst, ben);
                                    }
                                    if (neff != 7) {
                                        pos = wxAtoi(ti->GetAttribute("pos", "1"));
                                        eff = neff;
                                        in = wxAtoi(ti->GetAttribute("in", "1"));
                                        out = wxAtoi(ti->GetAttribute("out", "1"));
                                        bst = wxAtoi(ti->GetAttribute("bst", "0"));
                                        ben = wxAtoi(ti->GetAttribute("ben", "0"));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void MapLSPStrand(StrandElement* layer, wxXmlNode* node, const wxColor& c)
{
    int nodeNum = 0;
    for (wxXmlNode* nd = node->GetChildren(); nd != nullptr; nd = nd->GetNext()) {
        if (nd->GetName() == "Channels") {
            for (wxXmlNode* cnd = nd->GetChildren(); cnd != nullptr; cnd = cnd->GetNext()) {
                if (cnd->GetName() == "Channel") {
                    EffectLayer* el = layer->GetNodeLayer(nodeNum, true);
                    MapLSPEffects(el, cnd, c);
                    nodeNum++;
                    if (nodeNum >= layer->GetNodeLayerCount()) {
                        return;
                    }
                }
            }
        }
    }
}

void xLightsFrame::ImportLSP(const wxFileName& filename)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxStopWatch sw; // start a stopwatch timer

    LMSImportChannelMapDialog dlg(this, filename);
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;

    wxFileName msq_file(filename);
    wxString msq_doc = msq_file.GetFullPath();
    wxFileInputStream fin(msq_doc);
    wxZipInputStream zin(fin);
    wxZipEntry* ent = zin.GetNextEntry();

    wxXmlDocument seq_xml;
    std::map<wxString, wxXmlDocument> cont_xml;
    std::map<wxString, wxXmlNode*> nodes;
    std::map<wxString, wxXmlNode*> strandNodes;

    while (ent != nullptr) {
        if (ent->GetName() == "Sequence") {
            seq_xml.Load(zin);
        } else {
            std::string id("1");
            wxXmlDocument& doc = cont_xml[ent->GetName()];

            if (doc.Load(zin)) {
                for (wxXmlNode* nd = doc.GetRoot()->GetChildren(); nd != nullptr; nd = nd->GetNext()) {
                    if (nd->GetName() == "ControllerName") {
                        id = nd->GetChildren()->GetContent();
                    }
                }
                strandNodes[id] = doc.GetRoot();
                dlg.ccrNames.push_back(id);
                for (wxXmlNode* nd = doc.GetRoot()->GetChildren(); nd != nullptr; nd = nd->GetNext()) {
                    if (nd->GetName() == "Channels") {
                        for (wxXmlNode* cnd = nd->GetChildren(); cnd != nullptr; cnd = cnd->GetNext()) {
                            if (cnd->GetName() == "Channel") {
                                std::string cname;
                                for (wxXmlNode* cnnd = cnd->GetChildren(); cnnd != nullptr; cnnd = cnnd->GetNext()) {
                                    if (cnnd->GetName() == "Tracks") {
                                        for (wxXmlNode* tnd = cnnd->GetChildren(); tnd != nullptr; tnd = tnd->GetNext()) {
                                            if (tnd->GetName() == "Track") {
                                                for (wxXmlNode* tnd2 = tnd->GetChildren(); tnd2 != nullptr; tnd2 = tnd2->GetNext()) {
                                                    if (tnd2->GetName() == "Name") {
                                                        cname = tnd2->GetChildren()->GetContent();
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                cname = id + "/" + cname;
                                nodes[cname] = cnd;
                                dlg.channelNames.push_back(cname);
                                dlg.channelColors[cname] = xlWHITE;
                            }
                        }
                    }
                }
            } else {
                logger_base.warn("Could not parse XML file %s.", (const char*)ent->GetName().c_str());
                wxLogError("Could not parse XML file %s", ent->GetName().c_str());
            }
        }
        ent = zin.GetNextEntry();
    }

    std::sort(dlg.channelNames.begin(), dlg.channelNames.end(), stdlistNumberAwareStringCompare);
    dlg.channelNames.insert(dlg.channelNames.begin(), "");
    std::sort(dlg.ccrNames.begin(), dlg.ccrNames.end(), stdlistNumberAwareStringCompare);
    dlg.ccrNames.insert(dlg.ccrNames.begin(), "");

    dlg.Init();

    if (dlg.ShowModal() != wxID_OK) {
        return;
    }

    int row = 0;
    for (size_t m = 0; m < dlg.modelNames.size(); m++) {
        std::string modelName = dlg.modelNames[m];
        Model* mc = GetModel(modelName);
        ModelElement* model = nullptr;
        for (size_t i = 0; i < _sequenceElements.GetElementCount(); i++) {
            if (_sequenceElements.GetElement(i)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(i)->GetName()) {
                model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(i));
            }
        }
        if (dlg.ChannelMapGrid->GetCellValue(row, 3) != "" && !dlg.MapByStrand->IsChecked()) {
            MapLSPEffects(model->GetEffectLayer(0), nodes[dlg.ChannelMapGrid->GetCellValue(row, 3)],
                          dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4));
        }
        row++;

        if (!dlg.MapByStrand->IsChecked()) {
            for (int str = 0; str < mc->GetNumSubModels(); str++) {
                SubModelElement* se = model->GetSubModel(str);

                if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                    MapLSPEffects(se->GetEffectLayer(0), nodes[dlg.ChannelMapGrid->GetCellValue(row, 3)],
                                  dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4));
                }
                row++;
            }
        }
        for (int str = 0; str < mc->GetNumStrands(); str++) {
            StrandElement* se = model->GetStrand(str, true);

            if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                if (dlg.MapByStrand->IsChecked()) {
                    MapLSPStrand(se, strandNodes[dlg.ChannelMapGrid->GetCellValue(row, 3)],
                                 dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4));
                } else {
                    MapLSPEffects(se->GetEffectLayer(0), nodes[dlg.ChannelMapGrid->GetCellValue(row, 3)],
                                  dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4));
                }
            }
            row++;
            if (!dlg.MapByStrand->IsChecked()) {
                for (int n = 0; n < mc->GetStrandLength(str); n++) {
                    if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                        NodeLayer* nl = se->GetNodeLayer(n, true);
                        MapLSPEffects(nl, nodes[dlg.ChannelMapGrid->GetCellValue(row, 3)],
                                      dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4));
                    }
                    row++;
                }
            }
        }
    }

    float elapsedTime = sw.Time() / 1000.0; // msec => sec
    SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}

static void ImportServoData(int min_limit, int max_limit, EffectLayer* layer, std::string name,
                            const std::vector<VSAFile::vsaEventRecord>& events, int sequence_end_time, uint32_t timing, bool is_16bit = true)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Importing servo data for " + name);

    if (min_limit == max_limit) {
        logger_base.error("ImportServoData cannot have min limit and max limit equal. Aborting import as it would crash.");
        return;
    }
    if (layer == nullptr) {
        logger_base.crit("ImportServoData cannot have null layer to import onto - this is going to crash.");
    }

    float last_pos = -1.0;
    int last_time = 0;
    bool warn = true;

    for (int i = 0; i < events.size(); ++i) {
        std::string palette = "C_BUTTON_Palette1=#FFFFFF,C_CHECKBOX_Palette1=1";
        std::string settings;
        if (is_16bit) {
            settings += "E_CHECKBOX_16bit=1,";
        } else {
            settings += "E_CHECKBOX_16bit=0,";
        }
        settings += "E_CHOICE_Channel=" + name + ",";
        settings += "E_VALUECURVE_Servo=Active=TRUE|Id=ID_VALUECURVE_Servo|Type=Ramp|Min=0.00|Max=1000.00|";
        float start_pos = (((float)events[i].start_pos - (float)min_limit) * 100.0) / ((float)max_limit - (float)min_limit);
        settings += "P1=" + wxString::Format("%3.1f", start_pos * 10.0).ToStdString() + "|";
        float end_pos = (((float)events[i].end_pos - (float)min_limit) * 100.0) / ((float)max_limit - (float)min_limit);
        if (start_pos < 0.0) {
            if (warn) {
                DisplayWarning(wxString::Format("%s: Servo Limit Exceeded. start_pos < 0%% : %.2f min/max %d/%d", name, start_pos, min_limit, max_limit).ToStdString());
                warn = false;
            }
            start_pos = 0.0;
        } else if (start_pos > 100.0) {
            if (warn) {
                DisplayWarning(wxString::Format("%s: Servo Limit Exceeded. start_pos > 100%% : %.2f min/max %d/%d", name, start_pos, min_limit, max_limit).ToStdString());
                warn = false;
            }
            start_pos = 100.0;
        }
        if (end_pos > 100.0) {
            if (warn) {
                DisplayWarning(wxString::Format("%s: Servo Limit Exceeded. end_pos > 100%% : %.2f min/max %d/%d", name, end_pos, min_limit, max_limit).ToStdString());
                warn = false;
            }
            end_pos = 100.0;
        } else if (end_pos < 0.0) {
            if (warn) {
                DisplayWarning(wxString::Format("%s: Servo Limit Exceeded. end_pos < 0%% : %.2f min/max %d/%d", name, end_pos, min_limit, max_limit).ToStdString());
                warn = false;
            }
            end_pos = 0.0;
        }
        settings += "P2=" + wxString::Format("%3.1f", end_pos * 10.0).ToStdString() + "|RV=TRUE";
        if (last_pos == -1.0) {
            last_pos = start_pos;
        }
        if (events[i].start_time > 0) {
            std::string settings2;
            if (is_16bit) {
                settings2 += "E_CHECKBOX_16bit=1,";
            } else {
                settings2 += "E_CHECKBOX_16bit=0,";
            }
            settings2 += "E_CHOICE_Channel=" + name + ",";
            settings2 += "E_TEXTCTRL_Servo=" + wxString::Format("%3.1f", last_pos).ToStdString() + ",";
            layer->AddEffect(0, "Servo", settings2, palette, last_time, events[i].start_time * timing, false, false);
        }
        layer->AddEffect(0, "Servo", settings, palette, events[i].start_time * timing, events[i].end_time * timing, false, false);
        last_pos = end_pos;
        last_time = events[i].end_time * timing;

        // check for filling to end of sequence
        if (i == events.size() - 1) {
            if (last_time < sequence_end_time) {
                std::string settings3;
                if (is_16bit) {
                    settings3 += "E_CHECKBOX_16bit=1,";
                } else {
                    settings3 += "E_CHECKBOX_16bit=0,";
                }
                settings3 += "E_CHOICE_Channel=" + name + ",";
                settings3 += "E_TEXTCTRL_Servo=" + wxString::Format("%3.1f", last_pos).ToStdString() + ",";
                layer->AddEffect(0, "Servo", settings3, palette, last_time, sequence_end_time, false, false);
            }
        }
    }
    logger_base.debug("Importing servo data done.");
}

void xLightsFrame::ImportVsa(const wxFileName& filename)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxStopWatch sw; // start a stopwatch timer

    VsaImportDialog dlg(this);
    VSAFile vsa(filename.GetFullPath().ToStdString());
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;
    dlg.Init(&vsa, false);

    if (dlg.ShowModal() == wxID_CANCEL) {
        return;
    }

    const std::vector<VSAFile::vsaTrackRecord>& tracks = vsa.GetTrackInfo();
    const std::vector<std::vector<VSAFile::vsaEventRecord>>& events = vsa.GetEventInfo();
    const uint32_t vsa_timing = vsa.GetTiming();

    for (int m = 0; m < dlg.selectedModels.size(); ++m) {
        std::string modelName = dlg.selectedModels[m];
        if (modelName != "") {
            ModelElement* model = nullptr;
            for (size_t i = 0; i < _sequenceElements.GetElementCount(); i++) {
                if (_sequenceElements.GetElement(i)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(i)->GetName()) {
                    model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(i));
                    break;
                }
            }

            if (model != nullptr) {
                EffectLayer* layer;
                int layer_number = dlg.selectedLayers[m];
                while (model->GetEffectLayerCount() < layer_number + 1) {
                    model->AddEffectLayer();
                }

                layer = model->GetEffectLayer(layer_number);
                if (layer != nullptr && dlg.selectedChannels[m] != "") {
                    if (m < dlg.trackIndex.size()) {
                        bool is_16bit = true;
                        int idx = dlg.trackIndex[m];
                        if (idx < events.size()) {
                            switch ((VSAFile::vsaControllers)(tracks[idx].controller)) {
                            case VSAFile::MINISSC_SERVO:
                            case VSAFile::DMX_DIMMER:
                            case VSAFile::DMX_RELAY:
                                is_16bit = false;
                            default:
                                break;
                            }
                            ImportServoData(tracks[idx].min_limit, tracks[idx].max_limit, layer, dlg.selectedChannels[m], events[idx], _sequenceElements.GetSequenceEnd(), vsa_timing, is_16bit);
                        } else {
                            logger_base.error("ImportVSA: idx exceeds available events.");
                        }
                    } else {
                        logger_base.error("ImportVSA: m exceeds available tracks.");
                    }
                }
            }
        }
    }

    float elapsedTime = sw.Time() / 1000.0; // msec => sec
    SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}

void xLightsFrame::CloneXLightsEffects(EffectLayer* target, EffectLayer* src, bool eraseExisting)
{
    if (eraseExisting) {
        target->DeleteAllEffects();
    }

    for (int x = 0; x < src->GetEffectCount(); x++) {
        Effect* ef = src->GetEffect(x);
        if (!target->HasEffectsInTimeRange(ef->GetStartTimeMS(), ef->GetEndTimeMS())) {
            std::string settings = ef->GetSettingsAsString();

            // remove lock if it is there
            Replace(settings, ",X_Effect_Locked=True", "");
            target->AddEffect(0, ef->GetEffectName(), settings, ef->GetPaletteAsString(),
                              ef->GetStartTimeMS(), ef->GetEndTimeMS(), 0, false);
        }
    }
}

bool xLightsFrame::CloneXLightsEffects(const std::string& target,
                                       const std::string& source,
                                       SequenceElements& seqEl,
                                       bool eraseExisting)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    Element* from = seqEl.GetElement(source);
    Element* to = seqEl.GetElement(target);

    if (from == nullptr || to == nullptr) {
        logger_base.debug("Mapping xLights effect from %s to %s failed as the effect was not found in the source sequence.", (const char*)source.c_str(), (const char*)target.c_str());
        // printf("Source element %s doesn't exist\n", name.c_str());
        return false;
    }
    _sequenceElements.get_undo_mgr().CreateUndoStep();
    while (to->GetEffectLayerCount() < from->GetEffectLayerCount()) {
        to->AddEffectLayer();
    }
    for (size_t x = 0; x < from->GetEffectLayerCount(); x++) {
        CloneXLightsEffects(to->GetEffectLayer(x), from->GetEffectLayer(x), eraseExisting);
    }
    return true;
}
