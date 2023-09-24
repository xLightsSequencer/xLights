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
#include <wx/msgdlg.h>

#include "ViewObjectManager.h"
#include "GridlinesObject.h"
#include "RulerObject.h"
#include "ImageObject.h"
#include "MeshObject.h"
#include "TerrianObject.h"
#include "xLightsMain.h"

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
    wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE, "view_object");
    node->AddAttribute("LayoutGroup", "Unassigned");

    std::string name = GenerateObjectName(type);
    node->AddAttribute("name", name);

    node->AddAttribute("DisplayAs", type);

    if (type == "Gridlines") {
        view_object = new GridlinesObject(node, *this);
    } else if (type == "Ruler") {
        view_object = new RulerObject(node, *this);
    } else if (type == "Image") {
        view_object = new ImageObject(node, *this);
    } else if (type == "Mesh") {
        view_object = new MeshObject(node, *this);
    } else if (type == "Terrian") {
        view_object = new TerrianObject(node, *this);
    } else {
        wxMessageBox(type + " is not a valid type for View Object " + node->GetAttribute("name"));
        return nullptr;
    }

    AddViewObject(view_object);
    return view_object;
}

ViewObject* ViewObjectManager::CreateObject(wxXmlNode *node) const {
    std::string type = node->GetAttribute("DisplayAs").ToStdString();
    ViewObject *view_object = nullptr;
    if (type == "Gridlines") {
        view_object = new GridlinesObject(node, *this);
    } else if (type == "Ruler") {
        view_object = new RulerObject(node, *this);
    } else if (type == "Image") {
        view_object = new ImageObject(node, *this);
    } else if (type == "Mesh") {
        view_object = new MeshObject(node, *this);
    } else if (type == "Terrian") {
        view_object = new TerrianObject(node, *this);
    } else {
        wxASSERT(false);
    }
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

        if ("ViewObjectGroup" == view_object->GetDisplayAs()) {
            if (view_object->GetModelXml()->GetParent() != groupNode) {
                if (view_object->GetModelXml()->GetParent() != nullptr) {
                    view_object->GetModelXml()->GetParent()->RemoveChild(view_object->GetModelXml());
                }
                groupNode->AddChild(view_object->GetModelXml());
            }
        } else {
            if (view_object->GetModelXml()->GetParent() != modelNode) {
                if (view_object->GetModelXml()->GetParent() != nullptr) {
                    view_object->GetModelXml()->GetParent()->RemoveChild(view_object->GetModelXml());
                }
                modelNode->AddChild(view_object->GetModelXml());
            }
        }
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
                view_object->GetModelXml()->GetParent()->RemoveChild(view_object->GetModelXml());

                /*for (auto it2 = view_objects.begin(); it2 != view_objects.end(); ++it2) {
                    if (it2->second->GetDisplayAs() == "ObjectGroup") {
                        ModelGroup *group = (ModelGroup*)it2->second;
                        group->ModelRemoved(name);
                    }
                }*/
                view_objects.erase(it);
                delete view_object->GetModelXml();
                delete view_object;
                return;
            }
        }
    }
}

bool ViewObjectManager::MergeFromBase(const std::string& baseShowDir, bool prompt)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool changed = false;

    wxXmlDocument doc;
    doc.Load(baseShowDir + wxFileName::GetPathSeparator() + XLIGHTS_RGBEFFECTS_FILE);
    if (!doc.IsOk()) {
        return false;
    }

    wxXmlNode* objects = nullptr;
    for (wxXmlNode* m = doc.GetRoot(); m != nullptr; m = m->GetNext()) {
        for (wxXmlNode* mm = m->GetChildren(); mm != nullptr; mm = mm->GetNext()) {
            if (mm->GetName() == "view_objects") {
                objects = mm;
            }
        }
    }

    if (objects != nullptr) {
        // compare models and load changes/new models
        for (wxXmlNode* o = objects->GetChildren(); o != nullptr; o = o->GetNext()) {
            // we only update existing models that came from the base
            auto name = o->GetAttribute("name");

            auto curr = GetObject(name);

            if (curr == nullptr) {
                // model does not exist
                changed = true;
                o->AddAttribute("FromBase", "1");
                createAndAddObject(new wxXmlNode(*o));
                logger_base.debug("Adding object from base show folder: '%s'.", (const char*)name.c_str());
            } else {
                bool force = false;
                if (prompt && !curr->IsFromBase()) {
                    force = wxMessageBox(wxString::Format("Object %s found that clashes with base show directory. Do you want to take the base show directory version?", name), "Object clash", wxICON_QUESTION | wxYES_NO, xlights) == wxYES;
                }

                if (force || curr->IsFromBase()) {
                    // model does exist ... update it
                    if (force || curr->IsXmlChanged(o)) {
                        o->AddAttribute("FromBase", "1");
                        changed = true;
                        Delete(name);
                        createAndAddObject(new wxXmlNode(*o));
                        logger_base.debug("Updating object from base show folder: '%s'.", (const char*)name.c_str());
                    }
                } else {
                    logger_base.debug("Object '%s' NOT updated from base show folder as it never came from there.", (const char*)name.c_str());
                }
            }
        }
    }

    return changed;
}

bool ViewObjectManager::Rename(const std::string &oldName, const std::string &newName) {
    ViewObject *view_object = GetViewObject(oldName);
    if (view_object == nullptr) {
        return false;
    }
    view_object->GetModelXml()->DeleteAttribute("name");
    view_object->GetModelXml()->AddAttribute("name",newName);
    view_object->name = newName;

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

