#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include "media/AIModelStore.h"
#include "media/AudioLoader.h"
#include "media/AudioManager.h"
#include "media/IAudioDecoder.h"
#include "media/MediaCompatibility.h"
#include "media/StemSeparator.h"
#include "media/VideoDecodeSizeRegistry.h"
#include "support/TempDir.h"

using Catch::Matchers::WithinAbs;

namespace {

constexpr double kPi = 3.14159265358979323846;

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

double RmsOf(const float* p, long n) {
    double acc = 0;
    for (long i = 0; i < n; i++) acc += double(p[i]) * p[i];
    return std::sqrt(acc / double(n));
}

} // namespace

TEST_CASE("AudioManager decodes a mono WAV: duration, channels, rate", "[media][decode]") {
    TempDir td;
    std::string f = td.Write("mono.wav", MakeWav(22050, 1, Sine(22050, 440.0, 2.0)));
    AudioManager am(f, 50);
    REQUIRE(am.IsOk());
    CHECK(am.GetState() == 1);
    CHECK(am.GetChannels() == 1);
    CHECK(am.GetSampleRate() == 22050);
    CHECK(std::labs(am.LengthMS() - 2000) <= 50);
    CHECK(am.GetRate() > 0);
    CHECK(std::labs(am.GetTrackSize() - am.GetRate() * 2) <= am.GetRate() / 10);
    CHECK(am.FileName() == f);
}

TEST_CASE("AudioManager decodes a stereo WAV with distinct channels", "[media][decode]") {
    TempDir td;
    const int sr = 44100;
    auto l = Sine(sr, 440.0, 1.0, 0.8);
    std::vector<int16_t> r(l.size(), 0);
    std::string f = td.Write("st.wav", MakeWav(sr, 2, Interleave(l, r)));
    AudioManager am(f, 50);
    REQUIRE(am.IsOk());
    CHECK(am.GetChannels() == 2);
    long n = am.GetTrackSize();
    REQUIRE(n > 10000);
    float* L = am.GetRawLeftDataPtr(0);
    float* R = am.GetRawRightDataPtr(0);
    REQUIRE(L != nullptr);
    REQUIRE(R != nullptr);
    CHECK(RmsOf(L + 1000, 20000) > 0.4);
    CHECK(RmsOf(R + 1000, 20000) < 0.01);
}

TEST_CASE("AudioManager sample values and RMS of a full-scale sine", "[media][decode]") {
    TempDir td;
    const int sr = 44100;
    std::string f = td.Write("loud.wav", MakeWav(sr, 1, Sine(sr, 1000.0, 1.0, 0.5)));
    AudioManager am(f, 50);
    REQUIRE(am.IsOk());
    float* L = am.GetRawLeftDataPtr(0);
    REQUIRE(L != nullptr);
    double rms = RmsOf(L + 2000, 30000);
    // mono is upmixed to stereo, possibly with a -3dB pan law, so accept either level
    CHECK(rms > 0.24);
    CHECK(rms < 0.37);
    float mn = 0, mx = 0;
    for (long i = 2000; i < 32000; i++) {
        mn = std::min(mn, L[i]);
        mx = std::max(mx, L[i]);
    }
    CHECK(mx > 0.33f);
    CHECK(mx < 0.52f);
    CHECK(mn < -0.33f);
    CHECK(mn > -0.52f);
}

TEST_CASE("AudioManager silence decodes to near-zero samples", "[media][decode]") {
    TempDir td;
    std::string f = td.Write("sil.wav", MakeWav(44100, 1, std::vector<int16_t>(44100, 0)));
    AudioManager am(f, 50);
    REQUIRE(am.IsOk());
    float* L = am.GetRawLeftDataPtr(0);
    REQUIRE(L != nullptr);
    CHECK(RmsOf(L, am.GetTrackSize()) < 1e-4);
}

