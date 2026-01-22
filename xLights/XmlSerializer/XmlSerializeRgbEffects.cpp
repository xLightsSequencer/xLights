/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

void XmlSerializingVisitor::SerializeViewsObject(wxXmlNode* node, xLightsFrame* xlights) {
    wxXmlNode* viewsNode = new wxXmlNode(wxXML_ELEMENT_NODE, "views");
    SequenceViewManager* seqViewMgr = xlights->GetViewsManager();
    std::list<SequenceView*> views = seqViewMgr->GetViews();
    for (SequenceView* view : views) {
        std::string name = view->GetName();
        if (name != "Master View") {
            wxXmlNode* viewChild = new wxXmlNode(wxXML_ELEMENT_NODE, "view");
            viewChild->AddAttribute("name", name);
            viewChild->AddAttribute(XmlNodeKeys::ModelsAttribute, view->GetModelsString());
            viewsNode->AddChild(viewChild);
        }
    }
    node->AddChild(viewsNode);
}

void XmlSerializingVisitor::SerializeColorsObject(wxXmlNode* node, xLightsFrame* xlights) {
    wxXmlNode* colorsNode = new wxXmlNode(wxXML_ELEMENT_NODE, "colors");
    ColorManager* colorMgr = new ColorManager(xlights);
    std::map<std::string, xlColor> colors = colorMgr->GetColors();
    for (const auto& c : colors) {
        wxXmlNode* colorChild = new wxXmlNode(wxXML_ELEMENT_NODE, c.first);
        colorChild->AddAttribute(XmlNodeKeys::RedAttribute, std::to_string(c.second.red));
        colorChild->AddAttribute(XmlNodeKeys::GreenAttribute, std::to_string(c.second.green));
        colorChild->AddAttribute(XmlNodeKeys::BlueAttribute, std::to_string(c.second.blue));
        colorsNode->AddChild(colorChild);
    }
    node->AddChild(colorsNode);
}

void XmlSerializingVisitor::SerializeLayoutGroupsObject(wxXmlNode* node, xLightsFrame* xlights) {
    std::vector<LayoutGroup*> layoutGroups = xlights->LayoutGroups;
    wxXmlNode* lgNode = new wxXmlNode(wxXML_ELEMENT_NODE, "layoutGroups");
    for (LayoutGroup* lg : layoutGroups) {
        wxXmlNode* lgChild = new wxXmlNode(wxXML_ELEMENT_NODE, "layoutGroup");
        lgChild->AddAttribute("name", lg->GetName());
        lgChild->AddAttribute(XmlNodeKeys::BackgroundImageAttribute, lg->GetBackgroundImage());
        lgChild->AddAttribute(XmlNodeKeys::BackgroundBrightnessAttribute, std::to_string(lg->GetBackgroundBrightness()));
        lgChild->AddAttribute(XmlNodeKeys::BackgroundAlphaAttribute, std::to_string(lg->GetBackgroundAlpha()));
        lgChild->AddAttribute(XmlNodeKeys::ScaleImageAttribute, std::to_string(lg->GetBackgroundScaled()));
        lgNode->AddChild(lgChild);
    }
    node->AddChild(lgNode);
}

void XmlSerializingVisitor::SerializePerspectivesObject(wxXmlNode* node, xLightsFrame* xlights) {
    std::list<std::string> perspectives = xlights->GetPerspectives();
    wxXmlNode* perspectivesNode = new wxXmlNode(wxXML_ELEMENT_NODE, "perspectives");
    for (std::string p : perspectives) {
        wxXmlNode* pChild = new wxXmlNode(wxXML_ELEMENT_NODE, "perspective");
        pChild->AddAttribute("name", p);
        perspectivesNode->AddChild(pChild);
    }
    node->AddChild(perspectivesNode);
}

void XmlSerializingVisitor::SerializeSettingsObject(wxXmlNode* node, xLightsFrame* xlights) {
    wxXmlNode* settings = new wxXmlNode(wxXML_ELEMENT_NODE, "settings");
    wxXmlNode* scaleimage = new wxXmlNode(wxXML_ELEMENT_NODE, "scaleImage");
    scaleimage->AddAttribute("value", std::to_string(xlights->GetDefaultPreviewBackgroundScaled()));
    settings->AddChild(scaleimage);
    wxXmlNode* bkgimage = new wxXmlNode(wxXML_ELEMENT_NODE, "backgroundImage");
    bkgimage->AddAttribute("value", "tbd");
    settings->AddChild(bkgimage);
    wxXmlNode* bkgbright = new wxXmlNode(wxXML_ELEMENT_NODE, "backgroundBrightness");
    bkgbright->AddAttribute("value", std::to_string(xlights->GetDefaultPreviewBackgroundBrightness()));
    settings->AddChild(bkgbright);
    wxXmlNode* bkgalpha = new wxXmlNode(wxXML_ELEMENT_NODE, "backgroundAlpha");
    bkgalpha->AddAttribute("value", std::to_string(xlights->GetDefaultPreviewBackgroundAlpha()));
    settings->AddChild(bkgalpha);
    wxXmlNode* boundbox = new wxXmlNode(wxXML_ELEMENT_NODE, "Display2DBoundingBox");
    boundbox->AddAttribute("value", std::to_string(xlights->GetDisplay2DBoundingBox()));
    settings->AddChild(boundbox);
    wxXmlNode* grid = new wxXmlNode(wxXML_ELEMENT_NODE, "Display2DGrid");
    grid->AddAttribute("value", std::to_string(xlights->GetDisplay2DGrid()));
    settings->AddChild(grid);
    wxXmlNode* gridspace = new wxXmlNode(wxXML_ELEMENT_NODE, "Display2DGridSpacing");
    gridspace->AddAttribute("value", std::to_string(xlights->GetDisplay2DGridSpacing()));
    settings->AddChild(gridspace);
    wxXmlNode* center0 = new wxXmlNode(wxXML_ELEMENT_NODE, "Display2DCenter0");
    center0->AddAttribute("value", std::to_string(xlights->GetDisplay2DCenter0()));
    settings->AddChild(center0);
    wxXmlNode* laygrp = new wxXmlNode(wxXML_ELEMENT_NODE, "storedLayourGroup");
    laygrp->AddAttribute("value", xlights->GetStoredLayoutGroup());
    settings->AddChild(laygrp);
    wxXmlNode* layout3d = new wxXmlNode(wxXML_ELEMENT_NODE, "LayoutMode3D");
    layout3d->AddAttribute("value", "tbd");
    settings->AddChild(layout3d);
    wxXmlNode* previewW = new wxXmlNode(wxXML_ELEMENT_NODE, "previewWidth");
    previewW->AddAttribute("value", std::to_string(0));
    settings->AddChild(previewW);
    wxXmlNode* previewH = new wxXmlNode(wxXML_ELEMENT_NODE, "previewHeight");
    previewH->AddAttribute("value", std::to_string(0));
    settings->AddChild(previewH);
    node->AddChild(settings);
}
