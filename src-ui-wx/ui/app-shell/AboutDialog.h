#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

//(*Headers(AboutDialog)
#include <wx/dialog.h>
class wxFlexGridSizer;
class wxHyperlinkCtrl;
class wxStaticBitmap;
class wxStaticText;
class wxStdDialogButtonSizer;
//*)

class AboutDialog: public wxDialog
{
	public:

		AboutDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~AboutDialog();

		//(*Declarations(AboutDialog)
		wxFlexGridSizer* IconSizer;
		wxFlexGridSizer* MainSizer;
		wxHyperlinkCtrl* EULAHyperlinkCtrl;
		wxHyperlinkCtrl* PrivacyHyperlinkCtrl;
		wxStaticBitmap* IconBitmap;
		wxStaticText* LegalTextLabel;
		wxStaticText* VersionLabel;
		//*)

	protected:

		//(*Identifiers(AboutDialog)
		static const long ID_STATICBITMAP1;
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_HYPERLINKCTRL1;
		static const long ID_HYPERLINKCTRL2;
		//*)

	private:

		//(*Handlers(AboutDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
