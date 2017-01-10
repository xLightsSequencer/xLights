#ifndef PLAYLISTITEMIMAGEPANEL_H
#define PLAYLISTITEMIMAGEPANEL_H

//(*Headers(PlayListItemImagePanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/filepicker.h>
#include <wx/panel.h>
#include <wx/button.h>
//*)

class PlayListItemImage;
class ImageFilePickerCtrl;

class PlayListItemImagePanel: public wxPanel
{
    PlayListItemImage* _Image;

	public:

		PlayListItemImagePanel(wxWindow* parent, PlayListItemImage* Image,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemImagePanel();

		//(*Declarations(PlayListItemImagePanel)
		wxStaticText* StaticText2;
		wxStaticText* StaticText1;
		ImageFilePickerCtrl* FilePickerCtrl_ImageFile;
		wxTextCtrl* TextCtrl_Delay;
		wxButton* Button_PositionWindow;
		//*)

	protected:

		//(*Identifiers(PlayListItemImagePanel)
		static const long ID_STATICTEXT1;
		static const long ID_FILEPICKERCTRL1;
		static const long ID_BUTTON1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL1;
		//*)

	private:

		//(*Handlers(PlayListItemImagePanel)
		void OnButton_PositionWindowClick(wxCommandEvent& event);
		void OnFilePickerCtrl_ImageFileFileChanged(wxFileDirPickerEvent& event);
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
