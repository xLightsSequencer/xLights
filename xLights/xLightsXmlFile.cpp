#include "xLightsMain.h"
#include <wx/tokenzr.h>
#include "OptionChooser.h"
#include "../include/spxml-0.5/spxmlparser.hpp"
#include "../include/spxml-0.5/spxmlevent.hpp"
#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "xLightsXmlFile.h"
#include <wx/regex.h>
#include <wx/numdlg.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>

#define string_format wxString::Format

const wxString xLightsXmlFile::ERASE_MODE = "<rendered: erase-mode>";
const wxString xLightsXmlFile::CANVAS_MODE = "<rendered: canvas-mode>";

xLightsXmlFile::xLightsXmlFile(const wxFileName &filename)
	: wxFileName(filename),
	version_string(wxEmptyString),
	seq_duration(0.0),
	media_file(wxEmptyString),
	seq_type("Media"),
	seq_timing("50 ms"),
	image_dir(wxEmptyString),
	is_open(false),
	was_converted(false),
	sequence_loaded(false),
	audio(NULL)
{
	for(int i = 0; i < NUM_TYPES; ++i )
    {
        header_info.push_back("");
    }
    CreateNew();
}

xLightsXmlFile::~xLightsXmlFile()
{
    models.Clear();
    header_info.Clear();
    timing_list.Clear();
	if (audio != NULL)
	{
		delete audio;
		audio = NULL;
	}
}

bool xLightsXmlFile::IsXmlSequence(wxFileName &fname)
{
    char buf[1024];
    wxFile file(fname.GetFullPath());
    int i = file.Read(buf, 1024);
    file.Close();
    wxString bufs(buf, i);
    if (bufs.Contains("<xsequence")) {
        return true;
    }
    return false;
}

bool xLightsXmlFile::IsV3Sequence()
{
    char buf[1024];
    wxFile file(GetFullPath());
    int i = file.Read(buf, 1024);
    file.Close();
    if( (wxString(buf, i).Contains("<xsequence")) &&
        (wxString(buf, i).Contains("<tr>"))) {
        return true;
    }
    return false;
}

bool xLightsXmlFile::NeedsTimesCorrected()
{
    char buf[1024];
    wxFile file(GetFullPath());
    int i = file.Read(buf, 1024);
    file.Close();
    if( (wxString(buf, i).Contains("<xsequence")) &&
        (wxString(buf, i).Contains("FixedPointTiming"))) {
        return false;
    }
    return true;
}

int xLightsXmlFile::GetLastView()
{
    wxXmlNode* root=seqDocument.GetRoot();

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "lastView")
       {
            wxString last_view_string = e->GetNodeContent();
            return wxAtoi(last_view_string);
       }
    }
    return 0;
}

bool xLightsXmlFile::SaveCopy()
{
    wxString archive_dir = xLightsFrame::CurrentDir + GetPathSeparators() + "ArchiveV3";

    if( wxDir::Exists(archive_dir) == false)
    {
        if( !wxDir::Make(archive_dir) ) return false;
    }

    wxRenameFile(GetFullPath(), archive_dir + GetPathSeparators() + GetFullName());

    return true;
}

void xLightsXmlFile::SetSequenceType( const wxString& type )
{
    seq_type = type;

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
                    SetNodeContent(element, seq_type);
                }
            }
       }
    }
    if( type == "Animation" )
    {
        if (audio != NULL)
        {
            delete audio;
            audio = NULL;
        }
    }
}

int xLightsXmlFile::GetFrequency()
{
    int freq_ms = wxAtoi(seq_timing);
    return (int)(1000/freq_ms);
}

int xLightsXmlFile::GetSequenceTimingAsInt()
{
	return wxAtoi(seq_timing);
}

void xLightsXmlFile::SetSequenceTiming( const wxString& timing )
{
    seq_timing = timing;

    wxXmlNode* root=seqDocument.GetRoot();
    wxXmlNode* head;

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "head")
       {
            head = e;
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                if( element->GetName() == "sequenceTiming")
                {
                    SetNodeContent(element, seq_timing);
                }
            }
       }
    }
}

void xLightsXmlFile::SetMediaFile(const wxString& ShowDir, const wxString& filename, bool overwrite_tags )
{
    media_file = FixFile(ShowDir, filename);

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
                    SetNodeContent(element, media_file);
                }
            }
       }
    }

	if (audio != NULL)
	{
		delete audio;
		audio = NULL;
	}
	audio = new AudioManager(std::string(filename.c_str()), this, 4096, 32768);

	if( overwrite_tags )
    {
		SetMetaMP3Tags();
    }
}

void xLightsXmlFile::SetRenderMode(const wxString& mode)
{
    for(int i = 0; i < mDataLayers.GetNumLayers(); i++)
    {
        if( mDataLayers.GetDataLayer(i)->GetName() == "Nutcracker" )
        {
            mDataLayers.GetDataLayer(i)->SetDataSource(mode);
            return;
        }
    }
}

wxString xLightsXmlFile::GetRenderMode()
{
    for(int i = 0; i < mDataLayers.GetNumLayers(); i++)
    {
        if( mDataLayers.GetDataLayer(i)->GetName() == "Nutcracker" )
        {
            return mDataLayers.GetDataLayer(i)->GetDataSource();
        }
    }
    return ERASE_MODE;
}


void xLightsXmlFile::AddDisplayElement( const wxString& name, const wxString& type, const wxString& visible, const wxString& collapsed, const wxString& active )
{
    wxXmlNode* root=seqDocument.GetRoot();
    wxXmlNode* child;

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "DisplayElements")
       {
            child = AddChildXmlNode(e, "Element");
            child->AddAttribute("collapsed", collapsed);
            child->AddAttribute("active", active);
            child->AddAttribute("visible", visible);
            child->AddAttribute("type", type);
            child->AddAttribute("name", name);
            break;
       }
    }

}

void xLightsXmlFile::AddTimingDisplayElement( const wxString& name, const wxString& visible, const wxString& active )
{
    wxXmlNode* root=seqDocument.GetRoot();

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "DisplayElements")
        {
            wxXmlNode* insert_node = NULL;
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                wxString attr;
                element->GetAttribute("type", &attr);
                if( attr != "timing" )
                {
                    insert_node = element;
                    break;
                }
            }
            wxXmlNode* child;
            if( insert_node != NULL )
            {
                child = InsertChildXmlNode(e, insert_node, "Element");
            }
            else
            {
                wxXmlNode* first_child = e->GetChildren();
                if( first_child != NULL )
                {
                    child = InsertChildXmlNode(e, first_child, "Element");
                }
                else
                {
                    child = AddChildXmlNode(e, "Element");
                }
            }
            child->AddAttribute("active", active);
            child->AddAttribute("visible", visible);
            child->AddAttribute("type", "timing");
            child->AddAttribute("name", name);
            timing_list.push_back(name);
            break;
        }
    }

}
static wxString GetContent(wxXmlNode *node) {
    wxString s = node->GetContent();
    if (s.IsEmpty()) {
        node = node->GetChildren();
        while (node != NULL) {
            s = node->GetContent();
            if (!s.IsEmpty()) {
                return s;
            }
            node = node->GetNext();
        }
    }
    return s;
}

int xLightsXmlFile::AddColorPalette(StringIntMap &paletteCache, const wxString &palette) {
    int cnt = 0;
    wxXmlNode* root=seqDocument.GetRoot();
    wxXmlNode* child;

    int size = paletteCache.size();
    int i = paletteCache[palette];
    if (i != 0) {
        return i - 1;
    }

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "ColorPalettes")
        {
            cnt = size;
            AddChildXmlNode(e, "ColorPalette", palette);
            paletteCache[palette] = cnt + 1;
            return cnt;
        }
    }
    child = AddChildXmlNode(root, "ColorPalettes");
    AddChildXmlNode(child, "ColorPalette", palette);
    paletteCache[palette] = 1;
    return cnt;
}


wxXmlNode* xLightsXmlFile::AddElement( const wxString& name, const wxString& type )
{
    wxXmlNode* root=seqDocument.GetRoot();
    wxXmlNode* child = NULL;

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "ElementEffects")
       {
            child = AddChildXmlNode(e, "Element");
            child->AddAttribute("type", type);
            child->AddAttribute("name", name);
            break;
       }
    }

    return child;
}

wxXmlNode* xLightsXmlFile::AddFixedTiming( const wxString& name, const wxString& timing )
{
    wxXmlNode* root=seqDocument.GetRoot();
    wxXmlNode* child = NULL;

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "ElementEffects")
       {
            child = AddChildXmlNode(e, "Element");
            child->AddAttribute("fixed", timing);
            child->AddAttribute("type", "timing");
            child->AddAttribute("name", name);
            break;
       }
    }

    return child;
}

static wxString SplitPalette(wxString &data)
{
    wxString settings = data;
    data = "";

    wxString color;
    wxString before,after;
    while (!settings.IsEmpty()) {
        before=settings.BeforeFirst(',');
        settings=settings.AfterFirst(',');
        if (before.StartsWith("C_")) {
            color += "," + before;
        } else {
            data += "," + before;
        }
    }
    if (!color.IsEmpty()) {
        color = color.AfterFirst(',');
    }
    if (!data.IsEmpty()) {
        data = data.AfterFirst(',');
    }
    return color;
}

void xLightsXmlFile::AddEffect( wxXmlNode* node,
                                StringIntMap &paletteCache,
                                const wxString& name,
                                const wxString& d,
                                const wxString& protection,
                                const wxString& selected,
                                const wxString& id,
                                const wxString& start_time,
                                const wxString& end_time )
{
    wxString data = d;
    wxString palette = SplitPalette(data);
    int p = AddColorPalette(paletteCache, palette);
    wxXmlNode* effect = AddChildXmlNode(node, "Effect", data);
    effect->AddAttribute("name", name);
    effect->AddAttribute("protected", protection);
    effect->AddAttribute("selected", selected);
    if (id != "0") {
        effect->AddAttribute("id", id);
    }
    effect->AddAttribute("startTime", start_time);
    effect->AddAttribute("endTime", end_time);
    effect->AddAttribute("palette", string_format("%d", p));
}

void xLightsXmlFile::AddTimingEffect( wxXmlNode* node,
                                      const wxString& label,
                                      const wxString& protection,
                                      const wxString& selected,
                                      const wxString& start_time,
                                      const wxString& end_time )
{
    wxXmlNode* effect = AddChildXmlNode(node, "Effect");
    effect->AddAttribute("label", label);
    effect->AddAttribute("protected", protection);
    effect->AddAttribute("selected", selected);
    effect->AddAttribute("startTime", start_time);
    effect->AddAttribute("endTime", end_time);
}

