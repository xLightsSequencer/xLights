#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <map>
#include <vector>

#include "Node.h"
#include "../Color.h"

class DimmingCurve;

class Model
{
public:
    Model();
    virtual ~Model();
protected:
private:
    std::string name;

    int BufferHt,BufferWi;  // size of the buffer
    xlColor customColor;
    std::vector<NodeBaseClassPtr> Nodes;

    
    
    
    std::map<std::string, std::map<std::string, std::string> > faceInfo;
    DimmingCurve *modelDimmingCurve;
    
    
};

#endif // MODEL_H
