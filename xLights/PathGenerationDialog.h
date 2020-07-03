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

//(*Headers(PathGenerationDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/filepicker.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
//*)

#include <memory>

class PathGenerationDialog: public wxDialog
{
    std::unique_ptr<wxBitmap> _image;
    std::string _showFolder = "";
    bool unsaved = false;
    int _selected = -1;
    std::pair<float, float> _movefrom = {-1, -1};
    std::list<std::pair<float, float>> _points;
    std::list<std::tuple<int, std::string, std::pair<float,float>>> _undo;

    void RegenerateImage();
    std::list<std::pair<float, float>>::iterator GetPoint(int index);
    wxPoint CreatePoint(const std::pair<float, float>& pt) const;
    float TotalLength() const;
    static float GetDistance(const std::pair<float, float>& pt1, const std::pair<float, float>& pt2);
    std::pair<float, float> ConvertMousePosition(const wxPoint& pt) const;
    int GetMouseOverPoint(const std::pair<float, float>& pt, int start) const;
    int GetMouseOverLine(const std::pair<float, float>& pt) const;
    static float PointDistanceFromLineThrough(const std::pair<float, float>& pt, const std::pair<float, float>& lpt1, const std::pair<float, float>& lpt2);

	public:

		PathGenerationDialog(wxWindow* parent, const std::string& showFolder, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PathGenerationDialog();

		//(*Declarations(PathGenerationDialog)
		wxButton* Button_Close;
		wxButton* Button_FlipX;
		wxButton* Button_FlipY;
		wxButton* Button_Generate;
		wxButton* Button_Load;
		wxButton* Button_Rotate;
		wxFilePickerCtrl* FilePickerCtrl1;
		wxPanel* Panel1;
		wxSlider* Slider_Brightness;
		//*)

	protected:

		//(*Identifiers(PathGenerationDialog)
		static const long ID_PANEL1;
		static const long ID_FILEPICKERCTRL1;
		static const long ID_SLIDER1;
		static const long ID_BUTTON4;
		static const long ID_BUTTON5;
		static const long ID_BUTTON6;
		static const long ID_BUTTON3;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(PathGenerationDialog)
		void OnButton_LoadClick(wxCommandEvent& event);
		void OnButton_GenerateClick(wxCommandEvent& event);
		void OnButton_CloseClick(wxCommandEvent& event);
		void OnPanel1LeftDown(wxMouseEvent& event);
		void OnPanel1Paint(wxPaintEvent& event);
		void OnPanel1KeyDown(wxKeyEvent& event);
		void OnPanel1MouseMove(wxMouseEvent& event);
		void OnPanel1RightDown(wxMouseEvent& event);
		void OnPanel1LeftDClick(wxMouseEvent& event);
		void OnPanel1Resize(wxSizeEvent& event);
		void OnPanel1LeftUp1(wxMouseEvent& event);
		void OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event);
		void OnSlider_BrightnessCmdScrollChanged(wxScrollEvent& event);
		void OnSlider_BrightnessCmdSliderUpdated(wxScrollEvent& event);
		void OnButton_FlipXClick(wxCommandEvent& event);
		void OnButton_FlipYClick(wxCommandEvent& event);
		void OnButton_RotateClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

