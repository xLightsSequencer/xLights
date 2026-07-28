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

#include <functional>
#include <map>
#include <string>
#include <vector>
#include <pugixml.hpp>

#include "ObjectManager.h"

class ControllerObject;
class OutputModelManager;
class UICallbacks;
class ViewObject;
class RenderContext;

#ifdef GetObject
#undef GetObject  // Windows wingdi.h defines GetObject as GetObjectW
#endif

class ViewObjectManager : public ObjectManager
{
public:
    ViewObjectManager(RenderContext* rc);
    virtual ~ViewObjectManager();

    virtual BaseObject *GetObject(const std::string &name) const override;
    ViewObject *GetViewObject(const std::string &name) const;

    UICallbacks* GetUICallbacks() const override;
    OutputModelManager* GetOutputModelManager() const override;
    ViewObject* CreateAndAddObject(const std::string &type);
    ViewObject* CreateObject(pugi::xml_node node) const;
    void AddViewObject(ViewObject *view_object);
    void Delete(const std::string &name);
    bool Rename(const std::string &oldName, const std::string &newName);

    // Controller object lifecycle. These keep the objects in step with the
    // controller list, which lives in a different file (xlights_networks.xml).
    // Call sites mirror ModelManager::RenameController / DeleteController.
    ControllerObject* GetControllerObject(const std::string& controllerName) const;
    ControllerObject* CreateControllerObject(const std::string& controllerName);
    bool RenameController(const std::string& oldName, const std::string& newName);
    bool DeleteControllerObject(const std::string& controllerName);
    // Objects whose controller no longer exists. Not an error - a base show
    // merge produces them routinely - so they are reported, never auto-deleted.
    std::vector<std::string> GetOrphanedControllerObjects(const std::function<bool(const std::string&)>& controllerExists) const;
    // Both return true if the base rgb effects file loaded and the merge ran (even if nothing
    // changed); false only if the base file could not be loaded. Optional 'changed' reports
    // whether any view object content was actually modified.
    bool MergeFromBase(const std::string& baseShowDir, bool prompt, bool& acceptAll, bool& rejectAll, bool* changed = nullptr);
    static bool MergeBaseXml(const std::string& baseShowDir, pugi::xml_node localViewObjectsNode, bool* changed = nullptr);

    void LoadViewObjects(pugi::xml_node objectNode);

    ViewObject *operator[](const std::string &name) const;
    std::map<std::string, ViewObject*>::const_iterator begin() const;
    std::map<std::string, ViewObject*>::const_iterator end() const;
    unsigned int size() const;

    void clear();
protected:
    ViewObject *createAndAddObject(pugi::xml_node node);

private:
    std::map<std::string, ViewObject *> view_objects;
    RenderContext* _renderContext;

};
