// Most of this code is based on code linked from this article https://community.amd.com/thread/156793
// which is based on the code in VLC

#include "d3video.h"

#ifdef __WXMSW__

#include <cstdio>

#define COBJMACROS

#include <log4cpp/Category.hh>

struct video_format_t
{
    AVPixelFormat  _chroma;                               /**< picture chroma */
    unsigned int _width;                                 /**< picture width */
    unsigned int _height;                               /**< picture height */
    unsigned int _x_offset;               /**< start offset of visible area */
    unsigned int _y_offset;               /**< start offset of visible area */
    unsigned int _visible_width;                 /**< width of visible area */
    unsigned int _visible_height;               /**< height of visible area */

    unsigned int _bits_per_pixel;             /**< number of bits per pixel */

    unsigned int _sar_num;                   /**< sample/pixel aspect ratio */
    unsigned int _sar_den;

    unsigned int _frame_rate;                     /**< frame rate numerator */
    unsigned int _frame_rate_base;              /**< frame rate denominator */

    uint32_t _rmask, _gmask, _bmask;          /**< color masks for RGB chroma */
    int _rrshift;
    int _lrshift;
    int _rgshift;
    int _lgshift;
    int _rbshift;
    int _lbshift;
};

void DxDestroyVideoDecoder(D3Video_va_dxva2_t* va)
{
    if (va->decoder) va->decoder->Release();
    va->decoder = nullptr;

    for (unsigned i = 0; i < va->surface_count; i++)
    {
        va->surface[i].d3d->Release();
    }
    va->surface_count = 0;
}

