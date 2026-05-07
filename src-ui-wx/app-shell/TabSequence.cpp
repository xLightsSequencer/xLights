/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/stopwatch.h>
#include <wx/utils.h>
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include "settings/XLightsConfigAdapter.h"
#include <wx/wfstream.h>
#include <wx/sstream.h>

#include <pugixml.hpp>

#include "xLightsMain.h"
#include "media/JukeboxPanel.h"
#include "sequencer/SeqSettingsDialog.h"
#include "render/SequenceFile.h"
#include "effects/RenderableEffect.h"
#include "models/ModelGroup.h"
#include "models/SubModel.h"
#include "render/SequenceViewManager.h"
#include "layout/LayoutPanel.h"
#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"
#include "utils/ExternalHooks.h"
#include "sequencer/BufferPanel.h"
#include "sequencer/EffectIconPanel.h"
#include "media/JukeboxPanel.h"
#include "diagnostics/FindDataPanel.h"
#include "sequencer/EffectsPanel.h"
#include "sequencer/BlendingPanel.h"
#include "color/ColorPanel.h"
#include "layout/LayoutGroup.h"
#include "layout/ModelPreview.h"
#include "layout/ViewsModelsPanel.h"
#include "sequencer/PerspectivesPanel.h"
#include "shared/controls/ValueCurvesPanel.h"
#include "color/ColoursPanel.h"
#include "sequencer/MainSequencer.h"
#include "layout/HousePreviewPanel.h"
#include "utils/ExternalHooks.h"
#include "XmlSerializer/XmlSerializer.h"
#include "XmlSerializer/StringSerializingVisitor.h"

#include "xLightsVersion.h"
#include "sequencer/TopEffectsPanel.h"

#include <log.h>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

static void LogMemoryUsage(const std::string& label)
{
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        spdlog::debug("Memory [{}]: WorkingSet={}MB, Private={}MB",
            label,
            pmc.WorkingSetSize / (1024 * 1024),
            pmc.PrivateUsage / (1024 * 1024));
    }
#endif
}

void xLightsFrame::DisplayXlightsFilename(const wxString& filename) const
{
    xlightsFilename = filename;
    FileNameText->SetLabel(filename);
}

std::string xLightsFrame::BuildEffectsXml()
{
    XmlSerializer serializer;
    StringSerializingVisitor visitor;
    visitor.WriteOpenTag("xrgb");
    serializer.SerializeAllModels(AllModels, visitor);
    serializer.SerializeAllObjects(AllObjects, visitor);
    std::vector<LayoutGroupData> lgData;
    lgData.reserve(LayoutGroups.size());
    for (const auto& [name, lg] : LayoutGroups) {
        lgData.push_back({lg->GetName(), lg->GetBackgroundImage(),
                          lg->GetBackgroundBrightness(), lg->GetBackgroundAlpha(),
                          lg->GetBackgroundScaled(),
                          lg->GetPosX(), lg->GetPosY(),
                          lg->GetPaneWidth(), lg->GetPaneHeight()});
    }
    serializer.SerializeAllLayoutGroups(lgData, visitor);
    SerializePerspectives(visitor);
    SerializeSettings(visitor);
    _sequenceViewManager.Save(visitor);
    color_mgr.Save(visitor);
    viewpoint_mgr.Save(visitor);
    visitor.WriteCloseTag();
    
    return visitor.GetResult();
}

void xLightsFrame::SerializeSettings(BaseSerializingVisitor &visitor)
{
    visitor.WriteOpenTag("settings");
    for (const auto& kv : _xmlSettings) {
        BaseSerializingVisitor::AttrCollector attr;
        attr.Add("value", kv.second);
        visitor.WriteOpenTag(kv.first, attr, true);
    }
    visitor.WriteCloseTag();
}

void xLightsFrame::SerializePerspectives(BaseSerializingVisitor &visitor)
{
    BaseSerializingVisitor::AttrCollector attr;
    attr.Add("current", _currentPerspectiveName);
    visitor.WriteOpenTag("perspectives", attr);
    for (const auto& p : _perspectives) {
        BaseSerializingVisitor::AttrCollector attr;
        attr.Add("name", p.name);
        attr.Add("settings", p.settings);
        attr.Add("version", p.version.empty() ? "2.0" : p.version);
        visitor.WriteOpenTag("perspective", attr, true);
    }
    visitor.WriteCloseTag();
}

void xLightsFrame::OnBitmapButtonOpenSeqClick(wxCommandEvent& event)
{
    if (readOnlyMode) {
        DisplayError("Sequences cannot be opened in read only mode!", this);
        return;
    }
    OpenSequence("", nullptr);
}

void xLightsFrame::OnButtonNewSequenceClick(wxCommandEvent& event)
{
    NewSequence();
	EnableSequenceControls(true);
}

void xLightsFrame::ResetEffectsXml()
{
    AllModels.clear();
    AllObjects.clear();
    _sequenceViewManager.Reset();
    _effectPresetManager.Reset();
    _xmlSettings.clear();
}

