/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SequenceFacesPanel.h"

#include "model/FaceMatrixHelpers.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/SequenceElements.h"
#include "shared/utils/wxUtilities.h"
#include "utils/ExternalHooks.h"
#include "utils/FileUtils.h"
#include "xLightsMain.h"

#include <wx/button.h>
#include <wx/choicdlg.h>
#include <wx/choice.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/grid.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textdlg.h>

#include <limits>

SequenceFacesPanel::SequenceFacesPanel(wxWindow* parent, SequenceElements* seqElements,
                                       const std::string& showDirectory, xLightsFrame* xlFrame) :
    wxPanel(parent, wxID_ANY),
    _seqElements(seqElements),
    _showDirectory(showDirectory),
    _xlFrame(xlFrame)
{
    wxFlexGridSizer* mainSizer = new wxFlexGridSizer(0, 1, 0, 0);
    mainSizer->AddGrowableCol(0);
    mainSizer->AddGrowableRow(2);

    wxFlexGridSizer* nameSizer = new wxFlexGridSizer(0, 7, 0, 0);
    nameSizer->AddGrowableCol(1);
    nameSizer->Add(new wxStaticText(this, wxID_ANY, _("Face:")), 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    FaceChoice = new wxChoice(this, wxID_ANY);
    nameSizer->Add(FaceChoice, 1, wxALL | wxEXPAND | wxALIGN_CENTER_VERTICAL, 5);
    Button_Add = new wxButton(this, wxID_ANY, _("Add"));
    nameSizer->Add(Button_Add, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    Button_Delete = new wxButton(this, wxID_ANY, _("Delete"));
    nameSizer->Add(Button_Delete, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    Button_Rename = new wxButton(this, wxID_ANY, _("Rename"));
    nameSizer->Add(Button_Rename, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    Button_ImportFromModel = new wxButton(this, wxID_ANY, _("Import from Model"));
    nameSizer->Add(Button_ImportFromModel, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    Button_DownloadImages = new wxButton(this, wxID_ANY, _("Download Images"));
    nameSizer->Add(Button_DownloadImages, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    mainSizer->Add(nameSizer, 1, wxALL | wxEXPAND, 0);

    wxFlexGridSizer* placementSizer = new wxFlexGridSizer(0, 2, 0, 0);
    placementSizer->AddGrowableCol(1);
    placementSizer->Add(new wxStaticText(this, wxID_ANY, _("Image Placement:")), 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    ImagePlacementChoice = new wxChoice(this, wxID_ANY);
    ImagePlacementChoice->Append(_("Centered"));
    ImagePlacementChoice->Append(_("Scaled"));
    ImagePlacementChoice->Append(_("Scale Keep Aspect Ratio"));
    ImagePlacementChoice->Append(_("Scale Keep Aspect Ratio Crop"));
    ImagePlacementChoice->SetStringSelection("Scaled");
    placementSizer->Add(ImagePlacementChoice, 1, wxALL | wxEXPAND, 5);
    mainSizer->Add(placementSizer, 1, wxALL | wxEXPAND, 0);

    FacesGrid = new wxGrid(this, wxID_ANY);
    FacesGrid->CreateGrid(10, 2);
    FacesGrid->EnableEditing(true);
    FacesGrid->EnableGridLines(true);
    FacesGrid->SetColLabelSize(20);
    FacesGrid->SetRowLabelSize(150);
    FacesGrid->SetDefaultColSize(200, true);
    FacesGrid->SetColLabelValue(0, _("Eyes Open"));
    FacesGrid->SetColLabelValue(1, _("Eyes Closed"));
    int row = 0;
    for (const auto& phoneme : FaceMatrixHelpers::Phonemes()) {
        FacesGrid->SetRowLabelValue(row++, "Mouth - " + phoneme);
    }
    FacesGrid->SetDefaultCellFont(FacesGrid->GetFont());
    FacesGrid->SetDefaultCellTextColour(FacesGrid->GetForegroundColour());
    mainSizer->Add(FacesGrid, 1, wxALL | wxEXPAND, 5);

    SetSizer(mainSizer);

    FaceChoice->Bind(wxEVT_CHOICE, &SequenceFacesPanel::OnFaceChoiceSelect, this);
    Button_Add->Bind(wxEVT_BUTTON, &SequenceFacesPanel::OnButtonAddClick, this);
    Button_Delete->Bind(wxEVT_BUTTON, &SequenceFacesPanel::OnButtonDeleteClick, this);
    Button_Rename->Bind(wxEVT_BUTTON, &SequenceFacesPanel::OnButtonRenameClick, this);
    Button_ImportFromModel->Bind(wxEVT_BUTTON, &SequenceFacesPanel::OnButtonImportFromModelClick, this);
    Button_DownloadImages->Bind(wxEVT_BUTTON, &SequenceFacesPanel::OnButtonDownloadImagesClick, this);
    ImagePlacementChoice->Bind(wxEVT_CHOICE, &SequenceFacesPanel::OnImagePlacementSelect, this);
    FacesGrid->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, &SequenceFacesPanel::OnGridCellLeftDClick, this);
    FacesGrid->Bind(wxEVT_GRID_LABEL_LEFT_DCLICK, &SequenceFacesPanel::OnGridLabelLeftDClick, this);
    FacesGrid->Bind(wxEVT_GRID_CELL_CHANGED, &SequenceFacesPanel::OnGridCellChange, this);

    RefreshFaceList();
}

SequenceFacesPanel::~SequenceFacesPanel()
{
    ApplyPendingRenders();
}

std::string SequenceFacesPanel::GetSelectedFace() const
{
    int sel = FaceChoice->GetSelection();
    if (sel == wxNOT_FOUND) {
        return "";
    }
    return FaceChoice->GetString(sel).ToStdString();
}

void SequenceFacesPanel::RefreshFaceList(const std::string& select)
{
    FaceChoice->Clear();
    if (_seqElements != nullptr) {
        for (const auto& [name, def] : _seqElements->GetSequenceFaces().GetFaces()) {
            FaceChoice->Append(name);
        }
    }
    if (FaceChoice->GetCount() > 0) {
        if (select.empty() || !FaceChoice->SetStringSelection(select)) {
            FaceChoice->SetSelection(0);
        }
    }
    SelectFace(GetSelectedFace());
}

void SequenceFacesPanel::UpdateEnabledState()
{
    bool hasFace = !GetSelectedFace().empty();
    FaceChoice->Enable(FaceChoice->GetCount() > 0);
    Button_Delete->Enable(hasFace);
    Button_Rename->Enable(hasFace);
    Button_DownloadImages->Enable(hasFace);
    ImagePlacementChoice->Enable(hasFace);
    FacesGrid->Enable(hasFace);
}

void SequenceFacesPanel::SelectFace(const std::string& name)
{
    const auto* def = name.empty() || _seqElements == nullptr ? nullptr : _seqElements->GetSequenceFaces().GetFace(name);
    for (int r = 0; r < FacesGrid->GetNumberRows(); r++) {
        for (int c = 0; c < FacesGrid->GetNumberCols(); c++) {
            wxString key = FacesGrid->GetRowLabelValue(r) + "-" + FacesGrid->GetColLabelValue(c);
            key.Replace(" ", "");
            std::string value;
            if (def != nullptr) {
                auto it = def->find(key.ToStdString());
                if (it != def->end()) {
                    value = it->second;
                }
            }
            FacesGrid->SetCellValue(r, c, value);
            ValidateMatrixGrid(r, c);
        }
    }

    // when the key is missing (or holds an unknown value) the renderer scales
    // to fit, so display "Scaled" rather than leaving a stale selection
    std::string placement = "Scaled";
    if (def != nullptr && def->find("ImagePlacement") != def->end()) {
        placement = def->at("ImagePlacement");
    }
    if (!ImagePlacementChoice->SetStringSelection(placement)) {
        ImagePlacementChoice->SetStringSelection("Scaled");
    }
    UpdateEnabledState();
}

void SequenceFacesPanel::ValidateMatrixGrid(int r, int c) const
{
    wxString value = FacesGrid->GetCellValue(r, c);
    // FixFile mirrors how the renderer resolves relative / moved-sequence
    // paths, so a value that renders fine doesn't show red
    bool ok = value.empty() ||
              (_seqElements != nullptr && _seqElements->GetSequenceMedia().HasImage(value.ToStdString())) ||
              FileExists(value) ||
              FileExists(FileUtils::FixFile("", value.ToStdString()));
    if (!ok) {
        FacesGrid->SetCellBackgroundColour(r, c, *wxRED);
    } else {
        FacesGrid->SetCellBackgroundColour(r, c, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
    }
}

void SequenceFacesPanel::TouchFace(const std::string& name)
{
    if (_seqElements == nullptr) {
        return;
    }
    _touchedFaces.insert(name);
    _rendersApplied = false;
    _seqElements->IncrementChangeCount(nullptr);
}

void SequenceFacesPanel::SetFaceImage(const std::string& name, const std::string& key, int r, int c, const std::string& value)
{
    if (_seqElements == nullptr) {
        return;
    }
    auto& faces = _seqElements->GetSequenceFaces();
    const auto* def = faces.GetFace(name);
    SequenceFaces::FaceDefinition d = def != nullptr ? *def : SequenceFaces::FaceDefinition();
    d[key] = value;
    faces.SetFace(name, d);
    if (!value.empty()) {
        auto& media = _seqElements->GetSequenceMedia();
        // register with the media cache (lazily - no decode until first use)
        // so the Media tab lists it and it can be embedded in the .xsq; skip
        // files that don't resolve so a typo doesn't create a broken entry
        if (!media.HasImage(value) && FileExists(FileUtils::FixFile("", value))) {
            media.RegisterImage(value);
        }
        media.MarkUsedByMetadata(value);
        _mediaDirty = true;
    }
    TouchFace(name);
    if (r >= 0 && r < FacesGrid->GetNumberRows() && c >= 0 && c < FacesGrid->GetNumberCols()) {
        FacesGrid->SetCellValue(r, c, value);
        ValidateMatrixGrid(r, c);
    }
}

void SequenceFacesPanel::OnFaceChoiceSelect(wxCommandEvent& event)
{
    SelectFace(GetSelectedFace());
}

void SequenceFacesPanel::OnButtonAddClick(wxCommandEvent& event)
{
    if (_seqElements == nullptr) {
        return;
    }
    wxTextEntryDialog dlg(this, "New Face", "Enter name for new face definition");
    if (dlg.ShowModal() != wxID_OK) {
        return;
    }
    std::string name = dlg.GetValue().Trim(true).Trim(false).ToStdString();
    if (name.empty()) {
        return;
    }
    if (_seqElements->GetSequenceFaces().GetFace(name) != nullptr) {
        wxMessageBox("A sequence face with that name already exists.", "Duplicate name", wxICON_WARNING | wxOK, this);
        return;
    }
    SequenceFaces::FaceDefinition def;
    def["ImagePlacement"] = "Scaled";
    _seqElements->GetSequenceFaces().SetFace(name, def);
    TouchFace(name);
    RefreshFaceList(name);
}

void SequenceFacesPanel::OnButtonDeleteClick(wxCommandEvent& event)
{
    std::string name = GetSelectedFace();
    if (name.empty() || _seqElements == nullptr) {
        return;
    }
    int i = wxMessageBox("Are you sure you want to delete " + name + "?", "Delete Face Definition",
                         wxICON_WARNING | wxOK | wxCANCEL, this);
    if (i == wxID_OK || i == wxOK) {
        _seqElements->GetSequenceFaces().RemoveFace(name);
        TouchFace(name);
        RefreshFaceList();
    }
}

void SequenceFacesPanel::OnButtonRenameClick(wxCommandEvent& event)
{
    std::string name = GetSelectedFace();
    if (name.empty() || _seqElements == nullptr) {
        return;
    }
    wxTextEntryDialog dlg(this, "Rename Face", "Enter new name for " + name, name);
    if (dlg.ShowModal() != wxID_OK) {
        return;
    }
    std::string newName = dlg.GetValue().Trim(true).Trim(false).ToStdString();
    if (newName.empty() || newName == name) {
        return;
    }
    if (!_seqElements->GetSequenceFaces().RenameFace(name, newName)) {
        wxMessageBox("A sequence face with that name already exists.", "Duplicate name", wxICON_WARNING | wxOK, this);
        return;
    }
    RenameFaceReferences(name, newName);
    TouchFace(name);
    TouchFace(newName);
    RefreshFaceList(newName);
}

void SequenceFacesPanel::RenameFaceReferences(const std::string& oldName, const std::string& newName)
{
    if (_seqElements == nullptr) {
        return;
    }

    // Point referencing Faces effects at the new name - unless the effect's
    // model defines oldName itself (model definitions win, so those effects
    // were never using the sequence definition).
    auto modelShadows = [this, &oldName](const std::string& modelName) {
        if (_xlFrame == nullptr) {
            return false;
        }
        Model* m = _xlFrame->AllModels.GetModel(modelName);
        if (m != nullptr && m->GetDisplayAs() == DisplayAsType::ModelGroup) {
            auto* mg = dynamic_cast<ModelGroup*>(m);
            m = mg != nullptr ? mg->GetFirstModel() : nullptr;
        }
        return m != nullptr && m->GetFaceInfo().find(oldName) != m->GetFaceInfo().end();
    };

    auto scanLayer = [&](EffectLayer* layer) {
        for (int k = 0; k < layer->GetEffectCount(); ++k) {
            Effect* eff = layer->GetEffect(k);
            const std::string& effName = eff->GetEffectName();
            if (effName != "Faces" && effName != "CoroFaces") {
                continue;
            }
            if (eff->GetSettings().Get("E_CHOICE_Faces_FaceDefinition", "") == oldName) {
                eff->SetSetting("E_CHOICE_Faces_FaceDefinition", newName);
            }
        }
    };

    for (int i = 0; i < (int)_seqElements->GetElementCount(); ++i) {
        Element* e = _seqElements->GetElement(i);
        if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) {
            continue;
        }
        ModelElement* model = dynamic_cast<ModelElement*>(e);
        if (model == nullptr || modelShadows(model->GetModelName())) {
            continue;
        }

        for (int j = 0; j < (int)model->GetEffectLayerCount(); ++j) {
            scanLayer(model->GetEffectLayer(j));
        }
        for (int j = 0; j < (int)model->GetSubModelAndStrandCount(); ++j) {
            SubModelElement* sub = model->GetSubModel(j);
            for (int l = 0; l < (int)sub->GetEffectLayerCount(); ++l) {
                scanLayer(sub->GetEffectLayer(l));
            }
        }
    }
}

void SequenceFacesPanel::OnButtonImportFromModelClick(wxCommandEvent& event)
{
    if (_seqElements == nullptr || _xlFrame == nullptr) {
        return;
    }

    // every Matrix face definition on every model, as "model: face"
    wxArrayString choices;
    std::vector<std::pair<std::string, std::string>> sources; // model, face
    for (const auto& [modelName, model] : _xlFrame->AllModels) {
        if (model == nullptr) {
            continue;
        }
        for (const auto& [faceName, def] : model->GetFaceInfo()) {
            auto type = def.find("Type");
            if (type != def.end() && type->second == "Matrix") {
                choices.Add(modelName + ": " + faceName);
                sources.emplace_back(modelName, faceName);
            }
        }
    }

    if (choices.IsEmpty()) {
        wxMessageBox("No models have Matrix (image) face definitions.", "Nothing to import", wxICON_INFORMATION | wxOK, this);
        return;
    }

    wxMultiChoiceDialog dlg(this, "Select the model face definitions to copy into this sequence", "Import Faces from Models", choices);
    if (dlg.ShowModal() != wxID_OK) {
        return;
    }

    std::string lastAdded;
    wxString skipped;
    for (auto const& idx : dlg.GetSelections()) {
        const auto& [modelName, faceName] = sources[idx];
        Model* model = _xlFrame->AllModels.GetModel(modelName);
        if (model == nullptr) {
            continue;
        }
        if (_seqElements->GetSequenceFaces().GetFace(faceName) != nullptr) {
            skipped += (skipped.empty() ? "" : ", ") + faceName;
            continue;
        }
        auto it = model->GetFaceInfo().find(faceName);
        if (it == model->GetFaceInfo().end()) {
            continue;
        }
        _seqElements->GetSequenceFaces().SetFace(faceName, it->second);
        for (const auto& [key, value] : it->second) {
            if (SequenceFaces::IsImageKey(key) && !value.empty()) {
                auto& media = _seqElements->GetSequenceMedia();
                if (!media.HasImage(value) && FileExists(FileUtils::FixFile("", value))) {
                    media.RegisterImage(value);
                }
                media.MarkUsedByMetadata(value);
                _mediaDirty = true;
            }
        }
        TouchFace(faceName);
        lastAdded = faceName;
    }

    if (!skipped.empty()) {
        wxMessageBox("Already defined in this sequence, not overwritten: " + skipped, "Some faces skipped", wxICON_INFORMATION | wxOK, this);
    }
    if (!lastAdded.empty()) {
        RefreshFaceList(lastAdded);
    }
}

void SequenceFacesPanel::OnButtonDownloadImagesClick(wxCommandEvent& event)
{
    std::string name = GetSelectedFace();
    if (name.empty()) {
        return;
    }
    // sequence faces have no single model to size against - the download
    // dialog only uses the size to sort candidates, so use a large matrix
    FaceMatrixHelpers::DownloadFaceImages(
        this, _showDirectory, 100, 100,
        [this, &name](const std::string& key, int r, int c, const std::string& value) {
            SetFaceImage(name, key, r, c, value);
        });
}

void SequenceFacesPanel::OnImagePlacementSelect(wxCommandEvent& event)
{
    std::string name = GetSelectedFace();
    if (name.empty() || _seqElements == nullptr) {
        return;
    }
    auto& faces = _seqElements->GetSequenceFaces();
    const auto* def = faces.GetFace(name);
    SequenceFaces::FaceDefinition d = def != nullptr ? *def : SequenceFaces::FaceDefinition();
    d["ImagePlacement"] = ImagePlacementChoice->GetStringSelection().ToStdString();
    faces.SetFace(name, d);
    TouchFace(name);
}

void SequenceFacesPanel::SelectMatrixImage(int r, int c)
{
    std::string name = GetSelectedFace();
    if (name.empty()) {
        return;
    }
    wxString key = FacesGrid->GetRowLabelValue(r) + "-" + FacesGrid->GetColLabelValue(c);
    wxFileName fname(FacesGrid->GetCellValue(r, c));
    wxFileDialog dlg(this, "Choose Image File for " + key, fname.GetPath(),
                     wxEmptyString,
                     FaceMatrixHelpers::SupportedImageTypesFilter(),
                     wxFD_OPEN | wxFD_CHANGE_DIR);
    if (dlg.ShowModal() != wxID_OK) {
        return;
    }
    std::string new_filename = dlg.GetPath().ToStdString();
    ObtainAccessToURL(new_filename);
    key.Replace(" ", "");
    SetFaceImage(name, key.ToStdString(), r, c, new_filename);

    FaceMatrixHelpers::AutoFillMatrixPhonemes(
        new_filename, key.ToStdString(), c,
        [this, &name](const std::string& k) {
            const auto* def = _seqElements->GetSequenceFaces().GetFace(name);
            if (def == nullptr) return std::string();
            auto it = def->find(k);
            return it == def->end() ? std::string() : it->second;
        },
        [this, &name](const std::string& k, int row, int col, const std::string& value) {
            SetFaceImage(name, k, row, col, value);
        });
}

void SequenceFacesPanel::OnGridCellLeftDClick(wxGridEvent& event)
{
    SelectMatrixImage(event.GetRow(), event.GetCol());
}

void SequenceFacesPanel::OnGridLabelLeftDClick(wxGridEvent& event)
{
    if (event.GetRow() < 0) {
        return;
    }
    SelectMatrixImage(event.GetRow(), 0); // update eyes open column
}

void SequenceFacesPanel::OnGridCellChange(wxGridEvent& event)
{
    std::string name = GetSelectedFace();
    if (name.empty()) {
        return;
    }
    int r = event.GetRow();
    int c = event.GetCol();
    wxString key = FacesGrid->GetRowLabelValue(r) + "-" + FacesGrid->GetColLabelValue(c);
    key.Replace(" ", "");
    wxString value = FacesGrid->GetCellValue(r, c);
    value.Trim().Trim(false);
    if (value.StartsWith("\"")) {
        value = value.SubString(1, value.size());
    }
    if (value.EndsWith("\"")) {
        value = value.Truncate(value.size() - 1);
    }
    if (!value.IsEmpty()) {
        // hand-typed paths need a sandbox bookmark just like picked ones
        ObtainAccessToURL(value.ToStdString());
    }
    SetFaceImage(name, key.ToStdString(), r, c, value.ToStdString());
}

void SequenceFacesPanel::ApplyPendingRenders()
{
    if (_rendersApplied || _touchedFaces.empty() || _seqElements == nullptr || _xlFrame == nullptr) {
        return;
    }
    _rendersApplied = true;

    // Definition content lives outside effect settings, so effects using an
    // edited definition must be explicitly re-rendered. "Default"/empty
    // selections may resolve to a sequence face, so include those too.
    std::map<std::string, std::pair<int, int>> dirtyModels;
    const auto initRange = std::make_pair(std::numeric_limits<int>::max(), 0);

    auto scanLayer = [&](EffectLayer* layer, const std::string& modelName) {
        for (int k = 0; k < layer->GetEffectCount(); ++k) {
            Effect* eff = layer->GetEffect(k);
            const std::string& effName = eff->GetEffectName();
            if (effName != "Faces" && effName != "CoroFaces") {
                continue;
            }
            std::string face = eff->GetSettings().Get("E_CHOICE_Faces_FaceDefinition", "Default");
            if (face != "Default" && face != "" && _touchedFaces.find(face) == _touchedFaces.end()) {
                continue;
            }
            auto& range = dirtyModels.emplace(modelName, initRange).first->second;
            range.first = std::min(range.first, eff->GetStartTimeMS());
            range.second = std::max(range.second, eff->GetEndTimeMS());
        }
    };

    for (int i = 0; i < (int)_seqElements->GetElementCount(); ++i) {
        Element* e = _seqElements->GetElement(i);
        if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) {
            continue;
        }
        ModelElement* model = dynamic_cast<ModelElement*>(e);
        if (model == nullptr) {
            continue;
        }

        const std::string& modelName = model->GetModelName();
        for (int j = 0; j < (int)model->GetEffectLayerCount(); ++j) {
            scanLayer(model->GetEffectLayer(j), modelName);
        }

        for (int j = 0; j < (int)model->GetSubModelAndStrandCount(); ++j) {
            SubModelElement* sub = model->GetSubModel(j);
            for (int l = 0; l < (int)sub->GetEffectLayerCount(); ++l) {
                scanLayer(sub->GetEffectLayer(l), modelName);
            }
        }
    }

    for (const auto& [name, range] : dirtyModels) {
        _xlFrame->RenderEffectForModel(name, range.first, range.second);
    }
}