const GUID DXVA_NoEncrypt = {
    0x1b81bed0, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

const GUID IID_IDirectXVideoDecoderService = {
        0xfc51a551, 0xd5e7, 0x11d9, {0xaf,0x55,0x00,0x05,0x4e,0x43,0xff,0x02}
};

int DxCreateVideoDecoder(D3Video_va_dxva2_t* va, int codec_id, const video_format_t* fmt)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("DxCreateVideoDecoder id %d %dx%d", codec_id, fmt->_width, fmt->_height);

    va->width = fmt->_width;
    va->height = fmt->_height;

    /* Allocates all surfaces needed for the decoder */
    va->surface_width = (fmt->_width + 15) & ~15;
    va->surface_height = (fmt->_height + 15) & ~15;
    switch (codec_id) {
    case AV_CODEC_ID_H264:
        va->surface_count = 16 + 1;
        break;
    default:
        va->surface_count = 2 + 1;
        break;
    }
    LPDIRECT3DSURFACE9 surface_list[VA_DXVA2_MAX_SURFACE_COUNT];
    if (FAILED(va->vs->CreateSurface(va->surface_width,
        va->surface_height,
        va->surface_count - 1,
        va->render,
        D3DPOOL_DEFAULT,
        0,
        DXVA2_VideoDecoderRenderTarget,
        surface_list,
        nullptr)))
    {
        logger_base.error("IDirectXVideoAccelerationService_CreateSurface failed.");
        va->surface_count = 0;
        return -1;
    }

    for (unsigned i = 0; i < va->surface_count; i++) {
        D3Video_va_surface_t* surface = &va->surface[i];
        surface->d3d = surface_list[i];
        surface->refcount = 0;
        surface->order = 0;
    }

    logger_base.debug("IDirectXVideoAccelerationService_CreateSurface succeed with %d surfaces (%dx%d)", va->surface_count, fmt->_width, fmt->_height);

    DXVA2_VideoDesc dsc;
    ZeroMemory(&dsc, sizeof(dsc));
    dsc.SampleWidth = fmt->_width;
    dsc.SampleHeight = fmt->_height;
    dsc.Format = va->render;
    if (fmt->_frame_rate > 0 && fmt->_frame_rate_base > 0)
    {
        dsc.InputSampleFreq.Numerator = fmt->_frame_rate;
        dsc.InputSampleFreq.Denominator = fmt->_frame_rate_base;
    }
    else
    {
        dsc.InputSampleFreq.Numerator = 0;
        dsc.InputSampleFreq.Denominator = 0;
    }
    dsc.OutputFrameFreq = dsc.InputSampleFreq;
    dsc.UABProtectionLevel = FALSE;
    dsc.Reserved = 0;

    /* FIXME I am unsure we can let unknown everywhere */
    DXVA2_ExtendedFormat* ext = &dsc.SampleFormat;
    ext->SampleFormat = 0;//DXVA2_SampleUnknown;
    ext->VideoChromaSubsampling = 0;//DXVA2_VideoChromaSubsampling_Unknown;
    ext->NominalRange = 0;//DXVA2_NominalRange_Unknown;
    ext->VideoTransferMatrix = 0;//DXVA2_VideoTransferMatrix_Unknown;
    ext->VideoLighting = 0;//DXVA2_VideoLighting_Unknown;
    ext->VideoPrimaries = 0;//DXVA2_VideoPrimaries_Unknown;
    ext->VideoTransferFunction = 0;//DXVA2_VideoTransFunc_Unknown;

    /* List all configurations available for the decoder */
    UINT cfg_count = 0;
    DXVA2_ConfigPictureDecode* cfg_list = nullptr;
    if (FAILED(va->vs->GetDecoderConfigurations(va->input,
        &dsc,
        nullptr,
        &cfg_count,
        &cfg_list)))
    {
        logger_base.error("IDirectXVideoDecoderService_GetDecoderConfigurations failed");
        return -1;
    }
    logger_base.debug("we got %d decoder configurations", cfg_count);

    /* Select the best decoder configuration */
    int cfg_score = 0;
    for (unsigned i = 0; i < cfg_count; i++)
    {
        const DXVA2_ConfigPictureDecode* cfg = &cfg_list[i];

        logger_base.debug("configuration[%d] ConfigBitstreamRaw %d", i, cfg->ConfigBitstreamRaw);

        int score;
        if (cfg->ConfigBitstreamRaw == 1)
            score = 1;
        else if (codec_id == AV_CODEC_ID_H264 && cfg->ConfigBitstreamRaw == 2)
            score = 2;
        else
            continue;

        if (IsEqualGUID(cfg->guidConfigBitstreamEncryption, DXVA_NoEncrypt))
            score += 16;

        if (cfg_score < score) {
            va->cfg = *cfg;
            cfg_score = score;
        }
    }

    CoTaskMemFree(cfg_list);

    if (cfg_score <= 0) {
        logger_base.error("Failed to find a supported decoder configuration");
        return -1;
    }

    /* Create the decoder */
    IDirectXVideoDecoder* decoder;
    if (FAILED(va->vs->CreateVideoDecoder(va->input,
        &dsc,
        &va->cfg,
        surface_list,
        va->surface_count,
        &decoder)))
    {
        logger_base.error("IDirectXVideoDecoderService_CreateVideoDecoder failed");
        return -1;
    }
    va->decoder = decoder;
    logger_base.debug("IDirectXVideoDecoderService_CreateVideoDecoder succeed");
    return 0;
}

void DxDestroyVideoService(D3Video_va_dxva2_t* va)
{
    if (va->device) va->devmng->CloseDeviceHandle(va->device);
    if (va->vs) va->vs->Release();
}

void D3dDestroyDeviceManager(D3Video_va_dxva2_t* va)
{
    if (va->devmng) va->devmng->Release();
}

void D3dDestroyDevice(D3Video_va_dxva2_t* va)
{
    if (va->d3ddev) IDirect3DDevice9_Release(va->d3ddev);
    if (va->d3dobj) IDirect3D9_Release(va->d3dobj);
}

void D3Video_Close(D3Video_va_dxva2_t* external)
{
    D3Video_va_dxva2_t* va = external;

    //DxDestroyVideoConversion(va);
    DxDestroyVideoDecoder(va);
    DxDestroyVideoService(va);
    D3dDestroyDeviceManager(va);
    D3dDestroyDevice(va);

    if (va->hdxva2_dll) FreeLibrary(va->hdxva2_dll);
    if (va->hd3d9_dll) FreeLibrary(va->hd3d9_dll);
    free(va);
}

