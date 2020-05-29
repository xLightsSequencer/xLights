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
#include "osxMacUtils.h"
#include "UtilFunctions.h"
#include "BufferPanel.h"
#include "EffectIconPanel.h"
#include "JukeboxPanel.h"
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

// load the specified .?seq binary file
void xLightsFrame::SeqLoadXlightsXSEQ(const wxString& filename)
{
    // read xlights file
    wxFileName fn(filename);
    if (fn.GetExt() == "xseq") {
        ReadXlightsFile(filename);
        fn.SetExt("fseq");
    }
    else {
        ReadFalconFile(filename, nullptr);
    }
    DisplayXlightsFilename(fn.GetFullPath());
    SeqBaseChannel = 1;
    SeqChanCtrlBasic = false;
    SeqChanCtrlColor = false;
}

void xLightsFrame::ResetEffectsXml()
{
	_sequenceViewManager.Reset();
    ModelsNode=nullptr;
    ViewObjectsNode=nullptr;
    EffectsNode=nullptr;
    PalettesNode=nullptr;
    ModelGroupsNode=nullptr;
    LayoutGroupsNode=nullptr;
    SettingsNode=nullptr;
    PerspectivesNode = nullptr;
}

wxString xLightsFrame::LoadEffectsFileNoCheck()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    ResetEffectsXml();
    wxFileName effectsFile;
    effectsFile.AssignDir(CurrentDir);
    effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
    wxString myString = "Hello";
    UnsavedRgbEffectsChanges = false;

    if (!effectsFile.FileExists())
    {
        // file does not exist, so create an empty xml doc
        CreateDefaultEffectsXml();
    }
    else
    {
        // check if there is a autosave backup file which is newer than the file we have been asked to open
        wxFileName fn(effectsFile.GetFullPath());
        wxFileName xx = fn;
        xx.SetExt("xbkp");
        wxString asfile = xx.GetLongPath();

        if (!_renderMode && wxFile::Exists(asfile))
        {
            // the autosave file exists
            wxDateTime xmltime = fn.GetModificationTime();
            wxFileName asfn(asfile);
            wxDateTime xbkptime = asfn.GetModificationTime();

            if (xbkptime > xmltime)
            {
                // autosave file is newer
                if (wxMessageBox("Autosaved rgbeffects file found which seems to be newer than your current rgbeffects file ... would you like to open that instead?", "Newer file found", wxYES_NO) == wxYES)
                {
                    // run a backup ... equivalent of a F10
                    DoBackup(false, false, true);

                    // delete the old xml file
                    wxRemoveFile(effectsFile.GetFullPath());

                    // rename the autosave file
                    wxRenameFile(asfile, effectsFile.GetFullPath());
                }
                else
                {
                    if (wxFile::Exists(fn.GetFullPath()))
                    {
                        //set the backup to be older than the XML files to avoid re-promting
                        xmltime -= wxTimeSpan(0, 0, 3, 0);  //subtract 2 seconds as FAT time resulution is 2 seconds
                        asfn.SetTimes(&xmltime, &xmltime, &xmltime);
                    }
                }
            }
        }

        if (!EffectsXml.Load(effectsFile.GetFullPath()))
        {
            DisplayError("Unable to load RGB effects file ... creating a default one.", this);
            CreateDefaultEffectsXml();
        }
    }

    wxXmlNode* root = EffectsXml.GetRoot();
    if (root->GetName() != "xrgb")
    {
        DisplayError("Invalid RGB effects file ... creating a default one.", this);
        CreateDefaultEffectsXml();
    }
    ModelsNode = EffectsNode = PalettesNode = ModelGroupsNode = LayoutGroupsNode = SettingsNode = PerspectivesNode = nullptr;
	wxXmlNode* viewsNode = nullptr;
	wxXmlNode* colorsNode = nullptr;
	wxXmlNode* viewpointsNode = nullptr;
    for(wxXmlNode* e=root->GetChildren(); e!=nullptr; e=e->GetNext() )
    {
        if (e->GetName() == "models") ModelsNode=e;
        if (e->GetName() == "view_objects") ViewObjectsNode=e;
        if (e->GetName() == "effects") EffectsNode=e;
        if (e->GetName() == "palettes") PalettesNode=e;
		if (e->GetName() == "views") viewsNode = e;
		if (e->GetName() == "colors") colorsNode = e;
		if (e->GetName() == "Viewpoints") viewpointsNode = e;
        if (e->GetName() == "modelGroups") ModelGroupsNode=e;
        if (e->GetName() == "layoutGroups") LayoutGroupsNode=e;
        if (e->GetName() == "settings") SettingsNode=e;
        if (e->GetName() == "perspectives") PerspectivesNode=e;
    }
    if (ModelsNode == nullptr)
    {
        ModelsNode = new wxXmlNode(wxXML_ELEMENT_NODE, "models");
        root->AddChild(ModelsNode);
        UnsavedRgbEffectsChanges = true;
    }
    if (ViewObjectsNode == nullptr)
    {
        ViewObjectsNode = new wxXmlNode( wxXML_ELEMENT_NODE, "view_objects" );
        root->AddChild( ViewObjectsNode );
        wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE, "view_object");
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
    if (EffectsNode == nullptr)
    {
        EffectsNode = new wxXmlNode(wxXML_ELEMENT_NODE, "effects");
        EffectsNode->AddAttribute("version", XLIGHTS_RGBEFFECTS_VERSION);
        root->AddChild(EffectsNode);
        UnsavedRgbEffectsChanges = true;
    }
    if (PalettesNode == nullptr)
    {
        PalettesNode = new wxXmlNode(wxXML_ELEMENT_NODE, "palettes");
        root->AddChild(PalettesNode);
        UnsavedRgbEffectsChanges = true;
    }

    if (viewsNode == nullptr)
    {
        UnsavedRgbEffectsChanges = true;
    }
    else
    {
        _sequenceViewManager.Load(viewsNode, mSequenceElements.GetCurrentView());
    }

    if (colorsNode != nullptr)
    {
        color_mgr.Load(colorsNode);
    }

    if (viewpointsNode != nullptr)
    {
		viewpoint_mgr.Load(viewpointsNode);
	}

    if (ModelGroupsNode == nullptr)
    {
        ModelGroupsNode = new wxXmlNode(wxXML_ELEMENT_NODE, "modelGroups");
        root->AddChild(ModelGroupsNode);
        UnsavedRgbEffectsChanges = true;
    }

    if (LayoutGroupsNode == nullptr)
    {
        LayoutGroupsNode = new wxXmlNode(wxXML_ELEMENT_NODE, "layoutGroups");
        root->AddChild(LayoutGroupsNode);
        UnsavedRgbEffectsChanges = true;
    }

    if (PerspectivesNode == nullptr)
    {
        PerspectivesNode = new wxXmlNode(wxXML_ELEMENT_NODE, "perspectives");
        root->AddChild(PerspectivesNode);
        UnsavedRgbEffectsChanges = true;
    }

    if (SettingsNode == nullptr)
    {
        SettingsNode = new wxXmlNode(wxXML_ELEMENT_NODE, "settings");
        root->AddChild(SettingsNode);
        SetXmlSetting("previewWidth", "1280");
        SetXmlSetting("previewHeight", "720");
        UnsavedRgbEffectsChanges = true;
    }
    int previewWidth = wxAtoi(GetXmlSetting("previewWidth", "1280"));
    int previewHeight = wxAtoi(GetXmlSetting("previewHeight", "720"));
    if (previewWidth == 0 || previewHeight == 0)
    {
        previewWidth = 1280;
        previewHeight = 720;
    }
    SetPreviewSize(previewWidth, previewHeight);

    mBackgroundImage = FixFile(GetShowDirectory(), GetXmlSetting("backgroundImage", ""));
    ObtainAccessToURL(mBackgroundImage.ToStdString());
    if (mBackgroundImage != "" && (!wxFileExists(mBackgroundImage) || !wxIsReadable(mBackgroundImage))) {
        wxString fn = FixFile(mediaDirectory, GetXmlSetting("backgroundImage", ""));
        ObtainAccessToURL(fn.ToStdString());
        if (wxFileExists(fn) && wxIsReadable(mBackgroundImage)) {
            mBackgroundImage = fn;
        }
        else {
            //image doesn't exist there, lets look for it in show directory and media directory
            wxFileName name(mBackgroundImage);
            name.SetPath(CurrentDir);
            if (!name.Exists()) {
                name.SetPath(mediaDirectory);
            }
            if (name.Exists()) {
                mBackgroundImage = name.GetFullPath();
            }
        }
    }
    SetPreviewBackgroundImage(mBackgroundImage);
    SetDisplay2DBoundingBox(GetXmlSetting("Display2DBoundingBox", "0") == "1");
    layoutPanel->SetDisplay2DBoundingBox(GetDisplay2DBoundingBox());
    SetDisplay2DCenter0(GetXmlSetting("Display2DCenter0", "0") == "1");
    layoutPanel->SetDisplay2DCenter0(GetDisplay2DCenter0());

    //Load FSEQ and Backup directory settings
    fseqDirectory = GetXmlSetting("fseqDir", showDirectory);
    renderCacheDirectory = GetXmlSetting("renderCacheDir", fseqDirectory); // we user fseq directory if no setting is present
    backupDirectory = GetXmlSetting("backupDir", showDirectory);
    ObtainAccessToURL(fseqDirectory.ToStdString());
    ObtainAccessToURL(backupDirectory.ToStdString());
    if (!wxDir::Exists(fseqDirectory))
    {
        logger_base.warn("FSEQ Directory not Found ... switching to Show Directory.");
        fseqDirectory = showDirectory;
        SetXmlSetting("fseqDir", showDirectory);
        UnsavedRgbEffectsChanges = true;
    }
    FseqDir = fseqDirectory;
    if (!wxDir::Exists(renderCacheDirectory))
    {
        logger_base.warn("Render Cache Directory not Found ... switching to Show Directory.");
        renderCacheDirectory = showDirectory;
        SetXmlSetting("renderCacheDir", showDirectory);
        UnsavedRgbEffectsChanges = true;
    }
    if (!wxDir::Exists(backupDirectory))
    {
        logger_base.warn("Backup Directory not Found ... switching to Show Directory.");
        backupDirectory = showDirectory;
        SetXmlSetting("backupDir", showDirectory);
        UnsavedRgbEffectsChanges = true;
    }

    mStoredLayoutGroup = GetXmlSetting("storedLayoutGroup", "Default");

    // validate stored preview exists
    bool found_saved_preview = false;
    for (wxXmlNode* e = LayoutGroupsNode->GetChildren(); e != nullptr; e = e->GetNext())
    {
        if (e->GetName() == "layoutGroup")
        {
            wxString grp_name = e->GetAttribute("name");
            if (!grp_name.IsEmpty())
            {
                if (grp_name.ToStdString() == mStoredLayoutGroup)
                {
                    found_saved_preview = true;
                }
            }
        }
    }
    if (!found_saved_preview)
    {
        mStoredLayoutGroup = "Default";
    }

    // Do this here as it may switch the background image
    LayoutGroups.clear();
    layoutPanel->Reset();
    AllModels.SetLayoutsNode(LayoutGroupsNode);  // provides easy access to layout names for the model class
    for (wxXmlNode* e = LayoutGroupsNode->GetChildren(); e != nullptr; e = e->GetNext())
    {
        if (e->GetName() == "layoutGroup")
        {
            wxString grp_name = e->GetAttribute("name");
            if (!grp_name.IsEmpty())
            {
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

    mBackgroundBrightness = wxAtoi(GetXmlSetting("backgroundBrightness","100"));
    mBackgroundAlpha = wxAtoi(GetXmlSetting("backgroundAlpha","100"));
    SetPreviewBackgroundBrightness(mBackgroundBrightness, mBackgroundAlpha);
    mScaleBackgroundImage = wxAtoi(GetXmlSetting("scaleImage","0")) > 0;
    SetPreviewBackgroundScaled(mScaleBackgroundImage);

    std::string group = layoutPanel->GetCurrentLayoutGroup();
    if( group != "Default" && group != "All Models" && group != "Unassigned" ) {
        modelPreview->SetBackgroundBrightness(layoutPanel->GetBackgroundBrightnessForSelectedPreview(), layoutPanel->GetBackgroundAlphaForSelectedPreview());
        modelPreview->SetScaleBackgroundImage(layoutPanel->GetBackgroundScaledForSelectedPreview());
    }

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

    // update version
    EffectsNode->DeleteAttribute("version");
    EffectsNode->AddAttribute("version", XLIGHTS_RGBEFFECTS_VERSION);

    // Handle upgrade of networks file to the controller/output structure
    if (_outputManager.ConvertModelStartChannels(ModelsNode))
    {
        UnsavedRgbEffectsChanges = true;
        wxMessageBox("Your setup tab data has been converted to the new controller centric format.\nIf you choose to save either the Controller (Setup) or Layout Tab data it is critical you save both or some of your model start channels will break.\nIf this happens you can either repair them manually or roll back to a backup copy.");
    }

    displayElementsPanel->SetSequenceElementsModelsViews(&SeqData, &mSequenceElements, ModelsNode, ModelGroupsNode, &_sequenceViewManager);
    layoutPanel->ClearUndo();
    GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "LoadEffectsFile");
    mSequencerInitialize = false;

    // load the perspectives
    CheckForAndCreateDefaultPerpective();
    perspectivePanel->SetPerspectives(PerspectivesNode);
    LoadPerspectivesMenu(PerspectivesNode);

    float elapsedTime = sw.Time() / 1000.0; //msec => sec
    SetStatusText(wxString::Format(_("'%s' loaded in %4.3f sec."), filename, elapsedTime));
}

void xLightsFrame::LoadPerspectivesMenu(wxXmlNode* perspectivesNode)
{
    // Clear old menu items

    int menuCount = MenuItemPerspectives->GetMenuItemCount();
    int first = menuCount - 1;
    wxMenuItem* current_menuitem = MenuItemPerspectives->FindItemByPosition(first);
    while (current_menuitem != nullptr && !current_menuitem->IsSeparator())
    {
        first--;
        current_menuitem = MenuItemPerspectives->FindItemByPosition(first);
    }
    first++;
    current_menuitem =  first < menuCount ? MenuItemPerspectives->FindItemByPosition(first) : nullptr;
    while (current_menuitem != nullptr) {
        MenuItemPerspectives->Delete(current_menuitem);
        menuCount--;
        current_menuitem = first < menuCount ? MenuItemPerspectives->FindItemByPosition(first) : nullptr;
    }


    int pCount = 0;

    for(wxXmlNode* p=perspectivesNode->GetChildren(); p != nullptr; p=p->GetNext() )
    {
        if (p->GetName() == "perspective")
        {
            wxString name=p->GetAttribute("name");
            if (!name.IsEmpty())
            {
                int id = wxNewId();
                MenuItemPerspectives->AppendRadioItem(id,name);
                if (mCurrentPerpective != nullptr && (name == mCurrentPerpective->GetAttribute("name")))
                  MenuItemPerspectives->Check(id,true);
                PerspectiveId pmenu;
                pmenu.id=id;
                pmenu.p=p;
                perspectives[pCount] = pmenu;
                Connect(id, wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemLoadPerspectiveSelected);
                pCount++;
                if (pCount>=10) { return; }
            }
        }
    }


}

void xLightsFrame::OnMenuItemLoadPerspectiveSelected(wxCommandEvent& event)
{
    Notebook1->SetSelection(Notebook1->GetPageIndex(PanelSequencer));
    for (int i=0;i<10;i++) {
        if (perspectives[i].id == event.GetId()) {
            DoLoadPerspective(perspectives[i].p);
            return;
        }
    }
}

void xLightsFrame::SaveModelsFile()
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string filename = CurrentDir.ToStdString() + "/xScheduleData/GetModels.dat";

    if (!wxDir::Exists(CurrentDir + "/xScheduleData"))
    {
        logger_base.debug("Creating xScheduleData folder.");
        wxDir sd(CurrentDir);
        sd.Make(CurrentDir + "/xScheduleData");
    }

    logger_base.debug("Creating models JSON file: %s.", (const char *)filename.c_str());

    wxFile modelsJSON;
    if (!modelsJSON.Create(filename, true) || !modelsJSON.IsOpened())
    {
        logger_base.error("Unable to create file: %s.", (const char *)filename.c_str());
        return;
    }

    modelsJSON.Write("{\"models\":[");

    bool first = true;
    for (auto m = AllModels.begin(); m != AllModels.end(); ++m)
    {
        Model* model = m->second;
        if (model->GetDisplayAs() == "ModelGroup")
        {
            // Dont export model groups ... they arent useful

            //if (!first)
            //{
            //    modelsJSON.Write(",");
            //}
            //first = false;

            //ModelGroup* mg = static_cast<ModelGroup*>(model);
            //modelsJSON.Write("{\"name\":\"" + mg->name +
            //    "\",\"type\":\"" + mg->GetDisplayAs() +
            //    "\",\"startchannel\":\"" + wxString::Format("%i", mg->NodeStartChannel(0) + 1) +
            //    "\",\"channels\":\"" + wxString::Format("%i", mg->GetChanCount()) +
            //    "\",\"stringtype\":\"\"}");
        }
        else if (model->GetDisplayAs() == "SubModel")
        {
            // Dont export sub models ... they arent useful

            //if (!first)
            //{
            //    modelsJSON.Write(",");
            //}
            //first = false;

            //SubModel* sm = static_cast<SubModel*>(model);
            //int ch = sm->GetNumberFromChannelString(sm->ModelStartChannel);
            //modelsJSON.Write("{\"name\":\"" + sm->name +
            //    "\",\"type\":\"" + sm->GetDisplayAs() +
            //    "\",\"startchannel\":\"" + wxString::Format("%i", ch) +
            //    "\",\"channels\":\"" + wxString::Format("%i", sm->GetChanCount()) +
            //    "\",\"stringtype\":\"" + sm->GetStringType() + "\"}");
        }
        else
        {
            if (!first)
            {
                modelsJSON.Write(",");
            }
            first = false;

            long ch = model->GetNumberFromChannelString(model->ModelStartChannel);
            modelsJSON.Write("{\"name\":\""+model->name+
                              "\",\"type\":\""+model->GetDisplayAs()+
                              "\",\"startchannel\":\""+wxString::Format("%ld", (long)ch)+
                              "\",\"channels\":\""+ wxString::Format("%ld", (long)model->GetChanCount()) +
                              "\",\"stringtype\":\""+ model->GetStringType() +"\"}");
        }
    }

    modelsJSON.Write("]}");

    logger_base.debug("     models JSON file done.");
}

