
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ControllerPropertyAdapter.h"
#include "OutputPropertyAdapters.h"

#include <wx/settings.h>
#include <wx/msgdlg.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "outputs/Controller.h"
#include "outputs/OutputManager.h"
#include "models/OutputModelManager.h"
#include "controllers/ControllerCaps.h"
#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"
#include "models/Model.h"

static wxArrayString ACTIVETYPENAMES;

ControllerPropertyAdapter::ControllerPropertyAdapter(Controller& controller)
    : _controller(controller) {
}

int ControllerPropertyAdapter::EncodeChoices(const wxPGChoices& choices, const std::string& choice) {
    wxString c(choice);
    c.MakeLower();
    for (size_t i = 0; i < choices.GetCount(); i++) {
        if (choices[i].GetText().Lower() == c) return i;
    }
    return -1;
}

std::string ControllerPropertyAdapter::DecodeChoices(const wxPGChoices& choices, int choice) {
    if (choice < 0 || choice >= (int)choices.GetCount()) {
        return "";
    }
    return choices[choice].GetText().ToStdString();
}

void ControllerPropertyAdapter::AddModels(wxPGProperty* p, wxPGProperty* vp) {
    int mIdx = -1;
    wxPGChoices models;

    if (!_controller.GetVendor().empty()) {
        for (const auto& it : ControllerCaps::GetModels(_controller.GetType(), _controller.GetVendor())) {
            models.Add(it);
            if (it == _controller.GetModel()) {
                mIdx = models.GetCount() - 1;
            }
        }
    }
    p->SetChoices(models);
    if (models.GetCount() > 0) {
        p->Hide(false);
        if (mIdx >= 0) {
            p->SetChoiceSelection(mIdx);
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        } else {
            p->SetBackgroundColour(*wxRED);
            vp->Hide(true);
        }
    } else {
        p->Hide(true);
    }
}

void ControllerPropertyAdapter::AddVariants(wxPGProperty* p) {
    int variantIdx = -1;
    wxPGChoices variants;

    if (!_controller.GetVendor().empty() && !_controller.GetModel().empty()) {
        p->Hide(false);
        std::list<std::string> vars = ControllerCaps::GetVariants(_controller.GetType(), _controller.GetVendor(), _controller.GetModel());
        for (const auto& it : vars) {
            variants.Add(it);
            if (it == _controller.GetVariant()) {
                variantIdx = variants.GetCount() - 1;
            }
        }
        if (variants.GetCount() > 1) {
            if (variantIdx == -1) {
                variantIdx = 0;
                _controller.SetVariant(vars.front());
            }
        } else {
            _controller.SetVariant("");
        }
        p->SetChoices(variants);
        if (variants.GetCount() > 1) {
            p->Hide(false);
            if (variantIdx >= 0) {
                p->SetChoiceSelection(variantIdx);
                p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            } else {
                p->SetBackgroundColour(*wxRED);
            }
        } else {
            p->Hide(true);
        }
    } else {
        p->Hide(true);
    }
}

