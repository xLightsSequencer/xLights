# Windows half of XLightsCoreLib.cmake: the static library `xlights_core` built
# from the dependencies the root build has already located in the dependency
# bundle (ci_scripts/fetch_dependencies.ps1). Configure from the repository root:
#
#   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release ^
#         -DXL_BUILD_TESTS=ON -DXL_TEST_FULL_CORE=ON
#   cmake --build build --target xlights_unit_tests
#   ctest --test-dir build/tests/unit --output-on-failure
#
# SRC_CORE (from XLightsCoreSources.cmake) is set by the includer; everything
# else here comes from the root CMakeLists.txt's scope.

if(NOT DEFINED XL_BUNDLE)
    message(FATAL_ERROR "On Windows the full-core unit tests are built from the root project: "
                        "cmake -S . -B build -DXL_BUILD_TESTS=ON -DXL_TEST_FULL_CORE=ON")
endif()

file(GLOB SRC_EFFECTS ${XL_REPO_ROOT}/src-core/effects/*.cpp)
list(APPEND SRC_EFFECTS ${XL_REPO_ROOT}/src-core/effects/ispc/ISPCComputeUtilities.cpp)
set(XL_TEST_VULKAN ${XLIGHTS_VULKAN_ENABLED})
if(XL_TEST_VULKAN)
    list(APPEND SRC_EFFECTS ${SRC_VULKAN})
endif()

add_library(xlights_core STATIC ${SRC_CORE} ${SRC_EFFECTS})

# The SIMD kernels: the root build compiles them (VS generators can't enable the
# ISPC language), and xl_ispc_objects is the target that owns those rules.
if(ISPC_OBJECTS)
    target_sources(xlights_core PRIVATE ${ISPC_OBJECTS})
    add_dependencies(xlights_core xl_ispc_objects)
endif()

if(XL_TEST_VULKAN)
    target_compile_definitions(xlights_core PUBLIC HAVE_VULKAN HAVE_VULKAN_SHADER VK_NO_PROTOTYPES)
    target_include_directories(xlights_core PUBLIC
        ${XL_REPO_ROOT}/dependencies/Vulkan-Headers/include
        ${XL_REPO_ROOT}/dependencies/volk
        ${XL_REPO_ROOT}/dependencies/VulkanMemoryAllocator/include
        "$ENV{VULKAN_SDK}/include")
    target_link_directories(xlights_core PUBLIC "$ENV{VULKAN_SDK}/lib")
    target_link_libraries(xlights_core PUBLIC
        glslang glslang-default-resource-limits SPIRV-Tools-opt SPIRV-Tools)
    add_dependencies(xlights_core vulkan_shaders)
endif()

target_include_directories(xlights_core PUBLIC
    ${XL_REPO_ROOT}/src-core
    ${XL_REPO_ROOT}/src-core/render
    ${XL_REPO_ROOT}/src-core/utils
    ${CMAKE_CURRENT_LIST_DIR}                        # support/TempDir.h
    ${XL_REPO_ROOT}/dependencies
    ${XL_REPO_ROOT}/dependencies/spdlog/include
    ${XL_REPO_ROOT}/dependencies/pugixml/src
    ${XL_REPO_ROOT}/dependencies/midifile/include
    ${XL_REPO_ROOT}/include
    ${XL_BUNDLE}/include                             # also Box2D (liquidfun)
    ${XL_REPO_ROOT}/include/sol2-3.5.0
    ${FFMPEG_INCLUDE_DIR}
    ${LUA_INCLUDE_DIR}
    ${SDL2_INC}
    ${ZSTD_INC}
    ${CURL_INC})

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

if(FFMPEG_LIBRARY_DIRS)
    target_link_directories(xlights_core PUBLIC ${FFMPEG_LIBRARY_DIRS})
endif()
target_link_directories(xlights_core PUBLIC ${XL_WIN_LIB_HINTS})
target_link_libraries(xlights_core PUBLIC
    ${FFMPEG_LIBS}
    ${SDL2_LINK_TARGET}
    ${ZSTD_LINK_TARGET}
    ${LUA_LIB}
    ${MINIZIP_LIB}
    OpenGL::GL glu32
    ws2_32 winmm comctl32 rpcrt4 mfuuid propsys wmcodecdspuuid iphlpapi)
if(ZLIB_LINK_TARGET)
    target_link_libraries(xlights_core PUBLIC ${ZLIB_LINK_TARGET})
endif()
if(CURL_LINK_TARGET)
    target_link_libraries(xlights_core PUBLIC ${CURL_LINK_TARGET})
endif()
if(XLSXWRITER_LIB)
    target_link_libraries(xlights_core PUBLIC ${XLSXWRITER_LIB})
endif()
if(LIQUIDFUN_LIB_RELEASE)
    target_link_libraries(xlights_core PUBLIC
        optimized ${LIQUIDFUN_LIB_RELEASE}
        debug     ${LIQUIDFUN_LIB_DEBUG})
endif()
if(WEBP_LIB AND WEBPDEMUX_LIB)
    target_link_libraries(xlights_core PUBLIC ${WEBP_LIB} ${WEBPDEMUX_LIB})
endif()
if(MSVC AND NOT XLIGHTS_USE_VCPKG)
    target_link_options(xlights_core INTERFACE
        $<$<CONFIG:Release>:/NODEFAULTLIB:LIBCMT>
        $<$<CONFIG:Debug>:/NODEFAULTLIB:LIBCMTD>)
endif()
