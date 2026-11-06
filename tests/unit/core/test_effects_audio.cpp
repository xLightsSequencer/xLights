#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "effects/EffectManager.h"
#include "effects/MusicEffect.h"
#include "effects/PianoEffect.h"
#include "effects/VUMeterEffect.h"
#include "effects/RenderableEffect.h"
#include "media/AudioManager.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/RenderBuffer.h"
#include "render/SequenceElements.h"
#include "support/TempDir.h"
#include "utils/UtilClasses.h"

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr int kRate = 22050;
constexpr int kFrameMs = 50;

const char* kRedBlue = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#0000FF,C_CHECKBOX_Palette2=1";
const char* kRed = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1";

// ---- synthetic audio ----

void Put16(std::string& s, uint16_t v) {
    s.push_back(char(v & 0xFF));
    s.push_back(char((v >> 8) & 0xFF));
}
void Put32(std::string& s, uint32_t v) {
    for (int i = 0; i < 4; i++) s.push_back(char((v >> (8 * i)) & 0xFF));
}

std::string MakeWav(const std::vector<int16_t>& samples) {
    std::string s;
    uint32_t dataBytes = uint32_t(samples.size() * 2);
    s += "RIFF";
    Put32(s, 36 + dataBytes);
    s += "WAVEfmt ";
    Put32(s, 16);
    Put16(s, 1);
    Put16(s, 1);
    Put32(s, uint32_t(kRate));
    Put32(s, uint32_t(kRate * 2));
    Put16(s, 2);
    Put16(s, 16);
    s += "data";
    Put32(s, dataBytes);
    for (int16_t v : samples) Put16(s, uint16_t(v));
    return s;
}

// A tone between [fromS, toS) on top of silence, appended into a buffer of totalS seconds.
void AddTone(std::vector<int16_t>& v, double freq, double fromS, double toS, double amp = 0.8) {
    size_t a = size_t(fromS * kRate);
    size_t b = std::min(v.size(), size_t(toS * kRate));
    for (size_t i = a; i < b; i++) {
        v[i] = int16_t(std::lround(amp * 32767.0 * std::sin(2.0 * kPi * freq * double(i - a) / kRate)));
    }
}

std::vector<int16_t> Silence(double seconds) { return std::vector<int16_t>(size_t(seconds * kRate), 0); }

// 1 s of silence, then 1 s of a 440 Hz tone (MIDI note 69). Frames 0-19 silent, 20-39 loud.
std::vector<int16_t> SilentThenTone(double freq = 440.0) {
    auto v = Silence(2.0);
    AddTone(v, freq, 1.0, 2.0);
    return v;
}

// 1 s of a low tone, then 1 s of a high tone.
std::vector<int16_t> LowThenHigh(double lo, double hi) {
    auto v = Silence(2.0);
    AddTone(v, lo, 0.0, 1.0);
    AddTone(v, hi, 1.0, 2.0);
    return v;
}

// 150 ms bursts of 440 Hz every 500 ms, starting at 250 ms (frames 5-7, 15-17, 25-27, 35-37).
std::vector<int16_t> Beats() {
    auto v = Silence(2.0);
    for (double t = 0.25; t < 2.0; t += 0.5) AddTone(v, 440.0, t, t + 0.15);
    return v;
}

// ---- rendering ----

using Pixels = std::vector<xlColor>;

bool Lit(const xlColor& c) { return c.alpha != 0 && (c.red || c.green || c.blue); }
bool Is(const xlColor& c, int r, int g, int b) { return c.alpha == 255 && c.red == r && c.green == g && c.blue == b; }

struct Frames {
    int w = 0;
    int h = 0;
    std::vector<Pixels> f;

    const xlColor& at(size_t frame, int x, int y) const { return f[frame][(size_t)y * w + x]; }
    int lit(size_t frame) const {
        int n = 0;
        for (const auto& c : f[frame]) n += Lit(c) ? 1 : 0;
        return n;
    }
    // alpha-weighted brightness of one frame
    double energy(size_t frame) const {
        double e = 0;
        for (const auto& c : f[frame]) e += (c.alpha / 255.0) * std::max({ c.red, c.green, c.blue }) / 255.0;
        return e;
    }
    double energy(size_t from, size_t to) const {
        double e = 0;
        for (size_t i = from; i <= to && i < f.size(); ++i) e += energy(i);
        return e;
    }
    bool columnLit(size_t frame, int x) const {
        for (int y = 0; y < h; ++y) if (Lit(at(frame, x, y))) return true;
        return false;
    }
    int columnHeight(size_t frame, int x) const {
        int n = 0;
        for (int y = 0; y < h; ++y) n += Lit(at(frame, x, y)) ? 1 : 0;
        return n;
    }
    bool frameEqual(size_t a, const Frames& o, size_t b) const {
        if (w != o.w || h != o.h) return false;
        for (size_t i = 0; i < f[a].size(); ++i) {
            const auto& p = f[a][i];
            const auto& q = o.f[b][i];
            if (p.red != q.red || p.green != q.green || p.blue != q.blue || p.alpha != q.alpha) return false;
        }
        return true;
    }
    bool operator==(const Frames& o) const {
        if (w != o.w || h != o.h || f.size() != o.f.size()) return false;
        for (size_t i = 0; i < f.size(); ++i) if (!frameEqual(i, o, i)) return false;
        return true;
    }
};

// The desktop gets its audio from the open sequence; here the test hands the context an AudioManager directly.
struct MediaContext : public HeadlessRenderContext {
    AudioManager* media = nullptr;
    AudioManager* GetCurrentMediaManager() const override { return media; }
};

enum class Path { Fused, Snapshot };

std::vector<int> Range(int from, int to) {
    std::vector<int> r;
    for (int i = from; i <= to; ++i) r.push_back(i);
    return r;
}

struct Mark {
    int start;
    int end;
    std::string label;
};

struct AudioFixture {
    TempDir td;
    std::unique_ptr<AudioManager> audio;
    MediaContext ctx;
    std::unique_ptr<Model> model;
    Element* element = nullptr;
    EffectLayer* layer = nullptr;

    AudioFixture() {
        model.reset(ctx.AllModels.CreateDefaultModel("Single Line"));
        element = ctx._sequenceElements.AddElement("Fixture", "model", true, false, true, false, false);
        layer = element->AddEffectLayer();
    }

    void LoadAudio(const std::vector<int16_t>& samples) {
        audio = std::make_unique<AudioManager>(td.Write("a.wav", MakeWav(samples)), kFrameMs);
        REQUIRE(audio->IsOk());
        ctx.media = audio.get();
    }

    void AddTiming(const std::string& name, const std::vector<Mark>& marks) {
        Element* t = ctx._sequenceElements.AddElement(name, "timing", true, false, true, false, false);
        REQUIRE(t != nullptr);
        EffectLayer* el = t->GetEffectLayerCount() > 0 ? t->GetEffectLayer(0) : t->AddEffectLayer();
        for (const auto& m : marks) el->AddEffect(0, m.label, "", "", m.start, m.end, 0, false);
    }

    Effect* lastEffect = nullptr;

    Frames Render(const std::string& name, const std::string& settings, const std::string& palette,
                  int w, int h, const std::vector<int>& periods, int endMs = 2000, Path path = Path::Fused) {
        Effect* e = layer->AddEffect(0, name, settings, palette, 0, endMs, 0, false);
        REQUIRE(e != nullptr);
        lastEffect = e;
        RenderableEffect* fx = ctx.effectManager.GetEffect(name);
        REQUIRE(fx != nullptr);

        RenderBuffer rb(&ctx, nullptr, model.get());
        rb.InitBuffer(h, w, "None");
        xlColorVector colors;
        xlColorCurveVector curves;
        e->CopyPalette(colors, curves);
        rb.SetPalette(colors, curves);
        rb.SetFrameTimeInMs(kFrameMs);
        rb.SetEffectDuration(0, endMs);

        SettingsMap sm;
        e->CopySettingsMap(sm, true);

        Frames out;
        out.w = w;
        out.h = h;
        bool first = true;
        for (int p : periods) {
            rb.Clear();
            rb.SetState(p, first);
            first = false;
            std::unique_ptr<EffectFrameState> snap;
            if (path == Path::Snapshot) {
                snap = fx->AdvanceState(e, sm, rb);
                rb.pendingSnapshot = snap.get();
            }
            fx->Render(e, sm, rb);
            rb.pendingSnapshot = nullptr;
            out.f.emplace_back(rb.GetPixels(), rb.GetPixels() + (size_t)w * h);
        }
        return out;
    }
};

