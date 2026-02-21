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
#include "sequencer/SequenceMedia.h"
#include "sequencer/SequenceElements.h"
#include "sequencer/Element.h"
#include "sequencer/EffectLayer.h"  // also defines NodeLayer
#include "sequencer/Effect.h"
#include "UtilClasses.h"
#include "xLightsMain.h"
#include "ai/AIImageDialog.h"
#include "ai/aiType.h"

#include <wx/filename.h>
#include <wx/filedlg.h>
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/datetime.h>
#include <map>
#include <algorithm>

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

void MediaViewModel::Rebuild(SequenceMedia* media, const std::string& showDirectory)
{
    _groups.clear();

    if (media == nullptr) return;

    // Normalise the show directory: ensure it ends with the path separator
    // so we can do a simple prefix strip.
    wxString showDir = showDirectory;
    if (!showDir.IsEmpty() && showDir.Last() != wxFileName::GetPathSeparator()) {
        showDir += wxFileName::GetPathSeparator();
    }

    // Returns the display path for a file: strips show dir prefix when present.
    auto displayPath = [&](const std::string& fullPath) -> wxString {
        if (!showDir.IsEmpty()) {
            wxString wx = fullPath;
            if (wx.StartsWith(showDir)) {
                return wx.Mid(showDir.Length());
            }
        }
        return wxString(fullPath);
    };

    std::vector<std::string> paths = media->GetImagePaths();

    // Helper to build a leaf node
    auto makeLeaf = [&](const std::string& path, MediaNode* parent) -> std::shared_ptr<MediaNode> {
        auto node = std::make_shared<MediaNode>();
        node->isGroup = false;
        node->filePath = path;   // always the full path for lookups
        node->parent = parent;

        // Display label: just the filename portion of the display path
        wxFileName fn(displayPath(path));
        node->label = fn.GetFullName().ToStdString();

        auto entry = media->GetImage(path);
        if (entry) {
            wxSize sz = entry->GetImageSize();
            node->sizeStr = (sz.x > 0) ? wxString::Format("%dx%d", sz.x, sz.y) : "?";
            node->framesStr = (entry->GetImageCount() > 1)
                ? wxString::Format("%d", entry->GetImageCount()) : "-";
            node->statusStr = entry->IsEmbedded() ? "Embedded" : "External";
        }
        return node;
    };

    // Group all images (external and embedded alike) by their display directory
    std::map<wxString, std::shared_ptr<MediaNode>> dirGroups;
    for (const auto& path : paths) {
        auto entry = media->GetImage(path);
        if (!entry) continue;

        wxFileName fn(displayPath(path));
        wxString dir = fn.GetPath();
        if (dir.IsEmpty()) dir = "(show directory)";

        auto it = dirGroups.find(dir);
        if (it == dirGroups.end()) {
            auto grp = std::make_shared<MediaNode>();
            grp->isGroup = true;
            grp->label = dir.ToStdString();
            grp->parent = nullptr;
            dirGroups[dir] = grp;
            _groups.push_back(grp);
        }

        dirGroups[dir]->children.push_back(makeLeaf(path, dirGroups[dir].get()));
    }
}

