#ifndef ZCPPVISUALISEDIALOG_H
#define ZCPPVISUALISEDIALOG_H

//(*Headers(ZCPPVisualiseDialog)
#include <wx/dialog.h>
#include <wx/panel.h>
#include <wx/scrolbar.h>
#include <wx/sizer.h>
//*)

#include "controllers/ControllerUploadData.h"

class ZCPPVisualiseDialog: public wxDialog
{
    UDController& _cud;

	public:

		ZCPPVisualiseDialog(wxWindow* parent, UDController& cud, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ZCPPVisualiseDialog();

		//(*Declarations(ZCPPVisualiseDialog)
		wxPanel* Panel1;
		wxScrollBar* ScrollBar_Horizontal;
		wxScrollBar* ScrollBar_Vertical;
		//*)

	protected:

		//(*Identifiers(ZCPPVisualiseDialog)
		static const long ID_PANEL1;
		static const long ID_SCROLLBAR1;
		static const long ID_SCROLLBAR2;
		//*)

	private:

		//(*Handlers(ZCPPVisualiseDialog)
		void OnPanel1Paint(wxPaintEvent& event);
		void OnScrollBar_VerticalScroll(wxScrollEvent& event);
		void OnScrollBar_HorizontalScroll(wxScrollEvent& event);
		void OnScrollBar_HorizontalScrollThumbTrack(wxScrollEvent& event);
		void OnScrollBar_HorizontalScrollChanged(wxScrollEvent& event);
		void OnScrollBar_VerticalScrollChanged(wxScrollEvent& event);
		void OnScrollBar_VerticalScrollThumbTrack(wxScrollEvent& event);
		void OnPanel1Resize(wxSizeEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
