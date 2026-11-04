# Unit tests for src-core (Catch2).
#
# Standalone project -- it does not need wxWidgets, the dependency bundle, or the
# main xLights build, so it works the same on macOS, Linux and Windows:
#
#   cmake -S tests/unit -B build-tests -G Xcode      # macOS: open build-tests/xLights-unit-tests.xcodeproj
#   cmake -S tests/unit -B build-tests -G Ninja      # or Ninja / Make / VS
#   cmake --build build-tests && ctest --test-dir build-tests --output-on-failure
#
# It is also reachable from the root build with -DXL_BUILD_TESTS=ON.
#
# Layout: src-core sources go into the xlights_core_testlib static library via
# core_sources.cmake; each tests/unit/test_*.cpp is globbed into the one test
# executable. Growing coverage = add a source to core_sources.cmake (once the
# test needs it) and drop a test_*.cpp next to this file.
cmake_minimum_required(VERSION 3.20)
if(NOT CMAKE_PROJECT_NAME)
    project(xLights-unit-tests CXX C)
endif()

set(XL_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." CACHE INTERNAL "")
get_filename_component(XL_REPO_ROOT "${XL_REPO_ROOT}" ABSOLUTE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

include(CTest)

find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.1)
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/core_sources.cmake)

add_library(xlights_core_testlib STATIC ${XL_TEST_CORE_SOURCES})
target_include_directories(xlights_core_testlib PUBLIC ${XL_REPO_ROOT}/src-core ${XL_REPO_ROOT}/src-core/utils)
if(MSVC)
    target_compile_options(xlights_core_testlib PUBLIC /utf-8)
endif()

file(GLOB XL_TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/test_*.cpp)
add_executable(xlights_unit_tests ${XL_TEST_SOURCES})
target_link_libraries(xlights_unit_tests PRIVATE xlights_core_testlib Catch2::Catch2WithMain)

# Release builds use -ffast-math; the NaN/inf guards only mean something when
# the test is compiled the same way.
if(NOT MSVC)
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/test_float_checks.cpp
        PROPERTIES COMPILE_OPTIONS "-ffast-math;-O2;-Wno-unknown-warning-option;-Wno-nan-infinity-disabled")
endif()

include(Catch)
catch_discover_tests(xlights_unit_tests)
