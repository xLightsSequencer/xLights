#pragma once

// Platform-independent cursor type enum for model handle interaction.
// Convert to platform cursor (e.g., wxCursor) at the UI boundary only.
enum class CursorType {
    Default,      // wxCURSOR_DEFAULT - normal arrow
    Hand,         // wxCURSOR_HAND - draggable handle
    Sizing,       // wxCURSOR_SIZING - general resize
    SizeWE,       // wxCURSOR_SIZEWE - horizontal resize
    SizeNS,       // wxCURSOR_SIZENS - vertical resize
    SizeNWSE,     // wxCURSOR_SIZENWSE - diagonal NW-SE resize
    SizeNESW,     // wxCURSOR_SIZENESW - diagonal NE-SW resize
    Bullseye,     // wxCURSOR_BULLSEYE - add-point handle
    Wait          // wxCURSOR_WAIT - busy
};
