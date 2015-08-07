#ifndef MODELFACEDIALOG_H
#define MODELFACEDIALOG_H

//(*Headers(ModelFaceDialog)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/grid.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/choicebk.h>
//*)

#include <map>

class ModelClass;

class ModelFaceDialog: public wxDialog
{
	public:

		ModelFaceDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ModelFaceDialog();

		//(*Declarations(ModelFaceDialog)
		wxGrid* MatrixModelsGrid;
		wxGrid* SingleNodeGrid;
		wxStaticText* StaticText1;
		wxButton* MatrixDeleteButton;
		wxChoice* MatrixNameChoice;
		wxPanel* Matrix;
		wxChoicebook* FaceTypeChoice;
		wxGrid* NodeRangeGrid;
		//*)

        void SetFaceInfo(ModelClass *cls, std::map<wxString, std::map<wxString, wxString> > &info);
        void GetFaceInfo(std::map<wxString, std::map<wxString, wxString> > &info);
	protected:

		//(*Identifiers(ModelFaceDialog)
		static const long ID_PANEL1;
		static const long ID_GRID_COROFACES;
		static const long ID_PANEL2;
		static const long ID_GRID3;
		static const long ID_PANEL6;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_GRID1;
		static const long ID_PANEL3;
		static const long ID_CHOICEBOOK1;
		//*)

	private:
        ModelClass *modelClass;

		//(*Handlers(ModelFaceDialog)
		void OnMatrixNameChoiceSelect(wxCommandEvent& event);
		void OnButtonMatrixAddClicked(wxCommandEvent& event);
		void OnButtonMatrixDeleteClick(wxCommandEvent& event);
		void OnMatrixModelsGridCellLeftClick(wxGridEvent& event);
		void OnMatrixModelsGridCellChange(wxGridEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
    
    std::map<wxString, std::map<wxString, wxString> > matrixData;
    void FillMatrix(const wxString &s);
};

#endif
