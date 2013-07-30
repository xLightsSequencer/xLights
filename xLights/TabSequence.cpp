void xLightsFrame::CreateDefaultEffectsXml()
{
    wxXmlNode* root = new wxXmlNode( wxXML_ELEMENT_NODE, wxT("xrgb") );
    EffectsXml.SetRoot( root );
}


wxXmlNode* xLightsFrame::GetModelNode(const wxString& name)
{
    wxXmlNode* e;
    for(e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == wxT("model"))
        {
            if (name == e->GetAttribute(wxT("name"))) return e;
        }
    }
    return NULL;
}

void xLightsFrame::OnButton_PlayAllClick(wxCommandEvent& event)
{
    if (SeqData.size() == 0)
    {
        wxMessageBox(wxT("You must open a sequence first!"), wxT("Error"));
        return;
    }
    SeqPlayColumn=Grid1->GetGridCursorCol();
    if (SeqPlayColumn < 2)
    {
        wxMessageBox(wxT("Select a cell in a display element column before clicking Play"), wxT("Error"));
        return;
    }
    NextGridRowToPlay=Grid1->GetGridCursorRow();

    wxString ModelName=Grid1->GetColLabelValue(SeqPlayColumn);
    Choice_Models->SetStringSelection(ModelName);
    int sel=Choice_Models->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        wxMessageBox(_("Can not find model: ")+ModelName, _("Error"));
        return;
    }
    wxXmlNode* ModelXml=(wxXmlNode*)Choice_Models->GetClientData(sel);
    buffer.InitBuffer(ModelXml);
    ClearEffectWindow();
    StatusBar1->SetStatusText(_("Playback: RGB sequence"));
    EnableSequenceControls(false);
    PlayCurrentXlightsFile();
}

void xLightsFrame::ResetEffectStates()
{
    ResetEffectState[0]=true;
    ResetEffectState[1]=true;
}

void xLightsFrame::OnButton_PlayEffectClick(wxCommandEvent& event)
{
    int sel=Choice_Models->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        wxMessageBox(_("No model is selected"), _("ERROR"));
        return;
    }
    wxXmlNode* ModelXml=(wxXmlNode*)Choice_Models->GetClientData(sel);
    buffer.InitBuffer(ModelXml);
    ResetEffectStates();
    ClearEffectWindow();
    buffer.SetMixType(Choice_LayerMethod->GetStringSelection());
    StatusBar1->SetStatusText(_("Playback: effect"));
    EnableSequenceControls(false);
    ResetTimer(PLAYING_EFFECT);
}

void xLightsFrame::EnableSequenceControls(bool enable)
{
    Button_PlayEffect->Enable(enable && Choice_Models->GetCount() > 0);
    Button_PlayRgbSeq->Enable(enable && Grid1->GetNumberCols() > 2);
    Button_Models->Enable(enable && ModelsNode);
    Button_Presets->Enable(enable && EffectsNode);
    Button_PresetAdd->Enable(enable && EffectsNode);
    Button_PresetUpdate->Enable(enable && EffectsNode);
    Choice_Models->Enable(enable);
    Button_Pictures1_Filename->Enable(enable);
    TextCtrl_Pictures1_Filename->Enable(enable);
    Button_Pictures2_Filename->Enable(enable);
    TextCtrl_Pictures2_Filename->Enable(enable);
    ButtonSeqExport->Enable(enable && Grid1->GetNumberCols() > 2);
    BitmapButtonOpenSeq->Enable(enable);
    BitmapButtonSaveSeq->Enable(enable);
    BitmapButtonInsertRow->Enable(enable);
    BitmapButtonDeleteRow->Enable(enable);
    ButtonDisplayElements->Enable(enable && ModelsNode);
}

void xLightsFrame::OnButton_PresetsClick(wxCommandEvent& event)
{
    EffectListDialog dialog(this);
    wxString name;
    for(wxXmlNode* e=EffectsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == wxT("effect"))
        {
            name=e->GetAttribute(wxT("name"));
            if (!name.IsEmpty())
            {
                dialog.ListBox1->Append(name,e);
            }
        }
    }
    dialog.ShowModal();
    UpdateEffectsList();
    PresetsSelect();
    SaveEffectsFile();
}

void xLightsFrame::SetChoicebook(wxChoicebook* cb, wxString& PageName)
{
    for(size_t i=0; i<cb->GetPageCount(); i++)
    {
        if (cb->GetPageText(i) == PageName)
        {
            cb->SetSelection(i);
            return;
        }
    }
}

void xLightsFrame::SetEffectControls(wxString settings)
{
    long TempLong;
    wxColour color;
    wxWindow *CtrlWin;
    wxString before,after,name,value;
    int cnt=0;
    while (!settings.IsEmpty())
    {
        before=settings.BeforeFirst(',');
        after=settings.AfterFirst(',');
        switch (cnt)
        {
        case 0:
            SetChoicebook(Choicebook1,before);
            break;
        case 1:
            SetChoicebook(Choicebook2,before);
            break;
        case 2:
            Choice_LayerMethod->SetStringSelection(before);
            break;
        default:
            name=before.BeforeFirst('=');
            value=before.AfterFirst('=');
            CtrlWin=wxWindow::FindWindowByName(name);
            if (CtrlWin)
            {
                if (name.StartsWith(wxT("ID_SLIDER")))
                {
                    wxSlider* ctrl=(wxSlider*)CtrlWin;
                    if (value.ToLong(&TempLong)) ctrl->SetValue(TempLong);
                }
                else if (name.StartsWith(wxT("ID_TEXTCTRL")))
                {
                    wxTextCtrl* ctrl=(wxTextCtrl*)CtrlWin;
                    ctrl->SetValue(value);
                }
                else if (name.StartsWith(wxT("ID_CHOICE")))
                {
                    wxChoice* ctrl=(wxChoice*)CtrlWin;
                    ctrl->SetStringSelection(value);
                }
                else if (name.StartsWith(wxT("ID_BUTTON")))
                {
                    color.Set(value);
                    CtrlWin->SetBackgroundColour(color);
                    SetTextColor(CtrlWin);
                }
                else if (name.StartsWith(wxT("ID_CHECKBOX")))
                {
                    wxCheckBox* ctrl=(wxCheckBox*)CtrlWin;
                    if (value.ToLong(&TempLong)) ctrl->SetValue(TempLong!=0);
                }
                else
                {
                    wxMessageBox(wxT("Unknown type: ")+name, wxT("Internal Error"));
                }
            }
            else
            {
                wxMessageBox(wxT("Unable to find: ")+name, wxT("Internal Error"));
            }
            break;
        }
        settings=after;
        cnt++;
    }
    PaletteChanged=true;
    MixTypeChanged=true;
    ResetEffectStates();
}

// Set text to a color that contrasts with background
void xLightsFrame::SetTextColor(wxWindow* w)
{
    wxColour color=w->GetBackgroundColour();
    int test=color.Red()*0.299 + color.Green()*0.587 + color.Blue()*0.114;
    w->SetForegroundColour(test < 186 ? *wxWHITE : *wxBLACK);
}

void xLightsFrame::PresetsSelect()
{
    int NameIdx=Choice_Presets->GetSelection();
    if (NameIdx != wxNOT_FOUND)
    {
        wxXmlNode* x=(wxXmlNode*)Choice_Presets->GetClientData(NameIdx);
        SetEffectControls(x->GetAttribute("settings"));
    }
}

void xLightsFrame::OnChoice_PresetsSelect(wxCommandEvent& event)
{
    PresetsSelect();
}

