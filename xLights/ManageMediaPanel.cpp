/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ManageMediaPanel.h"
#include "render/SequenceMedia.h"
#include "VideoReader.h"
#include "render/SequenceElements.h"
#include "render/Element.h"
#include "render/EffectLayer.h"  // also defines NodeLayer
#include "render/Effect.h"
#include "UtilClasses.h"
#include "xLightsMain.h"
#include "ai/AIImageDialog.h"
#include "ai/aiType.h"
#include "ui/wxUtilities.h"

#include <wx/filename.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/datetime.h>
#include <map>
#include <algorithm>

#include "ExternalHooks.h"

static std::string MediaTypeName(MediaType t) {
    switch (t) {
        case MediaType::Image: return "Images";
        case MediaType::SVG: return "SVGs";
        case MediaType::Shader: return "Shaders";
        case MediaType::TextFile: return "Text Files";
        case MediaType::BinaryFile: return "Other Files";
        case MediaType::Video: return "Videos";
    }
    return "Unknown";
}

static wxString WildcardForMediaType(std::optional<MediaType> type) {
    if (!type.has_value()) return "All files (*.*)|*.*";
    switch (*type) {
        case MediaType::Image: return wxImage::GetImageExtWildcard();
        case MediaType::Video: return "Video Files|*.avi;*.mp4;*.mkv;*.mov;*.asf;*.flv;*.mpg;*.mpeg;*.m4v;*.wmv";
        case MediaType::Shader: return "Shader Files (*.fs)|*.fs";
        case MediaType::SVG: return "SVG Files (*.svg)|*.svg";
        case MediaType::TextFile: return "Text Files (*.txt)|*.txt";
        case MediaType::BinaryFile: return "Glediator Files|*.gled;*.out;*.csv";
    }
    return "All files (*.*)|*.*";
}

// ---------------------------------------------------------------------------
// MediaViewModel
// ---------------------------------------------------------------------------

MediaViewModel::MediaViewModel()
{
}

void MediaViewModel::Clear()
{
    _groups.clear();
}

void MediaViewModel::Rebuild(SequenceMedia* media, const std::string& showDirectory,
                             const std::list<std::string>& mediaDirs,
                             std::optional<MediaType> filterType)
{
    _groups.clear();

    if (media == nullptr) return;

    // Normalise the show directory: ensure it ends with the path separator
    // so we can do a simple prefix strip.
    wxString showDir = showDirectory;
    if (!showDir.IsEmpty() && showDir.Last() != wxFileName::GetPathSeparator()) {
        showDir += wxFileName::GetPathSeparator();
    }

    // Build normalised media directory list (with trailing separators)
    std::vector<wxString> normMediaDirs;
    for (const auto& md : mediaDirs) {
        if (md == showDirectory) continue;
        wxString d(md);
        if (!d.IsEmpty() && d.Last() != wxFileName::GetPathSeparator())
            d += wxFileName::GetPathSeparator();
        normMediaDirs.push_back(d);
    }

    // Returns the display path for a file: strips show/media dir prefix when present.
    // For relative cache keys, uses the resolved absolute path from the entry to
    // determine which directory the file belongs to.
    auto displayPath = [&](const std::string& cachePath, const std::string& resolvedPath) -> wxString {
        wxString wx(cachePath);
        // Try stripping show dir prefix from the cache key
        if (!showDir.IsEmpty() && wx.StartsWith(showDir)) {
            return wx.Mid(showDir.Length());
        }
        // Try stripping media dir prefixes from the cache key
        for (const auto& md : normMediaDirs) {
            if (wx.StartsWith(md))
                return wx.Mid(md.Length());
        }
        // For relative paths, check the resolved absolute path to determine
        // which directory the file actually lives in.
        if (!resolvedPath.empty()) {
            wxString resolved(resolvedPath);
            if (!showDir.IsEmpty() && resolved.StartsWith(showDir)) {
                return wx;  // relative to show dir — return as-is
            }
            for (const auto& md : normMediaDirs) {
                if (resolved.StartsWith(md)) {
                    // Prefix with the media dir so it groups separately
                    return wxString(md) + wx;
                }
            }
        }
        return wx;
    };

    auto allMedia = media->GetAllMediaPaths();

    // Filter if type specified
    if (filterType.has_value()) {
        std::erase_if(allMedia, [&](const auto& p) { return p.second != *filterType; });
    }

    // Group entries first by MediaType, then by directory within each type
    // Use an ordered map so types appear in a consistent order
    std::map<MediaType, std::vector<std::pair<std::string, MediaType>>> byType;
    for (const auto& [path, type] : allMedia) {
        byType[type].push_back({path, type});
    }

    // When filtering by a single type, skip the top-level type grouping
    // and go directly to directory grouping (single level).
    bool singleTypeMode = filterType.has_value() && byType.size() <= 1;

    for (const auto& [type, entries] : byType) {
        // In single-type mode, directory groups are top-level; otherwise
        // they nest under a type group node.
        MediaNode* typeParent = nullptr;
        std::shared_ptr<MediaNode> typeGroup;
        if (!singleTypeMode) {
            // Create a top-level group node for this media type
            typeGroup = std::make_shared<MediaNode>();
            typeGroup->isGroup = true;
            typeGroup->label = MediaTypeName(type);
            typeGroup->parent = nullptr;
            typeParent = typeGroup.get();
        }

        // Sub-group by directory within this type
        std::map<wxString, std::shared_ptr<MediaNode>> dirGroups;

        for (const auto& [path, mtype] : entries) {
            std::string resolvedPath;
            auto node = std::make_shared<MediaNode>();
            node->isGroup = false;
            node->filePath = path;
            node->mediaType = mtype;

            // Display label: just the filename portion
            wxFileName fnName(path);
            node->label = fnName.GetFullName().ToStdString();

            if (type == MediaType::Image) {
                auto entry = media->GetImage(path);
                if (!entry) continue;
                resolvedPath = entry->GetFilePath();
                node->canLoad = entry->IsOk();
                wxSize sz(entry->GetImageWidth(), entry->GetImageHeight());
                node->sizeStr = (sz.x > 0) ? wxString::Format("%dx%d", sz.x, sz.y) : "?";
                node->framesStr = (entry->GetImageCount() > 1)
                    ? wxString::Format("%d", entry->GetImageCount()) : "-";
                node->statusStr = entry->IsEmbedded() ? "Embedded" : "External";
                if (!node->canLoad)
                    node->statusStr += " (broken)";
            } else if (type == MediaType::Video) {
                auto entry = media->GetVideo(path);
                if (!entry) continue;
                resolvedPath = entry->GetFilePath();
                node->canLoad = true;
                node->sizeStr = "-";
                node->framesStr = "-";
                node->statusStr = "External";
            } else {
                // SVG, Shader, TextFile, BinaryFile — all share the base MediaCacheEntry interface
                MediaCacheEntry* baseEntry = nullptr;
                std::shared_ptr<MediaCacheEntry> holder;
                if (type == MediaType::SVG) {
                    auto e = media->GetSVG(path);
                    holder = e; baseEntry = e.get();
                } else if (type == MediaType::Shader) {
                    auto e = media->GetShader(path);
                    holder = e; baseEntry = e.get();
                } else if (type == MediaType::TextFile) {
                    auto e = media->GetTextFile(path);
                    holder = e; baseEntry = e.get();
                } else if (type == MediaType::BinaryFile) {
                    auto e = media->GetBinaryFile(path);
                    holder = e; baseEntry = e.get();
                }
                if (!baseEntry) continue;
                resolvedPath = baseEntry->GetFilePath();
                node->canLoad = true;
                node->sizeStr = "-";
                node->framesStr = "-";
                node->statusStr = baseEntry->IsEmbedded() ? "Embedded" : "External";
            }

            wxFileName fn(displayPath(path, resolvedPath));
            wxString dir = fn.GetPath();
            if (dir.IsEmpty()) dir = "(show directory)";

            auto it = dirGroups.find(dir);
            if (it == dirGroups.end()) {
                auto grp = std::make_shared<MediaNode>();
                grp->isGroup = true;
                grp->label = dir.ToStdString();
                grp->parent = typeParent;
                dirGroups[dir] = grp;
                if (singleTypeMode) {
                    _groups.push_back(grp);
                } else {
                    typeGroup->children.push_back(grp);
                }
            }

            node->parent = dirGroups[dir].get();
            dirGroups[dir]->children.push_back(node);
        }

        if (!singleTypeMode && typeGroup && !typeGroup->children.empty()) {
            _groups.push_back(typeGroup);
        }
    }
}

wxDataViewItem MediaViewModel::FindItem(const std::string& filePath) const
{
    for (const auto& typeGrp : _groups) {
        for (const auto& dirGrp : typeGrp->children) {
            if (!dirGrp->isGroup) {
                // In case a leaf is directly under a type group (shouldn't happen, but safe)
                if (dirGrp->filePath == filePath)
                    return wxDataViewItem(dirGrp.get());
                continue;
            }
            for (const auto& child : dirGrp->children) {
                if (child->filePath == filePath)
                    return wxDataViewItem(child.get());
            }
        }
    }
    return wxDataViewItem(nullptr);
}

std::string MediaViewModel::GetFilePath(const wxDataViewItem& item) const
{
    if (!item.IsOk()) return {};
    MediaNode* node = static_cast<MediaNode*>(item.GetID());
    if (!node || node->isGroup) return {};
    return node->filePath;
}

bool MediaViewModel::IsGroup(const wxDataViewItem& item) const
{
    if (!item.IsOk()) return false;
    MediaNode* node = static_cast<MediaNode*>(item.GetID());
    return node && node->isGroup;
}

void MediaViewModel::GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const
{
    if (!item.IsOk()) { variant = wxString(); return; }
    MediaNode* node = static_cast<MediaNode*>(item.GetID());
    if (!node) { variant = wxString(); return; }

    if (node->isGroup) {
        variant = (col == COL_NAME) ? wxString(node->label) : wxString();
        return;
    }

    switch (col) {
        case COL_NAME:   variant = wxString(node->label);    break;
        case COL_SIZE:   variant = node->sizeStr;             break;
        case COL_FRAMES: variant = node->framesStr;           break;
        case COL_STATUS: variant = node->statusStr;           break;
        default:         variant = wxString();                break;
    }
}

bool MediaViewModel::GetAttr(const wxDataViewItem& item, unsigned int col, wxDataViewItemAttr& attr) const
{
    if (!item.IsOk()) return false;
    MediaNode* node = static_cast<MediaNode*>(item.GetID());
    if (!node || node->isGroup) return false;
    if (!node->canLoad) {
        attr.SetColour(*wxRED);
        attr.SetBold(true);
        return true;
    }
    return false;
}

