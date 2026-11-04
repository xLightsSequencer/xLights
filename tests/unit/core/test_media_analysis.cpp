#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "media/AudioManager.h"
#include "media/ChordDetector.h"
#include "media/OnsetDetector.h"
#include "media/PitchDetector.h"
#include "media/Spectrogram.h"
#include "media/TempoDetector.h"
#include "support/TempDir.h"

using Catch::Matchers::WithinAbs;

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr int kSR = 44100;

void Put16(std::string& s, uint16_t v) {
    s.push_back(char(v & 0xFF));
    s.push_back(char((v >> 8) & 0xFF));
}
void Put32(std::string& s, uint32_t v) {
    for (int i = 0; i < 4; i++) s.push_back(char((v >> (8 * i)) & 0xFF));
}

// samples are interleaved 16-bit PCM
std::string MakeWav(int sampleRate, int channels, const std::vector<int16_t>& samples) {
    std::string s;
    uint32_t dataBytes = uint32_t(samples.size() * 2);
    s += "RIFF";
    Put32(s, 36 + dataBytes);
    s += "WAVEfmt ";
    Put32(s, 16);
    Put16(s, 1);
    Put16(s, uint16_t(channels));
    Put32(s, uint32_t(sampleRate));
    Put32(s, uint32_t(sampleRate * channels * 2));
    Put16(s, uint16_t(channels * 2));
    Put16(s, 16);
    s += "data";
    Put32(s, dataBytes);
    for (int16_t v : samples) Put16(s, uint16_t(v));
    return s;
}

std::vector<int16_t> Sine(int sampleRate, double freq, double seconds, double amp = 0.5) {
    size_t n = size_t(seconds * sampleRate);
    std::vector<int16_t> v(n);
    for (size_t i = 0; i < n; i++) {
        v[i] = int16_t(std::lround(amp * 32767.0 * std::sin(2.0 * kPi * freq * double(i) / sampleRate)));
    }
    return v;
}

std::vector<int16_t> Interleave(const std::vector<int16_t>& l, const std::vector<int16_t>& r) {
    std::vector<int16_t> o(l.size() * 2);
    for (size_t i = 0; i < l.size(); i++) {
        o[2 * i] = l[i];
        o[2 * i + 1] = r[i];
    }
    return o;
}


struct Lcg {
    uint32_t s;
    explicit Lcg(uint32_t seed) : s(seed) {}
    double next() {
        s = s * 1664525u + 1013904223u;
        return double(s >> 8) / double(1u << 24) * 2.0 - 1.0;
    }
};

int16_t ToPcm(double v) {
    v = std::max(-1.0, std::min(1.0, v));
    return int16_t(std::lround(v * 32767.0));
}

std::vector<int16_t> Silence(double seconds, int sr = kSR) {
    return std::vector<int16_t>(size_t(seconds * sr), 0);
}

std::vector<int16_t> Noise(double seconds, double amp, uint32_t seed = 1, int sr = kSR) {
    Lcg g(seed);
    std::vector<int16_t> v(size_t(seconds * sr));
    for (auto& x : v) x = ToPcm(amp * g.next());
    return v;
}

std::vector<int16_t> Chord(const std::vector<double>& freqs, double seconds, double amp = 0.6, int sr = kSR) {
    size_t n = size_t(seconds * sr);
    std::vector<int16_t> v(n);
    for (size_t i = 0; i < n; i++) {
        double acc = 0;
        for (double f : freqs) acc += std::sin(2.0 * kPi * f * double(i) / sr);
        v[i] = ToPcm(amp * acc / double(freqs.size()));
    }
    return v;
}

std::vector<int16_t> Append(std::vector<int16_t> a, const std::vector<int16_t>& b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

// Noise burst with exponential decay at each click time (seconds)
std::vector<int16_t> Clicks(const std::vector<double>& times, double seconds, int sr = kSR) {
    std::vector<double> acc(size_t(seconds * sr), 0.0);
    Lcg g(77);
    for (double t : times) {
        size_t start = size_t(t * sr);
        size_t len = size_t(0.03 * sr);
        for (size_t i = 0; i < len && start + i < acc.size(); i++) {
            acc[start + i] += 0.9 * g.next() * std::exp(-double(i) / (0.004 * sr));
        }
    }
    std::vector<int16_t> v(acc.size());
    for (size_t i = 0; i < acc.size(); i++) v[i] = ToPcm(acc[i]);
    return v;
}

std::vector<double> RegularTimes(double bpm, double seconds, double first = 0.25) {
    std::vector<double> t;
    double period = 60.0 / bpm;
    for (double x = first; x < seconds - 0.1; x += period) t.push_back(x);
    return t;
}

struct Loaded {
    TempDir td;
    std::unique_ptr<AudioManager> am;
    Loaded(const std::vector<int16_t>& mono, int sr = kSR, int channels = 1) {
        am = std::make_unique<AudioManager>(td.Write("s.wav", MakeWav(sr, channels, mono)), 50);
    }
    AudioManager* get() { return am.get(); }
};

double HzOfBin(int bin, int frameSize, long rate) { return double(bin) * double(rate) / double(frameSize); }

int ArgMax(const float* p, int n) {
    int best = 0;
    for (int i = 1; i < n; i++) if (p[i] > p[best]) best = i;
    return best;
}

} // namespace

TEST_CASE("Spectrogram of a sine peaks at the expected bin", "[media][spectrogram]") {
    for (double f : { 440.0, 1000.0, 3000.0, 8000.0 }) {
        Loaded L(Sine(kSR, f, 1.0, 0.8));
        REQUIRE(L.get()->IsOk());
        Spectrogram sg = ComputeSpectrogram(L.get());
        REQUIRE(sg.frames > 10);
        INFO("freq " << f);
        int expected = int(std::lround(f * sg.frameSize / double(sg.sampleRate)));
        int mid = sg.frames / 2;
        int peak = ArgMax(&sg.magnitudes[size_t(mid) * sg.bins], sg.bins);
        CHECK(std::abs(peak - expected) <= 1);
    }
}

