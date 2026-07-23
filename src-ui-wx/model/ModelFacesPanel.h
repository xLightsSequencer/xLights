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
#include <wx/filename.h>
#include <wx/timer.h>
#include <wx/colourdata.h>
#include <glm/glm.hpp>
#include <map>
#include <list>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <functional>

class Model;
class ModelPreview;
class OutputManager;
class ModelManager;

class ModelFacesPanel : public wxPanel {
public:
    ModelFacesPanel(wxWindow* parent, OutputManager* outputManager, wxWindowID id = wxID_ANY);
    ~ModelFacesPanel();

    void SetModelPreview(ModelPreview* preview);
    void SetChangeCallback(std::function<void()> cb) { _changeCallback = std::move(cb); }
    void SetSubModelCallbacks(std::function<std::vector<std::string>()> getNames,
                              std::function<std::string(const std::string&)> getRanges);
    // Live (in-flight, not-yet-saved) state info from the States tab, used by
    // the state-outline overlay so it reflects unsaved edits.
    void SetStateInfoCallback(std::function<std::map<std::string, std::map<std::string, std::string>>()> getStateInfo) { _getStateInfo = std::move(getStateInfo); }
    void RefreshStateOutlineChoice(const std::map<std::string, std::map<std::string, std::string>>& stateInfo);

    void SetFaceInfo(Model* cls, std::map<std::string, std::map<std::string, std::string>> const& info);
    [[nodiscard]] std::map<std::string, std::map<std::string, std::string>> GetFaceInfo() const;
    [[nodiscard]] const std::vector<std::pair<std::string, std::string>>& GetRenamedFaces() const { return _renamedFaces; }

    void OnActivate();
    void OnDeactivate();

    // Preview mouse events — connected in SetModelPreview(), public so shell can rewire
    void OnPreviewLeftDown(wxMouseEvent& event);
    void OnPreviewLeftUp(wxMouseEvent& event);
    void OnPreviewMouseMove(wxMouseEvent& event);
    void OnPreviewMouseLeave(wxMouseEvent& event);
    void OnPreviewLeftDClick(wxMouseEvent& event);

    // IDs copied from ModelFaceDialog — keep identical so event table works
    static const long FACES_DIALOG_IMPORT_SUB;
    static const long FACES_DIALOG_IMPORT_MODEL;
    static const long FACES_DIALOG_IMPORT_FILE;
    static const long FACES_DIALOG_EXPORT_TOOTHERS;
    static const long FACES_DIALOG_COPY;
    static const long FACES_DIALOG_RENAME;
    static const long FACES_DIALOG_DELETE;
    static const long FACES_DIALOG_SHIFT;
    static const long FACES_DIALOG_REVERSE;
    static const long FACES_DIALOG_BULK_COLOR_CHANGE;

    // UI controls (same names as ModelFaceDialog for easy diff)
    wxButton* ButtonMore = nullptr;
    wxButton* Button_DownloadImages = nullptr;
    wxCheckBox* CheckBox_OutputToLights = nullptr;
    wxCheckBox* CustomColorNodeRanges = nullptr;
    wxCheckBox* CustomColorSingleNode = nullptr;
    wxChoice* MatrixImagePlacementChoice = nullptr;
    wxChoice* NameChoice = nullptr;
    wxChoice* StateOutlineChoice = nullptr;
    wxChoice* OutlineFilterChoice = nullptr;
    wxChoice* EyesFilterChoice    = nullptr;
    wxChoice* MouthFilterChoice   = nullptr;
    wxChoicebook* FaceTypeChoice = nullptr;
    wxGrid* MatrixModelsGrid = nullptr;
    wxGrid* NodeRangeGrid = nullptr;
    wxGrid* SingleNodeGrid = nullptr;
    wxPanel* Matrix = nullptr;
    wxPanel* Panel_Matrix = nullptr;
    wxPanel* Panel_NodeRanges = nullptr;
    wxPanel* Panel_SingleNode = nullptr;
    wxStaticText* StaticText3 = nullptr;

private:
    // wxWindow IDs — new values, not reusing ModelFaceDialog's IDs
    static const wxWindowID ID_STATICTEXT2;
    static const wxWindowID ID_CHOICE3;
    static const wxWindowID ID_BUTTON_ADD;
    static const wxWindowID ID_BUTTON_MORE;
    static const wxWindowID ID_PANEL4;
    static const wxWindowID ID_CHECKBOX1;
    static const wxWindowID ID_GRID_COROFACES;
    static const wxWindowID ID_PANEL2;
    static const wxWindowID ID_PANEL8;
    static const wxWindowID ID_CHECKBOX2;
    static const wxWindowID ID_CHECKBOX3;
    static const wxWindowID ID_GRID3;
    static const wxWindowID ID_PANEL6;
    static const wxWindowID ID_PANEL7;
    static const wxWindowID ID_CHOICE2;
    static const wxWindowID ID_BUTTON1;
    static const wxWindowID ID_GRID1;
    static const wxWindowID ID_PANEL3;
    static const wxWindowID ID_CHOICEBOOK1;
    static const wxWindowID ID_PANEL5;
    static const wxWindowID ID_PANEL1;
    static const long ID_TIMER1;
    static const wxWindowID ID_CHOICE_STATE_OUTLINE;
    static const wxWindowID ID_CHOICE_OUTLINE_FILTER;
    static const wxWindowID ID_CHOICE_EYES_FILTER;
    static const wxWindowID ID_CHOICE_MOUTH_FILTER;

