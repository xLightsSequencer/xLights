#ifndef FPP_H
#define FPP_H

#include <wx/protocol/http.h>
#include "SimpleFTP.h"
#include <list>
#include "models/ModelManager.h"

class OutputManager;

class FPP
{
	wxHTTP _http;
    SimpleFTP _ftp;
	std::string _user;
	std::string _password;
	std::string _ip;
    std::string _version;
    bool _connected;
    OutputManager* _outputManager;
	
    std::string GetURL(const std::string& url, bool logresult = false);

public:
    std::string Version() { return _version; };
	void RestartFFPD();
	void E131Output(bool enable);
    FPP(OutputManager* outputManager, const std::string& ip, const std::string& user, const std::string& password);
    FPP(OutputManager* outputManager);
    bool IsConnected() const { return _connected; };
    ~FPP();
    bool SetInputUniversesBridge(std::list<int>& selected, wxWindow* parent);
    bool SetOutputUniversesPlayer(wxWindow* parent);
    bool SetChannelMemoryMaps(ModelManager* allmodels, wxWindow* parent);
    std::string SaveFPPUniverses(const std::string& onlyip, const std::list<int>& selected, bool onebased) const;
    std::string SaveFPPChannelMemoryMaps(ModelManager* allmodels) const;
    bool UploadSequence(std::string file, wxWindow* parent);
    static bool IsDefaultPassword(const std::string& user, const std::string& password);

};

#endif