wxDataViewItem MediaViewModel::GetParent(const wxDataViewItem& item) const
{
    if (!item.IsOk()) return wxDataViewItem(nullptr);
    MediaNode* node = static_cast<MediaNode*>(item.GetID());
    if (!node || node->isGroup) return wxDataViewItem(nullptr);
    return wxDataViewItem(node->parent);
}

bool MediaViewModel::IsContainer(const wxDataViewItem& item) const
{
    // Invalid item = root
    if (!item.IsOk()) return true;
    MediaNode* node = static_cast<MediaNode*>(item.GetID());
    return node && node->isGroup;
}

unsigned int MediaViewModel::GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const
{
    if (!item.IsOk()) {
        // Root level: return group nodes
        for (const auto& grp : _groups) {
            children.Add(wxDataViewItem(grp.get()));
        }
        return (unsigned int)_groups.size();
    }

    MediaNode* node = static_cast<MediaNode*>(item.GetID());
    if (!node || !node->isGroup) return 0;

    for (const auto& child : node->children) {
        children.Add(wxDataViewItem(child.get()));
    }
    return (unsigned int)node->children.size();
}

// ---------------------------------------------------------------------------
// ManageMediaPanel
// ---------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(ManageMediaPanel, wxPanel)
wxEND_EVENT_TABLE()

ManageMediaPanel::ManageMediaPanel(wxWindow* parent, SequenceMedia* sequenceMedia,
                                   SequenceElements* sequenceElements,
                                   const std::string& showDirectory,
                                   xLightsFrame* xlFrame,
                                   bool singleSelect,
                                   std::optional<MediaType> filterType,
                                   wxWindowID id)
    : wxPanel(parent, id)
    , _sequenceMedia(sequenceMedia)
    , _sequenceElements(sequenceElements)
    , _xlFrame(xlFrame)
    , _showDirectory(showDirectory)
    , _filterType(filterType)
    , _model(new MediaViewModel())
{
    wxFlexGridSizer* mainSizer = new wxFlexGridSizer(1, 2, 0, 0);
    mainSizer->AddGrowableCol(0);
    mainSizer->AddGrowableRow(0);

    // Left side: tree with columns
    _mediaTree = new wxDataViewCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                    (singleSelect ? wxDV_SINGLE : wxDV_MULTIPLE) | wxBORDER_SUNKEN);
    _mediaTree->AssociateModel(_model.get());

    _mediaTree->AppendTextColumn("File",   COL_NAME,   wxDATAVIEW_CELL_INERT, 300, wxALIGN_LEFT,
                                 wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
    _mediaTree->AppendTextColumn("Size",   COL_SIZE,   wxDATAVIEW_CELL_INERT,  80, wxALIGN_LEFT,
                                 wxDATAVIEW_COL_RESIZABLE);
    _mediaTree->AppendTextColumn("Frames", COL_FRAMES, wxDATAVIEW_CELL_INERT,  55, wxALIGN_LEFT,
                                 wxDATAVIEW_COL_RESIZABLE);
    // On macOS the native control expands the last column; giving it -1 (auto)
    // lets it take remaining space naturally rather than being clipped.
    _mediaTree->AppendTextColumn("Status", COL_STATUS, wxDATAVIEW_CELL_INERT,  -1, wxALIGN_LEFT,
                                 wxDATAVIEW_COL_RESIZABLE);
    // Minimum width: File(300) + Size(80) + Frames(55) + Status(~80) + scrollbar+padding(~50)
    _mediaTree->SetMinSize(wxSize(600, 300));

    mainSizer->Add(_mediaTree, 1, wxALL | wxEXPAND, 5);

    // Right side: preview and buttons
    wxFlexGridSizer* rightSizer = new wxFlexGridSizer(0, 1, 0, 0);
    rightSizer->AddGrowableCol(0);

    _preview = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize(150, 150));
    _preview->SetMinSize(wxSize(150, 150));
    _preview->SetMaxSize(wxSize(150, 150));
    rightSizer->Add(_preview, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

    _infoLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(150, -1),
                                  wxST_NO_AUTORESIZE);
    _infoLabel->SetMinSize(wxSize(150, -1));
    rightSizer->Add(_infoLabel, 0, wxALL | wxEXPAND, 5);

    rightSizer->Add(0, 10, 0);

    _addButton = new wxButton(this, wxID_ANY, "Add...");
    _addButton->SetToolTip("Add a media file to the sequence");
    _addButton->Show(!singleSelect);  // Hidden in SelectMediaDialog (it has its own "Add from disk...")
    rightSizer->Add(_addButton, 0, wxALL | wxEXPAND, 5);

    _aiGenerateButton = new wxButton(this, wxID_ANY, "AI Generate...");
    _aiGenerateButton->SetToolTip("Generate an image using an AI service and add it to the sequence media");
    rightSizer->Add(_aiGenerateButton, 0, wxALL | wxEXPAND, 5);

    rightSizer->Add(0, 10, 0);

    _renameButton = new wxButton(this, wxID_ANY, "Rename...");
    _renameButton->SetToolTip("Rename the selected embedded image and update all sequence references");
    _renameButton->Disable();
    rightSizer->Add(_renameButton, 0, wxALL | wxEXPAND, 5);

    rightSizer->Add(0, 10, 0);

    _embedButton = new wxButton(this, wxID_ANY, "Embed");
    _embedButton->SetToolTip("Store this media inside the sequence file");
    _embedButton->Disable();
    rightSizer->Add(_embedButton, 0, wxALL | wxEXPAND, 5);

    _extractButton = new wxButton(this, wxID_ANY, "Extract");
    _extractButton->SetToolTip("Reference this media from an external file path");
    _extractButton->Disable();
    rightSizer->Add(_extractButton, 0, wxALL | wxEXPAND, 5);

    rightSizer->Add(0, 10, 0);

    _embedAllButton = new wxButton(this, wxID_ANY, "Embed All");
    _embedAllButton->SetToolTip("Store all media inside the sequence file");
    rightSizer->Add(_embedAllButton, 0, wxALL | wxEXPAND, 5);

    _extractAllButton = new wxButton(this, wxID_ANY, "Extract All");
    _extractAllButton->SetToolTip("Reference all media from external file paths");
    rightSizer->Add(_extractAllButton, 0, wxALL | wxEXPAND, 5);

    rightSizer->Add(0, 10, 0);

    _removeButton = new wxButton(this, wxID_ANY, "Remove");
    _removeButton->SetToolTip("Remove the selected embedded media from the sequence");
    _removeButton->Disable();
    _removeButton->Show(!singleSelect);  // Hidden in SelectMediaDialog
    rightSizer->Add(_removeButton, 0, wxALL | wxEXPAND, 5);

    mainSizer->Add(rightSizer, 0, wxALL | wxEXPAND, 0);

    SetSizer(mainSizer);

    _mediaTree->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &ManageMediaPanel::OnTreeItemSelected, this);
    _mediaTree->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &ManageMediaPanel::OnTreeContextMenu, this);
    _addButton->Bind(wxEVT_BUTTON, &ManageMediaPanel::OnAddButtonClick, this);
    _aiGenerateButton->Bind(wxEVT_BUTTON, &ManageMediaPanel::OnAIGenerateButtonClick, this);
    _renameButton->Bind(wxEVT_BUTTON, &ManageMediaPanel::OnRenameButtonClick, this);

    // Bind mouse motion on the inner window so HitTest works correctly on macOS
    if (wxWindow* inner = _mediaTree->GetMainWindow()) {
        inner->Bind(wxEVT_MOTION, &ManageMediaPanel::OnTreeMouseMotion, this);
    }
    _embedButton->Bind(wxEVT_BUTTON, &ManageMediaPanel::OnEmbedButtonClick, this);
    _extractButton->Bind(wxEVT_BUTTON, &ManageMediaPanel::OnExtractButtonClick, this);
    _embedAllButton->Bind(wxEVT_BUTTON, &ManageMediaPanel::OnEmbedAllButtonClick, this);
    _extractAllButton->Bind(wxEVT_BUTTON, &ManageMediaPanel::OnExtractAllButtonClick, this);
    _removeButton->Bind(wxEVT_BUTTON, &ManageMediaPanel::OnRemoveButtonClick, this);

    Populate();
}

ManageMediaPanel::~ManageMediaPanel()
{
}

void ManageMediaPanel::Populate(const std::string& selectPath)
{
    _preview->SetBitmap(wxNullBitmap);
    _preview->Refresh();
    _infoLabel->SetLabel(wxEmptyString);
    _embedButton->Disable();
    _extractButton->Disable();
    _removeButton->Disable();

    // Rebuild the model
    _model->Clear();
    _model->Cleared();   // notify the control the whole model changed

    // AI generate only available for image type
    bool hasAI = _xlFrame && !_xlFrame->GetAIServices(aiType::TYPE::IMAGES).empty();
    _aiGenerateButton->Show(!_filterType.has_value() || *_filterType == MediaType::Image);
    _aiGenerateButton->Enable(hasAI);

    // Hide embed/extract buttons for non-embeddable types
    bool typeIsEmbeddable = !_filterType.has_value() ||
        (*_filterType != MediaType::Video && *_filterType != MediaType::BinaryFile);
    _embedButton->Show(typeIsEmbeddable);
    _extractButton->Show(typeIsEmbeddable);
    _embedAllButton->Show(typeIsEmbeddable);
    _extractAllButton->Show(typeIsEmbeddable);
    _renameButton->Show(typeIsEmbeddable);

    if (_sequenceMedia == nullptr) {
        _addButton->Disable();
        _renameButton->Disable();
        _embedAllButton->Disable();
        _extractAllButton->Disable();
        return;
    }
    _addButton->Enable();

    std::list<std::string> mediaDirs;
    if (_xlFrame) mediaDirs = _xlFrame->GetMediaFolders();
    _model->Rebuild(_sequenceMedia, _showDirectory, mediaDirs, _filterType);
    _model->Cleared();

    // Expand all top-level and second-level groups
    wxDataViewItemArray groups;
    _model->GetChildren(wxDataViewItem(nullptr), groups);
    for (const auto& grp : groups) {
        _mediaTree->Expand(grp);
        wxDataViewItemArray subGroups;
        _model->GetChildren(grp, subGroups);
        for (const auto& sub : subGroups) {
            if (_model->IsContainer(sub))
                _mediaTree->Expand(sub);
        }
    }

    // Determine Embed All / Extract All availability (only for filtered type if set)
    auto allPaths = _sequenceMedia->GetAllMediaPaths();
    bool hasEmbeddable = false, hasExtractable = false;
    for (const auto& [path, type] : allPaths) {
        if (_filterType.has_value() && type != *_filterType) continue;
        if (type == MediaType::Image) {
            auto entry = _sequenceMedia->GetImage(path);
            if (!entry) continue;
            if (!entry->IsEmbedded() && entry->IsEmbeddable()) hasEmbeddable = true;
            if (entry->IsEmbedded()) hasExtractable = true;
        } else if (type == MediaType::Video) {
            // Videos are not embeddable
        } else {
            // For other types, check via the appropriate accessor
            MediaCacheEntry* baseEntry = nullptr;
            std::shared_ptr<MediaCacheEntry> holder;
            if (type == MediaType::SVG) { auto e = _sequenceMedia->GetSVG(path); holder = e; baseEntry = e.get(); }
            else if (type == MediaType::Shader) { auto e = _sequenceMedia->GetShader(path); holder = e; baseEntry = e.get(); }
            else if (type == MediaType::TextFile) { auto e = _sequenceMedia->GetTextFile(path); holder = e; baseEntry = e.get(); }
            else if (type == MediaType::BinaryFile) { auto e = _sequenceMedia->GetBinaryFile(path); holder = e; baseEntry = e.get(); }
            if (!baseEntry) continue;
            if (!baseEntry->IsEmbedded() && baseEntry->IsEmbeddable()) hasEmbeddable = true;
            if (baseEntry->IsEmbedded()) hasExtractable = true;
        }
    }
    _embedAllButton->Enable(hasEmbeddable);
    _extractAllButton->Enable(hasExtractable);

    // Re-select and scroll to the requested path if provided
    if (!selectPath.empty()) {
        wxDataViewItem item = _model->FindItem(selectPath);
        if (item.IsOk()) {
            _mediaTree->Select(item);
            _mediaTree->EnsureVisible(item);
            MediaNode* node = static_cast<MediaNode*>(item.GetID());
            MediaType selType = (node && !node->isGroup) ? node->mediaType : MediaType::Image;
            UpdatePreview(selectPath, selType);
            UpdateButtons();
            wxDataViewEvent evt(wxEVT_DATAVIEW_SELECTION_CHANGED, _mediaTree, item);
            wxPostEvent(_mediaTree, evt);
        }
    }
}

