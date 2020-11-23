#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <wx/wx.h>

class wxChoice;

typedef enum {
    METHOD_OVERWRITE,
    METHOD_OVERWRITEIFZERO,
    METHOD_OVERWRITESKIPBLACK,
    METHOD_MASK,
    METHOD_UNMASK,
    METHOD_AVERAGE,
    METHOD_MAX,
    METHOD_OVERWRITEIFBLACK,
    METHOD_MASKPIXEL,
    METHOD_UNMASKPIXEL,
    METHOD_MIN,
    METHOD_BRIGHTNESS
} APPLYMETHOD;

void PopulateBlendModes(wxChoice* choice);

void Blend(uint8_t* buffer, size_t bufferSize, uint8_t* blendBuffer, size_t blendBufferSize, APPLYMETHOD applyMethod, size_t offset = 0);

void Overwrite(uint8_t* buffer, uint8_t* blendBuffer, size_t channels);
void OverwriteIfZero(uint8_t* buffer, uint8_t* blendBuffer, size_t channels);
void Mask(uint8_t* buffer, uint8_t* blendBuffer, size_t channels);
void Unmask(uint8_t* buffer, uint8_t* blendBuffer, size_t channels);
void Average(uint8_t* buffer, uint8_t* blendBuffer, size_t channels);
void Maximum(uint8_t* buffer, uint8_t* blendBuffer, size_t channels);
void Minimum(uint8_t* buffer, uint8_t* blendBuffer, size_t channels);
void Brightness(uint8_t* buffer, uint8_t* blendBuffer, size_t channels);
void OverwriteIfBlack(uint8_t* buffer, uint8_t* blendBuffer, size_t pixels);
void MaskPixel(uint8_t* buffer, uint8_t* blendBuffer, size_t pixels);
void UnmaskPixel(uint8_t* buffer, uint8_t* blendBuffer, size_t pixels);
void OverwriteSkipBlack(uint8_t* buffer, uint8_t* blendBuffer, size_t pixels);
APPLYMETHOD EncodeBlendMode(const std::string blendMode);
std::string DecodeBlendMode(APPLYMETHOD blendMode);