std::string VU(const std::string& type, const std::string& extra = "") {
    std::string s = "E_CHOICE_VUMeter_Type=" + type;
    if (!extra.empty()) s += "," + extra;
    return s;
}

// Every pixel is a mix of red and blue (the palette) - no green may appear.
bool RedBlueOnly(const Frames& fr) {
    for (const auto& px : fr.f)
        for (const auto& c : px)
            if (Lit(c) && c.green != 0) return false;
    return true;
}

const std::vector<std::string>& AllVuTypes() {
    static const std::vector<std::string> v = {
        "Spectrogram", "Spectrogram Peak", "Spectrogram Line", "Spectrogram Circle Line", "Volume Bars", "Waveform",
        "Frame Waveform", "On", "Color On", "Dominant Frequency Colour", "Dominant Frequency Colour Gradient",
        "Intensity Wave", "Pulse", "Level Bar", "Level Random Bar", "Level Color", "Level Pulse", "Level Jump",
        "Level Jump 100", "Level Pulse Color", "Level Shape", "Timing Event Bar", "Timing Event Bar Bounce",
        "Timing Event Random Bar", "Timing Event Bars", "Timing Event Spike", "Timing Event Sweep",
        "Timing Event Sweep 2", "Timing Event Timed Sweep", "Timing Event Timed Sweep 2",
        "Timing Event Alternate Timed Sweep", "Timing Event Alternate Timed Sweep 2",
        "Timing Event Timed Chase From Middle", "Timing Event Timed Chase To Middle", "Timing Event Color",
        "Timing Event Jump", "Timing Event Jump 100", "Timing Event Pulse", "Timing Event Pulse Color", "Note On",
        "Note Level Pulse", "Note Level Jump", "Note Level Jump 100", "Note Level Bar", "Note Level Random Bar"
    };
    return v;
}

const std::vector<Mark> kMarks = { { 1000, 1250, "a" }, { 1250, 1500, "b" }, { 1500, 1750, "a" }, { 1750, 2000, "b" } };

} // namespace

// ---------------------------------------------------------------------------
// VU Meter
// ---------------------------------------------------------------------------

namespace {

// Types that draw something even in silence by design (a baseline line, the first palette
// colour, the always-on bar set, the idle colour between timing marks, the peak markers).
const std::set<std::string> kDrawsInSilence = {
    "Spectrogram Peak", "Spectrogram Line", "Spectrogram Circle Line", "Frame Waveform", "Color On",
    "Intensity Wave", "Timing Event Bars", "Timing Event Color", "Level Shape"
};
// ...and of those, the ones that fill the whole buffer whether quiet or loud
const std::set<std::string> kFullInSilence = { "Color On", "Intensity Wave", "Timing Event Bars" };

int LeftmostLit(const Frames& fr, size_t f) {
    for (int x = 0; x < fr.w; ++x) if (fr.columnLit(f, x)) return x;
    return -1;
}

int TallestColumn(const Frames& fr, size_t f) {
    int best = -1, bh = -1;
    for (int x = 0; x < fr.w; ++x) {
        int h = fr.columnHeight(f, x);
        if (h > bh) { bh = h; best = x; }
    }
    return best;
}

// Columns lit in frame f, as a bitmask string ("##.." etc) for easy messages.
std::string Cols(const Frames& fr, size_t f) {
    std::string s;
    for (int x = 0; x < fr.w; ++x) s += fr.columnLit(f, x) ? '#' : '.';
    return s;
}

bool MirroredX(const Frames& a, size_t fa, const Frames& b, size_t fb) {
    for (int y = 0; y < a.h; ++y)
        for (int x = 0; x < a.w; ++x) {
            const auto& p = a.at(fa, x, y);
            const auto& q = b.at(fb, a.w - 1 - x, y);
            if (Lit(p) != Lit(q)) return false;
            if (Lit(p) && (p.red != q.red || p.blue != q.blue)) return false;
        }
    return true;
}

} // namespace

TEST_CASE("VU Meter: every type is palette-only, louder with sound, and the snapshot path matches the serial path", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(SilentThenTone());
    fx.AddTiming("T", kMarks);
    for (const auto& type : AllVuTypes()) {
        INFO("type " << type);
        // the default shape sensitivity makes the circle larger than this small buffer
        std::string extra = "E_CHOICE_VUMeter_TimingTrack=T";
        if (type == "Level Shape") extra += ",E_SLIDER_VUMeter_Sensitivity=10";
        Frames a = fx.Render("VU Meter", VU(type, extra), kRedBlue, 12, 10, Range(0, 39));
        Frames b = fx.Render("VU Meter", VU(type, extra), kRedBlue, 12, 10, Range(0, 39), 2000, Path::Snapshot);
        CHECK(a == b);

        Frames tail = a;
        tail.f.erase(tail.f.begin(), tail.f.begin() + 8);
        CHECK(RedBlueOnly(tail));

        // average brightness, silent half (frames 8-17) vs loud half (22-39)
        double silent = a.energy(8, 17) / 10.0;
        double loud = a.energy(22, 39) / 18.0;
        if (kFullInSilence.count(type) == 0) {
            CHECK(loud > silent);
        } else {
            CHECK(loud >= silent - 0.1);
        }
        if (kDrawsInSilence.count(type) == 0) {
            for (int f = 8; f <= 17; ++f) {
                INFO("frame " << f);
                CHECK(a.lit(f) == 0);
            }
        }
    }
}

TEST_CASE("VU Meter draws nothing without audio or timing track", "[effects][audiofx]") {
    AudioFixture fx; // no media, no timing track
    for (const auto& type : AllVuTypes()) {
        INFO("type " << type);
        for (Path p : { Path::Fused, Path::Snapshot }) {
            Frames fr = fx.Render("VU Meter", VU(type), kRedBlue, 8, 6, { 0, 5, 21, 30 }, 2000, p);
            for (size_t f = 0; f < fr.f.size(); ++f) CHECK(fr.lit(f) == 0);
        }
    }
    // a named but missing track: the always-on timing types show their idle state, the rest stay dark
    for (const char* t : { "Timing Event Bar", "Timing Event Jump 100", "Timing Event Pulse", "Pulse", "Timing Event Timed Sweep",
                           "Timing Event Timed Chase To Middle", "Timing Event Spike" }) {
        INFO(t);
        Frames fr = fx.Render("VU Meter", VU(t, "E_CHOICE_VUMeter_TimingTrack=Missing"), kRedBlue, 8, 6, { 0, 21, 30 });
        for (size_t f = 0; f < fr.f.size(); ++f) CHECK(fr.lit(f) == 0);
    }
    Frames idle = fx.Render("VU Meter", VU("Timing Event Bars", "E_CHOICE_VUMeter_TimingTrack=Missing"), kRedBlue, 12, 6, { 21 });
    CHECK(idle.lit(0) == 72);
}

TEST_CASE("VU Meter On follows the level and the gain", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(SilentThenTone());
    Frames fr = fx.Render("VU Meter", VU("On"), kRed, 4, 4, { 10, 30 });
    CHECK(fr.at(0, 0, 0).alpha == 0);
    CHECK(fr.at(1, 0, 0).alpha >= 250);
    CHECK(fr.at(1, 3, 3).red == 255);
    Frames half = fx.Render("VU Meter", VU("On", "E_SLIDER_VUMeter_Gain=-50"), kRed, 4, 4, { 30 });
    CHECK(std::abs(int(half.at(0, 0, 0).alpha) - 127) <= 3);
    // gain cannot push the level past full
    Frames boost = fx.Render("VU Meter", VU("On", "E_SLIDER_VUMeter_Gain=100"), kRed, 4, 4, { 30 });
    CHECK(boost.at(0, 0, 0).alpha == 255);
}

TEST_CASE("VU Meter Volume Bars scroll the level history in from the right", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(SilentThenTone());
    // 6 bars on 12 columns: 2 columns per frame of history, newest at the right
    Frames fr = fx.Render("VU Meter", VU("Volume Bars"), kRedBlue, 12, 10, { 22, 30 });
    CHECK(Cols(fr, 0) == "........####");
    for (int x = 0; x < 12; ++x) CHECK(fr.columnHeight(1, x) >= 9);
    // the bars are coloured bottom to top from the palette blend
    CHECK(fr.at(1, 0, 0).red > fr.at(1, 0, 0).blue);
    CHECK(fr.at(1, 0, 8).blue > fr.at(1, 0, 8).red);
}

