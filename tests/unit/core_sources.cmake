# src-core sources linked into the unit tests. Add a file here when a test
# needs it; if it drags in wx or heavy dependencies, prefer testing a leaf.
set(XL_TEST_CORE_SOURCES
    ${XL_REPO_ROOT}/src-core/utils/Color.cpp
    ${XL_REPO_ROOT}/src-core/utils/string_utils.cpp
    ${XL_REPO_ROOT}/src-core/utils/AppCallbacks.cpp
    ${XL_REPO_ROOT}/src-core/utils/NodeUtils.cpp
    ${XL_REPO_ROOT}/src-core/utils/XsqFileScanner.cpp
    ${XL_REPO_ROOT}/src-core/utils/ShowRedactor.cpp
    ${XL_REPO_ROOT}/dependencies/pugixml/src/pugixml.cpp
    ${XL_REPO_ROOT}/src-core/lyrics/LRCParser.cpp
    ${XL_REPO_ROOT}/src-core/lyrics/PhonemeDictionary.cpp
)
