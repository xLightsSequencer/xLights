#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <atomic>
#include <string_view>
#include <thread>

#include <spdlog/sinks/callback_sink.h>

#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/RenderCache.h"
#include "render/SequenceElements.h"
#include "utils/UtilClasses.h"

#include "test_render_common.h"


using namespace renderhelp;
namespace fs = std::filesystem;

namespace {

using Rgb = Rendered::Rgb;

// HeadlessRenderContext turns the render cache off when it builds its engine
// unless this variable is set.
struct CacheEnv {
#ifdef _WIN32
    CacheEnv() { _putenv_s("XL_HEADLESS_RENDERCACHE", "1"); }
    ~CacheEnv() { _putenv_s("XL_HEADLESS_RENDERCACHE", ""); }
#else
    CacheEnv() { setenv("XL_HEADLESS_RENDERCACHE", "1", 1); }
    ~CacheEnv() { unsetenv("XL_HEADLESS_RENDERCACHE"); }
#endif
};

// Flags the end of a render cache folder load, seen through the render logger.
struct LoadWatcher {
    struct State {
        std::atomic<bool> done{ false };
    };
    std::shared_ptr<State> watcher = std::make_shared<State>();
    std::shared_ptr<spdlog::sinks::callback_sink_mt> sink;
    spdlog::level::level_enum oldLevel = spdlog::level::info;

    LoadWatcher() {
        EnsureLoggers();
        auto st = watcher;
        sink = std::make_shared<spdlog::sinks::callback_sink_mt>([st](const spdlog::details::log_msg& m) {
            std::string_view text(m.payload.data(), m.payload.size());
            if (text.find("Cache contained") != std::string_view::npos) st->done = true;
        });
        auto logger = spdlog::get("render");
        oldLevel = logger->level();
        logger->set_level(spdlog::level::debug);
        logger->sinks().push_back(sink);
    }
    ~LoadWatcher() {
        auto logger = spdlog::get("render");
        auto& sinks = logger->sinks();
        sinks.erase(std::remove(sinks.begin(), sinks.end(), sink), sinks.end());
        logger->set_level(oldLevel);
    }
};

// A show whose render cache lives in <show>/RenderCache/seq_RENDER_CACHE.
struct CacheShow {
    CacheEnv env;
    TempDir dir;
    FixFileReset reset;
    LoadWatcher loaded;
    std::unique_ptr<HeadlessRenderContext> ctx;
    Rendered result;
    std::string xsqPath;

