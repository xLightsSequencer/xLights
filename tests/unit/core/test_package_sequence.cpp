#include <catch2/catch_test_macros.hpp>

#include <list>

#include "models/ModelManager.h"
#include "models/ViewObjectManager.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/SequenceMedia.h"
#include "render/SequencePackage.h"
#include "support/TempDir.h"
#include "utils/FileUtils.h"
#include "utils/ZipUtils.h"
#include "utils/xlImage.h"

#include <minizip/zip.h>
#include <pugixml.hpp>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace {

void SetEnv(const char* name, const std::string& value, bool unset) {
#ifdef _WIN32
    _putenv_s(name, unset ? "" : value.c_str());
#else
    if (unset) {
        unsetenv(name);
    } else {
        setenv(name, value.c_str(), 1);
    }
#endif
}

struct ScopedTmp {
    TempDir root;
    fs::path tmp;
    struct Saved {
        std::string value;
        bool had = false;
    };
    std::vector<std::pair<std::string, Saved>> saved;
    ScopedTmp() {
        tmp = root.path / "tmp";
        fs::create_directories(tmp);
        for (const char* n : { "TMPDIR", "TMP", "TEMP" }) {
            Saved s;
            if (const char* v = std::getenv(n)) {
                s.had = true;
                s.value = v;
            }
            saved.emplace_back(n, s);
            SetEnv(n, tmp.string(), false);
        }
    }
    ~ScopedTmp() {
        for (auto& [n, s] : saved) {
            SetEnv(n.c_str(), s.value, !s.had);
        }
    }
    std::vector<std::string> Leftovers() const {
        std::vector<std::string> out;
        std::error_code ec;
        for (auto& e : fs::directory_iterator(tmp, ec)) {
            auto n = e.path().filename().string();
            if (n.rfind("xl_unit_", 0) != 0) {
                out.push_back(n);
            }
        }
        return out;
    }
};

struct FixFileGuard {
    explicit FixFileGuard(const std::string& dir) { FileUtils::SetFixFileShowDir(dir); }
    ~FixFileGuard() { FileUtils::SetFixFileShowDir(""); }
};

bool BuildZip(const std::string& path, const std::vector<std::pair<std::string, std::string>>& entries) {
    zipFile zf = zipOpen(path.c_str(), APPEND_STATUS_CREATE);
    if (zf == nullptr) {
        return false;
    }
    for (const auto& e : entries) {
        zip_fileinfo zi{};
        if (zipOpenNewFileInZip(zf, e.first.c_str(), &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK) {
            zipClose(zf, nullptr);
            return false;
        }
        if (!e.second.empty()) {
            zipWriteInFileInZip(zf, e.second.data(), static_cast<unsigned>(e.second.size()));
        }
        zipCloseFileInZip(zf);
    }
    zipClose(zf, nullptr);
    return true;
}

std::string PngBytes(int seed) {
    xlImage img(3, 3);
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            img.SetRGBA(x, y, static_cast<uint8_t>(seed + x * 40), static_cast<uint8_t>(y * 60), 77, 255);
        }
    }
    std::vector<uint8_t> out;
    img.SaveAsPNG(out);
    return std::string(out.begin(), out.end());
}

std::string ReadFile(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

std::string XsqDoc(const std::string& media, const std::string& effectSettings = "", const std::string& extraHead = "") {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xsequence BaseChannel=\"0\" ChanCtrlBasic=\"0\" ChanCtrlColor=\"0\" FixedPointTiming=\"1\" ModelBlending=\"true\">"
           "<head><version>2024.01</version><mediaFile>" + media + "</mediaFile><sequenceTiming>50 ms</sequenceTiming><sequenceType>Media</sequenceType><sequenceDuration>10.000</sequenceDuration>" +
           extraHead + "</head><EffectDB><Effect>" + effectSettings + "</Effect></EffectDB></xsequence>";
}

constexpr const char* kRgbEffects = "<xrgb><settings><ShowGUID>secret-guid</ShowGUID></settings></xrgb>";

struct PackFixture {
    HeadlessRenderContext ctx;
    TempDir show;
    TempDir external;
    SequenceMedia media;
    PackFixture() {
        ctx.GetSequenceElements().SetViewsManager(&ctx._sequenceViewManager);
        ctx.showDirectory = show.path.string();
        show.Write("xlights_rgbeffects.xml", kRgbEffects);
    }
    bool Pack(const fs::path& out, const std::string& xsq, const std::string& audio, const std::vector<std::string>& alt = {}, const std::vector<std::string>& extra = {},
              SequencePackOptions opts = {}, std::vector<std::string>* warnings = nullptr, ProgressCallback cb = nullptr) {
        return SequencePackage::Pack(out, show.path.string(), xsq, audio, alt, extra, media, ctx.AllModels, ctx.AllObjects, ctx.GetSequenceElements(), opts, warnings, cb);
    }
};

std::set<std::string> Entries(const fs::path& zip) {
    auto v = ZipUtils::ListEntries(zip.string());
    return std::set<std::string>(v.begin(), v.end());
}

}

