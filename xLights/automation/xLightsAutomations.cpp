/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "../xLightsMain.h"
#include "../xLightsVersion.h"

#include "../FSEQFile.h"
#include "../outputs/Controller.h"
#include "../outputs/ControllerEthernet.h"
#include "../LayoutPanel.h"
#include "../ViewsModelsPanel.h"
#include "../controllers/ControllerCaps.h"
#include "../controllers/FPP.h"
#include "../controllers/Falcon.h"
#include "../../xSchedule/wxJSON/jsonreader.h"
#include "../../xSchedule/wxJSON/jsonwriter.h"
#include "../UtilFunctions.h"
#include "../JukeboxPanel.h"
#include "../outputs/E131Output.h"

std::string xLightsFrame::FindSequence(const std::string& seq)
{
    if (wxFile::Exists(seq))
        return seq;

    if (wxFile::Exists(CurrentDir + wxFileName::GetPathSeparator() + seq))
        return CurrentDir + wxFileName::GetPathSeparator() + seq;
    
    return "";
}

std::string xLightsFrame::ProcessAutomation(const std::string& msg)
{
    wxJSONValue val;
    wxJSONReader reader;
    if (reader.Parse(msg, &val) == 0) {
        if (!val.HasMember("cmd")) {
            return "{\"res\":504,\"msg\":\"Missing cmd.\"}";
        } else {
            auto cmd = val["cmd"].AsString();
            if (cmd == "renderAll") {
                if (CurrentSeqXmlFile == nullptr) {
                    return "{\"res\":504,\"msg\":\"No sequence open.\"}";
                }
                RenderAll();
                while (mRendering) {
                    wxYield();
                }
                return "{\"res\":200,\"msg\":\"Rendered.\"}";

            } else if (cmd == "loadSequence") {
                auto seq = val["seq"].AsString();
                seq = FindSequence(seq);
                if (seq == "") {
                    return "{\"res\":504,\"msg\":\"Sequence not found.\"}";
                }

                if (CurrentSeqXmlFile != nullptr && val["force"].AsString() != "true") {
                    return "{\"res\":504,\"msg\":\"Sequence already open.\"}";
                }

                auto oldPrompt = _promptBatchRenderIssues;
                _promptBatchRenderIssues = (val["promptIssues"].AsString() != "true"); // off by default
                OpenSequence(seq, nullptr);
                _promptBatchRenderIssues = oldPrompt;

                return "{\"res\":200,\"msg\":\"Sequence loaded.\"}";

            } else if (cmd == "closeSequence") {
                if (CurrentSeqXmlFile == nullptr) {
                    if (val["quiet"].AsString() != "true") {
                        return "{\"res\":504,\"msg\":\"No sequence open.\"}";
                    }
                    return "{\"res\":200,\"msg\":\"Sequence closed.\"}";
                }

                auto force = val["force"].AsString() == "true";

                if (mSavedChangeCount != _sequenceElements.GetChangeCount()) {
                    if (force) {
                        mSavedChangeCount = _sequenceElements.GetChangeCount();
                    } else {
                        return "{\"res\":504,\"msg\":\"Sequence has unsaved changes.\"}";
                    }
                }

                AskCloseSequence();
                return "{\"res\":200,\"msg\":\"Sequence closed.\"}";

            } else if (cmd == "closexLights") {
                auto force = val["force"].AsString() == "true";

                if (CurrentSeqXmlFile != nullptr && mSavedChangeCount != _sequenceElements.GetChangeCount()) {
                    if (force) {
                        mSavedChangeCount = _sequenceElements.GetChangeCount();
                    } else {
                        return "{\"res\":504,\"msg\":\"Sequence has unsaved changes.\"}";
                    }
                }

                if (UnsavedRgbEffectsChanges) {
                    if (force) {
                        UnsavedRgbEffectsChanges = false;
                    } else {
                        return "{\"res\":504,\"msg\":\"Layout has unsaved changes.\"}";
                    }
                }

                if (UnsavedNetworkChanges) {
                    if (force) {
                        UnsavedNetworkChanges = false;
                    } else {
                        return "{\"res\":504,\"msg\":\"Controller has unsaved changes.\"}";
                    }
                }

                wxCloseEvent evt;
                wxPostEvent(this, evt);

                return "{\"res\":200,\"msg\":\"xLights closed.\"}";

            } else if (cmd == "newSequence") {
                if (CurrentSeqXmlFile != nullptr && val["force"].AsString() != "true") {
                    return "{\"res\":504,\"msg\":\"Sequence already open.\"}";
                }

                auto media = val["mediaFile"].AsString();
                if (media == "null")
                    media = "";
                auto duration = val["durationSecs"].AsLong() * 1000;

                NewSequence(media, duration);
                EnableSequenceControls(true);
                return "{\"res\":200,\"msg\":\"Sequence created.\"}";

            } else if (cmd == "saveSequence") {
                if (CurrentSeqXmlFile == nullptr) {
                    return "{\"res\":504,\"msg\":\"No sequence open.\"}";
                }

                auto seq = val["seq"].AsString();

                if (seq != "" && seq != "null") {
                    SaveAsSequence(seq);
                } else {
                    if (xlightsFilename.IsEmpty()) {
                        return "{\"res\":504,\"msg\":\"Saving unnamed sequence needs a name to be sent.\"}";
                    }
                    SaveSequence();
                }

                return "{\"res\":200,\"msg\":\"Sequence saved.\"}";

            } else if (cmd == "batchRender") {
                wxArrayString files;
                auto seqs = val["seqs"].AsArray();
                for (size_t i = 0; i < seqs->Count(); i++) {
                    auto seq = seqs->Item(i).AsString();
                    seq = FindSequence(seq);
                    if (seq == "") {
                        return wxString::Format("{\"res\":504,\"msg\":\"Sequence not found '%s'.\"}", seqs->Item(i).AsString());
                    }
                    files.push_back(seq);
                }

                auto oldPrompt = _promptBatchRenderIssues;
                _promptBatchRenderIssues = (val["promptIssues"].AsString() != "true"); // off by default

                _renderMode = true;
                OpenRenderAndSaveSequences(files, false);

                while (_renderMode) {
                    wxYield();
                }

                _promptBatchRenderIssues = oldPrompt;

                return "{\"res\":200,\"msg\":\"Sequence batch rendered.\"}";

            } else if (cmd == "uploadController") {
                auto ip = val["ip"].AsString();
                Controller* c = _outputManager.GetControllerWithIP(ip);
                if (c == nullptr) {
                    return wxString::Format("{\"res\":504,\"msg\":\"Controller not found '%s'.\"}", ip);
                }

                // ensure all start channels etc are up to date
                RecalcModels();

                bool res = true;
                auto caps = GetControllerCaps(c->GetName());
                if (caps != nullptr) {
                    wxString message;
                    if (caps->SupportsInputOnlyUpload()) {
                        res = res && UploadInputToController(c, message);
                    }
                    res = res && UploadOutputToController(c, message);
                } else {
                    res = false;
                }
                if (res) {
                    return wxString::Format("{\"res\":200,\"msg\":\"Uploaded to controller %s.\"}", ip);
                }
                return wxString::Format("{\"res\":504,\"msg\":\"Upload to controller %s failed.\"}", ip);

            } else if (cmd == "uploadFPPConfig") {
                auto ip = val["ip"].AsString();
                auto udp = val["udp"].AsString();
                auto models = val["models"].AsString();
                auto map = val["displayMap"].AsString();

                // discover the FPP instances
                auto instances = FPP::GetInstances(this, &_outputManager);

                FPP* fpp = nullptr;
                for (const auto& it : instances) {
                    if (it->ipAddress == ip && it->fppType == FPP_TYPE::FPP) {
                        fpp = it;
                        break;
                    }
                }
                if (fpp == nullptr) {
                    return wxString::Format("{\"res\":504,\"msg\":\"FPP %s not found.\"}", ip);
                }

                std::map<int, int> udpRanges;
                wxJSONValue outputs = FPP::CreateUniverseFile(_outputManager.GetControllers(), false, &udpRanges);

                if (udp == "all") {
                    fpp->UploadUDPOut(outputs);
                    fpp->SetRestartFlag();
                } else if (udp == "proxy") {
                    fpp->UploadUDPOutputsForProxy(&_outputManager);
                    fpp->SetRestartFlag();
                }

                if (models == "true") {
                    wxJSONValue memoryMaps = FPP::CreateModelMemoryMap(&AllModels);
                    fpp->UploadModels(memoryMaps);
                }

                if (map == "true") {
                    std::string displayMap = FPP::CreateVirtualDisplayMap(&AllModels, GetDisplay2DCenter0());
                    fpp->UploadDisplayMap(displayMap);
                    fpp->SetRestartFlag();
                }

                //if restart flag is now set, restart and recheck range
                fpp->Restart("", true);

                return wxString::Format("{\"res\":200,\"msg\":\"Uploaded to FPP %s.\"}", ip);

            } else if (cmd == "uploadSequence") {
                bool res = true;
                auto ip = val["ip"].AsString();
                auto media = (val["media"].AsString() == "true");
                auto format = val["format"].AsString();
                auto xsq = val["seq"].AsString();
                xsq = FindSequence(xsq);

                if (xsq == "") {
                    return "{\"res\":504,\"msg\":\"Sequence not found.\"}";
                }

                auto fseq = xLightsXmlFile::GetFSEQForXSQ(xsq, GetFseqDirectory());
                auto m2 = xLightsXmlFile::GetMediaForXSQ(xsq, CurrentDir, GetMediaFolders());

                if (!wxFile::Exists(fseq)) {
                    return "{\"res\":504,\"msg\":\"Unable to find sequence FSEQ file.\"}";
                }

                // discover the FPP instances
                auto instances = FPP::GetInstances(this, &_outputManager);

                FPP* fpp = nullptr;
                for (const auto& it : instances) {
                    if (it->ipAddress == ip) {
                        fpp = it;
                        break;
                    }
                }
                if (fpp == nullptr) {
                    return wxString::Format("{\"res\":504,\"msg\":\"Player %s not found.\"}", ip);
                }

                int fseqType = 0;
                if (format == "v1") {
                    fseqType = 0;
                } else if (format == "v2std") {
                    fseqType = 1;
                } else if (format == "v2zlib") {
                    fseqType = 5;
                } else if (format == "v2uncompressedsparse") {
                    fseqType = 3;
                } else if (format == "v2uncompressed") {
                    fseqType = 4;
                } else if (format == "v2stdsparse") {
                    fseqType = 2;
                } else if (format == "v2zlibsparse") {
                    fseqType = 6;
                }

                if (!media) {
                    m2 = "";
                }

                FSEQFile* seq = FSEQFile::openFSEQFile(fseq);
                if (seq) {
                    fpp->PrepareUploadSequence(*seq, fseq, m2, fseqType);
                    static const int FRAMES_TO_BUFFER = 50;
                    std::vector<std::vector<uint8_t>> frames(FRAMES_TO_BUFFER);
                    for (size_t x = 0; x < frames.size(); x++) {
                        frames[x].resize(seq->getMaxChannel() + 1);
                    }

                    for (size_t frame = 0; frame < seq->getNumFrames(); frame++) {
                        int lastBuffered = 0;
                        size_t startFrame = frame;
                        //Read a bunch of frames so each parallel thread has more info to work with before returning out here
                        while (lastBuffered < FRAMES_TO_BUFFER && frame < seq->getNumFrames()) {
                            FSEQFile::FrameData* f = seq->getFrame(frame);
                            if (f != nullptr) {
                                if (!f->readFrame(&frames[lastBuffered][0], frames[lastBuffered].size())) {
                                    //logger_base.error("FPPConnect FSEQ file corrupt.");
                                    res = false;
                                }
                                delete f;
                            }
                            lastBuffered++;
                            frame++;
                        }
                        frame--;
                        for (int x = 0; x < lastBuffered; x++) {
                            fpp->AddFrameToUpload(startFrame + x, &frames[x][0]);
                        }
                    }
                    fpp->FinalizeUploadSequence();

                    if (fpp->fppType == FPP_TYPE::FALCONV4) {
                        // a falcon
                        std::string proxy = "";
                        auto c = _outputManager.GetControllers(fpp->ipAddress);
                        if (c.size() == 1)
                            proxy = c.front()->GetFPPProxy();
                        Falcon falcon(fpp->ipAddress, proxy);

                        if (falcon.IsConnected()) {
                            falcon.UploadSequence(fpp->GetTempFile(), fseq, fpp->mode == "remote" ? "" : m2, nullptr);
                        } else {
                            res = false;
                        }
                        fpp->ClearTempFile();
                    }
                    delete seq;
                } else {
                    return "{\"res\":504,\"msg\":\"Failed to generate FSEQ.\"}";
                }

                if (!res) {
                    return "{\"res\":504,\"msg\":\"Failed to upload.\"}";
                }

                return "{\"res\":200,\"msg\":\"Sequence uploaded.\"}";

            } else if (cmd == "checkSequence") {
                auto seq = val["seq"].AsString();
                seq = FindSequence(seq);
                if (seq == "") {
                    return "{\"res\":504,\"msg\":\"Sequence not found.\"}";
                }
                auto file = OpenAndCheckSequence(seq.ToStdString());

                return wxString::Format("{\"res\":200,\"msg\":\"Sequence checked.\",\"output\":\"%s\"}", JSONSafe(file));

            } else if (cmd == "changeShowFolder") {
                auto shw = val["folder"].AsString();
                if (!wxDir::Exists(shw)) {
                    return wxString::Format("{\"res\":504,\"msg\":\"Folder %s does not exist.\"}", shw);
                }

                auto force = val["force"].AsString() == "true";

                if (CurrentSeqXmlFile != nullptr && mSavedChangeCount != _sequenceElements.GetChangeCount()) {
                    if (force) {
                        mSavedChangeCount = _sequenceElements.GetChangeCount();
                    } else {
                        return "{\"res\":504,\"msg\":\"Sequence has unsaved changes.\"}";
                    }
                }

                if (UnsavedRgbEffectsChanges) {
                    if (force) {
                        UnsavedRgbEffectsChanges = false;
                    } else {
                        return "{\"res\":504,\"msg\":\"Layout has unsaved changes.\"}";
                    }
                }

                if (UnsavedNetworkChanges) {
                    if (force) {
                        UnsavedNetworkChanges = false;
                    } else {
                        return "{\"res\":504,\"msg\":\"Controller has unsaved changes.\"}";
                    }
                }

                displayElementsPanel->SetSequenceElementsModelsViews(nullptr, nullptr, nullptr, nullptr, nullptr);
                layoutPanel->ClearUndo();
                SetDir(shw, true);

                return wxString::Format("{\"res\":200,\"msg\":\"Show folder changed to %s.\"}", shw);

            } else if (cmd == "openController") {
                auto ip = val["ip"].AsString();
                ::wxLaunchDefaultBrowser(ip);

                return "{\"res\":200,\"msg\":\"Controller opened.\"}";

            } else if (cmd == "openControllerProxy") {
                auto ip = val["ip"].AsString();
                auto controller = _outputManager.GetControllerWithIP(ip);

                if (controller == nullptr) {
                    return "{\"res\":504,\"msg\":\"Controller not found.\"}";
                }

                auto proxy = controller->GetFPPProxy();

                if (proxy == "") {
                    return "{\"res\":504,\"msg\":\"Controller has no proxy.\"}";
                }

                ::wxLaunchDefaultBrowser(proxy);

                return "{\"res\":200,\"msg\":\"Proxy opened.\"}";

            } else if (cmd == "runDiscovery") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "exportModel") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
                // pass in name of the file to write to ... pass back the name of the file written to
            } else if (cmd == "exportModelAsCustom") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
                // pass in name of the file to write to ... pass back the name of the file written to
            } else if (cmd == "exportModelsCSV") {
                auto filename = val["filename"].AsString();

                if (filename == "" || filename == "null") {
                    wxFileName f;
                    f.AssignTempFileName("Models_");
                    filename = f.GetFullPath();
                }

                ExportModels(filename);

                return wxString::Format("{\"res\":200,\"msg\":\"Models Exported.\",\"output\":\"%s\"}", JSONSafe(filename));

            } else if (cmd == "shiftAllEffects") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
                // pass in number of MS
            } else if (cmd == "shiftSelectedEffects") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "unselectEffects") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "selectEffectsOnModel") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "selectAllEffectsOnAllModels") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "exportModel") {
                if (CurrentSeqXmlFile == nullptr) {
                    return "{\"res\":504,\"msg\":\"Sequence not open.\"}";
                }

                auto model = val["model"].AsString();
                if (AllModels.GetModel(model) == nullptr) {
                    return "{\"res\":504,\"msg\":\"Unknown model.\"}";
                }

                auto filename = val["filename"].AsString();
                auto format = val["format"].AsString();

                if (format == "lsp") {
                    format = "LSP";
                } else if (format == "lorclipboard") {
                    format = "Lcb";
                } else if (format == "lorclipboards5") {
                    format = "LcbS5";
                } else if (format == "vixenroutine") {
                    format = "Vir";
                } else if (format == "hls") {
                    format = "HLS";
                } else if (format == "eseq") {
                    format = "FPP";
                } else if (format == "eseqcompressed") {
                    format = "FPPCompressed";
                } else if (format == "avicompressed") {
                    format = "Com";
                } else if (format == "aviuncompressed") {
                    format = "Unc";
                } else if (format == "minleon") {
                    format = "Min";
                } else if (format == "gif") {
                    format = "GIF";
                } else {
                    return "{\"res\":504,\"msg\":\"Unknown format.\"}";
                }

                if (DoExportModel(0, 0, model, filename, format, false)) {
                    return "{\"res\":200,\"msg\":\"Model exported.\"}";

                } else {
                    return "{\"res\":504,\"msg\":\"Failed to export.\"}";
                }

            } else if (cmd == "turnOnOutputToLights") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "turnOffOutputToLights") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "playSequence") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "printLayout") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "printWiring") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "exportLayoutImage") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "exportWiringImage") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "cleanupFileLocations") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "hinksPixExport") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "packageLogFiles") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // MEDIUM PRIORITY
                // TODO
            } else if (cmd == "packageSequence") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // MEDIUM PRIORITY
                // TODO
            } else if (cmd == "purgeDownloadCache") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // MEDIUM PRIORITY
                // TODO
            } else if (cmd == "purgeRenderCache") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // MEDIUM PRIORITY
                // TODO
            } else if (cmd == "convertSequence") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "prepareAudio") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "resetToDefaults") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // MEDIUM PRIORITY
                // TODO
            } else if (cmd == "resetWindowLayout") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // MEDIUM PRIORITY
                // TODO
            } else if (cmd == "setAudioVolume") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "setAudioSpeed") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "gotoZoom") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "importSequence") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "getVersion") {
                return wxString::Format("{\"res\":200,\"version\":\"%s\"}", GetDisplayVersionString());
            } else if (cmd == "lightsOn") {
                //logger_base.debug("xFade turning lights on.");
                EnableOutputs(true);
                return wxString::Format("{\"res\":200,\"msg\":\"Lights on\"}");
            } else if (cmd == "lightsOff") {
                //logger_base.debug("xFade turning lights off.");
                DisableOutputs();
                return wxString::Format("{\"res\":200,\"msg\":\"Lights off\"}");
            } else if (cmd == "playJukebox") {
                int button = val["button"].AsLong();
                //logger_base.debug("xFade playing jukebox button %d.", button);

                if (CurrentSeqXmlFile != nullptr) {
                    jukeboxPanel->PlayItem(button);
                    return wxString::Format("{\"res\":200,\"msg\":\"Played button %d\"}", button);
                } else {
                    //logger_base.error("    Error - sequence not open.");
                    return "{\"res\":504,\"msg\":\"Sequence not open.\"}";
                }
            } else if (cmd == "getJukeboxButtonTooltips") {
                if (CurrentSeqXmlFile != nullptr) {
                    return "{\"res\":200, \"tooltips\":" + jukeboxPanel->GetTooltipsJSON() + "}";
                } else {
                    //logger_base.error("    Error - sequence not open.");
                    return "{\"res\":504,\"msg\":\"Sequence not open.\"}";
                }
            } else if (cmd == "getJukeboxButtonEffectPresent") {
                if (CurrentSeqXmlFile != nullptr) {
                    return "{\"res\":200, \"effects\":" + jukeboxPanel->GetEffectPresentJSON() + "}";
                } else {
                    //logger_base.error("    Error - sequence not open.");
                    return "{\"res\":504,\"msg\":\"Sequence not open.\"}";
                }
            } else if (cmd == "getOpenSequence") {
                //logger_base.debug("xFade getting sequence name.");

                if (CurrentSeqXmlFile != nullptr) {
                    return "{\"res\":200, \"seq\":\"" + CurrentSeqXmlFile->GetName() + "\"}";
                } else {
                    //logger_base.error("    Error - sequence not open.");
                    return "{\"res\":504,\"msg\":\"Sequence not open.\"}";
                }
            } else if (cmd == "getE131Tag") {
                //logger_base.debug("xFade getting E1.31 tag.");
                return "{\"res\":200, \"tag\":\"" + E131Output::GetTag() + "\"}";
            } else {
                return wxString::Format("{\"res\":504,\"msg\":\"Unknown command: '%s'.\"}", cmd);
            }
        }
    } else {
        return "{\"res\":504,\"msg\":\"Error parsing request.\"}";
    }
}