void xLightsFrame::OnButton_PresetAddClick(wxCommandEvent& event)
{
    wxTextEntryDialog dialog(this,_("Enter preset name"),_("Add New Preset"));
    int DlgResult;
    bool ok;
    wxString name;
    do
    {
        ok=true;
        DlgResult=dialog.ShowModal();
        if (DlgResult == wxID_OK)
        {
            // validate inputs
            name=dialog.GetValue();
            name.Trim();
            if (name.IsEmpty())
            {
                ok=false;
                wxMessageBox(_("A preset name cannot be empty"), _("ERROR"));
            }
            else if (Choice_Presets->FindString(name) != wxNOT_FOUND)
            {
                ok=false;
                wxMessageBox(_("That name is already in use"), _("ERROR"));
            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
    if (DlgResult != wxID_OK) return;

    // update Choice_Presets
    EffectsNode->AddChild(CreateEffectNode(name));
    UpdateEffectsList();
    Choice_Presets->SetStringSelection(name);
    SaveEffectsFile();
}

wxXmlNode* xLightsFrame::CreateEffectNode(wxString& name)
{
    wxXmlNode* NewXml=new wxXmlNode(wxXML_ELEMENT_NODE, wxT("effect"));
    NewXml->AddAttribute(wxT("name"), name);
    NewXml->AddAttribute(wxT("settings"), CreateEffectString());
    return NewXml;
}

wxString xLightsFrame::CreateEffectStringRandom()
{
    int eff1, eff2, layerOp;
    wxString s;
    s.clear();

    eff1 = rand() % eff_LASTEFFECT; //Temporarily prevent choosing to do a circle.
    eff2 = rand() % eff_LASTEFFECT;
    eff1 = (eff_NONE == eff1|| eff_TEXT == eff1 || eff_PICTURES == eff1)? eff1+1:eff1;
    eff2 = (eff_NONE == eff2|| eff_TEXT == eff2 || eff_PICTURES == eff2)? eff2+1:eff2;
    if(eff_PIANO == eff1) eff1 = eff_BARS; // 7-30-13 (scm) , protect us if e go out of range
    if(eff_PIANO == eff2) eff2 = eff_BARS; //

    layerOp = rand() % LASTLAYER;
    s=EffectNames[eff1]+wxT(",")+EffectNames[eff2]+wxT(",")+EffectLayerOptions[layerOp];
    s+=wxT(",ID_SLIDER_SparkleFrequency=")+wxString::Format(wxT("%d"),Slider_SparkleFrequency->GetMax()); // max is actually all teh way left, ie no sparkles

    s+=wxT(",ID_SLIDER_Brightness=")+wxString::Format(wxT("%d"),Slider_Brightness->GetValue());
    s+=wxT(",ID_SLIDER_Contrast=")+wxString::Format(wxT("%d"),0);
    s+=wxT(",ID_SLIDER_Speed1=")+wxString::Format(wxT("%d"),rand()%Slider_Speed1->GetMax()+Slider_Speed1->GetMin());
    s+=wxT(",ID_SLIDER_Speed2=")+wxString::Format(wxT("%d"),rand()%Slider_Speed2->GetMax()+Slider_Speed2->GetMin());
    s+=PageControlsToStringRandom(Choicebook1->GetPage(eff1));
    s+=SizerControlsToStringRandom(FlexGridSizer_Palette1);
    s+=PageControlsToStringRandom(Choicebook2->GetPage(eff2));
    s+=SizerControlsToStringRandom(FlexGridSizer_Palette2);
    return s;

}

wxString xLightsFrame::PageControlsToStringRandom(wxWindow* page)
{
    wxString s;
    wxWindowList &ChildList = page->GetChildren();
    for ( wxWindowList::Node *node = ChildList.GetFirst(); node; node = node->GetNext() )
    {
        wxWindow *ChildWin = (wxWindow *)node->GetData();
        wxString ChildName=ChildWin->GetName();
        if (ChildName.StartsWith(wxT("ID_SLIDER")))
        {
            wxSlider* ctrl=(wxSlider*)ChildWin;
            if(ChildName.Contains(wxT("Spirograph1_r")) || ChildName.Contains(wxT("Spirograph2_r")))
            {
                // always set little radius, r, to its minimum value
                s+=","+ChildName+"="+wxString::Format(wxT("%d"), 0 );
            }
            else if(ChildName.Contains(wxT("Spirograph1_Animate")) || ChildName.Contains(wxT("Spirograph2_Animate")))
            {
                // always animate spirograph
                s+=","+ChildName+"="+wxString::Format(wxT("%d"), 1 );
            }
            else
            {
                s+=","+ChildName+"="+wxString::Format(wxT("%d"),rand() % ctrl->GetMax() + ctrl->GetMin() );
            }
        }
        else if (ChildName.StartsWith(wxT("ID_TEXTCTRL")))
        {
            wxTextCtrl* ctrl=(wxTextCtrl*)ChildWin;
            s+=","+ChildName+"="+ctrl->GetValue();
        }
        else if (ChildName.StartsWith(wxT("ID_CHOICE")))
        {
            wxChoice* ctrl=(wxChoice*)ChildWin;
            s+=","+ChildName+"="+ctrl->GetString(rand()%ctrl->GetCount());
        }
        else if (ChildName.StartsWith(wxT("ID_CHECKBOX")))
        {
            wxCheckBox* ctrl=(wxCheckBox*)ChildWin;
            wxString v=(rand()%2) ? wxT("1") : wxT("0");
            s+=","+ChildName+"="+v;
        }
    }
    return s;
}
wxString xLightsFrame::SizerControlsToStringRandom(wxSizer* sizer)
{
    wxString s;
    wxSizerItemList &ChildList = sizer->GetChildren();
    for ( wxSizerItemList::iterator it = ChildList.begin(); it != ChildList.end(); ++it )
    {
        if (!(*it)->IsWindow()) continue;
        wxWindow *ChildWin = (*it)->GetWindow();
        wxString ChildName=ChildWin->GetName();
        if (ChildName.StartsWith(wxT("ID_BUTTON")))
        {
            wxColour color=ChildWin->GetBackgroundColour();
            s+=","+ChildName+"="+color.GetAsString(wxC2S_HTML_SYNTAX);
        }
        else if (ChildName.StartsWith(wxT("ID_CHECKBOX")))
        {
            wxCheckBox* ctrl=(wxCheckBox*)ChildWin;
            wxString v=(rand() % 2) ? wxT("1") : wxT("0");
            s+=","+ChildName+"="+v;
        }
    }
    return s;
}
wxString xLightsFrame::CreateEffectString()
{
    int PageIdx1=Choicebook1->GetSelection();
    int PageIdx2=Choicebook2->GetSelection();
    // ID_CHOICEBOOK1, ID_CHOICEBOOK2, ID_CHOICE_LayerMethod
    wxString s=Choicebook1->GetPageText(PageIdx1)+wxT(",")+Choicebook2->GetPageText(PageIdx2);
    s+=wxT(",")+Choice_LayerMethod->GetStringSelection();
    s+=wxT(",ID_SLIDER_SparkleFrequency=")+wxString::Format(wxT("%d"),Slider_SparkleFrequency->GetValue());
    s+=wxT(",ID_SLIDER_Brightness=")+wxString::Format(wxT("%d"),Slider_Brightness->GetValue());
    s+=wxT(",ID_SLIDER_Contrast=")+wxString::Format(wxT("%d"),Slider_Contrast->GetValue());
    s+=wxT(",ID_SLIDER_Speed1=")+wxString::Format(wxT("%d"),Slider_Speed1->GetValue());
    s+=wxT(",ID_SLIDER_Speed2=")+wxString::Format(wxT("%d"),Slider_Speed2->GetValue());
    s+=PageControlsToString(Choicebook1->GetPage(PageIdx1));
    if(PageIdx1==eff_TEXT)
    {
        s+=PageControlsToString(Notebook_Text1->GetPage(0));
        s+=PageControlsToString(Notebook_Text1->GetPage(1));
    }
    s+=SizerControlsToString(FlexGridSizer_Palette1);
    s+=PageControlsToString(Choicebook2->GetPage(PageIdx2));
    s+=SizerControlsToString(FlexGridSizer_Palette2);
    return s;
}

void xLightsFrame::OnButton_UpdateGridClick(wxCommandEvent& event)
{
    int r,c;
    wxString v=CreateEffectString();
    if ( Grid1->IsSelection() )
    {
        // iterate over entire grid looking for selected cells
        int nRows = Grid1->GetNumberRows();
        int nCols = Grid1->GetNumberCols();
        for (r=0; r<nRows; r++)
        {
            for (c=2; c<nCols; c++)
            {
                if (Grid1->IsInSelection(r,c))
                {
                    Grid1->SetCellValue(r,c,v);
                }
            }
        }
    }
    else
    {
        // copy to current cell
        r=Grid1->GetGridCursorRow();
        c=Grid1->GetGridCursorCol();
        if (c >=2)
        {
            Grid1->SetCellValue(r,c,v);
        }
    }
    UnsavedChanges = true;
}
void xLightsFrame::InsertRandomEffects(wxCommandEvent& event)
{
    int r,c;
    wxString v;

    if ( Grid1->IsSelection() )
    {
        // iterate over entire grid looking for selected cells
        int nRows = Grid1->GetNumberRows();
        int nCols = Grid1->GetNumberCols();
        for (r=0; r<nRows; r++)
        {
            for (c=2; c<nCols; c++)
            {
                if (Grid1->IsInSelection(r,c))
                {
                    v = CreateEffectStringRandom();
                    Grid1->SetCellValue(r,c,v);
                }
            }
        }
    }
    else
    {
        // copy to current cell
        r=Grid1->GetGridCursorRow();
        c=Grid1->GetGridCursorCol();
        if (c >=2)
        {
            v = CreateEffectStringRandom();
            Grid1->SetCellValue(r,c,v);
        }
    }
}
void xLightsFrame::DeleteSelectedEffects(wxCommandEvent& event)
{
    int r,c;
    wxString v;
    v.Clear();

    if ( Grid1->IsSelection() )
    {
        // iterate over entire grid looking for selected cells
        int nRows = Grid1->GetNumberRows();
        int nCols = Grid1->GetNumberCols();
        for (r=0; r<nRows; r++)
        {
            for (c=2; c<nCols; c++)
            {
                if (Grid1->IsInSelection(r,c))
                {
                    Grid1->SetCellValue(r,c,v);
                }
            }
        }
    }
    else
    {
        // copy to current cell
        r=Grid1->GetGridCursorRow();
        c=Grid1->GetGridCursorCol();
        if (c >=2)
        {
            Grid1->SetCellValue(r,c,v);
        }
    }
}

void xLightsFrame::OnButton_PresetUpdateClick(wxCommandEvent& event)
{
    int NameIdx=Choice_Presets->GetSelection();
    if (NameIdx == wxNOT_FOUND)
    {
        wxMessageBox(_("No preset name is selected"), _("ERROR"));
        return;
    }
    // update preset
    // delete old xml entry
    wxXmlNode* OldXml=(wxXmlNode*)Choice_Presets->GetClientData(NameIdx);
    EffectsNode->RemoveChild(OldXml);
    delete OldXml;

    wxString name=Choice_Presets->GetString(NameIdx);
    EffectsNode->AddChild(CreateEffectNode(name));
    UpdateEffectsList();
    Choice_Presets->SetStringSelection(name);
    SaveEffectsFile();
}

void xLightsFrame::OnChoice_LayerMethodSelect(wxCommandEvent& event)
{
    MixTypeChanged=true;
}

void xLightsFrame::OnButton_ModelsClick(wxCommandEvent& event)
{
    ModelListDialog dialog(this);
    wxString name;
    wxXmlNode* e;
    for(e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == wxT("model"))
        {
            name=e->GetAttribute(wxT("name"));
            if (!name.IsEmpty())
            {
                dialog.ListBox1->Append(name,e);
            }
        }
    }
    dialog.HtmlEasyPrint=HtmlEasyPrint;
    dialog.ShowModal();

    // append any new models to the main xml structure
    for(size_t i=0; i<dialog.ListBox1->GetCount(); i++)
    {
        e=(wxXmlNode*)dialog.ListBox1->GetClientData(i);
        if (!e->GetParent())
        {
            ModelsNode->AddChild(e);
        }
    }
    SaveEffectsFile();
    UpdateModelsList();
    EnableSequenceControls(true);
}

void xLightsFrame::OnCheckBox_PaletteClick(wxCommandEvent& event)
{
    PaletteChanged=true;
}

void xLightsFrame::UpdateEffectsList()
{
    wxString name;
    wxString SelectedStr=Choice_Presets->GetStringSelection();
    Choice_Presets->Clear();
    for(wxXmlNode* e=EffectsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == wxT("effect"))
        {
            name=e->GetAttribute(wxT("name"));
            if (!name.IsEmpty())
            {
                Choice_Presets->Append(name,e);
            }
        }
    }

    // select a preset if one exists
    Choice_Presets->SetSelection(-1);
    if (Choice_Presets->GetCount() > 0 && !SelectedStr.IsEmpty())
    {
        Choice_Presets->SetStringSelection(SelectedStr);
    }
}

void xLightsFrame::UpdateModelsList()
{
    //TODO: Add code to read in model list with v2 values
    wxString name;
    wxString SelectedStr=Choice_Models->GetStringSelection();
    Choice_Models->Clear();
    for(wxXmlNode* e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == wxT("model"))
        {
            name=e->GetAttribute(wxT("name"));
            if (!name.IsEmpty())
            {
                Choice_Models->Append(name,e);
            }
        }
    }

    // select a model if one exists
    if (Choice_Models->GetCount() > 0)
    {
        if (SelectedStr.IsEmpty() || !Choice_Models->SetStringSelection(SelectedStr))
        {
            Choice_Models->SetSelection(0);
        }
        Button_PlayEffect->Enable(play_mode == play_off);
    }
}

wxString xLightsFrame::PageControlsToString(wxWindow* page)
{
    wxString s;
    wxWindowList &ChildList = page->GetChildren();
    for ( wxWindowList::Node *node = ChildList.GetFirst(); node; node = node->GetNext() )
    {
        wxWindow *ChildWin = (wxWindow *)node->GetData();
        wxString ChildName=ChildWin->GetName();
        if (ChildName.StartsWith(wxT("ID_SLIDER")))
        {
            wxSlider* ctrl=(wxSlider*)ChildWin;
            s+=","+ChildName+"="+wxString::Format(wxT("%d"),ctrl->GetValue());
        }
        else if (ChildName.StartsWith(wxT("ID_TEXTCTRL")))
        {
            wxTextCtrl* ctrl=(wxTextCtrl*)ChildWin;
            s+=","+ChildName+"="+ctrl->GetValue();
        }
        else if (ChildName.StartsWith(wxT("ID_CHOICE")))
        {
            wxChoice* ctrl=(wxChoice*)ChildWin;
            s+=","+ChildName+"="+ctrl->GetStringSelection();
        }
        else if (ChildName.StartsWith(wxT("ID_CHECKBOX")))
        {
            wxCheckBox* ctrl=(wxCheckBox*)ChildWin;
            wxString v=(ctrl->IsChecked()) ? wxT("1") : wxT("0");
            s+=","+ChildName+"="+v;
        }
    }
    return s;
}

// used to save palette
wxString xLightsFrame::SizerControlsToString(wxSizer* sizer)
{
    wxString s;
    wxSizerItemList &ChildList = sizer->GetChildren();
    for ( wxSizerItemList::iterator it = ChildList.begin(); it != ChildList.end(); ++it )
    {
        if (!(*it)->IsWindow()) continue;
        wxWindow *ChildWin = (*it)->GetWindow();
        wxString ChildName=ChildWin->GetName();
        if (ChildName.StartsWith(wxT("ID_BUTTON")))
        {
            wxColour color=ChildWin->GetBackgroundColour();
            s+=","+ChildName+"="+color.GetAsString(wxC2S_HTML_SYNTAX);
        }
        else if (ChildName.StartsWith(wxT("ID_CHECKBOX")))
        {
            wxCheckBox* ctrl=(wxCheckBox*)ChildWin;
            wxString v=(ctrl->IsChecked()) ? wxT("1") : wxT("0");
            s+=","+ChildName+"="+v;
        }
    }
    return s;
}

void xLightsFrame::ResetEffectsXml()
{
    ModelsNode=0;
    EffectsNode=0;
    PalettesNode=0;
}

void xLightsFrame::LoadEffectsFile()
{
    ResetEffectsXml();
    wxFileName effectsFile;
    effectsFile.AssignDir( CurrentDir );
    effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
    if (!effectsFile.FileExists())
    {
        // file does not exist, so create an empty xml doc
        CreateDefaultEffectsXml();
    }
    else if (!EffectsXml.Load( effectsFile.GetFullPath() ))
    {
        wxMessageBox(_("Unable to load RGB effects file"), _("Error"));
        CreateDefaultEffectsXml();
    }
    wxXmlNode* root=EffectsXml.GetRoot();
    if (root->GetName() != wxT("xrgb"))
    {
        wxMessageBox(_("Invalid RGB effects file. Press Save File button to start a new file."), _("Error"));
        CreateDefaultEffectsXml();
    }
    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == wxT("models")) ModelsNode=e;
        if (e->GetName() == wxT("effects")) EffectsNode=e;
        if (e->GetName() == wxT("palettes")) PalettesNode=e;
    }
    if (ModelsNode == 0)
    {
        ModelsNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT("models") );
        root->AddChild( ModelsNode );
    }
    if (EffectsNode == 0)
    {
        EffectsNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT("effects") );
        root->AddChild( EffectsNode );
    }
    if (PalettesNode == 0)
    {
        PalettesNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT("palettes") );
        root->AddChild( PalettesNode );
    }
    UpdateModelsList();
    UpdateEffectsList();
}

