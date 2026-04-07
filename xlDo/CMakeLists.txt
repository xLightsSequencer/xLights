cmake_minimum_required(VERSION 3.20)

project(xlDo)

#add_definitions(xlDO)
add_compile_definitions(xlDO)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(wxWidgets REQUIRED net gl core base)
include(${wxWidgets_USE_FILE})

if(MSVC)
    add_compile_options(/utf-8)
endif()

include_directories(../include/curl ../include ../dependencies/spdlog/include ../src-core ../src-ui-wx)
link_directories(../lib/windows64)

set(SRC_FILES
    xlDo.cpp
    ../src-ui-wx/automation/automation.cpp
    ../src-ui-wx/automation/automation.h
	../src-core/utils/CurlManager.cpp
    ../src-core/utils/CurlManager.h
    )

add_executable(${PROJECT_NAME} ${SRC_FILES} )

target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})