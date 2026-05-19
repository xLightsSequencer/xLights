cmake_minimum_required(VERSION 3.20)
project(xLights VERSION 2026.04 LANGUAGES C CXX)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT xLights)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS ON)   # -std=gnu++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# ─── Output directory ───────────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# ─── Shorthand paths ────────────────────────────────────────────────────────
set(XL_INC  ${CMAKE_SOURCE_DIR}/include)
set(XL_DEPS ${CMAKE_SOURCE_DIR}/dependencies)
set(XL_LIB  ${CMAKE_SOURCE_DIR}/lib)

# ─── macOS prebuilt dependency tree (matches Xcode build layout) ───────────
# When macOS/scripts/download_deps has run, macOS/dependencies is a symlink
# to either /opt/local or a tag-specific tarball under macOS/. Reuse those
# prebuilt libs so the cmake build doesn't need a parallel copy.
#
# Single-config only: find_library can't use generator expressions, so the
# wx-config + lib dir are chosen at configure time from CMAKE_BUILD_TYPE.
#   Debug   → libdbg/ (dylibs)     — fast link, matches Xcode Debug.
#   Release → lib/    (static+LTO) — slow link, matches Xcode Release.
if(APPLE AND EXISTS "${CMAKE_SOURCE_DIR}/macOS/dependencies/bin/wx-config")
    set(XL_MAC_DEPS "${CMAKE_SOURCE_DIR}/macOS/dependencies")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(XL_MAC_LIB "${XL_MAC_DEPS}/libdbg")
        set(_xl_wxcfg "${XL_MAC_LIB}/wx/config/osx_cocoa-unicode-3.3")
    else()
        set(XL_MAC_LIB "${XL_MAC_DEPS}/lib")
        set(_xl_wxcfg "${XL_MAC_LIB}/wx/config/osx_cocoa-unicode-static-3.3")
    endif()
    set(XL_MAC_INC "${XL_MAC_DEPS}/include")
    if(EXISTS "${_xl_wxcfg}")
        set(wxWidgets_CONFIG_EXECUTABLE "${_xl_wxcfg}"
            CACHE FILEPATH "wxWidgets config executable")
    endif()
    # Lets find_package(CURL / zstd / SDL2 / WebP) discover deps-tree configs
    # ahead of any system install.
    list(PREPEND CMAKE_PREFIX_PATH "${XL_MAC_DEPS}")
    message(STATUS "macOS prebuilt deps: ${XL_MAC_DEPS} (lib dir: ${XL_MAC_LIB})")
endif()

if(MSVC)
    add_compile_options("/utf-8")
    # /MP — compile multiple source files in parallel within a single project.
    # The old .vcxproj had this; without it cmake builds are significantly slower.
    # Omit when a compiler launcher (sccache/ccache) is active because those
    # tools manage their own parallelism and /MP conflicts with them.
    if(NOT CMAKE_CXX_COMPILER_LAUNCHER)
        add_compile_options("/MP")
    endif()
    # Pin the runtime library so every .obj in the build uses the same CRT.
    # Visual Studio is a multi-config generator: CMAKE_BUILD_TYPE is empty at
    # configure time, so use a generator expression to pick MD vs MDd per config.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
        CACHE STRING "MSVC runtime library" FORCE)
endif()

add_compile_definitions(NOMINMAX)

# Always defined for any cmake build — suppresses VS-only #pragma comment(lib,…)
# directives in source files so cmake's target_link_libraries is the sole link mechanism.
add_compile_definitions(XLIGHTS_CMAKE_BUILD)

# ─── vcpkg static-libs option ───────────────────────────────────────────────
# Enable with:  cmake -DXLIGHTS_USE_VCPKG=ON -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
# For a fully static build also pass:  -DVCPKG_TARGET_TRIPLET=x64-windows-static
option(XLIGHTS_USE_VCPKG "Require all dependencies via vcpkg (no vendored fallback)" OFF)
if(XLIGHTS_USE_VCPKG)
    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)
    message(STATUS "XLIGHTS_USE_VCPKG=ON — all deps resolved via cmake CONFIG packages")
endif()

# ─── Platform compile definitions ───────────────────────────────────────────
if(WIN32)
    add_compile_definitions(_WINDOWS _CRT_SECURE_NO_WARNINGS IGNORE_VENDORS wxUSE_WEBP)
    # Use generator expressions — CMAKE_BUILD_TYPE is always empty for the
    # Visual Studio multi-config generator, so if/else on it never works.
    add_compile_definitions(
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Debug>:WXDEBUG>
        $<$<CONFIG:Debug>:__WXDEBUG__>
        $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    )
elseif(UNIX AND NOT APPLE)
    add_compile_options(
        -Wno-reorder -Wno-sign-compare -Wno-unused-variable
        -Wno-unused-but-set-variable -Wno-unused-function -Wno-unknown-pragmas
    )
    add_compile_definitions(IGNORE_VENDORS wxUSE_WEBP)
else()
    add_compile_definitions(IGNORE_VENDORS wxUSE_WEBP)
endif()

# ─── wxWidgets ──────────────────────────────────────────────────────────────
# On Windows the FindWxWidgets module requires wxWidgets_ROOT_DIR or WXWIN.
# Prefer the CMake cache variable, then the environment variable, then probe
# common install locations used by xLights developers.
if(WIN32 AND NOT wxWidgets_ROOT_DIR)
    if(DEFINED ENV{WXWIN})
        set(wxWidgets_ROOT_DIR "$ENV{WXWIN}" CACHE PATH "wxWidgets root directory")
    else()
        foreach(_wx_candidate
                "C:/software/wxWidgets"
                "C:/wxWidgets"
                "C:/wx"
                "$ENV{USERPROFILE}/wxWidgets")
            if(EXISTS "${_wx_candidate}/include/wx/wx.h")
                set(wxWidgets_ROOT_DIR "${_wx_candidate}" CACHE PATH "wxWidgets root directory")
                break()
            endif()
        endforeach()
    endif()
endif()
# Point FindWxWidgets at the right lib subdirectory (vc_x64_lib for 64-bit MSVC).
if(WIN32 AND wxWidgets_ROOT_DIR AND NOT wxWidgets_LIB_DIR)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(wxWidgets_LIB_DIR "${wxWidgets_ROOT_DIR}/lib/vc_x64_lib"
            CACHE PATH "wxWidgets library directory")
    else()
        set(wxWidgets_LIB_DIR "${wxWidgets_ROOT_DIR}/lib/vc_lib"
            CACHE PATH "wxWidgets library directory")
    endif()
endif()

find_package(wxWidgets 3.3 REQUIRED
    COMPONENTS core base gl net xml aui propgrid qa html)
include(${wxWidgets_USE_FILE})
# Ensure wx lib dir is in the linker search path for all targets (covers
# transitive #pragma comment(lib,...) references inside the wx static libs).
if(WIN32 AND wxWidgets_LIB_DIR)
    link_directories(${wxWidgets_LIB_DIR})
endif()

# ─── OpenGL ─────────────────────────────────────────────────────────────────
find_package(OpenGL REQUIRED)

