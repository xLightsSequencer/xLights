/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "XLightsPluginHost.h"

#include <wx/aui/aui.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/rearrangectrl.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

#include "xLightsMain.h"
#include "sequencer/MainSequencer.h"
#include "sequencer/RenderCommandEvent.h"
#include "effects/EffectPresetManager.h"
#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"

namespace {

// Every IPlugin* implementation below wraps a real wx object constructed by
// the host (XLightsPluginHost::CreateDockablePanel / PluginPanelImpl), never
// by the plugin - see xLightsPlugin.h's header comment for why that matters.

class PluginStaticTextImpl : public IPluginStaticText {
public:
    explicit PluginStaticTextImpl(wxStaticText* control) : control_(control) {}
    void SetText(const std::string& text) override { control_->SetLabel(wxString(text)); }

private:
    wxStaticText* control_;
};

class PluginListBoxImpl : public IPluginListBox {
public:
    explicit PluginListBoxImpl(wxListBox* control) : control_(control) {}

    void Clear() override { control_->Clear(); }
    void Append(const std::string& item) override { control_->Append(wxString(item)); }
    [[nodiscard]] int GetSelection() const override { return control_->GetSelection(); }
    [[nodiscard]] std::string GetString(int index) const override { return control_->GetString(index).ToStdString(); }

    void OnDoubleClick(std::function<void(int)> callback) override {
        control_->Bind(wxEVT_LISTBOX_DCLICK, [callback](wxCommandEvent& event) {
            callback(event.GetInt());
        });
    }

private:
    wxListBox* control_;
};

class PluginButtonImpl : public IPluginButton {
public:
    explicit PluginButtonImpl(wxButton* control) : control_(control) {}

    void OnClick(std::function<void()> callback) override {
        control_->Bind(wxEVT_BUTTON, [callback](wxCommandEvent&) { callback(); });
    }

private:
    wxButton* control_;
};

class PluginPanelImpl : public IPluginPanel {
public:
    PluginPanelImpl(wxPanel* panel, wxBoxSizer* sizer, XLightsPluginHost* host)
        : panel_(panel), sizer_(sizer), host_(host) {}

    IPluginStaticText* AddStaticText(const std::string& initialText) override {
        auto* control = new wxStaticText(panel_, wxID_ANY, wxString(initialText));
        sizer_->Add(control, 0, wxALL | wxEXPAND, 6);
        return static_cast<IPluginStaticText*>(host_->AdoptWidget(std::make_unique<PluginStaticTextImpl>(control)));
    }

    IPluginListBox* AddListBox() override {
        auto* control = new wxListBox(panel_, wxID_ANY);
        sizer_->Add(control, 1, wxALL | wxEXPAND, 6);
        return static_cast<IPluginListBox*>(host_->AdoptWidget(std::make_unique<PluginListBoxImpl>(control)));
    }

    IPluginButton* AddButton(const std::string& label) override {
        auto* control = new wxButton(panel_, wxID_ANY, wxString(label));
        sizer_->Add(control, 0, wxALL, 6);
        return static_cast<IPluginButton*>(host_->AdoptWidget(std::make_unique<PluginButtonImpl>(control)));
    }

    void OnShown(std::function<void()> callback) override {
        shownCallback_ = std::move(callback);
    }

    // Invoked by XLightsPluginHost's Tools-menu toggle handler right after
    // showing the pane - not part of IPluginPanel, plugins never call this.
    void FireShown() {
        if (shownCallback_) shownCallback_();
    }

private:
    wxPanel* panel_;
    wxBoxSizer* sizer_;
    XLightsPluginHost* host_;
    std::function<void()> shownCallback_;
};

} // namespace

XLightsPluginHost::XLightsPluginHost(xLightsFrame* frame) : frame_(frame) {}

XLightsPluginHost::~XLightsPluginHost() = default;

std::vector<std::string> XLightsPluginHost::GetPresetPaths() const {
    return frame_->GetEffectPresetManager().GetAllPresetPaths();
}

bool XLightsPluginHost::ApplyPreset(const std::string& presetPath) {
    return frame_->ApplyEffectsPreset(presetPath) != nullptr;
}

const Effect* XLightsPluginHost::GetSelectedEffect() const {
    MainSequencer* seq = frame_->GetMainSequencer();
    return seq != nullptr ? seq->GetSelectedEffect() : nullptr;
}

void XLightsPluginHost::OnSelectionChanged(SelectionChangedFn callback) {
    selectionCallbacks_.push_back(std::move(callback));
    EnsureSelectionEventsBound();
}

void XLightsPluginHost::EnsureSelectionEventsBound() {
    if (boundSelectionEvents_) return;
    boundSelectionEvents_ = true;
    frame_->Bind(EVT_SELECTED_EFFECT_CHANGED, &XLightsPluginHost::OnEffectSelected, this);
    frame_->Bind(EVT_UNSELECTED_EFFECT, &XLightsPluginHost::OnEffectUnselected, this);
}

