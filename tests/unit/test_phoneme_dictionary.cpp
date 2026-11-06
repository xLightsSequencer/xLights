#include <catch2/catch_test_macros.hpp>

#include <filesystem>

#include "lyrics/PhonemeDictionary.h"
#include "support/TempDir.h"
#include "utils/AppCallbacks.h"

namespace fs = std::filesystem;

namespace {
struct CaptureErrors {
    std::vector<std::string> messages;
    CaptureErrors() {
        AppCallbacks::SetDisplayMessageCallback([this](AppCallbacks::DisplayMessageLevel level, const std::string& m) {
            if (level == AppCallbacks::DisplayMessageLevel::Error) {
                messages.push_back(m);
            }
        });
    }
    ~CaptureErrors() { AppCallbacks::SetDisplayMessageCallback(nullptr); }
};

void WriteStandardFiles(const TempDir& d) {
    d.Write("standard_dictionary",
            "## comment\r\n"
            ";; another comment\r\n"
            "HELLO HH AH0 L OW1\r\n"
            "WORLD W ER1 L D\r\n"
            "\r\n");
    d.Write("extended_dictionary", "ZZZ Z IY1\n");
    d.Write("user_dictionary", "HELLO HH EY1\n");
    d.Write("phoneme_mapping",
            "# comment\n"
            ". AI\n"
            ". E\n"
            "HH etc\n"
            "AH0 AI\n"
            "L L\n"
            "OW1 O\n"
            "W WQ\n"
            "ER1 E\n"
            "EY1 E\n"
            "IY1 E\n");
}
}

TEST_CASE("InsertSpacesAfterPunctuation", "[phoneme]") {
    auto f = [](std::string s) {
        PhonemeDictionary::InsertSpacesAfterPunctuation(s);
        return s;
    };
    CHECK(f("a,b.c!d?e;f") == "a, b. c! d? e; f");
    CHECK(f("already, spaced. ok") == "already, spaced. ok");
    CHECK(f("trailing.") == "trailing.");
    CHECK(f("") == "");
}

TEST_CASE("PhonemeDictionary insert, query and remove", "[phoneme]") {
    PhonemeDictionary d;
    CHECK_FALSE(d.ContainsPhoneme("CAT"));
    d.InsertPhoneme({});
    CHECK(d.GetPhonemeList().empty());

    d.InsertPhoneme({ "CAT", "K", "AE1", "T" });
    d.InsertPhoneme({ "DOG", "D", "AO1", "G" });
    CHECK(d.ContainsPhoneme("CAT"));
    CHECK(d.GetPhoneme("CAT") == std::vector<std::string>{ "CAT", "K", "AE1", "T" });
    CHECK(d.GetPhoneme("missing").empty());
    CHECK(d.GetPhonemeList() == std::vector<std::string>{ "CAT", "DOG" });

    d.InsertPhoneme({ "CAT", "K", "AE2", "T" });
    CHECK(d.GetPhoneme("CAT")[2] == "AE2");

    d.RemovePhoneme("CAT");
    CHECK_FALSE(d.ContainsPhoneme("CAT"));
    CHECK(d.ContainsPhoneme("DOG"));
}

TEST_CASE("PhonemeDictionary loads dictionaries and mapping", "[phoneme]") {
    TempDir dir;
    WriteStandardFiles(dir);

    PhonemeDictionary d;
    CaptureErrors errors;
    d.LoadDictionaries({ "", (dir.path / "does_not_exist").string(), dir.path.string() });
    CHECK(errors.messages.empty());

    CHECK(d.ContainsPhoneme("WORLD"));
    CHECK(d.ContainsPhoneme("ZZZ"));
    CHECK_FALSE(d.ContainsPhoneme("## comment"));
    CHECK(d.ContainsPhonemeMap("L"));
    CHECK_FALSE(d.ContainsPhonemeMap("."));

    SECTION("user dictionary overrides the standard one") {
        CHECK(d.GetPhoneme("HELLO") == std::vector<std::string>{ "HELLO", "HH", "EY1" });
    }
    SECTION("mapping lines with '.' are the phoneme list") {
        std::vector<std::string> out;
        d.BreakdownWord("world", out);
        CHECK(out == std::vector<std::string>{ "WQ", "E", "L", "" });
    }
}

TEST_CASE("PhonemeDictionary only loads once", "[phoneme]") {
    TempDir dir;
    WriteStandardFiles(dir);
    PhonemeDictionary d;
    d.LoadDictionaries({ dir.path.string() });
    d.RemovePhoneme("WORLD");
    d.LoadDictionaries({ dir.path.string() });
    CHECK_FALSE(d.ContainsPhoneme("WORLD"));
}

TEST_CASE("PhonemeDictionary reports missing files", "[phoneme]") {
    TempDir dir;
    CaptureErrors errors;
    PhonemeDictionary d;
    d.LoadDictionaries({ dir.path.string() });
    CHECK_FALSE(errors.messages.empty());
    CHECK(d.GetPhonemeList().empty());
}

TEST_CASE("PhonemeDictionary progress callback reaches 100", "[phoneme]") {
    TempDir dir;
    std::string big;
    for (int i = 0; i < 2500; ++i) {
        big += "WORD" + std::to_string(i) + " W ER1 D\n";
    }
    dir.Write("big", big);
    PhonemeDictionary d;
    std::vector<int> seen;
    d.LoadDictionary("big", { dir.path.string() }, [&](int pct) { seen.push_back(pct); });
    REQUIRE_FALSE(seen.empty());
    CHECK(seen.back() == 100);
    for (int p : seen) {
        CHECK(p >= 0);
        CHECK(p <= 100);
    }
    CHECK(d.ContainsPhoneme("WORD2499"));
}

TEST_CASE("BreakdownWord maps phonemes", "[phoneme]") {
    PhonemeDictionary d;
    TempDir dir;
    dir.Write("standard_dictionary", "HELLO HH AH0 L OW1\nSTUFF HH HH AH0 XX\n");
    dir.Write("extended_dictionary", "");
    dir.Write("user_dictionary", "");
    dir.Write("phoneme_mapping", ". AI\nHH etc\nAH0 AI\nL L\nOW1 O\n");
    CaptureErrors errors;
    d.LoadDictionaries({ dir.path.string() });
    REQUIRE(errors.messages.empty());

    std::vector<std::string> out{ "stale" };
    SECTION("case-insensitive and punctuation stripped") {
        d.BreakdownWord("Hel,lo!", out);
        CHECK(out == std::vector<std::string>{ "etc", "AI", "L", "O" });
    }
    SECTION("unknown words give nothing and clear the output") {
        d.BreakdownWord("nonsense", out);
        CHECK(out.empty());
    }
    SECTION("entries with no pronunciation give nothing") {
        d.InsertPhoneme({ "LONE" });
        d.BreakdownWord("lone", out);
        CHECK(out.empty());
    }
    SECTION("consecutive etc collapses; an unmapped phoneme becomes empty") {
        d.BreakdownWord("stuff", out);
        CHECK(out == std::vector<std::string>{ "etc", "AI", "" });
    }
}
