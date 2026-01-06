#include <map>
#include <wx/wx.h>
#include <wx/textfile.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

#include "spdlog/spdlog.h"

#include <mutex>

static std::map<std::string, std::string> macLookup;
static std::mutex _macMutex;

void ProcessMACLine(const std::string& line)
{
	auto ls = wxSplit(line, '|');
	if (ls.size() == 2) {
		macLookup[ls[0]] = ls[1];
	}
}

void LoadMacLookup()
{
	#ifndef __WXMSW__
    wxString const MACLOOKUP = wxStandardPaths::Get().GetResourcesDir() + "/xScanner/MacLookup.txt";
	#else
	wxString const MACLOOKUP = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath() + wxFileName::GetPathSeparator() + "MacLookup.txt";
	#endif

	if (wxFile::Exists(MACLOOKUP)) {
        spdlog::debug("Loading {}", MACLOOKUP.ToStdString());
		wxTextFile f;
		if (f.Open(MACLOOKUP))
		{
			std::string str;
			for (str = f.GetFirstLine(); !f.Eof(); str = f.GetNextLine()) {
				ProcessMACLine(str);
				// do something with the current line in str
			}
			// do something with the last line in str		
			ProcessMACLine(str);
		}
	}
	else {
        spdlog::warn("{} Not Found!", MACLOOKUP.ToStdString());
	}

	if (macLookup.size() == 0) {
		macLookup["FFFFFFFF"] = "NOFILE";
	}
}

std::string LookupMacAddress(const std::string mac)
{
	std::unique_lock<std::mutex> lock(_macMutex);

	if (macLookup.size() == 0) {
		LoadMacLookup();
	}

	std::string res;

	if (mac != "" && macLookup.size() > 1) {
		auto macs = wxSplit(mac, '-');

		if (macs.size() > 2) {
			std::string s;
			for (size_t i = 0; i < 3; i++) {
				s += macs[i];
			}

			if (macLookup.find(s) != end(macLookup)) {
				res = macLookup[s];
                spdlog::debug("Mac {} found in lookup file => {}", s, res);
			}
		}
	}

	return res;
}
