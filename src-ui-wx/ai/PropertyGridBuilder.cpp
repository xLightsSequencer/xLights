#include "PropertyGridBuilder.h"

#include <wx/panel.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/sizer.h>

#include <functional>
#include <string>

namespace PropertyGridBuilder {

void Append(wxPropertyGrid* page, const std::vector<ServiceProperty>& props) {
    for (const auto& p : props) {
        switch (p.kind) {
            case ServiceProperty::Kind::Category: {
                page->Append(new wxPropertyCategory(p.label.empty() ? p.category : p.label));
                break;
            }
            case ServiceProperty::Kind::Bool: {
                bool v = std::holds_alternative<bool>(p.value) ? std::get<bool>(p.value) : false;
                auto* prop = page->Append(new wxBoolProperty(p.label, p.id, v));
                prop->SetEditor("CheckBox");
                if (!p.help.empty()) prop->SetHelpString(p.help);
                break;
            }
            case ServiceProperty::Kind::Int: {
                int v = std::holds_alternative<int>(p.value) ? std::get<int>(p.value) : 0;
                auto* prop = page->Append(new wxIntProperty(p.label, p.id, v));
                if (!p.help.empty()) prop->SetHelpString(p.help);
                break;
            }
            case ServiceProperty::Kind::String: {
                const std::string& v = std::holds_alternative<std::string>(p.value)
                                       ? std::get<std::string>(p.value)
                                       : std::string();
                auto* prop = page->Append(new wxStringProperty(p.label, p.id, v));
                if (!p.help.empty()) prop->SetHelpString(p.help);
                break;
            }
            case ServiceProperty::Kind::Secret: {
                const std::string& v = std::holds_alternative<std::string>(p.value)
                                       ? std::get<std::string>(p.value)
                                       : std::string();
                auto* prop = page->Append(new wxStringProperty(p.label, p.id, v));
                prop->SetAttribute(wxPG_STRING_PASSWORD, true);
                if (!p.help.empty()) prop->SetHelpString(p.help);
                break;
            }
            case ServiceProperty::Kind::Choice: {
                wxPGChoices choices;
                for (const auto& c : p.choices) choices.Add(c);
                const std::string& v = std::holds_alternative<std::string>(p.value)
                                       ? std::get<std::string>(p.value)
                                       : std::string();
                int sel = 0;
                for (size_t i = 0; i < p.choices.size(); ++i) {
                    if (p.choices[i] == v) { sel = static_cast<int>(i); break; }
                }
                auto* prop = page->Append(new wxEnumProperty(p.label, p.id, choices, sel));
                if (!p.help.empty()) prop->SetHelpString(p.help);
                break;
            }
        }
    }
}

wxPanel* MakePanel(wxWindow* parent,
                   const std::vector<ServiceProperty>& props,
                   std::function<void(const wxPropertyGridEvent&)> onChanged) {
    auto* panel = new wxPanel(parent);
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    auto* grid = new wxPropertyGrid(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                    wxPG_SPLITTER_AUTO_CENTER | wxPG_DEFAULT_STYLE);
    grid->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX, true);
    Append(grid, props);
    sizer->Add(grid, 1, wxEXPAND | wxALL, 0);
    panel->SetSizer(sizer);
    if (onChanged) {
        grid->Bind(wxEVT_PG_CHANGED,
                   [cb = std::move(onChanged)](wxPropertyGridEvent& e) { cb(e); });
    }
    return panel;
}

void Dispatch(const wxPropertyGridEvent& event,
              const std::function<void(const std::string&, bool)>& setBool,
              const std::function<void(const std::string&, int)>& setInt,
              const std::function<void(const std::string&, const std::string&)>& setString) {
    auto* prop = event.GetProperty();
    if (!prop) return;
    std::string id = prop->GetName().ToStdString();
    const wxVariant value = event.GetPropertyValue();
    const wxString type = value.GetType();
    if (type == "bool") {
        if (setBool) setBool(id, value.GetBool());
    } else if (type == "long") {
        // wxEnumProperty selection also arrives as a long. If the editor has a
        // string value (enum choice), prefer the choice label.
        if (prop->IsKindOf(CLASSINFO(wxEnumProperty))) {
            std::string s = prop->GetValueAsString().ToStdString();
            if (setString) setString(id, s);
        } else if (setInt) {
            setInt(id, static_cast<int>(value.GetLong()));
        }
    } else if (type == "string") {
        if (setString) setString(id, value.GetString().ToStdString());
    }
}

} // namespace PropertyGridBuilder
