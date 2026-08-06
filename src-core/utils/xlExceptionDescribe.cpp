/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "xlExceptionDescribe.h"

#include <exception>
#include <filesystem>
#include <string>
#include <system_error>
#include <typeinfo>

#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
#endif

#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/std.h> // fmt formatters for std::type_info (demangled), std::error_code, std::filesystem::path

#if defined(__APPLE__) && __has_include("osxUtils/ExternalHooksApple.h")
#include "osxUtils/ExternalHooksApple.h"
#define XL_HAVE_APPLE_EXCEPTION_DESCRIBE 1
#endif

std::string DescribeCurrentException()
{
    // Caller must be inside a catch handler; rethrow to recover the type.
    try {
        throw;
    } catch (char const* eMsg) {
        return fmt::format("C-string exception: \"{}\".", eMsg ? eMsg : "(null)");
    } catch (std::string const& eMsg) {
        return fmt::format("std::string exception: \"{}\".", eMsg);
    } catch (std::filesystem::filesystem_error const& e) {
        // Common xLights crash source (throwing fs::exists/copy/etc. on sandbox/iCloud
        // edge cases). Capture the error code and both paths so the offending file shows.
        return fmt::format("Filesystem exception of type \"{}\": \"{}\" (error {}: {}) path1=\"{}\" path2=\"{}\".",
                           typeid(e), e.what(), e.code().value(), e.code().message(),
                           e.path1().string(), e.path2().string());
    } catch (std::system_error const& e) {
        return fmt::format("System exception of type \"{}\": \"{}\" (error {}: {}).",
                           typeid(e), e.what(), e.code().value(), e.code().message());
    } catch (std::exception const& e) {
        // typeid is demangled by fmt's std::type_info formatter, so macOS/Linux reports
        // read "std::length_error" rather than the raw "St12length_error".
        return fmt::format("Standard STD exception of type \"{}\" with message \"{}\".",
                           typeid(e), e.what());
    } catch (...) {
#ifdef XL_HAVE_APPLE_EXCEPTION_DESCRIBE
        // Cocoa/AVFoundation/Metal raise NSExceptions, which no C++ handler above
        // can match - they were reaching the reports as a bare "unknown exception".
        std::string objc = DescribeCurrentAppleException();
        if (!objc.empty()) {
            return objc;
        }
#endif
#if defined(__GNUC__) || defined(__clang__)
        if (std::type_info const* t = abi::__cxa_current_exception_type()) {
            return fmt::format("An exception of non-standard type \"{}\" occurred.", *t);
        }
#endif
        // Every exception the C++ or Objective-C runtime raises is named above, so
        // reaching here means the stack was unwound by neither - a forced unwind or
        // a foreign runtime.  current_exception() is null only in that case, and
        // saying so is the difference between a searchable report and a dead end:
        // there is no C++ throw site to go looking for.
        if (std::current_exception() == nullptr) {
            return "A foreign (non-C++) exception occurred - the stack was unwound by neither the C++ nor the Objective-C runtime, so no type information exists.";
        }
        return "An unknown exception occurred.";
    }
}
