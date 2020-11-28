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

//(*Headers(NodeSelectGrid)
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/filepicker.h>
#include <wx/grid.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/textctrl.h>
//*)

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/renderer.h>
#include <wx/filepicker.h>
#include "wxModelGridCellRenderer.h"
#include "UtilClasses.h"

class Model;
class DrawGrid;
class DrawGridEvent;
class wxModelGridCellRenderer;

class NodeSelectGrid: public wxDialog
{
	void SetGridSizeForFont(const wxFont& font);

	public:

		NodeSelectGrid(bool multiline, const wxString &title, Model *m, const std::vector<wxString> &rows, wxWindow* parent,wxWindowID id=wxID_ANY);
        NodeSelectGrid(bool multiline, const wxString &title, Model *m, const wxString &row, wxWindow* parent, wxWindowID id = wxID_ANY);

		virtual ~NodeSelectGrid();

		//(*Declarations(NodeSelectGrid)
		DrawGrid* GridNodes;
		ImageFilePickerCtrl* FilePickerCtrl1;
		wxBitmapButton* BitmapButton1;
		wxButton* ButtonLoadModel;
		wxButton* ButtonNodeSelectCancel;
		wxButton* ButtonNodeSelectOK;
		wxButton* ButtonSelectAll;
		wxButton* ButtonSelectNone;
		wxButton* ButtonZoomMinus;
		wxButton* ButtonZoomPlus;
		wxCheckBox* CheckBox_OrderedSelection;
		wxSlider* SliderImgBrightness;
		wxTextCtrl* TextCtrl_Nodes;
		//*)

        static const long NODESELECT_CUT;
        static const long NODESELECT_COPY;
		static const long NODESELECT_PASTE;
		static const long NODESELECT_FIND;

        std::vector<wxString> GetRowData();
        wxString GetNodeList(const bool sort = true);

	protected:

		//(*Identifiers(NodeSelectGrid)
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON_SELECT_ALL;
		static const long ID_BUTTON_SELECT_NONE;
		static const long ID_BUTTON_LOAD_MODEL;
		static const long ID_BUTTON_ZOOM_PLUS;
		static const long ID_BUTTON_ZOOM_MINUS;
		static const long ID_FILEPICKERCTRL1;
		static const long ID_SLIDER_IMG_BRIGHTNESS;
		static const long ID_BITMAPBUTTON1;
		static const long ID_BUTTON_NODE_SELECT_OK;
		static const long ID_BUTTON_NODE_SELECT_CANCEL;
		static const long ID_GRID_NODES;
		static const long ID_TEXTCTRL1;
		//*)

        wxImage* bkg_image;
        wxModelGridCellRenderer* renderer;
        bool bkgrd_active;

	private:

		//(*Handlers(NodeSelectGrid)
		void OnButtonSelectAllClick(wxCommandEvent& event);
		void OnButtonSelectNoneClick(wxCommandEvent& event);
		void OnButtonNodeSelectOKClick(wxCommandEvent& event);
		void OnButtonNodeSelectCancelClick(wxCommandEvent& event);
		void OnGridNodesCellLeftDClick(wxGridEvent& event);
		void OnGridNodesCellRightDClick(wxGridEvent& event);
		void OnButtonLoadModelClick(wxCommandEvent& event);
		void OnGridNodesCellRightClick(wxGridEvent& event);
		void OnSliderImgBrightnessCmdScroll(wxScrollEvent& event);
		void OnButtonZoomPlusClick(wxCommandEvent& event);
		void OnButtonZoomMinusClick(wxCommandEvent& event);
		void OnBitmapButton1Click(wxCommandEvent& event);
		void OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event);
		void OnGridNodesCellSelect(wxGridRangeSelectEvent& event);
		void OnCheckBox_OrderedSelectionClick(wxCommandEvent& event);
		void OnTextCtrl_NodesText(wxCommandEvent& event);
		//*)

        void OnCut(wxCommandEvent& event);
        void OnCopy(wxCommandEvent& event);
        void OnPaste(wxCommandEvent& event);
        void OnGridPopup(wxCommandEvent& event);

        void CutOrCopyToClipboard(bool isCut);
        void UpdateBackground();
        void Paste();

        void LoadGrid(const std::vector<wxString>& rows);
        void ValidateWindow() const;
        std::vector<int> DecodeNodeList(const std::vector<wxString> &rows) const;
        wxString EncodeNodeLine(const std::vector<wxString> &nodes, const bool sort) const;
        void ImportModel(const std::string &filename);
        void ImportModelXML(wxXmlNode* xmlData);
		void UpdateSelectedFromText();
		void UpdateTextFromGrid();
		void AddNode(int col, int row);
		void RemoveNode(int col, int row);
		wxString ExpandNodes(const wxString& nodes) const;
		wxString CompressNodes(const wxString& nodes) const;
		void OnTextCtrl_NodesLoseFocus(wxFocusEvent& event);
		void OnKeyDown(wxKeyEvent& event);

		void SaveSettings();
		void GetMinMaxNode(long& min, long& max);
		void Find();

        Model *model;

        wxColour selectColor;
        wxColour unselectColor;
        wxColour selectBackColor;
        wxColour unselectBackColor;

		DECLARE_EVENT_TABLE()
};