TEST_CASE("Pack writes the sequence, rgbeffects, networks and audio", "[impexp][package][pack]") {
    ScopedTmp tmp;
    PackFixture f;
    f.show.Write("xlights_networks.xml", "<Networks/>");
    auto audio = f.external.Write("song.mp3", "FAKEAUDIO");
    auto xsq = f.show.Write("seq.xsq", XsqDoc(audio));
    auto out = f.show.path / "out.xsqz";
    std::vector<std::string> warnings;
    REQUIRE(f.Pack(out, xsq, audio, {}, {}, {}, &warnings));
    CHECK(warnings.empty());
    CHECK(Entries(out) == std::set<std::string>{ "seq.xsq", "xlights_rgbeffects.xml", "xlights_networks.xml", "song.mp3" });
    CHECK(*ZipUtils::ReadEntry(out.string(), "song.mp3") == "FAKEAUDIO");
}

TEST_CASE("Pack rewrites external paths inside the sequence to package relative ones", "[impexp][package][pack]") {
    ScopedTmp tmp;
    PackFixture f;
    auto audio = f.external.Write("song.mp3", "A");
    auto img = f.external.Write("pic.png", PngBytes(10));
    f.media.RegisterImage(img);
    auto xsq = f.show.Write("seq.xsq", XsqDoc(audio, "E_TEXTCTRL_Pictures_Filename=" + img + ",E_CHECKBOX_X=1"));
    auto out = f.show.path / "out.xsqz";
    REQUIRE(f.Pack(out, xsq, audio));
    CHECK(Entries(out).count("Images/pic.png") == 1);
    auto packed = ZipUtils::ReadEntry(out.string(), "seq.xsq");
    REQUIRE(packed.has_value());
    CHECK(packed->find("E_TEXTCTRL_Pictures_Filename=Images/pic.png,E_CHECKBOX_X=1") != std::string::npos);
    CHECK(packed->find("<mediaFile>song.mp3</mediaFile>") != std::string::npos);
    CHECK(packed->find(f.external.path.string()) == std::string::npos);
}

TEST_CASE("Pack keeps show relative paths for media inside the show folder", "[impexp][package][pack]") {
    ScopedTmp tmp;
    PackFixture f;
    FixFileGuard guard(f.show.path.string());
    fs::create_directories(f.show.path / "pics");
    auto img = f.show.Write("pics/inside.png", PngBytes(20));
    f.media.RegisterImage(img);
    auto xsq = f.show.Write("seq.xsq", XsqDoc("", "E_TEXTCTRL_Pictures_Filename=" + img));
    auto out = f.show.path / "out.xsqz";
    REQUIRE(f.Pack(out, xsq, ""));
    CHECK(Entries(out).count("pics/inside.png") == 1);
    auto packed = ZipUtils::ReadEntry(out.string(), "seq.xsq");
    REQUIRE(packed.has_value());
    CHECK(packed->find("E_TEXTCTRL_Pictures_Filename=pics/inside.png") != std::string::npos);
}

TEST_CASE("Pack excludeAudio drops primary and alternate tracks", "[impexp][package][pack]") {
    ScopedTmp tmp;
    PackFixture f;
    auto audio = f.external.Write("song.mp3", "A");
    auto alt = f.external.Write("drums.mp3", "B");
    auto xsq = f.show.Write("seq.xsq", XsqDoc(audio));
    auto out1 = f.show.path / "with.xsqz";
    REQUIRE(f.Pack(out1, xsq, audio, { alt }));
    CHECK(Entries(out1).count("song.mp3") == 1);
    CHECK(Entries(out1).count("drums.mp3") == 1);
    auto out2 = f.show.path / "without.xsqz";
    SequencePackOptions opts;
    opts.excludeAudio = true;
    REQUIRE(f.Pack(out2, xsq, audio, { alt }, {}, opts));
    CHECK(Entries(out2) == std::set<std::string>{ "seq.xsq", "xlights_rgbeffects.xml" });
}

TEST_CASE("Pack with missing audio and media still succeeds and warns", "[impexp][package][pack]") {
    ScopedTmp tmp;
    PackFixture f;
    f.media.RegisterImage(f.external.File("ghost.png"));
    auto xsq = f.show.Write("seq.xsq", XsqDoc(f.external.File("ghost.mp3")));
    auto out = f.show.path / "out.xsqz";
    std::vector<std::string> warnings;
    REQUIRE(f.Pack(out, xsq, f.external.File("ghost.mp3"), {}, { f.external.File("extra.dat") }, {}, &warnings));
    CHECK(warnings.size() == 3);
    CHECK(Entries(out) == std::set<std::string>{ "seq.xsq", "xlights_rgbeffects.xml" });
}