TEST_CASE("Spectrogram dimensions follow frame and hop size", "[media][spectrogram]") {
    Loaded L(Sine(kSR, 500.0, 1.0));
    REQUIRE(L.get()->IsOk());
    SpectrogramOptions o;
    o.frameSize = 2048;
    o.hopSize = 256;
    Spectrogram sg = ComputeSpectrogram(L.get(), o);
    long track = L.get()->GetTrackSize();
    CHECK(sg.bins == 1025);
    CHECK(sg.frameSize == 2048);
    CHECK(sg.frames == int((track - 2048) / 256) + 1);
    CHECK(sg.magnitudes.size() == size_t(sg.frames) * size_t(sg.bins));
    CHECK_THAT(double(sg.hopMS), WithinAbs(1000.0 * 256.0 / double(L.get()->GetRate()), 1e-3));
    CHECK(sg.sampleRate == L.get()->GetRate());
}

TEST_CASE("Spectrogram peakMag is the maximum magnitude", "[media][spectrogram]") {
    Loaded L(Sine(kSR, 700.0, 0.5, 0.5));
    Spectrogram sg = ComputeSpectrogram(L.get());
    REQUIRE_FALSE(sg.magnitudes.empty());
    float mx = *std::max_element(sg.magnitudes.begin(), sg.magnitudes.end());
    CHECK(sg.peakMag == mx);
    CHECK(sg.peakMag > 10.0f);
}

TEST_CASE("Spectrogram magnitude scales with amplitude", "[media][spectrogram]") {
    Loaded A(Sine(kSR, 1000.0, 0.5, 0.2));
    Loaded B(Sine(kSR, 1000.0, 0.5, 0.4));
    float pa = ComputeSpectrogram(A.get()).peakMag;
    float pb = ComputeSpectrogram(B.get()).peakMag;
    REQUIRE(pa > 0.0f);
    CHECK_THAT(double(pb / pa), WithinAbs(2.0, 0.1));
}

TEST_CASE("Spectrogram of silence is all zero", "[media][spectrogram]") {
    Loaded L(Silence(0.5));
    Spectrogram sg = ComputeSpectrogram(L.get());
    REQUIRE(sg.frames > 0);
    CHECK(sg.peakMag < 1e-3f);
}

TEST_CASE("Spectrogram of audio shorter than a frame is empty", "[media][spectrogram]") {
    Loaded L(Sine(kSR, 440.0, 0.01));
    REQUIRE(L.get()->IsOk());
    Spectrogram sg = ComputeSpectrogram(L.get());
    CHECK(sg.frames == 0);
    CHECK(sg.magnitudes.empty());
}

TEST_CASE("Spectrogram rejects invalid options", "[media][spectrogram]") {
    Loaded L(Sine(kSR, 440.0, 0.5));
    SpectrogramOptions o;
    o.frameSize = 1023;
    CHECK(ComputeSpectrogram(L.get(), o).frames == 0);
    o = SpectrogramOptions{};
    o.frameSize = 16;
    CHECK(ComputeSpectrogram(L.get(), o).frames == 0);
    o = SpectrogramOptions{};
    o.hopSize = 0;
    CHECK(ComputeSpectrogram(L.get(), o).frames == 0);
    o = SpectrogramOptions{};
    o.hopSize = o.frameSize + 1;
    CHECK(ComputeSpectrogram(L.get(), o).frames == 0);
}

TEST_CASE("Spectrogram tolerates null and failed audio", "[media][spectrogram]") {
    CHECK(ComputeSpectrogram(nullptr).frames == 0);
    TempDir td;
    AudioManager bad(td.File("none.wav"), 50);
    CHECK(ComputeSpectrogram(&bad).frames == 0);
}

TEST_CASE("Spectrogram separates two simultaneous tones", "[media][spectrogram]") {
    Loaded L(Chord({ 600.0, 3000.0 }, 0.5, 0.8));
    Spectrogram sg = ComputeSpectrogram(L.get());
    REQUIRE(sg.frames > 4);
    const float* row = &sg.magnitudes[size_t(sg.frames / 2) * sg.bins];
    int b1 = int(std::lround(600.0 * sg.frameSize / double(sg.sampleRate)));
    int b2 = int(std::lround(3000.0 * sg.frameSize / double(sg.sampleRate)));
    float between = row[(b1 + b2) / 2];
    CHECK(row[b1] > between * 20.0f);
    CHECK(row[b2] > between * 20.0f);
}

TEST_CASE("Spectrogram follows a frequency change over time", "[media][spectrogram]") {
    Loaded L(Append(Sine(kSR, 500.0, 0.5, 0.7), Sine(kSR, 2000.0, 0.5, 0.7)));
    Spectrogram sg = ComputeSpectrogram(L.get());
    REQUIRE(sg.frames > 20);
    int early = ArgMax(&sg.magnitudes[size_t(sg.frames / 5) * sg.bins], sg.bins);
    int late = ArgMax(&sg.magnitudes[size_t(sg.frames * 4 / 5) * sg.bins], sg.bins);
    CHECK_THAT(HzOfBin(early, sg.frameSize, sg.sampleRate), WithinAbs(500.0, 50.0));
    CHECK_THAT(HzOfBin(late, sg.frameSize, sg.sampleRate), WithinAbs(2000.0, 50.0));
}

