/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "XmlNodeKeys.h"
#include "ui/layout/LayoutGroup.h"
#include "../xLightsMain.h"

#include <memory>

void SerializeViewsObject(pugi::xml_node node, xLightsFrame* xlights) {
    pugi::xml_node viewsNode = node.append_child("views");
    SequenceViewManager* seqViewMgr = xlights->GetViewsManager();
    std::list<SequenceView*> views = seqViewMgr->GetViews();
    for (SequenceView* view : views) {
        std::string name = view->GetName();
        if (name != "Master View") {
            pugi::xml_node viewChild = viewsNode.append_child("view");
            viewChild.append_attribute("name") = name;
            viewChild.append_attribute(XmlNodeKeys::ModelsAttribute) = view->GetModelsString();
        }
    }
}

void SerializeColorsObject(pugi::xml_node node, xLightsFrame* xlights) {
    pugi::xml_node colorsNode = node.append_child("colors");
    auto colorMgr = std::make_unique<ColorManager>(xlights);
    std::map<std::string, xlColor> colors = colorMgr->GetColors();
    for (const auto& c : colors) {
        pugi::xml_node colorChild = colorsNode.append_child(c.first);
        colorChild.append_attribute(XmlNodeKeys::RedAttribute) = c.second.red;
        colorChild.append_attribute(XmlNodeKeys::GreenAttribute) = c.second.green;
        colorChild.append_attribute(XmlNodeKeys::BlueAttribute) = c.second.blue;
    }
}

void SerializeLayoutGroupsObject(pugi::xml_node node, xLightsFrame* xlights) {
    pugi::xml_node lgNode = node.append_child("layoutGroups");
    for (LayoutGroup* lg : xlights->LayoutGroups) {
        pugi::xml_node lgChild = lgNode.append_child("layoutGroup");
        lgChild.append_attribute("name") = lg->GetName();
        lgChild.append_attribute(XmlNodeKeys::BackgroundImageAttribute) = lg->GetBackgroundImage();
        lgChild.append_attribute(XmlNodeKeys::BackgroundBrightnessAttribute) = lg->GetBackgroundBrightness();
        lgChild.append_attribute(XmlNodeKeys::BackgroundAlphaAttribute) = lg->GetBackgroundAlpha();
        lgChild.append_attribute(XmlNodeKeys::ScaleImageAttribute) = (int)lg->GetBackgroundScaled();
        lgChild.append_attribute("PosX") = lg->GetPosX();
        lgChild.append_attribute("PosY") = lg->GetPosY();
        lgChild.append_attribute("PaneWidth") = lg->GetPaneWidth();
        lgChild.append_attribute("PaneHeight") = lg->GetPaneHeight();
    }
}

void SerializePerspectivesObject(pugi::xml_node node, xLightsFrame* xlights) {
    std::list<std::string> perspectives = xlights->GetPerspectives();
    pugi::xml_node perspectivesNode = node.append_child("perspectives");
    for (const std::string& p : perspectives) {
        pugi::xml_node pChild = perspectivesNode.append_child("perspective");
        pChild.append_attribute("name") = p;
    }
}

void SerializeSettingsObject(pugi::xml_node node, xLightsFrame* xlights) {
    pugi::xml_node settings = node.append_child("settings");
    settings.append_child("scaleImage").append_attribute("value") = (int)xlights->GetDefaultPreviewBackgroundScaled();
    settings.append_child("backgroundImage").append_attribute("value") = xlights->GetDefaultPreviewBackgroundImage();
    settings.append_child("backgroundBrightness").append_attribute("value") = xlights->GetDefaultPreviewBackgroundBrightness();
    settings.append_child("backgroundAlpha").append_attribute("value") = xlights->GetDefaultPreviewBackgroundAlpha();
    settings.append_child("Display2DBoundingBox").append_attribute("value") = (int)xlights->GetDisplay2DBoundingBox();
    settings.append_child("Display2DGrid").append_attribute("value") = (int)xlights->GetDisplay2DGrid();
    settings.append_child("Display2DGridSpacing").append_attribute("value") = xlights->GetDisplay2DGridSpacing();
    settings.append_child("Display2DCenter0").append_attribute("value") = (int)xlights->GetDisplay2DCenter0();
    settings.append_child("storedLayoutGroup").append_attribute("value") = xlights->GetStoredLayoutGroup();
    settings.append_child("LayoutMode3D").append_attribute("value") = xlights->GetXmlSetting("LayoutMode3D", "0");
    settings.append_child("previewWidth").append_attribute("value") = 0;
    settings.append_child("previewHeight").append_attribute("value") = 0;
}
