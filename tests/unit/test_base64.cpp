#include <catch2/catch_test_macros.hpp>

#include "utils/Base64.h"

using namespace Base64;

namespace {
std::string Enc(const std::string& s) {
    return Encode(reinterpret_cast<const uint8_t*>(s.data()), s.size());
}
std::string Dec(const std::string& s) {
    auto v = Decode(s);
    return std::string(v.begin(), v.end());
}
}

TEST_CASE("Base64 encodes the RFC 4648 vectors", "[base64]") {
    CHECK(Enc("") == "");
    CHECK(Enc("f") == "Zg==");
    CHECK(Enc("fo") == "Zm8=");
    CHECK(Enc("foo") == "Zm9v");
    CHECK(Enc("foob") == "Zm9vYg==");
    CHECK(Enc("fooba") == "Zm9vYmE=");
    CHECK(Enc("foobar") == "Zm9vYmFy");
}

TEST_CASE("Base64 decodes the RFC 4648 vectors", "[base64]") {
    CHECK(Dec("") == "");
    CHECK(Dec("Zg==") == "f");
    CHECK(Dec("Zm9vYmE=") == "fooba");
    CHECK(Dec("Zm9vYmFy") == "foobar");
}

TEST_CASE("Base64 decode ignores whitespace and padding", "[base64]") {
    CHECK(Dec("Zm9v\r\nYmFy") == "foobar");
    CHECK(Dec("Zm 9v YmFy") == "foobar");
    CHECK(Dec("Zm9vYg") == "foob");
}

TEST_CASE("Base64 round-trips every byte value", "[base64]") {
    std::vector<uint8_t> all;
    for (int i = 0; i < 256; ++i) all.push_back(static_cast<uint8_t>(i));
    for (size_t len : {0u, 1u, 2u, 3u, 4u, 255u, 256u}) {
        auto e = Encode(all.data(), len);
        CHECK(Decode(e) == std::vector<uint8_t>(all.begin(), all.begin() + len));
    }
}
