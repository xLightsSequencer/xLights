#pragma once

// Prefix wrapper for nanosvg to avoid symbol conflicts with wxWidgets'
// bundled nanosvg on Windows. All public nanosvg symbols are renamed
// with an xl_ prefix so the linker picks up our version.

// --- Public API function renames ---
#define nsvgParseFromFile       xl_nsvgParseFromFile
#define nsvgParse               xl_nsvgParse
#define nsvgDuplicatePath       xl_nsvgDuplicatePath
#define nsvgDelete              xl_nsvgDelete
#define nsvgCreateRasterizer    xl_nsvgCreateRasterizer
#define nsvgRasterize           xl_nsvgRasterize
#define nsvgDeleteRasterizer    xl_nsvgDeleteRasterizer

// --- Internal symbols that leak into the object file ---
#define nsvg__parseXML          xl_nsvg__parseXML
#define nsvg__colors            xl_nsvg__colors

// Enable all SVG color keywords
#ifndef NANOSVG_ALL_COLOR_KEYWORDS
#define NANOSVG_ALL_COLOR_KEYWORDS
#endif

// Include the actual nanosvg headers (they will use the prefixed names)
#include "nanosvg/src/nanosvg.h"
#include "nanosvg/src/nanosvgrast.h"
