/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/stopwatch.h>
#include <wx/config.h>
#include <wx/regex.h>
#include <wx/tokenzr.h>
#include <wx/uri.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

#include <pugixml.hpp>

#include "ui/sequencer/BufferPanel.h"
#include "ui/import-export/ConvertLogDialog.h"
#include "render/DataLayer.h"
#include "ui/effects/EffectAssist.h"
#include "utils/ExternalHooks.h"
#include "import_export/FileConverter.h"
#include "render/FontManager.h"
#include "ui/layout/HousePreviewPanel.h"
#include "ui/import-export/LMSImportChannelMapDialog.h"
#include "import_export/LOREdit.h"
#include "ui/layout/ModelPreview.h"
#include "ui/app-shell/SaveChangesDialog.h"
#include "ui/diagnostics/SearchPanel.h"
#include "ui/sequencer/SelectPanel.h"
#include "ui/sequencer/SeqSettingsDialog.h"
#include "ui/media/SequenceVideoPanel.h"
#include "ui/import-export/SuperStarImportDialog.h"
#include "UtilFunctions.h"
#include "ui/wxUtilities.h"
#include "import_export/VSAFile.h"
#include "ui/layout/ViewsModelsPanel.h"
#include "ui/import-export/VsaImportDialog.h"
#include "ui/import-export/xLightsImportChannelMapDialog.h"
#include "xLightsMain.h"
#include "render/SequenceMedia.h"
#include <wx/textdlg.h>
#include "xLightsVersion.h"
#include "models/DMX/DmxModel.h"
#include "models/ModelGroup.h"
#include "ui/sequencer/MainSequencer.h"

#include "render/SequencePackage.h"
#include "import_export/Vixen3.h"
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
#include "ui/wxUtilities.h"

#include <log.h>
void xLightsFrame::AddAllModelsToSequence()
{
    std::string models_to_add;
    bool first_model = true;
    for (auto& it : AllModels) {
        if (!_sequenceElements.ElementExists(it.second->GetName(), 0)) {
            if (!first_model) {
                models_to_add += ",";
            }
            models_to_add += it.second->GetName();
            first_model = false;
        }
    }

    _sequenceElements.AddMissingModelsToSequence(models_to_add);
}

