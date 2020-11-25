/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "RemoteFalconOptions.h"

#include <wx/xml/xml.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/config.h>
#include <wx/wxcrt.h>

#include "../../xLights/UtilFunctions.h"

#include <log4cpp/Category.hh>

RemoteFalconOptions::RemoteFalconOptions()
{
}

void RemoteFalconOptions::Load()
{
    wxConfigBase* config = wxConfigBase::Get();
    _token = config->Read(_("RemoteFalconToken"), wxEmptyString).ToStdString();
    _playDuring = config->Read(_("RemoteFalconPlayDuring"), wxEmptyString).ToStdString();
    _playlist = config->Read(_("RemoteFalconPlaylist"), -1);
    _immediatelyInterrupt = config->Read(_("RemoteFalconImmediatelyInterrupt"), true);
    _sendEnableDisable = config->Read(_("RemoteFalconSendEnableDisable"), true);
    _leadTime = config->Read(_("RemoteFalconLeadTime"), 5);
    ClearDirty();
}

void RemoteFalconOptions::Save()
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write(_("RemoteFalconToken"), wxString(_token));
    config->Write(_("RemoteFalconPlayDuring"), wxString(_playDuring));
    config->Write(_("RemoteFalconPlaylist"), _playlist);
    config->Write(_("RemoteFalconLeadTime"), _leadTime);
    config->Write(_("RemoteFalconImmediatelyInterrupt"), _immediatelyInterrupt);
    config->Write(_("RemoteFalconSendEnableDisable"), _sendEnableDisable);
    ClearDirty();
}

bool RemoteFalconOptions::IsValid() const
{
    return _token != "" && _playlist != -1;
}

bool RemoteFalconOptions::IsPlayDuring(const std::string& playlist)
{
	return _playDuring == "" || Contains(_playDuring, "|" + playlist + "|");
}

bool RemoteFalconOptions::IsDirty() const
{
    return _lastSavedChangeCount != _changeCount;
}

void RemoteFalconOptions::ClearDirty()
{
    _lastSavedChangeCount = _changeCount;
}