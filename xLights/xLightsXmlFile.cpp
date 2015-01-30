#include "xLightsXmlFile.h"
#include "xLightsMain.h"
#include <wx/tokenzr.h>

#define string_format wxString::Format

static const wxString xml_dev_ver = wxT("4.0.0a");

xLightsXmlFile::xLightsXmlFile()
{
    Init();
    SetPath( xLightsFrame::CurrentDir );
}

xLightsXmlFile::xLightsXmlFile(const wxFileName &filename)
: wxFileName(filename)
{
    Init();
}

xLightsXmlFile::~xLightsXmlFile()
{
    FreeMemory();
}

void xLightsXmlFile::Init()
{
    seq_duration = 0.0;
    has_audio_media = false;
    was_converted = false;
    Clear();
    latest_version = xml_dev_ver;
    for(int i = 0; i < NUM_TYPES; ++i )
    {
        header_info.push_back(_(""));
    }
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

bool xLightsXmlFile::Convert()
{
    if( !needs_conversion ) return false;

    wxString archive_dir = xLightsFrame::CurrentDir + GetPathSeparators() + wxT("ArchiveV3");
    if( wxDir::Exists(archive_dir) == false)
    {
        if( !wxDir::Make(archive_dir) ) return false;
    }

    wxRenameFile(GetFullPath(), archive_dir + GetPathSeparators() + GetFullName());

    Save();

    was_converted = true;

    return true;
}

void xLightsXmlFile::SetSequenceType( const wxString& type )
{
    bool found = false;
    seq_type = type;
    header_info[SEQ_TYPE] = type;

    if( !is_loaded ) // conversion process will take care of writing this info
        return;

    wxXmlNode* root=seqDocument.GetRoot();
    wxXmlNode* head;

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "head")
       {
            head = e;
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                if( element->GetName() == "sequenceType")
                {
                    SetNodeContent(element, type);
                    found = true;
                }
            }
       }
    }
    if( !found )
    {
        AddChildXmlNode(head, wxT("sequenceType"), header_info[HEADER_INFO_TYPES::SEQ_TYPE]);
    }
}

void xLightsXmlFile::SetMediaFile( const wxString& filename )
{
    bool found = false;
    media_file = filename;
    header_info[MEDIA_FILE] = filename;

    if( !is_loaded ) // conversion process will take care of writing this info
        return;

    wxXmlNode* root=seqDocument.GetRoot();
    wxXmlNode* head;

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "head")
       {
            head = e;
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                if( element->GetName() == "mediaFile")
                {
                    SetNodeContent(element, filename);
                    found = true;
                }
            }
       }
    }
    if( !found )
    {
        AddChildXmlNode(head, wxT("mediaFile"), header_info[HEADER_INFO_TYPES::MEDIA_FILE]);
    }
}

static wxString GetSetting(const wxString& setting, const wxString& text)
{
    wxString settings = text;
    wxString before;
    while (!settings.IsEmpty())
    {
        before=settings.BeforeFirst(',');
        if (before.Contains(setting))
        {
            wxString val = before.AfterLast('=');
            return val;
        }
        settings=settings.AfterFirst(',');
    }
    return wxT("");
}

static wxString remapV3Value(const wxString &st) {
    if (st.Contains("SparkleFrequency")) {
        wxString val = st.AfterLast('=');
        wxString key = st.BeforeLast('=');
        int i = 200 - atoi(val);
        return key + "=" + wxString::Format("%d", i);
    }
    return st;
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


wxXmlNode* xLightsXmlFile::AddChildXmlNode(wxXmlNode* node, const wxString& node_name, const wxString& node_data)
{
    wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, node_name);
    wxXmlNode* data_node = new wxXmlNode(new_node,wxXML_TEXT_NODE,wxT(""),node_data);
    node->AddChild(new_node);
    return new_node;
}

wxXmlNode* xLightsXmlFile::AddChildXmlNode(wxXmlNode* node, const wxString& node_name)
{
    wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, node_name);
    node->AddChild(new_node);
    return new_node;
}

void xLightsXmlFile::AddTimingAttributes(wxXmlNode* node, const wxString& name, const wxString& visible, const wxString& active)
{
    // inserts the element after the last "timing" entry to keep the XML pretty
    wxXmlNode* child;
    for(child=node->GetChildren(); child!=NULL; child=child->GetNext() )
    {
        if( child->GetAttribute("type") != "timing" ) break;
    }
    wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, wxT("Element"));
    new_node->AddAttribute(wxT("type"), wxT("timing"));
    new_node->AddAttribute(wxT("name"), name);
    new_node->AddAttribute(wxT("visible"), visible);
    new_node->AddAttribute(wxT("active"), active);
    node->InsertChild(new_node, child);
}

void xLightsXmlFile::Clear()
{
    is_loaded = false;
    needs_conversion = true;
    version_string = _("");
    last_time = _("0.0");
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

void xLightsXmlFile::SetNodeContent(wxXmlNode* node, const wxString& content)
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
                else if( element->GetName() == "sequenceType")
                {
                    SetNodeContent(element, header_info[SEQ_TYPE]);
                    seq_type = header_info[SEQ_TYPE];
                }
                else if( element->GetName() == "mediaFile")
                {
                    SetNodeContent(element, header_info[MEDIA_FILE]);
                    media_file = header_info[MEDIA_FILE];
                }
                else if( element->GetName() == "sequenceDuration")
                {
                    // try to correct bad formatted length
                    header_info[SEQ_DURATION].ToDouble(&seq_duration);
                    last_time = string_format("%.3f", seq_duration);
                    SetNodeContent(element, last_time);
                    header_info[SEQ_DURATION] = last_time;
                }
            }
       }
    }
}

