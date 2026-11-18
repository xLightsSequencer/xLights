#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <list>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#include "support/TempDir.h"
#include "utils/AppCallbacks.h"
#include "utils/ShowGuid.h"
#include "utils/SpecialOptions.h"
#include "utils/TempFileManager.h"
#include "utils/TraceLog.h"
#include "utils/xlExceptionDescribe.h"

namespace {

std::string Describe(const std::function<void()>& thrower) {
    try {
        thrower();
    } catch (...) {
        return DescribeCurrentException();
    }
    return "";
}

std::vector<std::string> Messages() {
    std::list<std::string> m;
    TraceLog::GetTraceMessages(m);
    return { m.begin(), m.end() };
}

struct DisplayHook {
    std::vector<std::pair<AppCallbacks::DisplayMessageLevel, std::string>> got;
    DisplayHook() {
        AppCallbacks::SetDisplayMessageCallback([this](AppCallbacks::DisplayMessageLevel l, const std::string& m) { got.push_back({ l, m }); });
    }
    ~DisplayHook() { AppCallbacks::SetDisplayMessageCallback(nullptr); }
};

} // namespace

TEST_CASE("DescribeCurrentException names C strings and std::string", "[misc][utils][exception]") {
    CHECK(Describe([] { throw "boom"; }) == "C-string exception: \"boom\".");
    CHECK(Describe([] { throw std::string("text"); }) == "std::string exception: \"text\".");
    CHECK(Describe([] { throw (const char*)nullptr; }) == "C-string exception: \"(null)\".");
}

TEST_CASE("DescribeCurrentException reports standard exceptions with type and message", "[misc][utils][exception]") {
    auto d = Describe([] { throw std::runtime_error("bad thing"); });
    CHECK(d.find("runtime_error") != std::string::npos);
    CHECK(d.find("bad thing") != std::string::npos);
    auto l = Describe([] { throw std::length_error("too long"); });
    CHECK(l.find("length_error") != std::string::npos);
    CHECK(l.find("too long") != std::string::npos);
}

TEST_CASE("DescribeCurrentException extracts the code from system errors", "[misc][utils][exception]") {
    auto d = Describe([] { throw std::system_error(std::make_error_code(std::errc::permission_denied), "denied"); });
    CHECK(d.find("System exception") != std::string::npos);
    CHECK(d.find("denied") != std::string::npos);
    CHECK(d.find("error " + std::to_string((int)std::errc::permission_denied)) != std::string::npos);
}

TEST_CASE("DescribeCurrentException extracts both paths from filesystem errors", "[misc][utils][exception]") {
    auto d = Describe([] {
        throw std::filesystem::filesystem_error("copy failed", "/from/here", "/to/there", std::make_error_code(std::errc::no_such_file_or_directory));
    });
    CHECK(d.find("Filesystem exception") != std::string::npos);
    CHECK(d.find("path1=\"/from/here\"") != std::string::npos);
    CHECK(d.find("path2=\"/to/there\"") != std::string::npos);
}

TEST_CASE("DescribeCurrentException describes non standard types", "[misc][utils][exception]") {
    auto d = Describe([] { throw 42; });
    struct Custom {};
#if defined(__GNUC__) || defined(__clang__)
    CHECK(d.find("non-standard type") != std::string::npos);
    CHECK(d.find("int") != std::string::npos);
    CHECK(Describe([] { throw Custom(); }).find("non-standard type") != std::string::npos);
#else
    // MSVC has no __cxa_current_exception_type, so the type can't be named.
    CHECK(d == "An unknown exception occurred.");
    CHECK(Describe([] { throw Custom(); }) == "An unknown exception occurred.");
#endif
}

TEST_CASE("TraceLog records messages in order for this thread", "[misc][utils][tracelog]") {
    TraceLog::ClearTraceMessages();
    TraceLog::AddTraceMessage("one");
    TraceLog::AddTraceMessage("two");
    TraceLog::AddTraceMessage("");
    std::vector<std::string> expect = { "one", "two", "" };
    CHECK(Messages() == expect);
    TraceLog::ClearTraceMessages();
    CHECK(Messages().empty());
}

TEST_CASE("TraceLog keeps only the last twenty messages", "[misc][utils][tracelog]") {
    TraceLog::ClearTraceMessages();
    for (int i = 0; i < 50; ++i) {
        TraceLog::AddTraceMessage(std::to_string(i));
    }
    auto m = Messages();
    REQUIRE(m.size() == 20);
    CHECK(m.front() == "30");
    CHECK(m.back() == "49");
    TraceLog::ClearTraceMessages();
}

