cmake_minimum_required(VERSION 3.20)

project(xScanner)

add_compile_definitions(EXCLUDENETWORKUI)
add_compile_definitions(DISCOVERYONLY)
add_compile_definitions(xScanner)

find_package(wxWidgets REQUIRED net gl core base scintilla regex zlib png tiff jpeg)
include(${wxWidgets_USE_FILE})

set(CMAKE_CXX_STANDARD 20)

if(WIN32)
  set(CMAKE_WIN32_EXECUTABLE ON)
elseif(APPLE)
  set(CMAKE_MACOSX_BUNDLE ON)
endif()

include_directories(../include/curl ../include ../xSchedule/xSMSDaemon ../xLights ../xLights/ffmpeg-dev/include)
link_directories(../lib/windows64)

set(XLIGHTS_FILES
    ../xLights/automation/automation.cpp
    ../xLights/automation/automation.h
    ../xLights/controllers/BaseController.cpp
    ../xLights/controllers/BaseController.h
    ../xLights/controllers/ControllerCaps.cpp
    ../xLights/controllers/ControllerCaps.h
    ../xLights/controllers/Falcon.cpp
    ../xLights/controllers/Falcon.h
    ../xLights/controllers/FPP.cpp
    ../xLights/controllers/FPP.h
    ../xLights/controllers/Pixlite16.cpp
    ../xLights/controllers/Pixlite16.h
    ../xLights/Discovery.cpp
    ../xLights/Discovery.h
    ../xLights/ExportSettings.cpp
    ../xLights/ExportSettings.h
    ../xLights/ExternalHooks.h
    ../xLights/JobPool.cpp
    ../xLights/JobPool.h
    ../xLights/Parallel.cpp
    ../xLights/Parallel.h
    ../xLights/TraceLog.cpp
    ../xLights/TraceLog.h
    ../xLights/UtilFunctions.cpp
    ../xLights/UtilFunctions.h
    ../xLights/xLightsVersion.cpp
    ../xLights/xLightsVersion.h
    )

file( GLOB WXJSON_FILES ../xSchedule/wxJSON/*cpp ../xSchedule/wxJSON/*h )
file( GLOB BASE_FILES ../common/*cpp ../common/*h )
file( GLOB XSCANNER_FILES *cpp *h )
file( GLOB OUTPUT_FILES ../xLights/outputs/*cpp ../xLights/outputs/*h )

list(FILTER OUTPUT_FILES EXCLUDE REGEX ".*serial_.*cpp$")

add_executable(${PROJECT_NAME} ${XLIGHTS_FILES} ${WXJSON_FILES} ${BASE_FILES} ${XSCANNER_FILES} ${OUTPUT_FILES})

target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})

target_sources(${PROJECT_NAME} PRIVATE resource.rc)

add_custom_command ( TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_LIST_DIR}/MacLookup.txt $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

source_group(${PROJECT_NAME} FILES ${XSCANNER_FILES})
source_group(xLights FILES ${XLIGHTS_FILES})
source_group(outputs FILES ${OUTPUT_FILES})
source_group(wxJSON FILES ${WXJSON_FILES})
source_group(common FILES ${BASE_FILES})

if(WIN32)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()