void xLightsXmlFile::SetTimingSectionName(wxString section, wxString name)
{
    bool found = false;
    wxXmlNode* root=seqDocument.GetRoot();

    for(wxXmlNode* e=root->GetChildren(); e!=NULL && !found; e=e->GetNext() )
    {
       if (e->GetName() == "ElementEffects")
       {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL && !found; element=element->GetNext() )
            {
                if (element->GetName() == "Element")
                {
                    wxString name_attr;
                    element->GetAttribute("type", &name_attr);
                    if( name_attr == _("timing"))
                    {
                        element->GetAttribute("name", &name_attr);
                        if( name_attr == section )
                        {
                            for( wxXmlAttribute* attr=element->GetAttributes(); attr!=NULL; attr=attr->GetNext() )
                            {
                                if( attr->GetValue() == section )
                                {
                                    attr->SetValue(name);
                                    int index = timing_list.Index(section);
                                    timing_list.Remove(section);
                                    timing_list.Insert(name, index);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (e->GetName() == "DisplayElements")
        {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL && !found; element=element->GetNext() )
            {
                if (element->GetName() == "Element")
                {
                    wxString name_attr;
                    element->GetAttribute("type", &name_attr);
                    if( name_attr == _("timing"))
                    {
                        element->GetAttribute("name", &name_attr);
                        if( name_attr == section )
                        {
                            for( wxXmlAttribute* attr=element->GetAttributes(); attr!=NULL; attr=attr->GetNext() )
                            {
                                if( attr->GetValue() == section )
                                {
                                    attr->SetValue(name);
                                    break;
                                }
                            }
                        }
                    }
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
            for(wxXmlNode* element=e->GetChildren(); element!=NULL && !found; element=element->GetNext() )
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

bool xLightsXmlFile::Load()
{
    bool success = false;
    bool models_defined = false;

    FreeMemory();  // always free current memory usage before a load

    FixVersionDifferences();

    for(int i = 0; i < NUM_TYPES; ++i )
    {
        header_info.push_back(_(""));
    }

    wxString SeqXmlFileName=GetFullPath();

    if (!FileExists())
    {
        wxMessageBox(_("File does not exist: ")+SeqXmlFileName);
        return false;
    }

    // read xml
    if (!seqDocument.Load(SeqXmlFileName))
    {
        wxMessageBox(_("Error loading: ")+SeqXmlFileName);
        return false;
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
                        // temporary reminder to developer when they need to re-convert
                        if( version_string != latest_version )
                        {
                            wxMessageBox("Your XML was converted with a previous v4 build...please re-convert original file to avoid errors!","Developer Warning");
                        }
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
                    else if( element->GetName() == "sequenceType")
                    {
                        header_info[SEQ_TYPE] = element->GetNodeContent();
                        seq_type = header_info[SEQ_TYPE];
                    }
                    else if( element->GetName() == "mediaFile")
                    {
                        header_info[MEDIA_FILE] = element->GetNodeContent();
                        media_file = header_info[MEDIA_FILE];
                    }
                    else if( element->GetName() == "sequenceDuration")
                    {
                        header_info[SEQ_DURATION] = element->GetNodeContent();
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
                            wxXmlNode* layer = element->GetChildren();
                            for(wxXmlNode* effect=layer->GetChildren(); effect!=NULL; effect=effect->GetNext() )
                            {
                                effect->GetAttribute("endTime", &attr);
                            }
                        }
                    }
                }
           }
        }
        success = true;
    }
    else
    {
        version_string = _("3.x");
    }
    is_loaded = true;
    return success;  // note that we return false for a v3 file since XML data is not populated until a Save is executed.
}

void xLightsXmlFile::StoreEndTime(wxString end_time)
{
    double time1, time2;
    end_time.ToDouble(&time1);
    last_time.ToDouble(&time2);
    if( time1 > time2 )
    {
        last_time = end_time;
    }
}

void xLightsXmlFile::SetSequenceDurationMS(int length)
{
    SetSequenceDuration(length / 1000.0f);
}

void xLightsXmlFile::SetSequenceDuration(double length)
{
    bool found = false;
    // try to correct bad formatted length
    last_time = string_format("%.3f", length);
    seq_duration = length;
    header_info[SEQ_DURATION] = last_time;

    wxXmlNode* root=seqDocument.GetRoot();
    wxXmlNode* head;

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "head")
       {
            head = e;
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                if( element->GetName() == "sequenceDuration")
                {
                    SetNodeContent(element, header_info[SEQ_DURATION]);
                    found = true;
                }
            }
       }
    }
    if( !found )
    {
        wxXmlNode* length = AddChildXmlNode(head, wxT("sequenceDuration"), header_info[SEQ_DURATION]);
    }
}


void xLightsXmlFile::Save()
{
    if( needs_conversion )
    {
        // assumes you have made a backup of the original file

        //if (log) log->AppendText(string_format("Saving XML file: %s\n", GetFullPath()));

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
        AddChildXmlNode(node, wxT("version"), xml_dev_ver);
        AddChildXmlNode(node, wxT("author"), header_info[HEADER_INFO_TYPES::AUTHOR]);
        AddChildXmlNode(node, wxT("author-email"), header_info[HEADER_INFO_TYPES::AUTHOR_EMAIL]);
        AddChildXmlNode(node, wxT("author-website"), header_info[HEADER_INFO_TYPES::WEBSITE]);
        AddChildXmlNode(node, wxT("song"), header_info[HEADER_INFO_TYPES::SONG]);
        AddChildXmlNode(node, wxT("artist"),header_info[HEADER_INFO_TYPES::ARTIST]);
        AddChildXmlNode(node, wxT("album"), header_info[HEADER_INFO_TYPES::ALBUM]);
        AddChildXmlNode(node, wxT("MusicURL"), header_info[HEADER_INFO_TYPES::URL]);
        AddChildXmlNode(node, wxT("comment"), header_info[HEADER_INFO_TYPES::COMMENT]);
        AddChildXmlNode(node, wxT("sequenceType"), header_info[HEADER_INFO_TYPES::SEQ_TYPE]);
        AddChildXmlNode(node, wxT("mediaFile"), header_info[HEADER_INFO_TYPES::MEDIA_FILE]);
        wxXmlNode* length = AddChildXmlNode(node, wxT("sequenceDuration"), header_info[HEADER_INFO_TYPES::SEQ_DURATION]);

        node = AddChildXmlNode(root, wxT("DisplayElements"));
        AddTimingAttributes(node, wxT("Imported Timing"), wxT("1"), wxT("1"));

        for(int i = 0; i < models.GetCount(); ++i)
        {
            child = AddChildXmlNode(node, wxT("Element"));
            child->AddAttribute(wxT("collapsed"),wxT("0"));
            child->AddAttribute(wxT("type"),wxT("model"));
            child->AddAttribute(wxT("name"),models[i]);
            child->AddAttribute(wxT("visible"),wxT("1"));
        }

        //if (log) log->AppendText(string_format(wxString("Total timings = %d\n"),timing.GetCount()));

        node = AddChildXmlNode(root, wxT("ElementEffects"));
        int num_effects = timing.GetCount();
        int effect_id = 1;

        for(int i = 0; i < models.GetCount(); ++i)
        {
            //if (log) log->AppendText(string_format(wxString("Processing Model = %s\n"),models[i]));
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
                    wxString settings(effect_string);
                    wxString eff1, eff2, prefix;
                    wxString effect1, effect2;
                    wxString before,after;
                    int cnt=0;
                    while (!settings.IsEmpty()) {
                        before=settings.BeforeFirst(',');
                        switch (cnt)
                        {
                        case 0:
                            effect1 = before;
                            break;
                        case 1:
                            effect2 = before;
                            break;
                        case 2:
                            prefix = before;
                            break;
                        default:
                            if (before.StartsWith("E1_")) {
                                eff1 += "," + remapV3Value(before);
                            } else if (before.StartsWith("E2_")) {
                                eff2 += "," + remapV3Value(before);
                            } else {
                                prefix += "," + remapV3Value(before);
                            }
                            break;
                        }
                        settings=settings.AfterFirst(',');
                        cnt++;
                    }

                    wxString data1 = SubstituteV3toV4tags(prefix + eff1);
                    wxString data2 = SubstituteV3toV4tags(prefix + eff2);
                    wxString end_time = timing[(j+1<num_effects)?j+1:j];

                    wxXmlNode* effect = AddChildXmlNode(layer1, wxT("Effect"), data1);
                    effect->AddAttribute(wxT("name"), effect1);
                    effect->AddAttribute(wxT("protected"), effect_protection[j]);
                    effect->AddAttribute(wxT("selected"), wxT("0"));
                    effect->AddAttribute(wxT("id"), string_format("%d",effect_id));
                    effect->AddAttribute(wxT("startTime"), timing[j]);
                    effect->AddAttribute(wxT("endTime"),end_time);

                    effect = AddChildXmlNode(layer2, wxT("Effect"), data2);
                    effect->AddAttribute(wxT("name"), effect2);
                    effect->AddAttribute(wxT("protected"), effect_protection[j]);
                    effect->AddAttribute(wxT("selected"), wxT("0"));
                    effect->AddAttribute(wxT("id"), string_format("%d",effect_id));
                    effect->AddAttribute(wxT("startTime"), timing[j]);
                    effect->AddAttribute(wxT("endTime"), end_time);

                    StoreEndTime(end_time);

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

        // remove Effect 1 and slider is 0 , then delete effect 2
        // remove Effect 2 and slider is 100 , then delete effect 1
        for(wxXmlNode* e=node->GetChildren(); e!=NULL; e=e->GetNext() )
        {
            wxXmlNode* layer1 = e->GetChildren();
            if( layer1 == NULL ) continue;
            wxXmlNode* layer2 = layer1->GetNext();
            wxXmlNode* effect1 = layer1->GetChildren();
            wxXmlNode* effect2 = layer2->GetChildren();

            while(effect1 != NULL && effect2 != NULL )
            {
                    wxString layer_effect_name;
                    effect1->GetAttribute("name", &layer_effect_name);
                // Capture next effects now in case we delete
                wxXmlNode* next_effect1 = effect1->GetNext();
                wxXmlNode* next_effect2 = effect2->GetNext();

                wxString content1 = effect1->GetNodeContent();

                wxString combine = GetSetting(wxT("T_CHOICE_LayerMethod"), content1);
                wxString morph = GetSetting(wxT("T_CHECKBOX_LayerMorph"), content1);
                wxString mix = GetSetting(wxT("T_SLIDER_EffectLayerMix"), content1);

                if( combine == "Effect 1" && morph == "0" && mix == "0" )
                {
                    layer2->RemoveChild(effect2);
                    delete effect2;
                }
                else if( combine == "Effect 2" && morph == "0" && mix == "100" )
                {
                    layer1->RemoveChild(effect1);
                    delete effect1;
                }

                effect1 = next_effect1;
                effect2 = next_effect2;
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

        // remove empty layers
        for(wxXmlNode* e=node->GetChildren(); e!=NULL; e=e->GetNext() )
        {
            for( wxXmlNode* layer=e->GetChildren(); layer!=NULL; )
            {
                wxXmlNode* child = layer->GetChildren();
                if( child == NULL )
                {
                    wxXmlNode* node_to_delete = layer;
                    layer = layer->GetNext();
                    e->RemoveChild(node_to_delete);
                    delete node_to_delete;
                }
                else
                {
                    layer = layer->GetNext();
                }
            }
        }

        // create Imported Timing elements
        child = AddChildXmlNode(node, wxT("Element"));
        child->AddAttribute(wxT("type"),wxT("timing"));
        child->AddAttribute(wxT("name"),wxT("Imported Timing"));
        wxXmlNode* layer = AddChildXmlNode(child, wxT("EffectLayer"));
        for(int j = 0; j < num_effects; ++j)
        {
            wxXmlNode* effect = AddChildXmlNode(layer, wxT("Effect"));
            effect->AddAttribute(wxT("protected"), timing_protection[j]);
            effect->AddAttribute(wxT("selected"), wxT("0"));
            effect->AddAttribute(wxT("label"), labels[j]);
            effect->AddAttribute(wxT("startTime"), timing[j]);
            effect->AddAttribute(wxT("endTime"), timing[(j+1<num_effects)?j+1:j]);
        }

        node = AddChildXmlNode(root, wxT("nextid"), string_format("%d",effect_id));

        // store off total length
        last_time.ToDouble(&seq_duration);
        SetNodeContent(length, last_time);
        header_info[HEADER_INFO_TYPES::SEQ_DURATION] = last_time;

        // write converted XML file to xLights directory
        doc->Save(GetFullPath());

        // release memory
        FreeNode(root);
        delete doc;
        version_string = latest_version;
        Load();
    }
    else
    {
        //if (log) log->AppendText(string_format("Saving XML file: %s\n", GetFullPath()));
        seqDocument.Save(GetFullPath());
    }
    needs_conversion = false;
    //if (log) log->AppendText(_("xLights XML saved successfully\n\n"));
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

        wxString filename = next_file.GetName();
        wxXmlNode* root=seqDocument.GetRoot();
        wxXmlNode* child;
        wxXmlNode* layer;

        for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
        {
            if (e->GetName() == "DisplayElements")
            {
                AddTimingAttributes(e, filename, wxT("1"), wxT("0"));
            }
            else if (e->GetName() == "ElementEffects")
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
            child->AddAttribute(wxT("selected"), wxT("0"));
            child->AddAttribute(wxT("label"), label);
            child->AddAttribute(wxT("startTime"), start_time);
            child->AddAttribute(wxT("endTime"), end_time);
        }
        timing_list.push_back(filename);
    }

    seqDocument.Save(GetFullPath());
}

void xLightsXmlFile::AddFixedTimingSection(wxString interval_name)
{
   if( needs_conversion ) // conversion process will take care of writing this info
        return;

    double interval;
    int ms = wxAtoi(interval_name);
    interval = ms / 1000;

    bool found = false;
    wxXmlNode* root=seqDocument.GetRoot();

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "DisplayElements")
        {
            AddTimingAttributes(e, interval_name, wxT("1"), wxT("0"));
            timing_list.push_back(interval_name);
        }
        else if (e->GetName() == "ElementEffects")
        {
            wxXmlNode* child;
            wxXmlNode* layer;
            wxXmlNode* effect;
            child = AddChildXmlNode(e, wxT("Element"));
            child->AddAttribute(wxT("type"),wxT("timing"));
            child->AddAttribute(wxT("name"),interval_name);
            child->AddAttribute(wxT("fixed"),string_format("%d",ms));
            layer = AddChildXmlNode(child, wxT("EffectLayer"));
        }
    }
}

wxString xLightsXmlFile::InsertMissing(wxString str,wxString missing_array,bool INSERT)
{
    int pos;
    wxStringTokenizer tkz(missing_array, "|");
    wxString replacement;
    wxString token1 = tkz.GetNextToken(); // get first two dummy tokens out
    wxString token2 = tkz.GetNextToken();
    while ( tkz.HasMoreTokens() )
    {
        token1 = tkz.GetNextToken();
        token2 = tkz.GetNextToken();
        pos=str.find(token1,0);
        replacement = "," + token2 + "</td>";
        if(pos<=0 && INSERT) // if we are INSERT mode we will add token 2 to the end of the xml string
        {
            str.Replace("</td>",replacement);
        }
        else if(pos>0 && !INSERT) // if we are in REPLACE mode (!INSERT), we replace token1 with token 2
        {
            str.Replace(token1,token2);
        }
    }
    return str;
}


// file should be full path
void xLightsXmlFile::FixVersionDifferences()
{
    wxString        str,fileout;
    wxTextFile      tfile;
    wxFile f;
    bool modified=false;
    fileout = GetFullPath() + ".out";
    //wxMessageBox(_("fixing: ")+file);
    // open the file
    wxString missing     = "xdummy|xdummy";
    wxString replace_str = "xdummy|xdummy";
    wxString Text1       = "xdummy|xdummy";
    wxString Text2       = "xdummy|xdummy";
    wxString Meteors1    = "xdummy|xdummy";
    wxString Meteors2    = "xdummy|xdummy";
    wxString Fire1       = "xdummy|xdummy";
    wxString Fire2       = "xdummy|xdummy";
    //
    //
    //  list all new tags that might have occured in previous versions
    //  list is pair. first token is what to search for, if it is missing, then put in 2nd token into xml string
    //
    missing      = missing + "|ID_SLIDER_Brightness|ID_SLIDER_Brightness=100";
    missing      = missing + "|ID_SLIDER_Contrast|ID_SLIDER_Contrast=0";

    /*  missing      = missing + "|ID_SLIDER_EffectLayerMix|ID_SLIDER_EffectLayerMix=0";
     missing      = missing + "|ID_TEXTCTRL_Effect1_Fadein|ID_TEXTCTRL_Effect1_Fadein=0";
     missing      = missing + "|ID_TEXTCTRL_Effect1_Fadeout|ID_TEXTCTRL_Effect1_Fadeout=0";
     missing      = missing + "|ID_TEXTCTRL_Effect2_Fadein|ID_TEXTCTRL_Effect2_Fadein=0";
     missing      = missing + "|ID_TEXTCTRL_Effect2_Fadeout|ID_TEXTCTRL_Effect2_Fadeout=0";
     missing      = missing + "|ID_CHECKBOX_Effect1_Fit|ID_CHECKBOX_Effect1_Fit=0";
     missing      = missing + "|ID_CHECKBOX_Effect2_Fit|ID_CHECKBOX_Effect2_Fit=0";
     */

    /*
     ID_SLIDER_EffectLayerMix=0,E1_TEXTCTRL_Fadein=0,E1_TEXTCTRL_Fadeout=0,E2_TEXTCTRL_Fadein=0
     ,E2_TEXTCTRL_Fadeout=0,E1_CHECKBOX_FitToTime=0,E2_CHECKBOX_FitToTime=0,ID_TEXTCTRL_Effect1_Fadein=0
     ,ID_TEXTCTRL_Effect1_Fadeout=0,ID_TEXTCTRL_Effect2_Fadein=0,ID_TEXTCTRL_Effect2_Fadeout=0
     ,ID_CHECKBOX_Effect1_Fit=0,ID_CHECKBOX_Effect2_Fit=0
     */
    //

    //   Meteors1 = Meteors1 + "|ID_CHECKBOX_Meteors1_FallUp|ID_CHECKBOX_Meteors1_FallUp=0";
    //    Meteors2 = Meteors2 + "|ID_CHECKBOX_Meteors2_FallUp|ID_CHECKBOX_Meteors2_FallUp=0";
    Meteors1 = Meteors1 + "|ID_CHOICE_Meteors1_Effect|ID_CHOICE_Meteors1_Effect=Meteor";
    Meteors2 = Meteors2 + "|ID_CHOICE_Meteors2_Effect|ID_CHOICE_Meteors2_Effect=Meteor";
    Meteors1 = Meteors1 + "|ID_SLIDER_Meteors1_Swirl_Intensity|ID_SLIDER_Meteors1_Swirl_Intensity=10";
    Meteors2 = Meteors2 + "|ID_SLIDER_Meteors2_Swirl_Intensity|ID_SLIDER_Meteors2_Swirl_Intensity=10";

    Fire1 = Fire1 + "|ID_SLIDER_Fire1_HueShift|ID_SLIDER_Fire1_HueShift=0";
    Fire2 = Fire2 + "|ID_SLIDER_Fire2_HueShift|ID_SLIDER_Fire2_HueShift=0";
    Fire1 = Fire1 + "|ID_CHECKBOX_Fire1_GrowFire|ID_CHECKBOX_Fire1_GrowFire=0";
    Fire2 = Fire2 + "|ID_CHECKBOX_Fire2_GrowFire|ID_CHECKBOX_Fire2_GrowFire=0";


    // Lots of variables to check for  text effect
    //    ,E1_TEXTCTRL_Text_Line1=God Bless the USA
    //	,E1_TEXTCTRL_Text_Line2=God Bless The USA
    //	,E1_TEXTCTRL_Text_Font1=
    //	,E1_CHOICE_Text_Dir1=left
    //	,E1_CHOICE_Text_Effect1=normal
    //	,E1_CHOICE_Text_Count1=none
    //	,E1_SLIDER_Text_Position1=45



    replace_str = replace_str + "|ID_BUTTON_Palette1_1|E1_BUTTON_Palette1";
    replace_str = replace_str + "|ID_BUTTON_Palette1_2|E1_BUTTON_Palette2";
    replace_str = replace_str + "|ID_BUTTON_Palette1_3|E1_BUTTON_Palette3";
    replace_str = replace_str + "|ID_BUTTON_Palette1_4|E1_BUTTON_Palette4";
    replace_str = replace_str + "|ID_BUTTON_Palette1_5|E1_BUTTON_Palette5";
    replace_str = replace_str + "|ID_BUTTON_Palette1_6|E1_BUTTON_Palette6";
    replace_str = replace_str + "|ID_BUTTON_Palette2_1|E2_BUTTON_Palette1";
    replace_str = replace_str + "|ID_BUTTON_Palette2_2|E2_BUTTON_Palette2";
    replace_str = replace_str + "|ID_BUTTON_Palette2_3|E2_BUTTON_Palette3";
    replace_str = replace_str + "|ID_BUTTON_Palette2_4|E2_BUTTON_Palette4";
    replace_str = replace_str + "|ID_BUTTON_Palette2_5|E2_BUTTON_Palette5";
    replace_str = replace_str + "|ID_BUTTON_Palette2_6|E2_BUTTON_Palette6";
    replace_str = replace_str + "|ID_CHECKBOX_Bars1_3D|E1_CHECKBOX_Bars_3D";
    replace_str = replace_str + "|ID_CHECKBOX_Bars1_Highlight|E1_CHECKBOX_Bars_Highlight";
    replace_str = replace_str + "|ID_CHECKBOX_Bars2_3D|E2_CHECKBOX_Bars_3D";
    replace_str = replace_str + "|ID_CHECKBOX_Bars2_Highlight|E2_CHECKBOX_Bars_Highlight";
    replace_str = replace_str + "|ID_CHECKBOX_Circles1_Bounce|E1_CHECKBOX_Circles_Bounce";
    replace_str = replace_str + "|ID_CHECKBOX_Circles1_Collide|E1_CHECKBOX_Circles_Collide";
    replace_str = replace_str + "|ID_CHECKBOX_Circles1_Radial|E1_CHECKBOX_Circles_Radial";
    replace_str = replace_str + "|ID_CHECKBOX_Circles1_Random_m|E1_CHECKBOX_Circles_Random_m";
    replace_str = replace_str + "|ID_CHECKBOX_Circles2_Bounce|E2_CHECKBOX_Circles_Bounce";
    replace_str = replace_str + "|ID_CHECKBOX_Circles2_Collide|E2_CHECKBOX_Circles_Collide";
    replace_str = replace_str + "|ID_CHECKBOX_Circles2_Radial|E2_CHECKBOX_Circles_Radial";
    replace_str = replace_str + "|ID_CHECKBOX_Circles2_Random_m|E2_CHECKBOX_Circles_Random_m";
    replace_str = replace_str + "|ID_CHECKBOX_ColorWash1_HFade|E1_CHECKBOX_ColorWash_HFade";
    replace_str = replace_str + "|ID_CHECKBOX_ColorWash1_VFade|E1_CHECKBOX_ColorWash_VFade";
    replace_str = replace_str + "|ID_CHECKBOX_ColorWash2_HFade|E2_CHECKBOX_ColorWash_HFade";
    replace_str = replace_str + "|ID_CHECKBOX_ColorWash2_VFade|E2_CHECKBOX_ColorWash_VFade";
    replace_str = replace_str + "|ID_CHECKBOX_Fire1_GrowFire|E1_CHECKBOX_Fire_GrowFire";
    replace_str = replace_str + "|ID_CHECKBOX_Fire2_GrowFire|E2_CHECKBOX_Fire_GrowFire";
    //    replace_str = replace_str + "|ID_CHECKBOX_Meteors1_FallUp|E1_CHECKBOX_Meteors_FallUp";
    //    replace_str = replace_str + "|ID_CHECKBOX_Meteors2_FallUp|E2_CHECKBOX_Meteors_FallUp";
    replace_str = replace_str + "|ID_CHECKBOX_Palette1_1|E1_CHECKBOX_Palette1";
    replace_str = replace_str + "|ID_CHECKBOX_Palette1_2|E1_CHECKBOX_Palette2";
    replace_str = replace_str + "|ID_CHECKBOX_Palette1_3|E1_CHECKBOX_Palette3";
    replace_str = replace_str + "|ID_CHECKBOX_Palette1_4|E1_CHECKBOX_Palette4";
    replace_str = replace_str + "|ID_CHECKBOX_Palette1_5|E1_CHECKBOX_Palette5";
    replace_str = replace_str + "|ID_CHECKBOX_Palette1_6|E1_CHECKBOX_Palette6";
    replace_str = replace_str + "|ID_CHECKBOX_Palette2_1|E2_CHECKBOX_Palette1";
    replace_str = replace_str + "|ID_CHECKBOX_Palette2_2|E2_CHECKBOX_Palette2";
    replace_str = replace_str + "|ID_CHECKBOX_Palette2_3|E2_CHECKBOX_Palette3";
    replace_str = replace_str + "|ID_CHECKBOX_Palette2_4|E2_CHECKBOX_Palette4";
    replace_str = replace_str + "|ID_CHECKBOX_Palette2_5|E2_CHECKBOX_Palette5";
    replace_str = replace_str + "|ID_CHECKBOX_Palette2_6|E2_CHECKBOX_Palette6";
    replace_str = replace_str + "|ID_CHECKBOX_Spirals1_3D|E1_CHECKBOX_Spirals_3D";
    replace_str = replace_str + "|ID_CHECKBOX_Spirals1_Blend|E1_CHECKBOX_Spirals_Blend";
    replace_str = replace_str + "|ID_CHECKBOX_Spirals2_3D|E2_CHECKBOX_Spirals_3D";
    replace_str = replace_str + "|ID_CHECKBOX_Spirals2_Blend|E2_CHECKBOX_Spirals_Blend";
    replace_str = replace_str + "|ID_CHECKBOX_Spirograph1_Animate|E1_CHECKBOX_Spirograph_Animate";
    replace_str = replace_str + "|ID_CHECKBOX_Spirograph2_Animate|E2_CHECKBOX_Spirograph_Animate";
    replace_str = replace_str + "|ID_CHECKBOX_Twinkle1_Strobe|E1_CHECKBOX_Twinkle_Strobe";
    replace_str = replace_str + "|ID_CHECKBOX_Twinkle2_Strobe|E2_CHECKBOX_Twinkle_Strobe";
    replace_str = replace_str + "|ID_CHOICE_Bars1_Direction|E1_CHOICE_Bars_Direction";
    replace_str = replace_str + "|ID_CHOICE_Bars2_Direction|E2_CHOICE_Bars_Direction";
    replace_str = replace_str + "|ID_CHOICE_Butterfly1_Colors|E1_CHOICE_Butterfly_Colors";
    replace_str = replace_str + "|ID_CHOICE_Butterfly2_Colors|E2_CHOICE_Butterfly_Colors";
    replace_str = replace_str + "|ID_CHOICE_LayerMethod|ID_CHOICE_LayerMethod";
    replace_str = replace_str + "|ID_CHOICE_Meteors1_Effect|E1_CHOICE_Meteors_Effect";
    replace_str = replace_str + "|ID_CHOICE_Meteors1_Type|E1_CHOICE_Meteors_Type";
    replace_str = replace_str + "|ID_CHOICE_Meteors2_Effect|E2_CHOICE_Meteors_Effect";
    replace_str = replace_str + "|ID_CHOICE_Meteors2_Type|E2_CHOICE_Meteors_Type";
    replace_str = replace_str + "|ID_CHOICE_Pictures1_Direction|E1_CHOICE_Pictures_Direction";
    replace_str = replace_str + "|ID_CHOICE_Pictures2_Direction|E2_CHOICE_Pictures_Direction";

    replace_str = replace_str + "|ID_SLIDER_Bars1_BarCount|E1_SLIDER_Bars_BarCount";
    replace_str = replace_str + "|ID_SLIDER_Bars2_BarCount|E2_SLIDER_Bars_BarCount";
    replace_str = replace_str + "|ID_SLIDER_Brightness|ID_SLIDER_Brightness";
    replace_str = replace_str + "|ID_SLIDER_Butterfly1_Chunks|E1_SLIDER_Butterfly_Chunks";
    replace_str = replace_str + "|ID_SLIDER_Butterfly1_Skip|E1_SLIDER_Butterfly_Skip";
    replace_str = replace_str + "|ID_SLIDER_Butterfly1_Style|E1_SLIDER_Butterfly_Style";
    replace_str = replace_str + "|ID_SLIDER_Butterfly2_Chunks|E2_SLIDER_Butterfly_Chunks";
    replace_str = replace_str + "|ID_SLIDER_Butterfly2_Skip|E2_SLIDER_Butterfly_Skip";
    replace_str = replace_str + "|ID_SLIDER_Butterfly2_Style|E2_SLIDER_Butterfly_Style";
    replace_str = replace_str + "|ID_SLIDER_Circles1_Count|E1_SLIDER_Circles_Count";
    replace_str = replace_str + "|ID_SLIDER_Circles1_Size|E1_SLIDER_Circles_Size";
    replace_str = replace_str + "|ID_SLIDER_Circles2|E2_SLIDER_Circles";
    replace_str = replace_str + "|ID_SLIDER_Circles2_count|E2_SLIDER_Circles_count";
    replace_str = replace_str + "|ID_SLIDER_ColorWash1_Count|E1_SLIDER_ColorWash_Count";
    replace_str = replace_str + "|ID_SLIDER_ColorWash2_Count|E2_SLIDER_ColorWash_Count";
    replace_str = replace_str + "|ID_SLIDER_Contrast|ID_SLIDER_Contrast";
    replace_str = replace_str + "|ID_SLIDER_Fire1_Height|E1_SLIDER_Fire_Height";
    replace_str = replace_str + "|ID_SLIDER_Fire1_HueShift|E1_SLIDER_Fire_HueShift";
    replace_str = replace_str + "|ID_SLIDER_Fire2_Height|E2_SLIDER_Fire_Height";
    replace_str = replace_str + "|ID_SLIDER_Fire2_HueShift|E2_SLIDER_Fire_HueShift";
    replace_str = replace_str + "|ID_SLIDER_Fireworks1_Count|E1_SLIDER_Fireworks_Count";
    replace_str = replace_str + "|ID_SLIDER_Fireworks1_Fade|E1_SLIDER_Fireworks_Fade";
    replace_str = replace_str + "|ID_SLIDER_Fireworks1_Number_Explosions|E1_SLIDER_Fireworks_Number_Explosions";
    replace_str = replace_str + "|ID_SLIDER_Fireworks1_Velocity|E1_SLIDER_Fireworks_Velocity";
    replace_str = replace_str + "|ID_SLIDER_Fireworks2_Count|E2_SLIDER_Fireworks_Count";
    replace_str = replace_str + "|ID_SLIDER_Fireworks2_Fade|E2_SLIDER_Fireworks_Fade";
    replace_str = replace_str + "|ID_SLIDER_Fireworks2_Number_Explosions|E2_SLIDER_Fireworks_Number_Explosions";
    replace_str = replace_str + "|ID_SLIDER_Fireworks2_Velocity|E2_SLIDER_Fireworks_Velocity";
    replace_str = replace_str + "|ID_SLIDER_Garlands1_Spacing|E1_SLIDER_Garlands_Spacing";
    replace_str = replace_str + "|ID_SLIDER_Garlands1_Type|E1_SLIDER_Garlands_Type";
    replace_str = replace_str + "|ID_SLIDER_Garlands2_Spacing|E2_SLIDER_Garlands_Spacing";
    replace_str = replace_str + "|ID_SLIDER_Garlands2_Type|E2_SLIDER_Garlands_Type";
    replace_str = replace_str + "|ID_SLIDER_Life1_Count|E1_SLIDER_Life_Count";
    replace_str = replace_str + "|ID_SLIDER_Life1_Seed|E1_SLIDER_Life_Seed";
    replace_str = replace_str + "|ID_SLIDER_Life2_Count|E2_SLIDER_Life_Count";
    replace_str = replace_str + "|ID_SLIDER_Life2_Seed|E2_SLIDER_Life_Seed";
    replace_str = replace_str + "|ID_SLIDER_Meteors1_Count|E1_SLIDER_Meteors_Count";
    replace_str = replace_str + "|ID_SLIDER_Meteors1_Length|E1_SLIDER_Meteors_Length";
    replace_str = replace_str + "|ID_SLIDER_Meteors1_Swirl_Intensity|E1_SLIDER_Meteors_Swirl_Intensity";
    replace_str = replace_str + "|ID_SLIDER_Meteors2_Count|E2_SLIDER_Meteors_Count";
    replace_str = replace_str + "|ID_SLIDER_Meteors2_Length|E2_SLIDER_Meteors_Length";
    replace_str = replace_str + "|ID_SLIDER_Meteors2_Swirl_Intensity|E2_SLIDER_Meteors_Swirl_Intensity";
    replace_str = replace_str + "|ID_SLIDER_Piano1_Keyboard|E1_SLIDER_Piano_Keyboard";
    replace_str = replace_str + "|ID_SLIDER_Piano2_Keyboard|E2_SLIDER_Piano_Keyboard";
    replace_str = replace_str + "|ID_SLIDER_Pictures1_GifSpeed|E1_SLIDER_Pictures_GifSpeed";
    replace_str = replace_str + "|ID_SLIDER_Pictures2_GifSpeed|E2_SLIDER_Pictures_GifSpeed";
    replace_str = replace_str + "|ID_SLIDER_Snowflakes1_Count|E1_SLIDER_Snowflakes_Count";
    replace_str = replace_str + "|ID_SLIDER_Snowflakes1_Type|E1_SLIDER_Snowflakes_Type";
    replace_str = replace_str + "|ID_SLIDER_Snowflakes2_Count|E2_SLIDER_Snowflakes_Count";
    replace_str = replace_str + "|ID_SLIDER_Snowflakes2_Type|E2_SLIDER_Snowflakes_Type";
    replace_str = replace_str + "|ID_SLIDER_Snowstorm1_Count|E1_SLIDER_Snowstorm_Count";
    replace_str = replace_str + "|ID_SLIDER_Snowstorm1_Length|E1_SLIDER_Snowstorm_Length";
    replace_str = replace_str + "|ID_SLIDER_Snowstorm2_Count|E2_SLIDER_Snowstorm_Count";
    replace_str = replace_str + "|ID_SLIDER_Snowstorm2_Length|E2_SLIDER_Snowstorm_Length";
    replace_str = replace_str + "|ID_SLIDER_SparkleFrequency|ID_SLIDER_SparkleFrequency";
    replace_str = replace_str + "|ID_SLIDER_Speed1|E1_SLIDER_Speed";
    replace_str = replace_str + "|ID_SLIDER_Speed2|E2_SLIDER_Speed";
    replace_str = replace_str + "|ID_SLIDER_Spirals1_Count|E1_SLIDER_Spirals_Count";
    replace_str = replace_str + "|ID_SLIDER_Spirals1_Direction|E1_SLIDER_Spirals_Direction";
    replace_str = replace_str + "|ID_SLIDER_Spirals1_Rotation|E1_SLIDER_Spirals_Rotation";
    replace_str = replace_str + "|ID_SLIDER_Spirals1_Thickness|E1_SLIDER_Spirals_Thickness";
    replace_str = replace_str + "|ID_SLIDER_Spirals2_Count|E2_SLIDER_Spirals_Count";
    replace_str = replace_str + "|ID_SLIDER_Spirals2_Direction|E2_SLIDER_Spirals_Direction";
    replace_str = replace_str + "|ID_SLIDER_Spirals2_Rotation|E2_SLIDER_Spirals_Rotation";
    replace_str = replace_str + "|ID_SLIDER_Spirals2_Thickness|E2_SLIDER_Spirals_Thickness";
    replace_str = replace_str + "|ID_SLIDER_Spirograph1_d|E1_SLIDER_Spirograph_d";
    replace_str = replace_str + "|ID_SLIDER_Spirograph1_R|E1_SLIDER_Spirograph_R";
    replace_str = replace_str + "|ID_SLIDER_Spirograph1_r|E1_SLIDER_Spirograph_r";
    replace_str = replace_str + "|ID_SLIDER_Spirograph2_d|E2_SLIDER_Spirograph_d";
    replace_str = replace_str + "|ID_SLIDER_Spirograph2_R|E2_SLIDER_Spirograph_R";
    replace_str = replace_str + "|ID_SLIDER_Spirograph2_r|E2_SLIDER_Spirograph_r";

    replace_str = replace_str + "|ID_SLIDER_Tree1_Branches|E1_SLIDER_Tree_Branches";
    replace_str = replace_str + "|ID_SLIDER_Tree2_Branches|E2_SLIDER_Tree_Branches";
    replace_str = replace_str + "|ID_SLIDER_Twinkle1_Count|E1_SLIDER_Twinkle_Count";
    replace_str = replace_str + "|ID_SLIDER_Twinkle1_Steps|E1_SLIDER_Twinkle_Steps";
    replace_str = replace_str + "|ID_SLIDER_Twinkle2_Count|E2_SLIDER_Twinkle_Count";
    replace_str = replace_str + "|ID_SLIDER_Twinkle2_Steps|E2_SLIDER_Twinkle_Steps";
    replace_str = replace_str + "|ID_TEXTCTRL_Pictures1_Filename|E1_TEXTCTRL_Pictures_Filename";
    replace_str = replace_str + "|ID_TEXTCTRL_Pictures2_Filename|E2_TEXTCTRL_Pictures_Filename";


    Text1 = Text1 + "|ID_TEXTCTRL_Text1_1_Font|ID_TEXTCTRL_Text1_1_Font=";
    Text2 = Text2 + "|ID_TEXTCTRL_Text2_1_Font|ID_TEXTCTRL_Text2_1_Font=";
    Text1 = Text1 + "|ID_TEXTCTRL_Text1_2_Font|ID_TEXTCTRL_Text1_2_Font=";
    Text2 = Text2 + "|ID_TEXTCTRL_Text2_2_Font|ID_TEXTCTRL_Text2_2_Font=";
    Text1 = Text1 + "|ID_CHOICE_Text1_1_Dir|ID_CHOICE_Text1_1_Dir=left";
    Text2 = Text2 + "|ID_CHOICE_Text2_1_Dir|ID_CHOICE_Text2_1_Dir=left";
    Text1 = Text1 + "|ID_CHOICE_Text1_2_Dir|ID_CHOICE_Text1_2_Dir=left";
    Text2 = Text2 + "|ID_CHOICE_Text2_2_Dir|ID_CHOICE_Text2_2_Dir=left";
    Text1 = Text1 + "|ID_SLIDER_Text1_1_Position|ID_SLIDER_Text1_1_Position=50";
    Text2 = Text2 + "|ID_SLIDER_Text2_1_Position|ID_SLIDER_Text2_1_Position=50";
    Text1 = Text1 + "|ID_SLIDER_Text1_2_Position|ID_SLIDER_Text1_2_Position=50";
    Text2 = Text2 + "|ID_SLIDER_Text2_2_Position|ID_SLIDER_Text2_2_Position=50";
    Text1 = Text1 + "|ID_SLIDER_Text1_1_TextRotation|ID_SLIDER_Text1_1_TextRotation=0";
    Text2 = Text2 + "|ID_SLIDER_Text2_1_TextRotation|ID_SLIDER_Text2_1_TextRotation=0";
    Text1 = Text1 + "|ID_SLIDER_Text1_2_TextRotation|ID_SLIDER_Text1_2_TextRotation=0";
    Text2 = Text2 + "|ID_SLIDER_Text2_2_TextRotation|ID_SLIDER_Text2_2_TextRotation=0";
    Text1 = Text1 + "|ID_CHECKBOX_Text1_COUNTDOWN1|ID_CHECKBOX_Text1_COUNTDOWN1=0";
    Text2 = Text2 + "|ID_CHECKBOX_Text2_COUNTDOWN1|ID_CHECKBOX_Text2_COUNTDOWN1=0";
    Text1 = Text1 + "|ID_CHECKBOX_Text1_COUNTDOWN2|ID_CHECKBOX_Text1_COUNTDOWN2=0";
    Text2 = Text2 + "|ID_CHECKBOX_Text2_COUNTDOWN2|ID_CHECKBOX_Text2_COUNTDOWN2=0";

    replace_str = replace_str + "|ID_TEXTCTRL_Text1_1_Font|E1_TEXTCTRL_Text_Font1";
    replace_str = replace_str + "|ID_TEXTCTRL_Text1_2_Font|E1_TEXTCTRL_Text_Font2";
    replace_str = replace_str + "|ID_TEXTCTRL_Text2_1_Font|E2_TEXTCTRL_Text_Font1";
    replace_str = replace_str + "|ID_TEXTCTRL_Text2_2_Font|E2_TEXTCTRL_Text_Font2";
    replace_str = replace_str + "|ID_TEXTCTRL_Text1_Line1|E1_TEXTCTRL_Text_Line1";
    replace_str = replace_str + "|ID_TEXTCTRL_Text1_Line2|E1_TEXTCTRL_Text_Line2";
    replace_str = replace_str + "|ID_TEXTCTRL_Text2_Line1|E2_TEXTCTRL_Text_Line1";
    replace_str = replace_str + "|ID_TEXTCTRL_Text2_Line2|E2_TEXTCTRL_Text_Line2";
    replace_str = replace_str + "|ID_SLIDER_Text1_1_Position|E1_SLIDER_Text_Position1";
    replace_str = replace_str + "|ID_SLIDER_Text1_2_Position|E1_SLIDER_Text_Position2";
    replace_str = replace_str + "|ID_SLIDER_Text2_1_Position|E2_SLIDER_Text_Position1";
    replace_str = replace_str + "|ID_SLIDER_Text2_2_Position|E2_SLIDER_Text_Position2";
    replace_str = replace_str + "|ID_CHOICE_Text1_1_Count|E1_CHOICE_Text_Count1";
    replace_str = replace_str + "|ID_CHOICE_Text1_2_Count|E1_CHOICE_Text_Count2";
    replace_str = replace_str + "|ID_CHOICE_Text2_1_Count|E2_CHOICE_Text_Count1";
    replace_str = replace_str + "|ID_CHOICE_Text2_2_Count|E2_CHOICE_Text_Count2";
    replace_str = replace_str + "|ID_CHOICE_Text1_1_Dir|E1_CHOICE_Text_Dir1";
    replace_str = replace_str + "|ID_CHOICE_Text1_2_Dir|E1_CHOICE_Text_Dir2";
    replace_str = replace_str + "|ID_CHOICE_Text2_1_Dir|E2_CHOICE_Text_Dir1";
    replace_str = replace_str + "|ID_CHOICE_Text2_2_Dir|E2_CHOICE_Text_Dir2";
    replace_str = replace_str + "|ID_CHOICE_Text1_1_Effect|E1_CHOICE_Text_Effect1";
    replace_str = replace_str + "|ID_CHOICE_Text1_2_Effect|E1_CHOICE_Text_Effect2";
    replace_str = replace_str + "|ID_CHOICE_Text2_1_Effect|E2_CHOICE_Text_Effect1";
    replace_str = replace_str + "|ID_CHOICE_Text2_2_Effect|E2_CHOICE_Text_Effect2";
    //
    //
    replace_str = replace_str + "|ID_CHECKBOX_Meteors1_FallUp|E1_ID_CHECKBOX_Meteors1_FallUp";
    replace_str = replace_str + "|ID_CHECKBOX_Meteors2_FallUp|E2_ID_CHECKBOX_Meteors1_FallUp";

    //  single strand effects
    replace_str = replace_str + "|E1_SLIDER_Single_Color_Mix1|ID_SLIDER_Single_Color_Mix1";
    replace_str = replace_str + "|E1_SLIDER_Single_Color_Spacing1|ID_SLIDER_Chase_Spacing1";
    replace_str = replace_str + "|E1_CHECKBOX_Single_Chase_3dFade1|ID_CHECKBOX_Chase_3dFade1";
    replace_str = replace_str + "|E2_SLIDER_Single_Color_Mix2|ID_SLIDER_Single_Color_Mix2";
    replace_str = replace_str + "|E2_SLIDER_Single_Color_Spacing2|ID_SLIDER_Chase_Spacing2";
    replace_str = replace_str + "|E2_CHECKBOX_Single_Group_Arches2|ID_CHECKBOX_Group_Arches2";

    replace_str = replace_str + "|ID_SLIDER_Single_Color_Mix1|E1_SLIDER_Color_Mix1";
    replace_str = replace_str + "|ID_SLIDER_Chase_Spacing1|E1_SLIDER_Chase_Spacing1";
    replace_str = replace_str + "|ID_CHECKBOX_Group_Arches1|E1_CHECKBOX_Chase_3dFade1";

    //  Single Strand
    replace_str = replace_str + "|E1_CHECKBOX_R_TO_L1|E1_CHOICE_Chase_Type1=Right-Left";

    //  RIPPLE
    replace_str = replace_str + "|E1_SLIDER5|E1_SLIDER_Ripple_Thickness";
    replace_str = replace_str + "|E2_SLIDER5|E2_SLIDER_Ripple_Thickness";
    replace_str = replace_str + "|E1_CHECKBOX3|E1_CHECKBOX_Ripple3D";
    replace_str = replace_str + "|E2_CHECKBOX3|E2_CHECKBOX_Ripple3D";


    /*
     E2_CHOICE_Ripple_Movement=Implode,
     E2_SLIDER_Ripple_Thickness=36,
     E2_SLIDER5=0, // ID_SLIDER_Ripple_Thickness
     E2_CHECKBOX_Ripple_Blend=1,
     E2_CHECKBOX3=1, // ID_CHECKBOX_Ripple3D
     */

    //    replace_str = replace_str + "|ID_CHECKBOX_Meteors1_FallUp|E1_CHECKBOX_Meteors_FallUp";
    //    replace_str = replace_str + "|ID_CHECKBOX_Meteors2_FallUp|E2_CHECKBOX_Meteors_FallUp";
    //

    //    E1_TEXTCTRL_Text_Font1=
    //	,E1_CHOICE_Text_Dir1=left
    //	,E1_CHOICE_Text_Effect1=normal
    //	,E1_CHOICE_Text_Count1=none
    //	,E1_SLIDER_Text_Position1=50
    //	,E1_TEXTCTRL_Text_Font2=
    //	,E1_CHOICE_Text_Dir2=left
    //	,E1_CHOICE_Text_Effect2=normal
    //	,E1_CHOICE_Text_Count2=none
    //	,E1_SLIDER_Text_Position2=50
    //	,E1_BUTTON_Palette1=#FF0000
    //	,E1_CHECKBOX_Palette1=0
    //	,E1_BUTTON_Palette2=#00FF00
    //	,E1_CHECKBOX_Palette2=0
    //	,E1_BUTTON_Palette3=#0000FF
    //	,E1_CHECKBOX_Palette3=1
    //	,E1_BUTTON_Palette4=#FFFF00
    //	,E1_CHECKBOX_Palette4=0
    //	,E1_BUTTON_Palette5=#FFFFFF
    //	,E1_CHECKBOX_Palette5=0
    //	,E1_BUTTON_Palette6=#000000
    //	,E1_CHECKBOX_Palette6=1


    //  this set will convert old, unsed tokens into a new not used token. this eliminates the error messages
    replace_str = replace_str + "|ID_SLIDER_Text1_1_TextRotation|E1_SLIDER_Text_Rotation1";
    replace_str = replace_str + "|ID_SLIDER_Text1_2_TextRotation|E1_SLIDER_Text_Rotation2";
    replace_str = replace_str + "|ID_SLIDER_Text2_1_TextRotation|E2_SLIDER_Text_Rotation1";
    replace_str = replace_str + "|ID_SLIDER_Text2_2_TextRotation|E2_SLIDER_Text_Rotation2";
    replace_str = replace_str + "|ID_CHECKBOX_Text1_COUNTDOWN1|E1_Text1_COUNTDOWN";
    replace_str = replace_str + "|ID_CHECKBOX_Text2_COUNTDOWN1|E1_Text2_COUNTDOWN";
    replace_str = replace_str + "|ID_CHECKBOX_Text1_COUNTDOWN2|E2_Text1_COUNTDOWN";
    replace_str = replace_str + "|ID_CHECKBOX_Text2_COUNTDOWN2|E2_Text2_COUNTDOWN";
    replace_str = replace_str + "|ID_SLIDER_Text_Rotation1|E1_SLIDER_Text_Rotation";
    replace_str = replace_str + "|ID_SLIDER_Text_Rotation2|E2_SLIDER_Text_Rotation";
    replace_str = replace_str + "|ID_Text1_Countdown|E1_Text_Countdown";
    replace_str = replace_str + "|ID_Text2_Countdown|E2_Text_Countdown";
    replace_str = replace_str + "|ID_Text1_COUNTDOWN|E1_Text_COUNTDOWN";
    replace_str = replace_str + "|ID_Text2_COUNTDOWN|E2_Text_COUNTDOWN";

    replace_str = replace_str + "|ID_TEXTCTRL_Effect1_Fadein|E1_TEXTCTRL_Fadein";
    replace_str = replace_str + "|ID_TEXTCTRL_Effect1_Fadeout|E1_TEXTCTRL_Fadeout";
    replace_str = replace_str + "|ID_TEXTCTRL_Effect2_Fadein|E2_TEXTCTRL_Fadein";
    replace_str = replace_str + "|ID_TEXTCTRL_Effect2_Fadeout|E2_TEXTCTRL_Fadeout";
    replace_str = replace_str + "|ID_CHECKBOX_Effect1_Fit|E1_CHECKBOX_FitToTime";
    replace_str = replace_str + "|ID_CHECKBOX_Effect2_Fit|E2_CHECKBOX_FitToTime";

    replace_str = replace_str + "|vertical text up|vert text up";
    replace_str = replace_str + "|vertical dext down|vert text down";
    replace_str = replace_str + "|count down seconds|seconds";
    replace_str = replace_str + "|count down to date|to date";

    if (!f.Create(fileout,true))
    {
        return;
    }

    tfile.Open(GetFullPath()); // open input file
    // read the first line
    str =  tfile.GetFirstLine() + "\n";


    f.Write(str);
    int pos,pos_SLIDER_Slider,pos_ID_TEXTCTRL4;

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
                str.Replace("SLIDER_Slider","SLIDER");
            }

            // do we have the old text1 font token?
            pos_ID_TEXTCTRL4=str.find("ID_TEXTCTRL4",0);
            if(pos_ID_TEXTCTRL4>0) // if we have ID_TEXTCTRL4 bad text,
            {
                modified=true;  // yes,fix it
                str.Replace("ID_TEXTCTRL4","ID_TEXTCTRL_Text1_1_Font");
            }

            //  166 tokens
            modified=true;
            str=InsertMissing(str,replace_str,false);
            str=InsertMissing(str,missing,true);

            //  now look to fill in any missing tokens

            /* comment out now with ver 25
             p=str.find("ID_SLIDER",0);
             if(p>0) // Look for lines that should have brightness and contrast, in other words all
             {
             modified=true;
             }

             p=str.find("ID_TEXTCTRL_Text1_Line1",0);
             if(p>0) // Is this a text 1 line?
             {
             modified=true;
             str=InsertMissing(str,Text1,true);
             }
             p=str.find("ID_TEXTCTRL_Text2_Line1",0);
             if(p>0) // is this a text 2 line?
             {
             modified=true;
             str=InsertMissing(str,Text2,true);
             }

             p=str.find("ID_CHOICE_Meteors1",0);
             if(p>0) // is there a meteors 1 effect on this line?
             {
             modified=true;
             str=InsertMissing(str,Meteors1,true); // fix any missing values
             }
             p=str.find("ID_CHOICE_Meteors2",0);
             if(p>0) // is there a meteors 1 effect on this line?
             {
             modified=true;
             str=InsertMissing(str,Meteors2,true);
             }

             p=str.find("ID_CHOICE_Fire11",0);
             if(p>0) // is there a meteors 1 effect on this line?
             {
             modified=true;
             str=InsertMissing(str,Fire1,true); // fix any missing values
             }
             p=str.find("ID_CHOICE_Fire2",0);
             if(p>0) // is there a meteors 1 effect on this line?
             {
             modified=true;
             str=InsertMissing(str,Fire2,true); // fix any missing values
             }
             */

        }
        str = str + "\n";
        f.Write(str); // placeholder, do whatever you want with the string

    }
    tfile.Close();
    f.Close();
    if(modified) wxCopyFile(fileout,GetFullPath(),true); // if we modified the file, copy over it
    wxRemoveFile(fileout); // get rid of temporary file
}
