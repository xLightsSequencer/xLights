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
#include <wx/regex.h>
#include <wx/timer.h>
#include <glm/glm.hpp>

//(*Headers(SubModelsDialog)
#include <wx/dialog.h>
class wxBoxSizer;
class wxButton;
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxGrid;
class wxGridEvent;
class wxListCtrl;
class wxNotebook;
class wxNotebookEvent;
class wxPanel;
class wxSearchCtrl;
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
class xlColor;
class OutputManager;

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

//https://forums.wxwidgets.org/viewtopic.php?f=20&t=41045
class StretchGrid : public wxGrid
{
public:
    StretchGrid (wxWindow *parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
	       long style = wxWANTS_CHARS, const wxString& name = wxGridNameStr);
    ~StretchGrid ();

protected:
    void OnGridWindowSize (wxSizeEvent& event);
    void OnColHeaderSize (wxGridSizeEvent& event);
    void AutoSizeLastCol ();
};

class SubModelsDialog : public wxDialog
{
    struct SubModelInfo {
        SubModelInfo() {}
        SubModelInfo(const wxString &n) : name(n), oldName(n) {}
        SubModelInfo(const SubModelInfo &n) : name(n.name), oldName(n.oldName),
            vertical(n.vertical), isRanges(n.isRanges), subBuffer(n.subBuffer),
            bufferStyle(n.bufferStyle), strands(n.strands) {}

        bool operator==(const SubModelInfo &n) const {
            return (name == n.name && oldName == n.oldName && vertical == n.vertical &&
                isRanges == n.isRanges && subBuffer == n.subBuffer &&
                strands == n.strands && bufferStyle == n.bufferStyle);
        }
        bool operator!=(const SubModelInfo &n) const {
            return !(*this == n);
        }

        wxString name;
        wxString oldName;
        bool vertical{false};
        bool isRanges{true};
        wxString subBuffer;
        wxString bufferStyle{"Default"};
        std::vector<wxString> strands;
    };

    wxTimer timer1;
    bool _oldOutputToLights = false;
    OutputManager* _outputManager = nullptr;
    std::vector<uint32_t> _selected;
    Model* model = nullptr;
    ModelPreview *modelPreview = nullptr;
    SubBufferPanel *subBufferPanel = nullptr;
    bool _isMatrix = false;

    bool m_creating_bound_rect;
    int m_bound_start_x;
    int m_bound_start_y;
    int m_bound_end_x;
    int m_bound_end_y;
    int mPointSize;

    void StartOutputToLights();
    bool StopOutputToLights();

public:
    std::vector<SubModelInfo*> _subModels;

    bool ReloadLayout = false;

    SubModelsDialog(wxWindow* parent, OutputManager* outputManager);
    virtual ~SubModelsDialog();

    void Setup(Model *m);
    void Save();

    //(*Declarations(SubModelsDialog)
    StretchGrid* NodesGrid;
    wxButton* AddButton;
    wxButton* AddRowButton;
    wxButton* ButtonCopy;
    wxButton* ButtonImport;
    wxButton* Button_Draw_Model;
    wxButton* Button_Edit;
    wxButton* Button_Export;
    wxButton* Button_MoveDown;
    wxButton* Button_MoveUp;
    wxButton* Button_ReverseNodes;
    wxButton* Button_ReverseRow;
    wxButton* Button_ReverseRows;
    wxButton* Button_SortRow;
    wxButton* DeleteButton;
    wxButton* DeleteRowButton;
    wxCheckBox* CheckBox_OutputToLights;
    wxCheckBox* LayoutCheckbox;
    wxChoice* ChoiceBufferStyle;
    wxFlexGridSizer* PreviewSizer;
    wxFlexGridSizer* SubBufferSizer;
    wxListCtrl* ListCtrl_SubModels;
    wxNotebook* TypeNotebook;
    wxPanel* ModelPreviewPanelLocation;
    wxPanel* Panel2;
    wxPanel* Panel3;
    wxPanel* SubBufferPanelHolder;
    wxSearchCtrl* SearchCtrl1;
    wxSplitterWindow* SplitterWindow1;
    wxStaticText* NodeNumberText;
    wxStaticText* StaticText1;
    wxStaticText* StaticText2;
    wxStaticText* StaticTextName;
    wxTextCtrl* TextCtrl_Name;
    //*)

protected:

