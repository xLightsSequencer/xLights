/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

// free functions to create a CSV/Printing export configuration
#include "ExportSettings.h"
#include "UtilFunctions.h"

#include <wx/window.h>
#include <wx/choicdlg.h>
#include <wx/string.h>
#include <wx/arrstr.h>

namespace ExportSettings
{
    SETTINGS GetSettings(wxWindow* parent) {
        SETTINGS sett = SETTINGS_NONE;
        wxArrayString choices;
        choices.Add("Port Start Channel");
        choices.Add("Port Universe Start Channel");
        choices.Add("Port Channel Count");
        choices.Add("Port Pixel Count");
        choices.Add("Port Current");
        choices.Add("Model Description");
        choices.Add("Model Start Channel");
        choices.Add("Model Universe Start Channel");
        choices.Add("Model Channel Count");
        choices.Add("Model Pixel Count");
        choices.Add("Model Current");

        wxMultiChoiceDialog dlg(parent, "Export to CSV Options", "Fields to Include", choices);
        //OptimiseDialogPosition(&dlg);

        if (dlg.ShowModal() == wxID_OK ) {
            for (auto const& idx : dlg.GetSelections()) {
                sett |= static_cast< SETTINGS >( 1 << idx );
            }
        }

        return sett;
    }
}