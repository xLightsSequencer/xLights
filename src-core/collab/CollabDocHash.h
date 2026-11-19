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

// A 64-bit hash of a sequence's document state, used to detect peers that
// drifted apart. It covers everything commands change: rows in master-view
// order (ids, names, render-disable, fixed timing), every layer (id, name)
// and effect (id, type, times, protected, settings, palette), tags, faces,
// song structure, effect symbols and embedded media. Effects are hashed in
// time order, since moves don't re-sort a layer. Per-user state (selection,
// collapse, visibility, views) is not part of it.
//
// DocHasher caches each row's hash and recomputes only the rows a command
// touched (Invalidate) plus any whose change count moved.

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

class SequenceElements;
class SequenceCommand;
class Element;

namespace Collab {

class DocHasher {
public:
    uint64_t Hash(const SequenceElements& seq);
    // Call after applying `cmd` to `seq`.
    void Invalidate(const SequenceElements& seq, const SequenceCommand& cmd);
    void InvalidateElement(uint64_t topLevelUid) { _elements.erase(topLevelUid); }
    void InvalidateAll();

    // Recomputes everything (no cache).
    static uint64_t FullHash(const SequenceElements& seq);
    // One line per hashed part ("tags 1234", "row Grid #5 99..."), for
    // finding what differs between two peers.
    static std::string Components(const SequenceElements& seq);

    // For tests: when false, Invalidate does nothing (a broken hasher).
    bool invalidateEnabled = true;

private:
    struct Cached {
        const Element* element = nullptr;
        int changeCount = 0;
        uint64_t hash = 0;
    };
    std::unordered_map<uint64_t, Cached> _elements;
    bool _globalValid = false;
    uint64_t _global = 0;
    // Embedded media payload hashes, keyed by the shared payload (held so its
    // address can't be reused).
    std::unordered_map<const void*, std::pair<std::shared_ptr<const std::string>, uint64_t>> _payloads;

    uint64_t GlobalHash(const SequenceElements& seq);
};

} // namespace Collab
