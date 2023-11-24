/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/utils.h>
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include <wx/xml/xml.h>
#include <wx/config.h>

#include "xLightsMain.h"
#include "SeqSettingsDialog.h"
#include "xLightsXmlFile.h"
#include "effects/RenderableEffect.h"
#include "models/ModelGroup.h"
#include "models/SubModel.h"
#include "SequenceViewManager.h"
#include "LayoutPanel.h"
#include "UtilFunctions.h"
#include "ExternalHooks.h"
#include "BufferPanel.h"
#include "EffectIconPanel.h"
#include "JukeboxPanel.h"
#include "FindDataPanel.h"
#include "EffectsPanel.h"
#include "TimingPanel.h"
#include "ColorPanel.h"
#include "LayoutGroup.h"
#include "ModelPreview.h"
#include "ViewsModelsPanel.h"
#include "PerspectivesPanel.h"
#include "ValueCurvesPanel.h"
#include "ColoursPanel.h"
#include "sequencer/MainSequencer.h"
#include "HousePreviewPanel.h"
#include "ExternalHooks.h"

#include "xLightsVersion.h"
#include "TopEffectsPanel.h"

#include <log4cpp/Category.hh>

void xLightsFrame::DisplayXlightsFilename(const wxString& filename) const
{
    xlightsFilename = filename;
    FileNameText->SetLabel(filename);
}

void xLightsFrame::OnBitmapButtonOpenSeqClick(wxCommandEvent& event)
{
    OpenSequence("", nullptr);
}

void xLightsFrame::OnButtonNewSequenceClick(wxCommandEvent& event)
{
    NewSequence();
	EnableSequenceControls(true);
}

void xLightsFrame::ResetEffectsXml()
{
    _sequenceViewManager.Reset();
    ModelsNode = nullptr;
    ViewObjectsNode = nullptr;
    EffectsNode = nullptr;
    PalettesNode = nullptr;
    ModelGroupsNode = nullptr;
    LayoutGroupsNode = nullptr;
    SettingsNode = nullptr;
    PerspectivesNode = nullptr;
}