TEST_CASE("VU Meter Waveform grows from the middle and y offset moves it", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(SilentThenTone());
    Frames fr = fx.Render("VU Meter", VU("Waveform"), kRedBlue, 12, 10, { 15, 30 });
    CHECK(fr.lit(0) == 0);
    CHECK(fr.columnHeight(1, 6) >= 8);
    Frames up = fx.Render("VU Meter", VU("Waveform", "E_SLIDER_VUMeter_Gain=-50,E_SLIDER_VUMeter_YOffset=40"), kRedBlue, 12, 10, { 30 });
    Frames mid = fx.Render("VU Meter", VU("Waveform", "E_SLIDER_VUMeter_Gain=-50"), kRedBlue, 12, 10, { 30 });
    CHECK(!Lit(up.at(0, 6, 2)));
    CHECK(Lit(mid.at(0, 6, 2)));
    CHECK(Lit(up.at(0, 6, 8)));

    // Frame Waveform draws the samples of this frame: flat line in silence, a zig-zag when loud
    Frames fw = fx.Render("VU Meter", VU("Frame Waveform", "E_SLIDER_VUMeter_Bars=12"), kRedBlue, 12, 10, { 10, 30 });
    std::set<int> rowsSilent, rowsLoud;
    for (int y = 0; y < 10; ++y)
        for (int x = 0; x < 12; ++x) {
            if (Lit(fw.at(0, x, y))) rowsSilent.insert(y);
            if (Lit(fw.at(1, x, y))) rowsLoud.insert(y);
        }
    CHECK(rowsSilent == std::set<int>{ 5 });
    CHECK(rowsLoud.size() > 3);
}

TEST_CASE("VU Meter Spectrogram moves right when the pitch rises", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(LowThenHigh(220.0, 1760.0));
    for (const char* t : { "Spectrogram", "Spectrogram Peak" }) {
        INFO(t);
        Frames fr = fx.Render("VU Meter", VU(t, "E_SLIDER_VUMeter_Bars=12,E_CHECKBOX_VUMeter_SlowDownFalls=0"), kRedBlue, 12, 10, { 10, 30 });
        int lo = TallestColumn(fr, 0);
        int hi = TallestColumn(fr, 1);
        CHECK(lo >= 0);
        CHECK(hi > lo);
    }
}

TEST_CASE("VU Meter Spectrogram options change the drawing", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(Beats());
    auto periods = Range(0, 20);
    for (const char* t : { "Spectrogram", "Spectrogram Peak", "Spectrogram Line", "Spectrogram Circle Line" }) {
        INFO(t);
        Frames base = fx.Render("VU Meter", VU(t), kRedBlue, 12, 10, periods);
        CHECK(base.energy(5, 7) > 0);
        for (const char* opt : { "E_CHECKBOX_VUMeter_SlowDownFalls=0", "E_CHECKBOX_VUMeter_LogarithmicX=1", "E_SLIDER_VUMeter_XOffset=30",
                                 "E_SLIDER_VUMeter_YOffset=30", "E_SLIDER_VUMeter_StartNote=60,E_SLIDER_VUMeter_EndNote=40",
                                 "E_SLIDER_VUMeter_Sensitivity=40" }) {
            INFO(opt);
            Frames v = fx.Render("VU Meter", VU(t, opt), kRedBlue, 12, 10, periods);
            CHECK(RedBlueOnly(v));
            CHECK(v.energy(0, 20) > 0);
        }
    }
    // falls are slowed: a frame after the burst still shows bars
    Frames slow = fx.Render("VU Meter", VU("Spectrogram"), kRedBlue, 12, 10, periods);
    Frames fast = fx.Render("VU Meter", VU("Spectrogram", "E_CHECKBOX_VUMeter_SlowDownFalls=0"), kRedBlue, 12, 10, periods);
    CHECK(slow.energy(9, 10) > fast.energy(9, 10));
    // line history: Spectrogram Line leaves fading trails of earlier frames
    Frames line = fx.Render("VU Meter", VU("Spectrogram Line", "E_SLIDER_VUMeter_Sensitivity=50"), kRedBlue, 12, 10, periods);
    bool partial = false;
    for (const auto& px : line.f)
        for (const auto& c : px) partial |= Lit(c) && c.alpha < 255;
    CHECK(partial);
}

TEST_CASE("VU Meter Dominant Frequency Colour picks the colour from the pitch", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(LowThenHigh(220.0, 880.0)); // notes 57 and 81
    Frames fr = fx.Render("VU Meter", VU("Dominant Frequency Colour"), kRedBlue, 4, 4, { 10, 30 });
    CHECK(Is(fr.at(0, 0, 0), 255, 0, 0));
    CHECK(Is(fr.at(1, 0, 0), 0, 0, 255));
    Frames gr = fx.Render("VU Meter", VU("Dominant Frequency Colour Gradient"), kRedBlue, 4, 4, { 10, 30 });
    CHECK(gr.at(0, 0, 0).red > gr.at(0, 0, 0).blue);
    CHECK(gr.at(1, 0, 0).blue > gr.at(1, 0, 0).red);
    // a note range that excludes both tones finds no dominant note above the sensitivity
    Frames none = fx.Render("VU Meter", VU("Dominant Frequency Colour", "E_SLIDER_VUMeter_StartNote=100,E_SLIDER_VUMeter_EndNote=120"), kRedBlue, 4, 4, { 10, 30 });
    CHECK(none.lit(0) == 0);
    CHECK(none.lit(1) == 0);
}

TEST_CASE("VU Meter note types only listen to their note range", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(SilentThenTone()); // note 69
    Frames in = fx.Render("VU Meter", VU("Note On", "E_SLIDER_VUMeter_StartNote=60,E_SLIDER_VUMeter_EndNote=75"), kRed, 4, 4, { 30 });
    Frames out = fx.Render("VU Meter", VU("Note On", "E_SLIDER_VUMeter_StartNote=100,E_SLIDER_VUMeter_EndNote=120"), kRed, 4, 4, { 30 });
    CHECK(in.at(0, 0, 0).alpha > out.at(0, 0, 0).alpha + 50);

    for (const char* t : { "Note Level Bar", "Note Level Random Bar", "Note Level Pulse", "Note Level Jump", "Note Level Jump 100" }) {
        INFO(t);
        for (Path p : { Path::Fused, Path::Snapshot }) {
            Frames hit = fx.Render("VU Meter", VU(t, "E_SLIDER_VUMeter_StartNote=60,E_SLIDER_VUMeter_EndNote=75"), kRedBlue, 12, 6, Range(10, 30), 2000, p);
            Frames miss = fx.Render("VU Meter", VU(t, "E_SLIDER_VUMeter_StartNote=110,E_SLIDER_VUMeter_EndNote=120"), kRedBlue, 12, 6, Range(10, 30), 2000, p);
            CHECK(hit.energy(12, 20) > 0);
            CHECK(miss.energy(0, 20) == 0);
        }
    }
}

TEST_CASE("VU Meter Color On and Intensity Wave map the level to the palette", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(SilentThenTone());
    Frames co = fx.Render("VU Meter", VU("Color On"), kRedBlue, 4, 4, { 10, 30 });
    CHECK(Is(co.at(0, 0, 0), 255, 0, 0));
    CHECK(co.at(1, 0, 0).blue > 240);
    CHECK(co.at(1, 0, 0).red < 15);

    // two colours: blend from the last colour (quiet) to the first (loud)
    Frames iw = fx.Render("VU Meter", VU("Intensity Wave"), kRedBlue, 12, 4, { 15, 30 });
    CHECK(Is(iw.at(0, 0, 0), 0, 0, 255));
    CHECK(iw.at(1, 11, 0).red > 240);
    // one colour: the level drives the alpha
    Frames one = fx.Render("VU Meter", VU("Intensity Wave"), kRed, 12, 4, { 15, 30 });
    CHECK(one.lit(0) == 0);
    CHECK(one.at(1, 11, 0).alpha > 240);
}

