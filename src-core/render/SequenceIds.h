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

#include <atomic>
#include <cstdint>

// Identity for sequence objects (Effect, EffectLayer, Element) that stays the
// same for the life of the object no matter what is inserted, sorted or
// deleted around it. Unlike Effect::GetID() (the effect's position in its
// layer, renumbered on every sort) and EffectLayer::GetIndex() (a per-process
// counter), a SeqObjectId can be carried across an edit history and restored
// onto a re-created object, e.g. by undo.
//
// Ids are in memory only and are never written to the .xsq.
//
// The high 16 bits hold an allocator prefix so that ids allocated by
// different processes can be told apart; it defaults to 0. The low 48 bits
// are a counter.
using SeqObjectId = uint64_t;
inline constexpr SeqObjectId kNoSeqObjectId = 0;

namespace SeqIds {

inline std::atomic<uint64_t>& Counter() {
    static std::atomic<uint64_t> counter{ 0 };
    return counter;
}

inline std::atomic<uint16_t>& Peer() {
    static std::atomic<uint16_t> peer{ 0 };
    return peer;
}

constexpr uint64_t kCounterMask = (uint64_t(1) << 48) - 1;

inline SeqObjectId Next() {
    uint64_t c = (Counter().fetch_add(1, std::memory_order_relaxed) + 1) & kCounterMask;
    return (uint64_t(Peer().load(std::memory_order_relaxed)) << 48) | c;
}

inline void SetPeer(uint16_t peer) { Peer().store(peer, std::memory_order_relaxed); }
inline uint16_t GetPeer() { return Peer().load(std::memory_order_relaxed); }
constexpr uint16_t PeerOf(SeqObjectId id) { return uint16_t(id >> 48); }

} // namespace SeqIds
