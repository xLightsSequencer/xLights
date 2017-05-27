#include <wx/utils.h> //check keyboard state -DJ
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include <wx/xml/xml.h>
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

void xLightsFrame::DisplayXlightsFilename(const wxString& filename)
{
    xlightsFilename=filename;
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


// load the specified .xseq binary file
void xLightsFrame::SeqLoadXlightsXSEQ(const wxString& filename)
{
    // read xlights file
    wxFileName fn(filename);
    if (fn.GetExt() == "xseq") {
        ReadXlightsFile(filename);
        fn.SetExt("fseq");
    } else {
        ReadFalconFile(filename, nullptr);
    }
    DisplayXlightsFilename(fn.GetFullPath());
    SeqBaseChannel=1;
    SeqChanCtrlBasic=false;
    SeqChanCtrlColor=false;
}


void xLightsFrame::ResetEffectsXml()
{
	_sequenceViewManager.Reset();
    ModelsNode=nullptr;
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

        if (wxFile::Exists(asfile))
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
                    // Touch the xml file to stop this prompt occuring again
                    fn.Touch();
                }
            }
        }

        if (!EffectsXml.Load(effectsFile.GetFullPath()))
        {
            logger_base.warn("Unable to load RGB effects file ... creating a default one.");
            wxMessageBox(_("Unable to load RGB effects file"), _("Error"));
            CreateDefaultEffectsXml();
        }
    }

    wxXmlNode* root=EffectsXml.GetRoot();
    if (root->GetName() != "xrgb")
    {
        logger_base.warn("Invalid RGB effects file... xrgb node not found ... creating a default one.");
        wxMessageBox(_("Invalid RGB effects file. Press Save File button to start a new file."), _("Error"));
        CreateDefaultEffectsXml();
    }
	wxXmlNode* viewsNode = nullptr;
	wxXmlNode* colorsNode = nullptr;
    for(wxXmlNode* e=root->GetChildren(); e!=nullptr; e=e->GetNext() )
    {
        if (e->GetName() == "models") ModelsNode=e;
        if (e->GetName() == "effects") EffectsNode=e;
        if (e->GetName() == "palettes") PalettesNode=e;
		if (e->GetName() == "views") viewsNode = e;
		if (e->GetName() == "colors") colorsNode = e;
        if (e->GetName() == "modelGroups") ModelGroupsNode=e;
        if (e->GetName() == "layoutGroups") LayoutGroupsNode=e;
        if (e->GetName() == "settings") SettingsNode=e;
        if (e->GetName() == "perspectives") PerspectivesNode=e;
    }
    if (ModelsNode == nullptr)
    {
        ModelsNode = new wxXmlNode( wxXML_ELEMENT_NODE, "models" );
        root->AddChild( ModelsNode );
        UnsavedRgbEffectsChanges = true;
    }
    if (EffectsNode == nullptr)
    {
        EffectsNode = new wxXmlNode( wxXML_ELEMENT_NODE, "effects" );
        EffectsNode->AddAttribute("version", XLIGHTS_RGBEFFECTS_VERSION);
        root->AddChild( EffectsNode );
        UnsavedRgbEffectsChanges = true;
    }
    if (PalettesNode == nullptr)
    {
        PalettesNode = new wxXmlNode( wxXML_ELEMENT_NODE, "palettes" );
        root->AddChild( PalettesNode );
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

    if (ModelGroupsNode == nullptr)
    {
        ModelGroupsNode = new wxXmlNode( wxXML_ELEMENT_NODE, "modelGroups" );
        root->AddChild( ModelGroupsNode );
        UnsavedRgbEffectsChanges = true;
    }

    if (LayoutGroupsNode == nullptr)
    {
        LayoutGroupsNode = new wxXmlNode( wxXML_ELEMENT_NODE, "layoutGroups" );
        root->AddChild( LayoutGroupsNode );
        UnsavedRgbEffectsChanges = true;
    }

    if (PerspectivesNode == nullptr)
    {
        PerspectivesNode = new wxXmlNode( wxXML_ELEMENT_NODE, "perspectives" );
        root->AddChild( PerspectivesNode );
        UnsavedRgbEffectsChanges = true;
    }

    if(SettingsNode== nullptr)
    {
        SettingsNode = new wxXmlNode( wxXML_ELEMENT_NODE, "settings" );
        root->AddChild( SettingsNode );
        SetXmlSetting("previewWidth","1280");
        SetXmlSetting("previewHeight","720");
        UnsavedRgbEffectsChanges = true;
    }
    int previewWidth=wxAtoi(GetXmlSetting("previewWidth","1280"));
    int previewHeight=wxAtoi(GetXmlSetting("previewHeight","720"));
    if (previewWidth==0 || previewHeight==0)
    {
        previewWidth = 1280;
        previewHeight = 720;
    }
    SetPreviewSize(previewWidth,previewHeight);

    mBackgroundImage = FixFile(GetShowDirectory(), GetXmlSetting("backgroundImage",""));
    ObtainAccessToURL(mBackgroundImage.ToStdString());
    if (mBackgroundImage != "" && (!wxFileExists(mBackgroundImage) || !wxIsReadable(mBackgroundImage))) {
        wxString fn = FixFile(mediaDirectory, GetXmlSetting("backgroundImage",""));
        ObtainAccessToURL(fn.ToStdString());
        if (wxFileExists(fn) && wxIsReadable(mBackgroundImage)) {
            mBackgroundImage = fn;
        } else {
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

    mStoredLayoutGroup = GetXmlSetting("storedLayoutGroup","Default");

    // validate stored preview exists
    bool found_saved_preview = false;
    for(wxXmlNode* e=LayoutGroupsNode->GetChildren(); e!=nullptr; e=e->GetNext() )
    {
        if (e->GetName() == "layoutGroup")
        {
            wxString grp_name=e->GetAttribute("name");
            if (!grp_name.IsEmpty())
            {
                if( grp_name.ToStdString() == mStoredLayoutGroup )
                {
                    found_saved_preview = true;
                }
            }
        }
    }
    if( !found_saved_preview )
    {
        mStoredLayoutGroup = "Default";
    }

    // Do this here as it may switch the background image
    LayoutGroups.clear();
    layoutPanel->Reset();
    AllModels.SetLayoutsNode(LayoutGroupsNode);  // provides easy access to layout names for the model class
    for(wxXmlNode* e=LayoutGroupsNode->GetChildren(); e!=nullptr; e=e->GetNext() )
    {
        if (e->GetName() == "layoutGroup")
        {
            wxString grp_name=e->GetAttribute("name");
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
    SetPreviewBackgroundBrightness(mBackgroundBrightness);
    mScaleBackgroundImage = wxAtoi(GetXmlSetting("scaleImage","0")) > 0;
    SetPreviewBackgroundScaled(mScaleBackgroundImage);

    std::string group = layoutPanel->GetCurrentLayoutGroup();
    if( group != "Default" && group != "All Models" && group != "Unassigned" ) {
        modelPreview->SetBackgroundBrightness(layoutPanel->GetBackgroundBrightnessForSelectedPreview());
        modelPreview->SetScaleBackgroundImage(layoutPanel->GetBackgroundScaledForSelectedPreview());
    }

    return effectsFile.GetFullPath();
}

void xLightsFrame::LoadEffectsFile()
{
    wxStopWatch sw; // start a stopwatch timer
    wxString filename=LoadEffectsFileNoCheck();
    // check version, do we need to convert?
    wxString version=EffectsNode->GetAttribute("version", "0000");
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
                if ( my_display == "1" ) {
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
                if ( selected == "1" ) {
                    groups_are_selected = true;
                    break;
                }
             }
        }
        // if no groups are selected then models remain as set above and all groups goto Default
        if( !groups_are_selected ) {
            for (wxXmlNode *group = ModelGroupsNode->GetChildren(); group != nullptr; group = group->GetNext()) {
                if (group->GetName() == "modelGroup") {
                    group->DeleteAttribute("selected");
                    group->DeleteAttribute("LayoutGroup");
                    group->AddAttribute("LayoutGroup", "Default");
                 }
            }
        } else { // otherwise need to set models in unchecked groups to unassigned
            std::set<std::string> modelsAdded;
            for (wxXmlNode *group = ModelGroupsNode->GetChildren(); group != nullptr; group = group->GetNext()) {
                if (group->GetName() == "modelGroup") {
                    std::string selected = group->GetAttribute("selected").ToStdString();
                    std::string layout_group = "Unassigned";
                    if( selected == "1" ) {
                        wxArrayString mn = wxSplit(group->GetAttribute("models"), ',');
                        for (int x = 0; x < mn.size(); x++) {
                            std::string name = mn[x].ToStdString();
                            if (modelsAdded.find(name) == modelsAdded.end()) {
                                modelsAdded.insert(mn[x].ToStdString());
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

    UpdateModelsList();
    displayElementsPanel->SetSequenceElementsModelsViews(&SeqData, &mSequenceElements, ModelsNode, ModelGroupsNode, &_sequenceViewManager);
    mSequencerInitialize = false;
    CheckForAndCreateDefaultPerpective();
    perspectivePanel->SetPerspectives(PerspectivesNode);
    LoadPerspectivesMenu(PerspectivesNode);
    float elapsedTime = sw.Time()/1000.0; //msec => sec
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

    for(wxXmlNode* p=perspectivesNode->GetChildren(); p!=NULL; p=p->GetNext() )
    {
        if (p->GetName() == "perspective")
        {
            wxString name=p->GetAttribute("name");
            if (!name.IsEmpty())
            {
                int id = wxNewId();
                MenuItemPerspectives->AppendRadioItem(id,name);
                if (mCurrentPerpective != NULL && (name == mCurrentPerpective->GetAttribute("name")))
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
        wxDir sd(CurrentDir);
        sd.Make(CurrentDir + "/xScheduleData");
    }

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
        if (!first)
        {
            modelsJSON.Write(",");
            first = false;
        }

        Model* model = m->second;
        if (model->GetDisplayAs() == "ModelGroup")
        {
            // Dont export model groups ... they arent useful

            //ModelGroup* mg = static_cast<ModelGroup*>(model);
            //modelsJSON.Write("{\"name\":\"" + mg->name +
            //    "\",\"type\":\"" + mg->GetDisplayAs() +
            //    "\",\"startchannel\":\"" + wxString::Format("%i", mg->NodeStartChannel(0) + 1) +
            //    "\",\"channels\":\"" + wxString::Format("%i", mg->GetChanCount()) +
            //    "\",\"stringtype\":\"\"}");
        }
        else
        {
            int ch = model->GetNumberFromChannelString(model->ModelStartChannel);
            modelsJSON.Write("{\"name\":\""+model->name+
                              "\",\"type\":\""+model->GetDisplayAs()+
                              "\",\"startchannel\":\""+wxString::Format("%i", ch)+
                              "\",\"channels\":\""+ wxString::Format("%i", model->GetChanCount()) +
                              "\",\"stringtype\":\""+ model->GetStringType() +"\"}");
        }
    }

    modelsJSON.Write("]}");
}

// returns true on success
bool xLightsFrame::SaveEffectsFile(bool backup)
{
    // dont save if currently saving
    std::unique_lock<std::mutex> lock(saveLock, std::try_to_lock);
    if (!lock.owns_lock()) return false;

	// Make sure the views are up to date before we save it
	_sequenceViewManager.Save(&EffectsXml);

	color_mgr.Save(&EffectsXml);

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
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.warn("Unable to save backup of RGB effects file");
        }
        else
        {
            wxMessageBox(_("Unable to save RGB effects file"), _("Error"));
        }
        return false;
    }
    if (!backup)
    {
        UnsavedRgbEffectsChanges = false;
    }

    SaveModelsFile();

    return true;
}

void xLightsFrame::CreateDefaultEffectsXml()
{
    wxXmlNode* root = new wxXmlNode( wxXML_ELEMENT_NODE, "xrgb" );
    EffectsXml.SetRoot( root );
    UnsavedRgbEffectsChanges = true;
}
bool xLightsFrame::RenameModel(const std::string OldName, const std::string& NewName)
{
    if (OldName == NewName) {
        return false;
    }
    Element* elem_to_rename = mSequenceElements.GetElement(OldName);
    if( elem_to_rename != NULL )
    {
        elem_to_rename->SetName(NewName);
    }
    bool internalsChanged = AllModels.Rename(OldName, NewName);
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
    RenderableEffect *reff = effectManager.GetEffect(PageName.ToStdString());
    if (reff != nullptr) {
        for(size_t i=0; i<cb->GetPageCount(); i++)
        {
            if (cb->GetPageText(i) == reff->ToolTip())
            {
                cb->ChangeSelection(i);
                return;
            }
        }
    }
}

void xLightsFrame::OnBitmapButtonSaveSeqClick(wxCommandEvent& event)
{
    SaveSequence();
}




static std::string chooseNewName(xLightsFrame *parent, std::vector<std::string> &names,
                                 const std::string &msg, const std::string curval) {
    wxTextEntryDialog dialog(parent, _("Enter new name"), msg, curval);
    int DlgResult;
    std::string NewName;
    do {
        DlgResult=dialog.ShowModal();
        if (DlgResult == wxID_OK) {
            // validate inputs
            NewName = dialog.GetValue().Trim();
            if (std::find(names.begin(), names.end(), NewName) == names.end()) {
                return NewName;
            }
        }
    }
    while (DlgResult == wxID_OK);
    return curval;
}

static void AddModelsToPreview(ModelGroup *grp, std::vector<Model *> &PreviewModels) {
    for (auto it2 = grp->Models().begin(); it2 != grp->Models().end(); it2++) {
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
    if (ModelsNode == nullptr) return; // this happens when xlights is first loaded

    playModel = nullptr;
    PreviewModels.clear();
    layoutPanel->GetMainPreview()->GetModels().clear();

    AllModels.LoadModels(ModelsNode,
                         modelPreview->GetVirtualCanvasWidth(),
                         modelPreview->GetVirtualCanvasHeight());

    std::vector<std::string> current;
    for (auto it = AllModels.begin(); it != AllModels.end(); it++) {
        current.push_back(it->first);
    }
    for (wxXmlNode* e=ModelGroupsNode->GetChildren(); e != NULL; e = e->GetNext()) {
        if (e->GetName() == "modelGroup") {
            std::string name = e->GetAttribute("name").ToStdString();
            current.push_back(name);
        }
    }
    for (wxXmlNode* e=ModelGroupsNode->GetChildren(); e != NULL; e = e->GetNext()) {
        if (e->GetName() == "modelGroup") {
            std::string name = e->GetAttribute("name").ToStdString();
            Model *model = AllModels[name];
            if (model != nullptr) {
                wxArrayString choices;
                choices.push_back("Rename Model");
                choices.push_back("Delete Model");
                choices.push_back("Rename Group");
                choices.push_back("Delete Group");

                wxString msg = "A model of name \'" + name + "\' already exists.  What action should we take?";
                wxSingleChoiceDialog dlg(this, msg, "Model/Group Name Conflict", choices, (void **)nullptr,
                                         wxDEFAULT_DIALOG_STYLE | wxOK | wxCENTRE | wxRESIZE_BORDER, wxDefaultPosition);
                bool done = false;
                do {
                    dlg.ShowModal();
                    int sel = dlg.GetSelection();
                    switch (sel) {
                        case 0:
                        case 1:
                            for (wxXmlNode* e2=ModelsNode->GetChildren(); e2!=NULL; e2=e2->GetNext()) {
                                if (e2->GetName() == "model") {
                                    std::string mname = e2->GetAttribute("name").ToStdString();
                                    if (mname == name) {
                                        UnsavedRgbEffectsChanges=true;
                                        if (sel == 1) {
                                            ModelsNode->RemoveChild(e2);
                                            done = true;
                                        } else {
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
                            UnsavedRgbEffectsChanges=true;
                            done = true;
                            if (e == nullptr) {
                                break;
                            }
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
    for (auto it = AllModels.begin(); it != AllModels.end(); it++) {
        Model *model = it->second;
        if (model->GetDisplayAs() != "ModelGroup") {
            if (model->GetLayoutGroup() == "Default" || model->GetLayoutGroup() == "All Previews") {
                PreviewModels.push_back(model);
            }
        }
    }

    // Now add all models to default House Preview that are in groups set to Default or All Previews
    for (auto it = AllModels.begin(); it != AllModels.end(); it++) {
        Model *model = it->second;
        if (model->GetDisplayAs() == "ModelGroup") {
            ModelGroup *grp = (ModelGroup*)model;
            if (model->GetLayoutGroup() == "All Previews" || model->GetLayoutGroup() == "Default") {
                AddModelsToPreview(grp, PreviewModels);
            }
        }
    }

    layoutPanel->UpdateModelList(true);
}

void xLightsFrame::OpenRenderAndSaveSequences(const wxArrayString &origFilenames) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (origFilenames.IsEmpty()) {
        EnableSequenceControls(true);
        printf("Done All Files\n");
        Destroy();
        return;
    }
    EnableSequenceControls(false);

    wxArrayString fileNames = origFilenames;
    wxString seq = fileNames[0];
    fileNames.RemoveAt(0);
    wxStopWatch sw; // start a stopwatch timer

    printf("Processing file %s\n", seq.ToStdString().c_str());
    OpenSequence(seq, nullptr);
    EnableSequenceControls(false);

    SetStatusText(_("Saving ") + xlightsFilename + _(" ... Rendering."));
    ProgressBar->Show();
    GaugeSizer->Layout();
    logger_base.info("Rendering on save.");
    RenderIseqData(true, NULL); // render ISEQ layers below the Nutcracker layer
    logger_base.info("   iseq below effects done.");
    ProgressBar->SetValue(10);
    RenderGridToSeqData([this, sw, fileNames] {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("   Effects done.");
        ProgressBar->SetValue(90);
        RenderIseqData(false, NULL);  // render ISEQ layers above the Nutcracker layer
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

        CallAfter(&xLightsFrame::OpenRenderAndSaveSequences, fileNames);
    } );
}
void xLightsFrame::SaveSequence()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (SeqData.NumFrames() == 0)
    {
        wxMessageBox("You must open a sequence first!", "Error");
        return;
    }

    wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
    wxPostEvent(this, playEvent);

    std::unique_lock<std::mutex> lock(saveLock);

    if (xlightsFilename.IsEmpty())
    {
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
                wxMessageBox(_("File name cannot be empty"), _("ERROR"));
            }
        }
        while (!ok);
        wxFileName oName(NewFilename);
        oName.SetExt("fseq");
        DisplayXlightsFilename(oName.GetFullPath());

        oName.SetExt("xml");
        CurrentSeqXmlFile->SetPath(oName.GetPath());
        CurrentSeqXmlFile->SetFullName(oName.GetFullName());
    }

    EnableSequenceControls(false);
    wxStopWatch sw; // start a stopwatch timer
    SetStatusText(_("Saving ")+xlightsFilename+_(" ... Saving xml."));
    logger_base.info("Saving XML file.");
    CurrentSeqXmlFile->Save(mSequenceElements);
    logger_base.info("XML file done.");

    if (mBackupOnSave)
    {
        DoBackup(false);
    }

    if (mRenderOnSave) {
        SetStatusText(_("Saving ") + xlightsFilename + _(" ... Rendering."));
        ProgressBar->Show();
        GaugeSizer->Layout();
        logger_base.info("Rendering on save.");
        RenderIseqData(true, NULL); // render ISEQ layers below the Nutcracker layer
        logger_base.info("   iseq below effects done.");
        ProgressBar->SetValue(10);
        RenderGridToSeqData([this, sw] {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.info("   Effects done.");
            ProgressBar->SetValue(90);
            RenderIseqData(false, NULL);  // render ISEQ layers above the Nutcracker layer
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
    SetStatusText(_("Saving ") + xlightsFilename + _(" ... Writing fseq."));
    WriteFalconPiFile(xlightsFilename);
    logger_base.info("fseq file done.");
    DisplayXlightsFilename(xlightsFilename);
    float elapsedTime = sw.Time()/1000.0; // now stop stopwatch timer and get elapsed time. change into seconds from ms
    wxString displayBuff = wxString::Format(_("%s     Updated in %7.3f seconds"),xlightsFilename,elapsedTime);
    logger_base.info("%s", (const char *)displayBuff.c_str());
    CallAfter(&xLightsFrame::SetStatusText, displayBuff, 0);
    EnableSequenceControls(true);
    mSavedChangeCount = mSequenceElements.GetChangeCount();
    mLastAutosaveCount = mSavedChangeCount;
}

void xLightsFrame::SaveAsSequence()
{
   if (SeqData.NumFrames() == 0)
    {
        wxMessageBox("You must open a sequence first!", "Error");
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
            wxMessageBox(_("File name cannot be empty"), _("ERROR"));
        }
    }
    while (!ok);
    wxFileName oName(NewFilename);
    oName.SetExt("fseq");
    DisplayXlightsFilename(oName.GetFullPath());

    oName.SetExt("xml");
    CurrentSeqXmlFile->SetPath(oName.GetPath());
    CurrentSeqXmlFile->SetFullName(oName.GetFullName());
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
    RenderIseqData(true, nullptr); // render ISEQ layers below the Nutcracker layer
    logger_base.info("   iseq below effects done.");
    ProgressBar->SetValue(10);
    RenderGridToSeqData([this, sw] {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("   Effects done.");
        ProgressBar->SetValue(90);
        RenderIseqData(false, nullptr);  // render ISEQ layers above the Nutcracker layer
        logger_base.info("   iseq above effects done. Render all complete.");
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
    wxWindowList &ChildList = parent->GetChildren();
    for (wxWindowList::iterator it = ChildList.begin(); it != ChildList.end(); ++it) {
        wxWindow * child = *it;
        child->Enable(enable);
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
    enableAllToolbarControls(WindowMgmtToolbar, enable && SeqData.NumFrames() > 0);
    enableAllToolbarControls(EffectsToolBar, enable && SeqData.NumFrames() > 0);
    enableAllToolbarControls(EditToolBar, enable && SeqData.NumFrames() > 0);
    enableAllToolbarControls(ViewToolBar, enable);
    enableAllToolbarControls(OutputToolBar, enable);

    enableAllChildControls(EffectsPanel1, enable && SeqData.NumFrames() > 0);
    enableAllChildControls(timingPanel, enable && SeqData.NumFrames() > 0);
    enableAllChildControls(bufferPanel, enable && SeqData.NumFrames() > 0);
    enableAllChildControls(perspectivePanel, enable && SeqData.NumFrames() > 0);
    enableAllChildControls(colorPanel, enable && SeqData.NumFrames() > 0);
    enableAllChildControls(effectPalettePanel, enable && SeqData.NumFrames() > 0);

    enableAllMenubarControls(MenuBar, enable);

    if (enable && SeqData.NumFrames() == 0) {
        //no file is loaded, disable save/render buttons
        EnableToolbarButton(MainToolBar,ID_AUITOOLBAR_SAVE,false);
        EnableToolbarButton(MainToolBar,ID_AUITOOLBAR_SAVEAS,false);
        EnableToolbarButton(MainToolBar,ID_AUITOOLBAR_RENDERALL,false);
        Menu_Settings_Sequence->Enable(false);
        MenuItem_File_Save_Sequence->Enable(false);
        MenuItem_File_Close_Sequence->Enable(false);
        MenuItem_PackageSequence->Enable(false);
        MenuItem_ExportEffects->Enable(false);
        MenuItem_ImportEffects->Enable(false);
        MenuSettings->Enable(ID_MENUITEM_RENDER_MODE, false);
    }
    if (!enable && SeqData.NumFrames() > 0) {
        //file is loaded, but we're doing something that requires controls disabled (such as rendering)
        //we need to also disable the quit button
        QuitMenuItem->Enable(false);
    } else {
        QuitMenuItem->Enable(true);
    }
}



//modifed for partially random -DJ
//void djdebug(const char* fmt, ...); //_DJ
std::string xLightsFrame::CreateEffectStringRandom(std::string &settings, std::string &palette)
{
    int eff1;
    if (EffectsPanel1->isRandom_()) { //avoid a few types of random effects
        eff1 = ChooseRandomEffect();
    } else {
        eff1 = EffectsPanel1->EffectChoicebook->GetSelection();
    }

    settings = EffectsPanel1->GetRandomEffectString(eff1);


    palette = colorPanel->GetRandomColorString();
    return effectManager[eff1]->Name();
}

int xLightsFrame::ChooseRandomEffect()
{
    int eff,count=0;
    const static int MAX_TRIES=10;

    do {
        count++;
        eff=rand() % effectManager.size();
    } while (!effectManager[eff]->CanBeRandom() && count < MAX_TRIES);

    if(count==MAX_TRIES) eff = 0; // we failed to find a good effect after MAX_TRIES attempts
    return eff;
}