TEST_CASE("Spectrogram of stereo input analyses the left channel", "[media][spectrogram]") {
    auto l = Sine(kSR, 1000.0, 0.5, 0.8);
    std::vector<int16_t> r = Sine(kSR, 4000.0, 0.5, 0.8);
    Loaded L(Interleave(l, r), kSR, 2);
    Spectrogram sg = ComputeSpectrogram(L.get());
    REQUIRE(sg.frames > 4);
    int peak = ArgMax(&sg.magnitudes[size_t(sg.frames / 2) * sg.bins], sg.bins);
    CHECK_THAT(HzOfBin(peak, sg.frameSize, sg.sampleRate), WithinAbs(1000.0, 50.0));
}

TEST_CASE("Spectrogram of white noise is broadband", "[media][spectrogram]") {
    Loaded L(Noise(0.5, 0.5));
    Spectrogram sg = ComputeSpectrogram(L.get());
    REQUIRE(sg.frames > 4);
    const float* row = &sg.magnitudes[size_t(sg.frames / 2) * sg.bins];
    double lo = 0, hi = 0;
    for (int k = 20; k < 200; k++) lo += row[k];
    for (int k = 300; k < 480; k++) hi += row[k];
    CHECK(lo / hi > 0.5);
    CHECK(lo / hi < 2.0);
}

TEST_CASE("RenderSpectrogramBGRA sizes the buffer and sets opaque alpha", "[media][spectrogram]") {
    Loaded L(Sine(kSR, 1000.0, 1.0, 0.8));
    Spectrogram sg = ComputeSpectrogram(L.get());
    std::vector<uint8_t> img;
    RenderSpectrogramBGRA(sg, 0, 1000, 64, 32, img);
    REQUIRE(img.size() == 64u * 32u * 4u);
    for (size_t i = 3; i < img.size(); i += 4) {
        REQUIRE(img[i] == 255);
    }
}

TEST_CASE("RenderSpectrogramBGRA puts a sine on the matching log-frequency row", "[media][spectrogram]") {
    Loaded L(Sine(kSR, 2000.0, 1.0, 0.8));
    Spectrogram sg = ComputeSpectrogram(L.get());
    const int W = 20, H = 200;
    std::vector<uint8_t> img;
    RenderSpectrogramBGRA(sg, 200, 800, W, H, img);
    int brightestRow = -1;
    int best = -1;
    for (int y = 0; y < H; y++) {
        const uint8_t* px = &img[size_t(y) * W * 4 + size_t(W / 2) * 4];
        int sum = px[0] + px[1] + px[2];
        if (sum > best) { best = sum; brightestRow = y; }
    }
    double nyq = 0.5 * sg.sampleRate;
    double norm = (std::log2(2000.0) - std::log2(40.0)) / (std::log2(nyq) - std::log2(40.0));
    int expectedRow = int((1.0 - norm) * H);
    CHECK(std::abs(brightestRow - expectedRow) <= 4);
    CHECK(best > 600);
}

TEST_CASE("RenderSpectrogramBGRA silence renders black", "[media][spectrogram]") {
    Loaded L(Silence(0.5));
    Spectrogram sg = ComputeSpectrogram(L.get());
    std::vector<uint8_t> img;
    RenderSpectrogramBGRA(sg, 0, 400, 16, 16, img);
    REQUIRE(img.size() == 16u * 16u * 4u);
    for (size_t i = 0; i < img.size(); i += 4) {
        REQUIRE(img[i] == 0);
        REQUIRE(img[i + 1] == 0);
        REQUIRE(img[i + 2] == 0);
    }
}

TEST_CASE("RenderSpectrogramBGRA handles degenerate arguments", "[media][spectrogram]") {
    Spectrogram empty;
    std::vector<uint8_t> img{ 1, 2, 3 };
    RenderSpectrogramBGRA(empty, 0, 100, 4, 4, img);
    REQUIRE(img.size() == 64);
    CHECK(std::all_of(img.begin(), img.end(), [](uint8_t b) { return b == 0; }));

    Loaded L(Sine(kSR, 1000.0, 0.5));
    Spectrogram sg = ComputeSpectrogram(L.get());
    RenderSpectrogramBGRA(sg, 100, 100, 4, 4, img);
    CHECK(std::all_of(img.begin(), img.end(), [](uint8_t b) { return b == 0; }));
    RenderSpectrogramBGRA(sg, 500, 100, 4, 4, img);
    CHECK(std::all_of(img.begin(), img.end(), [](uint8_t b) { return b == 0; }));
    RenderSpectrogramBGRA(sg, 0, 100, 0, 4, img);
    CHECK(img.empty());
}

TEST_CASE("RenderSpectrogramBGRA negative dimensions do not throw", "[media][spectrogram]") {
    Loaded L(Sine(kSR, 1000.0, 0.5));
    Spectrogram sg = ComputeSpectrogram(L.get());
    std::vector<uint8_t> img;
    CHECK_NOTHROW(RenderSpectrogramBGRA(sg, 0, 100, 4, -1, img));
    CHECK(img.empty());
    img.assign(16, 7);
    CHECK_NOTHROW(RenderSpectrogramBGRA(sg, 0, 100, -3, 4, img));
    CHECK(img.empty());
}

TEST_CASE("RenderSpectrogramBGRA accepts ranges outside the track", "[media][spectrogram]") {
    Loaded L(Sine(kSR, 1000.0, 0.5, 0.8));
    Spectrogram sg = ComputeSpectrogram(L.get());
    std::vector<uint8_t> img;
    RenderSpectrogramBGRA(sg, -5000, -1000, 8, 8, img);
    CHECK(img.size() == 8u * 8u * 4u);
    RenderSpectrogramBGRA(sg, 900000, 1000000, 8, 8, img);
    CHECK(img.size() == 8u * 8u * 4u);
}