std::vector<std::string> ManageMediaPanel::GetSelectedPaths() const
{
    wxDataViewItemArray items;
    _mediaTree->GetSelections(items);
    std::vector<std::string> paths;
    for (const auto& item : items) {
        std::string p = _model->GetFilePath(item);
        if (!p.empty())
            paths.push_back(p);
    }
    return paths;
}

void ManageMediaPanel::OnTreeItemSelected(wxDataViewEvent& event)
{
    std::vector<std::string> paths = GetSelectedPaths();
    if (paths.empty()) {
        _preview->SetBitmap(wxNullBitmap);
        _preview->Refresh();
        _infoLabel->SetLabel(wxEmptyString);
        _embedButton->Disable();
        _extractButton->Disable();
        _removeButton->Disable();
        return;
    }

    // Get the media type from the selected tree node
    wxDataViewItemArray items;
    _mediaTree->GetSelections(items);
    MediaNode* node = items.empty() ? nullptr : static_cast<MediaNode*>(items[0].GetID());
    MediaType selectedType = (node && !node->isGroup) ? node->mediaType : MediaType::Image;

    UpdatePreview(paths[0], selectedType);
    UpdateButtons();
}

void ManageMediaPanel::UpdateButtons()
{
    std::vector<std::string> paths = GetSelectedPaths();
    if (paths.empty() || _sequenceMedia == nullptr) {
        _renameButton->Disable();
        _embedButton->Disable();
        _extractButton->Disable();
        _removeButton->Disable();
        return;
    }
    // Build a lookup of path -> MediaType from the model
    auto allMedia = _sequenceMedia->GetAllMediaPaths();
    std::map<std::string, MediaType> mediaTypeMap;
    for (const auto& [p, t] : allMedia) {
        mediaTypeMap[p] = t;
    }

    bool canEmbed = false, canExtract = false, canRename = false, canRemove = false;
    for (const auto& path : paths) {
        auto typeIt = mediaTypeMap.find(path);
        if (typeIt == mediaTypeMap.end()) continue;
        MediaType mtype = typeIt->second;

        bool isEmbedded = false, isEmbeddable = false;
        if (mtype == MediaType::Image) {
            auto entry = _sequenceMedia->GetImage(path);
            if (!entry) continue;
            isEmbedded = entry->IsEmbedded();
            isEmbeddable = entry->IsEmbeddable();
        } else if (mtype == MediaType::Video) {
            auto entry = _sequenceMedia->GetVideo(path);
            if (!entry) continue;
            isEmbedded = false;
            isEmbeddable = false;
        } else {
            std::shared_ptr<MediaCacheEntry> holder;
            if (mtype == MediaType::SVG) holder = _sequenceMedia->GetSVG(path);
            else if (mtype == MediaType::Shader) holder = _sequenceMedia->GetShader(path);
            else if (mtype == MediaType::TextFile) holder = _sequenceMedia->GetTextFile(path);
            else if (mtype == MediaType::BinaryFile) holder = _sequenceMedia->GetBinaryFile(path);
            if (!holder) continue;
            isEmbedded = holder->IsEmbedded();
            isEmbeddable = holder->IsEmbeddable();
        }

        if (!isEmbedded && isEmbeddable) canEmbed = true;
        if (isEmbedded) { canExtract = true; canRemove = true; }
    }
    // Rename only for a single embedded image selection
    if (paths.size() == 1 && _sequenceMedia->HasImage(paths[0])) {
        auto entry = _sequenceMedia->GetImage(paths[0]);
        canRename = entry && entry->IsEmbedded();
    }
    _renameButton->Enable(canRename);
    _embedButton->Enable(canEmbed);
    _extractButton->Enable(canExtract);
    _removeButton->Enable(canRemove);
    // AI Generate only makes sense for images
    bool hasAI = _xlFrame && !_xlFrame->GetAIServices(aiType::TYPE::IMAGES).empty();
    _aiGenerateButton->Enable(hasAI);
}

void ManageMediaPanel::UpdatePreview(const std::string& filepath, MediaType type)
{
    auto refreshPreview = [this](const wxBitmap& bmp) {
        _preview->SetBitmap(bmp);
        _preview->InvalidateBestSize();
        _preview->Refresh();
        _preview->Update();
    };

    if (_sequenceMedia == nullptr) return;

    // Clear previous state
    _infoLabel->SetLabel(wxEmptyString);
    refreshPreview(wxNullBitmap);

    // Image preview
    if (type == MediaType::Image && _sequenceMedia->HasImage(filepath)) {
        auto entry = _sequenceMedia->GetImage(filepath);
        if (!entry || !entry->IsOk()) {
            refreshPreview(wxNullBitmap);
            _infoLabel->SetLabel(wxEmptyString);
            return;
        }

        wxSize sz(entry->GetImageWidth(), entry->GetImageHeight());
        wxString info = wxString::Format("%dx%d", sz.x, sz.y);
        if (entry->GetImageCount() > 1)
            info += wxString::Format("\n%d frames", entry->GetImageCount());
        if (entry->IsEmbedded()) {
            info += "\nEmbedded";
        } else {
            info += "\nExternal";
            wxFileName fn(filepath);
            if (fn.FileExists())
                info += wxString::Format("\n%.1f KB", fn.GetSize().ToDouble() / 1024.0);
            else
                info += "\n(file missing)";
        }
        _infoLabel->SetLabel(info);

        auto img = entry->GetFrame(0, false);
        if (img && img->IsOk()) {
            double scaleFactor = GetContentScaleFactor();
            double maxPx = 150.0 * scaleFactor;
            double scale = std::min(maxPx / img->GetWidth(), maxPx / img->GetHeight());
            int w = std::max(1, (int)(img->GetWidth() * scale));
            int h = std::max(1, (int)(img->GetHeight() * scale));
            wxBitmap bmp(xlImageToWxImage(*entry->GetScaledImage(0, w, h, false)));
            bmp.SetScaleFactor(scaleFactor);
            refreshPreview(bmp);
        } else {
            refreshPreview(wxNullBitmap);
        }
        return;
    }

    // Non-image media types
    auto allMedia = _sequenceMedia->GetAllMediaPaths();
    for (const auto& [p, t] : allMedia) {
        if (p != filepath) continue;
        wxString info = wxString(MediaTypeName(t));
        MediaCacheEntry* baseEntry = nullptr;
        std::shared_ptr<MediaCacheEntry> holder;
        switch (t) {
            case MediaType::SVG: { auto e = _sequenceMedia->GetSVG(p); holder = e; baseEntry = e.get(); break; }
            case MediaType::Shader: { auto e = _sequenceMedia->GetShader(p); holder = e; baseEntry = e.get(); break; }
            case MediaType::TextFile: { auto e = _sequenceMedia->GetTextFile(p); holder = e; baseEntry = e.get(); break; }
            case MediaType::BinaryFile: { auto e = _sequenceMedia->GetBinaryFile(p); holder = e; baseEntry = e.get(); break; }
            case MediaType::Video: { auto e = _sequenceMedia->GetVideo(p); holder = e; baseEntry = e.get(); break; }
            default: break;
        }
        if (baseEntry) {
            info += baseEntry->IsEmbedded() ? "\nEmbedded" : "\nExternal";
        }
        _infoLabel->SetLabel(info);

        // Try to show thumbnails for supported types
        double scaleFactor = GetContentScaleFactor();
        int maxPx = (int)(150.0 * scaleFactor);
        std::shared_ptr<xlImage> thumb;

        if (t == MediaType::SVG) {
            auto svgEntry = _sequenceMedia->GetSVG(p);
            if (svgEntry && !svgEntry->GetSVGContent().empty()) {
                thumb = svgEntry->GetThumbnail(maxPx, maxPx);
            }
        } else if (t == MediaType::Video) {
            auto vidEntry = _sequenceMedia->GetVideo(p);
            if (vidEntry && !vidEntry->GetResolvedPath().empty()) {
                thumb = vidEntry->GetThumbnail(maxPx, maxPx);
                // Add video dimensions and duration to info
                if (thumb && thumb->IsOk()) {
                    long duration = VideoReader::GetVideoLength(vidEntry->GetResolvedPath());
                    info += wxString::Format("\n%dx%d", thumb->GetWidth(), thumb->GetHeight());
                    if (duration > 0) {
                        info += wxString::Format("\n%.1fs", duration / 1000.0);
                    }
                    _infoLabel->SetLabel(info);
                }
            }
        }
        // TODO: Shader preview via effect rendering

        if (thumb && thumb->IsOk()) {
            wxBitmap bmp(xlImageToWxImage(*thumb));
            bmp.SetScaleFactor(scaleFactor);
            refreshPreview(bmp);
            return;
        }
        refreshPreview(wxNullBitmap);
        return;
    }
    refreshPreview(wxNullBitmap);
    _infoLabel->SetLabel(wxEmptyString);
}

