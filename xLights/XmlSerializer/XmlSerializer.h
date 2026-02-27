#pragma once

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
#include "XmlDeserializingModelFactory.h"
#include "XmlDeserializingObjectFactory.h"
#include "XmlSerializeFunctions.h"
#include "XmlSerializingVisitor.h"
#include <vector>
#include <wx/xml/xml.h>
#include "../LayoutGroup.h"

struct XmlSerializer {
    XmlSerializer() {}

    // Serialize all models into an XML document
    void SerializeAllModels(const ModelManager & allModels, xLightsFrame* xlights, wxXmlNode* root) {

        wxXmlNode* modelsNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelsNodeName);
        modelsNode->AddAttribute(XmlNodeKeys::TypeAttribute, XmlNodeKeys::RGBEffectsAttribute);

        wxXmlNode* modelGroupNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::GroupsNodeName);
        modelGroupNode->AddAttribute(XmlNodeKeys::TypeAttribute, XmlNodeKeys::RGBEffectsAttribute);

        XmlSerializingVisitor visitor{ modelsNode };
        XmlSerializingVisitor groupVisitor{ modelGroupNode };

        for (auto m = allModels.begin(); m != allModels.end(); ++m) {
            Model* model = m->second;
            if (model->GetDisplayAs() == "ModelGroup") {
                model->Accept(groupVisitor);
            } else {
                model->Accept(visitor);
            }
        }
        
        root->AddChild(modelsNode);
        root->AddChild(modelGroupNode);
    }

    // Serializes and Saves a single model into an XML document (only used for Export)
    void SerializeAndSaveModel(const Model* model, xLightsFrame* xlights) {
        wxString name = model->GetName();

        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (filename.IsEmpty())
            return;
        wxXmlDocument doc = SerializeModel(model, xlights, true);
        doc.Save(filename);
    }

    // Serialize a single model into an XML document
    wxXmlDocument SerializeModel(const Model* model, xLightsFrame* xlights, bool includeGroups = false) {
        wxXmlDocument doc;

        wxXmlNode* docNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelsNodeName);
        docNode->AddAttribute(XmlNodeKeys::TypeAttribute, XmlNodeKeys::ExportedAttribute);

        XmlSerializingVisitor visitor{ docNode };

        model->Accept(visitor);
        if (includeGroups) {
            XmlSerialize::SerializeModelGroupsForModel(model, docNode);
            //TODO - add dimensions here
        }

        doc.SetRoot(docNode);

        return doc;
    }

    // Serialize all layout groups into an XML document
    void SerializeAllLayoutGroups(const std::vector<LayoutGroup*>& layoutGroups, wxXmlNode* root) {
        wxXmlNode* lgNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::LayoutGroupsType);
        for (const LayoutGroup* lg : layoutGroups) {
            wxXmlNode* lgChild = new wxXmlNode(wxXML_ELEMENT_NODE, "layoutGroup");
            lgChild->AddAttribute("name", lg->GetName());
            lgChild->AddAttribute(XmlNodeKeys::BackgroundImageAttribute, lg->GetBackgroundImage());
            lgChild->AddAttribute(XmlNodeKeys::BackgroundBrightnessAttribute, std::to_string(lg->GetBackgroundBrightness()));
            lgChild->AddAttribute(XmlNodeKeys::BackgroundAlphaAttribute, std::to_string(lg->GetBackgroundAlpha()));
            lgChild->AddAttribute(XmlNodeKeys::ScaleImageAttribute, std::to_string(lg->GetBackgroundScaled()));
            lgChild->AddAttribute("PosX", std::to_string(lg->GetPosX()));
            lgChild->AddAttribute("PosY", std::to_string(lg->GetPosY()));
            lgChild->AddAttribute("PaneWidth", std::to_string(lg->GetPaneWidth()));
            lgChild->AddAttribute("PaneHeight", std::to_string(lg->GetPaneHeight()));
            lgNode->AddChild(lgChild);
        }
        root->AddChild(lgNode);
    }

    // Serialize all objects into an XML document
    void SerializeAllObjects(const ViewObjectManager & allObjects, xLightsFrame* xlights, wxXmlNode* root) {

        wxXmlNode* objectsNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ViewObjectsNodeName);

        XmlSerializingVisitor visitor{ objectsNode };

        for (auto v = allObjects.begin(); v != allObjects.end(); ++v) {
            ViewObject* object = v->second;
            object->Accept(visitor);
        }
        
        root->AddChild(objectsNode);
    }

    // Serialize a single object into an XML document
    wxXmlDocument SerializeObject(const BaseObject& object, xLightsFrame* xlights) {
        wxXmlDocument doc;

        wxXmlNode* docNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ViewObjectsNodeName);
        docNode->AddAttribute(XmlNodeKeys::TypeAttribute, XmlNodeKeys::ExportedAttribute);

        XmlSerializingVisitor visitor{ docNode };

        object.Accept(visitor);

        doc.SetRoot(docNode);

        return doc;
    }

    // Deserialize a single model from an XML document
    Model* DeserializeModel(const wxXmlDocument& doc, xLightsFrame* xlights, bool importing) {
        wxXmlNode* root = doc.GetRoot();
        wxXmlNode* model_node = root->GetChildren();
        return DeserializeModel(model_node, xlights, importing);
    }

    // Deserialize a single model XML node
    Model* DeserializeModel(wxXmlNode* model_node, xLightsFrame* xlights, bool importing) {
        XmlDeserializingModelFactory factory{};
        Model* model = factory.Deserialize(model_node, xlights, importing);
        return model;
    }

    // Deserialize a single ViewObject from an XML document
    ViewObject* DeserializeObject(const wxXmlDocument& doc, xLightsFrame* xlights, bool importing) {
        wxXmlNode* root = doc.GetRoot();
        wxXmlNode* object_node = root->GetChildren();
        return DeserializeObject(object_node, xlights, importing);
    }

    // Deserialize a single ViewObject XML node
    ViewObject* DeserializeObject(wxXmlNode* model_node, xLightsFrame* xlights, bool importing) {
        XmlDeserializingObjectFactory factory{};
        ViewObject* object = factory.Deserialize(model_node, xlights, importing);
        return object;
    }
};
