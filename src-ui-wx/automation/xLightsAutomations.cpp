/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "../xLightsMain.h"
#include "xLightsVersion.h"

#include "nlohmann/json.hpp"

#include "render/FSEQFile.h"
#include "render/SeqMediaMigration.h"
#include "media/MediaCompatibility.h"
#include "outputs/Controller.h"
#include "outputs/ControllerEthernet.h"
#include "layout/LayoutPanel.h"
#include "layout/ViewsModelsPanel.h"
#include "controllers/ControllerCaps.h"
#include "controllers/FPP.h"
#include "controllers/Falcon.h"
#include "utils/ip_utils.h"
#include "UtilFunctions.h"
#include "utils/ExternalHooks.h"
#include "shared/utils/wxUtilities.h"
#include "../xLightsApp.h"
#include "media/JukeboxPanel.h"
#include "outputs/E131Output.h"
#include "../../dependencies/wxHTTPServer/wxhttpserver.h"
#include "../sequencer/MainSequencer.h"
#include "../layout/ModelPreview.h"
#include <wx/uri.h>

#include "LuaRunner.h"
#include "setup/DiscoveryAuthDialog.h"

#include <log.h>

std::string xLightsFrame::FindSequence(const std::string& seq)
{
    if (FileExists(seq)) {
        std::error_code ec;
        auto abs = std::filesystem::absolute(seq, ec);
        return (!ec && !abs.empty()) ? abs.string() : seq;
    }

    if (FileExists(CurrentDir + wxFileName::GetPathSeparator() + seq))
        return CurrentDir + wxFileName::GetPathSeparator() + seq;

    return "";
}
static const char HTTP_ERROR_PAGE[] = "Could not process xLights Automation";
static bool HttpRequestFunction(HttpConnection &connection, HttpRequest &request) {
    return xLightsApp::__frame->ProcessHttpRequest(connection, request);
}

static wxString MIME_JSON = "application/json";
static wxString MIME_TEXT = "text/plain";

static std::map<std::string, std::string> ParseParams(const wxString &params) {
    std::map<std::string, std::string> p;
    std::string np = params;
    while (!np.empty()) {
        std::string np2 = np;
        size_t idx = np2.find('&');
        if (idx != std::string::npos) {
            np = np2.substr(idx + 1);
            np2 = np2.substr(0, idx);
        } else {
            np = "";
        }
        idx = np2.find('=');
        std::string value = "";
        if (idx != std::string::npos) {
            value = np2.substr(idx + 1);
            np2 = np2.substr(0, idx);
        }
        p[np2] = wxURI::Unescape(value);
    }
    return p;
}
inline bool ReadBool(const nlohmann::json& v) {
    if (v.is_boolean()) {
        return v.get<bool>();
    }
    if (v.is_number_integer()) {
        return v.get<int>() != 0;
    }
    return v.get<std::string>() == "true" || v.get<std::string>() == "1";
}
inline bool ReadBool(const std::string &v) {
    return v == "true" || v == "1";
}

static const char* MediaTypeName(MediaType t) {
    switch (t) {
    case MediaType::Image:      return "image";
    case MediaType::SVG:        return "svg";
    case MediaType::Shader:     return "shader";
    case MediaType::TextFile:   return "text";
    case MediaType::BinaryFile: return "binary";
    case MediaType::Video:      return "video";
    case MediaType::Audio:      return "audio";
    }
    return "unknown";
}

static std::optional<MediaType> MediaTypeFromName(const std::string& n) {
    if (n == "image") return MediaType::Image;
    if (n == "svg") return MediaType::SVG;
    if (n == "shader") return MediaType::Shader;
    if (n == "text") return MediaType::TextFile;
    if (n == "binary") return MediaType::BinaryFile;
    if (n == "video") return MediaType::Video;
    if (n == "audio") return MediaType::Audio;
    return std::nullopt;
}

static std::shared_ptr<MediaCacheEntry> LookupMediaEntry(SequenceMedia& media, const std::string& path, MediaType type) {
    switch (type) {
    case MediaType::Image:      return media.GetImage(path);
    case MediaType::SVG:        return media.GetSVG(path);
    case MediaType::Shader:     return media.GetShader(path);
    case MediaType::TextFile:   return media.GetTextFile(path);
    case MediaType::BinaryFile: return media.GetBinaryFile(path);
    case MediaType::Video:      return media.GetVideo(path);
    case MediaType::Audio:      return media.GetAudio(path);
    }
    return nullptr;
}


