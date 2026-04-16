/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ui/media/ManageMediaPanel.h"
#include "ui/media/ShaderPreviewGenerator.h"
#include "render/SequenceMedia.h"
#include "media/VideoReader.h"
#include "media/MediaCompatibility.h"
#include "media/VideoTranscoder.h"
#include "render/SequenceElements.h"
#include "render/Element.h"
#include "render/EffectLayer.h"  // also defines NodeLayer
#include "render/Effect.h"
#include "UtilClasses.h"
#include "xLightsMain.h"
#include "ai/AIImageDialog.h"
#include "ai/aiType.h"
#include "ui/shared/utils/wxUtilities.h"
#include "utils/xlImage.h"

#include <wx/filename.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/progdlg.h>
#include <wx/textdlg.h>
#include <wx/datetime.h>
#include <wx/config.h>
#include <wx/progdlg.h>
#include <filesystem>
#include <map>
#include <algorithm>

#include "utils/ExternalHooks.h"
#include "utils/FileUtils.h"
#include "effects/RenderableEffect.h"
#include "effects/EffectManager.h"

#include <set>

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

// Returns <showDirectory>/ImportedMedia/<seqStem>/<subFolder>, or empty if no
// sequence is loaded or no show directory is set.
// NOTE: GetSeqXmlFileName() may return the show directory path when the sequence
// has not been saved yet (legacy behaviour). Guard against that by requiring the
// path to be an existing file, not a directory, and allowing both .xsq and .xml
// saved sequence files.
static std::string ImportedMediaPath(xLightsFrame* xlFrame, const std::string& showDirectory, const std::string& subFolder)
{
    if (!xlFrame || showDirectory.empty()) return {};
    wxString seqFile = xlFrame->GetSeqXmlFileName();
    if (seqFile.IsEmpty()) return {};
    wxFileName seqFn(seqFile);
    // GetSeqXmlFileName can return the show directory when no sequence name is stored.
    wxString ext = seqFn.GetExt();
    if (ext.CmpNoCase("xsq") != 0 && ext.CmpNoCase("xml") != 0) return {};
    std::string seqStem = seqFn.GetName().ToStdString();
    if (seqStem.empty()) return {};
    return (std::filesystem::path(showDirectory) / "ImportedMedia" / seqStem / subFolder).string();
}

// Copies srcPath into targetDir (creating it if needed).
// If the file already exists at the destination it is reused.
// Returns the destination path on success, empty on failure.
static std::string CopyToDir(const std::string& srcPath, const std::string& targetDir)
{
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(targetDir), ec);
    if (ec) return {};
    wxChar pathSep = wxFileName::GetPathSeparator();
    wxString dest = wxString(targetDir) + pathSep + wxFileName(srcPath).GetFullName();
    if (wxFileExists(dest))
        return ToStdString(dest);  // already present — reuse
    if (!wxCopyFile(wxString(srcPath), dest, false))
        return {};
    return ToStdString(dest);
}

static wxString WildcardForMediaType(std::optional<MediaType> type) {
    if (!type.has_value()) return "All files (*.*)|*.*";
    switch (*type) {
        case MediaType::Image: return wxImage::GetImageExtWildcard();
        case MediaType::Video: return "Video Files|*.avi;*.mp4;*.mkv;*.mov;*.asf;*.flv;*.mpg;*.mpeg;*.m4v;*.wmv;*.gif";
        case MediaType::Shader: return "Shader Files (*.fs)|*.fs";
        case MediaType::SVG: return "SVG Files (*.svg)|*.svg";
        case MediaType::TextFile: return "Text Files (*.txt)|*.txt";
        case MediaType::BinaryFile: return "Glediator Files|*.gled;*.out;*.csv";
    }
    return "All files (*.*)|*.*";
}

// If the selected video is flagged incompatible with AVFoundation, offer the
// same Convert-Now path used on sequence load. Returns the path to register
// (either the original or a newly-written .mov), or empty if the user
// cancelled the whole add.
static std::string MaybeConvertIncompatibleVideo(wxWindow* parent,
                                                 const std::string& originalPath)
{
    std::string reason = MediaCompatibility::CheckVideoFile(originalPath);
    if (reason.empty()) return originalPath;

    wxFileName fn = wxFileName(wxString(originalPath));
    wxString msg = wxString::Format(
        "'%s' is in a format that will not render on upcoming versions of xLights.\n\n"
        "Reason: %s\n\n"
        "Convert it to a compatible .mov now?",
        fn.GetFullName(), wxString(reason));
    int answer = wxMessageBox(msg, "Incompatible Video",
                              wxYES_NO | wxCANCEL | wxICON_QUESTION, parent);
    if (answer == wxCANCEL) return {};
    if (answer == wxNO) return originalPath;

    std::string target = VideoTranscoder::SuggestedOutputPath(originalPath);
    if (target == originalPath) {
        std::filesystem::path p(originalPath);
        p.replace_filename(p.stem().string() + "_converted.mov");
        target = p.string();
    }

    wxProgressDialog progDlg("Converting Video",
                             wxString::Format("Converting %s...", fn.GetFullName()),
                             1000, parent,
                             wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT |
                             wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME);
    progDlg.SetSize(wxSize(520, -1));

    bool cancelled = false;
    auto progressCb = [&](int frame, int total) -> bool {
        int pct = 0;
        if (total > 0) {
            pct = (int)((double)frame / total * 1000.0);
            if (pct > 999) pct = 999;
        } else {
            pct = frame % 1000;
        }
        bool cont = progDlg.Update(pct,
            wxString::Format("Converting %s (frame %d)", fn.GetFullName(), frame));
        if (!cont) cancelled = true;
        return cont;
    };

    std::string err = VideoTranscoder::Transcode(originalPath, target, progressCb);
    progDlg.Update(1000);

    if (cancelled) {
        std::error_code ec;
        std::filesystem::remove(target, ec);
        return {};
    }
    if (!err.empty()) {
        std::error_code ec;
        std::filesystem::remove(target, ec);
        wxMessageBox(wxString::Format("Conversion failed: %s\n\nThe original file will be added as-is.",
                                      wxString(err)),
                     "Conversion Failed", wxICON_ERROR | wxOK, parent);
        return originalPath;
    }
    return target;
}

