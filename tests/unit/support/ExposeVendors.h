#pragma once

// The vendor handlers keep their parsers and encoders private. These tests reach them
// by widening access while including the handler headers (Itanium ABI platforms only:
// MSVC mangles access specifiers, so this must not be used on Windows).
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>
#include <pugixml.hpp>

// Everything a handler header includes must already be in the TU: the widening macros below
// would otherwise also rewrite those headers.
#include <curl/curl.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include "controllers/ControllerUploadData.h"
#include "controllers/WebSocketClient.h"
#include "models/ModelManager.h"
#include "models/ViewObjectManager.h"
#include "render/FSEQFile.h"
#include "utils/CurlManager.h"
#include "utils/UtilClasses.h"

// Most handler members sit in the default-private section of a class, so widen that too.
#define private public
#define protected public
#define class struct

#include "controllers/BaseController.h"

// CurlManager logs through a named "curl" logger that the app registers at startup.
inline void VendorsTestInit() {
    spdlog::set_level(spdlog::level::off);
    if (!spdlog::get("curl")) {
        spdlog::register_logger(std::make_shared<spdlog::logger>("curl", std::make_shared<spdlog::sinks::null_sink_mt>()));
    }
}