bool xLightsFrame::ProcessAutomation(std::vector<std::string> &paths,
                                     std::map<std::string, std::string> &params,
                                     const std::function<bool(const std::string &msg,
                                                              const std::string &jsonKey,
                                                              int responseCode,
                                                              bool msgIsJSON)> &sendResponse) {

    if (paths.size() == 0) {
        return sendResponse("No command", "msg", 503, false);
    }

    std::string cmd = paths[0];
    if (cmd == "getVersion") {
        return sendResponse(GetDisplayVersionString(), "version", 200, false);
    } else if (cmd == "openSequence" || cmd == "getOpenSequence" || cmd == "loadSequence") {
        wxString fname = "";
        if (paths.size() > 1) {
            fname = wxURI::Unescape(paths[1]);
        }
        bool force = false;
        bool prompt = false;
        
        if (params["_METHOD"] == "POST" && !params["_DATA"].empty()) {
            wxString data = params["_DATA"];
            try {
                nlohmann::json val = nlohmann::json::parse(data.ToStdString());
                // wxJSONReader reader;
                //  if (reader.Parse(data, &val) == 0)
                {
                    fname = val["seq"].get<std::string>();
                    if (val.contains("promptIssues")) {
                        prompt = ReadBool(params["promptIssues"]);
                    }
                    if (val.contains("force")) {
                        force = ReadBool(params["force"]);
                    }
                }
            } catch (const std::exception& e) {
                return sendResponse(wxString::Format("Failed to parse JSON data: %s", e.what()), "msg", 503, false);
            }
        } else {
            if (params["seq"] != "") {
                fname = params["seq"];
            }
            prompt = ReadBool(params["promptIssues"]);
            force = ReadBool(params["force"]);
        }
        if (fname.empty()) {
            if (CurrentSeqXmlFile != nullptr) {
                nlohmann::json j;
                j["seq"] = CurrentSeqXmlFile->GetName();
                j["fullseq"] = CurrentSeqXmlFile->GetFullPath();
                j["media"] = CurrentSeqXmlFile->GetMediaFile();
                j["len"] = CurrentSeqXmlFile->GetSequenceDurationMS();
                j["framems"] = CurrentSeqXmlFile->GetFrameMS();
                return sendResponse(j.dump(), "", 200, true);
            } else {
                return sendResponse("Sequence not open.", "msg", 503, false);
            }
        } else {
            std::string seq = FindSequence(fname);
            if (seq.empty()) {
                return sendResponse("Sequence not found.", "msg", 503, false);
            }
            if (CurrentSeqXmlFile != nullptr && force) {
                return sendResponse("Sequence already open.", "msg", 503, false);
            }
            auto oldPrompt = _promptBatchRenderIssues;
            auto oldRenderMode = _renderMode;
            if (!prompt) _renderMode = true;
            _promptBatchRenderIssues = prompt; // off by default
            OpenSequence(seq, nullptr);
            _promptBatchRenderIssues = oldPrompt;
            _renderMode = oldRenderMode;
            nlohmann::json j;
            j["seq"] = CurrentSeqXmlFile->GetName();
            j["fullseq"] = CurrentSeqXmlFile->GetFullPath();
            j["media"] = CurrentSeqXmlFile->GetMediaFile();
            j["len"] = CurrentSeqXmlFile->GetSequenceDurationMS();
            j["framems"] = CurrentSeqXmlFile->GetFrameMS();
            return sendResponse(j.dump(), "", 200, true);
        }
    } else if (cmd == "closeSequence") {
        if (CurrentSeqXmlFile == nullptr) {
            if (!ReadBool(params["quiet"])) {
                return sendResponse("Sequence not open.", "msg", 503, false);
            }
            return sendResponse("Sequence closed.", "msg", 200, false);
        }

        auto force = ReadBool(params["force"]);
        if (mSavedChangeCount != _sequenceElements.GetChangeCount()) {
            if (force) {
                mSavedChangeCount = _sequenceElements.GetChangeCount();
            } else {
                return sendResponse("Sequence has unsaved changes.", "msg", 504, false);
            }
        }

        AskCloseSequence();
        return sendResponse("Sequence closed.", "msg", 200, false);
    } else if (cmd == "saveLayout") {
        if (!layoutPanel->SaveEffects()) {
            return sendResponse("Failed to save layout.", "msg", 503, false);
        }

        if (!SaveNetworksFile()) {
            return sendResponse("Failed to controller tab.", "msg", 503, false);
        }

        return sendResponse("Layout and controller tab saved.", "msg", 200, false);

    } else if (cmd == "newSequence") {
        if (CurrentSeqXmlFile != nullptr && !ReadBool(params["force"])) {
            return sendResponse("Sequence already open.", "msg", 503, false);
        }

        auto media = params["mediaFile"];
        if (media == "null")
            media = "";
        auto duration = wxAtoi(params["durationSecs"]) * 1000;

        uint32_t frameMS = wxAtoi(params["frameMS"]); // this will be 0 if "null" so ok

        std::string view = params["view"];
        if (view == "null")
            view = "";

        NewSequence(media, duration, frameMS, view);
        EnableSequenceControls(true);
        return sendResponse("Sequence created.", "msg", 200, false);
    } else if (cmd == "saveSequence") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("No sequence open.", "msg", 503, false);
        }
        auto seq = params["seq"];

        if (seq != "" && seq != "null") {
            SaveAsSequence(seq);
        } else {
            if (xlightsFilename.IsEmpty()) {
                return sendResponse("Saving unnamed sequence needs a name to be sent.", "msg", 503, false);
            }
            SaveSequence();
        }
        return sendResponse("Sequence Saved.", "msg", 200, false);
    } else if (cmd == "renderAll") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("No sequence open.", "msg", 503, false);
        }
        auto ld = _lowDefinitionRender;
        auto highdef = params["highdef"];
        if (highdef == "true" && _lowDefinitionRender) {
            // override definition
            _lowDefinitionRender = false;
            _outputModelManager.AddImmediateWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Automation::renderAll");
            _outputModelManager.AddImmediateWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Automation::renderAll");
        }
        RenderAll();
        while (mRendering) {
            wxYield();
        }
        if (ld != _lowDefinitionRender) {
            _lowDefinitionRender = ld;
            _outputModelManager.AddImmediateWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Automation::renderAll");
            _outputModelManager.AddImmediateWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Automation::renderAll");
        }
        return sendResponse("Rendered.", "msg", 200, false);
    } else if (cmd == "batchRender") {
        wxArrayString files;

        auto ld = _lowDefinitionRender;
        auto highdef = params["highdef"];
        if (highdef == "true" && _lowDefinitionRender) {
            // override definition
            _lowDefinitionRender = false;
            _outputModelManager.AddImmediateWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Automation::batchRender");
            _outputModelManager.AddImmediateWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Automation::batchRender");
        }

        auto seqs = params["seqs_0"];
        int snum = 0;
        while (seqs != "") {
            auto seq = FindSequence(seqs);
            if (seq.empty()) {
                return sendResponse("Sequence not found '" + seq + "'", "msg", 503, false);
            }
            files.push_back(seq);
            snum++;
            seqs = params["seqs_" + std::to_string(snum)];
        }
        auto oldPrompt = _promptBatchRenderIssues;
        _promptBatchRenderIssues = ReadBool(params["promptIssues"]);

        _renderMode = true;
        _saveLowDefinitionRender = _lowDefinitionRender;
        OpenRenderAndSaveSequences(files, false);

        while (_renderMode) {
            wxYield();
        }

        _promptBatchRenderIssues = oldPrompt;
        if (ld != _lowDefinitionRender) {
            _lowDefinitionRender = ld;
            _outputModelManager.AddImmediateWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Automation::batchRender");
            _outputModelManager.AddImmediateWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Automation::batchRender");
        }
        return sendResponse("Sequence batch rendered.", "msg", 200, false);
    } else if (cmd == "uploadController") {
        auto ip = params["ip"];
        Controller* c = _outputManager.GetControllerWithIP(ip);
        if (c == nullptr) {
            return sendResponse("Controller not found '" + ip + "'", "msg", 503, false);
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
            return sendResponse("Uploaded to controller '" + ip + "'", "msg", 200, false);
        }
        return sendResponse("Upload to controller '" + ip + "' failed.", "msg", 503, false);
    } else if (cmd == "uploadFPPConfig") {
        auto ip = params["ip"];
        auto udp = params["udp"];
        auto models = params["models"];
        auto map = params["displayMap"];

        // discover the FPP instances
        wxDiscoveryDelegate delegate(nullptr);
        auto instances = DiscoverFPPInstances(&delegate);

        FPP* fpp = nullptr;
        std::string resolvedIp = ip_utils::ResolveIP(ip);
        for (const auto& it : instances) {
            if (it->fppType == FPP_TYPE::FPP &&
                (it->ipAddress == ip || (!resolvedIp.empty() && it->ipAddress == resolvedIp))) {
                fpp = it;
                break;
            }
        }
        if (fpp == nullptr) {
            return sendResponse("FPP not found '" + ip + "'.", "msg", 503, false);
        }

        if (udp == "all") {
            std::map<int, int> udpRanges;
            auto outputs = fpp->CreateUniverseFile(_outputManager.GetControllers(), false, &udpRanges);
            fpp->UploadUDPOut(outputs);
            fpp->SetRestartFlag();
        } else if (udp == "proxy") {
            fpp->UploadUDPOutputsForProxy(&_outputManager);
            fpp->SetRestartFlag();
        }

        if (models == "true" || models == "all") {
            auto memoryMaps = fpp->CreateModelMemoryMap(&AllModels, 0, std::numeric_limits<int32_t>::max());
            fpp->UploadModels(memoryMaps);
        } else if (udp == "local") {
            auto c = _outputManager.GetControllers(fpp->ipAddress);
            if (c.size() == 1) {
                auto const& memoryMaps = fpp->CreateModelMemoryMap(&AllModels, c.front()->GetStartChannel(), c.front()->GetEndChannel());
                fpp->UploadModels(memoryMaps);
            }
        }

        if (map == "true") {
            int pw, ph;
            GetLayoutPreview()->GetVirtualCanvasSize(pw, ph);
            std::map<std::string, std::string> virtualDisplayData;
            FPP::CreateVirtualDisplayMap(AllModels, AllObjects, pw, ph, virtualDisplayData);
            fpp->UploadDisplayMap(virtualDisplayData);
            // virtual display map  requires a restart
            fpp->SetRestartFlag(true);
        }

        //if restart flag is now set, restart and recheck range
        fpp->Restart(true);

        return sendResponse("Uploaded to FPP '" + ip + "'.", "msg", 200, false);
    } else if (cmd == "uploadSequence") {
        bool res = true;
        auto ip = params["ip"];
        auto media = ReadBool(params["media"]);
        auto format = params["format"];
        auto xsq = params["seq"];
        xsq = FindSequence(xsq);

        if (xsq.empty()) {
            return sendResponse("Sequence not found.", "msg", 503, false);
        }

        auto fseq = SequenceFile::GetFSEQForXSQ(xsq, GetFseqDirectory());
        auto m2 = SequenceFile::GetMediaForXSQ(xsq, CurrentDir, GetMediaFolders());

        if (!FileExists(fseq)) {
            return sendResponse("Unable to find sequence FSEQ file.", "msg", 503, false);
        }

        // discover the FPP instances
        wxDiscoveryDelegate delegate(nullptr);
        auto instances = DiscoverFPPInstances(&delegate);

        FPP* fpp = nullptr;
        std::string resolvedSequenceIp = ip_utils::ResolveIP(ip);
        for (const auto& it : instances) {
            if (it->ipAddress == ip || (!resolvedSequenceIp.empty() && it->ipAddress == resolvedSequenceIp)) {
                fpp = it;
                break;
            }
        }
        if (fpp == nullptr) {
            return sendResponse("Player " + ip + " not found.", "msg", 503, false);
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
            // every frame is read in order below to build the upload
            seq->setReadPattern(FSEQFile::ReadPattern::Bulk);
            fpp->PrepareUploadSequence(seq, fseq, m2, fseqType);
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
                            //spdlog::error("FPPConnect FSEQ file corrupt.");
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

            if (fpp->fppType == FPP_TYPE::FALCONV4V5) {
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
            return sendResponse("Failed to generate FSEQ.", "msg", 503, false);
        }

        if (!res) {
            return sendResponse("Failed to upload.", "msg", 503, false);
        }
        return sendResponse("Sequence uploaded.", "msg", 200, false);
    } else if (cmd == "checkSequence") {
        auto seq = params["seq"];
        seq = FindSequence(seq);
        if (seq.empty()) {
            return sendResponse("Sequence not found.", "msg", 503, false);
        }
        auto file = OpenAndCheckSequence(seq);

        std::string response = wxString::Format("{\"msg\":\"Sequence checked.\",\"output\":\"%s\"}", JSONSafe(file));
        return sendResponse(response, "", 200, true);
    } else if (cmd == "changeShowFolder") {
        auto shw = params["folder"];
        if (!wxDir::Exists(shw)) {
            return sendResponse("Folder does not exist.", "msg", 503, false);
        }

        auto force = ReadBool(params["force"]);
        if (CurrentSeqXmlFile != nullptr && mSavedChangeCount != _sequenceElements.GetChangeCount()) {
            if (force) {
                mSavedChangeCount = _sequenceElements.GetChangeCount();
            } else {
                return sendResponse("Sequence has unsaved changes.", "msg", 503, false);
            }
        }

        if (UnsavedRgbEffectsChanges) {
            if (force) {
                UnsavedRgbEffectsChanges = false;
            } else {
                return sendResponse("Layout has unsaved changes.", "msg", 503, false);
            }
        }

        if (UnsavedNetworkChanges) {
            if (force) {
                UnsavedNetworkChanges = false;
            } else {
                return sendResponse("Controller has unsaved changes.", "msg", 503, false);
            }
        }

        displayElementsPanel->SetSequenceElementsModelsViews(nullptr, nullptr, nullptr);
        layoutPanel->ClearUndo();
        SetDir(shw, true);

        return sendResponse("Show folder changed to " + shw + ".", "msg", 200, false);
    } else if (cmd == "openController") {
        auto ip = params["ip"];
        ::wxLaunchDefaultBrowser(ip);

        return sendResponse("Controller opened", "msg", 200, false);

    } else if (cmd == "openControllerProxy") {
        auto ip = params["ip"];
        auto controller = _outputManager.GetControllerWithIP(ip);

        if (controller == nullptr) {
            return "{\"res\":504,\"msg\":\"Controller not found.\"}";
        }

        auto proxy = controller->GetFPPProxy();

        if (proxy.empty()) {
            return "{\"res\":504,\"msg\":\"Controller has no proxy.\"}";
        }

        ::wxLaunchDefaultBrowser(proxy);

        return "{\"res\":200,\"msg\":\"Proxy opened.\"}";

    } else if (cmd == "exportModelsCSV") {
        auto filename = params["filename"];
        if (filename == "" || filename == "null") {
            wxFileName f;
            f.AssignTempFileName("Models_");
            filename = f.GetFullPath();
        }

        ExportModels(filename);

        std::string response = wxString::Format("{\"msg\":\"Models Exported.\",\"output\":\"%s\"}", JSONSafe(filename));
        return sendResponse(response, "", 200, true);
    } else if (cmd == "exportModel") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }

        auto model = params["model"];
        if (AllModels.GetModel(model) == nullptr) {
            return sendResponse("Unknown model.", "msg", 503, false);
        }

        auto filename = params["filename"];
        auto format = params["format"];

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
        } else if (format == "avicompressed" || format == "mp4compressed") {
            format = "Com";
        } else if (format == "mp4highquality") {
            format = "Hig";
        } else if (format == "aviuncompressed" || format == "mp4uncompressed") {
            format = "Unc";
        } else if (format == "losslessrgb") {
            format = "Los";
        } else if (format == "prores4444") {
            format = "Pro";
        } else if (format == "hdprores") {
            format = "HD ";
        } else if (format == "minleon") {
            format = "Min";
        } else if (format == "gif") {
            format = "GIF";
        } else {
            return sendResponse("Unknown format.", "msg", 503, false);
        }

        if (DoExportModel(0, 0, model, filename, format, false)) {
            return sendResponse("Model exported.", "msg", 200, false);
        } else {
            return sendResponse("Failed to export.", "msg", 503, false);
        }
    } else if (cmd == "exportModelWithRender") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }

        auto ld = _lowDefinitionRender;
        auto highdef = params["highdef"];
        auto model = params["model"];

        if (AllModels.GetModel(model) == nullptr) {
            return sendResponse("Unknown model.", "msg", 503, false);
        }

        if (highdef == "true" && _lowDefinitionRender) {
            // override definition
            _lowDefinitionRender = false;
            _outputModelManager.AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "Automation::exportModelWithRender");
            _outputModelManager.AddImmediateWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Automation::exportModelWithRender");
        }

        auto filename = params["filename"];
        auto format = params["format"];

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
        } else if (format == "avicompressed" || format == "mp4compressed") {
            format = "Com";
        } else if (format == "mp4highquality") {
            format = "Hig";
        } else if (format == "aviuncompressed" || format == "mp4uncompressed") {
            format = "Unc";
        } else if (format == "losslessrgb") {
            format = "Los";
        } else if (format == "prores4444") {
            format = "Pro";
        } else if (format == "hdprores") {
            format = "HD ";
        } else if (format == "minleon") {
            format = "Min";
        } else if (format == "gif") {
            format = "GIF";
        } else {
            return sendResponse("Unknown format.", "msg", 503, false);
        }

        if (DoExportModel(0, 0, model, filename, format, true)) {
            if (ld != _lowDefinitionRender) {
                _lowDefinitionRender = ld;
                _outputModelManager.AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "Automation::exportModelWithRender");  // Restore the models back to prior
                _outputModelManager.AddImmediateWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Automation::exportModelWithRender");
            }
            return sendResponse("Model exported.", "msg", 200, false);
        } else {
            if (ld != _lowDefinitionRender) {
                _lowDefinitionRender = ld;
                _outputModelManager.AddImmediateWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Automation::exportModelWithRender");
                _outputModelManager.AddImmediateWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Automation::exportModelWithRender");
            }
            return sendResponse("Failed to export.", "msg", 503, false);
        }
    } else if (cmd == "closexLights") {
        auto force = ReadBool(params["force"]);
        if (CurrentSeqXmlFile != nullptr && mSavedChangeCount != _sequenceElements.GetChangeCount()) {
            if (force) {
                mSavedChangeCount = _sequenceElements.GetChangeCount();
            } else {
                return sendResponse("Sequence has unsaved changes.", "msg", 503, false);
            }
        }

        if (UnsavedRgbEffectsChanges) {
            if (force) {
                UnsavedRgbEffectsChanges = false;
            } else {
                return sendResponse("Layout has unsaved changes.", "msg", 503, false);
            }
        }

        if (UnsavedNetworkChanges) {
            if (force) {
                UnsavedNetworkChanges = false;
            } else {
                return sendResponse("Controller has unsaved changes.", "msg", 503, false);
            }
        }

        // Click on the File quit menu item
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, wxID_EXIT);
        wxPostEvent(this, evt);

        return sendResponse("xLights closed.", "msg", 200, false);
    } else if (cmd == "lightsOn") {
        EnableOutputs(true);
        return sendResponse("Lights on.", "msg", 200, false);
    } else if (cmd == "lightsOff") {
        DisableOutputs();
        return sendResponse("Lights off.", "msg", 200, false);
    } else if (cmd == "playJukebox") {
        int button = wxAtoi(params["button"]);
        if (CurrentSeqXmlFile != nullptr) {
            jukeboxPanel->PlayItem(button);
            return sendResponse("Played button " + std::to_string(button), "msg", 200, false);
        } else {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
     } else if (cmd == "jukeboxButtonTooltips" || cmd == "getJukeboxButtonTooltips") {
        if (CurrentSeqXmlFile != nullptr) {
            return sendResponse(jukeboxPanel->GetTooltipsJSON(), "tooltips", 200, true);
        } else {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
    } else if (cmd == "jukeboxButtonEffectPresent" || cmd == "getJukeboxButtonEffectPresent") {
        if (CurrentSeqXmlFile != nullptr) {
            return sendResponse(jukeboxPanel->GetEffectPresentJSON(), "effects", 200, true);
        } else {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
    } else if (cmd == "e131Tag" || cmd == "getE131Tag") {
        return sendResponse(E131Output::GetTag(), "tag", 200, false);
    } else if (cmd == "addEthernetController") {
        auto c = new ControllerEthernet(&_outputManager);
        //c->SetProtocol(params["protocol"]);
        c->SetIP(params["ip"]);
        c->SetId(1);
        c->EnsureUniqueId();
        c->SetName(params["name"]);
        auto const vendors = ControllerCaps::GetVendors(c->GetType());
        if (std::find(vendors.begin(), vendors.end(), params["vendor"]) != vendors.end()) {
            c->SetVendor(params["vendor"]);
            auto models = ControllerCaps::GetModels(c->GetType(), params["vendor"]);
            if (std::find(models.begin(), models.end(), params["model"]) != models.end()) {
                c->SetModel(params["model"]);
                auto variants = ControllerCaps::GetVariants(c->GetType(), params["vendor"], params["model"]);
                if (std::find(variants.begin(), variants.end(), params["variant"]) != variants.end()) {
                    c->SetVariant(params["variant"]);
                }
            }
        }
        
        _outputManager.AddController(c);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Automation:ADDETHERNET");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "Automation:ADDETHERNET");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "Automation:ADDETHERNET", nullptr, c);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Automation:ADDETHERNET");
        return sendResponse("Added Ethernet Controller", "msg", 200, false);
    
    } else if (cmd == "packageSequence") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
        auto const filename = PackageSequence(false);
        std::string response = wxString::Format("{\"msg\":\"Sequence Packaged.\",\"output\":\"%s\"}", JSONSafe(filename));
        return sendResponse(response, "", 200, true);
    } else if (cmd == "packageLogFiles") {
        auto const filename = PackageDebugFiles(false);
        std::string response = wxString::Format("{\"msg\":\"Log Files Packaged.\",\"output\":\"%s\"}", JSONSafe(filename));
        return sendResponse(response, "", 200, true);

    } else if (cmd == "exportVideoPreview") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }

        auto filename = params["filename"];
        if (filename == "" || filename == "null") {
            filename = CurrentDir + wxFileName::GetPathSeparator() + CurrentSeqXmlFile->GetName() + ".mp4";
        }
        int width = params["width"].empty() ? -1 : (int)std::strtol(params["width"].c_str(), nullptr, 10);
        int height = params["height"].empty() ? -1 : (int)std::strtol(params["height"].c_str(), nullptr, 10);
        auto const worked = ExportVideoPreview(filename, width, height);
        if (worked) {
            std::string response = wxString::Format("{\"msg\":\"Export Video Preview.\",\"output\":\"%s\"}", JSONSafe(filename));
            return sendResponse(response, "", 200, true);
        }        
        return sendResponse("Export Video Preview Failed", "msg", 503, true);
    } else if (cmd == "runScript") {
        auto filename = params["filename"];
        if (filename.empty() || filename == "null" || !FileExists(filename)) {
            return sendResponse("Invalid Script Path.", "msg", 503, false);
        }

        LuaRunner runner(this);
        auto const worked = runner.Run_Script(filename, [](std::string const& m) {});
        if (worked) {
            std::string response = "{\"msg\":\"Script Was Successful.\"}";
            return sendResponse(response, "", 200, true);
        }
        return sendResponse("Script Failed", "msg", 503, true);
    } else if (cmd == "cloneModelEffects") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
        auto target = params["target"];
        auto source = params["source"];
        auto erase = false;

        if (!params["eraseModel"].empty()) {
            erase = ReadBool(params["eraseModel"]);
        }
        auto const worked = CloneXLightsEffects(target, source, _sequenceElements, erase);
        mainSequencer->PanelEffectGrid->Refresh();
        std::string response = wxString::Format("{\"msg\":\"Model Effects Cloned.\",\"worked\":\"%s\"}", JSONSafe(toStr(worked)));
        return sendResponse(response, "", 200, true);
    } else if (cmd == "addEffect") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
        auto target = params["target"];
        auto effect = params["effect"];
        auto settings = params["settings"];
        auto palette = params["palette"];
        Element* to = _sequenceElements.GetElement(target);
        int startTime = 0;
        int endTime = CurrentSeqXmlFile->GetSequenceDurationMS();
        int layer = 0;

        if (!params["layer"].empty()) {
            layer = (int)std::strtol(params["layer"].c_str(), nullptr, 10);
        }
        if (!params["startTime"].empty()) {
            startTime = (int)std::strtol(params["startTime"].c_str(), nullptr, 10);
        }
        if (!params["endTime"].empty()) {
            endTime = (int)std::strtol(params["endTime"].c_str(), nullptr, 10);
        }

        if (to == nullptr) {
            return sendResponse("target element doesn't exists.", "msg", 503, false);
        }
        _sequenceElements.get_undo_mgr().CreateUndoStep();
        while ((int)to->GetEffectLayerCount() < layer + 1) {
            to->AddEffectLayer();
        }
        auto valid = to->GetEffectLayer(layer)->AddEffect(0, effect, settings, palette,
                                                          startTime, endTime, 0, false);
        mainSequencer->PanelEffectGrid->Refresh();
        std::string response = wxString::Format("{\"msg\":\"Added Effects.\",\"worked\":\"%s\"}", JSONSafe(toStr(valid != nullptr)));
        return sendResponse(response, "", 200, true);
    } else if (cmd == "getModels") {
        std::string models;
        bool includeModels {true};
        bool includeGroups {true};
        auto sModels = params["models"];
        auto sGroups = params["groups"];
        includeModels = sModels != "false";
        includeGroups = sGroups != "false";
        for (auto m = (&AllModels)->begin(); m != (&AllModels)->end(); ++m) {
            if (m->second->GetDisplayAs() == DisplayAsType::ModelGroup && !includeGroups) {
                continue;
            }
            if (m->second->GetDisplayAs() != DisplayAsType::ModelGroup && !includeModels) {
                continue;
            }
            models += "\"" + JSONSafe(m->first) + "\",";
        }
        if (!models.empty()) {
            models.pop_back();//remove last comma
        }
        models = "[" + models + "]";
        return sendResponse(models, "models", 200, true);
        
    } else if (cmd == "deleteAllAliases") {
        std::string models;
        bool deleted = false;
        for (auto m = (&AllModels)->begin(); m != (&AllModels)->end(); ++m) {
            bool ret = m->second->DeleteAllAliases();
            if (ret) {
                models += (deleted ? ", " : "") + JSONSafe(m->first);
                deleted = deleted || ret;
            }
        }
        if (deleted) {
            MarkEffectsFileDirty();
            return sendResponse("\"" + models + "\"", "models", 200, true);
        } else {
        	return sendResponse("No aliases found to delete.", "msg", 503, false);
		}
    } else if (cmd == "getViews") {
        std::string views; 
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("No sequence open.", "msg", 503, false);
        }
        auto AllViews = GetViewsManager()->GetViews();
        for (auto it = AllViews.begin(); it != AllViews.end(); ++it) {
            views += "\"" + JSONSafe((*it)->GetName()) + "\",";            
        }
        if (!views.empty()) {
            views.pop_back(); // remove last comma
        }
        views = "[" + views + "]";
        return sendResponse(views, "views", 200, true);

    } else if (cmd == "makeMaster") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("No sequence open.", "msg", 503, false);
        }
        if (params["view"].empty()) {
            return sendResponse("No template view selected.", "msg", 504, false);
        }
        auto view = params["view"];

        displayElementsPanel->SelectView(view);

        // Click on the Make Master item
        displayElementsPanel->DoMakeMaster();
        std::string response = "{\"msg\":\"Master view updated.\"}";
        return sendResponse(response, "", 200, true);
    } else if (cmd == "getModel") {
        auto model = params["model"];
        auto m = AllModels.GetModel(model);
        if (nullptr == m) {
            return sendResponse("Unknown model.", "msg", 503, false);
        }
        auto json = m->GetAttributesAsJSON();
        return sendResponse(json, "model", 200, true);
        
    } else if (cmd == "getControllers") {
        std::string controllers;
        for (const auto& it : _outputManager.GetControllers()) {
            std::string json = it->GetJSONData() + ",";
            controllers += json;
        }
        if (!controllers.empty()) {
            controllers.pop_back();//remove last comma
        }
        controllers = "[" + controllers + "]";
        return sendResponse(controllers, "controllers", 200, true);
    } else if (cmd == "getControllerIPs") {
        std::string ipAddresses;
        for (const auto& it : _outputManager.GetControllers()) {
            if (!it->GetIP().empty()) {
                ipAddresses += "\"" + JSONSafe(it->GetIP()) + "\",";
            }
        }
        if (!ipAddresses.empty()) {
            ipAddresses.pop_back();//remove last comma
        }
        ipAddresses = "[" + ipAddresses + "]";
        return sendResponse(ipAddresses, "controllers", 200, true);
    } else if (cmd == "getControllerPortMap") {
        auto ip = params["ip"];
        auto name = params["name"];
        Controller* controller {nullptr};
        if (!name.empty()) {
            controller = _outputManager.GetController(name);
        }
        if (!ip.empty()) {
            controller = _outputManager.GetControllerWithIP(ip);
        }
        if (controller == nullptr) {
            return "{\"res\":504,\"msg\":\"Controller not found.\"}";
        }
        UDController cud(controller, &_outputManager, &AllModels, false);
        auto json = cud.ExportAsJSON();
        return sendResponse(json, "controllerportmap", 200, true);
    } else if (cmd == "getEffectIDs") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
        auto model = params["model"];
        Element* ele = _sequenceElements.GetElement(model);
        if (ele == nullptr) {
            return sendResponse("target element doesn't exists.", "msg", 503, false);
        }
        std::string layers = "[";
        for (size_t i = 0; i < ele->GetEffectLayerCount(); ++i) {
            std::string ids;
            auto effects = ele->GetEffectLayer(i)->GetAllEffects();
            for (auto* eff : effects) {
                ids += "\"" + std::to_string(eff->GetID()) + "\",";
            }
            if (!ids.empty()) {
                ids.pop_back(); // remove last comma
            }
            ids.insert(0, "[");
            ids.append("],");
            layers.append(ids);
        }
        layers.pop_back(); // remove last comma
        layers += "]";
        return sendResponse(layers, "effects", 200, true);
    } else if (cmd == "cleanupFileLocations") {

        bool res = CleanupRGBEffectsFileLocations();

        if (CurrentSeqXmlFile != nullptr) {
            res = res && CleanupSequenceFileLocations();
        }

        if (res) {
            std::string response = "{\"msg\":\"Cleanup file locations.\",\"worked\":\"true\"}";
            return sendResponse(response, "", 200, true);
        }
        else
        {
            return sendResponse("Cleanup file locations failed.", "msg", 503, false);
        }

    } else if (cmd == "getEffectSettings") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
        int id = 0;
        int layer = 0;

        if (!params["id"].empty()) {
            id = (int)std::strtol(params["id"].c_str(), nullptr, 10);
        }
        if (!params["layer"].empty()) {
            layer = (int)std::strtol(params["layer"].c_str(), nullptr, 10);
        }
        auto const& model = params["model"];
        Element* ele = _sequenceElements.GetElement(model);
        if (ele == nullptr) {
            return sendResponse("target element doesn't exists.", "msg", 503, false);
        }
        auto* lay = ele->GetEffectLayer(layer);
        if (lay == nullptr) {
            return sendResponse("target layer doesn't exists.", "msg", 503, false);
        }
        auto* eff = lay->GetEffectFromID(id);
        if (eff != nullptr) {

            std::string json = "{\"name\":\"" + eff->GetEffectName() + "\"" +
                                ",\"settings\":" + eff->GetSettingsAsJSON() +
                               ",\"palette\":" + eff->GetPaletteAsJSON() +
                               ",\"startTime\":" + std::to_string(eff->GetStartTimeMS()) +
                               ",\"endTime\":" + std::to_string(eff->GetEndTimeMS()) +
                                ",\"selected\":" + std::to_string(eff->GetSelected()) + "}";
            return sendResponse(json, "", 200, true);
        }
        return sendResponse("target effect doesn't exists.", "msg", 503, false);
    } else if (cmd == "setEffectSettings") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
        int id = 0;
        int layer = 0;

        if (!params["id"].empty()) {
            id = (int)std::strtol(params["id"].c_str(), nullptr, 10);
        }
        if (!params["layer"].empty()) {
            layer = (int)std::strtol(params["layer"].c_str(), nullptr, 10);
        }
        auto const& model = params["model"];
        Element* ele = _sequenceElements.GetElement(model);
        if (ele == nullptr) {
            return sendResponse("target element doesn't exists.", "msg", 503, false);
        }
        auto* lay = ele->GetEffectLayer(layer);
        if (lay == nullptr) {
            return sendResponse("target layer doesn't exists.", "msg", 503, false);
        }
        auto* eff = lay->GetEffectFromID(id);
        if (eff != nullptr) {

            if (!params["name"].empty()) {
                eff->SetEffectName(params["name"]);
            }
            if (!params["startTime"].empty()) {
                eff->SetStartTimeMS((int)std::strtol(params["startTime"].c_str(), nullptr, 10));
            }
            if (!params["endTime"].empty()) {
                eff->SetEndTimeMS((int)std::strtol(params["endTime"].c_str(), nullptr, 10));
            }
            if (!params["settings"].empty()) {
                eff->SetSettings(params["settings"], true , true);
            }
            if (!params["palette"].empty()) {
                eff->SetColourOnlyPalette(params["palette"], true);
            }
            mainSequencer->PanelEffectGrid->Refresh();
            mainSequencer->SelectEffect(eff);
            std::string response = wxString::Format("{\"msg\":\"Set Effect Settings.\",\"worked\":\"%s\"}", JSONSafe(toStr(eff != nullptr)));
            return sendResponse(response, "", 200, true);
        }
        return sendResponse("target effect doesn't exists.", "msg", 503, false);
    } else if (cmd == "deleteEffect") {
        // Mirrors setEffectSettings' lookup pattern (model/layer/id), then
        // the same capture-then-delete sequence EffectsGrid's own delete
        // path uses (CaptureEffectToBeDeleted for undo, then
        // EffectLayer::DeleteEffect -- both existing, already-used core
        // calls, not new deletion logic).
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
        int id = 0;
        int layer = 0;

        if (!params["id"].empty()) {
            id = (int)std::strtol(params["id"].c_str(), nullptr, 10);
        }
        if (!params["layer"].empty()) {
            layer = (int)std::strtol(params["layer"].c_str(), nullptr, 10);
        }
        auto const& model = params["model"];
        Element* ele = _sequenceElements.GetElement(model);
        if (ele == nullptr) {
            return sendResponse("target element doesn't exists.", "msg", 503, false);
        }
        auto* lay = ele->GetEffectLayer(layer);
        if (lay == nullptr) {
            return sendResponse("target layer doesn't exists.", "msg", 503, false);
        }
        auto* eff = lay->GetEffectFromID(id);
        if (eff == nullptr) {
            return sendResponse("target effect doesn't exists.", "msg", 503, false);
        }
        _sequenceElements.get_undo_mgr().CaptureEffectToBeDeleted(model, layer, eff->GetEffectName(),
                                                                   eff->GetSettingsAsString(), eff->GetPaletteAsString(),
                                                                   eff->GetStartTimeMS(), eff->GetEndTimeMS(),
                                                                   eff->GetSelected(), eff->GetProtected());
        lay->DeleteEffect(id);
        mainSequencer->PanelEffectGrid->Refresh();
        std::string response = "{\"msg\":\"Deleted Effect.\",\"worked\":\"true\"}";
        return sendResponse(response, "", 200, true);
    } else if (cmd == "importXLightsSequence") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
        auto filename = params["filename"];
        if (filename == "" || filename == "null"|| !wxFile::Exists(filename)) {
            return sendResponse("Import file not valid.", "msg", 503, false);
        }
        auto mapmethod = params["mapmethod"];
        if (mapmethod.empty()) mapmethod = "file";
        bool autoMap = (mapmethod == "auto") || (mapmethod == "both");
        auto importMediaParam = params["importmedia"];
        bool importMedia = importMediaParam.empty() ? true : ReadBool(importMediaParam);
        auto mapname = params["mapfile"];
        if (mapmethod != "auto") {
            if (mapname == "" || mapname == "null" || !wxFile::Exists(mapname)) {
                return sendResponse("Mapping File not valid.", "msg", 503, false);
            }
        }
        ImportXLights(wxFileName(filename), mapname, autoMap, importMedia);

        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(this, eventRowHeaderChanged);
        mainSequencer->PanelEffectGrid->Refresh();

        std::string response = "{\"msg\":\"Imported XLights Sequence.\",\"worked\":\"true\"}";
        return sendResponse(response, "", 200, true);
    } else if (cmd == "getShowFolder") {
        return sendResponse(JSONSafe(showDirectory), "folder", 200, false);
    } else if (cmd == "listSequences") {
        auto folder = params["folder"];
        if (folder.empty() || !wxDir::Exists(folder)) {
            return sendResponse("Folder does not exist.", "msg", 503, false);
        }
        // Single pass with explicit extension check avoids Windows 8.3 short-name
        // wildcard ambiguity where "*.xsq" also matches "*.xsqz" files.
        // std::map gives deduplication (by full path) and alphabetical order for free.
        static const std::set<std::string> validExts = {"xsq", "xsqz", "zip", "piz"};
        std::map<std::string, wxFileName> sortedFiles;
        wxDir dir(folder);
        if (dir.IsOpened()) {
            wxString filename;
            bool cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
            while (cont) {
                wxFileName fn(folder, filename);
                std::string ext = ToStdString(fn.GetExt().Lower());
                if (validExts.count(ext)) {
                    sortedFiles[ToStdString(fn.GetFullPath())] = fn;
                }
                cont = dir.GetNext(&filename);
            }
        }
        nlohmann::json arr = nlohmann::json::array();
        for (auto const& [path, fn] : sortedFiles) {
            nlohmann::json entry;
            entry["name"] = ToStdString(fn.GetName());
            entry["path"] = path;
            entry["type"] = (fn.GetExt().Lower() == "xsq") ? "sequence" : "package";
            arr.push_back(entry);
        }
        nlohmann::json resp;
        resp["sequences"] = arr;
        return sendResponse(resp.dump(), "", 200, true);
    } else if (cmd == "getSequenceInfo") {
        auto filename = params["filename"];
        if (filename.empty() || !wxFile::Exists(filename)) {
            return sendResponse("File not found.", "msg", 503, false);
        }
        SequencePackage xsqPkg(std::filesystem::path(ToStdString(wxString(filename))),
                               GetShowDirectory(), "", &AllModels);
        if (xsqPkg.IsPkg()) {
            xsqPkg.Extract();
        } else {
            xsqPkg.FindRGBEffectsFile();
        }
        if (!xsqPkg.IsValid()) {
            return sendResponse("Not a valid sequence or package.", "msg", 503, false);
        }
        SequenceFile xlf(xsqPkg.GetXsqFile().string());
        auto importDoc = xlf.Open(GetShowDirectory(), false, xsqPkg.GetXsqFile().string());
        if (!importDoc) {
            return sendResponse("Failed to open sequence file.", "msg", 503, false);
        }
        wxFileName fn(filename);
        nlohmann::json info;
        info["name"] = ToStdString(fn.GetName());
        info["duration"] = xlf.GetSequenceDurationMS();
        info["frameRate"] = xlf.GetFrequency();
        info["mediaFile"] = xlf.GetMediaFile();
        return sendResponse(info.dump(), "", 200, true);
    } else if (cmd == "setModelProperty") {
        auto model = params["model"];
        auto m = AllModels.GetModel(model);
        if (nullptr == m) {
            return sendResponse("Unknown model.", "msg", 503, false);
        }
        auto propKey = params["key"];
        auto propData = params["data"];
        if (propKey.empty() || propData.empty()) {
            return sendResponse("Key or Data was empty.", "msg", 503, false);
        }
        layoutPanel->SelectModel(model);
        wxPropertyGridEvent event2;
        event2.SetPropertyGrid(layoutPanel->GetPropertyEditor());
        wxStringProperty wsp("Model", propKey, propData);
        event2.SetProperty(&wsp);
        wxVariant value(propData);
        event2.SetPropertyValue(value);
        layoutPanel->OnPropertyGridChange(event2);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Automation:setModelProperty");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Automation:setModelProperty");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Automation:setModelProperty");
        std::string response = wxString::Format("{\"msg\":\"Set Model Property.\",\"worked\":\"%s\"}", JSONSafe(toStr(m != nullptr)));
        return sendResponse(response, "", 200, true);
    } else if (cmd == "getFseqDirectory") {
        return sendResponse(JSONSafe(GetFseqDirectory()), "folder", 200, false);
    } else if (cmd == "getMedia") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
        auto& media = _sequenceElements.GetSequenceMedia();
        std::optional<MediaType> onlyType;
        if (!params["type"].empty() && params["type"] != "null") {
            onlyType = MediaTypeFromName(params["type"]);
            if (!onlyType) {
                return sendResponse("Unknown media type '" + params["type"] + "'.", "msg", 503, false);
            }
        }
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& [path, type] : media.GetAllMediaPaths()) {
            if (onlyType && type != *onlyType) continue;
            auto entry = LookupMediaEntry(media, path, type);
            if (!entry) continue;
            nlohmann::json j;
            j["path"] = path;
            j["type"] = MediaTypeName(type);
            j["embedded"] = entry->IsEmbedded();
            j["embeddable"] = entry->IsEmbeddable();
            j["used"] = entry->IsUsed() || entry->IsUsedByMetadata();
            if (type == MediaType::Image) {
                auto img = std::static_pointer_cast<ImageCacheEntry>(entry);
                j["width"] = img->GetImageWidth();
                j["height"] = img->GetImageHeight();
                j["frames"] = img->GetImageCount();
            }
            arr.push_back(j);
        }
        nlohmann::json res;
        res["media"] = arr;
        return sendResponse(res.dump(), "", 200, true);
    } else if (cmd == "getMediaIssues" || cmd == "convertMedia") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
        // Same inputs the open-sequence compatibility warning uses.
        auto issues = MediaCompatibility::CheckSequenceMedia(
            CurrentSeqXmlFile->GetMediaFile(),
            _sequenceElements.GetSequenceMedia().GetVideoFilePaths());

        if (cmd == "getMediaIssues") {
            nlohmann::json arr = nlohmann::json::array();
            for (const auto& i : issues) {
                nlohmann::json j;
                j["path"] = i.filePath;
                j["reason"] = i.reason;
                j["type"] = i.isVideo ? "video" : "audio";
                j["canconvert"] = i.isVideo && i.canConvert();
                j["animatedgif"] = i.isAnimatedGif();
                arr.push_back(j);
            }
            nlohmann::json res;
            res["issues"] = arr;
            return sendResponse(res.dump(), "", 200, true);
        }

        auto result = seqmedia::ConvertIncompatibleVideos(_sequenceElements, issues);
        if (result.effectsUpdated > 0 || result.gifEffectsConverted > 0) {
            _sequenceElements.IncrementChangeCount(nullptr);
            if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr) {
                mainSequencer->PanelEffectGrid->ForceRefresh();
            }
        }
        nlohmann::json res;
        res["msg"] = wxString::Format("Converted %d of %d file(s). %d video effect(s) updated.",
                                      result.converted, result.attempted, result.effectsUpdated).ToStdString();
        res["attempted"] = result.attempted;
        res["converted"] = result.converted;
        res["effectsupdated"] = result.effectsUpdated;
        res["gifeffectsconverted"] = result.gifEffectsConverted;
        if (!result.failures.empty()) res["failed"] = result.failures;
        return sendResponse(res.dump(), "", result.failures.empty() ? 200 : 503, true);
    } else if (cmd == "embedMedia" || cmd == "extractMedia") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
        const bool embed = (cmd == "embedMedia");
        auto& media = _sequenceElements.GetSequenceMedia();

        std::optional<MediaType> onlyType;
        if (!params["type"].empty() && params["type"] != "null") {
            onlyType = MediaTypeFromName(params["type"]);
            if (!onlyType) {
                return sendResponse("Unknown media type '" + params["type"] + "'.", "msg", 503, false);
            }
        }
        const std::string file = (params["file"] == "null") ? "" : params["file"];
        if (!file.empty() && !media.HasMedia(file)) {
            return sendResponse("Media '" + file + "' is not referenced by this sequence.", "msg", 503, false);
        }

        // Extract needs somewhere to write the bytes back to. Default to
        // wherever the stored path resolves today so the effect keeps working
        // without a reference rewrite.
        std::string destDir = (params["dir"] == "null") ? "" : params["dir"];
        if (!embed && !destDir.empty()) {
            if (!IsInShowOrMediaFolder(destDir)) {
                return sendResponse("Extract directory must be inside the show or a media folder.", "msg", 503, false);
            }
            ObtainAccessToURL(destDir, true);
        }

        int changed = 0;
        int relativized = 0;
        std::vector<std::string> failures;
        for (const auto& [path, type] : media.GetAllMediaPaths()) {
            if (!file.empty() && path != file) continue;
            if (onlyType && type != *onlyType) continue;
            auto entry = LookupMediaEntry(media, path, type);
            if (!entry) continue;

            if (embed) {
                const bool already = entry->IsEmbedded();
                if (!already) {
                    // Embed only flips a flag - the base64 payload comes from
                    // the load. An entry the render never touched is still
                    // undecoded here, and embedding it would write an <Image>
                    // with no data.
                    if (!entry->isLoaded()) entry->Load();
                    if (!entry->IsEmbeddable()) {
                        if (!file.empty()) failures.push_back(path);
                        continue;
                    }
                }
                // Embedded bytes live in the document, so an absolute path only
                // pins the sequence to one machine - store it show/media
                // relative. Done for already-embedded entries too: a -1.png
                // frame series arrives from AddAnimatedImage pre-embedded under
                // whatever path the effect held, and would otherwise keep it.
                const std::string key = _sequenceElements.MakeMediaPathRelative(path);
                if (already) {
                    if (key != path) ++relativized;
                    continue;
                }
                media.EmbedMedia(key);
                if (key != path) ++relativized;
                ++changed;
            } else {
                if (!entry->IsEmbedded()) continue;
                std::string dest;
                if (!destDir.empty()) {
                    dest = (std::filesystem::path(destDir) / std::filesystem::path(path).filename()).string();
                } else {
                    dest = FileUtils::FixFile("", path);
                    if (dest.empty()) dest = entry->GetFilePath();
                }
                if (dest.empty()) { failures.push_back(path); continue; }
                bool ok;
                if (type == MediaType::Image) {
                    ok = media.ExtractImageToFile(path, dest);
                } else {
                    ok = entry->SaveToFile(dest);
                    if (ok) media.ExtractMedia(path);
                }
                if (!ok) { failures.push_back(path); continue; }
                if (dest != path) {
                    const std::string rel = MakeRelativePath(dest);
                    const std::string finalPath = rel.empty() ? dest : rel;
                    if (type == MediaType::Image) {
                        media.RenameImage(dest, finalPath);
                    } else {
                        media.RenameMedia(path, finalPath);
                    }
                    _sequenceElements.RewriteMediaReferences(path, finalPath);
                }
                ++changed;
            }
        }

        if (changed > 0 || relativized > 0) {
            _sequenceElements.IncrementChangeCount(nullptr);
            if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr) {
                mainSequencer->PanelEffectGrid->Refresh();
            }
        }
        nlohmann::json res;
        res["msg"] = (embed ? "Embedded " : "Extracted ") + std::to_string(changed) + " media file(s).";
        res["count"] = changed;
        if (embed) res["maderelative"] = relativized;
        if (!failures.empty()) res["failed"] = failures;
        return sendResponse(res.dump(), "", failures.empty() ? 200 : 503, true);
    }
    return false;
}


