#ifndef SUBMODELDRAWGRID_H
#define SUBMODELDRAWGRID_H

//(*Headers(SubModelDrawGrid)
#include <wx/sizer.h>
#include <wx/grid.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class Model;

class SubModelDrawGrid: public wxDialog
{
	public:

		SubModelDrawGrid(Model *m, wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~SubModelDrawGrid();

		//(*Declarations(SubModelDrawGrid)
		wxButton* ButtonSelectAll;
		wxButton* ButtonSelectNone;
		wxButton* ButtonSunDrawCancel;
		wxButton* Button_Select;
		wxButton* ButtonDeselect;
		wxButton* ButtonSubDrawOK;
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

        void LoadGrid();
        void ValidateWindow();

        Model *model;

        wxColour selectColor;
        wxColour unselectColor;
        wxColour selectBackColor;
        wxColour unselectBackColor;

		DECLARE_EVENT_TABLE()
};

#endif
