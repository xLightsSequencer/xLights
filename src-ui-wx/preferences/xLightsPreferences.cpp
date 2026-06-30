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
#include <string>
#include <vector>

#include <wx/bmpbndl.h>
#include <wx/treebook.h>
#include <wx/scrolwin.h>

#include "xLightsMain.h"

#include "ViewSettingsPanel.h"
#include "EffectsGridSettingsPanel.h"
#include "SequenceFileSettingsPanel.h"
#include "BackupSettingsPanel.h"
#include "OutputSettingsPanel.h"
#include "RandomEffectsSettingsPanel.h"
#include "ColorManagerSettingsPanel.h"
#include "OtherSettingsPanel.h"
#include "CheckSequenceSettingsPanel.h"
#include "ServicesPanel.h"
#include "KeyBindingsSettingsPanel.h"

namespace {
// Description of a preferences page: name, left-list icon, and a factory that
// builds the panel. Pages render in a left-hand list on every platform.
struct PrefPageDef {
    wxString name;
    wxBitmapBundle icon;
    std::function<wxWindow*(wxWindow*)> factory;
};

// Build a crisp, colourful page icon: a rounded coloured tile with a white
// glyph (macOS-Settings style). The tile carries its own background, so the
// icon stays legible in both light and dark mode; SVG keeps it sharp at any
// DPI. The glyph body uses "%C%" for any filled dots (substituted with white).
wxBitmapBundle PrefSvgIcon(const std::string& glyph, const std::string& tile) {
    // Custom raw-string delimiter so the ")" inside scale(...) etc. can't close
    // the literal early. %T% = tile colour, %C% = white (filled dots).
    std::string svg = std::string(R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><rect x="0" y="0" width="24" height="24" rx="5.5" fill="%T%"/><g transform="translate(3.5 3.5) scale(0.708)" fill="none" stroke="#FFFFFF" stroke-width="2.4" stroke-linecap="round" stroke-linejoin="round">)SVG")
        + glyph + R"SVG(</g></svg>)SVG";
    for (size_t p = svg.find("%T%"); p != std::string::npos; p = svg.find("%T%")) svg.replace(p, 3, tile);
    for (size_t p = svg.find("%C%"); p != std::string::npos; p = svg.find("%C%")) svg.replace(p, 3, "#FFFFFF");
    return wxBitmapBundle::FromSVG(svg.c_str(), wxSize(24, 24));
}

// A preferences dialog with a vertical list of pages on the left that selects
// the panel shown on the right. Used on every platform so Preferences matches
// the rest of the xLights UI.
class xlPreferencesListDialog : public wxDialog {
public:
    xlPreferencesListDialog(wxWindow* parent, const std::vector<PrefPageDef>& pages)
        : wxDialog(parent, wxID_ANY, _("Preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
        // Ensure every page's TransferDataTo/FromWindow runs on open and OK.
        SetExtraStyle(GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY);

        auto* topSizer = new wxBoxSizer(wxVERTICAL);
        auto* book = new wxTreebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);

        std::vector<wxBitmapBundle> images;
        images.reserve(pages.size());
        for (const auto& p : pages) {
            images.push_back(p.icon);
        }
        book->SetImages(images);

        const wxSize screenSize = wxGetDisplaySize();
        int minWidth = std::min((int)(screenSize.GetWidth() * 0.90), 850);
        int minHeight = std::min((int)(screenSize.GetHeight() * 0.45), 375);

        int idx = 0;
        for (const auto& p : pages) {
            // Wrap each panel in a scrolled window so tall panels stay usable.
            auto* scrolledWindow = new wxScrolledWindow(book, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxHSCROLL);
            scrolledWindow->SetScrollRate(10, 10);
            wxWindow* content = p.factory(scrolledWindow);
            auto* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(content, 1, wxEXPAND | wxALL, 2);
            scrolledWindow->SetSizer(sizer);
            scrolledWindow->FitInside();
            scrolledWindow->SetMinSize(wxSize(minWidth, minHeight));

            book->AddPage(scrolledWindow, p.name, idx == 0, idx);
            ++idx;
        }

        topSizer->Add(book, 1, wxEXPAND | wxALL, 5);
        topSizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

        SetSizer(topSizer);
        topSizer->SetSizeHints(this);
        Fit();
        CentreOnParent();
    }
};
}

