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
class ModelManager;

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
		SubModelInfo(const SubModelInfo &n) : name(n.name), oldName(n.oldName),
			vertical(n.vertical), isRanges(n.isRanges), subBuffer(n.subBuffer), strands(n.strands) {}

		bool operator==(const SubModelInfo &n) const { 
			return (name == n.name && oldName == n.oldName && vertical == n.vertical && 
				isRanges == n.isRanges && subBuffer == n.subBuffer && strands == n.strands);
		}
		bool operator!=(const SubModelInfo &n) const {
			return !(*this == n);
		}

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
    bool _isMatrix = false;

public:
    std::vector<SubModelInfo*> _subModels;

    SubModelsDialog(wxWindow* parent);
    virtual ~SubModelsDialog();

    void Setup(Model *m);
    void Save();

    //(*Declarations(SubModelsDialog)
    wxButton* AddButton;
    wxButton* AddRowButton;
    wxButton* ButtonCopy;
    wxButton* ButtonCopyModel;
    wxButton* Button_Draw_Model;
    wxButton* Button_Generate;
    wxButton* Button_MoveDown;
    wxButton* Button_MoveUp;
    wxButton* Button_ReverseNodes;
    wxButton* Button_ReverseRow;
    wxButton* Button_ReverseRows;
    wxButton* Button_Sub_Import;
    wxButton* Button_importCustom;
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
    static const long ID_BUTTONCOPY;
    static const long ID_BUTTON5;
    static const long ID_BUTTON_COPY_MODEL;
    static const long ID_BUTTON_SUB_IMPORT;
    static const long ID_BUTTON9;
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
    static const long ID_BUTTON_DRAW_MODEL;
    static const long ID_PANEL2;
    static const long ID_PANEL3;
    static const long ID_NOTEBOOK1;
    static const long ID_PANEL5;
    static const long ID_PANEL1;
    static const long ID_SPLITTERWINDOW1;
    //*)

    wxString GetSelectedName() const;
    int GetSelectedIndex() const;
    wxString GetSelectedNames();
    int GetSubModelInfoIndex(const wxString &str);
    SubModelInfo *GetSubModelInfo(const wxString &str);
    bool IsItemSelected(wxListCtrl* ctrl, int item) const;

    //void AddSubModelToList(SubModelInfo *submodel, int index = -1, bool load = false);
    void MoveSelectedModelsTo(int indexTo);
    void RemoveSubModelFromList(wxString name);
    wxString GenerateSubModelName(wxString basename);
    void ImportCustomModel(std::string filename);
    void FixNodes(wxXmlNode* n, const std::string& attribute, std::map<int, int>& nodeMap);

    void ApplySubmodelName();
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
    wxArrayString getModelList(ModelManager* modelManager);

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
    void OnButtonCopyModelClick(wxCommandEvent& event);
    void OnButton_Sub_CopyClick(wxCommandEvent& event);
    void OnButton_Draw_ModelClick(wxCommandEvent& event);
    void OnNodesGridLabelLeftDClick(wxGridEvent& event);
    void OnNodesGridCellLeftDClick(wxGridEvent& event);
    void OnButton_importCustomClick(wxCommandEvent& event);
    //*)

    void OnTextCtrl_NameText_KillFocus(wxFocusEvent& event);

    wxWindow* _parent;
    xLightsFrame* xlights;

    void OnDrop(wxCommandEvent& event);
    //void OnGridChar(wxKeyEvent& event);

    DECLARE_EVENT_TABLE()
};
