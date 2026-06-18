#pragma once

#include <list>
#include <string>
#include <utility>

#include "diagnostics/CheckSequenceReport.h"

class Effect;
class Element;
class ModelManager;
class OutputManager;
class SequenceElements;
class SequenceFile;

// Optional callback surface for knobs that don't have a portable
// definition. Defaults make it a no-op for non-wx hosts (iPad);
// desktop overrides to wire in user prefs and the wx progress
// dialog.
class SequenceCheckerCallbacks {
public:
    virtual ~SequenceCheckerCallbacks() = default;

    // Desktop exposes per-check disable toggles in
    // Preferences → CheckSequence (`DupUniverse`, `NonContigCh`,
    // `PreviewGroup`, `DupNodeMG`, `TransTime`, `CustomSize`,
    // `SketchImage`). When disabled, the matching warning is still
    // recorded as an "INFO / checkdisabled" entry so the user can
    // see they suppressed it. Default: all checks enabled.
    virtual bool IsCheckOptionDisabled(const std::string& /*option*/) const {
        return false;
    }

    // Desktop's render-cache mode pref: "Disabled" / "Locked Only" /
    // "Enabled". Drives the Video render-cache warning. Default:
    // "Enabled" (no warning).
    virtual std::string GetRenderCacheMode() const {
        return "Enabled";
    }

    // Optional progress reporting — desktop drives a wxProgressDialog,
    // iPad ignores. Percent in [0,100].
    virtual void OnProgress(int /*percent*/, const std::string& /*step*/) {}

    // Per-platform video codec compatibility probe. Returns "" if
    // the file decodes cleanly on this platform, or a human-readable
    // reason. Apple platforms wire this to
    // `MediaCompatibility::CheckVideoFile`; desktop FFmpeg builds
    // can skip the probe entirely (return "").
    virtual std::string CheckVideoCompatibility(const std::string& /*path*/) {
        return "";
    }
};

// Wx-free port of the bulk of `xLightsFrame::CheckSequence`. Both
// desktop and iPad consume this; desktop additionally wraps it
// with the wx-only network socket probe + OS / preferences checks
// that don't apply on iPad.
//
// All inputs are core types (`SequenceElements`, `ModelManager`,
// `OutputManager`, `SequenceFile`). Construct on every check;
// the object is single-shot and not thread-safe.
class SequenceChecker {
public:
    SequenceChecker(SequenceElements& elements,
                    ModelManager& models,
                    OutputManager& outputs,
                    SequenceFile* sequenceFile,
                    std::string showFolder,
                    SequenceCheckerCallbacks* callbacks = nullptr);
    ~SequenceChecker() = default;

    SequenceChecker(const SequenceChecker&) = delete;
    SequenceChecker& operator=(const SequenceChecker&) = delete;

    // Runs every check this layer supports and writes to `report`.
    // Caller is responsible for adding `REPORT_SECTIONS` first.
    // Returns the total error count.
    int RunFullCheck(CheckSequenceReport& report);

    // Granular entry points — desktop's wx wrapper calls each in
    // sequence (interleaved with its own network / OS / prefs
    // chunks). iPad calls `RunFullCheck` from the bridge.
    int RunControllerChecks(CheckSequenceReport& report);
    int RunModelChecks(CheckSequenceReport& report);
    int RunSequenceChecks(CheckSequenceReport& report);
    int RunFileReferenceChecks(CheckSequenceReport& report);

private:
    // Per-effect / per-element body. Mirrors the desktop
    // `CheckEffect` / `CheckElement` shape; faces / states / view
    // points / file lists are accumulated across the whole sequence
    // walk and consumed in `RunSequenceChecks` to emit summary
    // issues (missing face image, undefined state, unused view).
    void CheckEffect(Effect* ef, CheckSequenceReport& report,
                     const std::string& elementName,
                     const std::string& modelName,
                     int layerIndex,
                     bool node,
                     bool& videoCacheWarning,
                     bool& disabledEffects,
                     std::list<std::pair<std::string, std::string>>& faces,
                     std::list<std::pair<std::string, std::string>>& states,
                     std::list<std::string>& viewPoints,
                     std::list<std::string>& allFiles);

    void CheckElement(Element* e, CheckSequenceReport& report,
                      const std::string& name,
                      const std::string& modelName,
                      bool& videoCacheWarning,
                      bool& disabledEffects,
                      std::list<std::pair<std::string, std::string>>& faces,
                      std::list<std::pair<std::string, std::string>>& states,
                      std::list<std::string>& viewPoints,
                      bool& usesShader,
                      std::list<std::string>& allFiles);

    // Recursive start-channel chain check for "model after model"
    // references. Returns true if the chain terminates normally;
    // false if a loop was detected (issue already added to report).
    bool CheckStart(CheckSequenceReport& report,
                    const std::string& startModel,
                    std::list<std::string>& seen,
                    std::string& nextModel);

    // Convenience wrapper that records an issue and increments the
    // shared counters. Mirrors desktop's `LogAndTrack` helper.
    void RecordIssue(CheckSequenceReport& report,
                     const std::string& sectionId,
                     const CheckSequenceReport::ReportIssue& issue);

    SequenceElements& _elements;
    ModelManager& _models;
    OutputManager& _outputs;
    SequenceFile* _sequenceFile;
    std::string _showFolder;
    SequenceCheckerCallbacks* _callbacks;

    // Mutable counters threaded through the granular Run* entry
    // points — kept as members so the desktop wrapper can read
    // running totals between sections.
    int _errors = 0;
    int _warnings = 0;
};
