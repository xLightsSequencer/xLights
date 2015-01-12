#include "xLightsXmlFile.h"
#include "xLightsMain.h"

#define string_format wxString::Format

xLightsXmlFile::xLightsXmlFile()
:   is_loaded(false),
    major_version(3)
{

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
    if( node_data != wxString(""))
    {
        wxXmlNode* data_node = new wxXmlNode(new_node,wxXML_TEXT_NODE,wxT(""),node_data);
    }
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

const wxString xLightsXmlFile::GetVersion()
{
    if( major_version <= 3 )
    {
        return string_format("%d.x", major_version);
    }
    else
    {
        return version_string;
    }
}

void xLightsXmlFile::Clear()
{
    is_loaded = false;
    major_version = 3;  // default to old version until we detect new version
    version_string = _("");
    models.Clear();
    timing_protection.Clear();
    timing.Clear();
    label_protection.Clear();
    labels.Clear();
    effect_protection.Clear();
    effects.Clear();
}

void xLightsXmlFile::Load()
{
    if( is_loaded )
    {
        Clear();
    }

    SetPath( xLightsFrame::CurrentDir );
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

    bool models_defined = false;
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
    }

    if( !models_defined )  // no <tr> tag was found
    {
        return;
    }

    is_loaded = true;
}
