/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/imagwebp.cpp
// Purpose:     wxImage WebP handler
// Author:      Hermann Höhne
// Copyright:   (c) Hermann Höhne
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

// based on code by Sylvain Bougnoux and Khral Steelforge (https://forums.wxwidgets.org/viewtopic.php?t=39212)

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if wxUSE_IMAGE

#include "wx/imagwebp.h"
#include "webp/demux.h"
#include "webp/decode.h"
#include "webp/encode.h"

#ifndef WX_PRECOMP
    #include "wx/intl.h"
    #include "wx/log.h"
#endif

//-----------------------------------------------------------------------------
// wxWEBPHandler
//-----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxWEBPHandler, wxImageHandler);

#if wxUSE_STREAMS

#include <wx/mstream.h>

bool DecodeWebPDataIntoImage(wxImage *image, WebPData *webp_data, bool verbose) {
    WebPBitstreamFeatures features;
    VP8StatusCode status = WebPGetFeatures(webp_data->bytes, webp_data->size, &features);
    if (status != VP8_STATUS_OK) 
    {
        if (verbose) 
        {
           wxLogError("WebP: GetFeatures not OK.");
        }
        return false;
    }
    image->Create(features.width, features.height, false); // this allocates memory
    if (features.has_alpha) 
    {
        // image has alpha channel. needs to be decoded, then re-ordered.
        uint8_t * rgba = WebPDecodeRGBA(webp_data->bytes, webp_data->size, &features.width, &features.height);
        if (rgba == NULL) 
        {
            if (verbose) 
            {
               wxLogError("WebP: WebPDecodeRGBA failed.");
            }
            return false;
        }
        image->InitAlpha();
        unsigned char * rgb = image->GetData();
        unsigned char * alpha = image->GetAlpha();
        for (unsigned int index_pixel = 0; index_pixel < image->GetWidth() * image->GetHeight(); index_pixel++) {
            unsigned int index_rgba = index_pixel*4;
            unsigned int index_rgb = index_pixel*3;
            unsigned int index_alpha = index_pixel;
            rgb[index_rgb++] = rgba[index_rgba++]; // R
            rgb[index_rgb++] = rgba[index_rgba++]; // G
            rgb[index_rgb++] = rgba[index_rgba++]; // B
            alpha[index_alpha] = rgba[index_rgba]; // A
        }
        WebPFree(rgba);
    }
    else
    {
        // image has no alpha channel. decode into target buffer directly.
        int buffer_size = image->GetWidth() * image->GetHeight() * 3;
        int stride = image->GetWidth() * 3;
        uint8_t * output_buffer = WebPDecodeRGBInto(webp_data->bytes, webp_data->size, image->GetData(), buffer_size, stride);
        if (output_buffer == NULL)
        {
            if (verbose) 
            {
               wxLogError("WebP: WebPDecodeRGBInto failed.");
            }
            return false;
        }
    }
    image->SetMask(false); // all examples do this, so I do so as well
    return true;
}

bool DecodeWebPFrameIntoImage(wxImage *image, int index, WebPData *webp_data, bool verbose) 
{
    WebPDemuxer* demux = WebPDemux(webp_data);
    if (demux == NULL) 
    {
        if (verbose)
        {
            wxLogError("WebP: WebPDemux failed.");
        }
        return false;
    }
    //uint32_t width = WebPDemuxGetI(demux, WEBP_FF_CANVAS_WIDTH);
    //uint32_t height = WebPDemuxGetI(demux, WEBP_FF_CANVAS_HEIGHT);
    bool ok = false;
    WebPIterator iter;
    // wxImageHandler index starts from 0, WebPDemuxGetFrame seems to start from 1
    if (WebPDemuxGetFrame(demux, index+1, &iter)) 
    {
        ok = DecodeWebPDataIntoImage(image, &iter.fragment, verbose);
        WebPDemuxReleaseIterator(&iter);
    }
    WebPDemuxDelete(demux);
    return ok;
}

#include <iostream>

bool wxWEBPHandler::LoadFile(wxImage *image, wxInputStream& stream, bool verbose, int index)
{
    image->Destroy(); // all examples do this, so I do so as well
    wxMemoryOutputStream mos;
    stream.Read(mos); // this reads the entire file into memory
    wxStreamBuffer * mosb = mos.GetOutputStreamBuffer();
    WebPData webp_data;
    webp_data.bytes = reinterpret_cast<uint8_t *>(mosb->GetBufferStart());
    webp_data.size = mosb->GetBufferSize();
    // apparently, index can be -1 for "don't care", but libwebp does care
    if (index < 0) 
    {
        index = 0;
    }
    return DecodeWebPFrameIntoImage(image, index, &webp_data, verbose);
}

bool wxWEBPHandler::SaveFile(wxImage *image, wxOutputStream& stream, bool verbose)
{
    if (image->HasAlpha() && verbose)
    {
        wxLogWarning("WebP: Saving with alpha not implemented.");
    }
    //unsigned char * rgb = image->GetData();
    int stride = image->GetWidth();
    float quality_factor = 90;
    if (image->HasOption(wxIMAGE_OPTION_QUALITY))
    {
        quality_factor = image->GetOptionInt(wxIMAGE_OPTION_QUALITY) / 100.0f;
    }
    uint8_t * output = NULL;
    size_t output_size = WebPEncodeRGB(image->GetData(), image->GetWidth(), image->GetHeight(), stride, quality_factor, &output);
    stream.WriteAll(output, output_size);
    return true;
}

// TODO: implement int wxWEBPHandler::DoGetImageCount(wxInputStream & stream)

bool wxWEBPHandler::DoCanRead(wxInputStream& stream)
{
    // check header according to https://developers.google.com/speed/webp/docs/riff_container
    const std::string riff = "RIFF";
    const std::string webp = "WEBP";
    const int buffer_size = 12;
    char buffer[buffer_size];
    // it's ok to modify the stream position here
    stream.Read(buffer, buffer_size);
    if (stream.LastRead() != buffer_size) 
    {
        return false;
    }
    return std::string(buffer, 4) == riff && std::string(&buffer[8], 4) == webp;
}

#endif // wxUSE_STREAMS

#endif // wxUSE_IMAGE
