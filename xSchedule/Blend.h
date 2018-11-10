#ifndef BLEND_H
#define BLEND_H

#include <string>
#include <wx/wx.h>

class wxChoice;

typedef enum {
    METHOD_OVERWRITE,
    METHOD_OVERWRITEIFZERO,
    METHOD_MASK,
    METHOD_UNMASK,
    METHOD_AVERAGE,
    METHOD_MAX,
    METHOD_OVERWRITEIFBLACK,
    METHOD_MASKPIXEL,
    METHOD_UNMASKPIXEL,
    METHOD_MIN
} APPLYMETHOD;

void PopulateBlendModes(wxChoice* choice);

void Blend(wxByte* buffer, size_t bufferSize, wxByte* blendBuffer, size_t blendBufferSize, APPLYMETHOD applyMethod, size_t offset = 0);

void Overwrite(wxByte* buffer, wxByte* blendBuffer, size_t channels);
void OverwriteIfZero(wxByte* buffer, wxByte* blendBuffer, size_t channels);
void Mask(wxByte* buffer, wxByte* blendBuffer, size_t channels);
void Unmask(wxByte* buffer, wxByte* blendBuffer, size_t channels);
void Average(wxByte* buffer, wxByte* blendBuffer, size_t channels);
void Maximum(wxByte* buffer, wxByte* blendBuffer, size_t channels);
void Minimum(wxByte* buffer, wxByte* blendBuffer, size_t channels);
void OverwriteIfBlack(wxByte* buffer, wxByte* blendBuffer, size_t pixels);
void MaskPixel(wxByte* buffer, wxByte* blendBuffer, size_t pixels);
void UnmaskPixel(wxByte* buffer, wxByte* blendBuffer, size_t pixels);
APPLYMETHOD EncodeBlendMode(const std::string blendMode);
std::string DecodeBlendMode(APPLYMETHOD blendMode);

#endif
