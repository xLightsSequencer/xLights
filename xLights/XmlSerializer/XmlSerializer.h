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

    // Serialize all models into a SerializingVisitor
    static void SerializeAllModels(const ModelManager& allModels, BaseSerializingVisitor &visitor) {
        BaseSerializingVisitor::AttrCollector attr;
        attr.Add(XmlNodeKeys::TypeAttribute, XmlNodeKeys::RGBEffectsAttribute);
        visitor.WriteOpenTag(XmlNodeKeys::ModelsNodeName, attr);
        
        for (auto m = allModels.begin(); m != allModels.end(); ++m) {
            Model* model = m->second;
            if (model->GetDisplayAs() != "ModelGroup") {
                model->Accept(visitor);
            }
        }
        visitor.WriteCloseTag(XmlNodeKeys::ModelsNodeName);
        visitor.WriteOpenTag(XmlNodeKeys::GroupsNodeName, attr);
        for (auto m = allModels.begin(); m != allModels.end(); ++m) {
            Model* model = m->second;
            if (model->GetDisplayAs() == "ModelGroup") {
                model->Accept(visitor);
            }
        }
        visitor.WriteCloseTag(XmlNodeKeys::GroupsNodeName);
    }

    // Serialize all layout groups into an XML string fragment.
    static void SerializeAllLayoutGroups(const std::vector<LayoutGroup*>& layoutGroups, BaseSerializingVisitor &visitor) {
        BaseSerializingVisitor::AttrCollector emptyAttrs;
        visitor.WriteOpenTag(XmlNodeKeys::LayoutGroupsType, emptyAttrs);
        for (const LayoutGroup* lg : layoutGroups) {
            BaseSerializingVisitor::AttrCollector attr;
            attr.Add("name", lg->GetName());
            attr.Add(XmlNodeKeys::BackgroundImageAttribute,lg->GetBackgroundImage());
            attr.Add(XmlNodeKeys::BackgroundBrightnessAttribute, std::to_string(lg->GetBackgroundBrightness()));
            attr.Add(XmlNodeKeys::BackgroundAlphaAttribute, std::to_string(lg->GetBackgroundAlpha()));
            attr.Add(XmlNodeKeys::ScaleImageAttribute, std::to_string(lg->GetBackgroundScaled()));
            attr.Add("PosX", std::to_string(lg->GetPosX()));
            attr.Add("PosY", std::to_string(lg->GetPosY()));
            attr.Add("PaneWidth", std::to_string(lg->GetPaneWidth()));
            attr.Add("PaneHeight", std::to_string(lg->GetPaneHeight()));
            visitor.WriteOpenTag("layoutGroup", attr, true);
        }
        visitor.WriteCloseTag(XmlNodeKeys::LayoutGroupsType);
    }

    // Serialize all view objects into an XML string fragment.
    static void SerializeAllObjects(const ViewObjectManager& allObjects, BaseSerializingVisitor &visitor) {
        BaseSerializingVisitor::AttrCollector attr;
        visitor.WriteOpenTag(XmlNodeKeys::ViewObjectsNodeName, attr);
        for (auto v = allObjects.begin(); v != allObjects.end(); ++v) {
            ViewObject* object = v->second;
            object->Accept(visitor);
        }
        visitor.WriteCloseTag(XmlNodeKeys::ViewObjectsNodeName);
    }
    
    // Walk a wxXmlNode DOM element tree and write it directly into a visitor using
    // WriteOpenTag/WriteCloseTag. The node must be an element (wxXML_ELEMENT_NODE).
    // Child element nodes are recursed; text content inside elements is written via
    // AppendRaw so whitespace-only text nodes are skipped cleanly.
    static void WriteXmlNode(const wxXmlNode* node, BaseSerializingVisitor& visitor) {
        if (node == nullptr || node->GetType() != wxXML_ELEMENT_NODE) return;

        // Collect attributes from the wxXmlNode
        BaseSerializingVisitor::AttrCollector attrs;
        const wxXmlAttribute* attr = node->GetAttributes();
        while (attr != nullptr) {
            attrs.Add(attr->GetName().ToStdString(), attr->GetValue().ToStdString());
            attr = attr->GetNext();
        }

        // Determine whether this element has child element or text children
        const wxXmlNode* child = node->GetChildren();
        bool hasChildren = false;
        while (child != nullptr) {
            if (child->GetType() == wxXML_ELEMENT_NODE) {
                hasChildren = true;
                break;
            }
            if (child->GetType() == wxXML_TEXT_NODE) {
                const std::string text = child->GetContent().ToStdString();
                // Only count non-whitespace text as real content
                for (char c : text) {
                    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                        hasChildren = true;
                        break;
                    }
                }
            }
            if (hasChildren) break;
            child = child->GetNext();
        }

        const std::string name = node->GetName().ToStdString();

        if (!hasChildren) {
            // No meaningful children — self-closing tag
            visitor.WriteOpenTag(name, attrs, /*selfClose=*/true);
            return;
        }

        visitor.WriteOpenTag(name, attrs, /*selfClose=*/false);

        // Recurse over children
        child = node->GetChildren();
        while (child != nullptr) {
            if (child->GetType() == wxXML_ELEMENT_NODE) {
                WriteXmlNode(child, visitor);
            } else if (child->GetType() == wxXML_TEXT_NODE) {
                const std::string text = child->GetContent().ToStdString();
                // Skip whitespace-only text nodes
                bool allWhitespace = true;
                for (char c : text) {
                    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                        allWhitespace = false;
                        break;
                    }
                }
                if (!allWhitespace) {
                    visitor.WriteBodyText(text);
                }
            }
            child = child->GetNext();
        }

        visitor.WriteCloseTag(name);
    }

    // Serialize a single object into an XML document
    static void SerializeObject(const BaseObject& object, BaseSerializingVisitor &visitor) {
        visitor.WriteOpenTag(XmlNodeKeys::ViewObjectsNodeName, {{XmlNodeKeys::TypeAttribute, XmlNodeKeys::ExportedAttribute}});
        object.Accept(visitor);
        visitor.WriteCloseTag(XmlNodeKeys::ViewObjectsNodeName);
    }

    
    // Serializes and Saves a single model into an XML document (only used for Export)
    void SerializeAndSaveModel(const Model* model) {
        wxString name = model->GetName();

        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (filename.IsEmpty())
            return;
        ObtainAccessToURL(filename);
        wxXmlDocument doc = SerializeModel(model, true);
        doc.Save(filename);
    }

    // Serialize a single model into an XML document
    wxXmlDocument SerializeModel(const Model* model, bool includeGroups = false) {
        wxXmlDocument doc;

        wxXmlNode* docNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ModelsNodeName);
        docNode->AddAttribute(XmlNodeKeys::TypeAttribute, XmlNodeKeys::ExportedAttribute);

        XmlSerializingVisitor visitor{ docNode , includeGroups};
        model->Accept(visitor);
        if (includeGroups) {
            XmlSerialize::SerializeModelGroupsForModel(model, docNode);
            XmlSerialize::AddDimensions(docNode, model);
        }

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