// returns true on success
bool xLightsFrame::SaveEffectsFile()
{
    wxFileName effectsFile;
    effectsFile.AssignDir( CurrentDir );
    effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
    if (!EffectsXml.Save( effectsFile.GetFullPath() ))
    {
        wxMessageBox(_("Unable to save RGB effects file"), _("Error"));
        return false;
    }
    UnsavedChanges=false;
    return true;
}

// PaletteNum should be 1 or 2
void xLightsFrame::UpdateBufferPaletteFromMap(int PaletteNum, MapStringString& SettingsMap)
{
    wxColourVector newcolors;
    for (int i=1; i<=6; i++)
    {
        if (SettingsMap[wxString::Format(wxT("ID_CHECKBOX_Palette%d_%d"),PaletteNum,i)] ==  wxT("1"))
        {
            newcolors.push_back(wxColour(SettingsMap[wxString::Format(wxT("ID_BUTTON_Palette%d_%d"),PaletteNum,i)]));
        }
    }
    buffer.SetPalette(PaletteNum-1,newcolors);
}

void xLightsFrame::UpdateBufferPalette()
{
    wxColourVector newcolors;

    // effect 1
    if (CheckBox_Palette1_1->IsChecked()) newcolors.push_back(Button_Palette1_1->GetBackgroundColour());
    if (CheckBox_Palette1_2->IsChecked()) newcolors.push_back(Button_Palette1_2->GetBackgroundColour());
    if (CheckBox_Palette1_3->IsChecked()) newcolors.push_back(Button_Palette1_3->GetBackgroundColour());
    if (CheckBox_Palette1_4->IsChecked()) newcolors.push_back(Button_Palette1_4->GetBackgroundColour());
    if (CheckBox_Palette1_5->IsChecked()) newcolors.push_back(Button_Palette1_5->GetBackgroundColour());
    if (CheckBox_Palette1_6->IsChecked()) newcolors.push_back(Button_Palette1_6->GetBackgroundColour());
    buffer.SetPalette(0,newcolors);

    // effect 2
    newcolors.clear();
    if (CheckBox_Palette2_1->IsChecked()) newcolors.push_back(Button_Palette2_1->GetBackgroundColour());
    if (CheckBox_Palette2_2->IsChecked()) newcolors.push_back(Button_Palette2_2->GetBackgroundColour());
    if (CheckBox_Palette2_3->IsChecked()) newcolors.push_back(Button_Palette2_3->GetBackgroundColour());
    if (CheckBox_Palette2_4->IsChecked()) newcolors.push_back(Button_Palette2_4->GetBackgroundColour());
    if (CheckBox_Palette2_5->IsChecked()) newcolors.push_back(Button_Palette2_5->GetBackgroundColour());
    if (CheckBox_Palette2_6->IsChecked()) newcolors.push_back(Button_Palette2_6->GetBackgroundColour());
    buffer.SetPalette(1,newcolors);
}

void xLightsFrame::RenderEffectFromString(int layer, int period, MapStringString& SettingsMap)
{
    wxString LayerStr=layer==0 ? wxT("1") : wxT("2");
    wxString SpeedStr=SettingsMap[wxT("ID_SLIDER_Speed")+LayerStr];
    buffer.SetLayer(layer,period,wxAtoi(SpeedStr),ResetEffectState[layer]);
    ResetEffectState[layer]=false;
    wxString effect=SettingsMap[wxT("effect")+LayerStr];
    if (effect == wxT("Bars"))
    {
        buffer.RenderBars(wxAtoi(SettingsMap[wxT("ID_SLIDER_Bars")+LayerStr+wxT("_BarCount")]),
                          BarEffectDirections.Index(SettingsMap[wxT("ID_CHOICE_Bars")+LayerStr+wxT("_Direction")]),
                          SettingsMap[wxT("ID_CHECKBOX_Bars")+LayerStr+wxT("_Highlight")]==wxT("1"),
                          SettingsMap[wxT("ID_CHECKBOX_Bars")+LayerStr+wxT("_3D")]==wxT("1"));
    }
    else if (effect == wxT("Butterfly"))
    {
        buffer.RenderButterfly(ButterflyEffectColors.Index(SettingsMap[wxT("ID_CHOICE_Butterfly")+LayerStr+wxT("_Colors")]),
                               wxAtoi(SettingsMap[wxT("ID_SLIDER_Butterfly")+LayerStr+wxT("_Style")]),
                               wxAtoi(SettingsMap[wxT("ID_SLIDER_Butterfly")+LayerStr+wxT("_Chunks")]),
                               wxAtoi(SettingsMap[wxT("ID_SLIDER_Butterfly")+LayerStr+wxT("_Skip")]));
    }
    else if (effect == wxT("Color Wash"))
    {
        buffer.RenderColorWash(SettingsMap[wxT("ID_CHECKBOX_ColorWash")+LayerStr+wxT("_HFade")]==wxT("1"),
                               SettingsMap[wxT("ID_CHECKBOX_ColorWash")+LayerStr+wxT("_VFade")]==wxT("1"),
                               wxAtoi(SettingsMap[wxT("ID_SLIDER_ColorWash")+LayerStr+wxT("_Count")]));
    }
    else if (effect == wxT("Fire"))
    {
        buffer.RenderFire(wxAtoi(SettingsMap[wxT("ID_SLIDER_Fire")+LayerStr+wxT("_Height")]),
                          wxAtoi(SettingsMap[wxT("ID_SLIDER_HueShift")+LayerStr+wxT("_HueShift")]),
                          SettingsMap[wxT("ID_CHECKBOX_Fire")+LayerStr+wxT("_GrowFire")]==wxT("1"));
    }
    else if (effect == wxT("Garlands"))
    {
        buffer.RenderGarlands(wxAtoi(SettingsMap[wxT("ID_SLIDER_Garlands")+LayerStr+wxT("_Type")]),
                              wxAtoi(SettingsMap[wxT("ID_SLIDER_Garlands")+LayerStr+wxT("_Spacing")]));
    }
    else if (effect == wxT("Life"))
    {
        buffer.RenderLife(wxAtoi(SettingsMap[wxT("ID_SLIDER_Life")+LayerStr+wxT("_Count")]),

                          wxAtoi(SettingsMap[wxT("ID_SLIDER_Life")+LayerStr+wxT("_Seed")]));
    }
    else if (effect == wxT("Meteors"))
    {
        buffer.RenderMeteors(MeteorsEffectTypes.Index(SettingsMap[wxT("ID_CHOICE_Meteors")+LayerStr+wxT("_Type")]),
                             wxAtoi(SettingsMap[wxT("ID_SLIDER_Meteors")+LayerStr+wxT("_Count")]),
                             wxAtoi(SettingsMap[wxT("ID_SLIDER_Meteors")+LayerStr+wxT("_Length")]),
                             SettingsMap[wxT("ID_CHECKBOX_Meteors")+LayerStr+wxT("_FallUp")]==wxT("1"),
                             MeteorsEffect.Index(SettingsMap[wxT("ID_CHOICE_MeteorsEffect")+LayerStr+wxT("_Type")]),
                             wxAtoi(SettingsMap[wxT("ID_SLIDER_Meteors")+LayerStr+wxT("_Swirl_Intensity")]));

        //                 buffer.RenderMeteors(MeteorsEffectTypes.Index(SettingsMap[wxT("ID_CHOICE_Meteors")+LayerStr+wxT("_Type")]),
        //              wxAtoi(SettingsMap[wxT("Slider_Meteors")+LayerStr+wxT("_Count")]),
        //            wxAtoi(SettingsMap[wxT("Slider_Meteors")+LayerStr+wxT("_Length")]));
    }
    else if (effect == wxT("Pictures"))
    {
        buffer.RenderPictures(EffectDirections.Index(SettingsMap[wxT("ID_CHOICE_Pictures")+LayerStr+wxT("_Direction")]),
                              SettingsMap[wxT("ID_TEXTCTRL_Pictures")+LayerStr+wxT("_Filename")],
                              wxAtoi(SettingsMap[wxT("ID_SLIDER_Pictures")+LayerStr+wxT("_GifType")])
                             );
    }
    else if (effect == wxT("Snowflakes"))
    {
        buffer.RenderSnowflakes(wxAtoi(SettingsMap[wxT("ID_SLIDER_Snowflakes")+LayerStr+wxT("_Count")]),
                                wxAtoi(SettingsMap[wxT("ID_SLIDER_Snowflakes")+LayerStr+wxT("_Type")]));
    }
    else if (effect == wxT("Snowstorm"))
    {
        buffer.RenderSnowstorm(wxAtoi(SettingsMap[wxT("ID_SLIDER_Snowstorm")+LayerStr+wxT("_Count")]),
                               wxAtoi(SettingsMap[wxT("ID_SLIDER_Snowstorm")+LayerStr+wxT("_Length")]));
    }
    else if (effect == wxT("Spirals"))
    {
        buffer.RenderSpirals(wxAtoi(SettingsMap[wxT("ID_SLIDER_Spirals")+LayerStr+wxT("_Count")]),
                             wxAtoi(SettingsMap[wxT("ID_SLIDER_Spirals")+LayerStr+wxT("_Direction")]),
                             wxAtoi(SettingsMap[wxT("ID_SLIDER_Spirals")+LayerStr+wxT("_Rotation")]),
                             wxAtoi(SettingsMap[wxT("ID_SLIDER_Spirals")+LayerStr+wxT("_Thickness")]),
                             SettingsMap[wxT("ID_CHECKBOX_Spirals")+LayerStr+wxT("_Blend")]==wxT("1"),
                             SettingsMap[wxT("ID_CHECKBOX_Spirals")+LayerStr+wxT("_3D")]==wxT("1"));
    }
    else if (effect == wxT("Text"))
    {
        buffer.RenderText(wxAtoi(SettingsMap[wxT("ID_SLIDER_Text")+LayerStr+wxT("_1_Position")]),
                          SettingsMap[wxT("ID_TEXTCTRL_Text")+LayerStr+wxT("_Line1")],
                          SettingsMap[wxT("ID_TEXTCTRL_Text")+LayerStr+wxT("_1_Font")],
                          EffectDirections.Index(SettingsMap[wxT("ID_CHOICE_Text")+LayerStr+wxT("_1_Dir")]),
                          TextEffects.Index(SettingsMap[wxT("ID_CHOICE_Text")+LayerStr+wxT("_1_Effect")]),
                          TextCountDown.Index(SettingsMap[wxT("ID_CHOICE_Text")+LayerStr+wxT("_1_Count")]),
                          wxAtoi(SettingsMap[wxT("ID_SLIDER_Text")+LayerStr+wxT("_2_Position")]),
                          SettingsMap[wxT("ID_TEXTCTRL_Text")+LayerStr+wxT("_Line2")],
                          SettingsMap[wxT("ID_TEXTCTRL_Text")+LayerStr+wxT("_2_Font")],
                          EffectDirections.Index(SettingsMap[wxT("ID_CHOICE_Text")+LayerStr+wxT("_2_Dir")]),
                          TextEffects.Index(SettingsMap[wxT("ID_CHOICE_Text")+LayerStr+wxT("_2_Effect")]),
                          TextCountDown.Index(SettingsMap[wxT("ID_CHOICE_Text")+LayerStr+wxT("_2_Count")]));
    }
    else if (effect == wxT("Twinkle"))
    {
        buffer.RenderTwinkle(wxAtoi(SettingsMap[wxT("ID_SLIDER_Twinkle")+LayerStr+wxT("_Count")]),
                             wxAtoi(SettingsMap[wxT("ID_SLIDER_Twinkle")+LayerStr+wxT("_Steps")]),
                             SettingsMap[wxT("ID_CHECKBOX_Twinkle")+LayerStr+wxT("_Strobe")]==wxT("1"));
    }
    else if (effect == wxT("Tree"))
    {
        buffer.RenderTree(wxAtoi(SettingsMap[wxT("ID_SLIDER_Tree")+LayerStr+wxT("_Branches")]));
    }
    else if (effect == wxT("Spirograph"))
    {
        buffer.RenderSpirograph(wxAtoi(SettingsMap[wxT("ID_SLIDER_Spirograph")+LayerStr+wxT("_R")]),
                                wxAtoi(SettingsMap[wxT("ID_SLIDER_Spirograph")+LayerStr+wxT("_r")]),
                                wxAtoi(SettingsMap[wxT("ID_SLIDER_Spirograph")+LayerStr+wxT("_d")]),
                                wxAtoi(SettingsMap[wxT("ID_SLIDER_Spirograph")+LayerStr+wxT("_Animate")]));
    }
    else if (effect == wxT("Fireworks"))
    {
        buffer.RenderFireworks(wxAtoi(SettingsMap[wxT("ID_SLIDER_Fireworks")+LayerStr+wxT("_Number_Explosions")]),
                               wxAtoi(SettingsMap[wxT("ID_SLIDER_Fireworks")+LayerStr+wxT("_Count")]),
                               wxAtoi(SettingsMap[wxT("ID_SLIDER_Fireworks")+LayerStr+wxT("_Velocity")]),
                               wxAtoi(SettingsMap[wxT("ID_SLIDER_Fireworks")+LayerStr+wxT("_Fade")]));
    }
    else if (effect == wxT("Piano"))
    {
        buffer.RenderPiano(wxAtoi(SettingsMap[wxT("ID_SLIDER_Piano")+LayerStr+wxT("_Keyboard")]));
    }
    else if (effect == wxT("Circles"))
    {
        //  RenderCircles(int Count,int Steps, bool Strobe)
        /*buffer.RenderCircles(wxAtoi(SettingsMap[wxT("ID_SLIDER_Circles")+LayerStr+wxT("_Count")]),
                             wxAtoi(SettingsMap[wxT("ID_SLIDER_Circles")+LayerStr+wxT("_Size")]),
                             SettingsMap[wxT("ID_CHECKBOX_Circles")+LayerStr+wxT("_Bounce")]==wxT("1"));*/
        //buffer.RenderRadial(buffer.BufferWi/2, buffer.BufferHt/2,4, 4);
    }

}


