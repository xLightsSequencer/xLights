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

// Viewer / editor roles and row assignments in a live session: what a
// participant may change. The host decides (its Roster lists the roles and
// the assignments) and enforces it on every proposal; the same checks let an
// app refuse an edit before it applies locally.
//
// An assignment gives one row to one participant: only they may change it.
// A claim on a model also covers its submodel and strand rows; a claim on a
// model group covers the group's row and every member's (members of nested
// groups too, and their submodels and strands). Timing tracks are covered by
// their own claim only.

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "CollabMessages.h"
#include "render/SequenceCommands.h"

class SequenceElements;

namespace Collab {

// The rows (element uids, as the command names them: submodel and strand
// rows included) `cmd` writes, before it is applied.
std::vector<SeqObjectId> CommandRows(const SequenceCommand& cmd);

// The full names of the model rows a claim on `element` covers (the row
// itself; a group's members, recursively). Empty for a timing track or an
// element that isn't in the sequence.
std::set<std::string> CoveredNames(const SequenceElements& seq, SeqObjectId element);

// Looks up who holds a row under a set of assignments. Build it again when
// the assignments, the rows or the groups change.
class AssignmentIndex {
public:
    void Build(const SequenceElements& seq, const std::vector<Assignment>& assignments);
    // The claim that covers `row` (directly, through its model, or through a
    // group), or nullptr.
    const Assignment* Find(const SequenceElements& seq, SeqObjectId row) const;
    ParticipantId OwnerOf(const SequenceElements& seq, SeqObjectId row) const {
        const Assignment* a = Find(seq, row);
        return a == nullptr ? kNoParticipant : a->owner;
    }
    const Assignment* FindName(const std::string& fullName) const;
    bool Empty() const { return _assignments.empty(); }

private:
    std::vector<Assignment> _assignments;
    std::unordered_map<SeqObjectId, size_t> _byUid;
    std::unordered_map<std::string, size_t> _byName;
};

ParticipantRole RoleIn(const std::vector<RosterEntry>& roster, ParticipantId who);

// Why `author` may not make `cmd`: reason Reason::Viewer or
// Reason::Assigned (with `owner` and the row's name), or an empty reason.
struct EditRefusal {
    std::string reason;
    ParticipantId owner = kNoParticipant;
    std::string row;
    bool Refused() const { return !reason.empty(); }
};
EditRefusal CheckEdit(const SequenceElements& seq, const std::vector<RosterEntry>& roster, const AssignmentIndex& index, ParticipantId author,
                      const SequenceCommand& cmd);

// The host's rule for a claim: `requester` assigns `element` to `owner`
// (owner kNoParticipant: releases it). Returns the assignments after it, or
// sets `reason` (Reason::*) and readable `text` and returns them unchanged.
// The host may assign rows to any editor and release any claim; an editor
// may claim rows for itself and release its own. A claim may not overlap
// someone else's (a model inside a group they hold, or the other way
// round); the host's claim on a row that is already held replaces it.
std::vector<Assignment> ApplyClaim(const SequenceElements& seq, const std::vector<RosterEntry>& roster, const std::vector<Assignment>& assignments,
                                   ParticipantId requester, SeqObjectId element, ParticipantId owner, std::string& reason, std::string& text);

// A layout change from `beforeRgb` to `afterRgb` (rgbeffects text) that
// deletes or renames a model or group whose row `assignments` give to
// someone other than `author` would take that row from them: the row's name
// (and its owner in `owner`), or empty when it doesn't.
std::string LayoutTakesAssignedRow(const SequenceElements& seq, const std::vector<Assignment>& assignments, ParticipantId author,
                                   const std::string& beforeRgb, const std::string& afterRgb, ParticipantId* owner);

// `assignments` without those of `owner`, or of rows no longer in the
// sequence (owner kNoParticipant: only the latter).
std::vector<Assignment> WithoutAssignments(const SequenceElements& seq, const std::vector<Assignment>& assignments, ParticipantId owner);

} // namespace Collab
