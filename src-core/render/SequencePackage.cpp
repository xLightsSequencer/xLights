/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <algorithm>
#include <pugixml.hpp>
#include <fstream>

#include "render/SequencePackage.h"
#include "render/SequenceElements.h"
#include "models/ModelManager.h"
#include "models/Model.h"
#include "utils/ExternalHooks.h"

extern "C" {
#include "../../dependencies/libxlsxwriter/third_party/minizip/unzip.h"
}

#include <log.h>

static const std::string IMPORTED_MEDIA = "ImportedMedia";
static const std::string SUBFLD_IMAGES = "Images";
static const std::string SUBFLD_VIDEOS = "Videos";
static const std::string SUBFLD_FACES = "DownloadedFaces";
static const std::string SUBFLD_SHADERS = "Shaders";
static const std::string SUBFLD_GLEDIATORS = "Glediators";

SeqPkgImportOptions::SeqPkgImportOptions() {};

SeqPkgImportOptions::~SeqPkgImportOptions() {};

void SeqPkgImportOptions::SetImportActive(bool active)
{
    _importActive = active;
}

bool SeqPkgImportOptions::IsImportActive() const
{
    return _importActive;
}

void SeqPkgImportOptions::SetDir(MediaTargetDir dirType, const std::string& dirPath, bool saveAsDefault)
{
    _mediaDirs[dirType] = dirPath;

    if (saveAsDefault) {
        _defaultDirs[dirType] = dirPath;
    }
}

std::string SeqPkgImportOptions::GetDir(MediaTargetDir dirType)
{
    return _mediaDirs[dirType];
}

void SeqPkgImportOptions::RestoreDefaults()
{
    _mediaDirs.clear();

    for (const auto& dir : _defaultDirs) {
        _mediaDirs[dir.first] = dir.second;
    }
}

SequencePackage::SequencePackage(const std::filesystem::path& fileName, const std::string& showDir, const std::string& seqXmlFileName, ModelManager* models)
{
    _showDirectory = showDir;
    _seqXmlFileName = seqXmlFileName;
    _modelManager = models;

    std::string ext = fileName.extension().string();
    // normalize to lowercase for comparison
    for (auto& c : ext) c = std::tolower(c);

    if (ext == ".zip" || ext == ".piz" || ext == ".xsqz") {
        _xsqOnly = false;
        _pkgFile = fileName;
        FileExists(_pkgFile.string(), true);
        ObtainAccessToURL(_pkgFile.string());
    } else {
        _xsqOnly = true;
        _xsqFile = fileName;
        FileExists(_xsqFile.string(), true);
        ObtainAccessToURL(_xsqFile.string());
    }
}

SequencePackage::~SequencePackage()
{
    // cleanup extracted files
    if (!_xsqOnly && !_tempDir.empty() && std::filesystem::exists(_tempDir) && !_leaveFiles) {
        std::error_code ec;
        std::filesystem::remove_all(_tempDir, ec);
    }
}

void SequencePackage::InitDefaultImportOptions()
{
    if (_showDirectory.empty())
        return;

    // Set default target media directories based on a few assumptions. User
    // can still change these in the Mapping Dialog to whatever they would like.

    std::string showFolder = _showDirectory;

    // always default faces/shaders to default download folder as they tend to be reused
    _importOptions.SetDir(MediaTargetDir::FACES_DIR, (std::filesystem::path(showFolder) / SUBFLD_FACES).string(), true);

    std::filesystem::path targetXsq(_seqXmlFileName);
    std::string targetDir = targetXsq.parent_path().string();

    std::string mediaBaseFolder;

    if (targetDir != showFolder) {
        // target xsq is not at the root of show folder, assume user manages show folder
        // with a subfolder per sequence as Gil noted
        mediaBaseFolder = targetDir;
    } else {
        // default images/videos/glediators into new ImportedMedia/<NameOfSrcXsq>/<MediaType>
        mediaBaseFolder = (std::filesystem::path(showFolder) / IMPORTED_MEDIA / _xsqName).string();
    }

    // set the defaults for media sub folders
    _importOptions.SetDir(MediaTargetDir::GLEDIATORS_DIR, (std::filesystem::path(mediaBaseFolder) / SUBFLD_GLEDIATORS).string(), true);
    _importOptions.SetDir(MediaTargetDir::IMAGES_DIR, (std::filesystem::path(mediaBaseFolder) / SUBFLD_IMAGES).string(), true);
    _importOptions.SetDir(MediaTargetDir::SHADERS_DIR, (std::filesystem::path(mediaBaseFolder) / SUBFLD_SHADERS).string(), true);
    _importOptions.SetDir(MediaTargetDir::VIDEOS_DIR, (std::filesystem::path(mediaBaseFolder) / SUBFLD_VIDEOS).string(), true);
}