void xLightsFrame::NewSequence(const std::string& media, uint32_t durationMS, uint32_t frameMS, const std::string& defView)
{
    if (readOnlyMode) {
        DisplayError("Sequences cannot be created in read only mode!", this);
        return;
    }

    // close any open sequences
    if (!CloseSequence()) {
        return;
    }

    bool wizardactive;
    if (media.empty() && durationMS == 0) {
        wizardactive = true;
    } else {
        wizardactive = false;
    }

    // assign global xml file object
    CurrentSeqXmlFile = new SequenceFile(CurrentDir.ToStdString(), frameMS);

    if (_modelBlendDefaultOff) {
        CurrentSeqXmlFile->setSupportsModelBlending(false);
    }

    SeqSettingsDialog setting_dlg(this, CurrentSeqXmlFile, &_sequenceElements, mediaDirectories, wxT(""), _defaultSeqView, wizardactive, media, durationMS);
    int ret_code = wxID_ANY;
    if(wizardactive) {
        setting_dlg.Fit();
        ret_code = setting_dlg.ShowModal();
    }
    if (wizardactive && ret_code == wxID_CANCEL) {
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
    // Flip to sequencer tab
    Notebook1->SetSelection(Notebook1->GetPageIndex(PanelSequencer));

    // load media if available
    if (CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->HasAudioMedia()) {
        SetMediaFilename(CurrentSeqXmlFile->GetMedia()->FileName());
    }

    wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
    int ms = atoi(mss.c_str());

    spdlog::info("New sequence created Type {} Timing {}ms.", CurrentSeqXmlFile->GetSequenceType(), ms);

    // Build a minimal empty document for LoadSequencer (new sequence has no XML to load)
    pugi::xml_document emptyDoc;
    auto root = emptyDoc.append_child("xsequence");
    root.append_attribute("FixedPointTiming") = "1";
    root.append_attribute("ModelBlending") = CurrentSeqXmlFile->supportsModelBlending() ? "true" : "false";
    root.append_child("DisplayElements");
    root.append_child("ElementEffects");

    LoadSequencer(*CurrentSeqXmlFile, emptyDoc);
    CurrentSeqXmlFile->SetSequenceLoaded(true);
    CurrentSeqXmlFile->ApplyPendingTimings(this);
    if (_sequenceElements.GetNumberOfTimingElements() == 0) {
        if (CurrentSeqXmlFile->GetSequenceType() != "Effect") {
            // only add timing if the user didnt set up timings (effect sequences start with none)
            std::string new_timing = "New Timing";
            CurrentSeqXmlFile->AddNewTimingSection(new_timing, this);
            _sequenceElements.AddTimingToAllViews(new_timing);
        }
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
    if (revertToMenuItem) revertToMenuItem->Enable(true);

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
    bool isEffect = CurrentSeqXmlFile->GetSequenceType() == "Effect";
    if (isEffect) {
        displayElementsPanel->SelectView("Master View");
    } else if (view == "All Models") {
        AddAllModelsToSequence();
        displayElementsPanel->SelectView("Master View");
    } else if (view != "Empty") {
        displayElementsPanel->SelectView(view);
    }
    mainSequencer->ViewChoice->Show(!isEffect);
    mainSequencer->ViewLabel->Show(!isEffect);
    mainSequencer->Layout();
    displayElementsPanel->SetEffectSequenceMode(isEffect);

    SetAudioControls();
    Notebook1->SetSelection(Notebook1->GetPageIndex(PanelSequencer));

    if (isEffect) {
        // Auto-open Display Elements panel so user can add models
        wxCommandEvent evt;
        ShowDisplayElements(evt);
    }
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

void xLightsFrame::OpenSequence(const wxString& passed_filename, ConvertLogDialog* plog, const wxString &rp)
{
    ClearNonExistentFiles();

    bool loaded_fseq = false;
    wxString filename;
    wxString wildcards = "xLights Sequence files (*.xsq;*.xml)|*.xsq;*.xml|Old xLights Sequence files (*.xml)|*.xml|FSEQ files (*.fseq)|*.fseq|Sequence Backups (*.xbkp)|*.xbkp";
    if (passed_filename.IsEmpty()) {
        filename = wxFileSelector("Choose sequence file to open", CurrentDir, wxEmptyString, "*.xsq", wildcards, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    } else {
        filename = passed_filename;
    }
    spdlog::debug("Opening File: {}", filename.ToStdString());
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

        if (rp.IsEmpty()) {
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
                        if (wxMessageBox("Autosaved file found which seems to be newer than your sequence file ... would you like to open that instead and replace your xsq file?", "Newer file found", wxYES_NO) == wxYES) {
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
        }
        wxStopWatch sw; // start a stopwatch timer

        wxFileName selected_file(filename);
        SetPanelSequencerLabel(selected_file.GetName().ToStdString());
        wxFileName xml_file = selected_file;
        
        if (rp.IsEmpty()) {
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
                    spdlog::debug("Moving FSEQ File: '{}' to '{}'", fseq_file_SEQ_fold.GetPath().ToStdString(), fseq_file.GetPath().ToStdString());
                    wxRenameFile(fseq_file_SEQ_fold.GetFullPath(), fseq_file.GetFullPath());
                }
            } else {
                // if FSEQ File is Found in FSEQ Folder, remove old file next to the Seq File
                /***************************/
                // TODO: Maybe remove this if Keith/Gil/Dan think it's bad - Scott
                if (FileExists(fseq_file_SEQ_fold)) {
                    // remove FSEQ file next to seg file
                    spdlog::debug("Deleting old FSEQ File: '{}'", fseq_file_SEQ_fold.GetPath().ToStdString());
                    wxRemoveFile(fseq_file_SEQ_fold.GetFullPath()); //
                }
            }
        }

        xlightsFilename = fseq_file.GetFullPath(); // this need to be set , as it is checked when saving is triggered

        // load the fseq data file if it exists
        if (FileExists(fseq_file)) {
            spdlog::debug("Opening FSEQ File at: '{}'", fseq_file.GetFullPath().ToStdString());
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

            spdlog::debug("    Fseq file loaded.");
            spdlog::debug("        Channels {}", _seqData.NumChannels());
            spdlog::debug("        Frame Time {}", _seqData.FrameTime());
            spdlog::debug("        Frames {}", _seqData.NumFrames());
            spdlog::debug("        Length {}", _seqData.TotalTime());
        } else {
            spdlog::debug("Could not Find FSEQ File at: '{}'", fseq_file.GetFullPath().ToStdString());
        }
        
        wxFileName realPath = rp;
        if (rp.IsEmpty()) {
            realPath = xml_file;
        }

        // assign global xml file object
        CurrentSeqXmlFile = new SequenceFile(xml_file.GetFullPath().ToStdString());

        // open the xml file so we can see if it has media
        auto loadDoc = CurrentSeqXmlFile->Open(GetShowDirectory(), false, realPath.GetFullPath().ToStdString());

        // Check if sequence was created with a very old version of xLights
        if (loadDoc.has_value() && !CurrentSeqXmlFile->GetVersion().empty() &&
            IsVersionOlder("2020.01", CurrentSeqXmlFile->GetVersion())) {
            spdlog::warn("Sequence '{}' was last saved with xLights {} which is no longer directly supported.",
                         CurrentSeqXmlFile->GetName(), CurrentSeqXmlFile->GetVersion());
            if ((!_renderMode && !_checkSequenceMode) || _promptBatchRenderIssues) {
                if (wxMessageBox(
                    wxString::Format("This sequence was last saved with xLights %s which is no longer directly supported.\n\n"
                                     "To properly migrate this sequence, please first open and save it using a version of xLights from 2025.\n\n"
                                     "If you continue loading, effects may render very differently than expected.\n\n"
                                     "Do you want to continue loading anyway?",
                                     CurrentSeqXmlFile->GetVersion()),
                    "Old Sequence Version",
                    wxYES_NO | wxNO_DEFAULT | wxICON_WARNING) == wxNO) {
                    delete CurrentSeqXmlFile;
                    CurrentSeqXmlFile = nullptr;
                    return;
                }
            }
        }

        _renderCache.SetSequence(renderCacheDirectory, CurrentSeqXmlFile->GetName());

        // if fseq didn't have media check xml
        if (CurrentSeqXmlFile->GetMediaFile() != "") {
            media_file = mapFileName(wxString(CurrentSeqXmlFile->GetMediaFile()));
            ObtainAccessToURL(media_file.GetFullPath().ToStdString());
        }

        if (media_file.GetName() != "") {
            spdlog::debug("Media file from sequence: '{}'", media_file.GetFullPath().ToStdString());

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
                spdlog::debug("    Did not exist, attepting to map to: '{}'", media_file.GetFullPath().ToStdString());
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
                spdlog::debug("    Still did not exist, attepting to map to: '{}'", media_file.GetFullPath().ToStdString());
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
                        detect_audio.SetPath(GetShowDirectory() + GetPathSeparator() + audFile);
                        if (FileExists(detect_audio)) {
                            media_file = detect_audio;
                            ObtainAccessToURL(media_file.GetFullPath().ToStdString());
                            break;
                        }
                    }
                    fcont = audDirectory.GetNext(&audFile);
                }
                spdlog::debug("    Still did not exist, attepting to map to: '{}'", media_file.GetFullPath().ToStdString());
            }
        }

        // if fseq had media update xml
        wxString mfFP = media_file.GetFullPath();
        if (!mfFP.empty() && !CurrentSeqXmlFile->HasAudioMedia() && FileExists(mfFP) && media_file.IsFileReadable()) {
            CurrentSeqXmlFile->SetMediaFile(GetShowDirectory(), mfFP, true);
            int length_ms = CurrentSeqXmlFile->GetMedia()->LengthMS();
            CurrentSeqXmlFile->SetSequenceDurationMS(length_ms);
            playAnimation = false;
        } else {
            playAnimation = true;
        }

        if (CurrentSeqXmlFile->WasConverted()) {
            spdlog::debug("Loaded Sequence was Converted, need to check settings");
            // abort any in progress render ... as it may be using any already open media
            bool aborted = false;
            if (CurrentSeqXmlFile->GetMedia() != nullptr) {
                aborted = AbortRender();
            }

            SeqSettingsDialog setting_dlg(this, CurrentSeqXmlFile, &_sequenceElements, mediaDirectories, wxT("V3 file was converted. Please check settings!"), wxEmptyString);
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
        spdlog::debug("Sequence Timing: {}", ms);
        Notebook1->SetSelection(Notebook1->GetPageIndex(PanelSequencer));
        bool loaded_xml = loadDoc.has_value() && SeqLoadXlightsFile(*CurrentSeqXmlFile, *loadDoc, true);

        unsigned int numChan = GetMaxNumChannels();
        size_t memRequired = std::max(CurrentSeqXmlFile->GetSequenceDurationMS(), mMediaLengthMS) / ms;
        memRequired *= numChan;
        memRequired /= 1024; // ->kb
        memRequired /= 1024; // ->mb
        if (memRequired > (GetPhysicalMemorySizeMB() - 1024) && (_promptBatchRenderIssues || (!_renderMode && !_checkSequenceMode))) {
            DisplayWarning(wxString::Format("The setup requires a large amount of memory (%lu MB) which could result in performance issues.", (unsigned long)memRequired), this);
        }

        spdlog::debug("Sequence Num Channels: {} or {}", numChan, _seqData.NumChannels());
        spdlog::debug("Sequence Num Frames: {}", (int)(CurrentSeqXmlFile->GetSequenceDurationMS() / ms));

        if ((roundTo4(numChan) != _seqData.NumChannels()) ||
            (CurrentSeqXmlFile->GetSequenceDurationMS() / ms) > (long)_seqData.NumFrames()) {
            if (_seqData.NumChannels() > 0) {
                if (roundTo4(numChan) != _seqData.NumChannels()) {
                    spdlog::warn("Fseq file had {} channels but sequence has {} channels so dumping the fseq data.", _seqData.NumChannels(), numChan);
                } else {
                    if ((CurrentSeqXmlFile->GetSequenceDurationMS() / ms) > (long)_seqData.NumFrames()) {
                        spdlog::warn("Fseq file had {} frames but sequence has {} frames so dumping the fseq data.",
                                     CurrentSeqXmlFile->GetSequenceDurationMS() / ms,
                                     _seqData.NumFrames());
                    }
                }
            }
            _seqData.init(numChan, mMediaLengthMS / ms, ms);
        } else if (!loaded_fseq) {
            _seqData.init(numChan, CurrentSeqXmlFile->GetSequenceDurationMS() / ms, ms);
        }

        spdlog::debug("Initializing Display Elements");
        displayElementsPanel->Initialize();

        // if we loaded the fseq but not the xml then we need to populate views
        if (!CurrentSeqXmlFile->IsOpen()) {
            // SeqLoadXlightsFile/LoadSequencer was never called, so _viewsManager may not be set
            _sequenceElements.SetViewsManager(GetViewsManager());
            std::string new_timing = "New Timing";
            CurrentSeqXmlFile->AddNewTimingSection(new_timing, this);
            _sequenceElements.AddTimingToAllViews(new_timing);
            AddAllModelsToSequence();
            displayElementsPanel->SelectView("Master View");
        }

        spdlog::debug("Starting timers");
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

        bool isEffect = CurrentSeqXmlFile->GetSequenceType() == "Effect";
        mainSequencer->ViewChoice->Show(!isEffect);
        mainSequencer->ViewLabel->Show(!isEffect);
        mainSequencer->Layout();
        displayElementsPanel->SetEffectSequenceMode(isEffect);
        if (isEffect) {
            displayElementsPanel->SelectView("Master View");
        }

        EnableSequenceControls(true);
        Notebook1->SetSelection(Notebook1->GetPageIndex(PanelSequencer));

        AddToMRU(filename);
        UpdateRecentFilesList(false);
    }
}

