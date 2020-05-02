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

#include "xlGLCanvas.h"
#include "../BulkEditControls.h"

class ShaderConfig;

class ShaderPreview : public xlGLCanvas
{
public:
   ShaderPreview(wxWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                 const wxSize &size=wxDefaultSize,
                 long style=0,
                 const wxString &name=wxPanelNameStr,
                 bool coreProfile = true);
   virtual ~ShaderPreview();

protected:
   void InitializeGLContext() override;
};

//(*Headers(ShaderPanel)
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/filepicker.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class ShaderPanel: public wxPanel
{
    ShaderConfig* _shaderConfig = nullptr;

	public:

		ShaderPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ShaderPanel();

		//(*Declarations(ShaderPanel)
		BulkEditFilePickerCtrl* FilePickerCtrl1;
		BulkEditSlider* Slider_Shader_LeadIn;
		BulkEditSliderF2* Slider_Shader_Speed;
		BulkEditTextCtrl* TextCtrl_Shader_LeadIn;
		BulkEditTextCtrlF2* TextCtrl_Shader_Speed;
		BulkEditValueCurveButton* BitmapButton_Shader_Speed;
		wxButton* Button_Download;
		wxFlexGridSizer* FlexGridSizer1;
		wxFlexGridSizer* FlexGridSizer_Dynamic;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		//*)

	protected:

		//(*Identifiers(ShaderPanel)
		static const long ID_STATICTEXT1;
		static const long ID_0FILEPICKERCTRL_IFS;
		static const long ID_BUTTON1;
		static const long ID_STATICTEXT2;
		static const long IDD_SLIDER_Shader_LeadIn;
		static const long ID_TEXTCTRL_Shader_LeadIn;
		static const long ID_STATICTEXT3;
		static const long ID_SLIDER_Shader_Speed;
		static const long ID_VALUECURVE_Shader_Speed;
		static const long IDD_TEXTCTRL_Shader_Speed;
		//*)

		ShaderPreview *   _preview;
		static const long ID_CANVAS;
      friend class ShaderEffect;

	private:

		//(*Handlers(ShaderPanel)
		void OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event);
		void OnButton_DownloadClick(wxCommandEvent& event);
		//*)

        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        void OnLockButtonClick(wxCommandEvent& event);
        bool BuildUI(const wxString& filename);

		DECLARE_EVENT_TABLE()
};
