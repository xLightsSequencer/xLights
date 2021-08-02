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

 //(*Headers(BufferSizeDialog)
 #include <wx/bmpbuttn.h>
 #include <wx/combobox.h>
 #include <wx/dialog.h>
 #include <wx/sizer.h>
 #include <wx/spinctrl.h>
 #include <wx/statline.h>
 #include <wx/stattext.h>
 //*)

#include <wx/dir.h>

#include "ValueCurveButton.h"
#include "xlLockButton.h"

class BufferSizeDialog: public wxDialog
{
    void ValidateWindow();

	public:

		BufferSizeDialog(wxWindow* parent, bool usevc, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~BufferSizeDialog();
        void SetSizes(double top, double left, double bottom, double right, const std::string& topvc, const std::string& leftvc, const std::string& bottomvc, const std::string& rightvc, double x, double y, const std::string& xvc, const std::string& yvc);
        void OnVCChanged(wxCommandEvent& event); 

		//(*Declarations(BufferSizeDialog)
		ValueCurveButton* ValueCurve_Bottom;
		ValueCurveButton* ValueCurve_Left;
		ValueCurveButton* ValueCurve_Right;
		ValueCurveButton* ValueCurve_Top;
		ValueCurveButton* ValueCurve_XC;
		ValueCurveButton* ValueCurve_YC;
		wxComboBox* ComboBoxBufferPresets;
		wxSpinCtrlDouble* SpinCtrl_Bottom;
		wxSpinCtrlDouble* SpinCtrl_Left;
		wxSpinCtrlDouble* SpinCtrl_Right;
		wxSpinCtrlDouble* SpinCtrl_Top;
		wxSpinCtrlDouble* SpinCtrl_XC;
		wxSpinCtrlDouble* SpinCtrl_YC;
		wxStaticLine* StaticLine1;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		xlSizedBitmapButton* BitmapButtonDelete;
		xlSizedBitmapButton* BitmapButtonSave;
		//*)

	protected:

		//(*Identifiers(BufferSizeDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_VALUECURVE_BufferTop;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		static const long ID_VALUECURVE_BufferLeft;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL3;
		static const long ID_VALUECURVE_BufferBottom;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL4;
		static const long ID_VALUECURVE_BufferRight;
		static const long ID_STATICTEXT5;
		static const long ID_SPINCTRLDOUBLE1;
		static const long ID_VALUECURVE_BufferXC;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRLDOUBLE2;
		static const long ID_VALUECURVE_BufferYC;
		static const long ID_COMBOBOX_BUFFER_PRESET;
		static const long ID_BITMAPBUTTON_SAVE;
		static const long ID_BITMAPBUTTON_DELETE;
		static const long ID_STATICLINE1;
		//*)

	private:

		//(*Handlers(BufferSizeDialog)
		void OnSpinCtrl_TopChange(wxSpinDoubleEvent& event);
		void OnSpinCtrl_LeftChange(wxSpinDoubleEvent& event);
		void OnSpinCtrl_BottomChange(wxSpinDoubleEvent& event);
		void OnSpinCtrl_RightChange(wxSpinDoubleEvent& event);
		void OnValueCurve_Click(wxCommandEvent& event);
		void OnBitmapButtonSaveClick(wxCommandEvent& event);
		void OnBitmapButtonDeleteClick(wxCommandEvent& event);
		void OnSpinCtrl_XCChange(wxSpinDoubleEvent& event);
		void OnSpinCtrl_YCChange(wxSpinDoubleEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

		void LoadBufferPreset(wxString const& name);
		void SaveBufferPreset(wxString const& name);

		void OnBuffer_PresetsSelect(wxCommandEvent& event);
		void OnBufferChoiceDropDown(wxCommandEvent& event);
		void CreateBufferPresetsList(wxDir& directory, bool subdirs);
		void LoadAllBufferPresets();
		wxString FindBufferPreset(const wxString& name) const;

		wxString _lastShowDir;
		std::list<std::string> _loadedBufferPresets;
};