bool xLightsFrame::ProcessHttpRequest(HttpConnection& connection, HttpRequest& request)
{
    wxString uri = request.URI();
    wxString params = "";

    if (uri.find('?') != std::string::npos) {
        params = uri.substr(uri.find('?') + 1);
        uri = uri.substr(0, uri.find('?'));
    }
    std::vector<std::string> paths;
    std::map<std::string, std::string> paramMap = ParseParams(params);

    if (uri[0] == '/') {
        uri = uri.substr(1);
    }
    while (uri.find('/') != std::string::npos) {
        wxString p = uri.substr(0, uri.find('/'));
        paths.push_back(wxURI::Unescape(p));
        uri = uri.substr(uri.find('/') + 1);
    }
    paths.push_back(wxURI::Unescape(uri));

    wxString accept = request["Accept"];
    if (paths[0] == "xlDoAutomation") {
        paths.clear();
        params.clear();
        accept = MIME_JSON;

        try {
            nlohmann::json val = nlohmann::json::parse(request.Data().ToStdString());
            if (!val.contains("cmd")) {
                HttpResponse resp(connection, request, (HttpStatus::HttpStatusCode)503);
                resp.AddHeader("access-control-allow-origin", "*");
                resp.MakeFromText("{\"res\":503,\"msg\":\"Missing cmd.\"}", MIME_JSON);
                connection.SendResponse(resp);
                return true;
            } else {
                for (auto [mn, v] : val.items()) {
                    // nlohmann::json v = val[mn];
                    if (mn == "cmd") {
                        paths.push_back(v.get<std::string>());
                    } else if (v.is_string()) {
                        paramMap[mn] = v.get<std::string>();
                    } else if (v.is_number_integer()) {
                        paramMap[mn] = std::to_string(v.get<int>());
                    } else if (v.is_number_float()) {
                        paramMap[mn] = std::to_string(v.get<float>());
                    } else if (v.is_boolean()) {
                        paramMap[mn] = v.get<bool>() ? "true" : "false";
                    } else if (v.is_array()) {
                        for (size_t x = 0; x < v.size(); x++) {
                            std::string k = mn + "_" + std::to_string(x);
                            paramMap[k] = v[x].get<std::string>();
                        }
                    } else if (v.is_object()) {
                        // Nested objects (e.g. setEffectSettings' "settings"/
                        // "palette", documented in xlDo Commands.txt as real
                        // JSON objects, not strings) had no branch here at all,
                        // so they were silently dropped -- paramMap[mn] was
                        // never set, params["settings"].empty() was always
                        // true, and the code below that already calls
                        // eff->SetSettings()/SetColourOnlyPalette() never ran.
                        // Re-serializing back to a JSON string matches what
                        // those calls already expect (they're invoked with
                        // json=true).
                        paramMap[mn] = v.dump();
                    }
                }

                if (paramMap.empty()) {
                    paramMap["_METHOD"] = "GET";
                } else {
                    paramMap["_METHOD"] = "POST";
                }
            }
        } catch (const std::exception& /*ex*/) {
            
        }
       
    } else {
        paramMap["_METHOD"] = request.Method();
        if (request.Method() == "POST" || request.Method() == "PUT") {
            paramMap["_DATA"] = request.Data();
        }
    }

    return ProcessAutomation(paths, paramMap, [&](const std::string& msg, const std::string& jsonKey, int responseCode, bool isJson) {
        
        HttpResponse resp(connection, request, (HttpStatus::HttpStatusCode)responseCode);
        resp.AddHeader("access-control-allow-origin", "*");

        if (accept == MIME_JSON) {
            if (isJson) {
                if (jsonKey == "") {
                    resp.MakeFromText(msg, MIME_JSON);
                } else {
                    wxString json = "{\"" + jsonKey + "\":" + msg + "}";
                    resp.MakeFromText(json, MIME_JSON);
                }
            } else {
                wxString json = "{\"" + jsonKey + "\":\"" + msg + "\"}";
                resp.MakeFromText(json, MIME_JSON);
            }
        } else if (isJson) {
            resp.MakeFromText(msg, MIME_JSON);
        } else {
            resp.MakeFromText(msg, MIME_TEXT);
        }
        // The problem here is the connection may no longer be valid ... but i am not sure how to safely detect this
        // This means if the client suddenly disconnects then xLights crashes on the SendResponse call as connection and request objects have all been destroyed from under us with no way to know it has happened
        // adding this check helps but it still has a race condition
        if (_automationServer->IsConnectionValid(&connection)) {
            connection.SendResponse(resp);
            return true;
        } else {
            spdlog::warn("Automation did not send result because connection lost.");
        }
        return false;
    });
}

