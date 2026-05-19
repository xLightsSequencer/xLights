/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Import/export effect functions extracted from SeqFileUtilities.cpp

#include <wx/stopwatch.h>
#include "settings/XLightsConfigAdapter.h"
#include <wx/regex.h>
#include <wx/tokenzr.h>
#include <wx/uri.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/textdlg.h>

#include <pugixml.hpp>

#include "../import_export/LMSImportChannelMapDialog.h"
#include "LOREdit.h"
#include "../import_export/SuperStarImportDialog.h"
#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"
#include "VSAFile.h"
#include "../import_export/VsaImportDialog.h"
#include "effects/BufferStyles.h"
#include "import_export/EffectMapper.h"
#include "import_export/SuperStarImporter.h"
#include "sequencer/BufferPanel.h"
#include "FileConverter.h"
#include "../import_export/xLightsImportChannelMapDialog.h"
#include "../xLightsMain.h"
#include "../xLightsApp.h"
#include "xLightsVersion.h"
#include "render/SequenceMedia.h"
#include "utils/ExternalHooks.h"
#include "render/SequencePackage.h"
#include "Vixen3.h"
#include "effects/BarsEffect.h"
#include "effects/ButterflyEffect.h"
#include "effects/CurtainEffect.h"
#include "effects/FireEffect.h"
#include "effects/GarlandsEffect.h"
#include "effects/MeteorsEffect.h"
#include "effects/PinwheelEffect.h"
#include "effects/SnowflakesEffect.h"
#include "effects/SpiralsEffect.h"
#include "render/FontManager.h"
#include "models/DMX/DmxModel.h"
#include "models/ModelGroup.h"
#include "../sequencer/MainSequencer.h"
#include "render/RenderUtils.h"

#include <log.h>

static xlColor GetColor(const std::string& rgb)
{
    int i = wxAtoi(rgb);
    xlColor cl;
    cl.red = (i & 0xff);
    cl.green = ((i >> 8) & 0xFF);
    cl.blue = ((i >> 16) & 0xff);
    return cl;
}

void xLightsFrame::OnMenuItemImportEffects(wxCommandEvent& event)
{
    wxArrayString filters;
    filters.push_back("All|*.xsq;*.sup;*.lms;*.lpe;*.las;*.loredit;*.xml;*.hlsdata;*.vix;*.tim;*.msq;*.vsa;*.zip;*.piz;*.xsqz");
    filters.push_back("SuperStar File (*.sup)|*.sup");
    filters.push_back("LOR Music Sequences (*.lms)|*.lms");
    filters.push_back("LOR Pixel Editor Sequences (*.lpe)|*.lpe");
    filters.push_back("LOR Animation Sequences (*.las)|*.las");
    filters.push_back("LOR S5(*.loredit)|*.loredit");
    filters.push_back("xLights Sequence Package (*.zip;*.piz;*.xsqz)|*.zip;*.piz;*.xsqz");
    filters.push_back("xLights Sequence (*.xsq)|*.xsq");
    filters.push_back("Old xLights Sequence (*.xml)|*.xml");
    filters.push_back("HLS hlsIdata Sequences(*.hlsIdata)|*.hlsIdata");
    filters.push_back("Vixen 2.x Sequence(*.vix)|*.vix");
    filters.push_back("Vixen 3.x Sequence(*.tim)|*.tim");
    filters.push_back("LSP 2.x Sequence(*.msq)|*.msq");
    filters.push_back("VSA Files(*.vsa)|*.vsa");

    wxString filter;
    for (auto it = filters.begin(); it != filters.end(); ++it) {
        if (filter != "") {
            filter += "|";
        }
        filter += *it;
    }

    wxString lit = "";
    wxString ldir = "";
    auto* config = GetXLightsConfig();
    if (config != nullptr) {
        config->Read("xLightsLastImportType", &lit, "");
        config->Read("xLightsLastImportDir", &ldir, "");
    }

    wxFileDialog file(this, "Choose file to import", ldir, "", filter);
    if (lit != "") {
        int index = 0;

        for (auto it = filters.begin(); it != filters.end(); ++it) {
            if (lit == *it) {
                file.SetFilterIndex(index);
                break;
            }
            index++;
        }
    }

    if (file.ShowModal() == wxID_OK) {
        if (config != nullptr && file.GetFilterIndex() >= 0 && file.GetFilterIndex() < (int)filters.size()) {
            config->Write("xLightsLastImportType", filters[file.GetFilterIndex()]);
        } else {
            spdlog::warn("XLightsLastImportType not saved due to invalid filter index {}.", file.GetFilterIndex());
        }
        if (config != nullptr) {
            ldir = file.GetDirectory();
            config->Write("xLightsLastImportDir", ldir);
        }

        wxFileName fn = file.GetPath();
        if (!FileExists(fn)) {
            return;
        }
        wxString ext = fn.GetExt().Lower();
        if (ext == "lms" || ext == "las") {
            ImportLMS(fn);
        } else if (ext == "lpe") {
            ImportLPE(fn);
        } else if (ext == "loredit") {
            ImportS5(fn);
        } else if (ext == "hlsidata") {
            ImportHLS(fn);
        } else if (ext == "sup") {
            ImportSuperStar(fn);
        } else if (ext == "tim") {
            ImportVixen3(fn);
        } else if (ext == "vix") {
            ImportVix(fn);
        } else if (ext == "xml" || ext == "xsq" || ext == "zip" || ext == "xsqz" || ext == "piz") {
            ImportXLights(fn);
        } else if (ext == "msq") {
            ImportLSP(fn);
        } else if (ext == "vsa") {
            ImportVsa(fn);
        }
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(this, eventRowHeaderChanged);
        mainSequencer->PanelEffectGrid->Refresh();
    }
}

void xLightsFrame::ImportXLights(const wxFileName& filename, std::string const& mapFile)
{
    SequencePackage xsqPkg(std::filesystem::path(ToStdString(filename.GetFullPath())),
                           GetShowDirectory(), ToStdString(GetSeqXmlFileName()), &AllModels);

    if (xsqPkg.IsPkg()) {
        xsqPkg.Extract();
    } else {
        xsqPkg.FindRGBEffectsFile();
    }

    if (!xsqPkg.IsValid() && xsqPkg.IsPkg()) {
        wxMessageBox("The file you are attempting to import doesn't appear to be a valid xLights Sequence Package.", "Invalid Sequence Package", wxICON_ERROR | wxOK);
        return;
    }

    SequenceFile xlf(xsqPkg.GetXsqFile().string());
    auto importDoc = xlf.Open(GetShowDirectory(), true, xsqPkg.GetXsqFile().string());
    if (!importDoc) return;
    SequenceElements se(this);
    se.SetFrequency(_sequenceElements.GetFrequency());
    se.SetViewsManager(GetViewsManager()); // This must come first before LoadSequencerFile.
    se.LoadSequencerFile(xlf, *importDoc, GetShowDirectory(), true);
    xlf.AdjustEffectSettingsForVersion(se, this);
    xsqPkg.SetSequenceElements(&se);

    if (IsVersionOlder(xlf.GetVersion(), xlights_version_string)) {
        wxMessageBox(wxString::Format("Import version %s is newer than your current version %s.", xlf.GetVersion().c_str(), xlights_version_string.c_str()), "Version Warning");
    }
    if (_sequenceElements.GetFrequency() < xlf.GetFrequency()) {
        wxMessageBox(wxString::Format("The import sequence is using a higher FPS than you are currently using. %d FPS", xlf.GetFrequency()));
    }
    bool supportsModelBlending = xlf.supportsModelBlending();

    std::vector<Element*> elements;
    for (size_t e = 0; e < se.GetElementCount(); ++e) {
        Element* el = se.GetElement(e);
        elements.push_back(el);
    }
    ImportXLights(se, elements, xsqPkg, supportsModelBlending, true, false, false, mapFile, xlf.GetSequenceDurationMS());

    SetStatusText(wxString::Format("'%s' imported.", filename.GetPath()));
}

ModelElement* AddModel(Model* m, SequenceElements& se)
{
    if (m != nullptr) {
        se.AddMissingModelsToSequence(m->GetName(), true);
        ModelElement* model = dynamic_cast<ModelElement*>(se.GetElement(m->GetName()));
        model->Init(*m);
        return model;
    }
    return nullptr;
}

// backwards compatible for tabSequencer call
void xLightsFrame::ImportXLights(SequenceElements& se, const std::vector<Element*>& elements, const wxFileName& filename,
                                 bool modelBlending, bool showModelBlending, bool allowAllModels, bool clearSrc)
{
    SequencePackage xsqPkg(std::filesystem::path(ToStdString(filename.GetFullPath())),
                           GetShowDirectory(), ToStdString(GetSeqXmlFileName()), &AllModels);
    ImportXLights(se, elements, xsqPkg, modelBlending, showModelBlending, allowAllModels, clearSrc);
}

static std::vector<std::pair<int,int>> BuildMergedIntervals(Element* el)
{
    std::vector<std::pair<int,int>> v;
    for (size_t l = 0; l < el->GetEffectLayerCount(); ++l) {
        EffectLayer* layer = el->GetEffectLayer(l);
        for (int e = 0; e < layer->GetEffectCount(); ++e) {
            Effect* eff = layer->GetEffect(e);
            v.emplace_back(eff->GetStartTimeMS(), eff->GetEndTimeMS());
        }
    }
    if (v.empty()) return {};
    std::sort(v.begin(), v.end());
    std::vector<std::pair<int,int>> out;
    out.push_back(v[0]);
    for (size_t i = 1; i < v.size(); ++i) {
        if (v[i].first <= out.back().second)
            out.back().second = std::max(out.back().second, v[i].second);
        else
            out.push_back(v[i]);
    }
    return out;
}

void xLightsFrame::ImportXLights(SequenceElements& se, const std::vector<Element*>& elements, SequencePackage& xsqPkg,
                                 bool modelBlending, bool showModelBlending, bool allowAllModels, bool clearSrc, std::string const& mapFile, int sequenceDurationMS)
{
    std::map<std::string, EffectLayer*> layerMap;
    std::map<std::string, Element*> elementMap;
    xLightsImportChannelMapDialog dlg(this, wxFileName(xsqPkg.GetXsqFile().string()), false, true, false, false, showModelBlending);
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;
    if (showModelBlending) {
        dlg.SetModelBlending(modelBlending);
    }

    dlg.SetXsqPkg(&xsqPkg);
    if (sequenceDurationMS > 0) {
        dlg.SetSequenceDuration(sequenceDurationMS);
    }

    std::vector<EffectLayer*> mapped;
    std::vector<std::string> timingTrackNames;
    std::map<std::string, bool> timingTrackAlreadyExists;
    std::map<std::string, TimingElement*> timingTracks;

    for (const auto& it : elements) {
        Element* e = it;
        if (e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            ModelElement* el = dynamic_cast<ModelElement*>(e);
            bool hasEffects{ false };

            for (size_t l = 0; l < el->GetEffectLayerCount(); ++l) {
                hasEffects |= el->GetEffectLayer(l)->GetEffectCount() > 0;
            }
            if (hasEffects) {
                dlg.AddChannel(el->GetName(), el->GetModelEffectCount(), false,
                               sequenceDurationMS > 0 ? BuildMergedIntervals(el) : std::vector<std::pair<int,int>>{});
            }
            elementMap[el->GetName()] = el;
            int s = 0;
            for (int sm = 0; sm < el->GetSubModelAndStrandCount(); ++sm) {
                SubModelElement* sme = el->GetSubModel(sm);

                StrandElement* ste = dynamic_cast<StrandElement*>(sme);
                std::string smName = sme->GetName();
                if (ste != nullptr) {
                    ++s;
                    if (smName == "") {
                        smName = wxString::Format("Strand %d", s);
                    }
                }
                if (sme->HasEffects()) {
                    elementMap[el->GetName() + "/" + smName] = sme;
                    dlg.AddChannel(el->GetName() + "/" + smName, sme->GetEffectCount(), false,
                                   sequenceDurationMS > 0 ? BuildMergedIntervals(sme) : std::vector<std::pair<int,int>>{});
                }
                if (ste != nullptr) {
                    for (int n = 0; n < ste->GetNodeLayerCount(); ++n) {
                        NodeLayer* nl = ste->GetNodeLayer(n, true);
                        if (nl->GetEffectCount() > 0) {
                            std::string nodeName = nl->GetNodeName();
                            if (nodeName == "") {
                                nodeName = wxString::Format("Node %d", (int)(n + 1));
                            }
                            std::vector<std::pair<int,int>> nodeIntervals;
                            if (sequenceDurationMS > 0) {
                                for (int e = 0; e < nl->GetEffectCount(); ++e) {
                                    Effect* eff = nl->GetEffect(e);
                                    nodeIntervals.emplace_back(eff->GetStartTimeMS(), eff->GetEndTimeMS());
                                }
                            }
                            dlg.AddChannel(el->GetName() + "/" + smName + "/" + nodeName, nl->GetEffectCount(), true,
                                           std::move(nodeIntervals));
                            layerMap[el->GetName() + "/" + smName + "/" + nodeName] = nl;
                        }
                    }
                }
            }
        } else if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            TimingElement* tel = dynamic_cast<TimingElement*>(e);
            bool hasEffects{ false };
            for (int n = 0; n < (int)tel->GetEffectLayerCount(); ++n) {
                hasEffects |= tel->GetEffectLayer(n)->GetEffectCount() > 0;
            }
            if (hasEffects) {
                timingTrackNames.push_back(tel->GetName());
                timingTracks[tel->GetName()] = tel;

                // we want to know which timing tracks exist so we can preselect the ones which are not already present
                // a timing track is only considered to exist if it has at least one timing mark
                timingTrackAlreadyExists[tel->GetName()] = (_sequenceElements.GetTimingElement(tel->GetName()) != nullptr && _sequenceElements.GetTimingElement(tel->GetName())->HasEffects());
            }
        }
    }

    dlg.SortChannels();
    dlg.timingTracks = timingTrackNames;
    dlg.timingTrackAlreadyExists = timingTrackAlreadyExists;
    bool ok = dlg.InitImport();

    if (mapFile.empty()) {
        if (!ok || dlg.ShowModal() != wxID_OK) {
            return;
        }
    } else {
        dlg.LoadMappingFile(mapFile, true);
    }

    if (showModelBlending && dlg.GetImportModelBlending()) {
        CurrentSeqXmlFile->setSupportsModelBlending(modelBlending);
        GetSequenceElements().SetSupportsModelBlending(modelBlending);
    }

    bool lock = dlg.IsLockEffects();
    bool convertRender = dlg.IsConvertRender();

    // if the user is importing at least one timing element and the current sequence only has one timing track called New Timing with no timing marks in it ...
    if (dlg.TimingTrackListBox->GetCount() > 0 && _sequenceElements.GetNumberOfTimingElements() == 1 && _sequenceElements.GetTimingElement("New Timing") != nullptr && !_sequenceElements.GetTimingElement("New Timing")->HasEffects()) {
        // Delete the New Timing timing track
        _sequenceElements.DeleteElement("New Timing");
    }

    // build a mapping table so we can use it to fix up any 'Duplicate' Effects
    std::map<std::string, std::string> mapping;
    std::map<std::string, std::string> mappingModelType;
    for (size_t i = 0; i < dlg._dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = dlg._dataModel->GetNthChild(i);
        if (m->_mapping != "") {
            mapping[m->_mapping] = m->_model;
            mappingModelType[m->_mapping] = m->_mappingModelType;
        }
    }

    for (size_t tt = 0; tt < dlg.TimingTrackListBox->GetCount(); ++tt) {
        if (dlg.TimingTrackListBox->IsChecked(tt)) {
            TimingElement* tel = timingTracks[timingTrackNames[tt]];
            TimingElement* target = nullptr;
            if (_sequenceElements.GetTimingElement(tel->GetName()) != nullptr) {
                if (_sequenceElements.GetTimingElement(tel->GetName())->GetEffectLayer(0)->GetEffectCount() == 0) {
                    target = _sequenceElements.GetTimingElement(tel->GetName());
                }
            }

            if (target == nullptr) {
                target = static_cast<TimingElement*>(_sequenceElements.AddElement(tel->GetName(), "timing", true, tel->GetCollapsed(), tel->GetActive(), false, false));
                char cnt = '1';
                while (target == nullptr) {
                    target = static_cast<TimingElement*>(_sequenceElements.AddElement(tel->GetName() + "-" + cnt++, "timing", true, tel->GetCollapsed(), tel->GetActive(), false, false));
                }
            }

            for (int l = 0; l < (int)tel->GetEffectLayerCount(); ++l) {
                EffectLayer* src = tel->GetEffectLayer(l);
                while (l >= (int)target->GetEffectLayerCount()) {
                    target->AddEffectLayer();
                }
                EffectLayer* dst = target->GetEffectLayer(l);
                std::vector<EffectLayer*> mapped2;
                MapXLightsEffects(dst, src, mapped2, dlg.CheckBox_EraseExistingEffects->GetValue(), xsqPkg, lock, mapping, convertRender, mappingModelType);
            }
        }
    }

    for (size_t i = 0; i < dlg._dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = dlg._dataModel->GetNthChild(i);
        std::string modelName = m->_model;
        ModelElement* model = nullptr;
        for (size_t x = 0; x < _sequenceElements.GetElementCount(); ++x) {
            if (_sequenceElements.GetElement(x)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(x)->GetName()) {
                model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(x));
                break;
            }
        }
        if (m->_mapping != "") {
            if (model == nullptr) {
                model = AddModel(GetModel(modelName), _sequenceElements);
            }
            if (model == nullptr) {
                spdlog::error("Attempt to add model {} during xLights import failed.", (const char*)modelName.c_str());
            } else {
                MapXLightsEffects(model, m->_mapping, se, elementMap, layerMap, mapped, dlg.CheckBox_EraseExistingEffects->GetValue(), xsqPkg, lock, mapping, convertRender, mappingModelType);
            }
        }

        int str = 0;
        for (size_t j = 0; j < m->GetChildCount(); j++) {
            xLightsImportModelNode* s = m->GetNthChild(j);

            if ("" != s->_mapping) {
                if (model == nullptr) {
                    model = AddModel(GetModel(modelName), _sequenceElements);
                }
                if (model == nullptr) {
                    spdlog::error("Attempt to add model {} during xLights import failed.", (const char*)modelName.c_str());
                } else {
                    SubModelElement* ste = model->GetSubModel(str);
                    if (ste != nullptr) {
                        MapXLightsEffects(ste, s->_mapping, se, elementMap, layerMap, mapped, dlg.CheckBox_EraseExistingEffects->GetValue(), xsqPkg, lock, mapping, convertRender, mappingModelType);
                    }
                }
            }
            for (size_t n = 0; n < s->GetChildCount(); n++) {
                xLightsImportModelNode* ns = s->GetNthChild(n);
                if ("" != ns->_mapping) {
                    if (model == nullptr) {
                        model = AddModel(GetModel(modelName), _sequenceElements);
                    }
                    if (model == nullptr) {
                        spdlog::error("Attempt to add model {} during xLights import failed.", (const char*)modelName.c_str());
                    } else {
                        SubModelElement* ste = model->GetSubModel(str);
                        StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                        if (stre != nullptr) {
                            NodeLayer* nl = stre->GetNodeLayer(n, true);
                            if (nl != nullptr) {
                                MapXLightsStrandEffects(nl, ns->_mapping, layerMap, se, mapped, dlg.CheckBox_EraseExistingEffects->GetValue(), xsqPkg, lock, mapping, mappingModelType);
                            }
                        }
                    }
                }
            }
            str++;
        }
    }

    if (clearSrc) {
        for (const auto& it : mapped) {
            it->RemoveAllEffects(nullptr);
        }
    }

    if (xsqPkg.IsPkg()) {
        if (xsqPkg.HasMissingMedia()) {
            wxString missingAssets;
            for (const auto& missingAsset : xsqPkg.GetMissingMedia()) {
                missingAssets = missingAssets + wxString::Format("%s- %s\n", "    ", missingAsset);
            }

            wxString msgP1 = "The following assets were missing from the Sequence Package and could not be imported.";
            wxString msgP2 = "Once you source them, place them in your show folder and use 'Import Effects' again making sure to select 'Erase existing effects on imported models'";
            wxString msgP3 = "or update the effects individually.";

            wxMessageBox(wxString::Format("%s %s %s\n\n%s", msgP1, msgP2, msgP3, missingAssets), "Missing Assets", wxICON_WARNING | wxOK, this);
        }

        if (xsqPkg.ModelsChanged()) {
            GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "xLightsFrame::ImportXLights");
        }
    }
}

