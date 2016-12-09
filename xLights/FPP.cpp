#include "FPP.h"
#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <log4cpp/Category.hh>
#include <wx/regex.h>

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
    }
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