TEST_CASE("VU Meter Level Bar steps one bar per loud frame, Random Bar never repeats", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(SilentThenTone());
    for (Path p : { Path::Fused, Path::Snapshot }) {
        Frames fr = fx.Render("VU Meter", VU("Level Bar"), kRedBlue, 12, 4, Range(20, 32), 2000, p);
        for (size_t f = 1; f < fr.f.size(); ++f) {
            INFO("frame " << f << " " << Cols(fr, f - 1) << " -> " << Cols(fr, f));
            int a = LeftmostLit(fr, f - 1);
            int b = LeftmostLit(fr, f);
            REQUIRE(a >= 0);
            REQUIRE(b >= 0);
            CHECK(fr.lit(f) == 2 * 4);
            CHECK(b == (a + 2) % 12);
            // the colour alternates between the two palette entries
            CHECK(fr.at(f, b, 0).red != fr.at(f - 1, a, 0).red);
        }
        Frames rnd = fx.Render("VU Meter", VU("Level Random Bar"), kRedBlue, 12, 4, Range(20, 32), 2000, p);
        for (size_t f = 1; f < rnd.f.size(); ++f) {
            CHECK(rnd.lit(f) == 2 * 4);
            CHECK(LeftmostLit(rnd, f) != LeftmostLit(rnd, f - 1));
        }
    }
}

TEST_CASE("VU Meter level pulse, jump and colour react to each beat", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(Beats()); // bursts at frames 5-7, 15-17, 25-27, 35-37
    for (Path p : { Path::Fused, Path::Snapshot }) {
        Frames pulse = fx.Render("VU Meter", VU("Level Pulse"), kRed, 4, 4, Range(0, 39), 2000, p);
        CHECK(pulse.at(7, 0, 0).alpha == 255);
        CHECK(pulse.at(8, 0, 0).alpha < 255);
        CHECK(pulse.at(9, 0, 0).alpha < pulse.at(8, 0, 0).alpha);
        CHECK(pulse.lit(14) == 0);
        CHECK(pulse.at(17, 0, 0).alpha == 255);

        Frames jump = fx.Render("VU Meter", VU("Level Jump 100"), kRedBlue, 4, 12, Range(0, 39), 2000, p);
        CHECK(jump.columnHeight(7, 0) == 12);
        CHECK(jump.columnHeight(9, 0) < jump.columnHeight(8, 0));
        CHECK(jump.columnHeight(8, 0) < 12);
        CHECK(jump.lit(14) == 0);
        Frames jumpLevel = fx.Render("VU Meter", VU("Level Jump", "E_SLIDER_VUMeter_Gain=-30,E_SLIDER_VUMeter_Sensitivity=50"), kRedBlue, 4, 12, Range(0, 39), 2000, p);
        CHECK(jumpLevel.columnHeight(7, 0) < 12);
        CHECK(jumpLevel.columnHeight(7, 0) > 0);

        // Level Color switches colour once per beat, not once per loud frame
        Frames col = fx.Render("VU Meter", VU("Level Color"), kRedBlue, 4, 4, Range(0, 39), 2000, p);
        CHECK(col.lit(2) == 0);
        CHECK(Is(col.at(10, 0, 0), 255, 0, 0));
        CHECK(Is(col.at(20, 0, 0), 0, 0, 255));
        CHECK(Is(col.at(30, 0, 0), 255, 0, 0));
        Frames pc = fx.Render("VU Meter", VU("Level Pulse Color"), kRedBlue, 4, 4, Range(0, 39), 2000, p);
        CHECK(Is(pc.at(7, 0, 0), 255, 0, 0));
        CHECK(Is(pc.at(17, 0, 0), 0, 0, 255));
        CHECK(pc.at(18, 0, 0).alpha < 255);
    }
}

TEST_CASE("VU Meter Level Shape: louder is bigger and every shape draws", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(Beats());
    for (const char* shape : { "Circle", "Filled Circle", "Square", "Filled Square", "Diamond", "Filled Diamond", "Star",
                               "Filled Star", "Tree", "Filled Tree", "Crucifix", "Filled Crucifix", "Present",
                               "Filled Present", "Candy Cane", "Snowflake", "Heart", "Filled Heart" }) {
        INFO(shape);
        std::string s = std::string("E_CHOICE_VUMeter_Shape=") + shape + ",E_SLIDER_VUMeter_Sensitivity=12,E_CHECKBOX_VUMeter_SlowDownFalls=0";
        Frames a = fx.Render("VU Meter", VU("Level Shape", s), kRedBlue, 20, 20, { 2, 6 });
        Frames b = fx.Render("VU Meter", VU("Level Shape", s), kRedBlue, 20, 20, { 2, 6 }, 2000, Path::Snapshot);
        CHECK(a == b);
        CHECK(RedBlueOnly(a));
        CHECK(a.lit(1) > a.lit(0));
        CHECK(a.energy(1) > a.energy(0));
    }
    // filled shapes cover more than their outlines
    Frames ring = fx.Render("VU Meter", VU("Level Shape", "E_CHOICE_VUMeter_Shape=Circle,E_SLIDER_VUMeter_Sensitivity=12"), kRedBlue, 20, 20, { 6 });
    Frames disc = fx.Render("VU Meter", VU("Level Shape", "E_CHOICE_VUMeter_Shape=Filled Circle,E_SLIDER_VUMeter_Sensitivity=12"), kRedBlue, 20, 20, { 6 });
    CHECK(Lit(disc.at(0, 10, 10)));
    CHECK(!Lit(ring.at(0, 10, 10)));
    // offsets move the shape centre
    Frames left = fx.Render("VU Meter", VU("Level Shape", "E_CHOICE_VUMeter_Shape=Filled Square,E_SLIDER_VUMeter_Sensitivity=20,E_SLIDER_VUMeter_XOffset=-60"), kRedBlue, 20, 20, { 6 });
    Frames centre = fx.Render("VU Meter", VU("Level Shape", "E_CHOICE_VUMeter_Shape=Filled Square,E_SLIDER_VUMeter_Sensitivity=20"), kRedBlue, 20, 20, { 6 });
    CHECK(LeftmostLit(left, 0) < LeftmostLit(centre, 0));
    // slow falls: just after a beat the shape is still large
    Frames slow = fx.Render("VU Meter", VU("Level Shape", "E_CHOICE_VUMeter_Shape=Filled Circle,E_SLIDER_VUMeter_Sensitivity=12"), kRedBlue, 20, 20, Range(4, 9));
    Frames fast = fx.Render("VU Meter", VU("Level Shape", "E_CHOICE_VUMeter_Shape=Filled Circle,E_SLIDER_VUMeter_Sensitivity=12,E_CHECKBOX_VUMeter_SlowDownFalls=0"), kRedBlue, 20, 20, Range(4, 9));
    CHECK(slow.lit(5) > fast.lit(5));
}

TEST_CASE("VU Meter timing bars advance on each mark, bounce and honour the filter", "[effects][audiofx]") {
    AudioFixture fx;
    fx.AddTiming("T", kMarks); // marks at frames 20, 25, 30, 35, labels a b a b
    auto starts = [](const Frames& fr) {
        std::vector<int> v;
        for (size_t f : { 2, 7, 12, 17 }) v.push_back(LeftmostLit(fr, f));
        return v;
    };
    for (Path p : { Path::Fused, Path::Snapshot }) {
        // 3 bars of 4 columns
        Frames bar = fx.Render("VU Meter", VU("Timing Event Bar", "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Bars=3"), kRedBlue, 12, 4, Range(18, 39), 2000, p);
        CHECK(bar.lit(0) == 0);
        CHECK(starts(bar) == std::vector<int>{ 0, 4, 8, 0 });
        Frames bounce = fx.Render("VU Meter", VU("Timing Event Bar Bounce", "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Bars=3"), kRedBlue, 12, 4, Range(18, 39), 2000, p);
        CHECK(starts(bounce) == std::vector<int>{ 0, 4, 8, 4 });
        Frames filt = fx.Render("VU Meter", VU("Timing Event Bar", "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Bars=3,E_TEXTCTRL_Filter=a"), kRedBlue, 12, 4, Range(18, 39), 2000, p);
        CHECK(starts(filt) == std::vector<int>{ 0, 0, 4, 4 });
        Frames rx = fx.Render("VU Meter", VU("Timing Event Bar", "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Bars=3,E_TEXTCTRL_Filter=^b$,E_CHECKBOX_Regex=1"), kRedBlue, 12, 4, Range(18, 39), 2000, p);
        CHECK(starts(rx)[0] == -1);
        CHECK(starts(rx)[1] == 0);
        Frames rnd = fx.Render("VU Meter", VU("Timing Event Random Bar", "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Bars=4"), kRedBlue, 12, 4, Range(18, 39), 2000, p);
        auto r = starts(rnd);
        for (size_t i = 1; i < r.size(); ++i) CHECK(r[i] != r[i - 1]);
        // all bars lit, the starting colour rotating at each mark
        Frames all = fx.Render("VU Meter", VU("Timing Event Bars", "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Bars=2"), kRedBlue, 12, 4, Range(18, 39), 2000, p);
        CHECK(all.lit(2) == 48);
        const xlColor& l20 = all.at(2, 0, 0);
        const xlColor& r20 = all.at(2, 11, 0);
        CHECK((Is(l20, 255, 0, 0) || Is(l20, 0, 0, 255)));
        CHECK((Is(r20, 255, 0, 0) || Is(r20, 0, 0, 255)));
        CHECK(l20.red != r20.red);
        CHECK(all.frameEqual(2, all, 6)); // steady between marks
        CHECK(all.at(7, 0, 0).red == r20.red);
        CHECK(all.frameEqual(2, all, 12));
    }
}