void SequencePackage::Extract()
{
    if (_xsqOnly) {
        return;
    }

    auto progressUpdate = [this](int pct) -> bool {
        if (_progressCb) return _progressCb(pct);
        return false;
    };
    progressUpdate(0);

    unzFile uf = unzOpen(_pkgFile.string().c_str());
    if (uf == nullptr) {
        spdlog::error("Could not open the Sequence Package '{}'", _pkgFile.filename().string());
        progressUpdate(100);
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    _tempDir = std::filesystem::temp_directory_path() / (_pkgFile.stem().string() + "_" + std::to_string(ms));
    spdlog::debug("Extracting Sequence Package '{}' to '{}'", _pkgFile.filename().string(), _tempDir.string());

    unz_global_info gi;
    if (unzGetGlobalInfo(uf, &gi) != UNZ_OK) {
        spdlog::error("Could not read zip global info for '{}'", _pkgFile.filename().string());
        unzClose(uf);
        progressUpdate(100);
        return;
    }

    if (gi.number_entry == 0) {
        spdlog::error("No entries found in zip file '{}'", _pkgFile.filename().string());
        unzClose(uf);
        progressUpdate(100);
        return;
    }

    progressUpdate(10);
    int numEntryProcessed = 0;
    int progStep = (int)(std::floor(90.0 / gi.number_entry));

    int ret = unzGoToFirstFile(uf);
    while (ret == UNZ_OK) {
        char entryName[512];
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(uf, &fileInfo, entryName, sizeof(entryName), nullptr, 0, nullptr, 0) != UNZ_OK) {
            ret = unzGoToNextFile(uf);
            continue;
        }

        std::string entryNameStr(entryName);
        bool isDir = (!entryNameStr.empty() && entryNameStr.back() == '/') || (fileInfo.external_fa & 0x10);

        std::filesystem::path fnEntry = _tempDir / entryNameStr;

        if (fnEntry.string().find("__MACOSX") != std::string::npos) {
            spdlog::debug("   skipping MACOS Folder {}.", fnEntry.string());
            ret = unzGoToNextFile(uf);
            continue;
        }

#ifdef _WIN32
        // folder with spaces at begin and end breaks temp folder paths
        std::string fnStr = fnEntry.string();
        std::string sep(1, std::filesystem::path::preferred_separator);
        std::string spaceSep = " " + sep;
        std::string sepSpace = sep + " ";
        size_t pos;
        while ((pos = fnStr.find(spaceSep)) != std::string::npos) {
            fnStr.replace(pos, spaceSep.size(), sep);
        }
        while ((pos = fnStr.find(sepSpace)) != std::string::npos) {
            fnStr.replace(pos, sepSpace.size(), sep);
        }
        fnEntry = fnStr;
#endif

        std::filesystem::path fnOutput = fnEntry;

        spdlog::debug("   Extracting {} to {}.", fnEntry.string(), fnOutput.string());

#ifdef _WIN32
        if (fnOutput.string().length() > MAX_PATH) {
            spdlog::warn("Target filename longer than {} chars ({}). This will likely fail. {}.", MAX_PATH, (int)fnOutput.string().length(), fnOutput.string());
        }
#endif

        // Create output dir in temp if needed
        std::filesystem::path parentDir = fnOutput.parent_path();
        std::error_code ec;
        if (!std::filesystem::exists(parentDir, ec)) {
            std::filesystem::create_directories(parentDir, ec);
        }

        // handle file output
        if (!isDir) {
            if (unzOpenCurrentFile(uf) != UNZ_OK) {
                spdlog::error("Could not read file from package '{}'", entryNameStr);
            } else {
                std::ofstream fos(fnOutput, std::ios::binary);

                if (!fos.is_open()) {
                    spdlog::error("Could not create sequence file at '{}'", fnOutput.filename().string());
                } else {
                    char buf[8192];
                    int bytesRead;
                    while ((bytesRead = unzReadCurrentFile(uf, buf, sizeof(buf))) > 0) {
                        fos.write(buf, bytesRead);
                    }
                    fos.close();

                    std::string ext = fnOutput.extension().string();
                    if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
                    std::string stem = fnOutput.stem().string();

                    if (ext == "xsq") {
                        _xsqFile = fnOutput;
                        _xsqName = stem;
                    } else if (ext == "xml") {
                        if (stem == "xlights_rgbeffects") {
                            pugi::xml_document rgbEffects;
                            if (rgbEffects.load_file(fnOutput.string().c_str())) {
                                _rgbEffects = std::move(rgbEffects);
                                _xlEffects = fnOutput;
                                _pkgRoot = fnOutput.parent_path();
                                _hasRGBEffects = true;
                            }
                        } else if (stem == "xlights_networks") {
                            _xlNetworks = fnOutput;
                        } else {
                            pugi::xml_document doc;
                            if (doc.load_file(fnOutput.string().c_str())) {
                                if (std::string_view(doc.document_element().name()) == "xsequence") {
                                    _xsqFile = fnOutput;
                                    _xsqName = stem;
                                }
                            }
                        }
                    } else {
                        // assume other files are media for effects, images/videos/gediators/shaders/faces/etc
                        _media[fnOutput.filename().string()] = fnOutput;
                    }
                }
                unzCloseCurrentFile(uf);
            }
        }

        numEntryProcessed++;
        progressUpdate(10 + progStep * numEntryProcessed);

        ret = unzGoToNextFile(uf);
    }

    unzClose(uf);
    progressUpdate(100);

    if (_xsqFile.empty() || !FileExists(_xsqFile.string())) {
        spdlog::error("No sequence file found in package '{}'", _pkgFile.filename().string());
    } else {
        InitDefaultImportOptions();
    }
}

