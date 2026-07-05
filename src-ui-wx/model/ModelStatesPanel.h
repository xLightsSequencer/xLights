#pragma once
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/panel.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/choicebk.h>
#include <wx/grid.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/timer.h>
#include <wx/spinctrl.h>
#include <wx/colourdata.h>
#include <glm/glm.hpp>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

class Model;
class ModelPreview;
class OutputManager;
class ModelManager;
class xlColor;

class ModelStatesPanel : public wxPanel {
public:
    ModelStatesPanel(wxWindow* parent, OutputManager* outputManager, wxWindowID id = wxID_ANY);
    ~ModelStatesPanel();

    void SetModelPreview(ModelPreview* preview);
    void SetChangeCallback(std::function<void()> cb) { _changeCallback = std::move(cb); }
    void SetSubModelCallbacks(std::function<std::vector<std::string>()> getNames,
                              std::function<std::string(const std::string&)> getRanges);

    void SetStateInfo(Model* cls, std::map<std::string, std::map<std::string, std::string>> const& info);
    [[nodiscard]] std::map<std::string, std::map<std::string, std::string>> GetStateInfo() const;

    void OnActivate();
    void OnDeactivate();

    // Preview mouse events — connected in SetModelPreview(), public so shell can rewire
    void OnPreviewLeftDown(wxMouseEvent& event);
    void OnPreviewLeftUp(wxMouseEvent& event);
    void OnPreviewMouseMove(wxMouseEvent& event);
    void OnPreviewMouseLeave(wxMouseEvent& event);
    void OnPreviewLeftDClick(wxMouseEvent& event);

    // IDs copied from ModelStateDialog — keep identical so event table works
    static const long STATE_DIALOG_IMPORT_SUB;
    static const long STATE_DIALOG_IMPORT_ALL_SUB;
    static const long STATE_DIALOG_COPY_STATES;
    static const long STATE_DIALOG_IMPORT_MODEL;
    static const long STATE_DIALOG_IMPORT_FILE;
    static const long STATE_DIALOG_IMPORT_DOWNLOAD;
    static const long STATE_DIALOG_COPY;
    static const long STATE_DIALOG_RENAME;
    static const long STATE_DIALOG_SHIFT;
    static const long STATE_DIALOG_REVERSE;
    static const long STATE_DIALOG_CLEAR_SELECTED_ROWS;
    static const long STATE_DIALOG_CLEAR_STATES;
    static const long STATE_DIALOG_EXPORT_TOOTHERS;
    static const long STATE_DIALOG_BULK_COLOR_CHANGE;
    static const wxWindowID ID_MNU_ADD_STATE_BEFORE;
    static const wxWindowID ID_MNU_ADD_STATE_AFTER;
    static const wxWindowID ID_MNU_DELETE_STATE;
    static const wxWindowID ID_MNU_MOVE_STATE_UP;
    static const wxWindowID ID_MNU_MOVE_STATE_DOWN;
    static const wxWindowID ID_MNU_SORT;

    // UI controls (same names as ModelStateDialog for easy diff)
    wxButton*   Button_7Seg = nullptr;
    wxButton*   Button_7Segment = nullptr;
    wxButton*   Button_Import = nullptr;
    wxButton*   DeleteButton = nullptr;
    wxCheckBox* CheckBox_OutputToLights = nullptr;
    wxCheckBox* CustomColorNodeRanges = nullptr;
    wxCheckBox* CustomColorSingleNode = nullptr;
    wxChoice*   ChoiceColorDraw = nullptr;
    wxChoice*   StateFilterChoice = nullptr;
    wxChoice*   NameChoice = nullptr;
    wxButton*   Button_PlayStates = nullptr;
    wxSpinCtrl* Spin_StateSpeed = nullptr;
    wxChoicebook* StateTypeChoice = nullptr;
    wxGrid*     NodeRangeGrid = nullptr;
    wxGrid*     SingleNodeGrid = nullptr;
    wxStaticText* StaticText3 = nullptr;

private:
    // wxWindow IDs — new values, not reusing ModelStateDialog's IDs
    static const wxWindowID ID_STATICTEXT2;
    static const wxWindowID ID_CHOICE3;
    static const wxWindowID ID_BUTTON3;
    static const wxWindowID ID_BUTTON_IMPORT;
    static const wxWindowID ID_BUTTON4;
    static const wxWindowID ID_CHECKBOX1;
    static const wxWindowID ID_BUTTON1;
    static const wxWindowID ID_GRID_COROSTATES;
    static const wxWindowID ID_PANEL2;
    static const wxWindowID ID_CHECKBOX2;
    static const wxWindowID ID_CHECKBOX3;
    static const wxWindowID ID_BUTTON2;
    static const wxWindowID ID_GRID3;
    static const wxWindowID ID_PANEL6;
    static const wxWindowID ID_CHOICEBOOK1;
    static const wxWindowID ID_STATICTEXT1;
    static const wxWindowID ID_CHOICE_COLOR_DRAW;
    static const wxWindowID ID_CHOICE_STATE_FILTER;
    static const wxWindowID ID_BUTTON_PLAY_STATES;
    static const wxWindowID ID_SLIDER_STATE_SPEED;
    static const wxWindowID ID_STATE_ANIM_TIMER;
    static const long ID_TIMER1;

