#include "ColorWashEffect.h"

#include "ColorWashPanel.h"
#include <wx/checkbox.h>

ColorWashEffect::ColorWashEffect(int i) : RenderableEffect(i, "Color Wash")
{
    //ctor
}

ColorWashEffect::~ColorWashEffect()
{
    //dtor
}
static inline void SetCheckboxValue(wxWindow *w, int id, bool b) {
    wxCheckBox *c = (wxCheckBox*)w->FindWindowById(id);
    c->SetValue(b);
    wxCommandEvent evt(wxEVT_COMMAND_CHECKBOX_CLICKED, id);
    evt.SetEventObject(c);
    evt.SetInt(b);
    c->ProcessWindowEvent(evt);
}


void ColorWashEffect::SetDefaultParameters(ModelClass *cls) {
    ColorWashPanel *cwp = (ColorWashPanel*)panel;
    if (cwp == nullptr) {
        return;
    }
    SetCheckboxValue(cwp, cwp->ID_CHECKBOX_ColorWash_EntireModel, true);
}
wxPanel *ColorWashEffect::CreatePanel(wxWindow *parent) {
    return new ColorWashPanel(parent);
}
