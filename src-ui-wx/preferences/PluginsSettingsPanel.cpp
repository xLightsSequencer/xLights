/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PluginsSettingsPanel.h"

#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/filedlg.h>
#include <wx/filefn.h>
#include <wx/msgdlg.h>
#include <wx/button.h>

#include "xLightsMain.h"
#include "effects/EffectManager.h"
#include "plugins/XLightsPluginManager.h"

namespace {
    // Mirrors the folder xLightsFrame's constructor scans via
    // EffectManager::loadEffectPlugins() - kept in one place there, this is
    // just for display so users know where to drop plugin DLLs.
    wxString GetEffectPluginDir() {
        wxFileName exePath(wxStandardPaths::Get().GetExecutablePath());
        return exePath.GetPath() + wxFILE_SEP_PATH + "effect_plugins";
    }

    // Mirrors the folder xLightsFrame's constructor scans (after
    // CreateSequencer()) via XLightsPluginManager::loadPlugins().
    wxString GetXLightsPluginDir() {
        wxFileName exePath(wxStandardPaths::Get().GetExecutablePath());
        return exePath.GetPath() + wxFILE_SEP_PATH + "xlights_plugins";
    }

    // Matches the platform's plugin extension and each manager's filename-
    // substring filter (see EffectManager.cpp / XLightsPluginManager.h configs).
#ifdef _WIN32
    constexpr const char* kPluginExt = "dll";
#elif defined(__APPLE__)
    constexpr const char* kPluginExt = "dylib";
#else
    constexpr const char* kPluginExt = "so";
#endif
    constexpr const char* kPluginFileNameFilter = "EffectPlugin";
    constexpr const char* kXLightsPluginFileNameFilter = "XLightsPlugin";
}

PluginsSettingsPanel::PluginsSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id, const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, id, pos, size, wxTAB_TRAVERSAL), frame(f)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    wxNotebook* notebook = new wxNotebook(this, wxID_ANY);

    // --- Effect Plugins ---
    wxPanel* effectPage = new wxPanel(notebook);
    wxBoxSizer* effectSizer = new wxBoxSizer(wxVERTICAL);
    effectSizer->Add(new wxStaticText(effectPage, wxID_ANY,
        wxString::Format("Loaded at startup from: %s", GetEffectPluginDir())),
        0, wxALL, 6);

    effectPluginsList = new wxListCtrl(effectPage, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                        wxLC_REPORT | wxLC_SINGLE_SEL);
    effectPluginsList->AppendColumn("Name", wxLIST_FORMAT_LEFT, 160);
    effectPluginsList->AppendColumn("File", wxLIST_FORMAT_LEFT, 320);
    effectPluginsList->AppendColumn("Status", wxLIST_FORMAT_LEFT, 220);
    effectSizer->Add(effectPluginsList, 1, wxEXPAND | wxALL, 6);

    wxBoxSizer* effectButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* addButton = new wxButton(effectPage, wxID_ANY, "Add Plugin...");
    addButton->Bind(wxEVT_BUTTON, &PluginsSettingsPanel::OnAddEffectPluginClick, this);
    effectButtonSizer->Add(addButton, 0, wxALL, 6);
    wxButton* removeButton = new wxButton(effectPage, wxID_ANY, "Remove Selected...");
    removeButton->Bind(wxEVT_BUTTON, &PluginsSettingsPanel::OnRemoveEffectPluginClick, this);
    effectButtonSizer->Add(removeButton, 0, wxALL, 6);
    effectSizer->Add(effectButtonSizer, 0, wxALL, 0);

    effectPage->SetSizer(effectSizer);
    notebook->AddPage(effectPage, "Effect Plugins", true);

    PopulateEffectPluginsList();

    // --- xLights Plugins (dockable panels / Tools-menu UI extensions) ---
    wxPanel* programPage = new wxPanel(notebook);
    wxBoxSizer* programSizer = new wxBoxSizer(wxVERTICAL);
    programSizer->Add(new wxStaticText(programPage, wxID_ANY,
        wxString::Format("Loaded at startup from: %s", GetXLightsPluginDir())),
        0, wxALL, 6);

    xlightsPluginsList = new wxListCtrl(programPage, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                         wxLC_REPORT | wxLC_SINGLE_SEL);
    xlightsPluginsList->AppendColumn("Name", wxLIST_FORMAT_LEFT, 160);
    xlightsPluginsList->AppendColumn("File", wxLIST_FORMAT_LEFT, 320);
    xlightsPluginsList->AppendColumn("Status", wxLIST_FORMAT_LEFT, 220);
    programSizer->Add(xlightsPluginsList, 1, wxEXPAND | wxALL, 6);

    wxBoxSizer* programButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* addXLightsPluginButton = new wxButton(programPage, wxID_ANY, "Add Plugin...");
    addXLightsPluginButton->Bind(wxEVT_BUTTON, &PluginsSettingsPanel::OnAddXLightsPluginClick, this);
    programButtonSizer->Add(addXLightsPluginButton, 0, wxALL, 6);
    wxButton* removeXLightsPluginButton = new wxButton(programPage, wxID_ANY, "Remove Selected...");
    removeXLightsPluginButton->Bind(wxEVT_BUTTON, &PluginsSettingsPanel::OnRemoveXLightsPluginClick, this);
    programButtonSizer->Add(removeXLightsPluginButton, 0, wxALL, 6);
    programSizer->Add(programButtonSizer, 0, wxALL, 0);

    programPage->SetSizer(programSizer);
    notebook->AddPage(programPage, "xLights Plugins", false);

    PopulateXLightsPluginsList();

    mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 4);
    SetSizer(mainSizer);
}