// returns true on success
bool xLightsFrame::SaveEffectsFile(bool backup)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // dont save if currently saving
    std::unique_lock<std::mutex> lock(saveLock, std::try_to_lock);
    if (!lock.owns_lock()) return false;

	// Make sure the views are up to date before we save it
	_sequenceViewManager.Save(&EffectsXml);

	color_mgr.Save(&EffectsXml);

	viewpoint_mgr.Save(&EffectsXml);

    wxFileName effectsFile;
    effectsFile.AssignDir( CurrentDir );
    if (backup)
    {
        effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE_BACKUP));
    }
    else
    {
        effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
    }

    if (!EffectsXml.Save( effectsFile.GetFullPath() ))
    {
        if (backup)
        {
            logger_base.warn("Unable to save backup of RGB effects file");
        }
        else
        {
            DisplayError("Unable to save RGB effects file", this);
        }
        return false;
    }

    if (!backup)
    {
        SaveModelsFile();
        UnsavedRgbEffectsChanges = false;
    }

    return true;
}

void xLightsFrame::CreateDefaultEffectsXml()
{
    wxXmlNode* root = new wxXmlNode( wxXML_ELEMENT_NODE, "xrgb" );
    EffectsXml.SetRoot( root );
    UnsavedRgbEffectsChanges = true;
}

