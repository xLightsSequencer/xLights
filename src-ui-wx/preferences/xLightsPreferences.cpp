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
#include <vector>

#include <wx/preferences.h>
#include <wx/artprov.h>
#include <wx/bmpbndl.h>
#include <wx/listbook.h>
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

#include "grid_icon.xpm"
#include "settings_panel_icon.xpm"

namespace {
// Shared description of a preferences page so the macOS native editor and the
// desktop list dialog stay in lockstep when pages are added or reordered.
struct PrefPageDef {
    wxString name;
    wxBitmapBundle nativeIcon; // larger icon for the native macOS toolbar
    wxBitmapBundle listIcon;   // uniform small icon for the left-hand list
    std::function<wxWindow*(wxWindow*)> factory;
};
}

class xLightsPreferencesPage : public wxPreferencesPage {
public:
    xLightsPreferencesPage(const wxString &n, const wxBitmapBundle &i, std::function<wxWindow*(wxWindow*)> & f) : wxPreferencesPage(), m_icon(i), m_name(n), m_createFunction(f) {
    }

    virtual wxString GetName() const override {
        return m_name;
    }

    virtual wxBitmapBundle GetIcon() const override {
        return m_icon;
    }
    virtual wxWindow *CreateWindow (wxWindow *parent) override {
#ifdef __WXMSW__
        auto *scrolledWindow = new wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxHSCROLL);
        scrolledWindow->SetScrollRate(10, 10);

        wxWindow *content = m_createFunction(scrolledWindow);
        auto *sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(content, 1, wxEXPAND | wxALL, 2);

        scrolledWindow->SetSizer(sizer);
        scrolledWindow->FitInside();

        const wxSize screenSize = wxGetDisplaySize();
        int screenWidth = screenSize.GetWidth() * 0.90;
        int screenHeight = screenSize.GetHeight() * 0.45;

        int minWidth = std::min(screenWidth, 850);

        int minHeight = std::min(screenHeight, 375);
        int maxHeight = std::max(screenHeight, 250);

        scrolledWindow->SetMinSize(wxSize(minWidth, minHeight));
        scrolledWindow->SetMaxSize(wxSize(screenWidth, maxHeight));
        scrolledWindow->Layout();

        return scrolledWindow;
#else
        wxWindow *w = m_createFunction(parent);
#ifdef __WXOSX__
        //need to set a minimum width or the icons get moved into a flyout menu
        //which is more confusing
        w->SetMinSize(wxSize(500, -1));
#endif
        return w;
#endif
    }

private:
    wxBitmapBundle m_icon;
    wxString m_name;
    std::function<wxWindow*(wxWindow*)> m_createFunction;
};

#ifndef __WXOSX__
// A preferences dialog with a vertical list of pages on the left selects
// the panel shown on the right (instead of tabs across the top). Used on
// Windows/Linux; macOS keeps the native preferences window.
class xlPreferencesListDialog : public wxDialog {
public:
    xlPreferencesListDialog(wxWindow* parent, const std::vector<PrefPageDef>& pages)
        : wxDialog(parent, wxID_ANY, _("Preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
        // Ensure every page's TransferDataTo/FromWindow runs on open and OK.
        SetExtraStyle(GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY);

        auto* topSizer = new wxBoxSizer(wxVERTICAL);
        auto* listbook = new wxListbook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_LEFT);

        std::vector<wxBitmapBundle> images;
        images.reserve(pages.size());
        for (const auto& p : pages) {
            images.push_back(p.listIcon);
        }
        listbook->SetImages(images);

        const wxSize screenSize = wxGetDisplaySize();
        int minWidth = std::min((int)(screenSize.GetWidth() * 0.90), 850);
        int minHeight = std::min((int)(screenSize.GetHeight() * 0.45), 375);

        int idx = 0;
        for (const auto& p : pages) {
            // Wrap each panel in a scrolled window so tall panels stay usable.
            auto* scrolledWindow = new wxScrolledWindow(listbook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxHSCROLL);
            scrolledWindow->SetScrollRate(10, 10);
            wxWindow* content = p.factory(scrolledWindow);
            auto* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(content, 1, wxEXPAND | wxALL, 2);
            scrolledWindow->SetSizer(sizer);
            scrolledWindow->FitInside();
            scrolledWindow->SetMinSize(wxSize(minWidth, minHeight));

            listbook->AddPage(scrolledWindow, p.name, idx == 0, idx);
            ++idx;
        }

        topSizer->Add(listbook, 1, wxEXPAND | wxALL, 5);
        topSizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

        SetSizer(topSizer);
        topSizer->SetSizeHints(this);
        Fit();
        CentreOnParent();
    }
};
#endif