TEST_CASE("RenderSpectrogramBGRA dB floor changes the dim regions", "[media][spectrogram]") {
    Loaded L(Chord({ 1000.0, 5000.0 }, 0.5, 0.8));
    Spectrogram sg = ComputeSpectrogram(L.get());
    std::vector<uint8_t> a, b;
    RenderSpectrogramBGRA(sg, 0, 400, 8, 64, a, -80.0f);
    RenderSpectrogramBGRA(sg, 0, 400, 8, 64, b, -20.0f);
    long sa = 0, sb = 0;
    for (size_t i = 0; i < a.size(); i += 4) { sa += a[i] + a[i + 1] + a[i + 2]; sb += b[i] + b[i + 1] + b[i + 2]; }
    CHECK(sa > sb);
}

TEST_CASE("Tempo detector finds click tracks at several BPM", "[media][tempo]") {
    for (double bpm : { 90.0, 100.0, 120.0, 140.0 }) {
        double secs = 16.0;
        Loaded L(Clicks(RegularTimes(bpm, secs), secs));
        REQUIRE(L.get()->IsOk());
        TempoResult r = DetectTempo(L.get());
        INFO("bpm " << bpm << " detected " << r.bpm);
        CHECK_THAT(double(r.bpm), WithinAbs(bpm, 3.0));
        CHECK(r.confidence > 0.2f);
    }
}

TEST_CASE("Tempo detector finds 150 BPM or its half-time", "[media][tempo]") {
    double secs = 16.0;
    Loaded L(Clicks(RegularTimes(150.0, secs), secs));
    double bpm = double(DetectTempo(L.get()).bpm);
    bool ok = std::fabs(bpm - 150.0) <= 3.0 || std::fabs(bpm - 75.0) <= 2.0;
    INFO("detected " << bpm);
    CHECK(ok);
}

TEST_CASE("Tempo detector beat positions are evenly spaced", "[media][tempo]") {
    double secs = 12.0;
    Loaded L(Clicks(RegularTimes(120.0, secs), secs));
    TempoResult r = DetectTempo(L.get());
    REQUIRE(r.beatMS.size() > 10);
    for (size_t i = 1; i < r.beatMS.size(); i++) {
        REQUIRE(r.beatMS[i] > r.beatMS[i - 1]);
        CHECK(std::labs((r.beatMS[i] - r.beatMS[i - 1]) - 500) <= 15);
    }
    CHECK(r.beatMS.back() < L.get()->LengthMS());
    CHECK(r.beatMS.front() >= 0);
}

TEST_CASE("Tempo detector beat phase aligns with the clicks", "[media][tempo]") {
    double secs = 12.0;
    Loaded L(Clicks(RegularTimes(100.0, secs, 0.3), secs));
    TempoResult r = DetectTempo(L.get());
    REQUIRE_FALSE(r.beatMS.empty());
    double period = 600.0;
    double phase = std::fmod(double(r.beatMS.front()) - 300.0, period);
    if (phase < 0) phase += period;
    double dist = std::min(phase, period - phase);
    CHECK(dist < 60.0);
}

TEST_CASE("Tempo detector on silence stays finite", "[media][tempo]") {
    Loaded L(Silence(10.0));
    TempoResult r = DetectTempo(L.get());
    CHECK(r.confidence >= 0.0f);
    CHECK(r.confidence <= 1.0f);
    CHECK(r.bpm >= 0.0f);
}

TEST_CASE("Tempo detector returns zero for silence", "[media][tempo]") {
    Loaded L(Silence(10.0));
    TempoResult r = DetectTempo(L.get());
    CHECK(r.bpm == 0.0f);
    CHECK(r.confidence == 0.0f);
    CHECK(r.beatMS.empty());
}

TEST_CASE("Tempo detector returns zero for too-short audio", "[media][tempo]") {
    Loaded L(Clicks({ 0.1, 0.4 }, 0.6));
    TempoResult r = DetectTempo(L.get());
    CHECK(r.bpm == 0.0f);
    CHECK(r.beatMS.empty());
}

TEST_CASE("Tempo detector tolerates null and failed audio", "[media][tempo]") {
    CHECK(DetectTempo(nullptr).bpm == 0.0f);
    TempDir td;
    AudioManager bad(td.File("none.wav"), 50);
    CHECK(DetectTempo(&bad).bpm == 0.0f);
}

TEST_CASE("Tempo detector on white noise stays within the search range", "[media][tempo]") {
    Loaded L(Noise(10.0, 0.5));
    TempoResult r = DetectTempo(L.get());
    if (r.bpm != 0.0f) {
        CHECK(r.bpm >= 55.0f);
        CHECK(r.bpm <= 310.0f);
    }
    CHECK(r.confidence >= 0.0f);
    CHECK(r.confidence <= 1.0f);
}

TEST_CASE("Tempo detector respects a narrowed BPM window", "[media][tempo]") {
    double secs = 16.0;
    Loaded L(Clicks(RegularTimes(120.0, secs), secs));
    TempoDetectorOptions o;
    o.minBPM = 100.0f;
    o.maxBPM = 140.0f;
    TempoResult r = DetectTempo(L.get(), o);
    CHECK_THAT(double(r.bpm), WithinAbs(120.0, 3.0));
    o.minBPM = 100.0f;
    o.maxBPM = 101.0f;
    r = DetectTempo(L.get(), o);
    CHECK(r.bpm == 0.0f);
}

TEST_CASE("Tempo detector confidence is lower for noise than for a click track", "[media][tempo]") {
    double secs = 16.0;
    Loaded C(Clicks(RegularTimes(120.0, secs), secs));
    Loaded N(Noise(secs, 0.5, 9));
    CHECK(DetectTempo(C.get()).confidence > DetectTempo(N.get()).confidence);
}

