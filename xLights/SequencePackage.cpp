/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/xml/xml.h>
#include <wx/log.h>
#include <wx/time.h>
#include <wx/dir.h>
#include <wx/progdlg.h>

#include "SequencePackage.h"
#include "xLightsMain.h"
#include "ExternalHooks.h"

#include <log4cpp/Category.hh>

static const std::string IMPORTED_MEDIA = "ImportedMedia";
static const std::string SUBFLD_IMAGES = "Images";
static const std::string SUBFLD_VIDEOS = "Videos";
static const std::string SUBFLD_FACES = "DownloadedFaces";
static const std::string SUBFLD_SHADERS = "Shaders";
static const std::string SUBFLD_GLEDIATORS = "Glediators";
static const char PATH_SEP = wxFileName::GetPathSeparator();

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

SequencePackage::SequencePackage(const wxFileName& fileName, xLightsFrame* xlights)
{
    _xlights = xlights;

    if (fileName.GetExt() == "zip" || fileName.GetExt() == "piz") {
        _xsqOnly = false;
        _pkgFile = fileName;
    } else {
        _xsqOnly = true;
        _xsqFile = fileName;
    }
}

SequencePackage::~SequencePackage()
{
    // cleanup extracted files
    if (!_xsqOnly && _tempDir.Exists()) {
        wxDir::Remove(_tempDir.GetFullPath(), wxPATH_RMDIR_RECURSIVE);
    }
}

void SequencePackage::InitDefaultImportOptions()
{
    // Set default target media directories based on a few assumptions. User
    // can still change these in the Mapping Dialog to whatever they would like.

    std::string showFolder = _xlights->GetShowDirectory();

    // always default faces/shaders to default download folder as they tend to be reused
    _importOptions.SetDir(MediaTargetDir::FACES_DIR, wxString::Format("%s%c%s", showFolder, PATH_SEP, SUBFLD_FACES), true);
    _importOptions.SetDir(MediaTargetDir::SHADERS_DIR, wxString::Format("%s%c%s", showFolder, PATH_SEP, SUBFLD_SHADERS), true);

    wxFileName targetXsq(_xlights->GetSeqXmlFileName());
    wxString targetDir = targetXsq.GetPath();

    wxString mediaBaseFolder;

    if (targetDir.ToStdString() != showFolder) {
        // target xsq is not at the root of show folder, assume user manages show folder
        // with a subfolder per sequence as Gil noted
        mediaBaseFolder = targetDir;
    } else {
        // default images/videos/glediators into new ImportedMedia/<NameOfSrcXsq>/<MediaType>
        mediaBaseFolder = wxString::Format("%s%c%s%c%s", showFolder, PATH_SEP, IMPORTED_MEDIA, PATH_SEP, _xsqName);
    }

    // set the defaults for media sub folders
    _importOptions.SetDir(MediaTargetDir::GLEDIATORS_DIR, wxString::Format("%s%c%s", mediaBaseFolder, PATH_SEP, SUBFLD_GLEDIATORS), true);
    _importOptions.SetDir(MediaTargetDir::IMAGES_DIR, wxString::Format("%s%c%s", mediaBaseFolder, PATH_SEP, SUBFLD_IMAGES), true);
    _importOptions.SetDir(MediaTargetDir::VIDEOS_DIR, wxString::Format("%s%c%s", mediaBaseFolder, PATH_SEP, SUBFLD_VIDEOS), true);
}