void xLightsFrame::LoadEffectsFile()
{
    wxStopWatch sw; // start a stopwatch timer

    
    ResetEffectsXml();
    wxFileName effectsFile;
    effectsFile.AssignDir(CurrentDir);
    effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
    UnsavedRgbEffectsChanges = false;

    pugi::xml_document effectsXml;

    if (!FileExists(effectsFile)) {
        // file does not exist, so create an empty xml doc
        CreateDefaultEffectsXml(effectsXml);
    }
    else {
        // check if there is a autosave backup file which is newer than the file we have been asked to open
        wxFileName fn(effectsFile.GetFullPath());
        wxFileName xx = fn;
        xx.SetExt("xbkp");
        wxString asfile = xx.GetLongPath();

        if (!_renderMode && !_checkSequenceMode && FileExists(asfile)) {
            // the autosave file exists
            wxDateTime xmltime = fn.GetModificationTime();
            wxFileName asfn(asfile);
            wxDateTime xbkptime = asfn.GetModificationTime();

            if (xbkptime > xmltime) {
                // autosave file is newer
                wxString xmlTimeStr = xmltime.Format("%Y-%m-%d %H:%M:%S");
                wxString backupTimeStr = xbkptime.Format("%Y-%m-%d %H:%M:%S");

                // Build the message with both timestamps
                wxString msg = wxString::Format(
                    "An autosaved rgbeffects file was found that is newer than your current file.\n\n"
                    "Current file:  %s\n"
                    "Autosave file: %s\n\n"
                    "Would you like to use the autosave file instead?",
                    xmlTimeStr, backupTimeStr
                );
                if (wxMessageBox(msg, "Newer Autosave File Found", wxYES_NO | wxICON_QUESTION) == wxYES) {
                    // run a backup ... equivalent of a F10

                    // we have not actually read the backup location yet so lets just use the show folder
                    _backupDirectory = showDirectory;
                    DoBackup(false, false, true);
                    _backupDirectory = "";

                    // delete the old xml file
                    wxRemoveFile(effectsFile.GetFullPath());

                    // rename the autosave file
                    wxRenameFile(asfile, effectsFile.GetFullPath());
                }
                else {
                    if (FileExists(fn.GetFullPath())) {
                        //set the backup to be older than the XML files to avoid re-promting
                        xmltime -= wxTimeSpan(0, 0, 3, 0);  //subtract 2 seconds as FAT time resolution is 2 seconds
                        asfn.SetTimes(&xmltime, &xmltime, &xmltime);
                    }
                }
            }
        }
        pugi::xml_parse_result result = effectsXml.load_file(effectsFile.GetFullPath().ToStdString().c_str());
        if (!result) {
            DisplayError(wxString::Format("Unable to load RGB effects File ... Creating a Default One.\nError at offset: %td Error '%s'", result.offset, result.description()), this);
            CreateDefaultEffectsXml(effectsXml);
        }
    }

    pugi::xml_node root = effectsXml.document_element();
    if (std::string_view(root.name()) != "xrgb") {
        DisplayError("Invalid RGB effects file ... creating a default one.", this);
        CreateDefaultEffectsXml(effectsXml);
        root = effectsXml.document_element();
    }


    pugi::xml_node modelsNode = root.child("models");
    pugi::xml_node viewObjectsNode = root.child("view_objects");
    pugi::xml_node modelGroupsNode = root.child("modelGroups");
    pugi::xml_node effectsNode = root.child("effects");
    pugi::xml_node viewsNode = root.child("views");
    pugi::xml_node colorsNode = root.child("colors");
    pugi::xml_node viewpointsNode = root.child("Viewpoints");
    pugi::xml_node layoutGroupsNode = root.child("layoutGroups");
    pugi::xml_node perspectivesNode = root.child("perspectives");
    pugi::xml_node settingsNode = root.child("settings");
    if (settingsNode) {
        for (pugi::xml_node s = settingsNode.first_child(); s; s = s.next_sibling()) {
            _xmlSettings[s.name()] = s.attribute("value").as_string();
        }
    }

    // If this file was last saved by an xLights version before 2026.04, make a versioned backup
    // before we potentially migrate anything. String comparison works because versions are "YYYY.MM.x".
    std::string savedVersion = GetXmlSetting("xlightsVersion", "");
    if (!IsReadOnlyMode() && (savedVersion.empty() || savedVersion < "2026.04")) {
        wxFileName backupFn;
        backupFn.AssignDir(CurrentDir);
        if (savedVersion.empty()) {
            savedVersion = "pre2026.04";
        }
        backupFn.SetFullName(wxString::Format("xlights_rgbeffects_%s.xml", savedVersion));
        wxCopyFile(effectsFile.GetFullPath(), backupFn.GetFullPath(), true);
        spdlog::info("Backed up rgbeffects file to {} (was saved by version {})", backupFn.GetFullPath().ToStdString(), savedVersion);
    }

    // This is the earliest we can do the backup as now the settings node will be populated
    _backupDirectory = GetXmlSetting("backupDir", showDirectory);
    ObtainAccessToURL(_backupDirectory);
    if (!wxDir::Exists(_backupDirectory)) {
        spdlog::warn("Backup Directory not Found ... switching to Show Directory.");
        _backupDirectory = showDirectory;
        SetXmlSetting("backupDir", showDirectory);
        UnsavedRgbEffectsChanges = true;
    }
    
    if (_xmlSettings.empty()) {
        SetXmlSetting("previewWidth", "1280");
        SetXmlSetting("previewHeight", "720");
        SetXmlSetting("LayoutMode3D", "0");
        UnsavedRgbEffectsChanges = true;
    }
    
    // Load presets: try separate JSON file first, fall back to XML for migration
    {
        wxFileName presetsFile;
        presetsFile.AssignDir(CurrentDir);
        presetsFile.SetFullName(_(XLIGHTS_PRESETS_FILE));

        // Check for autosave backup of presets file
        wxFileName presetsBkp(presetsFile);
        presetsBkp.SetFullName(_(XLIGHTS_PRESETS_FILE_BACKUP));
        if (!_renderMode && !_checkSequenceMode && FileExists(presetsBkp.GetFullPath())) {
            if (FileExists(presetsFile.GetFullPath())) {
                wxDateTime jsonTime = presetsFile.GetModificationTime();
                wxDateTime bkpTime = presetsBkp.GetModificationTime();
                if (bkpTime > jsonTime) {
                    wxString msg = wxString::Format(
                        "An autosaved effect presets file was found that is newer than your current file.\n\n"
                        "Current file:  %s\n"
                        "Autosave file: %s\n\n"
                        "Would you like to use the autosave file instead?",
                        jsonTime.Format("%Y-%m-%d %H:%M:%S"), bkpTime.Format("%Y-%m-%d %H:%M:%S"));
                    if (wxMessageBox(msg, "Newer Autosave Presets File Found", wxYES_NO | wxICON_QUESTION) == wxYES) {
                        wxRemoveFile(presetsFile.GetFullPath());
                        wxRenameFile(presetsBkp.GetFullPath(), presetsFile.GetFullPath());
                    } else {
                        wxDateTime older = jsonTime;
                        older -= wxTimeSpan(0, 0, 3, 0);
                        presetsBkp.SetTimes(&older, &older, &older);
                    }
                }
            } else {
                // No main file but backup exists — use the backup
                wxRenameFile(presetsBkp.GetFullPath(), presetsFile.GetFullPath());
            }
        }

        if (_effectPresetManager.LoadJsonFile(presetsFile.GetFullPath().ToStdString())) {
            spdlog::debug("Loaded effect presets from {}", presetsFile.GetFullPath().ToStdString());
        } else {
            // Migrate from XML effects node
            _effectPresetManager.Load(effectsNode);
            spdlog::info("Migrated effect presets from xlights_rgbeffects.xml to JSON");
            UnsavedRgbEffectsChanges = true; // trigger save to create the new JSON file
        }
    }
    if (_effectPresetManager.GetVersion().empty()) {
        _effectPresetManager.SetVersion(XLIGHTS_RGBEFFECTS_VERSION);
    }
    // check version, do we need to convert?
    wxString effectsVersion = _effectPresetManager.GetVersion();
    if (effectsVersion < "0004") {
        wxMessageBox("Loading of xLights v3 rgbeffects is no longer supported.", "Error", wxOK | wxCENTRE |wxICON_ERROR, xLightsFrame::GetFrame());
    }
    if (effectsVersion < "0005") {
        //flip to AntiAlias=1 by default
        for (pugi::xml_node el = modelsNode.first_child(); el; el = el.next_sibling()) {
            if (std::string_view(el.attribute("Antialias").as_string("1")) == "0") {
                el.remove_attribute("Antialias");
                el.append_attribute("Antialias") = "1";
                UnsavedRgbEffectsChanges = true;
            }
        }
    }
    if (effectsVersion < "0006") {
        // need to convert models/groups to remove MyDisplay and add preview assignments
        for (pugi::xml_node model = modelsNode.first_child(); model; model = model.next_sibling()) {
            if (std::string_view(model.name()) == "model") {
                std::string my_display = model.attribute("MyDisplay").as_string();
                std::string layout_group = "Unassigned";
                if (my_display == "1") {
                    layout_group = "Default";
                }
                model.remove_attribute("MyDisplay");
                model.remove_attribute("LayoutGroup");
                model.append_attribute("LayoutGroup") = layout_group;
            }
        }
        // parse groups once to figure out if any of them are selected
        bool groups_are_selected = false;
        for (pugi::xml_node group = modelGroupsNode.first_child(); group; group = group.next_sibling()) {
            if (std::string_view(group.name()) == "modelGroup") {
                std::string selected = group.attribute("selected").as_string();
                if (selected == "1") {
                    groups_are_selected = true;
                    break;
                }
            }
        }
        // if no groups are selected then models remain as set above and all groups goto Default
        if (!groups_are_selected) {
            for (pugi::xml_node group = modelGroupsNode.first_child(); group; group = group.next_sibling()) {
                if (std::string_view(group.name()) == "modelGroup") {
                    group.remove_attribute("selected");
                    group.remove_attribute("LayoutGroup");
                    group.append_attribute("LayoutGroup") = "Default";
                }
            }
        }
        else { // otherwise need to set models in unchecked groups to unassigned
            std::set<std::string> modelsAdded;
            for (pugi::xml_node group = modelGroupsNode.first_child(); group; group = group.next_sibling()) {
                if (std::string_view(group.name()) == "modelGroup") {
                    std::string selected = group.attribute("selected").as_string();
                    std::string layout_group = "Unassigned";
                    if (selected == "1") {
                        wxArrayString mn = wxSplit(wxString(group.attribute("models").as_string()), ',');
                        for (int x = 0; x < (int)mn.size(); x++) {
                            std::string name = mn[x].Trim(true).Trim(false).ToStdString();
                            if (modelsAdded.find(name) == modelsAdded.end()) {
                                modelsAdded.insert(mn[x].Trim(true).Trim(false).ToStdString());
                            }
                        }
                        layout_group = "Default";
                    }
                    group.remove_attribute("selected");
                    group.remove_attribute("LayoutGroup");
                    group.append_attribute("LayoutGroup") = layout_group;
                }
            }
            // now move models back to unassigned that were not part of a checked group
            for (pugi::xml_node model = modelsNode.first_child(); model; model = model.next_sibling()) {
                if (std::string_view(model.name()) == "model") {
                    std::string mn = model.attribute("name").as_string();
                    if (modelsAdded.find(mn) == modelsAdded.end()) {
                        model.remove_attribute("LayoutGroup");
                        model.append_attribute("LayoutGroup") = "Unassigned";
                    }
                }
            }
        }
        UnsavedRgbEffectsChanges = true;
    }

    if (effectsVersion < "0007") {
        // fix any no longer supported smart remote settings *A*->*B*->*C* and a->*B*->*C*
        for (pugi::xml_node model = modelsNode.first_child(); model; model = model.next_sibling()) {
            if (std::string_view(model.name()) == "model") {
                for (pugi::xml_node cc = model.first_child(); cc; cc = cc.next_sibling()) {
                    std::string_view sr = cc.attribute("SmartRemote").as_string("0");
                    if (sr == "4") {
                        cc.remove_attribute("SmartRemote");
                        cc.append_attribute("SmartRemote") = "1";
                        cc.append_attribute("SRMaxCascade") = "3";
                    }
                    else if (sr == "5") {
                        cc.remove_attribute("SmartRemote");
                        cc.append_attribute("SmartRemote") = "2";
                        cc.append_attribute("SRMaxCascade") = "2";
                    }
                }
            }
        }
    }

    if (!modelsNode) {
        modelsNode = root.append_child("models");
    }
    if (!viewObjectsNode) {
        viewObjectsNode = root.append_child("view_objects");
        pugi::xml_node node = viewObjectsNode.append_child("view_object");
        node.append_attribute("DisplayAs") = "Gridlines";
        node.append_attribute("LayoutGroup") = "Default";
        node.append_attribute("name") = "Gridlines";
        node.append_attribute("GridLineSpacing") = "50";
        node.append_attribute("GridWidth") = "2000.0";
        node.append_attribute("GridHeight") = "1000.0";
        node.append_attribute("WorldPosX") = "0.0000";
        node.append_attribute("WorldPosY") = "0.0000";
        node.append_attribute("WorldPosZ") = "0.0000";
        node.append_attribute("ScaleX") = "1.0000";
        node.append_attribute("ScaleY") = "1.0000";
        node.append_attribute("ScaleZ") = "1.0000";
        node.append_attribute("RotateX") = "90.0";
        node.append_attribute("RotateY") = "0";
        node.append_attribute("RotateZ") = "0";
        node.append_attribute("versionNumber") = "3";
        node.append_attribute("Active") = "1";
        UnsavedRgbEffectsChanges = true;
    }


    if (!viewsNode) {
        UnsavedRgbEffectsChanges = true;
    } else {
        _sequenceViewManager.Load(viewsNode, _sequenceElements.GetCurrentView());
    }

    _defaultSeqView = GetXmlSetting("defaultSeqView", "");

    if (_sequenceViewManager.GetViewIndex(_defaultSeqView) == -1 && _defaultSeqView != "") {
        spdlog::warn("View Not Found ... clearing");
        _defaultSeqView.clear();
        SetXmlSetting("defaultSeqView", _defaultSeqView);
        UnsavedRgbEffectsChanges = true;
    }

    if (colorsNode) {
        color_mgr.Load(colorsNode);
    }

    if (viewpointsNode) {
        viewpoint_mgr.Load(viewpointsNode);
        layoutPanel->GetMainPreview()->RestoreDefaultCameraPosition();
        _housePreviewPanel->GetModelPreview()->RestoreDefaultCameraPosition();
    }

    if (!modelGroupsNode) {
        modelGroupsNode = root.append_child("modelGroups");
    }

    // perspectivesNode is used locally to load _perspectives, then discarded
    _perspectives.clear();
    _currentPerspectiveName = "";
    if (perspectivesNode) {
        _currentPerspectiveName = perspectivesNode.attribute("current").as_string();
        for (pugi::xml_node p = perspectivesNode.first_child(); p; p = p.next_sibling()) {
            if (std::string_view(p.name()) == "perspective") {
                Perspective pv;
                pv.name = p.attribute("name").as_string();
                pv.settings = p.attribute("settings").as_string();
                pv.version = p.attribute("version").as_string("2.0");
                _perspectives.push_back(pv);
            }
        }
    }

    int previewWidth = (int)std::strtol(GetXmlSetting("previewWidth", "1280").c_str(), nullptr, 10);
    int previewHeight = (int)std::strtol(GetXmlSetting("previewHeight", "720").c_str(), nullptr, 10);
    if (previewWidth == 0 || previewHeight == 0) {
        previewWidth = 1280;
        previewHeight = 720;
    }
    SetPreviewSize(previewWidth, previewHeight);

    mBackgroundImage = FileUtils::FixFile(GetShowDirectory(), GetXmlSetting("backgroundImage", ""));
    ObtainAccessToURL(mBackgroundImage);
    if (!mBackgroundImage.empty() && (!FileExists(mBackgroundImage) || !wxIsReadable(mBackgroundImage))) {
        //image doesn't exist there, lets look for it in media directories
        std::string bgImg = GetXmlSetting("backgroundImage", "");
        for (auto &dir : mediaDirectories) {
            wxString fn = FileUtils::FixFile(dir, bgImg);
            ObtainAccessToURL(fn.ToStdString());
            if (FileExists(fn) && wxIsReadable(fn)) {
                mBackgroundImage = fn.ToStdString();
                break;
            }
        }
        if (!FileExists(mBackgroundImage) || !wxIsReadable(mBackgroundImage)) {
            wxFileName name(mBackgroundImage);
            name.SetPath(CurrentDir);
            if (name.Exists()) {
                mBackgroundImage = name.GetFullPath().ToStdString();
                ObtainAccessToURL(mBackgroundImage);
            }
        }
    }
    SetPreviewBackgroundImage(mBackgroundImage);
    if (mBackgroundImage != GetXmlSetting("backgroundImage", "")) {
        SetXmlSetting("backgroundImage", mBackgroundImage);
    }

    SetDisplay2DBoundingBox(GetXmlSetting("Display2DBoundingBox", "0") == "1");
    layoutPanel->SetDisplay2DBoundingBox(GetDisplay2DBoundingBox());
    SetDisplay2DCenter0(GetXmlSetting("Display2DCenter0", "0") == "1");
    layoutPanel->SetDisplay2DCenter0(GetDisplay2DCenter0());
    SetDisplay2DGrid(GetXmlSetting("Display2DGrid", "0") == "1");
    SetDisplay2DGridSpacing(std::strtol(GetXmlSetting("Display2DGridSpacing", "100").c_str(), nullptr, 10));
    layoutPanel->SetDisplay2DGridSpacing(GetDisplay2DGrid(), GetDisplay2DGridSpacing());

    //Load FSEQ and Backup directory settings
    fseqDirectory = GetXmlSetting("fseqDir", showDirectory);
    if (wxDir::Exists(fseqDirectory) && !ObtainAccessToURL(fseqDirectory, true)) {
        std::string orig = fseqDirectory;
        PromptForDirectorySelection("Reselect FSEQ Directory", fseqDirectory);
        if (fseqDirectory != orig) {
            SetXmlSetting("fseqDir", fseqDirectory);
            UnsavedRgbEffectsChanges = true;
        }
    }
    if (!wxDir::Exists(fseqDirectory)) {
        spdlog::warn("FSEQ Directory not Found ... switching to Show Directory.");
        fseqDirectory = showDirectory;
        SetXmlSetting("fseqDir", showDirectory);
        UnsavedRgbEffectsChanges = true;
    }
    FseqDir = fseqDirectory;
    renderCacheDirectory = GetXmlSetting("renderCacheDir", fseqDirectory); // we user fseq directory if no setting is present
    if (wxDir::Exists(renderCacheDirectory) && !ObtainAccessToURL(renderCacheDirectory, true)) {
        std::string orig = renderCacheDirectory;
        PromptForDirectorySelection("Reselect RenderCache Directory", renderCacheDirectory);
        if (orig != renderCacheDirectory) {
            SetXmlSetting("renderCacheDir", renderCacheDirectory);
            UnsavedRgbEffectsChanges = true;
        }
    }
    if (!wxDir::Exists(renderCacheDirectory)) {
        spdlog::warn("Render Cache Directory not Found ... switching to Show Directory.");
        renderCacheDirectory = showDirectory;
        SetXmlSetting("renderCacheDir", showDirectory);
        UnsavedRgbEffectsChanges = true;
    }
    _renderCache.SetRenderCacheFolder(renderCacheDirectory);

    mStoredLayoutGroup = GetXmlSetting("storedLayoutGroup", "Default");

    // validate stored preview exists and load LayoutGroups from XML
    bool found_saved_preview = false;
    LayoutGroups.clear();
    layoutPanel->Reset();
    if (layoutGroupsNode) {
        for (pugi::xml_node e = layoutGroupsNode.first_child(); e; e = e.next_sibling()) {
            if (std::string_view(e.name()) == "layoutGroup") {
                std::string grp_name = e.attribute("name").as_string();
                if (!grp_name.empty()) {
                    if (grp_name == mStoredLayoutGroup) {
                        found_saved_preview = true;
                    }
                    auto grp = std::make_unique<LayoutGroup>(grp_name, this);
                    grp->SetFromXml(e);
                    AddPreviewOption(grp.get());
                    LayoutGroups.emplace(grp_name, std::move(grp));
                    layoutPanel->AddPreviewChoice(grp_name);
                }
            }
        }
    }
    if (!found_saved_preview) {
        mStoredLayoutGroup = "Default";
    }

    mBackgroundBrightness = (int)std::strtol(GetXmlSetting("backgroundBrightness", "100").c_str(), nullptr, 10);
    mBackgroundAlpha = (int)std::strtol(GetXmlSetting("backgroundAlpha", "100").c_str(), nullptr, 10);
    SetPreviewBackgroundBrightness(mBackgroundBrightness, mBackgroundAlpha);
    mScaleBackgroundImage = std::strtol(GetXmlSetting("scaleImage", "0").c_str(), nullptr, 10) > 0;
    SetPreviewBackgroundScaled(mScaleBackgroundImage);

    std::string group = layoutPanel->GetCurrentLayoutGroup();
    if (group != "Default" && group != "All Models" && group != "Unassigned") {
        modelPreview->SetBackgroundBrightness(layoutPanel->GetBackgroundBrightnessForSelectedPreview(), layoutPanel->GetBackgroundAlphaForSelectedPreview());
        modelPreview->SetScaleBackgroundImage(layoutPanel->GetBackgroundScaledForSelectedPreview());
    }
    
    auto* config = GetXLightsConfig();
    bool is_3d = config->ReadBool("LayoutMode3D", false);
    is_3d = GetXmlSetting("LayoutMode3D", is_3d ? "1" : "0") == "1";
    modelPreview->Set3D(is_3d);
    _housePreviewPanel->Set3d(is_3d);
    layoutPanel->Set3d(is_3d);

    UpdateLayoutSave();
    UpdateControllerSave();

    // update version
    _effectPresetManager.SetVersion(XLIGHTS_RGBEFFECTS_VERSION);
    SetXmlSetting("xlightsVersion", xlights_version_string);

    // Handle upgrade of networks file to the controller/output structure
    bool converted = _outputManager.ConvertModelStartChannels(modelsNode);

    displayElementsPanel->SetSequenceElementsModelsViews(&_seqData, &_sequenceElements, &_sequenceViewManager);
    layoutPanel->ClearUndo();

    // Merge base show folder models into the XML nodes before building the objects
    if (_outputManager.IsAutoUpdateFromBaseShowDir() && !_outputManager.GetBaseShowDir().empty()) {
        bool changed = false;
        changed |= AllModels.MergeBaseXml(_outputManager.GetBaseShowDir(), modelsNode, modelGroupsNode);
        changed |= AllObjects.MergeBaseXml(_outputManager.GetBaseShowDir(), viewObjectsNode);
        if (changed) {
            UnsavedRgbEffectsChanges = true;
        }
    }
    LoadModels(modelsNode, modelGroupsNode, viewObjectsNode);

    mSequencerInitialize = false;

    // load the perspectives
    CheckForAndCreateDefaultPerpective();
    perspectivePanel->SetPerspectives(this);
    LoadPerspectivesMenu();

    float elapsedTime = sw.Time() / 1000.0; //msec => sec
    SetStatusText(wxString::Format(_("'%s' loaded in %4.3f sec."), effectsFile.GetFullPath(), elapsedTime));

    UpdateLayoutSave();
    UpdateControllerSave();

    if (converted) {
        UnsavedRgbEffectsChanges = true;
        wxMessageBox("Your setup tab data has been converted to the new controller centric format.\nIf you choose to save either the Controller (Setup) or Layout Tab data it is critical you save both or some of your model start channels will break.\nIf this happens you can either repair them manually or roll back to a backup copy.");
    }
}

