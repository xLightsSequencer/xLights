#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <cstring>
#include <new>

// Last: FX.h defines short macros (byte, instance, ...) for the WLED code.
#include "effects/FX.h"

// CRGBPalette16's 5-8 colour constructors spread the colours evenly over its
// 16 entries with one gradient per pair of neighbouring colours.

namespace {

bool Same(const CRGB& a, const CRGB& b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

} // namespace

TEST_CASE("CRGBPalette16 gradients of 5 to 8 colours stay inside the 16 entries", "[effects][singlestrand]") {
    const CRGB c[8] = { CRGB(255, 0, 0), CRGB(0, 255, 0), CRGB(0, 0, 255), CRGB(255, 255, 0),
                        CRGB(0, 255, 255), CRGB(255, 0, 255), CRGB(255, 255, 255), CRGB(10, 20, 30) };
    for (int n = 5; n <= 8; ++n) {
        INFO("colours " << n);
        // Build the palette in place with a guard area right after its entries,
        // so a write past entry 15 shows up whatever the stack layout.
        constexpr size_t kGuard = 4 * sizeof(CRGB);
        alignas(CRGBPalette16) unsigned char buf[sizeof(CRGBPalette16) + kGuard];
        std::memset(buf, 0xA5, sizeof(buf));
        CRGBPalette16* p = nullptr;
        switch (n) {
        case 5: p = new (buf) CRGBPalette16(c[0], c[1], c[2], c[3], c[4]); break;
        case 6: p = new (buf) CRGBPalette16(c[0], c[1], c[2], c[3], c[4], c[5]); break;
        case 7: p = new (buf) CRGBPalette16(c[0], c[1], c[2], c[3], c[4], c[5], c[6]); break;
        case 8: p = new (buf) CRGBPalette16(c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7]); break;
        }
        bool guardIntact = true;
        for (size_t i = sizeof(CRGBPalette16); i < sizeof(buf); ++i) {
            guardIntact = guardIntact && buf[i] == 0xA5;
        }
        CHECK(guardIntact);

        // Every colour lands on its own stop: the first and last at the ends,
        // the rest at (16 * i) / (n - 1).
        CHECK(Same(p->entries[0], c[0]));
        CHECK(Same(p->entries[15], c[n - 1]));
        for (int i = 1; i < n - 1; ++i) {
            INFO("colour " << i + 1);
            CHECK(Same(p->entries[(16 * i) / (n - 1)], c[i]));
        }
    }
}
