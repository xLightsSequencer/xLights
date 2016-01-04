#include "MeteorsEffect.h"
#include "MeteorsPanel.h"

MeteorsEffect::MeteorsEffect(int id) : RenderableEffect(id, "Meteors")
{
    //ctor
}

MeteorsEffect::~MeteorsEffect()
{
    //dtor
}
wxPanel *MeteorsEffect::CreatePanel(wxWindow *parent) {
    return new MeteorsPanel(parent);
}