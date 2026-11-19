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

constexpr std::array<std::pair<MsgType, const char*>, 19> kTypeNames{ {
    { MsgType::Hello, "Hello" },
    { MsgType::Welcome, "Welcome" },
    { MsgType::Refuse, "Refuse" },
    { MsgType::Manifest, "Manifest" },
    { MsgType::NeedFiles, "NeedFiles" },
    { MsgType::File, "File" },
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
    { MsgType::Roster, "Roster" },
    { MsgType::Notice, "Notice" },
    { MsgType::Claim, "Claim" },
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

bool RoleFromName(const std::string& s, FileRole& out) {
    for (FileRole r : { FileRole::Layout, FileRole::Audio, FileRole::Media }) {
        if (s == FileRoleName(r)) {
            out = r;
            return true;
        }
    }
    return false;
}

json WriteFiles(const std::vector<FileEntry>& files) {
    json a = json::array();
    for (const auto& f : files) {
        a.push_back(json{ { "path", f.path }, { "role", FileRoleName(f.role) }, { "size", f.size }, { "hash", f.hash } });
    }
    return a;
}

bool PathOk(const std::string& p) {
    return !p.empty() && p.size() <= kMaxPathBytes && p.find('\0') == std::string::npos;
}

bool ReadFiles(const json& j, std::vector<FileEntry>& out) {
    auto it = j.find("files");
    if (it == j.end()) {
        return true;
    }
    if (!it->is_array() || it->size() > kMaxManifestFiles) {
        return false;
    }
    out.clear();
    out.reserve(it->size());
    for (const auto& f : *it) {
        if (!f.is_object()) {
            return false;
        }
        FileEntry e;
        std::string role;
        if (!Get(f, "path", e.path) || !Get(f, "role", role) || !RoleFromName(role, e.role) || !Get(f, "size", e.size) ||
            !Get(f, "hash", e.hash) || !PathOk(e.path)) {
            return false;
        }
        out.push_back(std::move(e));
    }
    return true;
}

bool ReadPaths(const json& j, std::vector<std::string>& out) {
    auto it = j.find("paths");
    if (it == j.end()) {
        return true;
    }
    if (!it->is_array() || it->size() > kMaxManifestFiles) {
        return false;
    }
    out.clear();
    for (const auto& p : *it) {
        if (!p.is_string() || !PathOk(p.get_ref<const std::string&>())) {
            return false;
        }
        out.push_back(p.get<std::string>());
    }
    return true;
}

json WritePresence(const PresenceInfo& p) {
    json j{ { "start", p.startMS }, { "end", p.endMS }, { "focus", p.focusedElements }, { "sel", p.selectedEffects }, { "lock", p.softLock } };
    if (!p.layoutSelected.empty()) {
        j["lsel"] = p.layoutSelected;
    }
    if (!p.layoutLock.object.empty()) {
        j["llock"] = json{ { "o", p.layoutLock.object }, { "n", p.layoutLock.name }, { "w", p.layoutLock.what } };
    }
    return j;
}

bool GetStrings(const json& j, const char* key, std::vector<std::string>& out) {
    auto it = j.find(key);
    if (it == j.end()) {
        return true;
    }
    if (!it->is_array() || it->size() > kMaxManifestFiles) {
        return false;
    }
    out.clear();
    for (const auto& v : *it) {
        if (!v.is_string() || v.get_ref<const std::string&>().size() > kMaxPathBytes) {
            return false;
        }
        out.push_back(v.get<std::string>());
    }
    return true;
}

bool ReadPresence(const json& j, PresenceInfo& p) {
    if (!j.is_object()) {
        return false;
    }
    bool ok = Get(j, "start", p.startMS) && Get(j, "end", p.endMS) && GetIds(j, "focus", p.focusedElements) &&
              GetIds(j, "sel", p.selectedEffects) && Get(j, "lock", p.softLock) && GetStrings(j, "lsel", p.layoutSelected);
    if (auto it = j.find("llock"); ok && it != j.end()) {
        ok = it->is_object() && Get(*it, "o", p.layoutLock.object) && Get(*it, "n", p.layoutLock.name) && Get(*it, "w", p.layoutLock.what);
    }
    return ok;
}

void WriteLayoutFiles(json& j, const Message& m) {
    if (m.layoutHash != 0) {
        j["lhash"] = m.layoutHash;
        j["lrgb"] = m.layoutFiles.rgbEffects;
        j["lnet"] = m.layoutFiles.networks;
    }
}

bool ReadLayoutFiles(const json& j, Message& m) {
    return Get(j, "lhash", m.layoutHash) && Get(j, "lrgb", m.layoutFiles.rgbEffects) && Get(j, "lnet", m.layoutFiles.networks);
}

bool GetBool(const json& j, const char* key, bool& out) {
    auto it = j.find(key);
    if (it == j.end()) {
        return true;
    }
    if (!it->is_boolean()) {
        return false;
    }
    out = it->get<bool>();
    return true;
}

json WriteRoster(const std::vector<RosterEntry>& roster) {
    json a = json::array();
    for (const auto& r : roster) {
        json p{ { "id", r.participant }, { "name", r.name }, { "device", r.deviceKind } };
        if (r.role == ParticipantRole::Viewer) {
            p["role"] = "viewer";
        }
        if (!r.secretHash.empty()) {
            p["sh"] = r.secretHash;
        }
        a.push_back(std::move(p));
    }
    return a;
}

json WriteAssignments(const std::vector<Assignment>& as) {
    json a = json::array();
    for (const auto& x : as) {
        a.push_back(json{ { "e", x.element }, { "o", x.owner }, { "n", x.name } });
    }
    return a;
}

bool ReadAssignments(const json& j, std::vector<Assignment>& out) {
    auto it = j.find("assign");
    if (it == j.end()) {
        return true;
    }
    if (!it->is_array() || it->size() > kMaxAssignments) {
        return false;
    }
    out.clear();
    for (const auto& a : *it) {
        if (!a.is_object()) {
            return false;
        }
        Assignment x;
        if (!Get(a, "e", x.element) || !Get(a, "o", x.owner) || !Get(a, "n", x.name) || x.element == kNoSeqObjectId || x.name.size() > kMaxPathBytes) {
            return false;
        }
        out.push_back(std::move(x));
    }
    return true;
}

bool ReadRoster(const json& j, std::vector<RosterEntry>& out) {
    auto it = j.find("people");
    if (it == j.end() || !it->is_array() || it->size() > kMaxRoster) {
        return false;
    }
    out.clear();
    for (const auto& p : *it) {
        if (!p.is_object()) {
            return false;
        }
        RosterEntry r;
        std::string role;
        if (!Get(p, "id", r.participant) || !Get(p, "name", r.name) || !Get(p, "device", r.deviceKind) || !Get(p, "role", role) ||
            !Get(p, "sh", r.secretHash) || r.participant == kNoParticipant) {
            return false;
        }
        if (role == "viewer") {
            r.role = ParticipantRole::Viewer;
        } else if (!role.empty() && role != "editor") {
            return false;
        }
        out.push_back(std::move(r));
    }
    return true;
}

} // namespace

const char* FileRoleName(FileRole r) {
    switch (r) {
    case FileRole::Layout: return "layout";
    case FileRole::Audio: return "audio";
    default: return "media";
    }
}

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
        if (m.liveLayout) {
            j["live"] = true;
        }
        break;
    case MsgType::Welcome:
        j["participant"] = m.participant;
        j["rev"] = m.rev;
        j["name"] = m.name;
        if (m.liveLayout) {
            j["live"] = true;
        }
        WriteLayoutFiles(j, m);
        break;
    case MsgType::Refuse:
    case MsgType::Bye:
    case MsgType::NeedSnapshot:
    case MsgType::Notice:
        j["reason"] = m.reason;
        j["text"] = m.text;
        break;
    case MsgType::Manifest:
        j["files"] = WriteFiles(m.files);
        break;
    case MsgType::NeedFiles:
        j["paths"] = m.paths;
        break;
    case MsgType::File:
        j["blob"] = m.blob;
        j["files"] = WriteFiles(m.files);
        j["reason"] = m.reason;
        break;
    case MsgType::Snapshot:
        j["rev"] = m.rev;
        j["hash"] = m.docHash;
        j["seq"] = m.clientSeq;
        j["blob"] = m.blob;
        j["uids"] = m.uids;
        WriteLayoutFiles(j, m);
        break;
    case MsgType::Propose:
        j["seq"] = m.clientSeq;
        j["base"] = m.baseRev;
        j["kind"] = KindName(m.kind);
        j["undoOf"] = m.undoOf;
        if (!m.layout.is_null()) {
            j["layout"] = m.layout;
        } else {
            j["cmd"] = m.command;
        }
        break;
    case MsgType::Commit:
        j["rev"] = m.rev;
        j["author"] = m.participant;
        j["seq"] = m.clientSeq;
        j["kind"] = KindName(m.kind);
        j["undoOf"] = m.undoOf;
        if (!m.layout.is_null()) {
            j["layout"] = m.layout;
            j["lhash"] = m.layoutHash;
            if (!m.text.empty()) {
                j["text"] = m.text;
            }
        } else {
            j["cmd"] = m.command;
        }
        j["hash"] = m.docHash;
        if (m.lenient) {
            j["lenient"] = true;
        }
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
        if (m.lockOrder != 0) {
            j["lorder"] = m.lockOrder;
        }
        break;
    case MsgType::Ping:
    case MsgType::Pong:
        j["rev"] = m.rev;
        j["time"] = m.time;
        if (m.layoutHash != 0) {
            j["lhash"] = m.layoutHash;
        }
        break;
    case MsgType::Roster:
        j["people"] = WriteRoster(m.roster);
        j["assign"] = WriteAssignments(m.assignments);
        break;
    case MsgType::Claim:
        j["assign"] = WriteAssignments(m.assignments);
        break;
    }
    // Strings that aren't valid UTF-8 are replaced rather than throwing; the
    // host applies the decoded form of every command, so all peers agree.
    return j.dump(-1, ' ', false, json::error_handler_t::replace);
}