int D3dCreateDevice(D3Video_va_dxva2_t* va)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    typedef LPDIRECT3D9(WINAPI * Create9func)(UINT SDKVersion);
    Create9func Create9 = (Create9func)GetProcAddress(va->hd3d9_dll, "Direct3DCreate9");

    if (!Create9) {
        logger_base.error("Cannot locate reference to Direct3DCreate9 ABI in DLL");
        return -1;
    }

    LPDIRECT3D9 d3dobj;
    d3dobj = Create9(D3D_SDK_VERSION);
    if (!d3dobj) {
        logger_base.error("Direct3DCreate9 failed");
        return -1;
    }
    va->d3dobj = d3dobj;

    D3DADAPTER_IDENTIFIER9* d3dai = &va->d3dai;
    if (FAILED(IDirect3D9_GetAdapterIdentifier(va->d3dobj,
        D3DADAPTER_DEFAULT, 0, d3dai)))
    {
        logger_base.warn("IDirect3D9_GetAdapterIdentifier failed");
        ZeroMemory(d3dai, sizeof(*d3dai));
    }

    D3DPRESENT_PARAMETERS* d3dpp = &va->d3dpp;
    ZeroMemory(d3dpp, sizeof(*d3dpp));
    d3dpp->Flags = D3DPRESENTFLAG_VIDEO;
    d3dpp->Windowed = TRUE;
    d3dpp->hDeviceWindow = NULL;
    d3dpp->SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp->MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpp->PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    d3dpp->BackBufferCount = 0;                  /* FIXME what to put here */
    d3dpp->BackBufferFormat = D3DFMT_X8R8G8B8;    /* FIXME what to put here */
    d3dpp->BackBufferWidth = 0;
    d3dpp->BackBufferHeight = 0;
    d3dpp->EnableAutoDepthStencil = FALSE;

    /* Direct3D needs a HWND to create a device, even without using ::Present
    this HWND is used to alert Direct3D when there's a change of focus window.
    For now, use GetShellWindow, as it looks harmless */
    LPDIRECT3DDEVICE9 d3ddev;
    if (FAILED(IDirect3D9_CreateDevice(d3dobj, D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL, GetShellWindow(),
        D3DCREATE_SOFTWARE_VERTEXPROCESSING |
        D3DCREATE_MULTITHREADED,
        d3dpp, &d3ddev)))
    {
        logger_base.error("IDirect3D9_CreateDevice failed");
        return -1;
    }
    va->d3ddev = d3ddev;

    return 0;
}

int D3dCreateDeviceManager(D3Video_va_dxva2_t* va)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    typedef HRESULT(WINAPI * CreateDeviceManager9_func)(UINT * pResetToken, IDirect3DDeviceManager9 * *);
    CreateDeviceManager9_func CreateDeviceManager9 =
        (CreateDeviceManager9_func)GetProcAddress(va->hdxva2_dll, "DXVA2CreateDirect3DDeviceManager9");

    if (!CreateDeviceManager9)
    {
        logger_base.error("cannot load function");
        return -1;
    }
    logger_base.debug("OurDirect3DCreateDeviceManager9 Success!");

    UINT token;
    IDirect3DDeviceManager9* devmng;
    if (FAILED(CreateDeviceManager9(&token, &devmng)))
    {
        logger_base.error(" OurDirect3DCreateDeviceManager9 failed");
        return -1;
    }

    HRESULT hr = devmng->ResetDevice(va->d3ddev, token);
    if (FAILED(hr))
    {
        logger_base.error("IDirect3DDeviceManager9_ResetDevice failed: %08x", (unsigned)hr);
        return -1;
    }

    devmng->AddRef();
    va->token = token;
    va->devmng = devmng;
    logger_base.debug("obtained IDirect3DDeviceManager9");

    return 0;
}

int DxCreateVideoService(D3Video_va_dxva2_t* va)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    typedef HRESULT(WINAPI * CreateVideoService_func)(IDirect3DDevice9*,
        REFIID riid,
        void** ppService);
    CreateVideoService_func CreateVideoService = (CreateVideoService_func)GetProcAddress(va->hdxva2_dll, "DXVA2CreateVideoService");

    if (!CreateVideoService)
    {
        logger_base.error("cannot load function");
        return 4;
    }
    logger_base.debug("DXVA2CreateVideoService Success!");

    HRESULT hr;
    HANDLE device;
    hr = va->devmng->OpenDeviceHandle(&device);
    if (FAILED(hr))
    {
        logger_base.error("OpenDeviceHandle failed");
        return -1;
    }
    va->device = device;

    IDirectXVideoDecoderService* vs;
    hr = va->devmng->GetVideoService(device,
        IID_IDirectXVideoDecoderService,
        (void**)& vs);
    if (FAILED(hr))
    {
        logger_base.error("GetVideoService failed");
        return -1;
    }
    va->vs = vs;

    return 0;
}