wxString xLightsFrame::LoadEffectsFileNoCheck()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    ResetEffectsXml();
    wxFileName effectsFile;
    effectsFile.AssignDir(CurrentDir);
    effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
    wxString myString = "Hello";
    UnsavedRgbEffectsChanges = false;

    if (!FileExists(effectsFile)) {
        // file does not exist, so create an empty xml doc
        CreateDefaultEffectsXml();
    }
    else {
        // check if there is a autosave backup file which is newer than the file we have been asked to open
        wxFileName fn(effectsFile.GetFullPath());
        wxFileName xx = fn;
        xx.SetExt("xbkp");
        wxString asfile = xx.GetLongPath();

        if (((!_renderMode && !_checkSequenceMode) || _promptBatchRenderIssues) && FileExists(asfile)) {
            // the autosave file exists
            wxDateTime xmltime = fn.GetModificationTime();
            wxFileName asfn(asfile);
            wxDateTime xbkptime = asfn.GetModificationTime();

            if (xbkptime > xmltime) {
                // autosave file is newer
                if (wxMessageBox("Autosaved rgbeffects file found which seems to be newer than your current rgbeffects file ... would you like to open that instead?", "Newer file found", wxYES_NO) == wxYES) {
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

        if (!EffectsXml.Load(effectsFile.GetFullPath())) {
            DisplayError("Unable to load RGB effects file ... creating a default one.", this);
            CreateDefaultEffectsXml();
        }
        wxXmlDoctype dt("");
        EffectsXml.SetDoctype(dt);
    }

    wxXmlNode* root = EffectsXml.GetRoot();
    if (root->GetName() != "xrgb") {
        DisplayError("Invalid RGB effects file ... creating a default one.", this);
        CreateDefaultEffectsXml();
    }

    ModelsNode = EffectsNode = PalettesNode = ModelGroupsNode = LayoutGroupsNode = SettingsNode = PerspectivesNode = nullptr;
    wxXmlNode* viewsNode = nullptr;
    wxXmlNode* colorsNode = nullptr;
    wxXmlNode* viewpointsNode = nullptr;
    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "models") ModelsNode = e;
        if (e->GetName() == "view_objects") ViewObjectsNode = e;
        if (e->GetName() == "effects") EffectsNode = e;
        if (e->GetName() == "palettes") PalettesNode = e;
        if (e->GetName() == "views") viewsNode = e;
        if (e->GetName() == "colors") colorsNode = e;
        if (e->GetName() == "Viewpoints") viewpointsNode = e;
        if (e->GetName() == "modelGroups") ModelGroupsNode = e;
        if (e->GetName() == "layoutGroups") LayoutGroupsNode = e;
        if (e->GetName() == "settings") SettingsNode = e;
        if (e->GetName() == "perspectives") PerspectivesNode = e;
    }

    // This is the earliest we can do the backup as now the settings node will be populated
    _backupDirectory = GetXmlSetting("backupDir", showDirectory);
    ObtainAccessToURL(_backupDirectory);
    if (!wxDir::Exists(_backupDirectory)) {
        logger_base.warn("Backup Directory not Found ... switching to Show Directory.");
        _backupDirectory = showDirectory;
        SetXmlSetting("backupDir", showDirectory);
        UnsavedRgbEffectsChanges = true;
    }

    if (ModelsNode == nullptr) {
        ModelsNode = new wxXmlNode(wxXML_ELEMENT_NODE, "models");
        root->AddChild(ModelsNode);
        UnsavedRgbEffectsChanges = true;
    }
    if (ViewObjectsNode == nullptr) {
        ViewObjectsNode = new wxXmlNode(wxXML_ELEMENT_NODE, "view_objects");
        root->AddChild(ViewObjectsNode);
        wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "view_object");
        ViewObjectsNode->AddChild(node);
        node->AddAttribute("DisplayAs", "Gridlines");
        node->AddAttribute("LayoutGroup", "Default");
        node->AddAttribute("name", "Gridlines");
        node->AddAttribute("GridLineSpacing", "50");
        node->AddAttribute("GridWidth", "2000.0");
        node->AddAttribute("GridHeight", "1000.0");
        node->AddAttribute("WorldPosX", "0.0000");
        node->AddAttribute("WorldPosY", "0.0000");
        node->AddAttribute("WorldPosZ", "0.0000");
        node->AddAttribute("ScaleX", "1.0000");
        node->AddAttribute("ScaleY", "1.0000");
        node->AddAttribute("ScaleZ", "1.0000");
        node->AddAttribute("RotateX", "90.0");
        node->AddAttribute("RotateY", "0");
        node->AddAttribute("RotateZ", "0");
        node->AddAttribute("versionNumber", "3");
        node->AddAttribute("Active", "1");
        UnsavedRgbEffectsChanges = true;
    }
    if (EffectsNode == nullptr) {
        EffectsNode = new wxXmlNode(wxXML_ELEMENT_NODE, "effects");
        EffectsNode->AddAttribute("version", XLIGHTS_RGBEFFECTS_VERSION);
        root->AddChild(EffectsNode);
        UnsavedRgbEffectsChanges = true;
    }
    if (PalettesNode == nullptr) {
        PalettesNode = new wxXmlNode(wxXML_ELEMENT_NODE, "palettes");
        root->AddChild(PalettesNode);
        UnsavedRgbEffectsChanges = true;
    }

    if (viewsNode == nullptr) {
        UnsavedRgbEffectsChanges = true;
    } else {
        _sequenceViewManager.Load(viewsNode, _sequenceElements.GetCurrentView());
    }

    _defaultSeqView = GetXmlSetting("defaultSeqView", "");

    if (_sequenceViewManager.GetViewIndex(_defaultSeqView) == -1 && _defaultSeqView != "") {
        logger_base.warn("View Not Found ... clearing");
        _defaultSeqView.clear();
        SetXmlSetting("defaultSeqView", _defaultSeqView);
        UnsavedRgbEffectsChanges = true;
    }

    if (colorsNode != nullptr) {
        color_mgr.Load(colorsNode);
    }

    if (viewpointsNode != nullptr) {
        viewpoint_mgr.Load(viewpointsNode);
        layoutPanel->GetMainPreview()->RestoreDefaultCameraPosition();
        _housePreviewPanel->GetModelPreview()->RestoreDefaultCameraPosition();
    }

    if (ModelGroupsNode == nullptr) {
        ModelGroupsNode = new wxXmlNode(wxXML_ELEMENT_NODE, "modelGroups");
        root->AddChild(ModelGroupsNode);
        UnsavedRgbEffectsChanges = true;
    }

    if (LayoutGroupsNode == nullptr) {
        LayoutGroupsNode = new wxXmlNode(wxXML_ELEMENT_NODE, "layoutGroups");
        root->AddChild(LayoutGroupsNode);
        UnsavedRgbEffectsChanges = true;
    }

    if (PerspectivesNode == nullptr) {
        PerspectivesNode = new wxXmlNode(wxXML_ELEMENT_NODE, "perspectives");
        root->AddChild(PerspectivesNode);
        UnsavedRgbEffectsChanges = true;
    }

    if (SettingsNode == nullptr) {
        SettingsNode = new wxXmlNode(wxXML_ELEMENT_NODE, "settings");
        root->AddChild(SettingsNode);
        SetXmlSetting("previewWidth", "1280");
        SetXmlSetting("previewHeight", "720");
        UnsavedRgbEffectsChanges = true;
    }
    int previewWidth = wxAtoi(GetXmlSetting("previewWidth", "1280"));
    int previewHeight = wxAtoi(GetXmlSetting("previewHeight", "720"));
    if (previewWidth == 0 || previewHeight == 0) {
        previewWidth = 1280;
        previewHeight = 720;
    }
    SetPreviewSize(previewWidth, previewHeight);

    mBackgroundImage = FixFile(GetShowDirectory(), GetXmlSetting("backgroundImage", ""));
    ObtainAccessToURL(mBackgroundImage.ToStdString());
    if (mBackgroundImage != "" && (!FileExists(mBackgroundImage) || !wxIsReadable(mBackgroundImage))) {
        //image doesn't exist there, lets look for it in media directories
        wxString bgImg = GetXmlSetting("backgroundImage", "");
        for (auto &dir : mediaDirectories) {
            wxString fn = FixFile(dir, bgImg);
            ObtainAccessToURL(fn.ToStdString());
            if (FileExists(fn) && wxIsReadable(fn)) {
                mBackgroundImage = fn;
                break;
            }
        }
        if (!FileExists(mBackgroundImage) || !wxIsReadable(mBackgroundImage)) {
            wxFileName name(mBackgroundImage);
            name.SetPath(CurrentDir);
            if (name.Exists()) {
                mBackgroundImage = name.GetFullPath();
                ObtainAccessToURL(name.GetFullPath().ToStdString());
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
    SetDisplay2DGridSpacing(wxAtol(GetXmlSetting("Display2DGridSpacing", "100")));
    layoutPanel->SetDisplay2DGridSpacing(GetDisplay2DGrid(), GetDisplay2DGridSpacing());

    //Load FSEQ and Backup directory settings
    fseqDirectory = GetXmlSetting("fseqDir", showDirectory);
    renderCacheDirectory = GetXmlSetting("renderCacheDir", fseqDirectory); // we user fseq directory if no setting is present
    ObtainAccessToURL(renderCacheDirectory);
    ObtainAccessToURL(fseqDirectory);
    if (!wxDir::Exists(fseqDirectory)) {
        logger_base.warn("FSEQ Directory not Found ... switching to Show Directory.");
        fseqDirectory = showDirectory;
        SetXmlSetting("fseqDir", showDirectory);
        UnsavedRgbEffectsChanges = true;
    }
    FseqDir = fseqDirectory;
    if (!wxDir::Exists(renderCacheDirectory)) {
        logger_base.warn("Render Cache Directory not Found ... switching to Show Directory.");
        renderCacheDirectory = showDirectory;
        SetXmlSetting("renderCacheDir", showDirectory);
        UnsavedRgbEffectsChanges = true;
    }

    mStoredLayoutGroup = GetXmlSetting("storedLayoutGroup", "Default");

    // validate stored preview exists
    bool found_saved_preview = false;
    for (wxXmlNode* e = LayoutGroupsNode->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "layoutGroup") {
            wxString grp_name = e->GetAttribute("name");
            if (!grp_name.IsEmpty()) {
                if (grp_name.ToStdString() == mStoredLayoutGroup) {
                    found_saved_preview = true;
                }
            }
        }
    }
    if (!found_saved_preview) {
        mStoredLayoutGroup = "Default";
    }

    // Do this here as it may switch the background image
    LayoutGroups.clear();
    layoutPanel->Reset();
    AllModels.SetLayoutsNode(LayoutGroupsNode);  // provides easy access to layout names for the model class
    for (wxXmlNode* e = LayoutGroupsNode->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "layoutGroup") {
            wxString grp_name = e->GetAttribute("name");
            if (!grp_name.IsEmpty()) {
                LayoutGroup* grp = new LayoutGroup(grp_name.ToStdString(), this, e);
                LayoutGroups.push_back(grp);
                AddPreviewOption(grp);
                layoutPanel->AddPreviewChoice(grp_name.ToStdString());
                //if( grp_name.ToStdString() == mStoredLayoutGroup )
                //{
                //    found_saved_preview = true;
                //}
            }
        }
    }

    mBackgroundBrightness = wxAtoi(GetXmlSetting("backgroundBrightness", "100"));
    mBackgroundAlpha = wxAtoi(GetXmlSetting("backgroundAlpha", "100"));
    SetPreviewBackgroundBrightness(mBackgroundBrightness, mBackgroundAlpha);
    mScaleBackgroundImage = wxAtoi(GetXmlSetting("scaleImage", "0")) > 0;
    SetPreviewBackgroundScaled(mScaleBackgroundImage);

    std::string group = layoutPanel->GetCurrentLayoutGroup();
    if (group != "Default" && group != "All Models" && group != "Unassigned") {
        modelPreview->SetBackgroundBrightness(layoutPanel->GetBackgroundBrightnessForSelectedPreview(), layoutPanel->GetBackgroundAlphaForSelectedPreview());
        modelPreview->SetScaleBackgroundImage(layoutPanel->GetBackgroundScaledForSelectedPreview());
    }

    UpdateLayoutSave();
    UpdateControllerSave();

    return effectsFile.GetFullPath();
}