// This ensures submodels are in the right order in the sequence elements after the user
// reorders them in the ssubmodel dialog
bool xLightsFrame::EnsureSequenceElementsAreOrderedCorrectly(const std::string ModelName, std::vector<std::string>& submodelOrder)
{
    ModelElement* elementToCheck = dynamic_cast<ModelElement*>(mSequenceElements.GetElement(ModelName));

    if (elementToCheck != nullptr)
    {
        // Check if they are already right and in the right order
        bool identical = true;
        if (elementToCheck->GetSubModelCount() == submodelOrder.size())
        {
            for (int  i = 0; i < elementToCheck->GetSubModelCount(); i++)
            {
                if (elementToCheck->GetSubModel(i)->GetName() != submodelOrder[i])
                {
                    identical = false;
                    break;
                }
            }
        }
        else
        {
            identical = false;
        }

        if (identical)
        {
            // no changes we can exit
            return false;
        }

        // Grab the existing elements
        std::list<SubModelElement*> oldList;
        for (int i = 0; i < elementToCheck->GetSubModelCount(); i++)
        {
            oldList.push_back(elementToCheck->GetSubModel(i));
        }

        // remove but dont delete all submodels
        elementToCheck->RemoveAllSubModels();

        // Now add them back in the right order
        for (auto msm = submodelOrder.begin(); msm != submodelOrder.end(); ++msm)
        {
            bool found = false;
            for (auto it = oldList.begin(); it != oldList.end(); ++it)
            {
                if ((*it)->GetName() == *msm)
                {
                    elementToCheck->AddSubModel(*it);
                    oldList.erase(it);
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                // add the submodel as it didnt previously exist
                elementToCheck->GetSubModel(*msm, true);
            }
        }

        // delete any that are no longer there
        for (auto it = oldList.begin(); it != oldList.end(); ++it)
        {
            delete *it;
        }

        return true;
    }

    return false;
}

bool xLightsFrame::RenameModel(const std::string OldName, const std::string& NewName)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool internalsChanged = false;

    if (OldName == NewName) {
        return false;
    }

    logger_base.debug("Renaming model '%s' to '%s'.", (const char*)OldName.c_str(), (const char *)NewName.c_str());

    Element* elem_to_rename = mSequenceElements.GetElement(OldName);
    if (elem_to_rename != nullptr)
    {
        elem_to_rename->SetName(NewName);
    }

    if (std::find(OldName.begin(), OldName.end(), '/') != OldName.end())
    {
        internalsChanged = AllModels.RenameSubModel(OldName, NewName);
    }
    else
    {
        internalsChanged = AllModels.Rename(OldName, NewName);
    }

    RenameModelInViews(OldName, NewName);
    mSequenceElements.RenameModelInViews(OldName, NewName);

    UnsavedRgbEffectsChanges = true;
    return internalsChanged;
}