void ControllerPropertyAdapter::UpdateProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties, OutputModelManager* outputModelManager) {

    wxPGProperty* p = propertyGrid->GetProperty("ControllerName");
    if (p) p->SetValue(_controller.GetName());

    p = propertyGrid->GetProperty("ControllerDescription");
    if (p) p->SetValue(_controller.GetDescription());

    p = propertyGrid->GetProperty("ControllerId");
    if (p) {
        p->Hide(!_controller.IsNeedsId());
        p->SetValue(_controller.GetId());
    }

    p = propertyGrid->GetProperty("AutoLayout");
    if (p) {
        p->SetValue(_controller.IsAutoLayout());
        p->Hide(!_controller.SupportsAutoLayout());
    }

    p = propertyGrid->GetProperty("AutoUpload");
    if (p) {
        p->SetValue(_controller.IsAutoUpload());
        p->Hide(!_controller.SupportsAutoUpload());
    }

    p = propertyGrid->GetProperty("AutoSize");
    if (p) {
        p->SetValue(_controller.IsAutoSize());
        p->Hide(!_controller.SupportsAutoSize());
    }

    p = propertyGrid->GetProperty("SuppressDuplicates");
    if (p) {
        p->SetValue(_controller.IsSuppressDuplicateFrames());
        p->Hide(!_controller.SupportsSuppressDuplicateFrames());
    }
    p = propertyGrid->GetProperty("Monitor");
    if (p) {
        p->SetValue(_controller.IsMonitoring());
    }

    p = propertyGrid->GetProperty("FullxLightsControl");
    if (p) {
        p->SetValue(_controller.IsFullxLightsControl());
        p->Hide(!_controller.SupportsFullxLightsControl());
    }
    if (_controller.SupportsFullxLightsControl() && _controller.IsFullxLightsControl()) {
        p = propertyGrid->GetProperty("DefaultBrightnessUnderFullxLightsControl");
        if (p) {
            p->SetValue(_controller.GetDefaultBrightnessUnderFullControl());
            p->Hide(!_controller.SupportsDefaultBrightness());
        }

        p = propertyGrid->GetProperty("DefaultGammaUnderFullxLightsControl");
        if (p) {
            p->SetValue(_controller.GetDefaultGammaUnderFullControl());
            p->Hide(!_controller.SupportsDefaultGamma());
        }
    } else {
        p = propertyGrid->GetProperty("DefaultBrightnessUnderFullxLightsControl");
        if (p) p->Hide(true);

        p = propertyGrid->GetProperty("DefaultGammaUnderFullxLightsControl");
        if (p) p->Hide(true);
    }
    p = propertyGrid->GetProperty("Active");
    if (p) {
        p->SetChoiceSelection((int)_controller.GetActive());
    }
}

