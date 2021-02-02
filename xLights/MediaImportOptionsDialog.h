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

//(*Headers(MediaImportOptionsDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include "SequencePackage.h"

class MediaImportOptionsDialog: public wxDialog
{
	public:

		MediaImportOptionsDialog(wxWindow* parent, SeqPkgImportOptions* importOptions, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MediaImportOptionsDialog();

		//(*Declarations(MediaImportOptionsDialog)
		wxButton* ButtonCancel;
		wxButton* ButtonFacesFolder;
		wxButton* ButtonGlediatorsFolder;
		wxButton* ButtonImagesFolder;
		wxButton* ButtonOK;
		wxButton* ButtonRestoreDefaults;
		wxButton* ButtonShadersFolder;
		wxButton* ButtonVideosFolder;
		wxStaticText* StaticText1;
		wxTextCtrl* FacesFolder;
		wxTextCtrl* GlediatorsFolder;
		wxTextCtrl* ImagesFolder;
		wxTextCtrl* ShadersFolder;
		wxTextCtrl* VideosFolder;
		//*)

	protected:

		//(*Identifiers(MediaImportOptionsDialog)
		static const long ID_STATICTEXT1;
		static const long ID_BUTTON_RESTORE_DEFAULTS;
		static const long ID_TEXTCTRL_FACES_FLD;
		static const long ID_BUTTON_FACES_FLD;
		static const long ID_TEXTCTRL_GLEDIATORS_FLD;
		static const long ID_BUTTON_GLEDIATORS_FLD;
		static const long ID_TEXTCTRL3;
		static const long ID_BUTTON_IMAGES_FLD;
		static const long ID_TEXTCTRL_SHADERS_FLD;
		static const long ID_BUTTON_SHADERS_FLD;
		static const long ID_TEXTCTRL_VIDEOS_FLD;
		static const long ID_BUTTON_VIDEOS_FLD;
		static const long ID_BUTTON_OK;
		static const long ID_BUTTON_CANCEL;
		//*)

	private:

		//(*Handlers(MediaImportOptionsDialog)
		void OnInit(wxInitDialogEvent& event);
		void OnButtonSelectFolderClick(wxCommandEvent& event);
		void OnButtonOKClick(wxCommandEvent& event);
		void OnButtonCancelClick(wxCommandEvent& event);
		void OnButtonRestoreDefaultsClick(wxCommandEvent& event);
		//*)
    
        void UpdateOptions();
        void GetCurrentLabelAndPath(int btnId, std::string& label, std::string& path);
        void UpdateFolderPath(int btnId, std::string& newPath);
        
        SeqPkgImportOptions* _importOptions;
        SeqPkgImportOptions _defaultOptions;

		DECLARE_EVENT_TABLE()
};