void SequencePackage::FindRGBEffectsFile()
{
    std::filesystem::path showDir = _xsqFile.parent_path();
    std::filesystem::path rgbEffectsPath = showDir / "xlights_rgbeffects.xml";
    if (FileExists(rgbEffectsPath.string())) {
        pugi::xml_document rgbEffects;
        if (rgbEffects.load_file(rgbEffectsPath.string().c_str())) {
            _xlEffects = rgbEffectsPath;
            _rgbEffects = std::move(rgbEffects);
            _hasRGBEffects = true;
        }
    }
}

bool SequencePackage::IsValid() const
{
    if (_xsqOnly) {
        return !_xsqFile.empty() && FileExists(_xsqFile.string());
    } else {
        return !_xsqFile.empty() && FileExists(_xsqFile.string()) && _hasRGBEffects;
    }
}

bool SequencePackage::IsPkg()
{
    return !_xsqOnly;
}

bool SequencePackage::HasRGBEffects() const
{
    return _hasRGBEffects;
}

bool SequencePackage::HasMedia() const
{
    return _media.size() > 0;
}

bool SequencePackage::HasMissingMedia() const
{
    return _missingMedia.size() > 0;
}

const std::filesystem::path& SequencePackage::GetXsqFile()
{
    return _xsqFile;
}

pugi::xml_document& SequencePackage::GetRgbEffectsFile()
{
    return _rgbEffects;
}

std::string SequencePackage::GetTempShowFolder() const
{
    return _xlEffects.parent_path().string();
}
std::string SequencePackage::GetTempDir() const {
    return _tempDir.string();
}

bool SequencePackage::ModelsChanged() const
{
    return _modelsChanged;
}

SeqPkgImportOptions* SequencePackage::GetImportOptions()
{
    if (!_xsqOnly) {
        return &_importOptions;
    } else {
        return nullptr;
    }
}

std::list<std::string> SequencePackage::GetMissingMedia()
{
    _missingMedia.unique();
    return _missingMedia;
}