TEST_CASE("VU Meter timing pulse, jump and colour start on the mark and fade", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(SilentThenTone());
    fx.AddTiming("T", kMarks);
    for (Path p : { Path::Fused, Path::Snapshot }) {
        Frames pulse = fx.Render("VU Meter", VU("Pulse", "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Bars=4"), kRed, 4, 4, Range(18, 30), 2000, p);
        CHECK(pulse.lit(1) == 0);
        CHECK(pulse.at(2, 0, 0).alpha == 255);
        CHECK(pulse.at(3, 0, 0).alpha < 255);
        CHECK(pulse.at(4, 0, 0).alpha < pulse.at(3, 0, 0).alpha);
        CHECK(pulse.lit(6) == 0);
        CHECK(pulse.at(7, 0, 0).alpha == 255);

        Frames tp = fx.Render("VU Meter", VU("Timing Event Pulse", "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Bars=4"), kRed, 4, 4, Range(18, 30), 2000, p);
        CHECK(tp.lit(1) == 0);
        CHECK(tp.at(2, 0, 0).alpha == 255);
        CHECK(tp.at(3, 0, 0).alpha < 255);
        CHECK(tp.lit(6) == 0);

        Frames tpc = fx.Render("VU Meter", VU("Timing Event Pulse Color", "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Bars=4"), kRedBlue, 4, 4, Range(18, 30), 2000, p);
        CHECK(tpc.at(2, 0, 0).red == 255);
        CHECK(tpc.at(7, 0, 0).blue == 255);

        Frames j100 = fx.Render("VU Meter", VU("Timing Event Jump 100", "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Bars=4"), kRedBlue, 2, 8, Range(18, 30), 2000, p);
        CHECK(j100.columnHeight(1, 0) == 0);
        CHECK(j100.columnHeight(2, 0) == 8);
        CHECK(j100.columnHeight(3, 0) < 8);
        CHECK(j100.columnHeight(4, 0) < j100.columnHeight(3, 0));
        Frames jl = fx.Render("VU Meter", VU("Timing Event Jump", "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Bars=4,E_SLIDER_VUMeter_Gain=-50"), kRedBlue, 2, 8, Range(18, 30), 2000, p);
        CHECK(jl.columnHeight(2, 0) > 0);
        CHECK(jl.columnHeight(2, 0) < 8);

        // Timing Event Color: full colour inside a mark, the sensitivity as alpha outside one
        Frames tc = fx.Render("VU Meter", VU("Timing Event Color", "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Sensitivity=40"), kRedBlue, 4, 4, Range(10, 30), 2000, p);
        CHECK(tc.at(0, 0, 0).alpha == 40 * 255 / 100);
        const xlColor& m1 = tc.at(12, 0, 0);
        const xlColor& m2 = tc.at(17, 0, 0);
        CHECK((Is(m1, 255, 0, 0) || Is(m1, 0, 0, 255)));
        CHECK((Is(m2, 255, 0, 0) || Is(m2, 0, 0, 255)));
        CHECK(m1.red != m2.red);
        CHECK(tc.frameEqual(12, tc, 20)); // the colour cycles back on the third mark
    }
}

TEST_CASE("VU Meter timing spike and sweeps move across the buffer", "[effects][audiofx]") {
    AudioFixture fx;
    fx.AddTiming("T", kMarks);
    // spike: the mark enters at the right one frame later and scrolls left
    Frames sp = fx.Render("VU Meter", VU("Timing Event Spike", "E_CHOICE_VUMeter_TimingTrack=T"), kRedBlue, 12, 4, { 20, 21, 23 });
    CHECK(sp.lit(0) == 0);
    CHECK(Cols(sp, 1) == "..........##");
    CHECK(Cols(sp, 2) == "......##....");
    for (const char* t : { "Timing Event Sweep", "Timing Event Sweep 2" }) {
        Frames sw = fx.Render("VU Meter", VU(t, "E_CHOICE_VUMeter_TimingTrack=T"), kRedBlue, 12, 4, { 21 });
        CHECK(Cols(sw, 0) == "..........##");
    }
    // timed sweeps travel left to right across one mark
    for (const char* t : { "Timing Event Timed Sweep", "Timing Event Timed Sweep 2" }) {
        INFO(t);
        Frames ts = fx.Render("VU Meter", VU(t, "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Bars=2"), kRedBlue, 12, 4, Range(20, 24));
        for (size_t f = 1; f < ts.f.size(); ++f) CHECK(LeftmostLit(ts, f) > LeftmostLit(ts, f - 1));
        CHECK(fx.Render("VU Meter", VU(t, "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Bars=2"), kRedBlue, 12, 4, { 10 }).lit(0) == 0);
    }
    // the alternate sweeps reverse direction on every other mark
    for (const char* t : { "Timing Event Alternate Timed Sweep", "Timing Event Alternate Timed Sweep 2" }) {
        INFO(t);
        for (Path p : { Path::Fused, Path::Snapshot }) {
            Frames as = fx.Render("VU Meter", VU(t, "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Bars=2"), kRedBlue, 12, 4, Range(18, 30), 2000, p);
            CHECK(as.lit(4) > 0);
            CHECK(MirroredX(as, 4, as, 9));
            CHECK(!as.frameEqual(4, as, 9));
        }
    }
    // chases are mirror images about the middle
    Frames to = fx.Render("VU Meter", VU("Timing Event Timed Chase To Middle", "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Bars=2"), kRedBlue, 12, 4, { 21 });
    CHECK(to.lit(0) > 0);
    CHECK(MirroredX(to, 0, to, 0));
    Frames from = fx.Render("VU Meter", VU("Timing Event Timed Chase From Middle", "E_CHOICE_VUMeter_TimingTrack=T,E_SLIDER_VUMeter_Bars=2"), kRedBlue, 13, 4, { 21 });
    CHECK(from.lit(0) > 0);
    CHECK(MirroredX(from, 0, from, 0));
}

TEST_CASE("VU Meter settings check, timing rename and frame parallelism", "[effects][audiofx]") {
    AudioFixture fx;
    fx.AddTiming("T", kMarks);
    RenderableEffect* vu = fx.ctx.effectManager.GetEffect("VU Meter");
    REQUIRE(vu != nullptr);
    auto check = [&](const std::string& settings, AudioManager* media) {
        Effect* e = fx.layer->AddEffect(0, "VU Meter", settings, kRedBlue, 0, 1000, 0, false);
        std::string all;
        for (const auto& s : vu->CheckEffectSettings(e->GetSettings(), media, fx.model.get(), e, false)) all += s + "\n";
        return all;
    };
    CHECK(check("E_CHOICE_VUMeter_Type=Spectrogram", nullptr).find("ERR") != std::string::npos);
    CHECK(check("E_CHOICE_VUMeter_Type=Timing Event Bar", nullptr).find("timing track") != std::string::npos);
    CHECK(check("E_CHOICE_VUMeter_Type=Timing Event Bar,E_CHOICE_VUMeter_TimingTrack=Missing", nullptr).find("unknown timing track") != std::string::npos);
    CHECK(check("E_CHOICE_VUMeter_Type=Timing Event Bar,E_CHOICE_VUMeter_TimingTrack=T", nullptr).find("ERR") == std::string::npos);
    CHECK(check("E_CHOICE_VUMeter_Type=Level Shape,E_CHOICE_VUMeter_Shape=SVG", nullptr).find("SVG") != std::string::npos);

    Effect* e = fx.layer->AddEffect(0, "VU Meter", "E_CHOICE_VUMeter_TimingTrack=T", kRedBlue, 0, 1000, 0, false);
    vu->RenameTimingTrack("X", "Y", e);
    CHECK(e->GetSettings().Get("E_CHOICE_VUMeter_TimingTrack", "") == "T");
    vu->RenameTimingTrack("T", "Beats", e);
    CHECK(e->GetSettings().Get("E_CHOICE_VUMeter_TimingTrack", "") == "Beats");

    auto mode = [&](const std::string& type, const std::string& extra = "") {
        SettingsMap sm;
        sm["CHOICE_VUMeter_Type"] = type;
        if (!extra.empty()) sm["CHOICE_VUMeter_Shape"] = extra;
        return vu->GetFrameParallelism(sm);
    };
    CHECK(mode("Waveform") == RenderableEffect::FrameParallelism::Pure);
    CHECK(mode("Spectrogram") == RenderableEffect::FrameParallelism::Stateful);
    CHECK(mode("Level Bar") == RenderableEffect::FrameParallelism::Snapshottable);
    CHECK(mode("Level Shape") == RenderableEffect::FrameParallelism::Snapshottable);
    CHECK(mode("Level Shape", "SVG") == RenderableEffect::FrameParallelism::Stateful);
    CHECK(mode("unused") == RenderableEffect::FrameParallelism::Stateful);
    // the classification and AdvanceState agree: only Snapshottable types return a snapshot
    RenderBuffer rb(&fx.ctx, nullptr, fx.model.get());
    rb.InitBuffer(4, 4, "None");
    rb.SetFrameTimeInMs(kFrameMs);
    rb.SetEffectDuration(0, 1000);
    rb.SetState(0, true);
    for (const auto& type : AllVuTypes()) {
        INFO(type);
        Effect* te = fx.layer->AddEffect(0, "VU Meter", VU(type), kRedBlue, 0, 1000, 0, false);
        SettingsMap sm;
        te->CopySettingsMap(sm, true);
        bool snap = vu->AdvanceState(te, sm, rb) != nullptr;
        CHECK(snap == (vu->GetFrameParallelism(sm) == RenderableEffect::FrameParallelism::Snapshottable));
    }
}

