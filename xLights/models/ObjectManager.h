#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

class wxXmlNode;

class ObjectManager
{
public:
    ObjectManager();
    virtual ~ObjectManager();

protected:

private:
    wxXmlNode *modelNode;

};

#endif // OBJECTMANAGER_H