void xLightsFrame::PlayRgbEffect(int EffectPeriod)
{
    wxString s;
    buffer.Clear();

    // update SparkleFrequency
    int freq=Slider_SparkleFrequency->GetValue();
    if (freq == Slider_SparkleFrequency->GetMax()) freq=0;
    buffer.SetSparkle(freq);

    int brightness=Slider_Brightness->GetValue();
    buffer.SetBrightness(brightness);

    int contrast=Slider_Contrast->GetValue();
    buffer.SetContrast(contrast);

    if (PaletteChanged)
    {
        UpdateBufferPalette();
        ResetEffectStates();
        PaletteChanged=false;
    }
    if (MixTypeChanged)
    {
        s=Choice_LayerMethod->GetStringSelection();
        buffer.SetMixType(s);
        MixTypeChanged=false;
    }

    // render effect 1
    buffer.SetLayer(0,EffectPeriod,Slider_Speed1->GetValue(),ResetEffectState[0]);
    ResetEffectState[0]=false;
    switch (Choicebook1->GetSelection())
    {
    case eff_NONE:
        break;   // none
    case eff_BARS:
        buffer.RenderBars(Slider_Bars1_BarCount->GetValue(),
                          Choice_Bars1_Direction->GetSelection(),
                          CheckBox_Bars1_Highlight->GetValue(),
                          CheckBox_Bars1_3D->GetValue());
        break;
    case eff_BUTTERFLY:
        buffer.RenderButterfly(Choice_Butterfly1_Colors->GetSelection(),
                               Slider_Butterfly1_Style->GetValue(),
                               Slider_Butterfly1_Chunks->GetValue(),
                               Slider_Butterfly1_Skip->GetValue());
        break;
    case eff_COLORWASH:
        buffer.RenderColorWash(CheckBox_ColorWash1_HFade->GetValue(),
                               CheckBox_ColorWash1_VFade->GetValue(),
                               Slider_ColorWash1_Count->GetValue());
        break;
    case eff_FIRE:
        buffer.RenderFire(Slider_Fire1_Height->GetValue(),
                          Slider_Fire1_HueShift->GetValue(),
                          CheckBox_Fire1_GrowFire->GetValue());
        break;
    case eff_GARLANDS:
        buffer.RenderGarlands(Slider_Garlands1_Type->GetValue(),
                              Slider_Garlands1_Spacing->GetValue());
        break;
    case eff_LIFE:
        buffer.RenderLife(Slider_Life1_Count->GetValue(),
                          Slider_Life1_Seed->GetValue());
        break;
    case eff_METEORS:
        buffer.RenderMeteors(Choice_Meteors1_Type->GetSelection(),
                             Slider_Meteors1_Count->GetValue(),
                             Slider_Meteors1_Length->GetValue(),
                             CheckBox_Meteors1_FallUp->GetValue(),
                             Choice_Meteors1_Effect->GetSelection(),
                             Slider_Meteors1_Swirl_Intensity->GetValue());
        break;
    case eff_PICTURES:
        buffer.RenderPictures(Choice_Pictures1_Direction->GetSelection(),
                              TextCtrl_Pictures1_Filename->GetValue(),
                              Slider_Pictures1_GifSpeed->GetValue());
        break;
    case eff_SNOWFLAKES:
        buffer.RenderSnowflakes(Slider_Snowflakes1_Count->GetValue(),
                                Slider_Snowflakes1_Type->GetValue());
        break;
    case eff_SNOWSTORM:
        buffer.RenderSnowstorm(Slider_Snowstorm1_Count->GetValue(),
                               Slider_Snowstorm1_Length->GetValue());
        break;
    case eff_SPIRALS:
        buffer.RenderSpirals(Slider_Spirals1_Count->GetValue(),
                             Slider_Spirals1_Direction->GetValue(),
                             Slider_Spirals1_Rotation->GetValue(),
                             Slider_Spirals1_Thickness->GetValue(),
                             CheckBox_Spirals1_Blend->GetValue(),
                             CheckBox_Spirals1_3D->GetValue());
        break;
    case eff_TEXT:
        buffer.RenderText(Slider_Text1_1_Position->GetValue(),
                          TextCtrl_Text1_Line1->GetValue(),
                          TextCtrl_Text1_1_Font->GetValue(),
                          Choice_Text1_1_Dir->GetSelection(),
                          Choice_Text1_1_Effect->GetSelection(),
                          Choice_Text1_1_Count->GetSelection(),
                          Slider_Text1_2_Position->GetValue(),
                          TextCtrl_Text1_Line2->GetValue(),
                          TextCtrl_Text1_2_Font->GetValue(),
                          Choice_Text1_2_Dir->GetSelection(),
                          Choice_Text1_2_Effect->GetSelection(),
                          Choice_Text1_2_Count->GetSelection());

        break;
    case eff_TWINKLE:
        buffer.RenderTwinkle(Slider_Twinkle1_Count->GetValue(),
                             Slider_Twinkle1_Steps->GetValue(),
                             CheckBox_Twinkle1_Strobe->GetValue());
        break;
    case eff_TREE:
        buffer.RenderTree(Slider_Tree1_Branches->GetValue());
        break;
    case eff_SPIROGRAPH:
        buffer.RenderSpirograph(Slider_Spirograph1_R->GetValue(),
                                Slider_Spirograph1_r->GetValue(),
                                Slider_Spirograph1_d->GetValue(),
                                CheckBox_Spirograph1_Animate->GetValue());
        break;
    case eff_FIREWORKS:
        buffer.RenderFireworks(Slider_Fireworks1_Number_Explosions->GetValue(),
                               Slider_Fireworks1_Count->GetValue(),
                               Slider_Fireworks1_Velocity->GetValue(),
                               Slider_Fireworks1_Fade->GetValue());
        break;
    case eff_PIANO:
        buffer.RenderPiano(Slider_Piano1_Keyboard->GetValue());
        break;

    case eff_CIRCLES:
        /*buffer.RenderCircles(Slider_Circles1_Count->GetValue(),
                             Slider_Circles1_Size->GetValue(),
                             CheckBox_Circles1_Bounce->GetValue());
        buffer.RenderRadial(buffer.BufferWi/2, buffer.BufferHt/2,4, 4);
	*/
    }

    // render effect 2
    buffer.SetLayer(1,EffectPeriod,Slider_Speed2->GetValue(),ResetEffectState[1]);
    ResetEffectState[1]=false;
    switch (Choicebook2->GetSelection())
    {
    case eff_NONE:
        break;   // none
    case eff_BARS:
        buffer.RenderBars(Slider_Bars2_BarCount->GetValue(),
                          Choice_Bars2_Direction->GetSelection(),
                          CheckBox_Bars2_Highlight->GetValue(),
                          CheckBox_Bars2_3D->GetValue());
        break;
    case eff_BUTTERFLY:
        buffer.RenderButterfly(Choice_Butterfly2_Colors->GetSelection(),
                               Slider_Butterfly2_Style->GetValue(),
                               Slider_Butterfly2_Chunks->GetValue(),
                               Slider_Butterfly2_Skip->GetValue());
        break;
    case eff_COLORWASH:
        buffer.RenderColorWash(CheckBox_ColorWash2_HFade->GetValue(),
                               CheckBox_ColorWash2_VFade->GetValue(),
                               Slider_ColorWash2_Count->GetValue());
        break;
    case eff_FIRE:
        buffer.RenderFire(Slider_Fire2_Height->GetValue(),
                          Slider_Fire2_HueShift->GetValue(),
                          CheckBox_Fire2_GrowFire->GetValue());
        break;
    case eff_GARLANDS:
        buffer.RenderGarlands(Slider_Garlands2_Type->GetValue(),
                              Slider_Garlands2_Spacing->GetValue());
        break;
    case eff_LIFE:
        buffer.RenderLife(Slider_Life2_Count->GetValue(),
                          Slider_Life2_Seed->GetValue());
        break;
    case eff_METEORS:
        buffer.RenderMeteors(Choice_Meteors2_Type->GetSelection(),
                             Slider_Meteors2_Count->GetValue(),
                             Slider_Meteors2_Length->GetValue(),
                             CheckBox_Meteors2_FallUp->GetValue(),
                             Choice_Meteors2_Effect->GetSelection(),
                             Slider_Meteors2_Swirl_Intensity->GetValue());
    case eff_PICTURES:
        buffer.RenderPictures(Choice_Pictures2_Direction->GetSelection(),
                              TextCtrl_Pictures2_Filename->GetValue(),
                              Slider_Pictures2_GifSpeed->GetValue());
        break;
    case eff_SNOWFLAKES:
        buffer.RenderSnowflakes(Slider_Snowflakes2_Count->GetValue(),
                                Slider_Snowflakes2_Type->GetValue());
        break;
    case eff_SNOWSTORM:
        buffer.RenderSnowstorm(Slider_Snowstorm2_Count->GetValue(),
                               Slider_Snowstorm2_Length->GetValue());
        break;
    case eff_SPIRALS:
        buffer.RenderSpirals(Slider_Spirals2_Count->GetValue(),
                             Slider_Spirals2_Direction->GetValue(),
                             Slider_Spirals2_Rotation->GetValue(),
                             Slider_Spirals2_Thickness->GetValue(),
                             CheckBox_Spirals2_Blend->GetValue(),
                             CheckBox_Spirals2_3D->GetValue());
        break;
    case eff_TEXT:
        buffer.RenderText(Slider_Text2_1_Position->GetValue(),
                          TextCtrl_Text2_Line1->GetValue(),
                          TextCtrl_Text2_1_Font->GetValue(),
                          Choice_Text2_1_Dir->GetSelection(),
                          Choice_Text2_1_Effect->GetSelection(),
                          Choice_Text2_1_Count->GetSelection(),
                          Slider_Text2_2_Position->GetValue(),
                          TextCtrl_Text2_Line2->GetValue(),
                          TextCtrl_Text2_2_Font->GetValue(),
                          Choice_Text2_2_Dir->GetSelection(),
                          Choice_Text2_2_Effect->GetSelection(),
                          Choice_Text2_2_Count->GetSelection());
        break;
    case eff_TWINKLE:
        buffer.RenderTwinkle(Slider_Twinkle2_Count->GetValue(),
                             Slider_Twinkle2_Steps->GetValue(),
                             CheckBox_Twinkle2_Strobe->GetValue());
        break;
    case eff_TREE:
        buffer.RenderTree(Slider_Tree2_Branches->GetValue());
        break;
    case eff_SPIROGRAPH:
        buffer.RenderSpirograph(Slider_Spirograph2_R->GetValue(),
                                Slider_Spirograph2_r->GetValue(),
                                Slider_Spirograph2_d->GetValue(),
                                CheckBox_Spirograph2_Animate->GetValue());
        break;
    case eff_FIREWORKS:
        buffer.RenderFireworks(Slider_Fireworks2_Number_Explosions->GetValue(),
                               Slider_Fireworks2_Count->GetValue(),
                               Slider_Fireworks2_Velocity->GetValue(),
                               Slider_Fireworks2_Fade->GetValue());
        break;
    case eff_PIANO:
        buffer.RenderPiano(Slider_Piano2_Keyboard->GetValue());
        break;

    case eff_CIRCLES:
        buffer.RenderCircles(Slider_Circles2_Count->GetValue(),
                             Slider_Circles2_Size->GetValue(),
                             CheckBox_Circles2_Bounce->GetValue());
        break;
    }
    buffer.CalcOutput();
    DisplayEffectOnWindow();
    if (CheckBoxLightOutput->IsChecked() && xout)
    {
        size_t NodeCnt=buffer.GetNodeCount();
        for(size_t i=0; i<NodeCnt; i++)
        {
            xout->SetIntensity((buffer.Nodes[i].getChanNum(0)),buffer.Nodes[i].GetChannelColorVal(0));
            xout->SetIntensity((buffer.Nodes[i].getChanNum(1)),buffer.Nodes[i].GetChannelColorVal(1));
            xout->SetIntensity((buffer.Nodes[i].getChanNum(2)),buffer.Nodes[i].GetChannelColorVal(2));
        }
    }
}