TEST_CASE("Tempo detector works on stereo input", "[media][tempo]") {
    double secs = 12.0;
    auto m = Clicks(RegularTimes(110.0, secs), secs);
    Loaded L(Interleave(m, m), kSR, 2);
    CHECK_THAT(double(DetectTempo(L.get()).bpm), WithinAbs(110.0, 3.0));
}

TEST_CASE("Tempo detector works on a low-rate source", "[media][tempo]") {
    double secs = 12.0;
    Loaded L(Clicks(RegularTimes(120.0, secs, 0.25), secs, 22050), 22050);
    REQUIRE(L.get()->IsOk());
    CHECK_THAT(double(DetectTempo(L.get()).bpm), WithinAbs(120.0, 3.0));
}

TEST_CASE("Onset detector finds each click near its time", "[media][onset]") {
    std::vector<double> times = { 0.5, 1.0, 1.5, 2.2, 3.0, 3.4, 4.1 };
    Loaded L(Clicks(times, 5.0));
    REQUIRE(L.get()->IsOk());
    auto on = DetectOnsets(L.get());
    REQUIRE(on.size() == times.size());
    for (size_t i = 0; i < times.size(); i++) {
        INFO("click " << i);
        CHECK(std::labs(on[i] - long(times[i] * 1000)) <= 40);
    }
}

TEST_CASE("Onset detector output is strictly ascending and respects spacing", "[media][onset]") {
    Loaded L(Clicks(RegularTimes(200.0, 6.0), 6.0));
    OnsetDetectorOptions o;
    o.minSpacingMS = 100;
    auto on = DetectOnsets(L.get(), o);
    REQUIRE(on.size() > 5);
    for (size_t i = 1; i < on.size(); i++) {
        CHECK(on[i] - on[i - 1] >= 100);
    }
}

TEST_CASE("Onset detector refractory period suppresses close clicks", "[media][onset]") {
    Loaded L(Clicks({ 1.0, 1.03, 2.0, 2.5 }, 3.5));
    OnsetDetectorOptions o;
    o.minSpacingMS = 200;
    auto on = DetectOnsets(L.get(), o);
    CHECK(on.size() == 3);
}

TEST_CASE("Onset detector returns nothing for silence", "[media][onset]") {
    Loaded L(Silence(3.0));
    CHECK(DetectOnsets(L.get()).empty());
}

TEST_CASE("Onset detector on a steady tone stays ordered and bounded", "[media][onset]") {
    Loaded L(Sine(kSR, 440.0, 3.0, 0.5));
    auto on = DetectOnsets(L.get());
    for (size_t i = 0; i < on.size(); i++) {
        CHECK(on[i] >= 0);
        CHECK(on[i] <= L.get()->LengthMS());
        if (i > 0) CHECK(on[i] - on[i - 1] >= 60);
    }
}

TEST_CASE("Onset detector marks a tone starting after silence", "[media][onset]") {
    Loaded L(Append(Silence(1.0), Sine(kSR, 880.0, 1.5, 0.7)));
    auto on = DetectOnsets(L.get());
    REQUIRE_FALSE(on.empty());
    CHECK(std::labs(on.front() - 1000) <= 60);
}

TEST_CASE("Onset detector handles short and invalid inputs", "[media][onset]") {
    CHECK(DetectOnsets(nullptr).empty());
    TempDir td;
    AudioManager bad(td.File("none.wav"), 50);
    CHECK(DetectOnsets(&bad).empty());

    Loaded tiny(Sine(kSR, 440.0, 0.02));
    CHECK(DetectOnsets(tiny.get()).empty());

    Loaded L(Clicks({ 0.5, 1.0 }, 2.0));
    OnsetDetectorOptions o;
    o.frameSize = 1001;
    CHECK(DetectOnsets(L.get(), o).empty());
    o = OnsetDetectorOptions{};
    o.hopSize = 0;
    CHECK(DetectOnsets(L.get(), o).empty());
    o = OnsetDetectorOptions{};
    o.hopSize = o.frameSize * 2;
    CHECK(DetectOnsets(L.get(), o).empty());
    o = OnsetDetectorOptions{};
    o.frameSize = 8;
    CHECK(DetectOnsets(L.get(), o).empty());
}

TEST_CASE("Onset detector sensitivity trades off count", "[media][onset]") {
    Lcg g(5);
    std::vector<double> times = { 0.5, 1.0, 1.5, 2.0, 2.5, 3.0 };
    auto clicks = Clicks(times, 4.0);
    auto noise = Noise(4.0, 0.05, 3);
    for (size_t i = 0; i < clicks.size(); i++) clicks[i] = ToPcm((clicks[i] + noise[i]) / 32767.0);
    (void)g;
    Loaded L(clicks);
    OnsetDetectorOptions lo, hi;
    lo.sensitivity = 1.2f;
    hi.sensitivity = 8.0f;
    CHECK(DetectOnsets(L.get(), lo).size() >= DetectOnsets(L.get(), hi).size());
    CHECK(DetectOnsets(L.get(), hi).size() >= 5);
}

TEST_CASE("Onset envelope metadata and first-frame zero", "[media][onset]") {
    Loaded L(Clicks({ 0.5, 1.5 }, 2.0));
    OnsetEnvelope env = ComputeOnsetEnvelope(L.get());
    REQUIRE_FALSE(env.flux.empty());
    CHECK(env.flux[0] == 0.0f);
    CHECK(env.hopSize == 512);
    CHECK(env.sampleRate == L.get()->GetRate());
    CHECK(std::labs(env.lengthMS - 2000) <= 50);
    long track = L.get()->GetTrackSize();
    CHECK(long(env.flux.size()) == (track - 1024) / 512 + 1);
    for (float f : env.flux) REQUIRE(f >= 0.0f);
}