    explicit CacheShow(const ShowSpec& spec, const std::string& mode = "Enabled") {
        EnsureLoggers();
        dir.Write("xlights_networks.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb version=\"2\"/>\n");
        dir.Write("xlights_rgbeffects.xml",
                  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb>\n<models>\n" + spec.models + "\n</models>\n<modelGroups>\n" + spec.groups + "\n</modelGroups>\n</xrgb>\n");
        xsqPath = dir.Write("seq.xsq", BuildXsq(spec));
        ctx = std::make_unique<HeadlessRenderContext>();
        REQUIRE(ctx->LoadShowFolder(dir.path.string()));
        REQUIRE(ctx->OpenSequence(xsqPath));
        ctx->_renderCache.Enable(mode);
        ctx->_renderCache.SetSequence(dir.path.string(), "seq");
        WaitForLoad();
    }
    ~CacheShow() {
        // Keep the files on disk like the desktop does on close; the effects then
        // drop their items without deleting the files.
        ctx->_renderCache.CleanupCache(&ctx->_sequenceElements);
        ctx->_renderCache.SetSequence("", "");
    }

    std::string Folder() const { return (dir.path / "RenderCache" / "seq_RENDER_CACHE").string(); }

    bool Render() {
        if (!ctx->RenderAndWait(60000)) return false;
        Capture();
        return true;
    }
    void Capture() {
        auto& sd = ctx->_seqData;
        result.channels = sd.NumChannels();
        result.frames = sd.NumFrames();
        result.data.assign(result.frames, std::vector<uint8_t>(result.channels));
        for (unsigned int f = 0; f < result.frames; ++f) {
            for (unsigned int c = 0; c < result.channels; ++c) result.data[f][c] = sd[f][c];
        }
    }

    // Write a new version of the sequence and reopen it, then reattach the cache
    // so it reloads what is on disk.
    void Reopen(const ShowSpec& spec) {
        ctx->_renderCache.CleanupCache(&ctx->_sequenceElements);
        dir.Write("seq.xsq", BuildXsq(spec));
        REQUIRE(ctx->OpenSequence(xsqPath));
        ctx->_renderCache.SetSequence(dir.path.string(), "seq");
        WaitForLoad();
    }

    // SetSequence loads the folder on a thread that holds the load mutex while it runs.
    // The thread logs "Cache contained N files." as its last step, so watch the
    // render logger for it rather than guessing how long the thread takes to start.
    void WaitForLoad() {
        auto start = std::chrono::steady_clock::now();
        while (!loaded.watcher->done.load() && std::chrono::steady_clock::now() - start < std::chrono::seconds(20)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
        REQUIRE(loaded.watcher->done.load());
        std::unique_lock<std::mutex> lock(ctx->_renderCache.GetLoadMutex());
        loaded.watcher->done = false;
    }
};

std::vector<std::string> CacheFiles(const std::string& folder) {
    std::vector<std::string> out;
    std::error_code ec;
    if (!fs::exists(folder, ec)) return out;
    for (auto& e : fs::directory_iterator(folder, ec)) {
        if (e.path().extension() == ".cache") out.push_back(e.path().filename().string());
    }
    std::sort(out.begin(), out.end());
    return out;
}

std::string ReadAll(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

void WriteAll(const std::string& path, const std::string& body) {
    std::ofstream(path, std::ios::binary | std::ios::trunc) << body;
}

// The blur slider makes any effect cacheable (RenderableEffect::SupportsRenderCache).
ShowSpec RedOn(const std::string& color = "#FF0000", int start = 0) {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", start, 1000, Pal(color), "B_SLIDER_Blur=1"));
    return s;
}

// Replace every opaque red RGBA pixel after the header with opaque green.
std::string RecolourFrames(std::string body) {
    auto hdr = body.find("RC_HEADEREND");
    REQUIRE(hdr != std::string::npos);
    const std::string red("\xFF\x00\x00\xFF", 4);
    const std::string green("\x00\xFF\x00\xFF", 4);
    size_t pos = hdr;
    int n = 0;
    while ((pos = body.find(red, pos)) != std::string::npos) {
        body.replace(pos, 4, green);
        pos += 4;
        ++n;
    }
    REQUIRE(n > 0);
    return body;
}

} // namespace

TEST_CASE("Render cache saves one file per cacheable effect", "[render2][rendercache]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"), "B_SLIDER_Blur=1,T_CHOICE_LayerMethod=Additive"));
    s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#0000FF")));
    CacheShow show(s);
    REQUIRE(show.Render());
    CHECK(show.result.rgb(5, 0) == Rgb{ 255, 0, 255 });
    // only the blurred top effect is cacheable; files use 1-based layer numbers
    CHECK(CacheFiles(show.Folder()) == std::vector<std::string>{ "On_M_1_0.cache" });

    std::string body = ReadAll(show.Folder() + "/On_M_1_0.cache");
    CHECK(body.find(std::string("Effect\0On\0", 10)) != std::string::npos);
    CHECK(body.find(std::string("Frames\0" "20\0", 10)) != std::string::npos);
    CHECK(body.find(std::string("Models\0" "1\0", 9)) != std::string::npos);
    CHECK(body.find("RC_HEADEREND") != std::string::npos);
}

TEST_CASE("Render cache hit reproduces the rendered frames", "[render2][rendercache]") {
    CacheShow show(RedOn());
    REQUIRE(show.Render());
    auto first = show.result.data;
    CHECK(show.result.rgb(0, 0) == Rgb{ 255, 0, 0 });
    REQUIRE(show.Render());
    CHECK(show.result.data == first);
}

