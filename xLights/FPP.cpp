#include "FPP.h"
#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <log4cpp/Category.hh>

FPP::FPP(std::string ip, std::string user, std::string password)
{
 	_user = user;
	_password = password;
	_ip = ip;
	
	_http.Connect(_ip);
}

bool FPP::IsConnected() const
{
    return (_http.IsConnected() && _http.IsOk() && !_http.IsClosed());
}

FPP::~FPP()
{
    _http.Close();
}

std::string FPP::GetURL(std::string url)
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