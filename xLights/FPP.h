#ifndef FPP_H
#define FPP_H

#include <wx/protocol/http.h>
#include "SimpleFTP.h"
#include <wx/xml/xml.h>
#include <list>
#include "models/ModelManager.h"
#include "xLightsMain.h"

class FPP
{
	wxHTTP _http;
    SimpleFTP _ftp;
	std::string _user;
	std::string _password;
	std::string _ip;
    std::string _version;
    bool _connected;
	
    std::string GetURL(const std::string& url, bool logresult = false);

public:
    std::string Version() { return _version; };
	void RestartFFPD();
	void E131Output(bool enable);
    FPP(const std::string& ip, const std::string& user, const std::string& password);
    FPP();
    bool IsConnected() const { return _connected; };
    ~FPP();
    bool SetInputUniversesBridge(wxXmlNode* root, std::list<int>& selected, wxWindow* parent);
    bool SetOutputUniversesPlayer(wxXmlNode* root, wxWindow* parent);
    bool SetChannelMemoryMaps(ModelManager* allmodels, xLightsFrame* frame, wxWindow* parent);
    std::string SaveFPPUniverses(wxXmlNode* root, const std::string& onlyip, const std::list<int>& selected, bool onebased);
    std::string SaveFPPChannelMemoryMaps(ModelManager* allmodels, xLightsFrame* frame);
    bool UploadSequence(std::string file, wxWindow* parent);
};

#endif