typedef struct {
    const char* name;
    const GUID& guid;
    int          codec;
} dxva2_mode_t;

const GUID GUID_NULL = {
        0x00000000, 0x0000,0x0000, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};

const GUID DXVA2_ModeMPEG2_MoComp = {
        0xe6a9f44b, 0x61b0,0x4563, {0x9e,0xa4,0x63,0xd2,0xa3,0xc6,0xfe,0x66}
};

const GUID DXVA2_ModeMPEG2_IDCT = {
        0xbf22ad00, 0x03ea,0x4690, {0x80,0x77,0x47,0x33,0x46,0x20,0x9b,0x7e}
};

const GUID DXVA2_ModeMPEG2_VLD = {
        0xee27417f, 0x5e28,0x4e65, {0xbe,0xea,0x1d,0x26,0xb5,0x08,0xad,0xc9}
};

const GUID DXVA2_ModeMPEG2and1_VLD = {
        0x86695f12, 0x340e,0x4f04, {0x9f,0xd3,0x92,0x53,0xdd,0x32,0x74,0x60}
};

const GUID DXVA2_ModeMPEG1_VLD = {
        0x6f3ec719, 0x3735,0x42cc, {0x80,0x63,0x65,0xcc,0x3c,0xb3,0x66,0x16}
};

