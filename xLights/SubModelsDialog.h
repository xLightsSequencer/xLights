#ifndef SUBMODELSDIALOG_H
#define SUBMODELSDIALOG_H

#include <map>
#include <vector>
#include <wx/listctrl.h>

//(*Headers(SubModelsDialog)
#include <wx/dialog.h>
class wxGridEvent;
class wxTextCtrl;
class wxNotebook;
class wxFlexGridSizer;
class wxListCtrl;
class wxButton;
class wxStdDialogButtonSizer;
class wxSplitterWindow;
class wxSplitterEvent;
class wxGrid;
class wxNotebookEvent;
class wxStaticText;
class wxPanel;
class wxCheckBox;
//*)

class Model;
class wxBookCtrlEvent;
class ModelPreview;
class SubBufferPanel;

class SubModelsDialog: public wxDialog
{
    int _sortOrder = 0;
    int _numSubModels = 0;

    public:

		SubModelsDialog(wxWindow* parent);
		virtual ~SubModelsDialog();

        void Setup(Model *m);
        void Save();

		//(*Declarations(SubModelsDialog)
		wxButton* DeleteRowButton;
		wxButton* Button_ReverseNodes;
		wxFlexGridSizer* PreviewSizer;
		wxGrid* NodesGrid;
		wxButton* AddButton;
		wxButton* Button_MoveDown;
		wxButton* AddRowButton;
		wxFlexGridSizer* SubBufferSizer;
		wxStaticText* StaticText1;
		wxButton* Button_Generate;
		wxPanel* Panel2;
		wxButton* DeleteButton;
		wxSplitterWindow* SplitterWindow1;
		wxStaticText* StaticTextName;
		wxTextCtrl* TextCtrl_Name;
		wxPanel* Panel3;
		wxPanel* ModelPreviewPanelLocation;
		wxPanel* SubBufferPanelHolder;
		wxNotebook* TypeNotebook;
		wxCheckBox* LayoutCheckbox;
		wxButton* Button_MoveUp;
		wxListCtrl* ListCtrl_SubModels;
		//*)

	protected:

		//(*Identifiers(SubModelsDialog)
		static const long ID_STATICTEXT1;
		static const long ID_LISTCTRL_SUB_MODELS;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON5;
		static const long ID_PANEL4;
		static const long ID_STATICTEXT_NAME;
		static const long ID_TEXTCTRL_NAME;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON6;
		static const long ID_GRID1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON_MOVE_UP;
		static const long ID_BUTTON_MOVE_DOWN;
		static const long ID_PANEL2;
		static const long ID_PANEL3;
		static const long ID_NOTEBOOK1;
		static const long ID_PANEL5;
		static const long ID_SPLITTERWINDOW1;
		static const long ID_PANEL1;
		//*)

	public:

		//(*Handlers(SubModelsDialog)
		void OnAddButtonClick(wxCommandEvent& event);
		void OnDeleteButtonClick(wxCommandEvent& event);
		void OnNameChoiceSelect(wxCommandEvent& event);
		void OnNodesGridCellChange(wxGridEvent& event);
		void OnNodesGridCellSelect(wxGridEvent& event);
		void OnLayoutCheckboxClick(wxCommandEvent& event);
		void OnAddRowButtonClick(wxCommandEvent& event);
		void OnDeleteRowButtonClick(wxCommandEvent& event);
		void OnSubBufferRangeChange(wxCommandEvent& event);
		void OnNodesGridCellLeftClick(wxGridEvent& event);
		void OnTypeNotebookPageChanged(wxBookCtrlEvent& event);
		void OnNodesGridLabelLeftClick(wxGridEvent& event);
		void OnButton_GenerateClick(wxCommandEvent& event);
		void OnButton_ReverseNodesClick(wxCommandEvent& event);
		void OnListCtrl_SubModelsItemSelect(wxListEvent& event);
		void OnListCtrl_SubModelsBeginDrag(wxListEvent& event);
		void OnListCtrl_SubModelsColumnClick(wxListEvent& event);
		void OnListCtrl_SubModelsKeyDown(wxListEvent& event);
		//*)

		DECLARE_EVENT_TABLE()


private:
    void SelectRow(int r);
    void Select(const wxString &name);
    void DisplayRange(const wxString &range);

    class SubModelInfo {
    public:
        SubModelInfo() {}
        SubModelInfo(const wxString &n) : name(n) {}

        wxString name;
        bool vertical;
        bool isRanges;
        wxString subBuffer;
        std::vector<wxString> strands;
    };

    void GenerateSegment(SubModelInfo& sm, int segments, int segment, bool horizontal, int count);
    SubModelInfo &GetSubModelInfo(const wxString &str);
    int GetSubModelInfoIndex(const wxString &str);
	void AddSubModelToList(SubModelInfo *submodel);
	wxString GetSelectedName();
	bool IsItemSelected(wxListCtrl* ctrl, int item);

    Model *model;
    ModelPreview *modelPreview;
    SubBufferPanel *subBufferPanel;
    std::vector<SubModelInfo> subModels;

};

#endif
