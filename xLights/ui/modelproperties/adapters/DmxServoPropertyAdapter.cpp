/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/msgdlg.h>

#include "DmxServoPropertyAdapter.h"
#include "../helpers/DmxAbilityPropertyHelpers.h"
#include "../helpers/DmxComponentPropertyHelpers.h"
#include "../../../models/DMX/DmxServo.h"
#include "../../../models/DMX/DmxImage.h"
#include "../../../models/DMX/Servo.h"
#include "../../../controllers/ControllerCaps.h"
#include "../../../models/OutputModelManager.h"

static const int SUPPORTED_SERVOS = 24;

DmxServoPropertyAdapter::DmxServoPropertyAdapter(Model& model)
    : DmxPropertyAdapter(model), _servo(static_cast<DmxServo&>(model)) {}

void DmxServoPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    // DmxModel base properties (# Channels + preset)
    auto p = grid->Append(new wxUIntProperty("# Channels", "DmxChannelCount", _servo.GetDmxChannelCount()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    if (_servo.HasPresetAbility()) {
        DmxAbilityPropertyHelpers::AddPresetProperties(grid, *_servo.GetPresetAbility(), _servo.GetDmxChannelCount());
    }

    p = grid->Append(new wxUIntProperty("Num Servos", "NumServos", (int)_servo.GetNumServos()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", SUPPORTED_SERVOS);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("16 Bit", "Bits16", _servo.Is16Bit()));
    p->SetAttribute("UseCheckbox", true);

    p = grid->Append(new wxUIntProperty("Brightness", "Brightness", (int)_servo.GetBrightness()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Transparency", "Transparency", _servo.GetTransparency()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    for (int i = 0; i < _servo.GetNumServos(); ++i) {
        ControllerCaps* caps = _servo.GetControllerCaps();
        DmxComponentPropertyHelpers::AddServoProperties(grid, *_servo.GetServo(i), _servo.IsPWMProtocol() && caps != nullptr && caps->SupportsPWM());
    }

    for (int i = 0; i < _servo.GetNumServos(); ++i) {
        DmxComponentPropertyHelpers::AddImageProperties(grid, *_servo.GetStaticImage(i));
    }

    for (int i = 0; i < _servo.GetNumServos(); ++i) {
        DmxComponentPropertyHelpers::AddImageProperties(grid, *_servo.GetMotionImage(i));
    }

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

int DmxServoPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();

    if ("DmxChannelCount" == event.GetPropertyName()) {
        int channels = (int)event.GetPropertyValue().GetLong();
        int min_channels = _servo.GetNumServos() * (_servo.Is16Bit() ? 2 : 1);
        if (channels < min_channels) {
            wxPGProperty* p = grid->GetPropertyByName("DmxChannelCount");
            if (p != nullptr) {
                p->SetValue(min_channels);
            }
            std::string msg = wxString::Format("You have %d servos at %d bits so you need %d channels minimum.", _servo.GetNumServos(), _servo.Is16Bit() ? 16 : 8, min_channels);
            wxMessageBox(msg, "Minimum Channel Violation", wxOK | wxCENTER);
            return 0;
        }
    }
    if ("NumServos" == name) {
        _servo.SetUpdateNodeNames(true);
        _servo.SetNumServos((int)event.GetPropertyValue().GetLong());
        _servo.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::NumServos");
        _servo.AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo::OnPropertyGridChange::NumServos");
        _servo.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::NumServos");
        _servo.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo::OnPropertyGridChange::NumServos");
        return 0;
    }
    if (event.GetPropertyName() == "Bits16") {
        bool is16bit = event.GetValue().GetBool();
        _servo.SetIs16Bit(is16bit);

        for (int i = 0; i < _servo.GetNumServos(); ++i) {
            if (_servo.GetServo(i) != nullptr) {
                _servo.GetServo(i)->SetChannel(is16bit ? i * 2 + 1 : i + 1);
            }
        }

        int min_channels = _servo.GetNumServos() * (is16bit ? 2 : 1);
        if (_servo.GetDmxChannelCount() < min_channels) {
            _servo.UpdateChannelCount(min_channels, true);
        }
        _servo.SetUpdateNodeNames(true);
        _servo.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::Bits16");
        _servo.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo::OnPropertyGridChange::Bits16");
        _servo.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::Bits16");
        return 0;
    }
    if ("Transparency" == name) {
        _servo.SetTransparency((int)event.GetPropertyValue().GetLong());
        _servo.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::Transparency");
        _servo.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::Transparency");
        return 0;
    }
    if ("Brightness" == name) {
        _servo.SetBrightness((int)event.GetPropertyValue().GetLong());
        _servo.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::Brightness");
        _servo.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::Brightness");
        return 0;
    }

    for (int i = 0; i < _servo.GetNumServos(); ++i) {
        if (DmxComponentPropertyHelpers::OnServoPropertyGridChange(grid, event, *_servo.GetServo(i), &_servo, _servo.GetModelScreenLocation().IsLocked() || _servo.IsFromBase()) == 0) {
            return 0;
        }
    }

    for (int i = 0; i < _servo.GetNumServos(); ++i) {
        if (DmxComponentPropertyHelpers::OnImagePropertyGridChange(grid, event, *_servo.GetStaticImage(i), &_servo, _servo.GetModelScreenLocation().IsLocked() || _servo.IsFromBase()) == 0) {
            return 0;
        }
    }

    for (int i = 0; i < _servo.GetNumServos(); ++i) {
        if (DmxComponentPropertyHelpers::OnImagePropertyGridChange(grid, event, *_servo.GetMotionImage(i), &_servo, _servo.GetModelScreenLocation().IsLocked() || _servo.IsFromBase()) == 0) {
            return 0;
        }
    }

    // DmxModel base handling
    _servo.IncrementChangeCount();
    if ("DmxChannelCount" == event.GetPropertyName()) {
        _servo.IncrementChangeCount();
        _servo.UpdateChannelCount((int)event.GetPropertyValue().GetLong(), true);
        return 0;
    }

    if (_servo.HasPresetAbility() && DmxAbilityPropertyHelpers::OnPresetPropertyGridChange(grid, event, *_servo.GetPresetAbility(), _servo.GetDmxChannelCount(), &_servo) == 0) {
        _servo.IncrementChangeCount();
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
