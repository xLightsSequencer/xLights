void xLightsFrame::CreateDefaultEffectsXml()
{
    wxXmlNode* root = new wxXmlNode( wxXML_ELEMENT_NODE, wxT("xrgb") );
    EffectsXml.SetRoot( root );
}

wxXmlNode* xLightsFrame::GetModelNode(const wxString& name)
{
    wxXmlNode* e;
    for(e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() ) {
        if (e->GetName() == wxT("model")) {
            if (name == e->GetAttribute(wxT("name"))) return e;
        }
    }
    return NULL;
}

void xLightsFrame::OnButton_PlayAllClick(wxCommandEvent& event)
{
    if (xlightsFilename.IsEmpty()) {
        wxMessageBox(wxT("You must open a sequence first!"), wxT("Error"));
        return;
    }
    SeqPlayColumn=Grid1->GetGridCursorCol();
    if (SeqPlayColumn < 2) {
        wxMessageBox(wxT("Select a cell in a display element column before clicking Play"), wxT("Error"));
        return;
    }
    NextGridRowToPlay=Grid1->GetGridCursorRow();

    wxString ModelName=Grid1->GetColLabelValue(SeqPlayColumn);
    Choice_Models->SetStringSelection(ModelName);
    int sel=Choice_Models->GetSelection();
    if (sel == wxNOT_FOUND) {
        wxMessageBox(_("Can not find model: ")+ModelName, _("Error"));
        return;
    }
    wxXmlNode* ModelXml=(wxXmlNode*)Choice_Models->GetClientData(sel);
    buffer.InitBuffer(ModelXml);
    ClearEffectWindow();
    SetPlayMode(play_rgbseq);
    PlayCurrentXlightsFile();
}

void xLightsFrame::OnButton_PlayEffectClick(wxCommandEvent& event)
{
    int sel=Choice_Models->GetSelection();
    if (sel == wxNOT_FOUND) {
        wxMessageBox(_("No model is selected"), _("ERROR"));
        return;
    }
    wxXmlNode* ModelXml=(wxXmlNode*)Choice_Models->GetClientData(sel);
    buffer.InitBuffer(ModelXml);
    ClearEffectWindow();
    buffer.SetMixType(Choice_LayerMethod->GetStringSelection());
    SetPlayMode(play_effect);
}

