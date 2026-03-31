
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "OutputPropertyAdapters.h"
#include "ControllerPropertyAdapter.h"

#include <wx/settings.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "outputs/Output.h"
#include "outputs/Controller.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/ControllerSerial.h"
#include "outputs/E131Output.h"
#include "outputs/ArtNetOutput.h"
#include "outputs/DDPOutput.h"
#include "outputs/KinetOutput.h"
#include "outputs/ZCPPOutput.h"
#include "outputs/ZCPP.h"
#include "outputs/TwinklyOutput.h"
#include "outputs/OPCOutput.h"
#include "outputs/xxxEthernetOutput.h"
#include "outputs/xxxSerialOutput.h"
#include "outputs/LOROptimisedOutput.h"
#include "outputs/LorControllers.h"
#include "OutputModelManager.h"
#include "models/ModelManager.h"
#include "UtilFunctions.h"
#include "ui/wxUtilities.h"

// =========================================================================
// Factory
// =========================================================================

std::unique_ptr<OutputPropertyAdapter> OutputPropertyAdapter::Create(Output& output) {
    auto type = output.GetType();
    if (type == OUTPUT_E131) return std::make_unique<E131OutputPropertyAdapter>(output);
    if (type == OUTPUT_ARTNET) return std::make_unique<ArtNetOutputPropertyAdapter>(output);
    if (type == OUTPUT_DDP) return std::make_unique<DDPOutputPropertyAdapter>(output);
    if (type == OUTPUT_KINET) return std::make_unique<KinetOutputPropertyAdapter>(output);
    if (type == OUTPUT_ZCPP) return std::make_unique<ZCPPOutputPropertyAdapter>(output);
    if (type == OUTPUT_TWINKLY) return std::make_unique<TwinklyOutputPropertyAdapter>(output);
    if (type == OUTPUT_OPC) return std::make_unique<OPCOutputPropertyAdapter>(output);
    if (type == OUTPUT_xxxETHERNET) return std::make_unique<xxxEthernetOutputPropertyAdapter>(output);
    if (type == OUTPUT_xxxSERIAL) return std::make_unique<xxxSerialOutputPropertyAdapter>(output);
    if (type == OUTPUT_LOR_OPT) return std::make_unique<LOROptimisedOutputPropertyAdapter>(output);
    return std::make_unique<OutputPropertyAdapter>(output);
}

// =========================================================================
// LOR custom property editors (moved from LOROptimisedOutput.cpp)
// =========================================================================

class DeleteLorControllerDialogAdapter : public wxPGEditorDialogAdapter {
public:
    DeleteLorControllerDialogAdapter() : wxPGEditorDialogAdapter() { }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property)) override {
        wxVariant v("");
        SetValue(v);
        return true;
    }
};

class DeleteLorControllerProperty : public wxStringProperty {
    LorControllers& _lc;
public:
    DeleteLorControllerProperty(LorControllers& lc, const wxString& label, const wxString& name)
        : wxStringProperty(label, name, wxEmptyString), _lc(lc) { }
    virtual ~DeleteLorControllerProperty() { }

    const wxPGEditor* DoGetEditorClass() const override {
        return wxPGEditor_TextCtrlAndButton;
    }

    virtual wxPGEditorDialogAdapter* GetEditorDialog() const override {
        int pos = wxAtoi(GetName().AfterLast('/'));
        auto it = _lc.GetControllers().begin();
        std::advance(it, pos);
        wxASSERT(it != _lc.GetControllers().end());
        _lc.GetControllers().erase(it);
        return new DeleteLorControllerDialogAdapter();
    }
};

// =========================================================================
// E131OutputPropertyAdapter
// =========================================================================

E131OutputPropertyAdapter::E131OutputPropertyAdapter(Output& output)
    : OutputPropertyAdapter(output), _e131(static_cast<E131Output&>(output)) {}

