/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "TextPanel.h"

#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "ui/shared/controls/BulkEditControls.h"
#include "ui/shared/controls/MediaPickerCtrl.h"
#include "render/FontManager.h"
#include "render/SequenceElements.h"
#include "render/Element.h"
#include "utils/ExternalHooks.h"

TextPanel::TextPanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata, /*deferBuild*/ true) {
    BuildFromJson(metadata);

    // Cache pointers to framework-built controls used by ValidateWindow.
    _textInput = dynamic_cast<wxTextCtrl*>(
        wxWindow::FindWindowByName("ID_TEXTCTRL_Text", this));
    _movementChoice = dynamic_cast<wxChoice*>(
        wxWindow::FindWindowByName("ID_CHOICE_Text_Dir", this));
    _lyricTrackChoice = dynamic_cast<wxChoice*>(
        wxWindow::FindWindowByName("ID_CHOICE_Text_LyricTrack", this));

    // Movement choice change updates the enable state of TextToCenter / NoRepeat.
    if (_movementChoice) {
        _movementChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) {
            ValidateWindow();
            e.Skip();
        });
    }
    // Text input change can disable the file picker / lyric track choice.
    if (_textInput) {
        _textInput->Bind(wxEVT_TEXT, [this](wxCommandEvent& e) {
            ValidateWindow();
            e.Skip();
        });
    }

    ValidateWindow();
}

wxWindow* TextPanel::CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                          const nlohmann::json& prop, int cols) {
    std::string id = prop.value("id", "");
    if (id == "Text_File_Row") {
        return BuildFileRow(parentWin, sizer, cols);
    }
    if (id == "Text_Font_XL_Row") {
        return BuildXLFontRow(parentWin, sizer, cols);
    }
    return nullptr;
}