void xLightsXmlFile::UpdateNextId( const wxString& value )
{
    wxXmlNode* root=seqDocument.GetRoot();

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "nextid")
       {
            SetNodeContent(e, value);
            break;
       }
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
    return "";
}
//deletes the setting from "text" and returns what the value was
static wxString DeleteSetting(const wxString& setting, wxString& text)
{
    wxString settings = text;
    wxString before;
    while (!settings.IsEmpty())
    {
        before=settings.BeforeFirst(',');
        if (before.Contains(setting))
        {
            wxString val = before.AfterLast('=');
            text.Replace(before, "");
            text.Replace(",,", ",");
            return val;
        }
        settings=settings.AfterFirst(',');
    }
    return "";
}
//replaces the setting from "text" and returns what the value was
static wxString ReplaceSetting(const wxString& setting, wxString& text, const wxString &newData)
{
    wxString settings = text;
    wxString before;
    while (!settings.IsEmpty())
    {
        before=settings.BeforeFirst(',');
        if (before.Contains(setting))
        {
            wxString val = before.AfterLast('=');
            text.Replace(before, setting + "=" + newData);
            return val;
        }
        settings=settings.AfterFirst(',');
    }
    return "";
}

static wxString remapV3Value(const wxString &st) {
    if (st.Contains("SparkleFrequency")) {
        wxString val = st.AfterLast('=');
        wxString key = st.BeforeLast('=');
        int i = 200 - wxAtoi(val);
        return key + "=" + wxString::Format("%d", i);
    }
    return st;
}

static wxString AdjustV3Effect(const wxString &effect, const wxString &data) {
    if (effect == "On") {
        //On effect in v4 uses the first entry of the palette.
        wxString newData(data);

        int r = wxAtoi(DeleteSetting("E_TEXTCTRL_EFF_ON_RED", newData));
        int g = wxAtoi(DeleteSetting("E_TEXTCTRL_EFF_ON_GRN", newData));
        int b = wxAtoi(DeleteSetting("E_TEXTCTRL_EFF_ON_BLU", newData));
        xlColor color(r, g, b);
        DeleteSetting("E_SLIDER_Eff_On_Red", newData);
        DeleteSetting("E_SLIDER_Eff_On_Grn", newData);
        DeleteSetting("E_SLIDER_Eff_On_Blu", newData);
        ReplaceSetting("C_BUTTON_Palette1", newData, color);
        ReplaceSetting("C_CHECKBOX_Palette1", newData, "1");
        newData += ",E_TEXTCTRL_Eff_On_Start=100,E_TEXTCTRL_Eff_On_End=100";
        return newData;
    }
    return data;
}

static wxString SubstituteV3toV4tags(const wxString& effect_string)
{
    wxString new_string = effect_string;
    new_string.Replace("ID_CHECKBOX_LayerMorph", "T_CHECKBOX_LayerMorph");
    new_string.Replace("ID_SLIDER_EffectLayerMix", "T_SLIDER_EffectLayerMix");
    new_string.Replace("E1_SLIDER_Speed", "T_SLIDER_Speed");
    new_string.Replace("E2_SLIDER_Speed", "T_SLIDER_Speed");
    new_string.Replace("E1_TEXTCTRL_Fadeout", "T_TEXTCTRL_Fadeout");
    new_string.Replace("E2_TEXTCTRL_Fadeout", "T_TEXTCTRL_Fadeout");
    new_string.Replace("E1_TEXTCTRL_Fadein", "T_TEXTCTRL_Fadein");
    new_string.Replace("E2_TEXTCTRL_Fadein", "T_TEXTCTRL_Fadein");
    new_string.Replace("E1_CHECKBOX_FitToTime", "T_CHECKBOX_FitToTime");
    new_string.Replace("E2_CHECKBOX_FitToTime", "T_CHECKBOX_FitToTime");
    new_string.Replace("E1_CHECKBOX_OverlayBkg", "T_CHECKBOX_OverlayBkg");
    new_string.Replace("E2_CHECKBOX_OverlayBkg", "T_CHECKBOX_OverlayBkg");
    new_string.Replace("ID_SLIDER_SparkleFrequency", "C_SLIDER_SparkleFrequency");
    new_string.Replace("ID_SLIDER_Brightness", "C_SLIDER_Brightness");
    new_string.Replace("ID_SLIDER_Contrast", "C_SLIDER_Contrast");
    new_string.Replace("E1_BUTTON_Palette1", "C_BUTTON_Palette1");
    new_string.Replace("E1_BUTTON_Palette2", "C_BUTTON_Palette2");
    new_string.Replace("E1_BUTTON_Palette3", "C_BUTTON_Palette3");
    new_string.Replace("E1_BUTTON_Palette4", "C_BUTTON_Palette4");
    new_string.Replace("E1_BUTTON_Palette5", "C_BUTTON_Palette5");
    new_string.Replace("E1_BUTTON_Palette6", "C_BUTTON_Palette6");
    new_string.Replace("E2_BUTTON_Palette1", "C_BUTTON_Palette1");
    new_string.Replace("E2_BUTTON_Palette2", "C_BUTTON_Palette2");
    new_string.Replace("E2_BUTTON_Palette3", "C_BUTTON_Palette3");
    new_string.Replace("E2_BUTTON_Palette4", "C_BUTTON_Palette4");
    new_string.Replace("E2_BUTTON_Palette5", "C_BUTTON_Palette5");
    new_string.Replace("E2_BUTTON_Palette6", "C_BUTTON_Palette6");
    new_string.Replace("E1_CHECKBOX_Palette1", "C_CHECKBOX_Palette1");
    new_string.Replace("E1_CHECKBOX_Palette2", "C_CHECKBOX_Palette2");
    new_string.Replace("E1_CHECKBOX_Palette3", "C_CHECKBOX_Palette3");
    new_string.Replace("E1_CHECKBOX_Palette4", "C_CHECKBOX_Palette4");
    new_string.Replace("E1_CHECKBOX_Palette5", "C_CHECKBOX_Palette5");
    new_string.Replace("E1_CHECKBOX_Palette6", "C_CHECKBOX_Palette6");
    new_string.Replace("E2_CHECKBOX_Palette1", "C_CHECKBOX_Palette1");
    new_string.Replace("E2_CHECKBOX_Palette2", "C_CHECKBOX_Palette2");
    new_string.Replace("E2_CHECKBOX_Palette3", "C_CHECKBOX_Palette3");
    new_string.Replace("E2_CHECKBOX_Palette4", "C_CHECKBOX_Palette4");
    new_string.Replace("E2_CHECKBOX_Palette5", "C_CHECKBOX_Palette5");
    new_string.Replace("E2_CHECKBOX_Palette6", "C_CHECKBOX_Palette6");
    new_string.Replace("E1_", "E_");
    new_string.Replace("E2_", "E_");

    new_string = "T_CHOICE_LayerMethod=" + new_string;

    return new_string;
}


wxXmlNode* xLightsXmlFile::AddChildXmlNode(wxXmlNode* node, const wxString& node_name, const wxString& node_data)
{
    wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, node_name);
    new wxXmlNode(new_node,wxXML_TEXT_NODE,"",node_data);
    node->AddChild(new_node);
    return new_node;
}

wxXmlNode* xLightsXmlFile::AddChildXmlNode(wxXmlNode* node, const wxString& node_name)
{
    wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, node_name);
    node->AddChild(new_node);
    return new_node;
}

wxXmlNode* xLightsXmlFile::InsertChildXmlNode(wxXmlNode* node, wxXmlNode* following_node, const wxString& node_name)
{
    wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, node_name);
    node->InsertChild(new_node, following_node);
    return new_node;
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
        new wxXmlNode(node,wxXML_TEXT_NODE,"",content);
    }
}

void xLightsXmlFile::SetHeaderInfo(HEADER_INFO_TYPES name_name, const wxString& node_value)
{
    wxXmlNode* root=seqDocument.GetRoot();

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "head")
       {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                if( element->GetName() == HEADER_STRINGS[name_name])
                {
                    SetNodeContent(element, node_value);
                    header_info[name_name] = node_value;
                }
            }
       }
    }
}

void xLightsXmlFile::SetTimingSectionName(const std::string & section, const std::string & name)
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
                    if( name_attr == "timing")
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
                    if( name_attr == "timing")
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

void xLightsXmlFile::DeleteTimingSection(const std::string & section)
{
    bool found = false;
    wxXmlNode* root=seqDocument.GetRoot();

    for(wxXmlNode* e=root->GetChildren(); e!=NULL && !found; e=e->GetNext() )
    {
        if (e->GetName() == "DisplayElements")
        {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL && !found; element=element->GetNext() )
            {
                if (element->GetName() == "Element")
                {
                    wxString attr;
                    element->GetAttribute("type", &attr);
                    if( attr == "timing")
                    {
                        element->GetAttribute("name", &attr);
                        if( attr == section )
                        {
                            e->RemoveChild(element);
                            delete element;
                            timing_list.Remove(section);
                            found = true;
                        }
                    }
                }
            }
        }
    }
    found = false;
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
                    if( attr == "timing")
                    {
                        element->GetAttribute("name", &attr);
                        if( attr == section )
                        {
                            e->RemoveChild(element);
                            delete element;
                            found = true;
                        }
                    }
                }
            }
        }
    }
}

