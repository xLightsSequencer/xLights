/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ControllerConnectionHistory.h"

#include <algorithm>
#include <cmath>

#include "Model.h"
#include "ModelManager.h"

static constexpr size_t MAX_UNDO_STEPS = 100;

std::string ControllerConnectionHistory::Key(const Model* m)
{
    const ControllerConnection& c = m->GetConstCtrlConn();
    std::string key = c.GetName() + "|" + c.GetProtocol() + "|" + c.GetColorOrder() + "|" + c.GetSmartRemoteType() + "|" + m->GetModelChain() + "|";
    for (int v : { c.GetProtocolSpeed(), c.GetCtrlPort(), c.GetBrightness(), c.GetStartNulls(), c.GetEndNulls(), c.GetGroupCount(),
                   c.GetReverse(), c.GetZigZag(), c.GetDMXChannel(), c.GetLORPortPixels(), c.GetSmartRemote(), c.GetSmartTs(),
                   (int)c.GetSRCascadeOnPort(), c.GetSRMaxCascade(), (int)std::lround(c.GetGamma() * 1000.0f) }) {
        key += std::to_string(v) + ",";
    }
    for (int p = ControllerConnection::USE_SMART_REMOTE; p <= ControllerConnection::TS_ACTIVE; ++p) {
        key += c.IsPropertySet((ControllerConnection::CTRL_PROPS)p) ? '1' : '0';
    }
    return key;
}

ControllerConnectionHistory::Snapshot ControllerConnectionHistory::Capture() const
{
    Snapshot snap;
    for (const auto& it : _mm) {
        const Model* m = it.second;
        if (m == nullptr || m->GetDisplayAs() == DisplayAsType::ModelGroup) {
            continue;
        }
        snap.emplace(it.first, State{ m, m->GetConstCtrlConn(), m->GetModelChain(), Key(m) });
    }
    return snap;
}

bool ControllerConnectionHistory::Same(const Snapshot& a, const Snapshot& b)
{
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), [](const auto& x, const auto& y) {
               return x.first == y.first && x.second.model == y.second.model && x.second.key == y.second.key;
           });
}

void ControllerConnectionHistory::Begin()
{
    _original = Capture();
    _last = _original;
    _baseline = _original;
    _undo.clear();
}

void ControllerConnectionHistory::SkipStep()
{
    _last = Capture();
    if (_undo.empty()) {
        _baseline = _last;
    }
}

bool ControllerConnectionHistory::HasChanges() const
{
    return !Same(Capture(), _baseline);
}

bool ControllerConnectionHistory::Record()
{
    Snapshot now = Capture();
    if (Same(now, _last)) {
        return false;
    }
    _undo.push_back(std::move(_last));
    if (_undo.size() > MAX_UNDO_STEPS) {
        _undo.erase(_undo.begin());
    }
    _last = std::move(now);
    return true;
}

std::vector<std::string> ControllerConnectionHistory::Restore(const Snapshot& snap)
{
    std::vector<std::string> changed;
    for (const auto& [name, state] : snap) {
        Model* m = _mm[name];
        // Skip models deleted or replaced since the snapshot.
        if (m == nullptr || m != state.model || Key(m) == state.key) {
            continue;
        }
        m->GetCtrlConn() = state.conn;
        m->SetModelChain(state.chain);
        m->IncrementChangeCount();
        changed.push_back(name);
    }
    return changed;
}

std::vector<std::string> ControllerConnectionHistory::Undo()
{
    if (_undo.empty()) {
        return {};
    }
    Snapshot target = std::move(_undo.back());
    _undo.pop_back();
    auto changed = Restore(target);
    _last = Capture();
    return changed;
}

std::vector<std::string> ControllerConnectionHistory::Revert()
{
    auto changed = Restore(_original);
    _undo.clear();
    _last = Capture();
    _baseline = _last;
    return changed;
}