TEST_CASE("AudioManager handles several source sample rates", "[media][decode]") {
    TempDir td;
    for (int sr : { 8000, 11025, 16000, 32000, 44100, 48000, 96000 }) {
        std::string f = td.Write("r" + std::to_string(sr) + ".wav", MakeWav(sr, 1, Sine(sr, 300.0, 1.0)));
        AudioManager am(f, 50);
        INFO("rate " << sr);
        REQUIRE(am.IsOk());
        CHECK(am.GetSampleRate() == sr);
        CHECK(std::labs(am.LengthMS() - 1000) <= 40);
    }
}

TEST_CASE("AudioManager resamples so a tone keeps its duration", "[media][decode]") {
    TempDir td;
    std::string f = td.Write("a.wav", MakeWav(8000, 1, Sine(8000, 200.0, 3.0)));
    AudioManager am(f, 50);
    REQUIRE(am.IsOk());
    double seconds = double(am.GetTrackSize()) / double(am.GetRate());
    CHECK_THAT(seconds, WithinAbs(3.0, 0.1));
}

TEST_CASE("AudioManager reports failure for a missing file", "[media][decode]") {
    TempDir td;
    AudioManager am(td.File("nope.wav"));
    CHECK_FALSE(am.IsOk());
    CHECK(am.GetState() != 1);
    CHECK(am.GetTrackSize() == 0);
    CHECK(am.LengthMS() == 0);
    CHECK(am.IsDataLoaded());
}

TEST_CASE("AudioManager reports failure for a garbage file", "[media][decode]") {
    TempDir td;
    std::string junk;
    unsigned x = 12345;
    for (int i = 0; i < 4096; i++) {
        x = x * 1664525u + 1013904223u;
        junk.push_back(char(x >> 24));
    }
    AudioManager am(td.Write("junk.wav", junk), 50);
    CHECK_FALSE(am.IsOk());
}

TEST_CASE("AudioManager reports failure for an empty file", "[media][decode]") {
    TempDir td;
    AudioManager am(td.Write("empty.wav", ""), 50);
    CHECK_FALSE(am.IsOk());
}

TEST_CASE("AudioManager copes with a truncated WAV", "[media][decode]") {
    TempDir td;
    std::string full = MakeWav(44100, 1, Sine(44100, 440.0, 2.0));
    std::string cut = full.substr(0, 44 + 20000);
    AudioManager am(td.Write("cut.wav", cut), 50);
    if (am.IsOk()) {
        CHECK(am.LengthMS() > 0);
        CHECK(am.LengthMS() < 2000);
    } else {
        CHECK(am.GetTrackSize() == 0);
    }
}

TEST_CASE("AudioManager copes with a header-only WAV", "[media][decode]") {
    TempDir td;
    AudioManager am(td.Write("hdr.wav", MakeWav(44100, 1, {})), 50);
    if (am.IsOk()) {
        CHECK(am.LengthMS() == 0);
    }
    SUCCEED();
}

TEST_CASE("AudioManager handles a WAV with a lying data size", "[media][decode]") {
    TempDir td;
    std::string w = MakeWav(44100, 1, Sine(44100, 440.0, 1.0));
    uint32_t huge = 0x7FFFFFF0u;
    std::memcpy(&w[40], &huge, 4);
    AudioManager am(td.Write("lie.wav", w), 50);
    if (am.IsOk()) {
        CHECK(std::labs(am.LengthMS() - 1000) <= 100);
    }
    SUCCEED();
}

TEST_CASE("AudioManager exposes consistent hash for identical content", "[media][decode]") {
    TempDir td;
    std::string body = MakeWav(22050, 1, Sine(22050, 500.0, 0.5));
    AudioManager a(td.Write("a.wav", body), 50);
    AudioManager b(td.Write("b.wav", body), 50);
    REQUIRE(a.IsOk());
    REQUIRE(b.IsOk());
    std::string ha = a.Hash();
    CHECK_FALSE(ha.empty());
    CHECK(ha == a.Hash());
    CHECK(ha == b.Hash());
}

