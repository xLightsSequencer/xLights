#include "xLightsXmlFile.h"
#include "xLightsMain.h"
#include <wx/tokenzr.h>

#define string_format wxString::Format

xLightsXmlFile::xLightsXmlFile()
{
    Clear();
    latest_version = _("4.0.0");
    SetPath( xLightsFrame::CurrentDir );
    for(int i = 0; i < NUM_TYPES; ++i )
    {
        header_info.push_back(_(""));
    }
}

xLightsXmlFile::~xLightsXmlFile()
{
    FreeMemory();
}

void xLightsXmlFile::FreeNode(wxXmlNode* node)
{
    if( node != NULL )
    {
        wxXmlNode* next_node = node->GetNext();
        if( next_node != NULL )
        {
            FreeNode(next_node);
            for(wxXmlNode* e=node->GetChildren(); e!=NULL; e=e->GetNext())
            {
                FreeNode(e);
            }
            delete node;
       }
    }
}

void xLightsXmlFile::FreeMemory()
{
    Clear();
    wxXmlNode* root=seqDocument.GetRoot();
    FreeNode(root);
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
    timing_list.Clear();
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

void xLightsXmlFile::DeleteTimingSection(wxString section)
{
    if( needs_conversion ) // conversion process will take care of writing this info
        return;

    bool found = false;
    wxXmlNode* root=seqDocument.GetRoot();

    for(wxXmlNode* e=root->GetChildren(); e!=NULL && !found; e=e->GetNext() )
    {
       if (e->GetName() == "ElementEffects")
       {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL & !found; element=element->GetNext() )
            {
                if (element->GetName() == "Element")
                {
                    wxString attr;
                    element->GetAttribute("type", &attr);
                    if( attr == _("timing"))
                    {
                        element->GetAttribute("name", &attr);
                        if( attr == section )
                        {
                            e->RemoveChild(element);
                            seqDocument.Save(GetFullPath());
                            timing_list.Remove(section);
                            delete element;
                            found = true;
                        }
                    }
                }
            }
       }
    }
}

