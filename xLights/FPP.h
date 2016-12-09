#ifndef FPP_H
#define FPP_H

#include <wx/protocol/http.h>

class FPP
{
	wxHTTP _http;
	std::string _user;
	std::string _password;
	std::string _ip;
	
    std::string GetURL(std::string url);

public:
	void RestartFFPD();
	void E131Output(bool enable);
    FPP(std::string ip, std::string user, std::string password);
    bool IsConnected() const;
    ~FPP();
};

#endif