TEST_CASE("Pack disambiguates media with the same file name", "[impexp][package][pack]") {
    ScopedTmp tmp;
    PackFixture f;
    TempDir other;
    auto a = f.external.Write("same.png", PngBytes(1));
    auto b = other.Write("same.png", PngBytes(2));
    f.media.RegisterImage(a);
    f.media.RegisterImage(b);
    auto xsq = f.show.Write("seq.xsq", XsqDoc("", "A=" + a + ",B=" + b));
    auto out = f.show.path / "out.xsqz";
    REQUIRE(f.Pack(out, xsq, ""));
    auto e = Entries(out);
    CHECK(e.count("Images/same.png") == 1);
    CHECK(e.count("Images/dup2/same.png") == 1);
    auto packed = *ZipUtils::ReadEntry(out.string(), "seq.xsq");
    CHECK(packed.find("A=Images/") != std::string::npos);
    CHECK(packed.find("B=Images/") != std::string::npos);
    CHECK(packed.find("A=Images/same.png") != packed.find("B=Images/same.png"));
}

TEST_CASE("Pack shares one entry between effects that use the same file", "[impexp][package][pack]") {
    ScopedTmp tmp;
    PackFixture f;
    auto img = f.external.Write("one.png", PngBytes(5));
    f.media.RegisterImage(img);
    f.media.RegisterImage(img);
    auto xsq = f.show.Write("seq.xsq", XsqDoc("", "A=" + img + ",B=" + img));
    auto out = f.show.path / "out.xsqz";
    REQUIRE(f.Pack(out, xsq, ""));
    auto packed = *ZipUtils::ReadEntry(out.string(), "seq.xsq");
    CHECK(packed.find("A=Images/one.png,B=Images/one.png") != std::string::npos);
    CHECK(ZipUtils::ListEntries(out.string()).size() == 3);
}

TEST_CASE("Pack removes private information from the package", "[impexp][package][pack]") {
    ScopedTmp tmp;
    PackFixture f;
    auto xsq = f.show.Write("seq.xsq", XsqDoc("", "", "<importedFrom>/home/me/secret.xsq</importedFrom><importedFrom>/home/me/other.xsq</importedFrom>"));
    auto out = f.show.path / "out.xsqz";
    REQUIRE(f.Pack(out, xsq, ""));
    auto rgb = *ZipUtils::ReadEntry(out.string(), "xlights_rgbeffects.xml");
    CHECK(rgb.find("ShowGUID") == std::string::npos);
    auto packed = *ZipUtils::ReadEntry(out.string(), "seq.xsq");
    CHECK(packed.find("importedFrom") == std::string::npos);
    CHECK(packed.find("secret.xsq") == std::string::npos);
}

TEST_CASE("Pack does not modify the source files", "[impexp][package][pack]") {
    ScopedTmp tmp;
    PackFixture f;
    auto audio = f.external.Write("song.mp3", "A");
    auto xsq = f.show.Write("seq.xsq", XsqDoc(audio));
    auto before = ReadFile(xsq);
    auto beforeRgb = ReadFile(f.show.path / "xlights_rgbeffects.xml");
    REQUIRE(f.Pack(f.show.path / "out.xsqz", xsq, audio));
    CHECK(ReadFile(xsq) == before);
    CHECK(ReadFile(f.show.path / "xlights_rgbeffects.xml") == beforeRgb);
}

TEST_CASE("Pack fails cleanly when the sequence or rgbeffects file is missing", "[impexp][package][pack]") {
    ScopedTmp tmp;
    PackFixture f;
    auto out = f.show.path / "out.xsqz";
    CHECK_FALSE(f.Pack(out, f.show.File("missing.xsq"), ""));
    CHECK_FALSE(fs::exists(out));
    auto xsq = f.show.Write("seq.xsq", XsqDoc(""));
    fs::remove(f.show.path / "xlights_rgbeffects.xml");
    CHECK_FALSE(f.Pack(out, xsq, ""));
    CHECK_FALSE(fs::exists(out));
}

TEST_CASE("Pack fails on an unparsable sequence and leaves no output", "[impexp][package][pack]") {
    ScopedTmp tmp;
    PackFixture f;
    auto xsq = f.show.Write("seq.xsq", "<xsequence><head>");
    auto out = f.show.path / "out.xsqz";
    CHECK_FALSE(f.Pack(out, xsq, ""));
    CHECK_FALSE(fs::exists(out));
}

TEST_CASE("Pack replaces an existing package and leaves no scratch files", "[impexp][package][pack]") {
    ScopedTmp tmp;
    PackFixture f;
    auto out = f.show.path / "out.xsqz";
    {
        std::ofstream(out, std::ios::binary) << "old contents";
    }
    auto xsq = f.show.Write("seq.xsq", XsqDoc(""));
    REQUIRE(f.Pack(out, xsq, ""));
    CHECK(Entries(out).count("seq.xsq") == 1);
    CHECK(tmp.Leftovers().empty());
}

