/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/sstream.h>

#include "DmxMotor.h"
#include "../BaseObject.h"
#include "../ModelScreenLocation.h"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

DmxMotor::DmxMotor(wxXmlNode* node, wxString _name)
    : DmxMotorBase(), node_xml(node), base_name(_name) {

}

DmxMotor::~DmxMotor()
{
}

void DmxMotor::SetChannelCoarse(int chan) {
    channel_coarse = chan;
    node_xml->DeleteAttribute("ChannelCoarse");
    node_xml->AddAttribute("ChannelCoarse", wxString::Format("%d", channel_coarse));
}

void DmxMotor::Init(BaseObject* base)
{
    this->base = base;
    channel_coarse = wxAtoi(node_xml->GetAttribute("ChannelCoarse", "0"));
    channel_fine = wxAtoi(node_xml->GetAttribute("ChannelFine", "0"));
    min_limit = wxAtoi(node_xml->GetAttribute("MinLimit", "-180"));
    max_limit = wxAtoi(node_xml->GetAttribute("MaxLimit", "180"));
    range_of_motion = wxAtof(node_xml->GetAttribute("RangeOfMotion", "180.0f"));
    orient_zero = wxAtoi(node_xml->GetAttribute("OrientZero", "0"));
    orient_home = wxAtoi(node_xml->GetAttribute("OrientHome", "0"));
    slew_limit = wxAtof(node_xml->GetAttribute("SlewLimit", "0.0f"));
    reverse = wxAtoi(node_xml->GetAttribute("Reverse", "0"));
    upside_down = wxAtoi(node_xml->GetAttribute("UpsideDown", "0"));
    if (reverse) {
        rev = -1;
    } else {
        rev = 1;
    }
}
    int DmxMotor::ConvertPostoCmd(float position)
    {
        float limited_pos = position;
        if (limited_pos > max_limit) {
            limited_pos = max_limit;
        } else if (limited_pos < min_limit) {
            limited_pos = min_limit;
        }

        if (upside_down) {
            limited_pos = -1.0f * limited_pos;
        }

        float goto_home = (float)max_value * (float)orient_home / range_of_motion;
        float amount_to_move = (float)max_value * limited_pos / range_of_motion * rev;
        float cmd = goto_home + amount_to_move;
        float full_spin = (float)max_value * 360.0 / range_of_motion;

        if (cmd < 0) {
            if (cmd + full_spin < max_value) {
                cmd += full_spin;
            } else {
                cmd = 0; // tbd....figure out which limit is closer to desired target
            }
        } else if (cmd > max_value) {
            if (cmd - full_spin >= 0.0f) {
                cmd -= full_spin;
            } else {
                cmd = max_value; // tbd....figure out which limit is closer to desired target
            }
        }
        return cmd;
    }

    float DmxMotor::GetPosition(int channel_value)
    {
        return ((1.0 - ((channel_value - min_value) / (float)(max_value - min_value))) * (rev * range_of_motion) - (rev * range_of_motion));
    }


void DmxMotor::AddTypeProperties(wxPropertyGridInterface *grid) {
    grid->Append(new wxPropertyCategory(base_name, base_name + "Properties"));
    
    auto p = grid->Append(new wxUIntProperty("Channel (Coarse)", base_name + "ChannelCoarse", channel_coarse));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Channel (Fine)", base_name + "ChannelFine", channel_fine));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Min Limit (deg)", base_name + "MinLimit", min_limit));
    p->SetAttribute("Min", -180);
    p->SetAttribute("Max", 0);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Max Limit (deg)", base_name + "MaxLimit", max_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 180);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxFloatProperty("Range of Motion", base_name + "RangeOfMotion", range_of_motion));
    p->SetAttribute("Precision", 1);
    p->SetAttribute("Step", 1.0);
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 65535);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Orient to Zero", base_name + "OrientZero", orient_zero));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 360);
    p->SetEditor("SpinCtrl");

    std::string label = "Orient Up";
    if( base_name == "PanMotor" ) {
        label = "Orient Forward";
    }
    p = grid->Append(new wxIntProperty(label, base_name + "OrientHome", orient_home));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 360);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxFloatProperty("Slew Limit (deg/sec)", base_name + "SlewLimit", slew_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 500);
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("Reverse", base_name + "Reverse", reverse));
    p->SetAttribute("UseCheckbox", true);

    p = grid->Append(new wxBoolProperty("Upside Down", base_name + "UpsideDown", upside_down));
    p->SetAttribute("UseCheckbox", true);

    grid->Collapse(base_name + "Properties");
}

