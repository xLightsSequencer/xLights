#ifndef FPP_H
#define FPP_H

#include <wx/protocol/http.h>
#include "SimpleFTP.h"
#include <list>
#include "models/ModelManager.h"

class OutputManager;
class wxJSONValue;

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
    
    bool GetURLAsJSON(const std::string& url, wxJSONValue& val);
    
    bool uploadFileViaHTTP(const std::string &fn, const std::string &localfile, wxWindow* parent, bool compress = false);

public:
    std::string Version() { return _version; };
	void RestartFFPD();
	void E131Output(bool enable);
    FPP(OutputManager* outputManager, const std::string& ip, const std::string& user = "", const std::string& password = "");
    FPP(OutputManager* outputManager);
    bool IsConnected() const { return _connected; };
    ~FPP();
    bool SetInputUniversesBridge(std::list<int>& selected, wxWindow* parent);
    bool SetOutputUniversesPlayer(wxWindow* parent);
    bool SetChannelMemoryMaps(ModelManager* allmodels, wxWindow* parent);
    std::string SaveFPPUniverses(const std::string& onlyip, const std::list<int>& selected, bool onebased, bool input = false) const;
    std::string SaveFPPUniversesV1(const std::string& onlyip, const std::list<int>& selected, bool onebased) const;
    std::string SaveFPPUniversesV2(const std::string& onlyip, const std::list<int>& selected, bool onebased, bool input) const;
    std::string SaveFPPChannelMemoryMaps(ModelManager* allmodels) const;
    
    bool UploadSequence(const std::string& file, const std::string& fseqDir, wxWindow* parent);
    static bool IsDefaultPassword(const std::string& user, const std::string& password);
    static bool Exists(const std::string& ip);
    
    bool SetOutputs(const std::string &controler, ModelManager* allmodels,
                    std::list<int>& selected, wxWindow* parent,
                    int maxstring, int maxdmx);
    bool UseFTP() const;
};

#endif
