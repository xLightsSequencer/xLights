#include "PlayListItemFile.h"
#include "PlayListItemFilePanel.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "PlayList.h"
#include "PlayListStep.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <log4cpp/Category.hh>
#include "../RunningSchedule.h"
#include "../../xLights/AudioManager.h"
#include <wx/file.h>

PlayListItemFile::PlayListItemFile(wxXmlNode* node) : PlayListItem(node)
{
    _started = false;
    _content = "";
	_fileName = "";
    PlayListItemFile::Load(node);
}

void PlayListItemFile::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _content = node->GetAttribute("Content", "");
    _fileName = node->GetAttribute("FileName", "");
}

PlayListItemFile::PlayListItemFile() : PlayListItem()
{
    _started = false;
    _content = "";
    _fileName = "";
}

PlayListItem* PlayListItemFile::Copy() const
{
    PlayListItemFile* res = new PlayListItemFile();
    res->_content = _content;
    res->_fileName = _fileName;
    res->_started = false;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemFile::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIFile");

    node->AddAttribute("Content", _content);
    node->AddAttribute("FileName", _fileName);

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemFile::GetTitle() const
{
    return "File";
}

void PlayListItemFile::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemFilePanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemFile::GetNameNoTime() const
{
    if (_name != "") return _name;

    return "File";
}

std::string PlayListItemFile::GetTooltip()
{
    return "Available variables:\n    %RUNNING_PLAYLIST% - current playlist\n    %RUNNING_PLAYLISTSTEP% - step name\n    %RUNNING_PLAYLISTSTEPMS% - Position in current step\n    %RUNNING_PLAYLISTSTEPMSLEFT% - Time left in current step\n    %RUNNING_SCHEDULE% - Name of schedule\n    %STEPNAME% - Current step\n    %ALBUM% - from mp3\n    %TITLE% - from mp3\n    %ARTIST% - from mp3\n    %SHOWDIR% - the current show directory";
}

std::string PlayListItemFile::ReplaceTags(const std::string s)
{
	wxString res = s;

	if (res.Contains("%SHOWDIR%"))
	{
		res.Replace("%SHOWDIR%", xScheduleFrame::GetScheduleManager()->GetShowDir(), true);
	}

	PlayList* pl = xScheduleFrame::GetScheduleManager()->GetRunningPlayList();
	if (pl != nullptr)
	{
		if (res.Contains("%RUNNING_PLAYLIST%"))
		{
			res.Replace("%RUNNING_PLAYLIST%", pl->GetNameNoTime(), true);
		}
		PlayListStep* pls = pl->GetRunningStep();
		if (pls != nullptr)
		{
			if (res.Contains("%RUNNING_PLAYLISTSTEP%"))
			{
				res.Replace("%RUNNING_PLAYLISTSTEP%", pls->GetNameNoTime(), true);
			}
			if (res.Contains("%RUNNING_PLAYLISTSTEPMS%"))
			{
				res.Replace("%RUNNING_PLAYLISTSTEPMS%", wxString::Format(wxT("%i"), pls->GetLengthMS()), true);
			}
			if (res.Contains("%RUNNING_PLAYLISTSTEPMSLEFT%"))
			{
				res.Replace("%RUNNING_PLAYLISTSTEPMSLEFT%", wxString::Format(wxT("%i"), pls->GetLengthMS() - pls->GetPosition()), true);
			}
		}
	}
	if (res.Contains("%RUNNING_SCHEDULE%"))
	{
		RunningSchedule* rs = xScheduleFrame::GetScheduleManager()->GetRunningSchedule();
		if (rs != nullptr && rs->GetPlayList()->IsRunning())
		{
			res.Replace("%RUNNING_SCHEDULE%", rs->GetSchedule()->GetName(), true);
		}
	}

	auto step = xScheduleFrame::GetScheduleManager()->GetRunningPlayList()->GetRunningStep();
	if (step == nullptr)
	{
		step = xScheduleFrame::GetScheduleManager()->GetStepContainingPlayListItem(GetId());
	}

	if (step != nullptr)
	{
		res.Replace("%STEPNAME%", step->GetNameNoTime());

		AudioManager* audio = step->GetAudioManager();
		if (audio != nullptr)
		{
			res.Replace("%TITLE%", audio->Title());
			res.Replace("%ARTIST%", audio->Artist());
			res.Replace("%ALBUM%", audio->Album());
		}
	}
	
	return res.ToStdString();
}

void PlayListItemFile::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (ms >= _delay && !_started)
    {
        _started = true;

        wxString content = ReplaceTags(_content);
        wxString fileName = ReplaceTags(_fileName);

        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        // ensure we are in the show directory before creating the file
        wxSetWorkingDirectory(xScheduleFrame::GetScheduleManager()->GetShowDir());
        logger_base.info("Currentdirectory %s.", (const char *)wxGetCwd().c_str());
        logger_base.info("Writing file %s. Content: '%s'", (const char *)fileName.c_str(), (const char *)content.c_str());

		wxFile f;
		
		f.Create(fileName, true);
		f.Write(content);
		f.Close();

        logger_base.info("File written.");
    }
}

void PlayListItemFile::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    _started = false;
}
