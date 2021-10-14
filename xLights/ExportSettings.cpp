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
        choices.Add("Model Description");
        choices.Add("Model Start Channel");
        choices.Add("Model Universe Start Channel");
        choices.Add("Model Channel Count");
        choices.Add("Model Pixel Count");

        wxMultiChoiceDialog dlg(parent, "Export to CSV Options", "Fields to Include", choices);
        //OptimiseDialogPosition(&dlg);

        if (dlg.ShowModal() == wxID_OK) {
            for (auto const& idx : dlg.GetSelections()) {
                switch (idx) {
                    case 0: {
                        sett |= SETTINGS_PORT_ABSADDRESS;
                        break;
                    }
                    case 1: {
                        sett |= SETTINGS_PORT_UNIADDRESS;
                        break;
                    }
                    case 2: {
                        sett |= SETTINGS_PORT_CHANNELS;
                        break;
                    }
                    case 3: {
                        sett |= SETTINGS_PORT_PIXELS;
                        break;
                    }
                    case 4: {
                        sett |= SETTINGS_MODEL_DESCRIPTIONS;
                        break;
                    }
                    case 5: {
                        sett |= SETTINGS_MODEL_ABSADDRESS;
                        break;
                    }
                    case 6: {
                        sett |= SETTINGS_MODEL_UNIADDRESS;
                        break;
                    }
                    case 7: {
                        sett |= SETTINGS_MODEL_CHANNELS;
                        break;
                    }
                    case 8: {
                        sett |= SETTINGS_MODEL_PIXELS;
                        break;
                    }
                }
            }
        }

        return sett;
    }
}