int DmxMotor::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked) {
    std::string name = event.GetPropertyName().ToStdString();

    if (base_name + "ChannelCoarse" == name) {
        channel_coarse = (int)event.GetPropertyValue().GetLong();
        node_xml->DeleteAttribute("ChannelCoarse");
        node_xml->AddAttribute("ChannelCoarse", wxString::Format("%d", channel_coarse));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::ChannelCoarse");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::ChannelCoarse");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxMotor::OnPropertyGridChange::ChannelCoarse");
        return 0;
    }
    else if (base_name + "ChannelFine" == name) {
        channel_fine = (int)event.GetPropertyValue().GetLong();
        node_xml->DeleteAttribute("ChannelFine");
        node_xml->AddAttribute("ChannelFine", wxString::Format("%d", channel_fine));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::ChannelFine");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::ChannelFine");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxMotor::OnPropertyGridChange::ChannelFine");
        return 0;
    }
    else if (base_name + "MinLimit" == name) {
        min_limit = (int)event.GetPropertyValue().GetLong();
        node_xml->DeleteAttribute("MinLimit");
        node_xml->AddAttribute("MinLimit", wxString::Format("%d", min_limit));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::MinLimit");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::MinLimit");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::MinLimit");
        return 0;
    }
    else if (base_name + "MaxLimit" == name) {
        max_limit = (int)event.GetPropertyValue().GetLong();
        node_xml->DeleteAttribute("MaxLimit");
        node_xml->AddAttribute("MaxLimit", wxString::Format("%d", max_limit));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::MaxLimit");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::MaxLimit");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::MaxLimit");
        return 0;
    }
    else if (base_name + "RangeOfMotion" == name) {
        range_of_motion = (float)event.GetPropertyValue().GetDouble();
        node_xml->DeleteAttribute("RangeOfMotion");
        node_xml->AddAttribute("RangeOfMotion", wxString::Format("%6.4f", range_of_motion));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::RangeOfMotion");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::RangeOfMotion");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::RangeOfMotion");
        return 0;
    }
    else if (base_name + "OrientZero" == name) {
        orient_zero = (int)event.GetPropertyValue().GetLong();
        node_xml->DeleteAttribute("OrientZero");
        node_xml->AddAttribute("OrientZero", wxString::Format("%d", orient_zero));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::OrientZero");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::OrientZero");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::OrientZero");
        return 0;
    }
    else if (base_name + "OrientHome" == name) {
        orient_home = (int)event.GetPropertyValue().GetLong();
        node_xml->DeleteAttribute("OrientHome");
        node_xml->AddAttribute("OrientHome", wxString::Format("%d", orient_home));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::OrientHome");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::OrientHome");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::OrientHome");
        return 0;
    }
    else if (base_name + "SlewLimit" == name) {
        slew_limit = (float)event.GetPropertyValue().GetDouble();
        node_xml->DeleteAttribute("SlewLimit");
        node_xml->AddAttribute("SlewLimit", wxString::Format("%6.4f", slew_limit));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::SlewLimit");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::SlewLimit");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::SlewLimit");
        return 0;
    }
    else if (base_name + "Reverse" == name) {
        node_xml->DeleteAttribute("Reverse");
        if (event.GetValue().GetBool()) {
            reverse = true;
            node_xml->AddAttribute("Reverse", "1");
        } else {
            reverse = false;
            node_xml->AddAttribute("Reverse", "0");
        }
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::Reverse");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMotor::OnPropertyGridChange::Reverse");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::Reverse");
        return 0;
    }
    else if (base_name + "UpsideDown" == name) {
        node_xml->DeleteAttribute("UpsideDown");
        if (event.GetValue().GetBool()) {
            upside_down = true;
            node_xml->AddAttribute("UpsideDown", "1");
        } else {
            upside_down = false;
            node_xml->AddAttribute("UpsideDown", "0");
        }
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::UpsideDown");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMotor::OnPropertyGridChange::UpsideDown");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::UpsideDown");
        return 0;
    }

    return -1;
}

// Serialise for export
void DmxMotor::Serialise(wxXmlNode* root, wxFile& f, const wxString& show_dir) const
{
    wxString res = "";

    wxXmlNode* child = root->GetChildren();
    while (child != nullptr) {
        if (child->GetName() == base_name) {
            wxXmlDocument new_doc;
            new_doc.SetRoot(new wxXmlNode(*child));
            wxStringOutputStream stream;
            new_doc.Save(stream);
            wxString s = stream.GetString();
            s = s.SubString(s.Find("\n") + 1, s.Length()); // skip over xml format header
            int index = s.Find(show_dir);
            while (index != wxNOT_FOUND) {
                s = s.SubString(0, index - 1) + s.SubString(index + show_dir.Length() + 1, s.Length());
                index = s.Find(show_dir);
            }
            res += s;
        }
        child = child->GetNext();
    }

    if (res != "")
    {
        f.Write(res);
    }
}

// Serialise for input
void DmxMotor::Serialise(wxXmlNode* root, wxXmlNode* model_xml, const wxString& show_dir) const
{
    wxXmlNode* node = nullptr;
    for (wxXmlNode* n = model_xml->GetChildren(); n != nullptr; n = n->GetNext())
    {
        if (n->GetName() == base_name)
        {
            node = n;
            break;
        }
    }

    if (node != nullptr) {
        // add new attributes from import
        for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
        {
            if (n->GetName() == base_name)
            {
                for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext())
                {
                    if (node->HasAttribute(a->GetName())) {
                        node->DeleteAttribute(a->GetName());
                    }
                    node->AddAttribute(a->GetName(), a->GetValue());
                }
                return;
            }
        }
    }
}
