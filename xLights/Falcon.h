#ifndef FALCON_H
#define FALCON_H

#include <wx/protocol/http.h>
#include <wx/xml/xml.h>
#include <list>

class Falcon
{
	wxHTTP _http;
	std::string _ip;
    std::string _version;
    bool _connected;
    std::string GetURL(const std::string& url);
    std::string PutURL(const std::string& url, const std::string& request);

public:
    Falcon(const std::string& ip);
    bool IsConnected() const { return _connected; };
    ~Falcon();
    void SetInputUniverses(const std::list<wxXmlNode>& inputs);
    void SetInputUniverses(const wxXmlNode* root);
};

#endif
