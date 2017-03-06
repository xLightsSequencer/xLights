#include "Blend.h"

//#define SIMD

#ifdef SIMD
#include "emmintrin.h"
#endif

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
    std::string bm = wxString(blendMode).Lower().ToStdString();

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
#ifdef SIMD
    __m128i zero = _mm_setr_epi32(0, 0, 0, 0);
    int simd = channels / 16;

    for (size_t i = 0; i < simd; ++i)
    {
        __m128i b = _mm_load_si128((__m128i*)(buffer + i * 16));
        __m128i bb = _mm_load_si128((__m128i*)(blendBuffer + i * 16));

        __m128i mask = _mm_cmpeq_epi8(b, zero); // sets FF where B is zero
        __m128i newv = _mm_and_si128(mask, bb); // grab bb where B has zero
        __m128i r = _mm_or_si128(b, newv); // merge them

        _mm_store_si128((__m128i*)(buffer + i * 16), r);
    }

    // do the residual
    for (size_t i = 0; i < channels % 16; ++i)
    {
        size_t offset = i + simd * 16;
        if (*(buffer + offset) == 0)
        {
            *(buffer + offset) = *(blendBuffer + offset);
        }
    }
#else
    for (size_t i = 0; i < channels; ++i)
    {
        if (*(buffer + i) == 0x00)
        {
            *(buffer + i) = *(blendBuffer + i);
        }
    }
#endif
}

void Mask(wxByte* buffer, wxByte* blendBuffer, size_t channels)
{
#ifdef SIMD
    __m128i zero = _mm_setr_epi32(0, 0, 0, 0);
    int simd = channels / 16;

    for (size_t i = 0; i < simd; ++i)
    {
        __m128i b = _mm_load_si128((__m128i*)(buffer + i * 16));
        __m128i bb = _mm_load_si128((__m128i*)(blendBuffer + i * 16));

        __m128i mask = _mm_cmpeq_epi8(bb, zero); // sets FF where BB is zero
        __m128i r = _mm_and_si128(mask, b); // and the mask

        _mm_store_si128((__m128i*)(buffer + i * 16), r);
    }

    // do the residual
    for (size_t i = 0; i < channels % 16; ++i)
    {
        size_t offset = i + simd * 16;
        if (*(blendBuffer + offset) > 0)
        {
            *(buffer + offset) = 0x00;
        }
    }
#else
    for (size_t i = 0; i < channels; ++i)
    {
        if (*(blendBuffer + i) > 0)
        {
            *(buffer + i) = 0x00;
        }
    }
#endif
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
#ifdef SIMD
    __m128i zero = _mm_setr_epi32(0, 0, 0, 0);
    int simd = channels / 16;

    for (size_t i = 0; i < simd; ++i)
    {
        __m128i b = _mm_load_si128((__m128i*)(buffer + i * 16));
        __m128i bb = _mm_load_si128((__m128i*)(blendBuffer + i * 16));

        __m128i mask = _mm_cmpeq_epi8(bb, zero); // sets FF where BB is zero
        __m128i r = _mm_andnot_si128(mask, b); // invert the mask and then and it

        _mm_store_si128((__m128i*)(buffer + i * 16), r);
    }

    // do the residual
    for (size_t i = 0; i < channels % 16; ++i)
    {
        size_t offset = i + simd * 16;
        if (*(blendBuffer + offset) == 0)
        {
            *(buffer + offset) = 0x00;
        }
    }
#else
    for (size_t i = 0; i < channels; ++i)
    {
        if (*(blendBuffer + i) == 0)
        {
            *(buffer + i) = 0x00;
        }
    }
#endif
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
#ifdef SIMD
    int simd = channels / 16;

    for (size_t i = 0; i < simd; ++i)
    {
        __m128i b = _mm_load_si128((__m128i*)(buffer + i * 16));
        __m128i bb = _mm_load_si128((__m128i*)(blendBuffer + i * 16));

        __m128i r = _mm_avg_epu8(b, bb);

        _mm_store_si128((__m128i*)(buffer + i * 16), r);
    }

    // do the residual
    for (size_t i = 0; i < channels % 16; ++i)
    {
        size_t offset = i + simd * 16;
        *(buffer + offset) = (wxByte)(((int)*(buffer + offset) + (int)*(blendBuffer + offset)) / 2);
    }
#else
    for (size_t i = 0; i < channels; ++i)
    {
        *(buffer + i) = (wxByte)(((int)*(buffer + i) + (int)*(blendBuffer + i)) / 2);
    }
#endif
}

void Maximum(wxByte* buffer, wxByte* blendBuffer, size_t channels)
{
#ifdef SIMD
    int simd = channels / 16;

    for (size_t i = 0; i < simd; ++i)
    {
        __m128i b = _mm_load_si128((__m128i*)(buffer + i * 16));
        __m128i bb = _mm_load_si128((__m128i*)(blendBuffer + i * 16));

        __m128i r = _mm_min_epu8(b, bb);

        _mm_store_si128((__m128i*)(buffer + i * 16), r);
    }

    // do the residual
    for (size_t i = 0; i < channels % 16; ++i)
    {
        size_t offset = i + simd * 16;
        *(buffer + offset) = std::max(*(buffer + offset), *(blendBuffer + offset));
    }
#else
    for (size_t i = 0; i < channels; ++i)
    {
        *(buffer + i) = std::max(*(buffer + i), *(blendBuffer + i));
    }
#endif
}

void Minimum(wxByte* buffer, wxByte* blendBuffer, size_t channels)
{
#ifdef SIMD
    int simd = channels / 16;

    for (size_t i = 0; i < simd; ++i)
    {
        __m128i b = _mm_load_si128((__m128i*)(buffer + i * 16));
        __m128i bb = _mm_load_si128((__m128i*)(blendBuffer + i * 16));

        __m128i r = _mm_max_epu8(b, bb);

        _mm_store_si128((__m128i*)(buffer + i * 16), r);
    }

    // do the residual
    for (size_t i = 0; i < channels % 16; ++i)
    {
        size_t offset = i + simd * 16;
        *(buffer + offset) = std::min(*(buffer + offset), *(blendBuffer + offset));
    }
#else
    for (size_t i = 0; i < channels; ++i)
    {
        *(buffer + i) = std::min(*(buffer + i), *(blendBuffer + i));
    }
#endif
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