void xLightsFrame::UpdateRgbPlaybackStatus(int seconds, const wxString& seqtype)
{
    int m=seconds/60;
    int s=seconds%60;
    StatusBar1->SetStatusText(wxString::Format(wxT("Playback: RGB ")+seqtype+wxT(" sequence %d:%02d"),m,s));
}

void xLightsFrame::TimerRgbSeq(long msec)
{
    long StartTime;
    int EffectPeriod;
    static int s_period=0;
    int rowcnt=Grid1->GetNumberRows();
    switch (SeqPlayerState)
    {
    case PLAYING_EFFECT:
        PlayRgbEffect(s_period);
        s_period++;
        break;
    case STARTING_SEQ_ANIM:
        ResetTimer(PLAYING_SEQ_ANIM, GetGridStartTimeMSec(NextGridRowToPlay));
        break;
    case PLAYING_SEQ_ANIM:
        if (xout && !xout->TxEmpty())
        {
            TxOverflowCnt++;
            break;
        }
        EffectPeriod = msec / XTIMER_INTERVAL;
        if (EffectPeriod >= SeqNumPeriods)
        {
            // sequence has finished
            if (xout) xout->alloff();
            ResetTimer(NO_SEQ);
            EnableSequenceControls(true);
        }
        else
        {
            if (NextGridRowToPlay < rowcnt && msec >= GetGridStartTimeMSec(NextGridRowToPlay))
            {
                // start next effect
                Grid1->MakeCellVisible(NextGridRowToPlay,SeqPlayColumn);
                Grid1->SelectBlock(NextGridRowToPlay,SeqPlayColumn,NextGridRowToPlay,SeqPlayColumn);
                SetEffectControls(Grid1->GetCellValue(NextGridRowToPlay,SeqPlayColumn));
                NextGridRowToPlay++;
            }
            PlayRgbEffect(EffectPeriod);
            if (EffectPeriod % 20 == 0) UpdateRgbPlaybackStatus(EffectPeriod/20,wxT("animation"));
        }
        break;
    case STARTING_SEQ:
        StartTime=GetGridStartTimeMSec(NextGridRowToPlay);
        if(PlayerDlg->MediaCtrl->GetState() == wxMEDIASTATE_PLAYING)
        {
            ResetTimer(PLAYING_SEQ, StartTime);
        }
        else
        {
            PlayerDlg->MediaCtrl->Seek(StartTime);
            PlayerDlg->MediaCtrl->Play();
        }
        break;
    case PLAYING_SEQ:
        if (xout && !xout->TxEmpty())
        {
            TxOverflowCnt++;
            break;
        }
        msec = PlayerDlg->MediaCtrl->Tell();
        EffectPeriod = msec / XTIMER_INTERVAL;
        if (EffectPeriod >= SeqNumPeriods || PlayerDlg->MediaCtrl->GetState() != wxMEDIASTATE_PLAYING)
        {
            // sequence has finished
            PlayerDlg->MediaCtrl->Stop();
            if (xout) xout->alloff();
            ResetTimer(NO_SEQ);
            EnableSequenceControls(true);
        }
        else
        {
            if (NextGridRowToPlay < rowcnt && msec >= GetGridStartTimeMSec(NextGridRowToPlay))
            {
                // start next effect
                Grid1->MakeCellVisible(NextGridRowToPlay,SeqPlayColumn);
                Grid1->SelectBlock(NextGridRowToPlay,SeqPlayColumn,NextGridRowToPlay,SeqPlayColumn);
                SetEffectControls(Grid1->GetCellValue(NextGridRowToPlay,SeqPlayColumn));
                NextGridRowToPlay++;
            }
            PlayRgbEffect(EffectPeriod);
            //TextCtrlLog->AppendText(wxString::Format(wxT("msec=%ld, period=%d\n"),msec,EffectPeriod));
            if (EffectPeriod % 20 == 0) UpdateRgbPlaybackStatus(EffectPeriod/20,wxT("music"));
        }
        break;
    }
}

void xLightsFrame::OpenPaletteDialog(const wxString& id1, const wxString& id2, wxSizer* PrimarySizer,wxSizer* SecondarySizer)
{
    PaletteMgmtDialog dialog(this);
    dialog.initialize(id1,id2,PalettesNode,PrimarySizer,SecondarySizer);
    dialog.ShowModal();
    SaveEffectsFile();
}

void xLightsFrame::OnButton_Palette1Click(wxCommandEvent& event)
{
    OpenPaletteDialog(wxT("1"),wxT("2"),FlexGridSizer_Palette1,FlexGridSizer_Palette2);
}

void xLightsFrame::OnButton_Palette2Click(wxCommandEvent& event)
{
    OpenPaletteDialog(wxT("2"),wxT("1"),FlexGridSizer_Palette2,FlexGridSizer_Palette1);
}

void xLightsFrame::UpdateFont(wxTextCtrl* FontCtrl)
{
    wxFont oldfont,newfont;
    oldfont.SetNativeFontInfoUserDesc(FontCtrl->GetValue());
    newfont=wxGetFontFromUser(this,oldfont);
    if (newfont.IsOk())
    {
        wxString FontDesc=newfont.GetNativeFontInfoUserDesc();
        FontDesc.Replace(wxT(" unknown-90"),wxT(""));
        FontCtrl->SetValue(FontDesc);
    }
}

void xLightsFrame::OnButton_Text1_1_FontClick(wxCommandEvent& event)
{
    UpdateFont(TextCtrl_Text1_1_Font);
}


void xLightsFrame::OnButton_Text1_2_FontClick(wxCommandEvent& event)
{
    UpdateFont(TextCtrl_Text1_2_Font);
}

void xLightsFrame::OnButton_Text2_1_FontClick(wxCommandEvent& event)
{
    UpdateFont(TextCtrl_Text2_1_Font);
}


void xLightsFrame::OnButton_Text2_2_FontClick(wxCommandEvent& event)
{
    UpdateFont(TextCtrl_Text2_2_Font);
}

void xLightsFrame::OnButton_Pictures1_FilenameClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Image", CurrentDir, "", "", wxImage::GetImageExtWildcard(), wxFD_OPEN );
    if (!filename.IsEmpty()) TextCtrl_Pictures1_Filename->SetValue(filename);
}

void xLightsFrame::OnButton_Pictures2_FilenameClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Image", CurrentDir, "", "", wxImage::GetImageExtWildcard(), wxFD_OPEN );
    if (!filename.IsEmpty()) TextCtrl_Pictures2_Filename->SetValue(filename);
}

void xLightsFrame::DisplayXlightsFilename(const wxString& filename)
{
    xlightsFilename=filename;
    StaticTextSequenceFileName->SetLabel(filename);
}

void xLightsFrame::GetModelNames(wxArrayString& a)
{
    wxString name;
    for(wxXmlNode* e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == wxT("model"))
        {
            name=e->GetAttribute(wxT("name"));
            if (!name.IsEmpty())
            {
                a.Add(name);
            }
        }
    }
}

void xLightsFrame::GetGridColumnLabels(wxArrayString& a)
{
    int n=Grid1->GetNumberCols();
    for(int i=2; i < n; i++)
    {
        a.Add(Grid1->GetColLabelValue(i));
    }
}

void xLightsFrame::ChooseModelsForSequence()
{
    if (SeqData.size() == 0)
    {
        wxMessageBox(wxT("You must open a sequence first!"), wxT("Error"));
        return;
    }
    SeqParmsDialog dialog(this);
    wxXmlNode* e;
    wxString name;
    int idx;

    // get list of models that are already in the grid
    wxArrayString labels;
    GetGridColumnLabels(labels);

    // populate the listbox with all models
    for(e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == wxT("model"))
        {
            name=e->GetAttribute(wxT("name"));
            // allow only models where MyDisplay is set?
            //if (!name.IsEmpty() && e->GetAttribute(wxT("MyDisplay"),wxT("0"))==wxT("1")) {
            if (!name.IsEmpty())
            {
                dialog.CheckListBox1->Append(name);
                idx=dialog.CheckListBox1->FindString(name);
                if (idx != wxNOT_FOUND && labels.Index(name,false) != wxNOT_FOUND)
                {
                    dialog.CheckListBox1->Check(idx,true);
                }
            }
        }
    }


    dialog.StaticText_Filename->SetLabel(xlightsFilename);
    if (dialog.ShowModal() != wxID_OK) return;

    // add checked models to grid

    size_t cnt = dialog.CheckListBox1->GetCount();
    for (size_t i=0; i < cnt; i++)
    {
        if (dialog.CheckListBox1->IsChecked(i))
        {
            name=dialog.CheckListBox1->GetString(i);
            idx=labels.Index(name,false);
            if (idx == wxNOT_FOUND)
            {
                // newly checked item, so add it
                Grid1->AppendCols();
                int colnum=Grid1->GetNumberCols()-1;
                Grid1->SetColLabelValue(colnum,name);

                for(int j = 0; j < Grid1->GetNumberRows(); j++)
                {
                    Grid1->SetCellOverflow(j,colnum, false);
                    Grid1->SetReadOnly(j,colnum,false);
                }
            }
            else
            {
                // item already exists
                labels[idx].Clear();
            }
        }
    }

    // any non-empty entries in labels represent items that were unchecked - so delete them from grid
    for(idx=labels.GetCount()-1; idx >= 0; idx--)
    {
        if (!labels[idx].IsEmpty())
        {
            Grid1->DeleteCols(idx+2);
        }
    }
    EnableSequenceControls(true);
}


void xLightsFrame::OnButton_ChannelMapClick(wxCommandEvent& event)
{
    if (SeqData.size() == 0)
    {
        wxMessageBox(wxT("You must open a sequence first!"), wxT("Error"));
        return;
    }
    ChannelMapDialog dialog(this);
    dialog.SpinCtrlBaseChannel->SetValue(SeqBaseChannel);
    dialog.CheckBox_EnableBasic->SetValue(SeqChanCtrlBasic);
    dialog.CheckBox_EnableColor->SetValue(SeqChanCtrlColor);
    dialog.SetNetInfo(&NetInfo);
    if (dialog.ShowModal() != wxID_OK) return;
    SeqBaseChannel=dialog.SpinCtrlBaseChannel->GetValue();
    SeqChanCtrlBasic=dialog.CheckBox_EnableBasic->GetValue();
    SeqChanCtrlColor=dialog.CheckBox_EnableColor->GetValue();
}

#include <wx/textfile.h>
#include <wx/string.h>
#include <wx/tokenzr.h>

wxString insert_missing(wxString str,wxString missing_array)
{
    int pos;
    wxStringTokenizer tkz(missing_array, wxT("|"));
    wxString replacement;
    wxString token1 = tkz.GetNextToken(); // get first two dummy tokens out
    wxString token2 = tkz.GetNextToken();
    while ( tkz.HasMoreTokens() )
    {
        token1 = tkz.GetNextToken();
        token2 = tkz.GetNextToken();
        pos=str.find(token1,0);
        replacement = wxT(",") + token2 + wxT("</td>");
        if(pos<=0) str.Replace(wxT("</td>"),replacement);

    }
    return str;
}

