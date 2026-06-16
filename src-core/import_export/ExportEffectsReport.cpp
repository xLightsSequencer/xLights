/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// wx-free port of xLightsFrame::ExportEffects / ExportElement / ExportNodes
// (src-ui-wx/xLightsMain.cpp and src-ui-wx/import_export/ExportEffects.cpp).
// Produces the same CSV columns as the desktop export.

#include "ExportEffectsReport.h"

#include "render/SequenceElements.h"
#include "render/Element.h"
#include "render/EffectLayer.h"
#include "models/ModelManager.h"
#include "models/Model.h"
#include "models/DisplayAsType.h"
#include "effects/RenderableEffect.h"
#include "UtilClasses.h"

#include <spdlog/spdlog.h>

#include <cstdio>
#include <list>
#include <map>
#include <string>

namespace {

static std::string FormatMS(int ms) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%02d:%02d.%03d",
             ms / 60000,
             (ms % 60000) / 1000,
             ms % 1000);
    return buf;
}

static void WriteEffectRow(FILE* f, const std::string& effectName,
                           int startMS, int endMS,
                           const std::string& description,
                           const std::string& elementName,
                           const std::string& elementType,
                           const std::string& files,
                           std::map<std::string, int>& freq,
                           std::map<std::string, int>& totalTime) {
    int duration = endMS - startMS;

    auto it = freq.find(effectName);
    if (it != freq.end()) {
        it->second++;
        totalTime[effectName] += duration;
    } else {
        freq[effectName] = 1;
        totalTime[effectName] = duration;
    }

    fprintf(f, "\"%s\",%s,%s,%s,\"%s\",\"%s\",%s,%s\n",
            effectName.c_str(),
            FormatMS(startMS).c_str(),
            FormatMS(endMS).c_str(),
            FormatMS(duration).c_str(),
            description.c_str(),
            elementName.c_str(),
            elementType.c_str(),
            files.c_str());
}

static int ExportElementLayers(FILE* f, Element* e,
                                const std::string& elementName,
                                const std::string& elementType,
                                ModelManager& allModels,
                                EffectManager& effectManager,
                                std::map<std::string, int>& freq,
                                std::map<std::string, int>& totalTime,
                                std::list<std::string>& allFiles) {
    int effects = 0;
    Model* m = allModels.GetModel(e->GetModelName());

    for (size_t j = 0; j < e->GetEffectLayerCount(); j++) {
        EffectLayer* el = e->GetEffectLayer(j);
        for (int k = 0; k < el->GetEffectCount(); k++) {
            Effect* ef = el->GetEffect(k);

            std::string fs;
            if (ef->GetEffectIndex() >= 0) {
                RenderableEffect* eff = effectManager[ef->GetEffectIndex()];
                if (eff && m) {
                    auto files = eff->GetFileReferences(m, ef->GetSettings());
                    for (const auto& fp : files) {
                        if (!fs.empty()) fs += ",";
                        fs += fp;
                    }
                    allFiles.splice(allFiles.end(), files);
                }
            }

            const SettingsMap& sm = ef->GetSettings();
            std::string desc = sm.Get("X_Effect_Description", "");

            WriteEffectRow(f, ef->GetEffectName(),
                           ef->GetStartTimeMS(), ef->GetEndTimeMS(),
                           desc, elementName, elementType, fs,
                           freq, totalTime);
            effects++;
        }
    }
    return effects;
}