std::string ManageMediaPanel::StrippedPath(const std::string& fullPath) const
{
    if (_showDirectory.empty()) return {};
    wxString showDir = _showDirectory;
    if (showDir.Last() != wxFileName::GetPathSeparator())
        showDir += wxFileName::GetPathSeparator();
    wxString wx = fullPath;
    if (wx.StartsWith(showDir))
        return wx.Mid(showDir.Length()).ToStdString();
    return {};
}

std::string ManageMediaPanel::EmbedWithRename(const std::string& fullPath)
{
    // Check if this is an image or another media type
    bool isImage = _sequenceMedia->HasImage(fullPath);

    // Compute the new (stripped) path. If it's the same, just embed as-is.
    std::string newPath = StrippedPath(fullPath);
    if (newPath.empty() || newPath == fullPath) {
        if (isImage)
            _sequenceMedia->EmbedImage(fullPath);
        else
            _sequenceMedia->EmbedMedia(fullPath);
        return fullPath;
    }

    // Rename in the cache first (rename is image-only for now)
    if (isImage) {
        if (!_sequenceMedia->RenameImage(fullPath, newPath)) {
            // Rename failed (e.g. newPath already exists) — embed under old name
            _sequenceMedia->EmbedImage(fullPath);
            return fullPath;
        }
    } else {
        // For non-image types, just embed under original name
        _sequenceMedia->EmbedMedia(fullPath);
        return fullPath;
    }

    // Update every effect in the sequence that references fullPath -> newPath
    if (_sequenceElements != nullptr) {
        for (size_t i = 0; i < _sequenceElements->GetElementCount(); ++i) {
            Element* e = _sequenceElements->GetElement(i);
            if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
            ModelElement* model = dynamic_cast<ModelElement*>(e);
            if (!model) continue;

            // Helper lambda to scan one EffectLayer
            auto scanLayer = [&](EffectLayer* layer) {
                for (size_t k = 0; k < layer->GetEffectCount(); ++k) {
                    Effect* eff = layer->GetEffect(k);
                    const SettingsMap& settings = eff->GetSettings();
                    // Collect keys first to avoid modifying map during iteration
                    std::vector<std::string> keysToUpdate;
                    for (auto it = settings.begin(); it != settings.end(); ++it) {
                        if (it->second == fullPath) {
                            keysToUpdate.push_back(it->first);
                        }
                    }
                    for (const auto& key : keysToUpdate) {
                        eff->SetSetting(key, newPath);
                    }
                }
            };

            for (size_t j = 0; j < model->GetEffectLayerCount(); ++j)
                scanLayer(model->GetEffectLayer(j));

            for (size_t j = 0; j < model->GetSubModelAndStrandCount(); ++j) {
                SubModelElement* sub = model->GetSubModel(j);
                for (size_t l = 0; l < sub->GetEffectLayerCount(); ++l)
                    scanLayer(sub->GetEffectLayer(l));
                if (sub->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                    StrandElement* strand = dynamic_cast<StrandElement*>(sub);
                    if (strand) {
                        for (int k = 0; k < strand->GetNodeLayerCount(); ++k)
                            scanLayer(strand->GetNodeLayer(k));
                    }
                }
            }
        }
    }

    _sequenceMedia->EmbedImage(newPath);
    return newPath;
}

void ManageMediaPanel::OnTreeMouseMotion(wxMouseEvent& event)
{
    event.Skip();   // allow normal processing (selection hover, etc.)

    wxWindow* inner = _mediaTree->GetMainWindow();
    if (!inner) return;

    wxDataViewItem item;
    wxDataViewColumn* col = nullptr;
    _mediaTree->HitTest(event.GetPosition(), item, col);

    if (item.IsOk() && !_model->IsGroup(item)) {
        std::string path = _model->GetFilePath(item);
        if (!path.empty() && _sequenceMedia && _sequenceMedia->HasMedia(path)) {
            // Show full path tooltip only for external media
            inner->SetToolTip(wxString(path));
            return;
        }
    }
    // No external media under cursor — clear tooltip
    inner->UnsetToolTip();
}

std::string ManageMediaPanel::ExtractWithRename(const std::string& fullPath)
{
    if (_sequenceMedia == nullptr) return {};

    // Build a suggested save path: if the key is a relative (stripped) path,
    // expand it under the show directory; otherwise use it as-is.
    wxString suggested;
    {
        wxFileName fn(fullPath);
        if (fn.IsAbsolute()) {
            suggested = fullPath;
        } else if (!_showDirectory.empty()) {
            wxFileName base(_showDirectory, "");
            base.AppendDir(wxEmptyString); // ensure trailing sep handled
            suggested = _showDirectory;
            if (suggested.Last() != wxFileName::GetPathSeparator())
                suggested += wxFileName::GetPathSeparator();
            suggested += fullPath;
        } else {
            suggested = fullPath;
        }
    }

    // Determine wildcard based on extension
    wxFileName fn(suggested);
    wxString ext = fn.GetExt().Lower();
    wxString wildcard;
    if (ext == "gif")
        wildcard = "GIF files (*.gif)|*.gif|All files (*.*)|*.*";
    else if (ext == "jpg" || ext == "jpeg")
        wildcard = "JPEG files (*.jpg;*.jpeg)|*.jpg;*.jpeg|All files (*.*)|*.*";
    else
        wildcard = "PNG files (*.png)|*.png|All files (*.*)|*.*";

    // Loop until the user picks a path inside the show/media folders or cancels.
    std::string newPath;
    for (;;) {
        wxFileDialog dlg(this, "Save Image As", fn.GetPath(), fn.GetFullName(),
                         wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dlg.ShowModal() != wxID_OK) return {};

        newPath = dlg.GetPath().ToStdString();
        bool outside = _xlFrame ? !_xlFrame->IsInShowOrMediaFolder(newPath)
                                : (!_showDirectory.empty() &&
                                   !wxString(newPath).StartsWith(wxString(_showDirectory)));
        if (!outside) break;

        int resp = wxMessageBox(
            "The selected location is outside the show/media folder(s).\n"
            "Please choose a location inside the show or a media folder.",
            "Invalid Location", wxOK | wxICON_WARNING, this);
        (void)resp;
        // Reset the dialog's default directory to the show directory for the retry
        fn = wxFileName(wxString(!_showDirectory.empty() ? _showDirectory : newPath),
                        fn.GetFullName());
    }
    ObtainAccessToURL(newPath);

    // Write data to disk and rename cache key
    if (!_sequenceMedia->ExtractImageToFile(fullPath, newPath)) {
        wxMessageBox("Failed to save image to:\n" + newPath,
                     "Extract Failed", wxICON_ERROR | wxOK, this);
        return {};
    }

    // Convert absolute path to relative (relative to show/media folder)
    std::string finalPath = newPath;
    if (_xlFrame) {
        std::string rel = _xlFrame->MakeRelativePath(newPath);
        if (!rel.empty()) {
            // Rename the cache key from absolute to relative
            _sequenceMedia->RenameImage(newPath, rel);
            finalPath = rel;
        }
    }

    // Update every effect in the sequence that referenced fullPath -> finalPath
    if (_sequenceElements != nullptr && finalPath != fullPath) {
        auto scanLayer = [&](EffectLayer* layer) {
            for (size_t k = 0; k < layer->GetEffectCount(); ++k) {
                Effect* eff = layer->GetEffect(k);
                const SettingsMap& settings = eff->GetSettings();
                std::vector<std::string> keysToUpdate;
                for (auto it = settings.begin(); it != settings.end(); ++it) {
                    if (it->second == fullPath)
                        keysToUpdate.push_back(it->first);
                }
                for (const auto& key : keysToUpdate)
                    eff->SetSetting(key, finalPath);
            }
        };

        for (size_t i = 0; i < _sequenceElements->GetElementCount(); ++i) {
            Element* e = _sequenceElements->GetElement(i);
            if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
            ModelElement* model = dynamic_cast<ModelElement*>(e);
            if (!model) continue;

            for (size_t j = 0; j < model->GetEffectLayerCount(); ++j)
                scanLayer(model->GetEffectLayer(j));

            for (size_t j = 0; j < model->GetSubModelAndStrandCount(); ++j) {
                SubModelElement* sub = model->GetSubModel(j);
                for (size_t l = 0; l < sub->GetEffectLayerCount(); ++l)
                    scanLayer(sub->GetEffectLayer(l));
                if (sub->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                    StrandElement* strand = dynamic_cast<StrandElement*>(sub);
                    if (strand) {
                        for (int k = 0; k < strand->GetNodeLayerCount(); ++k)
                            scanLayer(strand->GetNodeLayer(k));
                    }
                }
            }
        }
    }

    return finalPath;
}

void ManageMediaPanel::OnTreeContextMenu(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();
    if (!item.IsOk() || _model->IsGroup(item)) return;

    std::string path = _model->GetFilePath(item);
    if (path.empty() || _sequenceMedia == nullptr) return;

    if (!_sequenceMedia->HasImage(path)) return;  // context menu is image-only for now
    auto entry = _sequenceMedia->GetImage(path);
    if (!entry || entry->IsOk()) return;  // only show menu for broken images

    // Count total broken images to decide whether to offer bulk find
    int brokenCount = 0;
    for (const auto& p : _sequenceMedia->GetImagePaths()) {
        auto e = _sequenceMedia->GetImage(p);
        if (e && !e->IsOk()) ++brokenCount;
    }

    wxMenu menu;
    menu.Append(wxID_ANY, "Re-select Image...");
    menu.Bind(wxEVT_MENU, [this, path](wxCommandEvent&) {
        OnReSelectImage(path);
    }, menu.FindItemByPosition(0)->GetId());

    if (brokenCount > 1) {
        wxMenuItem* bulkItem = menu.Append(wxID_ANY, "Bulk Find Images...");
        menu.Bind(wxEVT_MENU, [this](wxCommandEvent&) {
            OnBulkFindImages();
        }, bulkItem->GetId());
    }

    PopupMenu(&menu);
}

