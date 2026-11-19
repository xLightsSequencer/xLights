/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CollabDocHash.h"

#include <algorithm>
#include <sstream>
#include <vector>

#include <pugixml.hpp>

#include "CollabCommandInfo.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/SequenceCommands.h"
#include "render/SequenceElementCommands.h"
#include "render/SequenceElements.h"
#include "render/SequenceMedia.h"
#include "render/SequenceMediaCommands.h"

namespace Collab {

namespace {

// FNV-1a 64 with explicit field separators.
struct Fnv {
    uint64_t h = 1469598103934665603ull;
    void Bytes(const void* p, size_t n) {
        const unsigned char* b = static_cast<const unsigned char*>(p);
        for (size_t i = 0; i < n; ++i) {
            h ^= b[i];
            h *= 1099511628211ull;
        }
    }
    void U64(uint64_t v) { Bytes(&v, sizeof(v)); }
    void Str(const std::string& s) {
        U64(s.size());
        Bytes(s.data(), s.size());
    }
};

void HashLayer(Fnv& f, const EffectLayer* layer) {
    f.U64(layer->GetUid());
    f.Str(layer->GetLayerName());
    std::vector<const Effect*> effects(layer->GetEffects().begin(), layer->GetEffects().end());
    std::stable_sort(effects.begin(), effects.end(), [](const Effect* a, const Effect* b) {
        if (a->GetStartTimeMS() != b->GetStartTimeMS()) {
            return a->GetStartTimeMS() < b->GetStartTimeMS();
        }
        if (a->GetEndTimeMS() != b->GetEndTimeMS()) {
            return a->GetEndTimeMS() < b->GetEndTimeMS();
        }
        return a->GetUid() < b->GetUid();
    });
    f.U64(effects.size());
    for (const Effect* e : effects) {
        f.U64(e->GetUid());
        f.Str(e->GetEffectName());
        f.U64((uint64_t)(int64_t)e->GetStartTimeMS());
        f.U64((uint64_t)(int64_t)e->GetEndTimeMS());
        f.U64(e->GetProtected() ? 1 : 0);
        f.Str(e->GetSettingsAsString());
        f.Str(e->GetPaletteAsString());
    }
}

void HashRow(Fnv& f, const Element* el) {
    f.U64(el->GetUid());
    f.U64((uint64_t)el->GetType());
    f.Str(el->GetFullName());
    f.U64(el->IsRenderDisabledFlag() ? 1 : 0);
    if (auto* te = dynamic_cast<const TimingElement*>(el)) {
        f.Str(te->GetSubType());
        f.U64((uint64_t)(int64_t)te->GetFixedTiming());
    }
    const auto& layers = el->GetEffectLayers();
    f.U64(layers.size());
    for (const EffectLayer* l : layers) {
        HashLayer(f, l);
    }
    if (auto* st = dynamic_cast<const StrandElement*>(el)) {
        f.U64((uint64_t)st->GetNodeLayerCount());
        for (int n = 0; n < st->GetNodeLayerCount(); ++n) {
            HashLayer(f, st->GetNodeLayer(n));
        }
    }
}

uint64_t ElementHash(const Element* el) {
    Fnv f;
    HashRow(f, el);
    if (auto* me = dynamic_cast<const ModelElement*>(el)) {
        f.U64((uint64_t)me->GetSubModelAndStrandCount());
        for (int x = 0; x < me->GetSubModelAndStrandCount(); ++x) {
            const SubModelElement* sme = me->GetSubModel(x);
            if (sme != nullptr) {
                HashRow(f, sme);
            }
        }
    }
    return f.h;
}

template<class T>
std::string XmlOf(const T& obj) {
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("r");
    obj.SaveToXml(root);
    std::ostringstream s;
    doc.save(s, "", pugi::format_raw);
    return s.str();
}

bool TouchesEverything(const SequenceCommand& cmd) {
    switch (cmd.Type()) {
    // These can rewrite effect settings anywhere in the sequence.
    case CommandType::RenameTimingTrack:
    case CommandType::RenameModelElement:
    case CommandType::SetEffectSymbol:
    case CommandType::SetEffectSymbolLink:
    case CommandType::DeleteElement:
        return true;
    case CommandType::Batch:
        for (const auto& m : static_cast<const BatchCommand&>(cmd).GetCommands()) {
            if (TouchesEverything(*m)) {
                return true;
            }
        }
        return false;
    default:
        return false;
    }
}

} // namespace

uint64_t DocHasher::GlobalHash(const SequenceElements& seq) {
    Fnv f;
    size_t n = seq.GetElementCount(MASTER_VIEW);
    f.U64(n);
    for (size_t i = 0; i < n; ++i) {
        const Element* el = seq.GetElement(i, MASTER_VIEW);
        f.U64(el != nullptr ? el->GetUid() : 0);
    }
    for (int t = 0; t < SequenceElements::TagCount; ++t) {
        f.U64((uint64_t)(int64_t)seq.GetTagPosition(t));
    }
    f.Str(XmlOf(seq.GetSequenceFaces()));
    f.Str(XmlOf(seq.GetSongStructureManager()));
    f.Str(XmlOf(seq.GetEffectSymbolManager()));

    std::unordered_map<const void*, std::pair<std::shared_ptr<const std::string>, uint64_t>> payloads;
    auto payloadHash = [&](const std::shared_ptr<const std::string>& p) -> uint64_t {
        if (!p) {
            return 0;
        }
        auto it = _payloads.find(p.get());
        uint64_t h;
        if (it != _payloads.end()) {
            h = it->second.second;
        } else {
            Fnv pf;
            pf.Str(*p);
            h = pf.h;
        }
        payloads[p.get()] = { p, h };
        return h;
    };
    const SequenceMedia& media = seq.GetSequenceMedia();
    auto entries = media.GetEmbeddedEntries();
    f.U64(entries.size());
    for (const auto& [key, entry] : entries) {
        if (!entry) {
            continue;
        }
        MediaEntryState s = SequenceMedia::StateOf(key, *entry);
        f.Str(s.key);
        f.Str(MediaEntryState::TypeName(s.type));
        f.Str(s.subtype);
        f.U64(s.embedded ? 1 : 0);
        f.U64(payloadHash(s.data));
        f.U64(s.frames.size());
        for (const auto& fr : s.frames) {
            f.U64((uint64_t)(int64_t)fr.timeMS);
            f.U64(payloadHash(fr.data));
        }
    }
    // Keep only the payloads still in use.
    _payloads = std::move(payloads);
    return f.h;
}

uint64_t DocHasher::Hash(const SequenceElements& seq) {
    if (!_globalValid) {
        _global = GlobalHash(seq);
        _globalValid = true;
    }
    Fnv f;
    f.U64(_global);
    size_t n = seq.GetElementCount(MASTER_VIEW);
    for (size_t i = 0; i < n; ++i) {
        const Element* el = seq.GetElement(i, MASTER_VIEW);
        if (el == nullptr) {
            continue;
        }
        Cached& c = _elements[el->GetUid()];
        if (c.element != el || c.changeCount != el->getChangeCount() || c.hash == 0) {
            c.element = el;
            c.changeCount = el->getChangeCount();
            c.hash = ElementHash(el);
        }
        f.U64(c.hash);
    }
    return f.h;
}

void DocHasher::InvalidateAll() {
    _elements.clear();
    _globalValid = false;
}

void DocHasher::Invalidate(const SequenceElements& seq, const SequenceCommand& cmd) {
    if (!invalidateEnabled) {
        return;
    }
    if (TouchesEverything(cmd)) {
        InvalidateAll();
        return;
    }
    switch (cmd.Type()) {
    case CommandType::AddEffect: {
        const auto& c = static_cast<const AddEffectCommand&>(cmd);
        InvalidateElement(TopLevelOf(seq, c.GetLayer().element));
        break;
    }
    case CommandType::DeleteEffect:
        InvalidateElement(TopLevelOf(seq, static_cast<const DeleteEffectCommand&>(cmd).Target().element));
        break;
    case CommandType::MoveEffect:
        InvalidateElement(TopLevelOf(seq, static_cast<const MoveEffectCommand&>(cmd).Target().element));
        break;
    case CommandType::ModifyEffect:
        InvalidateElement(TopLevelOf(seq, static_cast<const ModifyEffectCommand&>(cmd).Target().element));
        break;
    case CommandType::AddLayer:
        InvalidateElement(TopLevelOf(seq, static_cast<const AddLayerCommand&>(cmd).GetElement()));
        break;
    case CommandType::RemoveLayer:
        InvalidateElement(TopLevelOf(seq, static_cast<const RemoveLayerCommand&>(cmd).GetElement()));
        break;
    case CommandType::SetLayerName:
        InvalidateElement(TopLevelOf(seq, static_cast<const SetLayerNameCommand&>(cmd).GetLayer().element));
        break;
    case CommandType::ReplaceLayerContents:
        InvalidateElement(TopLevelOf(seq, static_cast<const ReplaceLayerContentsCommand&>(cmd).GetLayer().element));
        break;
    case CommandType::Batch:
        for (const auto& m : static_cast<const BatchCommand&>(cmd).GetCommands()) {
            Invalidate(seq, *m);
        }
        break;
    case CommandType::AddElement:
        InvalidateElement(static_cast<const AddElementCommand&>(cmd).GetState().uid);
        _globalValid = false;
        break;
    case CommandType::MoveElement:
        _globalValid = false;
        break;
    case CommandType::SetTimingFixed:
        InvalidateElement(TopLevelOf(seq, static_cast<const SetTimingFixedCommand&>(cmd).GetElement()));
        break;
    case CommandType::SetElementRenderDisabled:
        InvalidateElement(TopLevelOf(seq, static_cast<const SetElementRenderDisabledCommand&>(cmd).GetElement()));
        break;
    case CommandType::SetTag:
    case CommandType::SetSequenceFace:
    case CommandType::SetSongStructure:
    case CommandType::PutMedia:
    case CommandType::RemoveMedia:
    case CommandType::RenameMedia:
        _globalValid = false;
        break;
    default:
        InvalidateAll();
        break;
    }
}

std::string DocHasher::Components(const SequenceElements& seq) {
    std::ostringstream out;
    auto h = [](const std::string& s) {
        Fnv f;
        f.Str(s);
        return f.h;
    };
    std::string order;
    for (size_t i = 0; i < seq.GetElementCount(MASTER_VIEW); ++i) {
        const Element* el = seq.GetElement(i, MASTER_VIEW);
        order += std::to_string(el != nullptr ? el->GetUid() : 0) + ",";
    }
    out << "order " << order << "\n";
    out << "tags";
    for (int t = 0; t < SequenceElements::TagCount; ++t) {
        out << " " << seq.GetTagPosition(t);
    }
    out << "\n";
    out << "faces " << h(XmlOf(seq.GetSequenceFaces())) << "\n";
    out << "song " << h(XmlOf(seq.GetSongStructureManager())) << "\n";
    out << "symbols " << h(XmlOf(seq.GetEffectSymbolManager())) << "\n";
    for (const auto& [key, entry] : seq.GetSequenceMedia().GetEmbeddedEntries()) {
        if (!entry) {
            continue;
        }
        MediaEntryState s = SequenceMedia::StateOf(key, *entry);
        out << "media " << key << " " << MediaEntryState::TypeName(s.type) << " " << s.embedded << " " << (s.data ? h(*s.data) : 0) << " "
            << s.frames.size() << "\n";
    }
    for (size_t i = 0; i < seq.GetElementCount(MASTER_VIEW); ++i) {
        const Element* el = seq.GetElement(i, MASTER_VIEW);
        if (el != nullptr) {
            out << "row " << el->GetFullName() << " #" << el->GetUid() << " " << ElementHash(el) << "\n";
        }
    }
    return out.str();
}

uint64_t DocHasher::FullHash(const SequenceElements& seq) {
    DocHasher h;
    return h.Hash(seq);
}

} // namespace Collab
