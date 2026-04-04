/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <format>

#include <pugixml.hpp>

#include "ViewObjectManager.h"
#include "../render/UICallbacks.h"
#include "UtilFunctions.h"
#include "GridlinesObject.h"
#include "RulerObject.h"
#include "ImageObject.h"
#include "MeshObject.h"
#include "TerrainObject.h"
#include "xLightsMain.h"
#include "ModelGroup.h"
#include "XmlSerializer/XmlSerializer.h"

#include <log.h>

#ifdef GetObject
#undef GetObject  // Windows wingdi.h defines GetObject as GetObjectW
#endif

ViewObjectManager::ViewObjectManager(xLightsFrame* xl) : xlights(xl)
{
    //ctor
}

UICallbacks* ViewObjectManager::GetUICallbacks() const {
    return xlights ? xlights->GetUICallbacks() : nullptr;
}

OutputModelManager* ViewObjectManager::GetOutputModelManager() const {
    return xlights ? xlights->GetOutputModelManager() : nullptr;
}

ViewObjectManager::~ViewObjectManager()
{
    //dtor
    clear();
}

BaseObject* ViewObjectManager::GetObject(const std::string &name) const
{
    return (BaseObject*)GetViewObject(name);
}

ViewObject* ViewObjectManager::GetViewObject(const std::string &name) const
{
    auto it = view_objects.find(name);
    if (it == view_objects.end()) {
        return nullptr;
    }
    return it->second;
}

ViewObject* ViewObjectManager::CreateAndAddObject(const std::string &type) {
    ViewObject *view_object;
    
    if (type == "Gridlines") {
        view_object = new GridlinesObject(*this);
    } else if (type == "Ruler") {
        view_object = new RulerObject(*this);
    } else if (type == "Image") {
        view_object = new ImageObject(*this);
    } else if (type == "Mesh") {
        view_object = new MeshObject(*this);
    } else if (type == "Terrain") {
        view_object = new TerrainObject(*this);
    } else {
        if (auto* ui = GetUICallbacks()) {
            ui->ShowMessage(type + " is not a valid type for View Object ");
        }
        return nullptr;
    }
    view_object->SetName(GenerateObjectName(type));
    AddViewObject(view_object);

    return view_object;
}

ViewObject* ViewObjectManager::CreateObject(pugi::xml_node node) const {
    std::string type = node.attribute("DisplayAs").as_string();
    XmlSerializer serializer;
    ViewObject* view_object {nullptr};
    view_object = serializer.DeserializeObject(node, const_cast<ViewObjectManager&>(*this), false);
    return view_object;
}

void ViewObjectManager::AddViewObject(ViewObject *view_object) {
    if (view_object != nullptr) {
        auto it = view_objects.find(view_object->name);
        if (it != view_objects.end()) {
            delete it->second;
            it->second = nullptr;
        }
        view_objects[view_object->name] = view_object;
    }
}

void ViewObjectManager::clear() {
    for (auto it = view_objects.begin(); it != view_objects.end(); ++it) {
        if (it->second != nullptr) {
            delete it->second;
            it->second = nullptr;
        }
    }
    view_objects.clear();
}

void ViewObjectManager::LoadViewObjects(pugi::xml_node modelNode) {
    clear();
    for (pugi::xml_node e = modelNode.first_child(); e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "view_object") {
            std::string name = e.attribute("name").as_string();
            if (!name.empty()) {
                createAndAddObject(e);
            }
        }
    }
}

ViewObject *ViewObjectManager::createAndAddObject(pugi::xml_node node) {
    ViewObject* view_object = CreateObject(node);
    AddViewObject(view_object);
    return view_object;
}

void ViewObjectManager::Delete(const std::string &name) {

    if (xlights->CurrentSeqXmlFile != nullptr) {
        Element* elem_to_delete = xlights->GetSequenceElements().GetElement(name);
        if (elem_to_delete != nullptr) {
            // Delete the object from the sequencer grid and views
            xlights->GetSequenceElements().DeleteElement(name);
        }
    }

    // now delete the view_object
    for (auto it = view_objects.begin(); it != view_objects.end(); ++it) {
        if (it->first == name) {
            ViewObject *view_object = it->second;

            if (view_object != nullptr) {
                /*for (auto it2 = view_objects.begin(); it2 != view_objects.end(); ++it2) {
                    if (it2->second->GetDisplayAs() == "ObjectGroup") {
                        ModelGroup *group = (ModelGroup*)it2->second;
                        group->ModelRemoved(name);
                    }
                }*/
                view_objects.erase(it);
                delete view_object;
                return;
            }
        }
    }
}