void ControllerPropertyAdapter::AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {

    wxPGProperty* p = propertyGrid->Append(new wxStringProperty("Name", "ControllerName", _controller.GetName()));
    p->SetHelpString("This must be unique.");

    propertyGrid->Append(new wxStringProperty("Description", "ControllerDescription", _controller.GetDescription()));

    int v = 0;
    wxPGChoices vendors;
    for (const auto& it : ControllerCaps::GetVendors(_controller.GetType())) {
        vendors.Add(it);
        if (it == _controller.GetVendor()) {
            v = vendors.GetCount() - 1;
        }
    }
    if (vendors.GetCount() > 0) {
        propertyGrid->Append(new wxEnumProperty("Vendor", "Vendor", vendors, v));
        wxPGProperty* mp = propertyGrid->Append(new wxEnumProperty("Model/Category", "Model"));
        wxPGProperty* vp = propertyGrid->Append(new wxEnumProperty("Variant", "Variant"));
        AddVariants(vp);
        AddModels(mp, vp);
    }

    p = propertyGrid->Append(new wxUIntProperty("Id", "ControllerId", _controller.GetId()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 65335);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("For controllers that don't support universes the Id can be used with some start channel addressing modes.");

    p = propertyGrid->Append(new wxBoolProperty("Auto Layout Models", "AutoLayout", _controller.IsAutoLayout()));
    p->SetEditor("CheckBox");
    p->SetHelpString("Auto layout models causes xLights to move models around in the controllers channel range to optimise them. It also needs to be set to allow you to move modes in the visualiser between controllers.");

    p = propertyGrid->Append(new wxBoolProperty("Auto Upload Configuration", "AutoUpload", _controller.IsAutoUpload()));
    p->SetEditor("CheckBox");
    p->SetHelpString("This option will send your controller configuration to the controller when you turn on output to lights. This is known to cause delays in turning on output to lights when controllers are not reachable.");

    p = propertyGrid->Append(new wxBoolProperty("Auto Size", "AutoSize", _controller.IsAutoSize()));
    p->SetEditor("CheckBox");
    p->SetHelpString("This option will cause xLights to dynamically resize the number of channels on the controller to ensure there are exactly enough for the models on the controller.");

    p = propertyGrid->Append(new wxBoolProperty("Full xLights Control", "FullxLightsControl", _controller.IsFullxLightsControl()));
    p->SetEditor("CheckBox");
    p->SetHelpString("This option will when uploading erase the configuration of all unused ports on the controller per the configuration in xLights.");

    p = propertyGrid->Append(new wxUIntProperty("Default Port Brightness", "DefaultBrightnessUnderFullxLightsControl", _controller.GetDefaultBrightnessUnderFullControl()));
    p->SetAttribute("Min", 5);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This option will set the brightness of all ports to this value unless specifically overriden by a model.");

    p = propertyGrid->Append(new wxFloatProperty("Default Port Gamma", "DefaultGammaUnderFullxLightsControl", _controller.GetDefaultGammaUnderFullControl()));
    p->SetAttribute("Min", 0.1F);
    p->SetAttribute("Max", 5.0F);
    p->SetEditor("SpinCtrlDouble");
    p->SetHelpString("This option will set the Gamma of all ports to this value unless specifically overriden by a model.");

    if (ACTIVETYPENAMES.IsEmpty()) {
        ACTIVETYPENAMES.push_back("Active");
        ACTIVETYPENAMES.push_back("Inactive");
        ACTIVETYPENAMES.push_back("xLights Only");
    }
    p = propertyGrid->Append(new wxEnumProperty("Active", "Active", ACTIVETYPENAMES, wxArrayInt(), (int)_controller.GetActive()));
    p->SetHelpString("When inactive no data is output and any upload to FPP or xSchedule will also not output to the lights. When xLights only it will output when played in xLights but again FPP and xSchedule will not output to the lights.");

    p = propertyGrid->Append(new wxBoolProperty("Monitor", "Monitor", _controller.IsMonitoring()));
    p->SetEditor("CheckBox");
    p->SetHelpString("When selected, the show player will monitor the connectivity to the controller via ping packets or other network traffic to make sure the controller is up and running.  Disabling can reduce network traffic as well as reduce load on the controller.");

    p = propertyGrid->Append(new wxBoolProperty("Suppress Duplicate Frames", "SuppressDuplicates", _controller.IsSuppressDuplicateFrames()));
    p->SetEditor("CheckBox");
    p->SetHelpString("When selected if a data packet is the same in a subsequent frame then xLights will not send the duplicate frame trusting that the controller will just reuse the previously sent data. This can reduce unnecessary network traffic.");

    ControllerCaps* caps = ControllerCaps::GetControllerConfig(&_controller);
    if (caps) {
        for (const auto& prop : caps->GetExtraPropertyDefs()) {
            if (prop.type == "Enum") {
                wxPGChoices pgcValues;
                for (const auto& v : prop.values) {
                    pgcValues.Add(v);
                }
                int idx = pgcValues.Index(_controller.GetExtraProperty(prop.name, prop.defaultValue));
                propertyGrid->Append(new wxEnumProperty(prop.label, "Controller" + prop.name, pgcValues, idx));
            } else if (prop.type == "String") {
                propertyGrid->Append(new wxStringProperty(prop.label, "Controller" + prop.name, _controller.GetExtraProperty(prop.name, prop.defaultValue)));
            }
        }
    }
}

bool ControllerPropertyAdapter::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) {
    wxString const name = event.GetPropertyName();
    wxPropertyGrid* propertyGrid = (wxPropertyGrid*)event.GetEventObject();
    auto* om = _controller.GetOutputManager();

    if (name == "ControllerName") {
        auto cn = event.GetValue().GetString().Trim(true).Trim(false);
        if (om->GetController(cn) != nullptr || cn == "" || cn == NO_CONTROLLER) {
            DisplayError("Controller name '" + cn + "' blank or already used. Controller names must be unique and non blank.");
            outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "Controller::HandlePropertyEvent::ControllerName");
            return false;
        } else {
            _controller.SetName(cn);
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CONFIG_CHANGE, "Controller::HandlePropertyEvent::ControllerName");
            outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Controller::HandlePropertyEvent::ControllerName");
            return true;
        }
    } else if (name == "ControllerDescription") {
        _controller.SetDescription(event.GetValue().GetString().Trim(true).Trim(false));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "Controller::HandlePropertyEvent::Controllerdescription");
        return true;
    } else if (name == "ControllerId") {
        _controller.SetId(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "Controller::HandlePropertyEvent::ControllerId");
        return true;
    } else if (name == "Active") {
        if (ACTIVETYPENAMES.IsEmpty()) {
            ACTIVETYPENAMES.push_back("Active");
            ACTIVETYPENAMES.push_back("Inactive");
            ACTIVETYPENAMES.push_back("xLights Only");
        }
        _controller.SetActive(ACTIVETYPENAMES[event.GetValue().GetLong()]);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "Controller::HandlePropertyEvent::Active");
        return true;
    } else if (name == "AutoLayout") {
        _controller.SetAutoLayout(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "Controller::HandlePropertyEvent::AutoLayout");
        return true;
    } else if (name == "AutoUpload") {
        _controller.SetAutoUpload(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "Controller::HandlePropertyEvent::AutoUpload");
        return true;
    } else if (name == "SuppressDuplicates") {
        _controller.SetSuppressDuplicateFrames(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::SuppressDuplicates");
        return true;
    } else if (name == "Monitor") {
        _controller.SetMonitoring(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::Monitor");
        return true;
    } else if (name == "AutoSize") {
        if (event.GetValue().GetBool() && _controller.GetOutputCount() > 0 && _controller.GetProtocol() == OUTPUT_E131) {
            int universeSize = _controller.GetFirstOutput()->GetChannels();
            if (universeSize != 170 && universeSize != 510 && universeSize != 512) {
                wxMessageBox(
                    wxString::Format("The current Universe Size is %d.\n\nFor Auto Size to work correctly, you may need to update the Universe Size to a common value such as 510 or 512.", universeSize),
                    "Universe Size Warning",
                    wxOK | wxICON_WARNING);
            }
        }
        _controller.SetAutoSize(event.GetValue().GetBool(), outputModelManager);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CONFIG_CHANGE, "Controller::HandlePropertyEvent::AutoSize");
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Controller::HandlePropertyEvent::AutoSize");
        return true;
    } else if (name == "FullxLightsControl") {
        _controller.SetFullxLightsControl(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "Controller::HandlePropertyEvent::FullxLightsControl");
        return true;
    } else if (name == "DefaultBrightnessUnderFullxLightsControl") {
        _controller.SetDefaultBrightnessUnderFullControl(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::DefaultBrightnessUnderFullxLightsControl");
        return true;
    } else if (name == "DefaultGammaUnderFullxLightsControl") {
        _controller.SetDefaultGammaUnderFullControl(event.GetValue().GetDouble());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::DefaultGammaUnderFullxLightsControl");
        return true;
    } else if (name == "Vendor") {
        int idx = event.GetValue().GetLong();
        auto const vendors = ControllerCaps::GetVendors(_controller.GetType());
        auto it = begin(vendors);
        std::advance(it, idx);
        if (event.GetValue().GetLong() >= 0 && it != end(vendors)) {
            _controller.SetVendor(*it);

            auto models = ControllerCaps::GetModels(_controller.GetType(), *it);
            if (models.size() == 2) {
                _controller.SetModel(models.back());
                auto variants = ControllerCaps::GetVariants(_controller.GetType(), *it, models.front());
                if (variants.size() == 2) {
                    _controller.SetVariant(variants.back());
                } else {
                    _controller.SetVariant("");
                }
            } else {
                _controller.SetModel("");
                _controller.SetVariant("");
            }
        } else {
            _controller.SetVendor("");
            _controller.SetModel("");
            _controller.SetVariant("");
        }

        wxPGProperty* mp = propertyGrid->GetProperty("Model");
        wxPGProperty* vp = propertyGrid->GetProperty("Variant");
        AddVariants(vp);
        AddModels(mp, vp);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "Controller::HandlePropertyEvent::Vendor");
        return true;
    } else if (name == "Model") {
        auto const models = ControllerCaps::GetModels(_controller.GetType(), _controller.GetVendor());
        auto it = begin(models);
        std::advance(it, event.GetValue().GetLong());
        _controller.SetModel(*it);

        std::list<std::string> variants = ControllerCaps::GetVariants(_controller.GetType(), _controller.GetVendor(), *it);
        _controller.SetVariant(variants.front());

        wxPGProperty* mp = propertyGrid->GetProperty("Model");
        wxPGProperty* vp = propertyGrid->GetProperty("Variant");
        AddVariants(vp);
        AddModels(mp, vp);

        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "Controller::HandlePropertyEvent::Model");
        return true;
    } else if (name == "Variant") {
        auto const versions = ControllerCaps::GetVariants(_controller.GetType(), _controller.GetVendor(), _controller.GetModel());
        auto it = begin(versions);
        std::advance(it, event.GetValue().GetLong());
        _controller.SetVariant(*it);

        wxPGProperty* mp = propertyGrid->GetProperty("Model");
        wxPGProperty* vp = propertyGrid->GetProperty("Variant");
        AddVariants(vp);
        AddModels(mp, vp);

        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "Controller::HandlePropertyEvent::Variant");
        return true;
    } else {
        ControllerCaps* caps = ControllerCaps::GetControllerConfig(&_controller);
        if (caps) {
            for (const auto& prop : caps->GetExtraPropertyDefs()) {
                if (event.GetPropertyName() == "Controller" + prop.name) {
                    if (prop.type == "String") {
                        _controller.SetExtraProperty(prop.name, event.GetPropertyValue().GetString().ToStdString());
                    } else if (prop.type == "Enum") {
                        int idx = event.GetPropertyValue().GetLong();
                        if (idx < (int)prop.values.size()) {
                            _controller.SetExtraProperty(prop.name, prop.values[idx]);
                        }
                    }
                    outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::" + name);
                    return true;
                }
            }
        }
    }
    return false;
}