void PluginsSettingsPanel::PopulateEffectPluginsList() {
    effectPluginsList->DeleteAllItems();

    const auto emptyInfos = std::vector<EffectManager::PluginInfo>{};
    const auto& infos = frame != nullptr ? frame->GetEffectManager().GetPluginInfo() : emptyInfos;
    for (const auto& info : infos) {
        wxString name = info.loaded ? wxString(info.name) : wxString("(failed)");
        wxString status = info.loaded ? wxString("Loaded") : wxString::Format("Failed: %s", info.error);
        long idx = effectPluginsList->InsertItem(effectPluginsList->GetItemCount(), name);
        effectPluginsList->SetItem(idx, 1, info.path);
        effectPluginsList->SetItem(idx, 2, status);
    }

    for (const auto& pending : m_pendingAdds) {
        long idx = effectPluginsList->InsertItem(effectPluginsList->GetItemCount(), pending.first);
        effectPluginsList->SetItem(idx, 1, pending.second);
        effectPluginsList->SetItem(idx, 2, "Pending - restart required");
    }

    if (infos.empty() && m_pendingAdds.empty()) {
        long idx = effectPluginsList->InsertItem(0, "(none found)");
        effectPluginsList->SetItem(idx, 1, GetEffectPluginDir());
        effectPluginsList->SetItem(idx, 2, "-");
    }
}

