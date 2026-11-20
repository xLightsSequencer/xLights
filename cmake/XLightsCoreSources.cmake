# The wx-free core source list, shared by the root build and tests/unit so the
# two cannot drift. The includer sets XL_SRC_ROOT (the repository root) first.
#
# Sets in the includer's scope:
#   SRC_CORE      - every src-core .cpp (minus the ones other platforms/libs exclude)
#   SRC_VULKAN    - the Vulkan compute/graphics backend; the includer adds it (Linux/Windows only)
#   LIBUSB_*      - libusb detection result; uDMXOutput is dropped when absent
#   XL_MBEDTLS_INC  - Mbed TLS headers; every target that compiles SRC_CORE
#                    needs them on its include path
#   XL_MBEDTLS_DEFS - compile definitions those targets need
#   XL_MBEDTLS_LIBS - libraries those targets link (empty for the submodule)

# The LAN collaboration transport (src-core/collab/CollabNetPortable.cpp) uses
# Mbed TLS. By default it is compiled from the dependencies/mbedtls submodule
# (src-core/collab/CollabTls*.c); XL_SYSTEM_MBEDTLS uses an installed one
# instead (2.28 from the distro, or 3.x), with its default configuration.
option(XL_SYSTEM_MBEDTLS "Link an installed Mbed TLS instead of building dependencies/mbedtls" OFF)
if(XL_SYSTEM_MBEDTLS)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(XL_PC_MBEDTLS QUIET mbedtls mbedx509 mbedcrypto)
    endif()
    if(XL_PC_MBEDTLS_FOUND)
        set(XL_MBEDTLS_INC ${XL_PC_MBEDTLS_INCLUDE_DIRS})
        set(XL_MBEDTLS_LIBS ${XL_PC_MBEDTLS_LINK_LIBRARIES})
    else()
        # Not every Mbed TLS package ships pkg-config files.
        find_path(XL_MBEDTLS_INCLUDE_DIR mbedtls/ssl.h)
        find_library(XL_MBEDTLS_LIB mbedtls)
        find_library(XL_MBEDX509_LIB mbedx509)
        find_library(XL_MBEDCRYPTO_LIB mbedcrypto)
        if(NOT XL_MBEDTLS_INCLUDE_DIR OR NOT XL_MBEDTLS_LIB OR NOT XL_MBEDX509_LIB OR NOT XL_MBEDCRYPTO_LIB)
            message(FATAL_ERROR "XL_SYSTEM_MBEDTLS: Mbed TLS not found (Debian/Ubuntu: apt install libmbedtls-dev)")
        endif()
        set(XL_MBEDTLS_INC ${XL_MBEDTLS_INCLUDE_DIR})
        set(XL_MBEDTLS_LIBS ${XL_MBEDTLS_LIB} ${XL_MBEDX509_LIB} ${XL_MBEDCRYPTO_LIB})
    endif()
    set(XL_MBEDTLS_DEFS XL_MBEDTLS_EXTERNAL)
    message(STATUS "Mbed TLS: installed (${XL_MBEDTLS_LIBS})")
else()
    set(XL_MBEDTLS_INC ${XL_SRC_ROOT}/dependencies/mbedtls/include)
    set(XL_MBEDTLS_DEFS "")
    set(XL_MBEDTLS_LIBS "")
endif()

