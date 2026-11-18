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

#include <wx/dialog.h>
#include <memory>
#include <vector>

class xlPropertyGrid;
class wxPropertyGridEvent;
class Model;
class ModelPropertyAdapter;
class xLightsFrame;

// Generic Bulk Edit popup: shows the SAME type-specific property section
// (ModelPropertyAdapter::AddTypeProperties / OnPropertyGridChange - the same
// code the main Layout panel's property grid uses) for whichever model type
// every passed-in model shares, scoped to one "primary" model. Whatever
// properties the user changes while the dialog is open get replayed onto the
// rest of the models on OK. This works for any DisplayAsType's type-specific
// properties (string count, node/light count, strands/string, start
// location, direction, ...) because it goes through that type's own adapter
// instead of a hand-picked property list - the adapter already knows which
// setter(s) each of its properties maps to.
class BulkEditModelPropertiesDialog : public wxDialog {
public:
    // models must all share the same DisplayAs() type and have at least 2
    // entries; models[0] is used as the primary (the one the grid actually
    // edits live while the dialog is open).
    BulkEditModelPropertiesDialog(wxWindow* parent, xLightsFrame* xlights, const std::vector<Model*>& models);

    // Shows the dialog modally. If accepted, replays every property changed
    // while it was open onto every other model in the selection and returns
    // true. Returns false if cancelled, or if nothing was actually changed.
    bool ShowAndApply();

private:
    void OnPropertyGridChanging(wxPropertyGridEvent& event);
    void OnPropertyGridChanged(wxPropertyGridEvent& event);

    xLightsFrame* _xlights;
    std::vector<Model*> _models;
    Model* _primary;
    xlPropertyGrid* _grid = nullptr;
    std::unique_ptr<ModelPropertyAdapter> _adapter;
    std::vector<wxString> _changedProperties;
};
