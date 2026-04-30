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
#include <set>

#include "render/SequencePackage.h"
#include "render/SequenceElements.h"
#include "render/SequenceMedia.h"
#include "models/ModelManager.h"
#include "models/Model.h"
#include "models/ViewObjectManager.h"
#include "models/ViewObject.h"
#include "utils/ExternalHooks.h"
#include "utils/FileUtils.h"

#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <CoreText/CoreText.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

extern "C" {
#include "../../dependencies/libxlsxwriter/third_party/minizip/unzip.h"
#include "../../dependencies/libxlsxwriter/third_party/minizip/zip.h"
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
    std::string targetExt = targetXsq.extension().string();
    for (auto& c : targetExt) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    bool isSavedSeq = (targetExt == ".xsq" || targetExt == ".xml");

    std::string targetDir = targetXsq.parent_path().string();

    std::string mediaBaseFolder;

    if (isSavedSeq && targetDir != showFolder) {
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

std::filesystem::path SequencePackage::FindAndCopyAudio(const std::filesystem::path& targetDir)
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

    return CopyMediaToTarget(targetDir.string(), audioFile);
}

std::vector<std::pair<std::string, std::string>> SequencePackage::FindAndCopyAltAudioTracks(const std::filesystem::path& targetDir)
{
    std::vector<std::pair<std::string, std::string>> result;

    if (_xsqFile.empty() || !FileExists(_xsqFile.string())) {
        return result;
    }

    pugi::xml_document xsqDoc;
    if (!xsqDoc.load_file(_xsqFile.string().c_str())) {
        return result;
    }

    auto altTracksNode = xsqDoc.child("xsequence").child("head").child("altAudioTracks");
    if (!altTracksNode) {
        return result;
    }

    for (auto trackNode : altTracksNode.children("track")) {
        std::string shortname = trackNode.attribute("shortname").as_string("");
        std::string tpath = trackNode.text().as_string("");

        std::string filename;
        {
            std::filesystem::path p(tpath);
            filename = p.filename().string();
        }

        if (filename.empty()) {
            result.emplace_back(shortname, std::string());
            continue;
        }

        auto it = _media.find(filename);
        if (it != _media.end() && !it->second.empty() && FileExists(it->second.string())) {
            std::filesystem::path copied = CopyMediaToTarget(targetDir.string(), it->second);
            result.emplace_back(shortname, copied.string());
        } else {
            spdlog::warn("SequencePackage: Alt audio track '{}' ('{}') not found in package.", shortname, filename);
            result.emplace_back(shortname, std::string());
        }
    }

    return result;
}

bool SequencePackage::Repack(const std::filesystem::path& targetXsqz)
{
    if (_tempDir.empty() || !std::filesystem::exists(_tempDir)) {
        spdlog::error("SequencePackage::Repack: no temp dir to pack from ('{}')", _tempDir.string());
        return false;
    }

    ObtainAccessToURL(targetXsqz.string(), /*enforceWritable=*/true);

    // Write to a sibling .tmp and atomically rename so a crash / failure
    // mid-write can't corrupt the user's original package.
    std::filesystem::path tmpPath = targetXsqz;
    tmpPath += ".tmp";

    std::error_code ec;
    std::filesystem::remove(tmpPath, ec);

    zipFile zf = zipOpen(tmpPath.string().c_str(), APPEND_STATUS_CREATE);
    if (zf == nullptr) {
        spdlog::error("SequencePackage::Repack: zipOpen failed for '{}'", tmpPath.string());
        return false;
    }

    bool success = true;
    size_t filesPacked = 0;

    for (auto it = std::filesystem::recursive_directory_iterator(_tempDir, ec);
         it != std::filesystem::recursive_directory_iterator();
         ++it) {
        const auto& entry = *it;
        if (!entry.is_regular_file()) {
            continue;
        }

        std::string filename = entry.path().filename().string();
        // Skip Finder cruft and autosave backups that don't belong in a package
        if (filename == ".DS_Store" || filename.rfind(".xbkp") != std::string::npos) {
            continue;
        }

        // Zip paths are always forward-slash, relative to the temp dir root
        std::string relStr = std::filesystem::relative(entry.path(), _tempDir, ec).generic_string();
        if (ec || relStr.empty()) {
            continue;
        }

        zip_fileinfo zi{};
        if (zipOpenNewFileInZip(zf, relStr.c_str(), &zi,
                                nullptr, 0, nullptr, 0, nullptr,
                                Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK) {
            spdlog::error("SequencePackage::Repack: zipOpenNewFileInZip failed for '{}'", relStr);
            success = false;
            break;
        }

        std::ifstream fis(entry.path(), std::ios::binary);
        if (!fis.is_open()) {
            spdlog::error("SequencePackage::Repack: could not read '{}'", entry.path().string());
            zipCloseFileInZip(zf);
            success = false;
            break;
        }

        char buf[8192];
        while (fis.good()) {
            fis.read(buf, sizeof(buf));
            std::streamsize n = fis.gcount();
            if (n > 0) {
                if (zipWriteInFileInZip(zf, buf, static_cast<unsigned>(n)) != ZIP_OK) {
                    spdlog::error("SequencePackage::Repack: write failed for '{}'", relStr);
                    success = false;
                    break;
                }
            }
        }
        fis.close();
        zipCloseFileInZip(zf);

        if (!success) {
            break;
        }
        ++filesPacked;
    }

    zipClose(zf, nullptr);

    if (!success) {
        std::filesystem::remove(tmpPath, ec);
        return false;
    }

    // Atomic replace of the original .xsqz. std::filesystem::rename on
    // POSIX is rename(2) which is atomic within a single filesystem —
    // which is always the case here since the tmp and target are
    // siblings in the user's Files location.
    std::filesystem::rename(tmpPath, targetXsqz, ec);
    if (ec) {
        spdlog::error("SequencePackage::Repack: rename '{}' -> '{}' failed: {}",
                      tmpPath.string(), targetXsqz.string(), ec.message());
        std::filesystem::remove(tmpPath, ec);
        return false;
    }

    spdlog::info("SequencePackage::Repack: packed {} files into '{}'", filesPacked, targetXsqz.string());
    return true;
}

// =============================================================
// SequencePackage::Pack — wx-free comprehensive packager.
// =============================================================

namespace {

// In-zip subdirectory for external files of a given media type.
// These match desktop convention where reasonable (Images /
// Videos / Shaders) and add Meshes / Faces to cover assets that
// previously had no home. TextFile stays in root since it's
// rare and doesn't clump neatly.
std::string subdirForMediaType(MediaType t)
{
    switch (t) {
        case MediaType::Image:      return "Images";
        case MediaType::SVG:        return "Images";
        case MediaType::Shader:     return "Shaders";
        case MediaType::TextFile:   return "";
        case MediaType::BinaryFile: return "Glediators";
        case MediaType::Video:      return "Videos";
        case MediaType::Audio:      return "Audio";
    }
    return "";
}

// True iff `file` is a descendant of `canonDir`. `canonDir` must
// already be weakly-canonical (caller canonicalizes showDir once at
// the top of Pack instead of re-doing it per file). `file` is
// canonicalized on the fly since each file is different.
bool isUnderCanonDir(const std::filesystem::path& file, const std::filesystem::path& canonDir)
{
    if (canonDir.empty() || file.empty()) return false;
    std::error_code ec;
    auto canonFile = std::filesystem::weakly_canonical(file, ec);
    if (ec) canonFile = file;

    auto f = canonFile.begin();
    for (auto d = canonDir.begin(); d != canonDir.end(); ++d, ++f) {
        if (f == canonFile.end() || *f != *d) return false;
    }
    return true;
}

// Claim a zip-path for a file, disambiguating on collision by
// inserting a `dup2`, `dup3`, … subdir — filename never changes
// so PicturesEffect's `-1` / `_1` animation-sequence detection
// can't be tricked into triggering.
std::string claimUniqueZipPath(std::set<std::string>& claimed,
                               const std::string& typeSubdir,
                               const std::string& basename)
{
    std::string prefix = typeSubdir.empty() ? std::string() : (typeSubdir + "/");
    std::string candidate = prefix + basename;
    if (claimed.insert(candidate).second) {
        return candidate;
    }
    for (int i = 2; i < 10000; ++i) {
        candidate = prefix + "dup" + std::to_string(i) + "/" + basename;
        if (claimed.insert(candidate).second) {
            return candidate;
        }
    }
    // Fallback — we'll never hit this in practice.
    return prefix + basename;
}

// Walk a pugi tree and rewrite any attribute value / text-node
// value that contains one of the paths in `rewrites`. Caller must
// pre-sort by descending key length: `/a/b/foo.png.bak` must fire
// before `/a/b/foo.png` so the shorter prefix doesn't convert the
// inside of the longer path first and block the longer rule's
// match. Substring replacement (not just exact match) covers the
// packed-settings case — effect settings are serialized as
// `key1=v1,key2=v2,...` inside a single attribute, so file paths
// live as substrings inside larger attribute values.
void rewritePathsInXml(pugi::xml_node node,
                       const std::vector<std::pair<std::string, std::string>>& sortedRewrites)
{
    auto applyRewrites = [&sortedRewrites](std::string& value) -> bool {
        bool changed = false;
        for (const auto& kv : sortedRewrites) {
            const std::string& orig = kv.first;
            const std::string& repl = kv.second;
            if (orig.empty() || orig == repl) continue;
            size_t pos = 0;
            while ((pos = value.find(orig, pos)) != std::string::npos) {
                value.replace(pos, orig.length(), repl);
                pos += repl.length();
                changed = true;
            }
        }
        return changed;
    };

    for (auto attr : node.attributes()) {
        std::string value = attr.value();
        if (value.empty()) continue;
        if (applyRewrites(value)) {
            attr.set_value(value.c_str());
        }
    }
    // Some serializers put paths in text nodes — cover both.
    if (node.type() == pugi::node_pcdata) {
        std::string value = node.value();
        if (applyRewrites(value)) {
            node.set_value(value.c_str());
        }
    }
    for (auto child : node.children()) {
        rewritePathsInXml(child, sortedRewrites);
    }
}

// Stream a file into the open zip at the given zip-relative path.
// Pre-flights the source read before creating the zip entry: minizip
// has no remove-entry API, so if we opened the entry and then failed
// to read the source we'd leave a ~2-byte stub in the zip that
// reads back as a corrupt file. Opening the ifstream first means
// any "permission denied / file vanished" case returns false with
// no zip mutation.
bool addFileToZip(zipFile zf, const std::string& zipPath, const std::filesystem::path& source)
{
    std::ifstream fis(source, std::ios::binary);
    if (!fis.is_open()) {
        spdlog::error("Pack: could not open source '{}'", source.string());
        return false;
    }

    zip_fileinfo zi{};
    if (zipOpenNewFileInZip(zf, zipPath.c_str(), &zi,
                            nullptr, 0, nullptr, 0, nullptr,
                            Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK) {
        spdlog::error("Pack: zipOpenNewFileInZip failed for '{}'", zipPath);
        return false;
    }
    char buf[8192];
    bool ok = true;
    while (fis.good()) {
        fis.read(buf, sizeof(buf));
        std::streamsize n = fis.gcount();
        if (n > 0) {
            if (zipWriteInFileInZip(zf, buf, static_cast<unsigned>(n)) != ZIP_OK) {
                spdlog::error("Pack: write failed for '{}'", zipPath);
                ok = false;
                break;
            }
        }
    }
    fis.close();
    zipCloseFileInZip(zf);
    return ok;
}

// Write a byte buffer into the open zip at the given zip-relative path.
bool addBufferToZip(zipFile zf, const std::string& zipPath, const std::string& data)
{
    zip_fileinfo zi{};
    if (zipOpenNewFileInZip(zf, zipPath.c_str(), &zi,
                            nullptr, 0, nullptr, 0, nullptr,
                            Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK) {
        spdlog::error("Pack: zipOpenNewFileInZip failed for '{}'", zipPath);
        return false;
    }
    bool ok = true;
    if (!data.empty()) {
        if (zipWriteInFileInZip(zf, data.data(), static_cast<unsigned>(data.size())) != ZIP_OK) {
            spdlog::error("Pack: write failed for '{}'", zipPath);
            ok = false;
        }
    }
    zipCloseFileInZip(zf);
    return ok;
}

// Resolve a raw path string to an absolute, existing file using
// FileUtils::FixFile (which consults showDir + the media search
// chain). Returns empty if the file can't be found.
std::string resolveToAbsolute(const std::string& showDir, const std::string& raw)
{
    if (raw.empty()) return std::string();
    if (FileExists(raw)) {
        std::error_code ec;
        auto abs = std::filesystem::absolute(raw, ec);
        if (!ec) return abs.string();
        return raw;
    }
    std::string fixed = FileUtils::FixFile(showDir, raw);
    if (!fixed.empty() && FileExists(fixed)) {
        std::error_code ec;
        auto abs = std::filesystem::absolute(fixed, ec);
        if (!ec) return abs.string();
        return fixed;
    }
    return std::string();
}

// Compute the zip-relative path for a resolved absolute file.
// Files under showDir keep their existing relative path; external
// files are placed under `<subdir>/<basename>` with collision
// disambiguation. `canonShowDir` is the weakly-canonicalized
// showDir (computed once in Pack) — passed in instead of
// canonicalizing per call.
std::string computeZipPath(const std::filesystem::path& abs,
                           const std::string& showDir,
                           const std::filesystem::path& canonShowDir,
                           const std::string& typeSubdir,
                           std::set<std::string>& claimed)
{
    if (!showDir.empty() && isUnderCanonDir(abs, canonShowDir)) {
        std::error_code ec;
        auto rel = std::filesystem::relative(abs, std::filesystem::path(showDir), ec);
        if (!ec && !rel.empty()) {
            std::string relStr = rel.generic_string();
            if (claimed.insert(relStr).second) {
                return relStr;
            }
            // Pre-claimed by an earlier pass — unusual (same file
            // enumerated twice); just reuse the existing slot.
            return relStr;
        }
    }
    return claimUniqueZipPath(claimed, typeSubdir, abs.filename().string());
}

// Enumerate every media file to pack from a SequenceMedia, honoring
// `excludeVideos`. Embedded entries are skipped — they live inside
// the `.xsq` already.
void collectFromSequenceMedia(SequenceMedia& media,
                              const std::string& showDir,
                              const std::filesystem::path& canonShowDir,
                              bool excludeVideos,
                              std::map<std::string, std::string>& outRewrites,
                              std::map<std::string, std::string>& outToPack,
                              std::set<std::string>& claimed,
                              std::vector<std::string>& outWarnings)
{
    for (const auto& entry : media.GetAllMediaPaths()) {
        const std::string& raw = entry.first;
        MediaType t = entry.second;
        if (t == MediaType::Video && excludeVideos) continue;
        auto embed = media.GetMediaEmbedState(raw);
        if (embed.first /*isEmbedded*/) continue;

        std::string abs = resolveToAbsolute(showDir, raw);
        if (abs.empty()) {
            std::string msg = "Could not locate media file '" + raw + "' — not included in package";
            spdlog::warn("Pack: {}", msg);
            outWarnings.push_back(msg);
            continue;
        }
        if (outToPack.count(abs)) continue; // already queued (e.g. shared by multiple effects)

        std::string zipPath = computeZipPath(std::filesystem::path(abs), showDir, canonShowDir,
                                             subdirForMediaType(t), claimed);
        outToPack[abs] = zipPath;
        if (raw != zipPath) {
            outRewrites[raw] = zipPath;
        }
        if (abs != raw && abs != zipPath) {
            outRewrites[abs] = zipPath;
        }
    }
}

// Extract the typeface name from the stored wxFont native-info string.
// Three formats can appear:
//   Windows LOGFONT:  "0;-16;0;...;Bold;FaceName"  — face after last semicolon
//   Simple quoted:    "'Face Name' 12"               — between first pair of '
//   Bare word:        "Arial"                        — whole string (no spaces expected)
static std::string parseFaceNameFromFontString(const std::string& fontStr)
{
    if (fontStr.empty()) return {};

    // Windows LOGFONT serialisation — last semicolon-delimited token is the face name.
    auto lastSemi = fontStr.rfind(';');
    if (lastSemi != std::string::npos) {
        std::string face = fontStr.substr(lastSemi + 1);
        while (!face.empty() && std::isspace((unsigned char)face.back()))  face.pop_back();
        while (!face.empty() && std::isspace((unsigned char)face.front())) face.erase(face.begin());
        if (!face.empty()) return face;
    }

    // Quoted format used by LOR import: 'Face Name' 12
    if (fontStr.front() == '\'') {
        auto close = fontStr.find('\'', 1);
        if (close != std::string::npos) return fontStr.substr(1, close - 1);
    }

    // Bare: everything before the first space (or the whole string).
    auto sp = fontStr.find(' ');
    return sp != std::string::npos ? fontStr.substr(0, sp) : fontStr;
}

// Scan all effects in the sequence for FONTPICKER settings and return the
// unique set of OS typeface names that are referenced.
static std::set<std::string> collectFontFaceNames(SequenceElements& seqElements)
{
    std::set<std::string> faces;
    static const std::string FONTPICKER_PREFIX = "E_FONTPICKER_";

    size_t elemCount = seqElements.GetElementCount(MASTER_VIEW);
    for (size_t i = 0; i < elemCount; ++i) {
        Element* elem = seqElements.GetElement(i, MASTER_VIEW);
        if (!elem) continue;
        size_t layerCount = elem->GetEffectLayerCount();
        for (size_t l = 0; l < layerCount; ++l) {
            EffectLayer* layer = elem->GetEffectLayer(l);
            if (!layer) continue;
            int effectCount = layer->GetEffectCount();
            for (int e = 0; e < effectCount; ++e) {
                Effect* eff = layer->GetEffect(e);
                if (!eff) continue;
                const auto& settings = eff->GetSettings();
                for (const auto& [key, val] : settings) {
                    if (key.rfind(FONTPICKER_PREFIX, 0) != 0) continue;
                    std::string strVal = val;
                    if (strVal.empty()) continue;
                    std::string face = parseFaceNameFromFontString(strVal);
                    if (!face.empty()) {
                        spdlog::info("Pack: found font face '{}' in effect '{}' key '{}'",
                                      face, eff->GetEffectName(), key);
                        faces.insert(face);
                    }
                }
            }
        }
    }
    return faces;
}

// Resolve an OS typeface name to an absolute font file path.
// Returns empty string if the font cannot be located or is a guaranteed-present
// system font that does not need bundling.
#ifdef _WIN32
static std::string resolveFontFaceToFile(const std::string& faceName)
{
    // Build font directory paths from environment variables.
    auto getEnvW = [](const wchar_t* var) -> std::wstring {
        DWORD len = GetEnvironmentVariableW(var, nullptr, 0);
        if (len == 0) return {};
        std::wstring buf(len, L'\0');
        GetEnvironmentVariableW(var, buf.data(), len);
        if (!buf.empty() && buf.back() == L'\0') buf.pop_back();
        return buf;
    };

    auto wToUtf8 = [](const std::wstring& ws) -> std::string {
        if (ws.empty()) return {};
        int n = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (n <= 0) return {};
        std::string s(n - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, s.data(), n, nullptr, nullptr);
        return s;
    };

    std::filesystem::path sysFontsDir =
        std::filesystem::path(getEnvW(L"WINDIR")) / L"Fonts";
    std::filesystem::path perUserFontsDir =
        std::filesystem::path(getEnvW(L"LOCALAPPDATA")) / L"Microsoft" / L"Windows" / L"Fonts";

    std::string faceNameLower = faceName;
    std::transform(faceNameLower.begin(), faceNameLower.end(), faceNameLower.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });

    static const wchar_t* FONT_REG_KEY =
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";

    struct Entry { HKEY root; std::filesystem::path dir; };
    Entry entries[] = {
        { HKEY_CURRENT_USER, perUserFontsDir },   // per-user (Win10+)
        { HKEY_LOCAL_MACHINE, sysFontsDir }        // system-wide
    };

    for (const auto& entry : entries) {
        HKEY hKey = nullptr;
        if (RegOpenKeyExW(entry.root, FONT_REG_KEY, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
            continue;

        DWORD idx = 0;
        wchar_t valueName[512];
        wchar_t valueData[512];
        std::string found;

        while (found.empty()) {
            DWORD nameLen = 512;
            DWORD dataLen = sizeof(valueData);
            DWORD type = 0;
            LONG res = RegEnumValueW(hKey, idx++, valueName, &nameLen, nullptr,
                                     &type, reinterpret_cast<LPBYTE>(valueData), &dataLen);
            if (res == ERROR_NO_MORE_ITEMS) break;
            if (res != ERROR_SUCCESS || type != REG_SZ) continue;

            std::string regName = wToUtf8(valueName);
            // Strip trailing style/format annotations: "Arial Bold (TrueType)" -> "Arial Bold"
            // then strip bold/italic variants so "Arial (TrueType)" matches face "Arial".
            for (const char* sfx : { " (TrueType)", " (OpenType)", " (TrueType Collection)" }) {
                auto pos = regName.rfind(sfx);
                if (pos != std::string::npos) { regName = regName.substr(0, pos); break; }
            }
            std::string regLower = regName;
            std::transform(regLower.begin(), regLower.end(), regLower.begin(),
                           [](unsigned char c) { return (char)std::tolower(c); });

            // Match either exact ("Arial") or with style suffix ("Arial Bold").
            if (regLower != faceNameLower &&
                regLower.rfind(faceNameLower + " ", 0) != 0) continue;

            // Resolve relative paths against the font directory for this hive.
            std::wstring dataStr(valueData);
            std::filesystem::path fontPath =
                std::filesystem::path(dataStr).is_absolute()
                    ? std::filesystem::path(dataStr)
                    : entry.dir / dataStr;

            std::error_code ec;
            if (std::filesystem::exists(fontPath, ec))
                found = fontPath.string();
        }

        RegCloseKey(hKey);
        if (!found.empty()) return found;
    }
    return {};
}

#elif defined(__APPLE__)
static std::string resolveFontFaceToFile(const std::string& faceName)
{
    CFStringRef cfName = CFStringCreateWithCString(
        kCFAllocatorDefault, faceName.c_str(), kCFStringEncodingUTF8);
    if (!cfName) return {};

    CTFontDescriptorRef desc = CTFontDescriptorCreateWithNameAndSize(cfName, 12.0);
    CFRelease(cfName);
    if (!desc) return {};

    CFURLRef urlRef = (CFURLRef)CTFontDescriptorCopyAttribute(desc, kCTFontURLAttribute);
    CFRelease(desc);
    if (!urlRef) return {};

    char pathBuf[4096] = {};
    bool ok = CFURLGetFileSystemRepresentation(urlRef, true,
                                               reinterpret_cast<UInt8*>(pathBuf),
                                               sizeof(pathBuf));
    CFRelease(urlRef);
    if (!ok) return {};

    std::string path(pathBuf);
    // /System/Library/Fonts/ is present on every Mac — skip it.
    if (path.rfind("/System/Library/Fonts/", 0) == 0) return {};

    return path;
}

#else
static std::string resolveFontFaceToFile(const std::string&) { return {}; }
#endif

// Collect OS font files used by Text/Shape/etc. effects and queue them
// under Fonts/ in the zip.
static void collectFontFiles(SequenceElements& seqElements,
                              std::map<std::string, std::string>& outToPack,
                              std::set<std::string>& claimed,
                              std::vector<std::string>& outWarnings)
{
    auto faceNames = collectFontFaceNames(seqElements);
    if (faceNames.empty()) return;

    for (const auto& face : faceNames) {
        std::string fontPath = resolveFontFaceToFile(face);
        if (fontPath.empty()) {
            spdlog::debug("Pack: font '{}' not resolved to a bundleable file — skipping", face);
            continue;
        }
        if (outToPack.count(fontPath)) continue;
        std::string basename = std::filesystem::path(fontPath).filename().string();
        std::string zipPath = claimUniqueZipPath(claimed, "Fonts", basename);
        outToPack[fontPath] = zipPath;
        spdlog::info("Pack: bundling font '{}' from '{}' as '{}'", face, fontPath, zipPath);
    }
}

// Walk the in-memory sequence elements to find which face definitions each
// model uses. Returns map of model_name -> list of face definition names so
// all images for only those definitions are packaged.
static std::map<std::string, std::list<std::string>> collectFacesUsedInSequence(SequenceElements& seqElements)
{
    std::map<std::string, std::list<std::string>> result;

    size_t elemCount = seqElements.GetElementCount(MASTER_VIEW);
    for (size_t i = 0; i < elemCount; ++i) {
        Element* elem = seqElements.GetElement(i, MASTER_VIEW);
        if (!elem) continue;
        const std::string& modelName = elem->GetModelName();

        size_t layerCount = elem->GetEffectLayerCount();
        for (size_t l = 0; l < layerCount; ++l) {
            EffectLayer* layer = elem->GetEffectLayer(l);
            if (!layer) continue;
            int effectCount = layer->GetEffectCount();
            for (int e = 0; e < effectCount; ++e) {
                Effect* eff = layer->GetEffect(e);
                if (!eff) continue;
                const std::string& effName = eff->GetEffectName();
                if (effName != "Faces" && effName != "CoroFaces") continue;
                std::string faceName = eff->GetSettings().Get("E_CHOICE_Faces_FaceDefinition", "Default");
                if (!faceName.empty()) {
                    auto& lst = result[modelName];
                    if (std::find(lst.begin(), lst.end(), faceName) == lst.end())
                        lst.push_back(faceName);
                }
            }
        }
    }

    return result;
}

// Enumerate file references from every model (or view-object). We
// use the iterator pair exposed by `ModelManager::begin/end` /
// `ViewObjectManager::begin/end` — BaseObject itself doesn't
// expose enumeration. For models with Faces effects, all images for
// the used face definitions are included (every phoneme in those definitions).
template<typename ManagerT>
void collectFromObjectManager(ManagerT& mgr,
                              const std::string& showDir,
                              const std::filesystem::path& canonShowDir,
                              const std::string& defaultSubdir,
                              const std::map<std::string, std::list<std::string>>& modelFacesUsed,
                              std::map<std::string, std::string>& outRewrites,
                              std::map<std::string, std::string>& outToPack,
                              std::set<std::string>& claimed,
                              std::set<std::string>& claimedGroupDirs,
                              std::vector<std::string>& outWarnings)
{
    for (auto it = mgr.begin(); it != mgr.end(); ++it) {
        auto* obj = it->second;
        if (!obj) continue;

        std::list<std::string> refs = obj->GetFileReferences();
        // Face images are collected separately so they bypass the mesh
        // group-colocation logic and land under "Faces/" instead of "Objects/".
        std::list<std::string> faceRefs;
        if constexpr (std::is_same_v<ManagerT, ModelManager>) {
            auto facesIt = modelFacesUsed.find(it->first);
            if (facesIt != modelFacesUsed.end()) {
                const auto& usedDefs = facesIt->second;
                for (const auto& [defName, defMap] : obj->GetFaceInfo()) {
                    if (std::find(usedDefs.begin(), usedDefs.end(), defName) == usedDefs.end()) continue;
                    auto typeIt = defMap.find("Type");
                    if (typeIt == defMap.end() || typeIt->second != "Matrix") continue;
                    for (const auto& [key, val] : defMap) {
                        if (key != "CustomColors" && key != "ImagePlacement" && key != "Type" && !val.empty())
                            faceRefs.push_back(val);
                    }
                }
            }
        }

        // Resolve every ref to an absolute path up-front so we can
        // make a group-level decision (co-locate mesh + mtl +
        // textures vs. per-file scatter).
        struct Resolved { std::string raw; std::string abs; };
        std::vector<Resolved> resolved;
        for (const auto& raw : refs) {
            if (raw.empty()) continue;
            std::string abs = resolveToAbsolute(showDir, raw);
            if (abs.empty()) {
                std::string msg = "Could not locate file '" + raw + "' referenced by model '" + it->first + "' — not included in package";
                spdlog::warn("Pack: {}", msg);
                outWarnings.push_back(msg);
                continue;
            }
            resolved.push_back({raw, abs});
        }
        if (resolved.empty() && faceRefs.empty()) continue;

        // Group colocation: MeshObject::GetFileReferences returns
        // .obj + .mtl + texture images, all from the same source
        // directory. A .obj file's `mtllib` and the .mtl's `map_*`
        // refs are relative to the .obj's parent dir, so every file
        // in the group MUST stay in the same zip directory
        // (preserving sibling relative paths) or the mesh loader
        // won't find the .mtl / textures on extract.
        //
        // Strategy:
        //   - If every file in the group shares one parent dir
        //     under showDir: default per-file show-relative logic is
        //     already correct — sibling structure is preserved.
        //   - If they share one parent dir OUTSIDE showDir: allocate
        //     a `Objects/<parentBasename>/` subdir for this group
        //     and put every file there by basename.
        //   - If they don't share one parent: fall back to per-file
        //     placement (existing logic).
        std::filesystem::path sharedParent;
        bool sharedParentValid = true;
        for (const auto& r : resolved) {
            auto parent = std::filesystem::path(r.abs).parent_path();
            if (sharedParent.empty()) {
                sharedParent = parent;
            } else if (sharedParent != parent) {
                sharedParentValid = false;
                break;
            }
        }

        std::string groupSubdir;
        if (sharedParentValid && !sharedParent.empty() &&
            !isUnderCanonDir(sharedParent, canonShowDir)) {
            // External group — give the whole thing its own subdir
            // under Objects/ so the mesh's sibling refs resolve after
            // extract. Dedupe at the subdir level (not the filename
            // level) so two meshes with a same-named parent dir
            // don't collide.
            std::string parentName = sharedParent.filename().string();
            if (parentName.empty()) parentName = "object";
            groupSubdir = "Objects/" + parentName;
            int n = 2;
            while (claimedGroupDirs.count(groupSubdir)) {
                groupSubdir = "Objects/" + parentName + "_" + std::to_string(n++);
            }
            claimedGroupDirs.insert(groupSubdir);
        }

        for (const auto& r : resolved) {
            if (outToPack.count(r.abs)) continue;

            std::string zipPath;
            if (!groupSubdir.empty()) {
                // External group co-location
                std::string filename = std::filesystem::path(r.abs).filename().string();
                zipPath = groupSubdir + "/" + filename;
                // Files from the same source dir have unique basenames
                // (they're file-system siblings), so this should
                // always be fresh — but claim it anyway so an
                // unrelated later pack path can't accidentally reuse.
                claimed.insert(zipPath);
            } else {
                // Per-file placement. Mesh-type files default to
                // `Objects/` (matches the convention that house
                // meshes live under `<showDir>/Objects/`); other
                // files use the manager's defaultSubdir.
                std::string ext;
                {
                    auto e = std::filesystem::path(r.abs).extension().string();
                    for (auto& c : e) c = (char)std::tolower(c);
                    ext = e;
                }
                std::string subdir = defaultSubdir;
                if (ext == ".obj" || ext == ".glb" || ext == ".gltf" || ext == ".stl") {
                    subdir = "Objects";
                }
                zipPath = computeZipPath(std::filesystem::path(r.abs), showDir, canonShowDir,
                                         subdir, claimed);
            }

            outToPack[r.abs] = zipPath;
            if (r.raw != zipPath) {
                outRewrites[r.raw] = zipPath;
            }
            if (r.abs != r.raw && r.abs != zipPath) {
                outRewrites[r.abs] = zipPath;
            }
        }

        // Face images are processed independently of the mesh group-colocation
        // logic above. They always land under "Faces/" when external, preserving
        // their parent directory name so e.g. Faces/Woody/Woody_AI.png stays grouped.
        for (const auto& raw : faceRefs) {
            if (raw.empty()) continue;
            std::string abs = resolveToAbsolute(showDir, raw);
            if (abs.empty()) {
                std::string msg = "Could not locate face image '" + raw + "' referenced by model '" + it->first + "' — not included in package";
                spdlog::warn("Pack: {}", msg);
                outWarnings.push_back(msg);
                continue;
            }
            if (outToPack.count(abs)) continue;
            // Place under show-relative path if inside showDir, otherwise
            // use Faces/<parentBasename>/<filename> to preserve grouping.
            std::string zipPath;
            if (isUnderCanonDir(std::filesystem::path(abs), canonShowDir)) {
                std::error_code ec;
                auto rel = std::filesystem::relative(abs, showDir, ec);
                if (!ec && !rel.empty()) {
                    zipPath = rel.generic_string();
                    claimed.insert(zipPath);
                }
            }
            if (zipPath.empty()) {
                auto absPath = std::filesystem::path(abs);
                std::string parentName = absPath.parent_path().filename().string();
                if (parentName.empty()) parentName = "Faces";
                std::string subdir = "Faces/" + parentName;
                zipPath = claimUniqueZipPath(claimed, subdir, absPath.filename().string());
            }
            outToPack[abs] = zipPath;
            if (raw != zipPath) outRewrites[raw] = zipPath;
            if (abs != raw && abs != zipPath) outRewrites[abs] = zipPath;
        }
    }
}

} // namespace

bool SequencePackage::Pack(const std::filesystem::path& outputXsqz,
                           const std::string& showDir,
                           const std::string& sequenceXsqPath,
                           const std::string& audioPath,
                           const std::vector<std::string>& altAudioPaths,
                           const std::vector<std::string>& extraFiles,
                           SequenceMedia& media,
                           ModelManager& models,
                           ViewObjectManager& viewObjects,
                           SequenceElements& seqElements,
                           const SequencePackOptions& options,
                           std::vector<std::string>* outWarnings,
                           ProgressCallback progress)
{
    // Local buffer so helpers can accumulate warnings unconditionally.
    // Swapped into the caller's `outWarnings` at the end (when non-null)
    // so a single nullable-check at the API boundary is all we need.
    std::vector<std::string> warnings;

    auto tick = [&](int pct) {
        if (progress) progress(pct);
    };
    tick(0);

    if (!FileExists(sequenceXsqPath)) {
        spdlog::error("Pack: sequence .xsq does not exist at '{}'", sequenceXsqPath);
        return false;
    }
    // xlights_rgbeffects.xml + xlights_networks.xml are derived from
    // showDir — they always live at its root. Networks is packed as
    // an extra; rgbeffects is rewritten + packed explicitly below.
    std::string rgbEffectsXmlPath =
        (std::filesystem::path(showDir) / "xlights_rgbeffects.xml").string();
    std::string networksXmlPath =
        (std::filesystem::path(showDir) / "xlights_networks.xml").string();
    if (!FileExists(rgbEffectsXmlPath)) {
        spdlog::error("Pack: xlights_rgbeffects.xml does not exist at '{}'", rgbEffectsXmlPath);
        return false;
    }

    ObtainAccessToURL(outputXsqz.string(), /*enforceWritable=*/true);

    // Stage 1 — collect everything we'll pack. Builds both
    // {absolute path → zip-relative path} for file contents and
    // {original reference string → new zip-relative} for path
    // rewriting inside rgbeffects + .xsq.
    std::map<std::string, std::string> rewrites;            // originalRef → zipPath
    std::map<std::string, std::string> absToZip;            // absPath → zipPath
    std::set<std::string> claimedZipPaths;

    // Reserve the well-known root entries so nothing else claims them.
    claimedZipPaths.insert("xlights_rgbeffects.xml");
    auto xsqBasename = std::filesystem::path(sequenceXsqPath).filename().string();
    claimedZipPaths.insert(xsqBasename);

    // Tracks whole-subdir claims for per-object group colocation
    // (mesh + .mtl + textures kept together). Distinct from the
    // per-file `claimedZipPaths` set.
    std::set<std::string> claimedGroupDirs;

    // Canonicalize showDir once — `isUnderCanonDir` gets called
    // hundreds of times for shows with many model asset refs.
    std::filesystem::path canonShowDir;
    if (!showDir.empty()) {
        std::error_code ec;
        canonShowDir = std::filesystem::weakly_canonical(std::filesystem::path(showDir), ec);
        if (ec) canonShowDir = std::filesystem::path(showDir);
    }

    collectFromSequenceMedia(media, showDir, canonShowDir, options.excludeVideos,
                             rewrites, absToZip, claimedZipPaths, warnings);
    tick(20);

    auto modelFacesUsed = collectFacesUsedInSequence(seqElements);

    collectFromObjectManager(models, showDir, canonShowDir, "Images",
                             modelFacesUsed, rewrites, absToZip, claimedZipPaths, claimedGroupDirs, warnings);
    tick(35);

    collectFromObjectManager(viewObjects, showDir, canonShowDir, "Objects",
                             modelFacesUsed, rewrites, absToZip, claimedZipPaths, claimedGroupDirs, warnings);
    tick(45);

    // Font files used by Text / Shape / other effects with FONTPICKER controls.
    // Placed under Fonts/ in the zip; no path rewrite needed because font
    // strings store a face name, not a file path — the importer registers
    // the bundled file with AddPrivateFont() before rendering.
    collectFontFiles(seqElements, absToZip, claimedZipPaths, warnings);
    tick(50);

    // Audio is treated separately — comes from the sequence header,
    // not from SequenceMedia (audio isn't cached there). Primary
    // audio plus any alternate tracks land at the zip root under
    // their basenames so the imported sequence finds them without
    // any path rewrite. Both are gated by `excludeAudio`.
    auto queueAudio = [&](const std::string& raw) {
        if (raw.empty()) return;
        std::string abs = resolveToAbsolute(showDir, raw);
        if (abs.empty()) {
            std::string msg = "Audio file '" + raw + "' not found — not included in package";
            spdlog::warn("Pack: {}", msg);
            warnings.push_back(msg);
            return;
        }
        if (absToZip.count(abs)) return;  // already queued
        auto basename = std::filesystem::path(abs).filename().string();
        std::string zipPath = claimUniqueZipPath(claimedZipPaths, std::string(), basename);
        absToZip[abs] = zipPath;
        if (raw != zipPath)            rewrites[raw] = zipPath;
        if (abs != raw && abs != zipPath) rewrites[abs] = zipPath;
    };

    if (!options.excludeAudio) {
        queueAudio(audioPath);
        for (const auto& alt : altAudioPaths) queueAudio(alt);
    }

    // xlights_networks.xml is auto-included when present (show
    // folders without any controller config won't have one — that's
    // fine, we just skip it silently). Callers shouldn't pass it in
    // `extraFiles`; deriving it here keeps the API clean.
    if (FileExists(networksXmlPath)) {
        std::error_code ec;
        auto abs = std::filesystem::absolute(networksXmlPath, ec);
        std::string absStr = ec ? networksXmlPath : abs.string();
        if (!absToZip.count(absStr)) {
            absToZip[absStr] = "xlights_networks.xml";
            claimedZipPaths.insert("xlights_networks.xml");
        }
    }

    // Extra caller-supplied files (house image, data-layer .iseq
    // sources, etc.). Each goes to its show-relative path when
    // under showDir, else to the zip root (we don't know its type
    // well enough to pick a subdir).
    for (const auto& raw : extraFiles) {
        if (raw.empty()) continue;
        std::string abs = resolveToAbsolute(showDir, raw);
        if (abs.empty()) {
            std::string msg = "File '" + raw + "' not found — not included in package";
            spdlog::warn("Pack: {}", msg);
            warnings.push_back(msg);
            continue;
        }
        if (absToZip.count(abs)) continue;
        std::string zipPath = computeZipPath(std::filesystem::path(abs), showDir, canonShowDir,
                                             std::string(), claimedZipPaths);
        absToZip[abs] = zipPath;
        if (raw != zipPath)            rewrites[raw] = zipPath;
        if (abs != raw && abs != zipPath) rewrites[abs] = zipPath;
    }

    // Stage 2 — load + rewrite the two XML documents. Originals on
    // disk are untouched; we modify copies before packing.
    //
    // Flatten the rewrites map to a vector and sort descending by
    // original-path length so longer paths match first. Without
    // this, a short path that is a prefix of a longer one can
    // consume the longer path's head when the map's alphabetical
    // iteration order puts the short one first (e.g. rewriting
    // `/foo/bar.png` inside `/foo/bar.png.bak`).
    std::vector<std::pair<std::string, std::string>> sortedRewrites(rewrites.begin(), rewrites.end());
    std::sort(sortedRewrites.begin(), sortedRewrites.end(),
              [](const auto& a, const auto& b) {
                  return a.first.size() > b.first.size();
              });

    pugi::xml_document rgbEffectsDoc;
    if (!rgbEffectsDoc.load_file(rgbEffectsXmlPath.c_str())) {
        spdlog::error("Pack: could not parse '{}'", rgbEffectsXmlPath);
        return false;
    }
    rewritePathsInXml(rgbEffectsDoc.document_element(), sortedRewrites);

    pugi::xml_document xsqDoc;
    if (!xsqDoc.load_file(sequenceXsqPath.c_str())) {
        spdlog::error("Pack: could not parse '{}'", sequenceXsqPath);
        return false;
    }
    rewritePathsInXml(xsqDoc.document_element(), sortedRewrites);
    tick(55);

    // Stage 3 — open the output zip (to a .tmp sibling) and write
    // everything. Atomic rename at the end guards against
    // partial-write corruption.
    std::filesystem::path tmpPath = outputXsqz;
    tmpPath += ".tmp";
    std::error_code ec;
    std::filesystem::remove(tmpPath, ec);

    zipFile zf = zipOpen(tmpPath.string().c_str(), APPEND_STATUS_CREATE);
    if (zf == nullptr) {
        spdlog::error("Pack: zipOpen failed for '{}'", tmpPath.string());
        return false;
    }

    bool ok = true;

    // Serialize the rewritten XMLs to strings so we can hand them
    // directly to the zip without a temp file.
    struct MemWriter : pugi::xml_writer {
        std::string out;
        void write(const void* data, size_t size) override {
            out.append(static_cast<const char*>(data), size);
        }
    };
    {
        MemWriter mw;
        rgbEffectsDoc.save(mw, "  ");
        ok = addBufferToZip(zf, "xlights_rgbeffects.xml", mw.out);
    }
    if (ok) {
        MemWriter mw;
        xsqDoc.save(mw, "  ");
        ok = addBufferToZip(zf, xsqBasename, mw.out);
    }
    tick(65);

    // All other files — media, model + object assets, audio tracks,
    // caller extras. They're all in absToZip. A per-file write
    // failure (source can't be opened, zip layer refuses the entry)
    // is collected as a warning so the caller can explain what's
    // missing; we keep going and produce the best package we can.
    // Only catastrophic pre-loop state (bad zipOpen, XML parse
    // failure, rename failure) aborts the whole pack.
    int i = 0;
    int total = static_cast<int>(absToZip.size());
    int skipped = 0;
    for (const auto& kv : absToZip) {
        if (!addFileToZip(zf, kv.second, std::filesystem::path(kv.first))) {
            std::string msg = "Could not include '" + kv.first
                              + "' (permission denied or unreadable) — not in package";
            spdlog::warn("Pack: {}", msg);
            warnings.push_back(msg);
            ++skipped;
        }
        ++i;
        if (total > 0) {
            tick(65 + (35 * i) / total);
        }
    }

    zipClose(zf, nullptr);

    // Catastrophic failures (XML writes) — caller gets nothing.
    // `ok` at this point reflects only the rgbeffects + xsq writes;
    // per-file failures went to the warnings list instead.
    if (!ok) {
        spdlog::error("Pack: core XML writes failed — aborting without producing '{}'",
                      outputXsqz.string());
        std::filesystem::remove(tmpPath, ec);
        if (outWarnings) {
            outWarnings->insert(outWarnings->end(),
                                warnings.begin(), warnings.end());
        }
        return false;
    }

    std::filesystem::rename(tmpPath, outputXsqz, ec);
    if (ec) {
        spdlog::error("Pack: rename '{}' -> '{}' failed: {}",
                      tmpPath.string(), outputXsqz.string(), ec.message());
        std::filesystem::remove(tmpPath, ec);
        if (outWarnings) {
            outWarnings->insert(outWarnings->end(),
                                warnings.begin(), warnings.end());
        }
        return false;
    }

    tick(100);
    spdlog::info("Pack: wrote '{}' ({} assets packed, {} skipped)",
                 outputXsqz.string(),
                 absToZip.size() - static_cast<size_t>(skipped),
                 skipped);
    if (outWarnings) {
        outWarnings->insert(outWarnings->end(),
                            warnings.begin(), warnings.end());
    }
    return true;
}
