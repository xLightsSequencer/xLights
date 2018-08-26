#ifndef SUBMODELDRAWGRID_H
#define SUBMODELDRAWGRID_H

//(*Headers(SubModelDrawGrid)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/sizer.h>
//*)

class Model;

class SubModelDrawGrid: public wxDialog
{
	public:

		SubModelDrawGrid(Model *m, std::vector<wxString> rows, wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~SubModelDrawGrid();

		//(*Declarations(SubModelDrawGrid)
		wxButton* ButtonDeselect;
		wxButton* ButtonSelectAll;
		wxButton* ButtonSelectNone;
		wxButton* ButtonSubDrawOK;
		wxButton* ButtonSunDrawCancel;
		wxButton* Button_Select;
		wxGrid* GridNodes;
		//*)

        std::vector<wxString> GetRowData();

	protected:

		//(*Identifiers(SubModelDrawGrid)
		static const long ID_BUTTON_SELECT;
		static const long ID_BUTTON_DESELECT;
		static const long ID_BUTTON_SELECT_ALL;
		static const long ID_BUTTON_SELECT_NONE;
		static const long ID_GRID_NODES;
		static const long ID_BUTTON_SUB_DRAW_OK;
		static const long ID_BUTTON_SUB_DRAW_CANCEL;
		//*)

	private:

		//(*Handlers(SubModelDrawGrid)
		void OnButton_SelectClick(wxCommandEvent& event);
		void OnButtonSelectAllClick(wxCommandEvent& event);
		void OnButtonSelectNoneClick(wxCommandEvent& event);
		void OnButtonDeselectClick(wxCommandEvent& event);
		void OnButtonSubDrawOKClick(wxCommandEvent& event);
		void OnButtonSubDrawCancelClick(wxCommandEvent& event);
		//*)

        void LoadGrid(std::vector<wxString> rows);
        void ValidateWindow();
        std::vector<int> DecodeNodeList(const std::vector<wxString> &rows);

        Model *model;

        wxColour selectColor;
        wxColour unselectColor;
        wxColour selectBackColor;
        wxColour unselectBackColor;

		DECLARE_EVENT_TABLE()
};

#endif
