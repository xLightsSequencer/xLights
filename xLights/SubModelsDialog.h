#ifndef SUBMODELSDIALOG_H
#define SUBMODELSDIALOG_H

#include <map>
#include <vector>

//(*Headers(SubModelsDialog)
#include <wx/dialog.h>
class wxPanel;
class wxGrid;
class wxStdDialogButtonSizer;
class wxCheckBox;
class wxTextCtrl;
class wxNotebookEvent;
class wxNotebook;
class wxStaticText;
class wxListCtrl;
class wxFlexGridSizer;
class wxButton;
class wxGridEvent;
//*)

class Model;
class wxBookCtrlEvent;
class ModelPreview;
class SubBufferPanel;

class SubModelsDialog: public wxDialog
{
    public:

		SubModelsDialog(wxWindow* parent);
		virtual ~SubModelsDialog();
    
        void Setup(Model *m);
        void Save();

		//(*Declarations(SubModelsDialog)
		wxCheckBox* LayoutCheckbox;
		wxPanel* ModelPreviewPanelLocation;
		wxFlexGridSizer* SubBufferSizer;
		wxListCtrl* ListCtrl1;
		wxButton* AddRowButton;
		wxPanel* SubBufferPanelHolder;
		wxFlexGridSizer* PreviewSizer;
		wxStaticText* StaticText3;
		wxButton* Button_Generate;
		wxGrid* NodesGrid;
		wxTextCtrl* TextCtrl1;
		wxButton* AddButton;
		wxNotebook* TypeNotebook;
		wxButton* DeleteButton;
		wxButton* Button_ReverseNodes;
		wxButton* DeleteRowButton;
		//*)

	protected:

		//(*Identifiers(SubModelsDialog)
		static const long ID_LISTCTRL1;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON5;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON6;
		static const long ID_GRID1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_PANEL2;
		static const long ID_PANEL3;
		static const long ID_NOTEBOOK1;
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
    
    Model *model;
    ModelPreview *modelPreview;
    SubBufferPanel *subBufferPanel;
    std::vector<SubModelInfo> subModels;
    
};

#endif