TEST_CASE("Render cache frames loaded from disk are used instead of rendering", "[render2][rendercache]") {
    CacheShow show(RedOn());
    REQUIRE(show.Render());
    const std::string file = show.Folder() + "/On_M_1_0.cache";
    REQUIRE(fs::exists(file));

    show.ctx->_renderCache.CleanupCache(&show.ctx->_sequenceElements);
    show.ctx->_renderCache.SetSequence("", "");
    REQUIRE(fs::exists(file));
    WriteAll(file, RecolourFrames(ReadAll(file)));

    show.Reopen(RedOn());
    REQUIRE(show.Render());
    // the effect still asks for red, so green proves the frames came from the cache
    for (unsigned f : { 0u, 10u, 19u }) CHECK(show.result.rgb(f, 0) == Rgb{ 0, 255, 0 });
}

TEST_CASE("Render cache misses when an effect setting changes", "[render2][rendercache]") {
    CacheShow show(RedOn());
    REQUIRE(show.Render());
    const std::string file = show.Folder() + "/On_M_1_0.cache";
    show.ctx->_renderCache.CleanupCache(&show.ctx->_sequenceElements);
    show.ctx->_renderCache.SetSequence("", "");
    WriteAll(file, RecolourFrames(ReadAll(file)));

    SECTION("palette") {
        show.Reopen(RedOn("#0000FF"));
        REQUIRE(show.Render());
        CHECK(show.result.rgb(5, 0) == Rgb{ 0, 0, 255 });
    }
    SECTION("start time") {
        show.Reopen(RedOn("#FF0000", 100));
        REQUIRE(show.Render());
        CHECK(show.result.rgb(0, 0) == Rgb{ 0, 0, 0 });
        CHECK(show.result.rgb(5, 0) == Rgb{ 255, 0, 0 });
    }
    SECTION("extra setting") {
        ShowSpec s;
        s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"), "B_SLIDER_Blur=1,T_SLIDER_Brightness=100"));
        show.Reopen(s);
        REQUIRE(show.Render());
        CHECK(show.result.rgb(5, 0) == Rgb{ 255, 0, 0 });
    }
}

TEST_CASE("Render cache cleanup deletes files no effect matches", "[render2][rendercache]") {
    CacheShow show(RedOn());
    REQUIRE(show.Render());
    show.ctx->_renderCache.CleanupCache(&show.ctx->_sequenceElements);
    show.ctx->_renderCache.SetSequence("", "");
    REQUIRE(CacheFiles(show.Folder()).size() == 1);

    // The effect moves, so the old file is loaded but matches nothing.
    show.Reopen(RedOn("#FF0000", 100));
    REQUIRE(show.Render());
    CHECK(CacheFiles(show.Folder()) == (std::vector<std::string>{ "On_M_1_0.cache", "On_M_1_100.cache" }));
    show.ctx->_renderCache.CleanupCache(&show.ctx->_sequenceElements);
    CHECK(CacheFiles(show.Folder()) == std::vector<std::string>{ "On_M_1_100.cache" });
}

TEST_CASE("Render cache purge with delete removes the files", "[render2][rendercache]") {
    CacheShow show(RedOn());
    REQUIRE(show.Render());
    REQUIRE(CacheFiles(show.Folder()).size() == 1);
    show.ctx->_renderCache.Purge(&show.ctx->_sequenceElements, true);
    CHECK(CacheFiles(show.Folder()).empty());
}

TEST_CASE("Render cache purge without delete keeps the files", "[render2][rendercache]") {
    CacheShow show(RedOn());
    REQUIRE(show.Render());
    show.ctx->_renderCache.Purge(&show.ctx->_sequenceElements, false);
    CHECK(CacheFiles(show.Folder()).size() == 1);
}

TEST_CASE("Disabling the render cache removes the sequence cache folder", "[render2][rendercache]") {
    CacheShow show(RedOn());
    REQUIRE(show.Render());
    show.ctx->_renderCache.CleanupCache(&show.ctx->_sequenceElements);
    show.ctx->_renderCache.SetSequence("", "");
    REQUIRE(fs::exists(show.Folder()));
    show.ctx->_renderCache.Enable("Disabled");
    CHECK_FALSE(show.ctx->_renderCache.IsEnabled());
    show.ctx->_renderCache.SetSequence(show.dir.path.string(), "seq");
    CHECK_FALSE(fs::exists(show.Folder()));
}