void ManageMediaPanel::OnReSelectImage(const std::string& oldPath)
{
    if (_sequenceMedia == nullptr) return;

    // Default the file picker to the show directory (or the old path's dir if absolute)
    wxString defaultDir;
    {
        wxFileName fn(oldPath);
        if (fn.IsAbsolute() && fn.DirExists())
            defaultDir = fn.GetPath();
        else if (!_showDirectory.empty())
            defaultDir = _showDirectory;
    }
    wxFileName fnOld(oldPath);

    // --- Step 1: let the user pick a replacement file ---
    std::string pickedPath;
    {
        wxFileDialog dlg(this, "Re-select Image: " + wxString(oldPath),
                         defaultDir, fnOld.GetFullName(),
                         "Image files (*.png;*.jpg;*.jpeg;*.gif;*.bmp)|*.png;*.jpg;*.jpeg;*.gif;*.bmp|All files (*.*)|*.*",
                         wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dlg.ShowModal() != wxID_OK) return;
        pickedPath = dlg.GetPath().ToStdString();
    }

    // --- Step 2: if same path, just reload and done ---
    if (pickedPath == oldPath) {
        _sequenceMedia->RemoveImage(oldPath);
        _sequenceMedia->GetImage(oldPath);
        Populate(oldPath);
        return;
    }

    // --- Step 3: if the picked file is outside show/media folders, offer
    //             the same embed/copy choices as OnAddButtonClick ---
    bool outsideFolders = _xlFrame ? !_xlFrame->IsInShowOrMediaFolder(pickedPath)
                                   : (!_showDirectory.empty() &&
                                      !wxString(pickedPath).StartsWith(wxString(_showDirectory)));

    std::string finalPath = pickedPath;   // path we will ultimately use in cache/effects

    if (outsideFolders) {
        wxFileName fn(pickedPath);
        const std::string sep(1, wxFileName::GetPathSeparator());

        // Build copy-target list: show directory first, then extra media folders
        std::vector<std::string> copyTargets;
        if (!_showDirectory.empty())
            copyTargets.push_back(_showDirectory);
        if (_xlFrame) {
            for (const auto& md : _xlFrame->GetMediaFolders()) {
                if (md != _showDirectory)
                    copyTargets.push_back(md);
            }
        }

        wxArrayString choices;
        choices.Add("Embed in sequence");
        for (const auto& dir : copyTargets)
            choices.Add("Copy to: " + wxString(dir));

        wxSingleChoiceDialog choiceDlg(this,
            wxString::Format("'%s' is outside the show/media folder(s).\nChoose where to place it:",
                             fn.GetFullName()),
            "File Outside Show/Media Folder", choices);
        if (choiceDlg.ShowModal() == wxID_CANCEL) return;

        int sel = choiceDlg.GetSelection();
        if (sel == 0) {
            // Embed: load from disk, rename cache key to Images/<name>, mark embedded
            _sequenceMedia->GetImage(pickedPath);
            std::string embeddedName = "Images/" + fn.GetFullName().ToStdString();
            int suffix = 1;
            std::string candidate = embeddedName;
            // Avoid colliding with anything other than the old broken entry
            while (_sequenceMedia->HasImage(candidate) && candidate != oldPath) {
                candidate = "Images/" + fn.GetName().ToStdString() +
                            "_" + std::to_string(suffix++) + "." +
                            fn.GetExt().ToStdString();
            }
            embeddedName = candidate;
            _sequenceMedia->RenameImage(pickedPath, embeddedName);
            _sequenceMedia->EmbedImage(embeddedName);
            finalPath = embeddedName;
        } else {
            // Copy to one of the folders
            std::string targetDir = copyTargets[sel - 1];
            std::string newPath;
            if (_xlFrame && targetDir == _showDirectory) {
                newPath = _xlFrame->MoveToShowFolder(pickedPath, sep + "Images");
            } else {
                wxString dest = wxString(targetDir) + wxString(sep) + "Images";
                if (!wxDirExists(dest)) wxMkdir(dest);
                dest += wxString(sep) + fn.GetFullName();
                if (wxCopyFile(wxString(pickedPath), dest, false))
                    newPath = dest.ToStdString();
            }
            if (newPath.empty()) {
                wxMessageBox("Failed to copy file to the selected folder.", "Error",
                             wxICON_ERROR | wxOK, this);
                return;
            }
            finalPath = newPath;
        }

        // Convert to relative path if inside a show/media folder
        if (_xlFrame) {
            std::string rel = _xlFrame->MakeRelativePath(finalPath);
            if (!rel.empty()) finalPath = rel;
        }
    }

    // --- Step 4: rename the broken cache entry to finalPath and update effects ---
    // If finalPath is already in the cache (e.g. we embedded under a new key),
    // just drop the old broken entry; otherwise rename it.
    if (finalPath != oldPath) {
        if (!_sequenceMedia->RenameImage(oldPath, finalPath)) {
            // finalPath is already a valid cache entry — just remove the broken one
            _sequenceMedia->RemoveImage(oldPath);
        }
    }

    // Update every effect that referenced oldPath to point to finalPath
    if (_sequenceElements != nullptr && finalPath != oldPath) {
        auto scanLayer = [&](EffectLayer* layer) {
            for (size_t k = 0; k < layer->GetEffectCount(); ++k) {
                Effect* eff = layer->GetEffect(k);
                const SettingsMap& settings = eff->GetSettings();
                std::vector<std::string> keysToUpdate;
                for (auto it = settings.begin(); it != settings.end(); ++it) {
                    if (it->second == oldPath)
                        keysToUpdate.push_back(it->first);
                }
                for (const auto& key : keysToUpdate)
                    eff->SetSetting(key, finalPath);
            }
        };

        for (size_t i = 0; i < _sequenceElements->GetElementCount(); ++i) {
            Element* e = _sequenceElements->GetElement(i);
            if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
            ModelElement* model = dynamic_cast<ModelElement*>(e);
            if (!model) continue;

            for (size_t j = 0; j < model->GetEffectLayerCount(); ++j)
                scanLayer(model->GetEffectLayer(j));

            for (size_t j = 0; j < model->GetSubModelAndStrandCount(); ++j) {
                SubModelElement* sub = model->GetSubModel(j);
                for (size_t l = 0; l < sub->GetEffectLayerCount(); ++l)
                    scanLayer(sub->GetEffectLayer(l));
                if (sub->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                    StrandElement* strand = dynamic_cast<StrandElement*>(sub);
                    if (strand) {
                        for (int k = 0; k < strand->GetNodeLayerCount(); ++k)
                            scanLayer(strand->GetNodeLayer(k));
                    }
                }
            }
        }
    }

    // For external paths, force a reload from disk (removes stale cached data)
    auto entry = _sequenceMedia->GetImage(finalPath);
    if (entry && !entry->IsEmbedded()) {
        _sequenceMedia->RemoveImage(finalPath);
        _sequenceMedia->GetImage(finalPath);
    }

    Populate(finalPath);
}

void ManageMediaPanel::OnBulkFindImages()
{
    if (_sequenceMedia == nullptr) return;

    // Collect all broken image paths
    std::vector<std::string> brokenPaths;
    for (const auto& p : _sequenceMedia->GetImagePaths()) {
        auto e = _sequenceMedia->GetImage(p);
        if (e && !e->IsOk()) brokenPaths.push_back(p);
    }
    if (brokenPaths.empty()) return;

    // Ask user to pick a directory to search in
    wxString defaultDir = _showDirectory.empty() ? wxString() : wxString(_showDirectory);
    wxDirDialog dlg(this, "Select folder containing missing images", defaultDir,
                    wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dlg.ShowModal() != wxID_OK) return;

    std::string searchDir = dlg.GetPath().ToStdString();
    ObtainAccessToURL(searchDir);
    const std::string sep(1, wxFileName::GetPathSeparator());

    // Check if the chosen directory is outside the show/media folders
    bool outsideFolders = _xlFrame ? !_xlFrame->IsInShowOrMediaFolder(searchDir + sep)
                                   : false;

    // If outside, ask how to handle the files once
    int outsideAction = -1;   // 0 = embed, 1+ = copy to target[outsideAction-1]
    std::vector<std::string> copyTargets;
    if (outsideFolders) {
        if (!_showDirectory.empty())
            copyTargets.push_back(_showDirectory);
        if (_xlFrame) {
            for (const auto& md : _xlFrame->GetMediaFolders()) {
                if (md != _showDirectory)
                    copyTargets.push_back(md);
            }
        }

        wxArrayString choices;
        choices.Add("Embed in sequence");
        for (const auto& dir : copyTargets)
            choices.Add("Copy to: " + wxString(dir));

        wxSingleChoiceDialog choiceDlg(this,
            "The selected folder is outside the show/media folder(s).\n"
            "How should found images be handled?",
            "Files Outside Show/Media Folder", choices);
        if (choiceDlg.ShowModal() == wxID_CANCEL) return;
        outsideAction = choiceDlg.GetSelection();
    }

    // Scan broken images and try to find matches in the selected directory
    int found = 0;
    int notFound = 0;
    for (const auto& oldPath : brokenPaths) {
        // Extract just the filename from the broken path
        wxFileName fnOld(oldPath);
        wxString nameToFind = fnOld.GetFullName();
        if (nameToFind.IsEmpty()) { ++notFound; continue; }

        // Look for the file in the search directory (and subdirectories)
        wxString foundFile;
        wxDir dir(searchDir);
        if (dir.IsOpened()) {
            // Try exact name in the top directory first
            wxString candidate = searchDir + sep + nameToFind.ToStdString();
            if (wxFileExists(candidate)) {
                foundFile = candidate;
            } else {
                // Recurse into subdirectories
                wxString f;
                if (dir.GetFirst(&f, nameToFind, wxDIR_FILES | wxDIR_DIRS)) {
                    foundFile = searchDir + sep + f.ToStdString();
                } else {
                    // Try a recursive traversal
                    wxArrayString results;
                    wxDir::GetAllFiles(searchDir, &results, nameToFind, wxDIR_FILES | wxDIR_DIRS);
                    if (!results.IsEmpty())
                        foundFile = results[0];
                }
            }
        }

        if (foundFile.IsEmpty()) { ++notFound; continue; }

        std::string pickedPath = foundFile.ToStdString();
        std::string finalPath = pickedPath;

        if (outsideFolders) {
            wxFileName fn(pickedPath);
            if (outsideAction == 0) {
                // Embed
                _sequenceMedia->GetImage(pickedPath);
                std::string embeddedName = "Images/" + fn.GetFullName().ToStdString();
                int suffix = 1;
                std::string candidate = embeddedName;
                while (_sequenceMedia->HasImage(candidate) && candidate != oldPath) {
                    candidate = "Images/" + fn.GetName().ToStdString() +
                                "_" + std::to_string(suffix++) + "." +
                                fn.GetExt().ToStdString();
                }
                embeddedName = candidate;
                _sequenceMedia->RenameImage(pickedPath, embeddedName);
                _sequenceMedia->EmbedImage(embeddedName);
                finalPath = embeddedName;
            } else {
                // Copy to one of the target directories
                std::string targetDir = copyTargets[outsideAction - 1];
                std::string newPath;
                if (_xlFrame && targetDir == _showDirectory) {
                    newPath = _xlFrame->MoveToShowFolder(pickedPath, sep + "Images");
                } else {
                    wxString dest = wxString(targetDir) + wxString(sep) + "Images";
                    if (!wxDirExists(dest)) wxMkdir(dest);
                    dest += wxString(sep) + fn.GetFullName();
                    if (wxCopyFile(wxString(pickedPath), dest, false))
                        newPath = dest.ToStdString();
                }
                if (newPath.empty()) continue;  // skip this file on failure
                finalPath = newPath;
            }
        }

        // Convert to relative path if inside a show/media folder
        if (_xlFrame) {
            std::string rel = _xlFrame->MakeRelativePath(finalPath);
            if (!rel.empty()) finalPath = rel;
        }

        // Rename the broken cache entry and update effects
        if (finalPath != oldPath) {
            if (!_sequenceMedia->RenameImage(oldPath, finalPath)) {
                _sequenceMedia->RemoveImage(oldPath);
            }
        }

        if (_sequenceElements != nullptr && finalPath != oldPath) {
            auto scanLayer = [&](EffectLayer* layer) {
                for (size_t k = 0; k < layer->GetEffectCount(); ++k) {
                    Effect* eff = layer->GetEffect(k);
                    const SettingsMap& settings = eff->GetSettings();
                    std::vector<std::string> keysToUpdate;
                    for (auto it = settings.begin(); it != settings.end(); ++it) {
                        if (it->second == oldPath)
                            keysToUpdate.push_back(it->first);
                    }
                    for (const auto& key : keysToUpdate)
                        eff->SetSetting(key, finalPath);
                }
            };

            for (size_t i = 0; i < _sequenceElements->GetElementCount(); ++i) {
                Element* e = _sequenceElements->GetElement(i);
                if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
                ModelElement* model = dynamic_cast<ModelElement*>(e);
                if (!model) continue;

                for (size_t j = 0; j < model->GetEffectLayerCount(); ++j)
                    scanLayer(model->GetEffectLayer(j));

                for (size_t j = 0; j < model->GetSubModelAndStrandCount(); ++j) {
                    SubModelElement* sub = model->GetSubModel(j);
                    for (size_t l = 0; l < sub->GetEffectLayerCount(); ++l)
                        scanLayer(sub->GetEffectLayer(l));
                    if (sub->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                        StrandElement* strand = dynamic_cast<StrandElement*>(sub);
                        if (strand) {
                            for (int k = 0; k < strand->GetNodeLayerCount(); ++k)
                                scanLayer(strand->GetNodeLayer(k));
                        }
                    }
                }
            }
        }

        // Force reload for external paths
        auto entry = _sequenceMedia->GetImage(finalPath);
        if (entry && !entry->IsEmbedded()) {
            _sequenceMedia->RemoveImage(finalPath);
            _sequenceMedia->GetImage(finalPath);
        }

        ++found;
    }

    wxMessageBox(wxString::Format("Bulk find complete.\n\n"
                                  "Found and updated: %d image(s)\n"
                                  "Not found: %d image(s)", found, notFound),
                 "Bulk Find Images", wxICON_INFORMATION | wxOK, this);

    Populate();
}

