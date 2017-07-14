#ifndef PIXLITE_H
#define PIXLITE_H

#include <wx/protocol/http.h>
#include <list>
#include "models/ModelManager.h"

class Output;
class OutputManager;

class Pixlite16
{
    wxByte _pixliteData[338];
	std::string _ip;
    bool _connected;
    bool SendConfig(bool logresult = false);
    int DecodeStringPortProtocol(std::string protocol);
    int DecodeSerialOutputProtocol(std::string protocol);
    void UploadSerialOutput(int output, int protocol, int portstart, wxWindow* parent);
    int GetMaxStringOutputs() const;
    int GetMaxSerialOutputs() const;

public:
    Pixlite16(const std::string& ip);
    bool IsConnected() const { return _connected; };
    ~Pixlite16();
    void SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);
};

#endif
