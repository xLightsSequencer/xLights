/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SequenceBulkEdits.h"

#include "CommandExecutor.h"
#include "Effect.h"
#include "EffectLayer.h"
#include "Element.h"
#include "SequenceEditCapture.h"
#include "SequenceElements.h"

namespace {

bool Clashes(EffectLayer* layer, const Effect* eff, int start, int end) {
    for (const Effect* other : layer->GetAllEffectsByTime(start, end)) {
        if (other != eff) {
            return true;
        }
    }
    return false;
}

bool SameEffect(const Effect* e, const Effect* e2, const std::string& pal, const std::string& set) {
    if (e2 == nullptr) {
        return false;
    }
    if (e->GetEffectIndex() != e2->GetEffectIndex() || e->GetStartTimeMS() != e2->GetStartTimeMS() || e->GetEndTimeMS() != e2->GetEndTimeMS()) {
        return false;
    }
    return pal == e2->GetPaletteAsString() && set == e2->GetSettingsAsString();
}

// Where promoted effects go: a given layer, or the model's top layer, with a
// new empty layer inserted above it on first use if it holds effects.
class TargetLayer {
public:
    explicit TargetLayer(EffectLayer* layer) :
        _layer(layer) {}
    TargetLayer(CommandExecutor& exec, ModelElement* model) :
        _exec(&exec), _model(model) {
        if (model->GetEffectLayer(0)->GetEffectCount() == 0) {
            _layer = model->GetEffectLayer(0);
        }
    }
    bool Busy(int start, int end) const { return _layer != nullptr && _layer->HasEffectsInTimeRange(start, end); }
    EffectLayer* Get() {
        if (_layer == nullptr) {
            _layer = _exec->InsertLayer(_model, 0);
        }
        return _layer;
    }

private:
    CommandExecutor* _exec = nullptr;
    ModelElement* _model = nullptr;
    EffectLayer* _layer = nullptr;
};

bool Promotable(const Effect* e) {
    const std::string& name = e->GetEffectName();
    return name == "On" || name == "Color Wash";
}

} // namespace

std::vector<EffectLayer*> ShiftableLayers(const SequenceElements& seq, bool includeTiming) {
    std::vector<EffectLayer*> out;
    for (int i = 0; i < (int)seq.GetElementCount(MASTER_VIEW); ++i) {
        Element* ele = seq.GetElement(i, MASTER_VIEW);
        if (ele == nullptr || (!includeTiming && ele->GetType() == ElementType::ELEMENT_TYPE_TIMING)) {
            continue;
        }
        for (EffectLayer* l : ele->GetEffectLayers()) {
            out.push_back(l);
        }
        if (ele->GetType() != ElementType::ELEMENT_TYPE_MODEL) {
            continue;
        }
        ModelElement* me = dynamic_cast<ModelElement*>(ele);
        for (int s = 0; me != nullptr && s < me->GetStrandCount(); ++s) {
            StrandElement* st = me->GetStrand(s);
            for (int n = 0; st != nullptr && n < st->GetNodeLayerCount(); ++n) {
                if (NodeLayer* nl = st->GetNodeLayer(n)) {
                    out.push_back(nl);
                }
            }
        }
        for (int s = 0; me != nullptr && s < me->GetSubModelAndStrandCount(); ++s) {
            if (SubModelElement* sub = me->GetSubModel(s)) {
                for (EffectLayer* l : sub->GetEffectLayers()) {
                    out.push_back(l);
                }
            }
        }
    }
    return out;
}

namespace {

// Inside a SequenceEditCapture, which records what these change. Locked
// effects are never moved or deleted.
void Move(Effect* eff, int start, int end) {
    if (start < eff->GetStartTimeMS()) {
        eff->SetStartTimeMS(start);
        eff->SetEndTimeMS(end);
    } else {
        eff->SetEndTimeMS(end);
        eff->SetStartTimeMS(start);
    }
}

void ShiftAllOnLayer(EffectLayer* layer, int ms) {
    // backwards so deleting doesn't skip anything
    for (int i = layer->GetEffectCount() - 1; i >= 0; --i) {
        Effect* eff = layer->GetEffect(i);
        if (eff->IsLocked()) {
            continue;
        }
        int start = eff->GetStartTimeMS();
        int end = eff->GetEndTimeMS();
        if (start + ms < 0) {
            if (end + ms < 0) {
                layer->DeleteEffectByUid(eff->GetUid());
                continue;
            }
            Move(eff, 0, end + ms);
        } else {
            Move(eff, start + ms, end + ms);
        }
    }
}

void ShiftSelectedOnLayer(EffectLayer* layer, int ms) {
    if (ms < 0) {
        for (int i = 0; i < layer->GetEffectCount();) {
            Effect* eff = layer->GetEffect(i);
            int start = eff->GetStartTimeMS();
            int end = eff->GetEndTimeMS();
            if (eff->IsLocked()) {
                // stays put
            } else if (eff->GetSelected() && start + ms < 0) {
                if (end + ms < 0) {
                    // Deleted now so it can't clash with the effects after it.
                    if (layer->DeleteEffectByUid(eff->GetUid())) {
                        continue;
                    }
                } else {
                    Move(eff, 0, end + ms);
                }
            } else if (eff->GetSelected() && !Clashes(layer, eff, start + ms, end + ms)) {
                Move(eff, start + ms, end + ms);
            }
            ++i;
        }
    } else {
        for (int i = layer->GetEffectCount() - 1; i >= 0; --i) {
            Effect* eff = layer->GetEffect(i);
            if (!eff->GetSelected() || eff->IsLocked()) {
                continue;
            }
            int start = eff->GetStartTimeMS();
            int end = eff->GetEndTimeMS();
            if (!Clashes(layer, eff, start + ms, end + ms)) {
                Move(eff, start + ms, end + ms);
            }
        }
    }
}

} // namespace

