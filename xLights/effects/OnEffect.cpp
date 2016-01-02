#include "OnEffect.h"
#include "OnPanel.h"

OnEffect::OnEffect(int i) : RenderableEffect(i, "On")
{
    //ctor
}

OnEffect::~OnEffect()
{
    //dtor
}

wxPanel *OnEffect::CreatePanel(wxWindow *parent) {
    return new OnPanel(parent);
}
