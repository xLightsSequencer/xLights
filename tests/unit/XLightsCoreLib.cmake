# Defines the static library `xlights_core` from the same source list the root
# build uses (cmake/XLightsCoreSources.cmake), with the third-party dependencies
# src-core needs. Used by the unit tests when -DXL_TEST_FULL_CORE=ON.
#
# Linux and macOS (outside Xcode) locate dependencies with pkg-config / find_*:
#   Debian/Ubuntu: libcurl4-openssl-dev libavcodec-dev libavformat-dev libavutil-dev
#     libavfilter-dev libswscale-dev libswresample-dev libzstd-dev libminizip-dev
#     libsdl2-dev libgl-dev libxlsxwriter-dev libusb-1.0-0-dev liblua5.4-dev zlib1g-dev
#     glslc glslang-dev (Linux: the Vulkan backend is built in, as in the app)
#   (plus the dependencies/ submodules: spdlog pugixml nanosvg liquidfun midifile
#   volk Vulkan-Headers VulkanMemoryAllocator)
# On Windows build from the root project with -DXL_BUILD_TESTS=ON instead - it has
# already located the dependency bundle (see XLightsCoreLibWindows.cmake).
#
# On macOS the Xcode project's xLights-unit-tests scheme links the existing
# xLights-core target and doesn't use this file.

set(XL_SRC_ROOT ${XL_REPO_ROOT})
include(${XL_REPO_ROOT}/cmake/XLightsCoreSources.cmake)

# uDMXOutput is referenced by nothing and no longer compiles (OUTPUT_UDMX is
# undefined, Ping() overrides a non-virtual); the root build only avoids it
# because it is dropped whenever libusb is absent. Keep it out of the test lib.
list(REMOVE_ITEM SRC_CORE
    ${XL_REPO_ROOT}/src-core/outputs/uDMXOutput.cpp
    ${XL_REPO_ROOT}/src-core/outputs/uDMXOutput.h)
set(LIBUSB_FOUND FALSE)

if(WIN32)
    include(${CMAKE_CURRENT_LIST_DIR}/XLightsCoreLibWindows.cmake)
    return()
endif()

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libavfilter libswscale libswresample)
find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(ZSTD REQUIRED libzstd)
pkg_check_modules(MINIZIP REQUIRED minizip)

find_path(LUA_INCLUDE_DIR lua.h PATH_SUFFIXES lua5.4 lua5.3 lua)
find_library(LUA_LIB NAMES lua lua5.4 lua5.3)
if(NOT LUA_INCLUDE_DIR OR NOT LUA_LIB)
    message(FATAL_ERROR "Lua development package not found (liblua5.4-dev)")
endif()
find_library(XLSXWRITER_LIB NAMES xlsxwriter)