bool ShiftEffects(SequenceElements& seq, const std::vector<EffectLayer*>& layers, int ms, bool selectedOnly, const std::string& label) {
    if (ms == 0 || layers.empty()) {
        return false;
    }
    // A capture records the result in one pass; executing each move as a
    // command costs a lookup per effect.
    SequenceEditCapture capture(seq, label);
    for (EffectLayer* layer : layers) {
        if (selectedOnly) {
            ShiftSelectedOnLayer(layer, ms);
        } else {
            ShiftAllOnLayer(layer, ms);
        }
    }
    capture.Commit();
    return capture.MadeUndoStep();
}

bool ShiftEffects(SequenceElements& seq, int ms, bool includeTiming, bool selectedOnly, const std::string& label) {
    return ShiftEffects(seq, ShiftableLayers(seq, includeTiming), ms, selectedOnly, label);
}

int PromoteNodeEffects(SequenceElements& seq, ModelElement* element, const std::string& label) {
    if (element == nullptr) {
        return 0;
    }
    CommandExecutor& exec = seq.GetCommandExecutor();
    ScopedCommandBatch batch(exec, label);
    int promoted = 0;

    // nodes -> strands (straight to the model for a single strand)
    for (int x = 0; x < element->GetStrandCount(); ++x) {
        StrandElement* se = element->GetStrand(x);
        // A single strand promotes straight to the model, onto a new top
        // layer if the model's top layer is in use.
        TargetLayer target = element->GetStrandCount() <= 1 ? TargetLayer(exec, element) : TargetLayer(se->GetEffectLayer(0));
        if (se->GetNodeLayerCount() == 0) {
            continue;
        }
        NodeLayer* base = se->GetNodeLayer(0);
        for (int e = base->GetEffectCount() - 1; e >= 0; --e) {
            Effect* eff = base->GetEffect(e);
            if (target.Busy(eff->GetStartTimeMS(), eff->GetEndTimeMS()) || !Promotable(eff)) {
                continue;
            }
            const std::string pal = eff->GetPaletteAsString();
            const std::string set = eff->GetSettingsAsString();
            int mp = (eff->GetStartTimeMS() + eff->GetEndTimeMS()) / 2;
            bool collapse = true;
            for (int n = 1; n < se->GetNodeLayerCount() && collapse; ++n) {
                NodeLayer* node = se->GetNodeLayer(n);
                int idx = 0;
                collapse = node->HitTestEffectByTime(mp, idx) && SameEffect(eff, node->GetEffect(idx), pal, set);
            }
            if (!collapse) {
                continue;
            }
            exec.AddEffect(target.Get(), eff->GetEffectName(), set, pal, eff->GetStartTimeMS(), eff->GetEndTimeMS(), false, false);
            ++promoted;
            for (int n = 0; n < se->GetNodeLayerCount(); ++n) {
                NodeLayer* node = se->GetNodeLayer(n);
                int idx = 0;
                if (node->HitTestEffectByTime(mp, idx) && !node->GetEffect(idx)->IsLocked()) {
                    exec.DeleteEffect(node->GetEffect(idx));
                }
            }
        }
    }
    if (element->GetStrandCount() <= 1) {
        return promoted;
    }

    // strands -> model
    EffectLayer* base = element->GetStrand(0)->GetEffectLayer(0);
    TargetLayer target(exec, element);
    for (int e = base->GetEffectCount() - 1; e >= 0; --e) {
        Effect* eff = base->GetEffect(e);
        if (!Promotable(eff)) {
            continue;
        }
        const std::string pal = eff->GetPaletteAsString();
        const std::string set = eff->GetSettingsAsString();
        int mp = (eff->GetStartTimeMS() + eff->GetEndTimeMS()) / 2;
        bool collapse = true;
        for (int n = 0; n < element->GetStrandCount() && collapse; ++n) {
            StrandElement* se = element->GetStrand(n);
            for (int l = 0; l < (int)se->GetEffectLayerCount() && collapse; ++l) {
                EffectLayer* layer = se->GetEffectLayer(l);
                if (layer == base) {
                    continue;
                }
                int idx = 0;
                collapse = layer->HitTestEffectByTime(mp, idx) && SameEffect(eff, layer->GetEffect(idx), pal, set);
            }
        }
        if (!collapse) {
            continue;
        }
        exec.AddEffect(target.Get(), eff->GetEffectName(), set, pal, eff->GetStartTimeMS(), eff->GetEndTimeMS(), false, false);
        ++promoted;
        for (int n = 0; n < element->GetStrandCount(); ++n) {
            StrandElement* se = element->GetStrand(n);
            for (int l = 0; l < (int)se->GetEffectLayerCount(); ++l) {
                EffectLayer* layer = se->GetEffectLayer(l);
                int idx = 0;
                if (layer->HitTestEffectByTime(mp, idx) && !layer->GetEffect(idx)->IsLocked()) {
                    exec.DeleteEffect(layer->GetEffect(idx));
                }
            }
        }
    }
    return promoted;
}