TEST_CASE("VU Meter Timing Event Bars covers the whole width when it does not divide evenly", "[effects][audiofx]") {
    AudioFixture fx;
    fx.AddTiming("T", kMarks);
    Frames fr = fx.Render("VU Meter", VU("Timing Event Bars", "E_CHOICE_VUMeter_TimingTrack=T"), kRedBlue, 8, 6, { 21 });
    CHECK(fr.lit(0) == 48);
}

TEST_CASE("VU Meter level pulses stay dark in silence at the start of an effect", "[effects][audiofx]") {
    // no level has been seen yet, so nothing is fading out
    AudioFixture fx;
    fx.LoadAudio(SilentThenTone());
    for (const char* t : { "Level Pulse", "Level Pulse Color", "Note Level Pulse" }) {
        INFO(t);
        Frames fr = fx.Render("VU Meter", VU(t), kRedBlue, 4, 4, Range(0, 5));
        for (size_t f = 0; f < fr.f.size(); ++f) CHECK(fr.lit(f) == 0);
    }
}

// ---------------------------------------------------------------------------
// Piano
// ---------------------------------------------------------------------------

namespace {

// key up white, key up black, key down white, key down black, border
const char* kPianoPal = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#00FF00,C_CHECKBOX_Palette2=1,"
                        "C_BUTTON_Palette3=#0000FF,C_CHECKBOX_Palette3=1,C_BUTTON_Palette4=#FFFF00,C_CHECKBOX_Palette4=1,"
                        "C_BUTTON_Palette5=#00FFFF,C_CHECKBOX_Palette5=1";

int CountColour(const Frames& fr, size_t frame, int r, int g, int b) {
    int n = 0;
    for (const auto& c : fr.f[frame]) n += Is(c, r, g, b) ? 1 : 0;
    return n;
}

// C4 for 0.5 s, C#4 for 0.5 s, E4+G4 for 0.5 s, then nothing.
const std::vector<Mark> kPianoMarks = { { 0, 500, "C4" }, { 500, 1000, "C#4" }, { 1000, 1500, "E4 G4" } };

std::string Piano(const std::string& extra = "") {
    std::string s = "E_CHOICE_Piano_MIDITrack_APPLYLAST=N";
    if (!extra.empty()) s += "," + extra;
    return s;
}

} // namespace

TEST_CASE("Piano True Piano lights the pressed keys from the timing track", "[effects][audiofx]") {
    AudioFixture fx;
    fx.AddTiming("N", kPianoMarks);
    // 60..72 has 8 white keys, so each is 2 pixels wide on a 16 wide buffer
    Frames fr = fx.Render("Piano", Piano(), kPianoPal, 16, 10, { 2, 12, 22, 32 });

    // C4 down: the C key (x 0) is the "white key down" colour, E (x 4) is not
    CHECK(Is(fr.at(0, 0, 0), 0, 0, 255));
    CHECK(Is(fr.at(0, 4, 0), 255, 0, 0));
    CHECK(CountColour(fr, 0, 255, 255, 0) == 0);

    // C#4 down: a black key turns yellow and C is back up
    CHECK(Is(fr.at(1, 0, 0), 255, 0, 0));
    CHECK(Is(fr.at(1, 2, 7), 255, 255, 0));
    CHECK(CountColour(fr, 1, 0, 0, 255) == 0);

    // E4 and G4 together
    CHECK(Is(fr.at(2, 4, 0), 0, 0, 255));
    CHECK(Is(fr.at(2, 8, 0), 0, 0, 255));
    CHECK(Is(fr.at(2, 0, 0), 255, 0, 0));
    CHECK(Is(fr.at(2, 6, 0), 255, 0, 0));

    // no note: only the key-up colours
    CHECK(CountColour(fr, 3, 0, 0, 255) == 0);
    CHECK(CountColour(fr, 3, 255, 255, 0) == 0);
    CHECK(CountColour(fr, 3, 255, 0, 0) > 0);
    CHECK(CountColour(fr, 3, 0, 255, 0) > 0);
}

TEST_CASE("Piano without sharps folds a sharp onto the white key below and draws no black keys", "[effects][audiofx]") {
    AudioFixture fx;
    fx.AddTiming("N", kPianoMarks);
    Frames fr = fx.Render("Piano", Piano("E_CHECKBOX_Piano_ShowSharps=0"), kPianoPal, 16, 10, { 12, 32 });
    CHECK(Is(fr.at(0, 0, 0), 0, 0, 255)); // C#4 shows as C4
    CHECK(Is(fr.at(0, 0, 9), 0, 0, 255));
    for (size_t f = 0; f < fr.f.size(); ++f) {
        CHECK(CountColour(fr, f, 0, 255, 0) == 0);
        CHECK(CountColour(fr, f, 255, 255, 0) == 0);
    }
}

TEST_CASE("Piano Bars gives every key, sharp or not, its own column", "[effects][audiofx]") {
    AudioFixture fx;
    fx.AddTiming("N", kPianoMarks);
    // 13 keys x 2 pixels
    Frames fr = fx.Render("Piano", Piano("E_CHOICE_Piano_Type=Bars"), kPianoPal, 26, 9, { 2, 12, 32 });
    CHECK(Is(fr.at(0, 0, 0), 0, 0, 255));
    CHECK(Is(fr.at(0, 1, 0), 0, 0, 255));
    CHECK(Is(fr.at(0, 4, 0), 255, 0, 0));
    // white keys are 2/3 high, black keys sit from 1/3 up to the top
    CHECK(Is(fr.at(0, 0, 6), 0, 0, 255));
    CHECK(!Lit(fr.at(0, 0, 8)));
    CHECK(Is(fr.at(1, 2, 8), 255, 255, 0));
    CHECK(!Lit(fr.at(1, 2, 0)));
    CHECK(Is(fr.at(2, 2, 8), 0, 255, 0));

    // without sharps the white keys are full height and no black key is drawn
    Frames ns = fx.Render("Piano", Piano("E_CHOICE_Piano_Type=Bars,E_CHECKBOX_Piano_ShowSharps=0"), kPianoPal, 14, 9, { 2 });
    CHECK(Is(ns.at(0, 0, 8), 0, 0, 255));
    CHECK(CountColour(ns, 0, 0, 255, 0) == 0);
}

