#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/panel.h>
#include "Color.h"

class xlColorPicker: public wxPanel
{
	public:

		xlColorPicker(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long style=wxTAB_TRAVERSAL, const wxString& name="Any");
		virtual ~xlColorPicker();

        virtual void SetColor(xlColor& color ) = 0;
        virtual int GetActiveButton() = 0;
        virtual wxColour GetButtonColor(int p) = 0;
        virtual void SetButtonColor(int p, xlColor& color) = 0;
        virtual void ResetPanel() = 0;
        const xlColor& GetColor() { return mCurrentColor; }

        virtual void NotifyColorChange() = 0;

	protected:

		xlColor mCurrentColor;

	private:

		DECLARE_EVENT_TABLE()
};