static wxString LastDirConfigKey(std::optional<MediaType> type) {
    if (!type.has_value()) return "MediaManagerLastImageDir";
    switch (*type) {
        case MediaType::Video:      return "MediaManagerLastVideoDir";
        case MediaType::Shader:     return "MediaManagerLastShaderDir";
        case MediaType::SVG:        return "MediaManagerLastSVGDir";
        case MediaType::TextFile:   return "MediaManagerLastTextFileDir";
        case MediaType::BinaryFile: return "MediaManagerLastBinaryFileDir";
        default:                    return "MediaManagerLastImageDir";
    }
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
            node->label = ToStdString(fnName.GetFullName());

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
                node->canLoad = baseEntry->IsOk();
                node->sizeStr = "-";
                node->framesStr = "-";
                node->statusStr = baseEntry->IsEmbedded() ? "Embedded" : "External";
                if (!node->canLoad)
                    node->statusStr += " (not found)";
            }

            wxFileName fn(displayPath(path, resolvedPath));
            wxString dir = fn.GetPath();
            if (dir.IsEmpty()) dir = "(show directory)";

            auto it = dirGroups.find(dir);
            if (it == dirGroups.end()) {
                auto grp = std::make_shared<MediaNode>();
                grp->isGroup = true;
                grp->label = ToStdString(dir);
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
            // If there's only one directory sub-group, promote its children
            // directly into the type group to avoid redundant nesting
            // (e.g., "Shaders > Shaders > file.fs" becomes "Shaders > file.fs")
            if (typeGroup->children.size() == 1 && typeGroup->children[0]->isGroup) {
                auto& onlyDir = typeGroup->children[0];
                std::vector<std::shared_ptr<MediaNode>> promoted;
                for (auto& child : onlyDir->children) {
                    child->parent = typeGroup.get();
                    promoted.push_back(child);
                }
                typeGroup->children = std::move(promoted);
            }
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
    if (!node) return wxDataViewItem(nullptr);
    // Return actual parent for all nodes — nullptr parent means top-level (root).
    // Previously this returned nullptr for all group nodes, which broke GTK's
    // FindNode() for nested directory groups under type groups.
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
    SetExtraStyle(GetExtraStyle() | wxWS_EX_PROCESS_IDLE);

    // Two-column layout: tree on the left (grows), preview+buttons on the right.
    wxFlexGridSizer* mainSizer = new wxFlexGridSizer(1, 2, 0, 0);
    mainSizer->AddGrowableCol(0);
    mainSizer->AddGrowableRow(0);

    // Left side: tree with columns
    _mediaTree = new wxDataViewCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                    (singleSelect ? wxDV_SINGLE : wxDV_MULTIPLE) | wxBORDER_SUNKEN);
    _mediaTree->AssociateModel(_model.get());

    _mediaTree->AppendTextColumn("File",   COL_NAME,   wxDATAVIEW_CELL_INERT, 220, wxALIGN_LEFT,
                                 wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
    _mediaTree->AppendTextColumn("Size",   COL_SIZE,   wxDATAVIEW_CELL_INERT,  70, wxALIGN_LEFT,
                                 wxDATAVIEW_COL_RESIZABLE);
    _mediaTree->AppendTextColumn("Frames", COL_FRAMES, wxDATAVIEW_CELL_INERT,  50, wxALIGN_LEFT,
                                 wxDATAVIEW_COL_RESIZABLE);
    _mediaTree->AppendTextColumn("Status", COL_STATUS, wxDATAVIEW_CELL_INERT,  -1, wxALIGN_LEFT,
                                 wxDATAVIEW_COL_RESIZABLE);
    // Only constrain height — width fills whatever space the dialog provides.
    _mediaTree->SetMinSize(wxSize(-1, 100));
    mainSizer->Add(_mediaTree, 1, wxALL | wxEXPAND, 5);

    // Right side: preview + single-column stacked buttons.
    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

    _preview = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize(100, 100));
    _preview->SetMinSize(wxSize(100, 100));
    _preview->SetMaxSize(wxSize(100, 100));
    rightSizer->Add(_preview, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 4);

    _infoLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(130, -1),
                                  wxST_NO_AUTORESIZE);
    rightSizer->Add(_infoLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 4);

    rightSizer->Add(0, 2, 0);

    _addButton = new wxButton(this, wxID_ANY, "Add...");
    _addButton->SetToolTip("Add a media file to the sequence");
    _addButton->Show(!singleSelect);
    rightSizer->Add(_addButton, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 2);

    _aiGenerateButton = new wxButton(this, wxID_ANY, "AI Generate...");
    _aiGenerateButton->SetToolTip("Generate an image using an AI service and add it to the sequence media");
    rightSizer->Add(_aiGenerateButton, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 2);

    rightSizer->Add(0, 4, 0);

    _renameButton = new wxButton(this, wxID_ANY, "Rename...");
    _renameButton->SetToolTip("Rename the selected embedded image and update all sequence references");
    _renameButton->Disable();
    rightSizer->Add(_renameButton, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 2);

    rightSizer->Add(0, 4, 0);

    _embedButton = new wxButton(this, wxID_ANY, "Embed");
    _embedButton->SetToolTip("Store this media inside the sequence file");
    _embedButton->Disable();
    rightSizer->Add(_embedButton, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 2);

    _extractButton = new wxButton(this, wxID_ANY, "Extract");
    _extractButton->SetToolTip("Reference this media from an external file path");
    _extractButton->Disable();
    rightSizer->Add(_extractButton, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 2);

    _embedAllButton = new wxButton(this, wxID_ANY, "Embed All");
    _embedAllButton->SetToolTip("Store all media inside the sequence file");
    rightSizer->Add(_embedAllButton, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 2);

    _extractAllButton = new wxButton(this, wxID_ANY, "Extract All");
    _extractAllButton->SetToolTip("Reference all media from external file paths");
    rightSizer->Add(_extractAllButton, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 2);

    rightSizer->Add(0, 4, 0);

    _removeButton = new wxButton(this, wxID_ANY, "Remove");
    _removeButton->SetToolTip("Remove the selected embedded media from the sequence");
    _removeButton->Disable();
    _removeButton->Show(!singleSelect);
    rightSizer->Add(_removeButton, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 2);

    mainSizer->Add(rightSizer, 0, wxALL | wxEXPAND, 2);

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

    _previewTimer.SetOwner(this);
    Bind(wxEVT_TIMER, &ManageMediaPanel::OnPreviewTimer, this, _previewTimer.GetId());

    Populate();
}

