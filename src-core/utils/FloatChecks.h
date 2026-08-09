#pragma once

// Portable NaN / Inf / finite checks that survive -ffast-math.
//
// macOS, iPad and Linux Release builds compile with `-ffast-math`, which
// implies `-ffinite-math-only`. Under that flag clang/LLVM is licensed to
// assume no operand is NaN/Inf and folds `std::isnan(x)` to `false`,
// `std::isinf(x)` to `false` and `std::isfinite(x)` to `true` — silently
// turning defensive guards into no-ops in Release.
//
// The `__builtin_*` forms are NOT immune to this: clang 21 folds
// `__builtin_isfinite(x)` to a constant `1` under `-ffast-math`, and so does
// the equivalent bit-pattern compare, because LLVM recognises the idiom and
// re-classifies it against the `nnan`/`ninf` value facts. Only laundering the
// bits through an empty asm — so the optimizer has no fact left to reason
// with — actually preserves the check. Verified against the Release codegen,
// not assumed.
//
// MSVC compiles with `/fp:precise` and has no GNU-style inline asm, so it
// falls back to `std::*`.

#include <cmath>
#include <cstdint>
#include <cstring>

namespace xl {

#if (defined(__clang__) || defined(__GNUC__)) && !defined(_MSC_VER)

namespace detail {
inline uint32_t opaqueBits(float x)
{
    uint32_t u;
    std::memcpy(&u, &x, sizeof(u));
    __asm__("" : "+r"(u));
    return u;
}
inline uint64_t opaqueBits(double x)
{
    uint64_t u;
    std::memcpy(&u, &x, sizeof(u));
    __asm__("" : "+r"(u));
    return u;
}
} // namespace detail

inline bool isnan(float x) { return (detail::opaqueBits(x) & 0x7FFFFFFFU) > 0x7F800000U; }
inline bool isnan(double x) { return (detail::opaqueBits(x) & 0x7FFFFFFFFFFFFFFFULL) > 0x7FF0000000000000ULL; }
inline bool isinf(float x) { return (detail::opaqueBits(x) & 0x7FFFFFFFU) == 0x7F800000U; }
inline bool isinf(double x) { return (detail::opaqueBits(x) & 0x7FFFFFFFFFFFFFFFULL) == 0x7FF0000000000000ULL; }
inline bool isfinite(float x) { return (detail::opaqueBits(x) & 0x7F800000U) != 0x7F800000U; }
inline bool isfinite(double x) { return (detail::opaqueBits(x) & 0x7FF0000000000000ULL) != 0x7FF0000000000000ULL; }

#else

inline bool isnan(float  x) { return std::isnan(x); }
inline bool isnan(double x) { return std::isnan(x); }
inline bool isinf(float  x) { return std::isinf(x); }
inline bool isinf(double x) { return std::isinf(x); }
inline bool isfinite(float  x) { return std::isfinite(x); }
inline bool isfinite(double x) { return std::isfinite(x); }

#endif

} // namespace xl