void xLightsXmlFile::CreateNew()
{
    // construct the new XML file
    wxXmlNode* root=seqDocument.GetRoot();
    root = new wxXmlNode(wxXML_ELEMENT_NODE,"xsequence");
    root->AddAttribute("BaseChannel","0");
    root->AddAttribute("ChanCtrlBasic","0");
    root->AddAttribute("ChanCtrlColor","0");
    root->AddAttribute("FixedPointTiming","1");
    seqDocument.SetRoot(root);

    wxXmlNode* node;
    node = AddChildXmlNode(root, "head");
    AddChildXmlNode(node, "version", xlights_version_string);
    AddChildXmlNode(node, "author", header_info[AUTHOR]);
    AddChildXmlNode(node, "author-email", header_info[AUTHOR_EMAIL]);
    AddChildXmlNode(node, "author-website", header_info[WEBSITE]);
    AddChildXmlNode(node, "song", header_info[SONG]);
    AddChildXmlNode(node, "artist",header_info[ARTIST]);
    AddChildXmlNode(node, "album", header_info[ALBUM]);
    AddChildXmlNode(node, "MusicURL", header_info[URL]);
    AddChildXmlNode(node, "comment", header_info[COMMENT]);
    AddChildXmlNode(node, "sequenceTiming", seq_timing);
    AddChildXmlNode(node, "sequenceType", seq_type);
    AddChildXmlNode(node, "mediaFile", media_file);
    AddChildXmlNode(node, "sequenceDuration", GetSequenceDurationString());
    AddChildXmlNode(node, "imageDir", wxEmptyString);
    AddChildXmlNode(root, "DataLayers");
    mDataLayers.AddDataLayer("Nutcracker", "<auto-generated>", ERASE_MODE);
    AddChildXmlNode(root, "ColorPalettes");
    AddChildXmlNode(root, "DisplayElements");
    AddChildXmlNode(root, "ElementEffects");
    AddChildXmlNode(root, "nextid", "1");

    version_string = xlights_version_string;
}

bool xLightsXmlFile::Open(const wxString& ShowDir, bool ignore_audio)
{
    if( !FileExists() )
        return false;

    sequence_loaded = false;
    if( IsV3Sequence() )
    {
        return LoadV3Sequence();
    }
    else if( IsXmlSequence(*this) )
    {
        return LoadSequence(ShowDir, ignore_audio);
    }
    return false;
}

wxString xLightsXmlFile::FixFile(const wxString& ShowDir, const wxString& file)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    // This is cheating ... saves me from having every call know the showdir as long as an early one passes it in
	static wxString RememberShowDir;
	wxString sd;
	if (ShowDir == "")
	{
		sd = RememberShowDir;
	}
	else
	{
		if (ShowDir != RememberShowDir)
		{
			RememberShowDir = ShowDir;
		}
		sd = RememberShowDir;
	}

    if (file == "")
    {
        return file;
    }

	// exit if the file name is not an absolute path with a drive letter at the begginning
	if (!wxIsAbsolutePath(file) || wxFileExists(file))
	{
		return file;
	}
    
#ifndef __WXMSW__
    wxFileName fnUnix(file, wxPATH_UNIX);
    wxFileName fn3(sd, fnUnix.GetFullName());
    if (fn3.Exists()) {
        return fn3.GetFullPath();
    }
#endif
    wxFileName fnWin(file, wxPATH_WIN);
    wxFileName fn4(sd, fnWin.GetFullName());
    if (fn4.Exists()) {
        return fn4.GetFullPath();
    }
    
    
    wxString sdlc = sd;
    sdlc.LowerCase();
    wxString flc = file;
    flc.LowerCase();

    wxString path;
    wxString fname;
    wxString ext;
    wxFileName::SplitPath(sd, &path, &fname, &ext);
    wxArrayString parts = wxSplit(path, '\\', 0);
    if (fname == "")
    {
        // no subdirectory
        return file;
    }

    wxString showfolder = fname;
    wxString sflc = showfolder;
    sflc.LowerCase();

    if (flc.Contains(sflc))
    {
        int offset = flc.Find(sflc) + showfolder.Length();
        wxString relative = sd + file.SubString(offset, file.Length());

        if (wxFileExists(relative))
        {
            logger_base.debug("File location fixed: " + file + " -> " + relative);
            return relative;
        }
    }

    // Disabling this ... untested but even if it does work the scenarios are really not that useful
    //wxString fpath;
    //wxString ffname;
    //wxString fext;
    //wxFileName::SplitPath(file, &fpath, &ffname, &fext);
    //wxArrayString fparts = wxSplit(fpath, '\\', '\\');
    //if (fparts.Count() == 0)
    //{
    //	fparts = wxSplit(fpath, '/', '/');
    //}

    //int foundindex = -1;
    //int i = 0;
    //while (foundindex < 0 && i < fparts.Count())
    //{
    //	wxString fpartlc = fparts[i];
    //	fpartlc.LowerCase();

    //	if (fpartlc == sflc)
    //	{
    //		foundindex = i;
    //	}
    //	i++;
    //}

    //for (i = fparts.Count() - 1; i > foundindex; i--)
    //{
    //	wxString testfile = sd + "/";
    //	for (int j = foundindex + 1; j <= i; j++)
    //	{
    //		testfile = testfile + fparts[j] + "/";
    //	}
    //	testfile = testfile + ffname + fext;

    //	if (wxFileExists(testfile))
    //	{
    //		return testfile;
    //	}
    //}

	return file;
}

wxString xLightsXmlFile::FixEffectFileParameter(const wxString& paramname, const wxString& parametervalue, const wxString& ShowDir)
{
	int startparamname = parametervalue.Find(paramname);
	int endparamname = parametervalue.find("=", startparamname) - 1;
	int startvalue = endparamname + 2;
	int endvalue = parametervalue.find(",", startvalue) - 1;
	wxString file = parametervalue.SubString(startvalue, endvalue);
	wxString newfile = FixFile(ShowDir, file);
	wxString rc = parametervalue.Left(startvalue) + newfile + parametervalue.Right(parametervalue.Length() - endvalue - 1);
	return rc;
}

bool xLightsXmlFile::LoadV3Sequence()
{
    bool models_defined = false;
    wxArrayString timing_protection;
    wxArrayString timing;
    wxArrayString label_protection;
    wxArrayString labels;
    wxArrayString effect_protection;
    wxArrayString effects;

    FixVersionDifferences( GetFullPath() );

    // read v3 xml file into temporary document
    wxXmlDocument input_xml;
    wxString xml_doc = GetFullPath();
    if( !input_xml.Load(xml_doc) )  return false;

    wxXmlNode* input_root=input_xml.GetRoot();

    for(wxXmlNode* e=input_root->GetChildren(); e!=NULL; e=e->GetNext() )
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
                    for (size_t i = 0; i < models.GetCount(); ++i )
                    {
                        element=element->GetNext();
                        effects.push_back(element->GetNodeContent());
                        effect_protection.push_back(element->GetAttribute("Protected"));
                    }
                }
            }
            models_defined = true;
       }
    }

    if( !models_defined )  return false;  // no <tr> tag was found

    wxString end_time;
    wxString last_time = "0";
    double time1;

    // load the XML file
    AddTimingDisplayElement( "Imported Timing", "1", "1" );

    for(size_t i = 0; i < models.GetCount(); ++i)
    {
        AddDisplayElement( models[i], "model", "1", "0", "1" );
    }

    size_t num_effects = timing.GetCount();
    int effect_id = 1;
    wxXmlNode* child;
    StringIntMap paletteCache;

    for( size_t j = 0; j < num_effects; j++ )
    {
        timing[j].ToDouble(&time1);
        timing[j] = string_format("%d", (int)(time1 * 1000.0));
    }

    for(size_t i = 0; i < models.GetCount(); ++i)
    {
        child = AddElement( models[i], "model" );
        wxXmlNode* layer1 = AddChildXmlNode(child, "EffectLayer");
        wxXmlNode* layer2 = AddChildXmlNode(child, "EffectLayer");

        int time2 = wxAtoi(timing[num_effects-1]);
        time2 += 50;
        last_time = string_format("%d", time2);
        for(size_t j = 0; j < num_effects; ++j)
        {
            int next_effect = i+(j*models.GetCount());
            wxString effect_string = effects[next_effect];
            if(effect_string.length() > 100)
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
                data1 = AdjustV3Effect(effect1, data1);
                data2 = AdjustV3Effect(effect2, data2);

                wxString end_time = (j+1<num_effects) ? timing[j+1] : last_time;

                AddEffect( layer1, paletteCache, effect1, data1, effect_protection[j], "0", string_format("%d",effect_id), timing[j], end_time );
                AddEffect( layer2, paletteCache, effect2, data2, effect_protection[j], "0", string_format("%d",effect_id), timing[j], end_time );

                effect_id++;
            }
        }
    }

    if( seq_duration <= 0 )
    {
        SetSequenceDurationMS(wxAtoi(last_time));
    }

    CleanUpEffects();

    // create Imported Timing elements
    child = AddElement( "Imported Timing", "timing" );
    wxXmlNode* layer = AddChildXmlNode(child, "EffectLayer");
    for(size_t j = 0; j < num_effects; ++j)
    {
        AddTimingEffect( layer, labels[j], timing_protection[j], "0", timing[j], (j+1<num_effects) ? timing[j+1] : last_time );
    }

    UpdateNextId( string_format("%d",effect_id) );

    is_open = true;


    version_string = "4.2.19";

    timing_protection.Clear();
    timing.Clear();
    label_protection.Clear();
    labels.Clear();
    effect_protection.Clear();
    effects.Clear();

    SaveCopy();

    UpdateVersion("4.2.19");
    seqDocument.Save(GetFullPath());


    was_converted = true;

    return is_open;
}

void ConvertAllChildrenToFixedPointTiming(wxXmlNode *node) {
    while (node != nullptr) {
        if (node->GetName() == "Effect")
        {
            wxString start_time, end_time;
            int new_start_time, new_end_time;
            double t1, t2;
            node->GetAttribute("startTime", &start_time);
            node->GetAttribute("endTime", &end_time);
            start_time.ToDouble(&t1);
            end_time.ToDouble(&t2);
            new_start_time = (int)(t1 * 1000.0);
            new_end_time = (int)(t2 * 1000.0);
            node->DeleteAttribute("startTime");
            node->DeleteAttribute("endTime");
            node->AddAttribute("startTime", wxString::Format("%d", new_start_time));
            node->AddAttribute("endTime", wxString::Format("%d", new_end_time));
        } else {
            ConvertAllChildrenToFixedPointTiming(node->GetChildren());
        }
        node = node->GetNext();
    }
}
void xLightsXmlFile::ConvertToFixedPointTiming()
{
    wxXmlNode* root=seqDocument.GetRoot();

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "ElementEffects")
        {
            ConvertAllChildrenToFixedPointTiming(e->GetChildren());
        }
    }
}

