#include <catch2/catch_test_macros.hpp>

#include "lyrics/LyricBreakdown.h"
#include "lyrics/PhonemeDictionary.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/UndoManager.h"
#include "support/TempDir.h"

#include <cstdlib>
#include <string>
#include <tuple>
#include <vector>

namespace {

struct Fixture {
    HeadlessRenderContext ctx;
    EffectLayer* words = nullptr;
    EffectLayer* phonemes = nullptr;
    Fixture() {
        auto& se = ctx.GetSequenceElements();
        se.SetViewsManager(&ctx._sequenceViewManager);
        auto* t = se.AddElement("Lyrics", "timing", true, false, true, false, false);
        words = t->AddEffectLayer();
        phonemes = t->AddEffectLayer();
    }
    UndoManager& Undo() { return ctx.GetSequenceElements().get_undo_mgr(); }
};

using Mark = std::tuple<int, int, std::string>;

std::vector<Mark> Marks(EffectLayer* l) {
    std::vector<Mark> out;
    for (int i = 0; i < l->GetEffectCount(); ++i) {
        Effect* e = l->GetEffect(i);
        out.emplace_back(e->GetStartTimeMS(), e->GetEndTimeMS(), e->GetEffectName());
    }
    return out;
}

struct Dict {
    TempDir dir;
    PhonemeDictionary d;
    Dict() {
        dir.Write("standard_dictionary",
                  "HELLO HH AH0 L OW1\n"
                  "MOM M AA1 M\n"
                  "A AH0\n"
                  "SKY S K AY1\n"
                  "BIG B IH1 G\n"
                  "WORLD W ER1 L D\n");
        dir.Write("extended_dictionary", "");
        dir.Write("user_dictionary", "");
        dir.Write("phoneme_mapping",
                  ". AI\n. E\n. O\n. etc\n. MBP\n"
                  "HH etc\nAH0 AI\nL L\nOW1 O\nM MBP\nAA1 AI\nS etc\nK etc\nAY1 AI\nB MBP\nIH1 E\nG etc\nW O\nER1 E\nD etc\n");
        d.LoadDictionaries({ dir.path.string() });
    }
};

}

TEST_CASE("BreakdownPhrase splits a phrase evenly into words", "[impexp][lyrics][phrase]") {
    Fixture f;
    BreakdownPhrase(f.words, 0, 3000, "Hello big world", 20, f.Undo());
    CHECK(Marks(f.words) == std::vector<Mark>{ { 0, 1000, "Hello" }, { 1000, 2000, "big" }, { 2000, 3000, "world" } });
}

TEST_CASE("BreakdownPhrase rounds word boundaries to the frame grid", "[impexp][lyrics][phrase]") {
    Fixture f;
    BreakdownPhrase(f.words, 0, 1000, "one two three", 20, f.Undo());
    CHECK(Marks(f.words) == std::vector<Mark>{ { 0, 350, "one" }, { 350, 650, "two" }, { 650, 1000, "three" } });
}

TEST_CASE("BreakdownPhrase honours a non zero start and other frame rates", "[impexp][lyrics][phrase]") {
    Fixture f;
    BreakdownPhrase(f.words, 1000, 1400, "a b", 40, f.Undo());
    CHECK(Marks(f.words) == std::vector<Mark>{ { 1000, 1200, "a" }, { 1200, 1400, "b" } });
}

TEST_CASE("BreakdownPhrase treats punctuation and repeated separators as delimiters", "[impexp][lyrics][phrase]") {
    Fixture f;
    BreakdownPhrase(f.words, 0, 4000, "  Hello,,  world!\t(again)-ok ", 20, f.Undo());
    auto m = Marks(f.words);
    REQUIRE(m.size() == 4);
    CHECK(std::get<2>(m[0]) == "Hello");
    CHECK(std::get<2>(m[1]) == "world");
    CHECK(std::get<2>(m[2]) == "again");
    CHECK(std::get<2>(m[3]) == "ok");
}