void ControllerPropertyAdapter::ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const {

    auto p = propGrid->GetPropertyByName("ControllerId");
    auto const& name = _controller.GetName();

    if (p != nullptr) {
        // Id should be unique
        int id = _controller.GetId();
        for (const auto& it : om->GetControllers()) {
            if (it->GetName() != name && it->GetId() == id && it->GetId() != -1) {
                p->SetBackgroundColour(*wxRED);
                break;
            } else {
                p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            }
        }
    }

    p = propGrid->GetPropertyByName("ControllerName");
    if (p != nullptr) {
        wxString value = p->GetValueAsString();
        bool isValid = true;
        for (wxChar c : value) {
            if (c == ':') {
                isValid = false;
                break;
            }
        }
        if (!isValid || !om->IsControllerNameUnique(name)) {
            p->SetBackgroundColour(*wxRED);
        } else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    p = propGrid->GetPropertyByName("KeepChannelNumbers");
    if (p != nullptr) {
        ControllerCaps* c = nullptr;
        if (!_controller.GetModel().empty()) {
            c = ControllerCaps::GetControllerConfig(_controller.GetVendor(), _controller.GetModel(), _controller.GetVariant());
        } else {
            c = ControllerCaps::GetControllerConfigByVendor(_controller.GetVendor());
        }
        if (c && c->DDPStartsAtOne() && p->GetValue() == true) {
            p->SetBackgroundColour(*wxRED);
        } else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }
}

void ControllerPropertyAdapter::HandleExpanded(wxPropertyGridEvent& event, bool expanded) {
    if (_controller.GetOutputCount() > 0) {
        auto adapter = OutputPropertyAdapter::Create(*_controller.GetFirstOutput());
        adapter->HandleExpanded(event, expanded);
    }
}
