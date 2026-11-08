#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <cstdlib>
#include <vector>

#include "media/kiss_fft/kiss_fft.h"
#include "media/kiss_fft/tools/kiss_fftr.h"

using Catch::Matchers::WithinAbs;

namespace {

constexpr double kPi = 3.14159265358979323846;

struct Cfg {
    kiss_fft_cfg c;
    Cfg(int n, int inv) : c(kiss_fft_alloc(n, inv, nullptr, nullptr)) {}
    ~Cfg() { free(c); }
};

struct RCfg {
    kiss_fftr_cfg c;
    RCfg(int n, int inv) : c(kiss_fftr_alloc(n, inv, nullptr, nullptr)) {}
    ~RCfg() { free(c); }
};

std::vector<kiss_fft_cpx> Dft(const std::vector<kiss_fft_cpx>& x) {
    size_t n = x.size();
    std::vector<kiss_fft_cpx> out(n);
    for (size_t k = 0; k < n; k++) {
        double re = 0, im = 0;
        for (size_t t = 0; t < n; t++) {
            double a = -2.0 * kPi * double(k) * double(t) / double(n);
            re += x[t].r * std::cos(a) - x[t].i * std::sin(a);
            im += x[t].r * std::sin(a) + x[t].i * std::cos(a);
        }
        out[k].r = float(re);
        out[k].i = float(im);
    }
    return out;
}

std::vector<kiss_fft_cpx> RandomComplex(size_t n, unsigned seed) {
    std::vector<kiss_fft_cpx> v(n);
    unsigned s = seed;
    auto next = [&] {
        s = s * 1664525u + 1013904223u;
        return float(double(s >> 8) / double(1u << 24) * 2.0 - 1.0);
    };
    for (auto& c : v) {
        c.r = next();
        c.i = next();
    }
    return v;
}

} // namespace

TEST_CASE("kiss_fft matches a direct DFT for assorted sizes", "[media][kissfft]") {
    for (int n : { 1, 2, 3, 4, 5, 6, 8, 12, 15, 16, 30, 49, 64, 100, 128, 210 }) {
        INFO("n " << n);
        Cfg cfg(n, 0);
        REQUIRE(cfg.c != nullptr);
        auto in = RandomComplex(size_t(n), 100u + unsigned(n));
        std::vector<kiss_fft_cpx> out(static_cast<size_t>(n));
        kiss_fft(cfg.c, in.data(), out.data());
        auto ref = Dft(in);
        for (int k = 0; k < n; k++) {
            REQUIRE_THAT(double(out[k].r), WithinAbs(double(ref[k].r), 1e-3 * (1 + n * 0.05)));
            REQUIRE_THAT(double(out[k].i), WithinAbs(double(ref[k].i), 1e-3 * (1 + n * 0.05)));
        }
    }
}

TEST_CASE("kiss_fft of an impulse is flat", "[media][kissfft]") {
    const int n = 64;
    Cfg cfg(n, 0);
    std::vector<kiss_fft_cpx> in(n, kiss_fft_cpx{ 0, 0 }), out(n);
    in[0].r = 1.0f;
    kiss_fft(cfg.c, in.data(), out.data());
    for (int k = 0; k < n; k++) {
        CHECK_THAT(double(out[k].r), WithinAbs(1.0, 1e-5));
        CHECK_THAT(double(out[k].i), WithinAbs(0.0, 1e-5));
    }
}

TEST_CASE("kiss_fft of a constant concentrates in DC", "[media][kissfft]") {
    const int n = 32;
    Cfg cfg(n, 0);
    std::vector<kiss_fft_cpx> in(n, kiss_fft_cpx{ 2.0f, 0.0f }), out(n);
    kiss_fft(cfg.c, in.data(), out.data());
    CHECK_THAT(double(out[0].r), WithinAbs(64.0, 1e-3));
    for (int k = 1; k < n; k++) {
        CHECK_THAT(double(out[k].r), WithinAbs(0.0, 1e-3));
        CHECK_THAT(double(out[k].i), WithinAbs(0.0, 1e-3));
    }
}

TEST_CASE("kiss_fft forward then inverse scales by N", "[media][kissfft]") {
    for (int n : { 8, 20, 64, 90, 256 }) {
        INFO("n " << n);
        Cfg f(n, 0), b(n, 1);
        auto in = RandomComplex(size_t(n), 7u);
        std::vector<kiss_fft_cpx> mid(static_cast<size_t>(n)), out(static_cast<size_t>(n));
        kiss_fft(f.c, in.data(), mid.data());
        kiss_fft(b.c, mid.data(), out.data());
        for (int i = 0; i < n; i++) {
            REQUIRE_THAT(double(out[i].r) / n, WithinAbs(double(in[i].r), 1e-4));
            REQUIRE_THAT(double(out[i].i) / n, WithinAbs(double(in[i].i), 1e-4));
        }
    }
}