void xLightsFrame::LoadEffectsFile()
{
    // Clear out all the models before we load new ones
    AllModels.clear();

    wxStopWatch sw; // start a stopwatch timer
    wxString filename = LoadEffectsFileNoCheck();
    // check version, do we need to convert?
    wxString version = EffectsNode->GetAttribute("version", "0000");
    if (version < "0004")
    {
        // fix tags
        xLightsXmlFile::FixVersionDifferences(filename);

        // load converted file
        LoadEffectsFileNoCheck();

        // fix effect presets
        xLightsXmlFile::FixEffectPresets(EffectsNode);

        UnsavedRgbEffectsChanges = true;
    }
    if (version < "0005") {
        //flip to AntiAlias=1 by default
        for (wxXmlNode *el = ModelsNode->GetChildren(); el != nullptr; el = el->GetNext()) {
            if (el->GetAttribute("Antialias", "1") == "0") {
                el->DeleteAttribute("Antialias");
                el->AddAttribute("Antialias", "1");
                UnsavedRgbEffectsChanges = true;
            }
        }
    }
    if (version < "0006") {
        // need to convert models/groups to remove MyDisplay and add preview assignments
        for (wxXmlNode *model = ModelsNode->GetChildren(); model != nullptr; model = model->GetNext()) {
            if (model->GetName() == "model") {
                std::string my_display = model->GetAttribute("MyDisplay").ToStdString();
                std::string layout_group = "Unassigned";
                if (my_display == "1") {
                    layout_group = "Default";
                }
                model->DeleteAttribute("MyDisplay");
                model->DeleteAttribute("LayoutGroup");
                model->AddAttribute("LayoutGroup", layout_group);
            }
        }
        // parse groups once to figure out if any of them are selected
        bool groups_are_selected = false;
        for (wxXmlNode *group = ModelGroupsNode->GetChildren(); group != nullptr; group = group->GetNext()) {
            if (group->GetName() == "modelGroup") {
                std::string selected = group->GetAttribute("selected").ToStdString();
                if (selected == "1") {
                    groups_are_selected = true;
                    break;
                }
            }
        }
        // if no groups are selected then models remain as set above and all groups goto Default
        if (!groups_are_selected) {
            for (wxXmlNode *group = ModelGroupsNode->GetChildren(); group != nullptr; group = group->GetNext()) {
                if (group->GetName() == "modelGroup") {
                    group->DeleteAttribute("selected");
                    group->DeleteAttribute("LayoutGroup");
                    group->AddAttribute("LayoutGroup", "Default");
                }
            }
        }
        else { // otherwise need to set models in unchecked groups to unassigned
            std::set<std::string> modelsAdded;
            for (wxXmlNode *group = ModelGroupsNode->GetChildren(); group != nullptr; group = group->GetNext()) {
                if (group->GetName() == "modelGroup") {
                    std::string selected = group->GetAttribute("selected").ToStdString();
                    std::string layout_group = "Unassigned";
                    if (selected == "1") {
                        wxArrayString mn = wxSplit(group->GetAttribute("models"), ',');
                        for (int x = 0; x < mn.size(); x++) {
                            std::string name = mn[x].Trim(true).Trim(false).ToStdString();
                            if (modelsAdded.find(name) == modelsAdded.end()) {
                                modelsAdded.insert(mn[x].Trim(true).Trim(false).ToStdString());
                            }
                        }
                        layout_group = "Default";
                    }
                    group->DeleteAttribute("selected");
                    group->DeleteAttribute("LayoutGroup");
                    group->AddAttribute("LayoutGroup", layout_group);
                }
            }
            // now move models back to unassigned that were not part of a checked group
            for (wxXmlNode *model = ModelsNode->GetChildren(); model != nullptr; model = model->GetNext()) {
                if (model->GetName() == "model") {
                    std::string mn = model->GetAttribute("name").ToStdString();
                    if (modelsAdded.find(mn) == modelsAdded.end()) {
                        model->DeleteAttribute("LayoutGroup");
                        model->AddAttribute("LayoutGroup", "Unassigned");
                    }
                }
            }
        }
        UnsavedRgbEffectsChanges = true;
    }

    if (version < "0007") {
        // fix any no longer supported smart remote settings *A*->*B*->*C* and a->*B*->*C*
        for (wxXmlNode* model = ModelsNode->GetChildren(); model != nullptr; model = model->GetNext()) {
            if (model->GetName() == "model") {
                for (wxXmlNode* cc = model->GetChildren(); cc != nullptr; cc = cc->GetNext()) {
                    auto sr = cc->GetAttribute("SmartRemote", "0");
                    if (sr == "4") {
                        cc->DeleteAttribute("SmartRemote");
                        cc->AddAttribute("SmartRemote", "1");
                        cc->AddAttribute("SRMaxCascade", "3");
                    }
                    else if (sr == "5") {
                        cc->DeleteAttribute("SmartRemote");
                        cc->AddAttribute("SmartRemote", "2");
                        cc->AddAttribute("SRMaxCascade", "2");
                    }
                }
            }
        }
    }

    // update version
    EffectsNode->DeleteAttribute("version");
    EffectsNode->AddAttribute("version", XLIGHTS_RGBEFFECTS_VERSION);

    // Handle upgrade of networks file to the controller/output structure
    bool converted = _outputManager.ConvertModelStartChannels(ModelsNode);

    displayElementsPanel->SetSequenceElementsModelsViews(&_seqData, &_sequenceElements, ModelsNode, ModelGroupsNode, &_sequenceViewManager);
    layoutPanel->ClearUndo();
    GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LoadEffectsFile");
    mSequencerInitialize = false;

    // load the perspectives
    CheckForAndCreateDefaultPerpective();
    perspectivePanel->SetPerspectives(PerspectivesNode);
    LoadPerspectivesMenu(PerspectivesNode);

    float elapsedTime = sw.Time() / 1000.0; //msec => sec
    SetStatusText(wxString::Format(_("'%s' loaded in %4.3f sec."), filename, elapsedTime));

    UpdateLayoutSave();
    UpdateControllerSave();

    if (converted) {
        UnsavedRgbEffectsChanges = true;
        wxMessageBox("Your setup tab data has been converted to the new controller centric format.\nIf you choose to save either the Controller (Setup) or Layout Tab data it is critical you save both or some of your model start channels will break.\nIf this happens you can either repair them manually or roll back to a backup copy.");
    }
}

