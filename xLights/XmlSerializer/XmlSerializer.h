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
#include <wx/xml/xml.h>

struct XmlSerializer {
    XmlSerializer() {}

    // Serialize all models into an XML document
    void SerializeAllModels(const ModelManager & allModels, xLightsFrame* xlights, wxXmlNode* root) {

        wxXmlNode* modelsNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelsNodeName);
        modelsNode->AddAttribute(XmlNodeKeys::TypeAttribute, XmlNodeKeys::RGBEffectsAttribute);

        XmlSerializingVisitor visitor{ modelsNode };

        for (auto m = allModels.begin(); m != allModels.end(); ++m) {
            Model* model = m->second;
            model->Accept(visitor);
        }
        
        root->AddChild(modelsNode);
    }

    // Serializes and Saves a single model into an XML document
    void SerializeAndSaveModel(const BaseObject& object, xLightsFrame* xlights) {
        wxString name = object.GetName();

        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (filename.IsEmpty())
            return;
        wxXmlDocument doc = SerializeModel(object, xlights);
        doc.Save(filename);
    }

    // Serialize a single model into an XML document
    wxXmlDocument SerializeModel(const BaseObject& object, xLightsFrame* xlights) {
        wxXmlDocument doc;

        wxXmlNode* docNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelsNodeName);
        docNode->AddAttribute(XmlNodeKeys::TypeAttribute, XmlNodeKeys::ExportedAttribute);

        XmlSerializingVisitor visitor{ docNode };

        object.Accept(visitor);

        doc.SetRoot(docNode);

        return doc;
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