void xLightsFrame::LoadPerspectivesMenu()
{
    // Clear old menu items
    int menuCount = MenuItemPerspectives->GetMenuItemCount();
    int first = menuCount - 1;
    wxMenuItem* current_menuitem = MenuItemPerspectives->FindItemByPosition(first);
    while (current_menuitem != nullptr && !current_menuitem->IsSeparator()) {
        first--;
        current_menuitem = MenuItemPerspectives->FindItemByPosition(first);
    }
    first++;
    current_menuitem = first < menuCount ? MenuItemPerspectives->FindItemByPosition(first) : nullptr;
    while (current_menuitem != nullptr) {
        MenuItemPerspectives->Delete(current_menuitem);
        menuCount--;
        current_menuitem = first < menuCount ? MenuItemPerspectives->FindItemByPosition(first) : nullptr;
    }

    int pCount = 0;
    for (auto& p : _perspectives) {
        if (!p.name.empty()) {
            int id = wxNewId();
            MenuItemPerspectives->AppendRadioItem(id, p.name);
            if (mCurrentPerpective != nullptr && p.name == mCurrentPerpective->name)
                MenuItemPerspectives->Check(id, true);
            PerspectiveId pmenu;
            pmenu.id = id;
            pmenu.p = &p;
            perspectives[pCount] = pmenu;
            Connect(id, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemLoadPerspectiveSelected);
            pCount++;
            if (pCount >= 10) {
                return;
            }
        }
    }
}