void xLightsFrame::LoadPerspectivesMenu(wxXmlNode* perspectivesNode)
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

    for (wxXmlNode* p = perspectivesNode->GetChildren(); p != nullptr; p = p->GetNext()) {
        if (p->GetName() == "perspective") {
            wxString name = p->GetAttribute("name");
            if (!name.IsEmpty()) {
                int id = wxNewId();
                MenuItemPerspectives->AppendRadioItem(id, name);
                if (mCurrentPerpective != nullptr && (name == mCurrentPerpective->GetAttribute("name")))
                    MenuItemPerspectives->Check(id, true);
                PerspectiveId pmenu;
                pmenu.id = id;
                pmenu.p = p;
                perspectives[pCount] = pmenu;
                Connect(id, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemLoadPerspectiveSelected);
                pCount++;
                if (pCount >= 10) {
                    return;
                }
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
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string filename = CurrentDir.ToStdString() + "/xScheduleData/GetModels.dat";

    if (!wxDir::Exists(CurrentDir + "/xScheduleData")) {
        logger_base.debug("Creating xScheduleData folder.");
        wxDir sd(CurrentDir);
        sd.Make(CurrentDir + "/xScheduleData");
    }

    logger_base.debug("Creating models JSON file: %s.", (const char*)filename.c_str());

    wxFile modelsJSON;
    if (!modelsJSON.Create(filename, true) || !modelsJSON.IsOpened()) {
        logger_base.error("Unable to create file: %s.", (const char*)filename.c_str());
        return;
    }

    modelsJSON.Write("{\"models\":[");

    bool first = true;
    for (auto m = AllModels.begin(); m != AllModels.end(); ++m) {
        Model* model = m->second;
        if (model->GetDisplayAs() == "ModelGroup") {
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
        } else if (model->GetDisplayAs() == "SubModel") {
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
                             "\",\"type\":\"" + model->GetDisplayAs() +
                             "\",\"startchannel\":\"" + wxString::Format("%ld", (long)ch) +
                             "\",\"channels\":\"" + wxString::Format("%ld", (long)model->GetChanCount()) +
                             "\",\"stringtype\":\"" + model->GetStringType() + "\"}");
        }
    }

    modelsJSON.Write("]}");

    logger_base.debug("     models JSON file done.");
}

// returns true on success
bool xLightsFrame::SaveEffectsFile(bool backup)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // dont save if currently saving
    std::unique_lock<std::mutex> lock(saveLock, std::try_to_lock);
    if (!lock.owns_lock())
        return false;

    // Make sure the views are up to date before we save it
    _sequenceViewManager.Save(&EffectsXml);

    color_mgr.Save(&EffectsXml);

    viewpoint_mgr.Save(&EffectsXml);

    wxFileName effectsFile;
    effectsFile.AssignDir(CurrentDir);
    if (backup) {
        effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE_BACKUP));
    } else {
        effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
    }

    if (!EffectsXml.Save(effectsFile.GetFullPath())) {
        if (backup) {
            logger_base.warn("Unable to save backup of RGB effects file");
        } else {
            DisplayError("Unable to save RGB effects file", this);
        }
        return false;
    }

    if (!backup) {
#ifndef __WXOSX__
        SaveModelsFile();
#endif
        UnsavedRgbEffectsChanges = false;
    }

    UpdateLayoutSave();
    UpdateControllerSave();

    return true;
}

