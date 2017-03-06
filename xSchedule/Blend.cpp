#include "Blend.h"

void PopulateBlendModes(wxChoice* choice)
{
    choice->AppendString("Overwrite");
    choice->AppendString("Overwrite if zero");
    choice->AppendString("Overwrite if black");
    choice->AppendString("Mask out if not zero");
    choice->AppendString("Mask out if not black");
    choice->AppendString("Mask out if black");
    choice->AppendString("Mask out if zero");
    choice->AppendString("Average");
    choice->AppendString("Maximum");
    choice->AppendString("Minimum");
}

APPLYMETHOD EncodeBlendMode(const std::string blendMode)
{
    std::string bm = wxString(blendMode).Lower();

    if (bm == "Overwrite")
    {
        return APPLYMETHOD::METHOD_OVERWRITE;
    }
    else if (bm == "Overwrite if zero")
    {
        return APPLYMETHOD::METHOD_OVERWRITEIFZERO;
    }
    else if (bm == "Overwrite if black")
    {
        return APPLYMETHOD::METHOD_OVERWRITEIFBLACK;
    }
    else if (bm == "Mask out if not zero")
    {
        return APPLYMETHOD::METHOD_MASK;
    }
    else if (bm == "Mask out if not black")
    {
        return APPLYMETHOD::METHOD_MASKPIXEL;
    }
    else if (bm == "Mask out if black")
    {
        return APPLYMETHOD::METHOD_UNMASKPIXEL;
    }
    else if (bm == "Mask out if zero")
    {
        return APPLYMETHOD::METHOD_UNMASK;
    }
    else if (bm == "Average")
    {
        return APPLYMETHOD::METHOD_AVERAGE;
    }
    else if (bm == "Maximum")
    {
        return APPLYMETHOD::METHOD_MAX;
    }
    else if (bm == "Minimum")
    {
        return APPLYMETHOD::METHOD_MIN;
    }

    return APPLYMETHOD::METHOD_OVERWRITE;
}

std::string DecodeBlendMode(APPLYMETHOD blendMode)
{
    switch(blendMode)
    { 
    case APPLYMETHOD::METHOD_OVERWRITE:
        return "Overwrite";
    case APPLYMETHOD::METHOD_OVERWRITEIFZERO:
        return "Overwrite if zero";
    case APPLYMETHOD::METHOD_OVERWRITEIFBLACK:
        return "Overwrite if black";
    case APPLYMETHOD::METHOD_MASK:
        return "Mask out if not zero";
    case APPLYMETHOD::METHOD_MASKPIXEL:
        return "Mask out if not black";
    case APPLYMETHOD::METHOD_UNMASKPIXEL:
        return "Mask out if black";
    case APPLYMETHOD::METHOD_UNMASK:
        return "Mask out if zero";
    case APPLYMETHOD::METHOD_AVERAGE:
        return "Average";
    case APPLYMETHOD::METHOD_MAX:
        return "Maximum";
    case APPLYMETHOD::METHOD_MIN:
        return "Minimum";
    }

    return "Overwrite";
}

void Blend(wxByte* buffer, size_t bufferSize, wxByte* blendBuffer, size_t blendBufferSize, APPLYMETHOD applyMethod, size_t offset)
{
    if (offset > bufferSize) return;

    size_t bytesToUse = std::min(bufferSize - offset, blendBufferSize);
    wxByte* pb = buffer + offset;

    switch (applyMethod)
    {
    case APPLYMETHOD::METHOD_OVERWRITE:
        return Overwrite(pb, blendBuffer, bytesToUse);
    case APPLYMETHOD::METHOD_OVERWRITEIFZERO:
        return OverwriteIfZero(pb, blendBuffer, bytesToUse);
    case APPLYMETHOD::METHOD_MASK:
        return Mask(pb, blendBuffer, bytesToUse);
    case APPLYMETHOD::METHOD_UNMASK:
        return Unmask(pb, blendBuffer, bytesToUse);
    case APPLYMETHOD::METHOD_AVERAGE:
        return Average(pb, blendBuffer, bytesToUse);
    case APPLYMETHOD::METHOD_MAX:
        return Maximum(pb, blendBuffer, bytesToUse);
    case APPLYMETHOD::METHOD_OVERWRITEIFBLACK:
        return OverwriteIfBlack(pb, blendBuffer, bytesToUse / 3);
    case APPLYMETHOD::METHOD_MASKPIXEL:
        return MaskPixel(pb, blendBuffer, bytesToUse / 3);
    case APPLYMETHOD::METHOD_UNMASKPIXEL:
        return UnmaskPixel(pb, blendBuffer, bytesToUse / 3);
    case APPLYMETHOD::METHOD_MIN:
        return Minimum(pb, blendBuffer, bytesToUse);
    }
}

