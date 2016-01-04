#include "TextEffect.h"

#include "TextPanel.h"
#include <wx/checkbox.h>

TextEffect::TextEffect(int id) : RenderableEffect(id, "Text")
{
    //ctor
}

TextEffect::~TextEffect()
{
    //dtor
}

wxPanel *TextEffect::CreatePanel(wxWindow *parent) {
    return new TextPanel(parent);
}


static inline void SetCheckboxValue(wxWindow *w, int id, bool b) {
    wxCheckBox *c = (wxCheckBox*)w->FindWindowById(id);
    c->SetValue(b);
    wxCommandEvent evt(wxEVT_COMMAND_CHECKBOX_CLICKED, id);
    evt.SetEventObject(c);
    evt.SetInt(b);
    c->ProcessWindowEvent(evt);
}

void TextEffect::SetDefaultParameters(ModelClass *cls) {
    TextPanel *tp = (TextPanel*)panel;
    if (tp == nullptr) {
        return;
    }
    
    SetCheckboxValue(tp, tp->ID_CHECKBOX_TextToCenter1, false);
    SetCheckboxValue(tp, tp->ID_CHECKBOX_TextToCenter2, false);
    SetCheckboxValue(tp, tp->ID_CHECKBOX_TextToCenter3, false);
    SetCheckboxValue(tp, tp->ID_CHECKBOX_TextToCenter4, false);
}