void xLightsFrame::CreateDefaultEffectsXml()
{
    wxXmlNode* root = new wxXmlNode( wxXML_ELEMENT_NODE, "xrgb" );
    EffectsXml.SetRoot( root );
    wxXmlDoctype dt("");
    EffectsXml.SetDoctype(dt);
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
        if (elementToCheck->GetSubModelCount() == submodelOrder.size()) {
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
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool internalsChanged = false;

    if (OldName == NewName) {
        return false;
    }
    AbortRender();

    logger_base.debug("Renaming model '%s' to '%s'.", (const char*)OldName.c_str(), (const char*)NewName.c_str());

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
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool internalsChanged = false;

    if (OldName == NewName) {
        return false;
    }

    logger_base.debug("Renaming object '%s' to '%s'.", (const char*)OldName.c_str(), (const char*)NewName.c_str());

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

void xLightsFrame::UpdateModelsList()
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("        UpdateModelsList.");

    if (ModelsNode == nullptr 
        || ViewObjectsNode == nullptr
        || modelPreview == nullptr) return; // this happens when xlights is first loaded

    //abort any render as it will crash if the model changes
    AbortRender();

    playModel = nullptr;
    PreviewModels.clear();
    UnselectEffect();
    modelsChangeCount++;
    AllModels.LoadModels(ModelsNode,
        modelPreview->GetVirtualCanvasWidth(),
        modelPreview->GetVirtualCanvasHeight());

    AllObjects.LoadViewObjects(ViewObjectsNode);

    std::vector<std::string> current;
    for (const auto& it : AllModels) {
        current.push_back(it.first);
    }
    for (wxXmlNode* e = ModelGroupsNode->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "modelGroup") {
            std::string name = e->GetAttribute("name").Trim(true).Trim(false).ToStdString();
            current.push_back(name);
        }
    }
    for (wxXmlNode* e = ModelGroupsNode->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "modelGroup") {
            std::string name = e->GetAttribute("name").Trim(true).Trim(false).ToStdString();
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
                        for (wxXmlNode* e2 = ModelsNode->GetChildren(); e2 != nullptr; e2 = e2->GetNext()) {
                            if (e2->GetName() == "model") {
                                std::string mname = e2->GetAttribute("name").Trim(true).Trim(false).ToStdString();
                                if (mname == name) {
                                    UnsavedRgbEffectsChanges = true;
                                    if (sel == 1) {
                                        ModelsNode->RemoveChild(e2);
                                        done = true;
                                    }
                                    else {
                                        //rename
                                        std::string newName = chooseNewName(this, current, "Rename Model", mname);
                                        if (newName != mname) {
                                            current.push_back(newName);
                                            e2->DeleteAttribute("name");
                                            e2->AddAttribute("name", newName);
                                            done = true;
                                        }
                                    }
                                    AllModels.LoadModels(ModelsNode,
                                        modelPreview->GetVirtualCanvasWidth(),
                                        modelPreview->GetVirtualCanvasHeight());
                                }
                            }
                        }
                        break;
                    case 2: {
                        std::string newName = chooseNewName(this, current, "Rename Model Group", name);
                        if (newName != name) {
                            current.push_back(newName);
                            e->DeleteAttribute("name");
                            e->AddAttribute("name", newName);
                            done = true;
                        }
                    }
                          break;
                    case 3:
                        ModelGroupsNode->RemoveChild(e);
                        e = ModelGroupsNode->GetChildren();
                        UnsavedRgbEffectsChanges = true;
                        done = true;
                        if (e == nullptr) {
                            break;
                        }
                        break;
                    default:
                        break;
                    }
                } while (!done);
            }
        }
    }
    AllModels.LoadGroups(ModelGroupsNode,
        modelPreview->GetVirtualCanvasWidth(),
        modelPreview->GetVirtualCanvasHeight());

    wxString msg;

    // Add all models to default House Preview that are set to Default or All Previews
    for (const auto& it : AllModels) {
        Model* model = it.second;
        if (model->GetDisplayAs() != "ModelGroup") {
            if (model->GetLayoutGroup() == "Default" || model->GetLayoutGroup() == "All Previews") {
                PreviewModels.push_back(model);
            }
        }
    }

    // Now add all models to default House Preview that are in groups set to Default or All Previews
    for (const auto& it : AllModels) {
        Model* model = it.second;
        if (model->GetDisplayAs() == "ModelGroup") {
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
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string file;

    if (origFilename != "") {

        EnableSequenceControls(false);

        wxString seq = origFilename;

        printf("Processing file %s\n", (const char*)seq.c_str());
        logger_base.debug("Batch Check sequence processing file %s\n", (const char*)seq.c_str());
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
        logger_base.debug("Check sequence done.");
        CloseSequence();
    }

    return file;
}

void xLightsFrame::OpenAndCheckSequence(const wxArrayString& origFilenames, bool exitOnDone)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (origFilenames.IsEmpty()) {
        _checkSequenceMode = false;
        EnableSequenceControls(true);
        logger_base.debug("Batch check sequence done.");
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
        logger_base.debug("Batch render cancelled.");
        EnableSequenceControls(true);
        printf("Batch render cancelled.\n");
        if (exitOnDone) {
            Destroy();
        }
        else {
            CloseSequence();
        }
        return;
    }

    EnableSequenceControls(false);

    wxArrayString fileNames = origFilenames;
    wxString seq = fileNames[0];
    fileNames.RemoveAt(0);

    printf("Processing file %s\n", (const char*)seq.c_str());
    logger_base.debug("Batch Check sequence processing file %s\n", (const char*)seq.c_str());
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
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (origFilenames.IsEmpty()) {
        _lowDefinitionRender = _saveLowDefinitionRender;
        _renderMode = false;
        EnableSequenceControls(true);
        logger_base.debug("Batch render done.");
        printf("Done All Files\n");
        if (exitOnDone) {
            Destroy();
        } else {
            CloseSequence();
        }
        return;
    }

    if (wxGetKeyState(WXK_ESCAPE))
    {
        logger_base.debug("Batch render cancelled.");
        EnableSequenceControls(true);
        printf("Batch render cancelled.\n");
        if (exitOnDone) {
            Destroy();
        }
        else {
            CloseSequence();
        }
        return;
    }

    EnableSequenceControls(false);

    wxArrayString fileNames = origFilenames;
    wxString seq = fileNames[0];
    wxStopWatch sw; // start a stopwatch timer

    printf("Processing file %s\n", (const char *)seq.c_str());
    logger_base.debug("Batch Render Processing file %s\n", (const char *)seq.c_str());
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
    logger_base.info("Rendering on save.");
    RenderIseqData(true, nullptr); // render ISEQ layers below the Nutcracker layer
    logger_base.info("   iseq below effects done.");
    ProgressBar->SetValue(10);
    RenderGridToSeqData([this, sw, fileNames, exitOnDone, alreadyRetried] (bool aborted) {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("   Effects done.");
        ProgressBar->SetValue(90);
        RenderIseqData(false, nullptr);  // render ISEQ layers above the Nutcracker layer
        logger_base.info("   iseq above effects done. Render complete.");
        ProgressBar->SetValue(100);
        ProgressBar->Hide();
        _appProgress->SetValue(0);
        _appProgress->Reset();
        GaugeSizer->Layout();

        if (!aborted || alreadyRetried) {
            logger_base.info("Saving fseq file.");
            SetStatusText(_("Saving ") + xlightsFilename + _(" ... Writing fseq."));
            WriteFalconPiFile(xlightsFilename);
            logger_base.info("fseq file done.");
            DisplayXlightsFilename(xlightsFilename);
            float elapsedTime = sw.Time() / 1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
            wxString displayBuff = wxString::Format(_("%s     Updated in %7.3f seconds"), xlightsFilename, elapsedTime);
            logger_base.info("%s", (const char*)displayBuff.c_str());
            CallAfter(&xLightsFrame::SetStatusText, displayBuff, 0);
            mSavedChangeCount = _sequenceElements.GetChangeCount();
            mLastAutosaveCount = mSavedChangeCount;

            auto nFileNames = fileNames;
            nFileNames.RemoveAt(0);
            CallAfter(&xLightsFrame::OpenRenderAndSaveSequencesF, nFileNames, (exitOnDone ? RENDER_EXIT_ON_DONE : 0));
        } else {
            logger_base.info("Render was aborted, retrying.");
            CallAfter(&xLightsFrame::OpenRenderAndSaveSequencesF, fileNames, (exitOnDone ? RENDER_EXIT_ON_DONE : 0) | RENDER_ALREADY_RETRIED);
        }
    } );
}