wxDataViewItem MediaViewModel::FindItem(const std::string& filePath) const
{
    for (const auto& grp : _groups) {
        for (const auto& child : grp->children) {
            if (child->filePath == filePath)
                return wxDataViewItem(child.get());
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
                                   wxWindowID id)
    : wxPanel(parent, id)
    , _sequenceMedia(sequenceMedia)
    , _sequenceElements(sequenceElements)
    , _xlFrame(xlFrame)
    , _showDirectory(showDirectory)
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
    _addButton->SetToolTip("Add an image file to the sequence media");
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
    _embedButton->SetToolTip("Store this image inside the sequence file");
    _embedButton->Disable();
    rightSizer->Add(_embedButton, 0, wxALL | wxEXPAND, 5);

    _extractButton = new wxButton(this, wxID_ANY, "Extract");
    _extractButton->SetToolTip("Reference this image from an external file path");
    _extractButton->Disable();
    rightSizer->Add(_extractButton, 0, wxALL | wxEXPAND, 5);

    rightSizer->Add(0, 10, 0);

    _embedAllButton = new wxButton(this, wxID_ANY, "Embed All");
    _embedAllButton->SetToolTip("Store all images inside the sequence file");
    rightSizer->Add(_embedAllButton, 0, wxALL | wxEXPAND, 5);

    _extractAllButton = new wxButton(this, wxID_ANY, "Extract All");
    _extractAllButton->SetToolTip("Reference all images from external file paths");
    rightSizer->Add(_extractAllButton, 0, wxALL | wxEXPAND, 5);

    rightSizer->Add(0, 10, 0);

    _removeButton = new wxButton(this, wxID_ANY, "Remove");
    _removeButton->SetToolTip("Remove the selected embedded image(s) from the sequence");
    _removeButton->Disable();
    rightSizer->Add(_removeButton, 0, wxALL | wxEXPAND, 5);

    mainSizer->Add(rightSizer, 0, wxALL | wxEXPAND, 0);

    SetSizer(mainSizer);

    _mediaTree->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &ManageMediaPanel::OnTreeItemSelected, this);
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

    bool hasAI = _xlFrame && !_xlFrame->GetAIServices(aiType::TYPE::IMAGES).empty();
    _aiGenerateButton->Enable(hasAI);

    if (_sequenceMedia == nullptr) {
        _addButton->Disable();
        _renameButton->Disable();
        _embedAllButton->Disable();
        _extractAllButton->Disable();
        return;
    }
    _addButton->Enable();

    _model->Rebuild(_sequenceMedia, _showDirectory);
    _model->Cleared();

    // Expand all top-level groups
    wxDataViewItemArray groups;
    _model->GetChildren(wxDataViewItem(nullptr), groups);
    for (const auto& grp : groups) {
        _mediaTree->Expand(grp);
    }

    // Determine Embed All / Extract All availability
    std::vector<std::string> paths = _sequenceMedia->GetImagePaths();
    bool hasEmbeddable = false, hasExtractable = false;
    for (const auto& path : paths) {
        auto entry = _sequenceMedia->GetImage(path);
        if (!entry) continue;
        if (!entry->IsEmbedded() && entry->IsEmbeddable()) hasEmbeddable = true;
        if (entry->IsEmbedded()) hasExtractable = true;
    }
    _embedAllButton->Enable(hasEmbeddable);
    _extractAllButton->Enable(hasExtractable);

    // Re-select and scroll to the requested path if provided
    if (!selectPath.empty()) {
        wxDataViewItem item = _model->FindItem(selectPath);
        if (item.IsOk()) {
            _mediaTree->Select(item);
            _mediaTree->EnsureVisible(item);
            UpdatePreview(selectPath);
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
    UpdatePreview(paths[0]);
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
    bool canEmbed = false, canExtract = false, canRename = false, canRemove = false;
    for (const auto& path : paths) {
        auto entry = _sequenceMedia->GetImage(path);
        if (!entry) continue;
        if (!entry->IsEmbedded() && entry->IsEmbeddable()) canEmbed = true;
        if (entry->IsEmbedded()) { canExtract = true; canRemove = true; }
    }
    // Rename only for a single embedded selection
    if (paths.size() == 1) {
        auto entry = _sequenceMedia->GetImage(paths[0]);
        canRename = entry && entry->IsEmbedded();
    }
    _renameButton->Enable(canRename);
    _embedButton->Enable(canEmbed);
    _extractButton->Enable(canExtract);
    _removeButton->Enable(canRemove);
}

void ManageMediaPanel::UpdatePreview(const std::string& filepath)
{
    auto refreshPreview = [this](const wxBitmap& bmp) {
        _preview->SetBitmap(bmp);
        _preview->InvalidateBestSize();
        _preview->Refresh();
        _preview->Update();
    };

    if (_sequenceMedia == nullptr) return;

    auto entry = _sequenceMedia->GetImage(filepath);
    if (!entry || !entry->IsOk()) {
        refreshPreview(wxNullBitmap);
        _infoLabel->SetLabel(wxEmptyString);
        return;
    }

    wxSize sz = entry->GetImageSize();
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
        double scale = std::min(150.0 / img->GetWidth(), 150.0 / img->GetHeight());
        int w = std::max(1, (int)(img->GetWidth() * scale));
        int h = std::max(1, (int)(img->GetHeight() * scale));
        refreshPreview(wxBitmap(img->Scale(w, h, wxIMAGE_QUALITY_HIGH)));
    } else {
        refreshPreview(wxNullBitmap);
    }
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
    // Compute the new (stripped) path. If it's the same, just embed as-is.
    std::string newPath = StrippedPath(fullPath);
    if (newPath.empty() || newPath == fullPath) {
        _sequenceMedia->EmbedImage(fullPath);
        return fullPath;
    }

    // Rename in the cache first
    if (!_sequenceMedia->RenameImage(fullPath, newPath)) {
        // Rename failed (e.g. newPath already exists) — embed under old name
        _sequenceMedia->EmbedImage(fullPath);
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
        if (!path.empty() && _sequenceMedia) {
            auto entry = _sequenceMedia->GetImage(path);
            // Show full path tooltip only for external images
            if (entry && !entry->IsEmbedded()) {
                inner->SetToolTip(wxString(path));
                return;
            }
        }
    }
    // No external image under cursor — clear tooltip
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

    wxFileDialog dlg(this, "Save Image As", fn.GetPath(), fn.GetFullName(),
                     wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() != wxID_OK) return {};

    std::string newPath = dlg.GetPath().ToStdString();

    // Write data to disk and rename cache key
    if (!_sequenceMedia->ExtractImageToFile(fullPath, newPath)) {
        wxMessageBox("Failed to save image to:\n" + newPath,
                     "Extract Failed", wxICON_ERROR | wxOK, this);
        return {};
    }

    // Update every effect in the sequence that referenced oldPath -> newPath
    if (_sequenceElements != nullptr && newPath != fullPath) {
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
                    eff->SetSetting(key, newPath);
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

    return newPath;
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
    for (const auto& p : paths) {
        std::string path = p.ToStdString();
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
    _sequenceMedia->AddEmbeddedImage(namePath, img);
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
        auto entry = _sequenceMedia->GetImage(path);
        if (entry && !entry->IsEmbedded() && entry->IsEmbeddable()) {
            std::string newPath = EmbedWithRename(path);
            if (!newPath.empty()) lastNewPath = newPath;
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
        auto entry = _sequenceMedia->GetImage(path);
        if (entry && entry->IsEmbedded()) {
            std::string newPath = ExtractWithRename(path);
            if (!newPath.empty()) lastNewPath = newPath;
        }
    }
    if (!lastNewPath.empty())
        Populate(lastNewPath);
}

void ManageMediaPanel::OnEmbedAllButtonClick(wxCommandEvent& event)
{
    if (_sequenceMedia == nullptr) return;
    // Snapshot paths first since EmbedWithRename may rename keys mid-iteration
    std::vector<std::string> paths = _sequenceMedia->GetImagePaths();
    for (const auto& path : paths) {
        auto entry = _sequenceMedia->GetImage(path);
        if (entry && !entry->IsEmbedded() && entry->IsEmbeddable())
            EmbedWithRename(path);
    }
    Populate();
}

void ManageMediaPanel::OnExtractAllButtonClick(wxCommandEvent& event)
{
    if (_sequenceMedia == nullptr) return;

    // Ask the user for a target directory to save all embedded images
    wxString defaultDir = _showDirectory.empty() ? wxString() : wxString(_showDirectory);
    wxDirDialog dlg(this, "Choose directory to extract images into", defaultDir,
                    wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dlg.ShowModal() != wxID_OK) return;
    wxString destDir = dlg.GetPath();
    if (destDir.Last() != wxFileName::GetPathSeparator())
        destDir += wxFileName::GetPathSeparator();

    std::vector<std::string> paths = _sequenceMedia->GetImagePaths();
    int failed = 0;
    for (const auto& path : paths) {
        auto entry = _sequenceMedia->GetImage(path);
        if (!entry || !entry->IsEmbedded()) continue;

        // Build destination: use just the filename portion of the cache key
        wxFileName fn(path);
        wxString destPath = destDir + fn.GetFullName();
        if (destPath.IsEmpty()) { ++failed; continue; }

        std::string oldPath = path;
        std::string newPath = destPath.ToStdString();

        if (!_sequenceMedia->ExtractImageToFile(oldPath, newPath)) {
            ++failed;
            continue;
        }

        // Update effect references oldPath -> newPath
        if (_sequenceElements != nullptr && newPath != oldPath) {
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
                        eff->SetSetting(key, newPath);
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
        wxMessageBox(wxString::Format("%d image(s) could not be extracted.", failed),
                     "Extract All", wxICON_WARNING | wxOK, this);
    Populate();
}
void ManageMediaPanel::OnRemoveButtonClick(wxCommandEvent& event)
{
    if (_sequenceMedia == nullptr) return;

    // Collect selected paths that are embedded
    std::vector<std::string> toRemove;
    for (const auto& path : GetSelectedPaths()) {
        auto entry = _sequenceMedia->GetImage(path);
        if (entry && entry->IsEmbedded())
            toRemove.push_back(path);
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

    // Warn if any effects reference the images
    if (usageCount > 0) {
        wxString msg = wxString::Format(
            "%d effect(s) reference the selected image(s).\n"
            "Removing them will leave those effects with a missing image.\n\n"
            "Remove anyway?",
            usageCount);
        if (wxMessageBox(msg, "Remove Embedded Image(s)",
                         wxYES_NO | wxNO_DEFAULT | wxICON_WARNING, this) != wxYES)
            return;
    } else {
        wxString msg = wxString::Format(
            "Remove %d embedded image(s) from the sequence?",
            (int)toRemove.size());
        if (wxMessageBox(msg, "Remove Embedded Image(s)",
                         wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION, this) != wxYES)
            return;
    }

    for (const auto& path : toRemove)
        _sequenceMedia->RemoveImage(path);

    Populate();
}

// ---------------------------------------------------------------------------
// SelectMediaDialog
// ---------------------------------------------------------------------------

SelectMediaDialog::SelectMediaDialog(wxWindow* parent, SequenceMedia* sequenceMedia,
                                     SequenceElements* sequenceElements,
                                     const std::string& showDirectory,
                                     xLightsFrame* xlFrame)
    : wxDialog(parent, wxID_ANY, "Select Image", wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

    // Embed the panel (single-select mode via wxDV_SINGLE)
    _panel = new ManageMediaPanel(this, sequenceMedia, sequenceElements,
                                  showDirectory, xlFrame, /*singleSelect=*/true);

    topSizer->Add(_panel, 1, wxEXPAND | wxALL, 0);

    // OK / Cancel buttons
    wxStdDialogButtonSizer* btnSizer = new wxStdDialogButtonSizer();
    _okButton = new wxButton(this, wxID_OK, "OK");
    _okButton->Enable(false);
    btnSizer->AddButton(_okButton);
    btnSizer->AddButton(new wxButton(this, wxID_CANCEL, "Cancel"));
    btnSizer->Realize();
    topSizer->Add(btnSizer, 0, wxEXPAND | wxALL, 8);

    SetSizerAndFit(topSizer);
    Fit();
    //SetSize(wxSize(750, 500));

    // Enable OK when a single leaf item is selected in the panel's tree
    _panel->_mediaTree->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED,
                             &SelectMediaDialog::OnSelectionChanged, this);

    _okButton->Bind(wxEVT_BUTTON, &SelectMediaDialog::OnOK, this);
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

void SelectMediaDialog::OnOK(wxCommandEvent& event)
{
    EndModal(wxID_OK);
}

