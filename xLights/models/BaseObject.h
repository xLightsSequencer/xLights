#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include "../OutputModelManager.h"
#include "ModelScreenLocation.h"
#include <glm/mat3x3.hpp>

class xLightsFrame;
class wxPropertyGridInterface;
class wxXmlNode;
class ModelScreenLocation;
class ModelPreview;
class OutputManager;
class wxPropertyGridEvent;
class wxMenu;
class wxCommandEvent;

class BaseObject
{
public:
    BaseObject();
    virtual ~BaseObject();

    virtual void AddProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) = 0;
    virtual void UpdateProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) = 0;
    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) = 0;
    virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) = 0;
    virtual void AddSizeLocationProperties(wxPropertyGridInterface* grid) = 0;
    virtual void AddDimensionProperties(wxPropertyGridInterface* grid) = 0;
    virtual void HandlePropertyGridRightClick(wxPropertyGridEvent& event, wxMenu& mnu) {}
    virtual void HandlePropertyGridContextMenu(wxCommandEvent& event) {}
    virtual std::string GetDimension() const = 0;

    virtual const ModelScreenLocation &GetBaseObjectScreenLocation() const = 0;
    virtual ModelScreenLocation &GetBaseObjectScreenLocation() = 0;

    virtual bool CleanupFileLocations(xLightsFrame* frame) { return false; }
    virtual std::list<std::string> GetFileReferences() { return std::list<std::string>(); }
    virtual std::list<std::string> CheckModelSettings() { std::list<std::string> res; return res; };

    virtual glm::vec3 MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z);
    virtual glm::vec3 MoveHandle3D(float scale, int handle, glm::vec3 &rot, glm::vec3 &mov);
    void SelectHandle(int handle);
    void Lock(bool lock);
    bool IsLocked() const;
    virtual void AddASAPWork(uint32_t work, const std::string& from);
    virtual void ReloadModelXml() = 0;

    void EnableLayoutGroupProperty(wxPropertyGridInterface* grid, bool enable);
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
    bool IsXmlChanged(wxXmlNode* n) const;
    bool IsFromBase() const;
    void SetFromBase(bool fromBase);

    float GetTop();
    float GetBottom();
    float GetLeft();
    float GetRight();
    float GetFront();
    float GetBack();
    float GetHcenterPos();
    float GetVcenterPos();
    float GetDcenterPos();
    float GetWidth() const;
    float GetHeight() const;
    float GetDepth() const;

    const std::string &Name() const { return name;}
    const std::string &GetName() const { return name;}
    const std::string& GetDisplayAs() const { return DisplayAs; }

    wxXmlNode* GetModelXml() const;
    virtual void SetFromXml(wxXmlNode* ModelNode, bool zeroBased=false) = 0;

    virtual const std::string &GetLayoutGroup() const {return layout_group;}
    void SetLayoutGroup(const std::string &grp);

    virtual void IncrementChangeCount() { ++changeCount; uiObjectsInvalid = true; }

	void AddOffset(double deltax, double deltay, double deltaz);
    void RotateAboutPoint(glm::vec3 position, glm::vec3 angle);
    bool Scale(const glm::vec3& factor);

    bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2);

	virtual void UpdateXmlWithScale() = 0;

    bool IsActive() const { return _active; }
    void SetActive(bool active);

    std::string name;
    bool Selected = false;
    bool Highlighted = false;
    bool GroupSelected=false;

protected:
    std::string DisplayAs;
    wxXmlNode* ModelXml = nullptr;
    std::string layout_group;
    unsigned long changeCount = 0;
    bool _active = true;
    
    bool uiObjectsInvalid = true;

private:
};