TEST_CASE("TraceLog contexts are removed together on pop", "[misc][utils][tracelog]") {
    TraceLog::ClearTraceMessages();
    TraceLog::AddTraceMessage("outer");
    TraceLog::PushTraceContext();
    TraceLog::AddTraceMessage("inner1");
    TraceLog::AddTraceMessage("inner2");
    CHECK(Messages().size() == 4);
    TraceLog::PopTraceContext();
    std::vector<std::string> expect = { "outer" };
    CHECK(Messages() == expect);
    TraceLog::ClearTraceMessages();
}

TEST_CASE("TraceLog pop without a context empties the log", "[misc][utils][tracelog]") {
    TraceLog::ClearTraceMessages();
    TraceLog::AddTraceMessage("a");
    TraceLog::AddTraceMessage("b");
    TraceLog::PopTraceContext();
    CHECK(Messages().empty());
    TraceLog::PopTraceContext();
    TraceLog::ClearTraceMessages();
}

TEST_CASE("TraceLog keeps threads separate and labels them in the combined view", "[misc][utils][tracelog]") {
    TraceLog::ClearTraceMessages();
    TraceLog::AddTraceMessage("main-msg");
    std::vector<std::string> other;
    std::thread t([&] {
        TraceLog::AddTraceMessage("worker-msg");
        other = Messages();
    });
    t.join();
    CHECK(Messages() == std::vector<std::string>{ "main-msg" });
    CHECK(other == std::vector<std::string>{ "worker-msg" });

    std::list<std::string> all;
    TraceLog::GetAllTraceMessages(all);
    bool sawMain = false, sawWorker = false, sawHeader = false;
    for (auto const& s : all) {
        sawMain |= s == "  main-msg";
        sawWorker |= s == "  worker-msg";
        sawHeader |= s.rfind("thread ", 0) == 0;
    }
    CHECK(sawMain);
    CHECK(sawWorker);
    CHECK(sawHeader);
    TraceLog::ClearTraceMessages();
}

TEST_CASE("AppCallbacks display messages reach the registered callback", "[misc][utils][appcallbacks]") {
    DisplayHook h;
    AppCallbacks::DisplayError("e");
    AppCallbacks::DisplayWarning("w");
    AppCallbacks::DisplayInfo("i");
    DisplayError("unqualified");
    REQUIRE(h.got.size() == 4);
    CHECK(h.got[0].first == AppCallbacks::DisplayMessageLevel::Error);
    CHECK(h.got[0].second == "e");
    CHECK(h.got[1].first == AppCallbacks::DisplayMessageLevel::Warning);
    CHECK(h.got[2].first == AppCallbacks::DisplayMessageLevel::Info);
    CHECK(h.got[3].second == "unqualified");
}

TEST_CASE("AppCallbacks display messages are harmless without a callback", "[misc][utils][appcallbacks]") {
    AppCallbacks::SetDisplayMessageCallback(nullptr);
    AppCallbacks::DisplayError("nobody listens");
    AppCallbacks::DisplayWarning("");
    AppCallbacks::DisplayInfo(std::string(10000, 'x'));
    SUCCEED();
}

TEST_CASE("AppCallbacks callback can be replaced and removed", "[misc][utils][appcallbacks]") {
    int a = 0, b = 0;
    AppCallbacks::SetDisplayMessageCallback([&](AppCallbacks::DisplayMessageLevel, const std::string&) { ++a; });
    AppCallbacks::DisplayInfo("x");
    AppCallbacks::SetDisplayMessageCallback([&](AppCallbacks::DisplayMessageLevel, const std::string&) { ++b; });
    AppCallbacks::DisplayInfo("x");
    AppCallbacks::SetDisplayMessageCallback(nullptr);
    AppCallbacks::DisplayInfo("x");
    CHECK(a == 1);
    CHECK(b == 1);
}

TEST_CASE("AppCallbacks PostToMainThread runs inline by default and through a handler when set", "[misc][utils][appcallbacks]") {
    int ran = 0;
    AppCallbacks::SetPostToMainThread(nullptr);
    AppCallbacks::PostToMainThread([&] { ++ran; });
    CHECK(ran == 1);

    std::vector<std::function<void()>> queued;
    AppCallbacks::SetPostToMainThread([&](std::function<void()> f) { queued.push_back(std::move(f)); });
    AppCallbacks::PostToMainThread([&] { ++ran; });
    CHECK(ran == 1);
    REQUIRE(queued.size() == 1);
    queued[0]();
    CHECK(ran == 2);
    AppCallbacks::SetPostToMainThread(nullptr);
}

TEST_CASE("AppCallbacks thread crash handler hook is optional", "[misc][utils][appcallbacks]") {
    int n = 0;
    AppCallbacks::SetupThreadCrashHandler();
    AppCallbacks::SetSetupThreadCrashHandler([&] { ++n; });
    AppCallbacks::SetupThreadCrashHandler();
    CHECK(n == 1);
    AppCallbacks::SetSetupThreadCrashHandler(nullptr);
    AppCallbacks::SetupThreadCrashHandler();
    CHECK(n == 1);
}