void xLightsFrame::AddToMRU(const std::string& filename)
{
    if (!_renderMode) {
        if (mruFiles.Index(filename) != wxNOT_FOUND) {
            mruFiles.Remove(filename);
        }
        if (mruFiles.empty()) {
            mruFiles.push_back(filename);
        } else {
            mruFiles.Insert(filename, 0);
        }
    }
}

bool xLightsFrame::CloseSequence()
{
    spdlog::debug("Closing sequence.");

    if (_autoSavePerspecive && CurrentSeqXmlFile != nullptr) {
        // save perspective on this machine so we can restore it next time
        wxConfigBase* config = wxConfigBase::Get();
        wxString machinePerspective = m_mgr->SavePerspective();
        config->Write("xLightsMachinePerspective", machinePerspective);
        spdlog::debug("AutoSave perspective");
        LogPerspective(machinePerspective);
    }

    if (mSavedChangeCount != (unsigned int)_sequenceElements.GetChangeCount() && !_renderMode && !_checkSequenceMode) {
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
                wxFileName fn(CurrentSeqXmlFile->GetFullPath());
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
    ResetAllPanelDefaultSettings();
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
        if (mainSequencer->ViewChoice != nullptr) {
            mainSequencer->ViewChoice->Clear();
            mainSequencer->ViewChoice->Show();
        }
        if (mainSequencer->ViewLabel != nullptr)
            mainSequencer->ViewLabel->Show();
    }
    if (displayElementsPanel != nullptr)
        displayElementsPanel->SetEffectSequenceMode(false);
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
    xLightsFrame::CurrentSeqXmlFile = new SequenceFile(filename.ToStdString());
    auto loadDoc = CurrentSeqXmlFile->Open(GetShowDirectory(), false, filename.ToStdString());
    if (!loadDoc) return false;
    return SeqLoadXlightsFile(*xLightsFrame::CurrentSeqXmlFile, *loadDoc, ChooseModels);
}

// Load the xml file containing effects for a particular sequence
// Returns true if file exists and was read successfully
bool xLightsFrame::SeqLoadXlightsFile(SequenceFile& xml_file, pugi::xml_document& doc, bool ChooseModels)
{
    LoadSequencer(xml_file, doc);
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
    spdlog::debug("xLightsFrame::RenderIseqData bottom_layers {}", bottom_layers);

    DataLayerSet& data_layers = CurrentSeqXmlFile->GetDataLayers();
    ConvertParameters::ReadMode read_mode;
    if (bottom_layers && data_layers.GetNumLayers() == 1 &&
        data_layers.GetDataLayer(0)->GetName() == "Nutcracker") {
        DataLayer* nut_layer = data_layers.GetDataLayer(0);
        if (nut_layer->GetDataSource() == SequenceFile::CANVAS_MODE) {
            // Don't clear, v3 workflow of augmenting the existing fseq file
            return;
        }
    }

    if (bottom_layers) {
        spdlog::debug("xLightsFrame::RenderIseqData clearing sequence data.");
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
                spdlog::debug("xLightsFrame::RenderIseqData rendering {}.", (const char*)data_layer->GetDataSource().c_str());
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