void xLightsFrame::OnMenuItemLoadPerspectiveSelected(wxCommandEvent& event)
{
    Notebook1->SetSelection(Notebook1->GetPageIndex(PanelSequencer));
    for (size_t i = 0; i < 10; ++i) {
        if (perspectives[i].id == event.GetId()) {
            DoLoadPerspective(perspectives[i].p);
            return;
        }
    }
}

void xLightsFrame::SaveModelsFile()
{
    wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets after new file is written
    

    std::string filename = CurrentDir.ToStdString() + "/xScheduleData/GetModels.dat";

    if (!wxDir::Exists(CurrentDir + "/xScheduleData")) {
        spdlog::debug("Creating xScheduleData folder.");
        wxDir sd(CurrentDir);
        sd.Make(CurrentDir + "/xScheduleData");
    }

    spdlog::debug("Creating models JSON file: {}.", (const char*)filename.c_str());

    wxFile modelsJSON;
    if (!modelsJSON.Create(filename, true) || !modelsJSON.IsOpened()) {
        spdlog::error("Unable to create file: {}.", (const char*)filename.c_str());
        return;
    }

    modelsJSON.Write("{\"models\":[");

    bool first = true;
    for (auto m = AllModels.begin(); m != AllModels.end(); ++m) {
        Model* model = m->second;
        if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
            // Dont export model groups ... they arent useful

            // if (!first)
            //{
            //     modelsJSON.Write(",");
            // }
            // first = false;

            // ModelGroup* mg = static_cast<ModelGroup*>(model);
            // modelsJSON.Write("{\"name\":\"" + mg->name +
            //     "\",\"type\":\"" + mg->GetDisplayAs() +
            //     "\",\"startchannel\":\"" + wxString::Format("%i", mg->NodeStartChannel(0) + 1) +
            //     "\",\"channels\":\"" + wxString::Format("%i", mg->GetChanCount()) +
            //     "\",\"stringtype\":\"\"}");
        } else if (model->GetDisplayAs() == DisplayAsType::SubModel) {
            // Dont export SubModels ... they arent useful

            // if (!first)
            //{
            //     modelsJSON.Write(",");
            // }
            // first = false;

            // SubModel* sm = static_cast<SubModel*>(model);
            // int ch = sm->GetNumberFromChannelString(sm->ModelStartChannel);
            // modelsJSON.Write("{\"name\":\"" + sm->name +
            //     "\",\"type\":\"" + sm->GetDisplayAs() +
            //     "\",\"startchannel\":\"" + wxString::Format("%i", ch) +
            //     "\",\"channels\":\"" + wxString::Format("%i", sm->GetChanCount()) +
            //     "\",\"stringtype\":\"" + sm->GetStringType() + "\"}");
        } else {
            if (!first) {
                modelsJSON.Write(",");
            }
            first = false;

            long ch = model->GetNumberFromChannelString(model->ModelStartChannel);
            modelsJSON.Write("{\"name\":\"" + model->name +
                             "\",\"type\":\"" + DisplayAsTypeToString(model->GetDisplayAs()) +
                             "\",\"startchannel\":\"" + wxString::Format("%ld", (long)ch) +
                             "\",\"channels\":\"" + wxString::Format("%ld", (long)model->GetChanCount()) +
                             "\",\"stringtype\":\"" + model->GetStringType() + "\"}");
        }
    }

    modelsJSON.Write("]}");

    spdlog::debug("     models JSON file done.");
}

// returns true on success
bool xLightsFrame::SaveEffectsFile(bool backup)
{


    // dont save if currently saving
    std::unique_lock<std::mutex> lock(saveLock, std::try_to_lock);
    if (!lock.owns_lock())
        return false;

    auto saveXml = BuildEffectsXml();

    wxFileName effectsFile;
    effectsFile.AssignDir(CurrentDir);
    if (backup) {
        effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE_BACKUP));
    } else {
        effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
    }

    ObtainAccessToURL(effectsFile.GetFullPath());
    wxFileOutputStream fout(effectsFile.GetFullPath());
    wxBufferedOutputStream *bout = new wxBufferedOutputStream(fout, 2 * 1024 * 1024);
    size_t len = saveXml.length();
    bout->Write(saveXml.c_str(), len);
    if (bout->LastWrite() != len) {
        if (backup) {
            spdlog::warn("Unable to save backup of RGB effects file");
        } else {
            DisplayError("Unable to save RGB effects file", this);
        }
        delete bout;
        return false;
    }
    delete bout;

    if (!fout.Close()) {
        if (backup) {
            spdlog::warn("Unable to save backup of RGB effects file");
        } else {
            DisplayError("Unable to save RGB effects file", this);
        }
        return false;
    }

    // Save effect presets to separate JSON file
    {
        wxFileName presetsFile;
        presetsFile.AssignDir(CurrentDir);
        if (backup) {
            presetsFile.SetFullName(_(XLIGHTS_PRESETS_FILE_BACKUP));
        } else {
            presetsFile.SetFullName(_(XLIGHTS_PRESETS_FILE));
        }

        if (!_effectPresetManager.SaveJsonFile(presetsFile.GetFullPath().ToStdString())) {
            if (backup) {
                spdlog::warn("Unable to save backup of effect presets file");
            } else {
                DisplayError("Unable to save effect presets file", this);
            }
        }
    }

    if (!backup) {
        UnsavedRgbEffectsChanges = false;
    }

    UpdateLayoutSave();
    UpdateControllerSave();

    return true;
}

void xLightsFrame::CreateDefaultEffectsXml(pugi::xml_document& doc)
{
    doc.reset();
    doc.append_child("xrgb");
    UnsavedRgbEffectsChanges = true;
    UpdateLayoutSave();
    UpdateControllerSave();
}

// This ensures submodels are in the right order in the sequence elements after the user
// reorders them in the ssubmodel dialog
bool xLightsFrame::EnsureSequenceElementsAreOrderedCorrectly(const std::string ModelName, std::vector<std::string>& submodelOrder)
{
    ModelElement* elementToCheck = dynamic_cast<ModelElement*>(_sequenceElements.GetElement(ModelName));

    if (elementToCheck != nullptr) {
        // Check if they are already right and in the right order
        bool identical = true;
        if ((size_t)elementToCheck->GetSubModelCount() == submodelOrder.size()) {
            for (int i = 0; i < elementToCheck->GetSubModelCount(); i++) {
                if (elementToCheck->GetSubModel(i)->GetName() != submodelOrder[i]) {
                    identical = false;
                    break;
                }
            }
        } else {
            identical = false;
        }

        if (identical) {
            // no changes we can exit
            return false;
        }

        // Grab the existing elements
        std::list<SubModelElement*> oldList;
        for (int i = 0; i < elementToCheck->GetSubModelCount(); i++) {
            oldList.push_back(elementToCheck->GetSubModel(i));
        }

        // remove but dont delete all submodels
        elementToCheck->RemoveAllSubModels();

        // Now add them back in the right order
        for (auto msm = submodelOrder.begin(); msm != submodelOrder.end(); ++msm) {
            bool found = false;
            for (auto it = oldList.begin(); it != oldList.end(); ++it) {
                if ((*it)->GetName() == *msm) {
                    elementToCheck->AddSubModel(*it);
                    oldList.erase(it);
                    found = true;
                    break;
                }
            }

            if (!found) {
                // add the submodel as it didnt previously exist
                elementToCheck->GetSubModel(*msm, true);
            }
        }

        // delete any that are no longer there
        for (auto it = oldList.begin(); it != oldList.end(); ++it) {
            delete *it;
        }

        return true;
    }

    return false;
}

bool xLightsFrame::RenameModel(const std::string OldName, const std::string& NewName)
{
    
    bool internalsChanged = false;

    if (OldName == NewName) {
        return false;
    }
    AbortRender();

    spdlog::debug("Renaming model '{}' to '{}'.", (const char*)OldName.c_str(), (const char*)NewName.c_str());

    Element* elem_to_rename = _sequenceElements.GetElement(OldName);
    if (elem_to_rename != nullptr) {
        elem_to_rename->SetName(NewName);
    }

    if (std::find(OldName.begin(), OldName.end(), '/') != OldName.end()) {
        internalsChanged = AllModels.RenameSubModel(OldName, NewName);
    } else {
        internalsChanged = AllModels.Rename(OldName, NewName);
    }

    RenameModelInViews(OldName, NewName);
    _sequenceElements.RenameModelInViews(OldName, NewName);

    UnsavedRgbEffectsChanges = true;
    UpdateLayoutSave();
    UpdateControllerSave();
    return internalsChanged;
}

