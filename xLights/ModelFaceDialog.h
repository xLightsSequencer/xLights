#ifndef MODELFACEDIALOG_H
#define MODELFACEDIALOG_H

#include <wx/filename.h>

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
#include <list>
#include <string>

class Model;
class ModelPreview;
class FaceGrid;

class ModelFaceDialog: public wxDialog
{
    void PaintFace(wxDC& dc, int x, int y, const char* xpm[]);
    void DoSetPhonemes(wxFileName fn, std::string actualkey, std::string key, int count, int row, int col, std::string name, std::list<std::string> phonemes, std::string setPhoneme);
    void DoSetMatrixModels(wxFileName fn, std::string actualkey, std::string key, int count, int col, std::string name);
    void TryToSetAllMatrixModels(std::string name, std::string key, std::string new_filename, int row, int col);

    public:

		ModelFaceDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ModelFaceDialog();

		//(*Declarations(ModelFaceDialog)
		wxCheckBox* CustomColorNodeRanges;
		wxPanel* ModelPreviewPanelLocation;
		wxPanel* Panel_SingleNode;
		wxChoice* MatrixImagePlacementChoice;
		wxPanel* Panel_Matrix;
		wxPanel* Panel_NodeRanges;
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

        void SetFaceInfo(Model *cls, std::map<std::string, std::map<std::string, std::string> > &info);
        void GetFaceInfo(std::map<std::string, std::map<std::string, std::string> > &info);
	protected:

		//(*Identifiers(ModelFaceDialog)
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE3;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_PANEL4;
		static const long ID_CHECKBOX1;
		static const long ID_GRID_COROFACES;
		static const long ID_PANEL2;
		static const long ID_PANEL5;
		static const long ID_CHECKBOX2;
		static const long ID_GRID3;
		static const long ID_PANEL6;
		static const long ID_PANEL7;
		static const long ID_CHOICE2;
		static const long ID_GRID1;
		static const long ID_PANEL3;
		static const long ID_CHOICEBOOK1;
		static const long ID_PANEL1;
		//*)

	private:

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
		void OnMatrixModelsGridCellSelect(wxGridEvent& event);
		void OnMatrixModelsGridCellLeftClick1(wxGridEvent& event);
		void OnSingleNodeGridCellSelect(wxGridEvent& event);
		void OnSingleNodeGridCellLeftClick(wxGridEvent& event);
		void OnNodeRangeGridCellLeftClick(wxGridEvent& event);
		void OnNodeRangeGridCellSelect(wxGridEvent& event);
		void Paint(wxPaintEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
    
    std::map<std::string, std::map<std::string, std::string> > faceData;
    void SelectFaceModel(const std::string &s);
    ModelPreview *modelPreview;
    Model *model;
    void UpdatePreview(const std::string& channels, wxColor c);
    void GetValue(wxGrid *grid, wxGridEvent &event, std::map<std::string, std::string> &info);
};

#endif
