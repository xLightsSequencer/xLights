#ifndef FPP_H
#define FPP_H

#include <wx/protocol/http.h>

class FPP
{
	wxHTTP _http;
	std::string _user;
	std::string _password;
	std::string _ip;
    std::string _version;
    bool _connected;
	
    std::string GetURL(const std::string& url);

public:
    std::string Version() { return _version; };
	void RestartFFPD();
	void E131Output(bool enable);
    FPP(const std::string& ip, const std::string& user, const std::string& password);
    bool IsConnected() const { return _connected; };
    ~FPP();
};

#endif