TEST_CASE("Onset envelope peaks at the click frame", "[media][onset]") {
    Loaded L(Clicks({ 1.0 }, 2.0));
    OnsetEnvelope env = ComputeOnsetEnvelope(L.get());
    int peak = ArgMax(env.flux.data(), int(env.flux.size()));
    double ms = double(peak) * env.hopSize * 1000.0 / env.sampleRate;
    CHECK_THAT(ms, WithinAbs(1000.0, 40.0));
}

TEST_CASE("Onset envelope honours custom hop size", "[media][onset]") {
    Loaded L(Clicks({ 0.5 }, 2.0));
    OnsetDetectorOptions o;
    o.frameSize = 2048;
    o.hopSize = 1024;
    OnsetEnvelope env = ComputeOnsetEnvelope(L.get(), o);
    CHECK(env.hopSize == 1024);
    CHECK(long(env.flux.size()) == (L.get()->GetTrackSize() - 2048) / 1024 + 1);
}

TEST_CASE("Pitch detector recovers pure tones", "[media][pitch]") {
    for (double f : { 150.0, 220.0, 330.0, 440.0, 660.0, 880.0 }) {
        Loaded L(Sine(kSR, f, 1.0, 0.7));
        REQUIRE(L.get()->IsOk());
        PitchContour pc = DetectPitch(L.get());
        REQUIRE(pc.samples.size() > 20);
        INFO("freq " << f);
        int voiced = 0;
        for (size_t i = 5; i + 5 < pc.samples.size(); i++) {
            const auto& s = pc.samples[i];
            if (s.frequency > 0.0f) {
                voiced++;
                CHECK_THAT(double(s.frequency), WithinAbs(f, f * 0.01));
                CHECK(s.confidence > 0.5f);
            }
        }
        CHECK(voiced > int(pc.samples.size()) - 20);
    }
}

TEST_CASE("Pitch detector recovers low pure tones", "[media][pitch]") {
    for (double f : { 100.0, 110.0 }) {
        Loaded L(Sine(kSR, f, 1.0, 0.7));
        PitchContour pc = DetectPitch(L.get());
        REQUIRE(pc.samples.size() > 20);
        INFO("freq " << f);
        CHECK_THAT(double(pc.samples[pc.samples.size() / 2].frequency), WithinAbs(f, f * 0.02));
    }
}

TEST_CASE("Pitch detector recovers pure tones across the supported range", "[media][pitch]") {
    for (double f : { 80.0, 100.0, 110.0, 220.0, 440.0, 880.0 }) {
        Loaded L(Sine(kSR, f, 1.0, 0.7));
        PitchContour pc = DetectPitch(L.get());
        REQUIRE(pc.samples.size() > 20);
        INFO("freq " << f);
        for (size_t i = 5; i + 5 < pc.samples.size(); i += 7) {
            CHECK_THAT(double(pc.samples[i].frequency), WithinAbs(f, f * 0.04));
        }
    }
}

TEST_CASE("Pitch detector tracks a note change", "[media][pitch]") {
    Loaded L(Append(Sine(kSR, 220.0, 0.6, 0.7), Sine(kSR, 330.0, 0.6, 0.7)));
    PitchContour pc = DetectPitch(L.get());
    REQUIRE(pc.samples.size() > 40);
    auto at = [&](long ms) {
        for (const auto& s : pc.samples) if (s.timeMS >= ms) return s;
        return pc.samples.back();
    };
    CHECK_THAT(double(at(300).frequency), WithinAbs(220.0, 3.0));
    CHECK_THAT(double(at(900).frequency), WithinAbs(330.0, 4.0));
}

TEST_CASE("Pitch detector time stamps are ascending at the hop interval", "[media][pitch]") {
    Loaded L(Sine(kSR, 300.0, 1.0));
    PitchContour pc = DetectPitch(L.get());
    REQUIRE(pc.samples.size() > 5);
    CHECK_THAT(double(pc.frameHopMS), WithinAbs(1000.0 * 441.0 / double(L.get()->GetRate()), 1e-3));
    for (size_t i = 1; i < pc.samples.size(); i++) {
        REQUIRE(pc.samples[i].timeMS >= pc.samples[i - 1].timeMS);
        CHECK(std::labs((pc.samples[i].timeMS - pc.samples[i - 1].timeMS) - 10) <= 1);
    }
    CHECK(std::labs(pc.samples.front().timeMS - 23) <= 2);
}

TEST_CASE("Pitch detector marks silence unvoiced", "[media][pitch]") {
    Loaded L(Silence(1.0));
    PitchContour pc = DetectPitch(L.get());
    REQUIRE_FALSE(pc.samples.empty());
    for (const auto& s : pc.samples) {
        REQUIRE(s.frequency == 0.0f);
        REQUIRE(s.confidence == 0.0f);
    }
}

TEST_CASE("Pitch detector yields mostly unvoiced frames for white noise", "[media][pitch]") {
    Loaded L(Noise(1.0, 0.5));
    PitchContour pc = DetectPitch(L.get());
    REQUIRE_FALSE(pc.samples.empty());
    int voiced = 0;
    for (const auto& s : pc.samples) if (s.frequency > 0.0f) voiced++;
    CHECK(voiced < int(pc.samples.size()) / 5);
}

TEST_CASE("Pitch detector ignores tones outside its frequency window", "[media][pitch]") {
    Loaded L(Sine(kSR, 3000.0, 1.0, 0.7));
    PitchContour pc = DetectPitch(L.get());
    for (const auto& s : pc.samples) {
        if (s.frequency > 0.0f) {
            CHECK(s.frequency <= 1250.0f);
            CHECK(s.frequency >= 70.0f);
        }
    }
}

