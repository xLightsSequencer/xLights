#include "xLightsXmlFile.h"
#include "xLightsMain.h"

#define string_format wxString::Format

xLightsXmlFile::xLightsXmlFile()
{
    Clear();
    latest_version = _("4.0.0");
    SetPath( xLightsFrame::CurrentDir );
}

xLightsXmlFile::~xLightsXmlFile()
{
}

static wxString SubstituteV3toV4tags(const wxString& effect_string)
{
    wxString new_string = effect_string;
    new_string.Replace(wxString("ID_CHECKBOX_LayerMorph"), wxString("T_CHECKBOX_LayerMorph"));
    new_string.Replace(wxString("ID_SLIDER_EffectLayerMix"), wxString("T_SLIDER_EffectLayerMix"));
    new_string.Replace(wxString("E1_SLIDER_Speed"), wxString("T_SLIDER_Speed"));
    new_string.Replace(wxString("E2_SLIDER_Speed"), wxString("T_SLIDER_Speed"));
    new_string.Replace(wxString("E1_TEXTCTRL_Fadeout"), wxString("T_TEXTCTRL_Fadeout"));
    new_string.Replace(wxString("E2_TEXTCTRL_Fadeout"), wxString("T_TEXTCTRL_Fadeout"));
    new_string.Replace(wxString("E1_TEXTCTRL_Fadein"), wxString("T_TEXTCTRL_Fadein"));
    new_string.Replace(wxString("E2_TEXTCTRL_Fadein"), wxString("T_TEXTCTRL_Fadein"));
    new_string.Replace(wxString("E1_CHECKBOX_FitToTime"), wxString("T_CHECKBOX_FitToTime"));
    new_string.Replace(wxString("E2_CHECKBOX_FitToTime"), wxString("T_CHECKBOX_FitToTime"));
    new_string.Replace(wxString("E1_CHECKBOX_OverlayBkg"), wxString("T_CHECKBOX_OverlayBkg"));
    new_string.Replace(wxString("E2_CHECKBOX_OverlayBkg"), wxString("T_CHECKBOX_OverlayBkg"));
    new_string.Replace(wxString("ID_SLIDER_SparkleFrequency"), wxString("C_SLIDER_SparkleFrequency"));
    new_string.Replace(wxString("ID_SLIDER_Brightness"), wxString("C_SLIDER_Brightness"));
    new_string.Replace(wxString("ID_SLIDER_Contrast"), wxString("C_SLIDER_Contrast"));
    new_string.Replace(wxString("E1_BUTTON_Palette1"), wxString("C_BUTTON_Palette1"));
    new_string.Replace(wxString("E1_BUTTON_Palette2"), wxString("C_BUTTON_Palette2"));
    new_string.Replace(wxString("E1_BUTTON_Palette3"), wxString("C_BUTTON_Palette3"));
    new_string.Replace(wxString("E1_BUTTON_Palette4"), wxString("C_BUTTON_Palette4"));
    new_string.Replace(wxString("E1_BUTTON_Palette5"), wxString("C_BUTTON_Palette5"));
    new_string.Replace(wxString("E1_BUTTON_Palette6"), wxString("C_BUTTON_Palette6"));
    new_string.Replace(wxString("E2_BUTTON_Palette1"), wxString("C_BUTTON_Palette1"));
    new_string.Replace(wxString("E2_BUTTON_Palette2"), wxString("C_BUTTON_Palette2"));
    new_string.Replace(wxString("E2_BUTTON_Palette3"), wxString("C_BUTTON_Palette3"));
    new_string.Replace(wxString("E2_BUTTON_Palette4"), wxString("C_BUTTON_Palette4"));
    new_string.Replace(wxString("E2_BUTTON_Palette5"), wxString("C_BUTTON_Palette5"));
    new_string.Replace(wxString("E2_BUTTON_Palette6"), wxString("C_BUTTON_Palette6"));
    new_string.Replace(wxString("E1_CHECKBOX_Palette1"), wxString("C_CHECKBOX_Palette1"));
    new_string.Replace(wxString("E1_CHECKBOX_Palette2"), wxString("C_CHECKBOX_Palette2"));
    new_string.Replace(wxString("E1_CHECKBOX_Palette3"), wxString("C_CHECKBOX_Palette3"));
    new_string.Replace(wxString("E1_CHECKBOX_Palette4"), wxString("C_CHECKBOX_Palette4"));
    new_string.Replace(wxString("E1_CHECKBOX_Palette5"), wxString("C_CHECKBOX_Palette5"));
    new_string.Replace(wxString("E1_CHECKBOX_Palette6"), wxString("C_CHECKBOX_Palette6"));
    new_string.Replace(wxString("E2_CHECKBOX_Palette1"), wxString("C_CHECKBOX_Palette1"));
    new_string.Replace(wxString("E2_CHECKBOX_Palette2"), wxString("C_CHECKBOX_Palette2"));
    new_string.Replace(wxString("E2_CHECKBOX_Palette3"), wxString("C_CHECKBOX_Palette3"));
    new_string.Replace(wxString("E2_CHECKBOX_Palette4"), wxString("C_CHECKBOX_Palette4"));
    new_string.Replace(wxString("E2_CHECKBOX_Palette5"), wxString("C_CHECKBOX_Palette5"));
    new_string.Replace(wxString("E2_CHECKBOX_Palette6"), wxString("C_CHECKBOX_Palette6"));
    new_string.Replace(wxString("E1"), wxString("E"));
    new_string.Replace(wxString("E2"), wxString("E"));

    new_string = wxString("T_CHOICE_LayerMethod=") + new_string;

    return new_string;
}