void MapToStrandName(const std::string& name, std::vector<std::string>& strands)
{
    if (name.find("_") != name.npos) {
        int idx = name.find("_") + 1;
        // maybe map to a strand?  Name_0001, Name_0002... etc...

        int ppos = -1;
        int spos = -1;
        for (int x = idx; x < (int)name.size(); x++) {
            if (name[x] == 'P') {
                ppos = x;
            } else if (name[x] == 'S') {
                spos = x;
            } else if (name[x] < '0' || name[x] > '9') {
                return;
            }
        }
        std::string strandName;
        if (spos == -1 && ppos == -1) {
            // simple "strand" names of _#####
            strandName = name.substr(0, name.find("_") - 1);
        } else if (spos >= 0 && ppos > spos) {
            // more complex of _S###P###
            strandName = name.substr(0, ppos - 1);
        }
        if ("" != strandName && std::find(strands.begin(), strands.end(), strandName) == strands.end()) {
            strands.push_back(strandName);
        }
    }
}

void ReadHLSData(pugi::xml_node chand, std::vector<unsigned char>& data)
{
    for (pugi::xml_node chani = chand.first_child(); chani; chani = chani.next_sibling()) {
        if (std::string_view(chani.name()) == "IlluminationData") {
            for (pugi::xml_node block = chani.first_child(); block; block = block.next_sibling()) {
                wxString vals = block.text().get();
                int offset = wxAtoi(vals.SubString(0, vals.Find("-")));
                vals = vals.SubString(vals.Find("-") + 1, vals.size());
                while (!vals.IsEmpty()) {
                    wxString v = vals.BeforeFirst(',');
                    vals = vals.AfterFirst(',');
                    long iv = 0;
                    v.ToLong(&iv, 16);
                    data[offset] = iv;
                    offset++;
                }
            }
        }
    }
}

