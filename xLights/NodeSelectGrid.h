#ifndef NodeSelectGrid_H
#define NodeSelectGrid_H

//(*Headers(NodeSelectGrid)
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/filepicker.h>
#include <wx/grid.h>
#include <wx/sizer.h>
#include <wx/slider.h>
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
	public:

		NodeSelectGrid(Model *m, const std::vector<wxString> &rows, wxWindow* parent,wxWindowID id=wxID_ANY);
        NodeSelectGrid(Model *m, const wxString &row, wxWindow* parent, wxWindowID id = wxID_ANY);

		virtual ~NodeSelectGrid();

		//(*Declarations(NodeSelectGrid)
		DrawGrid* GridNodes;
		ImageFilePickerCtrl* FilePickerCtrl1;
		wxBitmapButton* BitmapButton1;
		wxButton* ButtonDeselect;
		wxButton* ButtonLoadModel;
		wxButton* ButtonNodeSelectCancel;
		wxButton* ButtonNodeSelectOK;
		wxButton* ButtonSelectAll;
		wxButton* ButtonSelectNone;
		wxButton* ButtonZoomMinus;
		wxButton* ButtonZoomPlus;
		wxButton* Button_Select;
		wxCheckBox* CheckBoxFreeHand;
		wxSlider* SliderImgBrightness;
		//*)

        static const long NODESELECT_CUT;
        static const long NODESELECT_COPY;
        static const long NODESELECT_PASTE;

        std::vector<wxString> GetRowData();
        wxString GetNodeList();

	protected:

		//(*Identifiers(NodeSelectGrid)
		static const long ID_BUTTON_SELECT;
		static const long ID_BUTTON_DESELECT;
		static const long ID_BUTTON_SELECT_ALL;
		static const long ID_BUTTON_SELECT_NONE;
		static const long ID_CHECKBOX_FREE_HAND;
		static const long ID_BUTTON_LOAD_MODEL;
		static const long ID_BUTTON_ZOOM_PLUS;
		static const long ID_BUTTON_ZOOM_MINUS;
		static const long ID_FILEPICKERCTRL1;
		static const long ID_SLIDER_IMG_BRIGHTNESS;
		static const long ID_BITMAPBUTTON1;
		static const long ID_BUTTON_NODE_SELECT_OK;
		static const long ID_BUTTON_NODE_SELECT_CANCEL;
		static const long ID_GRID_NODES;
		//*)

        wxImage* bkg_image;
        wxModelGridCellRenderer* renderer;
        bool bkgrd_active;

	private:

		//(*Handlers(NodeSelectGrid)
		void OnButton_SelectClick(wxCommandEvent& event);
		void OnButtonSelectAllClick(wxCommandEvent& event);
		void OnButtonSelectNoneClick(wxCommandEvent& event);
		void OnButtonDeselectClick(wxCommandEvent& event);
		void OnButtonNodeSelectOKClick(wxCommandEvent& event);
		void OnButtonNodeSelectCancelClick(wxCommandEvent& event);
		void OnGridNodesCellLeftDClick(wxGridEvent& event);
		void OnCheckBoxFreeHandClick(wxCommandEvent& event);
		void OnGridNodesCellRightDClick(wxGridEvent& event);
		void OnButtonLoadModelClick(wxCommandEvent& event);
		void OnGridNodesCellRightClick(wxGridEvent& event);
		void OnSliderImgBrightnessCmdScroll(wxScrollEvent& event);
		void OnButtonZoomPlusClick(wxCommandEvent& event);
		void OnButtonZoomMinusClick(wxCommandEvent& event);
		void OnBitmapButton1Click(wxCommandEvent& event);
		void OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event);
		//*)

        void OnCut(wxCommandEvent& event);
        void OnCopy(wxCommandEvent& event);
        void OnPaste(wxCommandEvent& event);
        void OnGridPopup(wxCommandEvent& event);

        void CutOrCopyToClipboard(bool isCut);
        void UpdateBackground();
        void Paste();

        void OnDrawGridEvent(DrawGridEvent& event);
        void LoadGrid(const std::vector<wxString>& rows);
        void ValidateWindow() const;
        std::vector<int> DecodeNodeList(const std::vector<wxString> &rows) const;
        wxString EncodeNodeLine(const std::vector<wxString> &nodes) const;
        void ImportModel(const std::string &filename);
        void ImportModelXML(wxXmlNode* xmlData);

        Model *model;

        const wxColour selectColor{ wxColour("white") };
        const wxColour unselectColor{ wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT) };
        const wxColour selectBackColor{ wxColour("grey") };
        const wxColour unselectBackColor{ wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX) };

		DECLARE_EVENT_TABLE()
};
#endif