bool xLightsXmlFile::LoadSequence(const wxString& ShowDir, bool ignore_audio)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Loading sequence " + GetFullPath());

	if (!seqDocument.Load(GetFullPath()))
	{
		logger_base.error("XML file load failed.");
		return false;
	}
    is_open = true;

    wxXmlNode* root=seqDocument.GetRoot();

    if( NeedsTimesCorrected() )
    {
        ConvertToFixedPointTiming();
        root->AddAttribute("FixedPointTiming","1");
    }

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
                else if( element->GetName() == "sequenceTiming")
                {
                    seq_timing = element->GetNodeContent();
                }
                else if( element->GetName() == "sequenceType")
                {
                    seq_type = element->GetNodeContent();
                }
                else if( element->GetName() == "mediaFile")
                {
                    if( !ignore_audio )
                    {
                        media_file = FixFile(ShowDir, element->GetNodeContent());
                        wxFileName mf = media_file;
                        if (audio != NULL)
                        {
                            delete audio;
                            audio = NULL;
                        }
                        if( mf.FileExists() )
                        {
                            audio = new AudioManager(std::string(media_file.c_str()), this, 4096, 32768);
                        }
                    }
                }
                else if( element->GetName() == "sequenceDuration")
                {
                    SetSequenceDuration(element->GetNodeContent());
                }
                else if( element->GetName() == "imageDir")
                {
                    image_dir = FixFile(ShowDir, element->GetNodeContent());
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
                    if( attr == "model")
                    {
                        element->GetAttribute("name", &attr);
                        models.push_back(attr);
                    }
                    else if( attr == "timing")
                    {
                        element->GetAttribute("name", &attr);
                        timing_list.push_back(attr);
                    }
                }
            }
       }
       if (e->GetName() == "DataLayers")
       {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                if (element->GetName() == "DataLayer")
                {
                    wxString name, source, data, num_frames, num_channels, channel_offset, lor_params;
                    element->GetAttribute("name", &name);
                    element->GetAttribute("source", &source);
                    element->GetAttribute("data", &data);
                    element->GetAttribute("num_frames", &num_frames);
                    element->GetAttribute("num_channels", &num_channels);
                    element->GetAttribute("channel_offset", &channel_offset);
                    element->GetAttribute("lor_params", &lor_params);
					data = FixFile("", data);
					source = FixFile("", source);
					if( name == "Nutcracker" )
                    {
                        mDataLayers.RemoveDataLayer(0);
                    }
                    DataLayer* new_data_layer = mDataLayers.AddDataLayer(name, source, data);
                    new_data_layer->SetNumFrames(atoi(num_frames.c_str()));
                    new_data_layer->SetNumChannels(atoi(num_channels.c_str()));
                    new_data_layer->SetChannelOffset(atoi(channel_offset.c_str()));
                    new_data_layer->SetLORConvertParams(atoi(lor_params.c_str()));
                }
            }
       }
    }

	if (audio != NULL)
	{
		if (audio->GetFrameInterval() < 0 && GetSequenceTimingAsInt() > 0)
		{
			audio->SetFrameInterval(GetSequenceTimingAsInt());
		}
	}

	logger_base.info("Sequence loaded.");

	return is_open;
}

void xLightsXmlFile::CleanUpEffects()
{
    wxXmlNode* root=seqDocument.GetRoot();
    wxXmlNode* node = NULL;

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "ElementEffects")
       {
            node = e;
            break;
       }
    }

    // connect timing gaps
    for(wxXmlNode* element=node->GetChildren(); element!=NULL; element=element->GetNext() )
    {
        for( wxXmlNode* layer=element->GetChildren(); layer!=NULL; layer=layer->GetNext() )
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
                if( end_time != start_time && start_time != "" )
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

            wxString combine = GetSetting("T_CHOICE_LayerMethod", content1);
            wxString morph = GetSetting("T_CHECKBOX_LayerMorph", content1);
            wxString mix = GetSetting("T_SLIDER_EffectLayerMix", content1);

            if( (combine == "Effect 1" && morph == "0" && mix == "0") ||
                (combine == "Effect 2" && morph == "0" && mix == "100"))
            {
                layer2->RemoveChild(effect2);
                delete effect2;
            }
            else if( (combine == "Effect 2" && morph == "0" && mix == "0") ||
                     (combine == "Effect 1" && morph == "0" && mix == "100"))
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
}

wxString xLightsXmlFile::GetSequenceDurationString() const
{
    return string_format("%.3f", seq_duration);
}

void xLightsXmlFile::SetSequenceDurationMS(int length)
{
    SetSequenceDuration(length / 1000.0f);
}

void xLightsXmlFile::SetSequenceDuration(double length)
{
    SetSequenceDuration(string_format("%.3f",length));
}

void xLightsXmlFile::SetSequenceDuration(const wxString& length)
{
    length.ToDouble(&seq_duration);

    wxXmlNode* root=seqDocument.GetRoot();

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "head")
       {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                if( element->GetName() == "sequenceDuration")
                {
                    SetNodeContent(element, length);
                    return;
                }
            }
       }
    }
}

wxString xLightsXmlFile::GetImageDir(wxWindow* parent)
{
    if( image_dir != wxEmptyString )
    {
        return image_dir;
    }

    wxDirDialog* dlg = new wxDirDialog(parent, _("Select Directory for storing image files for this sequence"), wxEmptyString, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));

    wxString newdir = wxEmptyString;
    if (dlg->ShowModal() != wxID_OK)
    {
        return wxEmptyString;
    }

    SetImageDir(dlg->GetPath());
    return image_dir;
}

void xLightsXmlFile::SetImageDir(const wxString& dir)
{
    image_dir = dir;

    wxXmlNode* root=seqDocument.GetRoot();

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "head")
       {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                if( element->GetName() == "imageDir")
                {
                    SetNodeContent(element, dir);
                    return;
                }
            }
            // need to create node
            AddChildXmlNode(e, "imageDir", image_dir);
       }
    }
}

void xLightsXmlFile::UpdateVersion() {
    UpdateVersion(xlights_version_string.ToStdString());
}
void xLightsXmlFile::UpdateVersion(const std::string &version)
{
    wxXmlNode* root=seqDocument.GetRoot();

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "head")
       {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                if( element->GetName() == "version")
                {
                    SetNodeContent(element, version);
                    return;
                }
            }
       }
    }
}

void xLightsXmlFile::ProcessAudacityTimingFiles(const wxString& dir, const wxArrayString& filenames, xLightsFrame* xLightsParent)
{
    wxTextFile f;
    wxString line;
    int r;

    for( size_t i = 0; i < filenames.Count(); ++i )
    {
        wxFileName next_file(filenames[i]);
        next_file.SetPath(dir);

        if (!f.Open(next_file.GetFullPath().c_str()))
        {
            //Add error dialog if open file failed
            return;
        }

        std::string filename = next_file.GetName().ToStdString();

        while( TimingAlreadyExists(filename, xLightsParent) )
        {
            filename += "_1";
        }

        Element* element = NULL;
        EffectLayer* effectLayer = NULL;
        wxXmlNode* layer = NULL;
        if( sequence_loaded )
        {
            element = xLightsParent->AddTimingElement(filename);
            effectLayer = element->GetEffectLayer(0);
        }
        else
        {
            AddTimingDisplayElement(filename, "1", "0" );
            wxXmlNode*  node = AddElement( filename, "timing" );
            layer = AddChildXmlNode(node, "EffectLayer");
        }

        wxArrayString start_times;
        wxArrayString end_times;
        std::vector<std::string> labels;

        for(r=0, line = f.GetFirstLine(); !f.Eof(); line = f.GetNextLine(), r++)
        {
            std::string::size_type ofs;
            if ((ofs = line.find("#") != std::string::npos)) line.erase(ofs); //remove comments
            while (!line.empty() && (line.Last() == ' ')) line.RemoveLast(); //trim trailing spaces
            if (line.empty())
            {
                --r;    //skip blank lines; don't add grid row
                continue;
            }

            wxStringTokenizer tkz(line, "\t");
            start_times.push_back(tkz.GetNextToken()); //first column = start time
            //pull in lyrics or other label text
            end_times.push_back(tkz.GetNextToken()); //second column = end time;
            std::string label = tkz.GetNextToken().ToStdString(); //third column = label/text
            for (;;) //collect remaining tokens into label
            {
                std::string more = tkz.GetNextToken().ToStdString();
                if (more.empty()) break;
                label += " " + more;
            }
            labels.push_back(label); //third column = label/text

        }

        double time1;
        for( size_t j = 0; j < start_times.GetCount(); j++ )
        {
            start_times[j].ToDouble(&time1);
            start_times[j] = string_format("%d", (int)(time1 * 1000.0));
            end_times[j].ToDouble(&time1);
            end_times[j] = string_format("%d", (int)(time1 * 1000.0));
        }

        int startTime, endTime;
        for( size_t k = 0; k < start_times.GetCount(); ++k )
        {
            startTime = TimeLine::RoundToMultipleOfPeriod(wxAtoi(start_times[k]),GetFrequency());
            endTime = TimeLine::RoundToMultipleOfPeriod(wxAtoi(end_times[k]),GetFrequency());
            if( startTime == endTime )
            {
                if( k == start_times.GetCount()-1 ) // last timing mark
                {
                    endTime = startTime + GetFrequency();
                }
                else
                {
                    endTime = TimeLine::RoundToMultipleOfPeriod(wxAtoi(start_times[k+1]),GetFrequency());
                }
            }

            if( sequence_loaded )
            {
                effectLayer->AddEffect(0,labels[k],"","",startTime,endTime,EFFECT_NOT_SELECTED,false);
            }
            else
            {
                AddTimingEffect(layer, labels[k], "0", "0", string_format("%d", startTime), string_format("%d", endTime));
            }
        }
    }
}