void xLightsFrame::StartAutomationListener()
{
    

    if (_automationServer != nullptr) {
        _automationServer->Stop();
        delete _automationServer;
        _automationServer = nullptr;
    }

    if (_xFadePort == 0) return;
    
    
    HttpServer *server = new HttpServer();
    HttpContext ctx;
    ctx.Port = ::GetxFadePort(_xFadePort);

    ctx.RequestHandler = HttpRequestFunction;
    ctx.MessageHandler = nullptr;

    // default error pages content
    ctx.ErrorPage400 = HTTP_ERROR_PAGE;
    ctx.ErrorPage404 = HTTP_ERROR_PAGE;

    if (!server->Start(ctx)) {
        spdlog::debug("xLights Automation could not listen on {}", ::GetxFadePort(_xFadePort));
        delete server;
        return;
    }
    spdlog::debug("xLights Automation listening on {}", ::GetxFadePort(_xFadePort));
    _automationServer = server;
}

std::string xLightsFrame::ProcessxlDoAutomation(const std::string& msg)
{
    std::vector<std::string> paths;
    std::map<std::string, std::string> paramMap;

    try
    {
        nlohmann::json val = nlohmann::json::parse(msg);
        if (!val.contains("cmd")) {
            return "{\"res\":504,\"msg\":\"Missing cmd.\"}";
        } else {
            for (auto [mn, v] : val.items()) {
                   if (mn == "cmd") {
                    paths.push_back(v.get<std::string>());
                } else if (v.is_string()) {
                    paramMap[mn] = v.get<std::string>();
                } else if (v.is_number_integer()) {
                    paramMap[mn] = std::to_string(v.get<int>());
                }
                else if (v.is_number_float()) {
                    paramMap[mn] = std::to_string(v.get<float>());
                } else if (v.is_boolean()) {
                    paramMap[mn] = v.get<bool>() ? "true" : "false";
                } else if (v.is_array()) {
                    for (size_t x = 0; x < v.size(); x++) {
                        std::string k = mn + "_" + std::to_string(x);
                        paramMap[k] = v[x].get<std::string>();
                    }
                } else if (v.is_object()) {
                    // See the matching branch in HttpRequestFunction above --
                    // nested objects (setEffectSettings' "settings"/"palette")
                    // had no branch here either, so they were silently dropped.
                    paramMap[mn] = v.dump();
                }
            }

            if (paramMap.empty()) {
                paramMap["_METHOD"] = "GET";
            } else {
                paramMap["_METHOD"] = "POST";
            }

            std::string result;
            bool processed = ProcessAutomation(paths, paramMap, [&](const std::string &msg,
                                                                    const std::string &jsonKey,
                                                                    int responseCode,
                                                                    bool isJson) {
                if (isJson) {
                    if (jsonKey == "") {
                        result = "{\"res\":" + std::to_string(responseCode) +"," + msg.substr(1);
                    } else {
                        result = "{\"res\":" + std::to_string(responseCode) +",\"" + jsonKey + "\":" + msg + "}";
                    }
                } else {
                    result = "{\"res\":" + std::to_string(responseCode) +",\"" + jsonKey + "\":\"" + msg + "\"}";
                }
                return true;
            });
            
            if (!processed) {
                auto cmd = val["cmd"].get<std::string>();
                return wxString::Format("{\"res\":504,\"msg\":\"Unknown command: '%s'.\"}", cmd);
            }
            return result;
        }
    } catch (std::exception &)
    {}
    return "{\"res\":504,\"msg\":\"Error parsing request.\"}";
}
 
/*

            

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
             else {
                return wxString::Format("{\"res\":504,\"msg\":\"Unknown command: '%s'.\"}", cmd);
            }
        }
    } else {
        return "{\"res\":504,\"msg\":\"Error parsing request.\"}";
    }
}
*/
