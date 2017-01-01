#include "PlayList.h"
#include "PlayListDialog.h"
#include "PlayListStep.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>

PlayList::PlayList(wxXmlNode* node)
{
    _dirty = false;
    _currentStep = nullptr;
    Load(node);
}

PlayList::PlayList()
{
    _dirty = false;
    _currentStep = nullptr;
    _firstOnlyOnce = false;
    _lastOnlyOnce = false;
    _name = "";
}

wxXmlNode* PlayList::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PlayList");

    if (_firstOnlyOnce)
    {
        res->AddAttribute("FirstOnce", "TRUE");
    }

    if (_lastOnlyOnce)
    {
        res->AddAttribute("LastOnce", "TRUE");
    }

    res->AddAttribute("Name", _name);

    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        res->AddChild((*it)->Save());
    }

    _dirty = false;

    return res;
}

void PlayList::Load(wxXmlNode* node)
{
    _firstOnlyOnce = (node->GetAttribute("FirstOnce", "FALSE") == "TRUE");
    _lastOnlyOnce = (node->GetAttribute("LastOnce", "FALSE") == "TRUE");
    _name = node->GetAttribute("Name", "");

    for (wxXmlNode* n = node->GetChildren(); n != nullptr; n = n->GetNext())
    {
        if (n->GetName() == "PlayListStep")
        {
            _steps.push_back(new PlayListStep(n));
        }
    }
}

PlayList* PlayList::Configure(wxWindow* parent)
{
    PlayListDialog dlg(parent, this);

    if (dlg.ShowModal() == wxID_CANCEL)
    {
        return nullptr;
    }

    return this;
}

bool PlayList::IsDirty() const
{
    bool res = _dirty;

    auto it = _steps.begin();
    while (!res && it != _steps.end())
    {
        res = res && (*it)->IsDirty();
        ++it;
    }

    return res;
}

void PlayList::AddStep(PlayListStep* item, int pos)
{
    std::list<PlayListStep*> newsteps;

    int i = 0;
    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        if (i == pos)
        {
            newsteps.push_back(item);
        }
        newsteps.push_back(*it);
        i++;
    }

    _steps = newsteps;
}