TEST_CASE("AudioManager Hash differs for different content", "[media][decode]") {
    TempDir td;
    AudioManager a(td.Write("a.wav", MakeWav(22050, 1, Sine(22050, 500.0, 0.5))), 50);
    AudioManager b(td.Write("b.wav", MakeWav(22050, 1, Sine(22050, 900.0, 0.5))), 50);
    REQUIRE(a.IsOk());
    REQUIRE(b.IsOk());
    CHECK(a.Hash() != b.Hash());
}

TEST_CASE("AudioManager GetAudioFileLength matches the file size for WAV", "[media][decode]") {
    TempDir td;
    std::string body = MakeWav(22050, 1, Sine(22050, 500.0, 0.5));
    std::string f = td.Write("len.wav", body);
    size_t len = AudioManager::GetAudioFileLength(f);
    CHECK(len > 0);
    CHECK(len <= body.size() + 1024);
}

TEST_CASE("AudioManager GetAudioFileLength for a missing file is zero", "[media][decode]") {
    TempDir td;
    CHECK(AudioManager::GetAudioFileLength(td.File("missing.wav")) == 0);
}

TEST_CASE("AudioManager filtered data pointers expose the full track", "[media][decode]") {
    TempDir td;
    AudioManager am(td.Write("t.wav", MakeWav(44100, 1, Sine(44100, 440.0, 1.0))), 50);
    REQUIRE(am.IsOk());
    long n = am.GetTrackSize();
    CHECK(am.IsDataLoaded(n));
    float* p = am.GetRawLeftDataPtr(n - 1);
    CHECK(p != nullptr);
    CHECK(am.GetLoadedData() == n);
}

TEST_CASE("AudioManager GetLeftDataMinMax tracks the amplitude of a sine", "[media][decode]") {
    TempDir td;
    AudioManager am(td.Write("t.wav", MakeWav(44100, 1, Sine(44100, 100.0, 1.0, 0.5))), 50);
    REQUIRE(am.IsOk());
    float mn = 0, mx = 0;
    am.GetLeftDataMinMax(1000, 20000, mn, mx, AUDIOSAMPLETYPE::RAW);
    CHECK(mx > 0.33f);
    CHECK(mx < 0.52f);
    CHECK(mn < -0.33f);
    CHECK(mn > -0.52f);
}

TEST_CASE("AudioManager frame data for a sine has energy near its note", "[media][decode]") {
    TempDir td;
    AudioManager am(td.Write("t.wav", MakeWav(44100, 1, Sine(44100, 440.0, 2.0, 0.7))), 50);
    REQUIRE(am.IsOk());
    am.DoPrepareFrameData();
    const FrameData* fd = am.GetFrameData(10, "", true);
    REQUIRE(fd != nullptr);
    CHECK(fd->max > 0.0f);
    CHECK(fd->max <= 1.01f);
    CHECK(fd->max >= fd->min);
}

TEST_CASE("AudioManager frame data beyond the end is safe", "[media][decode]") {
    TempDir td;
    AudioManager am(td.Write("t.wav", MakeWav(44100, 1, Sine(44100, 440.0, 1.0))), 50);
    REQUIRE(am.IsOk());
    am.DoPrepareFrameData();
    const FrameData* fd = am.GetFrameData(100000, "", false);
    (void)fd;
    SUCCEED();
}

TEST_CASE("AudioManager metadata map is accessible", "[media][decode]") {
    TempDir td;
    AudioManager am(td.Write("t.wav", MakeWav(44100, 1, Sine(44100, 440.0, 0.5))), 50);
    REQUIRE(am.IsOk());
    CHECK(am.Title().size() < 1000);
    (void)am.GetMetaData();
    CHECK(am.GetPlayingState() == MEDIAPLAYINGSTATE::STOPPED);
}

TEST_CASE("AudioManager MidiToFrequency known values", "[media][decode]") {
    CHECK_THAT(AudioManager::MidiToFrequency(69), WithinAbs(440.0, 1e-6));
    CHECK_THAT(AudioManager::MidiToFrequency(81), WithinAbs(880.0, 1e-5));
    CHECK_THAT(AudioManager::MidiToFrequency(57), WithinAbs(220.0, 1e-5));
    CHECK_THAT(AudioManager::MidiToFrequency(60), WithinAbs(261.6256, 1e-3));
    CHECK_THAT(AudioManager::MidiToFrequency(0), WithinAbs(8.1758, 1e-3));
}

