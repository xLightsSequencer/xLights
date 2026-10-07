/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "BaseShowPromotion.h"

#include "ControllerObject.h"
#include "Model.h"
#include "ModelGroup.h"
#include "ModelManager.h"
#include "ViewObject.h"
#include "ViewObjectManager.h"
#include "../outputs/Controller.h"
#include "../outputs/OutputManager.h"
#include "../utils/ExternalHooks.h"
#include "../utils/UtilFunctions.h"
#include "../XmlSerializer/XmlNodeKeys.h"
#include "../XmlSerializer/XmlSerializeFunctions.h"
#include "../XmlSerializer/XmlSerializingVisitor.h"
#include "globals.h"

#include <algorithm>
#include <filesystem>
#include <map>
#include <set>
#include <string_view>

#include <pugixml.hpp>
#include <spdlog/fmt/fmt.h>
#include <log.h>

namespace BaseShowPromotion {

namespace {

std::string NetworksPath(const std::string& baseDir) {
    return (std::filesystem::path(baseDir) / OutputManager::GetNetworksFileName()).string();
}

std::string RgbEffectsPath(const std::string& baseDir) {
    return baseDir + GetPathSeparator() + XLIGHTS_RGBEFFECTS_FILE;
}

// A missing file is an empty base, not an error: a freshly created base folder
// has neither file until something is promoted into it.
bool LoadIfPresent(const std::string& path, pugi::xml_document& doc, std::string& error) {
    if (!FileExists(path, true)) return true;
    pugi::xml_parse_result result = doc.load_file(path.c_str());
    if (!result) {
        error = fmt::format("Unable to read {}: {}", path, result.description());
        return false;
    }
    return true;
}

pugi::xml_node FindChildByAttr(pugi::xml_node parent, const char* element, const char* attr, const std::string& value) {
    for (pugi::xml_node n = parent.child(element); n; n = n.next_sibling(element)) {
        if (value == n.attribute(attr).as_string()) return n;
    }
    return {};
}

pugi::xml_node GetOrAddChild(pugi::xml_node parent, const char* name) {
    pugi::xml_node n = parent.child(name);
    return n ? n : parent.append_child(name);
}

// Put `node` (a detached copy) where the base's same-named entry was, so a
// replaced item keeps its place in the file, or at the end if it's new.
void ReplaceOrAppend(pugi::xml_node parent, pugi::xml_node existing, pugi::xml_node node) {
    if (existing) {
        parent.insert_copy_after(node, existing);
        parent.remove_child(existing);
    } else {
        parent.append_copy(node);
    }
}

// Write to a sibling temp file and rename over the original. The base folder is
// shared by every show linked to it, so a half-written file there would break
// all of them rather than just this one.
bool SaveAtomically(pugi::xml_document& doc, const std::string& path, std::string& error) {
    std::string const tmp = path + ".promote.tmp";
    if (!doc.save_file(tmp.c_str(), "  ")) {
        error = fmt::format("Unable to write {}", tmp);
        return false;
    }
    std::error_code ec;
    std::filesystem::rename(tmp, path, ec);
    if (ec) {
        std::filesystem::remove(tmp, ec);
        error = fmt::format("Unable to replace {}", path);
        return false;
    }
    return true;
}

struct BaseContents {
    std::set<std::string> controllers;
    std::map<int, std::string> controllerIds;
    std::set<std::string> models;
    std::set<std::string> groups;
    std::set<std::string> objects;
    std::set<std::string> layoutGroups;
};

void ReadBaseContents(const std::string& baseDir, BaseContents& out) {
    std::string err;
    pugi::xml_document net;
    if (LoadIfPresent(NetworksPath(baseDir), net, err)) {
        for (pugi::xml_node c = net.document_element().child("Controller"); c; c = c.next_sibling("Controller")) {
            out.controllers.insert(c.attribute("Name").as_string());
            out.controllerIds[c.attribute("Id").as_int(-1)] = c.attribute("Name").as_string();
        }
    }
    pugi::xml_document rgb;
    if (LoadIfPresent(RgbEffectsPath(baseDir), rgb, err)) {
        pugi::xml_node root = rgb.document_element();
        for (pugi::xml_node n = root.child("models").child("model"); n; n = n.next_sibling("model")) {
            out.models.insert(n.attribute("name").as_string());
        }
        for (pugi::xml_node n = root.child("modelGroups").child("modelGroup"); n; n = n.next_sibling("modelGroup")) {
            out.groups.insert(n.attribute("name").as_string());
        }
        for (pugi::xml_node n = root.child("view_objects").child("view_object"); n; n = n.next_sibling("view_object")) {
            out.objects.insert(n.attribute("name").as_string());
        }
        for (pugi::xml_node n = root.child(XmlNodeKeys::LayoutGroupsType).child("layoutGroup"); n; n = n.next_sibling("layoutGroup")) {
            out.layoutGroups.insert(n.attribute("name").as_string());
        }
    }
}

// The name before the first ':' of a start channel that chains off another
// model (">Model:1", "@Model:1") or names a controller ("!Ctrl:1").
std::string StartChannelReference(const std::string& sc, char prefix) {
    if (sc.size() < 2 || sc[0] != prefix) return {};
    auto colon = sc.find(':', 1);
    return sc.substr(1, colon == std::string::npos ? std::string::npos : colon - 1);
}

bool IsBuiltInLayoutGroup(const std::string& lg) {
    return lg.empty() || lg == "Default" || lg == "All Previews" || lg == "Unassigned";
}

class Planner {
public:
    Planner(const OutputManager& om, const ModelManager& mm, const ViewObjectManager& vom) :
        _om(om), _mm(mm), _vom(vom) {
        ReadBaseContents(om.GetBaseShowDir(), _base);
    }