TEST_CASE("Pack reports progress and ends at 100", "[impexp][package][pack]") {
    ScopedTmp tmp;
    PackFixture f;
    auto xsq = f.show.Write("seq.xsq", XsqDoc(""));
    std::vector<int> seen;
    REQUIRE(f.Pack(f.show.path / "out.xsqz", xsq, "", {}, {}, {}, nullptr, [&](int pct) {
        seen.push_back(pct);
        return true;
    }));
    REQUIRE_FALSE(seen.empty());
    CHECK(seen.front() == 0);
    CHECK(seen.back() == 100);
    CHECK(std::is_sorted(seen.begin(), seen.end()));
}

TEST_CASE("Pack includes caller supplied extra files at their show relative path", "[impexp][package][pack]") {
    ScopedTmp tmp;
    PackFixture f;
    FixFileGuard guard(f.show.path.string());
    fs::create_directories(f.show.path / "data");
    auto inside = f.show.Write("data/layer.iseq", "ISEQ");
    auto outside = f.external.Write("house.jpg", "JPG");
    auto xsq = f.show.Write("seq.xsq", XsqDoc(""));
    auto out = f.show.path / "out.xsqz";
    REQUIRE(f.Pack(out, xsq, "", {}, { inside, outside, "" }));
    auto e = Entries(out);
    CHECK(e.count("data/layer.iseq") == 1);
    CHECK(e.count("house.jpg") == 1);
}

TEST_CASE("Pack skips embedded images and keeps them out of the zip", "[impexp][package][pack]") {
    ScopedTmp tmp;
    PackFixture f;
    xlImage img(2, 2);
    img.SetRGBA(0, 0, 1, 2, 3, 255);
    f.media.AddEmbeddedImage("emb.png", img);
    auto xsq = f.show.Write("seq.xsq", XsqDoc(""));
    auto out = f.show.path / "out.xsqz";
    std::vector<std::string> warnings;
    REQUIRE(f.Pack(out, xsq, "", {}, {}, {}, &warnings));
    CHECK(warnings.empty());
    CHECK(Entries(out) == std::set<std::string>{ "seq.xsq", "xlights_rgbeffects.xml" });
}

TEST_CASE("Pack output can be extracted again with identical media bytes", "[impexp][package][roundtrip]") {
    ScopedTmp tmp;
    PackFixture f;
    auto audio = f.external.Write("song.mp3", std::string("\x00\x01\x02audio\xff", 9));
    auto pngData = PngBytes(33);
    auto img = f.external.Write("pic.png", pngData);
    f.media.RegisterImage(img);
    auto xsq = f.show.Write("seq.xsq", XsqDoc(audio, "E_TEXTCTRL_Pictures_Filename=" + img));
    auto out = f.show.path / "out.xsqz";
    REQUIRE(f.Pack(out, xsq, audio));

    TempDir dest;
    SequencePackage pkg(out, dest.path.string(), (dest.path / "new.xsq").string(), &f.ctx.AllModels);
    pkg.Extract();
    CHECK(pkg.IsPkg());
    CHECK(pkg.IsValid());
    CHECK(pkg.HasRGBEffects());
    CHECK(pkg.HasMedia());
    CHECK(pkg.GetXsqFile().filename() == "seq.xsq");
    CHECK_FALSE(pkg.GetImportOptions()->GetDir(MediaTargetDir::IMAGES_DIR).empty());
    CHECK(pkg.GetTempShowFolder() == pkg.GetXsqFile().parent_path().string());

    auto copied = pkg.FindAndCopyAudio(dest.path / "audio");
    REQUIRE_FALSE(copied.empty());
    CHECK(copied.filename() == "song.mp3");
    CHECK(ReadFile(copied) == std::string("\x00\x01\x02audio\xff", 9));
}

TEST_CASE("Extracted package cleans up its temp folder on destruction", "[impexp][package][extract]") {
    ScopedTmp tmp;
    TempDir d;
    auto zip = d.File("p.xsqz");
    REQUIRE(BuildZip(zip, { { "a.xsq", XsqDoc("") }, { "xlights_rgbeffects.xml", kRgbEffects } }));
    std::string tempDirUsed;
    {
        SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
        pkg.Extract();
        REQUIRE(pkg.IsValid());
        tempDirUsed = pkg.GetTempDir();
        CHECK(fs::exists(tempDirUsed));
    }
    CHECK_FALSE(fs::exists(tempDirUsed));
}

