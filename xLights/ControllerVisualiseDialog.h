#ifndef CONTROLLERVISUALISEDIALOG_H
#define CONTROLLERVISUALISEDIALOG_H

//(*Headers(ControllerVisualiseDialog)
#include <wx/dialog.h>
#include <wx/panel.h>
#include <wx/scrolbar.h>
#include <wx/sizer.h>
//*)

#include "controllers/ControllerUploadData.h"
#include <wx/prntbase.h>

class ControllerVisualiseDialog;
class Output;

class ControllerVisualisePrintout : public wxPrintout
{
    ControllerVisualiseDialog* _controllerDialog;
public:
    ControllerVisualisePrintout(ControllerVisualiseDialog* controllerDialog);
    virtual bool OnPrintPage(int pageNum) override;
};

class ControllerVisualiseDialog: public wxDialog
{
    UDController& _cud;
    std::string _ip;
    std::string _description;

	public:

		ControllerVisualiseDialog(wxWindow* parent, UDController& cud, const std::string& ip, const std::string& description, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~ControllerVisualiseDialog();

		//(*Declarations(ControllerVisualiseDialog)
		wxPanel* Panel1;
		wxScrollBar* ScrollBar_Horizontal;
		wxScrollBar* ScrollBar_Vertical;
		//*)

        static const long CONTROLLERVISUALISE_PRINT;
        static const long CONTROLLERVISUALISE_SAVE_CSV;

        void RenderPicture(wxBitmap& bitmap, bool printer);

	protected:

		//(*Identifiers(ControllerVisualiseDialog)
		static const long ID_PANEL1;
		static const long ID_SCROLLBAR1;
		static const long ID_SCROLLBAR2;
		//*)

	private:

		//(*Handlers(ControllerVisualiseDialog)
		void OnPanel1Paint(wxPaintEvent& event);
		void OnScrollBar_VerticalScroll(wxScrollEvent& event);
		void OnScrollBar_HorizontalScroll(wxScrollEvent& event);
		void OnScrollBar_HorizontalScrollThumbTrack(wxScrollEvent& event);
		void OnScrollBar_HorizontalScrollChanged(wxScrollEvent& event);
		void OnScrollBar_VerticalScrollChanged(wxScrollEvent& event);
		void OnScrollBar_VerticalScrollThumbTrack(wxScrollEvent& event);
		void OnPanel1Resize(wxSizeEvent& event);
		void OnPanel1RightDown(wxMouseEvent& event);
		void OnPanel1MouseWheel(wxMouseEvent& event);
		//*)

        void OnPopupCommand(wxCommandEvent & event);

		void RenderDiagram(wxDC& dc, int scale = 1, bool addHeader = false);

		void SaveCSV();

		DECLARE_EVENT_TABLE()
};

#endif