    void AddController(const std::string& name, bool selected) {
        Controller* c = _om.GetController(name);
        if (c == nullptr || !_seen.insert("c:" + name).second) return;
        bool const inBase = _base.controllers.count(name) > 0;
        // A dependency the base already provides doesn't need to be written,
        // and an item already linked to the base is identical to it.
        if (inBase && (!selected || c->IsFromBase())) return;

        plan.controllers.push_back(name);
        Note(name, selected, inBase);

        auto id = _base.controllerIds.find(c->GetId());
        if (id != _base.controllerIds.end() && id->second != name) {
            plan.warnings.push_back(fmt::format("Controller '{}' has id {}, which the base show folder's controller '{}' already uses.", name, c->GetId(), id->second));
        }
        if (auto* co = _vom.GetControllerObject(name); co != nullptr) {
            AddObject(co->GetName(), false);
        }
    }

    void AddModel(const std::string& rawName, bool selected) {
        // A submodel lives inside its parent's <model> node.
        std::string name = rawName;
        if (auto slash = name.find('/'); slash != std::string::npos) {
            name = name.substr(0, slash);
            selected = false;
        }
        Model* m = _mm.GetModel(name);
        if (m == nullptr || !_seen.insert("m:" + name).second) return;

        bool const isGroup = m->GetDisplayAs() == DisplayAsType::ModelGroup;
        bool const inBase = (isGroup ? _base.groups : _base.models).count(name) > 0;
        if (inBase && (!selected || m->IsFromBase())) return;

        (isGroup ? plan.groups : plan.models).push_back(name);
        Note(name, selected, inBase);

        if (!IsBuiltInLayoutGroup(m->GetLayoutGroup()) && _base.layoutGroups.count(m->GetLayoutGroup()) == 0) {
            plan.warnings.push_back(fmt::format("'{}' is on the '{}' preview, which the base show folder doesn't have.", name, m->GetLayoutGroup()));
        }

        if (isGroup) {
            for (const auto& member : static_cast<ModelGroup*>(m)->ModelNames()) {
                AddModel(member, false);
            }
            return;
        }

        std::string const& ctrl = m->GetControllerName();
        if (!ctrl.empty()) AddController(ctrl, false);
        std::string const sc = m->GetModelStartChannel();
        if (auto c = StartChannelReference(sc, '!'); !c.empty()) AddController(c, false);
        if (auto r = StartChannelReference(sc, '>'); !r.empty()) AddModel(r, false);
        if (auto r = StartChannelReference(sc, '@'); !r.empty()) AddModel(r, false);
    }