TEST_CASE("BreakdownPhrase keeps apostrophes and non ascii text inside words", "[impexp][lyrics][phrase]") {
    Fixture f;
    BreakdownPhrase(f.words, 0, 2000, "don't caf\xc3\xa9", 20, f.Undo());
    auto m = Marks(f.words);
    REQUIRE(m.size() == 2);
    CHECK(std::get<2>(m[0]) == "don't");
    CHECK(std::get<2>(m[1]) == "caf\xc3\xa9");
}

TEST_CASE("BreakdownPhrase with one word fills the whole range", "[impexp][lyrics][phrase]") {
    Fixture f;
    BreakdownPhrase(f.words, 200, 900, "alone", 20, f.Undo());
    CHECK(Marks(f.words) == std::vector<Mark>{ { 200, 900, "alone" } });
}

TEST_CASE("BreakdownPhrase of empty or delimiter only text adds nothing", "[impexp][lyrics][phrase]") {
    Fixture f;
    for (const char* s : { "", " ", "   \t  ", ",.;:-_!?", "{}[]()<>+=|", " , . " }) {
        BreakdownPhrase(f.words, 0, 1000, s, 20, f.Undo());
    }
    CHECK(f.words->GetEffectCount() == 0);
    CHECK_FALSE(f.Undo().CanUndo());
}

TEST_CASE("BreakdownPhrase words are contiguous and end exactly at the end time", "[impexp][lyrics][phrase]") {
    Fixture f;
    BreakdownPhrase(f.words, 130, 7777, "the quick brown fox jumps over the lazy dog again and again", 20, f.Undo());
    auto m = Marks(f.words);
    REQUIRE(m.size() == 12);
    CHECK(std::get<0>(m.front()) == 130);
    CHECK(std::get<1>(m.back()) == 7777);
    for (size_t i = 1; i < m.size(); ++i) {
        CHECK(std::get<0>(m[i]) == std::get<1>(m[i - 1]));
        CHECK(std::get<1>(m[i]) >= std::get<0>(m[i]));
    }
}

TEST_CASE("BreakdownPhrase with more words than frames does not overrun the range", "[impexp][lyrics][phrase]") {
    Fixture f;
    BreakdownPhrase(f.words, 0, 100, "a b c d e f g h i j k l", 20, f.Undo());
    for (const auto& m : Marks(f.words)) {
        CHECK(std::get<1>(m) <= 100);
        CHECK(std::get<0>(m) <= std::get<1>(m));
    }
}

TEST_CASE("BreakdownPhrase handles huge and zero length ranges without crashing", "[impexp][lyrics][phrase]") {
    Fixture f;
    BreakdownPhrase(f.words, 0, 2000000000, "big range", 20, f.Undo());
    BreakdownPhrase(f.phonemes, 1000, 1000, "zero", 20, f.Undo());
    SUCCEED();
}

TEST_CASE("BreakdownPhrase with start after end must not dereference a rejected effect", "[impexp][lyrics][phrase]") {
    Fixture f;
    BreakdownPhrase(f.words, 5000, 1000, "backwards phrase", 20, f.Undo());
    SUCCEED();
}

TEST_CASE("BreakdownPhrase records each word for undo", "[impexp][lyrics][phrase]") {
    Fixture f;
    f.Undo().CreateUndoStep();
    BreakdownPhrase(f.words, 0, 3000, "one two three", 20, f.Undo());
    REQUIRE(f.words->GetEffectCount() == 3);
    CHECK(f.Undo().CanUndo());
    f.Undo().UndoLastStep();
    CHECK(f.words->GetEffectCount() == 0);
}