// Helper: find a child XML node by tag name and "name" attribute
static pugi::xml_node FindVOChildByNameAttr(pugi::xml_node parent, std::string_view childTag, std::string_view name)
{
    for (pugi::xml_node n = parent.first_child(); n; n = n.next_sibling()) {
        if (std::string_view(n.name()) == childTag && std::string_view(n.attribute("name").as_string()) == name) {
            return n;
        }
    }
    return {};
}

// Helper: set attribute (add or update)
static void SetVOXmlAttribute(pugi::xml_node node, const char* attr, const char* value)
{
    auto a = node.attribute(attr);
    if (!a) {
        a = node.append_attribute(attr);
    }
    a.set_value(value);
}

// Helper: compare name attributes on child nodes
static bool CheckVONameAttrs(pugi::xml_node nn, pugi::xml_node cc)
{
    if (!nn.attribute("name").empty()) {
        return std::string_view(cc.attribute("name").as_string()) == std::string_view(nn.attribute("name").as_string());
    } else if (!nn.attribute("Name").empty()) {
        return std::string_view(cc.attribute("Name").as_string()) == std::string_view(nn.attribute("Name").as_string());
    }
    return true;
}

// Helper: compare two XML nodes for changes
static bool IsVOXmlNodeChanged(pugi::xml_node local, pugi::xml_node base)
{
    for (auto a : base.attributes()) {
        auto localAttr = local.attribute(a.name());
        if (localAttr.empty() || std::string_view(localAttr.as_string()) != std::string_view(a.as_string())) {
            return true;
        }
    }
    for (pugi::xml_node nn = base.first_child(); nn; nn = nn.next_sibling()) {
        bool found = false;
        for (pugi::xml_node cc = local.first_child(); cc; cc = cc.next_sibling()) {
            if (std::string_view(cc.name()) == std::string_view(nn.name()) && CheckVONameAttrs(nn, cc)) {
                found = true;
                for (auto a : nn.attributes()) {
                    auto ccAttr = cc.attribute(a.name());
                    if (ccAttr.empty() || std::string_view(ccAttr.as_string()) != std::string_view(a.as_string())) {
                        return true;
                    }
                }
            }
        }
        if (!found) return true;
    }
    return false;
}

// Shared method: merge base view objects XML into current XML, updating currentObjectsNode in-place.
// Populates changedObjects with names of objects that were added or updated.
static bool MergeBaseIntoCurrentObjectsXml(pugi::xml_node currentObjectsNode, pugi::xml_node baseObjectsNode,
                                           std::vector<std::string>& changedObjects)
{
    
    bool changed = false;

    if (!baseObjectsNode || !currentObjectsNode) return false;

    for (pugi::xml_node bo = baseObjectsNode.first_child(); bo; bo = bo.next_sibling()) {
        if (std::string_view(bo.name()) != "view_object") continue;
        std::string name = bo.attribute("name").as_string();
        if (name.empty()) continue;

        pugi::xml_node local = FindVOChildByNameAttr(currentObjectsNode, "view_object", name);

        if (!local) {
            // Object does not exist locally -- add it
            pugi::xml_node copy = currentObjectsNode.append_copy(bo);
            SetVOXmlAttribute(copy, "FromBase", "1");
            changedObjects.push_back(name);
            changed = true;
            spdlog::debug("MergeBase: Adding view object from base: '{}'.", name.c_str());
        } else if (std::string_view(local.attribute("FromBase").as_string()) == "1") {
            // Object exists and came from base -- update if changed
            if (IsVOXmlNodeChanged(local, bo)) {
                pugi::xml_node copy = currentObjectsNode.insert_copy_after(bo, local);
                SetVOXmlAttribute(copy, "FromBase", "1");

                currentObjectsNode.remove_child(local);
                changedObjects.push_back(name);
                changed = true;
                spdlog::debug("MergeBase: Updating view object from base: '{}'.", name.c_str());
            }
        }
        // If object exists locally without FromBase, skip silently
    }

    return changed;
}

