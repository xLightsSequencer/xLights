vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO openvinotoolkit/openvino.genai
    REF "2026.1.0.0"
    SHA512 dbe7df58ff53727fc9bfc4e4e5a7854babeb1df44dd1c04275c8d428a620dade54b4d31a4e6425263bbe96ba4e420819fd9ca395ffdc4bbd2713ce7a368d6f30
    HEAD_REF master
)

# Pre-fetch all FetchContent dependencies.  vcpkg sets FETCHCONTENT_FULLY_DISCONNECTED=ON
# so any dep not on disk causes a configure error.
# Using vcpkg_from_github ensures OUT_SOURCE_PATH points directly to the cmake root.

vcpkg_from_github(
    OUT_SOURCE_PATH MINJA_SOURCE_PATH
    REPO google/minja
    REF 3e4c61c616eda133cfb1e440fc7a14bf1729bbee
    SHA512 eabe2d81a3affc5486c9f86632322400c9d6de3351428175ead7153155f0c447d2499fa801a2c2bfca70c7128ca799ceb2469bb6707820effa4c89c5aa8b7e2e
    HEAD_REF main
)

vcpkg_from_github(
    OUT_SOURCE_PATH SAFETENSORS_SOURCE_PATH
    REPO hsnyder/safetensors.h
    REF 974a85d7dfd6e010558353226638bb26d6b9d756
    SHA512 9349e68d31e8d817380868047655c3f84ce1439c87e3a9266cf1bb06d108851c9cc7209a99aa504920e72f3692f6f4f566e2879e8acaef8077db13e2e0c81bda
    HEAD_REF main
)

vcpkg_from_github(
    OUT_SOURCE_PATH NLOHMANN_SOURCE_PATH
    REPO nlohmann/json
    REF v3.11.3
    SHA512 7df19b621de34f08d5d5c0a25e8225975980841ef2e48536abcf22526ed7fb99f88ad954a2cb823115db59ccc88d1dbe74fe6c281b5644b976b33fb78db9d717
    HEAD_REF master
)

# gguf-tools: required at compile time even when ENABLE_GGUF=OFF because gguf.hpp
# is included unconditionally in 2026.1.0.0 source files.
vcpkg_download_distfile(GGUFLIB_ARCHIVE
    URLS "https://github.com/Lourdle/gguf-tools/archive/bac796ada809ac293e685db59b075971181cb008.zip"
    FILENAME "gguf-tools-bac796ad.zip"
    SHA512 247badcd579f206beacc225734e1e07615fc4433102ff16408ca6f5b482313245af93a2bde72994eb051149d81deef3363171672fb465ad479863729b229ea13
)
vcpkg_extract_source_archive(GGUFLIB_SOURCE_PATH
    ARCHIVE "${GGUFLIB_ARCHIVE}"
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DENABLE_PYTHON=OFF
        -DENABLE_JS=OFF
        -DENABLE_SAMPLES=OFF
        -DENABLE_TESTS=OFF
        -DENABLE_TOOLS=OFF
        -DENABLE_XGRAMMAR=OFF
        -DENABLE_GGUF=ON
        -DCPACK_GENERATOR=VCPKG
        # openvino_tokenizers is a git submodule not present in the tarball;
        # it ships as a separate shared library alongside the OpenVINO runtime.
        -DBUILD_TOKENIZERS=OFF
        # Pre-populated FetchContent source directories
        "-DFETCHCONTENT_SOURCE_DIR_MINJA=${MINJA_SOURCE_PATH}"
        "-DFETCHCONTENT_SOURCE_DIR_SAFETENSORS.H=${SAFETENSORS_SOURCE_PATH}"
        "-DFETCHCONTENT_SOURCE_DIR_NLOHMANN_JSON=${NLOHMANN_SOURCE_PATH}"
        "-DFETCHCONTENT_SOURCE_DIR_GGUFLIB=${GGUFLIB_SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(
    PACKAGE_NAME OpenVINOGenAI
    CONFIG_PATH "runtime/cmake"
    DO_NOT_DELETE_PARENT_CONFIG_PATH
)

# The debug cmake targets file references paths without the debug/ prefix that vcpkg
# uses. Fix IMPORTED_LOCATION/IMPORTED_IMPLIB in the debug targets file.
file(READ "${CURRENT_PACKAGES_DIR}/share/OpenVINOGenAI/OpenVINOGenAITargets-debug.cmake" _debug_targets)
string(REPLACE "\${_IMPORT_PREFIX}/runtime/" "\${_IMPORT_PREFIX}/debug/runtime/" _debug_targets "${_debug_targets}")
file(WRITE "${CURRENT_PACKAGES_DIR}/share/OpenVINOGenAI/OpenVINOGenAITargets-debug.cmake" "${_debug_targets}")

vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
