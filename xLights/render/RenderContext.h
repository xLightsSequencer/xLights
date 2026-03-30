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

#include <list>
#include <string>

class AudioManager;
class EffectManager;
class Model;
class OutputModelManager;
class SequenceElements;
class UICallbacks;
enum class HEADER_INFO_TYPES;

// Abstract interface replacing direct xLightsFrame* dependencies in
// render/, effects/, and models/ code.  xLightsFrame implements this
// so existing callers can be migrated one at a time.

class RenderContext {
public:
    virtual ~RenderContext() = default;

    // ---- directory / file management ----
    virtual const std::string& GetShowDirectory() const = 0;
    virtual const std::string& GetFseqDirectory() const = 0;
    virtual const std::list<std::string>& GetMediaFolders() const = 0;

    virtual bool IsInShowFolder(const std::string& file) const = 0;
    virtual bool IsInShowOrMediaFolder(const std::string& file) const = 0;
    virtual std::string MoveToShowFolder(const std::string& file,
                                         const std::string& subdirectory,
                                         bool reuse = false) = 0;
    virtual std::string MakeRelativePath(const std::string& file) const = 0;

    // ---- sequence state ----
    virtual SequenceElements& GetSequenceElements() = 0;
    virtual AudioManager* GetCurrentMediaManager() const = 0;
    virtual const std::string& GetHeaderInfo(HEADER_INFO_TYPES type) const = 0;

    // ---- model access ----
    virtual Model* GetModel(const std::string& name) const = 0;

    // ---- managers ----
    virtual EffectManager& GetEffectManager() = 0;
    virtual OutputModelManager* GetOutputModelManager() = 0;

    // ---- rendering control ----
    virtual bool AbortRender(int maxTimeMs = 60000) = 0;
    virtual void RenderMainThreadEffects() {}

    // ---- misc ----
    virtual void SuspendAutoSave(bool suspend) = 0;
    virtual bool IsCheckSequenceOptionDisabled(const std::string& option) const { return false; }

    // ---- UI callbacks (nullptr when running headless) ----
    virtual UICallbacks* GetUICallbacks() { return nullptr; }
};
