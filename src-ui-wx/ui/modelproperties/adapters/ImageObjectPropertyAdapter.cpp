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
#include "ImageObjectPropertyAdapter.h"
#include "models/ImageObject.h"
#include "utils/ExternalHooks.h"

ImageObjectPropertyAdapter::ImageObjectPropertyAdapter(ImageObject& obj)
    : ViewObjectPropertyAdapter(obj), _image(obj) {}

void ImageObjectPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty *p = grid->Append(new wxImageFileProperty("Image",
                                             "Image",
                                             _image.GetImageFile()));
    p->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg"
                                        ";*.webp"
                                        "|All files (*.*)|*.*");

    p = grid->Append(new wxUIntProperty("Transparency", "Transparency", _image.GetTransparency()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");
    p = grid->Append(new wxUIntProperty("Brightness", "Brightness", _image.GetBrightness()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");
}

int ImageObjectPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("Image" == event.GetPropertyName()) {
        _image.ClearImages();
        std::string imageFile = ToStdString(event.GetValue().GetString());
        _image.SetImageFileDirect(imageFile);
        ObtainAccessToURL(imageFile);
        _image.IncrementChangeCount();
        _image.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ImageObject::OnPropertyGridChange::Image");
        _image.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ImageObject::OnPropertyGridChange::Image");
        return 0;
    } else if ("Transparency" == event.GetPropertyName()) {
        _image.SetTransparency((int)event.GetPropertyValue().GetLong());
        _image.IncrementChangeCount();
        _image.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ImageObject::OnPropertyGridChange::Transparency");
        _image.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ImageObject::OnPropertyGridChange::Transparency");
        return 0;
    } else if ("Brightness" == event.GetPropertyName()) {
        _image.SetBrightness((int)event.GetPropertyValue().GetLong());
        _image.IncrementChangeCount();
        _image.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ImageObject::OnPropertyGridChange::Brightness");
        _image.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ImageObject::OnPropertyGridChange::Brightness");
        return 0;
    }

    return ViewObjectPropertyAdapter::OnPropertyGridChange(grid, event);
}
