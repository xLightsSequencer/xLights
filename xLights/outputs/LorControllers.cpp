#include "LorControllers.h"

LorControllers::LorControllers()
{
    //ctor
}

LorControllers::LorControllers(wxXmlNode* node)
{
    wxXmlNode* ctrlr_node = node->GetChildren();
    for(wxXmlNode* e=ctrlr_node->GetChildren(); e!=nullptr; e=e->GetNext() )
    {
        _controllers.push_back(new LorController(e));
    }
    _changeCount = 0;
    _lastSavedChangeCount = 0;
}

LorControllers::~LorControllers()
{
    //dtor
}

void LorControllers::Save(wxXmlNode* node)
{
    for (auto it = _controllers.begin(); it != _controllers.end(); ++it)
    {
        wxXmlNode* cntrl_node = new wxXmlNode(wxXML_ELEMENT_NODE, "controller");
        node->AddChild(cntrl_node);
        (*it)->Save(cntrl_node);
    }
}

bool LorControllers::IsDirty() const
{
    bool res = _lastSavedChangeCount != _changeCount;

    for (auto it = _controllers.begin(); it != _controllers.end(); ++it)
    {
        res = res || (*it)->IsDirty();
    }

    return res;
}

void LorControllers::ClearDirty()
{
    _lastSavedChangeCount = _changeCount;

    for (auto it = _controllers.begin(); it != _controllers.end(); ++it)
    {
        (*it)->ClearDirty();
    }
}

