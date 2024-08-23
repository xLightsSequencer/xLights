#include "RGBEffects.h"
#include "xScheduleApp.h"
#include "xScheduleMain.h"

RGBEffects::RGBEffects() {
    std::string showDir = wxGetApp().GetFrame()->GetShowDir();

    _rgbEffects = new wxXmlDocument();
    _rgbEffects->Load(showDir + "/xlights_rgbeffects.xml");
}

std::list<std::string> RGBEffects::GetModels(const std::string& ofType) {
    std::list<std::string> models;
    wxXmlNode* root = _rgbEffects->GetRoot();
    for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext()) {
        if (n->GetName() == "models") {
            for (wxXmlNode* nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
                if (nn->GetName() == "model") {
                    if (nn->GetAttribute("DisplayAs") == ofType) {
                        models.push_back(nn->GetAttribute("name"));
                    }
                }
            }
        }
    }
    return models;
}

wxXmlNode* RGBEffects::GetModel(const std::string& model) {
    wxXmlNode* root = _rgbEffects->GetRoot();
    for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext()) {
        if (n->GetName() == "models") {
            for (wxXmlNode* nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
                if (nn->GetName() == "model") {
                    if (nn->GetAttribute("name") == model) {
                        return nn;
                    }
                }
            }
        }
    }
    return nullptr;
}