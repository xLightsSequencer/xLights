#ifndef BASEOBJECT_H
#define BASEOBJECT_H

#include <string>

class xLightsFrame;
class wxPropertyGridInterface;
class wxXmlNode;
class ModelScreenLocation;
class ModelPreview;

class BaseObject
{
public:
    BaseObject();
    virtual ~BaseObject();

    virtual void AddProperties(wxPropertyGridInterface *grid) = 0;
    virtual void AddTypeProperties(wxPropertyGridInterface *grid) = 0;
    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) = 0;

    virtual const ModelScreenLocation &GetBaseObjectScreenLocation() const = 0;
    virtual ModelScreenLocation &GetBaseObjectScreenLocation() = 0;

    virtual bool CleanupFileLocations(xLightsFrame* frame) { return false; }
    virtual std::list<std::string> GetFileReferences() { return std::list<std::string>(); }
    virtual std::list<std::string> CheckModelSettings() { std::list<std::string> res; return res; };

    void MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z);
    void SelectHandle(int handle);
    void Lock(bool lock);

    void SetTop(float y);
    void SetBottom(float y);
    void SetLeft(float x);
    void SetRight(float x);
    void SetFront(float y);
    void SetBack(float y);
    void SetHcenterPos(float pos);
    void SetVcenterPos(float pos);
    void SetWidth(float w);
    void SetHeight(float h);

    float GetTop();
    float GetBottom();
    float GetLeft();
    float GetRight();
    float GetFront();
    float GetBack();
    float GetHcenterPos();
    float GetVcenterPos();
    float GetWidth();
    float GetHeight();

    const std::string &Name() const { return name;}
    const std::string &GetName() const { return name;}
    const std::string& GetDisplayAs(void) const { return DisplayAs; }

    wxXmlNode* GetModelXml() const;
    virtual void SetFromXml(wxXmlNode* ModelNode, bool zeroBased=false) = 0;

    virtual const std::string &GetLayoutGroup() const {return layout_group;}
    void SetLayoutGroup(const std::string &grp);

    void IncrementChangeCount() { ++changeCount;};

    std::string name;

    bool Selected = false;
    bool Highlighted = false;
    bool GroupSelected=false;

protected:
    std::string DisplayAs;
    wxXmlNode* ModelXml;
    std::string layout_group;
    unsigned long changeCount;

private:
};

#endif // BASEOBJECT_H