    // Event handlers — identical signatures to ModelFaceDialog
    void OnMatrixNameChoiceSelect(wxCommandEvent& event);
    void OnButtonMatrixAddClicked(wxCommandEvent& event);
    void OnFaceDeleteSelected();
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
    void OnMatrixModelsGridLabelLeftDClick(wxGridEvent& event);
    void OnCheckBox_OutputToLightsClick(wxCommandEvent& event);
    void OnStateOutlineChoiceSelect(wxCommandEvent& event);
    void OnOutlineFilterSelect(wxCommandEvent& event);
    void OnEyesFilterSelect(wxCommandEvent& event);
    void OnMouthFilterSelect(wxCommandEvent& event);
    void UpdateFaceQuickPreview();
    void ResetFaceFilterDropdowns();
    void UpdateFaceFilterDropdownItems();
    std::vector<int> GetActiveFilterRows() const;
    void ApplyStateOutlineOverlay();
    void OnMatrixModelsGridResize(wxSizeEvent& event);
    void OnSingleNodeGridResize(wxSizeEvent& event);
    void OnNodeRangeGridResize(wxSizeEvent& event);
    void OnButtonMoreClick(wxCommandEvent& event);
    void OnAddBtnPopup(wxCommandEvent& event);
    void OnTimer1Trigger(wxTimerEvent& event);

    void NotifyChange();
    void PaintFace(wxDC& dc, int x, int y, const char* xpm[]);
    void TryToSetAllMatrixModels(std::string name, std::string key, std::string new_filename, int row, int col);
    void TryToFindPath(wxString& filename) const;
    void ValidateMatrixGrid(int r, int c) const;
    void StartOutputToLights();
    bool StopOutputToLights();
    void SelectFaceModel(const std::string& s);
    void UpdatePreview(const std::string& channels, wxColor c);
    std::vector<int> GetSelectedGridRows(wxGrid* grid, int fallbackRow);
    void UpdatePreviewRows(wxGrid* grid, int fallbackRow);
    void OnNodeRangeGridRangeSelect(wxGridRangeSelectEvent& event);
    void OnSingleNodeGridRangeSelect(wxGridRangeSelectEvent& event);
    void GetValue(wxGrid* grid, const int row, const int col, std::map<std::string, std::string>& info);
    void OnGridPopup(const int rightEventID, wxGridEvent& gridEvent);
    void ImportSubmodel(wxGridEvent& event);
    wxString getSubmodelNodes(Model* sm);
    void ImportFaces(const wxString& filename);
    void ImportFacesFromModel();
    void AddFaces(std::map<std::string, std::map<std::string, std::string>> const& faces);
    wxArrayString getModelList(ModelManager* modelManager);
    void CopyFaceData();
    void RenameFace();
    void RemoveNodes();
    void ShiftFaceNodes();
    void ReverseFaceNodes();
    void SelectMatrixImage(int r, int c);
    void RenderModel();
    void GetMouseLocation(int x, int y, glm::vec3& ray_origin, glm::vec3& ray_direction);
    void SelectAllInBoundingRect(bool shiftdwn);
    void ExportFacesToOtherModels();

    wxTimer timer1;
    OutputManager* _outputManager = nullptr;
    ModelPreview* _modelPreview = nullptr;
    std::function<void()> _changeCallback;
    std::function<std::vector<std::string>()> _getSubModelNames;
    std::function<std::string(const std::string&)> _getSubModelRanges;
    std::function<std::map<std::string, std::map<std::string, std::string>>()> _getStateInfo;
    std::vector<uint32_t> _selected;
    bool _isActive = false;
    bool m_creating_bound_rect = false;
    int m_bound_start_x = 0, m_bound_start_y = 0, m_bound_end_x = 0, m_bound_end_y = 0;
    int mPointSize;
    Model* model = nullptr;
    std::map<std::string, std::map<std::string, std::string>> faceData;
    std::vector<std::pair<std::string, std::string>> _renamedFaces;
    std::unordered_map<std::string, std::vector<size_t>> _nodeNameToIndex;

    DECLARE_EVENT_TABLE()
};