void xLightsFrame::SaveSequence()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_seqData.NumFrames() == 0)
    {
        DisplayError("You must open a sequence first!", this);
        return;
    }

    wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
    wxPostEvent(this, playEvent);

    std::unique_lock<std::mutex> lock(saveLock);

    if (xlightsFilename.IsEmpty()) {
        wxString NewFilename;

        wxString startname = CurrentSeqXmlFile->GetName();
        if (startname.IsEmpty() && !CurrentSeqXmlFile->GetMediaFile().empty()) {
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
        CurrentSeqXmlFile->SetPath(xmlFileName.GetPath());
        CurrentSeqXmlFile->SetFullName(xmlFileName.GetFullName());

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

    if (CurrentSeqXmlFile->GetExt().Lower() == "xml") {
        // Remove the old xml file as we are about to save it as an xsq
        wxRemoveFile(CurrentSeqXmlFile->GetFullPath());
        CurrentSeqXmlFile->SetExt("xsq");
    } else if (CurrentSeqXmlFile->GetExt().Lower() == "xbkp") {
        CurrentSeqXmlFile->SetExt("xsq");
    }
    SetStatusText(_("Saving ") + CurrentSeqXmlFile->GetFullPath() + _(" ... Saving xsq."));
    logger_base.info("Saving XSQ file.");
    CurrentSeqXmlFile->AddJukebox(jukeboxPanel->Save());
    CurrentSeqXmlFile->Save(_sequenceElements);
    logger_base.info("XSQ file done.");

    if (mBackupOnSave) {
        DoBackup(false);
    }

    if (mRenderOnSave) {

        // make sure any pending layout work is done before rendering
        while (!DoAllWork()) {}

        SetStatusText(_("Saving ") + xlightsFilename + _(" ... Rendering."));

        // If number of channels is wrong then lets just dump and reallocate before render
        if ((_seqData.NumChannels() != roundTo4(GetMaxNumChannels())) ||
            (_seqData.FrameTime() != CurrentSeqXmlFile->GetFrameMS()) )
        {
            logger_base.info("Render on Save: Number of channels was wrong ... reallocating sequence data memory before rendering and saving.");

            //need to abort any render going on in order to change the SeqData size
            AbortRender();

            wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
            int ms = wxAtoi(mss);

            _seqData.init(GetMaxNumChannels(), CurrentSeqXmlFile->GetSequenceDurationMS() / ms, ms);
        }

        ProgressBar->Show();
        GaugeSizer->Layout();
        logger_base.info("Rendering on save.");
        RenderIseqData(true, nullptr); // render ISEQ layers below the Nutcracker layer
        logger_base.info("   iseq below effects done.");
        ProgressBar->SetValue(10);
        RenderGridToSeqData([this, sw] (bool aborted) {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.info("   Effects done.");
            ProgressBar->SetValue(90);
            RenderIseqData(false, nullptr);  // render ISEQ layers above the Nutcracker layer
            logger_base.info("   iseq above effects done. Render complete.");
            ProgressBar->SetValue(100);
            _appProgress->SetValue(0);
            _appProgress->Reset();
            ProgressBar->Hide();
            GaugeSizer->Layout();

            logger_base.info("Saving fseq file.");

            SetStatusText(_("Saving ") + xlightsFilename + _(" ... Writing fseq."));
            WriteFalconPiFile(xlightsFilename);
            logger_base.info("fseq file done.", true);
            DisplayXlightsFilename(xlightsFilename);
            float elapsedTime = sw.Time()/1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
            wxString displayBuff = wxString::Format(_("%s     Updated in %7.3f seconds"),xlightsFilename,elapsedTime);
            logger_base.info("%s", (const char *) displayBuff.c_str());
            CallAfter(&xLightsFrame::SetStatusText, displayBuff, 0);
            EnableSequenceControls(true);
            mSavedChangeCount = _sequenceElements.GetChangeCount();
            mLastAutosaveCount = mSavedChangeCount;
        } );
        return;
    }
    wxString display_name;
    if (mSaveFseqOnSave) {
        SetStatusText(_("Saving ") + xlightsFilename + _(" ... Writing fseq."));
        WriteFalconPiFile(xlightsFilename, true);
        logger_base.info("fseq file done.");
        DisplayXlightsFilename(xlightsFilename);
        display_name = xlightsFilename;
    } else {
        display_name = CurrentSeqXmlFile->GetFullPath();
    }
    float elapsedTime = sw.Time() / 1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
    wxString displayBuff = wxString::Format(_("%s     Updated in %7.3f seconds"), display_name, elapsedTime);
    logger_base.info("%s", (const char *)displayBuff.c_str());
    CallAfter(&xLightsFrame::SetStatusText, displayBuff, 0);
    EnableSequenceControls(true);
    mSavedChangeCount = _sequenceElements.GetChangeCount();
    mLastAutosaveCount = mSavedChangeCount;
}

void xLightsFrame::SetSequenceTiming(int timingMS)
{
    if (CurrentSeqXmlFile == nullptr)
        return;

    if (_seqData.FrameTime() != timingMS) {
        AbortRender();
        _seqData.init(GetMaxNumChannels(), CurrentSeqXmlFile->GetSequenceDurationMS() / timingMS, timingMS);
    }
}

void xLightsFrame::SaveAsSequence()
{
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
    } while (!ok);

    SaveAsSequence(newFilename);
}

