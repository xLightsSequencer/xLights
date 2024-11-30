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

#include <wx/filename.h>
#include <wx/xml/xml.h>

#include <map>

#include "sequencer/Element.h"
#include "sequencer/Effect.h"

class xLightsFrame;

enum class MediaTargetDir
{
    FACES_DIR,
    GLEDIATORS_DIR,
    IMAGES_DIR,
    SHADERS_DIR,
    VIDEOS_DIR,
};


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
        SequencePackage(const wxFileName& zipFile, xLightsFrame* xlights);
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
        wxFileName& GetXsqFile();
        wxXmlDocument& GetRgbEffectsFile();
        std::string GetTempDir() const;

        std::string FixAndImportMedia(Effect* mappedEffect, EffectLayer *target);
        void ImportFaceInfo(Effect* mappedEffect, EffectLayer *target, const std::string& faceName);
        wxFileName CopyMediaToTarget(const std::string& targetFolder, const wxFileName& mediaToCopy);
        std::list<std::string> GetMissingMedia();
    private:
        xLightsFrame*   _xlights;
        bool            _xsqOnly = true;
        bool            _hasRGBEffects{false};
        wxFileName      _xsqFile;
        std::string     _xsqName;
        wxFileName      _pkgFile;
        wxFileName      _tempDir;
        wxXmlDocument   _rgbEffects;
        wxFileName      _xlNetworks;
        wxFileName _xlEffects;
        wxFileName _pkgRoot;
        bool _leaveFiles = false;
        std::list<std::string> _missingMedia;
        std::map<std::string, wxFileName> _media;
        bool _modelsChanged = false;
        SeqPkgImportOptions _importOptions;
    
        void InitDefaultImportOptions();
};
