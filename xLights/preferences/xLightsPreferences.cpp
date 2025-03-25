/***************************************************************
* This source files comes from the xLights project
* https://www.xlights.org
* https://github.com/xLightsSequencer/xLights
* See the github commit history for a record of contributing
* developers.
* Copyright claimed based on commit dates recorded in Github
* License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
**************************************************************/

#include <wx/preferences.h>
#include <wx/artprov.h>
#include <wx/scrolwin.h>

#include "../xLightsMain.h"

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

class xLightsPreferencesPage : public wxPreferencesPage {
public:
    xLightsPreferencesPage(const wxString &n, const wxBitmap &i, std::function<wxWindow*(wxWindow*)> & f) : wxPreferencesPage(), m_name(n), m_icon(i), m_createFunction(f) {
    }

    virtual wxString GetName() const override {
        return m_name;
    }

    virtual wxBitmap GetLargeIcon() const override {
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
    wxBitmap m_icon;
    wxString m_name;
    std::function<wxWindow*(wxWindow*)> m_createFunction;
};

void xLightsFrame::OnMenuItemPreferencesSelected(wxCommandEvent& event)
{
    if (readOnlyMode) {
        wxMessageBox("Preferences are not available in read only mode", "Read Only Mode", wxICON_INFORMATION | wxOK);
        return;
    }

    auto ld = _lowDefinitionRender;

    if (!mPreferencesEditor.get()) {
        wxImage gridImage(GRID_ICON_64);
        wxBitmap gridIcon(gridImage);
        wxImage settingsImage(SETTINGS_PANEL_ICON);
        wxBitmap settingIcon(settingsImage);

        mPreferencesEditor.reset(new wxPreferencesEditor("Preferences"));
        std::function<wxWindow*(wxWindow*)> f = [this] (wxWindow *p) { return (wxWindow*)(new BackupSettingsPanel(p, this));};
        mPreferencesEditor->AddPage(new xLightsPreferencesPage("Backup", wxArtProvider::GetBitmap(wxART_HARDDISK, wxART_BUTTON, wxSize(64, 64)), f));

        f = [this] (wxWindow *p) { return (wxWindow*)(new ViewSettingsPanel(p, this));};
        mPreferencesEditor->AddPage(new xLightsPreferencesPage("View", wxArtProvider::GetBitmap(wxART_FULL_SCREEN, wxART_BUTTON, wxSize(64, 64)), f));

        f = [this] (wxWindow *p) { return (wxWindow*)(new EffectsGridSettingsPanel(p, this));};
        mPreferencesEditor->AddPage(new xLightsPreferencesPage("Effects Grid", gridIcon, f));

        f = [this] (wxWindow *p) { return (wxWindow*)(new SequenceFileSettingsPanel(p, this));};
        mPreferencesEditor->AddPage(new xLightsPreferencesPage("Sequences", wxArtProvider::GetBitmap("xlART_SETTINGS", wxART_BUTTON, wxSize(64, 64)), f));

        f = [this] (wxWindow *p) { return (wxWindow*)(new OutputSettingsPanel(p, this));};
        mPreferencesEditor->AddPage(new xLightsPreferencesPage("Output", wxArtProvider::GetBitmap("xlART_OUTPUT_LIGHTS_ON", wxART_BUTTON, wxSize(64, 64)), f));

        f = [this](wxWindow* p) { return (wxWindow*)(new CheckSequenceSettingsPanel(p, this)); };
        mPreferencesEditor->AddPage(new xLightsPreferencesPage("Check Sequence", wxArtProvider::GetBitmap("xlART_SETTINGS", wxART_BUTTON, wxSize(64, 64)), f));

        f = [this] (wxWindow *p) { return (wxWindow*)(new RandomEffectsSettingsPanel(p, this));};
        mPreferencesEditor->AddPage(new xLightsPreferencesPage("Random Effects", wxArtProvider::GetBitmap("xlART_DICE_ICON", wxART_BUTTON, wxSize(64, 64)), f));

        f = [this] (wxWindow *p) { return (wxWindow*)(new ColorManagerSettingsPanel(p, this));};
        mPreferencesEditor->AddPage(new xLightsPreferencesPage("Colors", wxArtProvider::GetBitmap("xlART_RENDER_ALL", wxART_BUTTON, wxSize(64, 64)), f));

        f = [this] (wxWindow *p) { return (wxWindow*)(new OtherSettingsPanel(p, this));};
        mPreferencesEditor->AddPage(new xLightsPreferencesPage("Other", settingIcon, f));

#ifdef ENABLE_SERVICES
        f = [this] (wxWindow *p) { return (wxWindow*)(new ServicesPanel(p, this));};
        mPreferencesEditor->AddPage(new xLightsPreferencesPage("Services", wxArtProvider::GetBitmap("xlART_SETTINGS", wxART_BUTTON, wxSize(64, 64)), f));
#endif
    }

    mPreferencesEditor->Show(this);

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
