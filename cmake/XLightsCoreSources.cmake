# The wx-free core source list, shared by the root build and tests/unit so the
# two cannot drift. The includer sets XL_SRC_ROOT (the repository root) first.
#
# Sets in the includer's scope:
#   SRC_CORE      - every src-core .cpp (minus the ones other platforms/libs exclude)
#   LIBUSB_*      - libusb detection result; uDMXOutput is dropped when absent

# Core (wx-free)
file(GLOB_RECURSE SRC_CORE
    ${XL_SRC_ROOT}/src-core/XmlSerializer/*.cpp
    ${XL_SRC_ROOT}/src-core/ai/*.cpp
    ${XL_SRC_ROOT}/src-core/cad/*.cpp
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