TEST_CASE("Extracted package keeps its files when asked to", "[impexp][package][extract]") {
    ScopedTmp tmp;
    TempDir d;
    auto zip = d.File("p.xsqz");
    REQUIRE(BuildZip(zip, { { "a.xsq", XsqDoc("") }, { "xlights_rgbeffects.xml", kRgbEffects } }));
    std::string tempDirUsed;
    {
        SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
        pkg.SetLeaveFiles(true);
        pkg.Extract();
        tempDirUsed = pkg.GetTempDir();
    }
    CHECK(fs::exists(tempDirUsed));
}

TEST_CASE("Package entries cannot escape the extraction folder", "[impexp][package][extract]") {
    ScopedTmp tmp;
    TempDir d;
    auto zip = d.File("evil.xsqz");
    REQUIRE(BuildZip(zip, { { "a.xsq", XsqDoc("") }, { "xlights_rgbeffects.xml", kRgbEffects }, { "../escape1.txt", "x" }, { "sub/../../escape2.txt", "x" } }));
    {
        SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
        pkg.Extract();
        CHECK_FALSE(fs::exists(tmp.tmp / "escape1.txt"));
        CHECK_FALSE(fs::exists(tmp.tmp / "escape2.txt"));
        CHECK_FALSE(fs::exists(tmp.root.path / "escape2.txt"));
    }
}

TEST_CASE("Package extraction rejects absolute and backslash escapes but keeps nested paths", "[impexp][package][extract]") {
    ScopedTmp tmp;
    TempDir d;
    auto zip = d.File("evil2.xsqz");
    auto abs = (d.path / "abs_escape.txt").generic_string();
    REQUIRE(BuildZip(zip, { { "a.xsq", XsqDoc("") }, { "xlights_rgbeffects.xml", kRgbEffects }, { "..\\\\bs_escape.txt", "x" }, { abs, "x" }, { "C:/drive_escape.txt", "x" }, { "good/sub/../ok.png", "y" }, { "deep/er/pic.png", "z" } }));
    SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
    pkg.Extract();
    CHECK_FALSE(fs::exists(tmp.tmp / "bs_escape.txt"));
    CHECK_FALSE(fs::exists(d.path / "abs_escape.txt"));
    CHECK(pkg.IsValid());
    CHECK(pkg.HasMedia());
    CHECK(fs::exists(fs::path(pkg.GetTempDir()) / "deep" / "er" / "pic.png"));
    CHECK(fs::exists(fs::path(pkg.GetTempDir()) / "good" / "ok.png"));
}

TEST_CASE("Package extraction skips macOS resource fork folders", "[impexp][package][extract]") {
    ScopedTmp tmp;
    TempDir d;
    auto zip = d.File("p.xsqz");
    REQUIRE(BuildZip(zip, { { "a.xsq", XsqDoc("") }, { "xlights_rgbeffects.xml", kRgbEffects }, { "__MACOSX/._junk.png", "x" }, { "img.png", "y" } }));
    SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
    pkg.Extract();
    REQUIRE(pkg.IsValid());
    TempDir out;
    CHECK(pkg.FindAndCopyAudio(out.path).empty());
    CHECK(pkg.HasMedia());
    CHECK_FALSE(fs::exists(fs::path(pkg.GetTempDir()) / "__MACOSX"));
}

TEST_CASE("Package without rgbeffects or without a sequence is not valid", "[impexp][package][extract]") {
    ScopedTmp tmp;
    TempDir d;
    SECTION("no rgbeffects") {
        auto zip = d.File("a.xsqz");
        REQUIRE(BuildZip(zip, { { "a.xsq", XsqDoc("") } }));
        SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
        pkg.Extract();
        CHECK_FALSE(pkg.IsValid());
        CHECK_FALSE(pkg.HasRGBEffects());
    }
    SECTION("no sequence") {
        auto zip = d.File("b.xsqz");
        REQUIRE(BuildZip(zip, { { "xlights_rgbeffects.xml", kRgbEffects }, { "x.png", "x" } }));
        SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
        pkg.Extract();
        CHECK_FALSE(pkg.IsValid());
        CHECK(pkg.HasRGBEffects());
    }
    SECTION("unparsable rgbeffects") {
        auto zip = d.File("c.xsqz");
        REQUIRE(BuildZip(zip, { { "a.xsq", XsqDoc("") }, { "xlights_rgbeffects.xml", "<xrgb><settings>" } }));
        SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
        pkg.Extract();
        CHECK_FALSE(pkg.IsValid());
    }
}

