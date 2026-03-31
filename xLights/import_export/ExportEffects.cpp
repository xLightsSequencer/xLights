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
#include "../ui/wxUtilities.h"
#include "utils/ExternalHooks.h"
#include "../render/FSEQFile.h"
#include "Parallel.h"
#include "../ui/import-export/SeqExportDialog.h"
#include "ui/layout/LayoutPanel.h"
#include "../models/Model.h"
#include "../models/ModelGroup.h"
#include "../models/RulerObject.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Controller.h"

#include <xlsxwriter.h>

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

    constexpr double FACTOR = 1.3;

    uint32_t minchannel = 99999999;
    int32_t maxchannel = -1;

    lxw_workbook* workbook = workbook_new(filename.c_str());
    lxw_worksheet* modelsheet = workbook_add_worksheet(workbook, "Models");
    lxw_worksheet* groupsheet = workbook_add_worksheet(workbook, "Groups");
    lxw_worksheet* controllersheet = workbook_add_worksheet(workbook, "Controllers");
    lxw_worksheet* totalsheet = workbook_add_worksheet(workbook, "Totals");

    lxw_format* header_format = workbook_add_format(workbook);
    format_set_border(header_format, LXW_BORDER_MEDIUM);
    format_set_bold(header_format);

    lxw_format* format = workbook_add_format(workbook);
    format_set_border(format, LXW_BORDER_THIN);

    auto write_worksheet_string = [FACTOR](lxw_worksheet* sheet, int row, int col, std::string text, lxw_format* format, std::map<int, double>& col_widths) {
        worksheet_write_string(sheet, row, col, text.c_str(), format);
        col_widths[col] = std::max(text.size() + FACTOR, col_widths[col]);
    };

    RulerObject* ruler = RulerObject::GetRuler();

    std::vector<std::string> model_header_cols{ "Model Name", "Shadowing", "Description", "Display As", "Dimensions", "String Type", "String Count", "Node Count", "Light Count", "Est Current (Amps)", "Channels Per Node", "Channel Count", "Start Channel", "Start Channel No", "#Universe(or id):Start Channel", "End Channel No", "Default Buffer W x H", "Preview", "Controller Ports", "Connection Protocol", "Connection Attributes", "Controller Name", "Controller Type", "Protocol", "Controller Description", "IP", "Baud", "Universe/Id", "Universe Channel", "Controller Channel", "Active"};
    if (ruler != nullptr) {
        std::string unitDescription = ruler->GetUnitDescription();
        model_header_cols.push_back("Location X (" + unitDescription + ")");
        model_header_cols.push_back("Location Y (" + unitDescription + ")");
        model_header_cols.push_back("Location Z (" + unitDescription + ")");
    }
    model_header_cols.push_back("Aliases");

    std::map<int, double> _model_col_widths;
    for (int i = 0; i < (int)model_header_cols.size(); i++) {
        worksheet_write_string(modelsheet, 0, i, model_header_cols[i].c_str(), header_format);
        _model_col_widths[i] = model_header_cols[i].size() + FACTOR; // estimate column width
    }
    worksheet_freeze_panes(modelsheet, 1, 0);

    int modelCount = 0;
    int row = 1;
    // models
    for (auto const& m : AllModels) {
        Model* model = m.second;
        if (model->GetDisplayAs() != DisplayAsType::ModelGroup) {
            modelCount++;
            wxString const stch = model->GetModelStartChannel();
            uint32_t ch = model->GetFirstChannel() + 1;
            std::string type, description, ip, universe, inactive, baud, protocol, controllername;
            int32_t channeloffset;
            int stu = 0;
            int stuc = 0;
            GetControllerDetailsForChannel(ch, controllername, type, protocol, description, channeloffset, ip, universe, inactive, baud, stu, stuc);

            std::string current;

            wxString const stype = wxString(model->GetStringType());

            int32_t lightcount = (long)(model->GetNodeCount() * model->GetLightsPerNode());
            if (!stype.Contains("Node")) {
                if (model->GetNodeCount() == 1) {
                    lightcount = model->GetCoordCount(0);
                } else {
                    lightcount = model->NodesPerString() * model->GetLightsPerNode();
                }
            }

            if (stype.Contains("Node") || stype.Contains("Channel RGB")) {
                current = wxString::Format("%0.2f", (float)lightcount * AMPS_PER_PIXEL).ToStdString();
            }

            int w, h;
            model->GetBufferSize("Default", "2D", "None", w, h, 0);
            write_worksheet_string(modelsheet, row, 0, model->name, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 1, model->GetShadowModelFor(), format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 2, model->description, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 3, DisplayAsTypeToString(model->GetDisplayAs()), format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 4, model->GetDimension(), format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 5, model->GetStringType(), format, _model_col_widths);
            worksheet_write_number(modelsheet, row, 6, model->GetNumPhysicalStrings(), format);
            worksheet_write_number(modelsheet, row, 7, model->GetNodeCount(), format);
            worksheet_write_number(modelsheet, row, 8, lightcount, format);
            worksheet_write_number(modelsheet, row, 9, (float)lightcount * AMPS_PER_PIXEL, format);
            worksheet_write_number(modelsheet, row, 10, model->GetChanCountPerNode(), format);
            worksheet_write_number(modelsheet, row, 11, model->GetActChanCount(), format);
            write_worksheet_string(modelsheet, row, 12, stch, format, _model_col_widths);
            worksheet_write_number(modelsheet, row, 13, ch, format);
            write_worksheet_string(modelsheet, row, 14, wxString::Format("#%i:%i", stu, stuc), format, _model_col_widths);
            worksheet_write_number(modelsheet, row, 15, model->GetLastChannel() + 1, format);
            write_worksheet_string(modelsheet, row, 16, wxString::Format("%i x %i", w, h), format, _model_col_widths);

            write_worksheet_string(modelsheet, row, 17, model->GetLayoutGroup(), format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 18, model->GetControllerConnectionPortRangeString(), format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 19, model->GetControllerProtocol(), format, _model_col_widths);
            wxString con_attributes = model->GetControllerConnectionAttributeString();
            con_attributes.Replace(":", ",");
            if (con_attributes.StartsWith(",")) {
                con_attributes.Remove(0, 1);
            }
            write_worksheet_string(modelsheet, row, 20, con_attributes, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 21, controllername, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 22, type, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 23, protocol, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 24, description, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 25, ip, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 26, baud, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 27, universe, format, _model_col_widths);
            worksheet_write_number(modelsheet, row, 28, stuc, format);
            worksheet_write_number(modelsheet, row, 29, channeloffset, format);
            write_worksheet_string(modelsheet, row, 30, inactive, format, _model_col_widths);

            glm::vec3 position = model->GetBaseObjectScreenLocation().GetWorldPosition();
            if (ruler != nullptr) {
                worksheet_write_number(modelsheet, row, 31, ruler->Measure(position.x), format);
                worksheet_write_number(modelsheet, row, 32, ruler->Measure(position.y), format);
                worksheet_write_number(modelsheet, row, 33, ruler->Measure(position.z), format);
            }
            std::list<std::string> aliases = model->GetAliases();
            if (!aliases.empty()) {
                auto it = aliases.begin();
                std::string initial = *it;
                ++it;
                std::string separator = ", ";
                std::string a = (std::accumulate(it, aliases.end(), initial, [&separator](const std::string& a, const std::string& b) { return a + separator + b; }));
                write_worksheet_string(modelsheet, row, 34, a, format, _model_col_widths);
            };

            ++row;

            if (ch < minchannel) {
                minchannel = ch;
            }
            int32_t lastch = model->GetLastChannel() + 1;
            if (lastch > maxchannel) {
                maxchannel = lastch;
            }
        }
    }
    // set column widths
    for (auto const& [col, width] : _model_col_widths) {
        worksheet_set_column(modelsheet, col, col, width, NULL);
    }

    std::map<int, double> _group_col_widths;
    const std::vector<std::string> groupHeader{ "Group Name", "Models", "Models Count", "Default Buffer W x H", "Preview", "Aliases" };
    for (int i = 0; i < (int)groupHeader.size(); i++) {
        worksheet_write_string(groupsheet, 0, i, groupHeader[i].c_str(), header_format);
        _group_col_widths[i] = groupHeader[i].size() + FACTOR; // estimate column width
    }
    worksheet_freeze_panes(groupsheet, 1, 0);
    int groupCount = 0;
    row = 1;
    for (auto const& m : AllModels) {
        Model* model = m.second;
        if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
            groupCount++;
            ModelGroup* mg = static_cast<ModelGroup*>(model);
            std::string models;
            for (const auto& it : mg->ModelNames()) {
                if (models.empty()) {
                    models = it;
                } else {
                    models += ", " + it;
                }
            }
            int w, h;
            model->GetBufferSize("Default", "2D", "None", w, h, 0);

            write_worksheet_string(groupsheet, row, 0, model->name, format, _group_col_widths);
            write_worksheet_string(groupsheet, row, 1, models, format, _group_col_widths);
            worksheet_write_number(groupsheet, row, 2, mg->ModelNames().size(), format);
            write_worksheet_string(groupsheet, row, 3, wxString::Format("%d x %d", w, h), format, _group_col_widths);
            write_worksheet_string(groupsheet, row, 4, model->GetLayoutGroup(), format, _group_col_widths);
            std::list<std::string> aliases = model->GetAliases();
            if (!aliases.empty()) {
                auto it = aliases.begin();
                std::string initial = *it;
                ++it;
                std::string separator = ", ";
                std::string a = (std::accumulate(it, aliases.end(), initial,[&separator](const std::string& a, const std::string& b) { return a + separator + b; }));
                write_worksheet_string(groupsheet, row, 5, a, format, _group_col_widths);
            };
            ++row;
        }
    }
    for (auto const& [col, width] : _group_col_widths) {
        worksheet_set_column(groupsheet, col, col, width, NULL);
    }

    row = 1;
    std::map<int, double> _controller_col_widths;

    auto control_cols = OutputManager::GetExportHeaders();

    for (int i = 0; i < (int)control_cols.size(); i++) {
        worksheet_write_string(controllersheet, 0, i, control_cols[i].c_str(), header_format);
        _controller_col_widths[i] = control_cols[i].size() + FACTOR; // estimate column width
    }
    worksheet_freeze_panes(controllersheet, 1, 0);

    for (const auto& it : _outputManager.GetControllers()) {
        auto scolumns = it->GetExport();
        auto columns = wxSplit(scolumns, ',');
        for (int j = 0; j < (int)columns.size(); j++) {
            write_worksheet_string(controllersheet, row, j, columns[j], format, _controller_col_widths);
        }
        ++row;
        for (auto it2 : it->GetOutputs()) {
            auto s = it2->GetExport();
            if (!s.empty()) {
                auto scolumns2 = it2->GetExport();
                auto columns2 = wxSplit(scolumns2, ',');
                for (int k = 0; k < (int)columns2.size(); k++) {
                    write_worksheet_string(controllersheet, row, k, columns2[k], format, _controller_col_widths);
                }
                row++;
            }
        }
    }
    for (auto const& [col, width] : _controller_col_widths) {
        worksheet_set_column(controllersheet, col, col, width, NULL);
    }

    uint32_t bulbs = 0;
    uint32_t usedchannels = 0;
    if (minchannel == 99999999) {
        // No channels so we dont do this
        minchannel = 0;
        maxchannel = 0;
    } else {
        int* chused = (int*)malloc((maxchannel - minchannel + 1) * sizeof(int));
        memset(chused, 0x00, (maxchannel - minchannel + 1) * sizeof(int));

        for (auto const& m : AllModels) {
            Model* model = m.second;
            if (model->GetDisplayAs() != DisplayAsType::ModelGroup) {
                int ch = model->GetFirstChannel() + 1;
                int endch = model->GetLastChannel() + 1;

                int uniquechannels = 0;
                for (int i = ch; i <= endch; i++) {
                    wxASSERT(i - minchannel < maxchannel - minchannel + 1);
                    if (chused[i - minchannel] == 0) {
                        uniquechannels++;
                    }
                    chused[i - minchannel]++;
                }

                if (wxString(model->GetStringType()).StartsWith("Single Color")) {
                    bulbs += uniquechannels * model->GetCoordCount(0);
                } else if (wxString(model->GetStringType()).StartsWith("3 Channel")) {
                    bulbs += uniquechannels * model->GetNodeCount() / 3 * model->GetCoordCount(0);
                } else if (wxString(model->GetStringType()).StartsWith("4 Channel")) {
                    bulbs += uniquechannels * model->GetNodeCount() / 4 * model->GetCoordCount(0);
                } else if (wxString(model->GetStringType()).StartsWith("Strobes")) {
                    bulbs += uniquechannels * model->GetNodeCount() * model->GetCoordCount(0);
                } else if (model->GetStringType() == "Node Single Color") {
                    bulbs += uniquechannels * model->GetNodeCount() * model->GetCoordCount(0);
                } else {
                    int den = model->GetChanCountPerNode();
                    if (den == 0) {
                        den = 1;
                    }
                    bulbs += uniquechannels / den * model->GetLightsPerNode();
                }
            }
        }

        for (long i = 0; i < (long)(maxchannel - minchannel + 1); i++) {
            if (chused[i] > 0) {
                usedchannels++;
            }
        }

        free(chused);
    }

    worksheet_write_string(totalsheet, 0, 0, "Model Count", format);
    worksheet_write_number(totalsheet, 0, 1, modelCount, format);
    worksheet_write_string(totalsheet, 1, 0, "Group Count", format);
    worksheet_write_number(totalsheet, 1, 1, groupCount, format);
    worksheet_write_string(totalsheet, 2, 0, "First Used Channel", format);
    worksheet_write_number(totalsheet, 2, 1, minchannel, format);
    worksheet_write_string(totalsheet, 3, 0, "Last Used Channel", format);
    worksheet_write_number(totalsheet, 3, 1, maxchannel, format);
    worksheet_write_string(totalsheet, 4, 0, "Actual Used Channel", format);
    worksheet_write_number(totalsheet, 4, 1, usedchannels, format);
    worksheet_write_string(totalsheet, 5, 0, "Bulbs", format);
    worksheet_write_number(totalsheet, 5, 1, bulbs, format);

    worksheet_set_column(totalsheet, 0, 0, 25, NULL);

    lxw_error error = workbook_close(workbook);
    if (error) {
        DisplayError(wxString::Format("Unable to create Spreadsheet, Error %d = %s\n", error, lxw_strerror(error)).ToStdString());
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

