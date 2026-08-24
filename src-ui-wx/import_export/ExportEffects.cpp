/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Export functions extracted from Render.cpp, xLightsMain.cpp

#include <wx/file.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

#include "../xLightsMain.h"
#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"
#include "utils/ExternalHooks.h"
#include "render/FSEQFile.h"
#include "Parallel.h"
#include "../import_export/SeqExportDialog.h"
#include "layout/LayoutPanel.h"
#include "models/Model.h"
#include "import_export/ExportModels.h"
#include "outputs/OutputManager.h"
#include "outputs/Controller.h"

#include <log.h>
#include <filesystem>

// --- Extracted from xLightsMain.cpp ---

void xLightsFrame::ExportModels(wxString const& filename)
{
    // make sure everything is up to date
    if (Notebook1->GetSelection() != LAYOUTTAB) {
        layoutPanel->UnSelectAllModels();
    }
    RecalcModels();

    if (!::ExportModels(filename.ToStdString(), AllModels, _outputManager)) {
        DisplayError("Unable to create Spreadsheet. Usually a permissions error.");
    }
}


void xLightsFrame::ExportEffects(wxString const& filename)
{
    wxFile f(filename);

    if (!f.Create(filename, true) || !f.IsOpened()) {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString(), this);
        return;
    }

    std::map<std::string, int> effectfrequency;
    std::map<std::string, int> effecttotaltime;

    int effects = 0;
    f.Write(_("Effect Name,StartTime,EndTime,Duration,Description,Element,ElementType,Files\n"));

    std::list<std::string> files;

    for (size_t i = 0; i < _sequenceElements.GetElementCount(0); i++) {
        Element* e = _sequenceElements.GetElement(i);
        effects += ExportElement(f, e, effectfrequency, effecttotaltime, files);

        if (dynamic_cast<ModelElement*>(e) != nullptr) {
            for (int s = 0; s < dynamic_cast<ModelElement*>(e)->GetSubModelAndStrandCount(); s++) {
                SubModelElement* se = dynamic_cast<ModelElement*>(e)->GetSubModel(s);
                effects += ExportElement(f, se, effectfrequency, effecttotaltime, files);
            }
            for (int s = 0; s < dynamic_cast<ModelElement*>(e)->GetStrandCount(); s++) {
                StrandElement* se = dynamic_cast<ModelElement*>(e)->GetStrand(s);
                int node = 0;
                for (int n = 0; n < se->GetNodeLayerCount(); n++) {
                    NodeLayer* nl = se->GetNodeLayer(n);
                    effects += ExportNodes(f, se, nl, node++, effectfrequency, effecttotaltime, files);
                }
            }
        }
    }
    f.Write(wxString::Format("\"Effect Count\",%d\n", effects));
    f.Write(_("\n"));
    f.Write(_("Effect Usage Summary\n"));
    f.Write(_("Effect Name,Occurences,TotalTime\n"));
    for (auto it = effectfrequency.begin(); it != effectfrequency.end(); ++it) {
        int tt = effecttotaltime[it->first];
        f.Write(wxString::Format("\"%s\",%d,%02d:%02d.%03d\n",
                                 (const char*)it->first.c_str(),
                                 it->second,
                                 tt / 60000,
                                 (tt % 60000) / 1000,
                                 tt % 1000));
    }
    f.Write(_("\n"));
    f.Write(_("Summary of files used\n"));

    files.sort();
    files.unique();
    for (auto it = files.begin(); it != files.end(); ++it) {
        f.Write(wxString::Format("%s\n", *it));
    }

    f.Close();
}