TEST_CASE("Package extraction tolerates missing, empty and corrupt archives", "[impexp][package][extract]") {
    ScopedTmp tmp;
    TempDir d;
    {
        SequencePackage pkg(d.File("absent.xsqz"), d.path.string(), "t.xsq", nullptr);
        pkg.Extract();
        CHECK(pkg.IsPkg());
        CHECK_FALSE(pkg.IsValid());
        CHECK_FALSE(pkg.HasMedia());
    }
    {
        SequencePackage pkg(d.Write("junk.xsqz", "this is not a zip"), d.path.string(), "t.xsq", nullptr);
        pkg.Extract();
        CHECK_FALSE(pkg.IsValid());
    }
    {
        auto z = d.File("empty.xsqz");
        REQUIRE(BuildZip(z, {}));
        SequencePackage pkg(z, d.path.string(), "t.xsq", nullptr);
        pkg.Extract();
        CHECK_FALSE(pkg.IsValid());
    }
    {
        auto good = d.File("good.xsqz");
        REQUIRE(BuildZip(good, { { "a.xsq", XsqDoc("") }, { "xlights_rgbeffects.xml", kRgbEffects }, { "big.bin", std::string(200000, 'z') } }));
        auto bytes = ReadFile(good);
        auto cut = d.Write("cut.xsqz", bytes.substr(0, bytes.size() / 2));
        SequencePackage pkg(cut, d.path.string(), "t.xsq", nullptr);
        pkg.Extract();
        SUCCEED();
    }
}

TEST_CASE("Package extraction handles non ascii and nested entry names", "[impexp][package][extract]") {
    ScopedTmp tmp;
    TempDir d;
    auto zip = d.File("u.xsqz");
    REQUIRE(BuildZip(zip, { { "S\xc3\xa9q.xsq", XsqDoc("") }, { "xlights_rgbeffects.xml", kRgbEffects }, { "Im\xc3\xa1" "ges/\xe2\x98\x83.png", "snow" }, { "a/b/c/deep.png", "deep" }, { "caf\x82.png", "cp437" }, { "dir/", "" } }));
    SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
    pkg.Extract();
    CHECK(pkg.IsValid());
    CHECK(pkg.HasMedia());
}

TEST_CASE("Package extraction reports progress ending at 100", "[impexp][package][extract]") {
    ScopedTmp tmp;
    TempDir d;
    auto zip = d.File("p.xsqz");
    REQUIRE(BuildZip(zip, { { "a.xsq", XsqDoc("") }, { "xlights_rgbeffects.xml", kRgbEffects }, { "x.png", "x" } }));
    SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
    std::vector<int> seen;
    pkg.SetProgressCallback([&](int p) {
        seen.push_back(p);
        return false;
    });
    pkg.Extract();
    REQUIRE_FALSE(seen.empty());
    CHECK(seen.front() == 0);
    CHECK(seen.back() == 100);
}

TEST_CASE("Package default import folders depend on where the target sequence lives", "[impexp][package][extract]") {
    ScopedTmp tmp;
    TempDir show;
    TempDir d;
    auto zip = d.File("My Pack.xsqz");
    REQUIRE(BuildZip(zip, { { "Song.xsq", XsqDoc("") }, { "xlights_rgbeffects.xml", kRgbEffects } }));

    SECTION("new sequence uses ImportedMedia/<name>") {
        SequencePackage pkg(zip, show.path.string(), "new_sequence", nullptr);
        pkg.Extract();
        auto dir = pkg.GetImportOptions()->GetDir(MediaTargetDir::IMAGES_DIR);
        CHECK(dir == (show.path / "ImportedMedia" / "Song" / "Images").string());
        CHECK(pkg.GetImportOptions()->GetDir(MediaTargetDir::FACES_DIR) == (show.path / "DownloadedFaces").string());
        CHECK(pkg.GetImportOptions()->GetDir(MediaTargetDir::VIDEOS_DIR) == (show.path / "ImportedMedia" / "Song" / "Videos").string());
    }
    SECTION("saved sequence in a subfolder uses its own folder") {
        auto target = show.path / "sub" / "target.xsq";
        SequencePackage pkg(zip, show.path.string(), target.string(), nullptr);
        pkg.Extract();
        CHECK(pkg.GetImportOptions()->GetDir(MediaTargetDir::SHADERS_DIR) == (show.path / "sub" / "Shaders").string());
    }
}

TEST_CASE("Import options remember defaults and restore them", "[impexp][package][options]") {
    SeqPkgImportOptions o;
    CHECK(o.IsImportActive());
    o.SetImportActive(false);
    CHECK_FALSE(o.IsImportActive());
    CHECK(o.GetDir(MediaTargetDir::IMAGES_DIR).empty());
    o.SetDir(MediaTargetDir::IMAGES_DIR, "/a", true);
    o.SetDir(MediaTargetDir::VIDEOS_DIR, "/v", false);
    o.SetDir(MediaTargetDir::IMAGES_DIR, "/changed", false);
    CHECK(o.GetDir(MediaTargetDir::IMAGES_DIR) == "/changed");
    o.RestoreDefaults();
    CHECK(o.GetDir(MediaTargetDir::IMAGES_DIR) == "/a");
    CHECK(o.GetDir(MediaTargetDir::VIDEOS_DIR).empty());
}

