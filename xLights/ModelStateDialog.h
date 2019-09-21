#ifndef MODELSTATEDIALOG_H
#define MODELSTATEDIALOG_H

//(*Headers(ModelStateDialog)
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

#include <wx/colourdata.h>

#include <map>

class Model;
class ModelPreview;
class xLightsFrame;
class ModelManager;

class ModelStateDialog: public wxDialog
{
    static wxColourData _colorData;

    public:

		ModelStateDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ModelStateDialog();

		//(*Declarations(ModelFaceDialog)
		wxCheckBox* CustomColorNodeRanges;
		wxGrid* SingleNodeGrid;
		wxStaticText* StaticText3;
		wxPanel* Matrix;
		wxCheckBox* CustomColorSingleNode;
		wxButton* DeleteButton;
        wxButton* Button_Import;
		wxChoicebook* StateTypeChoice;
		wxGrid* NodeRangeGrid;
		wxChoice* NameChoice;
        wxPanel* ModelPreviewPanelLocation;
        wxButton* Button_7Segment;
        wxButton* Button_7Seg;
        //*)

        void SetStateInfo(Model *cls, std::map<std::string, std::map<std::string, std::string> > &info);
        void GetStateInfo(std::map<std::string, std::map<std::string, std::string> > &info);
	protected:

		//(*Identifiers(ModelStateDialog)
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE3;
		static const long ID_BUTTON3;
		static const long ID_BUTTON_IMPORT;
		static const long ID_BUTTON4;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON1;
		static const long ID_GRID_COROSTATES;
		static const long ID_PANEL2;
		static const long ID_CHECKBOX2;
		static const long ID_BUTTON2;
		static const long ID_GRID3;
		static const long ID_PANEL6;
		static const long ID_CHOICEBOOK1;
		static const long ID_PANEL_PREVIEW;
		//*)

        static const long STATE_DIALOG_IMPORT_SUB;
        static const long STATE_DIALOG_IMPORT_MODEL;
        static const long STATE_DIALOG_IMPORT_FILE;

	private:

		//(*Handlers(ModelStateDialog)
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
		void OnStateTypeChoicePageChanged(wxChoicebookEvent& event);
		void OnNodeRangeGridCellLeftClick(wxGridEvent& event);
		void OnSingleNodeGridCellLeftClick(wxGridEvent& event);
		void OnSingleNodeGridCellSelect(wxGridEvent& event);
		void OnNodeRangeGridCellSelect(wxGridEvent& event);
		void OnButton_7SegmentClick(wxCommandEvent& event);
		void OnNodeRangeGridCellRightClick(wxGridEvent& event);
		void OnNodeRangeGridLabelLeftDClick(wxGridEvent& event);
		void OnButton_ImportClick(wxCommandEvent& event);
		//*)

        void OnAddBtnPopup(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()

    std::map<std::string, std::map<std::string, std::string> > stateData;
    void SelectStateModel(const std::string &s);
    ModelPreview *modelPreview;
    Model *model;
    void UpdatePreview(const std::string& channels, wxColor c);
    void GetValue(wxGrid *grid, const int row, const int col, std::map<std::string, std::string> &info);
    void AddLabel(wxString label);
    void ValidateWindow();

    void OnGridPopup(const int rightEventID, wxGridEvent& gridEvent);
    void ImportSubmodel(wxGridEvent& event);
    wxString getSubmodelNodes(Model* sm);
    void ImportStates(const wxString& filename);
    void ImportStatesFromModel();
    void AddStates(std::map<std::string, std::map<std::string, std::string> > states);
    wxArrayString getModelList(ModelManager* modelManager);
};

#endif
