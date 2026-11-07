/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "Sha256.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <vector>

namespace {

constexpr uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

inline uint32_t rotr(uint32_t x, int n) {
    return (x >> n) | (x << (32 - n));
}

} // namespace

Sha256::Sha256() {
    _state[0] = 0x6a09e667;
    _state[1] = 0xbb67ae85;
    _state[2] = 0x3c6ef372;
    _state[3] = 0xa54ff53a;
    _state[4] = 0x510e527f;
    _state[5] = 0x9b05688c;
    _state[6] = 0x1f83d9ab;
    _state[7] = 0x5be0cd19;
}

void Sha256::transform(const uint8_t* block) {
    uint32_t w[64];
    for (int i = 0; i < 16; ++i) {
        w[i] = (uint32_t(block[i * 4]) << 24) | (uint32_t(block[i * 4 + 1]) << 16) |
               (uint32_t(block[i * 4 + 2]) << 8) | uint32_t(block[i * 4 + 3]);
    }
    for (int i = 16; i < 64; ++i) {
        uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    uint32_t a = _state[0], b = _state[1], c = _state[2], d = _state[3];
    uint32_t e = _state[4], f = _state[5], g = _state[6], h = _state[7];
    for (int i = 0; i < 64; ++i) {
        uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
        uint32_t ch = (e & f) ^ (~e & g);
        uint32_t t1 = h + S1 + ch + K[i] + w[i];
        uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t t2 = S0 + maj;
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }
    _state[0] += a;
    _state[1] += b;
    _state[2] += c;
    _state[3] += d;
    _state[4] += e;
    _state[5] += f;
    _state[6] += g;
    _state[7] += h;
}

void Sha256::update(const void* data, size_t len) {
    const uint8_t* p = static_cast<const uint8_t*>(data);
    _totalLen += len;
    if (_bufferLen > 0) {
        size_t take = std::min(len, sizeof(_buffer) - _bufferLen);
        memcpy(_buffer + _bufferLen, p, take);
        _bufferLen += take;
        p += take;
        len -= take;
        if (_bufferLen == sizeof(_buffer)) {
            transform(_buffer);
            _bufferLen = 0;
        }
    }
    while (len >= sizeof(_buffer)) {
        transform(p);
        p += sizeof(_buffer);
        len -= sizeof(_buffer);
    }
    if (len > 0) {
        memcpy(_buffer, p, len);
        _bufferLen = len;
    }
}

Sha256::Digest Sha256::finish() {
    uint64_t bitLen = _totalLen * 8;
    uint8_t pad[72] = { 0x80 };
    size_t padLen = (_bufferLen < 56) ? (56 - _bufferLen) : (120 - _bufferLen);
    uint8_t lenBytes[8];
    for (int i = 0; i < 8; ++i) {
        lenBytes[i] = uint8_t(bitLen >> (56 - i * 8));
    }
    update(pad, padLen);
    update(lenBytes, 8);

    Digest out;
    for (int i = 0; i < 8; ++i) {
        out[i * 4] = uint8_t(_state[i] >> 24);
        out[i * 4 + 1] = uint8_t(_state[i] >> 16);
        out[i * 4 + 2] = uint8_t(_state[i] >> 8);
        out[i * 4 + 3] = uint8_t(_state[i]);
    }
    return out;
}

std::string Sha256::ToHex(const Digest& d) {
    static const char* hex = "0123456789abcdef";
    std::string s;
    s.reserve(64);
    for (uint8_t b : d) {
        s.push_back(hex[b >> 4]);
        s.push_back(hex[b & 0x0F]);
    }
    return s;
}

std::string Sha256::HexOf(const void* data, size_t len) {
    Sha256 h;
    h.update(data, len);
    return ToHex(h.finish());
}

std::optional<std::string> Sha256::HexOfFile(const std::filesystem::path& file) {
    std::ifstream in(file, std::ios::binary);
    if (!in.is_open()) {
        return std::nullopt;
    }
    Sha256 h;
    std::vector<char> buf(1024 * 1024);
    while (in) {
        in.read(buf.data(), (std::streamsize)buf.size());
        std::streamsize n = in.gcount();
        if (n > 0) {
            h.update(buf.data(), (size_t)n);
        }
    }
    if (in.bad()) {
        return std::nullopt;
    }
    return ToHex(h.finish());
}
