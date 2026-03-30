/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// UI event handlers related to rendering — separated from the
// render orchestration in Render.cpp.

#include "xLightsMain.h"
#include "SeqExportDialog.h"
#include "ExternalHooks.h"
#include "UtilFunctions.h"
#include "models/ModelGroup.h"

void xLightsFrame::ExportModel(wxCommandEvent& command)
{
    unsigned int startFrame = 0;
    unsigned int endFrame = _seqData.NumFrames();
    std::string cmdStr = command.GetString().ToStdString();
    std::vector<std::string> as;
    Split(cmdStr, '|', as);
    if (as.size() == 3) {
        startFrame = (int)std::strtol(as[1].c_str(), nullptr, 10);
        endFrame = (int)std::strtol(as[2].c_str(), nullptr, 10);
    }

    std::string model = as[0];
    Model* m = GetModel(model);
    if (m == nullptr)
        return;

    bool isgroup = (m->GetDisplayAs() == DisplayAsType::ModelGroup);

    bool isboxed = false;
    if (dynamic_cast<ModelWithScreenLocation<BoxedScreenLocation>*>(m) != nullptr) {
        // line models, arches etc make no sense for videos
        isboxed = true;
    }

    SeqExportDialog dialog(this, m->GetName());
    dialog.ModelExportTypes(isgroup || !isboxed);
    dialog.SetExportType(command.GetString().Contains('|'), command.GetInt() == 1);

    if (dialog.ShowModal() == wxID_OK) {
        std::string filename = dialog.TextCtrlFilename->GetValue().ToStdString();
        ObtainAccessToURL(filename);
        EnableSequenceControls(false);
        std::string format = dialog.ChoiceFormat->GetStringSelection().ToStdString();

        DoExportModel(startFrame, endFrame, model, filename, format, command.GetInt() == 1);
    }
}
