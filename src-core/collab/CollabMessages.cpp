/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CollabMessages.h"

#include <random>

#include <array>
#include <cstring>

using nlohmann::json;

namespace Collab {

namespace {

constexpr std::array<std::pair<MsgType, const char*>, 13> kTypeNames{ {
    { MsgType::Hello, "Hello" },
    { MsgType::Welcome, "Welcome" },
    { MsgType::Refuse, "Refuse" },
    { MsgType::Snapshot, "Snapshot" },
    { MsgType::NeedSnapshot, "NeedSnapshot" },
    { MsgType::Propose, "Propose" },
    { MsgType::Commit, "Commit" },
    { MsgType::Reject, "Reject" },
    { MsgType::Resend, "Resend" },
    { MsgType::Presence, "Presence" },
    { MsgType::Ping, "Ping" },
    { MsgType::Pong, "Pong" },
    { MsgType::Bye, "Bye" },
} };

bool TypeFromName(const std::string& s, MsgType& out) {
    for (const auto& [t, n] : kTypeNames) {
        if (s == n) {
            out = t;
            return true;
        }
    }
    return false;
}

const char* KindName(EditKind k) {
    switch (k) {
    case EditKind::Undo: return "undo";
    case EditKind::Redo: return "redo";
    default: return "edit";
    }
}

bool KindFromName(const std::string& s, EditKind& out) {
    if (s == "edit") {
        out = EditKind::Edit;
    } else if (s == "undo") {
        out = EditKind::Undo;
    } else if (s == "redo") {
        out = EditKind::Redo;
    } else {
        return false;
    }
    return true;
}

// Rejects frames nested deeper than kMaxNesting before parsing them, so a
// hostile frame can't make the parser or the json destructor recurse deeply.
bool NestingOk(std::string_view s) {
    int depth = 0;
    bool inString = false;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (inString) {
            if (c == '\\') {
                ++i;
            } else if (c == '"') {
                inString = false;
            }
            continue;
        }
        if (c == '"') {
            inString = true;
        } else if (c == '{' || c == '[') {
            if (++depth > kMaxNesting) {
                return false;
            }
        } else if (c == '}' || c == ']') {
            --depth;
        }
    }
    return true;
}

// Optional-field readers: absent is fine (the default stays), present with
// the wrong type fails the whole frame.
bool Get(const json& j, const char* key, std::string& out) {
    auto it = j.find(key);
    if (it == j.end()) {
        return true;
    }
    if (!it->is_string()) {
        return false;
    }
    out = it->get_ref<const std::string&>();
    return true;
}

bool Get(const json& j, const char* key, uint64_t& out) {
    auto it = j.find(key);
    if (it == j.end()) {
        return true;
    }
    if (!it->is_number_unsigned()) {
        // Small non-negative values may arrive as signed integers.
        if (it->is_number_integer() && it->get<int64_t>() >= 0) {
            out = (uint64_t)it->get<int64_t>();
            return true;
        }
        return false;
    }
    out = it->get<uint64_t>();
    return true;
}

bool Get(const json& j, const char* key, int& out) {
    auto it = j.find(key);
    if (it == j.end()) {
        return true;
    }
    if (!it->is_number_integer()) {
        return false;
    }
    int64_t v = it->get<int64_t>();
    if (v < INT32_MIN || v > INT32_MAX) {
        return false;
    }
    out = (int)v;
    return true;
}

bool Get(const json& j, const char* key, ParticipantId& out) {
    uint64_t v = out;
    if (!Get(j, key, v) || v > 0xFFFF) {
        return false;
    }
    out = (ParticipantId)v;
    return true;
}

bool GetIds(const json& j, const char* key, std::vector<SeqObjectId>& out) {
    auto it = j.find(key);
    if (it == j.end()) {
        return true;
    }
    if (!it->is_array()) {
        return false;
    }
    out.clear();
    for (const auto& v : *it) {
        if (!v.is_number_unsigned()) {
            return false;
        }
        out.push_back(v.get<uint64_t>());
    }
    return true;
}

json WritePresence(const PresenceInfo& p) {
    return json{ { "start", p.startMS }, { "end", p.endMS }, { "focus", p.focusedElements }, { "sel", p.selectedEffects }, { "lock", p.softLock } };
}

bool ReadPresence(const json& j, PresenceInfo& p) {
    if (!j.is_object()) {
        return false;
    }
    return Get(j, "start", p.startMS) && Get(j, "end", p.endMS) && GetIds(j, "focus", p.focusedElements) &&
           GetIds(j, "sel", p.selectedEffects) && Get(j, "lock", p.softLock);
}

} // namespace

const char* MsgTypeName(MsgType t) {
    for (const auto& [type, n] : kTypeNames) {
        if (type == t) {
            return n;
        }
    }
    return "?";
}