static wxXmlNode* AddChildXmlNode(wxXmlNode* node, const wxString& node_name, const wxString& node_data)
{
    wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, node_name);
    wxXmlNode* data_node = new wxXmlNode(new_node,wxXML_TEXT_NODE,wxT(""),node_data);
    node->AddChild(new_node);
    return new_node;
}

static wxXmlNode* AddChildXmlNode(wxXmlNode* node, const wxString& node_name)
{
    wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, node_name);
    node->AddChild(new_node);
    return new_node;
}

static void AddTimingAttributes(wxXmlNode* node, const wxString& type, const wxString& name, const wxString& visible, const wxString& active)
{
    wxXmlNode* child = AddChildXmlNode(node, wxT("Element"), wxT(""));
    child->AddAttribute(wxT("type"), type);
    child->AddAttribute(wxT("name"), name);
    child->AddAttribute(wxT("visible"), visible);
    child->AddAttribute(wxT("active"), active);
}

void xLightsXmlFile::Clear()
{
    is_loaded = false;
    needs_conversion = true;
    version_string = _("");
    models.Clear();
    timing_protection.Clear();
    timing.Clear();
    label_protection.Clear();
    labels.Clear();
    effect_protection.Clear();
    effects.Clear();
    header_info.Clear();
    for(int i = 0; i < NUM_TYPES; ++i )
    {
        header_info.push_back(_(""));
    }
}

static void SetNodeContent(wxXmlNode* node, const wxString& content)
{
    wxXmlNode* element=node->GetChildren();
    if(element != NULL)
    {
        element->SetContent(content);
    }
    else
    {
        wxXmlNode* data_node = new wxXmlNode(node,wxXML_TEXT_NODE,wxT(""),content);
    }
}

void xLightsXmlFile::SetHeaderInfo(wxArrayString info)
{
    header_info = info;

    if( needs_conversion ) // conversion process will take care of writing this info
        return;

    wxXmlNode* root=seqDocument.GetRoot();

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "head")
       {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                if( element->GetName() == "author")
                {
                    SetNodeContent(element, header_info[AUTHOR]);
                }
                else if( element->GetName() == "author-email")
                {
                    SetNodeContent(element, header_info[AUTHOR_EMAIL]);
                }
                else if( element->GetName() == "author-website")
                {
                    SetNodeContent(element, header_info[WEBSITE]);
                }
                else if( element->GetName() == "song")
                {
                    SetNodeContent(element, header_info[SONG]);
                }
                else if( element->GetName() == "artist")
                {
                    SetNodeContent(element, header_info[ARTIST]);
                }
                else if( element->GetName() == "album")
                {
                    SetNodeContent(element, header_info[ALBUM]);
                }
                else if( element->GetName() == "MusicURL")
                {
                    SetNodeContent(element, header_info[URL]);
                }
                else if( element->GetName() == "comment")
                {
                    SetNodeContent(element, header_info[COMMENT]);
                }
            }
       }
    }
}