void E131OutputPropertyAdapter::UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    ControllerEthernet* ce = dynamic_cast<ControllerEthernet*>(c);

    auto p = propertyGrid->GetProperty("Universes");
    if (p) {
        p->SetValue((int)c->GetOutputs().size());
        if (c->IsAutoSize()) {
            p->ChangeFlag(wxPGFlags::ReadOnly, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Universes Count cannot be changed when an output is set to Auto Size.");
        } else {
            p->ChangeFlag(wxPGFlags::ReadOnly, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
            p->SetEditor("SpinCtrl");
        }
    }
    p = propertyGrid->GetProperty("UniversePerString");
    if (p) {
        p->SetValue(ce->IsUniversePerString());
        if (ce->IsAutoSize() && ce->SupportsUniversePerString()) {
            p->Hide(false);
        } else {
            p->Hide(true);
        }
    }
    p = propertyGrid->GetProperty("UniversesDisplay");
    if (p) {
        if (ce->GetOutputs().size() > 1) {
            p->SetValue(ce->GetOutputs().front()->GetUniverseString() + " - " + ce->GetOutputs().back()->GetUniverseString());
            p->Hide(false);
        } else {
            p->Hide(true);
        }
    }
    p = propertyGrid->GetProperty("IndivSizes");
    if (p) {
        if (ce->IsAutoSize()) {
            p->ChangeFlag(wxPGFlags::ReadOnly, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Individual Sizes cannot be changed when an output is set to Auto Size.");
        } else {
            bool v = !ce->AllSameSize() || ce->IsForcingSizes() || ce->IsUniversePerString();
            p->SetValue(v);
            p->ChangeFlag(wxPGFlags::ReadOnly, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
        }
    }
    if (!ce->AllSameSize() || ce->IsForcingSizes()) {
        p = propertyGrid->GetProperty("Channels");
        if (p) {
            p->Hide(true);
        }
        auto p2 = propertyGrid->GetProperty("Sizes");
        if (p2) {
            p2->Hide(false);
            if (ce->IsExpanded()) {
                expandProperties.push_back(p2);
            }
            while (propertyGrid->GetFirstChild(p2)) {
                propertyGrid->RemoveProperty(propertyGrid->GetFirstChild(p2));
            }
            for (const auto& it : ce->GetOutputs()) {
                p = propertyGrid->AppendIn(p2, new wxUIntProperty(it->GetUniverseString(), "Channels/" + it->GetUniverseString(), it->GetChannels()));
                p->SetAttribute("Min", 1);
                p->SetAttribute("Max", it->GetMaxChannels());
                p->SetEditor("SpinCtrl");
                auto modelsOnUniverse = modelManager->GetModelsOnChannels(it->GetStartChannel(), it->GetEndChannel(), 4);
                p->SetHelpString(wxString::Format("[%d-%d]\n", it->GetStartChannel(), it->GetEndChannel()) + modelsOnUniverse);
                if (modelsOnUniverse != "") {
                    if (IsDarkMode()) {
                        p->SetBackgroundColour(wxColour(104, 128, 79));
                    } else {
                        p->SetBackgroundColour(wxColour(208, 255, 158));
                    }
                }
            }
        }
    } else {
        p = propertyGrid->GetProperty("Channels");
        if (p) {
            p->Hide(false);
            p->SetValue(_e131.GetChannels());
            if (ce->IsAutoSize()) {
                p->ChangeFlag(wxPGFlags::ReadOnly, true);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
                p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
            } else {
                p->SetEditor("SpinCtrl");
                p->ChangeFlag(wxPGFlags::ReadOnly, false);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
                p->SetHelpString("");
            }
        }
        auto p2 = propertyGrid->GetProperty("Sizes");
        if (p2) {
            p2->Hide(true);
        }
    }
}

void E131OutputPropertyAdapter::AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) {
    auto p = propertyGrid->Insert(before, new wxUIntProperty("Start Universe", "Universe", _e131.GetUniverse()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 64000);
    p->SetEditor("SpinCtrl");

    p = propertyGrid->Insert(before, new wxUIntProperty("Universe Count", "Universes", c->GetOutputs().size()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1000);

    p = propertyGrid->Insert(before, new wxBoolProperty("Universe Per String", "UniversePerString", false));
    p->SetEditor("CheckBox");

    p = propertyGrid->Insert(before, new wxStringProperty("Universes", "UniversesDisplay", ""));
    p->ChangeFlag(wxPGFlags::ReadOnly, true);
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

    p = propertyGrid->Insert(before, new wxBoolProperty("Individual Sizes", "IndivSizes", false));
    p->SetEditor("CheckBox");

    p = propertyGrid->Insert(before, new wxUIntProperty("Channels per Universe", "Channels", _e131.GetChannels()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", _e131.GetMaxChannels());

    propertyGrid->Insert(before, new wxPropertyCategory("Sizes", "Sizes"));
}

bool E131OutputPropertyAdapter::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) {
    return false;
}

void E131OutputPropertyAdapter::RemoveProperties(wxPropertyGrid* propertyGrid) {
    propertyGrid->DeleteProperty("Universe");
    propertyGrid->DeleteProperty("Universes");
    propertyGrid->DeleteProperty("UniversePerString");
    propertyGrid->DeleteProperty("UniversesDisplay");
    propertyGrid->DeleteProperty("IndivSizes");
    propertyGrid->DeleteProperty("Channels");
    propertyGrid->DeleteProperty("Sizes");
}

// =========================================================================
// ArtNetOutputPropertyAdapter
// =========================================================================

ArtNetOutputPropertyAdapter::ArtNetOutputPropertyAdapter(Output& output)
    : OutputPropertyAdapter(output), _artnet(static_cast<ArtNetOutput&>(output)) {}

void ArtNetOutputPropertyAdapter::UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    ControllerEthernet* ce = dynamic_cast<ControllerEthernet*>(c);

    auto p = propertyGrid->GetProperty("Universes");
    if (p) {
        p->SetValue((int)c->GetOutputs().size());
        if (c->IsAutoSize()) {
            p->ChangeFlag(wxPGFlags::ReadOnly, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Universes Count cannot be changed when an output is set to Auto Size.");
        } else {
            p->SetEditor("SpinCtrl");
            p->ChangeFlag(wxPGFlags::ReadOnly, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
        }
    }
    p = propertyGrid->GetProperty("UniversePerString");
    if (p) {
        p->SetValue(ce->IsUniversePerString());
        if (ce->IsAutoSize() && ce->SupportsUniversePerString()) {
            p->Hide(false);
        } else {
            p->Hide(true);
        }
    }
    p = propertyGrid->GetProperty("UniversesDisplay");
    if (p) {
        if (ce->GetOutputs().size() > 1) {
            p->SetValue(ce->GetOutputs().front()->GetUniverseString() + " - " + ce->GetOutputs().back()->GetUniverseString());
            p->Hide(false);
        } else {
            p->Hide(true);
        }
    }
    p = propertyGrid->GetProperty("IndivSizes");
    if (p) {
        if (ce->IsAutoSize()) {
            p->ChangeFlag(wxPGFlags::ReadOnly, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Individual Sizes cannot be changed when an output is set to Auto Size.");
        } else {
            bool v = !ce->AllSameSize() || ce->IsForcingSizes() || ce->IsUniversePerString();
            p->SetValue(v);
            p->ChangeFlag(wxPGFlags::ReadOnly, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
        }
    }
    if (!ce->AllSameSize() || ce->IsForcingSizes()) {
        p = propertyGrid->GetProperty("Channels");
        if (p) {
            p->Hide(true);
        }
        auto p2 = propertyGrid->GetProperty("Sizes");
        if (p2) {
            p2->Hide(false);
            if (ce->IsExpanded()) {
                expandProperties.push_back(p2);
            }
            while (propertyGrid->GetFirstChild(p2)) {
                propertyGrid->RemoveProperty(propertyGrid->GetFirstChild(p2));
            }
            for (const auto& it : ce->GetOutputs()) {
                p = propertyGrid->AppendIn(p2, new wxUIntProperty(it->GetUniverseString(), "Channels/" + it->GetUniverseString(), it->GetChannels()));
                p->SetAttribute("Min", 1);
                p->SetAttribute("Max", it->GetMaxChannels());
                p->SetEditor("SpinCtrl");
                auto modelsOnUniverse = modelManager->GetModelsOnChannels(it->GetStartChannel(), it->GetEndChannel(), 4);
                p->SetHelpString(wxString::Format("[%d-%d]\n", it->GetStartChannel(), it->GetEndChannel()) + modelsOnUniverse);
                if (modelsOnUniverse != "") {
                    if (IsDarkMode()) {
                        p->SetBackgroundColour(wxColour(104, 128, 79));
                    } else {
                        p->SetBackgroundColour(wxColour(208, 255, 158));
                    }
                }
            }
        }
    } else {
        p = propertyGrid->GetProperty("Channels");
        if (p) {
            p->Hide(false);
            p->SetValue(_artnet.GetChannels());
            if (ce->IsAutoSize()) {
                p->ChangeFlag(wxPGFlags::ReadOnly, true);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
                p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
            } else {
                p->SetEditor("SpinCtrl");
                p->ChangeFlag(wxPGFlags::ReadOnly, false);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
                p->SetHelpString("");
            }
        }
        auto p2 = propertyGrid->GetProperty("Sizes");
        if (p2) {
            p2->Hide(true);
        }
    }
}

void ArtNetOutputPropertyAdapter::AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) {
    auto p = propertyGrid->Insert(before, new wxBoolProperty("Force source port", "ForceSourcePort", _artnet.isForceSourcePort()));
    p->SetEditor("CheckBox");
    p->SetHelpString("You should only set this option if your ArtNet device is not seeing the ArtNet packets because it strictly requires that packets come from port 0x1936.");

    p = propertyGrid->Insert(before, new wxUIntProperty("Start Universe", "Universe", _artnet.GetUniverse()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 32767);
    p->SetEditor("SpinCtrl");

    p = propertyGrid->Insert(before, new wxUIntProperty("Universe Count", "Universes", c->GetOutputs().size()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1000);

    p = propertyGrid->Insert(before, new wxBoolProperty("Universe Per String", "UniversePerString", false));
    p->SetEditor("CheckBox");

    p = propertyGrid->Insert(before, new wxStringProperty("Universes", "UniversesDisplay", ""));
    p->ChangeFlag(wxPGFlags::ReadOnly, true);
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

    p = propertyGrid->Insert(before, new wxBoolProperty("Individual Sizes", "IndivSizes", false));
    p->SetEditor("CheckBox");

    p = propertyGrid->Insert(before, new wxUIntProperty("Channels per Universe", "Channels", _artnet.GetChannels()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", _artnet.GetMaxChannels());

    propertyGrid->Insert(before, new wxPropertyCategory("Sizes", "Sizes"));
}

bool ArtNetOutputPropertyAdapter::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) {
    wxString const name = event.GetPropertyName();
    if (name == "ForceSourcePort") {
        _artnet.SetForceSourcePort(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ArtNetOutput::HandlePropertyEvent::ForceSourcePort");
        return true;
    }
    return false;
}

void ArtNetOutputPropertyAdapter::RemoveProperties(wxPropertyGrid* propertyGrid) {
    propertyGrid->DeleteProperty("ForceSourcePort");
    propertyGrid->DeleteProperty("Universe");
    propertyGrid->DeleteProperty("Universes");
    propertyGrid->DeleteProperty("UniversePerString");
    propertyGrid->DeleteProperty("UniversesDisplay");
    propertyGrid->DeleteProperty("IndivSizes");
    propertyGrid->DeleteProperty("Channels");
    propertyGrid->DeleteProperty("Sizes");
}

// =========================================================================
// DDPOutputPropertyAdapter
// =========================================================================

DDPOutputPropertyAdapter::DDPOutputPropertyAdapter(Output& output)
    : OutputPropertyAdapter(output), _ddp(static_cast<DDPOutput&>(output)) {}

void DDPOutputPropertyAdapter::UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    auto p = propertyGrid->GetProperty("Channels");
    if (p) {
        p->SetValue(_ddp.GetChannels());
        if (c->IsAutoSize()) {
            p->ChangeFlag(wxPGFlags::ReadOnly, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
        } else {
            p->SetEditor("SpinCtrl");
            p->ChangeFlag(wxPGFlags::ReadOnly, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
        }
    }
}

void DDPOutputPropertyAdapter::AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) {
    auto p = propertyGrid->Insert(before, new wxUIntProperty("Channels Per Packet", "ChannelsPerPacket", _ddp.GetChannelsPerPacket()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1440);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("It would be very rare that you would ever want to change this from the default.");

    p = propertyGrid->Insert(before, new wxBoolProperty("Keep Channel Numbers", "KeepChannelNumbers", _ddp.IsKeepChannelNumbers()));
    p->SetEditor("CheckBox");
    p->SetHelpString("When not selected DDP data arrives at each controller looking like it starts at channel 1. When selected it arrives with the xLights channel number.");

    p = propertyGrid->Insert(before, new wxUIntProperty("Channels", "Channels", _ddp.GetChannels()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", _ddp.GetMaxChannels());
}

bool DDPOutputPropertyAdapter::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) {
    wxString const name = event.GetPropertyName();

    if (name == "ChannelsPerPacket") {
        _ddp.SetChannelsPerPacket(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "DDPOutput::HandlePropertyEvent::ChannelsPerPacket");
        return true;
    } else if (name == "KeepChannelNumbers") {
        _ddp.SetKeepChannelNumber(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "DDPOutput::HandlePropertyEvent::KeepChannelNumbers");
        return true;
    } else if (name == "Channels") {
        _ddp.SetChannels(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "DDPOutput::HandlePropertyEvent::Channels");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "DDPOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "DDPOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DDPOutput::HandlePropertyEvent::Channels", nullptr);
        return true;
    }

    return false;
}

void DDPOutputPropertyAdapter::RemoveProperties(wxPropertyGrid* propertyGrid) {
    propertyGrid->DeleteProperty("ChannelsPerPacket");
    propertyGrid->DeleteProperty("KeepChannelNumbers");
    propertyGrid->DeleteProperty("Channels");
}

// =========================================================================
// KinetOutputPropertyAdapter
// =========================================================================

KinetOutputPropertyAdapter::KinetOutputPropertyAdapter(Output& output)
    : OutputPropertyAdapter(output), _kinet(static_cast<KinetOutput&>(output)) {}

void KinetOutputPropertyAdapter::UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    ControllerEthernet* ce = dynamic_cast<ControllerEthernet*>(c);
    auto p = propertyGrid->GetProperty("Version");
    if (p) {
        p->SetValue(_kinet.GetVersion());
    }

    p = propertyGrid->GetProperty("Universes");
    if (p) {
        p->SetValue((int)c->GetOutputs().size());
        if (c->IsAutoSize()) {
            p->ChangeFlag(wxPGFlags::ReadOnly, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Universes Count cannot be changed when an output is set to Auto Size.");
        } else {
            p->SetEditor("SpinCtrl");
            p->ChangeFlag(wxPGFlags::ReadOnly, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
        }
    }
    p = propertyGrid->GetProperty("UniversesDisplay");
    if (p) {
        if (ce->GetOutputs().size() > 1) {
            p->SetValue(ce->GetOutputs().front()->GetUniverseString() + " - " + ce->GetOutputs().back()->GetUniverseString());
            p->Hide(false);
        } else {
            p->Hide(true);
        }
    }
    p = propertyGrid->GetProperty("IndivSizes");
    if (p) {
        if (ce->IsAutoSize()) {
            p->ChangeFlag(wxPGFlags::ReadOnly, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Individual Sizes cannot be changed when an output is set to Auto Size.");
        } else {
            bool v = !ce->AllSameSize() || ce->IsForcingSizes() || ce->IsUniversePerString();
            p->SetValue(v);
            p->ChangeFlag(wxPGFlags::ReadOnly, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
        }
    }
    if (!ce->AllSameSize() || ce->IsForcingSizes()) {
        p = propertyGrid->GetProperty("Channels");
        if (p) {
            p->Hide(true);
        }
        auto p2 = propertyGrid->GetProperty("Sizes");
        if (p2) {
            p2->Hide(false);
            if (ce->IsExpanded()) {
                expandProperties.push_back(p2);
            }
            while (propertyGrid->GetFirstChild(p2)) {
                propertyGrid->RemoveProperty(propertyGrid->GetFirstChild(p2));
            }
            for (const auto& it : ce->GetOutputs()) {
                p = propertyGrid->AppendIn(p2, new wxUIntProperty(it->GetUniverseString(), "Channels/" + it->GetUniverseString(), it->GetChannels()));
                p->SetAttribute("Min", 1);
                p->SetAttribute("Max", it->GetMaxChannels());
                p->SetEditor("SpinCtrl");
                auto modelsOnUniverse = modelManager->GetModelsOnChannels(it->GetStartChannel(), it->GetEndChannel(), 4);
                p->SetHelpString(wxString::Format("[%d-%d]\n", it->GetStartChannel(), it->GetEndChannel()) + modelsOnUniverse);
                if (modelsOnUniverse != "") {
                    if (IsDarkMode()) {
                        p->SetBackgroundColour(wxColour(104, 128, 79));
                    } else {
                        p->SetBackgroundColour(wxColour(208, 255, 158));
                    }
                }
            }
        }
    } else {
        p = propertyGrid->GetProperty("Channels");
        if (p) {
            p->Hide(false);
            p->SetValue(_kinet.GetChannels());
            if (ce->IsAutoSize()) {
                p->ChangeFlag(wxPGFlags::ReadOnly, true);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
                p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
            } else {
                p->SetEditor("SpinCtrl");
                p->ChangeFlag(wxPGFlags::ReadOnly, false);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
                p->SetHelpString("");
            }
        }
        auto p2 = propertyGrid->GetProperty("Sizes");
        if (p2) {
            p2->Hide(true);
        }
    }
}

void KinetOutputPropertyAdapter::AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) {
    auto p = propertyGrid->Insert(before, new wxUIntProperty("Version", "Version", 1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 2);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("Kinet protocol version.");

    p = propertyGrid->Insert(before, new wxUIntProperty("Start Port", "Universe", _kinet.GetUniverse()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 255);
    p->SetEditor("SpinCtrl");

    p = propertyGrid->Insert(before, new wxUIntProperty("Port Count", "Universes", c->GetOutputs().size()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1000);

    p = propertyGrid->Insert(before, new wxStringProperty("Ports", "UniversesDisplay", ""));
    p->ChangeFlag(wxPGFlags::ReadOnly, true);
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

    p = propertyGrid->Insert(before, new wxBoolProperty("Individual Sizes", "IndivSizes", false));
    p->SetEditor("CheckBox");

    p = propertyGrid->Insert(before, new wxUIntProperty("Channels per Port", "Channels", _kinet.GetChannels()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", _kinet.GetMaxChannels());

    propertyGrid->Insert(before, new wxPropertyCategory("Sizes", "Sizes"));
}

void KinetOutputPropertyAdapter::RemoveProperties(wxPropertyGrid* propertyGrid) {
    propertyGrid->DeleteProperty("Version");
    propertyGrid->DeleteProperty("Universe");
    propertyGrid->DeleteProperty("Universes");
    propertyGrid->DeleteProperty("UniversesDisplay");
    propertyGrid->DeleteProperty("IndivSizes");
    propertyGrid->DeleteProperty("Channels");
    propertyGrid->DeleteProperty("Sizes");
}

// =========================================================================
// ZCPPOutputPropertyAdapter
// =========================================================================

ZCPPOutputPropertyAdapter::ZCPPOutputPropertyAdapter(Output& output)
    : OutputPropertyAdapter(output), _zcpp(static_cast<ZCPPOutput&>(output)) {}

void ZCPPOutputPropertyAdapter::UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    auto p = propertyGrid->GetProperty("Channels");
    if (p) {
        p->SetValue(_zcpp.GetChannels());
        if (c->IsAutoSize()) {
            p->ChangeFlag(wxPGFlags::ReadOnly, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
        } else {
            p->SetEditor("SpinCtrl");
            p->ChangeFlag(wxPGFlags::ReadOnly, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
        }
    }
}

void ZCPPOutputPropertyAdapter::AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) {
    auto p = propertyGrid->Insert(before, new wxStringProperty("Multicast Address", "MulticastAddressDisplay", ZCPP_GetDataMulticastAddress(_zcpp.GetIP())));
    p->ChangeFlag(wxPGFlags::ReadOnly, true);
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

    p = propertyGrid->Insert(before, new wxBoolProperty("Supports Virtual Strings", "SupportsVirtualStrings", _zcpp.IsSupportsVirtualStrings()));
    p->SetEditor("CheckBox");

    p = propertyGrid->Insert(before, new wxBoolProperty("Supports Smart Remotes", "SupportsSmartRemotes", _zcpp.IsSupportsSmartRemotes()));
    p->SetEditor("CheckBox");

    p = propertyGrid->Insert(before, new wxBoolProperty("Send Data Multicast", "SendDataMulticast", _zcpp.IsMulticast()));
    p->SetEditor("CheckBox");

    p = propertyGrid->Insert(before, new wxBoolProperty("Suppress Sending Configuration", "DontSendConfig", _zcpp.IsDontConfigure()));
    p->SetEditor("CheckBox");

    p = propertyGrid->Insert(before, new wxUIntProperty("Channels", "Channels", _zcpp.GetChannels()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", _zcpp.GetMaxChannels());
}

bool ZCPPOutputPropertyAdapter::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) {
    wxString const name = event.GetPropertyName();

    if (name == "SupportsVirtualStrings") {
        _zcpp.SetSupportsVirtualStrings(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ZCPPOutput::HandlePropertyEvent::SupportsVirtualStrings");
        return true;
    } else if (name == "SupportsSmartRemotes") {
        _zcpp.SetSupportsSmartRemotes(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ZCPPOutput::HandlePropertyEvent::SupportsSmartRemotes");
        return true;
    } else if (name == "SendDataMulticast") {
        _zcpp.SetMulticast(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ZCPPOutput::HandlePropertyEvent::SendDataMulticast");
        return true;
    } else if (name == "DontSendConfig") {
        _zcpp.SetDontConfigure(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ZCPPOutput::HandlePropertyEvent::DontSendConfig");
        return true;
    } else if (name == "Channels") {
        _zcpp.SetChannels(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ZCPPOutput::HandlePropertyEvent::Channels");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ZCPPOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ZCPPOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ZCPPOutput::HandlePropertyEvent::Channels", nullptr);
        return true;
    }
    return false;
}

void ZCPPOutputPropertyAdapter::RemoveProperties(wxPropertyGrid* propertyGrid) {
    propertyGrid->DeleteProperty("DontSendConfig");
    propertyGrid->DeleteProperty("SendDataMulticast");
    propertyGrid->DeleteProperty("SupportsSmartRemotes");
    propertyGrid->DeleteProperty("SupportsVirtualStrings");
    propertyGrid->DeleteProperty("Channels");
}

// =========================================================================
// TwinklyOutputPropertyAdapter
// =========================================================================

TwinklyOutputPropertyAdapter::TwinklyOutputPropertyAdapter(Output& output)
    : OutputPropertyAdapter(output), _twinkly(static_cast<TwinklyOutput&>(output)) {}

void TwinklyOutputPropertyAdapter::UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    auto p = propertyGrid->GetProperty("HTTPPort");
    if (p) {
        p->SetValue(_twinkly.GetHttpPort());
    }
    p = propertyGrid->GetProperty("Channels");
    if (p) {
        p->SetValue(_twinkly.GetChannels());
        if (c->IsAutoSize()) {
            p->ChangeFlag(wxPGFlags::ReadOnly, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
        } else {
            p->SetEditor("SpinCtrl");
            p->ChangeFlag(wxPGFlags::ReadOnly, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
        }
    }
}

void TwinklyOutputPropertyAdapter::AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) {
    auto p = propertyGrid->Insert(before, new wxUIntProperty("HTTP Port", "HTTPPort", 80));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 65535);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("Twinkly normally listens on port 80 but you may want to change the port if using Artnet To Twinkly.");

    p = propertyGrid->Insert(before, new wxUIntProperty("Channels", "Channels", _twinkly.GetChannels()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", _twinkly.GetMaxChannels());
}

void TwinklyOutputPropertyAdapter::RemoveProperties(wxPropertyGrid* propertyGrid) {
    propertyGrid->DeleteProperty("HTTPPort");
    propertyGrid->DeleteProperty("Channels");
}

bool TwinklyOutputPropertyAdapter::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) {
    wxString const name = event.GetPropertyName();
    if (name == "HTTPPort") {
        _twinkly.SetHttpPort(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "TwinklyOutput::HandlePropertyEvent::HTTPPort");
        return true;
    } else if (name == "Channels") {
        _twinkly.SetChannels(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "TwinklyOutput::HandlePropertyEvent::Channels");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "TwinklyOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "TwinklyOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "TwinklyOutput::HandlePropertyEvent::Channels", nullptr);
        return true;
    }
    return false;
}

// =========================================================================
// OPCOutputPropertyAdapter
// =========================================================================

OPCOutputPropertyAdapter::OPCOutputPropertyAdapter(Output& output)
    : OutputPropertyAdapter(output), _opc(static_cast<OPCOutput&>(output)) {}

void OPCOutputPropertyAdapter::UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    ControllerEthernet* ce = dynamic_cast<ControllerEthernet*>(c);

    auto p = propertyGrid->GetProperty("Channels");
    if (p) {
        p->SetValue(_opc.GetChannels());
        if (ce->IsAutoSize()) {
            p->ChangeFlag(wxPGFlags::ReadOnly, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
        } else {
            p->SetEditor("SpinCtrl");
            p->ChangeFlag(wxPGFlags::ReadOnly, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
        }
    }
}

void OPCOutputPropertyAdapter::AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) {
    auto p = propertyGrid->Insert(before, new wxUIntProperty("OPC Channel", "Universe", _opc.GetUniverse()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 255);
    p->SetEditor("SpinCtrl");

    p = propertyGrid->Insert(before, new wxUIntProperty("Message Data Size", "Channels", _opc.GetChannels()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", _opc.GetMaxChannels());
}

bool OPCOutputPropertyAdapter::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) {
    return false;
}

void OPCOutputPropertyAdapter::RemoveProperties(wxPropertyGrid* propertyGrid) {
    propertyGrid->DeleteProperty("Universe");
    propertyGrid->DeleteProperty("Channels");
}

// =========================================================================
// xxxEthernetOutputPropertyAdapter
// =========================================================================

xxxEthernetOutputPropertyAdapter::xxxEthernetOutputPropertyAdapter(Output& output)
    : OutputPropertyAdapter(output), _xxx(static_cast<xxxEthernetOutput&>(output)) {}

void xxxEthernetOutputPropertyAdapter::UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    ControllerEthernet* ce = dynamic_cast<ControllerEthernet*>(c);

    auto p = propertyGrid->GetProperty("Universes");
    if (p) {
        p->SetValue((int)c->GetOutputs().size());
        if (c->IsAutoSize()) {
            p->ChangeFlag(wxPGFlags::ReadOnly, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Port Count cannot be changed when an output is set to Auto Size.");
        } else {
            p->SetEditor("SpinCtrl");
            p->ChangeFlag(wxPGFlags::ReadOnly, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
        }
    }
    p = propertyGrid->GetProperty("UniversesDisplay");
    if (p) {
        if (ce->GetOutputs().size() > 1) {
            p->SetValue(ce->GetOutputs().front()->GetUniverseString() + " - " + ce->GetOutputs().back()->GetUniverseString());
            p->Hide(false);
        } else {
            p->Hide(true);
        }
    }
    p = propertyGrid->GetProperty("IndivSizes");
    if (p) {
        if (ce->IsAutoSize()) {
            p->ChangeFlag(wxPGFlags::ReadOnly, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Individual Sizes cannot be changed when an output is set to Auto Size.");
        } else {
            bool v = !ce->AllSameSize() || ce->IsForcingSizes() || ce->IsUniversePerString();
            p->SetValue(v);
            p->ChangeFlag(wxPGFlags::ReadOnly, false);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
            p->SetHelpString("");
        }
    }
    if (!ce->AllSameSize() || ce->IsForcingSizes()) {
        p = propertyGrid->GetProperty("Channels");
        if (p) {
            p->Hide(true);
        }
        auto p2 = propertyGrid->GetProperty("Sizes");
        if (p2) {
            p2->Hide(false);
            if (ce->IsExpanded()) {
                expandProperties.push_back(p2);
            }
            while (propertyGrid->GetFirstChild(p2)) {
                propertyGrid->RemoveProperty(propertyGrid->GetFirstChild(p2));
            }
            for (const auto& it : ce->GetOutputs()) {
                p = propertyGrid->AppendIn(p2, new wxUIntProperty(it->GetUniverseString(), "Channels/" + it->GetUniverseString(), it->GetChannels()));
                p->SetAttribute("Min", 1);
                p->SetAttribute("Max", it->GetMaxChannels());
                p->SetEditor("SpinCtrl");
                auto modelsOnUniverse = modelManager->GetModelsOnChannels(it->GetStartChannel(), it->GetEndChannel(), 4);
                p->SetHelpString(wxString::Format("[%d-%d]\n", it->GetStartChannel(), it->GetEndChannel()) + modelsOnUniverse);
                if (modelsOnUniverse != "") {
                    if (IsDarkMode()) {
                        p->SetBackgroundColour(wxColour(104, 128, 79));
                    } else {
                        p->SetBackgroundColour(wxColour(208, 255, 158));
                    }
                }
            }
        }
    } else {
        p = propertyGrid->GetProperty("Channels");
        if (p) {
            p->Hide(false);
            p->SetValue(_xxx.GetChannels());
            if (ce->IsAutoSize()) {
                p->ChangeFlag(wxPGFlags::ReadOnly, true);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
                p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
            } else {
                p->SetEditor("SpinCtrl");
                p->ChangeFlag(wxPGFlags::ReadOnly, false);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
                p->SetHelpString("");
            }
        }
        auto p2 = propertyGrid->GetProperty("Sizes");
        if (p2) {
            p2->Hide(true);
        }
    }
}

void xxxEthernetOutputPropertyAdapter::AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) {
    auto p = propertyGrid->Insert(before, new wxUIntProperty("Start Port", "Universe", _xxx.GetUniverse()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 64000);
    p->SetEditor("SpinCtrl");

    p = propertyGrid->Insert(before, new wxUIntProperty("Port Count", "Universes", c->GetOutputs().size()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1000);

    p = propertyGrid->Insert(before, new wxStringProperty("Ports", "UniversesDisplay", ""));
    p->ChangeFlag(wxPGFlags::ReadOnly, true);
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

    p = propertyGrid->Insert(before, new wxBoolProperty("Individual Sizes", "IndivSizes", false));
    p->SetEditor("CheckBox");

    p = propertyGrid->Insert(before, new wxUIntProperty("Channels per Port", "Channels", _xxx.GetChannels()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", _xxx.GetMaxChannels());

    propertyGrid->Insert(before, new wxPropertyCategory("Sizes", "Sizes"));
}

void xxxEthernetOutputPropertyAdapter::RemoveProperties(wxPropertyGrid* propertyGrid) {
    propertyGrid->DeleteProperty("ForceSourcePort");
    propertyGrid->DeleteProperty("Universe");
    propertyGrid->DeleteProperty("Universes");
    propertyGrid->DeleteProperty("UniversesDisplay");
    propertyGrid->DeleteProperty("IndivSizes");
    propertyGrid->DeleteProperty("Channels");
    propertyGrid->DeleteProperty("Sizes");
}

// =========================================================================
// xxxSerialOutputPropertyAdapter
// =========================================================================

xxxSerialOutputPropertyAdapter::xxxSerialOutputPropertyAdapter(Output& output)
    : OutputPropertyAdapter(output), _xxxSerial(static_cast<xxxSerialOutput&>(output)) {}

void xxxSerialOutputPropertyAdapter::UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    ControllerSerial* ce = dynamic_cast<ControllerSerial*>(c);

    auto p = propertyGrid->GetProperty("DeviceChannels");
    if (p) {
        if (ce->GetOutputs().size() > 1) {
            p->SetValue(static_cast<xxxSerialOutput*>(ce->GetOutputs().front())->GetDeviceChannels());
            p->Hide(false);
        } else {
            p->Hide(true);
        }
    }
}

void xxxSerialOutputPropertyAdapter::AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) {
    propertyGrid->Insert(before, new wxStringProperty("Device Channels", "DeviceChannels", _xxxSerial.GetDeviceChannels()));
}

void xxxSerialOutputPropertyAdapter::RemoveProperties(wxPropertyGrid* propertyGrid) {
    propertyGrid->DeleteProperty("DeviceChannels");
}

bool xxxSerialOutputPropertyAdapter::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) {
    wxString const name = event.GetPropertyName();
    if (name == "DeviceChannels") {
        _xxxSerial.SetDeviceChannels(event.GetValue().GetString().ToStdString());
        wxPropertyGrid* grid = dynamic_cast<wxPropertyGrid*>(event.GetEventObject());
        grid->GetProperty("Channels")->SetValueFromString(wxString::Format("%d", _xxxSerial.GetChannels()));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "xxxSerialOutput::HandlePropertyEvent::DeviceChannels");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "xxxSerialOutput::HandlePropertyEvent::DeviceChannels", nullptr);
        return true;
    }
    return false;
}

// =========================================================================
// LOROptimisedOutputPropertyAdapter
// =========================================================================

LOROptimisedOutputPropertyAdapter::LOROptimisedOutputPropertyAdapter(Output& output)
    : OutputPropertyAdapter(output), _lor(static_cast<LOROptimisedOutput&>(output)) {}

void LOROptimisedOutputPropertyAdapter::AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) {
    _lor.EnsureTypesInitialised();

    auto devs = _lor.GetControllers().GetControllers();
    auto p = propertyGrid->Insert(before, new wxUIntProperty("Devices", "Devices", devs.size()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    static wxPGChoices lorDeviceTypes;
    static wxPGChoices lorAddressModes;
    if (lorDeviceTypes.GetCount() == 0) {
        lorDeviceTypes.Add("AC Controller");
        lorDeviceTypes.Add("RGB Controller");
        lorDeviceTypes.Add("CCR");
        lorDeviceTypes.Add("CCB");
        lorDeviceTypes.Add("Pixie2");
        lorDeviceTypes.Add("Pixie4");
        lorDeviceTypes.Add("Pixie8");
        lorDeviceTypes.Add("Pixie16");
    }
    if (lorAddressModes.GetCount() == 0) {
        lorAddressModes.Add("Normal");
        lorAddressModes.Add("Legacy");
        lorAddressModes.Add("Split");
    }

    int i = 0;
    for (const auto& it : devs) {
        auto isPixie = StartsWith(it->GetType(), "Pixie");

        wxPGProperty* p2 = propertyGrid->Insert(before, new wxPropertyCategory(it->GetType() + " : " + it->GetDescription(), wxString::Format("Device%d", i)));

        p = propertyGrid->AppendIn(p2, new DeleteLorControllerProperty(_lor.GetControllers(), _("Delete this device"), wxString::Format("DeleteDevice/%d", i)));
        propertyGrid->LimitPropertyEditing(p);

        propertyGrid->AppendIn(p2, new wxEnumProperty("Device Type", wxString::Format("DeviceType/%d", i), lorDeviceTypes, ControllerPropertyAdapter::EncodeChoices(lorDeviceTypes, it->GetType())));

        std::string ch = "Channels";
        if (isPixie) {
            ch = "Channels Per Port";
        }
        p = propertyGrid->AppendIn(p2, new wxUIntProperty(ch, wxString::Format("DeviceChannels/%d", i), it->GetNumChannels()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", it->GetMaxChannels());
        p->SetEditor("SpinCtrl");

        p = propertyGrid->AppendIn(p2, new wxUIntProperty("Unit ID", wxString::Format("DeviceUnitID/%d", i), it->GetUnitId()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", it->GetMaxUnitId());
        p->SetEditor("SpinCtrl");

        p = propertyGrid->AppendIn(p2, new wxStringProperty("Unit ID - Hex", wxString::Format("DeviceUnitIDHex/%d", i), wxString::Format("0x%02x", it->GetUnitId())));
        p->ChangeFlag(wxPGFlags::ReadOnly, true);
        p->SetBackgroundColour(*wxLIGHT_GREY);

        if (!isPixie) {
            propertyGrid->AppendIn(p2, new wxEnumProperty("Address Mode", wxString::Format("DeviceAddressMode/%d", i), lorAddressModes, (int)it->GetAddressMode()));
        }

        propertyGrid->AppendIn(p2, new wxStringProperty("Description", wxString::Format("DeviceDescription/%d", i), it->GetDescription()));

        if (it->IsExpanded()) expandProperties.push_back(p2);

        ++i;
    }
}

void LOROptimisedOutputPropertyAdapter::HandleExpanded(wxPropertyGridEvent& event, bool expanded) {
    wxString name = event.GetPropertyName();

    if (name.StartsWith("Device") && isdigit(name[name.size() - 1])) {
        int index = wxAtoi(name.substr(6));
        auto it = _lor.GetControllers().GetControllers().begin();
        std::advance(it, index);
        wxASSERT(it != _lor.GetControllers().GetControllers().end());
        (*it)->SetExpanded(expanded);
    }
}

bool LOROptimisedOutputPropertyAdapter::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) {
    wxString name = event.GetPropertyName();

    static wxPGChoices lorDeviceTypes;
    if (lorDeviceTypes.GetCount() == 0) {
        lorDeviceTypes.Add("AC Controller");
        lorDeviceTypes.Add("RGB Controller");
        lorDeviceTypes.Add("CCR");
        lorDeviceTypes.Add("CCB");
        lorDeviceTypes.Add("Pixie2");
        lorDeviceTypes.Add("Pixie4");
        lorDeviceTypes.Add("Pixie8");
        lorDeviceTypes.Add("Pixie16");
    }

    if (StartsWith(name, "DeleteDevice/")) {
        _lor.RecalcTotalChannels();
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::DeleteDevice");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::DeleteDevice");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::DeleteDevice");
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::DeleteDevice");
        return true;
    } else if (name == "Devices") {
        while (event.GetValue().GetLong() < (long)_lor.GetControllers().GetControllers().size()) {
            delete _lor.GetControllers().GetControllers().back();
            _lor.GetControllers().GetControllers().pop_back();
        }
        while (event.GetValue().GetLong() > (long)_lor.GetControllers().GetControllers().size()) {
            _lor.GetControllers().GetControllers().push_back(new LorController());
        }
        _lor.RecalcTotalChannels();
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Devices");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Devices");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::Devices");
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::Devices");
    } else if (StartsWith(name, "DeviceType/")) {
        int index = wxAtoi(wxString(name).AfterLast('/'));
        auto it = _lor.GetControllers().GetControllers().begin();
        std::advance(it, index);
        wxASSERT(it != _lor.GetControllers().GetControllers().end());
        (*it)->SetType(ControllerPropertyAdapter::DecodeChoices(lorDeviceTypes, event.GetValue().GetLong()));
        _lor.RecalcTotalChannels();
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::DeviceType");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::DeviceType");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::DeviceType");
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::DeviceType");
    } else if (StartsWith(name, "DeviceChannels/")) {
        int index = wxAtoi(wxString(name).AfterLast('/'));
        auto it = _lor.GetControllers().GetControllers().begin();
        std::advance(it, index);
        wxASSERT(it != _lor.GetControllers().GetControllers().end());
        (*it)->SetNumChannels(event.GetValue().GetLong());
        _lor.RecalcTotalChannels();
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::DeviceChannels");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::DeviceChannels");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::DeviceChannels");
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::DeviceChannels");
    } else if (StartsWith(name, "DeviceUnitID/")) {
        int index = wxAtoi(wxString(name).AfterLast('/'));
        auto it = _lor.GetControllers().GetControllers().begin();
        std::advance(it, index);
        wxASSERT(it != _lor.GetControllers().GetControllers().end());
        (*it)->SetUnitID(event.GetValue().GetLong());
        _lor.RecalcTotalChannels();
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::DeviceUnitID");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::DeviceUnitID");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::DeviceUnitID");
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::DeviceUnitID");
    } else if (StartsWith(name, "DeviceAddressMode/")) {
        int index = wxAtoi(wxString(name).AfterLast('/'));
        auto it = _lor.GetControllers().GetControllers().begin();
        std::advance(it, index);
        wxASSERT(it != _lor.GetControllers().GetControllers().end());
        (*it)->SetMode((LorController::AddressMode)event.GetValue().GetLong());
        _lor.RecalcTotalChannels();
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::DeviceAddressMode");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::DeviceAddressMode");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::DeviceAddressMode");
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::DeviceAddressMode");
    } else if (StartsWith(name, "DeviceDescription/")) {
        int index = wxAtoi(wxString(name).AfterLast('/'));
        auto it = _lor.GetControllers().GetControllers().begin();
        std::advance(it, index);
        wxASSERT(it != _lor.GetControllers().GetControllers().end());
        (*it)->SetDescription(event.GetValue().GetString());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::DeviceDescription");
    }

    return false;
}