const GUID DXVA2_ModeH264_A = {
        0x1b81be64, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

const GUID DXVA2_ModeH264_B = {
        0x1b81be65, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

const GUID DXVA2_ModeH264_C = {
        0x1b81be66, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

const GUID DXVA2_ModeH264_D = {
        0x1b81be67, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

const GUID DXVA2_ModeH264_E = {
        0x1b81be68, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

const GUID DXVA2_ModeH264_F = {
        0x1b81be69, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

const GUID DXVA_ModeH264_VLD_WithFMOASO_NoFGT = {
        0xd5f04ff9, 0x3418,0x45d8, {0x95,0x61,0x32,0xa7,0x6a,0xae,0x2d,0xdd}
};

const GUID DXVADDI_Intel_ModeH264_A = {
        0x604F8E64, 0x4951,0x4c54, {0x88,0xFE,0xAB,0xD2,0x5C,0x15,0xB3,0xD6}
};

const GUID DXVADDI_Intel_ModeH264_C = {
        0x604F8E66, 0x4951,0x4c54, {0x88,0xFE,0xAB,0xD2,0x5C,0x15,0xB3,0xD6}
};

const GUID DXVADDI_Intel_ModeH264_E = { // DXVA_Intel_H264_ClearVideo
        0x604F8E68, 0x4951,0x4c54, {0x88,0xFE,0xAB,0xD2,0x5C,0x15,0xB3,0xD6}
};

const GUID DXVA2_ModeWMV8_A = {
        0x1b81be80, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

const GUID DXVA2_ModeWMV8_B = {
        0x1b81be81, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

const GUID DXVA2_ModeWMV9_A = {
        0x1b81be90, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

const GUID DXVA2_ModeWMV9_B = {
        0x1b81be91, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

const GUID DXVA2_ModeWMV9_C = {
        0x1b81be94, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

const GUID DXVA2_ModeVC1_A = {
        0x1b81beA0, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

const GUID DXVA2_ModeVC1_B = {
        0x1b81beA1, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

const GUID DXVA2_ModeVC1_C = {
        0x1b81beA2, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

const GUID DXVA2_ModeVC1_D = {
        0x1b81beA3, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

/* Conformity to the August 2010 update of the specification, ModeVC1_VLD2010 */
const GUID DXVA2_ModeVC1_D2010 = {
        0x1b81beA4, 0xa0c7,0x11d3, {0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5}
};

const GUID DXVA_Intel_VC1_ClearVideo = {
        0xBCC5DB6D, 0xA2B6,0x4AF0, {0xAC,0xE4,0xAD,0xB1,0xF7,0x87,0xBC,0x89}
};

const GUID DXVA_nVidia_MPEG4_ASP = {
        0x9947EC6F, 0x689B,0x11DC, {0xA3,0x20,0x00,0x19,0xDB,0xBC,0x41,0x84}
};

const GUID DXVA_ModeMPEG4pt2_VLD_Simple = {
        0xefd64d74, 0xc9e8,0x41d7, {0xa5,0xe9,0xe9,0xb0,0xe3,0x9f,0xa3,0x19}
};

const GUID DXVA_ModeMPEG4pt2_VLD_AdvSimple_NoGMC = {
        0xed418a9f, 0x10d,0x4eda,  {0x9a,0xe3,0x9a,0x65,0x35,0x8d,0x8d,0x2e}
};

const GUID DXVA_ModeMPEG4pt2_VLD_AdvSimple_GMC = {
        0xab998b5b, 0x4258,0x44a9, {0x9f,0xeb,0x94,0xe5,0x97,0xa6,0xba,0xae}
};

/* XXX Prefered modes must come first */
const dxva2_mode_t dxva2_modes[] = {
        { "MPEG-2 variable-length decoder",            DXVA2_ModeMPEG2_VLD,     AV_CODEC_ID_MPEG2VIDEO },
        { "MPEG-2 & MPEG-1 variable-length decoder",   DXVA2_ModeMPEG2and1_VLD, AV_CODEC_ID_MPEG2VIDEO },
        { "MPEG-2 motion compensation",                DXVA2_ModeMPEG2_MoComp,  0 },
        { "MPEG-2 inverse discrete cosine transform",  DXVA2_ModeMPEG2_IDCT,    0 },

        { "MPEG-1 variable-length decoder",            DXVA2_ModeMPEG1_VLD,     0 },

        { "H.264 variable-length decoder, film grain technology",                      DXVA2_ModeH264_F,                   AV_CODEC_ID_H264 },
        { "H.264 variable-length decoder, no film grain technology",                   DXVA2_ModeH264_E,                   AV_CODEC_ID_H264 },
        { "H.264 variable-length decoder, no film grain technology (Intel ClearVideo)",DXVADDI_Intel_ModeH264_E,           AV_CODEC_ID_H264 },
        { "H.264 variable-length decoder, no film grain technology, FMO/ASO",          DXVA_ModeH264_VLD_WithFMOASO_NoFGT, AV_CODEC_ID_H264 },
        { "H.264 inverse discrete cosine transform, film grain technology",            DXVA2_ModeH264_D,                   0             },
        { "H.264 inverse discrete cosine transform, no film grain technology",         DXVA2_ModeH264_C,                   0             },
        { "H.264 inverse discrete cosine transform, no film grain technology (Intel)", DXVADDI_Intel_ModeH264_C,           0             },
        { "H.264 motion compensation, film grain technology",                          DXVA2_ModeH264_B,                   0             },
        { "H.264 motion compensation, no film grain technology",                       DXVA2_ModeH264_A,                   0             },
        { "H.264 motion compensation, no film grain technology (Intel)",               DXVADDI_Intel_ModeH264_A,           0             },

        { "Windows Media Video 8 motion compensation", DXVA2_ModeWMV8_B, 0 },
        { "Windows Media Video 8 post processing",     DXVA2_ModeWMV8_A, 0 },

        { "Windows Media Video 9 IDCT",                DXVA2_ModeWMV9_C, 0 },
        { "Windows Media Video 9 motion compensation", DXVA2_ModeWMV9_B, 0 },
        { "Windows Media Video 9 post processing",     DXVA2_ModeWMV9_A, 0 },

        { "VC-1 variable-length decoder",              DXVA2_ModeVC1_D, AV_CODEC_ID_VC1 },
        { "VC-1 variable-length decoder",              DXVA2_ModeVC1_D, AV_CODEC_ID_WMV3 },
        { "VC-1 variable-length decoder",              DXVA2_ModeVC1_D2010, AV_CODEC_ID_VC1 },
        { "VC-1 variable-length decoder",              DXVA2_ModeVC1_D2010, AV_CODEC_ID_WMV3 },
        { "VC-1 inverse discrete cosine transform",    DXVA2_ModeVC1_C, 0 },
        { "VC-1 motion compensation",                  DXVA2_ModeVC1_B, 0 },
        { "VC-1 post processing",                      DXVA2_ModeVC1_A, 0 },

        { "VC-1 variable-length decoder (Intel)",      DXVA_Intel_VC1_ClearVideo, 0 },

        { "MPEG-4 Part 2 nVidia bitstream decoder",                                                         DXVA_nVidia_MPEG4_ASP,                 0 },
        { "MPEG-4 Part 2 variable-length decoder, Simple Profile",                                          DXVA_ModeMPEG4pt2_VLD_Simple,          0 },
        { "MPEG-4 Part 2 variable-length decoder, Simple&Advanced Profile, no global motion compensation",  DXVA_ModeMPEG4pt2_VLD_AdvSimple_NoGMC, 0 },
        { "MPEG-4 Part 2 variable-length decoder, Simple&Advanced Profile, global motion compensation",     DXVA_ModeMPEG4pt2_VLD_AdvSimple_GMC,   0 },

        { NULL, GUID_NULL, 0 }
};

const dxva2_mode_t* Dxva2FindMode(const GUID& guid)
{
    for (unsigned i = 0; dxva2_modes[i].name; i++) {
        if (IsEqualGUID(dxva2_modes[i].guid, guid))
            return &dxva2_modes[i];
    }
    return nullptr;
}

typedef struct {
    const char* name;
    D3DFORMAT    format;
    AVPixelFormat  codec;
} d3d_format_t;

static const d3d_format_t d3d_formats[] = {
        { "YV12",   (D3DFORMAT)MAKEFOURCC('Y','V','1','2'),    AV_PIX_FMT_YUV420P },
        { "NV12",   (D3DFORMAT)MAKEFOURCC('N','V','1','2'),    AV_PIX_FMT_NV12 },
        { nullptr, (D3DFORMAT)0, AV_PIX_FMT_NONE }
};

const d3d_format_t* D3dFindFormat(D3DFORMAT format)
{
    for (unsigned i = 0; d3d_formats[i].name; i++) {
        if (d3d_formats[i].format == format) return &d3d_formats[i];
    }
    return nullptr;
}

int DxFindVideoServiceConversion(D3Video_va_dxva2_t* va, GUID* input, D3DFORMAT* output)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    /* Retrieve supported modes from the decoder service */
    UINT input_count = 0;
    GUID* input_list = nullptr;
    if (FAILED(va->vs->GetDecoderDeviceGuids(&input_count, &input_list)))
    {
        logger_base.error("IDirectXVideoDecoderService_GetDecoderDeviceGuids failed");
        return -1;
    }

#ifdef VIDEO_EXTRALOGGING
    for (unsigned i = 0; i < input_count; i++) {
        const GUID& g = input_list[i];
        const dxva2_mode_t* mode = Dxva2FindMode(g);
        if (mode) {
            logger_base.debug("   - '%s' is supported by hardware", mode->name);
        }
        else {
            logger_base.warn("  - Unknown GUID = %08X-%04x-%04x-XXXX",
                (unsigned)g.Data1, g.Data2, g.Data3);
        }
    }
#endif

    /* Try all supported mode by our priority */
    for (unsigned i = 0; dxva2_modes[i].name; i++)
    {
        const dxva2_mode_t* mode = &dxva2_modes[i];
        if (!mode->codec || mode->codec != va->codec_id)
            continue;

        bool is_suported = false;
        for (unsigned count = 0; !is_suported && count < input_count; count++) {
            const GUID& g = input_list[count];
            is_suported = IsEqualGUID(mode->guid, g) == 0;
        }
        if (!is_suported)
            continue;

        logger_base.debug("Trying to use '%s' as input", mode->name);
        UINT      output_count = 0;
        D3DFORMAT* output_list = nullptr;
        if (FAILED(va->vs->GetDecoderRenderTargets(mode->guid,
            &output_count,
            &output_list)))
        {
            logger_base.error("IDirectXVideoDecoderService_GetDecoderRenderTargets failed");
            continue;
        }

#ifdef VIDEO_EXTRALOGGING
        for (unsigned j = 0; j < output_count; j++) {
            const D3DFORMAT f = output_list[j];
            const d3d_format_t* format = D3dFindFormat(f);
            if (format) {
                logger_base.debug("%s is supported for output", format->name);
            }
            else {
                logger_base.debug("%d is supported for output (%4.4s)", f, (const char*)& f);
            }
        }
#endif

        for (unsigned j = 0; d3d_formats[j].name; j++) {
            const d3d_format_t* format = &d3d_formats[j];

            bool is_suported = false;
            for (unsigned k = 0; !is_suported && k < output_count; k++) {
                is_suported = format->format == output_list[k];
            }
            if (!is_suported)
                continue;

            /* We have our solution */
            logger_base.debug("Using '%s' to decode to '%s'", mode->name, format->name);
            *input = mode->guid;
            *output = format->format;
            CoTaskMemFree(output_list);
            CoTaskMemFree(input_list);
            return 0;
        }
        CoTaskMemFree(output_list);
    }
    CoTaskMemFree(input_list);
    return -1;
}

D3Video_va_dxva2_t* D3Video_va_NewDxva2(int codec_id)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    D3Video_va_dxva2_t* va = (D3Video_va_dxva2_t*)calloc(1, sizeof(*va));
    if (!va) return nullptr;

    va->codec_id = codec_id;

    bool cont = true;

    /* Load dll*/
    va->hd3d9_dll = LoadLibrary(TEXT("D3D9.DLL"));
    if (!va->hd3d9_dll) {
        logger_base.error("cannot load d3d9.dll");
        cont = false;
    }
    if (cont)
    {
        va->hdxva2_dll = LoadLibrary(TEXT("DXVA2.DLL"));
        if (!va->hdxva2_dll) {
            logger_base.error("cannot load dxva2.dll");
            cont = false;
        }
    }

    if (cont)
    {
        logger_base.debug("D3D9 DLLs loaded");
        if (D3dCreateDevice(va)) {
            logger_base.error("Failed to create Direct3D device");
            cont = false;;
        }
    }

    if (cont)
    {
        logger_base.debug("D3dCreateDevice succeed");
        if (D3dCreateDeviceManager(va)) {
            logger_base.error("D3dCreateDeviceManager failed");
            cont = false;
        }
    }

    if (cont)
    {
        if (DxCreateVideoService(va)) {
            logger_base.error("DxCreateVideoService failed");
            cont = false;
        }
    }

    if (cont)
    {
        if (DxFindVideoServiceConversion(va, &va->input, &va->render)) {
            logger_base.error("DxFindVideoServiceConversion failed");
            cont = false;
        }
    }

    if (cont)
    {
        /* TODO print the hardware name/vendor for debugging purposes */
        return va;
    }

    D3Video_Close(va);
    return nullptr;
}

void DxCreateVideoConversion(D3Video_va_dxva2_t* va)
{
    switch (va->render) {
    case MAKEFOURCC('N', 'V', '1', '2'):
        va->output = (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2');
        break;
    default:
        va->output = va->render;
        break;
    }
    //    CopyInitCache(&va->surface_cache, va->surface_width);
}

int D3Video_Setup(D3Video_va_dxva2_t* external, void** hw, AVPixelFormat* chroma, int width, int height)
{
    D3Video_va_dxva2_t* va = external;

    if (va->width == width && va->height == height && va->decoder) {
    }
    else {

        /* */
        //DxDestroyVideoConversion(va);
        DxDestroyVideoDecoder(va);

        *chroma = AV_PIX_FMT_NONE;
        if (width <= 0 || height <= 0)
            return -1;

        /* FIXME transmit a video_format_t by VaSetup directly */
        video_format_t fmt;
        memset(&fmt, 0, sizeof(fmt));
        fmt._width = width;
        fmt._height = height;

        if (DxCreateVideoDecoder(va, va->codec_id, &fmt))
            return -1;
        /* */
        va->hw.decoder = va->decoder;
        va->hw.cfg = &va->cfg;
        va->hw.surface_count = va->surface_count;
        va->hw.surface = va->hw_surface;
        for (unsigned i = 0; i < va->surface_count; i++) {
            va->hw.surface[i] = va->surface[i].d3d;
        }

        DxCreateVideoConversion(va);
    }

    *hw = &va->hw;
    const d3d_format_t* output = D3dFindFormat(va->output);
    *chroma = output->codec;

    return 0;
}
#endif
