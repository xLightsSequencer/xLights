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

//(*Headers(ControllerModelDialog)
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/panel.h>
#include <wx/scrolbar.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
//*)
#include <wx/colour.h>
#include <wx/prntbase.h>

#include "controllers/ControllerUploadData.h"

class ControllerModelDialog;
class Output;
class BaseCMObject;
class xLightsFrame;
class ModelCMObject;
class PortCMObject;

class ControllerModelPrintout : public wxPrintout
{
    ControllerModelDialog* _controllerDialog;
public:
    ControllerModelPrintout(ControllerModelDialog* controllerDialog);
    virtual bool OnPrintPage(int pageNum) override;
};

class ControllerModelDialog: public wxDialog
{
	#pragma region Member Variables
	std::string _title;
    UDController* _cud = nullptr;
	Controller* _controller = nullptr;
	ModelManager* _mm = nullptr;
	xLightsFrame* _xLights = nullptr;
	ControllerCaps* _caps = nullptr;
	std::list<BaseCMObject*> _models;
	std::list<BaseCMObject*> _controllers;
	ModelCMObject* _dragging = nullptr;
	BaseCMObject* _popup = nullptr;
	bool _autoLayout = false;
	int _modelsy = 1;
	int _controllersy = 1;
	int _controllersx = 1;
	#pragma endregion

	BaseCMObject* GetControllerCMObjectAt(wxPoint mouse);
	BaseCMObject* GetModelsCMObjectAt(wxPoint mouse);
	PortCMObject* GetControllerPortAtLocation(wxPoint mouse);
	void ReloadModels();
	void ClearOver(wxPanel* panel, std::list<BaseCMObject*> list);
	std::string GetModelTooltip(ModelCMObject* m);
	std::string GetPortTooltip(UDControllerPort* port, int virtualString);
	void FixDMXChannels();

	public:

		ControllerModelDialog(wxWindow* parent, UDController* cud, ModelManager* mm, Controller* controller, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~ControllerModelDialog();

		//(*Declarations(ControllerModelDialog)
		wxCheckBox* CheckBox_HideOtherControllerModels;
		wxPanel* Panel3;
		wxPanel* Panel4;
		wxPanel* PanelController;
		wxPanel* PanelModels;
		wxScrollBar* ScrollBar_Controller_H;
		wxScrollBar* ScrollBar_Controller_V;
		wxScrollBar* ScrollBar_Models;
		wxTextCtrl* TextCtrl_Check;
		//*)

        static const long CONTROLLERModel_PRINT;
		static const long CONTROLLERModel_SAVE_CSV;
		static const long CONTROLLER_SMARTREMOTE_None;
		static const long CONTROLLER_SMARTREMOTE_A;
		static const long CONTROLLER_SMARTREMOTE_B;
		static const long CONTROLLER_SMARTREMOTE_C;
		static const long CONTROLLER_SMARTREMOTE_ABC;
		static const long CONTROLLER_SMARTREMOTE_BC;
		static const long CONTROLLER_DMXCHANNEL;
		static const long CONTROLLER_PROTOCOL;
		static const long CONTROLLER_BRIGHTNESS;
		static const long CONTROLLER_BRIGHTNESSCLEAR;

        void RenderPicture(wxBitmap& bitmap, bool printer);
		void DropFromModels(const wxPoint& location, const std::string& name, wxPanel* target);
		void DropFromController(const wxPoint& location, const std::string& name, wxPanel* target);
		bool IsDragging(ModelCMObject* dragging) const { return _dragging == dragging; }
		void Draw(wxPanel* panel, BaseCMObject* object, wxPoint mouse);
		bool Scroll(wxPanel* panel, int scrollByX, int scrollByY);
		wxPoint GetScrollPosition(wxPanel* panel) const;

	protected:

		//(*Identifiers(ControllerModelDialog)
		static const long ID_PANEL1;
		static const long ID_SCROLLBAR1;
		static const long ID_SCROLLBAR2;
		static const long ID_TEXTCTRL1;
		static const long ID_PANEL3;
		static const long ID_CHECKBOX1;
		static const long ID_PANEL2;
		static const long ID_SCROLLBAR3;
		static const long ID_PANEL4;
		//*)

	private:

		//(*Handlers(ControllerModelDialog)
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
		void OnCheckBox_HideOtherControllerModelsClick(wxCommandEvent& event);
		//*)

		void ScrollToKey(int keyCode);
		void OnKeyDown(wxKeyEvent& event);
		void OnPopupCommand(wxCommandEvent & event);
		void SaveCSV();

		DECLARE_EVENT_TABLE()
};