// Extract just the filename from a path, handling both / and \ separators
static std::string ExtractFilename(const std::string& path) {
    // Handle both Unix and DOS path separators
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

std::string SequencePackage::FixAndImportMedia(Effect* mappedEffect, EffectLayer* target)
{
    auto settings = mappedEffect->GetSettings();
    std::string effName = mappedEffect->GetEffectName();

    std::string settingEffectFile;
    std::string targetMediaFolder;

    if (effName == "Pictures") {
        targetMediaFolder = _importOptions.GetDir(MediaTargetDir::IMAGES_DIR);
        if (settings.Contains("E_FILEPICKER_Pictures_Filename")) {
            // old Pictures key
            settingEffectFile = "E_FILEPICKER_Pictures_Filename";
        } else {
            std::string v = settings["E_TEXTCTRL_Pictures_Filename"];
            auto &sm = mappedEffect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetSequenceMedia();
            auto &tm = target->GetParentElement()->GetSequenceElements()->GetSequenceMedia();
            if (sm.HasImage(v)) {
                auto img = sm.GetImage(v);
                if (img->IsEmbedded() && !tm.HasImage(v)) {
                    tm.AddEmbeddedImage(v, img->GetEmbeddedData());
                } else if (!img->IsEmbedded()) {
                    settingEffectFile = "E_TEXTCTRL_Pictures_Filename";
                }
            } else {
                settingEffectFile = "E_TEXTCTRL_Pictures_Filename";
            }
        }
    } else if (effName == "Video") {
        settingEffectFile = "E_FILEPICKERCTRL_Video_Filename";
        targetMediaFolder = _importOptions.GetDir(MediaTargetDir::VIDEOS_DIR);
    } else if (effName == "Shader") {
        settingEffectFile = "E_0FILEPICKERCTRL_IFS";
        targetMediaFolder = _importOptions.GetDir(MediaTargetDir::SHADERS_DIR);
    } else if (effName == "Glediator") {
        settingEffectFile = "E_FILEPICKERCTRL_Glediator_Filename";
        targetMediaFolder = _importOptions.GetDir(MediaTargetDir::GLEDIATORS_DIR);
    } else if (effName == "Faces") {
        std::string faceName = settings["E_CHOICE_Faces_FaceDefinition"];
        if (!faceName.empty()) {
            ImportFaceInfo(mappedEffect, target, faceName);
        }
    } else if (effName == "Shape") {
        std::string shapePath = settings["E_FILEPICKERCTRL_SVG"];
        if (!shapePath.empty()) {
            settingEffectFile = "E_FILEPICKERCTRL_SVG";
            targetMediaFolder = _importOptions.GetDir(MediaTargetDir::IMAGES_DIR);
        }
    } else if (effName == "Ripple") {
        std::string shapePath = settings["E_FILEPICKERCTRL_Ripple_SVG"];
        if (!shapePath.empty()) {
            settingEffectFile = "E_FILEPICKERCTRL_Ripple_SVG";
            targetMediaFolder = _importOptions.GetDir(MediaTargetDir::IMAGES_DIR);
        }
    }

    if (!settingEffectFile.empty()) {
        std::string settingPath = settings.Get(settingEffectFile, "");

        // extract just the filename from the path, handling both / and \ separators
        std::string picFileName = ExtractFilename(settingPath);

        // import the asset if we have it, otherwise track it as missing
        auto it = _media.find(picFileName);
        std::filesystem::path fileToCopy = (it != _media.end()) ? it->second : std::filesystem::path();

        if (!fileToCopy.empty() && FileExists(fileToCopy.string())) {
            std::filesystem::path copiedAsset = CopyMediaToTarget(targetMediaFolder, fileToCopy);
            settings.erase(settingEffectFile);
            std::string newSetting = copiedAsset.string();
            settings[settingEffectFile] = newSetting;
            if (effName == "Pictures") {
                auto &tm = target->GetParentElement()->GetSequenceElements()->GetSequenceMedia();
                if (!tm.HasImage(newSetting)) {
                    tm.GetImage(newSetting);
                }
            }
        } else {
            if (!picFileName.empty())
                _missingMedia.push_back(picFileName);
        }
    }

    return settings.AsString();
}

void SequencePackage::ImportFaceInfo(Effect* mappedEffect, EffectLayer* target, const std::string& faceName)
{
    if (_modelManager == nullptr)
        return;

    auto targetModelName = target->GetParentElement()->GetModelName();
    auto srcModelName = mappedEffect->GetParentEffectLayer()->GetParentElement()->GetModelName();
    Model* targetModel = (*_modelManager)[targetModelName];

    const auto& faceInfo = targetModel->GetFaceInfo().find(faceName);
    if (faceInfo != targetModel->GetFaceInfo().end()) {
        // face already defined don't overwrite it
        return;
    }

    if (_hasRGBEffects) {
        pugi::xml_node modelsNode;
        for (pugi::xml_node node = _rgbEffects.document_element().first_child(); node; node = node.next_sibling()) {
            if (std::string_view(node.name()) == "models") {
                modelsNode = node;
                break;
            }
        }

        if (modelsNode) {
            pugi::xml_node modelNode;
            for (pugi::xml_node model = modelsNode.first_child(); model; model = model.next_sibling()) {
                if (std::string_view(model.attribute("name").as_string()) == srcModelName) {
                    modelNode = model;
                    break;
                }
            }

            if (modelNode) {
                // find faceInfo node
                for (pugi::xml_node modelChild = modelNode.first_child(); modelChild; modelChild = modelChild.next_sibling()) {
                    if (std::string_view(modelChild.name()) == "faceInfo") {
                        std::string name = modelChild.attribute("Name").as_string();
                        std::string type = modelChild.attribute("Type").as_string();

                        // only import if type is matrix
                        if ((name == faceName || faceName == "Default") && type == "Matrix") {
                            std::map<std::string, std::string> faceAttributes;

                            for (pugi::xml_attribute attr = modelChild.first_attribute(); attr; attr = attr.next_attribute()) {
                                std::string attrName = attr.name();
                                std::string attrValue = attr.as_string();

                                // import files
                                if (attrName.substr(0, 5) == "Mouth" || attrName.substr(0, 4) == "Eyes") {
                                    if (!attrValue.empty()) {
                                        // extract just the filename from the path
                                        std::string faceFileName = ExtractFilename(attrValue);

                                        // import the asset if we have it, otherwise track it as missing
                                        auto it = _media.find(faceFileName);
                                        std::filesystem::path fileToCopy = (it != _media.end()) ? it->second : std::filesystem::path();

                                        if (!fileToCopy.empty() && FileExists(fileToCopy.string())) {
                                            std::filesystem::path copiedAsset = CopyMediaToTarget(_importOptions.GetDir(MediaTargetDir::FACES_DIR), fileToCopy);
                                            faceAttributes[attrName] = copiedAsset.string();
                                        } else {
                                            _missingMedia.push_back(faceFileName);
                                        }
                                    }
                                } else {
                                    faceAttributes[attrName] = attrValue;
                                }
                            }

                            targetModel->AddFace(faceAttributes);
                            targetModel->IncrementChangeCount();
                            _modelsChanged = true;

                            break;
                        }
                    }
                }
            }
        }
    }
}

std::filesystem::path SequencePackage::CopyMediaToTarget(const std::string& targetFolder, const std::filesystem::path& mediaToCopy)
{
    std::filesystem::path targetFile = std::filesystem::path(targetFolder) / mediaToCopy.filename();

    // Only import if file doesn't already exist in target folder
    if (!FileExists(targetFile.string())) {
        // make sure dir exists first
        std::error_code ec;
        std::filesystem::path targetDir = targetFile.parent_path();
        if (!std::filesystem::exists(targetDir, ec)) {
            std::filesystem::create_directories(targetDir, ec);
        }

        // now copy the asset
        std::filesystem::copy_file(mediaToCopy, targetFile, std::filesystem::copy_options::skip_existing, ec);
    }

    return targetFile;
}

std::filesystem::path SequencePackage::FindAndCopyAudio(const std::string& targetDir)
{
    static const std::vector<std::string> AUDIO_EXTS = {
        "mp3", "ogg", "m4p", "m4a", "aac", "wav", "flac", "wma", "au", "mp4"
    };

    // Try to identify the audio filename referenced in the XSQ mediaFile element
    std::string xsqAudioName;
    if (!_xsqFile.empty() && FileExists(_xsqFile.string())) {
        pugi::xml_document xsqDoc;
        if (xsqDoc.load_file(_xsqFile.string().c_str())) {
            auto headNode = xsqDoc.child("xsequence").child("head");
            if (headNode) {
                auto mediaFileNode = headNode.child("mediaFile");
                if (mediaFileNode) {
                    std::filesystem::path p(mediaFileNode.text().as_string());
                    xsqAudioName = p.filename().string();
                }
            }
        }
    }

    std::filesystem::path audioFile;

    // Prefer the file referenced by the XSQ
    if (!xsqAudioName.empty()) {
        auto it = _media.find(xsqAudioName);
        if (it != _media.end() && !it->second.empty() && FileExists(it->second.string())) {
            audioFile = it->second;
        }
    }

    // Fall back to first audio file found in _media by extension
    if (audioFile.empty()) {
        for (const auto& m : _media) {
            std::string ext = m.second.extension().string();
            if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
            // lowercase
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            for (const auto& ae : AUDIO_EXTS) {
                if (ext == ae) {
                    audioFile = m.second;
                    break;
                }
            }
            if (!audioFile.empty()) break;
        }
    }

    if (audioFile.empty()) {
        spdlog::warn("SequencePackage: No audio file found in package '{}'", _pkgFile.filename().string());
        return std::filesystem::path();
    }

    return CopyMediaToTarget(targetDir, audioFile);
}