    // Event handlers — identical signatures to ModelStateDialog
    void OnMatrixNameChoiceSelect(wxCommandEvent& event);
    void OnButtonMatrixAddClicked(wxCommandEvent& event);
    void OnButtonMatrixDeleteClick(wxCommandEvent& event);
    void OnCustomColorCheckboxClick(wxCommandEvent& event);
    void OnNodeRangeGridCellChange(wxGridEvent& event);
    void OnSingleNodeGridCellChange(wxGridEvent& event);
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
    void OnSingleNodeGridLabelLeftClick(wxGridEvent& event);
    void OnNodeRangeGridLabelLeftClick(wxGridEvent& event);
    void OnCheckBox_OutputToLightsClick(wxCommandEvent& event);
    void OnChoiceColorDrawSelect(wxCommandEvent& event);
    void OnStateFilterSelect(wxCommandEvent& event);
    void OnPlayStatesClick(wxCommandEvent& event);
    void OnStateAnimTimerTick(wxTimerEvent& event);
    void OnAddBtnPopup(wxCommandEvent& event);
    void OnTimer1Trigger(wxTimerEvent& event);
    void OnSingleNodeGridResize(wxSizeEvent& event);
    void OnNodeRangeGridResize(wxSizeEvent& event);

    void NotifyChange();
    void UpdateStateType();
    void SelectStateModel(const std::string& s);
    void SelectRow(wxGrid* grid, int const row);
    std::vector<int> GetSelectedGridRows(wxGrid* grid, int fallbackRow);
    void SelectRows(wxGrid* grid, int fallbackRow);
    void OnNodeRangeGridRangeSelect(wxGridRangeSelectEvent& event);
    void OnSingleNodeGridRangeSelect(wxGridRangeSelectEvent& event);
    void SetSingleNodeColor(wxGrid* grid, const int row, xlColor const& c, bool highlight);
    bool SetNodeColor(wxGrid* grid, int const row, xlColor const& c, bool highlight);
    void ClearNodeColor(Model* m);
    xlColor GetRowColor(wxGrid* grid, int const row, bool const prev, bool const force);

    void GetValue(wxGrid* grid, const int row, const int col, std::map<std::string, std::string>& info);
    void AddLabel(wxString label);
    void ValidateWindow();
    bool HasInvalidRows() const;
    void PopulateStateFilter();
    void UpdateStateFilter();
    void BuildStateAnimSteps();
    void StopStateAnimation();

    void OnGridPopup(const int rightEventID, wxGridEvent& gridEvent);
    void ImportSubmodel(wxGridEvent& event);
    wxString getSubmodelNodes(Model* sm);
    void ImportStates(const wxString& filename);
    void ImportStatesFromModel();
    void ImportStatesFromSubModels();
    void ExportStatesToOtherModels();
    std::string cleanSubName(std::string name);
    void AddStates(std::map<std::string, std::map<std::string, std::string>> const& states);
    wxArrayString getModelList(ModelManager* modelManager);

    wxString GetDownloadStates();

    void CopyStates(wxGridEvent& event);
    void ClearStates(wxGridEvent& event);
    void ClearSelectedStates(wxGridEvent& event);
    void AddBefore(wxGridEvent& event);
    void AddAfter(wxGridEvent& event);
    void DeleteSelected(wxGridEvent& event);
    void MoveSelectedUp(wxGridEvent& event);
    void MoveSelectedDown(wxGridEvent& event);
    void SortState(wxGridEvent& event);

    void CopyStateData();
    void RenameState();
    void RemoveNodes();

    void ShiftStateNodes();
    void ReverseStateNodes();

    void StartOutputToLights();
    bool StopOutputToLights();

    void RenderModel();
    void GetMouseLocation(int x, int y, glm::vec3& ray_origin, glm::vec3& ray_direction);
    void SelectAllInBoundingRect(bool shiftdwn);

    wxTimer timer1;
    wxTimer _stateAnimTimer;
    bool _stateAnimPlaying = false;
    int _stateAnimStep = 0;
    std::vector<std::vector<int>> _stateAnimSteps;
    bool _oldOutputToLights = false;
    OutputManager* _outputManager = nullptr;
    ModelPreview* _modelPreview = nullptr;
    std::function<void()> _changeCallback;
    std::function<std::vector<std::string>()> _getSubModelNames;
    std::function<std::string(const std::string&)> _getSubModelRanges;
    std::vector<uint32_t> _selected;
    bool _isActive = false;
    bool m_creating_bound_rect = false;
    int m_bound_start_x = 0;
    int m_bound_start_y = 0;
    int m_bound_end_x = 0;
    int m_bound_end_y = 0;
    int mPointSize;
    Model* model = nullptr;
    bool overRide = false;
    bool showDialog = true;

    std::unordered_map<std::string, std::vector<size_t>> _nodeNameToIndex;
    std::map<std::string, std::map<std::string, std::string>> stateData;

    DECLARE_EVENT_TABLE()
};