std::string NewRejoinSecret() {
    std::random_device rd;
    static const char* kHex = "0123456789abcdef";
    std::string s;
    for (int i = 0; i < 8; ++i) {
        uint32_t v = rd();
        for (int b = 0; b < 4; ++b) {
            s.push_back(kHex[(v >> (8 * b + 4)) & 0xF]);
            s.push_back(kHex[(v >> (8 * b)) & 0xF]);
        }
    }
    return s;
}

std::optional<nlohmann::json> ParseBoundedJson(std::string_view text) {
    if (!NestingOk(text)) {
        return std::nullopt;
    }
    json j = json::parse(text.begin(), text.end(), nullptr, false);
    if (j.is_discarded()) {
        return std::nullopt;
    }
    return j;
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
        ok = Get(j, "app", m.appVersion) && Get(j, "name", m.name) && Get(j, "device", m.deviceKind) && GetBool(j, "live", m.liveLayout);
        break;
    case MsgType::Welcome:
        ok = Get(j, "participant", m.participant) && Get(j, "rev", m.rev) && Get(j, "name", m.name) && m.participant != kNoParticipant &&
             GetBool(j, "live", m.liveLayout) && ReadLayoutFiles(j, m);
        break;
    case MsgType::Refuse:
    case MsgType::Bye:
    case MsgType::NeedSnapshot:
    case MsgType::Notice:
        ok = Get(j, "reason", m.reason) && Get(j, "text", m.text);
        break;
    case MsgType::Manifest: {
        ok = j.contains("files") && ReadFiles(j, m.files);
        break;
    }
    case MsgType::NeedFiles:
        ok = j.contains("paths") && ReadPaths(j, m.paths);
        break;
    case MsgType::File:
        // One file; a blob unless it says why there is none.
        ok = Get(j, "blob", m.blob) && ReadFiles(j, m.files) && Get(j, "reason", m.reason) && m.files.size() == 1 &&
             ((m.blob != 0) != !m.reason.empty());
        break;
    case MsgType::Snapshot: {
        ok = Get(j, "rev", m.rev) && Get(j, "hash", m.docHash) && Get(j, "seq", m.clientSeq) && Get(j, "blob", m.blob) && m.blob != 0;
        auto it = j.find("uids");
        ok = ok && it != j.end() && it->is_array() && ReadLayoutFiles(j, m);
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
            if (auto l = j.find("lenient"); l != j.end()) {
                ok = ok && l->is_boolean();
                m.lenient = ok && l->get<bool>();
            }
        }
        // Exactly one of a command and a layout edit.
        auto it = j.find("cmd");
        auto lay = j.find("layout");
        if (lay != j.end()) {
            ok = ok && it == j.end() && lay->is_object() && !lay->empty() && Get(j, "lhash", m.layoutHash) && Get(j, "text", m.text);
            if (ok) {
                m.layout = std::move(*lay);
            }
        } else {
            ok = ok && it != j.end() && it->is_object();
            if (ok) {
                m.command = std::move(*it);
            }
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
        ok = ok && it != j.end() && ReadPresence(*it, m.presence) && Get(j, "lorder", m.lockOrder);
        break;
    }
    case MsgType::Ping:
    case MsgType::Pong:
        ok = Get(j, "rev", m.rev) && Get(j, "time", m.time) && Get(j, "lhash", m.layoutHash);
        break;
    case MsgType::Roster:
        ok = ReadRoster(j, m.roster) && ReadAssignments(j, m.assignments);
        break;
    case MsgType::Claim:
        ok = j.contains("assign") && ReadAssignments(j, m.assignments) && !m.assignments.empty();
        break;
    }
    if (!ok) {
        return std::nullopt;
    }
    return m;
}

} // namespace Collab
