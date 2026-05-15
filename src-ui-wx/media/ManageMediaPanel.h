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
#include <list>
#include <optional>
#include <string>
#include <vector>
#include <memory>
#include <wx/timer.h>

#include "render/SequenceMedia.h"

class SequenceElements;
class xLightsFrame;
class xlImage;

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
    MediaType mediaType = MediaType::Image;
    bool canLoad = true;    // false when entry->IsOk() returns false

    // children (only valid for group nodes)
    std::vector<std::shared_ptr<MediaNode>> children;
    MediaNode* parent = nullptr;
};

// Custom wxDataViewModel for the two-level tree
class MediaViewModel : public wxDataViewModel
{
public:
    MediaViewModel();

    void Rebuild(SequenceMedia* media, const std::string& showDirectory,
                 const std::list<std::string>& mediaDirs = {},
                 std::optional<MediaType> filterType = std::nullopt);
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
    bool GetAttr(const wxDataViewItem& item, unsigned int col, wxDataViewItemAttr& attr) const override;

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
                     std::optional<MediaType> filterType = std::nullopt,
                     wxWindowID id = wxID_ANY);
    virtual ~ManageMediaPanel();

    void Populate(const std::string& selectPath = {});
    void ExpandGroups();
    void RequestExpandGroups();

private:
    void OnIdle(wxIdleEvent& event);
    void OnTreeItemSelected(wxDataViewEvent& event);
    void OnTreeMouseMotion(wxMouseEvent& event);
    void OnTreeContextMenu(wxDataViewEvent& event);
    void OnReloadMedia(const std::string& path);
    void OnReSelectImage(const std::string& oldPath);
    void OnBulkFindImages();
    void OnReSelectShader(const std::string& oldPath);
    void OnBulkFindShaders();
    void ReSelectMediaByType(const std::string& oldPath, MediaType type);
    void BulkFindMediaByType(MediaType type);
    void UpdateEffectPaths(const std::string& oldPath, const std::string& newPath);
    void OnAddButtonClick(wxCommandEvent& event);
    void OnAIGenerateButtonClick(wxCommandEvent& event);
    void OnRenameButtonClick(wxCommandEvent& event);
    void OnEmbedButtonClick(wxCommandEvent& event);
    void OnExtractButtonClick(wxCommandEvent& event);
    void OnEmbedAllButtonClick(wxCommandEvent& event);
    void OnExtractAllButtonClick(wxCommandEvent& event);
    void OnRemoveButtonClick(wxCommandEvent& event);

    void UpdateButtons();
    void UpdatePreview(const std::string& filepath, MediaType type = MediaType::Image);
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
    std::optional<MediaType> _filterType;

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
    wxButton* _removeButton = nullptr;
    bool _expandPending = false;

    // Animated preview
    void OnPreviewTimer(wxTimerEvent& event);
    void ShowPreviewFrame(size_t index);
    wxTimer _previewTimer;
    std::vector<std::shared_ptr<xlImage>> _previewFrames;
    std::vector<long> _previewFrameTimes;
    size_t _currentPreviewFrame = 0;

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
                      xLightsFrame* xlFrame = nullptr,
                      std::optional<MediaType> filterType = std::nullopt,
                      const std::string& selectPath = {});

    std::string GetSelectedPath() const;

private:
    void OnOK(wxCommandEvent& event);
    void OnSelectionChanged(wxDataViewEvent& event);
    void OnAddFromDisk(wxCommandEvent& event);

    ManageMediaPanel* _panel = nullptr;
    wxButton* _okButton = nullptr;
    wxButton* _addFromDiskButton = nullptr;
    std::optional<MediaType> _filterType;
};