TEST_CASE("ShowGuid reads the id from a show folder", "[misc][utils][showguid]") {
    TempDir t;
    t.Write("xlights_rgbeffects.xml", "<?xml version=\"1.0\"?><xrgb><settings><ShowGUID value=\"abc-123\"/></settings></xrgb>");
    CHECK(ShowGuid::ReadFromShowFolder(t.path.string()) == "abc-123");
}

TEST_CASE("ShowGuid also reads the alternate root element", "[misc][utils][showguid]") {
    TempDir t;
    t.Write("xlights_rgbeffects.xml", "<xlights><settings><Other value=\"1\"/><ShowGUID value=\"zzz\"/></settings></xlights>");
    CHECK(ShowGuid::ReadFromShowFolder(t.path.string()) == "zzz");
}

TEST_CASE("ShowGuid gives an empty id for anything unusable", "[misc][utils][showguid]") {
    TempDir t;
    CHECK(ShowGuid::ReadFromShowFolder("").empty());
    CHECK(ShowGuid::ReadFromShowFolder(t.File("missing")).empty());
    CHECK(ShowGuid::ReadFromShowFolder(t.path.string()).empty());

    t.Write("xlights_rgbeffects.xml", "");
    CHECK(ShowGuid::ReadFromShowFolder(t.path.string()).empty());
    t.Write("xlights_rgbeffects.xml", "<xrgb><settings></settings></xrgb>");
    CHECK(ShowGuid::ReadFromShowFolder(t.path.string()).empty());
    t.Write("xlights_rgbeffects.xml", "<xrgb><settings><ShowGUID/></settings></xrgb>");
    CHECK(ShowGuid::ReadFromShowFolder(t.path.string()).empty());
    t.Write("xlights_rgbeffects.xml", "<other><settings><ShowGUID value=\"x\"/></settings></other>");
    CHECK(ShowGuid::ReadFromShowFolder(t.path.string()).empty());
    t.Write("xlights_rgbeffects.xml", "<xrgb><ShowGUID value=\"x\"/></xrgb>");
    CHECK(ShowGuid::ReadFromShowFolder(t.path.string()).empty());
    t.Write("xlights_rgbeffects.xml", "<xrgb><settings><ShowGUID value=\"x\"");
    CHECK(ShowGuid::ReadFromShowFolder(t.path.string()).empty());
}

TEST_CASE("ShowGuid copies of a folder report the same id", "[misc][utils][showguid]") {
    TempDir a;
    TempDir b;
    const char* body = "<xrgb><settings><ShowGUID value=\"same\"/></settings></xrgb>";
    a.Write("xlights_rgbeffects.xml", body);
    b.Write("xlights_rgbeffects.xml", body);
    CHECK(ShowGuid::ReadFromShowFolder(a.path.string()) == ShowGuid::ReadFromShowFolder(b.path.string()));
}

TEST_CASE("TempFileManager removes registered files on cleanup", "[misc][utils][tempfiles]") {
    TempDir t;
    auto a = t.Write("a.tmp", "x");
    auto b = t.Write("b.tmp", "x");
    auto keep = t.Write("keep.txt", "x");
    TempFileManager m;
    m.AddTempFile(a);
    m.AddTempFile(b);
    m.AddTempFile(t.File("never_existed.tmp"));
    m.CleanupTempFiles();
    CHECK_FALSE(std::filesystem::exists(a));
    CHECK_FALSE(std::filesystem::exists(b));
    CHECK(std::filesystem::exists(keep));
    m.CleanupTempFiles();
}

TEST_CASE("TempFileManager cleans up when destroyed", "[misc][utils][tempfiles]") {
    TempDir t;
    auto a = t.Write("a.tmp", "x");
    {
        TempFileManager m;
        m.AddTempFile(a);
        CHECK(std::filesystem::exists(a));
    }
    CHECK_FALSE(std::filesystem::exists(a));
}

TEST_CASE("TempFileManager forgets a file after cleanup", "[misc][utils][tempfiles]") {
    TempDir t;
    auto a = t.Write("a.tmp", "x");
    TempFileManager m;
    m.AddTempFile(a);
    m.CleanupTempFiles();
    t.Write("a.tmp", "recreated");
    m.CleanupTempFiles();
    CHECK(std::filesystem::exists(a));
}

TEST_CASE("TempFileManager singleton is stable", "[misc][utils][tempfiles]") {
    CHECK(&TempFileManager::GetTempFileManager() == &TempFileManager::GetTempFileManager());
}

