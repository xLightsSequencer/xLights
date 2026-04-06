#pragma once

// Prefix wrapper for nanosvg to avoid symbol conflicts with wxWidgets'
// bundled nanosvg on Windows. All public nanosvg symbols are renamed
// with an xl_ prefix so the linker picks up our version.

// --- Public type renames (avoid struct layout mismatches with wx's nanosvg) ---
#define NSVGgradientStop    xl_NSVGgradientStop
#define NSVGgradient        xl_NSVGgradient
#define NSVGpaint           xl_NSVGpaint
#define NSVGpath            xl_NSVGpath
#define NSVGshape           xl_NSVGshape
#define NSVGimage           xl_NSVGimage
#define NSVGrasterizer      xl_NSVGrasterizer

// --- Internal types (in implementation sections) ---
#define NSVGcoordinate      xl_NSVGcoordinate
#define NSVGlinearData      xl_NSVGlinearData
#define NSVGradialData      xl_NSVGradialData
#define NSVGgradientData    xl_NSVGgradientData
#define NSVGattrib          xl_NSVGattrib
#define NSVGparser          xl_NSVGparser
#define NSVGNamedColor      xl_NSVGNamedColor
#define NSVGedge            xl_NSVGedge
#define NSVGpoint           xl_NSVGpoint
#define NSVGactiveEdge      xl_NSVGactiveEdge
#define NSVGmemPage         xl_NSVGmemPage
#define NSVGcachedPaint     xl_NSVGcachedPaint

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

// Include the actual nanosvg headers (they will use the prefixed names).
// Use a direct relative path to our dependencies copy so that the compiler
// never accidentally picks up wxWidgets' modified 3rdparty/nanosvg (which
// has a different NSVGshape layout with a paintOrder field).
#include "../../dependencies/nanosvg/src/nanosvg.h"
#include "../../dependencies/nanosvg/src/nanosvgrast.h"