void xLightsFrame::RenameModelInViews(const std::string old_name, const std::string& new_name)
{
	_sequenceViewManager.RenameModel(old_name, new_name);
}

void xLightsFrame::SetChoicebook(wxChoicebook* cb, const wxString& PageName)
{
    if (cb->GetChoiceCtrl()->GetStringSelection() == PageName)
        return; // no need to change

    RenderableEffect* reff = effectManager.GetEffect(PageName.ToStdString());
    if (reff != nullptr) {
        for (size_t i = 0; i < cb->GetPageCount(); i++) {
            if (cb->GetPageText(i) == reff->ToolTip()) {
                cb->ChangeSelection(i);
                return;
            }
        }
    }
}

bool xLightsFrame::RenameObject(const std::string OldName, const std::string& NewName)
{
    
    bool internalsChanged = false;

    if (OldName == NewName) {
        return false;
    }

    spdlog::debug("Renaming object '{}' to '{}'.", (const char*)OldName.c_str(), (const char*)NewName.c_str());

    Element* elem_to_rename = _sequenceElements.GetElement(OldName);
    if (elem_to_rename != nullptr) {
        elem_to_rename->SetName(NewName);
    }

    internalsChanged = AllObjects.Rename(OldName, NewName);

    UnsavedRgbEffectsChanges = true;
    UpdateLayoutSave();
    UpdateControllerSave();
    return internalsChanged;
}

void xLightsFrame::OnBitmapButtonSaveSeqClick(wxCommandEvent& event)
{
    SaveSequence();
}

static std::string chooseNewName(xLightsFrame* parent, std::vector<std::string>& names,
                                 const std::string& msg, const std::string curval)
{
    wxTextEntryDialog dialog(parent, _("Enter new name"), msg, curval);
    int DlgResult;
    do {
        DlgResult = dialog.ShowModal();
        if (DlgResult == wxID_OK) {
            // validate inputs
            std::string NewName = dialog.GetValue().Trim(true).Trim(false);
            if (std::find(names.begin(), names.end(), NewName) == names.end()) {
                return NewName;
            }
        }
    } while (DlgResult == wxID_OK);
    return curval;
}

static void AddModelsToPreview(ModelGroup* grp, std::vector<Model*>& PreviewModels)
{
    for (auto it2 = grp->Models().begin(); it2 != grp->Models().end(); ++it2) {
        Model* model = dynamic_cast<Model*>(*it2);
        ModelGroup* g2 = dynamic_cast<ModelGroup*>(*it2);
        SubModel* sm = dynamic_cast<SubModel*>(*it2);

        if (sm != nullptr) {
            model = sm->GetParent();
        }
        if (g2 != nullptr) {
            AddModelsToPreview(g2, PreviewModels);
        } else if (model != nullptr) {
            if (std::find(PreviewModels.begin(), PreviewModels.end(), model) == PreviewModels.end()) {
                PreviewModels.push_back(model);
            }
        }
    }
}

void xLightsFrame::LoadModels(pugi::xml_node modelsNode,
                              pugi::xml_node modelGroupsNode,
                              pugi::xml_node viewObjectsNode)
{
    auto logger_work = spdlog::get("work");
    logger_work->debug("        LoadModels.");

    playModel = nullptr;
    PreviewModels.clear();
    AllModels.LoadModels(modelsNode,
        modelPreview->GetVirtualCanvasWidth(),
        modelPreview->GetVirtualCanvasHeight());

    AllObjects.LoadViewObjects(viewObjectsNode);

    std::vector<std::string> current;
    for (const auto& it : AllModels) {
        current.push_back(it.first);
    }
    for (pugi::xml_node e = modelGroupsNode.first_child(); e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "modelGroup") {
            wxString nameStr = e.attribute("name").as_string();
            std::string name = nameStr.Trim(true).Trim(false).ToStdString();
            current.push_back(name);
        }
    }
    {
        pugi::xml_node e = modelGroupsNode.first_child();
        while (e) {
            pugi::xml_node nextE = e.next_sibling();
            if (std::string_view(e.name()) == "modelGroup") {
                wxString nameStr = e.attribute("name").as_string();
                std::string name = nameStr.Trim(true).Trim(false).ToStdString();
                Model* model = AllModels[name];
                if (model != nullptr) {
                    wxArrayString choices;
                    choices.push_back("Rename Model");
                    choices.push_back("Delete Model");
                    choices.push_back("Rename Group");
                    choices.push_back("Delete Group");

                    wxString msg = "A model of name \'" + name + "\' already exists.  What action should we take?";
                    wxSingleChoiceDialog dlg(this, msg, "Model/Group Name Conflict", choices, (void**)nullptr,
                        wxDEFAULT_DIALOG_STYLE | wxOK | wxCENTRE | wxRESIZE_BORDER, wxDefaultPosition);
                    bool done = false;
                    do {
                        dlg.ShowModal();
                        int sel = dlg.GetSelection();
                        switch (sel) {
                        case 0:
                        case 1:
                            for (pugi::xml_node e2 = modelsNode.first_child(); e2; e2 = e2.next_sibling()) {
                                if (std::string_view(e2.name()) == "model") {
                                    wxString mnameStr = e2.attribute("name").as_string();
                                    std::string mname = mnameStr.Trim(true).Trim(false).ToStdString();
                                    if (mname == name) {
                                        UnsavedRgbEffectsChanges = true;
                                        if (sel == 1) {
                                            modelsNode.remove_child(e2);
                                            done = true;
                                        }
                                        else {
                                            //rename
                                            std::string newName = chooseNewName(this, current, "Rename Model", mname);
                                            if (newName != mname) {
                                                current.push_back(newName);
                                                e2.remove_attribute("name");
                                                e2.append_attribute("name") = newName;
                                                done = true;
                                            }
                                        }
                                        AllModels.LoadModels(modelsNode,
                                            modelPreview->GetVirtualCanvasWidth(),
                                            modelPreview->GetVirtualCanvasHeight());
                                        break; // node may have been removed; stop iterating
                                    }
                                }
                            }
                            break;
                        case 2: {
                            std::string newName = chooseNewName(this, current, "Rename Model Group", name);
                            if (newName != name) {
                                current.push_back(newName);
                                e.remove_attribute("name");
                                e.append_attribute("name") = newName;
                                done = true;
                            }
                        }
                              break;
                        case 3:
                            modelGroupsNode.remove_child(e);
                            nextE = modelGroupsNode.first_child(); // restart from beginning
                            UnsavedRgbEffectsChanges = true;
                            done = true;
                            break;
                        default:
                            break;
                        }
                    } while (!done);
                }
            }
            e = nextE;
        }
    }
    AllModels.LoadGroups(modelGroupsNode,
        modelPreview->GetVirtualCanvasWidth(),
        modelPreview->GetVirtualCanvasHeight());

    // Add all models to default House Preview that are set to Default or All Previews
    for (const auto& it : AllModels) {
        Model* model = it.second;
        if (model->GetDisplayAs() != DisplayAsType::ModelGroup) {
            if (model->GetLayoutGroup() == "Default" || model->GetLayoutGroup() == "All Previews") {
                PreviewModels.push_back(model);
            }
        }
    }

    // Now add all models to default House Preview that are in groups set to Default or All Previews
    for (const auto& it : AllModels) {
        Model* model = it.second;
        if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
            ModelGroup* grp = (ModelGroup*)model;
            if (model->GetLayoutGroup() == "All Previews" || model->GetLayoutGroup() == "Default") {
                AddModelsToPreview(grp, PreviewModels);
            }
        }
    }

    layoutPanel->UpdateModelList(true);
    displayElementsPanel->UpdateModelsForSelectedView();


    UpdateLayoutSave();
    UpdateControllerSave();
}


void xLightsFrame::UpdateModelsList()
{
    auto logger_work = spdlog::get("work");
    logger_work->debug("        UpdateModelsList.");

    if (modelPreview == nullptr) return; // this happens when xlights is first loaded

    //abort any render as it will crash if the model changes
    AbortRender();

    playModel = nullptr;
    PreviewModels.clear();
    UnselectEffect();
    modelsChangeCount++;

    // Refresh model group pointer vectors before accessing them. Submodel add/remove operations
    // (e.g. SubModelsDialog Join) delete and recreate SubModel objects without going through
    // RecalcStartChannels, leaving group models vectors with dangling pointers.
    AllModels.ResetModelGroups();

    // Add all models to default House Preview that are set to Default or All Previews
    for (const auto& it : AllModels) {
        Model* model = it.second;
        if (model->GetDisplayAs() != DisplayAsType::ModelGroup) {
            if (model->GetLayoutGroup() == "Default" || model->GetLayoutGroup() == "All Previews") {
                PreviewModels.push_back(model);
            }
        }
    }

    // Now add all models to default House Preview that are in groups set to Default or All Previews
    for (const auto& it : AllModels) {
        Model* model = it.second;
        if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
            ModelGroup* grp = (ModelGroup*)model;
            if (model->GetLayoutGroup() == "All Previews" || model->GetLayoutGroup() == "Default") {
                AddModelsToPreview(grp, PreviewModels);
            }
        }
    }

    layoutPanel->UpdateModelList(true);
    displayElementsPanel->UpdateModelsForSelectedView();

    UpdateLayoutSave();
    UpdateControllerSave();
}

