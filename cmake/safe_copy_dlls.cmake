# safe_copy_dlls.cmake — called by the xLights POST_BUILD step.
# Copies files matching GLOB_PAT from SRC_DIR to DST_DIR only when the two
# directories are different (avoids "cannot copy file onto itself" on CI when
# the build output directory == the source DLL directory).
#
# Arguments (passed via -DVAR=value):
#   SRC_DIR   — source directory
#   DST_DIR   — destination directory
#   GLOB_PAT  — glob pattern relative to SRC_DIR (e.g. "*.dll")
cmake_minimum_required(VERSION 3.20)

get_filename_component(SRC_DIR "${SRC_DIR}" REALPATH)
get_filename_component(DST_DIR "${DST_DIR}" REALPATH)

if(SRC_DIR STREQUAL DST_DIR)
    return()   # same directory — nothing to do
endif()

file(GLOB files "${SRC_DIR}/${GLOB_PAT}")
if(files)
    file(COPY ${files} DESTINATION "${DST_DIR}")
endif()
