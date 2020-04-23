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

//(*Headers(VideoWindowPositionDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
//*)

class VideoWindowPositionDialog: public wxDialog
{
    wxSize _matrixSize;
    void ValidateWindow();

	public:

		VideoWindowPositionDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, const wxSize& matrixSize = wxSize(-1, -1), bool useMatrixSize = false, int matrixMultiplier = 1);
		virtual ~VideoWindowPositionDialog();
        wxSize GetDesiredSize() const;
        wxPoint GetDesiredPosition() const;

		//(*Declarations(VideoWindowPositionDialog)
		wxButton* Button_ok;
		wxCheckBox* CheckBox_SetSizeBasedOnMatrix;
		wxSpinCtrl* SpinCtrl_SizeMultiplier;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText_Position;
		//*)

	protected:

		//(*Identifiers(VideoWindowPositionDialog)
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL1;
		static const long ID_BUTTON1;
		//*)

	private:

		//(*Handlers(VideoWindowPositionDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnCheckBox_SetSizeBasedOnMatrixClick(wxCommandEvent& event);
		void OnSpinCtrl_SizeMultiplierChange(wxSpinEvent& event);
		//*)

        void OnMove(wxMoveEvent& event);
        void SetWindowPositionText();

		DECLARE_EVENT_TABLE()
};
