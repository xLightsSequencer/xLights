#include "FlickerFreeBitmapButton.h"

BEGIN_EVENT_TABLE(FlickerFreeBitmapButton,wxBitmapButton)
    EVT_ERASE_BACKGROUND(FlickerFreeBitmapButton::OnEraseBackGround)
END_EVENT_TABLE()

FlickerFreeBitmapButton::~FlickerFreeBitmapButton()
{
    //dtor
}