TEST_CASE("AudioManager MidiToFrequency doubles per octave", "[media][decode]") {
    for (int m = 0; m < 100; m += 7) {
        INFO("midi " << m);
        CHECK_THAT(AudioManager::MidiToFrequency(m + 12) / AudioManager::MidiToFrequency(m), WithinAbs(2.0, 1e-9));
    }
}

TEST_CASE("AudioManager MidiToNote names", "[media][decode]") {
    CHECK(AudioManager::MidiToNote(60) == "C4");
    CHECK(AudioManager::MidiToNote(69) == "A4");
    CHECK(AudioManager::MidiToNote(61) == "C#4");
    CHECK(AudioManager::MidiToNote(0) == "C-1");
    CHECK(AudioManager::MidiToNote(127) == "G9");
    CHECK(AudioManager::MidiToNote(71) == "B4");
    CHECK(AudioManager::MidiToNote(72) == "C5");
}

TEST_CASE("AudioLoader decodes a stereo WAV to 16-bit interleaved", "[media][loader]") {
    TempDir td;
    const int sr = 44100;
    auto l = Sine(sr, 440.0, 1.0, 0.5);
    std::vector<int16_t> r(l.size(), 0);
    std::string f = td.Write("a.wav", MakeWav(sr, 2, Interleave(l, r)));
    AudioLoader loader(f, sr);
    REQUIRE(loader.loadAudioData());
    CHECK(loader.state() == AudioLoader::State::Ok);
    const auto& pcm = loader.processedAudio();
    CHECK(std::llabs(long(pcm.size()) - long(sr) * 2) <= 4096);
    int maxL = 0, maxR = 0;
    for (size_t i = 4000; i + 1 < pcm.size() && i < 60000; i += 2) {
        maxL = std::max(maxL, std::abs(int(pcm[i])));
        maxR = std::max(maxR, std::abs(int(pcm[i + 1])));
    }
    CHECK(maxL > 14000);
    CHECK(maxL < 18000);
    CHECK(maxR < 100);
}

TEST_CASE("AudioLoader upmixes mono to identical stereo channels", "[media][loader]") {
    TempDir td;
    const int sr = 22050;
    std::string f = td.Write("m.wav", MakeWav(sr, 1, Sine(sr, 300.0, 1.0, 0.5)));
    AudioLoader loader(f, sr);
    REQUIRE(loader.loadAudioData());
    const auto& pcm = loader.processedAudio();
    REQUIRE(pcm.size() > 1000);
    for (size_t i = 0; i + 1 < pcm.size(); i += 2) {
        REQUIRE(pcm[i] == pcm[i + 1]);
    }
}

TEST_CASE("AudioLoader resamples to the requested rate", "[media][loader]") {
    TempDir td;
    std::string f = td.Write("m.wav", MakeWav(16000, 1, Sine(16000, 300.0, 1.0, 0.5)));
    AudioLoader loader(f, 32000);
    REQUIRE(loader.loadAudioData());
    size_t frames = loader.processedAudio().size() / 2;
    CHECK(std::llabs(long(frames) - 32000) <= 2000);
}

TEST_CASE("AudioLoader fails gracefully on a missing file", "[media][loader]") {
    TempDir td;
    AudioLoader loader(td.File("none.wav"));
    CHECK_FALSE(loader.loadAudioData());
    CHECK(loader.state() != AudioLoader::State::Ok);
    AudioReaderDecoderInitState rs;
    CHECK(loader.readerDecoderInitState(rs));
    CHECK(rs != AudioReaderDecoderInitState::Ok);
    CHECK(loader.processedAudio().empty());
}