    //(*Identifiers(SubModelsDialog)
    static const long ID_CHECKBOX2;
    static const long ID_STATICTEXT1;
    static const long ID_LISTCTRL_SUB_MODELS;
    static const long ID_SEARCHCTRL1;
    static const long ID_BUTTON3;
    static const long ID_BUTTON4;
    static const long ID_BUTTONCOPY;
    static const long ID_BUTTON_EDIT;
    static const long ID_BUTTON_IMPORT;
    static const long ID_BUTTON10;
    static const long ID_PANEL4;
    static const long ID_STATICTEXT_NAME;
    static const long ID_TEXTCTRL_NAME;
    static const long ID_STATICTEXT2;
    static const long ID_CHOICE_BUFFER_STYLE;
    static const long ID_CHECKBOX1;
    static const long ID_GRID1;
    static const long ID_BUTTON6;
    static const long ID_BUTTON8;
    static const long ID_BUTTON1;
    static const long ID_BUTTON2;
    static const long ID_BUTTON_MOVE_UP;
    static const long ID_BUTTON_MOVE_DOWN;
    static const long ID_BUTTON7;
    static const long ID_BUTTON_SORT_ROW;
    static const long ID_BUTTON_DRAW_MODEL;
    static const long ID_PANEL2;
    static const long ID_PANEL3;
    static const long ID_NOTEBOOK1;
    static const long ID_PANEL5;
    static const long ID_PANEL1;
    static const long ID_SPLITTERWINDOW1;
    static const long ID_STATICTEXT3;
    //*)
    static const long ID_TIMER1;

    static const long SUBMODEL_DIALOG_IMPORT_MODEL;
    static const long SUBMODEL_DIALOG_IMPORT_FILE;
    static const long SUBMODEL_DIALOG_IMPORT_CUSTOM;
    static const long SUBMODEL_DIALOG_IMPORT_CSV;
    static const long SUBMODEL_DIALOG_EXPORT_CSV;
    static const long SUBMODEL_DIALOG_EXPORT_XMODEL;
    static const long SUBMODEL_DIALOG_EXPORT_TOOTHERS;
    static const long SUBMODEL_DIALOG_GENERATE;
    static const long SUBMODEL_DIALOG_SHIFT;
    static const long SUBMODEL_DIALOG_FLIP_HOR;
    static const long SUBMODEL_DIALOG_FLIP_VER;
    static const long SUBMODEL_DIALOG_REVERSE;
    static const long SUBMODEL_DIALOG_JOIN;
    static const long SUBMODEL_DIALOG_JOIN_SS;
    static const long SUBMODEL_DIALOG_SPLIT;
    static const long SUBMODEL_DIALOG_SORT_BY_NAME;
    static const long SUBMODEL_DIALOG_REMOVE_DUPLICATE;
    static const long SUBMODEL_DIALOG_SUPPRESS_DUPLICATE;
    static const long SUBMODEL_DIALOG_SORT_POINTS;
    static const long SUBMODEL_DIALOG_REMOVE_ALL_DUPLICATE_LR;
    static const long SUBMODEL_DIALOG_REMOVE_ALL_DUPLICATE_TB;
    static const long SUBMODEL_DIALOG_SUPPRESS_ALL_DUPLICATE_LR;
    static const long SUBMODEL_DIALOG_SUPPRESS_ALL_DUPLICATE_TB;
    static const long SUBMODEL_DIALOG_EVEN_ROWS;
    static const long SUBMODEL_DIALOG_PIVOT_ROWS_COLUMNS;
    static const long SUBMODEL_DIALOG_SYMMETRIZE;
    static const long SUBMODEL_DIALOG_SORT_POINTS_ALL;
    static const long SUBMODEL_DIALOG_COMBINE_STRANDS;
    static const long SUBMODEL_DIALOG_EXPAND_STRANDS_ALL;
    static const long SUBMODEL_DIALOG_COMPRESS_STRANDS_ALL;
    static const long SUBMODEL_DIALOG_BLANKS_AS_ZERO;
    static const long SUBMODEL_DIALOG_BLANKS_AS_EMPTY;
    static const long SUBMODEL_DIALOG_REMOVE_BLANKS_ZEROS;

