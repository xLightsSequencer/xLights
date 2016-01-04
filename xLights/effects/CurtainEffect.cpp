#include "CurtainEffect.h"
#include "CurtainPanel.h"

CurtainEffect::CurtainEffect(int i) : RenderableEffect(i, "Curtain")
{
    //ctor
}

CurtainEffect::~CurtainEffect()
{
    //dtor
}
wxPanel *CurtainEffect::CreatePanel(wxWindow *parent) {
    return new CurtainPanel(parent);
}