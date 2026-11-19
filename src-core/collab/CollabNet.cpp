/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CollabNet.h"
#include "CollabNetWire.h"

#include <cstdio>
#include <random>

namespace Collab {

namespace {

uint32_t RandomU32() {
    static std::random_device rd;
    return rd();
}

} // namespace

std::string NewPairingCode() {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%06u", RandomU32() % 1000000u);
    return buf;
}

#ifndef __APPLE__
std::unique_ptr<INetTransport> CreateLanTransport(Dispatcher) {
    return nullptr;
}
std::unique_ptr<ISessionBrowser> CreateLanBrowser(Dispatcher) {
    return nullptr;
}
bool LanTransportAvailable() {
    return false;
}
#endif

namespace Wire {

namespace {

void PutU32(std::string& s, uint32_t v) {
    for (int i = 3; i >= 0; --i) {
        s.push_back((char)((v >> (8 * i)) & 0xFF));
    }
}

void PutU64(std::string& s, uint64_t v) {
    for (int i = 7; i >= 0; --i) {
        s.push_back((char)((v >> (8 * i)) & 0xFF));
    }
}

// Largest payload each type may carry; 0 = unknown type.
size_t MaxPayload(uint8_t type) {
    switch ((FrameType)type) {
    case FrameType::Welcome:
        return 4;
    case FrameType::Message:
        return kMaxMessageBytes;
    case FrameType::BlobStart:
        return 16;
    case FrameType::BlobData:
        return 8 + kBlobChunkBytes;
    case FrameType::BlobEnd:
        return 8;
    }
    return 0;
}

size_t MinPayload(uint8_t type) {
    switch ((FrameType)type) {
    case FrameType::Welcome:
        return 4;
    case FrameType::Message:
        return 1;
    case FrameType::BlobStart:
        return 16;
    case FrameType::BlobData:
        return 9;
    case FrameType::BlobEnd:
        return 8;
    }
    return 0;
}

} // namespace

std::string EncodeFrame(FrameType type, std::string_view payload) {
    std::string s;
    s.reserve(kHeaderBytes + payload.size());
    PutU32(s, (uint32_t)(payload.size() + 1));
    s.push_back((char)type);
    s.append(payload);
    return s;
}

std::string WelcomeFrame(PeerId assigned) {
    std::string p;
    PutU32(p, assigned);
    return EncodeFrame(FrameType::Welcome, p);
}

std::string BlobStartFrame(BlobId id, uint64_t size) {
    std::string p;
    PutU64(p, id);
    PutU64(p, size);
    return EncodeFrame(FrameType::BlobStart, p);
}

std::string BlobDataFrame(BlobId id, std::string_view bytes) {
    std::string p;
    p.reserve(8 + bytes.size());
    PutU64(p, id);
    p.append(bytes);
    return EncodeFrame(FrameType::BlobData, p);
}

std::string BlobEndFrame(BlobId id) {
    std::string p;
    PutU64(p, id);
    return EncodeFrame(FrameType::BlobEnd, p);
}

bool ReadU32(std::string_view p, size_t at, uint32_t& v) {
    if (at + 4 > p.size()) {
        return false;
    }
    v = 0;
    for (size_t i = 0; i < 4; ++i) {
        v = (v << 8) | (uint8_t)p[at + i];
    }
    return true;
}

bool ReadU64(std::string_view p, size_t at, uint64_t& v) {
    if (at + 8 > p.size()) {
        return false;
    }
    v = 0;
    for (size_t i = 0; i < 8; ++i) {
        v = (v << 8) | (uint8_t)p[at + i];
    }
    return true;
}

void FrameReader::Feed(const void* data, size_t len) {
    if (_bad || len == 0) {
        return;
    }
    if (_pos > 0 && _pos >= _buf.size() / 2) {
        _buf.erase(0, _pos);
        _pos = 0;
    }
    _buf.append((const char*)data, len);
}

FrameReader::Result FrameReader::Next(Frame& out) {
    if (_bad) {
        return Result::Bad;
    }
    std::string_view rest(_buf.data() + _pos, _buf.size() - _pos);
    if (rest.size() < kHeaderBytes) {
        return Result::More;
    }
    uint32_t len = 0;
    ReadU32(rest, 0, len);
    uint8_t type = (uint8_t)rest[4];
    size_t maxPayload = MaxPayload(type);
    // Checked before waiting for the payload, so a bogus length can't make
    // the reader buffer gigabytes first.
    if (maxPayload == 0 || len < 1 || len - 1 > maxPayload || len - 1 < MinPayload(type)) {
        _bad = true;
        return Result::Bad;
    }
    if (rest.size() < 4 + (size_t)len) {
        return Result::More;
    }
    out.type = (FrameType)type;
    out.payload.assign(rest.data() + kHeaderBytes, len - 1);
    _pos += 4 + (size_t)len;
    if (_pos == _buf.size()) {
        _buf.clear();
        _pos = 0;
    }
    return Result::Frame;
}

std::string NewSalt() {
    std::string s;
    for (size_t i = 0; i < kSaltBytes; ++i) {
        s.push_back((char)(RandomU32() & 0xFF));
    }
    return s;
}

std::string HexOf(std::string_view bytes) {
    static const char* digits = "0123456789abcdef";
    std::string out;
    for (unsigned char c : bytes) {
        out.push_back(digits[c >> 4]);
        out.push_back(digits[c & 0xF]);
    }
    return out;
}

bool SaltFromHex(std::string_view hex, std::string& salt) {
    if (hex.size() != kSaltBytes * 2) {
        return false;
    }
    auto nibble = [](char c) -> int {
        if (c >= '0' && c <= '9') {
            return c - '0';
        }
        if (c >= 'a' && c <= 'f') {
            return c - 'a' + 10;
        }
        if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        }
        return -1;
    };
    std::string out;
    for (size_t i = 0; i < hex.size(); i += 2) {
        int hi = nibble(hex[i]);
        int lo = nibble(hex[i + 1]);
        if (hi < 0 || lo < 0) {
            return false;
        }
        out.push_back((char)((hi << 4) | lo));
    }
    salt = out;
    return true;
}

std::string ServiceNameFor(const std::string& salt) {
    return std::string(kServicePrefix) + HexOf(salt);
}

bool SaltFromServiceName(std::string_view name, std::string& salt) {
    std::string_view prefix(kServicePrefix);
    if (name.substr(0, prefix.size()) != prefix) {
        return false;
    }
    std::string_view rest = name.substr(prefix.size());
    if (rest.size() < kSaltBytes * 2) {
        return false;
    }
    std::string_view tail = rest.substr(kSaltBytes * 2);
    if (!tail.empty() && tail[0] != ' ') {
        return false;
    }
    return SaltFromHex(rest.substr(0, kSaltBytes * 2), salt);
}

} // namespace Wire

} // namespace Collab
