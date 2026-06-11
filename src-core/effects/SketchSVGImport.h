#pragma once

#include <string>

// Converts an SVG file into the Sketch effect's `E_TEXTCTRL_SketchDef`
// string (normalised [0,1] paths). wx-free so both the desktop assist
// panel and the iPad path editor can share the same conversion. Mirrors
// the nanosvg-based logic that previously lived only in the wx
// SketchAssistPanel: cubic groups collapse to lines when collinear and
// to quadratics when the second control point coincides with the end.
//
// Returns an empty string when the file can't be parsed or has no paths.
std::string SketchDefFromSVGFile(const std::string& filename);
