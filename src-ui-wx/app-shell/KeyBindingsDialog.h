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

#include <functional>

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/intl.h>
#include <wx/sizer.h>

#include "preferences/KeyBindingsSettingsPanel.h"

class xLightsFrame;

// Standalone, MODELESS Key Bindings editor. Hosts the filterable, category-
// scoped, described KeyBindingsSettingsPanel so the window can stay open while
// you keep working in Layout/Sequencer. Edits apply to the live KeyBindingMap
// immediately; they are persisted (keyBindings.Save(), via the panel's
// TransferDataFromWindow) when the window is closed.
class KeyBindingsDialog : public wxDialog {
public:
    KeyBindingsDialog(wxWindow* parent, xLightsFrame* frame, std::function<void()> onClosed)
        : wxDialog(parent, wxID_ANY, _("Key Bindings"),
                   wxDefaultPosition, wxDefaultSize,
                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    {
        auto* top = new wxBoxSizer(wxVERTICAL);
        _panel = new KeyBindingsSettingsPanel(this, frame);
        top->Add(_panel, 1, wxEXPAND | wxALL, 5);

        // Save commits the staged edits; Cancel (and closing the window)
        // discards them - editing is not applied in real time.
        auto* btnRow = new wxBoxSizer(wxHORIZONTAL);
        btnRow->AddStretchSpacer(1);
        auto* cancel = new wxButton(this, wxID_CANCEL, _("Cancel"));
        auto* save = new wxButton(this, wxID_SAVE, _("Save"));
        btnRow->Add(cancel, 0, wxRIGHT, 8);
        btnRow->Add(save, 0);
        top->Add(btnRow, 0, wxEXPAND | wxALL, 8);
        save->SetDefault();

        // Size from the panel's content (list columns + editor + wheel); do not
        // hard-code a size, which used to open too small and squash the wheel.
        SetSizerAndFit(top);
        SetMinSize(GetSize());

        Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
            if (_panel != nullptr) _panel->CommitChanges();
            Close();
        }, wxID_SAVE);
        Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { Close(); }, wxID_CANCEL);
        Bind(wxEVT_CLOSE_WINDOW, [this, onClosed](wxCloseEvent&) {
            // No implicit save - closing without Save discards the working copy.
            if (onClosed) {
                onClosed();
            }
            Destroy();
        });
    }

private:
    KeyBindingsSettingsPanel* _panel = nullptr;
};