# Core (wx-free)
file(GLOB_RECURSE SRC_CORE
    ${XL_SRC_ROOT}/src-core/XmlSerializer/*.cpp
    ${XL_SRC_ROOT}/src-core/ai/*.cpp
    ${XL_SRC_ROOT}/src-core/cad/*.cpp
    ${XL_SRC_ROOT}/src-core/collab/*.cpp
    ${XL_SRC_ROOT}/src-core/collab/CollabTls*.c
    ${XL_SRC_ROOT}/src-core/controllers/*.cpp
	${XL_SRC_ROOT}/src-core/diagnostics/*.cpp
    ${XL_SRC_ROOT}/src-core/discovery/*.cpp
    ${XL_SRC_ROOT}/src-core/import_export/*.cpp
    ${XL_SRC_ROOT}/src-core/graphics/GLBackend.cpp
    ${XL_SRC_ROOT}/src-core/graphics/GLContextManager.cpp
    ${XL_SRC_ROOT}/src-core/graphics/xlFontInfo.cpp
    ${XL_SRC_ROOT}/src-core/graphics/xlGraphicsAccumulators.cpp
    ${XL_SRC_ROOT}/src-core/graphics/xlGraphicsCapability.cpp
    ${XL_SRC_ROOT}/src-core/graphics/xlMesh.cpp
    ${XL_SRC_ROOT}/src-core/lyrics/*.cpp
    ${XL_SRC_ROOT}/src-core/media/AudioLoader.cpp
    ${XL_SRC_ROOT}/src-core/media/AIModelStore.cpp
    ${XL_SRC_ROOT}/src-core/media/AudioManager.cpp
    ${XL_SRC_ROOT}/src-core/media/ChordDetector.cpp
    ${XL_SRC_ROOT}/src-core/media/FFmpegAudioDecoder.cpp
    ${XL_SRC_ROOT}/src-core/media/FFmpegVideoReader.cpp
    ${XL_SRC_ROOT}/src-core/media/MediaCompatibility.cpp
    ${XL_SRC_ROOT}/src-core/media/NoteImporter.cpp
    ${XL_SRC_ROOT}/src-core/media/OnsetDetector.cpp
    ${XL_SRC_ROOT}/src-core/media/PitchDetector.cpp
    ${XL_SRC_ROOT}/src-core/media/SDLAudioOutput.cpp
    ${XL_SRC_ROOT}/src-core/media/Spectrogram.cpp
    ${XL_SRC_ROOT}/src-core/media/StemSeparator.cpp
    ${XL_SRC_ROOT}/src-core/media/TempoDetector.cpp
    ${XL_SRC_ROOT}/src-core/media/FFmpegVideoWriter.cpp
    ${XL_SRC_ROOT}/src-core/media/VideoDecodeSizeRegistry.cpp
    ${XL_SRC_ROOT}/src-core/media/VideoReader.cpp
    ${XL_SRC_ROOT}/src-core/media/VideoTranscoder.cpp
    ${XL_SRC_ROOT}/src-core/media/VideoWriter.cpp
    ${XL_SRC_ROOT}/src-core/media/kiss_fft/kiss_fft.c
    ${XL_SRC_ROOT}/src-core/media/kiss_fft/tools/kiss_fftr.c
    ${XL_SRC_ROOT}/src-core/models/*.cpp
    ${XL_SRC_ROOT}/src-core/models/DMX/*.cpp
    ${XL_SRC_ROOT}/src-core/models/DMX/MovingHeads/*.cpp
    ${XL_SRC_ROOT}/src-core/outputs/*.cpp
    ${XL_SRC_ROOT}/src-core/render/*.cpp
    ${XL_SRC_ROOT}/src-core/settings/*.cpp
    ${XL_SRC_ROOT}/src-core/utils/*.cpp
)
set(SRC_VULKAN
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanComputeUtilities.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanGraphicsUtilities.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanShaderTranslate.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanEffectManager.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanLibs.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanBarsEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanColorWashEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanShockwaveEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanFanEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanSpiralsEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanGalaxyEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanCirclesEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanPlasmaEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanButterflyEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanPinwheelEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanKaleidoscopeEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanWarpEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanTreeEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanShimmerEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanCandleEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanWaveEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanGarlandsEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanFillEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanMeteorsEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanTwinkleEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanFireEffect.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanLifeEffect.cpp
    ${XL_SRC_ROOT}/src-core/graphics/vulkan/VulkanPipelineCache.cpp
    ${XL_SRC_ROOT}/src-core/graphics/vulkan/xlVulkanGraphicsContext.cpp
    ${XL_SRC_ROOT}/src-core/effects/vulkan/VulkanShaderEffect.cpp
)

if(WIN32)
    list(APPEND SRC_CORE
        ${XL_SRC_ROOT}/src-core/media/WindowsHardwareVideoReader.cpp
        ${XL_SRC_ROOT}/src-core/media/LiveCameraCapture.cpp)
    list(REMOVE_ITEM SRC_CORE
        ${XL_SRC_ROOT}/src-core/outputs/serial_posix.cpp
        ${XL_SRC_ROOT}/src-core/outputs/serial_osx.cpp
        ${XL_SRC_ROOT}/src-core/outputs/serial_win32.cpp)
elseif(APPLE)
    list(APPEND SRC_CORE ${XL_SRC_ROOT}/src-core/media/AVFoundationVideoWriter.cpp)
    list(REMOVE_ITEM SRC_CORE
        ${XL_SRC_ROOT}/src-core/media/WindowsHardwareVideoReader.cpp
        ${XL_SRC_ROOT}/src-core/outputs/serial_win32.cpp
        ${XL_SRC_ROOT}/src-core/outputs/serial_posix.cpp)
else()
    list(APPEND SRC_CORE ${XL_SRC_ROOT}/src-core/media/LiveCameraCapture.cpp)
    list(REMOVE_ITEM SRC_CORE
        ${XL_SRC_ROOT}/src-core/media/WindowsHardwareVideoReader.cpp
        ${XL_SRC_ROOT}/src-core/outputs/serial_win32.cpp
        ${XL_SRC_ROOT}/src-core/outputs/serial_osx.cpp)
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
        ${XL_SRC_ROOT}/src-core/outputs/uDMXOutput.h
        ${XL_SRC_ROOT}/src-core/outputs/uDMXOutput.cpp)
else()
    message(STATUS "libusb found — uDMX output enabled")
endif()

# The SPIR-V headers build_scripts/compile_vulkan_shaders.sh generates, for the
# shader target's BYPRODUCTS. Without them Ninja does not know the target writes
# the headers, so a changed shader only reaches the C++ that embeds it on the
# build after next.
function(xl_vulkan_shader_headers out_var)
    file(GLOB _shaders CONFIGURE_DEPENDS
        ${XL_SRC_ROOT}/src-core/effects/vulkan/shaders/*.comp
        ${XL_SRC_ROOT}/src-core/effects/vulkan/shaders/*.vert
        ${XL_SRC_ROOT}/src-core/effects/vulkan/shaders/*.frag
        ${XL_SRC_ROOT}/src-core/graphics/vulkan/shaders/*.vert
        ${XL_SRC_ROOT}/src-core/graphics/vulkan/shaders/*.frag)
    set(_headers "")
    foreach(_s ${_shaders})
        get_filename_component(_dir ${_s} DIRECTORY)
        get_filename_component(_base ${_s} NAME_WLE)
        get_filename_component(_ext ${_s} LAST_EXT)
        if(_ext STREQUAL ".comp")
            list(APPEND _headers ${_dir}/compiled/${_base}.spv.h)
        else()
            list(APPEND _headers ${_dir}/compiled/${_base}${_ext}.spv.h)
        endif()
    endforeach()
    set(${out_var} ${_shaders} ${_headers} PARENT_SCOPE)
    set(${out_var}_HEADERS ${_headers} PARENT_SCOPE)
endfunction()