static int ExportNodes(FILE* f, StrandElement* se,
                       ModelManager& allModels,
                       EffectManager& effectManager,
                       std::map<std::string, int>& freq,
                       std::map<std::string, int>& totalTime,
                       std::list<std::string>& allFiles) {
    Model* m = allModels.GetModel(se->GetModelName());
    int effects = 0;

    for (int n = 0; n < se->GetNodeLayerCount(); n++) {
        NodeLayer* nl = se->GetNodeLayer(n);
        if (!nl) continue;

        std::string nodeName;
        if (m) {
            nodeName = se->GetFullName() + "/" + m->GetNodeName(n, true);
        } else {
            nodeName = se->GetFullName() + "/Node " + std::to_string(n + 1);
        }

        for (int k = 0; k < nl->GetEffectCount(); k++) {
            Effect* ef = nl->GetEffect(k);

            std::string fs;
            if (ef->GetEffectIndex() >= 0) {
                RenderableEffect* eff = effectManager[ef->GetEffectIndex()];
                if (eff && m) {
                    auto files = eff->GetFileReferences(m, ef->GetSettings());
                    for (const auto& fp : files) {
                        if (!fs.empty()) fs += ",";
                        fs += fp;
                    }
                    allFiles.splice(allFiles.end(), files);
                }
            }

            const SettingsMap& sm = ef->GetSettings();
            std::string desc = sm.Get("X_Effect_Description", "");

            WriteEffectRow(f, ef->GetEffectName(),
                           ef->GetStartTimeMS(), ef->GetEndTimeMS(),
                           desc, nodeName, "Node", fs,
                           freq, totalTime);
            effects++;
        }
    }
    return effects;
}

} // namespace

bool ExportEffects(const std::string& filename,
                   SequenceElements& elements,
                   ModelManager& allModels) {
    FILE* f = fopen(filename.c_str(), "w");
    if (!f) {
        spdlog::error("ExportEffects: unable to create file {}", filename);
        return false;
    }

    std::map<std::string, int> freq;
    std::map<std::string, int> totalTime;
    std::list<std::string> allFiles;
    int totalEffects = 0;

    EffectManager& effectManager = elements.GetEffectManager();

    fprintf(f, "Effect Name,StartTime,EndTime,Duration,Description,Element,ElementType,Files\n");

    for (size_t i = 0; i < elements.GetElementCount(MASTER_VIEW); i++) {
        Element* e = elements.GetElement(i, MASTER_VIEW);
        if (!e || e->GetType() == ElementType::ELEMENT_TYPE_TIMING) continue;

        std::string typeName;
        switch (e->GetType()) {
        case ElementType::ELEMENT_TYPE_MODEL: {
            Model* m = allModels.GetModel(e->GetModelName());
            if (m && m->GetDisplayAs() == DisplayAsType::ModelGroup) {
                typeName = "Model Group";
            } else {
                typeName = "Model";
            }
            break;
        }
        case ElementType::ELEMENT_TYPE_SUBMODEL:
            typeName = "Submodel";
            break;
        case ElementType::ELEMENT_TYPE_STRAND:
            typeName = "Strand";
            break;
        default:
            typeName = "Unknown";
            break;
        }

        totalEffects += ExportElementLayers(f, e, e->GetFullName(), typeName,
                                            allModels, effectManager,
                                            freq, totalTime, allFiles);

        if (e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            ModelElement* me = static_cast<ModelElement*>(e);
            for (int s = 0; s < me->GetSubModelAndStrandCount(); s++) {
                SubModelElement* se = me->GetSubModel(s);
                if (!se) continue;
                std::string seType = (se->GetType() == ElementType::ELEMENT_TYPE_STRAND)
                                     ? "Strand" : "Submodel";
                totalEffects += ExportElementLayers(f, se, se->GetFullName(), seType,
                                                    allModels, effectManager,
                                                    freq, totalTime, allFiles);
            }
            for (int s = 0; s < me->GetStrandCount(); s++) {
                StrandElement* strand = me->GetStrand(s);
                if (!strand) continue;
                totalEffects += ExportNodes(f, strand, allModels, effectManager,
                                            freq, totalTime, allFiles);
            }
        }
    }

    fprintf(f, "\"Effect Count\",%d\n", totalEffects);
    fprintf(f, "\n");
    fprintf(f, "Effect Usage Summary\n");
    fprintf(f, "Effect Name,Occurences,TotalTime\n");
    for (const auto& [name, count] : freq) {
        int tt = totalTime.at(name);
        fprintf(f, "\"%s\",%d,%s\n",
                name.c_str(), count, FormatMS(tt).c_str());
    }
    fprintf(f, "\n");
    fprintf(f, "Summary of files used\n");

    allFiles.sort();
    allFiles.unique();
    for (const auto& fp : allFiles) {
        fprintf(f, "%s\n", fp.c_str());
    }

    fclose(f);
    return true;
}