    void SaveXML(Model* m);
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
    bool IsUniqueName(wxString const& newname) const;
    void ImportCustomModel(std::string filename);
    void FixNodes(wxXmlNode* n, const std::string& attribute, std::map<int, int>& nodeMap);

    void ApplySubmodelName();
    void PopulateList();
    void ValidateWindow();
    void Select(const wxString &name);
    void SelectAll(const wxString &names);
    void UnSelectAll();

    void Generate();
    void Shift();
    void FlipHorizontal();
    void FlipVertical();
    void Reverse();
    void RemoveDuplicates(bool suppress);
    void RemoveAllDuplicates(bool leftright, bool suppress);
    void MakeRowsUniform();
    void PivotRowsColumns();
    void CombineStrands();
    void OrderPoints(bool wholemodel);

    void GenerateSegment(SubModelInfo* sm, int segments, int segment, bool horizontal, int count);
    void DisplayRange(const wxString &range);
    void SelectRow(int r);
    bool SetNodeColor(int row, xlColor const& c, bool highlight);
    void ClearNodeColor(Model* m);
    wxString ReverseRow(wxString row);

    void ImportSubModel(std::string filename);
    void ReadSubModelXML(wxXmlNode* xmlData);
    void ImportSubModelXML(wxXmlNode* xmlData);
    void ImportCSVSubModel(wxString const& filename);
    wxArrayString getModelList(ModelManager* modelManager);
    void ExportSubModels(wxString const& filename);
    void ExportSubModelAsxModel(wxString const& filename, const std::string& name);
    void ExportSubmodelToOtherModels();

    void JoinSelectedModels(bool singlestrand);
    void SplitSelectedSubmodel();
    void SortSubModelsByName();
    void Symmetrize();

    void processAllStrands(wxString (*func)(wxString));

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
    void OnTypeNotebookPageChanged(wxBookCtrlEvent& event);
    void OnNodesGridLabelLeftClick(wxGridEvent& event);
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
    void OnButton_Sub_CopyClick(wxCommandEvent& event);
    void OnButton_Draw_ModelClick(wxCommandEvent& event);
    void OnNodesGridLabelLeftDClick(wxGridEvent& event);
    void OnNodesGridCellLeftDClick(wxGridEvent& event);
    void OnButton_SortRowClick(wxCommandEvent& event);
    void OnButtonImportClick(wxCommandEvent& event);
    void OnButton_EditClick(wxCommandEvent& event);
    void OnButton_ExportClick(wxCommandEvent& event);
    void OnListCtrl_SubModelsItemRClick(wxListEvent& event);
    void OnChoiceBufferStyleSelect(wxCommandEvent& event);
    void OnButton_SearchClick(wxCommandEvent& event);
    void OnInit(wxInitDialogEvent& event);
    void OnNodesGridCellRightClick(wxGridEvent& event);
    void OnCheckBox_OutputToLightsClick(wxCommandEvent& event);
    //*)

    void OnCancel(wxCloseEvent& event);
    void OnPreviewLeftUp(wxMouseEvent& event);
    void OnPreviewMouseLeave(wxMouseEvent& event);
    void OnPreviewLeftDown(wxMouseEvent& event);
    void OnPreviewLeftDClick(wxMouseEvent& event);
    void OnPreviewMouseMove(wxMouseEvent& event);
    void OnTimer1Trigger(wxTimerEvent& event);

    void OnImportBtnPopup(wxCommandEvent& event);
    void OnEditBtnPopup(wxCommandEvent& event);
    void OnExportBtnPopup(wxCommandEvent& event);
    void OnListPopup(wxCommandEvent& event);
    void OnNodesGridPopup(wxCommandEvent& event);

    void RenderModel();
    void GetMouseLocation(int x, int y, glm::vec3& ray_origin, glm::vec3& ray_direction);
    void SelectAllInBoundingRect(bool shiftdwn, bool cdwn);
    void RemoveNodes(bool suppress);

    void OnTextCtrl_NameText_KillFocus(wxFocusEvent& event);
    void OnSubbufferSize(wxSizeEvent& event);

    wxWindow* _parent = nullptr;
    xLightsFrame* xlights = nullptr;

    void OnDrop(wxCommandEvent& event);
    //void OnGridChar(wxKeyEvent& event);

    DECLARE_EVENT_TABLE()
};