TEST_CASE("Plain sequence import indexes media in the source show folder", "[impexp][package][plain]") {
    ScopedTmp tmp;
    TempDir src;
    TempDir show;
    src.Write("xlights_rgbeffects.xml", kRgbEffects);
    auto xsq = src.Write("seq.xsq", XsqDoc(""));
    fs::create_directories(src.path / "Media");
    src.Write("Media/clip.mp4", "VIDEO");
    src.Write("Media/dup.png", "FIRST");
    src.Write("backup.xbkp", "ignored");
    SequencePackage pkg(xsq, show.path.string(), (show.path / "new.xsq").string(), nullptr);
    CHECK_FALSE(pkg.IsPkg());
    CHECK(pkg.IsValid());
    pkg.FindRGBEffectsFile();
    CHECK(pkg.HasRGBEffects());
    CHECK(pkg.HasMedia());
    pkg.Extract();
    CHECK(pkg.GetTempDir().empty());
}

TEST_CASE("Plain sequence import without rgbeffects still indexes media", "[impexp][package][plain]") {
    TempDir src;
    TempDir show;
    auto xsq = src.Write("seq.xsq", XsqDoc(""));
    SequencePackage pkg(xsq, show.path.string(), "x", nullptr);
    pkg.FindRGBEffectsFile();
    CHECK_FALSE(pkg.HasRGBEffects());
    CHECK_FALSE(pkg.HasMedia());
    CHECK(pkg.IsValid());
}

TEST_CASE("Plain sequence import of a missing sequence is invalid", "[impexp][package][plain]") {
    TempDir src;
    SequencePackage pkg(src.File("absent.xsq"), src.path.string(), "x", nullptr);
    CHECK_FALSE(pkg.IsValid());
}

TEST_CASE("CopyMediaToTarget creates folders and never overwrites", "[impexp][package][copy]") {
    TempDir src;
    TempDir dst;
    auto original = src.Write("img.png", "NEW");
    SequencePackage pkg(src.File("x.xsq"), dst.path.string(), "x", nullptr);
    auto target = dst.path / "deep" / "er";
    auto copied = pkg.CopyMediaToTarget(target.string(), original);
    CHECK(copied == target / "img.png");
    CHECK(ReadFile(copied) == "NEW");
    std::ofstream(copied, std::ios::binary) << "LOCAL";
    auto again = pkg.CopyMediaToTarget(target.string(), original);
    CHECK(again == copied);
    CHECK(ReadFile(copied) == "LOCAL");
}

TEST_CASE("CopyMediaToTarget of a missing source copies nothing", "[impexp][package][copy]") {
    TempDir src;
    TempDir dst;
    SequencePackage pkg(src.File("x.xsq"), dst.path.string(), "x", nullptr);
    auto copied = pkg.CopyMediaToTarget(dst.path.string(), src.File("ghost.png"));
    CHECK_FALSE(fs::exists(copied));
}

TEST_CASE("Audio lookup prefers the file named in the sequence, then any audio extension", "[impexp][package][audio]") {
    ScopedTmp tmp;
    TempDir d;
    SECTION("named audio wins over other audio") {
        auto zip = d.File("p.xsqz");
        REQUIRE(BuildZip(zip, { { "a.xsq", XsqDoc("/old/place/wanted.wav") }, { "xlights_rgbeffects.xml", kRgbEffects }, { "aaa.mp3", "other" }, { "wanted.wav", "right" } }));
        SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
        pkg.Extract();
        TempDir out;
        auto p = pkg.FindAndCopyAudio(out.path);
        CHECK(p.filename() == "wanted.wav");
        CHECK(ReadFile(p) == "right");
    }
    SECTION("windows style path in the sequence") {
        auto zip = d.File("w.xsqz");
        REQUIRE(BuildZip(zip, { { "a.xsq", XsqDoc("C:\\Music\\win.mp3") }, { "xlights_rgbeffects.xml", kRgbEffects }, { "win.mp3", "w" } }));
        SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
        pkg.Extract();
        TempDir out;
        CHECK(pkg.FindAndCopyAudio(out.path).filename() == "win.mp3");
    }
    SECTION("falls back to an audio extension in any case") {
        auto zip = d.File("f.xsqz");
        REQUIRE(BuildZip(zip, { { "a.xsq", XsqDoc("missing.mp3") }, { "xlights_rgbeffects.xml", kRgbEffects }, { "pic.png", "p" }, { "Tune.FLAC", "t" } }));
        SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
        pkg.Extract();
        TempDir out;
        CHECK(pkg.FindAndCopyAudio(out.path).filename() == "Tune.FLAC");
    }
    SECTION("no audio at all") {
        auto zip = d.File("n.xsqz");
        REQUIRE(BuildZip(zip, { { "a.xsq", XsqDoc("") }, { "xlights_rgbeffects.xml", kRgbEffects }, { "pic.png", "p" } }));
        SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
        pkg.Extract();
        TempDir out;
        CHECK(pkg.FindAndCopyAudio(out.path).empty());
    }
}

