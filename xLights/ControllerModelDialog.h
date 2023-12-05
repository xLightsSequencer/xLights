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
#include <wx/slider.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
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
class SRCMObject;
class PortCMObject;

class ControllerModelPrintout : public wxPrintout
{
	wxPageSetupDialogData _page_setup;
	ControllerModelDialog* _controllerDialog;
	int _page_count;
	int _page_count_w;
	int _page_count_h;
	int _orient;
	wxPaperSize _paper_type;
	int _max_x, _max_y;
	wxSize _box_size;
	wxSize _panel_size;

public:
	ControllerModelPrintout(ControllerModelDialog* controllerDialog, const wxString& title, wxSize boxSize, wxSize panelSize);
	virtual bool OnPrintPage(int pageNum) override;
	virtual bool HasPage(int page) override;
	virtual void OnBeginPrinting() override;

	void preparePrint(const bool showPageSetupDialog = false);

	wxPrintData getPrintData() {
		return _page_setup.GetPrintData();
	}
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
	BaseCMObject* _dragging = nullptr;
	wxPoint _dragStartLocation = { -99999, -99999 };
	BaseCMObject* _popup = nullptr;
	bool _autoLayout = false;
	int _modelsy = 1;
	int _controllersy = 1;
	int _controllersx = 1;
	double _scale = 1;
	Model* _lastDropped = nullptr;
	#pragma endregion

	void DropModelFromModelsPaneOnModel(ModelCMObject* dropped, Model* on, bool rhs);
	BaseCMObject* GetControllerCMObjectAt(wxPoint mouse, wxPoint adjustedMouse);
	BaseCMObject* GetControllerToDropOn();
	BaseCMObject* GetModelsCMObjectAt(wxPoint mouse);
	PortCMObject* GetControllerPortAtLocation(wxPoint mouse);
    SRCMObject* GetControllerSRAtLocation();
    PortCMObject* GetPixelPort(int port) const;
	void ReloadModels();
	void ClearOver(wxPanel* panel, std::list<BaseCMObject*> list);
	std::string GetModelTooltip(ModelCMObject* m);
    std::string GetSRTooltip(SRCMObject* m);
    std::string GetPortTooltip(UDControllerPort* port, int virtualString);
	void FixDMXChannels();
	PortCMObject* GetPortContainingModel(Model* m);
	ModelCMObject* GetModelsCMObject(Model* m);

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
		wxSlider* Slider_Box_Scale;
		wxSlider* Slider_Font_Scale;
		wxSplitterWindow* SplitterWindow1;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl_Check;
		//*)

        static const long CONTROLLERModel_PRINT;
		static const long CONTROLLERModel_SAVE_CSV;
		static const long CONTROLLER_REMOVEALLMODELS;
		static const long CONTROLLER_DMXCHANNEL;
		static const long CONTROLLER_CASCADEDOWNPORT;
		static const long CONTROLLER_DMXCHANNELCHAIN;
		static const long CONTROLLER_PROTOCOL;
        static const long CONTROLLER_SETSMARTREMOTE;
		static const long CONTROLLER_REMOVEPORTMODELS;
		static const long CONTROLLER_MOVEMODELSTOPORT;
		static const long CONTROLLER_BRIGHTNESS;
		static const long CONTROLLER_BRIGHTNESSCLEAR;
        static const long CONTROLLER_SMARTREMOTETYPE;
        static const long CONTROLLER_REMOVESMARTREMOTE;
        static const long CONTROLLER_MODEL_STRINGS;
		static const long CONTROLLER_STARTNULLS;
        static const long CONTROLLER_ENDNULLS;
        static const long CONTROLLER_COLORORDER;
        static const long CONTROLLER_GROUPCOUNT;

		wxBitmap RenderPicture(int startY, int startX, int width, int height, wxString const& pageName);
		void DropFromModels(const wxPoint& location, const std::string& name, wxPanel* target);
		void DropFromController(const wxPoint& location, const std::string& name, wxPanel* target);
		bool IsDragging(BaseCMObject* dragging) const { return _dragging == dragging; }
		bool Scroll(wxPanel* panel, int scrollByX, int scrollByY);
		wxPoint GetScrollPosition(wxPanel* panel) const;
		void OnPopupCommand(wxCommandEvent& event);
        BaseCMObject* GetDragging() const
        {
        return _dragging;
        }

	protected:

		//(*Identifiers(ControllerModelDialog)
		static const long ID_PANEL1;
		static const long ID_SCROLLBAR1;
		static const long ID_SCROLLBAR2;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_BOX_SCALE;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER_FONT_SCALE;
		static const long ID_TEXTCTRL1;
		static const long ID_PANEL3;
		static const long ID_CHECKBOX1;
		static const long ID_PANEL2;
		static const long ID_SCROLLBAR3;
		static const long ID_PANEL4;
		static const long ID_SPLITTERWINDOW1;
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
		void OnSlider_ScaleCmdSliderUpdated(wxScrollEvent& event);
		//*)

		void ScrollToKey(int keyCode);
		void OnKeyDown(wxKeyEvent& event);
		void PrintScreen();
		void SaveCSV();
		double getFontSize();
		void EnsureSelectedModelIsVisible(ModelCMObject* cm);
        bool MaybeSetSmartRemote(wxKeyEvent& event);

		DECLARE_EVENT_TABLE()
};