void fix_version_differences(wxString file)
{
    wxString        str,fileout;
    wxTextFile      tfile;
    wxFile f;
    bool modified=false;
    fileout = file + ".out";
// open the file
    wxString missing = "xdummy|xdummy";
    wxString Text1   = "xdummy|xdummy";
    wxString Text2   = "xdummy|xdummy";
    wxString Meteors1 = "xdummy|xdummy";
    wxString Meteors2 = "xdummy|xdummy";
    wxString Fire1   = "xdummy|xdummy";
    wxString Fire2   = "xdummy|xdummy";
    //
    //
    //  list all new tags that might have occured in previous versions
    //  list is pair. first token is what to search for, if it is missing, then put in 2nd token into xml string
    //
    missing = missing + wxT("|ID_SLIDER_Brightness|ID_SLIDER_Brightness=100");
    missing = missing + wxT("|ID_SLIDER_Contrast|ID_SLIDER_Contrast=0");
    //
    Meteors1 = Meteors1 + wxT("|ID_CHECKBOX_Meteors1_FallUp|ID_CHECKBOX_Meteors1_FallUp=0");
    Meteors2 = Meteors2 + wxT("|ID_CHECKBOX_Meteors2_FallUp|ID_CHECKBOX_Meteors2_FallUp=0");
    Meteors1 = Meteors1 + wxT("|ID_CHOICE_Meteors1_Effect|ID_CHOICE_Meteors1_Effect=Meteor");
    Meteors2 = Meteors2 + wxT("|ID_CHOICE_Meteors2_Effect|ID_CHOICE_Meteors2_Effect=Meteor");
    Meteors1 = Meteors1 + wxT("|ID_SLIDER_Meteors1_Swirl_Intensity|ID_SLIDER_Meteors1_Swirl_Intensity=10");
    Meteors2 = Meteors2 + wxT("|ID_SLIDER_Meteors2_Swirl_Intensity|ID_SLIDER_Meteors2_Swirl_Intensity=10");

    Fire1 = Fire1 + wxT("|ID_SLIDER_Fire1_HueShift|ID_SLIDER_Fire1_HueShift=0");
    Fire2 = Fire2 + wxT("|ID_SLIDER_Fire2_HueShift|ID_SLIDER_Fire2_HueShift=0");
    Fire1 = Fire1 + wxT("|ID_CHECKBOX_Fire1_GrowFire|ID_CHECKBOX_Fire1_GrowFire=0");
    Fire2 = Fire2 + wxT("|ID_CHECKBOX_Fire2_GrowFire|ID_CHECKBOX_Fire2_GrowFire=0");


    // Lots of variables to check for  text effect
    Text1 = Text1 + wxT("|ID_TEXTCTRL_Text1_1_Font|ID_TEXTCTRL_Text1_1_Font=");
    Text1 = Text1 + wxT("|ID_CHOICE_Text1_1_Dir|ID_CHOICE_Text1_1_Dir=left");
    Text1 = Text1 + wxT("|ID_SLIDER_Text1_1_Position|ID_SLIDER_Text1_1_Position=50");
    Text1 = Text1 + wxT("|ID_SLIDER_Text1_1_TextRotation|ID_SLIDER_Text1_1_TextRotation=0");
    Text1 = Text1 + wxT("|ID_CHECKBOX_Text1_COUNTDOWN1|ID_CHECKBOX_Text1_COUNTDOWN1=0");
    Text1 = Text1 + wxT("|ID_TEXTCTRL_Text1_2_Font|ID_TEXTCTRL_Text1_2_Font=");
    Text1 = Text1 + wxT("|ID_CHOICE_Text1_2_Dir|ID_CHOICE_Text1_2_Dir=left");
    Text1 = Text1 + wxT("|ID_SLIDER_Text1_2_Position|ID_SLIDER_Text1_2_Position=50");
    Text1 = Text1 + wxT("|ID_SLIDER_Text1_2_TextRotation|ID_SLIDER_Text1_2_TextRotation=0");
    Text1 = Text1 + wxT("|ID_CHECKBOX_Text1_COUNTDOWN2|ID_CHECKBOX_Text1_COUNTDOWN2=0");

    //
    Text2 = Text2 + wxT("|ID_TEXTCTRL_Text2_1_Font|ID_TEXTCTRL_Text2_1_Font=");
    Text2 = Text2 + wxT("|ID_CHOICE_Text2_1_Dir|ID_CHOICE_Text2_1_Dir=left");
    Text2 = Text2 + wxT("|ID_SLIDER_Text2_1_Position|ID_SLIDER_Text2_1_Position=50");
    Text2 = Text2 + wxT("|ID_SLIDER_Text2_1_TextRotation|ID_SLIDER_Text2_1_TextRotation=0");
    Text2 = Text2 + wxT("|ID_CHECKBOX_Text2_COUNTDOWN1|ID_CHECKBOX_Text2_COUNTDOWN1=0");
    Text2 = Text2 + wxT("|ID_TEXTCTRL_Text2_2_Font|ID_TEXTCTRL_Text2_2_Font=");
    Text2 = Text2 + wxT("|ID_CHOICE_Text2_2_Dir|ID_CHOICE_Text2_2_Dir=left");
    Text2 = Text2 + wxT("|ID_SLIDER_Text2_2_Position|ID_SLIDER_Text2_2_Position=50");
    Text2 = Text2 + wxT("|ID_SLIDER_Text2_2_TextRotation|ID_SLIDER_Text2_2_TextRotation=0");
    Text2 = Text2 + wxT("|ID_CHECKBOX_Text2_COUNTDOWN2|ID_CHECKBOX_Text2_COUNTDOWN2=0");


    if (!f.Create(fileout,true))
    {
        return;
    }

    tfile.Open(file); // open input file
// read the first line
    str =  tfile.GetFirstLine() + "\n";


    f.Write(str);
    int p,pos,pos_ID_SLIDER_Brightness,pos_ID_SLIDER_Contrast,pos_SLIDER_Slider,pos_ID_TEXTCTRL4;

// read all lines one by one
// until the end of the file
    while(!tfile.Eof())
    {
        str = tfile.GetNextLine();

        pos=str.find("ID_SLIDER",0); // is this line a Effect?
        if(pos>0) // are we on the xml line containg the effect?
        {
            //  Yes

            //  do we have the bad SILDER_slider token?
            pos_SLIDER_Slider=str.find("SLIDER_Slider",0);
            if(pos_SLIDER_Slider>0) // if we have SLIDER_Slider bad text,
            {
                modified=true;  // yes,fix it
                str.Replace(wxT("SLIDER_Slider"),wxT("SLIDER"));
            }

// do we have the old text1 font token?
            pos_ID_TEXTCTRL4=str.find("ID_TEXTCTRL4",0);
            if(pos_ID_TEXTCTRL4>0) // if we have ID_TEXTCTRL4 bad text,
            {
                modified=true;  // yes,fix it
                str.Replace(wxT("ID_TEXTCTRL4"),wxT("ID_TEXTCTRL_Text1_1_Font"));
            }

//  now look to fill in any missing tokens
            p=str.find("ID_SLIDER",0);
            if(p>0) // Look for lines that should have brightness and contrast, in other words all
            {
                modified=true;
                str=insert_missing(str,missing);
            }

            p=str.find("ID_TEXTCTRL_Text1_Line1",0);
            if(p>0) // Is this a text 1 line?
            {
                modified=true;
                str=insert_missing(str,Text1);
            }
            p=str.find("ID_TEXTCTRL_Text2_Line1",0);
            if(p>0) // is this a text 2 line?
            {
                modified=true;
                str=insert_missing(str,Text2);
            }

            p=str.find("ID_CHOICE_Meteors1",0);
            if(p>0) // is there a meteors 1 effect on this line?
            {
                modified=true;
                str=insert_missing(str,Meteors1); // fix any missing values
            }
            p=str.find("ID_CHOICE_Meteors2",0);
            if(p>0) // is there a meteors 1 effect on this line?
            {
                modified=true;
                str=insert_missing(str,Meteors2);
            }

            p=str.find("ID_CHOICE_Fire11",0);
            if(p>0) // is there a meteors 1 effect on this line?
            {
                modified=true;
                str=insert_missing(str,Fire1); // fix any missing values
            }
            p=str.find("ID_CHOICE_Fire2",0);
            if(p>0) // is there a meteors 1 effect on this line?
            {
                modified=true;
                str=insert_missing(str,Fire2); // fix any missing values
            }
        }
        str = str + "\n";
        f.Write(str); // placeholder, do whatever you want with the string
    }
    tfile.Close();
    f.Close();
    if(modified) wxCopyFile(fileout,file,true); // if we modified the file, copy over it
}

void xLightsFrame::ProcessAudacityTimingFile(const wxString& filename)
{
    wxTextFile f;
    wxString line;

    double timeMark;
    int spacePos, r;

    if (!f.Open(filename.c_str()))
    {
        //Add error dialog if open file failed
        return;
    }
    for(r=0, line = f.GetFirstLine(); !f.Eof(); line = f.GetNextLine(), r++)
    {
        wxStringTokenizer tkz(line, wxT("\t"));
        wxString token = tkz.GetNextToken();

        Grid1->AppendRows();
        Grid1->SetCellValue(r,0,token);

    }
}

void xLightsFrame::ImportAudacityTimings()
{
    wxFileDialog* OpenDialog = new wxFileDialog(
        this, _("Choose Audacity timing file"), CurrentDir, wxEmptyString,
        _("Text files (*.txt)|*.txt"),		wxFD_OPEN, wxDefaultPosition);
    wxString fName;

    if (OpenDialog->ShowModal() == wxID_OK)
    {
        fName =	OpenDialog->GetPath();
        ProcessAudacityTimingFile(fName);
    }
    else
    {

    }

    // Clean up after ourselves
    OpenDialog->Destroy();
}

void xLightsFrame::ProcessxLightsXMLTimingsFile(const wxString& filename)
{

}

void xLightsFrame::ImportxLightsXMLTimings()
{
    wxFileDialog* OpenDialog = new wxFileDialog(
        this, _("Choose Audacity timing file"), CurrentDir, wxEmptyString,
        _("Text files (*.xml)|*.xml"),		wxFD_OPEN, wxDefaultPosition);
    wxString fName;

    if (OpenDialog->ShowModal() == wxID_OK)
    {
        fName =	OpenDialog->GetPath();
        SeqLoadXlightsFile(fName);
    }
    else
    {

    }

    // Clean up after ourselves
    OpenDialog->Destroy();

}
void xLightsFrame::SeqLoadXlightsXSEQ(const wxString& filename)
{
    // read xlights file
    ReadXlightsFile(filename);
    DisplayXlightsFilename(filename);
    SeqBaseChannel=1;
    SeqChanCtrlBasic=false;
    SeqChanCtrlColor=false;
}

void xLightsFrame::SeqLoadXlightsFile(const wxString& filename)
{
    // read xml sequence info
    wxFileName FileObj(filename);
    FileObj.SetExt("xml");
    SeqXmlFileName=FileObj.GetFullPath();
    int gridCol;
    if (!FileObj.FileExists())
    {
        ChooseModelsForSequence();
        return;
    }

    // read xml
    //  first fix any version specific changes
    fix_version_differences(SeqXmlFileName);


    wxXmlDocument doc;
    if (!doc.Load(SeqXmlFileName))
    {
        wxMessageBox(_("Error loading: ")+SeqXmlFileName);
        return;
    }
    wxXmlNode* root=doc.GetRoot();
    wxString tempstr=root->GetAttribute(wxT("BaseChannel"), wxT("1"));
    tempstr.ToLong(&SeqBaseChannel);
    tempstr=root->GetAttribute(wxT("ChanCtrlBasic"), wxT("0"));
    SeqChanCtrlBasic=tempstr!=wxT("0");
    tempstr=root->GetAttribute(wxT("ChanCtrlColor"), wxT("0"));
    SeqChanCtrlColor=tempstr!=wxT("0");

    wxXmlNode *tr, *td;
    wxString ColName;
    wxArrayInt DeleteCols;
    wxArrayString ModelNames;
    GetModelNames(ModelNames);
    SeqElementMismatchDialog dialog(this);
    dialog.ChoiceModels->Set(ModelNames);
    if (ModelNames.Count() > 0) dialog.ChoiceModels->SetSelection(0);
    int r,c; // row 0=heading, >=1 are data rows
    for(tr=root->GetChildren(), r=0; tr!=NULL; tr=tr->GetNext(), r++ )
    {
        if (tr->GetName() != wxT("tr")) continue;
        if (r > 0)
        {
            Grid1->AppendRows();
        }
        for(td=tr->GetChildren(), c=0, gridCol=0; td!=NULL; td=td->GetNext(), c++ )
        {
            if (td->GetName() != wxT("td")) continue;
            if (r==0)
            {
                if (c >= 2)
                {
                    ColName=td->GetNodeContent();
                    if (ModelNames.Index(ColName) == wxNOT_FOUND)
                    {
                        dialog.StaticTextMessage->SetLabel(wxT("Element '")+ColName+wxT("'\ndoes not exist in your list of models"));
                        dialog.Fit();
                        dialog.ShowModal();
                        if (dialog.RadioButtonAdd->GetValue())
                        {
                        }
                        else if (dialog.RadioButtonDelete->GetValue())
                        {
                            DeleteCols.Add(c);
                        }
                        else
                        {
                            // rename
                            Grid1->AppendCols();
                            Grid1->SetColLabelValue(Grid1->GetNumberCols()-1, dialog.ChoiceModels->GetStringSelection());
                        }
                    }
                    else
                    {
                        Grid1->AppendCols();
                        Grid1->SetColLabelValue(Grid1->GetNumberCols()-1, ColName);
                    }
                }
            }
            else if (DeleteCols.Index(c) == wxNOT_FOUND)
            {

                Grid1->SetCellValue(r-1,gridCol,td->GetNodeContent());
                gridCol++; //c does not work here since it is following the columns in the input file not the columns in the grid
                Grid1->SetCellOverflow(r-1,gridCol, false);
                Grid1->SetReadOnly(r-1,gridCol,false);

            }
        }
    }
    EnableSequenceControls(true);
}