TEST_CASE("Alternate audio tracks are copied or reported missing", "[impexp][package][audio]") {
    ScopedTmp tmp;
    TempDir d;
    auto zip = d.File("p.xsqz");
    auto head = "<altAudioTracks><track shortname=\"Drums\">/x/drums.mp3</track><track shortname=\"Gone\">/x/gone.mp3</track><track shortname=\"Blank\"></track></altAudioTracks>";
    REQUIRE(BuildZip(zip, { { "a.xsq", XsqDoc("", "", head) }, { "xlights_rgbeffects.xml", kRgbEffects }, { "drums.mp3", "D" } }));
    SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
    pkg.Extract();
    TempDir out;
    auto tracks = pkg.FindAndCopyAltAudioTracks(out.path);
    REQUIRE(tracks.size() == 3);
    CHECK(tracks[0].first == "Drums");
    CHECK(fs::path(tracks[0].second).filename() == "drums.mp3");
    CHECK(tracks[1] == std::make_pair(std::string("Gone"), std::string()));
    CHECK(tracks[2] == std::make_pair(std::string("Blank"), std::string()));
}

TEST_CASE("Alternate audio lookup without a sequence returns nothing", "[impexp][package][audio]") {
    TempDir d;
    SequencePackage pkg(d.File("none.xsqz"), d.path.string(), "t.xsq", nullptr);
    TempDir out;
    CHECK(pkg.FindAndCopyAltAudioTracks(out.path).empty());
    CHECK(pkg.FindAndCopyAudio(out.path).empty());
}

TEST_CASE("Repack writes the extracted tree back into a package", "[impexp][package][repack]") {
    ScopedTmp tmp;
    TempDir d;
    auto zip = d.File("p.xsqz");
    REQUIRE(BuildZip(zip, { { "a.xsq", XsqDoc("") }, { "xlights_rgbeffects.xml", kRgbEffects }, { "Images/pic.png", "PNG" }, { ".DS_Store", "junk" }, { "x.xsq.xbkp", "bk" } }));
    SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
    pkg.Extract();
    REQUIRE(pkg.IsValid());
    auto out = d.path / "repacked.xsqz";
    REQUIRE(pkg.Repack(out));
    CHECK(Entries(out) == std::set<std::string>{ "a.xsq", "xlights_rgbeffects.xml", "Images/pic.png" });
    CHECK(*ZipUtils::ReadEntry(out.string(), "Images/pic.png") == "PNG");
    CHECK(tmp.Leftovers().size() == 1);
}

TEST_CASE("Repack overwrites its own source package", "[impexp][package][repack]") {
    ScopedTmp tmp;
    TempDir d;
    auto zip = d.File("p.xsqz");
    REQUIRE(BuildZip(zip, { { "a.xsq", XsqDoc("") }, { "xlights_rgbeffects.xml", kRgbEffects } }));
    SequencePackage pkg(zip, d.path.string(), "t.xsq", nullptr);
    pkg.SetLeaveFiles(false);
    pkg.Extract();
    std::ofstream(fs::path(pkg.GetTempDir()) / "added.txt") << "new";
    REQUIRE(pkg.Repack(zip));
    CHECK(Entries(zip).count("added.txt") == 1);
}

TEST_CASE("Repack without a prior extract fails", "[impexp][package][repack]") {
    TempDir d;
    SequencePackage pkg(d.File("p.xsqz"), d.path.string(), "t.xsq", nullptr);
    CHECK_FALSE(pkg.Repack(d.path / "out.xsqz"));
}

TEST_CASE("RemoveConvertedCopies only deletes files the import copied", "[impexp][package][copy]") {
    TempDir src;
    TempDir dst;
    auto original = src.Write("v.avi", "V");
    auto preexisting = dst.Write("old.avi", "O");
    SequencePackage pkg(src.File("x.xsq"), dst.path.string(), "x", nullptr);
    auto copied = pkg.CopyMediaToTarget((dst.path / "Videos").string(), original);
    REQUIRE(fs::exists(copied));
    pkg.RemoveConvertedCopies({ { copied.string(), "converted.mp4" }, { preexisting, "old.mp4" }, { dst.File("nothing"), "x" } });
    CHECK_FALSE(fs::exists(copied));
    CHECK(fs::exists(preexisting));
}

TEST_CASE("Imported videos are tracked without duplicates and ignore blanks", "[impexp][package][copy]") {
    TempDir d;
    SequencePackage pkg(d.File("x.xsq"), d.path.string(), "x", nullptr);
    pkg.AddImportedVideo("a.mp4");
    pkg.AddImportedVideo("a.mp4");
    pkg.AddImportedVideo("");
    CHECK(pkg.GetImportedVideos().size() == 1);
    CHECK_FALSE(pkg.HasMissingMedia());
    CHECK(pkg.GetMissingMedia().empty());
}
