/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "FlickerFreeBitmapButton.h"

BEGIN_EVENT_TABLE(FlickerFreeBitmapButton,wxBitmapButton)
    EVT_ERASE_BACKGROUND(FlickerFreeBitmapButton::OnEraseBackGround)
END_EVENT_TABLE()

FlickerFreeBitmapButton::~FlickerFreeBitmapButton()
{
    //dtor
}
