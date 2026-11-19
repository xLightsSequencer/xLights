/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CollabRoles.h"

#include <algorithm>

#include <pugixml.hpp>

#include "CollabCommandInfo.h"
#include "models/ModelGroup.h"
#include "render/Element.h"
#include "render/RenderContext.h"
#include "render/SequenceElementCommands.h"
#include "render/SequenceElements.h"

namespace Collab {

namespace {

void Rows(const SequenceCommand& cmd, std::vector<SeqObjectId>& out) {
    switch (cmd.Type()) {
    case CommandType::Batch:
        for (const auto& m : static_cast<const BatchCommand&>(cmd).GetCommands()) {
            Rows(*m, out);
        }
        break;
    case CommandType::AddEffect:
        out.push_back(static_cast<const AddEffectCommand&>(cmd).GetLayer().element);
        break;
    case CommandType::DeleteEffect:
        out.push_back(static_cast<const DeleteEffectCommand&>(cmd).Target().element);
        break;
    case CommandType::MoveEffect:
        out.push_back(static_cast<const MoveEffectCommand&>(cmd).Target().element);
        break;
    case CommandType::ModifyEffect:
        out.push_back(static_cast<const ModifyEffectCommand&>(cmd).Target().element);
        break;
    case CommandType::SetEffectSymbolLink:
        out.push_back(static_cast<const SetEffectSymbolLinkCommand&>(cmd).GetTarget().element);
        break;
    case CommandType::AddLayer:
        out.push_back(static_cast<const AddLayerCommand&>(cmd).GetElement());
        break;
    case CommandType::RemoveLayer:
        out.push_back(static_cast<const RemoveLayerCommand&>(cmd).GetElement());
        break;
    case CommandType::SetLayerName:
        out.push_back(static_cast<const SetLayerNameCommand&>(cmd).GetLayer().element);
        break;
    case CommandType::ReplaceLayerContents:
        out.push_back(static_cast<const ReplaceLayerContentsCommand&>(cmd).GetLayer().element);
        break;
    case CommandType::AddElement:
        out.push_back(static_cast<const AddElementCommand&>(cmd).GetUid());
        break;
    case CommandType::DeleteElement:
        out.push_back(static_cast<const DeleteElementCommand&>(cmd).GetElement());
        break;
    case CommandType::MoveElement:
        out.push_back(static_cast<const MoveElementCommand&>(cmd).GetElement());
        break;
    case CommandType::RenameTimingTrack:
        out.push_back(static_cast<const RenameTimingTrackCommand&>(cmd).GetElement());
        break;
    case CommandType::RenameModelElement:
        out.push_back(static_cast<const RenameModelElementCommand&>(cmd).GetElement());
        break;
    case CommandType::SetTimingFixed:
        out.push_back(static_cast<const SetTimingFixedCommand&>(cmd).GetElement());
        break;
    case CommandType::SetElementRenderDisabled:
        out.push_back(static_cast<const SetElementRenderDisabledCommand&>(cmd).GetElement());
        break;
    case CommandType::PutMedia:
    case CommandType::RemoveMedia:
    case CommandType::RenameMedia:
    case CommandType::SetSequenceFace:
    case CommandType::SetEffectSymbol:
    case CommandType::SetTag:
    case CommandType::SetSongStructure:
        break;
    }
}

bool IsModelRow(const Element* el) {
    return el != nullptr && el->GetType() != ElementType::ELEMENT_TYPE_TIMING;
}

void AddGroupMembers(const RenderContext* rc, const std::string& name, std::set<std::string>& out, std::set<std::string>& visited) {
    if (rc == nullptr || !visited.insert(name).second) {
        return;
    }
    const auto* group = dynamic_cast<const ModelGroup*>(rc->GetModel(name));
    if (group == nullptr) {
        return;
    }
    for (const auto& m : group->ModelNames()) {
        out.insert(m);
        AddGroupMembers(rc, m, out, visited);
    }
}

std::string RowName(const SequenceElements& seq, SeqObjectId row) {
    const Element* el = seq.GetElementFromUid(row);
    return el == nullptr ? std::string() : el->GetFullName();
}

std::string NameIn(const std::vector<RosterEntry>& roster, ParticipantId id) {
    for (const auto& r : roster) {
        if (r.participant == id) {
            return r.name;
        }
    }
    return "someone else";
}

bool InRoster(const std::vector<RosterEntry>& roster, ParticipantId id) {
    return std::any_of(roster.begin(), roster.end(), [id](const RosterEntry& r) { return r.participant == id; });
}

} // namespace

std::vector<SeqObjectId> CommandRows(const SequenceCommand& cmd) {
    std::vector<SeqObjectId> out;
    Rows(cmd, out);
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    out.erase(std::remove(out.begin(), out.end(), kNoSeqObjectId), out.end());
    return out;
}

std::set<std::string> CoveredNames(const SequenceElements& seq, SeqObjectId element) {
    std::set<std::string> out;
    const Element* el = seq.GetElementFromUid(element);
    if (!IsModelRow(el)) {
        return out;
    }
    out.insert(el->GetFullName());
    std::set<std::string> visited;
    if (el->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
        AddGroupMembers(seq.GetRenderContext(), el->GetName(), out, visited);
    }
    return out;
}

void AssignmentIndex::Build(const SequenceElements& seq, const std::vector<Assignment>& assignments) {
    _assignments = assignments;
    _byUid.clear();
    _byName.clear();
    for (size_t i = 0; i < _assignments.size(); ++i) {
        _byUid.emplace(_assignments[i].element, i);
        for (const auto& n : CoveredNames(seq, _assignments[i].element)) {
            _byName.emplace(n, i); // the first claim wins; the host keeps them apart
        }
    }
}

const Assignment* AssignmentIndex::FindName(const std::string& fullName) const {
    auto it = _byName.find(fullName);
    return it == _byName.end() ? nullptr : &_assignments[it->second];
}

const Assignment* AssignmentIndex::Find(const SequenceElements& seq, SeqObjectId row) const {
    if (_assignments.empty() || row == kNoSeqObjectId) {
        return nullptr;
    }
    if (auto it = _byUid.find(row); it != _byUid.end()) {
        return &_assignments[it->second];
    }
    const Element* el = seq.GetElementFromUid(row);
    if (!IsModelRow(el)) {
        return nullptr;
    }
    SeqObjectId top = TopLevelOf(seq, row);
    if (top != row) {
        if (auto it = _byUid.find(top); it != _byUid.end()) {
            return &_assignments[it->second];
        }
    }
    if (const Assignment* a = FindName(el->GetFullName())) {
        return a;
    }
    if (const auto* sub = dynamic_cast<const SubModelElement*>(el); sub != nullptr && sub->GetModelElement() != nullptr) {
        return FindName(sub->GetModelElement()->GetName());
    }
    return nullptr;
}

ParticipantRole RoleIn(const std::vector<RosterEntry>& roster, ParticipantId who) {
    for (const auto& r : roster) {
        if (r.participant == who) {
            // The host (listed first) is always an editor.
            return who == roster.front().participant ? ParticipantRole::Editor : r.role;
        }
    }
    return ParticipantRole::Editor;
}

EditRefusal CheckEdit(const SequenceElements& seq, const std::vector<RosterEntry>& roster, const AssignmentIndex& index, ParticipantId author,
                      const SequenceCommand& cmd) {
    EditRefusal r;
    if (RoleIn(roster, author) == ParticipantRole::Viewer) {
        r.reason = Reason::Viewer;
        return r;
    }
    if (index.Empty()) {
        return r;
    }
    for (SeqObjectId row : CommandRows(cmd)) {
        const Assignment* a = index.Find(seq, row);
        if (a != nullptr && a->owner != author) {
            r.reason = Reason::Assigned;
            r.owner = a->owner;
            r.row = RowName(seq, row);
            if (r.row.empty()) {
                r.row = a->name;
            }
            return r;
        }
    }
    return r;
}

std::vector<Assignment> ApplyClaim(const SequenceElements& seq, const std::vector<RosterEntry>& roster, const std::vector<Assignment>& assignments,
                                   ParticipantId requester, SeqObjectId element, ParticipantId owner, std::string& reason, std::string& text) {
    auto refuse = [&](const char* why, std::string t) {
        reason = why;
        text = std::move(t);
        return assignments;
    };
    const bool isHost = !roster.empty() && roster.front().participant == requester;
    if (!InRoster(roster, requester)) {
        return refuse(Reason::Invalid, "Only people in the session can assign rows");
    }
    if (RoleIn(roster, requester) == ParticipantRole::Viewer) {
        return refuse(Reason::Viewer, "Viewers can't assign rows");
    }
    AssignmentIndex index;
    index.Build(seq, assignments);
    if (owner == kNoParticipant) {
        auto it = std::find_if(assignments.begin(), assignments.end(), [element](const Assignment& a) { return a.element == element; });
        const Assignment* held = it != assignments.end() ? &*it : index.Find(seq, element);
        if (held == nullptr) {
            return refuse(Reason::Invalid, "That row isn't assigned to anyone");
        }
        if (!isHost && held->owner != requester) {
            return refuse(Reason::Assigned, held->name + " is assigned to " + NameIn(roster, held->owner) + "; only they or the host can release it");
        }
        SeqObjectId gone = held->element;
        std::vector<Assignment> out;
        for (const auto& a : assignments) {
            if (a.element != gone) {
                out.push_back(a);
            }
        }
        return out;
    }
    const Element* el = seq.GetElementFromUid(element);
    if (el == nullptr) {
        return refuse(Reason::TargetGone, "That row is no longer in the sequence");
    }
    // A claim names the top-level row; a submodel's row comes with its model.
    SeqObjectId top = TopLevelOf(seq, element);
    if (top != element) {
        el = seq.GetElementFromUid(top);
        element = top;
        if (el == nullptr) {
            return refuse(Reason::TargetGone, "That row is no longer in the sequence");
        }
    }
    const std::string name = el->GetFullName();
    if (!InRoster(roster, owner)) {
        return refuse(Reason::Invalid, "That person is no longer in the session");
    }
    if (RoleIn(roster, owner) == ParticipantRole::Viewer) {
        return refuse(Reason::Viewer, NameIn(roster, owner) + " is a viewer, so rows can't be assigned to them");
    }
    if (!isHost && owner != requester) {
        return refuse(Reason::Invalid, "Only the host can assign rows to someone else");
    }
    std::vector<Assignment> others;
    bool same = false;
    for (const auto& a : assignments) {
        if (a.element == element) {
            if (a.owner == owner) {
                same = true;
            } else if (!isHost) {
                return refuse(Reason::Assigned, name + " is already assigned to " + NameIn(roster, a.owner));
            }
            continue;
        }
        others.push_back(a);
    }
    if (same) {
        return assignments;
    }
    // Every row both this claim and another participant's would cover.
    AssignmentIndex rest;
    rest.Build(seq, others);
    AssignmentIndex mine;
    mine.Build(seq, { Assignment{ element, owner, name } });
    const Assignment* clash = nullptr;
    auto check = [&](const Element* row) {
        if (clash == nullptr && row != nullptr && mine.Find(seq, row->GetUid()) != nullptr) {
            const Assignment* other = rest.Find(seq, row->GetUid());
            if (other != nullptr && other->owner != owner) {
                clash = other;
            }
        }
    };
    for (size_t i = 0; i < seq.GetElementCount(MASTER_VIEW) && clash == nullptr; ++i) {
        const Element* row = seq.GetElement(i, MASTER_VIEW);
        check(row);
        if (const auto* me = dynamic_cast<const ModelElement*>(row)) {
            for (int s = 0; s < me->GetSubModelAndStrandCount(); ++s) {
                check(me->GetSubModel(s));
            }
        }
    }
    if (clash != nullptr) {
        return refuse(Reason::Assigned, name + " overlaps " + clash->name + ", which is assigned to " + NameIn(roster, clash->owner));
    }
    others.push_back({ element, owner, name });
    return others;
}

namespace {
std::set<std::string> ModelRecordNames(const std::string& rgb) {
    std::set<std::string> out;
    pugi::xml_document doc;
    if (rgb.empty() || !doc.load_buffer(rgb.data(), rgb.size())) {
        return out;
    }
    pugi::xml_node root = doc.document_element();
    for (pugi::xml_node m : root.child("models").children("model")) {
        out.insert(m.attribute("name").as_string());
    }
    for (pugi::xml_node g : root.child("modelGroups").children("modelGroup")) {
        out.insert(g.attribute("name").as_string());
    }
    return out;
}
} // namespace

std::string LayoutTakesAssignedRow(const SequenceElements& seq, const std::vector<Assignment>& assignments, ParticipantId author,
                                   const std::string& beforeRgb, const std::string& afterRgb, ParticipantId* owner) {
    if (assignments.empty() || beforeRgb == afterRgb) {
        return {};
    }
    std::set<std::string> after = ModelRecordNames(afterRgb);
    AssignmentIndex index;
    index.Build(seq, assignments);
    for (const auto& name : ModelRecordNames(beforeRgb)) {
        if (after.count(name)) {
            continue;
        }
        const Element* row = seq.GetElement(name);
        const Assignment* a = row != nullptr ? index.Find(seq, row->GetUid()) : nullptr;
        if (a != nullptr && a->owner != author) {
            if (owner != nullptr) {
                *owner = a->owner;
            }
            return name;
        }
    }
    return {};
}

std::vector<Assignment> WithoutAssignments(const SequenceElements& seq, const std::vector<Assignment>& assignments, ParticipantId owner) {
    std::vector<Assignment> out;
    for (const auto& a : assignments) {
        if ((owner != kNoParticipant && a.owner == owner) || seq.GetElementFromUid(a.element) == nullptr) {
            continue;
        }
        out.push_back(a);
    }
    return out;
}

} // namespace Collab