TEST_CASE("AudioLoader fails gracefully on garbage input", "[media][loader]") {
    TempDir td;
    std::string junk(2000, 'x');
    AudioLoader loader(td.Write("g.wav", junk));
    CHECK_FALSE(loader.loadAudioData());
    CHECK(loader.processedAudio().empty());
}

TEST_CASE("IAudioDecoder DecodeFile fills buffers consistently", "[media][decoder]") {
    TempDir td;
    std::string f = td.Write("d.wav", MakeWav(44100, 2, Interleave(Sine(44100, 440.0, 1.0), Sine(44100, 880.0, 1.0))));
    DecodedAudioInfo info;
    uint8_t* pcm = nullptr;
    long pcmSize = 0, trackSize = 0;
    float *l = nullptr, *r = nullptr;
    bool ok = AudioManager::GetDecoder().DecodeFile(f, 44100, 1024, info, pcm, pcmSize, l, r, trackSize);
    REQUIRE(ok);
    CHECK(info.channels == 2);
    CHECK(info.sampleRate == 44100);
    CHECK(std::labs(trackSize - 44100) <= 2048);
    CHECK(pcmSize >= trackSize * 4);
    REQUIRE(l != nullptr);
    REQUIRE(r != nullptr);
    CHECK(l != r);
    CHECK(RmsOf(l + 1000, 20000) > 0.2);
    CHECK(RmsOf(r + 1000, 20000) > 0.2);
    for (long i = trackSize; i < trackSize + 1024; i++) {
        INFO("pad " << i);
        REQUIRE(std::fabs(l[i]) < 1e-3f);
    }
    free(pcm);
    free(l);
    if (r != l) free(r);
}

TEST_CASE("IAudioDecoder DecodeFile reports progress monotonically", "[media][decoder]") {
    TempDir td;
    std::string f = td.Write("d.wav", MakeWav(44100, 1, Sine(44100, 440.0, 3.0)));
    DecodedAudioInfo info;
    uint8_t* pcm = nullptr;
    long pcmSize = 0, trackSize = 0;
    float *l = nullptr, *r = nullptr;
    int last = -1;
    bool monotonic = true;
    bool ok = AudioManager::GetDecoder().DecodeFile(f, 44100, 0, info, pcm, pcmSize, l, r, trackSize, [&](int pct) {
        if (pct < last) monotonic = false;
        last = pct;
    });
    REQUIRE(ok);
    CHECK(monotonic);
    CHECK(last <= 100);
    free(pcm);
    free(l);
    if (r != l) free(r);
}

TEST_CASE("IAudioDecoder DecodeFile fails for a missing file", "[media][decoder]") {
    TempDir td;
    DecodedAudioInfo info;
    uint8_t* pcm = nullptr;
    long pcmSize = 0, trackSize = 0;
    float *l = nullptr, *r = nullptr;
    CHECK_FALSE(AudioManager::GetDecoder().DecodeFile(td.File("x.wav"), 44100, 0, info, pcm, pcmSize, l, r, trackSize));
}

TEST_CASE("IAudioDecoder EncodeToFile round trips a WAV", "[media][decoder]") {
    TempDir td;
    const size_t sr = 44100;
    std::vector<float> l(sr), r(sr);
    for (size_t i = 0; i < sr; i++) {
        l[i] = float(0.5 * std::sin(2.0 * kPi * 440.0 * double(i) / double(sr)));
        r[i] = -l[i];
    }
    std::string out = td.File("enc.wav");
    if (!AudioManager::EncodeAudio(l, r, sr, out)) {
        WARN("encoder unavailable on this platform; skipping");
        return;
    }
    AudioManager am(out, 50);
    REQUIRE(am.IsOk());
    CHECK(am.GetChannels() == 2);
    CHECK(std::labs(am.LengthMS() - 1000) <= 50);
    float* L = am.GetRawLeftDataPtr(0);
    float* R = am.GetRawRightDataPtr(0);
    REQUIRE(L != nullptr);
    REQUIRE(R != nullptr);
    CHECK_THAT(double(L[10000] + R[10000]), WithinAbs(0.0, 0.01));
    CHECK(RmsOf(L + 2000, 20000) > 0.3);
}

