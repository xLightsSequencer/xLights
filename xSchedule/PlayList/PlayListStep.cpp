#include "PlayListStep.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>
#include "PlayListItemVideo.h"
#include "PlayListItemFSEQ.h"
#include "PlayListItemAllOff.h"
#include "PlayListItemRunProcess.h"

PlayListStep::PlayListStep(wxXmlNode* node)
{
    _dirty = false;
    Load(node);
}

PlayListStep::PlayListStep()
{
    _name = "";
    _dirty = false;
}

wxXmlNode* PlayListStep::Save()
{
    wxXmlNode* res = new wxXmlNode();

    res->AddAttribute("Name", _name);

    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        res->AddChild((*it)->Save());
    }

    _dirty = false;

    return res;
}

void PlayListStep::Load(wxXmlNode* node)
{
    _name = node->GetAttribute("Name", "");

    for (wxXmlNode* n = node->GetChildren(); n != nullptr; n = n->GetNext())
    {
        if (n->GetName() == "PLIVideo")
        {
            _items.push_back(new PlayListItemVideo(n));
        }
        else if (n->GetName() == "PLIFSEQ")
        {
            _items.push_back(new PlayListItemFSEQ(n));
        }
        else if (n->GetName() == "PLIAllOff")
        {
            _items.push_back(new PlayListItemAllOff(n));
        }
        else if (n->GetName() == "PLIProcess")
        {
            _items.push_back(new PlayListItemRunProcess(n));
        }
    }
}

bool PlayListStep::IsDirty() const
{
    bool res = _dirty;

    auto it = _items.begin();
    while (!res && it != _items.end())
    {
        res = res && (*it)->IsDirty();
        ++it;
    }

    return res;
}