void ManageMediaPanel::OnAddButtonClick(wxCommandEvent& event)
{
    if (_sequenceMedia == nullptr) return;

    wxString defaultDir = _showDirectory.empty() ? wxString() : wxString(_showDirectory);
    wxFileDialog dlg(this, "Add Image to Sequence", defaultDir, wxEmptyString,
                     "Image files (*.png;*.jpg;*.jpeg;*.gif;*.bmp)|*.png;*.jpg;*.jpeg;*.gif;*.bmp|All files (*.*)|*.*",
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dlg.ShowModal() != wxID_OK) return;

    wxArrayString paths;
    dlg.GetPaths(paths);
    std::string lastPath;
    const std::string sep(1, wxFileName::GetPathSeparator());
    for (const auto& p : paths) {
        std::string path = p.ToStdString();

        // If the file is outside the show/media folders, require the user to
        // choose where to place it — no "use original location" option.
        bool outsideFolders = _xlFrame ? !_xlFrame->IsInShowOrMediaFolder(path)
                                       : (!_showDirectory.empty() && !wxString(path).StartsWith(wxString(_showDirectory)));
        if (outsideFolders) {
            wxFileName fn(path);

            // Build copy-target list: show directory first, then extra media folders
            std::vector<std::string> copyTargets;
            if (!_showDirectory.empty())
                copyTargets.push_back(_showDirectory);
            if (_xlFrame) {
                for (const auto& md : _xlFrame->GetMediaFolders()) {
                    if (md != _showDirectory)
                        copyTargets.push_back(md);
                }
            }

            wxArrayString choices;
            choices.Add("Embed in sequence");
            for (const auto& dir : copyTargets)
                choices.Add("Copy to: " + wxString(dir));

            wxSingleChoiceDialog choiceDlg(this,
                wxString::Format("'%s' is outside the show/media folder(s).\nChoose where to place it:",
                                 fn.GetFullName()),
                "File Outside Show/Media Folder", choices);
            if (choiceDlg.ShowModal() == wxID_CANCEL) continue;

            int sel = choiceDlg.GetSelection();
            if (sel == 0) {
                // Embed in sequence
                _sequenceMedia->GetImage(path);
                std::string embeddedName = "Images/" + fn.GetFullName().ToStdString();
                int suffix = 1;
                std::string candidate = embeddedName;
                while (_sequenceMedia->HasImage(candidate)) {
                    candidate = "Images/" + fn.GetName().ToStdString() +
                                "_" + std::to_string(suffix++) + "." +
                                fn.GetExt().ToStdString();
                }
                embeddedName = candidate;
                _sequenceMedia->RenameImage(path, embeddedName);
                _sequenceMedia->EmbedImage(embeddedName);
                lastPath = embeddedName;
                continue;
            } else {
                // One of the copy-to-folder choices
                std::string targetDir = copyTargets[sel - 1];
                std::string newPath;
                if (_xlFrame && targetDir == _showDirectory) {
                    newPath = _xlFrame->MoveToShowFolder(path, sep + "Images");
                } else {
                    wxString dest = wxString(targetDir) + wxString(sep) + "Images";
                    if (!wxDirExists(dest)) wxMkdir(dest);
                    dest += wxString(sep) + fn.GetFullName();
                    if (wxCopyFile(wxString(path), dest, false))
                        newPath = dest.ToStdString();
                }
                if (newPath.empty()) {
                    wxMessageBox("Failed to copy file to the selected folder.", "Error",
                                 wxICON_ERROR | wxOK, this);
                    continue;
                }
                path = newPath;
            }
        }

        // For external images now inside a show/media folder, store the relative path
        if (_xlFrame) {
            std::string rel = _xlFrame->MakeRelativePath(path);
            if (!rel.empty()) path = rel;
        }

        _sequenceMedia->GetImage(path);
        lastPath = path;
    }
    Populate(lastPath);
}

void ManageMediaPanel::OnAIGenerateButtonClick(wxCommandEvent& event)
{
    if (_xlFrame == nullptr || _sequenceMedia == nullptr) return;

    auto services = _xlFrame->GetAIServices(aiType::TYPE::IMAGES);
    if (services.empty()) {
        wxMessageBox("No AI image generation services are registered.\n"
                     "Configure an AI service in the Settings menu.",
                     "AI Generate", wxICON_INFORMATION | wxOK, this);
        return;
    }

    // Let user choose a service if there are multiple
    aiBase* serv = services[0];
    if (services.size() > 1) {
        wxArrayString choices;
        for (auto s : services)
            choices.push_back(s->GetLLMName());
        wxSingleChoiceDialog pick(this, "Choose AI Image Generator",
                                  "AI Image Generator", choices);
        if (pick.ShowModal() == wxID_CANCEL) return;
        serv = services[pick.GetSelection()];
    }

    AIImageDialog dlg(this, serv);
    dlg.SetEmbeddedMode(true);
    if (dlg.ShowModal() != wxID_OK) return;

    const wxImage& img = dlg.GetCurrentImage();
    if (!img.IsOk()) return;

    // Generate a unique name for this embedded image
    wxString name = wxString::Format("AIImages/ai_generated_%lld.png",
                                     (long long)wxDateTime::Now().GetTicks());
    std::string namePath = name.ToStdString();
    _sequenceMedia->AddEmbeddedImage(namePath, wxImageToXlImage(img));
    Populate(namePath);
}

void ManageMediaPanel::OnRenameButtonClick(wxCommandEvent& event)
{
    if (_sequenceMedia == nullptr) return;
    std::vector<std::string> paths = GetSelectedPaths();
    if (paths.size() != 1) return;
    const std::string& oldPath = paths[0];

    auto entry = _sequenceMedia->GetImage(oldPath);
    if (!entry || !entry->IsEmbedded()) return;

    // Suggest the current name (just the filename portion)
    wxFileName fn(oldPath);
    wxString suggested = fn.GetFullName().IsEmpty() ? wxString(oldPath) : fn.GetFullName();

    wxTextEntryDialog dlg(this, "Enter new name for the image:", "Rename Image", suggested);
    if (dlg.ShowModal() != wxID_OK) return;

    wxString newName = dlg.GetValue().Trim(true).Trim(false);
    if (newName.IsEmpty() || newName == suggested) return;

    // Build the new path: same directory prefix as old path, new filename
    wxString newPath;
    if (fn.GetPath().IsEmpty()) {
        newPath = newName;
    } else {
        newPath = fn.GetPath() + wxFileName::GetPathSeparator() + newName;
    }
    std::string newPathStr = newPath.ToStdString();

    if (!_sequenceMedia->RenameImage(oldPath, newPathStr)) {
        wxMessageBox("Could not rename: a media entry with that name already exists.",
                     "Rename", wxICON_WARNING | wxOK, this);
        return;
    }

    // Update all effect references oldPath -> newPathStr
    if (_sequenceElements != nullptr) {
        auto scanLayer = [&](EffectLayer* layer) {
            for (size_t k = 0; k < layer->GetEffectCount(); ++k) {
                Effect* eff = layer->GetEffect(k);
                const SettingsMap& settings = eff->GetSettings();
                std::vector<std::string> keysToUpdate;
                for (auto it = settings.begin(); it != settings.end(); ++it) {
                    if (it->second == oldPath)
                        keysToUpdate.push_back(it->first);
                }
                for (const auto& key : keysToUpdate)
                    eff->SetSetting(key, newPathStr);
            }
        };

        for (size_t i = 0; i < _sequenceElements->GetElementCount(); ++i) {
            Element* e = _sequenceElements->GetElement(i);
            if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
            ModelElement* model = dynamic_cast<ModelElement*>(e);
            if (!model) continue;

            for (size_t j = 0; j < model->GetEffectLayerCount(); ++j)
                scanLayer(model->GetEffectLayer(j));

            for (size_t j = 0; j < model->GetSubModelAndStrandCount(); ++j) {
                SubModelElement* sub = model->GetSubModel(j);
                for (size_t l = 0; l < sub->GetEffectLayerCount(); ++l)
                    scanLayer(sub->GetEffectLayer(l));
                if (sub->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                    StrandElement* strand = dynamic_cast<StrandElement*>(sub);
                    if (strand) {
                        for (int k = 0; k < strand->GetNodeLayerCount(); ++k)
                            scanLayer(strand->GetNodeLayer(k));
                    }
                }
            }
        }
    }

    Populate(newPathStr);
}