// Load base XML document and find <view_objects> node
static bool LoadBaseObjectXmlNodes(const std::string& baseShowDir, pugi::xml_document& doc, pugi::xml_node& baseObjects)
{
    baseObjects = {};
    std::string path = baseShowDir + GetPathSeparator() + XLIGHTS_RGBEFFECTS_FILE;
    pugi::xml_parse_result result = doc.load_file(path.c_str());
    if (!result) return false;

    pugi::xml_node root = doc.document_element();
    if (root) {
        for (pugi::xml_node mm = root.first_child(); mm; mm = mm.next_sibling()) {
            if (std::string_view(mm.name()) == "view_objects") baseObjects = mm;
        }
    }
    return true;
}

bool ViewObjectManager::MergeBaseXml(const std::string& baseShowDir, pugi::xml_node localViewObjectsNode)
{
    pugi::xml_document baseDoc;
    pugi::xml_node baseObjects;
    if (!LoadBaseObjectXmlNodes(baseShowDir, baseDoc, baseObjects)) return false;

    std::vector<std::string> changedObjects;
    return MergeBaseIntoCurrentObjectsXml(localViewObjectsNode, baseObjects, changedObjects);
}

bool ViewObjectManager::MergeFromBase(const std::string& baseShowDir, bool prompt)
{
    pugi::xml_document baseDoc;
    pugi::xml_node baseObjects;
    if (!LoadBaseObjectXmlNodes(baseShowDir, baseDoc, baseObjects)) return false;
    if (!baseObjects) return false;

    // Handle prompt mode: ask user about non-FromBase objects that clash with base
    if (prompt) {
        for (pugi::xml_node bo = baseObjects.first_child(); bo; bo = bo.next_sibling()) {
            if (std::string_view(bo.name()) != "view_object") continue;
            std::string name = bo.attribute("name").as_string();
            if (name.empty()) continue;
            auto curr = GetObject(name);
            if (curr != nullptr && !curr->IsFromBase()) {
                if (auto* ui = GetUICallbacks()) {
                    if (ui->PromptYesNo(std::format("Object {} found that clashes with base show directory. Do you want to take the base show directory version?", name),
                                        "Object clash")) {
                        curr->SetFromBase(true);
                    }
                }
            }
        }
    }

    // Serialize all current view objects to a temporary XML document for comparison
    pugi::xml_document tempDoc;
    pugi::xml_node currentObjectsNode = tempDoc.append_child("view_objects");
    {
        XmlSerializingVisitor visitor{currentObjectsNode};
        for (auto v = begin(); v != end(); ++v) {
            ViewObject* obj = v->second;
            obj->Accept(visitor);
        }
    }

    // Run the shared merge
    std::vector<std::string> changedObjects;
    bool changed = MergeBaseIntoCurrentObjectsXml(currentObjectsNode, baseObjects, changedObjects);

    // Apply changes: replace changed objects with new ones created from the updated XML
    for (const auto& name : changedObjects) {
        pugi::xml_node updatedNode = FindVOChildByNameAttr(currentObjectsNode, "view_object", name);
        if (updatedNode) {
            // Copy the node into a persistent document so it outlives the loop
            pugi::xml_document copyDoc;
            pugi::xml_node copy = copyDoc.append_copy(updatedNode);
            auto curr = GetViewObject(name);
            if (curr != nullptr) {
                Delete(name);
            }
            createAndAddObject(copy);
        }
    }

    return changed;
}

bool ViewObjectManager::Rename(const std::string &oldName, const std::string &newName) {
    ViewObject *view_object = GetViewObject(oldName);
    if (view_object == nullptr) {
        return false;
    }
    view_object->SetName(newName);

    bool changed = false;
    //for (auto it2 = view_objects.begin(); it2 != view_objects.end(); ++it2) {
    //    changed |= it2->second->ModelRenamed(oldName, newName);
    //}
    view_objects.erase(view_objects.find(oldName));
    view_objects[newName] = view_object;

    // go through all the view_object groups looking for things that might need to be renamed
    /*for (auto it = view_objects.begin(); it != view_objects.end(); ++it) {
        ModelGroup* mg = dynamic_cast<ModelGroup*>(it->second);
        if (mg != nullptr)
        {
            changed |= mg->ModelRenamed(oldName, newName);
        }
    }*/

    return changed;
}

std::map<std::string, ViewObject*>::const_iterator ViewObjectManager::begin() const {
    return view_objects.begin();
}

std::map<std::string, ViewObject*>::const_iterator ViewObjectManager::end() const {
    return view_objects.end();
}

unsigned int ViewObjectManager::size() const {
    return view_objects.size();
}

ViewObject *ViewObjectManager::operator[](const std::string &name) const {
    return GetViewObject(name);
}