void xLightsFrame::OnBitmapButtonOpenSeqClick(wxCommandEvent& event)
{
    wxArrayString SeqFiles,MediaFiles;
    wxDir::GetAllFiles(CurrentDir,&SeqFiles,"*.xseq");

    // get list of media files
    wxFileName oName;
    wxString filename;
    wxString nullString;
    char filetype;
    oName.AssignDir( CurrentDir );
    wxDir dir(CurrentDir);
    nullString.Clear();
    bool cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);

    if (UnsavedChanges && wxNO == wxMessageBox("Sequence changes will be lost.  Do you wish to continue?",
            "Sequence Changed Confirmation", wxICON_QUESTION | wxYES_NO))
    {
        return;
    }

    while ( cont )
    {
        oName.SetFullName(filename);
        filetype=ExtType(oName.GetExt());
        if (filetype=='a' || filetype=='v') MediaFiles.Add(oName.GetFullPath());
        cont = dir.GetNext(&filename);
    }

    // populate dialog
    SeqOpenDialog dialog(this);
    if (SeqFiles.Count() > 0)
    {
        dialog.ChoiceSeqFiles->Set(SeqFiles);
        dialog.ChoiceSeqFiles->SetSelection(0);
    }
    else
    {
        dialog.RadioButtonXlights->Enable(false);
        dialog.ChoiceSeqFiles->Enable(false);
        dialog.RadioBoxTimingChoice->Enable();
        dialog.RadioButtonNewMusic->SetValue(true);
    }
    if (MediaFiles.Count() > 0)
    {
        dialog.ChoiceMediaFiles->Set(MediaFiles);
        dialog.ChoiceMediaFiles->SetSelection(0);
    }
    else
    {
        dialog.RadioButtonNewMusic->Enable(false);
        dialog.ChoiceMediaFiles->Enable(false);
        dialog.RadioBoxTimingChoice->Disable();
        dialog.RadioButtonNewAnim->SetValue(true);
    }
    dialog.Fit();
    if (dialog.ShowModal() != wxID_OK) return;  // user pressed cancel

    // reset grid
    int n;
    n=Grid1->GetNumberCols();
    if (n > 2) Grid1->DeleteCols(2, n-2);
    n=Grid1->GetNumberRows();
    if (n > 0) Grid1->DeleteRows(0, n);
    mediaFilename.Clear();

    long duration;
    if (dialog.RadioButtonXlights->GetValue())
    {
        SeqLoadXlightsXSEQ(dialog.ChoiceSeqFiles->GetStringSelection());
        SeqLoadXlightsFile(dialog.ChoiceSeqFiles->GetStringSelection());
        return;
    }
    else if (dialog.RadioButtonNewMusic->GetValue())
    {
        UnsavedChanges = true;

        // determine media file length
        mediaFilename=dialog.ChoiceMediaFiles->GetStringSelection();

        DisplayXlightsFilename(nullString);

        if (!PlayerDlg->Play(mediaFilename))
        {
            wxMessageBox(wxT("Unable to load:\n")+mediaFilename,wxT("ERROR"));
            return;
        }
        for (int cnt=0; cnt < 50; cnt++)
        {
            duration=PlayerDlg->MediaCtrl->Length();  // milliseconds
            if (duration > 0) break;
            wxYield();
            wxMilliSleep(100);
        }
        PlayerDlg->MediaCtrl->Stop();
        if (duration <= 0)
        {
            wxMessageBox(wxT("Unable to determine the length of:\n")+mediaFilename,wxT("ERROR"));
            return;
        }
        switch (dialog.RadioBoxTimingChoice->GetSelection())
        {
        case 0:
            // No Timing mark import selected
            break;
        case 1:
            // Audacity File import
            ImportAudacityTimings();
            break;
        case 2:
            // Xlights XML timing import
            ImportxLightsXMLTimings();
            break;
        }
        //Clear xlights filename

    }
    else if (dialog.RadioButtonNewAnim->GetValue())
    {
        UnsavedChanges = true;
        duration=dialog.SpinCtrlDuration->GetValue();  // seconds
        DisplayXlightsFilename(nullString);
        if (duration <= 0)
        {
            wxMessageBox(wxT("Invalid value for duration"),wxT("ERROR"));
            return;
        }
        duration*=1000;  // convert to milliseconds
    }

    SeqData.clear();
    SeqNumChannels=NetInfo.GetTotChannels();
    int interval=Timer1.GetInterval();
    SeqNumPeriods=duration / interval;
    SeqDataLen=SeqNumPeriods * SeqNumChannels;
    SeqData.resize(SeqDataLen,0);
    int nSeconds=duration/1000;
    int nMinutes=nSeconds/60;
    nSeconds%=60;
    wxMessageBox(wxString::Format(wxT("Created empty sequence:\nChannels: %ld\nPeriods: %ld\nEach period is: %d msec\nTotal time: %d:%02d"),
                                  SeqNumChannels,SeqNumPeriods,interval,nMinutes,nSeconds));
}

void xLightsFrame::RenderGridToSeqData()
{
    MapStringString SettingsMap;
    wxString ColName,msg;
    long msec;
    size_t ChannelNum, NodeCnt;
    int rowcnt=Grid1->GetNumberRows();
    int colcnt=Grid1->GetNumberCols();
    wxXmlNode *ModelNode;
    LoadEffectFromString(wxT("None,None,Effect 1"), SettingsMap);
    for (int c=2; c<colcnt; c++) //c iterates through the columns of Grid1 retriving the effects for each model in the sequence.
    {
        ColName=Grid1->GetColLabelValue(c);
        ModelNode=GetModelNode(ColName);
        if (!ModelNode) continue;
        buffer.InitBuffer(ModelNode);
        if (!buffer.MyDisplay) continue;
        NodeCnt=buffer.GetNodeCount();
        ChannelNum=buffer.StartChannel-1+NodeCnt*3; // last channel
        if (ChannelNum > SeqNumChannels)
        {
            // need to add more channels to existing sequence
            msg=wxString::Format(wxT("Increasing sequence channel count from %ld to %d"),SeqNumChannels,ChannelNum);
            if (ChannelNum > NetInfo.GetTotChannels())
            {
                msg+=wxT("\n\nEither your model is incorrect or the networks you have defined on the Setup Tab are incorrect.\n\nYou should fix this before doing any more exports!");
            }
            wxMessageBox(msg);
            SeqNumChannels=ChannelNum;
            SeqDataLen=SeqNumChannels*SeqNumPeriods;
            SeqData.resize(SeqDataLen,0);
        }
        NextGridRowToPlay=0;
        for (int p=0; p<SeqNumPeriods; p++)
        {
            msec=p * XTIMER_INTERVAL;
            buffer.Clear();
            if (NextGridRowToPlay < rowcnt && msec >= GetGridStartTimeMSec(NextGridRowToPlay))
            {
                // start next effect
                wxYield();
                StatusBar1->SetStatusText(_(wxString::Format(wxT("Saving row %ld"),NextGridRowToPlay+1)));

                LoadEffectFromString(Grid1->GetCellValue(NextGridRowToPlay,c), SettingsMap);
                // TextCtrlLog->AppendText(wxT("effect")+LayerStr+wxT("=")+effect+wxT(", speed=")+SpeedStr+wxT("\n"));
                UpdateBufferPaletteFromMap(1,SettingsMap);
                UpdateBufferPaletteFromMap(2,SettingsMap);
                buffer.SetMixType(SettingsMap["LayerMethod"]);
                ResetEffectStates();
                // update SparkleFrequency
                int freq=wxAtoi(SettingsMap["ID_SLIDER_SparkleFrequency"]);
                if (freq == Slider_SparkleFrequency->GetMax()) freq=0;
                buffer.SetSparkle(freq);

                int brightness=wxAtoi(SettingsMap["ID_SLIDER_Brightness"]);
                buffer.SetBrightness(brightness);

                int contrast=wxAtoi(SettingsMap["ID_SLIDER_Contrast"]);
                buffer.SetContrast(contrast);

                NextGridRowToPlay++;
            } //  if (NextGridRowToPlay < rowcnt && msec >= GetGridStartTimeMSec(NextGridRowToPlay))
            RenderEffectFromString(0, p, SettingsMap);
            RenderEffectFromString(1, p, SettingsMap);
            buffer.CalcOutput();
            // update SeqData with contents of buffer
            for(int n=0; n<NodeCnt; n++)
            {
                SeqData[(buffer.Nodes[n].getChanNum(0))*SeqNumPeriods+p]=buffer.Nodes[n].GetChannelColorVal(0);

                SeqData[(buffer.Nodes[n].getChanNum(1))*SeqNumPeriods+p]=buffer.Nodes[n].GetChannelColorVal(1);

                SeqData[(buffer.Nodes[n].getChanNum(2))*SeqNumPeriods+p]=buffer.Nodes[n].GetChannelColorVal(2);

            } // for(int n=0; n<NodeCnt; n++)
        } //for (int p=0; p<SeqNumPeriods; p++)
    }
}

void xLightsFrame::OnBitmapButtonSaveSeqClick(wxCommandEvent& event)
{
    wxString NewFilename;
    bool ok;
    if (SeqData.size() == 0)
    {
        wxMessageBox(wxT("You must open a sequence first!"), wxT("Error"));
        return;
    }

    // save Grid1 to xml
    int rowcnt=Grid1->GetNumberRows();
    int colcnt=Grid1->GetNumberCols();
    if (colcnt <= 2)
    {
        wxMessageBox(wxT("No models in the grid!"), wxT("Warning"));
    }
    if (rowcnt == 0)
    {
        wxMessageBox(wxT("No grid rows to save!"), wxT("Warning"));
    }
    if (xlightsFilename.IsEmpty())
    {
        wxTextEntryDialog dialog(this,wxT("Enter a name for the sequence:"),wxT("Save As"));
        do
        {
            if (dialog.ShowModal() != wxID_OK) return;
            // validate inputs
            NewFilename=dialog.GetValue();
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
        oName.SetPath( CurrentDir );
        oName.SetExt(_(XLIGHTS_SEQUENCE_EXT));
        DisplayXlightsFilename(oName.GetFullPath());

        oName.SetExt("xml");
        SeqXmlFileName=oName.GetFullPath();
    }

    StatusBar1->SetStatusText(_("Saving ")+xlightsFilename);
    int r,c;
    wxXmlDocument doc;
    wxXmlNode *tr, *td;
    wxXmlNode* root = new wxXmlNode( wxXML_ELEMENT_NODE, wxT("xsequence") );
    doc.SetRoot( root );
    root->AddAttribute(wxT("BaseChannel"), wxString::Format(wxT("%ld"),SeqBaseChannel));
    root->AddAttribute(wxT("ChanCtrlBasic"), SeqChanCtrlBasic ? wxT("1") : wxT("0"));
    root->AddAttribute(wxT("ChanCtrlColor"), SeqChanCtrlColor ? wxT("1") : wxT("0"));

    // new items get added to the TOP of the xml structure, so add everything in reverse order

    // save data rows
    for (r=rowcnt-1; r>=0; r--)
    {
        tr=new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("tr"));
        for (c=colcnt-1; c>=0; c--)
        {
            td=new wxXmlNode(tr, wxXML_ELEMENT_NODE, wxT("td"));
            td->AddChild(new wxXmlNode(td, wxXML_TEXT_NODE, wxEmptyString, Grid1->GetCellValue(r,c)));
        }
    }

    // save labels to first row
    tr=new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("tr"));
    for (c=colcnt-1; c>=0; c--)
    {
        td=new wxXmlNode(tr, wxXML_ELEMENT_NODE, wxT("td"));
        td->AddChild(new wxXmlNode(td, wxXML_TEXT_NODE, wxEmptyString, Grid1->GetColLabelValue(c)));
    }
    doc.Save(SeqXmlFileName);

    RenderGridToSeqData();  // incorporate effects into xseq file
    WriteXLightsFile(xlightsFilename);
    UnsavedChanges = false;
    StatusBar1->SetStatusText(_("Updated ")+xlightsFilename);
}