void ManageMediaPanel::OnEmbedButtonClick(wxCommandEvent& event)
{
    if (_sequenceMedia == nullptr) return;
    std::vector<std::string> paths = GetSelectedPaths();
    if (paths.empty()) return;
    std::string lastNewPath;
    for (const auto& path : paths) {
        // Check if media exists and is embeddable (works for all types)
        if (_sequenceMedia->HasMedia(path)) {
            // For images, use the existing image-aware path
            if (_sequenceMedia->HasImage(path)) {
                auto entry = _sequenceMedia->GetImage(path);
                if (entry && !entry->IsEmbedded() && entry->IsEmbeddable()) {
                    std::string newPath = EmbedWithRename(path);
                    if (!newPath.empty()) lastNewPath = newPath;
                }
            } else {
                // Non-image: use generic EmbedMedia
                _sequenceMedia->EmbedMedia(path);
                lastNewPath = path;
            }
        }
    }
    Populate(lastNewPath);
}

void ManageMediaPanel::OnExtractButtonClick(wxCommandEvent& event)
{
    if (_sequenceMedia == nullptr) return;
    std::vector<std::string> paths = GetSelectedPaths();
    if (paths.empty()) return;
    std::string lastNewPath;
    for (const auto& path : paths) {
        if (_sequenceMedia->HasImage(path)) {
            auto entry = _sequenceMedia->GetImage(path);
            if (entry && entry->IsEmbedded()) {
                std::string newPath = ExtractWithRename(path);
                if (!newPath.empty()) lastNewPath = newPath;
            }
        } else if (_sequenceMedia->HasMedia(path)) {
            // Non-image: use generic ExtractMedia
            _sequenceMedia->ExtractMedia(path);
            lastNewPath = path;
        }
    }
    if (!lastNewPath.empty())
        Populate(lastNewPath);
}

void ManageMediaPanel::OnEmbedAllButtonClick(wxCommandEvent& event)
{
    if (_sequenceMedia == nullptr) return;
    // Snapshot paths first since EmbedWithRename may rename keys mid-iteration
    auto allPaths = _sequenceMedia->GetAllMediaPaths();
    for (const auto& [path, type] : allPaths) {
        if (type == MediaType::Image) {
            auto entry = _sequenceMedia->GetImage(path);
            if (entry && !entry->IsEmbedded() && entry->IsEmbeddable())
                EmbedWithRename(path);
        } else if (type != MediaType::Video) {
            // Non-image, non-video: use generic embed
            _sequenceMedia->EmbedMedia(path);
        }
    }
    Populate();
}

