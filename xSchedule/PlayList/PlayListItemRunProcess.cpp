#include "PlayListItemRunProcess.h"
#include "PlayListItemRunProcessPanel.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <log4cpp/Category.hh>

PlayListItemRunProcess::PlayListItemRunProcess(wxXmlNode* node) : PlayListItem(node)
{
    _started = false;
    _command = "";
    _waitForCompletion = false;
    PlayListItemRunProcess::Load(node);
}

void PlayListItemRunProcess::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _command = node->GetAttribute("Command", "");
    _waitForCompletion = (node->GetAttribute("Wait", "FALSE") == "TRUE");
}

PlayListItemRunProcess::PlayListItemRunProcess() : PlayListItem()
{
    _started = false;
    _command = "";
    _waitForCompletion = false;
}

PlayListItem* PlayListItemRunProcess::Copy() const
{
    PlayListItemRunProcess* res = new PlayListItemRunProcess();
    res->_command = _command;
    res->_started = false;
    res->_waitForCompletion = _waitForCompletion;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemRunProcess::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIProcess");

    node->AddAttribute("Command", _command);
    if (_waitForCompletion)
    {
        node->AddAttribute("Wait", "TRUE");
    }

    PlayListItem::Save(node);

    return node;
}

void PlayListItemRunProcess::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemRunProcessPanel(notebook, this), "Run Process", true);
}

std::string PlayListItemRunProcess::GetNameNoTime() const
{
    if (_name != "") return _name;

    return "Run Process";
}

void PlayListItemRunProcess::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems)
{
    if (ms >= _delay && !_started)
    {
        _started = true;

        // we need to run the process
        int flags = 0;
        if (_waitForCompletion)
        {
            flags = wxEXEC_BLOCK;
        }
        else
        {
            flags = wxEXEC_ASYNC;
        }

        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("Launching command %s wait %d.", (const char *)_command.c_str(), (int)_waitForCompletion);
        wxExecute(_command, flags);
        logger_base.info("Command launched.");
    }
}

void PlayListItemRunProcess::Start()
{
    _started = false;
}
