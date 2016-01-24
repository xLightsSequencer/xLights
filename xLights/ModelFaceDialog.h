#ifndef MODELFACEDIALOG_H
#define MODELFACEDIALOG_H

//(*Headers(ModelFaceDialog)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
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
		wxCheckBox* CustomColorNodeRanges;
		wxChoice* MatrixImagePlacementChoice;
		wxGrid* MatrixModelsGrid;
		wxGrid* SingleNodeGrid;
		wxStaticText* StaticText3;
		wxPanel* Matrix;
		wxCheckBox* CustomColorSingleNode;
		wxButton* DeleteButton;
		wxChoicebook* FaceTypeChoice;
		wxGrid* NodeRangeGrid;
		wxChoice* NameChoice;
		//*)

        void SetFaceInfo(ModelClass *cls, std::map<std::string, std::map<std::string, std::string> > &info);
        void GetFaceInfo(std::map<std::string, std::map<std::string, std::string> > &info);
	protected:

		//(*Identifiers(ModelFaceDialog)
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE3;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_CHECKBOX1;
		static const long ID_GRID_COROFACES;
		static const long ID_PANEL2;
		static const long ID_CHECKBOX2;
		static const long ID_GRID3;
		static const long ID_PANEL6;
		static const long ID_CHOICE2;
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
		void OnMatricImagePlacementChoiceSelect(wxCommandEvent& event);
		void OnCustomColorCheckboxClick(wxCommandEvent& event);
		void OnNodeRangeGridCellChange(wxGridEvent& event);
		void OnSingleNodeGridCellChange(wxGridEvent& event);
		void OnFaceTypeChoicePageChanged(wxChoicebookEvent& event);
		void OnNodeRangeGridCellLeftDClick(wxGridEvent& event);
		void OnSingleNodeGridCellLeftDClick(wxGridEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
    
    std::map<std::string, std::map<std::string, std::string> > faceData;
    void SelectFaceModel(const std::string &s);
};

#endif
