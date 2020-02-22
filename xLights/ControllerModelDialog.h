#pragma once

//(*Headers(ControllerModelDialog)
#include <wx/dialog.h>
#include <wx/panel.h>
#include <wx/scrolbar.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
//*)

#include "controllers/ControllerUploadData.h"
#include <wx/prntbase.h>

class ControllerModelDialog;
class Output;
class BaseCMObject;

class ControllerModelPrintout : public wxPrintout
{
    ControllerModelDialog* _controllerDialog;
public:
    ControllerModelPrintout(ControllerModelDialog* controllerDialog);
    virtual bool OnPrintPage(int pageNum) override;
};

class ControllerModelDialog: public wxDialog
{
    UDController& _cud;
    std::string _ip;
    std::string _description;
	std::list<BaseCMObject*> _models;
	std::list<BaseCMObject*> _controllers;

	BaseCMObject* GetControllerCMObjectAt(wxPoint mouse);
	BaseCMObject* GetModelsCMObjectAt(wxPoint mouse);

	public:

		ControllerModelDialog(wxWindow* parent, UDController& cud, const std::string& ip, const std::string& description, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~ControllerModelDialog();

		//(*Declarations(ControllerModelDialog)
		wxPanel* Panel3;
		wxPanel* Panel4;
		wxPanel* PanelController;
		wxPanel* PanelModels;
		wxScrollBar* ScrollBar_Controller_H;
		wxScrollBar* ScrollBar_Controller_V;
		wxScrollBar* ScrollBar_Models;
		wxSplitterWindow* SplitterWindow1;
		//*)

        static const long CONTROLLERModel_PRINT;
        static const long CONTROLLERModel_SAVE_CSV;

        void RenderPicture(wxBitmap& bitmap, bool printer);

	protected:

		//(*Identifiers(ControllerModelDialog)
		static const long ID_PANEL1;
		static const long ID_SCROLLBAR1;
		static const long ID_SCROLLBAR2;
		static const long ID_PANEL3;
		static const long ID_PANEL2;
		static const long ID_SCROLLBAR3;
		static const long ID_PANEL4;
		static const long ID_SPLITTERWINDOW1;
		//*)

	private:

		//(*Handlers(ControllerModelDialog)
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
		void OnPanelControllerLeftDown(wxMouseEvent& event);
		void OnPanelControllerKeyDown(wxKeyEvent& event);
		void OnPanelControllerLeftUp(wxMouseEvent& event);
		void OnPanelControllerLeftDClick(wxMouseEvent& event);
		void OnPanelControllerMouseMove(wxMouseEvent& event);
		void OnPanelControllerMouseEnter(wxMouseEvent& event);
		void OnPanelControllerMouseLeave(wxMouseEvent& event);
		void OnPanelControllerRightDown(wxMouseEvent& event);
		void OnPanelControllerPaint(wxPaintEvent& event);
		void OnPanelControllerResize(wxSizeEvent& event);
		void OnScrollBar_Controller_HScroll(wxScrollEvent& event);
		void OnScrollBar_Controller_HScrollThumbTrack(wxScrollEvent& event);
		void OnScrollBar_Controller_HScrollChanged(wxScrollEvent& event);
		void OnScrollBar_Controller_VScroll(wxScrollEvent& event);
		void OnScrollBar_Controller_VScrollThumbTrack(wxScrollEvent& event);
		void OnScrollBar_Controller_VScrollChanged(wxScrollEvent& event);
		void OnScrollBar_ModelsScroll(wxScrollEvent& event);
		void OnScrollBar_ModelsScrollThumbTrack(wxScrollEvent& event);
		void OnScrollBar_ModelsScrollChanged(wxScrollEvent& event);
		void OnPanelModelsPaint(wxPaintEvent& event);
		void OnPanelModelsKeyDown(wxKeyEvent& event);
		void OnPanelModelsLeftDown(wxMouseEvent& event);
		void OnPanelModelsLeftUp(wxMouseEvent& event);
		void OnPanelModelsLeftDClick(wxMouseEvent& event);
		void OnPanelModelsRightDown(wxMouseEvent& event);
		void OnPanelModelsMouseMove(wxMouseEvent& event);
		void OnPanelModelsMouseWheel(wxMouseEvent& event);
		void OnPanelModelsResize(wxSizeEvent& event);
		void OnPanelModelsMouseEnter(wxMouseEvent& event);
		void OnPanelModelsMouseLeave(wxMouseEvent& event);
		void OnPanelControllerMouseWheel(wxMouseEvent& event);
		//*)

        void OnPopupCommand(wxCommandEvent & event);

		void RenderDiagram(wxDC& dc, int scale = 1, bool addHeader = false);

		void SaveCSV();

		DECLARE_EVENT_TABLE()
};
