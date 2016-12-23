#ifndef TESTPRESET_H
#define TESTPRESET_H

#include <list>
#include <wx/xml/xml.h>

class TestPreset
{
protected:
	
	std::string _name;
	std::list<int> _channels;

public:
    TestPreset(wxXmlNode* node);
    TestPreset(const std::string& name);
    wxXmlNode* Save();
    virtual ~TestPreset() {};
    std::string GetName() { return _name; }
    std::list<int> GetChannels() const { return _channels; }
    void AddChannel(int ch);
    void AddChannelRange(int start, int end);
};

 #endif