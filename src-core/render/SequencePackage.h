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
class ViewObjectManager;
class SequenceMedia;

/// Options controlling what a `SequencePackage::Pack()` run includes.
/// `excludeAudio` drops the sequence's audio track from the zip;
/// `excludeVideos` drops every video the SequenceMedia walk would
/// otherwise include. Both default false. (Previously there was an
/// "exclude presets" option on desktop; that option stripped the
/// `<effects>` node from rgbeffects, but presets are stored under a
/// different element now so the strip was a no-op. "Exclude Videos"
/// is the replacement since videos routinely carry copyright.)
struct SequencePackOptions {
    bool excludeAudio  = false;
    bool excludeVideos = false;
};

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

        // Pack the current temp directory back into a `.xsqz` at
        // `targetXsqz`, overwriting atomically (writes to a sibling
        // `.tmp`, then renames). Returns true on success. Used by the
        // iPad flow when the user opens a `.xsqz` from Files, edits,
        // and saves — the desktop has its own "Package Sequence"
        // dialog that doesn't go through here.
        bool Repack(const std::filesystem::path& targetXsqz);

        // Pack a self-contained `.xsqz` for the current in-memory
        // sequence + supporting managers. Walks SequenceMedia,
        // every model/view-object's `GetFileReferences()`, and
        // Matrix face images. `xlights_rgbeffects.xml` and
        // `xlights_networks.xml` are derived from `showDir` — no
        // need to pass them. `audioPath` + `altAudioPaths` are
        // gated by `excludeAudio`; `extraFiles` is for caller-
        // specific adjuncts (house background image, `.iseq`
        // data-layer sources).
        //
        // Per-file failures are collected into `outWarnings` as
        // human-readable strings; the package is still produced.
        // Only catastrophic failures (missing `.xsq` / rgbeffects,
        // zipOpen, XML parse, final rename) return false.
        // Atomic write via sibling `.tmp` + rename.
        static bool Pack(const std::filesystem::path& outputXsqz,
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
                         std::vector<std::string>* outWarnings = nullptr,
                         ProgressCallback progress = nullptr);

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