std::string xLightsFrame::OpenAndCheckSequence(const std::string& origFilename)
{
    

    std::string file;

    if (origFilename != "") {

        EnableSequenceControls(false);

        wxString seq = origFilename;

        printf("Processing file %s\n", (const char*)seq.c_str());
        spdlog::debug("Batch Check sequence processing file {}\n", (const char*)seq.c_str());
        OpenSequence(seq, nullptr);
        EnableSequenceControls(false);

        // if the fseq directory is not the show directory then ensure the fseq folder is set right
        if (fseqDirectory != showDirectory) {
            if (!ObtainAccessToURL(fseqDirectory)) {
                wxMessageBox("Could not obtain read/write access to FSEQ directory " + fseqDirectory + ". " + "Try re-selecting the FSEQ directory in Preferences.", "Error",
                             wxOK | wxICON_ERROR);
            }
            wxFileName fn(xlightsFilename);
            fn.SetPath(fseqDirectory);
            xlightsFilename = fn.GetFullPath();
        }

        SetStatusText(_("Checking sequence ") + xlightsFilename + _("."));

        file = CheckSequence(false, true);

        _checkSequenceMode = false;
        EnableSequenceControls(true);
        spdlog::debug("Check sequence done.");
        CloseSequence();
    }

    return file;
}

void xLightsFrame::OpenAndCheckSequence(const wxArrayString& origFilenames, bool exitOnDone)
{
    

    if (origFilenames.IsEmpty()) {
        _checkSequenceMode = false;
        EnableSequenceControls(true);
        spdlog::debug("Batch check sequence done.");
        printf("Done All Files\n");
        if (exitOnDone) {
            Destroy();
        }
        else {
            CloseSequence();
        }
        return;
    }

    if (wxGetKeyState(WXK_ESCAPE)) {
        spdlog::debug("Batch check sequence cancelled.");
        _checkSequenceMode = false;
        EnableSequenceControls(true);
        printf("Batch check sequence cancelled.\n");

        auto* config = GetXLightsConfig();
        if (config != nullptr) {
            auto selectGridIcon = config->ReadBool("BatchRendererGridIconBackgrounds", false);
            if (selectGridIcon) {
                SetGridIconBackgrounds(selectGridIcon);
            }
            auto selectGroupEffect = config->ReadBool("BatchRendererGroupEffectBackgrounds", false);
            if (selectGroupEffect) {
                SetShowGroupEffectIndicator(selectGroupEffect);
            }
        }

        if (exitOnDone) {
            Destroy();
        }
        else {
            CloseSequence();
            SetStatusText(_("Batch Check Sequence Cancelled."));
        }
        return;
    }

    EnableSequenceControls(false);

    wxArrayString fileNames = origFilenames;
    wxString seq = fileNames[0];
    fileNames.RemoveAt(0);

    printf("Processing file %s\n", (const char*)seq.c_str());
    spdlog::debug("Batch Check sequence processing file {}\n", (const char*)seq.c_str());
    OpenSequence(seq, nullptr);
    EnableSequenceControls(false);

    // if the fseq directory is not the show directory then ensure the fseq folder is set right
    if (fseqDirectory != showDirectory) {
        if (!ObtainAccessToURL(fseqDirectory)) {
            wxMessageBox("Could not obtain read/write access to FSEQ directory " + fseqDirectory + ". "
                + "Try re-selecting the FSEQ directory in Preferences.", "Error",
                wxOK | wxICON_ERROR);
        }
        wxFileName fn(xlightsFilename);
        fn.SetPath(fseqDirectory);
        xlightsFilename = fn.GetFullPath();
    }

    SetStatusText(_("Checking sequence ") + xlightsFilename + _("."));

    CheckSequence(true, true);
    CallAfter(&xLightsFrame::OpenAndCheckSequence, fileNames, exitOnDone);
}

void xLightsFrame::OpenRenderAndSaveSequencesF(const wxArrayString& origFileNames, int flags)
{
    OpenRenderAndSaveSequences(origFileNames, flags & RENDER_EXIT_ON_DONE, flags & RENDER_ALREADY_RETRIED);
}

void xLightsFrame::OpenRenderAndSaveSequences(const wxArrayString &origFilenames, bool exitOnDone, bool alreadyRetried) {
    

    if (origFilenames.IsEmpty()) {
        _lowDefinitionRender = _saveLowDefinitionRender;
        _renderMode = false;
        EnableSequenceControls(true);
        spdlog::debug("Batch render done.");
        printf("Done All Files\n");
        wxBell();

        auto* config = GetXLightsConfig();
        if (config != nullptr) {
            auto selectGridIcon = config->ReadBool("BatchRendererGridIconBackgrounds", false);
            if (selectGridIcon) {
                SetGridIconBackgrounds(selectGridIcon);
            }
            auto selectGroupEffect = config->ReadBool("BatchRendererGroupEffectBackgrounds", false);
            if (selectGroupEffect) {
                SetShowGroupEffectIndicator(selectGroupEffect);
            }
        }

        if (exitOnDone) {
            Destroy();
        } else {
            CloseSequence();
            SetStatusText(_("Batch Render Done."));
        }
        return;
    }

    if (wxGetKeyState(WXK_ESCAPE))
    {
        spdlog::debug("Batch render cancelled.");
        _lowDefinitionRender = _saveLowDefinitionRender;
        _renderMode = false;
        EnableSequenceControls(true);
        printf("Batch render cancelled.\n");

        auto* config = GetXLightsConfig();
        if (config != nullptr) {
            auto selectGridIcon = config->ReadBool("BatchRendererGridIconBackgrounds", false);
            if (selectGridIcon) {
                SetGridIconBackgrounds(selectGridIcon);
            }
            auto selectGroupEffect = config->ReadBool("BatchRendererGroupEffectBackgrounds", false);
            if (selectGroupEffect) {
                SetShowGroupEffectIndicator(selectGroupEffect);
            }
        }

        if (exitOnDone) {
            Destroy();
        }
        else {
            CloseSequence();
            SetStatusText(_("Batch Render Cancelled."));
        }
        return;
    }

    EnableSequenceControls(false);

    wxArrayString fileNames = origFilenames;
    wxString seq = fileNames[0];
    wxStopWatch sw; // start a stopwatch timer

    auto b = _renderMode;
    _renderMode = false;

    wxString seqDisplay = seq;
    if ((seq.length() > 100) && seq.StartsWith(showDirectory)) {
        seqDisplay.Replace(showDirectory, "[Show Folder]", false);
    }

    if (fileNames.size() == 1) {
        SetStatusText("Batch Rendering " + seqDisplay + ". Last sequence.");
    } else {
        SetStatusText("Batch Rendering " + seqDisplay + ". " + wxString::Format("%d", (int)fileNames.size() - 1) + " sequences left to render.");
    }
    _renderMode = b;

    printf("Processing file %s\n", (const char *)seq.c_str());
    spdlog::debug("Batch Render Processing file {}\n", seq.ToStdString());
    LogMemoryUsage("batch-render sequence start: " + seq.ToStdString());
    OpenSequence(seq, nullptr);
    EnableSequenceControls(false);

    // if the fseq directory is not the show directory then ensure the fseq folder is set right
    if (fseqDirectory != showDirectory) {
        if (!ObtainAccessToURL(fseqDirectory)) {
            wxMessageBox("Could not obtain read/write access to FSEQ directory " + fseqDirectory + ". "
                         + "Try re-selecting the FSEQ directory in Preferences.", "Error",
                         wxOK | wxICON_ERROR);
        }
        wxFileName fn(xlightsFilename);
        fn.SetPath(fseqDirectory);
        xlightsFilename = fn.GetFullPath();
    }

    SetStatusText(_("Saving ") + xlightsFilename + _(" ... Rendering."));
    ProgressBar->Show();
    GaugeSizer->Layout();
    spdlog::info("Rendering on save.");
    RenderIseqData(true, nullptr); // render ISEQ layers below the Nutcracker layer
    spdlog::info("   iseq below effects done.");
    ProgressBar->SetValue(10);
    RenderGridToSeqData([this, sw, fileNames, exitOnDone, alreadyRetried] (bool aborted) {
        
        spdlog::info("   Effects done.");
        ProgressBar->SetValue(90);
        RenderIseqData(false, nullptr);  // render ISEQ layers above the Nutcracker layer
        spdlog::info("   iseq above effects done. Render complete.");
        ProgressBar->SetValue(100);
        ProgressBar->Hide();
        _appProgress->SetValue(0);
        _appProgress->Reset();
        GaugeSizer->Layout();

        if (!aborted || alreadyRetried) {
            spdlog::info("Saving fseq file.");
            LogMemoryUsage("batch-render fseq save: " + xlightsFilename.ToStdString());
            SetStatusText(_("Saving ") + xlightsFilename + _(" ... Writing fseq."));
            WriteFalconPiFile(xlightsFilename);
            spdlog::info("fseq file done.");
            DisplayXlightsFilename(xlightsFilename);
            float elapsedTime = sw.Time() / 1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
            wxString displayBuff = wxString::Format(_("%s     Updated in %7.3f seconds"), xlightsFilename, elapsedTime);
            spdlog::info("{}", (const char*)displayBuff.c_str());
            CallAfter(&xLightsFrame::SetStatusText, displayBuff, 0);
            mSavedChangeCount = _sequenceElements.GetChangeCount();
            mLastAutosaveCount = mSavedChangeCount;

            auto nFileNames = fileNames;
            nFileNames.RemoveAt(0);
            CallAfter(&xLightsFrame::OpenRenderAndSaveSequencesF, nFileNames, (exitOnDone ? RENDER_EXIT_ON_DONE : 0));
        } else {
            spdlog::info("Render was aborted, retrying.");
            CallAfter(&xLightsFrame::OpenRenderAndSaveSequencesF, fileNames, (exitOnDone ? RENDER_EXIT_ON_DONE : 0) | RENDER_ALREADY_RETRIED);
        }
    } );
}