# ─── FFmpeg ─────────────────────────────────────────────────────────────────
# vcpkg's FFmpeg port does not ship FFMPEGConfig.cmake so find_package(FFMPEG)
# in config mode fails. Use find_path / find_library for both paths — the vcpkg
# toolchain adds installed/ to CMAKE_PREFIX_PATH so the vcpkg-installed headers
# and libs are found automatically without extra hints.
if(XLIGHTS_USE_VCPKG)
    # _VCPKG_INSTALLED_DIR is set by vcpkg.cmake to the installed/ root.
    # Build an explicit hint so find_path/find_library don't depend on
    # CMAKE_PREFIX_PATH propagation order.
    if(DEFINED _VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        set(_vcpkg_triplet_dir "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
    elseif(DEFINED ENV{VCPKG_ROOT} AND DEFINED VCPKG_TARGET_TRIPLET)
        set(_vcpkg_triplet_dir "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}")
    else()
        set(_vcpkg_triplet_dir "")
    endif()

    find_path(FFMPEG_INCLUDE_DIR libavcodec/avcodec.h
        HINTS "${_vcpkg_triplet_dir}/include"
        NO_DEFAULT_PATH)
    if(NOT FFMPEG_INCLUDE_DIR)
        message(FATAL_ERROR "FFmpeg headers not found in '${_vcpkg_triplet_dir}/include'."
                            " Run: vcpkg install ffmpeg[avcodec,avformat,avfilter,avutil,swscale,swresample]")
    endif()
    foreach(_ff avcodec avformat avutil avfilter swscale swresample)
        string(TOUPPER ${_ff} _FF_UPPER)
        find_library(${_FF_UPPER}_LIB NAMES ${_ff}
            HINTS "${_vcpkg_triplet_dir}/lib"
            NO_DEFAULT_PATH)
        if(NOT ${_FF_UPPER}_LIB)
            message(FATAL_ERROR "FFmpeg lib '${_ff}' not found in '${_vcpkg_triplet_dir}/lib'.")
        endif()
        list(APPEND FFMPEG_LIBS ${${_FF_UPPER}_LIB})
    endforeach()
else()
    find_path(FFMPEG_INCLUDE_DIR libavcodec/avcodec.h
        HINTS ${XL_INC}/ffmpeg-6/include
        PATHS /usr/include /usr/local/include
        NO_DEFAULT_PATH)
    if(NOT FFMPEG_INCLUDE_DIR)
        find_path(FFMPEG_INCLUDE_DIR libavcodec/avcodec.h)
    endif()
    set(_FF_HINTS ${XL_MAC_LIB} ${XL_LIB}/windows64 /usr/lib /usr/local/lib)
    foreach(_ff avcodec avformat avutil avfilter swscale swresample)
        string(TOUPPER ${_ff} _FF_UPPER)
        find_library(${_FF_UPPER}_LIB NAMES ${_ff} HINTS ${_FF_HINTS})
        list(APPEND FFMPEG_LIBS ${${_FF_UPPER}_LIB})
    endforeach()
endif()

# ─── SDL2 ───────────────────────────────────────────────────────────────────
# Try cmake config first (works on all platforms when SDL2 is installed with cmake).
# Skip on Apple-with-deps so we don't pull in /opt/homebrew/include — that
# breaks sol2's `__has_include(<lua/lua.h>)` probe by exposing Homebrew's lua.
if(NOT XL_MAC_DEPS)
    find_package(SDL2 QUIET CONFIG)
endif()
if(SDL2_FOUND)
    # Modern SDL2 cmake config: target SDL2::SDL2
    set(SDL2_LINK_TARGET SDL2::SDL2)
    set(SDL2_INC "")
else()
    # Manual fallback — vendored on Windows, system on Linux.
    find_path(SDL2_INCLUDE_DIR SDL.h
        HINTS ${XL_DEPS}/sdl/include
        PATHS /usr/include/SDL2 /usr/local/include/SDL2
        NO_DEFAULT_PATH)
    if(NOT SDL2_INCLUDE_DIR)
        find_path(SDL2_INCLUDE_DIR SDL.h PATH_SUFFIXES SDL2)
    endif()
    find_library(SDL2_LIB NAMES SDL2 SDL2-static
        HINTS ${XL_MAC_LIB} ${XL_LIB}/windows64
        PATHS /usr/lib /usr/local/lib)
    set(SDL2_LINK_TARGET ${SDL2_LIB})
    set(SDL2_INC ${SDL2_INCLUDE_DIR})
endif()

# ─── zstd ───────────────────────────────────────────────────────────────────
find_package(zstd QUIET CONFIG)
if(TARGET zstd::libzstd_static OR TARGET zstd::libzstd_shared)
    if(TARGET zstd::libzstd_static)
        set(ZSTD_LINK_TARGET zstd::libzstd_static)
    else()
        set(ZSTD_LINK_TARGET zstd::libzstd_shared)
    endif()
    set(ZSTD_INC "")
else()
    find_path(ZSTD_INCLUDE_DIR zstd.h
        HINTS ${XL_INC}
        PATHS /usr/include /usr/local/include)
    if(WIN32)
        # libzstd_static.lib is MinGW-built and contains ___chkstk_ms (MSVC-incompatible).
        # Prefer libzstd_static_VS.lib (MSVC CRT) and its debug variant.
        find_library(ZSTD_LIB_RELEASE NAMES libzstd_static_VS zstd
            HINTS ${XL_LIB}/windows64)
        find_library(ZSTD_LIB_DEBUG NAMES libzstdd_static_VS libzstd_static_VS zstd
            HINTS ${XL_LIB}/windows64)
        set(ZSTD_LINK_TARGET optimized ${ZSTD_LIB_RELEASE} debug ${ZSTD_LIB_DEBUG})
    else()
        find_library(ZSTD_LIB NAMES zstd libzstd_static libzstd_static_VS
            HINTS ${XL_MAC_LIB}
            PATHS /usr/lib /usr/local/lib)
        set(ZSTD_LINK_TARGET ${ZSTD_LIB})
    endif()
    set(ZSTD_INC ${ZSTD_INCLUDE_DIR})
endif()

# ─── libcurl ────────────────────────────────────────────────────────────────
# CMake's built-in FindCURL works without pkg-config on all platforms.
find_package(CURL QUIET)
if(CURL_FOUND)
    set(CURL_LINK_TARGET CURL::libcurl)
    set(CURL_INC "")
else()
    find_path(CURL_INCLUDE_DIR curl/curl.h
        HINTS ${XL_MAC_INC} ${XL_INC}
        PATHS /usr/include /usr/local/include)
    find_library(CURL_LIB NAMES libcurl_msvc curl libcurl curl_imp libcurl_a
        HINTS ${XL_MAC_LIB} ${XL_LIB}/windows64
        PATHS /usr/lib /usr/local/lib)
    # The vcxproj links via #pragma comment(lib,"libcurl.dll.a") which MSVC accepts.
    # Fall back to the same vendored file when no .lib variant is found.
    if(NOT CURL_LIB AND MSVC AND EXISTS "${XL_LIB}/windows64/libcurl.dll.a")
        set(CURL_LIB "${XL_LIB}/windows64/libcurl.dll.a")
    endif()
    if(CURL_LIB)
        set(CURL_LINK_TARGET ${CURL_LIB})
    else()
        set(CURL_LINK_TARGET "")
        if(WIN32)
            message(WARNING "libcurl not found — install curl via vcpkg or set CURL_DIR.")
        endif()
    endif()
    set(CURL_INC "${CURL_INCLUDE_DIR}")
endif()

# ─── Lua (compiled from vendored source) ────────────────────────────────────
# Avoid relying on a pre-built .lib whose name varies across platforms/builds.
# lua.c and luac.c define their own main() — exclude them.
file(GLOB LUA_SRC ${XL_DEPS}/lua/src/*.c)
list(REMOVE_ITEM LUA_SRC
    ${XL_DEPS}/lua/src/lua.c
    ${XL_DEPS}/lua/src/luac.c)
set(LUA_INCLUDE_DIR ${XL_DEPS}/lua/src)

# sol2 probes `__has_include(<lua/lua.h>)` first; if a system install
# (e.g. Homebrew lua 5.5) is in the search path that probe wins, then
# sol2 errors on the wrong API. Stage the vendored lua 5.4 headers
# under a `lua/` subdir and add that as a PRIVATE -I so the probe
# resolves against vendored first.
set(LUA_STAGE_DIR ${CMAKE_BINARY_DIR}/lua_stage)
file(MAKE_DIRECTORY ${LUA_STAGE_DIR}/lua)
foreach(_h lua.h lualib.h lauxlib.h luaconf.h lua.hpp)
    configure_file(${XL_DEPS}/lua/src/${_h} ${LUA_STAGE_DIR}/lua/${_h} COPYONLY)
endforeach()

# ─── Source file collection ─────────────────────────────────────────────────

# Core (wx-free)
file(GLOB_RECURSE SRC_CORE
    src-core/XmlSerializer/*.cpp
    src-core/ai/*.cpp
    src-core/cad/*.cpp
    src-core/controllers/*.cpp
	src-core/diagnostics/*.cpp
    src-core/discovery/*.cpp
    src-core/import_export/*.cpp
    src-core/graphics/GLContextManager.cpp
    src-core/graphics/xlFontInfo.cpp
    src-core/graphics/xlGraphicsAccumulators.cpp
    src-core/graphics/xlMesh.cpp
    src-core/lyrics/*.cpp
    src-core/media/AudioLoader.cpp
    src-core/media/AIModelStore.cpp
    src-core/media/AudioManager.cpp
    src-core/media/ChordDetector.cpp
    src-core/media/FFmpegAudioDecoder.cpp
    src-core/media/FFmpegVideoReader.cpp
    src-core/media/MediaCompatibility.cpp
    src-core/media/OnsetDetector.cpp
    src-core/media/PitchDetector.cpp
    src-core/media/SDLAudioOutput.cpp
    src-core/media/Spectrogram.cpp
    src-core/media/StemSeparator.cpp
    src-core/media/TempoDetector.cpp
    src-core/media/VideoReader.cpp
    src-core/media/VideoTranscoder.cpp
    src-core/media/kiss_fft/kiss_fft.c
    src-core/media/kiss_fft/tools/kiss_fftr.c
    src-core/models/*.cpp
    src-core/models/DMX/*.cpp
    src-core/models/DMX/MovingHeads/*.cpp
    src-core/outputs/*.cpp
    src-core/render/*.cpp
    src-core/settings/*.cpp
    src-core/utils/*.cpp
)
if(WIN32)
    list(APPEND SRC_CORE src-core/media/WindowsHardwareVideoReader.cpp)
    list(REMOVE_ITEM SRC_CORE
        ${CMAKE_SOURCE_DIR}/src-core/outputs/serial_posix.cpp
        ${CMAKE_SOURCE_DIR}/src-core/outputs/serial_osx.cpp
        ${CMAKE_SOURCE_DIR}/src-core/outputs/serial_win32.cpp)
elseif(APPLE)
    # Match Xcode's EXCLUDED_SOURCE_FILE_NAMES on macOS: serial.cpp is the
    # cross-platform impl that already covers the POSIX path on Apple, so
    # serial_osx.cpp would double-define SerialPort::Open et al.
    list(REMOVE_ITEM SRC_CORE
        ${CMAKE_SOURCE_DIR}/src-core/media/WindowsHardwareVideoReader.cpp
        ${CMAKE_SOURCE_DIR}/src-core/outputs/serial_win32.cpp
        ${CMAKE_SOURCE_DIR}/src-core/outputs/serial_posix.cpp
        ${CMAKE_SOURCE_DIR}/src-core/outputs/serial_osx.cpp
        ${CMAKE_SOURCE_DIR}/src-core/outputs/uDMXOutput.cpp
        ${CMAKE_SOURCE_DIR}/src-core/outputs/uDMXOutput.h)
else()
    list(REMOVE_ITEM SRC_CORE
        ${CMAKE_SOURCE_DIR}/src-core/media/WindowsHardwareVideoReader.cpp
        ${CMAKE_SOURCE_DIR}/src-core/outputs/serial_win32.cpp
        ${CMAKE_SOURCE_DIR}/src-core/outputs/serial_osx.cpp)
endif()

# uDMX requires libusb — detect here so SRC_CORE can be filtered before add_executable
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(LIBUSB QUIET libusb-1.0)
endif()
if(NOT LIBUSB_FOUND)
    find_library(LIBUSB_LIBRARIES NAMES usb-1.0 usb)
    find_path(LIBUSB_INCLUDE_DIRS NAMES libusb.h PATH_SUFFIXES libusb-1.0)
    if(LIBUSB_LIBRARIES AND LIBUSB_INCLUDE_DIRS)
        set(LIBUSB_FOUND TRUE)
    endif()
endif()
if(NOT LIBUSB_FOUND)
    message(STATUS "libusb not found — uDMX output disabled")
    list(REMOVE_ITEM SRC_CORE
        ${CMAKE_SOURCE_DIR}/src-core/outputs/uDMXOutput.h
        ${CMAKE_SOURCE_DIR}/src-core/outputs/uDMXOutput.cpp)
else()
    message(STATUS "libusb found — uDMX output enabled")
endif()

# Effects
file(GLOB SRC_EFFECTS
    src-core/effects/*.cpp
    src-core/effects/ispc/ISPCComputeUtilities.cpp
)
# ─── ISPC (Intel SPMD Program Compiler — SIMD kernels) ──────────────────────
# Visual Studio generators do not support enable_language(ISPC).
# Use it on Linux/macOS (Ninja/Makefile), fall back to add_custom_command on VS.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
    set(_ispc_targets "neon-i32x8")
    set(_ispc_arch    "aarch64")
else()
    set(_ispc_targets "avx2-i32x16,avx1-i32x16,sse4.2-i32x8,sse2-i32x8")
    set(_ispc_arch    "x86_64")
endif()

if(NOT WIN32)
    # ── Linux / macOS: native CMake ISPC language support ──────────────────
    if(XL_MAC_DEPS AND NOT CMAKE_ISPC_COMPILER
       AND EXISTS "${XL_MAC_DEPS}/bin/ispc")
        set(CMAKE_ISPC_COMPILER "${XL_MAC_DEPS}/bin/ispc" CACHE FILEPATH "ispc")
    endif()
    include(CheckLanguage)
    check_language(ISPC)
    if(CMAKE_ISPC_COMPILER)
        enable_language(ISPC)
        set(CMAKE_ISPC_FLAGS "--target=${_ispc_targets} --arch=${_ispc_arch} --pic")
        file(GLOB _ispc_sources "${CMAKE_SOURCE_DIR}/src-core/effects/ispc/*.ispc")
        list(APPEND SRC_EFFECTS ${_ispc_sources})
        message(STATUS "ISPC: using enable_language(ISPC) — ${CMAKE_ISPC_COMPILER}")
    else()
        message(WARNING "ispc not found — SIMD kernels disabled (install ISPC and add to PATH)")
    endif()
else()
    # ── Windows / Visual Studio: add_custom_command per .ispc file ──────────
    # VS generators reject enable_language(ISPC); drive ispc.exe manually.
    find_program(ISPC_EXECUTABLE NAMES ispc ispc.exe
        HINTS "${CMAKE_SOURCE_DIR}"
        PATH_SUFFIXES bin)
    if(ISPC_EXECUTABLE)
        message(STATUS "ISPC: using add_custom_command — ${ISPC_EXECUTABLE}")
        # Multi-target produces: Name.obj (dispatcher) + Name_avx2/_avx/_sse4/_sse2.obj
        # Note: ISPC uses "_avx" (not "_avx1") as the suffix for the avx1-i32x16 target.
        set(_ispc_suffixes "_avx2" "_avx" "_sse4" "_sse2")
        set(_ispc_obj_dir "${CMAKE_CURRENT_BINARY_DIR}/ispc_objs")
        file(GLOB _ispc_sources "${CMAKE_SOURCE_DIR}/src-core/effects/ispc/*.ispc")
        set(ISPC_OBJECTS "")
        foreach(_src ${_ispc_sources})
            get_filename_component(_name "${_src}" NAME_WE)
            set(_primary "${_ispc_obj_dir}/${_name}.obj")
            set(_outputs "${_primary}")
            foreach(_sfx ${_ispc_suffixes})
                list(APPEND _outputs "${_ispc_obj_dir}/${_name}${_sfx}.obj")
            endforeach()
            add_custom_command(
                OUTPUT  ${_outputs}
                COMMAND "${CMAKE_COMMAND}" -E make_directory "${_ispc_obj_dir}"
                COMMAND "${ISPC_EXECUTABLE}" "${_src}"
                        "--target=${_ispc_targets}" "--arch=${_ispc_arch}"
                        -o "${_primary}"
                DEPENDS "${_src}"
                COMMENT "ISPC: ${_name}"
                VERBATIM
            )
            list(APPEND ISPC_OBJECTS ${_outputs})
        endforeach()
        set_source_files_properties(${ISPC_OBJECTS}
            PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)
    else()
        message(WARNING "ispc not found — SIMD kernels disabled (install ISPC and add to PATH)")
    endif()
endif()

# UI (wxWidgets-dependent)
file(GLOB_RECURSE SRC_UI
    src-ui-wx/xLightsApp.cpp
    src-ui-wx/xLightsMain.cpp
    src-ui-wx/ai/*.cpp
    src-ui-wx/app-shell/*.cpp
    src-ui-wx/automation/*.cpp
    src-ui-wx/color/*.cpp
    src-ui-wx/controllerproperties/*.cpp
    src-ui-wx/controllers/*.cpp
    src-ui-wx/diagnostics/*.cpp
    src-ui-wx/effectpanels/*.cpp
    src-ui-wx/effects/*.cpp
    src-ui-wx/graphics/opengl/xlGLCanvas.cpp
    src-ui-wx/graphics/opengl/xlOGL3GraphicsContext.cpp
    src-ui-wx/graphics/wxTextDrawingContext.cpp
    src-ui-wx/import_export/*.cpp
    src-ui-wx/layout/*.cpp
    src-ui-wx/media/*.cpp
    src-ui-wx/model/*.cpp
    src-ui-wx/modelproperties/*.cpp
    src-ui-wx/preferences/*.cpp
    src-ui-wx/render/*.cpp
    src-ui-wx/sequencer/*.cpp
    src-ui-wx/settings/*.cpp
    src-ui-wx/setup/*.cpp
    src-ui-wx/shared/controls/*.cpp
    src-ui-wx/shared/dialogs/*.cpp
    src-ui-wx/shared/utils/*.cpp
    src-ui-wx/support/*.cpp
)

# Vendored third-party sources
set(SRC_DEPS
    common/xlBaseApp.cpp
    dependencies/md5/md5.cpp
    dependencies/midifile/src/Binasc.cpp
    dependencies/midifile/src/MidiEvent.cpp
    dependencies/midifile/src/MidiEventList.cpp
    dependencies/midifile/src/MidiFile.cpp
    dependencies/midifile/src/MidiMessage.cpp
    dependencies/pugixml/src/pugixml.cpp
    dependencies/vamp-hostsdk/FFT.cpp
    dependencies/vamp-hostsdk/Files.cpp
    dependencies/vamp-hostsdk/PluginBufferingAdapter.cpp
    dependencies/vamp-hostsdk/PluginChannelAdapter.cpp
    dependencies/vamp-hostsdk/PluginHostAdapter.cpp
    dependencies/vamp-hostsdk/PluginInputDomainAdapter.cpp
    dependencies/vamp-hostsdk/PluginLoader.cpp
    dependencies/vamp-hostsdk/PluginSummarisingAdapter.cpp
    dependencies/vamp-hostsdk/PluginWrapper.cpp
    dependencies/vamp-hostsdk/RealTime.cpp
    dependencies/vamp-hostsdk/acsymbols.c
    dependencies/vamp-hostsdk/host-c.cpp
    dependencies/wxHTTPServer/connection.cpp
    dependencies/wxHTTPServer/context.cpp
    dependencies/wxHTTPServer/message.cpp
    dependencies/wxHTTPServer/pages.cpp
    dependencies/wxHTTPServer/request.cpp
    dependencies/wxHTTPServer/response.cpp
    dependencies/wxHTTPServer/server.cpp
    dependencies/wxHTTPServer/sha1.cpp
    dependencies/wxHTTPServer/status.cpp
    dependencies/wxJSON/jsonreader.cpp
    dependencies/wxJSON/jsonval.cpp
    dependencies/wxJSON/jsonwriter.cpp
    dependencies/wxLED/wxLED.cpp
    dependencies/libxlsxwriter/third_party/minizip/ioapi.c
    dependencies/libxlsxwriter/third_party/minizip/unzip.c
    dependencies/libxlsxwriter/third_party/minizip/zip.c
    $<$<BOOL:${WIN32}>:${CMAKE_SOURCE_DIR}/dependencies/libxlsxwriter/third_party/minizip/iowin32.c>
)

# ─── Executable ─────────────────────────────────────────────────────────────
if(WIN32)
    set(WIN32_RC xLights/resource.rc)
endif()

if(APPLE)
    # Stub source for Swift-dependent bridge symbols the cmake build
    # can't compile (see cmake/macOS_stubs/macOS_cmake_stubs.mm).
    set(XL_MACOS_CMAKE_STUBS
        ${CMAKE_SOURCE_DIR}/cmake/macOS_stubs/macOS_cmake_stubs.mm)
    # Non-Swift Apple bridges — same .mm files the Xcode build compiles.
    # Anything that includes xLights_{Apple_core,macOSLib_UI}-Swift.h is
    # skipped here and instead stubbed in macOS_cmake_stubs.mm.
    set(XL_MACOS_BRIDGES
        # Public/core side (src-core/media/*.mm): these are the classes
        # AudioManager / VideoReader / etc. actually instantiate on Apple.
        ${CMAKE_SOURCE_DIR}/src-core/media/AudioToolboxDecoder.mm
        ${CMAKE_SOURCE_DIR}/src-core/media/AVAudioEngineOutput.mm
        ${CMAKE_SOURCE_DIR}/src-core/media/AVFoundationVideoReader.mm
        ${CMAKE_SOURCE_DIR}/src-core/media/MediaCompatibility.mm
        ${CMAKE_SOURCE_DIR}/src-core/media/SoundClassifier.mm
        ${CMAKE_SOURCE_DIR}/src-core/media/StemSeparator.mm
        # Apple-platform back-ends (macOS/src-apple-core/): pure ObjC++,
        # no Swift bridging required.
        ${CMAKE_SOURCE_DIR}/macOS/src-apple-core/media/AudioToolboxBridge.mm
        ${CMAKE_SOURCE_DIR}/macOS/src-apple-core/media/AVAudioEngineOutputBridge.mm
        ${CMAKE_SOURCE_DIR}/macOS/src-apple-core/media/AVFoundationVideoBridge.mm
        ${CMAKE_SOURCE_DIR}/macOS/src-apple-core/media/MediaCompatibilityBridge.mm
        # SoundClassifierBridge.mm is skipped — src-core/media/SoundClassifier.mm
        # already defines XLSoundClassifierObserver (looks like a duplicated
        # work-in-progress on this branch).
        ${CMAKE_SOURCE_DIR}/macOS/src-apple-core/media/StemSeparatorBridge.mm
        ${CMAKE_SOURCE_DIR}/macOS/src-apple-core/osxUtils/XLMetricKit.mm
        ${CMAKE_SOURCE_DIR}/macOS/src-apple-core/osxUtils/xlVideoToolboxUtils.mm
        # UI-side bridges in macOS/src-mac-ui/osxUtilsUI/.
        ${CMAKE_SOURCE_DIR}/macOS/src-mac-ui/osxUtilsUI/InAppPurchaseDialog.cpp
        ${CMAKE_SOURCE_DIR}/macOS/src-mac-ui/osxUtilsUI/osxInAppPurchases.mm
        ${CMAKE_SOURCE_DIR}/src-ui-wx/shared/utils/xLightsTimer.mm)
    # Allow bridge .mm files to find the headers next to them via
    # "osxUtils/Foo.h" / "osxUtilsUI/Foo.h" style includes used in
    # the broader codebase.
    list(APPEND XL_APPLE_BRIDGE_INCLUDES
        ${CMAKE_SOURCE_DIR}/macOS/src-apple-core
        ${CMAKE_SOURCE_DIR}/macOS/src-mac-ui)
    # The cmake build doesn't compile the Metal canvas / Metal bridges,
    # so force the OpenGL canvas via xlGraphicsBase.h's escape hatch
    # (adding macOS/src-mac-ui to the include path otherwise flips the
    # `__has_include("osxUtilsUI/wxMetalCanvas.hpp")` probe to true).
    add_compile_definitions(FORCE_OPENGL_BASE)
endif()

add_executable(xLights WIN32 MACOSX_BUNDLE
    ${SRC_CORE}
    ${SRC_EFFECTS}
    ${SRC_UI}
    ${SRC_DEPS}
    ${LUA_SRC}
    ${WIN32_RC}
    ${XL_MACOS_CMAKE_STUBS}
    ${XL_MACOS_BRIDGES}
)

# Add ISPC-generated object files to the link step.
if(ISPC_OBJECTS)
    target_sources(xLights PRIVATE ${ISPC_OBJECTS})
endif()

# ─── IDE source grouping (Visual Studio Solution Explorer) ──────────────────
# source_group(TREE ...) mirrors the on-disk directory tree so files appear
# in a matching folder hierarchy in the VS Solution Explorer.
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_CORE} ${SRC_EFFECTS})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_UI})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_DEPS} ${LUA_SRC})

# ─── Include directories ────────────────────────────────────────────────────
target_include_directories(xLights PRIVATE
    ${LUA_STAGE_DIR}
    ${CMAKE_SOURCE_DIR}/src-core
    ${CMAKE_SOURCE_DIR}/src-core/render
    ${CMAKE_SOURCE_DIR}/src-core/utils
    ${CMAKE_SOURCE_DIR}/src-ui-wx
    ${XL_APPLE_BRIDGE_INCLUDES}

    ${XL_DEPS}
    ${XL_DEPS}/spdlog/include
    ${XL_DEPS}/lua/src
    ${XL_DEPS}/pugixml/src
    ${XL_DEPS}/midifile/include
    ${XL_DEPS}/liquidfun/liquidfun/Box2D
    ${XL_DEPS}/libxlsxwriter/include
    ${XL_DEPS}/pybind11/include
    ${XL_INC}
    ${XL_INC}/zlib
    ${XL_INC}/sol2-3.5.0
    ${FFMPEG_INCLUDE_DIR}
    ${LUA_INCLUDE_DIR}
    ${SDL2_INC}
    ${ZSTD_INC}
    ${CURL_INC}
)

# ─── zlib ───────────────────────────────────────────────────────────────────
if(NOT XLIGHTS_USE_VCPKG)
    # Prefer z.lib (plain static MSVC build, no ASM deps).
    # zlibwapi.lib ships without its companion gvmat64.obj / inffas8664.obj
    # ASM objects and causes LNK2001 on x64 MSVC — keep it as last resort.
    # Windows-only hint: ${XL_LIB}/windows64 ships MinGW-built libz.a that
    # macOS ld silently mis-identifies and rejects ("archive member '/' not
    # a mach-o file") — keep it off the Apple/Linux search path.
    if(WIN32)
        set(_ZLIB_HINTS ${XL_LIB}/windows64)
    else()
        set(_ZLIB_HINTS /usr/lib /usr/local/lib)
    endif()
    find_library(ZLIB_LIB NAMES z zlib zlibstatic zlibwapi
        HINTS ${_ZLIB_HINTS})
    if(ZLIB_LIB)
        set(ZLIB_LINK_TARGET ${ZLIB_LIB})
    else()
        set(ZLIB_LINK_TARGET "")
    endif()
else()
    find_package(ZLIB REQUIRED)
    set(ZLIB_LINK_TARGET ZLIB::ZLIB)
endif()

# Suppress static-CRT conflict that a /GL-compiled zlibwapi.lib can trigger.
if(MSVC AND NOT XLIGHTS_USE_VCPKG)
    target_link_options(xLights PRIVATE
        $<$<CONFIG:Release>:/NODEFAULTLIB:LIBCMT>
        $<$<CONFIG:Debug>:/NODEFAULTLIB:LIBCMTD>)
endif()

# ─── Link libraries ─────────────────────────────────────────────────────────
if(FFMPEG_LIBRARY_DIRS)
    target_link_directories(xLights PRIVATE ${FFMPEG_LIBRARY_DIRS})
endif()
target_link_libraries(xLights PRIVATE
    ${wxWidgets_LIBRARIES}
    ${FFMPEG_LIBS}
    ${SDL2_LINK_TARGET}
    ${ZSTD_LINK_TARGET}
    ${ZLIB_LINK_TARGET}
)
# Lua is compiled from source (LUA_SRC) — no pre-built lib needed.
# curl is optional: MSVC-compatible .lib may not be present in the vendored tree.
if(CURL_LINK_TARGET)
    target_link_libraries(xLights PRIVATE ${CURL_LINK_TARGET})
endif()

# ─── WebP ───────────────────────────────────────────────────────────────────
# Required for wxWidgets WebP image support (wxUSE_WEBP is defined above).
# Try cmake config first (vcpkg provides WebP::webp and WebP::webpdemux targets).
# Skip on Apple-with-deps — wx ships libwxwebp-3.3 baked in, and Homebrew's
# WebPConfig.cmake would drag /opt/homebrew/include onto the system path
# (breaks sol2's `__has_include(<lua/lua.h>)` probe by exposing Homebrew lua).
if(NOT XL_MAC_DEPS)
    find_package(WebP QUIET CONFIG)
endif()
if(WebP_FOUND AND TARGET WebP::webp AND TARGET WebP::webpdemux)
    target_link_libraries(xLights PRIVATE WebP::webp WebP::webpdemux)
    message(STATUS "WebP found via cmake CONFIG package")
else()
    # Manual fallback — vendored on Windows, system on Linux.
    if(WIN32)
        find_library(WEBP_LIB NAMES libwebp webp
            HINTS ${XL_LIB}/windows64
            PATHS /usr/lib /usr/local/lib
            NO_DEFAULT_PATH)
        if(NOT WEBP_LIB)
            find_library(WEBP_LIB NAMES libwebp webp)
        endif()
        find_library(WEBPDEMUX_LIB NAMES libwebpdemux webpdemux
            HINTS ${XL_LIB}/windows64
            PATHS /usr/lib /usr/local/lib
            NO_DEFAULT_PATH)
        if(NOT WEBPDEMUX_LIB)
            find_library(WEBPDEMUX_LIB NAMES libwebpdemux webpdemux)
        endif()
    else()
        find_library(WEBP_LIB NAMES webp
            PATHS /usr/lib /usr/local/lib)
        find_library(WEBPDEMUX_LIB NAMES webpdemux
            PATHS /usr/lib /usr/local/lib)
    endif()

    if(WEBP_LIB AND WEBPDEMUX_LIB)
        target_link_libraries(xLights PRIVATE ${WEBP_LIB} ${WEBPDEMUX_LIB})
        message(STATUS "WebP found via vendored libs: ${WEBP_LIB}, ${WEBPDEMUX_LIB}")
    else()
        if(XLIGHTS_USE_VCPKG)
            message(FATAL_ERROR "XLIGHTS_USE_VCPKG=ON but WebP not found. Run: vcpkg install libwebp")
        else()
            message(WARNING "libwebp or libwebpdemux not found — WebP image support may fail to link")
        endif()
    endif()
endif()

# OpenGL 
target_link_libraries(xLights PRIVATE OpenGL::GL)
if(WIN32)
	target_link_libraries(xLights PRIVATE glu32)
elseif(UNIX AND NOT APPLE)
	target_link_libraries(xLights PRIVATE GL GLU)
endif()

# vcpkg FFmpeg ships avfilter with vsrc_gfxcapture_winrt compiled against the
# static CRT (/MT) regardless of triplet, because it uses C++/WinRT.
# Suppress the conflicting static-CRT default lib so unresolved symbols fall
# through to the dynamic CRT (MSVCP140[D].dll) that xLights links against.
if(MSVC AND XLIGHTS_USE_VCPKG)
    target_link_options(xLights PRIVATE
        $<$<CONFIG:Debug>:/NODEFAULTLIB:LIBCMTD>
        $<$<NOT:$<CONFIG:Debug>>:/NODEFAULTLIB:LIBCMT>
    )
endif()

# libxlsxwriter — no cmake config package; find_library works for both vcpkg and vendored.
find_library(XLSXWRITER_LIB NAMES xlsxwriter xlsxwriterd
    HINTS ${XL_MAC_LIB} ${XL_LIB}/windows64 ${XL_DEPS}/libxlsxwriter ${XL_DEPS}/libxlsxwriter/lib)
if(XLSXWRITER_LIB)
    target_link_libraries(xLights PRIVATE ${XLSXWRITER_LIB})
elseif(XLIGHTS_USE_VCPKG)
    message(FATAL_ERROR "XLIGHTS_USE_VCPKG=ON but libxlsxwriter not found. Run: vcpkg install libxlsxwriter")
endif()

# LiquidFun (not in vcpkg — always use vendored)
if(WIN32)
    find_library(LIQUIDFUN_LIB_RELEASE NAMES liquidfun
        HINTS ${XL_LIB}/windows64 ${XL_DEPS}/liquidfun)
    find_library(LIQUIDFUN_LIB_DEBUG NAMES liquidfund liquidfun
        HINTS ${XL_LIB}/windows64 ${XL_DEPS}/liquidfun)
    if(LIQUIDFUN_LIB_RELEASE)
        target_link_libraries(xLights PRIVATE
            optimized ${LIQUIDFUN_LIB_RELEASE}
            debug     ${LIQUIDFUN_LIB_DEBUG})
    endif()
else()
    find_library(LIQUIDFUN_LIB NAMES liquidfun
        HINTS ${XL_MAC_LIB} ${XL_LIB}/linux ${CMAKE_SOURCE_DIR}/lib/linux)
    if(LIQUIDFUN_LIB)
        target_link_libraries(xLights PRIVATE ${LIQUIDFUN_LIB})
    endif()
endif()

# hidapi
if(XLIGHTS_USE_VCPKG)
    find_package(hidapi CONFIG REQUIRED)
    target_link_libraries(xLights PRIVATE hidapi::hidapi)
endif()

# Platform link extras
if(WIN32)
    target_link_libraries(xLights PRIVATE ws2_32 winmm comctl32 rpcrt4
                                          mfuuid propsys wmcodecdspuuid
                                          iphlpapi)
    if(NOT XLIGHTS_USE_VCPKG)
        # vendored pre-built libs (FFmpeg, SDL2, etc.) + lua pragma stubs
        target_link_directories(xLights PRIVATE ${XL_LIB}/windows64 ${XL_DEPS}/lua/src)
    endif()
elseif(UNIX AND NOT APPLE)
    target_link_libraries(xLights PRIVATE dl pthread)
    # ALSA — use pkg-config only on Linux where it is always available
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(ALSA QUIET alsa)
        if(ALSA_FOUND)
            target_link_libraries(xLights PRIVATE ${ALSA_LIBRARIES})
            target_include_directories(xLights PRIVATE ${ALSA_INCLUDE_DIRS})
        endif()
    endif()
    # libusb for uDMX (detected earlier, before add_executable)
    if(LIBUSB_FOUND)
        target_include_directories(xLights PRIVATE ${LIBUSB_INCLUDE_DIRS})
        target_link_libraries(xLights PRIVATE ${LIBUSB_LIBRARIES})
    endif()
elseif(APPLE)
    # System libs referenced by FFmpeg / wx / xLights itself.
    target_link_libraries(xLights PRIVATE bz2 iconv)
    # Apple frameworks referenced by FFmpeg's avcodec/avformat (VideoToolbox,
    # CoreMedia, AudioToolbox) and by various Apple bridges that the cmake
    # build skips compiling — Foundation/AppKit/Cocoa are needed for any
    # macOS wx app, Security for keychain, IOKit/SystemConfiguration via libcurl.
    target_link_libraries(xLights PRIVATE
        "-framework AVFoundation"
        "-framework Accelerate"    # vImage* (AVFoundationVideoReader)
        "-framework AppKit"
        "-framework ApplicationServices"
        "-framework AudioToolbox"
        "-framework AudioUnit"
        "-framework Carbon"
        "-framework Cocoa"
        "-framework CoreAudio"
        "-framework CoreFoundation"
        "-framework CoreImage"     # CIContext (VideoToolbox helpers)
        "-framework CoreMedia"
        "-framework CoreML"        # MLModel* (StemSeparator)
        "-framework CoreServices"
        "-framework CoreText"
        "-framework CoreVideo"
        "-framework Foundation"
        "-framework IOKit"
        "-framework ImageIO"
        "-framework Metal"
        "-framework MetalKit"
        "-framework MetricKit"
        "-framework OpenGL"
        "-framework QuartzCore"
        "-framework Security"
        "-framework SoundAnalysis" # SN* (SoundClassifier)
        "-framework StoreKit"      # SK* (InAppPurchases)
        "-framework SystemConfiguration"
        "-framework UniformTypeIdentifiers"
        "-framework VideoToolbox"
    )
    # libpostproc (FFmpeg companion, in the deps tree but not in FFMPEG_LIBS),
    # libhidapi (referenced by core controller discovery).
    find_library(POSTPROC_LIB NAMES postproc HINTS ${XL_MAC_LIB})
    if(POSTPROC_LIB)
        target_link_libraries(xLights PRIVATE ${POSTPROC_LIB})
    endif()
    find_library(HIDAPI_LIB NAMES hidapi HINTS ${XL_MAC_LIB})
    if(HIDAPI_LIB)
        target_link_libraries(xLights PRIVATE ${HIDAPI_LIB})
    endif()
endif()

# ─── Stem separation backends ───────────────────────────────────────────────
# XLIGHTS_CMAKE_BUILD is defined globally above, so #pragma comment(lib,...)
# in StemSeparator.cpp is already suppressed — no per-block define needed.
#
# Usage:
#   cmake -DXLIGHTS_USE_ORT=ON      # ONNX Runtime (vcpkg: onnxruntime[directml])
#   cmake -DXLIGHTS_USE_OPENVINO=ON # OpenVINO     (vcpkg: openvino)

# Default OFF on Apple — the macOS prebuilt deps tree doesn't ship onnxruntime.
# Stem separation on macOS uses the CoreML backend via AVAudioEngine instead.
if(APPLE)
    option(XLIGHTS_USE_ORT  "Enable ONNX Runtime stem separation" OFF)
else()
    option(XLIGHTS_USE_ORT  "Enable ONNX Runtime stem separation" ON)
endif()
option(XLIGHTS_USE_OPENVINO "Enable OpenVINO stem separation"     OFF)

if(XLIGHTS_USE_ORT)
    find_package(onnxruntime CONFIG
        HINTS
            "${CMAKE_SOURCE_DIR}/include/microsoft.ml.onnxruntime.directml.1.19.2"
            "${CMAKE_SOURCE_DIR}/include/onnxruntime-win-x64-gpu-1.25.0"
    )
    if(onnxruntime_FOUND)
        target_link_libraries(xLights PRIVATE onnxruntime::onnxruntime)
        message(STATUS "Stem separation: ONNX Runtime (cmake package)")
    else()
        find_path(ORT_INCLUDE_DIR onnxruntime_cxx_api.h
            HINTS
                "${CMAKE_SOURCE_DIR}/include/microsoft.ml.onnxruntime.directml.1.19.2/build/native/include"
                "${CMAKE_SOURCE_DIR}/include/onnxruntime-win-x64-gpu-1.25.0/include"
        )
        find_library(ORT_LIB onnxruntime
            HINTS
                "${CMAKE_SOURCE_DIR}/include/microsoft.ml.onnxruntime.directml.1.19.2/runtimes/win-x64/native"
                "${CMAKE_SOURCE_DIR}/include/onnxruntime-win-x64-gpu-1.25.0/lib"
        )
        if(ORT_INCLUDE_DIR AND ORT_LIB)
            target_include_directories(xLights PRIVATE ${ORT_INCLUDE_DIR})
            target_link_libraries(xLights PRIVATE ${ORT_LIB})
            message(STATUS "Stem separation: ONNX Runtime (vendored at ${ORT_LIB})")
        else()
            message(FATAL_ERROR
                "XLIGHTS_USE_ORT=ON but onnxruntime not found.\n"
                "Install via vcpkg: vcpkg install onnxruntime[directml]\n"
                "Or set ORT_INCLUDE_DIR and ORT_LIB manually.")
        endif()
    endif()
    target_compile_definitions(xLights PRIVATE HAVE_ORT)
endif()

if(XLIGHTS_USE_OPENVINO)
    find_package(OpenVINO REQUIRED COMPONENTS Runtime)
    target_link_libraries(xLights PRIVATE openvino::runtime)
    target_compile_definitions(xLights PRIVATE HAVE_OPENVINO)
    message(STATUS "Stem separation: OpenVINO enabled")
endif()


# ─── Windows post-build: copy DLLs and resources to output dir ──────────────
# Mirrors the PostBuildEvent in xLights/Xlights.vcxproj.
if(WIN32)
    # Use safe_copy_dlls.cmake so the copy is a no-op when the build output
    # directory is the same as the source bin/ directory (happens on CI when
    # Ninja builds directly into bin/).  Plain `copy` fails with "cannot copy
    # file onto itself" in that case.
    set(_SAFE_COPY "${CMAKE_SOURCE_DIR}/cmake/safe_copy_dlls.cmake")
    add_custom_command(TARGET xLights POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            "-DSRC_DIR=${CMAKE_SOURCE_DIR}/bin"
            "-DDST_DIR=$<TARGET_FILE_DIR:xLights>"
            "-DGLOB_PAT=*.dll"
            -P "${_SAFE_COPY}"
        COMMAND ${CMAKE_COMMAND}
            "-DSRC_DIR=${CMAKE_SOURCE_DIR}/bin64"
            "-DDST_DIR=$<TARGET_FILE_DIR:xLights>"
            "-DGLOB_PAT=*.dll"
            -P "${_SAFE_COPY}"
        COMMAND ${CMAKE_COMMAND}
            "-DSRC_DIR=${CMAKE_SOURCE_DIR}/bin"
            "-DDST_DIR=$<TARGET_FILE_DIR:xLights>"
            "-DGLOB_PAT=*.cfg"
            -P "${_SAFE_COPY}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/resources/dictionaries"
            "$<TARGET_FILE_DIR:xLights>/dictionaries"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/resources/effectmetadata"
            "$<TARGET_FILE_DIR:xLights>/effectmetadata"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/bin64/Vamp"
            "$<TARGET_FILE_DIR:xLights>/Vamp"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/resources/prompts"
            "$<TARGET_FILE_DIR:xLights>/prompts"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/resources/html"
            "$<TARGET_FILE_DIR:xLights>/html"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/resources/controllers"
            "$<TARGET_FILE_DIR:xLights>/controllers"
        COMMENT "Copying DLLs and resources to output directory"
    )
endif()

# ─── Install ────────────────────────────────────────────────────────────────
include(GNUInstallDirs)
install(TARGETS xLights DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY resources/ DESTINATION ${CMAKE_INSTALL_DATADIR}/xLights)

# ═══════════════════════════════════════════════════════════════════════════════
# xLights Qt — Phase 2+3: effect metadata + generic JSON-driven effect panel
# Enable with: cmake -DXLIGHTS_BUILD_QT=ON
# ═══════════════════════════════════════════════════════════════════════════════
option(XLIGHTS_BUILD_QT "Build the Qt6 UI target (xLights-Qt)" OFF)

if(XLIGHTS_BUILD_QT)
    find_package(Qt6 6.7 REQUIRED COMPONENTS Widgets Gui GuiPrivate Network Multimedia)

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    # ── nlohmann/json ─────────────────────────────────────────────────────
    find_package(nlohmann_json CONFIG QUIET)
    if(NOT nlohmann_json_FOUND)
        include(FetchContent)
        FetchContent_Declare(nlohmann_json_qt
            URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
            URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
        FetchContent_MakeAvailable(nlohmann_json_qt)
        set(XL_QT_JSON_TARGET nlohmann_json::nlohmann_json)
    else()
        set(XL_QT_JSON_TARGET nlohmann_json::nlohmann_json)
    endif()

    # ── spdlog ────────────────────────────────────────────────────────────
    find_package(spdlog CONFIG QUIET)
    if(NOT spdlog_FOUND)
        find_path(SPDLOG_INCLUDE_DIR spdlog/spdlog.h
            HINTS ${CMAKE_SOURCE_DIR}/dependencies/spdlog/include
                  ${CMAKE_SOURCE_DIR}/include)
        if(SPDLOG_INCLUDE_DIR)
            add_library(spdlog_qt INTERFACE)
            target_include_directories(spdlog_qt INTERFACE ${SPDLOG_INCLUDE_DIR})
            set(XL_QT_SPDLOG_TARGET spdlog_qt)
        else()
            message(WARNING "xLights-Qt: spdlog not found — logging will be absent")
            set(XL_QT_SPDLOG_TARGET "")
        endif()
    else()
        set(XL_QT_SPDLOG_TARGET spdlog::spdlog)
    endif()

    # ── Sources ───────────────────────────────────────────────────────────
    set(XL_QT_SOURCES
        src-ui-qt/App/main.cpp
        src-ui-qt/App/MainWindow.cpp
        src-ui-qt/App/QtXLightsApp.cpp
        src-ui-qt/Graphics/RhiCanvas.cpp
        src-ui-qt/Effects/EffectPanelWidget.cpp
        src-ui-qt/Effects/EffectControlBuilder.cpp
        src-ui-qt/Effects/controls/SliderSpinRow.cpp
        src-ui-qt/Effects/controls/CheckRow.cpp
        src-ui-qt/Effects/controls/ComboRow.cpp
        src-ui-qt/Effects/controls/TextRow.cpp
        src-ui-qt/Effects/controls/FilePickerRow.cpp
        src-ui-qt/Sequencer/SequencerModel.cpp
        src-ui-qt/Sequencer/SequencerController.cpp
        src-ui-qt/Sequencer/SequencerGridCanvas.cpp
        src-ui-qt/Sequencer/RowHeaderWidget.cpp
        src-ui-qt/Sequencer/TimingRulerCanvas.cpp
        src-ui-qt/Sequencer/SequencerWidget.cpp
        src-ui-qt/Bridge/QtSequenceDoc.cpp
        src-ui-qt/Bridge/QtEffectRenderer.cpp
        src-ui-qt/Bridge/QtRenderBridge.cpp
        src-ui-qt/Bridge/QtRenderContext.cpp
        src-ui-qt/Bridge/QtTextDrawingContext.cpp
    )
    # /EHa makes catch(...) capture Windows SEH exceptions (access violations)
    # so QtRenderBridge can fall back to the software renderer on a src-core crash.
    if(MSVC)
        # /EHa: catch(...) also catches Windows SEH (access violations) so
        # src-core model init/render crashes fall back gracefully.
        set_source_files_properties(
            src-ui-qt/Bridge/QtRenderBridge.cpp
            src-ui-qt/Bridge/QtSequenceDoc.cpp
            PROPERTIES COMPILE_OPTIONS "/EHa")
    endif()
    list(APPEND XL_QT_SOURCES
        src-ui-qt/Bridge/FseqWriter.cpp
        src-ui-qt/App/ExportDialog.cpp
        $<$<BOOL:${WIN32}>:${CMAKE_CURRENT_SOURCE_DIR}/src-ui-qt/App/xLightsQt.rc>
        src-ui-qt/App/PlaybackController.cpp
        src-ui-qt/App/TransportToolBar.cpp
        src-ui-qt/Effects/EffectToolBar.cpp
        src-ui-qt/Effects/PaletteWidget.cpp
        src-ui-qt/Effects/BufferWidget.cpp
        src-ui-qt/Effects/controls/ColorSwatchButton.cpp
        src-ui-qt/Preview/PreviewWidget.cpp
        src-ui-qt/Effects/SubBufferWidget.cpp
        src-ui-qt/Info/ModelInfoWindow.cpp
        src-ui-qt/Info/ControllerInfoWindow.cpp
        src-ui-qt/Layout/HousePreviewWidget.cpp
        src-ui-qt/App/RenderDetailDialog.cpp
    )

    # ── pugixml — compile from vendored source (same as wx build) ─────────
    list(APPEND XL_QT_SOURCES
        ${CMAKE_SOURCE_DIR}/dependencies/pugixml/src/pugixml.cpp)
    set(XL_QT_PUGIXML_TARGET "")   # no lib target needed; source compiled in

    qt_add_executable(xLights-Qt ${XL_QT_SOURCES})

    # ── src-core + effects (compiled separately here) ──────────────────────────
    # Exclude files that depend on the wx canvas OpenGL layer or are truly
    # optional (video transcoder, stem separation).
    # AudioManager IS included (effects reference it); its deps come from
    # md5 and vamp SDK sources appended below.
    set(XL_QT_CORE ${SRC_CORE} ${SRC_EFFECTS})
    list(REMOVE_ITEM XL_QT_CORE
        # OpenGL extension func-pointers defined in wx canvas — stubbed in QtOpenGLStubs.cpp
        ${CMAKE_SOURCE_DIR}/src-core/effects/OpenGLShaders.cpp
        ${CMAKE_SOURCE_DIR}/src-core/graphics/GLContextManager.cpp
        # ShaderEffect IS included: it calls HasShaderSupport() first (returns false), so safe
        # Qt RHI replaces raw OpenGL; video transcoder not needed for preview
        ${CMAKE_SOURCE_DIR}/src-core/media/VideoTranscoder.cpp
        ${CMAKE_SOURCE_DIR}/src-core/media/StemSeparator.cpp
    )
    # AudioManager deps not in SRC_CORE/SRC_EFFECTS: md5, VAMP host SDK, midifile
    set(XL_QT_AUDIO_DEPS
        ${CMAKE_SOURCE_DIR}/dependencies/md5/md5.cpp
        ${CMAKE_SOURCE_DIR}/dependencies/vamp-hostsdk/FFT.cpp
        ${CMAKE_SOURCE_DIR}/dependencies/vamp-hostsdk/Files.cpp
        ${CMAKE_SOURCE_DIR}/dependencies/vamp-hostsdk/PluginBufferingAdapter.cpp
        ${CMAKE_SOURCE_DIR}/dependencies/vamp-hostsdk/PluginChannelAdapter.cpp
        ${CMAKE_SOURCE_DIR}/dependencies/vamp-hostsdk/PluginHostAdapter.cpp
        ${CMAKE_SOURCE_DIR}/dependencies/vamp-hostsdk/PluginInputDomainAdapter.cpp
        ${CMAKE_SOURCE_DIR}/dependencies/vamp-hostsdk/PluginLoader.cpp
        ${CMAKE_SOURCE_DIR}/dependencies/vamp-hostsdk/PluginSummarisingAdapter.cpp
        ${CMAKE_SOURCE_DIR}/dependencies/vamp-hostsdk/PluginWrapper.cpp
        ${CMAKE_SOURCE_DIR}/dependencies/vamp-hostsdk/RealTime.cpp
        ${CMAKE_SOURCE_DIR}/dependencies/vamp-hostsdk/acsymbols.c
        ${CMAKE_SOURCE_DIR}/dependencies/vamp-hostsdk/host-c.cpp
        ${CMAKE_SOURCE_DIR}/dependencies/midifile/src/Binasc.cpp
        ${CMAKE_SOURCE_DIR}/dependencies/midifile/src/MidiEvent.cpp
        ${CMAKE_SOURCE_DIR}/dependencies/midifile/src/MidiEventList.cpp
        ${CMAKE_SOURCE_DIR}/dependencies/midifile/src/MidiFile.cpp
        ${CMAKE_SOURCE_DIR}/dependencies/midifile/src/MidiMessage.cpp
    )
    # GL extension function pointer stubs compiled WITHOUT Qt include paths
    # so <GL/glext.h> doesn't conflict with Qt's own OpenGL abstractions.
    # Windows-only: macOS/Linux ship a full GL ABI that exports these symbols,
    # so defining them again here would collide at link time.
    if(WIN32)
        add_library(_xlights_qt_gl_stubs OBJECT
            src-ui-qt/Bridge/QtGLPointerDefs.cpp)
        target_include_directories(_xlights_qt_gl_stubs PRIVATE
            ${CMAKE_SOURCE_DIR}/dependencies)   # provides GL/glext.h
        target_sources(xLights-Qt PRIVATE $<TARGET_OBJECTS:_xlights_qt_gl_stubs>)
    endif()

    # minizip — SequencePackage uses unz* functions; same sources as main target
    set(XL_QT_MINIZIP
        ${CMAKE_SOURCE_DIR}/dependencies/libxlsxwriter/third_party/minizip/ioapi.c
        ${CMAKE_SOURCE_DIR}/dependencies/libxlsxwriter/third_party/minizip/unzip.c
        ${CMAKE_SOURCE_DIR}/dependencies/libxlsxwriter/third_party/minizip/zip.c
        $<$<BOOL:${WIN32}>:${CMAKE_SOURCE_DIR}/dependencies/libxlsxwriter/third_party/minizip/iowin32.c>
    )
    target_sources(xLights-Qt PRIVATE
        ${XL_QT_CORE} ${XL_QT_AUDIO_DEPS} ${XL_QT_MINIZIP} ${LUA_SRC}
        src-ui-qt/Bridge/QtOpenGLStubs.cpp)
    if(ISPC_OBJECTS)
        target_sources(xLights-Qt PRIVATE ${ISPC_OBJECTS})
    endif()
    if(APPLE)
        # Apple bridge .mm files + stub for Swift-dep symbols — same as
        # the wx target, minus xLightsTimer.mm (wx-typed) and
        # InAppPurchaseDialog.cpp (wx dialog).
        set(XL_QT_MACOS_BRIDGES ${XL_MACOS_BRIDGES})
        list(REMOVE_ITEM XL_QT_MACOS_BRIDGES
            ${CMAKE_SOURCE_DIR}/src-ui-wx/shared/utils/xLightsTimer.mm
            ${CMAKE_SOURCE_DIR}/macOS/src-mac-ui/osxUtilsUI/InAppPurchaseDialog.cpp
            ${CMAKE_SOURCE_DIR}/macOS/src-mac-ui/osxUtilsUI/osxInAppPurchases.mm)
        target_sources(xLights-Qt PRIVATE
            ${XL_QT_MACOS_BRIDGES} ${XL_MACOS_CMAKE_STUBS})
    endif()

    target_include_directories(xLights-Qt PRIVATE
        ${LUA_STAGE_DIR}
        src-ui-qt
        ${CMAKE_SOURCE_DIR}/dependencies/pugixml/src
        # ── src-core include paths (mirror xLights target) ──────────────────
        ${CMAKE_SOURCE_DIR}/src-core
        ${CMAKE_SOURCE_DIR}/src-core/render
        ${CMAKE_SOURCE_DIR}/src-core/utils
        ${XL_APPLE_BRIDGE_INCLUDES}
        ${XL_DEPS}
        ${XL_DEPS}/spdlog/include
        ${XL_DEPS}/lua/src
        ${XL_DEPS}/midifile/include
        ${XL_DEPS}/liquidfun/liquidfun/Box2D
        ${XL_DEPS}/libxlsxwriter/include
        ${XL_DEPS}/pybind11/include
        ${XL_INC}
        ${XL_INC}/zlib
        ${XL_INC}/sol2-3.5.0
        ${FFMPEG_INCLUDE_DIR}
        ${SDL2_INC}
        ${ZSTD_INC}
        ${CURL_INC}
    )

    target_link_libraries(xLights-Qt PRIVATE
        Qt6::Widgets
        Qt6::Gui
        Qt6::GuiPrivate
        Qt6::Network
        Qt6::Multimedia
        ${XL_QT_JSON_TARGET}
        ${XL_QT_SPDLOG_TARGET}
        ${XL_QT_PUGIXML_TARGET}
        # ── src-core deps ────────────────────────────────────────────────────
        ${FFMPEG_LIBS}
        ${SDL2_LINK_TARGET}
        ${ZSTD_LINK_TARGET}
        ${ZLIB_LINK_TARGET}
    )
    if(CURL_LINK_TARGET)
        target_link_libraries(xLights-Qt PRIVATE ${CURL_LINK_TARGET})
    endif()
    if(WebP_FOUND AND TARGET WebP::webp)
        target_link_libraries(xLights-Qt PRIVATE WebP::webp)
        if(TARGET WebP::webpdemux)
            target_link_libraries(xLights-Qt PRIVATE WebP::webpdemux)
        endif()
    elseif(WEBP_LIB)
        target_link_libraries(xLights-Qt PRIVATE ${WEBP_LIB})
        if(WEBPDEMUX_LIB)
            target_link_libraries(xLights-Qt PRIVATE ${WEBPDEMUX_LIB})
        endif()
    endif()
    if(XLSXWRITER_LIB)
        target_link_libraries(xLights-Qt PRIVATE ${XLSXWRITER_LIB})
    endif()
    if(WIN32 AND LIQUIDFUN_LIB_RELEASE)
        target_link_libraries(xLights-Qt PRIVATE
            optimized ${LIQUIDFUN_LIB_RELEASE} debug ${LIQUIDFUN_LIB_DEBUG})
    elseif(LIQUIDFUN_LIB)
        target_link_libraries(xLights-Qt PRIVATE ${LIQUIDFUN_LIB})
    endif()
    if(WIN32)
        if(FFMPEG_LIBRARY_DIRS)
            target_link_directories(xLights-Qt PRIVATE ${FFMPEG_LIBRARY_DIRS})
        endif()
        target_link_libraries(xLights-Qt PRIVATE
            ws2_32 winmm comctl32 rpcrt4 mfuuid propsys wmcodecdspuuid iphlpapi
            opengl32 glu32)
        if(NOT XLIGHTS_USE_VCPKG)
            target_link_directories(xLights-Qt PRIVATE
                ${XL_LIB}/windows64 ${XL_DEPS}/lua/src)
        endif()
        target_link_options(xLights-Qt PRIVATE
            $<$<CONFIG:Release>:/NODEFAULTLIB:LIBCMT>
            $<$<CONFIG:Debug>:/NODEFAULTLIB:LIBCMTD>)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(xLights-Qt PRIVATE dl pthread)
    elseif(APPLE)
        # Same system libs + Apple frameworks as the wx target.
        target_link_libraries(xLights-Qt PRIVATE bz2 iconv
            "-framework AVFoundation"
            "-framework Accelerate"
            "-framework AppKit"
            "-framework ApplicationServices"
            "-framework AudioToolbox"
            "-framework AudioUnit"
            "-framework Carbon"
            "-framework Cocoa"
            "-framework CoreAudio"
            "-framework CoreFoundation"
            "-framework CoreImage"
            "-framework CoreMedia"
            "-framework CoreML"
            "-framework CoreServices"
            "-framework CoreText"
            "-framework CoreVideo"
            "-framework Foundation"
            "-framework IOKit"
            "-framework ImageIO"
            "-framework Metal"
            "-framework MetalKit"
            "-framework MetricKit"
            "-framework OpenGL"
            "-framework QuartzCore"
            "-framework Security"
            "-framework SoundAnalysis"
            "-framework SystemConfiguration"
            "-framework UniformTypeIdentifiers"
            "-framework VideoToolbox")
        if(POSTPROC_LIB)
            target_link_libraries(xLights-Qt PRIVATE ${POSTPROC_LIB})
        endif()
        if(HIDAPI_LIB)
            target_link_libraries(xLights-Qt PRIVATE ${HIDAPI_LIB})
        endif()
    endif()

    set_target_properties(xLights-Qt PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin-qt"
        WIN32_EXECUTABLE TRUE
    )

    if(APPLE)
        set_target_properties(xLights-Qt PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_BUNDLE_NAME "xLights Qt"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1.0"
        )
    endif()

    # Copy effectmetadata next to the exe so QtXLightsApp finds it at runtime.
    add_custom_command(TARGET xLights-Qt POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/resources/effectmetadata"
            "$<TARGET_FILE_DIR:xLights-Qt>/effectmetadata"
        COMMENT "Copying effectmetadata to bin-qt")

    message(STATUS "xLights-Qt target enabled — output: bin-qt/")
endif()