void xLightsXmlFile::Load()
{
    bool models_defined = false;

    FreeMemory();  // always free current memory usage before a load

    for(int i = 0; i < NUM_TYPES; ++i )
    {
        header_info.push_back(_(""));
    }

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
                        else if( attr == _("timing"))
                        {
                            element->GetAttribute("name", &attr);
                            timing_list.push_back(attr);
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

void xLightsXmlFile::Save(wxTextCtrl* log, bool rename_v3_file)
{
    if( needs_conversion )
    {
        if( rename_v3_file )
        {
            wxString new_filename = GetPathWithSep() + GetName() + "_v3." + GetExt();
            if (log) log->AppendText(string_format("Renaming: %s to %s\n", GetFullPath(), new_filename));
            wxRenameFile(GetFullPath(), new_filename);
        }

        if (log) log->AppendText(string_format("Saving XML file: %s\n", GetFullPath()));

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

        if (log) log->AppendText(string_format(wxString("Total timings = %d\n"),timing.GetCount()));

        node = AddChildXmlNode(root, wxT("ElementEffects"));
        int num_effects = timing.GetCount();
        int effect_id = 1;

        for(int i = 0; i < models.GetCount(); ++i)
        {
            if (log) log->AppendText(string_format(wxString("Processing Model = %s\n"),models[i]));
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

        // connect timing gaps
        for(wxXmlNode* e=node->GetChildren(); e!=NULL; e=e->GetNext() )
        {
            for( wxXmlNode* layer=e->GetChildren(); layer!=NULL; layer=layer->GetNext() )
            {
                wxXmlNode* eff1 = layer->GetChildren();
                if( eff1 == NULL ) continue;
                wxXmlNode* eff2 = eff1->GetNext();
                if( eff2 == NULL ) continue;

                while( eff1 != NULL && eff2 != NULL )
                {
                    wxString start_time;
                    wxString end_time;
                    eff1->GetAttribute("endTime", &end_time);
                    eff2->GetAttribute("startTime", &start_time);
                    if( end_time != start_time && start_time != _("") )
                    {
                        for( wxXmlAttribute* attr=eff1->GetAttributes(); attr!=NULL; attr=attr->GetNext() )
                        {
                            if( attr->GetName() == "endTime" )
                            {
                                attr->SetValue(start_time);
                                break;
                            }
                        }
                    }
                    eff1 = eff2;
                    eff2 = eff1->GetNext();
                }
            }
        }

        // remove "None" effects
        for(wxXmlNode* e=node->GetChildren(); e!=NULL; e=e->GetNext() )
        {
            for( wxXmlNode* layer=e->GetChildren(); layer!=NULL; layer=layer->GetNext() )
            {
                for( wxXmlNode* effect=layer->GetChildren(); effect!=NULL; )
                {
                    wxString layer_effect_name;
                    effect->GetAttribute("name", &layer_effect_name);
                    if( layer_effect_name == "None" )
                    {
                        wxXmlNode* node_to_delete = effect;
                        effect = effect->GetNext();
                        layer->RemoveChild(node_to_delete);
                        delete node_to_delete;
                    }
                    else
                    {
                        effect = effect->GetNext();
                    }
                }
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

        // release memory
        FreeNode(root);
        delete doc;
        version_string = latest_version;
    }
    else
    {
        if (log) log->AppendText(string_format("Saving XML file: %s\n", GetFullPath()));
        seqDocument.Save(GetFullPath());
    }
    needs_conversion = false;
    if (log) log->AppendText(_("xLights XML saved successfully\n\n"));
}

void xLightsXmlFile::ProcessAudacityTimingFiles(const wxString& dir, const wxArrayString& filenames)
{
    wxTextFile f;
    wxString line;
    int r;

    for( int i = 0; i < filenames.Count(); ++i )
    {
        wxFileName next_file(filenames[i]);
        next_file.SetPath(dir);

        if (!f.Open(next_file.GetFullPath().c_str()))
        {
            //Add error dialog if open file failed
            return;
        }

        wxString filename = filenames[i];
        wxXmlNode* root=seqDocument.GetRoot();
        wxXmlNode* child;
        wxXmlNode* layer;

        for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
        {
            if (e->GetName() == "ElementEffects")
            {
                child = AddChildXmlNode(e, wxT("Element"));
                child->AddAttribute(wxT("type"),wxT("timing"));
                child->AddAttribute(wxT("name"),filename);
                layer = AddChildXmlNode(child, wxT("EffectLayer"));
            }
        }

        for(r=0, line = f.GetFirstLine(); !f.Eof(); line = f.GetNextLine(), r++)
        {
            std::string::size_type ofs;
            if ((ofs = line.find("#")) != std::string::npos) line.erase(ofs); //remove comments
            while (!line.empty() && (line.Last() == ' ')) line.RemoveLast(); //trim trailing spaces
            if (line.empty())
            {
                --r;    //skip blank lines; don't add grid row
                continue;
            }

            wxStringTokenizer tkz(line, "\t");
            wxString start_time = tkz.GetNextToken(); //first column = start time
            //pull in lyrics or other label text
            wxString end_time = tkz.GetNextToken(); //second column = end time;
            wxString label = tkz.GetNextToken(); //third column = label/text
            for (;;) //collect remaining tokens into label
            {
                wxString more = tkz.GetNextToken();
                if (more.empty()) break;
                label += " " + more;
            }

            child = AddChildXmlNode(layer, wxT("Effect"));
            child->AddAttribute(wxT("protected"), _("0"));
            child->AddAttribute(wxT("label"), label);
            child->AddAttribute(wxT("startTime"), start_time);
            child->AddAttribute(wxT("endTime"), end_time);
        }
        timing_list.push_back(filename);
    }

    seqDocument.Save(GetFullPath());
}