TEST_CASE("BreakdownPhrase copes with very long and binary phrases", "[impexp][lyrics][phrase]") {
    Fixture f;
    std::string big;
    for (int i = 0; i < 2000; ++i) {
        big += "w" + std::to_string(i) + " ";
    }
    BreakdownPhrase(f.words, 0, 200000, big, 20, f.Undo());
    CHECK(f.words->GetEffectCount() == 2000);
    BreakdownPhrase(f.phonemes, 0, 1000, std::string("\xff\xfe\x00\x80 \xc3\x28", 7), 20, f.Undo());
    SUCCEED();
}

TEST_CASE("BreakdownWord spreads phonemes with short ones at 50 ms", "[impexp][lyrics][word]") {
    Fixture f;
    Dict dict;
    BreakdownWord(f.phonemes, 0, 800, "hello", 20, dict.d, f.Undo());
    CHECK(Marks(f.phonemes) == std::vector<Mark>{ { 0, 50, "etc" }, { 50, 300, "AI" }, { 300, 550, "L" }, { 550, 800, "O" } });
}

TEST_CASE("BreakdownWord is case and punctuation insensitive", "[impexp][lyrics][word]") {
    Fixture f;
    Dict dict;
    BreakdownWord(f.phonemes, 0, 800, "HeLLo!", 20, dict.d, f.Undo());
    CHECK(f.phonemes->GetEffectCount() == 4);
}

TEST_CASE("BreakdownWord MBP phonemes are short like etc", "[impexp][lyrics][word]") {
    Fixture f;
    Dict dict;
    BreakdownWord(f.phonemes, 0, 700, "mom", 20, dict.d, f.Undo());
    CHECK(Marks(f.phonemes) == std::vector<Mark>{ { 0, 50, "MBP" }, { 50, 650, "AI" }, { 650, 700, "MBP" } });
}

TEST_CASE("BreakdownWord with a single phoneme fills the range", "[impexp][lyrics][word]") {
    Fixture f;
    Dict dict;
    BreakdownWord(f.phonemes, 100, 600, "a", 20, dict.d, f.Undo());
    CHECK(Marks(f.phonemes) == std::vector<Mark>{ { 100, 600, "AI" } });
}

TEST_CASE("BreakdownWord of unknown or empty words adds nothing", "[impexp][lyrics][word]") {
    Fixture f;
    Dict dict;
    BreakdownWord(f.phonemes, 0, 1000, "zzzzzz", 20, dict.d, f.Undo());
    BreakdownWord(f.phonemes, 0, 1000, "", 20, dict.d, f.Undo());
    BreakdownWord(f.phonemes, 0, 1000, "!!!", 20, dict.d, f.Undo());
    CHECK(f.phonemes->GetEffectCount() == 0);
    CHECK_FALSE(f.Undo().CanUndo());
}

TEST_CASE("BreakdownWord with an empty dictionary adds nothing", "[impexp][lyrics][word]") {
    Fixture f;
    PhonemeDictionary empty;
    BreakdownWord(f.phonemes, 0, 1000, "hello", 20, empty, f.Undo());
    CHECK(f.phonemes->GetEffectCount() == 0);
}

TEST_CASE("BreakdownWord phonemes are contiguous, non empty and within the range", "[impexp][lyrics][word]") {
    Fixture f;
    Dict dict;
    for (int len : { 100, 150, 400, 777, 1000, 5000 }) {
        f.phonemes->DeleteAllEffects();
        BreakdownWord(f.phonemes, 40, 40 + len, "world", 20, dict.d, f.Undo());
        auto m = Marks(f.phonemes);
        INFO("length " << len);
        REQUIRE_FALSE(m.empty());
        CHECK(std::get<0>(m.front()) == 40);
        for (size_t i = 0; i < m.size(); ++i) {
            CHECK(std::get<1>(m[i]) > std::get<0>(m[i]));
            CHECK(std::get<1>(m[i]) <= 40 + len);
            if (i > 0) {
                CHECK(std::get<0>(m[i]) == std::get<1>(m[i - 1]));
            }
        }
    }
}