void xLightsXmlFile::ProcessLorTiming(const wxString& dir, const wxArrayString& filenames, xLightsFrame* xLightsParent)
{
    wxTextFile f;
    wxString line;
    int time;

    for (size_t i = 0; i < filenames.Count(); ++i )
    {
        wxFileName next_file(filenames[i]);
        next_file.SetPath(dir);

        if (!f.Open(next_file.GetFullPath().c_str()))
        {
            wxMessageBox("Failed to open file: " + next_file.GetFullPath());
            return;
        }

        std::string filename = next_file.GetName().ToStdString();

        wxArrayString grid_times;
        wxArrayString timing_options;

        wxXmlDocument input_xml;
        if( !input_xml.Load(next_file.GetFullPath()) )
        {
            wxMessageBox("Failed to load XML file: " + next_file.GetFullPath());
            return;
        }

        wxXmlNode* input_root=input_xml.GetRoot();

        for(wxXmlNode* e=input_root->GetChildren(); e!=NULL; e=e->GetNext() )
        {
            if (e->GetName() == "timingGrids")
            {
                for(wxXmlNode* grids=e->GetChildren(); grids!=NULL; grids=grids->GetNext() )
                {
                    if (grids->GetName() == "timingGrid")
                    {
                        wxString grid_type = grids->GetAttribute("type");
                        if( grid_type == "freeform" )
                        {
                            wxString grid_name = grids->GetAttribute("name");
                            wxString grid_id = grids->GetAttribute("saveID");
                            if( grid_name == "" )
                            {
                                grid_name = "Unnamed" + grid_id;
                            }
                            timing_options.push_back(grid_name);
                        }
                    }
                }
            }
        }

        OptionChooser opt_dialog(xLightsParent);
        opt_dialog.SetInstructionText("Choose Timing Grid to use for timing import:");
        opt_dialog.SetOptions(timing_options);
        wxArrayString timing_grids;
        if (opt_dialog.ShowModal() == wxID_OK)
        {
            opt_dialog.GetSelectedOptions(timing_grids);
        }
        else
        {
            return;
        }

        for(wxXmlNode* e=input_root->GetChildren(); e!=NULL; e=e->GetNext() )
        {
            if (e->GetName() == "timingGrids")
            {
                for(wxXmlNode* grids=e->GetChildren(); grids!=NULL; grids=grids->GetNext() )
                {
                    if (grids->GetName() == "timingGrid")
                    {
                        std::string grid_name = grids->GetAttribute("name").ToStdString();
                        std::string grid_id = grids->GetAttribute("saveID").ToStdString();
                        if( grid_name == "" )
                        {
                            grid_name = "Unnamed" + grid_id;
                        }
                        for (size_t i = 0; i < timing_grids.GetCount(); i++ )
                        {
                            if( grid_name == timing_grids[i] )
                            {
                                std::string new_timing_name = filename + ": " + grid_name;
                                Element* element = NULL;
                                EffectLayer* effectLayer = NULL;
                                wxXmlNode* layer = NULL;
                                if( sequence_loaded )
                                {
                                    element = xLightsParent->AddTimingElement(new_timing_name);
                                    effectLayer = element->GetEffectLayer(0);
                                }
                                else
                                {
                                    AddTimingDisplayElement(new_timing_name, "1", "0" );
                                    wxXmlNode*  node = AddElement( new_timing_name, "timing" );
                                    layer = AddChildXmlNode(node, "EffectLayer");
                                }

                                grid_times.Clear();
                                for(wxXmlNode* effect=grids->GetChildren(); effect!=NULL; effect=effect->GetNext() )
                                {
                                    wxString t1 = effect->GetAttribute("centisecond");
                                    time = wxAtoi(t1) * 10;
                                    t1 = string_format("%d",time);
                                    grid_times.push_back(t1);
                                }

                                // process the new timings
                                int startTime, endTime;
                                for (size_t k = 0; k < grid_times.GetCount()-1; ++k )
                                {
                                    startTime = TimeLine::RoundToMultipleOfPeriod(wxAtoi(grid_times[k]),xLightsParent->GetSequenceElements().GetFrequency());
                                    endTime = TimeLine::RoundToMultipleOfPeriod(wxAtoi(grid_times[k+1]),xLightsParent->GetSequenceElements().GetFrequency());
                                    if( sequence_loaded )
                                    {
                                        effectLayer->AddEffect(0,"","","",startTime,endTime,EFFECT_NOT_SELECTED,false);
                                    }
                                    else
                                    {
                                        AddTimingEffect(layer, "", "0", "0", string_format("%d", startTime), string_format("%d", endTime));
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

wxString xLightsXmlFile::UniqueTimingName(xLightsFrame* xLightsParent, wxString name)
{
    wxString testname = name;
    int testnamenum = 1;
    bool ok = false;
    do
    {
        ok = true;
        int num_elements = xLightsParent->GetSequenceElements().GetElementCount();
        for (int i = 0; i < num_elements; ++i)
        {
            Element* element = xLightsParent->GetSequenceElements().GetElement(i);
            if (element->GetType() == "timing")
            {
                if (element->GetName() == testname)
                {
                    testname = name + wxString::Format("_%d", testnamenum++);
                    ok = false;
                    break;
                }
            }
        }
    } while (!ok);
    return testname;
}

void xLightsXmlFile::ProcessXTiming(const wxString& dir, const wxArrayString& filenames, xLightsFrame* xLightsParent)
{
    wxTextFile f;
    wxString line;

    for (size_t i = 0; i < filenames.Count(); ++i)
    {
        wxFileName next_file(filenames[i]);
        next_file.SetPath(dir);

        if (!f.Open(next_file.GetFullPath().c_str()))
        {
            wxMessageBox("Failed to open file: " + next_file.GetFullPath());
            return;
        }

        std::string filename = next_file.GetName().ToStdString();

        wxXmlDocument input_xml;
        if (!input_xml.Load(next_file.GetFullPath()))
        {
            wxMessageBox("Failed to load XML file: " + next_file.GetFullPath());
            return;
        }

        wxXmlNode* e = input_xml.GetRoot();

        if (e->GetName() == "timing")
        {
            wxString name = e->GetAttribute("name");
            wxString v = e->GetAttribute("SourceVersion");

            name = UniqueTimingName(xLightsParent, name);

            Element* element = NULL;
            EffectLayer* effectLayer = NULL;
            wxXmlNode* layer = NULL;
            wxXmlNode* timing = NULL;
            if (sequence_loaded)
            {
                element = xLightsParent->AddTimingElement(std::string(name.c_str()));
            }
            else
            {
                AddTimingDisplayElement(name, "1", "0");
                timing = AddElement(name, "timing");
            }

            int l = 0;
            for (wxXmlNode* layers = e->GetChildren(); layers != NULL; layers = layers->GetNext())
            {
                if (layers->GetName() == "EffectLayer")
                {
                    l++;
                    if (sequence_loaded)
                    {
                        if (l == 1)
                        {
                            effectLayer = element->GetEffectLayer(0);
                        }
                        else
                        {
                            effectLayer = element->AddEffectLayer();
                        }
                    }
                    else
                    {
                        layer = AddChildXmlNode(timing, "EffectLayer");
                    }

                    for (wxXmlNode* effects = layers->GetChildren(); effects != NULL; effects = effects->GetNext())
                    {
                        if (effects->GetName() == "Effect")
                        {
                            wxString label = effects->GetAttribute("label");
                            wxString start = effects->GetAttribute("starttime");
                            wxString end = effects->GetAttribute("endtime");
                            if (sequence_loaded)
                            {
                                effectLayer->AddEffect(0, std::string(label.c_str()), "", "", wxAtoi(start), wxAtoi(end), EFFECT_NOT_SELECTED, false);
                            }
                            else
                            {
                                AddTimingEffect(layer, std::string(label.c_str()), "0", "0", start, end);
                            }
                        }
                    }
                }
            }
        }
    }
}

void xLightsXmlFile::ProcessError(const wxString& s)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.error(std::string(s.c_str()));
    wxMessageBox(s);
}

wxString RemoveTabs(const wxString& s, size_t tabs)
{
    wxString res = s;

    for (size_t i = 0; i < tabs; i++)
    {
        if (res[0] == '\t')
        {
            res = res.SubString(1, res.Length() - 1);
        }
    }
    return res;
}

void xLightsXmlFile::ProcessPapagayo(const wxString& dir, const wxArrayString& filenames, xLightsFrame* xLightsParent)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxTextFile f;
    wxString line;
    wxString desc;

    for (size_t i = 0; i < filenames.Count(); ++i)
    {
        int linenum = 1;
        wxFileName next_file(filenames[i]);
        next_file.SetPath(dir);

        logger_base.info("Loading papagayo file " + std::string(next_file.GetFullPath().c_str()));

        if (!f.Open(next_file.GetFullPath().c_str()))
        {
            ProcessError("Failed to open file: " + next_file.GetFullPath());
            return;
        }

        line = f.GetFirstLine();
        if (line.CmpNoCase("lipsync version 1"))
        {
            ProcessError(wxString::Format(_("Invalid papagayo file @line %d (header '%s')"), linenum, line.c_str()));
            return;
        }

        line = f.GetNextLine(); // filename which we ignore
        linenum++;

        wxRegEx number("^[0-9]+$");
        int samppersec = number.Matches(line = f.GetNextLine()) ? wxAtoi(line) : -1;
        linenum++;
        if (samppersec < 1)
        {
            ProcessError(wxString::Format(_("Invalid file @line %d ('%s' samples per sec)"), linenum, line.c_str()));
        }
        int ms = 1000 / samppersec;

        int maxframe = 4 * 60 * samppersec;
        if (GetMedia() != NULL)
        {
            maxframe = GetMedia()->LengthMS() / ms;
        }
        int offset = wxGetNumberFromUser("Enter the number of frames to offset the papagayo data by", "", "Offset", 0, 0, maxframe, xLightsParent);

        int numsamp = number.Matches(line = f.GetNextLine()) ? wxAtoi(line) : -1;
        linenum++;
        if (numsamp < 1)
        {
            ProcessError(wxString::Format(_("Invalid file @line %d ('%s' song samples)"), linenum, line.c_str()));
        }

        int numvoices = number.Matches(line = f.GetNextLine()) ? wxAtoi(line) : -1;
        linenum++;
        if (numvoices < 1)
        {
            ProcessError(wxString::Format(_("Invalid file @line %d ('%s' voices)"), linenum, line.c_str()));
        }
        logger_base.info("    Voices %d", numvoices);

        for (int v = 1; v <= numvoices; ++v)
        {
            wxString name = wxString::Format("Voice %d", v);
            name = UniqueTimingName(xLightsParent, name);
            logger_base.info("    Loading voice %d into timing track %s.", v, (const char *)name.c_str());

            wxString voicename = f.GetNextLine();
            linenum++;
            if (voicename.empty())
            {
                ProcessError(wxString::Format(_("Missing voice# %d of %d"), v, numvoices));
                return;
            }

            f.GetNextLine(); //all phrases for voice, "|" delimiter; TODO: do we need to save this?
            linenum++;
            desc = wxString::Format(_("voice# %d '%s' @line %d"), v, voicename, linenum);

            int numphrases = number.Matches(line = RemoveTabs(f.GetNextLine(),1)) ? wxAtoi(line) : -1;
            linenum++;
            if (numphrases < 0)
            {
                ProcessError(wxString::Format(_("Invalid file @line %d ('%s' phrases for %s)"), linenum, line.c_str(), desc.c_str()));
            }

            Element* element = NULL;
            wxXmlNode* timing = NULL, *l1 = NULL, *l2 = NULL, *l3 = NULL;
            EffectLayer *el1 = NULL, *el2 = NULL, *el3 = NULL;

            if (sequence_loaded)
            {
                element = xLightsParent->AddTimingElement(std::string(name.c_str()));
            }
            else
            {
                AddTimingDisplayElement(name, "1", "0");
                timing = AddElement(name, "timing");
            }

            if (sequence_loaded)
            {
                el1 = element->GetEffectLayer(0);
                el2 = element->AddEffectLayer();
                el3 = element->AddEffectLayer();
            }
            else
            {
                l1 = AddChildXmlNode(timing, "EffectLayer");
                l2 = AddChildXmlNode(timing, "EffectLayer");
                l3 = AddChildXmlNode(timing, "EffectLayer");
            }

            for (int p = 1; p <= numphrases; ++p)
            {
                wxString label = RemoveTabs(f.GetNextLine(), 2);
                linenum++;
                if (label == "")
                {
                    ProcessError(wxString::Format(_("Missing phrase# %d of %d for %s"), p, numphrases, desc.c_str()));
                    return;
                }

                //int start = number.Matches(line = RemoveTabs(f.GetNextLine(), 2)) ? TimeLine::RoundToMultipleOfPeriod((offset + wxAtoi(line)) * ms, GetFrequency()) : 0;
                int start = number.Matches(line = RemoveTabs(f.GetNextLine(),2)) ? (offset + wxAtoi(line)) * ms : 0;
                linenum++;

                //int end = number.Matches(line = RemoveTabs(f.GetNextLine(), 2)) ? TimeLine::RoundToMultipleOfPeriod((offset + wxAtoi(line)) * ms, GetFrequency()) : 0;
                int end = number.Matches(line = RemoveTabs(f.GetNextLine(),2)) ? (offset + wxAtoi(line)) * ms : 0;
                linenum++;
                desc = wxString::Format(_("voice# %d, phrase %d '%s', start frame %d end frame %d @line %d"), v, p, label.c_str(), start, end, linenum);

                if (sequence_loaded)
                {
                    el1->AddEffect(0, std::string(label.c_str()), "", "", start, end, EFFECT_NOT_SELECTED, false);
                }
                else
                {
                    AddTimingEffect(l1, std::string(label.c_str()), "0", "0", wxString::Format("%d", start), wxString::Format("%d", end));
                }

                int numwords = number.Matches(line = RemoveTabs(f.GetNextLine(),2)) ? wxAtoi(line) : -1;
                linenum++;
                if (numwords < 0)
                {
                    ProcessError(wxString::Format(_("Invalid file @line %d ('%s' words for %s)"), linenum, line.c_str(), desc.c_str()));
                }

                for (int w = 1; w <= numwords; ++w)
                {
                    line = RemoveTabs(f.GetNextLine(), 3);
                    linenum++;
                    int space1 = line.find(' ');
                    label = line.SubString(0, space1-1);
                    if (label == "")
                    {
                        ProcessError(wxString::Format(_("Missing word# %d of %d for %s"), w, numwords, desc.c_str()));
                        return;
                    }

                    int space2 = line.find(' ', space1 + 1);
                    wxString ss = line.SubString(space1 + 1, space2 - 1);
                    //start = number.Matches(ss) ? TimeLine::RoundToMultipleOfPeriod((offset + wxAtoi(ss)) * ms, GetFrequency()) : 0;
                    start = number.Matches(ss) ? (offset + wxAtoi(ss)) * ms : 0;
                    linenum++;

                    int space3 = line.find(' ', space2 + 1);
                    ss = line.SubString(space2 + 1, space3 - 1);
                    //end = number.Matches(ss) ? TimeLine::RoundToMultipleOfPeriod((offset + wxAtoi(ss)) * ms, GetFrequency()) : 0;
                    end = number.Matches(ss) ? (offset + wxAtoi(ss)) * ms : 0;
                    linenum++;
                    desc = wxString::Format(_("voice# %d, phrase# %d, word %d '%s', start frame %d end frame %d @line %d"), v, p, w, label.c_str(), start, end, linenum);

                    if (sequence_loaded)
                    {
                        el2->AddEffect(0, std::string(label.c_str()), "", "", start, end, EFFECT_NOT_SELECTED, false);
                    }
                    else
                    {
                        AddTimingEffect(l2, std::string(label.c_str()), "0", "0", wxString::Format("%d", start), wxString::Format("%d", end));
                    }

                    ss = line.SubString(space3 + 1, line.Length());
                    int numphonemes = number.Matches(ss) ? wxAtoi(ss) : -1;
                    linenum++;
                    if (numphonemes < 0)
                    {
                        ProcessError(wxString::Format(_("Invalid file @line %d ('%s' phonemes for %s)"), linenum, line.c_str(), desc.c_str()));
                    }

                    int outerend = end;
                    for (int ph = 1; ph <= numphonemes; ++ph)
                    {
                        line = RemoveTabs(f.GetNextLine(), 4);
                        linenum++;
                        int space1 = line.find(' ');

                        ss = line.SubString(0, space1 - 1);
                        //end = number.Matches(ss) ? TimeLine::RoundToMultipleOfPeriod((offset + wxAtoi(ss)) * ms, GetFrequency()) : 0;
                        end = number.Matches(ss) ? (offset + wxAtoi(ss)) * ms : 0;
                        linenum++;

                        if (ph == 1)
                        {
                            // dont do anything
                        }
                        else
                        {
                            if (sequence_loaded)
                            {
                                el3->AddEffect(0, std::string(label.c_str()), "", "", start, end, EFFECT_NOT_SELECTED, false);
                            }
                            else
                            {
                                AddTimingEffect(l3, std::string(label.c_str()), "0", "0", wxString::Format("%d", start), wxString::Format("%d", end));
                            }
                        }
                        label = line.SubString(space1 + 1, line.Length());
                        if (label == "")
                        {
                            ProcessError(wxString::Format(_("Missing phoneme# %d of %d for %s"), ph, numphonemes, desc.c_str()));
                            return;
                        }
                        start = end;

                        if (ph == numphonemes)
                        {
                            end = outerend;
                            if (sequence_loaded)
                            {
                                el3->AddEffect(0, std::string(label.c_str()), "", "", start, end, EFFECT_NOT_SELECTED, false);
                            }
                            else
                            {
                                AddTimingEffect(l3, std::string(label.c_str()), "0", "0", wxString::Format("%d", start), wxString::Format("%d", end));
                            }
                        }
                    }
                }
            }
        }
    }
}

wxString DecodeLSPTTColour(int att)
{
    switch (att)
    {
    case 1:
        return "x";
    case 2: 
        return "R";
    case 4:
        return "G";
    case 8:
        return "B";
    case 16:
        return "Y";
    case 32:
        return "P";
    case 64:
        return "O";
    case 128:
        return "z";
    case 256:
        return "Go";
    case 512:
        return "W";
    case 1024:
        return "System";
    }

    return wxString::Format("%d", att);
}

void xLightsXmlFile::ProcessLSPTiming(const wxString& dir, const wxArrayString& filenames, xLightsFrame* xLightsParent)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxTextFile f;
    wxString line;
    wxString desc;

    xLightsParent->SetCursor(wxCURSOR_WAIT);

    for (size_t i = 0; i < filenames.Count(); ++i)
    {
        wxFileName next_file(filenames[i]);
        next_file.SetPath(dir);

        logger_base.info("Decompressing LSP file " + std::string(next_file.GetFullPath().c_str()));

        wxFileInputStream fin(next_file.GetFullPath());
        wxZipInputStream zin(fin);
        wxZipEntry *ent = zin.GetNextEntry();

        wxXmlDocument seq_xml;

        while (ent != nullptr)
        {
            if (ent->GetName() == "Sequence")
            {
                logger_base.info("Extracting timing tracks from " + std::string(next_file.GetFullPath().c_str()) + "/" + std::string(ent->GetName().c_str()));
                seq_xml.Load(zin);

                wxXmlNode* e = seq_xml.GetRoot();

                if (e->GetName() == "MusicalSequence")
                {
                    for (wxXmlNode* tts = e->GetChildren(); tts != NULL; tts = tts->GetNext())
                    {
                        if (tts->GetName() == "TimingTracks")
                        {
                            for (wxXmlNode* t = tts->GetChildren(); t != NULL; t = t->GetNext())
                            {
                                if (t->GetName() == "Track") {
                                    wxString name = UniqueTimingName(xLightsParent, next_file.GetName());
                                    logger_base.info("  Track: " + std::string(name.c_str()));
                                    Element* element = NULL;
                                    EffectLayer* effectLayer = NULL;
                                    wxXmlNode* layer = NULL;
                                    wxXmlNode* timing = NULL;
                                    int present = 0;
                                    for (wxXmlNode* is = t->GetChildren(); is != NULL; is = is->GetNext()) {
                                        if (is->GetName() == "Intervals") {
                                            std::list<wxString> atts;
                                            for (wxXmlNode* ti = is->GetChildren(); ti != NULL; ti = ti->GetNext()) {
                                                if (ti->GetName() == "TimeInterval") {
                                                    if (ti->GetAttribute("eff") == "7") {
                                                        present |= wxAtoi(ti->GetAttribute("att"));
                                                    }
                                                }
                                            }

                                            int mask = 1;
                                            for (size_t i = 0; i < 10; i++) {
                                                if (present & mask) {
                                                    wxString tname = DecodeLSPTTColour(mask) + "-" + name;
                                                    logger_base.info("  Adding timing track " + std::string(tname.c_str()) + "(" + std::string(wxString::Format("%d",mask).c_str()) + ")");
                                                    if (sequence_loaded) {
                                                        element = xLightsParent->AddTimingElement(std::string(tname.c_str()));
                                                        effectLayer = element->GetEffectLayer(0);
                                                    }
                                                    else {
                                                        AddTimingDisplayElement(tname, "1", "0");
                                                        timing = AddElement(tname, "timing");
                                                        layer = AddChildXmlNode(timing, "EffectLayer");
                                                    }

                                                    int last = 0;
                                                    bool sevenfound = false;
                                                    bool fourfound = false;
                                                    for (wxXmlNode* ti = is->GetChildren(); ti != NULL; ti = ti->GetNext()) {
                                                        if (ti->GetName() == "TimeInterval") {
                                                            if (ti->GetAttribute("eff") == "7" && (wxAtoi(ti->GetAttribute("att")) & mask)) {
                                                                sevenfound = true;
                                                                int start = last;
                                                                int end = TimeLine::RoundToMultipleOfPeriod((int)(wxAtof(ti->GetAttribute("pos")) * 50.0 / 4410.0), GetFrequency());
                                                                if (start != end)
                                                                {
                                                                    wxString label = "";
                                                                    if (sequence_loaded) {
                                                                        effectLayer->AddEffect(0, std::string(label.c_str()), "", "", start, end, EFFECT_NOT_SELECTED, false);
                                                                    }
                                                                    else {
                                                                        AddTimingEffect(layer, std::string(label.c_str()), "0", "0", wxString::Format("%d", start), wxString::Format("%d", end));
                                                                    }
                                                                    last = end;
                                                                }
                                                            }
                                                            // we take the only the first 4 after we have found 7s
                                                            else if (ti->GetAttribute("eff") == "4" && sevenfound && !fourfound && (wxAtoi(ti->GetAttribute("att")) & mask)) {
                                                                fourfound = true;
                                                                int start = last;
                                                                int end = TimeLine::RoundToMultipleOfPeriod((int)(wxAtof(ti->GetAttribute("pos")) * 50.0 / 4410.0), GetFrequency());
                                                                if (start != end)
                                                                {
                                                                    wxString label = "";
                                                                    if (sequence_loaded) {
                                                                        effectLayer->AddEffect(0, std::string(label.c_str()), "", "", start, end, EFFECT_NOT_SELECTED, false);
                                                                    }
                                                                    else {
                                                                        AddTimingEffect(layer, std::string(label.c_str()), "0", "0", wxString::Format("%d", start), wxString::Format("%d", end));
                                                                    }
                                                                    last = end;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                mask = mask << 1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ent = zin.GetNextEntry();
        }
    }
    xLightsParent->SetCursor(wxCURSOR_ARROW);
}

wxArrayString xLightsXmlFile::GetTimingList(SequenceElements& seq_elements)
{
    timing_list.Clear();
    int num_elements = seq_elements.GetElementCount();
    for(int i = 0; i < num_elements; ++i)
    {
        Element* element = seq_elements.GetElement(i);
        if( element->GetType() == "timing" )
        {
            timing_list.push_back(element->GetName());
        }
    }
    return timing_list;
}

bool xLightsXmlFile::Save()
{
    UpdateVersion();
    return seqDocument.Save(GetFullPath());
}

void xLightsXmlFile::WriteEffects(EffectLayer *layer,
                                  wxXmlNode *effect_layer_node,
                                  StringIntMap &colorPalettes,
                                  wxXmlNode* colorPalette_node,
                                  StringIntMap &effectStrings,
                                  wxXmlNode* effectDB_Node) {
    int num_effects = layer->GetEffectCount();
    for(int k = 0; k < num_effects; ++k)
    {
        Effect* effect = layer->GetEffect(k);
        wxString effectString = effect->GetSettingsAsString();
        int size = effectStrings.size();
        int ref = effectStrings[effectString] - 1;
        if (ref == -1) {
            ref = size;
            effectStrings[effectString] = ref + 1;
            AddChildXmlNode(effectDB_Node, "Effect", effectString);
        }


        // Add effect node
        wxXmlNode* effect_node = AddChildXmlNode(effect_layer_node, "Effect");
        effect_node->AddAttribute("ref", string_format("%d", ref));
        effect_node->AddAttribute("name", effect->GetEffectName());
        if (effect->GetProtected()) {
            effect_node->AddAttribute("protected", "1");
        }
        if (effect->GetSelected()) {
            effect_node->AddAttribute("selected", "1");
        }
        if (effect->GetID()) {
            effect_node->AddAttribute("id", string_format("%d", effect->GetID()));
        }
        effect_node->AddAttribute("startTime", string_format("%d", effect->GetStartTimeMS()));
        effect_node->AddAttribute("endTime", string_format("%d", effect->GetEndTimeMS()));
        wxString palette = effect->GetPaletteAsString();
        if (palette != "") {
            size = colorPalettes.size();
            int pref = colorPalettes[palette] - 1;
            if (pref == -1) {
                pref = size;
                colorPalettes[palette] = pref + 1;
                AddChildXmlNode(colorPalette_node, "ColorPalette", palette);
            }
            effect_node->AddAttribute("palette", string_format("%d", pref));
        }
    }
}

// function used to save sequence data
void xLightsXmlFile::Save( SequenceElements& seq_elements)
{
    wxXmlNode* root = seqDocument.GetRoot();

    // Delete nodes that will be replaced
    for(wxXmlNode* e=root->GetChildren(); e!=NULL; )
    {
        if( e->GetName() == "DisplayElements" ||
            e->GetName() == "ElementEffects"  ||
            e->GetName() == "DataLayers" ||
            e->GetName() == "ColorPalettes" ||
            e->GetName() == "EffectDB" ||
            e->GetName() == "lastView")
        {
            wxXmlNode* node_to_delete = e;
            e = e->GetNext();
            root->RemoveChild(node_to_delete);
            delete node_to_delete;
        }
        else
        {
            e=e->GetNext();
        }
    }

    StringIntMap colorPalettes;
    wxXmlNode* colorPalette_node = AddChildXmlNode(root, "ColorPalettes");
    StringIntMap effectStrings;
    wxXmlNode* effectDB_Node = AddChildXmlNode(root, "EffectDB");

    // Now add new elements to our xml document
    wxXmlNode* data_layer = AddChildXmlNode(root, "DataLayers");
    wxXmlNode* display_node = AddChildXmlNode(root, "DisplayElements");
    wxXmlNode* elements_node = AddChildXmlNode(root, "ElementEffects");
    wxXmlNode* last_view_node = AddChildXmlNode(root, "lastView");

    SetNodeContent(last_view_node, wxString::Format("%d", seq_elements.GetCurrentView()));

    int num_data_layers = mDataLayers.GetNumLayers();
    for(int i = 0; i < num_data_layers; ++i )
    {
        DataLayer* layer = mDataLayers.GetDataLayer(i);
        wxXmlNode* layer_node = AddChildXmlNode(data_layer, "DataLayer");
        layer_node->AddAttribute("lor_params", wxString::Format("%d", layer->GetLORConvertParams()));
        layer_node->AddAttribute("channel_offset", wxString::Format("%d", layer->GetChannelOffset()));
        layer_node->AddAttribute("num_channels", wxString::Format("%d", layer->GetNumChannels()));
        layer_node->AddAttribute("num_frames", wxString::Format("%d", layer->GetNumFrames()));
        layer_node->AddAttribute("data", layer->GetDataSource());
        layer_node->AddAttribute("source", layer->GetSource());
        layer_node->AddAttribute("name", layer->GetName());
    }

    int num_elements = seq_elements.GetElementCount();
    for(int i = 0; i < num_elements; ++i)
    {
        Element* element = seq_elements.GetElement(i);

        // Add display elements
        wxXmlNode* display_element_node = AddChildXmlNode(display_node, "Element");
        display_element_node->AddAttribute("collapsed", string_format("%d", element->GetCollapsed()));
        display_element_node->AddAttribute("type", element->GetType());
        display_element_node->AddAttribute("name", element->GetName());
        display_element_node->AddAttribute("visible", string_format("%d", element->GetVisible()));
        display_element_node->AddAttribute("active", string_format("%d", element->GetActive()));
        if( element->GetType() == "timing" )
        {
            display_element_node->AddAttribute("views", element->GetViews());
        }

        // Add element node to ElementEffects
        wxXmlNode* element_effects_node = AddChildXmlNode(elements_node, "Element");
        element_effects_node->AddAttribute("type", element->GetType());
        element_effects_node->AddAttribute("name", element->GetName());

        // Add effect layers
        if( element->GetFixedTiming() )
        {
            element_effects_node->AddAttribute("fixed", string_format( "%d", element->GetFixedTiming()));
            // Add empty layer node
            AddChildXmlNode(element_effects_node, "EffectLayer");
        }
        else
        {
            int num_layers = element->GetEffectLayerCount();
            for(int j = 0; j < num_layers; ++j)
            {
                EffectLayer* layer = element->GetEffectLayer(j);

                // Add layer node
                wxXmlNode* effect_layer_node = AddChildXmlNode(element_effects_node, "EffectLayer");
                if( element->GetType() == "model" )
                {
                    WriteEffects(layer, effect_layer_node, colorPalettes,
                                 colorPalette_node,
                                 effectStrings,
                                 effectDB_Node);
                }
                else if( element->GetType() == "timing" )
                {
                    // Add effects
                    int num_effects = layer->GetEffectCount();
                    for(int k = 0; k < num_effects; ++k)
                    {
                        Effect* effect = layer->GetEffect(k);
                        // Add effect node
                        wxXmlNode* effect_node = AddChildXmlNode(effect_layer_node, "Effect", effect->GetSettingsAsString());

                        effect_node->AddAttribute("label", effect->GetEffectName());
                        if (effect->GetProtected()) {
                            effect_node->AddAttribute("protected", "1");
                        }
                        if (effect->GetSelected()) {
                            effect_node->AddAttribute("selected", "1");
                        }
                        effect_node->AddAttribute("startTime", string_format("%d", effect->GetStartTimeMS()));
                        effect_node->AddAttribute("endTime", string_format("%d", effect->GetEndTimeMS()));
                    }
                }
                else if( element->GetType() == "view" )
                {
                    int num_effects = layer->GetEffectCount();
                    for(int k = 0; k < num_effects; ++k)
                    {
                        Effect* effect = layer->GetEffect(k);
                        AddChildXmlNode(effect_layer_node, "Effect", effect->GetSettingsAsString());
                    }
                }
            }
            if( element->GetType() == "model" ) {

                int num_layers = element->getStrandLayerCount();
                for(int j = 0; j < num_layers; ++j)
                {
                    StrandLayer* layer = element->GetStrandLayer(j);

                    wxXmlNode* effect_layer_node = nullptr;
                    if (layer->GetEffectCount() != 0) {
                        effect_layer_node = AddChildXmlNode(element_effects_node, "Strand");
                        effect_layer_node->AddAttribute("index", string_format("%d", layer->GetStrand()));
                        if (layer->GetName() != "") {
                            effect_layer_node->AddAttribute("name", layer->GetName());
                        }
                        WriteEffects(layer, effect_layer_node, colorPalettes,
                                     colorPalette_node,
                                     effectStrings,
                                     effectDB_Node);
                    }

                    for (int n = 0; n < layer->GetNodeLayerCount(); n++) {
                        NodeLayer* nlayer = layer->GetNodeLayer(n);
                        if (nlayer->GetEffectCount() == 0) {
                            continue;
                        }
                        if (effect_layer_node == nullptr) {
                            effect_layer_node = AddChildXmlNode(element_effects_node, "Strand");
                            effect_layer_node->AddAttribute("index", string_format("%d", layer->GetStrand()));
                            if (layer->GetName() != "") {
                                effect_layer_node->AddAttribute("name", layer->GetName());
                            }
                        }
                        wxXmlNode* neffect_layer_node = AddChildXmlNode(effect_layer_node, "Node");
                        neffect_layer_node->AddAttribute("index", string_format("%d", n));
                        if (nlayer->GetName() != "") {
                            neffect_layer_node->AddAttribute("name", nlayer->GetName());
                        }
                        WriteEffects(nlayer, neffect_layer_node, colorPalettes,
                                     colorPalette_node,
                                     effectStrings,
                                     effectDB_Node);
                    }
                }
            }
        }
    }
    UpdateVersion();
    seqDocument.Save(GetFullPath());
}

bool xLightsXmlFile::TimingAlreadyExists(const std::string & section, xLightsFrame* xLightsParent)
{
    if( sequence_loaded )
    {
        timing_list = GetTimingList(xLightsParent->GetSequenceElements());
    }
    else
    {
        timing_list = GetTimingList();
    }
    for (size_t i = 0; i < timing_list.size(); ++i )
    {
        if( timing_list[i] == section )
        {
            return true;
        }
    }
    return false;
}



void xLightsXmlFile::AddNewTimingSection(const std::string & filename, xLightsFrame* xLightsParent,
                                         std::vector<int> &starts, std::vector<int> &ends, std::vector<std::string> &labels) {
    Element* element = NULL;
    EffectLayer* effectLayer = NULL;
    wxXmlNode* layer = NULL;
    if( sequence_loaded )
    {
        element = xLightsParent->AddTimingElement(filename);
        effectLayer = element->GetEffectLayer(0);
    }
    else
    {
        AddTimingDisplayElement(filename, "1", "0" );
        wxXmlNode*  node = AddElement( filename, "timing" );
        layer = AddChildXmlNode(node, "EffectLayer");
    }
    for (size_t k = 0; k < starts.size(); k++) {

        if( sequence_loaded )
        {
            effectLayer->AddEffect(0,labels[k],"","",starts[k],ends[k],EFFECT_NOT_SELECTED,false);
        }
        else
        {
            AddTimingEffect(layer, labels[k], "0", "0", string_format("%d", starts[k]), string_format("%d", ends[k]));
        }
    }
}
void xLightsXmlFile::AddNewTimingSection(const std::string & interval_name, xLightsFrame* xLightsParent)
{
    AddTimingDisplayElement( interval_name, "1", "0" );
    wxXmlNode* node;

    if( sequence_loaded )
    {
        xLightsParent->AddTimingElement(interval_name);
    }
    node = AddElement( interval_name, "timing" );

    AddChildXmlNode(node, "EffectLayer");
}

void xLightsXmlFile::AddFixedTimingSection(const std::string & interval_name, xLightsFrame* xLightsParent)
{
    AddTimingDisplayElement( interval_name, "1", "0" );
    wxXmlNode* node;

    if( interval_name == "Empty" )
    {
        if( sequence_loaded )
        {
            xLightsParent->AddTimingElement(interval_name);
        }
        node = AddElement( interval_name, "timing" );
    }
    else
    {
        int interval = wxAtoi(interval_name);;
        if( sequence_loaded )
        {
            Element* element = xLightsParent->AddTimingElement(interval_name);
            element->SetFixedTiming(interval);
            EffectLayer* effectLayer = element->GetEffectLayer(0);
            int time = 0;
            int end_time = GetSequenceDurationMS();
            int startTime, endTime, next_time;
            while( time <= end_time )
            {
                next_time = (time + interval <= end_time) ? time + interval : end_time;
                startTime = TimeLine::RoundToMultipleOfPeriod(time, GetFrequency());
                endTime = TimeLine::RoundToMultipleOfPeriod(next_time, GetFrequency());
                effectLayer->AddEffect(0,"","","",startTime,endTime,EFFECT_NOT_SELECTED,false);
                time += interval;
            }
        }
        node = AddFixedTiming( interval_name, string_format("%d",interval) );
    }

    AddChildXmlNode(node, "EffectLayer");
}

void xLightsXmlFile::SetMetaMP3Tags()
{
	SetHeaderInfo(SONG, audio->Title());
	SetHeaderInfo(ARTIST, audio->Artist());
	SetHeaderInfo(ALBUM, audio->Album());
}

void xLightsXmlFile::AdjustEffectSettingsForVersion(SequenceElements& elements, xLightsFrame* xLightsParent)
{
    std::string ver = GetVersion().ToStdString();
    std::vector<RenderableEffect*> effects(xLightsParent->GetEffectManager().size());
    int count = 0;
    for (int x = 0; x < xLightsParent->GetEffectManager().size(); x++) {
        RenderableEffect *eff = xLightsParent->GetEffectManager()[x];
        if (eff->needToAdjustSettings(ver)) {
            effects[x] = eff;
            count++;
        }
    }
    if (count > 0) {
        for( size_t i = 0; i < elements.GetElementCount(); i++ )  {
            Element* elem = elements.GetElement(i);
            if( elem->GetType() == "model" ) {
                for( int j = 0; j < elem->GetEffectLayerCount(); j++ ) {
                    EffectLayer* layer = elem->GetEffectLayer(j);
                    for( int k = 0; k < layer->GetEffectCount(); k++ ) {
                        Effect* eff = layer->GetEffect(k);
                        if ( effects[eff->GetEffectIndex()] != nullptr ) {
                            effects[eff->GetEffectIndex()]->adjustSettings(ver, eff);
                        }
                    }
                }
                for (int j = 0; j < elem->getStrandLayerCount(); j++) {
                    StrandLayer* layer = elem->GetStrandLayer(j);
                    for( int k = 0; k < layer->GetEffectCount(); k++ ) {
                        Effect* eff = layer->GetEffect(k);
                        if ( effects[eff->GetEffectIndex()] != nullptr ) {
                            effects[eff->GetEffectIndex()]->adjustSettings(ver, eff);
                        }
                    }
                    for (int k = 0; k < layer->GetNodeLayerCount(); k++) {
                        NodeLayer* nlayer = layer->GetNodeLayer(k);
                        for( int l = 0; l < nlayer->GetEffectCount(); l++ ) {
                            Effect* eff = nlayer->GetEffect(l);
                            if ( effects[eff->GetEffectIndex()] != nullptr ) {
                                effects[eff->GetEffectIndex()]->adjustSettings(ver, eff);
                            }
                        }
                    }
                }
            }
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
void xLightsXmlFile::FixVersionDifferences(const wxString& filename)
{
    wxString        str,fileout;
    wxTextFile      tfile;
    wxFile f;
    bool modified=false;
    fileout = filename + ".out";
    //wxMessageBox("fixing: "+file);
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

    tfile.Open(filename); // open input file
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
    if(modified) wxCopyFile(fileout,filename,true); // if we modified the file, copy over it
    wxRemoveFile(fileout); // get rid of temporary file
}

void xLightsXmlFile::FixEffectPresets(wxXmlNode* effects_node)
{
    wxString main_version = effects_node->GetAttribute("version", "0000");
    for(wxXmlNode* ele=effects_node->GetChildren(); ele!=NULL; ele=ele->GetNext() )
    {
        if (ele->GetName() == "effect")
        {
            wxString version = ele->GetAttribute("version", "0000");
            if( version == "0000" )
            {
                version = main_version;
            }
            if (version < "0003" )
            {
                wxString settings=ele->GetAttribute("settings");

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
                data1 = AdjustV3Effect(effect1, data1);
                data2 = AdjustV3Effect(effect2, data2);
                wxString palette1 = SplitPalette(data1);
                wxString palette2 = SplitPalette(data2);

                wxString start_time = wxString::Format("%d",1000);
                wxString end_time = wxString::Format("%d",4000);
                wxString row1 = wxString::Format("%d",1);
                wxString row2 = wxString::Format("%d",2);
                wxString column_start = wxString::Format("%d",1000);
                wxString eff_data = effect1 + "\t" + data1 + "\t" + palette1 + "\t" + start_time + "\t" + end_time + "\t" + row1 + "\t" + column_start + "\n";
                         eff_data += effect2 + "\t" + data2 + "\t" + palette2 + "\t" + start_time + "\t" + end_time + "\t" + row2 + "\t" + column_start + "\n";

                ele->DeleteAttribute("settings");
                ele->AddAttribute("settings", eff_data);
                ele->DeleteAttribute("version");
                ele->AddAttribute("version", XLIGHTS_RGBEFFECTS_VERSION);
            }
            else if( version == "0003" )
            {
                wxString settings=ele->GetAttribute("settings");
                wxArrayString all_efdata = wxSplit(settings, '\n');
                for (size_t i = 0; i < all_efdata.size()-1; i++ )
                {
                    wxArrayString efdata = wxSplit(all_efdata[i], '\t');
                    double time;
                    efdata[3].ToDouble(&time);
                    int new_start_time = (int)(time * 1000.0);
                    efdata[4].ToDouble(&time);
                    int new_end_time = (int)(time * 1000.0);
                    efdata[6].ToDouble(&time);
                    int column_start_time = (int)(time * 1000.0);
                    efdata[3] = wxString::Format("%d",new_start_time);
                    efdata[4] = wxString::Format("%d",new_end_time);
                    efdata[6] = wxString::Format("%d",column_start_time);
                    all_efdata[i] = wxJoin(efdata, '\t');
                }
                settings = wxJoin(all_efdata, '\n');
                ele->DeleteAttribute("settings");
                ele->AddAttribute("settings", settings);
                ele->DeleteAttribute("version");
                ele->AddAttribute("version", XLIGHTS_RGBEFFECTS_VERSION);
            }
        }
        else if (ele->GetName() == "effectGroup")
        {
            FixEffectPresets(ele);
        }
    }
}
