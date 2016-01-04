#include "SnowflakesEffect.h"
#include "SnowflakesPanel.h"

SnowflakesEffect::SnowflakesEffect(int id) : RenderableEffect(id, "Snowflakes")
{
    //ctor
}

SnowflakesEffect::~SnowflakesEffect()
{
    //dtor
}
wxPanel *SnowflakesEffect::CreatePanel(wxWindow *parent) {
    return new SnowflakesPanel(parent);
}