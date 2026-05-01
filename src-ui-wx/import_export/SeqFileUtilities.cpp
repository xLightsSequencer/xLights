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
#include "settings/XLightsConfigAdapter.h"
#include <wx/regex.h>
#include <wx/tokenzr.h>
#include <wx/uri.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

#include <pugixml.hpp>

#include "sequencer/BufferPanel.h"
#include "import_export/ConvertLogDialog.h"
#include "render/DataLayer.h"
#include "effects/EffectAssist.h"
#include "utils/ExternalHooks.h"
#include "import_export/FileConverter.h"
#include "render/FontManager.h"
#include "layout/HousePreviewPanel.h"
#include "import_export/LMSImportChannelMapDialog.h"
#include "import_export/LOREdit.h"
#include "layout/ModelPreview.h"
#include "app-shell/SaveChangesDialog.h"
#include "diagnostics/SearchPanel.h"
#include "sequencer/SelectPanel.h"
#include "sequencer/SeqSettingsDialog.h"
#include "media/SequenceVideoPanel.h"
#include "import_export/SuperStarImportDialog.h"
#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"
#include "import_export/VSAFile.h"
#include "layout/ViewsModelsPanel.h"
#include "import_export/VsaImportDialog.h"
#include "import_export/xLightsImportChannelMapDialog.h"
#include "render/SequenceMedia.h"
#include "media/MediaCompatibility.h"
#include "media/VideoTranscoder.h"
#include "utils/FileUtils.h"
#include <set>
#include <wx/textdlg.h>
#include <wx/richmsgdlg.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/progdlg.h>
#include <wx/filename.h>
#include "xLightsVersion.h"
#include "models/DMX/DmxModel.h"
#include "models/ModelGroup.h"
#include "sequencer/MainSequencer.h"

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
#include "shared/utils/wxUtilities.h"

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
    CurrentSeqXmlFile = new SequenceFile(ToStdString(CurrentDir), frameMS);

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
    FileUtils::ClearNonExistentFiles();
    _sequenceElements.GetSequenceMedia().ClearRelocations();

    bool loaded_fseq = false;
    wxString filename;
    wxString wildcards = "xLights Sequence files (*.xsq;*.xml)|*.xsq;*.xml|Old xLights Sequence files (*.xml)|*.xml|FSEQ files (*.fseq)|*.fseq|Sequence Backups (*.xbkp)|*.xbkp";
    if (passed_filename.IsEmpty()) {
        filename = wxFileSelector("Choose sequence file to open", CurrentDir, wxEmptyString, "*.xsq", wildcards, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    } else {
        filename = passed_filename;
    }
    spdlog::debug("Opening File: {}", ToStdString(filename));
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
            if (!_renderMode && !_checkSequenceMode && wxFileName(filename).GetExt().Lower() != "xbkp" && wxFileName(filename).GetExt().Lower() != "fseq") {
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
                    spdlog::debug("Moving FSEQ File: '{}' to '{}'", ToStdString(fseq_file_SEQ_fold.GetPath()), ToStdString(fseq_file.GetPath()));
                    wxRenameFile(fseq_file_SEQ_fold.GetFullPath(), fseq_file.GetFullPath());
                }
            } else {
                // if FSEQ File is Found in FSEQ Folder, remove old file next to the Seq File
                /***************************/
                // TODO: Maybe remove this if Keith/Gil/Dan think it's bad - Scott
                if (FileExists(fseq_file_SEQ_fold)) {
                    // remove FSEQ file next to seg file
                    spdlog::debug("Deleting old FSEQ File: '{}'", ToStdString(fseq_file_SEQ_fold.GetPath()));
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
            spdlog::debug("Could not Find FSEQ File at: '{}'", ToStdString(fseq_file.GetFullPath()));
        }
        
        wxFileName realPath = rp;
        if (rp.IsEmpty()) {
            realPath = xml_file;
        }

        // assign global xml file object
        CurrentSeqXmlFile = new SequenceFile(ToStdString(xml_file.GetFullPath()));

        // open the xml file so we can see if it has media
        auto loadDoc = CurrentSeqXmlFile->Open(GetShowDirectory(), false, ToStdString(realPath.GetFullPath()));

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

        // Check media compatibility with AVFoundation/AudioToolbox
        if (loaded_xml && !_renderMode && !_checkSequenceMode) {
            // Allow user to suppress this warning until the next xLights version release
            wxString suppressedVersion;
            GetXLightsConfig()->Read("xLightsSuppressMediaCompatWarnVersion", &suppressedVersion, "");
            if (suppressedVersion != xlights_version_string) {
                std::string audioFile = CurrentSeqXmlFile->GetMediaFile();
                std::vector<std::string> videoFiles = _sequenceElements.GetSequenceMedia().GetVideoFilePaths();
                auto issues = MediaCompatibility::CheckSequenceMedia(audioFile, videoFiles);
                if (!issues.empty()) {
                    // Build the file list once for the monospace box and the log.
                    wxString fileList;
                    for (const auto& issue : issues) {
                        wxString type = issue.isVideo ? "Video" : "Audio";
                        std::string basename = std::filesystem::path(issue.filePath).filename().string();
                        fileList += wxString::Format("  %s: %s\n    Reason: %s\n", type, basename, issue.reason);
                    }
                    if (!fileList.empty() && fileList.Last() == '\n') {
                        fileList.RemoveLast();
                    }
                    spdlog::warn("Media compatibility warning:\n{}", fileList.ToStdString());

                    // Custom dialog: wxRichMessageDialog (=NSAlert on macOS) collapses
                    // blank lines and is fixed-width, which made the multi-paragraph
                    // guidance unreadable. This dialog uses system-font wxStaticText
                    // for prose, a scrollable monospace wxTextCtrl for the file list
                    // (in case there are many files), and a one-line monospace text
                    // ctrl for the ffmpeg command so the user can copy it.
                    constexpr int kWrapWidth = 660;
                    wxDialog dlg(this, wxID_ANY, "Media Compatibility Warning",
                                 wxDefaultPosition, wxSize(720, 560),
                                 wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
                    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

                    auto* intro = new wxStaticText(&dlg, wxID_ANY,
                        "The following media files are in a format that will not render on upcoming versions of xLights:");
                    intro->Wrap(kWrapWidth);
                    topSizer->Add(intro, 0, wxALL, 12);

                    wxTextCtrl* fileListCtrl = new wxTextCtrl(&dlg, wxID_ANY, fileList,
                        wxDefaultPosition, wxSize(-1, 160),
                        wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxBORDER_SUNKEN);
                    fileListCtrl->SetFont(wxFont(wxFontInfo(11).Family(wxFONTFAMILY_TELETYPE)));
                    fileListCtrl->SetInsertionPoint(0);
                    // proportion=1 → grows to fill any extra vertical space when resized
                    topSizer->Add(fileListCtrl, 1, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 12);

                    auto* advice1 = new wxStaticText(&dlg, wxID_ANY,
                        "Consider re-encoding with Handbrake using H.264/H.265 (video) or AAC/MP3 (audio) for maximum compatibility and performance.");
                    advice1->Wrap(kWrapWidth);
                    topSizer->Add(advice1, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);

                    auto* advice2 = new wxStaticText(&dlg, wxID_ANY,
                        "If you need pixel-perfect lossless RGB video (the original reason for using uncompressed AVI), "
                        "re-encode the source as an uncompressed RGB MOV — support for the legacy QuickTime lossless "
                        "codecs (Animation/qtrle, PNG, etc.) is being dropped, so uncompressed RGB in a mov container "
                        "is the recommended bit-exact format going forward. With ffmpeg:");
                    advice2->Wrap(kWrapWidth);
                    topSizer->Add(advice2, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);

                    wxTextCtrl* cmdCtrl = new wxTextCtrl(&dlg, wxID_ANY,
                        "ffmpeg -i input.avi -c:v rawvideo -pix_fmt rgb24 output.mov",
                        wxDefaultPosition, wxDefaultSize,
                        wxTE_READONLY | wxBORDER_SUNKEN);
                    cmdCtrl->SetFont(wxFont(wxFontInfo(11).Family(wxFONTFAMILY_TELETYPE)));
                    topSizer->Add(cmdCtrl, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 12);

                    auto* note = new wxStaticText(&dlg, wxID_ANY,
                        "xLights' model export dialog can also write this format directly via \"Lossless RGB Video, *.mov\".");
                    note->Wrap(kWrapWidth);
                    topSizer->Add(note, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);

                    // If any flagged file is an animated GIF, mention that those
                    // are handled by switching the owning Video effect to a
                    // Pictures effect (which already plays GIFs natively) — no
                    // ffmpeg transcode needed for those.
                    bool anyGif = false;
                    for (const auto& issue : issues) {
                        if (issue.isAnimatedGif() && issue.canConvert()) {
                            anyGif = true;
                            break;
                        }
                    }
                    if (anyGif) {
                        auto* gifNote = new wxStaticText(&dlg, wxID_ANY,
                            "Animated GIFs above will be converted from Video effects to Pictures effects "
                            "(which natively play GIF animation) — no file conversion is performed for those.");
                        gifNote->Wrap(kWrapWidth);
                        topSizer->Add(gifNote, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
                    }

                    wxCheckBox* suppressCheck = new wxCheckBox(&dlg, wxID_ANY,
                        wxString::Format("Don't show this warning again for xLights %s", xlights_version_string));
                    topSizer->Add(suppressCheck, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);

                    // Figure out how many flagged entries are videos — only
                    // videos can be auto-converted here (audio would need a
                    // separate AudioToolbox-compatible path).
                    int videoIssueCount = 0;
                    for (const auto& issue : issues) {
                        if (issue.isVideo && issue.canConvert()) ++videoIssueCount;
                    }

                    // Custom ID so we can tell OK apart from "Convert Now".
                    const int ID_CONVERT_NOW = wxID_HIGHEST + 1;

                    wxBoxSizer* btnRow = new wxBoxSizer(wxHORIZONTAL);
                    btnRow->AddStretchSpacer(1);
                    if (videoIssueCount > 0) {
                        wxButton* convertBtn = new wxButton(&dlg, ID_CONVERT_NOW,
                            videoIssueCount == 1 ? "Convert Video Now..."
                                                 : wxString::Format("Convert %d Videos Now...", videoIssueCount));
                        btnRow->Add(convertBtn, 0, wxRIGHT, 8);
                        convertBtn->Bind(wxEVT_BUTTON, [&dlg, ID_CONVERT_NOW](wxCommandEvent&) {
                            dlg.EndModal(ID_CONVERT_NOW);
                        });
                    }
                    wxButton* okBtn = new wxButton(&dlg, wxID_OK, "OK");
                    btnRow->Add(okBtn, 0);
                    topSizer->Add(btnRow, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT | wxBOTTOM, 12);

                    dlg.SetSizer(topSizer);
                    dlg.SetMinSize(wxSize(600, 480));
                    dlg.Layout();
                    int dlgResult = dlg.ShowModal();
                    if (suppressCheck->IsChecked()) {
                        GetXLightsConfig()->Write("xLightsSuppressMediaCompatWarnVersion", wxString(xlights_version_string));
                        GetXLightsConfig()->Flush();
                    }

                    if (dlgResult == ID_CONVERT_NOW) {
                        ConvertIncompatibleVideos(issues);
                    }
                }
            }
        }

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

void xLightsFrame::ConvertIncompatibleVideos(const std::vector<MediaCompatibilityIssue>& issues)
{
    // Gather the video issues. Audio ones aren't handled here — users will
    // see the warning but need to re-encode audio separately. Animated GIFs
    // are split out: ffmpeg-transcoding a GIF to mp4/mov produces poor
    // results, so we instead rewrite the owning Video effect into a
    // Pictures effect (which plays animated GIFs natively).
    std::vector<MediaCompatibilityIssue> gifIssues;
    std::vector<std::pair<std::string, std::string>> jobs; // (source, target)
    for (const auto& issue : issues) {
        if (!issue.isVideo || !issue.canConvert()) continue;
        if (issue.isAnimatedGif()) {
            gifIssues.push_back(issue);
            continue;
        }
        std::string target = VideoTranscoder::SuggestedOutputPath(issue.filePath);
        if (target == issue.filePath) {
            // Source is already .mov (e.g. qtrle codec) — append _converted
            // so we don't overwrite the original.
            std::filesystem::path p(issue.filePath);
            p.replace_filename(p.stem().string() + "_converted.mov");
            target = p.string();
        }
        jobs.emplace_back(issue.filePath, target);
    }

    // Convert GIF effects first — instant, no progress dialog needed.
    int gifsConverted = ConvertGifVideoEffectsToPictures(gifIssues);

    if (jobs.empty()) {
        if (gifsConverted > 0) {
            wxMessageBox(wxString::Format(
                            "Converted %d animated GIF Video effect(s) to Pictures effects.\n"
                            "Remember to save the sequence to persist the changes.",
                            gifsConverted),
                         "GIF effect conversion results",
                         wxOK | wxICON_INFORMATION, this);
        }
        return;
    }

    // Progress dialog spans the whole batch; per-file we weight the progress
    // bar by frame counts we don't know up front, so just advance one tick
    // per file completed and use the file's own progress callback for the
    // fine-grained feedback inside the bar.
    wxProgressDialog progDlg("Converting video files",
                             "Preparing...", 1000, this,
                             wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT |
                             wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME);
    progDlg.SetSize(wxSize(520, -1));

    std::map<std::string, std::string> completed; // src -> dst
    std::vector<std::string> failures;
    bool userCancelled = false;

    for (size_t i = 0; i < jobs.size() && !userCancelled; ++i) {
        const auto& [src, dst] = jobs[i];
        std::string srcName = std::filesystem::path(src).filename().string();
        wxString baseMsg = wxString::Format("Converting %s (%zu of %zu)...",
                                            srcName, i + 1, jobs.size());
        progDlg.Update(0, baseMsg);

        auto progressCb = [&](int frame, int total) -> bool {
            int pct = 0;
            if (total > 0) {
                pct = (int)((double)frame / total * 1000.0);
                if (pct > 999) pct = 999;
            } else {
                pct = (frame % 1000);
            }
            bool cont = progDlg.Update(pct, baseMsg + wxString::Format(" frame %d", frame));
            if (!cont) userCancelled = true;
            return cont;
        };

        std::string err = VideoTranscoder::Transcode(src, dst, progressCb);
        if (userCancelled) break;
        if (!err.empty()) {
            spdlog::error("Video conversion failed for {}: {}", src, err);
            failures.push_back(srcName + ": " + err);
            // Remove any partial output so the user doesn't mistake it for
            // a finished file.
            std::error_code ec;
            std::filesystem::remove(dst, ec);
        } else {
            completed[src] = dst;
        }
    }
    progDlg.Update(1000);

    if (userCancelled) {
        // Don't rewrite effects if the user aborted — leaving the sequence
        // pointing at the originals is the least-surprising outcome.
        wxMessageBox("Conversion cancelled. Any files already completed were left in place but the sequence was not updated.",
                     "Cancelled", wxOK | wxICON_INFORMATION, this);
        return;
    }

    // Walk all video effects in the loaded sequence and rewrite filenames
    // where the (resolved) source path matches one we just converted. Track
    // the original stored keys so we can evict only those stale entries from
    // SequenceMedia — wiping the whole cache would also drop images, SVGs,
    // etc. and leave the Sequence Settings Media tab empty.
    int rewritten = 0;
    std::set<std::string> staleCacheKeys;
    std::set<std::string> newCacheKeys;

    auto rewriteEffectLayers = [&](Element* elem) {
        for (int layer = 0; layer < (int)elem->GetEffectLayerCount(); ++layer) {
            EffectLayer* el = elem->GetEffectLayer(layer);
            for (int k = 0; k < el->GetEffectCount(); ++k) {
                Effect* ef = el->GetEffect(k);
                if (ef->GetEffectName() != "Video") continue;
                SettingsMap& sm = ef->GetSettings();
                const std::string stored = sm["E_FILEPICKERCTRL_Video_Filename"];
                if (stored.empty()) continue;
                std::string resolved = FileUtils::FixFile("", stored);
                auto it = completed.find(resolved);
                if (it == completed.end()) continue;

                staleCacheKeys.insert(stored);
                staleCacheKeys.insert(resolved);

                // Preserve the relative-vs-absolute shape: if the original
                // stored value was an absolute path we write absolute; if it
                // was a bare filename or relative, preserve the directory
                // but use the destination's full filename (handles the case
                // where the output name differs, e.g. _converted.mov).
                std::filesystem::path storedPath(stored);
                std::string newStored;
                if (storedPath.is_absolute()) {
                    newStored = it->second;
                } else {
                    std::filesystem::path rewritten_path = storedPath;
                    rewritten_path.replace_filename(
                        std::filesystem::path(it->second).filename());
                    newStored = rewritten_path.string();
                }
                sm["E_FILEPICKERCTRL_Video_Filename"] = newStored;
                newCacheKeys.insert(newStored);
                ef->IncrementChangeCount();
                ++rewritten;
            }
        }
    };

    for (size_t e = 0; e < _sequenceElements.GetElementCount(); ++e) {
        Element* elem = _sequenceElements.GetElement(e);
        if (elem == nullptr) continue;
        rewriteEffectLayers(elem);
        if (elem->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            ModelElement* me = dynamic_cast<ModelElement*>(elem);
            for (int j = 0; j < me->GetStrandCount(); ++j) {
                StrandElement* se = me->GetStrand(j);
                rewriteEffectLayers(se);
            }
            for (int j = 0; j < me->GetSubModelAndStrandCount(); ++j) {
                Element* sme = me->GetSubModel(j);
                if (sme->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL) {
                    rewriteEffectLayers(sme);
                }
            }
        }
    }

    // Evict only the stale video entries so the next render picks up the
    // new .mov files. Leaves images/SVGs/audio/shader caches intact.
    auto& seqMedia = _sequenceElements.GetSequenceMedia();
    for (const auto& key : staleCacheKeys) {
        seqMedia.RemoveMedia(key);
    }
    // Pre-register the new .mov files so they appear in the Sequence Settings
    // Media tab immediately (the renderer would otherwise only lazily register
    // them on first use).
    for (const auto& key : newCacheKeys) {
        seqMedia.GetVideo(key);
    }

    wxString msg = wxString::Format("Converted %zu of %zu file(s). %d video effect(s) updated.",
                                    completed.size(), jobs.size(), rewritten);
    if (gifsConverted > 0) {
        msg += wxString::Format("\n%d animated GIF Video effect(s) converted to Pictures effects.",
                                gifsConverted);
    }
    if (!failures.empty()) {
        msg += "\n\nFailures:\n";
        for (const auto& f : failures) msg += "  " + f + "\n";
    }
    if (!completed.empty() || gifsConverted > 0) {
        msg += "\nRemember to save the sequence to persist the updated file references.";
    }
    wxMessageBox(msg, "Video conversion results",
                 wxOK | (failures.empty() ? wxICON_INFORMATION : wxICON_WARNING),
                 this);
}

int xLightsFrame::ConvertGifVideoEffectsToPictures(const std::vector<MediaCompatibilityIssue>& gifIssues)
{
    // ffmpeg can transcode an animated GIF into mp4/mov, but the result is
    // typically poor (palette / dithering / fps metadata get mangled). The
    // PicturesEffect already plays animated GIFs natively, so for GIF-backed
    // Video effects we swap the effect type to Pictures and map the
    // parameters across. Mirrors bravado67/xlights-gif-converter's mapping.
    if (gifIssues.empty()) return 0;

    // Match by both the resolved (absolute) path and any case-insensitive
    // .gif filename — different effects may store the same GIF as different
    // path shapes (relative, absolute, bare basename) and they all need to
    // be caught.
    std::set<std::string> gifResolved;
    for (const auto& issue : gifIssues) {
        gifResolved.insert(issue.filePath);
    }

    int rewritten = 0;
    // Track which cache keys to move from the video cache to the image cache.
    // Both the user-stored path and the FixFile-resolved path need to be
    // evicted because either could have been the registration key.
    std::set<std::string> staleVideoKeys;
    std::set<std::string> newImageKeys;

    auto rewriteEffectLayers = [&](Element* elem) {
        for (int layer = 0; layer < (int)elem->GetEffectLayerCount(); ++layer) {
            EffectLayer* el = elem->GetEffectLayer(layer);
            for (int k = 0; k < el->GetEffectCount(); ++k) {
                Effect* ef = el->GetEffect(k);
                if (ef->GetEffectName() != "Video") continue;
                SettingsMap& sm = ef->GetSettings();
                const std::string stored = sm["E_FILEPICKERCTRL_Video_Filename"];
                if (stored.empty()) continue;

                // Cheap extension check first to skip non-GIF Video effects.
                std::string lower = stored;
                std::transform(lower.begin(), lower.end(), lower.begin(),
                               [](unsigned char c) { return std::tolower(c); });
                if (lower.size() < 4 || lower.substr(lower.size() - 4) != ".gif") continue;

                // Only convert effects whose source file actually appears in
                // the issues list — keeps us from clobbering GIF effects that
                // resolve to a path the compatibility check didn't flag.
                std::string resolved = FileUtils::FixFile("", stored);
                if (gifResolved.find(resolved) == gifResolved.end() &&
                    gifResolved.find(stored) == gifResolved.end()) {
                    continue;
                }

                // Capture the few Video settings that translate cleanly to
                // the Pictures effect before we wipe E_ keys. Pictures
                // FrameRateAdj is 0..200 with no negative/reverse support,
                // so clamp non-positive Video_Speed values to 1.0.
                std::string videoSpeed = sm["E_TEXTCTRL_Video_Speed"];
                {
                    char* endp = nullptr;
                    const char* s = videoSpeed.c_str();
                    double sp = std::strtod(s, &endp);
                    if (endp == s || sp <= 0.0) videoSpeed = "1.0";
                }

                // Drop all E_* (Video-effect-specific) settings; preserve
                // B_* / T_* / X_* / palette which are layer-level and apply
                // to the Pictures effect equally.
                std::vector<std::string> toErase;
                for (const auto& it : sm) {
                    if (it.first.size() > 2 && it.first[0] == 'E' && it.first[1] == '_') {
                        toErase.push_back(it.first);
                    }
                }
                for (const auto& key : toErase) sm.erase(key);

                // Pictures-effect defaults that play a centered, looping GIF
                // at the same speed as the original Video effect.
                sm["E_TEXTCTRL_Pictures_Filename"] = stored;
                sm["E_TEXTCTRL_Pictures_FrameRateAdj"] = videoSpeed;
                sm["E_TEXTCTRL_Pictures_Speed"] = "1.0";
                sm["E_CHECKBOX_LoopGIF"] = "1";
                sm["E_CHECKBOX_SuppressGIFBackground"] = "1";
                sm["E_CHECKBOX_Pictures_PixelOffsets"] = "0";
                sm["E_CHECKBOX_Pictures_Shimmer"] = "0";
                sm["E_CHECKBOX_Pictures_TransparentBlack"] = "0";
                sm["E_CHECKBOX_Pictures_WrapX"] = "0";
                sm["E_TEXTCTRL_Pictures_TransparentBlack"] = "0";
                sm["E_CHOICE_Pictures_Direction"] = "none";
                sm["E_CHOICE_Scaling"] = "Scale To Fit";
                sm["E_SLIDER_PicturesXC"] = "0";
                sm["E_SLIDER_PicturesYC"] = "0";
                sm["E_SLIDER_Pictures_StartScale"] = "100";
                sm["E_SLIDER_Pictures_EndScale"] = "100";

                ef->SetEffectName("Pictures");
                ef->IncrementChangeCount();
                ++rewritten;

                staleVideoKeys.insert(stored);
                staleVideoKeys.insert(resolved);
                newImageKeys.insert(stored);
            }
        }
    };

    for (size_t e = 0; e < _sequenceElements.GetElementCount(); ++e) {
        Element* elem = _sequenceElements.GetElement(e);
        if (elem == nullptr) continue;
        rewriteEffectLayers(elem);
        if (elem->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            ModelElement* me = dynamic_cast<ModelElement*>(elem);
            for (int j = 0; j < me->GetStrandCount(); ++j) {
                StrandElement* se = me->GetStrand(j);
                rewriteEffectLayers(se);
            }
            for (int j = 0; j < me->GetSubModelAndStrandCount(); ++j) {
                Element* sme = me->GetSubModel(j);
                if (sme->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL) {
                    rewriteEffectLayers(sme);
                }
            }
        }
    }

    if (rewritten > 0) {
        spdlog::info("Converted {} animated GIF Video effect(s) to Pictures effects", rewritten);

        // Move the GIF entries from the SequenceMedia video cache to the image
        // cache so the Sequence Settings → Media tab shows them under the
        // right type and the renderer's lookup hits the multi-frame
        // ImageCacheEntry path that PicturesEffect uses for animated GIFs.
        auto& seqMedia = _sequenceElements.GetSequenceMedia();
        for (const auto& key : staleVideoKeys) {
            seqMedia.RemoveMedia(key);
        }
        for (const auto& key : newImageKeys) {
            seqMedia.GetImage(key);
        }

        // The grid still shows the old "Video" colour/label until it
        // re-paints; force a refresh so the user sees the change immediately.
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr) {
            mainSequencer->PanelEffectGrid->ForceRefresh();
        }
    }
    return rewritten;
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
        auto* config = GetXLightsConfig();
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
            while (!_renderEngine->IsRenderDone()) {
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

                    if (xmltime.IsValid() && xbkptime.IsValid() && xbkptime > xmltime) {
                        // set the backup to be older than the XML files to avoid re-promting
                        xmltime -= wxTimeSpan(0, 0, 0, 2); // subtract 2 seconds as FAT time resulution is 2 seconds
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