void XLightsPluginHost::OnEffectSelected(SelectedEffectChangedEvent& event) {
    for (auto& cb : selectionCallbacks_) {
        cb(event.effect);
    }
    event.Skip();
}

void XLightsPluginHost::OnEffectUnselected(wxCommandEvent& event) {
    for (auto& cb : selectionCallbacks_) {
        cb(nullptr);
    }
    event.Skip();
}

IPluginWidget* XLightsPluginHost::AdoptWidget(std::unique_ptr<IPluginWidget> widget) {
    IPluginWidget* raw = widget.get();
    widgets_.push_back(std::move(widget));
    return raw;
}

wxMenu* XLightsPluginHost::EnsurePluginsSubmenu() {
    if (pluginsSubmenu_ == nullptr) {
        pluginsSubmenu_ = new wxMenu();
        frame_->Menu1->AppendSubMenu(pluginsSubmenu_, "Plugins");
    }
    return pluginsSubmenu_;
}

IPluginPanel* XLightsPluginHost::CreateDockablePanel(const std::string& paneName, const std::string& caption,
                                                      const std::string& toolsMenuLabel) {
    wxString wxPaneName(paneName);

    wxPanel* panel = new wxPanel(frame_->PanelSequencer);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(sizer);

    frame_->m_mgr->AddPane(panel, wxAuiPaneInfo()
        .Name(wxPaneName)
        .Caption(wxString(caption))
        .Float()
        .BestSize(300, 400)
        .Hide());

    wxMenuItem* item = EnsurePluginsSubmenu()->AppendCheckItem(wxID_ANY, wxString(toolsMenuLabel));
    frame_->RegisterPluginToolsMenuItem(wxPaneName, item);

    auto panelImpl = std::make_unique<PluginPanelImpl>(panel, sizer, this);
    PluginPanelImpl* panelImplRaw = panelImpl.get();

    frame_->Bind(wxEVT_COMMAND_MENU_SELECTED, [this, wxPaneName, panelImplRaw](wxCommandEvent&) {
        wxAuiPaneInfo& pane = frame_->m_mgr->GetPane(wxPaneName);
        bool nowShown = !pane.IsShown();
        if (nowShown) {
            pane.Show();
        } else {
            pane.Hide();
        }
        frame_->m_mgr->Update();
        frame_->UpdateViewMenu();
        if (nowShown) {
            panelImplRaw->FireShown();
        }
    }, item->GetId());

    return static_cast<IPluginPanel*>(AdoptWidget(std::move(panelImpl)));
}

void XLightsPluginHost::AddPluginsMenuAction(const std::string& label, std::function<void()> callback) {
    wxMenuItem* item = EnsurePluginsSubmenu()->Append(wxID_ANY, wxString(label));
    frame_->Bind(wxEVT_COMMAND_MENU_SELECTED, [callback](wxCommandEvent&) {
        callback();
    }, item->GetId());
}

std::vector<std::string> XLightsPluginHost::GetAllEffectNames() const {
    EffectManager& mgr = frame_->GetEffectManager();
    std::vector<std::string> names;
    names.reserve(mgr.size());
    for (size_t i = 0; i < mgr.size(); ++i) {
        RenderableEffect* eff = mgr[i];
        if (eff != nullptr) {
            names.push_back(eff->Name());
        }
    }
    return names;
}

void XLightsPluginHost::SetEffectsToolbarEffects(const std::vector<std::string>& orderedVisibleNames) {
    frame_->RebuildEffectsToolbar(orderedVisibleNames);
}

bool XLightsPluginHost::ShowReorderDialog(const std::string& title, const std::string& message,
                                          std::vector<std::string>& items, std::vector<bool>& checked) {
    wxArrayString wxItems;
    wxArrayInt order;
    for (size_t i = 0; i < items.size(); ++i) {
        wxItems.Add(wxString(items[i]));
        int idx = static_cast<int>(i);
        order.Add(checked[i] ? idx : ~idx);
    }

    wxRearrangeDialog dlg(frame_, wxString(message), wxString(title), order, wxItems);
    if (dlg.ShowModal() != wxID_OK) {
        return false;
    }

    wxArrayInt newOrder = dlg.GetOrder();
    std::vector<std::string> newItems;
    std::vector<bool> newChecked;
    newItems.reserve(newOrder.size());
    newChecked.reserve(newOrder.size());
    for (size_t i = 0; i < newOrder.size(); ++i) {
        int idx = newOrder[i];
        bool isChecked = idx >= 0;
        size_t realIdx = static_cast<size_t>(isChecked ? idx : ~idx);
        newItems.push_back(items[realIdx]);
        newChecked.push_back(isChecked);
    }
    items = std::move(newItems);
    checked = std::move(newChecked);
    return true;
}

std::string XLightsPluginHost::GetUserDataDir() const {
    return wxStandardPaths::Get().GetUserDataDir().ToStdString();
}
