#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <filesystem>
#include <functional>
#include <pugixml.hpp>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "render/Element.h"
#include "render/Effect.h"

class ModelManager;

enum class MediaTargetDir
{
    FACES_DIR,
    GLEDIATORS_DIR,
    IMAGES_DIR,
    SHADERS_DIR,
    VIDEOS_DIR,
};

using ProgressCallback = std::function<bool(int pct)>;

class SeqPkgImportOptions {
    public:
        SeqPkgImportOptions();
        virtual ~SeqPkgImportOptions();
        void SetImportActive(bool active);
        bool IsImportActive() const;
        void SetDir(MediaTargetDir dirType, const std::string& dirPath, bool saveAsDefault = false);
        std::string GetDir(MediaTargetDir dirType);
        void RestoreDefaults();
    private:
        bool _importActive = true;
        std::map<MediaTargetDir, std::string> _mediaDirs;
        std::map<MediaTargetDir, std::string> _defaultDirs;
};

class SequencePackage {
    public:
        SequencePackage(const std::filesystem::path& zipFile, const std::string& showDir, const std::string& seqXmlFileName, ModelManager* models);
        virtual ~SequencePackage();
        void Extract();
        void FindRGBEffectsFile();
        bool IsValid() const;
        bool IsPkg();
        bool HasMedia() const;
        bool HasRGBEffects() const;
        bool HasMissingMedia() const;
        bool ModelsChanged() const;
        void SetLeaveFiles(bool leave) {
         _leaveFiles = leave;
        }
        std::string GetTempShowFolder() const;
        SeqPkgImportOptions* GetImportOptions();
        const std::filesystem::path& GetXsqFile();
        pugi::xml_document& GetRgbEffectsFile();
        std::string GetTempDir() const;

        std::string FixAndImportMedia(Effect* mappedEffect, EffectLayer *target);
        void ImportFaceInfo(Effect* mappedEffect, EffectLayer *target, const std::string& faceName);
        std::filesystem::path CopyMediaToTarget(const std::string& targetFolder, const std::filesystem::path& mediaToCopy);
        std::filesystem::path FindAndCopyAudio(const std::filesystem::path& targetDir);
        // Returns pairs of (shortname, copied_path) for alt tracks found in the package.
        // Entries with an empty path indicate tracks referenced in the XSQ but not bundled.
        std::vector<std::pair<std::string, std::string>> FindAndCopyAltAudioTracks(const std::filesystem::path& targetDir);
        std::list<std::string> GetMissingMedia();

        void SetSequenceElements(SequenceElements *se) { sequenceElements = se; };
        void SetProgressCallback(ProgressCallback cb) { _progressCb = std::move(cb); }
    private:
        std::string     _showDirectory;
        std::string     _seqXmlFileName;
        ModelManager*   _modelManager;
        bool            _xsqOnly = true;
        bool            _hasRGBEffects{false};
        std::filesystem::path _xsqFile;
        std::string     _xsqName;
        std::filesystem::path _pkgFile;
        std::filesystem::path _tempDir;
        pugi::xml_document _rgbEffects;
        std::filesystem::path _xlNetworks;
        std::filesystem::path _xlEffects;
        std::filesystem::path _pkgRoot;
        bool _leaveFiles = false;
        std::list<std::string> _missingMedia;
        std::map<std::string, std::filesystem::path> _media;
        bool _modelsChanged = false;
        SeqPkgImportOptions _importOptions;
        SequenceElements *sequenceElements;
        ProgressCallback _progressCb;

        void InitDefaultImportOptions();
};