ManageMediaPanel::~ManageMediaPanel()
{
    _previewTimer.Stop();
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
    // Rebuild the model data silently, then force GTK to rebuild its internal row
    // structure by dissociating and re-associating the model.  This is safer than
    // Cleared() + ItemAdded(): Cleared() sets GTK's internal count to 0, and mixing
    // it with ItemAdded() causes a count disparity that triggers the
    // "model changed without letting the view know" GTK-CRITICAL during paint.
    // AssociateModel() calls gtk_tree_view_set_model() which runs
    // gtk_tree_view_build_tree() synchronously, so Expand() works immediately after.
    _model->Rebuild(_sequenceMedia, _showDirectory, mediaDirs, _filterType);
    _mediaTree->AssociateModel(nullptr);
    _mediaTree->AssociateModel(_model.get());

    // Request expansion — the actual Expand() calls are deferred to an idle
    // handler so that GTK has fully realized and mapped the widget first.
    RequestExpandGroups();

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

void ManageMediaPanel::ExpandGroups()
{
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
}

void ManageMediaPanel::RequestExpandGroups()
{
    if (!_expandPending) {
        _expandPending = true;
        Bind(wxEVT_IDLE, &ManageMediaPanel::OnIdle, this);
    }
}

void ManageMediaPanel::OnIdle(wxIdleEvent& event)
{
    if (_expandPending && IsShownOnScreen()) {
        _expandPending = false;
        Unbind(wxEVT_IDLE, &ManageMediaPanel::OnIdle, this);
        ExpandGroups();
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
    _previewTimer.Stop();
    _previewFrames.clear();
    _previewFrameTimes.clear();
    _currentPreviewFrame = 0;

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

    double scaleFactor = GetContentScaleFactor();
    int maxPx = (int)(150.0 * scaleFactor);

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

        // Generate animated preview frames
        entry->GeneratePreview(maxPx, maxPx);
        for (int i = 0; i < (int)entry->GetPreviewFrameCount(); i++) {
            _previewFrames.push_back(entry->GetPreviewFrame(i));
            _previewFrameTimes.push_back(entry->GetPreviewFrameTime(i));
        }

        if (!_previewFrames.empty()) {
            ShowPreviewFrame(0);
            if (_previewFrames.size() > 1) {
                long interval = (_previewFrameTimes[0] > 0) ? _previewFrameTimes[0] : 50;
                _previewTimer.Start(interval);
            }
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

        // Generate preview frames for supported types
        if (t == MediaType::Shader && _xlFrame) {
            auto shaderEntry = _sequenceMedia->GetShader(p);
            if (shaderEntry) {
                GenerateShaderPreview(shaderEntry.get(), _xlFrame);
            }
        } else if (holder) {
            holder->GeneratePreview(maxPx, maxPx);
        }
        if (holder) {
            for (int i = 0; i < (int)holder->GetPreviewFrameCount(); i++) {
                _previewFrames.push_back(holder->GetPreviewFrame(i));
                _previewFrameTimes.push_back(holder->GetPreviewFrameTime(i));
            }
        }

        // Add extra info for video
        if (t == MediaType::Video) {
            auto vidEntry = _sequenceMedia->GetVideo(p);
            if (vidEntry && !vidEntry->GetResolvedPath().empty()) {
                long duration = VideoReader::GetVideoLength(vidEntry->GetResolvedPath());
                if (!_previewFrames.empty() && _previewFrames[0]->IsOk()) {
                    info += wxString::Format("\n%dx%d", _previewFrames[0]->GetWidth(), _previewFrames[0]->GetHeight());
                }
                if (duration > 0) {
                    info += wxString::Format("\n%.1fs", duration / 1000.0);
                }
            }
        }

        _infoLabel->SetLabel(info);

        if (!_previewFrames.empty()) {
            ShowPreviewFrame(0);
            if (_previewFrames.size() > 1) {
                long interval = (_previewFrameTimes[0] > 0) ? _previewFrameTimes[0] : 50;
                _previewTimer.Start(interval);
            }
            return;
        }
        refreshPreview(wxNullBitmap);
        return;
    }
    refreshPreview(wxNullBitmap);
    _infoLabel->SetLabel(wxEmptyString);
}

void ManageMediaPanel::OnPreviewTimer(wxTimerEvent& event)
{
    if (_previewFrames.empty()) {
        _previewTimer.Stop();
        return;
    }
    _currentPreviewFrame = (_currentPreviewFrame + 1) % _previewFrames.size();
    ShowPreviewFrame(_currentPreviewFrame);

    long interval = (_currentPreviewFrame < _previewFrameTimes.size() && _previewFrameTimes[_currentPreviewFrame] > 0)
                        ? _previewFrameTimes[_currentPreviewFrame]
                        : 50;
    _previewTimer.Start(interval);
}

void ManageMediaPanel::ShowPreviewFrame(size_t index)
{
    if (index >= _previewFrames.size() || !_previewFrames[index] || !_previewFrames[index]->IsOk()) return;

    const auto& img = _previewFrames[index];
    double scaleFactor = GetContentScaleFactor();

    // Scale to fill the 150x150 preview area, maintaining aspect ratio
    wxSize widgetSize = _preview->GetSize();
    int sw = img->GetWidth();
    int sh = img->GetHeight();
    if (widgetSize.x > 0 && widgetSize.y > 0) {
        double pw = widgetSize.x * scaleFactor;
        double ph = widgetSize.y * scaleFactor;
        double scale = std::min(pw / sw, ph / sh);
        sw = std::max(1, (int)(sw * scale));
        sh = std::max(1, (int)(sh * scale));
    }

    xlImage scaled(*img);
    if (sw != img->GetWidth() || sh != img->GetHeight()) {
        scaled.Rescale(sw, sh);
    }

    wxBitmap bmp(xlImageToWxImage(scaled));
    bmp.SetScaleFactor(scaleFactor);
    _preview->SetBitmap(bmp);
    _preview->Refresh();
}

std::string ManageMediaPanel::StrippedPath(const std::string& fullPath) const
{
    if (_showDirectory.empty()) return {};
    wxString showDir = _showDirectory;
    if (showDir.Last() != wxFileName::GetPathSeparator())
        showDir += wxFileName::GetPathSeparator();
    wxString wx = fullPath;
    if (wx.StartsWith(showDir))
        return ToStdString(wx.Mid(showDir.Length()));
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
        for (int i = 0; i < (int)_sequenceElements->GetElementCount(); ++i) {
            Element* e = _sequenceElements->GetElement(i);
            if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
            ModelElement* model = dynamic_cast<ModelElement*>(e);
            if (!model) continue;

            // Helper lambda to scan one EffectLayer
            auto scanLayer = [&](EffectLayer* layer) {
                for (int k = 0; k < layer->GetEffectCount(); ++k) {
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

            for (int j = 0; j < (int)model->GetEffectLayerCount(); ++j)
                scanLayer(model->GetEffectLayer(j));

            for (int j = 0; j < (int)model->GetSubModelAndStrandCount(); ++j) {
                SubModelElement* sub = model->GetSubModel(j);
                for (int l = 0; l < (int)sub->GetEffectLayerCount(); ++l)
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
    // event.GetPosition() is in inner-window coordinates; HitTest expects
    // _mediaTree client coordinates (which include the header row offset).
    wxPoint pos = inner->ClientToScreen(event.GetPosition());
    pos = _mediaTree->ScreenToClient(pos);
    _mediaTree->HitTest(pos, item, col);

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

        newPath = ToStdString(dlg.GetPath());
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
            for (int k = 0; k < layer->GetEffectCount(); ++k) {
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

        for (int i = 0; i < (int)_sequenceElements->GetElementCount(); ++i) {
            Element* e = _sequenceElements->GetElement(i);
            if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
            ModelElement* model = dynamic_cast<ModelElement*>(e);
            if (!model) continue;

            for (int j = 0; j < (int)model->GetEffectLayerCount(); ++j)
                scanLayer(model->GetEffectLayer(j));

            for (int j = 0; j < (int)model->GetSubModelAndStrandCount(); ++j) {
                SubModelElement* sub = model->GetSubModel(j);
                for (int l = 0; l < (int)sub->GetEffectLayerCount(); ++l)
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

    wxMenu menu;
    // Reload option for non-embedded items
    auto [isEmbedded, isEmbeddable] = _sequenceMedia->GetMediaEmbedState(path);
    if (!isEmbedded) {
        wxMenuItem* reloadItem = menu.Append(wxID_ANY, "Reload from Disk");
        menu.Bind(wxEVT_MENU, [this, path](wxCommandEvent&) {
            OnReloadMedia(path);
        }, reloadItem->GetId());
    }

    // Broken image options
    if (_sequenceMedia->HasImage(path)) {
        auto entry = _sequenceMedia->GetImage(path);
        if (entry && !entry->IsOk()) {
            if (menu.GetMenuItemCount() > 0)
                menu.AppendSeparator();

            wxMenuItem* reselectItem = menu.Append(wxID_ANY, "Re-select Image...");
            menu.Bind(wxEVT_MENU, [this, path](wxCommandEvent&) {
                OnReSelectImage(path);
            }, reselectItem->GetId());

            // Count total broken images to decide whether to offer bulk find
            int brokenCount = 0;
            for (const auto& p : _sequenceMedia->GetImagePaths()) {
                auto e = _sequenceMedia->GetImage(p);
                if (e && !e->IsOk()) ++brokenCount;
            }
            if (brokenCount > 1) {
                wxMenuItem* bulkItem = menu.Append(wxID_ANY, "Bulk Find Images...");
                menu.Bind(wxEVT_MENU, [this](wxCommandEvent&) {
                    OnBulkFindImages();
                }, bulkItem->GetId());
            }
        }
    }

    if (menu.GetMenuItemCount() > 0)
        PopupMenu(&menu);
}

void ManageMediaPanel::OnReloadMedia(const std::string& path)
{
    if (_sequenceMedia == nullptr) return;

    if (_sequenceMedia->ReloadMedia(path)) {
        Populate(path);
    }
}

void ManageMediaPanel::OnReSelectImage(const std::string& oldPath)
{
    if (_sequenceMedia == nullptr) return;

    // Default the file picker to the last used image dir, then show directory, then old path's dir
    wxString defaultDir;
    {
        wxConfigBase* config = wxConfigBase::Get();
        config->Read(LastDirConfigKey(MediaType::Image), &defaultDir);
        if (defaultDir.empty()) {
            wxFileName fn(oldPath);
            if (fn.IsAbsolute() && fn.DirExists())
                defaultDir = fn.GetPath();
            else if (!_showDirectory.empty())
                defaultDir = _showDirectory;
        }
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
        pickedPath = ToStdString(dlg.GetPath());
        wxConfigBase::Get()->Write(LastDirConfigKey(MediaType::Image), wxFileName(pickedPath).GetPath());
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

        std::string importedMediaDir = ImportedMediaPath(_xlFrame, _showDirectory, "Images");

        wxArrayString choices;
        choices.Add("Embed in sequence");
        if (!importedMediaDir.empty())
            choices.Add("Copy to ImportedMedia: " + wxString(importedMediaDir));
        for (const auto& dir : copyTargets)
            choices.Add("Copy to: " + wxString(dir));

        wxSingleChoiceDialog choiceDlg(this,
            wxString::Format("'%s' is outside the show/media folder(s).\nChoose where to place it:",
                             fn.GetFullName()),
            "File Outside Show/Media Folder", choices);
        if (choiceDlg.ShowModal() == wxID_CANCEL) return;

        int sel = choiceDlg.GetSelection();
        bool hasImported = !importedMediaDir.empty();
        // choices: 0=Embed, 1=ImportedMedia (if present), 1or2+=Copy to dirs
        int copyOffset = 1 + (hasImported ? 1 : 0);

        if (sel == 0) {
            // Embed: load from disk, rename cache key to Images/<name>, mark embedded
            _sequenceMedia->GetImage(pickedPath);
            std::string embeddedName = "Images/" + ToStdString(fn.GetFullName());
            int suffix = 1;
            std::string candidate = embeddedName;
            // Avoid colliding with anything other than the old broken entry
            while (_sequenceMedia->HasImage(candidate) && candidate != oldPath) {
                candidate = "Images/" + ToStdString(fn.GetName()) +
                            "_" + std::to_string(suffix++) + "." +
                            ToStdString(fn.GetExt());
            }
            embeddedName = candidate;
            _sequenceMedia->RenameImage(pickedPath, embeddedName);
            _sequenceMedia->EmbedImage(embeddedName);
            finalPath = embeddedName;
        } else if (hasImported && sel == 1) {
            // Copy to ImportedMedia/<seqStem>/Images
            std::string newPath = CopyToDir(pickedPath, importedMediaDir);
            if (newPath.empty()) {
                wxMessageBox("Failed to copy file to ImportedMedia folder.", "Error",
                             wxICON_ERROR | wxOK, this);
                return;
            }
            finalPath = newPath;
        } else {
            // Copy to one of the folders
            std::string targetDir = copyTargets[sel - copyOffset];
            std::string newPath;
            if (_xlFrame && targetDir == _showDirectory) {
                newPath = _xlFrame->MoveToShowFolder(pickedPath, sep + "Images");
            } else {
                wxString dest = wxString(targetDir) + wxString(sep) + "Images";
                if (!wxDirExists(dest)) wxMkdir(dest);
                dest += wxString(sep) + fn.GetFullName();
                if (wxCopyFile(wxString(pickedPath), dest, false))
                    newPath = ToStdString(dest);
            }
            if (newPath.empty()) {
                wxMessageBox("Failed to copy file to the selected folder.", "Error",
                             wxICON_ERROR | wxOK, this);
                return;
            }
            finalPath = newPath;
        }
    }

    // Convert to relative path if inside a show/media folder
    if (_xlFrame) {
        std::string rel = _xlFrame->MakeRelativePath(finalPath);
        if (!rel.empty()) finalPath = rel;
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
            for (int k = 0; k < layer->GetEffectCount(); ++k) {
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

        for (int i = 0; i < (int)_sequenceElements->GetElementCount(); ++i) {
            Element* e = _sequenceElements->GetElement(i);
            if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
            ModelElement* model = dynamic_cast<ModelElement*>(e);
            if (!model) continue;

            for (int j = 0; j < (int)model->GetEffectLayerCount(); ++j)
                scanLayer(model->GetEffectLayer(j));

            for (int j = 0; j < (int)model->GetSubModelAndStrandCount(); ++j) {
                SubModelElement* sub = model->GetSubModel(j);
                for (int l = 0; l < (int)sub->GetEffectLayerCount(); ++l)
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

    std::string searchDir = ToStdString(dlg.GetPath());
    ObtainAccessToURL(searchDir);
    const std::string sep(1, wxFileName::GetPathSeparator());

    // Check if the chosen directory is outside the show/media folders
    bool outsideFolders = _xlFrame ? !_xlFrame->IsInShowOrMediaFolder(searchDir + sep)
                                   : false;

    // If outside, ask how to handle the files once.
    int outsideAction = -1;
    std::vector<std::string> copyTargets;
    std::string bulkImportedMediaDir;
    if (outsideFolders) {
        if (!_showDirectory.empty())
            copyTargets.push_back(_showDirectory);
        if (_xlFrame) {
            for (const auto& md : _xlFrame->GetMediaFolders()) {
                if (md != _showDirectory)
                    copyTargets.push_back(md);
            }
        }

        bulkImportedMediaDir = ImportedMediaPath(_xlFrame, _showDirectory, "Images");

        wxArrayString choices;
        choices.Add("Embed in sequence");
        if (!bulkImportedMediaDir.empty())
            choices.Add("Copy to ImportedMedia: " + wxString(bulkImportedMediaDir));
        for (const auto& dir : copyTargets)
            choices.Add("Copy to: " + wxString(dir));

        wxSingleChoiceDialog choiceDlg(this,
            "The selected folder is outside the show/media folder(s).\n"
            "How should found images be handled?",
            "Files Outside Show/Media Folder", choices);
        if (choiceDlg.ShowModal() == wxID_CANCEL) return;
        int rawSel = choiceDlg.GetSelection();
        // Normalise so 0=embed, 1=importedMedia, 2+=copy regardless of whether
        // the importedMedia option was shown.
        // choices: 0=Embed, 1=ImportedMedia (if present), 1or2+=Copy to dirs
        if (rawSel == 0)
            outsideAction = 0;  // embed
        else if (!bulkImportedMediaDir.empty())
            outsideAction = rawSel;  // importedMedia=1 or copy=2+
        else
            outsideAction = rawSel + 1;  // no importedMedia slot, copy starts at 2
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
            wxString candidate = searchDir + sep + ToStdString(nameToFind);
            if (wxFileExists(candidate)) {
                foundFile = candidate;
            } else {
                // Recurse into subdirectories
                wxString f;
                if (dir.GetFirst(&f, nameToFind, wxDIR_FILES | wxDIR_DIRS)) {
                    foundFile = searchDir + sep + ToStdString(f);
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

        std::string pickedPath = ToStdString(foundFile);
        std::string finalPath = pickedPath;

        if (outsideFolders) {
            wxFileName fn(pickedPath);
            if (outsideAction == 0) {
                // Embed
                _sequenceMedia->GetImage(pickedPath);
                std::string embeddedName = "Images/" + ToStdString(fn.GetFullName());
                int suffix = 1;
                std::string candidate = embeddedName;
                while (_sequenceMedia->HasImage(candidate) && candidate != oldPath) {
                    candidate = "Images/" + ToStdString(fn.GetName()) +
                                "_" + std::to_string(suffix++) + "." +
                                ToStdString(fn.GetExt());
                }
                embeddedName = candidate;
                _sequenceMedia->RenameImage(pickedPath, embeddedName);
                _sequenceMedia->EmbedImage(embeddedName);
                finalPath = embeddedName;
            } else if (outsideAction == 1) {
                // Copy to ImportedMedia/<seqStem>/Images
                std::string newPath = CopyToDir(pickedPath, bulkImportedMediaDir);
                if (newPath.empty()) continue;  // skip this file on failure
                finalPath = newPath;
            } else {
                // Copy to one of the target directories
                std::string targetDir = copyTargets[outsideAction - 2];
                std::string newPath;
                if (_xlFrame && targetDir == _showDirectory) {
                    newPath = _xlFrame->MoveToShowFolder(pickedPath, sep + "Images");
                } else {
                    wxString dest = wxString(targetDir) + wxString(sep) + "Images";
                    if (!wxDirExists(dest)) wxMkdir(dest);
                    dest += wxString(sep) + fn.GetFullName();
                    if (wxCopyFile(wxString(pickedPath), dest, false))
                        newPath = ToStdString(dest);
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
                for (int k = 0; k < layer->GetEffectCount(); ++k) {
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

            for (int i = 0; i < (int)_sequenceElements->GetElementCount(); ++i) {
                Element* e = _sequenceElements->GetElement(i);
                if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
                ModelElement* model = dynamic_cast<ModelElement*>(e);
                if (!model) continue;

                for (int j = 0; j < (int)model->GetEffectLayerCount(); ++j)
                    scanLayer(model->GetEffectLayer(j));

                for (int j = 0; j < (int)model->GetSubModelAndStrandCount(); ++j) {
                    SubModelElement* sub = model->GetSubModel(j);
                    for (int l = 0; l < (int)sub->GetEffectLayerCount(); ++l)
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

int ManageMediaPanel::ResolveAllMissingMedia(wxWindow* parent, SequenceMedia* sequenceMedia,
                                              SequenceElements* sequenceElements,
                                              xLightsFrame* xlFrame,
                                              const std::string& showDirectory)
{
    if (sequenceElements == nullptr || xlFrame == nullptr) return 0;

    // Scan all effects across the entire sequence to find missing file references.
    // We scan effect settings directly rather than relying on the SequenceMedia cache,
    // because cache entries may not exist for files that were never rendered.
    std::set<std::string> brokenPathSet;

    auto scanLayerForBroken = [&](EffectLayer* layer) {
        for (int k = 0; k < layer->GetEffectCount(); ++k) {
            Effect* eff = layer->GetEffect(k);
            RenderableEffect* reff = xlFrame->GetEffectManager().GetEffect(eff->GetEffectIndex());
            if (reff == nullptr) continue;
            auto fileRefs = reff->GetFileReferences(nullptr, eff->GetSettings());
            for (const auto& filePath : fileRefs) {
                if (filePath.empty()) continue;
                std::string resolved = FileUtils::FixFile("", filePath);
                if (resolved.empty() || !FileExists(resolved)) {
                    brokenPathSet.insert(filePath);
                }
            }
        }
    };

    for (int i = 0; i < (int)sequenceElements->GetElementCount(); ++i) {
        Element* e = sequenceElements->GetElement(i);
        if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
        ModelElement* model = dynamic_cast<ModelElement*>(e);
        if (!model) continue;

        for (int j = 0; j < (int)model->GetEffectLayerCount(); ++j)
            scanLayerForBroken(model->GetEffectLayer(j));

        for (int j = 0; j < (int)model->GetSubModelAndStrandCount(); ++j) {
            SubModelElement* sub = model->GetSubModel(j);
            for (int l = 0; l < (int)sub->GetEffectLayerCount(); ++l)
                scanLayerForBroken(sub->GetEffectLayer(l));
            if (sub->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                StrandElement* strand = dynamic_cast<StrandElement*>(sub);
                if (strand) {
                    for (int k = 0; k < strand->GetNodeLayerCount(); ++k)
                        scanLayerForBroken(strand->GetNodeLayer(k));
                }
            }
        }
    }

    std::vector<std::string> brokenPaths(brokenPathSet.begin(), brokenPathSet.end());

    if (brokenPaths.empty()) {
        wxMessageBox("All media files are resolved. No missing files found.",
                     "Resolve Missing Media", wxICON_INFORMATION | wxOK, parent);
        return 0;
    }

    // Build list of directories to search automatically (show dir + media dirs)
    std::vector<std::string> autoDirs;
    if (!showDirectory.empty()) autoDirs.push_back(showDirectory);
    if (xlFrame) {
        for (const auto& md : xlFrame->GetMediaFolders()) {
            if (md != showDirectory)
                autoDirs.push_back(md);
        }
    }

    int found = 0;
    int notFound = 0;
    wxArrayString resolvedDetails;
    wxArrayString unresolvedDetails;

    // Normalize a name for fuzzy comparison: lowercase, strip spaces/underscores/dashes/trailing digits
    auto normalizeName = [](const wxString& name) -> wxString {
        wxString result;
        wxString lower = name.Lower();
        for (size_t i = 0; i < lower.Len(); ++i) {
            wxChar ch = lower[i];
            if (ch != ' ' && ch != '_' && ch != '-')
                result += ch;
        }
        // Strip trailing digits
        while (!result.IsEmpty() && wxIsdigit(result.Last()))
            result.RemoveLast();
        return result;
    };

    // Build a file index for a set of directories (scanned once, used for all lookups)
    struct FileIndex {
        std::map<std::string, wxString> exactMap;        // lowercase filename -> full path
        std::map<std::string, std::vector<wxString>> fuzzyMap;  // normalized base -> list of full paths
    };

    auto buildFileIndex = [&](const std::vector<std::string>& dirs) -> FileIndex {
        FileIndex idx;
        for (const auto& dir : dirs) {
            wxArrayString allFiles;
            wxDir::GetAllFiles(dir, &allFiles, wxEmptyString, wxDIR_FILES);
            for (const auto& f : allFiles) {
                wxFileName fn(f);
                std::string exactKey = fn.GetFullName().Lower().ToStdString();
                if (idx.exactMap.find(exactKey) == idx.exactMap.end()) {
                    idx.exactMap[exactKey] = f;
                }
                std::string fuzzyKey = normalizeName(fn.GetName()).ToStdString();
                if (!fuzzyKey.empty()) {
                    idx.fuzzyMap[fuzzyKey].push_back(f);
                }
            }
        }
        return idx;
    };

    // Helper: search a file index for a missing file (exact + fuzzy)
    auto searchIndexForFile = [&](const std::string& oldPath,
                                  const FileIndex& idx) -> wxString {
        wxFileName fnOld(oldPath);
        wxString nameToFind = fnOld.GetFullName();
        if (nameToFind.IsEmpty()) return wxString();

        // Exact filename match
        std::string exactKey = nameToFind.Lower().ToStdString();
        auto exactIt = idx.exactMap.find(exactKey);
        if (exactIt != idx.exactMap.end()) return exactIt->second;

        // Fuzzy match using normalized names
        wxString baseName = fnOld.GetName();
        wxString missingExt = fnOld.GetExt().Lower();
        if (baseName.IsEmpty()) return wxString();
        std::string fuzzyKey = normalizeName(baseName).ToStdString();

        auto fuzzyIt = idx.fuzzyMap.find(fuzzyKey);
        if (fuzzyIt == idx.fuzzyMap.end() || fuzzyIt->second.empty()) return wxString();

        // Prefer a candidate with the same extension as the missing file
        for (const auto& candidate : fuzzyIt->second) {
            if (wxFileName(candidate).GetExt().Lower() == missingExt) {
                return candidate;
            }
        }
        return fuzzyIt->second[0];
    };

    // Helper: update all effect references for an old path to a new path
    auto updateEffectRefs = [&](const std::string& oldPath, const std::string& newPath) {
        auto scanLayer = [&](EffectLayer* layer) {
            for (int k = 0; k < layer->GetEffectCount(); ++k) {
                Effect* eff = layer->GetEffect(k);
                const SettingsMap& settings = eff->GetSettings();
                std::vector<std::string> keysToUpdate;
                for (auto it = settings.begin(); it != settings.end(); ++it) {
                    if (it->second == oldPath)
                        keysToUpdate.push_back(it->first);
                }
                for (const auto& key : keysToUpdate)
                    eff->SetSetting(key, newPath);
            }
        };

        for (int i = 0; i < (int)sequenceElements->GetElementCount(); ++i) {
            Element* e = sequenceElements->GetElement(i);
            if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
            ModelElement* model = dynamic_cast<ModelElement*>(e);
            if (!model) continue;

            for (int j = 0; j < (int)model->GetEffectLayerCount(); ++j)
                scanLayer(model->GetEffectLayer(j));

            for (int j = 0; j < (int)model->GetSubModelAndStrandCount(); ++j) {
                SubModelElement* sub = model->GetSubModel(j);
                for (int l = 0; l < (int)sub->GetEffectLayerCount(); ++l)
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
    };

    // Phase 1: Automatically search show/media directories
    std::vector<std::string> stillMissing;
    if (!autoDirs.empty()) {
        FileIndex autoIndex = buildFileIndex(autoDirs);
        wxProgressDialog prog("Resolving Media", "Searching show and media folders...",
                              (int)brokenPaths.size(), parent,
                              wxPD_APP_MODAL | wxPD_AUTO_HIDE);
        int index = 0;
        for (const auto& oldPath : brokenPaths) {
            prog.Update(index++);
            wxString foundFile = searchIndexForFile(oldPath, autoIndex);
            if (!foundFile.IsEmpty()) {
                std::string finalPath = foundFile.ToStdString();
                std::string rel = xlFrame->MakeRelativePath(finalPath);
                if (!rel.empty()) finalPath = rel;
                if (finalPath != oldPath) {
                    updateEffectRefs(oldPath, finalPath);
                }
                resolvedDetails.Add(wxFileName(oldPath).GetFullName() + " -> " + wxFileName(finalPath).GetFullName());
                ++found;
            } else {
                stillMissing.push_back(oldPath);
            }
        }
    } else {
        stillMissing = brokenPaths;
    }

    // Phase 2: If files are still missing, ask user to pick an additional folder
    if (!stillMissing.empty()) {
        wxString msg = wxString::Format("Automatically resolved %d file(s).\n"
                                        "%zu file(s) still missing.\n\n"
                                        "Would you like to select another folder to search?",
                                        found, stillMissing.size());
        if (wxMessageBox(msg, "Resolve Missing Media", wxYES_NO | wxICON_QUESTION, parent) == wxYES) {
            wxString defaultDir = showDirectory.empty() ? wxString() : wxString(showDirectory);
            wxDirDialog dlg(parent, "Select folder containing missing media files", defaultDir,
                            wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
            if (dlg.ShowModal() == wxID_OK) {
                std::string userDir = dlg.GetPath().ToStdString();
                ObtainAccessToURL(userDir);
                std::vector<std::string> userDirs = { userDir };
                FileIndex userIndex = buildFileIndex(userDirs);

                wxProgressDialog prog2("Resolving Media", "Searching selected folder...",
                                       (int)stillMissing.size(), parent,
                                       wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT);
                int index = 0;
                bool aborted = false;
                for (const auto& oldPath : stillMissing) {
                    if (!prog2.Update(index++, wxString::Format("Searching for %s...", wxFileName(oldPath).GetFullName()))) {
                        // User aborted — track remaining as unresolved
                        for (size_t r = index - 1; r < stillMissing.size(); ++r) {
                            unresolvedDetails.Add(wxFileName(stillMissing[r]).GetFullName());
                            ++notFound;
                        }
                        aborted = true;
                        break;
                    }
                    wxString foundFile = searchIndexForFile(oldPath, userIndex);
                    if (!foundFile.IsEmpty()) {
                        std::string finalPath = foundFile.ToStdString();
                        std::string rel = xlFrame->MakeRelativePath(finalPath);
                        if (!rel.empty()) finalPath = rel;
                        if (finalPath != oldPath) {
                            updateEffectRefs(oldPath, finalPath);
                        }
                        resolvedDetails.Add(wxFileName(oldPath).GetFullName() + " -> " + wxFileName(finalPath).GetFullName());
                        ++found;
                    } else {
                        unresolvedDetails.Add(wxFileName(oldPath).GetFullName());
                        ++notFound;
                    }
                }
            }
        } else {
            for (const auto& p : stillMissing)
                unresolvedDetails.Add(wxFileName(p).GetFullName());
            notFound = (int)stillMissing.size();
        }
    }

    if (found > 0 || notFound > 0) {
        wxDialog dlg(parent, wxID_ANY, "Resolve Missing Media", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
        auto* sizer = new wxBoxSizer(wxVERTICAL);

        if (found > 0) {
            auto* resolvedHeader = new wxStaticText(&dlg, wxID_ANY, wxString::Format("Resolved %d file(s):", found));
            resolvedHeader->SetFont(resolvedHeader->GetFont().Bold());
            sizer->Add(resolvedHeader, 0, wxALL, 10);

            wxString resolvedText;
            for (const auto& r : resolvedDetails)
                resolvedText += r + "\n";
            auto* resolvedList = new wxTextCtrl(&dlg, wxID_ANY, resolvedText, wxDefaultPosition, wxSize(500, std::min(100, (int)resolvedDetails.GetCount() * 20 + 10)),
                                                wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);
            sizer->Add(resolvedList, 1, wxLEFT | wxRIGHT | wxEXPAND, 10);
        }

        if (notFound > 0) {
            auto* unresolvedHeader = new wxStaticText(&dlg, wxID_ANY, wxString::Format("Unresolved %d file(s):", notFound));
            unresolvedHeader->SetFont(unresolvedHeader->GetFont().Bold());
            sizer->Add(unresolvedHeader, 0, wxALL, 10);

            wxString unresolvedText;
            for (const auto& u : unresolvedDetails)
                unresolvedText += u + "\n";
            auto* unresolvedList = new wxTextCtrl(&dlg, wxID_ANY, unresolvedText, wxDefaultPosition, wxSize(500, std::min(100, (int)unresolvedDetails.GetCount() * 20 + 10)),
                                                  wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);
            sizer->Add(unresolvedList, 1, wxLEFT | wxRIGHT | wxEXPAND, 10);

            auto* tipText = new wxStaticText(&dlg, wxID_ANY, "Use Tools/Check Sequence for more details.");
            sizer->Add(tipText, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);
        }

        auto* btnSizer = dlg.CreateStdDialogButtonSizer(wxOK);
        sizer->Add(btnSizer, 0, wxALL | wxEXPAND, 10);

        dlg.SetSizerAndFit(sizer);
        dlg.CentreOnParent();
        dlg.ShowModal();
    }

    return found;
}

void ManageMediaPanel::OnAddButtonClick(wxCommandEvent& event)
{
    if (_sequenceMedia == nullptr) return;

    wxString defaultDir;
    {
        wxConfigBase* config = wxConfigBase::Get();
        config->Read(LastDirConfigKey(MediaType::Image), &defaultDir);
        if (defaultDir.empty())
            defaultDir = _showDirectory.empty() ? wxString() : wxString(_showDirectory);
    }
    wxFileDialog dlg(this, "Add Image to Sequence", defaultDir, wxEmptyString,
                     "Image files (*.png;*.jpg;*.jpeg;*.gif;*.bmp)|*.png;*.jpg;*.jpeg;*.gif;*.bmp|All files (*.*)|*.*",
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dlg.ShowModal() != wxID_OK) return;

    wxArrayString paths;
    dlg.GetPaths(paths);
    if (!paths.empty())
        wxConfigBase::Get()->Write(LastDirConfigKey(MediaType::Image), wxFileName(paths[0]).GetPath());
    std::string lastPath;
    const std::string sep(1, wxFileName::GetPathSeparator());
    for (const auto& p : paths) {
        std::string path = ToStdString(p);

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

            std::string importedMediaDir = ImportedMediaPath(_xlFrame, _showDirectory, "Images");

            wxArrayString choices;
            choices.Add("Embed in sequence");
            if (!importedMediaDir.empty())
                choices.Add("Copy to ImportedMedia: " + wxString(importedMediaDir));
            for (const auto& dir : copyTargets)
                choices.Add("Copy to: " + wxString(dir));

            wxSingleChoiceDialog choiceDlg(this,
                wxString::Format("'%s' is outside the show/media folder(s).\nChoose where to place it:",
                                 fn.GetFullName()),
                "File Outside Show/Media Folder", choices);
            if (choiceDlg.ShowModal() == wxID_CANCEL) continue;

            int sel = choiceDlg.GetSelection();
            bool hasImported = !importedMediaDir.empty();
            // choices: 0=Embed, 1=ImportedMedia (if present), 1or2+=Copy to dirs
            int copyOffset = 1 + (hasImported ? 1 : 0);

            if (sel == 0) {
                // Embed in sequence
                _sequenceMedia->GetImage(path);
                std::string embeddedName = "Images/" + ToStdString(fn.GetFullName());
                if (_sequenceMedia->HasImage(embeddedName)) {
                    int answer = wxMessageBox(
                        wxString::Format("'%s' is already embedded in the sequence.\n\nReplace it with the selected file?",
                                         fn.GetFullName()),
                        "Image Already Embedded",
                        wxYES_NO | wxCANCEL | wxICON_QUESTION, this);
                    if (answer == wxCANCEL) {
                        _sequenceMedia->RemoveImage(path);
                        continue;
                    }
                    if (answer == wxYES) {
                        _sequenceMedia->RemoveImage(embeddedName);
                        _sequenceMedia->RenameImage(path, embeddedName);
                        _sequenceMedia->EmbedImage(embeddedName);
                        lastPath = embeddedName;
                        continue;
                    }
                }
                int suffix = 1;
                std::string candidate = embeddedName;
                while (_sequenceMedia->HasImage(candidate)) {
                    candidate = "Images/" + ToStdString(fn.GetName()) +
                                "_" + std::to_string(suffix++) + "." +
                                ToStdString(fn.GetExt());
                }
                embeddedName = candidate;
                _sequenceMedia->RenameImage(path, embeddedName);
                _sequenceMedia->EmbedImage(embeddedName);
                lastPath = embeddedName;
                continue;
            } else if (hasImported && sel == 1) {
                // Copy to ImportedMedia/<seqStem>/Images
                std::string newPath = CopyToDir(path, importedMediaDir);
                if (newPath.empty()) {
                    wxMessageBox("Failed to copy file to ImportedMedia folder.", "Error",
                                 wxICON_ERROR | wxOK, this);
                    continue;
                }
                path = newPath;
            } else {
                // One of the copy-to-folder choices
                std::string targetDir = copyTargets[sel - copyOffset];
                std::string newPath;
                if (_xlFrame && targetDir == _showDirectory) {
                    // Check if destination exists with different content before MoveToShowFolder
                    // auto-suffixes it.
                    wxString destDir = wxString(targetDir) + wxString(sep) + "Images";
                    wxString destFile = destDir + wxString(sep) + fn.GetFullName();
                    if (wxFileExists(destFile) && !_xlFrame->FilesMatch(path, ToStdString(destFile))) {
                        int answer = wxMessageBox(
                            wxString::Format("'%s' already exists in the folder.\n\nReplace it with the selected file?",
                                             fn.GetFullName()),
                            "File Already Exists",
                            wxYES_NO | wxCANCEL | wxICON_QUESTION, this);
                        if (answer == wxCANCEL) continue;
                        if (answer == wxYES) {
                            if (wxCopyFile(wxString(path), destFile, true))
                                newPath = ToStdString(destFile);
                        } else {
                            // Add as New — let MoveToShowFolder generate a suffix name
                            newPath = _xlFrame->MoveToShowFolder(path, sep + "Images");
                        }
                    } else {
                        newPath = _xlFrame->MoveToShowFolder(path, sep + "Images");
                    }
                } else {
                    wxString destDir = wxString(targetDir) + wxString(sep) + "Images";
                    if (!wxDirExists(destDir)) wxMkdir(destDir);
                    wxString dest = destDir + wxString(sep) + fn.GetFullName();
                    if (wxFileExists(dest) &&
                        (!_xlFrame || !_xlFrame->FilesMatch(path, ToStdString(dest)))) {
                        int answer = wxMessageBox(
                            wxString::Format("'%s' already exists in the folder.\n\nReplace it with the selected file?",
                                             fn.GetFullName()),
                            "File Already Exists",
                            wxYES_NO | wxCANCEL | wxICON_QUESTION, this);
                        if (answer == wxCANCEL) continue;
                        if (answer == wxYES) {
                            if (wxCopyFile(wxString(path), dest, true))
                                newPath = ToStdString(dest);
                        } else {
                            // Add as New with suffix
                            int suffix = 1;
                            wxString suffixed = destDir + wxString(sep) + fn.GetName() +
                                               wxString::Format("_%d", suffix) + "." + fn.GetExt();
                            while (wxFileExists(suffixed)) {
                                ++suffix;
                                suffixed = destDir + wxString(sep) + fn.GetName() +
                                           wxString::Format("_%d", suffix) + "." + fn.GetExt();
                            }
                            if (wxCopyFile(wxString(path), suffixed, false))
                                newPath = ToStdString(suffixed);
                        }
                    } else {
                        if (wxCopyFile(wxString(path), dest, false))
                            newPath = ToStdString(dest);
                    }
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

        // If the path is already in the cache (e.g. same file added again after
        // being modified on disk), remove it first so GetImage reloads from disk.
        if (_sequenceMedia->HasImage(path))
            _sequenceMedia->RemoveImage(path);
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
    std::string namePath = ToStdString(name);
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
    std::string newPathStr = ToStdString(newPath);

    if (!_sequenceMedia->RenameImage(oldPath, newPathStr)) {
        wxMessageBox("Could not rename: a media entry with that name already exists.",
                     "Rename", wxICON_WARNING | wxOK, this);
        return;
    }

    // Update all effect references oldPath -> newPathStr
    if (_sequenceElements != nullptr) {
        auto scanLayer = [&](EffectLayer* layer) {
            for (int k = 0; k < layer->GetEffectCount(); ++k) {
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

        for (int i = 0; i < (int)_sequenceElements->GetElementCount(); ++i) {
            Element* e = _sequenceElements->GetElement(i);
            if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
            ModelElement* model = dynamic_cast<ModelElement*>(e);
            if (!model) continue;

            for (int j = 0; j < (int)model->GetEffectLayerCount(); ++j)
                scanLayer(model->GetEffectLayer(j));

            for (int j = 0; j < (int)model->GetSubModelAndStrandCount(); ++j) {
                SubModelElement* sub = model->GetSubModel(j);
                for (int l = 0; l < (int)sub->GetEffectLayerCount(); ++l)
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

        bool outside = _xlFrame ? !_xlFrame->IsInShowOrMediaFolder(ToStdString(destDir))
                                : (!_showDirectory.empty() &&
                                   !destDir.StartsWith(wxString(_showDirectory)));
        if (!outside) break;

        wxMessageBox(
            "The selected directory is outside the show/media folder(s).\n"
            "Please choose a directory inside the show or a media folder.",
            "Invalid Location", wxOK | wxICON_WARNING, this);
        defaultDir = destDir;  // keep the user's last choice as starting point
    }
    ObtainAccessToURL(ToStdString(destDir));
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
        std::string newPath = ToStdString(destPath);

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
                for (int k = 0; k < layer->GetEffectCount(); ++k) {
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

            for (int i = 0; i < (int)_sequenceElements->GetElementCount(); ++i) {
                Element* e = _sequenceElements->GetElement(i);
                if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
                ModelElement* model = dynamic_cast<ModelElement*>(e);
                if (!model) continue;

                for (int j = 0; j < (int)model->GetEffectLayerCount(); ++j)
                    scanLayer(model->GetEffectLayer(j));

                for (int j = 0; j < (int)model->GetSubModelAndStrandCount(); ++j) {
                    SubModelElement* sub = model->GetSubModel(j);
                    for (int l = 0; l < (int)sub->GetEffectLayerCount(); ++l)
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
            for (int k = 0; k < layer->GetEffectCount(); ++k) {
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

        for (int i = 0; i < (int)_sequenceElements->GetElementCount(); ++i) {
            Element* e = _sequenceElements->GetElement(i);
            if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
            ModelElement* model = dynamic_cast<ModelElement*>(e);
            if (!model) continue;

            for (int j = 0; j < (int)model->GetEffectLayerCount(); ++j)
                scanLayer(model->GetEffectLayer(j));

            for (int j = 0; j < (int)model->GetSubModelAndStrandCount(); ++j) {
                SubModelElement* sub = model->GetSubModel(j);
                for (int l = 0; l < (int)sub->GetEffectLayerCount(); ++l)
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
                                     std::optional<MediaType> filterType,
                                     const std::string& selectPath)
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
    SetMinSize(wxSize(600, 400));
    Fit();

    // Enable OK when a single leaf item is selected in the panel's tree
    _panel->_mediaTree->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED,
                             &SelectMediaDialog::OnSelectionChanged, this);

    _okButton->Bind(wxEVT_BUTTON, &SelectMediaDialog::OnOK, this);
    _addFromDiskButton->Bind(wxEVT_BUTTON, &SelectMediaDialog::OnAddFromDisk, this);

    // Double-click (or Enter) on a leaf item accepts the selection and closes
    _panel->_mediaTree->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED,
        [this](wxDataViewEvent& evt) {
            auto paths = _panel->GetSelectedPaths();
            if (paths.size() == 1)
                EndModal(wxID_OK);
            evt.Skip();
        });

    // Pre-select the specified media file if provided
    if (!selectPath.empty()) {
        _panel->Populate(selectPath);
    }
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

    wxString defaultDir;
    {
        wxConfigBase* config = wxConfigBase::Get();
        config->Read(LastDirConfigKey(_filterType), &defaultDir);
        if (defaultDir.empty())
            defaultDir = _panel->_showDirectory.empty() ? wxString() : wxString(_panel->_showDirectory);
    }
    wxFileDialog dlg(this, "Select Media File", defaultDir, wxEmptyString,
                     WildcardForMediaType(_filterType),
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() != wxID_OK) return;

    std::string path = ToStdString(dlg.GetPath());
    wxConfigBase::Get()->Write(LastDirConfigKey(_filterType), wxFileName(path).GetPath());
    const std::string sep(1, wxFileName::GetPathSeparator());

    // Determine which MediaType to register as
    MediaType regType = _filterType.has_value() ? *_filterType : MediaType::Image;

    // For videos, check AVFoundation compatibility up front and offer to
    // transcode before any copy/embed handling. Converted output (if any)
    // flows through the normal outside-folders path below.
    if (regType == MediaType::Video) {
        std::string maybe = MaybeConvertIncompatibleVideo(this, path);
        if (maybe.empty()) return;
        path = maybe;
    }

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

        // ImportedMedia option is not shown for shaders
        std::string importedMediaDir;
        if (regType != MediaType::Shader)
            importedMediaDir = ImportedMediaPath(_panel->_xlFrame, _panel->_showDirectory, MediaTypeName(regType));

        wxArrayString choices;
        choices.Add("Embed in sequence");
        if (!importedMediaDir.empty())
            choices.Add("Copy to ImportedMedia: " + wxString(importedMediaDir));
        for (const auto& dir : copyTargets)
            choices.Add("Copy to: " + wxString(dir));

        wxSingleChoiceDialog choiceDlg(this,
            wxString::Format("'%s' is outside the show/media folder(s).\nChoose where to place it:",
                             fn.GetFullName()),
            "File Outside Show/Media Folder", choices);
        if (choiceDlg.ShowModal() == wxID_CANCEL) return;

        int sel = choiceDlg.GetSelection();
        bool hasImported = !importedMediaDir.empty();
        int importedOffset = hasImported ? 1 : 0;

        if (hasImported && sel == 1) {
            // Copy to ImportedMedia/<seqStem>/<type>
            std::string newPath = CopyToDir(path, importedMediaDir);
            if (newPath.empty()) {
                wxMessageBox("Failed to copy file to ImportedMedia folder.", "Error",
                             wxICON_ERROR | wxOK, this);
                return;
            }
            path = newPath;
        } else {
            int adjustedSel = sel - importedOffset;
            if (adjustedSel == 0) {
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
                    std::string embeddedName = "Images/" + ToStdString(fn.GetFullName());
                    int suffix = 1;
                    std::string candidate = embeddedName;
                    while (_panel->_sequenceMedia->HasImage(candidate)) {
                        candidate = "Images/" + ToStdString(fn.GetName()) +
                                    "_" + std::to_string(suffix++) + "." +
                                    ToStdString(fn.GetExt());
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
                std::string targetDir = copyTargets[adjustedSel - 1];
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
                        newPath = ToStdString(dest);
                }
                if (newPath.empty()) {
                    wxMessageBox("Failed to copy file to the selected folder.", "Error",
                                 wxICON_ERROR | wxOK, this);
                    return;
                }
                path = newPath;
            }
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