TEST_CASE("Piano scale and x offset move the keyboard", "[effects][audiofx]") {
    AudioFixture fx;
    fx.AddTiming("N", kPianoMarks);
    Frames full = fx.Render("Piano", Piano(), kPianoPal, 16, 10, { 2 });
    Frames half = fx.Render("Piano", Piano("E_SLIDER_Piano_Scale=50"), kPianoPal, 16, 10, { 2 });
    CHECK(Lit(full.at(0, 0, 9)));
    CHECK(Lit(half.at(0, 0, 4)));
    CHECK(!Lit(half.at(0, 0, 9)));
    CHECK(half.lit(0) < full.lit(0));

    Frames off = fx.Render("Piano", Piano("E_SLIDER_Piano_XOffset=25"), kPianoPal, 16, 10, { 2 });
    // 25% of 16 = 4 pixels to the right: C is now at x 4
    CHECK(!Lit(off.at(0, 0, 0)));
    CHECK(!Lit(off.at(0, 3, 0)));
    CHECK(Is(off.at(0, 4, 0), 0, 0, 255));
}

TEST_CASE("Piano fade notes blends the pressed key back to the key colour over the note", "[effects][audiofx]") {
    AudioFixture fx;
    fx.AddTiming("N", kPianoMarks);
    Frames fr = fx.Render("Piano", Piano("E_CHECKBOX_Piano_FadeNotes=1"), kPianoPal, 16, 10, { 0, 4, 8 });
    const xlColor& a = fr.at(0, 0, 0);
    const xlColor& b = fr.at(1, 0, 0);
    const xlColor& c = fr.at(2, 0, 0);
    CHECK(a.blue > b.blue);
    CHECK(b.blue > c.blue);
    CHECK(a.red < c.red);
    CHECK(a.green == 0);
    // and the black-key fade path
    Frames sh = fx.Render("Piano", Piano("E_CHECKBOX_Piano_FadeNotes=1,E_CHOICE_Piano_Type=Bars"), kPianoPal, 26, 9, { 10, 18 });
    CHECK(sh.at(0, 2, 8).red > 0);
    CHECK(sh.at(0, 2, 8).green > 0);
    CHECK(sh.at(1, 2, 8).red < sh.at(0, 2, 8).red);
}

TEST_CASE("Piano reads note names, flats and MIDI numbers from the labels", "[effects][audiofx]") {
    AudioFixture fx;
    // A4 = 69, Bb4 = 70, 64 = E4, lower case and separators are accepted
    fx.AddTiming("N", { { 0, 500, "a4" }, { 500, 1000, "Bb4" }, { 1000, 1500, "64;72" } });
    Frames fr = fx.Render("Piano", Piano("E_CHOICE_Piano_Type=Bars"), kPianoPal, 26, 9, { 2, 12, 22 });
    CHECK(Is(fr.at(0, 18, 0), 0, 0, 255)); // key 69 is column 9
    CHECK(CountColour(fr, 0, 0, 0, 255) == 2 * 7);
    CHECK(Is(fr.at(1, 20, 8), 255, 255, 0)); // key 70 is column 10, a black key
    CHECK(Is(fr.at(2, 8, 0), 0, 0, 255));    // 64
    CHECK(Is(fr.at(2, 24, 0), 0, 0, 255));   // 72
}

TEST_CASE("Piano clamps the key range to the buffer and ignores an inverted range", "[effects][audiofx]") {
    AudioFixture fx;
    fx.AddTiming("N", kPianoMarks);
    // end below start collapses to one key (C4)
    Frames one = fx.Render("Piano", Piano("E_SPINCTRL_Piano_StartMIDI=60,E_SPINCTRL_Piano_EndMIDI=50,E_CHOICE_Piano_Type=Bars"), kPianoPal, 4, 4, { 2, 32 });
    CHECK(Is(one.at(0, 0, 0), 0, 0, 255));
    CHECK(Is(one.at(0, 3, 0), 0, 0, 255));
    CHECK(Is(one.at(1, 3, 0), 255, 0, 0));
    // a wide range on a narrow buffer is cut down to the buffer width
    Frames narrow = fx.Render("Piano", Piano("E_SPINCTRL_Piano_StartMIDI=0,E_SPINCTRL_Piano_EndMIDI=127"), kPianoPal, 8, 4, { 2 });
    CHECK(narrow.lit(0) > 0);
    CHECK(CountColour(narrow, 0, 0, 0, 255) == 0); // C4 is outside the first 8 keys
}

TEST_CASE("Piano with a missing timing track draws the idle keyboard", "[effects][audiofx]") {
    AudioFixture fx;
    Frames fr = fx.Render("Piano", Piano(), kPianoPal, 16, 10, { 2 });
    CHECK(CountColour(fr, 0, 255, 0, 0) > 0);
    CHECK(CountColour(fr, 0, 0, 0, 255) == 0);
    // a palette with fewer colours falls back to built-in key colours
    Frames one = fx.Render("Piano", Piano(), kRed, 16, 10, { 2 });
    CHECK(one.lit(0) > 0);
}

TEST_CASE("Piano settings check and timing track rename", "[effects][audiofx]") {
    AudioFixture fx;
    fx.AddTiming("N", kPianoMarks);
    fx.AddTiming("Empty", {});
    RenderableEffect* piano = fx.ctx.effectManager.GetEffect("Piano");
    REQUIRE(piano != nullptr);

    auto check = [&](const std::string& settings) {
        Effect* e = fx.layer->AddEffect(0, "Piano", settings, kPianoPal, 0, 1000, 0, false);
        auto res = piano->CheckEffectSettings(e->GetSettings(), nullptr, fx.model.get(), e, false);
        std::string all;
        for (const auto& s : res) all += s + "\n";
        return all;
    };
    CHECK(check("").find("needs a timing track") != std::string::npos);
    CHECK(check("E_CHOICE_Piano_MIDITrack_APPLYLAST=Empty").find("has no notes") != std::string::npos);
    std::string ok = check("E_CHOICE_Piano_MIDITrack_APPLYLAST=N");
    CHECK(ok.find("needs a timing track") == std::string::npos);
    CHECK(ok.find("has no notes") == std::string::npos);

    Effect* e = fx.layer->AddEffect(0, "Piano", "E_CHOICE_Piano_MIDITrack_APPLYLAST=N", kPianoPal, 0, 1000, 0, false);
    piano->RenameTimingTrack("Other", "X", e);
    CHECK(e->GetSettings().Get("E_CHOICE_Piano_MIDITrack_APPLYLAST", "") == "N");
    piano->RenameTimingTrack("N", "Notes", e);
    CHECK(e->GetSettings().Get("E_CHOICE_Piano_MIDITrack_APPLYLAST", "") == "Notes");
}

TEST_CASE("Piano Parse splits tab separated numbers", "[effects][audiofx]") {
    auto v = PianoEffect::Parse("1.5\t2\t-3");
    REQUIRE(v.size() == 3);
    CHECK(v[0] == 1.5f);
    CHECK(v[1] == 2.0f);
    CHECK(v[2] == -3.0f);
    CHECK(PianoEffect::Parse("").empty());
}

// ---------------------------------------------------------------------------
// Music
// ---------------------------------------------------------------------------

namespace {

// Notes 60-80 on 21 columns: one column per note, so the 440 Hz tone (note 69) is column 9.
// A high sensitivity keeps only the notes right around the tone.
std::string Music(const std::string& extra = "") {
    std::string s = "E_SLIDER_Music_Bars=21";
    if (extra.find("Music_Sensitivity") == std::string::npos) s += ",E_SLIDER_Music_Sensitivity=90";
    if (!extra.empty()) s += "," + extra;
    return s;
}

int LowestLit(const Frames& fr, size_t f, int x) {
    for (int y = 0; y < fr.h; ++y) if (Lit(fr.at(f, x, y))) return y;
    return -1;
}
int HighestLit(const Frames& fr, size_t f, int x) {
    for (int y = fr.h - 1; y >= 0; --y) if (Lit(fr.at(f, x, y))) return y;
    return -1;
}

} // namespace

TEST_CASE("Music lights the bar of the note being played, only while it plays", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(SilentThenTone());
    Frames fr = fx.Render("Music Effect", Music("E_CHOICE_Music_Type=On"), kRedBlue, 21, 8, Range(0, 39));
    for (int f = 0; f <= 17; ++f) CHECK(fr.lit(f) == 0);
    CHECK(fr.columnHeight(25, 9) == 8);
    CHECK(fr.columnHeight(30, 9) == 8);
    // far away notes stay dark
    CHECK(!fr.columnLit(25, 0));
    CHECK(!fr.columnLit(25, 20));
    CHECK(RedBlueOnly(fr));

    // lower sensitivity lets more of the spread-out spectrum through
    Frames loose = fx.Render("Music Effect", Music("E_CHOICE_Music_Type=On,E_SLIDER_Music_Sensitivity=50"), kRedBlue, 21, 8, { 25 });
    CHECK(loose.lit(0) > fr.lit(25));
    // scaling each note to its own peak lights every note that ever sounds
    Frames each = fx.Render("Music Effect", Music("E_CHOICE_Music_Type=On,E_SLIDER_Music_Sensitivity=50,E_CHOICE_Music_Scaling=Individual Notes"), kRedBlue, 21, 8, { 25 });
    CHECK(each.lit(0) > loose.lit(0));
    Frames all = fx.Render("Music Effect", Music("E_CHOICE_Music_Type=On,E_CHOICE_Music_Scaling=All Notes"), kRedBlue, 21, 8, { 10, 25 });
    CHECK(all.lit(0) == 0);
    CHECK(all.columnLit(1, 9));
}

