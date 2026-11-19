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

#include <cstdint>
#include <optional>
#include <string>

#include "SequenceCommands.h"

class Effect;
class SequenceElements;

// Applies a stream of edits to one effect's type, settings and palette (an
// effect settings panel being changed) as ModifyEffect commands, and groups
// a burst of them into a single undo step.
//
// The first change of a burst opens a new undo step whose one entry puts
// back the effect's complete type, settings and palette as they were before
// the burst, so undo restores every key the burst touched, including keys it
// added or removed. Later changes in the burst still apply as commands and
// reach observers, but record no undo of their own.
//
// A burst goes on while each change is to the same effect, comes within
// `idleMS` of the previous one, and nothing else has touched the undo
// history since (no other step recorded, no undo or redo). EndBurst ends it
// explicitly (the user selected another effect).
class EffectEditCoalescer {
public:
    static constexpr int kDefaultIdleMS = 1000;

    explicit EffectEditCoalescer(int idleMS = kDefaultIdleMS) :
        _idleMS(idleMS) {}

    // Returns true if the effect changed. `nowMS` is any monotonic clock.
    bool Apply(SequenceElements& seq, Effect* effect, std::optional<std::string> effectName, SettingsChange settings,
               SettingsChange palette, int64_t nowMS);
    bool Apply(SequenceElements& seq, Effect* effect, std::optional<std::string> effectName, SettingsChange settings,
               SettingsChange palette);

    void EndBurst() { _open = false; }
    // The next change that would start a new undo step adds to the step
    // recorded last instead, if nothing else has touched the undo history
    // since: one undo step for "embed this image, then use it".
    void JoinNextToCurrentStep(SequenceElements& seq);
    bool InBurst() const { return _open; }

    // The change Effect::SetSettings(settings, keepxsettings = true) makes:
    // the effect's settings become `settings`, except that its own X_ keys
    // (lock, render-disable, description, ...) are kept.
    static SettingsChange ReplaceSettingsKeepingX(const Effect& effect, const std::string& settings);

    static int64_t NowMS();

private:
    int _idleMS;
    bool _open = false;
    const SequenceElements* _seq = nullptr;
    SeqObjectId _effect = kNoSeqObjectId;
    uint64_t _historySerial = 0;
    int64_t _lastMS = 0;
    const SequenceElements* _joinSeq = nullptr;
    uint64_t _joinSerial = 0;
};