    void AddObject(const std::string& name, bool selected) {
        ViewObject* o = _vom.GetViewObject(name);
        if (o == nullptr || !_seen.insert("o:" + name).second) return;
        bool const inBase = _base.objects.count(name) > 0;
        if (inBase && (!selected || o->IsFromBase())) return;

        plan.objects.push_back(name);
        Note(name, selected, inBase);

        if (auto* co = dynamic_cast<ControllerObject*>(o); co != nullptr) {
            AddController(co->GetControllerName(), false);
        }
    }

    Plan plan;

private:
    void Note(const std::string& name, bool selected, bool inBase) {
        if (!selected) plan.pulledIn.push_back(name);
        if (inBase) plan.replaced.push_back(name);
    }

    const OutputManager& _om;
    const ModelManager& _mm;
    const ViewObjectManager& _vom;
    BaseContents _base;
    std::set<std::string> _seen;
};

void AppendList(std::string& out, const char* heading, const std::vector<std::string>& items) {
    if (items.empty()) return;
    // A large group pulls in every member; keep the prompt a readable size.
    constexpr size_t maxShown = 12;
    out += fmt::format("\n{}:\n", heading);
    for (size_t i = 0; i < items.size() && i < maxShown; ++i) {
        out += "    " + items[i] + "\n";
    }
    if (items.size() > maxShown) {
        out += fmt::format("    ... and {} more\n", items.size() - maxShown);
    }
}

pugi::xml_node SerializeForBase(const BaseObject& obj, pugi::xml_node scratch, const std::string& baseDir) {
    // forExport keeps file references absolute; the ones under the base folder
    // are then made relative to it, the way the base folder stores its own.
    XmlSerializingVisitor visitor{ scratch, true };
    obj.Accept(visitor);
    pugi::xml_node node = scratch.last_child();
    node.remove_attribute(XmlNodeKeys::FromBaseAttribute);
    node.remove_attribute("BaseModels");
    // Export-only sizing hints for .xmodel import; a layout never carries them,
    // and left in they'd make every later merge see the model as changed.
    node.remove_attribute("widthmm");
    node.remove_attribute("heightmm");
    node.remove_attribute("depthmm");
    XmlSerialize::RelativizeFileReferences(node, baseDir);
    return node;
}

} // namespace

Plan BuildPlan(const OutputManager& om, const ModelManager& models, const ViewObjectManager& objects,
               const std::vector<std::string>& controllerNames,
               const std::vector<std::string>& modelNames,
               const std::vector<std::string>& objectNames) {
    Planner p(om, models, objects);
    for (const auto& n : controllerNames) p.AddController(n, true);
    for (const auto& n : modelNames) p.AddModel(n, true);
    for (const auto& n : objectNames) p.AddObject(n, true);
    return std::move(p.plan);
}

std::string Describe(const Plan& plan, const std::string& baseShowDir) {
    std::string out = fmt::format("Copy {} item{} into the base show folder\n    {}\nand link {} to it?\n",
                                  plan.size(), plan.size() == 1 ? "" : "s", baseShowDir,
                                  plan.size() == 1 ? "it" : "them");
    AppendList(out, "Controllers", plan.controllers);
    AppendList(out, "Models", plan.models);
    AppendList(out, "Groups", plan.groups);
    AppendList(out, "Objects", plan.objects);
    AppendList(out, "Included because the selection depends on them", plan.pulledIn);
    AppendList(out, "Already in the base show folder (its copy will be replaced)", plan.replaced);
    AppendList(out, "Warnings", plan.warnings);
    return out;
}

bool Apply(const Plan& plan, OutputManager& om, ModelManager& models, ViewObjectManager& objects, std::string& error) {
    std::string const baseDir = om.GetBaseShowDir();
    if (baseDir.empty()) {
        error = "No base show folder is configured.";
        return false;
    }
    if (plan.empty()) return true;

    // Controllers first: a model written into the base before its controller
    // would load there with a dangling controller reference.
    if (!plan.controllers.empty()) {
        std::string const path = NetworksPath(baseDir);
        pugi::xml_document doc;
        if (!LoadIfPresent(path, doc, error)) return false;
        pugi::xml_node root = doc.document_element();
        if (!root) {
            pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
            decl.append_attribute("version") = "1.0";
            decl.append_attribute("encoding") = "UTF-8";
            root = doc.append_child("Networks");
        }
        for (const auto& name : plan.controllers) {
            Controller* c = om.GetController(name);
            if (c == nullptr) continue;
            pugi::xml_document scratch;
            pugi::xml_node node = c->Save(scratch.append_child("scratch"));
            node.attribute("FromBase").set_value("0");
            ReplaceOrAppend(root, FindChildByAttr(root, "Controller", "Name", name), node);
        }
        if (!SaveAtomically(doc, path, error)) return false;
        spdlog::info("Promoted {} controller(s) to base show folder.", plan.controllers.size());
    }

    if (!plan.models.empty() || !plan.groups.empty() || !plan.objects.empty()) {
        std::string const path = RgbEffectsPath(baseDir);
        pugi::xml_document doc;
        if (!LoadIfPresent(path, doc, error)) return false;
        pugi::xml_node root = doc.document_element();
        if (!root) root = doc.append_child("xrgb");

        pugi::xml_document scratchDoc;
        pugi::xml_node scratch = scratchDoc.append_child("scratch");

        pugi::xml_node modelsNode = GetOrAddChild(root, "models");
        for (const auto& name : plan.models) {
            if (Model* m = models.GetModel(name); m != nullptr) {
                ReplaceOrAppend(modelsNode, FindChildByAttr(modelsNode, "model", "name", name), SerializeForBase(*m, scratch, baseDir));
            }
        }
        if (!plan.groups.empty()) {
            pugi::xml_node groupsNode = GetOrAddChild(root, "modelGroups");
            for (const auto& name : plan.groups) {
                if (Model* m = models.GetModel(name); m != nullptr) {
                    ReplaceOrAppend(groupsNode, FindChildByAttr(groupsNode, "modelGroup", "name", name), SerializeForBase(*m, scratch, baseDir));
                }
            }
        }
        if (!plan.objects.empty()) {
            pugi::xml_node objectsNode = GetOrAddChild(root, "view_objects");
            for (const auto& name : plan.objects) {
                if (ViewObject* o = objects.GetViewObject(name); o != nullptr) {
                    ReplaceOrAppend(objectsNode, FindChildByAttr(objectsNode, "view_object", "name", name), SerializeForBase(*o, scratch, baseDir));
                }
            }
        }
        if (!SaveAtomically(doc, path, error)) {
            if (!plan.controllers.empty()) {
                error += "\nThe controllers were already copied into the base show folder.";
            }
            return false;
        }
        spdlog::info("Promoted {} model(s), {} group(s), {} object(s) to base show folder.",
                     plan.models.size(), plan.groups.size(), plan.objects.size());
    }

    for (const auto& name : plan.controllers) {
        if (Controller* c = om.GetController(name); c != nullptr) c->SetFromBase(true);
    }
    for (const auto& name : plan.models) {
        if (Model* m = models.GetModel(name); m != nullptr) m->SetFromBase(true);
    }
    for (const auto& name : plan.groups) {
        if (Model* m = models.GetModel(name); m != nullptr) m->SetFromBase(true);
    }
    for (const auto& name : plan.objects) {
        if (ViewObject* o = objects.GetViewObject(name); o != nullptr) o->SetFromBase(true);
    }
    return true;
}

} // namespace BaseShowPromotion