void xLightsFrame::SaveSequence()
{
    LogMemoryUsage("SaveSequence start: " + xlightsFilename.ToStdString());

    if (_seqData.NumFrames() == 0)
    {
        DisplayError("You must open a sequence first!", this);
        return;
    }

    if (readOnlyMode)
    {
        DisplayError("Sequences cannot be saved in read only mode!", this);
        return;
    }

    wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
    wxPostEvent(this, playEvent);

    std::unique_lock<std::mutex> lock(saveLock);

    if (xlightsFilename.IsEmpty()) {
        wxString NewFilename;

        wxString startname;
        if (!CurrentSeqXmlFile->GetMediaFile().empty()) {
            startname = wxFileName(CurrentSeqXmlFile->GetMediaFile()).GetName();
        }

        wxFileDialog fd(this,
                        "Choose filename to Save Sequence:",
                        CurrentDir,
                        startname,
                        strSequenceSaveAsFileTypes,
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        bool ok;
        do {
            if (fd.ShowModal() != wxID_OK) {
                return;
            }
            // validate inputs
            NewFilename=fd.GetPath();
            NewFilename.Trim();
            ok=true;
            if (NewFilename.IsEmpty()) {
                ok=false;
                DisplayError("File name cannot be empty", this);
            }
        }
        while (!ok);
        wxFileName xmlFileName(NewFilename);//set XML Path based on user input
        _renderCache.SetSequence(renderCacheDirectory, xmlFileName.GetName());
        xmlFileName.SetExt("xsq");
        CurrentSeqXmlFile->SetFullPath(ToStdString(xmlFileName.GetFullPath()));

        AddToMRU(xmlFileName.GetFullPath());
        UpdateRecentFilesList(false);

        wxFileName fseqFileName(NewFilename);//create FSEQ file name in seq folder
        fseqFileName.SetExt("fseq");
        DisplayXlightsFilename(fseqFileName.GetFullPath());
    }

    // if the fseq directory is not the show directory then ensure the fseq folder is set right
	// Only Change FSEQ save folder if the FSEQ Folder Setting is NOT the Show Dir
    if (fseqDirectory != showDirectory) {
        if (!ObtainAccessToURL(fseqDirectory)) {
            wxMessageBox("Could not obtain read/write access to FSEQ directory " + fseqDirectory + ". "
                         + "Try re-selecting the FSEQ directory in Preferences.", "Error",
                         wxOK | wxICON_ERROR);
        }
        wxFileName fn(xlightsFilename);
        fn.SetPath(fseqDirectory);
        xlightsFilename = fn.GetFullPath();
    }

    EnableSequenceControls(false);
    wxStopWatch sw; // start a stopwatch timer

    if (wxString(CurrentSeqXmlFile->GetExt()).Lower() == "xml") {
        // Remove the old xml file as we are about to save it as an xsq
        wxRemoveFile(CurrentSeqXmlFile->GetFullPath());
        CurrentSeqXmlFile->SetExt("xsq");
    } else if (wxString(CurrentSeqXmlFile->GetExt()).Lower() == "xbkp") {
        CurrentSeqXmlFile->SetExt("xsq");
    }
    // Sync jukebox UI state to sequence data before saving
    if (GetJukeboxPanel()) {
        GetJukeboxPanel()->SyncToData(CurrentSeqXmlFile->GetJukeboxButtons());
    }
    SetStatusText(_("Saving ") + CurrentSeqXmlFile->GetFullPath() + _(" ... Saving xsq."));
    spdlog::info("Saving XSQ file.");
    if (!CurrentSeqXmlFile->Save(_sequenceElements)) {
        wxMessageDialog msgDlg(this, "Error Saving Sequence to " + CurrentSeqXmlFile->GetFullPath(),
                               "Error Saving Sequence", wxOK | wxCENTRE);
        msgDlg.ShowModal();
    }
    spdlog::info("XSQ file done.");

    if (mBackupOnSave) {
        DoBackup(false);
    }

    if (mRenderOnSave) {

        // make sure any pending layout work is done before rendering
        while (!DoAllWork()) {}

        SetStatusText(_("Saving ") + xlightsFilename + _(" ... Rendering."));

        // If number of channels is wrong then lets just dump and reallocate before render
        if ((_seqData.NumChannels() != roundTo4(GetMaxNumChannels())) ||
            (_seqData.FrameTime() != (unsigned int)CurrentSeqXmlFile->GetFrameMS()) )
        {
            spdlog::info("Render on Save: Number of channels was wrong ... reallocating sequence data memory before rendering and saving.");

            //need to abort any render going on in order to change the SeqData size
            AbortRender();

            wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
            int ms = wxAtoi(mss);

            _seqData.init(GetMaxNumChannels(), CurrentSeqXmlFile->GetSequenceDurationMS() / ms, ms);
        }

        ProgressBar->Show();
        GaugeSizer->Layout();
        spdlog::info("Rendering on save.");
        RenderIseqData(true, nullptr); // render ISEQ layers below the Nutcracker layer
        spdlog::info("   iseq below effects done.");
        ProgressBar->SetValue(10);
        RenderGridToSeqData([this, sw] (bool aborted) {
            
            spdlog::info("   Effects done.");
            ProgressBar->SetValue(90);
            RenderIseqData(false, nullptr);  // render ISEQ layers above the Nutcracker layer
            spdlog::info("   iseq above effects done. Render complete.");
            ProgressBar->SetValue(100);
            _appProgress->SetValue(0);
            _appProgress->Reset();
            ProgressBar->Hide();
            GaugeSizer->Layout();

            spdlog::info("Saving fseq file.");
            LogMemoryUsage("SaveSequence fseq save: " + xlightsFilename.ToStdString());
            SetStatusText(_("Saving ") + xlightsFilename + _(" ... Writing fseq."));
            WriteFalconPiFile(xlightsFilename);
            spdlog::info("fseq file done.", true);
            DisplayXlightsFilename(xlightsFilename);
            float elapsedTime = sw.Time()/1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
            wxString displayBuff = wxString::Format(_("%s     Updated in %7.3f seconds"),xlightsFilename,elapsedTime);
            spdlog::info(displayBuff.ToStdString());
            CallAfter(&xLightsFrame::SetStatusText, displayBuff, 0);
            EnableSequenceControls(true);
            mSavedChangeCount = _sequenceElements.GetChangeCount();
            mLastAutosaveCount = mSavedChangeCount;
        } );
        return;
    }
    wxString display_name;
    if (mSaveFseqOnSave) {
        LogMemoryUsage("SaveSequence fseq save (no render): " + xlightsFilename.ToStdString());
        SetStatusText(_("Saving ") + xlightsFilename + _(" ... Writing fseq."));
        WriteFalconPiFile(xlightsFilename, true);
        spdlog::info("fseq file done.");
        DisplayXlightsFilename(xlightsFilename);
        display_name = xlightsFilename;
    } else {
        display_name = CurrentSeqXmlFile->GetFullPath();
    }
    float elapsedTime = sw.Time() / 1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
    wxString displayBuff = wxString::Format(_("%s     Updated in %7.3f seconds"), display_name, elapsedTime);
    spdlog::info(displayBuff.ToStdString());
    CallAfter(&xLightsFrame::SetStatusText, displayBuff, 0);
    EnableSequenceControls(true);
    mSavedChangeCount = _sequenceElements.GetChangeCount();
    mLastAutosaveCount = mSavedChangeCount;
}

void xLightsFrame::SetSequenceTiming(int timingMS)
{
    if (CurrentSeqXmlFile == nullptr)
        return;

    if (_seqData.FrameTime() != (unsigned int)timingMS) {
        AbortRender();
        _seqData.init(GetMaxNumChannels(), CurrentSeqXmlFile->GetSequenceDurationMS() / timingMS, timingMS);
    }
}

void xLightsFrame::SaveAsSequence()
{
    if (readOnlyMode) {
		DisplayError("Sequences cannot be saved in read only mode!", this);
		return;
	}

    if (_seqData.NumFrames() == 0) {
        DisplayError("You must open a sequence first!", this);
        return;
    }
    wxString newFilename;
    wxFileDialog fd(this,
                    "Choose filename to Save Sequence:",
                    CurrentDir,
                    CurrentSeqXmlFile->GetName(),
                    strSequenceSaveAsFileTypes,
                    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    bool ok;
    do {
        if (fd.ShowModal() != wxID_OK) {
            return;
        }
        // validate inputs
        newFilename = fd.GetPath();
        newFilename.Trim();
        ok = true;
        if (newFilename.IsEmpty()) {
            ok = false;
            DisplayError("File name cannot be empty", this);
        }
        if (ok) {
            wxFileName fnFile(newFilename);
            fnFile.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG | wxPATH_NORM_SHORTCUT);
            wxFileName fnDir(CurrentDir, "");
            fnDir.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG | wxPATH_NORM_SHORTCUT);
            wxString filePath = fnFile.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
            wxString showPath = fnDir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
            if (!filePath.StartsWith(showPath)) {
                ok = false;
                DisplayError("Sequence files must be saved within the current show directory:\n" + showPath +
                             "\n\nPlease choose a location inside the show directory.", this);
                fd.SetDirectory(CurrentDir);
            }
        }
    } while (!ok);

    SaveAsSequence(ToStdString(newFilename));
}