void xLightsFrame::LoadEffectFromString(wxString settings, MapStringString& SettingsMap)
{
    wxString before,after,name,value;
    int cnt=0;
    while (!settings.IsEmpty())
    {
        before=settings.BeforeFirst(',');
        after=settings.AfterFirst(',');
        switch (cnt)
        {
        case 0:
            SettingsMap.clear();
            SettingsMap["effect1"]=before;
            break;
        case 1:
            SettingsMap["effect2"]=before;
            break;
        case 2:
            SettingsMap["LayerMethod"]=before;
            break;
        default:
            name=before.BeforeFirst('=');
            value=before.AfterFirst('=');
            SettingsMap[name]=value;
            break;
        }
        settings=after;
        cnt++;
    }
}

void xLightsFrame::OnBitmapButtonInsertRowClick(wxCommandEvent& event)
{
    if (SeqData.size() == 0)
    {
        wxMessageBox(wxT("You must open a sequence first!"), wxT("Error"));
        return;
    }
    int r=Grid1->GetGridCursorRow();
    Grid1->InsertRows( r, 1 );
    // only the first 2 columns are editable; set everything else to read-only
    int n=Grid1->GetNumberCols();
    for (int c=2; c < n; c++)
    {
        Grid1->SetReadOnly(r,c);
    }
    UnsavedChanges = true;
}

void xLightsFrame::OnBitmapButtonDeleteRowClick(wxCommandEvent& event)
{
    if (SeqData.size() == 0)
    {
        wxMessageBox(wxT("You must open a sequence first!"), wxT("Error"));
        return;
    }
    if ( Grid1->IsSelection() )
    {
        wxGridUpdateLocker locker(Grid1);
        for ( int n = 0; n < Grid1->GetNumberRows(); )
        {
            if ( Grid1->IsInSelection( n , 0 ) )
                Grid1->DeleteRows( n, 1 );
            else
                n++;
        }
    }
    UnsavedChanges = true;
}

void xLightsFrame::OnButtonDisplayElementsClick(wxCommandEvent& event)
{
    if (SeqData.size() == 0)
    {
        wxMessageBox(wxT("You must open a sequence first!"), wxT("Error"));
        return;
    }
    ChooseModelsForSequence();
}

//copy row2 to row1
void xLightsFrame::CopyRow(int row1, int row2)
{
    int i, iMax;
    iMax = Grid1->GetNumberCols();
    for(i=0; i<iMax; i++)
    {
        Grid1->SetCellValue(row1,i,Grid1->GetCellValue(row2,i));
    }
}

// returns time in milliseconds
long xLightsFrame::GetGridStartTimeMSec(int row)
{
    double d=GetGridStartTime(row);
    return long(d*1000.0);
}

// returns time in seconds
double xLightsFrame::GetGridStartTime(int row)
{
    double d;
    wxString s = Grid1->GetCellValue(row,0);
    if (!s.IsEmpty() && s.ToDouble(&d)) return d;
    return 999999.9;
}

// see http://en.wikipedia.org/wiki/Insertion_sort
void xLightsFrame::NumericSort()
{
    //sort the table
    int i, iHole;
    double d;
    const int SortCol=0;

    Grid1->BeginBatch();
    int rowcnt = Grid1->GetNumberRows();
    Grid1->AppendRows(1); // temp swap row

    for (i=1; i < rowcnt; i++)
    {
        d=GetGridStartTime(i);
        CopyRow(rowcnt,i);
        iHole = i;
        while ((iHole > 0) && (GetGridStartTime(iHole-1) > d))
        {
            CopyRow(iHole,iHole-1);
            iHole--;
        }
        CopyRow(iHole,rowcnt);
    }
    Grid1->DeleteRows(rowcnt,1);
    Grid1->EndBatch();
    Grid1->ForceRefresh();
}

void xLightsFrame::OnGrid1CellChange(wxGridEvent& event)
{
    int row = event.GetRow(),
        col = event.GetCol();
    if (col==0)
    {
        // re-order table by start time (column 0)
        Grid1->EnableEditing(false);
        //StatusBar1->SetStatusText(_("New grid value=")+Grid1->GetCellValue(row,col));
        NumericSort();
        Grid1->EnableEditing(true);
    }
    UnsavedChanges = true;
    event.Skip();
}

void xLightsFrame::OnGrid1CellLeftClick(wxGridEvent& event)
{
    int row = event.GetRow(),
        col = event.GetCol();

    if ( row != effGridPrevY || col != effGridPrevX)
    {
        //set selected cell background
        Grid1->SetCellBackgroundColour( *wxYELLOW, row, col);
        Grid1->SetCellBackgroundColour(Grid1->GetDefaultCellBackgroundColour(), effGridPrevY, effGridPrevX);
    }
    effGridPrevY = row;
    effGridPrevX = col;

    if (col >= 2)
    {
        wxString EffectString=Grid1->GetCellValue(row,col);
        if (!EffectString.IsEmpty())
        {
            Choice_Presets->SetSelection(0);  // set to <grid>
            SetEffectControls(EffectString);
        }
    }
    Grid1->ForceRefresh();
    event.Skip();
}

void xLightsFrame::ClearEffectWindow()
{
    wxClientDC dc(ScrolledWindow1);
    dc.Clear();
}

void xLightsFrame::DisplayEffectOnWindow()
{
    wxPen pen;
    wxClientDC dc(ScrolledWindow1);
    wxColour color;
    wxCoord w, h;
    static wxCoord lastw, lasth;
    dc.GetSize(&w, &h);
    if (w!=lastw || h!=lasth)
    {
        // window was resized
        dc.Clear();
        lastw=w;
        lasth=h;
    }
    dc.SetAxisOrientation(true,true);
    if (buffer.RenderHt==1)
    {
        dc.SetDeviceOrigin(w/2,h/2); // set origin at center
    }
    else
    {
        dc.SetDeviceOrigin(w/2,h-1); // set origin at bottom center
    }
    double scaleX = double(w) / buffer.RenderWi;
    double scaleY = double(h) / buffer.RenderHt;
    double scale=scaleY < scaleX ? scaleY : scaleX;
    //scale=0.25;
    dc.SetUserScale(scale,scale);

    /*
            // check that origin is in the right place
            dc.SetUserScale(4,4);
            color.Set(0,0,255);
            pen.SetColour(color);
            dc.SetPen(pen);
            dc.DrawPoint(0,0);
            dc.DrawPoint(1,1);
            dc.DrawPoint(2,2);
            return;
    */
    // layer calculation and map to output
    size_t NodeCount=buffer.Nodes.size();
    double sx,sy;

    for(size_t i=0; i<NodeCount; i++)
    {
        // draw node on screen
        buffer.Nodes[i].GetColor(color);
        pen.SetColour(color);
        dc.SetPen(pen);
        sx=buffer.Nodes[i].screenX;
        sy=buffer.Nodes[i].screenY;
        //#     dc.DrawPoint(buffer.Nodes[i].screenX, buffer.Nodes[i].screenY);
        dc.DrawPoint(sx,sy);

    }
}

void xLightsFrame::OnChoicebookEffectPageChanged(wxChoicebookEvent& event)
{
    ResetEffectStates();
}

void xLightsFrame::OnButtonSeqExportClick(wxCommandEvent& event)
{
    if (SeqData.size() == 0)
    {
        wxMessageBox(wxT("You must open a sequence first!"), wxT("Error"));
        return;
    }
    if (Grid1->GetNumberCols() <= 2)
    {
        wxMessageBox(wxT("No models in the grid!"), wxT("Error"));
        return;
    }
    if (Grid1->GetNumberRows() == 0)
    {
        wxMessageBox(wxT("No grid rows to save!"), wxT("Error"));
        return;
    }
    int DlgResult;
    bool ok;
    wxString filename;
    SeqExportDialog dialog(this);
    do
    {
        ok=true;
        DlgResult=dialog.ShowModal();
        if (DlgResult == wxID_OK)
        {
            // validate inputs
            filename=dialog.TextCtrlFilename->GetValue();
            filename.Trim();
            if (filename.IsEmpty())
            {
                ok=false;
                wxMessageBox(_("The file name cannot be empty"), _("ERROR"));
            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
    if (DlgResult != wxID_OK) return;

    RenderGridToSeqData();

    wxFileName oName(filename);
    oName.SetPath( CurrentDir );
    wxString fullpath;
    wxString format=dialog.ChoiceFormat->GetStringSelection();
    wxStopWatch sw;
    wxString Out3=format.Left(3);
    StatusBar1->SetStatusText(_("Starting Export for ") + format + wxT("-") + Out3);

    if (Out3 == wxT("LOR"))
    {
        if (mediaFilename.IsEmpty())
        {
            oName.SetExt(_("las"));
        }
        else
        {
            oName.SetExt(_("lms"));
        }
        fullpath=oName.GetFullPath();
        WriteLorFile(fullpath);
    }
    else if (Out3 == wxT("Lcb"))
    {
        oName.SetExt(_("lcb"));
        fullpath=oName.GetFullPath();
        WriteLcbFile(fullpath);
    }
    else if (Out3 == wxT("Vix"))
    {
        oName.SetExt(_("vix"));
        fullpath=oName.GetFullPath();
        WriteVixenFile(fullpath);
    }
    else if (Out3 == wxT("Vir"))
    {
        oName.SetExt(_("vir"));
        fullpath=oName.GetFullPath();
        WriteVirFile(fullpath);
    }
    else if (Out3 == wxT("LSP"))
    {
        oName.SetExt(_("user"));
        fullpath=oName.GetFullPath();
        WriteLSPFile(fullpath);
        return;
    }
    else if (Out3 == wxT("HLS"))
    {
        oName.SetExt(_("hlsnc"));
        fullpath=oName.GetFullPath();
        WriteHLSFile(fullpath);
    }
    else if (Out3 == wxT("xLi"))
    {
        oName.SetExt(_(XLIGHTS_SEQUENCE_EXT));
        fullpath=oName.GetFullPath();
        WriteXLightsFile(fullpath);
    }

    StatusBar1->SetStatusText(_("Finished writing: " )+fullpath + wxString::Format(wxT(" in %ld ms "),sw.Time()));
}

void xLightsFrame::OnGrid1CellRightClick(wxGridEvent& event)
{

    wxMenu mnu;
    //mnu.SetClientData( data );
    mnu.Append(ID_DELETE_EFFECT, 	"Delete Highlighted Effect");
    mnu.Append(ID_IGNORE_CLICK, 	"Ignore Click");
    mnu.Append(ID_RANDOM_EFFECT, 	"Create Random Effect");
    mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnPopupClick, NULL, this);
    PopupMenu(&mnu);
}

void xLightsFrame::OnPopupClick(wxCommandEvent &event)
{
    void *data=static_cast<wxMenu *>(event.GetEventObject())->GetClientData();

    if(event.GetId() == ID_DELETE_EFFECT)
    {
        DeleteSelectedEffects(event);
    }
    if(event.GetId() == ID_RANDOM_EFFECT)
    {
        InsertRandomEffects(event);
    }
}

void xLightsFrame::OnbtRandomEffectClick(wxCommandEvent& event)
{
    int r,c;
    wxString v;

    int nRows = Grid1->GetNumberRows();
    int nCols = Grid1->GetNumberCols();

    for (c=2; c<nCols; c++)
    {
        for (r=0; r<nRows; r++)
        {
            v=CreateEffectStringRandom();
            Grid1->SetCellValue(r,c,v);
        }
    }
    UnsavedChanges = true;
}