wxWindow* TextPanel::BuildFileRow(wxWindow* parentWin, wxSizer* sizer, int cols) {
    // Standard "label | control | spacer | spacer" layout to match the
    // surrounding flat 4-col grid alignment. The hidden file picker holds
    // the actual path for serialization (saved as E_FILEPICKERCTRL_Text_File);
    // the MediaPickerCtrl is what the user actually interacts with.
    auto* label = new wxStaticText(parentWin, wxID_ANY, "From File");
    sizer->Add(label, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

    _hiddenFilePicker = new BulkEditFilePickerCtrl(parentWin, wxNewId(),
                                                    wxEmptyString,
                                                    "Select a text file",
                                                    "*.txt",
                                                    wxDefaultPosition, wxDefaultSize,
                                                    wxFLP_FILE_MUST_EXIST | wxFLP_OPEN | wxFLP_USE_TEXTCTRL,
                                                    wxDefaultValidator,
                                                    _T("ID_FILEPICKERCTRL_Text_File"));
    _hiddenFilePicker->Hide();

    _filePickerMedia = new MediaPickerCtrl(parentWin, wxID_ANY, MediaType::TextFile);
    _filePickerMedia->SetLinkedPicker(_hiddenFilePicker);
    sizer->Add(_filePickerMedia, 1, wxALL | wxEXPAND, 2);

    if (cols >= 3) sizer->Add(-1, -1, 1, wxALL, 1);
    if (cols >= 4) sizer->Add(-1, -1, 1, wxALL, 1);

    // File picker change updates the lyric track enable state.
    _hiddenFilePicker->Bind(wxEVT_FILEPICKER_CHANGED, [this](wxCommandEvent& e) {
        if (_hiddenFilePicker) {
            ObtainAccessToURL(_hiddenFilePicker->GetFileName().GetFullPath().ToStdString());
        }
        ValidateWindow();
        e.Skip();
    });

    return _filePickerMedia;
}

wxWindow* TextPanel::BuildXLFontRow(wxWindow* parentWin, wxSizer* sizer, int cols) {
    // The XL Font choice shares the suffix "Text_Font" with the wxFontPicker
    // above it (different prefix though — CHOICE vs FONTPICKER), which the
    // properties_ map can't represent without collision. Building the choice
    // as a custom control with the legacy name keeps the saved key
    // E_CHOICE_Text_Font intact while letting the framework own the
    // wxFontPicker via fontpicker controlType.
    auto* label = new wxStaticText(parentWin, wxID_ANY, "XL Font");
    sizer->Add(label, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    _xlFontChoice = new BulkEditChoice(parentWin, wxNewId(),
                                        wxDefaultPosition, wxDefaultSize,
                                        0, nullptr, 0, wxDefaultValidator,
                                        _T("ID_CHOICE_Text_Font"));

    _xlFontChoice->Append("Use OS Fonts");
    FontManager& font_mgr(FontManager::instance());
    const auto& xl_font_names = font_mgr.get_font_names();
    font_mgr.init();
    for (const auto& name : xl_font_names) {
        _xlFontChoice->Append(name);
    }
    _xlFontChoice->SetSelection(0);

    sizer->Add(_xlFontChoice, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    if (cols >= 3) sizer->Add(-1, -1, 1, wxALL, 1);
    if (cols >= 4) sizer->Add(-1, -1, 1, wxALL, 1);

    return _xlFontChoice;
}

void TextPanel::ValidateWindow() {
    JsonEffectPanel::ValidateWindow();

    // Movement none → Text_To_Center and No_Repeat are meaningless.
    if (_movementChoice) {
        bool moves = (_movementChoice->GetStringSelection() != "none");
        auto* toCenter = wxWindow::FindWindowByName("ID_CHECKBOX_TextToCenter", this);
        auto* noRepeat = wxWindow::FindWindowByName("ID_CHECKBOX_TextNoRepeat", this);
        if (toCenter) toCenter->Enable(moves);
        if (noRepeat) noRepeat->Enable(moves);
    }

    // Text source priority: a non-empty Text input wins; otherwise the file
    // picker wins; otherwise the lyric track is the source.
    bool textHasValue = _textInput && !_textInput->GetValue().IsEmpty();
    bool fileHasValue = _hiddenFilePicker &&
                        FileExists(_hiddenFilePicker->GetFileName().GetFullPath());

    if (_filePickerMedia) {
        _filePickerMedia->Enable(!textHasValue);
    }
    if (_hiddenFilePicker) {
        _hiddenFilePicker->Enable(!textHasValue);
    }
    if (_lyricTrackChoice) {
        bool lyricEnabled = !textHasValue && !fileHasValue && (_lyricTrackChoice->GetCount() > 1);
        _lyricTrackChoice->Enable(lyricEnabled);
    }
}

void TextPanel::SetPanelStatus(Model* /*cls*/) {
    // Populate the lyric track choice with phrase / word entries from the
    // sequence's lyric timing tracks. The legacy panel did this manually too;
    // none of the framework's standard dynamicOptions sources match the
    // "name - Phrases" / "name - Words" format that TextEffect::Render expects.
    if (_lyricTrackChoice == nullptr) return;

    wxString selection = _lyricTrackChoice->GetStringSelection();
    _lyricTrackChoice->Clear();
    _lyricTrackChoice->Append("");

    if (mSequenceElements != nullptr) {
        for (size_t i = 0; i < mSequenceElements->GetElementCount(); i++) {
            Element* e = mSequenceElements->GetElement(i);
            if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING) continue;
            auto* te = dynamic_cast<TimingElement*>(e);
            if (te == nullptr) continue;
            wxString n = e->GetName();
            if (e->GetEffectLayerCount() > 1) {
                if (te->HasLyrics(0)) _lyricTrackChoice->Append(n + " - Phrases");
                if (te->HasLyrics(1)) _lyricTrackChoice->Append(n + " - Words");
            } else {
                if (te->HasLyrics(0)) _lyricTrackChoice->Append(n + " - Phrases");
            }
        }
    }

    if (!selection.empty()) {
        _lyricTrackChoice->SetStringSelection(selection);
    }
    if (_lyricTrackChoice->GetSelection() == wxNOT_FOUND) {
        _lyricTrackChoice->SetSelection(0);
    }

    ValidateWindow();
}