TEST_CASE("Music does nothing without audio", "[effects][audiofx]") {
    AudioFixture fx;
    for (const char* t : { "Morph", "Bounce", "Collide", "Separate", "On" }) {
        Frames fr = fx.Render("Music Effect", Music(std::string("E_CHOICE_Music_Type=") + t), kRedBlue, 21, 8, { 0, 25 });
        CHECK(fr.lit(0) == 0);
        CHECK(fr.lit(1) == 0);
    }
    RenderableEffect* music = fx.ctx.effectManager.GetEffect("Music Effect");
    Effect* e = fx.layer->AddEffect(0, "Music Effect", "", kRedBlue, 0, 1000, 0, false);
    std::string msgs;
    for (const auto& s : music->CheckEffectSettings(e->GetSettings(), nullptr, fx.model.get(), e, false)) msgs += s;
    CHECK(msgs.find("no music") != std::string::npos);
    fx.LoadAudio(SilentThenTone());
    msgs.clear();
    for (const auto& s : music->CheckEffectSettings(e->GetSettings(), fx.audio.get(), fx.model.get(), e, false)) msgs += s;
    CHECK(msgs.find("no music") == std::string::npos);
}

TEST_CASE("Music Morph rises through the note, Bounce mirrors it, Collide is symmetric", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(SilentThenTone());
    Frames morph = fx.Render("Music Effect", Music("E_CHOICE_Music_Type=Morph"), kRedBlue, 21, 8, Range(0, 39));
    // early in the note the band sits at the bottom, late in the note it reaches the top
    CHECK(LowestLit(morph, 21, 9) == 0);
    CHECK(HighestLit(morph, 21, 9) < 7);
    CHECK(LowestLit(morph, 35, 9) > 0);
    CHECK(HighestLit(morph, 35, 9) == 7);
    int prevLow = -1;
    for (int f = 21; f <= 37; ++f) {
        int lo = LowestLit(morph, f, 9);
        if (lo < 0) continue;
        CHECK(lo >= prevLow);
        prevLow = lo;
    }

    // the first event of Bounce runs downwards: an exact vertical mirror of Morph
    Frames bounce = fx.Render("Music Effect", Music("E_CHOICE_Music_Type=Bounce"), kRedBlue, 21, 8, Range(0, 39));
    for (int f : { 21, 28, 35 }) {
        for (int y = 0; y < 8; ++y) {
            INFO("frame " << f << " y " << y);
            CHECK(Lit(bounce.at(f, 9, y)) == Lit(morph.at(f, 9, 7 - y)));
        }
    }

    for (const char* t : { "Collide", "Separate" }) {
        INFO(t);
        Frames c = fx.Render("Music Effect", Music(std::string("E_CHOICE_Music_Type=") + t), kRedBlue, 21, 8, Range(0, 39));
        CHECK(c.energy(20, 39) > 0);
        CHECK(c.lit(10) == 0);
        for (int f = 20; f <= 39; ++f)
            for (int y = 0; y < 4; ++y) CHECK(Lit(c.at(f, 9, y)) == Lit(c.at(f, 9, 7 - y)));
    }
    // Collide moves from the outside in, Separate from the middle out
    Frames in = fx.Render("Music Effect", Music("E_CHOICE_Music_Type=Collide"), kRedBlue, 21, 8, { 22 });
    Frames out = fx.Render("Music Effect", Music("E_CHOICE_Music_Type=Separate"), kRedBlue, 21, 8, { 22 });
    CHECK(Lit(in.at(0, 9, 0)));
    CHECK(!Lit(in.at(0, 9, 3)));
    CHECK(Lit(out.at(0, 9, 3)));
    CHECK(!Lit(out.at(0, 9, 0)));
}

TEST_CASE("Music colour treatments and fade", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(SilentThenTone());
    auto pure = [](const Frames& fr) {
        int mixed = 0;
        for (const auto& px : fr.f)
            for (const auto& c : px)
                if (Lit(c) && !Is(c, 255, 0, 0) && !Is(c, 0, 0, 255)) ++mixed;
        return mixed;
    };
    for (const char* t : { "Morph", "Collide", "On" }) {
        INFO(t);
        std::string base = std::string("E_CHOICE_Music_Type=") + t;
        Frames distinct = fx.Render("Music Effect", Music(base + ",E_CHOICE_Music_Colour=Distinct"), kRedBlue, 21, 8, Range(20, 38));
        CHECK(distinct.energy(0, 18) > 0);
        CHECK(pure(distinct) == 0);
        bool red = false, blue = false;
        for (const auto& px : distinct.f)
            for (const auto& c : px) {
                red |= Is(c, 255, 0, 0);
                blue |= Is(c, 0, 0, 255);
            }
        CHECK(red);
        CHECK(blue);
        Frames blend = fx.Render("Music Effect", Music(base + ",E_CHOICE_Music_Colour=Blend"), kRedBlue, 21, 8, Range(20, 38));
        CHECK(pure(blend) > 0);
        CHECK(RedBlueOnly(blend));
        // cycle: one colour per event, the first event takes the first colour
        Frames cycle = fx.Render("Music Effect", Music(base + ",E_CHOICE_Music_Colour=Cycle"), kRedBlue, 21, 8, Range(20, 38));
        for (int y = 0; y < 8; ++y)
            if (Lit(cycle.at(5, 9, y))) CHECK(Is(cycle.at(5, 9, y), 255, 0, 0));
        Frames fade = fx.Render("Music Effect", Music(base + ",E_CHECKBOX_Music_Fade=1"), kRedBlue, 21, 8, Range(20, 38));
        bool partial = false;
        for (const auto& px : fade.f)
            for (const auto& c : px) partial |= Lit(c) && c.alpha < 255;
        CHECK(partial);
    }
}

TEST_CASE("Music scale widens the bars, offset shifts them", "[effects][audiofx]") {
    AudioFixture fx;
    fx.LoadAudio(SilentThenTone());
    // 42 columns for 21 notes: scaled, note 69 covers columns 18-20
    Frames narrow = fx.Render("Music Effect", Music("E_CHOICE_Music_Type=On"), kRedBlue, 42, 4, { 25 });
    Frames wide = fx.Render("Music Effect", Music("E_CHOICE_Music_Type=On,E_CHECKBOX_Music_Scale=1"), kRedBlue, 42, 4, { 25 });
    CHECK(narrow.columnLit(0, 9));
    CHECK(!narrow.columnLit(0, 19));
    CHECK(wide.columnLit(0, 19));
    CHECK(wide.lit(0) > narrow.lit(0));
    for (int x = 21; x < 42; ++x) CHECK(!narrow.columnLit(0, x));

    // an offset of 10 columns moves note 69 to column 19, and nothing is drawn left of the offset
    Frames off = fx.Render("Music Effect", Music("E_CHOICE_Music_Type=On,E_SLIDER_Music_Offset=10"), kRedBlue, 21, 4, { 25 });
    CHECK(off.columnLit(0, 19));
    for (int x = 0; x < 10; ++x) CHECK(!off.columnLit(0, x));

    // reversed note range is swapped; logarithmic bars still find the tone
    Frames rev = fx.Render("Music Effect", Music("E_CHOICE_Music_Type=On,E_SLIDER_Music_StartNote=80,E_SLIDER_Music_EndNote=60"), kRedBlue, 21, 4, { 25 });
    CHECK(rev.columnLit(0, 9));
    Frames lg = fx.Render("Music Effect", "E_CHOICE_Music_Type=On,E_CHECKBOX_Music_LogarithmicX=1,E_SLIDER_Music_StartNote=0,E_SLIDER_Music_EndNote=127,E_SLIDER_Music_Bars=20,E_SLIDER_Music_Sensitivity=50", kRedBlue, 20, 4, { 10, 25 });
    CHECK(lg.lit(0) == 0);
    CHECK(lg.lit(1) > 0);
}

