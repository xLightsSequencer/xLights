#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include <map>
#include <string>

class Model;
class wxXmlNode;
class NetInfoClass;

class ModelManager
{
    public:
        ModelManager();
        virtual ~ModelManager();
    
    
        Model *operator[](const std::string &name) const;
    
        Model *createModel(wxXmlNode *node, NetInfoClass &ni);
    
        void clear();
    
        std::map<std::string, Model*>::const_iterator begin() const;
        std::map<std::string, Model*>::const_iterator end() const;
        int size() const;
    protected:
    private:
    
    
    std::map<std::string, Model *> models;
};

#endif // MODELMANAGER_H
