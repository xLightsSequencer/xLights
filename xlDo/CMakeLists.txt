cmake_minimum_required(VERSION 3.21)

set(APP_NAME xlDo)

project(${APP_NAME})

add_compile_options("SHELL:-target x86_64-apple-macos10.12")
add_compile_options("SHELL:-D__WXOSX__ -D__WXMAC__ -D__WXOSX_COCOA__ -DwxDEBUG_LEVEL=0 ")
add_compile_options("SHELL:-Wno-c++11-extensions")
add_compile_options("SHELL:-std=gnu++1z -stdlib=libc++")

find_package(wxWidgets REQUIRED COMPONENTS net xml core base)

file(GLOB SRC_XLDO "${PROJECT_SOURCE_DIR}/xlDo.cpp")
file(GLOB SRC_XLIGHTS "${PROJECT_SOURCE_DIR}/../xLights/UtilFunctions.cpp")
file(GLOB SRC_XLIGHTS_AUTOMATION "${PROJECT_SOURCE_DIR}/../xLights/automation/automation.cpp")
file(GLOB SRC_XSCHEDULE_JSON "${PROJECT_SOURCE_DIR}/../xSchedule/wxJSON/*.cpp")
add_executable(${APP_NAME} ${SRC_XLDO} ${SRC_XLIGHTS} ${SRC_XLIGHTS_AUTOMATION} ${SRC_XSCHEDULE_JSON})

target_link_libraries(${APP_NAME} ${wxWidgets_LIBRARIES} /opt/local/lib/liblog4cpp.a)

target_include_directories(${APP_NAME} PUBLIC /opt/local/include/wx-3.1)
target_include_directories(${APP_NAME} PUBLIC /opt/local/lib/wx/include/osx_cocoa-unicode-static-3.1)
target_include_directories(${APP_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/../include)