TEST_CASE("VideoDecodeSizeRegistry stores and max-combines sizes", "[media][registry]") {
    namespace R = VideoDecodeSizeRegistry;
    R::Clear();
    int w = -1, h = -1;
    CHECK_FALSE(R::GetMaxDecodeSize("a.mp4", w, h));
    R::SetMaxDecodeSize("a.mp4", 100, 50);
    REQUIRE(R::GetMaxDecodeSize("a.mp4", w, h));
    CHECK(w == 100);
    CHECK(h == 50);
    R::SetMaxDecodeSize("a.mp4", 80, 200);
    REQUIRE(R::GetMaxDecodeSize("a.mp4", w, h));
    CHECK(w == 100);
    CHECK(h == 200);
    R::SetMaxDecodeSize("a.mp4", 300, 10);
    REQUIRE(R::GetMaxDecodeSize("a.mp4", w, h));
    CHECK(w == 300);
    CHECK(h == 200);
    R::Clear();
}

TEST_CASE("VideoDecodeSizeRegistry keeps paths independent", "[media][registry]") {
    namespace R = VideoDecodeSizeRegistry;
    R::Clear();
    R::SetMaxDecodeSize("a.mp4", 10, 20);
    R::SetMaxDecodeSize("b.mp4", 30, 40);
    int w = 0, h = 0;
    REQUIRE(R::GetMaxDecodeSize("b.mp4", w, h));
    CHECK(w == 30);
    CHECK(h == 40);
    REQUIRE(R::GetMaxDecodeSize("a.mp4", w, h));
    CHECK(w == 10);
    CHECK(h == 20);
    CHECK_FALSE(R::GetMaxDecodeSize("c.mp4", w, h));
    R::Clear();
}

TEST_CASE("VideoDecodeSizeRegistry ignores empty paths and non-positive sizes", "[media][registry]") {
    namespace R = VideoDecodeSizeRegistry;
    R::Clear();
    int w = 0, h = 0;
    R::SetMaxDecodeSize("", 10, 10);
    CHECK_FALSE(R::GetMaxDecodeSize("", w, h));
    R::SetMaxDecodeSize("z.mp4", 0, 10);
    R::SetMaxDecodeSize("z.mp4", 10, 0);
    R::SetMaxDecodeSize("z.mp4", -5, -5);
    CHECK_FALSE(R::GetMaxDecodeSize("z.mp4", w, h));
    R::Clear();
}

TEST_CASE("VideoDecodeSizeRegistry Clear removes everything", "[media][registry]") {
    namespace R = VideoDecodeSizeRegistry;
    R::SetMaxDecodeSize("q.mp4", 1, 1);
    R::Clear();
    int w = 0, h = 0;
    CHECK_FALSE(R::GetMaxDecodeSize("q.mp4", w, h));
}

TEST_CASE("VideoDecodeSizeRegistry is safe under concurrent use", "[media][registry]") {
    namespace R = VideoDecodeSizeRegistry;
    R::Clear();
    std::vector<std::thread> ts;
    for (int t = 0; t < 4; t++) {
        ts.emplace_back([t] {
            for (int i = 1; i <= 200; i++) {
                R::SetMaxDecodeSize("shared.mp4", i, t + 1);
                int w, h;
                R::GetMaxDecodeSize("shared.mp4", w, h);
            }
        });
    }
    for (auto& t : ts) t.join();
    int w = 0, h = 0;
    REQUIRE(R::GetMaxDecodeSize("shared.mp4", w, h));
    CHECK(w == 200);
    CHECK(h == 4);
    R::Clear();
}

TEST_CASE("MediaCompatibilityIssue canConvert distinguishes open failures", "[media][compat]") {
    MediaCompatibilityIssue a{ "x.mp4", "Cannot open file", true };
    MediaCompatibilityIssue b{ "x.mp4", "Cannot read stream info", true };
    MediaCompatibilityIssue c{ "x.mp4", "Unsupported video codec", true };
    CHECK_FALSE(a.canConvert());
    CHECK_FALSE(b.canConvert());
    CHECK(c.canConvert());
}

