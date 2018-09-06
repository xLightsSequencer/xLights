#ifndef NodeSelectGrid_H
#define NodeSelectGrid_H

//(*Headers(NodeSelectGrid)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/sizer.h>
//*)

class Model;
class DrawGrid;
class DrawGridEvent;

class NodeSelectGrid: public wxDialog
{
	public:

		NodeSelectGrid(Model *m, std::vector<wxString> rows, wxWindow* parent,wxWindowID id=wxID_ANY);
        NodeSelectGrid(Model *m, wxString row, wxWindow* parent, wxWindowID id = wxID_ANY);

		virtual ~NodeSelectGrid();

		//(*Declarations(NodeSelectGrid)
		DrawGrid* GridNodes;
		wxButton* ButtonDeselect;
		wxButton* ButtonNodeSelectCancel;
		wxButton* ButtonNodeSelectOK;
		wxButton* ButtonSelectAll;
		wxButton* ButtonSelectNone;
		wxButton* Button_Select;
		wxCheckBox* CheckBoxFreeHand;
		//*)

        std::vector<wxString> GetRowData();
        wxString GetNodeList();

	protected:

		//(*Identifiers(NodeSelectGrid)
		static const long ID_CHECKBOX_FREE_HAND;
		static const long ID_BUTTON_SELECT;
		static const long ID_BUTTON_DESELECT;
		static const long ID_BUTTON_SELECT_ALL;
		static const long ID_BUTTON_SELECT_NONE;
		static const long ID_GRID_NODES;
		static const long ID_BUTTON_NODE_SELECT_OK;
		static const long ID_BUTTON_NODE_SELECT_CANCEL;
		//*)

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
		//*)

        void OnDrawGridEvent(DrawGridEvent& event);
        void LoadGrid(const std::vector<wxString> rows);
        void ValidateWindow() const;
        std::vector<int> DecodeNodeList(const std::vector<wxString> &rows) const;
        wxString EncodeNodeLine(const std::vector<wxString> &nodes) const;

        Model *model;

        wxColour selectColor;
        wxColour unselectColor;
        wxColour selectBackColor;
        wxColour unselectBackColor;

		DECLARE_EVENT_TABLE()
};
#endif