TEST_CASE("SpecialOptions stash accessors keep the last non empty value", "[misc][utils][specialoptions]") {
    auto prevShow = SpecialOptions::StashShowDir();
    auto prevExe = SpecialOptions::StashExeDir();
    SpecialOptions::StashShowDir("showA");
    CHECK(SpecialOptions::StashShowDir() == "showA");
    SpecialOptions::StashShowDir("");
    CHECK(SpecialOptions::StashShowDir() == "showA");
    SpecialOptions::StashExeDir("exeA");
    CHECK(SpecialOptions::StashExeDir() == "exeA");
    SpecialOptions::StashExeDir("");
    CHECK(SpecialOptions::StashExeDir() == "exeA");
    SpecialOptions::StashShowDir(prevShow.empty() ? "unused_show_dir" : prevShow);
    SpecialOptions::StashExeDir(prevExe.empty() ? "unused_exe_dir" : prevExe);
}

TEST_CASE("SpecialOptions reads options from the show folder file", "[misc][utils][specialoptions]") {
    auto prevShow = SpecialOptions::StashShowDir();
    auto prevExe = SpecialOptions::StashExeDir();
    {
        TempDir show;
        TempDir exe;
        show.Write("special.options", "<options><option name=\"Alpha\" value=\"1\"/><OPTION name=\"  Beta  \" value=\"two\"/><option name=\"\" value=\"skipped\"/><option name=\"NoValue\"/><other name=\"Gamma\" value=\"x\"/></options>");
        SpecialOptions::StashShowDir(show.path.string());
        SpecialOptions::StashExeDir(exe.path.string());
        SpecialOptions::GetOption("");
        CHECK(SpecialOptions::GetOption("Alpha", "d") == "1");
        CHECK(SpecialOptions::GetOption("Beta", "d") == "two");
        CHECK(SpecialOptions::GetOption("NoValue", "d").empty());
        CHECK(SpecialOptions::GetOption("Gamma", "d") == "d");
        CHECK(SpecialOptions::GetOption("alpha", "d") == "d");
        CHECK(SpecialOptions::GetOption("Missing") == "");
        CHECK(SpecialOptions::GetOption("Missing", "fallback") == "fallback");
    }
    SpecialOptions::StashShowDir(prevShow.empty() ? "unused_show_dir" : prevShow);
    SpecialOptions::StashExeDir(prevExe.empty() ? "unused_exe_dir" : prevExe);
    SpecialOptions::GetOption("");
}

TEST_CASE("SpecialOptions falls back to the executable folder", "[misc][utils][specialoptions]") {
    auto prevShow = SpecialOptions::StashShowDir();
    auto prevExe = SpecialOptions::StashExeDir();
    {
        TempDir show;
        TempDir exe;
        exe.Write("special.options", "<options><option name=\"FromExe\" value=\"yes\"/></options>");
        SpecialOptions::StashShowDir(show.path.string());
        SpecialOptions::StashExeDir(exe.path.string());
        SpecialOptions::GetOption("");
        CHECK(SpecialOptions::GetOption("FromExe", "d") == "yes");
    }
    SpecialOptions::StashShowDir(prevShow.empty() ? "unused_show_dir" : prevShow);
    SpecialOptions::StashExeDir(prevExe.empty() ? "unused_exe_dir" : prevExe);
    SpecialOptions::GetOption("");
}

TEST_CASE("SpecialOptions tolerates broken files and returns defaults", "[misc][utils][specialoptions]") {
    auto prevShow = SpecialOptions::StashShowDir();
    auto prevExe = SpecialOptions::StashExeDir();
    for (const char* body : { "", "<options", "not xml at all", "<options/>" }) {
        TempDir show;
        TempDir exe;
        show.Write("special.options", body);
        SpecialOptions::StashShowDir(show.path.string());
        SpecialOptions::StashExeDir(exe.path.string());
        SpecialOptions::GetOption("");
        INFO(body);
        CHECK(SpecialOptions::GetOption("Anything", "dflt") == "dflt");
    }
    SpecialOptions::StashShowDir(prevShow.empty() ? "unused_show_dir" : prevShow);
    SpecialOptions::StashExeDir(prevExe.empty() ? "unused_exe_dir" : prevExe);
    SpecialOptions::GetOption("");
}

TEST_CASE("SpecialOptions with no file anywhere returns the default", "[misc][utils][specialoptions]") {
    auto prevShow = SpecialOptions::StashShowDir();
    auto prevExe = SpecialOptions::StashExeDir();
    {
        TempDir a;
        TempDir b;
        SpecialOptions::StashShowDir(a.path.string());
        SpecialOptions::StashExeDir(b.path.string());
        SpecialOptions::GetOption("");
        CHECK(SpecialOptions::GetOption("x", "d") == "d");
    }
    SpecialOptions::StashShowDir(prevShow.empty() ? "unused_show_dir" : prevShow);
    SpecialOptions::StashExeDir(prevExe.empty() ? "unused_exe_dir" : prevExe);
    SpecialOptions::GetOption("");
}
