cmake_minimum_required(VERSION 3.20)

project(xlDo)

#add_definitions(xlDO)
add_compile_definitions(xlDO)

find_package(wxWidgets REQUIRED net gl core base)
include(${wxWidgets_USE_FILE})

include_directories(../include/curl ../include)
link_directories(../lib/windows64)

set(SRC_FILES
    xlDo.cpp
    ../xLights/automation/automation.cpp
    ../xLights/automation/automation.h
    )

file( GLOB WXJSON_FILES ../xSchedule/wxJSON/*cpp ../xSchedule/wxJSON/*h )

add_executable(${PROJECT_NAME} ${SRC_FILES} ${WXJSON_FILES})

target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})