void xLightsFrame::OnButton_PresetsClick(wxCommandEvent& event)
{
    EffectListDialog dialog(this);
    wxString name;
    for(wxXmlNode* e=EffectsNode->GetChildren(); e!=NULL; e=e->GetNext() ) {
        if (e->GetName() == wxT("effect")) {
            name=e->GetAttribute(wxT("name"));
            if (!name.IsEmpty()) {
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
    for(size_t i=0; i<cb->GetPageCount(); i++) {
        if (cb->GetPageText(i) == PageName) {
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
    while (!settings.IsEmpty()) {
        before=settings.BeforeFirst(',');
        after=settings.AfterFirst(',');
        switch (cnt) {
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
                if (CtrlWin) {
                    if (name.StartsWith(wxT("ID_SLIDER"))) {
                        wxSlider* ctrl=(wxSlider*)CtrlWin;
                        if (value.ToLong(&TempLong)) ctrl->SetValue(TempLong);
                    } else if (name.StartsWith(wxT("ID_TEXTCTRL"))) {
                        wxTextCtrl* ctrl=(wxTextCtrl*)CtrlWin;
                        ctrl->SetValue(value);
                    } else if (name.StartsWith(wxT("ID_CHOICE"))) {
                        wxChoice* ctrl=(wxChoice*)CtrlWin;
                        ctrl->SetStringSelection(value);
                    } else if (name.StartsWith(wxT("ID_BUTTON"))) {
                        color.Set(value);
                        CtrlWin->SetBackgroundColour(color);
                        SetTextColor(CtrlWin);
                    } else if (name.StartsWith(wxT("ID_CHECKBOX"))) {
                        wxCheckBox* ctrl=(wxCheckBox*)CtrlWin;
                        if (value.ToLong(&TempLong)) ctrl->SetValue(TempLong!=0);
                    } else {
                        wxMessageBox(wxT("Unknown type: ")+name, wxT("Internal Error"));
                    }
                } else {
                    wxMessageBox(wxT("Unable to find: ")+name, wxT("Internal Error"));
                }
                break;
        }
        settings=after;
        cnt++;
    }
    PaletteChanged=true;
    MixTypeChanged=true;
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
    if (NameIdx != wxNOT_FOUND && NameIdx > 0) {
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
            if (name.IsEmpty()) {
                ok=false;
                wxMessageBox(_("A preset name cannot be empty"), _("ERROR"));
            } else if (Choice_Presets->FindString(name) != wxNOT_FOUND) {
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

wxString xLightsFrame::CreateEffectString()
{
    int PageIdx1=Choicebook1->GetSelection();
    int PageIdx2=Choicebook2->GetSelection();
    // ID_CHOICEBOOK1, ID_CHOICEBOOK2, ID_CHOICE_LayerMethod
    wxString s=Choicebook1->GetPageText(PageIdx1)+wxT(",")+Choicebook2->GetPageText(PageIdx2);
    s+=wxT(",")+Choice_LayerMethod->GetStringSelection();
    s+=wxT(",ID_SLIDER_SparkleFrequency=")+wxString::Format(wxT("%d"),Slider_SparkleFrequency->GetValue());
    s+=wxT(",ID_SLIDER_Speed1=")+wxString::Format(wxT("%d"),Slider_Speed1->GetValue());
    s+=wxT(",ID_SLIDER_Speed2=")+wxString::Format(wxT("%d"),Slider_Speed2->GetValue());
    s+=PageControlsToString(Choicebook1->GetPage(PageIdx1));
    s+=SizerControlsToString(FlexGridSizer_Palette1);
    s+=PageControlsToString(Choicebook2->GetPage(PageIdx2));
    s+=SizerControlsToString(FlexGridSizer_Palette2);
    return s;
}

void xLightsFrame::OnButton_PresetUpdateClick(wxCommandEvent& event)
{
    int NameIdx=Choice_Presets->GetSelection();
    if (NameIdx == wxNOT_FOUND) {
        wxMessageBox(_("No preset name is selected"), _("ERROR"));
    } else if (NameIdx == 0) {
        // update grid
        int r,c;
        wxString v=CreateEffectString();
        if ( Grid1->IsSelection() ) {
            // iterate over entire grid looking for selected cells
            int nRows = Grid1->GetNumberRows();
            int nCols = Grid1->GetNumberCols();
            for (r=0; r<nRows; r++) {
                for (c=2; c<nCols; c++) {
                    if (Grid1->IsInSelection(r,c)) {
                        Grid1->SetCellValue(r,c,v);
                    }
                }
            }
        } else {
            // copy to current cell
            r=Grid1->GetGridCursorRow();
            c=Grid1->GetGridCursorCol();
            if (c >=2) {
                Grid1->SetCellValue(r,c,v);
            }
        }
    } else {
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
    for(e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() ) {
        if (e->GetName() == wxT("model")) {
            name=e->GetAttribute(wxT("name"));
            if (!name.IsEmpty()) {
                dialog.ListBox1->Append(name,e);
            }
        }
    }
    dialog.HtmlEasyPrint=HtmlEasyPrint;
    dialog.ShowModal();

    // append any new models to the main xml structure
    for(size_t i=0; i<dialog.ListBox1->GetCount(); i++) {
        e=(wxXmlNode*)dialog.ListBox1->GetClientData(i);
        if (!e->GetParent()) {
            ModelsNode->AddChild(e);
        }
    }
    SaveEffectsFile();
    UpdateModelsList();
}

void xLightsFrame::OnCheckBox_PaletteClick(wxCommandEvent& event)
{
    PaletteChanged=true;
}

// displays color chooser and updates the button's background color with the return value
void xLightsFrame::OnButton_ColorClick(wxCommandEvent& event)
{
    wxWindow* w=(wxWindow*)event.GetEventObject();
    if (ColourDialog1->ShowModal() == wxID_OK) {
        wxColourData retData = ColourDialog1->GetColourData();
        wxColour color = retData.GetColour();
        w->SetBackgroundColour(color);
        PaletteChanged=true;
    }
}

void xLightsFrame::UpdateEffectsList()
{
    wxString name;
    wxString SelectedStr=Choice_Presets->GetStringSelection();
    Choice_Presets->Clear();
    Choice_Presets->Append(wxT(" <grid>"));
    for(wxXmlNode* e=EffectsNode->GetChildren(); e!=NULL; e=e->GetNext() ) {
        if (e->GetName() == wxT("effect")) {
            name=e->GetAttribute(wxT("name"));
            if (!name.IsEmpty()) {
                Choice_Presets->Append(name,e);
            }
        }
    }

    // select a preset if one exists
    if (Choice_Presets->GetCount() > 0) {
        if (SelectedStr.IsEmpty() || !Choice_Presets->SetStringSelection(SelectedStr)) {
            Choice_Presets->SetSelection(0);
        }
    }
}

void xLightsFrame::UpdateModelsList()
{
    wxString name;
    wxString SelectedStr=Choice_Models->GetStringSelection();
    Choice_Models->Clear();
    for(wxXmlNode* e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() ) {
        if (e->GetName() == wxT("model")) {
            name=e->GetAttribute(wxT("name"));
            if (!name.IsEmpty()) {
                Choice_Models->Append(name,e);
            }
        }
    }

    // select a model if one exists
    if (Choice_Models->GetCount() > 0) {
        if (SelectedStr.IsEmpty() || !Choice_Models->SetStringSelection(SelectedStr)) {
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
        if (ChildName.StartsWith(wxT("ID_SLIDER"))) {
            wxSlider* ctrl=(wxSlider*)ChildWin;
            s+=","+ChildName+"="+wxString::Format(wxT("%d"),ctrl->GetValue());
        } else if (ChildName.StartsWith(wxT("ID_TEXTCTRL"))) {
            wxTextCtrl* ctrl=(wxTextCtrl*)ChildWin;
            s+=","+ChildName+"="+ctrl->GetValue();
        } else if (ChildName.StartsWith(wxT("ID_CHOICE"))) {
            wxChoice* ctrl=(wxChoice*)ChildWin;
            s+=","+ChildName+"="+ctrl->GetStringSelection();
        } else if (ChildName.StartsWith(wxT("ID_CHECKBOX"))) {
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
        if (ChildName.StartsWith(wxT("ID_BUTTON"))) {
            wxColour color=ChildWin->GetBackgroundColour();
            s+=","+ChildName+"="+color.GetAsString(wxC2S_HTML_SYNTAX);
        } else if (ChildName.StartsWith(wxT("ID_CHECKBOX"))) {
            wxCheckBox* ctrl=(wxCheckBox*)ChildWin;
            wxString v=(ctrl->IsChecked()) ? wxT("1") : wxT("0");
            s+=","+ChildName+"="+v;
        }
    }
    return s;
}

// returns true on success
void xLightsFrame::LoadEffectsFile()
{
    wxFileName effectsFile;
    effectsFile.AssignDir( CurrentDir );
    effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
    ModelsNode=0;
    EffectsNode=0;
    PalettesNode=0;
    if (!effectsFile.FileExists()) {
        // file does not exist, so create an empty xml doc
        CreateDefaultEffectsXml();
    } else if (!EffectsXml.Load( effectsFile.GetFullPath() )) {
        wxMessageBox(_("Unable to load RGB effects file"), _("Error"));
        CreateDefaultEffectsXml();
    }
    wxXmlNode* root=EffectsXml.GetRoot();
    if (root->GetName() != wxT("xrgb")) {
        wxMessageBox(_("Invalid RGB effects file. Press Save File button to start a new file."), _("Error"));
        CreateDefaultEffectsXml();
    }
    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() ) {
        if (e->GetName() == wxT("models")) ModelsNode=e;
        if (e->GetName() == wxT("effects")) EffectsNode=e;
        if (e->GetName() == wxT("palettes")) PalettesNode=e;
    }
    if (ModelsNode == 0) {
        ModelsNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT("models") );
        root->AddChild( ModelsNode );
    }
    if (EffectsNode == 0) {
        EffectsNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT("effects") );
        root->AddChild( EffectsNode );
    }
    if (PalettesNode == 0) {
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
    if (!EffectsXml.Save( effectsFile.GetFullPath() )) {
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
    for (int i=1; i<=6; i++) {
        if (SettingsMap[wxString::Format(wxT("ID_CHECKBOX_Palette%d_%d"),PaletteNum,i)] ==  wxT("1")) {
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

void xLightsFrame::RenderEffectFromString(int layer, MapStringString& SettingsMap)
{
    buffer.SetLayer(layer);
    wxString LayerStr=layer==0 ? wxT("1") : wxT("2");
    buffer.SetSpeed(wxAtoi(SettingsMap[wxT("ID_SLIDER_Speed")+LayerStr]));
    wxString effect=SettingsMap[wxT("effect")+LayerStr];
    SendToLogAndStatusBar(wxT("effect")+LayerStr+wxT("=")+effect);
    if (effect == wxT("Bars")) {
        buffer.RenderBars(wxAtoi(SettingsMap[wxT("ID_SLIDER_Bars")+LayerStr+wxT("_BarCount")]),
                          BarEffectDirections.Index(SettingsMap[wxT("ID_CHOICE_Bars")+LayerStr+wxT("_Direction")]),
                          SettingsMap[wxT("ID_CHECKBOX_Bars")+LayerStr+wxT("_Highlight")]==wxT("1"),
                          SettingsMap[wxT("ID_CHECKBOX_Bars")+LayerStr+wxT("_3D")]==wxT("1"));
    } else if (effect == wxT("Butterfly")) {
        buffer.RenderButterfly(ButterflyEffectColors.Index(SettingsMap[wxT("ID_CHOICE_Butterfly")+LayerStr+wxT("_Colors")]),
                               wxAtoi(SettingsMap[wxT("ID_SLIDER_Butterfly")+LayerStr+wxT("_Style")]),
                               wxAtoi(SettingsMap[wxT("ID_SLIDER_Butterfly")+LayerStr+wxT("_Chunks")]),
                               wxAtoi(SettingsMap[wxT("ID_SLIDER_Butterfly")+LayerStr+wxT("_Skip")]));
    } else if (effect == wxT("Color Wash")) {
        buffer.RenderColorWash(SettingsMap[wxT("ID_CHECKBOX_ColorWash")+LayerStr+wxT("_HFade")]==wxT("1"),
                               SettingsMap[wxT("ID_CHECKBOX_ColorWash")+LayerStr+wxT("_VFade")]==wxT("1"));
    } else if (effect == wxT("Fire")) {
        buffer.RenderFire();
    } else if (effect == wxT("Garlands")) {
        buffer.RenderGarlands(wxAtoi(SettingsMap[wxT("ID_SLIDER_Garlands")+LayerStr+wxT("_Type")]),
                              wxAtoi(SettingsMap[wxT("ID_SLIDER_Garlands")+LayerStr+wxT("_Spacing")]));
    } else if (effect == wxT("Life")) {
    } else if (effect == wxT("Meteors")) {
        buffer.RenderMeteors(MeteorsEffectTypes.Index(SettingsMap[wxT("ID_CHOICE_Meteors")+LayerStr+wxT("_Type")]),
                             wxAtoi(SettingsMap[wxT("Slider_Meteors")+LayerStr+wxT("_Count")]),
                             wxAtoi(SettingsMap[wxT("Slider_Meteors")+LayerStr+wxT("_Length")]));
    } else if (effect == wxT("Pictures")) {
        buffer.RenderPictures(PictureEffectDirections.Index(SettingsMap[wxT("ID_CHOICE_Pictures")+LayerStr+wxT("_Direction")]),
                              SettingsMap[wxT("ID_TEXTCTRL_Pictures")+LayerStr+wxT("_Filename")]);
    } else if (effect == wxT("Snowflakes")) {
    } else if (effect == wxT("Snowstorm")) {
    } else if (effect == wxT("Spirals")) {
        buffer.RenderSpirals(wxAtoi(SettingsMap[wxT("ID_SLIDER_Spirals")+LayerStr+wxT("_Count")]),
                             wxAtoi(SettingsMap[wxT("ID_SLIDER_Spirals")+LayerStr+wxT("_Direction")]),
                             wxAtoi(SettingsMap[wxT("ID_SLIDER_Spirals")+LayerStr+wxT("_Rotation")]),
                             wxAtoi(SettingsMap[wxT("ID_SLIDER_Spirals")+LayerStr+wxT("_Thickness")]),
                             SettingsMap[wxT("ID_CHECKBOX_Spirals")+LayerStr+wxT("_Blend")]==wxT("1"),
                             SettingsMap[wxT("ID_CHECKBOX_Spirals")+LayerStr+wxT("_3D")]==wxT("1"));
    } else if (effect == wxT("Text")) {
        buffer.RenderText(wxAtoi(SettingsMap[wxT("ID_SLIDER_Text")+LayerStr+wxT("_Top")]),
                          SettingsMap[wxT("ID_TEXTCTRL_Text")+LayerStr+wxT("_Line1")],
                          SettingsMap[wxT("ID_TEXTCTRL_Text")+LayerStr+wxT("_Line2")],
                          SettingsMap[wxT("ID_TEXTCTRL_Text")+LayerStr+wxT("_Font")]);
    }
}

void xLightsFrame::TimerEffect()
{
    wxString s;
    buffer.Clear();

    // update SparkleFrequency
    int freq=Slider_SparkleFrequency->GetValue();
    if (freq == Slider_SparkleFrequency->GetMax()) freq=0;
    buffer.SetSparkle(freq);

    if (PaletteChanged) {
        UpdateBufferPalette();
        PaletteChanged=false;
    }
    if (MixTypeChanged) {
        s=Choice_LayerMethod->GetStringSelection();
        buffer.SetMixType(s);
        MixTypeChanged=false;
    }

    // render effect 1
    buffer.SetSpeed(Slider_Speed1->GetValue());
    buffer.SetLayer(0);
    switch (Choicebook1->GetSelection())
    {
        case 0: break;   // none
        case 1:
            buffer.RenderBars(Slider_Bars1_BarCount->GetValue(),
                              Choice_Bars1_Direction->GetSelection(),
                              CheckBox_Bars1_Highlight->GetValue(),
                              CheckBox_Bars1_3D->GetValue());
            break;
        case 2:
            buffer.RenderButterfly(Choice_Butterfly1_Colors->GetSelection(),
                                   Slider_Butterfly1_Style->GetValue(),
                                   Slider_Butterfly1_Chunks->GetValue(),
                                   Slider_Butterfly1_Skip->GetValue());
            break;
        case 3:
            buffer.RenderColorWash(CheckBox_ColorWash1_HFade->GetValue(),
                                   CheckBox_ColorWash1_VFade->GetValue());
            break;
        case 4:
            buffer.RenderFire();
            break;
        case 5:
            buffer.RenderGarlands(Slider_Garlands1_Type->GetValue(),
                                  Slider_Garlands1_Spacing->GetValue());
            break;
        case 6:
            buffer.RenderLife(Slider_Life1_Count->GetValue(),
                              Slider_Life1_Seed->GetValue());
            break;
        case 7:
            buffer.RenderMeteors(Choice_Meteors1_Type->GetSelection(),
                                 Slider_Meteors1_Count->GetValue(),
                                 Slider_Meteors1_Length->GetValue());
            break;
        case 8:
            buffer.RenderPictures(Choice_Pictures1_Direction->GetSelection(),
                                  TextCtrl_Pictures1_Filename->GetValue());
            break;
        case 9:
            buffer.RenderSnowflakes();
            break;
        case 10:
            buffer.RenderSnowstorm(Slider_Snowstorm1_Count->GetValue(),
                                   Slider_Snowstorm1_Length->GetValue());
            break;
        case 11:
            buffer.RenderSpirals(Slider_Spirals1_Count->GetValue(),
                                 Slider_Spirals1_Direction->GetValue(),
                                 Slider_Spirals1_Rotation->GetValue(),
                                 Slider_Spirals1_Thickness->GetValue(),
                                 CheckBox_Spirals1_Blend->GetValue(),
                                 CheckBox_Spirals1_3D->GetValue());
            break;
        case 12:
            buffer.RenderText(Slider_Text1_Top->GetValue(),
                              TextCtrl_Text1_Line1->GetValue(),
                              TextCtrl_Text1_Line2->GetValue(),
                              TextCtrl_Text1_Font->GetValue());
            break;
    }

    // render effect 2
    buffer.SetSpeed(Slider_Speed2->GetValue());
    buffer.SetLayer(1);
    switch (Choicebook2->GetSelection())
    {
        case 0: break;   // none
        case 1:
            buffer.RenderBars(Slider_Bars2_BarCount->GetValue(),
                              Choice_Bars2_Direction->GetSelection(),
                              CheckBox_Bars2_Highlight->GetValue(),
                              CheckBox_Bars2_3D->GetValue());
            break;
        case 2:
            buffer.RenderButterfly(Choice_Butterfly2_Colors->GetSelection(),
                                   Slider_Butterfly2_Style->GetValue(),
                                   Slider_Butterfly2_Chunks->GetValue(),
                                   Slider_Butterfly2_Skip->GetValue());
            break;
        case 3:
            buffer.RenderColorWash(CheckBox_ColorWash2_HFade->GetValue(),
                                   CheckBox_ColorWash2_VFade->GetValue());
            break;
        case 4:
            buffer.RenderFire();
            break;
        case 5:
            buffer.RenderGarlands(Slider_Garlands2_Type->GetValue(),
                                  Slider_Garlands2_Spacing->GetValue());
            break;
        case 6:
            buffer.RenderLife(Slider_Life2_Count->GetValue(),
                              Slider_Life2_Seed->GetValue());
            break;
        case 7:
            buffer.RenderMeteors(Choice_Meteors2_Type->GetSelection(),
                                 Slider_Meteors2_Count->GetValue(),
                                 Slider_Meteors2_Length->GetValue());
            break;
        case 8:
            buffer.RenderPictures(Choice_Pictures2_Direction->GetSelection(),
                                  TextCtrl_Pictures2_Filename->GetValue());
            break;
        case 9:
            buffer.RenderSnowflakes();
            break;
        case 10:
            buffer.RenderSnowstorm(Slider_Snowstorm2_Count->GetValue(),
                                   Slider_Snowstorm2_Length->GetValue());
            break;
        case 11:
            buffer.RenderSpirals(Slider_Spirals2_Count->GetValue(),
                                 Slider_Spirals2_Direction->GetValue(),
                                 Slider_Spirals2_Rotation->GetValue(),
                                 Slider_Spirals2_Thickness->GetValue(),
                                 CheckBox_Spirals2_Blend->GetValue(),
                                 CheckBox_Spirals2_3D->GetValue());
            break;
        case 12:
            buffer.RenderText(Slider_Text2_Top->GetValue(),
                              TextCtrl_Text2_Line1->GetValue(),
                              TextCtrl_Text2_Line2->GetValue(),
                              TextCtrl_Text2_Font->GetValue());
            break;
    }
    buffer.CalcOutput();
    DisplayEffectOnWindow();
    if (CheckBoxLightOutput->IsChecked() && xout) {
        size_t ChannelNum=buffer.StartChannel-1;
        size_t NodeCnt=buffer.GetNodeCount();
        for(size_t i=0; i<NodeCnt; i++) {
            xout->SetIntensity(ChannelNum,buffer.Nodes[i].GetChannel(0));
            ChannelNum++;
            xout->SetIntensity(ChannelNum,buffer.Nodes[i].GetChannel(1));
            ChannelNum++;
            xout->SetIntensity(ChannelNum,buffer.Nodes[i].GetChannel(2));
            ChannelNum++;
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
    int period;
    long StartTime;
    int rowcnt=Grid1->GetNumberRows();
    switch (SeqPlayerState) {
        case STARTING_SEQ_ANIM:
            ResetTimer(PLAYING_SEQ_ANIM, GetGridStartTimeMSec(NextGridRowToPlay));
            break;
        case PLAYING_SEQ_ANIM:
            if (xout && !xout->TxEmpty()) {
                TxOverflowCnt++;
                break;
            }
            period = msec / XTIMER_INTERVAL;
            if (period >= SeqNumPeriods) {
                // sequence has finished
                if (xout) xout->alloff();
                SetPlayMode(play_off);
            } else {
                if (NextGridRowToPlay < rowcnt && msec >= GetGridStartTimeMSec(NextGridRowToPlay)) {
                    // start next effect
                    Grid1->MakeCellVisible(NextGridRowToPlay,SeqPlayColumn);
                    Grid1->SelectBlock(NextGridRowToPlay,SeqPlayColumn,NextGridRowToPlay,SeqPlayColumn);
                    SetEffectControls(Grid1->GetCellValue(NextGridRowToPlay,SeqPlayColumn));
                    NextGridRowToPlay++;
                }
                TimerEffect();
                if (period % 20 == 0) UpdateRgbPlaybackStatus(period/20,wxT("animation"));
            }
            break;
        case STARTING_SEQ:
            StartTime=GetGridStartTimeMSec(NextGridRowToPlay);
            if(PlayerDlg->MediaCtrl->GetState() == wxMEDIASTATE_PLAYING){
                ResetTimer(PLAYING_SEQ, StartTime);
            } else {
                PlayerDlg->MediaCtrl->Seek(StartTime);
                PlayerDlg->MediaCtrl->Play();
            }
            break;
        case PLAYING_SEQ:
            if (xout && !xout->TxEmpty()) {
                TxOverflowCnt++;
                break;
            }
            msec = PlayerDlg->MediaCtrl->Tell();
            period = msec / XTIMER_INTERVAL;
            if (period >= SeqNumPeriods || PlayerDlg->MediaCtrl->GetState() != wxMEDIASTATE_PLAYING) {
                // sequence has finished
                PlayerDlg->MediaCtrl->Stop();
                if (xout) xout->alloff();
                SetPlayMode(play_off);
            } else {
                if (NextGridRowToPlay < rowcnt && msec >= GetGridStartTimeMSec(NextGridRowToPlay)) {
                    // start next effect
                    Grid1->MakeCellVisible(NextGridRowToPlay,SeqPlayColumn);
                    Grid1->SelectBlock(NextGridRowToPlay,SeqPlayColumn,NextGridRowToPlay,SeqPlayColumn);
                    SetEffectControls(Grid1->GetCellValue(NextGridRowToPlay,SeqPlayColumn));
                    NextGridRowToPlay++;
                }
                TimerEffect();
                if (period % 20 == 0) UpdateRgbPlaybackStatus(period/20,wxT("music"));
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

void xLightsFrame::OnButton_Text1_FontClick(wxCommandEvent& event)
{
    wxFont oldfont,newfont;
    oldfont.SetNativeFontInfoUserDesc(TextCtrl_Text1_Font->GetValue());
    newfont=wxGetFontFromUser(this,oldfont);
    if (newfont.IsOk()) {
        TextCtrl_Text1_Font->SetValue(newfont.GetNativeFontInfoUserDesc());
    }
}

void xLightsFrame::OnButton_Text2_FontClick(wxCommandEvent& event)
{
    wxFont oldfont,newfont;
    oldfont.SetNativeFontInfoUserDesc(TextCtrl_Text2_Font->GetValue());
    newfont=wxGetFontFromUser(this,oldfont);
    if (newfont.IsOk()) {
        TextCtrl_Text2_Font->SetValue(newfont.GetNativeFontInfoUserDesc());
    }
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
    for(wxXmlNode* e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() ) {
        if (e->GetName() == wxT("model")) {
            name=e->GetAttribute(wxT("name"));
            if (!name.IsEmpty()) {
                a.Add(name);
            }
        }
    }
}

void xLightsFrame::GetGridColumnLabels(wxArrayString& a)
{
    int n=Grid1->GetNumberCols();
    for(int i=2; i < n; i++) {
        a.Add(Grid1->GetColLabelValue(i));
    }
}

void xLightsFrame::ChooseModelsForSequence()
{
    if (xlightsFilename.IsEmpty()) {
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
    for(e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() ) {
        if (e->GetName() == wxT("model")) {
            name=e->GetAttribute(wxT("name"));
            // allow only models where MyDisplay is set?
            //if (!name.IsEmpty() && e->GetAttribute(wxT("MyDisplay"),wxT("0"))==wxT("1")) {
            if (!name.IsEmpty()) {
                dialog.CheckListBox1->Append(name);
                idx=dialog.CheckListBox1->FindString(name);
                if (idx != wxNOT_FOUND && labels.Index(name,false) != wxNOT_FOUND) {
                    dialog.CheckListBox1->Check(idx,true);
                }
            }
        }
    }


    dialog.StaticText_Filename->SetLabel(xlightsFilename);
    if (dialog.ShowModal() != wxID_OK) return;

    // add checked models to grid
    wxGridCellAttr* readonly=new wxGridCellAttr;
    readonly->SetReadOnly();
    size_t cnt = dialog.CheckListBox1->GetCount();
    for (size_t i=0; i < cnt; i++) {
        if (dialog.CheckListBox1->IsChecked(i)) {
            name=dialog.CheckListBox1->GetString(i);
            idx=labels.Index(name,false);
            if (idx == wxNOT_FOUND) {
                // newly checked item, so add it
                Grid1->AppendCols();
                int colnum=Grid1->GetNumberCols()-1;
                Grid1->SetColLabelValue(colnum,name);
                Grid1->SetColAttr(colnum,readonly);  // this only sets cells to ro that already exist (not insterted rows)
            } else {
                // item already exists
                labels[idx].Clear();
            }
        }
    }

    // any non-empty entries in labels represent items that were unchecked - so delete them from grid
    for(idx=labels.GetCount()-1; idx >= 0; idx--) {
        if (!labels[idx].IsEmpty()) {
            Grid1->DeleteCols(idx+2);
        }
    }
}

void xLightsFrame::OnButton_ChannelMapClick(wxCommandEvent& event)
{
    if (xlightsFilename.IsEmpty()) {
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

void xLightsFrame::OnBitmapButtonOpenSeqClick(wxCommandEvent& event)
{
    wxArrayString files;
    wxDir::GetAllFiles(CurrentDir,&files,"*.xseq");
    wxString filename = wxGetSingleChoice("Select xLights sequence to open","Open Sequence",files,this);
    if (filename.IsEmpty()) return;  // user pressed cancel

    // reset grid
    int n;
    n=Grid1->GetNumberCols();
    if (n > 2) Grid1->DeleteCols(2, n-2);
    n=Grid1->GetNumberRows();
    if (n > 0) Grid1->DeleteRows(0, n);

    // read xlights file
    ReadXlightsFile(filename);
    DisplayXlightsFilename(filename);
    SeqBaseChannel=1;
    SeqChanCtrlBasic=false;
    SeqChanCtrlColor=false;

    // read xml sequence info
    wxFileName FileObj(filename);
    FileObj.SetExt("xml");
    SeqXmlFileName=FileObj.GetFullPath();
    if (!FileObj.FileExists()) {
        ChooseModelsForSequence();
        return;
    }

    // read xml
    wxXmlDocument doc;
    if (!doc.Load(SeqXmlFileName)) {
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
    for(tr=root->GetChildren(), r=0; tr!=NULL; tr=tr->GetNext(), r++ ) {
        if (tr->GetName() != wxT("tr")) continue;
        if (r > 0) {
            Grid1->AppendRows();
        }
        for(td=tr->GetChildren(), c=0; td!=NULL; td=td->GetNext(), c++ ) {
            if (td->GetName() != wxT("td")) continue;
            if (r==0) {
                if (c >= 2) {
                    ColName=td->GetNodeContent();
                    if (ModelNames.Index(ColName) == wxNOT_FOUND) {
                        dialog.StaticTextMessage->SetLabel(wxT("Element '")+ColName+wxT("'\ndoes not exist in your list of models"));
                        dialog.Fit();
                        dialog.ShowModal();
                        if (dialog.RadioButtonAdd->GetValue()) {
                        } else if (dialog.RadioButtonDelete->GetValue()) {
                            DeleteCols.Add(c);
                        } else {
                            // rename
                            Grid1->AppendCols();
                            Grid1->SetColLabelValue(Grid1->GetNumberCols()-1, dialog.ChoiceModels->GetStringSelection());
                        }
                    } else {
                        Grid1->AppendCols();
                        Grid1->SetColLabelValue(Grid1->GetNumberCols()-1, ColName);
                    }
                }
            } else if (DeleteCols.Index(c) == wxNOT_FOUND) {
                Grid1->SetCellValue(r-1,c,td->GetNodeContent());
            }
        }
    }

    // make new columns read-only
    wxGridCellAttr* readonly=new wxGridCellAttr;
    readonly->SetReadOnly();
    for (c=2; c < Grid1->GetNumberCols(); c++) {
        Grid1->SetColAttr(c,readonly);
    }
}

void xLightsFrame::OnBitmapButtonSaveSeqClick(wxCommandEvent& event)
{
    if (xlightsFilename.IsEmpty()) {
        wxMessageBox(wxT("You must open a sequence first!"), wxT("Error"));
        return;
    }

    // save Grid1 to xml
    int rowcnt=Grid1->GetNumberRows();
    int colcnt=Grid1->GetNumberCols();
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
    for (r=rowcnt-1; r>=0; r--) {
        tr=new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("tr"));
        for (c=colcnt-1; c>=0; c--) {
            td=new wxXmlNode(tr, wxXML_ELEMENT_NODE, wxT("td"));
            td->AddChild(new wxXmlNode(td, wxXML_TEXT_NODE, wxEmptyString, Grid1->GetCellValue(r,c)));
        }
    }

    // save labels to first row
    tr=new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("tr"));
    for (c=colcnt-1; c>=0; c--) {
        td=new wxXmlNode(tr, wxXML_ELEMENT_NODE, wxT("td"));
        td->AddChild(new wxXmlNode(td, wxXML_TEXT_NODE, wxEmptyString, Grid1->GetColLabelValue(c)));
    }

    doc.Save(SeqXmlFileName);

    // incorporate effects into xseq file

    MapStringString SettingsMap;
    wxString ColName;
    long msec;
    size_t ChannelNum, NodeCnt;
    LoadEffectFromString(wxT("None,None,Effect 1"), SettingsMap);
    for (c=2; c<colcnt; c++) {
        ColName=Grid1->GetColLabelValue(c);
        td=GetModelNode(ColName);
        if (!td) continue;
        buffer.InitBuffer(td);
        if (!buffer.MyDisplay) continue;
        NodeCnt=buffer.GetNodeCount();
        ChannelNum=buffer.StartChannel-1+NodeCnt*3; // last channel
        if (ChannelNum > SeqNumChannels) {
            // need to add more channels to existing sequence
            wxMessageBox(wxString::Format(wxT("Increasing sequence channel count from %ld to %d"),SeqNumChannels,ChannelNum));
            SeqNumChannels=ChannelNum;
            SeqDataLen=SeqNumChannels*SeqNumPeriods;
            SeqData.resize(SeqDataLen,0);
        }
        NextGridRowToPlay=0;
        for (int p=0; p<SeqNumPeriods; p++) {
            msec=p * XTIMER_INTERVAL;
            buffer.Clear();
            if (NextGridRowToPlay < rowcnt && msec >= GetGridStartTimeMSec(NextGridRowToPlay)) {
                // start next effect
                wxYield();
                LoadEffectFromString(Grid1->GetCellValue(NextGridRowToPlay,c), SettingsMap);
                UpdateBufferPaletteFromMap(1,SettingsMap);
                UpdateBufferPaletteFromMap(2,SettingsMap);
                buffer.SetMixType(SettingsMap["LayerMethod"]);
                // update SparkleFrequency
                int freq=wxAtoi(SettingsMap["ID_SLIDER_SparkleFrequency"]);
                if (freq == Slider_SparkleFrequency->GetMax()) freq=0;
                buffer.SetSparkle(freq);
                NextGridRowToPlay++;
            }
            RenderEffectFromString(0, SettingsMap);
            RenderEffectFromString(1, SettingsMap);
            buffer.CalcOutput();
            // update SeqData with contents of buffer
            ChannelNum=buffer.StartChannel-1;
            for(int n=0; n<NodeCnt; n++) {
                SeqData[ChannelNum*SeqNumPeriods+p]=buffer.Nodes[n].GetChannel(0);
                ChannelNum++;
                SeqData[ChannelNum*SeqNumPeriods+p]=buffer.Nodes[n].GetChannel(1);
                ChannelNum++;
                SeqData[ChannelNum*SeqNumPeriods+p]=buffer.Nodes[n].GetChannel(2);
                ChannelNum++;
            }
        }
    }
    WriteXLightsFile(xlightsFilename);
    StatusBar1->SetStatusText(_("Updated ")+xlightsFilename);
}

void xLightsFrame::LoadEffectFromString(wxString settings, MapStringString& SettingsMap)
{
    wxString before,after,name,value;
    int cnt=0;
    while (!settings.IsEmpty()) {
        before=settings.BeforeFirst(',');
        after=settings.AfterFirst(',');
        switch (cnt) {
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
    if (xlightsFilename.IsEmpty()) {
        wxMessageBox(wxT("You must open a sequence first!"), wxT("Error"));
        return;
    }
    int r=Grid1->GetGridCursorRow();
    Grid1->InsertRows( r, 1 );
    // only the first 2 columns are editable; set everything else to read-only
    int n=Grid1->GetNumberCols();
    for (int c=2; c < n; c++) {
        Grid1->SetReadOnly(r,c);
    }
}

void xLightsFrame::OnBitmapButtonDeleteRowClick(wxCommandEvent& event)
{
    if (xlightsFilename.IsEmpty()) {
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
}

void xLightsFrame::OnButtonDisplayElementsClick(wxCommandEvent& event)
{
    if (xlightsFilename.IsEmpty()) {
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
    for(i=0;i<iMax;i++) {
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
    int SortCol=0;

    Grid1->BeginBatch();
    int rowcnt = Grid1->GetNumberRows();
    Grid1->AppendRows(1); // temp swap row

    for (i=1; i < rowcnt; i++) {
        d=GetGridStartTime(i);
        CopyRow(rowcnt,i);
        iHole = i;
        while ((iHole > 0) && (GetGridStartTime(iHole-1) > d)) {
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
    if (col==0) {
        // re-order table by start time (column 0)
        Grid1->EnableEditing(false);
        //StatusBar1->SetStatusText(_("New grid value=")+Grid1->GetCellValue(row,col));
        NumericSort();
        Grid1->EnableEditing(true);
    }
}

void xLightsFrame::OnGrid1CellLeftClick(wxGridEvent& event)
{
    int row = event.GetRow(),
        col = event.GetCol();
    if (col >= 2) {
        wxString EffectString=Grid1->GetCellValue(row,col);
        if (!EffectString.IsEmpty()) {
            Choice_Presets->SetSelection(0);  // set to <grid>
            SetEffectControls(EffectString);
        }
    }
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
    if (w!=lastw || h!=lasth) {
        // window was resized
        dc.Clear();
        lastw=w;
        lasth=h;
    }
    dc.SetAxisOrientation(true,true);
    if (buffer.RenderHt==1) {
        dc.SetDeviceOrigin(w/2,h/2); // set origin at center
    } else {
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
    for(size_t i=0; i<NodeCount; i++) {
        // draw node on screen
        buffer.Nodes[i].GetColor(color);
        pen.SetColour(color);
        dc.SetPen(pen);
        //dc.DrawCircle(Nodes[i].screenX, Nodes[i].screenY,1);
        dc.DrawPoint(buffer.Nodes[i].screenX, buffer.Nodes[i].screenY);
    }
}
