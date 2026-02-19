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
#include <wx/dataview.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <string>
#include <vector>
#include <memory>

class SequenceMedia;
class SequenceElements;
class xLightsFrame;

// Column indices
enum MediaModelCol {
    COL_NAME   = 0,
    COL_SIZE   = 1,
    COL_FRAMES = 2,
    COL_STATUS = 3,
    COL_COUNT  = 4
};

// Node types used in the model
struct MediaNode {
    bool isGroup = false;
    std::string label;      // group label (directory path or "(Embedded)")
    std::string filePath;   // leaf: full original path
    // leaf display data
    wxString sizeStr;
    wxString framesStr;
    wxString statusStr;

    // children (only valid for group nodes)
    std::vector<std::shared_ptr<MediaNode>> children;
    MediaNode* parent = nullptr;
};

// Custom wxDataViewModel for the two-level tree
class MediaViewModel : public wxDataViewModel
{
public:
    MediaViewModel();

    void Rebuild(SequenceMedia* media, const std::string& showDirectory);
    void Clear();

    // Returns the file path for an item, or empty if it's a group node
    std::string GetFilePath(const wxDataViewItem& item) const;
    bool IsGroup(const wxDataViewItem& item) const;
    // Find the item for a given file path (invalid item if not found)
    wxDataViewItem FindItem(const std::string& filePath) const;

    // wxDataViewModel overrides
    unsigned int GetColumnCount() const override { return COL_COUNT; }
    wxString GetColumnType(unsigned int col) const override { return "string"; }

    void GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const override;
    bool SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col) override { return false; }

    wxDataViewItem GetParent(const wxDataViewItem& item) const override;
    bool IsContainer(const wxDataViewItem& item) const override;
    unsigned int GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const override;

private:
    // Top-level group nodes (owned here)
    std::vector<std::shared_ptr<MediaNode>> _groups;
};

class ManageMediaPanel : public wxPanel
{
public:
    ManageMediaPanel(wxWindow* parent, SequenceMedia* sequenceMedia,
                     SequenceElements* sequenceElements,
                     const std::string& showDirectory = {},
                     xLightsFrame* xlFrame = nullptr,
                     bool singleSelect = false,
                     wxWindowID id = wxID_ANY);
    virtual ~ManageMediaPanel();

    void Populate(const std::string& selectPath = {});

private:
    void OnTreeItemSelected(wxDataViewEvent& event);
    void OnTreeMouseMotion(wxMouseEvent& event);
    void OnAddButtonClick(wxCommandEvent& event);
    void OnAIGenerateButtonClick(wxCommandEvent& event);
    void OnRenameButtonClick(wxCommandEvent& event);
    void OnEmbedButtonClick(wxCommandEvent& event);
    void OnExtractButtonClick(wxCommandEvent& event);
    void OnEmbedAllButtonClick(wxCommandEvent& event);
    void OnExtractAllButtonClick(wxCommandEvent& event);

    void UpdateButtons();
    void UpdatePreview(const std::string& filepath);
    // Returns full paths for all selected leaf items
    std::vector<std::string> GetSelectedPaths() const;

    // Compute the stripped display path for a full path (empty if no change needed)
    std::string StrippedPath(const std::string& fullPath) const;

    // Rename image in cache + update all effect references, then embed it.
    // Returns the path actually used (stripped if renamed, original otherwise).
    std::string EmbedWithRename(const std::string& fullPath);

    // Prompt user for a save path, write embedded data to disk, update effect
    // references to the new path, and mark the entry as external.
    // Returns the new path on success, empty string on cancel/failure.
    std::string ExtractWithRename(const std::string& fullPath);

    SequenceMedia* _sequenceMedia = nullptr;
    SequenceElements* _sequenceElements = nullptr;
    xLightsFrame* _xlFrame = nullptr;
    std::string _showDirectory;

    wxDataViewCtrl* _mediaTree = nullptr;
    wxObjectDataPtr<MediaViewModel> _model;
    wxStaticBitmap* _preview = nullptr;
    wxStaticText* _infoLabel = nullptr;
    wxButton* _addButton = nullptr;
    wxButton* _aiGenerateButton = nullptr;
    wxButton* _renameButton = nullptr;
    wxButton* _embedButton = nullptr;
    wxButton* _extractButton = nullptr;
    wxButton* _embedAllButton = nullptr;
    wxButton* _extractAllButton = nullptr;

    wxDECLARE_EVENT_TABLE();

    friend class SelectMediaDialog;
};

// Modal dialog wrapping ManageMediaPanel in single-select mode.
// Use GetSelectedPath() after ShowModal() == wxID_OK.
class SelectMediaDialog : public wxDialog
{
public:
    SelectMediaDialog(wxWindow* parent, SequenceMedia* sequenceMedia,
                      SequenceElements* sequenceElements,
                      const std::string& showDirectory = {},
                      xLightsFrame* xlFrame = nullptr);

    std::string GetSelectedPath() const;

private:
    void OnOK(wxCommandEvent& event);
    void OnSelectionChanged(wxDataViewEvent& event);

    ManageMediaPanel* _panel = nullptr;
    wxButton* _okButton = nullptr;
};