std::string EncodeMessage(const Message& m) {
    json j{ { "v", m.protocol }, { "t", MsgTypeName(m.type) } };
    switch (m.type) {
    case MsgType::Hello:
        j["app"] = m.appVersion;
        j["name"] = m.name;
        j["device"] = m.deviceKind;
        break;
    case MsgType::Welcome:
        j["participant"] = m.participant;
        j["rev"] = m.rev;
        j["name"] = m.name;
        break;
    case MsgType::Refuse:
    case MsgType::Bye:
    case MsgType::NeedSnapshot:
        j["reason"] = m.reason;
        j["text"] = m.text;
        break;
    case MsgType::Snapshot:
        j["rev"] = m.rev;
        j["hash"] = m.docHash;
        j["seq"] = m.clientSeq;
        j["xsq"] = m.xsq;
        j["uids"] = m.uids;
        break;
    case MsgType::Propose:
        j["seq"] = m.clientSeq;
        j["base"] = m.baseRev;
        j["kind"] = KindName(m.kind);
        j["undoOf"] = m.undoOf;
        j["cmd"] = m.command;
        break;
    case MsgType::Commit:
        j["rev"] = m.rev;
        j["author"] = m.participant;
        j["seq"] = m.clientSeq;
        j["kind"] = KindName(m.kind);
        j["undoOf"] = m.undoOf;
        j["cmd"] = m.command;
        j["hash"] = m.docHash;
        break;
    case MsgType::Reject:
        j["seq"] = m.clientSeq;
        j["rev"] = m.rev;
        j["kind"] = KindName(m.kind);
        j["reason"] = m.reason;
        j["text"] = m.text;
        j["by"] = m.participant;
        break;
    case MsgType::Resend:
        j["rev"] = m.rev;
        break;
    case MsgType::Presence:
        j["participant"] = m.participant;
        j["presence"] = WritePresence(m.presence);
        break;
    case MsgType::Ping:
    case MsgType::Pong:
        j["rev"] = m.rev;
        j["time"] = m.time;
        break;
    }
    // Strings that aren't valid UTF-8 are replaced rather than throwing; the
    // host applies the decoded form of every command, so all peers agree.
    return j.dump(-1, ' ', false, json::error_handler_t::replace);
}

std::optional<Message> DecodeMessage(std::string_view frame) {
    if (frame.empty() || frame.size() > kMaxFrameBytes || !NestingOk(frame)) {
        return std::nullopt;
    }
    json j = json::parse(frame.begin(), frame.end(), nullptr, false);
    if (j.is_discarded() || !j.is_object()) {
        return std::nullopt;
    }
    Message m;
    std::string type;
    if (!Get(j, "v", m.protocol) || !Get(j, "t", type) || !TypeFromName(type, m.type)) {
        return std::nullopt;
    }
    if (m.protocol != kProtocolVersion && m.type != MsgType::Hello && m.type != MsgType::Refuse && m.type != MsgType::Bye) {
        return std::nullopt;
    }
    std::string kind = "edit";
    bool ok = true;
    switch (m.type) {
    case MsgType::Hello:
        ok = Get(j, "app", m.appVersion) && Get(j, "name", m.name) && Get(j, "device", m.deviceKind);
        break;
    case MsgType::Welcome:
        ok = Get(j, "participant", m.participant) && Get(j, "rev", m.rev) && Get(j, "name", m.name) && m.participant != kNoParticipant;
        break;
    case MsgType::Refuse:
    case MsgType::Bye:
    case MsgType::NeedSnapshot:
        ok = Get(j, "reason", m.reason) && Get(j, "text", m.text);
        break;
    case MsgType::Snapshot: {
        ok = Get(j, "rev", m.rev) && Get(j, "hash", m.docHash) && Get(j, "seq", m.clientSeq) && Get(j, "xsq", m.xsq);
        auto it = j.find("uids");
        ok = ok && it != j.end() && it->is_array();
        if (ok) {
            m.uids = std::move(*it);
        }
        break;
    }
    case MsgType::Propose:
    case MsgType::Commit: {
        ok = Get(j, "seq", m.clientSeq) && Get(j, "kind", kind) && KindFromName(kind, m.kind) && Get(j, "undoOf", m.undoOf);
        if (m.type == MsgType::Propose) {
            ok = ok && Get(j, "base", m.baseRev);
        } else {
            ok = ok && Get(j, "rev", m.rev) && Get(j, "author", m.participant) && Get(j, "hash", m.docHash) && m.rev != 0;
        }
        auto it = j.find("cmd");
        ok = ok && it != j.end() && it->is_object();
        if (ok) {
            m.command = std::move(*it);
        }
        break;
    }
    case MsgType::Reject:
        ok = Get(j, "seq", m.clientSeq) && Get(j, "rev", m.rev) && Get(j, "kind", kind) && KindFromName(kind, m.kind) &&
             Get(j, "reason", m.reason) && Get(j, "text", m.text) && Get(j, "by", m.participant);
        break;
    case MsgType::Resend:
        ok = Get(j, "rev", m.rev);
        break;
    case MsgType::Presence: {
        ok = Get(j, "participant", m.participant);
        auto it = j.find("presence");
        ok = ok && it != j.end() && ReadPresence(*it, m.presence);
        break;
    }
    case MsgType::Ping:
    case MsgType::Pong:
        ok = Get(j, "rev", m.rev) && Get(j, "time", m.time);
        break;
    }
    if (!ok) {
        return std::nullopt;
    }
    return m;
}

} // namespace Collab