TEST_CASE("MediaCompatibilityIssue isAnimatedGif", "[media][compat]") {
    CHECK(MediaCompatibilityIssue{ "a.gif", "r", true }.isAnimatedGif());
    CHECK(MediaCompatibilityIssue{ "a.GIF", "r", true }.isAnimatedGif());
    CHECK_FALSE(MediaCompatibilityIssue{ "a.gif", "r", false }.isAnimatedGif());
    CHECK_FALSE(MediaCompatibilityIssue{ "a.mp4", "r", true }.isAnimatedGif());
    CHECK_FALSE(MediaCompatibilityIssue{ "gif", "r", true }.isAnimatedGif());
    CHECK_FALSE(MediaCompatibilityIssue{ "", "r", true }.isAnimatedGif());
}

TEST_CASE("MediaCompatibility checks of empty and missing files", "[media][compat]") {
    TempDir td;
    CHECK(MediaCompatibility::CheckVideoFile("").empty());
    CHECK_FALSE(MediaCompatibility::CheckVideoFile(td.File("missing.mp4")).empty());
#ifndef __APPLE__
    CHECK(MediaCompatibility::CheckAudioFile(td.File("missing.wav")).empty());
#endif
}

TEST_CASE("MediaCompatibility accepts a plain WAV as audio", "[media][compat]") {
    TempDir td;
    std::string f = td.Write("ok.wav", MakeWav(44100, 1, Sine(44100, 440.0, 0.2)));
    CHECK(MediaCompatibility::CheckAudioFile(f).empty());
}

TEST_CASE("MediaCompatibility rejects garbage as video", "[media][compat]") {
    TempDir td;
    std::string f = td.Write("bad.mp4", std::string(512, 'z'));
    CHECK_FALSE(MediaCompatibility::CheckVideoFile(f).empty());
}

TEST_CASE("MediaCompatibility CheckSequenceMedia with nothing is clean", "[media][compat]") {
    CHECK(MediaCompatibility::CheckSequenceMedia("", {}).empty());
}

TEST_CASE("MediaCompatibility CheckSequenceMedia flags a missing video", "[media][compat]") {
    TempDir td;
    auto issues = MediaCompatibility::CheckSequenceMedia("", { td.File("gone.mp4") });
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].isVideo);
    CHECK(issues[0].filePath == td.File("gone.mp4"));
    CHECK_FALSE(issues[0].reason.empty());
    CHECK_FALSE(issues[0].canConvert());
}

TEST_CASE("AIModelStore CandidateModelDirs appends the models subdir", "[media][aimodel]") {
    auto d = AIModelStore::CandidateModelDirs({ "root1", "", "root2" });
    REQUIRE(d.size() == 2);
    CHECK(std::filesystem::path(d[0]) == std::filesystem::path("root1") / AIModelStore::kModelsSubdir);
    CHECK(std::filesystem::path(d[1]) == std::filesystem::path("root2") / AIModelStore::kModelsSubdir);
    CHECK(AIModelStore::CandidateModelDirs({}).empty());
}

TEST_CASE("AIModelStore EnsureDirectory creates nested directories", "[media][aimodel]") {
    TempDir td;
    std::string d = (td.path / "a" / "b" / "c").string();
    CHECK(AIModelStore::EnsureDirectory(d));
    CHECK(std::filesystem::is_directory(d));
    CHECK(AIModelStore::EnsureDirectory(d));
}

TEST_CASE("AIModelStore EnsureDirectory rejects empty path and plain files", "[media][aimodel]") {
    TempDir td;
    CHECK_FALSE(AIModelStore::EnsureDirectory(""));
    std::string f = td.Write("file.txt", "x");
    CHECK_FALSE(AIModelStore::EnsureDirectory(f));
}

