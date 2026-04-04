#pragma once
#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wnan-infinity-disabled"
#endif

#include <spdlog/spdlog.h>

// Pull in fmt's built-in enum formatter so that any enum (wx enums, etc.)
// can be passed directly to spdlog calls without manual casts.
#include <spdlog/fmt/bundled/format.h>
#ifdef __clang__
#  pragma clang diagnostic pop
#endif


using fmt::enums::format_as;
