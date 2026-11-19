/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CollabUidTable.h"

#include <algorithm>
#include <vector>

#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/SequenceCommands.h"
#include "render/SequenceElements.h"
#include "render/SequenceMutationGuard.h"

using nlohmann::json;

namespace Collab {

namespace {

// Effects in a layer in time order (moves don't re-sort a layer).
std::vector<Effect*> SortedEffects(const EffectLayer* layer) {
    std::vector<Effect*> out(layer->GetEffects().begin(), layer->GetEffects().end());
    std::stable_sort(out.begin(), out.end(), [](const Effect* a, const Effect* b) {
        return std::make_pair(a->GetStartTimeMS(), a->GetEndTimeMS()) < std::make_pair(b->GetStartTimeMS(), b->GetEndTimeMS());
    });
    return out;
}

json EffectJson(const Effect* e) {
    return json::array({ e->GetUid(), e->GetEffectName(), e->GetStartTimeMS(), e->GetEndTimeMS(), e->GetProtected(), e->GetSettingsAsString(),
                         e->GetPaletteAsString(), e->GetID() });
}

bool ReadEffect(const json& j, EffectState& s, int& position) {
    if (!j.is_array() || j.size() != 8 || !j[0].is_number_unsigned() || !j[1].is_string() || !j[2].is_number_integer() ||
        !j[3].is_number_integer() || !j[4].is_boolean() || !j[5].is_string() || !j[6].is_string() || !j[7].is_number_integer()) {
        return false;
    }
    position = j[7].get<int>();
    s.uid = j[0].get<uint64_t>();
    s.name = j[1].get<std::string>();
    s.startTimeMS = j[2].get<int>();
    s.endTimeMS = j[3].get<int>();
    s.isProtected = j[4].get<bool>();
    s.settings = j[5].get<std::string>();
    s.palette = j[6].get<std::string>();
    return s.uid != kNoSeqObjectId;
}

// Effects in the layer's own order (not document state, but the .xsq is
// written in it), each with its position id (also written).
json LayerTable(const EffectLayer* layer) {
    json effects = json::array();
    for (const Effect* e : layer->GetEffects()) {
        effects.push_back(EffectJson(e));
    }
    return json{ { "u", layer->GetUid() }, { "n", layer->GetLayerName() }, { "e", std::move(effects) } };
}

json RowTable(const Element* el) {
    json layers = json::array();
    for (const EffectLayer* l : el->GetEffectLayers()) {
        layers.push_back(LayerTable(l));
    }
    json j{ { "u", el->GetUid() }, { "n", el->GetName() }, { "r", el->IsRenderDisabledFlag() }, { "L", std::move(layers) } };
    if (auto* te = dynamic_cast<const TimingElement*>(el)) {
        j["f"] = te->GetFixedTiming();
        j["st"] = te->GetSubType();
    }
    if (auto* st = dynamic_cast<const StrandElement*>(el)) {
        json nodes = json::array();
        for (int n = 0; n < st->GetNodeLayerCount(); ++n) {
            nodes.push_back(LayerTable(st->GetNodeLayer(n)));
        }
        j["s"] = st->GetStrand();
        j["N"] = std::move(nodes);
    }
    return j;
}

bool Fail(std::string* problem, const std::string& what) {
    if (problem != nullptr) {
        *problem = what;
    }
    return false;
}

bool U64(const json& j, const char* key, uint64_t& out) {
    auto it = j.find(key);
    if (it == j.end() || !it->is_number_unsigned()) {
        return false;
    }
    out = it->get<uint64_t>();
    return true;
}

// Gives the layer its id and exactly the host's effects. Loading can change
// what the host had (load-time settings migration; the .xsq doesn't keep
// everything about a timing mark), so effects that differ are re-created
// from the host's state.
bool ApplyLayer(SequenceElements& seq, Element* el, EffectLayer* layer, const json& t, std::string* problem) {
    uint64_t uid = 0;
    auto e = t.find("e");
    if (!t.is_object() || !U64(t, "u", uid) || uid == kNoSeqObjectId || e == t.end() || !e->is_array()) {
        return Fail(problem, "malformed layer entry");
    }
    layer->SetUid(uid);
    // A timing layer's name isn't saved.
    auto name = t.find("n");
    if (name != t.end() && name->is_string() && layer->GetLayerName() != name->get_ref<const std::string&>()) {
        layer->SetLayerName(name->get<std::string>());
    }
    std::vector<EffectState> order; // the host's order
    std::vector<int> positions;
    for (const json& x : *e) {
        EffectState s;
        int pos = 0;
        if (!ReadEffect(x, s, pos)) {
            return Fail(problem, "malformed effect entry");
        }
        order.push_back(std::move(s));
        positions.push_back(pos);
    }
    std::vector<EffectState> want = order;
    std::stable_sort(want.begin(), want.end(), [](const EffectState& a, const EffectState& b) {
        return std::make_pair(a.startTimeMS, a.endTimeMS) < std::make_pair(b.startTimeMS, b.endTimeMS);
    });
    auto mine = SortedEffects(layer);
    bool same = mine.size() == want.size();
    for (size_t i = 0; same && i < mine.size(); ++i) {
        EffectState have = EffectState::Of(*mine[i]);
        have.uid = want[i].uid;
        same = have == want[i];
    }
    if (same) {
        for (size_t i = 0; i < mine.size(); ++i) {
            mine[i]->SetUid(want[i].uid);
        }
    } else {
        ReplaceLayerContentsCommand replace({ el->GetUid(), uid }, want);
        CommandResult r;
        if (replace.Apply(seq, ApplyMode::Lenient, &r) == nullptr) {
            return Fail(problem, "layer " + std::to_string(uid) + " of " + el->GetFullName() + " could not be restored: " + r.message);
        }
    }
    std::vector<Effect*> ordered;
    for (size_t i = 0; i < order.size(); ++i) {
        Effect* fx = layer->GetEffectFromUid(order[i].uid);
        if (fx == nullptr) {
            return Fail(problem, "effect " + std::to_string(order[i].uid) + " missing after restore");
        }
        fx->SetID(positions[i]);
        ordered.push_back(fx);
    }
    if (!layer->ReorderEffects(ordered)) {
        return Fail(problem, "layer " + std::to_string(uid) + " could not be put in the host's order");
    }
    return true;
}

bool ApplyRow(SequenceElements& seq, Element* el, const json& t, std::string* problem) {
    uint64_t uid = 0;
    auto layers = t.find("L");
    if (!t.is_object() || !U64(t, "u", uid) || uid == kNoSeqObjectId || layers == t.end() || !layers->is_array()) {
        return Fail(problem, "malformed row entry");
    }
    el->SetUid(uid);
    auto rd = t.find("r");
    if (rd != t.end() && rd->is_boolean() && el->IsRenderDisabledFlag() != rd->get<bool>()) {
        el->SetRenderDisabled(rd->get<bool>());
    }
    if (auto* te = dynamic_cast<TimingElement*>(el)) {
        auto f = t.find("f");
        auto st = t.find("st");
        if (f != t.end() && f->is_number_integer() && te->GetFixedTiming() != f->get<int>()) {
            te->SetFixedTiming(f->get<int>());
        }
        if (st != t.end() && st->is_string() && te->GetSubType() != st->get_ref<const std::string&>()) {
            te->SetSubType(st->get<std::string>());
        }
    }
    // The .xsq leaves out a submodel or strand row's trailing empty, unnamed
    // layers, and keeps only the interval of a fixed timing track; match the
    // host's layer count (contents follow below).
    if (dynamic_cast<SubModelElement*>(el) != nullptr || el->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
        while (el->GetEffectLayerCount() < layers->size()) {
            el->AddEffectLayer();
        }
        while (el->GetEffectLayerCount() > std::max<size_t>(1, layers->size())) {
            el->RemoveEffectLayer((int)el->GetEffectLayerCount() - 1);
        }
    }
    if (el->GetEffectLayerCount() != layers->size()) {
        return Fail(problem, "row " + el->GetFullName() + " has a different number of layers");
    }
    for (size_t i = 0; i < layers->size(); ++i) {
        if (!ApplyLayer(seq, el, el->GetEffectLayer((int)i), (*layers)[i], problem)) {
            return false;
        }
    }
    if (auto* st = dynamic_cast<StrandElement*>(el)) {
        auto nodes = t.find("N");
        if (nodes == t.end() || !nodes->is_array()) {
            return Fail(problem, "malformed strand entry");
        }
        // Node layers are created on demand; make the ones the host has.
        for (size_t n = 0; n < nodes->size(); ++n) {
            NodeLayer* nl = st->GetNodeLayer((int)n, true);
            if (nl == nullptr || !ApplyLayer(seq, el, nl, (*nodes)[n], problem)) {
                return false;
            }
        }
        if ((size_t)st->GetNodeLayerCount() != nodes->size()) {
            return Fail(problem, "strand " + el->GetFullName() + " has extra node layers");
        }
    }
    return true;
}

} // namespace

json CaptureUids(const SequenceElements& seq) {
    json out = json::array();
    for (size_t i = 0; i < seq.GetElementCount(MASTER_VIEW); ++i) {
        const Element* el = seq.GetElement(i, MASTER_VIEW);
        if (el == nullptr) {
            continue;
        }
        json row = RowTable(el);
        if (auto* me = dynamic_cast<const ModelElement*>(el)) {
            json subs = json::array();
            for (int x = 0; x < me->GetSubModelAndStrandCount(); ++x) {
                if (const SubModelElement* sme = me->GetSubModel(x)) {
                    subs.push_back(RowTable(sme));
                }
            }
            row["S"] = std::move(subs);
        }
        out.push_back(std::move(row));
    }
    return out;
}

bool ApplyUids(SequenceElements& seq, const json& table, std::string* problem) {
    SeqMutationGuard::Scope applying;
    if (!table.is_array()) {
        return Fail(problem, "uid table is not an array");
    }
    if (table.size() != seq.GetElementCount(MASTER_VIEW)) {
        return Fail(problem, "different number of rows");
    }
    for (size_t i = 0; i < table.size(); ++i) {
        const json& t = table[i];
        Element* el = seq.GetElement(i, MASTER_VIEW);
        auto name = t.is_object() ? t.find("n") : t.end();
        if (el == nullptr || name == t.end() || !name->is_string() || el->GetName() != name->get<std::string>()) {
            return Fail(problem, "row " + std::to_string(i) + " differs");
        }
        if (!ApplyRow(seq, el, t, problem)) {
            return false;
        }
        auto* me = dynamic_cast<ModelElement*>(el);
        auto subs = t.find("S");
        if (me == nullptr) {
            continue;
        }
        if (subs == t.end() || !subs->is_array()) {
            return Fail(problem, "malformed model entry");
        }
        for (const json& st : *subs) {
            auto sname = st.is_object() ? st.find("n") : st.end();
            auto strand = st.is_object() ? st.find("s") : st.end();
            SubModelElement* sme = nullptr;
            if (strand != st.end()) {
                if (!strand->is_number_integer()) {
                    return Fail(problem, "malformed strand index");
                }
                sme = me->GetStrand(strand->get<int>(), true);
            } else if (sname != st.end() && sname->is_string()) {
                for (int x = 0; x < me->GetSubModelCount(); ++x) {
                    if (me->GetSubModel(x)->GetName() == sname->get<std::string>()) {
                        sme = me->GetSubModel(x);
                        break;
                    }
                }
            }
            if (sme == nullptr) {
                return Fail(problem, "submodel row of " + el->GetName() + " not found");
            }
            if (!ApplyRow(seq, sme, st, problem)) {
                return false;
            }
        }
        if ((size_t)me->GetSubModelAndStrandCount() != subs->size()) {
            return Fail(problem, "model " + el->GetName() + " has different submodel rows");
        }
    }
    return true;
}

} // namespace Collab