void xLightsFrame::SaveAsSequence(const std::string& filename)
{
    wxFileName oName(filename);
    oName.SetExt("fseq");
    DisplayXlightsFilename(oName.GetFullPath());

    SetPanelSequencerLabel(ToStdString(oName.GetName()));

    oName.SetExt("xsq");
    CurrentSeqXmlFile->SetFullPath(ToStdString(oName.GetFullPath()));
    _renderCache.SetSequence(renderCacheDirectory, oName.GetName());
    SaveSequence();
    SetTitle(xlights_base_name + xlights_qualifier + " - " + filename);
    AddToMRU(filename);
    UpdateRecentFilesList(false);
}

void xLightsFrame::RenderAll()
{
    

    if (!_seqData.IsValidData()) {
        spdlog::warn("Aborting render all because sequence data has not been initialised.");
        return;
    }

    mRendering = true;
    EnableSequenceControls(false);
    wxYield();      // ensure all controls are disabled.
    wxStopWatch sw; // start a stopwatch timer
    _sequenceElements.GetSequenceMedia().MarkAllUnused();
    ProgressBar->Show();
    GaugeSizer->Layout();
    SetStatusText(_("Rendering all layers"));
    SuspendAutoSave(true); // no need to auto save during render all
    spdlog::debug("Rendering all.");
    spdlog::debug("Model blending: {}", CurrentSeqXmlFile->supportsModelBlending() ? "On" : "Off");
    RenderIseqData(true, nullptr); // render ISEQ layers below the Nutcracker layer
    spdlog::info("   iseq below effects done.");
    ProgressBar->SetValue(10);
    RenderGridToSeqData([this, sw] (bool aborted) {
        spdlog::info("   Effects done.");
        ProgressBar->SetValue(90);
        RenderIseqData(false, nullptr); // render ISEQ layers above the Nutcracker layer
        spdlog::info("   iseq above effects done. Render all complete.");
        ProgressBar->SetValue(100);
        float elapsedTime = sw.Time() / 1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
        wxString displayBuff = wxString::Format(_("Rendered in %7.3f seconds"), elapsedTime);
        CallAfter(&xLightsFrame::SetStatusText, displayBuff, 0);
        mRendering = false;
        SuspendAutoSave(false);
        EnableSequenceControls(true);
        ProgressBar->Hide();
        _appProgress->SetValue(0);
        _appProgress->Reset();
        GaugeSizer->Layout();
    });
}

static void enableAllChildControls(wxWindow* parent, bool enable)
{
    for (const auto& it : parent->GetChildren()) {
        it->Enable(enable);
        enableAllChildControls(it, enable);
        if (enable && it->GetName().StartsWith("ID_VALUECURVE")) {
            wxCommandEvent e(EVT_VC_CHANGED);
            e.SetInt(-1);
            e.SetEventObject(it);
            wxPostEvent(parent, e);
        }
    }
}

static void enableAllToolbarControls(wxAuiToolBar* parent, bool enable)
{
    enableAllChildControls((wxWindow*)parent, enable);
    for (int x = 0; x < (int)parent->GetToolCount(); x++) {
        wxAuiToolBarItem* item = parent->FindToolByIndex(x);
        parent->EnableTool(item->GetId(), enable);
    }
    parent->Refresh();
}

static void enableAllMenubarControls(wxMenuBar* parent, bool enable)
{
    for (int x = 0; x < (int)parent->GetMenuCount(); x++) {
        wxMenu* menu = parent->GetMenu(x);
        for (int y = 0; y < (int)menu->GetMenuItemCount(); y++) {
            wxMenuItem* item = menu->FindItemByPosition(y);
            menu->Enable(item->GetId(), enable);
        }
    }
    parent->Refresh();
}

void xLightsFrame::EnableSequenceControls(bool enable)
{
    enableAllToolbarControls(MainToolBar, enable);
    //enableAllToolbarControls(PlayToolBar, enable && SeqData.NumFrames() > 0);
    SetAudioControls();
    bool enableSeq = enable && _seqData.NumFrames() > 0;
    bool enableSeqNotAC = enable && _seqData.NumFrames() > 0 && !IsACActive();
    enableAllToolbarControls(WindowMgmtToolbar, enableSeq);
    enableAllToolbarControls(EffectsToolBar, enableSeqNotAC);
    enableAllToolbarControls(EditToolBar, enableSeq);
    enableAllToolbarControls(ACToolbar, enableSeq);
    mainSequencer->CheckBox_SuspendRender->Enable(enableSeq);
    enableAllToolbarControls(ViewToolBar, enable);
    PlayToolBar->EnableTool(ID_CHECKBOX_LIGHT_OUTPUT, enable);

    effectsPnl->ButtonUpdateEffect->Enable(enableSeq);
    effectsPnl->BitmapButtonRandomize->Enable(enableSeq);
    effectsPnl->BitmapButtonSelectedEffect->Enable(enableSeq);

    enableAllChildControls(EffectsPanel1, enableSeqNotAC);
    if (enableSeqNotAC) EffectsPanel1->ValidateWindow();
    enableAllChildControls(blendingPanel, enableSeqNotAC);
    if (enableSeqNotAC) blendingPanel->ValidateWindow();
    enableAllChildControls(bufferPanel, enableSeqNotAC);
    if (enableSeqNotAC) bufferPanel->ValidateWindow();
    enableAllChildControls(perspectivePanel, enableSeq);
    //if (enableSeq) perspectivePanel->ValidateWindow();
    enableAllChildControls(colorPanel, enableSeqNotAC);
    if (enableSeqNotAC) { colorPanel->ValidateWindow(); }
    enableAllChildControls(effectPalettePanel, enableSeqNotAC);
    //if (enableSeqNotAC) effectPalettePanel->ValidateWindow();
    enableAllChildControls(_valueCurvesPanel, enableSeqNotAC);
    //if (enableSeqNotAC) _valueCurvesPanel->ValidateWindow();
    enableAllChildControls(_coloursPanel, enableSeqNotAC);
    //if (enableSeqNotAC) _coloursPanel->ValidateWindow();
    enableAllChildControls(jukeboxPanel, enableSeqNotAC);
    enableAllChildControls(_findDataPanel, enableSeq);
    // if (enableSeqNotAC) jukeboxPanel->ValidateWindow();
    UpdateACToolbar(enable);

    enableAllMenubarControls(MenuBar, enable);

    if (enable && _seqData.NumFrames() == 0) {
        //no file is loaded, disable save/render buttons
        EnableToolbarButton(MainToolBar, ID_AUITOOLBAR_SAVE, false);
        EnableToolbarButton(MainToolBar, ID_AUITOOLBAR_SAVEAS, false);
        EnableToolbarButton(MainToolBar, ID_AUITOOLBAR_RENDERALL, false);
        Menu_Settings_Sequence->Enable(false);
        MenuItem_File_Save->Enable(false);
        MenuItem_File_SaveAs_Sequence->Enable(false);
        MenuItem_File_Close_Sequence->Enable(false);
        MenuItem_File_Export_Video->Enable(false);
        MenuItem_PackageSequence->Enable(false);
        MenuItem_GenerateLyrics->Enable(false);
        MenuItem_ExportEffects->Enable(false);
        MenuItem_PurgeRenderCache->Enable(false);
        MenuItem_ImportEffects->Enable(false);
        MenuItemShiftEffects->Enable(false);
        MenuItemShiftSelectedEffects->Enable(false);
        MenuItemShiftEffectsAndTiming->Enable(false);
        MenuItem_ColorReplace->Enable(false);
        if (revertToMenuItem) revertToMenuItem->Enable(false);
    }
    if (!enable && _seqData.NumFrames() > 0) {
        //file is loaded, but we're doing something that requires controls disabled (such as rendering)
        //we need to also disable the quit button
        QuitMenuItem->Enable(false);
    } else {
        QuitMenuItem->Enable(true);
    }

    if (MenuItem_CrashXLights != nullptr) {
        MenuItem_CrashXLights->Enable();
    }
    if (MenuItem_LogRenderState != nullptr) {
        MenuItem_LogRenderState->Enable();
    }

    MenuItem_ViewLog->Enable(true);
}

//modified for partially random -DJ
std::string xLightsFrame::CreateEffectStringRandom(std::string& settings, std::string& palette)
{
    int eff1 = ChooseRandomEffect();
    settings = EffectsPanel1->GetRandomEffectString(eff1);
    palette = colorPanel->GetRandomColorString();
    return effectManager[eff1]->Name();
}

int xLightsFrame::ChooseRandomEffect()
{
    if (_randomEffectsToUse.size() == 0) return 0;

    const int select = rand() % _randomEffectsToUse.size();
    const wxString effect = _randomEffectsToUse[select];
    const int index = effectManager.GetEffectIndex(effect);
    return index != -1 ? index : 0;
}

void xLightsFrame::VCChanged(wxCommandEvent& event)
{
    _valueCurvesPanel->Freeze();
    if (event.GetInt() == -1) {
        _valueCurvesPanel->UpdateValueCurveButtons(true);
    }
    enableAllChildControls(_valueCurvesPanel, true); // enable and disable otherwise if anything has been added while disabled wont be disabled.
    enableAllChildControls(_valueCurvesPanel, _seqData.NumFrames() > 0 && !IsACActive());
    _valueCurvesPanel->Thaw();
}

void xLightsFrame::ColourChanged(wxCommandEvent& event)
{
    _coloursPanel->Freeze();
    if (event.GetInt() == -1) {
        _coloursPanel->UpdateColourButtons(true, this);
    }
    enableAllChildControls(_coloursPanel, true); // enable and disable otherwise if anything has been added while disabled wont be disabled.
    enableAllChildControls(_coloursPanel, _seqData.NumFrames() > 0 && !IsACActive());
    _coloursPanel->Thaw();
}
