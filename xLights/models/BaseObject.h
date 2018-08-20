#ifndef BASEOBJECT_H
#define BASEOBJECT_H

#include <string>

class wxPropertyGridInterface;
class wxXmlNode;

class BaseObject
{
public:
    BaseObject();
    virtual ~BaseObject();

    virtual void AddProperties(wxPropertyGridInterface *grid) = 0;
    virtual void AddTypeProperties(wxPropertyGridInterface *grid) = 0;
    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) = 0;

    const std::string &Name() const { return name;}
    const std::string &GetName() const { return name;}
    const std::string& GetDisplayAs(void) const { return DisplayAs; }

    wxXmlNode* GetModelXml() const;

    virtual const std::string &GetLayoutGroup() const {return layout_group;}
    void SetLayoutGroup(const std::string &grp);

    void IncrementChangeCount() { ++changeCount;};

    std::string name;

protected:
    std::string DisplayAs;
    wxXmlNode* ModelXml;
    std::string layout_group;
    unsigned long changeCount;

private:
};

#endif // BASEOBJECT_H
