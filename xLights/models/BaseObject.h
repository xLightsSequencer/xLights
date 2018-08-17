#ifndef BASEOBJECT_H
#define BASEOBJECT_H

class wxPropertyGridInterface;

class BaseObject
{
public:
    BaseObject();
    virtual ~BaseObject();

    virtual void AddTypeProperties(wxPropertyGridInterface *grid) = 0;
    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) = 0;

protected:

private:
};

#endif // BASEOBJECT_H
