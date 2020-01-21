
#include <wx/preferences.h>
#include <wx/artprov.h>

#include "../xLightsMain.h"

#include "ViewSettingsPanel.h"
#include "EffectsGridSettingsPanel.h"
#include "SequenceFileSettingsPanel.h"
#include "BackupSettingsPanel.h"
#include "OutputSettingsPanel.h"
#include "RandomEffectsSettingsPanel.h"
#include "ColorManagerSettingsPanel.h"
#include "OtherSettingsPanel.h"

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
        wxWindow *w = m_createFunction(parent);
#ifdef __WXOSX__
        //need to set a minimum width or the icons get moved into a flyout menu
        //which is more confusing
        w->SetMinSize(wxSize(500, -1));
#endif
        return w;
    }

private:
    wxBitmap m_icon;
    wxString m_name;
    std::function<wxWindow*(wxWindow*)> m_createFunction;
};

void xLightsFrame::OnMenuItemPreferencesSelected(wxCommandEvent& event)
{
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
        f = [this] (wxWindow *p) { return (wxWindow*)(new RandomEffectsSettingsPanel(p, this));};
        mPreferencesEditor->AddPage(new xLightsPreferencesPage("Random Effects", wxArtProvider::GetBitmap("xlART_DICE_ICON", wxART_BUTTON, wxSize(64, 64)), f));
        f = [this] (wxWindow *p) { return (wxWindow*)(new ColorManagerSettingsPanel(p, this));};
        mPreferencesEditor->AddPage(new xLightsPreferencesPage("Colors", wxArtProvider::GetBitmap("xlART_RENDER_ALL", wxART_BUTTON, wxSize(64, 64)), f));
        f = [this] (wxWindow *p) { return (wxWindow*)(new OtherSettingsPanel(p, this));};
        mPreferencesEditor->AddPage(new xLightsPreferencesPage("Other", settingIcon, f));
    }

    mPreferencesEditor->Show(this);
}
