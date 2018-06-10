#ifndef SUBMODELSDIALOG_H
#define SUBMODELSDIALOG_H

#include <map>
#include <vector>
#include <wx/dnd.h>
#include <wx/listctrl.h>
#include <wx/xml/xml.h>

//(*Headers(SubModelsDialog)
#include <wx/dialog.h>
class wxButton;
class wxCheckBox;
class wxFlexGridSizer;
class wxGrid;
class wxGridEvent;
class wxListCtrl;
class wxNotebook;
class wxNotebookEvent;
class wxPanel;
class wxSplitterEvent;
class wxSplitterWindow;
class wxStaticText;
class wxStdDialogButtonSizer;
class wxTextCtrl;
//*)

class Model;
class wxBookCtrlEvent;
class ModelPreview;
class SubBufferPanel;
class xLightsFrame;
class LayoutPanel;

wxDECLARE_EVENT(EVT_SMDROP, wxCommandEvent);

class SubModelTextDropTarget : public wxTextDropTarget
{
	public:
		SubModelTextDropTarget(wxWindow* owner, wxListCtrl* list, wxString type)
		{
			_owner = owner;
			_list = list;
			_type = type;
		};

		virtual bool OnDropText(wxCoord x, wxCoord y, const wxString &data) override;
		virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override;

		wxWindow* _owner;
		wxListCtrl* _list;
		wxString _type;
};

class SubModelsDialog : public wxDialog
{
    class SubModelInfo {
    public:
        SubModelInfo() {}
        SubModelInfo(const wxString &n) : name(n), oldName(n) {}

        wxString name;
        wxString oldName;
        bool vertical;
        bool isRanges;
        wxString subBuffer;
        std::vector<wxString> strands;
    };

    Model *model;
    ModelPreview *modelPreview;
    SubBufferPanel *subBufferPanel;

public:
    std::vector<SubModelInfo*> _subModels;

    SubModelsDialog(wxWindow* parent);
    virtual ~SubModelsDialog();

    void Setup(Model *m);
    void Save();

    //(*Declarations(SubModelsDialog)
    wxButton* AddButton;
    wxButton* AddRowButton;
    wxButton* Button_Generate;
    wxButton* Button_MoveDown;
    wxButton* Button_MoveUp;
    wxButton* Button_ReverseNodes;
    wxButton* Button_ReverseRow;
    wxButton* Button_ReverseRows;
    wxButton* Button_Sub_Import;
    wxButton* DeleteButton;
    wxButton* DeleteRowButton;
    wxCheckBox* LayoutCheckbox;
    wxFlexGridSizer* PreviewSizer;
    wxFlexGridSizer* SubBufferSizer;
    wxGrid* NodesGrid;
    wxListCtrl* ListCtrl_SubModels;
    wxNotebook* TypeNotebook;
    wxPanel* ModelPreviewPanelLocation;
    wxPanel* Panel2;
    wxPanel* Panel3;
    wxPanel* SubBufferPanelHolder;
    wxSplitterWindow* SplitterWindow1;
    wxStaticText* StaticText1;
    wxStaticText* StaticTextName;
    wxTextCtrl* TextCtrl_Name;
    //*)

protected:

    //(*Identifiers(SubModelsDialog)
    static const long ID_STATICTEXT1;
    static const long ID_LISTCTRL_SUB_MODELS;
    static const long ID_BUTTON3;
    static const long ID_BUTTON4;
    static const long ID_BUTTON5;
    static const long ID_BUTTON_SUB_IMPORT;
    static const long ID_PANEL4;
    static const long ID_STATICTEXT_NAME;
    static const long ID_TEXTCTRL_NAME;
    static const long ID_CHECKBOX1;
    static const long ID_BUTTON6;
    static const long ID_BUTTON8;
    static const long ID_GRID1;
    static const long ID_BUTTON1;
    static const long ID_BUTTON2;
    static const long ID_BUTTON_MOVE_UP;
    static const long ID_BUTTON_MOVE_DOWN;
    static const long ID_BUTTON7;
    static const long ID_PANEL2;
    static const long ID_PANEL3;
    static const long ID_NOTEBOOK1;
    static const long ID_PANEL5;
    static const long ID_SPLITTERWINDOW1;
    static const long ID_PANEL1;
    //*)

    wxString GetSelectedName() const;
    int GetSelectedIndex() const;
    wxString GetSelectedNames();
    int GetSubModelInfoIndex(const wxString &str);
    SubModelInfo *GetSubModelInfo(const wxString &str);
    bool IsItemSelected(wxListCtrl* ctrl, int item) const;

    void AddSubModelToList(SubModelInfo *submodel, int index = -1, bool load = false);
    void MoveSelectedModelsTo(int indexTo);
    void RemoveSubModelFromList(wxString name);
    wxString GenerateSubModelName(wxString basename);

    void PopulateList();
    void ValidateWindow();
    void Select(const wxString &name);
    void SelectAll(const wxString &names);
    void UnSelectAll();

    void GenerateSegment(SubModelInfo* sm, int segments, int segment, bool horizontal, int count);
    void DisplayRange(const wxString &range);
    void SelectRow(int r);
    wxString ReverseRow(wxString row);

    void ImportSubModel(std::string filename);
    void ReadSubModelXML(wxXmlNode* xmlData);

private:

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
    void OnTextCtrl_NameText_Change(wxCommandEvent& event);
    void OnButton_ReverseRowsClick(wxCommandEvent& event);
    void OnButton_ReverseRowClick(wxCommandEvent& event);
    void OnButton_MoveDownClick(wxCommandEvent& event);
    void OnButton_MoveUpClick(wxCommandEvent& event);
    void OnButton_Sub_ImportClick(wxCommandEvent& event);
    //*)

    wxWindow* _parent;
    xLightsFrame* xlights;

    void OnDrop(wxCommandEvent& event);
    //void OnGridChar(wxKeyEvent& event);

    DECLARE_EVENT_TABLE()
};

#endif