TEST_CASE("Locked only render cache skips unlocked effects", "[render2][rendercache]") {
    CacheShow show(RedOn(), "Locked Only");
    REQUIRE(show.Render());
    CHECK(show.result.rgb(5, 0) == Rgb{ 255, 0, 0 });
    CHECK(CacheFiles(show.Folder()).empty());
}

TEST_CASE("Locked only render cache stores locked effects", "[render2][rendercache]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"), "B_SLIDER_Blur=1,X_Effect_Locked=True"));
    CacheShow show(s, "Locked Effects Only");
    REQUIRE(show.Render());
    CHECK(CacheFiles(show.Folder()).size() == 1);
}

TEST_CASE("Effects that are not cacheable write no cache files", "[render2][rendercache]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000")));
    s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#00FF00"), "B_SLIDER_Blur=1,T_CHECKBOX_Canvas=1"));
    s.effects.push_back(Eff("M", 2, "On", 0, 1000, Pal("#0000FF"), "B_SLIDER_Blur=1,B_CHECKBOX_OverlayBkg=1"));
    CacheShow show(s);
    REQUIRE(show.Render());
    CHECK(CacheFiles(show.Folder()).empty());
}

TEST_CASE("Render cache eligibility follows the cache mode and effect flags", "[render2][rendercache]") {
    EnsureLoggers();
    RenderCache rc;
    SettingsMap plain;
    SettingsMap locked;
    locked["Effect_Locked"] = "True";
    SettingsMap canvas;
    canvas["CHECKBOX_Canvas"] = "1";
    SettingsMap overlay;
    overlay["CHECKBOX_OverlayBkg"] = "1";

    CHECK(rc.IsEnabled());
    CHECK(rc.IsEffectOkForCaching(plain));
    CHECK_FALSE(rc.IsEffectOkForCaching(canvas));
    CHECK_FALSE(rc.IsEffectOkForCaching(overlay));

    rc.Enable("Locked Only");
    CHECK(rc.IsEnabled());
    CHECK_FALSE(rc.IsEffectOkForCaching(plain));
    CHECK(rc.IsEffectOkForCaching(locked));

    rc.Enable("Disabled");
    CHECK_FALSE(rc.IsEnabled());
    CHECK_FALSE(rc.IsEffectOkForCaching(locked));
    CHECK(rc.GetItem(nullptr, plain, nullptr) == nullptr);

    rc.Enable("something else");
    CHECK(rc.IsEnabled());
    // no sequence folder yet
    CHECK(rc.GetCacheFolder().empty());
    CHECK(rc.GetItem(nullptr, plain, nullptr) == nullptr);
}

TEST_CASE("Damaged render cache files are rejected", "[render2][rendercache]") {
    EnsureLoggers();
    TempDir dir;
    RenderCache rc;
    auto load = [&](const std::string& name, const std::string& body) {
        std::string p = dir.Write(name, body);
        RenderCacheItem item(&rc, p);
        return item.IsPurged();
    };
    using std::string;
    const string z("\0", 1);
    auto kv = [&](const string& k, const string& v) { return k + z + v + z; };
    const string head = kv("Effect", "On") + kv("Element", "M") + kv("EffectLayer", "0") + kv("StartMS", "0") +
                        kv("EndMS", "100") + kv("Frames", "2");

    CHECK(load("noheaderend_x.cache", head + kv("Models", "1")));
    CHECK(load("negmodels_x.cache", head + kv("Models", "-1") + "RC_HEADEREND" + z));
    CHECK(load("trunclist_x.cache", head + kv("Models", "1") + "RC_HEADEREND" + z + "M" + z));
    CHECK(load("zerosize_x.cache", head + kv("Models", "1") + "RC_HEADEREND" + z + "M" + z + "2" + z + "0" + z));
    CHECK(load("huge_x.cache", head + kv("Models", "1") + "RC_HEADEREND" + z + "M" + z + "999999" + z + "16" + z));
    // two frames of 16 bytes promised, only 8 present
    CHECK(load("short_x.cache", head + kv("Models", "1") + "RC_HEADEREND" + z + "M" + z + "2" + z + "16" + z + string(8, 'x')));
    // well formed: two 16 byte frames
    CHECK_FALSE(load("On_M_1_0.cache", head + kv("Models", "1") + "RC_HEADEREND" + z + "M" + z + "2" + z + "16" + z + string(32, 'x')));
    // missing file: not purged, just empty
    RenderCacheItem missing(&rc, dir.File("absent.cache"));
    CHECK_FALSE(missing.IsPurged());
    CHECK(missing.EffectName() == "absent");
}

