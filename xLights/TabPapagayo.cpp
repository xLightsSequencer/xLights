/***************************************************************
 * Name:      TabPapagayo.cpp
 * Purpose:   Code for Application Frame
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-11-03
 * Copyright: Matt Brown ()
 * License:
 **************************************************************/

#include "xLightsMain.h"

void xLightsFrame::OnButton_pgo_filenameClick(wxCommandEvent& event)
{
     wxString filename = wxFileSelector( "Choose Papagayo File", "", "", "", "Papagayo files (*.pgo)|*.pgo", wxFD_OPEN );
 //  wxString filename = "this5.pgo";
    if (!filename.IsEmpty()) TextCtrl_pgo_filename->SetValue(filename);
}

void xLightsFrame::OnButton_papagayo_output_sequenceClick(wxCommandEvent& event)
{
   wxString filename = wxFileSelector( "Choose Output xLights Sequence File", "", "", "", "xLights files (*.xml)|*.xml", wxFD_OPEN );

    if (!filename.IsEmpty()) TextCtrl_papagayo_output_filename->SetValue(filename);
}

void xLightsFrame::OnButtonStartPapagayoClick(wxCommandEvent& event)
{
    ButtonStartPapagayo->Enable(false);
    wxString OutputFormat = ChoiceOutputFormat->GetStringSelection();
    TextCtrlConversionStatus->Clear();

    // check inputs
    if (FileNames.IsEmpty()) {
        wxMessageBox(_("Please select one or more sequence files"), _("Error"));
    } else if (OutputFormat.IsEmpty()) {
        wxMessageBox(_("Please select an output format"), _("Error"));
    } else {
        for (size_t i=0; i < FileNames.GetCount(); i++) {
            DoConversion(FileNames[i], OutputFormat);
        }
        TextCtrlConversionStatus->AppendText(_("Finished converting all files\n"));
    }

    ButtonStartPapagayo->Enable(true);
}