# Effects: the same list the root build uses minus the Vulkan and ISPC kernels,
# which need glslc / ispc. Effects fall back to their CPU paths without them.
file(GLOB SRC_EFFECTS ${XL_REPO_ROOT}/src-core/effects/*.cpp)

# ISPC SIMD kernels, built exactly as the root build does (native CMake ISPC
# language support on Linux/macOS). Effects call them unconditionally.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
    set(_ispc_sets neon-i32x8)
    set(_ispc_arch aarch64)
else()
    set(_ispc_sets avx2-i32x8 avx1-i32x8 sse4-i32x8 sse2-i32x8)
    set(_ispc_arch x86_64)
endif()
include(CheckLanguage)
check_language(ISPC)
if(NOT CMAKE_ISPC_COMPILER)
    message(FATAL_ERROR "ispc not found: the effects need the SIMD kernels (apt install ispc)")
endif()
enable_language(ISPC)
# One --target list would build every ISA but CMake only links the first object;
# CMAKE_ISPC_INSTRUCTION_SETS makes it build and link one object per ISA plus the
# runtime dispatcher.
set(CMAKE_ISPC_FLAGS "--math-lib=fast --arch=${_ispc_arch} --pic")
set(CMAKE_ISPC_INSTRUCTION_SETS ${_ispc_sets})
file(GLOB _ispc_sources ${XL_REPO_ROOT}/src-core/effects/ispc/*.ispc)
list(APPEND SRC_EFFECTS ${_ispc_sources} ${XL_REPO_ROOT}/src-core/effects/ispc/ISPCComputeUtilities.cpp)

# Vulkan backend, as the Linux app builds it (Windows full-core goes through the
# root build). volk dlopens the loader, so a machine without Vulkan still links
# and the effects take their CPU paths; the [gpu] tests skip there.
set(XL_TEST_VULKAN FALSE)
if(UNIX AND NOT APPLE)
    find_program(GLSLC_EXECUTABLE glslc HINTS $ENV{VULKAN_SDK}/bin)
    if(NOT GLSLC_EXECUTABLE)
        message(FATAL_ERROR "glslc not found: the Vulkan backend needs it (apt install glslc)")
    endif()
    list(APPEND SRC_EFFECTS ${SRC_VULKAN})
    set(XL_TEST_VULKAN TRUE)
endif()

add_library(xlights_core STATIC ${SRC_CORE} ${SRC_EFFECTS})

if(XL_TEST_VULKAN)
    target_compile_definitions(xlights_core PUBLIC HAVE_VULKAN HAVE_VULKAN_SHADER VK_NO_PROTOTYPES)
    target_include_directories(xlights_core PUBLIC
        ${XL_REPO_ROOT}/dependencies/Vulkan-Headers/include
        ${XL_REPO_ROOT}/dependencies/volk
        ${XL_REPO_ROOT}/dependencies/VulkanMemoryAllocator/include)
    # glslang from the distro, in the root build's order (the packaged glslang is
    # built with the optimizer, so it needs SPIRV-Tools after it).
    foreach(_gl glslang-default-resource-limits SPIRV glslang MachineIndependent GenericCodeGen OSDependent SPIRV-Tools-opt SPIRV-Tools)
        find_library(GLSLANG_${_gl}_LIB NAMES ${_gl})
        if(GLSLANG_${_gl}_LIB)
            target_link_libraries(xlights_core PUBLIC ${GLSLANG_${_gl}_LIB})
        endif()
    endforeach()
    # The SPIR-V headers the vulkan/ sources #include are generated, not committed.
    xl_vulkan_shader_headers(_vulkan_shaders)
    add_custom_target(xlights_core_vulkan_shaders
        COMMAND ${CMAKE_COMMAND} -E env "GLSLC=${GLSLC_EXECUTABLE}"
                bash ${XL_REPO_ROOT}/build_scripts/compile_vulkan_shaders.sh
        BYPRODUCTS ${_vulkan_shaders_HEADERS}
        SOURCES ${_vulkan_shaders}
        COMMENT "Compiling Vulkan shaders to SPIR-V headers")
    add_dependencies(xlights_core xlights_core_vulkan_shaders)
endif()
target_include_directories(xlights_core PUBLIC
    ${XL_REPO_ROOT}/src-core
    ${XL_REPO_ROOT}/src-core/render
    ${XL_REPO_ROOT}/src-core/utils
    ${CMAKE_CURRENT_LIST_DIR}                        # support/TempDir.h
    ${XL_REPO_ROOT}/include                          # log.h
    ${XL_REPO_ROOT}/include/sol2-3.5.0
    ${XL_REPO_ROOT}/dependencies
    ${XL_REPO_ROOT}/dependencies/spdlog/include
    ${XL_REPO_ROOT}/dependencies/pugixml/src
    ${XL_REPO_ROOT}/dependencies/midifile/include
    ${XL_REPO_ROOT}/dependencies/liquidfun/liquidfun/Box2D
    ${FFMPEG_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${ZSTD_INCLUDE_DIRS}
    ${MINIZIP_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIR})
target_compile_definitions(xlights_core PUBLIC IGNORE_VENDORS wxUSE_WEBP XLIGHTS_CMAKE_BUILD NOMINMAX)
if(NOT MSVC)
    target_compile_options(xlights_core PRIVATE
        $<$<COMPILE_LANGUAGE:CXX,C>:-w>)   # the root build silences the same warnings; keep test logs readable
endif()
if(LIBUSB_FOUND)
    target_include_directories(xlights_core PUBLIC ${LIBUSB_INCLUDE_DIRS})
endif()

# Third-party sources the app compiles alongside core (not part of SRC_CORE).
target_sources(xlights_core PRIVATE
    ${XL_REPO_ROOT}/dependencies/md5/md5.cpp
    ${XL_REPO_ROOT}/dependencies/midifile/src/Binasc.cpp
    ${XL_REPO_ROOT}/dependencies/midifile/src/MidiEvent.cpp
    ${XL_REPO_ROOT}/dependencies/midifile/src/MidiEventList.cpp
    ${XL_REPO_ROOT}/dependencies/midifile/src/MidiFile.cpp
    ${XL_REPO_ROOT}/dependencies/midifile/src/MidiMessage.cpp
    ${XL_REPO_ROOT}/dependencies/pugixml/src/pugixml.cpp
    ${XL_REPO_ROOT}/dependencies/vamp-hostsdk/FFT.cpp
    ${XL_REPO_ROOT}/dependencies/vamp-hostsdk/Files.cpp
    ${XL_REPO_ROOT}/dependencies/vamp-hostsdk/PluginBufferingAdapter.cpp
    ${XL_REPO_ROOT}/dependencies/vamp-hostsdk/PluginChannelAdapter.cpp
    ${XL_REPO_ROOT}/dependencies/vamp-hostsdk/PluginHostAdapter.cpp
    ${XL_REPO_ROOT}/dependencies/vamp-hostsdk/PluginInputDomainAdapter.cpp
    ${XL_REPO_ROOT}/dependencies/vamp-hostsdk/PluginLoader.cpp
    ${XL_REPO_ROOT}/dependencies/vamp-hostsdk/PluginSummarisingAdapter.cpp
    ${XL_REPO_ROOT}/dependencies/vamp-hostsdk/PluginWrapper.cpp
    ${XL_REPO_ROOT}/dependencies/vamp-hostsdk/RealTime.cpp
    ${XL_REPO_ROOT}/dependencies/vamp-hostsdk/acsymbols.c
    ${XL_REPO_ROOT}/dependencies/vamp-hostsdk/host-c.cpp)

target_link_libraries(xlights_core PUBLIC
    ${FFMPEG_LINK_LIBRARIES}
    ${SDL2_LINK_LIBRARIES}
    ${ZSTD_LINK_LIBRARIES}
    ${MINIZIP_LINK_LIBRARIES}
    CURL::libcurl
    ZLIB::ZLIB
    ${LUA_LIB}
    Threads::Threads
    ${CMAKE_DL_LIBS})
# LiquidFun (physics effects). The repo ships prebuilt archives as
# lib/linux/libliquidfun.a.<arch>; the root Linux build copies one to libliquidfun.a.
find_library(LIQUIDFUN_LIB NAMES liquidfun HINTS ${XL_REPO_ROOT}/lib/linux)
if(NOT LIQUIDFUN_LIB AND EXISTS ${XL_REPO_ROOT}/lib/linux/libliquidfun.a.${CMAKE_SYSTEM_PROCESSOR})
    set(LIQUIDFUN_LIB ${XL_REPO_ROOT}/lib/linux/libliquidfun.a.${CMAKE_SYSTEM_PROCESSOR})
endif()
if(LIQUIDFUN_LIB)
    target_link_libraries(xlights_core PUBLIC ${LIQUIDFUN_LIB})
else()
    message(WARNING "LiquidFun not found: tests that pull in the physics effects will not link")
endif()

if(UNIX AND NOT APPLE)
    # The app gets X11 through wxGTK; core's GL context manager and display probe
    # need them directly (root links GL GLU EGL Xrandr).
    foreach(_lib GL GLU EGL Xrandr X11)
        find_library(XL_${_lib}_LIB NAMES ${_lib})
        if(XL_${_lib}_LIB)
            target_link_libraries(xlights_core PUBLIC ${XL_${_lib}_LIB})
        endif()
    endforeach()
endif()
# SequencePackage embeds fonts through fontconfig on Linux (wx brings it in for the app).
if(UNIX AND NOT APPLE)
    pkg_check_modules(FONTCONFIG QUIET fontconfig)
    if(FONTCONFIG_FOUND)
        target_include_directories(xlights_core PUBLIC ${FONTCONFIG_INCLUDE_DIRS})
        target_link_libraries(xlights_core PUBLIC ${FONTCONFIG_LIBRARIES})
    else()
        find_library(FONTCONFIG_LIB NAMES fontconfig)
        if(FONTCONFIG_LIB)
            target_link_libraries(xlights_core PUBLIC ${FONTCONFIG_LIB})
        endif()
    endif()
endif()

# DNS-SD (Discovery.cpp uses it when <dns_sd.h> is present; the root build links
# avahi-compat-libdns_sd for the same reason).
if(UNIX AND NOT APPLE)
    pkg_check_modules(DNSSD QUIET avahi-compat-libdns_sd)
    if(DNSSD_FOUND)
        target_include_directories(xlights_core PUBLIC ${DNSSD_INCLUDE_DIRS})
        target_link_libraries(xlights_core PUBLIC ${DNSSD_LIBRARIES})
    else()
        find_library(DNSSD_LIB NAMES dns_sd)
        if(DNSSD_LIB)
            target_link_libraries(xlights_core PUBLIC ${DNSSD_LIB})
        endif()
    endif()
endif()
if(XLSXWRITER_LIB)
    target_link_libraries(xlights_core PUBLIC ${XLSXWRITER_LIB})
endif()
if(LIBUSB_FOUND)
    target_link_libraries(xlights_core PUBLIC ${LIBUSB_LIBRARIES})
endif()