TEST_CASE("Pitch detector honours a custom frequency range", "[media][pitch]") {
    Loaded L(Sine(kSR, 150.0, 1.0, 0.7));
    PitchDetectorOptions o;
    o.minFreqHz = 100.0f;
    o.maxFreqHz = 300.0f;
    PitchContour pc = DetectPitch(L.get(), o);
    REQUIRE(pc.samples.size() > 20);
    const auto& s = pc.samples[pc.samples.size() / 2];
    CHECK_THAT(double(s.frequency), WithinAbs(150.0, 2.0));
}

TEST_CASE("Pitch detector voicing threshold of 1.1 marks everything unvoiced", "[media][pitch]") {
    Loaded L(Sine(kSR, 300.0, 0.5, 0.7));
    PitchDetectorOptions o;
    o.voicingThreshold = 1.1f;
    PitchContour pc = DetectPitch(L.get(), o);
    REQUIRE_FALSE(pc.samples.empty());
    for (const auto& s : pc.samples) {
        REQUIRE(s.frequency == 0.0f);
    }
}

TEST_CASE("Pitch detector rejects invalid options and inputs", "[media][pitch]") {
    CHECK(DetectPitch(nullptr).samples.empty());
    TempDir td;
    AudioManager bad(td.File("none.wav"), 50);
    CHECK(DetectPitch(&bad).samples.empty());

    Loaded L(Sine(kSR, 300.0, 0.5));
    PitchDetectorOptions o;
    o.frameSize = 2047;
    CHECK(DetectPitch(L.get(), o).samples.empty());
    o = PitchDetectorOptions{};
    o.hopSize = 0;
    CHECK(DetectPitch(L.get(), o).samples.empty());
    o = PitchDetectorOptions{};
    o.frameSize = 16;
    CHECK(DetectPitch(L.get(), o).samples.empty());
    o = PitchDetectorOptions{};
    o.minFreqHz = 1000.0f;
    o.maxFreqHz = 1001.0f;
    CHECK(DetectPitch(L.get(), o).samples.empty());

    Loaded tiny(Sine(kSR, 300.0, 0.02));
    CHECK(DetectPitch(tiny.get()).samples.empty());
}

TEST_CASE("Pitch detector confidence is within 0..1", "[media][pitch]") {
    Loaded L(Chord({ 200.0, 310.0, 530.0 }, 1.0, 0.8));
    PitchContour pc = DetectPitch(L.get());
    REQUIRE_FALSE(pc.samples.empty());
    for (const auto& s : pc.samples) {
        REQUIRE(s.confidence >= 0.0f);
        REQUIRE(s.confidence <= 1.0f);
    }
}

TEST_CASE("Pitch detector finds the fundamental of a harmonic-rich tone", "[media][pitch]") {
    size_t n = size_t(kSR);
    std::vector<int16_t> v(n);
    for (size_t i = 0; i < n; i++) {
        double t = double(i) / kSR;
        double s = std::sin(2 * kPi * 200 * t) + 0.6 * std::sin(2 * kPi * 400 * t) + 0.4 * std::sin(2 * kPi * 600 * t);
        v[i] = ToPcm(0.4 * s);
    }
    Loaded L(v);
    PitchContour pc = DetectPitch(L.get());
    REQUIRE(pc.samples.size() > 20);
    const auto& s = pc.samples[pc.samples.size() / 2];
    CHECK_THAT(double(s.frequency), WithinAbs(200.0, 3.0));
}

TEST_CASE("Pitch detector handles a low-rate source", "[media][pitch]") {
    Loaded L(Sine(8000, 250.0, 1.0, 0.7), 8000);
    REQUIRE(L.get()->IsOk());
    PitchContour pc = DetectPitch(L.get());
    REQUIRE(pc.samples.size() > 20);
    CHECK_THAT(double(pc.samples[pc.samples.size() / 2].frequency), WithinAbs(250.0, 3.0));
}

TEST_CASE("Chord detector names simple major triads", "[media][chord]") {
    struct Case { const char* name; std::vector<double> f; };
    std::vector<Case> cases = {
        { "C", { 261.63, 329.63, 392.00 } },
        { "G", { 196.00, 246.94, 293.66 } },
        { "F", { 174.61, 220.00, 261.63 } },
        { "D", { 293.66, 369.99, 440.00 } },
    };
    for (const auto& c : cases) {
        Loaded L(Chord(c.f, 2.0, 0.8));
        REQUIRE(L.get()->IsOk());
        HarmonyAnalysis h = DetectChords(L.get());
        INFO("expect " << c.name);
        REQUIRE_FALSE(h.chords.empty());
        CHECK(h.chords.front().name == c.name);
    }
}

TEST_CASE("Chord detector names simple minor triads", "[media][chord]") {
    struct Case { const char* name; std::vector<double> f; };
    std::vector<Case> cases = {
        { "Am", { 220.00, 261.63, 329.63 } },
        { "Em", { 164.81, 196.00, 246.94 } },
        { "Dm", { 293.66, 349.23, 440.00 } },
    };
    for (const auto& c : cases) {
        Loaded L(Chord(c.f, 2.0, 0.8));
        HarmonyAnalysis h = DetectChords(L.get());
        INFO("expect " << c.name);
        REQUIRE_FALSE(h.chords.empty());
        CHECK(h.chords.front().name == c.name);
    }
}