TEST_CASE("kiss_fft is linear", "[media][kissfft]") {
    const int n = 48;
    Cfg cfg(n, 0);
    auto a = RandomComplex(n, 1);
    auto b = RandomComplex(n, 2);
    std::vector<kiss_fft_cpx> sum(n), fa(n), fb(n), fs(n);
    for (int i = 0; i < n; i++) {
        sum[i].r = 2.0f * a[i].r + 3.0f * b[i].r;
        sum[i].i = 2.0f * a[i].i + 3.0f * b[i].i;
    }
    kiss_fft(cfg.c, a.data(), fa.data());
    kiss_fft(cfg.c, b.data(), fb.data());
    kiss_fft(cfg.c, sum.data(), fs.data());
    for (int k = 0; k < n; k++) {
        CHECK_THAT(double(fs[k].r), WithinAbs(2.0 * fa[k].r + 3.0 * fb[k].r, 1e-3));
        CHECK_THAT(double(fs[k].i), WithinAbs(2.0 * fa[k].i + 3.0 * fb[k].i, 1e-3));
    }
}

TEST_CASE("kiss_fft satisfies Parseval's relation", "[media][kissfft]") {
    const int n = 128;
    Cfg cfg(n, 0);
    auto in = RandomComplex(n, 3);
    std::vector<kiss_fft_cpx> out(n);
    kiss_fft(cfg.c, in.data(), out.data());
    double et = 0, ef = 0;
    for (int i = 0; i < n; i++) {
        et += double(in[i].r) * in[i].r + double(in[i].i) * in[i].i;
        ef += double(out[i].r) * out[i].r + double(out[i].i) * out[i].i;
    }
    CHECK_THAT(ef / n, WithinAbs(et, et * 1e-4));
}

TEST_CASE("kiss_fft time shift becomes a phase ramp", "[media][kissfft]") {
    const int n = 32;
    Cfg cfg(n, 0);
    std::vector<kiss_fft_cpx> in(n, kiss_fft_cpx{ 0, 0 }), out(n);
    in[3].r = 1.0f;
    kiss_fft(cfg.c, in.data(), out.data());
    for (int k = 0; k < n; k++) {
        double a = -2.0 * kPi * k * 3 / n;
        CHECK_THAT(double(out[k].r), WithinAbs(std::cos(a), 1e-4));
        CHECK_THAT(double(out[k].i), WithinAbs(std::sin(a), 1e-4));
    }
}

TEST_CASE("kiss_fft_stride reads strided input", "[media][kissfft]") {
    const int n = 16;
    Cfg cfg(n, 0);
    auto in = RandomComplex(n, 5);
    std::vector<kiss_fft_cpx> inter(n * 2, kiss_fft_cpx{ 9, 9 });
    for (int i = 0; i < n; i++) inter[size_t(i) * 2] = in[i];
    std::vector<kiss_fft_cpx> a(n), b(n);
    kiss_fft(cfg.c, in.data(), a.data());
    kiss_fft_stride(cfg.c, inter.data(), b.data(), 2);
    for (int k = 0; k < n; k++) {
        CHECK_THAT(double(b[k].r), WithinAbs(double(a[k].r), 1e-5));
        CHECK_THAT(double(b[k].i), WithinAbs(double(a[k].i), 1e-5));
    }
}

TEST_CASE("kiss_fft_alloc fills a caller-provided buffer and reports its size", "[media][kissfft]") {
    size_t need = 0;
    CHECK(kiss_fft_alloc(64, 0, nullptr, &need) == nullptr);
    REQUIRE(need > 0);
    std::vector<char> mem(need + 16);
    size_t len = mem.size();
    kiss_fft_cfg cfg = kiss_fft_alloc(64, 0, mem.data(), &len);
    REQUIRE(cfg != nullptr);
    std::vector<kiss_fft_cpx> in(64, kiss_fft_cpx{ 0, 0 }), out(64);
    in[0].r = 1.0f;
    kiss_fft(cfg, in.data(), out.data());
    CHECK_THAT(double(out[10].r), WithinAbs(1.0, 1e-5));
}

TEST_CASE("kiss_fft_alloc rejects a too-small buffer", "[media][kissfft]") {
    char tiny[8];
    size_t len = sizeof(tiny);
    CHECK(kiss_fft_alloc(64, 0, tiny, &len) == nullptr);
}

TEST_CASE("kiss_fft_next_fast_size returns sizes factorable by 2, 3 and 5", "[media][kissfft]") {
    CHECK(kiss_fft_next_fast_size(1) == 1);
    CHECK(kiss_fft_next_fast_size(7) == 8);
    CHECK(kiss_fft_next_fast_size(13) == 15);
    CHECK(kiss_fft_next_fast_size(64) == 64);
    CHECK(kiss_fft_next_fast_size(1000) == 1000);
    CHECK(kiss_fft_next_fast_size(1001) == 1024);
    for (int n = 1; n < 500; n++) {
        int m = kiss_fft_next_fast_size(n);
        REQUIRE(m >= n);
        while (m % 2 == 0) m /= 2;
        while (m % 3 == 0) m /= 3;
        while (m % 5 == 0) m /= 5;
        REQUIRE(m == 1);
    }
}