void xLightsXmlFile::Load()
{
    bool models_defined = false;

    Clear();

    wxString SeqXmlFileName=GetFullPath();

    if (!FileExists())
    {
        wxMessageBox(_("File does not exist: ")+SeqXmlFileName);
        return;
    }

    // read xml
    if (!seqDocument.Load(SeqXmlFileName))
    {
        wxMessageBox(_("Error loading: ")+SeqXmlFileName);
        return;
    }

    wxXmlNode* root=seqDocument.GetRoot();

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "tr")
       {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                if(!models_defined)
                {
                    if( models.GetCount() == 0)
                    {
                        // skip first two entries
                        element=element->GetNext();
                        element=element->GetNext();
                    }
                    models.push_back(element->GetNodeContent());
                }
                else
                {
                    timing.push_back(element->GetNodeContent());
                    timing_protection.push_back(element->GetAttribute("Protected"));
                    element=element->GetNext();
                    labels.push_back(element->GetNodeContent());
                    label_protection.push_back(element->GetAttribute("Protected"));
                    for( int i = 0; i < models.GetCount(); ++i )
                    {
                        element=element->GetNext();
                        effects.push_back(element->GetNodeContent());
                        effect_protection.push_back(element->GetAttribute("Protected"));
                    }
                }
            }
            models_defined = true;
       }
       else if(e->GetName() == "head")  // exit loop since file is new format
       {
            break;
       }
    }

    if( !models_defined )  // no <tr> tag was found
    {
        needs_conversion = false;
        for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
        {
           if (e->GetName() == "head")
           {
                for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
                {
                    if( element->GetName() == "version")
                    {
                        version_string = element->GetNodeContent();
                    }
                    else if( element->GetName() == "author")
                    {
                        header_info[AUTHOR] = element->GetNodeContent();
                    }
                    else if( element->GetName() == "author-email")
                    {
                        header_info[AUTHOR_EMAIL] = element->GetNodeContent();
                    }
                    else if( element->GetName() == "author-website")
                    {
                        header_info[WEBSITE] = element->GetNodeContent();
                    }
                    else if( element->GetName() == "song")
                    {
                        header_info[SONG] = element->GetNodeContent();
                    }
                    else if( element->GetName() == "artist")
                    {
                        header_info[ARTIST] = element->GetNodeContent();
                    }
                    else if( element->GetName() == "album")
                    {
                        header_info[ALBUM] = element->GetNodeContent();
                    }
                    else if( element->GetName() == "MusicURL")
                    {
                        header_info[URL] = element->GetNodeContent();
                    }
                    else if( element->GetName() == "comment")
                    {
                        header_info[COMMENT] = element->GetNodeContent();
                    }
                }
           }
           if (e->GetName() == "ElementEffects")
           {
                for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
                {
                    if (element->GetName() == "Element")
                    {
                        wxString attr;
                        element->GetAttribute("type", &attr);
                        if( attr == _("model"))
                        {
                            element->GetAttribute("name", &attr);
                            models.push_back(attr);
                        }
                    }
                }
           }
        }
    }
    else
    {
        version_string = _("3.x");
    }
    is_loaded = true;
}

