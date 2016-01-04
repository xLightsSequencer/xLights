#include "TreeEffect.h"
#include "TreePanel.h"

TreeEffect::TreeEffect(int id) : RenderableEffect(id, "Tree")
{
    //ctor
}

TreeEffect::~TreeEffect()
{
    //dtor
}
wxPanel *TreeEffect::CreatePanel(wxWindow *parent) {
    return new TreePanel(parent);
}