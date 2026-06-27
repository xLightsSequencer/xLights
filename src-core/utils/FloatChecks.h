#pragma once

// Portable NaN / Inf / finite checks.
//
// macOS and iPad Release builds compile with `-ffast-math`, which implies
// `-ffinite-math-only`. Under that flag clang/LLVM is licensed to assume no
// operand is NaN/Inf and may fold `std::isnan(x)` to `false`, `std::isinf(x)`
// to `false`, and `std::isfinite(x)` to `true` — silently turning defensive
// guards into no-ops in Release. The `__builtin_*` forms are preserved by
// clang/gcc even under `-ffinite-math-only`.
//
// MSVC compiles with `/fp:precise` (no `-ffast-math` equivalent enabled) and
// does not provide `__builtin_isnan` etc., so we fall back to `std::*` there.
//
// See `CLAUDE.md` "Release Builds Use -ffast-math" for the broader context.

#include <cmath>

namespace xl {

#if defined(__clang__) || defined(__GNUC__)

inline bool isnan(float  x) { return __builtin_isnan(x); }
inline bool isnan(double x) { return __builtin_isnan(x); }
inline bool isinf(float  x) { return __builtin_isinf(x); }
inline bool isinf(double x) { return __builtin_isinf(x); }
inline bool isfinite(float  x) { return __builtin_isfinite(x); }
inline bool isfinite(double x) { return __builtin_isfinite(x); }

#else

inline bool isnan(float  x) { return std::isnan(x); }
inline bool isnan(double x) { return std::isnan(x); }
inline bool isinf(float  x) { return std::isinf(x); }
inline bool isinf(double x) { return std::isinf(x); }
inline bool isfinite(float  x) { return std::isfinite(x); }
inline bool isfinite(double x) { return std::isfinite(x); }

#endif

} // namespace xl
