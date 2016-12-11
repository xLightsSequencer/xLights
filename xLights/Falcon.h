#ifndef FALCON_H
#define FALCON_H

#include <wx/protocol/http.h>
#include <wx/xml/xml.h>
#include <list>
#include "Models/ModelManager.h"

class Falcon
{
	wxHTTP _http;
	std::string _ip;
    std::string _version;
    std::string _model;
    bool _connected;
    std::string GetURL(const std::string& url, bool logresult = false);
    std::string PutURL(const std::string& url, const std::string& request, bool logresult = false);
    int DecodeStringPortProtocol(std::string protocol);
    void UploadStringPort(int output, int protocol, int portstart, int pixels, wxWindow* parent);
    int DecodeSerialOutputProtocol(std::string protocol);
    void UploadSerialOutput(int output, int protocol, int portstart, wxWindow* parent);
    void ResetStringOutputs();
    int GetMaxStringOutputs() const;
    int GetMaxSerialOutputs() const;

public:
    Falcon(const std::string& ip);
    bool IsConnected() const { return _connected; };
    ~Falcon();
    void SetInputUniverses(const std::list<wxXmlNode>& inputs);
    void SetInputUniverses(const wxXmlNode* root);
    void SetInputUniverses(const wxXmlNode* root, std::list<int>& selected);
    void SetOutputs(ModelManager* allmodels, wxXmlNode* root, std::list<int>& selected, wxWindow* parent);
};

#endif
