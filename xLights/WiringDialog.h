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

//(*Headers(WiringDialog)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
//*)

#include <wx/grid.h>
#include <wx/bitmap.h>
#include <map>
#include <list>
#include <wx/prntbase.h>
#include <wx/generic/statbmpg.h>

class WiringDialog;
class Model;

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
    wxPoint _start = wxPoint(0, 0);
    wxPoint _lastMouse = wxPoint(0, 0);
    wxString _modelname;
    wxBitmap _bmp;
    bool _rear;
    bool _multilight;
    ColorTheme _selectedTheme;
    int _cols;
    int _rows;
    int _fontSize;
    std::map<int, std::map<int, std::list<wxRealPoint>>> _points;
    void RenderMultiLight(wxBitmap& bitmap, std::map<int, std::map<int, std::list<wxRealPoint>>>& points, int width, int height, bool printer = false);
    wxBitmap Render(int w, int h);
    void RenderNodes(wxBitmap& bitmap, std::map<int, std::map<int, std::list<wxRealPoint>>>& points, int width, int height, bool printer = false);
    std::map<int, std::list<wxRealPoint>> ExtractPoints(wxGrid* grid, bool reverse);
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

    static const long ID_MNU_RESET;
    static const long ID_MNU_EXPORT;
    static const long ID_MNU_EXPORTLARGE;
    static const long ID_MNU_PRINT;
    static const long ID_MNU_DARK;
    static const long ID_MNU_GRAY;
    static const long ID_MNU_LIGHT;
    static const long ID_MNU_FRONT;
    static const long ID_MNU_REAR;
    static const long ID_MNU_FONTSMALLER;
    static const long ID_MNU_FONTLARGER;
    void Render();

    public:

		WiringDialog(wxWindow* parent, wxString modelname, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~WiringDialog();
        void SetColorTheme(COLORTHEMETYPE themeType);
        void SetData(wxGrid* grid, bool reverse);
        void SetData(Model* model);
        void DrawBitmap(wxBitmap& bitmap, bool printer = false);

		//(*Declarations(WiringDialog)
		wxGenericStaticBitmap* StaticBitmap_Wiring;
		//*)

	protected:

		//(*Identifiers(WiringDialog)
		static const long ID_STATICBITMAP1;
		//*)

	private:

		//(*Handlers(WiringDialog)
		void OnResize(wxSizeEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
