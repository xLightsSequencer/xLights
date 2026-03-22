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

#include <wx/xml/xml.h>
#include <wx/msgdlg.h>

#include "ViewObjectManager.h"
#include "UtilFunctions.h"
#include "GridlinesObject.h"
#include "RulerObject.h"
#include "ImageObject.h"
#include "MeshObject.h"
#include "TerrainObject.h"
#include "xLightsMain.h"
#include "ModelGroup.h"
#include "XmlSerializer/XmlSerializer.h"

#include <log4cpp/Category.hh>

ViewObjectManager::ViewObjectManager(xLightsFrame* xl) : xlights(xl)
{
    //ctor
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
        wxMessageBox(type + " is not a valid type for View Object ");
        return nullptr;
    }
    view_object->SetName(GenerateObjectName(type));
    AddViewObject(view_object);

    return view_object;
}

ViewObject* ViewObjectManager::CreateObject(wxXmlNode *node) const {
    std::string type = node->GetAttribute("DisplayAs").ToStdString();
    XmlSerializer serializer;
    ViewObject* view_object {nullptr};
    view_object = serializer.DeserializeObject(node, xlights, false);
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

void ViewObjectManager::LoadViewObjects(wxXmlNode *modelNode) {
    clear();
    this->modelNode = modelNode;
    for (wxXmlNode* e=modelNode->GetChildren(); e!=nullptr; e=e->GetNext()) {
        if (e->GetName() == "view_object") {
            std::string name = e->GetAttribute("name").ToStdString();
            if (!name.empty()) {
                createAndAddObject(e);
            }
        }
    }
}

ViewObject *ViewObjectManager::createAndAddObject(wxXmlNode *node) {
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
static wxXmlNode* FindVOChildByNameAttr(wxXmlNode* parent, const wxString& childTag, const wxString& name)
{
    for (wxXmlNode* n = parent->GetChildren(); n != nullptr; n = n->GetNext()) {
        if (n->GetName() == childTag && n->GetAttribute("name") == name) {
            return n;
        }
    }
    return nullptr;
}

// Helper: set attribute (delete + re-add)
static void SetVOXmlAttribute(wxXmlNode* node, const wxString& attr, const wxString& value)
{
    if (node->HasAttribute(attr)) node->DeleteAttribute(attr);
    node->AddAttribute(attr, value);
}

// Helper: compare name attributes on child nodes
static bool CheckVONameAttrs(wxXmlNode* nn, wxXmlNode* cc)
{
    if (nn->HasAttribute("name")) {
        return (cc->GetAttribute("name") == nn->GetAttribute("name"));
    } else if (nn->HasAttribute("Name")) {
        return (cc->GetAttribute("Name") == nn->GetAttribute("Name"));
    }
    return true;
}

// Helper: compare two XML nodes for changes
static bool IsVOXmlNodeChanged(wxXmlNode* local, wxXmlNode* base)
{
    for (wxXmlAttribute* a = base->GetAttributes(); a != nullptr; a = a->GetNext()) {
        if (!local->HasAttribute(a->GetName()) || local->GetAttribute(a->GetName()) != a->GetValue()) {
            return true;
        }
    }
    for (wxXmlNode* nn = base->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
        bool found = false;
        for (wxXmlNode* cc = local->GetChildren(); cc != nullptr; cc = cc->GetNext()) {
            if (cc->GetName() == nn->GetName() && CheckVONameAttrs(nn, cc)) {
                found = true;
                for (wxXmlAttribute* a = nn->GetAttributes(); a != nullptr; a = a->GetNext()) {
                    if (!cc->HasAttribute(a->GetName()) || cc->GetAttribute(a->GetName()) != a->GetValue()) {
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
static bool MergeBaseIntoCurrentObjectsXml(wxXmlNode* currentObjectsNode, wxXmlNode* baseObjectsNode,
                                           std::vector<std::string>& changedObjects)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool changed = false;

    if (baseObjectsNode == nullptr || currentObjectsNode == nullptr) return false;

    for (wxXmlNode* bo = baseObjectsNode->GetChildren(); bo != nullptr; bo = bo->GetNext()) {
        if (bo->GetName() != "view_object") continue;
        auto name = bo->GetAttribute("name");
        if (name.empty()) continue;

        wxXmlNode* local = FindVOChildByNameAttr(currentObjectsNode, "view_object", name);

        if (local == nullptr) {
            // Object does not exist locally -- add it
            wxXmlNode* copy = new wxXmlNode(*bo);
            SetVOXmlAttribute(copy, "FromBase", "1");
            currentObjectsNode->AddChild(copy);
            changedObjects.push_back(name.ToStdString());
            changed = true;
            logger_base.debug("MergeBase: Adding view object from base: '%s'.", (const char*)name.c_str());
        } else if (local->GetAttribute("FromBase") == "1") {
            // Object exists and came from base -- update if changed
            if (IsVOXmlNodeChanged(local, bo)) {
                wxXmlNode* copy = new wxXmlNode(*bo);
                SetVOXmlAttribute(copy, "FromBase", "1");

                currentObjectsNode->InsertChildAfter(copy, local);
                currentObjectsNode->RemoveChild(local);
                delete local;
                changedObjects.push_back(name.ToStdString());
                changed = true;
                logger_base.debug("MergeBase: Updating view object from base: '%s'.", (const char*)name.c_str());
            }
        }
        // If object exists locally without FromBase, skip silently
    }

    return changed;
}

// Load base XML document and find <view_objects> node
static bool LoadBaseObjectXmlNodes(const std::string& baseShowDir, wxXmlDocument& doc, wxXmlNode*& baseObjects)
{
    baseObjects = nullptr;
    doc.Load(baseShowDir + GetPathSeparator() + XLIGHTS_RGBEFFECTS_FILE);
    if (!doc.IsOk()) return false;

    for (wxXmlNode* m = doc.GetRoot(); m != nullptr; m = m->GetNext()) {
        for (wxXmlNode* mm = m->GetChildren(); mm != nullptr; mm = mm->GetNext()) {
            if (mm->GetName() == "view_objects") baseObjects = mm;
        }
    }
    return true;
}

bool ViewObjectManager::MergeBaseXml(const std::string& baseShowDir, wxXmlNode* localViewObjectsNode)
{
    wxXmlDocument baseDoc;
    wxXmlNode* baseObjects = nullptr;
    if (!LoadBaseObjectXmlNodes(baseShowDir, baseDoc, baseObjects)) return false;

    std::vector<std::string> changedObjects;
    return MergeBaseIntoCurrentObjectsXml(localViewObjectsNode, baseObjects, changedObjects);
}

bool ViewObjectManager::MergeFromBase(const std::string& baseShowDir, bool prompt)
{
    wxXmlDocument baseDoc;
    wxXmlNode* baseObjects = nullptr;
    if (!LoadBaseObjectXmlNodes(baseShowDir, baseDoc, baseObjects)) return false;
    if (baseObjects == nullptr) return false;

    // Handle prompt mode: ask user about non-FromBase objects that clash with base
    if (prompt) {
        for (wxXmlNode* bo = baseObjects->GetChildren(); bo != nullptr; bo = bo->GetNext()) {
            if (bo->GetName() != "view_object") continue;
            auto name = bo->GetAttribute("name");
            if (name.empty()) continue;
            auto curr = GetObject(name);
            if (curr != nullptr && !curr->IsFromBase()) {
                if (wxMessageBox(std::format("Object {} found that clashes with base show directory. Do you want to take the base show directory version?", name.ToStdString()),
                                 "Object clash", wxICON_QUESTION | wxYES_NO, xlights) == wxYES) {
                    curr->SetFromBase(true);
                }
            }
        }
    }

    // Serialize all current view objects to a temporary XML node for comparison
    wxXmlNode* currentObjectsNode = new wxXmlNode(wxXML_ELEMENT_NODE, "view_objects");
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
        wxXmlNode* updatedNode = FindVOChildByNameAttr(currentObjectsNode, "view_object", name);
        if (updatedNode != nullptr) {
            wxXmlNode* copy = new wxXmlNode(*updatedNode);
            auto curr = GetViewObject(name);
            if (curr != nullptr) {
                Delete(name);
            }
            createAndAddObject(copy);
        }
    }

    // Clean up temporary XML node
    delete currentObjectsNode;

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

