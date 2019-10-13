#pragma once

#include <wx/wx.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

#ifdef __WXMSW__

	#include <d3d9.h>
	#include <dxva2api.h>
	#include <libavcodec/dxva2.h>

	#define VA_DXVA2_MAX_SURFACE_COUNT (64)

	typedef struct {
		LPDIRECT3DSURFACE9 d3d;
		int                refcount;
		unsigned int       order;
	} D3Video_va_surface_t;

	typedef struct
	{
		int          codec_id;
		int          width;
		int          height;

		/* DLL */
		HINSTANCE             hd3d9_dll;
		HINSTANCE             hdxva2_dll;

		/* Direct3D */
		D3DPRESENT_PARAMETERS  d3dpp;
		LPDIRECT3D9            d3dobj;
		D3DADAPTER_IDENTIFIER9 d3dai;
		LPDIRECT3DDEVICE9      d3ddev;

		/* Device manager */
		UINT                     token;
		IDirect3DDeviceManager9* devmng;
		HANDLE                   device;

		/* Video service */
		IDirectXVideoDecoderService* vs;
		GUID                         input;
		D3DFORMAT                    render;

		/* Video decoder */
		DXVA2_ConfigPictureDecode    cfg;
		IDirectXVideoDecoder* decoder;

		/* Option conversion */
		D3DFORMAT                    output;

		/* */
		struct dxva_context hw;

		/* */
		unsigned     surface_count;
		unsigned     surface_order;
		int          surface_width;
		int          surface_height;

		D3Video_va_surface_t surface[VA_DXVA2_MAX_SURFACE_COUNT];
		LPDIRECT3DSURFACE9 hw_surface[VA_DXVA2_MAX_SURFACE_COUNT];
	} D3Video_va_dxva2_t;
	
	int D3Video_Setup(D3Video_va_dxva2_t* external, void** hw, AVPixelFormat* chroma, int width, int height);
    D3Video_va_dxva2_t* D3Video_va_NewDxva2(int codec_id);
    void D3Video_Close(D3Video_va_dxva2_t* external);

#else
	typedef struct {
		int dummy;
	} D3Video_va_surface_t;

	typedef struct
	{
		int dummy;
	} D3Video_va_dxva2_t;
	
	int D3Video_Setup(D3Video_va_dxva2_t* external, void** hw, AVPixelFormat* chroma, int width, int height) { return -1; }
	D3Video_va_dxva2_t* D3Video_va_NewDxva2(int codec_id) { return nullptr; }
	void D3Video_Close(D3Video_va_dxva2_t* external) {}

#endif