TEST_CASE("Render cache item name is the effect name prefix of the file", "[render2][rendercache]") {
    EnsureLoggers();
    TempDir dir;
    RenderCache rc;
    RenderCacheItem item(&rc, dir.File("Butterfly_Some Model_2_1500.cache"));
    CHECK(item.EffectName() == "Butterfly");
    CHECK(item.Description() == dir.File("Butterfly_Some Model_2_1500.cache"));
    // a purged item matches nothing and yields no frames
    item.PurgeFrames();
    CHECK(item.IsPurged());
    CHECK_FALSE(item.GetFrame(nullptr));
    CHECK_FALSE(item.IsDone(nullptr));
}

TEST_CASE("Render cache maximum size evicts the oldest files", "[render2][rendercache]") {
    EnsureLoggers();
    TempDir dir;
    fs::create_directories(dir.path / "RenderCache" / "a_RENDER_CACHE");
    const auto now = fs::file_time_type::clock::now();
    std::vector<fs::path> files;
    for (int i = 0; i < 4; ++i) {
        fs::path p = dir.path / "RenderCache" / "a_RENDER_CACHE" / ("On_M_1_" + std::to_string(i) + ".cache");
        WriteAll(p.string(), std::string(900 * 1024, 'x'));
        fs::last_write_time(p, now - std::chrono::hours(10 - i));
        files.push_back(p);
    }
    // not a cache file, so never evicted but still counted
    WriteAll((dir.path / "RenderCache" / "notes.txt").string(), "x");

    RenderCache rc;
    rc.SetRenderCacheFolder(dir.path.string());
    rc.SetMaximumSizeMB(0);
    for (auto& f : files) CHECK(fs::exists(f));

    // 3.5MB on disk with a 2MB cap: dropping the oldest leaves 2.6MB, which is
    // 2 whole MB, so eviction stops there.
    rc.SetMaximumSizeMB(2);
    CHECK_FALSE(fs::exists(files[0]));
    CHECK(fs::exists(files[1]));
    CHECK(fs::exists(files[2]));
    CHECK(fs::exists(files[3]));
    CHECK(fs::exists(dir.path / "RenderCache" / "notes.txt"));
}

TEST_CASE("Render cache maximum size drops any file over half the cap", "[render2][rendercache]") {
    EnsureLoggers();
    TempDir dir;
    fs::create_directories(dir.path / "RenderCache" / "b_RENDER_CACHE");
    fs::path big = dir.path / "RenderCache" / "b_RENDER_CACHE" / "On_M_1_0.cache";
    fs::path small = dir.path / "RenderCache" / "b_RENDER_CACHE" / "On_M_1_1.cache";
    WriteAll(big.string(), std::string(3 * 1024 * 1024, 'x'));
    WriteAll(small.string(), std::string(1024, 'x'));
    RenderCache rc;
    rc.SetMaximumSizeMB(4);
    rc.SetRenderCacheFolder(dir.path.string());
    // under the cap: nothing removed
    CHECK(fs::exists(big));
    rc.SetMaximumSizeMB(3);
    CHECK_FALSE(fs::exists(big));
    CHECK(fs::exists(small));
}

TEST_CASE("Render cache folder that does not exist is left alone", "[render2][rendercache]") {
    EnsureLoggers();
    TempDir dir;
    RenderCache rc;
    rc.SetRenderCacheFolder((dir.path / "missing").string());
    rc.SetMaximumSizeMB(1);
    CHECK_FALSE(fs::exists(dir.path / "missing"));
}