void MapHLSChannelInformation(xLightsFrame* xlights, EffectLayer* layer, pugi::xml_node tuniv, int frames, int frameTime,
                              const wxString& cn, wxColor color, Model& mc, bool byStrand, bool eraseExisting)
{
    if (cn == "") {
        return;
    }
    static 
    pugi::xml_node redNode;
    pugi::xml_node greenNode;
    pugi::xml_node blueNode;

    for (pugi::xml_node univ = tuniv.first_child(); univ; univ = univ.next_sibling()) {
        if (std::string_view(univ.name()) == "Universe") {
            for (pugi::xml_node channels = univ.first_child(); channels; channels = channels.next_sibling()) {
                if (std::string_view(channels.name()) == "Channels") {
                    for (pugi::xml_node chand = channels.first_child(); chand; chand = chand.next_sibling()) {
                        if (std::string_view(chand.name()) == "ChannelData") {
                            for (pugi::xml_node chani = chand.first_child(); chani; chani = chani.next_sibling()) {
                                if (std::string_view(chani.name()) == "ChanInfo") {
                                    wxString info = chani.text().get();
                                    if (info == cn + ", Normal") {
                                        // single channel, easy
                                        redNode = chand;
                                    } else if (info == cn + ", RGB-R") {
                                        redNode = chand;
                                    } else if (info == cn + ", RGB-G") {
                                        greenNode = chand;
                                    } else if (info == cn + ", RGB-B") {
                                        blueNode = chand;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (!redNode) {
        printf("Did not map %s\n", (const char*)cn.c_str());
        spdlog::info("Did not map {}", cn.ToStdString());
        return;
    }
    std::vector<unsigned char> redData(frames);
    std::vector<unsigned char> greenData(frames);
    std::vector<unsigned char> blueData(frames);
    std::vector<xlColor> colors(frames);
    ReadHLSData(redNode, redData);
    if (greenNode) {
        ReadHLSData(greenNode, greenData);
        ReadHLSData(blueNode, blueData);
        for (int x = 0; x < frames; x++) {
            colors[x].Set(redData[x], greenData[x], blueData[x]);
        }
    } else {
        xlColor c(color.Red(), color.Green(), color.Blue());
        HSVValue hsv = c.asHSV();
        for (int x = 0; x < frames; x++) {
            int i = redData[x];
            // for ramps up/down, HLS does a 1%-100% so the first cell is not linear and
            // we end up not able to map the ramps, we'll try and detect that here
            // and change to 0
            if (i <= 3 && i > 0) {
                if (x < (frames - 4)) {
                    if (i < redData[x + 1] && redData[x + 1] < redData[x + 2] && redData[x + 2] < redData[x + 3]) {
                        i = 0;
                    }
                }
                if (x > 4) {
                    if (i < redData[x - 1] && redData[x - 1] < redData[x - 2] && redData[x - 2] < redData[x - 3]) {
                        i = 0;
                    }
                }
            }
            hsv.value = ((double)i) / 255.0;
            colors[x] = hsv;
        }
    }
    xlights->DoConvertDataRowToEffects(layer, colors, frameTime, eraseExisting);
}

std::string FindHLSStrandName(const std::string& ccrName, int node, const std::vector<std::string>& channelNames)
{
    std::string r = ccrName + fmt::format("P{:03d}", node);
    if (std::find(channelNames.begin(), channelNames.end(), r) == channelNames.end()) {
        r = ccrName + fmt::format("P{:04d}", node);
    } else {
        return r;
    }
    if (std::find(channelNames.begin(), channelNames.end(), r) == channelNames.end()) {
        r = ccrName + fmt::format("P{:02d}", node);
    } else {
        return r;
    }
    if (std::find(channelNames.begin(), channelNames.end(), r) == channelNames.end()) {
        r = ccrName + fmt::format("_{:04d}", node);
    } else {
        return r;
    }
    if (std::find(channelNames.begin(), channelNames.end(), r) == channelNames.end()) {
        r = ccrName + fmt::format("_{:03d}", node);
    } else {
        return r;
    }
    if (std::find(channelNames.begin(), channelNames.end(), r) == channelNames.end()) {
        return r;
    }
    return "";
}

bool Contains(const std::vector<std::string>& array, const std::string& str)
{
    return std::find(array.begin(), array.end(), str) != array.end();
}

int Index(const std::vector<std::string>& array, const std::string& str)
{
    auto it = std::find(array.begin(), array.end(), str);
    if (it == array.end()) {
        return -1;
    }
    return it - array.begin();
}
void MapVixChannelInformation(xLightsFrame* xlights, EffectLayer* layer,
                              std::vector<unsigned char>& data,
                              int frameTime,
                              int numFrames,
                              const std::string& channelName,
                              const std::vector<std::string>& channels,
                              wxColor color,
                              Model& mc, bool eraseExisting)
{
    if (channelName == "") {
        return;
    }
    int channel = Index(channels, channelName);
    xlColorVector colors(numFrames);
    if (channel == wxNOT_FOUND) {
        int rchannel = Index(channels, channelName + "Red");
        if (rchannel == wxNOT_FOUND) {
            rchannel = Index(channels, channelName + "-R");
        }
        int gchannel = Index(channels, channelName + "Green");
        if (gchannel == wxNOT_FOUND) {
            gchannel = Index(channels, channelName + "-G");
        }
        int bchannel = Index(channels, channelName + "Blue");
        if (bchannel == wxNOT_FOUND) {
            bchannel = Index(channels, channelName + "-B");
        }
        if (rchannel == wxNOT_FOUND || gchannel == wxNOT_FOUND || bchannel == wxNOT_FOUND) {
            return;
        }
        for (int x = 0; x < numFrames; x++) {
            colors[x].Set(data[x + numFrames * rchannel], data[x + numFrames * gchannel], data[x + numFrames * bchannel]);
        }
    } else {
        xlColor c(color.Red(), color.Green(), color.Blue());
        HSVValue hsv = c.asHSV();
        for (int x = 0; x < numFrames; x++) {
            hsv.value = ((double)data[x + numFrames * channel]) / 255.0;
            colors[x] = hsv;
        }
    }
    xlights->DoConvertDataRowToEffects(layer, colors, frameTime, eraseExisting);
}

static void CheckForVixenRGB(const std::string& name, xlColor& c, xLightsImportChannelMapDialog& dlg)
{
    auto const channelNames{ dlg.GetChannelNames() };
    bool addRGB = false;
    std::string base;
    if (EndsWith(name, "Red") || EndsWith(name, "-R")) {
        c = xlRED;
        if (EndsWith(name, "-R")) {
            base = name.substr(0, name.size() - 2);
        } else {
            base = name.substr(0, name.size() - 3);
        }
        if ((Contains(channelNames, base + "Blue") && Contains(channelNames, base + "Green")) || (Contains(channelNames, base + "-B") && Contains(channelNames, base + "-G"))) {
            addRGB = true;
        }
    } else if (EndsWith(name, "Blue") || EndsWith(name, "-B")) {
        c = xlBLUE;
        if (EndsWith(name, "-B")) {
            base = name.substr(0, name.size() - 2);
        } else {
            base = name.substr(0, name.size() - 4);
        }
        if ((Contains(channelNames, base + "Red") && Contains(channelNames, base + "Green")) || (Contains(channelNames, base + "-R") && Contains(channelNames, base + "-G"))) {
            addRGB = true;
        }
    } else if (EndsWith(name, "Green") || EndsWith(name, "-G")) {
        c = xlGREEN;
        if (EndsWith(name, "-G")) {
            base = name.substr(0, name.size() - 2);
        } else {
            base = name.substr(0, name.size() - 5);
        }
        if ((Contains(channelNames, base + "Blue") && Contains(channelNames, base + "Red")) || (Contains(channelNames, base + "-B") && Contains(channelNames, base + "-R"))) {
            addRGB = true;
        }
    }
    dlg.channelColors[name] = c;
    if (addRGB) {
        dlg.channelColors[base] = xlBLACK;
        dlg.AddChannel(base);
    }
}

void xLightsFrame::ImportVix(const wxFileName& filename)
{
    spdlog::debug("Importing vixen file {}.", ToStdString(filename.GetFullName()));

    std::vector<unsigned char> VixSeqData;

    int time = 0;
    int frameTime = 50;

    xLightsImportChannelMapDialog dlg(this, filename, false, false, true, true, false);
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;

    std::vector<std::string> unsortedChannels;

    // pass 1, read the length, determine number of networks, units/network, channels per unit
    spdlog::debug("Reading vixen file.");

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filename.GetFullPath().mb_str());
    if (!result) {
        spdlog::error("Failed to parse Vixen file: {}", result.description());
        return;
    }

    pugi::xml_node root = doc.document_element();

    // Read EventPeriodInMilliseconds
    pugi::xml_node periodNode = root.child("EventPeriodInMilliseconds");
    if (periodNode) {
        frameTime = wxAtoi(periodNode.child_value());
    }

    // Read Time
    pugi::xml_node timeNode = root.child("Time");
    if (timeNode) {
        time = wxAtoi(timeNode.child_value());
    }

    // Read Profile
    pugi::xml_node profileNode = root.child("Profile");
    if (profileNode) {
        std::string NodeValue = profileNode.child_value();
        wxArrayInt VixChannels;
        wxArrayString VixChannelNames;
        SequenceData seqData;
        ConvertParameters params(filename.GetFullPath(),
                                 seqData,
                                 nullptr,
                                 ConvertParameters::ReadMode::READ_MODE_NORMAL,
                                 this,
                                 nullptr,
                                 nullptr);

        std::vector<xlColor> colors;
        FileConverter::LoadVixenProfile(params, NodeValue, VixChannels, VixChannelNames, colors);
        for (int x = 0; x < (int)VixChannelNames.size(); x++) {
            std::string name = VixChannelNames[x].ToStdString();
            xlColor c = colors[x];
            dlg.AddChannel(name);
            unsortedChannels.push_back(name);

            CheckForVixenRGB(name, c, dlg);
        }
    }

    // Read Channels
    pugi::xml_node channelsNode = root.child("Channels");
    if (channelsNode) {
        for (pugi::xml_node channel : channelsNode.children("Channel")) {
            int chanColor = wxAtoi(channel.attribute("color").value()) & 0xFFFFFF;
            std::string nameAttr = channel.attribute("name").value();
            if (!nameAttr.empty()) {
                dlg.AddChannel(nameAttr);
                unsortedChannels.push_back(nameAttr);

                xlColor c(chanColor, false);
                CheckForVixenRGB(nameAttr, c, dlg);
            } else {
                std::string NodeValue = channel.child_value();
                if (!NodeValue.empty()) {
                    dlg.AddChannel(NodeValue);
                    unsortedChannels.push_back(NodeValue);

                    xlColor c(chanColor, false);
                    CheckForVixenRGB(NodeValue, c, dlg);
                }
            }
        }
    }

    // Read EventValues (base64 encoded)
    pugi::xml_node eventValuesNode = root.child("EventValues");
    if (eventValuesNode) {
        std::string NodeValue = eventValuesNode.child_value();
        base64_decode(NodeValue, VixSeqData);
    }

    // I added the ceiling command because i had an example file that ended up one calculating number of frames one less than
    // the previous calculation because it had a partial last frame
    int numFrames = (int)std::ceil((float)time / (float)frameTime);

    dlg.SortChannels();

    spdlog::debug("Showing mapping dialog.");
    dlg.InitImport();

    if (dlg.ShowModal() != wxID_OK || dlg._dataModel == nullptr) {
        return;
    }

    spdlog::debug("Doing the import of the mapped channels.");
    for (size_t i = 0; i < dlg._dataModel->GetChildCount(); i++) {
        xLightsImportModelNode* m = dlg._dataModel->GetNthChild(i);
        std::string modelName = m->_model;
        Model* mc = GetModel(modelName);
        ModelElement* model = nullptr;
        for (size_t x = 0; x < _sequenceElements.GetElementCount(); x++) {
            if (_sequenceElements.GetElement(x)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(x)->GetName()) {
                model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(x));
                break;
            }
        }
        if (m->_mapping != "") {
            if (model == nullptr) {
                model = AddModel(GetModel(modelName), _sequenceElements);
            }
            if (model == nullptr) {
                spdlog::error("Attempt to add model {} during Vixen import failed.", modelName);
            } else {
                MapVixChannelInformation(this, model->GetEffectLayer(0),
                                         VixSeqData, frameTime, numFrames,
                                         m->_mapping,
                                         unsortedChannels,
                                         m->_color,
                                         *mc, dlg.CheckBox_EraseExistingEffects->GetValue());
            }
        }

        int str = 0;
        for (size_t j = 0; j < m->GetChildCount(); j++) {
            xLightsImportModelNode* s = m->GetNthChild(j);

            if ("" != s->_mapping) {
                    if (model == nullptr) {
                        model = AddModel(GetModel(modelName), _sequenceElements);
                    }
                    if (model == nullptr) {
                        spdlog::error("Attempt to add model {} during Vixen import failed.", modelName);
                } else {
                    SubModelElement* ste = model->GetSubModel(str);
                    if (ste != nullptr) {
                        MapVixChannelInformation(this, ste->GetEffectLayer(0),
                                                 VixSeqData, frameTime, numFrames,
                                                 s->_mapping,
                                                 unsortedChannels,
                                                 s->_color, *mc, dlg.CheckBox_EraseExistingEffects->GetValue());
                    }
                }
            }
            for (size_t n = 0; n < s->GetChildCount(); n++) {
                xLightsImportModelNode* ns = s->GetNthChild(n);
                if ("" != ns->_mapping) {
                    if (model == nullptr) {
                        model = AddModel(GetModel(modelName), _sequenceElements);
                    }
                    if (model == nullptr) {
                        spdlog::error("Attempt to add model {} during Vixen import failed.", modelName);
                    } else {
                        SubModelElement* ste = model->GetSubModel(str);
                        StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                        if (stre != nullptr) {
                            NodeLayer* nl = stre->GetNodeLayer(n, true);
                            if (nl != nullptr) {
                                MapVixChannelInformation(this, nl,
                                                         VixSeqData, frameTime, numFrames,
                                                         ns->_mapping,
                                                         unsortedChannels,
                                                         ns->_color, *mc, dlg.CheckBox_EraseExistingEffects->GetValue());
                            }
                        }
                    }
                }
            }
            str++;
        }
    }

    SetStatusText(wxString::Format("'%s' imported.", filename.GetPath()));
}

void xLightsFrame::ImportHLS(const wxFileName& filename)
{
    pugi::xml_document input_xml;
    if (!input_xml.load_file(filename.GetFullPath().mb_str()))
        return;

    LMSImportChannelMapDialog dlg(this, filename);
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;

    /*
     </ChannelData> | </IlluminationData>
     </Channels>
     </Universe>
     </TotalUniverses> | NumberOfTimeCells | MilliSecPerTimeUnit
     </HLS_OutputSequence>
     */
    int frames = 0;
    int frameTime = 0;
    pugi::xml_node totalUniverses;
    for (pugi::xml_node tuniv = input_xml.document_element().first_child(); tuniv; tuniv = tuniv.next_sibling()) {
        if (std::string_view(tuniv.name()) == "NumberOfTimeCells") {
            frames = tuniv.text().as_int();
        } else if (std::string_view(tuniv.name()) == "MilliSecPerTimeUnit") {
            frameTime = tuniv.text().as_int();
        } else if (std::string_view(tuniv.name()) == "TotalUniverses") {
            totalUniverses = tuniv;
            for (pugi::xml_node univ = tuniv.first_child(); univ; univ = univ.next_sibling()) {
                if (std::string_view(univ.name()) == "Universe") {
                    for (pugi::xml_node channels = univ.first_child(); channels; channels = channels.next_sibling()) {
                        if (std::string_view(channels.name()) == "Channels") {
                            for (pugi::xml_node chand = channels.first_child(); chand; chand = chand.next_sibling()) {
                                if (std::string_view(chand.name()) == "ChannelData") {
                                    for (pugi::xml_node chani = chand.first_child(); chani; chani = chani.next_sibling()) {
                                        if (std::string_view(chani.name()) == "ChanInfo") {
                                            std::string info = chani.text().get();
                                            if (info.find(", Normal") != info.npos) {
                                                std::string name = info.substr(0, info.find(", Normal"));
                                                dlg.channelNames.push_back(name);
                                                dlg.channelColors[name] = xlWHITE;
                                                MapToStrandName(name, dlg.ccrNames);
                                            } else if (info.find(", RGB-") != info.npos) {
                                                std::string name = info.substr(0, info.find(", RGB-"));
                                                std::string color = info.substr(info.size() - 1, 1);
                                                if (color == "R") {
                                                    dlg.channelNames.push_back(name);
                                                    dlg.channelColors[name] = xlBLACK;
                                                }
                                                dlg.channelNames.push_back(info);
                                                if (color == "R") {
                                                    dlg.channelColors[info] = xlRED;
                                                } else if (color == "G") {
                                                    dlg.channelColors[info] = xlGREEN;
                                                } else {
                                                    dlg.channelColors[info] = xlBLUE;
                                                }
                                                MapToStrandName(name, dlg.ccrNames);
                                            } // else "Dead Channel"
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    std::sort(dlg.channelNames.begin(), dlg.channelNames.end(), stdlistNumberAwareStringCompare);
    dlg.channelNames.insert(dlg.channelNames.begin(), "");
    std::sort(dlg.ccrNames.begin(), dlg.ccrNames.end(), stdlistNumberAwareStringCompare);
    dlg.ccrNames.insert(dlg.ccrNames.begin(), "");

    dlg.Init();

    if (dlg.ShowModal() != wxID_OK) {
        return;
    }

    int row = 0;
    for (size_t m = 0; m < dlg.modelNames.size(); m++) {
        std::string modelName = dlg.modelNames[m];
        Model* mc = GetModel(modelName);
        ModelElement* model = nullptr;
        for (size_t i = 0; i < _sequenceElements.GetElementCount(); i++) {
            if (_sequenceElements.GetElement(i)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(i)->GetName()) {
                model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(i));
            }
        }
        MapHLSChannelInformation(this, model->GetEffectLayer(0),
                                 totalUniverses, frames, frameTime,
                                 dlg.ChannelMapGrid->GetCellValue(row, 3),
                                 dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                 *mc, dlg.MapByStrand->GetValue(), false /*dlg.CheckBox_EraseExisting()->GetValue()*/);
        row++;

        if (!dlg.MapByStrand->GetValue()) {
            for (int str = 0; str < mc->GetNumSubModels(); str++) {
                SubModelElement* se = model->GetSubModel(str);
                EffectLayer* sl = se->GetEffectLayer(0);

                MapHLSChannelInformation(this, sl,
                                         totalUniverses, frames, frameTime,
                                         dlg.ChannelMapGrid->GetCellValue(row, 3),
                                         dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                         *mc, false, false /*dlg.CheckBox_EraseExisting()->GetValue()*/);
                row++;
            }
        }
        for (int str = 0; str < mc->GetNumStrands(); str++) {
            StrandElement* se = model->GetStrand(str, true);
            EffectLayer* sl = se->GetEffectLayer(0);

            if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                if (!dlg.MapByStrand->GetValue()) {
                    MapHLSChannelInformation(this, sl,
                                             totalUniverses, frames, frameTime,
                                             dlg.ChannelMapGrid->GetCellValue(row, 3),
                                             dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                             *mc, false, false /*dlg.CheckBox_EraseExisting()->GetValue()*/);
                } else {
                    std::string ccrName = dlg.ChannelMapGrid->GetCellValue(row, 3).ToStdString();
                    for (int n = 0; n < se->GetNodeLayerCount(); n++) {
                        EffectLayer* layer = se->GetNodeLayer(n, true);

                        wxString nm = FindHLSStrandName(ccrName, n + 1, dlg.channelNames);

                        MapHLSChannelInformation(this, layer,
                                                 totalUniverses, frames, frameTime,
                                                 nm,
                                                 dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                                 *mc, true, false /*dlg.CheckBox_EraseExisting()->GetValue()*/);
                    }
                }
            }
            row++;
            if (!dlg.MapByStrand->GetValue()) {
                for (int n = 0; n < mc->GetStrandLength(str); n++) {
                    if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                        MapHLSChannelInformation(this, se->GetNodeLayer(n, true),
                                                 totalUniverses, frames, frameTime,
                                                 dlg.ChannelMapGrid->GetCellValue(row, 3),
                                                 dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                                 *mc, false, false /*dlg.CheckBox_EraseExisting()->GetValue()*/);
                    }
                    row++;
                }
            }
        }
    }

    SetStatusText(wxString::Format("'%s' imported.", filename.GetPath()));
}

void xLightsFrame::ImportLMS(const wxFileName& filename)
{
    pugi::xml_document input_xml;
    if (!input_xml.load_file(filename.GetFullPath().mb_str()))
        return;
    ImportLMS(input_xml, filename);
    SetStatusText(wxString::Format("'%s' imported.", filename.GetPath()));
}

void xLightsFrame::ImportLPE(const wxFileName& filename)
{
    pugi::xml_document input_xml;
    if (!input_xml.load_file(filename.GetFullPath().mb_str()))
        return;
    ImportLPE(input_xml, filename);
    SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath()));
}

void xLightsFrame::ImportS5(const wxFileName& filename)
{
    pugi::xml_document input_xml;
    if (!input_xml.load_file(filename.GetFullPath().mb_str()))
        return;
    ImportS5(input_xml, filename);
    SetStatusText(wxString::Format("'%s' imported.", filename.GetPath()));
}

void xLightsFrame::ImportSuperStar(const wxFileName& filename)
{
    SuperStarImportDialog dlg(this);

    for (size_t i = 0; i < _sequenceElements.GetElementCount(); i++) {
        if (_sequenceElements.GetElement(i)->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            dlg.ChoiceSuperStarImportModel->Append(_sequenceElements.GetElement(i)->GetName());

            ModelElement* model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(i));
            for (int x = 0; x < model->GetSubModelAndStrandCount(); x++) {
                std::string fname = model->GetSubModel(x)->GetFullName();
                const std::string& name = model->GetSubModel(x)->GetName();
                if (name != "") {
                    dlg.ChoiceSuperStarImportModel->Append(fname);
                }
            }
        }
    }

    if (dlg.ShowModal() == wxID_CANCEL) {
        return;
    }

    wxString model_name = dlg.ChoiceSuperStarImportModel->GetStringSelection();
    if (model_name == "") {
        DisplayError("Please select the target model!");
        return;
    }

    wxStopWatch sw;

    // Read the file into a buffer and run the SuperStar XML preprocessor
    // (replaces the legacy FixXMLInputStream wrapper).
    wxFileInputStream fin(filename.GetFullPath());
    if (!fin.IsOk()) {
        DisplayError("Could not open SuperStar file.", this);
        return;
    }
    std::vector<char> xmlBuffer;
    {
        char chunk[4096];
        while (!fin.Eof()) {
            fin.Read(chunk, sizeof(chunk));
            size_t bytesRead = fin.LastRead();
            if (bytesRead == 0) break;
            xmlBuffer.insert(xmlBuffer.end(), chunk, chunk + bytesRead);
        }
    }
    SuperStar::PreprocessXmlBuffer(xmlBuffer);

    pugi::xml_document input_xml;
    if (!input_xml.load_buffer(xmlBuffer.data(), xmlBuffer.size())) {
        DisplayError("Problem loading superstar file.", this);
        return;
    }

    // Resolve the picked model name (top-level model OR submodel/strand).
    Element* model = nullptr;
    for (size_t i = 0; i < _sequenceElements.GetElementCount(); i++) {
        Element* candidate = _sequenceElements.GetElement(i);
        if (candidate->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
        if (candidate->GetName() == model_name) {
            model = candidate;
            break;
        }
        ModelElement* modelEl = dynamic_cast<ModelElement*>(candidate);
        if (modelEl == nullptr) continue;
        for (int x = 0; x < modelEl->GetSubModelAndStrandCount(); x++) {
            if (modelEl->GetSubModel(x)->GetFullName() == model_name) {
                model = modelEl->GetSubModel(x);
                break;
            }
        }
        if (model != nullptr) break;
    }
    if (model == nullptr) {
        DisplayError(wxString::Format("Target model '%s' not found.", model_name).ToStdString(), this);
        return;
    }

    Model* cls = GetModel(model->GetFullName());
    int bw = 1, bh = 1;
    if (cls != nullptr) {
        cls->GetBufferSize("Default", "2D", "None", bw, bh, 0);
    }

    SuperStar::Options opt;
    opt.xSize = wxAtoi(dlg.TextCtrl_SS_X_Size->GetValue());
    opt.ySize = wxAtoi(dlg.TextCtrl_SS_Y_Size->GetValue());
    opt.xOffset = wxAtoi(dlg.TextCtrl_SS_X_Offset->GetValue());
    opt.yOffset = wxAtoi(dlg.TextCtrl_SS_Y_Offset->GetValue());
    opt.imageResize = static_cast<SuperStar::ImageResize>(dlg.ImageResizeChoice->GetSelection());
    opt.layerBlend = dlg.Choice_LayerBlend->GetStringSelection().ToStdString();
    opt.timingOffsetMs = dlg.TimeAdjSpinCtrl->GetValue();
    opt.frameTimeMs = _seqData.FrameTime();
    opt.modelWidth = bw;
    opt.modelHeight = bh;
    opt.defaultGroupName = filename.GetName().ToStdString();

    SuperStar::Importer importer(model, &_sequenceElements.GetSequenceMedia(), opt);
    importer.SetPrefixPromptCallback([this, defGroup = filename.GetName()](std::string& prefix) -> bool {
        wxString defaultPrompt = defGroup.IsEmpty() ? wxString("SuperStar") : defGroup;
        wxTextEntryDialog d(this,
            "Enter a group name / prefix for embedded images:",
            "Embedded Image Group", defaultPrompt);
        if (d.ShowModal() == wxID_CANCEL) return false;
        prefix = d.GetValue().ToStdString();
        return true;
    });

    std::string err;
    bool ok = importer.Run(input_xml, &err);
    if (!ok && !err.empty()) {
        DisplayError(err, this);
    }

    float elapsedTime = sw.Time() / 1000.0;
    SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}

bool findRGB(pugi::xml_node e, pugi::xml_node chan, pugi::xml_node& rchannel, pugi::xml_node& gchannel, pugi::xml_node& bchannel)
{
    std::string idxs[3];
    int cnt = 0;
    for (pugi::xml_node n = chan.first_child(); n; n = n.next_sibling()) {
        if (std::string_view(n.name()) == "channels") {
            for (pugi::xml_node n2 = n.first_child(); n2; n2 = n2.next_sibling()) {
                if (std::string_view(n2.name()) == "channel" && cnt < 3) {
                    idxs[cnt] = n2.attribute("savedIndex").as_string();
                    cnt++;
                }
            }
        }
    }
    for (pugi::xml_node ch = e.first_child(); ch; ch = ch.next_sibling()) {
        if (std::string_view(ch.name()) == "channel") {
            std::string idx = ch.attribute("savedIndex").as_string();
            if (idx == idxs[0]) {
                rchannel = ch;
            }
            if (idx == idxs[1]) {
                gchannel = ch;
            }
            if (idx == idxs[2]) {
                bchannel = ch;
            }
        }
    }
    return true;
}

void GetRGBTimes(pugi::xml_node re, int& startms, int& endms)
{
    if (re) {
        startms = re.attribute("startCentisecond").as_int() * 10;
        endms = re.attribute("endCentisecond").as_int() * 10;
    } else {
        startms = 9999999;
        endms = 9999999;
    }
}
void GetIntensities(pugi::xml_node re, int& starti, int& endi)
{
    int intensity = re.attribute("intensity").as_int(-1);
    if (intensity == -1) {
        starti = re.attribute("startIntensity").as_int();
        endi = re.attribute("endIntensity").as_int();
    } else {
        starti = endi = intensity;
    }
}

class RGBData
{
public:
    int startms, endms;
    int starti, endi;
    bool shimmer;
};

void FillData(pugi::xml_node nd, RGBData& data)
{
    GetIntensities(nd, data.starti, data.endi);
    GetRGBTimes(nd, data.startms, data.endms);
    data.shimmer = std::string_view(nd.attribute("type").as_string()) == "shimmer";
}
void Insert(int x, std::vector<RGBData>& v, int startms)
{
    v.insert(v.begin() + x, 1, RGBData());
    v[x].startms = startms;
    v[x].endms = v[x + 1].startms;
    v[x].endi = v[x].starti = 0;
    v[x].shimmer = false;
}
void Split(int x, std::vector<RGBData>& v, int endms)
{
    v.insert(v.begin() + x, 1, RGBData());
    v[x].startms = v[x + 1].startms;
    v[x].endms = endms;
    v[x + 1].startms = endms;
    v[x].shimmer = v[x + 1].shimmer;
    v[x].starti = v[x + 1].starti;
    double d = endms - v[x].startms;
    d = d / double(v[x + 1].endms - v[x].startms);
    double newi = (v[x + 1].endi - v[x].starti) * d + v[x].starti;
    v[x].endi = v[x + 1].starti = newi;
}
#define MAXMS 99999999
int GetStartMS(int x, std::vector<RGBData>& v)
{
    if (x < (int)v.size()) {
        return v[x].startms;
    }
    return MAXMS;
}
int GetEndMS(int x, std::vector<RGBData>& v)
{
    if (x < (int)v.size()) {
        return v[x].endms;
    }
    return MAXMS;
}
void Resize(int x,
            std::vector<RGBData>& v, int startms)
{
    while (x >= (int)v.size()) {
        int i = v.size();
        v.push_back(RGBData());
        v[i].endms = MAXMS;
        v[i].startms = startms;
        v[i].starti = v[i].endi = 0;
        v[i].shimmer = false;
    }
}

void UnifyData(int x,
               std::vector<RGBData>& red,
               std::vector<RGBData>& green,
               std::vector<RGBData>& blue)
{
    int min = std::min(GetStartMS(x, red), std::min(GetStartMS(x, green), GetStartMS(x, blue)));
    Resize(x, red, min);
    Resize(x, green, min);
    Resize(x, blue, min);
    if (red[x].startms != min) {
        Insert(x, red, min);
    }
    if (green[x].startms != min) {
        Insert(x, green, min);
    }
    if (blue[x].startms != min) {
        Insert(x, blue, min);
    }
    min = std::min(GetEndMS(x, red), std::min(GetEndMS(x, green), GetEndMS(x, blue)));
    if (min == MAXMS) {
        return;
    }
    if (red[x].endms != min) {
        Split(x, red, min);
    }
    if (green[x].endms != min) {
        Split(x, green, min);
    }
    if (blue[x].endms != min) {
        Split(x, blue, min);
    }
}
bool GetRGBEffectData(RGBData& red, RGBData& green, RGBData& blue, xlColor& sc, xlColor& ec)
{
    sc.red = red.starti * 255 / 100;
    sc.green = green.starti * 255 / 100;
    sc.blue = blue.starti * 255 / 100;

    ec.red = red.endi * 255 / 100;
    ec.green = green.endi * 255 / 100;
    ec.blue = blue.endi * 255 / 100;

    return red.shimmer | blue.shimmer | green.shimmer;
}

void LoadRGBData(EffectManager& effectManager, EffectLayer* layer, pugi::xml_node rchannel, pugi::xml_node gchannel, pugi::xml_node bchannel)
{
    std::vector<RGBData> red, green, blue;
    while (rchannel) {
        int startms, endms;
        GetRGBTimes(rchannel, startms, endms);
        if (startms < endms) {
            red.resize(red.size() + 1);
            FillData(rchannel, red[red.size() - 1]);
        }
        rchannel = rchannel.next_sibling();
    }
    while (gchannel) {
        int startms, endms;
        GetRGBTimes(gchannel, startms, endms);
        if (startms < endms) {
            green.resize(green.size() + 1);
            FillData(gchannel, green[green.size() - 1]);
        }
        gchannel = gchannel.next_sibling();
    }
    while (bchannel) {
        int startms, endms;
        GetRGBTimes(bchannel, startms, endms);
        if (startms < endms) {
            blue.resize(blue.size() + 1);
            FillData(bchannel, blue[blue.size() - 1]);
        }
        bchannel = bchannel.next_sibling();
    }
    // have the data, now need to split it so common start/end times
    for (size_t x = 0; x < red.size() || x < green.size() || x < blue.size(); x++) {
        UnifyData(x, red, green, blue);
    }

    for (size_t x = 0; x < red.size() || x < green.size() || x < blue.size(); x++) {
        xlColor sc, ec;
        bool isShimmer = GetRGBEffectData(red[x], green[x], blue[x], sc, ec);

        int starttime = red[x].startms;
        int endtime = red[x].endms;

        if (ec == sc) {
            if (ec != xlBLACK) {
                std::string palette = "C_BUTTON_Palette1=" + (std::string)sc + ",C_CHECKBOX_Palette1=1," + "C_BUTTON_Palette2=#000000,C_CHECKBOX_Palette2=0";
                std::string settings = (isShimmer ? "E_CHECKBOX_On_Shimmer=1" : "");
                layer->AddEffect(0, "On", settings, palette, starttime, endtime, false, false);
            }
        } else if (sc == xlBLACK) {
            std::string palette = "C_BUTTON_Palette1=" + (std::string)ec + ",C_CHECKBOX_Palette1=1,"
                                                                           "C_BUTTON_Palette2=#000000,C_CHECKBOX_Palette2=0";
            std::string settings = "E_TEXTCTRL_Eff_On_Start=0";
            if (isShimmer) {
                settings += ",E_CHECKBOX_On_Shimmer=1";
            }
            layer->AddEffect(0, "On", settings, palette, starttime, endtime, false, false);
        } else if (ec == xlBLACK) {
            std::string palette = "C_BUTTON_Palette1=" + (std::string)sc + ",C_CHECKBOX_Palette1=1,"
                                                                           "C_BUTTON_Palette2=#000000,C_CHECKBOX_Palette2=0";
            std::string settings = "E_TEXTCTRL_Eff_On_End=0";
            if (isShimmer) {
                settings += ",E_CHECKBOX_On_Shimmer=1";
            }
            layer->AddEffect(0, "On", settings, palette, starttime, endtime, false, false);
        } else {
            std::string palette = "C_BUTTON_Palette1=" + (std::string)sc + ",C_CHECKBOX_Palette1=1,"
                                                                           "C_BUTTON_Palette2=" +
                                  (std::string)ec + ",C_CHECKBOX_Palette2=1";
            std::string settings = (isShimmer ? "E_CHECKBOX_ColorWash_Shimmer=1," : "");
            layer->AddEffect(0, "Color Wash", settings, palette, starttime, endtime, false, false);
        }
    }
}

void MapRGBEffects(EffectManager& effectManager, EffectLayer* layer, pugi::xml_node rchannel, pugi::xml_node gchannel, pugi::xml_node bchannel)
{
    pugi::xml_node re = rchannel.first_child();
    while (re && std::string_view(re.name()) != "effect")
        re = re.next_sibling();
    pugi::xml_node ge = gchannel.first_child();
    while (ge && std::string_view(ge.name()) != "effect")
        ge = ge.next_sibling();
    pugi::xml_node be = bchannel.first_child();
    while (be && std::string_view(be.name()) != "effect")
        be = be.next_sibling();
    LoadRGBData(effectManager, layer, re, ge, be);
}

std::string Scale255To100(wxString s, bool doscale)
{
    if (doscale)
        return wxString::Format("%d", wxAtoi(s) * 100 / 255).ToStdString();

    return s.ToStdString();
}

void MapOnEffects(EffectManager& effectManager, EffectLayer* layer, pugi::xml_node channel, int chancountpernode, const wxColor& color)
{
    std::string palette = "C_BUTTON_Palette1=#FFFFFF,C_CHECKBOX_Palette1=1";
    if (chancountpernode > 1) {
        xlColor color1(color.Red(), color.Green(), color.Blue());
        palette = "C_BUTTON_Palette1=" + (std::string)color1 + ",C_CHECKBOX_Palette1=1";
    }

    for (pugi::xml_node ch = channel.first_child(); ch; ch = ch.next_sibling()) {
        if (std::string_view(ch.name()) == "effect") {
            std::string type = ch.attribute("type").as_string();
            bool doscale = (type == "DMX intensity");
            int starttime = ch.attribute("startCentisecond").as_int() * 10;
            int endtime = ch.attribute("endCentisecond").as_int() * 10;
            std::string intensity = ch.attribute("intensity").as_string("-1");
            std::string starti, endi;
            if (intensity == "-1") {
                starti = Scale255To100(wxString(ch.attribute("startIntensity").as_string()), doscale);
                endi = Scale255To100(wxString(ch.attribute("endIntensity").as_string()), doscale);
            } else {
                starti = endi = Scale255To100(intensity, doscale);
            }

            if (type == "twinkle") {
                std::string efpalette = palette;
                int steps = std::max((float)(endtime - starttime - 1000) / 100.0, 0.0) + (rand01() * 10.0 - 5.0);
                steps = std::max(steps, 2);
                steps = std::min(steps, 200);
                std::string settings = "E_CHECKBOX_Twinkle_Strobe=1,E_SLIDER_Twinkle_Count=2,E_SLIDER_Twinkle_Steps=" + wxString::Format("%d", steps);

                if (starti == endi) {
                    if (starti != "100") {
                        efpalette += ",C_SLIDER_Brightness=" + starti;
                    }
                } else {
                    efpalette += wxString::Format(",C_VALUECURVE_Brightness=Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Ramp|Min=0.00|Max=400.00|P1=%s.00|P2=%s.00|RV=TRUE|", starti, endi);
                }

                layer->AddEffect(0, "Twinkle", settings, efpalette, starttime, endtime, false, false);
            } else {
                std::string settings;
                if ("100" != starti) {
                    settings += "E_TEXTCTRL_Eff_On_Start=" + starti;
                }
                if ("100" != endi) {
                    if (!settings.empty()) {
                        settings += ",";
                    }
                    settings += "E_TEXTCTRL_Eff_On_End=" + endi;
                }
                if (type != "intensity" && type != "DMX intensity") {
                    if (!settings.empty()) {
                        settings += ",";
                    }
                    settings += "E_CHECKBOX_On_Shimmer=1";
                }
                layer->AddEffect(0, "On", settings, palette, starttime, endtime, false, false);
            }
        }
    }
}

bool MapChannelInformation(EffectManager& effectManager, EffectLayer* layer, pugi::xml_document& input_xml, const wxString& nm, const wxColor& color, const Model& mc, bool eraseExisting)
{
    if ("" == nm) {
        return false;
    }

    if (eraseExisting)
        layer->DeleteAllEffects();

    pugi::xml_node channel;
    pugi::xml_node rchannel;
    pugi::xml_node gchannel;
    pugi::xml_node bchannel;
    for (pugi::xml_node e = input_xml.document_element().first_child(); !channel && e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "channels") {
            for (pugi::xml_node chan = e.first_child(); !channel && chan; chan = chan.next_sibling()) {
                std::string unit = chan.attribute("unit").as_string();
                std::string circuit = chan.attribute("circuit").as_string();
                std::string dedupname = std::string(chan.attribute("name").as_string()) + "_Unit_" + unit + "_Circuit_" + circuit;
                std::string chanName = chan.name();
                if ((chanName == "channel" || chanName == "rgbChannel") && (nm == chan.attribute("name").as_string() || nm == dedupname)) {
                    channel = chan;
                    if (chanName == "rgbChannel" && !findRGB(e, chan, rchannel, gchannel, bchannel)) {
                        return false;
                    }
                    break;
                }
            }
        }
    }
    if (!channel) {
        return false;
    }
    if (std::string_view(channel.name()) == "rgbChannel") {
        MapRGBEffects(effectManager, layer, rchannel, gchannel, bchannel);
    } else {
        MapOnEffects(effectManager, layer, channel, mc.GetChanCountPerNode(), color);
    }
    return true;
}

void MapCCRModel(int& node, const std::vector<std::string>& channelNames, ModelElement* model, xLightsImportModelNode* m, Model* mc, pugi::xml_document& input_xml, EffectManager& effectManager, bool eraseExisting)
{
    wxString ccrName = m->_mapping;

    for (int str = 0; str < mc->GetNumStrands(); ++str) {
        StrandElement* se = model->GetStrand(str, true);

        for (int n = 0; n < se->GetNodeLayerCount(); n++) {
            EffectLayer* layer = se->GetNodeLayer(n, true);
            wxString nm = ccrName + wxString::Format("-P%02d", (node + 1));
            if (std::find(channelNames.begin(), channelNames.end(), nm) == channelNames.end()) {
                nm = ccrName + wxString::Format(" p%02d", (node + 1));
            }
            if (std::find(channelNames.begin(), channelNames.end(), nm) == channelNames.end()) {
                nm = ccrName + wxString::Format("-P%d", (node + 1));
            }
            if (std::find(channelNames.begin(), channelNames.end(), nm) == channelNames.end()) {
                nm = ccrName + wxString::Format(" p%d", (node + 1));
            }
            if (std::find(channelNames.begin(), channelNames.end(), nm) == channelNames.end()) {
                nm = ccrName + wxString::Format(" P %02d", (node + 1));
            }
            MapChannelInformation(effectManager,
                                  layer, input_xml,
                                  nm, m->_color,
                                  *mc, eraseExisting);
            node++;
        }
    }
}

void MapCCRStrand(const std::vector<std::string>& channelNames, StrandElement* se, xLightsImportModelNode* s, Model* mc, pugi::xml_document& input_xml, EffectManager& effectManager, bool eraseExisting)
{
    int node = 0;
    wxString ccrName = s->_mapping;

    for (int n = 0; n < se->GetNodeLayerCount(); n++) {
        EffectLayer* layer = se->GetNodeLayer(n, true);
        wxString nm = ccrName + wxString::Format("-P%02d", (node + 1));
        if (std::find(channelNames.begin(), channelNames.end(), nm) == channelNames.end()) {
            nm = ccrName + wxString::Format(" p%02d", (node + 1));
        }
        if (std::find(channelNames.begin(), channelNames.end(), nm) == channelNames.end()) {
            nm = ccrName + wxString::Format("-P%d", (node + 1));
        }
        if (std::find(channelNames.begin(), channelNames.end(), nm) == channelNames.end()) {
            nm = ccrName + wxString::Format(" p%d", (node + 1));
        }
        if (std::find(channelNames.begin(), channelNames.end(), nm) == channelNames.end()) {
            nm = ccrName + wxString::Format(" P %02d", (node + 1));
        }
        MapChannelInformation(effectManager,
                              layer, input_xml,
                              nm, s->_color,
                              *mc, eraseExisting);
        node++;
    }
}

void MapCCR(const std::vector<std::string>& channelNames, ModelElement* model, xLightsImportModelNode* m, Model* mc, pugi::xml_document& input_xml, EffectManager& effectManager, bool eraseExisting)
{
    if (mc->GetDisplayAs() == DisplayAsType::ModelGroup) {
        ModelGroup* mg = (ModelGroup*)mc;
        int node = 0;
        for (auto it = mg->Models().begin(); it != mg->Models().end(); ++it) {
            MapCCRModel(node, channelNames, model, m, *it, input_xml, effectManager, eraseExisting);
        }
    } else {
        int node = 0;
        MapCCRModel(node, channelNames, model, m, mc, input_xml, effectManager, eraseExisting);
    }
}

bool xLightsFrame::ImportLMS(pugi::xml_document& input_xml, const wxFileName& filename)
{
    static 
    xLightsImportChannelMapDialog dlg(this, filename, true, true, true, true, false);
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;
    std::vector<std::string> timingTrackNames;
    std::map<std::string, pugi::xml_node> timingTracks;

    for (pugi::xml_node e = input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "channels") {
            for (pugi::xml_node chan = e.first_child(); chan; chan = chan.next_sibling()) {
                if (std::string_view(chan.name()) == "channel" || std::string_view(chan.name()) == "rgbChannel") {
                    std::string name = chan.attribute("name").as_string();
                    if (std::string_view(chan.name()) == "rgbChannel") {
                        dlg.channelColors[name] = xlBLACK;
                    } else {
                        std::string color = chan.attribute("color").as_string();
                        std::string unit = chan.attribute("unit").as_string();
                        std::string circuit = chan.attribute("circuit").as_string();
                        if (dlg.GetImportChannel(name)) {
                            name += "_Unit_" + unit + "_Circuit_" + circuit;
                        }
                        dlg.channelColors[name] = GetColor(color);
                    }

                    bool ccr = false;
                    if (std::string_view(chan.name()) == "rgbChannel") {
                        int idxDP = name.find("-P");
                        int idxUP = name.find(" P");
                        int idxSP = name.find(" p");
                        if (idxUP > idxSP) {
                            idxSP = idxUP;
                        }
                        if (idxDP > idxSP) {
                            idxSP = idxDP;
                        }
                        if (idxSP != wxNOT_FOUND) {
                            int i = wxAtoi(name.substr(idxSP + 2, name.size()));
                            if (i > 0 && name != "") {
                                ccr = true;
                                dlg.AddChannel(name);
                                // if (std::find(dlg.ccrNames.begin(), dlg.ccrNames.end(), name.substr(0, idxSP - 1)) == dlg.ccrNames.end())
                                if (name.substr(0, idxSP) != "" && std::find(dlg.ccrNames.begin(), dlg.ccrNames.end(), name.substr(0, idxSP)) == dlg.ccrNames.end()) {
                                    // dlg.ccrNames.push_back(name.substr(0, idxSP - 1));
                                    dlg.ccrNames.push_back(name.substr(0, idxSP));
                                }
                            }
                        }
                    }

                    if (!ccr && name != "") {
                        dlg.AddChannel(name);
                    }
                }
            }
        } else if (std::string_view(e.name()) == "timingGrids") {
            for (pugi::xml_node timing = e.first_child(); timing; timing = timing.next_sibling()) {
                if (std::string_view(timing.name()) == "timingGrid") {
                    std::string type = timing.attribute("type").as_string();
                    if (type != "fixed") {
                        std::string name = timing.attribute("name").as_string();
                        if (name != "") {
                            timingTrackNames.push_back(name);
                            timingTracks[name] = timing;
                        }
                    }
                }
            }
        }
    }

    dlg.SortChannels();
    std::sort(dlg.ccrNames.begin(), dlg.ccrNames.end(), stdlistNumberAwareStringCompare);
    dlg.timingTracks = timingTrackNames;

    dlg.InitImport();

    if (dlg.ShowModal() != wxID_OK || dlg._dataModel == nullptr) {
        return false;
    }

    if (dlg.TimeAdjustSpinCtrl->GetValue() != 0) {
        int offset = dlg.TimeAdjustSpinCtrl->GetValue();
        SuperStar::AdjustAllTimings(input_xml.document_element(), offset / 10);
    }

    for (size_t tt = 0; tt < dlg.TimingTrackListBox->GetCount(); ++tt) {
        if (dlg.TimingTrackListBox->IsChecked(tt)) {
            std::string name = dlg.TimingTrackListBox->GetString(tt).ToStdString();
            TimingElement* target = (TimingElement*)_sequenceElements.AddElement(name, "timing", true, true, false, false, false);
            char cnt = '1';
            while (target == nullptr) {
                target = (TimingElement*)_sequenceElements.AddElement(name + "-" + cnt++, "timing", true, true, false, false, false);
            }
            if (target->GetEffectLayerCount() == 0) {
                target->AddEffectLayer();
            }

            int offset = dlg.TimeAdjustSpinCtrl->GetValue();
            EffectLayer* targetLayer = target->GetEffectLayer(0);
            long last = offset;
            for (pugi::xml_node t = timingTracks[name].first_child(); t; t = t.next_sibling()) {
                if (std::string_view(t.name()) == "timing") {
                    int time = t.attribute("centisecond").as_int() * 10 + offset;
                    int adjTime = RoundToMultipleOfPeriod(time, CurrentSeqXmlFile->GetFrequency());
                    if (adjTime > last) {
                        targetLayer->AddEffect(0, "", "", "", last, adjTime, false, false);
                        last = adjTime;
                    }
                }
            }
        }
    }

    for (size_t i = 0; i < dlg._dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = dlg._dataModel->GetNthChild(i);
        std::string modelName = m->_model;
        Model* mc = GetModel(modelName);
        ModelElement* model = nullptr;
        for (size_t x = 0; x < _sequenceElements.GetElementCount(); x++) {
            if (_sequenceElements.GetElement(x)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(x)->GetName()) {
                model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(x));
                break;
            }
        }

        if (m->_mapping != "") {
            if (model == nullptr) {
                model = AddModel(GetModel(modelName), _sequenceElements);
            }
            if (model == nullptr) {
                spdlog::error("Attempt to add model {} during LMS import failed.", modelName);
            } else {
                if (std::find(dlg.ccrNames.begin(), dlg.ccrNames.end(), m->_mapping) != dlg.ccrNames.end()) {
                    MapCCR(dlg.GetChannelNames(), model, m, mc, input_xml, effectManager, dlg.CheckBox_EraseExistingEffects->GetValue());
                } else {
                    MapChannelInformation(effectManager,
                                          model->GetEffectLayer(0), input_xml,
                                          m->_mapping,
                                          m->_color, *mc, dlg.CheckBox_EraseExistingEffects->GetValue());
                }
            }
        }

        int str = 0;
        for (size_t j = 0; j < m->GetChildCount(); j++) {
            xLightsImportModelNode* s = m->GetNthChild(j);

            if ("" != s->_mapping) {
                if (model == nullptr) {
                    model = AddModel(GetModel(modelName), _sequenceElements);
                }
                if (model == nullptr) {
                    spdlog::error("Attempt to add model {} during LMS import failed.", modelName);
                } else {
                    if (std::find(dlg.ccrNames.begin(), dlg.ccrNames.end(), s->_mapping) != dlg.ccrNames.end()) {
                        StrandElement* se = model->GetStrand(str);
                        if (se != nullptr) {
                            MapCCRStrand(dlg.GetChannelNames(), se, s, mc, input_xml, effectManager, dlg.CheckBox_EraseExistingEffects->GetValue());
                        } else {
                            spdlog::debug("LMS Import: Strand {} not found.", str);
                        }
                    } else {
                        SubModelElement* ste = model->GetSubModel(str);
                        if (ste != nullptr) {
                            MapChannelInformation(effectManager,
                                                  ste->GetEffectLayer(0), input_xml,
                                                  s->_mapping,
                                                  s->_color, *mc, dlg.CheckBox_EraseExistingEffects->GetValue());
                        } else {
                            spdlog::debug("LMS Import: SubModel {} not found.", str);
                        }
                    }
                }
            }
            for (size_t n = 0; n < s->GetChildCount(); n++) {
                xLightsImportModelNode* ns = s->GetNthChild(n);
                if ("" != ns->_mapping) {
                    if (model == nullptr) {
                        model = AddModel(GetModel(modelName), _sequenceElements);
                    }
                    if (model == nullptr) {
                        spdlog::error("Attempt to add model {} during LMS import failed.", (const char*)modelName.c_str());
                    } else {
                        SubModelElement* ste = model->GetSubModel(str);
                        StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                        if (stre != nullptr) {
                            NodeLayer* nl = stre->GetNodeLayer(n, true);
                            if (nl != nullptr) {
                                MapChannelInformation(effectManager,
                                                      nl, input_xml,
                                                      ns->_mapping,
                                                      ns->_color, *mc, dlg.CheckBox_EraseExistingEffects->GetValue());
                            }
                        }
                    }
                }
            }
            str++;
        }
    }

    return true;
}

bool LPEHasEffects(const pugi::xml_document& input_xml, const wxString& model, int layer, bool left)
{
    for (pugi::xml_node e = input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        std::string ename = e.name();
        if (ename == "SequenceProps" || ename == "ArchivedProps") {
            for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
                std::string pname = prop.name();
                if (pname == "SeqProp" || pname == "ArchiveProp") {
                    std::string name = prop.attribute("name").as_string();
                    if (name == "") {
                        for (pugi::xml_node ap = prop.first_child(); ap; ap = ap.next_sibling()) {
                            if (std::string_view(ap.name()) == "PropClass") {
                                name = ap.attribute("Name").as_string();
                            }
                        }
                    }
                    if (name == model) {
                        for (pugi::xml_node track = prop.first_child(); track; track = track.next_sibling()) {
                            int id = track.attribute("id").as_int();
                            if (id == layer) {
                                for (pugi::xml_node eff = track.first_child(); eff; eff = eff.next_sibling()) {
                                    if (std::string_view(eff.name()) == "effect" && std::string_view(eff.attribute("type").as_string()) == "pixelEffect") {
                                        wxString settings = eff.attribute("pixelEffect").as_string();
                                        wxArrayString as = wxSplit(settings, '|');
                                        if (as.size() == 7) {
                                            wxString s;
                                            if (left) {
                                                s = as[5];
                                            } else {
                                                s = as[6];
                                            }
                                            wxArrayString ss = wxSplit(s, ':');
                                            if (ss[0] != "none")
                                                return true;
                                        } else if (as.size() == 5) {
                                            wxString s;
                                            if (left) {
                                                s = as[3];
                                            } else {
                                                s = as[4];
                                            }
                                            wxArrayString ss = wxSplit(s, ':');
                                            if (ss[0] != "none")
                                                return true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

wxString MapLPEEffectType(const wxString& effect)
{
    if (effect == "colorwash")
        return "Color Wash";
    if (effect == "picture")
        return "Pictures";
    if (effect == "lineshorizontal")
        return "";
    if (effect == "linesvertical")
        return "";
    if (effect == "countdown")
        return "Text"; // we dont support countdown

    return effect.Capitalize();
}

pugi::xml_node FindLastLPEEffectNode(pugi::xml_node effect, int start_centisecond, int end_centisecond, int end_intensity, int intensityChange, const wxString& settings, int& fadeInCS, int& fadeOutCS)
{
    int originalIntensity = end_intensity - intensityChange;

    fadeInCS = 0;
    fadeOutCS = 0;

    pugi::xml_node res = effect;
    pugi::xml_node n = effect.next_sibling();
    while (n) {
        int newstartCentisecond = n.attribute("startCentisecond").as_int();
        // int newendCentisecond = n.attribute("endCentisecond").as_int();
        int newstartIntensity = n.attribute("startIntensity").as_int(100);
        int newendIntensity = n.attribute("endIntensity").as_int(100);
        wxString newsettings = n.attribute("pixelEffect").as_string();
        if (settings != newsettings) {
            break;
        }

        int newintensityChange = newendIntensity - newstartIntensity;
        if (abs(intensityChange - newintensityChange) > 1) {
            // significant change in the amount of intensity change

            if (intensityChange > 0 && newintensityChange == 0 && fadeInCS == 0) {
                fadeInCS = newstartCentisecond;
                intensityChange = newintensityChange;
            } else if (intensityChange == 0 && newintensityChange < 0 && fadeOutCS == 0) {
                fadeOutCS = newstartCentisecond;
                intensityChange = newintensityChange;
            } else {
                // weird intensity change ... lets give up
                break;
            }
        }

        res = n;
        n = n.next_sibling();
    }

    // handle effects that are all fade in or all fade out
    // I use 5-95 as a proxy for 0-100 to maximise the use of fade in/out
    int newendCentisecond = res.attribute("endCentisecond").as_int();
    int newendIntensity = res.attribute("endIntensity").as_int(100);
    if (fadeOutCS == 0 && fadeInCS == 0 && originalIntensity < newendIntensity && newendIntensity > 95 && originalIntensity < 5) {
        // all fade in
        fadeInCS = newendCentisecond;
    } else if (fadeOutCS == 0 && fadeInCS == 0 && originalIntensity > newendIntensity && newendIntensity < 5 && originalIntensity > 95) {
        // all fade out
        fadeOutCS = start_centisecond;
    }

    return res;
}

wxString MapLPEBlend(const wxString& blend, bool left)
{
    if (!left)
        return "Normal";

    if (blend == "Mix_Average")
        return "Average";
    if (blend == "Mix_Overlay")
        return "Normal";
    if (blend == "Mix_Maximum")
        return "Max";
    if (blend == "Mix_Bottom_Top")
        return "Bottom-Top";
    if (blend == "Mix_Left-Right")
        return "Left-Right";
    if (blend == "Mix_Rt_Hides_Lt")
        return "1 is Mask";
    if (blend == "Mix_Rt_Reveals_Lt")
        return "2 reveals 1";

    return "Normal";
}

std::string ExtractLPEPallette(const wxArrayString& ps)
{
    std::string palette;

    int cnum = 0;
    wxArrayString c = wxSplit(ps[1], ';');
    for (int i = 0; i < (int)c.size(); i++) {
        wxString n = wxString::Format("%d", cnum + 1);

        wxArrayString cc = wxSplit(c[i], ',');
        if (cc.size() == 2) {
            wxString c1 = cc[0].substr(2); // drop transparency
            wxString active = cc[1];

            palette += ",C_BUTTON_Palette" + n + "=#" + c1;
            if (active == "1") {
                palette += ",C_CHECKBOX_Palette" + n + "=" + active;
            }
            cnum++;
        } else if (cc.size() == 3) {
            wxString c1 = cc[0].substr(2); // drop transparency
            wxString c2 = cc[1].substr(2); // drop transparency
            wxString active = cc[2];

            if (c1 == c2) {
                palette += ",C_BUTTON_Palette" + n + "=#" + c1;
                if (active == "1") {
                    palette += ",C_CHECKBOX_Palette" + n + "=" + active;
                }
            } else {
                palette += "C_BUTTON_Palette" + n + "=Active=TRUE|Id=ID_BUTTON_Palette" + n + "|Values=x=0.000^c=#" + c1 + ";x=1.000^c=#" + c2 + "|";
                if (active == "1") {
                    palette += ",C_CHECKBOX_Palette" + n + "=" + active;
                }
            }
            cnum++;
        } else {
            // Not sure what the last value is so ignoring it
            // int unknown1 = wxAtoi(c[i]);
            break;
        }
    }

    return palette;
}

// Used to rescale a parameter to a broader scale.
// Assumes the range is different but the translation is direct.
// If this is not the case then set the source Min/Max to the range that does map to the targetMin/Max and the conversion will
// clamp original values outside the supported range to the largest practical in the target
float Rescale(float original, float sourceMin, float sourceMax, float targetMin, float targetMax)

{
    if (original < sourceMin)
        original = sourceMin;
    if (original > sourceMax)
        original = sourceMax;

    return ((original - sourceMin) / (sourceMax - sourceMin)) * (targetMax - targetMin) + targetMin;
}

wxString RescaleWithRangeI(wxString r, wxString vcName, float sourceMin, float sourceMax, float targetMin, float targetMax, wxString& vc, float targetRealMin, float targetRealMax)
{
    if (r.Contains("R")) {
        // it is a range
        wxArrayString rr = wxSplit(r, 'R');
        vc = "," + vcName + "=Active=TRUE|Id=ID_" + vcName.substr(2) + "|Type=Ramp|Min=" + wxString::Format("%.2f", targetRealMin) +
             "|Max=" + wxString::Format("%.2f", targetRealMax) +
             "|P1=" + wxString::Format("%.2f", Rescale(wxAtof(rr[0]), sourceMin, sourceMax, targetMin, targetMax)) +
             "|P2=" + wxString::Format("%.2f", Rescale(wxAtof(rr[1]), sourceMin, sourceMax, targetMin, targetMax)) +
             "|RV=TRUE|";
        return wxString::Format("%d", (int)Rescale(wxAtof(rr[0]), sourceMin, sourceMax, targetMin, targetMax));
    } else {
        vc = "";
        return wxString::Format("%d", (int)Rescale(wxAtof(r), sourceMin, sourceMax, targetMin, targetMax));
    }
}

wxString RescaleWithRangeF(wxString r, wxString vcName, float sourceMin, float sourceMax, float targetMin, float targetMax, wxString& vc, float targetRealMin, float targetRealMax)
{
    if (r.Contains("R")) {
        // it is a range
        wxArrayString rr = wxSplit(r, 'R');
        vc = "," + vcName + "=Active=TRUE|Id=ID_" + vcName.substr(2) + "|Type=Ramp|Min=" + wxString::Format("%.2f", targetRealMin) +
             "|Max=" + wxString::Format("%.2f", targetRealMax) +
             "|P1=" + wxString::Format("%.2f", Rescale(wxAtof(rr[0]), sourceMin, sourceMax, targetMin, targetMax)) +
             "|P2=" + wxString::Format("%.2f", Rescale(wxAtof(rr[1]), sourceMin, sourceMax, targetMin, targetMax)) +
             "|RV=TRUE|";
    } else {
        vc = "";
    }
    return wxString::Format("%.1f", Rescale(wxAtof(r), sourceMin, sourceMax, targetMin, targetMax));
}

std::string LPEParseEffectSettings(const wxString& effectType, const wxArrayString& arrSettings, std::string& palette, int durationMS)
{
    static 

    std::string settings;

    if (arrSettings.size() > 1) {
        wxArrayString parms = wxSplit(arrSettings[2], ',');
        if (effectType == "butterfly") {
            wxString style = parms[0];
            wxString chunks = parms[1];
            wxString vcChunks;
            // VC bounds come from the effect's statics populated via Butterfly.json at startup.
            chunks = RescaleWithRangeI(chunks, "E_VALUECURVE_Butterfly_Chunks", 1, 10, 1, 10, vcChunks, ButterflyEffect::sChunksMin, ButterflyEffect::sChunksMax);
            wxString skip = parms[2];
            wxString vcSkip;
            skip = RescaleWithRangeI(skip, "E_VALUECURVE_Butterfly_Skip", 2, 10, 2, 10, vcSkip, ButterflyEffect::sSkipMin, ButterflyEffect::sSkipMax);
            wxString direction = parms[3];
            wxString hue = parms[4];
            wxString vcHue;
            hue = RescaleWithRangeI(hue, "C_VALUECURVE_Color_HueAdjust", 0, 359, -100, 100, vcHue, -100, 100);
            wxString speed = parms[5];
            wxString vcSpeed;
            speed = RescaleWithRangeI(speed, "E_VALUECURVE_Butterfly_Speed", 0, 50, 0, 50, vcSpeed, ButterflyEffect::sSpeedMin, ButterflyEffect::sSpeedMax);
            wxString colours = parms[6];

            if (style == "linear") {
                settings += ",E_SLIDER_Butterfly_Style=1";
            } else if (style == "radial") {
                settings += ",E_SLIDER_Butterfly_Style=2";
            } else if (style == "blocks") {
                settings += ",E_SLIDER_Butterfly_Style=3";
            } else if (style == "corner") {
                settings += ",E_SLIDER_Butterfly_Style=5";
            }
            settings += ",E_CHOICE_Butterfly_Colors=" + colours.Capitalize();
            settings += ",E_CHOICE_Butterfly_Direction=" + direction.Capitalize();
            settings += ",E_SLIDER_Butterfly_Chunks=" + chunks;
            settings += vcChunks;
            settings += ",E_SLIDER_Butterfly_Skip=" + skip;
            settings += vcSkip;
            settings += ",E_SLIDER_Butterfly_Speed=" + speed;
            settings += vcSpeed;

            if (hue != "0") {
                palette += ",C_SLIDER_Color_HueAdjust=" + hue;
                palette += vcHue;
            }
        } else if (effectType == "colorwash") {
            // full, full, none, 12
            wxString horizontalFade = parms[0];
            wxString verticalFade = parms[1];

            if (horizontalFade == "full") {
            } else if (horizontalFade == "left_to_right") {
                settings += ",E_CHECKBOX_ColorWash_HFade=1";
            } else if (horizontalFade == "right_to_left") {
                settings += ",E_CHECKBOX_ColorWash_HFade=1";
            } else if (horizontalFade == "center_on") {
                settings += ",E_CHECKBOX_ColorWash_HFade=1";
            } else if (horizontalFade == "center_off") {
                settings += ",E_CHECKBOX_ColorWash_HFade=1";
            }

            if (verticalFade == "full") {
            } else if (verticalFade == "top_to_bottom") {
                settings += ",E_CHECKBOX_ColorWash_VFade=1";
            } else if (verticalFade == "bottom_to_top") {
                settings += ",E_CHECKBOX_ColorWash_VFade=1";
            } else if (verticalFade == "center_on") {
                settings += ",E_CHECKBOX_ColorWash_VFade=1";
            } else if (verticalFade == "center_off") {
                settings += ",E_CHECKBOX_ColorWash_VFade=1";
            }
        } else if (effectType == "spirals") {
            // 1, left_to_right, 20, 50, 0, False, none, 12
            wxString repeat = parms[0];
            wxString vcRepeat;
            repeat = RescaleWithRangeI(repeat, "E_VALUECURVE_Spirals_Count", 1, 5, 1, 5, vcRepeat, SpiralsEffect::sCountMin, SpiralsEffect::sCountMax);
            wxString direction = parms[1];
            wxString rotation = parms[2];
            rotation = wxString::Format("%.2f", wxAtof(rotation) / 60.0);
            wxString vcRotation;
            rotation = RescaleWithRangeF(rotation, "E_VALUECURVE_Spirals_Rotation", 0, 50, 0, 50, vcRotation, SpiralsEffect::sRotationMin, SpiralsEffect::sRotationMax);
            rotation = wxString::Format("%d", (int)(wxAtof(rotation) * 10.0));
            wxString thickness = parms[3];
            wxString vcThickness;
            thickness = RescaleWithRangeI(thickness, "E_VALUECURVE_Spirals_Thickness", 0, 100, 0, 100, vcThickness, SpiralsEffect::sThicknessMin, SpiralsEffect::sThicknessMax);
            // wxString thicknessChange = parms[4]; //unused
            wxString blend = parms[5];
            wxString show3d = parms[6];
            wxString speed = parms[7];
            speed = wxString::Format("%d", (int)(wxAtof(speed) / (20.0 / ((float)durationMS / 1000.0))));
            wxString vcSpeed;
            if (direction == "right_to_left") {
                speed = RescaleWithRangeF(speed, "E_VALUECURVE_Spirals_Movement", 0, 50, 0, -50, vcSpeed, SpiralsEffect::sMovementMin, SpiralsEffect::sMovementMax);
            } else {
                speed = RescaleWithRangeF(speed, "E_VALUECURVE_Spirals_Movement", 0, 50, 0, 50, vcSpeed, SpiralsEffect::sMovementMin, SpiralsEffect::sMovementMax);
            }

            settings += ",E_SLIDER_Spirals_Count=" + repeat;
            settings += vcRepeat;

            settings += ",E_TEXTCTRL_Spirals_Movement=" + speed;
            settings += vcSpeed;

            settings += ",E_SLIDER_Spirals_Rotation=" + rotation;
            settings += vcRotation;

            settings += ",E_SLIDER_Spirals_Thickness=" + thickness;
            settings += vcThickness;

            // dont know what to do with thickness change

            if (blend == "True") {
                settings += ",E_CHECKBOX_Spirals_Blend=1,";
            }

            if (show3d == "none") {
            } else if (show3d == "trail_left") {
                settings += ",E_CHECKBOX_Spirals_3D=1";
            } else if (show3d == "trail_right") {
                settings += ",E_CHECKBOX_Spirals_3D=1";
            }
        } else if (effectType == "bars") {
            // down,2,False,False,8,0
            wxString direction = parms[0];
            wxString repeat = parms[1];
            wxString vcRepeat;
            // VC bounds come from the effect's statics populated via Bars.json at startup.
            repeat = RescaleWithRangeI(repeat, "E_VALUECURVE_Bars_BarCount", 1, 5, 1, 5, vcRepeat, BarsEffect::sBarCountMin, BarsEffect::sBarCountMax);
            wxString highlight = parms[2];
            wxString show3d = parms[3];
            wxString speed = parms[4];
            speed = wxString::Format("%d", (int)(wxAtof(speed) / (20.0 / ((float)durationMS / 1000.0))));
            wxString vcSpeed;
            speed = RescaleWithRangeF(speed, "E_VALUECURVE_Bars_Cycles", 0, 50, 0, 30, vcSpeed, BarsEffect::sCyclesMin, BarsEffect::sCyclesMax);
            wxString centre = parms[5];
            wxString vcCentre;
            centre = RescaleWithRangeI(centre, "E_VALUECURVE_Bars_Center", -50, 50, -100, 100, vcCentre, BarsEffect::sCenterMin, BarsEffect::sCenterMax);

            settings += ",E_SLIDER_Bars_BarCount=" + repeat;
            settings += vcRepeat;

            if (direction == "V_expand")
                direction = "expand";
            if (direction == "V_compress")
                direction = "compress";
            if (direction == "H_expand")
                direction = "H-expand";
            if (direction == "H_compress")
                direction = "H-compress";
            if (direction == "left")
                direction = "Left";
            if (direction == "right")
                direction = "Right";
            if (direction == "block_up")
                direction = "Alternate Up";
            if (direction == "block_down")
                direction = "Alternate Down";
            if (direction == "block_left")
                direction = "Alternate Right";
            if (direction == "block_right")
                direction = "Alternate Right";
            settings += ",E_CHOICE_Bars_Direction=" + direction;

            if (show3d == "True") {
                settings += ",E_CHECKBOX_Bars_3D=1";
            }

            if (highlight == "True") {
                settings += "E_CHECKBOX_Bars_Highlight=1";
            }

            settings += ",E_TEXTCTRL_Bars_Cycles=" + speed;
            settings += vcSpeed;

            settings += ",E_TEXTCTRL_Bars_Center=" + centre;
            settings += vcCentre;
        } else if (effectType == "countdown") {
            // 0,Arial,75,7
            wxString seconds = parms[0];
            wxString font = parms[1];
            wxString fontSize = parms[2];
            wxString vcCrap;
            fontSize = RescaleWithRangeI(fontSize, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
            wxString position = parms[3];
            position = RescaleWithRangeI(position, "IGNORE", -50, 50, -200, 200, vcCrap, -1, -1);

            settings += ",E_TEXTCTRL_Text=" + seconds;
            settings += ",E_CHOICE_Text_Count=seconds";
            settings += ",E_CHOICE_Text_Font=Use OS Fonts";
            settings += ",E_FONTPICKER_Text_Font='" + font + "' " + fontSize;
            settings += ",E_SLIDER_Text_XStart=" + position;
        } else if (effectType == "lineshorizontal") {
            // Bottom_to_Top,8,38

            // No xLights equivalent

            spdlog::warn("LPE conversion for Lines Horizontal does not exist.");
        } else if (effectType == "linesvertical") {
            // Left_to_Right,4,32

            // No xLights equivalent

            spdlog::warn("LPE conversion for Lines Vertical does not exist.");
        } else if (effectType == "curtain") {
            // center,open,0,once_at_speed,12
            wxString edge = parms[0];
            wxString movement = parms[1];
            wxString swag = parms[2];
            wxString vcSwag;
            swag = RescaleWithRangeF(swag, "E_VALUECURVE_Curtain_Swag", 0, 10, 0, 10, vcSwag, CurtainEffect::sSwagMin, CurtainEffect::sSwagMax);
            wxString repeat = parms[3];
            wxString speed = parms[4];
            wxString vcSpeed;
            // Curtain_Speed's pre-migration range was (0, 10) post-divisor. Write the VC in
            // that legacy form — UpgradeValueCurve rescales it to the new (0, 100) pre-divisor
            // form on the first sequence load.
            speed = RescaleWithRangeF(speed, "E_VALUECURVE_Curtain_Speed", 0, 50, 0, 10, vcSpeed, 0, 10);

            settings += ",E_CHOICE_Curtain_Edge=" + edge;
            movement.Replace("_", " ");
            settings += ",E_CHOICE_Curtain_Effect=" + movement;
            settings += ",E_SLIDER_Curtain_Swag=" + swag;
            settings += vcSwag;

            if (repeat == "once_at_speed") {
                settings += ",E_CHECKBOX_Curtain_Repeat=0";
            } else if (repeat == "once_fit_to_duration") {
                settings += ",E_CHECKBOX_Curtain_Repeat=0";
            } else if (repeat == "repeat_at_speed_rotate_colors") {
                settings += ",E_CHECKBOX_Curtain_Repeat=1";
            } else if (repeat == "repeat_at_speed_blend_colors") {
                settings += ",E_CHECKBOX_Curtain_Repeat=1";
            }
            settings += ",E_TEXTCTRL_Curtain_Speed=" + speed;
            settings += vcSpeed;
        } else if (effectType == "fire") {
            // 50,0
            wxString height = parms[0];
            wxString vcHeight;
            height = RescaleWithRangeI(height, "E_VALUECURVE_Fire_Height", 10, 100, 0, 100, vcHeight, FireEffect::sHeightMin, FireEffect::sHeightMax);
            wxString hueShift = parms[1];
            wxString vcHueShift;
            hueShift = RescaleWithRangeI(hueShift, "E_VALUECURVE_Fire_HueShift", 0, 359, 0, 100, vcHueShift, FireEffect::sHueShiftMin, FireEffect::sHueShiftMax);

            settings += ",E_SLIDER_Fire_Height=" + height;
            settings += vcHeight;
            settings += ",E_SLIDER_Fire_HueShift=" + hueShift;
            settings += vcHueShift;
        } else if (effectType == "fireworks") {
            // 10,50,2,30,normal,continuous
            wxString explosionRate = parms[0];
            wxString vcCrap;
            explosionRate = RescaleWithRangeI(explosionRate, "IGNORE", 1, 95, 1, 50, vcCrap, -1, -1);
            wxString particles = parms[1];
            particles = RescaleWithRangeI(particles, "IGNORE", 1, 100, 1, 100, vcCrap, -1, -1);
            wxString velocity = parms[2];
            velocity = RescaleWithRangeI(velocity, "IGNORE", 1, 10, 1, 10, vcCrap, -1, -1);
            wxString fade = parms[3];
            fade = RescaleWithRangeI(fade, "IGNORE", 1, 100, 1, 100, vcCrap, -1, -1);
            //wxString pattern = parms[4];    // not used
            //wxString rateChange = parms[5]; // not used
            settings += ",E_SLIDER_Fireworks_Explosions=" + explosionRate;
            settings += ",E_SLIDER_Fireworks_Count=" + particles;
            settings += ",E_SLIDER_Fireworks_Fade=" + fade;
            settings += ",E_SLIDER_Fireworks_Velocity=" + velocity;
        } else if (effectType == "garland") {
            // 3,34,once_at_speed,12,bottom_to_top
            wxString type = parms[0];
            wxString vcCrap;
            type = RescaleWithRangeI(type, "IGNORE", 0, 4, 0, 4, vcCrap, -1, -1);
            wxString spacing = parms[1];
            wxString vcSpacing;
            spacing = RescaleWithRangeI(spacing, "E_VALUECURVE_Garlands_Spacing", 0, 100, 1, 100, vcSpacing, GarlandsEffect::sSpacingMin, GarlandsEffect::sSpacingMax);
            wxString repeat = parms[2];
            wxString speed = parms[3];
            wxString vcSpeed;
            // Garlands_Cycles pre-migration range was (0, 20) post-divisor. Write the VC
            // in that legacy form — UpgradeValueCurve rescales it to the new (0, 200)
            // pre-divisor form with divisor 10 on the first sequence load.
            speed = RescaleWithRangeF(speed, "E_VALUECURVE_Garlands_Cycles", 0, 50, 0, 20, vcSpeed, 0, 20);
            wxString fill = parms[4];

            settings += ",E_SLIDER_Garlands_Type=" + type;

            if (fill == "bottom_to_top") {
                settings += ",E_CHOICE_Garlands_Direction=Up";
            } else if (fill == "top_to_bottom") {
                settings += ",E_CHOICE_Garlands_Direction=Down";
            } else if (fill == "left_to_right") {
                settings += ",E_CHOICE_Garlands_Direction=Right";
            } else if (fill == "right_to_left") {
                settings += ",E_CHOICE_Garlands_Direction=Left";
            }

            settings += ",E_SLIDER_Garlands_Spacing=" + spacing;
            settings += vcSpacing;

            if (repeat == "repeat_at_speed") {
                settings += ",E_TEXTCTRL_Garlands_Cycles=" + speed;
                settings += vcSpeed;
            } else if (repeat == "once_at_speed") {
                settings += ",E_TEXTCTRL_Garlands_Cycles=1.0";
            } else if (repeat == "once_fit_to_duration") {
                settings += ",E_TEXTCTRL_Garlands_Cycles=1.0";
            }
        } else if (effectType == "meteors") {
            // rainbow,10,25,down,0,12
            wxString colourScheme = parms[0];
            wxString count = parms[1];
            wxString vcCount;
            count = RescaleWithRangeI(count, "E_VALUECURVE_Meteors_Count", 1, 100, 1, 100, vcCount, MeteorsEffect::sCountMin, MeteorsEffect::sCountMax);
            wxString length = parms[2];
            wxString vcLength;
            length = RescaleWithRangeI(length, "E_VALUECURVE_Meteors_Length", 1, 100, 1, 100, vcLength, MeteorsEffect::sLengthMin, MeteorsEffect::sLengthMax);
            wxString effect = parms[3];
            wxString swirl = parms[4];
            wxString vcSwirl;
            swirl = RescaleWithRangeI(swirl, "E_VALUECURVE_Meteors_Swirl_Intensity", 0, 20, 0, 20, vcSwirl, MeteorsEffect::sSwirlMin, MeteorsEffect::sSwirlMax);
            wxString speed = parms[5];
            wxString vcSpeed;
            speed = RescaleWithRangeI(speed, "E_VALUECURVE_Meteors_Speed", 1, 50, 1, 50, vcSpeed, MeteorsEffect::sSpeedMin, MeteorsEffect::sSpeedMax);

            settings += ",E_CHOICE_Meteors_Type=" + colourScheme.Lower();
            settings += ",E_SLIDER_Meteors_Count=" + count;
            settings += vcCount;
            settings += ",E_SLIDER_Meteors_Length=" + length;
            settings += vcLength;
            settings += ",E_CHOICE_Meteors_Effect=" + effect.Capitalize();
            settings += ",E_SLIDER_Meteors_Swirl_Intensity=" + swirl;
            settings += vcSwirl;
            settings += ",E_SLIDER_Meteors_Speed=" + speed;
            settings += vcSpeed;
        } else if (effectType == "movie") {
            // xxx.avi,True,False
            wxString file = parms[0];
            wxString scale = parms[1];
            wxString fullLength = parms[2];

            settings += ",E_FILEPICKERCTRL_Video_Filename=" + file;

            if (scale == "True") {
                settings += ",E_CHECKBOX_Video_AspectRatio=0";
            } else {
                settings += ",E_CHECKBOX_Video_AspectRatio=1";
            }
            if (fullLength == "True") {
                settings += ",E_CHOICE_Video_DurationTreatment=Slow/Accelerate";
            } else {
                settings += ",E_CHOICE_Video_DurationTreatment=Normal";
            }
        } else if (effectType == "picture") {
            // file.jpg,True,none,0,10,19,12
            wxString file = parms[0];
            wxString scale = parms[1];
            wxString movement = parms[2];
            wxString x = parms[3];
            wxString vcCrap;
            x = RescaleWithRangeI(x, "IGNORE", -50, 50, -100, 100, vcCrap, -1, -1);
            wxString peekabooHoldTime = parms[4]; // not used
            peekabooHoldTime = RescaleWithRangeI(peekabooHoldTime, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
            wxString wiggle = parms[5]; // not used
            wiggle = RescaleWithRangeI(wiggle, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
            wxString speed = parms[6];
            speed = RescaleWithRangeF(speed, "IGNORE", 0, 50, 0, 20, vcCrap, -1, -1);

            settings += ",E_TEXTCTRL_Pictures_Filename=" + file;
            if (scale == "True") {
                settings += ",E_CHOICE_Scaling=Scale To Fit";
            } else {
                settings += ",E_CHOICE_Scaling=No Scaling";
            }

            movement.Replace("_", "-");
            if (movement == "peekaboo-bottom") {
                movement = "peekaboo";
            } else if (movement == "peekaboo-top") {
                movement = "peekaboo 180";
            } else if (movement == "peekaboo-left") {
                movement = "peekaboo 90";
            } else if (movement == "peekaboo-right") {
                movement = "peekaboo 270";
            }
            settings += ",E_CHOICE_Pictures_Direction=" + movement;
            settings += ",E_SLIDER_PicturesXC=" + x;
            settings += ",E_TEXTCTRL_Pictures_Speed=" + speed;
        } else if (effectType == "pinwheel") {
            // 3,1,6,color_per_arm,True,12,100,10,-23

            wxString arms = parms[0];
            wxString vcCrap;
            arms = RescaleWithRangeI(arms, "IGNORE", 1, 10, 1, 10, vcCrap, -1, -1);
            wxString width = parms[1];
            wxString vcWidth;
            width = RescaleWithRangeI(width, "E_VALUECURVE_Pinwheel_Thickness", 1, 10, 0, 100, vcWidth, PinwheelEffect::sThicknessMin, PinwheelEffect::sThicknessMax);
            wxString bend = parms[2];
            wxString vcBend;
            bend = RescaleWithRangeI(bend, "E_VALUECURVE_Pinwheel_Twist", -10, 10, -360, 360, vcBend, PinwheelEffect::sTwistMin, PinwheelEffect::sTwistMax);
            //wxString colour = parms[3]; // not used
            wxString CCW = parms[4];
            wxString speed = parms[5];
            wxString vcSpeed;
            speed = RescaleWithRangeI(speed, "E_VALUECURVE_Pinwheel_Speed", 0, 50, 0, 50, vcSpeed, PinwheelEffect::sSpeedMin, PinwheelEffect::sSpeedMax);
            wxString length = parms[6];
            wxString vcLength;
            length = RescaleWithRangeI(length, "E_VALUECURVE_Pinwheel_ArmSize", 1, 100, 0, 400, vcLength, PinwheelEffect::sArmSizeMin, PinwheelEffect::sArmSizeMax);
            wxString x = parms[7];
            wxString vcX;
            x = RescaleWithRangeI(x, "E_VALUECURVE_PinwheelXC", -50, 50, -100, 100, vcX, PinwheelEffect::sXCMin, PinwheelEffect::sXCMax);
            wxString y = parms[8];
            wxString vcY;
            y = RescaleWithRangeI(y, "E_VALUECURVE_PinwheelYC", -50, 50, -100, 100, vcY, PinwheelEffect::sYCMin, PinwheelEffect::sYCMax);

            settings += ",E_SLIDER_Pinwheel_Arms=" + arms;
            settings += ",E_SLIDER_Pinwheel_Thickness=" + width;
            settings += vcWidth;
            settings += ",E_SLIDER_Pinwheel_Twist=" + bend;
            settings += vcBend;
            if (CCW == "True") {
                settings += ",E_CHECKBOX_Pinwheel_Rotation=1";
            } else {
                settings += ",E_CHECKBOX_Pinwheel_Rotation=0";
            }
            settings += ",E_SLIDER_Pinwheel_Speed=" + speed;
            settings += vcSpeed;
            settings += ",E_SLIDER_Pinwheel_ArmSize=" + length;
            settings += vcLength;
            settings += ",E_CHOICE_Pinwheel_Style=New Render Method";
            settings += ",E_SLIDER_PinwheelXC=" + x;
            settings += vcX;
            settings += ",E_SLIDER_PinwheelYC=" + y;
            settings += vcY;
        } else if (effectType == "snowflakes") {
            // 5,1,0,12,60
            wxString count = parms[0];
            wxString vcCount;
            count = RescaleWithRangeI(count, "E_VALUECURVE_Snowflakes_Count", 1, 20, 1, 20, vcCount, SnowflakesEffect::sCountMin, SnowflakesEffect::sCountMax);
            wxString type = parms[1];
            wxString vcCrap;
            type = RescaleWithRangeI(type, "IGNORE", 0, 5, 0, 5, vcCrap, -1, -1);
            wxString direction = parms[2]; // not used
            direction = RescaleWithRangeI(direction, "IGNORE", -8, 8, -8, 8, vcCrap, -1, -1);
            wxString speed = parms[3];
            wxString vcSpeed;
            speed = RescaleWithRangeI(speed, "E_VALUECURVE_Snowflakes_Speed", 0, 50, 0, 50, vcSpeed, SnowflakesEffect::sSpeedMin, SnowflakesEffect::sSpeedMax);
            wxString accumulation = parms[4];
            accumulation = RescaleWithRangeI(accumulation, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);

            settings += ",E_SLIDER_Snowflakes_Count=" + count;
            settings += ",E_SLIDER_Snowflakes_Type=" + type;
            settings += ",E_SLIDER_Snowflakes_Speed=" + speed;
            if (accumulation == "0") {
                settings += ",E_CHOICE_Falling=Falling";
                settings += vcCount;
                settings += vcSpeed;
            } else {
                settings += ",E_CHOICE_Falling=Falling & Accumulating";
                settings += vcCount;
                settings += vcSpeed;
            }
        } else if (effectType == "text") {
            // Hello%26nbsp%3B%20Keith,50,left,0,10,0,4,True
            wxString text = wxURI::Unescape(parms[0]);
            text.Replace("&gt;", ">");
            text.Replace("&lt;", "<");
            text.Replace("&nbsp;", " ");
            text.Replace("&amp;", "&");
            wxString fontSize = parms[1];
            wxString vcCrap;
            fontSize = RescaleWithRangeI(fontSize, "IGNORE", 0, 149, 0, 149, vcCrap, -1, -1);
            wxString movement = parms[2];
            wxString position = parms[3];
            position = RescaleWithRangeI(position, "IGNORE", -50, 49, -200, 200, vcCrap, -1, -1);
            wxString peekabooHoldTime = parms[4]; // unused
            peekabooHoldTime = RescaleWithRangeI(peekabooHoldTime, "IGNORE", 0, 99, 0, 99, vcCrap, -1, -1);
            wxString bounce = parms[5]; // unused
            bounce = RescaleWithRangeI(bounce, "IGNORE", 0, 99, 0, 99, vcCrap, -1, -1);
            wxString speed = parms[6];
            speed = RescaleWithRangeI(speed, "IGNORE", 0, 50, 0, 50, vcCrap, -1, -1);
            if (parms.size() > 7) {
                //wxString unknown1 = parms[7]; // unused
            }

            settings += ",E_TEXTCTRL_Text=" + text;
            settings += ",E_CHOICE_Text_Font=Use OS Fonts";
            settings += ",E_FONTPICKER_Text_Font='segoe ui' " + fontSize;

            if (movement == "peekaboo_bottom") {
                settings += ",E_CHECKBOX_TextToCenter=1";
                movement = "up";
            }
            if (movement == "peekaboo_top") {
                settings += ",E_CHECKBOX_TextToCenter=1";
                movement = "down";
            }
            if (movement == "peekaboo_left") {
                settings += ",E_CHECKBOX_TextToCenter=1";
                movement = "left";
            }
            if (movement == "peekaboo_right") {
                settings += ",E_CHECKBOX_TextToCenter=1";
                movement = "right";
            }
            settings += ",E_CHOICE_Text_Dir=" + movement;
            settings += ",E_SLIDER_Text_XStart=" + position;
            settings += ",E_TEXTCTRL_Text_Speed=" + speed;
        } else if (effectType == "twinkle") {
            // 50,25,twinkle,random
            wxString rate = parms[0];
            wxString vcCrap;
            rate = RescaleWithRangeI(rate, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
            wxString density = parms[1];
            density = RescaleWithRangeI(density, "IGNORE", 0, 100, 0, 100, vcCrap, -1, -1);
            wxString mode = parms[2];
            wxString layout = parms[3];

            settings += ",E_SLIDER_Twinkle_Count=" + density;
            settings += ",E_SLIDER_Twinkle_Steps=" + rate;
            if (layout == "interval") {
                settings += ",E_CHECKBOX_Twinkle_ReRandom=0";
            } else if (layout == "random") {
                settings += ",E_CHECKBOX_Twinkle_ReRandom=1";
            }

            if (mode == "twinkle") {
                settings += ",E_CHECKBOX_Twinkle_Strobe=0";
            } else // pulse/flash
            {
                settings += ",E_CHECKBOX_Twinkle_Strobe=1";
            }

            spdlog::warn("LPE conversion for Twinkle not created yet.");
        } else {
            spdlog::warn("LPE conversion for {} not created yet.", effectType.ToStdString());
            wxASSERT(false);
        }
    }

    return settings;
}

void MapLPE(const EffectManager& effect_manager, int i, EffectLayer* layer, const pugi::xml_document& input_xml, const wxString& model, bool left, int frequency, bool eraseExisting)
{
    if (eraseExisting)
        layer->DeleteAllEffects();

    for (pugi::xml_node e = input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        std::string ename = e.name();
        if (ename == "SequenceProps" || ename == "ArchivedProps") {
            for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
                std::string pname = prop.name();
                if (pname == "SeqProp" || pname == "ArchiveProp") {
                    std::string name = prop.attribute("name").as_string();
                    if (name == "") {
                        for (pugi::xml_node ap = prop.first_child(); ap; ap = ap.next_sibling()) {
                            if (std::string_view(ap.name()) == "PropClass") {
                                name = ap.attribute("Name").as_string();
                            }
                        }
                    }
                    if (name == model) {
                        for (pugi::xml_node track = prop.first_child(); track; track = track.next_sibling()) {
                            int id = track.attribute("id").as_int();
                            if (id == i) {
                                // now to add effects
                                for (pugi::xml_node effect = track.first_child(); effect; effect = effect.next_sibling()) {
                                    wxString type = effect.attribute("type").as_string();

                                    if (std::string_view(effect.name()) != "effect" || type != "pixelEffect") {
                                        spdlog::warn("LPE import node {} type {} not known.", effect.name(), type.ToStdString());
                                    } else {
                                        int startCentisecond = effect.attribute("startCentisecond").as_int();
                                        int endCentisecond = effect.attribute("endCentisecond").as_int();
                                        int startIntensity = effect.attribute("startIntensity").as_int(100);
                                        int endIntensity = effect.attribute("endIntensity").as_int(100);
                                        wxString settings = effect.attribute("pixelEffect").as_string();
                                        wxArrayString settingsArray = wxSplit(settings, '|');
                                        wxString sideSettings;
                                        if (left) {
                                            if (settingsArray.size() == 7) {
                                                sideSettings = settingsArray[5];
                                            } else {
                                                sideSettings = settingsArray[3];
                                            }
                                        } else {
                                            if (settingsArray.size() == 7) {
                                                sideSettings = settingsArray[6];
                                            } else {
                                                sideSettings = settingsArray[4];
                                            }
                                        }
                                        wxArrayString effSettings = wxSplit(sideSettings, ':');
                                        wxString effectType = effSettings[0];
                                        if (effectType == "none") {
                                            // nothing to do
                                        } else {
                                            wxString ourEffectType = MapLPEEffectType(effectType);

                                            if (ourEffectType == "") {
                                                spdlog::warn("LPE import effect {} not known.", effectType.ToStdString());
                                            } else {
                                                // skip over the multiple nodes PE creates when fading isnt perfectly even
                                                int fadeInCS, fadeOutCS;
                                                pugi::xml_node lastnode = FindLastLPEEffectNode(effect, startCentisecond, endCentisecond, endIntensity, endIntensity - startIntensity, settings, fadeInCS, fadeOutCS);
                                                if (lastnode != effect) {
                                                    endCentisecond = lastnode.attribute("endCentisecond").as_int();
                                                    endIntensity = lastnode.attribute("endIntensity").as_int(100);
                                                    effect = lastnode;
                                                }

                                                // only create effect if there is nothing there
                                                if (!layer->HasEffectsInTimeRange(RoundToMultipleOfPeriod(startCentisecond * 10, frequency), RoundToMultipleOfPeriod(endCentisecond * 10, frequency))) {
                                                    wxString blend = MapLPEBlend(settingsArray[0], left);
                                                    int blendPos = wxAtoi(settingsArray[1]);
                                                    int sparkle = wxAtoi(settingsArray[2]);

                                                    // now we need to create the effect
                                                    std::string newpalette = ExtractLPEPallette(effSettings);
                                                    std::string newsettings = "T_CHOICE_LayerMethod=" + blend;

                                                    if (sparkle > 0) {
                                                        newpalette += ",C_SLIDER_SparkleFrequency=" + wxString::Format("%d", sparkle);
                                                    }
                                                    if (left && blendPos > 0) {
                                                        newsettings += ",T_SLIDER_EffectLayerMix=" + wxString::Format("%d", blendPos);
                                                    }

                                                    if (startIntensity == 100 && endIntensity == 100 && fadeInCS == 0 && fadeOutCS == 0) {
                                                        // dont need to do anything
                                                    } else if (startIntensity == endIntensity && fadeInCS == 0 && fadeOutCS == 0) {
                                                        // need to set brightness
                                                        newpalette += ",C_SLIDER_Brightness=" + wxString::Format("%d", startIntensity);
                                                    } else {
                                                        // need to set a brightness value curve
                                                        if (fadeInCS > 0) {
                                                            newsettings += ",T_TEXTCTRL_Fadein=" + wxString::Format("%.2f", (float)(fadeInCS - startCentisecond) / 100.0);
                                                        }
                                                        if (fadeOutCS > 0) {
                                                            newsettings += ",T_TEXTCTRL_Fadeout=" + wxString::Format("%.2f", (float)(endCentisecond - fadeOutCS) / 100.0);
                                                        }

                                                        if (fadeInCS == 0 && fadeOutCS == 0) {
                                                            newpalette += ",C_VALUECURVE_Brightness=Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Ramp|Min=0.00|Max=400.00|P1=" + wxString::Format("%d", startIntensity) + "|P2=" + wxString::Format("%d", endIntensity) + "|RV=TRUE|";
                                                        }
                                                    }

                                                    newsettings += LPEParseEffectSettings(effectType, effSettings, newpalette, (endCentisecond - startCentisecond) * 10);

                                                    layer->AddEffect(0, ourEffectType, newsettings, newpalette, RoundToMultipleOfPeriod(startCentisecond * 10, frequency), RoundToMultipleOfPeriod(endCentisecond * 10, frequency), false, false);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        return;
                    }
                }
            }
        }
    }
}

void MapLPEEffects(const EffectManager& effectManager, Element* model, const pugi::xml_document& input_xml, const wxString& mapping, int frequency, bool eraseExisting)
{
    static 

    int layer = 0;
    if (LPEHasEffects(input_xml, mapping, 0, true)) {
        spdlog::debug("Creating effects on model {} layer {} from {} layer 0 left hand side",
                      model->GetFullName(), layer + 1, mapping.ToStdString());
        MapLPE(effectManager, 0, model->GetEffectLayer(layer), input_xml, mapping, true, frequency, eraseExisting);
    }
    if (LPEHasEffects(input_xml, mapping, 0, false)) {
        layer++;
        if ((int)model->GetEffectLayerCount() < layer + 1) {
            model->AddEffectLayer();
        }
        spdlog::debug("Creating effects on model {} layer {} from {} layer 0 right hand side",
                      model->GetFullName(), layer + 1, mapping.ToStdString());
        MapLPE(effectManager, 0, model->GetEffectLayer(layer), input_xml, mapping, false, frequency, eraseExisting);
    }
    if (LPEHasEffects(input_xml, mapping, 1, true)) {
        layer++;
        if ((int)model->GetEffectLayerCount() < layer + 1) {
            model->AddEffectLayer();
        }
        spdlog::debug("Creating effects on model {} layer {} from {} layer 1 left hand side",
                      model->GetFullName(), layer + 1, mapping.ToStdString());
        MapLPE(effectManager, 1, model->GetEffectLayer(layer), input_xml, mapping, true, frequency, eraseExisting);
    }
    if (LPEHasEffects(input_xml, mapping, 1, false)) {
        layer++;
        if ((int)model->GetEffectLayerCount() < layer + 1) {
            model->AddEffectLayer();
        }
        spdlog::debug("Creating effects on model {} layer {} from {} layer 1 right hand side",
                      model->GetFullName(), layer + 1, mapping.ToStdString());
        MapLPE(effectManager, 1, model->GetEffectLayer(layer), input_xml, mapping, false, frequency, eraseExisting);
    }
}

void MapS5(const EffectManager& effect_manager, int layer, EffectLayer* el, const LOREdit& lorEdit, const wxString& model, Model* m, int frequency, int offset, bool eraseExisting)
{
    if (el == nullptr)
        return;

    if (eraseExisting)
        el->DeleteAllEffects();

    bool channelBlock = (m != nullptr && m->GetDisplayAs() == DisplayAsType::ChannelBlock);

    auto st = lorEdit.GetSequencingType(model);

    if (st == loreditType::CHANNELS) {
        auto effects = lorEdit.GetChannelEffects(model, 0, m, offset);

        for (const auto& it : effects) {
            if (!el->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                // for channel blocks we always use white as the colour comes from the channel block
                if (channelBlock)
                    palette = "C_BUTTON_Palette1=#ffffff,C_CHECKBOX_Palette1=1";
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    el->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        // pixel effects on a node ... not useful but whatever
        auto effects = lorEdit.GetTrackEffects(model, layer, offset);

        for (const auto& it : effects) {
            if (!el->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    el->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    }
}

void MapS5ChannelEffects(const EffectManager& effectManager, int node, EffectLayer* nl, Model* m, const LOREdit& lorEdit, const wxString& mapping, int frequency, int offset, bool eraseExisting)
{
    if (nl == nullptr)
        return;

    if (eraseExisting)
        nl->DeleteAllEffects();

    bool channelBlock = (m != nullptr && m->GetDisplayAs() == DisplayAsType::ChannelBlock);

    auto st = lorEdit.GetSequencingType(mapping);

    if (st == loreditType::CHANNELS) {
        auto effects = lorEdit.GetChannelEffects(mapping, node, m, offset);

        for (const auto& it : effects) {
            if (!nl->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                // for channel blocks we always use white as the colour comes from the channel block
                if (channelBlock)
                    palette = "C_BUTTON_Palette1=#ffffff,C_CHECKBOX_Palette1=1";
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    nl->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        // pixel effects on a node ... not useful but whatever
        auto effects = lorEdit.GetTrackEffects(mapping, 0, offset);

        for (const auto& it : effects) {
            if (!nl->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    nl->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    }
}

void MapS5ChannelEffects(const EffectManager& effectManager, EffectLayer* layer, const LOREdit& lorEdit, const wxString& mapping, int frequency, int offset, bool eraseExisting)
{
    if (eraseExisting)
        layer->DeleteAllEffects();

    Model* m = layer->GetParentElement()->GetSequenceElements()->GetRenderContext()->GetModel(layer->GetParentElement()->GetModelName());
    bool channelBlock = (m != nullptr && m->GetDisplayAs() == DisplayAsType::ChannelBlock);

    static wxRegEx regex("\\[(\\d+),(\\d+),(\\d+)\\]\\[(.*)\\]", wxRE_ADVANCED | wxRE_NEWLINE);
    if (regex.Matches(mapping)) {
        int const row = wxAtoi(regex.GetMatch(mapping, 1).ToStdString());
        int const col = wxAtoi(regex.GetMatch(mapping, 2).ToStdString());
        int const color = wxAtoi(regex.GetMatch(mapping, 3).ToStdString());
        std::string strColor = regex.GetMatch(mapping, 4).ToStdString();
        wxString name = mapping;
        wxString const end = regex.GetMatch(mapping, 0);
        name.Replace(end, "");

        auto effects = lorEdit.GetChannelEffects(name, row, col, color, offset);

        for (auto& it : effects) {
            if (!layer->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                LOREdit::setNodeColor(strColor, it);
                std::string palette = it.GetPalette();
                // for channel blocks we always use white as the colour comes from the channel block
                if (channelBlock)
                    palette = "C_BUTTON_Palette1=#ffffff,C_CHECKBOX_Palette1=1";
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    layer->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    }
}

void MapS5ChannelEffects(const EffectManager& effectManager, int node, EffectLayer* nl, int nodes, const LOREdit& lorEdit, const wxString& mapping, int frequency, int offset, bool eraseExisting)
{
    if (nl == nullptr)
        return;

    if (eraseExisting)
        nl->DeleteAllEffects();

    Model* m = nl->GetParentElement()->GetSequenceElements()->GetRenderContext()->GetModel(nl->GetParentElement()->GetModelName());
    bool channelBlock = (m != nullptr && m->GetDisplayAs() == DisplayAsType::ChannelBlock);

    auto st = lorEdit.GetSequencingType(mapping);

    if (st == loreditType::CHANNELS) {
        auto effects = lorEdit.GetChannelEffects(mapping, node, nodes, offset);

        for (const auto& it : effects) {
            if (!nl->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                // for channel blocks we always use white as the colour comes from the channel block
                if (channelBlock)
                    palette = "C_BUTTON_Palette1=#ffffff,C_CHECKBOX_Palette1=1";
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    nl->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        // pixel effects on a node ... not useful but whatever
        auto effects = lorEdit.GetTrackEffects(mapping, 0, offset);

        for (const auto& it : effects) {
            if (!nl->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    nl->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    }
}

void MapS5Effects(const EffectManager& effectManager, Element* model, const LOREdit& lorEdit, const wxString& mapping, int frequency, int offset, bool eraseExisting)
{
    // static 

    auto st = lorEdit.GetSequencingType(mapping);
    Model* m = model->GetSequenceElements()->GetRenderContext()->GetModel(model->GetModelName());

    if (st == loreditType::CHANNELS) {
        if (m->GetNodeCount() == 1) {
            MapS5ChannelEffects(effectManager, 0, model->GetEffectLayer(0), m, lorEdit, mapping, frequency, offset, eraseExisting);
        } else {
            int lr, lc;
            lorEdit.GetModelChannels(mapping, lr, lc);

            if (lr == 1 && lc == 1) {
                MapS5ChannelEffects(effectManager, 0, model->GetEffectLayer(0), m, lorEdit, mapping, frequency, offset, eraseExisting);
            } else {
                for (uint32_t i = 0; i < m->GetNodeCount(); i++) {
                    NodeLayer* nl = model->GetNodeEffectLayer(i);
                    if (nl != nullptr) {
                        MapS5ChannelEffects(effectManager, i, nl, m, lorEdit, mapping, frequency, offset, eraseExisting);
                    }
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        for (int i = 0; i < lorEdit.GetModelLayers(mapping); i++) {
            if ((int)model->GetEffectLayerCount() < i + 1) {
                model->AddEffectLayer();
            }
            MapS5(effectManager, i, model->GetEffectLayer(i), lorEdit, mapping, m, frequency, offset, eraseExisting);
        }
    }
}

void MapS5Effects(const EffectManager& effectManager, StrandElement* se, const LOREdit& lorEdit, const wxString& mapping, int frequency, int offset, bool eraseExisting)
{
    // static 

    auto st = lorEdit.GetSequencingType(mapping);
    Model* m = se->GetSequenceElements()->GetRenderContext()->GetModel(se->GetModelName());

    if (st == loreditType::CHANNELS) {
        if (se->GetNodeLayerCount() == 1) {
            MapS5ChannelEffects(effectManager, 0, se->GetEffectLayer(0), 1, lorEdit, mapping, frequency, offset, eraseExisting);
        } else {
            int lr, lc;
            lorEdit.GetModelChannels(mapping, lr, lc);

            if (lr == 1 && lc == 1) {
                MapS5ChannelEffects(effectManager, 0, se->GetEffectLayer(0), 1, lorEdit, mapping, frequency, offset, eraseExisting);
            } else {
                int nodes = se->GetNodeLayerCount();
                for (int i = 0; i < nodes; i++) {
                    NodeLayer* nl = se->GetNodeEffectLayer(i);
                    if (nl != nullptr) {
                        MapS5ChannelEffects(effectManager, i, nl, nodes, lorEdit, mapping, frequency, offset, eraseExisting);
                    }
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        for (int i = 0; i < lorEdit.GetModelLayers(mapping); i++) {
            if ((int)se->GetEffectLayerCount() < i + 1) {
                se->AddEffectLayer();
            }
            MapS5(effectManager, i, se->GetEffectLayer(i), lorEdit, mapping, m, frequency, offset, eraseExisting);
        }
    }
}

void MapS5Effects(const EffectManager& effectManager, SubModelElement* se, const LOREdit& lorEdit, const wxString& mapping, int frequency, int offset, bool eraseExisting)
{
    if (dynamic_cast<StrandElement*>(se) != nullptr) {
        return MapS5Effects(effectManager, dynamic_cast<StrandElement*>(se), lorEdit, mapping, frequency, offset, eraseExisting);
    }

    // static 

    auto st = lorEdit.GetSequencingType(mapping);
    Model* m = se->GetSequenceElements()->GetRenderContext()->GetModel(se->GetModelName());

    if (st == loreditType::CHANNELS) {
        if (m->GetNodeCount() == 1) {
            MapS5ChannelEffects(effectManager, 0, se->GetEffectLayer(0), m, lorEdit, mapping, frequency, offset, eraseExisting);
        } else {
            int lr, lc;
            lorEdit.GetModelChannels(mapping, lr, lc);

            if (lr == 1 && lc == 1) {
                MapS5ChannelEffects(effectManager, 0, se->GetEffectLayer(0), m, lorEdit, mapping, frequency, offset, eraseExisting);
            } else {
                for (uint32_t i = 0; i < m->GetNodeCount(); i++) {
                    NodeLayer* nl = se->GetNodeEffectLayer(i);
                    if (nl != nullptr) {
                        MapS5ChannelEffects(effectManager, i, nl, m, lorEdit, mapping, frequency, offset, eraseExisting);
                    }
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        for (int i = 0; i < lorEdit.GetModelLayers(mapping); i++) {
            if ((int)se->GetEffectLayerCount() < i + 1) {
                se->AddEffectLayer();
            }
            MapS5(effectManager, i, se->GetEffectLayer(i), lorEdit, mapping, m, frequency, offset, eraseExisting);
        }
    }
}

bool xLightsFrame::ImportS5(pugi::xml_document& input_xml, const wxFileName& filename)
{
    DisplayWarning(
        "WARNING: As at this release S5 import is experimental and its improvement relies on your feedback.\nIf it doesnt do a good job let us know by telling us:\n\
        - which effect\n\
        - which setting you had to fine tune\n\
        - what it was when it was converted\n\
        - what you changed it to.\n",
        this);

    LOREdit lorEdit(input_xml, CurrentSeqXmlFile->GetFrequency());

    xLightsImportChannelMapDialog dlg(this, filename, true, true, false, true, false);
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;

    dlg.timingTracks = lorEdit.GetTimingTracks();
    for (auto const& m : lorEdit.GetModelsWithEffects()) {
        dlg.AddChannel(m);
    }
    dlg.ccrNames = lorEdit.GetNodesWithEffects();

    dlg.SortChannels();

    dlg.InitImport("Stands and Channels");

    if (dlg.ShowModal() != wxID_OK || dlg._dataModel == nullptr) {
        return false;
    }

    spdlog::debug("Importing S5 effects from {}.", ToStdString(filename.GetFullPath()));

    int offset = dlg.TimeAdjustSpinCtrl->GetValue();

    for (size_t tt = 0; tt < dlg.TimingTrackListBox->GetCount(); ++tt) {
        if (dlg.TimingTrackListBox->IsChecked(tt)) {
            std::string name = dlg.TimingTrackListBox->GetString(tt).ToStdString();

            auto timings = lorEdit.GetTimings(name, offset);

            TimingElement* target = (TimingElement*)_sequenceElements.AddElement(name, "timing", true, true, false, false, false);
            char cnt = '1';
            while (target == nullptr) {
                target = (TimingElement*)_sequenceElements.AddElement(name + "-" + cnt++, "timing", true, true, false, false, false);
            }
            if (target->GetEffectLayerCount() == 0) {
                target->AddEffectLayer();
            }

            EffectLayer* targetLayer = target->GetEffectLayer(0);

            for (auto t : timings) {
                targetLayer->AddEffect(0, "", "", "", t.first, t.second, false, false);
            }
        }
    }

    for (size_t i = 0; i < dlg._dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = dlg._dataModel->GetNthChild(i);
        std::string modelName = m->_model;
        ModelElement* model = nullptr;
        for (size_t x = 0; x < _sequenceElements.GetElementCount(); x++) {
            if (_sequenceElements.GetElement(x)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(x)->GetName()) {
                model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(x));
                break;
            }
        }
        if (model != nullptr) {
            Model* mdl = model->GetSequenceElements()->GetRenderContext()->GetModel(model->GetModelName());

            if (m->_mapping != "") {
                    if (model == nullptr) {
                    model = AddModel(GetModel(modelName), _sequenceElements);
                }
                if (model == nullptr) {
                    spdlog::error("Attempt to add model {} during S5 import failed.", modelName);
                } else {
                    if (!LOREdit::IsNodeStrandMapping(m->_mapping))
                        MapS5Effects(effectManager, model, lorEdit, m->_mapping, CurrentSeqXmlFile->GetFrequency(), offset, dlg.CheckBox_EraseExistingEffects->GetValue());
                    else
                        MapS5ChannelEffects(effectManager, model->GetEffectLayer(0), lorEdit, m->_mapping, CurrentSeqXmlFile->GetFrequency(), offset, dlg.CheckBox_EraseExistingEffects->GetValue());
                }
            }

            int str = 0;
            for (size_t j = 0; j < m->GetChildCount(); j++) {
                xLightsImportModelNode* s = m->GetNthChild(j);

                if ("" != s->_mapping) {
        if (model == nullptr) {
                    model = AddModel(GetModel(modelName), _sequenceElements);
                }
                if (model == nullptr) {
                    spdlog::error("Attempt to add model {} during S5 import failed.", modelName);
                    }
                    else {
                        SubModelElement* ste = model->GetSubModel(str);
                        if (ste != nullptr) {
                            if (!LOREdit::IsNodeStrandMapping(s->_mapping))
                                MapS5Effects(effectManager, ste, lorEdit, s->_mapping, CurrentSeqXmlFile->GetFrequency(), offset, dlg.CheckBox_EraseExistingEffects->GetValue());
                            else
                                MapS5ChannelEffects(effectManager, ste->GetEffectLayer(0), lorEdit, s->_mapping, CurrentSeqXmlFile->GetFrequency(), offset, dlg.CheckBox_EraseExistingEffects->GetValue());
                        }
                    }
                }
                for (size_t n = 0; n < s->GetChildCount(); n++) {
                    xLightsImportModelNode* ns = s->GetNthChild(n);
                    if ("" != ns->_mapping) {
                        if (model == nullptr) {
                            model = AddModel(GetModel(modelName), _sequenceElements);
                        }
                        if (model == nullptr) {
                            spdlog::error("Attempt to add model {} during S5 import failed.", (const char*)modelName.c_str());
                        } else {
                            SubModelElement* ste = model->GetSubModel(str);
                            StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                            if (stre != nullptr) {
                                NodeLayer* nl = stre->GetNodeLayer(n, true);
                                if (nl != nullptr) {
                                    if (LOREdit::IsNodeStrandMapping(s->_mapping)) {
                                        MapS5ChannelEffects(effectManager, nl, lorEdit, s->_mapping, CurrentSeqXmlFile->GetFrequency(), offset, dlg.CheckBox_EraseExistingEffects->GetValue());
                                    } else {
                                        auto st = lorEdit.GetSequencingType(ns->_mapping);
                                        if (st == loreditType::CHANNELS) {
                                            MapS5ChannelEffects(effectManager, i, nl, mdl, lorEdit, ns->_mapping, CurrentSeqXmlFile->GetFrequency(), offset, dlg.CheckBox_EraseExistingEffects->GetValue());
                                        } else if (st == loreditType::TRACKS) {
                                            // no layers so we just map the first
                                            MapS5(effectManager, 0, nl, lorEdit, ns->_mapping, mdl, CurrentSeqXmlFile->GetFrequency(), offset, dlg.CheckBox_EraseExistingEffects->GetValue());
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                str++;
            }
        }
    }

    spdlog::debug("    Importing S5 effects done.");

    return true;
}

bool xLightsFrame::ImportLPE(pugi::xml_document& input_xml, const wxFileName& filename)
{

    DisplayWarning(
        "WARNING: As at this release PixelEditor import is experimental and its improvement relies on your feedback.\nIf it doesnt do a good job let us know by telling us:\n\
        - which effect\n\
        - which setting you had to fine tune\n\
        - what it was when it was converted\n\
        - what you changed it to.\n",
        this);

    xLightsImportChannelMapDialog dlg(this, filename, true, false, false, false, false);
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;
    std::vector<std::string> timingTrackNames;

    for (pugi::xml_node e = input_xml.document_element().first_child(); e; e = e.next_sibling()) {
        std::string ename = e.name();
        if (ename == "SequenceProps" || ename == "ArchivedProps") {
            for (pugi::xml_node prop = e.first_child(); prop; prop = prop.next_sibling()) {
                std::string pname = prop.name();
                if (pname == "SeqProp" || pname == "ArchiveProp") {
                    std::string name = prop.attribute("name").as_string();
                    if (name == "") {
                        for (pugi::xml_node ap = prop.first_child(); ap; ap = ap.next_sibling()) {
                            if (std::string_view(ap.name()) == "PropClass") {
                                name = ap.attribute("Name").as_string();
                            }
                        }
                    }
                    dlg.AddChannel(name);
                    dlg.channelColors[name] = xlBLACK;
                }
            }
        }
    }

    dlg.SortChannels();
    dlg.timingTracks = timingTrackNames;

    dlg.InitImport();

    if (dlg.ShowModal() != wxID_OK || dlg._dataModel == nullptr) {
        return false;
    }

    spdlog::debug("Importing LPE effects from {}.", ToStdString(filename.GetFullPath()));

    if (dlg.TimeAdjustSpinCtrl->GetValue() != 0) {
        int offset = dlg.TimeAdjustSpinCtrl->GetValue();
        SuperStar::AdjustAllTimings(input_xml.document_element(), offset / 10);
    }

    for (size_t i = 0; i < dlg._dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = dlg._dataModel->GetNthChild(i);
        std::string modelName = m->_model;
        ModelElement* model = nullptr;
        for (size_t x = 0; x < _sequenceElements.GetElementCount(); x++) {
            if (_sequenceElements.GetElement(x)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(x)->GetName()) {
                model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(x));
                break;
            }
        }

        if (m->_mapping != "") {
            if (model == nullptr) {
                model = AddModel(GetModel(modelName), _sequenceElements);
            }
            if (model == nullptr) {
                spdlog::error("Attempt to add model {} during LPE import failed.", modelName);
            } else {
                MapLPEEffects(effectManager, model, input_xml, m->_mapping, CurrentSeqXmlFile->GetFrequency(), dlg.CheckBox_EraseExistingEffects->GetValue());
            }
        }

        int str = 0;
        for (size_t j = 0; j < m->GetChildCount(); j++) {
            xLightsImportModelNode* s = m->GetNthChild(j);

            if ("" != s->_mapping) {
                if (model == nullptr) {
                    model = AddModel(GetModel(modelName), _sequenceElements);
                }
                if (model == nullptr) {
                    spdlog::error("Attempt to add model {} during LPE import failed.", modelName);
                } else {
                    SubModelElement* ste = model->GetSubModel(str);
                    if (ste != nullptr) {
                        MapLPEEffects(effectManager, ste, input_xml, s->_mapping, CurrentSeqXmlFile->GetFrequency(), dlg.CheckBox_EraseExistingEffects->GetValue());
                    }
                }
            }
            for (size_t n = 0; n < s->GetChildCount(); n++) {
                xLightsImportModelNode* ns = s->GetNthChild(n);
                if ("" != ns->_mapping) {
                    if (model == nullptr) {
                        model = AddModel(GetModel(modelName), _sequenceElements);
                    }
                    if (model == nullptr) {
                        spdlog::error("Attempt to add model {} during LPE import failed.", modelName);
                    } else {
                        SubModelElement* ste = model->GetSubModel(str);
                        StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                        if (stre != nullptr) {
                            NodeLayer* nl = stre->GetNodeLayer(n, true);
                            if (nl != nullptr) {
                                MapLPE(effectManager, 0, nl, input_xml, ns->_mapping, true, CurrentSeqXmlFile->GetFrequency(), dlg.CheckBox_EraseExistingEffects->GetValue());
                            }
                        }
                    }
                }
            }
            str++;
        }
    }

    spdlog::debug("    Importing LPE effects done.");

    return true;
}

void MapVixen3(Element* model, const Vixen3& vixen, const wxString& modelName, long offset, int frameMS, bool eraseExisting)
{
    if (eraseExisting) {
        for (const auto& it : model->GetEffectLayers()) {
            it->DeleteAllEffects();
        }
    }

    auto effects = vixen.GetEffects(modelName.ToStdString());

    for (const auto& it : effects) {
        long s = Vixen3::ConvertTiming(it.start + offset, frameMS);
        long e = Vixen3::ConvertTiming(it.end + offset, frameMS);

        // Vixen can have multiple effects in one time slot so add layers as needed
        EffectLayer* layer = nullptr;
        for (const auto& it : model->GetEffectLayers()) {
            if (!it->HasEffectsInTimeRange(s, e)) {
                layer = it;
                break;
            }
        }

        if (layer == nullptr)
            layer = model->AddEffectLayer();

        // now we need to create the effect
        std::string newpalette = it.GetPalette();
        std::string newsettings = it.GetSettings();
        std::string type = it.GetXLightsType();
        if (type != "") {
            if (layer->GetParentElement()->GetSequenceElements()->GetEffectManager().GetEffectIndex(type) < 0) {
    spdlog::debug("Vixen 3 import {} -> {} is not a valid effect.", it.type, type);
            } else {
                layer->AddEffect(0, type, newsettings, newpalette, s, e, false, false);
            }
        }
    }
}

void MapVixen3Effects(const EffectManager& effectManager, Element* model, const Vixen3& vixen, const wxString& mapping, long offset, int frameMS, bool eraseExisting)
{

    spdlog::debug("Creating effects on model {} from {}",
                  model->GetFullName(), mapping.ToStdString());
    MapVixen3(model, vixen, mapping, offset, frameMS, eraseExisting);
}

bool xLightsFrame::ImportVixen3(const wxFileName& filename)
{
   wxMessageBox(
        "WARNING: As at this release Vixen3 import is experimental and its improvement relies on your feedback.\nIf it doesnt do a good job let us know by telling us:\n\
        - which effect\n\
        - which setting you had to fine tune\n\
        - what it was when it was converted\n\
        - what you changed it to.\n\n\n\
AT THIS POINT IT JUST BRINGS IN THE EFFECTS. WE MAKE NO EFFORT TO GET THE SETTINGS RIGHT!");

    Vixen3 vixen(ToStdString(filename.GetFullPath()));

    if (!vixen.IsSystemFound()) {
        wxMessageBox("SystemConfig.xml could not be found. Import impossible.");
        return false;
    }

    xLightsImportChannelMapDialog dlg(this, filename, true, true, false, false, false);
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;

    auto models = vixen.GetModelsWithEffects();
    for (auto [model, numEff] : models) {
        auto effects = vixen.GetEffects(model);

        if (effects.front().type == "Data") {
            dlg.timingTracks.push_back(model);
        } else {
            dlg.AddChannel(model, numEff);
        }
    }
    dlg.SortChannels();
    auto timings = vixen.GetTimings();
    for (const auto& it : timings) {
        dlg.timingTracks.push_back(it);
    }

    dlg.InitImport();

    if (dlg.ShowModal() != wxID_OK || dlg._dataModel == nullptr) {
        return false;
    }

    spdlog::debug("Importing Vixen 3 effects from {}.", ToStdString(filename.GetFullPath()));

    int offset = dlg.TimeAdjustSpinCtrl->GetValue();

    for (size_t tt = 0; tt < dlg.TimingTrackListBox->GetCount(); ++tt) {
        if (dlg.TimingTrackListBox->IsChecked(tt)) {
            std::string name = dlg.TimingTrackListBox->GetString(tt).ToStdString();

            if (vixen.GetTimingType(name) == "Phrase") {
                TimingElement* element = AddTimingElement(name);
                EffectLayer* effectLayer = element->GetEffectLayer(0);
                if (effectLayer == nullptr) {
                    effectLayer = element->AddEffectLayer();
                }

                AddVixenMarksToLayer(vixen.GetTimings(name), effectLayer, CurrentSeqXmlFile->GetFrameMS());
                effectLayer = element->AddEffectLayer();
                AddVixenMarksToLayer(vixen.GetRelatedTiming(name, "Word"), effectLayer, CurrentSeqXmlFile->GetFrameMS());
                effectLayer = element->AddEffectLayer();
                AddVixenMarksToLayer(vixen.GetRelatedTiming(name, "Phoneme"), effectLayer, CurrentSeqXmlFile->GetFrameMS());
            } else {
                TimingElement* element = AddTimingElement(name);
                EffectLayer* effectLayer = element->GetEffectLayer(0);
                if (effectLayer == nullptr) {
                    effectLayer = element->AddEffectLayer();
                }

                AddVixenMarksToLayer(vixen.GetTimings(name), effectLayer, CurrentSeqXmlFile->GetFrameMS());
            }
        }
    }

    for (size_t i = 0; i < dlg._dataModel->GetChildCount(); ++i) {
        xLightsImportModelNode* m = dlg._dataModel->GetNthChild(i);
        std::string modelName = m->_model;
        ModelElement* model = nullptr;
        for (size_t x = 0; x < _sequenceElements.GetElementCount(); x++) {
            if (_sequenceElements.GetElement(x)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(x)->GetName()) {
                model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(x));
                break;
            }
        }

        if (m->_mapping != "") {
            if (model == nullptr) {
                model = AddModel(GetModel(modelName), _sequenceElements);
            }
            if (model == nullptr) {
                spdlog::error("Attempt to add model {} during Vixen 3 import failed.", modelName);
            } else {
                MapVixen3Effects(effectManager, model, vixen, m->_mapping, offset, CurrentSeqXmlFile->GetFrameMS(), dlg.CheckBox_EraseExistingEffects->GetValue());
            }
        }

        int str = 0;
        for (size_t j = 0; j < m->GetChildCount(); j++) {
            xLightsImportModelNode* s = m->GetNthChild(j);

            if ("" != s->_mapping) {
                if (model == nullptr) {
                    model = AddModel(GetModel(modelName), _sequenceElements);
                }
                if (model == nullptr) {
                    spdlog::error("Attempt to add model {} during Vixen 3 import failed.", modelName);
                } else {
                    SubModelElement* ste = model->GetSubModel(str);
                    if (ste != nullptr) {
                        MapVixen3Effects(effectManager, ste, vixen, s->_mapping, offset, CurrentSeqXmlFile->GetFrameMS(), dlg.CheckBox_EraseExistingEffects->GetValue());
                    }
                }
            }
            for (size_t n = 0; n < s->GetChildCount(); n++) {
                xLightsImportModelNode* ns = s->GetNthChild(n);
                if ("" != ns->_mapping) {
                    if (model == nullptr) {
                        model = AddModel(GetModel(modelName), _sequenceElements);
                    }
                    if (model == nullptr) {
                        spdlog::error("Attempt to add model {} during Vixen 3 import failed.", modelName);
                    } else {
                        SubModelElement* ste = model->GetSubModel(str);
                        StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                        if (stre != nullptr) {
                            NodeLayer* nl = stre->GetNodeLayer(n, true);
                            if (nl != nullptr) {
                                MapVixen3(model, vixen, ns->_mapping, offset, CurrentSeqXmlFile->GetFrameMS(), dlg.CheckBox_EraseExistingEffects->GetValue());
                            }
                        }
                    }
                }
            }
            str++;
        }
    }

    spdlog::debug("    Importing Vixen 3 effects done.");

    return true;
}

void AddLSPEffect(EffectLayer* layer, int pos, int epos, int in, int out, int eff, const wxColor& c, int bst, int ben)
{
    if (eff == 4) {
        // off
        return;
    }
    xlColor color(c.Red(), c.Green(), c.Blue());
    xlColor color2(xlBLACK);

    bool isShimmer = eff == 5 || eff == 6;
    std::string effect = "On";
    std::string settings = wxString::Format("E_TEXTCTRL_Eff_On_End=%d,E_TEXTCTRL_Eff_On_Start=%d", out, in).ToStdString();
    if (isShimmer) {
        settings += ",E_CHECKBOX_On_Shimmer=1";
    }

    if (bst != 0 && ben != 0) {
        color = xlColor(bst & 0xFFFFFF, false);
        color2 = xlColor(ben & 0xFFFFFF, false);
        if (color == color2) {
            color2 = xlBLACK;
        } else {
            effect = "Color Wash";
            settings = isShimmer ? "E_CHECKBOX_ColorWash_Shimmer=1" : "";
        }
    }

    if (xlBLACK == color && xlBLACK == color2) {
        // nutcracker or other effects imported into LSP generate "BLACK" effects in the sequence.  Don't import them.
        return;
    }

    std::string palette = "C_BUTTON_Palette1=" + (std::string)color + ",C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=" + (std::string)color2 + ",C_CHECKBOX_Palette2=1";

    int start_time = (int)(pos * 50.0 / 4410.0);
    int end_time = (int)((epos - 1) * 50.0 / 4410.0);
    layer->AddEffect(0, effect, settings, palette, start_time, end_time, false, false);
}

void MapLSPEffects(EffectLayer* layer, pugi::xml_node node, const wxColor& c)
{
    if (!node) {
        return;
    }
    int eff = -1;
    int in = 1, out = 1, pos = 1;

    int bst = 0, ben = 0;

    for (pugi::xml_node cnd = node.first_child(); cnd; cnd = cnd.next_sibling()) {
        if (std::string_view(cnd.name()) == "Tracks") {
            for (pugi::xml_node cnnd = cnd.first_child(); cnnd; cnnd = cnnd.next_sibling()) {
                if (std::string_view(cnnd.name()) == "Track") {
                    for (pugi::xml_node ind = cnnd.first_child(); ind; ind = ind.next_sibling()) {
                        if (std::string_view(ind.name()) == "Intervals") {
                            for (pugi::xml_node ti = ind.first_child(); ti; ti = ti.next_sibling()) {
                                if (std::string_view(ti.name()) == "TimeInterval") {
                                    int neff = ti.attribute("eff").as_int(4);
                                    if (eff != -1 && neff != 7) {
                                        int npos = ti.attribute("pos").as_int(1);
                                        AddLSPEffect(layer, pos, npos, in, out, eff, c, bst, ben);
                                    }
                                    if (neff != 7) {
                                        pos = ti.attribute("pos").as_int(1);
                                        eff = neff;
                                        in = ti.attribute("in").as_int(1);
                                        out = ti.attribute("out").as_int(1);
                                        bst = ti.attribute("bst").as_int(0);
                                        ben = ti.attribute("ben").as_int(0);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void MapLSPStrand(StrandElement* layer, pugi::xml_node node, const wxColor& c)
{
    int nodeNum = 0;
    for (pugi::xml_node nd = node.first_child(); nd; nd = nd.next_sibling()) {
        if (std::string_view(nd.name()) == "Channels") {
            for (pugi::xml_node cnd = nd.first_child(); cnd; cnd = cnd.next_sibling()) {
                if (std::string_view(cnd.name()) == "Channel") {
                    EffectLayer* el = layer->GetNodeLayer(nodeNum, true);
                    MapLSPEffects(el, cnd, c);
                    nodeNum++;
                    if (nodeNum >= layer->GetNodeLayerCount()) {
                        return;
                    }
                }
            }
        }
    }
}

void xLightsFrame::ImportLSP(const wxFileName& filename)
{
    static 
    wxStopWatch sw; // start a stopwatch timer

    LMSImportChannelMapDialog dlg(this, filename);
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;

    wxFileName msq_file(filename);
    wxString msq_doc = msq_file.GetFullPath();
    wxFileInputStream fin(msq_doc);
    wxZipInputStream zin(fin);
    wxZipEntry* ent = zin.GetNextEntry();

    pugi::xml_document seq_xml;
    std::map<wxString, pugi::xml_document> cont_xml;
    std::map<wxString, pugi::xml_node> nodes;
    std::map<wxString, pugi::xml_node> strandNodes;

    while (ent != nullptr) {
        // Read zip entry into buffer for pugixml
        std::vector<char> xmlBuffer;
        {
            char chunk[4096];
            while (!zin.Eof()) {
                zin.Read(chunk, sizeof(chunk));
                size_t bytesRead = zin.LastRead();
                if (bytesRead == 0) break;
                xmlBuffer.insert(xmlBuffer.end(), chunk, chunk + bytesRead);
            }
        }

        if (ent->GetName() == "Sequence") {
            seq_xml.load_buffer(xmlBuffer.data(), xmlBuffer.size());
        } else {
            std::string id("1");
            pugi::xml_document& doc = cont_xml[ent->GetName()];

            if (doc.load_buffer(xmlBuffer.data(), xmlBuffer.size())) {
                pugi::xml_node root = doc.document_element();
                for (pugi::xml_node nd = root.first_child(); nd; nd = nd.next_sibling()) {
                    if (std::string_view(nd.name()) == "ControllerName") {
                        id = nd.text().get();
                    }
                }
                strandNodes[id] = root;
                dlg.ccrNames.push_back(id);
                for (pugi::xml_node nd = root.first_child(); nd; nd = nd.next_sibling()) {
                    if (std::string_view(nd.name()) == "Channels") {
                        for (pugi::xml_node cnd = nd.first_child(); cnd; cnd = cnd.next_sibling()) {
                            if (std::string_view(cnd.name()) == "Channel") {
                                std::string cname;
                                for (pugi::xml_node cnnd = cnd.first_child(); cnnd; cnnd = cnnd.next_sibling()) {
                                    if (std::string_view(cnnd.name()) == "Tracks") {
                                        for (pugi::xml_node tnd = cnnd.first_child(); tnd; tnd = tnd.next_sibling()) {
                                            if (std::string_view(tnd.name()) == "Track") {
                                                for (pugi::xml_node tnd2 = tnd.first_child(); tnd2; tnd2 = tnd2.next_sibling()) {
                                                    if (std::string_view(tnd2.name()) == "Name") {
                                                        cname = tnd2.text().get();
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                cname = id + "/" + cname;
                                nodes[cname] = cnd;
                                dlg.channelNames.push_back(cname);
                                dlg.channelColors[cname] = xlWHITE;
                            }
                        }
                    }
                }
            } else {
                spdlog::warn("Could not parse XML file {}.", ent->GetName().ToStdString());
                wxLogError("Could not parse XML file %s", ent->GetName().c_str());
            }
        }
        ent = zin.GetNextEntry();
    }

    std::sort(dlg.channelNames.begin(), dlg.channelNames.end(), stdlistNumberAwareStringCompare);
    dlg.channelNames.insert(dlg.channelNames.begin(), "");
    std::sort(dlg.ccrNames.begin(), dlg.ccrNames.end(), stdlistNumberAwareStringCompare);
    dlg.ccrNames.insert(dlg.ccrNames.begin(), "");

    dlg.Init();

    if (dlg.ShowModal() != wxID_OK) {
        return;
    }

    int row = 0;
    for (size_t m = 0; m < dlg.modelNames.size(); m++) {
        std::string modelName = dlg.modelNames[m];
        Model* mc = GetModel(modelName);
        ModelElement* model = nullptr;
        for (size_t i = 0; i < _sequenceElements.GetElementCount(); i++) {
            if (_sequenceElements.GetElement(i)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(i)->GetName()) {
                model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(i));
            }
        }
        if (dlg.ChannelMapGrid->GetCellValue(row, 3) != "" && !dlg.MapByStrand->IsChecked()) {
            MapLSPEffects(model->GetEffectLayer(0), nodes[dlg.ChannelMapGrid->GetCellValue(row, 3)],
                          dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4));
        }
        row++;

        if (!dlg.MapByStrand->IsChecked()) {
            for (int str = 0; str < mc->GetNumSubModels(); str++) {
                SubModelElement* se = model->GetSubModel(str);

                if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                    MapLSPEffects(se->GetEffectLayer(0), nodes[dlg.ChannelMapGrid->GetCellValue(row, 3)],
                                  dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4));
                }
                row++;
            }
        }
        for (int str = 0; str < mc->GetNumStrands(); str++) {
            StrandElement* se = model->GetStrand(str, true);

            if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                if (dlg.MapByStrand->IsChecked()) {
                    MapLSPStrand(se, strandNodes[dlg.ChannelMapGrid->GetCellValue(row, 3)],
                                 dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4));
                } else {
                    MapLSPEffects(se->GetEffectLayer(0), nodes[dlg.ChannelMapGrid->GetCellValue(row, 3)],
                                  dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4));
                }
            }
            row++;
            if (!dlg.MapByStrand->IsChecked()) {
                for (int n = 0; n < mc->GetStrandLength(str); n++) {
                    if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                        NodeLayer* nl = se->GetNodeLayer(n, true);
                        MapLSPEffects(nl, nodes[dlg.ChannelMapGrid->GetCellValue(row, 3)],
                                      dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4));
                    }
                    row++;
                }
            }
        }
    }

    float elapsedTime = sw.Time() / 1000.0; // msec => sec
    SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}

static void ImportServoData(int min_limit, int max_limit, EffectLayer* layer, std::string name,
                            const std::vector<VSAFile::vsaEventRecord>& events, int sequence_end_time, uint32_t timing, bool is_16bit = true)
{
    spdlog::debug("Importing servo data for " + name);

    if (min_limit == max_limit) {
        spdlog::error("ImportServoData cannot have min limit and max limit equal. Aborting import as it would crash.");
        return;
    }
    if (layer == nullptr) {
        spdlog::critical("ImportServoData cannot have null layer to import onto - this is going to crash.");
    }

    if (layer->GetLayerName().empty()) {
        layer->SetLayerName(name);
    }
    
    float last_pos = -1.0;
    int last_time = 0;
    bool warn = true;

    for (int i = 0; i < (int)events.size(); ++i) {
        std::string palette = "C_BUTTON_Palette1=#FFFFFF,C_CHECKBOX_Palette1=1";
        std::string settings;
        if (is_16bit) {
            settings += "E_CHECKBOX_16bit=1,";
        } else {
            settings += "E_CHECKBOX_16bit=0,";
        }
        settings += "E_CHOICE_Channel=" + name + ",";
        float start_pos = (((float)events[i].start_pos - (float)min_limit) * 100.0) / ((float)max_limit - (float)min_limit);
        float end_pos = (((float)events[i].end_pos - (float)min_limit) * 100.0) / ((float)max_limit - (float)min_limit);

        settings += "E_TEXTCTRL_Servo=" + wxString::Format("%3.1f", start_pos).ToStdString() + ",";
        settings += "E_TEXTCTRL_EndValue=" + wxString::Format("%3.1f", end_pos).ToStdString() + ",";

        if (start_pos < 0.0) {
            if (warn) {
                DisplayWarning(wxString::Format("%s: Servo Limit Exceeded. start_pos < 0%% : %.2f min/max %d/%d", name, start_pos, min_limit, max_limit).ToStdString());
                warn = false;
            }
            start_pos = 0.0;
        } else if (start_pos > 100.0) {
            if (warn) {
                DisplayWarning(wxString::Format("%s: Servo Limit Exceeded. start_pos > 100%% : %.2f min/max %d/%d", name, start_pos, min_limit, max_limit).ToStdString());
                warn = false;
            }
            start_pos = 100.0;
        }
        if (end_pos > 100.0) {
            if (warn) {
                DisplayWarning(wxString::Format("%s: Servo Limit Exceeded. end_pos > 100%% : %.2f min/max %d/%d", name, end_pos, min_limit, max_limit).ToStdString());
                warn = false;
            }
            end_pos = 100.0;
        } else if (end_pos < 0.0) {
            if (warn) {
                DisplayWarning(wxString::Format("%s: Servo Limit Exceeded. end_pos < 0%% : %.2f min/max %d/%d", name, end_pos, min_limit, max_limit).ToStdString());
                warn = false;
            }
            end_pos = 0.0;
        }
        if (last_pos == -1.0) {
            last_pos = start_pos;
        }
        if (events[i].start_time > 0) {
            std::string settings2;
            if (is_16bit) {
                settings2 += "E_CHECKBOX_16bit=1,";
            } else {
                settings2 += "E_CHECKBOX_16bit=0,";
            }
            settings2 += "E_CHOICE_Channel=" + name + ",";
            settings2 += "E_TEXTCTRL_Servo=" + wxString::Format("%3.1f", last_pos).ToStdString() + ",";
            settings2 += "E_TEXTCTRL_EndValue=" + wxString::Format("%3.1f", last_pos).ToStdString() + ",";
            layer->AddEffect(0, "Servo", settings2, palette, last_time, events[i].start_time * timing, false, false);
        }
        layer->AddEffect(0, "Servo", settings, palette, events[i].start_time * timing, events[i].end_time * timing, false, false);
        last_pos = end_pos;
        last_time = events[i].end_time * timing;

        // check for filling to end of sequence
        if (i == (int)events.size() - 1) {
            if (last_time < sequence_end_time) {
                std::string settings3;
                if (is_16bit) {
                    settings3 += "E_CHECKBOX_16bit=1,";
                } else {
                    settings3 += "E_CHECKBOX_16bit=0,";
                }
                settings3 += "E_CHOICE_Channel=" + name + ",";
                settings3 += "E_TEXTCTRL_Servo=" + wxString::Format("%3.1f", last_pos).ToStdString() + ",";
                settings3 += "E_TEXTCTRL_EndValue=" + wxString::Format("%3.1f", last_pos).ToStdString() + ",";
                layer->AddEffect(0, "Servo", settings3, palette, last_time, sequence_end_time, false, false);
            }
        }
    }
    spdlog::debug("Importing servo data done.");
}

void xLightsFrame::ImportVsa(const wxFileName& filename)
{
    wxStopWatch sw; // start a stopwatch timer

    VsaImportDialog dlg(this);
    VSAFile vsa(ToStdString(filename.GetFullPath()));
    dlg.mSequenceElements = &_sequenceElements;
    dlg.xlights = this;
    dlg.Init(&vsa, false);

    if (dlg.ShowModal() == wxID_CANCEL) {
        return;
    }

    const std::vector<VSAFile::vsaTrackRecord>& tracks = vsa.GetTrackInfo();
    const std::vector<std::vector<VSAFile::vsaEventRecord>>& events = vsa.GetEventInfo();
    const uint32_t vsa_timing = vsa.GetTiming();

    for (int m = 0; m < (int)dlg.selectedModels.size(); ++m) {
        std::string modelName = dlg.selectedModels[m];
        if (modelName != "") {
            ModelElement* model = nullptr;
            for (size_t i = 0; i < _sequenceElements.GetElementCount(); i++) {
                if (_sequenceElements.GetElement(i)->GetType() == ElementType::ELEMENT_TYPE_MODEL && modelName == _sequenceElements.GetElement(i)->GetName()) {
                    model = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(i));
                    break;
                }
            }

            if (model != nullptr) {
                EffectLayer* layer;
                int layer_number = dlg.selectedLayers[m];
                while ((int)model->GetEffectLayerCount() < layer_number + 1) {
                    model->AddEffectLayer();
                }

                layer = model->GetEffectLayer(layer_number);
                if (layer != nullptr && dlg.selectedChannels[m] != "") {
                    if (m < (int)dlg.trackIndex.size()) {
                        bool is_16bit = true;
                        int idx = dlg.trackIndex[m];
                        if (idx < (int)events.size()) {
                            switch ((VSAFile::vsaControllers)(tracks[idx].controller)) {
                            case VSAFile::MINISSC_SERVO:
                            case VSAFile::DMX_DIMMER:
                            case VSAFile::DMX_RELAY:
                                is_16bit = false;
                            default:
                                break;
                            }
                            ImportServoData(tracks[idx].min_limit, tracks[idx].max_limit, layer, dlg.selectedChannels[m], events[idx], _sequenceElements.GetSequenceEnd(), vsa_timing, is_16bit);
                        } else {
                            spdlog::error("ImportVSA: idx exceeds available events.");
                        }
                    } else {
                        spdlog::error("ImportVSA: m exceeds available tracks.");
                    }
                }
            }
        }
    }

    float elapsedTime = sw.Time() / 1000.0; // msec => sec
    SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}

void xLightsFrame::CloneXLightsEffects(EffectLayer* target, EffectLayer* src, bool eraseExisting)
{
    if (eraseExisting) {
        target->DeleteAllEffects();
    }

    for (int x = 0; x < src->GetEffectCount(); x++) {
        Effect* ef = src->GetEffect(x);
        if (!target->HasEffectsInTimeRange(ef->GetStartTimeMS(), ef->GetEndTimeMS())) {
            std::string settings = ef->GetSettingsAsString();

            // remove lock if it is there
            Replace(settings, ",X_Effect_Locked=True", "");
            target->AddEffect(0, ef->GetEffectName(), settings, ef->GetPaletteAsString(),
                              ef->GetStartTimeMS(), ef->GetEndTimeMS(), 0, false);
        }
    }
}

bool xLightsFrame::CloneXLightsEffects(const std::string& target,
                                       const std::string& source,
                                       SequenceElements& seqEl,
                                       bool eraseExisting)
{
    static 

    Element* from = seqEl.GetElement(source);
    Element* to = seqEl.GetElement(target);

    if (from == nullptr || to == nullptr) {
        spdlog::debug("Mapping xLights effect from {} to {} failed as the effect was not found in the source sequence.", source, target);
        // printf("Source element %s doesn't exist\n", name.c_str());
        return false;
    }
    _sequenceElements.get_undo_mgr().CreateUndoStep();
    while (to->GetEffectLayerCount() < from->GetEffectLayerCount()) {
        to->AddEffectLayer();
    }
    for (size_t x = 0; x < from->GetEffectLayerCount(); x++) {
        CloneXLightsEffects(to->GetEffectLayer(x), from->GetEffectLayer(x), eraseExisting);
    }
    return true;
}
