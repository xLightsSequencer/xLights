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

#include <set>
#include <string>

class wxButton;
class wxChoice;
class wxCommandEvent;
class wxGrid;
class wxGridEvent;
class SequenceElements;
class xLightsFrame;

// "Faces" tab in the Sequence Settings dialog - edits the sequence-level
// Matrix (image) face definitions stored in the .xsq (SequenceFaces on
// SequenceElements). Mirrors the Matrix tab of ModelFaceDialog.
class SequenceFacesPanel : public wxPanel
{
public:
    SequenceFacesPanel(wxWindow* parent, SequenceElements* seqElements,
                       const std::string& showDirectory, xLightsFrame* xlFrame);
    virtual ~SequenceFacesPanel();

    // Re-render Faces effects that reference definitions edited in this
    // panel. Called on dialog close (and from the destructor as a fallback).
    void ApplyPendingRenders();

private:
    std::string GetSelectedFace() const;
    void SelectFace(const std::string& name);
    void RefreshFaceList(const std::string& select = "");
    void UpdateEnabledState();
    void ValidateMatrixGrid(int r, int c) const;
    void SetFaceImage(const std::string& name, const std::string& key, int r, int c, const std::string& value);
    void TouchFace(const std::string& name);
    void SelectMatrixImage(int r, int c);
    void RenameFaceReferences(const std::string& oldName, const std::string& newName);

    void OnFaceChoiceSelect(wxCommandEvent& event);
    void OnButtonAddClick(wxCommandEvent& event);
    void OnButtonDeleteClick(wxCommandEvent& event);
    void OnButtonRenameClick(wxCommandEvent& event);
    void OnButtonImportFromModelClick(wxCommandEvent& event);
    void OnButtonDownloadImagesClick(wxCommandEvent& event);
    void OnImagePlacementSelect(wxCommandEvent& event);
    void OnGridCellLeftDClick(wxGridEvent& event);
    void OnGridLabelLeftDClick(wxGridEvent& event);
    void OnGridCellChange(wxGridEvent& event);

    SequenceElements* _seqElements = nullptr;
    std::string _showDirectory;
    xLightsFrame* _xlFrame = nullptr;

    wxChoice* FaceChoice = nullptr;
    wxButton* Button_Add = nullptr;
    wxButton* Button_Delete = nullptr;
    wxButton* Button_Rename = nullptr;
    wxButton* Button_ImportFromModel = nullptr;
    wxButton* Button_DownloadImages = nullptr;
    wxChoice* ImagePlacementChoice = nullptr;
    wxGrid* FacesGrid = nullptr;

    std::set<std::string> _touchedFaces;
    bool _rendersApplied = false;
};
