/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "EffectEditCoalescer.h"

#include <chrono>
#include <memory>

#include "CommandExecutor.h"
#include "Effect.h"
#include "SequenceElements.h"
#include "UndoManager.h"
#include "../utils/UtilClasses.h"

namespace {
SettingsChange WholeMap(const SettingsMap& map) {
    SettingsChange c;
    c.replaceAll = true;
    for (const auto& [k, v] : map) {
        c.keys[k] = std::string(v);
    }
    return c;
}

bool Changes(const SettingsMap& current, const SettingsChange& change) {
    for (const auto& [k, v] : change.keys) {
        const SettingValue* cur = current.FindValue(k);
        if (v.has_value() ? (cur == nullptr || *cur != *v) : cur != nullptr) {
            return true;
        }
    }
    if (change.replaceAll) {
        for (const auto& [k, v] : current) {
            if (!change.keys.count(k)) {
                return true;
            }
        }
    }
    return false;
}
} // namespace

int64_t EffectEditCoalescer::NowMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

SettingsChange EffectEditCoalescer::ReplaceSettingsKeepingX(const Effect& effect, const std::string& settings) {
    SettingsChange change = SettingsChange::ReplaceWith(settings);
    for (const auto& [k, v] : effect.GetSettings()) {
        if (k.size() > 2 && k[0] == 'X' && k[1] == '_') {
            change.keys[k] = std::string(v);
        }
    }
    return change;
}

bool EffectEditCoalescer::Apply(SequenceElements& seq, Effect* effect, std::optional<std::string> effectName, SettingsChange settings,
                                SettingsChange palette) {
    return Apply(seq, effect, std::move(effectName), std::move(settings), std::move(palette), NowMS());
}

bool EffectEditCoalescer::Apply(SequenceElements& seq, Effect* effect, std::optional<std::string> effectName, SettingsChange settings,
                                SettingsChange palette, int64_t nowMS) {
    if (effect == nullptr) {
        return false;
    }
    if ((!effectName.has_value() || *effectName == effect->GetEffectName()) && !Changes(effect->GetSettings(), settings) &&
        !Changes(effect->GetPaletteMap(), palette)) {
        return false;
    }
    UndoManager& undo = seq.get_undo_mgr();
    const bool continues = _open && _seq == &seq && _effect == effect->GetUid() && undo.GetHistorySerial() == _historySerial &&
                           nowMS >= _lastMS && nowMS - _lastMS <= _idleMS;

    const std::string beforeName = effect->GetEffectName();
    const std::string beforeSettings = effect->GetSettingsAsString();
    const std::string beforePalette = effect->GetPaletteAsString();

    std::unique_ptr<SequenceCommand> restore;
    if (!continues) {
        std::optional<std::string> name;
        if (effect->GetEffectIndex() >= 0) {
            name = beforeName;
        }
        restore = std::make_unique<ModifyEffectCommand>(LocatorOf(effect), std::move(name), WholeMap(effect->GetSettings()),
                                                        WholeMap(effect->GetPaletteMap()));
    }

    seq.GetCommandExecutor().ModifyEffect(effect, std::move(effectName), std::move(settings), std::move(palette), false);

    if (effect->GetEffectName() == beforeName && effect->GetSettingsAsString() == beforeSettings &&
        effect->GetPaletteAsString() == beforePalette) {
        return false;
    }
    if (!continues) {
        undo.CreateUndoStep();
        undo.RecordInverse(CommandType::ModifyEffect, "", std::move(restore));
        _open = true;
        _seq = &seq;
        _effect = effect->GetUid();
    }
    _historySerial = undo.GetHistorySerial();
    _lastMS = nowMS;
    return true;
}