void xLightsFrame::OnMenuItemPreferencesSelected(wxCommandEvent& event)
{
    if (readOnlyMode) {
        wxMessageBox("Preferences are not available in read only mode", "Read Only Mode", wxICON_INFORMATION | wxOK);
        return;
    }

    auto ld = _lowDefinitionRender;

    wxImage gridImage(GRID_ICON_64);
    wxBitmap gridIcon(gridImage);
    wxImage settingsImage(SETTINGS_PANEL_ICON);
    wxBitmap settingIcon(settingsImage);

    const wxSize iconSize(64, 64);
    const wxSize listIconSize(24, 24);

    auto scaledBundle = [](const wxImage& img, const wxSize& sz) {
        return wxBitmapBundle(wxBitmap(img.Scale(sz.GetWidth(), sz.GetHeight(), wxIMAGE_QUALITY_HIGH)));
    };

    std::vector<PrefPageDef> pages;
    pages.push_back({ "Backup",
                      wxArtProvider::GetBitmapBundle(wxART_HARDDISK, wxART_BUTTON, wxSize(28, 28)),
                      wxArtProvider::GetBitmapBundle(wxART_HARDDISK, wxART_BUTTON, listIconSize),
                      [this](wxWindow* p) { return (wxWindow*)(new BackupSettingsPanel(p, this)); } });
    pages.push_back({ "View",
                      wxArtProvider::GetBitmapBundle(wxART_FULL_SCREEN, wxART_BUTTON, iconSize),
                      wxArtProvider::GetBitmapBundle(wxART_FULL_SCREEN, wxART_BUTTON, listIconSize),
                      [this](wxWindow* p) { return (wxWindow*)(new ViewSettingsPanel(p, this)); } });
    pages.push_back({ "Effects Grid",
                      wxBitmapBundle(gridIcon),
                      scaledBundle(gridImage, listIconSize),
                      [this](wxWindow* p) { return (wxWindow*)(new EffectsGridSettingsPanel(p, this)); } });
    pages.push_back({ "Sequences",
                      wxArtProvider::GetBitmapBundle("xlART_SETTINGS", wxART_BUTTON, iconSize),
                      wxArtProvider::GetBitmapBundle("xlART_SETTINGS", wxART_BUTTON, listIconSize),
                      [this](wxWindow* p) { return (wxWindow*)(new SequenceFileSettingsPanel(p, this)); } });
    pages.push_back({ "Output",
                      wxArtProvider::GetBitmapBundle("xlART_OUTPUT_LIGHTS_ON", wxART_BUTTON, iconSize),
                      wxArtProvider::GetBitmapBundle("xlART_OUTPUT_LIGHTS_ON", wxART_BUTTON, listIconSize),
                      [this](wxWindow* p) { return (wxWindow*)(new OutputSettingsPanel(p, this)); } });
    pages.push_back({ "Check Sequence",
                      wxArtProvider::GetBitmapBundle("xlART_SETTINGS", wxART_BUTTON, iconSize),
                      wxArtProvider::GetBitmapBundle("xlART_SETTINGS", wxART_BUTTON, listIconSize),
                      [this](wxWindow* p) { return (wxWindow*)(new CheckSequenceSettingsPanel(p, this)); } });
    pages.push_back({ "Random Effects",
                      wxArtProvider::GetBitmapBundle("xlART_DICE_ICON", wxART_BUTTON, wxSize(28, 28)),
                      wxArtProvider::GetBitmapBundle("xlART_DICE_ICON", wxART_BUTTON, listIconSize),
                      [this](wxWindow* p) { return (wxWindow*)(new RandomEffectsSettingsPanel(p, this)); } });
    pages.push_back({ "Colors",
                      wxArtProvider::GetBitmapBundle("xlART_RENDER_ALL", wxART_BUTTON, iconSize),
                      wxArtProvider::GetBitmapBundle("xlART_RENDER_ALL", wxART_BUTTON, listIconSize),
                      [this](wxWindow* p) { return (wxWindow*)(new ColorManagerSettingsPanel(p, this)); } });
    pages.push_back({ "Other",
                      wxBitmapBundle(settingIcon),
                      scaledBundle(settingsImage, listIconSize),
                      [this](wxWindow* p) { return (wxWindow*)(new OtherSettingsPanel(p, this)); } });
#ifdef ENABLE_SERVICES
    pages.push_back({ "Services",
                      wxArtProvider::GetBitmapBundle("xlART_SETTINGS", wxART_BUTTON, iconSize),
                      wxArtProvider::GetBitmapBundle("xlART_SETTINGS", wxART_BUTTON, listIconSize),
                      [this](wxWindow* p) { return (wxWindow*)(new ServicesPanel(p, _serviceManager.get())); } });
#endif

#ifdef __WXOSX__
    if (!mPreferencesEditor.get()) {
        mPreferencesEditor.reset(new wxPreferencesEditor("Preferences"));
        for (auto& p : pages) {
            std::function<wxWindow*(wxWindow*)> f = p.factory;
            mPreferencesEditor->AddPage(new xLightsPreferencesPage(p.name, p.nativeIcon, f));
        }
    }
    mPreferencesEditor->Show(this);
#else
    xlPreferencesListDialog dlg(this, pages);
    dlg.ShowModal();
#endif

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
