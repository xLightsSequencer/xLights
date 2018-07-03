#ifndef LYRICUSERDICTDIALOG_H
#define LYRICUSERDICTDIALOG_H

//(*Headers(LyricUserDictDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PhonemeDictionary;

class LyricUserDictDialog: public wxDialog
{
	public:

		LyricUserDictDialog(PhonemeDictionary* dictionary, const wxString &showDirectory, wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~LyricUserDictDialog();

		//(*Declarations(LyricUserDictDialog)
		wxButton* ButtonAddLyric;
		wxButton* ButtonDeleteRow;
		wxButton* ButtonLyricCancel;
		wxButton* ButtonLyricOK;
		wxGrid* GridUserLyricDict;
		wxStaticText* StaticText1;
		wxTextCtrl* TextCtrlNewLyric;
		wxTextCtrl* TextCtrlOldLyric;
		//*)

	protected:

		//(*Identifiers(LyricUserDictDialog)
		static const long ID_TEXTCTRL_NEW_LYRIC;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL_OLD_LYRIC;
		static const long ID_BUTTON_ADD_LYRIC;
		static const long ID_GRID_USER_LYRIC_DICT;
		static const long ID_BUTTON_DELETE_ROW;
		static const long ID_BUTTON_LYRIC_OK;
		static const long ID_BUTTON_LYRIC_CANCEL;
		//*)

	private:

        PhonemeDictionary * m_dictionary;
        wxString m_showDirectory;
        wxArrayString m_removeList;

        void ReadUserDictionary() const;
        void WriteUserDictionary() const;
        void InsertRow(const wxString& text, wxArrayString phonemeList) const;
        bool DoesGridContain(const wxString& text) const;
        bool IsValidPhoneme(const wxString& text) const;

		//(*Handlers(LyricUserDictDialog)
		void OnButtonAddLyricClick(wxCommandEvent& event);
		void OnButtonDeleteRowClick(wxCommandEvent& event);
		void OnButtonLyricOKClick(wxCommandEvent& event);
		void OnButtonLyricCancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