void xLightsFrame::SaveAsSequence(const std::string& filename)
{
    wxFileName oName(filename);
    oName.SetExt("fseq");
    DisplayXlightsFilename(oName.GetFullPath());

    SetPanelSequencerLabel(oName.GetName().ToStdString());

    oName.SetExt("xsq");
    CurrentSeqXmlFile->SetPath(oName.GetPath());
    CurrentSeqXmlFile->SetFullName(oName.GetFullName());
    _renderCache.SetSequence(renderCacheDirectory, oName.GetName());
    SaveSequence();
    SetTitle(xlights_base_name + xlights_qualifier + " - " + filename);
    AddToMRU(filename);
    UpdateRecentFilesList(false);
}

void xLightsFrame::RenderAll()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_seqData.IsValidData()) {
        logger_base.warn("Aborting render all because sequence data has not been initialised.");
        return;
    }

    mRendering = true;
    EnableSequenceControls(false);
    wxYield();      // ensure all controls are disabled.
    wxStopWatch sw; // start a stopwatch timer

    ProgressBar->Show();
    GaugeSizer->Layout();
    SetStatusText(_("Rendering all layers"));
    SuspendAutoSave(true); // no need to auto save during render all
    logger_base.debug("Rendering all.");
    logger_base.debug("Model blending: %s", CurrentSeqXmlFile->supportsModelBlending() ? "On" : "Off");
    RenderIseqData(true, nullptr); // render ISEQ layers below the Nutcracker layer
    logger_base.info("   iseq below effects done.");
    ProgressBar->SetValue(10);
    RenderGridToSeqData([this, sw] (bool aborted) {
        static log4cpp::Category& logger_base2 = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base2.info("   Effects done.");
        ProgressBar->SetValue(90);
        RenderIseqData(false, nullptr); // render ISEQ layers above the Nutcracker layer
        logger_base2.info("   iseq above effects done. Render all complete.");
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
    for (int x = 0; x < parent->GetToolCount(); x++) {
        wxAuiToolBarItem* item = parent->FindToolByIndex(x);
        parent->EnableTool(item->GetId(), enable);
    }
    parent->Refresh();
}

static void enableAllMenubarControls(wxMenuBar* parent, bool enable)
{
    for (int x = 0; x < parent->GetMenuCount(); x++) {
        wxMenu* menu = parent->GetMenu(x);
        for (int y = 0; y < menu->GetMenuItemCount(); y++) {
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
    enableAllChildControls(timingPanel, enableSeqNotAC);
    if (enableSeqNotAC) timingPanel->ValidateWindow();
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
        MenuItem_ColorReplace->Enable(false);
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