void xLightsXmlFile::Save(wxTextCtrl* log)
{
    if( needs_conversion )
    {
        wxString new_filename = GetName() + "_v4." + GetExt();
        SetFullName(new_filename);

        log->AppendText(string_format("Saving XML file: %s\n", GetFullPath()));

        // construct the new XML file
        wxXmlDocument *doc = new wxXmlDocument();
        wxXmlNode* root=doc->GetRoot();
        root = new wxXmlNode(wxXML_ELEMENT_NODE,wxT("xsequence"));
        root->AddAttribute(wxT("BaseChannel"),wxT("0"));
        root->AddAttribute(wxT("ChanCtrlBasic"),wxT("0"));
        root->AddAttribute(wxT("ChanCtrlColor"),wxT("0"));
        doc->SetRoot(root);

        wxXmlNode* node;
        wxXmlNode* child;
        node = AddChildXmlNode(root, wxT("head"));
        AddChildXmlNode(node, wxT("version"), wxT("4.0.0"));
        AddChildXmlNode(node, wxT("author"), header_info[HEADER_INFO_TYPES::AUTHOR]);
        AddChildXmlNode(node, wxT("author-email"), header_info[HEADER_INFO_TYPES::AUTHOR_EMAIL]);
        AddChildXmlNode(node, wxT("author-website"), header_info[HEADER_INFO_TYPES::WEBSITE]);
        AddChildXmlNode(node, wxT("song"), header_info[HEADER_INFO_TYPES::SONG]);
        AddChildXmlNode(node, wxT("artist"),header_info[HEADER_INFO_TYPES::ARTIST]);
        AddChildXmlNode(node, wxT("album"), header_info[HEADER_INFO_TYPES::ALBUM]);
        AddChildXmlNode(node, wxT("MusicURL"), header_info[HEADER_INFO_TYPES::URL]);
        AddChildXmlNode(node, wxT("comment"), header_info[HEADER_INFO_TYPES::COMMENT]);

        node = AddChildXmlNode(root, wxT("DisplayElements"));
        AddTimingAttributes(node, wxT("timing"), wxT("Song Timing"), wxT("1"), wxT("1"));
        AddTimingAttributes(node, wxT("timing"), wxT("t1"), wxT("1"), wxT("0"));
        AddTimingAttributes(node, wxT("timing"), wxT("t2"), wxT("1"), wxT("0"));

        for(int i = 0; i < models.GetCount(); ++i)
        {
            child = AddChildXmlNode(node, wxT("Element"));
            child->AddAttribute(wxT("collapsed"),wxT("0"));
            child->AddAttribute(wxT("type"),wxT("model"));
            child->AddAttribute(wxT("name"),models[i]);
            child->AddAttribute(wxT("visible"),wxT("1"));
        }

        log->AppendText(string_format(wxString("Total timings = %d\n"),timing.GetCount()));

        node = AddChildXmlNode(root, wxT("ElementEffects"));
        int num_effects = timing.GetCount();
        int effect_id = 1;

        for(int i = 0; i < models.GetCount(); ++i)
        {
            log->AppendText(string_format(wxString("Processing Model = %s\n"),models[i]));
            child = AddChildXmlNode(node, wxT("Element"));
            child->AddAttribute(wxT("type"),wxT("model"));
            child->AddAttribute(wxT("name"),models[i]);
            wxXmlNode* layer1 = AddChildXmlNode(child, wxT("EffectLayer"));
            wxXmlNode* layer2 = AddChildXmlNode(child, wxT("EffectLayer"));

            for(int j = 0; j < num_effects; ++j)
            {
                int next_effect = i+(j*models.GetCount());
                wxString effect_string = effects[next_effect];
                if(effect_string.length() > 500)
                {
                    wxArrayString parts = wxSplit(effect_string, ',');
                    wxArrayString parts_copy = parts;
                    parts_copy.RemoveAt(0,2);
                    effect_string = wxJoin(parts_copy, ',');

                    int eff1_start = effect_string.find(wxString(",E1_SLIDER_Speed"));
                    int eff2_start = effect_string.find(wxString("E2_SLIDER_Speed"));

                    wxString prefix = effect_string.substr(0, eff1_start);
                    wxString eff1 = effect_string.substr(0, eff2_start-1);
                    wxString eff2 = prefix + effect_string.substr(eff2_start, effect_string.length() - eff2_start);

                    wxString data1 = SubstituteV3toV4tags(eff1);
                    wxString data2 = SubstituteV3toV4tags(eff2);

                    wxXmlNode* effect = AddChildXmlNode(layer1, wxT("Effect"), data1);
                    effect->AddAttribute(wxT("name"), parts[0]);
                    effect->AddAttribute(wxT("protected"), effect_protection[j]);
                    effect->AddAttribute(wxT("id"), string_format("%d",effect_id));
                    effect->AddAttribute(wxT("startTime"), timing[j]);
                    effect->AddAttribute(wxT("endTime"), timing[(j+1<num_effects)?j+1:j]);

                    effect = AddChildXmlNode(layer2, wxT("Effect"), data2);
                    effect->AddAttribute(wxT("name"), parts[1]);
                    effect->AddAttribute(wxT("protected"), effect_protection[j]);
                    effect->AddAttribute(wxT("id"), string_format("%d",effect_id));
                    effect->AddAttribute(wxT("startTime"), timing[j]);
                    effect->AddAttribute(wxT("endTime"), timing[(j+1<num_effects)?j+1:j]);

                    effect_id++;
                }
            }
        }

        // prune unnecessary effects
        for(wxXmlNode* e=node->GetChildren(); e!=NULL; e=e->GetNext() )
        {
            wxXmlNode* layer1 = e->GetChildren();
            if( layer1 == NULL ) break;
            wxXmlNode* layer2 = layer1->GetNext();
            if( layer2 == NULL ) break;

            wxXmlNode* layer1_effect = layer1->GetChildren();
            if( layer1_effect == NULL ) break;
            wxXmlNode* layer2_effect = layer2->GetChildren();
            if( layer2_effect == NULL ) break;
            wxXmlNode* layer1_next_effect = layer1_effect->GetNext();
            if( layer1_next_effect == NULL ) break;
            wxXmlNode* layer2_next_effect = layer2_effect->GetNext();
            if( layer2_next_effect == NULL ) break;

            wxString layer1_effect_name;
            wxString layer2_effect_name;
            wxString layer1_next_effect_name;
            wxString layer2_next_effect_name;
            wxString end_time;

            while((layer1_next_effect != NULL) && (layer2_next_effect != NULL))
            {
                layer1_effect->GetAttribute("name", &layer1_effect_name);
                layer2_effect->GetAttribute("name", &layer2_effect_name);
                layer1_next_effect->GetAttribute("name", &layer1_next_effect_name);
                layer2_next_effect->GetAttribute("name", &layer2_next_effect_name);
                wxString layer1_effect_content = layer1_effect->GetContent();
                wxString layer2_effect_content = layer2_effect->GetContent();
                wxString layer1_next_effect_content = layer1_next_effect->GetContent();
                wxString layer2_next_effect_content = layer2_next_effect->GetContent();

                if( layer1_effect_name == layer1_next_effect_name &&
                    layer1_effect_content == layer1_next_effect_content &&
                    layer2_effect_name == layer2_next_effect_name &&
                    layer2_effect_content == layer2_next_effect_content)
                {
                    // remove next effect and copy end time
                    layer1_next_effect->GetAttribute("endTime", &end_time);
                    layer1_effect->DeleteAttribute("endTime");
                    layer1_effect->AddAttribute("endTime", end_time);
                    layer1->RemoveChild(layer1_next_effect);
                    layer2->RemoveChild(layer2_next_effect);
                }
                else
                {
                    layer1_effect = layer1_next_effect;
                    layer2_effect = layer2_next_effect;
                }
                layer1_next_effect = layer1_effect->GetNext();
                layer2_next_effect = layer2_effect->GetNext();
            }
        }

        // create Song Timing elements
        child = AddChildXmlNode(node, wxT("Element"));
        child->AddAttribute(wxT("type"),wxT("timing"));
        child->AddAttribute(wxT("name"),wxT("Song Timing"));
        wxXmlNode* layer = AddChildXmlNode(child, wxT("EffectLayer"));
        for(int j = 0; j < num_effects; ++j)
        {
            wxXmlNode* effect = AddChildXmlNode(layer, wxT("Effect"));
            effect->AddAttribute(wxT("protected"), timing_protection[j]);
            effect->AddAttribute(wxT("label"), labels[j]);
            effect->AddAttribute(wxT("startTime"), timing[j]);
            effect->AddAttribute(wxT("endTime"), timing[(j+1<num_effects)?j+1:j]);
        }

        // create t1 elements
        child = AddChildXmlNode(node, wxT("Element"));
        child->AddAttribute(wxT("type"),wxT("timing"));
        child->AddAttribute(wxT("name"),wxT("t1"));
        layer = AddChildXmlNode(child, wxT("EffectLayer"));

        // create t2 elements
        child = AddChildXmlNode(node, wxT("Element"));
        child->AddAttribute(wxT("type"),wxT("timing"));
        child->AddAttribute(wxT("name"),wxT("t2"));
        layer = AddChildXmlNode(child, wxT("EffectLayer"));

        node = AddChildXmlNode(root, wxT("nextid"), string_format("%d",effect_id));

        // write converted XML file to xLights directory
        doc->Save(GetFullPath());

        version_string = latest_version;
    }
    else
    {
        GetFullPath();

        log->AppendText(string_format("Saving XML file: %s\n", GetFullPath()));

        seqDocument.Save(GetFullPath());
    }
    needs_conversion = false;
    log->AppendText(_("xLights XML saved successfully\n\n"));
}
