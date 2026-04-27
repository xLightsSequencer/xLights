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

if(MSVC)
    add_compile_options("/utf-8")
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
    COMPONENTS core base gl net xml aui propgrid ribbon stc media qa html)
include(${wxWidgets_USE_FILE})
# Ensure wx lib dir is in the linker search path for all targets (covers
# transitive #pragma comment(lib,...) references inside the wx static libs).
if(WIN32 AND wxWidgets_LIB_DIR)
    link_directories(${wxWidgets_LIB_DIR})
endif()

# ─── OpenGL ─────────────────────────────────────────────────────────────────
find_package(OpenGL REQUIRED)

# ─── FFmpeg ─────────────────────────────────────────────────────────────────
if(XLIGHTS_USE_VCPKG)
    # vcpkg FFmpeg uses variables, not imported targets.
    find_package(FFMPEG REQUIRED)
    set(FFMPEG_INCLUDE_DIR "${FFMPEG_INCLUDE_DIRS}")
    set(FFMPEG_LIBS ${FFMPEG_LIBRARIES})
else()
    find_path(FFMPEG_INCLUDE_DIR libavcodec/avcodec.h
        HINTS ${XL_INC}/ffmpeg-6/include
        PATHS /usr/include /usr/local/include
        NO_DEFAULT_PATH)
    if(NOT FFMPEG_INCLUDE_DIR)
        find_path(FFMPEG_INCLUDE_DIR libavcodec/avcodec.h)
    endif()
    set(_FF_HINTS ${XL_LIB}/windows64 /usr/lib /usr/local/lib)
    foreach(_ff avcodec avformat avutil swscale swresample)
        string(TOUPPER ${_ff} _FF_UPPER)
        find_library(${_FF_UPPER}_LIB NAMES ${_ff} HINTS ${_FF_HINTS})
        list(APPEND FFMPEG_LIBS ${${_FF_UPPER}_LIB})
    endforeach()
endif()

# ─── SDL2 ───────────────────────────────────────────────────────────────────
# Try cmake config first (works on all platforms when SDL2 is installed with cmake).
find_package(SDL2 QUIET CONFIG)
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
        HINTS ${XL_LIB}/windows64
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
        HINTS ${XL_INC}
        PATHS /usr/include /usr/local/include)
    # On Windows the vendored file is a MinGW .dll.a — not usable by MSVC.
    # Look for a proper .lib; if absent, skip linking (project may use WinHTTP).
    find_library(CURL_LIB NAMES curl libcurl curl_imp
        HINTS ${XL_LIB}/windows64
        PATHS /usr/lib /usr/local/lib)
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

# ─── Source file collection ─────────────────────────────────────────────────

# Core (wx-free)
file(GLOB_RECURSE SRC_CORE
    src-core/XmlSerializer/*.cpp
    src-core/ai/*.cpp
    src-core/cad/*.cpp
    src-core/controllers/*.cpp
    src-core/discovery/*.cpp
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
    list(REMOVE_ITEM SRC_CORE
        ${CMAKE_SOURCE_DIR}/src-core/media/WindowsHardwareVideoReader.cpp
        ${CMAKE_SOURCE_DIR}/src-core/outputs/serial_win32.cpp
        ${CMAKE_SOURCE_DIR}/src-core/outputs/serial_posix.cpp)
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
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64")
    set(_ispc_targets "neon-i32x8")
    set(_ispc_arch    "aarch64")
else()
    set(_ispc_targets "avx2-i32x16,avx1-i32x16,sse4.2-i32x8,sse2-i32x8")
    set(_ispc_arch    "x86_64")
endif()

if(NOT WIN32)
    # ── Linux / macOS: native CMake ISPC language support ──────────────────
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

add_executable(xLights WIN32 MACOSX_BUNDLE
    ${SRC_CORE}
    ${SRC_EFFECTS}
    ${SRC_UI}
    ${SRC_DEPS}
    ${LUA_SRC}
    ${WIN32_RC}
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
    ${CMAKE_SOURCE_DIR}/src-core
    ${CMAKE_SOURCE_DIR}/src-core/render
    ${CMAKE_SOURCE_DIR}/src-core/utils
    ${CMAKE_SOURCE_DIR}/src-ui-wx

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

# ─── Link libraries ─────────────────────────────────────────────────────────
if(FFMPEG_LIBRARY_DIRS)
    target_link_directories(xLights PRIVATE ${FFMPEG_LIBRARY_DIRS})
endif()
target_link_libraries(xLights PRIVATE
    ${wxWidgets_LIBRARIES}
    ${FFMPEG_LIBS}
    ${SDL2_LINK_TARGET}
    ${ZSTD_LINK_TARGET}
)
# Lua is compiled from source (LUA_SRC) — no pre-built lib needed.
# curl is optional: MSVC-compatible .lib may not be present in the vendored tree.
if(CURL_LINK_TARGET)
    target_link_libraries(xLights PRIVATE ${CURL_LINK_TARGET})
endif()

# ─── WebP ───────────────────────────────────────────────────────────────────
# Required for wxWidgets WebP image support (wxUSE_WEBP is defined above).
# Try cmake config first (vcpkg provides WebP::webp and WebP::webpdemux targets).
find_package(WebP QUIET CONFIG)
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
    HINTS ${XL_LIB}/windows64 ${XL_DEPS}/libxlsxwriter ${XL_DEPS}/libxlsxwriter/lib)
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
        HINTS ${XL_LIB}/linux ${CMAKE_SOURCE_DIR}/lib/linux)
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
                                          mfuuid propsys wmcodecdspuuid)
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
endif()

# ─── Stem separation backends ───────────────────────────────────────────────
# XLIGHTS_CMAKE_BUILD is defined globally above, so #pragma comment(lib,...)
# in StemSeparator.cpp is already suppressed — no per-block define needed.
#
# Usage:
#   cmake -DXLIGHTS_USE_ORT=ON      # ONNX Runtime (vcpkg: onnxruntime[directml])
#   cmake -DXLIGHTS_USE_OPENVINO=ON # OpenVINO     (vcpkg: openvino)

option(XLIGHTS_USE_ORT      "Enable ONNX Runtime stem separation" ON)
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
    add_custom_command(TARGET xLights POST_BUILD
        COMMAND cmd /c "copy /Y \"${CMAKE_SOURCE_DIR}\\bin\\*.dll\" \"$<TARGET_FILE_DIR:xLights>\\\""
        COMMAND cmd /c "copy /Y \"${CMAKE_SOURCE_DIR}\\bin64\\*.dll\" \"$<TARGET_FILE_DIR:xLights>\\\""
        COMMAND cmd /c "copy /Y \"${CMAKE_SOURCE_DIR}\\bin\\*.cfg\" \"$<TARGET_FILE_DIR:xLights>\\\""
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
