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

#include <string>
#include "../models/OutputModelManager.h"
#include "BaseObjectVisitor.h"
#include "DisplayAsType.h"
#include "ModelScreenLocation.h"
#include <glm/mat3x3.hpp>

class RenderContext;
class ModelScreenLocation;
class IModelPreview;

class BaseObject
{
public:
    BaseObject();
    virtual ~BaseObject();

    virtual std::string GetDimension() const = 0;

    virtual const ModelScreenLocation &GetBaseObjectScreenLocation() const = 0;
    virtual ModelScreenLocation &GetBaseObjectScreenLocation() = 0;

    virtual bool CleanupFileLocations(RenderContext* ctx) { return false; }
    virtual std::list<std::string> GetFileReferences() { return std::list<std::string>(); }
    virtual std::list<std::string> CheckModelSettings() { std::list<std::string> res; return res; };

    // SpaceMouse 6-DOF input. Open a session on the active
    // handle (whatever `GetActiveHandleId()` reports), then drive
    // each frame's deltas through `Apply()`. Returns nullptr for
    // locked or fromBase objects so callers can skip cleanly.
    virtual std::unique_ptr<handles::SpaceMouseSession>
    BeginSpaceMouseSession();
    void SelectHandle(int handle);
    void Lock(bool lock);
    bool IsLocked() const;
    virtual void AddASAPWork(uint32_t work, const std::string& from) = 0;
    virtual void ReloadModel() = 0;

    void SetTop(float y);
    void SetBottom(float y);
    void SetLeft(float x);
    void SetRight(float x);
    void SetFront(float y);
    void SetBack(float y);
    void SetHcenterPos(float pos);
    void SetVcenterPos(float pos);
    void SetDcenterPos(float pos);
    void SetWidth(float w, bool ignoreLock = false);
    void SetHeight(float h, bool ignoreLock = false);
    void SetDepth(float d, bool ignoreLock = false);
    bool Rotate(ModelScreenLocation::MSLAXIS axis, float factor);
    void FlipHorizontal(bool ignoreLock = false);
    void FlipVertical(bool ignoreLock = false);

    [[nodiscard]] float GetTop();
    [[nodiscard]] float GetBottom();
    [[nodiscard]] float GetLeft();
    [[nodiscard]] float GetRight();
    [[nodiscard]] float GetFront();
    [[nodiscard]] float GetBack();
    [[nodiscard]] float GetHcenterPos();
    [[nodiscard]] float GetVcenterPos();
    [[nodiscard]] float GetDcenterPos();
    [[nodiscard]] float GetWidth() const;
    [[nodiscard]] float GetHeight() const;
    [[nodiscard]] float GetDepth() const;
    [[nodiscard]] float GetRestorableMWidth() const;
    [[nodiscard]] float GetRestorableMHeight() const;
    [[nodiscard]] float GetRestorableMDepth() const;

    [[nodiscard]] const std::string &Name() const { return name;}
    [[nodiscard]] const std::string &GetName() const { return name;}
    [[nodiscard]] DisplayAsType GetDisplayAs() const { return DisplayAs; }
    [[nodiscard]] std::string GetDisplayAsString() const { return DisplayAsTypeToString(DisplayAs); }
    void SetName(std::string const& newname);

    void SetDisplayAs(DisplayAsType type) { DisplayAs = type; }
    //void SetDisplayAs(const std::string& type) { DisplayAs = DisplayAsTypeFromString(type); }

    virtual void Setup() = 0;
    
    virtual const std::string &GetLayoutGroup() const {return layout_group;}
    void SetLayoutGroup(const std::string &grp, bool ignore_changes = false);

    virtual void IncrementChangeCount() { ++changeCount; uiObjectsInvalid = true; }

	void AddOffset(double deltax, double deltay, double deltaz);
    void RotateAboutPoint(glm::vec3 position, glm::vec3 angle);
    bool Scale(const glm::vec3& factor);

    [[nodiscard]] bool IsContained(IModelPreview* preview, int x1, int y1, int x2, int y2);

    virtual void Accept(BaseObjectVisitor &visitor) const {};

    [[nodiscard]] bool IsActive() const { return _active; }
    void SetActive(bool active);

    [[nodiscard]] bool IsFromBase() const { return _fromBase; }
    void SetFromBase(bool fromBase) { _fromBase = fromBase; }

    std::string name;
    
    bool Selected() const { return _selected; }
    bool Highlighted() const { return _highlighted; }
    bool GroupSelected() const { return _groupSelected; }
    
    void Selected(bool b) { _selected = b; }
    void Highlighted(bool b) { _highlighted = b; }
    void GroupSelected(bool b) { _groupSelected = b; }

protected:
    bool _selected = false;
    bool _highlighted = false;
    bool _groupSelected = false;

    
    DisplayAsType DisplayAs = DisplayAsType::Unknown;
    std::string layout_group;
    unsigned long changeCount = 0;
    bool _active = true;
    bool _fromBase = false;
    
    bool uiObjectsInvalid = true;

private:
};