void ManageMediaPanel::OnExtractAllButtonClick(wxCommandEvent& event)
{
    if (_sequenceMedia == nullptr) return;

    // Ask the user for a target directory — must be inside show/media folders.
    wxString defaultDir = _showDirectory.empty() ? wxString() : wxString(_showDirectory);
    wxString destDir;
    for (;;) {
        wxDirDialog dlg(this, "Choose directory to extract media into", defaultDir,
                        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        if (dlg.ShowModal() != wxID_OK) return;
        destDir = dlg.GetPath();

        bool outside = _xlFrame ? !_xlFrame->IsInShowOrMediaFolder(destDir.ToStdString())
                                : (!_showDirectory.empty() &&
                                   !destDir.StartsWith(wxString(_showDirectory)));
        if (!outside) break;

        wxMessageBox(
            "The selected directory is outside the show/media folder(s).\n"
            "Please choose a directory inside the show or a media folder.",
            "Invalid Location", wxOK | wxICON_WARNING, this);
        defaultDir = destDir;  // keep the user's last choice as starting point
    }
    ObtainAccessToURL(destDir.ToStdString());
    if (destDir.Last() != wxFileName::GetPathSeparator()) {
        destDir += wxFileName::GetPathSeparator();
    }

    auto allPaths = _sequenceMedia->GetAllMediaPaths();
    int failed = 0;
    for (const auto& [path, type] : allPaths) {
        // Skip non-embeddable types (Video)
        if (type == MediaType::Video) continue;

        // Check if this entry is embedded
        bool isEmbedded = false;
        if (type == MediaType::Image) {
            auto entry = _sequenceMedia->GetImage(path);
            if (!entry || !entry->IsEmbedded()) continue;
            isEmbedded = true;
        } else {
            // Check via type-specific accessor
            MediaCacheEntry* baseEntry = nullptr;
            std::shared_ptr<MediaCacheEntry> holder;
            if (type == MediaType::SVG) { auto e = _sequenceMedia->GetSVG(path); holder = e; baseEntry = e.get(); }
            else if (type == MediaType::Shader) { auto e = _sequenceMedia->GetShader(path); holder = e; baseEntry = e.get(); }
            else if (type == MediaType::TextFile) { auto e = _sequenceMedia->GetTextFile(path); holder = e; baseEntry = e.get(); }
            else if (type == MediaType::BinaryFile) { auto e = _sequenceMedia->GetBinaryFile(path); holder = e; baseEntry = e.get(); }
            if (!baseEntry || !baseEntry->IsEmbedded()) continue;
            isEmbedded = true;
        }
        if (!isEmbedded) continue;

        // Build destination: use just the filename portion of the cache key
        wxFileName fn(path);
        wxString destPath = destDir + fn.GetFullName();
        if (destPath.IsEmpty()) { ++failed; continue; }

        std::string oldPath = path;
        std::string newPath = destPath.ToStdString();

        ObtainAccessToURL(newPath);

        if (type == MediaType::Image) {
            if (!_sequenceMedia->ExtractImageToFile(oldPath, newPath)) {
                ++failed;
                continue;
            }
        } else {
            // For non-image types, save embedded data to file and mark as external
            MediaCacheEntry* baseEntry = nullptr;
            std::shared_ptr<MediaCacheEntry> holder;
            if (type == MediaType::SVG) { auto e = _sequenceMedia->GetSVG(oldPath); holder = e; baseEntry = e.get(); }
            else if (type == MediaType::Shader) { auto e = _sequenceMedia->GetShader(oldPath); holder = e; baseEntry = e.get(); }
            else if (type == MediaType::TextFile) { auto e = _sequenceMedia->GetTextFile(oldPath); holder = e; baseEntry = e.get(); }
            else if (type == MediaType::BinaryFile) { auto e = _sequenceMedia->GetBinaryFile(oldPath); holder = e; baseEntry = e.get(); }
            if (!baseEntry || !baseEntry->SaveToFile(newPath)) {
                ++failed;
                continue;
            }
            _sequenceMedia->ExtractMedia(oldPath);
        }

        // Convert absolute path to relative (relative to show/media folder)
        std::string finalPath = newPath;
        if (_xlFrame) {
            std::string rel = _xlFrame->MakeRelativePath(newPath);
            if (!rel.empty()) {
                if (type == MediaType::Image)
                    _sequenceMedia->RenameImage(newPath, rel);
                finalPath = rel;
            }
        }

        // Update effect references oldPath -> finalPath
        if (_sequenceElements != nullptr && finalPath != oldPath) {
            auto scanLayer = [&](EffectLayer* layer) {
                for (size_t k = 0; k < layer->GetEffectCount(); ++k) {
                    Effect* eff = layer->GetEffect(k);
                    const SettingsMap& settings = eff->GetSettings();
                    std::vector<std::string> keysToUpdate;
                    for (auto it = settings.begin(); it != settings.end(); ++it) {
                        if (it->second == oldPath)
                            keysToUpdate.push_back(it->first);
                    }
                    for (const auto& key : keysToUpdate)
                        eff->SetSetting(key, finalPath);
                }
            };

            for (size_t i = 0; i < _sequenceElements->GetElementCount(); ++i) {
                Element* e = _sequenceElements->GetElement(i);
                if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
                ModelElement* model = dynamic_cast<ModelElement*>(e);
                if (!model) continue;

                for (size_t j = 0; j < model->GetEffectLayerCount(); ++j)
                    scanLayer(model->GetEffectLayer(j));

                for (size_t j = 0; j < model->GetSubModelAndStrandCount(); ++j) {
                    SubModelElement* sub = model->GetSubModel(j);
                    for (size_t l = 0; l < sub->GetEffectLayerCount(); ++l)
                        scanLayer(sub->GetEffectLayer(l));
                    if (sub->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                        StrandElement* strand = dynamic_cast<StrandElement*>(sub);
                        if (strand) {
                            for (int k = 0; k < strand->GetNodeLayerCount(); ++k)
                                scanLayer(strand->GetNodeLayer(k));
                        }
                    }
                }
            }
        }
    }

    if (failed > 0)
        wxMessageBox(wxString::Format("%d media file(s) could not be extracted.", failed),
                     "Extract All", wxICON_WARNING | wxOK, this);
    Populate();
}
void ManageMediaPanel::OnRemoveButtonClick(wxCommandEvent& event)
{
    if (_sequenceMedia == nullptr) return;

    // Collect selected paths that are embedded (any media type)
    std::vector<std::string> toRemove;
    for (const auto& path : GetSelectedPaths()) {
        if (_sequenceMedia->HasImage(path)) {
            auto entry = _sequenceMedia->GetImage(path);
            if (entry && entry->IsEmbedded())
                toRemove.push_back(path);
        } else if (_sequenceMedia->HasMedia(path)) {
            // Check non-image types for embedded status
            auto allMedia = _sequenceMedia->GetAllMediaPaths();
            for (const auto& [p, t] : allMedia) {
                if (p != path) continue;
                MediaCacheEntry* baseEntry = nullptr;
                std::shared_ptr<MediaCacheEntry> holder;
                switch (t) {
                    case MediaType::SVG: { auto e = _sequenceMedia->GetSVG(p); holder = e; baseEntry = e.get(); break; }
                    case MediaType::Shader: { auto e = _sequenceMedia->GetShader(p); holder = e; baseEntry = e.get(); break; }
                    case MediaType::TextFile: { auto e = _sequenceMedia->GetTextFile(p); holder = e; baseEntry = e.get(); break; }
                    case MediaType::BinaryFile: { auto e = _sequenceMedia->GetBinaryFile(p); holder = e; baseEntry = e.get(); break; }
                    default: break;
                }
                if (baseEntry && baseEntry->IsEmbedded())
                    toRemove.push_back(path);
                break;
            }
        }
    }
    if (toRemove.empty()) return;

    // Scan all effects to find references to any of the paths being removed
    int usageCount = 0;
    if (_sequenceElements != nullptr) {
        auto scanLayer = [&](EffectLayer* layer) {
            for (size_t k = 0; k < layer->GetEffectCount(); ++k) {
                Effect* eff = layer->GetEffect(k);
                const SettingsMap& settings = eff->GetSettings();
                for (auto it = settings.begin(); it != settings.end(); ++it) {
                    for (const auto& path : toRemove) {
                        if (it->second == path) {
                            ++usageCount;
                            break;
                        }
                    }
                }
            }
        };

        for (size_t i = 0; i < _sequenceElements->GetElementCount(); ++i) {
            Element* e = _sequenceElements->GetElement(i);
            if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
            ModelElement* model = dynamic_cast<ModelElement*>(e);
            if (!model) continue;

            for (size_t j = 0; j < model->GetEffectLayerCount(); ++j)
                scanLayer(model->GetEffectLayer(j));

            for (size_t j = 0; j < model->GetSubModelAndStrandCount(); ++j) {
                SubModelElement* sub = model->GetSubModel(j);
                for (size_t l = 0; l < sub->GetEffectLayerCount(); ++l)
                    scanLayer(sub->GetEffectLayer(l));
                if (sub->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                    StrandElement* strand = dynamic_cast<StrandElement*>(sub);
                    if (strand) {
                        for (int k = 0; k < strand->GetNodeLayerCount(); ++k)
                            scanLayer(strand->GetNodeLayer(k));
                    }
                }
            }
        }
    }

    // Warn if any effects reference the media
    if (usageCount > 0) {
        wxString msg = wxString::Format(
            "%d effect(s) reference the selected media file(s).\n"
            "Removing them will leave those effects with missing media.\n\n"
            "Remove anyway?",
            usageCount);
        if (wxMessageBox(msg, "Remove Embedded Media",
                         wxYES_NO | wxNO_DEFAULT | wxICON_WARNING, this) != wxYES)
            return;
    } else {
        wxString msg = wxString::Format(
            "Remove %d embedded media file(s) from the sequence?",
            (int)toRemove.size());
        if (wxMessageBox(msg, "Remove Embedded Media",
                         wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION, this) != wxYES)
            return;
    }

    for (const auto& path : toRemove)
        _sequenceMedia->RemoveMedia(path);

    Populate();
}

// ---------------------------------------------------------------------------
// SelectMediaDialog
// ---------------------------------------------------------------------------

SelectMediaDialog::SelectMediaDialog(wxWindow* parent, SequenceMedia* sequenceMedia,
                                     SequenceElements* sequenceElements,
                                     const std::string& showDirectory,
                                     xLightsFrame* xlFrame,
                                     std::optional<MediaType> filterType)
    : wxDialog(parent, wxID_ANY,
               filterType.has_value() ? wxString("Select ") + MediaTypeName(*filterType) : wxString("Select Media"),
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , _filterType(filterType)
{
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

    // Embed the panel (single-select mode via wxDV_SINGLE)
    _panel = new ManageMediaPanel(this, sequenceMedia, sequenceElements,
                                  showDirectory, xlFrame, /*singleSelect=*/true,
                                  filterType);

    topSizer->Add(_panel, 1, wxEXPAND | wxALL, 0);

    // "Add from disk..." and OK / Cancel buttons
    wxBoxSizer* btnRowSizer = new wxBoxSizer(wxHORIZONTAL);
    _addFromDiskButton = new wxButton(this, wxID_ANY, "Add from disk...");
    _addFromDiskButton->SetToolTip("Browse for a file on disk to add to the sequence media");
    btnRowSizer->Add(_addFromDiskButton, 0, wxALL, 4);

    btnRowSizer->AddStretchSpacer();

    wxStdDialogButtonSizer* stdBtnSizer = new wxStdDialogButtonSizer();
    _okButton = new wxButton(this, wxID_OK, "OK");
    _okButton->Enable(false);
    stdBtnSizer->AddButton(_okButton);
    stdBtnSizer->AddButton(new wxButton(this, wxID_CANCEL, "Cancel"));
    stdBtnSizer->Realize();
    btnRowSizer->Add(stdBtnSizer, 0, wxALL, 4);

    topSizer->Add(btnRowSizer, 0, wxEXPAND | wxALL, 4);

    SetSizerAndFit(topSizer);
    Fit();

    // Enable OK when a single leaf item is selected in the panel's tree
    _panel->_mediaTree->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED,
                             &SelectMediaDialog::OnSelectionChanged, this);

    _okButton->Bind(wxEVT_BUTTON, &SelectMediaDialog::OnOK, this);
    _addFromDiskButton->Bind(wxEVT_BUTTON, &SelectMediaDialog::OnAddFromDisk, this);
}

std::string SelectMediaDialog::GetSelectedPath() const
{
    auto paths = _panel->GetSelectedPaths();
    if (paths.size() == 1)
        return paths[0];
    return {};
}

void SelectMediaDialog::OnSelectionChanged(wxDataViewEvent& event)
{
    auto paths = _panel->GetSelectedPaths();
    _okButton->Enable(paths.size() == 1);
    // Ensure panel's own handler (UpdatePreview etc.) still runs
    event.Skip();
}

void SelectMediaDialog::OnAddFromDisk(wxCommandEvent& event)
{
    if (!_panel->_sequenceMedia) return;

    wxString defaultDir = _panel->_showDirectory.empty() ? wxString() : wxString(_panel->_showDirectory);
    wxFileDialog dlg(this, "Select Media File", defaultDir, wxEmptyString,
                     WildcardForMediaType(_filterType),
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() != wxID_OK) return;

    std::string path = dlg.GetPath().ToStdString();
    const std::string sep(1, wxFileName::GetPathSeparator());

    // Determine which MediaType to register as
    MediaType regType = _filterType.has_value() ? *_filterType : MediaType::Image;

    // If the file is outside show/media folders, require the user to
    // choose where to place it.
    bool outsideFolders = _panel->_xlFrame
        ? !_panel->_xlFrame->IsInShowOrMediaFolder(path)
        : (!_panel->_showDirectory.empty() && !wxString(path).StartsWith(wxString(_panel->_showDirectory)));

    if (outsideFolders) {
        wxFileName fn(path);

        // Build copy-target list: show directory first, then extra media folders
        std::vector<std::string> copyTargets;
        if (!_panel->_showDirectory.empty())
            copyTargets.push_back(_panel->_showDirectory);
        if (_panel->_xlFrame) {
            for (const auto& md : _panel->_xlFrame->GetMediaFolders()) {
                if (md != _panel->_showDirectory)
                    copyTargets.push_back(md);
            }
        }

        wxArrayString choices;
        choices.Add("Embed in sequence");
        for (const auto& dir : copyTargets)
            choices.Add("Copy to: " + wxString(dir));

        wxSingleChoiceDialog choiceDlg(this,
            wxString::Format("'%s' is outside the show/media folder(s).\nChoose where to place it:",
                             fn.GetFullName()),
            "File Outside Show/Media Folder", choices);
        if (choiceDlg.ShowModal() == wxID_CANCEL) return;

        int sel = choiceDlg.GetSelection();
        if (sel == 0) {
            // Embed in sequence — register, rename to embedded key, embed
            std::string subdir = MediaTypeName(regType);
            // Register with original path first
            switch (regType) {
                case MediaType::Image:    _panel->_sequenceMedia->GetImage(path); break;
                case MediaType::Video:    _panel->_sequenceMedia->GetVideo(path); break;
                case MediaType::SVG:      _panel->_sequenceMedia->GetSVG(path); break;
                case MediaType::Shader:   _panel->_sequenceMedia->GetShader(path); break;
                case MediaType::TextFile: _panel->_sequenceMedia->GetTextFile(path); break;
                case MediaType::BinaryFile: _panel->_sequenceMedia->GetBinaryFile(path); break;
            }
            if (regType == MediaType::Image) {
                std::string embeddedName = "Images/" + fn.GetFullName().ToStdString();
                int suffix = 1;
                std::string candidate = embeddedName;
                while (_panel->_sequenceMedia->HasImage(candidate)) {
                    candidate = "Images/" + fn.GetName().ToStdString() +
                                "_" + std::to_string(suffix++) + "." +
                                fn.GetExt().ToStdString();
                }
                embeddedName = candidate;
                _panel->_sequenceMedia->RenameImage(path, embeddedName);
                _panel->_sequenceMedia->EmbedImage(embeddedName);
                path = embeddedName;
            } else {
                _panel->_sequenceMedia->EmbedMedia(path);
            }
            _panel->Populate(path);
            return;
        } else {
            // Copy to one of the folders
            std::string targetDir = copyTargets[sel - 1];
            std::string newPath;
            std::string subFolder = sep + MediaTypeName(regType);
            if (_panel->_xlFrame && targetDir == _panel->_showDirectory) {
                newPath = _panel->_xlFrame->MoveToShowFolder(path, subFolder);
            } else {
                wxString dest = wxString(targetDir) + wxString(subFolder);
                if (!wxDirExists(dest)) wxMkdir(dest);
                wxFileName fn2(path);
                dest += wxString(sep) + fn2.GetFullName();
                if (wxCopyFile(wxString(path), dest, false))
                    newPath = dest.ToStdString();
            }
            if (newPath.empty()) {
                wxMessageBox("Failed to copy file to the selected folder.", "Error",
                             wxICON_ERROR | wxOK, this);
                return;
            }
            path = newPath;
        }
    }

    // Make path relative if possible
    if (_panel->_xlFrame) {
        std::string rel = _panel->_xlFrame->MakeRelativePath(path);
        if (!rel.empty()) path = rel;
    }

    // Register with SequenceMedia
    switch (regType) {
        case MediaType::Image:    _panel->_sequenceMedia->GetImage(path); break;
        case MediaType::Video:    _panel->_sequenceMedia->GetVideo(path); break;
        case MediaType::SVG:      _panel->_sequenceMedia->GetSVG(path); break;
        case MediaType::Shader:   _panel->_sequenceMedia->GetShader(path); break;
        case MediaType::TextFile: _panel->_sequenceMedia->GetTextFile(path); break;
        case MediaType::BinaryFile: _panel->_sequenceMedia->GetBinaryFile(path); break;
    }

    _panel->Populate(path);
}

void SelectMediaDialog::OnOK(wxCommandEvent& event)
{
    EndModal(wxID_OK);
}

