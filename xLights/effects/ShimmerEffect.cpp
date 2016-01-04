#include "ShimmerEffect.h"
#include "ShimmerPanel.h"

ShimmerEffect::ShimmerEffect(int id) : RenderableEffect(id, "Shimmer")
{
    //ctor
}

ShimmerEffect::~ShimmerEffect()
{
    //dtor
}

wxPanel *ShimmerEffect::CreatePanel(wxWindow *parent) {
    return new ShimmerPanel(parent);
}