TEST_CASE("AIModelStore FindModel searches directories in order", "[media][aimodel]") {
    TempDir td;
    std::string d1 = (td.path / "d1").string();
    std::string d2 = (td.path / "d2").string();
    REQUIRE(AIModelStore::EnsureDirectory(d1));
    REQUIRE(AIModelStore::EnsureDirectory(d2));
    CHECK(AIModelStore::FindModel("m.onnx", { d1, d2 }).empty());
    std::ofstream(std::filesystem::path(d2) / "m.onnx") << "x";
    CHECK(std::filesystem::path(AIModelStore::FindModel("m.onnx", { d1, d2 })) == std::filesystem::path(d2) / "m.onnx");
    std::ofstream(std::filesystem::path(d1) / "m.onnx") << "x";
    CHECK(std::filesystem::path(AIModelStore::FindModel("m.onnx", { d1, d2 })) == std::filesystem::path(d1) / "m.onnx");
    CHECK(AIModelStore::FindModel("m.onnx", { "", d2 }) == (std::filesystem::path(d2) / "m.onnx").string());
    CHECK(AIModelStore::FindModel("m.onnx", {}).empty());
}

TEST_CASE("AIModelStore model names and URLs are well formed", "[media][aimodel]") {
    CHECK(std::string(AIModelStore::kDemucsOnnxModelName).find(".onnx") != std::string::npos);
    CHECK(std::string(AIModelStore::kDemucs6sOnnxModelName).find(".onnx") != std::string::npos);
    CHECK(std::string(AIModelStore::kDemucsOnnxDownloadURL).rfind("https://", 0) == 0);
    CHECK(std::string(AIModelStore::kDemucs6sOnnxDownloadURL).rfind("https://", 0) == 0);
    CHECK(std::string(AIModelStore::kDemucsDownloadURL).rfind("https://", 0) == 0);
}

TEST_CASE("SeparateStems refuses null audio and empty model path", "[media][stems]") {
    StemOutput out;
    CHECK_FALSE(SeparateStems(nullptr, "model.onnx", out));
    TempDir td;
    AudioManager am(td.Write("a.wav", MakeWav(44100, 1, Sine(44100, 440.0, 0.5))), 50);
    REQUIRE(am.IsOk());
    CHECK_FALSE(SeparateStems(&am, "", out));
    CHECK(out.drumsL.empty());
}

TEST_CASE("SeparateStems refuses a failed audio manager", "[media][stems]") {
    TempDir td;
    AudioManager bad(td.File("none.wav"));
    StemOutput out;
    CHECK_FALSE(SeparateStems(&bad, "model.onnx", out));
}

TEST_CASE("StemOptions and StemOutput defaults", "[media][stems]") {
    StemSeparatorOptions o;
    CHECK(o.chunkSamples > o.overlapSamples);
    CHECK(o.overlapSamples >= 0);
    StemOutput s;
    CHECK(s.sampleRate == 0);
    CHECK(s.guitarL.empty());
}

TEST_CASE("AudioManager SetStemData reports stem availability", "[media][stems]") {
    TempDir td;
    AudioManager am(td.Write("a.wav", MakeWav(44100, 1, Sine(44100, 440.0, 0.5))), 50);
    REQUIRE(am.IsOk());
    CHECK_FALSE(am.HasStemData());
    long n = am.GetTrackSize();
    std::vector<float> z(size_t(n), 0.0f);
    am.SetStemData(z, z, z, z, z, z, z, z);
    CHECK(am.HasStemData());
    CHECK_FALSE(am.HasSixStemData());
    am.SetStemData(z, z, z, z, z, z, z, z, z, z, z, z);
    CHECK(am.HasSixStemData());
    am.SetStemData({}, {}, {}, {}, {}, {}, {}, {});
    CHECK_FALSE(am.HasStemData());
}

TEST_CASE("MediaCompatibility CheckSequenceMedia skips empty entries", "[media][compat]") {
    TempDir td;
    auto issues = MediaCompatibility::CheckSequenceMedia("", { "", td.File("gone.mp4"), "" });
    CHECK(issues.size() == 1);
}