TEST_CASE("kiss_fftr puts a sine in the expected bin", "[media][kissfft]") {
    const int n = 1024;
    RCfg cfg(n, 0);
    REQUIRE(cfg.c != nullptr);
    for (int bin : { 1, 10, 100, 255, 400 }) {
        std::vector<float> in(n);
        for (int i = 0; i < n; i++) in[i] = float(std::sin(2.0 * kPi * bin * i / n));
        std::vector<kiss_fft_cpx> out(n / 2 + 1);
        kiss_fftr(cfg.c, in.data(), out.data());
        int best = 0;
        double bestMag = 0;
        for (int k = 0; k <= n / 2; k++) {
            double m = std::hypot(double(out[k].r), double(out[k].i));
            if (m > bestMag) { bestMag = m; best = k; }
        }
        INFO("bin " << bin);
        CHECK(best == bin);
        CHECK_THAT(bestMag, WithinAbs(n / 2.0, 0.05));
    }
}

TEST_CASE("kiss_fftr matches the complex FFT of real input", "[media][kissfft]") {
    const int n = 60;
    RCfg r(n, 0);
    Cfg c(n, 0);
    auto z = RandomComplex(n, 11);
    std::vector<float> in(n);
    std::vector<kiss_fft_cpx> cin(n), cout(n), rout(n / 2 + 1);
    for (int i = 0; i < n; i++) { in[i] = z[i].r; cin[i] = { z[i].r, 0.0f }; }
    kiss_fftr(r.c, in.data(), rout.data());
    kiss_fft(c.c, cin.data(), cout.data());
    for (int k = 0; k <= n / 2; k++) {
        CHECK_THAT(double(rout[k].r), WithinAbs(double(cout[k].r), 1e-4));
        CHECK_THAT(double(rout[k].i), WithinAbs(double(cout[k].i), 1e-4));
    }
}

TEST_CASE("kiss_fftr DC and Nyquist bins are purely real", "[media][kissfft]") {
    const int n = 128;
    RCfg cfg(n, 0);
    auto z = RandomComplex(n, 12);
    std::vector<float> in(n);
    for (int i = 0; i < n; i++) in[i] = z[i].r;
    std::vector<kiss_fft_cpx> out(n / 2 + 1);
    kiss_fftr(cfg.c, in.data(), out.data());
    CHECK_THAT(double(out[0].i), WithinAbs(0.0, 1e-5));
    CHECK_THAT(double(out[n / 2].i), WithinAbs(0.0, 1e-5));
}

TEST_CASE("kiss_fftr round trips through kiss_fftri scaled by N", "[media][kissfft]") {
    for (int n : { 16, 100, 512, 2048 }) {
        INFO("n " << n);
        RCfg f(n, 0), b(n, 1);
        auto z = RandomComplex(size_t(n), 21u);
        std::vector<float> in(n), out(n);
        for (int i = 0; i < n; i++) in[i] = z[i].r;
        std::vector<kiss_fft_cpx> spec(n / 2 + 1);
        kiss_fftr(f.c, in.data(), spec.data());
        kiss_fftri(b.c, spec.data(), out.data());
        for (int i = 0; i < n; i++) {
            REQUIRE_THAT(double(out[i]) / n, WithinAbs(double(in[i]), 1e-4));
        }
    }
}

TEST_CASE("kiss_fftr rejects odd sizes", "[media][kissfft]") {
    kiss_fftr_cfg c = kiss_fftr_alloc(15, 0, nullptr, nullptr);
    CHECK(c == nullptr);
    free(c);
}

TEST_CASE("kiss_fftr of silence is zero", "[media][kissfft]") {
    const int n = 256;
    RCfg cfg(n, 0);
    std::vector<float> in(n, 0.0f);
    std::vector<kiss_fft_cpx> out(n / 2 + 1, kiss_fft_cpx{ 1, 1 });
    kiss_fftr(cfg.c, in.data(), out.data());
    for (auto& c : out) {
        CHECK(c.r == 0.0f);
        CHECK(c.i == 0.0f);
    }
}

TEST_CASE("kiss_fftr autocorrelation via power spectrum finds a period", "[media][kissfft]") {
    const int n = 512, m = 2 * n;
    RCfg f(m, 0), b(m, 1);
    std::vector<float> in(m, 0.0f);
    for (int i = 0; i < n; i++) in[i] = float(std::sin(2.0 * kPi * i / 50.0));
    std::vector<kiss_fft_cpx> spec(m / 2 + 1);
    kiss_fftr(f.c, in.data(), spec.data());
    for (auto& c : spec) { c.r = c.r * c.r + c.i * c.i; c.i = 0; }
    std::vector<float> acf(m);
    kiss_fftri(b.c, spec.data(), acf.data());
    int best = 20;
    for (int t = 20; t < 80; t++) if (acf[t] > acf[best]) best = t;
    CHECK(best == 50);
}