TEST_CASE("Chord detector segments a progression in order", "[media][chord]") {
    auto c = Chord({ 261.63, 329.63, 392.00 }, 1.5, 0.8);
    auto g = Chord({ 196.00, 246.94, 293.66 }, 1.5, 0.8);
    auto am = Chord({ 220.00, 261.63, 329.63 }, 1.5, 0.8);
    Loaded L(Append(Append(c, g), am));
    HarmonyAnalysis h = DetectChords(L.get());
    REQUIRE(h.chords.size() == 3);
    CHECK(h.chords[0].name == "C");
    CHECK(h.chords[1].name == "G");
    CHECK(h.chords[2].name == "Am");
    CHECK(std::labs(h.chords[0].endMS - 1500) <= 250);
    CHECK(std::labs(h.chords[1].endMS - 3000) <= 250);
}

TEST_CASE("Chord segments are contiguous and cover the track", "[media][chord]") {
    auto c = Chord({ 261.63, 329.63, 392.00 }, 1.0, 0.8);
    auto f = Chord({ 174.61, 220.00, 261.63 }, 1.0, 0.8);
    Loaded L(Append(c, f));
    HarmonyAnalysis h = DetectChords(L.get());
    REQUIRE_FALSE(h.chords.empty());
    CHECK(h.chords.front().startMS == 0);
    for (size_t i = 0; i < h.chords.size(); i++) {
        CHECK(h.chords[i].endMS > h.chords[i].startMS);
        if (i > 0) CHECK(h.chords[i].startMS == h.chords[i - 1].endMS);
        CHECK(h.chords[i].name != "");
    }
    CHECK(h.chords.back().endMS == L.get()->LengthMS());
}

TEST_CASE("Chord detector estimates the key of a tonic triad", "[media][chord]") {
    Loaded L(Chord({ 261.63, 329.63, 392.00 }, 2.0, 0.8));
    HarmonyAnalysis h = DetectChords(L.get());
    CHECK(h.key == "C major");
}

TEST_CASE("Chord detector key of an A minor triad", "[media][chord]") {
    Loaded L(Chord({ 220.00, 261.63, 329.63 }, 2.0, 0.8));
    HarmonyAnalysis h = DetectChords(L.get());
    CHECK(h.key == "A minor");
}

TEST_CASE("Chord detector key of silence is unchanged", "[media][chord]") {
    Loaded L(Silence(2.0));
    HarmonyAnalysis h = DetectChords(L.get());
    CHECK(h.key == "C major");
}

TEST_CASE("Chord detector is transposition-consistent", "[media][chord]") {
    Loaded L(Chord({ 277.18, 349.23, 415.30 }, 2.0, 0.8));
    HarmonyAnalysis h = DetectChords(L.get());
    REQUIRE_FALSE(h.chords.empty());
    CHECK(h.chords.front().name == "C#");
}

TEST_CASE("Chord detector on silence yields one stable segment without crashing", "[media][chord]") {
    Loaded L(Silence(2.0));
    HarmonyAnalysis h = DetectChords(L.get());
    // silence has an all-zero chroma, so every template ties; the detector currently labels this "C"
    CHECK(h.chords.size() <= 1);
}

TEST_CASE("Chord detector handles noise without crashing", "[media][chord]") {
    Loaded L(Noise(2.0, 0.5));
    HarmonyAnalysis h = DetectChords(L.get());
    for (const auto& s : h.chords) CHECK_FALSE(s.name.empty());
    CHECK_FALSE(h.key.empty());
}

TEST_CASE("Chord detector rejects invalid options and inputs", "[media][chord]") {
    CHECK(DetectChords(nullptr).chords.empty());
    TempDir td;
    AudioManager bad(td.File("none.wav"), 50);
    HarmonyAnalysis hb = DetectChords(&bad);
    CHECK(hb.chords.empty());
    CHECK(hb.key.empty());

    Loaded L(Chord({ 261.63, 329.63, 392.00 }, 1.0));
    ChordDetectorOptions o;
    o.frameSize = 4097;
    CHECK(DetectChords(L.get(), o).chords.empty());
    o = ChordDetectorOptions{};
    o.hopSize = 0;
    CHECK(DetectChords(L.get(), o).chords.empty());
    o = ChordDetectorOptions{};
    o.frameSize = 8;
    CHECK(DetectChords(L.get(), o).chords.empty());

    Loaded tiny(Sine(kSR, 300.0, 0.05));
    CHECK(DetectChords(tiny.get()).chords.empty());
    CHECK(DetectChords(tiny.get()).key.empty());
}

TEST_CASE("Chord detector longer minSegmentMS suppresses brief changes", "[media][chord]") {
    auto c = Chord({ 261.63, 329.63, 392.00 }, 1.5, 0.8);
    auto g = Chord({ 196.00, 246.94, 293.66 }, 0.15, 0.8);
    Loaded L(Append(Append(c, g), c));
    ChordDetectorOptions o;
    o.minSegmentMS = 600;
    HarmonyAnalysis h = DetectChords(L.get(), o);
    REQUIRE_FALSE(h.chords.empty());
    for (const auto& s : h.chords) CHECK(s.name == "C");
}

TEST_CASE("Chord detector works for a stereo source", "[media][chord]") {
    auto m = Chord({ 261.63, 329.63, 392.00 }, 2.0, 0.8);
    Loaded L(Interleave(m, m), kSR, 2);
    HarmonyAnalysis h = DetectChords(L.get());
    REQUIRE_FALSE(h.chords.empty());
    CHECK(h.chords.front().name == "C");
}

TEST_CASE("Chord detector works for a 22 kHz source", "[media][chord]") {
    Loaded L(Chord({ 261.63, 329.63, 392.00 }, 2.0, 0.8, 22050), 22050);
    REQUIRE(L.get()->IsOk());
    HarmonyAnalysis h = DetectChords(L.get());
    REQUIRE_FALSE(h.chords.empty());
    CHECK(h.chords.front().name == "C");
}
