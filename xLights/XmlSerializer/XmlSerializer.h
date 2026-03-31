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
#include <pugixml.hpp>

#include "ui/layout/LayoutGroup.h"
#include "utils/ExternalHooks.h"

struct XmlSerializer {
    XmlSerializer() {}

    // Serialize all models into a SerializingVisitor
    static void SerializeAllModels(const ModelManager& allModels, BaseSerializingVisitor &visitor) {
        BaseSerializingVisitor::AttrCollector attr;
        attr.Add(XmlNodeKeys::TypeAttribute, XmlNodeKeys::RGBEffectsAttribute);
        visitor.WriteOpenTag(XmlNodeKeys::ModelsNodeName, attr);

        for (auto m = allModels.begin(); m != allModels.end(); ++m) {
            Model* model = m->second;
            if (model->GetDisplayAs() != DisplayAsType::ModelGroup) {
                model->Accept(visitor);
            }
        }
        visitor.WriteCloseTag();
        visitor.WriteOpenTag(XmlNodeKeys::GroupsNodeName, attr);
        for (auto m = allModels.begin(); m != allModels.end(); ++m) {
            Model* model = m->second;
            if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
                model->Accept(visitor);
            }
        }
        visitor.WriteCloseTag();
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
        visitor.WriteCloseTag();
    }

    // Serialize all view objects into an XML string fragment.
    static void SerializeAllObjects(const ViewObjectManager& allObjects, BaseSerializingVisitor &visitor) {
        BaseSerializingVisitor::AttrCollector attr;
        visitor.WriteOpenTag(XmlNodeKeys::ViewObjectsNodeName, attr);
        for (auto v = allObjects.begin(); v != allObjects.end(); ++v) {
            ViewObject* object = v->second;
            object->Accept(visitor);
        }
        visitor.WriteCloseTag();
    }

    // Walk a pugi::xml_node DOM element tree and write it directly into a visitor using
    // WriteOpenTag/WriteCloseTag. The node must be an element (node_element).
    // Child element nodes are recursed; text content inside elements is written via
    // WriteBodyText so whitespace-only text nodes are skipped cleanly.
    static void WriteXmlNode(pugi::xml_node node, BaseSerializingVisitor& visitor) {
        if (!node || node.type() != pugi::node_element) return;

        // Collect attributes from the node
        BaseSerializingVisitor::AttrCollector attrs;
        for (pugi::xml_attribute attr = node.first_attribute(); attr; attr = attr.next_attribute()) {
            attrs.Add(attr.name(), attr.value());
        }

        // Determine whether this element has child element or text children
        bool hasChildren = false;
        for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
            if (child.type() == pugi::node_element) {
                hasChildren = true;
                break;
            }
            if (child.type() == pugi::node_pcdata) {
                const char* text = child.value();
                // Only count non-whitespace text as real content
                for (const char* c = text; *c; ++c) {
                    if (*c != ' ' && *c != '\t' && *c != '\n' && *c != '\r') {
                        hasChildren = true;
                        break;
                    }
                }
            }
            if (hasChildren) break;
        }

        std::string name = node.name();

        if (!hasChildren) {
            // No meaningful children — self-closing tag
            visitor.WriteOpenTag(name, attrs, /*selfClose=*/true);
            return;
        }

        visitor.WriteOpenTag(name, attrs, /*selfClose=*/false);

        // Recurse over children
        for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
            if (child.type() == pugi::node_element) {
                WriteXmlNode(child, visitor);
            } else if (child.type() == pugi::node_pcdata) {
                const char* text = child.value();
                // Skip whitespace-only text nodes
                bool allWhitespace = true;
                for (const char* c = text; *c; ++c) {
                    if (*c != ' ' && *c != '\t' && *c != '\n' && *c != '\r') {
                        allWhitespace = false;
                        break;
                    }
                }
                if (!allWhitespace) {
                    visitor.WriteBodyText(text);
                }
            }
        }

        visitor.WriteCloseTag();
    }

    // Serialize a single object into an XML document
    static void SerializeObject(const BaseObject& object, BaseSerializingVisitor &visitor) {
        visitor.WriteOpenTag(XmlNodeKeys::ViewObjectsNodeName, {{XmlNodeKeys::TypeAttribute, XmlNodeKeys::ExportedAttribute}});
        object.Accept(visitor);
        visitor.WriteCloseTag();
    }


    // Serializes and Saves a single model into an XML document (only used for Export)
    void SerializeAndSaveModel(const Model* model) {
        wxString name = model->GetName();

        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (filename.IsEmpty())
            return;
        ObtainAccessToURL(filename);
        pugi::xml_document doc = SerializeModel(model, true);
        doc.save_file(filename.ToStdString().c_str());
    }

    // Serialize a single model into an XML document
    pugi::xml_document SerializeModel(const Model* model, bool includeGroups = false) {
        pugi::xml_document doc;

        pugi::xml_node docNode = doc.append_child(XmlNodeKeys::ModelsNodeName);
        docNode.append_attribute(XmlNodeKeys::TypeAttribute) = XmlNodeKeys::ExportedAttribute;

        XmlSerializingVisitor visitor{ docNode, includeGroups };
        model->Accept(visitor);
        if (includeGroups) {
            XmlSerialize::SerializeModelGroupsForModel(model, docNode);
            XmlSerialize::AddDimensions(docNode, model);
        }

        return doc;
    }

    // Deserialize a single model from an XML document
    Model* DeserializeModel(const pugi::xml_document& doc, xLightsFrame* xlights, bool importing) {
        pugi::xml_node root = doc.document_element();
        if (!root) return nullptr;
        pugi::xml_node model_node = root.first_child();
        if (!model_node) return nullptr;
        return DeserializeModel(model_node, xlights, importing);
    }

    // Deserialize a single model XML node
    Model* DeserializeModel(pugi::xml_node model_node, xLightsFrame* xlights, bool importing) {
        XmlDeserializingModelFactory factory{};
        Model* model = factory.Deserialize(model_node, xlights, importing);
        return model;
    }

    // Deserialize a single ViewObject from an XML document
    ViewObject* DeserializeObject(const pugi::xml_document& doc, xLightsFrame* xlights, bool importing) {
        pugi::xml_node root = doc.document_element();
        if (!root) return nullptr;
        pugi::xml_node object_node = root.first_child();
        if (!object_node) return nullptr;
        return DeserializeObject(object_node, xlights, importing);
    }

    // Deserialize a single ViewObject XML node
    ViewObject* DeserializeObject(pugi::xml_node model_node, xLightsFrame* xlights, bool importing) {
        XmlDeserializingObjectFactory factory{};
        ViewObject* object = factory.Deserialize(model_node, xlights, importing);
        return object;
    }
};