void PluginsSettingsPanel::OnAddEffectPluginClick(wxCommandEvent& event) {
    wxString wildcard = wxString::Format("Effect plugin (*.%s)|*.%s", kPluginExt, kPluginExt);
    wxFileDialog dlg(this, "Choose an effect plugin file", "", "", wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() != wxID_OK) return;

    wxString srcPath = dlg.GetPath();
    wxFileName srcFn(srcPath);
    wxString fileName = srcFn.GetFullName();

    if (!fileName.Contains(kPluginFileNameFilter)) {
        int answer = wxMessageBox(
            wxString::Format(
                "'%s' does not contain \"%s\" in its filename, which is required for xLights to "
                "discover it at startup. It will be copied, but xLights will not load it until it is renamed.\n\n"
                "Continue anyway?", fileName, kPluginFileNameFilter),
            "Plugin filename", wxYES_NO | wxICON_WARNING, this);
        if (answer != wxYES) return;
    }

    wxString destDir = GetEffectPluginDir();
    if (!wxFileName::DirExists(destDir)) {
        wxFileName::Mkdir(destDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }
    wxString destPath = destDir + wxFILE_SEP_PATH + fileName;

    if (!wxCopyFile(srcPath, destPath)) {
        wxMessageBox(wxString::Format("Failed to copy '%s' to '%s'.", srcPath, destDir),
                     "Copy failed", wxICON_ERROR, this);
        return;
    }

    wxMessageBox(
        wxString::Format("Copied '%s' to the effect plugins folder.\n\n"
                          "Restart xLights for it to be loaded - effect plugins are only "
                          "scanned once, early at startup.", fileName),
        "Plugin added", wxICON_INFORMATION, this);

    m_pendingAdds.emplace_back(fileName, destPath);
    PopulateEffectPluginsList();
}

void PluginsSettingsPanel::PopulateXLightsPluginsList() {
    xlightsPluginsList->DeleteAllItems();

    const auto emptyInfos = std::vector<XLightsPluginManager::PluginInfo>{};
    const auto* mgr = frame != nullptr ? frame->GetXLightsPluginManager() : nullptr;
    const auto& infos = mgr != nullptr ? mgr->GetPluginInfo() : emptyInfos;
    for (const auto& info : infos) {
        wxString name = info.loaded ? wxString(info.name) : wxString("(failed)");
        wxString status = info.loaded ? wxString("Loaded") : wxString::Format("Failed: %s", info.error);
        long idx = xlightsPluginsList->InsertItem(xlightsPluginsList->GetItemCount(), name);
        xlightsPluginsList->SetItem(idx, 1, info.path);
        xlightsPluginsList->SetItem(idx, 2, status);
    }

    for (const auto& pending : m_pendingXLightsPluginAdds) {
        long idx = xlightsPluginsList->InsertItem(xlightsPluginsList->GetItemCount(), pending.first);
        xlightsPluginsList->SetItem(idx, 1, pending.second);
        xlightsPluginsList->SetItem(idx, 2, "Pending - restart required");
    }

    if (infos.empty() && m_pendingXLightsPluginAdds.empty()) {
        long idx = xlightsPluginsList->InsertItem(0, "(none found)");
        xlightsPluginsList->SetItem(idx, 1, GetXLightsPluginDir());
        xlightsPluginsList->SetItem(idx, 2, "-");
    }
}

void PluginsSettingsPanel::OnAddXLightsPluginClick(wxCommandEvent& event) {
    wxString wildcard = wxString::Format("xLights plugin (*.%s)|*.%s", kPluginExt, kPluginExt);
    wxFileDialog dlg(this, "Choose an xLights plugin file", "", "", wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() != wxID_OK) return;

    wxString srcPath = dlg.GetPath();
    wxFileName srcFn(srcPath);
    wxString fileName = srcFn.GetFullName();

    if (!fileName.Contains(kXLightsPluginFileNameFilter)) {
        int answer = wxMessageBox(
            wxString::Format(
                "'%s' does not contain \"%s\" in its filename, which is required for xLights to "
                "discover it at startup. It will be copied, but xLights will not load it until it is renamed.\n\n"
                "Continue anyway?", fileName, kXLightsPluginFileNameFilter),
            "Plugin filename", wxYES_NO | wxICON_WARNING, this);
        if (answer != wxYES) return;
    }

    wxString destDir = GetXLightsPluginDir();
    if (!wxFileName::DirExists(destDir)) {
        wxFileName::Mkdir(destDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }
    wxString destPath = destDir + wxFILE_SEP_PATH + fileName;

    if (!wxCopyFile(srcPath, destPath)) {
        wxMessageBox(wxString::Format("Failed to copy '%s' to '%s'.", srcPath, destDir),
                     "Copy failed", wxICON_ERROR, this);
        return;
    }

    wxMessageBox(
        wxString::Format("Copied '%s' to the xLights plugins folder.\n\n"
                          "Restart xLights for it to be loaded - xLights plugins are only "
                          "scanned once, at startup.", fileName),
        "Plugin added", wxICON_INFORMATION, this);

    m_pendingXLightsPluginAdds.emplace_back(fileName, destPath);
    PopulateXLightsPluginsList();
}

void PluginsSettingsPanel::RemoveSelectedPlugin(wxListCtrl* list, size_t knownCount,
                                                 std::vector<std::pair<wxString, wxString>>& pendingAdds,
                                                 const wxString& kindLabel) {
    long sel = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel < 0) {
        wxMessageBox("Select a plugin in the list first.", "No plugin selected", wxICON_INFORMATION, this);
        return;
    }

    size_t idx = static_cast<size_t>(sel);
    bool isPending = idx >= knownCount && idx < knownCount + pendingAdds.size();
    if (idx >= knownCount + pendingAdds.size()) {
        // The "(none found)" placeholder row, or otherwise out of range.
        return;
    }

    wxString name = list->GetItemText(sel, 0);
    wxString path = list->GetItemText(sel, 1);

    if (isPending) {
        // Never loaded this session (both managers only scan once, at
        // startup), so the file isn't locked - a real delete is safe and
        // what's expected for something just added by mistake.
        int answer = wxMessageBox(
            wxString::Format("Delete '%s'?\n\n%s\n\nThis cannot be undone.", name, path),
            wxString::Format("Remove %s", kindLabel), wxYES_NO | wxICON_WARNING, this);
        if (answer != wxYES) {
            return;
        }

        if (!wxRemoveFile(path)) {
            wxMessageBox(wxString::Format("Could not delete '%s'.", path), "Delete failed", wxICON_ERROR, this);
            return;
        }
        pendingAdds.erase(pendingAdds.begin() + (idx - knownCount));
        return;
    }

    // Already known to the manager - i.e. loaded (or attempted) at startup.
    // On Windows, a currently-loaded DLL can't be deleted (DeleteFile fails
    // while the module stays mapped) but CAN be renamed - the same trick
    // self-updaters use to replace a running executable - so disable it by
    // renaming rather than deleting outright. That also makes it reversible
    // (rename it back) if picked by mistake.
    wxString disabledPath = path + ".removed";
    int answer = wxMessageBox(
        wxString::Format("Disable '%s'?\n\n%s\n\nIt will be renamed to '%s' so xLights stops loading "
                          "it starting next restart. If it's currently loaded this session, that "
                          "won't change until you restart. The file is kept, not deleted - rename it "
                          "back to undo.",
                          name, path, wxFileName(disabledPath).GetFullName()),
        wxString::Format("Disable %s", kindLabel), wxYES_NO | wxICON_WARNING, this);
    if (answer != wxYES) {
        return;
    }

    if (!wxRenameFile(path, disabledPath)) {
        wxMessageBox(
            wxString::Format("Could not rename '%s'.\n\nClose xLights and remove the file manually if this persists.", path),
            "Disable failed", wxICON_ERROR, this);
    }
}

void PluginsSettingsPanel::OnRemoveEffectPluginClick(wxCommandEvent& event) {
    const auto emptyInfos = std::vector<EffectManager::PluginInfo>{};
    const auto& infos = frame != nullptr ? frame->GetEffectManager().GetPluginInfo() : emptyInfos;
    RemoveSelectedPlugin(effectPluginsList, infos.size(), m_pendingAdds, "Effect Plugin");
    PopulateEffectPluginsList();
}

void PluginsSettingsPanel::OnRemoveXLightsPluginClick(wxCommandEvent& event) {
    const auto emptyInfos = std::vector<XLightsPluginManager::PluginInfo>{};
    const auto* mgr = frame != nullptr ? frame->GetXLightsPluginManager() : nullptr;
    const auto& infos = mgr != nullptr ? mgr->GetPluginInfo() : emptyInfos;
    RemoveSelectedPlugin(xlightsPluginsList, infos.size(), m_pendingXLightsPluginAdds, "xLights Plugin");
    PopulateXLightsPluginsList();
}