void Overwrite(wxByte* buffer, wxByte* blendBuffer, size_t channels)
{
    memcpy(buffer, blendBuffer, channels);
}

void OverwriteIfZero(wxByte* buffer, wxByte* blendBuffer, size_t channels)
{
    for (size_t i = 0; i < channels; ++i)
    {
        if (*(buffer + i) == 0x00)
        {
            *(buffer + i) = *(blendBuffer + i);
        }
    }
}

void Mask(wxByte* buffer, wxByte* blendBuffer, size_t channels)
{
    for (size_t i = 0; i < channels; ++i)
    {
        if (*(blendBuffer + i) > 0)
        {
            *(buffer + i) = 0x00;
        }
    }
}

void MaskPixel(wxByte* buffer, wxByte* blendBuffer, size_t pixels)
{
    for (size_t i = 0; i < pixels; ++i)
    {
        wxByte* p = blendBuffer + i * 3;
        auto sum = *p + *(p + 1) + *(p + 2);
        if (sum > 0)
        {
            wxByte* pp = buffer + i * 3;
            *pp = 0x00;
            *(pp + 1) = 0x00;
            *(pp + 2) = 0x00;
        }
    }
}

void Unmask(wxByte* buffer, wxByte* blendBuffer, size_t channels)
{
    for (size_t i = 0; i < channels; ++i)
    {
        if (*(blendBuffer + i) == 0)
        {
            *(buffer + i) = 0x00;
        }
    }
}

void UnmaskPixel(wxByte* buffer, wxByte* blendBuffer, size_t pixels)
{
    for (size_t i = 0; i < pixels; ++i)
    {
        wxByte* p = blendBuffer + i * 3;
        auto sum = *p + *(p + 1) + *(p + 2);
        if (sum == 0)
        {
            wxByte* pp = buffer + i * 3;
            *pp = 0x00;
            *(pp + 1) = 0x00;
            *(pp + 2) = 0x00;
        }
    }
}

void Average(wxByte* buffer, wxByte* blendBuffer, size_t channels)
{
    for (size_t i = 0; i < channels; ++i)
    {
        *(buffer + i) = (wxByte)(((int)*(buffer + i) + (int)*(blendBuffer + i)) / 2);
    }
}

void Maximum(wxByte* buffer, wxByte* blendBuffer, size_t channels)
{
    for (size_t i = 0; i < channels; ++i)
    {
        *(buffer + i) = std::max(*(buffer + i), *(blendBuffer + i));
    }
}

void Minimum(wxByte* buffer, wxByte* blendBuffer, size_t channels)
{
    for (size_t i = 0; i < channels; ++i)
    {
        *(buffer + i) = std::min(*(buffer + i), *(blendBuffer + i));
    }
}

void OverwriteIfBlack(wxByte* buffer, wxByte* blendBuffer, size_t pixels)
{
    for (size_t i = 0; i < pixels; ++i)
    {
        wxByte* p = buffer + i * 3;
        auto sum = *p + *(p + 1) + *(p + 2);
        if (sum == 0)
        {
            wxByte* pp = blendBuffer + i * 3;
            *p = *pp;
            *(p + 1) = *(pp + 1);
            *(p + 2) = *(pp + 2);
        }
    }
}