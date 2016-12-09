#include "FPP.h"
#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <log4cpp/Category.hh>
#include <wx/regex.h>
#include <wx/file.h>
#include <wx/filename.h>

FPP::FPP(const std::string& ip, const std::string& user, const std::string& password)
{
 	_user = user;
	_password = password;
	_ip = ip;
	
	_connected = _http.Connect(_ip);

    if (_connected)
    {
        std::string version = GetURL("//");

        //Version: <a href = 'about.php' class = 'nonULLink'>v1.6 - 25 - gd87f066
        static wxRegEx versionregex("(Version: .*?nonULLink..v)([0-9]+\\.[0-9]+)", wxRE_ADVANCED | wxRE_NEWLINE);
        if (versionregex.Matches(wxString(version)))
        {
            _version = versionregex.GetMatch(wxString(version), 2).ToStdString();
        }

        _ftp.Connect(ip, user, password);
    }
}

FPP::FPP()
{
    _connected = false;
}

FPP::~FPP()
{
    _http.Close();
}

std::string FPP::GetURL(const std::string& url)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString res;
    _http.SetMethod("GET");
    wxInputStream *httpStream = _http.GetInputStream(wxString(url));
    logger_base.debug("Making request to fpp '%s'.", (const char *)url.c_str());

    if (_http.GetError() == wxPROTO_NOERR)
    {
        wxStringOutputStream out_stream(&res);
        httpStream->Read(out_stream);

        logger_base.debug("Response from fpp '%s'.", (const char *)res.c_str());
    }
    else
    {
        logger_base.error("Unable to connect to fpp '%s'.", (const char *)url.c_str());
        wxMessageBox(_T("Unable to connect!"));
    }

    wxDELETE(httpStream);
    return res.ToStdString();
}

void FPP::RestartFFPD()
{
    GetURL("//fppxml.php?command=restartFPPD");
}

void FPP::E131Output(bool enable)
{
    if (enable)
    {
        GetURL("//fppjson.php?command=setSetting&plugin=&key=E131Enabled&value=1");
    }
    else
    {
        GetURL("//fppjson.php?command=setSetting&plugin=&key=E131Enabled&value=0");
    }
}

bool FPP::SetInputUniversesBridge(wxXmlNode* root, std::list<int>& selected, wxWindow* parent)
{
    if (_ftp.IsConnected())
    {
        // now create a universes file
        std::string file = SaveFPPUniverses(root, _ip, selected);

        bool cancelled = _ftp.UploadFile(file, "/home/fpp/media", "universes", true, false, parent);

        // deactive outputs to these inputs
        E131Output(false);

        // restart ffpd
        RestartFFPD();

        ::wxRemoveFile(wxString(file));

        return cancelled;
    }

    return true;
}

bool FPP::SetOutputUniversesPlayer(wxXmlNode* root, wxWindow* parent)
{
    if (_ftp.IsConnected())
    {
        // now create a universes file
        std::string file = SaveFPPUniverses(root, "", std::list<int>());

        bool cancelled = _ftp.UploadFile(file, "/home/fpp/media", "universes", true, false, parent);

        // restart ffpd
        RestartFFPD();

        ::wxRemoveFile(wxString(file));

        return cancelled;
    }

    return true;
}

std::string FPP::SaveFPPUniverses(wxXmlNode* root, const std::string& onlyip, const std::list<int>& selected)
{
    wxFileName fn;
    fn.AssignTempFileName("universes");
    std::string file = fn.GetFullName().ToStdString();
    wxFile universes;
    universes.Open(file, wxFile::write);
    int node = 0;

    if (universes.IsOpened())
    {
        wxXmlNode* e = root;
        long count = 1;

        for (e = e->GetChildren(); e != nullptr; e = e->GetNext())
        {
            if (e->GetName() == "network")
            {
                std::string type = std::string(e->GetAttribute("NetworkType", ""));

                if (type == "E131")
                {
                    std::string ip = std::string(e->GetAttribute("ComPort", ""));
                    if (onlyip == "" || ip == onlyip || std::find(selected.begin(), selected.end(), node) != selected.end())
                    {
                        int universe = wxAtoi(e->GetAttribute("BaudRate", ""));
                        wxString MaxChannelsStr = e->GetAttribute("MaxChannels", "0");
                        long chan;
                        MaxChannelsStr.ToLong(&chan);

                        int ucount = wxAtoi(e->GetAttribute("NumUniverses", "1"));

                        for (size_t i = 0; i < ucount; i++)
                        {
                            long end = count + chan - 1;

                            if (ip == "MULTICAST")
                            {
                                universes.Write("1," + wxString::Format("%d", universe + i).ToStdString() + "," + std::string(wxString::Format(wxT("%i"), count)) + "," + std::string(wxString::Format(wxT("%i"), chan)) + ",0,,\r\n");
                            }
                            else
                            {
                                universes.Write("1," + wxString::Format("%d", universe + i).ToStdString() + "," + std::string(wxString::Format(wxT("%i"), count)) + "," + std::string(wxString::Format(wxT("%i"), chan)) + ",1," + ip + ",\r\n");
                            }

                            count = end + 1;
                        }
                    }
                }
                node++;
            }
        }
        universes.Close();
    }

    return file;
}