void xLightsFrame::OnMenuItemPreferencesSelected(wxCommandEvent& event)
{
    if (readOnlyMode) {
        wxMessageBox("Preferences are not available in read only mode", "Read Only Mode", wxICON_INFORMATION | wxOK);
        return;
    }

    auto ld = _lowDefinitionRender;

    std::vector<PrefPageDef> pages;
    pages.push_back({ "Backup",
                      PrefSvgIcon(R"(<rect x="3" y="4" width="18" height="4" rx="1"/><path d="M5 8v11a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1V8"/><path d="M10 12h4"/>)", "#3B82F6"),
                      [this](wxWindow* p) { return (wxWindow*)(new BackupSettingsPanel(p, this)); } });
    pages.push_back({ "View",
                      PrefSvgIcon(R"(<path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7-10-7-10-7z"/><circle cx="12" cy="12" r="3"/>)", "#14B8A6"),
                      [this](wxWindow* p) { return (wxWindow*)(new ViewSettingsPanel(p, this)); } });
    pages.push_back({ "Effects Grid",
                      PrefSvgIcon(R"(<rect x="4" y="4" width="7" height="7" rx="1"/><rect x="13" y="4" width="7" height="7" rx="1"/><rect x="4" y="13" width="7" height="7" rx="1"/><rect x="13" y="13" width="7" height="7" rx="1"/>)", "#8B5CF6"),
                      [this](wxWindow* p) { return (wxWindow*)(new EffectsGridSettingsPanel(p, this)); } });
    pages.push_back({ "Sequences",
                      PrefSvgIcon(R"(<path d="M4 7h16"/><path d="M4 12h16"/><path d="M4 17h16"/><circle cx="8" cy="7" r="1.7" fill="%C%" stroke="none"/><circle cx="14" cy="12" r="1.7" fill="%C%" stroke="none"/><circle cx="10" cy="17" r="1.7" fill="%C%" stroke="none"/>)", "#6366F1"),
                      [this](wxWindow* p) { return (wxWindow*)(new SequenceFileSettingsPanel(p, this)); } });
    pages.push_back({ "Output",
                      PrefSvgIcon(R"(<path d="M9 18h6"/><path d="M10 21h4"/><path d="M12 3a6 6 0 0 0-4 10c.7.6 1 1.4 1 2h6c0-.6.3-1.4 1-2a6 6 0 0 0-4-10z"/>)", "#F59E0B"),
                      [this](wxWindow* p) { return (wxWindow*)(new OutputSettingsPanel(p, this)); } });
    pages.push_back({ "Check Sequence",
                      PrefSvgIcon(R"(<rect x="5" y="4" width="14" height="17" rx="2"/><rect x="9" y="2" width="6" height="3" rx="1"/><path d="M9 13l2 2 4-4"/>)", "#22C55E"),
                      [this](wxWindow* p) { return (wxWindow*)(new CheckSequenceSettingsPanel(p, this)); } });
    pages.push_back({ "Random Effects",
                      PrefSvgIcon(R"(<rect x="4" y="4" width="16" height="16" rx="3"/><circle cx="9" cy="9" r="1.3" fill="%C%" stroke="none"/><circle cx="15" cy="9" r="1.3" fill="%C%" stroke="none"/><circle cx="12" cy="12" r="1.3" fill="%C%" stroke="none"/><circle cx="9" cy="15" r="1.3" fill="%C%" stroke="none"/><circle cx="15" cy="15" r="1.3" fill="%C%" stroke="none"/>)", "#EC4899"),
                      [this](wxWindow* p) { return (wxWindow*)(new RandomEffectsSettingsPanel(p, this)); } });
    pages.push_back({ "Colors",
                      PrefSvgIcon(R"(<path d="M12 3a9 9 0 1 0 0 18c1 0 1.5-1 1-2-.4-.8 0-2 1-2h2a4 4 0 0 0 4-4c0-5-4-8-8-8z"/><circle cx="8" cy="11" r="1.2" fill="%C%" stroke="none"/><circle cx="12" cy="7.5" r="1.2" fill="%C%" stroke="none"/><circle cx="16" cy="11" r="1.2" fill="%C%" stroke="none"/>)", "#F97316"),
                      [this](wxWindow* p) { return (wxWindow*)(new ColorManagerSettingsPanel(p, this)); } });
    pages.push_back({ "Other",
                      PrefSvgIcon(R"(<path d="M4 8h9"/><path d="M17 8h3"/><circle cx="15" cy="8" r="2"/><path d="M4 16h3"/><path d="M11 16h9"/><circle cx="9" cy="16" r="2"/>)", "#64748B"),
                      [this](wxWindow* p) { return (wxWindow*)(new OtherSettingsPanel(p, this)); } });
    pages.push_back({ "Key Bindings",
                      PrefSvgIcon(R"(<rect x="2" y="6" width="20" height="12" rx="2"/><circle cx="6.5" cy="10.5" r="0.8" fill="%C%" stroke="none"/><circle cx="10" cy="10.5" r="0.8" fill="%C%" stroke="none"/><circle cx="13.5" cy="10.5" r="0.8" fill="%C%" stroke="none"/><circle cx="17.5" cy="10.5" r="0.8" fill="%C%" stroke="none"/><path d="M8 14.5h8"/>)", "#0EA5E9"),
                      [this](wxWindow* p) { return (wxWindow*)(new KeyBindingsSettingsPanel(p, this)); } });
#ifdef ENABLE_SERVICES
    pages.push_back({ "Services",
                      PrefSvgIcon(R"(<path d="M9 2v6"/><path d="M15 2v6"/><path d="M6 8h12v2a6 6 0 0 1-12 0z"/><path d="M12 16v6"/>)", "#F43F5E"),
                      [this](wxWindow* p) { return (wxWindow*)(new ServicesPanel(p, _serviceManager.get())); } });
#endif

    xlPreferencesListDialog dlg(this, pages);
    dlg.ShowModal();

    if (mRenderOnSave) {
        MainToolBar->SetToolShortHelp(ID_AUITOOLBAR_SAVE, _("Render All and Save"));
        MainToolBar->SetToolShortHelp(ID_AUITOOLBAR_SAVEAS, _("Render All and Save As"));
        MainToolBar->Realize();
    } else {
        MainToolBar->SetToolShortHelp(ID_AUITOOLBAR_SAVE, _("Save"));
        MainToolBar->SetToolShortHelp(ID_AUITOOLBAR_SAVEAS, _("Save As"));
        MainToolBar->Realize();
    }

    ResizeMainSequencer(); // just in case row height has changed

    if (ld != _lowDefinitionRender) {
            // just in case the user changes the low resolution renderer
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "Preferences Change");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Preferences Change");
    }
}