void xLightsFrame::RenameModelInViews(const std::string old_name, const std::string& new_name)
{
	_sequenceViewManager.RenameModel(old_name, new_name);
}

void xLightsFrame::SetChoicebook(wxChoicebook* cb, const wxString& PageName)
{
    if (cb->GetChoiceCtrl()->GetStringSelection() == PageName) return; // no need to change

    RenderableEffect* reff = effectManager.GetEffect(PageName.ToStdString());
    if (reff != nullptr) {
        for (size_t i = 0; i < cb->GetPageCount(); i++)
        {
            if (cb->GetPageText(i) == reff->ToolTip())
            {
                cb->ChangeSelection(i);
                return;
            }
        }
    }
}

bool xLightsFrame::RenameObject(const std::string OldName, const std::string& NewName)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool internalsChanged = false;

    if (OldName == NewName) {
        return false;
    }

    logger_base.debug("Renaming object '%s' to '%s'.", (const char*)OldName.c_str(), (const char *)NewName.c_str());

    Element* elem_to_rename = mSequenceElements.GetElement(OldName);
    if (elem_to_rename != nullptr)
    {
        elem_to_rename->SetName(NewName);
    }

    internalsChanged = AllObjects.Rename(OldName, NewName);

    UnsavedRgbEffectsChanges = true;
    return internalsChanged;
}

