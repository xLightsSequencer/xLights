#include "ScheduleManager.h"
#include <wx/xml/xml.h>
#include <wx/msgdlg.h>
#include <wx/log.h>

ScheduleManager::ScheduleManager(const std::string& showDir)
{
    _showDir = showDir;

    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    _dirty = false;
	wxXmlDocument doc;
	doc.Load(showDir + "/" + GetScheduleFile());

    if (doc.IsOk())
    {
        for (wxXmlNode* n = doc.GetRoot()->GetChildren(); n != nullptr; n = n->GetNext())
        {
            if (n->GetName() == "PlayList")
            {
                _playLists.push_back(new PlayList(n));
            }
            //else if (n->GetName() == "Schedule")
            //{
            //    _playLists.push_back(new Schedule(n));
            //}
        }
    }
}

ScheduleManager::~ScheduleManager()
{
	if (IsDirty())
	{
		if (wxMessageBox("Unsaved changes to the schedule. Save now?", "Unsave changes", wxYES_NO) == wxID_YES)
		{
			Save();
		}
	}
}

bool ScheduleManager::IsDirty()
{
    bool res = _dirty;

    auto it = _playLists.begin();
    while (!res && it != _playLists.end())
    {
        res = res && (*it)->IsDirty();
        ++it;
    }

    return res;
}

void ScheduleManager::Save()
{
	wxXmlDocument doc;
    wxXmlNode* root = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "xSchedule");
    doc.SetRoot(root);
	
	for(auto it = _playLists.begin(); it != _playLists.end(); ++it)
	{
		root->AddChild((*it)->Save());
	}

    doc.Save(_showDir + "/" + GetScheduleFile());
    ClearDirty();
}

void ScheduleManager::ClearDirty()
{
    _dirty = false;

    for (auto it = _playLists.begin(); it != _playLists.end(); ++it)
    {
        (*it)->ClearDirty();
    }
}