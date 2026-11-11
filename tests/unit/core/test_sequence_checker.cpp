#include <catch2/catch_test_macros.hpp>

#include "diagnostics/SequenceChecker.h"

namespace {

// An effect "extends beyond" the sequence when its end is after the end of
// the last frame the renderer draws.
bool ExtendsBeyond(int effectEndMS, int sequenceDurationMS, int frequency) {
    return effectEndMS > SequenceChecker::GetSequenceEndMS(sequenceDurationMS, frequency);
}

} // namespace

TEST_CASE("GetSequenceEndMS rounds the duration to the nearest frame boundary", "[diagnostics][checksequence]") {
    // 25 ms frames (40 fps). A media length of 235.299s must be treated as
    // 235.300s, otherwise every effect ending on the final frame is flagged.
    CHECK(SequenceChecker::GetSequenceEndMS(235299, 40) == 235300);
    CHECK(SequenceChecker::GetSequenceEndMS(235300, 40) == 235300);
    CHECK(SequenceChecker::GetSequenceEndMS(230000, 40) == 230000);
    CHECK(SequenceChecker::GetSequenceEndMS(111438, 40) == 111450);
    CHECK(SequenceChecker::GetSequenceEndMS(227493, 40) == 227500);

    // The renderer uses the nearest frame boundary, not a strict ceil:
    // 235.311s is closer to 235.300s than to 235.325s.
    CHECK(SequenceChecker::GetSequenceEndMS(235311, 40) == 235300);

    // 50 ms frames (20 fps).
    CHECK(SequenceChecker::GetSequenceEndMS(227493, 20) == 227500);
    CHECK(SequenceChecker::GetSequenceEndMS(230000, 20) == 230000);
}

TEST_CASE("GetSequenceEndMS keeps the duration when the frequency is unusable", "[diagnostics][checksequence]") {
    CHECK(SequenceChecker::GetSequenceEndMS(235299, 0) == 235299);
    CHECK(SequenceChecker::GetSequenceEndMS(235299, -40) == 235299);
}

TEST_CASE("Effects ending on the last frame are not past the sequence end", "[diagnostics][checksequence]") {
    // The exact cases that produced false positives: duration 235.299s and
    // effects snapped to the 235.300s frame boundary.
    CHECK_FALSE(ExtendsBeyond(235300, 235299, 40));

    // 225.085s rounds to 225.075s at 25 ms, so an effect at 225.100s is one
    // frame past and must still be reported.
    CHECK(ExtendsBeyond(225100, 225085, 40));

    // Genuine overruns of one or more frames stay reported.
    CHECK(ExtendsBeyond(235325, 235299, 40));
    CHECK(ExtendsBeyond(230025, 230000, 40));
    CHECK(ExtendsBeyond(230050, 230000, 40));
    CHECK_FALSE(ExtendsBeyond(230000, 230000, 40));
}