void xLightsFrame::OnBitmapButtonSaveSeqClick(wxCommandEvent& event)
{
    SaveSequence();
}

static std::string chooseNewName(xLightsFrame *parent, std::vector<std::string> &names,
                                 const std::string &msg, const std::string curval) {
    wxTextEntryDialog dialog(parent, _("Enter new name"), msg, curval);
    int DlgResult;
    do {
        DlgResult=dialog.ShowModal();
        if (DlgResult == wxID_OK) {
            // validate inputs
            std::string NewName = dialog.GetValue().Trim(true).Trim(false);
            if (std::find(names.begin(), names.end(), NewName) == names.end()) {
                return NewName;
            }
        }
    }
    while (DlgResult == wxID_OK);
    return curval;
}

static void AddModelsToPreview(ModelGroup *grp, std::vector<Model *> &PreviewModels) {
    for (auto it2 = grp->Models().begin(); it2 != grp->Models().end(); ++it2) {
        Model *model = dynamic_cast<Model*>(*it2);
        ModelGroup *g2 = dynamic_cast<ModelGroup*>(*it2);
        SubModel *sm = dynamic_cast<SubModel*>(*it2);

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

    if (ModelsNode == nullptr) return; // this happens when xlights is first loaded
    if (ViewObjectsNode == nullptr) return; // this happens when xlights is first loaded

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
}

void xLightsFrame::OpenRenderAndSaveSequences(const wxArrayString &origFilenames, bool exitOnDone) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (origFilenames.IsEmpty()) {
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
    fileNames.RemoveAt(0);
    wxStopWatch sw; // start a stopwatch timer

    printf("Processing file %s\n", (const char *)seq.c_str());
    logger_base.debug("Batch Render Processing file %s\n", (const char *)seq.c_str());
    OpenSequence(seq, nullptr);
    EnableSequenceControls(false);

    // if the fseq directory is not the show directory then ensure the fseq folder is set right
    if (fseqDirectory != showDirectory)
    {
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
    RenderGridToSeqData([this, sw, fileNames, exitOnDone] {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("   Effects done.");
        ProgressBar->SetValue(90);
        RenderIseqData(false, nullptr);  // render ISEQ layers above the Nutcracker layer
        logger_base.info("   iseq above effects done. Render complete.");
        ProgressBar->SetValue(100);
        ProgressBar->Hide();
        GaugeSizer->Layout();

        logger_base.info("Saving fseq file.");
        SetStatusText(_("Saving ") + xlightsFilename + _(" ... Writing fseq."));
        WriteFalconPiFile(xlightsFilename);
        logger_base.info("fseq file done.");
        DisplayXlightsFilename(xlightsFilename);
        float elapsedTime = sw.Time()/1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
        wxString displayBuff = wxString::Format(_("%s     Updated in %7.3f seconds"),xlightsFilename,elapsedTime);
        logger_base.info("%s", (const char *) displayBuff.c_str());
        CallAfter(&xLightsFrame::SetStatusText, displayBuff, 0);
        mSavedChangeCount = mSequenceElements.GetChangeCount();
        mLastAutosaveCount = mSavedChangeCount;

        CallAfter(&xLightsFrame::OpenRenderAndSaveSequences, fileNames, exitOnDone);
    } );
}

void xLightsFrame::SaveSequence()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (SeqData.NumFrames() == 0)
    {
        DisplayError("You must open a sequence first!", this);
        return;
    }

    wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
    wxPostEvent(this, playEvent);

    std::unique_lock<std::mutex> lock(saveLock);

    if (xlightsFilename.IsEmpty())
    {
        wxString NewFilename;

        wxString startname = CurrentSeqXmlFile->GetName();
        if (startname.IsEmpty() && !CurrentSeqXmlFile->GetMediaFile().empty() )
        {
            startname = wxFileName(CurrentSeqXmlFile->GetMediaFile()).GetName();
        }

        wxFileDialog fd(this,
                        "Choose filename to Save Sequence:",
                        CurrentDir,
                        startname,
                        strSequenceSaveAsFileTypes,
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        bool ok;
        do
        {
            if (fd.ShowModal() != wxID_OK)
            {
                return;
            }
            // validate inputs
            NewFilename=fd.GetPath();
            NewFilename.Trim();
            ok=true;
            if (NewFilename.IsEmpty())
            {
                ok=false;
                DisplayError("File name cannot be empty", this);
            }
        }
        while (!ok);
        wxFileName xmlFileName(NewFilename);//set XML Path based on user input
        _renderCache.SetSequence(renderCacheDirectory.ToStdString(), xmlFileName.GetName());
        xmlFileName.SetExt("xsq");
        CurrentSeqXmlFile->SetPath(xmlFileName.GetPath());
        CurrentSeqXmlFile->SetFullName(xmlFileName.GetFullName());

        wxFileName fseqFileName(NewFilename);//create FSEQ file name in seq folder
        fseqFileName.SetExt("fseq");
        DisplayXlightsFilename(fseqFileName.GetFullPath());
    }

    // if the fseq directory is not the show directory then ensure the fseq folder is set right
	// Only Change FSEQ save folder if the FSEQ Folder Setting is NOT the Show Dir
    if (fseqDirectory != showDirectory)
    {
        wxFileName fn(xlightsFilename);
        fn.SetPath(fseqDirectory);
        xlightsFilename = fn.GetFullPath();
    }

    EnableSequenceControls(false);
    wxStopWatch sw; // start a stopwatch timer
    CurrentSeqXmlFile->SetExt("xsq");
    SetStatusText(_("Saving ") + CurrentSeqXmlFile->GetFullPath() + _(" ... Saving xsq."));
    logger_base.info("Saving XSQ file.");
    CurrentSeqXmlFile->AddJukebox(jukeboxPanel->Save());
    CurrentSeqXmlFile->Save(mSequenceElements);
    logger_base.info("XSQ file done.");

    if (mBackupOnSave)
    {
        DoBackup(false);
    }

    if (mRenderOnSave) {

        // make sure any pending layout work is done before rendering
        while (!DoAllWork()) {}

        SetStatusText(_("Saving ") + xlightsFilename + _(" ... Rendering."));

        // If number of channels is wrong then lets just dump and reallocate before render
        if ((SeqData.NumChannels() != roundTo4(GetMaxNumChannels())) ||
            (SeqData.FrameTime() != CurrentSeqXmlFile->GetFrameMS()) )
        {
            logger_base.info("Render on Save: Number of channels was wrong ... reallocating sequence data memory before rendering and saving.");

            wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
            int ms = wxAtoi(mss);

            SeqData.init(GetMaxNumChannels(), CurrentSeqXmlFile->GetSequenceDurationMS() / ms, ms);
        }

        ProgressBar->Show();
        GaugeSizer->Layout();
        logger_base.info("Rendering on save.");
        RenderIseqData(true, nullptr); // render ISEQ layers below the Nutcracker layer
        logger_base.info("   iseq below effects done.");
        ProgressBar->SetValue(10);
        RenderGridToSeqData([this, sw] {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.info("   Effects done.");
            ProgressBar->SetValue(90);
            RenderIseqData(false, nullptr);  // render ISEQ layers above the Nutcracker layer
            logger_base.info("   iseq above effects done. Render complete.");
            ProgressBar->SetValue(100);
            ProgressBar->Hide();
            GaugeSizer->Layout();

            logger_base.info("Saving fseq file.");

            SetStatusText(_("Saving ") + xlightsFilename + _(" ... Writing fseq."));
            WriteFalconPiFile(xlightsFilename);
            logger_base.info("fseq file done.");
            DisplayXlightsFilename(xlightsFilename);
            float elapsedTime = sw.Time()/1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
            wxString displayBuff = wxString::Format(_("%s     Updated in %7.3f seconds"),xlightsFilename,elapsedTime);
            logger_base.info("%s", (const char *) displayBuff.c_str());
            CallAfter(&xLightsFrame::SetStatusText, displayBuff, 0);
            EnableSequenceControls(true);
            mSavedChangeCount = mSequenceElements.GetChangeCount();
            mLastAutosaveCount = mSavedChangeCount;
        } );
        return;
    }
    wxString display_name;
    if (mSaveFseqOnSave)
    {
        SetStatusText(_("Saving ") + xlightsFilename + _(" ... Writing fseq."));
        WriteFalconPiFile(xlightsFilename);
        logger_base.info("fseq file done.");
        DisplayXlightsFilename(xlightsFilename);
        display_name = xlightsFilename;
    }
    else
    {
        display_name = CurrentSeqXmlFile->GetFullPath();
    }
    float elapsedTime = sw.Time() / 1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
    wxString displayBuff = wxString::Format(_("%s     Updated in %7.3f seconds"), display_name, elapsedTime);
    logger_base.info("%s", (const char *)displayBuff.c_str());
    CallAfter(&xLightsFrame::SetStatusText, displayBuff, 0);
    EnableSequenceControls(true);
    mSavedChangeCount = mSequenceElements.GetChangeCount();
    mLastAutosaveCount = mSavedChangeCount;
}

void xLightsFrame::SetSequenceTiming(int timingMS)
{
    if (CurrentSeqXmlFile == nullptr) return;

    if (SeqData.FrameTime() != timingMS)
    {
        SeqData.init(GetMaxNumChannels(), CurrentSeqXmlFile->GetSequenceDurationMS() / timingMS, timingMS);
    }
}

void xLightsFrame::SaveAsSequence()
{
   if (SeqData.NumFrames() == 0)
    {
        DisplayError("You must open a sequence first!", this);
        return;
    }
    wxString NewFilename;
    wxFileDialog fd(this,
                    "Choose filename to Save Sequence:",
                    CurrentDir,
                    CurrentSeqXmlFile->GetName(),
                    strSequenceSaveAsFileTypes,
                    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    bool ok;
    do
    {
        if (fd.ShowModal() != wxID_OK)
        {
            return;
        }
        // validate inputs
        NewFilename=fd.GetPath();
        NewFilename.Trim();
        ok=true;
        if (NewFilename.IsEmpty())
        {
            ok=false;
            DisplayError("File name cannot be empty", this);
        }
    }
    while (!ok);
    wxFileName oName(NewFilename);
    oName.SetExt("fseq");
    DisplayXlightsFilename(oName.GetFullPath());

    SetPanelSequencerLabel(oName.GetName().ToStdString());

    oName.SetExt("xsq");
    CurrentSeqXmlFile->SetPath(oName.GetPath());
    CurrentSeqXmlFile->SetFullName(oName.GetFullName());
    _renderCache.SetSequence(renderCacheDirectory.ToStdString(), oName.GetName());
    SaveSequence();
    SetTitle(xlights_base_name + " - " + NewFilename);
}

void xLightsFrame::RenderAll()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!SeqData.IsValidData())
    {
        logger_base.warn("Aborting render all because sequence data has not been initialised.");
        return;
    }

    mRendering = true;
    EnableSequenceControls(false);
	wxYield(); // ensure all controls are disabled.
    wxStopWatch sw; // start a stopwatch timer

    ProgressBar->Show();
    GaugeSizer->Layout();
    SetStatusText(_("Rendering all layers"));
    logger_base.debug("Rendering all.");
    logger_base.debug("Model blending: %s", CurrentSeqXmlFile->supportsModelBlending() ? "On" : "Off");
    RenderIseqData(true, nullptr); // render ISEQ layers below the Nutcracker layer
    logger_base.info("   iseq below effects done.");
    ProgressBar->SetValue(10);
    RenderGridToSeqData([this, sw] {
        static log4cpp::Category &logger_base2 = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base2.info("   Effects done.");
        ProgressBar->SetValue(90);
        RenderIseqData(false, nullptr);  // render ISEQ layers above the Nutcracker layer
        logger_base2.info("   iseq above effects done. Render all complete.");
        ProgressBar->SetValue(100);
        float elapsedTime = sw.Time()/1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
        wxString displayBuff = wxString::Format(_("Rendered in %7.3f seconds"),elapsedTime);
        CallAfter(&xLightsFrame::SetStatusText, displayBuff, 0);
        mRendering = false;
        EnableSequenceControls(true);
        ProgressBar->Hide();
        GaugeSizer->Layout();
    });
}

static void enableAllChildControls(wxWindow *parent, bool enable) {
    for (const auto& it : parent->GetChildren()) {
        it->Enable(enable);
        enableAllChildControls(it, enable);
        if (enable && it->GetName().StartsWith("ID_VALUECURVE"))             {
            wxCommandEvent e(EVT_VC_CHANGED);
            e.SetInt(-1);
            e.SetEventObject(it);
            wxPostEvent(parent, e);
        }
    }
}

static void enableAllToolbarControls(wxAuiToolBar *parent, bool enable) {
    enableAllChildControls((wxWindow *)parent, enable);
    for (int x = 0; x < parent->GetToolCount(); x++) {
        wxAuiToolBarItem * item = parent->FindToolByIndex(x);
        parent->EnableTool(item->GetId(), enable);
    }
    parent->Refresh();
}

static void enableAllMenubarControls(wxMenuBar *parent, bool enable) {
    for (int x = 0; x < parent->GetMenuCount(); x++) {
        wxMenu * menu = parent->GetMenu(x);
        for (int y = 0; y < menu->GetMenuItemCount(); y++) {
            wxMenuItem *item = menu->FindItemByPosition(y);
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
    bool enableSeq = enable && SeqData.NumFrames() > 0;
    bool enableSeqNotAC = enable && SeqData.NumFrames() > 0 && !IsACActive();
    enableAllToolbarControls(WindowMgmtToolbar, enableSeq);
    enableAllToolbarControls(EffectsToolBar, enableSeqNotAC);
    enableAllToolbarControls(EditToolBar, enableSeq);
    enableAllToolbarControls(ACToolbar, enableSeq);
    mainSequencer->CheckBox_SuspendRender->Enable(enableSeq);
    enableAllToolbarControls(ViewToolBar, enable);
    enableAllToolbarControls(OutputToolBar, enable);


    enableAllChildControls(EffectsPanel1, enableSeqNotAC);
    //if (enableSeqNotAC) EffectsPanel1->ValidateWindow();
    enableAllChildControls(timingPanel, enableSeqNotAC);
    if (enableSeqNotAC) timingPanel->ValidateWindow();
    enableAllChildControls(bufferPanel, enableSeqNotAC);
    if (enableSeqNotAC) bufferPanel->ValidateWindow();
    enableAllChildControls(perspectivePanel, enableSeq);
    //if (enableSeq) perspectivePanel->ValidateWindow();
    enableAllChildControls(colorPanel, enableSeqNotAC);
    if (enableSeqNotAC) {colorPanel->ValidateWindow();}
    enableAllChildControls(effectPalettePanel, enableSeqNotAC);
    //if (enableSeqNotAC) effectPalettePanel->ValidateWindow();
    enableAllChildControls(_valueCurvesPanel, enableSeqNotAC);
    //if (enableSeqNotAC) _valueCurvesPanel->ValidateWindow();
    enableAllChildControls(_coloursPanel, enableSeqNotAC);
    //if (enableSeqNotAC) _coloursPanel->ValidateWindow();
    enableAllChildControls(jukeboxPanel, enableSeqNotAC);
    //if (enableSeqNotAC) jukeboxPanel->ValidateWindow();
    UpdateACToolbar(enable);

    enableAllMenubarControls(MenuBar, enable);

    if (enable && SeqData.NumFrames() == 0) {
        //no file is loaded, disable save/render buttons
        EnableToolbarButton(MainToolBar,ID_AUITOOLBAR_SAVE,false);
        EnableToolbarButton(MainToolBar,ID_AUITOOLBAR_SAVEAS,false);
        EnableToolbarButton(MainToolBar,ID_AUITOOLBAR_RENDERALL,false);
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
    }
    if (!enable && SeqData.NumFrames() > 0) {
        //file is loaded, but we're doing something that requires controls disabled (such as rendering)
        //we need to also disable the quit button
        QuitMenuItem->Enable(false);
    } else {
        QuitMenuItem->Enable(true);
    }

    if (MenuItem_CrashXLights != nullptr)
    {
        MenuItem_CrashXLights->Enable();
    }
    if (MenuItem_LogRenderState != nullptr)
    {
        MenuItem_LogRenderState->Enable();
    }
}

//modifed for partially random -DJ
//void djdebug(const char* fmt, ...); //_DJ
std::string xLightsFrame::CreateEffectStringRandom(std::string &settings, std::string &palette)
{
    int eff1 = ChooseRandomEffect();
    settings = EffectsPanel1->GetRandomEffectString(eff1);
    palette = colorPanel->GetRandomColorString();
    return effectManager[eff1]->Name();
}

int xLightsFrame::ChooseRandomEffect()
{
    if (_randomEffectsToUse.size() == 0)
    {
        return 0;
    }

    const int select = rand() % _randomEffectsToUse.size();
    const wxString effect = _randomEffectsToUse[select];
    const int index = effectManager.GetEffectIndex(effect);
    return index != -1 ? index : 0;
}

void xLightsFrame::VCChanged(wxCommandEvent& event)
{
    _valueCurvesPanel->Freeze();
    if (event.GetInt() == -1)
    {
        _valueCurvesPanel->UpdateValueCurveButtons(true);
    }
    enableAllChildControls(_valueCurvesPanel, true); // enable and disable otherwise if anything has been added while disabled wont be disabled.
    enableAllChildControls(_valueCurvesPanel, SeqData.NumFrames() > 0 && !IsACActive());
    _valueCurvesPanel->Thaw();
}

void xLightsFrame::ColourChanged(wxCommandEvent& event)
{
    _coloursPanel->Freeze();
    if (event.GetInt() == -1)
    {
        _coloursPanel->UpdateColourButtons(true, this);
    }
    enableAllChildControls(_coloursPanel, true); // enable and disable otherwise if anything has been added while disabled wont be disabled.
    enableAllChildControls(_coloursPanel, SeqData.NumFrames() > 0 && !IsACActive());
    _coloursPanel->Thaw();
}
