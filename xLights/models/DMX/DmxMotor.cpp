/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
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
    : node_xml(node), base_name(_name), channel(0),
    min_value(0), max_value(65535), range_of_motion(180.0f),
    _16bit(true)
{
}

DmxMotor::~DmxMotor()
{
}

void DmxMotor::SetChannel(int chan, BaseObject* base) {
    channel = chan;
    node_xml->DeleteAttribute("Channel");
    node_xml->AddAttribute("Channel", wxString::Format("%d", channel));
}

void DmxMotor::SetMinLimit(int val) {
    min_value = val;
    node_xml->DeleteAttribute("MinValue");
    node_xml->AddAttribute("MinValue", std::to_string(min_value));
}

void DmxMotor::SetMaxLimit(int val) {
    max_value = val;
    node_xml->DeleteAttribute("MaxValue");
    node_xml->AddAttribute("MaxValue", std::to_string(max_value));
}

void DmxMotor::SetRangeOfMotion(float val) {
    range_of_motion = val;
    node_xml->DeleteAttribute("RangeOfMotion");
    node_xml->AddAttribute("RangeOfMotion", std::to_string(range_of_motion));
}

void DmxMotor::Init(BaseObject* base) {
    this->base = base;
    channel = wxAtoi(node_xml->GetAttribute("Channel", "0"));
    min_value = wxAtoi(node_xml->GetAttribute("MinValue", "0"));
    max_value = wxAtoi(node_xml->GetAttribute("MaxValue", "65535"));
    range_of_motion = wxAtof(node_xml->GetAttribute("RangeOfMotion", "180.0f"));
    orient_zero = wxAtoi(node_xml->GetAttribute("OrientZero", "0"));
    orient_home = wxAtoi(node_xml->GetAttribute("OrientHome", "0"));
    slew_limit = wxAtof(node_xml->GetAttribute("SlewLimit", "0.0f"));
}

float DmxMotor::GetPosition(int channel_value) {
    return ((1.0 - ((channel_value - min_value) / (float)(max_value - min_value))) * range_of_motion - range_of_motion);
}

void DmxMotor::AddTypeProperties(wxPropertyGridInterface *grid) {
    grid->Append(new wxPropertyCategory(base_name, base_name + "Properties"));
    
    wxPGProperty* p = grid->Append(new wxUIntProperty("Channel", base_name + "Channel", channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Min Value", base_name + "MinValue", min_value));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", _16bit ? 65535 : 255);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Max Value", base_name + "MaxValue", max_value));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", _16bit ? 65535 : 255);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxFloatProperty("Range of Motion", base_name + "RangeOfMotion", range_of_motion));
    p->SetAttribute("Precision", 1);
    p->SetAttribute("Step", 1.0);
    p->SetAttribute("Min", -65535);
    p->SetAttribute("Max", 65535);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Orient to Zero", base_name + "OrientZero", orient_zero));
    p->SetAttribute("Min", -180);
    p->SetAttribute("Max", 180);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Orient Forward", base_name + "OrientHome", orient_home));
    p->SetAttribute("Min", -180);
    p->SetAttribute("Max", 180);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxFloatProperty("Slew Limit (deg/sec)", base_name + "SlewLimit", slew_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 500);
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("16 Bit", base_name + "Bits16", _16bit));
    p->SetAttribute("UseCheckbox", true);

    grid->Collapse(base_name + "Properties");
}

int DmxMotor::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked) {
    std::string name = event.GetPropertyName().ToStdString();

    if (base_name + "Channel" == name) {
        node_xml->DeleteAttribute("Channel");
        node_xml->AddAttribute("Channel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::Channel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::Channel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxMotor::OnPropertyGridChange::Channel");
        return 0;
    }
    else if (base_name + "MinLimit" == name) {
        node_xml->DeleteAttribute("MinLimit");
        node_xml->AddAttribute("MinLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::MinLimit");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::MinLimit");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::MinLimit");
        return 0;
    }
    else if (base_name + "MaxLimit" == name) {
        node_xml->DeleteAttribute("MaxLimit");
        node_xml->AddAttribute("MaxLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::MaxLimit");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::MaxLimit");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::MaxLimit");
        return 0;
    }
    else if (base_name + "RangeOfMotion" == name) {
        node_xml->DeleteAttribute("RangeOfMotion");
        node_xml->AddAttribute("RangeOfMotion", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::RangeOfMotion");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::RangeOfMotion");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::RangeOfMotion");
        return 0;
    }
    else if (base_name + "OrientZero" == name) {
        node_xml->DeleteAttribute("OrientZero");
        node_xml->AddAttribute("OrientZero", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::OrientZero");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::OrientZero");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::OrientZero");
        return 0;
    }
    else if (base_name + "OrientHome" == name) {
        node_xml->DeleteAttribute("OrientHome");
        node_xml->AddAttribute("OrientHome", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::OrientHome");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::OrientHome");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::OrientHome");
        return 0;
    }
    else if (base_name + "SlewLimit" == name) {
        node_xml->DeleteAttribute("SlewLimit");
        node_xml->AddAttribute("SlewLimit", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::SlewLimit");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::SlewLimit");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::SlewLimit");
        return 0;
    }
    else if (base_name + "Bits16" == name) {
        node_xml->DeleteAttribute("Bits16");
        if (event.GetValue().GetBool()) {
            _16bit = true;
            node_xml->AddAttribute("Bits16", "1");
        } else {
            _16bit = false;
            node_xml->AddAttribute("Bits16", "0");
        }
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::Bits16");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMotor::OnPropertyGridChange::Bits16");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::Bits16");
        return 0;
    }
    return -1;
}

void DmxMotor::Set16Bit(bool value)
{
    _16bit = value;
    if (!_16bit) {
        if (min_value > 255) {
            min_value = 255;
            node_xml->DeleteAttribute("MinValue");
            node_xml->AddAttribute("MinValue", wxString::Format("%d", min_value));
        }
        if (max_value > 255) {
            max_value = 255;
            node_xml->DeleteAttribute("MaxValue");
            node_xml->AddAttribute("MaxValue", wxString::Format("%d", max_value));
        }
    }
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
