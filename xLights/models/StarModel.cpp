#include "StarModel.h"

StarModel::StarModel(wxXmlNode *node, NetInfoClass &netInfo)
{
    SetFromXml(node, netInfo);
}

StarModel::~StarModel()
{
    //dtor
}
