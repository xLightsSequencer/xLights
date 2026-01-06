#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "spdlog/spdlog.h"
#include <spdlog/fmt/bundled/printf.h>
#include <spdlog/fmt/bundled/format.h>
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/ostream.h>
#include <thread>
#include <sstream>

#include <wx/string.h>

template <>
struct fmt::formatter<std::thread::id> : fmt::formatter<std::string> {
    auto format(const std::thread::id& id, fmt::format_context& ctx) const {
        std::ostringstream oss;
        oss << id;
        return fmt::formatter<std::string>::format(oss.str(), ctx);
    }
};

// Specialize fmt::formatter for wxString
template<>
struct fmt::formatter<wxString> : fmt::formatter<std::string> {
    auto format(const wxString& s, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(std::string(s.data(), s.length()), ctx);
    }
};

#define LOG_TRACEWX(s){ wxString ss(s); spdlog::trace(std::string(ss.data(), ss.length()));}
#define LOG_DEBUGWX(s){ wxString ss(s); spdlog::debug(std::string(ss.data(), ss.length()));}
#define LOG_INFOWX(s){ wxString ss(s); spdlog::info(std::string(ss.data(), ss.length()));}
#define LOG_WARNWX(s){ wxString ss(s); spdlog::warn(std::string(ss.data(), ss.length()));}
#define LOG_ERRORWX(s){ wxString ss(s); spdlog::error(std::string(ss.data(), ss.length()));}
#define LOG_CRITWX(s){ wxString ss(s); spdlog::critical(std::string(ss.data(), ss.length()));}

#define LOG_TRACE(format, ...) spdlog::trace(fmt::sprintf(format, __VA_ARGS__))
#define LOG_DEBUG(format, ...) spdlog::debug(fmt::sprintf(format, __VA_ARGS__))
#define LOG_INFO(format, ...) spdlog::info(fmt::sprintf(format, __VA_ARGS__))
#define LOG_WARN(format, ...) spdlog::warn(fmt::sprintf(format, __VA_ARGS__))
#define LOG_ERROR(format, ...) spdlog::error(fmt::sprintf(format, __VA_ARGS__))
#define LOG_CRIT(format, ...) spdlog::critical(fmt::sprintf(format, __VA_ARGS__))

#define LOGG_TRACE(logger, format, ...) logger->trace(fmt::sprintf(format, __VA_ARGS__))
#define LOGG_DEBUG(logger, format, ...) logger->debug(fmt::sprintf(format, __VA_ARGS__))
#define LOGG_INFO(logger, format, ...) logger->info(fmt::sprintf(format, __VA_ARGS__))
#define LOGG_WARN(logger, format, ...) logger->warn(fmt::sprintf(format, __VA_ARGS__))
#define LOGG_ERROR(logger, format, ...) logger->error(fmt::sprintf(format, __VA_ARGS__))
#define LOGG_CRIT(logger, format, ...) logger->critical(fmt::sprintf(format, __VA_ARGS__))