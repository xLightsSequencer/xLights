#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(WiringDialog)
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
//*)

#include <wx/grid.h>
#include <wx/bitmap.h>
#include <map>
#include <list>
#include <wx/prntbase.h>
#include <wx/generic/statbmpg.h>

class WiringDialog;
class Model;

// wxGenericStaticBitmap::SetBitmap() unconditionally resizes the control to
// the bitmap's best size, fighting our sizer (we always hand it a bitmap
// already sized to the control's current allocated area). This fork has no
// wxST_NO_AUTORESIZE style to opt out, so suppress the resize directly.
class WiringStaticBitmap : public wxGenericStaticBitmap
{
public:
    WiringStaticBitmap(wxWindow* parent, wxWindowID id, const wxBitmapBundle& bitmap,
                        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
                        long style = 0, const wxString& name = wxASCII_STR(wxStaticBitmapNameStr))
        : wxGenericStaticBitmap(parent, id, bitmap, pos, size, style, name)
    {
    }

    // wxStaticBitmapBase::GetBitmap() calls m_bitmapBundle.GetBitmapFor(this),
    // which rescales the bitmap to match this->GetDPIScaleFactor() (the real
    // Windows display-scaling ratio, unlike GetContentScaleFactor() which is
    // always 1 on this platform). We always hand it a bitmap already sized
    // in real pixels for the control's current allocated area, so that
    // DPI-based rescale silently stretches our pixel-exact render on any
    // non-100%-scaled display. Bypass it and return our bitmap as-is.
    wxBitmap GetBitmap() const override
    {
        return m_bitmapBundle.GetBitmap(m_bitmapBundle.GetDefaultSize());
    }

    void SetBitmap(const wxBitmapBundle& bitmap) override
    {
        m_bitmapBundle = bitmap;
        Refresh();
    }
};

class WiringPrintout : public wxPrintout
{
    WiringDialog* _wiringDialog;
public:
    WiringPrintout(WiringDialog* dialog);
    virtual bool OnPrintPage(int pageNum) override;
};

typedef enum COLORTHEMETYPE {
    DARK,
    GRAY,
    LIGHT
} COLORTHEMETYPE;

// A node's 2D layout position, plus its model-local depth (z) so the 3D
// wiring view can rotate/project it. z is unused (stays 0) everywhere the
// 2D view populates points; kept as a drop-in for wxRealPoint(x,y) call
// sites elsewhere in this file, which only ever access .x/.y.
struct WiringPoint {
    double x = 0;
    double y = 0;
    double z = 0;
    WiringPoint() = default;
    WiringPoint(double x_, double y_, double z_ = 0.0) : x(x_), y(y_), z(z_) {}
};

struct ColorTheme {
    COLORTHEMETYPE type;
    bool multiLightDark;
    wxColour background;
    wxColour messageFill;
    wxColour messageAltFill;
    wxColour messageOutline;
    wxColour wiringFill;
    wxColour wiringOutline;
    wxColour nodeFill;
    wxColour nodeOutline;
    wxColour labelFill;
    wxColour labelOutline;
};

class WiringDialog: public wxDialog
{
    float _zoom = 1.0f;
    wxRealPoint _start = wxRealPoint(0, 0);
    wxPoint _lastMouse = wxPoint(0, 0);
    wxString _modelname;
    wxBitmap _bmp;
    bool _rear;
    bool _multilight;
    bool _rotated;
    ColorTheme _selectedTheme;
    int _cols;
    int _rows;
    int _fontSize;
    int _rotation;
    bool _is3D = false;
    double _rotX = -0.4;  // pitch, radians
    double _rotY = 0.6;   // yaw, radians
    std::map<int, std::map<int, std::list<WiringPoint>>> _points;
    std::map<int, std::map<int, std::list<WiringPoint>>> _originalPoints;
    std::map<int, std::map<int, std::list<WiringPoint>>> Apply3DRotation(const std::map<int, std::map<int, std::list<WiringPoint>>>& points) const;
    void RenderMultiLight(wxBitmap& bitmap, std::map<int, std::map<int, std::list<WiringPoint>>>& points, int width, int height, bool printer = false);
    wxBitmap Render(int w, int h);
    void RenderNodes(wxBitmap& bitmap, std::map<int, std::map<int, std::list<WiringPoint>>>& points, int width, int height, bool printer = false);
    std::map<int, std::list<WiringPoint>> ExtractPoints(wxGrid* grid, bool reverse);
    void RotatePoints(int rotateBy);
    void RightClick(wxContextMenuEvent& event);
    void OnPopup(wxCommandEvent& event);
    void LeftDown(wxMouseEvent& event);
    void LeftUp(wxMouseEvent& event);
    void Motion(wxMouseEvent& event);
    void MouseWheel(wxMouseEvent& event);
    void Magnify(wxMouseEvent& event);
    void LeftDClick(wxMouseEvent& event);
    void CaptureLost(wxMouseCaptureLostEvent& event) {}
    void AdjustZoom(float by, wxPoint mousePos);
    void Export_DXF();

    static const long ID_MNU_RESET;
    static const long ID_MNU_EXPORT;
    static const long ID_MNU_EXPORTLARGE;
    static const long ID_MNU_EXPORTDXF;
    static const long ID_MNU_PRINT;
    static const long ID_MNU_DARK;
    static const long ID_MNU_GRAY;
    static const long ID_MNU_LIGHT;
    static const long ID_MNU_FRONT;
    static const long ID_MNU_REAR;
    static const long ID_MNU_FONTSMALLER;
    static const long ID_MNU_FONTLARGER;
    static const long ID_MNU_ROTATE;
    void Render();

    public:

		WiringDialog(wxWindow* parent, wxString modelname, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~WiringDialog();
        void SetColorTheme(COLORTHEMETYPE themeType);
        void SetData(wxGrid* grid, bool reverse);
        void SetData(Model* model);
        void DrawBitmap(wxBitmap& bitmap, bool printer = false);

		//(*Declarations(WiringDialog)
		WiringStaticBitmap* StaticBitmap_Wiring;
		wxStaticText* StaticText_FontSize;
		wxSlider* Slider_FontSize;
		wxCheckBox* CheckBox_3D;
		//*)

	protected:

		//(*Identifiers(WiringDialog)
		static const long ID_STATICBITMAP1;
		static const long ID_STATICTEXT_FONTSIZE;
		static const long ID_SLIDER_FONTSIZE;
		static const long ID_CHECKBOX_3D;
		//*)

	private:

		//(*Handlers(WiringDialog)
		void OnResize(wxSizeEvent& event);
		void OnFontSizeSliderChanged(wxCommandEvent& event);
		void On3DCheckBoxChanged(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