TEST_CASE("BreakdownWord skips phonemes shorter than one frame", "[impexp][lyrics][word]") {
    Fixture f;
    Dict dict;
    BreakdownWord(f.phonemes, 0, 40, "hello", 20, dict.d, f.Undo());
    for (const auto& m : Marks(f.phonemes)) {
        CHECK(std::get<1>(m) > std::get<0>(m));
        CHECK(std::get<1>(m) <= 40);
    }
}

TEST_CASE("BreakdownWord records each phoneme for undo", "[impexp][lyrics][word]") {
    Fixture f;
    Dict dict;
    f.Undo().CreateUndoStep();
    BreakdownWord(f.phonemes, 0, 800, "hello", 20, dict.d, f.Undo());
    REQUIRE(f.phonemes->GetEffectCount() == 4);
    f.Undo().UndoLastStep();
    CHECK(f.phonemes->GetEffectCount() == 0);
}

TEST_CASE("BreakdownWord with a reversed or zero range does not crash", "[impexp][lyrics][word]") {
    Fixture f;
    Dict dict;
    BreakdownWord(f.phonemes, 1000, 0, "hello", 20, dict.d, f.Undo());
    BreakdownWord(f.phonemes, 500, 500, "hello", 20, dict.d, f.Undo());
    BreakdownWord(f.phonemes, 0, 2000000000, "hello", 20, dict.d, f.Undo());
    SUCCEED();
}

TEST_CASE("A phrase broken into words then phonemes keeps timing nested", "[impexp][lyrics][pipeline]") {
    Fixture f;
    Dict dict;
    BreakdownPhrase(f.words, 0, 2000, "hello world", 20, f.Undo());
    REQUIRE(f.words->GetEffectCount() == 2);
    for (int i = 0; i < f.words->GetEffectCount(); ++i) {
        Effect* w = f.words->GetEffect(i);
        BreakdownWord(f.phonemes, w->GetStartTimeMS(), w->GetEndTimeMS(), w->GetEffectName(), 20, dict.d, f.Undo());
    }
    auto ph = Marks(f.phonemes);
    REQUIRE(ph.size() == 8);
    CHECK(std::get<0>(ph.front()) == 0);
    CHECK(std::get<1>(ph.back()) == 2000);
    for (size_t i = 1; i < ph.size(); ++i) {
        CHECK(std::get<0>(ph[i]) == std::get<1>(ph[i - 1]));
    }
    CHECK(std::get<1>(ph[3]) == 1000);
}

TEST_CASE("Words missing from the dictionary leave a gap in the phoneme layer", "[impexp][lyrics][pipeline]") {
    Fixture f;
    Dict dict;
    BreakdownPhrase(f.words, 0, 3000, "hello mystery world", 20, f.Undo());
    for (int i = 0; i < f.words->GetEffectCount(); ++i) {
        Effect* w = f.words->GetEffect(i);
        BreakdownWord(f.phonemes, w->GetStartTimeMS(), w->GetEndTimeMS(), w->GetEffectName(), 20, dict.d, f.Undo());
    }
    for (const auto& m : Marks(f.phonemes)) {
        CHECK((std::get<1>(m) <= 1000 || std::get<0>(m) >= 2000));
    }
}

TEST_CASE("Phoneme dictionary built in memory drives a breakdown", "[impexp][lyrics][pipeline]") {
    Fixture f;
    PhonemeDictionary d;
    TempDir dir;
    dir.Write("standard_dictionary", "");
    dir.Write("extended_dictionary", "");
    dir.Write("user_dictionary", "");
    dir.Write("phoneme_mapping", "K etc\nAE1 AI\nT etc\n");
    d.LoadDictionaries({ dir.path.string() });
    d.InsertPhoneme({ "CAT", "K", "AE1", "T" });
    BreakdownWord(f.phonemes, 0, 400, "cat", 20, d, f.Undo());
    CHECK(Marks(f.phonemes) == std::vector<Mark>{ { 0, 50, "etc" }, { 50, 350, "AI" }, { 350, 400, "etc" } });
}
