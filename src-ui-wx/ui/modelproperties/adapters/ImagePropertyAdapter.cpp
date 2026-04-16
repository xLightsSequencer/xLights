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

#include "ui/shared/utils/wxUtilities.h"
#include "ImagePropertyAdapter.h"
#include "models/ImageModel.h"
#include "models/OutputModelManager.h"
#include "utils/ExternalHooks.h"

ImagePropertyAdapter::ImagePropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _image(static_cast<ImageModel&>(model)) {}

void ImagePropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p = grid->Append(new wxImageFileProperty("Image",
                                             "Image",
                                             _image.GetImageFile()));
    p->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg"
                                        ";*.webp"
        "|All files (*.*)|*.*");

    p = grid->Append(new wxUIntProperty("Off Brightness", "OffBrightness", _image.GetOffBrightness()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 200);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("Read White As Alpha",
        "WhiteAsAlpha",
        _image.IsWhiteAsAlpha()));
    p->SetAttribute("UseCheckbox", true);
}

int ImagePropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    _image.IncrementChangeCount();
    if ("Image" == event.GetPropertyName()) {
        _image.ClearImageCache();
        std::string val = ToStdString(event.GetValue().GetString());
        _image.SetImageFile(val);
        _image.IncrementChangeCount();
        _image.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "ImagePropertyAdapter::OnPropertyGridChange::Image");
        return 0;
    } else if ("WhiteAsAlpha" == event.GetPropertyName()) {
        _image.SetWhiteAsAlpha(event.GetValue().GetBool());
        _image.ClearImageCache();
        _image.IncrementChangeCount();
        _image.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "ImagePropertyAdapter::OnPropertyGridChange::WhiteAsAlpha");
        return 0;
    } else if ("OffBrightness" == event.GetPropertyName()) {
        _image.SetOffBrightness(event.GetValue().GetInteger());
        _image.IncrementChangeCount();
        _image.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "ImagePropertyAdapter::OnPropertyGridChange::OffBrightness");
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}

void ImagePropertyAdapter::DisableUnusedProperties(wxPropertyGridInterface* grid) {
    wxPGProperty* p = grid->GetPropertyByName("ModelFaces");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelDimmingCurves");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelStates");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelStrandNodeNames");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("SubModels");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelPixelSize");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelPixelStyle");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelPixelBlackTransparency");
    if (p != nullptr) {
        p->Enable(false);
    }
}
