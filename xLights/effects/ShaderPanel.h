#ifndef SHADERPANEL_H
#define SHADERPANEL_H

#include "xlGLCanvas.h"

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
#include <wx/button.h>
#include <wx/filepicker.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

class ShaderPanel: public wxPanel
{
    ShaderConfig* _shaderConfig = nullptr;

	public:

		ShaderPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ShaderPanel();

		//(*Declarations(ShaderPanel)
		wxButton* Button_Download;
		wxFilePickerCtrl* FilePickerCtrl1;
		wxFlexGridSizer* FlexGridSizer_Dynamic;
		wxStaticText* StaticText1;
		//*)

	protected:

		//(*Identifiers(ShaderPanel)
		static const long ID_STATICTEXT1;
		static const long ID_0FILEPICKERCTRL_IFS;
		static const long ID_BUTTON1;
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
        void BuildUI(const wxString& filename);

		DECLARE_EVENT_TABLE()
};

#endif
