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

#include <wx/filename.h>

//(*Headers(ModelFaceDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/choicebk.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

#include <map>
#include <list>
#include <string>

class Model;
class ModelPreview;
class FaceGrid;
class xLightsFrame;
class ModelManager;

class ModelFaceDialog: public wxDialog
{
    const std::list<std::string> _phonemes = { "AI", "E", "etc", "FV", "L", "MBP", "O", "rest", "U", "WQ" };
    static wxColourData _colorData;

    void PaintFace(wxDC& dc, int x, int y, const char* xpm[]);
    void DoSetPhonemes(wxFileName fn, std::string actualkey, std::string key, int count, int row, int col, std::string name, std::list<std::string> phonemes, std::string setPhoneme);
    void DoSetMatrixModels(wxFileName fn, std::string actualkey, std::string key, int count, int col, std::string name);
    void TryToSetAllMatrixModels(std::string name, std::string key, std::string new_filename, int row, int col);
    bool IsValidPhoneme(const std::string phoneme) const;
    int GetRowForPhoneme(const std::string phoneme) const;

    public:

		ModelFaceDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ModelFaceDialog();

		//(*Declarations(ModelFaceDialog)
		wxButton* ButtonImport;
		wxButton* Button_DownloadImages;
		wxButton* DeleteButton;
		wxCheckBox* CustomColorNodeRanges;
		wxCheckBox* CustomColorSingleNode;
		wxChoice* MatrixImagePlacementChoice;
		wxChoice* NameChoice;
		wxChoicebook* FaceTypeChoice;
		wxGrid* MatrixModelsGrid;
		wxGrid* NodeRangeGrid;
		wxGrid* SingleNodeGrid;
		wxPanel* Matrix;
		wxPanel* ModelPreviewPanelLocation;
		wxPanel* Panel_Matrix;
		wxPanel* Panel_NodeRanges;
		wxPanel* Panel_SingleNode;
		wxStaticText* StaticText3;
		//*)

        static const long FACES_DIALOG_IMPORT_SUB;
        static const long FACES_DIALOG_IMPORT_MODEL;
        static const long FACES_DIALOG_IMPORT_FILE;
        static const long FACES_DIALOG_COPY;
        static const long FACES_DIALOG_RENAME;

        void SetFaceInfo(Model *cls, std::map<std::string, std::map<std::string, std::string> > &info);
        void GetFaceInfo(std::map<std::string, std::map<std::string, std::string> > &info);
	protected:

		//(*Identifiers(ModelFaceDialog)
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE3;
		static const long ID_BUTTON3;
		static const long ID_BUTTON_IMPORT;
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
		static const long ID_BUTTON1;
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
		void OnButton_DownloadImagesClick(wxCommandEvent& event);
		void OnNodeRangeGridCellRightClick(wxGridEvent& event);
		void OnNodeRangeGridLabelLeftDClick(wxGridEvent& event);
		void OnButtonImportClick(wxCommandEvent& event);
		//*)

        void OnAddBtnPopup(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()

    std::map<std::string, std::map<std::string, std::string> > faceData;
    void SelectFaceModel(const std::string &s);
    ModelPreview *modelPreview;
    Model *model;
    void UpdatePreview(const std::string& channels, wxColor c);
    void GetValue(wxGrid *grid, const int row, const int col, std::map<std::string, std::string> &info);
    void OnGridPopup(const int rightEventID, wxGridEvent& gridEvent);
    void ImportSubmodel(wxGridEvent& event);
    wxString getSubmodelNodes(Model* sm);
    void ImportFaces(const wxString& filename);
    void ImportFacesFromModel();
    void AddFaces(std::map<std::string, std::map<std::string, std::string> > faces);
    wxArrayString getModelList(ModelManager* modelManager);
    void CopyFaceData();
    void RenameFace();
};