void SequencePackage::Extract()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_xsqOnly) {
        return;
    }

    wxProgressDialog prog("Extract Package", "Extracting Sequence Package...", 100, _xlights, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    prog.Show();

    wxFileInputStream fis(_pkgFile.GetFullPath());

    if (!fis.IsOk()) {
        logger_base.error("Could not open the Sequence Package '%s'", (const char*)_pkgFile.GetFullName().c_str());
        prog.Update(100);
        return;
    }

    _tempDir = wxString::Format("%s%c%s_%lld", wxFileName::GetTempDir(), wxFileName::GetPathSeparator(), _pkgFile.GetName(), wxGetUTCTimeMillis());
    logger_base.debug("Extracting Sequence Package '%s' to '%s'", (const char*)_pkgFile.GetFullName().c_str(), (const char*)_tempDir.GetFullPath().c_str());

    wxZipInputStream zis(fis);
    std::unique_ptr<wxZipEntry> upZe;

    if (!zis.IsOk()) {
        logger_base.error("Could not open the zip file '%s'", (const char*)_pkgFile.GetFullName().c_str());
        prog.Update(100);
        return;
    }

    if (zis.GetTotalEntries() == 0) {
        logger_base.error("No entries found in zip file '%s'", (const char*)_pkgFile.GetFullName().c_str());
        prog.Update(100);
        return;
    }

    // start extracting each entry
    upZe.reset(zis.GetNextEntry());

    prog.Update(10);
    int numEntryProcessed = 0;
    int progStep = (int)(std::floor(90 / zis.GetTotalEntries()));

    while (upZe != nullptr) {
        wxString fnEntry = wxString::Format("%s%c%s", _tempDir.GetFullPath(), wxFileName::GetPathSeparator(), upZe->GetName());

        if (fnEntry.Contains("__MACOSX")) {
            logger_base.debug("   skipping MACOS Folder %s.", (const char*)fnEntry.c_str());
            upZe.reset(zis.GetNextEntry());
            continue;
        }

#ifdef __WXMSW__
        // folder with spaces at begin and end breaks temp folder paths
        fnEntry.Replace(" " + wxString(wxFileName::GetPathSeparator()), wxFileName::GetPathSeparator());
        fnEntry.Replace(wxString(wxFileName::GetPathSeparator()) + " ", wxFileName::GetPathSeparator());
#endif

        wxFileName fnOutput;
        upZe->IsDir() ? fnOutput.AssignDir(fnEntry) : fnOutput.Assign(fnEntry);

        logger_base.debug("   Extracting %s to %s.", (const char*)fnEntry.c_str(), (const char*)fnOutput.GetFullPath().c_str());

#ifdef __WXMSW__
        if (fnOutput.GetFullPath().length() > MAX_PATH) {
            logger_base.warn("Target filename longer than %d chars (%d). This will likely fail. %s.", MAX_PATH, (int)fnOutput.GetFullPath().length(), (const char*) fnOutput.GetFullPath().c_str());
        }
#endif

        // Create output dir in temp if needed
        if (!wxDirExists(fnOutput.GetPath())) {
            wxFileName::Mkdir(fnOutput.GetPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
        }

        // handle file output
        if (!upZe->IsDir()) {
            if (!zis.CanRead()) {
                logger_base.error("Could not read file from package '%s'", (const char*)upZe->GetName().c_str());
            } else {
                wxFileOutputStream fos(fnOutput.GetFullPath());

                if (!fos.IsOk()) {
                    logger_base.error("Could not create sequence file at '%s'", (const char*)fnOutput.GetFullName().c_str());
                } else {
                    zis.Read(fos);
                    wxString ext = fnOutput.GetExt();

                    if (ext == "xsq") {
                        _xsqFile = fnOutput;
                        _xsqName = fnOutput.GetName();
                    } else if (ext == "xml") {
                        if (fnOutput.GetName() == "xlights_rgbeffects") {
                            wxXmlDocument rgbEffects;
                            if (rgbEffects.Load(fnOutput.GetFullPath())) {
                                _rgbEffects = rgbEffects;
                                _pkgRoot = fnOutput.GetPath();
                                _hasRGBEffects = true;
                            }
                        } else if (fnOutput.GetName() == "xlights_networks") {
                            _xlNetworks = fnOutput;
                        } else {
                            wxXmlDocument doc;
                            if (doc.Load(fnOutput.GetFullPath())) {
                                if (doc.GetRoot()->GetName() == "xsequence") {
                                    _xsqFile = fnOutput;
                                    _xsqName = fnOutput.GetName();
                                }
                            }
                        }
                    } else {
                        // assume other files are media for effects, images/videos/gediators/shaders/faces/etc
                        _media[fnOutput.GetFullName()] = fnOutput;
                    }
                }
                fos.Close();
            }
        }

        numEntryProcessed++;
        prog.Update(10 + progStep * numEntryProcessed);

        // get next zip entry
        upZe.reset(zis.GetNextEntry());
    }

    // seems silly but if we don't call this twice the dialog sticks around after
    // this method has completed if the main app is busy, for example opening another
    // modal dialog like Mapping Dialog
    prog.Update(100);
    prog.Update(100);

    if (!_xsqFile.IsOk() || !FileExists(_xsqFile)) {
        logger_base.error("No sequence file found in package '%s'", (const char*)_pkgFile.GetFullName().c_str());
    } else {
        InitDefaultImportOptions();
    }
}

void SequencePackage::FindRGBEffectsFile()
{
    wxString showDir = wxPathOnly(_xsqFile.GetFullPath());
    if( wxFile::Exists(showDir + wxFileName::GetPathSeparator() + "xlights_rgbeffects.xml")) {
        wxXmlDocument rgbEffects;
        if (rgbEffects.Load(showDir + wxFileName::GetPathSeparator() + "xlights_rgbeffects.xml")) {
            _rgbEffects = rgbEffects;
            _hasRGBEffects = true;
        }
    }
}

bool SequencePackage::IsValid() const
{
    if (_xsqOnly) {
        return _xsqFile.IsOk() && FileExists(_xsqFile);
    } else {
        return _xsqFile.IsOk() && FileExists(_xsqFile) && _rgbEffects.IsOk();
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

wxFileName& SequencePackage::GetXsqFile()
{
    return _xsqFile;
}

wxXmlDocument& SequencePackage::GetRgbEffectsFile()
{
    return _rgbEffects;
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

std::string SequencePackage::FixAndImportMedia(Effect* mappedEffect, EffectLayer* target)
{
    auto settings = mappedEffect->GetSettings();
    wxString effName = mappedEffect->GetEffectName();

    wxString settingEffectFile = wxEmptyString;
    wxString targetMediaFolder = wxEmptyString;

    if (effName == "Pictures") {
        settingEffectFile = "E_FILEPICKER_Pictures_Filename";
        targetMediaFolder = _importOptions.GetDir(MediaTargetDir::IMAGES_DIR);
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
        wxString faceName = settings["E_CHOICE_Faces_FaceDefinition"];
        if (faceName != wxEmptyString) {
            ImportFaceInfo(mappedEffect, target, faceName);
        }
    } else if (effName == "Shape") {
        wxString shapePath = settings["E_FILEPICKERCTRL_SVG"];
        if (!shapePath.empty()) {
            settingEffectFile = "E_FILEPICKERCTRL_SVG";
            targetMediaFolder = _importOptions.GetDir(MediaTargetDir::IMAGES_DIR);
        }
    } else if (effName == "Ripple") {
        wxString shapePath = settings["E_FILEPICKERCTRL_Ripple_SVG"];
        if (!shapePath.empty()) {
            settingEffectFile = "E_FILEPICKERCTRL_Ripple_SVG";
            targetMediaFolder = _importOptions.GetDir(MediaTargetDir::IMAGES_DIR);
        }
    }

    if (!settingEffectFile.IsEmpty()) {
        wxString settingPath = settings.Get(settingEffectFile, "");

        // normalize path so wxFileName can properly parse it, we just need
        // it to parse properly so we can get to the real filename
        wxFileName picFilePath;
        if (settingPath.Contains("\\")) {
            picFilePath = wxFileName(settingPath, wxPATH_DOS);
        } else {
            picFilePath = wxFileName(settingPath);
        }

        // import the asset if we have it, otherwise track it as missing
        wxFileName fileToCopy = _media[picFilePath.GetFullName()];

        if (fileToCopy.IsOk() && FileExists(fileToCopy)) {
            wxFileName copiedAsset = CopyMediaToTarget(targetMediaFolder, fileToCopy);
            settings.erase(settingEffectFile);
            wxString newSetting = copiedAsset.GetFullPath().ToStdString();
            settings[settingEffectFile] = newSetting;
        } else {
            if (picFilePath != "")
                _missingMedia.push_back(picFilePath.GetFullName().ToStdString());
        }
    }

    return settings.AsString();
}

void SequencePackage::ImportFaceInfo(Effect* mappedEffect, EffectLayer* target, const std::string& faceName)
{
    auto targetModelName = target->GetParentElement()->GetModelName();
    auto srcModelName = mappedEffect->GetParentEffectLayer()->GetParentElement()->GetModelName();
    Model* targetModel = _xlights->AllModels[targetModelName];

    const auto& faceInfo = targetModel->faceInfo.find(faceName);
    if (faceInfo != targetModel->faceInfo.end()) {
        // face already defined don't overwrite it
        return;
    }

    wxXmlDocument srcRgbEffects;
    if (_rgbEffects.IsOk()) {
        wxXmlNode* modelsNode = nullptr;
        for (wxXmlNode* node = _rgbEffects.GetRoot()->GetChildren(); node != nullptr; node = node->GetNext()) {
            if (node->GetName() == "models") {
                modelsNode = node;
                break;
            }
        }

        if (modelsNode != nullptr) {
            wxXmlNode* modelNode = nullptr;
            for (wxXmlNode* model = modelsNode->GetChildren(); model != nullptr; model = model->GetNext()) {
                if (model->GetAttribute("name") == srcModelName) {
                    modelNode = model;
                    break;
                }
            }

            if (modelNode != nullptr) {
                // find faceInfo node
                for (wxXmlNode* modelChild = modelNode->GetChildren(); modelChild != nullptr; modelChild = modelChild->GetNext()) {
                    if (modelChild->GetName() == "faceInfo") {
                        wxString name = modelChild->GetAttribute("Name", "");
                        wxString type = modelChild->GetAttribute("Type", "");

                        // only import if type is matrix
                        if ((name == faceName || faceName == "Default") && type == "Matrix") {
                            wxXmlNode* newFaceInfo = new wxXmlNode(wxXML_ELEMENT_NODE, "faceInfo");

                            for (wxXmlAttribute* attr = modelChild->GetAttributes(); attr != nullptr; attr = attr->GetNext()) {
                                wxString attrName = attr->GetName();
                                wxString attrValue = attr->GetValue();

                                // import files
                                if (attrName.Left(5) == "Mouth" || attrName.Left(4) == "Eyes") {
                                    if (attrValue != wxEmptyString) {
                                        // normalize path so wxFileName can properly parse it, we just need
                                        // it to parse properly so we can get to the real filename
                                        wxFileName faceFile;
                                        if (attrValue.Contains("\\")) {
                                            faceFile = wxFileName(attrValue, wxPATH_DOS);
                                        } else {
                                            faceFile = wxFileName(attrValue);
                                        }

                                        // import the asset if we have it, otherwise track it as missing
                                        wxFileName fileToCopy = _media[faceFile.GetFullName()];

                                        if (fileToCopy.IsOk() && FileExists(fileToCopy)) {
                                            wxFileName copiedAsset = CopyMediaToTarget(_importOptions.GetDir(MediaTargetDir::FACES_DIR), fileToCopy);
                                            newFaceInfo->AddAttribute(attrName, copiedAsset.GetFullPath());
                                        } else {
                                            _missingMedia.push_back(fileToCopy.GetFullName().ToStdString());
                                        }
                                    }
                                } else {
                                    newFaceInfo->AddAttribute(attrName, attrValue);
                                }
                            }

                            targetModel->AddFace(newFaceInfo);
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

wxFileName SequencePackage::CopyMediaToTarget(const std::string& targetFolder, const wxFileName& mediaToCopy)
{
    wxFileName targetFile = wxString::Format("%s%c%s", targetFolder, PATH_SEP, mediaToCopy.GetFullName());

    // Only import if file doesn't alrady exist in target folder
    if (!FileExists(targetFile)) {
        // make sure dir exists first
        if (!wxDirExists(targetFile.GetPath())) {
            wxFileName::Mkdir(targetFile.GetPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
        }

        // now copy the asset
        wxCopyFile(mediaToCopy.GetFullPath(), targetFile.GetFullPath());
    }

    return targetFile;
}
